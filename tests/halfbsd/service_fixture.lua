-- SPDX-License-Identifier: BSD-2-Clause
-- Test-only foreground service. Every mode has a bounded lifetime.
local event = require('halfbsd.event')
local native = require('halfbsd.os')
local mode = assert(arg[1])
local loop <close> = assert(event.new())
local function say(text) io.stdout:write(text..'\n'); io.stdout:flush() end
if mode == 'once' then say('ONE SHOT'); os.exit(0, true) end
if mode == 'stubborn' then
    assert(native.signal_disposition('TERM', 'ignore'))
else
    assert(loop:watch_signal('TERM', function() say('STOPPED'); loop:stop() end))
end
-- Do not leave a persistent fixture behind if its parent test fails.
assert(loop:watch_timer(8, function() loop:stop() end))
local function ready()
    say('READY')
    if mode == 'crash' then
        assert(loop:watch_timer(0.05, function() os.exit(17, true) end))
    end
end
if mode == 'delayed' then
    assert(loop:watch_timer(0.1, ready))
else
    ready()
end
local result, err = loop:run()
if not result then error(err.message) end
