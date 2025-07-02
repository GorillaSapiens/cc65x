#include <stdio.h>
#include <stdlib.h>

float one    =  1.0f;
float two    =  2.0f;
float three  =  3.0f;
float four   =  4.0f;
float six    =  6.0f;
float twelve = 12.0f;

void test(int line, float f1, float f2) {
   if (f1 != f2) {
      printf("FAIL %d\n", line);
      exit(-1);
   }
}

int main(void) {
   float f;

   f = 1.0f + 3.0f;
   test(__LINE__, f, four);
   
   f = 4.0f - 1.0f;
   test(__LINE__, f, three);
   
   f = 2.0f * 3.0f;
   test(__LINE__, f, six);

   f = 12.0f / 3.0f;
   test(__LINE__, f, four);

   ////

   f = one + 3.0f;
   test(__LINE__, f, four);
   
   f = four - 1.0f;
   test(__LINE__, f, three);
   
   f = two * 3.0f;
   test(__LINE__, f, six);

   f = twelve / 3.0f;
   test(__LINE__, f, four);

   ////

   f = 1.0f + three;
   test(__LINE__, f, four);
   
   f = 4.0f - one;
   test(__LINE__, f, three);
   
   f = 2.0f * three;
   test(__LINE__, f, six);

   f = 12.0f / three;
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

   f = 1.0f;
   f += three;
   test(__LINE__, f, four);
   
   f = 4.0f;
   f -= one;
   test(__LINE__, f, three);
   
   f = 2.0f;
   f *= three;
   test(__LINE__, f, six);

   f = 12.0f;
   f /= three;
   test(__LINE__, f, four);

   ////

   f = one;
   f += 3.0f;
   test(__LINE__, f, four);
   
   f = four;
   f -= 1.0f;
   test(__LINE__, f, three);
   
   f = two;
   f *= 3.0f;
   test(__LINE__, f, six);

   f = twelve;
   f /= 3.0f;
   test(__LINE__, f, four);

   ////

   f = 1.0f;
   f += 3.0f;
   test(__LINE__, f, four);
   
   f = 4.0f;
   f -= 1.0f;
   test(__LINE__, f, three);
   
   f = 2.0f;
   f *= 3.0f;
   test(__LINE__, f, six);

   f = 12.0f;
   f /= 3.0f;
   test(__LINE__, f, four);

   ////

   printf("ok\n");
   return 0;
}
