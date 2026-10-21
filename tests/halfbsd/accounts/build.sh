#!/bin/sh
# SPDX-License-Identifier: BSD-2-Clause
set -eu
src=$(realpath "${1:?usage: build.sh source-tree bsd-make}")
bsd_make=${2:-make}
work=$(mktemp -d "${TMPDIR:-/tmp}/halfbsd-account-install.XXXXXX")
trap 'rm -rf "$work"' EXIT HUP INT TERM
mkdir "$work/obj"
for pass in 1 2; do
    MAKEOBJDIR="$work/obj" "$bsd_make" -m "$src/share/mk" -C "$src/usr.sbin/adduser" \
        SRCTOP="$src" MACHINE=amd64 MACHINE_ARCH=amd64 COMPILER_TYPE=gcc \
        COMPILER_VERSION=150000 SYSROOT="$work/sysroot" SHLIB_MAJOR=5 -j8 \
        INSTALL=install DESTDIR="$work/stage" BINOWN="$(id -un)" BINGRP="$(id -gn)" \
        ACCOUNTSOWN="$(id -un)" ACCOUNTSGRP="$(id -gn)" \
        ACCOUNTSDIR_OWN="$(id -un)" ACCOUNTSDIR_GRP="$(id -gn)" installfiles
    cmp "$src/usr.sbin/adduser/accounts.subr" "$work/stage/usr/share/halfbsd/accounts.subr"
    sh -n "$work/stage/usr/share/halfbsd/accounts.subr"
done
echo 'PASS fresh/repeated parallel shared account policy installation under staged SYSROOT'
