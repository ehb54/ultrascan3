#include "shs.h"

void SHS::build(int max_harmonics, shd_double delta, shd_double max_x) {
  this->max_harmonics = max_harmonics;

  x_t.clear();
  y_t.clear();
  y2_t.clear();

  for (shd_double tmp_x = 0; tmp_x <= max_x; tmp_x += delta) {
    x_t.push_back(tmp_x);
  }

  int x_size = (int)x_t.size();

  vector<shd_double> tmp_y(x_size);
  vector<shd_double> tmp_y2(x_size);

  for (int i = 0; i <= max_harmonics; ++i) {
    for (int j = 0; j < x_size; ++j) {
      nr::sphbes(i, x_t[j], tmp_y[j]);
    }
    natural_spline(x_t, tmp_y, tmp_y2);
    y_t.push_back(tmp_y);
    y2_t.push_back(tmp_y2);
  }

  // ok, so we *should* have all we need

  // retime to compute via sphbes

  {
    clock_t start_time = clock();
    shd_double res;

    for (int i = 0; i <= max_harmonics; ++i) {
      for (int j = 0; j < x_size; ++j) {
        nr::sphbes(i, x_t[j], res);
      }
    }
    clock_t shbes_time = clock() - start_time;

    printf("compute via sphbes: %gms\n", shbes_time * 1e3 / CLOCKS_PER_SEC);
  }

  // retime to compute via natural_spline

  {
    clock_t start_time = clock();
    shd_double res;

    vector<shd_double> yv(y_t.size());

    for (int j = 0; j < x_size; ++j) {
      apply_natural_spline(x_t[j], yv);
    }
    clock_t shbes_time = clock() - start_time;

    printf("compute via natural_spline (full vectoring): %gms\n",
           shbes_time * 1e3 / CLOCKS_PER_SEC);
  }
}

bool SHS::apply_natural_spline(vector<shd_double> &xa, vector<shd_double> &ya,
                               vector<shd_double> &y2a, shd_double x,
                               shd_double &y) {
  unsigned int klo = 0;
  unsigned int khi = xa.size() - 1;

  while (khi - klo > 1) {
    unsigned int k = (khi + klo) >> 1;
    if (xa[k] > x) {
      khi = k;
    } else {
      klo = k;
    }
  }

  if (khi == klo) {
    error_msg = "US_Saxs_Util::apply_natural_spline error finding point";
    return false;
  }

  shd_double h = xa[khi] - xa[klo];

  if (h <= 0e0) {
    error_msg = "US_Saxs_Util::apply_natural_spline zero or negative interval";
    return false;
  }

  shd_double a = (xa[khi] - x) / h;
  shd_double b = (x - xa[klo]) / h;

  y = a * ya[klo] + b * ya[khi] +
      ((a * a * a - a) * y2a[klo] + (b * b * b - b) * y2a[khi]) * (h * h) / 6e0;

  return true;
}

void SHS::natural_spline(vector<shd_double> &x, vector<shd_double> &y,
                         vector<shd_double> &y2) {
  double p;
  double qn;
  double sig;
  double un;
  vector<double> u(x.size());

  y2.resize(x.size());

  y2[0] = u[0] = 0e0;

  for (unsigned int i = 1; i < x.size() - 1; i++) {
    sig = (x[i] - x[i - 1]) / (x[i + 1] - x[i - 1]);
    p = sig * y2[i - 1] + 2e0;
    y2[i] = (sig - 1e0) / p;
    u[i] = (y[i + 1] - y[i]) / (x[i + 1] - x[i]) -
           (y[i] - y[i - 1]) / (x[i] - x[i - 1]);
    u[i] = (6e0 * u[i] / (x[i + 1] - x[i - 1]) - sig * u[i - 1]) / p;
  }

  qn = un = 0e0;

  y2[x.size() - 1] = 0e0;

  for (unsigned int k = x.size() - 2; k >= 1; k--) {
    y2[k] = y2[k] * y2[k + 1] + u[k];
  }
}

void SHS::natural_spline(vector<shd_double> &x, vector<shd_double> &y,
                         vector<shd_double> &y2, shd_double yp1,
                         shd_double ypn) {
  shd_double p;
  shd_double qn;
  shd_double sig;
  shd_double un;
  vector<shd_double> u(x.size());

  unsigned int n = x.size();

  y2.resize(x.size());

  y2[0] = -0.5e0;
  u[0] = (3.0 / (x[1] - x[0])) * ((y[1] - y[0]) / (x[1] - x[0]) - yp1);

  for (unsigned int i = 1; i < x.size() - 1; i++) {
    sig = (x[i] - x[i - 1]) / (x[i + 1] - x[i - 1]);
    p = sig * y2[i - 1] + 2e0;
    y2[i] = (sig - 1e0) / p;
    u[i] = (y[i + 1] - y[i]) / (x[i + 1] - x[i]) -
           (y[i] - y[i - 1]) / (x[i] - x[i - 1]);
    u[i] = (6e0 * u[i] / (x[i + 1] - x[i - 1]) - sig * u[i - 1]) / p;
  }

  qn = 0.5e0;
  un = (3.0 / (x[n - 1] - x[n - 2])) *
       (ypn - (y[n - 1] - y[n - 2]) / (x[n - 1] - x[n - 2]));

  y2[n - 1] = (un - qn * u[n - 2]) / (qn * y2[n - 2] + 1e0);

  for (unsigned int k = x.size() - 2; k >= 1; k--) {
    y2[k] = y2[k] * y2[k + 1] + u[k];
  }
}

bool SHS::apply_natural_spline(shd_double x, vector<shd_double> &yv) {
  shd_double *xp;
  shd_double *yp;

  vector<shd_double> *ya;
  vector<shd_double> *y2a;

  unsigned int klo = 0;
  unsigned int khi = x_t.size() - 1;

  while (khi - klo > 1) {
    unsigned int k = (khi + klo) >> 1;
    if (x_t[k] > x) {
      khi = k;
    } else {
      klo = k;
    }
  }

  shd_double h = x_t[khi] - x_t[klo];

  if (h <= 0e0) {
    error_msg = "US_Saxs_Util::apply_natural_spline zero or negative interval";
    return false;
  }

  if (khi == klo) {
    error_msg = "US_Saxs_Util::apply_natural_spline error finding point";
    return false;
  }

  shd_double a = (x_t[khi] - x) / h;
  shd_double b = (x - x_t[klo]) / h;

  shd_double hho6 = h * h / 6e0;

  shd_double aaama = a * a * a - a;
  shd_double bbbmb = b * b * b - b;
  for (int i = 0; i <= max_harmonics; ++i) {
    ya = &(y_t[i]);
    y2a = &(y2_t[i]);

    yv[i] = a * (*ya)[klo] + b * (*ya)[khi] +
            (aaama * (*y2a)[klo] + bbbmb * (*y2a)[khi]) * hho6;
  }
  return true;
}

bool SHS::write_c_table(string oname) {
  ofstream ofs(oname.c_str(), ios::out);

  cout << ">" << oname << endl;

  ofs << "// tables for sphbes" << endl;

  ofs << "static int        shd_max_harmonics = " << (y_t.size() - 1) << ";"
      << endl;
  ofs << "static int        shd_x_size        = " << (x_t.size()) << ";"
      << endl;
  ofs << "static shd_double shd_max_x         = " << (x_t.back()) << ";"
      << endl;

  ofs << "static shd_double shbes_x[ " << x_t.size() << " ] =" << endl;
  ofs << "  {" << endl;
  for (int i = 0; i < x_t.size() - 1; ++i) {
    ofs << "   " << x_t[i] << "," << endl;
  }
  ofs << "   " << x_t.back() << endl;
  ofs << "  };" << endl;

  ofs << "static shd_double shbes_y[ " << y_t.size() << " ][ " << y_t[0].size()
      << " ] =" << endl;
  ofs << "  {" << endl;
  for (int i = 0; i < (int)y_t.size() - 1; ++i) {
    ofs << "   {" << endl;
    for (int j = 0; j < (int)y_t[i].size() - 1; ++j) {
      ofs << "    " << y_t[i][j] << "," << endl;
    }
    ofs << "   " << y_t[i].back() << endl;
    ofs << "   }," << endl;
  }
  ofs << "   {" << endl;
  for (int j = 0; j < (int)y_t.back().size() - 1; ++j) {
    ofs << "    " << y_t.back()[j] << "," << endl;
  }
  ofs << "   " << y_t.back().back() << endl;
  ofs << "  }" << endl;
  ofs << " };" << endl;

  ofs << "static shd_double shbes_y2[ " << y2_t.size() << " ][ "
      << y2_t[0].size() << " ] =" << endl;
  ofs << "  {" << endl;
  for (int i = 0; i < (int)y2_t.size() - 1; ++i) {
    ofs << "   {" << endl;
    for (int j = 0; j < (int)y2_t[i].size() - 1; ++j) {
      ofs << "    " << y2_t[i][j] << "," << endl;
    }
    ofs << "   " << y2_t[i].back() << endl;
    ofs << "   }," << endl;
  }
  ofs << "   {" << endl;
  for (int j = 0; j < (int)y2_t.back().size() - 1; ++j) {
    ofs << "    " << y2_t.back()[j] << "," << endl;
  }
  ofs << "   " << y2_t.back().back() << endl;
  ofs << "  }" << endl;
  ofs << " };" << endl;

  ofs.close();
}
