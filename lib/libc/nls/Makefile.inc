#	from $NetBSD: Makefile.inc,v 1.7 1995/02/27 13:06:20 cgd Exp $

.PATH: ${LIBC_SRCTOP}/nls

SRCS+=	msgcat.c

SYM_MAPS+=${LIBC_SRCTOP}/nls/Symbol.map

# NOTE: C.msg should not be processed here, it's used as a template
#       for translators.

NLSNAME=	libc
# We don't want libc's NLS catalogues to be installed in the clibs package.
# Put them in locales instead, since anyone interested in NLS will have
# that installed.
NLSPACKAGE=	locales

NLS+=	be_BY.UTF-8
NLS+=	ca_ES.ISO8859-1
NLS+=	gl_ES.ISO8859-1
NLS+=	no_NO.ISO8859-1
