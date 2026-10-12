/* SPDX-License-Identifier: BSD-2-Clause */
#include <sys/event.h>
#include "internal.h"
#define HB_LOOP "halfbsd.event_loop"
#define HB_WATCHES 4096
struct watch {
	uintptr_t id;
	int fd, callback, target, signal_owned, one;
	short filter;
	struct sigaction old;
};
struct loop { int kq, stopped, dispatching;
	uintptr_t serial;
	struct watch watches[HB_WATCHES];
};
static struct loop *check(lua_State*L) {
	struct loop*l=luaL_checkudata(L,1,HB_LOOP);
	luaL_argcheck(L,l->kq>=0,1,"closed event loop");
	return l;
}

static void cancel_watch(lua_State *L,struct loop*l,struct watch*w) {
	struct kevent ev;
	if(!w->id)return;
	EV_SET(&ev,(uintptr_t)w->fd,w->filter,EV_DELETE,0,0,NULL);
	(void)kevent(l->kq,&ev,1,NULL,0,NULL);
	if(w->signal_owned)hb_signal_restore(w->fd,&w->old);
	else if(w->filter==EVFILT_PROCDESC||w->filter==EVFILT_READ||w->filter==EVFILT_WRITE||w->filter==EVFILT_VNODE)close(w->fd);
	luaL_unref(L,LUA_REGISTRYINDEX,w->callback);
	luaL_unref(L,LUA_REGISTRYINDEX,w->target);
	memset(w,0,sizeof(*w));
}

static int close_loop(lua_State*L) {
	struct loop*l=luaL_checkudata(L,1,HB_LOOP);
	if(l->kq>=0){for(size_t i=0;i<HB_WATCHES;i++)cancel_watch(L,l,&l->watches[i]);
		close(l->kq);
		l->kq=-1;
	}
	l->stopped=1;
	lua_pushboolean(L,1);
	return 1;
}

static int new_loop(lua_State*L) {
	struct loop*l=lua_newuserdatauv(L,sizeof(*l),0);
	memset(l,0,sizeof(*l));
	l->kq=-1;
	luaL_setmetatable(L,HB_LOOP);
	l->kq=kqueue();
	if(l->kq>=0&&l->kq<3){int copy=fcntl(l->kq,F_DUPFD_CLOEXEC,3);int saved=errno;close(l->kq);l->kq=copy;errno=saved;}
	if(l->kq<0)return hb_error(L,"event.new",NULL,errno);
	if(fcntl(l->kq,F_SETFD,FD_CLOEXEC)<0){int e=errno;
		close(l->kq);
		l->kq=-1;
		return hb_error(L,"event.new",NULL,e);
	}return 1;
}

static struct watch*slot(lua_State*L,struct loop*l,int callback,int target) {
	luaL_checktype(L,callback,LUA_TFUNCTION);
	for(size_t i=0;i<HB_WATCHES;i++)if(!l->watches[i].id) {
		struct watch*w=&l->watches[i];
		if(l->serial==INT64_MAX)luaL_error(L,"event watch identifiers exhausted");
		/* Registry allocations happen before acquiring a kernel resource. */
		w->target=LUA_NOREF;
		w->callback=LUA_NOREF;
		w->fd=-1;
		w->id=++l->serial;
		lua_pushvalue(L,callback);
		w->callback=luaL_ref(L,LUA_REGISTRYINDEX);
		if(target){lua_pushvalue(L,target);
			w->target=luaL_ref(L,LUA_REGISTRYINDEX);
		}return w;
	}luaL_error(L,"event watch limit exceeded");
	return NULL;
}

static int install(lua_State*L,struct loop*l,struct watch*w,unsigned flags,unsigned notes,intptr_t data) {
	struct kevent ev;
	EV_SET(&ev,(uintptr_t)w->fd,w->filter,EV_ADD|flags,notes,data,(void*)w->id);
	if(kevent(l->kq,&ev,1,NULL,0,NULL)<0){int e=errno;
		cancel_watch(L,l,w);
		return hb_error(L,"event.watch",NULL,e);
	}
	lua_pushinteger(L,(lua_Integer)w->id);
	return 1;
}

static int watch_timer(lua_State*L) {
	struct loop*l=check(L);
	int ms=hb_timeout(L,2,-1);
	luaL_argcheck(L,ms>0,2,"timer interval must be positive");
	int repeat=lua_isnoneornil(L,4)?0:hb_boolean(L,4);
	struct watch*w=slot(L,l,3,0);
	w->fd=(int)w->id;
	w->filter=EVFILT_TIMER;
	w->one=!repeat;
	/* Timer identifiers must be unique even after cancellation. */
	if(w->id>INT_MAX){cancel_watch(L,l,w);
		return hb_error(L,"event.timer",NULL,EOVERFLOW);
	}
	return install(L,l,w,repeat?0:EV_ONESHOT,0,ms);
}

static int watch_process(lua_State*L) {
	struct loop*l=check(L);
	struct hb_child*c=luaL_checkudata(L,2,HB_CHILD);
	luaL_argcheck(L,c->process!=NULL,2,"released process");
	struct watch*w=slot(L,l,3,2);
	w->one=1;
	w->fd=hp_descriptor(c->process);
	if(w->fd<0) {
		int e=errno;
		if(e!=ESRCH){cancel_watch(L,l,w);
			return hb_error(L,"event.process",NULL,e);
		}
		w->filter=EVFILT_USER;
		w->fd=(int)w->id;
		if(w->id>INT_MAX){cancel_watch(L,l,w);
			return hb_error(L,"event.process",NULL,EOVERFLOW);
		}
		int n=install(L,l,w,EV_ONESHOT,0,0);
		if(n!=1)return n;
		struct kevent trigger;
		EV_SET(&trigger,(uintptr_t)w->fd,EVFILT_USER,0,NOTE_TRIGGER,0,(void*)w->id);
		if(kevent(l->kq,&trigger,1,NULL,0,NULL)<0){e=errno;
			cancel_watch(L,l,w);
			return hb_error(L,"event.process",NULL,e);
		}return 1;
	}
	w->filter=EVFILT_PROCDESC;
	return install(L,l,w,EV_ONESHOT,NOTE_EXIT,0);
}

static int watch_signal(lua_State*L) {
	struct loop*l=check(L);
	int sig=hb_signal_number(L,2);
	luaL_argcheck(L,sig!=SIGCHLD,2,"use watch_process for managed children");
	struct watch*w=slot(L,l,3,0);
	w->fd=sig;
	w->filter=EVFILT_SIGNAL;
	if(hb_signal_disposition(sig,1,&w->old)<0){int e=errno;
		cancel_watch(L,l,w);
		return hb_error(L,"event.signal",NULL,e);
	}
	w->signal_owned=1;
	return install(L,l,w,EV_CLEAR,0,0);
}

static int watch_fd(lua_State*L,short filter) {
	struct loop*l=check(L);
	struct hp_fd*fd=hb_check_fd(L,2);
	struct watch*w=slot(L,l,3,2);
	w->filter=filter;
	w->fd=hp_fd_dup(fd);
	if(w->fd<0){int e=errno;
		cancel_watch(L,l,w);
		return hb_error(L,"event.fd",NULL,e);
	}
	return install(L,l,w,EV_CLEAR,0,0);
}

static int watch_read(lua_State*L){return watch_fd(L,EVFILT_READ);
}

static int watch_write(lua_State*L){return watch_fd(L,EVFILT_WRITE);
}

static int watch_path(lua_State*L) {
	struct loop*l=check(L);
	const char*path=hb_string(L,2,NULL);
	struct watch*w=slot(L,l,3,0);
	w->filter=EVFILT_VNODE;
	w->fd=open(path,O_RDONLY|O_NONBLOCK|O_NOFOLLOW|O_CLOEXEC);
	if(w->fd<0){int e=errno;
		cancel_watch(L,l,w);
		return hb_error(L,"event.path",NULL,e);
	}
	return install(L,l,w,EV_CLEAR,NOTE_WRITE|NOTE_DELETE|NOTE_RENAME|NOTE_ATTRIB|NOTE_REVOKE,0);
}

static int cancel(lua_State*L) {
	struct loop*l=check(L);
	lua_Integer id=luaL_checkinteger(L,2);
	for(size_t i=0;i<HB_WATCHES;i++)if(id>0&&l->watches[i].id==(uintptr_t)id){cancel_watch(L,l,&l->watches[i]);
		lua_pushboolean(L,1);
		return 1;
	}
	lua_pushboolean(L,0);
	return 1;
}

static int stop(lua_State*L){check(L)->stopped=1;
	return 0;
}

static size_t count(struct loop*l){size_t n=0;
	for(size_t i=0;i<HB_WATCHES;i++)n+=l->watches[i].id!=0;
	return n;
}

static int
step_impl(lua_State*L,struct loop*l,int ms)
{
	struct kevent ev;
	struct timespec until,now,timeout,*tp=NULL;
	if(ms>=0){clock_gettime(CLOCK_MONOTONIC,&until);
		until.tv_sec+=ms/1000;
		until.tv_nsec+=(ms%1000)*1000000L;
		if(until.tv_nsec>=1000000000L){until.tv_sec++;
			until.tv_nsec-=1000000000L;
	}}
	int n;
	for(;;) {
		if(ms>=0){clock_gettime(CLOCK_MONOTONIC,&now);
			timeout.tv_sec=until.tv_sec-now.tv_sec;
			timeout.tv_nsec=until.tv_nsec-now.tv_nsec;
			if(timeout.tv_nsec<0){timeout.tv_sec--;
				timeout.tv_nsec+=1000000000L;
			}if(timeout.tv_sec<0)timeout=(struct timespec){0,0};
			tp=&timeout;
		}
		n=kevent(l->kq,NULL,0,&ev,1,tp);
		if(n<0&&errno==EINTR)continue;
		break;
	}
	if(n<0)return hb_error(L,"event.step",NULL,errno);
	if(!n){lua_pushboolean(L,0);
		return 1;
	}
	struct watch*w=NULL;
	for(size_t i=0;i<HB_WATCHES;i++)if(l->watches[i].id==(uintptr_t)ev.udata){w=&l->watches[i];
		break;
	}
	if(!w){lua_pushboolean(L,0);
		return 1;
	}
	if(ev.flags&EV_ERROR){int e=(int)ev.data;
		cancel_watch(L,l,w);
		return hb_error(L,"event.delivery",NULL,e);
	}
	lua_rawgeti(L,LUA_REGISTRYINDEX,w->callback);
	lua_createtable(L,0,8);
	lua_pushinteger(L,(lua_Integer)w->id);
	lua_setfield(L,-2,"id");
	lua_pushinteger(L,ev.data);
	lua_setfield(L,-2,"count");
	lua_pushinteger(L,ev.fflags);
	lua_setfield(L,-2,"flags");
	lua_pushboolean(L,ev.flags&EV_EOF);
	lua_setfield(L,-2,"eof");
	if(w->target!=LUA_NOREF){lua_rawgeti(L,LUA_REGISTRYINDEX,w->target);
		lua_setfield(L,-2,"target");
	}
	if(w->filter==EVFILT_PROCDESC||w->filter==EVFILT_USER) {
		lua_rawgeti(L,LUA_REGISTRYINDEX,w->target);
		struct hb_child*c=luaL_checkudata(L,-1,HB_CHILD);
		struct hp_result r;
		int done=hp_wait(c->process,5000,&r);
		lua_pop(L,1);
		if(done!=1){int e=errno;
			cancel_watch(L,l,w);
			return hb_error(L,"event.process","reap",e);
		}
		hb_result(L,&r);
		lua_setfield(L,-2,"result");
	}
	if(w->one)cancel_watch(L,l,w);
	l->dispatching=1;
	int callback_status=lua_pcall(L,1,0,0);
	l->dispatching=0;
	if(callback_status!=LUA_OK) {
		int cause=lua_gettop(L);
		hb_error(L,"event.callback",NULL,ECANCELED);
		lua_pushliteral(L,"callback");
		lua_setfield(L,-2,"kind");
		lua_pushvalue(L,cause);
		lua_setfield(L,-2,"cause");
		return 2;
	}
	lua_pushboolean(L,1);
	return 1;
}

static int step(lua_State*L){struct loop*l=check(L);
	int ms=hb_timeout(L,2,-1);
	luaL_argcheck(L,!l->dispatching,1,"recursive loop dispatch is not allowed");
	if(!count(l)){lua_pushboolean(L,0);
		return 1;
	}return step_impl(L,l,ms);
}

static int run(lua_State*L) {
	struct loop*l=check(L);
	luaL_argcheck(L,!l->dispatching,1,"recursive loop dispatch is not allowed");
	l->stopped=0;
	while(!l->stopped&&l->kq>=0&&count(l)) {
		int n=step_impl(L,l,-1);
		if(n==2)return n;
		lua_pop(L,1);
	}lua_pushboolean(L,1);
	return 1;
}

int luaopen_halfbsd_event(lua_State*L) {
	static const luaL_Reg methods[]={{"__gc",close_loop},{"__close",close_loop},{"close",close_loop},{"cancel",cancel},{"stop",stop},{"step",step},{"run",run},{"watch_process",watch_process},{"watch_timer",watch_timer},{"watch_signal",watch_signal},{"watch_read",watch_read},{"watch_write",watch_write},{"watch_path",watch_path},{NULL,NULL}};
	if(luaL_newmetatable(L,HB_LOOP)){luaL_setfuncs(L,methods,0);
		lua_pushvalue(L,-1);
		lua_setfield(L,-2,"__index");
		lua_pushliteral(L,HB_LOOP);
		lua_setfield(L,-2,"__metatable");
	}lua_pop(L,1);
	lua_newtable(L);
	lua_pushcfunction(L,new_loop);
	lua_setfield(L,-2,"new");
	return 1;
}
