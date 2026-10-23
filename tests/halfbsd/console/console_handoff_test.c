/* SPDX-License-Identifier: BSD-2-Clause */
/* Actual seatd handoff helpers, fault injected without host device access. */
#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <unistd.h>
static int fail_open, fail_keyboard, fail_graphics, keyboard_state, graphics_state, closes;
static int terminal_open(void) { if(fail_open) { errno=EACCES; return -1; } return 19; }
static int terminal_set_keyboard(int fd, bool enable) {
    assert(fd==19); if(fail_keyboard && !enable) { errno=EIO; return -1; }
    keyboard_state=enable; return 0;
}
static int terminal_set_graphics(int fd, bool enable) {
    assert(fd==19); if(fail_graphics && enable) { errno=ENODEV; return -1; }
    graphics_state=enable; return 0;
}
static int close_fixture(int fd) { assert(fd==19); ++closes; return 0; }
#define close close_fixture
#include "console-handoff.inc"
int main(void) {
    keyboard_state=1;
    assert(console_open()==0 && !keyboard_state && graphics_state && closes==1);
    assert(console_close()==0 && keyboard_state && !graphics_state && closes==2);
    fail_graphics=1;
    assert(console_open()==-1 && errno==ENODEV && keyboard_state && !graphics_state && closes==3);
    fail_graphics=0; fail_keyboard=1;
    assert(console_open()==-1 && errno==EIO && keyboard_state && !graphics_state && closes==4);
    fail_keyboard=0; fail_open=1;
    assert(console_open()==-1 && errno==EACCES && closes==4);
    return 0;
}
