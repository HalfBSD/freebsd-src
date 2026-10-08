PACKAGE=	locales
LOCALEDIR=	${SHAREDIR}/locale
FILESNAME=	LC_TIME
.SUFFIXES:	.src .out

.src.out:
	grep -v -E '^(#$$|#[ ])' < ${.IMPSRC} > ${.TARGET}

LOCALES=




FILES=		${LOCALES:C/.+$/&.out/}
CLEANFILES=	${FILES}

.for f t in ${SAME}
DIRS+=		LOCALEDIR_$t
LOCALEDIR_$t=	${LOCALEDIR}/$t
LOCALEDIR_$tPACKAGE=	locales
SYMLINKS+=	../$f/${FILESNAME} \
    ${LOCALEDIR}/$t/${FILESNAME}
.endfor

.for f in ${LOCALES}
FILESDIR_${f}.out= ${LOCALEDIR}/${f}
FILESDIR_${f}.outPACKAGE=	locales
.endfor

.include <bsd.prog.mk>
