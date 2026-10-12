#!/usr/bin/env python3
"""TCP quota lifecycle regression; uses only local sockets and built utilities."""
from pathlib import Path
import socket
import struct
import subprocess
import sys
import tempfile
import time

from forwarding import recv_exact, wire_name

PACKET = struct.pack('!HHHHHH', 0x1234, 0x100, 1, 0, 0, 0) + wire_name('host.limit.test.') + struct.pack('!HH', 1, 1)


def answer(sock):
    sock.sendall(struct.pack('!H', len(PACKET)) + PACKET)
    packet = recv_exact(sock, struct.unpack('!H', recv_exact(sock, 2))[0])
    assert struct.unpack('!H', packet[2:4])[0] & 15 == 0
    assert socket.inet_aton('192.0.2.10') in packet


class Resolver:
    def __init__(self, build, root, ipv6=False, threads=1):
        self.build, self.root, self.ipv6 = build, root, ipv6
        self.threads = threads
        with socket.socket() as reserve:
            reserve.bind(('127.0.0.1', 0))
            self.port = reserve.getsockname()[1]
        self.config = root / 'unbound.conf'
        self.log = root / 'unbound.log'
        self.write()
        subprocess.run([str(build / 'unbound-checkconf'), str(self.config)], check=True, stdout=subprocess.DEVNULL)
        self.process = subprocess.Popen([str(build / 'unbound'), '-d', '-c', str(self.config)])
        with self.ready():
            pass

    def write(self, limit=1, idle=3000):
        limits = '' if limit is None else ''.join(
            f'    tcp-connection-limit: {address} {limit}\n'
            for address in ('127.0.0.5/32', '127.0.0.6/32') + (('::1/128',) if self.ipv6 else ()))
        self.config.write_text(f'''server:
    interface: 127.0.0.1
''' + ('    interface: ::1\n' if self.ipv6 else '') + f'''    port: {self.port}
    do-ip6: {'yes' if self.ipv6 else 'no'}
    num-threads: {self.threads}
    outgoing-range: 32
    do-not-query-localhost: no
    username: ""
    chroot: ""
    directory: "{self.root}"
    pidfile: "{self.root}/unbound.pid"
    logfile: "{self.log}"
    use-syslog: no
    tcp-idle-timeout: {idle}
    incoming-num-tcp: 32
    local-zone: "limit.test." static
    local-data: "host.limit.test. 60 IN A 192.0.2.10"
{limits}remote-control:
    control-enable: yes
    control-interface: "{self.root}/control.sock"
    control-use-cert: no
forward-zone:
    name: "."
    forward-first: no
    forward-addr: 127.0.0.1@9
''')

    def control(self, command):
        result = subprocess.check_output([str(self.build / 'unbound-control'), '-c', str(self.config), *command.split()], text=True)
        assert 'ok' in result, result

    def connect(self, source='127.0.0.5'):
        sock = socket.socket(socket.AF_INET6 if ':' in source else socket.AF_INET, socket.SOCK_STREAM)
        try:
            sock.settimeout(.5)
            sock.bind((source, 0))
            sock.connect(('::1' if ':' in source else '127.0.0.1', self.port))
            answer(sock)  # Ensure accept/accounting completed before the next operation.
            return sock
        except BaseException:
            sock.close()
            raise

    def ready(self, source='127.0.0.9'):
        deadline = time.monotonic() + 5
        while True:
            if self.process.poll() is not None:
                raise AssertionError(f'resolver exited: {self.log.read_text()}')
            try:
                return self.connect(source)
            except (OSError, EOFError):
                if time.monotonic() >= deadline:
                    raise
                time.sleep(.03)

    def released(self, source='127.0.0.5'):
        # FIN/RST cleanup is asynchronous; permit a bounded cleanup interval.
        with self.ready(source):
            pass

    def blocked(self, source='127.0.0.5'):
        try:
            sock = self.connect(source)
        except (OSError, EOFError):
            return
        sock.close()
        raise AssertionError(f'quota not enforced for {source}')

    def reload(self, command, limit=1, idle=3000):
        self.write(limit, idle)
        self.control(command)
        with self.ready():
            pass

    def stop(self):
        self.process.terminate()
        self.process.wait(timeout=10)
        assert self.process.returncode == 0, self.log.read_text()


def run(build, root, threads=1):
    resolver = Resolver(build, root, threads=threads)
    try:
        for _ in range(12):
            with resolver.ready('127.0.0.5'):
                pass
            resolver.released()
        print(f'PASS sequential TCP query/close/quota reuse ({threads} worker(s))')

        with resolver.ready('127.0.0.5') as first:
            resolver.blocked()
            with resolver.connect('127.0.0.6'):
                resolver.blocked('127.0.0.6')
            for _ in range(3):
                with resolver.connect('127.0.0.9'):
                    pass
            answer(first)
        resolver.released()
        resolver.released('127.0.0.6')
        print('PASS simultaneous cap, separate limited sources, and unrestricted clients')

        for _ in range(5):
            reset = resolver.ready('127.0.0.5')
            reset.setsockopt(socket.SOL_SOCKET, socket.SO_LINGER, struct.pack('ii', 1, 0))
            reset.close()
            resolver.released()
        # A partial DNS frame followed by a reset exercises read-side cleanup.
        partial = resolver.ready('127.0.0.5')
        partial.sendall(b'\x00')
        partial.setsockopt(socket.SOL_SOCKET, socket.SO_LINGER, struct.pack('ii', 1, 0))
        partial.close()
        resolver.released()
        resolver.reload('reload', idle=200)
        with resolver.ready('127.0.0.5') as idle:
            resolver.blocked()
            idle.settimeout(3)
            assert idle.recv(1) == b''
        resolver.released()
        print('PASS RST, partial-frame read cleanup, and server idle timeout quota release')

        for command in ('reload', 'fast_reload', 'fast_reload +p'):
            resolver.reload('reload')
            with resolver.ready('127.0.0.5') as held:
                resolver.reload(command)
                if command.startswith('fast_reload'):
                    answer(held)
                    resolver.blocked()
                else:
                    held.settimeout(1)
                    assert held.recv(1) == b''
            resolver.released()
            with resolver.ready('127.0.0.5') as held:
                resolver.reload(command, limit=2)
                if command == 'reload':
                    held.settimeout(1)
                    assert held.recv(1) == b''
                with resolver.ready('127.0.0.5') as second:
                    if command == 'reload':
                        with resolver.connect('127.0.0.5'):
                            resolver.blocked()
                    if command.startswith('fast_reload'):
                        answer(held)
                        resolver.blocked()
                        resolver.reload(command, limit=1)
                        answer(held)
                        answer(second)
                        resolver.blocked()
                if command.startswith('fast_reload'):
                    resolver.blocked()  # Limit was lowered to one; held still counts.
            resolver.released()
            if command.startswith('fast_reload'):
                with resolver.ready('127.0.0.5') as held:
                    resolver.reload(command, limit=None)
                    answer(held)
                    with resolver.connect('127.0.0.5') as second:
                        resolver.reload(command, limit=1)
                        answer(held)
                        answer(second)
                        resolver.blocked()
                    resolver.blocked()
                resolver.released()
            print(f'PASS {command}: active connections, unchanged/modified limits and cleanup')
        # Leave counted connections open during destruction to exercise shutdown.
        active = resolver.ready('127.0.0.5')
        resolver.stop()
        active.close()
        print('PASS shutdown with an active counted connection')
    finally:
        if resolver.process.poll() is None:
            resolver.stop()


def main():
    build = Path(sys.argv[1]).resolve()
    with tempfile.TemporaryDirectory(prefix='unbound-tcp-limits-') as directory:
        root = Path(directory)
        for threads in (1, 2):
            case = root / str(threads)
            case.mkdir()
            run(build, case, threads)
        try:
            with socket.socket(socket.AF_INET6) as probe:
                probe.bind(('::1', 0))
        except OSError:
            print('SKIP IPv6: loopback unavailable on this host')
        else:
            case = root / 'ipv6'
            case.mkdir()
            resolver = Resolver(build, case, ipv6=True)
            try:
                for _ in range(8):
                    with resolver.ready('::1'):
                        resolver.blocked('::1')
                    resolver.released('::1')
                with resolver.ready('::1') as held:
                    resolver.reload('fast_reload')
                    answer(held)
                    resolver.blocked('::1')
                resolver.released('::1')
                print('PASS IPv6 sequential/simultaneous quotas and active fast reload')
            finally:
                resolver.stop()


if __name__ == '__main__':
    main()
