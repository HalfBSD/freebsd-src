-- SPDX-License-Identifier: BSD-2-Clause
-- Explicit post-boot check only: never starts, stops or adopts system services.
-- /usr/bin/lua -E /usr/src/tests/halfbsd/phase4/check-live.lua
package.path=(arg[1]or'/usr/lib/halfbsd')..'/?.lua'
local json=require('json');local native=require('halfbsd.os')
local function checked(value,err)
    if value==nil then error(type(err)=='table'and json.encode(err)or tostring(err),2)end
    return value
end
local rows=checked(require('client').exchange('/var/run/halfbsd/control.sock',{version=1,command='status'}))
local states={};for _,row in ipairs(rows)do states[row.name]=row.state end
for _,name in ipairs({'rootfs','zfs','graphics','runtime','loopback','firewall','network','dns','sshd','ntpd','syslogd','devd','seatd','multiuser'})do
    assert(states[name]=='READY',name..' is '..tostring(states[name])..', expected READY')
    print('PASS live '..name..' READY')
end
local providers=require('adapters').attach({},native,require('halfbsd.ipc'))
local event=require('halfbsd.event')
local function probe(name,fn,parameters)
    local record={};local loop=checked(event.new())
    local ok,err=xpcall(function()
        local function check()
            if fn(parameters,record)then loop:stop()
            else checked(loop:watch_timer(0.01,check))end
        end
        checked(loop:watch_timer(5,function()error(name..' readiness deadline')end))
        check();checked(loop:run())
    end,function(e)return type(e)=='table'and json.encode(e)or tostring(e)end)
    providers.release(record);loop:close();assert(ok,err)
    print('PASS live '..name)
end
probe('DNS localhost UDP answer and TCP listener',providers.dns_ready,{port=53})
probe('SSH TCP listener',providers.tcp_ready,{port=22})
probe('seatd socket permissions and connection',providers.seat_ready,{path='/var/run/seatd.sock'})
print('PASS live multiuser status/listener checks; external DNS, DNSSEC, SSH login, NTP synchronization and shutdown need separate checks')
