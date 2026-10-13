# SPDX-License-Identifier: BSD-2-Clause
atf_test_case policy
policy_head() { atf_set descr 'Registry, graph, scheduler and protocol policy tests'; }
policy_body()
{
    modules=$(atf_config_get modules /usr/lib/halfbsd)
    atf_check -s exit:0 -o match:PASS -e empty /usr/bin/lua -E "$(atf_get_srcdir)/unit.lua" "$modules"
}
atf_test_case native
native_head() { atf_set descr 'Native userland manager integration and stress, plus Phase I gate'; atf_set timeout 1000; }
native_body()
{
    [ "$(uname -s)" = FreeBSD ] || atf_skip 'Native FreeBSD required'
    source_tree=$(atf_config_get source_tree /usr/src)
    [ -f "$source_tree/tests/halfbsd/run-userland.sh" ] || atf_skip 'Matching source tree required'
    atf_check -s exit:0 -o match:PASS -e empty env HALFBSD_SOURCE="$source_tree" \
        sh "$(atf_get_srcdir)/run.sh"
}
atf_test_case build
build_head() { atf_set descr 'PIE/archive flags and staged parallel module installation'; }
build_body()
{
    source_tree=$(atf_config_get source_tree /usr/src)
    [ -f "$source_tree/share/mk/bsd.prog.mk" ] || atf_skip 'Matching source tree required'
    atf_check -s exit:0 -o match:PASS -e empty sh "$(atf_get_srcdir)/build.sh" "$source_tree"
}
atf_init_test_cases() { atf_add_test_case policy; atf_add_test_case build; atf_add_test_case native; }
