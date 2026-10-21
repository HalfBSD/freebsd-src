-- SPDX-License-Identifier: BSD-2-Clause
-- Bounded, durable diagnostic snapshots. Never holds a mount open across cleanup.
local json=require('json')
local M={};M.__index=M
local function text(v)return tostring(type(v)=='table'and(v.message or v.name)or v):sub(1,256)end
function M.new(native,directory,warn)
    local self=setmetatable({native=native,directory=directory or'/var/db/halfbsd',events={},dropped=0,
        warn=warn or function(message)io.stderr:write('halfinit shutdown report: '..message..'\n')end},M)
    local ok,err=pcall(function()
        local prefix=''
        for component in self.directory:gmatch('[^/]+')do
            prefix=prefix..'/'..component
            local st,problem=native.stat(prefix)
            if not st and prefix==self.directory then
                assert(problem and problem.name=='ENOENT',problem)
                assert(native.mkdir(prefix,0x1c0));st=assert(native.stat(prefix))
            end
            assert(st and st.type=='directory'and st.uid==0 and(st.mode&0x12)==0,'unsafe report directory '..prefix)
        end
        local st=assert(native.stat(self.directory));self.device=st.device;self.inode=st.inode
        -- Copy the prior boot identity before publishing this boot's identity.
        local readable,previous=pcall(self.read,self,'boot.json')
        if not readable then self.warn('previous boot identity unavailable: '..text(previous));previous=nil end
        self:write('previous-boot.json',previous or{version=1,boot_id='unavailable',identity_available=false})
        local monotonic=assert(native.clock());local realtime=assert(native.clock('realtime'))
        local pid=native.identity().pid
        self.boot={version=1,boot_id=string.format('%d-%.0f-%.0f',pid,realtime*1e6,monotonic*1e6),
            manager_pid=pid,started_at=realtime,started_monotonic=monotonic}
        self:write('boot.json',self.boot)
        self:write_raw('boot-id',self.boot.boot_id..'\n')
        self.available=true
    end)
    if not ok then self.warn(text(err))end
    return self
end
function M:check()
    local st=assert(self.native.stat(self.directory),'report storage disappeared')
    assert(st.type=='directory'and st.uid==0 and(st.mode&0x12)==0 and
        st.device==self.device and st.inode==self.inode,'report storage changed; refusing replacement mount')
end
function M:read(name)
    self:check()
    local path=self.directory..'/'..name;local st,err=self.native.stat(path)
    if not st then assert(err and err.name=='ENOENT',err);return nil end
    assert(st.type=='file'and st.uid==0 and(st.mode&0x12)==0 and st.size<=65536,'unsafe report identity file')
    local file=assert(io.open(path));local data=file:read(65537);file:close()
    assert(data and #data<=65536,'oversized report identity')
    local value=json.decode(data);assert(type(value)=='table'and value.version==1 and type(value.boot_id)=='string','invalid previous boot identity')
    return value
end
function M:write_raw(name,data)
    self:check();assert(#data<=1048576,'shutdown report exceeds 1 MiB')
    assert(self.native.atomic_replace(self.directory..'/'..name,data,0x180))
end
function M:write(name,value)self:write_raw(name,json.encode(value)..'\n')end
function M:safe(fn)
    if not self.available then return false end
    local ok,err=pcall(fn)
    if not ok then self.last_error=text(err);self.warn(self.last_error)end
    return ok
end
function M:begin(engine)
    return self:safe(function()
        if self.started then return end
        self.started=assert(self.native.clock());self.started_at=assert(self.native.clock('realtime'))
        self:checkpoint(engine,'stopping',false)
    end)
end
function M:observe(item)
    if not self.started then return end
    if #self.events>=256 then self.dropped=self.dropped+1;return end
    self.events[#self.events+1]={service=item.service,previous=item.previous,state=item.state,
        reason=text(item.reason),monotonic=item.time}
end
function M:checkpoint(engine,phase,cleaned,details)
    return self:safe(function()
        local services={}
        for name,r in pairs(engine.records)do
            local result=r.result or{}
            services[#services+1]={name=name,state=r.state,pid=r.child and r.child.pid or result.pid,
                exit_code=result.exit_code,signal=result.signal,forced=r.shutdown_forced,
                cleanup_error=r.cleanup_error and text(r.cleanup_error),handle_retained=r.child~=nil}
        end
        table.sort(services,function(a,b)return a.name<b.name end)
        local failures={};for name,reason in pairs(engine.shutdown_failed)do failures[name]=text(reason)end
        self:write('last-shutdown.json',{version=1,boot_id=self.boot.boot_id,manager_pid=self.boot.manager_pid,
            phase=phase,cleaned=cleaned or false,reason=engine.shutdown_reason,
            target=engine.config and engine.config.target,shutdown_started_at=self.started_at,
            elapsed=self.started and(assert(self.native.clock())-self.started)or nil,
            recorded_at=assert(self.native.clock('realtime')),services=services,failures=failures,
            transitions=self.events,dropped_transitions=self.dropped,details=details,report_error=self.last_error})
    end)
end
function M.assess(previous,lua,pid1)
    if not previous or previous.boot_id=='unavailable'then return false,'No previous recorded Lua boot identity yet'end
    if not lua or lua.boot_id~=previous.boot_id then return false,'No shutdown report for the previous recorded Lua boot; old success is not evidence'end
    if lua.phase~='finished'or not lua.cleaned then return false,'Lua cleanup is incomplete or reported failure'end
    if not pid1 or pid1.boot_id~=lua.boot_id then return false,'Matching PID 1 exit evidence is missing'end
    if pid1.phase~='finished'or pid1.wait_result~=1 or not pid1.exited or pid1.exit_code~=0 or pid1.signal~=0 then
        return false,'PID 1 did not record a normal supervisor exit with status zero'
    end
    return true,'Lua cleanup completed and PID 1 reaped the supervisor with status zero'
end
function M.show(args)
    assert(#args==1,'usage: halfsvc shutdown-report')
    local native=require('halfbsd.os');local directory='/var/db/halfbsd'
    local function read(name)
        local path=directory..'/'..name;local st,err=native.stat(path)
        if not st then assert(err and err.name=='ENOENT',err and err.message);return nil end
        assert(st.type=='file'and st.uid==0 and(st.mode&0x12)==0 and st.size<=1048576,'unsafe shutdown report')
        local file=assert(io.open(path));local data=file:read(1048577);file:close()
        assert(data and #data<=1048576,'oversized shutdown report');return json.decode(data)
    end
    local previous=read('previous-boot.json');local lua=read('last-shutdown.json');local pid1=read('shutdown-pid1.json')
    local clean,reason=M.assess(previous,lua,pid1)
    print('Shutdown evidence: '..(clean and'CLEAN'or'INCOMPLETE / NOT YET VERIFIED'))
    print(reason)
    print('Previous recorded Lua boot: '..tostring(previous and previous.boot_id))
    if lua then
        print('Lua report: boot='..tostring(lua.boot_id)..' phase='..tostring(lua.phase)..' cleaned='..tostring(lua.cleaned)..' elapsed='..tostring(lua.elapsed))
        for name,failure in pairs(lua.failures or{})do print('Cleanup failure '..name..': '..tostring(failure))end
        for _,s in ipairs(lua.services or{})do if s.forced then print('Forced service '..s.name..': '..s.forced..' (signal '..tostring(s.signal)..')')end end
        print('Recorded service transitions: '..tostring(#(lua.transitions or{}))..'; omitted: '..tostring(lua.dropped_transitions or 0))
    end
    if pid1 then
        print('PID 1 report: boot='..tostring(pid1.boot_id)..' phase='..tostring(pid1.phase)..' wait_result='..tostring(pid1.wait_result)..
            ' exit_code='..tostring(pid1.exit_code)..' signal='..tostring(pid1.signal)..' elapsed='..tostring(pid1.elapsed))
    end
    print('Detailed records: '..directory..'/{previous-boot,last-shutdown,shutdown-pid1}.json')
    print('This verifies supervised cleanup/exit, not the kernel final poweroff or filesystem integrity.')
    return clean and 0 or 1
end
return M
