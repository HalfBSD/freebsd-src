-- SPDX-License-Identifier: BSD-2-Clause
local proc = require('proc')
-- Simultaneously feeds stdin and drains stdout/stderr without a shell.
local result, err = proc.run({'/bin/cat'}, {
    input='Hello from Lua\n', stdout='pipe', stderr='pipe',
    timeout=5, output_limit=1024*1024,
})
if not result then error(err.operation .. ': ' .. err.message) end
assert(result.success)
io.write(result.stdout)
