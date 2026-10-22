# Quieter Lua workstation boot

The three October 21 console photographs show successful automatic root login,
not a second authentication requirement. At the administrator's request,
the selector and its timer/input implementation have now been deleted.
`halfconsole` immediately logs in as root by default, with no prompt or delay.
An explicitly configured `console.user` or existing literal legacy console_user
is still honored. `console.autologin=false` still requests authenticated login.
Existing `select_timeout` values are accepted for upgrades but no longer cause
any waiting or input handling; new configurations omit that obsolete setting.

The photographs also identify two major sources of unnecessary output:

* Every normal service transition appeared twice: a plain progress line and a
  structured JSON logging record. PID-1-launched managers now print transition
  failures only. Structured transition records continue in the configured
  `log_file` (the installed default is `/var/run/halfbsd/boot.log`), without
  duplicating them onto the console. Early failure summaries remain visible.
* Foreground `devd -d` prints debugging details for every event and rule match.
  The workstation graph now uses `devd -d -q`. The existing quiet option now
  also filters foreground output, retaining warnings and errors. Foreground
  ownership, event delivery, and restart/readiness behavior are unchanged.

Seatd now uses its error logging level instead of logging normal connections
and disconnections. The repeated legacy console-user migration notice is
removed; literal legacy account configuration is still honored.

PID 1 startup/readiness/recovery messages, kernel diagnostics, firewall logging,
and daemon errors remain visible. This change does not suppress kernel messages
or alter firewall policy. Boot details can be read with:

```
less /var/run/halfbsd/boot.log
halfsvc status
halfsvc shutdown-report
```

Validation in this Linux workspace: Phase IV portable gate passed, including
policy, immediate console login policy, PID 1 handoff and shutdown-report tests. All 21
Phase III policy groups passed; graph assertions require quiet foreground devd
and error-level seatd. These are portable checks, not native FreeBSD builds or
live boot results. Native buildworld and the next Anvil boot remain pending.

The three photographs are local diagnostic inputs and must not be committed.
The administrator subsequently confirmed that loading hms restores mouse
recovery across USB-switch cycles. HALFBSD now selects hms built into its next
kernel; see HALFBSD-DEVD-HOTPLUG.md. Quiet logging is a separate change.

Selector removal: deleted console_input.c/.h and the obsolete PTY selector test,
removed their build/ATF references, and reran the Phase IV portable gate
successfully. Native helper compilation and boot verification remain pending.


## Native follow-up after installation

The administrator reports the rebuilt system boots successfully. The new console
photograph shows immediate root login without the removed account selector,
and hms0 attaching to the Razer Naga Hex mouse on USB reconnection.

The remaining repeated `Usage in Lua mode: service ...` and unsupported-action
messages are rejected legacy devd actions, not halfinit lifecycle debug output.
Current devd configuration still invokes RC-only devmatch quietstart, dhclient
quietstart and vtcons setkeyboard actions. Generic unmatched devices generate
these attempts repeatedly during USB attachment. Supporting these operations
under Lua requires explicit ownership/policy integration; simply hiding service
errors or falling back to RC would conceal the problem and risk duplicate DHCP.
The photograph alone cannot attribute every rejected invocation to a specific
rule, but these in-tree action call sites are confirmed incompatible.

The separate devd `notify_clients: send() failed; dropping unresponsive client`
warning means delivery to one broadcast subscriber failed and devd removed it.
The current message does not identify the subscriber or errno, so this photograph
cannot establish which client disconnected or whether it was slow versus closed.
USB attachment messages are kernel driver diagnostics, distinct from both issues.


The follow-up implementation is described in HALFBSD-DEVD-ACTIONS.md: recognized
legacy device actions now dispatch by startup ownership, normal subscriber
closure is quiet, and actual slow-client failures remain warnings. Native
validation of this follow-up remains pending.
