SRCS+=	amd64_tramp.S \
	start.S \
	setup.c \
	elf64_freebsd.c

.PATH: ${.CURDIR}/../loader/arch/x86
SRCS+=	nullconsole.c \
	comconsole.c \
	spinconsole.c
