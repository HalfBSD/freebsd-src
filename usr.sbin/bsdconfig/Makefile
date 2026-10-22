.include <src.opts.mk>

PACKAGE=	bsdconfig

SUBDIR=	diskmgmt \
	docsinstall \
	dot \
	include \
	includes \
	packages \
	password \
	share \
	timezone \
	ttys \
	usermgmt

.if ${MK_EXAMPLES} != "no"
SUBDIR+= examples
.endif

FILESDIR=	${LIBEXECDIR}/bsdconfig
FILES=		USAGE

SCRIPTS= bsdconfig

.include <bsd.prog.mk>
