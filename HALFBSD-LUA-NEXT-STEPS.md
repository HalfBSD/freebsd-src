# Current update — October 22, 2026

PID 1's independent recovery shell and local VT switching are now removed in
source. The sole boot/login/graphics console remains. See
[HALFBSD-CONSOLE-REDUCTION.txt](HALFBSD-CONSOLE-REDUCTION.txt) for behavior, tests,
external-media recovery and native validation still required for this pass.

The administrator accepted the prior RC-free native build/install/boot, DNS,
NTP synchronization, clean supervised shutdown and XDG account setup. Those
results predate this console reduction. Older pending/retain-recovery entries
below are historical and superseded. PAM removal and the remaining startup
responsibility audit remain future work.

---

# HalfBSD Lua startup: workstation follow-up

## Current update: traditional RC source removed

The RC removal requested by the administrator is implemented locally. See
HALFBSD-RC-REMOVAL.txt for deleted source, integration changes, validation and
limitations. Native build/boot acceptance remains pending. Earlier RC-retention
and RC-recovery descriptions below are historical and superseded: boot -s and
compatible external FreeBSD media are the recovery routes. The independent
shell and single-local-VT decisions remain separate TODOs.


October 20, 2026. Branch `halfbsd2`.

## TODO: decide whether to retire PID 1's independent recovery shell

The administrator notes that a compatible FreeBSD USB environment can import
the ZFS pool and repair the installation. Revisit whether this external-media
recovery policy makes the built-in root recovery shell unnecessary. This is
separate from RC removal: keep the C PID 1 recovery shell operational during
the current RC deletion, and make any later removal an explicit change with a
documented external recovery procedure.

## TODO: enforce a single local virtual console

The administrator wants only one local virtual console. `/etc/ttys` already
starts login only on ttyv0, but kernel VT code still defaults to 12 terminals
and retains switching shortcuts/ioctls; seatd also retains switching support.
Disabling extra login sessions did not eliminate those underlying facilities.

- Audit and enforce a single local console, including requests originating from
  keyboard shortcuts, utilities and compositors. Do not reintroduce extra gettys.
- Preserve the console ownership, graphics/text handoff and device release/
  reacquisition needed to start and exit Sway on that same console.
- Verify kernel configuration and seatd/libseat assumptions before removing
  shared code. Test Sway start/exit, console restoration, USB reconnection and
  local recovery on native HalfBSD.
- Preserve concurrent SSH sessions and distinct user/service identities; this
  is a restriction on local virtual consoles, not Unix multiuser operation.

Implementation and native validation are pending. Replace the earlier generic
“VT switching” acceptance item with this single-console requirement.

## October 21 workstation implementation batch

Shutdown acceptance now has persistent evidence: after the updated code has
booted once, reboot/poweroff normally and run `halfsvc shutdown-report` as root
on the next boot. See [HALFBSD-SHUTDOWN-REPORTS.md](HALFBSD-SHUTDOWN-REPORTS.md).
No manual console capture during shutdown is required.

The six-item agenda is now implemented as code, tests and an RC source audit;
native acceptance of this batch is pending. See
[HALFBSD-WORKSTATION-FOLLOWUP.md](HALFBSD-WORKSTATION-FOLLOWUP.md) for the complete
test batch and [HALFBSD-RC-RESPONSIBILITY-AUDIT.md](HALFBSD-RC-RESPONSIBILITY-AUDIT.md)
for remaining RC callers. The original sections below preserve the requested
behavior; this update supersedes their implementation-status statements.

- Mandatory devd is integrated before seatd. Manual devd startup restored Sway
  USB hotplug; automatic graph startup needs native verification.
- Native halfconsole implements root/named-account autologin, a bounded account
  selector, authenticated-login choice and literal-only console_user migration.
- service dispatches to halfsvc in actively selected Lua mode, never to an RC
  duplicate on failure. RC mode retains the original path.
- Optional workstation/network/maintenance configuration profiles reuse the
  existing graph; independent native and RC recovery are retained.
- New private PTY, policy and frontend tests pass on Linux. A native build/install
  gate and opt-in live DNSSEC/cache/NTP/service collector are included.

Next action: one full native build/install/boot and acceptance batch, including
console account/session behavior, Sway/VT/USB cycles, concurrent SSH, orderly
reboot/poweroff and recovery. No additional implementation prerequisite or
intermediate testing is requested before that batch. RC removal remains deferred.

## October 21 update

SSH authentication and ordinary DNS name resolution have now been confirmed by
the administrator through a remote SSH login and git pull. Base seatd/libseat
integration is implemented; see [HALFBSD-SEATD.txt](HALFBSD-SEATD.txt). Seatd is
required by normal multiuser rather than controlled by a seatd=true toggle.
Private Linux/staged tests pass; native build/device/desktop acceptance is pending.
The seatd section below records the original requested work; its graph gap is
now fixed. Console autologin, service frontend evaluation and profile work remain.

## Human-account defaults update

New login accounts created with adduser (also used by the installer) or bsdconfig
now receive `video` automatically. Dedicated service-account automation through
pw is unchanged. Root does not need membership. Existing accounts are not bulk
modified: grant `charlie` membership explicitly and start a fresh login session.
See [HALFBSD-ACCOUNT-POLICY.md](HALFBSD-ACCOUNT-POLICY.md) for exclusions and tests.

## Native milestone

The administrator reports **“Lua multiuser READY” on Anvil**, after the earlier
successful network checkpoint. The console then displayed a login prompt. This
is native startup-handoff acceptance for the full multiuser change, not merely
portable policy-test success. SSH login and ordinary hostname resolution have since been reported successful.
DNSSEC behavior, NTP synchronization, desktop operation and managed shutdown/
repeated boot results have not yet been reported. Retain the earlier test limitations in the Phase IV
handoff; the banner alone does not complete every acceptance criterion.

This file retains the requested work and its current status. The October 21
seatd implementation supersedes the original optional-service proposal.

## 1. Restore the intended console experience

The RC path used `/etc/rc.console`, which loaded `console_user` through rc.conf
and invoked native `login -f` for that account, defaulting to root. Lua mode
currently substitutes authenticated `/usr/bin/login`, so the reappearing prompt
is explained by the existing implementation rather than a failed boot.

Requested behavior:

- Automatically log into the local console as **root by default**.
- Allow configuring another existing account, such as `charlie`.
- Provide an explicit local account-selection/login path to choose a different
  user without making the ordinary boot wait at a mandatory login prompt.
- Preserve normal login/session setup, groups, home directory, controlling
  terminal, logout and subsequent session behavior.
- Keep this policy local to designated console terminals. SSH authentication
  remains unchanged; a root console must not imply a root desktop compositor.

Use the administrator-owned Lua configuration for the lasting policy, rather
than retaining a dependency on rc.conf or shell evaluation. Existing
`console_user` settings need a deliberate migration path that preserves the
chosen account without interpreting shell code inside the manager. Do not
silently overwrite administrator settings. The exact Lua field name and the
interaction for overriding autologin remain implementation decisions: an
interruptible local selector or explicit switch-user/login action can offer the
alternate account while preserving immediate default autologin.

Test root/default and named-account autologin, explicit alternate selection,
missing/invalid accounts, group setup, logout/relogin, terminal ownership and
recovery. Never turn an invalid account into unexpected root access; document
its failure behavior. Multiple terminals and SSH sessions must still coexist.

## 2. Base seatd integration (implemented; native tests pending)

Seatd/libseat 0.9.3 is imported under `contrib/seatd`, with native BSD build rules
and installation into the base system. The Lua workstation graph now requires
`/usr/sbin/seatd`; there is no startup toggle to set. It waits for a root:video
mode-0770 socket and a successful connection. The former missing target edge
is fixed. Legacy boolean fields in existing configurations are normalized in
memory with a warning, without changing administrator files.

Before native acceptance, complete the ports-package/library migration review,
verify the user's `video` membership, and test input/graphics access, virtual
terminal switching, restart and shutdown on Anvil. The full report and commands
are in [HALFBSD-SEATD.txt](HALFBSD-SEATD.txt). Private Linux tests do not prove
native device behavior. A compositor still belongs in a non-root user session.
Do not start another seatd through an RC script.

## 3. Provide one coherent service-control interface

`halfsvc` already supplies list/status/start/stop/restart/enable/disable/reload
through the manager's authenticated Unix socket. It should remain the control
engine for Lua-owned services; there is no need for another supervisor.

The earlier `service seatd start` failure was consistent with the RC-oriented
control path and the then-inactive Lua definition. Seatd is now registered and
required in the real graph; use `halfsvc status seatd` and the existing halfsvc
lifecycle commands. A new `service` frontend has not yet been implemented.

For administrator familiarity, evaluate whether `service` should become a thin
mode-aware frontend to halfsvc when Lua owns startup, with RC behavior retained
only in explicit RC mode. This is an option, not an implemented feature.
Dispatch must use verified active ownership, never launch an RC copy of a
Lua-owned daemon, and preserve peer-credential authorization and structured
errors. Document command differences and unregistered ports-service behavior.
Do not implement a competing scheduler or treat arbitrary RC scripts as native
Lua service definitions. Adding a ports daemon needs a real declarative service
and an explicit identity/environment/readiness policy.

## 4. Preserve users, sessions and service identities

HalfBSD is a workstation, but that does not imply one Unix account or only one
active session. Preserve simultaneous local and remote sessions, including
multiple sessions belonging to the same account where supported.

Users also express service roles and privilege boundaries. A `postgres` account
may own PostgreSQL processes/data without representing a person logged into the
desktop. Keep user/group lookup, supplementary groups, privilege transitions,
service ownership and session ownership distinct. Console-account selection
must not change the identity of unrelated daemons. Do not run PostgreSQL or a
compositor as root because the console autologin default is root.

## 5. Move toward explicit boot profiles, retain real recovery

The requested direction is to replace the everyday notion of “single-user
mode” with explicit startup profiles: normal workstation, networking diagnostic,
no-network maintenance and other deliberately selected service sets. Reuse the
existing registry, targets, dependency planner and supervisor. Do not introduce
another startup implementation per profile.

A profile specifies which services start. Separately specify whether it opens a
maintenance shell, starts normal console sessions, permits remote login or stops
existing sessions. A no-network profile alone does not exclude other local users
and is not the same as a maintenance transition that stops already-running
services. Preserve legitimate concurrent sessions under the normal profile.

The current implemented loader names are:

| Setting | Current behavior |
| --- | --- |
| `halfbsd.boot_mode=lua` | Lua orchestration |
| `halfbsd.boot_mode=rc` | Explicit RC recovery |
| `halfbsd.boot_stage=multiuser` | Default full configured graph and console login |
| `halfbsd.boot_stage=network` | DHCP/syslog inspection shell; DNS/SSH/NTP excluded |
| `halfbsd.boot_stage=single` | Earlier ZFS/AMDGPU diagnostic; no networking |

`halfbsd.boot_state=networking` is a proposed spelling, **not an accepted setting**.
Use the implemented names until a deliberate interface change is implemented.
Avoid accumulating aliases or duplicate selectors merely to accommodate naming.

The traditional operational single-user workflow may become redundant once
maintenance profiles cover it. Independent recovery is still necessary when
Lua itself, its libraries or its configuration cannot load. Keep PID 1's native
recovery shell and explicit RC fallback until replacements have demonstrated
those failure cases. A profile parsed by the broken manager cannot serve as its
own independent recovery path. This does not require retaining traditional RC
forever; retirement needs tested recovery rather than only equivalent service
selection.

## Suggested next implementation batch

Implement configurable console autologin next, and validate it together with
the newly integrated base seatd during one workstation boot. Use halfsvc for runtime control;
assess the thin `service` frontend in the same review. Preserve serial startup
and first-failure console summaries while bringing up the desktop path.

Afterward, verify real SSH access, DNS/DNSSEC, time synchronization, seatd user
access, console account switching, multiple concurrent sessions, orderly reboot
and poweroff, repeated boots and explicit recovery. Broader RC responsibility
coverage and maintenance-profile consolidation can then proceed from evidence.
