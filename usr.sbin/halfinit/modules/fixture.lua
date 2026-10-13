-- SPDX-License-Identifier: BSD-2-Clause
-- Disposable simulation service. Never invokes real host startup operations.
local event=require('halfbsd.event');local native=require('halfbsd.os');local proc=require('proc')
local mode=arg[1]or'daemon';local delay=tonumber(arg[2])or 0.01
local loop <close> = assert(event.new());local channel
local number=native.getenv('HALFINIT_READY_FD')
if number then channel=assert(proc.fd(tonumber(number)))end
if mode=='stubborn'then assert(native.signal_disposition('TERM','ignore'))
else assert(loop:watch_signal('TERM',function()loop:stop()end))end
-- Independent lifetime bound protects the host if a test supervisor crashes.
assert(loop:watch_timer(20,function()loop:stop()end))
assert(loop:watch_timer(delay,function()
    if mode=='fail' then os.exit(23,true)end
    if channel then
        local value,err=channel:write('READY\n');if not value then error(err.message)end
        channel:close();channel=nil
    end
    if mode=='oneshot' then loop:stop()
    elseif mode=='crash'then assert(loop:watch_timer(0.05,function()os.exit(17,true)end))end
end))
local result,err=loop:run();if not result then error(err.message)end
