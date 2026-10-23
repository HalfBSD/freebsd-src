#!/usr/bin/env python3
"""Source integrity and isolated fresh-install policy generation; no host writes."""
import os
from pathlib import Path
import subprocess
import tempfile
root = Path(__file__).resolve().parents[3]
for name in ('libexec/rc', 'sbin/rcorder', 'usr.sbin/sysrc', 'tests/etc', 'tools/tools/nanobsd'):
    path = root / name
    assert not any(p.is_file() for p in path.rglob('*')), name
for file in ('libexec/Makefile', 'sbin/Makefile', 'usr.sbin/Makefile'):
    text = (root / file).read_text()
    assert '\trc \\\n' not in text and '\trcorder \\\n' not in text and '\tsysrc \\\n' not in text
init = (root / 'sbin/init/init.c').read_text()
for symbol in ('runshutdown', 'runfinal', 'run_script', 'execute_script', 'lua_mode', 'boot_attempted', '_PATH_RUNCOM', '_PATH_CONSOLE_LOGIN'):
    assert symbol not in init, symbol
assert 'lua_shutdown();' in init and 'stopped(void)' in init
assert 'strcmp(typ->ty_name, "ttyv0") == 0' in init
assert '"/usr/sbin/halfconsole"' in (root / 'sbin/init/ttys').read_text()
for p in [* (root / 'release/tools').glob('*.conf'), root / 'release/tools/vmimage.subr']:
    assert '/etc/rc.conf' not in p.read_text(), p
print('PASS retired RC sources/build entries, sole startup/shutdown ownership and direct ttyv0 login')

lua = os.environ.get('HALFBSD_LUA', '/usr/bin/lua')
source = root / 'usr.sbin/bsdinstall/scripts/config'
renderer = root / 'usr.sbin/bsdinstall/scripts/config-halfbsd.lua'
template = root / 'usr.sbin/halfinit/examples/boot-system.lua'
with tempfile.TemporaryDirectory(prefix='halfbsd-installer-') as tmp:
    base = Path(tmp); stage = base / 'stage'; pending = base / 'etc'; boot = base / 'boot'
    for p in (stage / 'etc/halfbsd', stage / 'boot', pending, boot, stage / 'usr/share/examples/halfbsd'):
        p.mkdir(parents=True, exist_ok=True)
    installed_template = stage / 'usr/share/examples/halfbsd/boot-system.lua'
    installed_template.write_bytes(template.read_bytes())
    policy = stage / 'etc/halfbsd/system.lua'
    policy.write_bytes(template.read_bytes())
    (stage / 'etc/sysctl.conf').write_text('')
    runner = base / 'config.sh'
    runner.write_text(source.read_text().replace('/usr/bin/lua', lua).replace('/usr/libexec/bsdinstall/config-halfbsd.lua', str(renderer)))
    env = {**os.environ, 'BSDINSTALL_TMPETC': str(pending), 'BSDINSTALL_TMPBOOT': str(boot), 'BSDINSTALL_CHROOT': str(stage)}
    def configure(host):
        (pending / 'hostname').write_text(host + '\n')
        (pending / 'sysctl.conf.defaults').write_text('')
        (boot / 'loader.conf.defaults').write_text('')
        return subprocess.run(['sh', str(runner)], env=env, capture_output=True, timeout=10)
    assert configure('anvil.test').returncode == 0
    first = policy.read_bytes()
    subprocess.run([lua, '-E', '-e', 'local c=assert(loadfile(arg[1]))(); assert(c.boot.hostname=="anvil.test" and c.target=="multiuser" and c.boot.dns); os.exit(0)', '--', 'check', str(policy)], check=True)
    assert configure('other.test').returncode == 0 and policy.read_bytes() == first
    policy.write_bytes(template.read_bytes())
    assert configure('invalid;hostname').returncode != 0 and policy.read_bytes() == template.read_bytes()
    policy.unlink(); policy.symlink_to(installed_template)
    assert configure('anvil.test').returncode != 0 and policy.is_symlink()
print('PASS native policy renderer and actual installer publication: hostname, administrator preservation, invalid data and symlink rejection')
