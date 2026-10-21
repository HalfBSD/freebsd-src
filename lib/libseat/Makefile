PACKAGE= runtime
SEATDSRC= ${SRCTOP}/contrib/seatd
.PATH: ${SEATDSRC}/common ${SEATDSRC}/libseat ${SEATDSRC}/libseat/backend
LIB= seat
SHLIB_MAJOR= 1
# Upstream's public ABI is 1 even if an embedding build exports SHLIB_MAJOR.
SHLIB_NAME= libseat.so.1
SRCS= libseat.c noop.c seatd.c connection.c linked_list.c log.c
INCS= ${SEATDSRC}/include/libseat.h
INCSDIR= ${INCLUDEDIR}
DIRS+= INCSDIR
# Keep the installed static archive usable by PIE consumers too.
CFLAGS+= -fPIC -I${SEATDSRC}/include -D_XOPEN_SOURCE=700 -D__BSD_VISIBLE=1 \
	-DLIBSEAT=1 -DSEATD_ENABLED=1 -DSEATD_VERSION=\"0.9.3\" \
	-DSEATD_DEFAULTPATH=\"/var/run/seatd.sock\" \
	-DSEATD_INSTALLPATH=\"/usr/sbin/seatd\"
LDFLAGS+= -Wl,--version-script=${SEATDSRC}/libseat/libseat.syms
WARNS= 3
FILESGROUPS+= SEATPC
SEATPC= libseat.pc
SEATPCDIR= ${LIBDATADIR}/pkgconfig
SEATPCOWN= ${LIBOWN}
SEATPCGRP= ${LIBGRP}
CLEANFILES+= libseat.pc
libseat.pc: libseat.pc.in
	sed -e 's,@LIBDIR@,${LIBDIR},g; s,@INCLUDEDIR@,${INCLUDEDIR},g' \
	    ${.ALLSRC} > ${.TARGET}
.include <bsd.lib.mk>

# installincludes is also used directly during bootstrap staging.
# .ALLSRC in _INCSINS must contain headers only, so order rather than add
# the directory-creation target to that batch install's prerequisites.
.if ${MK_INCLUDES} != "no"
installincludes: installdirs-INCSDIR
.ORDER: installdirs-INCSDIR _INCSINS
.endif
