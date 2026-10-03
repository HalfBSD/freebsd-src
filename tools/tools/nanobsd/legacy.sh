#!/bin/sh
#
# Copyright (c) 2005 Poul-Henning Kamp All rights reserved.
# Copyright (c) 2016 M. Warner Losh <imp@FreeBSD.org>
#
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.
#
#

# Media geometry, only relevant if bios doesn't understand LBA.
[ -n "$NANO_SECTS" ] || NANO_SECTS=63
[ -n "$NANO_HEADS" ] || NANO_HEADS=16

# Functions and variable definitions used by the legacy nanobsd
# image building system.

calculate_partitioning ( ) (
	echo $NANO_MEDIASIZE $NANO_IMAGES \
		$NANO_SECTS $NANO_HEADS \
		$NANO_CODESIZE $NANO_CONFSIZE $NANO_DATASIZE |
	awk '
	{
		# size of cylinder in sectors
		cs = $3 * $4

		# number of full cylinders on media
		cyl = int ($1 / cs)

		if ($7 > 0) {
			# size of data partition in full cylinders
			dsl = int (($7 + cs - 1) / cs)
		} else {
			dsl = 0;
		}

		# size of config partition in full cylinders
		csl = int (($6 + cs - 1) / cs)

		# size of image partition(s) in full cylinders
		if ($5 == 0) {
			isl = int ((cyl - dsl - csl) / $2)
		} else {
			isl = int (($5 + cs - 1) / cs)
		}

		# First image partition start at second track
		print $3, isl * cs - $3, 1
		c = isl * cs;

		# Second image partition (if any) also starts offset one
		# track to keep them identical.
		if ($2 > 1) {
			print $3 + c, isl * cs - $3, 2
			c += isl * cs;
		}

		# Config partition starts at cylinder boundary.
		print c, csl * cs, 3
		c += csl * cs

		# Data partition (if any) starts at cylinder boundary.
		if ($7 > 0) {
			print c, dsl * cs, 4
		} else if ($7 < 0 && $1 > c) {
			print c, $1 - c, 4
		} else if ($1 < c) {
			print "Disk space overcommitted by", \
			    c - $1, "sectors" > "/dev/stderr"
			exit 2
		}
	}
	' > ${NANO_LOG}/_.partitioning
)

create_code_slice ( ) (
	pprint 2 "build code slice"
	pprint 3 "log: ${NANO_OBJ}/_.cs"

	(
	IMG=${NANO_DISKIMGDIR}/_.disk.image
	MNT=${NANO_OBJ}/_.mnt
	mkdir -p ${MNT}
	CODE_SIZE=`head -n 1 ${NANO_LOG}/_.partitioning | awk '{ print $2 }'`

	if [ "${NANO_MD_BACKING}" = "swap" ] ; then
		MD=`mdconfig -a -t swap -s ${CODE_SIZE} -x ${NANO_SECTS} \
			-y ${NANO_HEADS}`
	else
		echo "Creating md backing file..."
		rm -f ${IMG}
		dd if=/dev/zero of=${IMG} seek=${CODE_SIZE} count=0
		MD=`mdconfig -a -t vnode -f ${IMG} -x ${NANO_SECTS} \
			-y ${NANO_HEADS}`
	fi

	trap "echo 'Running exit trap code' ; df -i ${MNT} ; umount ${MNT} || true ; mdconfig -d -u $MD" 1 2 15 EXIT

	gpart create -s bsd "${MD}"
	gpart add -t freebsd-ufs -b 16 "${MD}"
	gpart list ${MD}

	# Create first image
	populate_slice /dev/${MD}${NANO_PARTITION_ROOT} ${NANO_WORLDDIR} ${MNT} "${NANO_ROOT}"
	mount /dev/${MD}a ${MNT}
	echo "Generating mtree..."
	( cd ${MNT} && mtree -c ) > ${NANO_OBJ}/_.mtree
	( cd ${MNT} && du -k ) > ${NANO_OBJ}/_.du
	nano_umount ${MNT}

	if [ "${NANO_MD_BACKING}" = "swap" ] ; then
		echo "Writing out _.disk.image..."
		dd conv=sparse if=/dev/${MD} of=${NANO_DISKIMGDIR}/_.disk.image bs=64k
	fi
	mdconfig -d -u $MD

	trap - 1 2 15 EXIT

	) > ${NANO_OBJ}/_.cs 2>&1
)


create_diskimage ( ) (
	echo "HalfBSD NanoBSD legacy BIOS images are unsupported; use an embedded UEFI profile." >&2
	exit 1
)
