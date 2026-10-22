-- SPDX-License-Identifier: BSD-2-Clause
local collect=assert(loadfile(assert(arg[1])))()
local function fixture()
    local f={files={},commands={},now=0,closed=0,monitor_closed=0,uid=0,exists=false}
    local native={identity=function()return {euid=f.uid}end,exists=function()return f.exists end,
        mkdir=function(_,mode)assert(mode==448);return true end,clock=function()return f.now end,
        getenv=function()return nil end}
    function native.atomic_replace(path,data,mode)assert(mode==384);f.files[path]=data;return true end
    function native.stat(path)
        return {type=path:match('%.sock$')and'socket'or'directory'}
    end
    function native.listdir(path)
        if path=='/var/run/user'then return {'1001'}end
        if path=='/var/run/user/1001'then return {'sway-ipc.1001.99.sock'}end
        return {}
    end
    local proc={run=function(argv,opts)
        assert(opts.timeout==2 and opts.output_limit==262144 and opts.stdout=='pipe')
        assert(argv[1]:sub(1,1)=='/'and argv[1]~='/bin/sh'and not argv[1]:find('kldload',1,true))
        if argv[1]:match('/swaymsg$')then assert(argv[4]=='-t'and argv[5]=='get_inputs')end
        f.commands[#f.commands+1]=argv
        return {exit_code=0,stdout='fixture inventory\n'}
    end}
    local serial,queue=0,{}
    local loop={}
    function loop:watch_read(fd,cb)serial=serial+1;queue[serial]={at=0,callback=cb};return serial end
    function loop:watch_timer(delay,cb,repeated)serial=serial+1;queue[serial]={at=f.now+delay,callback=cb,repeated=repeated and delay};return serial end
    function loop:cancel(id)queue[id]=nil end
    function loop:stop()self.stopped=true end
    function loop:close()f.closed=f.closed+1 end
    function loop:run()
        for _=1,100 do
            if self.stopped then return true end
            local id,time
            for key,value in pairs(queue)do if not time or value.at<time then id,time=key,value.at end end
            assert(id,'event loop stalled');local e=queue[id];queue[id]=nil;f.now=e.at;e.callback()
            if e.repeated then queue[id]={at=f.now+e.repeated,callback=e.callback,repeated=e.repeated}end
        end
        error('event budget exceeded')
    end
    setmetatable(loop,{__close=function(self)self:close()end})
    local delivered=false
    local fd={read=function()
        if not delivered then delivered=true;return '!system=DEVFS subsystem=CDEV type=CREATE cdev=input/event7\n'end
        return nil,{name='EAGAIN'}
    end,close=function()f.monitor_closed=f.monitor_closed+1 end}
    local ipc={connect=function(path,timeout)assert(path=='/var/run/devd.pipe'and timeout==1);return fd end}
    return {native=native,proc=proc,event={new=function()return loop end},ipc=ipc},f
end
local deps,f=fixture()
assert(collect({'/private/capture','5'},deps)==0)
assert(f.closed==1 and f.monitor_closed==1)
assert(f.files['/private/capture/devd-events.txt']:find('input/event7',1,true))
assert(f.files['/private/capture/snapshot-00.txt']and f.files['/private/capture/snapshot-01.txt'])
assert(f.files['/private/capture/README.txt']:find('completed',1,true))
local sway=false;for _,argv in ipairs(f.commands)do if argv[1]:match('/swaymsg$')then sway=true end end;assert(sway)
deps,f=fixture();deps.ipc.connect=function()return nil,{name='ENOENT'}end
assert(collect({'/private/capture','5'},deps)==0)
assert(f.files['/private/capture/devd-events.txt']:find('MONITOR UNAVAILABLE',1,true))
for _,args in ipairs({{'relative'},{'/private/capture','0'},{'/private/../capture','5'},{'/private/capture','5','relative-socket'}})do
    deps,f=fixture();assert(not pcall(collect,args,deps));assert(next(f.files)==nil)
end
deps,f=fixture();f.uid=1001;assert(not pcall(collect,{'/private/capture','5'},deps))
deps,f=fixture();f.exists=true;assert(not pcall(collect,{'/private/capture','5'},deps))
print('PASS passive input collector bounds, snapshots, IPC cleanup, missing monitor and argument safety (mock native APIs)')
