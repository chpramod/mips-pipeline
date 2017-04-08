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
 *  $Id: mem.h,v 1.1.1.1 2006/05/23 13:53:59 mainakc Exp $
 *
 *************************************************************************/
#ifndef __MEM_H__
#define __MEM_H__

#include <stdio.h>
#include <stdlib.h>
#include "machine.h"

/*
 * Physical memory: Assumes big-endian format!
 */

typedef unsigned long long LL;
typedef unsigned char Byte;
typedef unsigned int Word;

#define MEM_BAD  0x0ULL

#define MEM_ALIGN 3 /* bottom three bits zero */

#ifndef MIN
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#endif

class Mem {
public:
  Mem();
  ~Mem();

  int ReadImage (char *s);
  int ReadImage (FILE *fp);	// load fresh memory image
  int ReadImageAndDuplicate (FILE *fp, Mem*);
  int MergeImage (char *s);
  int MergeImage (FILE *fp);	// merge image from file with current mem
  int MergeImageAndDuplicate (FILE *fp, Mem*);
  void DumpImage (FILE *fp);	// write current mem to file
  
  LL Read (LL addr);
  void Write (LL addr, LL val);
  
  Word BEReadWord (LL addr) {
    return BEGetWord (addr, Read (addr));
  }
    
  void BEWriteWord (LL addr, Word val) {
    Write (addr, BESetWord (addr, Read (addr), val));
  }

  Word LEReadWord (LL addr) {
    return LEGetWord (addr, Read (addr));
  }
    
  void LEWriteWord (LL addr, Word val) {
    Write (addr, LESetWord (addr, Read (addr), val));
  }

  inline Byte BEGetByte (LL addr, LL data) {
    return (data >> ((~addr & 7)<<3)) & 0xff;
  };
  inline Byte LEGetByte (LL addr, LL data) {
    return (data >> ((addr & 7)<<3)) & 0xff;
  };

  inline Word BEGetHalfWord (LL addr, LL data) {
    return (data >> ((~addr & 6)<<3)) & 0xffff;
  };
  inline Word LEGetHalfWord (LL addr, LL data) {
    return (data >> ((addr & 6)<<3)) & 0xffff;
  };

  inline Word BEGetWord (LL addr, LL data) {
    return (data >> ((~addr & 4)<<3)) & 0xffffffff;
  };
  inline Word LEGetWord (LL addr, LL data) {
    return (data >> ((addr & 4)<<3)) & 0xffffffff;
  };

  inline LL BESetByte (LL addr, LL data, Byte val) {
    return ((LL)val<<((~addr & 7)<<3))|(data&~((LL)0xff<<((~addr & 7)<<3)));
  };
  inline LL LESetByte (LL addr, LL data, Byte val) {
    return ((LL)val<<((addr & 7)<<3))|(data&~((LL)0xff<<((addr & 7)<<3)));
  };


  inline LL BESetHalfWord (LL addr, LL data, Word val) {
    return ((LL)val<<((~addr & 6)<<3))|(data&~((LL)0xffff<<((~addr & 6)<<3)));
  };
  inline LL LESetHalfWord (LL addr, LL data, Word val) {
    return ((LL)val<<((addr & 6)<<3))|(data&~((LL)0xffff<<((addr & 6)<<3)));
  };

  inline LL BESetWord (LL addr, LL data, Word val) {
    return ((LL)val<<((~addr & 4)<<3))|(data&~((LL)0xffffffff<<((~addr & 4)<<3)));
  };
  inline LL LESetWord (LL addr, LL data, Word val) {
    return ((LL)val<<((addr & 4)<<3))|(data&~((LL)0xffffffff<<((addr & 4)<<3)));
  };

  void Clear (void) { freemem (); }

  int Compare (Mem *m, int verbose = 1);

private:
  int _chunks;
  int _numchunks;
  int _last_chunk_collect;

  // array of blocks of memory
  LL **_mem;

  // length of each block
  LL *_mem_len;

  // size of each block
  // LL *mem_sz;

  // start address for each block
  LL *_mem_addr;

  void freemem (void);
  void get_more (int chunkid, long long num, int merging = 0);
  int  create_new_chunk (LL addr, int ch);
  void merge_chunks (int chunk1, int chunk2);
  void chunk_gc (int force);

  inline int _binsearch (LL addr);
  int _binsearchw (LL addr);

  // Optimize for locality
  int _lastWriteRow, _lastReadRow;
};
  

#endif /* __MEM_H__ */
