#ifndef US_LUD_H
#define US_LUD_H

#include <math.h>
#include <qstring.h>
#include <stdio.h>

#include <iostream>
#include <vector>

using namespace std;

class US_LUD {
 public:
  bool caruanas(const vector<double> &q, const vector<double> &I, double &mu,
                double &sigma, double &amp);

  bool guos(const vector<double> &q, const vector<double> &I, double &mu,
            double &sigma, double &amp);

  QString errormsg;

 private:
  bool LUPDecompose(double **A, int N, double Tol, int *P);
  void LUPSolve(double **A, int *P, double *b, int N, double *x);
};

#endif  // US_LUD_H
