#ifndef SHS_H
#define SHS_H

#include <math.h>
typedef double shd_double;
#include <time.h>

#include <string>
#include <vector>

#include "shd.h"
#include "shd_global.h"
#include "shd_sh.h"

class SHS {
 private:
  vector<shd_double> x_t;
  vector<vector<shd_double> > y_t;
  vector<vector<shd_double> > y2_t;

  string error_msg;
  int max_harmonics;

 public:
  // SHS();
  // ~SHS();

  void build(int max_harmonics, shd_double delta, shd_double max_x);

  bool apply_natural_spline(vector<shd_double> &xa, vector<shd_double> &ya,
                            vector<shd_double> &y2a, shd_double x,
                            shd_double &y);

  void natural_spline(vector<shd_double> &x, vector<shd_double> &y,
                      vector<shd_double> &y2);

  void natural_spline(vector<shd_double> &x, vector<shd_double> &y,
                      vector<shd_double> &y2, shd_double yp1, shd_double ypn);

  bool apply_natural_spline(shd_double x, vector<shd_double> &yv);

  bool write_c_table(string oname);
};

#endif  // SHS
