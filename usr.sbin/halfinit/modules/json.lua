-- SPDX-License-Identifier: BSD-2-Clause
local M = {null={}}
local function quote(s)
    return '"'..s:gsub('[%z\1-\31\\"]', function(c)
        if c=='"' or c=='\\' then return '\\'..c end
        return ('\\u%04x'):format(c:byte())
    end)..'"'
end
function M.encode(value)
    local seen={}
    local function encode(v,depth)
        assert(depth<32,'JSON nesting limit')
        if v==M.null then return 'null' end
        local t=type(v)
        if t=='nil' then return 'null' end
        if t=='boolean' then return tostring(v) end
        if t=='string' then return quote(v) end
        if t=='number' then assert(v==v and math.abs(v)<math.huge,'nonfinite JSON number');return tostring(v) end
        assert(t=='table' and not seen[v],'invalid or cyclic JSON value');seen[v]=true
        local array=true;local count=0
        for k in pairs(v) do count=count+1;if type(k)~='number' or k%1~=0 or k<1 then array=false end end
        if count~=#v then array=false end
        local parts={}
        if array and count>0 then
            for i=1,#v do parts[i]=encode(v[i],depth+1) end
        else
            local keys={};for k in pairs(v) do assert(type(k)=='string','JSON object key');keys[#keys+1]=k end;table.sort(keys)
            for _,k in ipairs(keys) do parts[#parts+1]=quote(k)..':'..encode(v[k],depth+1) end
        end
        seen[v]=nil
        return (array and count>0 and '[' or '{')..table.concat(parts,',')..(array and count>0 and ']' or '}')
    end
    return encode(value,0)
end
function M.decode(s)
    assert(type(s)=='string' and #s<=1048576,'JSON size limit')
    local i,tokens=1,0
    local function ws()local _,last=s:find('^[ \r\n\t]*',i);i=(last or i-1)+1 end
    local function string_value()
        assert(s:sub(i,i)=='"','JSON string expected');i=i+1;local parts={}
        while i<=#s do
            local c=s:sub(i,i);i=i+1
            if c=='"' then return table.concat(parts) end
            assert(c:byte()>=32,'JSON control byte')
            if c=='\\' then
                local e=s:sub(i,i);i=i+1
                local escapes={['"']='"',['\\']='\\',['/']='/',b='\b',f='\f',n='\n',r='\r',t='\t'}
                if e=='u' then
                    local hex=s:sub(i,i+3);assert(hex:match('^%x%x%x%x$'),'invalid Unicode escape');i=i+4;local n=tonumber(hex,16)
                    if n>=0xd800 and n<=0xdbff then
                        assert(s:sub(i,i+1)=='\\u','missing low surrogate');i=i+2
                        local low=s:sub(i,i+3);assert(low:match('^%x%x%x%x$'),'invalid low surrogate');i=i+4;low=tonumber(low,16)
                        assert(low>=0xdc00 and low<=0xdfff,'invalid low surrogate');n=0x10000+(n-0xd800)*1024+low-0xdc00
                    else assert(n<0xdc00 or n>0xdfff,'unpaired low surrogate') end
                    parts[#parts+1]=utf8.char(n)
                else assert(escapes[e],'invalid JSON escape');parts[#parts+1]=escapes[e] end
            else parts[#parts+1]=c end
        end
        error('unterminated JSON string')
    end
    local parse
    parse=function(depth)
        tokens=tokens+1;assert(tokens<=65536 and depth<32,'JSON complexity limit');ws();local c=s:sub(i,i)
        if c=='"' then return string_value() end
        if c=='{' or c=='[' then
            local object=c=='{';i=i+1;local result={};ws();local finish=object and '}' or ']'
            if s:sub(i,i)==finish then i=i+1;return result end
            repeat
                ws();local key=object and string_value() or #result+1
                if object then ws();assert(s:sub(i,i)==':','missing JSON colon');i=i+1;assert(result[key]==nil,'duplicate JSON key') end
                result[key]=parse(depth+1);ws();c=s:sub(i,i);i=i+1
                if c==finish then return result end
                assert(c==',','missing JSON separator')
            until false
        end
        for text,value in pairs({['true']=true,['false']=false,['null']=M.null}) do
            if s:sub(i,i+#text-1)==text then i=i+#text;return value end
        end
        local tail=s:sub(i);local number=tail:match('^-?%d+%.?%d*[eE][+-]?%d+') or tail:match('^-?%d+%.%d+') or tail:match('^-?%d+')
        assert(number and not number:match('^-?0%d') and not number:match('%.[eE]'),'invalid JSON number')
        i=i+#number;local value=tonumber(number);assert(value and value==value and math.abs(value)<math.huge,'invalid JSON number');return value
    end
    local value=parse(0);ws();assert(i>#s,'trailing JSON bytes');return value
end
return M
