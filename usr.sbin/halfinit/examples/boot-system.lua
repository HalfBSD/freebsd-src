-- SPDX-License-Identifier: BSD-2-Clause
-- Planning is safe: halfinit --config FILE --dry-run
-- Live activation is for an isolated boot test machine, NOT a running rc host.
-- Copy and review under /etc/halfbsd; installworld does not enable this file.
return {
    target='multiuser',
    boot={mode='system',interfaces='auto',local_mounts='fstab',zfs=true,
        root_writable=false,
        -- Verify an existing immutable deny guard and enabled kernel hooks.
        -- 'load' installs workstation rules only over an empty deny baseline.
        firewall={ownership='verify',mode='workstation',ipv6=true},
        dns=true,sshd=true,ntpd=true,seatd=false},
}
