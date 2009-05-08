#include <stdlib.h>
#include <math.h>
#include "../include/us_ga_random_normal.h"

#ifdef MAIN
# include <stdio.h>
#endif

double_pair random_normal(double mean1, double sd1, double mean2, double sd2) {
   double s;
   double_pair v;
   do {
      v.x = 2 * drand48() - 1;
      v.y = 2 * drand48() - 1;
      s = v.x*v.x + v.y*v.y;
   } while (s > 1);
   s = sqrt(-2 * log(s) / s);
   v.x *= s * sd1;
   v.y *= s * sd2;
   v.x += mean1;
   v.y += mean2;
   return v;
}

#ifdef MAIN
int main(int argc, char **argv) {
   int i;
   double sum_x = 0e0;
   double sum_x_2 = 0e0;
   double max_x = -1e99;
   double max_y = -1e99;
   double min_x = 1e99;
   double min_y = 1e99;
   double sum_y = 0e0;
   double sum_y_2 = 0e0;
   double N = 0e0;
   double_pair v;
   if(argc < 7) {
      printf("random_normal seed trials mean1 sd1 mean2 sd2\n");
      exit(-1);
   }
   srand48(atoi(argv[1]));
   for(i = 0; i < atoi(argv[2]); i++) {
      v = random_normal(atof(argv[3]), atof(argv[4]),atof(argv[5]), atof(argv[6]));
      printf("%d %g %g\n", i, v.x, v.y);
      N++;
      sum_x += v.x;
      sum_x_2 += v.x * v.x;
      sum_y += v.y;
      sum_y_2 += v.y * v.y;
      if(v.x > max_x) {
         max_x = v.x;
      }
      if(v.x < min_x) {
         min_x = v.x;
      }
      if(v.y > max_x) {
         max_y = v.y;
      }
      if(v.y < min_y) {
         min_y = v.y;
      }

   }
   printf("trials %g\n"
          "\tmean1 = %g sd1 = %g [%g,%g]\n"
          "\tmean2 = %g sd2 = %g [%g,%g]\n"
          , N, 
          sum_x / N, sqrt((sum_x_2 - (sum_x * sum_x) / N)  / (N - 1)), min_x, max_x,
          sum_y / N, sqrt((sum_y_2 - (sum_y * sum_y) / N)  / (N - 1)), min_y, max_y
          );

   {
      int g;
      double sd1 = atof(argv[4]);
      double sd2 = atof(argv[6]);

      for(g = 0; g < 100; g++) {
         printf("g %d\n", g);
         for(i = 0; i < 20; i++) {
            v = random_normal(atof(argv[3]), sd1 ,atof(argv[5]), sd2);
            printf("\t%g %g\n", v.x, v.y);
         }
         if(!(g % 10)) {
            sd1 *= .7;
            sd2 *= .7;
         }
      }
   }   
}
#endif
