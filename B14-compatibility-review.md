# B14 pre-removal compatibility review

Native FreeBSD 11 is the floor. Retained FreeBSD32/IA32 and Linux interfaces
are independent of that floor. This is a source/dependency review; current
package binaries and runtime/build validation are unavailable on this Linux
host. No package dependency on native pre-11 support has been demonstrated.
Current libc/libsys sources and symbol maps will remain unchanged.

| Tier | Generation | Kernel/syscall coverage | Library ABI and shared consumers | Exclusively pre-11? | Disposition |
|---|---|---|---|---|---|
| `COMPAT_43` | 4.3BSD | 4.3BSD signal/socket/stat/process legacy calls | FreeBSD32 dispatch/layouts share older kernel wrappers; exported historical libc/libsys versions are preserved | No: shared 32-bit consumers remain | Retire native option/dispatch; preserve required 32-bit implementations |
| `COMPAT_FREEBSD4` | FreeBSD 4 | statfs, signals, sendfile, SysV IPC, uname/domain and PCI interfaces | FreeBSD32 dispatch/layouts share older kernel wrappers; exported historical libc/libsys versions are preserved | No: shared 32-bit consumers remain | Retire native option/dispatch; preserve required 32-bit implementations |
| `COMPAT_FREEBSD5` | FreeBSD 5 | legacy PCI ioctl selection | FreeBSD32 dispatch/layouts share older kernel wrappers; exported historical libc/libsys versions are preserved | No: shared 32-bit consumers remain | Retire native option/dispatch; preserve required 32-bit implementations |
| `COMPAT_FREEBSD6` | FreeBSD 6 | pread/pwrite, mmap/lseek/truncate, AIO and PCI interfaces | FreeBSD32 dispatch/layouts share older kernel wrappers; exported historical libc/libsys versions are preserved | No: shared 32-bit consumers remain | Retire native option/dispatch; preserve required 32-bit implementations |
| `COMPAT_FREEBSD7` | FreeBSD 7 | SysV IPC layouts, kinfo_proc and file-descriptor sysctls | FreeBSD32 dispatch/layouts share older kernel wrappers; exported historical libc/libsys versions are preserved | No: shared 32-bit consumers remain | Retire native option/dispatch; preserve required 32-bit implementations |
| `COMPAT_FREEBSD8` | FreeBSD 8 | no remaining configurable native tier (feature check only) | FreeBSD32 dispatch/layouts share older kernel wrappers; exported historical libc/libsys versions are preserved | No: shared 32-bit consumers remain | Retire native option/dispatch; preserve required 32-bit implementations |
| `COMPAT_FREEBSD9` | FreeBSD 9 | legacy umtx semaphore operations | FreeBSD32 dispatch/layouts share older kernel wrappers; exported historical libc/libsys versions are preserved | No: shared 32-bit consumers remain | Retire native option/dispatch; preserve required 32-bit implementations |
| `COMPAT_FREEBSD10` | FreeBSD 10 | pipe return convention and umtx locks/semaphores | FreeBSD32 dispatch/layouts share older kernel wrappers; exported historical libc/libsys versions are preserved | No: shared 32-bit consumers remain | Retire native option/dispatch; preserve required 32-bit implementations |
| `COMPAT_FREEBSD11` | FreeBSD 11 | stat/dirent/mknod, kevent, jail and SysV IPC layouts | Current ABI/versioned library symbols and retained kernel helpers are protected | No: protected tier | Keep unchanged |
| `COMPAT_FREEBSD12` | FreeBSD 12 | shm_open and closefrom | Current ABI/versioned library symbols and retained kernel helpers are protected | No: protected tier | Keep unchanged |
| `COMPAT_FREEBSD13` | FreeBSD 13 | swapoff | Current ABI/versioned library symbols and retained kernel helpers are protected | No: protected tier | Keep unchanged |
| `COMPAT_FREEBSD14` | FreeBSD 14 | getrlimit/setrlimit and scheduler cpuset arguments | Current ABI/versioned library symbols and retained kernel helpers are protected | No: protected tier | Keep unchanged |

There is no COMPAT_FREEBSD15 tier in this snapshot; current FreeBSD 15 system
calls are the default ABI. Any future compatibility tier must stay available.
COMPAT_43TTY is retained: old tty ioctl encodings remain part of contemporary
terminal/keyboard consumers and are independent of native syscall tiers.

## Exact source coverage per tier

**COMPAT_43**

`sys/vm/vm_mmap.c`, `sys/sys/ioccom.h`, `sys/sys/_sigset.h`, `sys/sys/ipc.h`, `sys/sys/sysproto.h`, `sys/sys/signalvar.h`, `sys/sys/signal.h`, `sys/kern/uipc_syscalls.c`, `sys/kern/kern_resource.c`, `sys/kern/sysv_shm.c`, `sys/kern/sys_generic.c`, `sys/kern/kern_prot.c`, `sys/kern/vfs_syscalls.c`, `sys/kern/kern_exit.c`, `sys/kern/kern_sig.c`, `sys/kern/kern_descrip.c`, `sys/kern/init_sysent.c`, `sys/kern/kern_xxx.c`, `sys/dev/atkbdc/atkbd.c`, `sys/dev/hid/hkbd.c`, `sys/dev/vt/vt_core.c`, `sys/dev/gpio/gpiokeys.c`, `sys/dev/usb/input/ukbd.c`, `sys/arm64/arm64/freebsd32_machdep.c`, `sys/arm64/arm64/exec_machdep.c`, `sys/arm64/linux/linux_sysvec.c`, `sys/compat/freebsd32/freebsd32_misc.c`, `sys/compat/freebsd32/freebsd32_sysent.c`, `sys/compat/freebsd32/freebsd32_proto.h`, `sys/compat/ia32/ia32_genassym.c`, `sys/amd64/ia32/ia32_syscall.c`, `sys/amd64/ia32/ia32_sigtramp.S`, `sys/amd64/ia32/ia32_signal.c`, `sys/amd64/amd64/exec_machdep.c`, `sys/arm/arm/exec_machdep.c`, `sys/i386/i386/machdep.c`, `sys/i386/i386/sigtramp.S`, `sys/i386/i386/genassym.c`, `sys/i386/i386/trap.c`, `sys/i386/i386/exec_machdep.c`, `sys/i386/include/sigframe.h`, `sys/i386/include/md_var.h`, `sys/i386/include/signal.h`

**COMPAT_FREEBSD4**

`sys/net/if_tuntap.c`, `sys/sys/ioccom.h`, `sys/sys/ipc.h`, `sys/sys/sysproto.h`, `sys/sys/msg.h`, `sys/sys/fcntl.h`, `sys/sys/shm.h`, `sys/sys/sem.h`, `sys/kern/vfs_bio.c`, `sys/kern/sysv_shm.c`, `sys/kern/sys_generic.c`, `sys/kern/sysv_msg.c`, `sys/kern/kern_mib.c`, `sys/kern/kern_sendfile.c`, `sys/kern/vfs_syscalls.c`, `sys/kern/kern_sig.c`, `sys/kern/init_sysent.c`, `sys/kern/vfs_vnops.c`, `sys/kern/kern_xxx.c`, `sys/kern/sysv_ipc.c`, `sys/kern/sysv_sem.c`, `sys/dev/pci/pci_user.c`, `sys/dev/atkbdc/atkbd.c`, `sys/dev/hid/hkbd.c`, `sys/dev/vt/vt_core.c`, `sys/dev/gpio/gpiokeys.c`, `sys/dev/usb/input/ukbd.c`, `sys/compat/freebsd32/freebsd32_ipc.h`, `sys/compat/freebsd32/freebsd32_misc.c`, `sys/compat/freebsd32/freebsd32_sysent.c`, `sys/compat/freebsd32/freebsd32_proto.h`, `sys/compat/ia32/ia32_genassym.c`, `sys/compat/ia32/ia32_sysvec.c`, `sys/amd64/ia32/ia32_sigtramp.S`, `sys/amd64/ia32/ia32_signal.c`, `sys/amd64/amd64/exec_machdep.c`, `sys/arm/arm/machdep.c`, `sys/i386/i386/sigtramp.S`, `sys/i386/i386/genassym.c`, `sys/i386/i386/exec_machdep.c`, `sys/i386/include/ucontext.h`, `sys/i386/include/sigframe.h`, `sys/i386/include/md_var.h`

**COMPAT_FREEBSD5**

`sys/net/if_tuntap.c`, `sys/sys/ioccom.h`, `sys/sys/ipc.h`, `sys/sys/msg.h`, `sys/sys/fcntl.h`, `sys/sys/shm.h`, `sys/sys/sem.h`, `sys/kern/vfs_bio.c`, `sys/kern/sysv_shm.c`, `sys/kern/sys_generic.c`, `sys/kern/sysv_msg.c`, `sys/kern/kern_mib.c`, `sys/kern/vfs_vnops.c`, `sys/kern/sysv_ipc.c`, `sys/kern/sysv_sem.c`, `sys/dev/pci/pci_user.c`, `sys/dev/atkbdc/atkbd.c`, `sys/dev/hid/hkbd.c`, `sys/dev/vt/vt_core.c`, `sys/dev/gpio/gpiokeys.c`, `sys/dev/usb/input/ukbd.c`, `sys/compat/freebsd32/freebsd32_ipc.h`, `sys/arm/arm/machdep.c`

**COMPAT_FREEBSD6**

`sys/vm/vm_mmap.c`, `sys/net/if_tuntap.c`, `sys/sys/ioccom.h`, `sys/sys/ipc.h`, `sys/sys/sysproto.h`, `sys/sys/msg.h`, `sys/sys/fcntl.h`, `sys/sys/shm.h`, `sys/sys/sem.h`, `sys/sys/signalvar.h`, `sys/kern/vfs_bio.c`, `sys/kern/vfs_aio.c`, `sys/kern/sysv_shm.c`, `sys/kern/sys_generic.c`, `sys/kern/sysv_msg.c`, `sys/kern/kern_mib.c`, `sys/kern/vfs_syscalls.c`, `sys/kern/init_sysent.c`, `sys/kern/vfs_vnops.c`, `sys/kern/sysv_ipc.c`, `sys/kern/sysv_sem.c`, `sys/dev/pci/pci_user.c`, `sys/dev/atkbdc/atkbd.c`, `sys/dev/hid/hkbd.c`, `sys/dev/vt/vt_core.c`, `sys/dev/kbdmux/kbdmux.c`, `sys/dev/gpio/gpiokeys.c`, `sys/dev/vkbd/vkbd.c`, `sys/dev/usb/input/ukbd.c`, `sys/compat/freebsd32/freebsd32_ipc.h`, `sys/compat/freebsd32/freebsd32_misc.c`, `sys/compat/freebsd32/freebsd32_sysent.c`, `sys/compat/freebsd32/freebsd32_proto.h`, `sys/arm/arm/machdep.c`

**COMPAT_FREEBSD7**

`sys/sys/ipc.h`, `sys/sys/sysproto.h`, `sys/sys/msg.h`, `sys/sys/fcntl.h`, `sys/sys/shm.h`, `sys/sys/sem.h`, `sys/kern/kern_proc.c`, `sys/kern/vfs_bio.c`, `sys/kern/sysv_shm.c`, `sys/kern/sysv_msg.c`, `sys/kern/kern_mib.c`, `sys/kern/kern_descrip.c`, `sys/kern/init_sysent.c`, `sys/kern/vfs_vnops.c`, `sys/kern/sysv_ipc.c`, `sys/kern/sysv_sem.c`, `sys/compat/freebsd32/freebsd32_ipc.h`, `sys/compat/freebsd32/freebsd32_sysent.c`, `sys/compat/freebsd32/freebsd32_proto.h`, `sys/arm/arm/machdep.c`

**COMPAT_FREEBSD8**

`sys/kern/kern_mib.c`

**COMPAT_FREEBSD9**

`sys/kern/kern_umtx.c`, `sys/kern/kern_mib.c`, `sys/arm/arm/machdep.c`

**COMPAT_FREEBSD10**

`sys/sys/sysproto.h`, `sys/kern/kern_umtx.c`, `sys/kern/kern_mib.c`, `sys/kern/sys_pipe.c`, `sys/kern/init_sysent.c`, `sys/compat/freebsd32/freebsd32_sysent.c`, `sys/compat/freebsd32/freebsd32_proto.h`

**COMPAT_FREEBSD11**

`sys/vm/vm_meter.c`, `sys/vm/swap_pager.c`, `sys/vm/vm_unix.c`, `sys/sys/sysproto.h`, `sys/kern/kern_mib.c`, `sys/kern/vfs_syscalls.c`, `sys/kern/kern_descrip.c`, `sys/kern/init_sysent.c`, `sys/kern/kern_event.c`, `sys/contrib/openzfs/include/sys/arc_impl.h`, `sys/contrib/openzfs/module/zfs/arc.c`, `sys/fs/devfs/devfs_devs.c`, `sys/compat/freebsd32/freebsd32_misc.c`, `sys/compat/freebsd32/freebsd32_sysent.c`, `sys/compat/freebsd32/freebsd32_proto.h`

**COMPAT_FREEBSD12**

`sys/opencrypto/crypto.c`, `sys/opencrypto/cryptodev.c`, `sys/sys/sysproto.h`, `sys/kern/kern_mib.c`, `sys/kern/uipc_ktls.c`, `sys/kern/kern_descrip.c`, `sys/kern/init_sysent.c`, `sys/kern/uipc_shm.c`, `sys/dev/vmm/vmm_dev.c`, `sys/compat/freebsd32/freebsd32_sysent.c`, `sys/compat/freebsd32/freebsd32_proto.h`

**COMPAT_FREEBSD13**

`sys/vm/swap_pager.c`, `sys/sys/sysproto.h`, `sys/sys/kbio.h`, `sys/kern/kern_mib.c`, `sys/kern/init_sysent.c`, `sys/dev/atkbdc/atkbd.c`, `sys/dev/hid/hkbd.c`, `sys/dev/vt/vt_core.c`, `sys/dev/adb/adb_kbd.c`, `sys/dev/kbd/kbd.c`, `sys/dev/kbdmux/kbdmux.c`, `sys/dev/gpio/gpiokeys.c`, `sys/dev/vkbd/vkbd.c`, `sys/dev/usb/input/ukbd.c`, `sys/arm64/arm64/exec_machdep.c`, `sys/compat/freebsd32/freebsd32_sysent.c`, `sys/compat/freebsd32/freebsd32_proto.h`, `sys/amd64/vmm/vmm_dev_machdep.c`

**COMPAT_FREEBSD14**

`sys/sys/sysproto.h`, `sys/kern/kern_mib.c`, `sys/kern/kern_prot.c`, `sys/kern/init_sysent.c`, `sys/dev/pci/pci_user.c`, `sys/dev/watchdog/watchdog.c`, `sys/dev/vmm/vmm_dev.c`, `sys/arm64/arm64/elf_machdep.c`, `sys/arm64/arm64/ptrauth.c`, `sys/arm64/include/cpu.h`, `sys/compat/freebsd32/freebsd32_sysent.c`, `sys/compat/freebsd32/freebsd32_proto.h`

## Shared boundary and implementation plan

- Keep the common syscall master as the source for both ABIs. Native generation
  reserves compatibility entries below 11; FreeBSD32 generation retains its
  complete dispatch and argument layouts. No syscall number changes.
- Replace retired option guards on required 32-bit code with COMPAT_FREEBSD32;
  remove native-only wrappers whose remaining users are solely retired native
  dispatch. Retain shared structs/prototypes needed by the 32-bit table.
- Preserve COMPAT_FREEBSD11/12/13/14 and all current library exports. No libc or
  libsys symbol may be removed solely on the basis of its version label.
- Restrict legacy umtx operations to the 32-bit dispatch, keeping contemporary
  native operations unchanged.
- Remove sys/kern/imgact_aout.c, sys/modules/aout, COMPAT_AOUT and their native
  configuration/build references. Keep sys/sys/imgact_aout.h, link_aout.h,
  nlist_aout.h and generic object-format readers: Linux i386, libpmcstat,
  profiling, libkvm/nlist/toolchain and cross-development consume them.
- execve's so-called good_aout is an ordinary C executable built as ELF; its
  execution test remains. Truncated/sparse-file negative tests remain too.

Native world/kernel/module, staged-install, package/Firefox and hardware/runtime
checks must happen on FreeBSD before this removal is committed.

## Native-only wrappers removed

`orecvmsg`, `osendmsg`, `freebsd10__umtx_unlock`, `freebsd10__umtx_lock`, `freebsd6_lio_listio`, `freebsd6_aio_write`, `freebsd6_aio_read`, `oftruncate`, `freebsd6_pwrite`, `freebsd6_pread`, `freebsd4_sendfile`, `ogetdirentries`, `freebsd6_ftruncate`, `freebsd6_truncate`, `otruncate`, `olstat`, `ostat`, `freebsd6_lseek`, `olseek`, `freebsd4_fhstatfs`, `freebsd4_getfsstat`, `freebsd4_fstatfs`, `freebsd4_statfs`, `osigstack`, `osigvec`, `osigreturn`, `osigaction`, `freebsd4_sigaction`, `ofstat`, `ogetkerninfo`, `osethostid`, `ommap`, `freebsd6_mmap`

## Shared native-layout wrappers retained for FreeBSD32

`ocreat`, `freebsd10_pipe`, `osigprocmask`, `osigpending`, `ogetpagesize`, `owait`, `ogethostname`, `osethostname`, `oaccept`, `osend`, `orecv`, `osigblock`, `osigsetmask`, `osigsuspend`, `orecvfrom`, `ogetpeername`, `ogethostid`, `ogetrlimit`, `osetrlimit`, `okillpg`, `oquota`, `ogetsockname`, `freebsd4_getdomainname`, `freebsd4_setdomainname`, `freebsd4_uname`

The dependency follow-up found that native-layout FreeBSD-7 SysV control
wrappers are also called by retained multiplexor helpers. Their implementations
and argument layouts remain under COMPAT_FREEBSD32. Their *native compatibility
slot registrations* are removed so loading sysvipc cannot resurrect retired
native slots 220, 224 or 229. Published standard SysV interfaces remain.

## Final source boundary

Native i386 signal/trampoline paths exclusive to retired tiers are also removed;
the i386 layout headers imported by AMD64 IA32 remain. Historical 4.3BSD
FreeBSD32 support has a separate COMPAT_FREEBSD32_43 option, used together with
COMPAT_FREEBSD32, so the existing default-disabled selection is preserved.
COMPAT_43TTY remains independently available. Native generated argument tables
and dispatch use the floor of 11; published library headers, symbol maps and
syscall constants are generated from the complete master and remain unchanged.
