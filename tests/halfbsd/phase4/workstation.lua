-- SPDX-License-Identifier: BSD-2-Clause
package.path=assert(arg[1])..'/?.lua'
local console=require('console');local config=require('config');local dependency=require('dependency')
local function reject(fn)assert(not pcall(fn),'unsafe input accepted')end
local lookup=function(name)if name=='root'or name=='charlie'then return {shell='/bin/sh'}end end
assert(console.select(nil,nil,lookup)=='root')
assert(console.select({user='charlie'},nil,lookup)=='charlie')
assert(console.select({},' charlie\n',lookup)=='charlie')
assert(console.select({},'login\n',lookup)=='')
assert(console.select({autologin=false},nil,lookup)=='')
reject(function()console.select({},'missing',lookup)end)
reject(function()console.select({user='missing'},nil,lookup)end)
reject(function()console.select({},'root;id',lookup)end)
reject(function()console.validate({user=false})end)
reject(function()console.select({},'root',function()return {shell='/usr/sbin/nologin'}end)end)
for _,value in ipairs({-1,31,0/0})do reject(function()console.validate({select_timeout=value})end)end
assert(console.legacy('console_user="charlie" # preferred\n',nil)=='charlie')
assert(console.legacy("console_user='root'\nconsole_user=charlie\n",nil)=='charlie')
assert(console.legacy('# console_user=$(unsafe)\n',nil)==nil)
for _,line in ipairs({'console_user="$(id)"','console_user="root"; id','console_user=$name','console_user=root;id'})do
    reject(function()console.legacy(line,nil)end)
end
print('PASS console defaults, selection, disabled/invalid accounts and literal-only RC migration')
local native={exists=function()return true end,user=function()return {uid=59,gid=59}end,
    fstab=function()return {}end,interfaces=function()error('offline profile must not discover Ethernet')end}
local previous=package.loaded['halfbsd.os'];package.loaded['halfbsd.os']=native
local file=os.tmpname()
local function load(text)
    local out=assert(io.open(file,'w'));out:write(text);out:close()
    return config.load(file)
end
local ok,problem=pcall(function()
    local c=load("return {profile='maintenance',boot={zfs=true,amdgpu=true,firewall={ownership='load'}},services={{name='unsafe',type='oneshot',command={'/bin/false'},enabled=true}}}")
    assert(c.target=='maintenance'and c.checkpoint=='maintenance'and not c.registry.unsafe)
    local plan=dependency.plan(c.registry,{[c.target]=true})
    for _,name in ipairs({'rootfs','zfs','graphics','runtime','loopback','firewall','maintenance'})do assert(plan.selected[name],name)end
    for _,name in ipairs({'dhcp-em0','dhcp-lo0','network','dns','sshd','ntpd','seatd','devd'})do assert(not plan.selected[name],name)end
    reject(function()load("return {profile='typo',services={}}")end)
    reject(function()load("return {profile=false,services={}}")end)
    native.interfaces=function()return {{name='em0',family='link',link_type=6}}end
    c=load("return {profile='network',boot={},services={{name='unsafe',type='oneshot',command={'/bin/false'}}}}")
    assert(c.checkpoint=='network'and not c.registry.unsafe)
    plan=dependency.plan(c.registry,{[c.target]=true})
    assert(plan.selected.network and not plan.selected.dns and not plan.selected.devd)
end)
package.loaded['halfbsd.os']=previous;os.remove(file);assert(ok,problem)
print('PASS offline/network profiles select existing graph without extra services or Ethernet requirement')
