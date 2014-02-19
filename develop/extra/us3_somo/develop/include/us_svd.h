#ifndef US_SVD
#define US_SVD


namespace SVD {
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
   extern char errormsg[ 128 ];

   bool dsvd(double **a, int m, int n, double *w, double **v);
}


#endif // US_SVD
