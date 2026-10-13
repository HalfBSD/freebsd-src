#!/usr/bin/env python3
"""Single-file layout migration; optional runtime verification with BUILD argument."""
import base64
import json
import os
from pathlib import Path
import socket
import subprocess
import sys
import tempfile
import time

from forwarding import Upstream, query

repo = Path(__file__).resolve().parents[3]
script = repo / 'usr.sbin/unbound/setup/local-unbound-setup.sh'


def prepare(root):
    for directory in ('etc', 'var/unbound/conf.d', 'var/backups', 'var/run'):
        (root / directory).mkdir(parents=True)
    (root / 'etc/resolv.conf').write_text('nameserver 192.0.2.53\n')


def setup(root, success=True, *options):
    result = subprocess.run(['sh', str(script), '-n', *options],
                            env=dict(os.environ, DESTDIR=str(root)),
                            capture_output=True, text=True, timeout=10)
    assert (result.returncode == 0) == success, result.stdout + result.stderr
    return result


# distribution used to recreate this symlink, aliasing the two config paths.
# Absolute links in a staged filesystem must not touch the active host's /var.
assert '../var/unbound' not in (repo / 'etc/Makefile').read_text()
for target in ('../var/unbound', '/var/unbound'):
    with tempfile.TemporaryDirectory(prefix='unbound-symlink-') as directory:
        root = Path(directory)
        prepare(root)
        old = root / 'var/unbound'
        link = root / 'etc/unbound'
        link.symlink_to(target)
        main = old / 'unbound.conf'
        records = '    local-data: "preserved.home.test. IN A 192.0.2.10"\n'
        original = ('server:\n    chroot: ""\n    directory: /etc/unbound\n'
                    '    auto-trust-anchor-file: /etc/unbound/root.key\n'
                    '    tls-cert-bundle: /etc/unbound/site.pem\n' + records +
                    'include: "/etc/unbound/control.conf"\n'
                    'include: "/etc/unbound/conf.d/*.conf"\n'
                    'include: "/etc/unbound/manual.conf"\n')
        main.write_text(original)
        setup(root, False, '-c', '/etc/unbound/site.conf')
        assert link.is_symlink() and main.read_text() == original
        # Validate before removing the link: missing fragments leave it intact.
        setup(root, False)
        assert link.is_symlink() and os.readlink(link) == target
        assert main.read_text() == original
        (old / 'control.conf').write_text('remote-control:\n    control-enable: yes\n')
        (old / 'conf.d/local.conf').write_text('server:\n    local-data: "extra.home.test. IN A 192.0.2.11"\n')
        manual = old / 'manual.conf'
        manual.write_text('server:\n    local-data-ptr: "192.0.2.10 preserved.home.test."\n')
        anchor = old / 'root.key'
        anchor.write_text('TEST-ONLY anchor state\n')
        certificate = old / 'site.pem'
        certificate.write_text('TEST-ONLY certificate data\n')
        setup(root)
        assert link.is_dir() and not link.is_symlink()
        config = link / 'unbound.conf'
        result = config.read_text()
        assert records in result and 'control-enable: yes' in result
        assert 'extra.home.test.' in result and not (old / 'conf.d').exists()
        assert 'include: "/var/unbound/manual.conf"' in result
        assert 'directory: "/var/unbound"' in result
        assert 'auto-trust-anchor-file: "/var/unbound/root.key"' in result
        assert 'tls-cert-bundle: "/var/unbound/site.pem"' in result
        assert not main.exists()
        assert manual.read_text().startswith('server:')
        assert anchor.read_text() == 'TEST-ONLY anchor state\n'
        assert certificate.read_text() == 'TEST-ONLY certificate data\n'
        backups = root / 'var/backups'
        assert any(p.read_text() == original for p in backups.glob('unbound.conf.*'))
        assert any(p.is_symlink() and os.readlink(p) == target for p in backups.glob('unbound-link.*'))
        setup(root)
        assert config.read_text() == result and not main.exists()
print('PASS relative/absolute legacy directory symlinks migrate safely and idempotently without losing manual files or anchor state')


with tempfile.TemporaryDirectory(prefix='unbound-layout-') as directory:
    root = Path(directory)
    old = root / 'var/unbound'
    prepare(root)
    main = old / 'unbound.conf'
    records = ('    local-zone: "home.test." static\n'
               '    local-data: "host.home.test. 60 IN A 192.0.2.10"\n'
               '    local-data: "alias.home.test. 60 IN CNAME host.home.test."\n'
               '    local-data-ptr: "192.0.2.10 host.home.test."\n'
               '    local-data: "text.home.test. IN TXT include:literal"\n')
    main.write_text('server:\n    username: ""\n    directory: /var/unbound\n'
                    '    chroot: "/var/unbound"\n'
                    '    auto-trust-anchor-file: /var/unbound/root.key\n'
                    '    pidfile: /var/unbound/unbound.pid\n' + records +
                    'include: "/var/unbound/control.conf"\n'
                    'include: "/var/unbound/lan-zones.conf"\n'
                    'include: "/var/unbound/conf.d/*.conf"\n')
    control = old / 'control.conf'
    control.write_text('remote-control:\n    control-enable: yes\n'
                       '    control-interface: /var/run/local_unbound.ctl\n'
                       '    control-use-cert: no\n')
    lan = old / 'lan-zones.conf'
    lan.write_text('server:\n    unblock-lan-zones: yes\n    insecure-lan-zones: yes\n'
                   '    local-data: "lan.home.test. IN A 192.0.2.11"\n')
    extra = old / 'conf.d/10-local.conf'
    extra.write_text('server:\n    local-data: "extra.home.test. IN A 192.0.2.12"\n')
    nested = old / 'conf.d/20-more.conf'
    nested.write_text('server:\n    local-data-ptr: "192.0.2.12 extra.home.test."\n')
    anchor = old / 'root.key'
    anchor.write_text('TEST-ONLY existing mutable anchor state\n')
    original_anchor = anchor.read_bytes()
    originals = {path.name: path.read_bytes() for path in (main, control, lan, extra, nested)}
    setup(root)
    config = root / 'etc/unbound/unbound.conf'
    contents = config.read_text()
    assert records in contents and 'include:literal' in contents
    assert 'control-enable: yes' in contents and 'unblock-lan-zones: yes' in contents
    assert 'lan.home.test.' in contents and 'extra.home.test.' in contents
    assert 'chroot: ""' in contents and 'pidfile: /var/run/local_unbound.pid' in contents
    assert 'include: "/var/unbound' not in contents
    assert not main.exists() and not control.exists() and not lan.exists()
    assert not (old / 'conf.d').exists()
    for name, data in originals.items():
        assert any(path.read_bytes() == data for path in (root / 'var/backups').glob(name + '.*'))
    assert anchor.read_bytes() == original_anchor
    assert config.stat().st_mode & 0o022 == 0  # No group/other write permission.
    snapshot = config.read_bytes()
    setup(root)
    setup(root)
    assert config.read_bytes() == snapshot and anchor.read_bytes() == original_anchor
    print('PASS legacy main/control/LAN/conf.d migration, local records, unique backups and repeated setup')

    # Never discard either side of a conflicting migration or custom chroot.
    main.write_text('server:\n    local-data: "other.test. IN A 192.0.2.99"\n')
    conflict = main.read_bytes()
    result = setup(root, False)
    assert 'Both ' in result.stderr
    assert main.read_bytes() == conflict and config.read_bytes() == snapshot
    main.unlink()
    config.write_text('server:\n    chroot: "/srv/private-jail"\n' + records)
    custom = config.read_bytes()
    result = setup(root, False)
    assert 'custom chroot' in result.stderr and config.read_bytes() == custom
    config.write_bytes(snapshot)
    print('PASS ambiguous migration and explicit custom chroot fail without replacing administrator configuration')

    # A custom TLS-authenticated control section is inlined without weakening it.
    control.write_text('remote-control:\n    control-enable: yes\n    control-use-cert: yes\n'
                       '    server-key-file: /etc/keys/server.key\n'
                       '    server-cert-file: /etc/keys/server.pem\n'
                       '    control-key-file: /etc/keys/client.key\n'
                       '    control-cert-file: /etc/keys/client.pem\n')
    tls_settings = control.read_text()
    config.write_text('server:\n' + records + 'include: "/var/unbound/control.conf"\n')
    setup(root)
    assert tls_settings in config.read_text() and 'control-use-cert: no' not in config.read_text()
    assert not control.exists()
    config.write_bytes(snapshot)
    # Absolute paths inside the former jail must still name the same files.
    config.write_text('server:\n    chroot: /var/unbound\n    directory: /\n'
                      '    auto-trust-anchor-file: /root.key\n'
                      'remote-control:\n    control-use-cert: yes\n'
                      '    server-key-file: /server.key\n'
                      '    control-cert-file: /client.pem\n')
    setup(root)
    rebased = config.read_text()
    assert 'directory: "/var/unbound/"' in rebased
    assert 'auto-trust-anchor-file: "/var/unbound/root.key"' in rebased
    assert 'server-key-file: "/var/unbound/server.key"' in rebased
    assert 'control-cert-file: "/var/unbound/client.pem"' in rebased
    assert 'control-use-cert: yes' in rebased
    config.write_bytes(snapshot)
    print('PASS TLS control settings and formerly jail-relative file paths retain their security/physical meaning')

    # Read failures/cycles must not retire fragments or commit partial output.
    config.write_text('server:\n' + records + 'include: "/var/unbound/lan-zones.conf"\n')
    missing = config.read_bytes()
    setup(root, False)
    assert config.read_bytes() == missing
    lan.write_text('include: "/var/unbound/lan-zones.conf"\n')
    cycle = lan.read_bytes()
    setup(root, False)
    assert config.read_bytes() == missing and lan.read_bytes() == cycle
    lan.unlink()
    config.write_bytes(snapshot)
    # Unknown manual includes are preserved outside the former jail, but are
    # not silently relocated with unreviewed jail-relative paths.
    custom_include = 'include: "/etc/unbound/manual.conf"\n'
    config.write_text('server:\n' + records + custom_include)
    setup(root)
    assert custom_include in config.read_text()
    config.write_text('server:\n    chroot: /var/unbound\n' + custom_include)
    unreviewed = config.read_bytes()
    setup(root, False)
    assert config.read_bytes() == unreviewed
    config.write_bytes(snapshot)
    for option in ('-o', '-C', '-f'):
        setup(root, False, option, '/unused')
    print('PASS missing/cyclic fragments preserve originals; removed setup options are rejected')

    if len(sys.argv) > 1:
        build = Path(sys.argv[1]).resolve()
        fixture = json.loads((repo / 'usr.sbin/unbound/tests/dnssec-fixture.json').read_text())
        upstream = Upstream(fixture)
        process = None
        try:
            key = base64.b64decode(fixture['dnskey'])
            trust = base64.b64encode(key[4:]).decode()
            anchor.write_text(f'signed.test. 60 IN DNSKEY 257 3 15 {trust}\n')
            with socket.socket() as reserve:
                reserve.bind(('127.0.0.1', 0))
                port = reserve.getsockname()[1]
            # Root test paths in this isolated filesystem; no duplicated product config.
            runtime = snapshot.decode().replace('/var/unbound', str(old))
            runtime = runtime.replace('/var/run', str(root / 'var/run'))
            runtime += f'''\nserver:
    interface: 127.0.0.1
    port: {port}
    do-not-query-localhost: no
    local-zone: "signed.test." transparent
    local-zone: "forward.test." transparent
    use-syslog: no
    logfile: "{root}/unbound.log"
forward-zone:
    name: "."
    forward-first: no
    forward-addr: 127.0.0.1@{upstream.port}
'''
            config.write_text(runtime)
            subprocess.run([str(build / 'unbound-checkconf'), str(config)], check=True)
            process = subprocess.Popen([str(build / 'unbound'), '-d', '-c', str(config)])
            for _ in range(100):
                assert process.poll() is None, (root / 'unbound.log').read_text()
                try:
                    query(port, 'host.home.test.')
                    break
                except OSError:
                    time.sleep(.05)
            else:
                raise AssertionError('resolver did not start')
            def control(*args):
                return subprocess.check_output([str(build / 'unbound-control'), '-c', str(config), *args], text=True)
            for tcp in (False, True):
                assert (1, socket.inet_aton('192.0.2.10')) in query(port, 'host.home.test.', tcp=tcp)[1]
                assert any(t == 5 for t, _ in query(port, 'alias.home.test.', tcp=tcp)[1])
                assert any(t == 12 for t, _ in query(port, '10.2.0.192.in-addr.arpa.', 12, tcp=tcp)[1])
                assert (1, socket.inet_aton('192.0.2.12')) in query(port, 'extra.home.test.', tcp=tcp)[1]
            flags, _ = query(port, 'good.signed.test.')
            assert flags & 0x20 and flags & 15 == 0
            assert query(port, 'bad.signed.test.')[0] & 15 == 2
            assert query(port, 'answer.forward.test.')[0] & 15 == 0
            count = sum(upstream.count.values())
            assert query(port, 'answer.forward.test.')[0] & 15 == 0
            assert sum(upstream.count.values()) == count
            # Observe an actual RFC5011 DNSKEY probe and atomic state-file update.
            for _ in range(100):
                if ';;last_success:' in anchor.read_text():
                    break
                time.sleep(.05)
            else:
                raise AssertionError('no automatic trust-anchor state update observed')
            assert sum(v for (name, rtype, tcp), v in upstream.count.items()
                       if name == 'signed.test.' and rtype == 48) > 0
            assert int(anchor.read_text().split(';;last_success: ')[1].split()[0]) > 0
            active_config, active_anchor = config.read_bytes(), anchor.read_bytes()
            setup(root)
            assert config.read_bytes() == active_config and anchor.read_bytes() == active_anchor
            assert (root / 'var/run/local_unbound.ctl').stat().st_mode & 0o007 == 0
            print('PASS consolidated config loads, local A/PTR/CNAME, caching, DNSSEC and actual trust-anchor state update')
            previous = '192.0.2.10'
            for command, address in (('reload', '192.0.2.13'), ('fast_reload', '192.0.2.14')):
                config.write_text(config.read_text().replace(previous + '"', address + '"'))
                previous = address
                assert 'ok' in control(command)
                for _ in range(100):
                    try:
                        answers = query(port, 'host.home.test.')[1]
                        if (1, socket.inet_aton(address)) in answers:
                            break
                    except OSError:
                        pass
                    time.sleep(.03)
                else:
                    raise AssertionError('reload did not pick up local data')
                assert 'is running' in control('status')
                assert query(port, 'good.signed.test.')[0] & 0x20
            print('PASS ordinary/fast reload from /etc layout and filesystem-authenticated Unix control')
        finally:
            if process:
                process.terminate()
                assert process.wait(timeout=10) == 0
            for server in (upstream.tcp, upstream.udp):
                server.shutdown()
                server.server_close()
