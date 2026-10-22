/* SPDX-License-Identifier: BSD-2-Clause */
#include <assert.h>
#include <stdio.h>
#include "halfbsd_actions.h"
int
main(void)
{
    assert(hb_device_action("/usr/sbin/halfinit --device-nomatch $*") == HB_DEVICE_MATCH);
    assert(hb_device_action("/etc/netif_hotplug $subsystem start") == HB_DEVICE_DHCP);
    assert(hb_device_action("/etc/rc.suspend acpi $notify") == HB_DEVICE_POWER);
    assert(hb_device_action("service devmatch quietstart $*") == HB_DEVICE_MATCH);
    assert(hb_device_action("service dhclient quietstart $subsystem") == HB_DEVICE_DHCP);
    assert(hb_device_action("service vtcons setkeyboard /dev/ukbd0") == HB_DEVICE_KEYBOARD);
    assert(hb_device_action("service vtcons setkeyboard /dev/kbd0") == HB_DEVICE_KEYBOARD);
    assert(hb_device_action("service power_profile $notify") == HB_DEVICE_POWER);
    assert(hb_device_action("service bluetooth quietstop $device-name") == HB_DEVICE_BLUETOOTH);
    assert(hb_device_action("service devmatch quietstart $*; echo unsafe") == HB_DEVICE_OTHER);
    assert(hb_device_action("service sshd restart") == HB_DEVICE_OTHER);
    assert(hb_device_peer_closed(EPIPE));
    assert(hb_device_peer_closed(ECONNRESET));
    assert(hb_device_peer_closed(ENOTCONN));
    assert(!hb_device_peer_closed(EAGAIN));
    assert(!hb_device_peer_closed(ENOBUFS));
    assert(!hb_device_peer_closed(0));
    puts("PASS exact devd action classification; unrelated and modified actions retain configured behavior");
    return 0;
}
