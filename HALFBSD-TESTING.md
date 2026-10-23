# HalfBSD validation backlog

This document tracks checks and acceptance evidence, separately from work to
change or remove functionality. Test commands and build requirements are in
[HALFBSD-DEVELOPMENT.md](HALFBSD-DEVELOPMENT.md). Check an item off only when its
results are recorded, identifying the tested commit and environment.

## Existing evidence

Before `0409b33055b2`, the administrator confirmed native RC-free build,
installation, multiuser startup, console login, SSH, DNS, NTP synchronization,
seatd/Sway, USB keyboard/mouse reconnection and CLEAN shutdown reports on Anvil.
XDG setup and the DNS acceptance item were also accepted. Earlier Phase I–IV
userspace suites passed as regular user and root. These are accepted results;
they do not need repeating merely to close an old TODO.

The latest recovery-shell and virtual-console removal has portable policy,
helper and FreeBSD-header compilation coverage. Its native acceptance remains
pending. A CLEAN shutdown report proves supervisor cleanup and PID 1 reaping,
not kernel filesystem integrity or final hardware poweroff.

## Latest console and recovery changes

- [ ] Build matching world and kernel natively, including the compatibility pass;
  install successfully. Retain compiler/build results.
- [ ] Boot to normal Lua multiuser and the expected root console prompt; verify
  SSH can remain active alongside local use.
- [ ] Run the native console ioctl check documented in the developer guide:
  one console, no extra ttyv devices, removed switching requests rejected.
- [ ] Start and exit Sway; confirm AMDGPU rendering, keyboard/mouse input and
  restoration of the text console. Repeat USB switching and seatd restart.
- [ ] Reboot and power off; inspect the subsequent shutdown reports for matching
  boot identities, completed cleanup and zero-status supervisor exit.
- [ ] In a disposable VM, check `boot -s`, invalid configuration, critical startup
  failure and manager exit: cleanup leads to the stopped state without a shell,
  automatic retry or RC execution. Check bounded shutdown and child reaping.
- [ ] Verify external-media recovery can access and repair the disposable system.

## Boot and service failure matrix — disposable VM

- [ ] Normal boot reaches multiuser with correct dependency ordering and no
  duplicate managers or daemons; missing/invalid configuration fails as designed.
- [ ] DHCP unavailable and route loss/recovery preserve documented service intent
  and local access; DNS or SSH failure remains visible and follows policy.
- [ ] Firewall initialization failure prevents dependent network activation.
- [ ] Service crashes exercise restart limits, hard dependencies and operator
  stop holds. Manager crashes produce controlled cleanup and stopped recovery.
- [ ] Complete at least 25 boot/shutdown cycles. Record readiness time, startup
  failures, leftover processes/resources, shutdown outcomes and filesystem
  errors. This original acceptance gate has not been demonstrated.

Do not inject destructive failures into Anvil's active installation. Traditional
RC is removed; its historical boot/recovery scenarios are no longer applicable.

## Broader validation

- [ ] Run the current native userspace regression suite as ordinary user and root,
  plus applicable ATF/Kyua tests; retain actual results and skips.
- [ ] Expand native sanitizer and resource-lifetime coverage for bindings,
  supervision and control protocol, including failure and cancellation paths.
- [ ] In isolation, test actual IPFW packets: allowed/blocked traffic, DNS policy,
  startup/failure behavior and policy replacement. Rule readback alone is not
  packet-level evidence.
- [ ] Audit current boot responsibilities: temporary-state cleanup, hostname,
  timezone, sysctl settings and devfs permissions. Record missing behavior as
  implementation work in the main guide.
- [ ] Check XDG ownership, modes and identity across supported account-creation
  paths and fresh logins; assess behavior after clearing volatile runtime state.
- [ ] Validate sleep/resume with graphics, input, networking and supervised services.

## Acceptance required after future implementation

These checks depend on changes that are not yet complete; listing them here does
not imply the functionality exists.

- [ ] Runtime-directory reconstruction/cleanup: recreate safely after volatile
  state loss, preserve isolation and reject links or unsafe ownership.
- [ ] Additional service integration: registration, dependencies, service-account
  identity, readiness, restart and shutdown (for example PostgreSQL).
- [ ] Complete descendant containment: detached/forking children are cleaned on
  stop/restart without affecting unrelated processes or reused identifiers.
- [ ] PAM replacement: console, SSH and `su` authentication, account restrictions,
  credentials/groups and session setup retain their intended behavior.
- [ ] Further source removals: native build/install and affected runtime coverage,
  with explicit compatibility and recovery implications.
