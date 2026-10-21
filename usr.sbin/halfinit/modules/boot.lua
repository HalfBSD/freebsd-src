-- SPDX-License-Identifier: BSD-2-Clause
-- Compile administrator boot policy into ordinary Phase II service definitions.
-- Inventory calls are read-only. Loading this module never starts a service.
local M={}
local function fields(value,allowed)
    assert(type(value)=='table','table required')
    for key in pairs(value)do assert(allowed[key],'unknown boot field: '..tostring(key))end
end
local function boolean(value,default)
    if value==nil then return default end
    assert(type(value)=='boolean','boolean required');return value
end
local function name(value)
    return type(value)=='string'and #value>0 and #value<16 and value:match('^[%w_.-]+$')~=nil
end
local function path(value)
    assert(type(value)=='string'and value:sub(1,1)=='/'and not value:find('\0',1,true),'absolute path required')
    assert(not value:find('//',1,true)and not(value..'/'):find('/../',1,true)and not(value..'/'):find('/./',1,true)and(value=='/'or value:sub(-1)~='/'),'canonical path required');return value
end
local function dense(value)
    assert(type(value)=='table','array required');local count=0
    for key in pairs(value)do assert(type(key)=='number'and key%1==0 and key>=1 and key<=#value,'dense array required');count=count+1 end
    assert(count==#value and count<=128,'array too large or sparse');return value
end
function M.compile(policy,os)
    fields(policy,{mode=true,interfaces=true,local_mounts=true,zfs=true,firewall=true,dns=true,sshd=true,ntpd=true,seatd=true,
        desktop=true,root_writable=true,swap=true,device_rules=true,hostname=true,syslogd=true,amdgpu=true})
    os=os or require('halfbsd.os')
    local services={}
    local function add(d)d.system=true;services[#services+1]=d;return d end
    local function target(n,requires,wants)return add({name=n,type='target',requires=requires or{},wants=wants or{}})end
    if policy.mode=='user'then
        fields(policy.desktop,{command=true,runtime=true,seat_socket=true,wayland_socket=true})
        local runtime=path(policy.desktop.runtime or assert(os.getenv('XDG_RUNTIME_DIR'),'XDG_RUNTIME_DIR is not set'))
        local command=dense(policy.desktop.command or{'/usr/local/bin/sway'});path(command[1])
        assert(command[1]:match('/(sway)$')or command[1]:match('/(niri)$'),'user desktop must select sway or niri explicitly')
        assert(not command[1]:match('/niri$')or policy.desktop.wayland_socket,'niri requires an explicit Wayland readiness socket')
        local wayland=path(policy.desktop.wayland_socket or runtime..'/wayland-0')
        assert(wayland:sub(1,#runtime+1)==runtime..'/','Wayland readiness socket must be under the user runtime directory')
        add({name='session',type='virtual',condition='root_ready',guard='user_session_guard',parameters={runtime=runtime}})
        add({name='seat',type='virtual',condition='socket_ready',requires={'session'},parameters={path=path(policy.desktop.seat_socket or'/var/run/seatd.sock')}})
        add({name='compositor',type='daemon',command=command,requires={'seat'},guard='exclusive_guard',parameters={user_mode=true,path=wayland,processes={'sway','niri'}},
            new_session=false,stdin='inherit',foreground=true,readiness='condition',condition='socket_ready',restart='never',start_timeout=30,
            env={XDG_RUNTIME_DIR=runtime,XDG_SESSION_TYPE='wayland',LIBSEAT_BACKEND='seatd'}})
        target('desktop',{'compositor'});return services
    end
    assert(policy.mode==nil or policy.mode=='system','unknown boot mode')
    local mounts=policy.local_mounts
    if mounts==nil or mounts=='fstab'then
        mounts={}
        for _,entry in ipairs(assert(os.fstab()))do
            local options={};for word in entry.options:gmatch('[^,]+')do options[word]=true end
            if entry.type~='swap'and entry.type~='zfs'and entry.path~='/'and not options.noauto then
                assert(entry.type~='nfs'and entry.type~='smbfs','remote fstab mounts require a separate network adapter')
                local native={fstype=entry.type,fspath=entry.path,from=entry.source}
                for word in entry.options:gmatch('[^,]+')do
                    local key,value=word:match('^([^=]+)=(.*)$')
                    if word~='rw'and word~='ro'and word~='auto'then native[key or word]=value or''end
                end
                mounts[#mounts+1]={options=native,readonly=options.ro or false,fsck=entry.pass>0}
            end
        end
    end
    dense(mounts)
    local interfaces=policy.interfaces
    if interfaces==nil or interfaces=='auto'then
        local possible,carrier={},{}
        for _,row in ipairs(assert(os.interfaces()))do
            if row.family=='link'and row.link_type==6 and not row.loopback and not row.name:match('^(bridge)')and not row.name:match('^(epair)')then
                possible[row.name]=true;if row.link_state==2 then carrier[row.name]=true end
            end
        end
        local candidates={};for n in pairs(next(carrier)and carrier or possible)do candidates[#candidates+1]=n end;table.sort(candidates)
        assert(#candidates>0,'no Ethernet interface found; configure interfaces explicitly')
        interfaces={candidates[1]}
    end
    dense(interfaces);assert(#interfaces>0,'at least one DHCP interface required')
    local unique={};for _,n in ipairs(interfaces)do assert(name(n)and not unique[n],'invalid/duplicate interface');unique[n]=true end
    local firewall=policy.firewall or{}
    fields(firewall,{ownership=true,mode=true,ipv6=true})
    local load=firewall.ownership=='load'
    assert(firewall.ownership==nil or firewall.ownership=='verify'or load,'unknown firewall ownership')
    local mode=firewall.mode or'workstation';assert(mode=='workstation'or mode=='closed'or mode=='open','unknown firewall mode')
    local ipv6=boolean(firewall.ipv6,true)
    local logs=boolean(policy.syslogd,true)
    local dns=boolean(policy.dns,true);local ssh=boolean(policy.sshd,true);local ntp=boolean(policy.ntpd,true);local seat=boolean(policy.seatd,false)
    local processes={'halfinit','dhclient'}
    if logs then processes[#processes+1]='syslogd' end
    if dns then processes[#processes+1]='local-unbound';processes[#processes+1]='unbound'end
    if ssh then processes[#processes+1]='sshd'end
    if ntp then processes[#processes+1]='ntpd'end
    if seat then processes[#processes+1]='seatd'end
    add({name='boot-preflight',type='virtual',condition='root_ready',guard='exclusive_guard',parameters={processes=processes}})
    local rules=dense(policy.device_rules or{})
    for _,rule in ipairs(rules)do
        fields(rule,{path=true,mode=true,uid=true,gid=true});path(rule.path)
        assert(rule.path:sub(1,5)=='/dev/','device rules require a devfs path')
        for _,key in ipairs({'mode','uid','gid'})do
            local value=rule[key]
            assert(value==nil or(type(value)=='number'and value%1==0 and value>=0 and value<=(key=='mode'and 4095 or 4294967295)),'invalid device permission')
        end
        assert(rule.uid==nil or rule.gid~=nil,'device owner requires a group')
    end
    add({name='devices',type='oneshot',action='boot_devices',requires={'boot-preflight'},parameters={rules=policy.device_rules or{}}})
    add({name='rootfs',type='oneshot',action='root_writable',requires={'devices'},parameters={update=boolean(policy.root_writable,false)}})
    local fsnodes={};local points={}
    for i,m in ipairs(mounts)do
        assert(type(m.options)=='table','mount options required');local point=path(m.options.fspath)
        assert(not points[point],'duplicate mount point');points[point]=i
    end
    for i,m in ipairs(mounts)do
        fields(m,{options=true,readonly=true,fsck=true})
        assert(type(m.options)=='table','mount options required');path(m.options.fspath)
        assert(m.options.fspath~='/','kernel root mount must not be duplicated')
        local supported={ufs=true,tmpfs=true,devfs=true,nullfs=true,cd9660=true,msdosfs=true}
        assert(supported[m.options.fstype],'unsupported native local filesystem')
        assert(type(m.options.from)=='string'and #m.options.from>0 and not m.options.from:find('\0',1,true),'mount source required')
        for key,value in pairs(m.options)do
            assert(type(key)=='string'and #key>0 and not key:find('\0',1,true)and type(value)=='string'and not value:find('\0',1,true),'invalid native mount option')
        end
        m.readonly=boolean(m.readonly,false);m.fsck=boolean(m.fsck,false)
        local before={'rootfs'};local parent
        for point,index in pairs(points)do
            if m.options.fspath:sub(1,#point+1)==point..'/'and(not parent or #point>#parent)then parent=point;before={'mount-'..index}end
        end
        if m.fsck then
            assert(m.options.fstype=='ufs'or m.options.fstype=='msdosfs','automatic fsck requires UFS or FAT')
            local n='fsck-'..i
            add({name=n,type='oneshot',command={'/sbin/fsck','-p',m.options.from},requires=before,guard='fsck_guard',parameters=m,success_codes={0},start_timeout=300})
            before={n}
        end
        local n='mount-'..i
        add({name=n,type='oneshot',action='local_mount',cleanup='unmount_owned',requires=before,parameters=m});fsnodes[#fsnodes+1]=n
    end
    target('localfs',{'rootfs'})
    -- Mounts are essential, not optional members of localfs.
    for _,n in ipairs(fsnodes)do table.insert(services[#services].requires,n)end
    if boolean(policy.zfs,true)then
        add({name='zfs',type='oneshot',command={'/sbin/zfs','mount','-a'},requires={'localfs'},guard='root_guard',start_timeout=120})
    else target('zfs',{'localfs'})end
    local platform='zfs'
    if boolean(policy.amdgpu,false)then
        add({name='amdgpu-load',type='oneshot',command={'/sbin/kldload','-n','-v','amdgpu'},
            requires={'zfs'},guard='root_guard',stdout='/dev/console',stderr='/dev/console',start_timeout=120})
        add({name='amdgpu-module',type='oneshot',command={'/sbin/kldstat','-n','amdgpu.ko'},
            requires={'amdgpu-load'},guard='root_guard',stdout='/dev/console',stderr='/dev/console'})
        add({name='graphics',type='virtual',condition='drm_ready',requires={'amdgpu-module'},start_timeout=15})
        platform='graphics'
    end
    add({name='temporary',type='oneshot',action='directories',guard='root_guard',requires={platform},parameters={directories={
        {path='/var/run',mode=493,uid=0,gid=0},{path='/tmp',mode=1023,uid=0,gid=0},{path='/var/tmp',mode=1023,uid=0,gid=0}}}})
    add({name='runtime',type='oneshot',action='directories',guard='root_guard',requires={'temporary'},parameters={directories={
        {path='/var/run/halfbsd',mode=493},{path='/var/run/dhclient',mode=493}}}})
    add({name='loopback',type='oneshot',command={'/sbin/ifconfig','lo0','inet','127.0.0.1/8','up'},requires={'runtime'},guard='loopback_guard'})
    local swap=policy.swap
    if swap==nil or swap=='fstab'then
        swap={};for _,entry in ipairs(assert(os.fstab()))do
            if entry.type=='swap'and not entry.options:match('noauto')then swap[#swap+1]=entry.source end
        end
    elseif swap==false then swap={}end
    dense(swap);local swaps={}
    for i,source in ipairs(swap)do
        path(source);assert(source:sub(1,5)=='/dev/','swap must be an existing device, not a file or automatic attachment')
        local n='swap-'..i;swaps[#swaps+1]=n
        add({name=n,type='oneshot',action='swap_up',cleanup='swap_down',requires={'runtime'},parameters={path=source}})
    end
    if policy.hostname then
        assert(type(policy.hostname)=='string'and #policy.hostname>0 and not policy.hostname:find('\0',1,true),'invalid hostname')
        add({name='hostname',type='oneshot',action='boot_hostname',requires={'runtime'},parameters={name=policy.hostname}})
    end
    local previous='loopback'
    if load then
        add({name='ipfw-baseline',type='oneshot',command={'/sbin/ipfw','-S','list'},verify='firewall_baseline',guard='root_guard',requires={previous}})
        previous='ipfw-baseline'
    end
    for _,family in ipairs(ipv6 and{'ip','ip6'}or{'ip'})do
        local n='ipfw-hook-'..family
        add({name=n,type='oneshot',command={'/sbin/sysctl','-n','net.inet'..(family=='ip6'and'6'or'')..'.'..family..'.fw.enable'},
            requires={previous},guard='root_guard',verify='exact_output',parameters={expected='1'}});previous=n
    end
    local expected={}
    local function rule(number,...)
        local arguments={...};local n='ipfw-rule-'..number
        local command={'/sbin/ipfw','-q','add',tostring(number)}
        for _,a in ipairs(arguments)do command[#command+1]=a end
        if number==65534 then
            table.insert(command,5,'set');table.insert(command,6,'31')
        else
            local text=table.concat(arguments,' ')
            if arguments[1]=='check-state'or arguments[#arguments]=='keep-state'then text=text..' :default'end
            expected[tostring(number)]=text
        end
        add({name=n,type='oneshot',command=command,guard='root_guard',requires={previous}});previous=n
    end
    if load then
        rule(65534,'deny','ip','from','any','to','any') -- Preserve deny protection throughout installation.
        rule(100,'allow','ip','from','any','to','any','via','lo0')
        rule(200,'deny','ip','from','any','to','127.0.0.0/8');rule(300,'deny','ip','from','127.0.0.0/8','to','any')
        if ipv6 then rule(400,'deny','ip6','from','any','to','::1');rule(500,'deny','ip6','from','::1','to','any')end
        if mode=='open'then rule(65000,'allow','ip','from','any','to','any')
        elseif mode=='closed'then rule(65000,'deny','ip','from','any','to','any')
        else
            if ipv6 then
                rule(1000,'allow','ipv6-icmp','from','::','to','ff02::/16','icmp6types','135')
                rule(1100,'allow','ipv6-icmp','from','fe80::/10','to','any','icmp6types','133,134,135,136')
                rule(1200,'allow','ipv6-icmp','from','any','to','any','icmp6types','1,2,3,4,135,136')
            end
            rule(2000,'check-state')
            rule(2500,'allow','udp','from','any','to','me','53','in','keep-state')
            rule(2600,'allow','tcp','from','any','to','me','22,53','in','setup','keep-state')
            rule(3000,'allow','tcp','from','me','to','any','out','setup','keep-state')
            rule(3100,'allow','udp','from','me','to','any','out','keep-state');rule(3200,'allow','icmp','from','me','to','any','out','keep-state')
            if ipv6 then rule(3300,'allow','ipv6-icmp','from','me','to','any','out','keep-state')end
            rule(4000,'allow','udp','from','0.0.0.0','68','to','255.255.255.255','67','out')
            rule(4100,'allow','udp','from','any','67','to','me','68','in')
            rule(4200,'allow','udp','from','any','67','to','255.255.255.255','68','in')
            if ipv6 then
                rule(4300,'allow','udp','from','fe80::/10','546','to','ff02::1:2','547','out')
                rule(4400,'allow','udp','from','fe80::/10','547','to','me','546','in')
            end
            rule(4500,'allow','icmp','from','any','to','me','in','icmptypes','3,11,12')
            rule(65000,'deny','log','logamount','100','ip','from','any','to','any')
        end
    end
    add({name='firewall',type='oneshot',command={'/sbin/ipfw','-S','list'},requires={previous},guard='root_guard',verify='firewall_policy',parameters={load=load,rules=expected}})
    local dhcp={}
    for _,interface in ipairs(interfaces)do
        local up='interface-'..interface
        local before={'firewall'};if policy.hostname then before[#before+1]='hostname' end
        add({name=up,type='oneshot',action='interface_up',requires=before,parameters={interface=interface}})
        local n='dhcp-'..interface;dhcp[#dhcp+1]=n
        add({name=n,type='daemon',command={'/sbin/dhclient','-d','-p','/var/run/dhclient/dhclient.'..interface..'.pid',interface},
            requires={up},guard='exclusive_guard',condition='interface_ready',readiness='condition',start_timeout=90,
            restart='on-failure',restart_delay=2,restart_window=300,parameters={interface=interface,processes={'dhclient'}}})
    end
    target('dhcp',dhcp)
    add({name='network',type='virtual',condition='route_ready',requires={'dhcp'},parameters={interface=interfaces[1]},start_timeout=30})
    local libraries={'/sbin/ldconfig','-elf','/lib','/usr/lib'}
    local local_lib,local_error=os.exists('/usr/local/lib');assert(local_lib~=nil,local_error)
    if local_lib then libraries[#libraries+1]='/usr/local/lib' end
    add({name='ldconfig',type='oneshot',command=libraries,guard='root_guard',requires={'runtime'}})
    local essentials={'network','firewall','runtime','ldconfig'}
    local compat_lib,compat_error=os.exists('/usr/lib32');assert(compat_lib~=nil,compat_error)
    if compat_lib then
        add({name='ldconfig32',type='oneshot',command={'/sbin/ldconfig','-32','/usr/lib32'},guard='root_guard',requires={'runtime'}})
        essentials[#essentials+1]='ldconfig32'
    end
    if logs then
        local before={'runtime','ldconfig','firewall'};if policy.hostname then before[#before+1]='hostname' end
        add({name='syslogd',type='daemon',command={'/usr/sbin/syslogd','-F','-ss'},requires=before,
            guard='exclusive_guard',restart='on-failure',stderr='/dev/console',parameters={processes={'syslogd'}}})
        essentials[#essentials+1]='syslogd'
    end
    if policy.hostname then essentials[#essentials+1]='hostname' end
    local networking={};for _,n in ipairs(essentials)do networking[#networking+1]=n end
    if dns then
        local user=assert(os.user('unbound'));local anchor='/var/unbound/root.key'
        add({name='dns-state',type='oneshot',action='directories',guard='root_guard',requires={'runtime'},parameters={directories={
            {path='/var/unbound',mode=493,uid=user.uid,gid=user.gid}}}})
        local before={'dns-state'}
        local exists,err=os.exists(anchor);assert(exists~=nil,err and err.message)
        if not exists then before[#before+1]='network'end
        add({name='dns-anchor',type='oneshot',command={'/usr/sbin/local-unbound-anchor','-a',anchor},user='unbound',requires=before,
            guard='anchor_init_guard',verify='anchor_ready',success_codes={0,1},start_timeout=120,
            parameters={anchor=anchor,uid=user.uid,processes={'unbound','local-unbound'}}})
        add({name='dns-check',type='oneshot',command={'/usr/sbin/local-unbound-checkconf','/etc/unbound/unbound.conf'},requires={'dns-anchor','firewall'},guard='root_guard'})
        add({name='dns',type='daemon',command={'/usr/sbin/local-unbound','-d','-c','/etc/unbound/unbound.conf'},requires={'dns-check','ldconfig'},
            readiness='condition',condition='dns_ready',guard='exclusive_guard',restart='on-failure',parameters={port=53,processes={'unbound','local-unbound'}}})
        essentials[#essentials+1]='dns'
    end
    if ssh then
        add({name='ssh-keys',type='oneshot',command={'/usr/bin/ssh-keygen','-A'},requires={'runtime','boot-preflight'},guard='root_guard'})
        add({name='ssh-check',type='oneshot',command={'/usr/sbin/sshd','-t'},requires={'ssh-keys'},guard='root_guard'})
        add({name='sshd',type='daemon',command={'/usr/sbin/sshd','-D'},requires={'network','firewall','ssh-check','ldconfig'},guard='exclusive_guard',
            readiness='condition',condition='tcp_ready',restart='on-failure',parameters={port=22,processes={'sshd'}}});essentials[#essentials+1]='sshd'
    end
    if ntp then
        add({name='ntp-state',type='oneshot',action='directories',guard='root_guard',requires={'runtime'},parameters={directories={{path='/var/db/ntp',mode=493}}}})
        add({name='ntpd',type='daemon',command={'/usr/sbin/ntpd','-n','-c','/etc/ntp.conf','-p','/var/run/ntpd.pid'},requires={'network','firewall','ntp-state','ldconfig'},
            guard='exclusive_guard',restart='on-failure',parameters={processes={'ntpd'}}});essentials[#essentials+1]='ntpd'
    end
    if logs then
        for _,d in ipairs(services)do
            if d.name=='dns'or d.name=='sshd'or d.name=='ntpd'then d.requires[#d.requires+1]='syslogd' end
        end
    end
    if seat then add({name='seatd',type='daemon',command={'/usr/local/bin/seatd'},requires={'runtime'},guard='exclusive_guard',restart='on-failure',
        readiness='condition',condition='socket_ready',parameters={path='/var/run/seatd.sock',processes={'seatd'}}})end
    for _,n in ipairs(swaps)do essentials[#essentials+1]=n;networking[#networking+1]=n end
    target('network-checkpoint',networking);target('minimal',{'runtime'});target('multiuser',essentials);target('gateway',{'multiuser'})
    if policy.desktop then
        fields(policy.desktop,{socket=true,user=true});assert(seat,'desktop system target requires seatd=true')
        local user=assert(os.user(assert(policy.desktop.user,'desktop user required')));assert(user.uid~=0,'root desktop forbidden')
        add({name='session',type='virtual',condition='external_desktop_ready',requires={'seatd'},parameters={socket=path(policy.desktop.socket),uid=user.uid},start_timeout=120})
        target('desktop',{'multiuser','session'})
    end
    return services
end
return M
