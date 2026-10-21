# HalfBSD workstation account defaults

October 21, 2026. New human login accounts receive supplementary `video`
membership automatically, allowing them to use the workstation seat as themselves.
Console autologin account selection is separate and is not changed here.

## Creation paths

The policy runs in:

- `adduser`, both interactive and batch modes. The installer delegates account
  creation to this utility, so it receives the same behavior.
- `bsdconfig` user creation, both interactive and noninteractive modes.

Both use `/usr/share/halfbsd/accounts.subr`. Existing requested supplementary
groups are preserved and `video` is added only if not already present. Comma-
and whitespace-separated lists are supported. Existing `/etc/adduser.conf`
settings remain intact; even an older empty default group list gets the new
workstation addition when a human account is created. `adduser` shows the final
groups in its confirmation summary. This does not change defaults saved for
other group settings.

These tools are human-account frontends. Their automatic addition excludes:

- Root and explicitly requested UID 0 accounts. Root already owns the seatd
  socket and needs no extra `video` membership.
- Accounts selecting `nologin`, `false` or `true` as their shell.
- Accounts using `/nonexistent` as their home.

Explicit group memberships are retained even for these excluded cases; this
policy does not silently remove an administrator's requested permissions.

Low-level `pw useradd`, `/etc/pw.conf`, package scripts and the shipped service
accounts are unchanged. Use that low-level path for dedicated roles such as
PostgreSQL, including service accounts that deliberately have a real shell and
data directory. Account names or UID ranges cannot reliably distinguish people
from services; this policy does not guess from names such as `postgres` or from
an arbitrary UID cutoff. Accounts made directly through `pw` need an explicit
`-G video` if they are intended to be human workstation accounts.

## Existing users

Installation does not rewrite live group databases or grant privileges to every
existing account. To grant the existing `charlie` account access, as root:

```sh
pw groupmod video -m charlie
id charlie
```

`-m` adds that member while preserving the group's other members. It does not
replace the user's other supplementary groups. Log out and establish a fresh
session before testing the desktop; running processes keep their prior groups.
Apply this deliberately to any other existing human accounts. Root needs no
change. Do not bulk-add dedicated daemon identities.

`video` grants access to the privileged seatd device broker. Membership makes
local GUI use possible; it does not select the console autologin account or
launch a compositor. Seatd still coordinates active sessions and device access.
SSH authentication and simultaneous local/remote sessions remain unchanged.

## Build and testing

This change contains shell policy and build/install wiring, with no C, kernel,
PID 1, seatd or DNS implementation changes.

Portable tests:

```sh
sh /usr/src/tests/halfbsd/accounts/run.sh
sh /usr/src/tests/halfbsd/accounts/build.sh /usr/src make
```

The first tests the shared group policy and executes the actual `adduser` interactive/batch
and `bsdconfig` interactive/noninteractive command-generation paths with mocked
account/UI operations. No active users, groups, homes or datasets are modified.
Cases cover human/SSH users, preserving other groups, avoiding duplicates,
root/UID 0, non-login shells, `/nonexistent` and explicit membership preservation.
It uses a private adduser copy whose policy import is relocated to the checkout.

The second verifies fresh and repeated parallel installation of the shared policy
under a staged SYSROOT with inherited build settings. The scripts are also
syntax-checked. These tests pass on Linux. The isolated case is registered with
ATF under `/usr/tests/halfbsd/accounts`; native ATF and real FreeBSD account
creation have not been executed in this workspace. No full buildworld/build32
or live account-database validation is claimed.

For native acceptance, create a disposable human account on Anvil, verify
`pw groupshow video` / `id ACCOUNT`, and confirm other memberships remain.
Also test a disposable non-login service account and verify that it does not
receive automatic `video` membership. Remove only those disposable accounts
using the ordinary account administration tools. Do not use production service
identities as test fixtures.
