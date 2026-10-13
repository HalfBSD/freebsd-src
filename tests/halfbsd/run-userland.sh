#!/bin/sh
# SPDX-License-Identifier: BSD-2-Clause
# Run installed Lua against temporary userland services, never the active rc.
set -eu
if [ "$(uname -s)" != FreeBSD ]; then
    echo 'Native FreeBSD required; Linux cannot validate process descriptors/kqueue.' >&2
    exit 1
fi
suite=$(realpath "$(dirname "$0")")
lua=${HALFBSD_LUA:-/usr/bin/lua}
case "$lua" in
    /*) ;;
    *) echo 'HALFBSD_LUA must be an absolute interpreter path' >&2; exit 1 ;;
esac
[ -x "$lua" ] || { echo "Lua interpreter missing: $lua" >&2; exit 1; }
[ -x /bin/timeout ] || { echo '/bin/timeout is required' >&2; exit 1; }
work=$(mktemp -d "${TMPDIR:-/tmp}/halfbsd-userland.XXXXXX")
cleanup()
{
    if [ "${HALFBSD_KEEP_TESTS:-0}" = 1 ]; then
        echo "Test artifacts: $work"
    else
        rm -rf "$work"
    fi
}
trap cleanup EXIT
trap 'exit 129' HUP
trap 'exit 130' INT
trap 'exit 143' TERM
uname -a
"$lua" -E -v
printf 'Running isolated userland supervision tests as uid %s\n' "$(id -u)"
failed=0
for scenario in startup parallel restart shutdown forced_shutdown timeout failure logging repeat_cleanup; do
    mkdir "$work/$scenario"
    if (cd "$work/$scenario" && /bin/timeout -k 3s 30s \
        "$lua" -E "$suite/supervision.lua" "$scenario" "$lua" "$suite/service_fixture.lua") \
        >"$work/$scenario/output" 2>&1; then
        cat "$work/$scenario/output"
    else
        failed=$((failed+1))
        printf 'FAIL %s\n' "$scenario"
        cat "$work/$scenario/output"
        for error_log in "$work/$scenario"/*.err; do
            [ -f "$error_log" ] || continue
            printf 'Fixture stderr: %s\n' "$error_log"
            cat "$error_log"
        done
    fi
done
[ "$failed" -eq 0 ] || { printf '%s scenario(s) failed\n' "$failed"; exit 1; }
printf 'PASS: all nine userland supervision scenarios\n'
