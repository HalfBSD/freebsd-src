/* $NetBSD: t_xdr.c,v 1.1 2011/01/08 06:59:37 pgoyette Exp $ */

/*
 * Copyright (c) 2008 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Ben Harris.
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
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*-
 * Copyright (c) 2001 Ben Harris
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
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/cdefs.h>

#include <rpc/types.h>
#include <rpc/xdr.h>

#include <string.h>

#include <atf-c.h>


char xdrdata[] = {
	0x3f, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* double 1.0 */
	0x00, 0x00, 0x00, 0x01, /* enum smallenum 1 */
	0xff, 0xff, 0xfb, 0x2e, /* enum medenum -1234 */
	0x00, 0x12, 0xd6, 0x87, /* enum bigenum 1234567 */
};

ATF_TC(xdr);
ATF_TC_HEAD(xdr, tc)
{
	atf_tc_set_md_var(tc, "descr",
		"Checks encoding/decoding of doubles and enumerations");
}
ATF_TC_BODY(xdr, tc)
{
	XDR x;
	double d;
	enum_t s;
	enum_t m;
	enum_t b;
	char newdata[sizeof(xdrdata)];

	xdrmem_create(&x, xdrdata, sizeof(xdrdata), XDR_DECODE);

	ATF_REQUIRE_MSG(xdr_double(&x, &d), "xdr_double DECODE failed");
	ATF_REQUIRE_EQ_MSG(d, 1.0, "double 1.0 decoded as %g", d);

	ATF_REQUIRE_MSG(xdr_enum(&x, &s), "xdr_enum DECODE failed");
	ATF_REQUIRE_EQ_MSG(s, 1, "1 decoded as %d", s);

	ATF_REQUIRE_MSG(xdr_enum(&x, &m), "xdr_enum DECODE failed");
	ATF_REQUIRE_EQ_MSG(m, -1234, "-1234 decoded as %d", m);

	ATF_REQUIRE_MSG(xdr_enum(&x, &b), "xdr_enum DECODE failed");
	ATF_REQUIRE_EQ_MSG(b, 1234567, "1234567 decoded as %d", b);

	xdr_destroy(&x);


	xdrmem_create(&x, newdata, sizeof(newdata), XDR_ENCODE);

	ATF_REQUIRE_MSG(xdr_double(&x, &d), "xdr_double ENCODE failed");
	ATF_REQUIRE_MSG(xdr_enum(&x, &s), "xdr_enum ENCODE failed");
	ATF_REQUIRE_MSG(xdr_enum(&x, &m), "xdr_enum ENCODE failed");
	ATF_REQUIRE_MSG(xdr_enum(&x, &b), "xdr_enum ENCODE failed");
	ATF_REQUIRE_MSG(memcmp(newdata, xdrdata, sizeof(xdrdata)) == 0,
		"xdr ENCODE result differs");

	xdr_destroy(&x);
}

ATF_TP_ADD_TCS(tp)
{
	ATF_TP_ADD_TC(tp, xdr);

	return atf_no_error();
}
