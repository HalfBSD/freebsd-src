# Using the HalfBSD firewall

HalfBSD's amd64 kernel contains IPFW and ZFS. Neither needs to be loaded
with `kldload`. IPFW starts automatically during networking startup, loads
rules quietly, and enables logging support. Select a policy in
`/etc/rc.conf` using one setting:

```sh
firewall_mode="open"
```

If the setting is absent or empty, the policy is `open`. Policy names are
lowercase and must be exactly one of the three values below. An unknown
name produces `Unknown firewall mode: <value>` and leaves the current
rules unchanged. A filename is never interpreted as a policy.

## Choose a mode

| Mode | Behavior |
|---|---|
| `open` | Allows network traffic in both directions. This is the default. Listening services, including SSH if enabled, are reachable subject to their own configuration. |
| `workstation` | Allows loopback, stateful outbound TCP/UDP/ICMP, replies to those connections, DHCP, and necessary IPv4/IPv6 control traffic. DNS, NTP, package downloads, web browsing, and outbound SSH work through outbound state. Unsolicited inbound connections, including inbound SSH, are denied. |
| `closed` | Allows loopback and denies other IP traffic in both directions. External DHCP, DNS, NTP, SSH, and package downloads are blocked. Local ZFS operations remain available; network-backed operations such as remote replication are blocked. |

`vm-nat` and `server` are reserved design directions, not currently supported
modes. There are no service-port, trusted-host, custom-script, rule-file,
or independent firewall NAT settings in this policy interface.

## Apply a policy

Run these commands as root on the installed HalfBSD system:

```sh
sysrc firewall_mode=workstation
service ipfw reload
```

Use `open` or `closed` in place of `workstation` as appropriate.
`service ipfw start` and `service ipfw restart` also apply the selected
policy. Loading is quiet unless a command fails.

To return to the default policy:

```sh
sysrc firewall_mode=open
service ipfw reload
```

To apply a mode temporarily without changing `/etc/rc.conf`:

```sh
sh /etc/rc.firewall open
```

The configured mode is reapplied at the next reload or boot. The example
`share/examples/ipfw/change_rules.sh open|workstation|closed` applies the
chosen policy and then saves it with `sysrc` if policy loading succeeds.

Changing policies flushes the current rules and dynamic connection state.
Existing network sessions can be interrupted. Apply restrictive modes
from the local console: `workstation` does not admit inbound SSH, and
`closed` blocks external connectivity completely. Connections opened
after a workstation policy load receive normal stateful reply handling.

`service ipfw stop` reports that IPFW is always enabled and fails without
disabling it. To permit traffic, select `open`.

## Check the running firewall

```sh
service ipfw status
sysctl net.inet.ip.fw.enable net.inet6.ip6.fw.enable
sysctl net.inet.ip.fw.verbose
ipfw list
ipfw -a list 65000
ipfw -d list
```

Both protocol enable values and the verbose value should be `1`.
The status command reports whether both protocol hooks are enabled and
shows the configured mode; use `ipfw list` to inspect the actual rules,
especially after a temporary policy change or failed reload.

Every policy installs an explicit ending at rule 65000:

- `open`: `allow ip from any to any`.
- `workstation` and `closed`: `deny log logamount 100 ip from any to any`.

Rule 65535 is the kernel's accepting fallback. This keeps networking
available before rc policy setup and if that setup fails or is delayed.
Restrictive policies do not rely on the fallback to deny packets.
Policy replacement uses a flush followed by rule installation; it is not
an atomic transaction, so the accepting fallback can apply during loading.
A failed command stops loading and returns failure; inspect the rules and
reapply a valid policy to recover.

## Logging

IPFW logging support is always enabled. Only rules containing `log`
produce packet log entries. Ordinary permitted workstation traffic is
not logged. The terminal deny rule logs at most 100 matching packets
per rule load, then continues denying without logging additional matches.

With the normal syslog configuration, inspect `/var/log/security`:

```sh
tail -f /var/log/security
```

Counters remain available through `ipfw -a list`. To reset log counters
without reloading the policy or dropping connection state:

```sh
ipfw resetlog
```

## Upgrading an existing system

Build and install the updated world and the `HALFBSD` amd64 kernel using
the project's normal upgrade procedure. Merge the updated rc defaults,
rc scripts, and `/etc/rc.firewall` using the normal configuration update
procedure, then reboot into the new kernel.

Remove obsolete firewall entries from `/etc/rc.conf` and service-specific
configuration files. The former enable, type, quiet, logging, custom-rule,
client/simple policy, service/trusted-host, co-script, and firewall NAT
settings no longer control the HalfBSD firewall. Replace the old policy
selection with `firewall_mode`. Old settings have no compatibility aliases.

ZFS pool import, boot-environment mounts, dataset mounts, and zvol swap
startup are unconditional. Remove the old ZFS service-enable setting and
`zfs_load`/`openzfs_load` entries from `/boot/loader.conf`; also remove
`zfs`/`openzfs` and `ipfw` from any explicit `kld_list` or loader module
configuration. New installer and VM-image configuration does not request
dynamic ZFS loading.

ZFS encryption-key loading and boot-once activation remain separate choices.
Existing dataset properties, pools, and boot environments are preserved.
The source tree retains loadable IPFW/ZFS implementations and optional
network tooling for other kernel configurations and specialized tests.

After reboot, check static availability:

```sh
sysctl kern.features.zfs
zpool status
zfs list
sysctl net.inet.ip.fw.default_to_accept
kldstat -v
```

The ZFS feature and accepting-default values should be `1`. IPFW and ZFS
registrations should be associated with the kernel rather than requiring
separate `ipfw.ko`, `zfs.ko`, or `openzfs.ko` files. Do not unload or reload
ZFS as part of policy changes.

## Implementation and validation

Static ZFS dependencies were checked against `sys/amd64/conf/NOTES`,
`sys/conf/files`, and the ZFS module declarations. The HalfBSD kernel now
selects `ZFS`, `ZSTDIO`, `crypto`, and `cryptodev`. The static file rules
also select required XDR, NFSv4 ACL, and zlib support. Existing
`sys/conf/kern.pre.mk` rules provide the ZFS compilation flags.
Boot-environment tests now check the kernel ZFS feature instead of loading
`zfs.ko`. The IPFW DTrace definitions require the IPFW provider rather than
an `ipfw.ko` file. A pre-existing empty ZFS boot-environment stop function
was given an explicit no-op so the changed script passes shell syntax checks.

IPFW selects `IPFIREWALL`, `IPFIREWALL_VERBOSE`, and
`IPFIREWALL_DEFAULT_TO_ACCEPT`. Existing rc ordering keeps policy loading
after `netif` and before `NETWORKING`. The policy functions provide a
place for future VM NAT and server rules without restoring the old
configuration model.

Validation commands:

```sh
python3 tools/build/check-halfbsd.py --make bmake
python3 tools/build/tests/halfbsd-firewall.py
make buildworld TARGET=amd64 TARGET_ARCH=amd64
make buildkernel TARGET=amd64 TARGET_ARCH=amd64 KERNCONF=HALFBSD
```

The existing build-policy checks and 13 isolated policy/rc tests passed.
The isolated tests use the actual rc command dispatcher and policy script
with stubbed configuration and kernel commands; they do not change the host
firewall. They cover mode validation, quiet loading, unconditional startup,
logging and hooks, command failures, IPv4-only operation, stop rejection,
and explicit policy endings. Shell syntax and whitespace checks passed.

World and kernel builds were attempted on the Linux development host with
BSD make. Both stopped before compilation because `cc` is unavailable;
no successful world or static-kernel build is claimed. Installation,
reboot, ZFS root import, DHCP/DNS/NTP operation, and actual packet logging
still require validation on a HalfBSD system.

The new installed-system ATF tests run policies in isolated VNET jails,
checking loopback, IPv4 and IPv6 stateful TCP, unsolicited inbound TCP,
closed-mode isolation, explicit endings, hooks/logging settings, and invalid
mode rejection. Run them after installing the new system and tests:

```sh
cd /usr/tests/sys/netpfil/ipfw
kyua test halfbsd
```

These native packet tests were not runnable on the Linux development host.

### Audit boundaries

The default upstream kernel policy is deny, while HalfBSD intentionally
requires accept before rc policy setup. The upstream rc script also couples
firewall startup to module loading, arbitrary rule files, NAT co-scripts,
and independent configuration switches. Those runtime paths were removed.
ZFS rc services formerly shared a service-enable switch and requested a
loadable module; those gates and requirements were removed.

The old four firewall rc variable names remain only in the isolated
negative tests, where deliberately setting them verifies they do not affect
startup or policy selection. No production compatibility aliases remain.
Generic kernel option definitions, module build machinery, historical
module-loading stress tests, generic boot-image tests, and vendored OpenZFS
fallback loading remain for alternate kernels or module-specific testing.
OpenZFS userland checks for an existing ZFS registration before considering
its fallback load, so the static HalfBSD kernel satisfies that path.

Follow-up work: run full world/kernel builds and native packet/boot tests;
check DHCP renewals and IPv6 networking on actual hardware; validate deny
log delivery and rate limits; exercise ZFS boot environments and encrypted
datasets. Future VM networking should be added as a dedicated policy,
including its forwarding/NAT setup, rather than independent firewall switches.

### Files changed

- `.github/workflows/cross-bootstrap-tools.yml`
- `HALFBSD-FIREWALL.md`
- `lib/libbe/tests/be_create.sh`
- `libexec/rc/rc.conf`
- `libexec/rc/rc.firewall`
- `libexec/rc/rc.d/ipfw`
- `libexec/rc/rc.d/ipfw_netflow`
- `libexec/rc/rc.d/zfs`
- `libexec/rc/rc.d/zfsbe`
- `libexec/rc/rc.d/zfskeys`
- `libexec/rc/rc.d/zpool`
- `libexec/rc/rc.d/zpoolreguid`
- `libexec/rc/rc.d/zpoolupgrade`
- `libexec/rc/rc.d/zvol`
- `release/tools/vmimage.subr`
- `sbin/bectl/tests/bectl_test.sh`
- `share/dtrace/ipfw.d`
- `share/examples/ipfw/change_rules.sh`
- `sys/amd64/conf/HALFBSD`
- `sys/conf/NOTES`
- `tests/sys/netpfil/ipfw/Makefile`
- `tests/sys/netpfil/ipfw/halfbsd.sh`
- `tools/build/check-halfbsd.py`
- `tools/build/tests/halfbsd-firewall.py`
- `usr.sbin/bsdinstall/scripts/config`
- `usr.sbin/bsdinstall/scripts/zfsboot`

The supplied `new_firewall.md` request file was removed after implementation.
