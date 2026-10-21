# HalfBSD Lua startup: workstation follow-up

October 20, 2026. Branch `halfbsd2`.

## Native milestone

The administrator reports **“Lua multiuser READY” on Anvil**, after the earlier
successful network checkpoint. The console then displayed a login prompt. This
is native startup-handoff acceptance for the full multiuser change, not merely
portable policy-test success. Actual SSH login, external DNS and DNSSEC behavior,
NTP synchronization, desktop operation and managed shutdown/repeated boot results
have not yet been reported. Retain the earlier test limitations in the Phase IV
handoff; the banner alone does not complete every acceptance criterion.

This file records requested next work. None of the proposed changes below is
implemented by this documentation update.

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

## 2. Make seatd part of the real workstation graph

The native boot graph already has a foreground `seatd` definition with Unix
socket readiness and supervision. It is disabled in the installed example.
There is also a graph gap: setting `seatd=true` creates the service, but the
current `multiuser` target does not itself require or want it. Desktop policy
pulls it in through the session dependency; ordinary multiuser does not.

Next implementation should enable seatd for the workstation policy and add the
appropriate target dependency so it actually starts. Preserve explicit
administrator policy, including installations without the seatd executable.
Define whether missing/failed seatd prevents desktop readiness or is optional
for text-console operation. Verify the installed seatd command-line options,
socket permissions and user/group access on Anvil; do not infer those from a
successful process spawn. Keep a compositor inside a non-root user session.

Test startup, socket readiness, user access, restart, dependency propagation and
shutdown through halfinit. Do not start a duplicate seatd through an RC script.

## 3. Provide one coherent service-control interface

`halfsvc` already supplies list/status/start/stop/restart/enable/disable/reload
through the manager's authenticated Unix socket. It should remain the control
engine for Lua-owned services; there is no need for another supervisor.

The reported `service seatd start` failure is consistent with an RC-oriented
control path and an unregistered/inactive Lua seatd configuration. It is not
proof that seatd cannot work under Lua. First register and activate it in the
real graph, then support normal `halfsvc start seatd` / `halfsvc status seatd`.

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

Implement configurable console autologin and seatd graph integration together,
then test both during one workstation boot. Use halfsvc for runtime control;
assess the thin `service` frontend in the same review. Preserve serial startup
and first-failure console summaries while bringing up the desktop path.

Afterward, verify real SSH access, DNS/DNSSEC, time synchronization, seatd user
access, console account switching, multiple concurrent sessions, orderly reboot
and poweroff, repeated boots and explicit recovery. Broader RC responsibility
coverage and maintenance-profile consolidation can then proceed from evidence.
