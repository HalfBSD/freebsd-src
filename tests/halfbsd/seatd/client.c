/* SPDX-License-Identifier: BSD-2-Clause
 * Private, VT-unbound fixture only. Never opens graphics/input devices.
 */
#include <assert.h>
#include <errno.h>
#include <poll.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <libseat.h>

static void
enabled(struct libseat *seat, void *data)
{
	(void)seat;
	*(bool *)data = true;
}

static void
disabled(struct libseat *seat, void *data)
{
	*(bool *)data = false;
	assert(libseat_disable_seat(seat) == 0);
}

int
main(void)
{
	alarm(15);
	const struct libseat_seat_listener listener = { enabled, disabled };
	for (int iteration = 0; iteration < 20; iteration++) {
		bool active = false;
		struct libseat *seat = libseat_open_seat(&listener, &active);
		assert(seat != NULL);
		assert(strcmp(libseat_seat_name(seat), "seat0") == 0);
		assert(libseat_get_fd(seat) >= 0);
		for (int tries = 0; !active && tries < 5; tries++)
			assert(libseat_dispatch(seat, 1000) >= 0);
		assert(active);
		/* A regular file is not a permitted seat device. */
		int fd = -1;
		assert(libseat_open_device(seat, "/etc/passwd", &fd) == -1);
		assert(fd == -1);
		assert(libseat_close_seat(seat) == 0);
	}
	puts("PASS private libseat protocol, callbacks, denied non-device path and repeated cleanup");
	return (0);
}
