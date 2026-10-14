-- SPDX-License-Identifier: BSD-2-Clause
-- INSTALL IN A DISPOSABLE VM ONLY. This fixture reboots its host!
local proc=require('proc');local native=require('halfbsd.os');local event=require('halfbsd.event')
assert(native.identity().euid==0,'VM fixture requires root')
assert(native.getenv('HALFBSD_PHASE4_DISPOSABLE_VM')=='YES','explicit disposable VM fixture authorization missing')
io.stdout:setvbuf('no')
print('HALFBSD_PHASE4_VM_READY monotonic='..assert(native.clock()))
local loop=assert(event.new())
assert(loop:watch_timer(5,function()loop:stop()end));assert(loop:run());loop:close()
local result,err=proc.run({'/sbin/shutdown','-r','now'})
assert(result,err and err.message)
assert(result.exit_code==0,'VM shutdown request failed')
