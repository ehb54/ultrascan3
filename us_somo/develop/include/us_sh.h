#ifndef US_SH
#define US_SH

#include <math.h>
#include <stdio.h>

#include <complex>
#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#ifndef M_2PI
#define M_2PI 6.28318530717958647692
#endif
#ifndef M_SQRT_PI_2
#define M_SQRT_PI_2 1.2533141373155001208
#endif

namespace nr {
bool plegendre(int l, int m, double x, double &result);
bool sphbes(int n, double x, double &sj);
}  // namespace nr

namespace sh {
bool spherical_harmonic(int l, int m, double theta, double phi,
                        std::complex<double> &result);
bool conj_spherical_harmonic(int l, int m, double theta, double phi,
                             std::complex<double> &result);
unsigned int fibonacci(unsigned int n);
void build_grid(std::vector<std::vector<double> > &fib_grid, unsigned int size);

bool alt_conj_sh(int max_harmonics, double theta, double phi,
                 std::complex<float> *Yp);
}  // namespace sh

#endif  // US_SH
