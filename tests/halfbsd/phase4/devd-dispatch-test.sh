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

# Include the real global event marker and execution helper together: testing
# only do_action omits namespace collisions in my_system's parameters.
awk '/^static const char nomatch =/ { print }
     /^my_system\(const char \*command,/ { active=1; print "static int" }
     active { print }
     active && /^}/ { exit }' "$src/sbin/devd/devd.cc" > "$work/devd-system.inc"
for dialect in c++17 gnu++17; do
    ${CXX:-c++} -std="$dialect" -Wall -Wextra -Wpedantic -Werror -Wshadow \
        -Wcast-align -I"$work" -fsyntax-only "$suite/devd_system_test.cc"
done
