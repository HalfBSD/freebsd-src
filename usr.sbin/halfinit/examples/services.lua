-- SPDX-License-Identifier: BSD-2-Clause
-- Safe disposable example; no host daemons, networking or mounts are changed.
-- Start with: halfinit --config /usr/share/examples/halfbsd/services.lua \
--     --runtime-dir /absolute/private/runtime-directory
return {
    target = 'demo',
    shutdown_timeout = 5,
    services = {
        { name = 'prepare', type = 'oneshot',
          command = {'/bin/echo', 'Preparing disposable services'} },
        { name = 'worker', type = 'daemon', requires = {'prepare'},
          command = {'/usr/bin/lua', '-E', '/usr/lib/halfbsd/fixture.lua', 'daemon', '0.05'},
          readiness = 'notify', restart = 'on-failure',
          restart_limit = 3, restart_delay = 1, stop_timeout = 1 },
        { name = 'demo', type = 'target', requires = {'worker'} },
    },
}
