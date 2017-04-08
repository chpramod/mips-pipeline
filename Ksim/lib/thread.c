/*************************************************************************
 *
 *  Copyright (c) 1998-99 Cornell University
 *  Computer Systems Laboratory
 *  Cornell University, Ithaca, NY 14853
 *  All Rights Reserved
 *
 *  Copyright (c) 1997-98 California Institute of Technology
 *  Department of Computer Science
 *  Pasadena, CA 91125.
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
 *  $Id: thread.c,v 1.1.1.1 2006/05/23 13:53:59 mainakc Exp $
 *
 *************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "thread.h"
#include "qops.h"

/* the ready queue */
thread_t *readyQh = NULL;
thread_t *readyQt = NULL;

#ifndef THREAD_FAST_ALLOC
#define THREAD_FAST_ALLOC  100	/* a bunch of statically allocated threads */
#endif

static thread_t *thread_freeq = NULL;
static thread_t THREADS[THREAD_FAST_ALLOC];
void (*thread_cleanup)(void) = NULL;

static Time_t inconsistent_timer = 0;

thread_t *timerQh = NULL;
thread_t *timerQt = NULL;

/*------------------------------------------------------------------------
 *
 *   Lazy timer: guaranteed to go off just when some process in the
 *   system exceeds this time when it gets context switched out.
 *
 *------------------------------------------------------------------------
 */
static void thread_insert_timer (thread_t *t, Time_t tm)
{
  thread_t *x;

  if (t->time >= tm) return;

  t->time = tm;

  for (x = timerQh; x; x = x->next) {
    if (x->time > tm)
      break;
  }
  if (!x) {
    if (timerQh) {
      timerQt->next = t;
    }
    else {
      timerQh = t;
    }
    timerQt = t;
  }
  else {
    t->next = x->next;
    x->next = t;
    if (!t->next)
      timerQt = t;
  }
}

void thread_pause (int delay)
{
  context_disable ();
  if (current_process) {
    thread_insert_timer (current_process, time_add (delay,
						    current_process->time));
  }
  context_switch (context_select ());
}

/*------------------------------------------------------------------------
 *
 * Interface to context library
 *
 *------------------------------------------------------------------------
 */

/* pre: interrupts disabled */
void thread_print_name (thread_t *t)
{
#ifdef DEBUG_MODE
  if (!t) t = current_process;

  if (t) {
    if (t->file) {
      if (t->name)
	printf ("[Thread %2d (%s), initiated in file `%s', line %d] ",
		t->tid, t->name, t->file, t->line);
      else
	printf ("[Thread %2d, initiated in file `%s', line %d] ",
		t->tid, t->file, t->line);
    }
    else {
      if (t->name)
	printf ("[Thread %2d (%s)] ", t->tid, t->name);
      else
	printf ("[Thread %2d] ", t->tid);
    }
  }
  else {
    printf ("[Main thread] ");
  }
#else
  if (t)
    printf ("[Thread %2d] ", t->tid);  
  else
    printf ("[Main thread] ");
#endif
}

/* print message */
/* pre: interrupts disabled */
void thread_printf (char *msg, ...)
{
  va_list ap;
  
  thread_print_name (NULL);
  va_start (ap, msg);
  vfprintf (stdout, msg, ap);
  va_end (ap);
}

/* pre: interrupts disabled */
void context_destroy (thread_t *t)
{
#ifdef DEBUG_MODE
#ifdef DEBUG_MODE2
  thread_print_name (t);
  printf ("EXIT CODE: %d\n", t->exit_code);
#endif
  if (t->name) free (t->name);
  if (t->file) free (t->file);
#endif /* DEBUG_MODE */
  if (t->sz == DEFAULT_STACK_SIZE) {
    t->next = thread_freeq;
    thread_freeq = t;
  }
  else
    free (t);
}


/* pre: interrupts disabled */


/* pre: interrupts disabled */
void context_timeout (void)
{
  process_t *p;

  if (current_process != NULL) {
    current_process->in_readyq = 1;
    q_ins (readyQh, readyQt, current_process);
    p = context_select ();
    context_switch (p);
  }
  else
    context_enable();
}

thread_t* context_select (void)
{
  thread_t *t, *x;

#ifdef CLASS_HACKERY_NONDET
  if (!readyQh) {
    ch_update_readyQ ();
  }
#endif
  q_del (readyQh, readyQt, t);
  if (!t) {
    context_cleanup ();
    if (thread_cleanup) (*thread_cleanup)();
#ifdef DEBUG_MODE2
    printf("\nNo more processes.\n");
#endif /* DEBUG_MODE2 */
    exit (0);
  }
  t->in_readyq = 0;
  inconsistent_timer = time_max (inconsistent_timer, t->time);
  while (timerQh && inconsistent_timer >= timerQh->time) {
    q_del (timerQh, timerQl, x);
    x->in_readyq = 1;
    q_ins (readyQh, readyQt, x);
  }
  return t;
}


/*------------------------------------------------------------------------
 *
 *  _thread_new --
 *
 *     Create a new thread with the specified stack size
 *
 *------------------------------------------------------------------------
 */
thread_t *
_thread_new (void (*f)(void), int stksz, char *name, int ready, 
	     char *file, int line)
{
  static int tid = 0;
  thread_t *t;

  context_disable ();
  if (tid == 0) {
    int i;
    for (i=0; i < THREAD_FAST_ALLOC; i++) {
      THREADS[i].next = thread_freeq;
      thread_freeq = THREADS+i;
    }
  }
  if (stksz == 0 || stksz == DEFAULT_STACK_SIZE) {
    stksz = DEFAULT_STACK_SIZE;
    if (thread_freeq) {
      t = thread_freeq;
      thread_freeq = thread_freeq->next;
    }
    else
      t = (thread_t*)malloc(sizeof(thread_t));
  }
  else
    t = (thread_t*)malloc(sizeof(thread_t)-DEFAULT_STACK_SIZE+stksz);
  if (!t) {
    printf ("Thread allocation failed, stack size=%d\n",stksz);
    exit (1);
  }
  t->sz = stksz;
  t->c.stack = (char*) t->s;
  t->c.sz = stksz;
  t->tid = tid++;
  t->line = line;
  t->exit_code = 0;
  t->in_readyq = 0;
  time_init (t->time);

#ifdef DEBUG_MODE
  if (file) {
    t->file = (char*)malloc(1+strlen(file));
    if (!t->file) {
      printf ("MALLOC failed\n");
      exit (1);
    }
    strcpy (t->file, file);
  }
  else {
    t->file = NULL;
  }
  if (name) {
    t->name = (char*)malloc(1+strlen(name));
    if (!t->name) {
      printf ("MALLOC failed\n");
      exit (1);
    }
    strcpy (t->name, name);
  }
  else
    t->name = NULL;
#else   /* !DEBUG_MODE */
  t->name = NULL;
#endif  /* DEBUG_MODE */

  context_init (t, f);
  if (ready) {
    t->in_readyq = 1;
    q_ins (readyQh, readyQt, t);
  }
  context_enable ();
  return t;
}


/* quit thread */
void thread_exit (int code)
{
  context_disable ();
  if (current_process)
    current_process->exit_code = code;
  context_exit ();
}

/* voluntary context switch */
void thread_idle (void)
{
  context_disable ();
  context_timeout ();
}

/* return name of the thread */
char *thread_name (void)
{
  return ((current_process) ? ((current_process)->name ? (current_process)->name : "-unknown-") : "-Main-thread-");
}

/* simulate function */
void simulate (void (*f)(void))
{
  thread_cleanup = f;
  thread_exit (0);
}

/*
 *
 *  Time stuff
 *
 */
void Delay (Time_t d)
{
  thread_t *t;

  t = (thread_t*)current_process;
  time_inc (t->time, d);
}

Time_t CurrentTime (void)
{
  thread_t *t;

  t = (thread_t*)current_process;
  if (!t) return 0;
  return t->time;
}

int thread_id (void)
{
  thread_t *t = (thread_t*)current_process;
  return t->tid;
}

/*------------------------------------------------------------------------
 *
 *  Save/restore thread state
 *
 *------------------------------------------------------------------------
 */
void thread_write (FILE *fp, thread_t *t, int save_ctxt) { }
void thread_read (FILE *fp, thread_t *t, int save_ctxt) { }
