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
 *  $Id: prs.h,v 1.1.1.1 2006/05/23 13:53:59 mainakc Exp $
 *
 *************************************************************************/
#ifndef __PRS_H__
#define __PRS_H__

#include "hash.h"
#include "lex.h"
#include "heap.h"
#include "mytime.h"
#include "array.h"

#ifdef __cplusplus
extern "C" {
#endif

struct expr_tree;
typedef struct expr_tree PrsExpr;

struct prs_event;
typedef struct prs_event PrsEvent;

typedef struct prs_node {
  struct prs_node *alias;	/* aliases */
  struct Hashlistelement *e;	/* bucket pointer */
  PrsEvent *queue;		/* non-NULL if on the event queue */
  unsigned int val:2;		/* 0,1,X */
  unsigned int bp:1;		/* breakpoint */
  unsigned int flag:1;		/* marker to avoid double-counting vars */
  unsigned int unstab:1;	/* don't report instability */
  unsigned int exclhi:1;	/* part of an exclhi/excllo directive */
  unsigned int excllo:1;
  unsigned int exq:1;		/* 1 if the queue pointer is not to
				   the normal event queue */
  int delay;			/* after delay on the node */
  int sz, max;
  PrsExpr **out;		/* fanout */
  PrsExpr *up, *dn;		/* pull-up/pull-down */
  unsigned int tc;		/* transition-count */
} PrsNode;

typedef struct excl_ring {
  PrsNode *n;
  struct excl_ring *next;
} PrsExclRing;

typedef struct raw_prs_node {
  struct prs_node *alias;
  struct Hashlistelement *e;	/* bucket pointer */
} RawPrsNode;

struct expr_tree {
  struct expr_tree *l, *r, *u;
  unsigned type:4;
  short val, valx;
};

struct prs_event {
  PrsNode *n;
  int val;
};

enum {
  PRS_AND, PRS_OR, PRS_NOT, PRS_VAR, PRS_NODE_UP, PRS_NODE_DN
};

enum {
  PRS_VAL_T = 0,
  PRS_VAL_F = 1,
  PRS_VAL_X = 2			/* note!!! we rely on these values!!! */
};
/* WARNING: if you change this, need to change propagate_up, and the tables
   just before it */

#define PRS_NO_WEAK_INTERFERENCE  0x1 /* use during reset phase;
					 prevents warnings related to
					 weak interference (but the node
					 becomes X) during the reset
					 phase. */

#define PRS_STOP_SIMULATION       0x2 /* use to stop simulation,
					 eg. by an external interrupt
				      */

#define PRS_ESTIMATE_ENERGY       0x4 /* estimate energy dissipated */

#define PRS_RANDOM_TIMING	  0x8 /* random timings */

typedef struct {
  struct Hashtable *H;		/* prs hash table */
  Heap *eventQueue;		/* event queue */

  A_DECL(PrsExclRing *, exhi);	/* exclusive high ring */
  A_DECL(PrsExclRing *, exlo);	/* exclusive low ring */

  Time_t time;			/* current time */
  unsigned int energy;		/* energy estimate */
  PrsEvent *ev_list;		/* memory allocation--avail list */
  unsigned int flags;		/* simulation control flags */
} Prs;


/*
 * WARNING: these two functions are *not* thread-safe. They should be
 * called before the process is initialized. Typically one could
 * create a process by reading in production rules from a file,
 * attaching channels to the production rules, and then starting up
 * the multi-threaded part of the simulation.
 */
Prs *prs_file (FILE *fp);
Prs *prs_lex (LEX_T *L);
Prs *prs_fopen (char *filename);

/* get node corresponding to node name */
PrsNode *prs_node (Prs *, char *s);

/* set/clear breakpoints */
void prs_set_bp (Prs *, PrsNode *n);
void prs_clr_bp (Prs *, PrsNode *n);

/* return fanout in _preallocated_ array. use the num function below
   to find out the # of elements in the array 

   fanout_rule returns the up/dn expression, whereas fanout returns
   the nodes.
*/
void prs_fanout (Prs *, PrsNode *n, PrsNode **l);
void prs_fanout_rule (Prs *, PrsNode *n, PrsExpr **l);
int prs_num_fanout (PrsNode *n);

/* return fanin in _preallocated_ array. use the num function below
   to find out the # of elements in the array */
void prs_fanin  (Prs *, PrsNode *n, PrsNode **l);
int prs_num_fanin (PrsNode *n);

/* set node to value */
void prs_set_node (Prs *, PrsNode *n, int value);

/* set node to value at specified time */
void prs_set_nodetime (Prs *, PrsNode *n, int value, int time);

/* run until either a breakpoint is reached or no more nodes left
   if there are no more nodes, ret val == NULL; otherwise return value 
   is the last node flipped
*/
PrsNode *prs_cycle (Prs *);

/* fire next event */
PrsNode *prs_step  (Prs *);

/* initialize circuit to all X */
void prs_initialize (Prs *);

extern char __prs_nodechstring[];
#define prs_nodechar(v) __prs_nodechstring[v]

#define prs_nodename(n) ((n)->e->key)

#define prs_nodeval(n)  ((n)->val)

#define prs_reset_time(p)  ((p)->time = 0)


/* free storage */
void prs_free (Prs *p);

/* apply function at each node */
void prs_apply (Prs *p, void *cookie, void (*f)(PrsNode *, void *));

/* dump node to stdout */
void prs_dump_node (PrsNode *n);
void prs_printrule (PrsNode *n);
void prs_print_expr (PrsExpr *n);

#ifdef __cplusplus
}
#endif

#endif /* __PRS_H__ */
