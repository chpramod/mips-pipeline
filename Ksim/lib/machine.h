/*-*-mode:c++-*-**********************************************************
 *
 *  Copyright (c) 1999 Cornell University
 *  Computer Systems Laboratory
 *  Ithaca, NY 14853
 *  All Rights Reserved
 *
 *  Permission to use, copy, modify, and distribute this software
 *  and its documentation for any purpose and without fee is hereby
 *  granted, provided that the above copyright notice appear in all
 *  copies. Cornell University makes no representations
 *  about the suitability of this software for any purpose. It is
 *  provided "as is" without express or implied warranty. Export of this
 *  software outside of the United States of America may require an
 *  export license.
 *
 *  $Id: machine.h,v 1.1.1.1 2006/05/23 13:53:59 mainakc Exp $
 *
 *************************************************************************/
#ifndef __MYMACHINE_H__
#define __MYMACHINE_H__

#if defined(__FreeBSD__)
#include <machine/endian.h>
#else
#if defined(__linux__)
#include <endian.h>
#endif
#endif

#if !defined (BIG_ENDIAN) && !defined(LITTLE_ENDIAN)

/* need lots of crap here */
#define BIG_ENDIAN 0
#define LITTLE_ENDIAN 1

#if defined (__i386__)

#define BYTE_ORDER LITTLE_ENDIAN

#elif defined (__sparc__)

#define BYTE_ORDER BIG_ENDIAN

#elif defined(__sgi)

#define BYTE_ORDER BIG_ENDIAN

#elif defined(__alpha__)

#define BYTE_ORDER LITTLE_ENDIAN

#else

#error What on earth?

#endif

#endif

#ifndef BYTE_ORDER

#if defined(LITTLE_ENDIAN) && !defined(BIG_ENDIAN)

#define BYTE_ORDER    LITTLE_ENDIAN
#define BIG_ENDIAN !LITTLE_ENDIAN

#if BYTE_ORDER == BIG_ENDIAN
#error Wowee
#endif

#elif defined (BIG_ENDIAN) && !defined(LITTLE_ENDIAN)

#define BYTE_ORDER    BIG_ENDIAN
#define LITTLE_ENDIAN !BIG_ENDIAN

#if BYTE_ORDER == LITTLE_ENDIAN
#error Wowee
#endif

#else

#error Fix byte order file!

#endif

#endif


#endif /* __MYMACHINE_H__ */
