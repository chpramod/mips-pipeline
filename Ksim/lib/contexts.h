/*************************************************************************
 *
 *  (c) 1998-1999 Cornell University
 *  Computer Systems Laboratory
 *  Ithaca, NY 14853.
 *  All Rights Reserved
 *
 *  (c) 1996-98 California Institute of Technology
 *  Department of Computer Science
 *  Pasadena, CA 91125.
 *  All Rights Reserved
 *
 *  $Id: contexts.h,v 1.1.1.1 2006/05/23 13:53:59 mainakc Exp $
 *
 *************************************************************************/
#ifndef __CONTEXTS_H__
#define __CONTEXTS_H__

#include <stdlib.h>
#include <setjmp.h>
#include <sys/time.h>
#include <stdio.h>
#if defined(__linux__) && defined(__ia64__)
#include <ucontext.h>
#endif

#ifndef DEFAULT_STACK_SIZE
#ifdef SYNCHRONOUS
#define DEFAULT_STACK_SIZE 0x4000
#else
#define DEFAULT_STACK_SIZE 0x8000
#endif
#define LARGE_STACK_SIZE (0x1000 * 16)
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*
 * A process record. The first field of a process record must be
 * a context.
 *
 * The user must provide:
 *
 *       struct process_record {
 *             context_t c;        // The c.stack field must be initialized
 *             ...
 *       };
 *
 */
typedef struct process_record process_t;


/*
 * A context consists of the state of the process, which is the
 * value of all registers (including pc) and the execution stack.
 *
 * For implementation purposes, we need the start address of the process
 * so that we can handle termination/invocation in a clean manner.
 *
 */
typedef struct {
#if defined(__linux__) && defined(__ia64__)
  ucontext_t uc;		/* IA64 state */
#endif
  jmp_buf buf;			/* state  */
  char *stack;			/* stack  */
  int sz;			/* stack size */
  void (*start) ();		/* entry point */
#ifdef FAIR
  int in_cs;			/* in critical section */
  int pending;			/* interrupt pending */
  int interrupted;              /* have you been interrupted lately? */
#endif
} context_t;


/*
 * At any given instant, "current_process" points to the process record
 * for the currently executing thread of control.
 */
#ifdef __ia64__
extern context_t *current_process;
extern ucontext_t dummy;
extern context_t *old_process;
#else
extern process_t *current_process;
#endif

/*
 * Under the timed scheduling option, the following user-defined routine
 * will be called when the time slice for a particular process is over.
 *
 * This routine is called with interrupts disabled, and should enable
 * interrupts on exit (usually by performing a context switch)
 *
 * (Provided by user)
 */
extern void context_timeout (void);


/*
 * The following routine returns the next process to be executed.
 *
 * (Provided by user)
 */
#ifdef __ia64__
extern context_t *context_select (void);
#else
extern process_t *context_select (void);
#endif

/*
 * Disable/enable timer interrupts.
 */
#ifdef FAIR
extern void context_disable (void);
extern void context_enable (void);
#else
#define context_disable() do { } while (0)
#define context_enable()  do { } while (0)
#endif


/*
 * Perform a context switch. This routine must be called with interrupts
 * disabled. It re-enables interrupts.
 */
#ifdef __ia64__
extern void context_switch (context_t *);
#else
extern void context_switch (process_t *);
#endif

/*
 *
 *  Call once before terminating computation. It must be called with
 *  interrupts disabled.
 *
 */
extern void context_cleanup (void);


/*
 * Initialize the context field of the process record.
 * This procedure must be called with p->c.stack initialized.
 */
#ifdef __ia64__
extern void context_init (context_t* , void (*f)(void));
#else
extern void context_init (process_t* , void (*f)(void));
#endif

/*
 * Unfair scheduling
 */
extern void context_unfair (void);

/*
 * Fair scheduling: can be used to undo the first unfair call
 */
extern void context_fair (void);

/*
 *  Exit
 */
extern void context_exit (void);

/*
 * Set time slice to that of process p: must be called with interrupts
 * disabled
 */
#ifdef __ia64__
extern void context_set_timer (context_t *p);
#else
extern void context_set_timer (process_t *p);
#endif

/*
 * User must provide a function that deletes the process record.
 */
#ifdef __ia64__
extern void context_destroy (context_t *);
#else
extern void context_destroy (process_t *);
#endif

/*
 *  Save context to file. Modifies stack state. NEVER call this function
 *  when p == current_process.
 *  Call only with interrupts disabled.
 */
#ifdef __ia64__
extern void context_write (FILE *fp, context_t *p);
#else
extern void context_write (FILE *fp, process_t *p);
#endif

/*
 *  Read context from file. Modifies stack state. NEVER call this function
 *  when p == current_process.
 *  Call only with interrupts disabled.
 */
#ifdef __ia64__
extern void context_read (FILE *fp, context_t *p);
#else
extern void context_read (FILE *fp, process_t *p);
#endif


#ifdef __ia64__
extern __inline__ unsigned long GetSP (void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __CONTEXTS_H__ */
