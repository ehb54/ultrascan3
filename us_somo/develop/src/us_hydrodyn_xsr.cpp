#include "../include/us_hydrodyn_xsr.h"

#include "../include/us3_defines.h"
// Added by qt3to4:
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QTextStream>
// #include <Q3PopupMenu>
#include <QBoxLayout>
#include <QCloseEvent>
#include <QVBoxLayout>

#if QT_VERSION < 0x040000 && defined(Q_OS_WIN)
#if !defined(Q_OS_WIN)

namespace xsr {

#define MAX_CONTRAST_POINTS 16

/******************************************************************
 ***                       mathFunctions.c                      ***
 ******************************************************************
 *
 * Additional Info:
 *
 * Date: 10/5/08 last modified 6/10/08
 * Author: A.E. Whitten
 * Description:
 *
 * Standalone math functions used by XSR
 *
 * Version 1.0: File created
 *
 * Version 1.1: Interpolation function added
 */

double linearFit(double *X, double *Y, double *sigY, int n, double *mb) {
  /* Performs a weighted LSQ fit Y = mX + b: sigY[i] is the estimated stdDev. of
   * the data point Y[i] n is the number of data points */

  int i;

  /* i is a counter */

  double A[2] = {0.0, 0.0}, B[2][2] = {{0.0, 0.0}, {0.0, 0.0}},
         Bi[2][2] = {{0.0, 0.0}, {0.0, 0.0}}, w, delta, chi2 = 0.0;

  /* A[], B[][] and Bi[][] are vectors and matricies involved in the
   * least-squares. Esentially: A = B x mb, hence mb = Bi x A w is the weight,;
   * delta temporarily stores the difference between exp. and calc. values; chi2
   * is a measure of the total descrepency */

  for (i = 0; i < n; i++) {
    if (sigY[i] > 0.0) {
      w = 1.0 / sigY[i];

      A[0] += w * w * Y[i] * X[i];
      A[1] += w * w * Y[i];

      B[0][0] += w * w * X[i] * X[i];
      B[0][1] += w * w * X[i];
      B[1][0] += w * w * X[i];
      B[1][1] += w * w;
    }
  }

  /* Calculate the analytic inverse of the 2x2 matrix B */

  Bi[0][0] = 1.0 / (B[0][0] * B[1][1] - B[0][1] * B[1][0]) * B[1][1];
  Bi[0][1] = -1.0 / (B[0][0] * B[1][1] - B[0][1] * B[1][0]) * B[0][1];
  Bi[1][0] = -1.0 / (B[0][0] * B[1][1] - B[0][1] * B[1][0]) * B[1][0];
  Bi[1][1] = 1.0 / (B[0][0] * B[1][1] - B[0][1] * B[1][0]) * B[0][0];

  mb[0] = Bi[0][0] * A[0] + Bi[0][1] * A[1]; /* gradient (scale factor) */
  mb[1] =
      Bi[1][0] * A[0] + Bi[1][1] * A[1]; /* intercept (background correction) */

  for (i = 0; i < n; i++) { /* calculate chi2 */

    w = 1.0 / sigY[i];
    delta = w * (mb[0] * X[i] + mb[1] - Y[i]);
    chi2 += delta * delta;
  }

  return (chi2 / (double)(n - 2)); /* return chi2 */
}

double lagrangeInterpolation(
    double *xn, double *yn, double x,
    int n) { /* performs an n - 1 order interpolation */

  int j, k;
  double yTemp, y = 0.0;

  /* taken from
   * http://www.efunda.com/math/num_interpolation/num_interpolation.cfm */

  for (k = 0; k < n; k++) {
    yTemp = 1.0;

    for (j = 0; j < n; j++)
      if (j != k) yTemp *= (x - xn[j]) / (xn[k] - xn[j]);

    y += yn[k] * yTemp;
  }

  return (y);
}

/******************************************************************
 ***                        XSFunctions.c                       ***
 ******************************************************************
 *
 * Additional Info:
 *
 * Date: 10/5/08 last modified 6/10/08
 * Author: A.E. Whitten
 * Description:
 *
 * Various functions used by XSR
 *
 * Version 1.0: File created
 *
 * Version 1.1: Slit-smearing correction added to the program
 */

double q[NPMOD]; /* This is initialised in getResolutionFunction */

int getResolutionFunction(exp_t *data) { /* Calculate the resolution funtion */

  int i;
  int j;
  int k;
  //      int status = 0;

  /* i, j and k are loop counters; status is a flag to indicate that the
   * resolution function is sufficiently small to set summation limits */

  double Sq1, Sq2;

  /* Sq1 defines the half length of the top of the trapezium describing the silt
   * profile and Sq2 is the half length of the bottom of the trapezium */

  for (i = 0; i < NPMOD; i++)
    q[i] = QMIN + QMAX * (double)i /
                      (double)(NPMOD - 1); /* set the q values at which the
                                              model will be evaluated */

  for (k = 0; k < data->nContrastPoints;
       k++) { /* calculate the resolution function */

    Sq1 = (data[k].A - data[k].L) / 2.0;
    Sq2 = (data[k].A + data[k].L) / 2.0;

    if (strncmp(data[k].resType, "SLIT", 4) ==
        0) /* calculate the slit resolution function */
      for (i = 0; i < NPRES; i++) {
        data[k].qSlit[i] = Sq2 * (double)i / (double)(NPRES - 1);

        if (data[k].qSlit[i] <= Sq1)
          data[k].slit[i] =
              2.0 / (Sq1 + Sq2); /* ensures unit area under the trapezoid */

        else if (data[k].qSlit[i] > Sq1 && data[k].qSlit[i] <= Sq2)
          data[k].slit[i] =
              2.0 / (Sq1 + Sq2) * (Sq2 - data[k].qSlit[i]) / (Sq2 - Sq1);

        else
          data[k].slit[i] = 0.0;
      }

    else if (strncmp(data[k].resType, "NIST", 4) ==
             0) /* calculates the NIST resolution function */
      for (j = 0; j < data[k].nDataPoints; j++)
        for (i = 0; i < NPRES; i++) {
          data[k].qResFunction[j][i] =
              data[k].qbar[j] +
              ((double)i - (NPRES - 1.0) / 2.0) / 2.0 * data[k].sigq[j];
          data[k].resFunction[j][i] =
              data[k].sF[j] * GAUSSIAN(data[k].qResFunction[j][i],
                                       data[k].qbar[j], data[k].sigq[j]);
        }
  }

  return (1);
}

int getScaleBackground(calc_t *model,
                       exp_t *data) { /* Front-end to caclulate the scale and
                                         background corrections */

  int i;

  for (i = 0; i < model->nContrastPoints; i++)
    model[i].chi2 = linearFit(model[i].Iq, data[i].Iq, data[i].sigIq,
                              data->nDataPoints, model[i].mb);

  return (1);
}

int getContacts(
    grid_t *grid) { /* initilises the pointers for the closest contacts */

  int i, j, k;

  /* i, j and k are loop counters */

  double r;

  /* r is the distance between two grid elements */

  for (j = 0; j < grid->nGridPoints; j++) {
    grid[j].nContacts = 0;

    for (i = 0; i < grid->nGridPoints; i++)
      for (k = 0; k < grid->nSymmOps; k++) {
        r = sqrt(X2(grid[i].xy[k][0] - grid[j].xy[0][0]) +
                 X2(grid[i].xy[k][1] - grid[j].xy[0][1]));

        if (r > 0.0 && r < 1.42) {
          grid[j].contact[grid[j].nContacts] =
              &grid[i];        /* if the grid element is close, point to it */
          grid[j].nContacts++; /* increment the contact counter */
        }
      }
  }

  return (1);
}

int initialiseGrid(grid_t *grid, int symmOp) { /* initialises the grid */

  int i, j, nFiles, nResidues, xSize, ySize;

  /* i and j are loop counters; nFiles is the number of pdb files to be read in
   * as a starting point; nResidues is the number of residues in the pdb file;
   * xSize and ySize are the x and y dimensions of the grid used in calculating
   * the pdb file */

  double x, y;

  /* x and y temporarily store the x and y coordinates read in from the pdb file
   */

  char filename[30], buffer[999];

  /* filename is the name of the pdb filename to be read in; buffer is a storage
   * buffer */

  FILE *input;

  /* input is a pointer to the pdb file */

  grid->nSymmOps = 1; /* assume for starters that there is no symmetry */

  fgets(buffer, 999, stdin);
  sscanf(buffer, "%d", &nFiles);

  for (i = 0; i < grid->nGridPoints; i++) {
    /* fill the grid with random points unless pdb files are being read in */

    if (nFiles == 0)
      grid[i].value = (int)((grid->nComponents + 1) * (double)rand() /
                            (double)RAND_MAX); /* 0(solv.), 1(1st comp.), etc */
    else
      grid[i].value = 0;

    grid[i].xy[0][0] =
        (double)(i % grid->nX) +
        0.5; /* the centre of the atoms is in the centre of each grid point */
    grid[i].xy[0][1] = (double)(i / grid->nX) + 0.5;
  }

  if (nFiles != 0)
    for (i = 0; i < nFiles;
         i++) { /* read in pdb files if instructed to do so */

      fgets(buffer, 999, stdin);
      sscanf(buffer, "%s%d%d%d", filename, &nResidues, &xSize, &ySize);

      input = us_fopen(filename, "r");

      for (j = 0; j < nResidues; j++) {
        fgets(buffer, 999, input);

        if (buffer[21] == 'A') { /* read in the asymmetric grid only */

          sscanf(buffer + 28, "%lf%lf", &x, &y);
          grid[(int)((y / grid->gridDistance - 0.5) + (grid->nY - ySize) / 2) *
                   grid->nX +
               (int)((x / grid->gridDistance - 0.5) + (grid->nX - xSize))]
              .value = i + 1;
        }
      }

      fclose(input);
    }

  switch (symmOp) {
    case 1: /* symmOp = 1 means no symmetry */
      break;

    case 2: /* summOp = 2 means no symmetry */
      grid->nSymmOps = 2;

      for (i = 0; i < grid->nGridPoints; i++) {
        /* Symmetry related coordinates */

        grid[i].xy[1][0] = (double)(2 * grid->nX) - grid[i].xy[0][0];
        grid[i].xy[1][1] = (double)grid->nY - grid[i].xy[0][1];
      }

      break;

    default: /* add others as required */
      break;
  }

  getContacts(grid); /* get the contacts for each grid element */

  return (1);
}

int getIqcalc(
    pr_t **pr, exp_t *data,
    calc_t *model) { /* calculate the smeared model scattering profiles */

  int i, j, k, l, m;

  /* all loop counters */

  double f[NPR];
  // double h;
  // fix this: emre
  // double prTotal[model->nContrastPoints][NPR];
  double prTotal[MAX_CONTRAST_POINTS][NPR];

  /* f is for temporary storage of a portion of the integral required to
   * calculate Iq; h is the distance between the f values; prTotal is the pr
   * total pr function at each contrast point */

  // fix this: emre
  // double Iq[model->nContrastPoints][NPMOD], qtemp;
  double Iq[MAX_CONTRAST_POINTS][NPMOD], qtemp;
  /* Iq is the unsmeared model scattering profile */

  for (k = 0; k < model->nContrastPoints; k++) /* calculate prTotal */
    for (j = 0; j < pr[0]->nPrPoints; j++) {
      prTotal[k][j] = 0.0;

      for (l = 1; l <= model->nComponents; l++)
        for (m = 1; m <= model->nComponents; m++)
          prTotal[k][j] += pr[l][m].Drho[k] * pr[l][m].pr[j];
    }

  for (i = 0; i < NPMOD; i++) { /* perform numerical integration Ic(q)
                                   = 2.0*PI*PI*integral(p(r)j0(qr))dr */

    for (j = 0; j < pr[0]->nPrPoints;
         j++) /* pre-calculate this term to prevent any recalculations */
      f[j] = 2.0 * PI * PI * j0(q[i] * pr[0]->r[j]);

    for (k = 0; k < model->nContrastPoints; k++) {
      Iq[k][i] = TRAPEZOID(f[0] * prTotal[k][0], 0.0,
                           pr[0]->r[0]); /* the spacing for the first point is
                                            different, hence use trapezoid */

      for (j = 2; j < pr[0]->nPrPoints; j += 2)
        Iq[k][i] += SIMPSON(f[j] * prTotal[k][j], f[j - 1] * prTotal[k][j - 1],
                            f[j - 2] * prTotal[k][j - 2],
                            pr[0]->r[j] - pr[0]->r[j - 1]);
    }
  }

  for (i = 0; i < model->nDataPoints; i++)
    for (k = 0; k < model->nContrastPoints; k++) model[k].Iq[i] = 0.0;

  for (k = 0; k < model->nContrastPoints; k++) /* do the smearing correction */
    for (i = 0; i < model->nDataPoints; i++) {
      model[k].Iq[i] = 0.0;

      if (strncmp(data[k].resType, "SLIT", 4) ==
          0) /* do the slit smearing correction */
        for (j = 0; j < NPRES; j++) {
          qtemp = sqrt(X2(data[k].qSlit[j]) + X2(data[k].q[i]));
          f[j] = data[k].slit[j] * interpolatedIq(q, Iq[k], qtemp) / qtemp;
          if (j != 0 && j % 2 == 0)
            model[k].Iq[i] += SIMPSON(f[j], f[j - 1], f[j - 2],
                                      data[k].qSlit[j] - data[k].qSlit[j - 1]);
        }

      else if (strncmp(data[k].resType, "NIST", 4) ==
               0) /* do the NIST smearing correction */
        for (j = 0; j < NPRES; j++) {
          f[j] = data[k].resFunction[i][j] *
                 interpolatedIq(q, Iq[k], data[k].qResFunction[i][j]) /
                 data[k].qResFunction[i][j];
          if (j != 0 && j % 2 == 0)
            model[k].Iq[i] += SIMPSON(
                f[j], f[j - 1], f[j - 2],
                data[k].qResFunction[i][j] - data[k].qResFunction[i][j - 1]);
        }

      else
        model[k].Iq[i] = interpolatedIq(q, Iq[k], model[k].q[i]) /
                         model[k].q[i]; /* just interpolate without smearing */
    }

  return (1);
}

double interpolatedIq(double *q, double *Iq, double qint) {
  int i;

  for (i = 0; i < NPMOD - 1; i++) {
    if (qint > q[i] && qint < q[i + 1] && i <= 1)
      return (lagrangeInterpolation(&q[0], &Iq[0], qint, 4));
    else if (qint > q[i] && qint < q[i + 1] && i >= NPMOD - 4)
      return (lagrangeInterpolation(&q[NPMOD - 4], &Iq[NPMOD - 4], qint, 4));
    else if (qint > q[i] && qint < q[i + 1])
      return (lagrangeInterpolation(&q[i], &Iq[i], qint, 4));
  }
  // error: check this
  return 0e0;
}

double RgPenalty(calc_t *model) { /* returns the RgPenalty */

  int i;
  double penalty = 0.0;

  for (i = 0; i < model->nContrastPoints; i++)
    if (model[i].Rg > model[i].RgMax) penalty += model[i].Rg - model[i].RgMax;

  /* the penalty is the difference between the specified and actual values, if
   * the actual value exceedes the specified value */

  return (penalty);
}

double partialCompactness(grid_t *grid) { /* calculates the compactness due a
                                             point surrounded by other points */

  int i;
  double Ne = 0.0;

  /* Ne is the number of surrounding points of the same type as the given point
   */

  if (grid->value != 0) {
    for (i = 0; i < grid->nContacts; i++)
      if (grid->value == grid->contact[i]->value) Ne += 1.0;

    return (
        exp(-0.5 * Ne) -
        exp(-0.5 *
            8.0)); /* 8.0 is the maximum number of contacts for a square grid */

  }

  else
    return (0.0); /* don't do anything if the point is solvent */
}

double adjustCompactness(int newValue,
                         grid_t *grid) { /* adjusts the compactness, without
                                            having to recalculate it */

  int i, oldValue;

  /* i is a loop counter; oldValue temporarily stores the previous grid value at
   * that point */

  double compactness = 0.0;

  /* compactness is the adjustment to the compactness of the grid */

  compactness -=
      partialCompactness(grid); /* subtract the contribution the gridpoint makes
                                   to the total compactness */

  for (i = 0; i < grid->nContacts; i++)
    compactness -= partialCompactness(
        grid->contact[i]); /* also subtract the contribution of each of the
                              surrounding points */

  oldValue =
      grid->value; /* change the value to the new value and store the old one */
  grid->value = newValue;

  compactness +=
      partialCompactness(grid); /* add the contribution the new gridpoint makes
                                   to the total compactness */

  for (i = 0; i < grid->nContacts; i++)
    compactness += partialCompactness(
        grid->contact[i]); /* also add the contribution of each of the
                              surrounding points */

  grid->value = oldValue; /* reset the grid value to the old value */

  return (compactness);
}

int getArea(grid_t *grid) { /* calculates the area of each component, and the
                               total area in the grid */

  int i;

  /* i is a loop counter */

  for (i = 0; i <= grid->nComponents; i++) /* Initialise the area */
    grid->area[i] = 0;

  for (i = 0; i < grid->nGridPoints;
       i++) /* calculate the area of each component */
    grid->area[grid[i].value]++;

  grid->totalArea = (int)0.0;

  for (i = 1; i <= grid->nComponents; i++) grid->totalArea += grid->area[i];

  return (1);
}

int adjustArea(int oldValue, int newValue,
               grid_t *grid) { /* adjusts the total area of each component,
                                  without recalculating the whole thing */

  grid->area[oldValue]--;
  grid->area[newValue]++;

  if (oldValue == 0 && newValue != 0)
    grid->totalArea++; /* the total area does not incorporate solvent volume */
  else if (oldValue != 0 && newValue == 0)
    grid->totalArea--;

  return (1);
}

double compactness(
    grid_t *grid) { /* returns the total compactness of the grid */

  int i;

  /* i is a loop counter */

  double compactness = 0.0;

  /* compactness is a measure of the compactness of the grid */

  for (i = 0; i < grid->nGridPoints; i++)
    compactness += partialCompactness(&grid[i]);

  return (compactness);
}

int partialLooseness(grid_t *grid, int value) {
  int i;
  double tempVal = 0.0;

  if (grid->value == value && grid->searched == 0) {
    grid->searched = 1;

    for (i = 0; i < grid->nContacts; i++)
      tempVal += partialLooseness(grid->contact[i], value);

    return ((int)(tempVal + 1));

  }

  else
    return (0);
}

double looseness(grid_t *grid) {
  int i, tempLooseness, bestLooseness[5] = {0, 0, 0, 0, 0};
  double looseness = 0.0;

  for (i = 0; i < grid->nGridPoints; i++) grid[i].searched = 0;

  for (i = 0; i < grid->nGridPoints; i++)
    if (grid[i].searched == 0 && grid[i].value != 0) {
      tempLooseness = partialLooseness(&grid[i], grid[i].value);

      if (tempLooseness > bestLooseness[grid[i].value])
        bestLooseness[grid[i].value] = tempLooseness;
    }

  //	for(i = 1; i < grid->nComponents + 1; i ++)
  //		fprintf(stdout,"grid->area[%d] = %d, bestLooseness[%d] = %d,
  //looseness = %lf\n",i, grid->area[i], i, bestLooseness[i],
  //log((double)grid->area[i]/(double)bestLooseness[i]));
  for (i = 1; i < grid->nComponents + 1; i++)
    if (grid->area[i] != 0)
      looseness += log((double)grid->area[i] / (double)bestLooseness[i]);

  return (looseness);
}

int updatePrGaussian(
    double *pr, double mean, int bin,
    double nSymmOps) { /* This function distributes the pr normally, with a
                          stdDev of 1/3 of the gridDistance */

  double cdf1, cdf2;

  /* cdf1 and cdf2 are cumulative distribution funtions, and are defined
   * according to: http://en.wikipedia.org/wiki/Normal_distribution */

  /* Because the grid elements have a defined size, the distance between two
   * grid elements is not a single value, but is instead a distribution. Let us
   * assume that the grid elements are circular (radius = R), and the distance
   * between the centres is d, then the distribution is zero at d - R, is a
   * maximum at d, and zero at d + R. We can approximate this distribution by
   * the normal distribution function: f(r) = 1/(0.333R x sqrt(2PI)) x exp( -(r
   * - d)^2 / (2 x 0.333R). This function is then integrated between the limits
   * of the pr bins */

  /* By definition cdf0 is approximately 0.0, and cdf3 is approximately 1.0, so
   * these are not evalutated */

  cdf1 =
      0.5 *
      (1.0 + erf(2.12132 * ((double)bin - mean))); /* 2.12132 = 3.0/sqrt(2.0) */
  cdf2 = 0.5 * (1.0 + erf(2.12132 * ((double)(bin + 1) - mean)));

  pr[bin - 1] += nSymmOps * cdf1;
  pr[bin] += nSymmOps * (cdf2 - cdf1); /* Distribute the pr appropriately */
  pr[bin + 1] += nSymmOps * (1.0 - cdf2);

  return (1);
}

int getPartialPr(pr_t **pr, grid_t *grid,
                 grid_t *point) { /* calculates pr for a "point" surround by the
                                     rest of the grid, neglecting contributions
                                     from it symmetry related self */

  int i, j, status;

  /* i and j are loop counters; status is a flag that indicates whether we have
   * attempted to evaluate the distance between point and point (i.e. distance =
   * 0.0) */

  double r;

  /* r is the distance between two grid elements */

  for (i = 0; i < grid->nGridPoints;
       i++) { /*compute the p(r) for position "point"*/

    status = 0;

    if (grid[i].value != 0) /* if the value is zero, then the pr remains
                               unchanged, so don't bother calculating it */
      for (j = 0; j < grid->nSymmOps && status == 0; j++) {
        r = sqrt(X2(grid[i].xy[j][0] - point->xy[0][0]) +
                 X2(grid[i].xy[j][1] - point->xy[0][1]));

        if (r == 0.0)
          status = 1; /* we want to treat p(r) due to symm related elements of
                         "point" seperately, so disregard all these */

        else
          updatePrGaussian(pr[point->value][grid[i].value].pr, r, (int)r,
                           (double)grid->nSymmOps);
        /* distribute the contribution normally, with a stdDev of 1/3 of the
         * gridDistance */
      }
  }

  return (1);
}

int getPr(pr_t **pr, grid_t *grid) {
  int i;
  // int j;
  int k;

  /* All loop counters */

  double r;

  /* r is the distance between a grid element and those related to it by
   * symmetry */

  zeroPr(pr);

  for (i = 0; i < grid->nGridPoints; i++) {
    getPartialPr(
        pr, grid,
        &grid[i]); /* get the pr due to a grid element surrounded by all others
                      except those related to it by symmetry */

    for (k = 1; k < grid->nSymmOps;
         k++) { /* treat the component neglected in getPartialPr (start at k =
                   0, because k = 0 is the asymmetric grid element */

      r = sqrt(X2(grid[i].xy[k][0] - grid[i].xy[0][0]) +
               X2(grid[i].xy[k][1] - grid[i].xy[0][1]));

      updatePrGaussian(pr[grid[i].value][grid[i].value].pr, r, (int)r,
                       (double)grid->nSymmOps);
      /* distribute the contribution normally, with a stdDev of 1/3 of the
       * gridDistance */
    }
  }

  return (1);
}

int zeroPr(pr_t **pr) { /* zero's the pr arrays in 2D  pr data structures */

  int i, j, k;

  /* All loop counters */

  for (k = 0; k < pr[0]->nComponents; k++)
    for (j = 0; j < pr[0]->nComponents; j++)
      for (i = 0; i < pr[0]->nPrPoints; i++) pr[k][j].pr[i] = 0.0;

  return (1);
}

int getMoments(pr_t **pr,
               calc_t *model) { /* Calculates zeroth (I0) and second (Rg)
                                   moments of the pr functions */

  int i;
  // int j;
  int k;
  int l;
  int m;

  /* All loop counters */

  // fix this: emre
  // double prTotal[model->nContrastPoints][NPR];
  double prTotal[MAX_CONTRAST_POINTS][NPR];

  /* prTotal is the pr at each contrast point */

  for (k = 0; k < model->nContrastPoints; k++) /* calculate the pr's first */
    for (i = 0; i < pr[0]->nPrPoints; i++) {
      prTotal[k][i] = 0.0;

      for (l = 1; l <= model->nComponents; l++)
        for (m = 1; m <= model->nComponents; m++)
          prTotal[k][i] += pr[l][m].Drho[k] * pr[l][m].pr[i];
    }

  for (k = 0; k < model->nContrastPoints; k++) { /* Do integration */

    model[k].I0 =
        2.0 * PI *
        TRAPEZOID(prTotal[k][0], 0.0,
                  pr[0]->r[0]); /* Spacing for the first point is different,
                                   hence use trapezoid rule */
    model[k].Rg = TRAPEZOID(prTotal[k][0] * X2(pr[0]->r[0]), 0.0, pr[0]->r[0]);

    for (i = 2; i < pr[0]->nPrPoints;
         i++) { /* The remainder of the integration can be performed with
                   simpson's rule */

      model[k].I0 += 2.0 * PI *
                     SIMPSON(prTotal[k][i], prTotal[k][i - 1],
                             prTotal[k][i - 2], pr[0]->r[i] - pr[0]->r[i - 1]);
      model[k].Rg += SIMPSON(prTotal[k][i] * X2(pr[0]->r[i]),
                             prTotal[k][i - 1] * X2(pr[0]->r[i - 1]),
                             prTotal[k][i - 2] * X2(pr[0]->r[i - 2]),
                             pr[0]->r[i] - pr[0]->r[i - 1]);
    }

    /* We have actually evaluated the twice the expectation value of r^2
     * (2<r^2>) above, Rg is actually sqrt(<r^2>/<r^0>), hence we must normalise
     * <r^2> and take the sqrt */

    if (model[k].Rg < 0.0)
      model[k].Rg =
          -sqrt(-PI * model[k].Rg /
                model[k].I0); /* if Rg^2 is negative, change the sign, take the
                                 sqrt, and change the sign of the result */
    else
      model[k].Rg = sqrt(PI * model[k].Rg / model[k].I0);
  }

  return (1);
}

double totalResidual(calc_t *model,
                     grid_t *grid) { /* return the total residual */

  int i;

  /* i is a loop counter */

  double residual = 0.0;

  /* residual is the total residual that is to be returned */

  for (i = 0; i < model->nContrastPoints;
       i++) /* component of the residual based on the agreement between
               calculated and experimental profiles */
    residual += model[i].chi2;

  /*            Chi**2 contribution                             Compactness
   * contribution                                   Rg penalty            */

  residual =
      residual / model->nContrastPoints +
      grid->compactnessWeight * grid->compactness / (double)grid->totalArea +
      RGPENALTYWEIGHT * grid->RgPenalty;
  residual += grid->loosenessWeight * grid->looseness;

  return (residual);
}

/*           **** Printing functions ****           */

int printIqModel(calc_t *model) { /* prints the Iq data to file */

  int i, j;

  /* i and j are loop counters */

  FILE *output;

  /* a pointer to the file where data is being written */

  for (i = 0; i < model->nContrastPoints;
       i++) { /*print best I(q) profile at each cycle*/

    output = us_fopen(model[i].outFilename, "w");

    fprintf(output,
            "Smeared Model I(q) data: Chi**2 = %lf, Unsmeared and unscaled "
            "moments: Ic(0) = %lf, Rgc = %lf\n",
            model[i].chi2, model[i].I0, model[i].Rg);
    fprintf(output, "   q     I(q)\n"); /* header informations */

    for (j = 0; j < model->nDataPoints; j++)
      fprintf(output, "%lf %lf\n", model[i].q[j],
              model[i].mb[0] * model[i].Iq[j] +
                  model[i].mb[1]); /* scattering data */

    fclose(output);
  }

  return (1);
}

int printPrModel(pr_t **pr) { /* print pr for each contrast point to file */

  int i, j, k, l;

  /* all loop counters */

  FILE *output;

  /* output is a pointer to output file */

  double totalPr;

  /* temporarily stores the total pr at each contrast point */

  output = us_fopen("model.pr", "w");

  for (j = 0; j < pr[0]->nContrastPoints + 1; j++) /* print out p(r) = 0.0 */
    fprintf(output, "%13.5E", 0.0);

  fprintf(output, "\n");

  for (i = 0; i < pr[0]->nPrPoints; i++) {
    fprintf(output, "%13.5E", pr[0]->r[i]);

    for (j = 0; j < pr[0]->nContrastPoints; j++) {
      totalPr = 0.0;

      for (k = 1; k < pr[0]->nComponents; k++)
        for (l = 1; l < pr[0]->nComponents; l++)
          totalPr += pr[k][l].Drho[j] * pr[k][l].pr[i];

      fprintf(output, "%13.5E", totalPr);
    }

    fprintf(output, "\n");
  }

  fclose(output);

  return (1);
}

int printPDB(grid_t *grid,
             const char *filename) { /* print the grid to file in a format
                                        readable by pdb veiwers */

  int i, j;

  /* both loop counters */

  FILE *gridPDB;

  /* gridPDB is a pointer to the file where the pdb format is written */

  gridPDB = us_fopen(filename, "w");

  for (i = 0; i < grid->nGridPoints;
       i++) /*output best grid at the end of each cycle for viewing*/
    for (j = 0; j < grid->nSymmOps; j++) {
      /* atom type depends on the component */

      if (grid[i].value == 1)
        fprintf(gridPDB,
                "ATOM%7d  C   ASP %c%4d    %8.3lf%8.3lf%8.3lf   1.00  1.00\n",
                i, j + 65, i, grid[i].xy[j][0] * grid->gridDistance,
                grid[i].xy[j][1] * grid->gridDistance, 0.0);
      else if (grid[i].value == 2)
        fprintf(gridPDB,
                "ATOM%7d  S   CYS %c%4d    %8.3lf%8.3lf%8.3lf   1.00  1.00\n",
                i, j + 65, i, grid[i].xy[j][0] * grid->gridDistance,
                grid[i].xy[j][1] * grid->gridDistance, 0.0);
      else if (grid[i].value == 3)
        fprintf(gridPDB,
                "ATOM%7d  N   ASP %c%4d    %8.3lf%8.3lf%8.3lf   1.00  1.00\n",
                i, j + 65, i, grid[i].xy[j][0] * grid->gridDistance,
                grid[i].xy[j][1] * grid->gridDistance, 0.0);
      else if (grid[i].value == 4)
        fprintf(gridPDB,
                "ATOM%7d  O   ASP %c%4d    %8.3lf%8.3lf%8.3lf   1.00  1.00\n",
                i, j + 65, i, grid[i].xy[j][0] * grid->gridDistance,
                grid[i].xy[j][1] * grid->gridDistance, 0.0);
    }

  fclose(gridPDB);

  return (1);
}

int printInformation(calc_t *model, grid_t *grid, int cycle) {
  int i;

  /* i is a loop counter */

  double partialResidual = 0.0;

  /* partial residual is a temporary varible used to store different components
   * of the total residual */

  fprintf(stdout, "\nCycle%4d: Information for current best model\n", cycle);
  fprintf(stdout, "_______________________________________\n\n");

  fprintf(
      stdout,
      "Total Residual = (Residuals for each contrast point)/nContrastPoints + "
      "CompactnessPenalty + LoosenessPenalty + RgPenalty\n");
  fprintf(stdout, "               = (");

  for (i = 0; i < model->nContrastPoints; i++) {
    if (i != 0) fprintf(stdout, " + ");
    fprintf(stdout, "%.3lf", model[i].chi2);
    partialResidual += model[i].chi2;
  }

  /* print a breakdown of the total residual */

  fprintf(stdout,
          ")/%d + (%.0lf x %.3lf)/%d + %.0lf x %.3lf + %.0lf x %.3lf\n         "
          "      = %.3lf + %.3lf + %.3lf + %.3lf\n               = %.3lf\n",
          model->nContrastPoints, grid->compactnessWeight, grid->compactness,
          grid->totalArea, grid->loosenessWeight, grid->looseness,
          RGPENALTYWEIGHT, grid->RgPenalty,
          partialResidual / model->nContrastPoints,
          grid->compactnessWeight * grid->compactness / (double)grid->totalArea,
          grid->loosenessWeight * grid->looseness,
          RGPENALTYWEIGHT * grid->RgPenalty, totalResidual(model, grid));

  fprintf(stdout, "\nArea of the components:\n");

  for (i = 1; i <= grid->nComponents;
       i++) /* print the area each component fills */
    fprintf(stdout, "%d: %lf Ang**2\n", i,
            X2(grid->gridDistance) * (double)grid->area[i]);

  fprintf(stdout, "\nMoments of the best model:\n");

  for (i = 0; i < model->nContrastPoints;
       i++) /* print the moments to file - and correction factors */
    fprintf(stdout,
            "Contrast point %d: I(0) =%12.5E, Rg =%6.2lf (scale factor "
            "correction =%10.3E, background correction =%10.3E)\n",
            i + 1, model[i].I0, model[i].Rg, model[i].mb[0], model[i].mb[1]);

  return (1);
}

/******************************************************************
 ***                            main.c                          ***
 ******************************************************************
 *
 * Additional Info:
 *
 * Date: 10/5/08 last modified 6/10/08
 * Author: A.E. Whitten
 * Description:
 *
 * main routine for XSR: Program that fits a dummy atom model
 * (that represents the cross-section of a rod) to small-angle
 * scattering data - including neutron contrast variation.
 *
 * Version 1.0: File created
 *
 * Version 1.1: Slit-smearing correction added to the program
 */

#define MAXRECONFACTOR 10
/* The total number of reconfigurations at each temperature is given by
 * MAXRECONFACTOR*(#Gridpoints in assymetric unit) */

int xsr_main(void) {
  int i;
  int j;
  int k;
  // int l;
  int symmOp, nX, nY, nComponents, nContrastPoints, nDataPoints, nPrPoints,
      *bestGrid, oldValue;
  int nReconfigurations, nSuccess, nRandomChanges, accepted, randomPoint,
      randomValue, cycle;

  /* i, j, k and l are counters; symmOp represents the symmetry of the filament;
   * nX and nY are the number of grid points in the asymmetric unit in the X and
   * Y directions; nComponents is the number of components with differing
   * contrast; nContrastPoints is the number of contrast points measured;
   * nDataPoints is the number of qValues measured; nPrPoints is the maximum
   * number of points in the pr profile to be processed; *bestgrid is an integer
   * array for storing the current best grid values; oldValue temporarily stores
   * the value of the point being altered in the annealing process;
   * nReconfigurations is a counter representing the number of reconfigurations
   * attempted in the annealing process; nSuccess is the number of
   * reconfigurations that cause a decrease in the residual; nRandomChanges are
   * the number of accepted reconfigurations that cause an increase in the
   * residual; is a flag that indicates whether a given reconfiguation is
   * accepted (>0) or rejected (=0); randomPoint is the gridpoint randomly
   * chosen for reconfiguration; randomValue is the randomly chosen value that
   * "randomPoint" is to be changed to; cycle is an incremental counter that
   * keeps track of the number of cycles performed.
   */

  double compactnessWeight, loosenessWeight, r, gridDistance, Drho[5],
      bestResidual, currentResidual, tempResidual, temperature;

  /* compactnessWeight is the weight of the compactness term in the total
   * residual; r is used to temporarily store the distance between a dummy atom
   * and its symmetry related partner; gridDistance is the distance between any
   * two grid elements; Drho[5] temporarily stores contrast values;
   * bestResidual, currentResidual and tempResidual store the best residual
   * reached, the residual of the current model, and the residual of the
   * reconfigured model; temperature is the current annealing temperature.
   */

  char buffer[999];

  /* buffer temporarily stores information read in from input files */

  FILE *input;

  /* input is a pointer to the scattering data files */

  grid_t *grid;

  /* grid contains spatial and related information about the model */

  calc_t *model;

  /* model contains the calculated scattering profiles and related properties of
   * the model */

  exp_t *data;

  /* data contains information read from the scattering data files */

  pr_t **pr, **tempPr;

  /* pr contains information regarding the pr of the current model; tempPr
   * contains information regarding the changes to the pr casued by the
   * reconfiguration */

  srand((unsigned int)time(NULL)); /* randomise the seed */

  fgets(buffer, 999, stdin);
  sscanf(buffer, "%d",
         &symmOp); /* the symmetry operation is read in from stdin */
  fgets(buffer, 999, stdin);
  sscanf(buffer, "%d%d", &nX,
         &nY); /* the number of gridpoints in x and y is read in from stdin */
  fgets(buffer, 999, stdin);
  sscanf(buffer, "%lf", &gridDistance); /* the distance between each gridpoint
                                           is read in from stdin */
  fgets(buffer, 999, stdin);
  sscanf(buffer, "%d",
         &nComponents); /* the number of components is read in from stdin */
  fgets(buffer, 999, stdin);
  sscanf(buffer, "%lf%lf%lf", &compactnessWeight, &loosenessWeight,
         &temperature); /* the number of components is read in from stdin */
  fgets(buffer, 999, stdin);
  sscanf(buffer, "%d", &nContrastPoints); /* the number of contrast points is
                                             read in from stdin */

  GRIDT(grid, nY * nX, "grid", "main",
        i); /* allocate an grid_t array: (GRIDT) is a macro */

  grid->nX = nX; /* store the local variables into the data structures */
  grid->nY = nY;
  grid->gridDistance = gridDistance;
  grid->nComponents = nComponents;
  grid->compactnessWeight = compactnessWeight;
  grid->loosenessWeight = loosenessWeight;

  INT(bestGrid, nY * nX, "bestGrid", "main",
      i); /* allocate an integer array: (INT) is a macro */

  CALCT(model, nContrastPoints, "model", "main",
        i); /* allocate an calc_t array: (CALCT) is a macro */
  model->nComponents = nComponents;

  EXPT(data, nY * nX, "grid", "main",
       i); /* allocate an exp_t array: (EXPT) is a macro */
  data->nContrastPoints = nContrastPoints;

  PRT2D(pr, nComponents + 1, nComponents + 1, "pr", "main", i,
        j); /* allocate a 2D pr_t array: (PRT2D) is a macro */
  PRT2D(tempPr, nComponents + 1, nComponents + 1, "tempPr", "main", i, j);
  pr[0]->nContrastPoints = nContrastPoints;

  for (i = 0; i < NPR;
       i++) /* initialise the distance array, r, for the pr function */
    for (j = 0; j < pr[0]->nComponents; j++)
      for (k = 0; k < pr[0]->nComponents; k++)
        pr[k][j].r[i] = (double)i * gridDistance +
                        0.5 * gridDistance; /* See Pr-ChoiceOfR.xls for
                                               justification of this scheme */

  for (i = 0; i < nContrastPoints;
       i++) { /* Read information from each data file and store it */

    fgets(buffer, 999, stdin);
    sscanf(buffer, "%s%d%lf%lf%lf%lf%lf", data[i].filename, &nDataPoints,
           &model[i].RgMax, &Drho[1], &Drho[2], &Drho[3], &Drho[4]);

    fgets(buffer, 999, stdin);
    sscanf(buffer, "%s%lf%lf", &data[i].resType, &data[i].A,
           &data[i].L); /* doesn't matter if these numbers are not present if
                           resType = NONE or NIST */

    if (strncmp(data[i].resType, "NIST", 4) == 0) {
      DOUBLE(
          data[i].sigq, NPEXP, "data[i].sigq", "main",
          j); /* allocate memory for the NIST resolution function parameters */
      DOUBLE(data[i].qbar, NPEXP, "data[i].qbar", "main", j);
      DOUBLE(data[i].sF, NPEXP, "data[i].sF", "main", j);

      DOUBLE2D(data[i].resFunction, NPEXP, NPRES, "data[i].resFunction", "main",
               j, k);
      DOUBLE2D(data[i].qResFunction, NPEXP, NPRES, "data[i].qResFunction",
               "main", j, k);

    }

    else if (strncmp(data[i].resType, "SLIT", 4) == 0) {
      DOUBLE(data[i].slit, NPEXP, "data[i].slit", "main", j);
      DOUBLE(data[i].qSlit, NPEXP, "data[i].qSlit", "main", j);
    }

    Drho[0] = 0.0; /* Drho[0] is the contrast of the solvent and Drho[1] is the
                      contrast of component 1 etc ... */

    for (j = 0; j < pr[0]->nComponents; j++)
      for (k = 0; k < pr[0]->nComponents; k++)
        pr[j][k].Drho[i] = Drho[j] * Drho[k];

    /* the notation is a little inconsistent here, but: pr[j][k].Drho[i] in the
     * product of the contrast of component j and k. The reason for this is not
     * immediately apparent but is related to the fact that the pr is broken up
     * into contributions between diffent components (i.e. pr[i][j].pr is the pr
     * for distances between component i and j, and pr[i][i] is the pr for
     * distances between DAMs in component i. Breaking the pr down in such a was
     * allows means that pr's for all contrast points can be calculated from
     * this basic set of pr's and means that a pr need not be evaluated for each
     * contrast point, speeding things up considerably. This is akin to
     * composite scattering functions */

    data[i].nDataPoints = nDataPoints;
    model[i].nDataPoints = nDataPoints;

    strcpy(model[i].outFilename, data[i].filename);
    strcat(model[i].outFilename,
           "_fit.dat"); /* the file that contains the calculated intensity data
                           has _fit.dat appended to the actual filename */

    input = us_fopen(data[i].filename, "r"); /* open the data file */

    for (j = 0; j < 5; j++)
      fgets(buffer, 999, input); /* scan over header information */

    for (j = 0; j < data[i].nDataPoints; j++) {
      fgets(buffer, 999, input);
      if (strncmp(data[i].resType, "NIST", 4) == 0)
        sscanf(buffer, "%lf%lf%lf%lf%lf%lf", &data[i].q[j], &data[i].Iq[j],
               &data[i].sigIq[j], &data[i].sigq[j], &data[i].qbar[j],
               &data[i].sF[j]);
      else
        sscanf(buffer, "%lf%lf%lf", &data[i].q[j], &data[i].Iq[j],
               &data[i].sigIq[j]);

      model[i].q[j] = data[i].q[j];

      //			data[i].sigIq[j] = (1.0
      //+ 2.5*GAUSSIAN(data[i].q[j], 0.12, 0.02)/GAUSSIAN(0.12, 0.12,
      //0.02))*data[i].sigIq[j];   /* Down weight points in this range */
    }

    fclose(input);
  }

  getResolutionFunction(data); /* initialise the resolution function (only needs
                                  to be done once) */
  initialiseGrid(
      grid,
      symmOp); /* initialise the grid (randomise) or read a previous run in */

  nPrPoints = 0;

  for (i = 0; i < grid->nGridPoints; i++)
    for (j = 0; j < grid->nGridPoints; j++)
      for (k = 0; k < grid->nSymmOps; k++) {
        r = sqrt(X2(grid[i].xy[0][0] - grid[j].xy[k][0]) +
                 X2(grid[i].xy[0][1] - grid[j].xy[k][1]));
        if ((int)r > nPrPoints)
          nPrPoints =
              (int)r + 2; /* determine the largest distance on the grid */
      }

  /* Note that during all calculations r has no units, to convert to a real
   * distance r must be multiplied by gridDistance */

  pr[0]->nPrPoints = nPrPoints; /*set the maximum number of PrPoints */
  tempPr[0]->nPrPoints = nPrPoints;

  printPDB(grid, "model.pdb"); /* print the initial model in pdb format */

  cycle = 0;
  bestResidual = 9999.9;
  getArea(grid);
  grid->looseness = looseness(grid);
  grid->compactness = compactness(grid);
  getPr(pr, grid);
  getMoments(pr, model);
  grid->RgPenalty = RgPenalty(model);
  getIqcalc(pr, data, model);
  getScaleBackground(model, data); /* this function also determines chi2 values
                                      for each contrast point */
  currentResidual = totalResidual(model, grid);

  do {
    if (currentResidual > bestResidual) {
      /* If the  reconfigured model is not as good as the old model, reset
       * everything */

      for (i = 0; i < grid->nGridPoints; i++) grid[i].value = bestGrid[i];

      getArea(grid);
      grid->looseness = looseness(grid);
      grid->compactness = compactness(grid);
      getPr(pr, grid);
      getMoments(pr, model);
      grid->RgPenalty = RgPenalty(model);
      getIqcalc(pr, data, model);
      getScaleBackground(model, data);
      currentResidual = totalResidual(model, grid);

    }

    else {
      /* If the reconfigured model is better than the old model, update the old
       * model */
      for (i = 0; i < grid->nGridPoints; i++) bestGrid[i] = grid[i].value;

      getMoments(pr, model);
      grid->RgPenalty = RgPenalty(model);
      getIqcalc(pr, data, model); /* If the last random change is not accepted,
                                     old chi2 values are stored */
      getScaleBackground(model, data);
      bestResidual = currentResidual;
    }

    printInformation(model, grid,
                     cycle); /* Print general info, pdb file, intensity data and
                                the pr's to file */
    printPDB(grid, "model.pdb");
    printIqModel(model);
    printPrModel(pr);

    /* ----------------- Print information to the screen so that the user can
     * easily monitor progress ----------------- */

    fprintf(stderr, "Cycle %d: Total Residual = (", cycle);

    for (i = 0; i < model->nContrastPoints; i++) {
      if (i != 0) fprintf(stderr, " + ");
      fprintf(stderr, "%.2lf", model[i].chi2);
    }

    fprintf(
        stderr, ")/%d + %.2lf + %.2lf + %.2lf = %.2lf\n",
        model->nContrastPoints,
        grid->compactnessWeight * grid->compactness / (double)grid->totalArea,
        grid->loosenessWeight * grid->looseness,
        RGPENALTYWEIGHT * grid->RgPenalty, totalResidual(model, grid));

    /* ----------------------------------------------------------------------------------------------------------------
     */

    cycle++; /* increment the cycle number */

    nSuccess = 0; /* reset progress parameters */
    nRandomChanges = 0;

    for (nReconfigurations = 0;
         nReconfigurations < MAXRECONFACTOR * grid->nGridPoints &&
         nSuccess + nRandomChanges < MAXRECONFACTOR / 10 * grid->nGridPoints;
         nReconfigurations++) {
      randomPoint =
          (int)(grid->nGridPoints * (double)rand() / (double)RAND_MAX);
      randomValue =
          (int)((grid->nComponents + 1) * (double)rand() / (double)RAND_MAX);

      if (randomValue !=
          grid[randomPoint].value) { /* if there has been no change, go back and
                                        try again! */

        adjustArea(grid[randomPoint].value, randomValue, grid);

        oldValue = grid[randomPoint].value;
        grid[randomPoint].value = randomValue;
        grid->looseness = looseness(grid);
        grid[randomPoint].value = oldValue;

        if (randomPoint % grid->nX == grid->nX - 1) {
          /* adjusting the compactness for atoms that share a boundry with a
           * symmetry related grid is tricky: just recalculate the whole thing
           * in such cases */

          oldValue = grid[randomPoint].value;
          grid[randomPoint].value = randomValue;
          grid->compactness = compactness(grid);
          grid[randomPoint].value = oldValue;

        }

        else
          grid->compactness +=
              adjustCompactness(randomValue, &grid[randomPoint]);

        zeroPr(tempPr);
        getPartialPr(tempPr, grid, &grid[randomPoint]);

        for (i = 0; i < pr[0]->nComponents; i++)
          for (j = 0; j < pr[0]->nPrPoints; j++) {
            /* getPartialPr calculates the pr due to a single atom surrounded by
             * others, neglecting any distances between itself and its symmetry
             * related self. If we change an atom from x to y, then we must
             * subtract: pr[x][1,2,3,4] from the current pr, and add
             * pr[y][1,2,3,4] */

            pr[grid[randomPoint].value][i].pr[j] -=
                2.0 * tempPr[grid[randomPoint].value][i].pr[j];
            pr[randomValue][i].pr[j] +=
                2.0 * tempPr[grid[randomPoint].value][i].pr[j];
          }

        for (i = 1; i < grid->nSymmOps;
             i++) { /* treat the component neglected in getPartialPr */

          r = sqrt(X2(grid[randomPoint].xy[i][0] - grid[randomPoint].xy[0][0]) +
                   X2(grid[randomPoint].xy[i][1] - grid[randomPoint].xy[0][1]));

          updatePrGaussian(
              pr[grid[randomPoint].value][grid[randomPoint].value].pr, r,
              (int)r, -(double)grid->nSymmOps);
          updatePrGaussian(pr[randomValue][randomValue].pr, r, (int)r,
                           (double)grid->nSymmOps);
        }

        getMoments(pr, model);
        grid->RgPenalty = RgPenalty(model);
        getIqcalc(pr, data, model);
        getScaleBackground(model, data);
        tempResidual = totalResidual(model, grid);

        if (tempResidual - currentResidual < 0.0)
          accepted = 1;
        else if (tempResidual - currentResidual > 0.0 &&
                 exp(-(tempResidual - currentResidual) / temperature) >
                     (double)rand() / (double)RAND_MAX)
          accepted = 2;

        /* model is accepted as the residual is lower (accepted == 1), or
         * because the temperature was high enough to allow the residual to go
         * higher (accepted == 2) */

        else
          accepted = 0; /* model is rejected */

        if (accepted > 0) { /* update the value of the point and various
                               residuals and progress parameters */

          grid[randomPoint].value = randomValue;

          currentResidual = tempResidual;
          if (accepted == 1) nSuccess++;
          if (accepted == 2) nRandomChanges++;

        }

        else { /* Undo any changes we made to the model */

          adjustArea(randomValue, grid[randomPoint].value, grid);
          grid->looseness = looseness(grid);

          if (randomPoint % grid->nX == grid->nX - 1)
            grid->compactness = compactness(grid);
          else
            grid->compactness -=
                adjustCompactness(randomValue, &grid[randomPoint]);

          for (i = 0; i < pr[0]->nComponents; i++)
            for (j = 0; j < pr[0]->nPrPoints; j++) {
              pr[grid[randomPoint].value][i].pr[j] +=
                  2.0 * tempPr[grid[randomPoint].value][i].pr[j];
              pr[randomValue][i].pr[j] -=
                  2.0 * tempPr[grid[randomPoint].value][i].pr[j];
            }

          for (i = 1; i < grid->nSymmOps;
               i++) { /* treat the component neglected in getPartialPr */

            r = sqrt(
                X2(grid[randomPoint].xy[i][0] - grid[randomPoint].xy[0][0]) +
                X2(grid[randomPoint].xy[i][1] - grid[randomPoint].xy[0][1]));

            updatePrGaussian(
                pr[grid[randomPoint].value][grid[randomPoint].value].pr, r,
                (int)r, (double)grid->nSymmOps);
            updatePrGaussian(pr[randomValue][randomValue].pr, r, (int)r,
                             -(double)grid->nSymmOps);
          }
        }
      }

      if (nReconfigurations % 100 ==
          0) /* periodically write information to the screen */
        fprintf(stderr,
                "nSuccess = %d, nRandomChanges = %d, nTotalChanges = %d(%d), "
                "NRecon = %d(%d), currentResidual = %.2lf\r",
                nSuccess, nRandomChanges, nSuccess + nRandomChanges,
                MAXRECONFACTOR / 10 * grid->nGridPoints, nReconfigurations,
                MAXRECONFACTOR * grid->nGridPoints, currentResidual);
    }

    fprintf(stderr,
            "nSuccess = %d, nRandomChanges = %d, nTotalChanges = %d(%d), "
            "NRecon = %d(%d), finalResidual = %.2lf    \n",
            nSuccess, nRandomChanges, nSuccess + nRandomChanges,
            MAXRECONFACTOR / 10 * grid->nGridPoints, nReconfigurations,
            MAXRECONFACTOR * grid->nGridPoints,
            currentResidual); /* write the information for the run completed at
                                 that temperature */

    temperature *= COOLINGFACTOR; /* decrease the temperature and start again */

  } while (nSuccess != 0); /* keep going while there are successes */

  return (1);
}
};  // namespace xsr

#endif
#endif

// ------- the class to run xsr

US_Hydrodyn_Xsr::US_Hydrodyn_Xsr(US_Hydrodyn *us_hydrodyn, QWidget *p,
                                 const char *)
    : QFrame(p) {
  this->us_hydrodyn = us_hydrodyn;
  this->our_saxs_options = &(us_hydrodyn->saxs_options);
  this->saxs_widget = &(us_hydrodyn->saxs_plot_widget);
  this->saxs_window = us_hydrodyn->saxs_plot_window;

  USglobal = new US_Config();
  setPalette(PALET_FRAME);
  setWindowTitle(us_tr("US-SOMO: SAXS Cross Sectional Analysis"));

  setupGUI();
  running = false;

  update_enables();

  editor_msg("blue", "THIS WINDOW IS UNDER DEVELOPMENT");

  global_Xpos += 30;
  global_Ypos += 30;

  setGeometry(global_Xpos, global_Ypos, 0, 0);
}

US_Hydrodyn_Xsr::~US_Hydrodyn_Xsr() {}

void US_Hydrodyn_Xsr::setupGUI() {
  int minHeight1 = 30;
#if QT_VERSION < 0x040000 || !defined(Q_OS_MAC)
  int minHeight3 = 30;
#endif

  lbl_title = new QLabel(us_tr("US-SOMO: SAXS Cross Sectional Analysis"), this);
  lbl_title->setFrameStyle(QFrame::WinPanel | QFrame::Raised);
  lbl_title->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
  lbl_title->setMinimumHeight(minHeight1);
  lbl_title->setPalette(PALET_FRAME);
  AUTFBACK(lbl_title);
  lbl_title->setFont(QFont(USglobal->config_list.fontFamily,
                           USglobal->config_list.fontSize + 1, QFont::Bold));

  pb_start = new QPushButton(us_tr("Start"), this);
  pb_start->setFont(QFont(USglobal->config_list.fontFamily,
                          USglobal->config_list.fontSize + 1));
  pb_start->setMinimumHeight(minHeight1);
  pb_start->setPalette(PALET_PUSHB);
  connect(pb_start, SIGNAL(clicked()), SLOT(start()));

  pb_stop = new QPushButton(us_tr("Stop"), this);
  pb_stop->setFont(QFont(USglobal->config_list.fontFamily,
                         USglobal->config_list.fontSize + 1));
  pb_stop->setMinimumHeight(minHeight1);
  pb_stop->setPalette(PALET_PUSHB);
  connect(pb_stop, SIGNAL(clicked()), SLOT(stop()));

  progress = new QProgressBar(this);
  progress->setMinimumHeight(minHeight1);
  progress->setPalette(PALET_NORMAL);
  AUTFBACK(progress);
  progress->reset();

  editor = new QTextEdit(this);
  editor->setPalette(PALET_NORMAL);
  AUTFBACK(editor);
  editor->setReadOnly(true);

#if QT_VERSION < 0x040000
#if QT_VERSION >= 0x040000 && defined(Q_OS_MAC)
  {
    //      Q3PopupMenu * file = new Q3PopupMenu;
    file->insertItem(us_tr("&Font"), this, SLOT(update_font()),
                     Qt::ALT + Qt::Key_F);
    file->insertItem(us_tr("&Save"), this, SLOT(save()), Qt::ALT + Qt::Key_S);
    file->insertItem(us_tr("Clear Display"), this, SLOT(clear_display()),
                     Qt::ALT + Qt::Key_X);

    QMenuBar *menu = new QMenuBar(this);
    AUTFBACK(menu);

    menu->insertItem(us_tr("&Messages"), file);
  }
#else
  QFrame *frame;
  frame = new QFrame(this);
  frame->setMinimumHeight(minHeight3);

  m = new QMenuBar(frame);
  m->setObjectName("menu");
  m->setMinimumHeight(minHeight1 - 5);
  m->setPalette(PALET_NORMAL);
  AUTFBACK(m);
  //   Q3PopupMenu * file = new Q3PopupMenu(editor);
  m->insertItem(us_tr("&File"), file);
  file->insertItem(us_tr("Font"), this, SLOT(update_font()),
                   Qt::ALT + Qt::Key_F);
  file->insertItem(us_tr("Save"), this, SLOT(save()), Qt::ALT + Qt::Key_S);
  file->insertItem(us_tr("Clear Display"), this, SLOT(clear_display()),
                   Qt::ALT + Qt::Key_X);
#endif
#else
#if defined(Q_OS_MAC)
  m = new QMenuBar(this);
  m->setObjectName("menu");
#else
  QFrame *frame;
  frame = new QFrame(this);
  frame->setMinimumHeight(minHeight3);
  frame->setPalette(PALET_NORMAL);
  AUTFBACK(frame);
  m = new QMenuBar(frame);
  m->setObjectName("menu");
#endif
  m->setMinimumHeight(minHeight1 - 5);
  m->setPalette(PALET_NORMAL);
  AUTFBACK(m);

  {
    QMenu *new_menu = m->addMenu(us_tr("&File"));

    QAction *qa1 = new_menu->addAction(us_tr("Font"));
    qa1->setShortcut(Qt::ALT + Qt::Key_F);
    connect(qa1, SIGNAL(triggered()), this, SLOT(update_font()));

    QAction *qa2 = new_menu->addAction(us_tr("Save"));
    qa2->setShortcut(Qt::ALT + Qt::Key_S);
    connect(qa2, SIGNAL(triggered()), this, SLOT(save()));

    QAction *qa3 = new_menu->addAction(us_tr("Clear Display"));
    qa3->setShortcut(Qt::ALT + Qt::Key_X);
    connect(qa3, SIGNAL(triggered()), this, SLOT(clear_display()));
  }
#endif

  editor->setWordWrapMode(QTextOption::WordWrap);
  // editor->setMinimumHeight(300);

  pb_help = new QPushButton(us_tr("Help"), this);
  pb_help->setFont(QFont(USglobal->config_list.fontFamily,
                         USglobal->config_list.fontSize + 1));
  pb_help->setMinimumHeight(minHeight1);
  pb_help->setPalette(PALET_PUSHB);
  connect(pb_help, SIGNAL(clicked()), SLOT(help()));

  pb_cancel = new QPushButton(us_tr("Close"), this);
  pb_cancel->setFont(QFont(USglobal->config_list.fontFamily,
                           USglobal->config_list.fontSize + 1));
  pb_cancel->setMinimumHeight(minHeight1);
  pb_cancel->setPalette(PALET_PUSHB);
  connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

  // build layout
  // grid for options

  QBoxLayout *vbl_editor_group = new QVBoxLayout(0);
  vbl_editor_group->setContentsMargins(0, 0, 0, 0);
  vbl_editor_group->setSpacing(0);
#if QT_VERSION < 0x040000 || !defined(Q_OS_MAC)
  vbl_editor_group->addWidget(frame);
#endif
  vbl_editor_group->addWidget(editor);

  QHBoxLayout *hbl_controls = new QHBoxLayout();
  hbl_controls->setContentsMargins(0, 0, 0, 0);
  hbl_controls->setSpacing(0);
  hbl_controls->addSpacing(4);
  hbl_controls->addWidget(pb_start);
  hbl_controls->addSpacing(4);
  hbl_controls->addWidget(progress);
  hbl_controls->addWidget(pb_stop);
  hbl_controls->addSpacing(4);

  QVBoxLayout *vbl_target_controls = new QVBoxLayout(0);
  vbl_target_controls->setContentsMargins(0, 0, 0, 0);
  vbl_target_controls->setSpacing(0);
  vbl_target_controls->addLayout(hbl_controls);

  QHBoxLayout *hbl_bottom = new QHBoxLayout();
  hbl_bottom->setContentsMargins(0, 0, 0, 0);
  hbl_bottom->setSpacing(0);
  hbl_bottom->addSpacing(4);
  hbl_bottom->addWidget(pb_help);
  hbl_bottom->addSpacing(4);
  hbl_bottom->addWidget(pb_cancel);
  hbl_bottom->addSpacing(4);

  QVBoxLayout *background = new QVBoxLayout(this);
  background->setContentsMargins(0, 0, 0, 0);
  background->setSpacing(0);
  background->addSpacing(4);
  background->addWidget(lbl_title);
  background->addSpacing(4);
  background->addLayout(vbl_editor_group);
  background->addSpacing(4);
  background->addLayout(vbl_target_controls);
  background->addSpacing(4);
  background->addLayout(hbl_bottom);
  background->addSpacing(4);
}

void US_Hydrodyn_Xsr::cancel() { close(); }

void US_Hydrodyn_Xsr::help() {
  US_Help *online_help;
  online_help = new US_Help(this);
  online_help->show_help("manual/somo/somo_saxs_xsr.html");
}

void US_Hydrodyn_Xsr::closeEvent(QCloseEvent *e) {
  // ((US_Hydrodyn *)us_hydrodyn)->saxs_2d_widget = false;

  global_Xpos -= 30;
  global_Ypos -= 30;
  e->accept();
}

void US_Hydrodyn_Xsr::clear_display() {
  editor->clear();
  editor->append("\n\n");
}

void US_Hydrodyn_Xsr::update_font() {
  bool ok;
  QFont newFont;
  newFont = QFontDialog::getFont(&ok, ft, this);
  if (ok) {
    ft = newFont;
  }
  editor->setFont(ft);
}

void US_Hydrodyn_Xsr::save() {
  QString fn;
  fn = QFileDialog::getSaveFileName(this, windowTitle(), QString(), QString());
  if (!fn.isEmpty()) {
    QString text = editor->toPlainText();
    QFile f(fn);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
      return;
    }
    QTextStream t(&f);
    t << text;
    f.close();
    //      editor->setModified( false );
    setWindowTitle(fn);
  }
}

void US_Hydrodyn_Xsr::start() {}

void US_Hydrodyn_Xsr::stop() {
  running = false;
  editor_msg("red", "Stopped by user request\n");
  update_enables();
}

void US_Hydrodyn_Xsr::update_enables() {
  pb_start->setEnabled(!running);
  pb_stop->setEnabled(running);
}

void US_Hydrodyn_Xsr::editor_msg(QString color, QString msg) {
  QColor save_color = editor->textColor();
  editor->setTextColor(color);
  editor->append(msg);
  editor->setTextColor(save_color);
}

// ------ US_Hydrodyn_Saxs entry

void US_Hydrodyn_Saxs::saxs_xsr() {
  if (((US_Hydrodyn *)us_hydrodyn)->sas_options_xsr_widget) {
    if (((US_Hydrodyn *)us_hydrodyn)->sas_options_xsr_window->isVisible()) {
      ((US_Hydrodyn *)us_hydrodyn)->sas_options_xsr_window->raise();
    } else {
      ((US_Hydrodyn *)us_hydrodyn)->sas_options_xsr_window->show();
    }
    return;
  } else {
    ((US_Hydrodyn *)us_hydrodyn)->sas_options_xsr_window =
        new US_Hydrodyn_SasOptionsXsr(
            our_saxs_options,
            &(((US_Hydrodyn *)us_hydrodyn)->sas_options_xsr_widget),
            us_hydrodyn);
    US_Hydrodyn::fixWinButtons(
        ((US_Hydrodyn *)us_hydrodyn)->sas_options_xsr_window);
    ((US_Hydrodyn *)us_hydrodyn)->sas_options_xsr_window->show();
  }

  US_Hydrodyn_Xsr *uhxsr = new US_Hydrodyn_Xsr((US_Hydrodyn *)us_hydrodyn);
  US_Hydrodyn::fixWinButtons(uhxsr);
  uhxsr->show();
}
