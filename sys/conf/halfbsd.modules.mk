# Default HalfBSD amd64 workstation modules. Keep hardware-driver choices
# separate from source removal: excluded implementations remain available to
# explicit MODULES_OVERRIDE, MODULES_EXTRA, or ALL_MODULES builds.
# External module Makefiles do not include this file.
# Filter the existing candidate set, respecting feature/firmware/arch choices.
# Keep ZFS dependencies even when a kernel does not compile them in.

HALFBSD_MODULES.top= \
	acl_nfs4 acpi aesni ahci alq amdgpio amdsmn amdsmu amdtemp asmc ath ath_dfs \
	ath_hal ath_hal_ar5210 ath_hal_ar5211 ath_hal_ar5212 ath_hal_ar5416 ath_hal_ar9300 \
	ath_main ath_rate axgbe backlight bce bge bhnd blake2 bnxt bridgestp bwi bwn bxe \
	bytgpio cam carp cc ccp cd9660 cd9660_iconv chromebook_platform chvgpio coretemp \
	cpuctl cpufreq crypto cryptodev ctl cuse cxgbe dummynet efirt em enic et evdev \
	fdescfs filemon firmware fusefs geom gpio hid hwpmc i2c iavf ice ice_ddp ichwd \
	if_bridge if_disc if_enc if_epair if_gif if_gre if_lagg if_ovpn if_stf if_tuntap \
	if_vlan if_vxlan if_wg iflib igc intelspi io ipdivert ipfw ipfw_nat ipfw_nat64 \
	ipfw_nptv6 ipfw_pmod ipsec isci itwd iwlwifi iwm iwn iwnfw iwx ix ixl ixv jme \
	kbdmux khelp ksyms libalias libiconv libmchain lindebugfs linprocfs linsysfs \
	linux linux64 linux_common linuxkpi linuxkpi_hdmi linuxkpi_video linuxkpi_wlan \
	md mdio mem mgb mii mmc mmcsd mpi3mr mpr mps mqueue msdosfs msdosfs_iconv msk \
	nctgpio ncthwm netgraph netlink nfe nlsysevent nmdm nullfs nvd nvme nvram \
	opensolaris ossl p2sb padlock padlock_rng pchtherm procfs pseudofs pt puc \
	qat qat_c2xxx qat_c2xxxfw qatfw qlnx ral ralfw random_fortuna rdrand_rng \
	rdseed_rng re rtsx rtw88 rtw89 rtwn rtwn_pci rtwn_usb rtwnfw scc sctp \
	sdhci sdhci_acpi sdhci_pci sdio sem sfxge siftr  sound spi superio \
	sysvipc tarfs tcp tests tmpfs tpm uart udf udf_iconv ufs uinput unionfs usb \
	videomode virtio vkbd vmd vmm wdatwd wlan wlan_acl wlan_amrr wlan_ccmp wlan_gcmp \
	wlan_rssadapt wlan_tkip wlan_wep wlan_xauth xdr xz zfs zlib

HALFBSD_MODULES.geom= \
	geom_cache geom_eli geom_label geom_mirror geom_mountver geom_nop geom_part \
	geom_union geom_uzip geom_zero

HALFBSD_MODULES.sound= \
	driver dummy hda hdsp hdspe uaudio

HALFBSD_MODULES.usb= \
	usb ehci ohci uhci xhci mtw rum run runfw rsu rsufw uath upgt ural zyd urtw \
	atp uhid uhid_snes ukbd ums uep wmt wsp ugold uled usbhid ucom u3g uark ubsa \
	ubser uchcom ucycom udbc ufoma uftdi ugensa ulpt umb umct umcs umodem \
	umoscom uplcom uslcom uvscom i2ctinyusb cp2112 udl uether axe axge cdce \
	cdceem mos smsc udav ipheth muge ure urndis umass uacpi quirk

.if ${MACHINE_CPUARCH} == "amd64" && !defined(ALL_MODULES) && \
    !defined(MODULES_OVERRIDE)
_halfbsd_candidates:= ${SUBDIR}
SUBDIR=
.for _halfbsd_module in ${_halfbsd_candidates}
.if !empty(HALFBSD_MODULES.${HALFBSD_MODULE_GROUP}:M${_halfbsd_module})
SUBDIR+= ${_halfbsd_module}
.endif
.endfor
.undef _halfbsd_candidates
.endif
