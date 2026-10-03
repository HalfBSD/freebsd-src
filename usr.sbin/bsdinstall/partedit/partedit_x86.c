/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2011 Nathan Whitehorn
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

#include <sys/types.h>
#include <sys/sysctl.h>
#include <string.h>

#include "partedit.h"

/* EFI partition size in bytes */
#define	EFI_BOOTPART_SIZE	(260 * 1024 * 1024)

static const char *
x86_bootmethod(void)
{
	static char fw[255] = "";
	size_t len = sizeof(fw);
	int error;
	
	if (strlen(fw) == 0) {
		error = sysctlbyname("machdep.bootmethod", fw, &len, NULL, -1);
		if (error != 0)
			return ("");
	}

	return (fw);
}

const char *
default_scheme(void)
{
	return ("GPT");
}

int
is_scheme_bootable(const char *part_type)
{
	return (strcmp(x86_bootmethod(), "UEFI") == 0 &&
	    strcmp(part_type, "GPT") == 0);
}

int
is_fs_bootable(const char *part_type, const char *fs)
{
	return (is_scheme_bootable(part_type) &&
	    strcmp(fs, "freebsd-ufs") == 0);
}

size_t
bootpart_size(const char *scheme)
{
	return (is_scheme_bootable(scheme) ? EFI_BOOTPART_SIZE : 0);
}

const char *
bootpart_type(const char *scheme, const char **mountpoint)
{
	if (!is_scheme_bootable(scheme))
		return (NULL);
	*mountpoint = "/boot/efi";
	return ("efi");
}

const char *
bootcode_path(const char *part_type)
{
	return (NULL);
}

const char *
partcode_path(const char *part_type, const char *fs_type)
{
	return (NULL);
}
