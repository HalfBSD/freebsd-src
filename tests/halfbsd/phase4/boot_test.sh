# SPDX-License-Identifier: BSD-2-Clause
atf_test_case policy
policy_head() { atf_set descr 'PID 1 readiness and boot policy invariants without host mutation'; }
policy_body()
{
    modules=$(atf_config_get modules /usr/lib/halfbsd)
    atf_check -s exit:0 -o match:PASS -e empty /usr/bin/lua -E "$(atf_get_srcdir)/policy.lua" "$modules" /usr/share/examples/halfbsd/boot-system.lua
}
atf_test_case handoff
handoff_head() { atf_set descr 'Private child pipe transfer, selection, reaping and forced shutdown'; atf_set timeout 15; }
handoff_body() { atf_check -s exit:0 -o match:PASS -e empty "$(atf_get_srcdir)/handoff_test"; }
atf_init_test_cases() { atf_add_test_case policy; atf_add_test_case handoff; }
