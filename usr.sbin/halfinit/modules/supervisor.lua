-- SPDX-License-Identifier: BSD-2-Clause
local service=require('service')
local dependency=require('dependency')
local M={};M.__index=M
local settled={READY=true,FAILED=true,SKIPPED=true}
local alive={STARTING=true,READY=true,STOPPING=true}
local function error_text(value)return type(value)=='table'and(value.message or 'native operation failed')or tostring(value)end
function M.new(config,options)
    options=options or {}
    local self=setmetatable({config=config,records={},roots={},blocked={},holds={},enabled={},loop=options.loop or assert(require('halfbsd.event').new()),
        proc=options.proc or require('proc'),native=options.native or require('halfbsd.os'),providers=options.providers or {},
        emit=options.emit,on_shutdown=options.on_shutdown,system_allowed=options.system_allowed or false,shutting=false,shutdown_failed={},pumping=false,again=false},M)
    self.clock=function()return self.native.clock('monotonic')end
    for name,d in pairs(config.registry)do
        self.records[name]={definition=d,state='INACTIVE',history={},attempts={},epoch=0,watches={},wanted=false}
        if d.enabled~=nil then self.enabled[name]=d.enabled end
        if d.enabled==false then self.blocked[name]=true end
    end
    return self
end
function M:transition(r,state,reason)
    service.transition(r,state,reason,self.clock,self.emit)
end
function M:watch(r,kind,...)
    local args={...};local callback=table.remove(args)
    local epoch=r.epoch;local id
    args[#args+1]=function(e)
        if r.epoch~=epoch then return end
        if kind=='watch_timer' or kind=='watch_process' then r.watches[id]=nil end
        callback(e)
    end
    id=assert(self.loop[kind](self.loop,table.unpack(args)))
    r.watches[id]=true;return id
end
function M:cancel(r,id)if id then self.loop:cancel(id);r.watches[id]=nil end end
function M:clear(r)
    for id in pairs(r.watches)do self.loop:cancel(id)end;r.watches={};r.epoch=r.epoch+1
    if r.notify then r.notify:close();r.notify=nil end
    if r.output then r.output:close();r.output=nil end
    if self.providers.release then self.providers.release(r.launch_definition or r.definition)end
end
function M:complete_oneshot(r)
    local d=r.launch_definition or r.definition
    if r.state~='STARTING'or not r.result or(d.verify and not r.output_done)then return end
    local accepted=r.result.success
    if d.success_codes and r.result.state=='exited'then
        accepted=false;for _,code in ipairs(d.success_codes)do if r.result.exit_code==code then accepted=true end end
    end
    if not accepted then self:failure(r,'oneshot process failed');return end
    if d.verify then
        local provider=self.providers[d.verify]
        if not provider then self:failure(r,'unknown output verifier');return end
        local ok,value=pcall(provider,d.parameters or{},d,r.output_buffer or'')
        if not ok or not value then self:failure(r,'output verification failed: '..error_text(value),type(value)=='table'and value or nil);return end
    end
    self:clear(r);self:ready(r)
end
function M:ready(r)
    if r.state~='STARTING' then return end
    self:cancel(r,r.start_watch);self:cancel(r,r.condition_watch)
    r.ready_at=self.clock()
    self:transition(r,'READY','readiness satisfied');self:pump()
end
function M:degrade(name)
    for other,r in pairs(self.records)do
        for _,required in ipairs(r.definition.requires)do
            if required==name and alive[r.state] and r.state~='STOPPING' then
                self:stop_record(r,'required service unavailable: '..name)
                self:degrade(other)
            end
        end
    end
end
function M:failure(r,reason,native_error)
    if r.state=='FAILED' then return end
    if native_error then r.error=native_error end
    if not r.child then self:cleanup_record(r)end
    self:clear(r);r.child=nil
    self:transition(r,'FAILED',reason);self:degrade(r.definition.name)
    local d=r.definition
    local retry=d.type=='daemon' and r.wanted and not self.shutting and
        (d.restart=='always' or (d.restart=='on-failure' and (r.start_failure or not (r.result and r.result.success))))
    if retry then
        local now=self.clock();local attempts={}
        for _,t in ipairs(r.attempts)do if now-t<d.restart_window then attempts[#attempts+1]=t end end
        r.attempts=attempts
        if #attempts<d.restart_limit then
            local delay=math.min(d.restart_max_delay,d.restart_delay*2^math.min(#attempts,30))
            attempts[#attempts+1]=now;r.next_restart=now+delay
            self:transition(r,'RESTARTING',reason..'; retry in '..delay..' seconds')
            self:watch(r,'watch_timer',delay,function()
                if r.wanted and not self.shutting then self:transition(r,'QUEUED','restart backoff elapsed')
                else self:transition(r,'INACTIVE','restart canceled')end
                r.next_restart=nil;self:pump()
            end)
        else r.exhausted=true end
    end
    self:pump()
end
function M:exit(r,result)
    r.result=result;r.child=nil
    if r.foreground then
        local ok,err=self.native.terminal_restore()
        if not ok then self.shutdown_failed[r.definition.name]=error_text(err)end
        r.foreground=nil;self.terminal_owner=nil
    end
    if r.state=='STOPPING' then
        local failure=r.stop_failure
        self:clear(r);r.stop_failure=nil
        if failure then self:failure(r,failure)
        else self:transition(r,'INACTIVE','process reaped')end
    elseif r.definition.type=='oneshot' then
        self:complete_oneshot(r)
    else self:failure(r,result.error and 'child ownership lost' or 'process exited')end
    self:pump()
end
function M:cleanup_record(r)
    local d=r.launch_definition
    if r.cleanup_error then return false end
    if not d or not d.cleanup or r.cleanup_attempted then return true end
    r.cleanup_attempted=true
    local ok,value=pcall(function()
        return assert(self.providers[d.cleanup],'unknown cleanup provider')(d.parameters or{},d,self)
    end)
    if not ok or not value then
        r.cleanup_error=error_text(value);self.shutdown_failed[d.name]=r.cleanup_error;return false
    end
    return true
end
function M:cleanup_actions()
    local roots={}
    for name,r in pairs(self.records)do if r.launch_definition and r.launch_definition.cleanup then roots[name]=true end end
    local plan=dependency.plan(self.config.registry,roots)
    for i=#plan.waves,1,-1 do
        for _,name in ipairs(plan.waves[i])do
            local r=self.records[name]
            if not r.child or r.child:status().state~='running'then self:cleanup_record(r)
            else self.shutdown_failed[name]='active child prevents native cleanup'end
        end
    end
    for _,r in pairs(self.records)do self:clear(r)end
end
function M:stop_record(r,reason,failure)
    if r.state=='STOPPING' then if failure then r.stop_failure=failure end;return end
    if r.state=='INACTIVE' then return end
    if not r.child then
        if failure then self:failure(r,failure);return end
        if not self:cleanup_record(r)then self:failure(r,'cleanup failed');return end
        self:clear(r)
        if r.state=='READY' or r.state=='STARTING' then self:transition(r,'STOPPING',reason)end
        self:transition(r,'INACTIVE',reason);return
    end
    self:cancel(r,r.start_watch);self:cancel(r,r.condition_watch)
    self:transition(r,'STOPPING',reason);r.stop_failure=failure
    if failure then r.start_failure=true end
    local delivered,err=r.child:signal('TERM',true)
    if delivered==nil then self.shutdown_failed[r.definition.name]=err.message end
    self:watch(r,'watch_timer',r.definition.stop_timeout,function()
        if r.child then
            if self.shutting then r.shutdown_forced='service stop timeout'end
            local killed,error=r.child:signal('KILL',true)
            if killed==nil then self.shutdown_failed[r.definition.name]=error.message end
        end
    end)
end
function M:start_record(r)
    if r.cleanup_error then self:transition(r,'FAILED','previous cleanup failed; manager restart required');return end
    local d=r.definition;r.launch_definition=d;r.result=nil;r.error=nil;r.ready_at=nil;r.start_failure=nil;r.epoch=r.epoch+1;r.exhausted=nil
    r.output_buffer='';r.output_done=nil;r.cleanup_attempted=nil
    self:transition(r,'STARTING','dependencies satisfied')
    r.started_at=self.clock();r.generation=(r.generation or 0)+1
    if d.system and not self.system_allowed then self:failure(r,'system task requires explicit --activate-system');return end
    if d.guard then
        local provider=self.providers[d.guard]
        if not provider then self:failure(r,'unknown startup guard');return end
        local ok,value=pcall(provider,d.parameters or{},d,self)
        if not ok or not value then self:failure(r,'startup guard failed: '..error_text(value),type(value)=='table'and value or nil);return end
        if value=='ready'then assert(d.type=='oneshot','a guard cannot adopt a daemon');self:ready(r);return end
    end
    if d.foreground then
        local terminal=self.native.terminal()
        if self.terminal_owner or not terminal.tty or not terminal.foreground or self.native.identity().euid==0 then
            self:failure(r,'foreground desktop needs the current non-root login terminal');return
        end
    end
    if d.type=='target' then self:ready(r);return end
    if d.type=='virtual' or d.action then
        local provider=self.providers[d.condition or d.action]
        if not provider then self:failure(r,'unknown provider');return end
        local function check()
            local ok,value=pcall(provider,d.parameters or {},d)
            if not ok then self:failure(r,'provider failed: '..error_text(value),type(value)=='table'and value or nil);return end
            if value then self:ready(r)end
        end
        r.start_watch=self:watch(r,'watch_timer',d.start_timeout,function()self:failure(r,'condition deadline')end)
        if d.action then check();if r.state=='STARTING' then self:failure(r,'action failed')end
        else
            check()
            if r.state=='STARTING' then
                local function retry()
                    check();if r.state=='STARTING' then r.condition_watch=self:watch(r,'watch_timer',0.1,retry)end
                end
                r.condition_watch=self:watch(r,'watch_timer',0.1,retry)
            end
        end
        return
    end
    local executable=d.command[1]:match('([^/]+)$')
    if executable=='sway'or executable=='niri'then
        local user=d.user and self.native.user(d.user)or{uid=self.native.identity().euid}
        if not user or user.uid==0 then self:failure(r,'compositors require an explicit non-root user session');return end
    end
    local opts={cwd=d.cwd,user=d.user,group=d.group,groups=d.groups,env=d.env,inherit_env=d.inherit_env,
        new_session=d.new_session~=false,process_group=true,stdin='null',stdout='null',stderr='null',exec_timeout=math.min(d.start_timeout,5)}
    if d.stdin=='inherit'then opts.stdin=nil end
    for _,key in ipairs({'stdout','stderr'})do
        if d[key]and d[key]~='null'then opts[key]={path=d[key],append=true,mode=0x180}end
    end
    if d.verify then opts.stdout='pipe'end
    local reader,writer
    if d.readiness=='notify' then
        reader,writer=self.proc.pipe(true)
        if not reader then self:failure(r,'notification pipe unavailable',writer);return end
        opts.inherit_fds={writer};opts.env=opts.env or {};local copy={};for k,v in pairs(opts.env)do copy[k]=v end
        copy.HALFINIT_READY_FD=tostring(writer:fileno());opts.env=copy
    end
    local child,err=self.proc.spawn(d.command,opts)
    if not child then
        if reader then reader:close();writer:close()end
        self:failure(r,(err.operation or 'spawn')..': '..(err.stage or err.kind or 'failure')..': '..err.message,err);return
    end
    r.child=child
    self:watch(r,'watch_process',child,function(e)self:exit(r,e.result)end)
    local remaining=d.start_timeout-(self.clock()-r.started_at)
    r.start_watch=self:watch(r,'watch_timer',math.max(0.001,remaining),function()
        self:stop_record(r,'startup deadline','readiness timeout')
    end)
    if d.foreground then
        local ok,err=child:foreground()
        if not ok then
            if writer then writer:close();reader:close()end
            r.error=err;self:stop_record(r,'terminal handoff failed','terminal handoff failed');return end
        r.foreground=true;self.terminal_owner=r
    end
    if remaining<=0 then
        if writer then writer:close();reader:close()end
        self:stop_record(r,'startup deadline','readiness timeout');return
    end
    if d.verify then
        r.output=assert(child.stdout);assert(r.output:nonblock(true));local id
        id=self:watch(r,'watch_read',r.output,function()
            while true do
                local bytes,read_error=r.output:read(4096)
                if not bytes then
                    if read_error and read_error.name=='EAGAIN'then return end
                    if read_error then self:stop_record(r,'stdout error','stdout capture failed');return end
                    self:cancel(r,id);r.output:close();r.output=nil;r.output_done=true;self:complete_oneshot(r);return
                end
                r.output_buffer=r.output_buffer..bytes
                if #r.output_buffer>65536 then self:stop_record(r,'output limit','verified output exceeds 64 KiB');return end
            end
        end)
    end
    if writer then
        writer:close();r.notify=reader;local bytes=''
        local id
        id=self:watch(r,'watch_read',reader,function()
            while true do
                local data,error=reader:read(1024)
                if not data then
                    if error and error.name=='EAGAIN' then return end
                    self:cancel(r,id);reader:close();r.notify=nil
                    if r.state=='STARTING' then self:stop_record(r,'notification closed','notification EOF/error')end
                    return
                end
                bytes=bytes..data
                if #bytes>1024 then self:stop_record(r,'invalid notification','notification too large');return end
                if bytes=='READY\n' then self:cancel(r,id);reader:close();r.notify=nil;self:ready(r);return end
                if bytes:find('\n',1,true) then self:stop_record(r,'invalid notification','invalid readiness record');return end
            end
        end)
    elseif d.type=='daemon' and d.readiness=='process' then self:ready(r)
    elseif d.type=='daemon' then
        local provider=self.providers[d.condition]
        if not provider then self:stop_record(r,'missing provider','unknown condition');return end
        local function check()
            local ok,value=pcall(provider,d.parameters or {},d)
            if not ok then
                if type(value)=='table'then r.error=value end
                self:stop_record(r,'provider failure','condition error: '..error_text(value));return
            end
            if value then self:ready(r)
            elseif r.state=='STARTING' then r.condition_watch=self:watch(r,'watch_timer',0.1,check)end
        end
        check()
    end
end
function M:replan()
    self.plan=dependency.plan(self.config.registry,self.roots)
    self.predecessors={}
    for name in pairs(self.records)do self.predecessors[name]={};self.records[name].wanted=self.plan.selected[name]and not self.blocked[name]or false end
    for a,edges in pairs(self.plan.edges)do for b in pairs(edges)do self.predecessors[b][#self.predecessors[b]+1]=a end end
end
function M:pump()
    if self.pumping then self.again=true;return end
    self.pumping=true
    repeat
        self.again=false
        if self.shutting then self:shutdown_step()
        else
            local starting=0;for _,r in pairs(self.records)do if r.state=='STARTING'then starting=starting+1 end end
            for _,name in ipairs(dependency.sorted(self.records))do
                local r=self.records[name];local hard_failed=false;local hard_ready=true
                for _,n in ipairs(r.definition.requires)do
                    local required=self.records[n]
                    if not required.wanted or required.state=='FAILED' or required.state=='SKIPPED'then hard_failed=true end
                    if required.state~='READY'then hard_ready=false end
                end
                if r.wanted then
                    if (r.state=='INACTIVE' or r.state=='SKIPPED')and not hard_failed then self:transition(r,'QUEUED','selected');self.again=true end
                    if r.state=='QUEUED' then
                        if hard_failed then self:transition(r,'SKIPPED','required service failed/disabled');self.again=true
                        else
                            local ordered=true
                            for _,n in ipairs(self.predecessors[name]or{})do local p=self.records[n];if p.wanted and not settled[p.state]then ordered=false end end
                            if hard_ready and ordered and starting<self.config.max_parallel then
                                self:start_record(r);if r.state=='STARTING'then starting=starting+1 end;self.again=true
                            end
                        end
                    end
                elseif r.state=='QUEUED' or r.state=='RESTARTING' or r.state=='SKIPPED' then
                    self:clear(r);self:transition(r,'INACTIVE','deselected');self.again=true
                end
            end
        end
    until not self.again
    self.pumping=false
end
function M:start(names)
    assert(not self.shutting,'manager is shutting down')
    for _,name in ipairs(names)do
        local r=assert(self.records[name],'unknown service');self.roots[name]=true;self.blocked[name]=nil;self.holds[name]=nil
        if r.state=='FAILED' then r.attempts={};self:transition(r,'INACTIVE','manual retry')end
    end
    self:replan();self:pump()
end
function M:stop(name)
    assert(not self.shutting,'manager is shutting down');assert(self.records[name],'unknown service')
    self.blocked[name]=true;self.holds[name]=true;self.roots[name]=nil
    self:degrade(name);self:stop_record(self.records[name],'operator stop');self:replan();self:pump()
end
function M:restart(name)
    local r=assert(self.records[name],'unknown service')
    if not r.child then
        if not self:cleanup_record(r)then return end
        self:clear(r)
        if r.state=='READY' or r.state=='STARTING'then self:transition(r,'STOPPING','operator restart')end
        if r.state~='INACTIVE'then self:transition(r,'INACTIVE','operator restart')end
        r.attempts={};self:start({name})
    else
        self.roots[name]=true;self.blocked[name]=nil;self.holds[name]=nil;self:degrade(name)
        self:stop_record(r,'operator restart');self:replan();self:pump()
    end
end
function M:shutdown(reason)
    if self.shutting then return end
    local roots={}
    for name,r in pairs(self.records)do
        if (r.launch_definition and r.launch_definition.cleanup and not r.cleanup_attempted)or r.child or r.state=='READY'or r.state=='STARTING'or r.state=='QUEUED'or r.state=='RESTARTING'then roots[name]=true end
    end
    self.shutdown_plan=dependency.plan(self.config.registry,roots)
    self.shutting=true;self.shutdown_wave=#self.shutdown_plan.waves
    self.shutdown_reason=reason or'manager request'
    if self.on_shutdown then
        local ok,err=pcall(self.on_shutdown,self)
        if not ok then self.shutdown_report_error=error_text(err)end
    end
    for _,r in pairs(self.records)do r.wanted=false end
    self.shutdown_deadline=self.clock()+self.config.shutdown_timeout
    self.shutdown_timer=assert(self.loop:watch_timer(self.config.shutdown_timeout,function()
        for name,r in pairs(self.records)do if r.child then
            r.shutdown_forced='overall shutdown deadline'
            r.child:signal('KILL',true);self.shutdown_failed[name]='overall shutdown deadline'
        end end
        self.force_timer=assert(self.loop:watch_timer(5,function()self.finished=true;self.loop:stop()end))
    end))
    self:pump()
end
function M:shutdown_step()
    local waves=self.shutdown_plan.waves
    while self.shutdown_wave>0 do
        local complete=true
        for _,name in ipairs(waves[self.shutdown_wave])do
            local r=self.records[name]
            if r.state~='INACTIVE' and not (r.state=='FAILED'and not r.child and not(r.launch_definition and r.launch_definition.cleanup)) then self:stop_record(r,'manager shutdown')end
            if r.child then complete=false end
        end
        if not complete then return end
        self.shutdown_wave=self.shutdown_wave-1
    end
    -- Include manually started/deselected children not in the latest plan.
    for _,r in pairs(self.records)do if r.child then self:stop_record(r,'manager shutdown');return end end
    self.finished=true;self.loop:cancel(self.shutdown_timer);self.loop:stop()
end
function M:status()
    local result={}
    for _,name in ipairs(dependency.sorted(self.records))do
        local r=self.records[name];result[#result+1]={name=name,type=r.definition.type,state=r.state,wanted=r.wanted,
            enabled=self.enabled[name]~=false,blocked=self.blocked[name]or false,pid=r.child and r.child.pid or nil,result=r.result,transition=r.last_transition,
            started_at=r.started_at,ready_at=r.ready_at,generation=r.generation or 0,error=r.error,
            next_restart=r.next_restart,exhausted=r.exhausted or false,restarts=#r.attempts}
    end
    return result
end
return M
