-- SPDX-License-Identifier: BSD-2-Clause
local service=require('service')
local dependency=require('dependency')
local M={}
local function paths(value)
    if value==nil then return {} end
    assert(type(value)=='table','path array expected');local count=0
    for key,path in pairs(value)do
        assert(type(key)=='number'and key%1==0 and key>=1 and key<=#value,'dense path array expected')
        assert(type(path)=='string'and path:sub(1,1)=='/'and not path:find('\0',1,true),'absolute path expected')
        count=count+1
    end
    assert(count==#value and count<=1024,'invalid path array size');return value
end
-- Definitions/configuration have no OS, IO, require, debug or process functions.
-- Declarative loading cannot accidentally start services.
function M.read(path)
    local env={assert=assert,error=error,pairs=pairs,ipairs=ipairs,tonumber=tonumber,tostring=tostring,
        math=math,string=string,table=table}
    local chunk,err=loadfile(path,'t',env);assert(chunk,err)
    local value=chunk();assert(type(value)=='table','configuration must return a table');return value
end
function M.registry(definitions)
    local result={};assert(type(definitions)=='table','services array expected')
    local size=0;for key in pairs(definitions)do assert(type(key)=='number'and key%1==0 and key>=1 and key<=#definitions,'dense service array expected');size=size+1 end
    assert(size==#definitions,'service array contains holes')
    for _,d in ipairs(definitions)do
        d=service.validate(d);assert(not result[d.name],'duplicate service: '..d.name);result[d.name]=d
    end
    assert(#definitions<=1024,'service registry exceeds 1024 entries');dependency.validate(result);return result
end
function M.load(path,system_path)
    local config=path and M.read(path) or {services=M.read(system_path),target='minimal'}
    local allowed={services=true,target=true,include=true,service_dirs=true,runtime_dir=true,state_file=true,shutdown_timeout=true,
        max_parallel=true,log_file=true}
    for k in pairs(config)do assert(allowed[k],'unknown configuration field: '..tostring(k))end
    if config.services==nil then config.services={}end
    assert(type(config.services)=='table','services array expected')
    for _,file in ipairs(paths(config.include))do
        local definition=M.read(file)
        if definition.name then config.services[#config.services+1]=definition
        else
            -- Validate density before merging: ipairs would silently drop holes.
            local count=0
            for key in pairs(definition)do
                assert(type(key)=='number'and key%1==0 and key>=1 and key<=#definition,'dense included service array expected')
                count=count+1
            end
            assert(count==#definition,'included service array contains holes')
            for _,d in ipairs(definition)do config.services[#config.services+1]=d end
        end
    end
    for _,dir in ipairs(paths(config.service_dirs))do
        local names=assert(require('halfbsd.os').listdir(dir));table.sort(names)
        for _,name in ipairs(names)do if name:match('%.lua$')then config.services[#config.services+1]=M.read(dir..'/'..name)end end
    end
    config.registry=M.registry(config.services)
    if config.target==nil then config.target='minimal'end;assert(service.name(config.target)and config.registry[config.target],'invalid target')
    if config.max_parallel==nil then config.max_parallel=128 end;assert(type(config.max_parallel)=='number' and config.max_parallel%1==0 and config.max_parallel>=1 and config.max_parallel<=256,'max_parallel must be 1..256')
    if config.shutdown_timeout==nil then config.shutdown_timeout=15 end;assert(type(config.shutdown_timeout)=='number'and config.shutdown_timeout>0 and config.shutdown_timeout<=300,'shutdown timeout must be 0..300 seconds')
    for _,k in ipairs({'runtime_dir','state_file','log_file'})do assert(config[k]==nil or (type(config[k])=='string'and config[k]:sub(1,1)=='/'and not config[k]:find('\0',1,true)),'invalid path '..k)end
    return config
end
return M
