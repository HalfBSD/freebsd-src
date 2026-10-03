SYSDIR?=${SRCTOP}/sys
.include "${SYSDIR}/conf/kern.opts.mk"

SUBDIR=	if_cxgbe
SUBDIR+=	if_cxl
SUBDIR+=	if_cc
SUBDIR+=	if_cxgbev
SUBDIR+=	if_cxlv
SUBDIR+=	if_ccv
SUBDIR+=	t4_firmware
SUBDIR+=	t5_firmware
SUBDIR+=	t6_firmware
SUBDIR+=	t7_firmware
SUBDIR+=	${_tom}
SUBDIR+=	ccr

.if ${MACHINE_CPUARCH} == "amd64" || ${MACHINE_CPUARCH} == "aarch64"
_tom=	tom
.endif

.if ${MACHINE_CPUARCH} == "i386"
_tom=	tom
.endif

.include <bsd.subdir.mk>
