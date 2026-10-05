# SPDX-License-Identifier: BSD-2-Clause
# Exercise installed HalfBSD policies in isolated VNET jails.

. $(atf_get_srcdir)/../common/utils.subr

setup_policy_network()
{
	grep -q 'policy_workstation()' /etc/rc.firewall ||
	    atf_skip "HalfBSD rc.firewall must be installed"
	firewall_init ipfw
	epair=$(vnet_mkepair)
	vnet_mkjail halfbsd ${epair}a
	vnet_mkjail peer ${epair}b
	jexec halfbsd ifconfig lo0 inet 127.0.0.1/8 up
	jexec peer ifconfig lo0 inet 127.0.0.1/8 up
	jexec halfbsd ifconfig ${epair}a inet 192.0.2.1/24 up
	jexec peer ifconfig ${epair}b inet 192.0.2.2/24 up
	jexec halfbsd ifconfig lo0 inet6 ::1/128
	jexec halfbsd ifconfig ${epair}a inet6 2001:db8::1/64 -ifdisabled
	jexec peer ifconfig ${epair}b inet6 2001:db8::2/64 -ifdisabled
	# Wait for IPv6 duplicate address detection before testing the policy.
	sleep 2
	firewall_tcp_server peer 4 2222 "REPLY"
	firewall_tcp_server peer 6 2222 "REPLY6"
	firewall_tcp_server halfbsd 4 2222 "INBOUND"
}

check_local_and_logging()
{
	atf_check -s exit:0 -o ignore jexec halfbsd ping -c 1 127.0.0.1
	atf_check -s exit:0 -o ignore jexec halfbsd ping -6 -c 1 ::1
	atf_check -o inline:"1\n" jexec halfbsd sysctl -n net.inet.ip.fw.verbose
	atf_check -o inline:"1\n" jexec halfbsd sysctl -n net.inet.ip.fw.enable
	atf_check -o inline:"1\n" jexec halfbsd sysctl -n net.inet6.ip6.fw.enable
	# The policy ending must precede the accepting kernel fallback.
	atf_check -o match:'65535.*allow ip from any to any' \
	    jexec halfbsd ipfw list 65535
}

atf_test_case open cleanup
open_head()
{
	atf_set descr "HalfBSD open policy permits inbound and outbound traffic"
	atf_set require.user root
}
open_body()
{
	setup_policy_network
	atf_check -s exit:0 -o empty jexec halfbsd /bin/sh /etc/rc.firewall open
	check_local_and_logging
	atf_check -o match:'65000.*allow ip from any to any' \
	    jexec halfbsd ipfw list 65000
	atf_check -o inline:"REPLY\n" jexec halfbsd nc -nN -w 2 192.0.2.2 2222
	atf_check -o inline:"INBOUND\n" jexec peer nc -nN -w 2 192.0.2.1 2222
}
open_cleanup()
{
	firewall_cleanup ipfw
}

atf_test_case workstation cleanup
workstation_head()
{
	atf_set descr "HalfBSD workstation permits stateful outbound traffic and blocks unsolicited inbound TCP"
	atf_set require.user root
}
workstation_body()
{
	setup_policy_network
	atf_check -s exit:0 -o empty jexec halfbsd /bin/sh /etc/rc.firewall workstation
	check_local_and_logging
	atf_check -o match:'65000.*deny log.*ip from any to any' \
	    jexec halfbsd ipfw list 65000
	atf_check -s exit:0 -o ignore jexec halfbsd ping -c 1 192.0.2.2
	atf_check -o inline:"REPLY\n" jexec halfbsd nc -nN -w 2 192.0.2.2 2222
	atf_check -o inline:"REPLY6\n" jexec halfbsd nc -6 -nN -w 2 2001:db8::2 2222
	atf_check -s exit:1 -o empty -e ignore jexec peer nc -n -z -w 2 192.0.2.1 2222
	# Invalid names and filenames must preserve the currently active rules.
	jexec halfbsd ipfw list > before
	atf_check -s exit:1 -e match:'Unknown firewall mode: /etc/rc.firewall' \
	    jexec halfbsd /bin/sh /etc/rc.firewall /etc/rc.firewall
	jexec halfbsd ipfw list > after
	atf_check cmp before after
}
workstation_cleanup()
{
	firewall_cleanup ipfw
}

atf_test_case closed cleanup
closed_head()
{
	atf_set descr "HalfBSD closed policy permits loopback and explicitly blocks external traffic"
	atf_set require.user root
}
closed_body()
{
	setup_policy_network
	atf_check -s exit:0 -o empty jexec halfbsd /bin/sh /etc/rc.firewall closed
	check_local_and_logging
	atf_check -o match:'65000.*deny log.*ip from any to any' \
	    jexec halfbsd ipfw list 65000
	atf_check -s exit:1 -o empty -e ignore jexec halfbsd nc -n -z -w 2 192.0.2.2 2222
	atf_check -s exit:1 -o empty -e ignore jexec peer nc -n -z -w 2 192.0.2.1 2222
}
closed_cleanup()
{
	firewall_cleanup ipfw
}

atf_init_test_cases()
{
	atf_add_test_case open
	atf_add_test_case workstation
	atf_add_test_case closed
}
