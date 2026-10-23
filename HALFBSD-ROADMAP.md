# HalfBSD implementation agenda

Ordered agenda for functionality to remove, change or add. Outstanding checks
and acceptance evidence belong in [HALFBSD-TESTING.md](HALFBSD-TESTING.md).
These items describe proposed work, not confirmed defects or completed changes.

1. **Remove PAM**, replacing its remaining responsibilities in console login,
   SSH and `su`.
2. **Contain complete service process trees**, including children that fork or
   detach.
3. **Recreate XDG runtime directories at boot**, with correct ownership and
   permissions.
4. **Add runtime-state cleanup**, defining what gets cleared at boot and after
   sessions end.
5. **Add application service definitions**, starting with a consistent way to
   integrate PostgreSQL and other installed services.
6. **Complete hostname and timezone configuration** under HalfBSD's
   configuration system.
7. **Centralize sysctl policy**, applying workstation defaults and administrator
   overrides through Lua startup.
8. **Centralize devfs permissions**, including consistent rules for newly
   attached devices.
9. **Reduce remaining compatibility code**, removing unused paths after
   identifying their consumers.
10. **Simplify boot diagnostics**, keeping normal startup concise while retaining
    detailed logs and an explicit verbose mode.
