//Added by qt3to4:
#include <QCloseEvent>
#include <QFrame>
#include <QLabel>
#ifndef US_HYDRODYN_XSR
#define US_HYDRODYN_XSR

#if QT_VERSION < 0x040000 && defined(Q_OS_WIN)
#if !defined(Q_OS_WIN)

/****************************************************************** 
 ***                       mathFunctions.h                      *** 
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

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if defined(WIN32)

double erf(double x) {
   // constants
   double a1 = 0.254829592;
   double a2 = -0.284496736;
   double a3 = 1.421413741;
   double a4 = -1.453152027;
   double a5 = 1.061405429;
   double p = 0.3275911;

   // Save the sign of x
   int sign = 1;
   if (x < 0)
      sign = -1;
   x = fabs(x);

   // A&S formula 7.1.26
   double t = 1.0 / (1.0 + p * x);
   double y = 1.0 - (((((a5 * t + a4) * t) + a3) * t + a2) * t + a1) * t * exp(-x * x);

   return sign * y;
}
#endif

#define PI 3.141592653589793238462643383279502884197169399375105820974944592
/* definition of Pi to 63 dec. pl. */

#define TRAPEZOID(f0, f1, h) ((h) * ((f0) + (f1)) / 2.0)
/* trapezoidal intergration macro */

#define SIMPSON(f0, f1, f2, h) ((h) / 3.0 * ((f0) + 4.0 * (f1) + (f2)))
/* Simpson's integration macro */

#define X2(x) ((x) * (x))
/* square of a number macro */

#define X3(x) ((x) * (x) * (x))
/* cube of a number macro */

#define GAUSSIAN(value, mean, stdDev) \
   (1.0 / sqrt(2.0 * PI * (stdDev) * (stdDev)) \
    * exp(-((value) - (mean)) * ((value) - (mean)) / (2.0 * (stdDev) * (stdDev))))
/*normal distribution macro */

#define INT(p, columns, name, func_name, i) \
   { \
      if ((p = ( int * ) malloc(columns * sizeof(int))) == NULL) { \
         fprintf(stderr, "\nError: Can't allocate memory for *%s in function %s.\n", name, func_name); \
         return (0); \
      } \
\
      for (i = 0; i < columns; i++) \
         p[ i ] = 0; \
   }
/* integer array memory allocation macro */

#define DOUBLE(p, columns, name, func_name, i) \
   { \
      if ((p = ( double * ) malloc((columns) * sizeof(double))) == NULL) { \
         fprintf(stderr, "\nError: Can't allocate memory for *%s in function %s.\n", name, func_name); \
         return (0); \
      } \
\
      for (i = 0; i < columns; i++) \
         p[ i ] = 0.0; \
   } \
   /* double memory allocation macro */

#define DOUBLE2D(p, rows, columns, name, func_name, i, j) \
   { \
      if ((p = ( double ** ) malloc((rows) * sizeof(double))) == NULL) { \
         fprintf(stderr, "\nError: Can't allocate memory for **%s in function %s.\n", name, func_name); \
         return (0); \
      } \
\
      if ((p[ 0 ] = ( double * ) malloc((rows) * (columns) * sizeof(double))) == NULL) { \
         fprintf(stderr, "\nError: Can't allocate memory for *%s in function %s.\n", name, func_name); \
         return (0); \
      } \
\
      for (i = 1; i < rows; i++) \
         p[ i ] = p[ i - 1 ] + columns; \
\
      for (i = 0; i < rows; i++) \
         for (j = 0; j < columns; j++) \
            p[ i ][ j ] = 0.0; \
   } \
   /* 2D double memory allocation macro */

double linearFit(double *X, double *Y, double *sigY, int n, double *mb); /* LSQ fit of y = mx + b: returns chi2 */

double lagrangeInterpolation(double *xn, double *yn, double x, int n);

/****************************************************************** 
 ***                        XSFunctions.h                       *** 
 ******************************************************************
 *
 * Additional Info:
 *
 * Date: 10/5/08 last modified 6/10/08
 * Author: A.E. Whitten
 * Description:
 * 
 * Function prototypes for mathFunctions.c
 *
 * Version 1.0: File created
 *
 * Version 1.1: Interpolation function added
 */

#define NPMOD 51
/* number of points to use in calculating the calclated model profile */

#define QMAX 0.50
/* maximum q-value to calculate */

#define QMIN 1.0E-6
/* minimum q-value to calculate */

#define NPR 151
/* maximum number of pr points to expect */

#define COOLINGFACTOR 0.95
/* temperature scheduling factor */

#define RGPENALTYWEIGHT 1.0
/* weight of the Rg penalty in the total residual */

#define NPEXP 200
/* maximum number of experimental data points to expect */

#define NPRES 17
/* number of points that describe the resolution function */

#define CALCT(p, columns, name, funcName, i) \
   { \
      if ((p = ( calc_t * ) malloc((columns) * sizeof(calc_t))) == NULL) { \
         fprintf(stderr, "\nError: Can't allocate memory for *%s in function %s.\n", name, funcName); \
         return (0); \
      } \
\
      p->nContrastPoints = columns; \
   }
/* calc_t array memory allocation macro */

#define EXPT(p, columns, name, funcName, i) \
   { \
      if ((p = ( exp_t * ) malloc((columns) * sizeof(exp_t))) == NULL) { \
         fprintf(stderr, "\nError: Can't allocate memory for *%s in function %s.\n", name, funcName); \
         return (0); \
      } \
\
      p->nContrastPoints = columns; \
   }
/* exp_t array memory allocation macro */

#define GRIDT(p, columns, name, funcName, i) \
   { \
      if ((p = ( grid_t * ) malloc((columns) * sizeof(grid_t))) == NULL) { \
         fprintf(stderr, "\nError: Can't allocate memory for *%s in function %s.\n", name, funcName); \
         return (0); \
      } \
\
      p->nGridPoints = columns; \
   }
/* grid_t array memory allocation macro */

#define PRT2D(p, rows, columns, name, funcName, i, j) \
   { \
      if ((p = ( pr_t ** ) malloc((rows) * sizeof(pr_t))) == NULL) { \
         fprintf(stderr, "\nError: Can't allocate memory for *%s in function %s.\n", name, funcName); \
         return (0); \
      } \
\
      if ((p[ 0 ] = ( pr_t * ) malloc((rows) * (columns) * sizeof(pr_t))) == NULL) { \
         fprintf(stderr, "\nError: Can't allocate memory for *%s in function %s.\n", name, funcName); \
         return (0); \
      } \
\
      for (i = 1; i < rows; i++) \
         p[ i ] = p[ i - 1 ] + columns; /* Point the pointers to the allocated memory */ \
\
\
      for (i = 0; i < (rows); i++) \
         for (j = 0; j < (columns); j++) \
            for (k = 0; k < NPR; k++) \
               p[ i ][ j ].pr[ k ] = 0.0; /* Point the pointers to the allocated memory */ \
\
      p[ 0 ]->nComponents = rows; \
   }
/* 2D pr_t array memory allocation macro */

typedef struct grids {
      int nGridPoints, nX, nY, nComponents, nSymmOps, nContacts, value, area[ 5 ], totalArea, searched;

      /* nGridPoints is the number of gridPoints, and the size of the grid array; nX and nY are the dimensions of
    * the grid in X and Y dimensions; nComponents is the number of different components the grid can be comprised of;
    * nSymmOps is the number of symmetry operations required to generate the entire grid; nContacts is the number of
    * neighbours in the grid that are the same type of component; area is the total area occupied by each component;
    * totalArea is the entire area occupied by non-solvent components; searched is a flag for the connectivity search */

      double xy[ 4 ][ 2 ], gridDistance, compactness, compactnessWeight, looseness, loosenessWeight, RgPenalty;

      /* xy[4][] are the xy coordinates of a given point and all its symmetry related points; gridDistance is the conversion
    * factor, to turn grid units into actual distances; compactness is a measure of the compactness of the components in the
    * grid; compactnessWeight is the weight given to the compactness in the total residual; RgPenalty is the penalty to the
    * total residual if any contrast point exceedes a specified value */

      struct grids *contact[ 8 ];

      /* contact is a pointer to another grid element that neigbours a given grid element */

} grid_t; /* data structure containing information pertaining to grids */

typedef struct {
      char filename[ 30 ], resType[ 4 ];

      /* filename is the name of the file contain the contrast variation data */

      int nDataPoints, nContrastPoints;

      /* nDataPoints is the number of data points in the scattering data; nContrastPoints is the number of contrast variation data
    * sets, and is the size of the data array; resMax and resMin refer to the limits that are summed over when apply the resolution
    * function to the model */

      double q[ NPEXP ], Iq[ NPEXP ], sigIq[ NPEXP ], *sigq, *qbar, *sF, A, L, *slit, *qSlit, **resFunction,
         **qResFunction;

      /* q is the momentum transfer (scattering angle 4PIsin(theta)/lambda); Iq is the scattered intensity; sigq is the stdDev (spread)
    * of q that causes smearing; qbar is the mean q; sF is the shadow factor due to the beam stop; A and L are the slit smearing 
    * parameters used in GNOM; slit and qSlit represent the Slit beam profile, and resFunction and qResFunction is the NIST resolution 
    * function calculated from the various parameters mentioned */

} exp_t; /* data structure containing information pertaining to measured scattering data */

typedef struct {
      int nDataPoints, nContrastPoints, nComponents;

      /* nDataPoints is the number of data points in the scattering data; nContrastPoints is the number of contrast variation data
    * sets, and is the size of the data array; nComponents is the number of different components the structure can be comprised of */

      double Iq[ NPEXP ], q[ NPEXP ], mb[ 2 ], I0, Rg, chi2, RgMax;

      /* Iq is the calculated scattering profile (smeared by the resolution function); q is the associated momentum transfer (same as
    * experimental values); mb contains scale (mb[0]) and background corrections (mb[1]) to compare the calculated and experimental
    * data; I0 is the calculated zero angle scattering of cross-section; Rg is the radius of gyration of cross-section; chi2 is a
    * measure of agreeement between the experimental and calculated data; RgMax is the maxium expected Rg value (used to calculate
    * the Rg penalty */

      char outFilename[ 30 ];

      /* outFilename is the filename intensity data is written to (_fit.data extension) */

} calc_t; /* data structure containing information pertaining to calculated scattering data */

typedef struct {
      int nPrPoints, nComponents, nContrastPoints;

      /* nPrPoints is the number of pr points used in various calculations; nComponents is the number of different components the 
    * structure can be comprised of, and is the size of the array in each dimension; nContrastPoints is the number of contrast 
    * variation data sets */

      double pr[ NPR ], r[ NPR ], Drho[ 5 ];

      /* pr stores the pr data; r is the associated distance; Drho[5] is the contrast**2 of that pr type */

} pr_t; /* data structure containing information pertaining to calculated pr data */

int getResolutionFunction(exp_t *data); /* calculates the instrumental resolution funtion */

int getScaleBackground(
   calc_t *model, exp_t *data); /* determines scale and background corrections for calculated scattering profiles */

int initialiseGrid(grid_t *grid, int symmOp); /* initialise the grid information */

int getIqcalc(pr_t **pr, exp_t *data, calc_t *model); /* calculates the scattering profile from the pr, and smears it */

double interpolatedIq(double *q, double *Iq, double qint); /* interpolates the intensity profiles */

double RgPenalty(calc_t *model); /* returns the Rg penalty */

double adjustCompactness(int newValue, grid_t *grid); /* returns the adjustment to the compactness for a reconfiguration */

double partialCompactness(grid_t *grid); /* returns the compactness for a given grid element */

double compactness(grid_t *grid); /* returns the total compactness for the grid */

int partialLooseness(grid_t *grid, int value); /* returns the looseness for the grid */

double looseness(grid_t *grid); /* returns the looseness for the grid */

int updatePrGaussian(
   double *pr, double mean, int bin,
   double nSymmOps); /* smears the contribution to the pr according to a gaussian distribution */

int getPartialPr(
   pr_t **pr, grid_t *grid, grid_t *point); /* calculates the contribution to the pr of a single grid element */

int getPr(pr_t **pr, grid_t *grid); /* determines the total pr function */

int zeroPr(pr_t **pr); /* zeros the pr data type */

int getMoments(pr_t **pr, calc_t *model); /* calculates I0 and Rg from the pr distribution */

double totalResidual(calc_t *model, grid_t *grid); /* calculates the total residual that is minimised */

int printIqModel(calc_t *model); /* prints Iq information */

int printPrModel(pr_t **pr); /* prints the pr of the model at each contrast point */

int printPDB(grid_t *grid, char *filename); /* prints a pbd verison of the grid */

int printInformation(calc_t *model, grid_t *grid, int cycle); /* prints general information to an output file */

#endif
#endif

#include "us_hydrodyn.h"
#include "us_hydrodyn_saxs.h"
#include "us_saxs_util.h"

class US_EXTERN US_Hydrodyn_Xsr : public QFrame {
      Q_OBJECT

      friend class US_Hydrodyn_Saxs;

   public:
      US_Hydrodyn_Xsr(US_Hydrodyn *us_hydrodyn, QWidget *p = 0, const char *name = 0);
      ~US_Hydrodyn_Xsr();

   private:
      US_Config *USglobal;

      QLabel *lbl_title;

      QPushButton *pb_start;
      QProgressBar *progress;
      QPushButton *pb_stop;

      QFont ft;
      QTextEdit *editor;
      QMenuBar *m;

      QPushButton *pb_help;
      QPushButton *pb_cancel;

      void editor_msg(QString color, QString msg);

      QString filename;
      saxs_options *our_saxs_options;
      US_Hydrodyn *us_hydrodyn;
      US_Hydrodyn_Saxs *saxs_window;
      bool *saxs_widget;

      bool keep_files;

      bool running;

#ifdef WIN32
#if QT_VERSION < 0x040000
#pragma warning(disable : 4251)
#endif
#endif

      vector<US_Saxs_Scan> data;

      bool run(QString out_filename, vector<US_Saxs_Scan> data, bool keep_files = false);
#ifdef WIN32
#if QT_VERSION < 0x040000
#pragma warning(default : 4251)
#endif
#endif


      QString error_msg;

   private slots:

      void setupGUI();

      void start();
      void stop();

      void clear_display();
      void update_font();
      void save();

      void cancel();
      void help();

      void update_enables();

   protected slots:

      void closeEvent(QCloseEvent *);
};

#endif
