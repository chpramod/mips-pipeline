/*-*-mode:c++-*-**********************************************************
 *
 *  Copyright (c) 1999 Cornell University
 *  Computer Systems Laboratory
 *  Cornell University, Ithaca, NY 14853
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
 *  $Id: tcp.h,v 1.1.1.1 2006/05/23 13:53:59 mainakc Exp $
 *
 **************************************************************************
 */
#ifndef __TCP_H__
#define __TCP_H__


#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netinet/in.h>


/*
 *
 *  Simple tcp/ip interface
 *
 */
#define TCP_CONNECT 1		// make the connection
#define TCP_SETUP   2		// just save parameters

#define TCP_CONNECTED 0x1
#define TCP_ERROR     0x2
#define TCP_CLIENT    0x4

// All routines (unless otherwise specified) return -1 on error

class Tcp {
 public:
  Tcp (char *host, short port, int mode = TCP_CONNECT);	// client call
  Tcp (short port);		// server call

  ~Tcp ();

  // Send len bytes
  //  returns # of bytes sent, -1 on error
  int Send (char *msg, int len);

  // Recv len bytes
  //  returns # of bytes received, -1 on error
  int Recv (char *msg, int len);

  // returns a file descriptor for the next connection
  int Accept (struct sockaddr *addr = NULL, int *addrlen = NULL);

  // connect
  int Connect (void);

  // returns port# for servers (so you can use it for selects)
  int FileDescriptor (void);

  // Read until \n received. Useful for PP (protocol processing)
  static int assemble_line (int, char *, int);

  // Turn process into a daemon
  //    - close all file descriptors
  //    - disassociate from the controlling terminal
  //    - ignore a bunch of signals
  static void daemon (void);

private:
  struct sockaddr_in _addr;
  int   _fd;
  int   _port;			// for listening
  unsigned int   _state;
};


#endif /* __TCP_H__ */
