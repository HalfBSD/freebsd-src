-- SPDX-License-Identifier: BSD-2-Clause
-- PID 1 owns the handoff pipe; the existing supervisor owns all service children.
local M={}
local critical={'devices','runtime','loopback','firewall'}
function M.new(native,proc,number,checkpoint)
    local identity=native.identity()
    assert(identity.euid==0 and identity.ppid==1,'boot handoff requires root directly launched by PID 1')
    assert(number==3,'PID 1 boot handoff uses descriptor 3')
    local root
    for _,mount in ipairs(assert(native.mounts()))do if mount.path=='/'then root=mount end end
    assert(root and root.type=='zfs','initial Phase IV integration requires a kernel-mounted ZFS root; use RC for other layouts')
    return setmetatable({fd=assert(proc.fd(number)),sent=false,checkpoint=checkpoint},{__index=M})
end
-- Retain the original failure across dependency propagation and cleanup so a
-- console photograph taken at the recovery prompt still includes its cause.
function M:observe(item,record)
    if self.first_failure or item.state~='FAILED'then return end
    local result=record.result or{}
    self.first_failure={service=item.service,reason=tostring(item.reason),
        exit_status=result.exit_code,signal=result.signal,
        output=(record.output_buffer or''):sub(1,4096)}
end
function M:summary()
    if not self.first_failure then return ''end
    return '[halfinit boot] FIRST FAILURE '..require('json').encode(self.first_failure)..'\n'
end
function M:check(engine,control_ready)
    if self.sent then return end
    local ready=control_ready and not engine.shutting
    local required={}
    for _,name in ipairs(critical)do required[#required+1]=name end
    required[#required+1]=engine.config.target
    for _,name in ipairs(required)do
        local r=assert(engine.records[name],'boot configuration lacks critical service '..name)
        if r.state=='FAILED'or r.state=='SKIPPED'then self:fail();engine:shutdown();return end
        if r.state~='READY'then ready=false end
    end
    if ready then
        local message=self.checkpoint and'NETWORK\n'or'READY\n'
        assert(self.fd:write(message)==#message,'PID 1 readiness handoff failed')
        self.sent=true;self.fd:close()
    end
end
function M:fail()
    if self.sent then return end
    self.fd:write('FAIL\n');self.fd:close();self.sent=true
end
function M.network(engine)
    if engine.shutting then return end
    local r=engine.records.network
    if not r or not r.wanted or engine.holds.network or engine.blocked.network then return end
    local available=engine.providers.route_ready(r.definition.parameters or{})
    if r.state=='READY'and not available then engine:failure(r,'network address/default route lost')
    elseif r.state=='FAILED'and available then engine:start({'network'})end
end
return M
