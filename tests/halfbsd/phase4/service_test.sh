#!/bin/sh
# SPDX-License-Identifier: BSD-2-Clause
# Exercise the real frontend with private command paths, never host services.
set -eu
src=${1:?source tree required}
work=$(mktemp -d "${TMPDIR:-/tmp}/halfbsd-service.XXXXXX")
trap 'rm -rf "$work"' EXIT HUP INT TERM
cat > "$work/kenv" <<'EOF'
#!/bin/sh
[ "${TEST_MODE}" != absent ] || exit 1
printf '%s\n' "$TEST_MODE"
EOF
cat > "$work/halfsvc" <<'EOF'
#!/bin/sh
printf '%s\n' "$@"
exit "${TEST_CLIENT_STATUS:-0}"
EOF
printf '%s\n' 'echo RC-ENTERED; exit 17' > "$work/rc.subr"
chmod +x "$work/kenv" "$work/halfsvc"
sed -e "s,/bin/kenv,$work/kenv,g" -e "s,/usr/sbin/halfsvc,$work/halfsvc,g" \
    -e "s,/etc/rc.subr,$work/rc.subr,g" "$src/usr.sbin/service/service.sh" > "$work/service"
export TEST_MODE=lua
[ "$(sh "$work/service" seatd status)" = "$(printf 'status\nseatd')" ]
[ "$(sh "$work/service" local_unbound restart)" = "$(printf 'restart\ndns')" ]
[ "$(sh "$work/service" -l)" = list ]
# Unknown names are passed to the authenticated manager, never to RC.
[ "$(sh "$work/service" ports-daemon start)" = "$(printf 'start\nports-daemon')" ]
if TEST_CLIENT_STATUS=9 sh "$work/service" seatd start; then exit 1; else [ "$?" = 9 ]; fi
for action in onestart forcestop setup; do
    if sh "$work/service" seatd "$action" > "$work/out" 2>&1; then exit 1; fi
    ! grep -q RC-ENTERED "$work/out"
done
for TEST_MODE in absent garbage; do
    export TEST_MODE
    if sh "$work/service" seatd start > "$work/out" 2>&1; then exit 1; fi
    ! grep -q RC-ENTERED "$work/out"
done
export TEST_MODE=rc
if sh "$work/service" seatd status > "$work/out"; then exit 1; else [ "$?" = 17 ]; fi
grep -q RC-ENTERED "$work/out"
echo 'PASS service dispatch, alias, error propagation, ownership failure and explicit RC selection'
