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
 *  $Id: mutex.h,v 1.1.1.1 2006/05/23 13:53:59 mainakc Exp $
 *
 *************************************************************************/
#ifndef __MUTEX_H__
#define __MUTEX_H__

#include "thread.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  thread_t *hd, *tl;
  int busy;
} mutex_t;

typedef struct {
  thread_t *hd, *tl;
  mutex_t *lock;
} cond_t;

mutex_t *mutex_new (void);
void mutex_free (mutex_t *);
void mutex_lock (mutex_t *);
void mutex_unlock (mutex_t *);

cond_t *cond_new (mutex_t *);
void cond_free (cond_t *);
void cond_wait (cond_t *);
void cond_signal (cond_t *);
int cond_waiting (cond_t *);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __MUTEX_H__ */
