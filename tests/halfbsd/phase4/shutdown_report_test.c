/* SPDX-License-Identifier: BSD-2-Clause */
#include <sys/stat.h>
#include <sys/wait.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "shutdown_report.h"
#include "halfbsd.h"

static void
identity(int dir, pid_t pid)
{
	int fd = openat(dir, "boot-id", O_WRONLY | O_CREAT | O_EXCL, 0600);
	assert(fd >= 0);
	char data[64];
	int n = snprintf(data, sizeof(data), "%jd-100000-100\n", (intmax_t)pid);
	assert(n > 0 && (size_t)n < sizeof(data));
	assert(write(fd, data, (size_t)n) == n);
	assert(close(fd) == 0);
}

static void
record(int dir, char *buffer, size_t capacity)
{
	int fd = openat(dir, "shutdown-pid1.json", O_RDONLY);
	assert(fd >= 0);
	struct stat st;
	assert(fstat(fd, &st) == 0 && (st.st_mode & 0777) == 0600);
	ssize_t n = read(fd, buffer, capacity - 1);
	assert(n > 0);
	buffer[n] = '\0';
	assert(close(fd) == 0);
}

int
main(void)
{
	char path[] = "/tmp/halfbsd-shutdown.XXXXXX";
	assert(mkdtemp(path) != NULL);
	int dir = open(path, O_RDONLY | O_DIRECTORY);
	assert(dir >= 0);
	identity(dir, 42);
	struct hb_shutdown_report report;
	char buffer[1024];
	assert(hb_shutdown_report_begin(&report, path, 42) == 0);
	record(dir, buffer, sizeof(buffer));
	assert(strstr(buffer, "\"phase\":\"waiting\"") != NULL);
	pid_t child = fork();
	assert(child >= 0);
	if (child == 0)
		_exit(0);
	int status;
	assert(waitpid(child, &status, 0) == child);
	assert(hb_shutdown_report_finish(&report, 1, status, 0) == 0);
	record(dir, buffer, sizeof(buffer));
	assert(strstr(buffer, "\"exit_code\":0") != NULL);
	assert(strstr(buffer, "\"boot_id\":\"42-100000-100\"") != NULL);
	/* The forced-stop path uses an isolated child and a readiness pipe. */
	int ready[2];
	assert(pipe(ready) == 0);
	child = fork();
	assert(child >= 0);
	if (child == 0) {
		close(ready[0]);
		(void)signal(SIGTERM, SIG_IGN);
		if (write(ready[1], "R", 1) != 1)
			_exit(1);
		for (;;)
			pause();
	}
	close(ready[1]);
	assert(read(ready[0], buffer, 1) == 1);
	close(ready[0]);
	assert(unlinkat(dir, "boot-id", 0) == 0);
	identity(dir, child);
	assert(hb_shutdown_report_begin(&report, path, child) == 0);
	assert(hb_boot_stop(child, 20, &status) == 1);
	assert(WIFSIGNALED(status) && WTERMSIG(status) == SIGKILL);
	assert(hb_shutdown_report_finish(&report, 1, status, 0) == 0);
	record(dir, buffer, sizeof(buffer));
	assert(strstr(buffer, "\"signal\":9") != NULL);
	assert(hb_shutdown_report_finish(&report, 0, 0, ETIMEDOUT) == 0);
	record(dir, buffer, sizeof(buffer));
	assert(strstr(buffer, "\"wait_result\":0") != NULL);
	assert(hb_shutdown_report_begin(&report, path, 42) == -1 && errno == EINVAL);
	assert(unlinkat(dir, "boot-id", 0) == 0);
	identity(dir, 42);
	assert(chmod(path, 0777) == 0);
	assert(hb_shutdown_report_begin(&report, path, 42) == -1);
	assert(chmod(path, 0700) == 0);
	assert(hb_shutdown_report_begin(&report, path, 42) == 0);
	/* Replacing the pathname must not redirect the final write to a new FS. */
	char moved[128];
	int n = snprintf(moved, sizeof(moved), "%s-old", path);
	assert(n > 0 && (size_t)n < sizeof(moved));
	assert(rename(path, moved) == 0 && mkdir(path, 0700) == 0);
	assert(hb_shutdown_report_finish(&report, 1, 0, 0) == -1 && errno == ESTALE);
	assert(rmdir(path) == 0);
	assert(symlink(moved, path) == 0);
	assert(hb_shutdown_report_begin(&report, path, 42) == -1);
	assert(unlink(path) == 0);
	assert(unlinkat(dir, "boot-id", 0) == 0);
	assert(symlinkat("/dev/null", dir, "boot-id") == 0);
	assert(hb_shutdown_report_begin(&report, moved, 42) == -1);
	assert(unlinkat(dir, "boot-id", 0) == 0);
	assert(unlinkat(dir, "shutdown-pid1.json", 0) == 0);
	assert(close(dir) == 0 && rmdir(moved) == 0);
	puts("PASS persistent PID 1 graceful/forced/timeout reports, atomic replacement, permissions and storage identity");
	return 0;
}
