#!/usr/bin/env python3
"""Offline forwarding regression: python3 forwarding.py /path/to/binaries.

Uses only Python's standard library, loopback sockets, and the four built Unbound
programs. The fixture contains a test-only TLS key and an Ed25519 DNSKEY.
Signatures and certificates are valid
from 2020 until 2036. It is never an operational trust anchor.
"""
import base64
import collections
import json
from pathlib import Path
import socket
import socketserver
import ssl
import struct
import subprocess
import sys
import tempfile
import threading
import time


def wire_name(name):
    return b''.join(bytes([len(s)]) + s.encode()
                    for s in name.rstrip('.').split('.')) + b'\0'


def read_name(packet, offset):
    labels = []
    while packet[offset]:
        length = packet[offset]
        if length & 0xc0 == 0xc0:
            target = ((length & 63) << 8) | packet[offset + 1]
            suffix, _ = read_name(packet, target)
            return '.'.join(labels) + ('.' if labels else '') + suffix, offset + 2
        labels.append(packet[offset + 1:offset + 1 + length].decode())
        offset += length + 1
    return '.'.join(labels) + '.', offset + 1


def rr(name, rtype, data, ttl=60):
    return wire_name(name) + struct.pack('!HHIH', rtype, 1, ttl, len(data)) + data


def recv_exact(sock, count):
    data = b''
    while len(data) < count:
        part = sock.recv(count - len(data))
        if not part:
            raise EOFError('short TCP DNS response')
        data += part
    return data


class Upstream:
    def __init__(self, fixture, address='192.0.2.80', tls=None):
        self.fixture = fixture
        self.address = socket.inet_aton(address)
        self.count = collections.Counter()
        self.edns_options = collections.Counter()
        self.drop = False
        owner = self

        class UDP(socketserver.BaseRequestHandler):
            def handle(self):
                packet, sock = self.request
                response = owner.reply(packet, False)
                if response:
                    sock.sendto(response, self.client_address)

        class TCP(socketserver.BaseRequestHandler):
            def handle(self):
                self.request.settimeout(3)
                try:
                    while True:
                        size = struct.unpack('!H', recv_exact(self.request, 2))[0]
                        response = owner.reply(recv_exact(self.request, size), True)
                        if response:
                            self.request.sendall(struct.pack('!H', len(response)) + response)
                except (EOFError, OSError):
                    pass

        class Server(socketserver.ThreadingTCPServer):
            def get_request(self):
                sock, address = super().get_request()
                if tls:
                    try:
                        sock = tls.wrap_socket(sock, server_side=True)
                    except OSError:
                        sock.close()
                        raise
                return sock, address

        self.tcp = Server(('127.0.0.1', 0), TCP)
        self.tcp.daemon_threads = True
        self.port = self.tcp.server_address[1]
        self.udp = socketserver.ThreadingUDPServer(('127.0.0.1', self.port), UDP)
        self.udp.daemon_threads = True
        for server in (self.tcp, self.udp):
            threading.Thread(target=server.serve_forever, daemon=True).start()

    def reply(self, packet, tcp):
        name, end = read_name(packet, 12)
        rtype, _ = struct.unpack('!HH', packet[end:end + 4])
        question = packet[12:end + 4]
        offset = end + 4
        # Outgoing requests have only a question and optional additional records.
        for _ in range(struct.unpack('!H', packet[10:12])[0]):
            _, offset = read_name(packet, offset)
            kind, _, _, size = struct.unpack('!HHIH', packet[offset:offset + 10])
            offset += 10
            if kind == 41:
                data = packet[offset:offset + size]
                while data:
                    code, length = struct.unpack('!HH', data[:4])
                    self.edns_options[code] += 1
                    data = data[4 + length:]
            offset += size
        self.count[(name, rtype, tcp)] += 1
        if self.drop:
            return None
        flags, answer, authority = 0x8180, [], []
        if name == 'fallback.forward.test.' and not tcp:
            flags |= 0x200
        elif name == 'negative.forward.test.':
            flags |= 3
            soa = wire_name('ns.forward.test.') + wire_name('hostmaster.forward.test.')
            soa += struct.pack('!IIIII', 1, 60, 60, 60, 60)
            authority.append(rr('forward.test.', 6, soa))
        elif name.endswith('signed.test.'):
            flags |= 0x400
            if rtype == 48 and name == 'signed.test.':
                answer += [rr(name, 48, base64.b64decode(self.fixture['dnskey'])),
                           rr(name, 46, base64.b64decode(self.fixture['rrsig_dnskey']))]
            elif rtype == 1 and name in ('good.signed.test.', 'bad.signed.test.'):
                address = self.address if name.startswith('good.') else socket.inet_aton('192.0.2.81')
                signature = 'rrsig_good' if name.startswith('good.') else 'rrsig_bad'
                answer += [rr(name, 1, address),
                           rr(name, 46, base64.b64decode(self.fixture[signature]))]
        elif rtype == 28:
            answer.append(rr(name, 28, socket.inet_pton(socket.AF_INET6, '2001:db8::80')))
        elif rtype == 1:
            if name == 'alias.forward.test.':
                answer.append(rr(name, 5, wire_name('answer.forward.test.')))
                name = 'answer.forward.test.'
            answer.append(rr(name, 1, self.address, 2 if name == 'expired.forward.test.' else 10 if name == 'prefetch.forward.test.' else 60))
        return packet[:2] + struct.pack('!HHHHH', flags, 1, len(answer), len(authority), 0) + question + b''.join(answer + authority)

    def close(self):
        for server in (self.tcp, self.udp):
            server.shutdown()
            server.server_close()


def query(port, name, rtype=1, tcp=False, cd=False, options=b'', source=None):
    flags = 0x100 | (0x10 if cd else 0)
    packet = struct.pack('!HHHHHH', 1234, flags, 1, 0, 0, 1)
    packet += wire_name(name) + struct.pack('!HH', rtype, 1)
    packet += b'\0' + struct.pack('!HHIH', 41, 1232, 0x8000, len(options)) + options
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM if tcp else socket.SOCK_DGRAM) as sock:
        sock.settimeout(10)
        if source:
            sock.bind((source, 0))
        sock.connect(('127.0.0.1', port))
        if tcp:
            sock.sendall(struct.pack('!H', len(packet)) + packet)
            response = recv_exact(sock, struct.unpack('!H', recv_exact(sock, 2))[0])
        else:
            sock.send(packet)
            response = sock.recv(65535)
    header = struct.unpack('!HHHHHH', response[:12])
    offset = 12
    for _ in range(header[2]):
        _, offset = read_name(response, offset)
        offset += 4
    answers = []
    for _ in range(header[3]):
        _, offset = read_name(response, offset)
        rtype, _, _, size = struct.unpack('!HHIH', response[offset:offset + 10])
        offset += 10
        answers.append((rtype, response[offset:offset + size]))
        offset += size
    return header[1], answers


def main():
    bindir = Path(sys.argv[1]).resolve()
    fixture = json.loads(Path(__file__).with_name('dnssec-fixture.json').read_text())
    upstream = Upstream(fixture)
    replacement = Upstream(fixture, '192.0.2.82')
    encrypted = None
    process = None
    try:
        with tempfile.TemporaryDirectory(prefix='unbound-regression-') as directory:
            root = Path(directory)
            config, log = root / 'unbound.conf', root / 'unbound.log'
            with socket.socket() as reserve:
                reserve.bind(('127.0.0.1', 0))
                port = reserve.getsockname()[1]
            key = base64.b64decode(fixture['dnskey'])
            trust = base64.b64encode(key[4:]).decode()

            def write_config(upstream_port, local='192.0.2.10'):
                config.write_text(f'''server:
    interface: 127.0.0.1@{port}
    do-ip6: no
    do-not-query-localhost: no
    username: ""
    chroot: ""
    directory: "{root}"
    pidfile: "{root}/unbound.pid"
    logfile: "{log}"
    use-syslog: no
    module-config: "validator iterator"
    access-control: 127.0.0.0/8 allow
    access-control: 127.0.0.3/32 refuse
    prefetch: yes
    serve-expired: yes
    serve-expired-client-timeout: 0
    statistics-cumulative: yes
    local-zone: "home.test." static
    local-zone: "forward.test." transparent
    local-zone: "signed.test." transparent
    local-data: "host.home.test. 60 IN A {local}"
    local-data: "alias.home.test. 60 IN CNAME host.home.test."
    local-data-ptr: "192.0.2.10 host.home.test."
    trust-anchor: "signed.test. DNSKEY 257 3 15 {trust}"
remote-control:
    control-enable: yes
    control-interface: "{root}/control.sock"
    control-use-cert: no
forward-zone:
    name: "."
    forward-first: no
    forward-addr: 127.0.0.1@{upstream_port}
''')

            def control(*args):
                return subprocess.check_output([str(bindir / 'unbound-control'), '-c', str(config), *args], text=True)

            write_config(upstream.port)
            subprocess.run([str(bindir / 'unbound-checkconf'), str(config)], check=True)
            process = subprocess.Popen([str(bindir / 'unbound'), '-d', '-c', str(config)])
            for _ in range(100):
                if process.poll() is not None:
                    raise RuntimeError(log.read_text())
                try:
                    query(port, 'host.home.test.')
                    break
                except OSError:
                    time.sleep(.05)
            else:
                raise RuntimeError('resolver did not become ready')

            for tcp in (False, True):
                flags, answers = query(port, 'host.home.test.', tcp=tcp)
                assert flags & 15 == 0 and (1, socket.inet_aton('192.0.2.10')) in answers
            assert any(t == 5 for t, _ in query(port, 'alias.home.test.')[1])
            assert any(t == 12 for t, _ in query(port, '10.2.0.192.in-addr.arpa.', 12)[1])
            print('PASS local A/CNAME/PTR and incoming UDP/TCP')

            for tcp in (False, True):
                assert (1, socket.inet_aton('192.0.2.10')) in query(
                    port, 'host.home.test.', tcp=tcp, source='127.0.0.2')[1]
                assert query(port, 'host.home.test.', tcp=tcp, source='127.0.0.3')[0] & 15 == 5
            assert 'ok' in control('local_zone', 'admin.home.test.', 'static')
            assert 'ok' in control('local_data', 'admin.home.test. 60 IN A 192.0.2.14')
            for tcp in (False, True):
                assert (1, socket.inet_aton('192.0.2.14')) in query(
                    port, 'admin.home.test.', tcp=tcp, source='127.0.0.2')[1]
            assert 'ok' in control('local_data_remove', 'admin.home.test.')
            assert 'ok' in control('local_zone_remove', 'admin.home.test.')
            assert query(port, 'admin.home.test.')[0] & 15 == 3
            for command in ('view_list_local_zones', 'view_local_zone', 'view_local_data'):
                result = subprocess.run([str(bindir / 'unbound-control'), '-c', str(config),
                                         command, 'private'], capture_output=True)
                assert result.returncode != 0, command
            print('PASS LAN-style source ACLs and global local-zone/data administration')

            flags, answers = query(port, 'answer.forward.test.')
            assert flags & 15 == 0 and (1, socket.inet_aton('192.0.2.80')) in answers
            assert upstream.count[('answer.forward.test.', 1, False)]
            count = sum(upstream.count.values())
            query(port, 'answer.forward.test.', tcp=True)
            assert sum(upstream.count.values()) == count
            assert query(port, 'negative.forward.test.')[0] & 15 == 3
            count = sum(upstream.count.values())
            assert query(port, 'negative.forward.test.')[0] & 15 == 3
            assert sum(upstream.count.values()) == count
            assert any(t == 5 for t, _ in query(port, 'alias.forward.test.')[1])
            print('PASS forwarding, CNAME chain, positive and negative cache hits')

            for command, argument, name in (
                    ('flush_type', 'answer.forward.test. A', 'answer.forward.test.'),
                    ('flush', 'answer.forward.test.', 'answer.forward.test.'),
                    ('flush_zone', 'forward.test.', 'answer.forward.test.'),
                    ('flush_negative', '', 'negative.forward.test.')):
                query(port, name)
                count = sum(upstream.count.values())
                control(command, *argument.split())
                query(port, name)
                assert sum(upstream.count.values()) > count, command
            for command in ('flush_bogus', 'flush_negative'):
                result = subprocess.run([str(bindir / 'unbound-control'), '-c', str(config),
                                         command, '+c'], capture_output=True)
                assert result.returncode != 0, command
            stats = control('stats_noreset')
            assert not any(word in stats for word in ('cachedb', 'subnet', 'dnscrypt',
                                                      'mem.http.', 'num.query.https',
                                                      'num.query.quic', 'mem.quic'))
            print('PASS in-memory cache flush commands and statistics')
            # Unsupported ECS is ignored, never propagated, and does not split cache.
            for tcp in (False, True):
                ecs = struct.pack('!HHHBB', 8, 7, 1, 24, 0) + b'\xc0\x00\x02'
                count = sum(upstream.count.values())
                assert query(port, 'answer.forward.test.', tcp=tcp, options=ecs)[0] & 15 == 0
                assert sum(upstream.count.values()) == count
            assert 'subnet' not in control('stats_noreset')
            assert query(port, 'ecs.forward.test.', options=ecs)[0] & 15 == 0
            assert upstream.edns_options[8] == 0
            # Bad ECS contents have no interpretation when ECS is unsupported.
            assert query(port, 'ecs-malformed.forward.test.',
                         options=struct.pack('!HH', 8, 1) + b'x')[0] & 15 == 0
            print('PASS unsupported ECS with ordinary EDNS and shared cache over UDP/TCP')

            # Multiple requests can coexist on one incoming TCP connection.
            packets = []
            for identity, name in enumerate(('host.home.test.', 'pipeline.forward.test.',
                                              'alias.home.test.'), 2000):
                packet = struct.pack('!HHHHHH', identity, 0x100, 1, 0, 0, 1)
                packet += wire_name(name) + struct.pack('!HH', 1, 1)
                packet += b'\0' + struct.pack('!HHIH', 41, 1232, 0x8000, 0)
                packets.append(struct.pack('!H', len(packet)) + packet)
            with socket.create_connection(('127.0.0.1', port), timeout=10) as sock:
                sock.sendall(b''.join(packets))
                identities = set()
                for _ in packets:
                    reply = recv_exact(sock, struct.unpack('!H', recv_exact(sock, 2))[0])
                    identity, flags = struct.unpack('!HH', reply[:4])
                    assert flags & 15 == 0 and flags & 0x8000
                    identities.add(identity)
                assert identities == {2000, 2001, 2002}
                sock.sendall(packets[0])
                reply = recv_exact(sock, struct.unpack('!H', recv_exact(sock, 2))[0])
                assert struct.unpack('!H', reply[:2])[0] == 2000
            print('PASS pipelined TCP requests and connection reuse')

            assert query(port, 'fallback.forward.test.')[0] & 15 == 0
            assert upstream.count[('fallback.forward.test.', 1, False)]
            assert upstream.count[('fallback.forward.test.', 1, True)]
            print('PASS upstream truncation and outgoing TCP fallback')
            assert (28, socket.inet_pton(socket.AF_INET6, '2001:db8::80')) in query(port, 'aaaa.forward.test.', 28, cd=True)[1]
            count = sum(upstream.count.values())
            assert (28, socket.inet_pton(socket.AF_INET6, '2001:db8::80')) in query(port, 'aaaa.forward.test.', 28)[1]
            assert sum(upstream.count.values()) == count
            print('PASS ordinary AAAA records and shared CD/non-CD cache entries')

            flags, _ = query(port, 'good.signed.test.')
            assert flags & 0x20 and flags & 15 == 0, hex(flags)
            assert query(port, 'good.signed.test.', tcp=True)[0] & 0x20
            assert query(port, 'bad.signed.test.')[0] & 15 == 2
            assert query(port, 'bad.signed.test.', cd=True)[0] & 15 == 0
            control('flush_bogus')
            assert query(port, 'bad.signed.test.')[0] & 15 == 2
            print('PASS DNSSEC valid signature, corrupt signature rejection, CD handling')

            query(port, 'prefetch.forward.test.')
            count = upstream.count[('prefetch.forward.test.', 1, False)]
            time.sleep(9.2)
            query(port, 'prefetch.forward.test.')
            for _ in range(100):
                if upstream.count[('prefetch.forward.test.', 1, False)] > count:
                    break
                time.sleep(.05)
            else:
                raise AssertionError('prefetch did not refresh the upstream answer')
            print('PASS cache prefetch')

            query(port, 'expired.forward.test.')
            upstream.drop = True
            time.sleep(3)
            assert query(port, 'expired.forward.test.')[0] & 15 == 0
            upstream.drop = False
            print('PASS serve-expired with unavailable upstream')

            upstream.drop = True
            assert 'ok' in control('forward_add', '.', f'127.0.0.1@{upstream.port}', f'127.0.0.1@{replacement.port}')
            assert (1, socket.inet_aton('192.0.2.82')) in query(port, 'failover.forward.test.')[1]
            upstream.drop = False
            print('PASS forwarding with one unavailable upstream and runtime forward change')

            stats = control('stats_noreset')
            assert 'total.num.cachehits=' in stats and 'total.num.cachemiss=' in stats
            write_config(replacement.port, '192.0.2.11')
            assert 'ok' in control('reload')
            for _ in range(100):
                if (1, socket.inet_aton('192.0.2.11')) in query(port, 'host.home.test.')[1]:
                    break
                time.sleep(.05)
            else:
                raise AssertionError('reload did not update local data')
            assert (1, socket.inet_aton('192.0.2.82')) in query(port, 'reload.forward.test.')[1]
            assert replacement.count[('reload.forward.test.', 1, False)]
            print('PASS statistics, authenticated-by-filesystem control, reload and forwarding change')

            write_config(replacement.port, '192.0.2.12')
            control('fast_reload', '+v')
            assert (1, socket.inet_aton('192.0.2.12')) in query(port, 'host.home.test.')[1]
            for tcp in (False, True):
                assert (1, socket.inet_aton('192.0.2.12')) in query(
                    port, 'host.home.test.', tcp=tcp, source='127.0.0.2')[1]
                assert query(port, 'host.home.test.', tcp=tcp, source='127.0.0.3')[0] & 15 == 5
            print('PASS fast reload preserving global records and source ACLs')

            # Use a private test CA to exercise both acceptance and rejection
            # of upstream certificates, without external network access.
            for name in ('ca', 'cert', 'key'):
                (root / f'{name}.pem').write_text(fixture[f'tls_{name}'])
            context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
            context.load_cert_chain(root / 'cert.pem', root / 'key.pem')
            encrypted = Upstream(fixture, tls=context)
            process.terminate()
            process.wait(timeout=10)
            for hostname in ('upstream.test', 'wrong.test'):
                write_config(encrypted.port)
                text = config.read_text().replace(
                    '    use-syslog: no',
                    f'    tls-cert-bundle: "{root}/ca.pem"\n'
                    '    outbound-msg-retry: 1\n    max-sent-count: 2\n'
                    '    use-syslog: no')
                text = text.replace('    forward-first: no',
                                    '    forward-first: no\n    forward-tls-upstream: yes')
                text = text.replace(f'127.0.0.1@{encrypted.port}\n',
                                    f'127.0.0.1@{encrypted.port}#{hostname}\n')
                config.write_text(text)
                subprocess.run([str(bindir / 'unbound-checkconf'), str(config)], check=True)
                process = subprocess.Popen([str(bindir / 'unbound'), '-d', '-c', str(config)])
                for _ in range(100):
                    try:
                        query(port, 'host.home.test.')
                        break
                    except OSError:
                        time.sleep(.05)
                flags, answers = query(port, 'tls.forward.test.')
                if hostname == 'upstream.test':
                    assert flags & 15 == 0 and (1, socket.inet_aton('192.0.2.80')) in answers
                    (root / 'ca-reload.pem').write_text(fixture['tls_ca'])
                    config.write_text(text.replace(str(root / 'ca.pem'), str(root / 'ca-reload.pem')))
                    assert 'ok' in control('reload')
                    for _ in range(100):
                        try:
                            assert 'is running' in control('status')
                            break
                        except subprocess.CalledProcessError:
                            time.sleep(.05)
                    else:
                        raise AssertionError('outgoing TLS reload did not recover')
                    assert (1, socket.inet_aton('192.0.2.80')) in query(port, 'tls-reload.forward.test.')[1]
                    config.write_text(text)
                    control('fast_reload', '+v')
                    assert (1, socket.inet_aton('192.0.2.80')) in query(port, 'tls-fast-reload.forward.test.', tcp=True)[1]
                else:
                    assert flags & 15 == 2, hex(flags)
                process.terminate()
                process.wait(timeout=10)
            print('PASS outgoing TLS certificate validation, CA-path reload/fast_reload, and hostname mismatch rejection')

            # Remote control retains mutual TLS authentication independently of DoH.
            with socket.socket() as reserve:
                reserve.bind(('127.0.0.1', 0))
                control_port = reserve.getsockname()[1]
            for name in ('server_cert', 'server_key', 'client_cert', 'client_key'):
                (root / f'control_{name}.pem').write_text(fixture[f'control_{name}'])
            write_config(upstream.port)
            text = config.read_text().replace(
                f'    control-interface: "{root}/control.sock"\n    control-use-cert: no',
                f'    control-interface: 127.0.0.1\n    control-port: {control_port}\n'
                '    control-use-cert: yes\n'
                f'    server-cert-file: "{root}/control_server_cert.pem"\n'
                f'    server-key-file: "{root}/control_server_key.pem"\n'
                f'    control-cert-file: "{root}/control_client_cert.pem"\n'
                f'    control-key-file: "{root}/control_client_key.pem"')
            config.write_text(text)
            subprocess.run([str(bindir / 'unbound-checkconf'), str(config)], check=True)
            process = subprocess.Popen([str(bindir / 'unbound'), '-d', '-c', str(config)])
            for _ in range(100):
                try:
                    query(port, 'host.home.test.')
                    break
                except OSError:
                    time.sleep(.05)
            assert 'is running' in control('status')
            assert 'ok' in control('reload')
            for _ in range(100):
                try:
                    assert 'is running' in control('status')
                    break
                except subprocess.CalledProcessError:
                    time.sleep(.05)
            else:
                raise AssertionError('TLS control did not recover after reload')
            control('fast_reload', '+v')
            assert 'is running' in control('status')
            context = ssl.create_default_context(cafile=str(root / 'control_server_cert.pem'))
            context.check_hostname = False
            try:
                with socket.create_connection(('127.0.0.1', control_port), timeout=5) as plain:
                    with context.wrap_socket(plain, server_hostname='control.test') as sock:
                        sock.sendall(b'UBCT1 status\n')
                        reply = sock.recv(4096)
                        assert b'is running' not in reply
            except (ssl.SSLError, ConnectionResetError):
                pass
            assert 'is running' in control('status')
            process.terminate()
            process.wait(timeout=10)
            print('PASS remote-control mutual TLS, reload/fast_reload, and missing certificate rejection')

            for text in ('view:\n name: private\n',
                         'server:\n access-control-view: 127.0.0.0/8 private\n',
                         'server:\n interface-view: 127.0.0.1 private\n',
                         'server:\n local-zone: home.test. noview\n',
                         'server:\n tls-service-key: key.pem\n',
                         'server:\n ssl-service-pem: cert.pem\n',
                         'server:\n tls-port: 853\n',
                         'server:\n tls-additional-port: 8853\n',
                         'server:\n tls-session-ticket-keys: ticket.key\n',
                         'server:\n tls-ciphers: DEFAULT\n',
                         'server:\n ssl-service-key: key.pem\n',
                         'server:\n tls-service-pem: cert.pem\n',
                         'server:\n ssl-port: 853\n',
                         'server:\n additional-ssl-port: 8853\n',
                         'server:\n tls-ciphersuites: TLS_AES_256_GCM_SHA384\n',
                         'server:\n pad-responses: yes\n',
                         'server:\n pad-responses-block-size: 468\n',
                         'server:\n quic-port: 853\n',
                         'server:\n quic-size: 8m\n',
                         'server:\n https-port: 443\n',
                         'server:\n http-endpoint: /dns-query\n',
                         'dnscrypt:\n dnscrypt-enable: yes\n',
                         'server:\n send-client-subnet: 192.0.2.0/24\n',
                         'server:\n module-config: "subnetcache validator iterator"\n',
                         'cachedb:\n backend: redis\n',
                         'server:\n cachedb-no-store: yes\n',
                         'server:\n module-config: "validator cachedb iterator"\n',
                         'server:\n dns64-prefix: 64:ff9b::/96\n',
                         'python:\n python-script: example.py\n',
                         'dynlib:\n dynlib-file: example.so\n',
                         'dnstap:\n dnstap-enable: yes\n',
                         'server:\n module-config: "dns64 validator iterator"\n'):
                config.write_text(text)
                result = subprocess.run([str(bindir / 'unbound-checkconf'), str(config)], capture_output=True)
                assert result.returncode != 0, text
            print('PASS rejection of removed directives and module names')
    finally:
        if process is not None:
            process.terminate()
            try:
                process.wait(timeout=10)
            except subprocess.TimeoutExpired:
                process.kill()
                process.wait()
        upstream.close()
        replacement.close()
        if encrypted is not None:
            encrypted.close()


if __name__ == '__main__':
    main()
