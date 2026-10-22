#!/bin/sh

# SPDX-License-Identifier: BSD-2-Clause
#
#  Copyright (c) 2009 Douglas Barton
#  All rights reserved.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions
#  are met:
#  1. Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
#  2. Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
#
#  THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
#  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
#  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
#  ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
#  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
#  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
#  OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
#  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
#  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
#  OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
#  SUCH DAMAGE.

# halfinit owns all managed services; never evaluate RC configuration.
if [ "$#" -eq 1 ] && [ "$1" = -l ]; then
    exec /usr/sbin/halfsvc list
fi
if [ "$#" -ne 2 ]; then
    echo 'Usage: service NAME start|stop|restart|status|enable|disable (or service -l)' >&2
    exit 1
fi
service_name=$1
case "$service_name" in local_unbound|local-unbound|unbound) service_name=dns ;; esac
case "$2" in
start|stop|restart|status|enable|disable) exec /usr/sbin/halfsvc "$2" "$service_name" ;;
*) echo 'service: unsupported action; use halfsvc --help' >&2; exit 1 ;;
esac
