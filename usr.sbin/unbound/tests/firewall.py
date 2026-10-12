#!/usr/bin/env python3
"""Audit repository IPFW policy and ordering using mocked kernel commands.

This is source/script verification, not native FreeBSD packet-filter testing.
"""
import os
from pathlib import Path
import re
import subprocess
import tempfile

repo = Path(__file__).resolve().parents[3]
rc = repo / 'libexec/rc'
source = (rc / 'rc.firewall').read_text()
kernel = (repo / 'sys/amd64/conf/HALFBSD').read_text()
assert re.search(r'^options\s+IPFIREWALL\s', kernel, re.M)
assert not re.search(r'^options\s+IPFIREWALL_DEFAULT_TO_ACCEPT\b', kernel, re.M)
assert re.search(r'^firewall_mode="workstation"', (rc / 'rc.conf').read_text(), re.M)
with tempfile.TemporaryDirectory(prefix='unbound-firewall-audit-') as directory:
    root = Path(directory)
    trace = root / 'trace'
    helpers = root / 'helpers'
    helpers.write_text(f'SYSCTL="{root}/sysctl"\nload_rc_config() {{ :; }}\nafexists() {{ return 1; }}\n')
    for name in ('ipfw', 'sysctl'):
        path = root / name
        path.write_text('''#!/bin/sh
printf '%s\n' "''' + name + ''' $*" >>"$TRACE"
case "$*" in
    "-q -S list 65534")
        if [ "${EXISTING_GUARD:-0}" = 1 ]; then
            echo '65534 set 31 deny ip from any to any'
        fi ;;
    "-q add ${FAIL_RULE:-never}"*) exit 1 ;;
esac
exit 0
''')
        path.chmod(0o755)
    harness = root / 'rc.firewall'
    harness.write_text(source.replace('. /etc/rc.subr', f'. "{helpers}"')
                            .replace('. /etc/network.subr', ':')
                            .replace('fwcmd="/sbin/ipfw -q"', f'fwcmd="{root}/ipfw -q"')
                            .replace('/var/run/ipfw.ready', str(root / 'ready')))
    environment = dict(os.environ, TRACE=str(trace))
    rendered = {}
    for mode in ('open', 'workstation', 'closed'):
        trace.write_text('')
        result = subprocess.run(['sh', str(harness), mode], env=environment, capture_output=True)
        assert result.returncode == 0, result.stderr
        rendered[mode] = trace.read_text().splitlines()
        assert (root / 'ready').exists()
        guard = 'ipfw -q add 65534 set 31 deny ip from any to any'
        assert rendered[mode].index(guard) < rendered[mode].index('ipfw -q -f flush')
        assert 'sysctl net.inet.ip.fw.enable=1' in rendered[mode]
        assert 'ipfw -q -f flush' in rendered[mode]
        assert 'ipfw -q add 100 allow ip from any to any via lo0' in rendered[mode]
    assert 'ipfw -q add 65000 allow ip from any to any' in rendered['open']
    deny = 'ipfw -q add 65000 deny log logamount 100 ip from any to any'
    assert deny in rendered['workstation'] and deny in rendered['closed']
    for rule in ('ipfw -q add 2000 check-state',
                 'ipfw -q add 3000 allow tcp from me to any out setup keep-state',
                 'ipfw -q add 3100 allow udp from me to any out keep-state'):
        assert rule in rendered['workstation']
    for rule in ('ipfw -q add 2500 allow udp from any to me 53 in keep-state',
                 'ipfw -q add 2600 allow tcp from any to me 22,53 in setup keep-state'):
        assert rule in rendered['workstation']
        assert rule not in rendered['closed']
    print('PASS default any-source SSH/DNS allowances, persistent deny guard, loopback and outbound DNS/DHCP')
    trace.write_text('')
    result = subprocess.run(['sh', str(harness), 'workstation'],
                            env=dict(environment, FAIL_RULE='3100'), capture_output=True)
    assert result.returncode != 0
    assert 'ipfw -q -f flush' in trace.read_text()
    assert deny not in trace.read_text()
    assert guard in trace.read_text()
    assert not (root / 'ready').exists()
    for failed in ('65534', '100', '2000', '2500', '2600', '3000', '4000', '65000'):
        (root / 'ready').touch()
        trace.write_text('')
        result = subprocess.run(['sh', str(harness), 'workstation'],
            env=dict(environment, FAIL_RULE=failed), capture_output=True)
        assert result.returncode != 0 and not (root / 'ready').exists()
        if failed == '65534':
            assert '-f flush' not in trace.read_text()
    trace.write_text('')
    result = subprocess.run(['sh', str(harness)],
        env=dict(environment, EXISTING_GUARD='1'), capture_output=True)
    assert result.returncode == 0 and guard not in trace.read_text()
    assert deny in trace.read_text() and (root / 'ready').exists()
    print('PASS default invocation, idempotent guard, and failures at guard/loopback/state/service/outbound/DHCP/final-rule stages')
    trace.write_text('')
    result = subprocess.run(['sh', str(harness), 'invalid-mode'], env=environment, capture_output=True)
    assert result.returncode != 0 and not trace.read_text()
    # Explicit LAN settings replace the default any-source DNS/SSH allowances.
    trace.write_text('')
    result = subprocess.run(['sh', str(harness), 'workstation'],
        env=dict(environment, firewall_trusted_interface='em1',
                 firewall_trusted_networks='192.168.50.0/24 10.2.0.0/16'), capture_output=True)
    assert result.returncode == 0, result.stderr
    for network in ('192.168.50.0/24', '10.2.0.0/16'):
        assert f'allow udp from {network} to me 53 in recv em1 keep-state' in trace.read_text()
        assert f'allow tcp from {network} to me 22,53 in recv em1 setup keep-state' in trace.read_text()
    assert 'allow udp from any to me 53 in keep-state' not in trace.read_text()
    assert 'allow tcp from any to me 22,53 in setup keep-state' not in trace.read_text()
    for incomplete in ({'firewall_trusted_networks': '192.168.50.0/24'},
                       {'firewall_trusted_interface': 'em1'}):
        trace.write_text('')
        result = subprocess.run(['sh', str(harness), 'workstation'],
            env=dict(environment, **incomplete), capture_output=True)
        assert result.returncode != 0
        assert guard in trace.read_text() and '-f flush' not in trace.read_text()
        assert not (root / 'ready').exists()
    print('PASS explicit LAN interface/network DNS and SSH allowances; incomplete policy rejected')
    # Execute the real service prestart functions with service/file mocks.
    for service, function in (('local_unbound', 'local_unbound_prestart'),
                              ('sshd', 'sshd_precmd')):
        text = (rc / 'rc.d' / service).read_text()
        body = re.search(r'^' + function + r'\(\)\n\{.*?^\}', text, re.M | re.S).group()
        config = root / 'config'
        config.write_text('fixture')
        gate = root / 'gate'
        gate.write_text('warn() { :; }\nrun_rc_command() { :; }\n'
                        'mock_service() { return "${STATUS_RESULT:-0}"; }\n'
                        f'local_unbound_config="{config}"\nlocal_unbound_anchor="{config}"\n'
                        + body.replace('/var/run/ipfw.ready', str(root / 'ready'))
                              .replace('/usr/sbin/service', 'mock_service')
                        + '\n' + function + '\n')
        for ready, status, success in ((False, '0', False), (True, '1', False), (True, '0', True)):
            (root / 'ready').unlink(missing_ok=True)
            if ready:
                (root / 'ready').touch()
            result = subprocess.run(['sh', str(gate)], env=dict(environment, STATUS_RESULT=status))
            assert (result.returncode == 0) == success
    print('PASS executed DNS/SSH startup gates reject missing readiness and failed firewall status')


# Build the rc metadata dependency graph, including BEFORE edges. This checks
# declared ordering only; it neither runs rcorder nor proves boot success.
files = [p for p in (rc / 'rc.d').iterdir() if p.is_file()]
providers, metadata = {}, {}
for path in files:
    text = path.read_text(errors='replace')
    entries = {key: re.findall(r'^#\s*' + key + r':\s*(.*)$', text, re.M)
               for key in ('PROVIDE', 'REQUIRE', 'BEFORE')}
    metadata[path.name] = {key: ' '.join(value).split() for key, value in entries.items()}
    for provided in metadata[path.name]['PROVIDE']:
        providers.setdefault(provided, []).append(path.name)
edges = {name: set() for name in metadata}
for name, entries in metadata.items():
    for requirement in entries['REQUIRE']:
        for provider in providers.get(requirement, []):
            edges[provider].add(name)
    for before in entries['BEFORE']:
        edges[name].update(providers.get(before, []))
def reaches(start, target):
    pending, visited = [start], set()
    while pending:
        name = pending.pop()
        if name == target:
            return True
        if name not in visited:
            visited.add(name)
            pending.extend(edges.get(name, ()))
    return False
assert reaches('ipfw', 'netif')
assert not reaches('netif', 'ipfw')
assert reaches('ipfw', 'NETWORKING')
assert reaches('ipfw', 'sshd')
assert reaches('ipfw', 'netwait')
assert reaches('netwait', 'local_unbound')
for service in ('local_unbound', 'sshd'):
    text = (rc / 'rc.d' / service).read_text()
    assert '[ ! -f /var/run/ipfw.ready ]' in text
    assert '/usr/sbin/service ipfw status' in text
print('PASS rc graph: IPFW precedes netif, Unbound and SSH; DNS/SSH start gates enforce readiness')
print('NATIVE IPFW POLICY, PACKET BLOCKING AND BOOT ORDER ARE NOT VERIFIED')
