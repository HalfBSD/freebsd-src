# HalfBSD engineering notes

For C and build-system changes, use the applicable checks below before reporting
success. These notes record failures found during the Lua foundation work.

- Use the repository warning settings and strict C17 checks with `-Wpedantic`
  and `-Werror`. Run Clang when available; GCC alone missed native Clang
  indentation diagnostics. Keep labels before statements, and make nested
  control-flow scopes clear with braces and separate statements.
- Match imported code's language dialect in the production makefiles. GNU modes
  expose legacy macros such as unix that strict C hides. Compile using effective
  BSD make CSTD/CFLAGS as well as strict smoke-test flags; otherwise portable
  strict-only tests can miss collisions in buildworld's default GNU dialect.
- Account for buildworld's 32-bit compatibility pass, even on AMD64. Match
  counter bounds to their actual types; guard against wrap and reused IDs.
  A freestanding width check is useful but does not prove a full build32.
- Test generated build targets and flags with a staged SYSROOT and inherited
  settings such as SHLIB_MAJOR. Ordinary Makefile parsing is insufficient.
  Preserve the existing Lua bootstrap archive and PIE archive selection.
- Link the complete executable and its actual archives as PIE, rejecting text
  relocations. Static archives are not automatically position independent;
  bsd.lib.mk's NO_PIC controls shared targets and does not supply PIC objects.
  Keep hardening enabled rather than suppressing linker diagnostics.
- Inspect effective compiler definitions alongside shared configuration headers;
  avoid duplicate command-line definitions of Lua configuration macros.
- Check native syscall declarations in the repository headers, including buffer
  qualifiers and length conventions. FreeBSD kenv() takes a writable char * even
  for KENV_SET; use writable storage and include the terminating NUL in its size.
  Compile new call sites against those declarations rather than permissive stubs.
- Include the header that owns each network type explicitly: sockaddr_in needs
  netinet/in.h; arpa/inet.h exposing it on Linux does not establish portability.
  Avoid casting sockaddr pointers to more strictly aligned address structures.
  Check the reported length and copy needed fields into aligned local storage.
- Treat EINPROGRESS as an operation still in flight: retain its socket across
  event callbacks and close it only after completion, failure or cancellation.
  A zero-time poll followed by close restarts rather than observes a handshake.
  Test delayed completion, refusal, deadlines and cleanup; immediate Linux
  loopback success does not prove correct FreeBSD asynchronous behavior. Keep
  readiness fixtures nonblocking too: readiness can disappear before accept.
- Verify installation into an empty DESTDIR, including bootstrap staging, public
  headers and archives, and repeated parallel installation. New include
  directories need creation and ordering, not only an mtree entry.
- For Lua C bindings, preserve argument indices before pushing temporaries;
  respect luaL_Buffer's stack ownership rules. Exercise omitted/nil optional
  arguments, boundary-sized data and allocation growth, not just small inputs.
- For native mount updates, inspect FreeBSD's option-merging semantics. MNT_UPDATE
  alone does not override a retained ro option; request rw/noro explicitly when
  authorized and verify the resulting kernel mount state. Model retained options
  and failed readback in portable tests; only native testing proves ZFS behavior.
- Clearly distinguish syntax checks, real compilation/linking, portable Linux
  smoke tests and native FreeBSD runtime tests. Temporary API declarations or
  ENOSYS link stubs never establish FreeBSD ABI or kernel behavior. Report native
  buildworld/buildkernel and ATF results as pending until they actually run.
- Preserve current boot behavior. Do not commit or push fixes until the user
  authorizes them. Document corrections and validation limits in the handoff.
