PACKAGE=	locales
LOCALEDIR=	${SHAREDIR}/locale
FILESNAME=	LC_COLLATE
.SUFFIXES:	.src .LC_COLLATE
MAPLOC=		${.CURDIR}/../../tools/tools/locale/etc/final-maps

CLDR_VERSION=	"34.0"

.include <bsd.endian.mk>

.src.LC_COLLATE:
	localedef ${LOCALEDEF_ENDIAN} -D -U -i ${.IMPSRC} \
	-V ${CLDR_VERSION} \
	-f ${MAPLOC}/map.${.TARGET:T:R:E:C/@.*//} ${.OBJDIR}/${.IMPSRC:T:R}




FILES=		${LOCALES:S/$/.LC_COLLATE/}
CLEANFILES=	${FILES}

.for f t in ${SAME}
DIRS+=	LOCALEDIR_$t
LOCALEDIR_${t}PACKAGE=	locales
LOCALEDIR_$t=	${LOCALEDIR}/$t
SYMLINKS+=	../$f/${FILESNAME} \
    ${LOCALEDIR}/$t/${FILESNAME}
.endfor

.for f in ${LOCALES}
FILESDIR_${f}.LC_COLLATE= ${LOCALEDIR}/${f}
FILESDIR_${f}.LC_COLLATEPACKAGE=	locales
.endfor

.for f t in ${LOCALES_MAPPED}
FILES+=	$t.LC_COLLATE
FILESDIR_$t.LC_COLLATE=	${LOCALEDIR}/$t
FILESDIR_${t}.LC_COLLATEPACKAGE=	locales
$t.LC_COLLATE: ${.CURDIR}/$f.src
	localedef ${LOCALEDEF_ENDIAN} -D -U -i ${.ALLSRC} \
	-V ${CLDR_VERSION} \
		-f ${MAPLOC}/map.${.TARGET:T:R:E:C/@.*//} \
		${.OBJDIR}/${.TARGET:T:R}
.endfor

.include <bsd.prog.mk>
