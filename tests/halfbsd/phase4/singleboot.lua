-- SPDX-License-Identifier: BSD-2-Clause
package.path=assert(arg[1])..'/?.lua'
local single=require('singleboot')
local function fixture()
    local f={commands={},lines={},messages={},closed=0,code=0,card=true}
    local n={identity=function()return {euid=0,ppid=1}end,
        mounts=function()return {{path='/',type='zfs',source='tank/root',readonly=true},{path='/dev',type='devfs'}}end,
        stat=function()return {type='device'}end,exists=function()return true end,
        listdir=function()return f.card and {'renderD128','card0'}or{}end}
    local p={fd=function(fd)assert(fd==3);return {write=function(_,text)f.messages[#f.messages+1]=text;return #text end,
        close=function()f.closed=f.closed+1 end}end}
    function p.run(argv,opts)
        f.commands[#f.commands+1]=argv
        assert(opts.new_session and opts.process_group and opts.stdin=='null')
        assert(opts.stdout.path=='/dev/console'and opts.stderr.path=='/dev/console')
        assert(opts.timeout>0 and opts.timeout<=120 and opts.exec_timeout==5)
        if f.exec_error then return nil,{operation='spawn',name='ENOENT',message='missing executable'}end
        return {exit_code=f.code}
    end
    local function emit(line)f.lines[#f.lines+1]=line end
    return f,n,p,emit
end
local f,n,p,emit=fixture()
assert(single.run(n,p,3,emit)==0)
assert(#f.commands==3 and f.commands[1][1]=='/sbin/zfs'and f.commands[2][1]=='/sbin/kldload')
assert(f.commands[2][4]=='amdgpu'and f.commands[3][1]=='/sbin/kldstat')
assert(f.messages[1]=='SINGLE\n'and #f.messages==1 and f.closed==1)
assert(f.lines[1]:find('networking and multiuser services are disabled',1,true))
print('PASS single-stage ZFS/AMDGPU command order, live console output, bounded execution and shell handoff')
for _,failure in ipairs({'exit','exec','card','root','devfs'})do
    f,n,p,emit=fixture()
    if failure=='exit'then f.code=1 end
    if failure=='exec'then f.exec_error=true end
    if failure=='card'then f.card=false end
    if failure=='root'then n.mounts=function()return {{path='/',type='ufs'}}end end
    if failure=='devfs'then n.mounts=function()return {{path='/',type='zfs'}}end end
    assert(single.run(n,p,3,emit)==1)
    assert(f.messages[1]=='FAIL\n'and #f.messages==1 and f.closed==1)
    assert(f.lines[#f.lines]:find('FAILED',1,true))
end
print('PASS failed ZFS/AMDGPU/exec checks close notification and report recovery, never multiuser READY')
f,n,p,emit=fixture();n.identity=function()return {euid=1001,ppid=1}end
assert(not pcall(single.run,n,p,3,emit)and #f.commands==0)
f,n,p,emit=fixture();n.identity=function()return {euid=0,ppid=22}end
assert(not pcall(single.run,n,p,3,emit)and #f.commands==0)
print('PASS diagnostic activation rejects non-root and non-PID-1 callers before mutations')
-- The internal entry must not read/compile the full boot configuration at all.
local config=require('config');local original_load=config.load;local original_run=single.run
local original_os=package.loaded['halfbsd.os'];local original_proc=package.loaded.proc
local entry=require('init');local entered=false
config.load=function()error('diagnostic entry attempted to load the multiuser configuration')end
package.loaded['halfbsd.os']={};package.loaded.proc={}
single.run=function(native,proc,fd)
    assert(native==package.loaded['halfbsd.os']and proc==package.loaded.proc and fd==3)
    entered=true;return 0
end
local ok,err=pcall(function()
    assert(entry.main({'--boot-single','--boot-notify-fd','3'},'/absent')==0 and entered)
    assert(not pcall(entry.main,{'--boot-single','--boot-notify-fd','3','--config','/absent/system.lua'},'/absent'))
end)
config.load=original_load;single.run=original_run
package.loaded['halfbsd.os']=original_os;package.loaded.proc=original_proc
assert(ok,err)
print('PASS diagnostic launcher entry bypasses multiuser configuration and rejects mixed activation')
