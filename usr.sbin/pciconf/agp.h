/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2000 Doug Rabson
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

#ifndef _PCICONF_AGP_H_
#define _PCICONF_AGP_H_

/* Read-only PCI AGP capability decoding; no driver or ioctl interfaces. */
#define AGP_MODE_GET_SBA(x)		(((x) & 0x00000200U) >> 9)
#define AGP_MODE_GET_AGP(x)		(((x) & 0x00000100U) >> 8)
#define AGP_MODE_GET_MODE_3(x)		(((x) & 0x00000008U) >> 3)
#define AGP_MODE_GET_RATE(x)		((x) & 0x00000007U)
#define AGP_MODE_V2_RATE_1x		0x00000001
#define AGP_MODE_V2_RATE_2x		0x00000002
#define AGP_MODE_V2_RATE_4x		0x00000004
#define AGP_MODE_V3_RATE_4x		0x00000001
#define AGP_MODE_V3_RATE_8x		0x00000002
#define AGP_CAPID		0x0
#define AGP_STATUS		0x4

#endif /* _PCICONF_AGP_H_ */
