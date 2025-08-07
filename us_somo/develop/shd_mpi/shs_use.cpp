#include "../shd_mpi/shs_use.h"

#include "../shd_mpi/shs_data.h"

SHS_USE::SHS_USE(int max_harmonics) {
  this->max_harmonics = max_harmonics;
  if (max_harmonics > shd_max_harmonics) {
    cerr << "Error: Requested " << max_harmonics
         << " which is greater than maximum possible harmonics of "
         << shd_max_harmonics << endl;
    exit(-1);
  }
}

bool SHS_USE::shs_compute_sphbes(shd_double x, shd_double *yv) {
  if (x > shd_max_x) {
    error_msg = "SHS_USE::x too large";
    return false;
  }

  // shd_double *xp;
  // shd_double *yp;

  shd_double *ya;
  shd_double *y2a;

  unsigned int klo = 0;
  unsigned int khi = shd_x_size - 1;

  while (khi - klo > 1) {
    unsigned int k = (khi + klo) >> 1;
    if (shbes_x[k] > x) {
      khi = k;
    } else {
      klo = k;
    }
  }

  shd_double h = shbes_x[khi] - shbes_x[klo];

  if (h <= 0e0) {
    error_msg = "SHS_USE::apply_natural_spline zero or negative interval";
    return false;
  }

  if (khi == klo) {
    error_msg = "SHS_USE::apply_natural_spline error finding point";
    return false;
  }

  shd_double a = (shbes_x[khi] - x) / h;
  shd_double b = (x - shbes_x[klo]) / h;

  shd_double hho6 = h * h / 6e0;

  shd_double aaama = a * a * a - a;
  shd_double bbbmb = b * b * b - b;
  for (int i = 0; i <= max_harmonics; ++i) {
    ya = &(shbes_y[i][0]);
    y2a = &(shbes_y2[i][0]);

    yv[i] = a * ya[klo] + b * ya[khi] +
            (aaama * y2a[klo] + bbbmb * y2a[khi]) * hho6;
    // if ( isnan( yv[ i ] ) )
    // {
    //    error_msg = "SHS_USE::is nan";
    //    return false;
    // }
  }
  return true;
}
