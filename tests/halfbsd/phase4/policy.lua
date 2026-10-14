-- SPDX-License-Identifier: BSD-2-Clause
package.path=assert(arg[1])..'/?.lua'
local handoff=require('handoff');local count=0
local function test(name,fn)fn();count=count+1;print('PASS '..name)end
local function rejects(fn)assert(not pcall(fn),'unsafe input accepted')end
local function fixture()
    local writes={};local closed=0
    local n={identity=function()return {euid=0,ppid=1}end,mounts=function()return {{path='/',type='zfs'}}end}
    local p={fd=function(number)assert(number==3);return {write=function(_,s)writes[#writes+1]=s;return #s end,close=function()closed=closed+1 end}end}
    return n,p,writes,function()return closed end
end
test('handoff accepts only root directly owned by PID 1 and supported root',function()
    local n,p=fixture();assert(handoff.new(n,p,3))
    rejects(function()handoff.new(n,p,4)end)
    n.identity=function()return {euid=0,ppid=22}end;rejects(function()handoff.new(n,p,3)end)
    n.identity=function()return {euid=1001,ppid=1}end;rejects(function()handoff.new(n,p,3)end)
    n.identity=function()return {euid=0,ppid=1}end
    n.mounts=function()return {{path='/',type='ufs'}}end;rejects(function()handoff.new(n,p,3)end)
end)
local function engine()
    local e={records={},holds={},blocked={},providers={}}
    for _,name in ipairs({'devices','runtime','loopback','firewall'})do e.records[name]={state='READY'}end
    e.records.network={state='FAILED',wanted=true,definition={parameters={}}}
    function e:shutdown()self.shutting=true end
    return e
end
test('local recovery login readiness does not require DHCP; notification once',function()
    local n,p,w,closed=fixture();local h=handoff.new(n,p,3);local e=engine()
    h:check(e,false);assert(#w==0);e.records.firewall.state='STARTING';h:check(e,true);assert(#w==0)
    e.records.firewall.state='READY';h:check(e,true);h:check(e,true);h:fail()
    assert(#w==1 and w[1]=='READY\n'and closed()==1)
end)
test('critical startup failure closes handoff and initiates ordered shutdown',function()
    local n,p,w,closed=fixture();local h=handoff.new(n,p,3);local e=engine()
    e.records.devices.state='FAILED';h:check(e,true);h:fail()
    assert(e.shutting and #w==1 and w[1]=='FAIL\n'and closed()==1)
end)
test('route loss, recovery, operator holds and shutdown preserve intent',function()
    local e=engine();local available=false;local failures=0;local starts=0
    e.providers.route_ready=function()return available end
    function e:failure(r)failures=failures+1;r.state='FAILED'end
    function e:start(names)assert(names[1]=='network');starts=starts+1;self.records.network.state='READY'end
    e.records.network.state='READY';handoff.network(e);assert(failures==1)
    available=true;handoff.network(e);assert(starts==1)
    e.holds.network=true;available=false;handoff.network(e);assert(failures==1)
    e.holds.network=nil;e.shutting=true;handoff.network(e);assert(failures==1)
end)
test('boot graph prepares devices and loopback before exposing services',function()
    local native={exists=function()return true end,user=function()return {uid=59,gid=59}end,interfaces=function()return {{name='em0',family='link',link_type=6}}end,fstab=function()return {}end}
    local registry=require('config').registry(require('boot').compile({hostname='fixture',swap={'/dev/fixture'}},native))
    local plan=require('dependency').plan(registry,{multiuser=true});local wave={}
    for i,names in ipairs(plan.waves)do for _,name in ipairs(names)do wave[name]=i end end
    assert(wave.devices<wave.rootfs and wave.temporary<wave.runtime and wave.loopback<wave.firewall)
    assert(wave.firewall<wave.dns and wave.firewall<wave['dhcp-em0'])
    assert(wave.hostname and wave['swap-1'])
    rejects(function()require('boot').compile({swap={'/tmp/swapfile'}},native)end)
end)
test('swap is released once only when activated by this manager',function()
    local enabled=0;local disabled=0;local busy=false
    local native={identity=function()return {euid=0}end,stat=function()return {type='device'}end,
        swap_on=function()if busy then return nil,{name='EBUSY'}end;enabled=enabled+1;return true end,
        swap_off=function()disabled=disabled+1;return true end}
    local p=require('adapters').attach({},native,{})
    local d={};assert(p.swap_up({path='/dev/test'},d));assert(p.swap_down({},d));assert(p.swap_down({},d))
    assert(enabled==1 and disabled==1)
    busy=true;assert(p.swap_up({path='/dev/test'},{}));assert(p.swap_down({},{}));assert(disabled==1)
    native.stat=function()return {type='file'}end;rejects(function()p.swap_up({path='/dev/test'},{})end)
    local inode=7;busy=false;native.stat=function()return {type='device',inode=inode,device=1}end
    local owned={};assert(p.swap_up({path='/dev/test'},owned));inode=8
    rejects(function()p.swap_down({},owned)end);assert(disabled==1)
    inode=7;assert(p.swap_down({},owned));assert(disabled==2)
end)
test('essential devfs and explicit permission policy fail safely',function()
    local changed=0
    local native={identity=function()return {euid=0}end,mounts=function()return {{path='/dev',type='devfs'}}end,
        stat=function()return {type='device'}end,chmod=function()changed=changed+1;return true end,
        chown=function()changed=changed+1;return true end}
    local p=require('adapters').attach({},native,{})
    assert(p.boot_devices({rules={{path='/dev/fixture',mode=384,uid=0,gid=0}}}));assert(changed==2)
    native.mounts=function()return {}end;rejects(function()p.boot_devices({})end)
end)
test('shutdown retains read-only status and rejects activation/persistence/reload',function()
    local control=require('control');local e={shutting=true,records={fixture={}},plan={}}
    function e:status()return {{name='fixture',state='STOPPING'}}end
    function e:shutdown()end
    assert(control.request({version=1,command='status'},0,0,e,{}))
    assert(control.request({version=1,command='shutdown'},0,0,e,{}))
    for _,cmd in ipairs({'start','stop','restart','enable','disable','reload'})do
        rejects(function()control.request({version=1,command=cmd,service=cmd~='reload'and'fixture'or nil},0,0,e,{})end)
    end
end)
test('boot configuration requires trusted path ownership and rejects links',function()
    local path=os.tmpname();local file=assert(io.open(path,'w'));file:write('return {target="minimal"}');file:close()
    local previous=package.loaded['halfbsd.os'];local unsafe=false;local link=false
    package.loaded['halfbsd.os']={stat=function(prefix)
        return {uid=unsafe and 1001 or 0,mode=493,type=prefix==path and(link and'symlink'or'file')or'directory'}
    end}
    local ok,err=pcall(function()
        assert(require('config').read(path,0).target=='minimal')
        unsafe=true;rejects(function()require('config').read(path,0)end)
        unsafe=false;link=true;rejects(function()require('config').read(path,0)end)
    end)
    package.loaded['halfbsd.os']=previous;assert(os.remove(path));assert(ok,err)
end)
print('PASS Phase IV policy groups: '..count..' (no live boot validation)')
