/*
 * Project:  LevenbergMarquardtLeastSquaresFitting
 *
 * File:     lmcurve.h
 *
 * Contents: Simplified interface for one-dimensional curve fitting
 *
 * Author:   Joachim Wuttke 2010
 *
 * Homepage: joachimwuttke.de/lmfit
 */

#ifndef US_LM_H
#define US_LM_H

#include <float.h>
#include <math.h>

#include "us_extern.h"
#include "us_math2.h"

#define LM_USRTOL 30 * DBL_EPSILON

/*! \class US_LM
 * Class of static functions for Levenberg-Marquardt computations
 */

class US_UTIL_EXTERN US_LM {
 public:
  //! Class holding input controls for LM computations
  class US_UTIL_EXTERN LM_Control {
   public:
    double ftol;       //!< relative error desired in the sum of squares
    double xtol;       //!< relat. error between last two approximations
    double gtol;       //!< orthogonality desired between fvec and derivs
    double epsilon;    //!< step used to calculate the jacobian.
    double stepbound;  //!< initial bound to steps in the outer loop.
    int maxcall;       //!< maximum number of iterations.
    int scale_diag;    //!< TESTWISE automatic diag rescaling?
    int printflags;    //!< OR'ed to produce more noise

    //! Constructor for LM_Control class
    LM_Control(double = LM_USRTOL, double = LM_USRTOL, double = LM_USRTOL,
               double = LM_USRTOL, double = 100.0, int = 100, int = 1, int = 0);
  };

  //! Collection of output status parameters from LM computations.
  class US_UTIL_EXTERN LM_Status {
   public:
    double fnorm;  //!< norm of the residue vector fvec.
    int nfev;      //!< actual number of iterations.
    int info;      //!< status (index for infmsg and shortmsg).

    //! Constructor for LM_Status class
    LM_Status(double = 0.0, int = 0, int = 0);
  };

  //! Collection of LM Curve data
  class US_UTIL_EXTERN LM_CurveData {
   public:
    double *t;                      //!< test value array
    double *y;                      //!< Y value array
    double (*f)(double, double *);  //!< Function for eval.

    //! Constructor for LM_CurveData class
    LM_CurveData(double *, double *, double (*)(double, double *));
  };

  //! Recommended control parameter settings.
  const LM_Control lm_control_double;  //!< controls in double format
  const LM_Control lm_control_float;   //!< controls in float format

  //! Status message string for termination condition
  static QString lm_statmsg(LM_Status *, bool = false);

  //! Standard monitoring routine.
  static void lm_printout_std(int n_par, double *par, int m_dat,
                              const void *data, const double *fvec,
                              int printflags, int iflag, int iter, int nfev);

  //! RMSD Norm
  static double lm_rmsdnorm(int, const double *);

  //! Refined calculation of Eucledian norm, often used in printout routine.
  static double lm_enorm(int, const double *);

  //! The actual minimization. */
  static void lmmin(int n_par, double *par, int m_dat, const void *data,
                    void (*evaluate)(double *par, int m_dat, const void *data,
                                     double *fvec, int *info),
                    const LM_Control *control, LM_Status *status,
                    void (*printout)(int n_par, double *par, int m_dat,
                                     const void *data, const double *fvec,
                                     int printflags, int iflag, int iter,
                                     int nfev));

  /** Legacy low-level interface. **/

  //! Alternative to lm_minimize, allowing full control, and read-out
  //!  of auxiliary arrays. For usage, see implementation of lmmin.
  static void lm_lmdif(
      int m, int n, double *x, double *fvec, double ftol, double xtol,
      double gtol, int maxfev, double epsfcn, double *diag, int mode,
      double factor, int *info, int *nfev, double *fjac, int *ipvt, double *qtf,
      double *wa1, double *wa2, double *wa3, double *wa4,
      void (*evaluate)(double *par, int m_dat, const void *data, double *fvec,
                       int *info),
      void (*printout)(int n_par, double *par, int m_dat, const void *data,
                       const double *fvec, int printflags, int iflag, int iter,
                       int nfev),
      int printflags, const void *data);

  //      extern const char *lm_infmsg[];
  //      extern const char *lm_shortmsg[];

  //! Evaluate functions
  static void lmcurve_evaluate(double *par, int m_dat, const void *data,
                               double *fvec, int * /* info */);

  //! LM Curve Fit
  static void lmcurve_fit(int n_par, double *par, int m_dat, const double *t,
                          const double *y, double (*f)(double t, double *par),
                          const LM_Control *control, LM_Status *status);

  //! LM Curve Fit by RMSD
  static void lmcurve_fit_rmsd(int n_par, double *par, int m_dat,
                               const double *t, const double *y,
                               double (*f)(double t, double *par),
                               const LM_Control *control, LM_Status *status);

  //*************************************************************************/
  //  lm_lmdif (low-level, modified legacy interface for full control)
  //*************************************************************************/

  static void lm_lmpar(int n, double *r, int ldr, int *ipvt, double *diag,
                       double *qtb, double delta, double *par, double *x,
                       double *sdiag, double *aux, double *xdi);
  static void lm_qrfac(int m, int n, double *a, int pivot, int *ipvt,
                       double *rdiag, double *acnorm, double *wa);
  static void lm_qrsolv(int n, double *r, int ldr, int *ipvt, double *diag,
                        double *qtb, double *x, double *sdiag, double *wa);
  //      extern unsigned int exponential_terms;
  //      extern double       minusoneoverfourpisq;
  //      double compute_exponential_f( double t, const double *par );

};  // Class US_LM

#endif /* US_LM_H */
