PACKAGE= runtime
.PATH: ${SRCTOP}/contrib/lua/src
PROG= lua
SRCS= lua.c
MAN= lua.1 lua-halfbsd.7
LIBADD= halfbsd halfproc lua pthread m
CFLAGS+= -I${SRCTOP}/contrib/lua/src -I${SRCTOP}/lib/liblua -DLUA_USE_POSIX
CFLAGS.lua.c+= -DluaL_openlibs=halfbsd_openlibs
LDFLAGS+= -Wl,-E
.include <bsd.prog.mk>
