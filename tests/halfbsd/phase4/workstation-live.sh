#!/bin/sh
# SPDX-License-Identifier: BSD-2-Clause
# Explicit opt-in: queries live services, writes only a NEW report directory.
# Never stops services, changes configuration, or reboots this host.
set -eu
[ "${1:-}" = --live ] || {
    echo 'Usage: workstation-live.sh --live NEW-RESULT-DIR KNOWN-SIGNED-DOMAIN KNOWN-BOGUS-DOMAIN' >&2
    exit 2
}
[ "$#" -eq 4 ]
[ "$(uname -s)" = FreeBSD ] || { echo 'Native FreeBSD required' >&2; exit 1; }
[ "$(id -u)" = 0 ] || { echo 'Run as root for local Unbound statistics access' >&2; exit 1; }
results=$2; signed=$3; bogus=$4
for domain in "$signed" "$bogus"; do
    case "$domain" in ''|-*|*[!a-zA-Z0-9.-]*) echo 'Invalid DNS test name' >&2; exit 2 ;; esac
done
[ ! -e "$results" ] && [ ! -L "$results" ]
mkdir -m 700 "$results"
src=${HALFBSD_SOURCE:-/usr/src}
failed=0
record()
{
    name=$1; shift
    if /usr/bin/timeout -k 2s 20s "$@" > "$results/$name.log" 2>&1; then
        echo "PASS $name" | tee -a "$results/summary.txt"
    else
        echo "FAIL $name (see $name.log)" | tee -a "$results/summary.txt"
        failed=1
    fi
}
record system uname -a
record mode /bin/kenv halfbsd.boot_active
[ "$(cat "$results/mode.log")" = lua ] || { echo 'Expected Lua boot' >&2; exit 1; }
record live-state /usr/bin/lua -E "$src/tests/halfbsd/phase4/check-live.lua"
record service-status /usr/sbin/service seatd status
record devd-status /usr/sbin/service devd status
record process-inventory /bin/ps -axo pid,ppid,user,command
record session-inventory /usr/bin/who
record current-groups /usr/bin/id
record udp-dns /usr/bin/drill -D @127.0.0.1 "$signed" A
record tcp-dns /usr/bin/drill -t -D @127.0.0.1 "$signed" A
record bogus-dns /usr/bin/drill -D @127.0.0.1 "$bogus" A
record cache-before /usr/sbin/local-unbound-control -c /etc/unbound/unbound.conf stats_noreset
record cached-query /usr/bin/drill -D @127.0.0.1 "$signed" A
record cache-after /usr/sbin/local-unbound-control -c /etc/unbound/unbound.conf stats_noreset
before=$(awk -F= '$1=="total.num.cachehits" {print $2}' "$results/cache-before.log")
after=$(awk -F= '$1=="total.num.cachehits" {print $2}' "$results/cache-after.log")
if ! awk -v a="$before" -v b="$after" 'BEGIN {exit !(a ~ /^[0-9]+$/ && b ~ /^[0-9]+$/ && b>a)}'; then
    echo 'FAIL aggregate cache hits did not increase across repeated query' | tee -a "$results/summary.txt"; failed=1
fi
for protocol in udp tcp; do
    if ! grep -q 'rcode: NOERROR' "$results/$protocol-dns.log" ||
        ! grep -Eq '^;; flags: .* ad[ ;]' "$results/$protocol-dns.log"; then
        echo "FAIL $protocol DNSSEC authenticated answer" | tee -a "$results/summary.txt"; failed=1
    fi
done
if ! grep -q 'rcode: SERVFAIL' "$results/bogus-dns.log"; then
    echo 'FAIL known-bogus DNSSEC answer must SERVFAIL' | tee -a "$results/summary.txt"; failed=1
fi
record ntp-peers /usr/bin/ntpq -pn
if ! grep -q '^\*' "$results/ntp-peers.log"; then
    echo 'FAIL NTP has no selected synchronization peer yet' | tee -a "$results/summary.txt"; failed=1
fi
echo 'MANUAL gates: fresh console selection/groups, Sway/VT/USB cycling, simultaneous SSH, reboot, poweroff and recovery.' | tee -a "$results/summary.txt"
echo 'DNS names must independently be known signed/bogus; DNS output alone cannot establish fixture validity.' >> "$results/summary.txt"
exit "$failed"
