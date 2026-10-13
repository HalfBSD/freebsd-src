#!/bin/sh
# SPDX-License-Identifier: BSD-2-Clause
# Does not mount, change interfaces, change IPFW or start system daemons.
set -eu
suite=$(realpath "$(dirname "$0")")
src=${HALFBSD_SOURCE:-/usr/src}
modules=${HALFBSD_MODULES:-$src/usr.sbin/halfinit/modules}
lua=${HALFBSD_LUA:-/usr/bin/lua}
"$lua" -E "$suite/policy.lua" "$modules"
if [ "${1:-}" = --policy-only ]; then exit 0; fi
[ "$(uname -s)" = FreeBSD ] || { echo 'Native Phase III checks require FreeBSD.' >&2; exit 1; }
work=$(mktemp -d "${TMPDIR:-/tmp}/halfbsd-phase3.XXXXXX")
trap 'rm -rf "$work"' EXIT HUP INT TERM
# Compile fixture helpers from this source checkout against the installed world.
cc -std=c17 -Wall -Wextra -Wpedantic -Werror "$suite/probe_helper.c" -o "$work/probe_helper"
cc -std=c17 -Wall -Wextra -Wpedantic -Werror -fPIE -pie \
    "$suite/terminal_helper.c" -lhalfproc -lpthread -o "$work/terminal_helper"
cat > "$work/plan.lua" <<EOF_PROFILE
return {target='minimal',runtime_dir='$work/runtime',boot={interfaces={'lo0'},
    local_mounts={},zfs=false,dns=false,sshd=false,ntpd=false}}
EOF_PROFILE
manager=${HALFBSD_HALFINIT:-/usr/sbin/halfinit}
"$manager" --module-dir "$modules" --config "$work/plan.lua" --dry-run > "$work/plan.json"
"$manager" --module-dir "$modules" --config "$work/plan.lua" --shadow > "$work/shadow.json"
[ ! -e "$work/runtime" ]
echo 'PASS real adapter dry-run and shadow create no runtime files'
uname -a
/bin/timeout -k 5s 30s "$lua" -E "$suite/native.lua" "$modules" "$work/probe_helper"
/bin/timeout -k 5s 15s "$work/terminal_helper"
# Keep the full Phase I and Phase II native regression gate.
sh "$src/tests/halfbsd/phase2/run.sh"
echo 'PASS Phase III non-destructive userspace adapter suite (real boot activation remains untested)'
