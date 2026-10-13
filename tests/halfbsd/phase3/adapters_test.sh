# SPDX-License-Identifier: BSD-2-Clause
atf_test_case policy
policy_head() { atf_set descr 'Phase III boot adapter graph and ownership policies'; }
policy_body()
{
    modules=$(atf_config_get modules /usr/lib/halfbsd)
    atf_check -s exit:0 -o match:PASS -e empty /usr/bin/lua -E "$(atf_get_srcdir)/policy.lua" "$modules"
}
atf_test_case native
native_head() { atf_set descr 'Read-only inventories, loopback probes and private terminal handoff'; atf_set timeout 60; }
native_body()
{
    [ "$(uname -s)" = FreeBSD ] || atf_skip 'Native FreeBSD required'
    modules=$(atf_config_get modules /usr/lib/halfbsd)
    atf_check -s exit:0 -o match:PASS -e empty /usr/bin/lua -E "$(atf_get_srcdir)/native.lua" "$modules" "$(atf_get_srcdir)/probe_helper"
    atf_check -s exit:0 -o match:PASS -e empty "$(atf_get_srcdir)/terminal_helper"
}
atf_init_test_cases() { atf_add_test_case policy; atf_add_test_case native; }
