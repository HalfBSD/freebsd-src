-- SPDX-License-Identifier: BSD-2-Clause
package.path=assert(arg[1])..'/?.lua'
local match=require('devmatch')
local event='? at vendor=0x1532 product=0x0041 on uhub0'
local function fixture(options)
    local o=options or{};local calls={};local now=0
    local d={native={identity=function()return {euid=o.uid or 0}end,
        clock=function()now=now+(o.step or 0);return now end},proc={}}
    function d.proc.run(argv,settings)
        assert(argv[1]:sub(1,1)=='/'and settings.stdout=='pipe'and settings.stderr=='pipe')
        assert(settings.timeout>0 and settings.timeout<=5 and settings.output_limit==32768)
        calls[#calls+1]=argv
        if o.fail and argv[1]==o.fail then return nil,{message='injected resource failure'}end
        if argv[1]=='/bin/kenv'then
            assert(#argv==2 and argv[2]=='-q')
            local values={['halfbsd.boot_active']=o.owner or'lua',boot_safe=o.safe,devmatch_blocklist=o.blocked}
            local lines={};for key,value in pairs(values)do lines[#lines+1]=key..'="'..value..'"'end
            return {success=not o.env_failure,exit_code=o.env_failure and 255 or 0,
                stdout=o.environment or table.concat(lines,'\n')}
        elseif argv[1]=='/sbin/devmatch'then
            assert(argv[2]=='-p'and argv[3]==event and #argv==3)
            return {success=not o.match_failure,stdout=o.output or'hms.ko\nhidmap.ko\nhms.ko\n',stderr='match failure'}
        elseif argv[1]=='/sbin/kldstat'then return {success=o.loaded or false,exit_code=o.loaded and 0 or 1}
        elseif argv[1]=='/sbin/kldload'then
            assert(argv[2]=='-n'and #argv==3)
            return {success=not o.load_failure,stderr='load failure'}
        end
        error('unexpected command')
    end
    return d,calls
end
local function loads(calls)local out={};for _,v in ipairs(calls)do if v[1]=='/sbin/kldload'then out[#out+1]=v[3]end end;return table.concat(out,',')end
local function run(o)local d,c=fixture(o);local ok,e=pcall(match.main,{'--device-nomatch',event},d);return ok,c,e end
local ok,c=run();assert(ok and loads(c)=='hms,hidmap')
ok,c=run({blocked='hms.ko hidmap'});assert(ok and loads(c)=='')
ok,c=run({loaded=true});assert(ok and loads(c)=='')
for _,safe in ipairs({'YES','1','true','on'})do ok,c=run({safe=safe});assert(ok and #c==1)end
for _,o in ipairs({{uid=1001},{owner='rc'},{owner='unknown'},{fail='/bin/kenv'},
    {fail='/sbin/devmatch'},{env_failure=true},{environment='boot_safe=not-quoted'},{match_failure=true},{output='hms.ko /tmp/evil.ko'},
    {output='hms.ko bad;name.ko'},{output='hms.ko ../bad.ko'},
    {output=string.rep('a',129)..'.ko'},{step=20}})do
    ok,c=run(o);assert(not ok and loads(c)=='')
end
ok,c=run({load_failure=true});assert(not ok and loads(c)=='hms')
local names={};for i=1,33 do names[i]='driver'..i..'.ko'end
ok,c=run({output=table.concat(names,'\n')});assert(not ok and loads(c)=='')
for _,invalid in ipairs({'x','? at x on y\nother','? at x on y\0bad',string.rep('x',8193)})do
    local d,calls=fixture();assert(not pcall(match.main,{'--device-nomatch',invalid},d)and #calls==0)
end
print('PASS bounded direct device matching, safe boot, blocklists, loaded modules, invalid output, ownership and failure paths')
