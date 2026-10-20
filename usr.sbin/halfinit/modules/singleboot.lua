-- SPDX-License-Identifier: BSD-2-Clause
-- Bring-up checkpoint only. PID 1, not Lua, owns the interactive recovery shell.
local M={}
local function diagnostic(err)
    if type(err)=='table'then
        return tostring(err.operation or 'native operation')..': '..tostring(err.message or err.name or err.kind)
    end
    return tostring(err)
end
function M.run(native,proc,number,output)
    local function log(text)
        text='[halfinit single] '..text
        if output then output(text)else io.stdout:write(text..'\n');io.stdout:flush()end
    end
    log('Lua entered diagnostic single-user boot; networking and multiuser services are disabled')
    local identity=assert(native.identity())
    assert(identity.euid==0 and identity.ppid==1,'single boot requires root directly launched by PID 1')
    assert(number==3,'single boot requires PID 1 notification descriptor 3')
    local notify=assert(proc.fd(number))
    local function command(label,argv,timeout)
        log('BEGIN '..label..': '..table.concat(argv,' '))
        local result,err=proc.run(argv,{stdin='null',
            stdout={path='/dev/console',append=true},stderr={path='/dev/console',append=true},
            new_session=true,process_group=true,timeout=timeout,grace=2,exec_timeout=5})
        assert(result,diagnostic(err))
        assert(result.exit_code==0,label..' failed: exit='..tostring(result.exit_code)..' signal='..tostring(result.signal))
        log('OK '..label)
    end
    local ok,err=xpcall(function()
        log('BEGIN verify kernel ZFS root and devfs')
        local root,devfs
        for _,m in ipairs(assert(native.mounts()))do
            if m.path=='/'then root=m end
            if m.path=='/dev'and m.type=='devfs'then devfs=true end
        end
        assert(root and root.type=='zfs','kernel root must already be mounted as ZFS')
        assert(devfs,'PID 1 has not mounted devfs')
        log('OK kernel ZFS root: '..tostring(root.source)..'; readonly='..tostring(root.readonly))
        assert(native.stat('/dev/console'),'console device missing')
        log('OK /dev/console is present; kernel console output is active')
        command('mount datasets from already-imported ZFS pools',{'/sbin/zfs','mount','-a'},120)
        local mounted={}
        for _,m in ipairs(assert(native.mounts()))do
            if m.type=='zfs'then mounted[#mounted+1]=tostring(m.source)..' on '..m.path end
        end
        table.sort(mounted)
        for _,description in ipairs(mounted)do log('ZFS '..description)end
        command('load AMDGPU (or confirm already loaded)',{'/sbin/kldload','-n','-v','amdgpu'},120)
        command('show loaded AMDGPU module',{'/sbin/kldstat','-n','amdgpu.ko'},10)
        local nodes={};local card=false
        local exists,exists_error=native.exists('/dev/dri')
        assert(exists~=nil,diagnostic(exists_error))
        if exists then
            for _,name in ipairs(assert(native.listdir('/dev/dri')))do
                if name:match('^card%d+$')then card=true end
                if name:match('^card%d+$')or name:match('^renderD%d+$')then nodes[#nodes+1]=name end
            end
        end
        table.sort(nodes)
        log('DRM device nodes: '..(#nodes>0 and table.concat(nodes,', ')or 'none; driver load alone does not prove GPU attachment'))
        assert(card,'AMDGPU loaded but no DRM card device appeared; inspect kernel graphics messages')
        log('OK diagnostic tasks finished; requesting PID 1 single-user shell (not multiuser)')
        assert(notify:write('SINGLE\n')==7,'single-user handoff write failed')
    end,diagnostic)
    if not ok then
        log('FAILED '..diagnostic(err)..'; returning to PID 1 recovery shell')
        notify:write('FAIL\n')
    end
    notify:close()
    -- No live children or event loop remain; PID 1 waits for this process to exit.
    return ok and 0 or 1
end
return M
