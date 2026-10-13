-- SPDX-License-Identifier: BSD-2-Clause
-- Run from an existing foreground login terminal, with seatd already available.
-- XDG_RUNTIME_DIR must be private and owned by the logged-in user.
-- First: halfinit --config FILE --dry-run
-- Then:  halfinit --config FILE --activate-system
-- No autologin, PAM changes, or root compositor launch.
-- Sway readiness defaults to XDG_RUNTIME_DIR/wayland-0. Override wayland_socket
-- when appropriate; niri requires an explicit absolute readiness socket.
return {boot={mode='user',desktop={command={'/usr/local/bin/sway'}}}}
