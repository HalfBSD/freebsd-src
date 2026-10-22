-- SPDX-License-Identifier: BSD-2-Clause
-- Invoked by input-hotplug.sh. Only inventories, devd broadcast and Sway GET IPC.
return function(args,dependencies)
    local d=dependencies or{}
    local native=d.native or require('halfbsd.os');local proc=d.proc or require('proc')
    local event=d.event or require('halfbsd.event');local ipc=d.ipc or require('halfbsd.ipc')
    assert(#args>=1 and #args<=3,'usage: input-hotplug.sh NEW-ABSOLUTE-DIRECTORY [SECONDS] [SWAYSOCK]')
    local directory=args[1]
    assert(type(directory)=='string'and directory:sub(1,1)=='/'and not directory:find('\0',1,true),'absolute report directory required')
    for part in directory:gmatch('[^/]+')do assert(part~='.'and part~='..','canonical report directory required')end
    local seconds=tonumber(args[2]or'45')
    assert(seconds and seconds%1==0 and seconds>=5 and seconds<=60,'duration must be 5..60 seconds')
    if args[3]then assert(args[3]:sub(1,1)=='/'and not args[3]:find('\0',1,true),'absolute Sway socket required')end
    assert(native.identity().euid==0,'run as root for complete device inventories')
    local exists,err=native.exists(directory);assert(exists~=nil,err and err.message)
    assert(not exists,'report directory already exists');assert(native.mkdir(directory,0x1c0))
    -- Reserve the final MiB for broadcast evidence and the capture summary.
    local bytes,limit=0,7*1024*1024
    local function save(name,data)
        assert(name:match('^[a-zA-Z0-9_.-]+$'))
        if bytes+#data>limit then data='CAPTURE SIZE LIMIT REACHED\n'end
        if bytes+#data>limit then return end
        assert(native.atomic_replace(directory..'/'..name,data,0x180));bytes=bytes+#data
    end
    local function command(argv)
        local result,problem=proc.run(argv,{stdout='pipe',stderr='pipe',timeout=2,grace=0.1,exec_timeout=1,output_limit=262144})
        local head='$ '..table.concat(argv,' ')..'\n'
        if not result then return head..'UNAVAILABLE: '..tostring(problem and(problem.message or problem.name))..'\n'end
        return head..'exit='..tostring(result.exit_code)..' signal='..tostring(result.signal)..'\n'..(result.stdout or'')..(result.stderr or'')..'\n'
    end
    local function commands(list)
        local rows={}
        for _,argv in ipairs(list)do rows[#rows+1]=command(argv)end
        return table.concat(rows)
    end
    local function sockets()
        if args[3]then return {args[3]}end
        local roots={'/tmp'};local runtime=native.getenv('XDG_RUNTIME_DIR')
        if runtime and runtime:sub(1,1)=='/'then roots[#roots+1]=runtime end
        for i,name in ipairs(native.listdir('/var/run/user')or{})do
            if i>32 then break end
            if name:match('^%d+$')then roots[#roots+1]='/var/run/user/'..name end
        end
        local result,seen={},{}
        for _,root in ipairs(roots)do
            local st=native.stat(root)
            if st and st.type=='directory'then
                for _,name in ipairs(native.listdir(root)or{})do
                    if name:match('^sway%-ipc%.%d+%.%d+%.sock$')then
                        local path=root..'/'..name;local socket=native.stat(path)
                        if socket and socket.type=='socket'and not seen[path]then
                            seen[path]=true;result[#result+1]=path
                            if #result==8 then return result end
                        end
                    end
                end
            end
        end
        return result
    end
    local inventory={
        {'/usr/bin/uname','-a'},{'/sbin/kldstat'},
        {'/usr/local/sbin/pkg','info','-x','^(sway|wlroots|libinput|libudev-devd|libevdev|seatd)'},
        {'/usr/bin/ldd','/usr/local/bin/sway'},{'/usr/sbin/devinfo','-v'},
        {'/usr/bin/sockstat','-u'},{'/sbin/dmesg'}}
    save('before.txt',commands(inventory))
    local loop <close> = assert(event.new())
    local monitor,problem=ipc.connect('/var/run/devd.pipe',1)
    local messages=monitor and''or('MONITOR UNAVAILABLE: '..tostring(problem and(problem.message or problem.name))..'\n')
    local total,watch=0,nil
    local function close_monitor()
        if watch then loop:cancel(watch);watch=nil end
        if monitor then monitor:close();monitor=nil end
    end
    if monitor then
        watch=assert(loop:watch_read(monitor,function()
            while monitor do
                local data,error=monitor:read(4096)
                if not data then
                    if error and error.name=='EAGAIN'then return end
                    messages=messages..'\nMONITOR CLOSED: '..tostring(error and error.name)..'\n';close_monitor();return
                end
                total=total+#data
                if total>524288 then messages=messages..'\nEVENT CAPTURE LIMIT REACHED\n';close_monitor();return end
                messages=messages..data
            end
        end))
    end
    local index=0
    local function snapshot()
        local rows={'monotonic='..tostring(native.clock())..'\n',commands({
            {'/sbin/sysctl','kern.evdev.rcpt_mask'},
            {'/sbin/sysctl','-x','kern.evdev.input'},
            {'/usr/sbin/usbconfig','list'},{'/bin/ls','-l','/dev/input'},
            {'/bin/ps','-axo','pid,ppid,user,command'},
            {'/usr/sbin/halfsvc','status'}})}
        for _,socket in ipairs(sockets())do rows[#rows+1]=command({'/usr/local/bin/swaymsg','-s',socket,'-t','get_inputs','-r'})end
        save(string.format('snapshot-%02d.txt',index),table.concat(rows));index=index+1
    end
    local ok,failure=xpcall(function()
        print('Capture active for approximately '..seconds..' seconds. Switch away/back, move and click the mouse. Do not restart Sway yet.')
        snapshot()
        assert(loop:watch_timer(3,snapshot,true))
        assert(loop:watch_timer(seconds,function()loop:stop()end))
        assert(loop:run())
    end,function(e)return tostring(type(e)=='table'and(e.message or e.name)or e)end)
    close_monitor()
    assert(native.atomic_replace(directory..'/devd-events.txt',messages,0x180))
    save('after.txt',commands(inventory))
    local summary='Passive inventories and devd broadcast; no evdev device opened, no input grabs or key-event recording.\n'..
        'Each command has a 2-second deadline; requested monitoring window excludes inventory overhead.\n'..
        'Sway GET_INPUTS appears only if its IPC socket is discovered or explicitly supplied.\n'..
        'Boot policy, drivers, permissions and services were not changed. Command failures remain in individual files.\n'..
        'Event-loop result: '..(ok and'completed'or failure)..'\n'
    assert(native.atomic_replace(directory..'/README.txt',summary,0x180))
    print('Capture saved to '..directory)
    return ok and 0 or 1
end
