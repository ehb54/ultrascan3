#include "../include/us_saxs_util.h"

// note: this program uses cout and/or cerr and this should be replaced

static std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os,
                                            const QString& str) {
  return os << qPrintable(str);
}

bool US_Saxs_Util::bspline_prepare(unsigned int knots, unsigned int degree) {
  // http://en.wikipedia.org/wiki/B-spline
  // uniform b spline

  if (!sgp_exp_q.size()) {
    errormsg = "Error: prepare bspline: q grid must be defined first";
    return false;
  }

  if (knots < 3 || degree < 3) {
    errormsg = "Error: prepare bspline: knots & N must be at least 3";
    return false;
  }

  if (knots - degree - 1 <= 0) {
    errormsg =
        "Error: prepare bspline: knots - degree - 1 must be greater than zero";
    return false;
  }

  if (sgp_exp_q.size() < knots || sgp_exp_q.size() < degree) {
    errormsg =
        "Error: prepare bspline: q grid must be bigger than number of knots or "
        "N";
    return false;
  }

  bspline_net.clear();
  bspline_omega.clear();
  bspline_degree = degree;
  bspline_basis_functions = knots - degree - 1;

  double range = sgp_exp_q.back() - sgp_exp_q[0];

  double delta = range / (double)(knots - 1);
  for (unsigned int i = 0; i < knots; i++) {
    bspline_net.push_back(sgp_exp_q[0] + i * delta);
  }

  // compute omega:

  for (unsigned int k = 0; k < bspline_basis_functions; k++) {
    vector<double> this_omega;
    for (unsigned int i = k; i <= k + degree + 1; i++) {
      double this_i_omega = 1e0;
      for (unsigned int ii = k; ii <= k + degree + 1; ii++) {
        if (ii != i && ii < knots && i < knots) {
          this_i_omega *= bspline_net[ii] - bspline_net[i];
        }
      }
      this_omega.push_back(1e0 / this_i_omega);
    }
    bspline_omega.push_back(this_omega);
  }

  // debugging output:

  QString qs;

  qs += "bspline net:";

  for (unsigned int i = 0; i < bspline_net.size(); i++) {
    qs += QString(" %1").arg(bspline_net[i]);
  }

  qs += QString("\nbspline basis functions: %1\n").arg(bspline_basis_functions);

  qs += "bspline omega:\n";

  for (unsigned int i = 0; i < bspline_omega.size(); i++) {
    qs += QString("bspline basis function %1 omegas: ").arg(i);
    for (unsigned int j = 0; j < bspline_omega[i].size(); j++) {
      qs += QString(" %1").arg(bspline_omega[i][j]);
    }
    qs += "\n";
  }

  cout << qs;

  return true;
}

bool US_Saxs_Util::bspline_basis(unsigned int basis, double q, double& value) {
  if (basis > bspline_basis_functions) {
    errormsg = QString(
                   "Error: bspline_basis: requested basis function %1 but only "
                   "%2 defined")
                   .arg(basis)
                   .arg(bspline_basis_functions);
    return false;
  }

  value = 0e0;
  for (unsigned int i = basis;
       i <= basis + bspline_degree + 1 /* && i < bspline_net.size() */; i++) {
    double diff = q - bspline_net[i];
    if (diff > 0e0) {
      // double this_value = pow( diff, bspline_degree );
      value += pow(diff, (int)bspline_degree) * bspline_omega[basis][i - basis];
      // check omega:
      double omega = 1e0;
      for (unsigned int j = basis; j <= basis + bspline_degree + 1; j++) {
        if (i != j) {
          omega *= bspline_net[j] - bspline_net[i];
        }
      }
      if (fabs(1e0 / omega - bspline_omega[basis][i - basis]) > 1e-5) {
        cout << QString("stored omega different basis %1 pos %2 %3 %4\n")
                    .arg(basis)
                    .arg(i)
                    .arg(1e0 / omega)
                    .arg(bspline_omega[basis][i - basis]);
      }
      // this_value /= omega;
      // value += this_value;
    }
  }
  value *= ((double)bspline_degree + 1e0);  // / ( ( double ) bspline_degree );

  return true;
}

bool US_Saxs_Util::bspline_test() {
  sgp_exp_q.clear();
  for (double q = 0e0; q < 7e0; q += .1) {
    sgp_exp_q.push_back(q);
  }

  if (!bspline_prepare(10, 3)) {
    return false;
  }

  for (unsigned int basis = 0; basis < bspline_basis_functions; basis++) {
    cout << QString("basis function %1: q B(q)\n").arg(basis);
    for (double q = sgp_exp_q[0]; q <= 7e0 /* sgp_exp_q.back() */; q += .05) {
      double value;
      if (!bspline_basis(basis, q, value)) {
        return false;
      }
      cout << QString("%1 %2\n").arg(q).arg(value);
    }
  }
  return true;
}
