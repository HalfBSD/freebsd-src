#!/usr/bin/env python3
"""Offline tests of the compiled-in defaults; requires a C compiler and LD_PRELOAD.

The interposer rejects external destinations and advances only the resolver's
wall clock, so recovery/backoff tests need neither Internet access nor sleeps.
"""
import base64
import json
import os
from pathlib import Path
import socket
import ssl
import subprocess
import sys
import tempfile
import time

from forwarding import Upstream, query


def main():
    binaries = Path(sys.argv[1]).resolve()
    fixture = json.loads(Path(__file__).with_name('dnssec-fixture.json').read_text())
    servers = [Upstream(fixture) for _ in range(6)]
    override = Upstream(fixture)
    trust = base64.b64encode(base64.b64decode(fixture["dnskey"])[4:]).decode()
    process = None
    try:
        with tempfile.TemporaryDirectory(prefix='unbound-providers-') as directory:
            root = Path(directory)
            helper = root / 'redirect.so'
            subprocess.run([os.environ.get('CC', 'cc'), '-shared', '-fPIC', '-Wall',
                            '-Wextra', '-Werror', '-o', str(helper),
                            str(Path(__file__).with_name('provider-redirect.c'))] +
                           ([] if sys.platform.startswith('freebsd') else ['-ldl']), check=True)
            mapping = root / 'map'
            offset = 0

            def map_servers(available):
                temporary = root / 'map.new'
                temporary.write_text(' '.join(str(s.port if i in available else 0)
                                              for i, s in enumerate(servers)) + f' {offset}\n')
                temporary.replace(mapping)

            def counts():
                return [sum(s.count.values()) for s in servers]

            with socket.socket() as reserve:
                reserve.bind(('127.0.0.1', 0))
                port = reserve.getsockname()[1]
            config = root / 'unbound.conf'
            config.write_text(f'''server:
    interface: 127.0.0.1@{port}
    do-ip6: no
    num-threads: 2
    outgoing-range: 128
    num-queries-per-thread: 64
    do-not-query-localhost: no
    username: ""
    chroot: ""
    directory: "{root}"
    use-syslog: no
    logfile: "{root}/log"
    pidfile: "{root}/pid"
    module-config: "validator iterator"
    infra-cache-min-rtt: 50
    infra-cache-max-rtt: 200
    serve-expired: yes
    serve-expired-client-timeout: 0
    local-zone: "forward.test." transparent
    local-zone: "signed.test." transparent
    local-data: "local.home.test. 60 IN A 192.0.2.10"
    trust-anchor: "signed.test. DNSKEY 257 3 15 {trust}"
remote-control:
    control-enable: yes
    control-interface: "{root}/control"
    control-use-cert: no
''')
            map_servers(set(range(6)))
            environment = dict(os.environ, LD_PRELOAD=str(helper), UNBOUND_TEST_MAP=str(mapping))
            subprocess.run([str(binaries / 'unbound-checkconf'), str(config)], check=True)
            process = subprocess.Popen([str(binaries / 'unbound'), '-d', '-c', str(config)], env=environment)

            def control(*args):
                return subprocess.check_output([str(binaries / 'unbound-control'), '-c',
                                                str(config), *args], text=True)

            for _ in range(100):
                if process.poll() is not None:
                    raise RuntimeError((root / 'log').read_text())
                try:
                    query(port, 'local.home.test.')
                    break
                except OSError:
                    time.sleep(.05)
            else:
                raise AssertionError('startup failed')

            def resolve(name, provider, tcp=False):
                before = counts()
                flags, _ = query(port, name + '.forward.test.', tcp=tcp)
                assert flags & 15 == 0, (name, hex(flags), (root / 'log').read_text())
                after = counts()
                assert sum(after[provider*2:provider*2+2]) > sum(before[provider*2:provider*2+2])
                assert after[(provider+1)*2:] == before[(provider+1)*2:], (before, after)

            resolve('primary', 0)
            before = counts()
            query(port, 'primary.forward.test.', tcp=True)
            assert counts() == before
            map_servers({1, 2, 3, 4, 5})
            resolve('one-primary', 0, tcp=True)
            assert counts()[2:] == [0, 0, 0, 0]
            print('PASS default startup, Cloudflare priority, healthy peer, UDP/TCP and caching', flush=True)

            map_servers({2, 3, 4, 5})
            resolve('secondary', 1)
            map_servers({4, 5})
            resolve('tertiary', 2)
            print('PASS transport failure moves from Cloudflare to Google to OpenDNS', flush=True)

            map_servers(set(range(6)))
            resolve('cooldown', 2)
            offset += 31
            map_servers(set(range(6)))
            # Recovery must run while idle, rather than waiting for a cache miss.
            before = counts()
            for _ in range(120):
                if sum(counts()[:2]) > sum(before[:2]):
                    break
                time.sleep(.05)
            else:
                raise AssertionError('higher-priority recovery timer did not probe while idle')
            time.sleep(.1)  # Let the received probe complete its transport callback.
            resolve('recovery', 0)
            resolve('stable', 0)
            print('PASS bounded retry and higher-priority recovery', flush=True)

            before = counts()
            assert query(port, 'negative.forward.test.')[0] & 15 == 3
            assert query(port, 'servfail.forward.test.')[0] & 15 == 2
            assert query(port, 'good.signed.test.')[0] & 0x20
            assert query(port, 'bad.signed.test.')[0] & 15 == 2
            resolve('after-validation', 0)
            assert counts()[2:] == before[2:]
            print('PASS NXDOMAIN and DNSSEC failure do not change provider', flush=True)

            map_servers(set())
            offset += 61
            map_servers(set())
            assert query(port, 'outage.forward.test.')[0] & 15 == 2
            assert query(port, 'local.home.test.')[0] & 15 == 0
            assert query(port, 'primary.forward.test.')[0] & 15 == 0
            print('PASS complete outage returns SERVFAIL and preserves local DNS', flush=True)

            for arguments in [('forward', 'off'), ('forward_remove', '.')]:
                result = subprocess.run([str(binaries / 'unbound-control'), '-c', str(config),
                                         *arguments], capture_output=True)
                assert result.returncode != 0
            unsafe = root / 'unsafe.conf'
            unsafe.write_text(config.read_text() +
                              '\nforward-zone:\n    name: .\n    forward-first: yes\n'
                              '    forward-addr: 1.1.1.1\n')
            assert subprocess.run([str(binaries / 'unbound-checkconf'), str(unsafe)],
                                  capture_output=True).returncode != 0
            unsafe.write_text(config.read_text() + '\nserver:\n    tls-upstream: yes\n')
            assert subprocess.run([str(binaries / 'unbound-checkconf'), str(unsafe)],
                                  capture_output=True).returncode != 0
            assert 'ok' in control('forward', f'127.0.0.1@{override.port}')
            assert query(port, 'override.forward.test.')[0] & 15 == 0
            assert sum(override.count.values()) > 0
            assert 'ok' in control('reload')
            time.sleep(.2)
            offset += 301
            map_servers(set(range(6)))
            resolve('reloaded', 0)
            assert 'total.num.queries=' in control('stats_noreset')
            assert 'ok' in control('fast_reload')
            time.sleep(.2)
            resolve('fast-reloaded', 0)
            print('PASS runtime override, recursive-fallback rejection, reload and statistics', flush=True)

            # Silent UDP loss exercises real retries/timeouts, not just socket errors.
            servers[0].drop = servers[1].drop = True
            resolve('udp-timeout', 1)
            servers[0].drop = servers[1].drop = False
            print('PASS silent UDP timeouts and shared health across two workers', flush=True)

            # Exercise TLS on the built-in path, including all provider names.
            process.terminate()
            process.wait(timeout=5)
            process = None
            for server in servers:
                server.close()
            servers = []
            certificate, key = root / 'tls.pem', root / 'tls.key'
            subprocess.run(['openssl', 'req', '-x509', '-newkey', 'rsa:2048', '-nodes',
                            '-days', '2', '-subj', '/CN=one.one.one.one', '-addext',
                            'subjectAltName=DNS:one.one.one.one,DNS:dns.google,DNS:dns.opendns.com',
                            '-keyout', str(key), '-out', str(certificate)],
                           check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
            context.load_cert_chain(certificate, key)
            servers = [Upstream(fixture, tls=context) for _ in range(6)]
            config.write_text(config.read_text() +
                              f'\nserver:\n    tls-upstream: yes\n    tls-cert-bundle: "{certificate}"\n')
            offset = 0
            map_servers(set(range(6)))
            subprocess.run([str(binaries / 'unbound-checkconf'), str(config)], check=True)
            process = subprocess.Popen([str(binaries / 'unbound'), '-d', '-c', str(config)], env=environment)
            for _ in range(100):
                try:
                    query(port, 'local.home.test.')
                    break
                except OSError:
                    time.sleep(.05)
            resolve('tls-primary', 0)
            for server in servers[:2]:
                for connection in server.connections:
                    connection.shutdown(socket.SHUT_RDWR)
                    connection.close()
            map_servers({2, 3, 4, 5})
            resolve('tls-secondary', 1)
            for server in servers[2:4]:
                for connection in server.connections:
                    connection.shutdown(socket.SHUT_RDWR)
                    connection.close()
            map_servers({4, 5})
            resolve('tls-tertiary', 2)
            assert all(not any(count for (_, _, tcp), count in s.count.items() if not tcp)
                       for s in servers)
            print('PASS authenticated built-in TLS forwarding and three-provider failover, with no plaintext downgrade', flush=True)

            process.terminate()
            process.wait(timeout=5)
            process = None
            wrong_ca = root / 'wrong-ca.pem'
            wrong_ca.write_text(fixture['tls_ca'])
            config.write_text(config.read_text().replace(str(certificate), str(wrong_ca)))
            map_servers(set(range(6)))
            before = counts()
            process = subprocess.Popen([str(binaries / 'unbound'), '-d', '-c', str(config)], env=environment)
            for _ in range(100):
                try:
                    query(port, 'local.home.test.')
                    break
                except OSError:
                    time.sleep(.05)
            assert query(port, 'tls-rejected.forward.test.')[0] & 15 == 2
            assert counts() == before
            print('PASS built-in TLS rejects an untrusted certificate without sending DNS in plaintext', flush=True)
    finally:
        if process:
            process.terminate()
            process.wait(timeout=5)
        for server in servers + [override]:
            server.close()


if __name__ == '__main__':
    main()
