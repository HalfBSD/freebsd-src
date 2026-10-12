/* SPDX-License-Identifier: BSD-2-Clause */
#ifndef HALFBSD_LUA_H
#define HALFBSD_LUA_H
#include <lua.h>
int luaopen_proc(lua_State *);
int luaopen_halfbsd_os(lua_State *);
int luaopen_halfbsd_event(lua_State *);
int luaopen_halfbsd_log(lua_State *);
void halfbsd_openlibs(lua_State *);
#endif
