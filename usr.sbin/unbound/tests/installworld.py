#!/usr/bin/env python3
"""Standalone installworld refresh with stale installed RC; no live daemons."""
import os
from pathlib import Path
import subprocess
import tempfile

repo = Path(__file__).resolve().parents[3]
source = (repo / 'usr.sbin/unbound/setup/local-unbound-refresh.sh').read_text()
assert '. /etc/rc.subr' not in source
assert '/bin/sh ${.CURDIR:Q}/usr.sbin/unbound/setup/local-unbound-refresh.sh' in (repo / 'Makefile.inc1').read_text()
with tempfile.TemporaryDirectory(prefix='unbound-refresh-') as tmp:
    root = Path(tmp)
    (root / 'etc/rc.d').mkdir(parents=True)
    (root / 'etc/rc.d/local_unbound').write_text('exit 99 # obsolete -f option\n')
    trace = root / 'trace'
    for name, body in {
        'setup': 'echo setup >> "$TRACE"\nexit "${SETUP_STATUS:-0}"',
        'checkconf': 'echo checkconf >> "$TRACE"\nexit "${CHECK_STATUS:-0}"',
        'su': '[ "$1 $2 $3" = "-m unbound -c" ] || exit 99\nshift 3\nexec /bin/sh -c "$1"',
        'pgrep': 'exit "${PROCESS_STATUS:-0}"',
        'halfsvc': 'echo "halfsvc $*" >> "$TRACE"\nexit "${CONTROL_STATUS:-0}"',
    }.items():
        path = root / name
        path.write_text('#!/bin/sh\n' + body + '\n')
        path.chmod(0o755)
    adapted = source
    for installed, local in [('local-unbound-setup', 'setup'), ('local-unbound-checkconf', 'checkconf'), ('halfsvc', 'halfsvc')]:
        adapted = adapted.replace('/usr/sbin/' + installed, str(root / local))
    adapted = adapted.replace('/bin/pgrep', str(root / 'pgrep')).replace('/usr/bin/su', str(root / 'su'))
    script = root / 'refresh'
    script.write_text(adapted)
    cases = [({}, 0, ['setup', 'checkconf', 'halfsvc restart dns']),
             ({'PROCESS_STATUS': '1'}, 0, ['setup', 'checkconf']),
             ({'SETUP_STATUS': '2'}, 2, ['setup']),
             ({'CHECK_STATUS': '2'}, 2, ['setup', 'checkconf']),
             ({'CONTROL_STATUS': '3'}, 3, ['setup', 'checkconf', 'halfsvc restart dns']),
             ({'PROCESS_STATUS': '2'}, 1, ['setup', 'checkconf'])]
    for settings, expected, commands in cases:
        trace.write_text('')
        result = subprocess.run(['sh', str(script)], env={**os.environ, 'TRACE': str(trace), **settings},
                                timeout=5, capture_output=True)
        assert result.returncode == expected, (settings, result.stderr)
        assert trace.read_text().splitlines() == commands
    assert (root / 'etc/rc.d/local_unbound').read_text().startswith('exit 99')
print('PASS independent installworld refresh, stale RC ignored, no unwanted activation, validation/control/inspection failures propagated')
