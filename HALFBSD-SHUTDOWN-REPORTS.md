# Shutdown evidence that survives reboot

October 21, 2026 — `halfbsd2`

No photographs, keyboard input during shutdown, or external console capture are
needed. The Lua supervisor and C PID 1 now save persistent evidence separately.
Use normal reboot/poweroff, then read the report after the next Lua boot.

## How to test on Anvil

1. Build/install the updated world using the normal procedure and reboot. This
   first reboot loads the new PID 1 and Lua code; the old running processes do
   not gain logging merely because installworld replaced their executable files.
2. Once that updated Lua boot reaches readiness, perform an ordinary reboot:
   `shutdown -r now`. Do not use quick/forced reboot options for this test.
3. After the next boot, become root and run:

   ```sh
   halfsvc shutdown-report > ~/shutdown-report.txt
   cat ~/shutdown-report.txt
   ```

4. Send that text. It is deliberately short. A nonzero command exit means the
   evidence is incomplete or unsuccessful; the summary explains why.
5. Repeat with normal poweroff (`shutdown -p now`), turn the machine back on,
   and read the report again.

Use the operating-system shutdown command, not `halfsvc shutdown`, which stops
the manager rather than requesting kernel poweroff. No failure injection is
required on Anvil. Stubborn-child tests use disposable private children in the
regression suite; kernel shutdown failure scenarios still belong in a VM.

## What is saved

All state lives in `/var/db/halfbsd/`, created root-owned with mode 0700. Files
are atomically replaced with mode 0600 and synchronized before publication is
considered successful. Snapshots replace fixed filenames rather than grow an
unbounded log.

| File | Contents |
| --- | --- |
| `boot.json` | Current recorded Lua supervisor identity, PID and start clocks |
| `boot-id` | Bounded diagnostic identity shared with PID 1; not an authentication token |
| `previous-boot.json` | Previous recorded Lua supervisor identity, archived when the next supervisor initializes reporting |
| `last-shutdown.json` | Lua cleanup phase, success flag, duration, reason, target, service states/results, forced stops, cleanup errors and ordered transitions |
| `shutdown-pid1.json` | PID 1 waiting/finished phase, wait result, supervisor exit code or terminating signal, errno and elapsed time |

The Lua file is saved at shutdown start, before deferred filesystem cleanup,
and after cleanup. The transition trace is capped at 256 entries; the report
counts omitted transitions. Full service-result summaries remain separate from
that trace. Messages are bounded and complete report size is capped at 1 MiB.
The records avoid storing service environments or arbitrary captured output
except the existing bounded first-startup-failure diagnostic.

The report command reads files directly and does not require a live manager
socket or network. Raw JSON remains available if a deeper investigation needs
the transition order:

```sh
cat /var/db/halfbsd/last-shutdown.json
cat /var/db/halfbsd/shutdown-pid1.json
```

## Interpreting the summary

`Shutdown evidence: CLEAN` requires all of these:

- The Lua report belongs to the previous recorded Lua supervisor, rather than
  being an older successful shutdown left on disk.
- Lua reached its final cleanup phase with `cleaned=true`.
- PID 1's report has the same identity and records a completed wait and normal
  supervisor exit with status zero.

A service may require TERM-to-KILL escalation and still be fully reaped within
policy. Those forced service stops appear explicitly in the summary. An overall
deadline remains a cleanup failure under existing behavior. If PID 1 observes
the supervisor itself dying by SIGKILL, the report shows signal 9 rather than
claiming a clean exit. The signal records what PID 1 observed, not proof of who
sent it.

`INCOMPLETE / NOT YET VERIFIED` is not automatically proof of a broken shutdown.
It can mean first use, logging/storage failure, an interrupted shutdown, an old
report, unsuccessful cleanup, a still-pending filesystem stage, or nonzero/
signal termination of the supervisor. Missing evidence never becomes success.
After a hard reset during a recorded Lua session, the next recorded boot's
identity differs from the old shutdown file, so stale success is rejected.

“Previous recorded Lua boot” is intentional: explicit RC recovery, early
single-stage diagnostics, and failures before writable reporting storage is
available do not initialize this reporter. The files cannot establish what
happened during an intervening unrecorded boot. They also cannot prove firmware
poweroff, physical disk durability under faulty hardware, or the integrity of
the filesystem after the kernel's final shutdown. Successful subsequent boot
and filesystem diagnostics remain complementary evidence.

## Behavior and storage safety

Only real PID 1 handoff sessions initialize persistent reporting, after storage
and the exclusive manager lock are ready. Ordinary userspace simulations,
dry-runs and test managers do not write host shutdown state. The new
`halfsvc shutdown-report` command is a read-only local operation; root is needed
to access the private directory. Remote-control authentication is unchanged.

Report files and directories must have trusted ownership and cannot be
group/world writable. Writers refuse symlink traversal. Storage device/inode
identity is checked between snapshots so a newly exposed underlying mount
cannot silently receive a final report intended for another filesystem.
No file/directory descriptor stays open across resource cleanup; logging must
not pin a mount and create an unmount failure itself.

Consequently, if this manager owns and unmounts the filesystem containing
`/var/db/halfbsd`, the final record may remain `filesystem_cleanup_pending`
and PID 1 may retain its `waiting` snapshot. That is incomplete evidence, not a
false clean result. The supported ZFS-root configuration generally leaves
datasets mounted by the existing `zfs mount -a` operation to later OS cleanup;
this reporting change neither adopts nor changes ownership of those mounts.

I/O failures produce diagnostics but do not cancel service cleanup, change
restart/shutdown ordering, or prevent PID 1 from applying its existing stop
deadline. Snapshot writes are synchronous filesystem operations, so an actual
filesystem I/O stall can still delay them, just as it can delay final sync.

## Validation performed

Passed on Linux:

- All Phase IV portable checks, including actual file writes, fsync/rename,
  private child reaping, forced termination and persistent PID 1 snapshots.
- Lua tests for bounded transitions, matching identities, next-boot retention,
  hard-reset/stale-success detection, cleanup escalation, read-only failures
  and changed storage identity.
- All 15 Phase II policy groups, including a reporting observer that throws:
  cleanup still completes and the callback runs only once.
- Strict GCC C17 compilation with pedantic/error, prototype, shadow, alignment
  and conversion checks. The real C reporter test links as PIE with text
  relocations rejected.
- Existing foundation archive/PIE build-setting regression checks.
- The isolated C reporter test also passes Linux AddressSanitizer and UBSan.
  Leak detection was disabled because LeakSanitizer cannot run under this
  workspace's tracing environment; native leak checking remains pending.

These tests never signal active host services or reboot the host. Native
FreeBSD world/build32, ATF and the first real reboot/poweroff reports remain
pending. Clang is unavailable locally. The new reporter source uses checked
POSIX file/time APIs and retains the current init state-machine algorithms.

Native regression entry remains:

```sh
sh /usr/src/tests/halfbsd/phase4/run.sh
```

Then use the normal reboot/poweroff sequence above. There is no need to try to
capture the console while devices and sessions are being shut down.
