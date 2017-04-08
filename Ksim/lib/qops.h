/*************************************************************************
 *
 *  Copyright (c) 1998 Cornell University
 *  Computer Systems Laboratory
 *  Ithaca, NY 14853
 *  All Rights Reserved
 *
 *  Permission to use, copy, modify, and distribute this software
 *  and its documentation for any purpose and without fee is hereby
 *  granted, provided that the above copyright notice appear in all
 *  copies. The California Institute of Technology makes no representations
 *  about the suitability of this software for any purpose. It is
 *  provided "as is" without express or implied warranty. Export of this
 *  software outside of the United States of America may require an
 *  export license.
 *
 *  $Id: qops.h,v 1.1.1.1 2006/05/23 13:53:59 mainakc Exp $
 *
 *************************************************************************/
#ifndef __QOPS_H__
#define __QOPS_H__

/*
 *  Queue format:
 *
 *    Two pointers: head (hd) and tail (tl)
 *    Elements have a "next" field for the queue.
 *
 *    Queue empty when hd == NULL
 *    When hd == NULL, "tl" the value of tl can be anything
 *  
 *
 */

#define q_ins(hd,tl,p)				\
  p->next = NULL;				\
  if (hd == NULL) {				\
    hd = p;					\
  }						\
  else {					\
    tl->next = p;				\
  }						\
  tl = p;


#define q_del(hd, tl, t)						 \
  if (hd) {								 \
    t = hd;								 \
    hd = hd->next;							 \
  }									 \
  else									 \
    t = NULL;

#define q_delete_item(hd,tl,prev,cur)		\
   if (!prev) {					\
     q_del(hd,tl,cur);				\
   }						\
   else {					\
     prev->next = cur->next;			\
     if (!cur->next)				\
       tl = prev;				\
   }

#define l_ins(x,p)				\
   do {						\
     (p)->next = (x);				\
     (x) = p;					\
   } while (0)

#define l_del(x,p)				\
   do {						\
     (p) = (x);					\
     (x) = (x)->next;				\
   } while (0)

#define l_step(x)				\
     (x) = (x)->next


#define q_step(x)   l_step(x)

#endif /* __QOPS_H__ */
