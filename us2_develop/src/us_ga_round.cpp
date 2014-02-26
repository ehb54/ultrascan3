#include <math.h>
#include "../include/us_ga_round.h"

double roundn(double d, double e, int n) {
  double m = pow(10,n-1) / e;
  d = nearbyint(d * m) / m;
  return d;
}

double floorn(double d, double e, int n) {
  double m = pow(10,n-1) / e;
  d = floor(d * m) / m;
  return d;
}

#ifdef MAIN
#include <stdio.h>
int main() {
  double x = 1.2345678e0;
  int i;
  for(i = 1; i < 5; i++) {
    printf("%d %g\n", i, roundn(x, 1e0, i));
  }
  return 0;
}
#endif
