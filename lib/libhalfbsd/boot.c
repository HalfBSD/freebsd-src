/* SPDX-License-Identifier: BSD-2-Clause */
#include <sys/param.h>
#include <sys/mount.h>
#ifdef __FreeBSD__
#include <sys/sysctl.h>
#include <sys/user.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/sockio.h>
#include <net/route.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif
#include <fstab.h>
#include <termios.h>
#include "internal.h"

#define HB_FS_ROWS 1024
#define HB_FS_TEXT (1024 * 1024)
struct fs_row { size_t spec, path, type, options; int pass; };
struct fs_copy { struct fs_row rows[HB_FS_ROWS]; char text[HB_FS_TEXT]; };
int
hb_terminal(lua_State *L)
{
	int tty = isatty(STDIN_FILENO);
	struct termios attributes;
	if (tty && tcgetattr(STDIN_FILENO, &attributes) < 0)
		return hb_error(L, "terminal", NULL, errno);
	lua_createtable(L, 0, 3);
	lua_pushboolean(L, tty); lua_setfield(L, -2, "tty");
	pid_t group = tcgetpgrp(STDIN_FILENO);
	lua_pushboolean(L, group >= 0 && group == getpgrp()); lua_setfield(L, -2, "foreground");
	lua_pushboolean(L, tty && (attributes.c_lflag & TOSTOP) != 0);
	lua_setfield(L, -2, "stop_background_output");
	return 1;
}
int
hb_terminal_restore(lua_State *L)
{
	if (hp_terminal_restore(STDIN_FILENO) < 0)
		return hb_error(L, "terminal_restore", NULL, errno);
	lua_pushboolean(L, 1); return 1;
}

int
hb_fstab(lua_State *L)
{
	/* No Lua allocations while libc's global fstab cursor is open. */
	struct fs_copy *copy = lua_newuserdatauv(L, sizeof(*copy), 0);
	size_t count = 0, used = 0;
	int error = 0;
	errno = 0;
	if (!setfsent()) {
		error = errno ? errno : EIO; endfsent();
		return hb_error(L, "fstab", NULL, error);
	}
	for (;;) {
		errno = 0;
		struct fstab *entry = getfsent();
		if (!entry) { error = errno; break; }
		if (count == HB_FS_ROWS) { error = EOVERFLOW; break; }
		const char *fields[] = {entry->fs_spec, entry->fs_file, entry->fs_vfstype, entry->fs_mntops};
		size_t *offsets[] = {&copy->rows[count].spec, &copy->rows[count].path,
		    &copy->rows[count].type, &copy->rows[count].options};
		for (size_t i = 0; i < 4; i++) {
			if (!fields[i]) { error = EPROTO; break; }
			size_t length = strnlen(fields[i], HB_FS_TEXT);
			if (length >= HB_FS_TEXT - used) { error = EOVERFLOW; break; }
			*offsets[i] = used;
			memcpy(copy->text + used, fields[i], length + 1);
			used += length + 1;
		}
		if (error) break;
		copy->rows[count++].pass = entry->fs_passno;
	}
	endfsent();
	if (error) return hb_error(L, "fstab", NULL, error);
	lua_newtable(L);
	for (size_t i = 0; i < count; i++) {
		lua_createtable(L, 0, 5);
#define FS_STRING(key, member) lua_pushstring(L, copy->text + copy->rows[i].member); lua_setfield(L, -2, key)
		FS_STRING("source", spec); FS_STRING("path", path);
		FS_STRING("type", type); FS_STRING("options", options);
#undef FS_STRING
		lua_pushinteger(L, copy->rows[i].pass); lua_setfield(L, -2, "pass");
		lua_rawseti(L, -2, (lua_Integer)i + 1);
	}
	return 1;
}

int
hb_mounts(lua_State *L)
{
#ifdef __FreeBSD__
	int count = getfsstat(NULL, 0, MNT_NOWAIT);
	if (count < 0) return hb_error(L, "mounts", NULL, errno);
	if (count > 4096) return hb_error(L, "mounts", NULL, EOVERFLOW);
	size_t capacity = (size_t)count + 32;
	struct statfs *list = lua_newuserdatauv(L, capacity * sizeof(*list), 0);
	count = getfsstat(list, (long)(capacity * sizeof(*list)), MNT_NOWAIT);
	if (count < 0) return hb_error(L, "mounts", NULL, errno);
	if ((size_t)count > capacity) return hb_error(L, "mounts", NULL, EAGAIN);
	lua_newtable(L);
	for (int i = 0; i < count; i++) {
		lua_createtable(L, 0, 4);
		lua_pushstring(L, list[i].f_mntonname); lua_setfield(L, -2, "path");
		lua_pushstring(L, list[i].f_mntfromname); lua_setfield(L, -2, "source");
		lua_pushstring(L, list[i].f_fstypename); lua_setfield(L, -2, "type");
		lua_pushboolean(L, (list[i].f_flags & MNT_RDONLY) != 0); lua_setfield(L, -2, "readonly");
		lua_rawseti(L, -2, i + 1);
	}
	return 1;
#else
	return hb_error(L, "mounts", NULL, ENOSYS);
#endif
}

int
hb_processes(lua_State *L)
{
#ifdef __FreeBSD__
	int mib[] = {CTL_KERN, KERN_PROC, KERN_PROC_PROC, 0};
	size_t size = 0;
	if (sysctl(mib, 4, NULL, &size, NULL, 0) < 0)
		return hb_error(L, "processes", NULL, errno);
	if (size > 16 * 1024 * 1024) return hb_error(L, "processes", NULL, EOVERFLOW);
	size += 32 * sizeof(struct kinfo_proc);
	size_t capacity = size;
	struct kinfo_proc *list = lua_newuserdatauv(L, size, 0);
	if (sysctl(mib, 4, list, &size, NULL, 0) < 0)
		return hb_error(L, "processes", NULL, errno == ENOMEM ? EAGAIN : errno);
	if (size > capacity || size % sizeof(*list)) return hb_error(L, "processes", NULL, EPROTO);
	lua_newtable(L);
	for (size_t i = 0; i < size / sizeof(*list); i++) {
		lua_createtable(L, 0, 4);
		lua_pushinteger(L, list[i].ki_pid); lua_setfield(L, -2, "pid");
		lua_pushinteger(L, list[i].ki_uid); lua_setfield(L, -2, "uid");
		lua_pushlstring(L, list[i].ki_comm, strnlen(list[i].ki_comm, sizeof(list[i].ki_comm)));
		lua_setfield(L, -2, "command");
		lua_pushinteger(L, list[i].ki_jid); lua_setfield(L, -2, "jail");
		lua_rawseti(L, -2, (lua_Integer)i + 1);
	}
	return 1;
#else
	return hb_error(L, "processes", NULL, ENOSYS);
#endif
}
int
hb_interface_up(lua_State *L)
{
	const char *name = hb_string(L, 1, NULL);
#ifdef __FreeBSD__
	luaL_argcheck(L, strlen(name) > 0 && strlen(name) < IFNAMSIZ, 1, "invalid interface name");
	struct ifreq request;
	memset(&request, 0, sizeof(request));
	memcpy(request.ifr_name, name, strlen(name) + 1);
	int fd = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0), error = 0;
	if (fd < 0) return hb_error(L, "interface_up", NULL, errno);
	if (ioctl(fd, SIOCGIFFLAGS, &request) < 0) error = errno;
	if (!error) {
		request.ifr_flags = (short)((unsigned short)request.ifr_flags | IFF_UP);
		if (ioctl(fd, SIOCSIFFLAGS, &request) < 0) error = errno;
	}
	close(fd);
	if (error) return hb_error(L, "interface_up", NULL, error);
	lua_pushboolean(L, 1); return 1;
#else
	(void)name;
	return hb_error(L, "interface_up", NULL, ENOSYS);
#endif
}
int
hb_default_routes(lua_State *L)
{
#ifdef __FreeBSD__
	int mib[] = {CTL_NET, PF_ROUTE, 0, AF_INET, NET_RT_DUMP, 0};
	size_t size = 0;
	if (sysctl(mib, 6, NULL, &size, NULL, 0) < 0) return hb_error(L, "default_routes", NULL, errno);
	if (size > 16 * 1024 * 1024) return hb_error(L, "default_routes", NULL, EOVERFLOW);
	size += 4096;
	size_t capacity = size;
	unsigned char *data = lua_newuserdatauv(L, size, 0);
	if (sysctl(mib, 6, data, &size, NULL, 0) < 0)
		return hb_error(L, "default_routes", NULL, errno == ENOMEM ? EAGAIN : errno);
	if (size > capacity) return hb_error(L, "default_routes", NULL, EOVERFLOW);
	lua_newtable(L); lua_Integer count = 0;
	for (size_t offset = 0; offset < size;) {
		struct rt_msghdr header;
		if (size - offset < sizeof(header)) return hb_error(L, "default_routes", NULL, EPROTO);
		memcpy(&header, data + offset, sizeof(header));
		if (header.rtm_msglen < sizeof(header) || header.rtm_msglen > size - offset || header.rtm_version != RTM_VERSION)
			return hb_error(L, "default_routes", NULL, EPROTO);
		struct sockaddr_in dst, mask, gateway;
		memset(&dst, 0, sizeof(dst)); memset(&mask, 0, sizeof(mask)); memset(&gateway, 0, sizeof(gateway));
		size_t position = sizeof(header);
		for (int slot = 0; slot < RTAX_MAX; slot++) {
			if (!(header.rtm_addrs & (1 << slot))) continue;
			if (header.rtm_msglen - position < 2) return hb_error(L, "default_routes", NULL, EPROTO);
			size_t length = data[offset + position];
			size_t advance = length ? (length + sizeof(long) - 1) & ~(sizeof(long) - 1) : sizeof(long);
			if (length == 1 || advance > header.rtm_msglen - position) return hb_error(L, "default_routes", NULL, EPROTO);
			struct sockaddr_in *out = slot == RTAX_DST ? &dst : slot == RTAX_NETMASK ? &mask : slot == RTAX_GATEWAY ? &gateway : NULL;
			if (out) memcpy(out, data + offset + position, length < sizeof(*out) ? length : sizeof(*out));
			position += advance;
		}
		if (dst.sin_family == AF_INET && dst.sin_addr.s_addr == INADDR_ANY && mask.sin_addr.s_addr == INADDR_ANY &&
		    gateway.sin_family == AF_INET && (header.rtm_flags & RTF_UP) && !(header.rtm_flags & RTF_HOST)) {
			char name[IFNAMSIZ], host[INET_ADDRSTRLEN];
			if (!if_indextoname(header.rtm_index, name) || !inet_ntop(AF_INET, &gateway.sin_addr, host, sizeof(host)))
				return hb_error(L, "default_routes", NULL, errno);
			lua_createtable(L, 0, 2);
			lua_pushstring(L, name); lua_setfield(L, -2, "interface");
			lua_pushstring(L, host); lua_setfield(L, -2, "gateway");
			lua_rawseti(L, -2, ++count);
		}
		offset += header.rtm_msglen;
	}
	return 1;
#else
	return hb_error(L, "default_routes", NULL, ENOSYS);
#endif
}

int
hb_sync(lua_State *L)
{
	sync(); lua_pushboolean(L, 1); return 1;
}
int
hb_swap_on(lua_State *L)
{
	const char *path = hb_string(L, 1, NULL);
#ifdef __FreeBSD__
	if (swapon(path) < 0) return hb_error(L, "swapon", NULL, errno);
	lua_pushboolean(L, 1); return 1;
#else
	(void)path; return hb_error(L, "swapon", NULL, ENOSYS);
#endif
}
int
hb_swap_off(lua_State *L)
{
	const char *path = hb_string(L, 1, NULL);
#ifdef __FreeBSD__
	if (swapoff(path, 0) < 0) return hb_error(L, "swapoff", NULL, errno);
	lua_pushboolean(L, 1); return 1;
#else
	(void)path; return hb_error(L, "swapoff", NULL, ENOSYS);
#endif
}
