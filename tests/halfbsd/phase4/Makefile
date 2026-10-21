PACKAGE= tests
TESTSDIR= ${TESTSBASE}/halfbsd/phase4
ATF_TESTS_SH= boot_test
PROGS= handoff_test console_input_test
SRCS.handoff_test= handoff_test.c halfbsd.c
.PATH: ${SRCTOP}/sbin/init
.PATH: ${SRCTOP}/usr.sbin/halfconsole
SRCS.console_input_test= console_input_test.c console_input.c
CFLAGS.console_input_test+= -D_POSIX_C_SOURCE=200809L -I${SRCTOP}/usr.sbin/halfconsole
LIBADD.console_input_test= util
CFLAGS.handoff_test+= -I${SRCTOP}/sbin/init
BINDIR= ${TESTSDIR}
MAN=
testsFILES+= singleboot.lua policy.lua workstation.lua service_test.sh workstation-live.sh build-console.sh config_install.sh run.sh vm-cycles.sh vm-ready.lua check-live.lua
WARNS= 6
.include <bsd.test.mk>
