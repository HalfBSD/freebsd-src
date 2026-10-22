#!/bin/sh
# SPDX-License-Identifier: BSD-2-Clause
# Exercise the real frontend with private command paths, never host services.
set -eu
src=${1:?source tree required}
work=$(mktemp -d "${TMPDIR:-/tmp}/halfbsd-service.XXXXXX")
trap 'rm -rf "$work"' EXIT HUP INT TERM
cat > "$work/halfsvc" <<'EOF'
#!/bin/sh
printf '%s\n' "$@"
exit "${TEST_CLIENT_STATUS:-0}"
EOF
chmod +x "$work/halfsvc"
sed -e "s,/usr/sbin/halfsvc,$work/halfsvc,g" "$src/usr.sbin/service/service.sh" > "$work/service"
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
# Manager failure must be propagated; there is no second control backend.
if TEST_CLIENT_STATUS=3 sh "$work/service" dns restart > "$work/out"; then exit 1; else [ "$?" = 3 ]; fi
echo 'PASS sole-manager service dispatch, aliases, retired actions and failure propagation'
