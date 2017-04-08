#include <stdio.h>

int
main(int argc, char **argv)
{
   int i, j;

   j = 0;

   for (i = 0; i < 10; i++) {
      j += i;
   }

   printf("j = %d\n", j);
   return 0;  
}
