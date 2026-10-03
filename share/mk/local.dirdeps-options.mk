
# avoid duplication
DIRDEPS.AUDIT.yes= lib/libbsm
DIRDEPS.CASPER.yes+= lib/libcasper/libcasper
DIRDEPS.JAIL.yes+= lib/libjail
DIRDEPS.OPENSSL.yes+= secure/lib/libcrypto
DIRDEPS.OPENSSL.no+= lib/libmd

MK_FDT.${DEP_MACHINE} ?= yes

.-include <site.dirdeps-options.mk>
