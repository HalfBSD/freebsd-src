-- SPDX-License-Identifier: BSD-2-Clause
-- Run from a private temporary working directory on native FreeBSD only.
local manager,client,module_dir,system_file=assert(arg[1]),assert(arg[2]),assert(arg[3]),assert(arg[4])
package.path=module_dir..'/?.lua'
local json=require('json');local ipc=require('halfbsd.ipc');local event=require('halfbsd.event')
local proc=require('proc');local native=require('halfbsd.os');local exchange=require('client').exchange
local fixture=module_dir..'/fixture.lua';local cwd=assert(proc.run({'/bin/pwd'},{stdout='pipe',timeout=2})).stdout:gsub('\n$','')
local socket=cwd..'/runtime/control.sock';local configuration=cwd..'/system.lua';local managed
local function check(v,e)if v==nil then error(e and(e.operation or e.kind)..': '..e.message or 'nil result')end;return v end
local function pause(seconds)local loop <close> =check(event.new());check(loop:watch_timer(seconds,function()end));check(loop:run())end
local function request(command,name)return exchange(socket,{version=1,command=command,service=name},3)end
local function until_state(name,state,timeout)
    local deadline=native.clock()+(timeout or 5)
    repeat
        local value=request('status',name)
        if value and value.state==state then return value end
        pause(0.01)
    until native.clock()>deadline
    error('service '..name..' did not become '..state)
end
local function definition(name,mode,more)
    local d={name=name,type=mode=='oneshot'and'oneshot'or'daemon',command={'/usr/bin/lua','-E',fixture,mode,'0.001'},
        readiness=mode=='oneshot'and'process'or'notify',start_timeout=2,stop_timeout=0.05,restart='never'}
    for k,v in pairs(more or{})do d[k]=v end
    return d
end
local function serialize(value)
    if type(value)=='table'then local parts={};for k,v in pairs(value)do parts[#parts+1]='['..serialize(k)..']='..serialize(v)end;return'{'..table.concat(parts,',')..'}'end
    if type(value)=='string'then return string.format('%q',value)end
    return tostring(value)
end
local definitions={definition('worker','daemon'),definition('stubborn','stubborn'),definition('crash','crash',{restart='on-failure',restart_limit=2,restart_delay=0.02,restart_max_delay=0.04}),
    definition('timeout','daemon',{start_timeout=0.03}),{name='dependent',type='target',requires={'crash'}},
    {name='boot',type='target'},definition('missing','daemon',{command={cwd..'/missing-executable'}}),
    definition('slow','daemon',{start_timeout=3})}
definitions[4].command[5]='1'
definitions[8].command[5]='1'
local function write_config(defs)
    check(native.atomic_replace(configuration,'return '..serialize({services=defs,target='boot',runtime_dir=cwd..'/runtime',shutdown_timeout=3,max_parallel=128}),0x180))
end
local function start()
    managed=check(proc.spawn({manager,'--module-dir',module_dir,'--config',configuration,'--run-for','540'},{new_session=true,stdout={path='manager.stdout'},stderr={path='manager.stderr'}}))
    local deadline=native.clock()+5
    repeat
        if native.exists(socket)then local value=request('list');if value then return end end
        local status=managed:status();assert(status.state=='running','manager exited during startup');pause(0.01)
    until native.clock()>deadline
    error('manager control socket unavailable')
end
local function stop()
    check(request('shutdown'));local result=check(managed:wait(8));assert(result.success,'manager shutdown failed');managed=nil
end
local ok,problem=xpcall(function()
    write_config(definitions);start()
    check(request('start','worker'));until_state('worker','READY')
    local cli=check(proc.run({client,'--module-dir',module_dir,'--socket',socket,'status','worker'},
        {stdout='pipe',stderr='pipe',timeout=5}))
    assert(cli.success and json.decode(cli.stdout).state=='READY',cli.stderr)
    local first=check(request('status','worker')).generation
    check(request('restart','worker'));local deadline=native.clock()+4
    repeat
        local value=request('status','worker');if value and value.state=='READY'and value.generation>first then break end
        pause(0.01);assert(native.clock()<deadline,'restart did not complete')
    until false
    print('PASS readiness and runtime start/restart')
    check(request('start','timeout'));until_state('timeout','FAILED')
    assert(not check(request('status','timeout')).pid)
    print('PASS readiness timeout releases child ownership')
    check(request('start','missing'));local missing=until_state('missing','FAILED')
    assert(missing.error and missing.error.name=='ENOENT'and not missing.pid)
    print('PASS execution failure retains native errno without an owned child')
    check(request('start','stubborn'));until_state('stubborn','READY');check(request('stop','stubborn'));until_state('stubborn','INACTIVE')
    print('PASS forced service stop')
    check(request('start','dependent'));until_state('crash','FAILED')
    assert(check(request('status','crash')).exhausted)
    assert(check(request('status','dependent')).state~='READY')
    definitions[3]=definition('crash','daemon',{restart='on-failure'})
    definitions[3].requires={}
    write_config(definitions);check(request('reload'));check(request('start','crash'))
    until_state('dependent','READY');print('PASS restart exhaustion and dependency recovery')
    check(native.atomic_replace(configuration,'return {invalid=true}',0x180))
    local value,error=request('reload');assert(value==nil and error)
    assert(check(request('status','worker')).state=='READY');write_config(definitions)
    for _=1,10 do check(request('reload'))end
    print('PASS valid/invalid transactional reload')
    local connections={}
    for _=1,40 do local fd=ipc.connect(socket,1);if fd then connections[#connections+1]=fd end end
    for _,fd in ipairs(connections)do fd:close()end
    pause(0.05);check(request('list'));print('PASS disconnected control clients')
    definitions[#definitions+1]=definition('configauto','daemon',{enabled=true})
    write_config(definitions);check(request('reload'));until_state('configauto','READY')
    check(request('stop','configauto'));until_state('configauto','INACTIVE')
    check(request('reload'));assert(check(request('status','configauto')).state=='INACTIVE')
    definitions[#definitions]=nil;write_config(definitions);check(request('reload'))
    print('PASS reload activation preferences, explicit stop holds and inactive removal')
    check(request('disable','worker'));until_state('worker','INACTIVE');stop();start()
    assert(check(request('status','worker')).enabled==false)
    check(request('enable','worker'));until_state('worker','READY');stop();start();until_state('worker','READY')
    check(request('stop','worker'));until_state('worker','INACTIVE')
    print('PASS enable/disable persistence across manager restart')
    -- Actual foreground processes: 50 concurrent services, not a mock backend.
    for i=1,50 do definitions[#definitions+1]=definition('parallel'..i,'daemon')end
    local required={};for i=1,50 do required[i]='parallel'..i end
    definitions[#definitions+1]={name='parallel',type='target',requires=required}
    write_config(definitions);check(request('reload'));check(request('start','parallel'));until_state('parallel','READY',10)
    local active=0;for _,r in ipairs(check(request('list')))do if r.pid and r.state=='READY'then active=active+1 end end
    assert(active>=50);check(request('stop','parallel'))
    -- Stop explicit constituent services before the repeated-worker stress.
    for i=1,50 do check(request('stop','parallel'..i))end
    print('PASS 50 concurrent supervised processes')
    math.randomseed(42);local tasks={}
    for i=1,100 do
        local d=definition('task'..i,'oneshot')
        if i>3 then d.requires={'task'..math.random(1,i-1)}end
        tasks[#tasks+1]=d.name;definitions[#definitions+1]=d
    end
    definitions[#definitions+1]={name='tasks',type='target',requires=tasks}
    write_config(definitions);check(request('reload'));check(request('start','tasks'));until_state('tasks','READY',15)
    local rows={};for _,row in ipairs(check(request('list')))do rows[row.name]=row end
    for _,d in ipairs(definitions)do if d.name:match('^task%d')then
        assert(rows[d.name].state=='READY')
        for _,parent in ipairs(d.requires or{})do assert(rows[d.name].started_at>=rows[parent].transition.time)end
    end end
    print('PASS 100 native randomized dependency tasks')

    for i=1,1000 do
        check(request('start','worker'));until_state('worker','READY')
        check(request('stop','worker'));until_state('worker','INACTIVE')
    end
    print('PASS 1000 native start/stop operations')
    -- Kernel peer credentials are acquired by the server; requests cannot supply UID.
    local value,error=request('eval','worker');assert(not value and error)
    local forbidden=exchange(socket,{version=1,command='start',service='worker',uid=0},1);assert(forbidden==nil)
    print('PASS protocol rejects arbitrary execution and spoofed credentials')
    if native.identity().euid==0 then
        local nobody=check(native.user('nobody'))
        check(native.chmod(cwd,0x1ed)) -- Only the disposable test directory.
        local peer_test=cwd..'/peer.lua'
        check(native.atomic_replace(peer_test,'package.path='..string.format('%q',module_dir..'/?.lua')..'; local exchange=require("client").exchange; '..
            'assert(exchange('..string.format('%q',socket)..',{version=1,command="status"},3)); '..
            'local value,err=exchange('..string.format('%q',socket)..',{version=1,command="start",service="worker"},3); assert(value==nil and err)',0x1a4))
        local result=check(proc.run({'/usr/bin/lua','-E',peer_test},{user=nobody.uid,group=nobody.gid,groups={},stdout='pipe',stderr='pipe',timeout=5}))
        assert(result.success,result.stderr);print('PASS isolated native cross-UID read-only/mutation authorization')
    else print('SKIP cross-UID credential test (requires root in isolated fixtures)')end
    check(request('start','slow'));until_state('slow','STARTING')
    check(request('start','worker'));check(managed:signal('TERM',true));assert(check(managed:wait(8)).success);managed=nil
    assert(not native.exists(socket));print('PASS manager TERM cleans owned services')
    local log=assert(io.open('manager.stderr','r'));local text=log:read('*a');log:close()
    assert(text:find('"event":"transition"',1,true)and text:find('"previous":',1,true))
    print('PASS structured lifecycle logging')
    local callback_test=cwd..'/callback.lua'
    check(native.atomic_replace(callback_test,'local event=require("halfbsd.event"); local proc=require("proc"); '..
        'local child=assert(proc.spawn({"/usr/bin/lua","-E",'..string.format('%q',fixture)..',"daemon","1"},{new_session=true,stdout="null",stderr="null"})); '..
        'local loop <close> = assert(event.new()); assert(loop:watch_timer(0.01,function()error("injected callback failure")end)); '..
        'local ok,err=loop:run(); assert(not ok and err); assert(proc.shutdown(0.05,true)); assert(#proc.active()==0)',0x180))
    local callback_result=check(proc.run({'/usr/bin/lua','-E',callback_test},{stdout='pipe',stderr='pipe',timeout=5}))
    assert(callback_result.success,callback_result.stderr);print('PASS native callback failure and explicit resource cleanup')
    local dry=check(proc.run({manager,'--module-dir',module_dir,'--config',configuration,'--dry-run','--target','parallel'},
        {stdout='pipe',stderr='pipe',timeout=5}));assert(dry.success and json.decode(dry.stdout).valid)
    assert(not native.exists(socket));print('PASS dry run leaves manager inactive')
    -- Exercise the proposed system graph using the actual engine with fixtures.
    local system=cwd..'/simulation.lua'
    check(native.atomic_replace(system,'return {include={'..string.format('%q',system_file)..'},target="desktop"}',0x180))
    for _,scenario in ipairs({'success','failure','recovery','exhaustion','forced'})do
        local result=check(proc.run({manager,'--module-dir',module_dir,'--config',system,'--simulate','--scenario',scenario,'--runtime-dir',cwd..'/runtime'},
            {stdout='pipe',stderr={path='simulation-'..scenario..'.stderr'},timeout=15,output_limit=1048576}))
        assert(result.success,'simulation '..scenario..' failed');local report=json.decode(result.stdout);assert(report.cleaned)
        local dns;for _,row in ipairs(report.status)do if row.name=='dns'then dns=row end end
        assert(dns,'simulation omitted DNS fixture')
        if scenario=='failure'then
            assert(dns.state=='FAILED'and not dns.ready_at)
            for _,row in ipairs(report.startup_status)do if row.name=='sshd'then assert(row.state=='SKIPPED')end end
        elseif scenario=='exhaustion'then assert(dns.exhausted and dns.restarts==2)
        elseif scenario=='recovery'then assert(dns.ready_at and dns.generation>1 and not dns.exhausted)
        else
            assert(dns.ready_at)
            if scenario=='forced'then assert(dns.result.signal==9)end
        end
    end
    print('PASS complete synthetic desktop boot/shutdown scenarios')
end,debug.traceback)
if managed then managed:terminate(1,true)end
check(proc.shutdown(0.1,true))
if not ok then io.stderr:write(problem..'\n');os.exit(1,true)end
print('PASS Phase II native service manager integration/stress suite')
