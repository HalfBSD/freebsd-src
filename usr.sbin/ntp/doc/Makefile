.include <src.opts.mk>

PACKAGE=ntp
MAN=	ntp.conf.5 ntp.keys.5
MAN+=	ntp-keygen.8 ntpd.8 ntpdate.8 ntpdc.8 ntpq.8 ntptime.8 sntp.8

CLEANFILES=	ntp.conf.5 ntp.keys.5
CLEANFILES+=	ntp-keygen.8 ntpd.8 ntpdc.8 ntpq.8 sntp.8

SUFFIXES8=	.1ntp-keygenmdoc \
		.1ntpdmdoc \
		.1ntpdcmdoc \
		.1ntpqmdoc \
		.1sntpmdoc

.SUFFIXES: .5 .8 .5mdoc ${SUFFIXES8}

.5mdoc.5:
	sed '/^\.Dt /s/5mdoc/5/' ${.ALLSRC} > ${.TARGET}

.for s in ${SUFFIXES8}
$s.8:
	sed '/^\.Dt /s/1[a-z-][a-z-]*/8/' ${.ALLSRC} > ${.TARGET}
.endfor

ntp.conf.5: ntp.conf.5mdoc

ntp.keys.5: ntp.keys.5mdoc

ntp-keygen.8: ntp-keygen.1ntp-keygenmdoc

ntpd.8:	ntpd.1ntpdmdoc

ntpdc.8: ntpdc.1ntpdcmdoc

ntpq.8: ntpq.1ntpqmdoc

sntp.8: sntp.1sntpmdoc

.PATH: ${SRCTOP}/contrib/ntp/util \
	${SRCTOP}/contrib/ntp/ntpd \
	${SRCTOP}/contrib/ntp/ntpdc \
	${SRCTOP}/contrib/ntp/ntpq \
	${SRCTOP}/contrib/ntp/ntpsnmpd \
	${SRCTOP}/contrib/ntp/sntp \
	${SRCTOP}/contrib/ntp/util

.include <bsd.prog.mk>
