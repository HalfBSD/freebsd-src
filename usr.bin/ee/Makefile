.PATH:	${SRCTOP}/contrib/ee

CFLAGS+= -DHAS_NCURSES -DHAS_UNISTD -DHAS_STDARG -DHAS_STDLIB \
	 -DHAS_SYS_WAIT

PACKAGE=ee
PROG=	ee
LINKS=	${BINDIR}/ee ${BINDIR}/ree ${BINDIR}/ee ${BINDIR}/edit
MLINKS=	ee.1 ree.1 ee.1 edit.1
LIBADD=	tinfow ncursesw

WARNS?=	2


NLSLINKS_pt_BR.ISO8859-1= pt_PT.ISO8859-1

NLSSRCFILES=ee.msg
.for lang in ${NLS}
. if exists(${.CURDIR}/nls/${lang}/ee.msg)
NLSSRCDIR_${lang}= ${.CURDIR}/nls/${lang}
. else
NLSSRCDIR_${lang}= ${SRCTOP}/contrib/ee
. endif
.endfor

.include <bsd.prog.mk>
