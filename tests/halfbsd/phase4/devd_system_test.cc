/* SPDX-License-Identifier: BSD-2-Clause */
/* Compile the actual helper with its global namespace context. Host syscall
 * declarations suffice for this warning regression, not FreeBSD ABI testing. */
#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <paths.h>
struct test_config {
    void close_pidfile() {}
};
static test_config cfg;
#include "devd-system.inc"
int main()
{
    return nomatch == '?' && my_system(nullptr) == 1 ? 0 : 1;
}
