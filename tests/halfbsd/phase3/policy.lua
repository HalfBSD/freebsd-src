-- SPDX-License-Identifier: BSD-2-Clause
package.path=assert(arg[1])..'/?.lua'
local config=require('config');local boot=require('boot');local dependency=require('dependency')
local count=0
local function test(name,fn)fn();count=count+1;print('PASS '..name)end
local function rejects(fn)assert(not pcall(fn),'unsafe policy accepted')end
local function fixture()
    local f={uid=0,rows={{path='/',source='tank/root',type='zfs',readonly=false}},processes={},clock=0,mutations=0}
    local native={}
    function native.identity()return {euid=f.uid,pid=1}end
    function native.processes()return f.processes end
    function native.mounts()return f.rows end
    function native.mount(p)
        f.mutations=f.mutations+1;f.rows[#f.rows+1]={path=p.fspath,source=p.from,type=p.fstype,readonly=false};return true
    end
    function native.unmount(path)
        f.mutations=f.mutations+1;for i,r in ipairs(f.rows)do if r.path==path then table.remove(f.rows,i);break end end;return true
    end
    function native.fstab()return {}end
    function native.interfaces()return {{name='em0',family='link',link_type=6,link_state=2},{name='em0',family='inet',up=true,address='192.0.2.2'}}end
    function native.default_routes()return {{interface='em0',gateway='192.0.2.1'}}end
    function native.user()return {uid=59,gid=59}end
    function native.exists()return true end
    function native.clock()return f.clock end
    function native.getenv()return '/tmp/session'end
    function native.terminal()return {tty=true,foreground=true}end
    function native.stat()return {type='directory',uid=f.uid,mode=448}end
    local ipc={tcp=function()return {close=function()end}end,connected=function()return true end}
    local providers=require('adapters').attach({},native,ipc)
    return f,native,providers,ipc
end
local function registry(policy,native)return config.registry(boot.compile(policy,native))end
local function position(plan,name)
    for i,wave in ipairs(plan.waves)do for _,n in ipairs(wave)do if n==name then return i end end end
    error('service absent: '..name)
end
test('system graph is declarative and preserves compiled DNS configuration',function()
    local f,n=fixture();local r=registry({},n);local p=dependency.plan(r,{multiuser=true})
    assert(f.mutations==0 and r.dns.command[1]=='/usr/sbin/local-unbound')
    assert(r.dns.command[4]=='/etc/unbound/unbound.conf')
    assert(position(p,'firewall')<position(p,'dhcp-em0')and position(p,'dns-anchor')<position(p,'dns'))
    assert(not r.gateway.enabled and not r.desktop)
end)
test('workstation requires foreground devd before seatd and excludes duplicate ownership',function()
    local _,n=fixture();local r=registry({},n);local plan=dependency.plan(r,{multiuser=true})
    assert(plan.selected.devd and r.devd.command[1]=='/sbin/devd'and r.devd.command[2]=='-d')
    assert(r.devd.guard=='exclusive_guard'and r.devd.restart=='on-failure')
    assert(r.devd.condition=='socket_ready'and r.devd.parameters.path=='/var/run/devd.pipe')
    assert(r.devd.parameters.processes[1]=='devd')
    local guarded=false
    for _,p in ipairs(r['boot-preflight'].parameters.processes)do if p=='devd'then guarded=true end end
    assert(guarded)
    assert(position(plan,'firewall')<position(plan,'devd'))
    assert(position(plan,'devd')<position(plan,'seatd'))
    assert(position(plan,'seatd')<position(plan,'multiuser'))
    local minimal=dependency.plan(r,{minimal=true});assert(not minimal.selected.devd)
    local checkpoint=dependency.plan(r,{['network-checkpoint']=true});assert(not checkpoint.selected.devd)
end)
test('workstation always requires base seatd; legacy generated setting migrates without editing config',function()
    local f,n=fixture();local r=registry({},n);local plan=dependency.plan(r,{multiuser=true})
    assert(plan.selected.seatd and r.seatd.command[1]=='/usr/sbin/seatd')
    assert(r.seatd.condition=='seat_ready'and r.seatd.command[5]=='video')
    assert(r.seatd.env.SEATD_VTBOUND=='1')
    assert(position(plan,'seatd')<position(plan,'multiuser'))
    rejects(function()registry({seatd=false},n)end)
    local path=os.tmpname();local text='return {boot={seatd=false,interfaces={"em0"}},target="multiuser"}'
    local file=assert(io.open(path,'w'));file:write(text);file:close()
    local previous=package.loaded['halfbsd.os'];package.loaded['halfbsd.os']=n
    local ok,problem=pcall(function()
        local c=config.load(path);assert(c.boot.seatd==nil and c.registry.seatd)
        assert(dependency.plan(c.registry,{multiuser=true}).selected.seatd)
        local read=assert(io.open(path));assert(read:read('*a')==text);read:close()
    end)
    package.loaded['halfbsd.os']=previous;assert(os.remove(path));assert(ok,problem)
end)
test('seatd readiness enforces root/video socket permissions before connection',function()
    local f,n,p,ipc=fixture();local st={type='socket',uid=0,gid=44,mode=0x1f8};local probes=0
    n.group=function(name)assert(name=='video');return {gid=44}end
    n.stat=function()return st end
    ipc.probe=function()probes=probes+1;return true end
    assert(p.seat_ready({path='/var/run/seatd.sock'}));assert(probes==1)
    for _,change in ipairs({{uid=1001},{gid=0},{mode=0x1ff},{type='file'}})do
        local old={};for key,value in pairs(change)do old[key]=st[key];st[key]=value end
        assert(not p.seat_ready({path='/var/run/seatd.sock'}))
        for key,value in pairs(old)do st[key]=value end
    end
    assert(probes==1)
    assert(p.seat_ready({path='/var/run/seatd.sock'}));assert(probes==2)
    n.stat=function()return nil,{name='ENOENT'}end;assert(not p.seat_ready({path='/var/run/seatd.sock'}))
end)
test('root remount explicitly overrides read-only options and verifies kernel readback',function()
    local f,n,p=fixture();f.rows[1].readonly=true
    local calls=0
    n.mount=function(options,flags)
        calls=calls+1
        assert(flags==0x10000 and options.fstype=='zfs'and options.fspath=='/'and options.from=='tank/root')
        -- nmount merges prior options: MNT_UPDATE alone retains the old ro option.
        if options.rw==''then f.rows[1].readonly=false end
        return true
    end
    rejects(function()p.root_writable({update=false})end);assert(calls==0)
    assert(p.root_writable({update=true}));assert(calls==1 and not f.rows[1].readonly)
    assert(p.root_writable({update=true}));assert(calls==1) -- writable roots need no syscall
    f.rows[1].readonly=true
    n.mount=function()return true end
    rejects(function()p.root_writable({update=true})end) -- success without changed state is insufficient
    n.mount=function()return nil,{name='EROFS',code=30}end
    local ok,err=pcall(p.root_writable,{update=true});assert(not ok and err.name=='EROFS')
    f.uid=1001;calls=0;n.mount=function()calls=calls+1;return true end
    rejects(function()p.root_writable({update=true})end);assert(calls==0)
end)
test('nested filesystems order parents before checks and child mounts',function()
    local f,n=fixture();local r=registry({interfaces={'em0'},zfs=false,local_mounts={
        {options={fstype='ufs',fspath='/var/log',from='/dev/log'},fsck=true},
        {options={fstype='ufs',fspath='/var',from='/dev/var'}}}},n)
    local p=dependency.plan(r,{minimal=true})
    assert(position(p,'mount-2')<position(p,'fsck-1')and position(p,'fsck-1')<position(p,'mount-1'))
    assert(position(p,'mount-1')<position(p,'runtime')and not p.selected.firewall)
end)
test('invalid manifest, interfaces, paths and mount duplication reject',function()
    local f,n=fixture()
    for _,policy in ipairs({{unknown=true},{interfaces={}},{interfaces={'em0','em0'}},
        {interfaces={'thisnameistoolong'}},{firewall={ownership='flush'}},
        {local_mounts={{options={fstype='ufs',fspath='/var/../etc',from='/dev/test'}}}},
        {local_mounts={{options={fstype='ufs',fspath='/var',from='/a'}},{options={fstype='ufs',fspath='/var',from='/b'}}}}})do
        rejects(function()registry(policy,n)end)
    end
end)
test('fstab skips kernel root, swap, ZFS and noauto; FAT fsck supported',function()
    local f,n=fixture();n.fstab=function()return {
        {path='/',source='root',type='ufs',options='rw',pass=1},
        {path='none',source='/dev/swap',type='swap',options='sw',pass=0},
        {path='/skip',source='x',type='ufs',options='rw,noauto',pass=0},
        {path='/boot/efi',source='/dev/efi',type='msdosfs',options='rw',pass=2}}end
    local r=registry({},n);assert(r['fsck-1'].command[1]=='/sbin/fsck'and not r['mount-2'])
end)
test('already mounted filesystems are neither checked nor adopted',function()
    local f,n,p=fixture();local d={};local m={options={fstype='zfs',fspath='/',from='tank/root'}}
    assert(p.fsck_guard(m)=='ready');assert(p.local_mount(m,d));assert(p.unmount_owned(m,d))
    assert(f.mutations==0);rejects(function()p.local_mount({options={fstype='ufs',fspath='/',from='other'}},{})end)
end)
test('only created mounts are released; changed ownership refuses cleanup',function()
    local f,n,p=fixture();local d={};local m={options={fstype='tmpfs',fspath='/test',from='tmpfs'}}
    assert(p.local_mount(m,d));assert(p.unmount_owned(m,d));assert(p.unmount_owned(m,d));assert(f.mutations==2)
    assert(p.local_mount(m,d));f.rows[2].source='foreign';rejects(function()p.unmount_owned(m,d)end)
end)
test('root privilege, existing daemons and second managers gate activation',function()
    local f,n,p=fixture();local e={proc={active=function()return {{pid=22}}end}}
    f.processes={{pid=1,uid=0,jail=0,command='halfinit'},{pid=22,uid=0,jail=0,command='dhclient'}}
    assert(p.exclusive_guard({processes={'halfinit','dhclient'}},{},e))
    f.processes[#f.processes+1]={pid=23,uid=0,jail=0,command='dhclient'}
    rejects(function()p.exclusive_guard({processes={'dhclient'}},{},e)end)
    f.uid=1001;rejects(function()p.root_guard()end)
end)
test('firewall readback requires immutable default and guard; loading never flushes',function()
    local f,n,p=fixture();local baseline='65535 set 31 deny ip from any to any\n'
    assert(p.firewall_baseline({},nil,baseline))
    rejects(function()p.firewall_baseline({},nil,baseline..'00100 set 0 allow ip from any to any\n')end)
    local guard='65534 set 31 deny ip from any to any\n'
    assert(p.firewall_policy({load=false},nil,guard..baseline))
    rejects(function()p.firewall_policy({},nil,guard:gsub('31','0')..baseline)end)
    local r=registry({firewall={ownership='load'}},n)
    assert(r.firewall.parameters.rules['2000']=='check-state :default')
    assert(r.firewall.parameters.rules['2500']:match('keep%-state :default$'))
    assert(r['ipfw-rule-65534'].command[5]=='set'and r['ipfw-rule-65534'].command[6]=='31')
    for _,d in pairs(r)do if d.command then for _,a in ipairs(d.command)do assert(a~='flush')end end end
    local policy={load=true,rules={['100']='allow ip from any to any via lo0'}}
    assert(p.firewall_policy(policy,nil,guard..baseline..'00100 set 0 allow ip from any to any via lo0\n'))
    rejects(function()p.firewall_policy(policy,nil,guard..baseline..'00100 set 0 allow ip from any to any\n')end)
end)
test('IPFW kernel orphan counter is accepted without weakening deny-policy verification',function()
    local _,_,p=fixture()
    local counter='65535 set 31 count ip from any to any not // orphaned dynamic states counter\n'
    local deny='65535 set 31 deny ip from any to any\n'
    local guard='65534 set 31 deny ip from any to any\n'
    assert(p.firewall_baseline({},nil,counter..deny))
    assert(p.firewall_baseline({},nil,deny..counter))
    assert(p.firewall_policy({load=true,rules={}},nil,guard..counter..deny))
    assert(p.firewall_policy({load=false},nil,guard..counter..deny))
    for _,bad in ipairs({counter,counter..counter..deny,counter..deny..deny,
        counter..deny:gsub('deny','allow'),counter:gsub(' not //',' //')..deny,
        counter:gsub('set 31','set 0')..deny,counter:gsub('65535','65534')..deny,
        counter:gsub('count ip','allow ip')..deny,counter:gsub('counter','other')..deny,
        counter..deny..'00100 set 0 allow ip from any to any\n'})do
        rejects(function()p.firewall_baseline({},nil,bad)end)
        rejects(function()p.firewall_policy({load=true,rules={}},nil,guard..bad)end)
    end
    rejects(function()p.firewall_policy({load=false},nil,counter..deny)end)
end)
test('route readiness requires both address and default route on selected interface',function()
    local f,n,p=fixture();assert(p.route_ready({interface='em0'}));assert(not p.route_ready({interface='em1'}))
    n.default_routes=function()return {}end;assert(not p.route_ready({interface='em0'}))
end)
test('DNS readiness requires matching successful UDP and TCP; probes close',function()
    local f,n,p,ipc=fixture();local closed=0;local query;local answer;local d={}
    ipc.udp=function()return {write=function(_,text)query=text;return #text end,
        read=function()return answer,{name='EAGAIN'}end,close=function()closed=closed+1 end}end
    assert(not p.dns_ready({},d));local id=string.unpack('>I2',query)
    answer=string.pack('>I2I2I2I2I2I2',id+1,0x8000,1,0,0,0);assert(not p.dns_ready({},d))
    answer=string.pack('>I2I2I2I2I2I2',id,0x8002,1,0,0,0);assert(not p.dns_ready({},d))
    answer=string.pack('>I2I2I2I2I2I2',id,0x8000,1,1,0,0)..query:sub(13);assert(p.dns_ready({},d)and closed==1)
    assert(not p.dns_ready({},d));p.release(d);assert(closed==2)
end)
test('TCP readiness retains pending sockets and releases refusal, timeout and cancellation',function()
    local f,n,p,ipc=fixture();local made,closed,calls=0,0,0;local first;local d={}
    ipc.tcp=function()made=made+1;local fd={close=function()closed=closed+1 end};first=first or fd;return fd end
    ipc.connected=function(fd)assert(fd==first);calls=calls+1;return calls>=3 end
    assert(not p.tcp_ready({port=22},d));assert(not p.tcp_ready({port=22},d))
    assert(p.tcp_ready({port=22},d)and made==1 and closed==1)
    ipc.connected=function()return nil,{name='ECONNREFUSED'}end
    assert(not p.tcp_ready({port=22},d)and made==2 and closed==2)
    ipc.connected=function()return false end
    assert(not p.tcp_ready({port=22},d));f.clock=1
    assert(not p.tcp_ready({port=22},d)and made==4 and closed==3)
    p.release(d);p.release(d);assert(closed==4)
end)
test('DNS retains the UDP answer while its single TCP handshake completes',function()
    local f,n,p,ipc=fixture();local query;local reads,polls,made,closed=0,0,0,0;local first;local d={}
    ipc.udp=function()return {write=function(_,text)query=text;return #text end,
        read=function()
            reads=reads+1;assert(reads==1,'consumed UDP response was requested again')
            local id=string.unpack('>I2',query)
            return string.pack('>I2I2I2I2I2I2',id,0x8000,1,1,0,0)..query:sub(13)
        end,close=function()closed=closed+1 end}end
    ipc.tcp=function()made=made+1;first={close=function()closed=closed+1 end};return first end
    ipc.connected=function(fd)assert(fd==first);polls=polls+1;return polls>=3 end
    assert(not p.dns_ready({},d));assert(not p.dns_ready({},d));assert(not p.dns_ready({},d))
    assert(p.dns_ready({},d)and reads==1 and made==1 and closed==2)
end)
test('desktop requires non-root, private runtime and foreground login terminal',function()
    local f,n,p=fixture();local r=registry({mode='user',desktop={command={'/usr/local/bin/sway'}}},n)
    assert(r.compositor.foreground and r.compositor.new_session==false and r.compositor.stdin=='inherit')
    rejects(function()p.user_session_guard({runtime='/tmp/session'})end)
    f.uid=1001;assert(p.user_session_guard({runtime='/tmp/session'}))
    n.terminal=function()return {tty=true,foreground=true,stop_background_output=true}end
    rejects(function()p.user_session_guard({runtime='/tmp/session'})end)
    rejects(function()registry({mode='user',desktop={command={'/usr/local/bin/niri'}}},n)end)
    n.terminal=function()return {tty=false,foreground=false}end
    rejects(function()p.user_session_guard({runtime='/tmp/session'})end)
end)
local function synchronous(defs,providers)
    local loop={serial=0}
    function loop:watch_timer()self.serial=self.serial+1;return self.serial end
    function loop:cancel()return true end
    function loop:stop()self.stopped=true end
    local cfg={registry=config.registry(defs),max_parallel=128,shutdown_timeout=1}
    local engine=require('supervisor').new(cfg,{loop=loop,proc={active=function()return {}end},native={clock=function()return 0 end},providers=providers,system_allowed=true})
    return engine
end
test('cleanup runs once in reverse dependency order, including failed actions',function()
    local order={};local providers={ok=function()return true end,fail=function()error('post-mount error')end,
        clean=function(_,d)order[#order+1]=d.name;return true end}
    local e=synchronous({{name='parent',type='oneshot',action='ok',cleanup='clean'},
        {name='child',type='oneshot',action='ok',cleanup='clean',requires={'parent'}}},providers)
    e:start({'child'});assert(e.records.child.state=='READY');e:shutdown();e:cleanup_actions()
    assert(table.concat(order,',')=='child,parent')
    local f=synchronous({{name='failed',type='oneshot',action='fail',cleanup='clean'}},providers)
    f:start({'failed'});f:shutdown();f:cleanup_actions();assert(order[3]=='failed'and #order==3)
end)
test('oneshot verification waits for EOF, accepts declared exit codes, preserves result',function()
    local e=synchronous({{name='check',type='oneshot',command={'/fixture'},verify='verify',success_codes={0,1}}},
        {verify=function(_,d,text)return text=='verified'end})
    local r=e.records.check;r.launch_definition=r.definition;r.state='STARTING';r.result={state='exited',exit_code=1,success=false}
    r.output_buffer='verified';e:complete_oneshot(r);assert(r.state=='STARTING')
    r.output_done=true;e:complete_oneshot(r);assert(r.state=='READY'and not r.result.success)
end)
test('manager prepares storage before runtime lock/control and cleans owned mounts',function()
    local f,native,p,ipc=fixture();local operations={};local files={}
    local saved={};local names={'halfbsd.os','halfbsd.ipc','halfbsd.log','halfbsd.event','proc','control'}
    for _,name in ipairs(names)do saved[name]=package.loaded[name]end
    native.stat=function(path)
        if path=='/'or files[path]then return {type='directory',uid=0,mode=493}end
        return nil,{name='ENOENT'}
    end
    native.mkdir=function(path)assert(#f.rows==2,'runtime files opened before mount');files[path]=true;return true end
    native.chmod=function()return true end
    native.exists=function()return false end
    ipc.lock=function()assert(#f.rows==2);operations[#operations+1]='lock';return {close=function()operations[#operations+1]='unlock'end}end
    local unmount=native.unmount
    native.unmount=function(path)assert(operations[#operations]=='unlock','owned mount released while runtime lock open');return unmount(path)end
    local loop={timers={},serial=0}
    function loop:watch_timer(delay,callback)self.serial=self.serial+1;self.timers[self.serial]=callback;return self.serial end
    function loop:watch_signal()self.serial=self.serial+1;return self.serial end
    function loop:cancel(id)self.timers[id]=nil;return true end
    function loop:stop()self.stopped=true end
    function loop:close()end
    function loop:run()
        self.stopped=false
        for _,callback in pairs(self.timers)do callback();if self.stopped then return true end end
        error('bootstrap did not make progress')
    end
    package.loaded['halfbsd.os']=native;package.loaded['halfbsd.ipc']=ipc
    package.loaded['halfbsd.log']={new=function()return {write=function()return true end,close=function()end}end}
    package.loaded['halfbsd.event']={new=function()return loop end}
    package.loaded.proc={active=function()return {}end,shutdown=function()return true end}
    package.loaded.control={new=function()assert(#f.rows==2);operations[#operations+1]='control';return {close=function()end}end}
    local cfg={boot={mode='system'},target='minimal',runtime_dir='/var/run/halfbsd',max_parallel=128,shutdown_timeout=1,
        registry=config.registry({
            {name='mount',type='oneshot',action='local_mount',cleanup='unmount_owned',system=true,
                parameters={options={fstype='tmpfs',fspath='/var',from='tmpfs'}}},
            {name='runtime',type='oneshot',action='directories',requires={'mount'},system=true,
                parameters={directories={{path='/var/run/halfbsd',mode=493}}}},
            {name='minimal',type='target',requires={'runtime'}}})}
    local load=config.load;config.load=function()return cfg end
    local ok,value=pcall(function()return require('init').main({'--config','/fixture','--activate-system','--run-for','1'},arg[1])end)
    config.load=load;for _,name in ipairs(names)do package.loaded[name]=saved[name]end
    assert(ok,value);assert(value==0 and f.mutations==2 and #f.rows==1)
    assert(table.concat(operations,',')=='lock,control,unlock')
end)
print('PASS Phase III policy tests: '..count..' (mock inventory/kernel operations)')
