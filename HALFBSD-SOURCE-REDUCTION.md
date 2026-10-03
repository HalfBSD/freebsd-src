# HalfBSD source-tree reduction

Measured on October 3, 2026, using these fixed snapshots:

- Original: `releng/15.1` at `e38a7085f3a8ecd317c947591fe42b5ca9ab317b`.
- HalfBSD: `halfbsd2` at `18799602c9b05a998468d3b9ff453c92e222409e`.

Both snapshots include the FreeBSD 15.1-RELEASE-p4 security and errata
updates. The HalfBSD snapshot includes the additional October 2–3 removals
and compatibility/build repairs. Measurements include the report and review
files as committed at that snapshot; they exclude this document update and
the staged deletion of `HALFBSD-CLEANUP-REPORT.txt` and
`HALFBSD-GROUP-B-REVIEW.txt`.

## Size and change estimates

HalfBSD has removed roughly **16% of the files and 12% of the tree's
content** relative to the original.

| Measure | Original | HalfBSD | Net reduction |
|---|---:|---:|---:|
| Tracked files | 109,076 | 91,804 | 17,272 (15.8%) |
| Uncompressed file content | 1,538,792,947 bytes | 1,361,564,761 bytes | 177,228,186 bytes (11.5%) |
| Text lines | 41,835,868 | 36,621,924 | 5,213,944 (12.5%) |

File-path accounting:

- 17,292 original paths were deleted (15.9% of original files).
- 1,383 existing paths were modified (1.3% of original files).
- 20 paths were added.
- 90,401 original paths are unchanged (82.9% of original files).
- Altogether, 17.1% of original file paths were deleted or modified.

Git's default rename-aware diff reports 18,686 changed files,
27,023 inserted lines, and 5,241,069 deleted lines. This differs from the
path accounting because Git recognizes some moves as renames. Git's line
counts also use its own text/binary classification and diff rules.

### Largest directory reductions

MB below means 1,000,000 bytes. Reductions are net original-minus-current
content sizes, including edits and additions within each directory.

| Directory | Net content removed |
|---|---:|
| `share` | 54.8 MB |
| `crypto` | 40.9 MB |
| `contrib` | 29.3 MB |
| `sys` | 26.8 MB |
| `lib` | 7.7 MB |
| `usr.sbin` | 6.9 MB |
| `usr.bin` | 3.4 MB |
| `sbin` | 3.0 MB |
| `stand` | 1.6 MB |
| `libexec` | 1.1 MB |
| `tests` | 0.8 MB |
| `bin` | 0.5 MB |

## Functionality removed from the base system

This inventory describes the combined result of the branch's removal
commits and the current source tree. Names identify base-system components;
they do not imply that equivalent third-party software cannot be installed.

### Architectures, boot, and virtualization

- PowerPC build targets, kernel sources, boot components, userland runtime
  support, diagnostic backends, and release/CI entry points.
- RISC-V build targets, architecture sources, and minidump support.
- Legacy x86 BIOS boot loaders and the `boot0cfg` utility. UEFI boot remains.
  Installer and NanoBSD integration now use UEFI; the ZFS installer defaults
  to GPT/UEFI and rejects retired boot methods before changing disks.
- Open Firmware loader support and remaining PowerPC/RISC-V build metadata,
  native crypto, sound, tracing, and diagnostic remnants.
- Forth boot-loader implementation and integration. Lua loader support remains.
- PXE boot, TFTP client/server support, and associated boot-loader integration.
- BOOTP, bootparam, remote boot, and reverse ARP services: `bootpd`,
  `bootparamd`, `rbootd`, and `rarpd`.
- Xen guest drivers, hypercalls, loader integration, and Xen-specific tools.
  Direct PVH boot, Firecracker kernel/image support, and its UART workaround
  were also removed on October 2, 2026. The LinuxKPI `xen/xen.h` compatibility
  header
  and the exported `lkpi_xen_initial_domain()` and `lkpi_xen_pv_domain()`
  functions were restored on October 2, 2026, after the Xen removal caused
  AMDGPU module loading to fail with an undefined `lkpi_xen_initial_domain`
  symbol. Both queries always return `false`; they preserve the interface
  expected by external drivers such as drm-kmod without restoring Xen
  guest support. Source checks verified the restored header, function
  definitions, and existing kernel/module build integration; kernel
  compilation and hardware testing remain unverified.
- Hyper-V drivers, guest utilities, and integration services.
- The `nuageinit` cloud-init implementation and startup integration.

### Networking and network filesystems

- PF and IPFILTER firewall engines, modules, control tools, proxies, and
  associated libraries; PF-dependent ALTQ support and remaining ALTQ
  scaffolding. IPFW and dummynet remain.
- NFS client/server implementation and related mount, administration,
  locking, status, quota, and GSS support, plus leftover WebNFS state and
  NFS-specific tests. Local NFSv4-style ACL support remains.
- RPC-over-TLS kernel integration, `rpc.tlsclntd`/`rpc.tlsservd`, their startup
  services, and the NFS callback RPC client. Generic RPC, rpcbind, and
  general kernel TLS support remain.
- SMBFS/SMB1 client support, `mount_smbfs`, `smbutil`, and supporting library code.
- Userland PPP, PPPoE daemon, Bluetooth PPP daemon, kernel Netgraph PPP
  components, SLIP, and associated dial-up utilities and integration.
- OFED/InfiniBand/RDMA libraries, tools, kernel integration, OpenSM,
  diagnostic programs, and management components. Later cleanup removed
  orphaned `bnxt_re`, `iw_cxgbe`, `irdma`, `mthca`, `qlnxr`, and `krping`
  sources/modules, private RDMA bridges in the bnxt/ice/qlnx Ethernet drivers,
  and iSER transport/discovery paths. The bnxt, ice, qlnx, and cxgbe Ethernet
  drivers and ordinary TCP iSCSI support remain.
  On October 3, 2026, after the measured snapshots, remaining InfiniBand
  networking, its lagg/IPv6/packet-hashing hooks, and orphaned `mlxfw`
  sources/modules were removed. Ethernet lagg, LinuxKPI, and bnxt remain;
  bnxt build flags no longer depend on retired OFED scaffolding.
- The mlx5 driver source tree, its Ethernet and hardware-offload components,
  kernel modules, and `mlx5tool`, removed by `f779687025e8` ("Updates").
  The mlx4 source tree, modules, and build integration were also removed
  on October 2, 2026.
- The `inetd` superserver and its configuration/startup integration.
- Telnet client/server sources and supporting library code.
- The legacy remote-command suite and related remote status, user-listing,
  and broadcast tools/services, including `rup`, `ruptime`, `rusers`,
  `rwall`, `rwho`, and `rwhod`.
- The base FTP client (`ftp`/tnftp). The base `ftpd` daemon had already been
  removed upstream by `259bb93b80c0` ("Remove ftpd(8)") and is absent from
  both measured snapshots; its removal is not attributed to HalfBSD here.
  The bundled Heimdal FTP server sources were removed with Heimdal.
- Finger client/server and talk client/server.
- SNMP daemon, tools, and supporting bsnmp/libbegemot libraries.
- TCP wrappers, `libwrap`, `tcpd` utilities, and wrapper integration in sshd
  and rpcbind.
- Blacklist/blocklist daemons, control tools, libraries, helpers, and OpenSSH
  authentication reporting/configuration integration.
- The RPC traffic-generation utilities `spray` and `rpc.sprayd`.

### High-availability storage

- HAST distributed block replication, its daemon and control utility, UCARP
  failover examples, startup configuration, build option, package metadata,
  and dedicated account/group were removed on October 3, 2026. Cleanup of
  previously installed HAST programs, startup script, and examples is
  unconditional. Shared GEOM and `libpjdlog` remain for retained consumers.
  This removal is later than the fixed snapshots measured above.

### Authentication and directory services

- TACACS+ libraries and NSS/PAM modules, PAM RADIUS and its base library,
  and Hesiod lookup support were removed on October 3, 2026, after the
  fixed snapshots measured above. Local NSS/PAM and WPA RADIUS/EAP remain.
  Existing consumers of these retired libraries or optional Hesiod libc
  symbols require replacement or rebuilding.

- MIT Kerberos, Heimdal Kerberos, kernel Kerberos support, Kerberos PAM
  integration, and associated GSS/RPCSEC_GSS and error-table components.
- NIS/YP clients, servers, libraries, account/password services, and tools.

### Mail and printing

- Sendmail and its libraries, configuration data, administrative tools,
  mail wrapper, and restricted shell.
- DMA mail transport and base mail programs/services, including `mail`,
  `rmail`, `mail.local`, `biff`/`comsat`, `from`, and `vacation`.
- Legacy LPR printing system and its spooler/administrative integration,
  plus the `lptcontrol` parallel-port control utility.

### Console, hardware, and system administration

- The legacy syscons console, its data, and libvgl. The vt console remains;
  its startup service was renamed from `syscons` to `vtcons`.
- The default `/etc/ttys` configuration now provides only the `ttyv0` local
  session, automatically logged in through `/etc/rc.console`. Set
  `console_user="charlie"` in `/etc/rc.conf` to select an existing account;
  an unset, empty, or invalid account name falls back to root. The helper
  loads the standard rc configuration, including `/etc/rc.conf.local`, and
  runs `/usr/bin/login -f` for the selected user on each new session.
  Init prepares the controlling terminal and resets its modes before starting
  login; exiting the session starts a fresh automatic login. Extra virtual-terminal,
  serial-terminal, and other console login entries were removed on October 2,
  2026. The non-login `console` entry remains for single-user authentication
  policy. The setting does not change the single-user recovery shell.
  Wayland startup remains manual.
- The getty program, gettytab configuration, build integration, and dependent
  installer auxiliary consoles and NanoBSD tty overrides were removed.
  Installed copies are listed for cleanup in `ObsoleteFiles.inc`.
  Validation covered FreeBSD amd64 init compilation, argument handling and
  terminal setup failure paths in a host test harness, shell syntax, and
  build/configuration checks. Full world builds and boot testing remain
  unverified.
- **Follow-up to consider:** remove the unused vt kernel virtual console slots.
  They remain available for now; only `ttyv0` has a configured login session.
- The periodic maintenance framework, its daily/weekly/monthly/security
  scripts, configuration, and cron schedules. Cron itself remains.
- Serial communication tools `tip`/`cu`, modem dialers, `comcontrol`, and
  legacy serial startup configuration.
- The `uhso` USB modem driver and `uhsoctl` control utility.
- The `moused` mouse daemon and its service integration.
- The `msconvd` console mouse service and related files.
- APM power-management support and utilities. ACPI remains.
- Legacy floppy controller support and `fdcontrol`, `fdformat`, `fdread`,
  and `fdwrite`.
- Legacy disk-management utilities `fdisk` and `bsdlabel`.
- The `chio` medium-changer utility and `pnpinfo` PnP inspection utility.
- The `domainname` utility.
- The `/rescue` system and its build integration.
- `freebsd-update` and its `phttpget` download helper.
- Message-of-the-day support, removed by `9597cc59e74c`.
- Terminal messaging utilities `wall`, `write`, and `msgs`.

### User tools, documentation, and locale data

- Shared documentation (`share/doc`), historical BSD papers and manuals,
  and NTP HTML documentation and assets, removed October 3, 2026. The
  `HTML` and `SHAREDOCS` build options and troff document build support
  are retired; licenses alongside retained third-party sources remain.
- The tcsh shell (`csh`/`tcsh` in the base system).
- The `ee` editor.
- The `locate` database search utility, database-building helpers, and
  update integration, plus the `banner` utility.
- GNU diff sources. The base BSD diff implementation remains.
- GNU dialog and its `dpv`/libdpv/libfigpar components.
- Games and associated data/utilities, including `fortune`, `caesar`,
  `factor`, `grdc`, `morse`, `number`, `pom`, `primes`, and `random`.
- The `calendar` reminder utility and its data/integration, plus the
  `cal`/`ncal` display commands, their tests, and `libcalendar` helper.
- Installed manual-page readers, lookup/indexing utilities, manual sources,
  manual build rules, and mandoc sources.
- Most locale definitions and aliases. The remaining locale source data
  centers on `C.UTF-8` and `en_US.UTF-8`; timezone data remains separate
  and was updated to 2026d.

Other deleted content includes obsolete GNU scaffolding, upstream CI and
contribution metadata, historical release/update notes, and temporary
removal-planning documents, retired subsystem accounts/groups, unused MAC
callbacks, and stale build/package/upgrade metadata. These contribute to the
size reduction but do not represent separate runtime functionality.

## Scheduler simplification after the measured snapshots

On October 3, 2026, C3 made ULE the mandatory scheduler. The 4BSD implementation,
scheduler-choice options, boot-time registry/selector, operation table, and
scheduler dispatch shims were removed. ULE now directly exports the existing
scheduling API. Shared tracing, statistics, and CPU-topology diagnostics remain;
read-only scheduler identity sysctls report `ULE`. Existing custom kernel
configurations must remove `SCHED_ULE` and `SCHED_4BSD` options. These changes
are later than the fixed snapshots measured above and are not included in their
size totals. Full FreeBSD builds and runtime scheduler testing remain required.

## Measurement method and limits

- Count tracked blob entries and their uncompressed sizes with
  `git ls-tree -r -l` at each fixed snapshot. Include symlink blobs and
  bundled code, documentation, data, tests, and binary assets.
- Compare paths directly for added/deleted files and blob IDs/sizes for
  modified files. Moves count as a deletion and an addition in this method.
- Read blobs with `git cat-file --batch`. For the text-line estimate,
  exclude blobs containing NUL bytes; count newline characters and any
  final unterminated line. This yields 721 binary-classified files in the
  original and 532 in HalfBSD.
- Obtain Git's separate diff statistics with `git diff --shortstat`
  between the two snapshots.
- Sum file sizes by top-level directory to obtain directory reductions.
- Review branch commit history and deleted source/build paths to identify
  removed functionality, including removals hidden under generic commit titles.

These are repository-content measurements, not installed-system size,
executable-code-only measurements, build-time estimates, or a measurement of
security attack-surface reduction. Git history and build artifacts are excluded.
