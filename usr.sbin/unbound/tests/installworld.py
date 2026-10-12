#!/usr/bin/env python3
"""Test installworld's hook against an obsolete installed rc script.

Runs the real setup utility and source rc script in a temporary filesystem;
rc.subr dispatch, service/privilege switching, and anchor bootstrap are mocked.
No native installworld, daemon restart, or live trust-anchor update is performed.
"""
import os
from pathlib import Path
import re
import shlex
import subprocess
import tempfile

repo = Path(__file__).resolve().parents[3]
source_rc = (repo / 'libexec/rc/rc.d/local_unbound').read_text()
makefile = (repo / 'Makefile.inc1').read_text()
match = re.search(r'^\.if make\(installworld\) && \$\{MK_UNBOUND\}.*?^\.endif', makefile, re.M | re.S)
assert match
block = match.group()
recipe = next(line.strip() for line in block.splitlines() if line.startswith('\t'))
assert recipe == '/bin/sh ${.CURDIR:Q}/libexec/rc/rc.d/local_unbound setup'
assert '${MK_UNBOUND} != "no"' in block
assert '!defined(NO_ROOT)' in block and '!defined(CROSSBUILD_HOST)' in block
assert '!defined(DESTDIR) || empty(DESTDIR) || ${DESTDIR} == "/"' in block

with tempfile.TemporaryDirectory(prefix='unbound-installworld-') as directory:
    root = Path(directory)
    for name in ('etc/rc.d', 'var/unbound/conf.d', 'var/backups', 'var/run', 'bin'):
        (root / name).mkdir(parents=True)
    # Exercise quoting of the source root in the make recipe.
    source_tree = root / 'source tree'
    current_rc = source_tree / 'libexec/rc/rc.d/local_unbound'
    current_rc.parent.mkdir(parents=True)
    trace = root / 'trace'
    helpers = root / 'rc.subr'
    helpers.write_text('''load_rc_config() { :; }
checkyesno() { [ "$local_unbound_tls" = yes ]; }
warn() { echo "$*" >&2; }
run_rc_command() {
    case "$1" in
    setup) local_unbound_setup ;;
    restart)
        echo restart >> "$TRACE"
        local_unbound_prestart ;;
    anchor) local_unbound_anchor ;;
    *) return 1 ;;
    esac
}
''')
    # Adapt only absolute OS paths for the isolated test filesystem.
    adapted = source_rc.replace('. /etc/rc.subr', f'. {shlex.quote(str(helpers))}')
    adapted = adapted.replace('command="/usr/sbin/local-unbound"', f'command="{root}/bin/local-unbound"')
    adapted = adapted.replace('/var/run/ipfw.ready', str(root / 'var/run/ipfw.ready'))
    adapted = adapted.replace('/usr/sbin/service', str(root / 'bin/service'))
    # Anchor existence tests must see the DESTDIR fixture, while the -a argument
    # remains the actual configured path passed to the helper.
    adapted = adapted.replace('[ ! -f ${local_unbound_config} ]', '[ ! -f "${DESTDIR}${local_unbound_config}" ]')
    adapted = adapted.replace('[ -f ${local_unbound_anchor} ]', '[ -f "${DESTDIR}${local_unbound_anchor}" ]')
    adapted = adapted.replace('[ ! -s ${local_unbound_anchor} ]', '[ ! -s "${DESTDIR}${local_unbound_anchor}" ]')
    current_rc.write_text(adapted)
    installed_rc = root / 'etc/rc.d/local_unbound'
    legacy = adapted.replace('${command}-setup -n \\',
                             '${command}-setup -n -f ${local_unbound_forwardconf} \\')
    assert legacy != adapted
    installed_rc.write_text(legacy)
    setup = root / 'bin/local-unbound-setup'
    setup.write_text('''#!/bin/sh
printf 'setup %s\\n' "$*" >> "$TRACE"
[ "${FAIL_SETUP:-0}" = 0 ] || exit 1
exec /bin/sh ''' + shlex.quote(str(repo / 'usr.sbin/unbound/setup/local-unbound-setup.sh')) + ''' "$@"
''')
    checkconf = root / 'bin/local-unbound-checkconf'
    checkconf.write_text('#!/bin/sh\necho configtest >> "$TRACE"\nexit "${CHECK_STATUS:-0}"\n')
    anchor_tool = root / 'bin/local-unbound-anchor'
    anchor_tool.write_text('''#!/bin/sh
echo anchor >> "$TRACE"
[ "$1" = -a ] || exit 1
printf 'TEST-ONLY bootstrap fixture\\n' > "${DESTDIR}${2}"
''')
    su = root / 'bin/su'
    su.write_text('#!/bin/sh\nread -r command\nexec /bin/sh -c "$command"\n')
    service = root / 'bin/service'
    service.write_text('''#!/bin/sh
if [ "$*" = 'ipfw status' ]; then exit 0; fi
exec /bin/sh "$DESTDIR/etc/rc.d/local_unbound" "$2"
''')
    for executable in (setup, checkconf, anchor_tool, su, service):
        executable.chmod(0o755)
    config = root / 'var/unbound/unbound.conf'
    anchor = root / 'var/unbound/root.key'
    records = ('server:\n'
               '    local-zone: "home.test." static\n'
               '    local-data: "host.home.test. IN A 192.0.2.10"\n'
               '    local-data-ptr: "192.0.2.10 host.home.test."\n'
               '    auto-trust-anchor-file: /var/unbound/root.key\n')
    config.write_text(records)
    anchor.write_text('TEST-ONLY existing mutable trust-anchor state\n')
    original_anchor = anchor.read_bytes()
    (root / 'etc/resolv.conf').write_text('search home.test\nnameserver 192.0.2.53\n')
    (root / 'var/run/ipfw.ready').touch()
    environment = dict(os.environ, DESTDIR=str(root), TRACE=str(trace),
                       PATH=str(root / 'bin') + ':' + os.environ['PATH'],
                       local_unbound_tls='yes',
                       local_unbound_forwardconf='/var/unbound/forward.conf')
    # First reproduce the old installed script passing -f to the new utility.
    trace.write_text('')
    before = subprocess.run([str(service), 'local_unbound', 'setup'],
                            env=environment, timeout=10, capture_output=True, text=True)
    assert before.returncode != 0
    assert ' -f /var/unbound/forward.conf ' in trace.read_text()
    assert 'usage:' in before.stderr
    assert config.read_text() == records and anchor.read_bytes() == original_anchor
    print('PASS reproduced obsolete installed rc -f / new setup utility mismatch')

    hook = recipe.replace('${.CURDIR:Q}', shlex.quote(str(source_tree)))
    for running in ('', '1234'):
        trace.write_text('')
        after = subprocess.run(['/bin/sh', '-c', hook],
                               env=dict(environment, rc_pid=running), timeout=10, capture_output=True, text=True)
        assert after.returncode == 0, after.stdout + after.stderr
        lines = trace.read_text().splitlines()
        assert lines[0].startswith('setup -n ') and ' -f ' not in lines[0]
        assert lines[1:] == (['configtest', 'restart'] if running else [])
        assert config.read_text().startswith(records)
        assert 'tls-upstream: yes' in config.read_text()
        assert 'tls-cert-bundle: /etc/ssl/cert.pem' in config.read_text()
        assert anchor.read_bytes() == original_anchor
        assert not (root / 'var/unbound/forward.conf').exists()
        assert 'forward-zone:' not in config.read_text() and 'forward.conf' not in config.read_text()
        assert installed_rc.read_text() == legacy  # etcupdate-managed file is untouched.
    result = config.read_bytes()
    trace.write_text('')
    subprocess.run(['/bin/sh', '-c', hook], env=environment, check=True, timeout=10, capture_output=True)
    assert config.read_bytes() == result and anchor.read_bytes() == original_anchor
    print('PASS source-tree hook succeeds with stale installed rc, preserves records/TLS/anchor and is idempotent')

    # Bootstrap still runs from the source service prestart after a restart.
    anchor.unlink()
    trace.write_text('')
    subprocess.run(['/bin/sh', '-c', hook], env=dict(environment, rc_pid='1234'),
                   check=True, timeout=10, capture_output=True)
    assert trace.read_text().splitlines()[1:] == ['configtest', 'restart', 'anchor']
    assert anchor.read_text() == 'TEST-ONLY bootstrap fixture\n'
    print('PASS missing-anchor initialization still dispatches through service prestart (bootstrap mocked)')

    for failure, expected in (('FAIL_SETUP', []), ('CHECK_STATUS', ['configtest'])):
        trace.write_text('')
        failed = subprocess.run(['/bin/sh', '-c', hook],
                                env=dict(environment, rc_pid='1234', **{failure: '1'}), timeout=10, capture_output=True)
        assert failed.returncode != 0
        assert trace.read_text().splitlines()[1:] == expected
    print('PASS setup/config-validation failures propagate without restarting the daemon')

    # rc.conf path overrides must reach the new setup utility unchanged.
    site = root / 'var/unbound/site.conf'
    site_anchor = root / 'var/unbound/site.key'
    site_records = records.replace('/var/unbound/root.key', '/var/unbound/site.key')
    site.write_text(site_records)
    site_anchor.write_bytes(original_anchor)
    trace.write_text('')
    subprocess.run(['/bin/sh', '-c', hook],
                   env=dict(environment, rc_pid='1234',
                            local_unbound_config='/var/unbound/site.conf',
                            local_unbound_controlconf='/var/unbound/admin.conf',
                            local_unbound_anchor='/var/unbound/site.key'),
                   check=True, timeout=10, capture_output=True)
    assert site.read_text().startswith(site_records)
    assert site_anchor.read_bytes() == original_anchor
    assert ' -c /var/unbound/site.conf ' in trace.read_text()
    assert ' -o /var/unbound/admin.conf ' in trace.read_text()
    assert ' -a /var/unbound/site.key ' in trace.read_text()
    assert installed_rc.read_text() == legacy
    print('PASS rc.conf configuration/control/anchor path overrides reach the new utility')
