PACKAGE=	iscsi
PROG=		iscsid
SRCS=		discovery.c iscsid.c login.c
CFLAGS+=	-I${.CURDIR}
CFLAGS+=	-I${SRCTOP}/sys/cam
CFLAGS+=	-I${SRCTOP}/sys/dev/iscsi
CFLAGS+=	-I${SRCTOP}/lib/libiscsiutil

LIBADD=		iscsiutil util

.include <bsd.prog.mk>
