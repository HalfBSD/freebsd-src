-- SPDX-License-Identifier: BSD-2-Clause
-- Planning is safe: halfinit --config FILE --dry-run
-- Installed as /etc/halfbsd/system.lua only when no administrator file exists.
-- PID 1 selects Lua by default; halfbsd.boot_mode=rc selects recovery.
-- Never activate this system graph alongside an already running rc system.
return {
    target='multiuser',
    max_parallel=1, -- Deterministic initial boot; uses the existing scheduler limit.
    log_file='/var/run/halfbsd/boot.log',
    boot={mode='system',interfaces='auto',local_mounts='fstab',zfs=true,
        root_writable=true,swap='fstab',hostname='halfbsd',
        -- Fresh Lua boot installs rules over the kernel's empty deny baseline.
        -- Existing administrator policy must use ownership='verify'.
        firewall={ownership='load',mode='workstation',ipv6=true},
        dns=true,sshd=true,ntpd=true,syslogd=true,seatd=false},
}
