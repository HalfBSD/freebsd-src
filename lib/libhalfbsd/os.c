/* SPDX-License-Identifier: BSD-2-Clause */
#include <sys/mount.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <limits.h>
#include "internal.h"
extern char **environ;
static int success(lua_State*L,int result,const char*name) {
	if(result<0)return hb_error(L,name,NULL,errno);
	lua_pushboolean(L,1);
	return 1;
}

static mode_t mode(lua_State*L,int index,mode_t def) {
	lua_Integer v=luaL_optinteger(L,index,def);
	luaL_argcheck(L,v>=0&&v<=07777,index,"invalid file mode");
	return (mode_t)v;
}

static int metadata(lua_State*L) {
	const char*path=hb_string(L,1,NULL);
	int follow=lua_isnoneornil(L,2)?0:hb_boolean(L,2);
	struct stat st;
	if((follow?stat(path,&st):lstat(path,&st))<0)return hb_error(L,"stat",NULL,errno);
	lua_createtable(L,0,12);
#define N(key,value) lua_pushinteger(L,(lua_Integer)(value));lua_setfield(L,-2,key)
	N("uid",st.st_uid);
	N("gid",st.st_gid);
	N("mode",st.st_mode&07777);
	N("size",st.st_size);
	N("inode",st.st_ino);
	N("device",st.st_dev);
	N("links",st.st_nlink);
#undef N
	const char*type=S_ISREG(st.st_mode)?"file":S_ISDIR(st.st_mode)?"directory":S_ISLNK(st.st_mode)?"symlink":S_ISFIFO(st.st_mode)?"fifo":S_ISSOCK(st.st_mode)?"socket":"device";
	lua_pushstring(L,type);
	lua_setfield(L,-2,"type");
	lua_pushnumber(L,st.st_mtim.tv_sec+st.st_mtim.tv_nsec/1e9);
	lua_setfield(L,-2,"mtime");
	return 1;
}

static int exists(lua_State*L) {
	struct stat st;
	const char*p=hb_string(L,1,NULL);
	if(lstat(p,&st)<0){if(errno!=ENOENT&&errno!=ENOTDIR)return hb_error(L,"exists",NULL,errno);
		lua_pushboolean(L,0);
		return 1;
	}lua_pushboolean(L,1);
	return 1;
}

static int mkdir_lua(lua_State*L){const char*p=hb_string(L,1,NULL);
	mode_t m=mode(L,2,0755);
	return success(L,mkdir(p,m),"mkdir");
}

static int rmdir_lua(lua_State*L){return success(L,rmdir(hb_string(L,1,NULL)),"rmdir");
}

static int unlink_lua(lua_State*L){return success(L,unlink(hb_string(L,1,NULL)),"unlink");
}

static int chmod_lua(lua_State*L){const char*p=hb_string(L,1,NULL);
	mode_t m=mode(L,2,0600);
	return success(L,lchmod(p,m),"chmod");
}

static int chown_lua(lua_State*L){const char*p=hb_string(L,1,NULL);
	lua_Integer u=luaL_checkinteger(L,2),g=luaL_checkinteger(L,3);
	luaL_argcheck(L,u>=0&&(uintmax_t)u<UINT_MAX,2,"invalid uid");
	luaL_argcheck(L,g>=0&&(uintmax_t)g<UINT_MAX,3,"invalid gid");
	return success(L,lchown(p,(uid_t)u,(gid_t)g),"chown");
}

static int symlink_lua(lua_State*L){const char*target=hb_string(L,1,NULL),*path=hb_string(L,2,NULL);
	return success(L,symlink(target,path),"symlink");
}

static int readlink_lua(lua_State*L) {
	const char*p=hb_string(L,1,NULL);
	char*buffer=lua_newuserdatauv(L,1024*1024,0);
	ssize_t n=readlink(p,buffer,1024*1024);
	if(n<0)return hb_error(L,"readlink",NULL,errno);
	if(n==1024*1024)return hb_error(L,"readlink",NULL,EOVERFLOW);
	lua_pushlstring(L,buffer,(size_t)n);
	return 1;
}

static int open_lua(lua_State*L) {
	const char*path=hb_string(L,1,NULL),*kind=luaL_optstring(L,2,"read");
	mode_t m=mode(L,3,0600);
	int flags;
	if(!strcmp(kind,"read"))flags=O_RDONLY;
	else if(!strcmp(kind,"write"))flags=O_WRONLY|O_CREAT|O_TRUNC;
	else if(!strcmp(kind,"append"))flags=O_WRONLY|O_CREAT|O_APPEND;
	else if(!strcmp(kind,"exclusive"))flags=O_WRONLY|O_CREAT|O_EXCL;
	else return luaL_argerror(L,2,"expected read, write, append or exclusive");
	struct hb_fd*f=lua_newuserdatauv(L,sizeof(*f),0);
	f->fd=NULL;
	luaL_setmetatable(L,HB_FD);
	int fd=open(path,flags|O_NOFOLLOW|O_CLOEXEC,m);
	if(fd<0)return hb_error(L,"open",NULL,errno);
	f->fd=hp_fd_new(fd);
	if(!f->fd)return hb_error(L,"open",NULL,errno);
	return 1;
}

/* All C resources below are closed before any Lua allocation. Walk parent
* components using directory descriptors, rejecting symlink components. */
static int atomic_replace(lua_State*L) {
	size_t path_length;
	const char*path=hb_string(L,1,&path_length);
	luaL_argcheck(L,path_length>0&&path[path_length-1]!='/',1,"replacement needs a final filename");
	size_t length;
	luaL_checktype(L,2,LUA_TSTRING);
	const char*data=lua_tolstring(L,2,&length);
	luaL_argcheck(L,length<=HB_MAX_IO,2,"replacement exceeds 64 MiB");
	mode_t m=mode(L,3,0600);
	char*copy=strdup(path);
	if(!copy)return hb_error(L,"atomic_replace","allocate",errno);
	int dir=open(path[0]=='/'?"/":".",O_RDONLY|O_DIRECTORY|O_CLOEXEC),fd=-1,e=0,committed=0;
	char temp[64]={0},*save,*component=strtok_r(copy,"/",&save),*base=NULL;
	if(dir<0){e=errno;
		goto out;
	}
	if(!component){e=EINVAL;
		goto out;
	}
	for(;;) {
		if(!strcmp(component,"..")||!strcmp(component,".")){e=EINVAL;
			goto out;
		}
		char*next=strtok_r(NULL,"/",&save);
		if(!next){base=component;
			break;
		}
		int d=openat(dir,component,O_RDONLY|O_DIRECTORY|O_NOFOLLOW|O_CLOEXEC);
		if(d<0){e=errno;
			goto out;
		}close(dir);
		dir=d;
		component=next;
	}
	for(int i=0;i<64;i++) {
		snprintf(temp,sizeof(temp),".halfbsd-%08x-%08x",arc4random(),arc4random());
		fd=openat(dir,temp,O_WRONLY|O_CREAT|O_EXCL|O_NOFOLLOW|O_CLOEXEC,0600);
		if(fd>=0)break;
		if(errno!=EEXIST){e=errno;
			goto out;
		}
	}if(fd<0){e=EEXIST;
		goto out;
	}
	for(size_t offset=0;offset<length;) {
		ssize_t n=hp_write(fd,data+offset,length-offset);
		if(n<=0){e=n<0?errno:EIO;
			goto out;
		}offset+=(size_t)n;
	}
	if(fchmod(fd,m)<0||fsync(fd)<0){e=errno;
		goto out;
	}
	if(close(fd)<0){fd=-1;
		e=errno;
		goto out;
	}fd=-1;
	if(renameat(dir,temp,dir,base)<0){e=errno;
		goto out;
	}committed=1;
	if(fsync(dir)<0)e=errno;
	out:
	if(fd>=0)close(fd);
	if(dir>=0){if(!committed&&temp[0])unlinkat(dir,temp,0);
		close(dir);
	}free(copy);
	if(e){hb_error(L,"atomic_replace",committed?"directory_sync":"replace",e);
		lua_pushboolean(L,committed);
		lua_setfield(L,-2,"committed");
		return 2;
	}
	lua_pushboolean(L,1);
	return 1;
}

static int hostname(lua_State*L){char name[MAXHOSTNAMELEN+1];
	if(gethostname(name,sizeof(name))<0)return hb_error(L,"hostname",NULL,errno);
	name[MAXHOSTNAMELEN]=0;
	lua_pushstring(L,name);
	return 1;
}

static int set_hostname(lua_State*L){size_t n;
	const char*s=hb_string(L,1,&n);
	luaL_argcheck(L,n>0&&n<MAXHOSTNAMELEN,1,"invalid hostname length");
	return success(L,sethostname(s,(int)n),"set_hostname");
}

static int clock_lua(lua_State*L){const char*k=luaL_optstring(L,1,"monotonic");
	clockid_t id;
	if(!strcmp(k,"monotonic"))id=CLOCK_MONOTONIC;
	else if(!strcmp(k,"realtime"))id=CLOCK_REALTIME;
	else return luaL_argerror(L,1,"expected monotonic or realtime");
	struct timespec t;
	if(clock_gettime(id,&t)<0)return hb_error(L,"clock",NULL,errno);
	lua_pushnumber(L,t.tv_sec+t.tv_nsec/1e9);
	return 1;
}

static int set_clock(lua_State *L)
{
	lua_Integer seconds=luaL_checkinteger(L,1),nanoseconds=luaL_optinteger(L,2,0);
	luaL_argcheck(L,seconds>=0,1,"negative realtime clock value");
	luaL_argcheck(L,nanoseconds>=0&&nanoseconds<1000000000,2,"invalid nanoseconds");
	struct timespec time={(time_t)seconds,(long)nanoseconds};
	return success(L,clock_settime(CLOCK_REALTIME,&time),"set_clock");
}

static int getenv_lua(lua_State*L){const char*v=getenv(hb_string(L,1,NULL));
	if(v)lua_pushstring(L,v);
	else lua_pushnil(L);
	return 1;
}

static int setenv_lua(lua_State*L){const char*k=hb_string(L,1,NULL);
	luaL_argcheck(L,*k&&!strchr(k,'='),1,"invalid variable name");
	if(lua_isnoneornil(L,2))return success(L,unsetenv(k),"unsetenv");
	return success(L,setenv(k,hb_string(L,2,NULL),1),"setenv");
}

static int environment(lua_State*L){
	lua_newtable(L);
	for(char**p=environ;*p;p++){const char*eq=strchr(*p,'=');
		if(eq){lua_pushlstring(L,*p,(size_t)(eq-*p));
			lua_pushstring(L,eq+1);
			lua_rawset(L,-3);
	}}return 1;
}

static int identity(lua_State*L) {
	lua_newtable(L);
#define ID(k,v) lua_pushinteger(L,(lua_Integer)(v));lua_setfield(L,-2,k)
	ID("pid",getpid());
	ID("ppid",getppid());
	ID("uid",getuid());
	ID("euid",geteuid());
	ID("gid",getgid());
	ID("egid",getegid());
	ID("group",getpgrp());
	ID("session",getsid(0));
#undef ID
	int n=getgroups(0,NULL);
	if(n<0)return hb_error(L,"identity","groups",errno);
	gid_t*g=lua_newuserdatauv(L,((size_t)n+1)*sizeof(*g),0);
	n=getgroups(n,g);
	if(n<0)return hb_error(L,"identity","groups",errno);
	lua_createtable(L,n,0);
	for(int i=0;i<n;i++){lua_pushinteger(L,g[i]);
		lua_rawseti(L,-2,i+1);
	}lua_setfield(L,-3,"groups");
	lua_pop(L,1);
	return 1;
}

static int lookup_user(lua_State*L) {
	char*buffer=lua_newuserdatauv(L,1024*1024,0);
	struct passwd pw,*result;
	int e;
	if(lua_isinteger(L,1)){lua_Integer id=lua_tointeger(L,1);
		luaL_argcheck(L,id>=0&&(uintmax_t)id<UINT_MAX,1,"invalid uid");
		e=getpwuid_r((uid_t)id,&pw,buffer,1024*1024,&result);
	}
	else e=getpwnam_r(hb_string(L,1,NULL),&pw,buffer,1024*1024,&result);
	if(e||!result)return hb_error(L,"user",NULL,e?e:ENOENT);
	lua_createtable(L,0,5);
	lua_pushstring(L,pw.pw_name);
	lua_setfield(L,-2,"name");
	lua_pushinteger(L,pw.pw_uid);
	lua_setfield(L,-2,"uid");
	lua_pushinteger(L,pw.pw_gid);
	lua_setfield(L,-2,"gid");
	lua_pushstring(L,pw.pw_dir);
	lua_setfield(L,-2,"home");
	lua_pushstring(L,pw.pw_shell);
	lua_setfield(L,-2,"shell");
	return 1;
}

static int lookup_group(lua_State*L) {
	char*buffer=lua_newuserdatauv(L,1024*1024,0);
	struct group gr,*result;
	int e;
	if(lua_isinteger(L,1)){lua_Integer id=lua_tointeger(L,1);
		luaL_argcheck(L,id>=0&&(uintmax_t)id<UINT_MAX,1,"invalid gid");
		e=getgrgid_r((gid_t)id,&gr,buffer,1024*1024,&result);
	}
	else e=getgrnam_r(hb_string(L,1,NULL),&gr,buffer,1024*1024,&result);
	if(e||!result)return hb_error(L,"group",NULL,e?e:ENOENT);
	lua_createtable(L,0,3);
	lua_pushstring(L,gr.gr_name);
	lua_setfield(L,-2,"name");
	lua_pushinteger(L,gr.gr_gid);
	lua_setfield(L,-2,"gid");
	lua_newtable(L);
	for(size_t i=0;gr.gr_mem[i];i++){lua_pushstring(L,gr.gr_mem[i]);
		lua_rawseti(L,-2,(lua_Integer)i+1);
	}lua_setfield(L,-2,"members");
	return 1;
}

static int disposition(lua_State*L){int sig=hb_signal_number(L,1);
	const char*k=hb_string(L,2,NULL);
	int action=!strcmp(k,"ignore")?0:!strcmp(k,"default")?2:-1;
	luaL_argcheck(L,action>=0,2,"expected ignore or default");
	return success(L,hb_signal_disposition(sig,action,NULL),"signal_disposition");
}

static int mount_lua(lua_State*L) {
	/* A bounded native nmount key/value array. No shell or implicit flags. */
	luaL_checktype(L,1,LUA_TTABLE);
	lua_Integer flags=luaL_optinteger(L,2,0);
	luaL_argcheck(L,flags>=0&&flags<=INT_MAX,2,"invalid mount flags");
	struct iovec iov[128];
	char *storage=lua_newuserdatauv(L,1024*1024,0);
	size_t used=0;
	size_t count=0;
	lua_pushnil(L);
	while(lua_next(L,1)) {
		if(count==128)luaL_error(L,"mount options exceed 64 entries");
		size_t a,b;
		const char*k=hb_string(L,-2,&a),*v=hb_string(L,-1,&b);
		if(a+b+2>1024*1024-used)luaL_argerror(L,1,"mount options exceed 1 MiB");
		memcpy(storage+used,k,a+1);
		iov[count++]=(struct iovec){storage+used,a+1};used+=a+1;
		memcpy(storage+used,v,b+1);
		iov[count++]=(struct iovec){storage+used,b+1};used+=b+1;
		lua_pop(L,1);
	}return success(L,nmount(iov,(unsigned int)count,(int)flags),"mount");
}

static int unmount_lua(lua_State*L){const char*p=hb_string(L,1,NULL);
	lua_Integer flags=luaL_optinteger(L,2,0);
	luaL_argcheck(L,flags>=0&&flags<=INT_MAX,2,"invalid unmount flags");
	return success(L,unmount(p,(int)flags),"unmount");
}

int luaopen_halfbsd_os(lua_State*L) {
	static const luaL_Reg f[]={{"stat",metadata},{"exists",exists},{"mkdir",mkdir_lua},{"rmdir",rmdir_lua},{"unlink",unlink_lua},{"chmod",chmod_lua},{"chown",chown_lua},{"symlink",symlink_lua},{"readlink",readlink_lua},{"open",open_lua},{"atomic_replace",atomic_replace},{"hostname",hostname},{"set_hostname",set_hostname},{"clock",clock_lua},{"set_clock",set_clock},{"getenv",getenv_lua},{"setenv",setenv_lua},{"environment",environment},{"identity",identity},{"user",lookup_user},{"group",lookup_group},{"signal_disposition",disposition},{"mount",mount_lua},{"unmount",unmount_lua},{NULL,NULL}};
	hb_fd_metatable(L);
	luaL_newlib(L,f);
	return 1;
}
