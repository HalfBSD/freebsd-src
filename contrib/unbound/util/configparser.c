/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 38 "util/configparser.y"

#include "config.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "util/configyyrename.h"
#include "util/config_file.h"
#include "util/net_help.h"
#include "sldns/str2wire.h"

int ub_c_lex(void);
void ub_c_error(const char *message);

static void validate_acl_action(const char* action);

/* these need to be global, otherwise they cannot be used inside yacc */
extern struct config_parser_state* cfg_parser;

#if 0
#define OUTYY(s)  printf s /* used ONLY when debugging */
#else
#define OUTYY(s)
#endif


#line 101 "util/configparser.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "configparser.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_SPACE = 3,                      /* SPACE  */
  YYSYMBOL_LETTER = 4,                     /* LETTER  */
  YYSYMBOL_NEWLINE = 5,                    /* NEWLINE  */
  YYSYMBOL_COMMENT = 6,                    /* COMMENT  */
  YYSYMBOL_COLON = 7,                      /* COLON  */
  YYSYMBOL_ANY = 8,                        /* ANY  */
  YYSYMBOL_ZONESTR = 9,                    /* ZONESTR  */
  YYSYMBOL_STRING_ARG = 10,                /* STRING_ARG  */
  YYSYMBOL_VAR_FORCE_TOPLEVEL = 11,        /* VAR_FORCE_TOPLEVEL  */
  YYSYMBOL_VAR_SERVER = 12,                /* VAR_SERVER  */
  YYSYMBOL_VAR_VERBOSITY = 13,             /* VAR_VERBOSITY  */
  YYSYMBOL_VAR_NUM_THREADS = 14,           /* VAR_NUM_THREADS  */
  YYSYMBOL_VAR_PORT = 15,                  /* VAR_PORT  */
  YYSYMBOL_VAR_OUTGOING_RANGE = 16,        /* VAR_OUTGOING_RANGE  */
  YYSYMBOL_VAR_INTERFACE = 17,             /* VAR_INTERFACE  */
  YYSYMBOL_VAR_PREFER_IP4 = 18,            /* VAR_PREFER_IP4  */
  YYSYMBOL_VAR_DO_IP4 = 19,                /* VAR_DO_IP4  */
  YYSYMBOL_VAR_DO_IP6 = 20,                /* VAR_DO_IP6  */
  YYSYMBOL_VAR_DO_NAT64 = 21,              /* VAR_DO_NAT64  */
  YYSYMBOL_VAR_PREFER_IP6 = 22,            /* VAR_PREFER_IP6  */
  YYSYMBOL_VAR_DO_UDP = 23,                /* VAR_DO_UDP  */
  YYSYMBOL_VAR_DO_TCP = 24,                /* VAR_DO_TCP  */
  YYSYMBOL_VAR_TCP_MSS = 25,               /* VAR_TCP_MSS  */
  YYSYMBOL_VAR_OUTGOING_TCP_MSS = 26,      /* VAR_OUTGOING_TCP_MSS  */
  YYSYMBOL_VAR_TCP_IDLE_TIMEOUT = 27,      /* VAR_TCP_IDLE_TIMEOUT  */
  YYSYMBOL_VAR_EDNS_TCP_KEEPALIVE = 28,    /* VAR_EDNS_TCP_KEEPALIVE  */
  YYSYMBOL_VAR_EDNS_TCP_KEEPALIVE_TIMEOUT = 29, /* VAR_EDNS_TCP_KEEPALIVE_TIMEOUT  */
  YYSYMBOL_VAR_SOCK_QUEUE_TIMEOUT = 30,    /* VAR_SOCK_QUEUE_TIMEOUT  */
  YYSYMBOL_VAR_CHROOT = 31,                /* VAR_CHROOT  */
  YYSYMBOL_VAR_USERNAME = 32,              /* VAR_USERNAME  */
  YYSYMBOL_VAR_DIRECTORY = 33,             /* VAR_DIRECTORY  */
  YYSYMBOL_VAR_LOGFILE = 34,               /* VAR_LOGFILE  */
  YYSYMBOL_VAR_PIDFILE = 35,               /* VAR_PIDFILE  */
  YYSYMBOL_VAR_MSG_CACHE_SIZE = 36,        /* VAR_MSG_CACHE_SIZE  */
  YYSYMBOL_VAR_MSG_CACHE_SLABS = 37,       /* VAR_MSG_CACHE_SLABS  */
  YYSYMBOL_VAR_NUM_QUERIES_PER_THREAD = 38, /* VAR_NUM_QUERIES_PER_THREAD  */
  YYSYMBOL_VAR_RRSET_CACHE_SIZE = 39,      /* VAR_RRSET_CACHE_SIZE  */
  YYSYMBOL_VAR_RRSET_CACHE_SLABS = 40,     /* VAR_RRSET_CACHE_SLABS  */
  YYSYMBOL_VAR_OUTGOING_NUM_TCP = 41,      /* VAR_OUTGOING_NUM_TCP  */
  YYSYMBOL_VAR_INFRA_HOST_TTL = 42,        /* VAR_INFRA_HOST_TTL  */
  YYSYMBOL_VAR_INFRA_LAME_TTL = 43,        /* VAR_INFRA_LAME_TTL  */
  YYSYMBOL_VAR_INFRA_CACHE_SLABS = 44,     /* VAR_INFRA_CACHE_SLABS  */
  YYSYMBOL_VAR_INFRA_CACHE_NUMHOSTS = 45,  /* VAR_INFRA_CACHE_NUMHOSTS  */
  YYSYMBOL_VAR_INFRA_CACHE_LAME_SIZE = 46, /* VAR_INFRA_CACHE_LAME_SIZE  */
  YYSYMBOL_VAR_NAME = 47,                  /* VAR_NAME  */
  YYSYMBOL_VAR_STUB_ZONE = 48,             /* VAR_STUB_ZONE  */
  YYSYMBOL_VAR_STUB_HOST = 49,             /* VAR_STUB_HOST  */
  YYSYMBOL_VAR_STUB_ADDR = 50,             /* VAR_STUB_ADDR  */
  YYSYMBOL_VAR_TARGET_FETCH_POLICY = 51,   /* VAR_TARGET_FETCH_POLICY  */
  YYSYMBOL_VAR_HARDEN_SHORT_BUFSIZE = 52,  /* VAR_HARDEN_SHORT_BUFSIZE  */
  YYSYMBOL_VAR_HARDEN_LARGE_QUERIES = 53,  /* VAR_HARDEN_LARGE_QUERIES  */
  YYSYMBOL_VAR_FORWARD_ZONE = 54,          /* VAR_FORWARD_ZONE  */
  YYSYMBOL_VAR_FORWARD_HOST = 55,          /* VAR_FORWARD_HOST  */
  YYSYMBOL_VAR_FORWARD_ADDR = 56,          /* VAR_FORWARD_ADDR  */
  YYSYMBOL_VAR_DO_NOT_QUERY_ADDRESS = 57,  /* VAR_DO_NOT_QUERY_ADDRESS  */
  YYSYMBOL_VAR_HIDE_IDENTITY = 58,         /* VAR_HIDE_IDENTITY  */
  YYSYMBOL_VAR_HIDE_VERSION = 59,          /* VAR_HIDE_VERSION  */
  YYSYMBOL_VAR_IDENTITY = 60,              /* VAR_IDENTITY  */
  YYSYMBOL_VAR_VERSION = 61,               /* VAR_VERSION  */
  YYSYMBOL_VAR_HARDEN_GLUE = 62,           /* VAR_HARDEN_GLUE  */
  YYSYMBOL_VAR_MODULE_CONF = 63,           /* VAR_MODULE_CONF  */
  YYSYMBOL_VAR_TRUST_ANCHOR_FILE = 64,     /* VAR_TRUST_ANCHOR_FILE  */
  YYSYMBOL_VAR_TRUST_ANCHOR = 65,          /* VAR_TRUST_ANCHOR  */
  YYSYMBOL_VAR_VAL_OVERRIDE_DATE = 66,     /* VAR_VAL_OVERRIDE_DATE  */
  YYSYMBOL_VAR_BOGUS_TTL = 67,             /* VAR_BOGUS_TTL  */
  YYSYMBOL_VAR_VAL_CLEAN_ADDITIONAL = 68,  /* VAR_VAL_CLEAN_ADDITIONAL  */
  YYSYMBOL_VAR_VAL_PERMISSIVE_MODE = 69,   /* VAR_VAL_PERMISSIVE_MODE  */
  YYSYMBOL_VAR_INCOMING_NUM_TCP = 70,      /* VAR_INCOMING_NUM_TCP  */
  YYSYMBOL_VAR_MSG_BUFFER_SIZE = 71,       /* VAR_MSG_BUFFER_SIZE  */
  YYSYMBOL_VAR_KEY_CACHE_SIZE = 72,        /* VAR_KEY_CACHE_SIZE  */
  YYSYMBOL_VAR_KEY_CACHE_SLABS = 73,       /* VAR_KEY_CACHE_SLABS  */
  YYSYMBOL_VAR_TRUSTED_KEYS_FILE = 74,     /* VAR_TRUSTED_KEYS_FILE  */
  YYSYMBOL_VAR_VAL_NSEC3_KEYSIZE_ITERATIONS = 75, /* VAR_VAL_NSEC3_KEYSIZE_ITERATIONS  */
  YYSYMBOL_VAR_USE_SYSLOG = 76,            /* VAR_USE_SYSLOG  */
  YYSYMBOL_VAR_OUTGOING_INTERFACE = 77,    /* VAR_OUTGOING_INTERFACE  */
  YYSYMBOL_VAR_ROOT_HINTS = 78,            /* VAR_ROOT_HINTS  */
  YYSYMBOL_VAR_DO_NOT_QUERY_LOCALHOST = 79, /* VAR_DO_NOT_QUERY_LOCALHOST  */
  YYSYMBOL_VAR_CACHE_MAX_TTL = 80,         /* VAR_CACHE_MAX_TTL  */
  YYSYMBOL_VAR_HARDEN_DNSSEC_STRIPPED = 81, /* VAR_HARDEN_DNSSEC_STRIPPED  */
  YYSYMBOL_VAR_ACCESS_CONTROL = 82,        /* VAR_ACCESS_CONTROL  */
  YYSYMBOL_VAR_LOCAL_ZONE = 83,            /* VAR_LOCAL_ZONE  */
  YYSYMBOL_VAR_LOCAL_DATA = 84,            /* VAR_LOCAL_DATA  */
  YYSYMBOL_VAR_INTERFACE_AUTOMATIC = 85,   /* VAR_INTERFACE_AUTOMATIC  */
  YYSYMBOL_VAR_STATISTICS_INTERVAL = 86,   /* VAR_STATISTICS_INTERVAL  */
  YYSYMBOL_VAR_DO_DAEMONIZE = 87,          /* VAR_DO_DAEMONIZE  */
  YYSYMBOL_VAR_USE_CAPS_FOR_ID = 88,       /* VAR_USE_CAPS_FOR_ID  */
  YYSYMBOL_VAR_STATISTICS_CUMULATIVE = 89, /* VAR_STATISTICS_CUMULATIVE  */
  YYSYMBOL_VAR_OUTGOING_PORT_PERMIT = 90,  /* VAR_OUTGOING_PORT_PERMIT  */
  YYSYMBOL_VAR_OUTGOING_PORT_AVOID = 91,   /* VAR_OUTGOING_PORT_AVOID  */
  YYSYMBOL_VAR_DLV_ANCHOR_FILE = 92,       /* VAR_DLV_ANCHOR_FILE  */
  YYSYMBOL_VAR_DLV_ANCHOR = 93,            /* VAR_DLV_ANCHOR  */
  YYSYMBOL_VAR_NEG_CACHE_SIZE = 94,        /* VAR_NEG_CACHE_SIZE  */
  YYSYMBOL_VAR_HARDEN_REFERRAL_PATH = 95,  /* VAR_HARDEN_REFERRAL_PATH  */
  YYSYMBOL_VAR_PRIVATE_ADDRESS = 96,       /* VAR_PRIVATE_ADDRESS  */
  YYSYMBOL_VAR_PRIVATE_DOMAIN = 97,        /* VAR_PRIVATE_DOMAIN  */
  YYSYMBOL_VAR_REMOTE_CONTROL = 98,        /* VAR_REMOTE_CONTROL  */
  YYSYMBOL_VAR_CONTROL_ENABLE = 99,        /* VAR_CONTROL_ENABLE  */
  YYSYMBOL_VAR_CONTROL_INTERFACE = 100,    /* VAR_CONTROL_INTERFACE  */
  YYSYMBOL_VAR_CONTROL_PORT = 101,         /* VAR_CONTROL_PORT  */
  YYSYMBOL_VAR_SERVER_KEY_FILE = 102,      /* VAR_SERVER_KEY_FILE  */
  YYSYMBOL_VAR_SERVER_CERT_FILE = 103,     /* VAR_SERVER_CERT_FILE  */
  YYSYMBOL_VAR_CONTROL_KEY_FILE = 104,     /* VAR_CONTROL_KEY_FILE  */
  YYSYMBOL_VAR_CONTROL_CERT_FILE = 105,    /* VAR_CONTROL_CERT_FILE  */
  YYSYMBOL_VAR_CONTROL_USE_CERT = 106,     /* VAR_CONTROL_USE_CERT  */
  YYSYMBOL_VAR_TCP_REUSE_TIMEOUT = 107,    /* VAR_TCP_REUSE_TIMEOUT  */
  YYSYMBOL_VAR_MAX_REUSE_TCP_QUERIES = 108, /* VAR_MAX_REUSE_TCP_QUERIES  */
  YYSYMBOL_VAR_EXTENDED_STATISTICS = 109,  /* VAR_EXTENDED_STATISTICS  */
  YYSYMBOL_VAR_LOCAL_DATA_PTR = 110,       /* VAR_LOCAL_DATA_PTR  */
  YYSYMBOL_VAR_JOSTLE_TIMEOUT = 111,       /* VAR_JOSTLE_TIMEOUT  */
  YYSYMBOL_VAR_STUB_PRIME = 112,           /* VAR_STUB_PRIME  */
  YYSYMBOL_VAR_UNWANTED_REPLY_THRESHOLD = 113, /* VAR_UNWANTED_REPLY_THRESHOLD  */
  YYSYMBOL_VAR_LOG_TIME_ASCII = 114,       /* VAR_LOG_TIME_ASCII  */
  YYSYMBOL_VAR_DOMAIN_INSECURE = 115,      /* VAR_DOMAIN_INSECURE  */
  YYSYMBOL_VAR_VAL_SIG_SKEW_MIN = 116,     /* VAR_VAL_SIG_SKEW_MIN  */
  YYSYMBOL_VAR_VAL_SIG_SKEW_MAX = 117,     /* VAR_VAL_SIG_SKEW_MAX  */
  YYSYMBOL_VAR_VAL_MAX_RESTART = 118,      /* VAR_VAL_MAX_RESTART  */
  YYSYMBOL_VAR_CACHE_MIN_TTL = 119,        /* VAR_CACHE_MIN_TTL  */
  YYSYMBOL_VAR_VAL_LOG_LEVEL = 120,        /* VAR_VAL_LOG_LEVEL  */
  YYSYMBOL_VAR_AUTO_TRUST_ANCHOR_FILE = 121, /* VAR_AUTO_TRUST_ANCHOR_FILE  */
  YYSYMBOL_VAR_KEEP_MISSING = 122,         /* VAR_KEEP_MISSING  */
  YYSYMBOL_VAR_ADD_HOLDDOWN = 123,         /* VAR_ADD_HOLDDOWN  */
  YYSYMBOL_VAR_DEL_HOLDDOWN = 124,         /* VAR_DEL_HOLDDOWN  */
  YYSYMBOL_VAR_SO_RCVBUF = 125,            /* VAR_SO_RCVBUF  */
  YYSYMBOL_VAR_EDNS_BUFFER_SIZE = 126,     /* VAR_EDNS_BUFFER_SIZE  */
  YYSYMBOL_VAR_PREFETCH = 127,             /* VAR_PREFETCH  */
  YYSYMBOL_VAR_PREFETCH_KEY = 128,         /* VAR_PREFETCH_KEY  */
  YYSYMBOL_VAR_SO_SNDBUF = 129,            /* VAR_SO_SNDBUF  */
  YYSYMBOL_VAR_SO_REUSEPORT = 130,         /* VAR_SO_REUSEPORT  */
  YYSYMBOL_VAR_HARDEN_BELOW_NXDOMAIN = 131, /* VAR_HARDEN_BELOW_NXDOMAIN  */
  YYSYMBOL_VAR_IGNORE_CD_FLAG = 132,       /* VAR_IGNORE_CD_FLAG  */
  YYSYMBOL_VAR_LOG_QUERIES = 133,          /* VAR_LOG_QUERIES  */
  YYSYMBOL_VAR_LOG_REPLIES = 134,          /* VAR_LOG_REPLIES  */
  YYSYMBOL_VAR_LOG_LOCAL_ACTIONS = 135,    /* VAR_LOG_LOCAL_ACTIONS  */
  YYSYMBOL_VAR_TCP_UPSTREAM = 136,         /* VAR_TCP_UPSTREAM  */
  YYSYMBOL_VAR_SSL_UPSTREAM = 137,         /* VAR_SSL_UPSTREAM  */
  YYSYMBOL_VAR_TCP_AUTH_QUERY_TIMEOUT = 138, /* VAR_TCP_AUTH_QUERY_TIMEOUT  */
  YYSYMBOL_VAR_FORWARD_FIRST = 139,        /* VAR_FORWARD_FIRST  */
  YYSYMBOL_VAR_STUB_SSL_UPSTREAM = 140,    /* VAR_STUB_SSL_UPSTREAM  */
  YYSYMBOL_VAR_FORWARD_SSL_UPSTREAM = 141, /* VAR_FORWARD_SSL_UPSTREAM  */
  YYSYMBOL_VAR_TLS_CERT_BUNDLE = 142,      /* VAR_TLS_CERT_BUNDLE  */
  YYSYMBOL_VAR_STUB_TCP_UPSTREAM = 143,    /* VAR_STUB_TCP_UPSTREAM  */
  YYSYMBOL_VAR_FORWARD_TCP_UPSTREAM = 144, /* VAR_FORWARD_TCP_UPSTREAM  */
  YYSYMBOL_VAR_STUB_FIRST = 145,           /* VAR_STUB_FIRST  */
  YYSYMBOL_VAR_MINIMAL_RESPONSES = 146,    /* VAR_MINIMAL_RESPONSES  */
  YYSYMBOL_VAR_RRSET_ROUNDROBIN = 147,     /* VAR_RRSET_ROUNDROBIN  */
  YYSYMBOL_VAR_MAX_UDP_SIZE = 148,         /* VAR_MAX_UDP_SIZE  */
  YYSYMBOL_VAR_DELAY_CLOSE = 149,          /* VAR_DELAY_CLOSE  */
  YYSYMBOL_VAR_UDP_CONNECT = 150,          /* VAR_UDP_CONNECT  */
  YYSYMBOL_VAR_UNBLOCK_LAN_ZONES = 151,    /* VAR_UNBLOCK_LAN_ZONES  */
  YYSYMBOL_VAR_INSECURE_LAN_ZONES = 152,   /* VAR_INSECURE_LAN_ZONES  */
  YYSYMBOL_VAR_INFRA_CACHE_MIN_RTT = 153,  /* VAR_INFRA_CACHE_MIN_RTT  */
  YYSYMBOL_VAR_INFRA_CACHE_MAX_RTT = 154,  /* VAR_INFRA_CACHE_MAX_RTT  */
  YYSYMBOL_VAR_INFRA_KEEP_PROBING = 155,   /* VAR_INFRA_KEEP_PROBING  */
  YYSYMBOL_VAR_NAT64_PREFIX = 156,         /* VAR_NAT64_PREFIX  */
  YYSYMBOL_VAR_HARDEN_ALGO_DOWNGRADE = 157, /* VAR_HARDEN_ALGO_DOWNGRADE  */
  YYSYMBOL_VAR_IP_TRANSPARENT = 158,       /* VAR_IP_TRANSPARENT  */
  YYSYMBOL_VAR_IP_DSCP = 159,              /* VAR_IP_DSCP  */
  YYSYMBOL_VAR_DISABLE_DNSSEC_LAME_CHECK = 160, /* VAR_DISABLE_DNSSEC_LAME_CHECK  */
  YYSYMBOL_VAR_IP_RATELIMIT = 161,         /* VAR_IP_RATELIMIT  */
  YYSYMBOL_VAR_IP_RATELIMIT_SLABS = 162,   /* VAR_IP_RATELIMIT_SLABS  */
  YYSYMBOL_VAR_IP_RATELIMIT_SIZE = 163,    /* VAR_IP_RATELIMIT_SIZE  */
  YYSYMBOL_VAR_RATELIMIT = 164,            /* VAR_RATELIMIT  */
  YYSYMBOL_VAR_RATELIMIT_SLABS = 165,      /* VAR_RATELIMIT_SLABS  */
  YYSYMBOL_VAR_RATELIMIT_SIZE = 166,       /* VAR_RATELIMIT_SIZE  */
  YYSYMBOL_VAR_OUTBOUND_MSG_RETRY = 167,   /* VAR_OUTBOUND_MSG_RETRY  */
  YYSYMBOL_VAR_MAX_SENT_COUNT = 168,       /* VAR_MAX_SENT_COUNT  */
  YYSYMBOL_VAR_MAX_QUERY_RESTARTS = 169,   /* VAR_MAX_QUERY_RESTARTS  */
  YYSYMBOL_VAR_RATELIMIT_FOR_DOMAIN = 170, /* VAR_RATELIMIT_FOR_DOMAIN  */
  YYSYMBOL_VAR_RATELIMIT_BELOW_DOMAIN = 171, /* VAR_RATELIMIT_BELOW_DOMAIN  */
  YYSYMBOL_VAR_IP_RATELIMIT_FACTOR = 172,  /* VAR_IP_RATELIMIT_FACTOR  */
  YYSYMBOL_VAR_RATELIMIT_FACTOR = 173,     /* VAR_RATELIMIT_FACTOR  */
  YYSYMBOL_VAR_IP_RATELIMIT_BACKOFF = 174, /* VAR_IP_RATELIMIT_BACKOFF  */
  YYSYMBOL_VAR_RATELIMIT_BACKOFF = 175,    /* VAR_RATELIMIT_BACKOFF  */
  YYSYMBOL_VAR_CAPS_WHITELIST = 176,       /* VAR_CAPS_WHITELIST  */
  YYSYMBOL_VAR_CACHE_MAX_NEGATIVE_TTL = 177, /* VAR_CACHE_MAX_NEGATIVE_TTL  */
  YYSYMBOL_VAR_PERMIT_SMALL_HOLDDOWN = 178, /* VAR_PERMIT_SMALL_HOLDDOWN  */
  YYSYMBOL_VAR_CACHE_MIN_NEGATIVE_TTL = 179, /* VAR_CACHE_MIN_NEGATIVE_TTL  */
  YYSYMBOL_VAR_QNAME_MINIMISATION = 180,   /* VAR_QNAME_MINIMISATION  */
  YYSYMBOL_VAR_QNAME_MINIMISATION_STRICT = 181, /* VAR_QNAME_MINIMISATION_STRICT  */
  YYSYMBOL_VAR_IP_FREEBIND = 182,          /* VAR_IP_FREEBIND  */
  YYSYMBOL_VAR_DEFINE_TAG = 183,           /* VAR_DEFINE_TAG  */
  YYSYMBOL_VAR_LOCAL_ZONE_TAG = 184,       /* VAR_LOCAL_ZONE_TAG  */
  YYSYMBOL_VAR_ACCESS_CONTROL_TAG = 185,   /* VAR_ACCESS_CONTROL_TAG  */
  YYSYMBOL_VAR_LOCAL_ZONE_OVERRIDE = 186,  /* VAR_LOCAL_ZONE_OVERRIDE  */
  YYSYMBOL_VAR_ACCESS_CONTROL_TAG_ACTION = 187, /* VAR_ACCESS_CONTROL_TAG_ACTION  */
  YYSYMBOL_VAR_ACCESS_CONTROL_TAG_DATA = 188, /* VAR_ACCESS_CONTROL_TAG_DATA  */
  YYSYMBOL_VAR_SERVE_EXPIRED = 189,        /* VAR_SERVE_EXPIRED  */
  YYSYMBOL_VAR_SERVE_EXPIRED_TTL = 190,    /* VAR_SERVE_EXPIRED_TTL  */
  YYSYMBOL_VAR_SERVE_EXPIRED_TTL_RESET = 191, /* VAR_SERVE_EXPIRED_TTL_RESET  */
  YYSYMBOL_VAR_SERVE_EXPIRED_REPLY_TTL = 192, /* VAR_SERVE_EXPIRED_REPLY_TTL  */
  YYSYMBOL_VAR_SERVE_EXPIRED_CLIENT_TIMEOUT = 193, /* VAR_SERVE_EXPIRED_CLIENT_TIMEOUT  */
  YYSYMBOL_VAR_EDE_SERVE_EXPIRED = 194,    /* VAR_EDE_SERVE_EXPIRED  */
  YYSYMBOL_VAR_SERVE_ORIGINAL_TTL = 195,   /* VAR_SERVE_ORIGINAL_TTL  */
  YYSYMBOL_VAR_FAKE_DSA = 196,             /* VAR_FAKE_DSA  */
  YYSYMBOL_VAR_FAKE_SHA1 = 197,            /* VAR_FAKE_SHA1  */
  YYSYMBOL_VAR_LOG_IDENTITY = 198,         /* VAR_LOG_IDENTITY  */
  YYSYMBOL_VAR_HIDE_TRUSTANCHOR = 199,     /* VAR_HIDE_TRUSTANCHOR  */
  YYSYMBOL_VAR_HIDE_HTTP_USER_AGENT = 200, /* VAR_HIDE_HTTP_USER_AGENT  */
  YYSYMBOL_VAR_HTTP_USER_AGENT = 201,      /* VAR_HTTP_USER_AGENT  */
  YYSYMBOL_VAR_TRUST_ANCHOR_SIGNALING = 202, /* VAR_TRUST_ANCHOR_SIGNALING  */
  YYSYMBOL_VAR_AGGRESSIVE_NSEC = 203,      /* VAR_AGGRESSIVE_NSEC  */
  YYSYMBOL_VAR_USE_SYSTEMD = 204,          /* VAR_USE_SYSTEMD  */
  YYSYMBOL_VAR_SHM_ENABLE = 205,           /* VAR_SHM_ENABLE  */
  YYSYMBOL_VAR_SHM_KEY = 206,              /* VAR_SHM_KEY  */
  YYSYMBOL_VAR_ROOT_KEY_SENTINEL = 207,    /* VAR_ROOT_KEY_SENTINEL  */
  YYSYMBOL_VAR_PAD_QUERIES = 208,          /* VAR_PAD_QUERIES  */
  YYSYMBOL_VAR_PAD_QUERIES_BLOCK_SIZE = 209, /* VAR_PAD_QUERIES_BLOCK_SIZE  */
  YYSYMBOL_VAR_IPSECMOD_ENABLED = 210,     /* VAR_IPSECMOD_ENABLED  */
  YYSYMBOL_VAR_IPSECMOD_HOOK = 211,        /* VAR_IPSECMOD_HOOK  */
  YYSYMBOL_VAR_IPSECMOD_IGNORE_BOGUS = 212, /* VAR_IPSECMOD_IGNORE_BOGUS  */
  YYSYMBOL_VAR_IPSECMOD_MAX_TTL = 213,     /* VAR_IPSECMOD_MAX_TTL  */
  YYSYMBOL_VAR_IPSECMOD_WHITELIST = 214,   /* VAR_IPSECMOD_WHITELIST  */
  YYSYMBOL_VAR_IPSECMOD_STRICT = 215,      /* VAR_IPSECMOD_STRICT  */
  YYSYMBOL_VAR_UDP_UPSTREAM_WITHOUT_DOWNSTREAM = 216, /* VAR_UDP_UPSTREAM_WITHOUT_DOWNSTREAM  */
  YYSYMBOL_VAR_FOR_UPSTREAM = 217,         /* VAR_FOR_UPSTREAM  */
  YYSYMBOL_VAR_AUTH_ZONE = 218,            /* VAR_AUTH_ZONE  */
  YYSYMBOL_VAR_ZONEFILE = 219,             /* VAR_ZONEFILE  */
  YYSYMBOL_VAR_MASTER = 220,               /* VAR_MASTER  */
  YYSYMBOL_VAR_URL = 221,                  /* VAR_URL  */
  YYSYMBOL_VAR_FOR_DOWNSTREAM = 222,       /* VAR_FOR_DOWNSTREAM  */
  YYSYMBOL_VAR_FALLBACK_ENABLED = 223,     /* VAR_FALLBACK_ENABLED  */
  YYSYMBOL_VAR_LOW_RTT = 224,              /* VAR_LOW_RTT  */
  YYSYMBOL_VAR_LOW_RTT_PERMIL = 225,       /* VAR_LOW_RTT_PERMIL  */
  YYSYMBOL_VAR_FAST_SERVER_PERMIL = 226,   /* VAR_FAST_SERVER_PERMIL  */
  YYSYMBOL_VAR_FAST_SERVER_NUM = 227,      /* VAR_FAST_SERVER_NUM  */
  YYSYMBOL_VAR_ALLOW_NOTIFY = 228,         /* VAR_ALLOW_NOTIFY  */
  YYSYMBOL_VAR_TLS_WIN_CERT = 229,         /* VAR_TLS_WIN_CERT  */
  YYSYMBOL_VAR_TCP_CONNECTION_LIMIT = 230, /* VAR_TCP_CONNECTION_LIMIT  */
  YYSYMBOL_VAR_ANSWER_COOKIE = 231,        /* VAR_ANSWER_COOKIE  */
  YYSYMBOL_VAR_COOKIE_SECRET = 232,        /* VAR_COOKIE_SECRET  */
  YYSYMBOL_VAR_IP_RATELIMIT_COOKIE = 233,  /* VAR_IP_RATELIMIT_COOKIE  */
  YYSYMBOL_VAR_FORWARD_NO_CACHE = 234,     /* VAR_FORWARD_NO_CACHE  */
  YYSYMBOL_VAR_STUB_NO_CACHE = 235,        /* VAR_STUB_NO_CACHE  */
  YYSYMBOL_VAR_LOG_SERVFAIL = 236,         /* VAR_LOG_SERVFAIL  */
  YYSYMBOL_VAR_DENY_ANY = 237,             /* VAR_DENY_ANY  */
  YYSYMBOL_VAR_UNKNOWN_SERVER_TIME_LIMIT = 238, /* VAR_UNKNOWN_SERVER_TIME_LIMIT  */
  YYSYMBOL_VAR_LOG_TAG_QUERYREPLY = 239,   /* VAR_LOG_TAG_QUERYREPLY  */
  YYSYMBOL_VAR_DISCARD_TIMEOUT = 240,      /* VAR_DISCARD_TIMEOUT  */
  YYSYMBOL_VAR_WAIT_LIMIT = 241,           /* VAR_WAIT_LIMIT  */
  YYSYMBOL_VAR_WAIT_LIMIT_COOKIE = 242,    /* VAR_WAIT_LIMIT_COOKIE  */
  YYSYMBOL_VAR_WAIT_LIMIT_NETBLOCK = 243,  /* VAR_WAIT_LIMIT_NETBLOCK  */
  YYSYMBOL_VAR_WAIT_LIMIT_COOKIE_NETBLOCK = 244, /* VAR_WAIT_LIMIT_COOKIE_NETBLOCK  */
  YYSYMBOL_VAR_STREAM_WAIT_SIZE = 245,     /* VAR_STREAM_WAIT_SIZE  */
  YYSYMBOL_VAR_TLS_USE_SNI = 246,          /* VAR_TLS_USE_SNI  */
  YYSYMBOL_VAR_TLS_PROTOCOLS = 247,        /* VAR_TLS_PROTOCOLS  */
  YYSYMBOL_VAR_IPSET = 248,                /* VAR_IPSET  */
  YYSYMBOL_VAR_IPSET_NAME_V4 = 249,        /* VAR_IPSET_NAME_V4  */
  YYSYMBOL_VAR_IPSET_NAME_V6 = 250,        /* VAR_IPSET_NAME_V6  */
  YYSYMBOL_VAR_EDNS_CLIENT_STRING = 251,   /* VAR_EDNS_CLIENT_STRING  */
  YYSYMBOL_VAR_EDNS_CLIENT_STRING_OPCODE = 252, /* VAR_EDNS_CLIENT_STRING_OPCODE  */
  YYSYMBOL_VAR_NSID = 253,                 /* VAR_NSID  */
  YYSYMBOL_VAR_ZONEMD_PERMISSIVE_MODE = 254, /* VAR_ZONEMD_PERMISSIVE_MODE  */
  YYSYMBOL_VAR_ZONEMD_CHECK = 255,         /* VAR_ZONEMD_CHECK  */
  YYSYMBOL_VAR_ZONEMD_REJECT_ABSENCE = 256, /* VAR_ZONEMD_REJECT_ABSENCE  */
  YYSYMBOL_VAR_INTERFACE_AUTOMATIC_PORTS = 257, /* VAR_INTERFACE_AUTOMATIC_PORTS  */
  YYSYMBOL_VAR_EDE = 258,                  /* VAR_EDE  */
  YYSYMBOL_VAR_DNS_ERROR_REPORTING = 259,  /* VAR_DNS_ERROR_REPORTING  */
  YYSYMBOL_VAR_INTERFACE_ACTION = 260,     /* VAR_INTERFACE_ACTION  */
  YYSYMBOL_VAR_INTERFACE_TAG = 261,        /* VAR_INTERFACE_TAG  */
  YYSYMBOL_VAR_INTERFACE_TAG_ACTION = 262, /* VAR_INTERFACE_TAG_ACTION  */
  YYSYMBOL_VAR_INTERFACE_TAG_DATA = 263,   /* VAR_INTERFACE_TAG_DATA  */
  YYSYMBOL_VAR_PROXY_PROTOCOL_PORT = 264,  /* VAR_PROXY_PROTOCOL_PORT  */
  YYSYMBOL_VAR_STATISTICS_INHIBIT_ZERO = 265, /* VAR_STATISTICS_INHIBIT_ZERO  */
  YYSYMBOL_VAR_HARDEN_UNKNOWN_ADDITIONAL = 266, /* VAR_HARDEN_UNKNOWN_ADDITIONAL  */
  YYSYMBOL_VAR_DISABLE_EDNS_DO = 267,      /* VAR_DISABLE_EDNS_DO  */
  YYSYMBOL_VAR_LOG_DESTADDR = 268,         /* VAR_LOG_DESTADDR  */
  YYSYMBOL_VAR_COOKIE_SECRET_FILE = 269,   /* VAR_COOKIE_SECRET_FILE  */
  YYSYMBOL_VAR_ITER_SCRUB_NS = 270,        /* VAR_ITER_SCRUB_NS  */
  YYSYMBOL_VAR_ITER_SCRUB_CNAME = 271,     /* VAR_ITER_SCRUB_CNAME  */
  YYSYMBOL_VAR_ITER_SCRUB_RRSIG = 272,     /* VAR_ITER_SCRUB_RRSIG  */
  YYSYMBOL_VAR_MAX_GLOBAL_QUOTA = 273,     /* VAR_MAX_GLOBAL_QUOTA  */
  YYSYMBOL_VAR_HARDEN_UNVERIFIED_GLUE = 274, /* VAR_HARDEN_UNVERIFIED_GLUE  */
  YYSYMBOL_VAR_LOG_TIME_ISO = 275,         /* VAR_LOG_TIME_ISO  */
  YYSYMBOL_VAR_ITER_SCRUB_PROMISCUOUS = 276, /* VAR_ITER_SCRUB_PROMISCUOUS  */
  YYSYMBOL_VAR_LOG_THREAD_ID = 277,        /* VAR_LOG_THREAD_ID  */
  YYSYMBOL_YYACCEPT = 278,                 /* $accept  */
  YYSYMBOL_toplevelvars = 279,             /* toplevelvars  */
  YYSYMBOL_toplevelvar = 280,              /* toplevelvar  */
  YYSYMBOL_force_toplevel = 281,           /* force_toplevel  */
  YYSYMBOL_serverstart = 282,              /* serverstart  */
  YYSYMBOL_contents_server = 283,          /* contents_server  */
  YYSYMBOL_content_server = 284,           /* content_server  */
  YYSYMBOL_stub_clause = 285,              /* stub_clause  */
  YYSYMBOL_stubstart = 286,                /* stubstart  */
  YYSYMBOL_contents_stub = 287,            /* contents_stub  */
  YYSYMBOL_content_stub = 288,             /* content_stub  */
  YYSYMBOL_forward_clause = 289,           /* forward_clause  */
  YYSYMBOL_forwardstart = 290,             /* forwardstart  */
  YYSYMBOL_contents_forward = 291,         /* contents_forward  */
  YYSYMBOL_content_forward = 292,          /* content_forward  */
  YYSYMBOL_authstart = 293,                /* authstart  */
  YYSYMBOL_contents_auth = 294,            /* contents_auth  */
  YYSYMBOL_content_auth = 295,             /* content_auth  */
  YYSYMBOL_server_num_threads = 296,       /* server_num_threads  */
  YYSYMBOL_server_verbosity = 297,         /* server_verbosity  */
  YYSYMBOL_server_statistics_interval = 298, /* server_statistics_interval  */
  YYSYMBOL_server_statistics_cumulative = 299, /* server_statistics_cumulative  */
  YYSYMBOL_server_extended_statistics = 300, /* server_extended_statistics  */
  YYSYMBOL_server_statistics_inhibit_zero = 301, /* server_statistics_inhibit_zero  */
  YYSYMBOL_server_shm_enable = 302,        /* server_shm_enable  */
  YYSYMBOL_server_shm_key = 303,           /* server_shm_key  */
  YYSYMBOL_server_port = 304,              /* server_port  */
  YYSYMBOL_server_interface = 305,         /* server_interface  */
  YYSYMBOL_server_outgoing_interface = 306, /* server_outgoing_interface  */
  YYSYMBOL_server_outgoing_range = 307,    /* server_outgoing_range  */
  YYSYMBOL_server_outgoing_port_permit = 308, /* server_outgoing_port_permit  */
  YYSYMBOL_server_outgoing_port_avoid = 309, /* server_outgoing_port_avoid  */
  YYSYMBOL_server_outgoing_num_tcp = 310,  /* server_outgoing_num_tcp  */
  YYSYMBOL_server_incoming_num_tcp = 311,  /* server_incoming_num_tcp  */
  YYSYMBOL_server_interface_automatic = 312, /* server_interface_automatic  */
  YYSYMBOL_server_interface_automatic_ports = 313, /* server_interface_automatic_ports  */
  YYSYMBOL_server_do_ip4 = 314,            /* server_do_ip4  */
  YYSYMBOL_server_do_ip6 = 315,            /* server_do_ip6  */
  YYSYMBOL_server_do_nat64 = 316,          /* server_do_nat64  */
  YYSYMBOL_server_do_udp = 317,            /* server_do_udp  */
  YYSYMBOL_server_do_tcp = 318,            /* server_do_tcp  */
  YYSYMBOL_server_prefer_ip4 = 319,        /* server_prefer_ip4  */
  YYSYMBOL_server_prefer_ip6 = 320,        /* server_prefer_ip6  */
  YYSYMBOL_server_tcp_mss = 321,           /* server_tcp_mss  */
  YYSYMBOL_server_outgoing_tcp_mss = 322,  /* server_outgoing_tcp_mss  */
  YYSYMBOL_server_tcp_idle_timeout = 323,  /* server_tcp_idle_timeout  */
  YYSYMBOL_server_max_reuse_tcp_queries = 324, /* server_max_reuse_tcp_queries  */
  YYSYMBOL_server_tcp_reuse_timeout = 325, /* server_tcp_reuse_timeout  */
  YYSYMBOL_server_tcp_auth_query_timeout = 326, /* server_tcp_auth_query_timeout  */
  YYSYMBOL_server_tcp_keepalive = 327,     /* server_tcp_keepalive  */
  YYSYMBOL_server_tcp_keepalive_timeout = 328, /* server_tcp_keepalive_timeout  */
  YYSYMBOL_server_sock_queue_timeout = 329, /* server_sock_queue_timeout  */
  YYSYMBOL_server_tcp_upstream = 330,      /* server_tcp_upstream  */
  YYSYMBOL_server_udp_upstream_without_downstream = 331, /* server_udp_upstream_without_downstream  */
  YYSYMBOL_server_ssl_upstream = 332,      /* server_ssl_upstream  */
  YYSYMBOL_server_tls_cert_bundle = 333,   /* server_tls_cert_bundle  */
  YYSYMBOL_server_tls_win_cert = 334,      /* server_tls_win_cert  */
  YYSYMBOL_server_tls_use_sni = 335,       /* server_tls_use_sni  */
  YYSYMBOL_server_tls_protocols = 336,     /* server_tls_protocols  */
  YYSYMBOL_server_use_systemd = 337,       /* server_use_systemd  */
  YYSYMBOL_server_do_daemonize = 338,      /* server_do_daemonize  */
  YYSYMBOL_server_use_syslog = 339,        /* server_use_syslog  */
  YYSYMBOL_server_log_time_ascii = 340,    /* server_log_time_ascii  */
  YYSYMBOL_server_log_time_iso = 341,      /* server_log_time_iso  */
  YYSYMBOL_server_log_queries = 342,       /* server_log_queries  */
  YYSYMBOL_server_log_replies = 343,       /* server_log_replies  */
  YYSYMBOL_server_log_tag_queryreply = 344, /* server_log_tag_queryreply  */
  YYSYMBOL_server_log_servfail = 345,      /* server_log_servfail  */
  YYSYMBOL_server_log_destaddr = 346,      /* server_log_destaddr  */
  YYSYMBOL_server_log_thread_id = 347,     /* server_log_thread_id  */
  YYSYMBOL_server_log_local_actions = 348, /* server_log_local_actions  */
  YYSYMBOL_server_chroot = 349,            /* server_chroot  */
  YYSYMBOL_server_username = 350,          /* server_username  */
  YYSYMBOL_server_directory = 351,         /* server_directory  */
  YYSYMBOL_server_logfile = 352,           /* server_logfile  */
  YYSYMBOL_server_pidfile = 353,           /* server_pidfile  */
  YYSYMBOL_server_root_hints = 354,        /* server_root_hints  */
  YYSYMBOL_server_dlv_anchor_file = 355,   /* server_dlv_anchor_file  */
  YYSYMBOL_server_dlv_anchor = 356,        /* server_dlv_anchor  */
  YYSYMBOL_server_auto_trust_anchor_file = 357, /* server_auto_trust_anchor_file  */
  YYSYMBOL_server_trust_anchor_file = 358, /* server_trust_anchor_file  */
  YYSYMBOL_server_trusted_keys_file = 359, /* server_trusted_keys_file  */
  YYSYMBOL_server_trust_anchor = 360,      /* server_trust_anchor  */
  YYSYMBOL_server_trust_anchor_signaling = 361, /* server_trust_anchor_signaling  */
  YYSYMBOL_server_root_key_sentinel = 362, /* server_root_key_sentinel  */
  YYSYMBOL_server_domain_insecure = 363,   /* server_domain_insecure  */
  YYSYMBOL_server_hide_identity = 364,     /* server_hide_identity  */
  YYSYMBOL_server_hide_version = 365,      /* server_hide_version  */
  YYSYMBOL_server_hide_trustanchor = 366,  /* server_hide_trustanchor  */
  YYSYMBOL_server_hide_http_user_agent = 367, /* server_hide_http_user_agent  */
  YYSYMBOL_server_identity = 368,          /* server_identity  */
  YYSYMBOL_server_version = 369,           /* server_version  */
  YYSYMBOL_server_http_user_agent = 370,   /* server_http_user_agent  */
  YYSYMBOL_server_nsid = 371,              /* server_nsid  */
  YYSYMBOL_server_so_rcvbuf = 372,         /* server_so_rcvbuf  */
  YYSYMBOL_server_so_sndbuf = 373,         /* server_so_sndbuf  */
  YYSYMBOL_server_so_reuseport = 374,      /* server_so_reuseport  */
  YYSYMBOL_server_ip_transparent = 375,    /* server_ip_transparent  */
  YYSYMBOL_server_ip_freebind = 376,       /* server_ip_freebind  */
  YYSYMBOL_server_ip_dscp = 377,           /* server_ip_dscp  */
  YYSYMBOL_server_stream_wait_size = 378,  /* server_stream_wait_size  */
  YYSYMBOL_server_edns_buffer_size = 379,  /* server_edns_buffer_size  */
  YYSYMBOL_server_msg_buffer_size = 380,   /* server_msg_buffer_size  */
  YYSYMBOL_server_msg_cache_size = 381,    /* server_msg_cache_size  */
  YYSYMBOL_server_msg_cache_slabs = 382,   /* server_msg_cache_slabs  */
  YYSYMBOL_server_num_queries_per_thread = 383, /* server_num_queries_per_thread  */
  YYSYMBOL_server_jostle_timeout = 384,    /* server_jostle_timeout  */
  YYSYMBOL_server_delay_close = 385,       /* server_delay_close  */
  YYSYMBOL_server_udp_connect = 386,       /* server_udp_connect  */
  YYSYMBOL_server_unblock_lan_zones = 387, /* server_unblock_lan_zones  */
  YYSYMBOL_server_insecure_lan_zones = 388, /* server_insecure_lan_zones  */
  YYSYMBOL_server_rrset_cache_size = 389,  /* server_rrset_cache_size  */
  YYSYMBOL_server_rrset_cache_slabs = 390, /* server_rrset_cache_slabs  */
  YYSYMBOL_server_infra_host_ttl = 391,    /* server_infra_host_ttl  */
  YYSYMBOL_server_infra_lame_ttl = 392,    /* server_infra_lame_ttl  */
  YYSYMBOL_server_infra_cache_numhosts = 393, /* server_infra_cache_numhosts  */
  YYSYMBOL_server_infra_cache_lame_size = 394, /* server_infra_cache_lame_size  */
  YYSYMBOL_server_infra_cache_slabs = 395, /* server_infra_cache_slabs  */
  YYSYMBOL_server_infra_cache_min_rtt = 396, /* server_infra_cache_min_rtt  */
  YYSYMBOL_server_infra_cache_max_rtt = 397, /* server_infra_cache_max_rtt  */
  YYSYMBOL_server_infra_keep_probing = 398, /* server_infra_keep_probing  */
  YYSYMBOL_server_target_fetch_policy = 399, /* server_target_fetch_policy  */
  YYSYMBOL_server_harden_short_bufsize = 400, /* server_harden_short_bufsize  */
  YYSYMBOL_server_harden_large_queries = 401, /* server_harden_large_queries  */
  YYSYMBOL_server_harden_glue = 402,       /* server_harden_glue  */
  YYSYMBOL_server_harden_unverified_glue = 403, /* server_harden_unverified_glue  */
  YYSYMBOL_server_harden_dnssec_stripped = 404, /* server_harden_dnssec_stripped  */
  YYSYMBOL_server_harden_below_nxdomain = 405, /* server_harden_below_nxdomain  */
  YYSYMBOL_server_harden_referral_path = 406, /* server_harden_referral_path  */
  YYSYMBOL_server_harden_algo_downgrade = 407, /* server_harden_algo_downgrade  */
  YYSYMBOL_server_harden_unknown_additional = 408, /* server_harden_unknown_additional  */
  YYSYMBOL_server_use_caps_for_id = 409,   /* server_use_caps_for_id  */
  YYSYMBOL_server_caps_whitelist = 410,    /* server_caps_whitelist  */
  YYSYMBOL_server_private_address = 411,   /* server_private_address  */
  YYSYMBOL_server_private_domain = 412,    /* server_private_domain  */
  YYSYMBOL_server_prefetch = 413,          /* server_prefetch  */
  YYSYMBOL_server_prefetch_key = 414,      /* server_prefetch_key  */
  YYSYMBOL_server_deny_any = 415,          /* server_deny_any  */
  YYSYMBOL_server_unwanted_reply_threshold = 416, /* server_unwanted_reply_threshold  */
  YYSYMBOL_server_do_not_query_address = 417, /* server_do_not_query_address  */
  YYSYMBOL_server_do_not_query_localhost = 418, /* server_do_not_query_localhost  */
  YYSYMBOL_server_access_control = 419,    /* server_access_control  */
  YYSYMBOL_server_interface_action = 420,  /* server_interface_action  */
  YYSYMBOL_server_module_conf = 421,       /* server_module_conf  */
  YYSYMBOL_server_val_override_date = 422, /* server_val_override_date  */
  YYSYMBOL_server_val_sig_skew_min = 423,  /* server_val_sig_skew_min  */
  YYSYMBOL_server_val_sig_skew_max = 424,  /* server_val_sig_skew_max  */
  YYSYMBOL_server_val_max_restart = 425,   /* server_val_max_restart  */
  YYSYMBOL_server_cache_max_ttl = 426,     /* server_cache_max_ttl  */
  YYSYMBOL_server_cache_max_negative_ttl = 427, /* server_cache_max_negative_ttl  */
  YYSYMBOL_server_cache_min_negative_ttl = 428, /* server_cache_min_negative_ttl  */
  YYSYMBOL_server_cache_min_ttl = 429,     /* server_cache_min_ttl  */
  YYSYMBOL_server_bogus_ttl = 430,         /* server_bogus_ttl  */
  YYSYMBOL_server_val_clean_additional = 431, /* server_val_clean_additional  */
  YYSYMBOL_server_val_permissive_mode = 432, /* server_val_permissive_mode  */
  YYSYMBOL_server_aggressive_nsec = 433,   /* server_aggressive_nsec  */
  YYSYMBOL_server_ignore_cd_flag = 434,    /* server_ignore_cd_flag  */
  YYSYMBOL_server_disable_edns_do = 435,   /* server_disable_edns_do  */
  YYSYMBOL_server_serve_expired = 436,     /* server_serve_expired  */
  YYSYMBOL_server_serve_expired_ttl = 437, /* server_serve_expired_ttl  */
  YYSYMBOL_server_serve_expired_ttl_reset = 438, /* server_serve_expired_ttl_reset  */
  YYSYMBOL_server_serve_expired_reply_ttl = 439, /* server_serve_expired_reply_ttl  */
  YYSYMBOL_server_serve_expired_client_timeout = 440, /* server_serve_expired_client_timeout  */
  YYSYMBOL_server_ede_serve_expired = 441, /* server_ede_serve_expired  */
  YYSYMBOL_server_serve_original_ttl = 442, /* server_serve_original_ttl  */
  YYSYMBOL_server_fake_dsa = 443,          /* server_fake_dsa  */
  YYSYMBOL_server_fake_sha1 = 444,         /* server_fake_sha1  */
  YYSYMBOL_server_val_log_level = 445,     /* server_val_log_level  */
  YYSYMBOL_server_val_nsec3_keysize_iterations = 446, /* server_val_nsec3_keysize_iterations  */
  YYSYMBOL_server_zonemd_permissive_mode = 447, /* server_zonemd_permissive_mode  */
  YYSYMBOL_server_add_holddown = 448,      /* server_add_holddown  */
  YYSYMBOL_server_del_holddown = 449,      /* server_del_holddown  */
  YYSYMBOL_server_keep_missing = 450,      /* server_keep_missing  */
  YYSYMBOL_server_permit_small_holddown = 451, /* server_permit_small_holddown  */
  YYSYMBOL_server_key_cache_size = 452,    /* server_key_cache_size  */
  YYSYMBOL_server_key_cache_slabs = 453,   /* server_key_cache_slabs  */
  YYSYMBOL_server_neg_cache_size = 454,    /* server_neg_cache_size  */
  YYSYMBOL_server_local_zone = 455,        /* server_local_zone  */
  YYSYMBOL_server_local_data = 456,        /* server_local_data  */
  YYSYMBOL_server_local_data_ptr = 457,    /* server_local_data_ptr  */
  YYSYMBOL_server_minimal_responses = 458, /* server_minimal_responses  */
  YYSYMBOL_server_rrset_roundrobin = 459,  /* server_rrset_roundrobin  */
  YYSYMBOL_server_unknown_server_time_limit = 460, /* server_unknown_server_time_limit  */
  YYSYMBOL_server_discard_timeout = 461,   /* server_discard_timeout  */
  YYSYMBOL_server_wait_limit = 462,        /* server_wait_limit  */
  YYSYMBOL_server_wait_limit_cookie = 463, /* server_wait_limit_cookie  */
  YYSYMBOL_server_wait_limit_netblock = 464, /* server_wait_limit_netblock  */
  YYSYMBOL_server_wait_limit_cookie_netblock = 465, /* server_wait_limit_cookie_netblock  */
  YYSYMBOL_server_max_udp_size = 466,      /* server_max_udp_size  */
  YYSYMBOL_server_nat64_prefix = 467,      /* server_nat64_prefix  */
  YYSYMBOL_server_define_tag = 468,        /* server_define_tag  */
  YYSYMBOL_server_local_zone_tag = 469,    /* server_local_zone_tag  */
  YYSYMBOL_server_access_control_tag = 470, /* server_access_control_tag  */
  YYSYMBOL_server_access_control_tag_action = 471, /* server_access_control_tag_action  */
  YYSYMBOL_server_access_control_tag_data = 472, /* server_access_control_tag_data  */
  YYSYMBOL_server_local_zone_override = 473, /* server_local_zone_override  */
  YYSYMBOL_server_interface_tag = 474,     /* server_interface_tag  */
  YYSYMBOL_server_interface_tag_action = 475, /* server_interface_tag_action  */
  YYSYMBOL_server_interface_tag_data = 476, /* server_interface_tag_data  */
  YYSYMBOL_server_ip_ratelimit = 477,      /* server_ip_ratelimit  */
  YYSYMBOL_server_ip_ratelimit_cookie = 478, /* server_ip_ratelimit_cookie  */
  YYSYMBOL_server_ratelimit = 479,         /* server_ratelimit  */
  YYSYMBOL_server_ip_ratelimit_size = 480, /* server_ip_ratelimit_size  */
  YYSYMBOL_server_ratelimit_size = 481,    /* server_ratelimit_size  */
  YYSYMBOL_server_ip_ratelimit_slabs = 482, /* server_ip_ratelimit_slabs  */
  YYSYMBOL_server_ratelimit_slabs = 483,   /* server_ratelimit_slabs  */
  YYSYMBOL_server_ratelimit_for_domain = 484, /* server_ratelimit_for_domain  */
  YYSYMBOL_server_ratelimit_below_domain = 485, /* server_ratelimit_below_domain  */
  YYSYMBOL_server_ip_ratelimit_factor = 486, /* server_ip_ratelimit_factor  */
  YYSYMBOL_server_ratelimit_factor = 487,  /* server_ratelimit_factor  */
  YYSYMBOL_server_ip_ratelimit_backoff = 488, /* server_ip_ratelimit_backoff  */
  YYSYMBOL_server_ratelimit_backoff = 489, /* server_ratelimit_backoff  */
  YYSYMBOL_server_outbound_msg_retry = 490, /* server_outbound_msg_retry  */
  YYSYMBOL_server_max_sent_count = 491,    /* server_max_sent_count  */
  YYSYMBOL_server_max_query_restarts = 492, /* server_max_query_restarts  */
  YYSYMBOL_server_low_rtt = 493,           /* server_low_rtt  */
  YYSYMBOL_server_fast_server_num = 494,   /* server_fast_server_num  */
  YYSYMBOL_server_fast_server_permil = 495, /* server_fast_server_permil  */
  YYSYMBOL_server_qname_minimisation = 496, /* server_qname_minimisation  */
  YYSYMBOL_server_qname_minimisation_strict = 497, /* server_qname_minimisation_strict  */
  YYSYMBOL_server_pad_queries = 498,       /* server_pad_queries  */
  YYSYMBOL_server_pad_queries_block_size = 499, /* server_pad_queries_block_size  */
  YYSYMBOL_server_ipsecmod_enabled = 500,  /* server_ipsecmod_enabled  */
  YYSYMBOL_server_ipsecmod_ignore_bogus = 501, /* server_ipsecmod_ignore_bogus  */
  YYSYMBOL_server_ipsecmod_hook = 502,     /* server_ipsecmod_hook  */
  YYSYMBOL_server_ipsecmod_max_ttl = 503,  /* server_ipsecmod_max_ttl  */
  YYSYMBOL_server_ipsecmod_whitelist = 504, /* server_ipsecmod_whitelist  */
  YYSYMBOL_server_ipsecmod_strict = 505,   /* server_ipsecmod_strict  */
  YYSYMBOL_server_edns_client_string = 506, /* server_edns_client_string  */
  YYSYMBOL_server_edns_client_string_opcode = 507, /* server_edns_client_string_opcode  */
  YYSYMBOL_server_ede = 508,               /* server_ede  */
  YYSYMBOL_server_dns_error_reporting = 509, /* server_dns_error_reporting  */
  YYSYMBOL_server_proxy_protocol_port = 510, /* server_proxy_protocol_port  */
  YYSYMBOL_stub_name = 511,                /* stub_name  */
  YYSYMBOL_stub_host = 512,                /* stub_host  */
  YYSYMBOL_stub_addr = 513,                /* stub_addr  */
  YYSYMBOL_stub_first = 514,               /* stub_first  */
  YYSYMBOL_stub_no_cache = 515,            /* stub_no_cache  */
  YYSYMBOL_stub_ssl_upstream = 516,        /* stub_ssl_upstream  */
  YYSYMBOL_stub_tcp_upstream = 517,        /* stub_tcp_upstream  */
  YYSYMBOL_stub_prime = 518,               /* stub_prime  */
  YYSYMBOL_forward_name = 519,             /* forward_name  */
  YYSYMBOL_forward_host = 520,             /* forward_host  */
  YYSYMBOL_forward_addr = 521,             /* forward_addr  */
  YYSYMBOL_forward_first = 522,            /* forward_first  */
  YYSYMBOL_forward_no_cache = 523,         /* forward_no_cache  */
  YYSYMBOL_forward_ssl_upstream = 524,     /* forward_ssl_upstream  */
  YYSYMBOL_forward_tcp_upstream = 525,     /* forward_tcp_upstream  */
  YYSYMBOL_auth_name = 526,                /* auth_name  */
  YYSYMBOL_auth_zonefile = 527,            /* auth_zonefile  */
  YYSYMBOL_auth_master = 528,              /* auth_master  */
  YYSYMBOL_auth_url = 529,                 /* auth_url  */
  YYSYMBOL_auth_allow_notify = 530,        /* auth_allow_notify  */
  YYSYMBOL_auth_zonemd_check = 531,        /* auth_zonemd_check  */
  YYSYMBOL_auth_zonemd_reject_absence = 532, /* auth_zonemd_reject_absence  */
  YYSYMBOL_auth_for_downstream = 533,      /* auth_for_downstream  */
  YYSYMBOL_auth_for_upstream = 534,        /* auth_for_upstream  */
  YYSYMBOL_auth_fallback_enabled = 535,    /* auth_fallback_enabled  */
  YYSYMBOL_rcstart = 536,                  /* rcstart  */
  YYSYMBOL_contents_rc = 537,              /* contents_rc  */
  YYSYMBOL_content_rc = 538,               /* content_rc  */
  YYSYMBOL_rc_control_enable = 539,        /* rc_control_enable  */
  YYSYMBOL_rc_control_port = 540,          /* rc_control_port  */
  YYSYMBOL_rc_control_interface = 541,     /* rc_control_interface  */
  YYSYMBOL_rc_control_use_cert = 542,      /* rc_control_use_cert  */
  YYSYMBOL_rc_server_key_file = 543,       /* rc_server_key_file  */
  YYSYMBOL_rc_server_cert_file = 544,      /* rc_server_cert_file  */
  YYSYMBOL_rc_control_key_file = 545,      /* rc_control_key_file  */
  YYSYMBOL_rc_control_cert_file = 546,     /* rc_control_cert_file  */
  YYSYMBOL_server_disable_dnssec_lame_check = 547, /* server_disable_dnssec_lame_check  */
  YYSYMBOL_server_log_identity = 548,      /* server_log_identity  */
  YYSYMBOL_server_tcp_connection_limit = 549, /* server_tcp_connection_limit  */
  YYSYMBOL_server_answer_cookie = 550,     /* server_answer_cookie  */
  YYSYMBOL_server_cookie_secret = 551,     /* server_cookie_secret  */
  YYSYMBOL_server_cookie_secret_file = 552, /* server_cookie_secret_file  */
  YYSYMBOL_server_iter_scrub_ns = 553,     /* server_iter_scrub_ns  */
  YYSYMBOL_server_iter_scrub_cname = 554,  /* server_iter_scrub_cname  */
  YYSYMBOL_server_iter_scrub_rrsig = 555,  /* server_iter_scrub_rrsig  */
  YYSYMBOL_server_max_global_quota = 556,  /* server_max_global_quota  */
  YYSYMBOL_server_iter_scrub_promiscuous = 557, /* server_iter_scrub_promiscuous  */
  YYSYMBOL_ipsetstart = 558,               /* ipsetstart  */
  YYSYMBOL_contents_ipset = 559,           /* contents_ipset  */
  YYSYMBOL_content_ipset = 560,            /* content_ipset  */
  YYSYMBOL_ipset_name_v4 = 561,            /* ipset_name_v4  */
  YYSYMBOL_ipset_name_v6 = 562             /* ipset_name_v6  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_int16 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if !defined yyoverflow

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* !defined yyoverflow */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  2
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   551

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  278
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  285
/* YYNRULES -- Number of rules.  */
#define YYNRULES  553
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  837

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   532


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int16 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,   139,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
     155,   156,   157,   158,   159,   160,   161,   162,   163,   164,
     165,   166,   167,   168,   169,   170,   171,   172,   173,   174,
     175,   176,   177,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,   198,   199,   200,   201,   202,   203,   204,
     205,   206,   207,   208,   209,   210,   211,   212,   213,   214,
     215,   216,   217,   218,   219,   220,   221,   222,   223,   224,
     225,   226,   227,   228,   229,   230,   231,   232,   233,   234,
     235,   236,   237,   238,   239,   240,   241,   242,   243,   244,
     245,   246,   247,   248,   249,   250,   251,   252,   253,   254,
     255,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   179,   179,   179,   180,   180,   181,   182,   182,   182,
     183,   185,   192,   198,   199,   200,   200,   200,   201,   201,
     202,   202,   202,   203,   203,   203,   204,   204,   204,   205,
     205,   206,   207,   207,   207,   208,   208,   208,   209,   209,
     210,   210,   211,   211,   212,   212,   213,   213,   214,   214,
     215,   215,   216,   216,   217,   217,   217,   218,   218,   219,
     219,   219,   220,   220,   220,   221,   221,   222,   222,   223,
     223,   224,   224,   225,   225,   225,   226,   226,   227,   227,
     228,   228,   228,   229,   229,   230,   230,   231,   231,   232,
     232,   232,   233,   233,   234,   234,   235,   235,   236,   236,
     237,   237,   238,   238,   239,   239,   240,   240,   241,   241,
     241,   242,   242,   242,   243,   243,   243,   244,   244,   244,
     244,   245,   245,   246,   246,   246,   247,   247,   247,   248,
     248,   249,   250,   250,   250,   251,   251,   251,   252,   252,
     253,   253,   254,   254,   255,   256,   256,   257,   257,   258,
     258,   259,   259,   260,   260,   261,   262,   262,   263,   263,
     263,   264,   264,   265,   265,   266,   266,   266,   267,   267,
     268,   269,   269,   270,   271,   271,   272,   272,   273,   273,
     273,   274,   274,   275,   275,   275,   276,   276,   277,   278,
     278,   279,   279,   280,   280,   281,   281,   282,   282,   283,
     283,   283,   284,   284,   284,   285,   285,   286,   286,   286,
     287,   287,   288,   288,   288,   288,   289,   289,   289,   290,
     290,   291,   291,   292,   292,   293,   293,   294,   295,   295,
     296,   296,   297,   297,   298,   298,   298,   299,   300,   300,
     300,   302,   310,   324,   325,   326,   326,   326,   326,   326,
     327,   327,   327,   329,   337,   351,   352,   353,   353,   353,
     353,   354,   354,   354,   356,   376,   377,   378,   378,   378,
     378,   379,   379,   379,   380,   380,   380,   383,   392,   401,
     412,   421,   430,   439,   448,   459,   468,   481,   496,   505,
     514,   523,   532,   541,   550,   557,   566,   575,   584,   593,
     602,   611,   620,   629,   638,   651,   662,   673,   684,   693,
     706,   719,   728,   737,   746,   753,   762,   771,   780,   789,
     798,   812,   821,   830,   839,   848,   857,   866,   875,   884,
     893,   900,   907,   933,   941,   948,   955,   962,   969,   977,
     985,   993,  1000,  1011,  1022,  1029,  1038,  1047,  1056,  1065,
    1072,  1079,  1086,  1102,  1110,  1118,  1128,  1138,  1148,  1162,
    1170,  1183,  1194,  1202,  1215,  1224,  1233,  1242,  1251,  1261,
    1271,  1279,  1292,  1301,  1309,  1318,  1326,  1339,  1348,  1357,
    1367,  1374,  1384,  1394,  1404,  1414,  1424,  1434,  1444,  1454,
    1464,  1474,  1481,  1488,  1495,  1504,  1513,  1522,  1531,  1538,
    1548,  1556,  1565,  1572,  1590,  1603,  1616,  1629,  1638,  1647,
    1656,  1665,  1674,  1684,  1694,  1705,  1714,  1723,  1732,  1741,
    1750,  1759,  1768,  1777,  1786,  1799,  1812,  1821,  1828,  1837,
    1846,  1855,  1864,  1874,  1882,  1895,  1903,  1958,  1965,  1980,
    1990,  2000,  2007,  2014,  2021,  2028,  2043,  2058,  2065,  2072,
    2086,  2107,  2128,  2140,  2152,  2164,  2185,  2197,  2209,  2218,
    2227,  2236,  2244,  2252,  2265,  2278,  2293,  2308,  2317,  2326,
    2336,  2346,  2355,  2364,  2373,  2379,  2388,  2397,  2407,  2417,
    2427,  2436,  2449,  2462,  2474,  2488,  2500,  2514,  2523,  2534,
    2543,  2552,  2559,  2569,  2576,  2583,  2592,  2601,  2611,  2621,
    2631,  2641,  2648,  2655,  2664,  2673,  2683,  2693,  2703,  2710,
    2717,  2724,  2732,  2742,  2752,  2762,  2772,  2782,  2788,  2789,
    2790,  2790,  2790,  2791,  2791,  2791,  2792,  2792,  2794,  2804,
    2813,  2820,  2827,  2834,  2841,  2848,  2855,  2865,  2872,  2883,
    2892,  2908,  2915,  2924,  2933,  2942,  2951,  2961,  2967,  2968,
    2969,  2969,  2971,  2986
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "SPACE", "LETTER",
  "NEWLINE", "COMMENT", "COLON", "ANY", "ZONESTR", "STRING_ARG",
  "VAR_FORCE_TOPLEVEL", "VAR_SERVER", "VAR_VERBOSITY", "VAR_NUM_THREADS",
  "VAR_PORT", "VAR_OUTGOING_RANGE", "VAR_INTERFACE", "VAR_PREFER_IP4",
  "VAR_DO_IP4", "VAR_DO_IP6", "VAR_DO_NAT64", "VAR_PREFER_IP6",
  "VAR_DO_UDP", "VAR_DO_TCP", "VAR_TCP_MSS", "VAR_OUTGOING_TCP_MSS",
  "VAR_TCP_IDLE_TIMEOUT", "VAR_EDNS_TCP_KEEPALIVE",
  "VAR_EDNS_TCP_KEEPALIVE_TIMEOUT", "VAR_SOCK_QUEUE_TIMEOUT", "VAR_CHROOT",
  "VAR_USERNAME", "VAR_DIRECTORY", "VAR_LOGFILE", "VAR_PIDFILE",
  "VAR_MSG_CACHE_SIZE", "VAR_MSG_CACHE_SLABS",
  "VAR_NUM_QUERIES_PER_THREAD", "VAR_RRSET_CACHE_SIZE",
  "VAR_RRSET_CACHE_SLABS", "VAR_OUTGOING_NUM_TCP", "VAR_INFRA_HOST_TTL",
  "VAR_INFRA_LAME_TTL", "VAR_INFRA_CACHE_SLABS",
  "VAR_INFRA_CACHE_NUMHOSTS", "VAR_INFRA_CACHE_LAME_SIZE", "VAR_NAME",
  "VAR_STUB_ZONE", "VAR_STUB_HOST", "VAR_STUB_ADDR",
  "VAR_TARGET_FETCH_POLICY", "VAR_HARDEN_SHORT_BUFSIZE",
  "VAR_HARDEN_LARGE_QUERIES", "VAR_FORWARD_ZONE", "VAR_FORWARD_HOST",
  "VAR_FORWARD_ADDR", "VAR_DO_NOT_QUERY_ADDRESS", "VAR_HIDE_IDENTITY",
  "VAR_HIDE_VERSION", "VAR_IDENTITY", "VAR_VERSION", "VAR_HARDEN_GLUE",
  "VAR_MODULE_CONF", "VAR_TRUST_ANCHOR_FILE", "VAR_TRUST_ANCHOR",
  "VAR_VAL_OVERRIDE_DATE", "VAR_BOGUS_TTL", "VAR_VAL_CLEAN_ADDITIONAL",
  "VAR_VAL_PERMISSIVE_MODE", "VAR_INCOMING_NUM_TCP", "VAR_MSG_BUFFER_SIZE",
  "VAR_KEY_CACHE_SIZE", "VAR_KEY_CACHE_SLABS", "VAR_TRUSTED_KEYS_FILE",
  "VAR_VAL_NSEC3_KEYSIZE_ITERATIONS", "VAR_USE_SYSLOG",
  "VAR_OUTGOING_INTERFACE", "VAR_ROOT_HINTS", "VAR_DO_NOT_QUERY_LOCALHOST",
  "VAR_CACHE_MAX_TTL", "VAR_HARDEN_DNSSEC_STRIPPED", "VAR_ACCESS_CONTROL",
  "VAR_LOCAL_ZONE", "VAR_LOCAL_DATA", "VAR_INTERFACE_AUTOMATIC",
  "VAR_STATISTICS_INTERVAL", "VAR_DO_DAEMONIZE", "VAR_USE_CAPS_FOR_ID",
  "VAR_STATISTICS_CUMULATIVE", "VAR_OUTGOING_PORT_PERMIT",
  "VAR_OUTGOING_PORT_AVOID", "VAR_DLV_ANCHOR_FILE", "VAR_DLV_ANCHOR",
  "VAR_NEG_CACHE_SIZE", "VAR_HARDEN_REFERRAL_PATH", "VAR_PRIVATE_ADDRESS",
  "VAR_PRIVATE_DOMAIN", "VAR_REMOTE_CONTROL", "VAR_CONTROL_ENABLE",
  "VAR_CONTROL_INTERFACE", "VAR_CONTROL_PORT", "VAR_SERVER_KEY_FILE",
  "VAR_SERVER_CERT_FILE", "VAR_CONTROL_KEY_FILE", "VAR_CONTROL_CERT_FILE",
  "VAR_CONTROL_USE_CERT", "VAR_TCP_REUSE_TIMEOUT",
  "VAR_MAX_REUSE_TCP_QUERIES", "VAR_EXTENDED_STATISTICS",
  "VAR_LOCAL_DATA_PTR", "VAR_JOSTLE_TIMEOUT", "VAR_STUB_PRIME",
  "VAR_UNWANTED_REPLY_THRESHOLD", "VAR_LOG_TIME_ASCII",
  "VAR_DOMAIN_INSECURE", "VAR_VAL_SIG_SKEW_MIN", "VAR_VAL_SIG_SKEW_MAX",
  "VAR_VAL_MAX_RESTART", "VAR_CACHE_MIN_TTL", "VAR_VAL_LOG_LEVEL",
  "VAR_AUTO_TRUST_ANCHOR_FILE", "VAR_KEEP_MISSING", "VAR_ADD_HOLDDOWN",
  "VAR_DEL_HOLDDOWN", "VAR_SO_RCVBUF", "VAR_EDNS_BUFFER_SIZE",
  "VAR_PREFETCH", "VAR_PREFETCH_KEY", "VAR_SO_SNDBUF", "VAR_SO_REUSEPORT",
  "VAR_HARDEN_BELOW_NXDOMAIN", "VAR_IGNORE_CD_FLAG", "VAR_LOG_QUERIES",
  "VAR_LOG_REPLIES", "VAR_LOG_LOCAL_ACTIONS", "VAR_TCP_UPSTREAM",
  "VAR_SSL_UPSTREAM", "VAR_TCP_AUTH_QUERY_TIMEOUT", "VAR_FORWARD_FIRST",
  "VAR_STUB_SSL_UPSTREAM", "VAR_FORWARD_SSL_UPSTREAM",
  "VAR_TLS_CERT_BUNDLE", "VAR_STUB_TCP_UPSTREAM",
  "VAR_FORWARD_TCP_UPSTREAM", "VAR_STUB_FIRST", "VAR_MINIMAL_RESPONSES",
  "VAR_RRSET_ROUNDROBIN", "VAR_MAX_UDP_SIZE", "VAR_DELAY_CLOSE",
  "VAR_UDP_CONNECT", "VAR_UNBLOCK_LAN_ZONES", "VAR_INSECURE_LAN_ZONES",
  "VAR_INFRA_CACHE_MIN_RTT", "VAR_INFRA_CACHE_MAX_RTT",
  "VAR_INFRA_KEEP_PROBING", "VAR_NAT64_PREFIX",
  "VAR_HARDEN_ALGO_DOWNGRADE", "VAR_IP_TRANSPARENT", "VAR_IP_DSCP",
  "VAR_DISABLE_DNSSEC_LAME_CHECK", "VAR_IP_RATELIMIT",
  "VAR_IP_RATELIMIT_SLABS", "VAR_IP_RATELIMIT_SIZE", "VAR_RATELIMIT",
  "VAR_RATELIMIT_SLABS", "VAR_RATELIMIT_SIZE", "VAR_OUTBOUND_MSG_RETRY",
  "VAR_MAX_SENT_COUNT", "VAR_MAX_QUERY_RESTARTS",
  "VAR_RATELIMIT_FOR_DOMAIN", "VAR_RATELIMIT_BELOW_DOMAIN",
  "VAR_IP_RATELIMIT_FACTOR", "VAR_RATELIMIT_FACTOR",
  "VAR_IP_RATELIMIT_BACKOFF", "VAR_RATELIMIT_BACKOFF",
  "VAR_CAPS_WHITELIST", "VAR_CACHE_MAX_NEGATIVE_TTL",
  "VAR_PERMIT_SMALL_HOLDDOWN", "VAR_CACHE_MIN_NEGATIVE_TTL",
  "VAR_QNAME_MINIMISATION", "VAR_QNAME_MINIMISATION_STRICT",
  "VAR_IP_FREEBIND", "VAR_DEFINE_TAG", "VAR_LOCAL_ZONE_TAG",
  "VAR_ACCESS_CONTROL_TAG", "VAR_LOCAL_ZONE_OVERRIDE",
  "VAR_ACCESS_CONTROL_TAG_ACTION", "VAR_ACCESS_CONTROL_TAG_DATA",
  "VAR_SERVE_EXPIRED", "VAR_SERVE_EXPIRED_TTL",
  "VAR_SERVE_EXPIRED_TTL_RESET", "VAR_SERVE_EXPIRED_REPLY_TTL",
  "VAR_SERVE_EXPIRED_CLIENT_TIMEOUT", "VAR_EDE_SERVE_EXPIRED",
  "VAR_SERVE_ORIGINAL_TTL", "VAR_FAKE_DSA", "VAR_FAKE_SHA1",
  "VAR_LOG_IDENTITY", "VAR_HIDE_TRUSTANCHOR", "VAR_HIDE_HTTP_USER_AGENT",
  "VAR_HTTP_USER_AGENT", "VAR_TRUST_ANCHOR_SIGNALING",
  "VAR_AGGRESSIVE_NSEC", "VAR_USE_SYSTEMD", "VAR_SHM_ENABLE",
  "VAR_SHM_KEY", "VAR_ROOT_KEY_SENTINEL", "VAR_PAD_QUERIES",
  "VAR_PAD_QUERIES_BLOCK_SIZE", "VAR_IPSECMOD_ENABLED",
  "VAR_IPSECMOD_HOOK", "VAR_IPSECMOD_IGNORE_BOGUS", "VAR_IPSECMOD_MAX_TTL",
  "VAR_IPSECMOD_WHITELIST", "VAR_IPSECMOD_STRICT",
  "VAR_UDP_UPSTREAM_WITHOUT_DOWNSTREAM", "VAR_FOR_UPSTREAM",
  "VAR_AUTH_ZONE", "VAR_ZONEFILE", "VAR_MASTER", "VAR_URL",
  "VAR_FOR_DOWNSTREAM", "VAR_FALLBACK_ENABLED", "VAR_LOW_RTT",
  "VAR_LOW_RTT_PERMIL", "VAR_FAST_SERVER_PERMIL", "VAR_FAST_SERVER_NUM",
  "VAR_ALLOW_NOTIFY", "VAR_TLS_WIN_CERT", "VAR_TCP_CONNECTION_LIMIT",
  "VAR_ANSWER_COOKIE", "VAR_COOKIE_SECRET", "VAR_IP_RATELIMIT_COOKIE",
  "VAR_FORWARD_NO_CACHE", "VAR_STUB_NO_CACHE", "VAR_LOG_SERVFAIL",
  "VAR_DENY_ANY", "VAR_UNKNOWN_SERVER_TIME_LIMIT",
  "VAR_LOG_TAG_QUERYREPLY", "VAR_DISCARD_TIMEOUT", "VAR_WAIT_LIMIT",
  "VAR_WAIT_LIMIT_COOKIE", "VAR_WAIT_LIMIT_NETBLOCK",
  "VAR_WAIT_LIMIT_COOKIE_NETBLOCK", "VAR_STREAM_WAIT_SIZE",
  "VAR_TLS_USE_SNI", "VAR_TLS_PROTOCOLS", "VAR_IPSET", "VAR_IPSET_NAME_V4",
  "VAR_IPSET_NAME_V6", "VAR_EDNS_CLIENT_STRING",
  "VAR_EDNS_CLIENT_STRING_OPCODE", "VAR_NSID",
  "VAR_ZONEMD_PERMISSIVE_MODE", "VAR_ZONEMD_CHECK",
  "VAR_ZONEMD_REJECT_ABSENCE", "VAR_INTERFACE_AUTOMATIC_PORTS", "VAR_EDE",
  "VAR_DNS_ERROR_REPORTING", "VAR_INTERFACE_ACTION", "VAR_INTERFACE_TAG",
  "VAR_INTERFACE_TAG_ACTION", "VAR_INTERFACE_TAG_DATA",
  "VAR_PROXY_PROTOCOL_PORT", "VAR_STATISTICS_INHIBIT_ZERO",
  "VAR_HARDEN_UNKNOWN_ADDITIONAL", "VAR_DISABLE_EDNS_DO",
  "VAR_LOG_DESTADDR", "VAR_COOKIE_SECRET_FILE", "VAR_ITER_SCRUB_NS",
  "VAR_ITER_SCRUB_CNAME", "VAR_ITER_SCRUB_RRSIG", "VAR_MAX_GLOBAL_QUOTA",
  "VAR_HARDEN_UNVERIFIED_GLUE", "VAR_LOG_TIME_ISO",
  "VAR_ITER_SCRUB_PROMISCUOUS", "VAR_LOG_THREAD_ID", "$accept",
  "toplevelvars", "toplevelvar", "force_toplevel", "serverstart",
  "contents_server", "content_server", "stub_clause", "stubstart",
  "contents_stub", "content_stub", "forward_clause", "forwardstart",
  "contents_forward", "content_forward", "authstart", "contents_auth",
  "content_auth", "server_num_threads", "server_verbosity",
  "server_statistics_interval", "server_statistics_cumulative",
  "server_extended_statistics", "server_statistics_inhibit_zero",
  "server_shm_enable", "server_shm_key", "server_port", "server_interface",
  "server_outgoing_interface", "server_outgoing_range",
  "server_outgoing_port_permit", "server_outgoing_port_avoid",
  "server_outgoing_num_tcp", "server_incoming_num_tcp",
  "server_interface_automatic", "server_interface_automatic_ports",
  "server_do_ip4", "server_do_ip6", "server_do_nat64", "server_do_udp",
  "server_do_tcp", "server_prefer_ip4", "server_prefer_ip6",
  "server_tcp_mss", "server_outgoing_tcp_mss", "server_tcp_idle_timeout",
  "server_max_reuse_tcp_queries", "server_tcp_reuse_timeout",
  "server_tcp_auth_query_timeout", "server_tcp_keepalive",
  "server_tcp_keepalive_timeout", "server_sock_queue_timeout",
  "server_tcp_upstream", "server_udp_upstream_without_downstream",
  "server_ssl_upstream", "server_tls_cert_bundle", "server_tls_win_cert",
  "server_tls_use_sni", "server_tls_protocols", "server_use_systemd",
  "server_do_daemonize", "server_use_syslog", "server_log_time_ascii",
  "server_log_time_iso", "server_log_queries", "server_log_replies",
  "server_log_tag_queryreply", "server_log_servfail",
  "server_log_destaddr", "server_log_thread_id",
  "server_log_local_actions", "server_chroot", "server_username",
  "server_directory", "server_logfile", "server_pidfile",
  "server_root_hints", "server_dlv_anchor_file", "server_dlv_anchor",
  "server_auto_trust_anchor_file", "server_trust_anchor_file",
  "server_trusted_keys_file", "server_trust_anchor",
  "server_trust_anchor_signaling", "server_root_key_sentinel",
  "server_domain_insecure", "server_hide_identity", "server_hide_version",
  "server_hide_trustanchor", "server_hide_http_user_agent",
  "server_identity", "server_version", "server_http_user_agent",
  "server_nsid", "server_so_rcvbuf", "server_so_sndbuf",
  "server_so_reuseport", "server_ip_transparent", "server_ip_freebind",
  "server_ip_dscp", "server_stream_wait_size", "server_edns_buffer_size",
  "server_msg_buffer_size", "server_msg_cache_size",
  "server_msg_cache_slabs", "server_num_queries_per_thread",
  "server_jostle_timeout", "server_delay_close", "server_udp_connect",
  "server_unblock_lan_zones", "server_insecure_lan_zones",
  "server_rrset_cache_size", "server_rrset_cache_slabs",
  "server_infra_host_ttl", "server_infra_lame_ttl",
  "server_infra_cache_numhosts", "server_infra_cache_lame_size",
  "server_infra_cache_slabs", "server_infra_cache_min_rtt",
  "server_infra_cache_max_rtt", "server_infra_keep_probing",
  "server_target_fetch_policy", "server_harden_short_bufsize",
  "server_harden_large_queries", "server_harden_glue",
  "server_harden_unverified_glue", "server_harden_dnssec_stripped",
  "server_harden_below_nxdomain", "server_harden_referral_path",
  "server_harden_algo_downgrade", "server_harden_unknown_additional",
  "server_use_caps_for_id", "server_caps_whitelist",
  "server_private_address", "server_private_domain", "server_prefetch",
  "server_prefetch_key", "server_deny_any",
  "server_unwanted_reply_threshold", "server_do_not_query_address",
  "server_do_not_query_localhost", "server_access_control",
  "server_interface_action", "server_module_conf",
  "server_val_override_date", "server_val_sig_skew_min",
  "server_val_sig_skew_max", "server_val_max_restart",
  "server_cache_max_ttl", "server_cache_max_negative_ttl",
  "server_cache_min_negative_ttl", "server_cache_min_ttl",
  "server_bogus_ttl", "server_val_clean_additional",
  "server_val_permissive_mode", "server_aggressive_nsec",
  "server_ignore_cd_flag", "server_disable_edns_do",
  "server_serve_expired", "server_serve_expired_ttl",
  "server_serve_expired_ttl_reset", "server_serve_expired_reply_ttl",
  "server_serve_expired_client_timeout", "server_ede_serve_expired",
  "server_serve_original_ttl", "server_fake_dsa", "server_fake_sha1",
  "server_val_log_level", "server_val_nsec3_keysize_iterations",
  "server_zonemd_permissive_mode", "server_add_holddown",
  "server_del_holddown", "server_keep_missing",
  "server_permit_small_holddown", "server_key_cache_size",
  "server_key_cache_slabs", "server_neg_cache_size", "server_local_zone",
  "server_local_data", "server_local_data_ptr", "server_minimal_responses",
  "server_rrset_roundrobin", "server_unknown_server_time_limit",
  "server_discard_timeout", "server_wait_limit",
  "server_wait_limit_cookie", "server_wait_limit_netblock",
  "server_wait_limit_cookie_netblock", "server_max_udp_size",
  "server_nat64_prefix", "server_define_tag", "server_local_zone_tag",
  "server_access_control_tag", "server_access_control_tag_action",
  "server_access_control_tag_data", "server_local_zone_override",
  "server_interface_tag", "server_interface_tag_action",
  "server_interface_tag_data", "server_ip_ratelimit",
  "server_ip_ratelimit_cookie", "server_ratelimit",
  "server_ip_ratelimit_size", "server_ratelimit_size",
  "server_ip_ratelimit_slabs", "server_ratelimit_slabs",
  "server_ratelimit_for_domain", "server_ratelimit_below_domain",
  "server_ip_ratelimit_factor", "server_ratelimit_factor",
  "server_ip_ratelimit_backoff", "server_ratelimit_backoff",
  "server_outbound_msg_retry", "server_max_sent_count",
  "server_max_query_restarts", "server_low_rtt", "server_fast_server_num",
  "server_fast_server_permil", "server_qname_minimisation",
  "server_qname_minimisation_strict", "server_pad_queries",
  "server_pad_queries_block_size", "server_ipsecmod_enabled",
  "server_ipsecmod_ignore_bogus", "server_ipsecmod_hook",
  "server_ipsecmod_max_ttl", "server_ipsecmod_whitelist",
  "server_ipsecmod_strict", "server_edns_client_string",
  "server_edns_client_string_opcode", "server_ede",
  "server_dns_error_reporting", "server_proxy_protocol_port", "stub_name",
  "stub_host", "stub_addr", "stub_first", "stub_no_cache",
  "stub_ssl_upstream", "stub_tcp_upstream", "stub_prime", "forward_name",
  "forward_host", "forward_addr", "forward_first", "forward_no_cache",
  "forward_ssl_upstream", "forward_tcp_upstream", "auth_name",
  "auth_zonefile", "auth_master", "auth_url", "auth_allow_notify",
  "auth_zonemd_check", "auth_zonemd_reject_absence", "auth_for_downstream",
  "auth_for_upstream", "auth_fallback_enabled", "rcstart", "contents_rc",
  "content_rc", "rc_control_enable", "rc_control_port",
  "rc_control_interface", "rc_control_use_cert", "rc_server_key_file",
  "rc_server_cert_file", "rc_control_key_file", "rc_control_cert_file",
  "server_disable_dnssec_lame_check", "server_log_identity",
  "server_tcp_connection_limit", "server_answer_cookie",
  "server_cookie_secret", "server_cookie_secret_file",
  "server_iter_scrub_ns", "server_iter_scrub_cname",
  "server_iter_scrub_rrsig", "server_max_global_quota",
  "server_iter_scrub_promiscuous", "ipsetstart", "contents_ipset",
  "content_ipset", "ipset_name_v4", "ipset_name_v6", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-216)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -216,   265,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
     -13,   158,   231,    52,   -14,  -215,    26,    27,    31,    32,
      33,    83,   116,   117,   118,   120,   121,   122,   194,   196,
     199,   200,   202,   205,   211,   212,   225,   226,   227,   232,
     233,   256,   257,   258,   269,   271,   272,   273,   274,   275,
     278,   279,   280,   281,   282,   283,   284,   285,   286,   287,
     289,   290,   292,   294,   295,   296,   299,   300,   301,   302,
     304,   305,   306,   307,   308,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   354,   355,
     356,   357,   358,   359,   361,   363,   364,   366,   367,   368,
     369,   370,   371,   372,   373,   374,   375,   376,   377,   378,
     379,   380,   381,   382,   384,   385,   386,   387,   388,   389,
     390,   391,   392,   393,   394,   395,   396,   397,   398,   399,
     400,   401,   402,   403,   404,   405,   406,   407,   408,   409,
     410,   411,   412,   413,   414,   415,   416,   417,   418,   419,
     420,   421,   422,   423,   424,   425,   426,   427,   428,   429,
     430,   431,   432,   433,   434,   435,   436,   437,   438,   439,
     440,   441,   442,   443,   444,   445,   446,   447,   448,   449,
     450,   451,   452,   453,   454,   456,   457,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,   469,   470,
     471,   472,   474,   475,   476,   477,   478,   479,   480,   481,
     482,   483,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,   484,
     485,   486,   487,   488,   489,   490,   491,  -216,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,  -216,   492,   493,   494,   495,
     496,   497,   498,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,   499,   500,   501,   502,   504,   505,   506,   507,   508,
     509,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,  -216,   510,   511,   512,   513,   514,   515,   516,   517,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,   518,
     519,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,  -216,   520,   521,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,   522,   523,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,  -216,   524,   525,   526,   527,   528,  -216,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,   529,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
     530,   531,  -216,  -216,  -216,   532,  -216,  -216,  -216,  -216,
    -216,  -216,   533,   534,   535,   536,  -216,  -216,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,   537,   538,   539,  -216,  -216,  -216,  -216,  -216,  -216,
     540,   541,  -216,  -216,  -216,  -216,  -216
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int16 yydefact[] =
{
       2,     0,     1,    11,    12,   242,   254,   517,   264,   547,
       3,    10,    14,     5,   244,     6,   256,   266,   519,   549,
       4,   241,   253,     9,     7,     8,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    13,    15,    16,    83,    86,    95,   229,   183,
     184,    17,    32,    74,    18,    87,    88,    43,    67,    82,
     225,    19,    20,    21,    24,    25,    22,    23,    26,    27,
      28,   222,   223,   224,    29,    30,    31,   119,   195,   120,
     197,   201,   216,   218,   182,    84,    73,    99,   239,   117,
     118,   206,   203,   232,   122,   121,    33,    34,    35,    36,
      37,    75,    89,    90,   106,    61,    71,    62,   187,   188,
     100,    53,    54,   186,    57,    55,    56,    58,   220,   110,
     114,   126,   135,   158,   138,   212,   111,    68,    38,    39,
      40,    97,   127,   128,   129,   130,    41,    42,    44,    45,
      47,    48,    46,   132,   133,   139,    49,    50,    51,    59,
     238,    78,   115,    92,   134,   230,    85,   153,    93,    94,
     112,   113,   204,    98,    52,    76,    79,   166,    60,    63,
     101,   102,   103,    77,   154,   155,   104,    64,    65,    66,
     196,   116,   231,   173,   174,   175,   176,   177,   178,   179,
     180,   185,   105,    72,   221,   107,   108,   109,   156,    69,
      70,    91,    80,    81,    96,   123,   124,   205,   207,   208,
     209,   210,   211,   125,   131,   159,   160,   162,   164,   165,
     163,   167,   168,   169,   136,   215,   137,   142,   143,   140,
     141,   144,   145,   147,   146,   149,   148,   150,   151,   152,
     198,   200,   199,   157,   170,   171,   172,   189,   191,   190,
     192,   193,   194,   217,   219,   226,   227,   228,   161,   181,
     202,   213,   214,   233,   234,   235,   237,   236,   240,     0,
       0,     0,     0,     0,     0,     0,     0,   243,   245,   246,
     247,   249,   250,   251,   252,   248,     0,     0,     0,     0,
       0,     0,     0,   255,   257,   258,   259,   260,   261,   262,
     263,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   265,   267,   268,   269,   270,   274,   275,   276,   271,
     272,   273,     0,     0,     0,     0,     0,     0,     0,     0,
     518,   520,   522,   521,   527,   523,   524,   525,   526,     0,
       0,   548,   550,   551,   278,   277,   285,   288,   286,   300,
     295,   296,   297,   301,   298,   299,   302,   303,   304,   308,
     309,   310,   330,   331,   332,   333,   334,   362,   363,   364,
     370,   371,   291,   372,   373,   376,   374,   375,   380,   381,
     382,   398,   345,   346,   349,   350,   383,   402,   339,   341,
     403,   411,   412,   413,   292,   361,   433,   434,   340,   427,
     320,   287,   335,   399,   407,   385,     0,     0,   437,   293,
     279,   319,   390,   280,   289,   290,   336,   337,   435,   387,
     392,   393,   306,   305,   281,   438,   365,   397,   321,   344,
     404,   405,   406,   410,   426,   338,   431,   429,   430,   353,
     360,   394,   395,   354,   355,   386,   415,   323,   324,   329,
     311,   313,   307,   314,   439,   440,   447,   366,   367,   368,
     369,   377,   378,   379,   448,   388,   356,   358,   536,   458,
     463,   461,   460,   464,   462,   471,   472,   473,     0,     0,
     467,   468,   469,   470,   391,   408,   432,   409,   477,   478,
     357,   449,     0,     0,     0,     0,     0,   417,   418,   419,
     420,   421,   422,   423,   424,   425,   537,   347,   348,   351,
     342,   414,   318,   283,   284,   343,   479,   480,   481,   483,
     482,   484,   485,   486,   312,   474,   476,   475,   315,     0,
     539,   540,   459,   326,   396,   441,   325,   442,   443,   444,
       0,     0,   359,   316,   317,     0,   488,   352,   428,   294,
     489,   490,     0,     0,     0,     0,   491,   282,   389,   416,
     327,   541,   542,   543,   544,   545,   384,   322,   546,   328,
     492,   493,   494,   499,   497,   498,   495,   496,   500,   501,
     502,   503,   505,   506,   504,   507,   515,   508,   509,   510,
     514,   516,   511,   512,   513,   528,   530,   529,   532,   533,
     534,   535,   531,   552,   553,   400,   436,   465,   466,   450,
     451,     0,     0,     0,   538,   445,   446,   487,   401,   455,
       0,     0,   454,   452,   453,   456,   457
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,  -216,  -216,  -216,  -216
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,     1,    10,    11,    12,    20,   252,    13,    14,    21,
     487,    15,    16,    22,   503,    17,    23,   521,   253,   254,
     255,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
     385,   386,   387,   388,   389,   390,   391,   392,   393,   394,
     395,   396,   397,   398,   399,   400,   401,   402,   403,   404,
     405,   406,   407,   408,   409,   410,   411,   412,   413,   414,
     415,   416,   417,   418,   419,   420,   421,   422,   423,   424,
     425,   426,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   488,   489,   490,   491,   492,   493,   494,
     495,   504,   505,   506,   507,   508,   509,   510,   522,   523,
     524,   525,   526,   527,   528,   529,   530,   531,    18,    24,
     540,   541,   542,   543,   544,   545,   546,   547,   548,   468,
     469,   470,   471,   472,   473,   474,   475,   476,   477,   478,
      19,    25,   551,   552,   553
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,   549,   550,   554,   555,    60,    61,
      62,   556,   557,   558,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   532,   533,   534,   535,   536,
     537,   538,   539,   559,   104,   105,   106,   107,   108,   511,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   132,   133,   134,   560,   561,   562,   135,
     563,   564,   565,   136,   137,   138,   139,   140,   141,   142,
     143,   144,   145,   146,   147,   148,   149,   150,   151,   152,
     153,   154,   155,   156,   157,   158,   159,   160,   161,   162,
     163,   164,   165,   166,   167,   168,   169,   170,   171,   172,
     173,   174,   175,   176,   177,   178,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,   197,   198,   199,   200,   201,   202,
     203,   204,   205,   206,   566,   479,   567,   480,   481,   568,
     569,   207,   570,   208,   209,   571,   210,   211,   212,   213,
     214,   572,   573,   215,   216,   217,   218,   219,   220,   221,
     222,   223,   224,   225,   226,   574,   575,   576,   227,   228,
     229,   230,   577,   578,   231,   232,   233,   234,   235,   236,
     237,   238,   239,   240,   241,   242,   243,   244,   245,   246,
     247,   248,   249,   250,   251,     2,   579,   580,   581,   512,
     482,   513,   514,   515,   516,   517,     3,     4,   496,   582,
     518,   583,   584,   585,   586,   587,   497,   498,   588,   589,
     590,   591,   592,   593,   594,   595,   596,   597,   483,   598,
     599,   484,   600,   485,   601,   602,   603,   519,   520,   604,
     605,   606,   607,     5,   608,   609,   610,   611,   612,     6,
     613,   614,   615,   616,   617,   618,   619,   620,   621,   622,
     623,   624,   625,   626,   627,   628,   629,   630,   631,   632,
     633,   634,   635,   636,   637,   638,   639,   640,   641,   642,
     643,   644,   645,   646,   647,   648,   649,   650,   651,   652,
     653,   654,   655,     7,   656,   657,   658,   659,   660,   661,
     499,   662,   500,   663,   664,   501,   665,   666,   667,   668,
     669,   670,   671,   672,   673,   674,   675,   676,   677,   678,
     679,   680,   681,   486,   682,   683,   684,   685,   686,   687,
     688,   689,   690,   691,   692,   693,   694,   695,   696,   697,
     698,   699,   700,   701,   702,   703,   704,   705,   706,   707,
     708,   709,   710,   711,   712,   713,   714,   715,   716,   717,
     718,   719,   720,   721,   722,   723,   724,   725,   726,   727,
     728,   729,   730,   731,   732,   733,   734,   735,   736,   737,
     738,   739,   740,   741,   742,   743,   744,   745,   746,   747,
     748,   749,   750,   751,   752,   502,   753,   754,   755,   756,
     757,   758,   759,   760,   761,   762,   763,   764,   765,   766,
     767,   768,   769,     8,   770,   771,   772,   773,   774,   775,
     776,   777,   778,   779,   780,   781,   782,   783,   784,   785,
     786,   787,   788,   789,   790,   791,   792,   793,   794,   795,
     796,   797,   798,     9,   799,   800,   801,   802,   803,   804,
     805,   806,   807,   808,   809,   810,   811,   812,   813,   814,
     815,   816,   817,   818,   819,   820,   821,   822,   823,   824,
     825,   826,   827,   828,   829,   830,   831,   832,   833,   834,
     835,   836
};

static const yytype_int16 yycheck[] =
{
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,   249,   250,    10,    10,    51,    52,
      53,    10,    10,    10,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    99,   100,   101,   102,   103,
     104,   105,   106,    10,   107,   108,   109,   110,   111,    47,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   128,   129,   130,   131,   132,
     133,   134,   135,   136,   137,   138,    10,    10,    10,   142,
      10,    10,    10,   146,   147,   148,   149,   150,   151,   152,
     153,   154,   155,   156,   157,   158,   159,   160,   161,   162,
     163,   164,   165,   166,   167,   168,   169,   170,   171,   172,
     173,   174,   175,   176,   177,   178,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,   197,   198,   199,   200,   201,   202,
     203,   204,   205,   206,   207,   208,   209,   210,   211,   212,
     213,   214,   215,   216,    10,    47,    10,    49,    50,    10,
      10,   224,    10,   226,   227,    10,   229,   230,   231,   232,
     233,    10,    10,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,    10,    10,    10,   251,   252,
     253,   254,    10,    10,   257,   258,   259,   260,   261,   262,
     263,   264,   265,   266,   267,   268,   269,   270,   271,   272,
     273,   274,   275,   276,   277,     0,    10,    10,    10,   217,
     112,   219,   220,   221,   222,   223,    11,    12,    47,    10,
     228,    10,    10,    10,    10,    10,    55,    56,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,   140,    10,
      10,   143,    10,   145,    10,    10,    10,   255,   256,    10,
      10,    10,    10,    48,    10,    10,    10,    10,    10,    54,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    98,    10,    10,    10,    10,    10,    10,
     139,    10,   141,    10,    10,   144,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,   235,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,   234,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,   218,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,   248,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int16 yystos[] =
{
       0,   279,     0,    11,    12,    48,    54,    98,   218,   248,
     280,   281,   282,   285,   286,   289,   290,   293,   536,   558,
     283,   287,   291,   294,   537,   559,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      51,    52,    53,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,   107,   108,   109,   110,   111,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   138,   142,   146,   147,   148,   149,
     150,   151,   152,   153,   154,   155,   156,   157,   158,   159,
     160,   161,   162,   163,   164,   165,   166,   167,   168,   169,
     170,   171,   172,   173,   174,   175,   176,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,   198,   199,
     200,   201,   202,   203,   204,   205,   206,   207,   208,   209,
     210,   211,   212,   213,   214,   215,   216,   224,   226,   227,
     229,   230,   231,   232,   233,   236,   237,   238,   239,   240,
     241,   242,   243,   244,   245,   246,   247,   251,   252,   253,
     254,   257,   258,   259,   260,   261,   262,   263,   264,   265,
     266,   267,   268,   269,   270,   271,   272,   273,   274,   275,
     276,   277,   284,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,   314,   315,   316,   317,   318,   319,   320,   321,   322,
     323,   324,   325,   326,   327,   328,   329,   330,   331,   332,
     333,   334,   335,   336,   337,   338,   339,   340,   341,   342,
     343,   344,   345,   346,   347,   348,   349,   350,   351,   352,
     353,   354,   355,   356,   357,   358,   359,   360,   361,   362,
     363,   364,   365,   366,   367,   368,   369,   370,   371,   372,
     373,   374,   375,   376,   377,   378,   379,   380,   381,   382,
     383,   384,   385,   386,   387,   388,   389,   390,   391,   392,
     393,   394,   395,   396,   397,   398,   399,   400,   401,   402,
     403,   404,   405,   406,   407,   408,   409,   410,   411,   412,
     413,   414,   415,   416,   417,   418,   419,   420,   421,   422,
     423,   424,   425,   426,   427,   428,   429,   430,   431,   432,
     433,   434,   435,   436,   437,   438,   439,   440,   441,   442,
     443,   444,   445,   446,   447,   448,   449,   450,   451,   452,
     453,   454,   455,   456,   457,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,   470,   471,   472,
     473,   474,   475,   476,   477,   478,   479,   480,   481,   482,
     483,   484,   485,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,   497,   498,   499,   500,   501,   502,
     503,   504,   505,   506,   507,   508,   509,   510,   547,   548,
     549,   550,   551,   552,   553,   554,   555,   556,   557,    47,
      49,    50,   112,   140,   143,   145,   235,   288,   511,   512,
     513,   514,   515,   516,   517,   518,    47,    55,    56,   139,
     141,   144,   234,   292,   519,   520,   521,   522,   523,   524,
     525,    47,   217,   219,   220,   221,   222,   223,   228,   255,
     256,   295,   526,   527,   528,   529,   530,   531,   532,   533,
     534,   535,    99,   100,   101,   102,   103,   104,   105,   106,
     538,   539,   540,   541,   542,   543,   544,   545,   546,   249,
     250,   560,   561,   562,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10,    10,    10,    10,
      10,    10,    10,    10,    10,    10,    10
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int16 yyr1[] =
{
       0,   278,   279,   279,   280,   280,   280,   280,   280,   280,
     280,   281,   282,   283,   283,   284,   284,   284,   284,   284,
     284,   284,   284,   284,   284,   284,   284,   284,   284,   284,
     284,   284,   284,   284,   284,   284,   284,   284,   284,   284,
     284,   284,   284,   284,   284,   284,   284,   284,   284,   284,
     284,   284,   284,   284,   284,   284,   284,   284,   284,   284,
     284,   284,   284,   284,   284,   284,   284,   284,   284,   284,
     284,   284,   284,   284,   284,   284,   284,   284,   284,   284,
     284,   284,   284,   284,   284,   284,   284,   284,   284,   284,
     284,   284,   284,   284,   284,   284,   284,   284,   284,   284,
     284,   284,   284,   284,   284,   284,   284,   284,   284,   284,
     284,   284,   284,   284,   284,   284,   284,   284,   284,   284,
     284,   284,   284,   284,   284,   284,   284,   284,   284,   284,
     284,   284,   284,   284,   284,   284,   284,   284,   284,   284,
     284,   284,   284,   284,   284,   284,   284,   284,   284,   284,
     284,   284,   284,   284,   284,   284,   284,   284,   284,   284,
     284,   284,   284,   284,   284,   284,   284,   284,   284,   284,
     284,   284,   284,   284,   284,   284,   284,   284,   284,   284,
     284,   284,   284,   284,   284,   284,   284,   284,   284,   284,
     284,   284,   284,   284,   284,   284,   284,   284,   284,   284,
     284,   284,   284,   284,   284,   284,   284,   284,   284,   284,
     284,   284,   284,   284,   284,   284,   284,   284,   284,   284,
     284,   284,   284,   284,   284,   284,   284,   284,   284,   284,
     284,   284,   284,   284,   284,   284,   284,   284,   284,   284,
     284,   285,   286,   287,   287,   288,   288,   288,   288,   288,
     288,   288,   288,   289,   290,   291,   291,   292,   292,   292,
     292,   292,   292,   292,   293,   294,   294,   295,   295,   295,
     295,   295,   295,   295,   295,   295,   295,   296,   297,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,   308,
     309,   310,   311,   312,   313,   314,   315,   316,   317,   318,
     319,   320,   321,   322,   323,   324,   325,   326,   327,   328,
     329,   330,   331,   332,   333,   334,   335,   336,   337,   338,
     339,   340,   341,   342,   343,   344,   345,   346,   347,   348,
     349,   350,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,   361,   362,   363,   364,   365,   366,   367,   368,
     369,   370,   371,   372,   373,   374,   375,   376,   377,   378,
     379,   380,   381,   382,   383,   384,   385,   386,   387,   388,
     389,   390,   391,   392,   393,   394,   395,   396,   397,   398,
     399,   400,   401,   402,   403,   404,   405,   406,   407,   408,
     409,   410,   411,   412,   413,   414,   415,   416,   417,   418,
     419,   420,   421,   422,   423,   424,   425,   426,   427,   428,
     429,   430,   431,   432,   433,   434,   435,   436,   437,   438,
     439,   440,   441,   442,   443,   444,   445,   446,   447,   448,
     449,   450,   451,   452,   453,   454,   455,   456,   457,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,   470,   471,   472,   473,   474,   475,   476,   477,   478,
     479,   480,   481,   482,   483,   484,   485,   486,   487,   488,
     489,   490,   491,   492,   493,   494,   495,   496,   497,   498,
     499,   500,   501,   502,   503,   504,   505,   506,   507,   508,
     509,   510,   511,   512,   513,   514,   515,   516,   517,   518,
     519,   520,   521,   522,   523,   524,   525,   526,   527,   528,
     529,   530,   531,   532,   533,   534,   535,   536,   537,   537,
     538,   538,   538,   538,   538,   538,   538,   538,   539,   540,
     541,   542,   543,   544,   545,   546,   547,   548,   549,   550,
     551,   552,   553,   554,   555,   556,   557,   558,   559,   559,
     560,   560,   561,   562
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     0,     2,     2,     1,     1,     2,     2,     2,
       1,     1,     1,     2,     0,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     2,     1,     2,     0,     1,     1,     1,     1,     1,
       1,     1,     1,     2,     1,     2,     0,     1,     1,     1,
       1,     1,     1,     1,     1,     2,     0,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       3,     3,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     3,     2,     2,     2,
       2,     2,     2,     2,     2,     3,     3,     2,     2,     2,
       3,     3,     4,     4,     4,     3,     4,     4,     2,     2,
       2,     2,     2,     2,     2,     3,     3,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     3,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     1,     2,     0,
       1,     1,     1,     1,     1,     1,     1,     1,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     3,     2,
       2,     2,     2,     2,     2,     2,     2,     1,     2,     0,
       1,     1,     2,     2
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)]);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep)
{
  YY_USE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 11: /* force_toplevel: VAR_FORCE_TOPLEVEL  */
#line 186 "util/configparser.y"
        {
		OUTYY(("\nP(force-toplevel)\n"));
		cfg_parser->started_toplevel = 0;
	}
#line 2463 "util/configparser.c"
    break;

  case 12: /* serverstart: VAR_SERVER  */
#line 193 "util/configparser.y"
        {
		OUTYY(("\nP(server:)\n"));
		cfg_parser->started_toplevel = 1;
	}
#line 2472 "util/configparser.c"
    break;

  case 241: /* stub_clause: stubstart contents_stub  */
#line 303 "util/configparser.y"
        {
		/* stub end */
		if(cfg_parser->cfg->stubs &&
			!cfg_parser->cfg->stubs->name)
			yyerror("stub-zone without name");
	}
#line 2483 "util/configparser.c"
    break;

  case 242: /* stubstart: VAR_STUB_ZONE  */
#line 311 "util/configparser.y"
        {
		struct config_stub* s;
		OUTYY(("\nP(stub_zone:)\n"));
		cfg_parser->started_toplevel = 1;
		s = (struct config_stub*)calloc(1, sizeof(struct config_stub));
		if(s) {
			s->next = cfg_parser->cfg->stubs;
			cfg_parser->cfg->stubs = s;
		} else {
			yyerror("out of memory");
		}
	}
#line 2500 "util/configparser.c"
    break;

  case 253: /* forward_clause: forwardstart contents_forward  */
#line 330 "util/configparser.y"
        {
		/* forward end */
		if(cfg_parser->cfg->forwards &&
			!cfg_parser->cfg->forwards->name)
			yyerror("forward-zone without name");
	}
#line 2511 "util/configparser.c"
    break;

  case 254: /* forwardstart: VAR_FORWARD_ZONE  */
#line 338 "util/configparser.y"
        {
		struct config_stub* s;
		OUTYY(("\nP(forward_zone:)\n"));
		cfg_parser->started_toplevel = 1;
		s = (struct config_stub*)calloc(1, sizeof(struct config_stub));
		if(s) {
			s->next = cfg_parser->cfg->forwards;
			cfg_parser->cfg->forwards = s;
		} else {
			yyerror("out of memory");
		}
	}
#line 2528 "util/configparser.c"
    break;

  case 264: /* authstart: VAR_AUTH_ZONE  */
#line 357 "util/configparser.y"
        {
		struct config_auth* s;
		OUTYY(("\nP(auth_zone:)\n"));
		cfg_parser->started_toplevel = 1;
		s = (struct config_auth*)calloc(1, sizeof(struct config_auth));
		if(s) {
			s->next = cfg_parser->cfg->auths;
			cfg_parser->cfg->auths = s;
			/* defaults for auth zone */
			s->for_downstream = 1;
			s->for_upstream = 1;
			s->fallback_enabled = 0;
			s->zonemd_check = 0;
			s->zonemd_reject_absence = 0;
		} else {
			yyerror("out of memory");
		}
	}
#line 2551 "util/configparser.c"
    break;

  case 277: /* server_num_threads: VAR_NUM_THREADS STRING_ARG  */
#line 384 "util/configparser.y"
        {
		OUTYY(("P(server_num_threads:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->num_threads = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 2563 "util/configparser.c"
    break;

  case 278: /* server_verbosity: VAR_VERBOSITY STRING_ARG  */
#line 393 "util/configparser.y"
        {
		OUTYY(("P(server_verbosity:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->verbosity = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 2575 "util/configparser.c"
    break;

  case 279: /* server_statistics_interval: VAR_STATISTICS_INTERVAL STRING_ARG  */
#line 402 "util/configparser.y"
        {
		OUTYY(("P(server_statistics_interval:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "") == 0 || strcmp((yyvsp[0].str), "0") == 0)
			cfg_parser->cfg->stat_interval = 0;
		else if(atoi((yyvsp[0].str)) == 0)
			yyerror("number expected");
		else cfg_parser->cfg->stat_interval = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 2589 "util/configparser.c"
    break;

  case 280: /* server_statistics_cumulative: VAR_STATISTICS_CUMULATIVE STRING_ARG  */
#line 413 "util/configparser.y"
        {
		OUTYY(("P(server_statistics_cumulative:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->stat_cumulative = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 2601 "util/configparser.c"
    break;

  case 281: /* server_extended_statistics: VAR_EXTENDED_STATISTICS STRING_ARG  */
#line 422 "util/configparser.y"
        {
		OUTYY(("P(server_extended_statistics:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->stat_extended = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 2613 "util/configparser.c"
    break;

  case 282: /* server_statistics_inhibit_zero: VAR_STATISTICS_INHIBIT_ZERO STRING_ARG  */
#line 431 "util/configparser.y"
        {
		OUTYY(("P(server_statistics_inhibit_zero:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->stat_inhibit_zero = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 2625 "util/configparser.c"
    break;

  case 283: /* server_shm_enable: VAR_SHM_ENABLE STRING_ARG  */
#line 440 "util/configparser.y"
        {
		OUTYY(("P(server_shm_enable:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->shm_enable = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 2637 "util/configparser.c"
    break;

  case 284: /* server_shm_key: VAR_SHM_KEY STRING_ARG  */
#line 449 "util/configparser.y"
        {
		OUTYY(("P(server_shm_key:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "") == 0 || strcmp((yyvsp[0].str), "0") == 0)
			cfg_parser->cfg->shm_key = 0;
		else if(atoi((yyvsp[0].str)) == 0)
			yyerror("number expected");
		else cfg_parser->cfg->shm_key = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 2651 "util/configparser.c"
    break;

  case 285: /* server_port: VAR_PORT STRING_ARG  */
#line 460 "util/configparser.y"
        {
		OUTYY(("P(server_port:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0)
			yyerror("port number expected");
		else cfg_parser->cfg->port = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 2663 "util/configparser.c"
    break;

  case 286: /* server_interface: VAR_INTERFACE STRING_ARG  */
#line 469 "util/configparser.y"
        {
		OUTYY(("P(server_interface:%s)\n", (yyvsp[0].str)));
		if(cfg_parser->cfg->num_ifs == 0)
			cfg_parser->cfg->ifs = calloc(1, sizeof(char*));
		else cfg_parser->cfg->ifs = realloc(cfg_parser->cfg->ifs,
				(cfg_parser->cfg->num_ifs+1)*sizeof(char*));
		if(!cfg_parser->cfg->ifs)
			yyerror("out of memory");
		else
			cfg_parser->cfg->ifs[cfg_parser->cfg->num_ifs++] = (yyvsp[0].str);
	}
#line 2679 "util/configparser.c"
    break;

  case 287: /* server_outgoing_interface: VAR_OUTGOING_INTERFACE STRING_ARG  */
#line 482 "util/configparser.y"
        {
		OUTYY(("P(server_outgoing_interface:%s)\n", (yyvsp[0].str)));
		if(cfg_parser->cfg->num_out_ifs == 0)
			cfg_parser->cfg->out_ifs = calloc(1, sizeof(char*));
		else cfg_parser->cfg->out_ifs = realloc(
			cfg_parser->cfg->out_ifs,
			(cfg_parser->cfg->num_out_ifs+1)*sizeof(char*));
		if(!cfg_parser->cfg->out_ifs)
			yyerror("out of memory");
		else
			cfg_parser->cfg->out_ifs[
				cfg_parser->cfg->num_out_ifs++] = (yyvsp[0].str);
	}
#line 2697 "util/configparser.c"
    break;

  case 288: /* server_outgoing_range: VAR_OUTGOING_RANGE STRING_ARG  */
#line 497 "util/configparser.y"
        {
		OUTYY(("P(server_outgoing_range:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0)
			yyerror("number expected");
		else cfg_parser->cfg->outgoing_num_ports = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 2709 "util/configparser.c"
    break;

  case 289: /* server_outgoing_port_permit: VAR_OUTGOING_PORT_PERMIT STRING_ARG  */
#line 506 "util/configparser.y"
        {
		OUTYY(("P(server_outgoing_port_permit:%s)\n", (yyvsp[0].str)));
		if(!cfg_mark_ports((yyvsp[0].str), 1,
			cfg_parser->cfg->outgoing_avail_ports, 65536))
			yyerror("port number or range (\"low-high\") expected");
		free((yyvsp[0].str));
	}
#line 2721 "util/configparser.c"
    break;

  case 290: /* server_outgoing_port_avoid: VAR_OUTGOING_PORT_AVOID STRING_ARG  */
#line 515 "util/configparser.y"
        {
		OUTYY(("P(server_outgoing_port_avoid:%s)\n", (yyvsp[0].str)));
		if(!cfg_mark_ports((yyvsp[0].str), 0,
			cfg_parser->cfg->outgoing_avail_ports, 65536))
			yyerror("port number or range (\"low-high\") expected");
		free((yyvsp[0].str));
	}
#line 2733 "util/configparser.c"
    break;

  case 291: /* server_outgoing_num_tcp: VAR_OUTGOING_NUM_TCP STRING_ARG  */
#line 524 "util/configparser.y"
        {
		OUTYY(("P(server_outgoing_num_tcp:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->outgoing_num_tcp = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 2745 "util/configparser.c"
    break;

  case 292: /* server_incoming_num_tcp: VAR_INCOMING_NUM_TCP STRING_ARG  */
#line 533 "util/configparser.y"
        {
		OUTYY(("P(server_incoming_num_tcp:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->incoming_num_tcp = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 2757 "util/configparser.c"
    break;

  case 293: /* server_interface_automatic: VAR_INTERFACE_AUTOMATIC STRING_ARG  */
#line 542 "util/configparser.y"
        {
		OUTYY(("P(server_interface_automatic:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->if_automatic = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 2769 "util/configparser.c"
    break;

  case 294: /* server_interface_automatic_ports: VAR_INTERFACE_AUTOMATIC_PORTS STRING_ARG  */
#line 551 "util/configparser.y"
        {
		OUTYY(("P(server_interface_automatic_ports:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->if_automatic_ports);
		cfg_parser->cfg->if_automatic_ports = (yyvsp[0].str);
	}
#line 2779 "util/configparser.c"
    break;

  case 295: /* server_do_ip4: VAR_DO_IP4 STRING_ARG  */
#line 558 "util/configparser.y"
        {
		OUTYY(("P(server_do_ip4:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->do_ip4 = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 2791 "util/configparser.c"
    break;

  case 296: /* server_do_ip6: VAR_DO_IP6 STRING_ARG  */
#line 567 "util/configparser.y"
        {
		OUTYY(("P(server_do_ip6:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->do_ip6 = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 2803 "util/configparser.c"
    break;

  case 297: /* server_do_nat64: VAR_DO_NAT64 STRING_ARG  */
#line 576 "util/configparser.y"
        {
		OUTYY(("P(server_do_nat64:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->do_nat64 = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 2815 "util/configparser.c"
    break;

  case 298: /* server_do_udp: VAR_DO_UDP STRING_ARG  */
#line 585 "util/configparser.y"
        {
		OUTYY(("P(server_do_udp:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->do_udp = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 2827 "util/configparser.c"
    break;

  case 299: /* server_do_tcp: VAR_DO_TCP STRING_ARG  */
#line 594 "util/configparser.y"
        {
		OUTYY(("P(server_do_tcp:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->do_tcp = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 2839 "util/configparser.c"
    break;

  case 300: /* server_prefer_ip4: VAR_PREFER_IP4 STRING_ARG  */
#line 603 "util/configparser.y"
        {
		OUTYY(("P(server_prefer_ip4:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->prefer_ip4 = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 2851 "util/configparser.c"
    break;

  case 301: /* server_prefer_ip6: VAR_PREFER_IP6 STRING_ARG  */
#line 612 "util/configparser.y"
        {
		OUTYY(("P(server_prefer_ip6:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->prefer_ip6 = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 2863 "util/configparser.c"
    break;

  case 302: /* server_tcp_mss: VAR_TCP_MSS STRING_ARG  */
#line 621 "util/configparser.y"
        {
		OUTYY(("P(server_tcp_mss:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->tcp_mss = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 2875 "util/configparser.c"
    break;

  case 303: /* server_outgoing_tcp_mss: VAR_OUTGOING_TCP_MSS STRING_ARG  */
#line 630 "util/configparser.y"
        {
		OUTYY(("P(server_outgoing_tcp_mss:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->outgoing_tcp_mss = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 2887 "util/configparser.c"
    break;

  case 304: /* server_tcp_idle_timeout: VAR_TCP_IDLE_TIMEOUT STRING_ARG  */
#line 639 "util/configparser.y"
        {
		OUTYY(("P(server_tcp_idle_timeout:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("number expected");
		else if (atoi((yyvsp[0].str)) > 120000)
			cfg_parser->cfg->tcp_idle_timeout = 120000;
		else if (atoi((yyvsp[0].str)) < 1)
			cfg_parser->cfg->tcp_idle_timeout = 1;
		else cfg_parser->cfg->tcp_idle_timeout = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 2903 "util/configparser.c"
    break;

  case 305: /* server_max_reuse_tcp_queries: VAR_MAX_REUSE_TCP_QUERIES STRING_ARG  */
#line 652 "util/configparser.y"
        {
		OUTYY(("P(server_max_reuse_tcp_queries:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("number expected");
		else if (atoi((yyvsp[0].str)) < 1)
			cfg_parser->cfg->max_reuse_tcp_queries = 0;
		else cfg_parser->cfg->max_reuse_tcp_queries = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 2917 "util/configparser.c"
    break;

  case 306: /* server_tcp_reuse_timeout: VAR_TCP_REUSE_TIMEOUT STRING_ARG  */
#line 663 "util/configparser.y"
        {
		OUTYY(("P(server_tcp_reuse_timeout:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("number expected");
		else if (atoi((yyvsp[0].str)) < 1)
			cfg_parser->cfg->tcp_reuse_timeout = 0;
		else cfg_parser->cfg->tcp_reuse_timeout = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 2931 "util/configparser.c"
    break;

  case 307: /* server_tcp_auth_query_timeout: VAR_TCP_AUTH_QUERY_TIMEOUT STRING_ARG  */
#line 674 "util/configparser.y"
        {
		OUTYY(("P(server_tcp_auth_query_timeout:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("number expected");
		else if (atoi((yyvsp[0].str)) < 1)
			cfg_parser->cfg->tcp_auth_query_timeout = 0;
		else cfg_parser->cfg->tcp_auth_query_timeout = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 2945 "util/configparser.c"
    break;

  case 308: /* server_tcp_keepalive: VAR_EDNS_TCP_KEEPALIVE STRING_ARG  */
#line 685 "util/configparser.y"
        {
		OUTYY(("P(server_tcp_keepalive:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->do_tcp_keepalive = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 2957 "util/configparser.c"
    break;

  case 309: /* server_tcp_keepalive_timeout: VAR_EDNS_TCP_KEEPALIVE_TIMEOUT STRING_ARG  */
#line 694 "util/configparser.y"
        {
		OUTYY(("P(server_tcp_keepalive_timeout:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("number expected");
		else if (atoi((yyvsp[0].str)) > 6553500)
			cfg_parser->cfg->tcp_keepalive_timeout = 6553500;
		else if (atoi((yyvsp[0].str)) < 1)
			cfg_parser->cfg->tcp_keepalive_timeout = 0;
		else cfg_parser->cfg->tcp_keepalive_timeout = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 2973 "util/configparser.c"
    break;

  case 310: /* server_sock_queue_timeout: VAR_SOCK_QUEUE_TIMEOUT STRING_ARG  */
#line 707 "util/configparser.y"
        {
		OUTYY(("P(server_sock_queue_timeout:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("number expected");
		else if (atoi((yyvsp[0].str)) > 6553500)
			cfg_parser->cfg->sock_queue_timeout = 6553500;
		else if (atoi((yyvsp[0].str)) < 1)
			cfg_parser->cfg->sock_queue_timeout = 0;
		else cfg_parser->cfg->sock_queue_timeout = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 2989 "util/configparser.c"
    break;

  case 311: /* server_tcp_upstream: VAR_TCP_UPSTREAM STRING_ARG  */
#line 720 "util/configparser.y"
        {
		OUTYY(("P(server_tcp_upstream:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->tcp_upstream = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 3001 "util/configparser.c"
    break;

  case 312: /* server_udp_upstream_without_downstream: VAR_UDP_UPSTREAM_WITHOUT_DOWNSTREAM STRING_ARG  */
#line 729 "util/configparser.y"
        {
		OUTYY(("P(server_udp_upstream_without_downstream:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->udp_upstream_without_downstream = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 3013 "util/configparser.c"
    break;

  case 313: /* server_ssl_upstream: VAR_SSL_UPSTREAM STRING_ARG  */
#line 738 "util/configparser.y"
        {
		OUTYY(("P(server_ssl_upstream:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->ssl_upstream = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 3025 "util/configparser.c"
    break;

  case 314: /* server_tls_cert_bundle: VAR_TLS_CERT_BUNDLE STRING_ARG  */
#line 747 "util/configparser.y"
        {
		OUTYY(("P(server_tls_cert_bundle:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->tls_cert_bundle);
		cfg_parser->cfg->tls_cert_bundle = (yyvsp[0].str);
	}
#line 3035 "util/configparser.c"
    break;

  case 315: /* server_tls_win_cert: VAR_TLS_WIN_CERT STRING_ARG  */
#line 754 "util/configparser.y"
        {
		OUTYY(("P(server_tls_win_cert:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->tls_win_cert = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 3047 "util/configparser.c"
    break;

  case 316: /* server_tls_use_sni: VAR_TLS_USE_SNI STRING_ARG  */
#line 763 "util/configparser.y"
        {
		OUTYY(("P(server_tls_use_sni:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->tls_use_sni = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 3059 "util/configparser.c"
    break;

  case 317: /* server_tls_protocols: VAR_TLS_PROTOCOLS STRING_ARG  */
#line 772 "util/configparser.y"
        {
		OUTYY(("P(server_tls_protocols:%s)\n", (yyvsp[0].str)));
		if(!cfg_tls_protocols_is_valid((yyvsp[0].str)))
			yyerror("tls-protocols: valid values are 'TLSv1.2' and 'TLSv1.3'.");
		free(cfg_parser->cfg->tls_protocols);
		cfg_parser->cfg->tls_protocols = (yyvsp[0].str);
	}
#line 3071 "util/configparser.c"
    break;

  case 318: /* server_use_systemd: VAR_USE_SYSTEMD STRING_ARG  */
#line 781 "util/configparser.y"
        {
		OUTYY(("P(server_use_systemd:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->use_systemd = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 3083 "util/configparser.c"
    break;

  case 319: /* server_do_daemonize: VAR_DO_DAEMONIZE STRING_ARG  */
#line 790 "util/configparser.y"
        {
		OUTYY(("P(server_do_daemonize:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->do_daemonize = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 3095 "util/configparser.c"
    break;

  case 320: /* server_use_syslog: VAR_USE_SYSLOG STRING_ARG  */
#line 799 "util/configparser.y"
        {
		OUTYY(("P(server_use_syslog:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->use_syslog = (strcmp((yyvsp[0].str), "yes")==0);
#if !defined(HAVE_SYSLOG_H) && !defined(UB_ON_WINDOWS)
		if(strcmp((yyvsp[0].str), "yes") == 0)
			yyerror("no syslog services are available. "
				"(reconfigure and compile to add)");
#endif
		free((yyvsp[0].str));
	}
#line 3112 "util/configparser.c"
    break;

  case 321: /* server_log_time_ascii: VAR_LOG_TIME_ASCII STRING_ARG  */
#line 813 "util/configparser.y"
        {
		OUTYY(("P(server_log_time_ascii:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->log_time_ascii = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 3124 "util/configparser.c"
    break;

  case 322: /* server_log_time_iso: VAR_LOG_TIME_ISO STRING_ARG  */
#line 822 "util/configparser.y"
        {
		OUTYY(("P(server_log_time_iso:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->log_time_iso = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 3136 "util/configparser.c"
    break;

  case 323: /* server_log_queries: VAR_LOG_QUERIES STRING_ARG  */
#line 831 "util/configparser.y"
        {
		OUTYY(("P(server_log_queries:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->log_queries = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 3148 "util/configparser.c"
    break;

  case 324: /* server_log_replies: VAR_LOG_REPLIES STRING_ARG  */
#line 840 "util/configparser.y"
        {
		OUTYY(("P(server_log_replies:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->log_replies = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 3160 "util/configparser.c"
    break;

  case 325: /* server_log_tag_queryreply: VAR_LOG_TAG_QUERYREPLY STRING_ARG  */
#line 849 "util/configparser.y"
        {
		OUTYY(("P(server_log_tag_queryreply:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->log_tag_queryreply = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 3172 "util/configparser.c"
    break;

  case 326: /* server_log_servfail: VAR_LOG_SERVFAIL STRING_ARG  */
#line 858 "util/configparser.y"
        {
		OUTYY(("P(server_log_servfail:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->log_servfail = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 3184 "util/configparser.c"
    break;

  case 327: /* server_log_destaddr: VAR_LOG_DESTADDR STRING_ARG  */
#line 867 "util/configparser.y"
        {
		OUTYY(("P(server_log_destaddr:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->log_destaddr = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 3196 "util/configparser.c"
    break;

  case 328: /* server_log_thread_id: VAR_LOG_THREAD_ID STRING_ARG  */
#line 876 "util/configparser.y"
        {
		OUTYY(("P(server_log_thread_id:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->log_thread_id = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 3208 "util/configparser.c"
    break;

  case 329: /* server_log_local_actions: VAR_LOG_LOCAL_ACTIONS STRING_ARG  */
#line 885 "util/configparser.y"
        {
		OUTYY(("P(server_log_local_actions:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->log_local_actions = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 3220 "util/configparser.c"
    break;

  case 330: /* server_chroot: VAR_CHROOT STRING_ARG  */
#line 894 "util/configparser.y"
        {
		OUTYY(("P(server_chroot:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->chrootdir);
		cfg_parser->cfg->chrootdir = (yyvsp[0].str);
	}
#line 3230 "util/configparser.c"
    break;

  case 331: /* server_username: VAR_USERNAME STRING_ARG  */
#line 901 "util/configparser.y"
        {
		OUTYY(("P(server_username:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->username);
		cfg_parser->cfg->username = (yyvsp[0].str);
	}
#line 3240 "util/configparser.c"
    break;

  case 332: /* server_directory: VAR_DIRECTORY STRING_ARG  */
#line 908 "util/configparser.y"
        {
		OUTYY(("P(server_directory:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->directory);
		cfg_parser->cfg->directory = (yyvsp[0].str);
		/* change there right away for includes relative to this */
		if((yyvsp[0].str)[0]) {
			char* d;
#ifdef UB_ON_WINDOWS
			w_config_adjust_directory(cfg_parser->cfg);
#endif
			d = cfg_parser->cfg->directory;
			/* adjust directory if we have already chroot,
			 * like, we reread after sighup */
			if(cfg_parser->chroot && cfg_parser->chroot[0] &&
				strncmp(d, cfg_parser->chroot, strlen(
				cfg_parser->chroot)) == 0)
				d += strlen(cfg_parser->chroot);
			if(d[0]) {
				if(chdir(d))
				log_err("cannot chdir to directory: %s (%s)",
					d, strerror(errno));
			}
		}
	}
#line 3269 "util/configparser.c"
    break;

  case 333: /* server_logfile: VAR_LOGFILE STRING_ARG  */
#line 934 "util/configparser.y"
        {
		OUTYY(("P(server_logfile:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->logfile);
		cfg_parser->cfg->logfile = (yyvsp[0].str);
		cfg_parser->cfg->use_syslog = 0;
	}
#line 3280 "util/configparser.c"
    break;

  case 334: /* server_pidfile: VAR_PIDFILE STRING_ARG  */
#line 942 "util/configparser.y"
        {
		OUTYY(("P(server_pidfile:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->pidfile);
		cfg_parser->cfg->pidfile = (yyvsp[0].str);
	}
#line 3290 "util/configparser.c"
    break;

  case 335: /* server_root_hints: VAR_ROOT_HINTS STRING_ARG  */
#line 949 "util/configparser.y"
        {
		OUTYY(("P(server_root_hints:%s)\n", (yyvsp[0].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->root_hints, (yyvsp[0].str)))
			yyerror("out of memory");
	}
#line 3300 "util/configparser.c"
    break;

  case 336: /* server_dlv_anchor_file: VAR_DLV_ANCHOR_FILE STRING_ARG  */
#line 956 "util/configparser.y"
        {
		OUTYY(("P(server_dlv_anchor_file:%s)\n", (yyvsp[0].str)));
		log_warn("option dlv-anchor-file ignored: DLV is decommissioned");
		free((yyvsp[0].str));
	}
#line 3310 "util/configparser.c"
    break;

  case 337: /* server_dlv_anchor: VAR_DLV_ANCHOR STRING_ARG  */
#line 963 "util/configparser.y"
        {
		OUTYY(("P(server_dlv_anchor:%s)\n", (yyvsp[0].str)));
		log_warn("option dlv-anchor ignored: DLV is decommissioned");
		free((yyvsp[0].str));
	}
#line 3320 "util/configparser.c"
    break;

  case 338: /* server_auto_trust_anchor_file: VAR_AUTO_TRUST_ANCHOR_FILE STRING_ARG  */
#line 970 "util/configparser.y"
        {
		OUTYY(("P(server_auto_trust_anchor_file:%s)\n", (yyvsp[0].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->
			auto_trust_anchor_file_list, (yyvsp[0].str)))
			yyerror("out of memory");
	}
#line 3331 "util/configparser.c"
    break;

  case 339: /* server_trust_anchor_file: VAR_TRUST_ANCHOR_FILE STRING_ARG  */
#line 978 "util/configparser.y"
        {
		OUTYY(("P(server_trust_anchor_file:%s)\n", (yyvsp[0].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->
			trust_anchor_file_list, (yyvsp[0].str)))
			yyerror("out of memory");
	}
#line 3342 "util/configparser.c"
    break;

  case 340: /* server_trusted_keys_file: VAR_TRUSTED_KEYS_FILE STRING_ARG  */
#line 986 "util/configparser.y"
        {
		OUTYY(("P(server_trusted_keys_file:%s)\n", (yyvsp[0].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->
			trusted_keys_file_list, (yyvsp[0].str)))
			yyerror("out of memory");
	}
#line 3353 "util/configparser.c"
    break;

  case 341: /* server_trust_anchor: VAR_TRUST_ANCHOR STRING_ARG  */
#line 994 "util/configparser.y"
        {
		OUTYY(("P(server_trust_anchor:%s)\n", (yyvsp[0].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->trust_anchor_list, (yyvsp[0].str)))
			yyerror("out of memory");
	}
#line 3363 "util/configparser.c"
    break;

  case 342: /* server_trust_anchor_signaling: VAR_TRUST_ANCHOR_SIGNALING STRING_ARG  */
#line 1001 "util/configparser.y"
        {
		OUTYY(("P(server_trust_anchor_signaling:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else
			cfg_parser->cfg->trust_anchor_signaling =
				(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 3377 "util/configparser.c"
    break;

  case 343: /* server_root_key_sentinel: VAR_ROOT_KEY_SENTINEL STRING_ARG  */
#line 1012 "util/configparser.y"
        {
		OUTYY(("P(server_root_key_sentinel:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else
			cfg_parser->cfg->root_key_sentinel =
				(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 3391 "util/configparser.c"
    break;

  case 344: /* server_domain_insecure: VAR_DOMAIN_INSECURE STRING_ARG  */
#line 1023 "util/configparser.y"
        {
		OUTYY(("P(server_domain_insecure:%s)\n", (yyvsp[0].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->domain_insecure, (yyvsp[0].str)))
			yyerror("out of memory");
	}
#line 3401 "util/configparser.c"
    break;

  case 345: /* server_hide_identity: VAR_HIDE_IDENTITY STRING_ARG  */
#line 1030 "util/configparser.y"
        {
		OUTYY(("P(server_hide_identity:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->hide_identity = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 3413 "util/configparser.c"
    break;

  case 346: /* server_hide_version: VAR_HIDE_VERSION STRING_ARG  */
#line 1039 "util/configparser.y"
        {
		OUTYY(("P(server_hide_version:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->hide_version = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 3425 "util/configparser.c"
    break;

  case 347: /* server_hide_trustanchor: VAR_HIDE_TRUSTANCHOR STRING_ARG  */
#line 1048 "util/configparser.y"
        {
		OUTYY(("P(server_hide_trustanchor:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->hide_trustanchor = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 3437 "util/configparser.c"
    break;

  case 348: /* server_hide_http_user_agent: VAR_HIDE_HTTP_USER_AGENT STRING_ARG  */
#line 1057 "util/configparser.y"
        {
		OUTYY(("P(server_hide_user_agent:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->hide_http_user_agent = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 3449 "util/configparser.c"
    break;

  case 349: /* server_identity: VAR_IDENTITY STRING_ARG  */
#line 1066 "util/configparser.y"
        {
		OUTYY(("P(server_identity:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->identity);
		cfg_parser->cfg->identity = (yyvsp[0].str);
	}
#line 3459 "util/configparser.c"
    break;

  case 350: /* server_version: VAR_VERSION STRING_ARG  */
#line 1073 "util/configparser.y"
        {
		OUTYY(("P(server_version:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->version);
		cfg_parser->cfg->version = (yyvsp[0].str);
	}
#line 3469 "util/configparser.c"
    break;

  case 351: /* server_http_user_agent: VAR_HTTP_USER_AGENT STRING_ARG  */
#line 1080 "util/configparser.y"
        {
		OUTYY(("P(server_http_user_agent:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->http_user_agent);
		cfg_parser->cfg->http_user_agent = (yyvsp[0].str);
	}
#line 3479 "util/configparser.c"
    break;

  case 352: /* server_nsid: VAR_NSID STRING_ARG  */
#line 1087 "util/configparser.y"
        {
		OUTYY(("P(server_nsid:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->nsid_cfg_str);
		cfg_parser->cfg->nsid_cfg_str = (yyvsp[0].str);
		free(cfg_parser->cfg->nsid);
		cfg_parser->cfg->nsid = NULL;
		cfg_parser->cfg->nsid_len = 0;
		if (*(yyvsp[0].str) == 0)
			; /* pass; empty string is not setting nsid */
		else if (!(cfg_parser->cfg->nsid = cfg_parse_nsid(
					(yyvsp[0].str), &cfg_parser->cfg->nsid_len)))
			yyerror("the NSID must be either a hex string or an "
			    "ascii character string prepended with ascii_.");
	}
#line 3498 "util/configparser.c"
    break;

  case 353: /* server_so_rcvbuf: VAR_SO_RCVBUF STRING_ARG  */
#line 1103 "util/configparser.y"
        {
		OUTYY(("P(server_so_rcvbuf:%s)\n", (yyvsp[0].str)));
		if(!cfg_parse_memsize((yyvsp[0].str), &cfg_parser->cfg->so_rcvbuf))
			yyerror("buffer size expected");
		free((yyvsp[0].str));
	}
#line 3509 "util/configparser.c"
    break;

  case 354: /* server_so_sndbuf: VAR_SO_SNDBUF STRING_ARG  */
#line 1111 "util/configparser.y"
        {
		OUTYY(("P(server_so_sndbuf:%s)\n", (yyvsp[0].str)));
		if(!cfg_parse_memsize((yyvsp[0].str), &cfg_parser->cfg->so_sndbuf))
			yyerror("buffer size expected");
		free((yyvsp[0].str));
	}
#line 3520 "util/configparser.c"
    break;

  case 355: /* server_so_reuseport: VAR_SO_REUSEPORT STRING_ARG  */
#line 1119 "util/configparser.y"
        {
		OUTYY(("P(server_so_reuseport:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->so_reuseport =
			(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 3533 "util/configparser.c"
    break;

  case 356: /* server_ip_transparent: VAR_IP_TRANSPARENT STRING_ARG  */
#line 1129 "util/configparser.y"
        {
		OUTYY(("P(server_ip_transparent:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->ip_transparent =
			(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 3546 "util/configparser.c"
    break;

  case 357: /* server_ip_freebind: VAR_IP_FREEBIND STRING_ARG  */
#line 1139 "util/configparser.y"
        {
		OUTYY(("P(server_ip_freebind:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->ip_freebind =
			(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 3559 "util/configparser.c"
    break;

  case 358: /* server_ip_dscp: VAR_IP_DSCP STRING_ARG  */
#line 1149 "util/configparser.y"
        {
		OUTYY(("P(server_ip_dscp:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("number expected");
		else if (atoi((yyvsp[0].str)) > 63)
			yyerror("value too large (max 63)");
		else if (atoi((yyvsp[0].str)) < 0)
			yyerror("value too small (min 0)");
		else
			cfg_parser->cfg->ip_dscp = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 3576 "util/configparser.c"
    break;

  case 359: /* server_stream_wait_size: VAR_STREAM_WAIT_SIZE STRING_ARG  */
#line 1163 "util/configparser.y"
        {
		OUTYY(("P(server_stream_wait_size:%s)\n", (yyvsp[0].str)));
		if(!cfg_parse_memsize((yyvsp[0].str), &cfg_parser->cfg->stream_wait_size))
			yyerror("memory size expected");
		free((yyvsp[0].str));
	}
#line 3587 "util/configparser.c"
    break;

  case 360: /* server_edns_buffer_size: VAR_EDNS_BUFFER_SIZE STRING_ARG  */
#line 1171 "util/configparser.y"
        {
		OUTYY(("P(server_edns_buffer_size:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0)
			yyerror("number expected");
		else if (atoi((yyvsp[0].str)) < 12)
			yyerror("edns buffer size too small");
		else if (atoi((yyvsp[0].str)) > 65535)
			cfg_parser->cfg->edns_buffer_size = 65535;
		else cfg_parser->cfg->edns_buffer_size = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 3603 "util/configparser.c"
    break;

  case 361: /* server_msg_buffer_size: VAR_MSG_BUFFER_SIZE STRING_ARG  */
#line 1184 "util/configparser.y"
        {
		OUTYY(("P(server_msg_buffer_size:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0)
			yyerror("number expected");
		else if (atoi((yyvsp[0].str)) < 4096)
			yyerror("message buffer size too small (use 4096)");
		else cfg_parser->cfg->msg_buffer_size = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 3617 "util/configparser.c"
    break;

  case 362: /* server_msg_cache_size: VAR_MSG_CACHE_SIZE STRING_ARG  */
#line 1195 "util/configparser.y"
        {
		OUTYY(("P(server_msg_cache_size:%s)\n", (yyvsp[0].str)));
		if(!cfg_parse_memsize((yyvsp[0].str), &cfg_parser->cfg->msg_cache_size))
			yyerror("memory size expected");
		free((yyvsp[0].str));
	}
#line 3628 "util/configparser.c"
    break;

  case 363: /* server_msg_cache_slabs: VAR_MSG_CACHE_SLABS STRING_ARG  */
#line 1203 "util/configparser.y"
        {
		OUTYY(("P(server_msg_cache_slabs:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0) {
			yyerror("number expected");
		} else {
			cfg_parser->cfg->msg_cache_slabs = atoi((yyvsp[0].str));
			if(!is_pow2(cfg_parser->cfg->msg_cache_slabs))
				yyerror("must be a power of 2");
		}
		free((yyvsp[0].str));
	}
#line 3644 "util/configparser.c"
    break;

  case 364: /* server_num_queries_per_thread: VAR_NUM_QUERIES_PER_THREAD STRING_ARG  */
#line 1216 "util/configparser.y"
        {
		OUTYY(("P(server_num_queries_per_thread:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0)
			yyerror("number expected");
		else cfg_parser->cfg->num_queries_per_thread = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 3656 "util/configparser.c"
    break;

  case 365: /* server_jostle_timeout: VAR_JOSTLE_TIMEOUT STRING_ARG  */
#line 1225 "util/configparser.y"
        {
		OUTYY(("P(server_jostle_timeout:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->jostle_time = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 3668 "util/configparser.c"
    break;

  case 366: /* server_delay_close: VAR_DELAY_CLOSE STRING_ARG  */
#line 1234 "util/configparser.y"
        {
		OUTYY(("P(server_delay_close:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->delay_close = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 3680 "util/configparser.c"
    break;

  case 367: /* server_udp_connect: VAR_UDP_CONNECT STRING_ARG  */
#line 1243 "util/configparser.y"
        {
		OUTYY(("P(server_udp_connect:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->udp_connect = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 3692 "util/configparser.c"
    break;

  case 368: /* server_unblock_lan_zones: VAR_UNBLOCK_LAN_ZONES STRING_ARG  */
#line 1252 "util/configparser.y"
        {
		OUTYY(("P(server_unblock_lan_zones:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->unblock_lan_zones =
			(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 3705 "util/configparser.c"
    break;

  case 369: /* server_insecure_lan_zones: VAR_INSECURE_LAN_ZONES STRING_ARG  */
#line 1262 "util/configparser.y"
        {
		OUTYY(("P(server_insecure_lan_zones:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->insecure_lan_zones =
			(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 3718 "util/configparser.c"
    break;

  case 370: /* server_rrset_cache_size: VAR_RRSET_CACHE_SIZE STRING_ARG  */
#line 1272 "util/configparser.y"
        {
		OUTYY(("P(server_rrset_cache_size:%s)\n", (yyvsp[0].str)));
		if(!cfg_parse_memsize((yyvsp[0].str), &cfg_parser->cfg->rrset_cache_size))
			yyerror("memory size expected");
		free((yyvsp[0].str));
	}
#line 3729 "util/configparser.c"
    break;

  case 371: /* server_rrset_cache_slabs: VAR_RRSET_CACHE_SLABS STRING_ARG  */
#line 1280 "util/configparser.y"
        {
		OUTYY(("P(server_rrset_cache_slabs:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0) {
			yyerror("number expected");
		} else {
			cfg_parser->cfg->rrset_cache_slabs = atoi((yyvsp[0].str));
			if(!is_pow2(cfg_parser->cfg->rrset_cache_slabs))
				yyerror("must be a power of 2");
		}
		free((yyvsp[0].str));
	}
#line 3745 "util/configparser.c"
    break;

  case 372: /* server_infra_host_ttl: VAR_INFRA_HOST_TTL STRING_ARG  */
#line 1293 "util/configparser.y"
        {
		OUTYY(("P(server_infra_host_ttl:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->host_ttl = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 3757 "util/configparser.c"
    break;

  case 373: /* server_infra_lame_ttl: VAR_INFRA_LAME_TTL STRING_ARG  */
#line 1302 "util/configparser.y"
        {
		OUTYY(("P(server_infra_lame_ttl:%s)\n", (yyvsp[0].str)));
		verbose(VERB_DETAIL, "ignored infra-lame-ttl: %s (option "
			"removed, use infra-host-ttl)", (yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 3768 "util/configparser.c"
    break;

  case 374: /* server_infra_cache_numhosts: VAR_INFRA_CACHE_NUMHOSTS STRING_ARG  */
#line 1310 "util/configparser.y"
        {
		OUTYY(("P(server_infra_cache_numhosts:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0)
			yyerror("number expected");
		else cfg_parser->cfg->infra_cache_numhosts = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 3780 "util/configparser.c"
    break;

  case 375: /* server_infra_cache_lame_size: VAR_INFRA_CACHE_LAME_SIZE STRING_ARG  */
#line 1319 "util/configparser.y"
        {
		OUTYY(("P(server_infra_cache_lame_size:%s)\n", (yyvsp[0].str)));
		verbose(VERB_DETAIL, "ignored infra-cache-lame-size: %s "
			"(option removed, use infra-cache-numhosts)", (yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 3791 "util/configparser.c"
    break;

  case 376: /* server_infra_cache_slabs: VAR_INFRA_CACHE_SLABS STRING_ARG  */
#line 1327 "util/configparser.y"
        {
		OUTYY(("P(server_infra_cache_slabs:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0) {
			yyerror("number expected");
		} else {
			cfg_parser->cfg->infra_cache_slabs = atoi((yyvsp[0].str));
			if(!is_pow2(cfg_parser->cfg->infra_cache_slabs))
				yyerror("must be a power of 2");
		}
		free((yyvsp[0].str));
	}
#line 3807 "util/configparser.c"
    break;

  case 377: /* server_infra_cache_min_rtt: VAR_INFRA_CACHE_MIN_RTT STRING_ARG  */
#line 1340 "util/configparser.y"
        {
		OUTYY(("P(server_infra_cache_min_rtt:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->infra_cache_min_rtt = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 3819 "util/configparser.c"
    break;

  case 378: /* server_infra_cache_max_rtt: VAR_INFRA_CACHE_MAX_RTT STRING_ARG  */
#line 1349 "util/configparser.y"
        {
		OUTYY(("P(server_infra_cache_max_rtt:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->infra_cache_max_rtt = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 3831 "util/configparser.c"
    break;

  case 379: /* server_infra_keep_probing: VAR_INFRA_KEEP_PROBING STRING_ARG  */
#line 1358 "util/configparser.y"
        {
		OUTYY(("P(server_infra_keep_probing:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->infra_keep_probing =
			(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 3844 "util/configparser.c"
    break;

  case 380: /* server_target_fetch_policy: VAR_TARGET_FETCH_POLICY STRING_ARG  */
#line 1368 "util/configparser.y"
        {
		OUTYY(("P(server_target_fetch_policy:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->target_fetch_policy);
		cfg_parser->cfg->target_fetch_policy = (yyvsp[0].str);
	}
#line 3854 "util/configparser.c"
    break;

  case 381: /* server_harden_short_bufsize: VAR_HARDEN_SHORT_BUFSIZE STRING_ARG  */
#line 1375 "util/configparser.y"
        {
		OUTYY(("P(server_harden_short_bufsize:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->harden_short_bufsize =
			(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 3867 "util/configparser.c"
    break;

  case 382: /* server_harden_large_queries: VAR_HARDEN_LARGE_QUERIES STRING_ARG  */
#line 1385 "util/configparser.y"
        {
		OUTYY(("P(server_harden_large_queries:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->harden_large_queries =
			(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 3880 "util/configparser.c"
    break;

  case 383: /* server_harden_glue: VAR_HARDEN_GLUE STRING_ARG  */
#line 1395 "util/configparser.y"
        {
		OUTYY(("P(server_harden_glue:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->harden_glue =
			(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 3893 "util/configparser.c"
    break;

  case 384: /* server_harden_unverified_glue: VAR_HARDEN_UNVERIFIED_GLUE STRING_ARG  */
#line 1405 "util/configparser.y"
       {
               OUTYY(("P(server_harden_unverified_glue:%s)\n", (yyvsp[0].str)));
               if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
                       yyerror("expected yes or no.");
               else cfg_parser->cfg->harden_unverified_glue =
                       (strcmp((yyvsp[0].str), "yes")==0);
               free((yyvsp[0].str));
       }
#line 3906 "util/configparser.c"
    break;

  case 385: /* server_harden_dnssec_stripped: VAR_HARDEN_DNSSEC_STRIPPED STRING_ARG  */
#line 1415 "util/configparser.y"
        {
		OUTYY(("P(server_harden_dnssec_stripped:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->harden_dnssec_stripped =
			(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 3919 "util/configparser.c"
    break;

  case 386: /* server_harden_below_nxdomain: VAR_HARDEN_BELOW_NXDOMAIN STRING_ARG  */
#line 1425 "util/configparser.y"
        {
		OUTYY(("P(server_harden_below_nxdomain:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->harden_below_nxdomain =
			(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 3932 "util/configparser.c"
    break;

  case 387: /* server_harden_referral_path: VAR_HARDEN_REFERRAL_PATH STRING_ARG  */
#line 1435 "util/configparser.y"
        {
		OUTYY(("P(server_harden_referral_path:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->harden_referral_path =
			(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 3945 "util/configparser.c"
    break;

  case 388: /* server_harden_algo_downgrade: VAR_HARDEN_ALGO_DOWNGRADE STRING_ARG  */
#line 1445 "util/configparser.y"
        {
		OUTYY(("P(server_harden_algo_downgrade:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->harden_algo_downgrade =
			(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 3958 "util/configparser.c"
    break;

  case 389: /* server_harden_unknown_additional: VAR_HARDEN_UNKNOWN_ADDITIONAL STRING_ARG  */
#line 1455 "util/configparser.y"
        {
		OUTYY(("P(server_harden_unknown_additional:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->harden_unknown_additional =
			(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 3971 "util/configparser.c"
    break;

  case 390: /* server_use_caps_for_id: VAR_USE_CAPS_FOR_ID STRING_ARG  */
#line 1465 "util/configparser.y"
        {
		OUTYY(("P(server_use_caps_for_id:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->use_caps_bits_for_id =
			(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 3984 "util/configparser.c"
    break;

  case 391: /* server_caps_whitelist: VAR_CAPS_WHITELIST STRING_ARG  */
#line 1475 "util/configparser.y"
        {
		OUTYY(("P(server_caps_whitelist:%s)\n", (yyvsp[0].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->caps_whitelist, (yyvsp[0].str)))
			yyerror("out of memory");
	}
#line 3994 "util/configparser.c"
    break;

  case 392: /* server_private_address: VAR_PRIVATE_ADDRESS STRING_ARG  */
#line 1482 "util/configparser.y"
        {
		OUTYY(("P(server_private_address:%s)\n", (yyvsp[0].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->private_address, (yyvsp[0].str)))
			yyerror("out of memory");
	}
#line 4004 "util/configparser.c"
    break;

  case 393: /* server_private_domain: VAR_PRIVATE_DOMAIN STRING_ARG  */
#line 1489 "util/configparser.y"
        {
		OUTYY(("P(server_private_domain:%s)\n", (yyvsp[0].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->private_domain, (yyvsp[0].str)))
			yyerror("out of memory");
	}
#line 4014 "util/configparser.c"
    break;

  case 394: /* server_prefetch: VAR_PREFETCH STRING_ARG  */
#line 1496 "util/configparser.y"
        {
		OUTYY(("P(server_prefetch:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->prefetch = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 4026 "util/configparser.c"
    break;

  case 395: /* server_prefetch_key: VAR_PREFETCH_KEY STRING_ARG  */
#line 1505 "util/configparser.y"
        {
		OUTYY(("P(server_prefetch_key:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->prefetch_key = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 4038 "util/configparser.c"
    break;

  case 396: /* server_deny_any: VAR_DENY_ANY STRING_ARG  */
#line 1514 "util/configparser.y"
        {
		OUTYY(("P(server_deny_any:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->deny_any = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 4050 "util/configparser.c"
    break;

  case 397: /* server_unwanted_reply_threshold: VAR_UNWANTED_REPLY_THRESHOLD STRING_ARG  */
#line 1523 "util/configparser.y"
        {
		OUTYY(("P(server_unwanted_reply_threshold:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->unwanted_threshold = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 4062 "util/configparser.c"
    break;

  case 398: /* server_do_not_query_address: VAR_DO_NOT_QUERY_ADDRESS STRING_ARG  */
#line 1532 "util/configparser.y"
        {
		OUTYY(("P(server_do_not_query_address:%s)\n", (yyvsp[0].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->donotqueryaddrs, (yyvsp[0].str)))
			yyerror("out of memory");
	}
#line 4072 "util/configparser.c"
    break;

  case 399: /* server_do_not_query_localhost: VAR_DO_NOT_QUERY_LOCALHOST STRING_ARG  */
#line 1539 "util/configparser.y"
        {
		OUTYY(("P(server_do_not_query_localhost:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->donotquery_localhost =
			(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 4085 "util/configparser.c"
    break;

  case 400: /* server_access_control: VAR_ACCESS_CONTROL STRING_ARG STRING_ARG  */
#line 1549 "util/configparser.y"
        {
		OUTYY(("P(server_access_control:%s %s)\n", (yyvsp[-1].str), (yyvsp[0].str)));
		validate_acl_action((yyvsp[0].str));
		if(!cfg_str2list_insert(&cfg_parser->cfg->acls, (yyvsp[-1].str), (yyvsp[0].str)))
			fatal_exit("out of memory adding acl");
	}
#line 4096 "util/configparser.c"
    break;

  case 401: /* server_interface_action: VAR_INTERFACE_ACTION STRING_ARG STRING_ARG  */
#line 1557 "util/configparser.y"
        {
		OUTYY(("P(server_interface_action:%s %s)\n", (yyvsp[-1].str), (yyvsp[0].str)));
		validate_acl_action((yyvsp[0].str));
		if(!cfg_str2list_insert(
			&cfg_parser->cfg->interface_actions, (yyvsp[-1].str), (yyvsp[0].str)))
			fatal_exit("out of memory adding acl");
	}
#line 4108 "util/configparser.c"
    break;

  case 402: /* server_module_conf: VAR_MODULE_CONF STRING_ARG  */
#line 1566 "util/configparser.y"
        {
		OUTYY(("P(server_module_conf:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->module_conf);
		cfg_parser->cfg->module_conf = (yyvsp[0].str);
	}
#line 4118 "util/configparser.c"
    break;

  case 403: /* server_val_override_date: VAR_VAL_OVERRIDE_DATE STRING_ARG  */
#line 1573 "util/configparser.y"
        {
		OUTYY(("P(server_val_override_date:%s)\n", (yyvsp[0].str)));
		if(*(yyvsp[0].str) == '\0' || strcmp((yyvsp[0].str), "0") == 0) {
			cfg_parser->cfg->val_date_override = 0;
		} else if(strlen((yyvsp[0].str)) == 14) {
			cfg_parser->cfg->val_date_override =
				cfg_convert_timeval((yyvsp[0].str));
			if(!cfg_parser->cfg->val_date_override)
				yyerror("bad date/time specification");
		} else {
			if(atoi((yyvsp[0].str)) == 0)
				yyerror("number expected");
			cfg_parser->cfg->val_date_override = atoi((yyvsp[0].str));
		}
		free((yyvsp[0].str));
	}
#line 4139 "util/configparser.c"
    break;

  case 404: /* server_val_sig_skew_min: VAR_VAL_SIG_SKEW_MIN STRING_ARG  */
#line 1591 "util/configparser.y"
        {
		OUTYY(("P(server_val_sig_skew_min:%s)\n", (yyvsp[0].str)));
		if(*(yyvsp[0].str) == '\0' || strcmp((yyvsp[0].str), "0") == 0) {
			cfg_parser->cfg->val_sig_skew_min = 0;
		} else {
			cfg_parser->cfg->val_sig_skew_min = atoi((yyvsp[0].str));
			if(!cfg_parser->cfg->val_sig_skew_min)
				yyerror("number expected");
		}
		free((yyvsp[0].str));
	}
#line 4155 "util/configparser.c"
    break;

  case 405: /* server_val_sig_skew_max: VAR_VAL_SIG_SKEW_MAX STRING_ARG  */
#line 1604 "util/configparser.y"
        {
		OUTYY(("P(server_val_sig_skew_max:%s)\n", (yyvsp[0].str)));
		if(*(yyvsp[0].str) == '\0' || strcmp((yyvsp[0].str), "0") == 0) {
			cfg_parser->cfg->val_sig_skew_max = 0;
		} else {
			cfg_parser->cfg->val_sig_skew_max = atoi((yyvsp[0].str));
			if(!cfg_parser->cfg->val_sig_skew_max)
				yyerror("number expected");
		}
		free((yyvsp[0].str));
	}
#line 4171 "util/configparser.c"
    break;

  case 406: /* server_val_max_restart: VAR_VAL_MAX_RESTART STRING_ARG  */
#line 1617 "util/configparser.y"
        {
		OUTYY(("P(server_val_max_restart:%s)\n", (yyvsp[0].str)));
		if(*(yyvsp[0].str) == '\0' || strcmp((yyvsp[0].str), "0") == 0) {
			cfg_parser->cfg->val_max_restart = 0;
		} else {
			cfg_parser->cfg->val_max_restart = atoi((yyvsp[0].str));
			if(!cfg_parser->cfg->val_max_restart)
				yyerror("number expected");
		}
		free((yyvsp[0].str));
	}
#line 4187 "util/configparser.c"
    break;

  case 407: /* server_cache_max_ttl: VAR_CACHE_MAX_TTL STRING_ARG  */
#line 1630 "util/configparser.y"
        {
		OUTYY(("P(server_cache_max_ttl:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->max_ttl = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 4199 "util/configparser.c"
    break;

  case 408: /* server_cache_max_negative_ttl: VAR_CACHE_MAX_NEGATIVE_TTL STRING_ARG  */
#line 1639 "util/configparser.y"
        {
		OUTYY(("P(server_cache_max_negative_ttl:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->max_negative_ttl = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 4211 "util/configparser.c"
    break;

  case 409: /* server_cache_min_negative_ttl: VAR_CACHE_MIN_NEGATIVE_TTL STRING_ARG  */
#line 1648 "util/configparser.y"
        {
		OUTYY(("P(server_cache_min_negative_ttl:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->min_negative_ttl = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 4223 "util/configparser.c"
    break;

  case 410: /* server_cache_min_ttl: VAR_CACHE_MIN_TTL STRING_ARG  */
#line 1657 "util/configparser.y"
        {
		OUTYY(("P(server_cache_min_ttl:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->min_ttl = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 4235 "util/configparser.c"
    break;

  case 411: /* server_bogus_ttl: VAR_BOGUS_TTL STRING_ARG  */
#line 1666 "util/configparser.y"
        {
		OUTYY(("P(server_bogus_ttl:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->bogus_ttl = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 4247 "util/configparser.c"
    break;

  case 412: /* server_val_clean_additional: VAR_VAL_CLEAN_ADDITIONAL STRING_ARG  */
#line 1675 "util/configparser.y"
        {
		OUTYY(("P(server_val_clean_additional:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->val_clean_additional =
			(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 4260 "util/configparser.c"
    break;

  case 413: /* server_val_permissive_mode: VAR_VAL_PERMISSIVE_MODE STRING_ARG  */
#line 1685 "util/configparser.y"
        {
		OUTYY(("P(server_val_permissive_mode:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->val_permissive_mode =
			(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 4273 "util/configparser.c"
    break;

  case 414: /* server_aggressive_nsec: VAR_AGGRESSIVE_NSEC STRING_ARG  */
#line 1695 "util/configparser.y"
        {
		OUTYY(("P(server_aggressive_nsec:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else
			cfg_parser->cfg->aggressive_nsec =
				(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 4287 "util/configparser.c"
    break;

  case 415: /* server_ignore_cd_flag: VAR_IGNORE_CD_FLAG STRING_ARG  */
#line 1706 "util/configparser.y"
        {
		OUTYY(("P(server_ignore_cd_flag:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->ignore_cd = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 4299 "util/configparser.c"
    break;

  case 416: /* server_disable_edns_do: VAR_DISABLE_EDNS_DO STRING_ARG  */
#line 1715 "util/configparser.y"
        {
		OUTYY(("P(server_disable_edns_do:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->disable_edns_do = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 4311 "util/configparser.c"
    break;

  case 417: /* server_serve_expired: VAR_SERVE_EXPIRED STRING_ARG  */
#line 1724 "util/configparser.y"
        {
		OUTYY(("P(server_serve_expired:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->serve_expired = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 4323 "util/configparser.c"
    break;

  case 418: /* server_serve_expired_ttl: VAR_SERVE_EXPIRED_TTL STRING_ARG  */
#line 1733 "util/configparser.y"
        {
		OUTYY(("P(server_serve_expired_ttl:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->serve_expired_ttl = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 4335 "util/configparser.c"
    break;

  case 419: /* server_serve_expired_ttl_reset: VAR_SERVE_EXPIRED_TTL_RESET STRING_ARG  */
#line 1742 "util/configparser.y"
        {
		OUTYY(("P(server_serve_expired_ttl_reset:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->serve_expired_ttl_reset = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 4347 "util/configparser.c"
    break;

  case 420: /* server_serve_expired_reply_ttl: VAR_SERVE_EXPIRED_REPLY_TTL STRING_ARG  */
#line 1751 "util/configparser.y"
        {
		OUTYY(("P(server_serve_expired_reply_ttl:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->serve_expired_reply_ttl = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 4359 "util/configparser.c"
    break;

  case 421: /* server_serve_expired_client_timeout: VAR_SERVE_EXPIRED_CLIENT_TIMEOUT STRING_ARG  */
#line 1760 "util/configparser.y"
        {
		OUTYY(("P(server_serve_expired_client_timeout:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->serve_expired_client_timeout = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 4371 "util/configparser.c"
    break;

  case 422: /* server_ede_serve_expired: VAR_EDE_SERVE_EXPIRED STRING_ARG  */
#line 1769 "util/configparser.y"
        {
		OUTYY(("P(server_ede_serve_expired:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->ede_serve_expired = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 4383 "util/configparser.c"
    break;

  case 423: /* server_serve_original_ttl: VAR_SERVE_ORIGINAL_TTL STRING_ARG  */
#line 1778 "util/configparser.y"
        {
		OUTYY(("P(server_serve_original_ttl:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->serve_original_ttl = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 4395 "util/configparser.c"
    break;

  case 424: /* server_fake_dsa: VAR_FAKE_DSA STRING_ARG  */
#line 1787 "util/configparser.y"
        {
		OUTYY(("P(server_fake_dsa:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
#if defined(HAVE_SSL) || defined(HAVE_NETTLE)
		else fake_dsa = (strcmp((yyvsp[0].str), "yes")==0);
		if(fake_dsa)
			log_warn("test option fake_dsa is enabled");
#endif
		free((yyvsp[0].str));
	}
#line 4411 "util/configparser.c"
    break;

  case 425: /* server_fake_sha1: VAR_FAKE_SHA1 STRING_ARG  */
#line 1800 "util/configparser.y"
        {
		OUTYY(("P(server_fake_sha1:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
#if defined(HAVE_SSL) || defined(HAVE_NETTLE)
		else fake_sha1 = (strcmp((yyvsp[0].str), "yes")==0);
		if(fake_sha1)
			log_warn("test option fake_sha1 is enabled");
#endif
		free((yyvsp[0].str));
	}
#line 4427 "util/configparser.c"
    break;

  case 426: /* server_val_log_level: VAR_VAL_LOG_LEVEL STRING_ARG  */
#line 1813 "util/configparser.y"
        {
		OUTYY(("P(server_val_log_level:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->val_log_level = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 4439 "util/configparser.c"
    break;

  case 427: /* server_val_nsec3_keysize_iterations: VAR_VAL_NSEC3_KEYSIZE_ITERATIONS STRING_ARG  */
#line 1822 "util/configparser.y"
        {
		OUTYY(("P(server_val_nsec3_keysize_iterations:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->val_nsec3_key_iterations);
		cfg_parser->cfg->val_nsec3_key_iterations = (yyvsp[0].str);
	}
#line 4449 "util/configparser.c"
    break;

  case 428: /* server_zonemd_permissive_mode: VAR_ZONEMD_PERMISSIVE_MODE STRING_ARG  */
#line 1829 "util/configparser.y"
        {
		OUTYY(("P(server_zonemd_permissive_mode:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else	cfg_parser->cfg->zonemd_permissive_mode = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 4461 "util/configparser.c"
    break;

  case 429: /* server_add_holddown: VAR_ADD_HOLDDOWN STRING_ARG  */
#line 1838 "util/configparser.y"
        {
		OUTYY(("P(server_add_holddown:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->add_holddown = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 4473 "util/configparser.c"
    break;

  case 430: /* server_del_holddown: VAR_DEL_HOLDDOWN STRING_ARG  */
#line 1847 "util/configparser.y"
        {
		OUTYY(("P(server_del_holddown:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->del_holddown = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 4485 "util/configparser.c"
    break;

  case 431: /* server_keep_missing: VAR_KEEP_MISSING STRING_ARG  */
#line 1856 "util/configparser.y"
        {
		OUTYY(("P(server_keep_missing:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->keep_missing = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 4497 "util/configparser.c"
    break;

  case 432: /* server_permit_small_holddown: VAR_PERMIT_SMALL_HOLDDOWN STRING_ARG  */
#line 1865 "util/configparser.y"
        {
		OUTYY(("P(server_permit_small_holddown:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->permit_small_holddown =
			(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 4510 "util/configparser.c"
    break;

  case 433: /* server_key_cache_size: VAR_KEY_CACHE_SIZE STRING_ARG  */
#line 1875 "util/configparser.y"
        {
		OUTYY(("P(server_key_cache_size:%s)\n", (yyvsp[0].str)));
		if(!cfg_parse_memsize((yyvsp[0].str), &cfg_parser->cfg->key_cache_size))
			yyerror("memory size expected");
		free((yyvsp[0].str));
	}
#line 4521 "util/configparser.c"
    break;

  case 434: /* server_key_cache_slabs: VAR_KEY_CACHE_SLABS STRING_ARG  */
#line 1883 "util/configparser.y"
        {
		OUTYY(("P(server_key_cache_slabs:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0) {
			yyerror("number expected");
		} else {
			cfg_parser->cfg->key_cache_slabs = atoi((yyvsp[0].str));
			if(!is_pow2(cfg_parser->cfg->key_cache_slabs))
				yyerror("must be a power of 2");
		}
		free((yyvsp[0].str));
	}
#line 4537 "util/configparser.c"
    break;

  case 435: /* server_neg_cache_size: VAR_NEG_CACHE_SIZE STRING_ARG  */
#line 1896 "util/configparser.y"
        {
		OUTYY(("P(server_neg_cache_size:%s)\n", (yyvsp[0].str)));
		if(!cfg_parse_memsize((yyvsp[0].str), &cfg_parser->cfg->neg_cache_size))
			yyerror("memory size expected");
		free((yyvsp[0].str));
	}
#line 4548 "util/configparser.c"
    break;

  case 436: /* server_local_zone: VAR_LOCAL_ZONE STRING_ARG STRING_ARG  */
#line 1904 "util/configparser.y"
        {
		OUTYY(("P(server_local_zone:%s %s)\n", (yyvsp[-1].str), (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "static")!=0 && strcmp((yyvsp[0].str), "deny")!=0 &&
		   strcmp((yyvsp[0].str), "refuse")!=0 && strcmp((yyvsp[0].str), "redirect")!=0 &&
		   strcmp((yyvsp[0].str), "transparent")!=0 && strcmp((yyvsp[0].str), "nodefault")!=0
		   && strcmp((yyvsp[0].str), "typetransparent")!=0
		   && strcmp((yyvsp[0].str), "always_transparent")!=0
		   && strcmp((yyvsp[0].str), "block_a")!=0
		   && strcmp((yyvsp[0].str), "always_refuse")!=0
		   && strcmp((yyvsp[0].str), "always_nxdomain")!=0
		   && strcmp((yyvsp[0].str), "always_nodata")!=0
		   && strcmp((yyvsp[0].str), "always_deny")!=0
		   && strcmp((yyvsp[0].str), "always_null")!=0
		   && strcmp((yyvsp[0].str), "inform")!=0 && strcmp((yyvsp[0].str), "inform_deny")!=0
		   && strcmp((yyvsp[0].str), "inform_redirect") != 0
		   && strcmp((yyvsp[0].str), "ipset") != 0) {
			yyerror("local-zone type: expected static, deny, "
				"refuse, redirect, transparent, "
				"typetransparent, inform, inform_deny, "
				"inform_redirect, always_transparent, block_a, "
				"always_refuse, always_nxdomain, "
				"always_nodata, always_deny, always_null, "
				"nodefault or ipset");
			free((yyvsp[-1].str));
			free((yyvsp[0].str));
		} else if(strcmp((yyvsp[0].str), "nodefault")==0) {
			if(!cfg_strlist_insert(&cfg_parser->cfg->
				local_zones_nodefault, (yyvsp[-1].str)))
				fatal_exit("out of memory adding local-zone");
			free((yyvsp[0].str));
#ifdef USE_IPSET
		} else if(strcmp((yyvsp[0].str), "ipset")==0) {
			size_t len = strlen((yyvsp[-1].str));
			/* Make sure to add the trailing dot.
			 * These are str compared to domain names. */
			if((yyvsp[-1].str)[len-1] != '.') {
				if(!((yyvsp[-1].str) = realloc((yyvsp[-1].str), len+2))) {
					fatal_exit("out of memory adding local-zone");
				}
				(yyvsp[-1].str)[len] = '.';
				(yyvsp[-1].str)[len+1] = 0;
			}
			if(!cfg_strlist_insert(&cfg_parser->cfg->
				local_zones_ipset, (yyvsp[-1].str)))
				fatal_exit("out of memory adding local-zone");
			free((yyvsp[0].str));
#endif
		} else {
			if(!cfg_str2list_insert(&cfg_parser->cfg->local_zones,
				(yyvsp[-1].str), (yyvsp[0].str)))
				fatal_exit("out of memory adding local-zone");
		}
	}
#line 4606 "util/configparser.c"
    break;

  case 437: /* server_local_data: VAR_LOCAL_DATA STRING_ARG  */
#line 1959 "util/configparser.y"
        {
		OUTYY(("P(server_local_data:%s)\n", (yyvsp[0].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->local_data, (yyvsp[0].str)))
			fatal_exit("out of memory adding local-data");
	}
#line 4616 "util/configparser.c"
    break;

  case 438: /* server_local_data_ptr: VAR_LOCAL_DATA_PTR STRING_ARG  */
#line 1966 "util/configparser.y"
        {
		char* ptr;
		OUTYY(("P(server_local_data_ptr:%s)\n", (yyvsp[0].str)));
		ptr = cfg_ptr_reverse((yyvsp[0].str));
		free((yyvsp[0].str));
		if(ptr) {
			if(!cfg_strlist_insert(&cfg_parser->cfg->
				local_data, ptr))
				fatal_exit("out of memory adding local-data");
		} else {
			yyerror("local-data-ptr could not be reversed");
		}
	}
#line 4634 "util/configparser.c"
    break;

  case 439: /* server_minimal_responses: VAR_MINIMAL_RESPONSES STRING_ARG  */
#line 1981 "util/configparser.y"
        {
		OUTYY(("P(server_minimal_responses:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->minimal_responses =
			(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 4647 "util/configparser.c"
    break;

  case 440: /* server_rrset_roundrobin: VAR_RRSET_ROUNDROBIN STRING_ARG  */
#line 1991 "util/configparser.y"
        {
		OUTYY(("P(server_rrset_roundrobin:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->rrset_roundrobin =
			(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 4660 "util/configparser.c"
    break;

  case 441: /* server_unknown_server_time_limit: VAR_UNKNOWN_SERVER_TIME_LIMIT STRING_ARG  */
#line 2001 "util/configparser.y"
        {
		OUTYY(("P(server_unknown_server_time_limit:%s)\n", (yyvsp[0].str)));
		cfg_parser->cfg->unknown_server_time_limit = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 4670 "util/configparser.c"
    break;

  case 442: /* server_discard_timeout: VAR_DISCARD_TIMEOUT STRING_ARG  */
#line 2008 "util/configparser.y"
        {
		OUTYY(("P(server_discard_timeout:%s)\n", (yyvsp[0].str)));
		cfg_parser->cfg->discard_timeout = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 4680 "util/configparser.c"
    break;

  case 443: /* server_wait_limit: VAR_WAIT_LIMIT STRING_ARG  */
#line 2015 "util/configparser.y"
        {
		OUTYY(("P(server_wait_limit:%s)\n", (yyvsp[0].str)));
		cfg_parser->cfg->wait_limit = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 4690 "util/configparser.c"
    break;

  case 444: /* server_wait_limit_cookie: VAR_WAIT_LIMIT_COOKIE STRING_ARG  */
#line 2022 "util/configparser.y"
        {
		OUTYY(("P(server_wait_limit_cookie:%s)\n", (yyvsp[0].str)));
		cfg_parser->cfg->wait_limit_cookie = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 4700 "util/configparser.c"
    break;

  case 445: /* server_wait_limit_netblock: VAR_WAIT_LIMIT_NETBLOCK STRING_ARG STRING_ARG  */
#line 2029 "util/configparser.y"
        {
		OUTYY(("P(server_wait_limit_netblock:%s %s)\n", (yyvsp[-1].str), (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0) {
			yyerror("number expected");
			free((yyvsp[-1].str));
			free((yyvsp[0].str));
		} else {
			if(!cfg_str2list_insert(&cfg_parser->cfg->
				wait_limit_netblock, (yyvsp[-1].str), (yyvsp[0].str)))
				fatal_exit("out of memory adding "
					"wait-limit-netblock");
		}
	}
#line 4718 "util/configparser.c"
    break;

  case 446: /* server_wait_limit_cookie_netblock: VAR_WAIT_LIMIT_COOKIE_NETBLOCK STRING_ARG STRING_ARG  */
#line 2044 "util/configparser.y"
        {
		OUTYY(("P(server_wait_limit_cookie_netblock:%s %s)\n", (yyvsp[-1].str), (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0) {
			yyerror("number expected");
			free((yyvsp[-1].str));
			free((yyvsp[0].str));
		} else {
			if(!cfg_str2list_insert(&cfg_parser->cfg->
				wait_limit_cookie_netblock, (yyvsp[-1].str), (yyvsp[0].str)))
				fatal_exit("out of memory adding "
					"wait-limit-cookie-netblock");
		}
	}
#line 4736 "util/configparser.c"
    break;

  case 447: /* server_max_udp_size: VAR_MAX_UDP_SIZE STRING_ARG  */
#line 2059 "util/configparser.y"
        {
		OUTYY(("P(server_max_udp_size:%s)\n", (yyvsp[0].str)));
		cfg_parser->cfg->max_udp_size = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 4746 "util/configparser.c"
    break;

  case 448: /* server_nat64_prefix: VAR_NAT64_PREFIX STRING_ARG  */
#line 2066 "util/configparser.y"
        {
		OUTYY(("P(nat64_prefix:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->nat64_prefix);
		cfg_parser->cfg->nat64_prefix = (yyvsp[0].str);
	}
#line 4756 "util/configparser.c"
    break;

  case 449: /* server_define_tag: VAR_DEFINE_TAG STRING_ARG  */
#line 2073 "util/configparser.y"
        {
		char* p, *s = (yyvsp[0].str);
		OUTYY(("P(server_define_tag:%s)\n", (yyvsp[0].str)));
		while((p=strsep(&s, " \t\n")) != NULL) {
			if(*p) {
				if(!config_add_tag(cfg_parser->cfg, p))
					yyerror("could not define-tag, "
						"out of memory");
			}
		}
		free((yyvsp[0].str));
	}
#line 4773 "util/configparser.c"
    break;

  case 450: /* server_local_zone_tag: VAR_LOCAL_ZONE_TAG STRING_ARG STRING_ARG  */
#line 2087 "util/configparser.y"
        {
		size_t len = 0;
		uint8_t* bitlist = config_parse_taglist(cfg_parser->cfg, (yyvsp[0].str),
			&len);
		free((yyvsp[0].str));
		OUTYY(("P(server_local_zone_tag:%s)\n", (yyvsp[-1].str)));
		if(!bitlist) {
			yyerror("could not parse tags, (define-tag them first)");
			free((yyvsp[-1].str));
		}
		if(bitlist) {
			if(!cfg_strbytelist_insert(
				&cfg_parser->cfg->local_zone_tags,
				(yyvsp[-1].str), bitlist, len)) {
				yyerror("out of memory");
				free((yyvsp[-1].str));
			}
		}
	}
#line 4797 "util/configparser.c"
    break;

  case 451: /* server_access_control_tag: VAR_ACCESS_CONTROL_TAG STRING_ARG STRING_ARG  */
#line 2108 "util/configparser.y"
        {
		size_t len = 0;
		uint8_t* bitlist = config_parse_taglist(cfg_parser->cfg, (yyvsp[0].str),
			&len);
		free((yyvsp[0].str));
		OUTYY(("P(server_access_control_tag:%s)\n", (yyvsp[-1].str)));
		if(!bitlist) {
			yyerror("could not parse tags, (define-tag them first)");
			free((yyvsp[-1].str));
		}
		if(bitlist) {
			if(!cfg_strbytelist_insert(
				&cfg_parser->cfg->acl_tags,
				(yyvsp[-1].str), bitlist, len)) {
				yyerror("out of memory");
				free((yyvsp[-1].str));
			}
		}
	}
#line 4821 "util/configparser.c"
    break;

  case 452: /* server_access_control_tag_action: VAR_ACCESS_CONTROL_TAG_ACTION STRING_ARG STRING_ARG STRING_ARG  */
#line 2129 "util/configparser.y"
        {
		OUTYY(("P(server_access_control_tag_action:%s %s %s)\n", (yyvsp[-2].str), (yyvsp[-1].str), (yyvsp[0].str)));
		if(!cfg_str3list_insert(&cfg_parser->cfg->acl_tag_actions,
			(yyvsp[-2].str), (yyvsp[-1].str), (yyvsp[0].str))) {
			yyerror("out of memory");
			free((yyvsp[-2].str));
			free((yyvsp[-1].str));
			free((yyvsp[0].str));
		}
	}
#line 4836 "util/configparser.c"
    break;

  case 453: /* server_access_control_tag_data: VAR_ACCESS_CONTROL_TAG_DATA STRING_ARG STRING_ARG STRING_ARG  */
#line 2141 "util/configparser.y"
        {
		OUTYY(("P(server_access_control_tag_data:%s %s %s)\n", (yyvsp[-2].str), (yyvsp[-1].str), (yyvsp[0].str)));
		if(!cfg_str3list_insert(&cfg_parser->cfg->acl_tag_datas,
			(yyvsp[-2].str), (yyvsp[-1].str), (yyvsp[0].str))) {
			yyerror("out of memory");
			free((yyvsp[-2].str));
			free((yyvsp[-1].str));
			free((yyvsp[0].str));
		}
	}
#line 4851 "util/configparser.c"
    break;

  case 454: /* server_local_zone_override: VAR_LOCAL_ZONE_OVERRIDE STRING_ARG STRING_ARG STRING_ARG  */
#line 2153 "util/configparser.y"
        {
		OUTYY(("P(server_local_zone_override:%s %s %s)\n", (yyvsp[-2].str), (yyvsp[-1].str), (yyvsp[0].str)));
		if(!cfg_str3list_insert(&cfg_parser->cfg->local_zone_overrides,
			(yyvsp[-2].str), (yyvsp[-1].str), (yyvsp[0].str))) {
			yyerror("out of memory");
			free((yyvsp[-2].str));
			free((yyvsp[-1].str));
			free((yyvsp[0].str));
		}
	}
#line 4866 "util/configparser.c"
    break;

  case 455: /* server_interface_tag: VAR_INTERFACE_TAG STRING_ARG STRING_ARG  */
#line 2165 "util/configparser.y"
        {
		size_t len = 0;
		uint8_t* bitlist = config_parse_taglist(cfg_parser->cfg, (yyvsp[0].str),
			&len);
		free((yyvsp[0].str));
		OUTYY(("P(server_interface_tag:%s)\n", (yyvsp[-1].str)));
		if(!bitlist) {
			yyerror("could not parse tags, (define-tag them first)");
			free((yyvsp[-1].str));
		}
		if(bitlist) {
			if(!cfg_strbytelist_insert(
				&cfg_parser->cfg->interface_tags,
				(yyvsp[-1].str), bitlist, len)) {
				yyerror("out of memory");
				free((yyvsp[-1].str));
			}
		}
	}
#line 4890 "util/configparser.c"
    break;

  case 456: /* server_interface_tag_action: VAR_INTERFACE_TAG_ACTION STRING_ARG STRING_ARG STRING_ARG  */
#line 2186 "util/configparser.y"
        {
		OUTYY(("P(server_interface_tag_action:%s %s %s)\n", (yyvsp[-2].str), (yyvsp[-1].str), (yyvsp[0].str)));
		if(!cfg_str3list_insert(&cfg_parser->cfg->interface_tag_actions,
			(yyvsp[-2].str), (yyvsp[-1].str), (yyvsp[0].str))) {
			yyerror("out of memory");
			free((yyvsp[-2].str));
			free((yyvsp[-1].str));
			free((yyvsp[0].str));
		}
	}
#line 4905 "util/configparser.c"
    break;

  case 457: /* server_interface_tag_data: VAR_INTERFACE_TAG_DATA STRING_ARG STRING_ARG STRING_ARG  */
#line 2198 "util/configparser.y"
        {
		OUTYY(("P(server_interface_tag_data:%s %s %s)\n", (yyvsp[-2].str), (yyvsp[-1].str), (yyvsp[0].str)));
		if(!cfg_str3list_insert(&cfg_parser->cfg->interface_tag_datas,
			(yyvsp[-2].str), (yyvsp[-1].str), (yyvsp[0].str))) {
			yyerror("out of memory");
			free((yyvsp[-2].str));
			free((yyvsp[-1].str));
			free((yyvsp[0].str));
		}
	}
#line 4920 "util/configparser.c"
    break;

  case 458: /* server_ip_ratelimit: VAR_IP_RATELIMIT STRING_ARG  */
#line 2210 "util/configparser.y"
        {
		OUTYY(("P(server_ip_ratelimit:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->ip_ratelimit = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 4932 "util/configparser.c"
    break;

  case 459: /* server_ip_ratelimit_cookie: VAR_IP_RATELIMIT_COOKIE STRING_ARG  */
#line 2219 "util/configparser.y"
        {
		OUTYY(("P(server_ip_ratelimit_cookie:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->ip_ratelimit_cookie = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 4944 "util/configparser.c"
    break;

  case 460: /* server_ratelimit: VAR_RATELIMIT STRING_ARG  */
#line 2228 "util/configparser.y"
        {
		OUTYY(("P(server_ratelimit:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->ratelimit = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 4956 "util/configparser.c"
    break;

  case 461: /* server_ip_ratelimit_size: VAR_IP_RATELIMIT_SIZE STRING_ARG  */
#line 2237 "util/configparser.y"
        {
		OUTYY(("P(server_ip_ratelimit_size:%s)\n", (yyvsp[0].str)));
		if(!cfg_parse_memsize((yyvsp[0].str), &cfg_parser->cfg->ip_ratelimit_size))
			yyerror("memory size expected");
		free((yyvsp[0].str));
	}
#line 4967 "util/configparser.c"
    break;

  case 462: /* server_ratelimit_size: VAR_RATELIMIT_SIZE STRING_ARG  */
#line 2245 "util/configparser.y"
        {
		OUTYY(("P(server_ratelimit_size:%s)\n", (yyvsp[0].str)));
		if(!cfg_parse_memsize((yyvsp[0].str), &cfg_parser->cfg->ratelimit_size))
			yyerror("memory size expected");
		free((yyvsp[0].str));
	}
#line 4978 "util/configparser.c"
    break;

  case 463: /* server_ip_ratelimit_slabs: VAR_IP_RATELIMIT_SLABS STRING_ARG  */
#line 2253 "util/configparser.y"
        {
		OUTYY(("P(server_ip_ratelimit_slabs:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0) {
			yyerror("number expected");
		} else {
			cfg_parser->cfg->ip_ratelimit_slabs = atoi((yyvsp[0].str));
			if(!is_pow2(cfg_parser->cfg->ip_ratelimit_slabs))
				yyerror("must be a power of 2");
		}
		free((yyvsp[0].str));
	}
#line 4994 "util/configparser.c"
    break;

  case 464: /* server_ratelimit_slabs: VAR_RATELIMIT_SLABS STRING_ARG  */
#line 2266 "util/configparser.y"
        {
		OUTYY(("P(server_ratelimit_slabs:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0) {
			yyerror("number expected");
		} else {
			cfg_parser->cfg->ratelimit_slabs = atoi((yyvsp[0].str));
			if(!is_pow2(cfg_parser->cfg->ratelimit_slabs))
				yyerror("must be a power of 2");
		}
		free((yyvsp[0].str));
	}
#line 5010 "util/configparser.c"
    break;

  case 465: /* server_ratelimit_for_domain: VAR_RATELIMIT_FOR_DOMAIN STRING_ARG STRING_ARG  */
#line 2279 "util/configparser.y"
        {
		OUTYY(("P(server_ratelimit_for_domain:%s %s)\n", (yyvsp[-1].str), (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0) {
			yyerror("number expected");
			free((yyvsp[-1].str));
			free((yyvsp[0].str));
		} else {
			if(!cfg_str2list_insert(&cfg_parser->cfg->
				ratelimit_for_domain, (yyvsp[-1].str), (yyvsp[0].str)))
				fatal_exit("out of memory adding "
					"ratelimit-for-domain");
		}
	}
#line 5028 "util/configparser.c"
    break;

  case 466: /* server_ratelimit_below_domain: VAR_RATELIMIT_BELOW_DOMAIN STRING_ARG STRING_ARG  */
#line 2294 "util/configparser.y"
        {
		OUTYY(("P(server_ratelimit_below_domain:%s %s)\n", (yyvsp[-1].str), (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0) {
			yyerror("number expected");
			free((yyvsp[-1].str));
			free((yyvsp[0].str));
		} else {
			if(!cfg_str2list_insert(&cfg_parser->cfg->
				ratelimit_below_domain, (yyvsp[-1].str), (yyvsp[0].str)))
				fatal_exit("out of memory adding "
					"ratelimit-below-domain");
		}
	}
#line 5046 "util/configparser.c"
    break;

  case 467: /* server_ip_ratelimit_factor: VAR_IP_RATELIMIT_FACTOR STRING_ARG  */
#line 2309 "util/configparser.y"
        {
		OUTYY(("P(server_ip_ratelimit_factor:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->ip_ratelimit_factor = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 5058 "util/configparser.c"
    break;

  case 468: /* server_ratelimit_factor: VAR_RATELIMIT_FACTOR STRING_ARG  */
#line 2318 "util/configparser.y"
        {
		OUTYY(("P(server_ratelimit_factor:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->ratelimit_factor = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 5070 "util/configparser.c"
    break;

  case 469: /* server_ip_ratelimit_backoff: VAR_IP_RATELIMIT_BACKOFF STRING_ARG  */
#line 2327 "util/configparser.y"
        {
		OUTYY(("P(server_ip_ratelimit_backoff:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->ip_ratelimit_backoff =
			(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 5083 "util/configparser.c"
    break;

  case 470: /* server_ratelimit_backoff: VAR_RATELIMIT_BACKOFF STRING_ARG  */
#line 2337 "util/configparser.y"
        {
		OUTYY(("P(server_ratelimit_backoff:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->ratelimit_backoff =
			(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 5096 "util/configparser.c"
    break;

  case 471: /* server_outbound_msg_retry: VAR_OUTBOUND_MSG_RETRY STRING_ARG  */
#line 2347 "util/configparser.y"
        {
		OUTYY(("P(server_outbound_msg_retry:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->outbound_msg_retry = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 5108 "util/configparser.c"
    break;

  case 472: /* server_max_sent_count: VAR_MAX_SENT_COUNT STRING_ARG  */
#line 2356 "util/configparser.y"
        {
		OUTYY(("P(server_max_sent_count:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->max_sent_count = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 5120 "util/configparser.c"
    break;

  case 473: /* server_max_query_restarts: VAR_MAX_QUERY_RESTARTS STRING_ARG  */
#line 2365 "util/configparser.y"
        {
		OUTYY(("P(server_max_query_restarts:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->max_query_restarts = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 5132 "util/configparser.c"
    break;

  case 474: /* server_low_rtt: VAR_LOW_RTT STRING_ARG  */
#line 2374 "util/configparser.y"
        {
		OUTYY(("P(low-rtt option is deprecated, use fast-server-num instead)\n"));
		free((yyvsp[0].str));
	}
#line 5141 "util/configparser.c"
    break;

  case 475: /* server_fast_server_num: VAR_FAST_SERVER_NUM STRING_ARG  */
#line 2380 "util/configparser.y"
        {
		OUTYY(("P(server_fast_server_num:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) <= 0)
			yyerror("number expected");
		else cfg_parser->cfg->fast_server_num = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 5153 "util/configparser.c"
    break;

  case 476: /* server_fast_server_permil: VAR_FAST_SERVER_PERMIL STRING_ARG  */
#line 2389 "util/configparser.y"
        {
		OUTYY(("P(server_fast_server_permil:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->fast_server_permil = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 5165 "util/configparser.c"
    break;

  case 477: /* server_qname_minimisation: VAR_QNAME_MINIMISATION STRING_ARG  */
#line 2398 "util/configparser.y"
        {
		OUTYY(("P(server_qname_minimisation:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->qname_minimisation =
			(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 5178 "util/configparser.c"
    break;

  case 478: /* server_qname_minimisation_strict: VAR_QNAME_MINIMISATION_STRICT STRING_ARG  */
#line 2408 "util/configparser.y"
        {
		OUTYY(("P(server_qname_minimisation_strict:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->qname_minimisation_strict =
			(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 5191 "util/configparser.c"
    break;

  case 479: /* server_pad_queries: VAR_PAD_QUERIES STRING_ARG  */
#line 2418 "util/configparser.y"
        {
		OUTYY(("P(server_pad_queries:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->pad_queries =
			(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 5204 "util/configparser.c"
    break;

  case 480: /* server_pad_queries_block_size: VAR_PAD_QUERIES_BLOCK_SIZE STRING_ARG  */
#line 2428 "util/configparser.y"
        {
		OUTYY(("P(server_pad_queries_block_size:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0)
			yyerror("number expected");
		else cfg_parser->cfg->pad_queries_block_size = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 5216 "util/configparser.c"
    break;

  case 481: /* server_ipsecmod_enabled: VAR_IPSECMOD_ENABLED STRING_ARG  */
#line 2437 "util/configparser.y"
        {
	#ifdef USE_IPSECMOD
		OUTYY(("P(server_ipsecmod_enabled:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->ipsecmod_enabled = (strcmp((yyvsp[0].str), "yes")==0);
	#else
		OUTYY(("P(Compiled without IPsec module, ignoring)\n"));
	#endif
		free((yyvsp[0].str));
	}
#line 5232 "util/configparser.c"
    break;

  case 482: /* server_ipsecmod_ignore_bogus: VAR_IPSECMOD_IGNORE_BOGUS STRING_ARG  */
#line 2450 "util/configparser.y"
        {
	#ifdef USE_IPSECMOD
		OUTYY(("P(server_ipsecmod_ignore_bogus:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->ipsecmod_ignore_bogus = (strcmp((yyvsp[0].str), "yes")==0);
	#else
		OUTYY(("P(Compiled without IPsec module, ignoring)\n"));
	#endif
		free((yyvsp[0].str));
	}
#line 5248 "util/configparser.c"
    break;

  case 483: /* server_ipsecmod_hook: VAR_IPSECMOD_HOOK STRING_ARG  */
#line 2463 "util/configparser.y"
        {
	#ifdef USE_IPSECMOD
		OUTYY(("P(server_ipsecmod_hook:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->ipsecmod_hook);
		cfg_parser->cfg->ipsecmod_hook = (yyvsp[0].str);
	#else
		OUTYY(("P(Compiled without IPsec module, ignoring)\n"));
		free((yyvsp[0].str));
	#endif
	}
#line 5263 "util/configparser.c"
    break;

  case 484: /* server_ipsecmod_max_ttl: VAR_IPSECMOD_MAX_TTL STRING_ARG  */
#line 2475 "util/configparser.y"
        {
	#ifdef USE_IPSECMOD
		OUTYY(("P(server_ipsecmod_max_ttl:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->ipsecmod_max_ttl = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	#else
		OUTYY(("P(Compiled without IPsec module, ignoring)\n"));
		free((yyvsp[0].str));
	#endif
	}
#line 5280 "util/configparser.c"
    break;

  case 485: /* server_ipsecmod_whitelist: VAR_IPSECMOD_WHITELIST STRING_ARG  */
#line 2489 "util/configparser.y"
        {
	#ifdef USE_IPSECMOD
		OUTYY(("P(server_ipsecmod_whitelist:%s)\n", (yyvsp[0].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->ipsecmod_whitelist, (yyvsp[0].str)))
			yyerror("out of memory");
	#else
		OUTYY(("P(Compiled without IPsec module, ignoring)\n"));
		free((yyvsp[0].str));
	#endif
	}
#line 5295 "util/configparser.c"
    break;

  case 486: /* server_ipsecmod_strict: VAR_IPSECMOD_STRICT STRING_ARG  */
#line 2501 "util/configparser.y"
        {
	#ifdef USE_IPSECMOD
		OUTYY(("P(server_ipsecmod_strict:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->ipsecmod_strict = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	#else
		OUTYY(("P(Compiled without IPsec module, ignoring)\n"));
		free((yyvsp[0].str));
	#endif
	}
#line 5312 "util/configparser.c"
    break;

  case 487: /* server_edns_client_string: VAR_EDNS_CLIENT_STRING STRING_ARG STRING_ARG  */
#line 2515 "util/configparser.y"
        {
		OUTYY(("P(server_edns_client_string:%s %s)\n", (yyvsp[-1].str), (yyvsp[0].str)));
		if(!cfg_str2list_insert(
			&cfg_parser->cfg->edns_client_strings, (yyvsp[-1].str), (yyvsp[0].str)))
			fatal_exit("out of memory adding "
				"edns-client-string");
	}
#line 5324 "util/configparser.c"
    break;

  case 488: /* server_edns_client_string_opcode: VAR_EDNS_CLIENT_STRING_OPCODE STRING_ARG  */
#line 2524 "util/configparser.y"
        {
		OUTYY(("P(edns_client_string_opcode:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("option code expected");
		else if(atoi((yyvsp[0].str)) > 65535 || atoi((yyvsp[0].str)) < 0)
			yyerror("option code must be in interval [0, 65535]");
		else cfg_parser->cfg->edns_client_string_opcode = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 5338 "util/configparser.c"
    break;

  case 489: /* server_ede: VAR_EDE STRING_ARG  */
#line 2535 "util/configparser.y"
        {
		OUTYY(("P(server_ede:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->ede = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 5350 "util/configparser.c"
    break;

  case 490: /* server_dns_error_reporting: VAR_DNS_ERROR_REPORTING STRING_ARG  */
#line 2544 "util/configparser.y"
        {
		OUTYY(("P(server_dns_error_reporting:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->dns_error_reporting = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 5362 "util/configparser.c"
    break;

  case 491: /* server_proxy_protocol_port: VAR_PROXY_PROTOCOL_PORT STRING_ARG  */
#line 2553 "util/configparser.y"
        {
		OUTYY(("P(server_proxy_protocol_port:%s)\n", (yyvsp[0].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->proxy_protocol_port, (yyvsp[0].str)))
			yyerror("out of memory");
	}
#line 5372 "util/configparser.c"
    break;

  case 492: /* stub_name: VAR_NAME STRING_ARG  */
#line 2560 "util/configparser.y"
        {
		OUTYY(("P(name:%s)\n", (yyvsp[0].str)));
		if(cfg_parser->cfg->stubs->name)
			yyerror("stub name override, there must be one name "
				"for one stub-zone");
		free(cfg_parser->cfg->stubs->name);
		cfg_parser->cfg->stubs->name = (yyvsp[0].str);
	}
#line 5385 "util/configparser.c"
    break;

  case 493: /* stub_host: VAR_STUB_HOST STRING_ARG  */
#line 2570 "util/configparser.y"
        {
		OUTYY(("P(stub-host:%s)\n", (yyvsp[0].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->stubs->hosts, (yyvsp[0].str)))
			yyerror("out of memory");
	}
#line 5395 "util/configparser.c"
    break;

  case 494: /* stub_addr: VAR_STUB_ADDR STRING_ARG  */
#line 2577 "util/configparser.y"
        {
		OUTYY(("P(stub-addr:%s)\n", (yyvsp[0].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->stubs->addrs, (yyvsp[0].str)))
			yyerror("out of memory");
	}
#line 5405 "util/configparser.c"
    break;

  case 495: /* stub_first: VAR_STUB_FIRST STRING_ARG  */
#line 2584 "util/configparser.y"
        {
		OUTYY(("P(stub-first:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->stubs->isfirst=(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 5417 "util/configparser.c"
    break;

  case 496: /* stub_no_cache: VAR_STUB_NO_CACHE STRING_ARG  */
#line 2593 "util/configparser.y"
        {
		OUTYY(("P(stub-no-cache:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->stubs->no_cache=(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 5429 "util/configparser.c"
    break;

  case 497: /* stub_ssl_upstream: VAR_STUB_SSL_UPSTREAM STRING_ARG  */
#line 2602 "util/configparser.y"
        {
		OUTYY(("P(stub-ssl-upstream:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->stubs->ssl_upstream =
			(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 5442 "util/configparser.c"
    break;

  case 498: /* stub_tcp_upstream: VAR_STUB_TCP_UPSTREAM STRING_ARG  */
#line 2612 "util/configparser.y"
        {
                OUTYY(("P(stub-tcp-upstream:%s)\n", (yyvsp[0].str)));
                if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
                        yyerror("expected yes or no.");
                else cfg_parser->cfg->stubs->tcp_upstream =
                        (strcmp((yyvsp[0].str), "yes")==0);
                free((yyvsp[0].str));
        }
#line 5455 "util/configparser.c"
    break;

  case 499: /* stub_prime: VAR_STUB_PRIME STRING_ARG  */
#line 2622 "util/configparser.y"
        {
		OUTYY(("P(stub-prime:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->stubs->isprime =
			(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 5468 "util/configparser.c"
    break;

  case 500: /* forward_name: VAR_NAME STRING_ARG  */
#line 2632 "util/configparser.y"
        {
		OUTYY(("P(name:%s)\n", (yyvsp[0].str)));
		if(cfg_parser->cfg->forwards->name)
			yyerror("forward name override, there must be one "
				"name for one forward-zone");
		free(cfg_parser->cfg->forwards->name);
		cfg_parser->cfg->forwards->name = (yyvsp[0].str);
	}
#line 5481 "util/configparser.c"
    break;

  case 501: /* forward_host: VAR_FORWARD_HOST STRING_ARG  */
#line 2642 "util/configparser.y"
        {
		OUTYY(("P(forward-host:%s)\n", (yyvsp[0].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->forwards->hosts, (yyvsp[0].str)))
			yyerror("out of memory");
	}
#line 5491 "util/configparser.c"
    break;

  case 502: /* forward_addr: VAR_FORWARD_ADDR STRING_ARG  */
#line 2649 "util/configparser.y"
        {
		OUTYY(("P(forward-addr:%s)\n", (yyvsp[0].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->forwards->addrs, (yyvsp[0].str)))
			yyerror("out of memory");
	}
#line 5501 "util/configparser.c"
    break;

  case 503: /* forward_first: VAR_FORWARD_FIRST STRING_ARG  */
#line 2656 "util/configparser.y"
        {
		OUTYY(("P(forward-first:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "no") != 0)
			yyerror("forward-first must be no: recursive fallback is prohibited");
		else cfg_parser->cfg->forwards->isfirst=0;
		free((yyvsp[0].str));
	}
#line 5513 "util/configparser.c"
    break;

  case 504: /* forward_no_cache: VAR_FORWARD_NO_CACHE STRING_ARG  */
#line 2665 "util/configparser.y"
        {
		OUTYY(("P(forward-no-cache:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->forwards->no_cache=(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 5525 "util/configparser.c"
    break;

  case 505: /* forward_ssl_upstream: VAR_FORWARD_SSL_UPSTREAM STRING_ARG  */
#line 2674 "util/configparser.y"
        {
		OUTYY(("P(forward-ssl-upstream:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->forwards->ssl_upstream =
			(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 5538 "util/configparser.c"
    break;

  case 506: /* forward_tcp_upstream: VAR_FORWARD_TCP_UPSTREAM STRING_ARG  */
#line 2684 "util/configparser.y"
        {
                OUTYY(("P(forward-tcp-upstream:%s)\n", (yyvsp[0].str)));
                if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
                        yyerror("expected yes or no.");
                else cfg_parser->cfg->forwards->tcp_upstream =
                        (strcmp((yyvsp[0].str), "yes")==0);
                free((yyvsp[0].str));
        }
#line 5551 "util/configparser.c"
    break;

  case 507: /* auth_name: VAR_NAME STRING_ARG  */
#line 2694 "util/configparser.y"
        {
		OUTYY(("P(name:%s)\n", (yyvsp[0].str)));
		if(cfg_parser->cfg->auths->name)
			yyerror("auth name override, there must be one name "
				"for one auth-zone");
		free(cfg_parser->cfg->auths->name);
		cfg_parser->cfg->auths->name = (yyvsp[0].str);
	}
#line 5564 "util/configparser.c"
    break;

  case 508: /* auth_zonefile: VAR_ZONEFILE STRING_ARG  */
#line 2704 "util/configparser.y"
        {
		OUTYY(("P(zonefile:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->auths->zonefile);
		cfg_parser->cfg->auths->zonefile = (yyvsp[0].str);
	}
#line 5574 "util/configparser.c"
    break;

  case 509: /* auth_master: VAR_MASTER STRING_ARG  */
#line 2711 "util/configparser.y"
        {
		OUTYY(("P(master:%s)\n", (yyvsp[0].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->auths->masters, (yyvsp[0].str)))
			yyerror("out of memory");
	}
#line 5584 "util/configparser.c"
    break;

  case 510: /* auth_url: VAR_URL STRING_ARG  */
#line 2718 "util/configparser.y"
        {
		OUTYY(("P(url:%s)\n", (yyvsp[0].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->auths->urls, (yyvsp[0].str)))
			yyerror("out of memory");
	}
#line 5594 "util/configparser.c"
    break;

  case 511: /* auth_allow_notify: VAR_ALLOW_NOTIFY STRING_ARG  */
#line 2725 "util/configparser.y"
        {
		OUTYY(("P(allow-notify:%s)\n", (yyvsp[0].str)));
		if(!cfg_strlist_insert(&cfg_parser->cfg->auths->allow_notify,
			(yyvsp[0].str)))
			yyerror("out of memory");
	}
#line 5605 "util/configparser.c"
    break;

  case 512: /* auth_zonemd_check: VAR_ZONEMD_CHECK STRING_ARG  */
#line 2733 "util/configparser.y"
        {
		OUTYY(("P(zonemd-check:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->auths->zonemd_check =
			(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 5618 "util/configparser.c"
    break;

  case 513: /* auth_zonemd_reject_absence: VAR_ZONEMD_REJECT_ABSENCE STRING_ARG  */
#line 2743 "util/configparser.y"
        {
		OUTYY(("P(zonemd-reject-absence:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->auths->zonemd_reject_absence =
			(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 5631 "util/configparser.c"
    break;

  case 514: /* auth_for_downstream: VAR_FOR_DOWNSTREAM STRING_ARG  */
#line 2753 "util/configparser.y"
        {
		OUTYY(("P(for-downstream:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->auths->for_downstream =
			(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 5644 "util/configparser.c"
    break;

  case 515: /* auth_for_upstream: VAR_FOR_UPSTREAM STRING_ARG  */
#line 2763 "util/configparser.y"
        {
		OUTYY(("P(for-upstream:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->auths->for_upstream =
			(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 5657 "util/configparser.c"
    break;

  case 516: /* auth_fallback_enabled: VAR_FALLBACK_ENABLED STRING_ARG  */
#line 2773 "util/configparser.y"
        {
		OUTYY(("P(fallback-enabled:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->auths->fallback_enabled =
			(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 5670 "util/configparser.c"
    break;

  case 517: /* rcstart: VAR_REMOTE_CONTROL  */
#line 2783 "util/configparser.y"
        {
		OUTYY(("\nP(remote-control:)\n"));
		cfg_parser->started_toplevel = 1;
	}
#line 5679 "util/configparser.c"
    break;

  case 528: /* rc_control_enable: VAR_CONTROL_ENABLE STRING_ARG  */
#line 2795 "util/configparser.y"
        {
		OUTYY(("P(control_enable:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->remote_control_enable =
			(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 5692 "util/configparser.c"
    break;

  case 529: /* rc_control_port: VAR_CONTROL_PORT STRING_ARG  */
#line 2805 "util/configparser.y"
        {
		OUTYY(("P(control_port:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0)
			yyerror("control port number expected");
		else cfg_parser->cfg->control_port = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 5704 "util/configparser.c"
    break;

  case 530: /* rc_control_interface: VAR_CONTROL_INTERFACE STRING_ARG  */
#line 2814 "util/configparser.y"
        {
		OUTYY(("P(control_interface:%s)\n", (yyvsp[0].str)));
		if(!cfg_strlist_append(&cfg_parser->cfg->control_ifs, (yyvsp[0].str)))
			yyerror("out of memory");
	}
#line 5714 "util/configparser.c"
    break;

  case 531: /* rc_control_use_cert: VAR_CONTROL_USE_CERT STRING_ARG  */
#line 2821 "util/configparser.y"
        {
		OUTYY(("P(control_use_cert:%s)\n", (yyvsp[0].str)));
		cfg_parser->cfg->control_use_cert = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 5724 "util/configparser.c"
    break;

  case 532: /* rc_server_key_file: VAR_SERVER_KEY_FILE STRING_ARG  */
#line 2828 "util/configparser.y"
        {
		OUTYY(("P(rc_server_key_file:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->server_key_file);
		cfg_parser->cfg->server_key_file = (yyvsp[0].str);
	}
#line 5734 "util/configparser.c"
    break;

  case 533: /* rc_server_cert_file: VAR_SERVER_CERT_FILE STRING_ARG  */
#line 2835 "util/configparser.y"
        {
		OUTYY(("P(rc_server_cert_file:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->server_cert_file);
		cfg_parser->cfg->server_cert_file = (yyvsp[0].str);
	}
#line 5744 "util/configparser.c"
    break;

  case 534: /* rc_control_key_file: VAR_CONTROL_KEY_FILE STRING_ARG  */
#line 2842 "util/configparser.y"
        {
		OUTYY(("P(rc_control_key_file:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->control_key_file);
		cfg_parser->cfg->control_key_file = (yyvsp[0].str);
	}
#line 5754 "util/configparser.c"
    break;

  case 535: /* rc_control_cert_file: VAR_CONTROL_CERT_FILE STRING_ARG  */
#line 2849 "util/configparser.y"
        {
		OUTYY(("P(rc_control_cert_file:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->control_cert_file);
		cfg_parser->cfg->control_cert_file = (yyvsp[0].str);
	}
#line 5764 "util/configparser.c"
    break;

  case 536: /* server_disable_dnssec_lame_check: VAR_DISABLE_DNSSEC_LAME_CHECK STRING_ARG  */
#line 2856 "util/configparser.y"
        {
		OUTYY(("P(disable_dnssec_lame_check:%s)\n", (yyvsp[0].str)));
		if (strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->disable_dnssec_lame_check =
			(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 5777 "util/configparser.c"
    break;

  case 537: /* server_log_identity: VAR_LOG_IDENTITY STRING_ARG  */
#line 2866 "util/configparser.y"
        {
		OUTYY(("P(server_log_identity:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->log_identity);
		cfg_parser->cfg->log_identity = (yyvsp[0].str);
	}
#line 5787 "util/configparser.c"
    break;

  case 538: /* server_tcp_connection_limit: VAR_TCP_CONNECTION_LIMIT STRING_ARG STRING_ARG  */
#line 2873 "util/configparser.y"
        {
		OUTYY(("P(server_tcp_connection_limit:%s %s)\n", (yyvsp[-1].str), (yyvsp[0].str)));
		if (atoi((yyvsp[0].str)) < 0)
			yyerror("positive number expected");
		else {
			if(!cfg_str2list_insert(&cfg_parser->cfg->tcp_connection_limits, (yyvsp[-1].str), (yyvsp[0].str)))
				fatal_exit("out of memory adding tcp connection limit");
		}
	}
#line 5801 "util/configparser.c"
    break;

  case 539: /* server_answer_cookie: VAR_ANSWER_COOKIE STRING_ARG  */
#line 2884 "util/configparser.y"
        {
		OUTYY(("P(server_answer_cookie:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->do_answer_cookie = (strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 5813 "util/configparser.c"
    break;

  case 540: /* server_cookie_secret: VAR_COOKIE_SECRET STRING_ARG  */
#line 2893 "util/configparser.y"
        {
		uint8_t secret[32];
		size_t secret_len = sizeof(secret);

		OUTYY(("P(server_cookie_secret:%s)\n", (yyvsp[0].str)));
		if(sldns_str2wire_hex_buf((yyvsp[0].str), secret, &secret_len)
		|| (secret_len != 16))
			yyerror("expected 128 bit hex string");
		else {
			cfg_parser->cfg->cookie_secret_len = secret_len;
			memcpy(cfg_parser->cfg->cookie_secret, secret, sizeof(secret));
		}
		free((yyvsp[0].str));
	}
#line 5832 "util/configparser.c"
    break;

  case 541: /* server_cookie_secret_file: VAR_COOKIE_SECRET_FILE STRING_ARG  */
#line 2909 "util/configparser.y"
        {
		OUTYY(("P(cookie_secret_file:%s)\n", (yyvsp[0].str)));
		free(cfg_parser->cfg->cookie_secret_file);
		cfg_parser->cfg->cookie_secret_file = (yyvsp[0].str);
	}
#line 5842 "util/configparser.c"
    break;

  case 542: /* server_iter_scrub_ns: VAR_ITER_SCRUB_NS STRING_ARG  */
#line 2916 "util/configparser.y"
        {
		OUTYY(("P(server_iter_scrub_ns:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->iter_scrub_ns = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 5854 "util/configparser.c"
    break;

  case 543: /* server_iter_scrub_cname: VAR_ITER_SCRUB_CNAME STRING_ARG  */
#line 2925 "util/configparser.y"
        {
		OUTYY(("P(server_iter_scrub_cname:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->iter_scrub_cname = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 5866 "util/configparser.c"
    break;

  case 544: /* server_iter_scrub_rrsig: VAR_ITER_SCRUB_RRSIG STRING_ARG  */
#line 2934 "util/configparser.y"
        {
		OUTYY(("P(server_iter_scrub_rrsig:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->iter_scrub_rrsig = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 5878 "util/configparser.c"
    break;

  case 545: /* server_max_global_quota: VAR_MAX_GLOBAL_QUOTA STRING_ARG  */
#line 2943 "util/configparser.y"
        {
		OUTYY(("P(server_max_global_quota:%s)\n", (yyvsp[0].str)));
		if(atoi((yyvsp[0].str)) == 0 && strcmp((yyvsp[0].str), "0") != 0)
			yyerror("number expected");
		else cfg_parser->cfg->max_global_quota = atoi((yyvsp[0].str));
		free((yyvsp[0].str));
	}
#line 5890 "util/configparser.c"
    break;

  case 546: /* server_iter_scrub_promiscuous: VAR_ITER_SCRUB_PROMISCUOUS STRING_ARG  */
#line 2952 "util/configparser.y"
        {
		OUTYY(("P(server_iter_scrub_promiscuous:%s)\n", (yyvsp[0].str)));
		if(strcmp((yyvsp[0].str), "yes") != 0 && strcmp((yyvsp[0].str), "no") != 0)
			yyerror("expected yes or no.");
		else cfg_parser->cfg->iter_scrub_promiscuous =
			(strcmp((yyvsp[0].str), "yes")==0);
		free((yyvsp[0].str));
	}
#line 5903 "util/configparser.c"
    break;

  case 547: /* ipsetstart: VAR_IPSET  */
#line 2962 "util/configparser.y"
        {
		OUTYY(("\nP(ipset:)\n"));
		cfg_parser->started_toplevel = 1;
	}
#line 5912 "util/configparser.c"
    break;

  case 552: /* ipset_name_v4: VAR_IPSET_NAME_V4 STRING_ARG  */
#line 2972 "util/configparser.y"
        {
	#ifdef USE_IPSET
		OUTYY(("P(name-v4:%s)\n", (yyvsp[0].str)));
		if(cfg_parser->cfg->ipset_name_v4)
			yyerror("ipset name v4 override, there must be one "
				"name for ip v4");
		free(cfg_parser->cfg->ipset_name_v4);
		cfg_parser->cfg->ipset_name_v4 = (yyvsp[0].str);
	#else
		OUTYY(("P(Compiled without ipset, ignoring)\n"));
		free((yyvsp[0].str));
	#endif
	}
#line 5930 "util/configparser.c"
    break;

  case 553: /* ipset_name_v6: VAR_IPSET_NAME_V6 STRING_ARG  */
#line 2987 "util/configparser.y"
        {
	#ifdef USE_IPSET
		OUTYY(("P(name-v6:%s)\n", (yyvsp[0].str)));
		if(cfg_parser->cfg->ipset_name_v6)
			yyerror("ipset name v6 override, there must be one "
				"name for ip v6");
		free(cfg_parser->cfg->ipset_name_v6);
		cfg_parser->cfg->ipset_name_v6 = (yyvsp[0].str);
	#else
		OUTYY(("P(Compiled without ipset, ignoring)\n"));
		free((yyvsp[0].str));
	#endif
	}
#line 5948 "util/configparser.c"
    break;


#line 5952 "util/configparser.c"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (YY_("syntax error"));
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 3001 "util/configparser.y"


/* parse helper routines could be here */
static void
validate_acl_action(const char* action)
{
	if(strcmp(action, "deny")!=0 &&
		strcmp(action, "refuse")!=0 &&
		strcmp(action, "deny_non_local")!=0 &&
		strcmp(action, "refuse_non_local")!=0 &&
		strcmp(action, "allow_setrd")!=0 &&
		strcmp(action, "allow")!=0 &&
		strcmp(action, "allow_snoop")!=0 &&
		strcmp(action, "allow_cookie")!=0)
	{
		yyerror("expected deny, refuse, deny_non_local, "
			"refuse_non_local, allow, allow_setrd, "
			"allow_snoop or allow_cookie as access control action");
	}
}
