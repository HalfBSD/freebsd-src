/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2004 Christian Limpach.
 * Copyright (c) 2004-2006,2008 Kip Macy
 * Copyright (c) 2008 The NetBSD Foundation, Inc.
 * Copyright (c) 2013 Roger Pau Monné <roger.pau@citrix.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/cdefs.h>
#include "opt_kstack_pages.h"

#include <sys/param.h>
#include <sys/boot.h>
#include <sys/kernel.h>
#include <sys/linker.h>
#include <sys/reboot.h>
#include <sys/systm.h>
#include <sys/tslog.h>

#include <vm/vm.h>
#include <vm/pmap.h>
#include <vm/vm_param.h>

#include <machine/clock.h>
#include <machine/cpu.h>
#include <machine/md_var.h>
#include <machine/metadata.h>
#include <machine/pc/bios.h>
#include <x86/acpica_machdep.h>
#include <x86/init.h>
#include <x86/pvh.h>

extern uint64_t hammer_time(uint64_t, uint64_t);
uint64_t hammer_time_pvh(vm_paddr_t);

static void pvh_parse_preload_data(uint64_t);
static void pvh_parse_memmap(vm_paddr_t *, int *);
static void pvh_clock_init(void);
static void pvh_delay(int);

static struct init_ops pvh_init_ops = {
	.parse_preload_data = pvh_parse_preload_data,
	.early_clock_source_init = pvh_clock_init,
	.early_delay = pvh_delay,
	.parse_memmap = pvh_parse_memmap,
};

static struct pvh_start_info *start_info;

/* PVH guests have no legacy PIT. TSC calibration uses hypervisor CPUID. */
static void
pvh_clock_init(void)
{
}

static void
pvh_delay(int n)
{
	uint64_t end;

	if (tsc_freq == 0)
		panic("PVH requires a calibrated TSC");
	end = rdtsc() + tsc_freq * n / 1000000;
	while (rdtsc() < end)
		cpu_spinwait();
}

uint64_t
hammer_time_pvh(vm_paddr_t start_info_paddr)
{
	struct pvh_modlist_entry *mod;
	uint64_t physfree;

	start_info = (struct pvh_start_info *)(start_info_paddr + KERNBASE);
	/* No console or per-CPU state is available until hammer_time(). */
	if (start_info->magic != PVH_START_MAGIC || start_info->version < 1 ||
	    start_info->memmap_paddr == 0 || start_info->memmap_entries == 0)
		halt();

	/*
	 * Select the higher address to use as physfree: either after
	 * start_info, after the kernel, after the memory map or after any of
	 * the modules.  We assume enough memory to be available after the
	 * selected address for the needs of very early memory allocations.
	 */
	physfree = roundup2(start_info_paddr + sizeof(struct pvh_start_info),
	    PAGE_SIZE);
	physfree = MAX(roundup2((vm_paddr_t)_end - KERNBASE, PAGE_SIZE),
	    physfree);

	if (start_info->memmap_paddr != 0)
		physfree = MAX(roundup2(start_info->memmap_paddr +
		    start_info->memmap_entries *
		    sizeof(struct pvh_memmap_entry), PAGE_SIZE),
		    physfree);

	if (start_info->modlist_paddr != 0) {
		unsigned int i;

		if (start_info->nr_modules == 0) {
			halt();
		}
		mod = (struct pvh_modlist_entry *)
		    (start_info->modlist_paddr + KERNBASE);
		for (i = 0; i < start_info->nr_modules; i++)
			physfree = MAX(roundup2(mod[i].paddr + mod[i].size,
			    PAGE_SIZE), physfree);
	}

	/* Set the hooks for early functions that diverge from bare metal */
	init_ops = pvh_init_ops;

	/* Now we can jump into the native init function */
	return (hammer_time(0, physfree));
}

static void
pvh_parse_preload_data(uint64_t modulep __unused)
{
	vm_ooffset_t off;
	vm_paddr_t metadata;
	char *envp;

	TSENTER();
	if (start_info->modlist_paddr != 0) {
		struct pvh_modlist_entry *mod;

		mod = (struct pvh_modlist_entry *)
		    (start_info->modlist_paddr + KERNBASE);
		preload_metadata = (caddr_t)(mod[0].paddr + KERNBASE);

		/* Initialize preload_kmdp */
		preload_initkmdp(false);
		if (preload_kmdp == NULL) {
			panic("Unable to find kernel metadata");
		}

		metadata = MD_FETCH(preload_kmdp, MODINFOMD_MODULEP,
		    vm_paddr_t);
		off = mod[0].paddr + KERNBASE - metadata;

		preload_bootstrap_relocate(off);

		boothowto = MD_FETCH(preload_kmdp, MODINFOMD_HOWTO, int);
		envp = MD_FETCH(preload_kmdp, MODINFOMD_ENVP, char *);
		if (envp != NULL)
			envp += off;
		init_static_kenv(envp, 0);

		if (MD_FETCH(preload_kmdp, MODINFOMD_EFI_MAP, void *) != NULL)
			strlcpy(bootmethod, "UEFI", sizeof(bootmethod));
		else
			strlcpy(bootmethod, "BIOS", sizeof(bootmethod));
	} else {
		static char kenv_buffer[PAGE_SIZE];

		/* Provide a static kenv so the command line can be parsed. */
		init_static_kenv(kenv_buffer, sizeof(kenv_buffer));

		/* Parse the PVH command line. */
		if (start_info->cmdline_paddr != 0)
			boot_parse_cmdline_delim(
			    (char *)(start_info->cmdline_paddr + KERNBASE),
			    ", \t\n");
		strlcpy(bootmethod, "PVH", sizeof(bootmethod));
	}

	boothowto |= boot_env_to_howto();

	/* Use the ACPI tables supplied by the PVH loader. */
	acpi_set_root(start_info->rsdp_paddr);

	TSEXIT();
}

static void
pvh_parse_memmap_start_info(vm_paddr_t *physmap,
    int *physmap_idx)
{
	const struct pvh_memmap_entry *entries;
	size_t nentries;
	size_t i;

	/* Extract the memory map supplied by the PVH loader. */
	entries = (const struct pvh_memmap_entry *)
	    (start_info->memmap_paddr + KERNBASE);
	nentries = start_info->memmap_entries;

	/* Convert into E820 format and handle one by one. */
	for (i = 0; i < nentries; i++) {
		struct bios_smap entry;

		entry.base = entries[i].addr;
		entry.length = entries[i].size;

		/* PVH memory types use the E820 address range values. */
		entry.type = entries[i].type;

		bios_add_smap_entries(&entry, sizeof(entry), physmap, physmap_idx);
	}
}

static void
pvh_parse_memmap(vm_paddr_t *physmap, int *physmap_idx)
{
	if (start_info->version < 1 || start_info->memmap_paddr == 0 ||
	    start_info->memmap_entries == 0)
		panic("PVH loader did not supply a memory map");
	pvh_parse_memmap_start_info(physmap, physmap_idx);
}
