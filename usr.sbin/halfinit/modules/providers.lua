-- SPDX-License-Identifier: BSD-2-Clause
-- Explicit native initialization operations. System definitions are protected
-- from execution in Phase II; custom administrator tasks may select providers.
local M={}
function M.native()
    local os=require('halfbsd.os');local ipc=require('halfbsd.ipc')
    return {
        interface_discovery=function()assert(os.interfaces());return true end,
        network_ready=function(p)
            for _,interface in ipairs(assert(os.interfaces()))do
                if interface.up and not interface.loopback and interface.family=='inet' and interface.address~='0.0.0.0' and
                    (not p.interface or interface.name==p.interface)then return true end
            end
            return false
        end,
        root_ready=function()local st=os.stat('/');return st and st.type=='directory'end,
        exists=function(p)local ok,err=os.exists(assert(p.path));if ok==nil then error(err)end;return ok end,
        socket_ready=function(p)local ok,err=ipc.probe(assert(p.path));if ok==nil then error(err)end;return ok end,
        directories=function(p)
            for _,item in ipairs(p.directories or{})do
                local st,err=os.stat(item.path)
                if not st then assert(err.name=='ENOENT');assert(os.mkdir(item.path,item.mode or 0x1ed))
                else assert(st.type=='directory','non-directory prerequisite')end
                if item.mode then assert(os.chmod(item.path,item.mode))end
                if item.uid then assert(os.chown(item.path,item.uid,assert(item.gid)))end
            end
            return true
        end,
        mount=function(p)assert(os.mount(assert(p.options),p.flags or 0));return true end,
        unmount=function(p)assert(os.unmount(assert(p.path),p.flags or 0));return true end,
        manual=function()return false end,
    }
end
return M
