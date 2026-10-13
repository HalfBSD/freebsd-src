-- SPDX-License-Identifier: BSD-2-Clause
package.path=assert(arg[1])..'/?.lua'
local helper=assert(arg[2]);local native=require('halfbsd.os');local ipc=require('halfbsd.ipc')
local proc=require('proc');local event=require('halfbsd.event');local config=require('config')
local providers=require('providers').native()
local function engine(definitions,override)
    return require('supervisor').new({registry=config.registry(definitions),max_parallel=128,shutdown_timeout=2},
        {providers=override or providers})
end
local function ready(e,name)
    local timer=assert(e.loop:watch_timer(5,function()error('native adapter test timeout')end))
    local function check()
        local r=e.records[name]
        if r.state=='READY'then e.loop:stop()
        elseif r.state=='FAILED'then error(r.last_transition.reason)
        else assert(e.loop:watch_timer(0.01,check))end
    end
    check();assert(e.loop:run());e.loop:cancel(timer)
end
local mounts=assert(native.mounts());local root=false
for _,row in ipairs(mounts)do if row.path=='/'then root=true end end;assert(root)
local processes=assert(native.processes());local self=false
for _,row in ipairs(processes)do if row.pid==native.identity().pid then self=true end end;assert(self)
assert(type(native.fstab())=='table'and type(native.default_routes())=='table')
assert(type(native.interfaces())=='table'and type(native.terminal().tty)=='boolean')
print('PASS native read-only mount, fstab, process, route, interface and terminal inventories')
local e=engine({{name='check',type='oneshot',command={'/bin/echo','verified'},verify='exact_output',parameters={expected='verified'}}})
e:start({'check'});ready(e,'check');assert(e.records.check.result.success)
e:shutdown();if not e.finished then assert(e.loop:run())end;e.loop:close();e:cleanup_actions()
print('PASS actual oneshot pipe capture, process reaping and output verification')
local failed=engine({{name='check',type='oneshot',command={'/bin/echo','wrong'},verify='exact_output',parameters={expected='verified'}}})
failed:start({'check'});assert(failed.loop:watch_timer(5,function()error('invalid output did not fail')end))
local function failure_check()
    if failed.records.check.state=='FAILED'then failed.loop:stop()else assert(failed.loop:watch_timer(0.01,failure_check))end
end
failure_check();assert(failed.loop:run())
assert(failed.records.check.state=='FAILED');failed:shutdown();if not failed.finished then assert(failed.loop:run())end;failed.loop:close()
print('PASS actual invalid command readback fails closed')
local child=assert(proc.spawn({helper},{stdout='pipe',new_session=true}));local fd=child.stdout;assert(fd:nonblock(true))
local loop=assert(event.new());local text='';local port
assert(loop:watch_read(fd,function()
    local bytes,err=fd:read(128);if bytes then text=text..bytes;port=tonumber(text:match('^(%d+)\n'));if port then loop:stop()end
    elseif not err or err.name~='EAGAIN'then error('probe fixture exited')end
end))
assert(loop:watch_timer(3,function()error('probe fixture did not publish port')end));assert(loop:run());loop:close();fd:close()
assert(port and port>1024);local probe={};local poll=assert(event.new())
local function check()
    if providers.dns_ready({port=port},probe)then poll:stop()
    else assert(poll:watch_timer(0.01,check))end
end
local ok,why=xpcall(function()
    assert(poll:watch_timer(3,function()error('UDP/TCP readiness timeout')end));check();assert(poll:run())
    assert(ipc.tcp_probe('127.0.0.1',port))
end,debug.traceback)
providers.release(probe);poll:close();assert(child:signal('TERM'));assert(child:wait(3));assert(ok,why)
assert(#proc.active()==0);assert(proc.shutdown(0.1,true))
print('PASS native UDP query and TCP readiness against ephemeral loopback fixture; all children released')
