#!/bin/sh
# SPDX-License-Identifier: BSD-2-Clause
# Install desktop packages and ports after upgrading the base system.

set -eu

case "${1:-}" in
-h|--help)
	echo "Usage: $0 [package | port:category/name ...]"
	echo "Without arguments, build drm-kmod from ports and install desktop packages."
	echo "The name drm-kmod always selects port:graphics/drm-kmod."
	echo "Set PORTSDIR to override /usr/ports."
	exit 0
	;;
esac

if [ "$#" -eq 0 ]; then
	set -- port:graphics/drm-kmod seatd wayland sway foot fontconfig noto-basic \
	    noto-emoji jetbrains-mono nerd-fonts-symbols
fi

if [ "$(id -u)" -ne 0 ]; then
	echo "Run this script as root to install packages." >&2
	exit 1
fi

# Check port paths before beginning installation.
PORTSDIR=${PORTSDIR:-/usr/ports}
for item do
	case "$item" in
	drm-kmod) origin=graphics/drm-kmod ;;
	port:*) origin=${item#port:} ;;
	-*)
		echo "Unknown option: $item (use --help for usage)." >&2
		exit 1
		;;
	*) continue ;;
	esac
	case "$origin" in
	''|/*|*..*)
		echo "Invalid port origin: $origin" >&2
		exit 1
		;;
	esac
	if [ ! -f "$PORTSDIR/$origin/Makefile" ]; then
		echo "Port not found: $PORTSDIR/$origin (set PORTSDIR to your ports tree)." >&2
		exit 1
	fi
done

# Use the base-system wrapper so pkg can bootstrap on a fresh installation.
export ASSUME_ALWAYS_YES=yes
for item do
	case "$item" in
	drm-kmod) origin=graphics/drm-kmod ;;
	port:*) origin=${item#port:} ;;
	*)
		/usr/sbin/pkg install -y -- "$item"
		continue
		;;
	esac
	# Build dependencies from ports too, including the versioned DRM driver
	# selected by the drm-kmod meta-port.  Stop on build/install failures.
	/usr/bin/make -C "$PORTSDIR/$origin" BATCH=yes install clean
done
