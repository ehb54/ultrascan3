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

namespace LM {
extern "C" {

/** Compact high-level interface. **/

/* Collection of control (input) parameters. */
typedef struct {
  double ftol;      /* relative error desired in the sum of squares. */
  double xtol;      /* relative error between last two approximations. */
  double gtol;      /* orthogonality desired between fvec and its derivs. */
  double epsilon;   /* step used to calculate the jacobian. */
  double stepbound; /* initial bound to steps in the outer loop. */
  int maxcall;      /* maximum number of iterations. */
  int scale_diag;   /* UNDOCUMENTED, TESTWISE automatical diag rescaling? */
  int printflags;   /* OR'ed to produce more noise */
} lm_control_struct;

/* Collection of status (output) parameters. */
typedef struct {
  double fnorm; /* norm of the residue vector fvec. */
  int nfev;     /* actual number of iterations. */
  int info;     /* status (index for lm_infmsg and lm_shortmsg). */
} lm_status_struct;

/* Recommended control parameter settings. */
extern const lm_control_struct lm_control_double;
extern const lm_control_struct lm_control_float;

/* Standard monitoring routine. */
void lm_printout_std(int n_par, const double *par, int m_dat, const void *data,
                     const double *fvec, int printflags, int iflag, int iter,
                     int nfev);

double lm_rmsdnorm(int, const double *);

/* Refined calculation of Eucledian norm, typically used in printout routine. */
double lm_enorm(int, const double *);

/* The actual minimization. */
void lmmin(int n_par, double *par, int m_dat, const void *data,
           void (*evaluate)(const double *par, int m_dat, const void *data,
                            double *fvec, int *info),
           const lm_control_struct *control, lm_status_struct *status,
           void (*printout)(int n_par, const double *par, int m_dat,
                            const void *data, const double *fvec,
                            int printflags, int iflag, int iter, int nfev));

/** Legacy low-level interface. **/

/* Alternative to lm_minimize, allowing full control, and read-out
   of auxiliary arrays. For usage, see implementation of lmmin. */
void lm_lmdif(int m, int n, double *x, double *fvec, double ftol, double xtol,
              double gtol, int maxfev, double epsfcn, double *diag, int mode,
              double factor, int *info, int *nfev, double *fjac, int *ipvt,
              double *qtf, double *wa1, double *wa2, double *wa3, double *wa4,
              void (*evaluate)(const double *par, int m_dat, const void *data,
                               double *fvec, int *info),
              void (*printout)(int n_par, const double *par, int m_dat,
                               const void *data, const double *fvec,
                               int printflags, int iflag, int iter, int nfev),
              int printflags, const void *data);

extern const char *lm_infmsg[];
extern const char *lm_shortmsg[];

void lmcurve_fit(int n_par, double *par, int m_dat, const double *t,
                 const double *y, double (*f)(double t, const double *par),
                 const lm_control_struct *control, lm_status_struct *status);

void lmcurve_fit_rmsd(int n_par, double *par, int m_dat, const double *t,
                      const double *y, double (*f)(double t, const double *par),
                      const lm_control_struct *control,
                      lm_status_struct *status);

extern unsigned int exponential_terms;
extern double minusoneoverfourpisq;
double compute_exponential_f(double t, const double *par);
}
}  // namespace LM

#endif /* US_LM_H */
