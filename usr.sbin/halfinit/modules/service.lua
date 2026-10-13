-- SPDX-License-Identifier: BSD-2-Clause
local M={}
local kinds={daemon=true,oneshot=true,target=true,virtual=true}
local policies={never=true,['on-failure']=true,always=true}
local keys={name=true,description=true,type=true,command=true,requires=true,wants=true,after=true,before=true,
    restart=true,restart_delay=true,restart_limit=true,restart_window=true,restart_max_delay=true,
    start_timeout=true,stop_timeout=true,readiness=true,condition=true,action=true,parameters=true,
    env=true,inherit_env=true,cwd=true,user=true,group=true,groups=true,enabled=true,system=true,
    stdout=true,stderr=true}
function M.name(name)
    return type(name)=='string' and #name<=64 and name:match('^[a-zA-Z][a-zA-Z0-9_.-]*$')~=nil
end
local function string(s) return type(s)=='string' and not s:find('\0',1,true) end
local function array(value,validate)
    assert(type(value)=='table','array expected');local n=0
    for k,v in pairs(value) do assert(type(k)=='number' and k%1==0 and k>=1 and k<=#value,'dense array required');assert(validate(v),'invalid array element');n=n+1 end
    assert(n==#value,'array contains holes')
end
local function number(v,min,max)assert(type(v)=='number' and v==v and v>=min and v<=max,'invalid time/resource limit')end
function M.validate(d)
    assert(type(d)=='table','service definition must be a table')
    for k in pairs(d) do assert(keys[k],'unknown service field: '..tostring(k)) end
    assert(M.name(d.name),'invalid service name');assert(kinds[d.type],'unknown service type')
    if d.description==nil then d.description=d.name end;assert(string(d.description) and #d.description<=1024,'invalid description')
    for _,key in ipairs({'requires','wants','after','before'}) do if d[key]==nil then d[key]={}end;array(d[key],M.name) end
    if d.type=='daemon' or (d.type=='oneshot' and not d.action) then
        array(d.command,string);assert(#d.command>0 and #d.command<=4096 and d.command[1]:sub(1,1)=='/','absolute executable required')
    else assert(d.command==nil,'non-process task cannot have command') end
    if d.type~='oneshot' then assert(not d.action,'action requires oneshot') end
    assert(d.action==nil or M.name(d.action),'invalid action provider')
    assert(d.condition==nil or M.name(d.condition),'invalid condition provider')
    if d.type=='virtual' then assert(d.condition,'virtual needs a condition provider') end
    assert(d.parameters==nil or type(d.parameters)=='table','parameters table expected')
    if d.restart==nil then d.restart='never'end;assert(policies[d.restart],'unsupported restart policy')
    assert(d.type=='daemon' or d.restart=='never','restart policy requires daemon')
    for key,default in pairs({restart_delay=1,restart_max_delay=30,restart_window=60,start_timeout=10,stop_timeout=5}) do
        if d[key]==nil then d[key]=default end;number(d[key],0.01,3600)
    end
    assert(d.restart_max_delay>=d.restart_delay,'backoff maximum below initial delay')
    if d.restart_limit==nil then d.restart_limit=5 end;number(d.restart_limit,0,1000);assert(d.restart_limit%1==0,'restart limit must be integer')
    if d.readiness==nil then d.readiness='process'end;assert(d.readiness=='process' or d.readiness=='notify' or d.readiness=='condition','invalid readiness strategy')
    if d.readiness=='condition' then assert(d.condition,'condition readiness needs provider') end
    assert(d.type=='daemon' or d.readiness=='process','readiness option requires daemon')
    for _,k in ipairs({'enabled','system','inherit_env'}) do assert(d[k]==nil or type(d[k])=='boolean','boolean expected: '..k) end
    if d.env~=nil then assert(type(d.env)=='table','env table expected');for k,v in pairs(d.env) do assert(string(k) and k~='' and not k:find('=',1,true) and (string(v) or v==false),'invalid environment') end end
    assert(d.cwd==nil or (string(d.cwd) and d.cwd:sub(1,1)=='/'),'invalid cwd')
    for _,k in ipairs({'user','group'}) do assert(d[k]==nil or string(d[k]) or (type(d[k])=='number' and d[k]>=0 and d[k]%1==0),'invalid credentials') end
    if d.groups~=nil then array(d.groups,function(v)return type(v)=='number' and v>=0 and v%1==0 end) end
    for _,key in ipairs({'stdout','stderr'})do
        local output=d[key]
        assert(output==nil or output=='null' or (string(output) and output:sub(1,1)=='/'),'output must be null or an absolute append-only file path')
    end
    return d
end
M.transitions={INACTIVE={QUEUED=true},QUEUED={STARTING=true,SKIPPED=true,INACTIVE=true,FAILED=true},
    STARTING={READY=true,FAILED=true,STOPPING=true,RESTARTING=true},READY={STOPPING=true,FAILED=true,RESTARTING=true},
    STOPPING={INACTIVE=true,FAILED=true},FAILED={QUEUED=true,RESTARTING=true,INACTIVE=true},
    RESTARTING={QUEUED=true,STOPPING=true,INACTIVE=true,FAILED=true},SKIPPED={QUEUED=true,INACTIVE=true}}
function M.transition(record,state,reason,clock,emit)
    if record.state==state then return end
    if not (M.transitions[record.state] and M.transitions[record.state][state]) then
        error({kind='transition',service=record.definition.name,previous=record.state,state=state,message='illegal service transition'})
    end
    local item={time=clock(),service=record.definition.name,previous=record.state,state=state,reason=reason or ''}
    record.state=state;record.last_transition=item
    record.history[#record.history+1]=item;if #record.history>64 then table.remove(record.history,1) end
    if emit then emit(item) end
end
return M
