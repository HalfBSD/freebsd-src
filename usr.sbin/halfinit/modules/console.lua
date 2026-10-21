-- SPDX-License-Identifier: BSD-2-Clause
-- Policy only: native login retains PAM, credentials, groups and tty ownership.
local M={}
function M.account(value)
    return type(value)=='string'and #value>0 and #value<=64 and value:match('^[a-zA-Z_][a-zA-Z0-9_.-]*$')~=nil
end
function M.validate(value)
    if value==nil then value={}end
    assert(type(value)=='table','console must be a table')
    for key in pairs(value)do assert(key=='user'or key=='autologin'or key=='select_timeout','unknown console field: '..tostring(key))end
    local user=value.user;if user==nil then user='root'end
    assert(M.account(user),'invalid console user')
    local enabled=value.autologin;if enabled==nil then enabled=true end
    assert(type(enabled)=='boolean','console.autologin must be boolean')
    local timeout=value.select_timeout;if timeout==nil then timeout=3 end
    assert(type(timeout)=='number'and timeout==timeout and timeout>=0 and timeout<=30,'console.select_timeout must be 0..30')
    return {user=user,autologin=enabled,select_timeout=timeout}
end
-- Only literal legacy assignments are accepted; never evaluate shell code.
function M.legacy(text,current)
    assert(#text<=1048576,'legacy console configuration too large')
    for line in (text..'\n'):gmatch('(.-)\n')do
        line=line:match('^%s*(.-)%s*$')
        if line:sub(1,1)~='#'and line:find('console_user',1,true)then
            local rhs=assert(line:match('^console_user%s*=%s*(.-)%s*$'),'unsupported legacy console_user; set console.user in system.lua')
            local user=rhs:match('^"([%w_.-]+)"%s*#?.*$')or rhs:match("^'([%w_.-]+)'%s*#?.*$")
                or rhs:match('^([%w_.-]+)%s*$')or rhs:match('^([%w_.-]+)%s+#.*$')
            -- Quoted values must end at the quote or a comment, not shell code.
            if rhs:sub(1,1)=='"'or rhs:sub(1,1)=="'"then
                local quote=rhs:sub(1,1);local closing=rhs:find(quote,2,true)
                local tail=closing and rhs:sub(closing+1):match('^%s*(.-)%s*$')
                assert(tail and(tail==''or tail:sub(1,1)=='#'),'unsupported legacy console_user expression')
            end
            assert(M.account(user),'unsupported legacy console_user; set console.user in system.lua')
            current=user
        end
    end
    return current
end
function M.select(policy,input,lookup)
    policy=M.validate(policy)
    if not policy.autologin then return ''end
    if input~=nil then
        input=input:match('^%s*(.-)%s*$')
        if input=='login'or input=='?'then return ''end
        if input~=''then policy.user=input end
    end
    assert(M.account(policy.user),'invalid console account selection')
    local account=assert(lookup(policy.user),'console account does not exist: '..policy.user)
    assert(account.shell and account.shell~=''and not account.shell:match('/nologin$')and not account.shell:match('/false$'),
        'console account has no login shell')
    return policy.user
end
function M.main(args)
    assert(#args==0,'halfconsole takes no arguments')
    local native=require('halfbsd.os');local driver=require('halfbsd.console')
    local c=require('config').read('/etc/halfbsd/system.lua',0)
    local policy=c.console
    if policy==nil then
        local user
        for _,path in ipairs({'/etc/rc.conf','/etc/rc.conf.local'})do
            local st,err=native.stat(path)
            if st then
                assert(st.type=='file'and st.uid==0 and(st.mode&0x12)==0,'unsafe legacy console configuration')
                local file=assert(io.open(path));local text=file:read(1048577);file:close()
                user=M.legacy(text or'',user)
            else assert(err and err.name=='ENOENT','cannot inspect '..path)end
        end
        policy={user=user or'root'}
        if user then io.stderr:write('halfconsole: using literal legacy console_user; migrate to console.user in system.lua\n')end
    end
    policy=M.validate(policy)
    if not policy.autologin then return ''end
    -- Validate the default before allowing a timeout to select it.
    M.select(policy,nil,native.user)
    return M.select(policy,driver.choose(policy.user,policy.select_timeout),native.user)
end
return M
