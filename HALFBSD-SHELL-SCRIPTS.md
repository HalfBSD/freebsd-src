# Shell-script inventory

The inventory is [HALFBSD-SHELL-SCRIPTS.tsv](HALFBSD-SHELL-SCRIPTS.tsv): 6,456
entries, with filename, absolute checkout path, brief description and detection
method. Paths refer to `/home/charlie/Development/freebsd-src`, not installed
system paths. This is a source inventory, not a list of programs installed or
executed during boot.

The scan includes tracked files and nonignored working-tree files. It recognizes
shell interpreter shebangs (including env), shell filename suffixes such as
`.sh` and `.sh.in`, sourced `.subr` files, and additional executable files
classified as shell scripts by file(1). It includes Bourne-family and C-shell
scripts, imported sources, developer tools, tests and fixtures. Shell-named
fixtures/fragments are intentionally retained even without executable shebangs.
Git metadata, ignored build output, symlinks and binary files are excluded.

Descriptions are extracted from introductory comments where possible, otherwise
inferred from directory and filename and marked as lacking a descriptive header.
They are an initial navigation aid, not a manual semantic review of 6,456 files.
Detection cannot establish the absence of unmarked, nonexecutable shell fragments
or embedded shell code in Makefiles, C strings and other language files. Those
are outside this standalone-script inventory. Refresh the inventory after source
changes; adding a row does not imply that the script should be removed.
