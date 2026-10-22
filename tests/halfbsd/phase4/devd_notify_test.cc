/* SPDX-License-Identifier: BSD-2-Clause */
/* Actual notify_clients body; injected send results, no native kernel claims. */
#include <cassert>
#include <cerrno>
#include <cstdarg>
#include <cstdio>
#include <list>
#include <sys/socket.h>
#include <syslog.h>
#include "halfbsd_actions.h"
using std::list;
struct client_t { int fd, socktype; };
static list<client_t> clients;
static unsigned int num_clients;
static int send_error, send_calls, close_calls, logged_priority;
static bool interrupted, partial;
static ssize_t
test_send(int fd, const void *, size_t length, int flags)
{
    assert(fd == 5 && (flags & MSG_NOSIGNAL));
    ++send_calls;
    if (interrupted && send_calls == 1) { errno = EINTR; return -1; }
    if (send_error) { errno = send_error; return -1; }
    return static_cast<ssize_t>(partial ? length - 1 : length);
}
static int test_close(int fd) { assert(fd == 5); ++close_calls; return 0; }
static void devdlog(int priority, const char *, ...) { logged_priority = priority; }
#define send test_send
#define close test_close
#include "devd-notify.inc"
#undef send
#undef close
int
main()
{
    for (int error : {EPIPE, ECONNRESET, ENOTCONN, EAGAIN, ENOBUFS, 0}) {
        clients = {{5, SOCK_STREAM}}; num_clients = 1;
        send_error = error; partial = error == 0; interrupted = false;
        send_calls = close_calls = 0; logged_priority = -1;
        notify_clients("event", 5);
        assert(clients.empty() && num_clients == 0 && close_calls == 1 && send_calls == 1);
        assert(logged_priority == (hb_device_peer_closed(error) ? LOG_DEBUG : LOG_WARNING));
    }
    clients = {{5, SOCK_SEQPACKET}}; num_clients = 1;
    send_error = 0; partial = false; interrupted = true;
    send_calls = close_calls = 0; logged_priority = -1;
    notify_clients("event", 5);
    assert(num_clients == 1 && clients.size() == 1 && send_calls == 2 && close_calls == 0 && logged_priority == -1);
    puts("PASS actual devd notification body: orderly disconnect, slow subscriber, partial delivery, EINTR retry and single cleanup");
}
