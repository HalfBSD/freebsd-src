# HalfBSD source build options

This inventory describes the options available in this source checkout as of
October 4, 2026. It is based on `share/mk/src.opts.mk`,
`share/mk/bsd.opts.mk`, `share/mk/bsd.mkopt.mk`, and `sys/conf/kern.opts.mk`.
HalfBSD has removed or restricted some options that stock FreeBSD permits.

## Using /etc/src.conf

Use flags like these:

```make
WITHOUT_BLUETOOTH=1
WITHOUT_TESTS=1
WITH_CCACHE_BUILD=1
```

Flags are checked for presence, so `WITH_FOO=no` does not disable an option.
Use `WITHOUT_FOO=1`. If both forms are present, `WITHOUT_` generally wins.
The build system translates these flags into `MK_FOO=yes` or `MK_FOO=no`.

The tree uses `${SRCTOP}/src.conf` when that file exists, otherwise
`/etc/src.conf`, unless `SRCCONF` overrides the location.

## Enabled by default

The following suffixes remain registered and can normally be disabled with
`WITHOUT_`. Exceptions and dependencies are described below.

```text
ACCT                     ASSERT_DEBUG             AT
AUDIT                    AUTOFS                   BLUETOOTH
BSDINSTALL               BSD_CPIO                 BZIP2
CAROOT                   CCD                      CLANG
CLANG_BOOTSTRAP          CPP                      CROSS_COMPILER
CUSE                     CXGBETOOL                DEBUG_FILES
DEPEND_CLEANUP           DICT                     DOCCOMPRESS
DTRACE                   DYNAMICROOT              ELFTOOLCHAIN_BOOTSTRAP
EXAMPLES                 FILE                     GOOGLETEST
GPIO                     ICONV                    INCLUDES
INSTALLLIB               JEMALLOC_LG_VADDR_WIDE   KDUMP
KVM                      LDNS                     LDNS_UTILS
LEGACY_CONSOLE           LLD                      LLD_BOOTSTRAP
LLVM_BINUTILS            LLVM_COV                 LLVM_CXXFILT
LOADER_GELI              LOADER_IA32              LOCALES
LS_COLORS                MACHDEP_OPTIMIZATIONS    MAKE
MAKE_CHECK_USE_SANDBOX   MALLOC_PRODUCTION        NETCAT
NETGRAPH                 NETLINK_SUPPORT          NLS
NLS_CATALOGS             NS_CACHING               NTP
PKGBOOTSTRAP             PMC                      QUOTAS
RELRO                    REPRODUCIBLE_BUILD       SERVICESDB
SETUID_LOGIN             SOUND                    SOURCELESS
SOURCELESS_UCODE         SSP                      STATS
SYSTEM_COMPILER          SYSTEM_LINKER            TESTS
TEXTPROC                 TOOLCHAIN                UNBOUND
UTMPX                    VI                       WARNS
WERROR                   WPA_SUPPLICANT_EAPOL     ZFS_TESTS
ZONEINFO
```

`SOUND` is additionally forced on for the top-level amd64 build.
The core options listed later are also normally enabled by default.

## Disabled by default

The following suffixes can normally be enabled with `WITH_`:

```text
ASAN                     BEARSSL                  BHYVE_SNAPSHOT
BIND_NOW                 BRANCH_PROTECTION        CCACHE_BUILD
CLANG_EXTRAS             CLANG_FORMAT             CLEAN
CTF                      DETECT_TZ_CHANGES         DISK_IMAGE_TOOLS_BOOTSTRAP
DTRACE_ASAN              DTRACE_TESTS             EXPERIMENTAL
INSTALL_AS_USER          LLVM_ASSERTIONS          LLVM_FULL_DEBUGINFO
LLVM_LINK_STATIC_LIBRARIES LLVM_TARGET_ALL        LLVM_TARGET_BPF
LLVM_TARGET_MIPS         LOADER_VERBOSE           LOADER_VERIEXEC_PASS_MANIFEST
OPENLDAP                 PTHREADS_ASSERTIONS      RETPOLINE
RUN_TESTS                SORT_THREADS             STALE_STAGED
UBSAN                    UNDEFINED_VERSION        ZEROREGS
ZONEINFO_LEAPSECONDS_SUPPORT
```

`STALE_STAGED` also has a dependent default following `STAGING`. Its initial
default is off; the dependent default applies when no earlier value was set.

## Options with dependent defaults

These accept explicit `WITH_` and `WITHOUT_` overrides, although later
dependency rules can still force them off.

| Option suffix | Default follows |
|---|---|
| `CLANG_FULL` | `CLANG` |
| `LOADER_VERIEXEC` | `BEARSSL` |
| `LOADER_EFI_SECUREBOOT` | `LOADER_VERIEXEC` |
| `LOADER_VERIEXEC_VECTX` | `LOADER_VERIEXEC` |
| `VERIEXEC` | `BEARSSL` |
| `BZIP2_SUPPORT` | `BZIP2` |
| `INET_SUPPORT` | `INET` |
| `INET6_SUPPORT` | `INET6` |
| `KVM_SUPPORT` | `KVM` |
| `NETGRAPH_SUPPORT` | `NETGRAPH` |
| `PAM_SUPPORT` | `PAM` |
| `TESTS_SUPPORT` | `TESTS` |
| `WIRELESS_SUPPORT` | `WIRELESS` |
| `STAGING_PROG` | `STAGING` |

`MAKE_CHECK_USE_SANDBOX` also has a dependent default following `TESTS`, but
its initial default is on. Dependent defaults do not replace an existing
`MK_` value.

## Architecture-dependent options

- `FDT`: defaults off on amd64/i386, on elsewhere.
- `LIB32`: enabled on amd64 and supported aarch64 compiler configurations;
  forced off elsewhere.
- `LLDB`: defaults on.
- `OPENSSL_KTLS`: defaults on for amd64/aarch64, off elsewhere.
- `OPENMP`: defaults on for amd64/aarch64/i386, off elsewhere.
- `PIE`: defaults off on armv7/i386, on elsewhere.
- `LLVM_TARGET_AARCH64`, `LLVM_TARGET_ARM`, `LLVM_TARGET_RISCV`, and
  `LLVM_TARGET_X86`: the native backend defaults on; other backends follow
  `LLVM_TARGET_ALL`. Aarch64 also enables ARM by default.
- `BHYVE_SNAPSHOT`: forced off outside amd64.
- `BRANCH_PROTECTION`: forced off outside aarch64.
- `LOADER_IA32`: forced off outside amd64.
- `CXGBETOOL`: forced off outside amd64/aarch64/i386.
- `EFI`: forced off on i386.

## Core options required by HalfBSD

For a top-level amd64 source build, these are forced on. Their `WITHOUT_`
forms are ignored with a warning:

```text
ACPI       BHYVE       BOOT        CDDL       CRYPT
EFI        INET        INET6       IPFW       JAIL
NETLINK    OPENSSH     OPENSSL     PAM        SOUND
USB        VT         WIRELESS    ZFS        LOADER_LUA
LOADER_ZFS
```

The restriction applies when `SRCTOP` is defined, the current directory is
`SRCTOP`, the make level is zero, and `BOOTSTRAPPING` is not defined.
Sub-builds and other architectures retain ordinary option handling.
Explicit command-line `MK_` assignments that disable a required core option
are rejected.

`CASPER` is required globally by `src.opts.mk`.

## Additional kernel/module build options

Enabled by default:

```text
FORMAT_EXTENSIONS
IPSEC_SUPPORT
KERNEL_SYMBOLS
SCTP_SUPPORT
SPLIT_KERNEL_DEBUG
```

Disabled by default:

```text
KERNEL_BIN
KERNEL_RETPOLINE
RATELIMIT
```

`KERNEL_BIN` is restricted to arm/arm64, and `KERNEL_RETPOLINE` to i386/amd64.
These flags affect kernel build mechanics and module selection; kernel
configuration files still determine compiled-in kernel features.

The standalone kernel option definitions default `REPRODUCIBLE_BUILD` and
`VERIEXEC` off if no `MK_` value was already supplied. In-tree builds can
inherit values established by the general source option machinery.
Disabling `SPLIT_KERNEL_DEBUG` also disables `KERNEL_SYMBOLS`.

## Settings without a WITH_/WITHOUT_ prefix

```make
INIT_ALL=none       # none, pattern, or zero; default none
LIBC_MALLOC=jemalloc
```

The kernel changes `INIT_ALL=zero` back to `none` on amd64.
`jemalloc` is currently the only accepted `LIBC_MALLOC` value.

## Options that belong outside /etc/src.conf

These exist, but `share/mk/src.sys.mk` explicitly rejects changing them in
`src.conf`. Set them through `src-env.conf`, the environment, or make
arguments:

```text
AUTO_OBJ
DIRDEPS_BUILD
DIRDEPS_CACHE
META_ERROR_TARGET
META_MODE
STAGING
SYSROOT
UNIFIED_OBJDIR
```

These early build options are registered by `share/mk/sys.mk`.

## Removed options, vestiges, and dependencies

- `WITH_MAN` cannot restore manual-page output: `MK_MAN=no` is forcibly
  propagated to recursive builds by `share/mk/bsd.opts.mk`.
- `GAMES` and `FINGER` are absent from the option registries.
- `DOCCOMPRESS` and `OPENLDAP` remain registered, but the source search found
  no remaining literal consumers of their `MK_` variables outside the option
  definitions. Treat them as likely vestigial.
- Dependencies can override a requested setting. Disabling `TESTS` disables
  DTrace/ZFS tests; disabling `CLANG` disables its extras; disabling
  `NETGRAPH` disables Bluetooth.
- Disabling `CDDL` disables CTF, DTrace, loader ZFS, and ZFS. Disabling
  `OPENSSL` disables OpenSSH, OpenSSL KTLS, LDNS, pkg bootstrap, loader ZFS,
  and ZFS. The top-level amd64 required-option rules restrict these choices.
- Disabling `CROSS_COMPILER` disables Clang, elftoolchain, and LLD bootstrap
  options. Disabling `TOOLCHAIN` disables Clang, LLD, LLDB, and LLVM binutils.
- `ASAN` forces LLVM binutils on; LLVM binutils forces LLVM C++ demangling on.
- `CLEAN` disables `DEPEND_CLEANUP`.

Other direct `WITH_`/`WITHOUT_` switches exist for specialized make targets,
release tooling, or vendored projects. They are not part of the general
`src.conf` option registry described here.

## Inspection limits

This inventory was produced by source inspection. `bmake` was unavailable
on the inspection host, so every configuration was not evaluated
dynamically. Registration establishes that the build recognizes an option;
it does not guarantee every combination builds successfully.
