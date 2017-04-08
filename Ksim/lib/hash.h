/***********************************************************************
 *    Copyright 1991                                                   *
 *    Department of Computer Science                                   *
 *    California Institute of Technology, Pasadena, CA 91125 USA       *
 *    All rights reserved                                              *
 ***********************************************************************/

#ifndef HASHH
#define HASHH

#ifdef __cplusplus
extern "C" {
#endif


struct Hashlistelement {
   char* key;
   union {
      int i;
      char* p;
   } value;
   struct Hashlistelement *next;
};

struct Hashtable {
   int size;
   struct Hashlistelement **head;
   int nelements;
}; 

/* You can access all elements in struct Hashtable *h as follows:
o	int i;
	struct Hashlistelement *e;
	for (i = 0; i < h->size; i++)
	  for (e = h->head[i]; e; e = e->next)
	    access(e);
  nelements is the total number of elements in the hashtable.
*/


extern struct Hashtable* createHashtable  ( int );
 /* Create a new hashtable of the specified size. The table will grow as
    needed.
 */
extern struct Hashlistelement* addHashtable 
   ( struct Hashtable* , const char* key );
 /* Add a new element to the table. It is an error if the element is already
    in the table. New memory is allocated for the key. Note that the
    value field of the returned element is not initialized.
 */
extern struct Hashlistelement* associateHashtable
   ( struct Hashtable* , const char* );
 /* Find an element in the table. Returns 0 if not found.
 */
extern void disassociateHashtable  ( struct Hashtable* , const char* );
 /* Delete an element from the table. It is an error if the element is not
    in the table. The key field and the Hashlistelement are free()d
 */
extern void releaseHashtable  ( struct Hashtable* );
 /* Delete all elements from the table, and delete the table itself.
 */
/* extern void printHashstatistics  (struct Hashtable*); */

#ifdef __cplusplus
}
#endif

#endif
