# Remaining RC responsibilities: source audit

Current status: the framework described in this baseline audit has now been
removed locally. HALFBSD-RC-REMOVAL.txt supersedes the retention statements below
and records which responsibilities remain unsupported or need native policy.
Native validation of the RC-free world remains pending.


October 21, 2026. This is source review, not native runtime verification.
RC remains installed; this batch deletes none of its infrastructure.

| Area | Current Lua coverage | Remaining dependency or review |
| --- | --- | --- |
| Main startup/shutdown | C PID 1 launches halfinit, owns login/recovery, and requests ordered Lua shutdown | Native reboot/poweroff and repeated-cycle evidence still required |
| Console | New halfconsole selects policy and execs native login | Prove PAM/session/groups, terminal restoration and logout/relogin on FreeBSD |
| Device events | Required foreground devd before seatd | Existing devd rules still invoke legacy actions; configuring a graph service does not migrate the action handlers |
| Network hotplug | Foreground DHCP and address/route readiness for initial selected interfaces | `sbin/devd/devd.conf` invokes `/etc/netif_hotplug`; `libexec/rc/netif_hotplug` sources rc.subr and calls rc.d/netif. Needs explicit ownership design before replacement |
| Suspend/resume and ACPI | No Lua sleep/resume orchestration | devd.conf invokes `/etc/rc.suspend`, `/etc/rc.resume` and ACPI helpers. Those still traverse RC dependencies; graphics/input/service lifecycle must be tested across sleep |
| Installation/upgrade | Configuration-preserving halfinit install and Unbound migration; setup restart now delegates to halfsvc in active Lua mode | `Makefile.inc1` still invokes the source local_unbound setup RC wrapper for setup parameters. Decouple configuration/setup entry points before deleting wrappers; do not bypass Lua ownership for daemon control |
| Filesystems/swap | Kernel ZFS root, supported fstab mounts/checks, already-imported ZFS datasets, owned swap | RC scripts cover additional pool/key/import, diskless, growfs and other unsupported layouts. Keep unsupported policies explicit; do not silently claim parity |
| Runtime directories | Explicit native directory preparation | Traditional cleanvar/cleartmp/virecover and stale-state policies need separate assessment; creating directories is not general temporary-state cleanup |
| Sysctl/device policy | Explicit graph operations and device_rules | General rc.d/sysctl, sysctl_lastload and devfs policy are not automatically imported. Devd module matching now has direct bounded dispatch with loader blocklists; see HALFBSD-DEVD-ACTIONS.md. Native autoload validation is pending |
| Host identity/time | Hostname and foreground NTP | hostid, ZFS host identity and adjkerntz responsibilities need review against the actual installation and kernel settings; process readiness does not prove time synchronization |
| Logging | Foreground syslogd plus structured manager logs | Log rotation, persistent boot diagnostics and RC dmesg/accounting hooks need coverage decisions |
| Ports services | Registered Lua services use halfsvc; service now dispatches by active mode | Unregistered rc.d package scripts remain unsupported in Lua mode. Add explicit service definitions and package integration rather than automatic shell execution |
| Desktop/session | Mandatory seatd/devd, regular-user compositor path and native login | Multi-session/VT/USB, console selector and power-management acceptance remain native gates |

The read-only inspection used the actual `sbin/devd/devd.conf`, `libexec/rc/`
helpers and rc.d scripts, `Makefile.inc1`, and Lua boot definitions. This is a
bounded workstation audit, not a claim to inventory every external package or
administrator script on Anvil. Review the installed `/etc/devd` and
`/usr/local/etc/devd` rules as well; package-provided actions may add callers.

The next reduction should follow evidence: inventory those remaining callers,
decide which supported workstation responsibilities matter, implement their
native/Lua replacements, and test failure/recovery paths. None of this warrants
deleting `/etc/rc`, rc.subr, rcorder or recovery support in the current batch.
