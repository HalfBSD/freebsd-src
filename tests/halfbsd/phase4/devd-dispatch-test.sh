#!/bin/sh
# SPDX-License-Identifier: BSD-2-Clause
set -eu
src=${1:?source tree required}
suite=$(realpath "$(dirname "$0")")
work=$(mktemp -d "${TMPDIR:-/tmp}/halfbsd-devd-dispatch.XXXXXX")
trap 'rm -rf "$work"' EXIT HUP INT TERM
awk '/^action::do_action\(config &c\)/ { active=1; print "bool" }
     /^match::match/ { if (active) exit }
     active { print }' "$src/sbin/devd/devd.cc" > "$work/devd-action.inc"
[ -s "$work/devd-action.inc" ]
${CXX:-c++} -std=c++17 -Wall -Wextra -Wpedantic -Werror -Wshadow -Wcast-align \
    -fPIE -pie -Wl,-z,text -I"$src/sbin/devd" -I"$work" \
    "$suite/devd_dispatch_test.cc" -o "$work/test"
"$work/test"

awk '/^notify_clients\(const char \*data, int len\)/ { active=1; print "static void" }
     active { print }
     active && /^}/ { exit }' "$src/sbin/devd/devd.cc" > "$work/devd-notify.inc"
${CXX:-c++} -std=c++17 -Wall -Wextra -Wpedantic -Werror -Wshadow -Wcast-align \
    -fPIE -pie -Wl,-z,text -I"$src/sbin/devd" -I"$work" \
    "$suite/devd_notify_test.cc" -o "$work/notify"
"$work/notify"
