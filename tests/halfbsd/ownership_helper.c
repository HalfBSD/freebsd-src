/* SPDX-License-Identifier: BSD-2-Clause */
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <lua.h>
#include <lauxlib.h>
#include <halfbsd_lua.h>
static volatile sig_atomic_t interrupted;
static pthread_t main_thread;
static void caught(int number) { interrupted = number; }
static void *interrupt_wait(void *unused)
{
	(void)unused;
	struct timespec delay = {0, 10000000};
	while (nanosleep(&delay, &delay) < 0) if (errno != EINTR) return NULL;
	(void)pthread_kill(main_thread, SIGUSR1);
	return NULL;
}
static int fd_count(void)
{
	int count = 0;
	for (int i = 0; i < 4096; i++) if (fcntl(i, F_GETFD) >= 0) count++;
	return count;
}
int main(int argc, char **argv)
{
	if (argc != 2) return 98;
	int before = fd_count();
	lua_State *L = luaL_newstate();
	if (!L) return 1;
	halfbsd_openlibs(L);
	lua_pushstring(L, argv[1]); lua_setglobal(L, "helper");
	struct sigaction action = {0}, previous;
	action.sa_handler = caught; sigemptyset(&action.sa_mask);
	if (sigaction(SIGUSR1, &action, &previous)) return 11;
	main_thread = pthread_self(); pthread_t interrupter;
	if (pthread_create(&interrupter, NULL, interrupt_wait, NULL)) return 12;
	int interrupted_wait = luaL_dostring(L,
	    "local e=assert(require('halfbsd.event').new()); "
	    "assert(e:watch_timer(0.05,function() end)); assert(e:run()); e:close()");
	pthread_join(interrupter, NULL); sigaction(SIGUSR1, &previous, NULL);
	if (interrupted_wait != LUA_OK || !interrupted) { lua_close(L); return 13; }
	const char *program =
	    "local p=require('proc'); "
	    "for i=1,64 do assert(p.spawn({'/usr/bin/true'})) end; "
	    "collectgarbage('collect'); "
	    "assert(p.spawn({helper,'mark','state-close-marker'})); "
	    "assert(p.spawn({'/bin/sleep','1'})); collectgarbage('collect')";
	/* Force a descriptor-exhaustion path in this test process only. */
	struct rlimit saved, limited;
	if (getrlimit(RLIMIT_NOFILE, &saved)) return 7;
	limited = saved; if (limited.rlim_cur > 16) limited.rlim_cur = 16;
	if (setrlimit(RLIMIT_NOFILE, &limited)) return 8;
	const char *exhaust = "local p=require('proc'); local failed=false; "
	    "for i=1,32 do local c,e=p.spawn({'/bin/sleep','1'}); "
	    "if not c then assert(e.code and e.kind=='resource'); failed=true; break end end; "
	    "assert(failed); assert(p.shutdown(0.01))";
	int exhausted = luaL_dostring(L, exhaust);
	if (setrlimit(RLIMIT_NOFILE, &saved)) return 9;
	if (exhausted != LUA_OK) {
		fprintf(stderr, "resource failure test failed\n"); lua_close(L); return 10;
	}
	if (luaL_dostring(L, program) != LUA_OK) {
		fprintf(stderr, "%s\n", lua_tostring(L, -1)); lua_close(L); return 2;
	}
	lua_close(L); /* Must not kill running children or strand their reaper. */
	struct timespec pause = {0, 20000000};
	for (int i = 0; i < 500 && fd_count() != before; i++) {
		struct timespec remaining = pause;
		while (nanosleep(&remaining, &remaining) < 0) if (errno != EINTR) return 3;
	}
	FILE *marker = fopen("state-close-marker", "r");
	if (!marker) { fprintf(stderr, "child died on state closure\n"); return 6; }
	fclose(marker);
	int status;
	if (waitpid(-1, &status, WNOHANG) >= 0 || errno != ECHILD) {
		fprintf(stderr, "unreaped or running child after state destruction\n"); return 4;
	}
	if (fd_count() != before) { fprintf(stderr, "descriptor leak\n"); return 5; }
	puts("PASS state destruction, reaping and descriptors"); return 0;
}
