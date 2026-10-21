PACKAGE= runtime
# Upstream uses strict C. GNU mode defines `unix`, which collides with the
# sockaddr union member used by both the daemon and client backend.
CSTD= c17
SEATDSRC= ${SRCTOP}/contrib/seatd
.PATH: ${SEATDSRC}/common ${SEATDSRC}/seatd
PROG= seatd
SRCS= seatd.c log.c linked_list.c terminal.c connection.c evdev.c hidraw.c \
	drm.c wscons.c poller.c seat.c client.c server.c
MAN= seatd.8
CFLAGS+= -I${SEATDSRC}/include -D_XOPEN_SOURCE=700 -D__BSD_VISIBLE=1 \
	-DSEATD_VERSION=\"0.9.3\" -DSEATD_DEFAULTPATH=\"/var/run/seatd.sock\" \
	-DSEATD_INSTALLPATH=\"/usr/sbin/seatd\"
# Imported code follows upstream's strict warnings; retain world hardening.
WARNS= 3
.include <bsd.prog.mk>
