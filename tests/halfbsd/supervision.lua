-- SPDX-License-Identifier: BSD-2-Clause
-- Test-private supervision scenarios; no installed services are launched.
local scenario, lua, fixture = assert(arg[1]), assert(arg[2]), assert(arg[3])
local proc = require('proc')
local event = require('halfbsd.event')
local native = require('halfbsd.os')
local logging = require('halfbsd.log')
local loops = {}
local launches = 0
local function check(value, err)
    if value == nil then error(err and (err.operation..': '..err.message) or 'unexpected nil') end
    return value
end
local function loop()
    local l = check(event.new()); loops[#loops+1] = l
    check(l:watch_timer(5, function() error('scenario deadline exceeded') end))
    return l
end
local function spawn(mode, opts)
    opts = opts or {}; opts.new_session = true; opts.stdin = 'null'
    launches = launches + 1
    opts.stdout = opts.stdout or 'pipe'; opts.stderr = {path='fixture-'..launches..'.err'}
    return check(proc.spawn({lua, '-E', fixture, mode}, opts))
end
local function readiness(l, child, callback)
    check(child.stdout:nonblock(true))
    local pending = ''
    check(l:watch_read(child.stdout, function(e)
        while true do
            local data, err = child.stdout:read()
            if not data then
                if err then assert(err.name == 'EAGAIN', err.message) else l:cancel(e.id) end
                break
            end
            pending = pending .. data
            assert(#pending < 4096, 'fixture output exceeded bound')
            if pending:find('READY\n', 1, true) then
                l:cancel(e.id); callback(); break
            end
        end
    end))
end
local function run(l) check(l:run()) end
local cases = {}
function cases.startup()
    local l = loop(); local child = spawn('delayed'); local ready = false
    assert(child:status().state == 'running')
    -- Creation is distinct from application readiness, observed through IO.
    assert(not ready)
    readiness(l, child, function() ready=true; l:stop() end)
    run(l); assert(ready)
    local result = check(child:terminate(1,true)); assert(result.success)
    assert(#proc.active() == 0)
end
function cases.parallel()
    local l=loop(); local children={spawn('polite'),spawn('delayed'),spawn('polite')}
    local count=0
    for _, child in ipairs(children) do
        readiness(l,child,function()count=count+1;if count==#children then l:stop() end end)
    end
    run(l); assert(count==3)
    check(proc.shutdown(1,true))
    for _, child in ipairs(children) do assert(check(child:wait(0)).success) end
    assert(#proc.active()==0)
end
function cases.restart()
    local l=loop();local first=spawn('crash');local replacement;local exits=0
    check(l:watch_process(first,function(e)
        assert(e.result.state=='exited' and e.result.exit_code==17 and not e.result.success)
        exits=exits+1;assert(exits==1, 'unexpected repeated exit notification')
        replacement=spawn('polite')
        readiness(l,replacement,function()l:stop()end)
    end))
    run(l);assert(exits==1 and replacement)
    assert(first:signal('TERM')==false)
    assert(replacement:status().state=='running')
    assert(check(replacement:terminate(1,true)).success)
end
function cases.shutdown()
    local l=loop();local child=spawn('polite');local observed=false
    readiness(l,child,function()check(child:signal('TERM',true))end)
    check(l:watch_process(child,function(e)
        assert(e.result.success);observed=true;l:stop()
    end))
    run(l);assert(observed and check(child:wait(0)).success)
    assert(child:kill()==false)
end
function cases.forced_shutdown()
    local l=loop();local child=spawn('stubborn')
    readiness(l,child,function()l:stop()end);run(l)
    local started=native.clock()
    local result=check(child:terminate(0.05,true))
    assert(result.state=='signaled' and result.signal==9)
    assert(native.clock()-started<7, 'forced shutdown exceeded bound')
end
function cases.timeout()
    local result,err=proc.run({lua,'-E',fixture,'stubborn'}, {
        new_session=true,stdin='null',stdout='pipe',stderr={path='timeout.err'},timeout=0.2,grace=0.05,
    })
    assert(result==nil and err.kind=='timeout')
    assert(#proc.active()==0)
end
function cases.failure()
    local child,err=proc.spawn({'/halfbsd-test-missing-executable'})
    assert(child==nil and err.name=='ENOENT' and err.stage=='exec')
    child,err=proc.spawn({lua,'-E',fixture,'once'},{cwd='/halfbsd-test-missing-directory'})
    assert(child==nil and err.stage=='cwd')
    check(proc.shutdown(0.05));assert(#proc.active()==0)
end
function cases.logging()
    local logger=check(logging.new({console=false,file='supervisor.json',service='test-manager'}))
    check(logger:write('info','starting'))
    local result=check(proc.run({lua,'-E',fixture,'once'}, {
        stdin='null',stdout={path='service.log'},stderr='null',timeout=2,
    }))
    assert(result.success)
    check(logger:write('notice','exited',{exit_code=result.exit_code,pid=result.pid}));logger:close()
    local file=assert(io.open('service.log'));assert(file:read('*a')=='ONE SHOT\n');file:close()
    file=assert(io.open('supervisor.json'));local data=file:read('*a');file:close()
    assert(data:find('"service":"test-manager"',1,true))
    assert(data:find('"exit_code":0',1,true))
end
function cases.repeat_cleanup()
    for _=1,48 do
        local result=check(proc.run({lua,'-E',fixture,'once'}, {
            stdin='null',stdout='pipe',stderr='pipe',timeout=2,
        }))
        assert(result.success and result.stdout=='ONE SHOT\n')
    end
    collectgarbage('collect');assert(#proc.active()==0)
end
local passed, problem=xpcall(function()assert(cases[scenario],scenario)()end,debug.traceback)
-- Cleanup runs on both success and assertion failure, using owned handles only.
for _, l in ipairs(loops) do l:close() end
local clean,err=proc.shutdown(0.05,true)
if not clean then passed=false;problem=(problem or '')..'\ncleanup: '..err.message end
if not passed then io.stderr:write(tostring(problem)..'\n');os.exit(1,true) end
print('PASS '..scenario)
