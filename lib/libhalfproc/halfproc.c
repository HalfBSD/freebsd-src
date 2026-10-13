/* SPDX-License-Identifier: BSD-2-Clause
 * Native child ownership: descriptor identity, autonomous reaping, no GC kills.
 */
#include <sys/types.h>
#include <sys/event.h>
#include <sys/procdesc.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <limits.h>
#include <poll.h>
#include <pthread.h>
#include <signal.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "halfproc.h"
#include <termios.h>

struct hp_io {
	atomic_uint refs;
	pthread_mutex_t lock;
	int fd[2];
	struct hp_fd *end[2];
};
struct hp_fd { struct hp_io *io; int index; };
struct hp_context {
	atomic_uint refs;
	pthread_mutex_t lock;
	pthread_cond_t changed;
	int kq, closing, monitor_error;
	size_t count;
	struct hp_process *head;
};
struct hp_process {
	atomic_uint refs;
	struct hp_context *ctx;
	struct hp_process *next;
	int pd;
	struct hp_result result;
	struct hp_fd *keep[3];
};

static int
above_stdio(int fd)
{
	int copy, saved;
	if (fd < 0 || fd >= 3) return fd;
	copy = fcntl(fd, F_DUPFD_CLOEXEC, 3);
	saved = errno;
	close(fd);
	errno = saved;
	return copy;
}

static struct hp_io *
io_new(int first, int second)
{
	struct hp_io *io = calloc(1, sizeof(*io));
	int e;
	if (!io) goto failed;
	e = pthread_mutex_init(&io->lock, NULL);
	if (e) { free(io); io = NULL; errno = e; goto failed; }
	io->fd[0] = first; io->fd[1] = second;
	io->end[0] = calloc(2, sizeof(struct hp_fd));
	if (!io->end[0]) {
		pthread_mutex_destroy(&io->lock); free(io); io = NULL;
		goto failed;
	}
	io->end[1] = io->end[0] + 1;
	io->end[0]->io = io; io->end[1]->io = io;
	io->end[1]->index = 1;
	atomic_init(&io->refs, second < 0 ? 1 : 2);
	return io;
failed:
	e = errno;
	if (first >= 0) close(first);
	if (second >= 0) close(second);
	errno = e;
	return NULL;
}

struct hp_fd *
hp_fd_new(int fd)
{
	struct hp_io *io;
	fd = above_stdio(fd);
	if (fd < 0) return NULL;
	if (fcntl(fd, F_SETFD, FD_CLOEXEC) < 0) {
		int e = errno; close(fd); errno = e; return NULL;
	}
	io = io_new(fd, -1);
	return io ? io->end[0] : NULL;
}

int
hp_pipe(struct hp_fd **reader, struct hp_fd **writer, int nonblock)
{
	int fds[2], e;
	struct hp_io *io;
	if (pipe2(fds, O_CLOEXEC | (nonblock ? O_NONBLOCK : 0)) < 0) return -1;
	fds[0] = above_stdio(fds[0]);
	if (fds[0] < 0) { e = errno; close(fds[1]); errno = e; return -1; }
	fds[1] = above_stdio(fds[1]);
	if (fds[1] < 0) { e = errno; close(fds[0]); errno = e; return -1; }
	io = io_new(fds[0], fds[1]);
	if (!io) return -1;
	*reader = io->end[0]; *writer = io->end[1];
	return 0;
}
void hp_fd_ref(struct hp_fd *fd) { atomic_fetch_add(&fd->io->refs, 1); }
void
hp_fd_unref(struct hp_fd *fd)
{
	struct hp_io *io;
	if (!fd) return;
	io = fd->io;
	if (atomic_fetch_sub(&io->refs, 1) != 1) return;
	if (io->fd[0] >= 0) close(io->fd[0]);
	if (io->fd[1] >= 0) close(io->fd[1]);
	pthread_mutex_destroy(&io->lock);
	free(io->end[0]); free(io);
}
int
hp_fd_close(struct hp_fd *fd)
{
	int result = 0, e = 0;
	pthread_mutex_lock(&fd->io->lock);
	if (fd->io->fd[fd->index] >= 0) {
		result = close(fd->io->fd[fd->index]); e = errno;
		fd->io->fd[fd->index] = -1;
	}
	pthread_mutex_unlock(&fd->io->lock);
	errno = e; return result;
}
int
hp_fd_dup(struct hp_fd *fd)
{
	int result, e;
	pthread_mutex_lock(&fd->io->lock);
	result = fcntl(fd->io->fd[fd->index], F_DUPFD_CLOEXEC, 3); e = errno;
	pthread_mutex_unlock(&fd->io->lock);
	errno = e; return result;
}
int
hp_fd_number(struct hp_fd *fd)
{
	int result;
	pthread_mutex_lock(&fd->io->lock); result = fd->io->fd[fd->index];
	pthread_mutex_unlock(&fd->io->lock);
	if (result < 0) errno = EBADF;
	return result;
}
int
hp_fd_nonblock(struct hp_fd *fd, int enabled)
{
	int flags, result = -1, e;
	pthread_mutex_lock(&fd->io->lock);
	flags = fcntl(fd->io->fd[fd->index], F_GETFL);
	if (flags >= 0) result = fcntl(fd->io->fd[fd->index], F_SETFL,
	    enabled ? flags | O_NONBLOCK : flags & ~O_NONBLOCK);
	e = errno; pthread_mutex_unlock(&fd->io->lock); errno = e; return result;
}

/* Do not let a broken service pipe terminate the supervisor. Preserve the
 * caller's mask and any SIGPIPE which was already pending. */
ssize_t
hp_write(int fd, const void *data, size_t length)
{
	sigset_t block, old, pending;
	struct timespec zero = {0, 0};
	ssize_t result;
	int e, had_pending, mask_error;
	sigemptyset(&block); sigaddset(&block, SIGPIPE);
	mask_error = pthread_sigmask(SIG_BLOCK, &block, &old);
	if (mask_error) { errno = mask_error; return -1; }
	if (sigpending(&pending) < 0) {
		e = errno; pthread_sigmask(SIG_SETMASK, &old, NULL); errno = e; return -1;
	}
	had_pending = sigismember(&pending, SIGPIPE);
	do { result = write(fd, data, length); } while (result < 0 && errno == EINTR);
	e = errno;
	if (result < 0 && e == EPIPE && !had_pending)
		while (sigtimedwait(&block, NULL, &zero) < 0 && errno == EINTR) {}
	pthread_sigmask(SIG_SETMASK, &old, NULL);
	errno = e; return result;
}

static void ctx_ref(struct hp_context *ctx) { atomic_fetch_add(&ctx->refs, 1); }
static void
ctx_unref(struct hp_context *ctx)
{
	if (atomic_fetch_sub(&ctx->refs, 1) != 1) return;
	close(ctx->kq); pthread_cond_destroy(&ctx->changed);
	pthread_mutex_destroy(&ctx->lock); free(ctx);
}
void hp_process_ref(struct hp_process *p) { atomic_fetch_add(&p->refs, 1); }
void
hp_process_unref(struct hp_process *p)
{
	if (!p || atomic_fetch_sub(&p->refs, 1) != 1) return;
	for (int i = 0; i < 3; i++) hp_fd_unref(p->keep[i]);
	ctx_unref(p->ctx); free(p);
}

/* Caller holds the context lock; only this reaper calls pdwait. */
static void
reap(struct hp_context *ctx)
{
	struct hp_process **link = &ctx->head, *p;
	int status, e;
	while ((p = *link) != NULL) {
		do { e = pdwait(p->pd, &status, WEXITED | WNOHANG, NULL, NULL); }
		while (e < 0 && errno == EINTR);
		if (e < 0 && errno != ECHILD && errno != ESRCH) {
			if (errno != EWOULDBLOCK) ctx->monitor_error = errno;
			link = &p->next; continue;
		}
		p->result.error = e < 0 ? errno : 0;
		p->result.status = e < 0 ? 0 : status;
		p->result.done = 1;
		clock_gettime(CLOCK_MONOTONIC, &p->result.finished);
		*link = p->next; ctx->count--;
		close(p->pd); p->pd = -1;
		for (int i = 0; i < 3; i++) {
			hp_fd_unref(p->keep[i]); p->keep[i] = NULL;
		}
		pthread_cond_broadcast(&ctx->changed);
		hp_process_unref(p); /* Release active-list ownership. */
	}
}
static void *
reaper(void *argument)
{
	struct hp_context *ctx = argument;
	struct kevent event;
	struct timespec backoff = {1, 0};
	int n, e;
	for (;;) {
		n = kevent(ctx->kq, NULL, 0, &event, 1, NULL); e = errno;
		pthread_mutex_lock(&ctx->lock);
		if (n < 0 && e != EINTR) ctx->monitor_error = e;
		else if (n > 0) ctx->monitor_error = 0;
		reap(ctx);
		if (ctx->closing && ctx->count == 0) {
			pthread_mutex_unlock(&ctx->lock); break;
		}
		pthread_cond_broadcast(&ctx->changed);
		pthread_mutex_unlock(&ctx->lock);
		/* Exceptional kernel/resource failure recovery, never a spin loop. */
		if (n < 0 && e != EINTR) nanosleep(&backoff, NULL);
	}
	ctx_unref(ctx); return NULL;
}
struct hp_context *
hp_context_new(void)
{
	struct hp_context *ctx = calloc(1, sizeof(*ctx));
	pthread_condattr_t attr;
	pthread_attr_t thread_attr;
	pthread_t thread;
	struct kevent events[2];
	int e;
	if (!ctx) return NULL;
	ctx->kq = above_stdio(kqueue());
	if (ctx->kq < 0) { free(ctx); return NULL; }
	if (fcntl(ctx->kq, F_SETFD, FD_CLOEXEC) < 0) goto fail_fd;
	e = pthread_mutex_init(&ctx->lock, NULL); if (e) { errno = e; goto fail_fd; }
	e = pthread_condattr_init(&attr); if (e) { errno = e; goto fail_lock; }
	e = pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);
	if (!e) e = pthread_cond_init(&ctx->changed, &attr);
	pthread_condattr_destroy(&attr);
	if (e) { errno = e; goto fail_lock; }
	EV_SET(&events[0], 1, EVFILT_USER, EV_ADD | EV_CLEAR, 0, 0, NULL);
	/* A sleeping kernel timer also handles failed registrations and external
	 * reaping interference; normal exit delivery remains descriptor-driven. */
	EV_SET(&events[1], 2, EVFILT_TIMER, EV_ADD, 0, 1000, NULL);
	if (kevent(ctx->kq, events, 2, NULL, 0, NULL) < 0) goto fail_cond;
	atomic_init(&ctx->refs, 2); /* Caller and detached reaper. */
	e = pthread_attr_init(&thread_attr);
	if (e) { errno = e; goto fail_cond; }
	e = pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);
	if (!e) e = pthread_create(&thread, &thread_attr, reaper, ctx);
	pthread_attr_destroy(&thread_attr);
	if (e) { errno = e; goto fail_cond; }
	return ctx;
fail_cond:
	e = errno; pthread_cond_destroy(&ctx->changed); errno = e;
fail_lock:
	e = errno; pthread_mutex_destroy(&ctx->lock); errno = e;
fail_fd:
	e = errno; close(ctx->kq); free(ctx); errno = e; return NULL;
}
void
hp_context_close(struct hp_context *ctx)
{
	struct kevent wake;
	pthread_mutex_lock(&ctx->lock); ctx->closing = 1;
	pthread_mutex_unlock(&ctx->lock);
	EV_SET(&wake, 1, EVFILT_USER, 0, NOTE_TRIGGER, 0, NULL);
	(void)kevent(ctx->kq, &wake, 1, NULL, 0, NULL);
	ctx_unref(ctx);
}

struct child_error { int code; int stage; };
static const char *const stages[] = {
	"exec", "descriptors", "signals", "session", "cwd", "groups", "gid", "uid"
};
static void
child_fail(int fd, int stage)
{
	struct child_error failure = {errno, stage};
	const char *bytes = (const char *)&failure;
	size_t left = sizeof(failure);
	while (left) {
		ssize_t n = write(fd, bytes, left);
		if (n < 0 && errno == EINTR) continue;
		if (n <= 0) break;
		bytes += n; left -= (size_t)n;
	}
	_exit(127);
}
static int compare_int(const void *a, const void *b) {
	int x = *(const int *)a, y = *(const int *)b; return (x > y) - (x < y);
}
struct hp_process *
hp_spawn(struct hp_context *ctx, const struct hp_spec *spec, const char **stage)
{
	int std[3] = {-1, -1, -1}, pipefd[2] = {-1, -1};
	int preserve[HP_MAX_INHERIT + 1], nsave, e, status;
	struct hp_process *p = NULL;
	struct child_error failure;
	struct kevent event;
	struct pollfd pollfd;
	pid_t pid;
	size_t received = 0;
	*stage = "arguments";
	if (!spec || !spec->argv || !spec->argv[0] || !spec->env ||
	    (spec->ninherit && !spec->inherit) ||
	    spec->ninherit > HP_MAX_INHERIT || spec->exec_timeout_ms < 0 ||
	    spec->ngroups < 0 || (spec->ngroups && !spec->groups)) { errno = EINVAL; return NULL; }
	for (size_t i = 0; i < spec->ninherit; i++) {
		if (spec->inherit[i] < 3 || fcntl(spec->inherit[i], F_GETFD) < 0) {
			if (spec->inherit[i] < 3) errno = EINVAL;
			return NULL;
		}
		preserve[i] = spec->inherit[i];
	}
	*stage = "descriptors";
	for (int i = 0; i < 3; i++) {
		if (spec->stdfd[i] < 0) continue;
		std[i] = fcntl(spec->stdfd[i], F_DUPFD_CLOEXEC, 3);
		if (std[i] < 0) goto failed;
	}
	if (pipe2(pipefd, O_CLOEXEC) < 0) goto failed;
	pipefd[0] = above_stdio(pipefd[0]);
	if (pipefd[0] < 0) goto failed;
	pipefd[1] = above_stdio(pipefd[1]);
	if (pipefd[1] < 0) goto failed;
	preserve[spec->ninherit] = pipefd[1];
	nsave = (int)spec->ninherit + 1;
	qsort(preserve, (size_t)nsave, sizeof(int), compare_int);
	for (int i = 1; i < nsave; i++)
		if (preserve[i] == preserve[i-1]) { errno = EINVAL; goto failed; }
	p = calloc(1, sizeof(*p)); if (!p) goto failed;
	p->pd = -1; p->ctx = ctx; atomic_init(&p->refs, 1); ctx_ref(ctx);
	for (int i = 0; i < 3; i++) if ((p->keep[i] = spec->keep[i])) hp_fd_ref(p->keep[i]);
	pthread_mutex_lock(&ctx->lock);
	if (ctx->closing || ctx->monitor_error || ctx->count >= HP_MAX_CHILDREN) {
		e = ctx->closing ? ESHUTDOWN : (ctx->monitor_error ? ctx->monitor_error : EAGAIN);
		pthread_mutex_unlock(&ctx->lock); errno = e; goto failed;
	}
	*stage = "fork";
	pid = pdfork(&p->pd, PD_CLOEXEC | PD_DAEMON);
	if (pid < 0) {
		e = errno; pthread_mutex_unlock(&ctx->lock); errno = e; goto failed;
	}
	if (pid == 0) {
		struct sigaction action;
		sigset_t empty;
		unsigned int low = 3;
		for (int i = 0; i < 3; i++) {
			if (std[i] < 0) close(i);
			else if (dup2(std[i], i) < 0) child_fail(pipefd[1], 1);
		}
		for (size_t i = 0; i < spec->ninherit; i++)
			if (fcntl(spec->inherit[i], F_SETFD, 0) < 0) child_fail(pipefd[1], 1);
		for (int i = 0; i < nsave; i++) {
			if (low < (unsigned int)preserve[i] &&
			    close_range(low, (unsigned int)preserve[i]-1, 0) < 0) child_fail(pipefd[1], 1);
			low = (unsigned int)preserve[i] + 1;
		}
		if (close_range(low, UINT_MAX, 0) < 0) child_fail(pipefd[1], 1);
		memset(&action, 0, sizeof(action)); action.sa_handler = SIG_DFL;
		sigemptyset(&action.sa_mask);
		for (int i = 1; i < NSIG; i++) if (i != SIGKILL && i != SIGSTOP)
			if (sigaction(i, &action, NULL) < 0 && errno != EINVAL) child_fail(pipefd[1], 2);
		sigemptyset(&empty);
		if (sigprocmask(SIG_SETMASK, &empty, NULL) < 0) child_fail(pipefd[1], 2);
		if (spec->session && setsid() < 0) child_fail(pipefd[1], 3);
		if (!spec->session && spec->group && setpgid(0, 0) < 0) child_fail(pipefd[1], 3);
		if (spec->cwd && chdir(spec->cwd) < 0) child_fail(pipefd[1], 4);
		if (spec->credentials) {
			if (setgroups(spec->ngroups, spec->groups) < 0) child_fail(pipefd[1], 5);
			if (setresgid(spec->gid, spec->gid, spec->gid) < 0) child_fail(pipefd[1], 6);
			if (setresuid(spec->uid, spec->uid, spec->uid) < 0) child_fail(pipefd[1], 7);
		}
		execve(spec->argv[0], spec->argv, spec->env); child_fail(pipefd[1], 0);
	}
	int descriptor_error = 0;
	if (p->pd < 3) {
		int copy = fcntl(p->pd, F_DUPFD_CLOEXEC, 3);
		if (copy < 0) descriptor_error = errno;
		else { close(p->pd); p->pd = copy; }
	}
	p->result.pid = pid; p->result.own_group = spec->session || spec->group;
	clock_gettime(CLOCK_MONOTONIC, &p->result.started);
	p->next = ctx->head; ctx->head = p; ctx->count++; hp_process_ref(p);
	EV_SET(&event, p->pd, EVFILT_PROCDESC, EV_ADD | EV_ONESHOT, NOTE_EXIT, 0, NULL);
	status = kevent(ctx->kq, &event, 1, NULL, 0, NULL); e = errno;
	pthread_mutex_unlock(&ctx->lock);
	for (int i = 0; i < 3; i++) { if (std[i] >= 0) close(std[i]); std[i] = -1; }
	close(pipefd[1]); pipefd[1] = -1;
	if (status < 0 || descriptor_error) {
		if (descriptor_error) e = descriptor_error;
		*stage = "monitor"; goto child_failed;
	}
	pollfd.fd = pipefd[0]; pollfd.events = POLLIN | POLLHUP;
	/* poll is only the bounded exec handshake, not process supervision. */
	struct timespec before, now;
	clock_gettime(CLOCK_MONOTONIC, &before);
	int remaining = spec->exec_timeout_ms;
	while (received < sizeof(failure)) {
		clock_gettime(CLOCK_MONOTONIC, &now);
		int64_t elapsed = (now.tv_sec-before.tv_sec)*1000 + (now.tv_nsec-before.tv_nsec)/1000000;
		remaining = elapsed >= spec->exec_timeout_ms ? 0 : spec->exec_timeout_ms-(int)elapsed;
		status = poll(&pollfd, 1, remaining);
		if (status < 0 && errno == EINTR) continue;
		if (status <= 0) { e = status == 0 ? ETIMEDOUT : errno; *stage = "exec_handshake"; goto child_failed; }
		ssize_t n = read(pipefd[0], (char *)&failure+received, sizeof(failure)-received);
		if (n < 0 && errno == EINTR) continue;
		if (n < 0) { e = errno; *stage = "exec_handshake"; goto child_failed; }
		if (n == 0) {
			if (received) { e = EPROTO; *stage = "exec_handshake"; goto child_failed; }
			close(pipefd[0]); return p;
		}
		received += (size_t)n;
	}
	e = failure.code;
	*stage = failure.stage >= 0 && failure.stage < 8 ? stages[failure.stage] : "exec_handshake";
child_failed:
	(void)hp_signal(p, SIGKILL, 0);
	close(pipefd[0]); pipefd[0] = -1;
	hp_process_unref(p); p = NULL; errno = e; return NULL;
failed:
	e = errno;
	for (int i = 0; i < 3; i++) if (std[i] >= 0) close(std[i]);
	if (pipefd[0] >= 0) close(pipefd[0]);
	if (pipefd[1] >= 0) close(pipefd[1]);
	hp_process_unref(p); errno = e; return NULL;
}

static void
deadline(struct timespec *time, int ms)
{
	clock_gettime(CLOCK_MONOTONIC, time);
	time->tv_sec += ms/1000; time->tv_nsec += (ms%1000)*1000000L;
	if (time->tv_nsec >= 1000000000L) { time->tv_sec++; time->tv_nsec -= 1000000000L; }
}
void
hp_status(struct hp_process *p, struct hp_result *result)
{
	pthread_mutex_lock(&p->ctx->lock); *result = p->result;
	pthread_mutex_unlock(&p->ctx->lock);
}
int
hp_wait_any(struct hp_process **processes, size_t count, int timeout,
    size_t *index, struct hp_result *result)
{
	struct hp_context *ctx;
	struct timespec until;
	int e = 0;
	if (!count || count > HP_MAX_CHILDREN || timeout < -1) { errno = EINVAL; return -1; }
	ctx = processes[0]->ctx;
	for (size_t i = 0; i < count; i++) if (processes[i]->ctx != ctx) { errno = EINVAL; return -1; }
	if (timeout >= 0) deadline(&until, timeout);
	pthread_mutex_lock(&ctx->lock);
	for (;;) {
		for (size_t i = 0; i < count; i++) if (processes[i]->result.done) {
			*index = i; *result = processes[i]->result;
			pthread_mutex_unlock(&ctx->lock); return 1;
		}
		if (ctx->monitor_error) { e = ctx->monitor_error; break; }
		if (!timeout) { e = ETIMEDOUT; break; }
		e = timeout < 0 ? pthread_cond_wait(&ctx->changed, &ctx->lock) :
		    pthread_cond_timedwait(&ctx->changed, &ctx->lock, &until);
		if (e) break;
	}
	pthread_mutex_unlock(&ctx->lock); errno = e;
	return e == ETIMEDOUT ? 0 : -1;
}
int hp_wait(struct hp_process *p, int ms, struct hp_result *r) {
	size_t index; return hp_wait_any(&p, 1, ms, &index, r);
}
int
hp_signal(struct hp_process *p, int signal, int group)
{
	int result, e;
	pthread_mutex_lock(&p->ctx->lock);
	if (p->result.done) { pthread_mutex_unlock(&p->ctx->lock); return 0; }
	if (group && !p->result.own_group) { pthread_mutex_unlock(&p->ctx->lock); errno = EINVAL; return -1; }
	/* Holding this lock prevents reaping/reusing the leader PID during killpg. */
	result = group ? killpg(p->result.pid, signal) : pdkill(p->pd, signal);
	e = errno; pthread_mutex_unlock(&p->ctx->lock);
	if (result < 0 && e != ESRCH) { errno = e; return -1; }
	return result < 0 ? 0 : 1;
}
static int
terminal_group(int fd, pid_t group)
{
	sigset_t block, old;
	sigemptyset(&block); sigaddset(&block, SIGTTOU);
	int error = pthread_sigmask(SIG_BLOCK, &block, &old);
	if (error) { errno = error; return -1; }
	int result = tcsetpgrp(fd, group), saved = errno;
	error = pthread_sigmask(SIG_SETMASK, &old, NULL);
	if (result < 0) { errno = saved; return -1; }
	if (error) { errno = error; return -1; }
	return 0;
}
int
hp_foreground(struct hp_process *p, int fd)
{
	int result, saved;
	pthread_mutex_lock(&p->ctx->lock);
	if (p->result.done || !p->result.own_group) {
		saved = p->result.done ? ESRCH : EINVAL;
		pthread_mutex_unlock(&p->ctx->lock); errno = saved; return -1;
	}
	/* Reaping cannot release/reuse the leader PID during this handoff.
	 * A child that read before handoff may have stopped on SIGTTIN. */
	result = terminal_group(fd, p->result.pid); saved = errno;
	if (result == 0 && pdkill(p->pd, SIGCONT) < 0) {
		saved = errno;
		(void)terminal_group(fd, getpgrp());
		result = -1;
	}
	pthread_mutex_unlock(&p->ctx->lock); errno = saved; return result;
}
int
hp_terminal_restore(int fd)
{
	/* This is the caller's own living group, never a saved child PID. */
	return terminal_group(fd, getpgrp());
}
int
hp_descriptor(struct hp_process *p)
{
	int fd, e;
	pthread_mutex_lock(&p->ctx->lock);
	fd = p->pd < 0 ? -1 : fcntl(p->pd, F_DUPFD_CLOEXEC, 3);
	e = p->pd < 0 ? ESRCH : errno;
	pthread_mutex_unlock(&p->ctx->lock); errno = e; return fd;
}
struct hp_fd *
hp_process_stream(struct hp_process *p, int index)
{
	struct hp_fd *fd;
	if (index < 0 || index >= 3) { errno = EINVAL; return NULL; }
	pthread_mutex_lock(&p->ctx->lock); fd = p->keep[index];
	if (fd) hp_fd_ref(fd);
	pthread_mutex_unlock(&p->ctx->lock); return fd;
}
size_t
hp_active(struct hp_context *ctx, struct hp_process **out, size_t capacity)
{
	size_t count = 0;
	pthread_mutex_lock(&ctx->lock);
	for (struct hp_process *p = ctx->head; p && count < capacity; p = p->next) {
		hp_process_ref(p); out[count++] = p;
	}
	pthread_mutex_unlock(&ctx->lock); return count;
}
int
hp_shutdown(struct hp_context *ctx, int grace, int group)
{
	struct hp_process *children[HP_MAX_CHILDREN];
	struct hp_result result;
	if (grace < 0) { errno = EINVAL; return -1; }
	size_t count = hp_active(ctx, children, HP_MAX_CHILDREN);
	struct timespec start, now;
	int error = 0;
	clock_gettime(CLOCK_MONOTONIC, &start);
	for (size_t i = 0; i < count; i++)
		if (hp_signal(children[i], SIGTERM, group && children[i]->result.own_group) < 0 && !error) error = errno;
	/* One deadline for the whole set, then kill all survivors together. */
	for (size_t i = 0; i < count; i++) {
		clock_gettime(CLOCK_MONOTONIC, &now);
		int64_t elapsed = (now.tv_sec-start.tv_sec)*1000 + (now.tv_nsec-start.tv_nsec)/1000000;
		int remaining = elapsed >= grace ? 0 : grace-(int)elapsed;
		int n = hp_wait(children[i], remaining, &result);
		if (n < 0 && !error) error = errno;
	}
	for (size_t i = 0; i < count; i++)
		if (hp_signal(children[i], SIGKILL, group && children[i]->result.own_group) < 0 && !error) error = errno;
	clock_gettime(CLOCK_MONOTONIC, &start);
	for (size_t i = 0; i < count; i++) {
		clock_gettime(CLOCK_MONOTONIC, &now);
		int64_t elapsed = (now.tv_sec-start.tv_sec)*1000 + (now.tv_nsec-start.tv_nsec)/1000000;
		int remaining = elapsed >= 5000 ? 0 : 5000-(int)elapsed;
		if (hp_wait(children[i], remaining, &result) != 1 && !error) error = errno;
		hp_process_unref(children[i]);
	}
	if (error) { errno = error; return -1; }
	return 0;
}
