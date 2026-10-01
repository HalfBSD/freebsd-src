.include <src.opts.mk>

PROG=		gh-bc
PROGNAME=	bc

BCDIR=		${SRCTOP}/contrib/${PROGNAME}
VERSION!=	cat ${SRCTOP}/contrib/${PROGNAME}/VERSION.txt

SRCS=		args.c bc.c bc_lex.c bc_parse.c data.c dc.c dc_lex.c dc_parse.c file.c history.c
SRCS+=		lang.c lex.c main.c num.c opt.c parse.c program.c rand.c read.c vector.c  vm.c
SRCS+=		bc_help.c dc_help.c lib.c lib2.c

LIBADD=		edit

MAN=		bc.1 dc.1

LINKS=		${BINDIR}/bc ${BINDIR}/dc

.PATH:		${BCDIR}/src ${BCDIR}/gen ${BCDIR}/manuals ${.OBJDIR}

CATALOGS=	en_US.UTF-8

NLSNAME=	bc
NLSSRCDIR=	${BCDIR}/locales

CFLAGS+=	-I${BCDIR}/include

CFLAGS+=	-DBC_DEFAULT_BANNER=0
CFLAGS+=	-DBC_DEFAULT_DIGIT_CLAMP=0
CFLAGS+=	-DBC_DEFAULT_EXPR_EXIT=1
CFLAGS+=	-DBC_DEFAULT_PROMPT=0
CFLAGS+=	-DBC_DEFAULT_SIGINT_RESET=1
CFLAGS+=	-DBC_DEFAULT_TTY_MODE=1
CFLAGS+=	-DBC_ENABLED=1
CFLAGS+=	-DBC_ENABLE_AFL=0
CFLAGS+=	-DBC_ENABLE_EDITLINE=1
CFLAGS+=	-DBC_ENABLE_EXTRA_MATH=1
CFLAGS+=	-DBC_ENABLE_HISTORY=1
CFLAGS+=	-DBC_ENABLE_LIBRARY=0
CFLAGS+=	-DBC_ENABLE_MEMCHECK=0
CFLAGS+=	-DBC_EXCLUDE_EXTRA_MATH=0

CFLAGS+=	-DDC_DEFAULT_DIGIT_CLAMP=0
CFLAGS+=	-DDC_DEFAULT_EXPR_EXIT=1
CFLAGS+=	-DDC_DEFAULT_PROMPT=0
CFLAGS+=	-DDC_DEFAULT_SIGINT_RESET=1
CFLAGS+=	-DDC_DEFAULT_TTY_MODE=0
CFLAGS+=	-DDC_ENABLED=1

CFLAGS+=	-DBUILD_TYPE=A
CFLAGS+=	-DMAINEXEC=${PROGNAME}
CFLAGS+=	-DNDEBUG
CFLAGS+=	-DNLSPATH=/usr/share/nls/%L/%N.cat

CFLAGS+=	-DVERSION=${VERSION}

# prevent floating point incompatibilities caused by -flto on some architectures
.if ${MACHINE_ARCH} != riscv64
CFLAGS+=	-flto
.endif

HAS_TESTS=	yes
SUBDIR.${MK_TESTS}+= tests

.if ${MK_NLS_CATALOGS} == "no"
CFLAGS+=	-DBC_ENABLE_NLS=0
MAN_SRC_BC=	bc/N.1
MAN_SRC_DC=	dc/N.1
.else
CFLAGS+=	-DBC_ENABLE_NLS=1
MAN_SRC_BC=	bc/A.1
MAN_SRC_DC=	dc/A.1

.for catalog in ${CATALOGS}
NLS+=           ${catalog:C/.*://}
NLSSRCFILES_${catalog:C/.*://}= ${catalog:C/.*://}.msg
.endfor

NLSLINKS_en_US.UTF-8+=	C
.endif

lib.c:		lib.bc Makefile
		cd ${BCDIR} && sh gen/strgen.sh gen/lib.bc ${.OBJDIR}/lib.c 0 bc_lib bc_lib_name "" 1

lib2.c:		lib2.bc Makefile
		cd ${BCDIR} && sh gen/strgen.sh gen/lib2.bc ${.OBJDIR}/lib2.c 0 bc_lib2 bc_lib2_name "" 1

bc_help.c:	bc_help.txt Makefile
		cd ${BCDIR} && sh gen/strgen.sh gen/bc_help.txt ${.OBJDIR}/bc_help.c 0 bc_help "" 1 0

dc_help.c:	dc_help.txt Makefile
		cd ${BCDIR} && sh gen/strgen.sh gen/dc_help.txt ${.OBJDIR}/dc_help.c 0 dc_help "" 1 0

bc.1:
		${CP} ${BCDIR}/manuals/${MAN_SRC_BC} ${.OBJDIR}/bc.1

dc.1:
		${CP} ${BCDIR}/manuals/${MAN_SRC_DC} ${.OBJDIR}/dc.1

.include <bsd.prog.mk>
