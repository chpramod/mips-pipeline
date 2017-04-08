#define x 8

int log2 (int n)
{
   if (n == 1) return 0;
   return (1+log2(n/2));
}


void main (void)
{
  if ((x & (x-1)) != 0) {
     printf("Input is not a power of 2. Cannot compute log2(%d).\n", x);
  }
  else {
     printf ("log2(%d) is %d\n", x, log2(x));
  }
  exit (2);
}

