-- SPDX-License-Identifier: BSD-2-Clause
local config_module=require('config');local dependency=require('dependency');local json=require('json')
local M={}
local function failure(error)
    return type(error)=='table' and (error.message or json.encode(error)) or tostring(error)
end
local function read_state(path,native)
    local exists,error=native.exists(path);assert(exists~=nil,error and error.message)
    if not exists then return {} end
    local metadata=assert(native.stat(path));assert(metadata.type=='file'and metadata.uid==native.identity().euid and (metadata.mode&0x12)==0,'unsafe enabled state file')
    local file=assert(io.open(path,'r'));local text=file:read(65537);file:close()
    assert(#text<=65536,'enabled state file too large');local value=json.decode(text)
    assert(type(value)=='table'and value.version==1 and type(value.enabled)=='table','invalid enabled state')
    for name,enabled in pairs(value.enabled)do assert(require('service').name(name)and type(enabled)=='boolean','invalid enabled state entry')end
    return value.enabled
end
local function parse(args)
    local opts={};local value={['--config']='config',['--target']='target',['--runtime-dir']='runtime_dir',['--state-file']='state_file',['--scenario']='scenario',['--run-for']='run_for'}
    local flags={['--check']='check',['--graph']='graph',['--dry-run']='dry',['--simulate']='simulate',['--shadow']='shadow',['--help']='help'}
    local i=1
    while args[i]do
        if value[args[i]]then opts[value[args[i]]]=assert(args[i+1],'missing option value');i=i+2
        else assert(flags[args[i]],'unknown option '..args[i]);opts[flags[args[i]]]=true;i=i+1 end
    end
    return opts
end
local function simulation(config,module_dir,scenario)
    local fixture=module_dir..'/fixture.lua'
    for _,d in pairs(config.registry)do
        d.system=nil
        if d.type~='target'then
            local kind=d.type=='daemon'and'daemon'or'oneshot'
            d.type=kind;d.action=nil;d.condition=nil;d.parameters=nil
            d.readiness=kind=='daemon'and'notify'or'process'
            d.command={'/usr/bin/lua','-E',fixture,kind,'0.02'}
            d.start_timeout=3;d.stop_timeout=0.1
        end
    end
    local dns=config.registry.dns
    if dns and scenario~='success'then
        if scenario=='failure'then
            dns.command[4]='fail';dns.restart='never'
            -- Inject a hard dependent so this fixture demonstrates propagation.
            if config.registry.sshd then table.insert(config.registry.sshd.requires,'dns')end
        elseif scenario=='exhaustion'or scenario=='recovery'then
            dns.command[4]='crash';dns.restart='on-failure';dns.restart_limit=2;dns.restart_delay=0.05;dns.restart_max_delay=0.1
        elseif scenario=='forced'then dns.command[4]='stubborn'
        else error('unknown simulation scenario')end
    end
end
function M.main(args,module_dir)
    local opts=parse(args)
    if opts.help then io.write('halfinit [--config FILE] [--target NAME] [--runtime-dir DIR] [--run-for SECONDS] [--check|--graph|--dry-run|--shadow|--simulate] [--scenario success|failure|recovery|exhaustion|forced]\n');return 0 end
    if opts.run_for then
        opts.run_for=tonumber(opts.run_for)
        assert(opts.run_for and opts.run_for>0 and opts.run_for<=3600,'--run-for must be >0 and <=3600 seconds')
    end
    local config=config_module.load(opts.config,module_dir..'/services/system.lua')
    assert(not opts.scenario or opts.simulate,'--scenario requires --simulate')
    if opts.simulate then
        local scenarios={success=true,failure=true,recovery=true,exhaustion=true,forced=true}
        assert(scenarios[opts.scenario or 'success'],'unknown simulation scenario')
    end
    config.target=opts.target or config.target;assert(config.registry[config.target],'unknown target')
    local plan=dependency.plan(config.registry,{[config.target]=true})
    local native=require('halfbsd.os')
    if opts.check or opts.graph or opts.dry or opts.shadow then
        local missing={}
        for name in pairs(plan.selected)do
            local d=config.registry[name]
            if d.command and not native.exists(d.command[1])then missing[#missing+1]={service=name,path=d.command[1]}end
            if d.condition=='manual'then missing[#missing+1]={service=name,prerequisite='explicit provider/session policy'}end
            if d.action=='mount'and not(d.parameters and d.parameters.options)then missing[#missing+1]={service=name,prerequisite='explicit mount configuration'}end
        end
        local observed={}
        if opts.shadow then
            local flags={local_unbound_enable='dns',sshd_enable='sshd',ntpd_enable='ntpd',seatd_enable='seatd',firewall_enable='firewall'}
            for _,path in ipairs({'/etc/defaults/rc.conf','/etc/rc.conf','/etc/rc.conf.local'})do
                local file=io.open(path,'r')
                if file then
                    local text=file:read(65537);file:close();assert(#text<=65536,'shadow input exceeds bound')
                    for line in text:gmatch('[^\n]+')do
                        local key,value=line:match('^%s*([%w_]+)%s*=%s*["\']?(%u+)')
                        if flags[key]and(value=='YES'or value=='NO')then observed[flags[key]]={rc_enabled=value=='YES',source=path,selected=plan.selected[flags[key]]or false}end
                    end
                end
            end
        end
        local shutdown={};for i=#plan.waves,1,-1 do shutdown[#shutdown+1]=plan.waves[i]end
        io.write(json.encode({valid=true,target=config.target,startup=plan.waves,shutdown=shutdown,missing=missing,
            execution='planning only; existing rc services remain external',shadow=opts.shadow or false,observed=observed})..'\n')
        return 0
    end
    assert(opts.config or opts.simulate,'use --check/--dry-run, --simulate, or an explicit userspace --config')
    if opts.simulate then simulation(config,module_dir,opts.scenario or'success')end
    local uid=native.identity().euid
    local dir=opts.runtime_dir or config.runtime_dir or(uid==0 and'/var/run/halfbsd'or nil)
    assert(dir,'unprivileged managers require an explicit --runtime-dir')
    local stat,err=native.stat(dir)
    if not stat then assert(err.name=='ENOENT');assert(native.mkdir(dir,uid==0 and 0x1ed or 0x1c0));stat=assert(native.stat(dir))end
    assert(stat.type=='directory'and stat.uid==uid and (stat.mode&0x12)==0,'runtime directory must be owned and not group/world writable')
    local ipc=require('halfbsd.ipc');local lock=assert(ipc.lock(dir..'/manager.lock'))
    local state_path=opts.state_file or config.state_file or dir..'/enabled.json'
    local enabled=read_state(state_path,native)
    local logger=assert(require('halfbsd.log').new({service='halfinit',file=config.log_file}))
    local engine
    local function emit(item)
        local r=engine.records[item.service];local result=r.result
        local fields={event='transition',service=item.service,state=item.state,previous=item.previous,reason=item.reason,monotonic=item.time}
        if r.child then fields.pid=r.child.pid elseif result then fields.pid=result.pid end
        if result then fields.exit_status=result.exit_code or(result.signal and('signal:'..result.signal)or result.state)end
        if r.error and r.error.code then fields.errno=r.error.code;fields.error_name=r.error.name end
        local ok,error=logger:write(item.state=='FAILED'and'error'or'info','service state changed',fields)
        if not ok then io.stderr:write('halfinit log error: '..error.message..'\n')end
    end
    engine=require('supervisor').new(config,{providers=require('providers').native(),emit=emit})
    for name,value in pairs(enabled)do if engine.records[name]then engine.blocked[name]=not value or nil;engine.enabled[name]=value end end
    local handlers={}
    handlers.persist=function(name,value)
        local next_enabled={};for n,v in pairs(enabled)do next_enabled[n]=v end;next_enabled[name]=value
        local encoded=json.encode({version=1,enabled=next_enabled})
        assert(#encoded<=65536,'enabled state exceeds persistent size limit')
        assert(native.atomic_replace(state_path,encoded,0x180));enabled=next_enabled;engine.enabled[name]=value
        if value then engine:start({name})else engine:stop(name)end
        logger:write('info','enable preference updated',{event=value and'enable'or'disable',service=name})
        return {persisted=true}
    end
    handlers.reload=function()
        assert(opts.config,'system graph reload requires an explicit config')
        local candidate=config_module.load(opts.config,module_dir..'/services/system.lua')
        candidate.target=opts.target or candidate.target
        assert(candidate.target==config.target,'use explicit start/stop to change targets during this manager session')
        for _,key in ipairs({'runtime_dir','state_file','log_file'})do assert(candidate[key]==config[key],'cannot relocate runtime/state/log paths during reload')end
        for name,r in pairs(engine.records)do
            local next_definition=candidate.registry[name]
            if r.child or r.state=='READY'or r.state=='STARTING'then
                assert(next_definition and next_definition.type==r.definition.type,'stop active service before removal/type change')
                for _,kind in ipairs({'requires','wants','after','before'})do assert(json.encode(next_definition[kind])==json.encode(r.definition[kind]),'stop active service before dependency change')end
            end
        end
        for name in pairs(engine.roots)do assert(candidate.registry[name],'reload removes requested root')end
        -- All validation precedes this commit; running processes retain their current launch attributes.
        for name,r in pairs(engine.records)do if not candidate.registry[name]then
            engine:clear(r);engine.records[name]=nil;engine.enabled[name]=nil;engine.blocked[name]=nil;engine.holds[name]=nil
        end end
        for name,d in pairs(candidate.registry)do
            if engine.records[name]then engine.records[name].definition=d
            else
                engine.records[name]={definition=d,state='INACTIVE',history={},attempts={},epoch=0,watches={},wanted=false}
                engine.enabled[name]=enabled[name]
                if enabled[name]==false then engine.blocked[name]=true end
            end
            if enabled[name]==nil then
                engine.enabled[name]=d.enabled
                -- Definition preferences change without clearing explicit stops.
                if d.enabled==false then
                    engine.blocked[name]=true;engine:degrade(name)
                    engine:stop_record(engine.records[name],'disabled by configuration')
                elseif not engine.holds[name]then engine.blocked[name]=nil
                end
            end
            if not engine.holds[name]and(enabled[name]==true or(enabled[name]==nil and d.enabled==true))then engine.roots[name]=true end
        end
        config=candidate;engine.config=candidate;engine:replan();engine:pump()
        logger:write('info','configuration reloaded',{event='reload'});return {reloaded=true}
    end
    local control
    local ok,problem=xpcall(function()
        control=require('control').new(engine.loop,dir..'/control.sock',uid,engine,handlers)
        for _,sig in ipairs({'TERM','INT','HUP'})do
            assert(engine.loop:watch_signal(sig,function()
                if sig=='HUP'then local success,error=pcall(handlers.reload);if not success then logger:write('error','reload rejected',{event='reload',reason=failure(error)})end
                else engine:shutdown()end
            end))
        end
        if opts.run_for then assert(engine.loop:watch_timer(opts.run_for,function()engine:shutdown()end))end
        engine:start({config.target})
        for name,d in pairs(config.registry)do if (enabled[name]==true or(d.enabled and enabled[name]~=false))and not engine.blocked[name]then engine:start({name})end end
        if opts.simulate then
            local function conclude()
                engine.simulation_snapshot=engine:status();engine:shutdown()
            end
            assert(engine.loop:watch_timer(opts.scenario=='recovery'and 0.3 or 0.7,function()
                if opts.scenario=='recovery'and engine.records.dns then
                    engine.records.dns.definition.command[4]='daemon';engine:start({'dns'})
                    assert(engine.loop:watch_timer(0.7,conclude))
                else conclude()end
            end))
        end
        local completed,loop_error=engine.loop:run();if not completed then error(loop_error.message)end
    end,function(e)return failure(e)..'\n'..debug.traceback()end)
    if control then control:close()end
    engine.loop:close()
    local cleaned,cleanup_error=engine.proc.shutdown(0.1,true)
    lock:close();logger:close()
    if not ok then io.stderr:write(problem..'\n');return 1 end
    if not cleaned then io.stderr:write(cleanup_error.message..'\n');return 1 end
    if next(engine.shutdown_failed)then io.stderr:write(json.encode(engine.shutdown_failed)..'\n');return 1 end
    if opts.simulate then io.write(json.encode({simulation=true,scenario=opts.scenario or'success',
        startup_status=engine.simulation_snapshot,status=engine:status(),cleaned=true})..'\n')end
    return 0
end
return M
