# HalfBSD source-tree reduction

Measured on October 2, 2026, using these fixed snapshots:

- Original: `releng/15.1` at `e38a7085f3a8ecd317c947591fe42b5ca9ab317b`.
- HalfBSD: `halfbsd2` at `fc9e45d1f9055c0bb92e71c4d391f73d41d0bbb5`.

Both snapshots include the FreeBSD 15.1-RELEASE-p4 security and errata
updates. These measurements precede the addition of this report.

## Size and change estimates

HalfBSD has removed roughly **15% of the files and 11% of the tree's
content** relative to the original.

| Measure | Original | HalfBSD | Net reduction |
|---|---:|---:|---:|
| Tracked files | 109,076 | 93,242 | 15,834 (14.5%) |
| Uncompressed file content | 1,538,792,947 bytes | 1,376,592,186 bytes | 162,200,761 bytes (10.5%) |
| Text lines | 41,835,868 | 37,075,865 | 4,760,003 (11.4%) |

File-path accounting:

- 15,854 original paths were deleted (14.5% of original files).
- 1,304 existing paths were modified (1.2% of original files).
- 20 paths were added.
- 91,918 original paths are unchanged (84.3% of original files).
- Altogether, 15.7% of original file paths were deleted or modified.

Git's default rename-aware diff reports 17,167 changed files,
25,181 inserted lines, and 4,785,286 deleted lines. This differs from the
path accounting because Git recognizes some moves as renames. Git's line
counts also use its own text/binary classification and diff rules.

### Largest directory reductions

MB below means 1,000,000 bytes. Reductions are net original-minus-current
content sizes, including edits and additions within each directory.

| Directory | Net content removed |
|---|---:|
| `share` | 51.0 MB |
| `crypto` | 40.9 MB |
| `contrib` | 26.5 MB |
| `sys` | 19.3 MB |
| `lib` | 7.7 MB |
| `usr.sbin` | 6.7 MB |
| `sbin` | 3.0 MB |
| `usr.bin` | 3.0 MB |
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
- Forth boot-loader implementation and integration. Lua loader support remains.
- PXE boot, TFTP client/server support, and associated boot-loader integration.
- BOOTP, bootparam, remote boot, and reverse ARP services: `bootpd`,
  `bootparamd`, `rbootd`, and `rarpd`.
- Xen guest drivers, hypercalls, loader integration, and Xen-specific tools.
  Generic PVH boot remains for non-Xen use, including Firecracker.
- Hyper-V drivers, guest utilities, and integration services.
- The `nuageinit` cloud-init implementation and startup integration.

### Networking and network filesystems

- PF and IPFILTER firewall engines, modules, control tools, proxies, and
  associated libraries; PF-dependent ALTQ support. IPFW and dummynet remain.
- NFS client/server implementation and related mount, administration,
  locking, status, quota, and GSS support. Local NFSv4-style ACL support remains.
- SMBFS/SMB1 client support, `mount_smbfs`, `smbutil`, and supporting library code.
- Userland PPP, PPPoE daemon, Bluetooth PPP daemon, kernel Netgraph PPP
  components, SLIP, and associated dial-up utilities and integration.
- OFED/InfiniBand/RDMA libraries, tools, kernel integration, OpenSM,
  diagnostic programs, and management components.
- The mlx5 driver source tree, its Ethernet and hardware-offload components,
  kernel modules, and `mlx5tool`, removed by `f779687025e8` ("Updates").
  The mlx4 source tree remains.
- The `inetd` superserver and its configuration/startup integration.
- Telnet client/server sources and supporting library code.
- The legacy remote-command suite and related remote status, user-listing,
  and broadcast tools/services, including `rup`, `ruptime`, `rusers`,
  `rwall`, `rwho`, and `rwhod`.
- The base FTP client (`ftp`/tnftp). This does not describe removal of `ftpd`.
- Finger client/server and talk client/server.
- SNMP daemon, tools, and supporting bsnmp/libbegemot libraries.

### Authentication and directory services

- MIT Kerberos, Heimdal Kerberos, kernel Kerberos support, Kerberos PAM
  integration, and associated GSS/RPCSEC_GSS and error-table components.
- NIS/YP clients, servers, libraries, account/password services, and tools.

### Mail and printing

- Sendmail and its libraries, configuration data, administrative tools,
  mail wrapper, and restricted shell.
- DMA mail transport and base mail programs/services, including `mail`,
  `rmail`, `mail.local`, `biff`/`comsat`, `from`, and `vacation`.
- Legacy LPR printing system and its spooler/administrative integration.

### Console, hardware, and system administration

- The legacy syscons console, its data, and libvgl. The vt console remains;
  its startup service was renamed from `syscons` to `vtcons`.
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

- The tcsh shell (`csh`/`tcsh` in the base system).
- The `ee` editor.
- GNU diff sources. The base BSD diff implementation remains.
- GNU dialog and its `dpv`/libdpv/libfigpar components.
- Games and associated data/utilities, including `fortune`, `caesar`,
  `factor`, `grdc`, `morse`, `number`, `pom`, `primes`, and `random`.
- The `calendar` reminder utility and its data/integration. `cal`/`ncal` remain.
- Installed manual-page readers, lookup/indexing utilities, manual sources,
  manual build rules, and mandoc sources.
- Most locale definitions and aliases. The remaining locale source data
  centers on `C.UTF-8` and `en_US.UTF-8`; timezone data remains separate
  and was updated to 2026d.

Other deleted content includes obsolete GNU scaffolding, upstream CI and
contribution metadata, historical release/update notes, and temporary
removal-planning documents. These contribute to the size reduction but do
not represent separate runtime functionality.

## Measurement method and limits

- Count tracked blob entries and their uncompressed sizes with
  `git ls-tree -r -l` at each fixed snapshot. Include symlink blobs and
  bundled code, documentation, data, tests, and binary assets.
- Compare paths directly for added/deleted files and blob IDs/sizes for
  modified files. Moves count as a deletion and an addition in this method.
- Read blobs with `git cat-file --batch`. For the text-line estimate,
  exclude blobs containing NUL bytes; count newline characters and any
  final unterminated line. This yields 721 binary-classified files in the
  original and 614 in HalfBSD.
- Obtain Git's separate diff statistics with `git diff --shortstat`
  between the two snapshots.
- Sum file sizes by top-level directory to obtain directory reductions.
- Review branch commit history and deleted source/build paths to identify
  removed functionality, including removals hidden under generic commit titles.

These are repository-content measurements, not installed-system size,
executable-code-only measurements, build-time estimates, or a measurement of
security attack-surface reduction. Git history and build artifacts are excluded.
