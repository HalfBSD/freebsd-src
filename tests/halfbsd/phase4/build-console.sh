#!/bin/sh
# SPDX-License-Identifier: BSD-2-Clause
# Native-only full helper link and repeated installation into private DESTDIR.
set -eu
[ "$(uname -s)" = FreeBSD ] || { echo 'Native FreeBSD required' >&2; exit 1; }
src=${1:?source tree required}
work=$(mktemp -d "${TMPDIR:-/tmp}/halfbsd-console-build.XXXXXX")
trap 'rm -rf "$work"' EXIT HUP INT TERM
mkdir "$work/obj" "$work/stage"
# These existing standard directories are normally supplied by installworld's
# mtree pass; the helper introduces no new installation directory.
mkdir -p "$work/stage/usr/sbin" "$work/stage/usr/share/man/man8"
export MAKEOBJDIR="$work/obj"
owner=$(id -un); group=$(id -gn)
# Reuse the actual completed world archives, including Lua's PIE variant.
# Override for nonstandard MAKEOBJDIRPREFIX/buildworld layouts.
arch=$(uname -p)
objtop=${HALFBSD_OBJTOP:-/usr/obj$src/$arch.$arch}
for archive in lib/liblua/liblua_pie.a lib/libhalfbsd/libhalfbsd.a lib/libhalfproc/libhalfproc.a; do
    [ -f "$objtop/$archive" ] || {
        echo "Missing native world archive $objtop/$archive; finish buildworld or set HALFBSD_OBJTOP" >&2
        exit 1
    }
done
make -C "$src/usr.sbin/halfconsole" SRCTOP="$src" OBJTOP="$objtop" \
    MK_PIE=yes MK_DEBUG_FILES=no all
i=0
while [ "$i" -lt 2 ]; do
    make -j4 -C "$src/usr.sbin/halfconsole" SRCTOP="$src" OBJTOP="$objtop" MK_PIE=yes \
        DESTDIR="$work/stage" BINOWN="$owner" BINGRP="$group" \
        MANOWN="$owner" MANGRP="$group" MK_DEBUG_FILES=no install
    i=$((i+1))
done
[ -x "$work/stage/usr/sbin/halfconsole" ]
# Invocation outside PID 1 must fail before reading policy or running login.
if "$work/stage/usr/sbin/halfconsole" > "$work/guard.log" 2>&1; then exit 1; fi
grep -q 'requires a root console session launched by PID 1' "$work/guard.log"
echo 'PASS native halfconsole full PIE link, private repeated installation and activation guard'
