#!/bin/sh
# SPDX-License-Identifier: BSD-2-Clause
# Builds host protocol code with an explicit console fixture; no device access.
set -eu
src=${HALFBSD_SOURCE:-$(realpath "$(dirname "$0")/../../..")}
work=$(mktemp -d "${TMPDIR:-/tmp}/halfbsd-seatd.XXXXXX")
child=
cleanup()
{
    if [ -n "$child" ]; then kill -TERM "$child" 2>/dev/null || :; wait "$child" || :; fi
    rm -rf "$work"
}
trap cleanup EXIT HUP INT TERM
# The isolated smoke test must not run as root.
[ "$(id -u)" -ne 0 ] || { echo 'Run seatd private fixtures as an ordinary user.' >&2; exit 1; }
cd "$src/contrib/seatd"
# client.c defines _GNU_SOURCE itself. _DEFAULT_SOURCE exposes host Linux
# extensions elsewhere without redefining that macro; FreeBSD uses BSD_VISIBLE.
flags='-std=c17 -Wall -Wextra -Wpedantic -Werror -D_XOPEN_SOURCE=700 -D_DEFAULT_SOURCE -D__BSD_VISIBLE=1'
for test in linked_list connection poller; do
    case "$test" in
        linked_list) sources='common/linked_list.c' ;;
        connection) sources='common/connection.c' ;;
        poller) sources='common/linked_list.c seatd/poller.c' ;;
    esac
    ${CC:-cc} $flags -Iinclude "tests/$test.c" $sources -o "$work/$test"
    "$work/$test"
done
${CC:-cc} $flags -Iinclude -fsyntax-only common/terminal.c
set --
for file in libseat/libseat.c libseat/backend/noop.c libseat/backend/seatd.c \
    common/connection.c common/linked_list.c common/log.c; do
    object="$work/$(basename "$file" .c).o"
    ${CC:-cc} $flags -Iinclude -fPIC -DLIBSEAT=1 -DSEATD_ENABLED=1 \
        -DSEATD_DEFAULTPATH="\"$work/seatd.sock\"" -c "$file" -o "$object"
    set -- "$@" "$object"
done
ar crs "$work/libseat.a" "$@"
${CC:-cc} -shared "$@" -Wl,--version-script=libseat/libseat.syms \
    -Wl,-soname,libseat.so.1 -Wl,-z,text -o "$work/libseat.so.1"
ln -s libseat.so.1 "$work/libseat.so"
${CC:-cc} $flags -Iinclude -fPIE -pie -DSEATD_VERSION='"0.9.3"' \
    -DSEATD_DEFAULTPATH="\"$work/seatd.sock\"" -DSEATD_INSTALLPATH='"/usr/sbin/seatd"' \
    seatd/*.c common/log.c common/linked_list.c "$src/tests/halfbsd/seatd/terminal_fixture.c" common/connection.c \
    common/evdev.c common/hidraw.c common/drm.c common/wscons.c \
    -Wl,-z,text -o "$work/seatd"
${CC:-cc} $flags -Iinclude -fPIE -pie "$src/tests/halfbsd/seatd/client.c" \
    -L"$work" -Wl,-rpath,"$work" -Wl,-z,text -lseat -o "$work/client"
${CC:-cc} $flags -Iinclude -fPIE -pie "$src/tests/halfbsd/seatd/client.c" \
    "$work/libseat.a" -Wl,-z,text -o "$work/static-client"
"$work/seatd" -u "$(id -un)" -g "$(id -gn)" -l error >"$work/server.log" 2>&1 &
child=$!
tries=0
while [ ! -S "$work/seatd.sock" ]; do
    kill -0 "$child" || { cat "$work/server.log"; exit 1; }
    tries=$((tries + 1)); [ "$tries" -le 100 ] || { cat "$work/server.log"; exit 1; }
    sleep 0.05
done
# The client has a native alarm deadline; cleanup always reaps the private daemon.
for binary in "$work/client" "$work/static-client"; do
    if ! LIBSEAT_BACKEND=seatd SEATD_SOCK="$work/seatd.sock" "$binary"; then
        cat "$work/server.log"; exit 1
    fi
done
kill -TERM "$child"; wait "$child"; child=
[ ! -e "$work/seatd.sock" ]
echo "PASS $(uname -s) seatd/libseat strict C17, PIE/shared link and private graceful shutdown; console fixture; no native console/device validation"
