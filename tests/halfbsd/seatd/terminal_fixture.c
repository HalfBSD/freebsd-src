/* SPDX-License-Identifier: BSD-2-Clause */
/* Private protocol fixture only. No host console or graphics/input access. */
#include <stdbool.h>
#include <fcntl.h>
#include "terminal.h"
int terminal_open(void) { return open("/dev/null", O_RDWR); }
int terminal_set_keyboard(int fd, bool enable) { (void)fd; (void)enable; return 0; }
int terminal_set_graphics(int fd, bool enable) { (void)fd; (void)enable; return 0; }
