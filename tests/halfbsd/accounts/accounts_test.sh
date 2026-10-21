# SPDX-License-Identifier: BSD-2-Clause
atf_test_case policy
policy_head()
{
    atf_set descr 'Human account video policy and mocked adduser/bsdconfig creation'
}
policy_body()
{
    source=$(atf_config_get halfbsd_source /usr/src)
    [ -f "$source/usr.sbin/adduser/accounts.subr" ] || atf_skip 'HalfBSD source tree required'
    atf_check -s exit:0 -o ignore -e ignore env HALFBSD_SOURCE="$source" \
        sh "$(atf_get_srcdir)/run.sh"
}
atf_init_test_cases()
{
    atf_add_test_case policy
}
