/***********************************************************************
 *    Copyright 1991                                                   *
 *    Department of Computer Science                                   *
 *    California Institute of Technology, Pasadena, CA 91125 USA       *
 *    All rights reserved                                              *
 ***********************************************************************/

#ifndef INTHASHH
#define INTHASHH

#ifdef __cplusplus
extern "C" {
#endif

struct IntHashelement {
   unsigned int key;
   union {
      unsigned int i;
      void* p;
   } value;
   struct IntHashelement *next;
};

struct IntHashtable {
   int size;
   struct IntHashelement **head;
   int nelements;
}; 

extern struct IntHashtable* createIntHashtable( int );
extern struct IntHashelement*
   addIntHashtable( struct IntHashtable* , unsigned int );
extern struct IntHashelement*
   associateIntHashtable( struct IntHashtable* , unsigned int );
extern void disassociateIntHashtable( struct IntHashtable* , unsigned int );
extern void releaseIntHashtable( struct IntHashtable* );

#ifdef __cplusplus
}
#endif

#endif
