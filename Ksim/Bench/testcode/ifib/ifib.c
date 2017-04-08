#include <stdio.h>

int ifib (int n)
{ 
  int x, y, z, i;

  if (n == 0 || n == 1) return n;
  x = 0; y = 1;
  for (i=2; i <= n; i++) {
	z = x + y;
	x = y;
	y = z;
  }
  return y;
}

int main (void)
{ 
 int d;
 int i;
 d = 12;
 printf ("Series: ");
 for (i=1; i <= d; i++) {
	printf (" %d", ifib(i));
 }
 printf ("\n");
 return 99;
}
