/*************************************************************************
 *
 *  (c) 1998-1999 Cornell University
 *  Computer Systems Laboratory
 *  Ithaca, NY 14853.
 *  All Rights Reserved
 *
 *
 *  (c) 1996-1998 California Institute of Technology
 *  Department of Computer Science
 *  Pasadena, CA 91125.
 *  All Rights Reserved
 *
 *  $Id: guess_jmpbuf.c,v 1.1.1.1 2006/05/23 13:53:59 mainakc Exp $
 *
 *************************************************************************/

#include <stdio.h>
#include <setjmp.h>

jmp_buf buf;

#define SZ sizeof(buf)/sizeof(unsigned long)
#define E(n) ((unsigned long*)&buf)[n]

#define Abs(a)  ((a) < 0 ? -(a) : (a))

#define UP 1
#define DN 2

void f ( int x )
{
  long stkdist, pcdist;
  long s,p;
  int stkpos = 0, pcpos = 0;
  int i;
  int grow_dir;

  _setjmp (buf);

  stkdist = (long)E(0)-(long)&i;
  pcdist  = (long)E(0)-(long)&f;

  if ((long)&i > (long)&x) {
    grow_dir = UP;
    stkdist = -1;
  }
  else {
    grow_dir = DN;
    stkdist = 1;
  }

  for (i=0; i < SZ; i++) {
    s = (long)E(i)-(long)&i;
    p = (long)E(i)-(long)&f;

    /* pick a pc that's close */
    if (Abs(p) < Abs(pcdist))  { pcdist = p; pcpos = i; }
    
    /* pick an sp that's close and growing in the same way */
    if ((stkdist < 0 && grow_dir == UP) 
	|| (stkdist > 0 && grow_dir == DN) 
	||(((grow_dir == UP && s > 0) || (grow_dir == DN && s < 0)) 
	   && Abs(s) < Abs(stkdist)))  {
      stkdist = s;
      stkpos = i; 
    }
  }
  if (stkpos  == pcpos || x > 1) {
    if (x == 1) 
      printf ("#error Okay, I'm hopelessly confused. You figure this out\n");
    printf ("#error Function entry point: %x\n", &f);
    printf ("#error Stack: %x\n", &i);
    printf ("#error Stack grows: %s\n", grow_dir == UP ? "UP" : "DN");
    for (i=0; i < SZ; i++)
      printf ("#error jb[%d] = %x\n", i, E(i));
  }
  else {
    printf ("#define PC_OFFSET %d\n", pcpos);
    printf ("#define SP_OFFSET %d\n", stkpos);
    if (grow_dir == UP)
      printf ("#define STACK_DIR_UP\n");
    else
      printf ("#define STACK_DIR_DN\n");
  }
}

int main (int argc, char *argv)
{
  f(argc);
  return 0;
}
