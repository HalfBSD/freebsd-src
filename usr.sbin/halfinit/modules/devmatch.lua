-- SPDX-License-Identifier: BSD-2-Clause
-- One bounded devd action, not a second service manager. No shell execution.
local M={}
function M.main(args,dependencies)
    local d=dependencies or{}
    local native=d.native or require('halfbsd.os');local proc=d.proc or require('proc')
    assert(#args==2 and args[1]=='--device-nomatch','device match requires exactly one event')
    assert(native.identity().euid==0,'device matching requires root')
    local event=args[2]
    assert(type(event)=='string'and #event>=8 and #event<=8192 and not event:find('[%z\r\n]')and event:find(' on ',1,true),
        'invalid device nomatch event')
    local deadline=native.clock('monotonic')+15
    local function run(argv)
        local remaining=deadline-native.clock('monotonic')
        assert(remaining>0,'device module match deadline exceeded')
        local result,err=proc.run(argv,{stdin='null',stdout='pipe',stderr='pipe',
            timeout=math.min(remaining,5),exec_timeout=1,grace=0.1,output_limit=32768})
        assert(result,err and err.message or'device command failed')
        return result
    end
    -- Read one complete environment snapshot. kenv's missing-key exit status
    -- does not distinguish ENOENT from other syscall errors; never interpret a
    -- failed blocklist read as an empty blocklist.
    local environment=run({'/bin/kenv','-q'})
    assert(environment.success,'cannot read kernel environment')
    local settings={}
    for line in (environment.stdout or''):gmatch('[^\n]+')do
        local key=line:match('^([^=]+)=')
        if key=='halfbsd.boot_active'or key=='boot_safe'or key=='devmatch_blocklist'then
            assert(settings[key]==nil,'duplicate device policy setting')
            settings[key]=assert(line:match('^[^=]+="(.*)"$'),'invalid device policy setting')
        end
    end
    local function value(name)return settings[name]or''end
    assert(value('halfbsd.boot_active')=='lua','device match requires HalfBSD startup ownership')
    local safe=value('boot_safe'):lower()
    if safe=='yes'or safe=='true'or safe=='1'or safe=='on'then return 0 end
    local blocked={}
    for word in value('devmatch_blocklist'):gmatch('%S+')do blocked[word:gsub('%.ko$','')]=true end
    local r=run({'/sbin/devmatch','-p',event})
    assert(r.success,'device matching failed: '..(r.stderr or''))
    local modules,seen={},{}
    for word in (r.stdout or''):gmatch('%S+')do
        local name=word:gsub('%.ko$','')
        assert(#name<=128 and name:match('^[%w_][%w_.%-]*$')and not name:find('..',1,true),
            'invalid module name from devmatch')
        if not seen[name]then
            seen[name]=true;modules[#modules+1]=name
            assert(#modules<=32,'too many module candidates')
        end
    end
    -- Validate the complete result before any module load. No global devctl
    -- freeze: each kldload completes independently, with normal kernel attach.
    for _,name in ipairs(modules)do
        if not blocked[name]then
            local loaded=run({'/sbin/kldstat','-q','-n',name})
            if not loaded.success then
                assert(loaded.exit_code==1 and not loaded.signal,'cannot inspect module '..name)
                local result=run({'/sbin/kldload','-n',name})
                assert(result.success,'cannot load module '..name..': '..(result.stderr or''))
            end
        end
    end
    return 0
end
return M
