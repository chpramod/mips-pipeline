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
 *  $Id: count.c,v 1.1.1.1 2006/05/23 13:53:59 mainakc Exp $
 *
 *************************************************************************/
#include <stdio.h>
#include "thread.h"
#include "count.h"
#include "qops.h"

/* allocate initialized counter */
count_t *count_new (int init_val)
{
  count_t *c;
  
  context_disable ();
  c = (count_t*)malloc(sizeof(count_t));
  if (!c) {
    printf ("FATAL ERROR: malloc failed, file %s, line %d\n",
	    __FILE__, __LINE__);
    exit (1);
  }
  c->cnt = init_val;
  c->hd = NULL;
  c->tl = NULL;
  
  context_enable ();
  return c;
}

/* free counter */
void _count_free (count_t *c, char *file, int line)
{
  context_disable ();
  if (c->hd) {
    thread_t *t;
    thread_printf ("ERROR: count_free(), file `%s', line %d\n", file, line);
    thread_printf ("\n");
    thread_printf ("Pending processes:\n");
    for (t = c->hd; t; t = t->next) {
      thread_printf ("");
      thread_print_name (t);
      printf ("\n");
    }
  }
  free (c);
  context_enable ();
}

/*
 * wait for count c to exceed value
 */
void count_await (count_t *c, unsigned int val)
{
  context_disable ();
  if (c->cnt < val) {
    current_process->cdata1 = (void*)val;
    q_ins (c->hd, c->tl, current_process);
    context_switch (context_select ());
  }
  else {
    context_enable ();
  }
}

/* increment counter */
void count_increment (count_t *c, unsigned int amount)
{
  extern thread_t *readyQh;
  extern thread_t *readyQt;

  context_disable ();
  c->cnt += amount;
  if (amount > 0 && c->hd) {
    thread_t *t, *prev;
    prev = NULL;
    for (t = c->hd; t; t = t->next) {
      if (((unsigned int)t->cdata1) >= c->cnt) {
	q_ins (readyQh, readyQt, t);
	t->in_readyq = 1;
	if (prev) {
	  prev->next = t->next;
	}
	else {
	  c->hd = t->next;
	}
      }
      else {
	prev = t;
      }
    }
  }
  context_enable ();
}
