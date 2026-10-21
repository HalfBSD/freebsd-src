/* SPDX-License-Identifier: BSD-2-Clause */
#ifndef HALFBSD_CONSOLE_INPUT_H
#define HALFBSD_CONSOLE_INPUT_H
#include <stddef.h>
/* 1: complete line, 0: deadline/default, -1: errno failure. No fd ownership. */
int hb_console_choose(int, double, char *, size_t);
#endif
