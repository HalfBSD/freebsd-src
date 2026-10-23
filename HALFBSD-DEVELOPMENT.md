# HalfBSD development and validation

Current reference: October 22, 2026. Engineering requirements remain in
[AGENTS.md](AGENTS.md). This replaces historical build-failure and Lua phase logs.

## Runtime and service contracts

Use Lua 5.4.8 directly from `contrib/lua/src`; do not vendor another runtime.
`usr.bin/lua` is ordinary userland; `stand/liblua` is the restricted loader build.
`libhalfproc` owns pdfork/process descriptors, signals and reaping. `libhalfbsd`
exports proc, event, filesystem, IPC, inventory and logging bindings. Lua policy
lives in `usr.sbin/halfinit/modules`; native launchers preload the existing APIs.

Detailed public contracts and working examples already live alongside source:

- [Native Lua API, ownership and errors](usr.bin/lua/lua-halfbsd.7).
- [Service schema, readiness and control](usr.sbin/halfinit/halfinit.8).
- [Control utility](usr.sbin/halfsvc/halfsvc.8).
- [Console login](usr.sbin/halfconsole/halfconsole.8).
- [Foreground, pipe and filesystem examples](share/examples/halfbsd).
- [Native embedding/resource-lifetime example](tests/halfbsd/ownership_helper.c).

Execution uses absolute argv without an implicit shell. Successful exec is not
application readiness. `requires` activates and gates on READY; `wants` tolerates
failure; `after`/`before` order selected services without activating them. Cycles
are rejected. Restarts have finite windows and capped backoff; shutdown follows
reverse dependency waves with TERM/KILL deadlines and explicit cleanup.

Processes are owned through descriptors. Never signal recycled process-group
IDs after their leaders are reaped; detached/escaped descendants are not fully
contained. Call explicit shutdown before destroying an orchestrator. Lua code
is trusted code, not an OS sandbox. Peer credentials, not JSON fields, authorize
control mutations. Errors retain errno and distinguish exec failure, nonzero
exit, signal termination and timeout. API details belong in the references above.

For embedding, use `halfbsd_openlibs` and protected Lua calls. Installed linkage:
`cc -I/usr/include/lua app.c -lhalfbsd -lhalfproc -llua -lm -lpthread -Wl,-E`.
Base Makefiles use `LIBADD=halfbsd halfproc lua pthread m` and source includes.

## Build discipline

Use matching kernel/world and the normal native HalfBSD upgrade procedure.
AMD64 buildworld includes 32-bit compatibility work. Full native builds and tests
cannot be inferred from Linux checks or header compilation.

Keep bootstrap and PIE Lua archive selection. Foundation static archives contain
PIC objects even with `NO_PIC`: that setting suppresses shared targets, not
position independence. Preserve hardening and reject text relocations. Check
staged SYSROOT, inherited SHLIB_MAJOR, empty DESTDIR, and repeated parallel
header/configuration installation; Makefile parsing alone is insufficient.

Prefer strict C17 plus `-Wpedantic -Werror` for new portable code. Match imported
sources' intended dialect in production Makefiles. Both seatd/libseat explicitly
select `CSTD=c17`; strict-only smoke tests previously missed GNU17's legacy
`unix` macro collision. Inspect effective CSTD/CFLAGS and actual predefinitions.

GNU17 is C17 with GNU extensions, not a newer C standard. Low-level type-generic
macros, statement expressions, inline assembly and compiler attributes can make
systems code practical and preserve mature implementations. Some extensions also
work under strict modes with reserved spellings. POSIX/FreeBSD APIs are not GNU
language extensions. Retain GNU mode where needed; a global C17 conversion needs
its own audit rather than a blanket switch based on one imported-code failure.

Native declarations and asynchronous lifetime rules matter: use owned syscall
headers, writable kenv buffers with NUL-inclusive lengths, aligned sockaddr
copies, correct Lua stack indices, and sockets retained through EINPROGRESS.
Explicit rw/noro mount updates require kernel readback. These lessons and
strict warning requirements are maintained in AGENTS.md, not duplicated here.

## Regression entry points

On native FreeBSD, ordinary user and root runs use disposable fixtures:

```sh
sh /usr/src/tests/halfbsd/phase4/run.sh
```

It includes earlier phase suites. Phase I has nine supervision scenarios; native
Phase II/III/IV userspace transcripts passed as regular user and root, including
cross-UID authorization, 50 concurrent processes, 100 randomized dependency tasks
and 1,000 lifecycle operations. This is not evidence that every broader ATF,
credential or native sanitizer case was run. With installed tests, also run
`kyua test -k /usr/tests/halfbsd/Kyuafile` and retain its actual results.

Useful targeted source-tree checks:

```sh
sh tests/halfbsd/accounts/run.sh
sh tests/halfbsd/console/run.sh
python3 tests/halfbsd/console/headers.py
python3 tests/halfbsd/console/user_headers.py
python3 tools/build/tests/halfbsd-init-headers.py
sh tests/halfbsd/seatd/run.sh              # ordinary user; private socket/console fixture
sh tests/halfbsd/seatd/build.sh "$PWD" bmake
```

Linux portable policy gate:
`HALFBSD_SOURCE="$PWD" HALFBSD_LUA=/path/to/in-tree-lua sh tests/halfbsd/phase4/run.sh --portable-only`.
Use an interpreter built from this checkout. No downloaded interpreter or ports
runtime is required. Linux syscall mocks/stubs never establish FreeBSD behavior.

## Latest console reduction

Commit `0409b33055b2` removes the PID 1 shell and kernel/seatd VT switching,
roughly 1,400 net implementation lines. Shared framebuffer, keyboard and graphics
handoff remain. Inert reserved structure slots preserve pre-change DRM sizes and
retained offsets; no extra-window allocation or switching implementation remains.
The layout fixture records baseline `8b28f7bbbf46` for AMD64/i386, with/without
cut/paste. Read-only identity queries remain for console consumers.

Current Linux results: Phase II 16 groups, Phase III 21 groups, Phase IV portable
gate, real extracted stopped-state/handoff helpers with faults, private seatd
static/shared/PIE protocol tests and repeated libseat staging pass. Actual FreeBSD
headers compile init in C17/GNU17 and console/logo sources in kernel-module context
for AMD64/i386. Existing state-cast/%m and kernel parameter/signedness exceptions
are explicit in the check scripts. Clang was unavailable. These are unlinked
host-compiler objects/header checks, not native world/kernel or hardware tests.

After native installation, an optional non-graphics ioctl check is:

```sh
cc -std=c17 -Wall -Wextra -Wpedantic -Werror tests/halfbsd/console/native.c -o /tmp/halfbsd-console-check
/tmp/halfbsd-console-check
```

It skips old kernels unless the new read-only `kern.vt.console_count=1` marker
exists, then checks absence of extra ttyv devices and rejection of removed switch
ioctls. It does not change graphics/keyboard modes. Still test real Sway start/exit,
console restoration, USB reconnection and orderly reboot on Anvil.

Native buildworld/buildkernel, current build32/ATF and hardware acceptance of this
latest reduction remain pending. The working pre-reduction Anvil results are
recorded in [HALFBSD.md](HALFBSD.md), separately from this new change.
