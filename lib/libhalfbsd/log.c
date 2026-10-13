/* SPDX-License-Identifier: BSD-2-Clause */
#include <stdio.h>
#include <sys/stat.h>
#include <locale.h>
#include "internal.h"
#define HB_LOG "halfbsd.logger"
struct logger { int fd, console, closed;
	locale_t numeric;
};
static int close_logger(lua_State*L) {
	struct logger*g=luaL_checkudata(L,1,HB_LOG);
	if(g->fd>=0)close(g->fd);
	g->fd=-1;
	g->closed=1;
	if(g->numeric)freelocale(g->numeric);
	g->numeric=NULL;
	return 0;
}

static void quote(luaL_Buffer*b,const char*s,size_t n) {
	static const char hex[]="0123456789abcdef";
	luaL_addchar(b,'"');
	for(size_t i=0;i<n;i++) {
		unsigned char c=(unsigned char)s[i];
		if(c=='"'||c=='\\'){luaL_addchar(b,'\\');
			luaL_addchar(b,(char)c);
		}
		else if(c<32||c>=127){char escaped[]={'\\','u','0','0',hex[c>>4],hex[c&15]};
			luaL_addlstring(b,escaped,6);
		}
		else luaL_addchar(b,(char)c);
	}
	luaL_addchar(b,'"');
}

static int new_logger(lua_State*L) {
	static const char*const options[]={"console","file","service",NULL};
	hb_options(L,1,options);
	if(lua_isnoneornil(L,1)){lua_settop(L,0);
		lua_newtable(L);
	}
	lua_getfield(L,1,"console");
	int console=lua_isnil(L,-1)?1:hb_boolean(L,-1);
	lua_pop(L,1);
	lua_getfield(L,1,"service");
	if(!lua_isnil(L,-1)){size_t n;
		(void)hb_string(L,-1,&n);
		luaL_argcheck(L,n<=1024,1,"service name too long");
	}
	else{lua_pop(L,1);
		lua_pushliteral(L,"halfbsd");
	}
	int service=lua_gettop(L);
	lua_getfield(L,1,"file");
	const char*path=lua_isnil(L,-1)?NULL:hb_string(L,-1,NULL);
	struct logger*g=lua_newuserdatauv(L,sizeof(*g),1);
	g->fd=-1;
	g->numeric=NULL;
	g->console=console;
	g->closed=0;
	luaL_setmetatable(L,HB_LOG);
	lua_pushvalue(L,service);
	lua_setiuservalue(L,-2,1);
	g->numeric=newlocale(LC_NUMERIC_MASK,"C",NULL);
	if(!g->numeric)return hb_error(L,"log.new","locale",errno);
	if(path){g->fd=open(path,O_WRONLY|O_APPEND|O_CREAT|O_CLOEXEC|O_NOFOLLOW|O_NONBLOCK,0600);
		if(g->fd>=0&&g->fd<3){int copy=fcntl(g->fd,F_DUPFD_CLOEXEC,3);int saved=errno;close(g->fd);g->fd=copy;errno=saved;}
		if(g->fd<0)return hb_error(L,"log.new","open",errno);
		struct stat metadata;
		int stat_result=fstat(g->fd,&metadata);
		if(stat_result<0 || !S_ISREG(metadata.st_mode)) {
			int e=stat_result<0?errno:EINVAL;
			close(g->fd);g->fd=-1;return hb_error(L,"log.new","file_type",e);
		}
	}
	return 1;
}

struct log_field {
	const char *key, *value;
	size_t key_length, value_length;
	int quoted;
	char scalar[64];
};

static int
write_logger(lua_State *L)
{
	struct logger *g = luaL_checkudata(L, 1, HB_LOG);
	luaL_argcheck(L, !g->closed, 1, "closed logger");
	const char *level = hb_string(L, 2, NULL);
	static const char *const levels[] = {
	    "debug", "info", "notice", "warning", "error", "critical", NULL};
	int found = 0;
	for (size_t i = 0; levels[i]; i++)
		found |= !strcmp(level, levels[i]);
	luaL_argcheck(L, found, 2, "unknown severity");
	size_t message_length;
	luaL_checktype(L, 3, LUA_TSTRING);
	const char *message = lua_tolstring(L, 3, &message_length);
	luaL_argcheck(L, message_length <= 65536, 3, "message exceeds 64 KiB");
	int has_fields = !lua_isnoneornil(L, 4);
	if (has_fields)
		luaL_checktype(L, 4, LUA_TTABLE);
	/* Reserve the optional argument before a buffer pushes its placeholder. */
	lua_settop(L, 4);
	lua_getiuservalue(L, 1, 1);
	size_t service_length;
	const char *service = lua_tolstring(L, -1, &service_length);
	/* Snapshot and pin all strings before any buffer allocation can run GC.
	 * Once buffering starts, its box must remain at the top of the stack. */
	lua_newtable(L);
	int snapshot = lua_gettop(L);
	struct log_field fields[64];
	size_t count = 0;
	if (has_fields) {
		lua_pushnil(L);
		while (lua_next(L, 4)) {
			if (count == 64)
				luaL_argerror(L, 4, "too many structured fields");
			struct log_field *field = &fields[count];
			field->key = hb_string(L, -2, &field->key_length);
			if (field->key_length > 1024)
				luaL_argerror(L, 4, "field name too long");
			field->quoted = lua_type(L, -1) == LUA_TSTRING;
			if (field->quoted) {
				field->value = lua_tolstring(L, -1, &field->value_length);
				if (field->value_length > 65536)
					luaL_argerror(L, 4, "field value too long");
			} else if (lua_isboolean(L, -1)) {
				field->value = lua_toboolean(L, -1) ? "true" : "false";
				field->value_length = strlen(field->value);
			} else if (lua_type(L, -1) == LUA_TNUMBER) {
				double value = lua_tonumber(L, -1);
				if (!isfinite(value))
					luaL_argerror(L, 4, "nonfinite number");
				locale_t old = uselocale(g->numeric);
				if (!old)
					return hb_error(L, "log.write", "locale", errno);
				if (lua_isinteger(L, -1))
					snprintf(field->scalar, sizeof(field->scalar), "%jd",
					    (intmax_t)lua_tointeger(L, -1));
				else
					snprintf(field->scalar, sizeof(field->scalar), "%.17g", value);
				(void)uselocale(old);
				field->value = field->scalar;
				field->value_length = strlen(field->value);
			} else
				luaL_argerror(L, 4, "fields must contain strings, numbers or booleans");
			lua_pushvalue(L, -2);
			lua_rawseti(L, snapshot, (lua_Integer)(count * 2 + 1));
			lua_pushvalue(L, -1);
			lua_rawseti(L, snapshot, (lua_Integer)(count * 2 + 2));
			count++;
			lua_pop(L, 1);
		}
	}
	struct timespec now;
	if (clock_gettime(CLOCK_REALTIME, &now) < 0)
		return hb_error(L, "log.write", "clock", errno);
	luaL_Buffer buffer;
	luaL_buffinit(L, &buffer);
	char stamp[64];
	snprintf(stamp, sizeof(stamp), "{\"time\":%jd.%09ld,\"level\":",
	    (intmax_t)now.tv_sec, now.tv_nsec);
	luaL_addstring(&buffer, stamp);
	quote(&buffer, level, strlen(level));
	luaL_addstring(&buffer, ",\"service\":");
	quote(&buffer, service, service_length);
	luaL_addstring(&buffer, ",\"message\":");
	quote(&buffer, message, message_length);
	if (has_fields) {
		luaL_addstring(&buffer, ",\"fields\":{");
		for (size_t i = 0; i < count; i++) {
			if (i)
				luaL_addchar(&buffer, ',');
			quote(&buffer, fields[i].key, fields[i].key_length);
			luaL_addchar(&buffer, ':');
			if (fields[i].quoted)
				quote(&buffer, fields[i].value, fields[i].value_length);
			else
				luaL_addlstring(&buffer, fields[i].value, fields[i].value_length);
		}
		luaL_addchar(&buffer, '}');
	}
	luaL_addstring(&buffer, "}\n");
	luaL_pushresult(&buffer);
	size_t length;
	const char *line = lua_tolstring(L, -1, &length);
	int outputs[] = {g->console ? STDERR_FILENO : -1, g->fd};
	int error = 0;
	for (size_t i = 0; i < 2; i++) {
		if (outputs[i] < 0)
			continue;
		for (size_t offset = 0; offset < length;) {
			ssize_t result = hp_write(outputs[i], line + offset, length - offset);
			if (result <= 0) {
				if (!error)
					error = result < 0 ? errno : EIO;
				break;
			}
			offset += (size_t)result;
		}
	}
	if (error)
		return hb_error(L, "log.write", NULL, error);
	lua_pushboolean(L, 1);
	return 1;
}

int luaopen_halfbsd_log(lua_State*L) {
	static const luaL_Reg methods[]={{"__gc",close_logger},{"__close",close_logger},{"close",close_logger},{"write",write_logger},{NULL,NULL}};
	if(luaL_newmetatable(L,HB_LOG)){luaL_setfuncs(L,methods,0);
		lua_pushvalue(L,-1);
		lua_setfield(L,-2,"__index");
		lua_pushliteral(L,HB_LOG);
		lua_setfield(L,-2,"__metatable");
	}
	lua_pop(L,1);
	lua_newtable(L);
	lua_pushcfunction(L,new_logger);
	lua_setfield(L,-2,"new");
	return 1;
}
