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
 *  $Id: queue.h,v 1.1.1.1 2006/05/23 13:53:59 mainakc Exp $
 *
 *************************************************************************/
#ifndef __QUEUE_H__
#define __QUEUE_H__

#include "sim.h"
#include "tasking.h"

/*
 * Circular buffer queue pointer implementation
 *
 */

#if defined(ASYNCHRONOUS)
#error These are clocked queues
#endif

class UnCheckPoint;

class Queue {
 public:
  Queue (char *name, int size);
  // queue size
  
  int Get (void);
  // Get data: returns slot for the next used spot

  int Put (void);
  // Put data: returns slot for next free slot

  int isEmpty (void);
  // Return 1 if the queue is empty, 0 otherwise

  int isFull (void);
  // Return 1 if the queue is full, 0 otherwise
  
  ~Queue ();

  int GetHeadIndex(void);
  int PutHeadIndex(void);

  count_t Getg(void);

  count_t Getp(void);

  count_t GetgetCount(void);

  count_t GetputCount(void);

  // checkpoint queue helpers
  void Save (FILE *fp);
  void Restore (FILE *fp, UnCheckPoint *uc);

 private:
  eventcount *pc, *gc;
  count_t p,g;
  char *n0, *n1;
  int put, get;
  int putCount,getCount;
  int sz;
};
  





#endif /* __QUEUE_H__ */
