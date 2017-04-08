/*************************************************************************
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
 *  $Id: tcp.cc,v 1.1.1.1 2006/05/23 13:53:59 mainakc Exp $
 *
 **************************************************************************
 */
#include "tcp.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include "misc.h"
#include <fcntl.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <errno.h>


#ifndef INADDR_NONE
#define INADDR_NONE 0xffffffff
#endif

#if defined(__FreeBSD__)
#if __FreeBSD__ < 4
typedef int socklen_t;
#endif
#endif


Tcp::Tcp (char *host, short port, int mode)
{
  struct hostent *hp;

  if ((hp = gethostbyname (host)) == NULL) {
    fatal_error ("Tcp::Tcp: gethostbyname `%s' failed!\n", host);
  }

  if ((_fd = socket (AF_INET, SOCK_STREAM, 0)) < 0) {
    perror ("socket");
    exit (1);
  }
  
  _addr.sin_family = AF_INET;
  bcopy (hp->h_addr, &_addr.sin_addr, sizeof (_addr.sin_addr));
  _addr.sin_port = htons (port);

  _state = TCP_CLIENT;

  if (mode == TCP_CONNECT) {
    if (connect (_fd, (struct sockaddr*)&_addr, sizeof(_addr)) < 0) {
      perror ("Connect failed");
      exit (1);
    }
    _state |= TCP_CONNECTED;
  }
}

Tcp::Tcp (short port)
{
  if ((_port = socket (AF_INET, SOCK_STREAM, 0)) < 0) {
    perror ("socket");
    exit (1);
  }
  _addr.sin_family = AF_INET;
  _addr.sin_port = htons (port);
  _addr.sin_addr.s_addr = htons (INADDR_ANY);
  
  if (bind (_port, (struct sockaddr *)&_addr, sizeof(_addr)) < 0) {
    perror ("bind failed");
    exit (1);
  }
  
  _state = 0;

  if (listen (_port, 2) < 0) {
    perror ("listen failed");
    exit (1);
  }
}


Tcp::~Tcp (void)
{
  if (_state & TCP_CLIENT) {
    if (_state & TCP_CONNECTED) 
      close (_fd);
  }
  else {
    close (_port);
  }
}

int Tcp::Send (char *msg, int len)
{
  if (!(_state & TCP_CLIENT))
    return -1;

  if (!(_state & TCP_CONNECTED))
    return -1;

  return write (_fd, msg, len);
}


int Tcp::Recv (char *msg, int len)
{
  if (!(_state & TCP_CLIENT))
    return -1;

  if (!(_state & TCP_CONNECTED))
    return -1;

  return read (_fd, msg, len);
}


int Tcp::Accept (struct sockaddr *addr, int *addrlen)
{
  if (_state  & TCP_CLIENT)
    return -1;

#if defined(__alpha__)
  return accept (_port, addr, addrlen);
#else
  return accept (_port, addr, (socklen_t *)addrlen);
#endif
}


int Tcp::Connect (void)
{
  if (!(_state & TCP_CLIENT))
    return -1;

  if (_state & TCP_CONNECTED)
    return 0;

  if (connect (_fd, (struct sockaddr*)&_addr, sizeof(_addr)) < 0) {
    return -1;
  }

  _state |= TCP_CONNECTED;
  return 0;
}


int Tcp::FileDescriptor (void)
{
  if (_state & TCP_CLIENT)
    return _fd;
  else
    return _port;
}

int Tcp::assemble_line (int fd, char *s, int len)
{
  int nbytes = 0;
  int err;

  while (1) {
    err = read (fd, s + nbytes, 1);
    if (err < 0) return -1;
    nbytes += err;
    if (s[nbytes-1] == '\n')
      return nbytes;
  }
}

void Tcp::daemon (void)
{
  int childpid, fd;

  chdir ("/");

  if (getppid() != 1) {

#ifdef SIGPIPE
    signal (SIGPIPE, SIG_IGN);
#endif
#ifdef SIGIO
    signal (SIGIO, SIG_IGN);
#endif
#ifdef SIGTTOU
    signal(SIGTTOU, SIG_IGN);
#endif
#ifdef SIGTTIN
    signal(SIGTTIN, SIG_IGN);
#endif
#ifdef SIGTSTP
    signal(SIGTSTP, SIG_IGN);
#endif

    if ( (childpid = fork()) < 0)
      fatal_error ("can't fork first child");
    else if (childpid >0)
      exit(0);     /* parent */
 
#ifdef LINUX
    if (setpgrp() == -1)
      fatal_error ("Can't change process group");
          
    signal(SIGHUP, SIG_IGN);  /* immune from pgrp leader death */
          
    if ( (childpid = fork()) < 0)
      fatal_error ("Can't fork second child");
    else if (childpid > 0)
      exit(0); /* First child */
          
    /* second child */
#else
#ifdef SIGTSTP
    if (setpgrp(0, getpid()) == -1)
      fatal_error ("can't change process group");

    if ( (fd = open("/dev/tty", O_RDWR)) >= 0) {
      ioctl(fd, TIOCNOTTY, (char *)NULL); /* lose controlling TTY*/
      close(fd);
    }

#else /* System V */
    if (setpgrp() == -1)
      fatal_error ("Can't change process group");
          
    signal(SIGHUP, SIG_IGN);  /* immune from pgrp leader death */
          
    if ( (childpid = fork()) < 0)
      fatal_error ("Can't fork second child");
    else if (childpid > 0)
      exit(0); /* First child */
          
    /* second child */
#endif
#endif
  }  /* End of if test for ppid != 1 */

  for (fd = 0; fd < NOFILE; fd++)
    close (fd);

  errno = 0;

  umask(0);

  signal(SIGCHLD, SIG_IGN);
}
