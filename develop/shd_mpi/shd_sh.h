#ifndef SHD_SH
#define SHD_SH

#include <stdio.h>
#include <math.h>
#include <vector>
#include <complex>

typedef float shd_double;

#ifndef M_PI
#define M_PI       3.14159265358979323846
#endif
#ifndef M_2PI
# define M_2PI 6.28318530717958647692
#endif
#ifndef M_SQRT_PI_2
# define M_SQRT_PI_2 1.2533141373155001208
#endif

#ifndef M_ONE_OVER_4PI
# define M_ONE_OVER_4PI 0.0795774715459477
#endif

namespace nr {
   bool plegendre( int l, int m, shd_double x, shd_double &result );
   bool sphbes( int n, 
                shd_double x, 
                shd_double &sj
                );
}

namespace sh {
   bool spherical_harmonic( int l, int m, shd_double theta, shd_double phi, std::complex < shd_double > &result );
   bool conj_spherical_harmonic( int l, int m, shd_double theta, shd_double phi, std::complex < shd_double > &result );
   unsigned int fibonacci( unsigned int n );
   void build_grid( std::vector < std::vector < shd_double > > &fib_grid, unsigned int size );

   bool alt_conj_sh( int max_harmonics, 
                     shd_double theta,
                     shd_double phi,
                     std::complex < shd_double > * Yp );
}

#endif // SHD_SH
