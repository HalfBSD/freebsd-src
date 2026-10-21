-- SPDX-License-Identifier: BSD-2-Clause
local json=require('json')
local service=require('service')
local M={};M.__index=M
local readonly={list=true,status=true,graph=true}
function M.authorized(uid,owner,command)return readonly[command] or uid==0 or uid==owner end
function M.request(request,uid,owner,engine,handlers)
    assert(type(request)=='table' and request.version==1,'unsupported protocol version')
    for k in pairs(request)do assert(k=='version' or k=='command' or k=='service','unknown request field')end
    local command=request.command;assert(type(command)=='string','command required')
    assert(readonly[command]or command=='start'or command=='stop'or command=='restart'or command=='enable'or command=='disable'or command=='reload'or command=='shutdown','unknown command')
    if command=='list'or command=='graph'or command=='reload'or command=='shutdown'then assert(request.service==nil,'command does not take a service')end
    assert(M.authorized(uid,owner,command),'permission denied')
    if command=='status' and request.service~=nil then assert(service.name(request.service),'invalid service name') end
    if command=='list' or command=='status' then
        local result=engine:status()
        if request.service then for _,r in ipairs(result)do if r.name==request.service then return r end end;error('unknown service')end
        return result
    end
    if command=='graph' then return engine.plan end
    assert(not(engine.config and engine.config.checkpoint),'diagnostic profile control is read-only; use system reboot for cleanup')
    assert(not engine.shutting or command=='shutdown','manager is shutting down')
    if command=='reload' then return handlers.reload() end
    if command=='shutdown' then engine:shutdown();return {accepted=true} end
    assert(service.name(request.service)and engine.records[request.service],'unknown/invalid service name')
    local name=request.service
    if command=='start' then engine:start({name})
    elseif command=='stop' then engine:stop(name)
    elseif command=='restart' then
        engine:restart(name)
    elseif command=='enable' or command=='disable' then handlers.persist(name,command=='enable') end
    return {accepted=true,service=name,command=command}
end
function M.new(loop,socket,owner,engine,handlers)
    local ipc=require('halfbsd.ipc');local native=require('halfbsd.os')
    local self=setmetatable({loop=loop,path=socket,clients={},ipc=ipc,owner=owner,engine=engine,handlers=handlers,closed=false},M)
    local old,err=native.stat(socket)
    if old then assert(old.type=='socket'and old.uid==owner,'refuse to replace foreign/non-socket path');assert(native.unlink(socket))
    else assert(err.name=='ENOENT','cannot inspect socket')end
    local ok,problem=pcall(function()
        self.listener=assert(ipc.listen(socket))
        assert(native.chmod(socket,0x1b6)) -- 0666; peer UID controls writes.
        self.accept_watch=assert(loop:watch_read(self.listener,function()self:accept()end))
    end)
    if not ok then
        if self.listener then self.listener:close();native.unlink(socket)end
        error(problem)
    end
    return self
end
function M:drop(c)
    if c.closed then return end;c.closed=true
    for _,id in ipairs(c.watches)do self.loop:cancel(id)end
    c.fd:close();self.clients[c]=nil
end
function M:accept()
    local count=0;for _ in pairs(self.clients)do count=count+1 end
    -- Bounded acceptance work per dispatch, even under continuous connections.
    for _=1,32 do
        local fd,peer=self.ipc.accept(self.listener)
        if not fd then
            if peer.name=='EAGAIN'then return end
            if peer.name=='EINTR'or peer.name=='ECONNABORTED'then self:accept_later();return end
            error(peer.message)
        end
        if count>=32 then fd:close()
        else
            local c={fd=fd,uid=peer.uid,buffer='',watches={}};self.clients[c]=true;count=count+1
            c.watches[#c.watches+1]=assert(self.loop:watch_timer(3,function()self:drop(c)end))
            c.watches[#c.watches+1]=assert(self.loop:watch_read(fd,function()self:read(c)end))
        end
    end
    self:accept_later()
end
function M:accept_later()
    if not self.accept_timer then self.accept_timer=assert(self.loop:watch_timer(0.001,function()self.accept_timer=nil;self:accept()end))end
end
function M:respond(c,response)
    if c.closed then return end
    local ok,encoded=pcall(json.encode,response)
    if not ok or #encoded>1048575 then encoded=json.encode({version=1,ok=false,error={kind='resource',message='response too large'}})end
    c.pending=encoded..'\n';c.offset=1
    -- Only a single request is accepted on each connection.
    for _,id in ipairs(c.watches)do self.loop:cancel(id)end;c.watches={}
    c.watches[#c.watches+1]=assert(self.loop:watch_timer(3,function()self:drop(c)end))
    c.watches[#c.watches+1]=assert(self.loop:watch_write(c.fd,function()self:write(c)end))
    self:write(c)
end
function M:write(c)
    if c.closed then return end
    while c.offset<=#c.pending do
        local data=c.pending:sub(c.offset,math.min(#c.pending,c.offset+65535))
        local n,err=c.fd:write(data)
        if not n then if err.name=='EAGAIN'then return end;self:drop(c);return end
        if n==0 then self:drop(c);return end
        c.offset=c.offset+n
    end
    if c.offset>#c.pending then self:drop(c)end
end
function M:read(c)
    if c.closed or c.pending then return end
    while true do
        local data,err=c.fd:read(4096)
        if not data then if not err or err.name~='EAGAIN'then self:drop(c)end;return end
        c.buffer=c.buffer..data
        if #c.buffer>8192 then self:respond(c,{version=1,ok=false,error={kind='protocol',message='request too large'}});return end
        local line=c.buffer:match('^(.-)\n')
        if line then
            local ok,result=pcall(function()
                assert(#c.buffer==#line+1,'multiple requests/trailing bytes')
                return M.request(json.decode(line),c.uid,self.owner,self.engine,self.handlers)
            end)
            self:respond(c,{version=1,ok=ok,data=ok and result or nil,
                error=not ok and {kind='request',message=type(result)=='table'and(result.message or 'operation failed')or tostring(result)}or nil})
            return
        end
    end
end
function M:close()
    if self.closed then return end;self.closed=true
    if self.accept_watch then self.loop:cancel(self.accept_watch)end
    if self.accept_timer then self.loop:cancel(self.accept_timer)end
    for c in pairs(self.clients)do self:drop(c)end
    if self.listener then self.listener:close()end
    require('halfbsd.os').unlink(self.path)
end
return M
