/*-
 * Copyright (c) 2003 Peter Wemm <peter@FreeBSD.org>
 * Copyright (c) 2011-2012 Spectra Logic Corporation
 * Copyright (c) 2013 Roger Pau Monne <royger@FreeBSD.org>
 * All rights reserved.
 *
 * This software was developed by Cherry G. Mathew <cherry@zyx.in>
 * under sponsorship from Spectra Logic Corporation.
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
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

#include <machine/asmacros.h>
#include <machine/psl.h>
#include <machine/pmap.h>
#include <machine/specialreg.h>

#include "assym.inc"

#define	VTOP(x)		((x) - KERNBASE)
#define	ENTRY_SIZE	8 /* sizeof(uint64_t) */

#define	GDT_CODE	0x08
#define	GDT_DATA	0x10

/*
 * PVH loaders identify the 32-bit entry using ELF note type 18.
 * The note owner is part of the boot ABI, including on Firecracker.
 */
.section .note.pvh,"a",@note
.p2align 2
	.long	4, 4, 18
	.asciz	"Xen"
	.long	VTOP(pvh_start32)

	.text
/* PVH entry point. */
	.code32
ENTRY(pvh_start32)

	/* Load flat GDT */
	movl	$VTOP(gdtdesc32), %eax
	lgdt	(%eax)
	jmp	$GDT_CODE, $VTOP(reload_cs)

reload_cs:
	movw	$GDT_DATA, %ax
	movw	%ax, %ds
	movw	%ax, %es
	movw	%ax, %ss

	movl	$VTOP(bootstack), %esp

	/* Don't trust what the loader gives for eflags. */
	pushl	$PSL_KERNEL
	popfl

	/*
	 * Create the page tables.
	 * The first 1GB is mapped using 2MB entries.
	 */
	movl	$0, %eax
pgbuild:
	cmp	$(PAGE_SIZE/ENTRY_SIZE), %eax
	jae	pgbuild_done

	/* PT4[i] = VTOP(&PT3[0]) | PG_V | PG_RW | PG_U */
	movl	$VTOP(PT4), %ecx
	movl	$VTOP(PT3), %edx
	orl	$(PG_V | PG_RW | PG_U), %edx
	movl	%edx, (%ecx,%eax,ENTRY_SIZE)

	/* PT3[i] = VTOP(&PT2[0]) | PG_V | PG_RW | PG_U */
	movl	$VTOP(PT3), %ecx
	movl	$VTOP(PT2), %edx
	orl	$(PG_V | PG_RW | PG_U), %edx
	movl	%edx, (%ecx,%eax,ENTRY_SIZE)

	/* PT2[i] = i * 2MiB | PG_V | PG_RW | PG_PS | PG_U */
	movl	$VTOP(PT2), %ecx
	movl	%eax, %edx
	shll	$PDRSHIFT, %edx
	orl	$(PG_V | PG_RW | PG_PS | PG_U), %edx
	movl	%edx, (%ecx,%eax,ENTRY_SIZE)

	inc	%eax
	jmp	pgbuild

pgbuild_done:
	/* Turn on EFER.LME */
	movl	$MSR_EFER, %ecx
	rdmsr
	orl	$EFER_LME, %eax
	wrmsr

	/* Turn on PAE */
	movl	%cr4, %eax
	orl	$CR4_PAE, %eax
	movl	%eax, %cr4

	/* Set %cr3 for PT4 */
	movl	$VTOP(PT4), %eax
	movl	%eax, %cr3

	/* Turn on paging (implicitly sets EFER.LMA) */
	movl	%cr0, %eax
	orl	$CR0_PG, %eax
	movl	%eax, %cr0

	/* Now we're in compatibility mode. Set %cs for long mode */
	movl	$VTOP(gdtdesc), %eax
	lgdt	(%eax)
	ljmp	$GDT_CODE, $VTOP(longmode)

	.code64
longmode:
	/* We're still running V=P, jump to entry point */
	movq	$bootstack, %rsp
	movq	$start_kernel, %rax
	pushq	%rax
	ret

start_kernel:
	/*
	 * Pass %ebx as the argument to hammer_time_pvh, it contains
	 * the startup info.
	 */
	movq	%rbx, %rdi
	call	hammer_time_pvh
	movq	%rax, %rsp
	call	mi_startup

	/* NOTREACHED */
0:	hlt
	jmp 	0b

/* Space for initial page tables */
	.data
	.p2align 12,0x40
PT4:
	.space	0x1000
PT3:
	.space	0x1000
PT2:
	.space	0x1000

/* 64bit GDT */
gdtdesc:
	.word	gdtend - gdt - 1
	.long	VTOP(gdt)		# low
	.long	0			# high
gdt:
	.long	0			# null descriptor
	.long	0
	.long	0x00000000		# %cs
	.long	0x00209800
	.long	0x00000000		# %ds
	.long	0x00008000
gdtend:

/* 32bit GDT */
gdtdesc32:
	.word	gdt32end - gdt32 - 1
	.long	VTOP(gdt32)
	.long	0
gdt32:
	.long	0			# null descriptor
	.long	0
	.long	0x0000ffff		# %cs
	.long	0x00cf9a00
	.long	0x0000ffff		# %ds, %es, %ss
	.long	0x00cf9200
gdt32end:
