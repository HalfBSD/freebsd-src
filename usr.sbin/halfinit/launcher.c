/* SPDX-License-Identifier: BSD-2-Clause */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lua.h>
#include <lauxlib.h>
#include <halfbsd_lua.h>
#ifndef HALF_ENTRY
#define HALF_ENTRY "init"
#endif
#ifndef HALF_MODULE_DIR
#define HALF_MODULE_DIR "/usr/lib/halfbsd"
#endif
struct invocation { int argc; char **argv; const char *directory; };
static int launch(lua_State *L)
{
	struct invocation *call = lua_touserdata(L, lua_upvalueindex(1));
	halfbsd_openlibs(L);
	lua_getglobal(L, "package");
	lua_pushfstring(L, "%s/?.lua", call->directory); lua_setfield(L, -2, "path");
	lua_pushliteral(L, ""); lua_setfield(L, -2, "cpath");
	lua_pop(L, 1);
	lua_getglobal(L, "require"); lua_pushliteral(L, HALF_ENTRY); lua_call(L, 1, 1);
	lua_getfield(L, -1, "main"); lua_remove(L, -2);
	lua_newtable(L); int n = 0;
	for (int i = 1; i < call->argc; i++) {
		if (!strcmp(call->argv[i], "--module-dir")) { i++; continue; }
		lua_pushstring(L, call->argv[i]); lua_rawseti(L, -2, ++n);
	}
	lua_pushstring(L, call->directory); lua_call(L, 2, 1);
	return 1;
}
int main(int argc, char **argv)
{
	struct invocation call = {argc, argv, HALF_MODULE_DIR};
	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "--module-dir")) {
			if (++i == argc || argv[i][0] != '/' || strpbrk(argv[i], ";?")) {
				fprintf(stderr, "--module-dir requires an absolute directory without Lua path separators\n"); return 2;
			}
			call.directory = argv[i];
		}
	}
	lua_State *L = luaL_newstate();
	if (!L) { fprintf(stderr, "cannot allocate Lua state\n"); return 1; }
	lua_pushlightuserdata(L, &call); lua_pushcclosure(L, launch, 1);
	int status = lua_pcall(L, 0, 1, 0), result = 1;
	if (status == LUA_OK && lua_isinteger(L, -1)) result = (int)lua_tointeger(L, -1);
	else {
		const char *message = lua_tostring(L, -1);
		fprintf(stderr, "%s: %s\n", argv[0], message ? message : "structured Lua error");
	}
	lua_close(L); return result;
}
