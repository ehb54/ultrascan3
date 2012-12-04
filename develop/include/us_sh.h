#ifndef US_SH
#define US_SH

#include <stdio.h>
#include <math.h>
#include <vector>
#include <complex>
#ifndef M_2PI
# define M_2PI 6.28318530717958647692
#endif
#ifndef M_SQRT_PI_2
# define M_SQRT_PI_2 1.2533141373155001208
#endif

namespace nr {
   bool plegendre( int l, int m, double x, double &result );
   bool sphbes( int n, 
                double x, 
                double &sj
                );
}

namespace sh {
   bool spherical_harmonic( int l, int m, double theta, double phi, std::complex < double > &result );
   bool conj_spherical_harmonic( int l, int m, double theta, double phi, std::complex < double > &result );
   unsigned int fibonacci( unsigned int n );
   void build_grid( std::vector < std::vector < double > > &fib_grid, unsigned int size );
}

#endif // US_SH
