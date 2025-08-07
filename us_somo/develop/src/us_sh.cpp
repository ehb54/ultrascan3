#include "../include/us_sh.h"

#include <iostream>
// #include <iomanip>

namespace nr {
bool plegendre(int l, int m, double x, double &result) {
  double fact;
  double oldfact;
  double pll = 0e0;
  double pmm;
  double pmmp1;
  double omx2;

  int i;
  int ll;

  if (m < 0 || m > l || fabs(x) > 1e0) {
    printf("nr::plegendre m < 0 || m > l || fabs(x) > 1e0 m %d l %d x %g\n", m,
           l, x);
    return false;
  }

  pmm = 1e0;

  if (m > 0) {
    omx2 = (1e0 - x) * (1e0 + x);
    fact = 1e0;
    for (i = 1; i <= m; i++) {
      pmm *= omx2 * fact / (fact + 1e0);
      fact += 2e0;
    }
  }
  pmm = sqrt((2e0 * m + 1e0) * pmm / (4e0 * M_PI));

  if (m & 1) {
    pmm = -pmm;
  }

  if (l == m) {
    result = pmm;
    return true;
  } else {
    pmmp1 = x * sqrt(2e0 * m + 3e0) * pmm;
    if (l == (m + 1)) {
      result = pmmp1;
      return true;
    } else {
      oldfact = sqrt(2e0 * m + 3e0);
      for (ll = m + 2; ll <= l; ll++) {
        fact = sqrt((4e0 * ll * ll - 1e0) / (ll * ll - m * m));
        pll = (x * pmmp1 - pmm / oldfact) * fact;
        oldfact = fact;
        pmm = pmmp1;
        pmmp1 = pll;
      }
      result = pll;
      return true;
    }
  }
  printf("nr::plegendre drop out\n");
  return false;
}

bool chebev(double a, double b, double c[], int m, double x, double &res) {
  double d = 0e0;
  double dd = 0e0;
  double sv;
  double y;
  double y2;

  if ((x - a) * (x - b) > 0e0) {
    std::cout << "nr::chebev x not in range in routine chebev" << std::endl;
    return false;
  }

  y2 = 2e0 * (y = (2e0 * x - a - b) / (b - a));
  for (int j = m - 1; j >= 1; j--) {
    sv = d;
    d = y2 * d - dd + c[j];
    dd = sv;
  }
  res = y * d - dd + 0.5 * c[0];
  return true;
}

#define NUSE1 5
#define NUSE2 5

bool beschb(double x, double &gam1, double &gam2, double &gampl,
            double &gammi) {
  double xx;
  static double c1[] = {-1.142022680371172e0,
                        6.516511267076e-3,
                        3.08709017308e-4,
                        -3.470626964e-6,
                        6.943764e-9,
                        3.6780e-11,
                        -1.36e-13};
  static double c2[] = {1.843740587300906e0, -0.076852840844786e0,
                        1.271927136655e-3,   -4.971736704e-6,
                        -3.3126120e-8,       2.42310e-10,
                        -1.70e-13,           -1.0e-15};

  xx = 8e0 * x * x - 1e0;

  if (!chebev(-1.0, 1.0, c1, NUSE1, xx, gam1)) {
    return false;
  }
  if (!chebev(-1.0, 1.0, c2, NUSE2, xx, gam2)) {
    return false;
  }
  gampl = gam2 - x * gam1;
  gammi = gam2 + x * gam1;
  return true;
}
#undef NUSE1
#undef NUSE2

#define EPS 1.0e-10
#define FPMIN 1.0e-30
#define MAXIT 10000

bool bessj(double x, double xnu, double *rj) {
  int isign;
  int i;
  int l;
  int nl;
  double a;
  double b;
  double br;
  double bi;
  double c;
  double cr;
  double ci;
  double d;
  double del;
  double del1;
  double den;
  double di;
  double dlr;
  double dli;
  double dr;
  double e;
  double f;
  double fact;
  double fact2;
  double fact3;
  double ff;
  double gam;
  double gam1;
  double gam2;
  double gammi;
  double gampl;
  double h;
  double p;
  double pimu;
  double pimu2;
  double q;
  double r;
  double rjl;
  double rjl1;
  double rjmu;
  // double rjp1;
  double rjpl;
  double rjtemp;
  double ry1;
  double rymu;
  double rymup;
  double sum;
  double sum1;
  double temp;
  double w;
  double x2;
  double xi;
  double xi2;
  double xmu;
  double xmu2;

  if (x <= 0.0 || xnu < 0.0) {
    std::cout << "nr::bessj bad arguments" << std::endl;
    return false;
  }
  if (x < 2) {
    nl = (int)(xnu + 0.5);
  } else {
    int tmp = (int)(xnu - x + 1.5);
    if (0 > tmp) {
      nl = 0;
    } else {
      nl = tmp;
    }
  }

  xmu = xnu - nl;
  xmu2 = xmu * xmu;
  xi = 1e0 / x;
  xi2 = 2e0 * xi;
  w = xi2 / M_PI;
  isign = 1;
  h = xnu * xi;
  if (h < FPMIN) {
    h = FPMIN;
  }
  b = xi2 * xnu;
  d = 0E0;
  c = h;

  for (i = 1; i <= MAXIT; i++) {
    b += xi2;
    d = b - d;
    if (fabs(d) < FPMIN) {
      d = FPMIN;
    }
    c = b - 1e0 / c;
    if (fabs(c) < FPMIN) {
      c = FPMIN;
    }
    d = 1e0 / d;
    del = c * d;
    h = del * h;
    if (d < 0e0) {
      isign = -isign;
    }
    if (fabs(del - 1e0) < EPS) {
      break;
    }
  }

  if (i > MAXIT) {
    std::cout << "nr::bessj x too large; try asymptotic expansion" << std::endl;
    return false;
  }

  rjl = isign * FPMIN;
  rjpl = h * rjl;
  rjl1 = rjl;
  // rjp1 = rjpl;
  fact = xnu * xi;
  for (l = nl; l >= 1; l--) {
    rjtemp = fact * rjl + rjpl;
    fact -= xi;
    rjpl = fact * rjtemp - rjl;
    rjl = rjtemp;
  }
  if (rjl == 0e0) {
    rjl = EPS;
  }
  f = rjpl / rjl;
  if (x < 2) {
    x2 = 0.5 * x;
    pimu = M_PI * xmu;
    fact = (fabs(pimu) < EPS ? 1.0 : pimu / sin(pimu));
    d = -log(x2);
    e = xmu * d;
    fact2 = (fabs(e) < EPS ? 1.0 : sinh(e) / e);
    if (!beschb(xmu, gam1, gam2, gampl, gammi)) {
      return false;
    }
    ff = 2e0 / M_PI * fact * (gam1 * cosh(e) + gam2 * fact2 * d);
    e = exp(e);
    p = e / (gampl * M_PI);
    q = 1e0 / (e * M_PI * gammi);
    pimu2 = 5e-1 * pimu;
    fact3 = (fabs(pimu2) < EPS ? 1.0 : sin(pimu2) / pimu2);
    r = M_PI * pimu2 * fact3 * fact3;
    c = 1e0;
    d = -x2 * x2;
    sum = ff + r * q;
    sum1 = p;
    for (i = 1; i <= MAXIT; i++) {
      ff = (i * ff + p + q) / (i * i - xmu2);
      c *= (d / i);
      p /= (i - xmu);
      q /= (i + xmu);
      del = c * (ff + r * q);
      sum += del;
      del1 = c * p - i * del;
      sum1 += del1;
      if (fabs(del) < (1e0 + fabs(sum)) * EPS) {
        break;
      }
    }
    if (i > MAXIT) {
      std::cout << "nr::bessj series failed to converge" << std::endl;
      return false;
    }
    rymu = -sum;
    ry1 = -sum1 * xi2;
    rymup = xmu * xi * rymu - ry1;
    rjmu = w / (rymup - f * rymu);
  } else {
    a = 0.25 - xmu2;
    p = -0.5 * xi;
    q = 1.0;
    br = 2.0 * x;
    bi = 2.0;
    fact = a * xi / (p * p + q * q);
    cr = br + q * fact;
    ci = bi + p * fact;
    den = br * br + bi * bi;
    dr = br / den;
    di = -bi / den;
    dlr = cr * dr - ci * di;
    dli = cr * di + ci * dr;
    temp = p * dlr - q * dli;
    q = p * dli + q * dlr;
    p = temp;

    for (i = 2; i <= MAXIT; i++) {
      a += 2 * (i - 1);
      bi += 2.0;
      dr = a * dr + br;
      di = a * di + bi;
      if (fabs(dr) + fabs(di) < FPMIN) {
        dr = FPMIN;
      }
      fact = a / (cr * cr + ci * ci);
      cr = br + cr * fact;
      ci = bi - ci * fact;
      if (fabs(cr) + fabs(ci) < FPMIN) {
        cr = FPMIN;
      }
      den = dr * dr + di * di;
      dr /= den;
      di /= -den;
      dlr = cr * dr - ci * di;
      dli = cr * di + ci * dr;
      temp = p * dlr - q * dli;
      q = p * dli + q * dlr;
      p = temp;
      if (fabs(dlr - 1e0) + fabs(dli) < EPS) {
        break;
      }
    }
    if (i > MAXIT) {
      std::cout << "nr::bessj cf2 failed in bessy" << std::endl;
      return false;
    }
    gam = (p - f) / q;
    rjmu = sqrt(w / ((p - f) * gam + q));
    rjmu = (rjl >= 0e0 ? fabs(rjmu) : -fabs(rjmu));
  }
  fact = rjmu / rjl;
  *rj = rjl1 * fact;
  return true;
}

bool sphbes(int n, double x, double &sj) {
  double order;
  double rj;

  if (n < 0 || x < 0e0) {
    std::cout << "nr::sphbes bad arguments n " << n << " x " << x << std::endl;
    return false;
  }

  if (x == 0e0) {
    if (n) {
      sj = 0e0;
    } else {
      sj = 1e0;
    }
    // printf( "sh: %d %.8g %.8g\n", n, x, sj );
    return true;
  }

  order = n + 5e-1;
  if (bessj(x, order, &rj)) {
    sj = (M_SQRT_PI_2 / sqrt(x)) * rj;
    // printf( "sh: %d %.8g %.8g\n", n, x, sj );
    return true;
  }
  return false;
}

}  // namespace nr

namespace sh {
bool spherical_harmonic(int l, int m, double theta, double phi,
                        std::complex<double> &result) {
  bool r_sign = false;
  bool i_sign = false;
  if (m < 0) {
    // Reflect and adjust sign if m < 0:
    r_sign = m & 1;
    i_sign = !(m & 1);
    m = -m;
  }

  if (m & 1) {
    // Check phase if theta is outside [0, PI]:
    double mod = fmod(theta, M_2PI);
    if (mod < 0e0) {
      mod += M_2PI;
    }
    if (mod > M_PI) {
      r_sign = !r_sign;
      i_sign = !i_sign;
    }
  }

  double p;

  if (!nr::plegendre(l, m, cos(theta), p)) {
    return false;
  }

  // printf( "sh pl l %d m %d theta %g p %g\n", l, m, theta, p );
  double mphi = (double)m * phi;
  double r = p * cos(mphi);
  double i = p * sin(mphi);
  if (r_sign) {
    r = -r;
  }
  if (i_sign) {
    i = -i;
  }

  result = std::complex<double>(r, i);

  /*
     printf( "sh pl l %d m %d theta %g p %g phi %g mphi %g cosmphi %g sinmphi %g
     p*cosmphi %g p*sinmphi %g\n", l, m, theta, p, phi, mphi, cos( mphi ), sin(
     mphi ), p * cos( mphi ), p * sin( mphi )
     );
  */
  // std::cout << "sh l " << l << " m " << m << " theta " << theta << " phi " <<
  // phi << " result " << result << std::endl;
  return true;
}

bool conj_spherical_harmonic(int l, int m, double theta, double phi,
                             std::complex<double> &result) {
  bool r_sign = false;
  bool i_sign = false;
  if (m < 0) {
    // Reflect and adjust sign if m < 0:
    r_sign = m & 1;
    i_sign = !(m & 1);
    m = -m;
  }

  if (m & 1) {
    // Check phase if theta is outside [0, PI]:
    double mod = fmod(theta, M_2PI);
    if (mod < 0e0) {
      mod += M_2PI;
    }
    if (mod > M_PI) {
      r_sign = !r_sign;
      i_sign = !i_sign;
    }
  }

  double p;

  if (!nr::plegendre(l, m, cos(theta), p)) {
    return false;
  }

  // printf( "sh pl l %d m %d theta %g p %g\n", l, m, theta, p );
  double mphi = (double)m * phi;
  double r = p * cos(mphi);
  double i = p * sin(mphi);
  if (r_sign) {
    r = -r;
  }
  if (!i_sign) {
    i = -i;
  }

  result = std::complex<double>(r, i);

  // printf( "csh: %d %d %f %f (%.8g,%.8g)\n", l, m, theta, phi, r, i );
  return true;
}

bool alt_conj_sh(int max_harmonics, double theta, double phi,
                 std::complex<float> *Yp) {
  double mod = fmod(theta, M_2PI);
  if (mod < 0e0) {
    mod += M_2PI;
  }

  double p;
  double mphi;
  double r;
  double i;

  if (mod > M_PI) {
    for (int l = 0; l <= max_harmonics; ++l) {
      for (int m = -(int)l; m < 0; ++m) {
        mphi = (double)m * phi;

        nr::plegendre(l, -m, cos(theta), p);

        mphi = (double)m * phi;
        r = p * cos(mphi);
        i = p * sin(mphi);

        *Yp = std::complex<float>(r, i);
        ++Yp;
      }

      for (int m = 0; m <= (int)l; ++m) {
        mphi = (double)m * phi;

        nr::plegendre(l, m, cos(theta), p);

        mphi = (double)m * phi;
        r = p * cos(mphi);
        i = p * sin(mphi);

        if (m & 1) {
          r = -r;
        } else {
          i = -i;
        }
        *Yp = std::complex<float>(r, i);
        ++Yp;
      }
    }
  } else {
    for (int l = 0; l <= max_harmonics; ++l) {
      for (int m = -(int)l; m < 0; ++m) {
        mphi = (double)m * phi;

        nr::plegendre(l, -m, cos(theta), p);

        mphi = (double)m * phi;
        r = p * cos(mphi);
        i = p * sin(mphi);

        if (m & 1) {
          r = -r;
          i = -i;
        }
        *Yp = std::complex<float>(r, i);
        ++Yp;
      }

      for (int m = 0; m <= (int)l; ++m) {
        mphi = (double)m * phi;

        nr::plegendre(l, m, cos(theta), p);

        mphi = (double)m * phi;
        r = p * cos(mphi);
        i = -p * sin(mphi);

        *Yp = std::complex<float>(r, i);
        ++Yp;
      }
    }
  }
  return true;
}

unsigned int fibonacci(unsigned int n) {
  if (n == 0) {
    return 0;
  } else {
    if (n == 1) {
      return 1;
    } else {
      return fibonacci(n - 1) + fibonacci(n - 2);
    }
  }
}

void build_grid(std::vector<std::vector<double> > &fib_grid,
                unsigned int size) {
  unsigned int f = fibonacci(size);
  unsigned int f_1 = fibonacci(size - 1);
  fib_grid.resize(f);

  std::vector<double> tmp_f(2);

  for (unsigned int i = 0; i < f; i++) {
    tmp_f[0] = acos(1e0 - 2e0 * i / (double)f);
    tmp_f[1] = M_2PI * (double)((i * size) % f_1) / (double)f_1;
    fib_grid[i] = tmp_f;
  }
}
}  // namespace sh
