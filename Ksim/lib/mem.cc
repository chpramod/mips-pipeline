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
 *  $Id: mem.cc,v 1.2 2007/08/15 06:35:37 mainakc Exp $
 *
 *************************************************************************/
#include <stdio.h>
#include "mem.h"
#include "misc.h"
#include "sim.h"
#include <assert.h>

#ifdef SYNCHRONOUS

#define CHUNK_SIZE 1024		 /*Ocean: 8192*/
/*
#define CHUNK_SIZE 16384*/

#define CHUNK_THRESHOLD 16 /* gc threshold */

static int adj_merge_size = CHUNK_SIZE;

/*
#define ADJ_LIMIT 1024*1024*32
*/

#define ADJ_LIMIT CHUNK_SIZE	 /*Ocean*/ 
/*
#define ADJ_LIMIT 1024*32*/

#else 

#define CHUNK_SIZE 16384
#define CHUNK_THRESHOLD 16 /* gc threshold */
static int adj_merge_size = 16384;
#define ADJ_LIMIT 1024*32

#endif

#define DOUBLE_THRESHOLD (1024*1024*256)

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

int Mem::Compare (Mem *m, int verbose)
{
    int i, j;
    int count = 10;

    chunk_gc (1);
    m->chunk_gc (1);

    if (m->_chunks != _chunks) {
      goto err;
    }
    for (i=0; i < _chunks; i++) {
      if (_mem_addr[i] != m->_mem_addr[i]) goto err;
      if (_mem_len[i] == m->_mem_len[i]) continue;
      if (_mem_len[i] < m->_mem_len[i]) {
	for (j=_mem_len[i]; j < m->_mem_len[i]; j++)
	  if (m->_mem[i][j] != MEM_BAD)
	    goto err;
      }
      else {
	for (j=m->_mem_len[i]; j < _mem_len[i]; j++)
	  if (_mem[i][j] != MEM_BAD)
	    goto err;
      }
    }
    for (i=0; i < _chunks; i++) {
      for (j=0; j < MIN(_mem_len[i],m->_mem_len[i]); j++)
	if (_mem[i][j] != m->_mem[i][j]) {
	  count--;
	  if (count == 0) goto err;
	  if (verbose) {
	    printf ("[0x%08x%08x]  0x%08x%08x  != 0x%08x%08x\n",
		    (unsigned int)(((j+_mem_addr[i])<<MEM_ALIGN) >> 32),
		    (unsigned int)(((j+_mem_addr[i])<<MEM_ALIGN) & 0xffffffff),
		    (unsigned int) (_mem[i][j] >> 32),
		    (unsigned int) (_mem[i][j] & 0xffffffff),
		    (unsigned int) (m->_mem[i][j] >> 32),
		    (unsigned int) (m->_mem[i][j] & 0xffffffff));
	  }
	}
    }
    if (count != 10) goto err;

    return 1;

  err:
    if (verbose) {
      printf ("First mem:\n");
      for (i=0; i < _chunks; i++) {
	printf ("\t*0x%08x%08x %lu\n",
		(unsigned int)((_mem_addr[i]<<MEM_ALIGN) >> 32),
		(unsigned int)((_mem_addr[i]<<MEM_ALIGN) & 0xffffffff),
		_mem_len[i]);
      }
      printf ("Second mem:\n");
      for (i=0; i < m->_chunks; i++) {
	printf ("\t*0x%08x%08x %lu\n",
		(unsigned int)((m->_mem_addr[i]<<MEM_ALIGN) >> 32),
		(unsigned int)((m->_mem_addr[i]<<MEM_ALIGN) & 0xffffffff),
		m->_mem_len[i]);
      }
    }
    return 0;
   }


/*
 * free memory image
 */
void Mem::freemem (void)
{
/*
  int i;

  context_disable ();

  if (_chunks > 0) {
    for (i=0; i < _chunks; i++) {
      if (_mem[i]) 
        FREE (_mem[i]);
    }
    if (_mem) FREE (_mem);
    if (_mem_len) FREE (_mem_len);
    if (_mem_addr) FREE (_mem_addr);
    _mem = NULL;
    _mem_len = NULL;
    _mem_addr = NULL;
    _chunks = 0;
    _numchunks = 0;
    _last_chunk_collect = 0;
  }

  context_enable ();
*/
}

Mem::~Mem (void)
{
  freemem();
}

Mem::Mem (void)
{
  _chunks = 0;
  _numchunks = 0;
  _last_chunk_collect = 0;
  _mem = NULL;
  _mem_len = NULL;
  _mem_addr = NULL;

  _lastReadRow = -1;
  _lastWriteRow = -1;
}

static int foobar;

#define DUMPCHUNKS(xyz)						\
do {									\
  int _x;								\
  printf ("addr=%qx [%d] :: ", (xyz) << MEM_ALIGN, foobar);		\
  for (_x=0; _x<_chunks; _x++) {						\
    printf ("%d:[%qx, %qx] ", _x, _mem_addr[_x] << MEM_ALIGN, (_mem_addr[_x]+_mem_len[_x]-1)<<MEM_ALIGN);	\
  }; printf ("\n");  						\
} while (0)

#ifdef DEBUG_INV
#define CHECK_INV(p,f)						\
   {								\
     int _i;							\
     for (_i=0; _i < _chunks; _i++) {				\
       if (_i < _chunks-1) {					\
	 if (_mem_addr[_i]+_mem_len[_i]-1 < _mem_addr[_i+1])	\
	   continue;						\
	 printf (p ": INVARIANT FAILED @ %d!\n", _i);		\
	 break;							\
       }							\
     }								\
     if (_i != _chunks) {					\
        DUMPCHUNKS(f);						\
     }								\
   }
#else
#define CHECK_INV(p,f)
#endif

/*
 * add one more slot to memory array
 */
void 
Mem::get_more (int ch, long long num, int merging)
{
  long long newnum;
  unsigned long long t;
  if (num <= 0) return;
  
  CHECK_INV ("get_more", 0);

  if (!merging) {
    Assert ((ch == _chunks - 1) ||
	    (_mem_len[ch] + num + _mem_addr[ch] - 1 < _mem_addr[ch+1]),
	    "Overlap violation");

    if (_mem_len[ch] < DOUBLE_THRESHOLD)
      newnum = _mem_len[ch];
    else 
      newnum = CHUNK_SIZE;

    if (newnum < num)
      newnum = num;

    if (ch < _chunks - 1) {
      /* double if possible */
      if (_mem_addr[ch] + _mem_len[ch] + newnum - 1 < _mem_addr[ch+1]) {
	num = newnum;
      }
      else {
	num = _mem_addr[ch+1] - _mem_addr[ch] - _mem_len[ch];
      }
    }
    else {
      num = newnum;
    }
    _mem_len[ch] += num;
    REALLOC (_mem[ch], LL, _mem_len[ch]);
    for (t=_mem_len[ch]-1;t>=_mem_len[ch]-num;t--) {
       _mem[ch][t] = MEM_BAD;
    }
    CHECK_INV ("get_more1", 0);
    return;
  }
  /* merging */
  if (_mem_len[ch] == 0) {
    _mem_len[ch] = num;
    MALLOC (_mem[ch], LL, _mem_len[ch]);
    for (t=0;t<=_mem_len[ch]-1;t++) {
       _mem[ch][t] = MEM_BAD;
    }
  }
  else {
    _mem_len[ch] += num;
    REALLOC (_mem[ch], LL, _mem_len[ch]);
    for (t=_mem_len[ch]-1;t>=_mem_len[ch]-num;t--) {
       _mem[ch][t] = MEM_BAD;
    }
  }
}

/*
 * create new chunk, at position "chunks-1"
 *
 * Return values:
 *
 *   1  => created a new chunk at array position "chunks-1"
 *   0  => resized array at position ch
 */
int Mem::create_new_chunk (LL addr, int ch)
{
  int i, gap;
  static int last_create = -1;

#if 0
  printf ("Creating new chunk: 0x%llx @ %d, curcount = %d\n", addr,
	  ch, chunks);
#endif

  if (_chunks == _numchunks) {
    if (_numchunks == 0) {
      _numchunks = 4;
      MALLOC (_mem, LL *, _numchunks);
      MALLOC (_mem_len, LL, _numchunks);
      MALLOC (_mem_addr, LL, _numchunks);
    }
    else {
      _numchunks *= 2;
      REALLOC (_mem, LL *, _numchunks);
      REALLOC (_mem_len, LL, _numchunks);
      REALLOC (_mem_addr, LL, _numchunks);
    }
  }
  if (0 <= ch && ch < _chunks) {
    /* the new chunk is right next to "ch" */
    if (_mem_addr[ch] - addr >= CHUNK_SIZE) {
      _mem_len[_chunks] = CHUNK_SIZE;
    }
    else {
      /* next chunk is close by, we need to just adjust the size of
	 the next chunk by CHUNK_SIZE *backward*
      */
      if (ch > 0) {
	if (last_create == ch) {
	  adj_merge_size *= 2;
	  if (adj_merge_size > ADJ_LIMIT)
	    adj_merge_size = ADJ_LIMIT;
	}
	last_create = ch;
	
	gap = _mem_addr[ch] - _mem_len[ch-1] - _mem_addr[ch-1];
	gap = MIN (gap, adj_merge_size);
	REALLOC (_mem[ch], LL, _mem_len[ch] + gap);
	for (i=_mem_len[ch]-1; i >= 0; i--) {
	  _mem[ch][i+gap] = _mem[ch][i];
        }
	for (i=0; i < gap; i++) {
	  _mem[ch][i] = MEM_BAD;
	}
	_mem_addr[ch] = _mem_addr[ch] - gap;
	_mem_len[ch] = _mem_len[ch] + gap;
	return 0;
      }
      _mem_len[_chunks] = _mem_addr[ch] - addr;
    }
    Assert (_mem_len[_chunks] > 0, "What?");
  }
  else {
    _mem_len[_chunks] = CHUNK_SIZE;
  }
  MALLOC (_mem[_chunks], LL, _mem_len[_chunks]);
  _mem_addr[_chunks] = 0;
  for (i=0;i<=_mem_len[_chunks]-1;i++) {
     _mem[_chunks][i] = MEM_BAD;
  }
  _chunks++;
  return 1;
}

inline int Mem::_binsearch (LL addr)
{
  int i, j, m;
  
  if (_chunks == 0) return -1;
  
  i = 0; j = _chunks;

  while ((i+1) != j) {
    m = (i+j)/2;
    if (_mem_addr[m] > addr)
      j = m;
    else
      i = m;
  }
  foobar = i;
  if ((_mem_addr[i] <= addr) && (addr < _mem_addr[i] + _mem_len[i]))
    return i;
  else {
    return -1;
  }
}


int Mem::_binsearchw (LL addr)
{
  int i, j, m;
  
  if (_chunks == 0) return 0;
  
  i = 0; j = _chunks;

  while ((i+1) != j) {
    m = (i+j)/2;
    if (addr < _mem_addr[m])
      j = m;
    else
      i = m;
  }
#if 0
  foobar = i;
#endif
  if ((_mem_addr[i] <= addr) && (addr <= (_mem_addr[i] + _mem_len[i]))) {
     if ((i > 0) && ((_mem_addr[i-1] + _mem_len[i-1]) == addr))
      return i-1;
    return i;
  }
  else {
    /* not found */
    if (addr < _mem_addr[0])
      return 0;
    else if (addr > _mem_addr[_chunks-1] + _mem_len[_chunks-1])
      return _chunks;
    else
      return i+1;
  }
}

/*
 * get value @ address addr
 */
LL 
Mem::Read (LL addr)
{
  int i, w;

  addr >>= MEM_ALIGN;

#if 0
  for (i=0; i < _chunks; i++) {
    if (_mem_addr[i] <= addr && addr < _mem_addr[i]+_mem_len[i]) {
      w = _binsearch (addr);
      if (w != i) {
	printf (" ** READ: bin=%d, lin=%d **\n", w, i);
	DUMPCHUNKS(addr);
      }
      return _mem[i][addr-_mem_addr[i]];
    }
  }
  if (_binsearch (addr) != -1) {
    printf ("READ miss, bin=%d\n", foobar);
    DUMPCHUNKS(addr);
  }
  return MEM_BAD;
#endif
  if ((_lastReadRow != -1) &&
      (_mem_addr[_lastReadRow] <= addr) && 
      (addr < _mem_addr[_lastReadRow]+_mem_len[_lastReadRow])) {
     return _mem[_lastReadRow][addr-_mem_addr[_lastReadRow]];
  }
  w = _binsearch (addr);
  _lastReadRow = w;
  if (w == -1) {
    return MEM_BAD;
  }
  else {
    return _mem[w][addr-_mem_addr[w]];
  }
}

/* merge two chunks */
void Mem::merge_chunks (int c1, int c2)
{
  unsigned int i;

  Assert (_mem_addr[c1]+_mem_len[c1] == _mem_addr[c2], "What?");
  Assert (c1 < c2, "Hmm...");

#if 0
  printf ("Nchunks = %d\n", _chunks);
#endif

  get_more (c1, _mem_len[c2], 1);
  for (i=0; i < _mem_len[c2]; i++) {
    _mem[c1][i+_mem_addr[c2]-_mem_addr[c1]] = _mem[c2][i];
  }
  FREE (_mem[c2]);
  for (i=c2; i < _chunks-1; i++) {
    _mem[i] = _mem[i+1];
    _mem_len[i] = _mem_len[i+1];
    _mem_addr[i] = _mem_addr[i+1];
  }
  _mem_len[i] = 0;
  _chunks--;
  CHECK_INV ("merge_chunks",0);

  return;
}

/*
 *  garbage collect chunks periodically
 */
void Mem::chunk_gc (int force)
{
  int i;

  if (force || (_chunks - _last_chunk_collect > CHUNK_THRESHOLD)) {
    CHECK_INV("chunk_gc0", 0);
    i = 0;
    while (i < _chunks-1) {
      if (_mem_addr[i]+_mem_len[i] == _mem_addr[i+1])
	merge_chunks (i, i+1);
      else
	i++;
    }
    _last_chunk_collect = _chunks;
    CHECK_INV("chunk_gc1", 0);
  }

}

/*
 * store value at addr
 */
void 
Mem::Write (LL addr, LL val)
{
  int i, j, w;
  int ch, ret;
  LL len, sz;
  LL *data;

  addr >>= MEM_ALIGN;

//  CHECK_INV ("wbefore", addr);

//  context_disable ();

  if ((_lastWriteRow != -1) &&
      (_mem_addr[_lastWriteRow] <= addr) && 
      (addr < _mem_addr[_lastWriteRow]+_mem_len[_lastWriteRow])) {
     _mem[_lastWriteRow][addr-_mem_addr[_lastWriteRow]] = val;
     return;
  }
  w = _binsearchw (addr);
  _lastWriteRow = w;

  if ((w < _chunks) &&
      ((_mem_addr[w] <= addr) && (addr < _mem_addr[w]+_mem_len[w]))) {
    _mem[w][addr-_mem_addr[w]] = val;
    //  CHECK_INV("write0", addr);
//    context_enable ();
    return;
  }
  i = w;

  if ((i < _chunks) && (_chunks > 0) && (addr == (_mem_addr[i] + _mem_len[i]))) {
    /* check merging chunks! */
    if ((i < (_chunks-1)) && (_mem_addr[i+1] == addr)) {
      /* merge two chunks! */
      merge_chunks (i,i+1);
      _mem[i][addr-_mem_addr[i]] = val;
//      CHECK_INV("write1", addr);
//      context_enable();
      return;
    }
    /* extend chunk! */
    get_more (i, addr-_mem_addr[i]-_mem_len[i]+1);
    _mem[i][addr-_mem_addr[i]] = val;
//    CHECK_INV("write2", addr);
//    context_enable();
    return;
  }
  /* we need to move other chunks to the end */
  ch = i;
  ret = create_new_chunk (addr, ch);
  if (ret == 1) {
    /* save data and len */
    data = _mem[_chunks-1];
    len = _mem_len[_chunks-1];

    /* move other chunks up */
    for (j = _chunks-1; j >= ch+1; j--) {
      _mem_addr[j] = _mem_addr[j-1];
      _mem_len[j] = _mem_len[j-1];
      _mem[j] = _mem[j-1];
    }

    /* set chunk info */
    _mem[ch] = data;
    _mem_len[ch] = len;
    _mem_addr[ch] = addr;
    get_more (ch, addr-_mem_addr[ch]-_mem_len[ch]+1);
    _mem[ch][0] = val;
    // chunk_gc(0);

  }
  else if (ret == 0) {
    _mem[ch][addr - _mem_addr[ch]] = val;
    // chunk_gc(0);
  }
  else {
//     fatal_error ("Unknown ret value");
  }

//  CHECK_INV("write3", addr);

//  context_enable();
//  return;
}

/*
 * read memory image from a file
 */
int 
Mem::ReadImage (FILE *fp)
{
  freemem ();
  return MergeImage (fp);
}

int
Mem::ReadImageAndDuplicate (FILE *fp, Mem *dup_m)
{
  freemem ();
  return MergeImageAndDuplicate (fp, dup_m);
}
/*
 * read memory image from a file
 */
int 
Mem::ReadImage (char *s)
{
  FILE *fp;
  int i;

  if (!s) return 0;
  if (!(fp = fopen (s, "r")))
    return 0;
  freemem ();
  i = MergeImage (fp);
  fclose (fp);

  return i;
}

/*
 * read memory image from a file
 */
int 
Mem::MergeImage (char *s)
{
  FILE *fp;
  int i;

  if (!s) return 0;
  if (!(fp = fopen (s, "r")))
    return 0;
  i = MergeImage (fp);
  fclose (fp);

  return i;
}

/*
 * read memory image from a file
 */
int 
Mem::MergeImage (FILE *fp)
{
  char buf[1024];
  unsigned int a0,a1,v0,v1;
  unsigned int len;
  LL a, v, tmp;
  int endian;

  while (fgets (buf, 1024, fp)) {
    if (buf[0] == '!') {
      break;
    }
    if (buf[0] == '\n') continue;
    if (buf[0] == '#') continue;
    if (buf[0] == '@') {
      /*
	Format: @address data length:

	 "length" data items starting at "address" all contain
	 "data"
      */
#if MEM_ALIGN == 3
       if (sscanf (buf+1, "0x%8x%8x 0x%8x%8x %lu", &a0, &a1, &v0, &v1, &len) != 5) {
	  fatal_error ("sscanf error in image file");
       }
      a = ((LL)a0 << 32) | (LL)a1;
      v = ((LL)v0 << 32) | (LL)v1;
#elif MEM_ALIGN == 2
      sscanf (buf+1, "0x%8x 0x%8x %lu", &a, &v, &len);
#else
#error Unknown memory format
#endif
      while (len > 0) {
	Write (a, v);
	a += (1 << MEM_ALIGN);
	len--;
      }
      continue;
    }
    else if (buf[0] == '*') {
      /*
	Format: *address length
	        <list of data>

	 "length" locations starting at "address" contain the data
	 specified in the following "length" lines.
      */
#if MEM_ALIGN == 3
       if (sscanf (buf+1, "0x%8x%8x %lu", &a0, &a1, &len) != 3) {
	  fatal_error ("sscanf error in image file");
       }
      a = ((LL)a0 << 32) | (LL)a1;
#elif MEM_ALIGN == 2
      sscanf (buf+1, "0x%8x %lu", &a, &len);
#else
#error Unknown memory format
#endif
      while ((len > 0) && (fgets (buf, 1024, fp))) {
#if MEM_ALIGN == 3
	 if (sscanf (buf, "0x%8x%8x", &v0, &v1) != 2) {
	    fatal_error ("sscanf error in image file");
	 }

	v = ((LL)v0 << 32) | (LL)v1;
#elif MEM_ALIGN == 2
	sscanf (buf, "0x%8x", &v);
#else
#error Unknown memory format
#endif
	Write (a, v);
	a += (1 << MEM_ALIGN);
	len--;
      }
      if (len != 0) {
	fatal_error ("Memory format error, *0xaddr len format ends prematurely");
      }
      continue;
    }
    else if ((buf[0] == '+') || (buf[0] == '-')) {
      /* 
	 Handle non-aligned data

	 + or - followed by
	 Format: address length
	         <list of data>

         Starting at address, the following length *bytes* are
	 specified below.

	 Assumptions:
	      length < (1 << MEM_ALIGN)	 
	      +  means big-endian
	      -  means little-endian
      */
      if (buf[0] == '+')
	endian = 1; /* big-endian */
      else
	endian = 0; /* little-endian */
#if MEM_ALIGN == 3
      if (sscanf (buf+1, "0x%8x%8x %lu", &a0, &a1, &len) != 3) {
	 fatal_error ("sscanf error in image file");
      }
      a = ((LL)a0 << 32) | (LL)a1;
#elif MEM_ALIGN == 2
      sscanf (buf+1, "0x%8x %lu", &a, &len);
#else
#error Unknown memory format
#endif
      while ((len > 0) && (fgets (buf, 1024, fp))) {
#if MEM_ALIGN == 3
	if (sscanf (buf, "0x%2x", &v0) != 1) {
	   fatal_error ("sscanf error in image file");
	}
	v = v0;
#elif MEM_ALIGN == 2
	sscanf (buf, "0x%2x", &v);
#else
#error Unknown memory format
#endif
	tmp = Read (a);
	/* write the byte! */
	if (endian) {
	  tmp = (tmp & ~(0xffULL << ((~a & 3)<<3))) | (v << ((~a&3)<<3));
	}
	else {
	  tmp = (tmp & ~(0xffULL << ((a & 3)<<3))) | (v << ((a&3)<<3));
	}
	Write (a, tmp);
	a += 1;
	len--;
      }
      if (len != 0) {
	fatal_error ("Memory format error, *0xaddr len format ends prematurely");
      }
      continue;
    }
#if MEM_ALIGN == 3
    if (sscanf (buf, "0x%8x%8x 0x%8x%8x", &a0, &a1, &v0, &v1) != 4) {
       fatal_error ("sscanf error in image file");       
    }
    a = ((LL)a0 << 32) | (LL)a1;
    v = ((LL)v0 << 32) | (LL)v1;
#elif MEM_ALIGN == 2
    sscanf (buf, "0x%8x 0x%8x", &a, &v);
#else
#error Unknown memory format
#endif
    Write (a, v);
  }

  a = 0;
  for (a1=0; a1 < _chunks; a1++) {
     assert(_mem_len != NULL);
     a += _mem_len[a1];
  }

#if 0
  {
    DUMPCHUNKS(0);
    
    printf ("************************************************************************\n");
    chunk_gc (1);
    DUMPCHUNKS(0);
    printf ("************************************************************************\n");
  }
#endif

//  printf("Mem::MergeImage, returning %d\n", a);
  return a;
}

int
Mem::MergeImageAndDuplicate (FILE *fp, Mem *dup_m)
{
  char buf[1024];
  unsigned int a0,a1,v0,v1;
  unsigned int len;
  LL a, v, tmp;
  int endian;

  while (fgets (buf, 1024, fp)) {
    if (buf[0] == '!') {
      break;
    }
    if (buf[0] == '\n') continue;
    if (buf[0] == '#') continue;
    if (buf[0] == '@') {
      /*
        Format: @address data length:

         "length" data items starting at "address" all contain
         "data"
      */
#if MEM_ALIGN == 3
       if (sscanf (buf+1, "0x%8x%8x 0x%8x%8x %lu", &a0, &a1, &v0, &v1, &len) != 5) {
          fatal_error ("sscanf error in image file");
       }
      a = ((LL)a0 << 32) | (LL)a1;
      v = ((LL)v0 << 32) | (LL)v1;
#elif MEM_ALIGN == 2
      sscanf (buf+1, "0x%8x 0x%8x %lu", &a, &v, &len);
#else
#error Unknown memory format
#endif
      while (len > 0) {
        Write (a, v);
        dup_m->Write (a, v);
        a += (1 << MEM_ALIGN);
        len--;
      }
      continue;
    }
    else if (buf[0] == '*') {
      /*
        Format: *address length
                <list of data>

         "length" locations starting at "address" contain the data
         specified in the following "length" lines.
      */
#if MEM_ALIGN == 3
       if (sscanf (buf+1, "0x%8x%8x %lu", &a0, &a1, &len) != 3) {
          fatal_error ("sscanf error in image file");
       }
      a = ((LL)a0 << 32) | (LL)a1;
#elif MEM_ALIGN == 2
      sscanf (buf+1, "0x%8x %lu", &a, &len);
#else
#error Unknown memory format
#endif
      while ((len > 0) && (fgets (buf, 1024, fp))) {
#if MEM_ALIGN == 3
         if (sscanf (buf, "0x%8x%8x", &v0, &v1) != 2) {
            fatal_error ("sscanf error in image file");
         }

        v = ((LL)v0 << 32) | (LL)v1;
#elif MEM_ALIGN == 2
        sscanf (buf, "0x%8x", &v);
#else
#error Unknown memory format
#endif
        Write (a, v);
        dup_m->Write (a, v);
        a += (1 << MEM_ALIGN);
        len--;
      }
      if (len != 0) {
        fatal_error ("Memory format error, *0xaddr len format ends prematurely");
      }
      continue;
    }
    else if ((buf[0] == '+') || (buf[0] == '-')) {
      /*
         Handle non-aligned data

         + or - followed by
         Format: address length
                 <list of data>

         Starting at address, the following length *bytes* are
         specified below.

         Assumptions:
              length < (1 << MEM_ALIGN)
              +  means big-endian
              -  means little-endian
      */
      if (buf[0] == '+')
        endian = 1; /* big-endian */
      else
        endian = 0; /* little-endian */
#if MEM_ALIGN == 3
      if (sscanf (buf+1, "0x%8x%8x %lu", &a0, &a1, &len) != 3) {
         fatal_error ("sscanf error in image file");
      }
      a = ((LL)a0 << 32) | (LL)a1;
#elif MEM_ALIGN == 2
      sscanf (buf+1, "0x%8x %lu", &a, &len);
#else
#error Unknown memory format
#endif
      while ((len > 0) && (fgets (buf, 1024, fp))) {
#if MEM_ALIGN == 3
        if (sscanf (buf, "0x%2x", &v0) != 1) {
           fatal_error ("sscanf error in image file");
        }
        v = v0;
#elif MEM_ALIGN == 2
        sscanf (buf, "0x%2x", &v);
#else
#error Unknown memory format
#endif
        tmp = Read (a);
        /* write the byte! */
        if (endian) {
          tmp = (tmp & ~(0xffULL << ((~a & 3)<<3))) | (v << ((~a&3)<<3));
        }
        else {
          tmp = (tmp & ~(0xffULL << ((a & 3)<<3))) | (v << ((a&3)<<3));
        }
        Write (a, tmp);
        dup_m->Write (a, tmp);
        a += 1;
        len--;
      }
      if (len != 0) {
        fatal_error ("Memory format error, *0xaddr len format ends prematurely");
      }
      continue;
    }
#if MEM_ALIGN == 3
    if (sscanf (buf, "0x%8x%8x 0x%8x%8x", &a0, &a1, &v0, &v1) != 4) {
       fatal_error ("sscanf error in image file");
    }
    a = ((LL)a0 << 32) | (LL)a1;
    v = ((LL)v0 << 32) | (LL)v1;
#elif MEM_ALIGN == 2
    sscanf (buf, "0x%8x 0x%8x", &a, &v);
#else
#error Unknown memory format
#endif
    Write (a, v);
    dup_m->Write (a, v);
  }

  a = 0;
  for (a1=0; a1 < _chunks; a1++) {
     assert(_mem_len != NULL);
     a += _mem_len[a1];
  }
  return a;
}

/* Dump memory image */
void 
Mem::DumpImage (FILE *fp)
{
  int i, j, k;
  LL addr, data;
  unsigned int len;

  chunk_gc (1);

  for (i=0; i < _chunks; i++) {
    fprintf (fp, "# Chunk %d of %d\n", i, _chunks);
    addr = _mem_addr[i] << MEM_ALIGN;

    for (j=0; j < _mem_len[i]; j++)
      if (_mem[i][j] != _mem[i][0])
	break;
    if (j == _mem_len[i]) {
      data = _mem[i][0];
      len = _mem_len[i];
#if MEM_ALIGN == 3
      fprintf (fp, "@0x%08x%08x 0x%08x%08x %lu\n",
	       (unsigned int)(addr >> 32),
	       (unsigned int)(addr & 0xffffffff),
	       (unsigned int)(data >> 32),
	       (unsigned int)(data & 0xffffffff), len);
#elif MEM_ALIGN == 2
      fprintf (fp, "@0x%08x 0x%08x %lu\n", (unsigned int)addr,
	       (unsigned int) data, len);
#else
#error What?
#endif
    }
    else {
#if MEM_ALIGN == 3
	fprintf (fp, "*0x%08x%08x %lu\n",
		 (unsigned int)(addr >> 32),
		 (unsigned int)(addr & 0xffffffff),
		 (unsigned int) _mem_len[i]);
#elif MEM_ALIGN == 2
	fprintf (fp, "*0x%08x 0x%08x\n", (unsigned int)addr,
		 (unsigned int) _mem_len[i]);
#else
#error Unknown mem format
#endif
	for (j=0; j < _mem_len[i]; j++) {
	  data = _mem[i][j];
#if MEM_ALIGN == 3
	fprintf (fp, "0x%08x%08x\n",
		 (unsigned int)(data >> 32),
		 (unsigned int)(data & 0xffffffff));
#elif MEM_ALIGN == 2
	fprintf (fp, "0x%08x\n", (unsigned int) data);
#else
#error Unknown mem format
#endif
	}
    }
  }
}
