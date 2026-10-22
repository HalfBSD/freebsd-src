#!/bin/sh
# SPDX-License-Identifier: BSD-2-Clause
# Live installworld refresh; never sources RC or controls a daemon directly.
set -eu
/usr/sbin/local-unbound-setup -n -u unbound -w /var/unbound \
    -c /etc/unbound/unbound.conf -a /var/unbound/root.key
/usr/bin/su -m unbound -c '/usr/sbin/local-unbound-checkconf /etc/unbound/unbound.conf'
# A fresh/staged installation must not activate a new resolver. An existing
# resolver belongs to halfinit; its dependencies and trust-anchor startup apply.
status=0
/bin/pgrep -x 'local-unbound|unbound' >/dev/null || status=$?
case "$status" in
0) /usr/sbin/halfsvc restart dns ;;
1) : ;;
*) echo 'Cannot inspect running resolver; refusing an uncontrolled restart' >&2; exit 1 ;;
esac
