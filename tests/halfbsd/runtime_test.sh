# SPDX-License-Identifier: BSD-2-Clause
run_case()
{
	[ "$(uname -s)" = FreeBSD ] || atf_skip "native FreeBSD kernel required"
	lua=$(atf_config_get lua /usr/bin/lua)
	[ -x "$lua" ] || atf_skip "build/install the HalfBSD Lua interpreter first"
	atf_check -s exit:0 -o match:PASS -e empty "$lua" -E \
	    "$(atf_get_srcdir)/runtime.lua" "$1" "$(atf_get_srcdir)/proc_helper"
}
atf_test_case processes
processes_head() { atf_set "descr" "Direct execution, errors, environments, descriptors, pipes and timeouts"; atf_set "timeout" "60"; }
processes_body() { run_case processes; }
atf_test_case lifecycle
lifecycle_head() { atf_set "descr" "Signals, concurrency, groups, GC ownership and autonomous child reaping"; atf_set "timeout" "120"; }
lifecycle_body() { run_case lifecycle; }
atf_test_case events
events_head() { atf_set "descr" "Native kqueue process, timer, signal, IO, vnode and cancellation events"; atf_set "timeout" "60"; }
events_body() { run_case events; }
atf_test_case filesystem
filesystem_head() { atf_set "descr" "Temporary filesystem operations, atomic replacement and metadata"; }
filesystem_body() { run_case filesystem; }
atf_test_case logging
logging_head() { atf_set "descr" "File logging, escaping, severity and structured failures"; }
logging_body() { run_case logging; }
atf_test_case credentials
credentials_head() { atf_set "descr" "Drop child credentials only, with explicit supplementary groups"; atf_set "require.user" "root"; }
credentials_body() { run_case credentials; }
atf_test_case ownership
ownership_head() { atf_set "descr" "Closing Lua state leaves children alive and reaps them without leaking descriptors"; atf_set "timeout" "30"; }
ownership_body()
{
	[ "$(uname -s)" = FreeBSD ] || atf_skip "native FreeBSD kernel required"
	atf_check -s exit:0 -o match:PASS -e empty "$(atf_get_srcdir)/ownership_helper" "$(atf_get_srcdir)/proc_helper"
}
atf_test_case headers
headers_head() { atf_set "descr" "Lua headers install into fresh bootstrap and world directories without races"; }
headers_body()
{
	source_tree=$(atf_config_get source_tree /usr/src)
	[ -f "$source_tree/lib/liblua/Makefile" ] || atf_skip "source tree required (source_tree configuration)"
	atf_check -s exit:0 -o match:PASS -e empty sh \
	    "$(atf_get_srcdir)/header_install.sh" "$source_tree" "$(atf_config_get bmake make)"
}
atf_test_case libraries
libraries_head() { atf_set "descr" "Foundation archives and interpreter linkage survive inherited SHLIB_MAJOR"; }
libraries_body()
{
	source_tree=$(atf_config_get source_tree /usr/src)
	[ -f "$source_tree/lib/liblua/Makefile" ] || atf_skip "source tree required (source_tree configuration)"
	atf_check -s exit:0 -o match:PASS -e empty sh \
	    "$(atf_get_srcdir)/library_build.sh" "$source_tree" "$(atf_config_get bmake make)"
}
atf_test_case supervision
supervision_head() { atf_set "descr" "Isolated userland startup, supervision and shutdown scenarios"; atf_set "timeout" "300"; }
supervision_body()
{
	[ "$(uname -s)" = FreeBSD ] || atf_skip "native FreeBSD kernel required"
	atf_check -s exit:0 -o match:PASS -e empty env \
	    HALFBSD_LUA="$(atf_config_get lua /usr/bin/lua)" \
	    sh "$(atf_get_srcdir)/run-userland.sh"
}
atf_init_test_cases()
{
	atf_add_test_case supervision
	atf_add_test_case libraries
	atf_add_test_case headers
	atf_add_test_case ownership
	atf_add_test_case processes
	atf_add_test_case lifecycle
	atf_add_test_case events
	atf_add_test_case filesystem
	atf_add_test_case logging
	atf_add_test_case credentials
}
