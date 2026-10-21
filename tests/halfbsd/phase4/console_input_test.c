/* SPDX-License-Identifier: BSD-2-Clause */
#include <sys/types.h>
#include <sys/wait.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#ifdef __FreeBSD__
#include <libutil.h>
#else
#include <pty.h>
#endif
#include "console_input.h"

static void
interrupted(int number)
{
	(void)number;
}

int
main(void)
{
	int master, slave;
	char input[128];
	assert(openpty(&master, &slave, NULL, NULL, NULL) == 0);
	int flags = fcntl(slave, F_GETFL);
	assert(flags >= 0);
	assert(hb_console_choose(slave, 0, input, sizeof(input)) == 0);
	assert(write(master, "charlie\n", 8) == 8);
	assert(hb_console_choose(slave, 1, input, sizeof(input)) == 1);
	assert(strcmp(input, "charlie\n") == 0);
	assert(fcntl(slave, F_GETFL) == flags);
	assert(write(master, "partial", 7) == 7);
	assert(hb_console_choose(slave, 0.02, input, sizeof(input)) == 0);
	assert(write(master, "root\n", 5) == 5);
	assert(hb_console_choose(slave, 1, input, sizeof(input)) == 1);
	assert(strcmp(input, "root\n") == 0);
	assert(write(master, "toolong\n", 8) == 8);
	assert(hb_console_choose(slave, 1, input, 3) == -1 && errno == EOVERFLOW);
	assert(tcflush(slave, TCIFLUSH) == 0);
	struct sigaction action;
	memset(&action, 0, sizeof(action));
	action.sa_handler = interrupted;
	assert(sigemptyset(&action.sa_mask) == 0);
	assert(sigaction(SIGUSR1, &action, NULL) == 0);
	pid_t writer = fork();
	assert(writer >= 0);
	if (writer == 0) {
		struct timespec pause = {0, 20000000};
		(void)nanosleep(&pause, NULL);
		(void)kill(getppid(), SIGUSR1);
		(void)nanosleep(&pause, NULL);
		_exit(write(master, "login\n", 6) == 6 ? 0 : 1);
	}
	assert(hb_console_choose(slave, 1, input, sizeof(input)) == 1);
	assert(strcmp(input, "login\n") == 0);
	int status;
	assert(waitpid(writer, &status, 0) == writer);
	assert(WIFEXITED(status) && WEXITSTATUS(status) == 0);
	close(master);
	assert(hb_console_choose(slave, 1, input, sizeof(input)) == -1);
	close(slave);
	puts("PASS private console PTY selection, deadline, partial-input flush, bounds, EINTR and disconnect");
	return 0;
}
