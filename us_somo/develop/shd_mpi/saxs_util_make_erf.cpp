#include <math.h>
#include <stdint.h>

#include <algorithm>
#include <complex>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

void natural_spline(vector<double> &x, vector<double> &y, vector<double> &y2) {
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

int main(int, char **) {
  vector<double> x;
  vector<double> y;

  for (double p = -6e0; p <= 6.01e0;) {
    x.push_back(p);
    y.push_back(erf(x.back()));
    if (fabs(y.back()) > .9) {
      p += 0.01;
    } else {
      if (fabs(y.back()) > .8) {
        p += 0.005;
      } else {
        if (fabs(y.back()) > .6) {
          p += 0.002;
        } else {
          if (fabs(y.back()) > .3) {
            p += 0.001;
          } else {
            if (fabs(y.back()) > .1) {
              p += 0.0005;
            } else {
              p += 0.00025;
            }
          }
        }
      }
    }
  }

  vector<double> y2;
  natural_spline(x, y, y2);

  int pts = (int)x.size();

  printf("#define USUS_ERF_MAX_PT %d\n\n", pts - 1);

  printf("static double erf_x[ %d ] = {\n", pts);
  for (int i = 0; i < pts; i++) {
    if (i) {
      printf("   ,");
    } else {
      printf("   ");
    }
    printf("%.20g\n", x[i]);
  }

  printf("};\n");

  printf("static double erf_y[ %d ] = {\n", pts);
  for (int i = 0; i < pts; i++) {
    if (i) {
      printf("   ,");
    } else {
      printf("   ");
    }
    printf("%.20g\n", y[i]);
  }

  printf("};\n");

  printf("static double erf_y2[ %d ] = {\n", pts);
  for (int i = 0; i < pts; i++) {
    if (i) {
      printf("   ,");
    } else {
      printf("   ");
    }
    printf("%.20g\n", y2[i]);
  }

  printf("};\n");
}
