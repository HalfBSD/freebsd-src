PACKAGE= tests
TESTSDIR= ${TESTSBASE}/halfbsd/phase3
ATF_TESTS_SH= adapters_test
PROGS= probe_helper terminal_helper
LIBADD.terminal_helper= halfproc pthread
CFLAGS.terminal_helper.c+= -I${SRCTOP}/lib/libhalfproc
BINDIR= ${TESTSDIR}
MAN=
FILES= policy.lua native.lua run.sh probe_helper.c terminal_helper.c
FILESDIR= ${TESTSDIR}
WARNS= 6
.include <bsd.test.mk>
