#!/bin/sh
# SPDX-License-Identifier: BSD-2-Clause
# Native BSD make recipes and staging checks; does not claim a FreeBSD build.
set -eu
src=$(realpath "${1:?usage: build.sh source-tree bsd-make}")
bsd_make=${2:-make}
work=$(mktemp -d "${TMPDIR:-/tmp}/halfbsd-seat-build.XXXXXX")
trap 'rm -rf "$work"' EXIT HUP INT TERM
mkdir "$work/lib" "$work/daemon"
query()
{
    part=$1; variable=$2
    MAKEOBJDIR="$work/lib" "$bsd_make" -m "$src/share/mk" -C "$src/$part" \
        SRCTOP="$src" MACHINE=amd64 MACHINE_ARCH=amd64 COMPILER_TYPE=gcc \
        COMPILER_VERSION=130000 SYSROOT="$work/sysroot" SHLIB_MAJOR=5 MK_PIE=yes \
        -V "$variable"
}
[ "$(query lib/libseat SHLIB_NAME)" = libseat.so.1 ]
[ "$(query lib/libseat SONAME)" = libseat.so.1 ]
case " $(query lib/libseat _LIBS) " in *' libseat.a libseat.so.1 '*) ;; *) exit 1 ;; esac
[ "$(query usr.sbin/seatd BINDIR)" = /usr/sbin ]
case " $(query usr.sbin/seatd CFLAGS) " in *' -fPIE '*) ;; *) exit 1 ;; esac
case " $(query lib/libseat CFLAGS) " in *'-DSEATD_ENABLED=1'*) ;; *) exit 1 ;; esac
case " $(query lib/libseat CFLAGS) " in *' -fPIC '*) ;; *) exit 1 ;; esac
# Compile both colliding files with the dialect actually selected by BSD make.
# Strict-only fixture flags would hide GNU mode's predefined `unix` macro.
for part in lib/libseat usr.sbin/seatd; do
    standard=$(query "$part" CSTD)
    [ "$standard" = c17 ]
    case " $(query "$part" CFLAGS) " in *' -std=c17 '*) ;; *) exit 1 ;; esac
    case "$part" in
        lib/libseat) input=libseat/backend/seatd.c ;;
        usr.sbin/seatd) input=seatd/seatd.c ;;
    esac
    ${CC:-cc} -std="$standard" -Wall -Wextra -Wpedantic -Werror \
        -D_XOPEN_SOURCE=700 -D_DEFAULT_SOURCE -D__BSD_VISIBLE=1 \
        -DSEATD_VERSION='"0.9.3"' -DSEATD_DEFAULTPATH='"/var/run/seatd.sock"' \
        -I"$src/contrib/seatd/include" -fPIC -c "$src/contrib/seatd/$input" \
        -o "$work/dialect.o"
done
# The compatibility pass must keep its own metadata directory and ABI.
[ "$(MAKEOBJDIR="$work/lib" "$bsd_make" -m "$src/share/mk" -C "$src/lib/libseat" \
    SRCTOP="$src" MACHINE=amd64 MACHINE_ARCH=i386 COMPILER_TYPE=gcc \
    COMPILER_VERSION=130000 LIBDIR=/usr/lib32 LIBDATADIR=/usr/lib32 SHLIB_MAJOR=5 \
    -V SEATPCDIR)" = /usr/lib32/pkgconfig ]
# Compile recipes must resolve the client backend, not daemon's same-named file.
MAKEOBJDIR="$work/lib" "$bsd_make" -n -m "$src/share/mk" -C "$src/lib/libseat" \
    SRCTOP="$src" MACHINE=amd64 MACHINE_ARCH=amd64 COMPILER_TYPE=gcc \
    COMPILER_VERSION=130000 SYSROOT="$work/sysroot" SHLIB_MAJOR=5 seatd.pico > "$work/recipe"
grep -q '/libseat/backend/seatd.c' "$work/recipe" || exit 1
# Fresh/repeated parallel API and metadata installation, with real generated .pc.
for pass in 1 2; do
    MAKEOBJDIR="$work/lib" "$bsd_make" -j8 -m "$src/share/mk" -C "$src/lib/libseat" \
        SRCTOP="$src" MACHINE=amd64 MACHINE_ARCH=amd64 COMPILER_TYPE=gcc \
        COMPILER_VERSION=130000 INSTALL=install DESTDIR="$work/stage" \
        BINOWN="$(id -un)" BINGRP="$(id -gn)" LIBOWN="$(id -un)" LIBGRP="$(id -gn)" \
        SEATPCDIR_OWN="$(id -un)" SEATPCDIR_GRP="$(id -gn)" \
        INCSDIR_OWN="$(id -un)" INCSDIR_GRP="$(id -gn)" installincludes installfiles
    cmp "$src/contrib/seatd/include/libseat.h" "$work/stage/usr/include/libseat.h"
    PCFILE="$work/stage/usr/libdata/pkgconfig/libseat.pc"
    [ -f "$PCFILE" ]
    if command -v pkgconf >/dev/null 2>&1; then
        [ "$(pkgconf --modversion "$PCFILE")" = 0.9.3 ]
        [ "$(pkgconf --variable=have_seatd "$PCFILE")" = true ]
    fi
done
echo 'PASS staged BSD make recipes, fixed ABI under inherited SHLIB_MAJOR, PIE, fresh/repeated parallel header/pkgconfig installation'
