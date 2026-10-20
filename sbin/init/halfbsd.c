/* SPDX-License-Identifier: BSD-2-Clause
 * PID 1's single supervisor handoff; service reaping remains in libhalfproc.
 */
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "halfbsd.h"
int
hb_boot_mode(const char *override)
{
	if (override && *override) {
		if (!strcmp(override, "rc")) return 0;
		if (!strcmp(override, "lua")) return 1;
		errno = EINVAL; return -1;
	}
	return 1;
}

int
hb_boot_stage(const char *stage)
{
	if (!stage || !*stage || !strcmp(stage, "single")) return 0;
	if (!strcmp(stage, "multiuser")) return 1;
	errno = EINVAL; return -1;
}
void
hb_boot_trace(const char *path, const char *message)
{
	int saved = errno;
	int fd = open(path, O_WRONLY | O_APPEND | O_NOCTTY | O_CLOEXEC | O_NONBLOCK);
	if (fd >= 0) {
		char line[512];
		int length = snprintf(line, sizeof(line), "[HalfBSD init] %s\n", message);
		if (length > 0 && (size_t)length < sizeof(line)) {
			size_t offset = 0;
			while (offset < (size_t)length) {
				ssize_t n = write(fd, line + offset, (size_t)length - offset);
				if (n < 0 && errno == EINTR) continue;
				if (n <= 0) break;
				offset += (size_t)n;
			}
		}
		close(fd);
	}
	errno = saved;
}

int
hb_boot_pipe(int fds[2])
{
	if (pipe2(fds, O_CLOEXEC | O_NONBLOCK) < 0) return -1;
	for (int i = 0; i < 2; i++) {
		if (fds[i] >= 3) continue;
		int copy = fcntl(fds[i], F_DUPFD_CLOEXEC, 3);
		if (copy < 0) { int error = errno; close(fds[0]); close(fds[1]); errno = error; return -1; }
		close(fds[i]); fds[i] = copy;
	}
	return 0;
}
int
hb_boot_child_fd(int fds[2])
{
	close(fds[0]);
	if (fds[1] != 3 && dup2(fds[1], 3) < 0) return -1;
	if (fds[1] != 3) close(fds[1]);
	return fcntl(3, F_SETFD, 0);
}
/* A supervisor must not own the login terminal that getty revokes. */
int
hb_boot_stdio(const char *null_path, const char *console_path)
{
	if (setsid() < 0) return -1;
	int input = open(null_path, O_RDONLY | O_NOCTTY | O_CLOEXEC);
	if (input < 0) return -1;
	int output = open(console_path, O_WRONLY | O_APPEND | O_NOCTTY | O_CLOEXEC);
	if (output < 0) { int error = errno; close(input); errno = error; return -1; }
	int result = dup2(input, STDIN_FILENO);
	if (result >= 0) result = dup2(output, STDOUT_FILENO);
	if (result >= 0) result = dup2(output, STDERR_FILENO);
	int error = errno;
	if (input > STDERR_FILENO) close(input);
	if (output > STDERR_FILENO) close(output);
	/* dup2 to itself preserves CLOEXEC when init originally had closed stdio. */
	if (result >= 0) {
		for (int i = 0; i < 3; i++) {
			if (fcntl(i, F_SETFD, 0) < 0) return -1;
		}
		return 0;
	}
	errno = error; return -1;
}
int
hb_boot_read(int fd, char text[16], size_t *used)
{
	if (*used >= 15) { errno = EPROTO; return -1; }
	ssize_t n = read(fd, text + *used, 15 - *used);
	if (n < 0) return errno == EAGAIN || errno == EINTR ? 0 : -1;
	if (!n) { errno = EPIPE; return -1; }
	*used += (size_t)n; text[*used] = '\0';
	if (*used == sizeof(HB_BOOT_READY) - 1 &&
	    !memcmp(text, HB_BOOT_READY, sizeof(HB_BOOT_READY) - 1)) return 1;
	if (*used == sizeof(HB_BOOT_SINGLE) - 1 &&
	    !memcmp(text, HB_BOOT_SINGLE, sizeof(HB_BOOT_SINGLE) - 1)) return 2;
	if (memchr(text, '\n', *used)) { errno = EPROTO; return -1; }
	return 0;
}
int
hb_boot_wait(pid_t pid, int milliseconds, int *status)
{
	struct timespec begin, now;
	if (pid <= 1 || milliseconds < 0) { errno = EINVAL; return -1; }
	if (clock_gettime(CLOCK_MONOTONIC, &begin) < 0) return -1;
	for (;;) {
		pid_t result = waitpid(pid, status, WNOHANG);
		if (result == pid) return 1;
		if (result < 0 && errno != EINTR) return -1;
		if (clock_gettime(CLOCK_MONOTONIC, &now) < 0) return -1;
		int64_t elapsed = (int64_t)(now.tv_sec - begin.tv_sec) * 1000 +
		    (now.tv_nsec - begin.tv_nsec) / 1000000;
		if (elapsed >= milliseconds) { errno = ETIMEDOUT; return 0; }
		int remaining = milliseconds - (int)elapsed;
		(void)poll(NULL, 0, remaining < 100 ? remaining : 100);
	}
}
int
hb_boot_stop(pid_t pid, int milliseconds, int *status)
{
	if (pid <= 1) { errno = EINVAL; return -1; }
	int result = hb_boot_wait(pid, 0, status);
	if (result != 0) return result;
	/* PID 1 is the only reaper, so an unreaped child PID cannot be reused. */
	if (kill(pid, SIGTERM) < 0 && errno != ESRCH) return -1;
	if (kill(pid, SIGCONT) < 0 && errno != ESRCH) return -1;
	result = hb_boot_wait(pid, milliseconds, status);
	if (result != 0) return result;
	if (kill(pid, SIGKILL) < 0 && errno != ESRCH) return -1;
	return hb_boot_wait(pid, 5000, status);
}
