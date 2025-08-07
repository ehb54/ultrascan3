#ifndef SHS_USE_H
#define SHS_USE_H

#include <math.h>
typedef double shd_double;

#include <stdlib.h>
#include <time.h>

#include <iostream>
#include <string>
#include <vector>

#include "shd_global.h"

using namespace std;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef M_4PI
#define M_4PI (4e0 * 3.14159265358979323846)
#endif

class SHS_USE {
  int max_harmonics;

 public:
  string error_msg;

  SHS_USE(int max_harmonics);

  bool shs_compute_sphbes(shd_double x, shd_double* yv);
};

#endif
