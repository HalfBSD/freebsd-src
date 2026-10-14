#!/bin/sh
# SPDX-License-Identifier: BSD-2-Clause
# Safe userspace checks; does not select a boot mode or change host policy.
set -eu
suite=$(realpath "$(dirname "$0")")
src=${HALFBSD_SOURCE:-/usr/src}
modules=${HALFBSD_MODULES:-$src/usr.sbin/halfinit/modules}
lua=${HALFBSD_LUA:-/usr/bin/lua}
work=$(mktemp -d "${TMPDIR:-/tmp}/halfbsd-phase4.XXXXXX")
trap 'rm -rf "$work"' EXIT HUP INT TERM
"$lua" -E "$suite/policy.lua" "$modules" "$src/usr.sbin/halfinit/examples/boot-system.lua"
cc -D_GNU_SOURCE -std=c17 -Wall -Wextra -Wpedantic -Werror \
    -Wstrict-prototypes -Wmissing-prototypes -Wshadow -Wcast-align \
    -I"$src/sbin/init" "$suite/handoff_test.c" "$src/sbin/init/halfbsd.c" -o "$work/handoff_test"
"$work/handoff_test"
if [ "${1:-}" = --portable-only ]; then exit 0; fi
[ "$(uname -s)" = FreeBSD ] || { echo 'Native regression gate requires FreeBSD.' >&2; exit 1; }
sh "$src/tests/halfbsd/phase3/run.sh"
echo 'PASS Phase IV userspace checks; actual VM boot/shutdown acceptance is separate'
