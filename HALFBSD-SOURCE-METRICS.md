# HalfBSD source reduction — October 22, 2026

Compared committed HalfBSD `a5c1e1c1bbfebd3e9e99905dcdb03eb418787d04` with patched FreeBSD 15.1
`e38a7085f3a8ecd317c947591fe42b5ca9ab317b` (the same RELEASE-p4 baseline used
by the October 3 report). Also measured the original 15.1 release tag and the
old HalfBSD snapshot. Local uncommitted documentation and this report are excluded.

## Current totals

| Measure | FreeBSD 15.1-p4 | HalfBSD | Net removed | Reduction |
| --- | ---: | ---: | ---: | ---: |
| Tracked blob files | 109,076 | 83,759 | 25,317 | 23.21% |
| Uncompressed bytes | 1,538,792,947 | 1,278,722,596 | 260,070,351 | 16.90% |
| Text newline count | 41,835,225 | 33,762,617 | 8,072,608 | 19.30% |

The tree contains **8,072,608 fewer text lines**, **25,317 fewer files** and
**260,070,351 fewer bytes** (about 248 MiB). These are net changes after additions.

## Source-code subset

The whole-tree total includes documentation, locale data, tests and build input.
For a narrower estimate, files with common source-language suffixes contain:

| Measure | FreeBSD | HalfBSD | Net removed | Reduction |
| --- | ---: | ---: | ---: | ---: |
| Source-suffix files | 53,072 | 44,308 | 8,764 | 16.51% |
| Source-suffix text lines | 27,093,615 | 23,094,385 | 3,999,230 | 14.76% |
| Source-suffix bytes | 940,996,464 | 824,428,853 | 116,567,611 | 12.39% |

That is approximately **4.00 million source-file lines removed net (14.76%)**.
This suffix-based estimate includes comments, blank lines, tests and imported
code; it is not executable statements or production-only code. It misses
extensionless scripts, Makefiles and uncommon languages. The exact suffix set
is recorded in the measurement script.

## Deletions versus additions

With rename detection disabled, comparison to patched FreeBSD gives:

- 25,551 deleted paths, 234 added paths and 1,701 modified paths.
- Text diff: 8,137,645 deleted lines and 64,842 added lines.
- Net diff: 8,072,803 fewer lines. The 195-line difference from tree newline
  totals comes from diff line accounting versus literal newline-byte counting.

Path moves appear as deletion/addition in this view. Net tree totals do not
depend on rename detection. Binary changes are included in file/byte totals,
but excluded from text-line totals.

## Largest directory reductions

Net whole-tree text newline reductions, including each directory’s data/tests:

| Directory | Net lines removed |
| --- | ---: |
| `sys/` | 3,568,995 |
| `crypto/` | 1,207,335 |
| `share/` | 1,128,083 |
| `contrib/` | 1,047,777 |
| `lib/` | 318,103 |
| `usr.sbin/` | 290,603 |
| `sbin/` | 148,461 |
| `usr.bin/` | 137,269 |
| `stand/` | 74,784 |
| `libexec/` | 63,162 |
| `tests/` | 26,117 |
| `bin/` | 18,521 |
| `include/` | 11,773 |
| `krb5/` | 10,231 |

Kernel and driver reductions in `sys/` are the largest component. Directory
totals measure content changes; they do not assign individual removals to
subsystems or prove that every removed line was reachable in the old system.
Implemented removal areas remain summarized in
[HALFBSD-SOURCE-REDUCTION.md](HALFBSD-SOURCE-REDUCTION.md).

## Progress since the previous snapshot

- 8,045 additional files removed net since `18799602c9b0`.
- 82,842,165 additional bytes removed net since `18799602c9b0`.
- 2,858,753 additional text newlines removed net since `18799602c9b0`.

The old report’s file and byte totals reproduce exactly. Its text totals used
slightly different line accounting: this scan counts newline bytes explicitly,
giving 41,835,225 versus 36,621,370 for those old snapshots. Compare fresh
measurements to fresh measurements rather than mixing the two definitions.

## Original unpatched release comparison

The original release commit is `96841ea08dcfa84b954a32dc5ae1a26c28966cf4`.
Against that snapshot, current HalfBSD removes net:

- 25,309 files (23.20%).
- 259,946,371 bytes (16.89%).
- 8,068,643 text newlines (19.29%).

The patched baseline is preferable for isolating HalfBSD changes from upstream
security/errata updates. Neither comparison measures installed image size,
running kernel size, build speed or percentage reduction in attack surface.

## Reproduction

Run from the source checkout; historical Git objects must be available:

```sh
python3 tools/build/halfbsd-source-metrics.py /tmp/halfbsd-source-metrics.json
```

The script reads committed blobs through Git without checking out other trees.
It counts every blob path (including symlink blobs), sums uncompressed blob
sizes, excludes NUL-containing blobs from text totals and counts literal LF bytes.
Duplicate content is counted once per path. `HEAD` means the commit at execution,
so future runs naturally measure a newer snapshot. No installed files are read.

[Raw snapshot and directory metrics](docs/audits/HALFBSD-SOURCE-METRICS-2026-10-22.json)
retain full commit identities and path-change counts.
