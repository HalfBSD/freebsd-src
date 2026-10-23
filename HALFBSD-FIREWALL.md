# HalfBSD firewall

Current Lua implementation, October 22, 2026. This replaces the former RC
firewall instructions. `/etc/rc.conf`, sysrc and `/etc/rc.firewall` no longer
select or load policy.

## Policy

The HALFBSD kernel includes IPFW with a default **deny**, not default accept.
The Lua boot graph verifies kernel hooks and the initial deny baseline before
loading rules. It adds an unmatched-packet deny guard at rule 65534 in reserved
set 31, loads the selected rules without a flush, and verifies readback. Network
services depend on successful firewall readiness. A kernel orphaned-dynamic-state
counter is accepted without treating it as an allow rule.

Configure the existing `boot.firewall` table in `/etc/halfbsd/system.lua`:

```lua
firewall={ownership='load',mode='workstation',ipv6=true},
```

| Mode | Behavior |
| --- | --- |
| `workstation` (default) | Loopback; stateful outbound traffic/replies; DHCP and necessary control traffic; incoming SSH TCP 22 and DNS UDP/TCP 53 **from any source**; deny other unsolicited inbound traffic. |
| `closed` | Loopback only; blocks external networking. |
| `open` | Explicitly allows traffic; not a restrictive workstation policy. |

The any-source SSH/DNS allowances are intentional administrator policy. They are
not LAN-only and Unbound has no source-network ACL. SSH authentication remains.
The current Lua schema has no trusted-network/interface restriction fields;
older RC-only settings are retired. IPFW remains the source-access authority.
No automatic gateway/NAT policy is inferred from a second network interface.

`ownership='load'` is for a fresh boot with only the expected kernel deny baseline;
it refuses an existing unexpected rule set. `ownership='verify'` checks an
explicitly pre-established policy rather than taking it over. This is not a
runtime mode-switching service. Boot-policy changes require a planned reboot;
`halfsvc reload` does not replace firewall policy. Closed mode blocks DHCP/DNS
requirements of a normal workstation graph; use an intentionally configured
offline maintenance profile rather than expecting normal network readiness.
Prepare external recovery before making restrictive boot changes.

## Inspect

```sh
halfsvc status firewall
sysctl net.inet.ip.fw.default_to_accept
sysctl net.inet.ip.fw.enable net.inet6.ip6.fw.enable
ipfw -S list
ipfw -a list
ipfw -d list
```

The default-to-accept value should be 0. Enabled protocol hooks should be 1 for
the families selected in policy. Read actual rules rather than treating a READY
label or a compiled kernel feature as proof of packet filtering.
The workstation ending denies with bounded logging at rule 65000; packet log
routing follows syslog configuration, normally `/var/log/security`.
`ipfw resetlog` resets logging counters without replacing the policy.
Do not use removed `service ipfw`, readiness-marker files or RC scripts.

## Limits and evidence

Source: [boot graph](usr.sbin/halfinit/modules/boot.lua) and
[policy readback](usr.sbin/halfinit/modules/adapters.lua). Source/policy tests
cover initialization order, deny guards, malformed readback and failure gating.
The administrator confirmed working Lua boot, DHCP, SSH and DNS on Anvil.
That does not establish isolated IPv4/IPv6 packet-policy or failure-injection
coverage. Native VNET tests, DHCP/link-loss behavior, logging limits and
sleep/resume remain separate validation work. Do not test disruptive firewall
failure scenarios over the only remote connection.
