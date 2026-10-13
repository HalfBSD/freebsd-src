#!/bin/sh
# SPDX-License-Identifier: BSD-2-Clause
set -eu
suite=$(realpath "$(dirname "$0")")
src=${HALFBSD_SOURCE:-/usr/src}
modules=${HALFBSD_MODULES:-$src/usr.sbin/halfinit/modules}
lua=${HALFBSD_LUA:-/usr/bin/lua}
manager=${HALFBSD_HALFINIT:-/usr/sbin/halfinit}
client=${HALFBSD_HALFSVC:-/usr/sbin/halfsvc}
"$lua" -E "$suite/unit.lua" "$modules"
if [ "$(uname -s)" != FreeBSD ]; then
    echo 'Native integration/stress and Phase I gates require FreeBSD.' >&2
    exit 1
fi
[ -x "$manager" ] && [ -x "$client" ] || { echo 'Install the Phase II world first.' >&2; exit 1; }
# The original nine scenarios remain a mandatory regression gate.
sh "$src/tests/halfbsd/run-userland.sh"
work=$(mktemp -d "${TMPDIR:-/tmp}/halfbsd-phase2.XXXXXX")
cleanup()
{
    # Cancellation of the test driver must not leave its private manager alive.
    if [ -S "$work/runtime/control.sock" ]; then
        "$client" --module-dir "$modules" --socket "$work/runtime/control.sock" shutdown \
            >"$work/cancel-cleanup.log" 2>&1 || :
    fi
    if [ "${HALFBSD_KEEP_TESTS:-0}" = 1 ]; then echo "Artifacts: $work"; else rm -rf "$work"; fi
}
trap cleanup EXIT
trap 'exit 129' HUP
trap 'exit 130' INT
trap 'exit 143' TERM
uname -a
"$lua" -E "$suite/ipc.lua" "$work/ipc-check"
(cd "$work" && /bin/timeout -k 5s 600s "$lua" -E "$suite/native.lua" \
    "$manager" "$client" "$modules" "$src/usr.sbin/halfinit/services/system.lua")
