/* SPDX-License-Identifier: BSD-2-Clause */
/* Shared legacy syscall declarations retained for FreeBSD32 only.
 * Argument layouts are copied from syscalls.master; keep their ABI unchanged.
 * Included by sysproto.h while its PAD macros are defined. */
#ifndef _FREEBSD32_LEGACY_PROTO_H_
#define _FREEBSD32_LEGACY_PROTO_H_
#ifdef COMPAT_FREEBSD32
#ifdef COMPAT_FREEBSD32_43

struct ocreat_args {
	char path_l_[PADL_(const char *)]; const char * path; char path_r_[PADR_(const char *)];
	char mode_l_[PADL_(int)]; int mode; char mode_r_[PADR_(int)];
};
struct osigprocmask_args {
	char how_l_[PADL_(int)]; int how; char how_r_[PADR_(int)];
	char mask_l_[PADL_(osigset_t)]; osigset_t mask; char mask_r_[PADR_(osigset_t)];
};
struct ogethostname_args {
	char hostname_l_[PADL_(char *)]; char * hostname; char hostname_r_[PADR_(char *)];
	char len_l_[PADL_(u_int)]; u_int len; char len_r_[PADR_(u_int)];
};
struct osethostname_args {
	char hostname_l_[PADL_(char *)]; char * hostname; char hostname_r_[PADR_(char *)];
	char len_l_[PADL_(u_int)]; u_int len; char len_r_[PADR_(u_int)];
};
struct oaccept_args {
	char s_l_[PADL_(int)]; int s; char s_r_[PADR_(int)];
	char name_l_[PADL_(struct sockaddr *)]; struct sockaddr * name; char name_r_[PADR_(struct sockaddr *)];
	char anamelen_l_[PADL_(__socklen_t *)]; __socklen_t * anamelen; char anamelen_r_[PADR_(__socklen_t *)];
};
struct osend_args {
	char s_l_[PADL_(int)]; int s; char s_r_[PADR_(int)];
	char buf_l_[PADL_(const void *)]; const void * buf; char buf_r_[PADR_(const void *)];
	char len_l_[PADL_(int)]; int len; char len_r_[PADR_(int)];
	char flags_l_[PADL_(int)]; int flags; char flags_r_[PADR_(int)];
};
struct orecv_args {
	char s_l_[PADL_(int)]; int s; char s_r_[PADR_(int)];
	char buf_l_[PADL_(void *)]; void * buf; char buf_r_[PADR_(void *)];
	char len_l_[PADL_(int)]; int len; char len_r_[PADR_(int)];
	char flags_l_[PADL_(int)]; int flags; char flags_r_[PADR_(int)];
};
struct osigblock_args {
	char mask_l_[PADL_(int)]; int mask; char mask_r_[PADR_(int)];
};
struct osigsetmask_args {
	char mask_l_[PADL_(int)]; int mask; char mask_r_[PADR_(int)];
};
struct osigsuspend_args {
	char mask_l_[PADL_(osigset_t)]; osigset_t mask; char mask_r_[PADR_(osigset_t)];
};
struct orecvfrom_args {
	char s_l_[PADL_(int)]; int s; char s_r_[PADR_(int)];
	char buf_l_[PADL_(void *)]; void * buf; char buf_r_[PADR_(void *)];
	char len_l_[PADL_(size_t)]; size_t len; char len_r_[PADR_(size_t)];
	char flags_l_[PADL_(int)]; int flags; char flags_r_[PADR_(int)];
	char from_l_[PADL_(struct sockaddr *)]; struct sockaddr * from; char from_r_[PADR_(struct sockaddr *)];
	char fromlenaddr_l_[PADL_(__socklen_t *)]; __socklen_t * fromlenaddr; char fromlenaddr_r_[PADR_(__socklen_t *)];
};
struct ogetpeername_args {
	char fdes_l_[PADL_(int)]; int fdes; char fdes_r_[PADR_(int)];
	char asa_l_[PADL_(struct sockaddr *)]; struct sockaddr * asa; char asa_r_[PADR_(struct sockaddr *)];
	char alen_l_[PADL_(__socklen_t *)]; __socklen_t * alen; char alen_r_[PADR_(__socklen_t *)];
};
struct ogetrlimit_args {
	char which_l_[PADL_(u_int)]; u_int which; char which_r_[PADR_(u_int)];
	char rlp_l_[PADL_(struct orlimit *)]; struct orlimit * rlp; char rlp_r_[PADR_(struct orlimit *)];
};
struct osetrlimit_args {
	char which_l_[PADL_(u_int)]; u_int which; char which_r_[PADR_(u_int)];
	char rlp_l_[PADL_(struct orlimit *)]; struct orlimit * rlp; char rlp_r_[PADR_(struct orlimit *)];
};
struct okillpg_args {
	char pgid_l_[PADL_(int)]; int pgid; char pgid_r_[PADR_(int)];
	char signum_l_[PADL_(int)]; int signum; char signum_r_[PADR_(int)];
};
struct ogetsockname_args {
	char fdes_l_[PADL_(int)]; int fdes; char fdes_r_[PADR_(int)];
	char asa_l_[PADL_(struct sockaddr *)]; struct sockaddr * asa; char asa_r_[PADR_(struct sockaddr *)];
	char alen_l_[PADL_(__socklen_t *)]; __socklen_t * alen; char alen_r_[PADR_(__socklen_t *)];
};
int	ocreat(struct thread *, struct ocreat_args *);
int	osigprocmask(struct thread *, struct osigprocmask_args *);
int	osigpending(struct thread *, struct osigpending_args *);
int	ogetpagesize(struct thread *, struct ogetpagesize_args *);
int	owait(struct thread *, struct owait_args *);
int	ogethostname(struct thread *, struct ogethostname_args *);
int	osethostname(struct thread *, struct osethostname_args *);
int	oaccept(struct thread *, struct oaccept_args *);
int	osend(struct thread *, struct osend_args *);
int	orecv(struct thread *, struct orecv_args *);
int	osigblock(struct thread *, struct osigblock_args *);
int	osigsetmask(struct thread *, struct osigsetmask_args *);
int	osigsuspend(struct thread *, struct osigsuspend_args *);
int	orecvfrom(struct thread *, struct orecvfrom_args *);
int	ogetpeername(struct thread *, struct ogetpeername_args *);
int	ogethostid(struct thread *, struct ogethostid_args *);
int	ogetrlimit(struct thread *, struct ogetrlimit_args *);
int	osetrlimit(struct thread *, struct osetrlimit_args *);
int	okillpg(struct thread *, struct okillpg_args *);
int	oquota(struct thread *, struct oquota_args *);
int	ogetsockname(struct thread *, struct ogetsockname_args *);


#endif

struct freebsd4_getdomainname_args {
	char domainname_l_[PADL_(char *)]; char * domainname; char domainname_r_[PADR_(char *)];
	char len_l_[PADL_(int)]; int len; char len_r_[PADR_(int)];
};
struct freebsd4_setdomainname_args {
	char domainname_l_[PADL_(char *)]; char * domainname; char domainname_r_[PADR_(char *)];
	char len_l_[PADL_(int)]; int len; char len_r_[PADR_(int)];
};
struct freebsd4_uname_args {
	char name_l_[PADL_(struct utsname *)]; struct utsname * name; char name_r_[PADR_(struct utsname *)];
};
int	freebsd4_getdomainname(struct thread *, struct freebsd4_getdomainname_args *);
int	freebsd4_setdomainname(struct thread *, struct freebsd4_setdomainname_args *);
int	freebsd4_uname(struct thread *, struct freebsd4_uname_args *);




struct freebsd7___semctl_args {
	char semid_l_[PADL_(int)]; int semid; char semid_r_[PADR_(int)];
	char semnum_l_[PADL_(int)]; int semnum; char semnum_r_[PADR_(int)];
	char cmd_l_[PADL_(int)]; int cmd; char cmd_r_[PADR_(int)];
	char arg_l_[PADL_(union semun_old *)]; union semun_old * arg; char arg_r_[PADR_(union semun_old *)];
};
struct freebsd7_msgctl_args {
	char msqid_l_[PADL_(int)]; int msqid; char msqid_r_[PADR_(int)];
	char cmd_l_[PADL_(int)]; int cmd; char cmd_r_[PADR_(int)];
	char buf_l_[PADL_(struct msqid_ds_old *)]; struct msqid_ds_old * buf; char buf_r_[PADR_(struct msqid_ds_old *)];
};
struct freebsd7_shmctl_args {
	char shmid_l_[PADL_(int)]; int shmid; char shmid_r_[PADR_(int)];
	char cmd_l_[PADL_(int)]; int cmd; char cmd_r_[PADR_(int)];
	char buf_l_[PADL_(struct shmid_ds_old *)]; struct shmid_ds_old * buf; char buf_r_[PADR_(struct shmid_ds_old *)];
};
int	freebsd7___semctl(struct thread *, struct freebsd7___semctl_args *);
int	freebsd7_msgctl(struct thread *, struct freebsd7_msgctl_args *);
int	freebsd7_shmctl(struct thread *, struct freebsd7_shmctl_args *);


int	freebsd10_pipe(struct thread *, struct freebsd10_pipe_args *);


#endif
#endif
