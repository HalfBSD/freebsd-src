#!/usr/bin/env python3
"""Compile init against repository FreeBSD headers; no ABI/link/runtime claim."""
import os
from pathlib import Path
import subprocess
import tempfile
root = Path(__file__).resolve().parents[3]
compiler = os.environ.get('CC', 'cc')
builtin = subprocess.check_output([compiler, '-print-file-name=include'], text=True).strip()
for standard in ('c17', 'gnu17'):
    for arch, flags in [('amd64', []), ('i386', ['-m32'])]:
        with tempfile.TemporaryDirectory(prefix='halfbsd-init-headers-') as tmp:
            stage = Path(tmp)
            (stage / 'machine').symlink_to(root / 'sys' / arch / 'include')
            (stage / 'x86').symlink_to(root / 'sys/x86/include')
            for name in ('aio.h', 'errno.h', 'fcntl.h', 'linker_set.h', 'poll.h', 'stdatomic.h', 'stdint.h', 'syslog.h', 'ucontext.h'):
                (stage / name).symlink_to(root / 'sys/sys' / name)
            command = [compiler, *flags, '-nostdinc', '-isystem', builtin,
                       '-I' + str(stage), '-Iinclude', '-Isys', '-Ilib/libutil', '-Isbin/init',
                       '-D__FreeBSD__=15', '-DLOGIN_CAP', '-DCOMPAT_SYSV_INIT',
                       f'-std={standard}', '-Wall', '-Wextra', '-Wpedantic', '-Werror', '-Wshadow', '-Wcast-align',
                       # Existing PID 1 recursive-state casts and FreeBSD %m formats
                       # are intentional pre-existing conventions, not new APIs.
                       '-Wno-error=cast-function-type', '-Wno-error=format',
                       '-c', 'sbin/init/init.c', '-o', str(stage / 'init.o')]
            result = subprocess.run(command, cwd=root, capture_output=True, text=True)
            if result.returncode:
                print(result.stderr)
                raise SystemExit(result.returncode)
            print(f'PASS {arch} {standard} init object compilation against actual FreeBSD headers (known state-cast/%m warnings allowed; no link/runtime validation)')
