/* SPDX-License-Identifier: BSD-2-Clause */
#include <sys/socket.h>
#include <net/if.h>
#ifdef __FreeBSD__
#include <net/if_dl.h>
#endif
#include <netinet/in.h>
#include <dirent.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <stddef.h>
#include "internal.h"
#define HB_DISCOVERY "halfbsd.discovery_guard"
struct discovery { DIR *dir; struct ifaddrs *interfaces; };
static int release(lua_State *L)
{
	struct discovery *g = luaL_checkudata(L, 1, HB_DISCOVERY);
	if (g->dir) closedir(g->dir);
	if (g->interfaces) freeifaddrs(g->interfaces);
	g->dir = NULL; g->interfaces = NULL;
	return 0;
}
static struct discovery *guard(lua_State *L)
{
	if (luaL_newmetatable(L, HB_DISCOVERY)) {
		lua_pushcfunction(L, release); lua_setfield(L, -2, "__gc");
	}
	lua_pop(L, 1);
	struct discovery *g = lua_newuserdatauv(L, sizeof(*g), 0);
	memset(g, 0, sizeof(*g)); luaL_setmetatable(L, HB_DISCOVERY);
	return g;
}
int hb_listdir(lua_State *L)
{
	const char *path = hb_string(L, 1, NULL);
	struct discovery *g = guard(L);
	g->dir = opendir(path);
	if (!g->dir) return hb_error(L, "listdir", NULL, errno);
	if (fcntl(dirfd(g->dir), F_SETFD, FD_CLOEXEC) < 0) {
		int e = errno; closedir(g->dir); g->dir = NULL;
		return hb_error(L, "listdir", NULL, e);
	}
	lua_newtable(L); int table = lua_gettop(L); lua_Integer n = 0;
	for (;;) {
		errno = 0; struct dirent *entry = readdir(g->dir);
		if (!entry) {
			int e = errno; closedir(g->dir); g->dir = NULL;
			if (e) return hb_error(L, "listdir", NULL, e);
			break;
		}
		if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) continue;
		if (n == 65536) { closedir(g->dir); g->dir = NULL; return hb_error(L, "listdir", NULL, EOVERFLOW); }
		lua_pushstring(L, entry->d_name); lua_rawseti(L, table, ++n);
	}
	return 1;
}
int hb_interfaces(lua_State *L)
{
	struct discovery *g = guard(L);
	if (getifaddrs(&g->interfaces) < 0) return hb_error(L, "interfaces", NULL, errno);
	lua_newtable(L); lua_Integer count = 0;
	for (struct ifaddrs *a = g->interfaces; a; a = a->ifa_next) {
		if (!a->ifa_addr) continue;
		int family = a->ifa_addr->sa_family;
		int link = 0;
#ifdef __FreeBSD__
		link = family == AF_LINK;
#else
		link = family == AF_PACKET;
#endif
		if (family != AF_INET && family != AF_INET6 && !link) continue;
		if (count == 65536) {
			freeifaddrs(g->interfaces); g->interfaces = NULL;
			return hb_error(L, "interfaces", NULL, EOVERFLOW);
		}
		char host[NI_MAXHOST] = {0};
		socklen_t length = family == AF_INET ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6);
		int error = link ? 0 : getnameinfo(a->ifa_addr, length, host, sizeof(host), NULL, 0, NI_NUMERICHOST);
		if (error) { int code = error == EAI_SYSTEM ? errno : EINVAL; freeifaddrs(g->interfaces); g->interfaces = NULL; return hb_error(L, "interfaces", "address", code); }
		lua_createtable(L, 0, 5);
		lua_pushstring(L, a->ifa_name); lua_setfield(L, -2, "name");
		lua_pushstring(L, host); lua_setfield(L, -2, "address");
		lua_pushstring(L, link ? "link" : "inet");
		if (family == AF_INET6) { lua_pop(L, 1); lua_pushliteral(L, "inet6"); }
		lua_setfield(L, -2, "family");
#ifdef __FreeBSD__
		if (link) {
			struct sockaddr_dl link_address = {0};
			size_t link_size = offsetof(struct sockaddr_dl, sdl_type) +
			    sizeof(link_address.sdl_type);
			if (a->ifa_addr->sa_len < link_size) {
				freeifaddrs(g->interfaces); g->interfaces = NULL;
				return hb_error(L, "interfaces", "link address", EPROTO);
			}
			/* sockaddr does not guarantee sockaddr_dl's alignment. */
			memcpy(&link_address, a->ifa_addr, link_size);
			lua_pushinteger(L, link_address.sdl_type);
			lua_setfield(L, -2, "link_type");
			if (a->ifa_data) {
				struct if_data data;
				memcpy(&data, a->ifa_data, sizeof(data));
				lua_pushinteger(L, data.ifi_link_state);
				lua_setfield(L, -2, "link_state");
			}
		}
#endif
		lua_pushboolean(L, (a->ifa_flags & IFF_UP) != 0); lua_setfield(L, -2, "up");
		lua_pushboolean(L, (a->ifa_flags & IFF_LOOPBACK) != 0); lua_setfield(L, -2, "loopback");
		lua_rawseti(L, -2, ++count);
	}
	freeifaddrs(g->interfaces); g->interfaces = NULL;
	return 1;
}
