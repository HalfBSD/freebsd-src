-- SPDX-License-Identifier: BSD-2-Clause
package.path=assert(arg[1])..'/?.lua'
local reporter=require('shutdown_report');local json=require('json')
local files,rows,warnings={},{},{}
for _,p in ipairs({'/var','/var/db'})do rows[p]={type='directory',uid=0,mode=493,device=1,inode=1}end
local clock,pid,readonly=10,42,false
local native={identity=function()return {pid=pid}end,clock=function(kind)return kind=='realtime'and(100000+clock)or clock end}
function native.stat(path)
    if rows[path]then return rows[path]end
    if files[path]then return {type='file',uid=0,mode=384,size=#files[path]}end
    return nil,{name='ENOENT'}
end
function native.mkdir(path,mode)rows[path]={type='directory',uid=0,mode=mode,device=1,inode=2};return true end
function native.atomic_replace(path,data,mode)
    assert(mode==384)
    if readonly then return nil,{name='EROFS',message='read-only storage'}end
    files[path]=data;return true
end
local original=io.open
io.open=function(path)
    if files[path]then return {read=function(_,n)return files[path]:sub(1,n)end,close=function()end}end
    return nil,'not present'
end
local function warning(message)warnings[#warnings+1]=message end
local function read(name)return json.decode(assert(files['/var/db/halfbsd/'..name]))end
local ok,err=pcall(function()
    local r=reporter.new(native,nil,warning);assert(r.available and #warnings==0)
    local boot=read('boot.json');assert(files['/var/db/halfbsd/boot-id']==boot.boot_id..'\n')
    local engine={records={dns={state='READY',child={pid=55}}},shutdown_failed={}}
    r:begin(engine);assert(read('last-shutdown.json').phase=='stopping')
    for i=1,300 do r:observe({service='dns',previous='READY',state='STOPPING',reason='manager shutdown',time=clock})end
    engine.records.dns={state='INACTIVE',result={pid=55,signal=9},shutdown_forced='service stop timeout'}
    clock=12;assert(r:checkpoint(engine,'filesystem_cleanup_pending',false))
    assert(r:checkpoint(engine,'finished',true,{children_released=true}))
    local finished=read('last-shutdown.json')
    assert(finished.cleaned and finished.elapsed==2 and finished.boot_id==boot.boot_id)
    assert(#finished.transitions==256 and finished.dropped_transitions==44)
    assert(finished.services[1].signal==9 and finished.services[1].forced=='service stop timeout')
    local parent={boot_id=finished.boot_id,phase='finished',wait_result=1,exited=true,exit_code=0,signal=0}
    assert(reporter.assess(boot,finished,parent))
    parent.signal=9;assert(not reporter.assess(boot,finished,parent));parent.signal=0
    assert(not reporter.assess({boot_id='other'},finished,parent))
    assert(not reporter.assess(boot,finished,nil))
    assert(not reporter.assess(nil,finished,parent))
    clock=20;pid=43;local next_boot=reporter.new(native,nil,warning)
    assert(next_boot.available and read('previous-boot.json').boot_id==finished.boot_id)
    assert(read('last-shutdown.json').boot_id~=read('boot.json').boot_id)
    files['/var/db/halfbsd/shutdown-pid1.json']=json.encode(parent)
    local loaded=package.loaded['halfbsd.os'];package.loaded['halfbsd.os']=native
    local output={};local saved_print=print
    print=function(line)output[#output+1]=line end
    local shown,result=pcall(require('client').main,{'shutdown-report'})
    print=saved_print;package.loaded['halfbsd.os']=loaded
    assert(shown and result==0 and output[1]=='Shutdown evidence: CLEAN')
    -- No shutdown in that boot: a third boot must not attribute old success to it.
    clock=30;pid=44;reporter.new(native,nil,warning)
    assert(read('previous-boot.json').boot_id~=read('last-shutdown.json').boot_id)
    next_boot:begin(engine);local before=files['/var/db/halfbsd/last-shutdown.json']
    rows['/var/db/halfbsd'].inode=3
    assert(not next_boot:checkpoint(engine,'finished',true))
    assert(files['/var/db/halfbsd/last-shutdown.json']==before and #warnings>0)
    rows['/var/db/halfbsd'].inode=2
    readonly=true;local failed=reporter.new(native,nil,warning);assert(not failed.available)
    assert(not failed:checkpoint(engine,'finished',true));readonly=false
    rows['/var/db/halfbsd'].mode=511
    local unsafe=reporter.new(native,nil,warning);assert(not unsafe.available)
end)
io.open=original;assert(ok,err)
print('PASS durable shutdown snapshots, bounded traces, escalation, prior-boot correlation and storage failures')
