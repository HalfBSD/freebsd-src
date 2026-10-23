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
sh "$src/tests/halfbsd/console/run.sh"
# Parse the opt-in live test without executing against host services.
HALFBSD_LIVE_CHECK="$suite/check-live.lua" "$lua" -E -e 'assert(loadfile(os.getenv("HALFBSD_LIVE_CHECK")))'
"$lua" -E "$suite/singleboot.lua" "$modules"
"$lua" -E "$suite/policy.lua" "$modules" "$src/usr.sbin/halfinit/examples/boot-system.lua"
"$lua" -E "$suite/workstation.lua" "$modules"
"$lua" -E "$suite/devmatch.lua" "$modules"
cc -std=c17 -Wall -Wextra -Wpedantic -Werror -fPIE -pie -Wl,-z,text \
    -I"$src/sbin/devd" "$suite/devd_action_test.c" -o "$work/devd_action_test"
sh "$suite/devd-dispatch-test.sh" "$src"
"$work/devd_action_test"
sh "$suite/devd-dispatch-test.sh" "$src"
"$lua" -E "$suite/shutdown_report.lua" "$modules"
"$lua" -E "$suite/../input-hotplug-test.lua" "$suite/../input-hotplug.lua"
sh "$suite/service_test.sh" "$src"
cc -D_GNU_SOURCE -std=c17 -Wall -Wextra -Wpedantic -Werror \
    -Wstrict-prototypes -Wmissing-prototypes -Wshadow -Wcast-align \
    -I"$src/sbin/init" "$suite/handoff_test.c" "$src/sbin/init/halfbsd.c" -o "$work/handoff_test"
"$work/handoff_test"
cc -D_GNU_SOURCE -std=c17 -Wall -Wextra -Wpedantic -Werror -fPIE -pie -Wl,-z,text \
    -Wstrict-prototypes -Wmissing-prototypes -Wshadow -Wcast-align \
    -I"$src/sbin/init" "$suite/shutdown_report_test.c" "$src/sbin/init/shutdown_report.c" \
    "$src/sbin/init/halfbsd.c" -o "$work/shutdown_report_test"
"$work/shutdown_report_test"
if [ "${1:-}" = --portable-only ]; then exit 0; fi
[ "$(uname -s)" = FreeBSD ] || { echo 'Native regression gate requires FreeBSD.' >&2; exit 1; }
sh "$suite/build-console.sh" "$src"
sh "$suite/config_install.sh" "$src"
sh "$src/tests/halfbsd/phase3/run.sh"
echo 'PASS Phase IV userspace checks; actual VM boot/shutdown acceptance is separate'
