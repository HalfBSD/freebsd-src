/* SPDX-License-Identifier: BSD-2-Clause */
#ifndef INIT_SHUTDOWN_REPORT_H
#define INIT_SHUTDOWN_REPORT_H
#include <sys/types.h>
#include <time.h>
struct hb_shutdown_report {
	/* Borrowed path must remain valid through finish; no descriptors retained. */
	const char *directory;
	dev_t device;
	ino_t inode;
	pid_t pid;
	char boot_id[96];
	struct timespec started;
};
int hb_shutdown_report_begin(struct hb_shutdown_report *, const char *, pid_t);
/* Call finish only after begin succeeds; neither call retains open fds. */
int hb_shutdown_report_finish(struct hb_shutdown_report *, int, int, int);
#endif
