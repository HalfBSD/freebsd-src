# C dialects and the seatd build failure

The seatd import exposed a mismatch between its upstream build configuration
and HalfBSD's new BSD Makefiles. This was an integration error: the initial
tests compiled in strict C mode, while buildworld inherited GNU mode.

## What GNU17 means

`gnu17` means the C17 language with GNU extensions enabled. `c17` selects the
ISO C17 dialect. They use the same language-standard generation; switching to
`c17` is not an upgrade to a newer language, a different compiler, or a different
runtime library. GNU mode is useful for code that intentionally relies on its
extensions. Strict mode offers a narrower, more portable language contract.

Neither mode alone proves standards compliance. For example, a compiler may
still accept extensions in strict mode unless appropriate diagnostics such as
`-Wpedantic` are enabled. Language selection also does not replace warnings,
linker hardening, tests, or explicit operating-system API feature macros.

## Why GNU extensions can be desirable

Systems code sometimes needs capabilities that ISO C17 does not express well.
GNU extensions can make that code more practical:

- `typeof` lets a macro preserve an expression's type without requiring the
  caller to repeat it. This helps implement generic low-level helpers.
- Statement expressions let a macro contain local variables and statements
  while returning a value. Combined with type inference, they can help helpers
  evaluate arguments only once rather than repeat side effects.
- Extended inline assembly lets code describe machine instructions and their
  register and memory effects to the compiler. Architecture-specific code may
  need this when ordinary C or available intrinsics cannot express an operation.
- Compiler attributes can express alignment, section placement, calling
  conventions, and diagnostic contracts that matter to kernels, loaders, and
  libraries.

These facilities can improve generated code, diagnostics, or maintainability
when used deliberately. They also help existing systems software build without
rewriting established compiler-specific mechanisms. Clang supports many GNU
extensions too, so their use does not necessarily require the GCC compiler.

There is an important qualification: GNU language mode is not required for
every compiler extension. Compilers accept some attributes, built-ins, reserved
spellings, and assembly forms under strict C17 as well; diagnostic behavior
depends on the construct and warning settings. Each actual requirement should
be checked rather than assuming that any compiler-specific feature requires
`gnu17`. Likewise, POSIX and FreeBSD system calls are library interfaces, not
GNU language extensions. Using those APIs does not itself justify GNU mode.

The costs are a wider language contract, possible compiler dependencies, and
additional behavior such as the legacy predefined macro involved here. GNU mode
is desirable when its capabilities serve a concrete source requirement; it is
not inherently a better choice for ordinary portable C. Preferring strict C17
for HalfBSD components that compile cleanly under it is reasonable. Retaining
GNU mode where it is demonstrably needed avoids turning that preference into
an unrelated rewrite of working low-level code.

## Why this failed

`contrib/seatd/meson.build` selects `c_std=c11`, with warnings treated as errors.
Our BSD Makefiles omitted `CSTD`, so `share/mk/bsd.sys.mk` supplied its default
`gnu17` and added the corresponding compiler option.

In the reported FreeBSD compiler invocation, GNU mode predefines the legacy
macro `unix` as `1`. The imported sources use `unix` as a union member:

```c
struct sockaddr_un unix;
```

The preprocessor replaces that member name with `1`, producing invalid C.
Uses such as `addr.unix.sun_family` fail for the same reason. Both the libseat
client backend and the seatd daemon contain this member, so fixing only the
library would leave a second failure later in the build.

Strict mode suppresses this legacy unreserved macro on the compilers examined.
The host GCC check retained `__unix__` in both modes, but exposed `unix` only
under GNU17. Reserved implementation names such as `__unix__` avoid this
particular collision with ordinary application identifiers.

## The correction

Both `lib/libseat/Makefile` and `usr.sbin/seatd/Makefile` now explicitly select
`CSTD=c17`. This preserves upstream's strict-dialect intent while using
HalfBSD's C17 baseline. It leaves imported implementation files unchanged and
retains existing warning, PIC/PIE, and linker-hardening settings. Explicit
FreeBSD/POSIX API visibility definitions remain in place: strict C syntax does
not mean an operating-system service must use only ISO C library functions.

The build regression now checks BSD make's effective `CSTD` and `CFLAGS`, then
compiles both affected files using the selected dialect. It reproduced the
macro collision before the correction and passes afterward. Existing staged
installation checks and the private shared/static PIE runtime tests also pass
on Linux. Native FreeBSD buildworld, build32, and device testing remain pending;
Clang was unavailable in this workspace.

## Should HalfBSD replace GNU17 everywhere?

For code that does not need GNU extensions, strict C17 is a sensible default
choice for new components. It makes the intended language contract explicit
and helps keep imported builds consistent with upstream.

A tree-wide switch is a separate engineering project. The base system contains
kernel code, boot code, libraries, utilities, and imported software with different
language requirements. Changing `share/mk/bsd.sys.mk` globally could break code
that legitimately relies on GNU behavior or alter declarations exposed by
system headers. This seatd failure does not establish that those components
can all use strict C17 unchanged.

The practical policy is to select each imported component's intended dialect
explicitly and test its actual production flags. If HalfBSD later adopts a
broader strict-C default, first inventory explicit and inherited dialects,
compile affected components with Clang and GCC where available, and validate
native world, kernel, bootstrap, and 32-bit compatibility builds. Keep exceptions
grounded in actual source requirements rather than disabling diagnostics.

No global compiler-dialect change is part of this fix. C23 would be another
language migration, with its own compiler and compatibility requirements; it
is unnecessary to resolve this collision.

The lasting lesson is that a stricter smoke test can accidentally conceal a
production-build problem when its flags differ. Test the effective build
configuration as well as additional strict-warning configurations. This is
also recorded in `AGENTS.md` for future imports.
