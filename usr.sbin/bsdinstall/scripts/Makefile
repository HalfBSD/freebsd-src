.include <bsd.compat.pre.mk>

SCRIPTS=auto \
	adduser \
	bootconfig \
	checksum \
	config \
	docsinstall \
	entropy \
	fetchmissingdists \
	finalconfig \
	firmware \
	hardening \
	hostname \
	jail \
	mirrorselect \
	mount \
	mount_aux \
	netconfig \
	pkgbase \
	rootpass \
	script \
	time \
	umount \
	zfsboot

FILES= config-halfbsd.lua
FILESDIR= ${LIBEXECDIR}/bsdinstall
BINDIR= ${LIBEXECDIR}/bsdinstall

pkgbase: pkgbase.in
	sed "s|%%_ALL_libcompats%%|${_ALL_libcompats}|" < ${.ALLSRC} > ${.TARGET}

.include <bsd.prog.mk>
