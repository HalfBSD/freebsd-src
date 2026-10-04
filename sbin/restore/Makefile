.PATH: ${.CURDIR:H}/dump

PACKAGE=ufs
PROG=	restore
SRCS=	main.c interactive.c restore.c dirs.c symtab.c tape.c utilities.c
WARNS?=	2
CFLAGS+= -D_ACL_PRIVATE

.include <bsd.prog.mk>
