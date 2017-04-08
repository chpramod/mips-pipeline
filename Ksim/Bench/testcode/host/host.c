#include <stdio.h>

typedef unsigned long long LL;
typedef unsigned char Byte;

int sum (int n)
{
  if (n <= 0) return 1;
  else return n + sum (n-1);
}

char *endianness (void)
{
  long *l;
  char c[4];
  
  l = (long*)c;

  c[0] = 1;
  c[1] = 0;
  c[2] = 0; 
  c[3] = 0;

  if (*l == 1)
    return "little";
  else
    return "big";
}

int main (void)
{
  FILE *fp;
  int i, j;
  LL data[4];
  Byte buf[128];
  fp = fopen ("test-out", "w");
  if (!fp) {
    printf ("could not open file test-out for writing\n");
    return 1;
  }
  fprintf (fp, "Does a mips-sgi-irix5 binary work on an x86?\n");
  fprintf (fp, "In spite of what libbfd docs say?\n");
  fprintf (fp, "Your host seems to be of type %s-endian\n", endianness ());
  printf ("Summation\n\n");
  i = 5;
  do {
    printf ("Int: ");
    j = sum (i);
    printf ("sum %d = %d\n", i, j);
    fprintf (fp, "sum[%d] = %d\n", i, j);
    i--;
  } while (i > 0);
  fclose (fp);
  return 0;
}
