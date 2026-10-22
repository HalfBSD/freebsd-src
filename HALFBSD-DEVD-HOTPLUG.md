# Mandatory device-event service

The administrator confirmed that starting the existing base `devd` and then
launching Sway restores mouse and keyboard operation across USB-switch detach
and reattach cycles. No explicit `hms` load was needed for that successful test.
Earlier speculation about a missing mouse driver is therefore not the basis
of this change. Kernel logs already showed keyboard reattachment and new evdev
nodes; the missing Lua-started device-event service was the actionable gap.

Lua multiuser startup now requires `/sbin/devd -d`. The `-d` flag keeps this
existing daemon in the foreground so halfinit retains process ownership.
Readiness uses a connection to `/var/run/devd.pipe`, which devd creates after
parsing its configuration and opening the kernel device-event source. This
establishes an available event endpoint, not proof that every consumer has
processed every pending event.

The dependency graph orders runtime directories, shared-library setup and the
firewall before devd, then devd before seatd. Multiuser readiness requires both.
This keeps existing devd actions behind firewall initialization. The existing
exclusive-process guards reject a running devd instead of adopting it or
starting a duplicate. Restart-on-failure and reverse-dependency shutdown use
the existing supervisor. There is no new enable flag or RC invocation.

Single-user diagnostics and the network-only checkpoint do not activate devd.
Traditional RC recovery retains its existing devd startup. No kernel driver,
input permissions, recipient mask, seatd implementation, or devd configuration
has been changed. devd still executes its existing configured actions, including
legacy shell/RC actions where configured; this change does not migrate those
actions to Lua or claim complete networking hotplug ownership.

## Validation

Linux portable checks passed: all 21 Phase III policy groups, all 14 Phase IV
policy groups, diagnostic-stage checks, and the compiled PID 1 handoff tests.
New graph assertions cover the foreground command, event-socket readiness,
duplicate-process guards, mandatory multiuser selection, firewall/seat ordering,
and exclusion from diagnostic targets. The installed default graph test now
requires devd and seatd. No C source or native build rules changed.

The administrator's manual test validates the existing native daemon and USB
hotplug behavior. Automatic startup under this revised graph, native regression
suites, reboot and shutdown remain to be verified on Anvil.

## After installation

Reboot normally into Lua multiuser. Do not also run `service devd start`:

```sh
halfsvc status devd
halfsvc status seatd
pgrep -laf devd
/usr/bin/lua -E /usr/src/tests/halfbsd/phase4/check-live.lua
```

Expect devd and seatd READY, with one foreground `/sbin/devd -d`. Launch Sway
as the regular user and repeat USB-switch cycles, verifying mouse and keyboard
each time. Verify orderly reboot afterward. Existing manually launched devd
must be stopped before any attempted manager activation on the same boot;
normal reboot avoids overlapping ownership.

## October 21: confirmed mouse driver and built-in kernel support

The administrator subsequently confirmed keyboard recovery across repeated USB
switch cycles, but mouse recovery still failed. `kldstat -v` showed HID transport
and keyboard drivers without hms. Loading `hms` restored mouse recovery across
switch cycles on Anvil. This is native evidence for the missing mouse driver;
re-enumerated event node numbers alone were not evidence of a device identity
bug. The administrator also observed the symptom on stock FreeBSD.

HALFBSD's AMD64 kernel configuration now includes `device hms`, so the mouse
driver is available without module autoload or an explicit boot-time kldload.
The existing hid, hidbus and evdev devices supply its other dependencies;
sys/conf/files includes hidmap.c automatically when hms is selected.
No new module-loading service, loader setting or console mouse daemon is needed.
Hms delivers evdev mouse events used by Wayland.

Source dependency checks passed in the Linux workspace. A native kernel build
and a reboot into that new kernel remain pending. The successful Anvil test
used the loadable module, not the newly configured built-in driver. After
installing the rebuilt HALFBSD kernel, verify `kldstat -v` lists hidbus/hms,
inspect `dmesg` for mouse attachment, and repeat USB switch cycles in Sway
without manually loading hms. Keep mandatory devd and seatd running.

The separate Lua-mode devmatch RC-action limitation remains an autoload audit
item for other drivers; building hms in removes that dependency for this mouse.
