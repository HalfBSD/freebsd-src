CONFGROUPS=	CONFTTYS
PACKAGE=runtime
PROG=	init
SRCS= init.c halfbsd.c shutdown_report.c
PRECIOUSPROG=
INSTALLFLAGS=-b -B.bak
CFLAGS+=-DLOGIN_CAP -DCOMPAT_SYSV_INIT
LIBADD=	util

CONFTTYSNAME=	ttys
CONFTTYS+=	ttys

NO_SHARED?=	YES

.include <bsd.prog.mk>
