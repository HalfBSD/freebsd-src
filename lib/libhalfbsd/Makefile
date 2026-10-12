PACKAGE= runtime
LIB= halfbsd
SRCS= common.c proc.c event.c os.c log.c init.c
INCS= halfbsd_lua.h
LIBADD= halfproc lua
CFLAGS+= -I${SRCTOP}/lib/libhalfproc -I${SRCTOP}/contrib/lua/src -I${SRCTOP}/lib/liblua
WARNS= 6
.include <bsd.lib.mk>
