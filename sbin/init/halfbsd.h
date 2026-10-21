/* SPDX-License-Identifier: BSD-2-Clause */
#ifndef INIT_HALFBSD_H
#define INIT_HALFBSD_H
#include <sys/types.h>
#include <stddef.h>
#define HB_BOOT_PROGRAM "/usr/sbin/halfinit"
#define HB_BOOT_CONFIG "/etc/halfbsd/system.lua"
#define HB_BOOT_ACTIVE "halfbsd.boot_active"
#define HB_BOOT_OVERRIDE "halfbsd.boot_mode"
#define HB_BOOT_STAGE "halfbsd.boot_stage"
#define HB_BOOT_RESULT "halfbsd.boot_result"
#define HB_BOOT_NETWORK "NETWORK\n"
#define HB_BOOT_SINGLE "SINGLE\n"
#define HB_BOOT_READY "READY\n"
#define HB_BOOT_FAIL "FAIL\n"
/* 0 explicit RC recovery, 1 default/explicit Lua, -1 invalid override. */
int hb_boot_mode(const char *);
/* 0 single-user, 1 full multiuser, 2 network inspection (default), -1 invalid. */
int hb_boot_stage(const char *);
void hb_boot_trace(const char *, const char *);
int hb_boot_pipe(int [2]);
int hb_boot_child_fd(int [2]);
int hb_boot_stdio(const char *, const char *);
/* 3 NETWORK, 2 SINGLE, 1 READY, 0 pending, -1 invalid/closed; caller retains and reaps its child. */
int hb_boot_read(int, char [16], size_t *);
int hb_boot_wait(pid_t, int, int *);
int hb_boot_stop(pid_t, int, int *);
#endif
