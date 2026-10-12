#!/usr/bin/env python3
"""Check setup and upgrade migration in an isolated DESTDIR."""
import os
from pathlib import Path
import subprocess
import shlex
import sys
import tempfile


script = Path(__file__).resolve().parents[1] / 'setup/local-unbound-setup.sh'
with tempfile.TemporaryDirectory(prefix='unbound-setup-') as directory:
    root = Path(directory)
    for path in ('etc', 'var/unbound/conf.d', 'var/backups'):
        (root / path).mkdir(parents=True, exist_ok=True)
    resolver = root / 'etc/resolv.conf'
    resolver.write_text('search home.test\nnameserver 192.0.2.53\n')
    environment = dict(os.environ, DESTDIR=str(root))
    subprocess.run(['sh', str(script), '-n'], env=environment, check=True)
    config = root / 'etc/unbound/unbound.conf'
    assert 'forward.conf' not in config.read_text()
    assert not (root / 'var/unbound/forward.conf').exists()
    assert 'control.conf' not in config.read_text()
    assert 'control-enable: yes' in config.read_text()
    assert 'unblock-lan-zones: yes' in config.read_text()
    assert 'chroot: ""' in config.read_text()
    assert not (root / 'var/unbound/control.conf').exists()
    assert not (root / 'var/unbound/lan-zones.conf').exists()
    assert 'search home.test' in resolver.read_text()
    assert '\nnameserver 127.0.0.1\n' in resolver.read_text()
    assert '\nnameserver 192.0.2.53\n' not in resolver.read_text()
    assert 'libc="NO"' in (root / 'etc/resolvconf.conf').read_text()
    assert 'unbound_conf' not in (root / 'etc/resolvconf.conf').read_text()
    subprocess.run(['sh', str(script), '-n', '-t'], env=environment, check=True)
    assert 'tls-upstream: yes' in config.read_text()
    assert 'tls-cert-bundle: /etc/ssl/cert.pem' in config.read_text()
    print('PASS fresh setup uses built-in DNS and ignores DHCP DNS')

    legacy = root / 'var/unbound/forward.conf'
    legacy.write_text('forward-zone:\n name: .\n forward-tls-upstream: yes\n'
                      ' forward-addr: 192.0.2.53@853#old.test\n')
    policy = ('    access-control: 192.0.2.0/24 allow\n'
              '    interface-action: 0.0.0.0 allow\n'
              '    local-zone-override: home.test. 192.0.2.0/24 refuse\n'
              '    define-tag: "trusted\nold"\n'
              '    access-control-tag-data: 192.0.2.0/24 trusted "TXT\nold"\n')
    records = ('    local-data: "custom.home.test. IN A 192.0.2.10"\n'
               '    local-data: "meta.home.test. IN TXT access-control:literal"\n'
               '    local-data: "multi.home.test. IN TXT first\naccess-control:literal"\n'
               '    tls-cert-bundle: /etc/ssl/cert.pem\n')
    custom = 'server:\n' + policy + records
    config.write_text(custom + 'include: "/var/unbound/forward.conf" # legacy\n')
    subprocess.run(['sh', str(script), '-n'], env=environment, check=True)
    result = config.read_text()
    assert 'server:\n' + records in result and 'forward.conf' not in result
    assert 'access-control: 192.0.2.0/24' not in result
    assert 'interface-action:' not in result and 'local-zone-override:' not in result
    assert 'define-tag:' not in result and 'access-control-tag-data:' not in result
    assert '\nold' not in result
    assert 'tls-upstream: yes' in result
    assert not legacy.exists()
    assert any('forward-addr: 192.0.2.53' in path.read_text()
               for path in (root / 'var/backups').glob('forward.conf.*'))
    subprocess.run(['sh', str(script), '-n'], env=environment, check=True)
    assert config.read_text() == result
    print('PASS upgrade removes client policy, preserves quoted local records/TLS, and is idempotent')

    # Exercise the actual service setup function with mocked rc.subr actions.
    # This checks restart policy without running a FreeBSD service on Linux.
    rc = script.parents[3] / 'libexec/rc/rc.d/local_unbound'
    content = rc.read_text()
    start = content.index('local_unbound_setup()\n{')
    end = content.index('\n}\n', start) + 3
    helper = root / 'fake-unbound-setup'
    helper.write_text('#!/bin/sh\nexit "${HELPER_STATUS:-0}"\n')
    helper.chmod(0o755)
    trace = root / 'trace'
    harness = root / 'rc-test.sh'
    harness.write_text(f'''command={shlex.quote(str(root / 'fake-unbound'))}
local_unbound_workdir=/var/unbound
local_unbound_config=/etc/unbound/unbound.conf
local_unbound_anchor=/var/unbound/root.key
checkyesno() {{ return 1; }}
local_unbound_configtest() {{ echo configtest >> "$TRACE"; return "${{CHECK_STATUS:-0}}"; }}
run_rc_command() {{ echo "$1" >> "$TRACE"; }}
''' + content[start:end] + '\nlocal_unbound_setup\n')
    for pid, helper_status, check_status, expected in [
            ('', '0', '0', []), ('1234', '0', '0', ['configtest', 'restart']),
            ('1234', '1', '0', []), ('1234', '0', '1', ['configtest'])]:
        trace.write_text('')
        result = subprocess.run(['sh', str(harness)], env=dict(environment, rc_pid=pid,
                                HELPER_STATUS=helper_status, CHECK_STATUS=check_status,
                                TRACE=str(trace)), capture_output=True)
        assert trace.read_text().splitlines() == expected
        assert (result.returncode == 0) == (helper_status == '0' and check_status == '0')
    print('PASS service setup validates before replacing a running daemon; failures do not restart')

# Keep the installworld upgrade case in the usual setup regression entry point.
subprocess.run([sys.executable, str(script.parents[1] / 'tests/installworld.py')], check=True)

# Consolidated layout migration is part of the standard setup suite.
subprocess.run([sys.executable, str(script.parents[1] / 'tests/layout.py'), *sys.argv[1:]], check=True)
