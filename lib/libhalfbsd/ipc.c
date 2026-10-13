/* SPDX-License-Identifier: BSD-2-Clause */
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <poll.h>
#include <stddef.h>
#include <limits.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "internal.h"

static struct hb_fd *owner(lua_State *L)
{
	struct hb_fd *f = lua_newuserdatauv(L, sizeof(*f), 0);
	f->fd = NULL;
	luaL_setmetatable(L, HB_FD);
	return f;
}
static int address(lua_State *L, int index, struct sockaddr_un *sun)
{
	size_t length;
	const char *path = hb_string(L, index, &length);
	luaL_argcheck(L, length > 0 && length < sizeof(sun->sun_path), index,
	    "Unix socket path is empty or too long");
	memset(sun, 0, sizeof(*sun));
	sun->sun_family = AF_UNIX;
	memcpy(sun->sun_path, path, length + 1);
#ifdef __FreeBSD__
	sun->sun_len = sizeof(*sun);
#endif
	return 0;
}
static int listen_unix(lua_State *L)
{
	struct sockaddr_un sun;
	address(L, 1, &sun);
	struct hb_fd *f = owner(L);
	int fd = socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC | SOCK_NONBLOCK, 0);
	if (fd < 0) return hb_error(L, "ipc.listen", NULL, errno);
	/* No automatic unlink: the caller must own a lock and verify stale paths. */
	int error = 0;
	if (bind(fd, (struct sockaddr *)&sun, sizeof(sun)) < 0) error = errno;
	if (!error && listen(fd, 32) < 0) {
		error = errno;
		unlink(sun.sun_path);
	}
	if (error) { close(fd); return hb_error(L, "ipc.listen", NULL, error); }
	f->fd = hp_fd_new(fd);
	if (!f->fd) {
		error = errno;
		unlink(sun.sun_path);
		return hb_error(L, "ipc.listen", NULL, error);
	}
	return 1;
}
static int accept_unix(lua_State *L)
{
	struct hp_fd *listener = hb_check_fd(L, 1);
	struct hb_fd *f = owner(L);
	int fd = accept4(hp_fd_number(listener), NULL, NULL, SOCK_CLOEXEC | SOCK_NONBLOCK);
	if (fd < 0) return hb_error(L, "ipc.accept", NULL, errno);
	uid_t uid; gid_t gid;
	if (getpeereid(fd, &uid, &gid) < 0) {
		int error = errno; close(fd); return hb_error(L, "ipc.accept", "credentials", error);
	}
	f->fd = hp_fd_new(fd);
	if (!f->fd) return hb_error(L, "ipc.accept", NULL, errno);
	lua_createtable(L, 0, 2);
	lua_pushinteger(L, uid); lua_setfield(L, -2, "uid");
	lua_pushinteger(L, gid); lua_setfield(L, -2, "gid");
	return 2;
}
static int connect_unix(lua_State *L)
{
	struct sockaddr_un sun;
	address(L, 1, &sun);
	int timeout = hb_timeout(L, 2, 3000);
	struct hb_fd *f = owner(L);
	int fd = socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC | SOCK_NONBLOCK, 0);
	if (fd < 0) return hb_error(L, "ipc.connect", NULL, errno);
	int result = connect(fd, (struct sockaddr *)&sun, sizeof(sun)), error = 0;
	if (result < 0 && errno != EINPROGRESS) error = errno;
	if (result < 0 && !error) {
		struct pollfd p = {fd, POLLOUT, 0};
		struct timespec start, now;
		if (clock_gettime(CLOCK_MONOTONIC, &start) < 0) {
			error = errno; close(fd);
			return hb_error(L, "ipc.connect", "clock", error);
		}
		int remaining = timeout;
		for (;;) {
			result = poll(&p, 1, remaining);
			if (result >= 0 || errno != EINTR) break;
			if (clock_gettime(CLOCK_MONOTONIC, &now) < 0) {
				result = -1;
				break;
			}
			int64_t elapsed = (now.tv_sec - start.tv_sec) * 1000 +
			    (now.tv_nsec - start.tv_nsec) / 1000000;
			remaining = elapsed >= timeout ? 0 : timeout - (int)elapsed;
		}
		if (result <= 0) error = result == 0 ? ETIMEDOUT : errno;
		else {
			socklen_t length = sizeof(error);
			if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &length) < 0) error = errno;
		}
	}
	if (error) { close(fd); return hb_error(L, "ipc.connect", NULL, error); }
	f->fd = hp_fd_new(fd);
	if (!f->fd) return hb_error(L, "ipc.connect", NULL, errno);
	return 1;
}
static int lock_file(lua_State *L)
{
	const char *path = hb_string(L, 1, NULL);
	struct hb_fd *f = owner(L);
	int fd = open(path, O_RDWR | O_CREAT | O_CLOEXEC | O_NOFOLLOW | O_NONBLOCK, 0600);
	if (fd < 0) return hb_error(L, "ipc.lock", NULL, errno);
	struct stat st;
	int error = 0;
	if (fstat(fd, &st) < 0) error = errno;
	else if (!S_ISREG(st.st_mode) || st.st_uid != geteuid() || (st.st_mode & 077) != 0)
		error = EACCES;
	if (!error && flock(fd, LOCK_EX | LOCK_NB) < 0) error = errno;
	if (error) { close(fd); return hb_error(L, "ipc.lock", NULL, error); }
	f->fd = hp_fd_new(fd);
	if (!f->fd) return hb_error(L, "ipc.lock", NULL, errno);
	return 1;
}
static int socket_probe(lua_State *L)
{
	struct sockaddr_un sun;
	address(L, 1, &sun);
	int fd = socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC | SOCK_NONBLOCK, 0);
	if (fd < 0) return hb_error(L, "ipc.probe", NULL, errno);
	int result = connect(fd, (struct sockaddr *)&sun, sizeof(sun));
	int error = errno;
	close(fd);
	if (result < 0 && error != ECONNREFUSED && error != ENOENT && error != EINPROGRESS)
		return hb_error(L, "ipc.probe", NULL, error);
	lua_pushboolean(L, result == 0 || error == EINPROGRESS);
	return 1;
}
static void ipv4_address(lua_State *L, struct sockaddr_in *sin)
{
	const char *host = hb_string(L, 1, NULL);
	lua_Integer port = luaL_checkinteger(L, 2);
	luaL_argcheck(L, port > 0 && port <= 65535, 2, "port must be 1..65535");
	memset(sin, 0, sizeof(*sin));
	sin->sin_family = AF_INET;
#ifdef __FreeBSD__
	sin->sin_len = sizeof(*sin);
#endif
	sin->sin_port = htons((uint16_t)port);
	luaL_argcheck(L, inet_pton(AF_INET, host, &sin->sin_addr) == 1, 1, "numeric IPv4 address required");
}
static int udp_connection(lua_State *L)
{
	struct sockaddr_in sin;
	ipv4_address(L, &sin);
	struct hb_fd *f = owner(L);
	int fd = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC | SOCK_NONBLOCK, 0);
	if (fd < 0) return hb_error(L, "ipc.udp", NULL, errno);
	if (connect(fd, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
		int error = errno; close(fd); return hb_error(L, "ipc.udp", NULL, error);
	}
	f->fd = hp_fd_new(fd);
	if (!f->fd) return hb_error(L, "ipc.udp", NULL, errno);
	return 1;
}
static int tcp_probe(lua_State *L)
{
	struct sockaddr_in sin;
	ipv4_address(L, &sin);
	int fd = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC | SOCK_NONBLOCK, 0);
	if (fd < 0) return hb_error(L, "ipc.tcp_probe", NULL, errno);
	int result = connect(fd, (struct sockaddr *)&sin, sizeof(sin)), error = 0;
	if (result < 0) error = errno;
	if (error == EINPROGRESS) {
		struct pollfd p = {fd, POLLOUT, 0};
		result = poll(&p, 1, 0);
		if (result > 0) {
			socklen_t length = sizeof(error);
			if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &length) < 0) error = errno;
		} else if (result < 0) error = errno;
	}
	close(fd);
	if (error && error != EINPROGRESS && error != ECONNREFUSED && error != ETIMEDOUT &&
	    error != ENETUNREACH && error != EHOSTUNREACH && error != EINTR)
		return hb_error(L, "ipc.tcp_probe", NULL, error);
	lua_pushboolean(L, error == 0);
	return 1;
}
int luaopen_halfbsd_ipc(lua_State *L)
{
	static const luaL_Reg methods[] = {
	    {"listen", listen_unix}, {"accept", accept_unix}, {"connect", connect_unix},
	    {"lock", lock_file}, {"probe", socket_probe}, {"udp", udp_connection},
	    {"tcp_probe", tcp_probe}, {NULL, NULL}};
	hb_fd_metatable(L);
	luaL_newlib(L, methods);
	return 1;
}
