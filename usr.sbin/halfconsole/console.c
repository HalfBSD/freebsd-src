/* SPDX-License-Identifier: BSD-2-Clause */
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <lua.h>
#include <lauxlib.h>
#include <halfbsd_lua.h>
static int
launch(lua_State *L)
{
	halfbsd_openlibs(L);
	lua_getglobal(L, "package");
	lua_pushliteral(L, "/usr/lib/halfbsd/?.lua");
	lua_setfield(L, -2, "path");
	lua_pushliteral(L, "");
	lua_setfield(L, -2, "cpath");
	lua_pop(L, 1);
	lua_getglobal(L, "require");
	lua_pushliteral(L, "console");
	lua_call(L, 1, 1);
	lua_getfield(L, -1, "main");
	lua_newtable(L);
	lua_call(L, 1, 1);
	return 1;
}

int
main(int argc, char **argv)
{
	(void)argv;
	if (argc != 1 || getuid() != 0 || geteuid() != 0 ||
	    getppid() != 1 || !isatty(STDIN_FILENO)) {
		fprintf(stderr, "halfconsole requires a root console session launched by PID 1\n");
		return 1;
	}
	lua_State *L = luaL_newstate();
	if (L == NULL) {
		fprintf(stderr, "halfconsole: cannot allocate Lua state\n");
		return 1;
	}
	lua_pushcfunction(L, launch);
	int result = lua_pcall(L, 0, 1, 0);
	char *user = NULL;
	if (result == LUA_OK && lua_type(L, -1) == LUA_TSTRING) {
		user = strdup(lua_tostring(L, -1));
		if (user == NULL)
			fprintf(stderr, "halfconsole: allocation failure; using authenticated login\n");
	} else {
		fprintf(stderr, "halfconsole: %s; using authenticated login\n",
		    lua_tostring(L, -1) ? lua_tostring(L, -1) : "invalid console policy");
	}
	lua_close(L);
	/* Do not create a supervisor child: login must inherit this session/tty. */
	if (user != NULL && user[0] != '\0')
		execl("/usr/bin/login", "login", "-f", "--", user, (char *)NULL);
	else
		execl("/usr/bin/login", "login", (char *)NULL);
	perror("halfconsole: exec login");
	free(user);
	return 1;
}
