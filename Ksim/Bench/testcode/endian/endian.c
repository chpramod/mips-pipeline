int main(void)
{
   int x = 0x12345678;
   char *c = (char*)&x;
   printf("%#x\n", *c);
   return 0;
}
