PACKAGE=	locales
FILESGROUPS=	FILES ALWAYS
ALWAYSPACKAGE=	runtime
LOCALEDIR=	${SHAREDIR}/locale
FILESNAME=	LC_CTYPE
ALWAYSNAME=	LC_CTYPE
.SUFFIXES:	.src .LC_CTYPE
MAPLOC=		${.CURDIR}/../../tools/tools/locale/etc/final-maps

.include <bsd.endian.mk>
.include <src.opts.mk>

.src.LC_CTYPE:
	localedef ${LOCALEDEF_ENDIAN} -D -U -c -w ${MAPLOC}/widths.txt \
	-f ${MAPLOC}/map.${.IMPSRC:T:R:E} \
	-i ${.IMPSRC} ${.OBJDIR}/${.IMPSRC:T:R}

LOCALES+=	C.UTF-8
.if ${MK_LOCALES} != no

SAME+=		C.UTF-8 en_US.UTF-8
.endif

# C.UTF-8 is the default locale, so it should always been installed.
ALWAYS=		C.UTF-8.LC_CTYPE
FILES=		${LOCALES:NC.UTF-8:C/.+$/&.LC_CTYPE/}
CLEANFILES=	${FILES}

.for f t in ${SAME}
DIRS+=	LOCALEDIR_$t
LOCALEDIR_$t=	${LOCALEDIR}/$t
LOCALEDIR_$tPACKAGE=	locales
SYMLINKS+=	../$f/${FILESNAME} \
    ${LOCALEDIR}/$t/${FILESNAME}
.endfor

.for f in ${LOCALES}
FILESDIR_${f}.LC_CTYPE= ${LOCALEDIR}/${f}
FILESDIR_${f}.LC_CTYPEPACKAGE=	locales
.endfor
ALWAYSDIR_C.UTF-8.LC_CTYPE= ${LOCALEDIR}/C.UTF-8


.for s t in ${SYMPAIRS}
${t:S/src$/LC_CTYPE/}: $s
	localedef ${LOCALEDEF_ENDIAN} -D -U -c -w ${MAPLOC}/widths.txt \
	-f ${MAPLOC}/map.${.TARGET:T:R:C/^.*\.//} \
	-i ${.ALLSRC} ${.OBJDIR}/${.TARGET:T:R}
.endfor

.include <bsd.prog.mk>
