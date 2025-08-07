/*
 * svdcomp - SVD decomposition routine.
 * Takes an mxn matrix a and decomposes it into udv, where u,v are
 * left and right orthogonal transformation matrices, and d is a
 * diagonal matrix of singular values.
 *
 * This routine is adapted from svdecomp.c in XLISP-STAT 2.1 which is
 * code from Numerical Recipes adapted by Luke Tierney and David Betz.
 *
 * Input to dsvd is as follows:
 *   a = mxn matrix to be decomposed, gets overwritten with u
 *       *** if m < n, a must be preallocated nxn
 *   m = row dimension of a
 *   n = column dimension of a
 *   w = preallocated n-vector returns the vector of singular values of a
 *   v = preallocated nxn matrix returns the right orthogonal transformation
 * matrix
 */

#include "../include/us_svd.h"

namespace SVD {

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define SIGN(a, b) ((b) >= 0.0 ? fabs(a) : -fabs(a))

char errormsg[128];

static double PYTHAG(double a, double b) {
  double at = fabs(a), bt = fabs(b), ct, result;

  if (at > bt) {
    ct = bt / at;
    result = at * sqrt(1.0 + ct * ct);
  } else if (bt > 0.0) {
    ct = at / bt;
    result = bt * sqrt(1.0 + ct * ct);
  } else
    result = 0.0;
  return (result);
}

void dvalidate(double **a, int m, int n, double *w, double **v) {
  double sum;
  cout << "SVD::dvalidate\n";
  cout << "SVD::dvalidate m " << m << "\n";
  cout << "SVD::dvalidate n " << n << "\n";

  cout << "SVD::dvalidate A\n";
  sum = 0e0;
  for (int i = 0; i < m; ++i) {
    for (int j = 0; j < n; ++j) {
      sum += a[i][j];
    }
  }
  cout << "SVD::dvalidate A ok " << sum << "\n";

  cout << "SVD::dvalidate w\n";
  sum = 0e0;
  for (int j = 0; j < n; ++j) {
    sum += w[j];
  }
  cout << "SVD::dvalidate w ok " << sum << "\n";

  cout << "SVD::dvalidate v\n";
  sum = 0e0;
  for (int i = 0; i < n; ++i) {
    for (int j = 0; j < n; ++j) {
      sum += v[i][j];
    }
  }
  cout << "SVD::dvalidate v ok " << sum << "\n";
}

void dinfo(double **a, int m, int n, double * /* w */, double ** /* v */) {
  cout << "SVD::dinfo\n";
  cout << "SVD::dinfo m " << m << "\n";
  cout << "SVD::dinfo n " << n << "\n";

  cout << "SVD::dinfo A\n";
  for (int i = 0; i < m; ++i) {
    cout << i << ":";
    for (int j = 0; j < n; ++j) {
      cout << "\t" << a[i][j];
    }
    cout << "\n";
  }
  cout << "\n";
}

bool dsvd(double **a, int m, int n, double *w, double **v) {
  int flag, i, its, j, jj, k, l, nm;
  double c, f, h, s, x, y, z;
  double anorm = 0.0, g = 0.0, scale = 0.0;
  double *rv1;

  if (m < n) {
    /*
     * Input to dsvd is as follows:
     *   a = mxn matrix to be decomposed, gets overwritten with u
     *       *** if m < n, a must be preallocated nxn
     *   m = row dimension of a
     *   n = column dimension of a
     *   w = preallocated n-vector returns the vector of singular values of a
     *   v = preallocated nxn matrix returns the right orthogonal transformation
     * matrix
     */

    // transpose a to an nxm
    double aT[n][m];
    for (int i = 0; i < m; ++i) {
      for (int j = 0; j < n; ++j) {
        aT[j][i] = a[i][j];
      }
    }

    double *aTuse[n];
    for (int j = 0; j < n; ++j) {
      aTuse[j] = aT[j];
    }

    // w can stay the same?

    double wT[m];

    // allocate vt
    double vT[m][m];

    double *vTuse[m];

    for (int i = 0; i < m; ++i) {
      vTuse[i] = vT[i];
    }

    // cout_dpp( "dsvd aTuse", aTuse, n, m );

    if (!dsvd(aTuse, n, m, wT, vTuse)) {
      return false;
    }

    // cout_dpp( "dsvd aTuse after dsvd", aTuse, n, m );
    // cout_dp( "dsvd wT    after dsvd", wT, n );
    // cout_dpp( "dsvd vTuse after dsvd", vTuse, m, n );

    // need to put back results in reverse transposed

    for (int i = 0; i < m; ++i) {
      for (int j = 0; j < m; ++j) {
        a[i][j] = vT[j][i];
      }
    }

    for (int i = 0; i < m; ++i) {
      w[i] = wT[i];
    }
    for (int i = m; i < n; ++i) {
      w[i] = 0e0;
    }

    for (int i = 0; i < n; ++i) {
      for (int j = 0; j < n; ++j) {
        v[i][j] = aT[j][i];
      }
    }

    return true;

    // strcpy( errormsg, "SVD: Error: #rows must be > #cols" );
    // return false;
  }

  rv1 = (double *)malloc((unsigned int)n * sizeof(double));

  /* Householder reduction to bidiagonal form */
  for (i = 0; i < n; i++) {
    /* left-hand reduction */
    l = i + 1;
    rv1[i] = scale * g;
    g = s = scale = 0.0;
    if (i < m) {
      for (k = i; k < m; k++) scale += fabs((double)a[k][i]);
      if (scale) {
        for (k = i; k < m; k++) {
          a[k][i] = (double)((double)a[k][i] / scale);
          s += ((double)a[k][i] * (double)a[k][i]);
        }
        f = (double)a[i][i];
        g = -SIGN(sqrt(s), f);
        h = f * g - s;
        a[i][i] = (double)(f - g);
        if (i != n - 1) {
          for (j = l; j < n; j++) {
            for (s = 0.0, k = i; k < m; k++)
              s += ((double)a[k][i] * (double)a[k][j]);
            f = s / h;
            for (k = i; k < m; k++) a[k][j] += (double)(f * (double)a[k][i]);
          }
        }
        for (k = i; k < m; k++) a[k][i] = (double)((double)a[k][i] * scale);
      }
    }
    w[i] = (double)(scale * g);

    /* right-hand reduction */
    g = s = scale = 0.0;
    if (i < m && i != n - 1) {
      for (k = l; k < n; k++) scale += fabs((double)a[i][k]);
      if (scale) {
        for (k = l; k < n; k++) {
          a[i][k] = (double)((double)a[i][k] / scale);
          s += ((double)a[i][k] * (double)a[i][k]);
        }
        f = (double)a[i][l];
        g = -SIGN(sqrt(s), f);
        h = f * g - s;
        a[i][l] = (double)(f - g);
        for (k = l; k < n; k++) rv1[k] = (double)a[i][k] / h;
        if (i != m - 1) {
          for (j = l; j < m; j++) {
            for (s = 0.0, k = l; k < n; k++)
              s += ((double)a[j][k] * (double)a[i][k]);
            for (k = l; k < n; k++) a[j][k] += (double)(s * rv1[k]);
          }
        }
        for (k = l; k < n; k++) a[i][k] = (double)((double)a[i][k] * scale);
      }
    }
    anorm = MAX(anorm, (fabs((double)w[i]) + fabs(rv1[i])));
  }

  /* accumulate the right-hand transformation */
  for (i = n - 1; i >= 0; i--) {
    if (i < n - 1) {
      if (g) {
        for (j = l; j < n; j++)
          v[j][i] = (double)(((double)a[i][j] / (double)a[i][l]) / g);
        /* double division to avoid underflow */
        for (j = l; j < n; j++) {
          for (s = 0.0, k = l; k < n; k++)
            s += ((double)a[i][k] * (double)v[k][j]);
          for (k = l; k < n; k++) v[k][j] += (double)(s * (double)v[k][i]);
        }
      }
      for (j = l; j < n; j++) v[i][j] = v[j][i] = 0.0;
    }
    v[i][i] = 1.0;
    g = rv1[i];
    l = i;
  }

  /* accumulate the left-hand transformation */
  for (i = n - 1; i >= 0; i--) {
    l = i + 1;
    g = (double)w[i];
    if (i < n - 1)
      for (j = l; j < n; j++) a[i][j] = 0.0;
    if (g) {
      g = 1.0 / g;
      if (i != n - 1) {
        for (j = l; j < n; j++) {
          for (s = 0.0, k = l; k < m; k++)
            s += ((double)a[k][i] * (double)a[k][j]);
          f = (s / (double)a[i][i]) * g;
          for (k = i; k < m; k++) a[k][j] += (double)(f * (double)a[k][i]);
        }
      }
      for (j = i; j < m; j++) a[j][i] = (double)((double)a[j][i] * g);
    } else {
      for (j = i; j < m; j++) a[j][i] = 0.0;
    }
    ++a[i][i];
  }

  /* diagonalize the bidiagonal form */
  for (k = n - 1; k >= 0; k--) {     /* loop over singular values */
    for (its = 0; its < 30; its++) { /* loop over allowed iterations */
      flag = 1;
      for (l = k; l >= 0; l--) { /* test for splitting */
        nm = l - 1;
        if (fabs(rv1[l]) + anorm == anorm) {
          flag = 0;
          break;
        }
        if (fabs((double)w[nm]) + anorm == anorm) break;
      }
      if (flag) {
        c = 0.0;
        s = 1.0;
        for (i = l; i <= k; i++) {
          f = s * rv1[i];
          if (fabs(f) + anorm != anorm) {
            g = (double)w[i];
            h = PYTHAG(f, g);
            w[i] = (double)h;
            h = 1.0 / h;
            c = g * h;
            s = (-f * h);
            for (j = 0; j < m; j++) {
              y = (double)a[j][nm];
              z = (double)a[j][i];
              a[j][nm] = (double)(y * c + z * s);
              a[j][i] = (double)(z * c - y * s);
            }
          }
        }
      }
      z = (double)w[k];
      if (l == k) {    /* convergence */
        if (z < 0.0) { /* make singular value nonnegative */
          w[k] = (double)(-z);
          for (j = 0; j < n; j++) v[j][k] = (-v[j][k]);
        }
        break;
      }
      if (its >= 30) {
        free((void *)rv1);
        strcpy(errormsg, "SVD: Error: No convergence after 30,000! iterations");
        return false;
      }

      /* shift from bottom 2 x 2 minor */
      x = (double)w[l];
      nm = k - 1;
      y = (double)w[nm];
      g = rv1[nm];
      h = rv1[k];
      f = ((y - z) * (y + z) + (g - h) * (g + h)) / (2.0 * h * y);
      g = PYTHAG(f, 1.0);
      f = ((x - z) * (x + z) + h * ((y / (f + SIGN(g, f))) - h)) / x;

      /* next QR transformation */
      c = s = 1.0;
      for (j = l; j <= nm; j++) {
        i = j + 1;
        g = rv1[i];
        y = (double)w[i];
        h = s * g;
        g = c * g;
        z = PYTHAG(f, h);
        rv1[j] = z;
        c = f / z;
        s = h / z;
        f = x * c + g * s;
        g = g * c - x * s;
        h = y * s;
        y = y * c;
        for (jj = 0; jj < n; jj++) {
          x = (double)v[jj][j];
          z = (double)v[jj][i];
          v[jj][j] = (double)(x * c + z * s);
          v[jj][i] = (double)(z * c - x * s);
        }
        z = PYTHAG(f, h);
        w[j] = (double)z;
        if (z) {
          z = 1.0 / z;
          c = f * z;
          s = h * z;
        }
        f = (c * g) + (s * y);
        x = (c * y) - (s * g);
        for (jj = 0; jj < m; jj++) {
          y = (double)a[jj][j];
          z = (double)a[jj][i];
          a[jj][j] = (double)(y * c + z * s);
          a[jj][i] = (double)(z * c - y * s);
        }
      }
      rv1[l] = 0.0;
      rv1[k] = f;
      w[k] = (double)x;
    }
  }
  free((void *)rv1);
  return true;
}

static float fPYTHAG(float a, float b) {
  float at = fabs(a), bt = fabs(b), ct, result;

  if (at > bt) {
    ct = bt / at;
    result = at * sqrt(1.0 + ct * ct);
  } else if (bt > 0.0) {
    ct = at / bt;
    result = bt * sqrt(1.0 + ct * ct);
  } else
    result = 0.0;
  return (result);
}

bool fsvd(float **a, int m, int n, float *w, float **v) {
  int flag, i, its, j, jj, k, l, nm;
  float c, f, h, s, x, y, z;
  float anorm = 0.0, g = 0.0, scale = 0.0;
  float *rv1;

  if (m < n) {
    strcpy(errormsg, "SVD: Error: #rows must be > #cols");
    return false;
  }

  rv1 = (float *)malloc((unsigned int)n * sizeof(float));

  /* Householder reduction to bidiagonal form */
  for (i = 0; i < n; i++) {
    /* left-hand reduction */
    l = i + 1;
    rv1[i] = scale * g;
    g = s = scale = 0.0;
    if (i < m) {
      for (k = i; k < m; k++) scale += fabs((float)a[k][i]);
      if (scale) {
        for (k = i; k < m; k++) {
          a[k][i] = (float)((float)a[k][i] / scale);
          s += ((float)a[k][i] * (float)a[k][i]);
        }
        f = (float)a[i][i];
        g = -SIGN(sqrt(s), f);
        h = f * g - s;
        a[i][i] = (float)(f - g);
        if (i != n - 1) {
          for (j = l; j < n; j++) {
            for (s = 0.0, k = i; k < m; k++)
              s += ((float)a[k][i] * (float)a[k][j]);
            f = s / h;
            for (k = i; k < m; k++) a[k][j] += (float)(f * (float)a[k][i]);
          }
        }
        for (k = i; k < m; k++) a[k][i] = (float)((float)a[k][i] * scale);
      }
    }
    w[i] = (float)(scale * g);

    /* right-hand reduction */
    g = s = scale = 0.0;
    if (i < m && i != n - 1) {
      for (k = l; k < n; k++) scale += fabs((float)a[i][k]);
      if (scale) {
        for (k = l; k < n; k++) {
          a[i][k] = (float)((float)a[i][k] / scale);
          s += ((float)a[i][k] * (float)a[i][k]);
        }
        f = (float)a[i][l];
        g = -SIGN(sqrt(s), f);
        h = f * g - s;
        a[i][l] = (float)(f - g);
        for (k = l; k < n; k++) rv1[k] = (float)a[i][k] / h;
        if (i != m - 1) {
          for (j = l; j < m; j++) {
            for (s = 0.0, k = l; k < n; k++)
              s += ((float)a[j][k] * (float)a[i][k]);
            for (k = l; k < n; k++) a[j][k] += (float)(s * rv1[k]);
          }
        }
        for (k = l; k < n; k++) a[i][k] = (float)((float)a[i][k] * scale);
      }
    }
    anorm = MAX(anorm, (fabs((float)w[i]) + fabs(rv1[i])));
  }

  /* accumulate the right-hand transformation */
  for (i = n - 1; i >= 0; i--) {
    if (i < n - 1) {
      if (g) {
        for (j = l; j < n; j++)
          v[j][i] = (float)(((float)a[i][j] / (float)a[i][l]) / g);
        /* float division to avoid underflow */
        for (j = l; j < n; j++) {
          for (s = 0.0, k = l; k < n; k++)
            s += ((float)a[i][k] * (float)v[k][j]);
          for (k = l; k < n; k++) v[k][j] += (float)(s * (float)v[k][i]);
        }
      }
      for (j = l; j < n; j++) v[i][j] = v[j][i] = 0.0;
    }
    v[i][i] = 1.0;
    g = rv1[i];
    l = i;
  }

  /* accumulate the left-hand transformation */
  for (i = n - 1; i >= 0; i--) {
    l = i + 1;
    g = (float)w[i];
    if (i < n - 1)
      for (j = l; j < n; j++) a[i][j] = 0.0;
    if (g) {
      g = 1.0 / g;
      if (i != n - 1) {
        for (j = l; j < n; j++) {
          for (s = 0.0, k = l; k < m; k++)
            s += ((float)a[k][i] * (float)a[k][j]);
          f = (s / (float)a[i][i]) * g;
          for (k = i; k < m; k++) a[k][j] += (float)(f * (float)a[k][i]);
        }
      }
      for (j = i; j < m; j++) a[j][i] = (float)((float)a[j][i] * g);
    } else {
      for (j = i; j < m; j++) a[j][i] = 0.0;
    }
    ++a[i][i];
  }

  /* diagonalize the bidiagonal form */
  for (k = n - 1; k >= 0; k--) {     /* loop over singular values */
    for (its = 0; its < 30; its++) { /* loop over allowed iterations */
      flag = 1;
      for (l = k; l >= 0; l--) { /* test for splitting */
        nm = l - 1;
        if (fabs(rv1[l]) + anorm == anorm) {
          flag = 0;
          break;
        }
        if (fabs((float)w[nm]) + anorm == anorm) break;
      }
      if (flag) {
        c = 0.0;
        s = 1.0;
        for (i = l; i <= k; i++) {
          f = s * rv1[i];
          if (fabs(f) + anorm != anorm) {
            g = (float)w[i];
            h = fPYTHAG(f, g);
            w[i] = (float)h;
            h = 1.0 / h;
            c = g * h;
            s = (-f * h);
            for (j = 0; j < m; j++) {
              y = (float)a[j][nm];
              z = (float)a[j][i];
              a[j][nm] = (float)(y * c + z * s);
              a[j][i] = (float)(z * c - y * s);
            }
          }
        }
      }
      z = (float)w[k];
      if (l == k) {    /* convergence */
        if (z < 0.0) { /* make singular value nonnegative */
          w[k] = (float)(-z);
          for (j = 0; j < n; j++) v[j][k] = (-v[j][k]);
        }
        break;
      }
      if (its >= 30) {
        free((void *)rv1);
        strcpy(errormsg, "SVD: Error: No convergence after 30,000! iterations");
        return false;
      }

      /* shift from bottom 2 x 2 minor */
      x = (float)w[l];
      nm = k - 1;
      y = (float)w[nm];
      g = rv1[nm];
      h = rv1[k];
      f = ((y - z) * (y + z) + (g - h) * (g + h)) / (2.0 * h * y);
      g = fPYTHAG(f, 1.0);
      f = ((x - z) * (x + z) + h * ((y / (f + SIGN(g, f))) - h)) / x;

      /* next QR transformation */
      c = s = 1.0;
      for (j = l; j <= nm; j++) {
        i = j + 1;
        g = rv1[i];
        y = (float)w[i];
        h = s * g;
        g = c * g;
        z = fPYTHAG(f, h);
        rv1[j] = z;
        c = f / z;
        s = h / z;
        f = x * c + g * s;
        g = g * c - x * s;
        h = y * s;
        y = y * c;
        for (jj = 0; jj < n; jj++) {
          x = (float)v[jj][j];
          z = (float)v[jj][i];
          v[jj][j] = (float)(x * c + z * s);
          v[jj][i] = (float)(z * c - x * s);
        }
        z = PYTHAG(f, h);
        w[j] = (float)z;
        if (z) {
          z = 1.0 / z;
          c = f * z;
          s = h * z;
        }
        f = (c * g) + (s * y);
        x = (c * y) - (s * g);
        for (jj = 0; jj < m; jj++) {
          y = (float)a[jj][j];
          z = (float)a[jj][i];
          a[jj][j] = (float)(y * c + z * s);
          a[jj][i] = (float)(z * c - y * s);
        }
      }
      rv1[l] = 0.0;
      rv1[k] = f;
      w[k] = (float)x;
    }
  }
  free((void *)rv1);
  return true;
}

void dpp_to_vvd(double **dpp, int m, int n,
                std::vector<std::vector<double> > &vvd) {
  vvd.resize(m);
  for (int i = 0; i < m; ++i) {
    vvd[i].resize(n);
    for (int j = 0; j < n; ++j) {
      vvd[i][j] = dpp[i][j];
    }
  }
}

void dp_to_vd(double *dp, int m, std::vector<double> &vd) {
  vd.resize(m);
  for (int i = 0; i < m; ++i) {
    vd[i] = dp[i];
  }
}

void cout_vvd(const char *tag, std::vector<std::vector<double> > &vvd) {
  cout << "SVD::cout_vvd " << tag << endl;

  int m = (int)vvd.size();

  if (m) {
    int n = (int)vvd[0].size();
    cout << "shape " << m << " " << n << endl;
  }

  for (int i = 0; i < m; ++i) {
    cout << i << ":";
    int n = (int)vvd[i].size();
    for (int j = 0; j < n; ++j) {
      cout << "\t" << vvd[i][j];
    }
    cout << "\n";
  }
  cout << "\n";
}

void cout_vvi(const char *tag, std::vector<std::vector<int> > &vvi) {
  cout << "SVD::cout_vvi " << tag << endl;

  int m = (int)vvi.size();
  if (m) {
    int n = (int)vvi[0].size();
    cout << "shape " << m << " " << n << endl;
  }

  for (int i = 0; i < m; ++i) {
    cout << i << ":";
    int n = (int)vvi[i].size();
    for (int j = 0; j < n; ++j) {
      cout << "\t" << vvi[i][j];
    }
    cout << "\n";
  }
  cout << "\n";
}

void cout_vd(const char *tag, std::vector<double> &vd) {
  cout << "SVD::cout_vd " << tag << endl;

  int m = (int)vd.size();
  for (int i = 0; i < m; ++i) {
    cout << "\t" << vd[i];
  }
  cout << "\n";
}

void cout_dpp(const char *tag, double **dpp, int m, int n) {
  cout << "SVD::cout_dpp " << tag << endl;

  for (int i = 0; i < m; ++i) {
    cout << i << ":";
    for (int j = 0; j < n; ++j) {
      cout << "\t" << dpp[i][j];
    }
    cout << "\n";
  }
  cout << "\n";
}

void cout_dp(const char *tag, double *dp, int m) {
  cout << "SVD::cout_dp " << tag << endl;

  for (int i = 0; i < m; ++i) {
    cout << "\t" << dp[i];
  }
  cout << "\n";
}

void dp_inv_nz(double *dp, int m) {  // invert nonzeros inplace
  for (int i = 0; i < m; ++i) {
    if (dp[i]) {
      dp[i] = 1 / dp[i];
    }
  }
}

void vvd_shape(std::vector<std::vector<double> > &vvd, int &m, int &n) {
  m = (int)vvd.size();
  if (m) {
    n = (int)vvd[0].size();
  } else {
    n = 0;
  }
}

std::vector<std::vector<double> > vvd_transpose(
    std::vector<std::vector<double> > &vvd) {
  int m;
  int n;

  vvd_shape(vvd, m, n);

  std::vector<std::vector<double> > result(n);
  for (int i = 0; i < n; ++i) {
    result[i].resize(m);
    for (int j = 0; j < m; ++j) {
      result[i][j] = vvd[j][i];
    }
  }
  return result;
}

std::vector<std::vector<double> > vvd_usmult(
    std::vector<std::vector<double> > &U, std::vector<double> &S) {
  std::vector<std::vector<double> > result;
  int m = (int)U.size();
  if (!m) {
    cout << "SVD::vvd_usmult U has no rows\n";
    return result;
  }
  int m1 = (int)U[0].size();
  if (!m1) {
    cout << "SVD::vvd_mult U has no columns\n";
    return result;
  }
  if (m1 != m) {
    cout << "SVD::vvd_mult U is not square\n";
    return result;
  }

  int n = (int)S.size();
  if (!n) {
    cout << "SVD::vvd_mult S is empty\n";
    return result;
  }
  result.resize(m);
  for (int i = 0; i < m; ++i) {
    result[i].resize(n);
  }

  for (int i = 0; i < m; ++i) {
    for (int j = 0; j < n; ++j) {
      result[i][j] = U[i][j] * S[j];
    }
  }
  return result;
}

std::vector<std::vector<double> > vvd_mult(
    std::vector<std::vector<double> > &A,
    std::vector<std::vector<double> > &B) {
  std::vector<std::vector<double> > result;

  int m = (int)A.size();
  if (!m) {
    cout << "SVD::vvd_mult A has no rows\n";
    return result;
  }
  int n = (int)A[0].size();
  if (!n) {
    cout << "SVD::vvd_mult A has no columns\n";
    return result;
  }
  int o = (int)B.size();
  if (!o) {
    cout << "SVD::vvd_mult B has no rows\n";
    return result;
  }
  int p = (int)B[0].size();
  if (!p) {
    cout << "SVD::vvd_mult B has no columns\n";
    return result;
  }
  if (n != o) {
    cout << "SVD::vvd_mult incompatible A & B, columns of A differs from rows "
            "of B\n";
    return result;
  }
  result.resize(m);
  for (int i = 0; i < m; ++i) {
    result[i].resize(p);
  }

  for (int i = 0; i < m; ++i) {
    for (int j = 0; j < p; ++j) {
      for (int k = 0; k < n; ++k) {
        result[i][j] += A[i][k] * B[k][j];
      }
    }
  }

  return result;
}

bool pinv(std::vector<std::vector<double> > A,
          std::vector<std::vector<double> > &Ainv) {
  // # define DEBUG_PINV

  // moore-penrose pseudo inverse Compute SVD A=UΣVT Ainv=VΣ+UT
#if defined(DEBUG_PINV)
  cout << "SVD::pinv" << endl;
  cout_vvd("pinv A", A);
#endif

  // convert to dpp

  int m;
  int n;

  vvd_shape(A, m, n);

  if (m < n) {
    std::vector<std::vector<double> > At;
    std::vector<std::vector<double> > Ainvt;

    At = vvd_transpose(A);

#if defined(DEBUG_PINV)
    cout_vvd("pinv At", At);
#endif

    if (pinv(At, Ainvt)) {
      Ainv = vvd_transpose(Ainvt);
#if defined(DEBUG_PINV)
      cout_vvd("inverted final result Ainv", Ainv);
#endif
      return true;
    }
  }

#if defined(DEBUG_PINV)
  cout << "SVD::pinv A m " << m << " n " << n << endl;
#endif

  double *app[m];

  for (int i = 0; i < m; ++i) {
    app[i] = &(A[i][0]);
  }

#if defined(DEBUG_PINV)
  cout_dpp("pinv app ", app, m, n);
#endif

  // compute svd

  std::vector<double> W(n);
  double *w = &(W[0]);

  std::vector<std::vector<double> > V(n);
  double *v[n];
  for (int i = 0; i < n; ++i) {
    V[i].resize(n);
    v[i] = &(V[i][0]);
  }

  if (!dsvd(app, m, n, w, v)) {
    return false;
  }

#if defined(DEBUG_PINV)
  cout_dpp("pinv U ", app, m, n);
  cout_dp("pinv D ", w, n);
  cout_dpp("pinv V ", v, n, n);
#endif

  dp_inv_nz(w, n);

#if defined(DEBUG_PINV)
  cout_dp("pinv Dinv ", w, n);
#endif

  // multiply back to produce Ainv
  // Ainv=VΣ+UT
  // which is really v * w * app

  Ainv.resize(n);
  for (int i = 0; i < n; ++i) {
    Ainv[i].resize(m);
  }

  // multiply v * w
  for (int i = 0; i < n; ++i) {
    for (int j = 0; j < n; ++j) {
      v[i][j] *= w[j];
    }
  }

#if defined(DEBUG_PINV)
  cout_dpp("pinv v now = v*w ", v, n, n);

  cout_vvd("pinv Ainv before mm ", Ainv);
#endif
  // compute Ainv = v*app
  for (int i = 0; i < n; ++i) {
    for (int j = 0; j < m; ++j) {
      for (int k = 0; k < n; ++k) {
        Ainv[i][j] += v[i][k] * app[j][k];
      }
    }
  }

#if defined(DEBUG_PINV)
  cout_vvd("pinv Ainv after mm ", Ainv);
#endif

  return true;
}

double vvd2_maxnorm(std::vector<std::vector<double> > &A,
                    std::vector<std::vector<double> > &B) {
  double result = 1e99;

  int m1;
  int n1;
  int m2;
  int n2;

  vvd_shape(A, m1, n1);
  vvd_shape(B, m2, n2);
  if (!m1) {
    cout << "SVD::vvd2_maxnorm A has no rows\n";
    return result;
  }
  if (!n1) {
    cout << "SVD::vvd2_maxnorm A has no columns\n";
    return result;
  }

  if (m1 != m2 || n1 != n2) {
    cout << "SVD::vvd2_maxnorm incompatible: A B shapes differ\n";
    return result;
  }

  result = 0e0;

  for (int i = 0; i < m1; ++i) {
    for (int j = 0; j < n1; ++j) {
      double absdiff = fabs(A[i][j] - B[i][j]);
      if (result < absdiff) {
        result = absdiff;
      }
    }
  }
  return result;
}
};  // namespace SVD
