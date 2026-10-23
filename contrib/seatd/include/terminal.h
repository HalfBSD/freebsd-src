#ifndef _SEATD_TERMINAL_H
#define _SEATD_TERMINAL_H

#include <stdbool.h>

int terminal_open(void);

int terminal_set_keyboard(int fd, bool enable);
int terminal_set_graphics(int fd, bool enable);

#endif
