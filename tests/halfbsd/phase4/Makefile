PACKAGE= tests
TESTSDIR= ${TESTSBASE}/halfbsd/phase4
ATF_TESTS_SH= boot_test
PROGS= handoff_test shutdown_report_test
SRCS.handoff_test= handoff_test.c halfbsd.c
.PATH: ${SRCTOP}/sbin/init
CFLAGS.handoff_test+= -I${SRCTOP}/sbin/init
SRCS.shutdown_report_test= shutdown_report_test.c shutdown_report.c halfbsd.c
CFLAGS.shutdown_report_test+= -I${SRCTOP}/sbin/init
BINDIR= ${TESTSDIR}
MAN=
testsFILES+= singleboot.lua policy.lua workstation.lua shutdown_report.lua service_test.sh workstation-live.sh build-console.sh config_install.sh run.sh vm-cycles.sh vm-ready.lua check-live.lua
WARNS= 6
.include <bsd.test.mk>
