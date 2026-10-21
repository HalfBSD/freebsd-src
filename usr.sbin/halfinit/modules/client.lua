-- SPDX-License-Identifier: BSD-2-Clause
local json=require('json')
local M={}
function M.exchange(socket,request,timeout)
    local ipc=require('halfbsd.ipc');local event=require('halfbsd.event')
    local fd,err=ipc.connect(socket,timeout or 3)
    if not fd then return nil,err end
    local connection <close> = fd
    local loop <close> = assert(event.new())
    local input=json.encode(request)..'\n';assert(#input<=8192,'request too large')
    local offset,output,response,failure=1,'',nil,nil
    assert(loop:watch_timer(timeout or 3,function()failure={kind='timeout',message='control deadline'};loop:stop()end))
    local writer
    writer=assert(loop:watch_write(fd,function()
        while offset<=#input do
            local n,error=fd:write(input:sub(offset))
            if not n then if error.name=='EAGAIN'then return end;failure=error;loop:stop();return end
            if n==0 then failure={kind='io',message='empty write'};loop:stop();return end
            offset=offset+n
        end
        loop:cancel(writer)
    end))
    assert(loop:watch_read(fd,function()
        while true do
            local bytes,error=fd:read(65536)
            if not bytes then
                if error and error.name=='EAGAIN'then return end
                failure=error or {kind='io',message='control connection closed without response'};loop:stop();return
            end
            output=output..bytes
            if #output>1048576 then failure={kind='protocol',message='response too large'};loop:stop();return end
            local line=output:match('^(.-)\n')
            if line then
                local ok,value=pcall(json.decode,line)
                if not ok or type(value)~='table' or value.version~=1 or type(value.ok)~='boolean' then
                    failure={kind='protocol',message='invalid control response'}
                elseif not value.ok then
                    failure=type(value.error)=='table'and type(value.error.message)=='string'and value.error or{kind='protocol',message='invalid error response'}
                elseif value.data==nil then failure={kind='protocol',message='missing response data'}
                else response=value.data end
                loop:stop();return
            end
        end
    end))
    local ok,error=loop:run();fd:close()
    if not ok then return nil,error end
    return response,failure
end
function M.main(args)
    if args[1]=='shutdown-report'then return require('shutdown_report').show(args)end
    if args[1]=='--help'then io.write('halfsvc [--socket PATH] list|status [SERVICE]|graph|start|stop|restart|enable|disable SERVICE|reload|shutdown\nhalfsvc shutdown-report (root, reads persistent evidence without manager IPC)\n');return 0 end
    local socket='/var/run/halfbsd/control.sock';local i=1
    if args[i]=='--socket'then socket=assert(args[i+1]);i=i+2 end
    local command=args[i]or'status';local name=args[i+1];assert(args[i+2]==nil,'too many arguments')
    local value,err=M.exchange(socket,{version=1,command=command,service=name})
    if not value then io.stderr:write((err.message or 'control failure')..'\n');return 1 end
    io.stdout:write(json.encode(value)..'\n');return 0
end
return M
