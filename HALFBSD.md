# HalfBSD workstation

Current source: October 22, 2026, branch `halfbsd2`. This guide supersedes the
individual Lua phase, boot-checkpoint, account, device-event, shutdown and RC
handoffs. Historical implementation details remain in Git.

## Startup architecture

The FreeBSD C `/sbin/init` remains PID 1. It launches `/usr/sbin/halfinit`, which
embeds the in-tree Lua 5.4.8 runtime. Lua supplies one dependency graph and
supervisor; native process descriptors and kqueue supply ownership and events.
`halfsvc` controls that manager. Traditional RC, rc.subr, rcorder, sysrc and
RC-only configuration tools are removed; `service` is a thin halfsvc frontend.
Package rc.d scripts are not automatically executed or converted.

Supported normal boot assumes AMD64, UEFI and a kernel-mounted ZFS root with the
runtime available. The graph prepares devices, writable storage, supported local
mounts, already-imported ZFS datasets, runtime directories, swap, IPFW, DHCP,
Unbound, SSH, NTP, logging, devd and seatd. Independent services use the scheduler;
the installed example selects `max_parallel=1` for deterministic initial startup.
IPFW readiness precedes network services. DHCP supplies addresses/routes, not DNS.
AMDGPU remains an external module; the HALFBSD kernel includes hms/HID support.

Administrator policy lives in `/etc/halfbsd/system.lua`. Installation seeds it
only if absent; upgrades preserve edits. System definitions/modules live in
`/usr/lib/halfbsd`, examples in `/usr/share/examples/halfbsd`. See the
[shipped policy](usr.sbin/halfinit/examples/boot-system.lua) and
[configuration/service reference](usr.sbin/halfinit/halfinit.8).
Never start a second system manager over an already running installation.

## Console and recovery — latest change

There is one physical console, `ttyv0`, shared by boot text, login and graphics.
Additional local virtual terminals and switching are removed. Seatd accepts one
local graphical client and rejects session switching. Concurrent SSH sessions
and separate human/service identities remain supported. This does not remove
PTYs, Unix multiuser operation or the graphical console itself.

PID 1's independent recovery shell is removed, including `init_shell`, the
alternate-shell prompt and recovery password/exec paths. Invalid configuration,
critical failure or unexpected supervisor exit cleans up and enters a stopped
state. PID 1 keeps reaping and honoring shutdown signals; it does not retry boot
or execute another startup framework. The console says:

```text
Startup stopped. No recovery shell; repair using external FreeBSD media.
```

**Prepare compatible external FreeBSD media or an older working boot environment
before installing this change.** `boot -s` now stops without a shell. The retired
`halfbsd.boot_mode=rc` is rejected. External media must provide the tools to
import the pool, select/repair a boot environment and fix installed files.

`halfbsd.boot_stage=multiuser` remains the default. `single` and `network` are
optional diagnostic checkpoints; neither opens an inspection shell anymore.
The Lua `profile='maintenance'` or `profile='network'` configuration selects a
limited service graph with normal login; it is not independent recovery when
Lua or its configuration is broken.

## Daily operation

```sh
kenv halfbsd.boot_active       # lua
kenv halfbsd.boot_result       # multiuser-ready after successful startup
halfsvc status
halfsvc status dns
halfsvc restart seatd
halfsvc reload
less /var/run/halfbsd/boot.log
```

`halfsvc` supports list/status/graph, start/stop/restart, enable/disable and
transactional reload. Native peer credentials authenticate mutations; status
remains read-only. Enable/disable preferences persist separately from shipped
definitions. Use `shutdown -r now` or `shutdown -p now` for operating-system
reboot/poweroff; `halfsvc shutdown` stops the manager, not the kernel.

After a normal reboot or poweroff, as root:

```sh
halfsvc shutdown-report
```

`CLEAN` means the previous recorded Lua boot completed cleanup and PID 1 observed
a normal zero-status supervisor exit with matching identities. Missing/stale
records are not success. Bounded JSON records live in `/var/db/halfbsd`; details
are in `last-shutdown.json` and `shutdown-pid1.json`. This proves supervised
cleanup/exit, not final firmware poweroff or filesystem integrity.

## Users and desktop

The local console defaults to immediate root login, without an account selector.
Set `console={user='charlie',autologin=true}` to choose another existing account,
or `autologin=false` for authenticated login. Run Sway/Niri as an ordinary user,
not root. devd and base seatd are mandatory workstation services; no
`seatd=true` toggle or package RC startup is needed.

Human accounts created with adduser, the installer or bsdconfig automatically
receive `video` while retaining requested groups. After successful creation,
`/var/run/xdg/NAME` is created/repaired, user-owned and mode `0700`, under a
root-owned `0711` parent. Links are rejected. Root/UID 0, non-login shells and
`/nonexistent` accounts are excluded. Direct `pw`/package account creation is
unchanged; use it for dedicated service roles, including roles with real shells.

The base sh `/etc/profile` exports `XDG_RUNTIME_DIR=/var/run/xdg/$(id -un)` from
the actual identity. Existing accounts are not bulk-migrated. As root, after
checking these paths are not symlinks, provision an existing human account:

```sh
pw groupmod video -m charlie
install -d -o root -g wheel -m 0711 /var/run/xdg
install -d -o charlie -m 0700 /var/run/xdg/charlie
```

Start a fresh login session. Root can be provisioned explicitly if desired.
This adds no PAM dependency for directory creation/export; existing PAM login
and pam_xdg configuration have not yet been removed. There is no new XDG
session cleanup or boot reconstruction if `/var/run` is manually cleared or
made volatile. Third-party shells need their own system environment integration.

Devd module matching uses bounded direct argv execution and honors loader
blocklists. Legacy DHCP/keyboard actions cannot launch duplicate initialization;
the supervisor owns DHCP and kbdmux owns keyboard attachment. Optional power or
Bluetooth actions without providers remain unsupported. Built-in hms and devd
restored USB mouse/keyboard switching on Anvil before the latest console change.

## Acceptance and next work

Administrator-confirmed on Anvil before commit `0409b33055b2`: RC-free native
build/install/reboot, normal Lua multiuser, console login, SSH, DNS, NTP
synchronization, seatd/Sway, USB recovery and CLEAN shutdown reports. The
administrator also accepted XDG setup and the DNS acceptance item.

**The new recovery-shell/VT removal has not yet had a native build or boot.**
Portable tests and FreeBSD-header AMD64/i386 object checks pass; they do not
replace Anvil validation. After installing matching kernel/world, check login,
Sway start/exit, text restoration, USB reconnection, seatd restart, concurrent
SSH and the next shutdown report. Tests are listed in
[HALFBSD-DEVELOPMENT.md](HALFBSD-DEVELOPMENT.md).

Remaining work: native acceptance of this change; review temporary-state cleanup,
host identity/timezone, general sysctl/devfs policy, sleep/resume and ports-service
integration; separately design PAM removal. Arbitrary escaped daemon-descendant
containment, broader native ATF/sanitizer coverage, isolated IPFW packet testing,
and the originally proposed 25 VM boot/shutdown cycles remain unproven.

See [firewall policy](HALFBSD-FIREWALL.md), [DNS](UNBOUND.md) and
[source-reduction boundaries](HALFBSD-SOURCE-REDUCTION.md).
