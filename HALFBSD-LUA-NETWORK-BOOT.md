# HalfBSD Lua network inspection checkpoint

The default is now `halfbsd.boot_mode=lua`, `halfbsd.boot_stage=network`.
This extends the successful ZFS/AMDGPU single-user checkpoint without enabling
full multiuser startup. The first reported native attempt reached the new graph but failed because the
root remount remained read-only. The adapter now explicitly supplies `rw` with
`MNT_UPDATE`; the prior request retained the existing `ro` mount option. Portable
regression tests reproduce that failure and pass with the fix. A new native boot
is required to verify the correction.

## What starts

The existing scheduler runs serially: devices, writable root, configured local
filesystems/ZFS, AMDGPU and DRM readiness, runtime directories, configured swap,
loopback, IPFW, Ethernet/DHCP and address/default-route readiness. Library hints,
hostname and local syslog are also initialized. IPFW must succeed before DHCP.
The existing supervisor owns DHCP and syslog throughout console inspection.

DNS, SSH, NTP, seatd, desktop, gettys and administrator-added services stay off.
Persisted enable settings do not activate additional services. Only status/list/
graph control commands work. Normal reboot invokes existing managed cleanup.
Exiting the inspection shell gives another inspection shell, without restarting
initialization or advancing to multiuser.

`/etc/halfbsd/system.lua` supplies storage, swap, hostname, interface and firewall
policy. Existing administrator files are preserved. Root must be ZFS; the AMDGPU
module and firmware must be installed. An unavailable required component enters
recovery, without automatically running RC. Trusted Lua configuration is still
executed when loaded: keep it declarative.

## Before the test

Keep console access and a known-good boot environment. From the working RC boot,
update, build and install world/kernel using the usual procedure. Then run:

```sh
sh /usr/src/tests/halfbsd/phase4/run.sh
halfinit --config /etc/halfbsd/system.lua --network-checkpoint --dry-run
```

The dry run must include `graphics`, `firewall`, `dhcp-<interface>` and `network`,
and exclude DNS/SSH/NTP. It does not activate the graph or modify networking.
Tests must be rerun on FreeBSD; local Linux policy results are not native proof.

At the loader, explicitly select the intended checkpoint to override any old
loader settings:

```text
set halfbsd.boot_mode=lua
set halfbsd.boot_stage=network
boot
```

Use plain `boot`, not `boot -s`, which bypasses the Lua handoff.

## Console evidence

Look for:

```text
[HalfBSD init] launching Lua NETWORK inspection checkpoint
[HalfBSD init] exec /usr/sbin/halfinit; console attached, notification fd=3
[halfinit boot] ... -> STARTING: ...
[halfinit boot] ... -> READY: ...
[HalfBSD init] NETWORK READY; opening inspection shell and retaining the Lua supervisor
[HalfBSD init] Network checkpoint ready; DHCP/syslog remain supervised while you inspect.
```

Service names and reasons fill the ellipses. Record the last transition on a
failure. PID 1 prints a waiting heartbeat every five seconds, with a 600-second
startup bound; individual services have shorter deadlines. DHCP failure means
recovery rather than a successful network checkpoint.

At the shell:

```sh
kenv halfbsd.boot_active       # lua
kenv halfbsd.boot_stage        # network
kenv halfbsd.boot_result       # network-ready
pgrep -laf halfinit
halfsvc status
mount
swapinfo
ifconfig
netstat -rn -f inet
ipfw list
ping -c 3 1.1.1.1
cat /var/run/halfbsd/boot.log
```

`network-ready` records the startup handoff; use `halfsvc status` for current
health. Root should be writable. Confirm the selected Ethernet address and default
route, and verify firewall rules separately. Ping depends on upstream ICMP policy.
DNS names and Git hostnames will not resolve from this checkpoint because Unbound
is deliberately not started. Use RC recovery to fetch subsequent revisions.
Do not run the entire `/etc/rc` over this partially initialized system.

Use normal `reboot` to test ordered cleanup. Do not use `reboot -q`, which bypasses
normal shutdown. Photograph failures and report the boot variables and service
status. No live managed shutdown or network-checkpoint boot has been verified yet.

## Recovery

At the next loader prompt:

```text
set halfbsd.boot_mode=rc
boot
```

This selects traditional RC explicitly; a hard reset does not select RC on its
own. To return to the proven minimal diagnostic instead:

```text
set halfbsd.boot_mode=lua
set halfbsd.boot_stage=single
boot
```

The `single` diagnostic leaves root read-only and starts no networking. Full
`halfbsd.boot_stage=multiuser` remains available as a later test, but is not
needed for this checkpoint. Keep the staged bring-up until network and shutdown
results are confirmed.

## Root remount correction

On the next attempt, look for `rootfs STARTING -> READY`, followed by local
filesystem/ZFS and graphics initialization. If `rootfs` still fails, record its
exact reason and use RC recovery. We retain readback validation rather than
proceeding on a successful syscall alone. No automatic RC fallback is introduced.

## Diagnosing a later firewall-stage failure

A subsequent administrator screenshot shows rootfs, local filesystems, ZFS,
AMDGPU/graphics, runtime, swap and loopback READY before shutdown. DHCP was still
queued and firewall was skipped. The originating failure is above the photographed
region, so this evidence does not identify the failed firewall prerequisite.
From that recovery shell, preserve the log before rebooting:

```sh
grep -n -E 'FAILED|SKIPPED|verification|errno' /var/run/halfbsd/boot.log | head -20
```

The new diagnostic repeats `FIRST FAILURE` after cleanup, immediately before the
`boot.shutdown` record. It retains the first failed service, reason, process exit
status/signal and up to 4096 bytes of captured output, encoded on one console line.
Later dependency skips and cleanup failures cannot replace that original cause.
This diagnostic passed portable policy tests; native execution is pending.

## IPFW protected counter correction

The administrator supplied the missing baseline output: two rules numbered 65535
in set 31, the first `count ip from any to any not // orphaned dynamic states
counter`, the second `deny ip from any to any`. The kernel's
`sys/netpfil/ipfw/ip_fw_dynamic.c:dyn_add_protected_rule()` creates the counter,
marks its comment instruction F_NOT so it never matches normal packets, and
inserts it through `ipfw_add_protected_rule()`. It is internal accounting, not an
accepting policy. The validator incorrectly assumed there was only one row.

Both baseline and final policy validation now recognize at most one exact
protected counter while still requiring the actual default deny and final deny
guard. Altered/duplicate counters, accepting defaults and unexpected load-policy
rules remain errors. No firewall rules are removed or changed by this correction.
The regression reproduced the original failure before the fix. All 18 Phase III
and 14 Phase IV policy groups pass locally; native boot validation is pending.
The first-failure summary described above is included in the pending changes.
