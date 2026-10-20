# HalfBSD: Lua → ZFS → AMDGPU → single-user shell

October 20, 2026. Branch: `halfbsd2`.

This is the next diagnostic boot checkpoint. It replaces the full multiuser
startup attempt with a small, serial Lua task sequence and visible console output.
**The new path has not yet been booted on Anvil or in a FreeBSD VM.**

## What this boot does

1. The kernel mounts the root filesystem and PID 1 performs its existing devfs setup.
2. C `/sbin/init` announces its diagnostic build and selected startup mode.
3. PID 1 starts the native `halfinit` launcher, which announces Lua initialization.
4. Lua verifies the kernel-mounted ZFS root, devfs and console.
5. Lua runs `/sbin/zfs mount -a` for already-imported pools and lists ZFS mounts.
6. Lua runs `/sbin/kldload -n -v amdgpu`, then `/sbin/kldstat -n amdgpu.ko`.
7. Lua lists DRM device nodes and requires a `cardN` device.
8. Lua sends the distinct `SINGLE` message and exits. PID 1 verifies both that
   message and successful process exit, then opens its existing single-user shell.

There is no DHCP, networking setup, firewall rule loader, DNS, SSH, NTP, syslog,
getty, compositor, swap activation or full service-manager control socket.
Kernel IPFW policy is unchanged. **SSH will not be available: use the physical
console.** `/etc/halfbsd/system.lua` and persisted enable/disable state are not
loaded by this diagnostic path, so an old multiuser configuration cannot enable
services accidentally.

This does not import pools, unlock encrypted storage, repair filesystems or
remount root writable. ZFS datasets mounted by this stage and the loaded AMDGPU
module are retained for inspection. The graphics module and firmware must already
be installed and compatible with the running kernel; this code does not install
DRM packages or firmware.

A loaded module and a DRM card are bring-up evidence, **not proof of accelerated
rendering, a working compositor, or which driver owns every device node**. Read
the kernel's AMDGPU messages and verify that the physical console remains usable.

## Before the test

- Keep a known-good ZFS boot environment and recovery console access. For example,
  use `bectl create pre-lua-single` before installing the experimental world.
- Build/install the updated world and the matching supported kernel using your
  normal native procedure. Local Linux checks are not a substitute for that build.
- Confirm that your AMDGPU module/firmware matches the installed kernel.
- Optionally run the safe userspace suite after installation, as your user and root:

  ```sh
  sh /usr/src/tests/halfbsd/phase4/run.sh
  ```

These tests do not actually load AMDGPU, mount datasets or switch the active boot.
Do not launch `halfinit --boot-single` from the running RC system; the entry
requires root directly launched by PID 1 and its private notification channel.

## Select the test at the loader prompt

The source now defaults to Lua's single-user diagnostic stage. To remove doubt
about a previously configured RC override, explicitly select both values:

```text
set halfbsd.boot_mode=lua
set halfbsd.boot_stage=single
boot
```

Use **plain `boot`**, not `boot -s`. `boot -s` requests the existing direct PID 1
single-user shell before Lua startup; reaching that shell would not test the Lua
handoff. Loader-prompt settings are for this boot; persistent `loader.conf`
settings and boot-environment selection should be reviewed separately.

## Messages to look for

PID 1 prints to `/dev/console` because its own standard descriptors are closed.
The launcher/Lua output is unbuffered. Native ZFS/kldload/kldstat stdout and stderr
also go directly to the console. Kernel graphics output can appear between lines.

The important messages, in approximate order, are:

```text
[HalfBSD init] PID 1 diagnostic build 2026-10-20 started; default Lua single-user stage
[HalfBSD init] Lua selected (no automatic RC fallback)
[HalfBSD init] initial devfs setup finished; entering startup state machine
[HalfBSD init] launching Lua SINGLE-USER diagnostic stage
[HalfBSD init] exec /usr/sbin/halfinit; console attached, notification fd=3
[halfinit launcher] executable entered; allocating Lua state
[halfinit launcher] opening native Lua libraries
[halfinit launcher] loading /usr/lib/halfbsd/init.lua
[halfinit single] Lua entered diagnostic single-user boot; networking and multiuser services are disabled
[halfinit single] BEGIN verify kernel ZFS root and devfs
[halfinit single] OK kernel ZFS root: ...; readonly=...
[halfinit single] OK /dev/console is present; kernel console output is active
[halfinit single] BEGIN mount datasets from already-imported ZFS pools: /sbin/zfs mount -a
[halfinit single] OK mount datasets from already-imported ZFS pools
[halfinit single] ZFS ... on ...
[halfinit single] BEGIN load AMDGPU (or confirm already loaded): /sbin/kldload -n -v amdgpu
[halfinit single] OK load AMDGPU (or confirm already loaded)
[halfinit single] BEGIN show loaded AMDGPU module: /sbin/kldstat -n amdgpu.ko
[halfinit single] OK show loaded AMDGPU module
[halfinit single] DRM device nodes: card0, renderD128
[halfinit single] OK diagnostic tasks finished; requesting PID 1 single-user shell (not multiuser)
[HalfBSD init] SINGLE READY received and Lua exited successfully; handing console to shell
[HalfBSD init] Lua diagnostic stage completed; opening PID 1 single-user shell
[HalfBSD init] Lua ZFS/AMDGPU diagnostic completed; this is the single-user shell handoff.
```

Dataset names, root read-only state, module output and DRM node numbers vary.
PID 1's `waiting for Lua boot: N seconds elapsed` messages may interleave with
these lines every five seconds. They indicate that PID 1 is still waiting for
Lua; they do not mean a task has succeeded.

The normal existing prompt may then appear:

```text
Enter full pathname of shell or RETURN for /bin/sh:
```

Press **RETURN**. Existing single-user console password policy is retained.

## Verify success at the shell

```sh
kenv halfbsd.boot_active
kenv halfbsd.boot_stage
kenv halfbsd.boot_result
mount -t zfs
kldstat -n amdgpu.ko
ls /dev/dri
```

The three kernel variables should print:

```text
lua
single
single-ready
```

`single-ready` is only published after the explicit successful Lua checkpoint.
A shell alone is not proof: a failed stage also opens a recovery shell.

The diagnostic Lua process intentionally exits before the shell starts.
`pgrep halfinit` finding no persistent supervisor is therefore expected, as is
`halfsvc` finding no control socket. Those checks were appropriate for full
multiuser supervision, not this checkpoint.

Exiting the diagnostic shell or pressing Ctrl-D keeps the system in single-user
handling. It does not start networking or rerun the ZFS/graphics stage. Reboot
when changing the test stage or choosing RC. Normal reboot/halt/poweroff still
use the existing PID 1 mechanisms; no RC shutdown scripts are run in Lua mode.

## If it stops or fails

Record the last complete `BEGIN`, `OK` or `FAILED` line, elapsed wait messages and
any kernel messages around the failure. A phone photo of the console is useful
when the graphics driver itself disrupts the display.

| Last observation | What it establishes / where to look |
|---|---|
| No new diagnostic-build banner | Does not establish that this `init` ran. Check installed world, selected boot environment, init path and console. Loader/kernel failures occur before these prints. |
| `explicit halfbsd.boot_mode=rc override detected` | RC was explicitly selected. This is not a reset-triggered fallback. Check the loader setting. |
| `exec /usr/sbin/halfinit` with no launcher message | Inspect executable availability, runtime linker/libraries and native execution errors. |
| `opening native Lua libraries` is the last launcher message | Localizes the stop to native library initialization before the Lua manager entry. |
| `loading .../init.lua` but no Lua single-stage line | Inspect installed Lua modules and the error printed by the launcher. |
| ZFS `BEGIN` without its `OK` | Dataset mounting is pending or failed. Native command errors are on the console. |
| AMDGPU `BEGIN` followed by a blank screen | May be a modeset or kernel/firmware/driver problem. Module/kernel ABI and kernel graphics messages matter; a blank screen is not a successful shell handoff. |
| Module loaded, but no DRM card | The task reports failure rather than claiming graphics attachment succeeded. |
| `FAILED ...` or `boot_result=failed` | A recovery shell is expected. This is not successful diagnostic acceptance. |
| `SINGLE READY` plus `single-ready` and an interactive shell | Confirms the new Lua single-stage handoff, subject to the graphics limitations above. |

ZFS mounting and AMDGPU loading each have a 120-second process timeout; module
listing has ten seconds. PID 1 retains an outer 600-second wait and reports its
progress. These are userspace bounds: they cannot guarantee recovery from an
uninterruptible kernel/graphics hang. A failed Lua process is terminated/reaped
before recovery; PID 1 announces that wait too.

There is **no automatic switch to RC** after a failure or hard reset. To recover,
reboot and select at the loader prompt:

```text
set halfbsd.boot_mode=rc
boot
```

Use the known-good boot environment if the installed binaries cannot execute.
The earlier observed RC boot after a hard reset remains unexplained; the actual
boot environment, init binary and overrides from that attempt were not supplied.

## Later checkpoints

The previous full Lua graph is retained, but it is no longer the default stage.
After this checkpoint works, we can add back one dependency group at a time.
Do not set the following merely to get past a failure:

```text
set halfbsd.boot_mode=lua
set halfbsd.boot_stage=multiuser
boot
```

That explicitly restores the prior full multiuser attempt, including networking.
It requires a valid `/etc/halfbsd/system.lua` and its full boot prerequisites.

## Validation recorded for this change

- Passed actual Linux portable C tests for default/explicit stage selection,
  fragmented `READY`/`SINGLE`, malformed/EOF messages, immediate trace output,
  errno preservation, descriptor/session handling and private-child reaping.
- Passed single-stage Lua fixture tests for command ordering, console redirection,
  deadlines, ZFS/AMDGPU/exec failures, missing DRM card, notification cleanup and
  rejection of non-root/non-PID-1 callers before mutations.
- Passed existing 11 Phase IV, 16 Phase III and 14 Phase II pure policy groups.
- Passed strict GCC C17, `-Wpedantic -Werror` compilation of the changed portable
  helper and native launcher. Clang is unavailable here.
- Passed portable-helper ASan/UBSan with leak detection disabled under tracing.
- Passed staged SYSROOT/PIE/archive queries, fresh/repeated parallel module/example
  installation, configuration preservation, public-header installation and
  inherited ABI/archive selection checks.
- Native full `init`, `buildworld`, `buildkernel`, build32, ATF/native regression
  reruns, real ZFS/AMDGPU activation and actual console shell handoff remain pending.

Only in-tree Lua/bmake were built under `/tmp/halfbsd-single-check`; no tools were
downloaded. No changes have been applied remotely to Anvil. See
[HALFBSD-LUA-PHASE4.txt](HALFBSD-LUA-PHASE4.txt) for the longer historical handoff.
