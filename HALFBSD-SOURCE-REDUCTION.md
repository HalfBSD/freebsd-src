# HalfBSD source reduction and remaining audits

Consolidated October 22, 2026. This replaces the long architectural and locale
assessment narratives. The table below contains historical measurements; it is
not a fresh scan or a claim that all proposed removals have happened.
Current startup and DNS behavior are in [HALFBSD.md](HALFBSD.md) and
[UNBOUND.md](UNBOUND.md). Git retains the original per-change reports.

Fresh measurements are in [HALFBSD-SOURCE-METRICS.md](HALFBSD-SOURCE-METRICS.md).
They supersede the historical totals below for current-tree comparisons.

## Measured history

Fixed October 3 snapshots: FreeBSD `releng/15.1` at `e38a7085f3a8` versus HalfBSD
at `18799602c9b0`, both including 15.1-RELEASE-p4 updates.

| Measure | FreeBSD | HalfBSD | Net reduction |
| --- | ---: | ---: | ---: |
| Tracked files | 109,076 | 91,804 | 17,272 (15.8%) |
| Uncompressed blob bytes | 1,538,792,947 | 1,361,564,761 | 177,228,186 (11.5%) |
| Text lines | 41,835,868 | 36,621,924 | 5,213,944 (12.5%) |

Use `git ls-tree -r -l` and blob content for reproducible file/byte counts. Text
estimates exclude NUL-containing blobs; path moves and Git rename-aware diff
counts differ. These totals exclude subsequent architecture, driver, Unbound,
RC and console changes. They measure repository content, not installed size,
compile time or exploitable/reachable code. A driver absent from the running
kernel is different from a reachable network service.

## Implemented reduction areas

- Retired PowerPC, RISC-V and native ARM/ARM64 targets; retain shared x86/i386
  code and compatibility work needed by AMD64. Imported LLVM cross-target
  recognition/backends are a separate matter.
- UEFI-focused boot: retired legacy BIOS boot paths, Forth, PXE/TFTP/BOOTP and
  alternate Kboot/U-Boot/kshim paths. Bhyve userboot and shared EFI/Lua remain.
- Removed NFS networking, related GSS/RPC service infrastructure, SMB1/SMBFS,
  PF/IPFILTER/ALTQ, reviewed router-discovery/routing and SAN protocols.
  IPFW/dummynet, shared network/VFS infrastructure and local ACLs remain.
- Removed reviewed Xen/Hyper-V/VMware/cloud integration, Fibre Channel adapters,
  legacy RAID/SCSI families, CardBus/PC Card, parallel ports and obsolete
  NIC/Wi-Fi/audio/USB/tape leaves. Modern retained drivers and shared CAM, PCI,
  USB, HDA, PHY and Ethernet support remain.
- ULE is mandatory; the alternate 4BSD scheduler and selector/dispatch machinery
  are gone. Reviewed legacy Netgraph WAN nodes are removed, not generic Netgraph.
- Removed old DRM2/TTM/AGP; keep LinuxKPI, modern external drm-kmod/AMDGPU,
  framebuffer/backlight and the physical console. LinuxKPI Xen query exports
  needed by external DRM remain false-valued interfaces, not Xen guest support.
- Retired native compatibility below FreeBSD 11 and native a.out execution;
  reserved syscall slots, current exports and independent FreeBSD32/Linux
  compatibility boundaries remain. Do not equate those with obsolete native ABIs.
- Removed reviewed mail/printing, games, messaging, legacy administration and
  duplicate utility/documentation stacks, tcsh, moused/msconvd, `/rescue`,
  freebsd-update and most locale data. Base sh, editor/development tools and
  timezone data remain. Local console mouse daemons are not needed for Wayland
  evdev/hms input.
- Reduced Unbound and removed traditional RC, rcorder, sysrc, RC-only bsdconfig
  screens and NanoBSD/diskless support. Replaced startup policy with halfinit.
  The latest change removes the independent PID 1 shell and VT switching while
  preserving one boot/login/graphics console; native validation of that change
  is pending.

These are summaries of implemented branches, not a full deletion manifest.
Exact source boundaries and historical tests are recoverable from commits.
Removal of an OS target does not authorize deleting shared compiler/runtime code.

## Open architectural decisions

Original audit IDs are retained for searching history. These are proposals,
not authorization to remove more code or evidence of native validation.

| Area / old IDs | Dependency boundary / next decision |
| --- | --- |
| CTL HA/network targets (B3) | Bhyve virtio-SCSI uses local CTL and `/dev/cam/ctl`; preserve that subset. |
| Extra GEOM classes/Gate (B4–B5) | Preserve GEOM core, GPT/labels, selected encryption/swap/image consumers; inventory installations before removing providers beneath pools. |
| UFS and administration (B6–B7) | Installer/media, makefs/fstyp and recovery still have dependencies. Replace required media before deleting UFS/quotas/snapshot tools; preserve ZFS equivalents. |
| i386 target/runtime (B8, E1) | AMD64 still consumes i386 source and build32. Separate hardware targets, FreeBSD32/Linux32 and package compatibility. Keep LinuxKPI for DRM. |
| OpenBSM (B13) | Login/su/SSH, libbsm, audit APIs/syscalls and tests need coordinated treatment. Distributed collection is separable from local auditing. |
| VirtIO guest code (B15) | VM testing can need guest devices; bhyve host emulation/lib9p are different consumers. |
| Network offloads (B16) | Separate optional TOE/IPsec/SR-IOV paths from ordinary retained NIC/checksum/segmentation support. |
| Installer/release (C1) | Establish a tested UEFI/GPT/ZFS install and external recovery path. Current RC-free live/media recipes are not verified. |
| Remaining legacy graphics (C4) | VT switching is removed; shared EFI/DRM handoff remains. VBE/x86 BIOS/re-POST and sleep/resume need their own dependency review. |
| Wi-Fi modes (C5) | Review obsolete crypto/AP/mesh separately; preserve required WPA2/WPA3 and institutional EAP. |
| Privilege defaults/PAM (C6) | PAM removal is future work. Preserve explicit authentication, credentials and service roles until replacements are implemented/tested. |
| Toolchain (C7) | Trace EFI elfcopy, bootstrap, C/C++ packages, debugging, sanitizers and desired cross-compilation before pruning backends/tools. |
| GELI/swap (E3) | Decide encrypted boot/swap/recovery policy before removal. |
| SCTP/IPsec (E4) | Separate unused protocols from KTLS and legitimate VPN/package consumers. |
| MAC/racct/jails (E5) | Audit security/API/test and local virtualization consumers; lack of everyday use is not proof of independence. |
| Netgraph/netmap (E6) | Keep useful programmable/bhyve networking boundaries; no wholesale deletion presumed. |
| Bluetooth (E7) | Decide workstation hardware policy; current Lua startup has no full Bluetooth service provider. |
| NSS/small compatibility libraries (E8) | Check libc linkage, authentication and packages before removal. |
| Historical utilities/formatting (E9) | Inventory actual retained consumers; avoid reconstructing removed tools through compatibility wrappers. |

Also audit startup responsibilities formerly handled by RC: temporary/stale-state
cleanup, hostid/ZFS identity/timezone handling, general sysctl/devfs permissions,
log rotation/accounting and suspend/resume. Normal boot success does not prove
parity with every optional FreeBSD service. Ports daemons need explicit native
service definitions, identities and readiness policy; do not evaluate rc.d
scripts in the new manager.

## Locale boundary

The default login class selects UTF-8/C.UTF-8. C and POSIX byte semantics still
exist in libc and are different from pruning installed locale data. No universal
UTF-8 libc conversion was implemented by the prior audit.

In C/POSIX, conversion accepts individual bytes (`MB_CUR_MAX=1`); UTF-8 validates
multibyte input and can reject arbitrary bytes. Changing startup defaults or
aliasing C/POSIX affects regex, sort/cut/wc, ctype, shell/build scripts and external
packages even where names are absent from searches. Preserve deliberate binary
processing. Locale-neutral formatting is not the same as Unicode character
classification or collation.

A future project must coordinate initial rune/ctype objects, setlocale/newlocale,
thread-local state, byte/wide conversion, exported symbols and static early-boot
availability without depending on mounted locale files. Test invalid/incomplete
UTF-8, explicit C/POSIX, empty environments, thread locales, tools/build scripts
and packages. Environment changes alone cannot establish this behavior.
The prior October 6 source search found 97 native non-test, 90 native-test,
193 imported non-test and 22 imported-test matched lines; these are dated search
hits, not a complete dependency graph or patch estimate.

## Audit data and build options

Keep detailed inventories outside the root narrative:

- [Kernel-module source assessment](docs/audits/HALFBSD-KERNEL-MODULES.tsv).
- [AMD64 evaluated module-build inventory](docs/audits/HALFBSD-KERNEL-MODULE-INVENTORY.txt).
- [Locale source-search evidence](docs/audits/HALFBSD-LOCALE-AUDIT.tsv).
- [Dated build-option reference](docs/audits/HALFBSD-BUILD-OPTIONS.md).

They are snapshots, not Anvil's loaded modules or a guarantee every option
combination builds. For the current host use `kldstat -v` and inspect `.ko` files
under `/boot/kernel` and `/boot/modules`; some module implementations also build
into HALFBSD. External AMDGPU is outside the in-tree module list. Current build
policy lives in `share/mk/src.opts.mk`, `sys/amd64/conf/HALFBSD` and
`tools/build/check-halfbsd.py`. Prefer actual source/build evaluation over old
inventory classifications.
