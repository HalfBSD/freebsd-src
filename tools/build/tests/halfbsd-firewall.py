#!/usr/bin/env python3
# SPDX-License-Identifier: BSD-2-Clause
"""Exercise the actual HalfBSD policy scripts with isolated command stubs.

No host firewall, sysctls, or configuration files are changed. Packet behavior
is covered separately by the FreeBSD ATF tests in tests/sys/netpfil/ipfw.
"""

import os
from pathlib import Path
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[3]


class FirewallTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory(prefix="halfbsd-firewall-")
        self.addCleanup(self.tmp.cleanup)
        self.base = Path(self.tmp.name)
        self.log = self.base / "commands"
        self.env = dict(os.environ, COMMAND_LOG=str(self.log), TEST_IPV6="yes")
        command = self.base / "command"
        command.write_text("""#!/bin/sh
kind=${0##*/}
printf '%s' "$kind" >> "$COMMAND_LOG"
printf '\\t%s' "$@" >> "$COMMAND_LOG"
printf '\\n' >> "$COMMAND_LOG"
case "$kind" in
ipfw)
    [ "$*" != "$FAIL_IPFW_MATCH" ] || exit 1
    ;;
sysctl)
    [ "$*" != "$FAIL_SYSCTL_MATCH" ] || exit 1
    if [ "$1" = -n ]; then
        case "$2" in
        net.inet.ip.fw.enable) echo "${TEST_STATUS4:-1}" ;;
        net.inet6.ip6.fw.enable) echo "${TEST_STATUS6:-1}" ;;
        *) exit 1 ;;
        esac
    fi
    ;;
esac
exit 0
""")
        command.chmod(0o755)
        for name in ["ipfw", "sysctl"]:
            (self.base / name).symlink_to(command)
        # Keep the repository's real rc command dispatcher, but replace its
        # host configuration and kernel probes at the command boundary.
        subr = self.base / "rc.subr"
        subr.write_text(f"""kenv() {{ return 1; }}
. '{ROOT}/libexec/rc/rc.subr'
SYSCTL='{self.base}/sysctl'
rc_debug=NO
rc_info=NO
rc_startmsgs=NO
load_rc_config() {{
    firewall_mode=$TEST_MODE
    firewall_enable=NO
    firewall_quiet=NO
    firewall_logging=NO
    firewall_type=/nonexistent/rules
}}
check_jail() {{ return 1; }}
""")
        network = self.base / "network.subr"
        network.write_text('afexists() { [ "$TEST_IPV6" = yes ]; }\n')
        for source, name in [("libexec/rc/rc.firewall", "rc.firewall"),
                             ("libexec/rc/rc.d/ipfw", "rc.ipfw")]:
            text = (ROOT / source).read_text()
            for old, new in [("/etc/rc.subr", str(subr)),
                             ("/etc/network.subr", str(network)),
                             ("/sbin/ipfw", str(self.base / "ipfw")),
                             ("/etc/rc.firewall", str(self.base / "rc.firewall"))]:
                text = text.replace(old, new)
            (self.base / name).write_text(text)

    def run_script(self, name="rc.firewall", args=(), **env):
        return subprocess.run(["/bin/sh", str(self.base / name), *args],
                              env=dict(self.env, **env), capture_output=True,
                              text=True)

    def commands(self):
        if not self.log.exists():
            return []
        return [line.split("\t") for line in self.log.read_text().splitlines()]

    def assert_success(self, result):
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(result.stdout, "")

    def test_default_open_and_obsolete_knobs_ignored(self):
        self.assert_success(self.run_script())
        self.assertEqual(self.commands()[-1],
                         ["ipfw", "-q", "add", "65000", "allow", "ip",
                          "from", "any", "to", "any"])

    def test_explicit_policy_overrides_configuration(self):
        self.assert_success(self.run_script(args=("closed",), TEST_MODE="open"))
        self.assertEqual(self.commands()[-1][4:6], ["deny", "log"])

    def test_invalid_modes_do_not_change_rules_or_sysctls(self):
        for mode in ["UNKNOWN", "client", "simple", "vm-nat", "server",
                     str(self.base / "rc.firewall"), "open; echo bad"]:
            with self.subTest(mode=mode):
                result = self.run_script(args=(mode,))
                self.assertNotEqual(result.returncode, 0)
                self.assertIn("Unknown firewall mode: " + mode, result.stderr)
                self.assertEqual(self.commands(), [])

    def test_workstation_explicit_deny_state_and_dhcp(self):
        self.assert_success(self.run_script(TEST_MODE="workstation"))
        commands = [c for c in self.commands() if c[0] == "ipfw"]
        state = next(i for i, c in enumerate(commands) if "check-state" in c)
        outbound = [i for i, c in enumerate(commands) if "keep-state" in c]
        self.assertTrue(outbound)
        self.assertTrue(all(state < i for i in outbound))
        rules = [" ".join(c) for c in commands]
        self.assertTrue(any("0.0.0.0 68 to 255.255.255.255 67 out" in c
                            for c in rules))
        self.assertTrue(any("any 67 to me 68 in" in c for c in rules))
        self.assertTrue(any("fe80::/10 547 to me 546 in" in c for c in rules))
        self.assertEqual(commands[-1][3:6], ["65000", "deny", "log"])
        self.assertEqual(sum("log" in c for c in commands), 1)

    def test_closed_allows_only_loopback_and_ends_in_deny(self):
        self.assert_success(self.run_script(TEST_MODE="closed"))
        commands = [c for c in self.commands() if c[0] == "ipfw"]
        allows = [c for c in commands if "allow" in c]
        self.assertEqual(len(allows), 1)
        self.assertEqual(allows[0][-2:], ["via", "lo0"])
        self.assertEqual(commands[-1][3:6], ["65000", "deny", "log"])

    def test_ipv4_only_omits_ipv6_rules_and_sysctl(self):
        self.assert_success(self.run_script(TEST_MODE="workstation", TEST_IPV6="no"))
        commands = repr(self.commands())
        for token in ["inet6", "ip6", "ipv6-icmp", "fe80", "ff02", "::1"]:
            self.assertNotIn(token, commands)

    def test_all_modes_quiet_and_enable_logging_and_hooks(self):
        for mode in ["open", "workstation", "closed"]:
            with self.subTest(mode=mode):
                self.log.write_text("")
                self.assert_success(self.run_script(TEST_MODE=mode))
                commands = self.commands()
                for key in ["net.inet.ip.fw.verbose", "net.inet.ip.fw.enable",
                            "net.inet6.ip6.fw.enable"]:
                    self.assertIn(["sysctl", key + "=1"], commands)
                self.assertTrue(all(c[1] == "-q" for c in commands if c[0] == "ipfw"))

    def test_rule_failure_propagates(self):
        result = self.run_script(TEST_MODE="workstation",
                                 FAIL_IPFW_MATCH="-q add 2000 check-state")
        self.assertNotEqual(result.returncode, 0)
        self.assertEqual(self.commands()[-1][-1], "check-state")

    def test_logging_failure_prevents_flush(self):
        result = self.run_script(FAIL_SYSCTL_MATCH="net.inet.ip.fw.verbose=1")
        self.assertNotEqual(result.returncode, 0)
        self.assertFalse(any(c[0] == "ipfw" for c in self.commands()))

    def test_rc_start_reload_restart_unconditional_and_quiet(self):
        for operation in ["start", "reload", "restart"]:
            with self.subTest(operation=operation):
                self.log.write_text("")
                self.assert_success(self.run_script("rc.ipfw", (operation,),
                                                    TEST_MODE="workstation"))
                self.assertEqual(self.commands()[-1][3:6], ["65000", "deny", "log"])

    def test_rc_rejects_stop_without_disabling_hooks(self):
        result = self.run_script("rc.ipfw", ("stop",))
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("IPFW is always enabled", result.stderr)
        self.assertFalse(any("=0" in arg for c in self.commands() for arg in c))

    def test_rc_propagates_invalid_policy(self):
        result = self.run_script("rc.ipfw", ("reload",), TEST_MODE="invalid")
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("Unknown firewall mode: invalid", result.stderr)
        self.assertEqual(self.commands(), [])

    def test_status_requires_both_hooks_enabled(self):
        result = self.run_script("rc.ipfw", ("status",), TEST_MODE="open")
        self.assertEqual(result.returncode, 0, result.stderr)
        result = self.run_script("rc.ipfw", ("status",), TEST_STATUS6="0")
        self.assertNotEqual(result.returncode, 0)


if __name__ == "__main__":
    unittest.main(verbosity=2)
