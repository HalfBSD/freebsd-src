/* SPDX-License-Identifier: BSD-2-Clause
 * Harmless private children only: never run PID 1 or signal host services.
 */
#include <sys/wait.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "halfbsd.h"

static void
channel(void)
{
	int fds[2]; char text[16] = {0}; size_t used = 0;
	assert(hb_boot_pipe(fds) == 0);
	assert(fds[0] >= 3 && fds[1] >= 3);
	assert(hb_boot_read(fds[0], text, &used) == 0);
	assert(write(fds[1], "REA", 3) == 3);
	assert(hb_boot_read(fds[0], text, &used) == 0);
	assert(write(fds[1], "DY\n", 3) == 3);
	assert(hb_boot_read(fds[0], text, &used) == 1);
	close(fds[0]); close(fds[1]);
	assert(hb_boot_pipe(fds) == 0); used = 0;
	assert(write(fds[1], "SIN", 3) == 3);
	assert(hb_boot_read(fds[0], text, &used) == 0);
	assert(write(fds[1], "GLE\n", 4) == 4);
	assert(hb_boot_read(fds[0], text, &used) == 2);
	close(fds[0]); close(fds[1]);
	assert(hb_boot_pipe(fds) == 0); used = 0;
	assert(write(fds[1], "NET", 3) == 3);
	assert(hb_boot_read(fds[0], text, &used) == 0);
	assert(write(fds[1], "WORK\n", 5) == 5);
	assert(hb_boot_read(fds[0], text, &used) == 3);
	close(fds[0]); close(fds[1]);
	const char *invalid[] = {"FAIL\n", "READY\nX", "REA\0Y\n", "SINGLE\nX", "SIN\0LE\n"};
	const size_t lengths[] = {5, 7, 6, 8, 7};
	for (size_t i = 0; i < sizeof(lengths) / sizeof(lengths[0]); i++) {
		assert(hb_boot_pipe(fds) == 0); used = 0;
		assert(write(fds[1], invalid[i], lengths[i]) == (ssize_t)lengths[i]);
		assert(hb_boot_read(fds[0], text, &used) == -1 && errno == EPROTO);
		close(fds[0]); close(fds[1]);
	}
	assert(hb_boot_pipe(fds) == 0); used = 0; close(fds[1]);
	assert(hb_boot_read(fds[0], text, &used) == -1 && errno == EPIPE);
	close(fds[0]);
	puts("PASS fragmented readiness, malformed messages and EOF");
}

static void
trace(void)
{
	char path[] = "/tmp/halfbsd-trace.XXXXXX", text[128] = {0};
	int fd = mkstemp(path); assert(fd >= 0);
	errno = ERANGE; hb_boot_trace(path, "visible progress"); assert(errno == ERANGE);
	ssize_t n = read(fd, text, sizeof(text) - 1); assert(n > 0);
	assert(!strcmp(text, "[HalfBSD init] visible progress\n"));
	close(fd); assert(unlink(path) == 0);
	hb_boot_trace(path, "absent destination"); assert(errno == ERANGE);
	puts("PASS immediate progress trace and preserved errno on unavailable console");
}

static void
children(void)
{
	int fds[2], status; char text[16] = {0}; size_t used = 0;
	assert(hb_boot_pipe(fds) == 0);
	pid_t child = fork(); assert(child >= 0);
	if (child == 0) {
		assert(hb_boot_child_fd(fds) == 0);
		assert(!(fcntl(3, F_GETFD) & FD_CLOEXEC));
		assert(write(3, HB_BOOT_READY, 6) == 6); _exit(0);
	}
	close(fds[1]);
	assert(hb_boot_wait(child, 2000, &status) == 1 && WIFEXITED(status));
	assert(hb_boot_read(fds[0], text, &used) == 1); close(fds[0]);
	assert(hb_boot_stop(child, 10, &status) == -1 && errno == ECHILD);
	assert(hb_boot_stop(1, 10, &status) == -1 && errno == EINVAL);
	assert(hb_boot_pipe(fds) == 0); child = fork(); assert(child >= 0);
	if (child == 0) {
		close(fds[0]); assert(signal(SIGTERM, SIG_IGN) != SIG_ERR);
		assert(write(fds[1], "X", 1) == 1); for (;;) pause();
	}
	close(fds[1]);
	for (;;) {
		char byte; ssize_t n = read(fds[0], &byte, 1);
		if (n == 1) break;
		assert(n < 0 && (errno == EAGAIN || errno == EINTR));
		usleep(1000);
	}
	close(fds[0]);
	assert(hb_boot_wait(child, 0, &status) == 0 && errno == ETIMEDOUT);
	assert(hb_boot_stop(child, 10, &status) == 1);
	assert(WIFSIGNALED(status) && WTERMSIG(status) == SIGKILL);
	child = fork(); assert(child >= 0);
	if (child == 0) {
		close(0); close(1); close(2);
		assert(hb_boot_pipe(fds) == 0 && fds[0] >= 3 && fds[1] >= 3);
		assert(hb_boot_child_fd(fds) == 0);
		assert(hb_boot_stdio("/dev/null", "/dev/null") == 0);
		assert(getsid(0) == getpid());
		for (int i = 0; i < 4; i++) assert(!(fcntl(i, F_GETFD) & FD_CLOEXEC));
		assert(write(1, "X", 1) == 1); _exit(0);
	}
	assert(hb_boot_wait(child, 2000, &status) == 1);
	assert(WIFEXITED(status) && WEXITSTATUS(status) == 0);
	puts("PASS inherited descriptor, closed stdio/session setup, exact child reaping and bounded forced shutdown");
}

int
main(void)
{
	assert(hb_boot_mode(NULL) == 1);
	assert(hb_boot_mode("") == 1);
	assert(hb_boot_mode("lua") == 1);
	assert(hb_boot_mode("rc") == 0);
	assert(hb_boot_mode("unknown") == -1 && errno == EINVAL);
	puts("PASS default Lua, explicit RC recovery and invalid override rejection");
	assert(hb_boot_stage(NULL) == 1 && hb_boot_stage("") == 1);
	assert(hb_boot_stage("single") == 0 && hb_boot_stage("multiuser") == 1);
	assert(hb_boot_stage("network") == 2);
	assert(hb_boot_stage("unknown") == -1 && errno == EINVAL);
	puts("PASS multiuser default and explicit single/network recovery stages");
	channel(); trace(); children(); return 0;
}
