#!/usr/bin/env python3
"""Actual repository FreeBSD declarations; syntax checks, not native ABI execution."""
from pathlib import Path
import subprocess,tempfile
root=Path(__file__).resolve().parents[3];cc=__import__('os').environ.get('CC','cc');builtin=subprocess.check_output([cc,'-print-file-name=include'],text=True).strip()
with tempfile.TemporaryDirectory(prefix='halfbsd-console-user-') as tmp:
 t=Path(tmp);(t/'machine').symlink_to(root/'sys/amd64/include');(t/'x86').symlink_to(root/'sys/x86/include')
 for name in ['aio.h','errno.h','fcntl.h','linker_set.h','poll.h','stdatomic.h','stdint.h','syslog.h','ucontext.h']:(t/name).symlink_to(root/'sys/sys'/name)
 for fn in ['contrib/seatd/common/terminal.c','contrib/seatd/seatd/seat.c','contrib/seatd/seatd/client.c','contrib/seatd/seatd/server.c','usr.sbin/vidcontrol/vidcontrol.c','tests/halfbsd/console/native.c']:
  cmd=[cc,'-nostdinc','-isystem',builtin,'-I'+tmp,'-Iinclude','-Isys','-Ilib/libutil','-Icontrib/seatd/include','-Iusr.sbin/vidcontrol','-U__linux__','-D__FreeBSD__=15','-D_XOPEN_SOURCE=700','-D__BSD_VISIBLE=1','-std=c17','-Wall','-Wextra','-Wpedantic','-Werror','-fsyntax-only',fn]
  r=subprocess.run(cmd,cwd=root,capture_output=True,text=True);
  if r.returncode: raise SystemExit(r.stderr)
  print('PASS FreeBSD userland headers, strict C17:', fn)
