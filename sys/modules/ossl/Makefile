.PATH:	${SRCTOP}/sys/crypto/openssl
.PATH:	${SRCTOP}/sys/crypto/openssl/${MACHINE_CPUARCH}

KMOD=	ossl
OBJS+=	${OBJS.${MACHINE_CPUARCH}}
SRCS=	bus_if.h \
	cryptodev_if.h \
	device_if.h \
	ossl.c \
	ossl_aes.c \
	ossl_chacha20.c \
	ossl_poly1305.c \
	ossl_sha1.c \
	ossl_sha256.c \
	ossl_sha512.c \
	${SRCS.${MACHINE_CPUARCH}}

SRCS.amd64= \
	aes-gcm-avx512.S \
	aesni-x86_64.S \
	aesni-gcm-x86_64.S \
	chacha-x86_64.S \
	ghash-x86_64.S \
	poly1305-x86_64.S \
	sha1-x86_64.S \
	sha256-x86_64.S \
	sha512-x86_64.S \
	ossl_aes_gcm.c \
	ossl_aes_gcm_avx512.c \
	ossl_x86.c

SRCS.i386= \
	aesni-x86.S \
	chacha-x86.S \
	poly1305-x86.S \
	sha1-586.S \
	sha256-586.S \
	sha512-586.S \
	ossl_x86.c




CFLAGS+= -I${SRCTOP}/sys/crypto/openssl

.include <bsd.kmod.mk>
