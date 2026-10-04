/*-
 * Copyright (c) 2010-2022 Hans Petter Selasky
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdint.h>
#include <err.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/sysctl.h>

#include <dev/usb/usb_ioctl.h>

#include "usbtest.h"


static uint8_t usb_ts_select[USB_TS_MAX_LEVELS];

const char *indent[USB_TS_MAX_LEVELS] = {
	" ",
	"   ",
	"     ",
	"       ",
	"         ",
	"           ",
	"             ",
	"               ",
};

/* a perceptual white noise generator (after HPS' invention) */

int32_t
usb_ts_rand_noise(void)
{
	uint32_t temp;
	const uint32_t prime = 0xFFFF1D;
	static uint32_t noise_rem = 1;

	if (noise_rem & 1) {
		noise_rem += prime;
	}
	noise_rem /= 2;

	temp = noise_rem;

	/* unsigned to signed conversion */

	temp ^= 0x800000;
	if (temp & 0x800000) {
		temp |= (-0x800000);
	}
	return temp;
}

uint8_t
usb_ts_show_menu(uint8_t level, const char *title, const char *fmt,...)
{
	va_list args;
	uint8_t x;
	uint8_t retval;
	char *pstr;
	char buf[16];
	char menu[80 * 20];

	va_start(args, fmt);
	vsnprintf(menu, sizeof(menu), fmt, args);
	va_end(args);

	printf("[");

	for (x = 0; x != level; x++) {
		if ((x + 1) == level)
			printf("%d", usb_ts_select[x]);
		else
			printf("%d.", usb_ts_select[x]);
	}

	printf("] - %s:\n\n", title);

	x = 1;
	for (pstr = menu; *pstr; pstr++) {
		if (x != 0) {
			printf("%s", indent[level]);
			x = 0;
		}
		printf("%c", *pstr);

		if (*pstr == '\n')
			x = 1;
	}

	printf("\n>");

	if (fgets(buf, sizeof(buf), stdin) == NULL)
		err(1, "Cannot read input");

	if (buf[0] == 'x')
		retval = 255;
	else
		retval = atoi(buf);

	usb_ts_select[level] = retval;

	return (retval);
}

void
get_string(char *ptr, int size)
{
	printf("\nEnter string>");

	if (fgets(ptr, size, stdin) == NULL)
		err(1, "Cannot read input");

	ptr[size - 1] = 0;

	size = strlen(ptr);

	/* strip trailing newline, if any */
	if (size == 0)
		return;
	else if (ptr[size - 1] == '\n')
		ptr[size - 1] = 0;
}

int
get_integer(void)
{
	char buf[32];

	printf("\nEnter integer value>");

	if (fgets(buf, sizeof(buf), stdin) == NULL)
		err(1, "Cannot read input");

	if (strcmp(buf, "x\n") == 0)
		return (-1);
	if (strcmp(buf, "r\n") == 0)
		return (-2);

	return ((int)strtol(buf, 0, 0));
}

static void
show_host_select(uint8_t level)
{
	int force_fs = 0;
	int error;
	uint32_t duration = 60;

	struct uaddr uaddr = {};

	uint8_t retval;

	while (1) {

		error = sysctlbyname("hw.usb.ehci.no_hs", NULL, NULL,
		    &force_fs, sizeof(force_fs));

		if (error != 0) {
			printf("WARNING: Could not set non-FS mode "
			    "to %d (error=%d)\n", force_fs, errno);
		}
		retval = usb_ts_show_menu(level, "Select Host Mode Test (via LibUSB)",
		    " 1) Select USB device (VID=0x%04x, PID=0x%04x, ugen%u.%u)\n"
		    " 2) Manually enter USB vendor and product ID\n"
		    " 3) Force FULL speed operation: <%s>\n"
		    " 4) Mass Storage (UMASS)\n"
		    " 5) Modem (UMODEM)\n"
		    "10) Start String Descriptor Test\n"
		    "11) Start Port Reset Test\n"
		    "12) Start Set Config Test\n"
		    "13) Start Get Descriptor Test\n"
		    "14) Start Suspend and Resume Test\n"
		    "15) Start Set and Clear Endpoint Stall Test\n"
		    "16) Start Set Alternate Interface Setting Test\n"
		    "17) Start Invalid Control Request Test\n"
		    "30) Duration: <%d> seconds\n"
		    "x) Return to previous menu\n",
		    uaddr.vid, uaddr.pid, uaddr.bus, uaddr.addr,
		    force_fs ? "YES" : "NO",
		    (int)duration);

		switch (retval) {
		case 0:
			break;
		case 1:
			show_host_device_selection(level + 1, &uaddr);
			break;
		case 2:
			/* only match VID and PID */
			uaddr.vid = get_integer() & 0xFFFF;
			uaddr.pid = get_integer() & 0xFFFF;
			uaddr.bus = 0;
			uaddr.addr = 0;
			break;
		case 3:
			force_fs ^= 1;
			break;
		case 4:
			show_host_msc_test(level + 1, uaddr, duration);
			break;
		case 5:
			show_host_modem_test(level + 1, uaddr, duration);
			break;
		case 10:
			usb_get_string_desc_test(uaddr);
			break;
		case 11:
			usb_port_reset_test(uaddr, duration);
			break;
		case 12:
			usb_set_config_test(uaddr, duration);
			break;
		case 13:
			usb_get_descriptor_test(uaddr, duration);
			break;
		case 14:
			usb_suspend_resume_test(uaddr, duration);
			break;
		case 15:
			usb_set_and_clear_stall_test(uaddr);
			break;
		case 16:
			usb_set_alt_interface_test(uaddr);
			break;
		case 17:
			usb_control_ep_error_test(uaddr);
			break;
		case 30:
			duration = get_integer();
			break;
		default:
			return;
		}
	}
}

int
main(int argc, char **argv)
{
	show_host_select(1);

	return (0);
}
