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
    local e={config={target='multiuser'},records={multiuser={state='READY'}},holds={},blocked={},providers={}}
    for _,name in ipairs({'devices','runtime','loopback','firewall'})do e.records[name]={state='READY'}end
    e.records.network={state='FAILED',wanted=true,definition={parameters={}}}
    function e:shutdown()self.shutting=true end
    return e
end
test('multiuser handoff waits for the complete target and notifies once',function()
    local n,p,w,closed=fixture();local h=handoff.new(n,p,3);local e=engine()
    e.records.multiuser.state='STARTING';h:check(e,true);assert(#w==0)
    e.records.multiuser.state='READY';h:check(e,false);assert(#w==0);e.records.firewall.state='STARTING';h:check(e,true);assert(#w==0)
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
    assert(wave.hostname and wave['swap-1'] and wave['swap-1']<wave.multiuser)
    assert(wave.ldconfig<wave.dns and wave.syslogd<wave.multiuser)
    assert(wave.hostname<wave['dhcp-em0'] and wave.hostname<wave.syslogd)
    assert(wave.graphics<wave.runtime and wave['amdgpu-load']<wave.graphics)
    assert(wave.dns<wave['ssh-check']and wave.dns<wave.ntpd)
    assert(wave['ssh-keys']<wave['ssh-check']and wave['ssh-check']<wave.sshd)
    assert(wave.sshd<wave.multiuser and wave.ntpd<wave.multiuser)
    assert(wave.seatd<wave.multiuser and wave.runtime<wave.seatd)
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
test('required target failure refuses the PID 1 multiuser handoff',function()
    local n,p,w=fixture();local h=handoff.new(n,p,3);local e=engine()
    e.records.multiuser.state='SKIPPED';h:check(e,true)
    assert(e.shutting and w[1]=='FAIL\n')
end)
if arg[2]then
    test('installed default policy contains the complete workstation boot graph',function()
        local default=require('config').read(arg[2])
        assert(default.target=='multiuser'and default.boot.hostname=='halfbsd'and default.max_parallel==1)
        local n={exists=function()return true end,user=function()return {uid=59,gid=59}end,
            fstab=function()return {}end,interfaces=function()return {{name='em0',family='link',link_type=6}}end}
        local r=require('config').registry(require('boot').compile(default.boot,n))
        local plan=require('dependency').plan(r,{multiuser=true})
        for _,name in ipairs({'devices','rootfs','localfs','zfs','temporary','runtime','loopback',
            'firewall','dhcp-em0','network','dns','sshd','ntpd','syslogd','devd','seatd','ldconfig','ldconfig32','hostname'})do
            assert(plan.selected[name],'default omitted '..name)
        end
    end)
end
test('network checkpoint projects policy without activating downstream or custom services',function()
    local config=require('config');local original={boot={mode='system',dns=true,sshd=true,ntpd=true,seatd=true},
        services={{name='custom'}},include={'missing'},service_dirs={'missing'},target='desktop'}
    local c=config.network_checkpoint(original)
    assert(original.boot.dns and original.target=='desktop')
    assert(c.target=='network-checkpoint'and c.max_parallel==1 and c.boot.root_writable and c.boot.amdgpu)
    assert(not c.boot.dns and not c.boot.sshd and not c.boot.ntpd and not c.boot.seatd)
    assert(#c.services==0 and #c.include==0 and #c.service_dirs==0)
    local n={exists=function()return true end,user=function()error('DNS user lookup forbidden')end,
        fstab=function()return {}end,interfaces=function()return {{name='em0',family='link',link_type=6}}end}
    local registry=config.registry(require('boot').compile(c.boot,n))
    local plan=require('dependency').plan(registry,{['network-checkpoint']=true})
    for _,name in ipairs({'graphics','rootfs','runtime','firewall','dhcp-em0','network','syslogd'})do
        assert(plan.selected[name],'checkpoint omitted '..name)
    end
    for _,name in ipairs({'dns','sshd','ntpd','seatd','multiuser','desktop'})do assert(not plan.selected[name])end
end)
test('network handoff notifies once without shutting down supervised children',function()
    local n,p,w,closed=fixture();local h=handoff.new(n,p,3,true);local e=engine()
    e.config.target='network-checkpoint';e.records['network-checkpoint']={state='STARTING'}
    h:check(e,true);assert(#w==0)
    e.records['network-checkpoint'].state='READY';h:check(e,true);h:check(e,true)
    assert(w[1]=='NETWORK\n'and #w==1 and closed()==1 and not e.shutting)
    e.config.checkpoint='network';function e:status()return {}end
    local control=require('control');assert(control.request({version=1,command='status'},0,0,e,{}))
    for _,cmd in ipairs({'start','stop','restart','enable','disable','reload','shutdown'})do
        rejects(function()control.request({version=1,command=cmd,service=cmd~='reload'and cmd~='shutdown'and'network'or nil},0,0,e,{})end)
    end
end)
test('recovery summary retains the first failure and bounded output after cleanup',function()
    local n,p=fixture();local h=handoff.new(n,p,3,true)
    assert(h:summary()=='')
    local record={result={exit_code=1},output_buffer='sysctl: unknown oid\n'..string.rep('x',5000)}
    h:observe({service='ipfw-hook-ip6',state='FAILED',reason='oneshot process failed'},record)
    record.result.exit_code=0;record.output_buffer=''
    h:observe({service='firewall',state='SKIPPED',reason='dependency failed'},{})
    h:observe({service='another',state='FAILED',reason='cleanup failed'},{})
    local line=h:summary();assert(line:match('^%[halfinit boot%] FIRST FAILURE '))
    local result=require('json').decode(line:match('FIRST FAILURE (.+)'))
    assert(result.service=='ipfw-hook-ip6'and result.exit_status==1)
    assert(result.reason=='oneshot process failed'and #result.output==4096)
    assert(result.output:find('\n',1,true)) -- JSON preserves content without splitting the console line
    assert(select(2,line:gsub('\n',''))==1)
end)
print('PASS Phase IV policy groups: '..count..' (no live boot validation)')
