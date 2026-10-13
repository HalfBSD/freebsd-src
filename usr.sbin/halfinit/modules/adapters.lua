-- SPDX-License-Identifier: BSD-2-Clause
-- Native boot adapters, attached to the existing supervisor provider registry.
local json=require('json')
local M={}
local function mounted(os,path)
    for _,row in ipairs(assert(os.mounts()))do if row.path==path then return row end end
end
local function same_mount(row,p)
    return row and row.type==p.options.fstype and row.source==p.options.from
end
function M.attach(providers,os,ipc)
    local probes=setmetatable({},{__mode='k'})
    local created=setmetatable({},{__mode='k'})
    local serial=0;local pending={};local deferred={}
    local function release(d)
        local probe=probes[d]
        if probe then probe.fd:close();probes[d]=nil end
    end
    providers.release=release
    providers.root_guard=function()
        assert(os.identity().euid==0,'system initialization requires root');return true
    end
    providers.exclusive_guard=function(p,d,engine)
        if not p.user_mode then providers.root_guard()end
        if d.foreground then
            local exists,err=os.exists(assert(p.path))
            assert(exists~=nil,err and err.message);assert(not exists,'Wayland readiness socket already exists')
        end
        local owned={[os.identity().pid]=true}
        for _,child in ipairs(engine.proc.active())do owned[child.pid]=true end
        local inventory=assert(os.processes());local jail
        for _,process in ipairs(inventory)do if process.pid==os.identity().pid then jail=process.jail end end
        assert(jail~=nil,'manager is missing from process inventory')
        for _,process in ipairs(inventory)do
            if process.jail==jail and not owned[process.pid]then
                for _,name in ipairs(p.processes or{})do
                    assert(process.command~=name or(name=='halfinit'and process.uid~=os.identity().euid),'already running outside this manager: '..name)
                end
            end
        end
        return true
    end
    providers.root_writable=function(p)
        providers.root_guard()
        local root=assert(mounted(os,'/'),'kernel root mount is missing')
        if root.readonly then
            assert(p.update,'root is read-only; explicit root_writable permission is required')
            assert(os.mount({fstype=root.type,fspath='/',from=root.source},0x10000)) -- MNT_UPDATE
            assert(not assert(mounted(os,'/')).readonly,'root remount remained read-only')
        end
        return true
    end
    providers.local_mount=function(p,d)
        providers.root_guard()
        local row=mounted(os,p.options.fspath)
        if row then
            assert(same_mount(row,p),'mount point is already occupied by another filesystem')
            assert(row.readonly==(p.readonly or false),'mounted filesystem has different read-only policy')
            return true
        end
        assert(os.mount(p.options,p.readonly and 1 or 0))
        created[d]=p
        assert(same_mount(mounted(os,p.options.fspath),p),'mount did not appear in kernel inventory')
        return true
    end
    providers.unmount_owned=function(p,d,engine)
        local owned=created[d]
        if not owned then return true end
        if engine and(engine.shutting or engine.finalizing)then
            if not deferred[d]then pending[#pending+1]=d;deferred[d]=true end
            return true
        end
        local row=mounted(os,owned.options.fspath)
        if row then
            assert(same_mount(row,owned),'filesystem ownership changed; refusing to unmount')
            assert(os.unmount(owned.options.fspath,0))
        end
        created[d]=nil;return true
    end
    providers.finish=function()
        local errors={}
        for _,d in ipairs(pending)do
            local ok,err=pcall(providers.unmount_owned,d.parameters or{},d)
            if not ok or not err then errors[d.name]=type(err)=='table'and err.message or tostring(err)end
        end
        pending={};deferred={};return errors
    end
    providers.fsck_guard=function(p)
        providers.root_guard()
        local row=mounted(os,p.options.fspath)
        for _,entry in ipairs(assert(os.mounts()))do
            assert(entry.source~=p.options.from or entry.path==p.options.fspath,'fsck source is mounted elsewhere')
        end
        if row then assert(same_mount(row,p),'unexpected mounted filesystem');return 'ready'end
        return true
    end
    providers.interface_up=function(p)
        providers.root_guard();assert(os.interface_up(p.interface));return true
    end
    providers.interface_ready=function(p)
        for _,row in ipairs(assert(os.interfaces()))do
            if row.name==p.interface and row.up and row.family=='inet'and row.address~='0.0.0.0'then return true end
        end
        return false
    end
    providers.route_ready=function(p)
        local address=providers.interface_ready(p)
        if not address then return false end
        for _,row in ipairs(assert(os.default_routes()))do if row.interface==p.interface then return true end end
        return false
    end
    providers.exact_output=function(p,d,text)
        assert(text:match('^%s*(.-)%s*$')==p.expected,'unexpected kernel command output');return true
    end
    providers.firewall_baseline=function(p,d,text)
        local count=0
        for line in text:gmatch('[^\n]+')do
            count=count+1
            assert(line:match('^65535%s+set%s+31%s+deny%s+ip%s+from%s+any%s+to%s+any%s*$'),
                'IPFW has existing policy or an accepting default; refuse to replace it')
        end
        assert(count==1,'IPFW kernel baseline is not the expected single deny rule');return true
    end
    providers.firewall_policy=function(p,d,text)
        local lines={}
        for line in text:gmatch('[^\n]+')do
            local number,set,body=line:match('^(%d+)%s+set%s+(%d+)%s+(.+)$')
            if tonumber(number)==65534 or tonumber(number)==65535 then assert(set=='31','deny guard must be in immutable set 31')end
            if p.load and tonumber(number)~=65534 and tonumber(number)~=65535 then assert(set=='0'and p.rules[tostring(tonumber(number))],'unexpected IPFW rule')end
            assert(number,'invalid IPFW readback');number=tonumber(number)
            lines[number]=lines[number]or{};lines[number][#lines[number]+1]=body
        end
        assert(lines[65534]and #lines[65534]==1 and lines[65534][1]=='deny ip from any to any','immutable IPFW deny guard is missing')
        assert(lines[65535]and #lines[65535]==1 and lines[65535][1]=='deny ip from any to any','default deny is missing')
        if p.load then
            for number,expected in pairs(p.rules)do
                assert(lines[tonumber(number)]and #lines[tonumber(number)]==1 and lines[tonumber(number)][1]==expected,
                    'IPFW rule readback differs at '..number)
            end
        end
        return true
    end
    providers.anchor_ready=function(p)
        local st=assert(os.stat(p.anchor))
        assert(st.type=='file'and st.uid==p.uid and(st.mode&0x12)==0,'unsafe or incorrectly owned DNSSEC anchor')
        local f <close> = assert(io.open(p.anchor,'r'));local text=f:read(1048577)
        assert(#text<=1048576 and(text:find('DNSKEY',1,true)or text:find(' DS ',1,true)),'DNSSEC anchor contains no trust record')
        return true
    end
    providers.tcp_ready=function(p)
        local ok,err=ipc.tcp_probe(p.address or'127.0.0.1',p.port)
        if ok==nil then error(err)end;return ok
    end
    providers.anchor_init_guard=function(p,d,engine)
        providers.exclusive_guard(p,d,engine)
        local st,err=os.stat(p.anchor)
        if not st then assert(err.name=='ENOENT',err);return true end
        providers.anchor_ready(p);return 'ready'
    end
    providers.dns_ready=function(p,d)
        local probe=probes[d]
        if probe and os.clock()-probe.time>0.5 then release(d);probe=nil end
        if not probe then
            serial=(serial+1)%65536
            local fd=assert(ipc.udp('127.0.0.1',p.port or 53))
            probe={fd=fd,time=os.clock(),id=serial};probes[d]=probe
            probe.question='\9localhost\0'..string.pack('>I2I2',1,1)
            local query=string.pack('>I2I2I2I2I2I2',serial,0x100,1,0,0,0)..probe.question
            assert(fd:write(query));return false
        end
        local bytes,err=probe.fd:read(1024)
        if not bytes then
            if not err or err.name~='EAGAIN'then release(d)end
            return false
        end
        if #bytes<12 then return false end
        local id,flags,questions,answers=string.unpack('>I2I2I2I2',bytes)
        if id~=probe.id or(flags&0xfa0f)~=0x8000 or questions~=1 or answers==0 or
            bytes:sub(13,12+#probe.question)~=probe.question then return false end
        local ok,why=ipc.tcp_probe('127.0.0.1',p.port or 53)
        if ok==nil then error(why)end
        if ok then release(d);return true end
        return false
    end
    providers.user_session_guard=function(p)
        local uid=os.identity().euid;assert(uid~=0,'desktop sessions must start as the logged-in user')
        local st=assert(os.stat(p.runtime))
        assert(st.type=='directory'and st.uid==uid and(st.mode&0x3f)==0,'XDG runtime directory must be private and user-owned')
        local tty=assert(os.terminal());assert(tty.tty and tty.foreground,'desktop launch requires the existing foreground login terminal')
        assert(not tty.stop_background_output,'desktop manager console logging requires terminal TOSTOP disabled')
        return true
    end
    providers.external_desktop_ready=function(p,d)
        local st=os.stat(p.socket)
        if not st then return false end
        assert(st.type=='socket'and st.uid==p.uid and p.uid~=0,'foreign desktop manager socket')
        local probe=probes[d]
        if probe and os.clock()-probe.time>1 then release(d);probe=nil end
        if not probe then
            local fd,err=ipc.connect(p.socket,0)
            if not fd then return false end
            probe={fd=fd,time=os.clock(),text=''};probes[d]=probe
            local request=json.encode({version=1,command='status',service='compositor'})..'\n'
            local n=fd:write(request)
            if n~=#request then release(d)end
            return false
        end
        local bytes,err=probe.fd:read(4096)
        if not bytes then if not err or err.name~='EAGAIN'then release(d)end;return false end
        probe.text=probe.text..bytes;assert(#probe.text<=65536,'desktop status too large')
        local line=probe.text:match('^(.-)\n')
        if not line then return false end
        local response=json.decode(line);release(d)
        return response.version==1 and response.ok==true and type(response.data)=='table'and response.data.state=='READY'
    end
    return providers
end
return M
