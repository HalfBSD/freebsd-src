/* SPDX-License-Identifier: BSD-2-Clause */
#include <sys/event.h>
#include <sys/stat.h>
#include <grp.h>
#include <pwd.h>
#include <limits.h>
#include "internal.h"
#define HB_MANAGER "halfbsd.process_context"
#define HB_LAUNCH "halfbsd.launch"
extern char **environ;
struct manager { struct hp_context *ctx;
};
#define HB_RUN "halfbsd.run_resources"
struct run_resources { int kq, pd, input, output[2];
};
static int run_gc(lua_State *L) {
	struct run_resources *r=luaL_checkudata(L,1,HB_RUN);
	int *fds[]={&r->kq,&r->pd,&r->input,&r->output[0],&r->output[1]};
	for(size_t i=0;i<5;i++){if(*fds[i]>=0)close(*fds[i]);
		*fds[i]=-1;
	}
	return 0;
}

struct launch {
	struct hp_spec spec;
	char **argv, **env;
	gid_t *groups;
	int inherit[HP_MAX_INHERIT], opened[3];
	struct hp_fd *parent[3], *child[3];
	struct hp_process *process;
};
static int
launch_gc(lua_State *L)
{
	struct launch *g = luaL_checkudata(L, 1, HB_LAUNCH);
	if (g->argv) { for (size_t i=0; g->argv[i]; i++) free(g->argv[i]);
		free(g->argv);
		g->argv=NULL;
	}
	if (g->env) { for (size_t i=0; g->env[i]; i++) free(g->env[i]);
		free(g->env);
		g->env=NULL;
	}
	free(g->groups);
	g->groups=NULL;
	for (int i=0; i<3; i++) {
		if (g->opened[i]>=0) close(g->opened[i]);
		g->opened[i]=-1;
		hp_fd_unref(g->parent[i]);
		g->parent[i]=NULL;
		hp_fd_unref(g->child[i]);
		g->child[i]=NULL;
	}
	hp_process_unref(g->process);
	g->process=NULL;
	return 0;
}

static int manager_gc(lua_State *L) {
	struct manager *m=luaL_checkudata(L,1,HB_MANAGER);
	if (m->ctx) hp_context_close(m->ctx);
	m->ctx=NULL;
	return 0;
}

static struct hp_context *context(lua_State *L) {
	struct manager *m=lua_touserdata(L,lua_upvalueindex(1));
	if (!m->ctx) luaL_error(L,"process context is closed");
	return m->ctx;
}

static struct hp_process *child(lua_State *L,int index) {
	struct hb_child *c=luaL_checkudata(L,index,HB_CHILD);
	luaL_argcheck(L,c->process!=NULL,index,"released process handle");
	return c->process;
}

static int child_gc(lua_State *L) {
	struct hb_child *c=luaL_checkudata(L,1,HB_CHILD);
	hp_process_unref(c->process);
	c->process=NULL;
	return 0;
}

static int status(lua_State *L) {
	struct hp_result r;
	hp_status(child(L,1),&r);
	hb_result(L,&r);
	return 1;
}

static int wait_child(lua_State *L) {
	struct hp_process *p=child(L,1);
	int ms=hb_timeout(L,2,-1);
	struct hp_result r;
	if (hp_wait(p,ms,&r)!=1) return hb_error(L,"wait",NULL,errno);
	hb_result(L,&r);
	return 1;
}

static int signal_child(lua_State *L) {
	struct hp_process *p=child(L,1);
	int sig=hb_signal_number(L,2);
	int group=lua_isnoneornil(L,3)?0:hb_boolean(L,3);
	int n=hp_signal(p,sig,group);
	if(n<0) return hb_error(L,"signal",NULL,errno);
	lua_pushboolean(L,n);
	return 1;
}

static int kill_child(lua_State *L) {
	struct hp_process *p=child(L,1);
	int group=lua_isnoneornil(L,2)?0:hb_boolean(L,2);
	int n=hp_signal(p,SIGKILL,group);
	if(n<0)return hb_error(L,"kill",NULL,errno);
	lua_pushboolean(L,n);
	return 1;
}
static int foreground_child(lua_State *L)
{
	if (hp_foreground(child(L, 1), STDIN_FILENO) < 0)
		return hb_error(L, "foreground", NULL, errno);
	lua_pushboolean(L, 1); return 1;
}

static int terminate_child(lua_State *L) {
	struct hp_process *p=child(L,1);
	int ms=hb_timeout(L,2,5000);
	int group=lua_isnoneornil(L,3)?0:hb_boolean(L,3);
	struct hp_result r;
	if(hp_signal(p,SIGTERM,group)<0)return hb_error(L,"terminate","TERM",errno);
	int n=hp_wait(p,ms,&r);
	if(n==0) {
		if(hp_signal(p,SIGKILL,group)<0)return hb_error(L,"terminate","KILL",errno);
		n=hp_wait(p,5000,&r);
	}
	if(n!=1)return hb_error(L,"terminate","wait",errno);
	hb_result(L,&r);
	return 1;
}

static int child_index(lua_State *L) {
	(void)child(L,1);
	const char *key=hb_string(L,2,NULL);
	if(!strcmp(key,"pid")) {struct hp_result r;
		hp_status(child(L,1),&r);
		lua_pushinteger(L,r.pid);
		return 1;
	}
	lua_getiuservalue(L,1,1);
	lua_getfield(L,-1,key);
	if(!lua_isnil(L,-1))return 1;
	lua_pop(L,2);
	luaL_getmetatable(L,HB_CHILD);
	lua_getfield(L,-1,key);
	return 1;
}

static void *allocate(lua_State *L,size_t n) {
	void *p=calloc(1,n);
	if(!p)luaL_error(L,"native allocation failed: ENOMEM");
	return p;
}

static char *copy_string(lua_State *L,int index) {
	size_t length;
	const char *s=hb_string(L,index,&length);
	long argument_limit=sysconf(_SC_ARG_MAX);
	if(argument_limit<1)argument_limit=262144;
	if(length>=(size_t)argument_limit)luaL_argerror(L,index,"argument exceeds ARG_MAX");
	char *copy=strdup(s);
	if(!copy)luaL_error(L,"native allocation failed: ENOMEM");
	return copy;
}

/* All arrays are dense, bounded and string typed;
Lua length alone cannot
* detect holes or non-array keys. */
static size_t dense(lua_State *L,int index,size_t max) {
	luaL_checktype(L,index,LUA_TTABLE);
	index=lua_absindex(L,index);
	size_t n=lua_rawlen(L,index),seen=0;
	luaL_argcheck(L,n<=max,index,"array too large");
	lua_pushnil(L);
	while(lua_next(L,index)) {
		if(!lua_isinteger(L,-2)||lua_tointeger(L,-2)<1||
		(lua_Unsigned)lua_tointeger(L,-2)>n)luaL_argerror(L,index,"expected dense argument array");
		seen++;
		lua_pop(L,1);
	}
	luaL_argcheck(L,seen==n,index,"array contains holes");
	return n;
}

static void environment(lua_State *L,struct launch *g,int opts) {
	lua_newtable(L);
	int map=lua_gettop(L);
	lua_getfield(L,opts,"inherit_env");
	int inherit=lua_isnil(L,-1)?1:hb_boolean(L,-1);
	lua_pop(L,1);
	if(inherit)for(char **p=environ;*p;p++) {
		const char *eq=strchr(*p,'=');
		if(!eq)continue;
		lua_pushlstring(L,*p,(size_t)(eq-*p));
		lua_pushstring(L,eq+1);
		lua_rawset(L,map);
	}
	lua_getfield(L,opts,"env");
	if(!lua_isnil(L,-1)) {
		luaL_checktype(L,-1,LUA_TTABLE);
		int env=lua_gettop(L);
		lua_pushnil(L);
		while(lua_next(L,env)) {
			const char *key=hb_string(L,-2,NULL);
			if(!*key||strchr(key,'='))luaL_error(L,"invalid environment variable name");
			if(lua_isboolean(L,-1)&&!lua_toboolean(L,-1)) {
				lua_pushvalue(L,-2);
				lua_pushnil(L);
				lua_rawset(L,map);
			} else {
				(void)hb_string(L,-1,NULL);
				lua_pushvalue(L,-2);
				lua_pushvalue(L,-2);
				lua_rawset(L,map);
			}
			lua_pop(L,1);
		}
	}
	lua_pop(L,1);
	size_t n=0;
	lua_pushnil(L);
	while(lua_next(L,map)){n++;
		lua_pop(L,1);
	}
	if(n>65536)luaL_error(L,"environment too large");
	size_t environment_bytes=(n+1)*sizeof(char*);
	long argument_limit=sysconf(_SC_ARG_MAX);
	if(argument_limit<1)argument_limit=262144;
	g->env=allocate(L,(n+1)*sizeof(char*));
	n=0;
	lua_pushnil(L);
	while(lua_next(L,map)) {
		size_t a,b;
		const char*k=lua_tolstring(L,-2,&a),*v=lua_tolstring(L,-1,&b);
		if(a+b+2>(size_t)argument_limit || environment_bytes>(size_t)argument_limit-(a+b+2))luaL_error(L,"environment exceeds ARG_MAX");
		environment_bytes+=a+b+2;
		g->env[n]=allocate(L,a+b+2);
		memcpy(g->env[n],k,a);
		g->env[n][a]='=';
		memcpy(g->env[n]+a+1,v,b+1);
		n++;
		lua_pop(L,1);
	}
	lua_pop(L,1);
	g->spec.env=g->env;
}

static int credentials(lua_State *L,struct launch *g,int opts) {
	g->spec.uid=getuid();
	g->spec.gid=getgid();
	char buffer[65536];
	struct passwd pw,*user=NULL;
	struct group gr,*group=NULL;
	lua_getfield(L,opts,"user");
	if(!lua_isnil(L,-1)) {
		int e;
		if(lua_isinteger(L,-1)) {
			lua_Integer id=lua_tointeger(L,-1);
			if(id<0||(uintmax_t)id>=UINT_MAX)luaL_error(L,"invalid uid");
			e=getpwuid_r((uid_t)id,&pw,buffer,sizeof(buffer),&user);
		} else e=getpwnam_r(hb_string(L,-1,NULL),&pw,buffer,sizeof(buffer),&user);
		if(e||!user){errno=e?e:ENOENT;return -1;}
		g->spec.uid=user->pw_uid;
		g->spec.gid=user->pw_gid;
		g->spec.credentials=1;
		int count=0;
		(void)getgrouplist(user->pw_name,user->pw_gid,NULL,&count);
		if(count<1||count>65536)luaL_error(L,"invalid supplementary group count");
		g->groups=allocate(L,(size_t)count*sizeof(gid_t));
		if(getgrouplist(user->pw_name,user->pw_gid,g->groups,&count)<0)luaL_error(L,"supplementary groups changed during lookup");
		g->spec.ngroups=count;
	}
	lua_pop(L,1);
	lua_getfield(L,opts,"group");
	if(!lua_isnil(L,-1)) {
		if(lua_isinteger(L,-1)) {
			lua_Integer id=lua_tointeger(L,-1);
			if(id<0||(uintmax_t)id>=UINT_MAX)luaL_error(L,"invalid gid");
			g->spec.gid=(gid_t)id;
		} else {
			int e=getgrnam_r(hb_string(L,-1,NULL),&gr,buffer,sizeof(buffer),&group);
			if(e||!group){errno=e?e:ENOENT;return -1;}
			g->spec.gid=group->gr_gid;
		}
		g->spec.credentials=1;
	}
	lua_pop(L,1);
	lua_getfield(L,opts,"groups");
	if(!lua_isnil(L,-1)) {
		size_t n=dense(L,-1,65536);
		free(g->groups);
		g->groups=NULL;
		g->groups=allocate(L,(n+1)*sizeof(gid_t));
		g->spec.ngroups=(int)n;
		for(size_t i=0;i<n;i++) {
			lua_rawgeti(L,-1,(lua_Integer)i+1);
			lua_Integer id=luaL_checkinteger(L,-1);
			if(id<0||(uintmax_t)id>=UINT_MAX)luaL_error(L,"invalid supplementary gid");
			g->groups[i]=(gid_t)id;
			lua_pop(L,1);
		}
		g->spec.credentials=1;
	}
	lua_pop(L,1);
	g->spec.groups=g->groups;
	return 0;
}

static int
redirect(lua_State *L,struct launch *g,int opts,int index)
{
	static const char *const names[]={"stdin","stdout","stderr"};
	static const char *const fileopts[]={"path","append","mode",NULL};
	lua_getfield(L,opts,names[index]);
	int fd=index;
	if(lua_isnil(L,-1)) { /* Inherit an originally closed descriptor as closed. */
		if(fcntl(index,F_GETFD)<0&&errno==EBADF)fd=-1;
	} else if(luaL_testudata(L,-1,HB_FD)) {
		g->parent[index]=hb_check_fd(L,-1);
		hp_fd_ref(g->parent[index]);
		fd=hp_fd_number(g->parent[index]);
		if(fd<0)return -1;
	} else if(lua_istable(L,-1)) {
		int table=lua_gettop(L);
		hb_options(L,table,fileopts);
		lua_getfield(L,table,"path");
		const char*path=hb_string(L,-1,NULL);
		lua_getfield(L,table,"append");
		int append=lua_isnil(L,-1)?0:hb_boolean(L,-1);
		lua_getfield(L,table,"mode");
		lua_Integer mode=luaL_optinteger(L,-1,0600);
		if(mode<0||mode>0777)luaL_error(L,"invalid redirection mode");
		fd=open(path,O_CLOEXEC|O_NOFOLLOW|O_NOCTTY|O_NONBLOCK|(index==0?O_RDONLY:O_WRONLY|O_CREAT|(append?O_APPEND:O_TRUNC)),(mode_t)mode);
		lua_pop(L,3);
		g->opened[index]=fd;
		if(fd<0)return -1;
		struct stat metadata;
		if(fstat(fd,&metadata)<0)return -1;
		if(!S_ISREG(metadata.st_mode)&&!S_ISCHR(metadata.st_mode)){errno=EINVAL;return -1;}
		int flags=fcntl(fd,F_GETFL);if(flags<0||fcntl(fd,F_SETFL,flags&~O_NONBLOCK)<0)return -1;
	} else {
		const char *name=hb_string(L,-1,NULL);
		if(!strcmp(name,"null")) {fd=open("/dev/null",O_CLOEXEC|(index==0?O_RDONLY:O_WRONLY));
			g->opened[index]=fd;
			if(fd<0)return -1;
		}
		else if(!strcmp(name,"closed"))fd=-1;
		else if(!strcmp(name,"pipe")) {
			struct hp_fd *r,*w;
			if(hp_pipe(&r,&w,0)<0)return -1;
			g->parent[index]=index==0?w:r;
			g->child[index]=index==0?r:w;
			fd=hp_fd_number(g->child[index]);
		} else if(strcmp(name,"inherit"))luaL_error(L,"unknown redirection '%s'",name);
	}
	g->spec.stdfd[index]=fd;
	g->spec.keep[index]=g->parent[index];
	lua_pop(L,1);
	return 0;
}

static int
spawn(lua_State *L)
{
	static const char *const options[]={"cwd","env","inherit_env","user","group","groups","stdin","stdout","stderr","new_session","process_group","inherit_fds","exec_timeout",NULL};
	struct hp_context *ctx=context(L);
	size_t argc=dense(L,1,4096);
	luaL_argcheck(L,argc>0,1,"executable is required");
	hb_options(L,2,options);
	if(lua_isnoneornil(L,2)){lua_settop(L,1);
		lua_newtable(L);
	}
	int opts=2;
	struct launch*g=lua_newuserdatauv(L,sizeof(*g),0);
	memset(g,0,sizeof(*g));
	for(int i=0;i<3;i++)g->opened[i]=-1;
	luaL_setmetatable(L,HB_LAUNCH);
	int guard=lua_gettop(L);
	g->argv=allocate(L,(argc+1)*sizeof(char*));
	for(size_t i=0;i<argc;i++){lua_rawgeti(L,1,(lua_Integer)i+1);
		g->argv[i]=copy_string(L,-1);
		lua_pop(L,1);
	}
	if(!g->argv[0][0])luaL_argerror(L,1,"empty executable");
	g->spec.argv=g->argv;
	environment(L,g,opts);
	if(credentials(L,g,opts)<0)return hb_error(L,"spawn","credentials",errno);
	lua_getfield(L,opts,"cwd");
	if(!lua_isnil(L,-1))g->spec.cwd=hb_string(L,-1,NULL);
	lua_pop(L,1);
	lua_getfield(L,opts,"new_session");
	g->spec.session=lua_isnil(L,-1)?0:hb_boolean(L,-1);
	lua_pop(L,1);
	lua_getfield(L,opts,"process_group");
	g->spec.group=lua_isnil(L,-1)?0:hb_boolean(L,-1);
	lua_pop(L,1);
	lua_getfield(L,opts,"exec_timeout");
	g->spec.exec_timeout_ms=hb_timeout(L,-1,10000);
	lua_pop(L,1);
	lua_getfield(L,opts,"inherit_fds");
	if(!lua_isnil(L,-1)) {
		size_t n=dense(L,-1,HP_MAX_INHERIT);
		g->spec.ninherit=n;
		g->spec.inherit=g->inherit;
		for(size_t i=0;i<n;i++) {lua_rawgeti(L,-1,(lua_Integer)i+1);
			lua_Integer fd=luaL_testudata(L,-1,HB_FD)?hp_fd_number(hb_check_fd(L,-1)):luaL_checkinteger(L,-1);
			if(fd<3||fd>INT_MAX)luaL_error(L,"inherited descriptor must be >=3");
			g->inherit[i]=(int)fd;
			lua_pop(L,1);
		}
	}
	lua_pop(L,1);
	for(int i=0;i<3;i++)if(redirect(L,g,opts,i)<0)return hb_error(L,"spawn","redirection",errno);
	size_t bytes=(argc+1)*sizeof(char*);
	long argument_limit=sysconf(_SC_ARG_MAX);if(argument_limit<1)argument_limit=262144;
	for(size_t i=0;g->argv[i];i++)bytes+=strlen(g->argv[i])+1;
	for(size_t i=0;g->env[i];i++)bytes+=strlen(g->env[i])+1+sizeof(char*);
	if(bytes>(size_t)argument_limit)return hb_error(L,"spawn","arguments",E2BIG);
	const char*stage;
	g->process=hp_spawn(ctx,&g->spec,&stage);
	if(!g->process)return hb_error(L,"spawn",stage,errno);
	/* The guard owns all allocations across possible Lua allocation errors. */
	struct hb_child*c=lua_newuserdatauv(L,sizeof(*c),1);
	c->process=NULL;
	luaL_setmetatable(L,HB_CHILD);
	lua_newtable(L);
	static const char*const streams[]={"stdin","stdout","stderr"};
	for(int i=0;i<3;i++) {
		if(g->child[i])hp_fd_close(g->child[i]);
		if(g->parent[i]) {hb_push_fd(L,g->parent[i]);
			g->parent[i]=NULL;
			lua_setfield(L,-2,streams[i]);
		}
	}
	lua_setiuservalue(L,-2,1);
	c->process=g->process;
	g->process=NULL;
	/* Move the guard to argument 1 without removing the returned child. */
	lua_pushvalue(L,guard);
	lua_replace(L,1);
	launch_gc(L);
	return 1;
}

static int
run_child(lua_State *L)
{
	static const char *const keys[]={"cwd","env","inherit_env","user","group","groups","stdin","stdout","stderr","new_session","process_group","inherit_fds","exec_timeout","timeout","grace","output_limit","input",NULL};
	struct hp_result ignored;
	int own_group;
	(void)context(L);
	hb_options(L,2,keys);
	luaL_checktype(L,1,LUA_TTABLE);
	if(lua_isnoneornil(L,2)){lua_settop(L,1);
		lua_newtable(L);
	}
	lua_getfield(L,2,"timeout");
	int timeout=hb_timeout(L,-1,-1);
	lua_pop(L,1);
	lua_getfield(L,2,"grace");
	int grace=hb_timeout(L,-1,5000);
	lua_pop(L,1);
	lua_getfield(L,2,"output_limit");
	lua_Integer limit=luaL_optinteger(L,-1,1024*1024);
	lua_pop(L,1);
	luaL_argcheck(L,limit>0&&limit<=HB_MAX_IO,2,"output_limit must be 1..64 MiB");
	lua_getfield(L,2,"input");
	size_t input_size=0;
	const char *input_data=NULL;
	if(!lua_isnil(L,-1)){luaL_checktype(L,-1,LUA_TSTRING);
		input_data=lua_tolstring(L,-1,&input_size);
		luaL_argcheck(L,input_size<=HB_MAX_IO,2,"input exceeds 64 MiB");
	}
	char *buffers[2]={NULL,NULL};
	size_t sizes[2]={0,0};
	const char *const names[]={"stdout","stderr"};
	for(int i=0;i<2;i++){lua_getfield(L,2,names[i]);
		if(lua_type(L,-1)==LUA_TSTRING&&!strcmp(lua_tostring(L,-1),"pipe"))buffers[i]=lua_newuserdatauv(L,(size_t)limit,0);
	}
	struct run_resources *r=lua_newuserdatauv(L,sizeof(*r),0);
	memset(r,0xff,sizeof(*r));
	luaL_setmetatable(L,HB_RUN);
	int guard=lua_gettop(L);
	lua_newtable(L);
	int opts=lua_gettop(L);
	lua_pushnil(L);
	while(lua_next(L,2)){const char*k=lua_tostring(L,-2);
		if(strcmp(k,"timeout")&&strcmp(k,"grace")&&strcmp(k,"output_limit")&&strcmp(k,"input")){lua_pushvalue(L,-2);
			lua_pushvalue(L,-2);
			lua_rawset(L,opts);
		}
		lua_pop(L,1);
	}
	if(input_data){lua_pushliteral(L,"pipe");
		lua_setfield(L,opts,"stdin");
	}
	lua_pushvalue(L,lua_upvalueindex(1));
	lua_pushcclosure(L,spawn,1);
	lua_pushvalue(L,1);
	lua_pushvalue(L,opts);
	lua_call(L,2,2);
	if(lua_isnil(L,-2))return 2;
	lua_pop(L,1);
	int handle=lua_gettop(L);
	struct hp_process *p=child(L,handle);
	int error=0;
	const char *stage="monitor";
	r->kq=kqueue();
	if(r->kq<0){error=errno;
		goto failed;
	}
	if(fcntl(r->kq,F_SETFD,FD_CLOEXEC)<0){error=errno;
		goto failed;
	}
	struct kevent ev;
	r->pd=hp_descriptor(p);
	if(r->pd>=0){EV_SET(&ev,r->pd,EVFILT_PROCDESC,EV_ADD|EV_ONESHOT,NOTE_EXIT,0,(void*)1);
		if(kevent(r->kq,&ev,1,NULL,0,NULL)<0){error=errno;
			goto failed;
	}}
	else if(errno!=ESRCH){error=errno;
		goto failed;
	}
	lua_getiuservalue(L,handle,1);
	int streams=lua_gettop(L);
	for(int i=0;i<2;i++)if(buffers[i]) {
		lua_getfield(L,streams,names[i]);
		struct hp_fd *fd=hb_check_fd(L,-1);
		if(hp_fd_nonblock(fd,1)<0){error=errno;
			goto failed;
		}
		r->output[i]=hp_fd_dup(fd);
		lua_pop(L,1);
		if(r->output[i]<0){error=errno;
			goto failed;
		}
		EV_SET(&ev,r->output[i],EVFILT_READ,EV_ADD,0,0,(void*)(uintptr_t)(i+2));
		if(kevent(r->kq,&ev,1,NULL,0,NULL)<0){error=errno;
			goto failed;
		}
	}
	lua_getfield(L,streams,"stdin");
	if(luaL_testudata(L,-1,HB_FD)) {
		struct hp_fd *fd=hb_check_fd(L,-1);
		/* Only automatically created input pipes are closed by run. */
		lua_getfield(L,opts,"stdin");
		int automatic=lua_type(L,-1)==LUA_TSTRING&&!strcmp(lua_tostring(L,-1),"pipe");
		lua_pop(L,1);
		if(automatic) {
			if(input_size){if(hp_fd_nonblock(fd,1)<0){error=errno;
					goto failed;
				}
				r->input=hp_fd_dup(fd);
				if(r->input<0){error=errno;
					goto failed;
				}
				EV_SET(&ev,r->input,EVFILT_WRITE,EV_ADD,0,0,(void*)4);
				if(kevent(r->kq,&ev,1,NULL,0,NULL)<0){error=errno;
					goto failed;
			}}
			/* Closing the original leaves the explicit writer duplicate alive. */
			hp_fd_close(fd);
		}
	}
	lua_pop(L,1);
	if(timeout>=0){EV_SET(&ev,1,EVFILT_TIMER,EV_ADD|EV_ONESHOT,0,timeout?timeout:1,(void*)5);
		if(kevent(r->kq,&ev,1,NULL,0,NULL)<0){error=errno;
			goto failed;
	}}
	size_t sent=0;
	struct hp_result result;
	for(;;) {
		hp_status(p,&result);
		if(result.done&&r->output[0]<0&&r->output[1]<0)break;
		int n=kevent(r->kq,NULL,0,&ev,1,NULL);
		if(n<0){if(errno==EINTR)continue;
			error=errno;
			goto failed;
		}
		if(ev.flags&EV_ERROR){error=(int)ev.data;
			goto failed;
		}
		uintptr_t tag=(uintptr_t)ev.udata;
		if(tag==5){error=ETIMEDOUT;
			stage="timeout";
			goto failed;
		}
		if(tag==1){if(hp_wait(p,5000,&result)!=1){error=errno;
				goto failed;
			}
			continue;
		}
		if(tag==4) {
			ssize_t wrote=hp_write(r->input,input_data+sent,input_size-sent);
			if(wrote<0){if(errno==EAGAIN||errno==EINTR)continue;
				error=errno;
				stage="input";
				goto failed;
			}
			sent+=(size_t)wrote;
			if(sent==input_size){close(r->input);
				r->input=-1;
			}
			continue;
		}
		if(tag==2||tag==3) {
			int i=(int)tag-2;
			char scratch[8192];
			ssize_t got=read(r->output[i],scratch,sizeof(scratch));
			if(got<0){if(errno==EINTR||errno==EAGAIN)continue;
				error=errno;
				stage="output";
				goto failed;
			}
			if(!got){close(r->output[i]);
				r->output[i]=-1;
				continue;
			}
			if((size_t)got>(size_t)limit-sizes[i]){error=EFBIG;
				stage="output_limit";
				goto failed;
			}
			memcpy(buffers[i]+sizes[i],scratch,(size_t)got);
			sizes[i]+=(size_t)got;
		}
	}
	hb_result(L,&result);
	for(int i=0;i<2;i++)if(buffers[i]){lua_pushlstring(L,buffers[i],sizes[i]);
		lua_setfield(L,-2,names[i]);
	}
	lua_pushvalue(L,guard);
	lua_replace(L,1);
	run_gc(L);
	return 1;
failed:
	hp_status(p,&ignored);
	own_group=ignored.own_group;
	(void)hp_signal(p,SIGTERM,own_group);
	if(hp_wait(p,grace,&ignored)!=1){(void)hp_signal(p,SIGKILL,own_group);
		(void)hp_wait(p,5000,&ignored);
	}
	lua_pushvalue(L,guard);
	lua_replace(L,1);
	run_gc(L);
	return hb_error(L,"run",stage,error);
}

#define HB_SNAPSHOT "halfbsd.process_snapshot"
struct snapshot { size_t count; struct hp_process *processes[HP_MAX_CHILDREN]; };
static int snapshot_gc(lua_State *L) {
	struct snapshot *s=luaL_checkudata(L,1,HB_SNAPSHOT);
	for(size_t i=0;i<s->count;i++){hp_process_unref(s->processes[i]);s->processes[i]=NULL;}
	s->count=0;return 0;
}
static int active_children(lua_State *L) {
	struct hp_context *ctx=context(L);
	struct snapshot *s=lua_newuserdatauv(L,sizeof(*s),0);memset(s,0,sizeof(*s));luaL_setmetatable(L,HB_SNAPSHOT);
	s->count=hp_active(ctx,s->processes,HP_MAX_CHILDREN);
	lua_createtable(L,(int)s->count,0);int result=lua_gettop(L);
	static const char *const names[]={"stdin","stdout","stderr"};
	for(size_t i=0;i<s->count;i++) {
		struct hb_child *c=lua_newuserdatauv(L,sizeof(*c),1);c->process=NULL;luaL_setmetatable(L,HB_CHILD);
		c->process=s->processes[i];s->processes[i]=NULL;
		lua_newtable(L);for(int j=0;j<3;j++) {
			struct hb_fd *fd=lua_newuserdatauv(L,sizeof(*fd),0);fd->fd=NULL;luaL_setmetatable(L,HB_FD);
			fd->fd=hp_process_stream(c->process,j);
			if(fd->fd)lua_setfield(L,-2,names[j]);else lua_pop(L,1);
		}
		lua_setiuservalue(L,-2,1);lua_rawseti(L,result,(lua_Integer)i+1);
	}
	return 1;
}
static int wait_any(lua_State *L) {
	size_t n=dense(L,1,HP_MAX_CHILDREN);
	luaL_argcheck(L,n>0,1,"children required");
	struct hp_process *list[HP_MAX_CHILDREN];
	for(size_t i=0;i<n;i++){lua_rawgeti(L,1,(lua_Integer)i+1);
		list[i]=child(L,-1);
		lua_pop(L,1);
	}
	struct hp_result r;
	size_t index;
	if(hp_wait_any(list,n,hb_timeout(L,2,-1),&index,&r)!=1)return hb_error(L,"wait_any",NULL,errno);
	lua_pushinteger(L,(lua_Integer)index+1);
	hb_result(L,&r);
	return 2;
}

static int shutdown_children(lua_State *L) {
	int grace=hb_timeout(L,1,5000),group=lua_isnoneornil(L,2)?1:hb_boolean(L,2);
	if(hp_shutdown(context(L),grace,group)<0)return hb_error(L,"shutdown",NULL,errno);
	lua_pushboolean(L,1);
	return 1;
}

static int pipe_lua(lua_State *L) {
	int nonblock=lua_isnoneornil(L,1)?0:hb_boolean(L,1);
	/* Allocate both owners before acquiring descriptors. */
	struct hb_fd*r=lua_newuserdatauv(L,sizeof(*r),0);
	r->fd=NULL;
	luaL_setmetatable(L,HB_FD);
	struct hb_fd*w=lua_newuserdatauv(L,sizeof(*w),0);
	w->fd=NULL;
	luaL_setmetatable(L,HB_FD);
	if(hp_pipe(&r->fd,&w->fd,nonblock)<0)return hb_error(L,"pipe",NULL,errno);
	return 2;
}

static int fd_lua(lua_State *L) {
	lua_Integer source=luaL_checkinteger(L,1);
	luaL_argcheck(L,source>=0&&source<=INT_MAX,1,"invalid descriptor");
	struct hb_fd*f=lua_newuserdatauv(L,sizeof(*f),0);
	f->fd=NULL;
	luaL_setmetatable(L,HB_FD);
	int copy=fcntl((int)source,F_DUPFD_CLOEXEC,3);
	if(copy<0)return hb_error(L,"fd",NULL,errno);
	f->fd=hp_fd_new(copy);
	if(!f->fd)return hb_error(L,"fd",NULL,errno);
	return 1;
}

int
luaopen_proc(lua_State *L)
{
	static const luaL_Reg methods[]={{"__gc",child_gc},{"__index",child_index},{"status",status},{"wait",wait_child},{"signal",signal_child},{"terminate",terminate_child},{"kill",kill_child},{"foreground",foreground_child},{NULL,NULL}};
	hb_fd_metatable(L);
	if(luaL_newmetatable(L,HB_CHILD)){luaL_setfuncs(L,methods,0);
		lua_pushliteral(L,HB_CHILD);
		lua_setfield(L,-2,"__metatable");
	}
	lua_pop(L,1);
	if(luaL_newmetatable(L,HB_LAUNCH)){lua_pushcfunction(L,launch_gc);
		lua_setfield(L,-2,"__gc");
	}
	lua_pop(L,1);
	if(luaL_newmetatable(L,HB_SNAPSHOT)){lua_pushcfunction(L,snapshot_gc);lua_setfield(L,-2,"__gc");}
	lua_pop(L,1);
	if(luaL_newmetatable(L,HB_RUN)){lua_pushcfunction(L,run_gc);
		lua_setfield(L,-2,"__gc");
	}
	lua_pop(L,1);
	if(luaL_newmetatable(L,HB_MANAGER)){lua_pushcfunction(L,manager_gc);
		lua_setfield(L,-2,"__gc");
	}
	lua_pop(L,1);
	struct manager*m=lua_newuserdatauv(L,sizeof(*m),0);
	m->ctx=NULL;
	luaL_setmetatable(L,HB_MANAGER);
	m->ctx=hp_context_new();
	if(!m->ctx)return luaL_error(L,"process monitor initialization: %s",strerror(errno));
	int manager=lua_gettop(L);
	lua_newtable(L);
	static const luaL_Reg functions[]={{"spawn",spawn},{"run",run_child},{"active",active_children},{"wait_any",wait_any},{"shutdown",shutdown_children},{NULL,NULL}};
	lua_pushvalue(L,manager);
	luaL_setfuncs(L,functions,1);
	static const luaL_Reg plain[]={{"wait",wait_child},{"status",status},{"signal",signal_child},{"terminate",terminate_child},{"kill",kill_child},{"pipe",pipe_lua},{"fd",fd_lua},{NULL,NULL}};
	luaL_setfuncs(L,plain,0);
	return 1;
}
