#!/bin/sh
# SPDX-License-Identifier: BSD-2-Clause
# Generated world targets/flags, not a claim of native compilation.
set -eu
src=$(realpath "${1:?usage: build.sh source-tree [bsd-make]}")
bsd_make=${2:-make}
work=$(mktemp -d "${TMPDIR:-/tmp}/halfbsd-phase2-build.XXXXXX")
trap 'rm -rf "$work"' EXIT HUP INT TERM
mkdir "$work/obj"
query()
{
    component=$1; variable=$2
    MAKEOBJDIR="$work/obj" "$bsd_make" -m "$src/share/mk" -C "$src/$component" \
        SRCTOP="$src" MACHINE=amd64 MACHINE_ARCH=amd64 COMPILER_TYPE=gcc \
        COMPILER_VERSION=130000 SYSROOT="$work/sysroot" MK_PIE=yes SHLIB_MAJOR=5 -V "$variable"
}
for component in usr.sbin/halfinit usr.sbin/halfsvc; do
    [ "$(query "$component" BINDIR)" = /usr/sbin ]
    case "$(query "$component" LDADD)" in
        *-lhalfbsd*-lhalfproc*-llua_pie*) ;;
        *) echo "Incorrect archive selection: $component" >&2; exit 1 ;;
    esac
    case " $(query "$component" CFLAGS) " in *' -fPIE '*) ;; *) exit 1 ;; esac
    case " $(query "$component" LDFLAGS) " in *' -Wl,-E '*) ;; *) exit 1 ;; esac
done
# The actual bsd.files.mk installation creates directories with proper ordering.
for pass in 1 2; do
    MAKEOBJDIR="$work/obj" "$bsd_make" -m "$src/share/mk" -C "$src/usr.sbin/halfinit" \
        -j8 installfiles SRCTOP="$src" MACHINE=amd64 MACHINE_ARCH=amd64 \
        COMPILER_TYPE=gcc COMPILER_VERSION=130000 INSTALL=install DESTDIR="$work/stage" \
        SHAREOWN="$(id -un)" SHAREGRP="$(id -gn)" \
        MODULESDIR_OWN="$(id -un)" MODULESDIR_GRP="$(id -gn)" \
        DEFINITIONSDIR_OWN="$(id -un)" DEFINITIONSDIR_GRP="$(id -gn)" \
        EXAMPLESDIR_OWN="$(id -un)" EXAMPLESDIR_GRP="$(id -gn)"
    for module in init client service config dependency supervisor control json providers adapters boot fixture; do
        cmp "$src/usr.sbin/halfinit/modules/$module.lua" "$work/stage/usr/lib/halfbsd/$module.lua"
    done
    cmp "$src/usr.sbin/halfinit/services/system.lua" "$work/stage/usr/lib/halfbsd/services/system.lua"
    cmp "$src/usr.sbin/halfinit/examples/services.lua" "$work/stage/usr/share/examples/halfbsd/services.lua"
    for example in boot-system desktop-user; do
        cmp "$src/usr.sbin/halfinit/examples/$example.lua" "$work/stage/usr/share/examples/halfbsd/$example.lua"
    done
    [ ! -e "$work/stage/etc" ]
done
echo 'PASS Phase II generated PIE/archive flags and fresh/repeated parallel module installation'
