/* SPDX-License-Identifier: BSD-2-Clause */
#ifndef HALFBSD_INTERNAL_H
#define HALFBSD_INTERNAL_H
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <signal.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <halfproc.h>
#include "halfbsd_lua.h"
#define HB_FD "halfbsd.fd"
#define HB_CHILD "halfbsd.child"
#define HB_MAX_IO (64U * 1024U * 1024U)
struct hb_fd { struct hp_fd *fd; };
struct hb_child { struct hp_process *process; };
int hb_listdir(lua_State *);
int hb_interfaces(lua_State *);
int hb_sync(lua_State *);
int hb_swap_on(lua_State *);
int hb_swap_off(lua_State *);
int hb_fstab(lua_State *);
int hb_mounts(lua_State *);
int hb_processes(lua_State *);
int hb_terminal(lua_State *);
int hb_terminal_restore(lua_State *);
int hb_interface_up(lua_State *);
int hb_default_routes(lua_State *);
const char *hb_string(lua_State *, int, size_t *);
int hb_error(lua_State *, const char *, const char *, int);
int hb_timeout(lua_State *, int, int);
int hb_boolean(lua_State *, int);
void hb_options(lua_State *, int, const char *const *);
void hb_result(lua_State *, const struct hp_result *);
int hb_signal_number(lua_State *, int);
void hb_push_fd(lua_State *, struct hp_fd *);
struct hp_fd *hb_check_fd(lua_State *, int);
void hb_fd_metatable(lua_State *);
int hb_signal_disposition(int, int, struct sigaction *);
void hb_signal_restore(int, const struct sigaction *);
#endif
