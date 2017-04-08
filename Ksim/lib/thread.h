/*************************************************************************
 *
 *  Copyright (c) 1997 California Institute of Technology
 *  Department of Computer Science
 *  Pasadena, CA 91125.
 *  All Rights Reserved
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
 *  $Id: thread.h,v 1.1.1.1 2006/05/23 13:53:59 mainakc Exp $
 *
 *************************************************************************/
#ifndef __THREAD_H__
#define __THREAD_H__

#include "contexts.h"
#include "mytime.h"

#ifdef __cplusplus
extern "C" {
#endif

struct process_record {
  context_t c;
  int sz;			/* size of stack */
  int tid;
  char *name;
  char *file;
  int line;
  int exit_code;
  Time_t time;			/* thread local time */
  void *cdata1;			/* this space for rent */
  void *cdata2;			/* more space for rent */
  int color;			/* odd/even queue setup */
  int in_readyq;		/* 1 if in the readyq */
  struct process_record *next;
  double s[(DEFAULT_STACK_SIZE+sizeof(double)-1)/sizeof(double)]; 
         /* stack; this crazy construct is to make the "s" field aligned
	    properly on a sparc
         */
};

typedef struct process_record thread_t;

thread_t *_thread_new (void (*f)(void), int stksz, char *name, int
		       ready, char *file, int line);

#define thread_new(x,y)  _thread_new(x,y,NULL,1,__FILE__,__LINE__)
  /* create new thread */

#define thread_named_new(x,y,nm) _thread_new(x,y,nm,1,__FILE__,__LINE__)
  /* create new thread with a specified name */

void thread_idle (void);
  /* voluntary context switch */

void thread_exit (int);
  /* quit thread */

char *thread_name (void);
  /* returns the name of the current thread if the thread was named,
     otherwise "-unknown-"
  */

void thread_printf (char *msg, ...);
  /* prints [thread id (name) file and line number] before the rest of 
     the message
  */

void thread_print_name (thread_t *);
  /* prints [thread id (name) file and line number] without a newline
   */

void thread_pause (int delay);
  /* suspend current process for some approximate amount of time */

#define thread_self ((thread_t*)current_process)

extern void (*thread_cleanup)(void);


void simulate (void (*f)(void));
  /* call as the last thing from the main process to begin simulation */

  /* thread timing functions */
void Delay (Time_t);
Time_t CurrentTime ();

int thread_id (void);

  /* thread save/restore functions */
void thread_write (FILE *fp, thread_t *t, int save_ctxt);
void thread_read (FILE *fp, thread_t *t, int save_ctxt);  

#ifdef __cplusplus
}
#endif

#endif /* __THREAD_H__ */
