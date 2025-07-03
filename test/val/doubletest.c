#include <stdio.h>
#include <stdlib.h>

double one    =  1.0;
double two    =  2.0;
double three  =  3.0;
double four   =  4.0;
double six    =  6.0;
double twelve = 12.0;

void test(int line, double f1, double f2) {
   if (f1 != f2) {
      printf("%ld\n", *((long *) &f1));
      printf("%ld\n", *((long *) &f2));
      printf("FAIL %d\n", line);
      exit(-1);
   }
}

int main(void) {
   double f;

   f = 1.0 + 3.0;
   test(__LINE__, f, four);
   
   f = 4.0 - 1.0;
   test(__LINE__, f, three);
   
   f = 2.0 * 3.0;
   test(__LINE__, f, six);

   f = 12.0 / 3.0;
   test(__LINE__, f, four);

   ////

   f = one + 3.0;
   test(__LINE__, f, four);
   
   f = four - 1.0;
   test(__LINE__, f, three);
   
   f = two * 3.0;
   test(__LINE__, f, six);

   f = twelve / 3.0;
   test(__LINE__, f, four);

   ////

   f = 1.0 + three;
   test(__LINE__, f, four);
   
   f = 4.0 - one;
   test(__LINE__, f, three);
   
   f = 2.0 * three;
   test(__LINE__, f, six);

   f = 12.0 / three;
   test(__LINE__, f, four);

   ////

   f = one + three;
   test(__LINE__, f, four);
   
   f = four - one;
   test(__LINE__, f, three);
   
   f = two * three;
   test(__LINE__, f, six);

   f = twelve / three;
   test(__LINE__, f, four);

   ////

   f = one;
   f += three;
   test(__LINE__, f, four);
   
   f = four;
   f -= one;
   test(__LINE__, f, three);
   
   f = two;
   f *= three;
   test(__LINE__, f, six);

   f = twelve;
   f /= three;
   test(__LINE__, f, four);

   ////

   f = 1.0;
   f += three;
   test(__LINE__, f, four);
   
   f = 4.0;
   f -= one;
   test(__LINE__, f, three);
   
   f = 2.0;
   f *= three;
   test(__LINE__, f, six);

   f = 12.0;
   f /= three;
   test(__LINE__, f, four);

   ////

   f = one;
   f += 3.0;
   test(__LINE__, f, four);
   
   f = four;
   f -= 1.0;
   test(__LINE__, f, three);
   
   f = two;
   f *= 3.0;
   test(__LINE__, f, six);

   f = twelve;
   f /= 3.0;
   test(__LINE__, f, four);

   ////

   f = 1.0;
   f += 3.0;
   test(__LINE__, f, four);
   
   f = 4.0;
   f -= 1.0;
   test(__LINE__, f, three);
   
   f = 2.0;
   f *= 3.0;
   test(__LINE__, f, six);

   f = 12.0;
   f /= 3.0;
   test(__LINE__, f, four);

   ////

   printf("ok\n");
   return 0;
}
