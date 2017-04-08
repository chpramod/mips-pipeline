/***********************************************************************
 *    Copyright 1991                                                   *
 *    Department of Computer Science                                   *
 *    California Institute of Technology, Pasadena, CA 91125 USA       *
 *    All rights reserved                                              *
 ***********************************************************************/

#include <stdio.h>
#include <strings.h>
#include <stdlib.h>

#include "inthash.h"
#include "misc.h"

static int T[] =
{  1, 87, 49, 12,176,178,102,166,121,193,  6, 84,249,230, 44,163,
  14,197,213,181,161, 85,218, 80, 64,239, 24,226,236,142, 38,200,
 110,177,104,103,141,253,255, 50, 77,101, 81, 18, 45, 96, 31,222,
  25,107,190, 70, 86,237,240, 34, 72,242, 20,214,244,227,149,235,
  97,234, 57, 22, 60,250, 82,175,208,  5,127,199,111, 62,135,248,
 174,169,211, 58, 66,154,106,195,245,171, 17,187,182,179,  0,243,
 132, 56,148, 75,128,133,158,100,130,126, 91, 13,153,246,216,219,
 119, 68,223, 78, 83, 88,201, 99,122, 11, 92, 32,136,114, 52, 10,
 138, 30, 48,183,156, 35, 61, 26,143, 74,251, 94,129,162, 63,152,
 170,  7,115,167,241,206,  3,150, 55, 59,151,220, 90, 53, 23,131,
 125,173, 15,238, 79, 95, 89, 16,105,137,225,224,217,160, 37,123,
 118, 73,  2,157, 46,116,  9,145,134,228,207,212,202,215, 69,229,
  27,188, 67,124,168,252, 42,  4, 29,108, 21,247, 19,205, 39,203,
 233, 40,186,147,198,192,155, 33,164,191, 98,204,165,180,117, 76,
 140, 36,210,172, 41, 54,159,  8,185,232,113,196,231, 47,146,120,
  51, 65, 28,144,254,221, 93,189,194,139,112, 43, 71,109,184,209};


static int
hash( struct IntHashtable* h, unsigned int k)
{
   register unsigned int sum;
   int size=h->size;
/*
 * 8, 16, 24, or 32 bit hashing function--- from CACM June 1990, p.679
 */
   if ( size <= (1<<8)) {
      sum = T[ (0xff & k)];
      k = k >> 8;
      sum = T[ sum ^ (0xff & k)];
      k = k >> 8;
      sum = T[ sum ^ (0xff & k)];
      k = k >> 8;
      sum = T[ sum ^ (0xff & k)];
   } else if ( size <= (1<<16)) {
      register unsigned int sum1;
      sum  = T[ 0xff & k];
      sum1 = T[ 0xff & ( 1+(0xff & k))];
      k = k >> 8;
      sum  = T[ sum ^ (0xff & k)];
      sum1 = T[ sum1 ^ (0xff & k)];
      k = k >> 8;
      sum  = T[ sum ^ (0xff & k)];
      sum1 = T[ sum1 ^ (0xff & k)];
      k = k >> 8;
      sum  = T[ sum ^ (0xff & k)];
      sum1 = T[ sum1 ^ (0xff & k)];
      sum |= sum1 << 8;
   } else if ( size <= (1<<24)) {
      register unsigned int sum1, sum2;
      sum  = T[ 0xff & k];
      sum1 = T[ 0xff & ( 1+(0xff & k))];
      sum2 = T[ 0xff & ( 2+(0xff & k))];
      k = k >> 8;
      sum  = T[ sum ^ (0xff & k)];
      sum1 = T[ sum1 ^ (0xff & k)];
      sum2 = T[ sum2 ^ (0xff & k)];
      k = k >> 8;
      sum  = T[ sum ^ (0xff & k)];
      sum1 = T[ sum1 ^ (0xff & k)];
      sum2 = T[ sum2 ^ (0xff & k)];
      sum |= (sum1 << 8) | (sum2 << 16);
   } else {
      register unsigned int sum1, sum2, sum3;
      sum  = T[ 0xff & k];
      sum1 = T[ 0xff & ( 1+(0xff & k))];
      sum2 = T[ 0xff & ( 2+(0xff & k))];
      sum3 = T[ 0xff & ( 3+(0xff & k))];
      k = k >> 8;
      sum  = T[ sum ^ (0xff & k)];
      sum1 = T[ sum1 ^ (0xff & k)];
      sum2 = T[ sum2 ^ (0xff & k)];
      sum3 = T[ sum3 ^ (0xff & k)];
      k = k >> 8;
      sum  = T[ sum ^ (0xff & k)];
      sum1 = T[ sum1 ^ (0xff & k)];
      sum2 = T[ sum2 ^ (0xff & k)];
      sum3 = T[ sum3 ^ (0xff & k)];
      k = k >> 8;
      sum  = T[ sum ^ (0xff & k)];
      sum1 = T[ sum1 ^ (0xff & k)];
      sum2 = T[ sum2 ^ (0xff & k)];
      sum3 = T[ sum3 ^ (0xff & k)];
      sum |= (sum1 << 8) | (sum2 << 16) | (sum3 << 24);
   }
   return sum & (size-1);
}

#if 0
static void
statsIntHashtable( FILE *fp, struct IntHashtable *ht)
{
   int i, num_in_bucket;
   struct IntHashelement *h;
   fprintf( fp, "Bucket counts (size %d):\n", ht->size);
   for( i=0; i<ht->size; i++) {
      for( num_in_bucket=0, h=ht->head[i]; h; h=h->next, num_in_bucket++) ;
      fprintf( fp, "%d ", num_in_bucket);
   }
   fprintf( fp, "\n");
}
#endif

static void
expandIntHashtable( ht)
struct IntHashtable *ht;
{
   struct IntHashtable Ht;
   Ht.size = ( ht->size << 2);
   MALLOC(Ht.head,struct IntHashelement*, Ht.size);
   Ht.nelements = ht->nelements;
   {
      int i;
      for( i=0; i<Ht.size; i++)
	 Ht.head[i] = 0;
      for( i=0; i<ht->size; i++) {
	 struct IntHashelement *h=ht->head[i], *hnext;
	 for( ; h; h=hnext) {
	    int bucket;
	    hnext = h->next;
	    bucket = hash( &Ht, h->key);
	    h->next = Ht.head[bucket];
	    Ht.head[bucket] = h;
	 }
      }
   }
   FREE( (char*) ht->head);
   *ht = Ht;
}

struct IntHashtable*
createIntHashtable( s)
int s;
{

   struct IntHashtable* h;

   NEW (h, struct IntHashtable);

   for( h->size=1; s > h->size; h->size <<= 1) ;

   MALLOC (h->head, struct IntHashelement *, h->size);
   h->nelements = 0;

   for( s=0; s< h->size; s++) h->head[s] = 0;

   return h;
}

struct IntHashelement*
addIntHashtable( struct IntHashtable* h, unsigned int k)
{

   int bucket;
   struct IntHashelement* p;

   if ( h->nelements > ( h->size << 2)) {
#if 0
      statsIntHashtable( stderr, h);
#endif
      expandIntHashtable( h);
   }

   bucket = hash( h, k);

   for( p = h->head[bucket]; p; p=p->next)
      if ( p->key == k) {
	 (void) fprintf( stderr,
			 "addIntHashtable: Key '%u' already entered.\n", k);
	 exit( 1);
      }

   NEW (p, struct IntHashelement);
   p->key = k;
   p->next = h->head[bucket];
   h->head[bucket] = p;
   h->nelements++;

   return p;

}

struct IntHashelement*
associateIntHashtable( struct IntHashtable* h, unsigned int k)
{
   int bucket;
   struct IntHashelement* p;

   bucket = hash( h, k);

   for( p = h->head[bucket]; p; p=p->next)
      if ( p->key == k) return p;

   return 0;
}

void
disassociateIntHashtable( struct IntHashtable* h, unsigned int k)
{
   int bucket;
   struct IntHashelement *p, *pnext, **prev;

   bucket = hash( h, k);

   prev = &h->head[bucket];
   for( p = *prev; p; p=pnext) {
      pnext = p->next;
      if ( p->key == k) {
	 *prev = p->next;
	 FREE( (char *) p);
	 h->nelements--;
	 return;
      } else {
	 prev = &p->next;
      }
   }
   (void) fprintf( stderr,
		   "disassociateIntHashtable: Key '%u' not found.\n", k);
   exit( 1);
}

void
releaseIntHashtable( struct IntHashtable* h)
{

   while( h->size--) {
      while( h->head[h->size]) {
	 struct IntHashelement* p;
	 p = h->head[h->size];
	 h->head[h->size] = h->head[h->size]->next;

	 FREE( (char*) p);
      }
   }

   FREE( (char*) h->head);
   FREE( (char*) h);

}

