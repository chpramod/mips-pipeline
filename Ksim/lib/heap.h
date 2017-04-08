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
 *  $Id: heap.h,v 1.1.1.1 2006/05/23 13:53:59 mainakc Exp $
 *
 *************************************************************************/
#ifndef __HEAP_H__
#define __HEAP_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned long long heap_key_t;

typedef struct {
  int sz;
  int max;
  void **value;
  heap_key_t *key;
} Heap;

Heap *heap_new (int sz);
void heap_insert (Heap *H, heap_key_t key, void *value);
void *heap_remove_min (Heap *H);
void *heap_remove_min_key (Heap *H, heap_key_t *keyp);
void heap_insert_random (Heap *H, void *value);
void *heap_peek_min (Heap *H);
heap_key_t heap_peek_minkey (Heap *H);

#ifdef __cplusplus
}
#endif

#endif /* __HEAP_H__ */
