.include <src.opts.mk>

PACKAGE=	lib${LIB}
LIB=		compiler_rt
MK_SSP=		no
NO_PIC=
WARNS?=		2

CFLAGS+=	${PICFLAG}
CFLAGS+=	-fvisibility=hidden
CFLAGS+=	-DVISIBILITY_HIDDEN
CFLAGS+=	-I${SRCTOP}/contrib/llvm-project/libunwind/include

# gcc has incompatible internal declarations for __divtc3 and __multc3, but has
# no option to silence its warning, so make warnings non-fatal.
MK_WERROR.gcc=	no

.include "Makefile.inc"

# Out-of-line LSE atomics helpers for aarch64

.if ${MK_INSTALLLIB} != "no"
SYMLINKS+=	libcompiler_rt.a ${LIBDIR}/libgcc.a
.endif

.include <bsd.lib.mk>
