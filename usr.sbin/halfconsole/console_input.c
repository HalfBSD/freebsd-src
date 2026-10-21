/* SPDX-License-Identifier: BSD-2-Clause */
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <poll.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include "console_input.h"

static double
monotonic(void)
{
	struct timespec now;
	if (clock_gettime(CLOCK_MONOTONIC, &now) == -1)
		return -1;
	return (double)now.tv_sec + (double)now.tv_nsec / 1000000000.0;
}

static int
select_line(int descriptor, double seconds, char *input, size_t capacity)
{
	struct pollfd fd = { descriptor, POLLIN, 0 };
	if (!isfinite(seconds) || seconds < 0 || seconds > 30 || capacity < 2) {
		errno = EINVAL;
		return -1;
	}
	if (seconds == 0)
		return 0;
	double now = monotonic();
	if (now < 0)
		return -1;
	double deadline = now + seconds;
	for (;;) {
		now = monotonic();
		if (now < 0)
			return -1;
		if (now >= deadline) {
			/* Discard an incomplete canonical line before entering login. */
			return tcflush(descriptor, TCIFLUSH) == -1 ? -1 : 0;
		}
		int result = poll(&fd, 1, (int)((deadline - now) * 1000) + 1);
		if (result == -1) {
			if (errno == EINTR)
				continue;
			return -1;
		}
		if (result == 0)
			continue;
		if (fd.revents & (POLLHUP | POLLERR | POLLNVAL)) {
			errno = EIO;
			return -1;
		}
		if (!(fd.revents & POLLIN))
			continue;
		ssize_t count = read(descriptor, input, capacity - 1);
		if (count == -1 && (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK))
			continue;
		if (count <= 0) {
			if (count == 0)
				errno = EIO;
			return -1;
		}
		if (memchr(input, '\0', (size_t)count) != NULL ||
		    memchr(input, '\n', (size_t)count) == NULL) {
			errno = EOVERFLOW;
			return -1;
		}
		input[count] = '\0';
		return 1;
	}
}

int
hb_console_choose(int descriptor, double seconds, char *input, size_t capacity)
{
	int flags = fcntl(descriptor, F_GETFL);
	if (flags == -1 || fcntl(descriptor, F_SETFL, flags | O_NONBLOCK) == -1)
		return -1;
	int result = select_line(descriptor, seconds, input, capacity);
	int saved = errno;
	if (fcntl(descriptor, F_SETFL, flags) == -1)
		return -1;
	errno = saved;
	return result;
}
