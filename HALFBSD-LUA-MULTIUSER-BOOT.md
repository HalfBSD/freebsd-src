# HalfBSD Lua multiuser bring-up

October 20, 2026. The administrator confirmed `NETWORK READY` on Anvil with DHCP
and syslog supervised. This change enables the remaining multiuser services in
one pass. **The administrator has now confirmed the native “Lua multiuser READY”
message on Anvil**, followed by the console login prompt. This establishes the
startup handoff; application-level checks and managed shutdown remain pending.
Requested workstation follow-up is in
[HALFBSD-LUA-NEXT-STEPS.md](HALFBSD-LUA-NEXT-STEPS.md).

## What changes

With no loader override, PID 1 now selects Lua `multiuser`. It reuses the existing
dependency graph and supervisor, keeping the example's `max_parallel=1`. Startup
includes writable storage, ZFS, AMDGPU, runtime directories, swap, hostname,
loopback, IPFW, DHCP, library hints, syslog, Unbound, SSH and NTP. NTP and SSH
configuration verification wait for DNS readiness when DNS is configured.
AMDGPU defaults on for system boot policies; an explicit `amdgpu=false` remains
respected. Existing administrator files and service enable/disable settings are
preserved. Review custom policies that explicitly disable required services.

Unbound uses `/etc/unbound/unbound.conf`, `/var/unbound/root.key` and the existing
compiled provider hierarchy. Existing anchors are checked; missing anchors use
the existing initialization adapter. SSH prepares host keys, checks configuration
and runs foreground with TCP readiness. NTP runs foreground after DNS/network;
its process readiness does **not** mean the clock has synchronized.

When all required services reach readiness, PID 1 opens authenticated console
logins using existing `/etc/ttys` handling. Lua mode invokes native login instead
of the legacy `rc.console` helper. There is no automatic root autologin. The
supervisor remains alive and its runtime controls work normally. Console progress
reopens the console each transition so login's descriptor revocation does not
silence later service changes. `/etc/rc` is not run in Lua mode.

Desktop/autologin are not enabled automatically: they require an explicit user
session policy. This is multiuser boot with console/SSH access, not automatic
compositor launch. Cron, devd/hotplug, arbitrary rc.conf services and complete RC
feature parity are not claimed by this change. The default-mode change adds no
kernel code; rebuilding the kernel solely for this change is unnecessary.

## Update and test

Use the working RC environment to pull and perform the normal world build/install.
Do not launch `halfinit --activate-system` alongside RC's running services.
Before reboot, run the safe regression gate and inspect the full graph:

```sh
sh /usr/src/tests/halfbsd/phase4/run.sh
halfinit --config /etc/halfbsd/system.lua --dry-run
```

The graph should include DNS, sshd, ntpd, graphics, firewall and DHCP, and target
`multiuser`. The installed example already includes these services; upgrades do
not overwrite administrator customizations. Keep a known-good boot environment
and physical console access.

No stage override is required by the new default. If old settings persist, remove
or update their entries once in `/boot/loader.conf` or `/boot/loader.conf.local`.
For the first attempt, select explicitly at the loader:

```text
set halfbsd.boot_mode=lua
set halfbsd.boot_stage=multiuser
boot
```

Use plain boot, not `boot -s`. Expect:

```text
[HalfBSD init] ... default Lua multiuser stage
[HalfBSD init] launching Lua MULTIUSER stage
[halfinit boot] dns ... -> READY: ...
[halfinit boot] sshd ... -> READY: ...
[halfinit boot] ntpd ... -> READY: ...
[HalfBSD init] Lua multiuser READY; starting console logins
```

At the login prompt, authenticate normally. Then:

```sh
kenv halfbsd.boot_active       # lua
kenv halfbsd.boot_stage        # multiuser
kenv halfbsd.boot_result       # multiuser-ready
halfsvc status
/usr/bin/lua -E /usr/src/tests/halfbsd/phase4/check-live.lua
getent hosts example.com
ntpq -pn
```

`check-live.lua` is an opt-in read-only check of required service states, local
DNS UDP response/TCP listener and SSH TCP listener. It neither starts services
nor changes networking. It is not invoked automatically by the isolated suites,
since those must work under RC too. Separately test an actual SSH login from
another LAN machine. DNSSEC validation/failure, external forwarding/failover and
NTP synchronization need their normal additional checks; a READY label is not
proof of those properties.

Use normal `reboot` and confirm subsequent boot succeeds. Do not use `reboot -q`.
Shutdown uses existing reverse dependencies, graceful deadlines and escalation
before PID 1 completes reboot. Native shutdown/repeated boot acceptance remains
pending. A manager or critical startup failure enters recovery, not automatic RC.

## Recovery

The previous network checkpoint remains available:

```text
set halfbsd.boot_mode=lua
set halfbsd.boot_stage=network
boot
```

For the ordinary RC recovery environment:

```text
set halfbsd.boot_mode=rc
boot
```

On failure, photograph the final `FIRST FAILURE` summary after cleanup; it retains
the original service, reason and bounded output. The log is
`/var/run/halfbsd/boot.log`. A full multiuser acceptance claim requires the real
boot, login, external service and shutdown results, not portable tests alone.
