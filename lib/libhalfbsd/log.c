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

static int write_logger(lua_State*L) {
	struct logger*g=luaL_checkudata(L,1,HB_LOG);
	luaL_argcheck(L,!g->closed,1,"closed logger");
	const char*level=hb_string(L,2,NULL);
	static const char*const levels[]={"debug","info","notice","warning","error","critical",NULL};
	int found=0;
	for(size_t i=0;levels[i];i++)found|=!strcmp(level,levels[i]);
	luaL_argcheck(L,found,2,"unknown severity");
	size_t n;
	luaL_checktype(L,3,LUA_TSTRING);
	const char*message=lua_tolstring(L,3,&n);
	luaL_argcheck(L,n<=65536,3,"message exceeds 64 KiB");
	if(!lua_isnoneornil(L,4))luaL_checktype(L,4,LUA_TTABLE);
	struct timespec now;
	if(clock_gettime(CLOCK_REALTIME,&now)<0)return hb_error(L,"log.write","clock",errno);
	luaL_Buffer b;
	luaL_buffinit(L,&b);
	char stamp[64];
	snprintf(stamp,sizeof(stamp),"{\"time\":%jd.%09ld,\"level\":",(intmax_t)now.tv_sec,now.tv_nsec);
	luaL_addstring(&b,stamp);
	quote(&b,level,strlen(level));
	luaL_addstring(&b,",\"service\":");
	lua_getiuservalue(L,1,1);
	size_t length;
	const char*s=lua_tolstring(L,-1,&length);
	quote(&b,s,length);
	lua_pop(L,1);
	luaL_addstring(&b,",\"message\":");
	quote(&b,message,n);
	if(!lua_isnoneornil(L,4)) {
		luaL_addstring(&b,",\"fields\":{");
		size_t count=0;
		lua_pushnil(L);
		while(lua_next(L,4)) {
			if(++count>64)luaL_argerror(L,4,"too many structured fields");
			size_t klen;
			const char*k=hb_string(L,-2,&klen);
			if(klen>1024)luaL_argerror(L,4,"field name too long");
			if(count>1)luaL_addchar(&b,',');
			quote(&b,k,klen);
			luaL_addchar(&b,':');
			if(lua_type(L,-1)==LUA_TSTRING){size_t vlen;
				const char*v=lua_tolstring(L,-1,&vlen);
				if(vlen>65536)luaL_argerror(L,4,"field value too long");
				quote(&b,v,vlen);
			}
			else if(lua_isboolean(L,-1))luaL_addstring(&b,lua_toboolean(L,-1)?"true":"false");
			else if(lua_type(L,-1)==LUA_TNUMBER){double v=lua_tonumber(L,-1);
				if(!isfinite(v))luaL_argerror(L,4,"nonfinite number");
				char number[64];
				locale_t old=uselocale(g->numeric);
				if(!old)return hb_error(L,"log.write","locale",errno);
				if(lua_isinteger(L,-1))snprintf(number,sizeof(number),"%jd",(intmax_t)lua_tointeger(L,-1));
				else snprintf(number,sizeof(number),"%.17g",v);
				(void)uselocale(old);
				luaL_addstring(&b,number);
			}
			else luaL_argerror(L,4,"fields must contain strings, numbers or booleans");
			lua_pop(L,1);
		}
		luaL_addchar(&b,'}');
	}
	luaL_addstring(&b,"}\n");
	luaL_pushresult(&b);
	const char*line=lua_tolstring(L,-1,&length);
	int outputs[]={g->console?STDERR_FILENO:-1,g->fd};
	int error=0;
	for(size_t i=0;i<2;i++) {
		if(outputs[i]<0)
			continue;
		for(size_t offset=0;offset<length;) {
			ssize_t result=hp_write(outputs[i],line+offset,length-offset);
			if(result<=0) {
				if(!error)
					error=result<0?errno:EIO;
				break;
			}
			offset+=(size_t)result;
		}
	}
	if(error)return hb_error(L,"log.write",NULL,error);
	lua_pushboolean(L,1);
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
