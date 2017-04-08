/**************************************************************************
 *
 *  Copyright (c) 1999-2000 Cornell University
 *  Computer Systems Laboratory
 *  Ithaca, NY 14853
 *  All Rights Reserved
 *
 *  $Id: prs.c,v 1.1.1.1 2006/05/23 13:53:59 mainakc Exp $
 *
 *************************************************************************/
#include <string.h>
#include <math.h>
#include "prs.h"
#include "misc.h"
#include "contexts.h"
#include "heap.h"

/* used for printing purposes */
char __prs_nodechstring[] = { '1', '0', 'X' };

/* forward declarations */
static void propagate_up (Prs *p, PrsExpr *e, int prev, int val);
static void parse_file (LEX_T *l, Prs *p);
static void init_tables (void);
static void merge_or_up (PrsNode *n, PrsExpr *e);
static void merge_or_dn (PrsNode *n, PrsExpr *e);
static void mk_out_link (PrsExpr *e, PrsNode *n);
static PrsNode *raw_lookup (char *s, struct Hashtable *H);
static void canonicalize_hashtable (Prs *p);
static void canonicalize_excllist (Prs *p);

/*
 *  Token definitions
 */
static int TOK_AND, TOK_OR, TOK_NOT, TOK_LPAR, TOK_RPAR,
  TOK_ID, TOK_ARROW, TOK_UP, TOK_DN, TOK_EQUAL, TOK_COMMA;

/* 
 *  Access node data structure stored in expr
 */
#define NODE(expr) ((PrsNode *)(((struct Hashlistelement *)(expr)->l)->value.p))

static void random_init (void);

/*
 *
 *   Read prs from file "fp", return data structure
 *
 */
Prs *prs_file (FILE *fp)
{
  Prs *p;
  LEX_T *l;

  random_init ();
  l = lex_file (fp);
  p = prs_lex (l);
  lex_free (l);

  return p;
}

/*
 *
 *   Read prs from file "s"
 *
 */
Prs *prs_fopen (char *s)
{
  Prs *p;
  LEX_T *l;

  random_init ();
  l = lex_fopen (s);
  p = prs_lex (l);
  lex_free (l);

  return p;
}

/*
 *  1 if it is an identifier token, 0 otherwise
 */
static int lex_is_id (LEX_T *l)
{
  if (lex_sym (l) == l_id || lex_sym (l) == l_string)
    return 1;
  else 
    return 0;
}

/*
 *  Return pointer to string (which will change if you read in the next 
 *  token) corresponding to the identifier
 */
static char *lex_mustbe_id (LEX_T *l)
{
  if (!lex_is_id (l)) {
    fatal_error ("Expecting `id'\n\t%s", lex_errstring (l));
  }
  if (lex_sym (l) == l_string) {
    lex_getsym (l);
    lex_prev(l)[strlen(lex_prev(l))-1] = '\0';
    return lex_prev(l)+1;
  }
  else {
    lex_getsym (l);
    return lex_prev(l);
  }
}

/*
 *  Parse prs file and return simulation data structure
 */
Prs *prs_lex (LEX_T *L)
{
  Prs *p;
  int numtoks = 0;

  init_tables ();

#ifdef __addtok
#error naming conflict in macro
#endif
#define __addtok(name,string)  do { numtoks+=!lex_istoken(L,string); name=lex_addtoken(L,string); } while(0)

  __addtok (TOK_AND, "&");
  __addtok (TOK_OR, "|");
  __addtok (TOK_NOT, "~");
  __addtok (TOK_LPAR, "(");
  __addtok (TOK_RPAR, ")");
  __addtok (TOK_ARROW, "->");
  __addtok (TOK_UP, "+");
  __addtok (TOK_DN, "-");
  __addtok (TOK_EQUAL, "=");
  __addtok (TOK_COMMA, ",");

  TOK_ID = l_id;

#undef __addtok

  NEW (p, Prs);
  p->H = createHashtable (128);
  p->eventQueue = heap_new (128);
  p->time = 0;
  p->ev_list = NULL;
  p->energy = 0;
  A_INIT (p->exhi);
  A_INIT (p->exlo);

  lex_getsym (L);

  parse_file (L,p);

  lex_deltokens (L, numtoks);

  canonicalize_hashtable (p);
  canonicalize_excllist (p);

  return p;
}


/*
 *  Return node pointer corresponding to name "s"
 */
PrsNode *prs_node (Prs *p, char *s)
{
  return raw_lookup (s, p->H);
}


/*
 *  Set a breakpoint on node n
 */
void prs_set_bp (Prs *p, PrsNode *n)
{
  n->bp = 1;
}

/*
 *  Clear breakpoint on node n
 */
void prs_clr_bp (Prs *p, PrsNode *n)
{
  n->bp = 0;
}


/* 
 *  Return fanout of node in array "l" (preallocated!)
 */
void prs_fanout (Prs *p, PrsNode *n, PrsNode **l)
{
  int i;
  PrsExpr *e;

  for (i=0; i < n->sz; i++) {
    e = n->out[i];
    while (e && e->type != PRS_NODE_UP && e->type != PRS_NODE_DN)
      e = e->u;
    Assert (e, "You've *got* to be kidding");
    l[i] = NODE(e);
  }
}

/* 
 *  Return fanout of node in array "l" (preallocated!)
 */
void prs_fanout_rule (Prs *p, PrsNode *n, PrsExpr **l)
{
  int i;
  PrsExpr *e;

  for (i=0; i < n->sz; i++) {
    e = n->out[i];
    while (e && e->type != PRS_NODE_UP && e->type != PRS_NODE_DN)
      e = e->u;
    Assert (e, "You've *got* to be kidding");
    l[i] = e;
  }
}

int prs_num_fanout (PrsNode *n)
{
  return n->sz;
}


/* 
 *  Return fanin of node in array "l", with length "*len"
 *
 */
static void apply (PrsExpr *e, void (*f)(PrsExpr *, void *), void *cookie)
{
  if (!e) return;

  (*f)(e, cookie);

  switch (e->type) {
  case PRS_AND:
  case PRS_OR:
  case PRS_NOT:
    for (e = e->l; e; e = e->r)
      apply (e, f, cookie);
    break;
  case PRS_NODE_UP:
  case PRS_NODE_DN:
    apply (e->r, f, cookie);
    break;
  case PRS_VAR:
    break;
  default:
    Assert (0, "Corrupt data structure. Insert $$$ to continue.");
    break;
  }
}

static void countvars (PrsExpr *e, int *x)
{
  if (e->type == PRS_VAR && NODE(e)->flag == 0) {
    NODE(e)->flag = 1;
    (*x)++;
  }
}

static void clrflags (PrsExpr *e, void *x)
{
  if (e->type == PRS_VAR)
    NODE(e)->flag = 0;
}

static void add_to_varlist (PrsExpr *e, PrsNode ***l)
{
  if (e->type == PRS_VAR && NODE(e)->flag == 0) {
    NODE(e)->flag = 1;
    *(*l) = (PrsNode *)e->l;
    (*l)++;
  }
}

typedef void (*APPLYFN) (PrsExpr *, void *);

void prs_fanin  (Prs *p, PrsNode *n, PrsNode **l)
{
  apply (n->up, (APPLYFN) add_to_varlist, &l);
  apply (n->dn, (APPLYFN) add_to_varlist, &l);
  apply (n->up, clrflags, NULL);
  apply (n->dn, clrflags, NULL);
}


int prs_num_fanin (PrsNode *n)
{
  int num = 0;

  apply (n->up, (APPLYFN) countvars, &num);
  apply (n->dn, (APPLYFN) countvars, &num);
  apply (n->up, clrflags, NULL);
  apply (n->dn, clrflags, NULL);

  return num;
}

/*
 *
 *  Memory allocation/deallocation functions
 *
 *  WARNING: the newnode()/delnode(), newrawnode(), newexpr()
 *  functions are *NOT* thread-safe. They should only be called by the
 *  parsing functions (which are not thread-safe either).
 *
 */
#ifndef UNIT_MALLOC
static PrsNode *node_list = NULL;
#endif

static PrsNode *newnode (void)
{
  PrsNode *n;
  int i;

#ifdef UNIT_MALLOC

  NEW (n, PrsNode);

#else
  if (!node_list) {
    MALLOC (node_list, PrsNode, 1024);
    for (i=0; i < 1023; i++) 
      node_list[i].alias = node_list+i+1;
    node_list[i].alias = NULL;
  }
  n = node_list;
  node_list = node_list->alias;
#endif

  n->queue = 0;
  n->bp = 0;
  n->val = PRS_VAL_X;
  n->sz = 0;
  n->max = 0;
  n->out = NULL;
  n->alias = NULL;
  n->up = NULL;
  n->dn = NULL;
  n->unstab = 0;
  n->exclhi = 0;
  n->excllo = 0;
  n->exq = 0;
  n->tc = 0;

  return n;
}

static void delnode (PrsNode *n)
{
#if UNIT_MALLOC
  FREE (n);
#else
    n->alias = node_list;
    node_list = n;
#endif
}

static RawPrsNode *newrawnode (void)
{
  static int max_nodes = 0;
  static int num_nodes = 0;
  static RawPrsNode *n = NULL;
  RawPrsNode *p;

  if (max_nodes == num_nodes) {
    max_nodes = 1024;
    num_nodes = 0;
    MALLOC (n, RawPrsNode, max_nodes);
  }
  p = &n[num_nodes++];
  p->alias = NULL;
  p->e = NULL;
  return p;
}
 
static PrsExpr *expr_list = NULL;

static PrsExpr *newexpr (void)
{
  PrsExpr *e;
  int i;

  if (!expr_list) {
    MALLOC (expr_list, PrsExpr, 1024);
    for (i=0; i < 1023; i++)
      expr_list[i].r = expr_list+i+1;
    expr_list[i].r = NULL;
  }
  e = expr_list;
  expr_list = expr_list->r;

  e->l = NULL;
  e->r = NULL;
  e->u = NULL;
  e->type = 0;
  e->val = 0;
  e->valx = 0;

  return e;
}
 
static void delexpr (PrsExpr *e)
{
  e->r = expr_list;
  expr_list = e;
}

/*
 *  Allocate a new event (thread-safe, unlike other allocs :) )
 *
 */
static PrsEvent *newevent (Prs *p, PrsNode *n, int val)
{
  PrsEvent *e;
  int i;

  if (!p->ev_list) {
    context_disable ();
    MALLOC (p->ev_list, PrsEvent, 1024);
    context_enable ();
    for (i=0; i < 1023; i++) {
      p->ev_list[i].n = (PrsNode *)(p->ev_list+i+1);
    }
    p->ev_list[i].n = NULL;
  }
  e = p->ev_list;
  p->ev_list = (PrsEvent *)p->ev_list->n;

  Assert (n->queue == 0, "newevent() on a node already on the event queue");

  e->n = n;
  e->val = val;
  n->queue = e;
  n->exq = 0;
  return e;
}

static void deleteevent (Prs *p, PrsEvent *e)
{
  e->n = (PrsNode *)p->ev_list;
  p->ev_list = e;
}


/*
 *  Execute "set" function: puts (n:=value) on the pending event queue.
 */
void prs_set_node (Prs *p, PrsNode *n, int value)
{
  prs_set_nodetime (p, n, value, p->time);
}


/*
 *  Put (n := value) on queue @ time t
 */
void prs_set_nodetime (Prs *p, PrsNode *n, int value, int time)
{
  PrsEvent *pe;

  if (n->queue && value == n->val) {
    printf ("WARNING: ignoring set_node on `%s' [interferes with pending event]\n", n->e->key);
    return;
  }
  if (value == n->val) return;

  if (n->queue) {
    printf ("WARNING: pending value for node `%s'; ignoring request\n", n->e->key);
    return;
  }

  pe = newevent (p, n, value);
  context_disable ();
  heap_insert (p->eventQueue, time, pe);
  context_enable ();
}



/*
 *  Run until either a breakpoint is encountered, or no more events
 *  are pending.
 *
 *  Return value: NULL => no more events; non-NULL => bp on that node
 */
PrsNode *prs_cycle (Prs *p)
{
  PrsNode *n;

  while (n = prs_step (p)) {
    if (n->bp || (p->flags & PRS_STOP_SIMULATION))
      break;
  }
  return n;
}

L_A_DECL(PrsEvent *, pendingQ);

typedef struct prs_eventlist {
  struct prs_event *p;
  Time_t t;
} PrsEventArray;

L_A_DECL(PrsEventArray,prsexclhi);
L_A_DECL(PrsEventArray,prsexcllo);

static void insert_exclhiQ (PrsEvent *pe, Time_t t)
{
  A_NEW (prsexclhi, PrsEventArray);
  A_NEXT (prsexclhi).p = pe;
  A_NEXT (prsexclhi).t = t;
  A_INC (prsexclhi);
  pe->n->exq = 1;
}

static void insert_exclloQ (PrsEvent *pe, Time_t t)
{
  A_NEW (prsexcllo, PrsEventArray);
  A_NEXT (prsexcllo).p = pe;
  A_NEXT (prsexcllo).t = t;
  A_INC (prsexcllo);
  pe->n->exq = 1;
}

static void insert_pendingQ (PrsEvent *pe)
{
  A_NEW (pendingQ, PrsEvent *);
  A_NEXT (pendingQ) = pe;
  A_INC (pendingQ);
}

	
#ifdef OLD
static int pending_weak[3][3] = { 
  { 0, 0, 1 },
  { 0, 0, 1 },
  { 1, 1, 0 }
};
#endif
static int pending_weak[3][3] = { 
  { 0, 0, 1 },
  { 0, 0, 0 },
  { 1, 0, 1 }
};

#define NEWTIME(p,ev)					\
  time_add (p->time,					\
	    (ev)->n->delay ?				\
	       (p->flags & PRS_RANDOM_TIMING ?		\
		   random_number() : (ev)->n->delay	\
		)					\
            : ((p->time) > 0 ? -1 : 0)			\
	    )

extern double drand48(void);

#define MAX_VALUE (1<<16)	/* maximum value of random numbers */
static double LN_MAX_VAL;

/*------------------------------------------------------------------------
 *
 *  Initialize random number generator
 *
 *------------------------------------------------------------------------
 */
static
void random_init (void)
{
  LN_MAX_VAL = log(MAX_VALUE);
}

/*------------------------------------------------------------------------
 *
 * Random number generator, distribution 1/(1+x) from 0..MAX_VALUE so that
 * the std. dev of the distribution is large  (approx MAX_VALUE/sqrt(2))
 * compared to the mean ( O (MAX_VALUE/ln(MAX_VALUE+1)) )
 *
 *------------------------------------------------------------------------
 */
static unsigned int 
random_number (void)
{
  double d;
  unsigned int val;

  d = drand48();
  val = exp(d*LN_MAX_VAL)-1;
  return val;
}
				 
/*
 *  Execute next event. Returns node corresponding to the event.
 */
PrsNode *prs_step  (Prs *p)
{
  PrsEventArray *ea;
  PrsExclRing *er;
  PrsEvent *pe, *ne;
  PrsNode *n;
  int i, j;
  int prev, flag;
  heap_key_t t;

  if (pe = (PrsEvent *) heap_remove_min_key (p->eventQueue,
					     (heap_key_t*)&p->time)) {
    n = pe->n;
    n->queue = NULL;
    if (pe->val == PRS_VAL_X && n->val == PRS_VAL_X) return n;
    context_disable ();

    Assert (n->unstab || n->val != pe->val, 
	    "Vacuous firings on the event queue");
#if 0
    printf ("%s: %c\n", prs_nodename (n), prs_nodechar (pe->val));
#endif
    prev = n->val;
    n->val = pe->val;
    deleteevent (p, pe);
    for (i=0; i < n->sz; i++)
      propagate_up (p, n->out[i], prev, n->val);

    if (n->exclhi && n->val == PRS_VAL_F) {
      /* 
	 if n->exclhi and n is now low
            check if any of the nodes in n's exclhi rings
	    are enabled; if so, insert them into the exclQ.
      */
      for (j=0; j < A_LEN(p->exhi); j++) {
	er = p->exhi[j];
	flag = 0; /* flag for matches */
	do {
	  if (er->n == n) {
	    flag = 1;
	    break;
	  }
	  er = er->next;
	} while (er != p->exhi[j]);
	if (flag) {
	  er = p->exhi[j];
	  do {
	    if (!er->n->queue && er->n->up && er->n->up->val ==	PRS_VAL_T) {
	      ne = newevent (p, er->n, PRS_VAL_T);
	      insert_exclhiQ (ne, NEWTIME (p, ne));
	    }
	    er = er->next;
	  } while (er != p->exhi[j]);
	}
      }
    }
    if (n->excllo && n->val == PRS_VAL_T) {
      /* 
	 dual stuff
      */
      for (j=0; j < A_LEN(p->exlo); j++) {
	er = p->exlo[j];
	flag = 0; /* flag for matches */
	do {
	  if (er->n == n) {
	    flag = 1;
	    break;
	  }
	  er = er->next;
	} while (er != p->exlo[j]);
	if (flag) {
	  er = p->exlo[j];
	  do {
	    if (!er->n->queue && er->n->dn && er->n->dn->val ==	PRS_VAL_T) {
	      ne = newevent (p, er->n, PRS_VAL_F);
	      insert_exclloQ (ne, NEWTIME (p, ne));
	    }
	    er = er->next;
	  } while (er != p->exlo[j]);
	}
      }
    }


    /* energy estimate: C approximately the fanout of the node */
    if (p->flags & PRS_ESTIMATE_ENERGY)
      p->energy += n->sz;
    n->tc++;
#if 0
    dump_prs (p);
#endif
    for (i=0; i < A_LEN(pendingQ); i++) {
      ne = pendingQ[i];
      if (ne->n->up->val != PRS_VAL_F && ne->n->dn->val != PRS_VAL_F) {
	  /* there is interference. if there is weak interference,
	     don't report it unless we're supposed to.

	     weak = (X & T) or (T & X)
	  */
	if (!(p->flags & PRS_NO_WEAK_INTERFERENCE) ||
	    !pending_weak[ne->n->up->val][ne->n->dn->val]) {
	  printf ("WARNING: %sinterference `%s'\n",
	       pending_weak[ne->n->up->val][ne->n->dn->val]?
		  "weak-" : "", prs_nodename (ne->n));
	}
	/* turn node to "X" */
	ne->val = PRS_VAL_X;
	if (ne->n->val != PRS_VAL_X) {
	  heap_insert (p->eventQueue, NEWTIME (p, ne), ne);
	}
	else {
	  ne->n->queue = NULL;
	  ne->n->exq = 0;
	  deleteevent (p, ne);
	}
      }
      else {
	/* insert pending event into event heap */
	if (ne->n->val != ne->val) {
	  heap_insert (p->eventQueue, NEWTIME (p, ne), ne);
	}
	else {
	  ne->n->queue = NULL;
	  ne->n->exq = 0;
	  deleteevent (p, ne);
	}
      }
    }
    A_LEN(pendingQ) = 0;

    /* process created excl events
	 - put the event onto the heap if it does not violate
	   any existing excl hi/lo directive

	 - drop the event otherwise!
    */
    for (i=0; i < A_LEN(prsexclhi); i++) {
      ea = &prsexclhi[i];
      /* look through the events. if any of them have a pending
	 queue entry, then we're done
      */
      for (j=0; j < A_LEN(p->exhi); j++) {
	er = p->exhi[j];

#define VALMATCH(v) \
	(er->n->val==(v)||((er->n->queue&&er->n->queue->val==(v) && !er->n->exq)))

	prev = 0; /* count # of true/pending true nodes */
	flag = 0; /* flag for matches */
	do {
	  if (VALMATCH(PRS_VAL_T)) prev++;
	  if (er->n == ea->p->n)  flag = 1;
	  er = er->next;
	} while (er != p->exhi[j]);
	if (flag) {
	  if (!prev) {
	    /* insert event into real queue */
	    heap_insert (p->eventQueue, ea->t, ea->p);
	    ea->p->n->exq = 0;
	  }
	}
      }
      if (ea->p->n->exq) {
	ea->p->n->queue = NULL;
	ea->p->n->exq = 0;
	deleteevent (p, ea->p);
      }
    }
    A_LEN(prsexclhi) = 0;

    for (i=0; i < A_LEN(prsexcllo); i++) {
      ea = &prsexcllo[i];
      for (j=0; j < A_LEN(p->exlo); j++) {
	er = p->exlo[j];

	prev = 0; /* count # of true/pending true nodes */
	flag = 0; /* flag for matches */
	do {
	  if (VALMATCH(PRS_VAL_F)) prev++;
	  if (er->n == ea->p->n)  flag = 1;
	  er = er->next;
	} while (er != p->exlo[j]);
	if (flag) {
	  if (!prev) {
	    /* insert event into real queue */
	    heap_insert (p->eventQueue, ea->t, ea->p);
	    ea->p->n->exq = 0;
	  }
	}
      }
      if (ea->p->n->exq) {
	ea->p->n->queue = NULL;
	ea->p->n->exq = 0;
	deleteevent (p, ea->p);
      }
    }
    A_LEN(prsexcllo) = 0;

    context_enable ();
  }
  return pe == NULL ? NULL : n;
}


static struct event_update {
  unsigned vacuous:1;		/* 1 if vacuous firing */
  unsigned unstab:1;		/* 1 if unstable */
  unsigned interf:1;		/* 1 if interference */
  unsigned weak:1;		/* 1 if weak-inter/weak-unstab */
} prs_upguard[3][3], prs_dnguard[3][3];
    
static int not_table[3];

/* first index: guard state; second index: pending event state */

#define SET(x,a,b,c,e) do { x.vacuous=a; x.unstab=b; x.interf=c; x.weak=e; } while(0)

static void init_tables (void)
{
  /* UP-rules */

  /* guard = T, event = T or X: vacuous firing */
  SET(prs_upguard[PRS_VAL_T][PRS_VAL_T],1,0,0,0);
  SET(prs_upguard[PRS_VAL_T][PRS_VAL_X],1,0,0,0);
  /* guard = T, event = F: inteference */
  SET(prs_upguard[PRS_VAL_T][PRS_VAL_F],0,0,1,0);

  /* guard = F, event = T: unstable node */
  SET(prs_upguard[PRS_VAL_F][PRS_VAL_T],0,1,0,0);
  /* guard = F, event = X: vacuous */
  SET(prs_upguard[PRS_VAL_F][PRS_VAL_X],1,0,0,0);
  SET(prs_upguard[PRS_VAL_F][PRS_VAL_F],1,0,0,0);
  
  /* guard = X, event = T: weak unstab */
  SET(prs_upguard[PRS_VAL_X][PRS_VAL_T],0,1,0,1);
  /* guard = X, event = F: weak interf */
  SET(prs_upguard[PRS_VAL_X][PRS_VAL_T],0,0,1,1);
  /* guard = X, event = X: nothing */
  SET(prs_upguard[PRS_VAL_X][PRS_VAL_X],1,0,0,0);

  /* DN-rules */

  /* guard = T, event = F or X: vacuous firing */
  SET(prs_dnguard[PRS_VAL_T][PRS_VAL_F],1,0,0,0);
  SET(prs_dnguard[PRS_VAL_T][PRS_VAL_X],1,0,0,0);
  /* guard = T, event = F: inteference */
  SET(prs_dnguard[PRS_VAL_T][PRS_VAL_T],0,0,1,0);

  /* guard = F, event = F: unstable node */
  SET(prs_dnguard[PRS_VAL_F][PRS_VAL_F],0,1,0,0);
  /* guard = F, event = F/X: vacuous */
  SET(prs_dnguard[PRS_VAL_F][PRS_VAL_X],1,0,0,0);
  SET(prs_dnguard[PRS_VAL_F][PRS_VAL_T],1,0,0,0);
  
  /* guard = F, event = T: weak interf */
  SET(prs_dnguard[PRS_VAL_X][PRS_VAL_T],0,0,1,1);
  /* guard = F, event = F: weak unstab */
  SET(prs_dnguard[PRS_VAL_X][PRS_VAL_T],0,1,0,1);
  /* guard = X, event = X: nothing */
  SET(prs_dnguard[PRS_VAL_X][PRS_VAL_X],1,0,0,0);

  not_table[PRS_VAL_T] = PRS_VAL_F;
  not_table[PRS_VAL_F] = PRS_VAL_T;
  not_table[PRS_VAL_X] = PRS_VAL_X;
}
#undef SET

/*
 *  main expression evaluation engine.
 */
static void propagate_up (Prs *p, PrsExpr *e, int prev, int val)
{
  PrsNode *n;
  PrsEvent *pe;
  struct event_update *eu;

  register int old_val, new_val;
  register PrsExpr *u;

 start:
  u = e->u;
  Assert (e, "propagate_up: NULL argument expression");
  Assert (u, "propagate_up: no up-link for expression");

  switch (u->type) {
  case PRS_AND:
    old_val = (u->val > 0) ? PRS_VAL_F : ((u->valx == 0) ? PRS_VAL_T : 
					  PRS_VAL_X);
    /* remove old value information */

    /* 1. reduce X count */
    u->valx -= (prev >> 1);
    /* 2. for "AND", we count the number of zeros. If the # of zeros ==
       0, the node is a 1 */
    u->val -= (prev&1);

    /* add new value information */
    u->valx += (val >> 1);
    u->val += (val&1);
    
    new_val = (u->val > 0) ? PRS_VAL_F : ((u->valx == 0) ? PRS_VAL_T : 
					  PRS_VAL_X);
    if (old_val != new_val) {
      prev = old_val;
      val = new_val;
      e = u;
      goto start;
    }
    break;

  case PRS_OR:
    old_val = (u->val > 0) ? PRS_VAL_T : ((u->valx == 0) ? PRS_VAL_F : 
					  PRS_VAL_X);
    
    /* remove old value information */

    /* 1. reduce X count */
    u->valx -= (prev >> 1);
    /* 2. for "OR", we count the number of 1's */
    u->val -= !prev;
    /*u->val -= (1^(prev&1)^(prev>>1));*/

    /* add new value information */
    u->valx += (val >> 1);
    u->val += !val;
    /*u->val += (1^(val&1)^(val>>1));*/
    
    new_val = (u->val > 0) ? PRS_VAL_T : ((u->valx == 0) ? PRS_VAL_F : 
					  PRS_VAL_X);
    if (old_val != new_val) {
      prev = old_val;
      val = new_val;
      e = u;
      goto start;
    }
    break;
  case PRS_NOT:
    /* for NOT, we simply store the value of the inverted node */
    old_val = u->val;
    u->val = not_table[val];
    if (old_val != u->val) {
      prev = old_val;
      val = u->val;
      e = u;
      goto start;
    }
    break;
  case PRS_NODE_UP:
    u->val = val;
    n = NODE (u);
    if (!n->queue) {
      if (val != PRS_VAL_F && n->val != val) {
	pe = newevent (p, n, val);
	if (n->exclhi) {
	  insert_exclhiQ (pe, NEWTIME (p, pe));
	}
	else {
	  if (n->dn) 
	    insert_pendingQ (pe);
	  else
	    heap_insert (p->eventQueue, NEWTIME (p, pe), pe);
	}
      }
      /* is this right??
	  - n->dn might not have been updated yet!
      */
      else if (val == PRS_VAL_F && n->dn && n->dn->val == PRS_VAL_T) {
	pe = newevent (p, n, PRS_VAL_F);
	if (n->excllo) {
	  insert_exclloQ (pe, NEWTIME (p, pe));
	}
	else {
	  insert_pendingQ (pe);
	}
      }
    }
    else if (!n->exq) {
      if (val == PRS_VAL_F && n->dn && n->dn->val == PRS_VAL_T &&
	  n->queue->val == PRS_VAL_X && n->val != PRS_VAL_F) {
	/* there is a pending "X" in the queue */
	n->queue->val = PRS_VAL_F;
	break;
      }
      eu = &prs_upguard[val][n->queue->val];
      if (!eu->vacuous) {
	if (eu->unstab && !n->unstab || eu->interf) {
	  n->queue->val = PRS_VAL_X;
	  if (eu->interf)
	    printf ("WARNING: %sinterference `%s'+\n",
		    eu->weak ? "weak-" : "", prs_nodename (n));
	  if (eu->unstab && !n->unstab) 
	    printf ("WARNING: %sunstable `%s'+\n",
		    eu->weak ? "weak-" : "", prs_nodename (n));
	}
      }
    }
    break;
  case PRS_NODE_DN:
    u->val = val;
    n = NODE (u);
    if (!n->queue) {
      if (val != PRS_VAL_F && n->val != not_table[val]) {
	pe = newevent (p, n, not_table[val]);
	if (n->excllo) {
	  insert_exclloQ (pe, NEWTIME (p, pe));
	}
	else {
	  if (n->up) 
	    insert_pendingQ (pe);
	  else 
	    heap_insert (p->eventQueue, NEWTIME (p, pe), pe);
	}
      }
      else if (val == PRS_VAL_F && n->up && n->up->val == PRS_VAL_T) {
	pe = newevent (p, n, PRS_VAL_T);
	if (n->exclhi) {
	  insert_exclhiQ (pe, NEWTIME (p, pe));
	}
	else {
	  insert_pendingQ (pe);
	}
      }
    }
    else if (!n->exq) {
      if (val == PRS_VAL_F && n->up && n->up->val == PRS_VAL_T &&
	  n->queue->val == PRS_VAL_X && n->val != PRS_VAL_T) {
	/* there is a pending "X" in the queue */
	n->queue->val = PRS_VAL_T;
	break;
      }
      eu = &prs_dnguard[val][n->queue->val];
      if (!eu->vacuous) {
	if (eu->unstab && !n->unstab || eu->interf) {
	  n->queue->val = PRS_VAL_X;
	  if (eu->interf)
	    printf ("WARNING: %sinterference `%s'-\n",
		    eu->weak ? "weak-" : "", prs_nodename (n));
	  if (eu->unstab && !n->unstab) 
	    printf ("WARNING: %sunstable `%s'-\n",
		    eu->weak ? "weak-" : "", prs_nodename (n));
	}
      }
    }
    break;
  case PRS_VAR:
    Assert (0, "This is insane");
    break;
  default:
    fatal_error ("prop_up: unknown type %d\n", e->type);
    break;
  }
}

static void parse_prs (LEX_T *l, struct Hashtable *H);
static void parse_connection (LEX_T *l, struct Hashtable *H);
static void parse_excl (LEX_T *l, int ishi, Prs *p);

static void parse_file (LEX_T *l, Prs *p)
{
  PrsExpr *e, *ret;
#ifdef VERBOSE
  int nrules = 0;
#endif
  
  while (!(lex_sym (l) == l_err) && !lex_eof (l)) {
#ifdef VERBOSE
    if (((nrules + 1) % 10000) == 0) {
      printf ("%dK...", (nrules+1)/1000);
      fflush (stdout);
    }
#endif
    if (lex_have_keyw (l, "connect") || lex_have (l, TOK_EQUAL))
      parse_connection (l, p->H);
    else if (lex_have_keyw (l, "exclhi") || lex_have_keyw (l, "excl")) {
      parse_excl (l, 1, p);
    }
    else if (lex_have_keyw (l, "excllo")) {
      parse_excl (l, 0, p);
    }
    else {
      parse_prs (l, p->H);
    }
#ifdef VERBOSE
    nrules++;
#endif
  }
#ifdef VERBOSE
  printf ("Read %d rules\n\n", nrules);
#endif
}


/* self-flattening alias tree */
static PrsNode *canonical_name (PrsNode *n)
{
  PrsNode *r, *tmp;

  r = n;
  while (n->alias)
    n = n->alias;
  if (n != r) {
    while (r->alias) {
      tmp = r->alias;
      r->alias = n;
      r = tmp;
    }
  }
  return n;
}

static void canonicalize_hashtable (Prs *p)
{
  int i;
  struct Hashlistelement *e;

  for (i=0; i < p->H->size; i++)
    for (e = p->H->head[i]; e; e = e->next) {
      e->value.p = (void*) canonical_name ((PrsNode *)e->value.p);
    }
}

static void canonicalize_excllist (Prs *p)
{
  int i;
  PrsExclRing *r;

  for (i=0; i < A_LEN(p->exhi); i++) {
    r = p->exhi[i];
    do {
      r->n = canonical_name (r->n);
      r = r->next;
    } while (r != p->exhi[i]);
  }
  for (i=0; i < A_LEN(p->exlo); i++) {
    r = p->exlo[i];
    do {
      r->n = canonical_name (r->n);
      r = r->next;
    } while (r != p->exlo[i]);
  }
}

static PrsNode *raw_lookup (char *s, struct Hashtable *H)
{
  struct Hashlistelement *e;
  PrsNode *n, *r;

  e = associateHashtable (H, s);
  if (!e) {
    return NULL;
  }
  else {
    n = (PrsNode *)e->value.p;
    return canonical_name (n);
  }
}

static PrsNode *raw_insert (char *s, struct Hashtable *H)
{
  struct Hashlistelement *e;
  PrsNode *n;

  e = addHashtable (H, s);
  n = newnode ();
  /* link from node <-> bucket */
  n->e = e;
  e->value.p = (void*)n;

  return n;
}

/*
 *  Returns PrsNode * corresponding to node name "s"
 */
static PrsNode *lookup (char *s, struct Hashtable *H)
{
  PrsNode *n;

  n = raw_lookup (s, H);
  if (!n) {
    return raw_insert (s, H);
  }
  else
    return n;
}


/* 
 *  Merge data structures (except alias stuff and hash bucket). 
 *  n1 gets stuff that used to be in n2.
 *
 *  [ n1 := n1 UNION n2 ]
*/
static void merge_nodes (PrsNode *n1, PrsNode *n2)
{
  PrsExpr *e;
  int i;

  Assert (n1->val == n2->val, "Oh my god. You aren't parsing???");
  Assert (n1->queue == NULL, "Oh my god. You aren't parsing???");
  Assert (n2->queue == NULL, "Oh my god. You aren't parsing???");
  Assert (n1->bp == 0, "Oh my god. You aren't parsing???");
  Assert (n2->bp == 0, "Oh my god. You aren't parsing???");

  if (n2->up) {
    Assert (n2->up->r, "Hmm...");
    merge_or_up (n1, n2->up->r);
    delexpr (n2->up);
  }
  if (n2->dn) {
    Assert (n2->dn->r, "Hmm...");
    merge_or_dn (n1, n2->dn->r);
    delexpr (n2->dn);
  }
  for (i=0; i < n2->sz; i++) {
    e = n2->out[i];
    Assert (e, "yow!");
    Assert (e->type == PRS_VAR, "woweee!");
    Assert ((PrsNode*)e->l == n2, "Oh my god");
    mk_out_link (e, n1);
    Assert ((PrsNode *)e->l == n1, "You're hosed.");
  }
  if (n2->max > 0) {
    FREE (n2->out);
  }
  n2->max = 0;
  n2->sz = 0;
}


/*
 *
 *  Implement "connect" operation
 *
 */
static void parse_connection (LEX_T *l, struct Hashtable *H)
{
  char *s;
  PrsNode *n1, *n2;
  RawPrsNode *n;

  s = lex_mustbe_id (l);
  n1 = lookup (s, H);
  s = lex_mustbe_id (l);
  n2 = lookup (s, H);

  if (n1 == n2) return;

  /* make the connection:
       make the alias traversal *through* the buckets, so changing
       a PrsNode -> RawPrsNode is a constant time operation
  */
  merge_nodes (n1, n2);		/* n1 := n1 UNION n2 */
  Assert ((PrsNode *)n2->e->value.p == n2, "Oh my god. You're dead.");

  n = newrawnode ();
  n2->e->value.p = (void *) n;
  n->e = n2->e;
  n->alias = n1;

  delnode (n2);
}


static PrsExpr *expr (LEX_T *l, struct Hashtable *H);


/*
 *
 *  Make links: e --> n's hash list element
 *              e <-- n's out[] array
 */
static void mk_out_link (PrsExpr *e, PrsNode *n)
{
  e->l = (PrsExpr *)n->e;
  e->r = NULL;
  if (n->max == n->sz) {
    if (n->max == 0) {
      n->max = 4;
      MALLOC (n->out, PrsExpr *, n->max);
    }
    else {
      n->max *= 2;
      REALLOC (n->out, PrsExpr *, n->max);
    }
  }
  n->out[n->sz++] = e;
}


/*
 * Parse excl directives
 */
static void parse_excl (LEX_T *l, int ishi, Prs *p)
{
  PrsNode *n;
  PrsExclRing *r, *s;

  lex_mustbe (l, TOK_LPAR);

  r = NULL;

  do {
    n = lookup (lex_mustbe_id (l), p->H);
    NEW (s, PrsExclRing);
    s->n = n;
    if (!r) {
      r = s;
    }
    else {
      s->next = r->next;
    }
    r->next = s;
    if (ishi)
      n->exclhi = 1;
    else
      n->excllo = 1;
  } while (lex_have (l, TOK_COMMA));

  if (ishi) {
    A_NEW (p->exhi, PrsExclRing *);
    A_NEXT (p->exhi) = r;
    A_INC (p->exhi);
  }
  else {
    A_NEW (p->exlo, PrsExclRing *);
    A_NEXT (p->exlo) = r;
    A_INC (p->exlo);
  }
  lex_mustbe (l, TOK_RPAR);
}

/*
 *
 * Welcome to yet-another-expression parser...
 *
 */
static PrsExpr *unary (LEX_T *l, struct Hashtable *H)
{
  PrsExpr *e;
  PrsNode *n;
  char *s;

  if (lex_is_id (l)) {
    s = lex_mustbe_id (l);
    n = lookup (s, H);
    e = newexpr();
    e->type = PRS_VAR;
    e->val = 0;
    e->valx = 1;
    mk_out_link (e,n);
    Assert (NODE(e) == n, "Invariant: NODE(e) == n violated!");
  }
  else if (lex_have (l, TOK_NOT)) {
    e = newexpr();
    e->type = PRS_NOT;
    e->val = PRS_VAL_X;
    e->l = unary (l, H);
    e->l->u = e;
  }
  else if (lex_have (l, TOK_LPAR)) {
    e = expr (l, H);
    lex_mustbe (l, TOK_RPAR);
  }
  else {
    fatal_error ("unary: Expected `(', `id', `~'\n\t%s", lex_errstring 
		 (l));
  }
  return e;
}

static PrsExpr *term (LEX_T *l, struct Hashtable *H)
{
  PrsExpr *e, *ret;

  ret = unary (l, H);
  if (ret && lex_have (l, TOK_AND)) {
    e = ret;
    ret = newexpr ();
    e->u = ret;
    ret->l = e;
    ret->type = PRS_AND;
    ret->val = 0;
    ret->valx = 1;
    do {
      e->r = unary (l, H);
      e = e->r;
      if (e) {
	e->u = ret;
	ret->valx++;
      }
    } while (e && lex_have (l, TOK_AND));
  }
  return ret;
}

static PrsExpr *expr (LEX_T *l, struct Hashtable *H)
{
  PrsExpr *e, *ret;

  ret = term (l, H);
  if (ret && lex_have (l, TOK_OR)) {
    e = ret;
    ret = newexpr ();
    e->u = ret;
    ret->l = e;
    ret->type = PRS_OR;
    ret->val = 0;
    ret->valx = 1;
    do {
      e->r = term (l, H);
      e = e->r;
      if (e) {
	e->u = ret;
	ret->valx++;
      }
    } while (e && lex_have (l, TOK_OR));
  }
  return ret;
}


/*
 *
 *  Take two expressions e1 and e2, and construct expression
 *  (e1 | e2). If the root of either is |, it shares the | node.
 *
 */
static PrsExpr *or_merge (PrsExpr *e1, PrsExpr *e2)
{
  PrsExpr *e;

  if (e2->type == PRS_OR) {
    e = e1;
    e1 = e2;
    e2 = e;
  }
  if (e1->type == PRS_OR) {
#if 0
    /* old code. why was this ever here? */
    e = e1->l;
    while (e->r)
      e = e->r;
    e->r = e2;
    e2->u = e1;
    e1->valx ++;
    e = e1;
#endif
    e2->r = e1->l;
    e2->u = e1;
    e1->valx ++;
    e = e1;
  }
  else {
    e = newexpr();
    e->val = 0;
    e->valx = 2;
    e->type = PRS_OR;
    e->l = e1;
    e->l->r = e2;
    e1->u = e;
    e2->u = e;
  }
  return e;
}

/*
 *  add a disjunct to a pull-up
 */
static void merge_or_up (PrsNode *n, PrsExpr *e)
{
  Assert (n->up->r, "merge_or_up: no ->r field for expression tree root");

  e = or_merge (n->up->r, e);
  n->up->r = e;
  e->u = n->up;
}

/*
 *  add a disjunct to a pull-down
 */
static void merge_or_dn (PrsNode *n, PrsExpr *e)
{
  Assert (n->dn->r, "merge_or_dn: no ->r field for expression tree root");

  e = or_merge (n->dn->r, e);
  n->dn->r = e;
  e->u = n->dn;
}
  

/*
 *
 *  Parse single production rule
 *
 */
static void parse_prs (LEX_T *l, struct Hashtable *H)
{
  PrsExpr *e, *ne;
  PrsNode *n;
  int v, delay;

  if (lex_have_keyw (l, "after")) {
    lex_mustbe (l, l_integer);
    delay = lex_integer (l);
  }
  else {
    delay = 10;
  }

  e = expr (l, H);
  if (!e) return;
  lex_mustbe (l, TOK_ARROW);
  n = lookup(lex_mustbe_id (l), H);
  n->delay = delay;
  if (lex_have (l, TOK_UP)) {
    v = 1;
  }
  else if (lex_have (l, TOK_DN)) {
    v = 0;
  }
  else {
    fatal_error ("Expected `+' or `-'\n\t%s", lex_errstring (l));
  }

  if (v) {
    if (n->up) {
      merge_or_up (n, e);
      return;
    }
  }
  else {
    if (n->dn) {
      merge_or_dn (n, e);
      return;
    }
  }
  ne = newexpr ();
  ne->r = e;
  e->u = ne;
  ne->type = v ? PRS_NODE_UP : PRS_NODE_DN;
  ne->val = PRS_VAL_X;
  ne->l = (PrsExpr *)n->e;
  if (v) {
    n->up = ne;
  }
  else {
    n->dn = ne;
  }
}


/*
 *
 *  Debugging
 *
 */
static void print_expr_tree (PrsExpr *e)
{
  PrsExpr *x;

  Assert (e, "print_expr_tree: NULL expression argument");

  switch (e->type) {
  case PRS_AND:
    printf ("(& :[#0=%d, #X=%d] ", e->val, e->valx);
    x = e->l;
    while (x) {
      Assert (x->u == e, "print: Invariant violation: uplink(downlink(expr)) != expr");
      print_expr_tree (x);
      x = x->r;
      if (x) printf (" ");
    }
    printf (")");
    break;

  case PRS_OR:
    printf ("(| :[#1=%d, #X=%d] ", e->val, e->valx);
    x = e->l;
    while (x) {
      Assert (x->u == e, "print: Invariant violation: uplink(downlink(expr)) != expr");
      print_expr_tree (x);
      x = x->r;
      if (x) printf (" ");
    }
    printf (")");
    break;

    break;
  case PRS_NOT:
    printf ("(~ ");
    Assert (e->l->u == e, "print: Invariant violation: uplink(downlink(expr)) != expr");
    print_expr_tree (e->l);
    printf (")");
    break;
  case PRS_NODE_UP:
    printf ("(UP-%c :[%s] ", prs_nodechar(e->val), prs_nodename(NODE(e)));
    Assert (e->r->u == e, "print: Invariant violation: uplink(downlink(expr)) != expr");
    print_expr_tree (e->r);
    printf (")");
    
    break;
  case PRS_NODE_DN:
    printf ("(DN-%c :[%s] ", prs_nodechar(e->val), prs_nodename(NODE(e)));
    Assert (e->r->u == e, "print: Invariant violation: uplink(downlink(expr)) != expr");
    print_expr_tree (e->r);
    printf (")");
    break;
  case PRS_VAR:
    printf ("%s", prs_nodename (NODE(e)));
    break;
  default:
    fatal_error ("prop_up: unknown type %d\n", e->type);
    break;
  }
}

/*  prec:
 *        1 = ~
 *        2 = &
 *        3 = |
 *        4 = top-level
 */
static void print_rule_tree (PrsExpr *e, int prec)
{
  PrsExpr *x;

  Assert (e, "print_rule_tree: NULL expression argument");

  switch (e->type) {
  case PRS_AND:
    if (prec < 2) printf ("(");
    x = e->l;
    while (x) {
      Assert (x->u == e, "print: Invariant violation: uplink(downlink(expr)) != expr");
      print_rule_tree (x, 2);
      x = x->r;
      if (x) printf (" & ");
    }
    if (prec < 2) printf (")");
    break;

  case PRS_OR:
    if (prec < 3) printf ("(");
    x = e->l;
    while (x) {
      Assert (x->u == e, "print: Invariant violation: uplink(downlink(expr)) != expr");
      print_rule_tree (x, 3);
      x = x->r;
      if (x) printf (" | ");
    }
    if (prec < 3) printf (")");
    break;

    break;
  case PRS_NOT:
    printf ("~");
    Assert (e->l->u == e, "print: Invariant violation: uplink(downlink(expr)) != expr");
    print_rule_tree (e->l, 1);
    break;
  case PRS_NODE_UP:
    Assert (e->r->u == e, "print: Invariant violation: uplink(downlink(expr)) != expr");
    print_rule_tree (e->r, 4);
    printf (" -> %s+", prs_nodename (NODE(e)));
    
    break;
  case PRS_NODE_DN:
    Assert (e->r->u == e, "print: Invariant violation: uplink(downlink(expr)) != expr");
    print_rule_tree (e->r, 4);
    printf (" -> %s-", prs_nodename (NODE(e)));

    break;
  case PRS_VAR:
    printf ("%s", prs_nodename (NODE(e)));
    break;
  default:
    fatal_error ("prop_up: unknown type %d\n", e->type);
    break;
  }
}


/*
 *  Free storage
 */
void prs_free (Prs *p)
{
  /* XXX: fixme */
}

void prs_apply (Prs *p, void *cookie, void (*f)(PrsNode *, void *))
{
  int i; 
  struct Hashlistelement *e;
  PrsNode *n;

  for (i=0; i < p->H->size; i++)
    for (e = p->H->head[i]; e; e = e->next) {
      n = (PrsNode *)e->value.p;
      if (n->alias) continue;
      (*f)(n,cookie);
    }
}

void prs_dump_node (PrsNode *n)
{
  if (n->up) {
    print_expr_tree (n->up);
    printf ("\n");
  }
  if (n->dn) {
    print_expr_tree (n->dn);
    printf ("\n");
  }
}

void prs_printrule (PrsNode *n)
{
  if (n->up) {
    print_rule_tree (n->up, 0);
    printf ("\n");
  }
  if (n->dn) {
    print_rule_tree (n->dn, 0);
    printf ("\n");
  }
}

void prs_print_expr (PrsExpr *n)
{
  print_rule_tree (n, 0);
  printf ("\n");
}


static void dump_node (PrsNode *n, void *v)
{
  prs_dump_node (n);
}

void dump_prs (Prs *p)
{
  prs_apply (p, NULL, dump_node);
}

static void setX (PrsNode *n, void *cookie)
{
  if (n->queue && !n->exq)
    n->queue->val = PRS_VAL_X;
  else
    prs_set_node ((Prs*)cookie, n, PRS_VAL_X);
}

void prs_initialize (Prs *p)
{
  PrsNode *n;

  prs_apply (p, (void*)p, setX);

  while (n = prs_step (p)) {
    ;
  }
}
