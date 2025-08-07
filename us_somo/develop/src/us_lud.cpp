#include "../include/us_lud.h"

bool US_LUD::LUPDecompose(double **A, int N, double Tol, int *P) {
  int i, j, k, imax;
  double maxA, *ptr, absA;

  for (i = 0; i <= N; i++) {
    P[i] = i;  // Unit permutation matrix, P[N] initialized with N
  }

  for (i = 0; i < N; i++) {
    maxA = 0.0;
    imax = i;

    for (k = i; k < N; k++) {
      if ((absA = fabs(A[k][i])) > maxA) {
        maxA = absA;
        imax = k;
      }
    }

    if (maxA < Tol) {
      errormsg = "Error: LUPDecompose: maxA < Tolerance\n";
      return false;  // failure, matrix is degenerate
    }

    if (imax != i) {
      // pivoting P
      j = P[i];
      P[i] = P[imax];
      P[imax] = j;

      // pivoting rows of A
      ptr = A[i];
      A[i] = A[imax];
      A[imax] = ptr;

      // counting pivots starting from N (for determinant)
      P[N]++;
    }

    for (j = i + 1; j < N; j++) {
      A[j][i] /= A[i][i];

      for (k = i + 1; k < N; k++) A[j][k] -= A[j][i] * A[i][k];
    }
  }

  return true;  // decomposition done
}

/* INPUT: A,P filled in LUPDecompose; b - rhs vector; N - dimension
 * OUTPUT: x - solution vector of A*x=b
 */
void US_LUD::LUPSolve(double **A, int *P, double *b, int N, double *x) {
  for (int i = 0; i < N; i++) {
    x[i] = b[P[i]];

    for (int k = 0; k < i; k++) x[i] -= A[i][k] * x[k];
  }

  for (int i = N - 1; i >= 0; i--) {
    for (int k = i + 1; k < N; k++) x[i] -= A[i][k] * x[k];

    x[i] = x[i] / A[i][i];
  }
}

bool US_LUD::caruanas(const vector<double> &q, const vector<double> &I,
                      double &mu, double &sigma, double &amp) {
  errormsg = "";

  vector<vector<double> > Av(3);
  Av[0].resize(3);
  Av[1].resize(3);
  Av[2].resize(3);

  vector<double> B(3);

  int N = (int)q.size();

  double sumq = 0e0;
  double sumq2 = 0e0;
  double sumq3 = 0e0;
  double sumq4 = 0e0;

  double sumlnI = 0e0;
  double sumqlnI = 0e0;
  double sumq2lnI = 0e0;

  for (int i = 0; i < N; ++i) {
    double tq = q[i];
    double tI = I[i];
    if (tI <= 0) {
      errormsg = "Error: Intensity values less than or equal zero\n";
      return false;
    }

    double tq2 = tq * tq;
    double tq3 = tq2 * tq;
    double tq4 = tq2 * tq2;

    double lnI = log(tI);
    double qlnI = tq * lnI;
    double q2lnI = tq2 * lnI;

    sumq += tq;
    sumq2 += tq2;
    sumq3 += tq3;
    sumq4 += tq4;

    sumlnI += lnI;
    sumqlnI += qlnI;
    sumq2lnI += q2lnI;
  }

  Av[0][0] = (double)N;
  Av[0][1] = sumq;
  Av[0][2] = sumq2;

  Av[1][0] = sumq;
  Av[1][1] = sumq2;
  Av[1][2] = sumq3;

  Av[2][0] = sumq2;
  Av[2][1] = sumq3;
  Av[2][2] = sumq4;

  B[0] = sumlnI;
  B[1] = sumqlnI;
  B[2] = sumq2lnI;

  vector<double *> A(3);
  A[0] = (double *)&Av[0][0];
  A[1] = (double *)&Av[1][0];
  A[2] = (double *)&Av[2][0];

  vector<int> P(4);

  double Tol = 1e-10;

  // for ( int i = 0; i < 3; ++i ) {
  //    for ( int j = 0; j < 3; ++j ) {
  //       cout << A[i][j] << " ";
  //    }
  //    cout << endl;
  // }

  if (!LUPDecompose((double **)&A[0], 3, Tol, (int *)&P[0])) {
    return false;
  }

  vector<double> x(3);

  LUPSolve((double **)&A[0], (int *)&P[0], (double *)&B[0], 3, (double *)&x[0]);

  // for ( int i = 0; i < 3; ++i ) {
  //    printf( "x[%d] = %f\n", i, x[i] );
  // }

  if (x[2] >= 0e0) {
    errormsg = "caruanas: resulting c is not negative";
    return false;
  }

  mu = -5e-1 * x[1] / x[2];
  sigma = sqrt(-5e-1 / x[2]);
  amp = exp(x[0] - ((x[1] * x[1]) / (4 * x[2])));

  return true;
}

bool US_LUD::guos(const vector<double> &q, const vector<double> &I, double &mu,
                  double &sigma, double &amp) {
  errormsg = "";

  vector<vector<double> > Av(3);
  Av[0].resize(3);
  Av[1].resize(3);
  Av[2].resize(3);

  vector<double> B(3);

  int N = (int)q.size();

  double sumI2 = 0e0;
  double sumqI2 = 0e0;
  double sumq2I2 = 0e0;
  double sumq3I2 = 0e0;
  double sumq4I2 = 0e0;

  double sumI2lnI = 0e0;
  double sumqI2lnI = 0e0;
  double sumq2I2lnI = 0e0;

  for (int i = 0; i < N; ++i) {
    double tq = q[i];
    double tI = I[i];
    if (tI <= 0) {
      errormsg = "Error: Intensity values less than or equal zero\n";
      return false;
    }

    double tI2 = tI * tI;

    double tq2 = tq * tq;
    double tq3 = tq2 * tq;
    double tq4 = tq2 * tq2;

    double tqI2 = tq * tI2;
    double tq2I2 = tq2 * tI2;
    double tq3I2 = tq3 * tI2;
    double tq4I2 = tq4 * tI2;

    double lnI = log(tI);
    double I2lnI = tI2 * lnI;
    ;
    double qI2lnI = tq * I2lnI;
    double q2I2lnI = tq2 * I2lnI;

    sumI2 += tI2;
    sumqI2 += tqI2;
    sumq2I2 += tq2I2;
    sumq3I2 += tq3I2;
    sumq4I2 += tq4I2;

    sumI2lnI += I2lnI;
    sumqI2lnI += qI2lnI;
    sumq2I2lnI += q2I2lnI;
  }

  Av[0][0] = sumI2;
  Av[0][1] = sumqI2;
  Av[0][2] = sumq2I2;

  Av[1][0] = sumqI2;
  Av[1][1] = sumq2I2;
  Av[1][2] = sumq3I2;

  Av[2][0] = sumq2I2;
  Av[2][1] = sumq3I2;
  Av[2][2] = sumq4I2;

  B[0] = sumI2lnI;
  B[1] = sumqI2lnI;
  B[2] = sumq2I2lnI;

  vector<double *> A(3);
  A[0] = (double *)&Av[0][0];
  A[1] = (double *)&Av[1][0];
  A[2] = (double *)&Av[2][0];

  vector<int> P(4);

  double Tol = 1e-50;

  // cout << "A:\n";
  // for ( int i = 0; i < 3; ++i ) {
  //    for ( int j = 0; j < 3; ++j ) {
  //       cout << A[i][j] << " ";
  //    }
  //    cout << endl;
  // }

  // cout << "B:\n";
  // for ( int i = 0; i < 3; ++i ) {
  //    cout << B[i] << " ";
  // }
  // cout << endl;

  if (!LUPDecompose((double **)&A[0], 3, Tol, (int *)&P[0])) {
    return false;
  }

  vector<double> x(3);

  LUPSolve((double **)&A[0], (int *)&P[0], (double *)&B[0], 3, (double *)&x[0]);

  for (int i = 0; i < 3; ++i) {
    printf("x[%d] = %f\n", i, x[i]);
  }

  if (x[2] >= 0e0) {
    errormsg = "guos: resulting c is not negative";
    return false;
  }

  mu = -5e-1 * x[1] / x[2];
  sigma = sqrt(-5e-1 / x[2]);
  amp = exp(x[0] - ((x[1] * x[1]) / (4 * x[2])));

  return true;
}
