PACKAGE= runtime
.PATH: ${SRCTOP}/usr.sbin/halfinit
PROG= halfconsole
SRCS= console.c console_input.c
MAN= halfconsole.8
LIBADD= halfbsd halfproc lua pthread m
CSTD= c17
CFLAGS+= -D_POSIX_C_SOURCE=200809L -I${SRCTOP}/contrib/lua/src -I${SRCTOP}/lib/liblua -I${SRCTOP}/lib/libhalfbsd
WARNS= 6
.include <bsd.prog.mk>
