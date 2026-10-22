# HalfBSD device-event action integration

## Confirmed native result and remaining errors

After the last installed kernel/world, the administrator reported successful
boot and immediate console autologin. The October 21 photograph shows hms0
attaching to the Razer Naga Hex after reconnecting the USB switch. The earlier
manual hms load had already restored repeated mouse-switch recovery.

The photograph also shows repeated `Usage in Lua mode: service ...` and an
unsupported-action error. These originate from devd's existing RC-style actions,
not the service scheduler's progress logging. Generic unmatched devices invoked
`service devmatch quietstart`; network links invoked `service dhclient quietstart`;
keyboard events invoked `service vtcons setkeyboard`. The restricted service
frontend correctly refused these unsupported RC operations.

## Changes

Devd recognizes the exact shipped action templates before shell expansion and
checks PID 1's `halfbsd.boot_active` ownership using the native kenv declaration.
This also handles existing installed devd configuration files without requiring
etcupdate to replace them. Modified/custom actions are not rewritten.

* Under traditional RC ownership, these actions retain their original execution.
* Under HalfBSD ownership, module-match events directly execute
  `/usr/sbin/halfinit --device-nomatch EVENT`. EVENT is one raw argument, never a
  shell command. This internal entry executes one operation; it does not load the
  service registry, acquire the manager lock or launch a second supervisor.
* Legacy DHCP start actions are not executed. The existing scheduler owns the
  foreground dhclient processes. Their restart policy is now `always`, so a
  successful exit on link loss is also recovered using the existing bounded
  retry/backoff mechanism. Operator holds, shutdown and retry exhaustion remain
  effective; events cannot bypass them or select an unconfigured interface.
* Legacy vtcons keyboard switching is not executed. The supported HALFBSD kernel
  has kbdmux, and kbd_register attaches registered keyboards to that multiplexer.
  Hotplug does not need to run the broader RC console configuration script.
* The shipped power-profile and Bluetooth RC action templates currently have no
  corresponding HalfBSD provider. Each category emits one explicit warning per
  devd lifetime instead of repeatedly invoking the incompatible service frontend.
  This is a documented missing capability, not a successful no-op migration.
* Missing or invalid startup ownership refuses recognized actions. There is no
  implicit RC fallback. Unrelated administrator actions keep their existing
  behavior, including their existing shell execution semantics.

The bounded module-match handler requires root and HalfBSD startup ownership.
It reads one kernel-environment snapshot, honors boot_safe and the loader/kernel
`devmatch_blocklist`, validates/deduplicates all module names before loading, and
skips already loaded modules. It uses direct argv calls to the existing devmatch,
kldstat and kldload utilities, with per-command output/deadline limits, a 15-second
userland operation deadline and at most 32 module candidates. Kernel calls can
still impose kernel-side latency; this is not a hard real-time guarantee.

No rc.conf code is evaluated. Legacy shell-defined devmatch blocklists are not
imported; use the kernel/loader setting for this path. The handler does not
freeze the global device tree: modules load individually and normal kernel
attachment proceeds. Built-in hms does not depend on successful autoload.

## Subscriber warning

Previously any failed devd subscriber send printed “dropping unresponsive
client,” including ordinary peer closure. Devd now retries EINTR, uses
MSG_NOSIGNAL, and treats EPIPE/ECONNRESET/ENOTCONN as debug-level disconnections.
Slow subscribers, partial delivery and other errors remain warnings; the message
now includes the error reason. Each failed subscriber is removed exactly once.
This does not identify which historical subscriber caused the photographed
warning, and it does not weaken the resource-abuse cleanup policy.

## Validation and next native checks

Linux validation passed:

* Phase IV portable gate, including new module-match and action tests.
* All 21 Phase III policy groups, including DHCP restart policy selection.
* All 16 Phase II policy groups, including successful-exit recovery, retry bounds
  and cancellation by operator stop.
* The actual devd action and notification function bodies compiled as strict
  C++17 with pedantic warnings, Werror and PIE/text-relocation rejection, using
  injected config/kenv/exec/send results. Tests cover explicit RC selection,
  missing/invalid ownership, literal raw event arguments, failed matching,
  orderly/slow subscribers, short sends, EINTR and single cleanup.
* The action classifier compiled under strict C17. The new kenv call signature
  also compiled against the actual repository headers in a freestanding check.
* Effective BSD make MODULES includes the new installed devmatch.lua.

These extracted-function tests do not establish the FreeBSD ABI, daemon linkage
or native kernel behavior. Clang is unavailable in this Linux workspace.
Native buildworld/buildkernel, ATF, live autoload, DHCP link-loss recovery and USB
switch validation remain pending. Existing full userspace suites can be run with
`sh /usr/src/tests/halfbsd/phase4/run.sh` after installation.

On Anvil, rebuild/install world, reboot, and switch USB away/back repeatedly.
Verify keyboard/mouse recovery and absence of service usage spam. Use
`halfsvc status devd`, `halfsvc status seatd`, and `halfsvc status` to inspect
ownership. Test DHCP interruption only with local console recovery available;
this workspace has not changed Anvil's active network or firewall.

The new screenshot is an untracked diagnostic input and must not be committed.
