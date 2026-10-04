# HalfBSD architectural removal audit

Audit date: October 3, 2026.
Source snapshot: `e1444ad6eb26`.

This document tracks the original architectural assessment and subsequent
user-authorized implementation. Open groups appear first; processed groups
appear together below with their implementation status and validation limits.
Original recommendations in processed groups are retained as audit history.

**NFS remains excluded.** The user corrected the NFS requirements in `bar.txt`:
HalfBSD does not want NFS in the source tree at all. This audit follows that
correction.

Remaining work includes CTL high availability, storage and
platform simplification and ABI-sensitive reductions. UFS,
Netgraph, RPC, and compatibility layers require explicit dependency boundaries.

The review covers the current tree, including build selections, kernel
configurations, libraries, utilities, startup scripts, release machinery, tests,
and previous removals. It is a source-level architectural assessment. External
package dependencies and hardware behavior remain unverified; those limits are
reflected in the classifications. No builds or hardware tests were performed as
part of this read-only audit.

Two distinctions matter throughout:

- Disabling a feature in `CHARLIE` does not necessarily prevent its modules or
  userland components from being built.
- Source size, installed size, build time, and reachable attack surface are
  different measures. Removing an unloaded driver reduces maintenance burden
  but provides less immediate security benefit than removing an enabled network
  service.

For each recommendation below, “no core impact expected” means no identified
dependency from retained bhyve, Wayland, browsers, audio, ordinary networking,
ZFS, SSH, pkg, or development tools. It does **not** certify every external
package. NFS compatibility is deliberately outside the requirements.

Size estimates below count tracked file content in the named source subsets.
They exclude Git history and build artifacts and are not estimates of installed
size or executable attack surface. Subsets are not a complete deletion manifest
and should not be summed as a promised reduction.

## Open groups

Stable audit IDs are preserved for discussion and follow-up work.

### B. Likely removal candidates requiring dependency cleanup

#### B3. CTL high availability and network-target infrastructure

**Purpose and locations:** `sys/cam/ctl`, particularly HA machinery and network
frontends; `usr.sbin/ctld`, `ctladm`, `usr.bin/ctlstat`.

**Why present / fit:** Storage targets and clustered storage controllers.
HA/network serving does not fit; local virtual storage can.

**Dependency finding:** `usr.sbin/bhyve/pci_virtio_scsi.c` uses CTL headers,
operations, and `/dev/cam/ctl`.

**Disposition and impact:** Remove HA and SAN frontends while preserving the
local CTL subset needed by bhyve. Complete CTL deletion requires deliberately
dropping or replacing bhyve’s virtio-SCSI backend.

**Benefit / risk / confidence:** Potentially substantial simplification, but
high dependency risk for complete removal. High confidence in the boundary;
medium confidence in a minimal replacement.

**Validation:** CTL-backed guest disks, SCSI commands, resets, flush/discard,
concurrent I/O, and guest shutdown.

#### B4. Non-ZFS GEOM volume-management classes

**Purpose and locations:** `sys/geom`, `sys/modules/geom`, `lib/geom`.
Candidates: RAID/RAID3, stripe, concat, shsec, virstor, journal, multipath, Linux
LVM, and CCD.

**Why present / fit:** Alternative volume management, hardware RAID metadata,
UFS journaling, and server storage. Most duplicate capabilities HalfBSD assigns
to ZFS.

**Dependencies:** GEOM itself is essential. Installer encryption/swap paths
currently use GEOM mirror in some configurations. Other classes have
recovery/testing uses.

**Disposition and impact:** Delete selected classes after removing consumers.
Keep GEOM core, disks, GPT, labels where used, ELI pending encryption policy,
and useful test/image classes. `WITHOUT_CCD` covers only CCD.

**Benefit / risk / confidence:** High cumulative simplification.
Moderate-to-high risk if used beneath existing pools; high confidence in target
direction.

**Validation:** Installer layouts, ZFS on supported providers, encrypted swap,
disk labels, boot environments, and retained GEOM tests.

#### B5. GEOM Gate remote block access

**Purpose and locations:** `sys/geom/gate`, its module,
`sbin/ggate/{ggatec,ggated,ggatel}`, startup integration.

**Why present / fit:** Exporting and consuming block devices over a network;
also a local userland block-provider interface. Network export is outside scope.

**Dependencies:** `ggated` and `ggatec` are clear removal candidates. `ggatel`
and the kernel interface can serve testing or unusual development uses.

**Disposition and impact:** Delete remote daemon/client first. Complete
subsystem removal requires deciding whether userland-backed block-device
development is retained.

**Benefit / risk / confidence:** Moderate protocol/service reduction. Low risk
for remote pieces, moderate for complete removal; high/medium confidence
respectively.

**Validation:** World/modules, GEOM tests, and any disk-image tooling using Gate.

#### B6. UFS/FFS as a native storage architecture

**Purpose and locations:** `sys/ufs`, `lib/libufs`, filesystem
creation/checking/dump/restore utilities, quota tools, loader UFS support,
installer choices, rc scripts, and release recipes.

**Why present / fit:** FreeBSD’s traditional root/storage filesystem. It
conflicts with ZFS-only native storage.

**Dependency finding:** Current `release/amd64/make-memstick.sh` installation
media still mount a UFS root. EFI `boot1` uses UFS support; `makefs` creates FFS
images; `fstyp` links `libufs`; rc diskless/memory-filesystem paths also need
review.

**Disposition and impact:** Replace release/recovery media and installer paths
before deleting UFS. Retaining a restricted interoperability reader is a
separate policy choice, not justification for UFS root.

**Benefit / risk / confidence:** High architectural reduction, though `sys/ufs`
plus `libufs` alone are only approximately **1.36 MB**. High boot/recovery risk;
high confidence in eventual removal, medium confidence in readiness.

**Validation:** Build and boot fresh installation/recovery media; install ZFS
root; encrypted boot; boot environments; upgrade and damaged-pool recovery.

#### B7. UFS quotas, snapshots, and administration tools

**Purpose and locations:** `quotacheck`, `quotaon`, `quota`, `edquota`,
`repquota`, `quot`, `snapinfo`, `mksnap_ffs`, and associated kernel/library/rc
support.

**Why present / fit:** UFS multiuser storage administration. These do not
implement ZFS dataset quotas or snapshots.

**Dependencies:** UFS and `libufs`; therefore remove with B6. The quota rc
script is currently installed independently of several feature selections.

**Disposition and impact:** Exclude `MK_QUOTAS`, then delete the UFS-specific
stack. Preserve ZFS quota, reservation, snapshot, and accounting properties.

**Benefit / risk / confidence:** Moderate utility/kernel reduction and removal
of setuid tools. Low risk once UFS is retired; high confidence.

**Validation:** Installer/recovery builds, ZFS quotas/snapshots, world/install
manifests, and obsolete-file cleanup.

#### B8. Non-amd64 operating-system targets

**Purpose and locations:** `sys/arm`, `sys/arm64`, native i386 kernel/platform
code, `sys/dts`, architecture-specific userland implementations, loaders,
release/CI targets, and board drivers.

**Why present / fit:** FreeBSD supports multiple architectures and embedded
systems; HalfBSD does not.

**Dependencies:** **amd64 still consumes i386-located code**, including some
compatibility and boot material. Shared x86, ACPI, compiler definitions, ELF
recognition, and cross-development support need separate treatment.

**Disposition and impact:** Retire target/build entry points first; delete
architecture-exclusive implementation after tracing amd64 inclusions. Do not
delete `sys/i386` wholesale or equate OS-target retirement with eliminating LLVM
cross-compilation.

**Benefit / risk / confidence:** High maintenance reduction. The four measured
source areas contain approximately **10.49 MB across 1,231 files**, with
additional glue elsewhere. Moderate-to-high risk; high confidence in policy,
medium confidence in deletion boundaries.

**Validation:** Native and cross-host amd64 world/kernel builds, EFI,
libc/runtime, debuggers, and external kernel modules.

#### B13. OpenBSM auditing

**Purpose and locations:** `sys/security/audit`, `sys/bsm`, `contrib/openbsm`,
`libbsm`, `libauditd`, audit tools and daemons.

**Why present / fit:** Security event auditing and compliance. Distributed audit
collection is a poor fit; local forensic auditing has potential workstation
value.

**Dependencies:** Login, su, SSH, kernel syscall annotations, MAC hooks, and
audit tests. `CHARLIE` excludes `AUDIT`, but `libbsm` and `libauditd` remain
unconditional library selections. `lib/Makefile` also contains the apparent typo
`SUBDIR_DEPEND_libauditdm`.

**Disposition and impact:** Remove `auditdistd` first. Full audit removal
requires optional userland integration and explicit treatment of exported
APIs/syscalls; `WITHOUT_AUDIT` is not complete source retirement.

**Benefit / risk / confidence:** Approximately **3.24 MB in the measured
subset**. Moderate security/ABI risk; high confidence for distributed
collection, medium for complete removal.

**Validation:** Login/su/sshd/PAM, clean world/kernel, syscall generation, and
package consumers of `libbsm`.

#### B15. VirtIO guest framework and guest-only filesystems

**Purpose and locations:** `sys/dev/virtio`, `sys/fs/p9fs`, guest clocks and
related modules.

**Why present / fit:** Running FreeBSD inside hypervisors. This is not the
primary HalfBSD role, but can simplify testing HalfBSD itself under bhyve.

**Dependencies:** bhyve’s host VirtIO emulation is separate. `lib9p` is used by
bhyve’s host shared-directory implementation and must not disappear with guest
p9fs.

**Disposition and impact:** Prefer removing unnecessary guest devices
first—ballooning, guest console/GPU, and unsupported transports. Complete VirtIO
deletion requires choosing a different virtual test configuration.

**Benefit / risk / confidence:** Moderate driver reduction. Moderate
testing/recovery risk; medium confidence for full removal, higher for unused
leaves.

**Validation:** bhyve host emulation and shared directories; boot the chosen
HalfBSD test VM; physical-machine boot.

#### B16. Enterprise network offload paths

**Purpose and locations:** TCP offload core, `toecore`, Chelsio TOE/iSCSI
offload, inline IPsec offload, selected SR-IOV/VF support, and administration
tools such as `cxgbetool`.

**Why present / fit:** High-throughput server and appliance workloads. Ordinary
Ethernet remains useful, including fast workstation NICs.

**Dependencies:** NIC drivers often combine ordinary Ethernet and offload paths.
KTLS can use software crypto and is not dependent on retaining TOE.

**Disposition and impact:** Strip unused offload capabilities before considering
whole-driver removal. Preserve checksum/segmentation offloads and relevant NIC
support. SR-IOV may still serve local virtualization.

**Benefit / risk / confidence:** Potentially high kernel complexity reduction.
Moderate-to-high NIC regression risk; medium confidence.

**Validation:** Retained NIC compilation, sustained traffic, VLAN/bridge/NAT,
KTLS where retained, and bhyve networking.

### C. Simplification/refactoring candidates

#### C1. Installer, release, NanoBSD, and diskless infrastructure

**Locations and function:** `usr.sbin/bsdinstall`, `bsdconfig`, `release`,
`tools/tools/nanobsd`, and `libexec/rc/rc.initdiskless` support many
deployment/storage/platform choices.

FreeBSD contains this generality for servers, embedded appliances, cloud images,
and alternate roots. HalfBSD needs a clear UEFI/GPT/ZFS installation and recovery
path.

**Dependencies and disposition:** Current release media still use UFS;
installer scripts offer UFS; NanoBSD uses UFS layouts; diskless startup creates
writable overlays. Replace required media behavior before deleting alternatives.
Cloud deployment recipes and NanoBSD can then disappear coherently. `bsdconfig`
can shrink to retained administration tasks.

**Impact / benefit / risk / confidence:** High configuration reduction without
libc ABI change. High install/recovery risk; high confidence in simplification,
medium in replacement readiness.

**Validation:** Fresh install, reinstall, upgrade, recovery media, boot
environments, encrypted layouts, and refusal of unsupported configurations
before disk writes.

#### C4. Console backends and virtual-terminal policy

**Locations:** `sys/dev/vt`, legacy VGA/VESA/DPMS code, `sys/compat/x86bios`,
keyboard/control tools.

UEFI makes VBE/BIOS graphics paths suspect. However, `vga_pci` also uses x86 BIOS
support for graphics re-POST, so complete emulator removal needs more than a
boot-policy argument.

**Dependencies and disposition:** Prefer EFI framebuffer plus DRM console
handoff. Preserve vt, keyboard translation, evdev, and desktop VT/session
ioctls. A single login session does not prove only one kernel VT is needed.

**Impact / benefit / risk / confidence:** Moderate legacy reduction. High
graphics/recovery risk; medium confidence.

**Validation:** AMDGPU load/unload, compositor startup and exit, console
recovery, session switching, display failure, and suspend/resume.

#### C5. Wi-Fi security and mode selection

**Locations:** `sys/net80211`, WPA build glue, `contrib/wpa`, `hostapd`.

FreeBSD supports AP, mesh, WEP/TKIP, WPS, and many authentication modes. HalfBSD
chiefly needs a modern Wi-Fi client.

**Dependencies and disposition:** Remove obsolete cryptography and unused
AP/mesh modes selectively. `hostapd` is a candidate if workstation hotspot
service is excluded. Preserve WPA2/WPA3 and enterprise EAP needed on normal
institutional networks. `WITHOUT_WPA_SUPPLICANT_EAPOL` disables useful
authentication alongside unwanted features.

**Impact / benefit / risk / confidence:** Moderate parser/mode reduction. High
Wi-Fi compatibility risk; medium confidence.

**Validation:** Personal and enterprise networks, certificate authentication,
roaming, reconnect, suspend/resume, and supported adapters.

#### C6. Privilege and service defaults

**Locations:** PAM policies, utility Makefiles, `syslogd`, rc configuration,
startup services.

Several utilities install setuid/setgid: login, su, passwd, chpass, ping,
traceroute, crontab, at, newgrp, quota, and ulog-helper.

**Dependencies and disposition:** Remove privilege-bearing programs only with
their capabilities. Reduce unnecessary privilege where a tested design supports
it. Keep local syslog/newsyslog; disable unwanted remote listener behavior rather
than deleting logging.

**Impact / benefit / risk / confidence:** Potentially meaningful attack-surface
reduction, usually small build savings. Authentication/administration risk;
medium confidence.

**Validation:** File modes in staged manifests, unprivileged operation,
login/PAM, scheduled jobs, logging/rotation, diagnostics, and clean upgrade
behavior.

#### C7. Toolchain implementation and target selection

**Locations:** `lib/clang`, `usr.bin/clang`, LLVM sources, elftoolchain,
compiler-runtime and debugger code.

The toolchain is a development requirement. Redundant implementations and
unnecessary default LLVM backends may still be reduced.

**Dependencies and disposition:** Standardize selected installed tools and
backends before deleting source. EFI currently requires elftoolchain `elfcopy`;
compiler/debugger target-recognition code can survive OS-target removal. Keep
sanitizers, tracing, profiling, and cross-compilation where useful.

**Impact / benefit / risk / confidence:** Potentially large build savings, but
source pruning creates an upstream-maintenance fork. High development/bootstrap
risk; medium confidence.

**Validation:** Bootstrap, world/kernel, EFI, C/C++ package builds, sanitizers,
debugger/core handling, and intended cross-target builds.

### E. Investigate further

#### E1. Linux executable emulation and 32-bit runtime

**Locations:** `sys/compat/linux`, amd64 Linux/Linux32 code,
FreeBSD32/IA32 compatibility, `lib32`, compatibility runtime loaders.

FreeBSD provides these for binary compatibility. Linux applications, proprietary
development tools, games, and browser-related packages can make them useful even
on amd64-only hardware.

**Dependencies / disposition:** Keep LinuxKPI regardless. Determine actual
package requirements; retiring Linux32/FreeBSD32 is a separate decision from
native i386 target retirement. Initially exclude unwanted runtime builds rather
than declaring source deletion safe.

**Benefit / risk / confidence:** Moderate-to-high reduction. High package
compatibility risk; low confidence in complete removal without package evidence.
The measured Linux ABI subset is approximately **2.36 MB**.

**Validation:** Package dependency inventory, executable interpreter/ELF-class
inspection, current application tests, and compatibility ABI tests.

#### E3. GELI and encrypted swap

**Locations:** `sys/geom/eli`, userland GEOM ELI, loader GELI support, installer
encryption, rc swap/ELI scripts.

ZFS native encryption does not automatically replace block encryption or
encrypted swap. Current installer code actively uses GELI.

**Disposition:** Keep until HalfBSD specifies a complete boot, root-data, swap,
and recovery encryption model.

**Benefit / risk / confidence:** Moderate simplification possible; high
confidentiality/boot risk. Low confidence in complete removal.

**Validation:** Cold boot/key entry, swap encryption, recovery, key loss
handling, boot environments, and encrypted dataset behavior.

#### E4. SCTP and IPsec

**Locations:** `sys/netinet/sctp*`, libc SCTP wrappers, `sys/netipsec`,
`libipsec`, `setkey`.

SCTP kernel support has few obvious workstation uses, but browser WebRTC using
userspace SCTP is a separate implementation. IPsec can support legitimate
workstation VPNs.

**Dependencies / disposition:** libc exposes SCTP APIs; ping, nc, and traceroute
have IPsec integration. Trace package consumers and VPN needs before source
deletion. Kernel SCTP exclusion is easier than libc API removal.

**Benefit / risk / confidence:** Moderate protocol reduction; medium-to-high
compatibility risk. Medium confidence for SCTP kernel retirement, low for blanket
IPsec removal.

**Validation:** Socket ABI, diagnostics, browsers/WebRTC, required VPNs,
IPv4/IPv6, and package builds.

#### E5. MAC policies, resource accounting, and jails

**Locations:** `sys/security/mac*`, `sys/kern/kern_racct*`, RCTL, jail
kernel/userland support and rc integration.

MLS/Biba-style policies may be enterprise-oriented, but the MAC framework also
supports useful local restrictions. RACCT/RCTL and jails can support package
building and development isolation.

**Dependencies / disposition:** `mdo`/`mac_do`, NTP privilege policies,
development/package workflows, and service isolation need inspection. Prefer
pruning unused policy modules. Preserve ordinary POSIX resource limits
independently.

**Benefit / risk / confidence:** Moderate reduction, high security/development
workflow risk if removed broadly. Medium confidence for policy leaves; low for
framework deletion.

**Validation:** Package-building workflow, jail/VNET tests where retained,
mdo/PAM, NTP, and resource-limit tests.

#### E6. Netgraph, netmap, and network-appliance extensions

**Locations:** `sys/netgraph`, `libnetgraph`, `sys/dev/netmap`, bhyve networking
backends; CARP, VXLAN, multicast-routing modules, multiple-FIB/routing algorithms.

Netgraph looks dispensable after PPP removal, but Bluetooth and optional bhyve
networking consume it. Netmap also has a bhyve backend and useful
networking-development applications.

**Disposition:** Remove telecom leaves now. A full Netgraph cut requires
replacing Bluetooth or dropping it and retiring that bhyve backend. Evaluate
CARP/VXLAN/multicast routing separately; ordinary multicast and bridges remain
useful.

**Benefit / risk / confidence:** Netgraph’s measured subset is approximately
**2.06 MB**. Moderate-to-high feature risk; medium confidence in selective
pruning, low in blanket removal.

**Validation:** Bluetooth, bhyve tap/bridge/NAT, retained backend tests, IPv6,
multicast discovery, and networking diagnostics.

#### E7. Bluetooth

**Locations:** `sys/netgraph/bluetooth`, Bluetooth libraries, userland utilities
and daemons, USB attachment.

Bluetooth supports workstation input and potentially audio. Its Netgraph
dependency makes removal attractive architecturally, but it is not inherently
an enterprise feature.

**Disposition:** Decide supported Bluetooth use cases before removing the
stack. Preserve required input/audio capability or provide a concrete
replacement.

**Benefit / risk / confidence:** Moderate reduction with visible peripheral
loss. Low confidence in complete removal.

**Validation:** Pairing, keyboard/mouse reconnection, audio where supported,
suspend/resume, and boot-time input availability.

#### E8. NSS caching and small compatibility libraries

**Locations:** `usr.sbin/nscd`, libc cache integration, `libcompat`,
`libpathconv`, `libnetbsd`, `libopenbsd`, `libulog` and setuid ulog-helper.

`nscd` may be unnecessary with local account databases. `libpathconv` has no
identified live in-tree functional consumer outside its tests; dependency/package
metadata remains. `libcompat` retains historical APIs including `rexec`.

**Dependencies / disposition:** `libnetbsd` is used by `makefs`, `nmtree`, and
`getaddrinfo`; `libopenbsd` is used by m4. `libulog` exposes utempter compatibility
used by potential terminal packages. These are not all orphaned.

**Benefit / risk / confidence:** Small size reduction, possible
authentication/terminal/package ABI consequences. Medium confidence for
nscd/pathconv; low for broad library deletion.

**Validation:** External link dependencies, local NSS/DNS, terminal applications,
m4, filesystem-image creation, and package builds.

#### E9. Remaining historical utilities and formatting pipelines

**Locations:** `soelim`, `mkstr`, `xstr`, `lorder`, `asa`, `ul`, `col`, `pr`,
`compress`, `what`, and similar commands.

Some are now less useful after documentation/man removal; others remain portable
scripting or development interfaces.

**Dependencies / disposition:** No retained in-tree functional consumer was
identified for several historical source-processing tools, but script/package
usage is not proven absent. Treat `soelim`, `mkstr`, and `xstr` as early
investigation targets. Keep standard text processing and archive interoperability
unless a concrete dependency review supports retirement.

**Benefit / risk / confidence:** Small savings; moderate script/ports
compatibility risk. Medium confidence for isolated historical tools, low for
wholesale utility pruning.

**Validation:** Base/package build scripts, archive handling, test fixtures, and
install aliases.


## Retained architecture — D. Keep

These are important boundaries against overly broad cuts.

| Capability and major locations | Why it remains; dependencies and consequences |
|---|---|
| **ZFS and XDR** — `sys/contrib/openzfs`, `sys/xdr`, `lib/libc/xdr`, CDDL compatibility headers | OpenZFS nvpair serialization directly uses XDR. Removing RPC networking does not authorize removing serialization or its headers. Keep; high confidence. Validate pool import, send/receive, boot, and compatibility with existing pools. |
| **CAM and ordinary storage** — `sys/cam`, AHCI/NVMe/USB mass-storage drivers, `libcam`, `camcontrol`, `nvmecontrol` | Modern SATA and USB storage still use CAM/SCSI interfaces. Keep disk/passthrough infrastructure even when SAN, tape, and old controllers disappear. Validate storage and ZFS under load. |
| **GEOM core, GPT, and required provider interfaces** — `sys/geom`, `libgeom`, `gpart` | ZFS still needs underlying block devices; UEFI installations need partitioning. Keep shared infrastructure while pruning classes. Validate installation, provider discovery, and recovery. |
| **FAT/ESP and removable-media support** — `msdosfs`, `newfs_msdos`, `fsck_msdosfs`, CD9660/UDF as supported | FAT is required for the EFI System Partition. ZFS-only native storage does not eliminate removable-media interoperability or guest image creation. Keep image tools including `makefs`/`mkimg`; test ESP and media generation. |
| **tmpfs, devfs, fdescfs, nullfs, FUSE, and pseudo-filesystem frameworks** — `sys/fs` | These implement runtime namespaces, temporary storage, development environments, and third-party filesystems rather than competing root-storage architectures. Keep shared frameworks; review individual leaves. Test package builds and applications. |
| **LinuxKPI, ACPI, PCI/IOMMU, HID/evdev/uinput, I²C/GPIO support used by laptops** | AMD graphics, modern Wi-Fi, power management, touchpads, input injection, and device access rely on these. LinuxKPI is separate from Linux executable emulation. Preserve the restored Xen query stubs used by AMDGPU. Test external modules and physical hardware. |
| **Unix/POSIX APIs, IPC, libc/libsys, NSS core, PAM, local users and permissions** | Browsers, development tools, shells, login, SSH, and packages rely on them. Age is not grounds for removing SysV IPC, PTYs, signals, sockets, or account APIs. Keep; test ABI and applications. |
| **IPFW, libalias, bridge, tuntap, BPF, ordinary IPv4/IPv6 and DNS/DHCP** | These support workstation networking and local VM NAT. BPF is required by DHCP. Keep pfil integration; redundant-firewall removal does not imply removing the shared hooks. Test networking and guest connectivity. |
| **bhyve host support, libvmmapi, userboot, lib9p, nmdm, required CTL subset** | Host capabilities must be distinguished from guest drivers. Keep presently used backends until replacements are validated. Test multiple guest disk/network configurations and shared directories. |
| **Capsicum/Casper, crypto, SSH/FIDO support, trust roots** | These protect or support retained tools and authentication. FIDO libraries support hardware-backed SSH keys. Keep; validate SSH, pkg/TLS, and sandboxed tools. |
| **Diagnostics, crash recovery, tracing, compiler tools and tests** | DDB, DTrace, ktrace/truss, libkvm/procstat, PMC, dumps, network diagnostics, config, and build tools support development and maintenance. Do not remove network-assisted debugging solely because it uses networking. |
| **Local services: cron, NTP, syslog/newsyslog, devd/devmatch, power management** | Scheduling, time, logging, device handling, and power are normal workstation capabilities. Simplify obsolete modes and defaults rather than deleting entire services. |

Removing any of these broad capabilities has high dependency risk and little
justification under the stated goals. Their tests should remain part of the
common validation baseline.

## Recommended removal order and validation

Prioritize remaining work as follows:

1. **Bounded cleanup:** B3 CTL HA/network remnants and B5
   GEOM Gate, with shared interfaces traced first.
2. **Supported deployment architecture:** B8 platform scope and C1 installation,
   release and recovery paths, building on the implemented C2 policy.
3. **Storage simplification:** B4 unwanted GEOM classes and B6–B7 UFS after
   recovery media, encryption and required local storage consumers are resolved.
4. **ABI-sensitive and framework decisions:** B13, B15–B16, the remaining C groups,
   and E investigations, with explicit hardware/package/runtime requirements.

Every removal should include its build entries, generated inputs, headers,
configuration, rc scripts, tests for retired functionality, package/dependency
metadata, installation aliases, and obsolete-file cleanup. Historical mentions
inside retained upstream code are not automatically live dependencies.

The common validation gate should include:

- Clean amd64 world, kernel, **all retained modules**, staged installation, and
  packaging.
- Supported release/recovery image generation and UEFI/ZFS boot.
- External AMDGPU/Wi-Fi module compilation and loading.
- Wayland, browser, audio, suspend/resume, SSH, pkg, and development/package-build
  smoke tests.
- ZFS import, boot environments, snapshots, send/receive, and recovery.
- bhyve guest storage, tap/bridge/IPFW NAT, console, and shared directories.
- ABI/symbol/syscall checks for any library or kernel-interface removal.
- Focused retained suites under `tests/sys`, including CAM, GEOM, VFS,
  networking, ACL, Capsicum, IPC, sound, and vmm as applicable.

**The principal dependency boundaries are clear: preserve XDR for ZFS, local CTL
for current bhyve virtio-SCSI, LinuxKPI for AMD graphics and Wi-Fi, and the shared
Unix/network/storage infrastructure beneath the capabilities being removed.**

## Processed groups

### Implementation update — October 3, 2026

The follow-up removals A1 (HAST), A2 (QLogic/Emulex Fibre Channel),
A3 (legacy parallel SCSI and RAID), A4 (CardBus/PC Card and parallel ports),
A5 (the eleven reviewed legacy Ethernet/Wi-Fi families),
A6 (the seven reviewed non-HDA audio-controller families per `A6.md`),
A7 (the four legacy synchronous-WAN Netgraph leaf nodes per `A7.md`),
A10 (Kboot/native U-Boot/standalone USB boot), A11 (in-tree DRM2 and AGP),
A12 (the revised utility list per `A12.md`),
A13 (TACACS+, PAM RADIUS, and Hesiod),
B1 (remaining InfiniBand networking and Mellanox firmware support),
B9–B12 (kernel/userland RPC, service definitions, and NFS export/GSS remnants),
A8 (RIP/IPv4 router discovery), A9 (VMware/cloud drivers), B2 (SAN protocols),
C2 (maintained workstation build/kernel/module policy), and C3
(the alternate scheduler and scheduler-selection machinery), E2 (tape, changer
and remote tape), and E10 (reviewed USB gadget/handheld leaves), and B14 (native pre-11
FreeBSD ABI tiers and a.out execution) are implemented.
Their original assessments below remain as audit history. Processed means the
authorized source changes are implemented; native build and hardware validation
is still outstanding where recorded. NFS remains removed.

A6/E10/E2 were committed as `56e7b627b60b`. The user explicitly requested committing and pushing B14 with native
validation still outstanding.

A8/B2/A9/C2 were committed as `569873ccb4c3`; A5 and the audit
reorganization were committed as `94d9b81249ef`. A5 removes all eleven
user-approved families and dedicated Intel firmware, while preserving the shared
Realtek header, retained PHY drivers, MII, net80211, iflib, firmware loading,
LinuxKPI, `em`, and `ath`. Policy checks include explicit ALL_MODULES builds;
source-reference, shell-syntax and whitespace checks cover the current changes.

ULE is now compiled unconditionally and directly provides the existing public
scheduling entry points. The scheduler registry, operation table, dispatch
shims, boot-time selector, and scheduler-choice options are removed. Existing
scheduler tracing, statistics, CPU-topology diagnostics, affinity, and real-time
scheduling interfaces remain. `kern.sched.name` and `kern.sched.available`
report the fixed read-only value `ULE`; loader settings cannot select 4BSD.
Custom kernel configurations must remove `SCHED_ULE` and `SCHED_4BSD` options.

Validation includes source/build reference checks, makefile conditional
structure, rc shell syntax, mtree structure, and whitespace checks. Preprocessor
comparison verified that normal libc account/resolver paths remain unchanged
with Hesiod disabled. Scheduler validation checks public entry-point coverage,
retained ULE function bodies, initialization stages, and shared diagnostics.
Follow-up checks verified obsolete-module/header/tool cleanup, retained storage
and Netgraph implementation preservation, and unchanged `pciconf` PCI capability
decoding. Mocked shell tests exercised the renamed network hotplug helper’s
NOAUTO, already-up, DHCP, static-route, stop, Wi-Fi child and forced-start paths.
This Linux host has no native FreeBSD compiler/build environment; world,
kernel, and module builds and runtime/hardware validation remain outstanding.
Before deployment, perform a clean world/kernel/module build and test Ethernet
lagg/LACP, login/NSS/PAM, SMP scheduling, affinity and real-time priorities,
latency under workstation load, suspend/resume, and bhyve CPU load. Also test
SATA/NVMe/USB disk discovery and ZFS, UART and network hotplug, representative
retained Netgraph graphs/bhyve networking, and external AMDGPU compilation,
module loading, accelerated Wayland, display hotplug and console handoff.

### A. Processed strong removal candidates

#### A1. HAST distributed block replication

**Implementation status:** Removed from the working tree after the audit, on
October 3, 2026, at the user’s request. The original assessment below is retained
as audit history. Source/build/startup/package/account integration was removed,
and installed-file cleanup was made unconditional. Shared GEOM and `libpjdlog`
remain. See the implementation validation reported separately.

**Purpose and locations:** HAST replicates block storage between machines for
high availability. Sources are `sbin/hastd`, `sbin/hastctl`, associated rc
integration, configuration, and package metadata.

**FreeBSD rationale / HalfBSD fit:** Useful for redundant storage servers; no
compelling role in a single-workstation ZFS storage model.

**Dependencies:** HAST uses GEOM, crypto/checksum, compression, threading, and
utility support. Those shared facilities remain needed elsewhere. Its protocol,
configuration parser, and replication machinery can disappear together.

**Disposition and impact:** Exclude with `WITHOUT_HAST`, then delete the coherent
subsystem. No core impact expected; existing HAST configurations cease to work.
Do not remove `libpjdlog` merely because HAST disappears: `decryptcore` uses it.

**Benefit / risk / confidence:** Moderate complexity reduction and removal of a
privileged network daemon. Low risk; high confidence.

**Validation:** Clean world/kernel/module builds; staged installation and
obsolete-file cleanup; confirm no HAST service or package remains.

#### A2. Fibre Channel support

**Implementation status:** Removed on October 3, 2026, at the user’s request.
The `isp`, `ispfw`, and `ocs_fc` driver/firmware sources and modules, ISP-only
options, kernel configuration entries, source-index/documentation-generation
entries, and adapter-specific CTL setup instructions are removed. Installed
kernel-module cleanup is unconditional. Shared CAM, PCI, CTL, and retained
storage drivers remain. Static integration checks passed; kernel/module builds,
SATA/NVMe/USB enumeration, ZFS import, and bhyve storage tests require FreeBSD.
The original assessment below is retained as audit history.

**Purpose and locations:** QLogic and Emulex FC adapters and firmware:
`sys/dev/isp`, `sys/dev/ispfw`, `sys/dev/ocs_fc`, their modules, and kernel
configuration entries.

**FreeBSD rationale / HalfBSD fit:** SAN connectivity and storage targets. This
is directly outside the workstation architecture.

**Dependencies:** These drivers consume CAM, PCI, firmware, and—in some
configurations—target functionality. Keep shared CAM and PCI infrastructure.
Remove adapter-specific target hooks with the drivers.

**Disposition and impact:** Delete source after removing build/configuration
entries. FC hardware stops working; no core impact expected on supported
AHCI/NVMe/USB storage.

**Benefit / risk / confidence:** High driver/firmware reduction. These three
source directories contain approximately **8.48 MB across 72 tracked files**,
excluding associated glue. Low risk under the declared hardware policy; high
confidence.

**Validation:** Kernel and all retained modules; SATA/NVMe/USB storage
enumeration, ZFS import, and bhyve storage tests.

#### A3. Obsolete parallel SCSI and hardware RAID families

**Implementation status:** Removed on October 3, 2026, at the user’s request.
Removed `aic7xxx`/`ahc`/`ahd`, `sym`, `ida`, `ips`, `mlx`, `aac`/`aacraid`,
and `hpt27xx`, `hptiop`, `hptmv`, `hptnr`, and `hptrr`, including driver-specific
Linux ioctl shims, firmware/host binary blobs, assembler tooling, `mlxcontrol`,
modules, options, configurations, generated dependency and documentation entries.
The now-unused `SOURCELESS_HOST` option and kernel configuration fragment were
removed. Installed module/tool cleanup is unconditional. CAM, DMA, PCI, disk
interfaces, and modern `mps`, `mpr`, and `mpi3mr` SAS HBAs remain. Full FreeBSD
build and storage validation remain outstanding; the original assessment below
is retained as audit history.

**Purpose and locations:** Candidates include `aic7xxx`/`ahc`/`ahd`, `sym`, `ida`,
`ips`, `mlx`, `aac`/`aacraid`, older HighPoint drivers, and their administration
tools such as `mlxcontrol`. Sources lie under `sys/dev`, `sys/modules`, and
`usr.sbin`.

**FreeBSD rationale / HalfBSD fit:** Historical servers, expansion cards, and
hardware-managed arrays. HalfBSD should expose supported disks directly to ZFS.

**Dependencies:** Shared CAM, DMA, PCI, and disk interfaces stay. Administration
programs disappear only with their matching drivers.

**Disposition and impact:** Remove coherent driver families, not CAM broadly.
Hardware ABI and device support disappear; no libc ABI change expected.

**Benefit / risk / confidence:** High cumulative maintenance/build reduction.
Low architectural risk, moderate hardware-coverage risk; high confidence for the
oldest families.

**Validation:** Supported storage-controller inventory; clean kernel/modules;
disk discovery, SMART/passthrough operations, and ZFS stress.

**Boundary:** Modern `mps`, `mpr`, and `mpi3mr` SAS HBAs belong in E, because
direct-attached ZFS storage is a legitimate workstation use.

#### A4. CardBus, PC Card, and parallel-port peripheral support

**Implementation status:** Removed on October 3, 2026, at the user’s request.
Removed CardBus/PC Card bridges, headers and bus interfaces; parallel-port
controllers, bus, printer/network/I/O, parallel PPS/clock/I2C drivers; `dumpcis`;
modules/options/configurations, header installation and documentation entries.
The retained Realtek driver no longer registers on CardBus; CardBus-only Audigy
initialization is removed. PUC retains serial ports and skips parallel ports
without renumbering mixed-card ports. The generic Ethernet/Wi-Fi hotplug helper
was renamed from `pccard_ether` to `netif_hotplug`, with matching devd/install
updates, preserving network hotplug. Shared PCI/UART/USB/USB serial, GPIO PPS,
and I2C remain. Installed cleanup is unconditional. Full FreeBSD builds and
runtime/hardware checks remain outstanding; the original assessment below is
retained as audit history.

**Purpose and locations:** `sys/dev/cardbus`, `pccard`, `pccbb`, `exca`, `ppbus`,
`ppc`; modules including `lpt`, `ppi`, `plip`; `usr.sbin/dumpcis`; startup helper
`libexec/rc/pccard_ether`.

**FreeBSD rationale / HalfBSD fit:** Laptop expansion sockets, parallel
printers, parallel networking, and old peripherals. These are poor matches for
modern amd64 hardware; LPR has already been removed.

**Dependencies:** CardBus attachment portions of otherwise useful drivers need
separate cleanup. Parallel drivers share `ppbus`; that framework can disappear
when its consumers do.

**Disposition and impact:** Delete these bus/peripheral capabilities after
attachment cleanup. Keep PCI, UART, USB, and USB serial. No core impact expected.

**Benefit / risk / confidence:** Moderate reduction of probe paths, modules, and
historical configuration. Low risk; high confidence.

**Validation:** Kernel/module build and retained Wi-Fi attachment paths; USB
printing and serial devices remain functional.

#### A5. Clearly obsolete Ethernet and Wi-Fi generations

**Purpose and locations:** Initial candidates include `le`, `dc`, `fxp`, `rl`,
`sis`, `ste`, `xl`, and legacy wireless `ipw`, `iwi`, `wpi`, `malo`, plus
corresponding firmware/modules.

**FreeBSD rationale / HalfBSD fit:** Broad support for older PCI, Fast Ethernet,
and early wireless hardware. These do not justify permanent maintenance in the
modern hardware baseline.

**Dependencies:** Preserve MII, net80211, iflib, firmware loading, LinuxKPI, and
shared driver families still serving retained devices. Some wireless families
have multiple bus attachments.

**Disposition and impact:** Remove by supported-device family. This changes
hardware availability, not ordinary socket/libc interfaces. Do not infer that
`re`, `em`, or all `ath` support is obsolete.

**Benefit / risk / confidence:** High aggregate build/maintenance reduction;
hardware coverage is the principal risk. High confidence for the listed oldest
generations.

**Validation:** NIC support matrix; Ethernet throughput, DHCP, suspend/resume,
Wi-Fi association and roaming on retained adapters.

**Implementation status (2026-10-03):** Removed the eleven families after
user review and approval, including their modules, dedicated Intel firmware,
driver tools and kernel configuration/build references. Preserved
`sys/dev/rl/if_rlreg.h`, which `re` and retained MII PHY drivers share.
MII, net80211, iflib, firmware loading, LinuxKPI, `em`, and `ath` remain.
Added unconditional installed module/manual cleanup. Repository policy and
reference checks cover the removal; full FreeBSD builds and retained-adapter
throughput, DHCP, suspend/resume, association and roaming need native testing.

#### A6. Historical non-HDA audio-controller backends

**Implementation status:** Removed on October 3, 2026, after the detailed
`A6.md` hardware review and explicit user approval of all seven families.
Removed `snd_ich`, `snd_via82c686`, `snd_via8233`, `snd_neomagic`, `snd_solo`,
`snd_t4dwave`, and `snd_vibes`, their private headers, NeoMagic coefficient
blob, modules, kernel selections and `snd_driver` dependency entries.
Installed module/manual cleanup is unconditional.

The approved hardware boundary includes early-amd64 AC’97 coverage: ICH7,
nForce4/410, AMD-8111 and VIA controllers through the explicitly named VT8251.
This is a controller-family retirement, not a blanket AC’97 removal.
Shared PCM, mixers and AC’97 helpers remain for retained drivers. HDA controller
and codec support, onboard analog/headphone/microphone/line audio, GPU
HDMI/DisplayPort, USB Audio Class, virtual audio and professional audio drivers
remain. No optical storage, CAM/SCSI, SATA/PATA or USB-drive implementation is
changed; digital CD/DVD/Blu-ray access and software playback use retained paths.

Policy/default/ALL_MODULES, source-reference, retained-source and whitespace
checks passed. Native FreeBSD builds and the requested analog output/input,
mixer/mute, AMD/Intel/NVIDIA HDMI/DisplayPort, USB playback/recording,
browser/media-player/Wayland, suspend/resume and optical-drive detection/playback
tests remain outstanding. The original assessment below is retained as history.

**Purpose and locations:** Legacy PCM implementations under `sys/dev/sound/pci`
and module selections in `sys/modules/sound/driver`. Candidates include
AC’97-era and older controllers such as `ich`, `via82c686`, `via8233`,
`neomagic`, `solo`, `t4dwave`, and `vibes`.

**FreeBSD rationale / HalfBSD fit:** Older integrated audio and sound cards.
Modern HDA/HDMI and USB audio should be the primary baseline.

**Dependencies:** Keep the sound/PCM framework, mixers, HDA, USB audio, and
virtual audio facilities. Professional audio cards require a separate hardware
policy rather than automatic deletion.

**Disposition and impact:** Delete selected hardware backends. Audio support
disappears only for those devices; no expected effect on retained desktop audio.

**Benefit / risk / confidence:** Moderate reduction. Low architectural risk,
moderate device-coverage risk; medium-high confidence.

**Validation:** HDA analog audio, AMD HDMI/DisplayPort audio, USB audio,
recording, mixer controls, and suspend/resume.

#### A7. Frame Relay and obsolete synchronous-WAN Netgraph protocols

**Revised scope:** The user’s October 3, 2026 revision in `A7.md` takes precedence
and explicitly preserves Netgraph as a programmable networking playground.
Remove only Frame Relay, LMI signaling, RFC1490 Frame Relay encapsulation, and
Cisco HDLC on synchronous WAN links.

**Implementation status:** Removed on October 3, 2026. Deleted
`sys/netgraph/ng_frame_relay*`, `ng_lmi*`, `ng_rfc1490*`, and `ng_cisco*`, their
four kernel module directories, build/options/configuration entries, dedicated
headers, and the Frame Relay example. Removed their debug-cookie/include entries
from `libnetgraph` and obsolete header-test exclusions. Installed module/header
and example cleanup is unconditional. Manual pages had already been removed;
there were no dedicated retained protocol tests to delete.

**Dependencies and retained functionality:** Reverse-dependency review found no
retained Netgraph node requiring these implementations. The only retained source
consumer of the dedicated headers was `libnetgraph`’s debug-cookie table. Generic
Netgraph infrastructure/APIs, all other node implementations, `libnetgraph`,
`ngctl`/`nghook`, remaining examples/tests, and bhyve networking remain.
Protocol identity constants used by packet capture/interface classification are
not implementations of these removed nodes and remain.

**FreeBSD rationale / HalfBSD fit:** These nodes serve legacy telecom/router
synchronous-WAN graphs, with no identified workstation, ordinary Ethernet/IP,
VM/NAT, or experimental Netgraph requirement. Removal is limited to constructing
those specific Frame Relay/Cisco-HDLC graphs.

**Benefit / risk / confidence:** Modest source/module reduction and removal of
obsolete protocol surface. Low expected risk; high confidence after checking
reverse dependencies.

**Validation:** Static reference, build-integration, cleanup and preservation
checks passed. This Linux host cannot build/run FreeBSD Netgraph modules.
Before deployment, build retained modules and userland tools, create/use
representative generic nodes, and verify retained bhyve networking paths.

#### A8. RIP daemons and IPv6 router-renumbering service

**Implementation status:** Removed on October 3, 2026, at the user's request.
Deleted `routed`, its `rtquery` tool and integrated `rdisc.c`, `route6d`,
`rip6query`, and `rrenumd`. Removed their build/dependency entries, rc scripts
and defaults, the ROUTED option, RIP package description, unused RIP header,
and obsolete crunch example entry. Installed binary, rc-script, header, and
manual cleanup is unconditional. The CARP IPv4 unicast test now routes through
the elected master explicitly instead of invoking RIP daemons.

`rdisc` implements IPv4 ICMP router discovery inside `routed`, rather than a
separate service. Removing it drops discovery on networks that rely exclusively
on those advertisements; DHCP-provided and static IPv4 default routes remain.
IPv6 discovery/SLAAC (`rtsold`, `ndp`, and kernel neighbor discovery), `route`,
shared socket/IPsec facilities, and `rtadvd` remain. The independent router
renumbering receiver in `rtadvd` is outside this daemon-removal scope.
Shell syntax, build/dependency reference, retained-networking source, installed
cleanup, and whitespace checks passed. This Linux host cannot validate a
FreeBSD world build or live DHCP/default routes, SLAAC, DNS, SSH, VM NAT,
and CARP; those checks remain required on FreeBSD before deployment.

**Purpose and locations:** `sbin/routed`, `usr.sbin/route6d`, `usr.sbin/rrenumd`,
associated rc scripts and configuration. Query tools can accompany the services
where they serve only those protocols.

**FreeBSD rationale / HalfBSD fit:** Dynamic-routing participation and router
administration. A workstation needs routes, DHCP, IPv6 neighbor discovery, and
possibly local NAT—not RIP infrastructure.

**Dependencies:** Shared socket and IPsec utilities remain independently useful.
No identified dependency from basic workstation networking.

**Disposition and impact:** `WITHOUT_ROUTED` covers the IPv4 daemon, but IPv6
services need explicit build cleanup. Delete service sources afterward. Preserve
`route`, `rtsold`, `ndp`, and IPv6.

**Benefit / risk / confidence:** Small-to-moderate reduction of privileged
network-facing services. Low risk; high confidence.

**Validation:** DHCP, default routes, IPv6 SLAAC, DNS, SSH, and VM NAT.

#### A9. VMware-specific guest devices and cloud-only NICs

**Implementation status:** Removed on October 3, 2026, at the user's request.
Deleted VMware PVSCSI, VMXNET3, VMCI, and x86 GuestRPC implementation/header,
plus ENA (including its dedicated ena-com library), GVE, and MANA drivers and
modules. Removed kernel/config/module selections, dedicated documentation
inputs, VMware Vagrant release generation, and ENA-specific release references.
Installed-module/header/manual cleanup is unconditional. Retained `vmware.h`
because CPU identification, TSC frequency, and APIC topology use its hypercalls;
generic guest detection and hardware/firmware compatibility workarounds remain.
Intel VMX under `sys/amd64/vmm` is bhyve host support and remains untouched.

Static source/build dependency, shell syntax, installed cleanup, and whitespace
checks passed. Native kernel/module/world builds and the runtime checks below
remain required on FreeBSD.

**Purpose and locations:** `sys/dev/vmware`, `sys/modules/vmware`, VMware
guest-RPC support under `sys/x86`; separately, cloud-oriented `ena`, `gve`, and
`mana` drivers.

**FreeBSD rationale / HalfBSD fit:** Running FreeBSD as a guest on VMware and
cloud platforms. Outside the physical-workstation target.

**Dependencies:** Guest devices use shared PCI/network/storage facilities.
bhyve device emulation is separate and does not require VMware guest drivers.

**Disposition and impact:** Remove guest/platform-specific code and release
references. Do not remove generic hypervisor detection or CPU workarounds by
name alone.

**Benefit / risk / confidence:** Moderate driver reduction; loss of those guest
environments. Low architectural risk; high confidence.

**Validation:** Native boot, retained NICs/storage, bhyve host operation, and any
retained virtual test environment.

#### A10. Kboot, U-Boot, and nonstandard host boot paths

**Implementation status:** Removed on October 3, 2026, at the user’s request.
Deleted Kboot/Linux-kexec, native U-Boot/ubldr, kshim, and its dependent old
standalone USB boot library/test/tool. Removed loader knobs/options/dependencies,
USB sysinit cross-tool integration, native-U-Boot loader metadata examples,
Kboot-only EFI metadata branches, and Linux/Kboot boot-test image/script paths.
Installed loaders/help/backups cleanup is unconditional. Shared `libsa`, Lua,
FDT, EFI and bhyve userboot remain. Kernel USB and EFI USB handling are unchanged.
References to external U-Boot firmware implementing UEFI, hardware vendor names,
and bhyve guest firmware are separate from native ubldr and remain. EFI
compatibility handling for firmware quirks is preserved. Static checks passed;
native EFI builds, fresh-install/boot-environment tests and bhyve userboot runtime
checks remain outstanding. The original assessment below is retained as history.

**Purpose and locations:** `stand/kboot`, `stand/kshim`, `stand/uboot`, related
options and platform glue.

**FreeBSD rationale / HalfBSD fit:** Booting through Linux/kexec or embedded
firmware. HalfBSD deliberately selects native UEFI boot.

**Dependencies:** Retain shared `libsa`, loader Lua, EFI infrastructure, and
bhyve `userboot`.

**Disposition and impact:** Exclude with relevant loader knobs, then remove
source and build integration. Native boot alternatives disappear; no libc ABI
effect.

**Benefit / risk / confidence:** Moderate configuration reduction; approximately
**0.31 MB in 81 files** in the three named directories. Low risk; high confidence.

**Validation:** EFI loader build, fresh installation, boot environments, and
bhyveload/userboot.

#### A11. Old in-tree DRM2 and AGP graphics stack

**Implementation status:** Removed on October 3, 2026, at the user’s request.
Deleted in-tree DRM2/TTM, its Tegra backend, AGP drivers/module/interfaces and
ioctl header, dedicated old-DRM generation tools, obsolete DRM2 build-option
descriptions, build/configuration/debug options, header installation, and
AGP documentation-generation/header-test entries. Installed AGP/DRM2 module
and AGP header cleanup is unconditional. `pciconf` preserves read-only AGP
capability decoding through a small private register-definition header;
it no longer includes obsolete driver/ioctl headers. Only `drm2` was removed
from the loader blacklist; current external graphics module policy remains.
LinuxKPI (including its AGP type stub), framebuffer/backlight/video-mode/console
support and external AMDGPU module names remain. Static checks passed; external
drm-kmod is not present in this checkout, so actual AMDGPU compilation/loading,
Wayland acceleration, display hotplug, console handoff and suspend/resume remain
unverified. The original assessment below is retained as audit history.

**Purpose and locations:** `sys/dev/drm2`, AGP drivers, related entries in
`sys/conf/files`, legacy graphics modules/options.

**FreeBSD rationale / HalfBSD fit:** Older graphics-driver generations.
HalfBSD’s AMD desktop depends on current external DRM drivers and LinuxKPI.

**Dependencies:** Remove the old implementation independently. Shared
framebuffer, backlight, video-mode, console, and LinuxKPI interfaces require
preservation where current drivers use them.

**Disposition and impact:** Delete DRM2 once retained graphics modules are
verified against their actual interfaces. Legacy GPU support disappears;
current drm-kmod must be rebuilt and tested.

**Benefit / risk / confidence:** DRM2 alone contains approximately **1.20 MB
across 77 files**. Low conceptual risk, moderate integration risk; high
confidence in retirement of the old implementation.

**Validation:** External AMDGPU module compilation/loading, accelerated Wayland,
display hotplug, console handoff, and suspend/resume.

#### A12. Obsolete interactive Unix conveniences and non-system application utilities

**Revised scope:** The user’s October 3, 2026 revision in `A12.md` is authoritative.
Audit the explicitly listed terminal/printing/recreational utilities and remove
their remaining dedicated integration without broadening into useful shell,
scripting, diagnostic, or text-processing tools.

**Implementation status:** Completed on October 3, 2026. Removed the remaining
`asa`, `enigma` (including its `crypt` command alias), `leave`, setuid `lock`,
`look`, `mesg`, and `ul` sources/build/dependency entries. Removed ASA tests and
mtree entries. Installed binaries/alias/tests cleanup is unconditional; `ul`
is no longer conditional obsolete cleanup under `MK_TEXTPROC`.

**Already removed:** `banner`, `biff`, `msgs`, `talk`, `write`, and the listed
`caesar`, `factor`, `fortune`, `grdc`, `morse`, `number`, `pom`, `primes`, and
`random` utilities. Their dedicated daemons/startup/examples/data and game
switches were already removed. No remaining exclusive comsat/talkd/inetd or
PAM service configuration requires deletion. Historical obsolete-file cleanup
entries remain so upgrades still remove those files.

**Dependencies and retained functionality:** No retained build/tool consumer
requires the seven removed programs. Preserve shared `libcrypt`, PAM, curses,
termcap/terminfo, TTY permissions, dictionaries and service-name/protocol data.
Dictionary files are shared data, not exclusive `look` infrastructure. Service
names such as biff/comsat/talk do not enable the removed daemons. General shell,
console, SSH, terminal, development, package-management and administrative tools
remain; `lockf` and kernel locking/writing primitives are unrelated.

**Benefit / risk / confidence:** Low expected runtime impact for the explicitly
listed utilities; high confidence after reference and dependency review.

**Validation:** Static source/build/configuration/test/cleanup and preservation
checks passed. This Linux host cannot build/install FreeBSD world or verify
retained runtime workflows; native world/install and normal workstation login,
terminal, SSH and administrative checks remain required.

#### A13. Enterprise login authentication backends

**Implementation status:** Removed from the working tree after the audit, on
October 3, 2026, at the user’s request. TACACS+, PAM RADIUS, and Hesiod source,
build integration, generated dependencies, and installed-file cleanup were
handled together. Local NSS/PAM and WPA RADIUS/EAP remain. The original
assessment below is retained as audit history.

**Purpose and locations:** `lib/libtacplus`, `lib/nss_tacplus`, PAM
`pam_tacplus`/`pam_radius`, `lib/libradius`; Hesiod code in libc and
`usr.bin/hesinfo`.

**FreeBSD rationale / HalfBSD fit:** Centralized enterprise/network-device
authentication and directory integration. Local accounts and SSH fit HalfBSD
better.

**Dependencies:** TACACS consumers are its PAM and NSS modules. Hesiod reaches
passwd/group/shell lookup. Base `libradius` is distinct from WPA’s bundled
RADIUS/EAP machinery.

**Disposition and impact:** Delete TACACS/PAM RADIUS together; exclude Hesiod
first and then clean its libc branches. Preserve NSS core and Wi-Fi enterprise
authentication. External consumers of the deleted libraries or Hesiod symbols
need rebuilding/replacement.

**Benefit / risk / confidence:** Modest source reduction and fewer authentication
protocols. Low core risk after configuration cleanup; high confidence.

**Validation:** Local passwd/group lookup, PAM login/su/sshd, password changes,
DNS, and enterprise Wi-Fi.

### B. Processed likely removal candidates requiring dependency cleanup

#### B1. Remaining InfiniBand networking and Mellanox firmware scaffolding

**Implementation status:** Removed from the working tree after the audit, on
October 3, 2026, at the user’s request. InfiniBand networking and lagg/IPv6/TCP
receive/packet-hashing integration, orphaned `mlxfw`, and stale mlx5 configuration
entries were removed. Ethernet lagg and bnxt/LinuxKPI remain. Protocol and
hardware identifier constants are retained for ABI and diagnostic recognition.
The original assessment below is retained as audit history.

**Purpose and locations:** `sys/net/if_infiniband.c`, `sys/net/infiniband.h`,
`sys/modules/if_infiniband`, InfiniBand branches in `if_lagg.c` and Ethernet
handling; `sys/dev/mlxfw` and its module.

**Why present / fit:** These supported RDMA adapters and link aggregation. The
underlying RDMA and mlx4/mlx5 implementations have already been removed.

**Dependency finding:** `if_lagg` explicitly depends on `if_infiniband`;
`sys/conf/files` also selects InfiniBand code for `lagg`. This is an active
dependency, not just forgotten source. Remaining `mlxfw` references include
configuration/build glue; no retained adapter consumer was identified.

**Disposition and impact:** Remove InfiniBand branches from retained Ethernet
aggregation before deleting its module. Delete orphaned `mlxfw` separately.
Keep Ethernet lagg if desired.

**Benefit / risk / confidence:** Modest size reduction, strong architectural
cleanup. Moderate risk; high confidence.

**Validation:** `if_lagg` module build/load, Ethernet aggregation/failover,
ARP/ND, and all retained NIC modules.

Also remove stale mlx5 entries from `sys/amd64/conf/GENERIC`. Empty mlx4
directories are not remaining tracked implementations.

#### B2. iSCSI and NVMe over Fabrics

**Implementation status:** Removed on October 3, 2026, at the user's request.
Deleted iSCSI and NVMe-oF initiators/targets/transports, ctld and its discovery,
iSNS, authentication and configuration parsers, iscsid/iscsictl, libiscsiutil,
libnvmf, NVMe-oF example tools/devd configuration, and SAN-only regression tests.
Removed Chelsio cxgbei and its iSCSI transmit glue from retained TCP offload.
Removed fabric commands from nvmecontrol and ctladm, fabric state/transport
formatting from nvmecontrol/camcontrol, CTL fabric ioctl declarations/dispatch,
and CAM registration of the retired transports. Build options, libraries,
headers, module selections, package/rc integration, and dependency metadata are
cleaned up. Installed modules/binaries/scripts/headers/manuals/test files have
unconditional cleanup; administrator-created SAN configuration is preserved.
Local NVMe/CAM, CTL local backends/frontends/tools, CTL HA (B3), ordinary ZFS,
SCSI protocol identifiers/serialization, and bhyve virtio-SCSI remain. Portable
ZFS property-test helpers and NIC firmware protocol definitions are separate
from the removed SAN implementation.

Static source/build dependency, shell syntax, installed cleanup, and whitespace
checks passed. Native kernel/module/world builds and the runtime checks below
remain required on FreeBSD.

**Purpose and locations:** `sys/dev/iscsi`, `sys/dev/nvmf`, `lib/libiscsiutil`,
`lib/libnvmf`, `iscsid`, `iscsictl`, `ctld`, transport modules, startup scripts,
and devd configuration.

**Why present / fit:** Network block-storage initiators and targets. SAN
functionality is outside HalfBSD’s storage model.

**Dependency finding:** `ctld` handles both iSCSI and NVMe-oF. Retained
`camcontrol` and `nvmecontrol` link `libnvmf`; `camcontrol` uses it to describe
NVMe-oF transports. Merely disabling `MK_ISCSI` leaves substantial functionality.

**Disposition and impact:** Remove fabric commands/transport formatting from
retained administration tools; then delete libraries, transports, daemons,
discovery/iSNS code, and configuration. Preserve local NVMe, CAM, and ordinary
ZFS.

**Benefit / risk / confidence:** High protocol/daemon reduction; the measured
protocol/library/daemon subset is approximately **0.96 MB in 81 files**.
Moderate risk; high confidence.

**Validation:** Clean world/modules; local `nvmecontrol` and `camcontrol`; ZFS
import/stress; bhyve block, NVMe, AHCI, and virtio-SCSI.

#### B9. Kernel SunRPC networking

**Implementation status:** Removed on October 3, 2026, at the user's request.
Removed kernel SunRPC transports, clients, services, authentication,
Netlink RPC transport/parser, and `krpc` source/build integration. Generic
Netlink and `genl` monitoring remain. Kernel XDR stays available for ZFS and
local consumers, with its own allocation class and no RPC transport includes.
The traditional `rpc/types.h` and `rpc/xdr.h` serialization interfaces remain.
The legacy RPC TLS syscall slot remains unimplemented with its number reserved.

Repository policy/ALL_MODULES, source/header/dependency, retained symbol and
function-body, syscall, shell-syntax, mtree and whitespace checks passed.
Full world/kernel/modules builds, package ABI/rebuild checks, ZFS and filesystem
runtime tests, SSH/pkg/networking and generic Netlink smoke tests remain
outstanding on a native FreeBSD environment. Installed-file cleanup is
unconditional. The original assessment below is retained as audit history.

**Purpose and locations:** `sys/rpc`, `sys/modules/krpc`, kernel RPC options and
Netlink RPC plumbing.

**Why present / fit:** Kernel network services, primarily the now-removed NFS
stack.

**Dependencies:** `genl` contains a specific RPC parser; VFS export code includes
RPC authentication definitions. ZFS needs XDR, **not the full RPC
transport/service implementation**.

**Disposition and impact:** Remove kernel RPC networking after separating
shared definitions/XDR and removing its diagnostic parser. Retain general
Netlink. External modules using kernel RPC lose that interface.

**Benefit / risk / confidence:** Moderate networking/kernel simplification.
Moderate risk; high confidence in candidacy.

**Validation:** ZFS kernel module, import/export and send/receive; general
`genl`/Netlink operation; clean all-module build.

#### B10. Historical RPC service library and generated interfaces

**Implementation status:** Removed on October 3, 2026, at the user's request.
Removed `librpcsvc`, `include/rpcsvc`, Secure RPC generated inputs,
service examples and RPC-specific tests. Removed dependency/build metadata and
unconditional library selection. No generated RPC source consumers remain, so
`rpcgen` and its bootstrap selection were also retired. The existing XDR
byte-vector regression now lives under `lib/libc/tests/xdr` and calls the retained
serialization primitives directly without generated code.

Repository policy/ALL_MODULES, source/header/dependency, retained symbol and
function-body, syscall, shell-syntax, mtree and whitespace checks passed.
Full world/kernel/modules builds, package ABI/rebuild checks, ZFS and filesystem
runtime tests, SSH/pkg/networking and generic Netlink smoke tests remain
outstanding on a native FreeBSD environment. Installed-file cleanup is
unconditional. The original assessment below is retained as audit history.

**Purpose and locations:** `lib/librpcsvc`, `include/rpcsvc`: remote execution,
users/status/wall, bootparam, remote quota, and Secure RPC definitions.

**Why present / fit:** Historical Unix network services whose programs have
mostly disappeared.

**Dependencies:** `rpcbind/security.c` still includes `rquota.h`; libc Secure
RPC generates code from `crypt.x`; libc RPC tests link `librpcsvc`;
`lib/Makefile` retains a PAM build-order dependency.

**Disposition and impact:** Remove obsolete services and their dependencies
together. Do not remove the entire include directory before separating libc’s
remaining build inputs.

**Benefit / risk / confidence:** Small-to-moderate reduction and good orphan
cleanup. External programs linking `librpcsvc` break; moderate risk, high
confidence.

**Validation:** Clean bootstrap/world, generated headers, libc RPC tests while
retained, and package ABI inventory.

#### B11. rpcbind, rpcinfo, Secure RPC, and libc RPC transport support

**Implementation status:** Removed on October 3, 2026, at the user's request.
Removed rpcbind/rpcinfo, rc settings and startup integration,
libc RPC transports/services, Secure RPC, RPC database lookups and `getent rpc`.
Removed transport headers, netconfig data/API selection, RPC option descriptions,
and NSS RPC configuration. Retained `bindresvport`/`bindresvport_sa` as ordinary
networking helpers under libc/net, with unchanged implementations and symbol
versions, declared in `netdb.h`. Generic XDR APIs and symbol versions remain.
Tcpdump keeps packet decoding with RPC database/header detection disabled.
This deliberately removes libc RPC symbols without compatibility stubs; external
packages using those APIs require rebuilding or replacement.

Repository policy/ALL_MODULES, source/header/dependency, retained symbol and
function-body, syscall, shell-syntax, mtree and whitespace checks passed.
Full world/kernel/modules builds, package ABI/rebuild checks, ZFS and filesystem
runtime tests, SSH/pkg/networking and generic Netlink smoke tests remain
outstanding on a native FreeBSD environment. Installed-file cleanup is
unconditional. The original assessment below is retained as audit history.

**Purpose and locations:** `usr.sbin/rpcbind`, `usr.bin/rpcinfo`,
`lib/libc/rpc`, installed RPC configuration and headers.

**Why present / fit:** Generic RPC applications and historical authentication.
With NFS/NIS removed, rpcbind has no identified required base-system service.

**Dependencies:** libc exports RPC, DES-authentication, and related APIs.
`getent` exposes RPC database lookup. `rpcgen` remains a bootstrap tool while
generated RPC sources exist. Third-party applications may use these APIs.

**Disposition and impact:** Remove rpcbind/rpcinfo first. Full libc RPC deletion
needs a deliberate ABI transition or retained compatibility implementation.
Keep XDR.

**Benefit / risk / confidence:** Moderate service/parser reduction; the measured
RPC subset is approximately **1.11 MB**, not wholly removable. Low risk for
daemons, high libc/package risk; high/medium confidence respectively.

**Validation:** Clean bootstrap, libc symbol comparison, package rebuilds, ZFS
serialization, and SSH/pkg/networking smoke tests.

#### B12. NFS export and GSS remnants

**Implementation status:** Removed on October 3, 2026, at the user's request.
Removed `vfs_export.c`, network export address/credential lists,
export-check callbacks, jail export tracking, and remaining kernel GSS headers.
Mount export options and legacy export flags return EOPNOTSUPP before a mount
is changed. Removed ZFS export-only callbacks/credential cloning and FUSE's
NFS-only implicit opens and export tests. Retained local file-handle operations,
ZFS NFSv4-style ACLs, syscall numbering, legacy mount argument layouts and numeric
flag values. Retired mount/vfsops/jail fields are reserved slots rather than
active export state; external filesystem modules need rebuilding.

Repository policy/ALL_MODULES, source/header/dependency, retained symbol and
function-body, syscall, shell-syntax, mtree and whitespace checks passed.
Full world/kernel/modules builds, package ABI/rebuild checks, ZFS and filesystem
runtime tests, SSH/pkg/networking and generic Netlink smoke tests remain
outstanding on a native FreeBSD environment. Installed-file cleanup is
unconditional. The original assessment below is retained as audit history.

**Purpose and locations:** `sys/kern/vfs_export.c`, export-related mount
interfaces, filesystem export callbacks, `sys/rpc/rpcsec_gss.h`, and the
remaining `sys/kgssapi/gssapi.h`.

**Why present / fit:** Filesystem export and authentication infrastructure left
after NFS/GSS removal. No network-export requirement remains.

**Dependencies:** VFS interfaces are shared across filesystems; ZFS still
implements file-handle and filesystem operations. File-handle syscalls may have
non-NFS administration consumers.

**Disposition and impact:** Remove proven export/GSS-only implementation while
preserving necessary VFS contracts and syscall numbering. Do not remove
NFSv4-style ACLs: ZFS uses those local permissions semantics.

**Benefit / risk / confidence:** Moderate cleanup. Kernel/module ABI risk is
significant; medium confidence until callback and file-handle consumers are
fully resolved.

**Validation:** ZFS mounting, permissions/ACLs, VFS/file tests, external modules,
and file-handle API consumers.

#### B14. Historical FreeBSD ABIs and a.out executables

**Implementation status:** Source removal implemented on October 3, 2026,
according to `B14.md`; committed at the user’s explicit request; native validation remains pending. The native ABI
floor is FreeBSD 11. Retired COMPAT_43 and COMPAT_FREEBSD4/5/6/7/9/10 options,
native dispatch and exclusive wrappers are removed; FreeBSD 8 had only feature
advertising, and FreeBSD 15 is the current ABI rather than a separate option.
Historical syscall slots remain reserved. COMPAT_FREEBSD11/12/13/14 remain.

FreeBSD32/IA32 retain their historical dispatch, layouts and shared helpers
under COMPAT_FREEBSD32. Optional 4.3BSD interfaces use COMPAT_FREEBSD32_43,
disabled by default. Private legacy declarations serve those retained callers;
shared FreeBSD-7 SysV wrappers remain without their retired native slot
registrations. Native pre-11 umtx operations are rejected while their 32-bit
implementations remain. COMPAT_43TTY remains a separate terminal interface.

Native a.out activation, module, kernel options and build integration are
removed, with unconditional installed-module cleanup. Object-format headers,
readers and generic exec tests remain for retained consumers. Linux/Linux32,
ELF, toolchain, EFI, debugging and bhyve implementation are preserved.
Published syscall numbers, libc/libsys sources and exported symbol maps remain
unchanged. The pre-removal boundary review is in `B14-compatibility-review.md`.

Source checks cover generated tables, published ABI artifacts and HalfBSD build
policy. Clean FreeBSD world/kernel/modules, staged installation, exported binary
symbol comparisons, current packages/Firefox, retained ABI execution and the
other runtime checks required by B14 remain outstanding on this Linux host.

**Purpose and locations:** `COMPAT_FREEBSD4` through older compatibility
options, compatibility syscall implementations, libc symbol maps,
`sys/modules/aout`, and executable-format support.

**Why present / fit:** Running older binaries. Compatibility with very old
applications is outside HalfBSD’s purpose, but current package compatibility
is useful.

**Dependencies:** Newer compatibility entry points may still support retained
libc symbols and packages. Native amd64 compatibility is separate from
FreeBSD32.

**Disposition and impact:** Remove ancient compatibility tiers and a.out first.
Keep syscall slots reserved; do not renumber APIs or indiscriminately remove
symbol versions.

**Benefit / risk / confidence:** Moderate-to-high simplification. Low risk for
ancient formats, high risk for recent ABI tiers; high/medium confidence.

**Validation:** libc/libsys symbol and syscall checks, clean world, current
packages, debugger/core handling, and compatibility tests appropriate to the
retained contract.

### C. Processed simplification/refactoring candidates

#### C2. Fixed HalfBSD build policy and maintained kernel configuration

**Implementation status:** Completed on October 3, 2026, at the user's request.
The default amd64 kernel is now `HALFBSD`, derived from CHARLIE with VNET,
modular IPsec/SCTP support and Wi-Fi/iflib cores enabled. CHARLIE is an include
alias; HALFBSD-DEBUG/KASAN/KCSAN/KMSAN provide purposeful diagnostic builds.
Top-level amd64 runtime builds require the core workstation features; bootstrap,
native-tool, individual-component and lib32 sub-build overrides remain usable.
Cross-host compiler/toolchain controls remain selectable. Kernel-only option
handling remains compatible with external module builds.

`sys/conf/halfbsd.modules.mk` defines the default in-tree workstation module
set, including local NVMe/CAM/CTL, ZFS, bhyve VMM, VNET plumbing, current physical
NIC/Wi-Fi families, USB devices, HDA/USB/pro audio, and LinuxKPI/graphics APIs.
GEOM defaults retain GPT/labels/ELI/mirror and useful image/test classes.
The filter respects existing architecture, option and firmware selections;
MODULES_OVERRIDE, MODULES_EXTRA, WITHOUT_MODULES, and explicit ALL_MODULES builds
remain available. Excluded implementations are not deleted by this build-policy
change; A5 source retirement was subsequently approved and implemented;
pending B3/B4/B13/B15/B16 decisions remain separate.

Removed 24 descriptions of nonexistent options; preserved meta/dirdeps controls.
Reconciled 133 committed dependency files, removing 242 stale edges and updating
32 moved-source edges, including obsolete duplicate GNU CSU and ncurses paths.
GENERIC's stale mlx5 selections had already been removed by B1. Cross-host CI
now checks the policy, builds HALFBSD and its selected modules on amd64, and
covers diagnostic variants on Ubuntu with clang 18. The arm64 cross-host build
remains pending B8. Added `tools/build/check-halfbsd.py` to validate these choices
without a compiler or object-tree writes. Corrected a pre-existing test-mtree
indentation error exposed by these checks.

BSD-make policy/override/firmware tests, source/dependency checks, shell syntax,
mtree structure, and whitespace checks passed. Full world/kernel/module builds,
staged packaging, native debug/sanitizer execution, hardware tests and external
drm-kmod/Wi-Fi module loading still require a FreeBSD build/test environment.

**Locations:** `share/mk/src.opts.mk`, `sys/conf/kern.opts.mk`,
`sys/modules/Makefile`, `sys/amd64/conf`, `targets`, CI, release packages,
option-description files.

Many features disabled in `CHARLIE` still build as modules. `GENERIC` retains
references to deleted mlx5 code. Retired-feature descriptions such as
`WITHOUT_GAMES`, `WITHOUT_FINGER`, and old DRM options remain in
`tools/build/options`.

**Disposition:** Define a supported HalfBSD feature/module set, retire
nonexistent options, regenerate dependency metadata, and keep purposeful
debug/sanitizer configurations. Avoid removing cross-host build support or
out-of-tree module conventions.

**Impact / benefit / risk / confidence:** High maintenance benefit, potentially
large build savings. Moderate integration risk; high confidence.

**Validation:** Clean world/kernel/modules, staged packaging, supported
debug/sanitizer builds, cross-host bootstrap, and external drm-kmod/Wi-Fi modules.

#### C3. Choose ULE and retire the alternate scheduler

**Implementation status:** Completed on October 3, 2026, at the user’s request.
ULE is mandatory. 4BSD and the registration/selection/dispatch layer are deleted;
ULE directly implements the existing scheduling API. Common tracing, statistics,
and CPU-topology support live in `kern_switch.c`. ULE initialization retains its
original boot stages and ordering; the empty scheduler CPU-accounting startup
callback is removed. Fixed read-only scheduler identity sysctls remain.
The original assessment below is retained as audit history. See the implementation
update above for validation and outstanding build/runtime checks.

**Locations:** `sys/kern/sched_4bsd.c`, `sched_ule.c`, scheduler
registration/interfaces, kernel options and tests.

FreeBSD retains scheduler choice; modern SMP workstations can standardize on
ULE. The current tree has scheduler registration, and GENERIC enables both
implementations.

**Dependencies and disposition:** Delete 4BSD after checking scheduler-specific
branches. Retain shared scheduling APIs, real-time behavior, affinity, and
diagnostics. Simplifying selection machinery is secondary.

**Impact / benefit / risk / confidence:** Modest build reduction and fewer
execution paths. Kernel/module interface risk; medium-high confidence.

**Validation:** SMP stress, latency under build/browser load, affinity,
real-time priorities, suspend/resume, and bhyve CPU load.

### E. Processed investigations

#### E2. Tape, media-changer, and remote-tape support

**Implementation status:** Completed according to the revised `E2.md`.
Removed the class-specific `sa` and `ch` CAM peripherals, their private SCSI
headers and changer ioctl header, kernel selections and tape options, `mt`,
`tcopy`, `rmt`, and `libmt`. CAM aggregates and libcam no longer compile the
tape/changer sources. Removed remote tape clients from dump/restore, the
rdump/rrestore aliases, distribution symlink, and obsolete dependency metadata.
Added cleanup for installed programs, libraries, private headers and manuals.

**Hardware/dependency audit:** `sa` accepts only `T_SEQUENTIAL`; `ch` accepts
only `T_CHANGER`. Neither attaches disks, optical drives or SES enclosures.
`camdd` used libmt exclusively for XML tape block-limit probing. That branch
and library dependency are removed; disk geometry/probing, CAM SCSI/NVMe I/O,
regular files, pipes and threaded copy paths remain. Tape positioning/record
handling is also removed from dd/pax/restore. File and archive operations and
local UFS dump/restore remain. These utilities default to standard streams
instead of `/dev/sa0` and no longer use the tape-specific `TAPE` setting.
Generic SCSI command definitions and passthrough remain available to retained disk and diagnostic consumers.

**Preserved boundaries:** mps, mpr, mpi3mr, SAS disks, CAM core/da/ada/nda/pass,
SES and sesutil, camcontrol, bhyve, ZFS, USB mass storage and SATA/USB optical
support remain. Optical changer scheduling in scsi_cd is preserved; it is
separate from robotic tape-library support. `sys/mtio.h` remains as compatibility
definitions required by retained compiler-rt sanitizer consumers; it supplies
no driver or tape service. `_PATH_DEFTAPE` remains in the shared compatibility
header for retained libarchive and header tests. NetBSD/Solaris/Linux
compatibility code in contributed software remains untouched. External consumers of retired libcam tape APIs
must adapt; the retained disk APIs are unchanged.

**Validation:** Build-selection, dependency/source-reference, preserved-storage
source comparisons and whitespace checks pass. Native world/kernel/module
builds, disk-copy/archive runtime tests, SAS/SES/optical discovery, ZFS and bhyve
hardware/runtime validation require FreeBSD and remain outstanding.

#### E10. Obsolete USB gadget and handheld support

**Implementation status:** Completed according to `E10.md`. Removed gadget
examples, all descriptor templates, `cfumass`, device-side RAM storage `usfs`,
and the fixed-ID `uipaq`, `uvisor`, and `urio` families after the device review
below. Removed modules, kernel selections, gadget build options, cfumass rc
configuration, and release OTG setup; added installed-artifact cleanup.
`usbtest` retains host storage, modem, and control-endpoint diagnostics.

**Preserved boundaries:** All FireWire code, modules, fwcontrol, dcons and
integration remain unchanged. Shared USB host/core, dual-role framework and
libusb ABI hooks remain; the retired template provider is absent. Host modem
drivers, ucom, bridges, tethering, HID, audio, optical and mass storage remain.
No additional modem leaves were removed: ufoma is class-based, ugensa covers
Google interfaces/calculators/GPS, and uvscom relevance needs separate review.

**Validation:** BSD make selection checks and source-preservation checks pass.
Native world/kernel/module builds and USB/FireWire hardware tests require a
FreeBSD environment and remain outstanding.

<details>
<summary>Pre-removal USB identity and dependency review</summary>

##### E10 device-ID inventory and dependency review

Prepared before source removal. IDs are `vendor:product` hexadecimal.

The bounded removal set is USB templates, four gadget examples, cfumass,
uipaq, uvisor and urio. Other host modem/serial drivers are retained.


##### uipaq — 454 exact IDs

| Vendor | Products (every listed ID) |
|---|---|
| `0104` | `00be` |
| `03f0` | `1016`, `1116`, `1216`, `2016`, `2116`, `2216`, `3016`, `3116`, `3216`, `4016`, `4116`, `4216`, `5016`, `5116`, `5216` |
| `0409` | `00d5`, `00d6`, `00d7`, `8024`, `8025` |
| `043e` | `9c01` |
| `045e` | `00ce`, `0400`, `0401`, `0402`, `0403`, `0404`, `0405`, `0406`, `0407`, `0408`, `0409`, `040a`, `040b`, `040c`, `040d`, `040e`, `040f`, `0410`, `0411`, `0412`, `0413`, `0414`, `0415`, `0416`, `0417`, `0432`, `0433`, `0434`, `0435`, `0436`, `0437`, `0438`, `0439`, `043a`, `043b`, `043c`, `043d`, `043e`, `043f`, `0440`, `0441`, `0442`, `0443`, `0444`, `0445`, `0446`, `0447`, `0448`, `0449`, `044a`, `044b`, `044c`, `044d`, `044e`, `044f`, `0450`, `0451`, `0452`, `0453`, `0454`, `0455`, `0456`, `0457`, `0458`, `0459`, `045a`, `045b`, `045c`, `045d`, `045e`, `045f`, `0460`, `0461`, `0462`, `0463`, `0464`, `0465`, `0466`, `0467`, `0468`, `0469`, `046a`, `046b`, `046c`, `046d`, `046e`, `046f`, `0470`, `0471`, `0472`, `0473`, `0474`, `0475`, `0476`, `0477`, `0478`, `0479`, `047a`, `047b`, `04c8`, `04c9`, `04ca`, `04cb`, `04cc`, `04cd`, `04ce`, `04d7`, `04d8`, `04d9`, `04da`, `04db`, `04dc`, `04dd`, `04de`, `04df`, `04e0`, `04e1`, `04e2`, `04e3`, `04e4`, `04e5`, `04e6`, `04e7`, `04e8`, `04e9`, `04ea` |
| `049f` | `0003`, `0032` |
| `04a4` | `0014` |
| `04ad` | `0301`, `0302`, `0303`, `0306` |
| `04b7` | `0531` |
| `04c5` | `1058`, `1079` |
| `04da` | `2500` |
| `04dd` | `9102`, `9121`, `9123`, `9151`, `91ac`, `9242` |
| `04e8` | `5f00`, `5f01`, `5f02`, `5f03`, `5f04`, `6611`, `6613`, `6615`, `6617`, `6619`, `661b`, `662e`, `6630`, `6632` |
| `04f1` | `3011`, `3012` |
| `0502` | `1631`, `1632`, `16e1`, `16e2`, `16e3` |
| `0536` | `01a0` |
| `0543` | `0ed9`, `1527`, `1529`, `152b`, `152e`, `1921`, `1922`, `1923` |
| `05e0` | `2000`, `2001`, `2002`, `2003`, `2004`, `2005`, `2006`, `2007`, `2008`, `2009`, `200a` |
| `067e` | `1001` |
| `07cf` | `2001`, `2002`, `2003` |
| `0930` | `0700`, `0705`, `0706`, `0707`, `0708`, `0709`, `070a`, `070b` |
| `094b` | `0001` |
| `0960` | `0065`, `0066`, `0067` |
| `0961` | `0010` |
| `099e` | `0052`, `4000` |
| `0b05` | `4200`, `4201`, `4202`, `420f`, `9200`, `9202` |
| `0bb4` | `00ce`, `00cf`, `0a01`, `0a02`, `0a03`, `0a04`, `0a05`, `0a06`, `0a07`, `0a08`, `0a09`, `0a0a`, `0a0b`, `0a0c`, `0a0d`, `0a0e`, `0a0f`, `0a10`, `0a11`, `0a12`, `0a13`, `0a14`, `0a15`, `0a16`, `0a17`, `0a18`, `0a19`, `0a1a`, `0a1b`, `0a1c`, `0a1d`, `0a1e`, `0a1f`, `0a20`, `0a21`, `0a22`, `0a23`, `0a24`, `0a25`, `0a26`, `0a27`, `0a28`, `0a29`, `0a2a`, `0a2b`, `0a2c`, `0a2d`, `0a2e`, `0a2f`, `0a30`, `0a31`, `0a32`, `0a33`, `0a34`, `0a35`, `0a36`, `0a37`, `0a38`, `0a39`, `0a3a`, `0a3b`, `0a3c`, `0a3d`, `0a3e`, `0a3f`, `0a40`, `0a41`, `0a42`, `0a43`, `0a44`, `0a45`, `0a46`, `0a47`, `0a48`, `0a49`, `0a4a`, `0a4b`, `0a4c`, `0a4d`, `0a4e`, `0a4f`, `0a50`, `0a51`, `0a52`, `0a53`, `0a54`, `0a55`, `0a56`, `0a57`, `0a58`, `0a59`, `0a5a`, `0a5b`, `0a5c`, `0a5d`, `0a5e`, `0a5f`, `0a60`, `0a61`, `0a62`, `0a63`, `0a64`, `0a65`, `0a66`, `0a67`, `0a68`, `0a69`, `0a6a`, `0a6b`, `0a6c`, `0a6d`, `0a6e`, `0a6f`, `0a70`, `0a71`, `0a72`, `0a73`, `0a74`, `0a75`, `0a76`, `0a77`, `0a78`, `0a79`, `0a7a`, `0a7b`, `0a7c`, `0a7d`, `0a7e`, `0a7f`, `0a80`, `0a81`, `0a82`, `0a83`, `0a84`, `0a85`, `0a86`, `0a87`, `0a88`, `0a89`, `0a8a`, `0a8b`, `0a8c`, `0a8d`, `0a8e`, `0a8f`, `0a90`, `0a91`, `0a92`, `0a93`, `0a94`, `0a95`, `0a96`, `0a97`, `0a98`, `0a99`, `0a9a`, `0a9b`, `0a9c`, `0a9d`, `0a9e`, `0a9f`, `0bce` |
| `0bf8` | `1001` |
| `0c44` | `03a2` |
| `0c8e` | `6000` |
| `0cad` | `9001` |
| `0f4e` | `0200` |
| `0f98` | `0201` |
| `0fb8` | `3001`, `3002`, `3003`, `4001` |
| `1066` | `00ce`, `0300`, `0500`, `0600`, `0700` |
| `1114` | `0001`, `0004`, `0006` |
| `1182` | `1388` |
| `11d9` | `1002`, `1003` |
| `1231` | `ce01`, `ce02` |
| `1690` | `0601` |
| `22b8` | `4204`, `4214`, `4224`, `4234`, `4244` |
| `3340` | `011c`, `0326`, `0426`, `043a`, `051c`, `053a`, `071c`, `0b1c`, `0e3a`, `0f1c`, `0f3a`, `1326`, `191c`, `2326`, `3326` |
| `3708` | `20ce`, `21ce` |
| `4113` | `0210`, `0211`, `0400`, `0410` |
| `413c` | `4001`, `4002`, `4003`, `4004`, `4005`, `4006`, `4007`, `4008`, `4009` |
| `4505` | `0010` |
| `5e04` | `ce00` |

Host-side fixed-ID Windows CE/Pocket PC/Windows Mobile synchronization,
including Compaq/HP iPAQ, Dell Axim, Casio, Acer/ASUS, HTC, Mio/MiTAC,
Psion and other handheld/data-collector USB Sync products. The table names
Pocket PC 2002/2003 and legacy smartphone/GPS/industrial handheld families;
approximate generation is late 1990s through 2000s. It is not a USB serial
bridge or generic CDC matcher. Removal loses these products’ serial sync and
legacy HTC modem mode, not contemporary CDC/RNDIS/NCM/MBIM/iPhone tethering.

##### uvisor — 25 exact IDs

| Vendor | Products (every listed ID) |
|---|---|
| `04e8` | `6601` |
| `054c` | `0038`, `0066`, `0095`, `009a`, `00da`, `0169` |
| `081e` | `df00` |
| `082d` | `0100`, `0200`, `0300` |
| `0830` | `0001`, `0002`, `0003`, `0020`, `0031`, `0040`, `0050`, `0060`, `0061`, `0070` |
| `091e` | `0004` |
| `0e67` | `0002` |
| `12ef` | `0100` |
| `4766` | `0001` |

Host-side Palm/Visor/Treo, Sony CLIE, Garmin iQue, Fossil WristPDA,
AlphaSmart Dana, Aceeca MEZ1000 and Tapwave Zodiac sync interfaces.
Approximate generation: late 1990s/2000s. Fixed product-ID protocol, not
generic serial or contemporary phone tethering. Removal loses Palm OS sync.

##### urio — 3 exact IDs

| Vendor | Products (every listed ID) |
|---|---|
| `045a` | `5001`, `5002` |
| `0841` | `0001` |

Host-side Diamond Rio 500/600/800 portable music players (around 2000).
Fixed product-ID proprietary transfer/control interface, not generic storage
or USB Audio Class. Removal loses the Rio-specific device/ioctl interface.

##### uvscom — 5 exact IDs

| Vendor | Products (every listed ID) |
|---|---|
| `05db` | `0003`, `0005`, `0009`, `000a`, `0011` |

Retained pending a separate hardware relevance decision: SUNTAC U-Cable
A4/D2/P1, Ir-Trinity and Slipper U. The source identifies particular legacy
products, but this review does not establish whether a useful serial/modem
application remains. No uvscom IDs or implementation will be removed.

##### Gadget/device-mode targets

Gadget audio, keyboard, mouse and modem examples match device-mode interface
classes rather than physical peripheral VIDs/PIDs. Templates supply configurable
identities; the defaults below describe the HalfBSD machine as a USB peripheral.
These do not match attached USB host peripherals. cfumass likewise matches a
device-mode SCSI bulk-only mass-storage interface; it depends on USB, template
descriptors and CTL, and is not umass or an optical-storage driver.

| Template | Default vendor:product |
|---|---|
| audio | `16c0:27e0` |
| cdce | `16c0:27e1` |
| cdceem | `16c0:27df` |
| kbd | `16c0:27db` |
| midi | `16c0:27de` |
| modem | `16c0:27dd` |
| mouse | `16c0:27da` |
| msc | `16c0:27df` |
| mtp | `16c0:27e2` |
| multi | `16c0:05dc` |
| phone | `16c0:05dc` |
| serialnet | `16c0:05dc` |

Shared dependencies retained: USB host controllers/core, ucom/tty, PCI/DMA,
CAM/CTL, host storage, HID/input, audio, networking, hotplug and power management.
No retained driver includes the handheld implementations. uipaq/uvisor depend
on ucom; urio depends on USB; these are consumers, not shared providers.

Reverse integration needing cleanup: module/conf selectors, usb_serial.c
selection alternatives, private handheld ioctl header installation, gadget
option descriptions, ARM gadget release setup and usbtest’s device-side menus.
usbtest’s host storage/modem/control tests and USB diagnostics remain.

All host modem families remain: u3g/umb/umodem/ufoma/ugensa/usie/uvscom and
serial bridges. ugensa includes a Google vendor-class match and an HP calculator;
ufoma matches an interface protocol instead of fixed obsolete IDs. No generic
modem/serial driver is inferred removable from its name or age.

FireWire, fwcontrol, dcons and all their source/build/configuration integration
are expressly outside this removal; preservation will be checked against HEAD.


**Additional device-mode leaf:** `usfs` / `ustorage_fs.c` matches only
`USB_MODE_DEVICE` and generic mass-storage BBB interfaces; it has no physical
VID/PID table. It exposes RAM-backed storage as a peripheral and depends only
on the USB framework/template descriptors. Its module, NOTES entry, and source
record are the only reverse consumers. Remove this gadget while retaining
host `umass` and host mass-storage diagnostics.

</details>
