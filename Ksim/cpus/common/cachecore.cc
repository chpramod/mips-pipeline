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
 *  $Id: cachecore.cc,v 1.1.1.1 2006/05/23 13:53:58 mainakc Exp $
 *
 *************************************************************************/
#include "cachecore.h"


void CacheCore::RegisterDefault (char *name)
{
  char buf[1024];

  sprintf (buf, "%s.Lines", name);
  ::RegisterDefault (buf, 32);

  sprintf (buf, "%s.Sets", name);
  ::RegisterDefault (buf, 1);

  sprintf (buf, "%s.LineSize", name);
  ::RegisterDefault (buf, 128);

  sprintf (buf, "%s.EndianNess", name);
  ::RegisterDefault (buf, "big");

  sprintf (buf, "%s.Banks", name);
  ::RegisterDefault (buf, 1);
}


// private function
static int logbase2 (LL x)
{
  int i = 0;
  LL y;

  y = 1;
  if (y == x) return i;

  for (i=1; x > y; i++) {
    y <<= 1;
    if (y == x) return i;
    if (y == 0) {
      printf ("You're insane\n");
      exit (1);
    }
  }
  return -1;
}



// Create a cache data array
CacheCore::CacheCore (char *name)
{
  char buf[1024];
  int i, j;

  Assert (name && (strlen (name) < 1000), "What the heck are you doing?");

  // generic sanity checks
  Assert (sizeof(Byte) == 1, "Hmm...");
  Assert (sizeof(Word) == 4, "Oh no!");
  Assert (sizeof(LL) == 8, "Yikes");

  _name = Strdup (name);

  sprintf (buf, "%s.Lines", name);
  _nlines = ParamGetInt (buf);

  sprintf (buf, "%s.Banks", name);
  _nbanks = ParamGetInt (buf);
  
  _nlines *= _nbanks;

  sprintf (buf, "%s.Sets", name);
  nsets = _nsets = ParamGetInt (buf);

  sprintf (buf, "%s.LineSize", name);
  _nbytes = ParamGetInt (buf);

  // number of doublewords in each cache line
  nDWords = _nbytes >> 3;

  _lgdata = logbase2 (_nlines*_nbytes);

  // sanity checks
  if (_lgdata < 0)
    fatal_error ("All cache parameters must be powers of 2 [%s]", name);

  // more sanity checks
  if ((_nbytes % 8) != 0)
    fatal_error ("A cache line must be a multiple of a double-word (64 bits)");
    
  _idxmask = (((LL)1 << _lgdata)-1);
  _tagsmask = ~_idxmask;
  _idxsa = logbase2 (_nbytes);
  _banksa = logbase2 (_nbytes*_nlines/_nbanks);

  _bytemask = (((LL)1 << _idxsa)-1);
  _hwordmask = (_bytemask >> 1) << 1;
  _wordmask = (_bytemask >> 2) << 2;
  _dwordmask = (_bytemask >> 3) << 3;

  sprintf (buf, "%s.EndianNess", name);
  if (!ParamGetString (buf) || (strcmp (ParamGetString (buf), "big")) == 0) {
#if BYTE_ORDER == BIG_ENDIAN
    // matches local cpu
    _endianness = 0;
#else
    _endianness = (_bytemask) & 0x7;
#endif
  }
  else if (strcmp (ParamGetString (buf), "little") == 0) {
#if BYTE_ORDER == BIG_ENDIAN
    // matches local cpu
    _endianness = (_bytemask) & 0x7;
#else
    _endianness = 0;
#endif
  }
  else
    fatal_error ("%s.EndianNess: should be `big' or `little'", name);

  _c = (CacheLine *)malloc(sizeof(CacheLine)*_nlines*_nsets);
  if (!_c)
    fatal_error ("Malloc failed, size=%d\n", sizeof(CacheLine)*_nlines*_nsets);

  // clear the cache bits
  for (i=0; i < _nlines*_nsets; i += _nsets) {
    for (j=0; j < _nsets; j++) {
      _c[i+j].valid = 0;
      _c[i+j].dirty = 0;
#ifdef DIRTY_SHARING
      _c[i+j].owned = 0;
#endif
      _c[i+j].lru = 0;
      _c[i+j].way = j;
      MALLOC (_c[i+j].b, Byte, _nbytes);
    }
  }
}

// Create a cache data array
CacheCore::CacheCore (int nlines, int nbanks, int nsets, int nbytes)
{
  int i, j;

  // generic sanity checks
  Assert (sizeof(Byte) == 1, "Hmm...");
  Assert (sizeof(Word) == 4, "Oh no!");
  Assert (sizeof(LL) == 8, "Yikes");

  _name = Strdup ("Cache");
  _nlines = nlines;
  _nbanks = nbanks;
  _nlines *= _nbanks;
  _nsets = nsets;
  _nbytes = nbytes;

  // number of doublewords in each cache line
  nDWords = _nbytes >> 3;

  _lgdata = logbase2 (_nlines*_nbytes);

  // sanity checks
  if (_lgdata < 0)
    fatal_error ("All cache parameters must be powers of 2");

  // more sanity checks
  if ((_nbytes % 8) != 0)
    fatal_error ("A cache line must be a multiple of a double-word (64 bits)");
    
  _idxmask = (((LL)1 << _lgdata)-1);
  _tagsmask = ~_idxmask;
  _idxsa = logbase2 (_nbytes);
  _banksa = logbase2 (_nbytes*_nlines/_nbanks);

  _bytemask = (((LL)1 << _idxsa)-1);
  _hwordmask = (_bytemask >> 1) << 1;
  _wordmask = (_bytemask >> 2) << 2;
  _dwordmask = (_bytemask >> 3) << 3;

#if BYTE_ORDER == BIG_ENDIAN
    // matches local cpu
  _endianness = 0;
#else
  _endianness = (_bytemask) & 0x7;
#endif

  _c = (CacheLine *)malloc(sizeof(CacheLine)*_nlines*_nsets);
  if (!_c)
    fatal_error ("Malloc failed, size=%d\n", sizeof(CacheLine)*_nlines*_nsets);

  // clear the valid bit
  for (i=0; i < _nlines*_nsets; i += _nsets) {
    for (j=0; j < _nsets; j++) {
      _c[i+j].valid = 0;
      _c[i+j].dirty = 0;
#ifdef DIRTY_SHARING
      _c[i+j].owned = 0;
#endif
      _c[i+j].way = j;
      MALLOC (_c[i+j].b, Byte, _nbytes);
    }
  }
}




CacheCore::~CacheCore ()
{
  int i;
  context_disable ();		// will be a NOP for unfair sims
  for (i=0; i < _nlines*_nsets; i++)
    FREE (_c[i].b);
  free (_c);
  free (_name);
  context_enable ();		// more NOPs for unfair sims
}

// Hit test
CacheLine *CacheCore::GetLine (LL addr)
{
  CacheLine *cl;
  int i;
  
  cl = GetLines (addr);
  
  for (i=0; i < _nsets; i++) {
     if (cl[i].valid && (cl[i].tag == ((addr & _tagsmask) >> _lgdata)))
      return &cl[i]; // hit
  }
  // miss
  return NULL;
}


void CacheCore::Invalidate (void)
{
  int i;

  for (i=0; i < _nlines*_nsets; i++)
    _c[i].valid = 0;
}
