/* SPDX-License-Identifier: BSD-2-Clause */
/* Actual PID 1 stopped state; injected operations never signal/reboot the host. */
#include <assert.h>
#include <stdbool.h>
#include <signal.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/wait.h>
#define __unused __attribute__((unused))
#define _PATH_CONSOLE "/dev/console"
typedef long (*state_func_t)(void);
typedef state_func_t (*state_t)(void);
static state_func_t stopped(void);
typedef struct test_session { struct test_session *se_next; int se_flags; } session_t;
#define SE_SHUTDOWN 1
static session_t session;
static session_t *sessions = &session;
static bool Reboot;
static int howto, reaped, slept, notifications, shutdowns;
static state_t requested_transition;
static void sync_fixture(void) { assert(!"unexpected sync"); }
static int reboot_fixture(int flags) { (void)flags; assert(!"unexpected reboot"); return -1; }
static void emergency(const char *fmt, ...) { (void)fmt; assert(!"unexpected emergency"); }
static void lua_shutdown(void) { ++shutdowns; }
static void hb_boot_trace(const char *path, const char *text) { assert(path && text); ++notifications; }
static void collect_child(pid_t child) { assert(child == 17); ++reaped; }
static pid_t wait_fixture(pid_t child, int *status, int flags) {
    assert(child == -1 && status && flags == WNOHANG);
    return reaped == 0 ? 17 : 0;
}
static int action_fixture(int signo, const struct sigaction *action, struct sigaction *old) {
    assert(signo == SIGCHLD && action->sa_handler && !old); return 0;
}
static int mask_fixture(int how, const sigset_t *set, sigset_t *old) {
    assert(set);
    if(how == SIG_BLOCK) { assert(old); sigfillset(old); }
    else { assert(how == SIG_SETMASK && !old); }
    return 0;
}
static int suspend_fixture(const sigset_t *set) {
    assert(sigismember(set, SIGCHLD) == 0);
    ++slept; Reboot = true; requested_transition = stopped; return -1;
}
#define sync sync_fixture
#define reboot reboot_fixture
#define waitpid wait_fixture
/* struct sigaction must retain its native type name. */
#define sigprocmask mask_fixture
#define sigsuspend suspend_fixture
#include "stopped.inc"
int main(void) {
    state_func_t result = stopped();
    assert(result == (state_func_t)stopped);
    assert(shutdowns == 1 && reaped == 1 && slept == 1 && notifications == 1);
    assert(requested_transition == NULL);
    assert(session.se_flags & SE_SHUTDOWN);
    return 0;
}
