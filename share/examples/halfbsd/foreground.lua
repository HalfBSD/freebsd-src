-- SPDX-License-Identifier: BSD-2-Clause
-- Run explicitly; this example is never part of the boot sequence.
local proc = require('proc')
local event = require('halfbsd.event')
local log = assert(require('halfbsd.log').new({service='example'}))
local loop <close> = assert(event.new())
local child = assert(proc.spawn({'/bin/sleep', '60'}, {new_session=true}))
assert(log:write('info', 'Child created', {pid=child.pid}))
loop:watch_process(child, function(e)
    assert(log:write('notice', 'Child exited', {state=e.result.state, pid=e.result.pid}))
    loop:stop()
end)
loop:watch_signal('TERM', function()
    local result, err = child:terminate(5, true)
    if not result then assert(log:write('error', err.message, err)) end
    loop:stop()
end)
local result, err = loop:run()
if not result then error(err.message) end
assert(proc.shutdown(5))
