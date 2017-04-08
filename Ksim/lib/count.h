/*************************************************************************
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
 *
 *  $Id: count.h,v 1.1.1.1 2006/05/23 13:53:59 mainakc Exp $
 *
 *************************************************************************/
#ifndef __COUNT_H__
#define __COUNT_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  unsigned int cnt;
  thread_t *hd, *tl;
} count_t;

count_t *count_new (int init_val);
void _count_free (count_t *, char *file, int line);
void count_await (count_t *, unsigned int val);
void count_increment (count_t *, unsigned int amount);
#define count_advance(c) count_increment(c,1)

#define count_free(c) _count_free(c,__FILE__,__LINE__)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __COUNT_H__ */
