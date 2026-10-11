# Unbound reduction plan

HalfBSD will keep one Unbound implementation for forwarding and caching DNS.
There will be no new feature switches or compatibility loaders for removed code.
Work will proceed in small groups so that failures can be attributed to a specific
removal. Changes will remain uncommitted for review.

## First pass: independent components

Record source counts before editing. Remove DNS64 synthesis and the Python and
dynamic-library query module loaders first. Delete their implementations, module
registration, callback allowlists, configuration fields, lexer and parser rules,
build rules, configuration probes, and user documentation. Regenerate generated
files from their sources where the necessary tools are available. Check for
remaining references and compile the four retained programs before expanding the
scope. Do not claim runtime verification from a source scan.

Next examine cachedb/Redis, dnstap, and EDNS Client Subnet. Their optional blocks
are candidates for complete deletion, including their dependencies and parser
rules. Remove each only with its callers and resource cleanup accounted for.
Keep ordinary in-memory caches, EDNS handling, logging, and statistics.

## Shared server and policy code

Trace HTTPS, QUIC, DNSCrypt, and incoming TLS listener creation and callbacks.
Remove server-specific implementations while keeping outgoing TLS sessions,
certificate checks, and authenticated remote control. TLS transport helpers must
be classified by their callers before deletion.

Trace views, response-IP, RPZ, authority zones, zone transfers, and stub zones
through daemon setup, worker processing, reloads, cache processing, and iterator
state. Delete independent pieces first. Preserve shared resolver internals where
removing them would require a major rewrite, and record the precise dependency.
Keep local-zone, local-data, reverse records, access controls, and DNSSEC.

## Forwarding enforcement

Keep the iterator's upstream selection, retries, UDP/TCP transport, response
scrubbing, CNAME handling, cache integration, and DNSSEC query support. Require an
explicit root forward-zone with upstreams and no forward-first fallback. Enforce
the invariant during initial configuration, reload, and runtime forward changes.
Test that failure of every forwarder produces an error rather than recursion.
Do not remove recursive iterator states until their forwarding dependencies are
understood and the changes can be independently tested.

## Utilities and integration

Keep unbound, unbound-control, unbound-checkconf, and unbound-anchor. The private
libunbound currently supplies these programs, including anchor initialization;
it is not an unused library. Search the whole tree before deleting any auxiliary
library API or executable.

Review local-unbound-setup, resolvconf, rc scripts, and installation metadata.
The target layout is /etc/unbound/unbound.conf for static configuration,
/etc/unbound/forward.conf for DHCP upstreams, /var/unbound/root.key for mutable
trust anchors, and /var/run for runtime files. Consolidate control.conf and
lan-zones.conf only with their settings preserved. Keep existing installations
working; document substantial migration work separately.

## Verification and reporting

For each independently completed removal, compile the affected programs and run
available upstream regression tests. Verify local A/CNAME/PTR answers, external
forwarding over UDP and TCP, positive and negative cache behavior, prefetch and
serve-expired, DNSSEC success and failure, multiple-forwarder failover, reloads,
and outgoing TLS certificate validation. Use controlled upstream servers where
possible. Keep protocol tests for retained code. Run buildworld and buildkernel
if a usable HalfBSD build environment is available.

Record deleted files separately from build-only changes, source file and line
counts, executable sizes, linked libraries, installed program counts, and actual
test results. Record unavailable checks explicitly. Provide a remaining-work list
instead of representing an incremental pass as the whole reduction.

For future Capsicum work, inventory files opened on reload or log reopening,
trust-anchor reads and atomic updates, control socket and PID operations, and
outgoing socket creation. Keep current privilege dropping, chroot, access control,
and secure trust-anchor behavior. Do not implement a sandbox in this task.

## Initial baseline

The initial working tree was clean. contrib/unbound contained 372 files, including
247 C/header/lexer/parser files with 194,386 lines (including generated files).
HalfBSD installs four Unbound programs. Executable size and actual linked-library
measurements are unavailable until a build can be produced. The library's build
dependencies are OpenSSL libssl/libcrypto and pthread; anchor also uses bsdxml.
The current Linux environment has no visible compiler, make, flex, bison,
autoconf, or OpenSSL development headers, including after an approved host-tool
inspection. Full HalfBSD builds additionally require a supported build host.

## First pass status

The independent DNS64, Python/dynamic-library loader, and dnstap removals are
complete. The baseline, first group, and final group compiled all four programs
with a temporary portable Linux toolchain. The offline runtime suite passed,
including DNSSEC, prefetch, serve-expired, reload, fast reload, and outgoing TLS
certificate verification. Measurements and remaining work are recorded in
UNBOUND-REDUCTION-RESULTS.txt. Full HalfBSD builds and configuration migration
remain pending.

## Second pass status (2026-10-11)

The second pass removes Redis/cachedb, EDNS Client Subnet/subnetcache, DNSCrypt,
DNS-over-HTTPS server handling, and DNS-over-QUIC in that order. Each stage
compiled unbound, unbound-control, unbound-checkconf, and unbound-anchor and
passed the forwarding regression suite before the next subsystem was removed.
Implementations, configuration fields and directives, parser/lexer rules,
registration and event callbacks, dependency probes, generated files, build
rules, statistics, and user documentation were deleted or regenerated.

The iterator was not modified. Shared UDP/TCP transport, caches, DNSSEC, local
records, outgoing TLS, remote-control certificate authentication, privilege
handling, and chroot remain. HTTP/1.1 clients used for downloads also remain.
The regression now covers cache administration, ignored ECS, upstream ECS
non-propagation, pipelined TCP, DNSSEC over TCP, and mutual-TLS control with
reload and fast_reload. Exact measurements, stage results, limitations, deleted
files, and future work are appended to UNBOUND-REDUCTION-RESULTS.txt.

The next separate passes must address incoming DNS-over-TLS, RPZ/response-IP,
views, authority zones and transfers, stub zones, and forwarding-only
configuration enforcement. Configuration path migration and full HalfBSD
builds still require their own validation. This pass adds no build switches,
compatibility support, Capsicum implementation, commit, or push.

## Third pass status: independent removals and iterator constraint (2026-10-11)

Incoming DNS-over-TLS service and client-specific views have been physically
removed. All four retained utilities compile after each removal; the forwarding
suite passed after each stage, with additional source ACL, global local-record
administration, and outgoing TLS reload coverage. Shared outgoing TLS, HTTPS
clients used by anchor, and authenticated control TLS remain.

RPZ and global response-IP removal require a scope decision. iterator.c contains
six RPZ callback calls in three policy-only response-rewriting blocks. Deleting
the RPZ implementation while keeping those calls would break linking; retaining
no-op callbacks would violate the prohibition on compatibility shims. RPZ also
uses the response-IP address trees and rewrite engine. The user prohibited all
iterator changes, so these two subsystems remain supported pending authorization
to delete only the RPZ hooks. No iterator or recursive-resolution code has been
modified. This third pass is therefore partial, not the completion of all four
requested removals. See the third-pass results appendix and handoff report.

Configuration directories and service integration remain unchanged. No Capsicum,
build switches, compatibility layers, commit, or push were introduced.
# 2026-10-11 forwarding-policy update

The built-in provider policy supersedes the earlier requirement to configure an
explicit catch-all zone. Without an explicit root override, the resolver now
installs Cloudflare, Google, then OpenDNS forwarding targets from its source.
Provider selection uses transport health, bounded backoff and periodic recovery;
DNS error answers and DNSSEC validation failures do not change provider priority.
Generated upstream configuration and DHCP upstream selection are removed.
Recursive fallback is prohibited in configuration, runtime control and missing
forward-delegation handling. Configuration directories remain unchanged.

See UNBOUND-DNS-PROVIDERS-HANDOFF.txt for implementation, measurements, tests,
upgrade behavior and native-verification limits. This update accompanies the implementation commit, authorized on 2026-10-11.
