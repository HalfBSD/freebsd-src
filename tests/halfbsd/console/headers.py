#!/usr/bin/env python3
"""FreeBSD-header syntax and DRM layout checks; no native kernel/runtime claim."""
import json
import os
from pathlib import Path
import subprocess
import tempfile
root = Path(__file__).resolve().parents[3]
cc = os.environ.get('CC', 'cc')
builtin = subprocess.check_output([cc, '-print-file-name=include'], text=True).strip()
expected = json.loads((Path(__file__).with_name('layout.json')).read_text())
for arch in ('amd64', 'i386'):
    for cut in (False, True):
        key = arch + ('-nocutpaste' if cut else '')
        with tempfile.TemporaryDirectory(prefix='halfbsd-console-headers-') as tmp:
            stage = Path(tmp)
            (stage / 'machine').symlink_to(root / 'sys' / arch / 'include')
            (stage / 'x86').symlink_to(root / 'sys/x86/include')
            for name in ('syscons', 'splash', 'kbd', 'kbdmux', 'evdev', 'ddb', 'global', 'teken'):
                text = '#define SC_NO_CUTPASTE 1\n' if cut and name == 'syscons' else ''
                if name == 'splash': text += '#define DEV_SPLASH 1\n'
                (stage / ('opt_' + name + '.h')).write_text(text)
            flags = (['-m32'] if arch == 'i386' else []) + [
                '-nostdinc', '-ffreestanding', '-isystem', builtin,
                '-I' + tmp, '-Isys', '-Icontrib/ck/include',
                '-D_KERNEL', '-DKLD_MODULE', '-D__FreeBSD__=15',
                '-DCOMPAT_FREEBSD32', '-std=gnu17', '-Wall', '-Wextra', '-Werror',
                # Existing callback parameters and mouse/history arithmetic
                # already have these warnings in the pre-change kernel source.
                '-Wno-error=unused-parameter', '-Wno-error=sign-compare']
            code = '#include <dev/vt/vt.h>\n'
            for name, size in expected[key].items():
                if name in ('vt_device', 'vt_window'):
                    expr = 'sizeof(struct ' + name + ')'
                else:
                    kind = 'vt_device' if name.startswith('vd_') else 'vt_window'
                    expr = '__builtin_offsetof(struct ' + kind + ', ' + name + ')+1'
                code += '_Static_assert(' + expr + ' == ' + str(size) + ', "DRM layout: ' + name + '");\n'
            (stage / 'layout.c').write_text(code)
            for source in (str(stage / 'layout.c'), 'sys/dev/vt/vt_core.c', 'sys/dev/vt/vt_cpulogos.c'):
                result = subprocess.run([cc, *flags, '-c', source, '-o', str(stage / 'check.o')], cwd=root, capture_output=True, text=True)
                if result.returncode:
                    raise SystemExit(result.stderr)
            print('PASS', key, 'FreeBSD console headers, core/logo object compilation and baseline DRM field layout')
