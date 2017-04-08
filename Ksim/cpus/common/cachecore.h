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
 *  $Id: cachecore.h,v 1.1.1.1 2006/05/23 13:53:58 mainakc Exp $
 *
 *************************************************************************/
#ifndef __CACHECORE_H__
#define __CACHECORE_H__

#include "sim.h"
#include "mem.h"

struct CacheLine {
  unsigned int dirty:2;
  unsigned int valid:1;
#ifdef DIRTY_SHARING
  unsigned int owned:1;
#endif
  unsigned int lru;		// lru bits, enjoy
  int way;			// which way
  LL tag;
  Byte *b;			// data
};

/*
 * Assumption: a cache line >= sizeof(LL) (LL == DWord)
 *
 * Cache stored in host format, whatever that may be (BIG/LITTLE)
 *
 */

#define DIRTY(c)  (c)->dirty
#define VALID(c)  (c)->valid
#ifdef DIRTY_SHARING
#define OWNED(c)  (c)->owned
#endif

class CacheCore {
public:
  // name.params are read from config file
  CacheCore (char *name);
  CacheCore (int nlines, int nbanks, int nsets, int nbytes);
  ~CacheCore ();

  // get cache line(s) corresponding to address. No check made re:
  // tags, etc.
  inline CacheLine *GetLines (LL addr) {
    return &_c[_nsets*((addr & _idxmask) >> _idxsa)];
  };

  inline CacheLine *GetLinesFromIndex (int index) {
    return &_c[_nsets*index];
  };

  inline int LineNumber (LL addr) {
    return (int)((addr & _idxmask) >> _idxsa);
  };

  inline int BankNumber (LL addr) {
    return (int)((addr & _idxmask) >> _banksa);
  };

  inline int NumBanks (void) {
    return _nbanks;
  };

  inline int NumLines (void) {
    return _nlines;
  };

  inline int NumSets (void) {
    return _nsets;
  };

  inline int GetLineSize(void) {
    return _nbytes;
  };

  //  lookup address in cache, check tags. 
  //  return NULL if no match, or the cache line if there is a match.


  CacheLine *GetLine (LL addr);

  inline int HitTest (LL addr) {
    return GetLine(addr) == NULL ? 0 : 1;
  };

  // get data corresponding to address in the cache line

  inline Byte GetByte (CacheLine *c, LL addr) {
    addr ^= _endianness;
    return c->b[addr & _bytemask];
  };

  inline Word GetHalfWord (CacheLine *c, LL addr) {
    addr ^= _endianness;
    return 0xffff & (*((Word *)&c->b[addr & _hwordmask]));
  };

  inline Word GetWord (CacheLine *c, LL addr) {
    addr ^= _endianness;
    return *((Word *)&c->b[addr & _wordmask]);
  };

  inline LL GetDWord (CacheLine *c, LL addr) {
    addr ^= _endianness;
    return *((LL *)&c->b[addr & _dwordmask]);
  };

  inline int CacheLineStart (LL addr) {
    return (addr & _bytemask) == 0;
  }

  // set data corresponding to address in cache line

  inline void SetByte (CacheLine *c, LL addr, Byte data) {
    addr ^= _endianness;
    c->b[addr & _bytemask] = data;
  };

  inline void SetHalfWord (CacheLine *c, LL addr, Word data) {
    addr ^= _endianness;
#if BYTE_ORDER == BIG_ENDIAN
    c->b[((addr & _bytemask) >> 1) << 1] = data >> 8;
    c->b[1+(((addr & _bytemask) >> 1)<<1)] = data & 0xff;
#else
    c->b[((addr & _bytemask) >> 1)<<1] = data & 0xff;
    c->b[1+(((addr & _bytemask) >> 1)<<1)] = data >> 8;
#endif
  };

  inline void SetWord (CacheLine *c, LL addr, Word data) {
    addr ^= _endianness;
    *((Word*)&c->b[addr & _wordmask]) = data;
  };

  inline void SetDWord (CacheLine *c, LL addr, LL data) {
    addr ^= _endianness;
    *((LL*)&c->b[addr & _dwordmask]) = data;
  };

  // set tags for "c" to tags corresponding to "addr"
  //  side-effect: makes line valid

  inline void SetTags (CacheLine *c, LL addr) {
    c->tag = (addr & _tagsmask) >> _lgdata;
    c->valid = 1;
  };

  // invalidate all cache lines!
  void Invalidate (void);

  // # of sets in cache
  int nsets;

  // # of LL's in each cache line
  int nDWords;

  // address of start of cache line
  inline LL GetBaseAddr (LL addr) { return addr & ~_bytemask; };

  // address of current cached data

  inline LL GetLineAddr (CacheLine *c, LL addr) {
    return (c->tag << _lgdata) | (addr&_idxmask&~_bytemask);
  };

  inline LL GetTagsMask (void) {
    return _tagsmask;
  };

  inline int GetLgData (void) {
    return _lgdata;
  };

  inline LL GetByteMask (void) {
    return _bytemask;
  }

  inline int GetIndSA (void) {
     return _idxsa;
  }

  // register default parameter values
  static void RegisterDefault (char *name);

private:
  int _nbanks;			// # of cache banks
  int _nlines;			// # of cache lines
  int _nsets;			// # of sets
  int _nbytes;			// # of bytes in each cache line

  int _lgdata;			// lg (lines*bytes)
  LL  _tagsmask;		// mask for tags
  LL  _idxmask;			// mask for index
  int _idxsa;			// idx shift amount

  int _banksa;			// bank # shift amount

  LL  _bytemask;		// mask for byte index
  LL  _hwordmask;		// mask for half-word index
  LL  _wordmask;		// mask for word index
  LL  _dwordmask;		// mask for double-word index
  LL  _endianness;		// xor mask

  // cache line array (# lines * # sets). 
  // Storage:
  //  line0,set0  line0,set1, ... line0, setN-1, line1,set0, ... etc

  CacheLine *_c;

  char *_name;
  
};

#endif /* __CACHECORE_H__ */
