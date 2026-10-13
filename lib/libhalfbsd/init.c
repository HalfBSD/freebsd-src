/* SPDX-License-Identifier: BSD-2-Clause */
#include "internal.h"
void halfbsd_openlibs(lua_State*L) {
	luaL_openlibs(L);
	luaL_getsubtable(L,LUA_REGISTRYINDEX,LUA_PRELOAD_TABLE);
	static const luaL_Reg modules[]={{"proc",luaopen_proc},{"halfbsd.event",luaopen_halfbsd_event},{"halfbsd.os",luaopen_halfbsd_os},{"halfbsd.log",luaopen_halfbsd_log},{"halfbsd.ipc",luaopen_halfbsd_ipc},{NULL,NULL}};
	luaL_setfuncs(L,modules,0);
	lua_pop(L,1);
}
