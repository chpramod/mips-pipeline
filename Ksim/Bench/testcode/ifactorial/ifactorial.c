#define x 10

int factorial (int n)
{
   int i, f=1;
  
   for (i=2; i<=n; i++) {
      f *= i;
   }
   return f;
}


void main (void)
{
  printf ("%d! is %d\n", x, factorial(x));
  exit (2);
}

