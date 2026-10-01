SRCS+=	amd64_tramp.S \
	start.S \
	setup.c \
	elf64_freebsd.c

.PATH: ${BOOTSRC}/common
SRCS+=	nullconsole.c \
	comconsole.c \
	spinconsole.c
