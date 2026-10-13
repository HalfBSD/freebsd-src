PACKAGE= runtime
# These foundation libraries are installed static archives.
# NO_PIC suppresses shared-library targets; archive objects still need PIC
# to link into the default PIE interpreter and other userland executables.
NO_PIC=
CFLAGS+= -fPIC
LIB= halfbsd
SRCS= common.c proc.c event.c os.c log.c ipc.c discovery.c init.c
INCS= halfbsd_lua.h
LIBADD= halfproc lua
CFLAGS+= -I${SRCTOP}/lib/libhalfproc -I${SRCTOP}/contrib/lua/src -I${SRCTOP}/lib/liblua
WARNS= 6
.include <bsd.lib.mk>
