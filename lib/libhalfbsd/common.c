/* SPDX-License-Identifier: BSD-2-Clause */
#include <sys/wait.h>
#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include "internal.h"
const char *
hb_string(lua_State *L, int index, size_t *length)
{
	size_t size;
	luaL_checktype(L, index, LUA_TSTRING);
	const char *value = lua_tolstring(L, index, &size);
	if (memchr(value, 0, size)) luaL_argerror(L, index, "embedded NUL is not allowed");
	if (length) *length = size;
	return value;
}

int
hb_boolean(lua_State *L, int index)
{
	luaL_checktype(L, index, LUA_TBOOLEAN);
	return lua_toboolean(L, index);
}

void
hb_options(lua_State *L, int index, const char *const *names)
{
	if (lua_isnoneornil(L, index)) return;
	luaL_checktype(L, index, LUA_TTABLE);
	index = lua_absindex(L, index);
	lua_pushnil(L);
	while (lua_next(L, index)) {
		const char *key = hb_string(L, -2, NULL);
		int found = 0;
		for (size_t i = 0; names[i]; i++) if (!strcmp(key, names[i])) { found = 1;
			break;
		}
		if (!found) luaL_error(L, "unknown option '%s'", key);
		lua_pop(L, 1);
	}
}

int
hb_timeout(lua_State *L, int index, int fallback)
{
	if (lua_isnoneornil(L, index)) return fallback;
	luaL_checktype(L, index, LUA_TNUMBER);
	double value = lua_tonumber(L, index);
	if (!isfinite(value) || value < 0 || value > INT_MAX/1000.0)
	luaL_argerror(L, index, "timeout must be finite, nonnegative seconds");
	return (int)ceil(value * 1000.0);
}

static const char *
error_name(int code)
{
#define E(name) if (code == name) return #name
	E(E2BIG); E(ECANCELED); E(ENOEXEC); E(ELOOP); E(ERANGE);
	E(ENOENT);
	E(EACCES);
	E(EPERM);
	E(EAGAIN);
	E(ENOMEM);
	E(EMFILE);
	E(ENFILE);
	E(EINVAL);
	E(EBUSY);
	E(ESRCH);
	E(ECHILD);
	E(ETIMEDOUT);
	E(EBADF);
	E(EPIPE);
	E(EINTR);
	E(EIO);
	E(EEXIST);
	E(ENOTDIR);
	E(ENOTEMPTY);
	E(ENOSYS);
	E(ENOTSUP);
	E(EROFS);
	E(EXDEV);
	E(EOVERFLOW);
	E(EPROTO);
	E(EFBIG);
	E(ESHUTDOWN);
#undef E
	return "UNKNOWN";
}

int
hb_error(lua_State *L, const char *operation, const char *stage, int code)
{
	lua_pushnil(L);
	lua_createtable(L, 0, 6);
	const char *kind = code == ETIMEDOUT ? "timeout" :
	code == ENOENT ? "not_found" : (code == EACCES || code == EPERM) ? "permission" :
	(code == ENOMEM || code == EMFILE || code == ENFILE || code == EAGAIN) ? "resource" : "system";
	lua_pushstring(L, kind);
	lua_setfield(L, -2, "kind");
	lua_pushinteger(L, code);
	lua_setfield(L, -2, "code");
	lua_pushstring(L, error_name(code));
	lua_setfield(L, -2, "name");
	lua_pushstring(L, strerror(code));
	lua_setfield(L, -2, "message");
	lua_pushstring(L, operation);
	lua_setfield(L, -2, "operation");
	if (stage) { lua_pushstring(L, stage);
		lua_setfield(L, -2, "stage");
	}
	return 2;
}

void
hb_result(lua_State *L, const struct hp_result *result)
{
	lua_createtable(L, 0, 10);
	lua_pushinteger(L, result->pid);
	lua_setfield(L, -2, "pid");
	lua_pushboolean(L, result->own_group);
	lua_setfield(L, -2, "own_group");
	lua_pushnumber(L, result->started.tv_sec + result->started.tv_nsec/1e9);
	lua_setfield(L, -2, "started");
	if (!result->done) { lua_pushliteral(L, "running");
		lua_setfield(L, -2, "state");
		return;
	}
	lua_pushnumber(L, result->finished.tv_sec + result->finished.tv_nsec/1e9);
	lua_setfield(L, -2, "finished");
	if (result->error) {
		lua_pushliteral(L, "lost");
		lua_setfield(L, -2, "state");
		lua_pushinteger(L, result->error);
		lua_setfield(L, -2, "code");
	} else if (WIFEXITED(result->status)) {
		lua_pushliteral(L, "exited");
		lua_setfield(L, -2, "state");
		lua_pushinteger(L, WEXITSTATUS(result->status));
		lua_setfield(L, -2, "exit_code");
		lua_pushboolean(L, WEXITSTATUS(result->status) == 0);
		lua_setfield(L, -2, "success");
	} else if (WIFSIGNALED(result->status)) {
		lua_pushliteral(L, "signaled");
		lua_setfield(L, -2, "state");
		lua_pushinteger(L, WTERMSIG(result->status));
		lua_setfield(L, -2, "signal");
		lua_pushboolean(L, WCOREDUMP(result->status));
		lua_setfield(L, -2, "core_dumped");
		lua_pushboolean(L, 0);
		lua_setfield(L, -2, "success");
	} else { lua_pushliteral(L, "lost");
		lua_setfield(L, -2, "state");
	}
}

int
hb_signal_number(lua_State *L, int index)
{
	if (lua_isinteger(L, index)) {
		lua_Integer n = lua_tointeger(L, index);
		luaL_argcheck(L, n >= 0 && n < NSIG, index, "invalid signal number");
		return (int)n;
	}
	const char *name = hb_string(L, index, NULL);
	if (!strncmp(name, "SIG", 3)) name += 3;
#define S(sig) if (!strcmp(name, #sig)) return SIG##sig
	S(HUP);
	S(INT);
	S(QUIT);
	S(ILL);
	S(TRAP);
	S(ABRT);
	S(FPE);
	S(KILL);
	S(BUS);
	S(SEGV);
	S(SYS);
	S(PIPE);
	S(ALRM);
	S(TERM);
	S(URG);
	S(STOP);
	S(TSTP);
	S(CONT);
	S(CHLD);
	S(TTIN);
	S(TTOU);
	S(IO);
	S(XCPU);
	S(XFSZ);
	S(VTALRM);
	S(PROF);
	S(WINCH);
	S(USR1);
	S(USR2);
#ifdef SIGINFO
	S(INFO);
#endif
#ifdef SIGEMT
	S(EMT);
#endif
#undef S
	return luaL_argerror(L, index, "unknown signal name");
}

static pthread_mutex_t signal_lock = PTHREAD_MUTEX_INITIALIZER;
static int watched_signals[NSIG];
int
hb_signal_disposition(int signal, int watching, struct sigaction *old)
{
	struct sigaction action;
	int result, e;
	if (signal <= 0 || signal >= NSIG || signal == SIGKILL || signal == SIGSTOP || signal == SIGCHLD) { errno = EINVAL;
		return -1;
	}
	pthread_mutex_lock(&signal_lock);
	if (watched_signals[signal]) { pthread_mutex_unlock(&signal_lock);
		errno = EBUSY;
		return -1;
	}
	memset(&action, 0, sizeof(action));
	sigemptyset(&action.sa_mask);
	action.sa_handler = watching == 2 ? SIG_DFL : SIG_IGN;
	result = sigaction(signal, &action, old);
	e = errno;
	if (result == 0 && watching == 1) watched_signals[signal] = 1;
	pthread_mutex_unlock(&signal_lock);
	errno = e;
	return result;
}

void
hb_signal_restore(int signal, const struct sigaction *old)
{
	pthread_mutex_lock(&signal_lock);
	(void)sigaction(signal, old, NULL);
	watched_signals[signal] = 0;
	pthread_mutex_unlock(&signal_lock);
}

struct hp_fd *
hb_check_fd(lua_State *L, int index)
{
	struct hb_fd *fd = luaL_checkudata(L, index, HB_FD);
	luaL_argcheck(L, fd->fd != NULL, index, "released descriptor");
	return fd->fd;
}

void
hb_push_fd(lua_State *L, struct hp_fd *fd)
{
	struct hb_fd *handle = lua_newuserdatauv(L, sizeof(*handle), 0);
	handle->fd = fd;
	luaL_setmetatable(L, HB_FD);
}

static int fd_gc(lua_State *L) {
	struct hb_fd *fd = luaL_checkudata(L, 1, HB_FD);
	hp_fd_unref(fd->fd);
	fd->fd = NULL;
	return 0;
}

static int fd_close(lua_State *L) {
	if (hp_fd_close(hb_check_fd(L, 1)) < 0) return hb_error(L, "close", NULL, errno);
	lua_pushboolean(L, 1);
	return 1;
}

static int fd_number(lua_State *L) {
	int fd = hp_fd_number(hb_check_fd(L, 1));
	if (fd < 0) return hb_error(L, "fileno", NULL, errno);
	lua_pushinteger(L, fd);
	return 1;
}

static int fd_nonblock(lua_State *L) {
	struct hp_fd *fd = hb_check_fd(L, 1);
	int enabled = hb_boolean(L, 2);
	if (hp_fd_nonblock(fd, enabled) < 0) return hb_error(L, "nonblock", NULL, errno);
	lua_pushboolean(L, 1);
	return 1;
}

static int fd_read(lua_State *L) {
	struct hp_fd *source = hb_check_fd(L, 1);
	lua_Integer count = luaL_optinteger(L, 2, 65536);
	luaL_argcheck(L, count > 0 && count <= 1024*1024, 2, "read size must be 1..1048576");
	/* Lua owns allocation even if a later push raises an allocation error. */
	char *data = lua_newuserdatauv(L, (size_t)count, 0);
	int fd = hp_fd_dup(source), e;
	ssize_t n;
	if (fd < 0) return hb_error(L, "read", NULL, errno);
	do { n = read(fd, data, (size_t)count);
	} while (n < 0 && errno == EINTR);
	e = errno;
	close(fd);
	if (n < 0) return hb_error(L, "read", NULL, e);
	if (n == 0) lua_pushnil(L);
	else lua_pushlstring(L, data, (size_t)n);
	return 1;
}

static int fd_write(lua_State *L) {
	struct hp_fd *target = hb_check_fd(L, 1);
	size_t length;
	luaL_checktype(L, 2, LUA_TSTRING);
	const char *data = lua_tolstring(L, 2, &length);
	luaL_argcheck(L, length <= HB_MAX_IO, 2, "write exceeds 64 MiB");
	int fd = hp_fd_dup(target), e;
	if (fd < 0) return hb_error(L, "write", NULL, errno);
	ssize_t n = hp_write(fd, data, length);
	e = errno;
	close(fd);
	if (n < 0) return hb_error(L, "write", NULL, e);
	lua_pushinteger(L, n);
	return 1;
}

void
hb_fd_metatable(lua_State *L)
{
	static const luaL_Reg methods[] = {
		{"__gc", fd_gc}, {"__close", fd_close}, {"close", fd_close},
		{"fileno", fd_number}, {"nonblock", fd_nonblock},
		{"read", fd_read}, {"write", fd_write}, {NULL, NULL}};
	if (luaL_newmetatable(L, HB_FD)) {
		luaL_setfuncs(L, methods, 0);
		lua_pushvalue(L, -1);
		lua_setfield(L, -2, "__index");
		lua_pushliteral(L, HB_FD);
		lua_setfield(L, -2, "__metatable");
	}
	lua_pop(L, 1);
}
