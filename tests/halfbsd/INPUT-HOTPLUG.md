# Optional input hotplug capture

The immediate mouse issue was resolved on Anvil by loading hms; the next
HALFBSD kernel builds it in. This tool remains available for future diagnosis.
Run as root on FreeBSD, leaving Sway running:

```sh
sh /usr/src/tests/halfbsd/input-hotplug.sh /var/tmp/mouse-capture-1 45
```

Use a new absolute directory each time. Switch USB away and back, then move the
mouse. An optional third argument supplies the absolute Sway IPC socket path.
Snapshots discover common Sway socket locations and query GET_INPUTS only.

The private directory contains before/after inventories, periodic snapshots,
devd-events.txt and a summary. Captures include device identities and process
command lines; review before sharing. They contain no raw input events or
keystrokes. No service restarts, driver loads or permission changes occur.

Commands have individual deadlines/output limits; files have an aggregate size
budget. Inventory collection can extend wall-clock runtime beyond the selected
5..60 second monitoring window. Missing utilities are recorded, not installed.
The monitor uses the broadcast devd socket rather than consuming /dev/devctl.
Synchronous inventory collection can delay callbacks; this is diagnostic evidence,
not a guarantee of lossless event capture under load. Native collector execution
is pending; its injected-API regression passes on Linux in the Phase IV gate.
