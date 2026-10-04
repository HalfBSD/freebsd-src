/*	$NetBSD: types.h,v 1.13 2000/06/13 01:02:44 thorpej Exp $	*/

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2009, Sun Microsystems, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met:
 * - Redistributions of source code must retain the above copyright notice, 
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice, 
 *   this list of conditions and the following disclaimer in the documentation 
 *   and/or other materials provided with the distribution.
 * - Neither the name of Sun Microsystems, Inc. nor the names of its 
 *   contributors may be used to endorse or promote products derived 
 *   from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Scalar types and allocation helpers for XDR serialization
 */
#ifndef _RPC_TYPES_H
#define _RPC_TYPES_H

#include <sys/types.h>
#include <sys/_null.h>

typedef int32_t bool_t;
typedef int32_t enum_t;

typedef uint32_t rpcprog_t;
typedef uint32_t rpcvers_t;
typedef uint32_t rpcproc_t;
typedef uint32_t rpcprot_t;
typedef uint32_t rpcport_t;
typedef  int32_t rpc_inline_t;

#define __dontcare__	-1

#ifndef FALSE
#	define FALSE	(0)
#endif
#ifndef TRUE
#	define TRUE	(1)
#endif

#ifdef _KERNEL
#ifdef _SYS_MALLOC_H_
MALLOC_DECLARE(M_XDR);
#endif
#define mem_alloc(bsize)	malloc(bsize, M_XDR,  M_WAITOK|M_ZERO)
#define mem_free(ptr, bsize)	free(ptr, M_XDR)
#else
#define mem_alloc(bsize)	calloc(1, bsize)
#define mem_free(ptr, bsize)	free(ptr)
#endif


#endif /* !_RPC_TYPES_H */
