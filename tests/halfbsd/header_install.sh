#!/bin/sh
# SPDX-License-Identifier: BSD-2-Clause
# Source-only regression; no library compilation or root privileges needed.
# On Linux, pass an in-tree-built bmake as argument 2. On FreeBSD, use make.
set -eu
src=$(realpath "${1:?usage: header_install.sh source-tree [bsd-make]}")
bsd_make=${2:-make}
work=$(mktemp -d "${TMPDIR:-/tmp}/halfbsd-lua-headers.XXXXXX")
trap 'rm -rf "$work"' EXIT HUP INT TERM
mkdir "$work/obj"
owner=$(id -un)
group=$(id -gn)
install_headers()
{
    destination=$1
    shift
    MAKEOBJDIR="$work/obj" "$bsd_make" -m "$src/share/mk" \
        -C "$src/lib/liblua" -j8 installincludes \
        SRCTOP="$src" MACHINE=amd64 MACHINE_ARCH=amd64 \
        COMPILER_TYPE=gcc COMPILER_VERSION=130000 \
        INSTALL=install DESTDIR="$destination" INCLUDEDIR=/usr/include \
        INCSOWN="$owner" INCSGRP="$group" \
        INCSDIR_OWN="$owner" INCSDIR_GRP="$group" "$@"
    for header in lua.h lauxlib.h lualib.h luaconf.h; do
        cmp "$src/contrib/lua/src/$header" "$destination/usr/include/lua/$header"
    done
    cmp "$src/lib/liblua/luaconf.local.h" "$destination/usr/include/lua/luaconf.local.h"
}
# Each starts without any destination directory, reproducing legacy staging.
install_headers "$work/legacy" BOOTSTRAPPING=yes
install_headers "$work/world"
# A repeated parallel install must remain safe and leave all headers intact.
install_headers "$work/legacy" BOOTSTRAPPING=yes
printf '%s\n' 'PASS: fresh bootstrap, fresh world, and repeated parallel Lua header installation'
