PACKAGE= tests
SUBDIR+= accounts phase2 phase3 phase4 seatd
TESTSDIR= ${TESTSBASE}/halfbsd
ATF_TESTS_SH= runtime_test
PROGS= proc_helper ownership_helper
LIBADD.ownership_helper= halfbsd halfproc lua pthread m
CFLAGS.ownership_helper.c+= -I${SRCTOP}/contrib/lua/src -I${SRCTOP}/lib/liblua -I${SRCTOP}/lib/libhalfbsd
BINDIR= ${TESTSDIR}
MAN=
FILES= runtime.lua header_install.sh library_build.sh \
	run-userland.sh supervision.lua service_fixture.lua input-hotplug.sh input-hotplug.lua input-hotplug-test.lua
FILESDIR= ${TESTSDIR}
WARNS= 6
.include <bsd.test.mk>
