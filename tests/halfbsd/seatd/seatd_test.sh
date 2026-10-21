# SPDX-License-Identifier: BSD-2-Clause
atf_test_case private
private_head()
{
    atf_set descr 'Private seatd/libseat protocol and upstream unit tests, no VT/device access'
    atf_set require.user unprivileged
    atf_set require.progs cc
    atf_set timeout 60
}
private_body()
{
    source=$(atf_config_get halfbsd_source /usr/src)
    [ -f "$source/contrib/seatd/seatd/seatd.c" ] || atf_skip 'HalfBSD source tree required'
    atf_check -s exit:0 -o ignore -e ignore env HALFBSD_SOURCE="$source" \
        sh "$(atf_get_srcdir)/run.sh"
}
atf_init_test_cases()
{
    atf_add_test_case private
}
