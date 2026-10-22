-- SPDX-License-Identifier: BSD-2-Clause
-- Fresh-install template generation only; administrator Lua is never rewritten.
return function(template_path,hostname_path)
    local function read(path)
        local f=assert(io.open(path,'r'));local text=f:read(1048577);f:close()
        assert(#text<=1048576,'installer input exceeds size limit');return text
    end
    local hostname=read(hostname_path):match('^%s*(.-)%s*$')
    assert(#hostname>0 and #hostname<=253 and hostname:match('^[a-zA-Z0-9][a-zA-Z0-9.%-]*$'),'invalid hostname')
    local config=assert(load(read(template_path),'installer template','t',{}))()
    assert(type(config)=='table'and type(config.boot)=='table','invalid boot template')
    config.boot.hostname=hostname
    local count,seen=0,{}
    local function encode(value,depth)
        count=count+1;assert(count<=4096 and depth<=32,'installer template too complex')
        local kind=type(value)
        if kind=='string'then return string.format('%q',value)end
        if kind=='boolean'then return tostring(value)end
        if kind=='number'then assert(value==value and math.abs(value)<math.huge);return tostring(value)end
        assert(kind=='table'and not seen[value],'unsupported installer template value')
        seen[value]=true;local keys,rows={},{}
        for key in pairs(value)do assert(type(key)=='string'or type(key)=='number');keys[#keys+1]=key end
        table.sort(keys,function(a,b)return type(a)==type(b)and a<b or type(a)<type(b)end)
        for _,key in ipairs(keys)do rows[#rows+1]='['..encode(key,depth+1)..']='..encode(value[key],depth+1)end
        seen[value]=nil;return '{'..table.concat(rows,',')..'}'
    end
    return '-- HalfBSD installed boot policy. Administrator-owned.\nreturn '..encode(config,0)..'\n'
end
