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
 *  $Id: prs_proc.h,v 1.1.1.1 2006/05/23 13:53:59 mainakc Exp $
 *
 *************************************************************************/
#ifndef __PRS_PROC_H__
#define __PRS_PROC_H__

#include "sim.h"
#include "prs.h"
#include "inthash.h"
#include "mytime.h"

struct PrsChannel {
  int nd, na;
  PrsNode **d, **a;		// (node1,node2)
  int vd, va;			// value (# of `1's)
  int td, ta;			// thresholds
  int state;			// state
  unsigned int data;		// send/recv data
  Time_t time;			// suspension time

  thread_t *susp;		// suspended thread, if any

  void (*dfn)(Prs *p, PrsChannel *c);
  void (*rst)(Prs *p, PrsChannel *c);
};
  

class PrsProc : public SimObject {
public:
  PrsProc (FILE *fp);
  PrsProc (char *s);
  ~PrsProc ();
  
  PrsNode *Node (char *s);
  
  void Send (int port, unsigned int data);
  unsigned int Recv (int port);

  // NOTE: all channel creation functions save the arrays passed to
  // them. Do NOT FREE them after calling the functions. Don't
  // allocate them on the stack either.
  // These functions are not thread-safe; they must be called
  // before simulation begins.

  // dualrail channel
  //   -- [d, a] are the data and ack rails
  //   -- d[2n] == bit[n].0, d[2n+1] == bit[n].1
  //   -- bit[0] == lsb

  //  nd == length of d == 2*num_of_bits ...
  int DRSendPort (int nd, PrsNode **d, int na, PrsNode **a);
  int DRRecvPort (int nd, PrsNode **d, int na, PrsNode **a);
  
  FAKE_SIM_TEMPLATE;
private:
  struct IntHashtable *_H;	// channel node hash table

  thread_t *_self_wait;		// self-wait queue

  Prs *_p;			// production rule data structure

  PrsChannel *_ports;		// created ports
  int _numports;		// number of ports
  int _maxports;

  int _NewPort();		// return new port #

  // communication mechanics
  void _HandshakeMechanics (int i);
  int _PendingEvent (int i);

  void update (int i, PrsNode *n);

  // send/recv
  unsigned int _Handshake (int i);
};

#endif /* __PRS_PROC_H__ */
