#ifndef SHS_USE_H
#define SHS_USE_H

#include <math.h>
typedef double shd_double;

#include "shd_global.h"
#include <vector>
#include <string>
#include <iostream>
#include <time.h>
#include <stdlib.h>

using namespace std;

class SHS_USE
{
   int    max_harmonics;
 public:
   string error_msg;

   SHS_USE( int max_harmonics );

   bool shs_compute_sphbes( shd_double              x,
                            vector < shd_double > & yv );
};

#endif
