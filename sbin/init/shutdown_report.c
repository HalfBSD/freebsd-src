/* SPDX-License-Identifier: BSD-2-Clause */
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "shutdown_report.h"

static int
open_directory(const char *path)
{
	if (path[0] != '/') {
		errno = EINVAL;
		return -1;
	}
	char *copy = strdup(path);
	if (copy == NULL)
		return -1;
	int fd = open("/", O_RDONLY | O_DIRECTORY | O_CLOEXEC);
	char *save = NULL;
	for (char *part = strtok_r(copy, "/", &save); fd >= 0 && part != NULL;
	    part = strtok_r(NULL, "/", &save)) {
		if (strcmp(part, ".") == 0 || strcmp(part, "..") == 0) {
			close(fd);
			fd = -1;
			errno = EINVAL;
			break;
		}
		int next = openat(fd, part, O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
		int saved = errno;
		close(fd);
		fd = next;
		errno = saved;
	}
	free(copy);
	if (fd >= 0) {
		struct stat st;
		if (fstat(fd, &st) == -1) {
			int saved = errno;
			close(fd);
			errno = saved;
			return -1;
		}
		if (st.st_uid != geteuid() || (st.st_mode & 0022) != 0) {
			close(fd);
			errno = EPERM;
			return -1;
		}
	}
	return fd;
}

static int
write_report(struct hb_shutdown_report *r, const char *phase, int result, int status, int error)
{
	int dir = open_directory(r->directory);
	if (dir == -1)
		return -1;
	struct stat st;
	struct timespec now, real;
	int failure = 0, fd = -1;
	if (fstat(dir, &st) == -1) {
		failure = errno;
		goto done;
	}
	if (st.st_dev != r->device || st.st_ino != r->inode) {
		failure = ESTALE;
		goto done;
	}
	if (clock_gettime(CLOCK_MONOTONIC, &now) == -1 || clock_gettime(CLOCK_REALTIME, &real) == -1) {
		failure = errno;
		goto done;
	}
	char data[768];
	int exited = result == 1 && WIFEXITED(status);
	int termination_signal = result == 1 && WIFSIGNALED(status) ? WTERMSIG(status) : 0;
	double elapsed = (double)(now.tv_sec - r->started.tv_sec) +
	    (double)(now.tv_nsec - r->started.tv_nsec) / 1000000000.0;
	int length = snprintf(data, sizeof(data),
	    "{\"version\":1,\"boot_id\":\"%s\",\"phase\":\"%s\",\"manager_pid\":%jd,"
	    "\"wait_result\":%d,\"exited\":%s,\"exit_code\":%d,\"signal\":%d,\"wait_errno\":%d,"
	    "\"elapsed\":%.6f,\"recorded_at\":%jd.%09ld}\n",
	    r->boot_id, phase, (intmax_t)r->pid, result, exited ? "true" : "false",
	    exited ? WEXITSTATUS(status) : -1, termination_signal, error, elapsed,
	    (intmax_t)real.tv_sec, real.tv_nsec);
	if (length < 0 || (size_t)length >= sizeof(data)) {
		failure = EOVERFLOW;
		goto done;
	}
	/* Only this caller writes this private temporary name. No fd pins storage
	 * between snapshots, so logging cannot prevent owned mount cleanup. */
	if (unlinkat(dir, ".shutdown-pid1.tmp", 0) == -1 && errno != ENOENT) {
		failure = errno;
		goto done;
	}
	fd = openat(dir, ".shutdown-pid1.tmp", O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW | O_CLOEXEC, 0600);
	if (fd == -1) {
		failure = errno;
		goto done;
	}
	for (size_t offset = 0; offset < (size_t)length;) {
		ssize_t count = write(fd, data + offset, (size_t)length - offset);
		if (count == -1 && errno == EINTR)
			continue;
		if (count <= 0) {
			failure = count == 0 ? EIO : errno;
			goto done;
		}
		offset += (size_t)count;
	}
	if (fchmod(fd, 0600) == -1 || fsync(fd) == -1) {
		failure = errno;
		goto done;
	}
	if (close(fd) == -1) {
		fd = -1;
		failure = errno;
		goto done;
	}
	fd = -1;
	if (renameat(dir, ".shutdown-pid1.tmp", dir, "shutdown-pid1.json") == -1 || fsync(dir) == -1)
		failure = errno;
done:
	if (fd >= 0)
		close(fd);
	if (failure != 0)
		(void)unlinkat(dir, ".shutdown-pid1.tmp", 0);
	close(dir);
	if (failure != 0) {
		errno = failure;
		return -1;
	}
	return 0;
}

int
hb_shutdown_report_begin(struct hb_shutdown_report *r, const char *directory, pid_t pid)
{
	memset(r, 0, sizeof(*r));
	r->directory = directory;
	r->pid = pid;
	if (pid <= 1) {
		errno = EINVAL;
		return -1;
	}
	int dir = open_directory(directory);
	if (dir == -1)
		return -1;
	struct stat st;
	int fd = -1, failure = 0;
	if (fstat(dir, &st) == -1) {
		failure = errno;
		goto done;
	}
	r->device = st.st_dev;
	r->inode = st.st_ino;
	fd = openat(dir, "boot-id", O_RDONLY | O_NOFOLLOW | O_CLOEXEC | O_NONBLOCK);
	if (fd == -1) {
		failure = errno;
		goto done;
	}
	if (fstat(fd, &st) == -1) {
		failure = errno;
		goto done;
	}
	if (!S_ISREG(st.st_mode) || st.st_uid != geteuid() ||
	    (st.st_mode & 0022) != 0 || st.st_nlink != 1 || st.st_size < 2 || (uintmax_t)st.st_size >= sizeof(r->boot_id)) {
		failure = EINVAL;
		goto done;
	}
	ssize_t count;
	do {
		count = read(fd, r->boot_id, sizeof(r->boot_id) - 1);
	} while (count == -1 && errno == EINTR);
	if (count == -1) {
		failure = errno;
		goto done;
	}
	if (count != st.st_size || r->boot_id[count - 1] != '\n') {
		failure = EINVAL;
		goto done;
	}
	r->boot_id[count - 1] = '\0';
	char prefix[32];
	int prefix_size = snprintf(prefix, sizeof(prefix), "%jd-", (intmax_t)pid);
	if (prefix_size <= 0 || (size_t)prefix_size >= sizeof(prefix) ||
	    strncmp(r->boot_id, prefix, (size_t)prefix_size) != 0) {
		failure = EINVAL;
		goto done;
	}
	for (ssize_t i = 0; i < count - 1; i++) {
		if ((r->boot_id[i] < '0' || r->boot_id[i] > '9') && r->boot_id[i] != '-') {
			failure = EINVAL;
			goto done;
		}
	}
	if (clock_gettime(CLOCK_MONOTONIC, &r->started) == -1)
		failure = errno;
done:
	if (fd >= 0)
		close(fd);
	close(dir);
	if (failure != 0) {
		errno = failure;
		return -1;
	}
	return write_report(r, "waiting", 0, 0, 0);
}

int
hb_shutdown_report_finish(struct hb_shutdown_report *r, int result, int status, int error)
{
	return write_report(r, "finished", result, status, error);
}
