.PATH: ${.CURDIR}/../../include

# Select the Kerberos bootstrap headers according to the build options.
.include <src.opts.mk>

LIB=		egacy
SRC=
INCSGROUPS=	INCS SYSINCS CASPERINC UFSINCS FFSINCS MSDOSFSINCS DISKINCS
INCSGROUPS+=	MACHINESYSINCS
INCS=

SYSINCSDIR=	${INCLUDEDIR}/sys
CASPERINCDIR=	${INCLUDEDIR}/casper
# Also add ufs/ffs/msdosfs/disk headers to allow building makefs as a bootstrap tool
UFSINCSDIR=	${INCLUDEDIR}/ufs/ufs
FFSINCSDIR=	${INCLUDEDIR}/ufs/ffs
MSDOSFSINCSDIR=	${INCLUDEDIR}/fs/msdosfs
DISKINCSDIR=	${INCLUDEDIR}/sys/disk
MACHINESYSINCSDIR=	${INCLUDEDIR}/machine

BOOTSTRAPPING?=	0


.if ${.MAKE.OS} == "Darwin"
_XCODE_ROOT!=xcode-select -p
# since macOS 10.14 C headers are no longer installed in /usr but only
# provided via the SDK
.if ${_XCODE_ROOT} == "/Library/Developer/CommandLineTools"
# Only command line tools installed -> host headers are in the SDKs directory
_MACOS_SDK_DIR=${_XCODE_ROOT}/SDKs/MacOSX.sdk/
.else
# Full XCode installed -> host headers are below Platforms/MacOSX.platform
_MACOS_SDK_DIR=${_XCODE_ROOT}/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk
.endif
HOST_INCLUDE_ROOT=${_MACOS_SDK_DIR}/usr/include
.if !exists(${HOST_INCLUDE_ROOT}/stdio.h)
.error You must install the macOS SDK (try xcode-select --install)
.endif
.else
HOST_INCLUDE_ROOT=/usr/include
.endif

# Allow building libc-internal files (also on non-FreeBSD hosts)
CFLAGS+=	-I${.CURDIR}/libc-bootstrap
# Symbol versioning is not required for -legacy (and macOS bootstrap)
MK_SYMVER=	no

_WITH_PWCACHEDB!= grep -c pwcache_groupdb ${HOST_INCLUDE_ROOT}/grp.h || true
.if ${_WITH_PWCACHEDB} == 0
.PATH: ${.CURDIR}/../../contrib/libc-pwcache
CFLAGS.pwcache.c+=	-I${.CURDIR}/../../contrib/libc-pwcache
SRCS+=		pwcache.c
.endif

_WITH_STRSVIS!=	grep -c strsvis ${HOST_INCLUDE_ROOT}/vis.h 2>/dev/null || true
.if ${_WITH_STRSVIS} == 0
.PATH: ${.CURDIR}/../../contrib/libc-vis
INCS+=		vis.h
SRCS+=		vis.c unvis.c
CFLAGS.vis.c+=	-I${.CURDIR}/../../contrib/libc-vis -DHAVE_VIS=0 -DHAVE_SVIS=0
CFLAGS.unvis.c+=	-I${.CURDIR}/../../contrib/libc-vis -DHAVE_VIS=0 -DHAVE_SVIS=0
.endif

_WITH_REALLOCARRAY!= grep -c reallocarray ${HOST_INCLUDE_ROOT}/stdlib.h || true
.if ${_WITH_REALLOCARRAY} == 0
.PATH: ${.CURDIR}/../../lib/libc/stdlib
INCS+=		stdlib.h
SRCS+=		reallocarray.c
.endif

.if exists(${HOST_INCLUDE_ROOT}/sys/stat.h)
_WITH_UTIMENS!= grep -c utimensat ${HOST_INCLUDE_ROOT}/sys/stat.h || true
.else
_WITH_UTIMENS= 0
.endif
.if ${_WITH_UTIMENS} == 0
SYSINCS+=	stat.h
SRCS+=		futimens.c utimensat.c
.endif

_WITH_EXPLICIT_BZERO!= cat ${HOST_INCLUDE_ROOT}/strings.h ${HOST_INCLUDE_ROOT}/string.h | grep -c explicit_bzero || true
.if ${_WITH_EXPLICIT_BZERO} == 0
# .PATH: ${SRCTOP}/sys/libkern
# Adding sys/libkern to .PATH breaks building the cross-build compat library
# since that attempts to build strlcpy.c from libc and adding libkern here will
# cause it to pick the file from libkern instead (which won't compile).
# Avoid modifying .PATH by creating a copy in the build directory instead.
explicit_bzero.c: ${SRCTOP}/sys/libkern/explicit_bzero.c
	cp ${.ALLSRC} ${.TARGET}
CLEANFILES+=	explicit_bzero.c
INCS+=		strings.h
SRCS+=		explicit_bzero.c
.endif

_WITH_FSPACECTL!= grep -c fspacectl ${HOST_INCLUDE_ROOT}/fcntl.h || true
.if ${_WITH_FSPACECTL} == 0
INCS+=		fcntl.h
SRCS+=		fspacectl.c
.endif

.if exists(${HOST_INCLUDE_ROOT}/capsicum_helpers.h)
_WITH_CAPH_ENTER!= grep -c caph_enter ${HOST_INCLUDE_ROOT}/capsicum_helpers.h || true
_WITH_CAPH_RIGHTS_LIMIT!= grep -c caph_rights_limit ${HOST_INCLUDE_ROOT}/capsicum_helpers.h || true
.endif
.if !defined(_WITH_CAPH_ENTER) || ${_WITH_CAPH_ENTER} == 0 || ${_WITH_CAPH_RIGHTS_LIMIT} == 0
.PATH: ${SRCTOP}/lib/libcapsicum
INCS+=		capsicum_helpers.h
.PATH: ${SRCTOP}/lib/libcasper/libcasper
INCS+=		libcasper.h
.endif

.include <bsd.lib.mk>
