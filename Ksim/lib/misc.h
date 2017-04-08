/*************************************************************************
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
 *  $Id: misc.h,v 1.1.1.1 2006/05/23 13:53:59 mainakc Exp $
 *
 *************************************************************************/
#ifndef __MISC_H__
#define __MISC_H__

#include <stdlib.h> /* malloc.h is deprecated */
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MALLOC

#define MALLOC(var,type,size) do { if (size == 0) { fprintf (stderr, "FATAL: allocating zero-length block.\n\tFile %s, line %d\n", __FILE__, __LINE__); exit (2); } else { var = (type *) malloc (sizeof(type)*(size)); if (!var) { fprintf (stderr, "FATAL: malloc of size %d failed!\n\tFile %s, line %d\n", sizeof(type)*(size), __FILE__, __LINE__); fflush(stderr); exit (1); } } } while (0)

#define REALLOC(var,type,size) do { if (size == 0) { fprintf (stderr, "FATAL: allocating zero-length block.\n\tFile %s, line %d\n", __FILE__, __LINE__); exit (2); } else { var = (type *) realloc (var, sizeof(type)*(size)); if (!var) { fprintf (stderr, "FATAL: realloc of size %d failed!\n\tFile %s, line %d\n", sizeof(type)*(size), __FILE__, __LINE__); exit (1); } } } while (0)

#ifdef NEW
#undef NEW
#endif
#define NEW(var,type) MALLOC(var,type,1)
/*#define FREE(var)   free(var)*/
#define FREE(var) 

#endif /* MALLOC */

#define Max(a,b) ( ((a) > (b)) ? (a) : (b) )

void fatal_error (char *s, ...);
void warning (char *s, ...);
char *Strdup (char *);

#define Assert(a,b) do { if (!(a)) { fprintf (stderr, "Assertion failed, file %s, line %d\n", __FILE__, __LINE__); fprintf (stderr, "Assertion: " #a "\n"); fprintf (stderr, "ERR: " b "\n"); exit (4); } } while (0)

#define MEMCHK(x)  Assert (x, "out of memory")

#ifdef __cplusplus
}
#endif


#endif /* __MISC_H__ */
