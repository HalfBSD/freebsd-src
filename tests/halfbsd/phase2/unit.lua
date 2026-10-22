-- SPDX-License-Identifier: BSD-2-Clause
local directory=assert(arg[1]);package.path=directory..'/?.lua'
local config=require('config');local dependency=require('dependency');local service=require('service');local json=require('json')
local control=require('control')
local count=0
local function test(name,fn)fn();count=count+1;print('PASS '..name)end
local function rejects(fn)assert(not pcall(fn),'invalid input accepted')end
local function definitions(n)
    local d={};for i=1,n do d[i]={name='s'..i,type='oneshot',command={'/fixture','oneshot',tostring((i%7+1)/100)}}end
    return d
end
local function fake()
    local f={time=0,serial=0,events={},children={}}
    local loop={}
    function loop:watch_timer(delay,cb)f.serial=f.serial+1;f.events[f.serial]={at=f.time+delay,callback=cb};return f.serial end
    function loop:cancel(id)f.events[id]=nil;return true end
    function loop:stop()self.stopped=true end
    function loop:watch_process(child,cb)f.serial=f.serial+1;child.callback=cb;child.watch=f.serial;return f.serial end
    function loop:step()
        local nextid,when
        for id,e in pairs(f.events)do if not when or e.at<when or(e.at==when and id<nextid)then nextid,when=id,e.at end end
        assert(nextid,'no event can advance test');local e=f.events[nextid];f.events[nextid]=nil;f.time=when;e.callback({id=nextid});return true
    end
    function loop:run()self.stopped=false;local budget=20000;while not self.stopped do budget=budget-1;assert(budget>0,'test event budget exceeded');self:step()end;return true end
    function loop:close()f.events={}end
    local proc={}
    function proc.spawn(argv)
        f.serial=f.serial+1;local child={pid=f.serial,state='running',own_group=true};f.children[child]=true
        local function finish(code,signal)
            if child.state~='running'then return end
            child.state=signal and'signaled'or'exited';child.result={pid=child.pid,state=child.state,exit_code=code,signal=signal,success=not signal and code==0}
            f.children[child]=nil
            loop:watch_timer(0.0001,function()if child.callback then child.callback({result=child.result})end end)
        end
        function child:signal(sig)
            if self.state~='running'then return false end
            if sig=='TERM'and argv[2]=='stubborn'then return true end
            finish(nil,sig=='KILL'and 9 or 15);return true
        end
        function child:status()return self.result or{state='running',pid=self.pid}end
        if argv[2]=='oneshot'or argv[2]=='exit_zero'or argv[2]=='crash' or argv[2]=='fail'then loop:watch_timer(tonumber(argv[3])or 0.01,function()finish((argv[2]=='oneshot'or argv[2]=='exit_zero')and 0 or 17)end)end
        return child
    end
    function proc.active()local result={};for c in pairs(f.children)do result[#result+1]=c end;return result end
    return {loop=loop,proc=proc,native={clock=function()return f.time end}},f
end
local function engine(defs)
    local options,f=fake();local cfg={registry=config.registry(defs),max_parallel=128,shutdown_timeout=1}
    return require('supervisor').new(cfg,options),f
end
local function advance(e,predicate)
    for _=1,20000 do if predicate()then return end;e.loop:step()end
    error('test failed to converge')
end
test('100 services register without process creation',function()
    local r=config.registry(definitions(100));local n=0;for _ in pairs(r)do n=n+1 end;assert(n==100)
end)
test('schema, references and all cycle types reject',function()
    rejects(function()config.registry({{name='x',type='target'},{name='x',type='target'}})end)
    rejects(function()config.registry({{name='x',type='bogus'}})end)
    rejects(function()config.registry({{name='x',type='target',requires={'missing'}}})end)
    for _,kind in ipairs({'requires','after','wants','before'})do
        rejects(function()config.registry({{name='a',type='target',[kind]={'b'}},{name='b',type='target',[kind]={'a'}}})end)
    end
    rejects(function()config.registry({{name='x',type='daemon',command={'relative'},restart='forever'}})end)
    rejects(function()config.registry({{name='x',type='oneshot',command={'/x',[3]='hole'}}})end)
    for _,field in ipairs({'requires','restart','readiness','env','cwd','parameters','groups','description'})do
        rejects(function()config.registry({{name='x',type='daemon',command={'/x'},[field]=false}})end)
    end
end)
test('ordering never activates an otherwise absent service',function()
    local r=config.registry({{name='a',type='target',after={'b'}},{name='b',type='target'}})
    local p=dependency.plan(r,{a=true});assert(p.selected.a and not p.selected.b)
end)
test('100 randomized tasks obey dependencies and overlap',function()
    math.randomseed(42);local defs=definitions(100);local roots={};local wanted={}
    for i,d in ipairs(defs)do
        wanted[#wanted+1]=d.name
        if i>3 then d.requires={'s'..math.random(1,i-1)}end
    end
    defs[#defs+1]={name='boot',type='target',requires=wanted}
    local e=engine(defs);e:start({'boot'});local concurrent=0
    for _,r in pairs(e.records)do if r.state=='STARTING'then concurrent=concurrent+1 end end;assert(concurrent>1)
    advance(e,function()return e.records.boot.state=='READY'end)
    for _,r in pairs(e.records)do
        assert(r.state=='READY')
        for _,name in ipairs(r.definition.requires)do
            local parent=e.records[name];local started
            for _,h in ipairs(r.history)do if h.state=='STARTING'then started=h.time end end
            assert(started>=parent.last_transition.time)
        end
    end
    e:shutdown();if not e.finished then e.loop:run()end;assert(#e.proc.active()==0)
end)
test('restart exhaustion and failed hard dependencies are bounded',function()
    local e=engine({{name='bad',type='daemon',command={'/fixture','crash','0.01'},restart='on-failure',restart_limit=2,restart_delay=0.01,restart_max_delay=0.02},
        {name='dependent',type='target',requires={'bad'}}})
    e:start({'dependent'});advance(e,function()return e.records.bad.exhausted end)
    assert(e.records.bad.state=='FAILED'and #e.records.bad.attempts==2)
    assert(e.records.dependent.state~='READY');e:shutdown();if not e.finished then e.loop:run()end;assert(#e.proc.active()==0)
end)
test('never, on-failure and always distinguish successful exits',function()
    for _,case in ipairs({{'never','crash',false},{'on-failure','exit_zero',false},{'always','exit_zero',true}})do
        local e=engine({{name='worker',type='daemon',command={'/fixture',case[2],'0.01'},
            restart=case[1],restart_limit=1,restart_delay=0.01}})
        e:start({'worker'});advance(e,function()return e.records.worker.state=='FAILED'end)
        assert((e.records.worker.exhausted or false)==case[3])
        assert(#e.records.worker.attempts==(case[3]and 1 or 0))
    end
end)
test('DHCP-style clean exit restarts remain bounded and operator stop cancels recovery',function()
    local e=engine({{name='dhcp-test',type='daemon',command={'/fixture','exit_zero','0.01'},
        restart='always',restart_limit=2,restart_delay=0.01}})
    e:start({'dhcp-test'})
    advance(e,function()return e.records['dhcp-test'].state=='RESTARTING'end)
    assert(e.records['dhcp-test'].result.success and #e.records['dhcp-test'].attempts==1)
    e:stop('dhcp-test')
    assert(e.records['dhcp-test'].state=='INACTIVE'and e.holds['dhcp-test'])
    local elapsed=false;e.loop:watch_timer(1,function()elapsed=true end)
    advance(e,function()return elapsed end)
    assert(#e.proc.active()==0 and e.records['dhcp-test'].state=='INACTIVE')
    e:shutdown();if not e.finished then e.loop:run()end;assert(e.finished)
end)
test('50 supervised children and reverse shutdown',function()
    local defs={};for i=1,50 do defs[#defs+1]={name='d'..i,type='daemon',command={'/fixture','daemon'}}end
    local requires={};for i=1,50 do requires[i]='d'..i end
    defs[#defs+1]={name='boot',type='target',requires=requires}
    local e=engine(defs);e:start({'boot'});assert(e.records.boot.state=='READY'and #e.proc.active()==50)
    e:shutdown();if not e.finished then e.loop:run()end;assert(e.finished and #e.proc.active()==0)
    assert(e.records.boot.history[#e.records.boot.history].state=='INACTIVE')
end)
test('1000 start-stop operations retain bounded histories',function()
    local e=engine({{name='worker',type='daemon',command={'/fixture','daemon'}}})
    for _=1,1000 do
        e:start({'worker'});assert(e.records.worker.state=='READY');e:stop('worker')
        advance(e,function()return e.records.worker.state=='INACTIVE'end)
        assert(#e.proc.active()==0)
    end
    assert(#e.records.worker.history<=64)
end)
test('shutdown during startup and forced deadlines release children',function()
    local e=engine({{name='slow',type='oneshot',command={'/fixture','oneshot','10'}},
        {name='stubborn',type='daemon',command={'/fixture','stubborn'},stop_timeout=0.01}})
    e:start({'slow','stubborn'});assert(e.records.slow.state=='STARTING')
    e:shutdown();if not e.finished then e.loop:run()end
    assert(#e.proc.active()==0 and e.finished)
    assert(e.records.stubborn.result.signal==9)
end)
test('creation resource failures do not start hard dependents',function()
    local options=fake()
    options.proc.spawn=function()return nil,{operation='spawn',kind='resource',name='EMFILE',code=24,message='resource exhausted'}end
    local cfg={registry=config.registry({{name='worker',type='daemon',command={'/fixture'}},
        {name='dependent',type='target',requires={'worker'}}}),max_parallel=128,shutdown_timeout=1}
    local e=require('supervisor').new(cfg,options);e:start({'dependent'})
    assert(e.records.worker.state=='FAILED'and e.records.dependent.state=='SKIPPED')
    assert(e:status()[2].error.name=='EMFILE')
    assert(#e.proc.active()==0)
end)
test('native action and virtual readiness cancel their startup deadlines',function()
    local options,f=fake();options.providers={ready=function()return true end}
    local cfg={registry=config.registry({{name='condition',type='virtual',condition='ready',start_timeout=0.02},
        {name='action',type='oneshot',action='ready',start_timeout=0.02}}),max_parallel=128,shutdown_timeout=1}
    local e=require('supervisor').new(cfg,options);e:start({'condition','action'})
    options.loop:watch_timer(0.1,function()options.loop:stop()end);options.loop:run()
    assert(f.time==0.1 and e.records.condition.state=='READY'and e.records.action.state=='READY')
    assert(#e.proc.active()==0)
end)
test('shutdown reporting is once-only and observer errors never prevent child cleanup',function()
    local e=engine({{name='worker',type='daemon',command={'/fixture','stubborn'},stop_timeout=0.01}})
    local calls=0;e.on_shutdown=function()calls=calls+1;error('diagnostic storage failed')end
    e:start({'worker'});e:shutdown();e:shutdown()
    if not e.finished then e.loop:run()end
    assert(calls==1 and e.shutdown_report_error:find('diagnostic storage failed',1,true))
    assert(#e.proc.active()==0 and e.records.worker.shutdown_forced=='service stop timeout')
end)
test('control authorization and protocol boundaries',function()
    local e=engine({{name='worker',type='target'}});e:replan()
    assert(control.authorized(42,1001,'status'))
    assert(not control.authorized(42,1001,'start'));assert(control.authorized(1001,1001,'start'))
    rejects(function()control.request({version=1,command='start',service='worker'},42,1001,e,{})end)
    rejects(function()control.request({version=1,command='eval',service='worker'},1001,1001,e,{})end)
    rejects(function()control.request({version=1,command='start',service='; rm -rf /'},1001,1001,e,{})end)
    rejects(function()control.request({version=99,command='list'},1001,1001,e,{})end)
end)
test('JSON protocol rejects ambiguous and pathological requests',function()
    local text=json.encode({version=1,command='status',test='a\n"b'});assert(json.decode(text).test=='a\n"b')
    for _,bad in ipairs({'{"x":1,"x":2}','01','1.','1.e1','1.E1','[1,]','{"x":NaN}','"\\uD800"','{} trailing'})do rejects(function()json.decode(bad)end)end
end)
test('illegal lifecycle transitions are structured errors',function()
    local r={definition={name='x'},state='INACTIVE',history={}}
    local ok,err=pcall(service.transition,r,'READY','bad',function()return 0 end)
    assert(not ok and err.kind=='transition')
end)
print('PASS pure policy tests: '..count..' (no native process/kernel validation)')
