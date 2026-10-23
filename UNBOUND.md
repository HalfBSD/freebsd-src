# HalfBSD DNS resolver

Current summary: October 22, 2026. This supersedes the Unbound reduction plans,
per-pass ChatGPT handoffs, configuration/installworld reports and duplicate TODOs.
Detailed diffs and historical measurements remain in Git.

## Architecture and configuration

HalfBSD runs a forwarding, caching, DNSSEC-validating Unbound. Incoming IPv4 UDP
and TCP listen on `0.0.0.0:53` by default. IPFW controls which sources can reach
it; there is no Unbound client-network authorization stage. Positive/negative
cache, prefetch, serve-expired, local-zone/data/PTR, DNS Cookies, EDNS, TCP limits,
logging, statistics, reload and authenticated administration remain.

| Path | Purpose |
| --- | --- |
| `/etc/unbound/unbound.conf` | Administrator-owned static configuration |
| `/var/unbound/root.key` | Mutable DNSSEC trust anchor |
| `/var/run/local_unbound.pid` | Runtime PID |
| `/var/run/local_unbound.ctl` | Local control socket |

No generated forward.conf, control.conf, lan-zones.conf or automatic conf.d
includes remain. Ordinary administrator includes are still supported.
Configuration and record edits are preserved across repeated setup/installation.
Migration handles the old `/etc/unbound -> /var/unbound` symlink and old fragment
layout, backs up consumed files under `/var/backups`, and refuses ambiguous
configurations/custom chroots instead of discarding administrator data.

Default chroot is disabled: the simplified layout otherwise needs duplicate
configuration, mounts or synchronization. This deliberately loses filesystem
containment. Optional core chroot support remains; privilege dropping, DNSSEC,
upstream certificate validation and remote-control authentication remain.
No Capsicum sandbox has been implemented.

## Compiled provider policy

| Priority | Provider | IPv4 addresses |
| --- | --- | --- |
| 1 | Cloudflare | `1.1.1.1`, `1.0.0.1` |
| 2 | Google | `8.8.8.8`, `8.8.4.4` |
| 3 | OpenDNS | `208.67.222.222`, `208.67.220.220` |

The higher provider is used while either address is healthy. Final transport
failures trigger failover; NXDOMAIN, valid SERVFAIL responses and DNSSEC failures
must not cause provider shopping. Recovery backoff is 30, 60, 120, 240, then at
most 300 seconds per address, with a shared recovery lease and two-success
hysteresis. A five-second worker timer schedules normal recovery queries while
idle. Existing RTT selection chooses within the eligible provider.

DHCP configures network addresses/routes but never selects DNS. All-upstream
outage returns SERVFAIL for uncached external requests; local data and existing
cache/serve-expired behavior continue. Catch-all forwarding cannot be removed or
changed to recursive fallback: `forward off`, `forward_remove .` and
`forward-first: yes` are rejected. Shared iterator internals remain because
forwarding, DNSSEC, retry, CNAME and cache processing depend on them.

An explicit configured root forward-zone or runtime forwarding override uses
ordinary configured selection rather than built-in priority. Runtime overrides
are reset by reload. Outbound DNS-over-TLS remains optional; `tls-upstream: yes`
uses port 853 and provider authentication names, with a valid CA bundle such as
`/etc/ssl/cert.pem`. TLS errors never enable plaintext fallback.

## Operate

```sh
halfsvc status dns
local-unbound-checkconf /etc/unbound/unbound.conf
local-unbound-control stats_noreset
local-unbound-control reload
local-unbound-control flush example.com
halfsvc restart dns
```

Inspect the configuration and control socket permissions if control fails.
Native utilities use the `local-unbound` prefix; upstream portable builds use
unbound, unbound-control, unbound-checkconf and unbound-anchor. The required
private libunbound is retained; it is not an unused consumer-free library.

Add ordinary records under the existing server section, using your own address:

```text
local-zone: "home.arpa." static
local-data: "anvil.home.arpa. IN A 192.168.50.10"
local-data: "desktop.home.arpa. IN CNAME anvil.home.arpa."
local-data-ptr: "192.168.50.10 anvil.home.arpa."
```

Validate before reloading. Do not replace the entire configuration with this
snippet. DNSSEC trust-anchor state must remain writable by the Unbound account.
Installworld invokes the source setup/refresh helper directly, validates as the
unprivileged account, and requests `halfsvc restart dns` only if DNS is running.
It does not depend on an installed RC script, restore retired `-f`, start a
second daemon or use `/var/run/ipfw.ready`. A restart acknowledgement is not
proof of final readiness; inspect `halfsvc status dns`. If an old upgrade already
stopped DNS, `halfsvc restart dns` restores the supervised service.

## Reduction and fixes

Permanently removed: DNS64, Python/dynlib processing, dnstap, Redis/cachedb,
EDNS Client Subnet/subnetcache, DNSCrypt, DoH, DoQ, incoming DNS-over-TLS, views,
RPZ, response-IP and source-network/interface ACL authorization. Removed
configuration/parser/build/dependency support, not new disabled build profiles.
Outbound TLS and control TLS remain. Removed dependency support includes Python,
SWIG, frame streams/protobuf-c, hiredis, libsodium, nghttp2 and ngtcp2 dependencies
specific to those features; not all were linked in the original native build.

TCP quota accounting now belongs to each accepted connection, releases exactly
once on close/failure, and safely rebinds active counts during synchronized fast
reload. Mesh/listener/declaration warning repairs preserve strict native builds.
Setup migration, direct installworld dispatch and Lua service ownership fixes
preserve records, TLS settings and anchors.

Historical comparable Linux GCC/LTO measurements: first-pass baseline had
372 files/194,386 C/H/Y/LEX lines; after the ACL pass, 320 files/155,187 lines.
The ACL pass alone reduced stripped daemon size from 1,214,976 to 1,190,464 bytes.
These are dated source/ELF snapshots, not current native release-size claims;
comments/generated parser lines are included and later fixes changed totals.

## Validation and future work

The administrator accepted native RC-free startup and DNS behavior on Anvil,
including the DNS acceptance agenda. Earlier Linux builds compiled all four
utilities and exercised forwarding, cache, local records, DNSSEC, TLS/control,
provider outage/recovery, reload, TCP quotas and configuration migration.
Those results do not imply a native sanitizer run or every failure scenario.

Retained regression entry points (BUILD is the portable build directory):

```sh
python3 usr.sbin/unbound/tests/forwarding.py "$BUILD"
python3 usr.sbin/unbound/tests/providers.py "$BUILD"
python3 usr.sbin/unbound/tests/tcp-limits.py "$BUILD"
python3 usr.sbin/unbound/tests/setup.py "$BUILD"
```

Future independent removals: authority zones/AXFR/IXFR/NOTIFY/update machinery;
stub zones; ipsecmod/ipset; EDNS client-string metadata; review NAT64 transport
separately from DNS64/ordinary IPv6. Audit private-library/auxiliary consumers,
foreign packaging and stale test targets before deletion. Do not rewrite the
shared iterator, compromise standards/security, or remove local records.
Retire migration/setup machinery only after defining an upgrade boundary.

For future Capsicum work, inventory post-init configuration/CA/key reads, log
reopening, trust-anchor atomic writes, outbound sockets, reload and runtime
administration. Do not add a broker or sandbox merely to tick a box. Optimize
cache/thread/memory settings only from measurements. The old ISP resolver
failure's cause was never established; do not label it DNSSEC without evidence.
