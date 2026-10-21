# Workstation follow-up: console, services, profiles and acceptance

October 21, 2026 — `halfbsd2`

This batch implements the remaining workstation policy from the six-item
agenda. It retains RC and the existing native recovery shell. No live boot,
shutdown or desktop test has been performed from this Linux workspace.

## Current evidence

The administrator has confirmed Lua multiuser readiness, console and SSH use,
ordinary DNS resolution, automatic seatd startup, and Sway graphics/keyboard.
Starting existing base devd manually restored mouse and keyboard operation
across USB-switch cycles, without an explicit hms load. Mandatory supervised
devd startup was implemented in the preceding commit. Automatic devd under the
new graph and this batch still need native acceptance.

## Console experience

PID 1 substitutes `/usr/sbin/halfconsole` for `/etc/rc.console` in Lua mode.
It uses the same controlling-terminal preparation, then execs native `login`
directly: no shell evaluation or extra process supervisor. Other terminal
entries, simultaneous sessions and SSH authentication keep their existing
behavior.

The administrator-owned `/etc/halfbsd/system.lua` accepts:

```lua
console={user='root',autologin=true,select_timeout=3},
```

These are also the defaults. Use `user='charlie'` for that existing account.
Fresh installs contain the explicit settings; upgrades preserve the existing
file. At the console expect:

```text
HalfBSD console: automatic login as root in 3 seconds.
Enter another account, or 'login' for authenticated login:
```

Type an account and Return to select it without a password, or `login` and
Return for authenticated login. An empty line selects the configured account.
Timeout selects it too. `select_timeout=0` skips the delay;
`autologin=false` uses authenticated login immediately. Logging out returns to
the selector on PID 1's next session. This deliberately grants passwordless
access on that designated physical console, including root by default. It does
not authorize passwordless SSH or launch a compositor as root.

With no console table, the helper recognizes literal `console_user=...` settings
in trusted `/etc/rc.conf` and `/etc/rc.conf.local`, in that order. It emits a
migration warning and does not rewrite either file. Copy the desired account
into the Lua table to finish migration. It never evaluates shell substitutions,
functions, sourced files or computed values. Unsupported expressions, unsafe
legacy files, missing accounts, non-login shells and selection errors lead to
authenticated login. Invalid configuration rejected earlier by halfinit enters
the existing boot recovery path. No invalid named account silently becomes root.

## Service control

`service` now consults PID 1's `halfbsd.boot_active` marker before loading any RC
configuration. In Lua mode these commands dispatch to authenticated halfsvc:

```sh
service -l
service seatd status
service devd status
service sshd restart
service local_unbound status
```

Supported actions are start, stop, restart, status, enable and disable.
`local_unbound`, `local-unbound` and `unbound` map to the Lua `dns` service.
Use halfsvc directly for graph, reload and alternate control sockets.
Missing control sockets, unknown services, unsupported RC options/actions and
missing/invalid boot ownership produce errors, never an RC-started duplicate.
The manager still enforces peer-credential authorization. Mandatory target
members remain required; disabling one can prevent a complete boot and should
not be used to configure an ordinary workstation.

In explicitly active RC mode the previous RC path remains. Lua mode does not
automatically run unregistered ports scripts; add a proper service definition.
RC-specific switches, including jail dispatch, are not emulated by this Lua
frontend. Explicit RC mode retains its original options.

## Explicit startup profiles

The top-level `profile` in `system.lua` selects an existing dependency-graph
target. There is no second scheduler or new loader spelling.

| Profile | Service selection |
| --- | --- |
| `workstation` (default) | Existing configured multiuser/desktop target, including mandatory devd and seatd |
| `network` | Network checkpoint graph; no DNS, SSH, NTP, devd, seatd or custom service files |
| `maintenance` | Filesystems/ZFS, optional configured graphics, runtime, loopback, firewall, configured swap and library hints; no DHCP or network/desktop daemons |

Select `profile='maintenance'` for a fresh boot into the offline-service set.
No Ethernet adapter is required for that profile. Diagnostic profiles discard
custom service includes and saved enable preferences, fix their target and
restrict runtime control to read-only requests. They cannot accidentally start
a saved daemon just because it was enabled during a workstation session.
Configuration profiles enter normal configured console sessions after readiness;
they are not a live transition that ejects existing users or clears inherited
kernel interface addresses. They select services, not a separate authentication
or packet-isolation policy. The existing configured IPFW policy still applies.

Loader `halfbsd.boot_stage=single` and `halfbsd.boot_stage=network` remain
unchanged independent diagnostic entries; the latter opens its existing root
inspection shell. `halfbsd.boot_stage=multiuser` remains the default entry for
loading system.lua. `halfbsd.boot_state` is still unsupported. Recovery remains:

```text
set halfbsd.boot_mode=rc
boot
```

Native single-user recovery remains available even when Lua cannot load.
Neither recovery mechanism has been removed or replaced with a Lua profile.

## Tests and build limits

Passed locally on Linux:

- Phase II: 14 policy groups, including simulated concurrency/restart/shutdown.
- Phase III: 21 policy groups, including mandatory devd ordering/readiness.
- Phase IV: 14 policy groups and diagnostic/PID 1 handoff checks.
- New console policy tests: root/named/default/authenticated selection, invalid
  accounts, literal-only legacy migration and unsafe shell expression rejection.
- New profile tests: offline graph without Ethernet discovery, network graph,
  custom-service exclusion and invalid profile rejection.
- Real private PTY C test: complete input, bounded input, timeout, partial-line
  discard, EINTR recovery and disconnect; linked as PIE with text-relocation
  rejection. This tests terminal input, not FreeBSD login/PAM behavior.
- Real service frontend with private command paths: Lua dispatch, aliases,
  errors, no RC fallback, and explicit RC selection.
- Strict GCC C17 compilation of both helper sources with pedantic/error and
  additional prototype/shadow/alignment/conversion diagnostics.
- BSD make queries with staged SYSROOT/inherited SHLIB_MAJOR: amd64/i386 helper
  PIE flags and existing native/Lua PIE archive selection remain intact.
- Actual configuration publication into an empty DESTDIR and parallel repeated
  publication preserve administrator console/profile settings byte-for-byte.

Clang was unavailable. Native buildworld/buildkernel/build32, ATF, full helper
link/install and actual console/PAM/boot tests remain pending. The native
Phase IV runner now includes a full halfconsole build and repeated private
DESTDIR install using the completed buildworld's actual native/PIE archives;
set HALFBSD_OBJTOP if their location differs from the standard amd64 layout.
It never replaces the
host executable during that test. No stubs establish native validation.
No new external tools or runtimes were downloaded; local checks reused the
existing temporary builds of in-tree Lua and BSD make.

## One native acceptance batch

After building/installing world and kernel, reboot once with console access.
Then run the existing regression gate as the regular user and root:

```sh
sh /usr/src/tests/halfbsd/phase4/run.sh
```

The gate includes Phase I–III native regressions. Separately run the opt-in
live collector with a new output directory and independently verified signed
and intentionally bogus DNS names:

```text
sh /usr/src/tests/halfbsd/phase4/workstation-live.sh --live NEW-DIRECTORY SIGNED-NAME BOGUS-NAME
```

Run the live collector as root for Unbound statistics access. It records live
service/listener status, frontend status, process/session inventory, UDP/TCP
DNSSEC AD responses, bogus-answer SERVFAIL, aggregate cache-hit growth across
a repeated query and NTP peer selection. Aggregate counters can include other
clients; existing isolated Unbound regressions provide stronger cache attribution.
It only queries services and writes its report directory. An NTP
process without a selected peer is reported as not yet synchronized. It does
not stop services or reboot, and it does not validate fixture-domain ownership
or authoritative correctness for you.

Complete the manual gates in the same boot:

1. Confirm configured autologin, explicit alternate selection and authenticated
   login; check `id`, home, terminal and logout/relogin. Test invalid/missing
   account policy from a recoverable configuration. A default root shell must
   not be used to start Sway.
2. As the regular desktop user, launch Sway; exercise mouse/keyboard and repeated
   USB-switch cycles, VT switching, exit/relaunch and concurrent SSH sessions.
3. Verify exactly one devd, seatd and each supervised daemon. Capture halfsvc
   status and boot logs, not only the READY banner.
4. Perform orderly reboot and poweroff after loading the updated code; read
   `halfsvc shutdown-report` as root after the next boot and check for filesystem
   errors. Persistent reporting replaces the need to capture console output;
   see [HALFBSD-SHUTDOWN-REPORTS.md](HALFBSD-SHUTDOWN-REPORTS.md).
5. Test explicit RC and native recovery from the loader. Run repeated boot cycles
   using the existing disposable-VM harness before claiming long-term coverage.

Do not treat this implementation as completed native acceptance. Record each
outcome independently, including failures, in the handoff.
