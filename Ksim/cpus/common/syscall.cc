/*-*-mode:c++-*-**********************************************************
 *
 *  Copyright (c) 1999 Cornell University
 *  School of Electrical Engineering
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
 *  $Id: syscall.cc,v 1.3 2007/11/23 19:58:22 mainakc Exp $
 *
 *************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#ifdef LINUX
#include <time.h>
#endif
#include <sys/time.h>
#include <signal.h>
#include "mem.h"
#include "syscall.h"
#include "misc.h"
#include "mips.h"

// map open(), creat() flags from host architecture to native
// architecture
unsigned int mode_remap (unsigned int);

extern int errno;

#define MINUS_ONE  0xffffffffffffffffULL

#define UNIX_RETURN(x) 				\
   do { 					\
     SetReg(REG_V0, x);				\
     if (((signed)GetReg(REG_V0)) < 0) { 	\
       SetReg(REG_A3,1);			\
       SetReg(REG_V0, errno); 			\
     } 						\
     else { 					\
       SetReg(REG_A3,0);			\
     } 						\
     return;					\
   } while (0)

#include "mips-irix5.h"

/*------------------------------------------------------------------------
 *
 * system call emulation
 *
 *------------------------------------------------------------------------
 */
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/uio.h>
#include <sys/times.h>

SysCall::SysCall (void) 
{
  int i;
  char buf[1024];
  
  for (i=0; i < 256; i++)
    _fdlist[i] = -1;
  
  // open file descriptors
  _fdlist[0] = 0;
  _fdlist[1] = 1;
  _fdlist[2] = 2;
  _argc = 0;
  
  _filelog = NULL;
  _filelogtail = NULL;
}

SysCall::SysCall (Mipc * ms)
{      
  int i;          
  char buf[1024];
  
  for (i=0; i < 256; i++)
    _fdlist[i] = -1;
   
  // open file descriptors
  _fdlist[0] = 0;
  _fdlist[1] = 1;
  _fdlist[2] = 2;
  _argc = 0;
 
  _filelog = NULL;
  _filelogtail = NULL;
} 


SysCall::~SysCall (void)
{
  int i;

  for (i=0; i < _argc; i++)
    FREE (_argv[i]);
  if (_argc)
    FREE (_argv);
}

void SysCall::Reset (void)
{
  int i;

  for (i=3; i < 256; i++) {
    if (_fdlist[i] > 0) {
      close (_fdlist[i]);
      _fdlist[i] = -1;
    }
  }
}

void SysCall::ArgumentSetup (int argc, char **argv, LL addr, char *input_file, unsigned thread_id)
{
  int i;
  Assert (argc > 0, "SysCall::ArgumentSetup: argc not positive?!");

  if (_argc) {
    for (i=0; i < _argc; i++)
      FREE (_argv[i]);
    if (_argc)
      FREE (_argv);
  }
  _argc = argc;
  MALLOC (_argv, char *, _argc);
  for (i=0; i < _argc; i++) {
    _argv[i] = Strdup (argv[i]);
  }
  _argaddr = addr;
}


char *SysCall::SysCallName (int num)
{
  switch (num) {

  case SYS_syscall:
    return "syscall";
    break;
  case SYS_exit:
    return "exit";
    break;
  case SYS_fork:
    return "fork";
    break;
  case SYS_read:
    return "read";
    break;
  case SYS_write:
    return "write";
    break;
  case SYS_open:
    return "open";
    break;
  case SYS_close:
    return "close";
    break;
  case SYS_creat:
    return "creat";
    break;
  case SYS_link:
    return "link";
    break;
  case SYS_unlink:
    return "unlink";
    break;
  case SYS_execv:
    return "execv";
    break;
  case SYS_chdir:
    return "chdir";
    break;
  case SYS_time:
    return "time";
    break;
  case SYS_chmod:
    return "chmod";
    break;
  case SYS_chown:
    return "chown";
    break;
  case SYS_brk:
    return "brk";
    break;
  case SYS_stat:
    return "stat";
    break;
  case SYS_lseek:
    return "lseek";
    break;
  case SYS_getpid:
    return "getpid";
    break;
  case SYS_mount:
    return "mount";
    break;
  case SYS_umount:
    return "umount";
    break;
  case SYS_setuid:
    return "setuid";
    break;
  case SYS_getuid:
    return "getuid";
    break;
  case SYS_stime:
    return "stime";
    break;
  case SYS_ptrace:
    return "ptrace";
    break;
  case SYS_alarm:
    return "alarm";
    break;
  case SYS_pause:
    return "pause";
    break;
  case SYS_utime:
    return "utime";
    break;
  case SYS_access:
    return "access";
    break;
  case SYS_nice:
    return "nice";
    break;
  case SYS_statfs:
    return "statfs";
    break;
  case SYS_sync:
    return "sync";
    break;
  case SYS_kill:
    return "kill";
    break;
  case SYS_fstatfs:
    return "fstatfs";
    break;
  case SYS_setpgrp:
    return "setpgrp";
    break;
  case SYS_dup:
    return "dup";
    break;
  case SYS_pipe:
    return "pipe";
    break;
  case SYS_times:
    return "times";
    break;
  case SYS_profil:
    return "profil";
    break;
  case SYS_plock:
    return "plock";
    break;
  case SYS_setgid:
    return "setgid";
    break;
  case SYS_getgid:
    return "getgid";
    break;
  case SYS_msgsys:
    return "msgsys";
    break;
  case SYS_sysmips:
    return "sysmips";
    break;
  case SYS_acct:
    return "acct";
    break;
  case SYS_shmsys:
    return "shmsys";
    break;
  case SYS_semsys:
    return "semsys";
    break;
  case SYS_ioctl:
    return "ioctl";
    break;
  case SYS_uadmin:
    return "uadmin";
    break;
  case SYS_sysmp:
    return "sysmp";
    break;
  case SYS_utssys:
    return "utssys";
    break;
  case SYS_execve:
    return "execve";
    break;
  case SYS_umask:
    return "umask";
    break;
  case SYS_chroot:
    return "chroot";
    break;
  case SYS_fcntl:
    return "fcntl";
    break;
  case SYS_ulimit:
    return "ulimit";
    break;
  case SYS_rmdir:
    return "rmdir";
    break;
  case SYS_mkdir:
    return "mkdir";
    break;
  case SYS_getdents:
    return "getdents";
    break;
  case SYS_sysfs:
    return "sysfs";
    break;
  case SYS_getmsg:
    return "getmsg";
    break;
  case SYS_putmsg:
    return "putmsg";
    break;
  case SYS_poll:
    return "poll";
    break;
  case SYS_sigreturn:
    return "sigreturn";
    break;
  case SYS_accept:
    return "accept";
    break;
  case SYS_bind:
    return "bind";
    break;
  case SYS_connect:
    return "connect";
    break;
  case SYS_gethostid:
    return "gethostid";
    break;
  case SYS_getpeername:
    return "getpeername";
    break;
  case SYS_getsockname:
    return "getsockname";
    break;
  case SYS_getsockopt:
    return "getsockopt";
    break;
  case SYS_listen:
    return "listen";
    break;
  case SYS_recv:
    return "recv";
    break;
  case SYS_recvfrom:
    return "recvfrom";
    break;
  case SYS_recvmsg:
    return "recvmsg";
    break;
  case SYS_select:
    return "select";
    break;
  case SYS_send:
    return "send";
    break;
  case SYS_sendmsg:
    return "sendmsg";
    break;
  case SYS_sendto:
    return "sendto";
    break;
  case SYS_sethostid:
    return "sethostid";
    break;
  case SYS_setsockopt:
    return "setsockopt";
    break;
  case SYS_shutdown:
    return "shutdown";
    break;
  case SYS_socket:
    return "socket";
    break;
  case SYS_gethostname:
    return "gethostname";
    break;
  case SYS_sethostname:
    return "sethostname";
    break;
  case SYS_getdomainname:
    return "getdomainname";
    break;
  case SYS_setdomainname:
    return "setdomainname";
    break;
  case SYS_truncate:
    return "truncate";
    break;
  case SYS_ftruncate:
    return "ftruncate";
    break;
  case SYS_rename:
    return "rename";
    break;
  case SYS_symlink:
    return "symlink";
    break;
  case SYS_readlink:
    return "readlink";
    break;
  case SYS_nfssvc:
    return "nfssvc";
    break;
  case SYS_getfh:
    return "getfh";
    break;
  case SYS_async_daemon:
    return "async_daemon";
    break;
  case SYS_exportfs:
    return "exportfs";
    break;
  case SYS_setregid:
    return "setregid";
    break;
  case SYS_setreuid:
    return "setreuid";
    break;
  case SYS_getitimer:
    return "getitimer";
    break;
  case SYS_setitimer:
    return "setitimer";
    break;
  case SYS_adjtime:
    return "adjtime";
    break;
  case SYS_BSD_getime:
    return "BSD_getime";
    break;
  case SYS_sproc:
    return "sproc";
    break;
  case SYS_prctl:
    return "prctl";
    break;
  case SYS_procblk:
    return "procblk";
    break;
  case SYS_sprocsp:
    return "sprocsp";
    break;
  case SYS_mmap:
    return "mmap";
    break;
  case SYS_munmap:
    return "munmap";
    break;
  case SYS_mprotect:
    return "mprotect";
    break;
  case SYS_msync:
    return "msync";
    break;
  case SYS_madvise:
    return "madvise";
    break;
  case SYS_pagelock:
    return "pagelock";
    break;
  case SYS_getpagesize:
    return "getpagesize";
    break;
  case SYS_quotactl:
    return "quotactl";
    break;
  case SYS_BSDgetpgrp:
    return "BSDgetpgrp";
    break;
  case SYS_BSDsetpgrp:
    return "BSDsetpgrp";
    break;
  case SYS_vhangup:
    return "vhangup";
    break;
  case SYS_fsync:
    return "fsync";
    break;
  case SYS_fchdir:
    return "fchdir";
    break;
  case SYS_getrlimit:
    return "getrlimit";
    break;
  case SYS_setrlimit:
    return "setrlimit";
    break;
  case SYS_cacheflush:
    return "cacheflush";
    break;
  case SYS_cachectl:
    return "cachectl";
    break;
  case SYS_fchown:
    return "fchown";
    break;
  case SYS_fchmod:
    return "fchmod";
    break;
  case SYS_socketpair:
    return "socketpair";
    break;
  case SYS_sysinfo:
    return "sysinfo";
    break;
  case SYS_nuname:
    return "nuname";
    break;
  case SYS_xstat:
    return "xstat";
    break;
  case SYS_lxstat:
    return "lxstat";
    break;
  case SYS_fxstat:
    return "fxstat";
    break;
  case SYS_xmknod:
    return "xmknod";
    break;
  case SYS_ksigaction:
    return "ksigaction";
    break;
  case SYS_sigpending:
    return "sigpending";
    break;
  case SYS_sigprocmask:
    return "sigprocmask";
    break;
  case SYS_sigsuspend:
    return "sigsuspend";
    break;
  case SYS_sigpoll:
    return "sigpoll";
    break;
  case SYS_swapctl:
    return "swapctl";
    break;
  case SYS_getcontext:
    return "getcontext";
    break;
  case SYS_setcontext:
    return "setcontext";
    break;
  case SYS_waitsys:
    return "waitsys";
    break;
  case SYS_sigstack:
    return "sigstack";
    break;
  case SYS_sigaltstack:
    return "sigaltstack";
    break;
  case SYS_sigsendset:
    return "sigsendset";
    break;
  case SYS_statvfs:
    return "statvfs";
    break;
  case SYS_fstatvfs:
    return "fstatvfs";
    break;
  case SYS_getpmsg:
    return "getpmsg";
    break;
  case SYS_putpmsg:
    return "putpmsg";
    break;
  case SYS_lchown:
    return "lchown";
    break;
  case SYS_priocntl:
    return "priocntl";
    break;
  case SYS_ksigqueue:
    return "ksigqueue";
    break;
  case SYS_backdoor:
    return "-backdoor-";
    break;
  default:
    return "-unknown-";
    break;
  }
  return "-hmm-";
}


void SysCall::PrintError (char *s, ...)
{
  va_list ap;

  fflush (stdout);
  printf ("*syscall* [pc:0x%08x] num=%d %s(): ", (int)pc, (int)GetReg (REG_V0),
	  SysCallName (GetReg(REG_V0)));

  va_start (ap, s);
  vfprintf (stdout, s, ap);
  va_end (ap);
  printf ("\n");
  fflush (stdout);
}

#define READ_FNAME(addr)					\
    i = 0;							\
    do {							\
      c = ReadByte (addr+i);					\
      buf[i] = c;						\
      i++;							\
      if (i == 1023) {						\
	PrintError ("input filename exceeds 1023 chars!");	\
	buf[1023] = 0;						\
	break;							\
      }								\
    } while (c != 0);


void SysCall::EmulateSysCall (void)
{
  static char buf[1024];
  int i;
  char c;
  int x, y;
  static int depth = 0;
  LL curaddr;

  quit = 0;

  //  printf("v0: %d, a0: %x, a1: %x, a2: %x, a3: %x\n",
  //	 (int)GetReg(REG_V0), (int)GetReg(REG_A0), (int)GetReg(REG_A1),
  //	 (int)GetReg(REG_A2), (int)GetReg(REG_A3));
 
  switch (GetReg (REG_V0)) {

  case SYS_exit:		// exit (a0)
    printf ("EXIT called, code: %d\n", GetReg(REG_A0));
    quit = 1;
    break;

  case SYS_read:		// read (fd, addr, len)
   { 
   
    char * buffer_temp; 
    if (_fdlist[0xff & GetReg (REG_A0)] < 0) {
      PrintError ("invalid fd (app=%d, real=%d)", 
		  (int)GetReg(REG_A0),
		  _fdlist[0xff & GetReg (REG_A0)]);
      UNIX_RETURN (MINUS_ONE);
    }

    if (GetReg(REG_A2) > 0) {
      if (_fdlist[0xff & GetReg (REG_A0)] == 0) {

	x = read (_fdlist[0xff & GetReg(REG_A0)], &c, 1);
	if (x == 1) {
	  WriteByte (GetReg(REG_A1), c);
	  UNIX_RETURN (1ULL);
	}
	else {
	  UNIX_RETURN (x);
	}
	break;
      }
      else {
	if (GetReg (REG_A2) > 1024) {
          buffer_temp = (char * )malloc(GetReg (REG_A2));
           x = read (_fdlist[0xff & GetReg(REG_A0)], buffer_temp,GetReg (REG_A2));
	}
	else {
	  x = read (_fdlist[0xff & GetReg(REG_A0)], buf, (int)GetReg (REG_A2));
	}
	if (x < 0) {
	  UNIX_RETURN (x);
	  break;
	}
        
        if(GetReg (REG_A2) > 1024)
         {
           for (i=0; i < x; i++)
              WriteByte (GetReg (REG_A1)+i, buffer_temp[i]);
           free(buffer_temp);
         }
	else
           for (i=0; i < x; i++)
              WriteByte (GetReg (REG_A1)+i, buf[i]);
        UNIX_RETURN (x);
	break;
      }
    }
    UNIX_RETURN (0);
    break;
   }
 
  case SYS_write:		// write (fd, addr, len)
    {
     if (_fdlist[0xff & GetReg(REG_A0)] < 0) {
      PrintError ("invalid fd (app=%d, real=%d)", 
		  (int)GetReg(REG_A0),
		  _fdlist[0xff & GetReg (REG_A0)]);
      UNIX_RETURN (MINUS_ONE);
     }
     for (i=0; i < GetReg(REG_A2); i++) {
      c = ReadByte (GetReg(REG_A1)+i);
      x = write (_fdlist[0xff & GetReg(REG_A0)], &c, 1);
      if (x != 1) {
	if (x == 0) {
	  UNIX_RETURN (i+1);
        }
	UNIX_RETURN (MINUS_ONE);
      }
     }
     UNIX_RETURN (GetReg(REG_A2));
     break; 
   }

  case SYS_open:		// open (file, type)
    READ_FNAME (GetReg(REG_A0));
    {
      int fd;

      for (i=0; i < 256; i++)
	if (_fdlist[i] == -1)
	  break;
      if (i == 256)
	UNIX_RETURN(MINUS_ONE);
      _fdlist[i] = open (buf, mode_remap ((int)GetReg(REG_A1)), (int)GetReg(REG_A2));
      if (_fdlist[i] < 0) {
	PrintError ("error opening file `%s'", buf);
	UNIX_RETURN (MINUS_ONE);
      }
      else {
#if 0
        printf("syscall: fd is %d\n",i);
#endif
	UNIX_RETURN(i);
      }
    }
    break;
  case SYS_close:		// close (fd)
  {
   int tmpfd = _fdlist[0xff & GetReg(REG_A0)];
   _fdlist[0xff & GetReg(REG_A0)] = -1;
   UNIX_RETURN (close (tmpfd));
   break; 
  }

  case SYS_creat:		// creat (file, int)
    READ_FNAME(GetReg(REG_A0));
    for (i=0; i < 256; i++) 
      if (_fdlist[i] == -1)
	break;
    if (i == 256)
      UNIX_RETURN (MINUS_ONE);
    _fdlist[i] = creat (buf, GetReg(REG_A1));
#if 0
        printf("syscall: fd is %d\n",i);
#endif
    UNIX_RETURN (i);
    break;

  case SYS_ioctl:		// ioctl (fd, request, ...)
    // XXX: fixme. This needs to translate the ioctl from 
    // irix5.3 to local host
    //printf ("pc: %x, ioctl, r4=%x, r5=%x, r6=%x\n",
    //pc,
    //GetReg (REG_A0), GetReg(REG_A1), GetReg(REG_A2));
    if (_fdlist[0xff & GetReg(REG_A0)] < 0) {
      PrintError ("invalid fd (app=%d, real=%d)", 
		  (int)GetReg(REG_A0),
		  _fdlist[0xff & GetReg (REG_A0)]);
      UNIX_RETURN (MINUS_ONE);
    }
    switch (GetReg (REG_A1)) {
    case SIM_TCGETA:
      break;
    default:
      PrintError ("fd (app=%d, real=%d), cmd=%d (%x)", 
		  (int)GetReg(REG_A0),
		  _fdlist[0xff & GetReg (REG_A0)],
		  (int)GetReg (REG_A1), (int)GetReg (REG_A1));
      break;
    }
    UNIX_RETURN (0);
    break;

  case SYS_xstat:
    READ_FNAME(GetReg(REG_A1));
    {
      int fd;

      fd = open (buf, O_RDONLY);
      if (fd < 0) {
	PrintError ("error opening file `%s'", buf);
	UNIX_RETURN (MINUS_ONE);
      }
      else {
	i = _emulate_fxstat (fd, GetReg(REG_A2));
	close (fd);
	UNIX_RETURN(i);
      }
    }
    break;

  case SYS_access:
    READ_FNAME (GetReg (REG_A0));
    UNIX_RETURN(access (buf, GetReg(REG_A1)));
    break;

  case SYS_fxstat:		// fstat with a twist...
    UNIX_RETURN (_emulate_fxstat (_fdlist[0xff & GetReg(REG_A1)], GetReg(REG_A2)));
    break;

  case SYS_prctl:		// prctl
    if (GetReg(REG_A0) == 14) {
      UNIX_RETURN (0);
    }
    else {
      PrintError ("code %d unimplemented", (int)GetReg (REG_A0));
    }
    break;

  case SYS_brk:			// brk()
    // since we have no memory management, just return ok! :)
    //SetReg (REG_V0,0);
    //printf ("request for %x bytes\n", (unsigned int)GetReg (REG_A0));
    UNIX_RETURN (0);
    break;

  case SYS_BSD_getime:
    UNIX_RETURN (_emulate_gettime (GetReg(REG_A0), GetReg(REG_A1)));
    break;

  case SYS_syscall:
    // unbelivably, this is a syscall!
    if (depth)
      fatal_error ("Recursive call to SYS_syscall???");
    {
      LL a0, a1, a2, a3;
      a0 = GetReg (REG_A0);
      a1 = GetReg (REG_A1);
      a2 = GetReg (REG_A2);
      a3 = GetReg (REG_A3);
      SetReg (REG_V0, a0);
      SetReg (REG_A0, a1);
      SetReg (REG_A1, a2);
      SetReg (REG_A2, a3);
      // wow!
      depth++;
      EmulateSysCall ();
      depth--;
      SetReg (REG_A0, a0);
      SetReg (REG_A1, a1);
      SetReg (REG_A2, a2);
      return;
    }      
    break;

  case SYS_sysmp: // sysmp (int cmd, ...)
    
    switch (GetReg (REG_A0)) {
    case SIM_MP_PGSIZE:
      UNIX_RETURN (4096);
      break;

    default:
      PrintError ("unimpl cmd=%d", (int)GetReg (REG_A0));
      UNIX_RETURN (MINUS_ONE);
      break;
    }

  case SYS_ksigaction:
    // no one knows what this does...
    PrintError ("unimpl");
    UNIX_RETURN (0);
    break;

  case SYS_lseek:
  
    if (_fdlist[0xff & GetReg (REG_A0)] < 0) {
      PrintError ("invalid fd (app=%d, real=%d)", 
		  (int)GetReg(REG_A0),
		  _fdlist[0xff & GetReg (REG_A0)]);
      UNIX_RETURN (MINUS_ONE);
    }
    else {
      UNIX_RETURN (lseek (_fdlist[0xff & GetReg (REG_A0)],
			  GetReg (REG_A1),
			  GetReg (REG_A2)));
    }
    break;

  case SYS_time:
    {
      time_t x, y;
      
      x = time (&y);
      
      if (GetReg (REG_A0) != 0) {
	WriteWord (GetReg (REG_A0), (int)y);
      }
      UNIX_RETURN (x);
    }
    break;

  case SYS_syssgi:
    switch (GetReg (REG_A0)) {

    default:
      PrintError ("unimpl, cmd=%d", (int)GetReg (REG_A0));
      break;
    }
    break;

  case SYS_getpid:
    UNIX_RETURN (getpid());
    break;

  case SYS_kill:
    UNIX_RETURN (kill ((int)GetReg (REG_A0), (int)GetReg(REG_A1)));
    break;

  case SYS_backdoor:
    /*
     * simulator backdoor calls go here...
     */
    switch (GetReg (REG_A0)) {
    case BackDoor_SetArgs:
      if ((_argc > 0) && (GetCpuId()==0)) {
	/* we have space to store args starting at _argaddr */

	/* 1. adjust $sp, keeping it double-word aligned */
	SetReg (REG_SP, GetReg (REG_SP) - 4*(_argc + 1 + !(_argc & 1)));

	/* write argc */
	WriteWord (GetReg (REG_SP), _argc);

	/* write argv */
	curaddr = _argaddr;
	for (i=0; i < _argc; i++) {
	  WriteWord (GetReg (REG_SP) + (4+4*i), curaddr);
	  y = strlen (_argv[i]);
	  for (x=0; x <= y; x++) {
	    WriteByte (curaddr, _argv[i][x]);
	    curaddr++;
	  }
	}
      }
      UNIX_RETURN (0);
      break;
    case BackDoor_Delay :
      PAUSE(GetReg(REG_A1)*2000);  //us
      UNIX_RETURN (0);
      break;
    case BackDoor_SimTime:
      /* return time in register $a0 */
      SetReg (REG_A0, GetTime());
      UNIX_RETURN (0);
      break;
    case BackDoor_CpuId:
      SetReg (REG_A0, GetCpuId());
      UNIX_RETURN (0);
      break;
    case BackDoor_ResetStats: 
      ResetCall();
      UNIX_RETURN (0);
      break;
    case BackDoor_PlaceRange:
      PlaceRangeCall((unsigned)GetReg(REG_A1),(unsigned)GetReg(REG_A2),
                                              (unsigned)GetReg(REG_A3));
      UNIX_RETURN (0);
      break;
    case BackDoor_ActiveProcs:
      UNIX_RETURN (0);
      break;
    case BackDoor_Create:
      CreateCall((unsigned)GetReg(REG_A1),(unsigned)GetReg(REG_A2));
      UNIX_RETURN (0);
      break;
    case BackDoor_WaitForEnd:
      WaitCall((unsigned)GetReg(REG_A1));
      UNIX_RETURN (0);
      break;
    case BackDoor_IOMap:
      UNIX_RETURN (0);
      break;
    case BackDoor_IOMapGeneral:
      UNIX_RETURN (0);
      break;
    case BackDoor_MemoryMap:
      UNIX_RETURN (0);
      break;
    case BackDoor_GetPhysAddr:
      UNIX_RETURN (0);
      break;
    case BackDoor_GetNodeNum:
      UNIX_RETURN (0);
      break;
    case BackDoor_HitWBInvalD:
      UNIX_RETURN (0);
      break;
    case BackDoor_HitWBInvalS:
      UNIX_RETURN (0);
      break;
    case BackDoor_IndexWBInvalD:
      UNIX_RETURN (0);
      break;
    case BackDoor_IndexWBInvalS:
      UNIX_RETURN (0);
      break;
    case BackDoor_FlushD:
       UNIX_RETURN (0);
       break;
    case BackDoor_FlushS:
       UNIX_RETURN (0);
       break;
    case BackDoor_PageFlushD:
       UNIX_RETURN (0);
       break;
    case BackDoor_PageFlushS:
       UNIX_RETURN (0);
       break;
    default:
      PrintError ("Unknown backdoor call %d\n", (int)GetReg(REG_A0));
      break;
    }
    UNIX_RETURN (0);
    break;

  default:
    PrintError ("Unimplemented system call");
    UNIX_RETURN (0);
    break;
  }
}


/*************************************************************************
 *
 *
 *  Translate mips-sgi-irix structures to/from host structures
 *
 *
 *************************************************************************
 */

/* mode types for open() */

unsigned int mode_remap (unsigned int x)
{
  unsigned int res = 0;
 
#define FIXUP(v)   do  { if (x & SIM_O_##v) res = res |O_##v; x &= ~SIM_O_##v; } while (0)
#define DONT_HAVE(v) if (x & SIM_O_##v) { printf ("skipping flag: " #v); }
  
  FIXUP(RDONLY);
  FIXUP(WRONLY);
  FIXUP(RDWR);
  FIXUP(NDELAY);
  FIXUP(APPEND);
  DONT_HAVE(SYNC);
  FIXUP(NONBLOCK);
  DONT_HAVE(DIRECT);
  FIXUP(CREAT);
  FIXUP(TRUNC);
#undef FIXUP
#undef DONT_HAVE

  if (x != 0) {
    printf ("mode_remap: could not properly emulate all flags...\n");
  }
  return res;
}

LL SysCall::_emulate_gettime(LL ts_addr, LL tz_addr)
{
  struct timeval tv;
  mips_timestruc_t mtv;

  long x, i;

  i = gettimeofday (&tv, 0);

#define WORD_COPY(field,lfield)  do {                                        \
			    x = (long)&mtv.field - (long)&mtv;               \
			    Assert (sizeof (mtv.field) == sizeof(tv.lfield), \
			            "Oh my god. You're dead");               \
			    Assert (sizeof (mtv.field) == 4,                 \
			            "Oh my god. You're dead");               \
			    WriteWord (ts_addr+x, tv.lfield);              \
			  } while (0)

  WORD_COPY (tv_sec,tv_sec);
  WORD_COPY (tv_nsec,tv_sec);
#undef WORD_COPY
  return i;
}


/*
 *
 *  fstat call
 *
 */
LL SysCall::_emulate_fxstat (int fd, LL addr)
{
  struct stat sb;
  struct mips_stat msb;
  long x, i;

  i = fstat (fd, &sb);
#if 0
  printf("mode is 0x%x\n",sb.st_mode);
  printf("kkk\n");
  printf("dev_t is %d\n",sizeof(dev_t));
  printf("ino_t is %d\n",sizeof(ino_t));
  printf("mode_t  is %d\n",sizeof(mode_t));
  printf("sb mode_t  is %d\n",sizeof(sb.st_mode));
  printf("msb mode_t  is %d\n",sizeof(msb.st_mode));
  printf("nlink_t  is %d\n",sizeof(nlink_t));
  printf("uid_t is %d\n",sizeof(uid_t));
  printf("gid_t is %d\n",sizeof(gid_t));
  printf("off_t is %d\n",sizeof(off_t));
#endif

#define DWORD_COPY(field,lfield)  do {                                        \
	                    x = (long)&msb.field - (long)&msb;               \
			    Assert (sizeof (msb.field) == sizeof(sb.lfield), \
			            "Oh my god. You're dead");               \
			    Assert (sizeof (msb.field) == 8,                 \
			            "Oh my god. You're dead");               \
			    WriteWord (addr+x, (int)sb.lfield);              \
			    WriteWord (addr+x+4, *(int*)(&sb.lfield+4)); \
			  } while (0)

#define WORD_COPY(field,lfield)  do {                                        \
			    x = (long)&msb.field - (long)&msb;               \
			    Assert (sizeof (msb.field) == sizeof(sb.lfield), \
			            "Oh my god. You're dead");               \
			    Assert (sizeof (msb.field) == 4,                 \
			            "Oh my god. You're dead");               \
			    WriteWord (addr+x, sb.lfield);              \
			  } while (0)

#define HALF_COPY(field,lfield)  do {                                        \
			    x = (long)&msb.field - (long)&msb;               \
			    Assert (sizeof (msb.field) == sizeof(sb.lfield), \
			            "Oh my god. You're dead");               \
			    Assert (sizeof (msb.field) == 2,                 \
			            "Oh my god. You're dead");               \
			    WriteHalf (addr+x, sb.lfield);           \
			  } while (0)

#define H_TO_WORD_COPY(field,lfield)  do {                           \
                        x = (long)&msb.field - (long)&msb;               \
                        Assert (sizeof (msb.field) == 2*sizeof(sb.lfield), \
                                 "Oh my god. You're dead");               \
                        Assert (sizeof (msb.field) == 4,                 \
                                  "Oh my god. You're dead");               \
                        WriteWord(addr+x, sb.lfield);           \
                      } while (0)


#define D_TO_WORD_COPY(field,lfield)  do {                                   \
			    x = (long)&msb.field - (long)&msb;               \
			    Assert (2*sizeof (msb.field)== sizeof(sb.lfield),\
			            "Oh my god. You're dead");               \
			    Assert (sizeof (msb.field) == 4,                 \
			            "Oh my god. You're dead");               \
			    WriteWord (addr+x, (int)sb.lfield);        \
			  } while (0)

#define D_TO_HALF_COPY(field,lfield)  do {                         \
                            x = (long)&msb.field - (long)&msb;               \
                            Assert (4*sizeof (msb.field)== sizeof(sb.lfield),\
                                    "Oh my god. You're dead");               \
                            Assert (sizeof (msb.field) == 2,                 \
                                    "Oh my god. You're dead");               \
                            WriteWord (addr+x, (short)sb.lfield);  \
                          } while (0)

#define W_TO_HALF_COPY(field,lfield)  do {                                   \
			    x = (long)&msb.field - (long)&msb;               \
			    Assert (2*sizeof (msb.field)== sizeof(sb.lfield),\
			            "Oh my god. You're dead");               \
			    Assert (sizeof (msb.field) == 2,                 \
			            "Oh my god. You're dead");               \
			    WriteHalf (addr+x, (short)sb.lfield);        \
			  } while (0)

#if defined(__i386__) && defined (__FreeBSD__)
  WORD_COPY (st_dev,st_dev);
  WORD_COPY (st_ino,st_ino);
  HALF_COPY (st_mode,st_mode);
  HALF_COPY (st_nlink,st_nlink);
  WORD_COPY (st_uid,st_uid);
  WORD_COPY (st_gid,st_gid);
  WORD_COPY (st_rdev,st_rdev);
  D_TO_WORD_COPY (st_size,st_size);
  D_TO_WORD_COPY (st_blocks,st_blocks);
  WORD_COPY (st_blksize,st_blksize);
#elif defined(__i386__) && defined(__linux__)
//  printf("Linux is here!\n");
//  DWORD_COPY (st_dev,st_dev);
  D_TO_WORD_COPY (st_dev,st_dev);
  WORD_COPY (st_ino,st_ino);
  W_TO_HALF_COPY (st_mode,st_mode);
  W_TO_HALF_COPY (st_nlink,st_nlink);
  WORD_COPY (st_uid,st_uid);
  WORD_COPY (st_gid,st_gid);
  D_TO_WORD_COPY (st_rdev,st_rdev);
  WORD_COPY (st_size,st_size);
  WORD_COPY (st_blocks,st_blocks);
  WORD_COPY (st_blksize,st_blksize);
#elif defined(__ia64__) && defined (__linux__)
  D_TO_WORD_COPY (st_dev,st_dev);
  D_TO_WORD_COPY (st_ino,st_ino);
  W_TO_HALF_COPY (st_mode,st_mode);
  D_TO_HALF_COPY (st_nlink,st_nlink);
  WORD_COPY (st_uid,st_uid);
  WORD_COPY (st_gid,st_gid);
  D_TO_WORD_COPY (st_rdev,st_rdev);
  D_TO_WORD_COPY (st_size,st_size);
  D_TO_WORD_COPY (st_blocks,st_blocks);
  D_TO_WORD_COPY (st_blksize,st_blksize);
#else
#error Unknown machine/OS combination
#endif

#if 0
  printf("st_size offset is %llu\n",(long)&msb.st_size - (long)&msb);
  printf("file size is %d\n",sb.st_size);
#endif
#if defined(__i386__) && defined (__FreeBSD__)
  WORD_COPY (st_atimespec.tv_sec,st_atimespec.tv_sec);
  WORD_COPY (st_atimespec.tv_nsec,st_atimespec.tv_nsec);
#elif defined(__i386__) && defined (__linux__)
  WORD_COPY (st_atimespec.tv_sec,st_atime);
#elif defined(__ia64__) && defined (__linux__)
  D_TO_WORD_COPY (st_atimespec.tv_sec,st_atim.tv_sec);
  D_TO_WORD_COPY (st_atimespec.tv_nsec,st_atim.tv_nsec);
#else
#error Unknown machine/OS combination
#endif
   
#undef DWORD_COPY 
#undef WORD_COPY
#undef HALF_COPY
#undef D_TO_WORD_COPY

  return i;
}


