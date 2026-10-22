/* SPDX-License-Identifier: BSD-2-Clause */
#ifndef HALFBSD_ACTIONS_H
#define HALFBSD_ACTIONS_H
#include <string.h>
#include <errno.h>
enum hb_device_action { HB_DEVICE_OTHER, HB_DEVICE_MATCH, HB_DEVICE_DHCP,
    HB_DEVICE_KEYBOARD, HB_DEVICE_POWER, HB_DEVICE_BLUETOOTH };
static inline enum hb_device_action
hb_device_action(const char *command)
{
    if (!strcmp(command, "service devmatch quietstart $*")) return HB_DEVICE_MATCH;
    if (!strcmp(command, "service dhclient quietstart $subsystem")) return HB_DEVICE_DHCP;
    if (!strcmp(command, "service vtcons setkeyboard /dev/ukbd0") ||
        !strcmp(command, "service vtcons setkeyboard /dev/kbd0")) return HB_DEVICE_KEYBOARD;
    if (!strcmp(command, "service power_profile $notify")) return HB_DEVICE_POWER;
    if (!strcmp(command, "service bluetooth quietstart $device-name") ||
        !strcmp(command, "service bluetooth quietstop $device-name")) return HB_DEVICE_BLUETOOTH;
    return HB_DEVICE_OTHER;
}
static inline int
hb_device_peer_closed(int error)
{
    return error == EPIPE || error == ECONNRESET || error == ENOTCONN;
}
#endif
