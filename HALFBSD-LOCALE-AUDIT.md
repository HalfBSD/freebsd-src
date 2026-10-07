# HalfBSD locale compatibility audit

**Audit date:** 2026-10-06

**Status:** source review and implementation estimate; proposed changes below are illustrative and have not been applied.

## Conclusion

Using C.UTF-8 as the normal environment is already partly implemented:
usr.bin/login/login.conf:49-50 sets charset=UTF-8 and lang=C.UTF-8 in
the default login class. Removing C/POSIX behavior throughout libc is a
different project. It changes byte-processing behavior and the startup
contract of every C program, including packages outside this repository.

Estimated effort for one engineer with a FreeBSD build/test VM:
* Finish normal-environment defaults, preserving C/POSIX: 1-3 days.
* Make C.UTF-8 the libc startup default, preserving explicit C/POSIX:
  8-15 engineer days, including initialization and regression validation.
* Make C/POSIX names aliases for UTF-8 and eliminate their byte semantics:
  20-35 engineer days for the base system, plus package validation.
* Reject C/POSIX names entirely, migrate in-tree callers and tests:
  30-50 engineer days for the base system, plus ongoing package patches.
These are planning estimates from source inspection, not measured delivery
times. They are alternative scopes, not additive. The final scope cannot
have a finite compatibility guarantee for uninspected third-party software.

## Scope and evidence

Inspected the current working tree, including previous local_unbound and
en_US.UTF-8 edits. No runtime behavior changes were made for this audit.
[HALFBSD-LOCALE-AUDIT.tsv](HALFBSD-LOCALE-AUDIT.tsv) contains reproducible path/line evidence for four
search classes: direct setlocale/newlocale C/POSIX calls, literal environment
assignments, MB_CUR_MAX single-byte branches, and classic/default tables.

| Source group | Matched lines | Distinct files |
|---|---:|---:|
| Native non-test | 97 | 49 |
| Native test | 90 | 57 |
| Imported non-test | 193 | 76 |
| Imported test | 22 | 19 |

Native non-test subcounts:
* Direct setlocale/newlocale calls: 8 lines in 7 files.
* Literal C/POSIX environment settings: 58 lines in 23 files.
* MB_CUR_MAX single-byte/multibyte branches: 17 lines in 11 files.
* Classic/default rune references: 14 lines in 8 files.

Imported non-test direct calls: 15 lines in 13 files.
Native test direct calls: 24 lines in 5 files; environment assignments:
49 lines in 39 files; byte-mode checks: 17 lines in 13 files.
Imported test direct calls: 13 lines in 12 files.
Subcounts overlap. These counts are search hits, not an exact patch size.
The inventory includes retained sources, comments and platform-specific code
that may not be built. Literal single-line searches miss indirect calls,
multiline expressions, generated inputs and assumptions without these names.
The implicit dependencies below are essential even when absent from counts.

## 1. What actually differs

lib/libc/locale/none.c:94 implements C byte conversion: one nonzero input
byte produces one wide character. Both initial ctype objects have
MB_CUR_MAX=1 and a single-byte table limit of 256 (none.c:189 onward).
lib/libc/locale/utf8.c:65 configures MB_CUR_MAX=4 and single-byte limit=128.
UTF-8 decoding checks multibyte sequences and returns conversion errors for
invalid encodings; incomplete sequences require state across calls.

Source-derived examples to verify on FreeBSD, not executed results:
* mbrtowc on bytes C3 A9: C consumes C3 as one character; UTF-8 consumes
  both bytes and produces U+00E9.
* mbrtowc on byte FF: C accepts it; UTF-8 returns an encoding error.
* wcrtomb(U+00E9): C emits one E9 byte; UTF-8 emits C3 A9.
* cut -c 1 on a UTF-8 e-acute: C selects the first byte; UTF-8 selects
  the complete character. Source dispatch: usr.bin/cut/cut.c:130.
* wc -m on UTF-8 text counts different units in the two locales.
  Invalid UTF-8 introduces warning paths (usr.bin/wc/wc.c:299 onward).

C.UTF-8 does retain C-style date, decimal, monetary and message defaults.
lib/libc/locale/ldpart.c:63 uses built-in defaults for C, POSIX and C.*.
lib/libc/locale/collate.c has separate built-in C/POSIX/C.UTF-8 objects
with the same byte-collation fallback. Thus replacing LC_COLLATE=C alone
does not imply linguistic sorting. Changing LC_ALL also changes LC_CTYPE,
which affects decoding, case folding and program algorithm selection.

## 2. Libc implementation work

At least these 10 implementation/header files need coordinated review:
setlocale.c, setrunelocale.c, none.c, utf8.c, table.c, xlocale.c,
xlocale_private.h, collate.c, ldpart.c, runetype.c, all in lib/libc/locale.
Also inspect Symbol.map, ctype inline headers, multibyte wrappers, regex,
and the localedef/locale data generation and install rules.

Required changes if UTF-8 really becomes universal:
* Change actual initial ctype objects and dispatch functions, not just
  current_categories strings or environment variables.
* Change empty-environment fallback, newlocale NULL-name handling,
  querylocale defaults and failed-setlocale rollback (setlocale.c,
  xlocale.c). Define whether old names return old or canonical names.
* Update the built-in locale used by NULL locale handles and partial-mask
  newlocale construction (xlocale_private.h:215), thread rune selection
  (setrunelocale.c:202), locale duplication and reference counting.
* Preserve agreement between global/per-thread rune tables, MB_CUR_MAX,
  legacy __mb_cur_max/__mb_sb_limit, and narrow/wide ctype functions.
* Provide UTF-8 tables before program code and without relying on a mounted
  /usr/share/locale. Current C.UTF-8 loading opens LC_CTYPE from disk;
  global initialization currently uses static tables. An always-UTF-8
  startup needs a static generated UTF-8 table or an equally robust design.
  Calling setlocale from startup would introduce allocation, I/O and
  failure cases and would need constructor/static-link/early-boot review.
* Retain published symbols. _DefaultRuneLocale is exported in Symbol.map;
  deleting its definition can break linkage independently of locale names.

Rejecting names is more disruptive than aliasing them. Callers may fail,
ignore failure and keep a previous locale, or dereference a NULL locale_t.
Aliasing avoids many lookup failures but silently changes existing behavior.

## 3. Native runtime callers: exact direct-call list

bin/date/date.c:213 -- C LC_TIME for RFC 2822 dates. Low effort; verify
  exact English output using C.UTF-8.
lib/libfetch/http.c:883 -- HTTP date parsing. Low effort; verify format
  parsing and restoring the previous locale.
usr.bin/ctags/ctags.c:138 -- C collation. Low effort; compare tag order.
usr.sbin/makefs/makefs.c:111 -- reproducible directory-entry sorting.
  Low/moderate effort; compare image/manifest ordering.
usr.sbin/pw/psdate.c:124 -- private C locale for date parsing. Low effort;
  validate newlocale failure handling and English date forms.
usr.bin/ident/ident.c:247 -- byte character classification. Moderate;
  preserve intended scanning of arbitrary binary files.
usr.bin/sort/sort.c:346,352 -- recognizes C/POSIX names for byte_sort.
  Moderate; verify the C.UTF-8 path, byte ordering, invalid input, and
  case/numeric options. Do not simply rename literals and assume equality.

## 4. Implicit utility and regex dependencies

The 11 native files with explicit MB_CUR_MAX mode branches belong to cut,
hexdump, join, sed, tr, wc and libc regex. Other affected code includes
sort's wide-string conversion, sh output quoting and ls filename rendering.

For each, choose and document behavior for invalid input and arbitrary
filesystem bytes. Filesystem names and file contents are not necessarily
UTF-8. Removing LC_ALL=C eliminates the existing escape hatch for operating
on those bytes. Preserving a comparable capability may require byte flags
or explicit locale-independent scanning paths, not just locale edits.

Review regex character classes, ranges, case-insensitive matching, lengths
and offsets, incomplete sequences at read-buffer boundaries, diagnostics,
exit status and output quoting. Numeric and date semantics are relatively
easy; byte/character semantics consume most compatibility effort.

## 5. Build and imported sources

The 23 native environment-setting files include top-level Makefiles,
METALOG sorting, release manifests, sys/conf scripts, localedef-adjacent
build tools, libsysdecode generators, ncurses rules, getconf, lorder and
zoneinfo. See TSV for every line. Many inputs are ASCII and can be migrated
directly, but compare generated outputs and reproducible metadata.

Cross-build commands run host tools. This repository cannot guarantee that
each supported host has a locale named C.UTF-8. Keep host C settings or
introduce verified host-locale selection if the policy is target-only.
Replacing all host LC_ALL=C assignments needs separate host validation.

Imported runtime review includes:
* awk, xz and ZFS: numeric C settings; C.UTF-8 uses the same decimal defaults.
* file/libmagic: explicit C ctype locales and temporary locale switching;
  check byte classification and magic matching on non-text inputs.
* OpenSSH utf8.c: explicit C fallback; review quoting and invalid-byte output.
* libc-vis fallback: check whether its platform-conditional path is selected.
* libc++: classic facets, locale construction and codecs need review even
  if a direct C call is platform-gated. locale.cpp's __cloc is conditional;
  iostream.cpp's visible newlocale call is Windows-only. Neither is evidence
  of an unconditional FreeBSD startup call. The classic locale and default
  C++ stream assumptions still need targeted tests.

Do not mechanically edit upstream test fixtures or platform-only code.
Identify built paths first; preserve manageable upstream diffs.

## 6. Testing and acceptance work

Native locale tests already depend on the startup C behavior without
calling setlocale. btowc_test.c:60 onward expects single bytes to round-trip;
mbrtowc_test.c:56, mblen_test.c:52, mbtowc_test.c:51 and wcrtomb_test.c:54
assert MB_CUR_MAX==1. Changing names alone leaves these failures intact.
newlocale_test.c tests C/POSIX/C.UTF-8 isolation and object lifetime.
usr.bin/locale/locale.c:559 explicitly adds C and POSIX to locale -a;
its output/tests must match any new name policy.

Required matrix on a FreeBSD VM:
* Environment unset, LANG=C.UTF-8, LC_ALL=C/POSIX, category-only settings,
  explicit/partial newlocale, thread switching and invalid locale names.
* ASCII, valid non-ASCII UTF-8, FF, lone continuation bytes, truncated
  sequences, overlong encodings, embedded NUL, split read buffers, Unicode
  upper/lowercase and non-ASCII whitespace.
* Locale/conversion/ctype/stdio/regex tests, shell expansion/quoting tests,
  cut/tr/sed/grep/awk/sort/join/wc/ls/hexdump/printf tests.
* Mail/HTTP date parsing, pw dates, libmagic, archive filenames, SSH paths
  and terminal output, C++ streams and codecvt, ZFS numeric output.
* buildworld/installworld, reproducible generated outputs, static binaries,
  first boot, single-user mode, recovery/chroot with missing locale files,
  existing-binary smoke tests and representative desktop/server packages.
Do not replace failing byte tests with UTF-8 expectations until deciding
which old behavior is intentionally removed and how byte operations work.

## 7. Work breakdown for aliasing/removing byte semantics

Design: name policy, raw-byte capability, initialization, ABI       2-3 days
Libc defaults, embedded UTF-8 data, thread/object integration       4-7 days
Native callers and utility/regex behavior                         4-7 days
Imported built paths and build/host scripts                       3-5 days
Tests, complete FreeBSD build, boot and regression validation      7-13 days
Total                                                           20-35 days
Rejecting old names adds caller/test/packaging migration: roughly
10-15 days within the base system. Package repair/validation is additional.
Planning assumes one engineer, working build infrastructure and no major
unexpected compatibility defects. Full base validation is the uncertainty,
not the seven straightforward explicit callers.

The estimated initial patch touches roughly 30-50 implementation/config
files plus test changes. This is a planning range; the inventory's 201
distinct files are a review surface, not 201 required edits.

## 8. Standards and external compatibility

POSIX specifies the POSIX locale as the C-program default when setlocale
is not called, and selects it for the C/POSIX environment names. Universal
UTF-8 startup or rejecting these names is a deliberate compatibility
departure. Source references:
[POSIX locale requirements](https://pubs.opengroup.org/onlinepubs/009695399/basedefs/xbd_chap07.html)
[POSIX environment-variable requirements](https://pubs.opengroup.org/onlinepubs/9799919799/basedefs/V1_chap08.html)

Existing binaries can continue linking but change behavior when the same
libc APIs process bytes differently. Package sources, user scripts and
locally compiled software are outside this audit; inventory counts cannot
establish that they are safe. There are no installed C/POSIX locale data
directories to remove, and the UTF-8 implementation is already present.
Any binary-size savings require building and comparing artifacts; embedding
UTF-8 tables for startup could increase libc size instead.

## Limits

This is a static compatibility audit. No modified-libc experiments, FreeBSD
tests, boot validation or package matrix were run. The workspace has no
usable C compiler/BSD make established in the preceding verification.
Exact delivery time and exact final file count require a chosen scope and
a FreeBSD prototype. No locale semantics were changed by this audit.

## 9. Example implementation changes

These examples show the shape of a future patch. They are not a complete
implementation and have not been compiled or run. Which examples are needed
depends on whether the project changes defaults, aliases old names, or rejects
old names. A default-only change leaves intentional explicit C callers intact.

### 9.1 Date and numeric callers

For a policy that removes explicit C uses, the small date changes look like:

```diff
--- a/bin/date/date.c
+++ b/bin/date/date.c
@@
-		setlocale(LC_TIME, "C");
+		setlocale(LC_TIME, "C.UTF-8");
```

Apply the analogous LC_TIME change to `lib/libfetch/http.c`. Verify exact
RFC 2822 output, English weekday/month parsing, and restoration of the saved
locale. These category-specific edits do not change LC_CTYPE.

The `pw` parser constructs a locale object instead:

```diff
--- a/usr.sbin/pw/psdate.c
+++ b/usr.sbin/pw/psdate.c
@@
-	l = newlocale(LC_ALL_MASK, "C", NULL);
+	l = newlocale(LC_ALL_MASK, "C.UTF-8", NULL);
```

That edit changes all selected categories, including character conversion,
and introduces a dependency on the C.UTF-8 ctype data under the current libc.
Review allocation failure before passing `l` to `strptime_l`, and compare all
accepted date forms. The existing call does not establish that changing its
locale string alone would be sufficient.

The numeric C selections in awk, xz, and ZFS can use C.UTF-8's existing
decimal defaults, but should remain category-specific rather than changing
LC_ALL. Compare decimal parsing and machine-readable output byte for byte.

### 9.2 Environment fallback is only one part of libc startup

An unset-environment fallback change would look like:

```diff
--- a/lib/libc/locale/setlocale.c
+++ b/lib/libc/locale/setlocale.c
@@
-	/* 4. if none is set, fall to "C" */
+	/* 4. if none is set, fall to "C.UTF-8" */
     if (env == NULL || !*env)
-		env = "C";
+		env = "C.UTF-8";
```

This affects programs which ask to load the environment, such as through
`setlocale(LC_ALL, "")`. It does **not** change the initial locale before any
`setlocale` call. Nor does renaming `current_categories` change the function
pointers, rune tables, or `MB_CUR_MAX` in the actual initial ctype object.

For true UTF-8 startup, introduce generated static UTF-8 ctype data and
initialize the global object consistently. The following is design pseudocode,
not a paste-ready initializer:

```c
/* All of these must agree before application constructors run. */
initial_global_ctype.name = "C.UTF-8";
initial_global_ctype.runes = embedded_utf8_rune_table;
initial_global_ctype.converters = utf8_converter_functions;
initial_global_ctype.mb_cur_max = 4;
initial_global_ctype.mb_sb_limit = 128;
initial_category_names = "C.UTF-8";
legacy_global_ctype_values = initial_global_ctype.values;
```

The actual patch must account for the currently file-local UTF-8 conversion
functions, generated-data build dependencies, `_CurrentRuneLocale`, thread
rune selection, and the ABI globals. A default-only policy keeps the separate
`__xlocale_C_locale` byte object. A universal policy needs to redesign that
object and its NULL-handle selection too.

### 9.3 Aliasing C/POSIX needs a defined name policy

If old names are accepted as UTF-8 aliases, centralize name normalization
before loading any category. For example, conceptually:

```c
static const char *
canonical_locale_name(const char *name)
{
    if (strcmp(name, "C") == 0 || strcmp(name, "POSIX") == 0)
        return "C.UTF-8";
    return name;
}
```

Do not insert this only in `setrunelocale.c`: callers can enter through both
`setlocale` and `newlocale`, load partial category masks, query saved names,
duplicate objects, and restore locales. Decide whether querying an alias
returns its original spelling or C.UTF-8, then test all those paths.

With strict rejection, validate names before mutating categories, preserve
the existing failure/rollback contract, and migrate callers that ignore
`setlocale` failure or assume `newlocale` always succeeds.

### 9.4 Sort needs an algorithm review

`usr.bin/sort/sort.c` currently selects `byte_sort` by comparing the active
collation name with the names returned after selecting C and POSIX. An explicit
C.UTF-8 branch may be appropriate because its collation tables use the same
fallback, but `byte_sort` participates in more than a display-name choice.

Trace its uses alongside `mb_cur_max`, `bwstring` conversion, case folding,
and numeric options before changing detection. Test both ordering and malformed
input. Matching collation tables does not prove every sort option has matching
character semantics.

### 9.5 Preserve useful operations on bytes

Existing `cut -b` can serve byte selection in many cases. `cut -c` should
remain character selection under UTF-8. Do not change every utility to skip
UTF-8 errors solely to reproduce C behavior: define the expected behavior
per operation, including warnings and exit status.

For regex-driven tools, identify whether a binary-safe mode already exists.
Where it does not, choose between documenting an intentional behavior change
and implementing an explicit byte-processing path. New flags would be a
separate interface change requiring tests and documentation.

### 9.6 Build-script changes need host coverage

For target-side scripts where the locale is guaranteed, an edit can be as small
as `LC_ALL=C.UTF-8 sort ...`. For cross-build rules, host availability is a
separate question. Probe with the actual host libc/tool before selecting a
name; do not assume C.UTF-8 exists on every host or silently accept a failed
locale selection. Compare generated files, manifest ordering and diagnostics.

### 9.7 Locale listing and tests

For a strict one-name policy, remove the unconditional C/POSIX additions in
`usr.bin/locale/locale.c` and adjust its tests. For an alias policy, decide
whether `locale -a` advertises aliases and test the answer.

For startup tests, use explicit expectations for each supported mode:

```c
/* Example assertions for a future UTF-8 startup policy. */
ATF_REQUIRE(MB_CUR_MAX == 4);
ATF_REQUIRE(setlocale(LC_CTYPE, NULL) != NULL);
/* Also assert the expected name, converters, invalid-byte behavior,
 * per-thread state and behavior without installed locale data. */
```

Changing `MB_CUR_MAX == 1` to `== 4` alone is not a sufficient test migration.
Replace byte round-trip expectations with explicit UTF-8 cases where behavior
is intentionally changed; retain byte-mode tests wherever that mode remains
supported.

## 10. Concrete implementation sequence

1. Select the scope and write acceptance rules for C/POSIX names, startup,
   partial locale objects and invalid bytes. Use the estimate for that scope.
2. Establish a FreeBSD VM baseline with current build and test outputs.
   Record which inventory paths are actually compiled by HalfBSD.
3. Implement libc initialization and locale-name policy with focused tests.
   Test static binaries, constructors, threads and missing locale data before
   changing utilities.
4. Migrate date/numeric/collation callers and verify their formats.
5. Resolve byte-processing behavior in utilities, regex, libmagic, SSH and
   C++ facets/codecs. Keep each behavior change reviewable independently.
6. Migrate target scripts; validate host tools and generated outputs separately.
7. Update locale listing and tests; run the complete validation matrix.
8. Compare built artifacts for size and ABI compatibility. Run boot/recovery
   checks and a representative package matrix before making the new default
   the release behavior.

The prototype should narrow the estimate after steps 2–3. An implementation
cannot responsibly claim exact package compatibility from a source-name search.

## 11. Complete matched-file index

The following index lists every file in the inventory, with matched source
line numbers. Links open the files; the TSV retains the exact matched text
and search category. Line numbers describe the audited working tree and may
move after future changes. Files absent from this index can still depend on
the implicit startup locale.

### Native implementation and build files

| File | Matched lines |
|---|---|
| [Makefile](Makefile) | 406, 443, 616, 641, 649, 708, 718, 733, 740, 742, 766, 776, 786 |
| [Makefile.inc1](Makefile.inc1) | 1012, 1239, 1245, 1298, 1302, 1571, 1578, 1725, 1767, 1871, 1878, 1887, 1894 |
| [bin/date/date.c](bin/date/date.c) | 213 |
| [include/_ctype.h](include/_ctype.h) | 126 |
| [include/runetype.h](include/runetype.h) | 86 |
| [lib/libc/locale/Symbol.map](lib/libc/locale/Symbol.map) | 73 |
| [lib/libc/locale/none.c](lib/libc/locale/none.c) | 191, 203 |
| [lib/libc/locale/setrunelocale.c](lib/libc/locale/setrunelocale.c) | 72, 87, 105, 202 |
| [lib/libc/locale/table.c](lib/libc/locale/table.c) | 45, 250 |
| [lib/libc/locale/xlocale.c](lib/libc/locale/xlocale.c) | 87 |
| [lib/libc/locale/xlocale_private.h](lib/libc/locale/xlocale_private.h) | 177, 215 |
| [lib/libc/regex/engine.c](lib/libc/regex/engine.c) | 159 |
| [lib/libc/regex/regcomp.c](lib/libc/regex/regcomp.c) | 1147, 1149, 1882 |
| [lib/libc/regex/regexec.c](lib/libc/regex/regexec.c) | 221 |
| [lib/libfetch/http.c](lib/libfetch/http.c) | 883 |
| [lib/libsysdecode/mkioctls](lib/libsysdecode/mkioctls) | 13, 28 |
| [lib/libsysdecode/mklinuxtables](lib/libsysdecode/mklinuxtables) | 32 |
| [lib/libsysdecode/mktables](lib/libsysdecode/mktables) | 35 |
| [lib/ncurses/ncurses/Makefile](lib/ncurses/ncurses/Makefile) | 174 |
| [lib/ncurses/tinfo/Makefile](lib/ncurses/tinfo/Makefile) | 236, 273 |
| [release/Makefile](release/Makefile) | 173 |
| [release/tools/vmimage.subr](release/tools/vmimage.subr) | 120, 305 |
| [sbin/route/Makefile](sbin/route/Makefile) | 33 |
| [share/zoneinfo/Makefile](share/zoneinfo/Makefile) | 103, 108, 115 |
| [stand/common/newvers.sh](stand/common/newvers.sh) | 45 |
| [sys/conf/Makefile.arm](sys/conf/Makefile.arm) | 95 |
| [sys/conf/Makefile.arm64](sys/conf/Makefile.arm64) | 91 |
| [sys/conf/newvers.sh](sys/conf/newvers.sh) | 185 |
| [sys/dev/bhnd/tools/nvram_map_gen.sh](sys/dev/bhnd/tools/nvram_map_gen.sh) | 8 |
| [tools/build/absolute-symlink.sh](tools/build/absolute-symlink.sh) | 24 |
| [tools/make_libdeps.sh](tools/make_libdeps.sh) | 32 |
| [tools/tools/sysdoc/sysdoc.sh](tools/tools/sysdoc/sysdoc.sh) | 27 |
| [usr.bin/ctags/ctags.c](usr.bin/ctags/ctags.c) | 138 |
| [usr.bin/cut/cut.c](usr.bin/cut/cut.c) | 130, 132 |
| [usr.bin/getaddrinfo/Makefile](usr.bin/getaddrinfo/Makefile) | 16 |
| [usr.bin/getconf/Makefile](usr.bin/getconf/Makefile) | 19, 22, 37 |
| [usr.bin/hexdump/conv.c](usr.bin/hexdump/conv.c) | 98 |
| [usr.bin/ident/ident.c](usr.bin/ident/ident.c) | 247 |
| [usr.bin/join/join.c](usr.bin/join/join.c) | 397 |
| [usr.bin/lorder/lorder.sh](usr.bin/lorder/lorder.sh) | 32, 33 |
| [usr.bin/sed/compile.c](usr.bin/sed/compile.c) | 802 |
| [usr.bin/sed/process.c](usr.bin/sed/process.c) | 498 |
| [usr.bin/sort/sort.c](usr.bin/sort/sort.c) | 346, 352 |
| [usr.bin/tr/str.c](usr.bin/tr/str.c) | 219, 259 |
| [usr.bin/tr/tr.c](usr.bin/tr/tr.c) | 287 |
| [usr.bin/vi/catalog/Makefile](usr.bin/vi/catalog/Makefile) | 37, 83, 89, 99 |
| [usr.bin/wc/wc.c](usr.bin/wc/wc.c) | 224, 299, 336 |
| [usr.sbin/makefs/makefs.c](usr.sbin/makefs/makefs.c) | 111 |
| [usr.sbin/pw/psdate.c](usr.sbin/pw/psdate.c) | 124 |

### Native tests

| File | Matched lines |
|---|---|
| [bin/hostname/tests/hostname_test.sh](bin/hostname/tests/hostname_test.sh) | 33 |
| [bin/sh/tests/builtins/cd8.0](bin/sh/tests/builtins/cd8.0) | 5 |
| [bin/sh/tests/builtins/locale1.0](bin/sh/tests/builtins/locale1.0) | 26, 32, 38 |
| [bin/sh/tests/builtins/locale2.0](bin/sh/tests/builtins/locale2.0) | 2 |
| [bin/sh/tests/expansion/pathname1.0](bin/sh/tests/expansion/pathname1.0) | 3 |
| [bin/sh/tests/expansion/pathname2.0](bin/sh/tests/expansion/pathname2.0) | 3 |
| [cddl/usr.sbin/dtrace/tests/tools/genmakefiles.sh](cddl/usr.sbin/dtrace/tests/tools/genmakefiles.sh) | 90 |
| [lib/libc/tests/locale/btowc_test.c](lib/libc/tests/locale/btowc_test.c) | 57 |
| [lib/libc/tests/locale/mblen_test.c](lib/libc/tests/locale/mblen_test.c) | 52, 77 |
| [lib/libc/tests/locale/mbrlen_test.c](lib/libc/tests/locale/mbrlen_test.c) | 52, 80 |
| [lib/libc/tests/locale/mbrtowc_test.c](lib/libc/tests/locale/mbrtowc_test.c) | 56, 99 |
| [lib/libc/tests/locale/mbsnrtowcs_test.c](lib/libc/tests/locale/mbsnrtowcs_test.c) | 156 |
| [lib/libc/tests/locale/mbsrtowcs_test.c](lib/libc/tests/locale/mbsrtowcs_test.c) | 131 |
| [lib/libc/tests/locale/mbstowcs_test.c](lib/libc/tests/locale/mbstowcs_test.c) | 93 |
| [lib/libc/tests/locale/mbtowc_test.c](lib/libc/tests/locale/mbtowc_test.c) | 51, 78 |
| [lib/libc/tests/locale/wcrtomb_test.c](lib/libc/tests/locale/wcrtomb_test.c) | 54 |
| [lib/libc/tests/locale/wcsnrtombs_test.c](lib/libc/tests/locale/wcsnrtombs_test.c) | 158 |
| [lib/libc/tests/locale/wcsrtombs_test.c](lib/libc/tests/locale/wcsrtombs_test.c) | 129 |
| [lib/libc/tests/locale/wcstombs_test.c](lib/libc/tests/locale/wcstombs_test.c) | 107 |
| [lib/libc/tests/locale/wctomb_test.c](lib/libc/tests/locale/wctomb_test.c) | 64 |
| [lib/libc/tests/stdio/printbasic_test.c](lib/libc/tests/stdio/printbasic_test.c) | 99, 142 |
| [lib/libc/tests/stdio/printfloat_test.c](lib/libc/tests/stdio/printfloat_test.c) | 91, 123, 144, 157, 171, 193, 202, 218, 231, 266, 297, 338 |
| [lib/libc/tests/stdio/scanfloat_test.c](lib/libc/tests/stdio/scanfloat_test.c) | 157, 206, 284 |
| [lib/libc/tests/stdlib/strfmon_test.c](lib/libc/tests/stdlib/strfmon_test.c) | 235 |
| [lib/libc/tests/string/wcscasecmp_test.c](lib/libc/tests/string/wcscasecmp_test.c) | 40, 51, 63, 79, 91, 105 |
| [tests/sys/cddl/zfs/include/constants.cfg](tests/sys/cddl/zfs/include/constants.cfg) | 38, 39 |
| [tests/sys/cddl/zfs/tests/cli_root/zfs_diff/zfs_diff_001_pos.ksh](tests/sys/cddl/zfs/tests/cli_root/zfs_diff/zfs_diff_001_pos.ksh) | 69 |
| [tests/sys/cddl/zfs/tests/cli_user/zfs_list/zfs_list_002_pos.ksh](tests/sys/cddl/zfs/tests/cli_user/zfs_list/zfs_list_002_pos.ksh) | 105 |
| [tests/sys/cddl/zfs/tests/cli_user/zfs_list/zfs_list_005_pos.ksh](tests/sys/cddl/zfs/tests/cli_user/zfs_list/zfs_list_005_pos.ksh) | 93 |
| [tools/test/sort/regression/Makefile](tools/test/sort/regression/Makefile) | 15, 16, 21, 22 |
| [tools/test/stress2/default.cfg](tools/test/stress2/default.cfg) | 70 |
| [tools/test/stress2/misc/bench.sh](tools/test/stress2/misc/bench.sh) | 34 |
| [tools/test/stress2/misc/burnin.sh](tools/test/stress2/misc/burnin.sh) | 34 |
| [tools/test/stress2/misc/dumpfs.sh](tools/test/stress2/misc/dumpfs.sh) | 32 |
| [tools/test/stress2/misc/fsck.sh](tools/test/stress2/misc/fsck.sh) | 106 |
| [tools/test/stress2/misc/fsck10.sh](tools/test/stress2/misc/fsck10.sh) | 93 |
| [tools/test/stress2/misc/fsck11.sh](tools/test/stress2/misc/fsck11.sh) | 97 |
| [tools/test/stress2/misc/fsck12.sh](tools/test/stress2/misc/fsck12.sh) | 96 |
| [tools/test/stress2/misc/fsck13.sh](tools/test/stress2/misc/fsck13.sh) | 106 |
| [tools/test/stress2/misc/fsck8.sh](tools/test/stress2/misc/fsck8.sh) | 98 |
| [tools/test/stress2/misc/fsck9.sh](tools/test/stress2/misc/fsck9.sh) | 96 |
| [tools/test/stress2/misc/nullfs11.sh](tools/test/stress2/misc/nullfs11.sh) | 35 |
| [tools/test/stress2/misc/pthread2.sh](tools/test/stress2/misc/pthread2.sh) | 33 |
| [tools/test/stress2/misc/pthread4.sh](tools/test/stress2/misc/pthread4.sh) | 33 |
| [tools/test/stress2/misc/pthread7.sh](tools/test/stress2/misc/pthread7.sh) | 33 |
| [tools/test/stress2/misc/sched.sh](tools/test/stress2/misc/sched.sh) | 54 |
| [tools/test/stress2/misc/snap10.sh](tools/test/stress2/misc/snap10.sh) | 52 |
| [tools/test/stress2/misc/snap11.sh](tools/test/stress2/misc/snap11.sh) | 65 |
| [tools/test/stress2/misc/split.sh](tools/test/stress2/misc/split.sh) | 52 |
| [tools/test/stress2/misc/tar.sh](tools/test/stress2/misc/tar.sh) | 42 |
| [tools/test/stress2/tools/fast.sh](tools/test/stress2/tools/fast.sh) | 40 |
| [tools/test/stress2/tools/ministat.sh](tools/test/stress2/tools/ministat.sh) | 36 |
| [usr.bin/bmake/unit-tests/Makefile](usr.bin/bmake/unit-tests/Makefile) | 804, 805 |
| [usr.bin/gh-bc/tests/Makefile](usr.bin/gh-bc/tests/Makefile) | 60, 65 |
| [usr.bin/locale/tests/locale_test.sh](usr.bin/locale/tests/locale_test.sh) | 36, 103, 170 |
| [usr.bin/printf/tests/regress.sh](usr.bin/printf/tests/regress.sh) | 10 |
| [usr.bin/wc/tests/wc_test.sh](usr.bin/wc/tests/wc_test.sh) | 12 |

### Imported implementation and build files

| File | Matched lines |
|---|---|
| [contrib/bc/configure.sh](contrib/bc/configure.sh) | 452 |
| [contrib/bc/gen/strgen.sh](contrib/bc/gen/strgen.sh) | 30, 31 |
| [contrib/bmake/ChangeLog](contrib/bmake/ChangeLog) | 4187 |
| [contrib/bmake/configure](contrib/bmake/configure) | 57, 7509 |
| [contrib/byacc/aclocal.m4](contrib/byacc/aclocal.m4) | 1022, 1023, 1024, 2016, 2044 |
| [contrib/byacc/configure](contrib/byacc/configure) | 70, 71, 72, 73, 75, 76, 77, 7095, 7096, 7097, 7554, 7584, 7807, 7808, 7809, 7810, 7812, 7813, 7814 |
| [contrib/byacc/install-sh](contrib/byacc/install-sh) | 485, 486 |
| [contrib/elftoolchain/common/native-elf-format](contrib/elftoolchain/common/native-elf-format) | 22 |
| [contrib/file/configure](contrib/file/configure) | 57, 15055, 16556 |
| [contrib/file/ltmain.sh](contrib/file/ltmain.sh) | 136 |
| [contrib/file/magic/Magdir/console](contrib/file/magic/Magdir/console) | 758 |
| [contrib/file/magic/scripts/create_filemagic_flac](contrib/file/magic/scripts/create_filemagic_flac) | 8, 54 |
| [contrib/file/src/apprentice.c](contrib/file/src/apprentice.c) | 591 |
| [contrib/file/src/funcs.c](contrib/file/src/funcs.c) | 731, 763 |
| [contrib/file/src/readcdf.c](contrib/file/src/readcdf.c) | 118, 127 |
| [contrib/ldns/configure](contrib/ldns/configure) | 57, 19680 |
| [contrib/ldns/drill/install-sh](contrib/ldns/drill/install-sh) | 468, 469 |
| [contrib/ldns/install-sh](contrib/ldns/install-sh) | 485, 486 |
| [contrib/libc-vis/vis.c](contrib/libc-vis/vis.c) | 112 |
| [contrib/libedit/el.c](contrib/libedit/el.c) | 297 |
| [contrib/libevent/build-aux/install-sh](contrib/libevent/build-aux/install-sh) | 480, 481 |
| [contrib/libevent/configure](contrib/libevent/configure) | 127, 18962 |
| [contrib/libpcap/configure](contrib/libpcap/configure) | 55, 14141 |
| [contrib/libpcap/install-sh](contrib/libpcap/install-sh) | 485, 486 |
| [contrib/llvm-project/compiler-rt/lib/sanitizer_common/symbolizer/scripts/build_symbolizer.sh](contrib/llvm-project/compiler-rt/lib/sanitizer_common/symbolizer/scripts/build_symbolizer.sh) | 194 |
| [contrib/llvm-project/libcxx/include/__chrono/formatter.h](contrib/llvm-project/libcxx/include/__chrono/formatter.h) | 628 |
| [contrib/llvm-project/libcxx/include/__locale_dir/locale_base_api/ibm.h](contrib/llvm-project/libcxx/include/__locale_dir/locale_base_api/ibm.h) | 34 |
| [contrib/llvm-project/libcxx/include/__thread/thread.h](contrib/llvm-project/libcxx/include/__thread/thread.h) | 141 |
| [contrib/llvm-project/libcxx/src/iostream.cpp](contrib/llvm-project/libcxx/src/iostream.cpp) | 110 |
| [contrib/llvm-project/libcxx/src/locale.cpp](contrib/llvm-project/libcxx/src/locale.cpp) | 77, 225, 499, 509, 735, 746, 757, 768, 819, 832, 845, 858, 981 |
| [contrib/ncurses/configure](contrib/ncurses/configure) | 70, 71, 72, 73, 75, 76, 77, 30204, 30205, 30206, 30207, 30209, 30210, 30211 |
| [contrib/ncurses/install-sh](contrib/ncurses/install-sh) | 492, 493 |
| [contrib/ncurses/man/MKterminfo.sh](contrib/ncurses/man/MKterminfo.sh) | 46, 47, 48, 49, 50 |
| [contrib/ncurses/misc/csort](contrib/ncurses/misc/csort) | 33, 35, 36 |
| [contrib/ncurses/misc/ncurses-config.in](contrib/ncurses/misc/ncurses-config.in) | 34, 36, 37 |
| [contrib/ncurses/ncurses/Makefile.in](contrib/ncurses/ncurses/Makefile.in) | 256 |
| [contrib/ncurses/ncurses/base/MKlib_gen.sh](contrib/ncurses/ncurses/base/MKlib_gen.sh) | 60, 61, 62, 63, 64 |
| [contrib/ncurses/progs/MKtermsort.sh](contrib/ncurses/progs/MKtermsort.sh) | 40, 41, 42, 43, 44 |
| [contrib/ntp/configure](contrib/ntp/configure) | 57, 35424 |
| [contrib/ntp/scripts/build/mkver.in](contrib/ntp/scripts/build/mkver.in) | 21, 22, 24 |
| [contrib/ntp/sntp/configure](contrib/ntp/sntp/configure) | 57, 28515 |
| [contrib/ntp/sntp/libevent/build-aux/install-sh](contrib/ntp/sntp/libevent/build-aux/install-sh) | 452, 453 |
| [contrib/ntp/sntp/libevent/configure](contrib/ntp/sntp/libevent/configure) | 55, 20281 |
| [contrib/one-true-awk/bugs-fixed/space.awk](contrib/one-true-awk/bugs-fixed/space.awk) | 11 |
| [contrib/one-true-awk/main.c](contrib/one-true-awk/main.c) | 140 |
| [contrib/openbsm/config/config.guess](contrib/openbsm/config/config.guess) | 907, 908 |
| [contrib/openbsm/configure](contrib/openbsm/configure) | 129, 14579 |
| [contrib/openpam/configure](contrib/openpam/configure) | 57, 20508 |
| [contrib/openpam/install-sh](contrib/openpam/install-sh) | 485, 486 |
| [contrib/openpam/ltmain.sh](contrib/openpam/ltmain.sh) | 136 |
| [contrib/tcpdump/configure](contrib/tcpdump/configure) | 55, 9946 |
| [contrib/tcpdump/install-sh](contrib/tcpdump/install-sh) | 485, 486 |
| [contrib/tzcode/Makefile](contrib/tzcode/Makefile) | 601, 756, 872, 875, 977, 978, 1116 |
| [contrib/tzcode/tzselect.ksh](contrib/tzcode/tzselect.ksh) | 746, 747 |
| [contrib/tzdata/Makefile](contrib/tzdata/Makefile) | 602, 757, 873, 876, 978, 979, 1094, 1095, 1128 |
| [contrib/unbound/configure](contrib/unbound/configure) | 57, 26516 |
| [contrib/unbound/install-sh](contrib/unbound/install-sh) | 485, 486 |
| [contrib/unbound/ltmain.sh](contrib/unbound/ltmain.sh) | 136 |
| [contrib/unifdef/unifdefall.sh](contrib/unifdef/unifdefall.sh) | 46 |
| [contrib/xz/ChangeLog](contrib/xz/ChangeLog) | 985, 995, 3570 |
| [contrib/xz/src/liblzma/validate_map.sh](contrib/xz/src/liblzma/validate_map.sh) | 84 |
| [contrib/xz/src/xz/args.c](contrib/xz/src/xz/args.c) | 453 |
| [crypto/openssh/freebsd-namespace.sh](crypto/openssh/freebsd-namespace.sh) | 13 |
| [crypto/openssh/install-sh](crypto/openssh/install-sh) | 485, 486 |
| [crypto/openssh/utf8.c](crypto/openssh/utf8.c) | 350 |
| [crypto/openssl/Configurations/unix-Makefile.tmpl](crypto/openssl/Configurations/unix-Makefile.tmpl) | 535, 536, 537 |
| [sys/contrib/openzfs/cmd/mount_zfs.c](sys/contrib/openzfs/cmd/mount_zfs.c) | 165 |
| [sys/contrib/openzfs/cmd/zfs/zfs_main.c](sys/contrib/openzfs/cmd/zfs/zfs_main.c) | 9426 |
| [sys/contrib/openzfs/cmd/zpool/zpool_main.c](sys/contrib/openzfs/cmd/zpool/zpool_main.c) | 13827 |
| [sys/contrib/openzfs/config/ax_code_coverage.m4](sys/contrib/openzfs/config/ax_code_coverage.m4) | 148 |
| [sys/contrib/openzfs/config/config.rpath](sys/contrib/openzfs/config/config.rpath) | 652 |
| [sys/contrib/openzfs/config/host-cpu-c-abi.m4](sys/contrib/openzfs/config/host-cpu-c-abi.m4) | 139 |
| [sys/contrib/openzfs/config/lib-prefix.m4](sys/contrib/openzfs/config/lib-prefix.m4) | 216, 217, 219 |
| [sys/contrib/openzfs/config/po.m4](sys/contrib/openzfs/config/po.m4) | 81, 85 |
| [sys/contrib/openzfs/config/rpm.am](sys/contrib/openzfs/config/rpm.am) | 85, 106 |
| [sys/contrib/openzfs/scripts/kmodtool](sys/contrib/openzfs/scripts/kmodtool) | 70 |

### Imported tests

| File | Matched lines |
|---|---|
| [contrib/bmake/unit-tests/Makefile](contrib/bmake/unit-tests/Makefile) | 801, 802 |
| [contrib/libarchive/libarchive/test/main.c](contrib/libarchive/libarchive/test/main.c) | 2656, 2670 |
| [contrib/libarchive/libarchive/test/test_entry.c](contrib/libarchive/libarchive/test/test_entry.c) | 970 |
| [contrib/libarchive/libarchive/test/test_pax_filename_encoding.c](contrib/libarchive/libarchive/test/test_pax_filename_encoding.c) | 203 |
| [contrib/libarchive/test_utils/test_main.c](contrib/libarchive/test_utils/test_main.c) | 3569 |
| [contrib/netbsd-tests/lib/libc/gen/t_vis.c](contrib/netbsd-tests/lib/libc/gen/t_vis.c) | 169 |
| [contrib/netbsd-tests/lib/libc/locale/t_mbrtowc.c](contrib/netbsd-tests/lib/libc/locale/t_mbrtowc.c) | 132 |
| [contrib/netbsd-tests/lib/libc/locale/t_mbsnrtowcs.c](contrib/netbsd-tests/lib/libc/locale/t_mbsnrtowcs.c) | 75 |
| [contrib/netbsd-tests/lib/libc/locale/t_mbstowcs.c](contrib/netbsd-tests/lib/libc/locale/t_mbstowcs.c) | 154 |
| [contrib/netbsd-tests/lib/libc/locale/t_mbtowc.c](contrib/netbsd-tests/lib/libc/locale/t_mbtowc.c) | 78 |
| [contrib/netbsd-tests/lib/libc/locale/t_wctomb.c](contrib/netbsd-tests/lib/libc/locale/t_wctomb.c) | 111 |
| [contrib/netbsd-tests/lib/libc/string/t_strerror.c](contrib/netbsd-tests/lib/libc/string/t_strerror.c) | 128 |
| [contrib/netbsd-tests/usr.bin/cut/t_cut.sh](contrib/netbsd-tests/usr.bin/cut/t_cut.sh) | 97 |
| [crypto/openssh/regress/agent-pkcs11-cert.sh](crypto/openssh/regress/agent-pkcs11-cert.sh) | 8 |
| [crypto/openssh/regress/agent-pkcs11-restrict.sh](crypto/openssh/regress/agent-pkcs11-restrict.sh) | 79 |
| [crypto/openssh/regress/agent-restrict.sh](crypto/openssh/regress/agent-restrict.sh) | 55 |
| [crypto/openssh/regress/unittests/utf8/tests.c](crypto/openssh/regress/unittests/utf8/tests.c) | 84 |
| [sys/contrib/openzfs/tests/zfs-tests/include/default.cfg.in](sys/contrib/openzfs/tests/zfs-tests/include/default.cfg.in) | 54, 55 |
| [sys/contrib/openzfs/tests/zfs-tests/tests/functional/cli_user/zfs_list/zfs_list_002_pos.ksh](sys/contrib/openzfs/tests/zfs-tests/tests/functional/cli_user/zfs_list/zfs_list_002_pos.ksh) | 97 |
