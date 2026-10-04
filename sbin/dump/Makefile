#	dump.h			header file
#	itime.c			reads /etc/dumpdates
#	main.c			driver
#	optr.c			operator interface
#	tape.c			handles the mag tape and opening/closing
#	traverse.c		traverses the file system
#	unctime.c		undo ctime
#
#	DEBUG			use local directory to find ddate and dumpdates
#	TDEBUG			trace out the process forking

PACKAGE=ufs
PROG=	dump
CONFS=	/dev/null
CONFSGRP=	operator
CONFSMODE=	664
CONFSNAME_/dev/null=	dumpdates
SRCS=	itime.c main.c optr.c tape.c traverse.c unctime.c cache.c
LIBADD=	ufs
WARNS?=	2

.include <bsd.prog.mk>
