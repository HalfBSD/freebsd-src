#!/bin/sh
# SPDX-License-Identifier: BSD-2-Clause
# No host console, mount, login, signal or reboot operations are executed.
set -eu
ulimit -c 0
src=${HALFBSD_SOURCE:-$(realpath "$(dirname "$0")/../../..")}
suite="$src/tests/halfbsd/console"
# Removed code must not return behind an option or a different caller.
! grep -Eq 'get_shell|init_shell|DEBUGSHELL|_PATH_BSHELL' "$src/sbin/init/init.c"
! grep -Eq 'vt_proc_window_switch|vt_allocate_window|case VT_(ACTIVATE|SETMODE|WAITACTIVE|RELDISP|OPENQRY|LOCKSWITCH)' "$src/sys/dev/vt/vt_core.c"
! grep -Eq 'SEATD_VTBOUND|seat_vt_|server_handle_vt_' "$src/contrib/seatd/seatd/server.c"

work=$(mktemp -d "${TMPDIR:-/tmp}/halfbsd-console.XXXXXX")
trap 'rm -rf "$work"' EXIT HUP INT TERM
awk '/^static void$/ { last=$0; next }
 /^stopped_child\(/ { active=1; print "static void" }
 /^stopped\(void\)/ { active=1; print "static state_func_t" }
 active { print }
 active && /^}/ { active=0 }' "$src/sbin/init/init.c" |
    sed 's/sigaction(SIGCHLD/action_fixture(SIGCHLD/' > "$work/stopped.inc"
awk '/^static int console_open/ { active=1 }
 /^\/\*/ { if(active) exit }
 active { print }' "$src/contrib/seatd/seatd/seat.c" > "$work/console-handoff.inc"
for test in stopped console_handoff; do
    ${CC:-cc} -D_POSIX_C_SOURCE=200809L -std=c17 -Wall -Wextra -Wpedantic -Werror \
        -Wno-error=cast-function-type -Wshadow -Wcast-align -fPIE -pie -Wl,-z,text \
        -I"$work" "$suite/${test}_test.c" -o "$work/$test"
    "$work/$test"
done
printf 'PASS actual stopped PID 1 state: no fork/exec, child reaping, signal wakeup and shutdown transition\n'
printf 'PASS actual seatd console handoff: success, close and open/keyboard/graphics failures with rollback\n'
