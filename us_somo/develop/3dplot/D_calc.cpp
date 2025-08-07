#include "D_calc.h"

#include <math.h>
#include <stdio.h>

// For some reson WIN32 is not picking up M_PI from math.h...

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static float R = 8.314e7;
static float T = (float)293.15;
static float eta = (float)0.01;
static float N = (float)6.022e23;
static float rho = 1.0;

float D_calc(float s, float vbar, float k) {
  // vbar for DNA .55, proteins .69 to .81 usually .72-.74
  // k 1 to 4
  return (float)((R * T) /
                 (N * k * 6.0 * eta * M_PI *
                  pow((9.0 * s * k * vbar * eta) / (2.0 * (1.0 - vbar * rho)),
                      0.5)));
}

float s_calc(float D, float vbar, float k) {
  return (float)(pow(R * T / (D * N * k * 6.0 * eta * M_PI), 2) * 2.0 *
                 (1.0 - vbar * rho) / (9.0 * k * vbar * eta));
}

float k_calc(float s, float D, float vbar) {
  return (float)pow(pow(R * T / (D * N * 6.0 * eta * M_PI), 2) * 2.0 *
                        (1.0 - vbar * rho) / (9.0 * s * vbar * eta),
                    1e0 / 3e0);
}

#ifdef MAIN
main() {
  printf("%g %g %g %g\n", D_calc(3e-13, .72, 1), D_calc(3e-13, .72, 2),
         D_calc(3e-13, .72, 3), D_calc(3e-13, .72, 4));

  printf("%g %g %g %g\n", s_calc(1.1524e-06, .72, 1),
         s_calc(4.07436e-07, .72, 2), s_calc(2.2178e-07, .72, 3),
         s_calc(1.4405e-07, .72, 4));

  printf("%g %g %g %g\n", k_calc(3e-13, 1.1524e-06, .72),
         k_calc(3e-13, 4.07436e-07, .72), k_calc(3e-13, 2.2178e-07, .72),
         k_calc(3e-13, 1.4405e-07, .72));
}
#endif
