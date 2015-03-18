/*
 * Sun RPC is a product of Sun Microsystems, Inc. and is provided for
 * unrestricted use provided that this legend is included on all tape
 * media and as a part of the software program in whole or part.  Users
 * may copy or modify Sun RPC without charge, but are not authorized
 * to license or distribute it to anyone else except as part of a product or
 * program developed by the user.
 *
 * SUN RPC IS PROVIDED AS IS WITH NO WARRANTIES OF ANY KIND INCLUDING THE
 * WARRANTIES OF DESIGN, MERCHANTIBILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE, OR ARISING FROM A COURSE OF DEALING, USAGE OR TRADE PRACTICE.
 *
 * Sun RPC is provided with no support and without any obligation on the
 * part of Sun Microsystems, Inc. to assist in its use, correction,
 * modification or enhancement.
 *
 * SUN MICROSYSTEMS, INC. SHALL HAVE NO LIABILITY WITH RESPECT TO THE
 * INFRINGEMENT OF COPYRIGHTS, TRADE SECRETS OR ANY PATENTS BY SUN RPC
 * OR ANY PART THEREOF.
 *
 * In no event will Sun Microsystems, Inc. be liable for any lost revenue
 * or profits or other special, indirect and consequential damages, even if
 * Sun has been advised of the possibility of such damages.
 *
 * Sun Microsystems, Inc.
 * 2550 Garcia Avenue
 * Mountain View, California  94043
 */
/* fixincludes should not add extern "C" to this file */
/*
 * Rpc additions to <sys/types.h>
 */
#ifndef _RPC_TYPES_H
#define _RPC_TYPES_H 1

#include <hdElastosMantle.h>
#include <lwip/netdb.h>
#include <lwip/sockets.h>

#include <string.h>

#ifndef RT_USING_MINILIBC
typedef unsigned int u_int;
typedef unsigned char u_char;
typedef unsigned long u_long;

typedef rt_int8_t   int8_t;
typedef UInt8  uint8_t;
typedef rt_int16_t  int16_t;
typedef UInt16 uint16_t;
typedef Int32  int32_t;
typedef UInt32 uint32_t;
#else
#include <sys/types.h>
#include <stdint.h>
#endif

typedef long long   int64_t;
typedef unsigned long long uint64_t;

typedef int bool_t;
typedef int enum_t;

#ifndef RT_USING_NEWLIB
typedef unsigned long dev_t;
#endif

#if !defined(RT_USING_NEWLIB) && !defined(RT_USING_MINILIBC)
typedef Int32  ssize_t;
#endif

/* This needs to be changed to uint32_t in the future */
typedef unsigned long rpcprog_t;
typedef unsigned long rpcvers_t;
typedef unsigned long rpcproc_t;
typedef unsigned long rpcprot_t;
typedef unsigned long rpcport_t;

#define        __dontcare__    -1

#ifndef FALSE
# define  FALSE   (0)
#endif

#ifndef TRUE
# define  TRUE    (1)
#endif

#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN  64
#endif

#endif /* rpc/types.h */
