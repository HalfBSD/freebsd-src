/* SPDX-License-Identifier: BSD-2-Clause */
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
static volatile sig_atomic_t interrupted;
static void caught(int signal) { interrupted = signal; }
static void fail(void) { perror("proc_helper"); exit(99); }
int main(int argc, char **argv)
{
	if (argc < 2) return 98;
	if (!strcmp(argv[1], "mark")) {
		if (argc != 3) return 98;
		sleep(1);
		FILE *marker = fopen(argv[2], "w");
		if (!marker) fail();
		fputs("alive", marker);
		if (fclose(marker)) fail();
		return 0;
	}
	if (!strcmp(argv[1], "exit")) return argc == 3 ? atoi(argv[2]) : 0;
	if (!strcmp(argv[1], "sleep")) {
		if (argc != 3) return 98;
		struct timespec delay = {atoi(argv[2]), 0};
		while (nanosleep(&delay, &delay) < 0) if (errno != EINTR) fail();
		return 0;
	}
	if (!strcmp(argv[1], "group")) {
		int barrier[2]; if (pipe(barrier)) fail();
		signal(SIGTERM, SIG_IGN);
		pid_t descendant = fork(); if (descendant < 0) fail();
		if (descendant == 0) {
			close(barrier[0]); signal(SIGTERM, SIG_DFL);
			if (write(barrier[1], "r", 1) != 1) _exit(99);
			close(barrier[1]); for (;;) pause();
		}
		close(barrier[1]); char byte;
		if (read(barrier[0], &byte, 1) != 1) fail();
		close(barrier[0]);
		puts("ready"); fflush(stdout);
		int status; pid_t waited;
		do { waited = waitpid(descendant, &status, 0); } while (waited < 0 && errno == EINTR);
		if (waited < 0) fail();
		return WIFSIGNALED(status) && WTERMSIG(status) == SIGTERM ? 0 : 99;
	}
	if (!strcmp(argv[1], "ignore")) {
		signal(SIGTERM, SIG_IGN);
		puts("ready"); fflush(stdout);
		for (;;) pause();
	}
	if (!strcmp(argv[1], "signal")) {
		struct sigaction action = {0};
		action.sa_handler = caught; sigemptyset(&action.sa_mask);
		if (sigaction(SIGUSR1, &action, NULL) < 0) fail();
		puts("ready"); fflush(stdout);
		while (!interrupted) pause();
		return 0;
	}
	if (!strcmp(argv[1], "argv")) {
		for (int i = 2; i < argc; i++) {
			for (const unsigned char *s = (const unsigned char *)argv[i]; *s; s++)
				printf("%02x", *s);
			putchar('\n');
		}
		return 0;
	}
	if (!strcmp(argv[1], "env")) {
		if (argc != 3) return 98;
		const char *s = getenv(argv[2]);
		if (s) puts(s);
		return s ? 0 : 10;
	}
	if (!strcmp(argv[1], "cwd")) {
		char *path = getcwd(NULL, 0);
		if (!path) fail();
		puts(path); free(path); return 0;
	}
	if (!strcmp(argv[1], "identity")) {
		printf("%ld %ld %ld %ld %ld %ld\n", (long)getpid(), (long)getpgrp(),
		    (long)getsid(0), (long)getuid(), (long)getgid(), (long)getgroups(0,NULL));
		return 0;
	}
	if (!strcmp(argv[1], "fds")) {
		for (int i = 3; i < 4096; i++) if (fcntl(i, F_GETFD) >= 0) printf("%d\n", i);
		return 0;
	}
	if (!strcmp(argv[1], "copy")) {
		char buffer[8192]; ssize_t n;
		while ((n = read(STDIN_FILENO, buffer, sizeof(buffer))) != 0) {
			if (n < 0) { if (errno == EINTR) continue; fail(); }
			for (ssize_t offset = 0; offset < n;) {
				ssize_t written = write(STDOUT_FILENO, buffer + offset, (size_t)(n-offset));
				if (written < 0) { if (errno == EINTR) continue; fail(); }
				offset += written;
			}
		} return 0;
	}
	if (!strcmp(argv[1], "output")) {
		char buffer[8192]; memset(buffer, 'x', sizeof(buffer));
		for (int i = 0; i < 64; i++) {
			if (fwrite(buffer, sizeof(buffer), 1, stdout) != 1) fail();
			if (fwrite(buffer, sizeof(buffer), 1, stderr) != 1) fail();
		} return 0;
	}
	return 98;
}
