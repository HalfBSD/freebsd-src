#!/bin/sh
# SPDX-License-Identifier: BSD-2-Clause
# Check world/bootstrap library selection with an inherited shared ABI version.
set -eu
src=$(realpath "${1:?usage: library_build.sh source-tree [bsd-make]}")
bsd_make=${2:-make}
work=$(mktemp -d "${TMPDIR:-/tmp}/halfbsd-lua-libraries.XXXXXX")
trap 'rm -rf "$work"' EXIT HUP INT TERM
mkdir "$work/obj"
query()
{
    component=$1
    variable=$2
    shift 2
    MAKEOBJDIR="$work/obj" "$bsd_make" -m "$src/share/mk" \
        -C "$src/$component" SRCTOP="$src" MACHINE=amd64 MACHINE_ARCH=amd64 \
        COMPILER_TYPE=gcc COMPILER_VERSION=130000 MK_PIE=yes SHLIB_MAJOR=5 \
        SYSROOT="$work/sysroot" "$@" -V "$variable"
}
for context in world bootstrap; do
    case "$context" in
        world) set -- ;;
        bootstrap) set -- BOOTSTRAPPING=yes ;;
    esac
    [ "$(query lib/libhalfbsd _LIBS "$@")" = libhalfbsd.a ]
    [ "$(query lib/libhalfproc _LIBS "$@")" = libhalfproc.a ]
    case " $(query lib/liblua _LIBS "$@") " in
        *' liblua.a liblua_pie.a '*) ;;
        *) echo 'Lua static or PIE archive is missing' >&2; exit 1 ;;
    esac
    for component in lib/libhalfbsd lib/libhalfproc lib/liblua; do
        [ -z "$(query "$component" SHLIB_NAME "$@")" ]
        case "$component" in
            lib/libhalfbsd|lib/libhalfproc)
                case " $(query "$component" CFLAGS "$@") " in
                    *' -fPIC '*) ;;
                    *) echo "Archive objects are not PIC: $component" >&2; exit 1 ;;
                esac ;;
        esac
    done
    [ "$(query lib/liblua LUASTATIC "$@")" = liblua.a ]
    [ "$(query lib/liblua LUASTATICDIR "$@")" = /usr/lib ]
    case "$(query usr.bin/lua LDADD "$@")" in
        *-lhalfbsd*-lhalfproc*-llua_pie*) ;;
        *) echo 'Interpreter lost its native/Lua archive dependencies' >&2; exit 1 ;;
    esac
done
printf '%s\n' 'PASS: inherited ABI version cannot enable shared foundation libraries; Lua archive linkage preserved'
