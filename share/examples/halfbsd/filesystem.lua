-- SPDX-License-Identifier: BSD-2-Clause
local native = require('halfbsd.os')
local path = assert(arg[1], 'provide an output path in a directory you own')
local result, err = native.atomic_replace(path, 'HalfBSD\n', 0x180)
if not result then error(err.message) end
local metadata = assert(native.stat(path))
print(metadata.type, metadata.size, metadata.uid)
