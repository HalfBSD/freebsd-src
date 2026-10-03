# HalfBSD architectural removal audit

Audit date: October 3, 2026.
Source snapshot: `e1444ad6eb26`.

This is an architectural assessment, not authorization to remove the listed
components. No source changes were made during the audit.

**NFS remains excluded.** The user corrected the NFS requirements in `bar.txt`:
HalfBSD does not want NFS in the source tree at all. This audit follows that
correction.

The strongest next cuts are SAN functionality, legacy hardware families,
alternate platform/boot support, orphaned networking infrastructure, and
historical authentication services. UFS, Netgraph, RPC, and compatibility layers
offer further reductions, but their dependencies prevent treating them as simple
directory deletions.

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

## Implementation update — October 3, 2026

The follow-up removals A1 (HAST), A2 (QLogic/Emulex Fibre Channel),
A3 (legacy parallel SCSI and RAID), A4 (CardBus/PC Card and parallel ports),
A7 (the four legacy synchronous-WAN Netgraph leaf nodes per `A7.md`),
A10 (Kboot/native U-Boot/standalone USB boot), A11 (in-tree DRM2 and AGP),
A12 (the revised utility list per `A12.md`),
A13 (TACACS+, PAM RADIUS, and Hesiod),
B1 (remaining InfiniBand networking and Mellanox firmware support),
A8 (RIP/IPv4 router discovery), A9 (VMware/cloud drivers), B2 (SAN protocols),
C2 (maintained workstation build/kernel/module policy), and C3
(the alternate scheduler and scheduler-selection machinery) are implemented.
Their original assessments below remain as audit history; completed items are
no longer pending removal candidates. NFS remains removed.

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

## A. Strong removal candidates

### A1. HAST distributed block replication

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

### A2. Fibre Channel support

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

### A3. Obsolete parallel SCSI and hardware RAID families

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

### A4. CardBus, PC Card, and parallel-port peripheral support

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

### A5. Clearly obsolete Ethernet and Wi-Fi generations

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

### A6. Historical audio-controller backends

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

### A7. Frame Relay and obsolete synchronous-WAN Netgraph protocols

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

### A8. RIP daemons and IPv6 router-renumbering service

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

### A9. VMware-specific guest devices and cloud-only NICs

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

### A10. Kboot, U-Boot, and nonstandard host boot paths

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

### A11. Old in-tree DRM2 and AGP graphics stack

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

### A12. Obsolete interactive Unix conveniences and non-system application utilities

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

### A13. Enterprise login authentication backends

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

## B. Likely removal candidates requiring dependency cleanup

### B1. Remaining InfiniBand networking and Mellanox firmware scaffolding

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

### B2. iSCSI and NVMe over Fabrics

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

### B3. CTL high availability and network-target infrastructure

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

### B4. Non-ZFS GEOM volume-management classes

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

### B5. GEOM Gate remote block access

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

### B6. UFS/FFS as a native storage architecture

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

### B7. UFS quotas, snapshots, and administration tools

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

### B8. Non-amd64 operating-system targets

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

### B9. Kernel SunRPC networking

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

### B10. Historical RPC service library and generated interfaces

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

### B11. rpcbind, rpcinfo, Secure RPC, and libc RPC transport support

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

### B12. NFS export and GSS remnants

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

### B13. OpenBSM auditing

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

### B14. Historical FreeBSD ABIs and a.out executables

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

### B15. VirtIO guest framework and guest-only filesystems

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

### B16. Enterprise network offload paths

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

## C. Simplification/refactoring candidates

### C1. Installer, release, NanoBSD, and diskless infrastructure

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

### C2. Fixed HalfBSD build policy and maintained kernel configuration

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
change; pending A5/B3/B4/B13/B15/B16 decisions remain separate.

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

### C3. Choose ULE and retire the alternate scheduler

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

### C4. Console backends and virtual-terminal policy

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

### C5. Wi-Fi security and mode selection

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

### C6. Privilege and service defaults

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

### C7. Toolchain implementation and target selection

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

## D. Keep

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

## E. Investigate further

### E1. Linux executable emulation and 32-bit runtime

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

### E2. Modern SAS HBAs, enclosure services, and tape support

**Locations:** `mps`, `mpr`, `mpi3mr`, `ses`, `sa`, `ch`, `sesutil`, `mt`,
`tcopy`, `rmt`, `libmt`.

SAS HBAs can expose disks directly to ZFS. Tape and changers are stronger removal
candidates, but backup/recovery policy should decide them.

**Dependencies / disposition:** `camdd` links `libmt`, so deleting that library
requires cleanup even if tape devices disappear. Remove `rmt` if its
remote-backup use is excluded; preserve CAM and applicable direct-storage
drivers.

**Benefit / risk / confidence:** Moderate reduction, potentially lost
backup/storage capability. Medium confidence.

**Validation:** Supported storage inventory, `camdd`, CAM tests, backup/restore
workflow, and ZFS disks.

### E3. GELI and encrypted swap

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

### E4. SCTP and IPsec

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

### E5. MAC policies, resource accounting, and jails

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

### E6. Netgraph, netmap, and network-appliance extensions

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

### E7. Bluetooth

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

### E8. NSS caching and small compatibility libraries

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

### E9. Remaining historical utilities and formatting pipelines

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

### E10. USB gadget, obsolete handheld/modem devices, and FireWire

**Locations:** USB gadget/template modules, `cfumass`, `uipaq`, `uvisor`, `urio`,
modem-specific drivers; `sys/dev/firewire`, `fwcontrol`, dcons.

USB device-mode implementations and historical handheld protocols are poor fits.
But USB serial is essential for embedded development, modern USB networking can
provide tethering, and FireWire has niche audio/debugging uses.

**Dependencies / disposition:** Remove gadget examples, obsolete device
families, and `cfumass` service where hardware policy excludes device mode.
Preserve USB host infrastructure, ucom and common serial bridges, tethering, and
diagnostic facilities actually used.

**Benefit / risk / confidence:** Moderate driver reduction;
peripheral/development risk. Medium confidence.

**Validation:** USB storage, serial development boards, phone tethering, audio,
HID, suspend/resume, and any retained remote debugging.

## Recommended removal order and validation

Prioritize the work as follows:

1. **Finish proven orphan cleanup:** mlx5 configuration references, `mlxfw`,
   InfiniBand branches, obsolete RPC service definitions, and retired-option/build
   metadata.
2. **Remove bounded capabilities:** HAST, FC, oldest controller/NIC/audio
   families, CardBus/parallel support, historical Netgraph protocols, RIP
   services, and obsolete terminal utilities.
3. **Retire SAN transports:** clean retained administration-tool dependencies,
   then remove iSCSI/NVMe-oF and CTL network/HA frontends.
4. **Establish one supported build/install architecture:** amd64, UEFI, GPT,
   ZFS, explicit module selections, and validated recovery media.
5. **Remove alternate native storage:** UFS and unwanted GEOM classes after
   installation/recovery and encryption dependencies are resolved.
6. **Address ABI-sensitive reductions separately:** libc RPC, auditing,
   historical compatibility, Linux/32-bit runtimes, and framework-level pruning.

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
