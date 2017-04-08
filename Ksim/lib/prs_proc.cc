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
 *  $Id: prs_proc.cc,v 1.1.1.1 2006/05/23 13:53:59 mainakc Exp $
 *
 *************************************************************************/
#include "prs_proc.h"
#include "qops.h"

extern "C" {

extern thread_t *readyQh;
extern thread_t *readyQt;

};

#define DATA0 1		// => set data valid (wait for ack)
#define DATA1 2		// => send data neutral (wait for nack)
#define WAIT0 3
#define WAIT1 4

#define IDLE 0

static char *prs_debug[] = { "idle", "d0", "d1", "w0", "w1" };

static void prsinit (Prs *p)
{
  PrsNode *r;

  r = prs_node (p, "Reset");
  if (!r) return;

  p->flags |= PRS_NO_WEAK_INTERFERENCE;
  prs_set_node (p, r, PRS_VAL_T);
  prs_cycle (p);
  p->flags &= ~PRS_NO_WEAK_INTERFERENCE;
}

PrsProc::PrsProc (FILE *fp)
{
  _p = prs_file (fp);
  prsinit (_p);
}

PrsProc::PrsProc (char *s)
{
  _p = prs_fopen (s);
  prsinit (_p);
}

PrsProc::~PrsProc ()
{
  prs_free (_p);
}

PrsNode *PrsProc::Node (char *s)
{
  return prs_node (_p, s);
}


void PrsProc::Send (int i, unsigned int v)
{
  _ports[i].data = v;
  _Handshake (i);
}

unsigned int PrsProc::Recv  (int i)
{
  return _Handshake (i);
}


/*
 *
 *  Canonical handshake protocol:
 *
 *  {IDLE}->{DATA0} data+; {WAIT0} [ack]; {DATA1} data-; {WAIT1} [~ack] {IDLE}
 *
 *
 */

/*
 *   Complete communication
 */
unsigned int PrsProc::_Handshake (int i)
{
  // disable interrupts
  context_disable ();

  Assert (0 <= i && i < _numports, "Invalid port to Send() function");

  // we must be in the IDLE state to even attempt a handshake
  if (_ports[i].state != IDLE) {
    // schedule prs main loop
    if (_self_wait) {
      // prs main loop is waiting
      q_ins (readyQh, readyQt, _self_wait);
      _self_wait->in_readyq = 1;
      _self_wait = NULL;
    }
    else {
      // prs main loop is running
      _p->flags |= PRS_STOP_SIMULATION;
    }

    // suspend current process
    Assert (_ports[i].susp == NULL, "What on earth?");
    _ports[i].susp = current_process;
    context_switch (context_select ());
    context_disable ();
    Assert (_ports[i].susp == NULL, "oh my gosh");
  }

  Assert (_ports[i].state == IDLE, "oh my gosh");

  // set data, start handshake
  _ports[i].state = DATA0;
  _ports[i].time = current_process->time;

  //printf ("idle -> data0\n");

  // schedule prs main loop mechanics
  if (_self_wait) {
    q_ins (readyQh, readyQt, _self_wait);
    _self_wait->in_readyq = 1;
    _self_wait = NULL;
  }
  else {
    _p->flags |= PRS_STOP_SIMULATION;
  }

  Assert (_ports[i].susp == NULL, "oh my gosh");
  _ports[i].susp = current_process;
  context_switch (context_select ());

  Assert (_ports[i].susp == NULL, "oh my gosh");
  Assert (_ports[i].state == IDLE, "Hmm?");

  current_process->time = _ports[i].time;

  return _ports[i].data;
}

// XXX: wrong. This needs to be the max of the time at which the
// port action last occurred and the time at which the port is
// ready to communicate.
//
// Every prs action and external process action should max its time
// with the current port time.
//

/*
 *
 *  Should ONLY be called by the main loop
 *
 */
void PrsProc::_HandshakeMechanics (int i)
{
  switch (_ports[i].state) {
  case DATA0:
    //printf ("data0 -> wait0\n");
    _ports[i].time = time_max (_ports[i].time, _p->time);
    (*_ports[i].dfn) (_p, &_ports[i]);
    _ports[i].state = WAIT0;
    break;
  case WAIT0:
    //printf ("wait0 -> data1\n");
    _ports[i].state = DATA1;
    _ports[i].time = time_max (_ports[i].time, _p->time);
    break;
  case DATA1:
    //printf ("data1 -> wait1\n");
    (*_ports[i].dfn) (_p, &_ports[i]);
    _ports[i].state = WAIT1;
    break;
  case WAIT1:
    //printf ("wait1 -> idle\n");
    _ports[i].state = IDLE;
    Assert (_ports[i].susp, "What on earth?");
    context_disable ();
    q_ins (readyQh, readyQt, _ports[i].susp);
    _ports[i].susp->in_readyq = 1;
    _ports[i].susp = NULL;
    context_enable ();
    break;
  default:
    fatal_error ("Invalid handshake protocol!\n");
    break;
  }
}


/*
 *   Return a new port
 */
int PrsProc::_NewPort (void)
{
  int i;

  if (_numports == _maxports) {
    if (_maxports == 0) {
      _maxports = 4;
      MALLOC (_ports, PrsChannel, _maxports);
    }
    else {
      _maxports += 4;
      REALLOC (_ports, PrsChannel, _maxports);
    }
  }
  i = _numports++;

  _ports[i].nd = 0;
  _ports[i].na = 0;
  _ports[i].state = IDLE;
  _ports[i].susp = NULL;
  _ports[i].data = (unsigned int)NULL;

  return i;
}


/*
 *
 * Return 1 if there is a pending event that a handshake protocol
 * needs to take care of
 *
 */
int PrsProc::_PendingEvent (int i)
{
  //static char *state[] = { "idle", "data0", "data1", "wait0", "wait1" };

  //printf ("check pending, state=%s\n", state[_ports[i].state]);

  // data0, data1 states involve spontaneous responses
  if (_ports[i].state == DATA0  || _ports[i].state == DATA1) return 1;

  // we can respond from wait states if we've passed the thresholds
  if (_ports[i].ta > 0) {
    if ((_ports[i].va == _ports[i].ta  && _ports[i].state == WAIT0)
	|| (_ports[i].va == 0 && _ports[i].state == WAIT1))
      return 1;
  }
  else {
    if ((_ports[i].va == -_ports[i].ta && _ports[i].state == WAIT0)
	|| (_ports[i].va == _ports[i].na && _ports[i].state == WAIT1))
      return 1;
  }
  return 0;
}

// update ack values
void PrsProc::update (int i, PrsNode *n)
{
  if (prs_nodeval (n) == PRS_VAL_T) {
    _ports[i].va += (_ports[i].ta > 0 ? 1 : -1);
  }
  else if (prs_nodeval (n) == PRS_VAL_F) {
    _ports[i].va += (_ports[i].ta > 0 ? -1 : 1);
  }
  else {
    fatal_error ("Channel node `%s' became X\n", prs_nodename (n));
  }
}


/*
 *
 * Main simulation loop
 *
 */
void PrsProc::MainLoop (void)
{
  PrsNode *n;
  int i, k;
  int err = 0;
  
  context_disable ();
  _p->flags &= ~PRS_STOP_SIMULATION;

  // reset everything
  for (i=0; i < _numports; i++) {
    (*_ports[i].rst) (_p, &_ports[i]);
    prs_cycle (_p);
  }

  // make sure nothing is "X"
  for (i=0; i < _numports; i++) {
    for (k=0; k < _ports[i].nd; k++) {
      if (prs_nodeval (_ports[i].d[k]) == PRS_VAL_X) {
	printf ("Data node `%s' has value `%c'\n",
		prs_nodename (_ports[i].d[k]), 
		prs_nodechar(prs_nodeval(_ports[i].d[k])));
	err = 1;
      }
    }

    for (k=0; k < _ports[i].na; k++) {
      if (prs_nodeval (_ports[i].a[k]) == PRS_VAL_X) {
	printf ("Ack node `%s' has value `%c'\n",
		prs_nodename (_ports[i].a[k]), 
		prs_nodechar(prs_nodeval(_ports[i].a[k])));
	err = 1;
      }
      prs_set_bp (_p, _ports[i].a[k]);
    }
  }

  // set Reset 0
  n = prs_node (_p, "Reset");
  prs_set_node (_p, n, PRS_VAL_F);
  context_enable ();
  
  if (err) thread_exit (1);

  while (1) {
    context_disable ();
    _p->flags &= ~PRS_STOP_SIMULATION;
    context_enable ();
    n = prs_cycle (_p);
    context_disable ();
    // this is incredibly inefficient...
    if (n) {
      // there was a breakpoint on this node.
      //printf ("node %s became %c\n", prs_nodename (n),
      //prs_nodechar(prs_nodeval (n)));
      for (i=0; i < _numports; i++) {
	for (k=0; k < _ports[i].na; k++) {
	  if (_ports[i].a[k] == n) {
	    // if node is part of port i
	    update (i, n);
	  }
	  if (_PendingEvent (i)) {
	    //printf ("pending event\n");
	    _HandshakeMechanics (i);
	  }
	}
      }
      context_enable ();
    }
    else {
      err = 0;
      for (i=0; i < _numports; i++) {
	if (_PendingEvent (i)) {
	  //printf ("pending event\n");
	  _HandshakeMechanics (i);
	  err = 1;
	}
      }
      if (!err) {
	_self_wait = current_process;
	context_switch (context_select ());
      }
      else {
	context_enable ();
      }
    }
  }
}

/*------------------------------------------------------------------------
 *
 *   Channel definitions
 *
 *------------------------------------------------------------------------
 */

/*
 *
 *     Dual-rail
 *
 */

//
//  channel data := value
//
static void dr_val_to_chan (Prs *_p, PrsChannel *p)
{
  int i, x;

  if (p->state == DATA0) {
    for (i=0; i < p->nd/2; i++) {
      x = p->data & 1;
      p->data >>= 1;
      if (x) {
	prs_set_nodetime (_p, p->d[2*i], PRS_VAL_F, time_add (p->time,1));
	prs_set_nodetime (_p, p->d[2*i+1], PRS_VAL_T, time_add(p->time,1));
      }
      else {
	prs_set_nodetime (_p, p->d[2*i], PRS_VAL_T, time_add (p->time,1));
	prs_set_nodetime (_p, p->d[2*i+1], PRS_VAL_F, time_add (p->time,1));
      }
    }
  }
  else {
    for (i=0; i < p->nd/2; i++) {
      prs_set_node (_p, p->d[2*i], PRS_VAL_F);
      prs_set_node (_p, p->d[2*i+1], PRS_VAL_F);
    }
  }      
}

//
//  value := channel data
//
static void dr_chan_to_val (Prs *_p, PrsChannel *p)
{
  int i, x;

  if (p->state == DATA1) {
    x = 0;
    for (i=p->na/2-1; i >= 0; i--) {
      x <<= 1;
      if (prs_nodeval (p->a[2*i]) == PRS_VAL_F)
	x |= 1;
    }
    p->data = x;
    if (p->td > 0) {
      for (i=0; i < p->nd; i++)
	prs_set_node (_p, p->d[i], PRS_VAL_F);
    }
    else {
      for (i=0; i < p->nd; i++)
	prs_set_node (_p, p->d[i], PRS_VAL_T);
    }
  }
  else {
    if (p->td > 0) {
      for (i=0; i < p->nd; i++)
	prs_set_node (_p, p->d[i], PRS_VAL_T);
    }
    else {
      for (i=0; i < p->nd; i++)
	prs_set_node (_p, p->d[i], PRS_VAL_F);
    }
  }
}

static void dr_set_ack_zero (Prs *_p, PrsChannel *c)
{
  int i;
  
  if (c->ta > 0) {
    for (i=0; i < c->na; i++) {
      prs_set_node (_p, c->a[i], PRS_VAL_F);
    }
  }
  else {
    for (i=0; i < c->na; i++) {
      prs_set_node (_p, c->a[i], PRS_VAL_T);
    }
  }
}

static void dr_set_data_zero (Prs *_p, PrsChannel *c)
{
  int i;
  
  if (c->td > 0) {
    for (i=0; i < c->nd; i++) {
      prs_set_node (_p, c->d[i], PRS_VAL_F);
    }
  }
  else {
    for (i=0; i < c->nd; i++) {
      prs_set_node (_p, c->d[i], PRS_VAL_T);
    }
  }
}

/*
 * Active send
 */
int PrsProc::DRSendPort (int nd, PrsNode **d, int na, PrsNode **a)
{
  int i;
  int err = 0;
  PrsChannel *p;

  Assert (nd > 0, "Number of data nodes <= 0!");
  Assert (na > 0, "Number of ack nodes <= 0!");

  Assert ((nd & 1) == 0, "Number of data nodes for DR channel must be even");
  
  
  for (i=0; i < nd; i++)
    Assert (d[i], "NULL data port");

  for (i=0; i < na; i++)
    Assert (a[i], "NULL ack port");

  p = &_ports[i = _NewPort ()];
  
  p->nd = nd;
  p->na = na;
  p->d = d;
  p->a = a;

  p->td = nd/2;
  p->ta = na;
  
  p->va = 0;
  p->vd = 0;

  p->dfn = dr_val_to_chan;
  p->rst = dr_set_data_zero;

  return i;
}


/*
 *   Passive receive
 */
int PrsProc::DRRecvPort (int nd, PrsNode **d, int na, PrsNode **a)
{
  int i;
  PrsChannel *p;

  Assert (nd > 0, "Number of data nodes <= 0!");
  Assert (na > 0, "Number of ack nodes <= 0!");

  Assert ((nd & 1) == 0, "Number of data nodes for DR channel must be even");
  
  for (i=0; i < nd; i++)
    Assert (d[i], "NULL data port");

  for (i=0; i < na; i++)
    Assert (a[i], "NULL ack port");

  p = &_ports[i = _NewPort ()];

  p->nd = na;
  p->d = a;

  p->na = nd;
  p->a = d;

  p->td = -na;
  p->ta = nd/2;
  
  p->va = 0;
  p->vd = 0;

  p->dfn = dr_chan_to_val;
  p->rst = dr_set_data_zero;

  return i;
}
