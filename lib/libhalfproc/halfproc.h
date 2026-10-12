/* SPDX-License-Identifier: BSD-2-Clause */
#ifndef HALFBSD_HALFPROC_H
#define HALFBSD_HALFPROC_H
#include <sys/types.h>
#include <stddef.h>
#include <time.h>

#define HP_MAX_CHILDREN 1024
#define HP_MAX_INHERIT 256
struct hp_context;
struct hp_process;
struct hp_fd;
struct hp_spec {
	char *const *argv;
	char *const *env;
	const char *cwd;
	int stdfd[3]; /* -1 closes that standard descriptor. */
	struct hp_fd *keep[3]; /* Retain parent IO resources until exit. */
	const int *inherit;
	size_t ninherit;
	int session, group, credentials;
	uid_t uid;
	gid_t gid;
	const gid_t *groups;
	int ngroups;
	int exec_timeout_ms;
};
struct hp_result {
	pid_t pid;
	int done, status, error, own_group;
	struct timespec started, finished;
};
struct hp_context *hp_context_new(void);
void hp_context_close(struct hp_context *);
struct hp_process *hp_spawn(struct hp_context *, const struct hp_spec *, const char **);
void hp_process_ref(struct hp_process *);
void hp_process_unref(struct hp_process *);
void hp_status(struct hp_process *, struct hp_result *);
int hp_wait(struct hp_process *, int, struct hp_result *);
int hp_wait_any(struct hp_process **, size_t, int, size_t *, struct hp_result *);
int hp_signal(struct hp_process *, int, int);
int hp_descriptor(struct hp_process *);
struct hp_fd *hp_process_stream(struct hp_process *, int);
size_t hp_active(struct hp_context *, struct hp_process **, size_t);
int hp_shutdown(struct hp_context *, int, int);
struct hp_fd *hp_fd_new(int);
int hp_pipe(struct hp_fd **, struct hp_fd **, int);
void hp_fd_ref(struct hp_fd *);
void hp_fd_unref(struct hp_fd *);
int hp_fd_close(struct hp_fd *);
int hp_fd_dup(struct hp_fd *);
int hp_fd_number(struct hp_fd *);
int hp_fd_nonblock(struct hp_fd *, int);
ssize_t hp_write(int, const void *, size_t);
#endif
