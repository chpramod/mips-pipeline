#include <stdio.h>

int rfib (int x)
{
  if (x == 0 || x == 1) return x;
  else return rfib(x-1) + rfib(x-2);
}

int main (void)
{ 
 int d;
 int i;
 d = 12;
 printf ("Series: ");
 for (i=1; i <= d; i++) {
	printf (" %d", rfib(i));
 }
 printf ("\n");
 return 99;
}
