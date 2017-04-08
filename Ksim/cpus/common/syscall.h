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
 *  $Id: syscall.h,v 1.2 2007/11/23 19:58:22 mainakc Exp $
 *
 *************************************************************************/
#ifndef __MY_SYSCALL_H__
#define __MY_SYSCALL_H__

#include "mem.h"
#include "app_syscall.h"

#define REG_V0 2
#define REG_V1 3

#define REG_A0 4
#define REG_A1 5
#define REG_A2 6
#define REG_A3 7
#define REG_T0 8
#define REG_T1 9

#define REG_GP 28
#define REG_SP 29

typedef struct filelog_s {
   unsigned v0;
   unsigned a0;
   unsigned a1;
   unsigned a2;
   unsigned length;
   char *buf;
   struct filelog_s *next;
} filelog_t;

class Mipc;

class SysCall {
 public:
  SysCall ();
  SysCall (Mipc *ms);
  ~SysCall ();

  void Reset (void);

  /* 
   * Call this for argument setup backdoor call
   *
   *   - argc, argv : arguments
   *   - addr       : address in simulated memory where the
   *                  argv array can be stored.
   *
   */
  void ArgumentSetup (int argc, char **argv, LL addr, char *input_file=NULL, unsigned thread_id=0);

  virtual LL GetDWord (LL addr) = 0;
  virtual void SetDWord (LL addr, LL value) = 0;

  virtual Word GetWord (LL addr) = 0;
  virtual void SetWord (LL addr, Word value) = 0;
  
  virtual void SetReg (int regnum, LL value) = 0;
  virtual LL   GetReg (int regnum) = 0;

  inline void WriteByte (LL addr, char byte) {
    LL value;
    
    value = GetDWord (addr);
    value = m->BESetByte (addr, value, byte);
    SetDWord (addr, value);
  };

  inline char ReadByte (LL addr) {
    LL value;

    value = GetDWord (addr);
    return m->BEGetByte (addr, value);
  };

  inline void WriteWord (LL addr, Word w) {
    SetWord (addr, w);
  };

  inline Word ReadWord (LL addr) {
    return GetWord (addr);
  };

  inline void WriteHalf (LL addr, Word w) {
    LL value;

    value = GetDWord (addr);
    value = m->BESetHalfWord (addr, value, w);
    SetDWord (addr, value);
  };

  inline Word ReadHalf (LL addr) {
    LL value;

    value = GetDWord (addr);
    return m->BEGetHalfWord (addr, value);
  };

  virtual void EmulateSysCall (void);

  virtual void PrintError (char *s, ...);
  virtual char *SysCallName (int num);

  virtual LL GetTime (void) { return 0; }
  virtual int GetCpuId (void) { return 0; }
  
  virtual void CreateCall(unsigned,unsigned) {}	//Must be overridden
  virtual void WaitCall(unsigned) {}	//Must be overridden
  virtual void ResetCall(void) {}	//Must be overridden
  virtual void PlaceRangeCall(unsigned,unsigned,unsigned) {} //Must be overridden
  int quit;

  LL pc;

  LL _num_load;
  LL _num_store;
  LL _num_load_since_reset;
  LL _num_store_since_reset;

 protected:
  Mem *m;
  int _fdlist[256];

  LL _emulate_fxstat (int fd, LL addr);
  LL _emulate_gettime(LL ts_addr, LL tz_addr);

  int _argc;
  char **_argv;
  LL _argaddr;

  FILE *_fpstate, *_fpwrite;
  char *_fdlogname, _fdwritename[128];
  filelog_t *_filelog, *_filelogtail;
};

#endif /* __MY_SYSCALL_H__ */

