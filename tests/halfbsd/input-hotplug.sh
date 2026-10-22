#!/bin/sh
# SPDX-License-Identifier: BSD-2-Clause
set -eu
[ "$(uname -s)" = FreeBSD ] || { echo 'Native FreeBSD required' >&2; exit 1; }
suite=$(realpath "$(dirname "$0")")
lua=${HALFBSD_LUA:-/usr/bin/lua}
exec "$lua" -E -e 'os.exit(assert(loadfile(arg[0]))()(arg))' -- "$suite/input-hotplug.lua" "$@"
