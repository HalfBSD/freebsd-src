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
assert re.search(r'^options\s+IPFIREWALL_DEFAULT_TO_ACCEPT\b', kernel, re.M)
assert re.search(r'^firewall_mode="open"', (rc / 'rc.conf').read_text(), re.M)
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
    "-q add ${FAIL_RULE:-never}"*) exit 1 ;;
esac
exit 0
''')
        path.chmod(0o755)
    harness = root / 'rc.firewall'
    harness.write_text(source.replace('. /etc/rc.subr', f'. "{helpers}"')
                            .replace('. /etc/network.subr', ':')
                            .replace('fwcmd="/sbin/ipfw -q"', f'fwcmd="{root}/ipfw -q"'))
    environment = dict(os.environ, TRACE=str(trace))
    rendered = {}
    for mode in ('open', 'workstation', 'closed'):
        trace.write_text('')
        result = subprocess.run(['sh', str(harness), mode], env=environment, capture_output=True)
        assert result.returncode == 0, result.stderr
        rendered[mode] = trace.read_text().splitlines()
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
    # Neither restrictive policy has a trusted-LAN DNS/SSH exception.
    assert not any(re.search(r'\b(?:53|22)\b', line) for line in rendered['workstation'])
    print('PASS rendered IPFW modes: loopback/stateful outgoing DNS; default open and no LAN DNS/SSH exceptions confirmed')
    trace.write_text('')
    result = subprocess.run(['sh', str(harness), 'workstation'],
                            env=dict(environment, FAIL_RULE='3100'), capture_output=True)
    assert result.returncode != 0
    assert 'ipfw -q -f flush' in trace.read_text()
    assert deny not in trace.read_text()
    print('PASS injected policy-load failure stops before final deny; accepting kernel fallback remains a source-confirmed deficiency')
    trace.write_text('')
    result = subprocess.run(['sh', str(harness), 'invalid-mode'], env=environment, capture_output=True)
    assert result.returncode != 0 and not trace.read_text()

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
assert reaches('ipfw', 'NETWORKING')
assert reaches('ipfw', 'sshd')
assert reaches('ipfw', 'netwait')
assert reaches('netwait', 'local_unbound')
print('PASS rc source graph: IPFW precedes local_unbound through netwait and also precedes NETWORKING/sshd')
print('NATIVE IPFW POLICY, PACKET BLOCKING AND BOOT ORDER ARE NOT VERIFIED')
