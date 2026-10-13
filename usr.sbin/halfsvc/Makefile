PACKAGE= runtime
.PATH: ${SRCTOP}/usr.sbin/halfinit
PROG= halfsvc
SRCS= launcher.c
MAN= halfsvc.8
LIBADD= halfbsd halfproc lua pthread m
CFLAGS+= -I${SRCTOP}/contrib/lua/src -I${SRCTOP}/lib/liblua -I${SRCTOP}/lib/libhalfbsd
CFLAGS.launcher.c+= -DHALF_ENTRY=\"client\"
LDFLAGS+= -Wl,-E
WARNS= 6
.include <bsd.prog.mk>
