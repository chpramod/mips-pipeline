#define x 10

int factorial (int n)
{
   if (n <= 1) return 1;
   return (n*factorial(n-1));
}


void main (void)
{
  printf ("%d! is %d\n", x, factorial(x));
  exit (2);
}

