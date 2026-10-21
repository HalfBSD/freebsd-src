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
atf_test_case singleboot
singleboot_head() { atf_set descr 'Single-stage ZFS/AMDGPU execution and failure policy without host mutation'; }
singleboot_body()
{
    modules=$(atf_config_get modules /usr/lib/halfbsd)
    atf_check -s exit:0 -o match:PASS -e empty /usr/bin/lua -E "$(atf_get_srcdir)/singleboot.lua" "$modules"
}
atf_test_case console_input
console_input_head() { atf_set descr 'Private PTY console selection, deadlines and interrupted calls'; atf_set timeout 10; }
console_input_body() { atf_check -s exit:0 -o match:PASS -e empty "$(atf_get_srcdir)/console_input_test"; }
atf_test_case workstation
workstation_head() { atf_set descr 'Console policy and diagnostic profile isolation without host mutation'; }
workstation_body()
{
    modules=$(atf_config_get modules /usr/lib/halfbsd)
    atf_check -s exit:0 -o match:PASS -e empty /usr/bin/lua -E "$(atf_get_srcdir)/workstation.lua" "$modules"
}
atf_init_test_cases() { atf_add_test_case singleboot; atf_add_test_case policy; atf_add_test_case handoff; atf_add_test_case console_input; atf_add_test_case workstation; }
