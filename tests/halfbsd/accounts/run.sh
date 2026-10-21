#!/bin/sh
# SPDX-License-Identifier: BSD-2-Clause
# No real users/groups, homes, ZFS datasets or host configuration are changed.
set -eu
src=${HALFBSD_SOURCE:-$(realpath "$(dirname "$0")/../../..")}
. "$src/usr.sbin/adduser/accounts.subr"
expect()
{
    wanted=$1; shift
    actual=$(halfbsd_login_groups "$@")
    [ "$actual" = "$wanted" ] || { printf 'expected <%s>, got <%s>\n' "$wanted" "$actual" >&2; exit 1; }
}
expect video alice /bin/sh /home/alice '' 1001
expect video sshonly /bin/sh /home/sshonly '' 1002
expect wheel,audio,video alice /bin/sh /home/alice wheel,audio 1001
expect wheel,video alice /bin/sh /home/alice wheel,video 1001
expect 'video wheel' alice /bin/sh /home/alice 'video wheel' 1001
expect videogames,video alice /bin/sh /home/alice videogames 1001
expect '' root /bin/sh /root '' 0
expect '' aliasroot /bin/sh /root '' 0
expect '' daemon /usr/sbin/nologin /var/empty '' 65
expect '' daemon /bin/false /var/empty '' 65
expect '' daemon /usr/bin/true /var/empty '' 65
expect '' daemon /bin/sh /nonexistent '' 65
# Explicit memberships are retained even for service/root accounts.
expect video daemon /usr/sbin/nologin /nonexistent video 65
expect wheel root /bin/sh /root wheel 0
printf 'PASS shared human-account group policy and service/root exclusions\n'
work=$(mktemp -d "${TMPDIR:-/tmp}/halfbsd-accounts.XXXXXX")
trap 'rm -rf "$work"' EXIT HUP INT TERM
# Relocate only the trusted policy import in a private test copy of adduser.
# pw is always intercepted, and -Z/-D disable dataset/home creation.
awk -v policy="$src/usr.sbin/adduser/accounts.subr" '
    $0 == ". /usr/share/halfbsd/accounts.subr || exit 1" { $0 = ". \"" policy "\" || exit 1" }
    { print }
' "$src/usr.sbin/adduser/adduser.sh" > "$work/adduser"
cat > "$work/pw" <<'MOCK'
#!/bin/sh
case "$1" in
usershow) exit 1 ;;
usernext) echo 1001:1001 ;;
show) [ "$2" = group ] ;;
useradd) printf '%s\n' "$@" > "$PW_CAPTURE" ;;
*) echo "unexpected pw operation: $*" >&2; exit 1 ;;
esac
MOCK
chmod +x "$work/pw"
printf '/bin/sh\n/bin/false\n' > "$work/shells"
batch()
{
    account=$1 shell=$2 home=$3 groups=$4 uid=$5
    printf '%s:%s:::::Fixture:%s:%s:\n' "$account" "$uid" "$home" "$shell" > "$work/users"
    PW_CAPTURE="$work/args" PWCMD="$work/pw" procowner=0 ETCSHELLS="$work/shells" \
        sh "$work/adduser" -N -Z -D -w no -G "$groups" -f "$work/users" > "$work/output" 2>&1
    [ -s "$work/args" ] || { cat "$work/output"; exit 1; }
}
batch alice /bin/sh /home/alice wheel 1001
[ "$(sed -n '/^-G$/{n;p;}' "$work/args")" = wheel,video ]
batch bob /bin/sh /home/bob video 1002
[ "$(sed -n '/^-G$/{n;p;}' "$work/args")" = video ]
batch svc /usr/sbin/nologin /nonexistent '' 65
! grep -q '^-G$' "$work/args"
batch root /bin/sh /root '' 0
! grep -q '^-G$' "$work/args"
printf 'PASS actual adduser batch argv with mocked pw; no active account database touched\n'
printf 'jane\nJane\n\n\nwheel\n\n\n\n\n\n\n\n\n' > "$work/answers"
PW_CAPTURE="$work/args" PWCMD="$work/pw" procowner=0 ETCSHELLS="$work/shells" \
    sh "$work/adduser" -N -Z -D -w no < "$work/answers" > "$work/interactive" 2>&1
[ "$(sed -n '/^-G$/{n;p;}' "$work/args")" = wheel,video ]
grep -q 'Groups.*wheel,video' "$work/interactive"
printf 'PASS actual adduser interactive confirmation and group argv with mocked pw\n'

# Exercise the real bsdconfig function with its UI/database framework stubbed.
awk '/^f_user_add\(\)/ { copy=1 } copy { print } copy && /^}$/ { exit }' \
    "$src/usr.sbin/bsdconfig/usermgmt/share/user.subr" > "$work/bsdconfig-function"
(
    set +u
    . "$work/bsdconfig-function"
    SUCCESS=0 FAILURE=1 DIALOG_OK=0 DIALOG_ESC=255 msg_yes=yes msg_no=no
    for key in NO_CONFIRM USER USER_ACCOUNT_EXPIRE USER_DOTFILES_CREATE USER_GECOS \
        USER_GID USER_GROUPS USER_HOME USER_HOME_CREATE USER_LOGIN_CLASS USER_PASSWORD \
        USER_PASSWORD_EXPIRE USER_SHELL USER_UID; do eval "VAR_$key=$key"; done
    f_getvar()
    {
        local value=
        case "$2" in
        input) value=fixture ;;
        user_shell) value=$fixture_shell ;;
        user_home_dir) value=$fixture_home ;;
        user_member_groups) value=$fixture_groups ;;
        user_uid) value=1001 ;;
        user_home_create) value=no ;;
        esac
        eval "$2=\$value"
    }
    f_quietly() { return 1; } # No real pw invocation.
    f_isset() { return 0; }
    f_interactive() { [ "$interactive" = yes ]; }
    f_shell_escape() { eval "$2=\$1"; }
    f_eval_catch() { for command in "$@"; do :; done; printf '%s\n' "$command" > "$work/bsdconfig-command"; }
    f_show_msg() { :; }
    f_show_info() { :; }
    f_dialog_title() { :; }
    f_dialog_title_restore() { :; }
    f_dialog_noyes() { return 0; }
    f_dialog_menu_user_add() { return 0; }
    f_dialog_menutag_fetch() { eval "$1=X"; }
    f_dprintf() { :; }
    f_user_create_homedir() { echo 'unexpected home creation' >&2; exit 1; }
    f_user_copy_dotfiles() { echo 'unexpected dotfile creation' >&2; exit 1; }
    for interactive in no yes; do
        fixture_shell=/bin/sh fixture_home=/home/fixture fixture_groups=wheel
        f_user_add fixture
        grep -q -- "-G 'wheel,video'" "$work/bsdconfig-command"
        fixture_shell=/usr/sbin/nologin fixture_home=/nonexistent fixture_groups=
        f_user_add fixture
        ! grep -q -- "-G 'video'" "$work/bsdconfig-command"
    done
)
printf 'PASS actual bsdconfig interactive/noninteractive command paths with mocked UI and account operations\n'
