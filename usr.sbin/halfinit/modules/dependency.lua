-- SPDX-License-Identifier: BSD-2-Clause
local M={}
local function sorted(t)local keys={};for k in pairs(t)do keys[#keys+1]=k end;table.sort(keys);return keys end
M.sorted=sorted
function M.validate(registry)
    for name,d in pairs(registry) do
        for _,kind in ipairs({'requires','wants','after','before'})do
            for _,other in ipairs(d[kind])do assert(registry[other],name..': missing '..kind..' dependency '..other);assert(other~=name,name..': self dependency')end
        end
    end
    local all={};for name in pairs(registry)do all[name]=true end
    M.plan(registry,all,true)
end
function M.plan(registry,roots,all)
    local selected={}
    local function select(name)
        assert(registry[name],'unknown service: '..tostring(name))
        if selected[name]then return end;selected[name]=true
        for _,kind in ipairs({'requires','wants'})do for _,n in ipairs(registry[name][kind])do select(n)end end
    end
    for name in pairs(roots)do select(name)end
    local edges,degree={},{}
    for name in pairs(selected)do edges[name]={};degree[name]=0 end
    local function edge(a,b)if selected[a]and selected[b]and not edges[a][b]then edges[a][b]=true;degree[b]=degree[b]+1 end end
    for name in pairs(selected)do
        local d=registry[name]
        for _,kind in ipairs({'requires','wants','after'})do for _,n in ipairs(d[kind])do edge(n,name)end end
        for _,n in ipairs(d.before)do edge(name,n)end
    end
    local waves,remaining={},{}
    for name in pairs(selected)do remaining[name]=true end
    while next(remaining)do
        local wave={};for _,name in ipairs(sorted(remaining))do if degree[name]==0 then wave[#wave+1]=name end end
        assert(#wave>0,'dependency/ordering cycle: '..table.concat(sorted(remaining),','))
        waves[#waves+1]=wave
        for _,name in ipairs(wave)do remaining[name]=nil;for other in pairs(edges[name])do degree[other]=degree[other]-1 end end
    end
    return {selected=selected,edges=edges,waves=waves}
end
return M
