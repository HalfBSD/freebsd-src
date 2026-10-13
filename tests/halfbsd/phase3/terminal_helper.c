/* SPDX-License-Identifier: BSD-2-Clause
 * Terminal handoff uses a private PTY in a disposable child session.
 */
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <poll.h>
#include <string.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <halfproc.h>
extern char **environ;

static int
exercise(void)
{
	int result = 1, master = -1, slave = -1;
	struct hp_context *ctx = NULL;
	struct hp_process *child = NULL;
	if (setsid() < 0) goto out;
	master = posix_openpt(O_RDWR | O_NOCTTY | O_CLOEXEC);
	if (master < 0 || grantpt(master) < 0 || unlockpt(master) < 0) goto out;
	char *path = ptsname(master);
	if (!path) goto out;
	slave = open(path, O_RDWR | O_NOCTTY | O_CLOEXEC);
	if (slave < 0 || ioctl(slave, TIOCSCTTY, 0) < 0) goto out;
	ctx = hp_context_new(); if (!ctx) goto out;
	char executable[] = "/bin/cat";
	char *arguments[] = {executable, NULL};
	struct termios attributes;
	if (tcgetattr(slave, &attributes) < 0) goto out;
	attributes.c_lflag &= (tcflag_t)~ECHO;
	if (tcsetattr(slave, TCSANOW, &attributes) < 0) goto out;
	struct hp_spec spec = {.argv = arguments, .env = environ,
	    .stdfd = {slave, slave, slave}, .group = 1, .exec_timeout_ms = 3000};
	const char *stage;
	child = hp_spawn(ctx, &spec, &stage); if (!child) goto out;
	struct hp_result status;
	hp_status(child, &status);
	if (hp_signal(child, SIGSTOP, 0) < 0 || hp_foreground(child, slave) < 0 ||
	    tcgetpgrp(slave) != status.pid) goto out;
	if (write(master, "ready\n", 6) != 6) goto out;
	struct pollfd input = {master, POLLIN, 0};
	if (poll(&input, 1, 3000) <= 0) goto out;
	char data[64];
	ssize_t length = read(master, data, sizeof(data));
	if (length < 5 || memcmp(data, "ready", 5) != 0 ||
	    hp_signal(child, SIGTERM, 1) < 0 || hp_wait(child, 3000, &status) != 1)
		goto out;
	errno = 0;
	if (hp_foreground(child, slave) != -1 || errno != ESRCH ||
	    hp_terminal_restore(slave) < 0 || tcgetpgrp(slave) != getpgrp()) goto out;
	result = 0;
out:
	if (ctx && hp_shutdown(ctx, 100, 1) < 0) result = 1;
	if (child) hp_process_unref(child);
	if (ctx) hp_context_close(ctx);
	/* Closing our controlling terminal sends HUP only to this private session. */
	if (slave >= 0) close(slave);
	if (master >= 0) close(master);
	return result;
}
int
main(void)
{
	pid_t pid = fork(); if (pid < 0) return 1;
	if (pid == 0) {
		struct sigaction action = {.sa_handler = SIG_IGN};
		sigemptyset(&action.sa_mask); (void)sigaction(SIGHUP, &action, NULL);
		_exit(exercise());
	}
	int status;
	while (waitpid(pid, &status, 0) < 0) if (errno != EINTR) return 1;
	if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) return 1;
	puts("PASS private PTY handoff, restoration and reaped-handle rejection"); return 0;
}
