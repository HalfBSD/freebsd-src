#!/bin/sh
# SPDX-License-Identifier: BSD-2-Clause
# Exercise actual publication rules without installing world or changing /etc.
set -eu
src=$(realpath "${1:?source tree required}")
bsd_make=${2:-make}
work=$(mktemp -d "${TMPDIR:-/tmp}/halfbsd-config-install.XXXXXX")
trap 'rm -rf "$work"' EXIT HUP INT TERM
mkdir "$work/obj"
seed()
{
    MAKEOBJDIR="$work/obj" "$bsd_make" -m "$src/share/mk" -C "$src/usr.sbin/halfinit" \
        SRCTOP="$src" MACHINE=amd64 MACHINE_ARCH=amd64 COMPILER_TYPE=gcc COMPILER_VERSION=130000 \
        INSTALL=install BINOWN="$(id -un)" BINGRP="$(id -gn)" DESTDIR="$work/stage" install-boot-config
}
seed
cmp "$src/usr.sbin/halfinit/examples/boot-system.lua" "$work/stage/etc/halfbsd/system.lua"
printf '%s\n' "return {console={user='charlie'},profile='maintenance',boot={}}" > "$work/stage/etc/halfbsd/system.lua"
cp "$work/stage/etc/halfbsd/system.lua" "$work/expected"
seed & first=$!
seed & second=$!
wait "$first"; wait "$second"
cmp "$work/expected" "$work/stage/etc/halfbsd/system.lua"
echo 'PASS empty DESTDIR config publication and parallel upgrades preserve administrator console/profile policy'
