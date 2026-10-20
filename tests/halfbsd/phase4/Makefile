PACKAGE= tests
TESTSDIR= ${TESTSBASE}/halfbsd/phase4
ATF_TESTS_SH= boot_test
PROGS= handoff_test
SRCS.handoff_test= handoff_test.c halfbsd.c
.PATH: ${SRCTOP}/sbin/init
CFLAGS.handoff_test+= -I${SRCTOP}/sbin/init
BINDIR= ${TESTSDIR}
MAN=
FILES= singleboot.lua policy.lua run.sh vm-cycles.sh vm-ready.lua
FILESDIR= ${TESTSDIR}
WARNS= 6
.include <bsd.test.mk>
