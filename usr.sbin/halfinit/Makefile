PACKAGE= runtime
PROG= halfinit
SRCS= launcher.c
MAN= halfinit.8
LIBADD= halfbsd halfproc lua pthread m
CFLAGS+= -I${SRCTOP}/contrib/lua/src -I${SRCTOP}/lib/liblua -I${SRCTOP}/lib/libhalfbsd
LDFLAGS+= -Wl,-E
FILESGROUPS= MODULES DEFINITIONS EXAMPLES
MODULES= modules/init.lua modules/client.lua modules/service.lua modules/config.lua \
	modules/dependency.lua modules/supervisor.lua modules/control.lua modules/json.lua \
	modules/providers.lua modules/fixture.lua
MODULESDIR= /usr/lib/halfbsd
DEFINITIONS= services/system.lua
DEFINITIONSDIR= /usr/lib/halfbsd/services
EXAMPLES= examples/services.lua
EXAMPLESDIR= /usr/share/examples/halfbsd
WARNS= 6
.include <bsd.prog.mk>
