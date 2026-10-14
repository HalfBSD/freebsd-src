-- SPDX-License-Identifier: BSD-2-Clause
-- Planning is safe: halfinit --config FILE --dry-run
-- Live activation is for an isolated boot test machine, NOT a running rc host.
-- Copy and review under /etc/halfbsd; installworld does not enable this file.
return {
    target='multiuser',
    log_file='/var/run/halfbsd/boot.log',
    boot={mode='system',interfaces='auto',local_mounts='fstab',zfs=true,
        root_writable=true,swap='fstab',
        -- Fresh Lua boot installs rules over the kernel's empty deny baseline.
        -- Existing administrator policy must use ownership='verify'.
        firewall={ownership='load',mode='workstation',ipv6=true},
        dns=true,sshd=true,ntpd=true,seatd=false},
}
