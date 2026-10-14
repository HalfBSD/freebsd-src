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
    for module in init client service config dependency supervisor control json providers adapters boot handoff fixture; do
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
# Seed once during program installation; preserve admin bytes and mode thereafter.
seed()
{
    MAKEOBJDIR="$work/obj" "$bsd_make" -m "$src/share/mk" -C "$src/usr.sbin/halfinit" \
        install-boot-config SRCTOP="$src" MACHINE=amd64 MACHINE_ARCH=amd64 \
        COMPILER_TYPE=gcc COMPILER_VERSION=130000 INSTALL=install DESTDIR="$work/stage" \
        BINOWN="$(id -un)" BINGRP="$(id -gn)"
}
seed
cmp "$src/usr.sbin/halfinit/examples/boot-system.lua" "$work/stage/etc/halfbsd/system.lua"
printf 'return {target="custom"}\n' > "$work/stage/etc/halfbsd/system.lua"
cp "$work/stage/etc/halfbsd/system.lua" "$work/admin.lua"
chmod 640 "$work/stage/etc/halfbsd/system.lua"
seed & first=$!
seed & second=$!
wait "$first"; wait "$second"
cmp "$work/admin.lua" "$work/stage/etc/halfbsd/system.lua"
case "$(LC_ALL=C ls -ld "$work/stage/etc/halfbsd/system.lua")" in -rw-r-----*) ;; *) exit 1;; esac
rm "$work/stage/etc/halfbsd/system.lua"
seed & first=$!
seed & second=$!
wait "$first"; wait "$second"
cmp "$src/usr.sbin/halfinit/examples/boot-system.lua" "$work/stage/etc/halfbsd/system.lua"
# Existing symlinks are preserved so the secure loader can reject them.
rm "$work/stage/etc/halfbsd/system.lua"
ln -s "$work/admin.lua" "$work/stage/etc/halfbsd/system.lua"
seed
[ -L "$work/stage/etc/halfbsd/system.lua" ]
cmp "$work/admin.lua" "$work/stage/etc/halfbsd/system.lua"
echo 'PASS fresh/parallel boot configuration seeding and administrator preservation'
# Check the effective live-root recipe too, without running it on this host.
for destination in '' /; do
    MAKEOBJDIR="$work/obj" "$bsd_make" -n -m "$src/share/mk" -C "$src/usr.sbin/halfinit" \
        install-boot-config SRCTOP="$src" MACHINE=amd64 MACHINE_ARCH=amd64 \
        COMPILER_TYPE=gcc COMPILER_VERSION=130000 INSTALL=install DESTDIR="$destination" \
        BINOWN="$(id -un)" BINGRP="$(id -gn)" > "$work/live-install.sh"
    sh -n "$work/live-install.sh"
done
echo 'PASS effective empty-DESTDIR and root-DESTDIR seed recipe syntax (not executed)'
