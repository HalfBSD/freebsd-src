/* SPDX-License-Identifier: BSD-2-Clause */
/* The actual action::do_action body is extracted from devd.cc by the runner.
 * Stub ownership/config/exec only: this does not test the FreeBSD kenv ABI. */
#include <cassert>
#include <cstdio>
#include <cstring>
#include <string>
#include <syslog.h>
#include "halfbsd_actions.h"
using std::string;
static const int KENV_GET = 1;
static string owner = "lua", executed, event;
static int logs, child_status;
static bool missing;
static int
kenv(int op, const char *name, char *buffer, int capacity)
{
    assert(op == KENV_GET && !strcmp(name, "halfbsd.boot_active"));
    if (missing) return -1;
    assert(owner.size() + 1 < static_cast<size_t>(capacity));
    strcpy(buffer, owner.c_str());
    return static_cast<int>(owner.size());
}
static void devdlog(int, const char *, ...) { ++logs; }
static int
my_system(const char *command, const char *nomatch = nullptr)
{
    executed = command;
    event = nomatch ? nomatch : "";
    return child_status;
}
class config {
public:
    const string raw = "? at vendor=0x1532 'quoted' ; not-a-shell on uhub0";
    const string &get_variable(const char *key) { assert(!strcmp(key, "*")); return raw; }
    string expand_string(const char *command) { return command; }
};
class action {
    string _cmd;
public:
    explicit action(const char *command) : _cmd(command) {}
    bool do_action(config &);
};
#include "devd-action.inc"
int
main()
{
    config c;
    action match("service devmatch quietstart $*");
    assert(match.do_action(c) && executed == "device module match" && event == c.raw);
    child_status = 1;
    assert(!match.do_action(c));
    child_status = 0;
    for (const char *command : {"service dhclient quietstart $subsystem",
        "service vtcons setkeyboard /dev/ukbd0", "service vtcons setkeyboard /dev/kbd0"}) {
        executed.clear();
        assert(action(command).do_action(c) && executed.empty());
    }
    executed.clear(); logs = 0;
    action power("service power_profile $notify");
    assert(!power.do_action(c) && !power.do_action(c) && logs == 1 && executed.empty());
    owner = "rc";
    assert(match.do_action(c) && executed == "service devmatch quietstart $*" && event.empty());
    owner = "unexpected"; executed.clear();
    assert(!match.do_action(c) && executed.empty());
    missing = true;
    assert(!match.do_action(c) && executed.empty());
    missing = false; owner = "lua";
    assert(action("service custom restart").do_action(c) && executed == "service custom restart");
    puts("PASS actual devd action body: ownership, raw argv, failures, no duplicate DHCP/keyboard RC dispatch, bounded unsupported warnings and explicit RC recovery");
}
