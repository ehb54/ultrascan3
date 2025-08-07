//! \file us_fit_worker.cpp

#include "us_fit_worker.h"
#include <cfloat>
#include "us_gui_settings.h"
#include "us_settings.h"
#include "us_sleep.h"

const double dflt_min = ( double ) FLT_MIN;
const double dflt_max = ( double ) FLT_MAX;

// Construct fit worker thread
US_FitWorker::US_FitWorker(US_EqMath *emath, FitCtrlPar &fitpars, QObject *parent) :
    QThread(parent), emath(emath), fitpars(fitpars) {
   dbg_level = US_Settings::us_debug();
   abort = false;
   paused = false;

   redefine_work();
}

// Destroy fit worker thread
US_FitWorker::~US_FitWorker() {
   wait();
}

// Redefine work
void US_FitWorker::redefine_work() {
   mxiters = fitpars.mxiters;
   tolerance = fitpars.fittoler;
   k_iter = 0;
   nlsmeth = fitpars.nlsmeth;
   modelx = fitpars.modelx;
   lambda = fitpars.lam_start;
   ntpts = fitpars.ntpts;
   ndsets = fitpars.ndsets;
   nfpars = fitpars.nfpars;

   fitpars.k_iter = 0;
   fitpars.k_step = 0;
   fitpars.ndecomps = 0;
   fitpars.nfuncev = 0;
   fitpars.variance = 0.0;
   fitpars.std_dev = 0.0;
   fitpars.improve = 0.0;
   DbgLv(1) << "Define improve=0";
   fitpars.lambda = lambda;
   fitpars.completed = false;
   fitpars.converged = false;
   fitpars.aborted = false;
   fitpars.infomsg = "???";
}

// Run fit iterations
void US_FitWorker::run() {
   fit_iterations(); // do all the work here

   quit();
   exec();

   emit work_complete(); // signal that the thread's work is done
}

// Flag that work should be paused or resumed
void US_FitWorker::flag_paused(bool pauseit) {
   paused = pauseit;
}

// Flag that work should be aborted
void US_FitWorker::flag_abort() {
   abort = true;
}

// Do the work of the thread:  run fit iterations
int US_FitWorker::fit_iterations() {
   int stati = 0;
   int stats = 0;
   bool autocnvg = fitpars.autocnvg;
   double fltolr = tolerance * 1e-6;

   // With autoconverge, start method is either Lev.-Marquardt or Quasi-Newton
   nlsmeth = autocnvg ? ((nlsmeth == 0) ? 0 : 3) : nlsmeth;
   variance = tolerance * 2.0;
   k_iter = 0;
   DbgLv(1) << "FWk: fit_iterations";
   DbgLv(1) << "FWk:  mxiters" << mxiters << " tolerance" << tolerance;
   stati = emath->calc_model(fitpars.guess);
   variance = emath->calc_residuals();

   // Iterate fitting for max iter count or until convergence
   while (k_iter < mxiters && variance > tolerance) {
      stati = 0;
      old_vari = variance;

      // Do an interation with a selected method
      switch (nlsmeth) {
         case 0: // Levenberg-Marquardt
            stati = fit_iter_LM();
            break;
         case 1: // Modified Gauss-Newton
            stati = fit_iter_MGN();
            break;
         case 2: // Hybrid Method
            stati = fit_iter_HM();
            break;
         case 3: // Quasi-Newton
            stati = fit_iter_QN();
            break;
         case 4: // Generalized Linear LS
            stati = fit_iter_GLLS();
            break;
         case 5: // NonNegative LS
            stati = fit_iter_NNLS();
            break;
      }

      fitpars.k_iter = ++k_iter;
      fitpars.variance = variance;
      fitpars.std_dev = (variance > 0.0) ? sqrt(variance) : variance;
      fitpars.improve = variance - old_vari;
      DbgLv(1) << "EOI improve v ov" << fitpars.improve << variance << old_vari;
      fitpars.lambda = lambda;
      fitpars.completed = (k_iter >= mxiters);

      if (stati != 0) {
         fitpars.aborted = true;
         stats = stati;
      }

      emit work_progress(fitpars.k_step);

      if (abort) {
         fitpars.aborted = true;
         fitpars.infomsg = "User Aborted";
      }

      if (fitpars.converged || fitpars.completed || fitpars.aborted) {
         if (fitpars.completed)
            fitpars.infomsg = tr("%1 iterations reached").arg(mxiters);

         break;
      }

      check_paused();

      // With autoconverge, flip-flop between Lev.-Marquardt and Quasi-Newton
      nlsmeth = (autocnvg && qAbs(fitpars.improve) < fltolr && (nlsmeth == 0 || nlsmeth == 3)) ? (3 - nlsmeth)
                                                                                               : nlsmeth;
      DbgLv(1) << "EOI  autocnvg" << autocnvg << " nlsmeth ftol" << nlsmeth << fltolr;
   }

   return stats;
}

// Run an iteration:  Levenberg-Marquardt
int US_FitWorker::fit_iter_LM() {
   int stati = 0;
   int lamloop = 0;
   double old_vari = variance;
   double dv_toler = tolerance * 1e-12;

   // Create the jacobian matrix ( points rows by parameters columns )
   emath->calc_jacobian();

   // Get the (parameters by parameters) info matrix:  J' * J
   US_Matrix::tmm(fitpars.jacobian, fitpars.info, ntpts, nfpars);

   // Add Lambda to the info matrix diagonal
   for (int ii = 0; ii < nfpars; ii++)
      fitpars.info[ ii ][ ii ] += lambda;

   // Compute the B vector:  Jacobian times yDelta vector
   emath->calc_B();

   // LLtranspose matrix is a copy of the info matrix
   US_Matrix::mcopy(fitpars.info, fitpars.LLtr, nfpars, nfpars);

   fitpars.ndecomps++;
   check_paused();

   // Cholesky decomposition of the LLtr matrix
   stati = US_Matrix::Cholesky_Decomposition(fitpars.LLtr, nfpars) ? 0 : -2;
   check_paused();

   // Cholesky Solve:  replace B vector
   stati = US_Matrix::Cholesky_SolveSystem(fitpars.LLtr, fitpars.BB, nfpars) ? 0 : -3;
   check_paused();
   DbgLv(1) << "FWk:   ChoSS stati" << stati << " LLtr0" << fitpars.LLtr[ 0 ][ 0 ] << "lambda" << lambda
            << fitpars.lambda << "lam start,step" << fitpars.lam_start << fitpars.lam_step;

   // Test guess vector is original guess + B vector
   US_Matrix::vsum(fitpars.guess, fitpars.BB, fitpars.tguess, nfpars);
   DbgLv(1) << "FWk:   ii tguess guess BB" << 0 << fitpars.tguess[ 0 ] << fitpars.guess[ 0 ] << fitpars.BB[ 0 ];
   DbgLv(1) << "FWk:   ii tguess guess BB" << 1 << fitpars.tguess[ 1 ] << fitpars.guess[ 1 ] << fitpars.BB[ 1 ];

   // Y-Guess is modified based on test-guess
   stati = emath->calc_model(fitpars.tguess);
   check_paused();

   // Compute new Y-Delta and determine variance
   variance = emath->calc_residuals();

   if (variance < 0.0) {
      DbgLv(1) << "FWk: *** VARIANCE<0 ***" << variance;
      double dlt1 = qAbs(fitpars.y_delta[ 0 ]);
      double dltn = qAbs(fitpars.y_delta[ ntpts - 1 ]);
      int lgdl = qRound(log10(max(dlt1, dltn)));
      double dlmx = pow(10.0, lgdl);
      fitpars.infomsg = tr("Huge Variance w/ ydelta max %1").arg(dlmx);
      DbgLv(1) << "FWk: *** VARI<0: lgd dlm dl1 dln" << lgdl << dlmx << dlt1 << dltn;
      return -101;
   }

   double dv = variance - old_vari;
   if (qAbs(dv) < dv_toler)
      old_vari = variance;

   DbgLv(1) << "FWk:    old,new vari" << old_vari << variance << dv << dv_toler;

   // Modify guess based on direction of iteration variance change
   if (variance < old_vari) { // Reduction:  guess is test-guess; Lambda reduced
      lambda /= fitpars.lam_step;

      US_Matrix::vcopy(fitpars.tguess, fitpars.guess, nfpars);
   }

   else if (variance == old_vari) { // No change:  convergence!
      fitpars.converged = true;
      fitpars.improve = dv;
      DbgLv(1) << "CNV improve nv ov" << fitpars.improve << variance << old_vari;
      fitpars.infomsg = tr("Variance change ") + ((dv == 0.0) ? tr("zero") : tr("near zero"));
   }

   else // variance > old_vari
   { // Greater variance:  increase Lambda
      US_Matrix::add_diag(fitpars.info, (-lambda), nfpars);

      lamloop++;
      lambda *= pow(( double ) fitpars.lam_step, ( double ) lamloop);

      if (lambda < 1.0e10) {
         US_Matrix::add_diag(fitpars.info, lambda, nfpars);
         for (int ii = 0; ii < nfpars; ii++)
            fitpars.info[ ii ][ ii ] += lambda;
      }

      else {
         lambda = 1.0e6;
         fitpars.converged = true;
         fitpars.infomsg = "Lambda large";
      }

      // Y-guess based on current Guess vector
      stati = emath->calc_model(fitpars.guess);
   }

   // Bump step count
   fitpars.k_step++;

   return stati;
}

// Run an iteration:  Modified Gauss-Newton
int US_FitWorker::fit_iter_MGN() {
   int stati = 0;

   return stati;
}

// Run an iteration:  Hybrid Method
int US_FitWorker::fit_iter_HM() {
   int stati = 0;

   return stati;
}

// Run an iteration:  Quasi-Newton
int US_FitWorker::fit_iter_QN() {
   int stati = 0;
   QVector<double> vsearch(nfpars);
   QVector<double> vgamma(nfpars);
   QVector<double> vdelta(nfpars);
   double *search = vsearch.data();
   double *gamma = vgamma.data();
   double *delta = vdelta.data();

   if (k_iter == 0) {
      QVector<double *> vminf;
      QVector<double> vdinf;
      double **wminf = US_Matrix::construct(vminf, vdinf, nfpars, nfpars);

      // Set up the Hessian matrix, initialized to an identity matrix
      US_Matrix::mident(fitpars.info, nfpars);

      // Create the (points x parameters) jacobian matrix
      DbgLv(1) << "FW:QN: calc_jac";
      emath->calc_jacobian();

      // Get the (parameters x parameters) info matrix:  J' * J
      DbgLv(1) << "FW:QN: calc_AtA";
      US_Matrix::tmm(fitpars.jacobian, fitpars.info, ntpts, nfpars);

      // Compute the B vector:  Jacobian-transpose times yDelta vector:  J' * d
      emath->calc_B();

      // Create a work matrix that is a copy of the info matrix
      US_Matrix::mcopy(fitpars.info, wminf, nfpars, nfpars);

      // Cholesky Invert from info to LL-transpose
      DbgLv(1) << "FW:QN: ChoInv";
      US_Matrix::Cholesky_Invert(wminf, fitpars.LLtr, nfpars);

      // Copy LL-transpose to info matrix
      US_Matrix::mcopy(fitpars.LLtr, fitpars.info, nfpars, nfpars);
      DbgLv(1) << "FW:QN: End iter0";

      variance = emath->calc_residuals();
   }

   check_paused();

   // Get a test variance value from B array; return now if convergence
   double test_vari = US_Matrix::dotproduct(fitpars.BB, nfpars);

   test_vari = (test_vari > 0.0) ? sqrt(test_vari) : test_vari;

   DbgLv(1) << "FW:QN:  test_vari" << test_vari;
   if (test_vari > 0.0 && sqrt(test_vari) < tolerance)
      stati = 0;

   else {
      // Build search array ( info * B ) and gamma ( B copy )
      US_Matrix::mvv(fitpars.info, fitpars.BB, search, nfpars, nfpars);

      US_Matrix::vcopy(fitpars.BB, gamma, nfpars);
      DbgLv(1) << "FW:QN:  search0 gamma0" << search[ 0 ] << gamma[ 0 ];

      double resids = variance * ( double ) nfpars;
      double alpha = linesearch(search, resids);
      DbgLv(1) << "FW:QN:  linesearch  return - alpha" << alpha;

      if (alpha == 0.0)
         stati = 0;

      if (alpha < 0.0) {
         stati = -410;
      }

      else {
         for (int ii = 0; ii < nfpars; ii++)
            fitpars.guess[ ii ] += (search[ ii ] * alpha);

         // Update solution, needed to calculate y_delta
         stati = emath->calc_model(fitpars.guess);
         check_paused();
         DbgLv(1) << "FW:QN:  calc_model return";

         // Calculate the Jacobian matrix
         emath->calc_jacobian();
         check_paused();
         DbgLv(1) << "FW:QN:  calc_jac return";

         // Compute new Y-Delta and determine variance
         variance = emath->calc_residuals();

         // Compute the B vector:  Jacobian-transpose times yDelta vector
         emath->calc_B();
         check_paused();
         DbgLv(1) << "FW:QN:  calc_B return";

         // Update gamma and delta arrays
         for (int ii = 0; ii < nfpars; ii++) {
            gamma[ ii ] -= fitpars.BB[ ii ];
            delta[ ii ] = search[ ii ] * alpha;
         }

         // Update Quasi-Newton
         updateQN(gamma, delta);
         DbgLv(1) << "FW:QN:  updateQN  return";
      } // END:  alpha > 0.0
   } // END:  test_vari >= tolerance

   // Bump step count
   fitpars.k_step++;

   return stati;
}

// Run an iteration:  Generalized Linear Least Squares
int US_FitWorker::fit_iter_GLLS() {
   int stati = 0;

   return stati;
}

// Run an iteration:  NonNegative Least Squares
int US_FitWorker::fit_iter_NNLS() {
   int stati = 0;

   return stati;
}


// Block/Resume based on paused flag setting
void US_FitWorker::check_paused() {
   while (paused) { // If paused, loop to sleep and re-check
      US_Sleep::msleep(10);
      qApp->processEvents();
   }
}

// Line search
double US_FitWorker::linesearch(double *search, double f0) {
   double alpha = 0.0;

   double x0 = 0.0;
   double x1 = 0.5;
   double x2 = 1.0;
   double old_f0 = 0.0;
   double old_f1 = 0.0;
   double old_f2 = 0.0;
   double hh = 0.1;
   int iter = 0;

   double f1 = calc_testParameter(search, x1);
   if (f1 < 0.0)
      return (0.0);

   double f2 = calc_testParameter(search, x2);
   if (f2 < 0.0)
      return (0.0);

   while (f0 >= 10000.0 || f0 < 0.0 || f1 >= 10000.0 || f1 < 0.0 || f2 >= 10000.0 || f2 < 0.0) {
      x1 /= 10.0;
      x2 /= 10.0;

      f1 = calc_testParameter(search, x1);
      if (f1 < 0.0)
         return (0.0);

      f2 = calc_testParameter(search, x2);
      if (f2 < 0.0)
         return (0.0);

      if (x1 < dflt_min)
         return (-1.0);
   }

   bool check_flag = true;

   while (check_flag) {
      if ((isNan(f0) && isNan(f1)) || (isNan(f1) && isNan(f2)) || (isNan(f0) && isNan(f2)))
         return (-2.0);

      if ((qAbs(f2 - old_f2) < dflt_min) && (qAbs(f1 - old_f1) < dflt_min) && (qAbs(f0 - old_f0) < dflt_min))
         return (0.0);

      old_f0 = f0;
      old_f1 = f1;
      old_f2 = f2;

      if (((qAbs(f2 - f0) < dflt_min) && (qAbs(f1 - f0) < dflt_min)) || ((qAbs(f2 - f1) < dflt_min) && f0 > f1))
         return (0.0);

      if ((qAbs(x0) < dflt_min) && (qAbs(x1) < dflt_min) && (qAbs(x2) < dflt_min))
         return (0.0);

      if (((qAbs(f0 - f1) < dflt_min) && (qAbs(f1 - f2) < dflt_min)) || ((qAbs(f0 - f1) < dflt_min) && f2 > f1))
         return (0.0);

      if (f0 > f1 && f2 > f1) {
         check_flag = false;
         break;
      }

      else if ((f2 > f1 && f1 > f0) || (f1 > f0 && f1 > f2) || (f1 == f1 && f1 > f0)) { // Shift left
         x2 = x1;
         f2 = f1;
         x1 = (x0 + x2) * 0.5;

         f1 = calc_testParameter(search, x1);
         if (f1 < 0.0)
            return (0.0);
      }

      else if (f0 > f1 && f1 > f2) { // Shift right
         x0 = x1;
         f0 = f1;
         x1 = x2;
         f1 = f2;
         x2 = x2 + (pow(2.0, ( double ) (iter + 2)) * hh);

         f2 = calc_testParameter(search, x2);
         if (f2 < 0.0)
            return (0.0);
      }

      iter++;
   }

   x1 = (x0 + x2) * 0.5;
   hh = x1 - x0;

   f1 = calc_testParameter(search, x1);
   if (f1 < 0.0)
      return (0.0);

   while (true) {
      if (f0 < f1) { // Shift left
         x2 = x1;
         f2 = f1;
         x1 = x0;
         f1 = f0;
         x0 = x1 - hh;

         f0 = calc_testParameter(search, x0);
         if (f0 < 0.0)
            break;
      }

      if (f2 < f1) { // Shift right
         x0 = x1;
         f0 = f1;
         x1 = x2;
         f1 = f2;
         x2 = x1 + hh;

         f2 = calc_testParameter(search, x2);
         if (f2 < 0.0)
            break;
      }

      if (qAbs(f0 - f1 * 2.0 + f2) < dflt_min)
         break;
      double xmin = x1 + (hh * (f0 - f2)) / (2. * (f0 - f1 * 2. + f2));
      double fmin = calc_testParameter(search, xmin);
      if (fmin < 0.0)
         break;

      if (fmin < f1) {
         x1 = xmin;
         f1 = fmin;
      }

      hh /= 2.0;
      if (hh < tolerance) {
         alpha = x1;
         break;
      }

      x0 = x1 - hh;
      x2 = x1 + hh;

      f0 = calc_testParameter(search, x0);
      if (f0 < 0.0)
         break;
      f2 = calc_testParameter(search, x2);
      if (f2 < 0.0)
         break;
   }

   return alpha;
}

// Calculate test parameter
double US_FitWorker::calc_testParameter(double *search, double step) {
   double resid = 0.0;

   for (int ii = 0; ii < nfpars; ii++)
      fitpars.tguess[ ii ] = fitpars.guess[ ii ] + search[ ii ] * step;

   if (emath->calc_model(fitpars.tguess) < 0.0) {
      for (int ii = 0; ii < nfpars; ii++)
         fitpars.tguess[ ii ] = fitpars.guess[ ii ];

      emath->calc_model(fitpars.guess);
   }

   resid = emath->calc_residuals();

   return resid;
}

// Update information matrix for Quasi-Newton
void US_FitWorker::updateQN(double *gamma, double *delta) {
   QVector<double> vhgamma(nfpars);
   QVector<double> vvv(nfpars);
   QVector<double *> mvvtrns(nfpars);
   QVector<double *> mhgtrns(nfpars);
   QVector<double *> mdatrns(nfpars);
   QVector<double> dvvtrns(nfpars * nfpars);
   QVector<double> dhgtrns(nfpars * nfpars);
   QVector<double> ddatrns(nfpars * nfpars);
   double *hgamma = vhgamma.data();
   double *vv = vvv.data();

   // Construct the work matrices,  all parameters x parameters
   double **vvtrns = US_Matrix::construct(mvvtrns, dvvtrns, nfpars, nfpars);
   double **hgtrns = US_Matrix::construct(mhgtrns, dhgtrns, nfpars, nfpars);
   double **datrns = US_Matrix::construct(mdatrns, ddatrns, nfpars, nfpars);

   // Compute the h-gamma array:  info-matrix times gamma
   US_Matrix::mvv(fitpars.info, gamma, hgamma, nfpars, nfpars);

   // Compute scalars:  lambda = gamma dot hgamma;  deltgam = delta dot gamma
   double lambda = US_Matrix::dotproduct(gamma, hgamma, nfpars);
   double deltgam = US_Matrix::dotproduct(delta, gamma, nfpars);

   // Compute vv array:  scaled delta minus scaled hgamma
   for (int ii = 0; ii < nfpars; ii++)
      vv[ ii ] = (delta[ ii ] / deltgam) - (hgamma[ ii ] / lambda);

   // Compute the 3 transpose matrices, each Mij = Vi1 * V1j
   for (int ii = 0; ii < nfpars; ii++) {
      double rowvv = vv[ ii ];
      double rowda = delta[ ii ];
      double rowhg = hgamma[ ii ];

      for (int jj = 0; jj < nfpars; jj++) {
         vvtrns[ ii ][ jj ] = vv[ jj ] * rowvv;
         datrns[ ii ][ jj ] = delta[ jj ] * rowda;
         hgtrns[ ii ][ jj ] = hgamma[ jj ] * rowhg;
      }
   }

   // Update the info matrix by add/subtract of 3 scaled transpose matrices
   for (int ii = 0; ii < nfpars; ii++)
      for (int jj = 0; jj < nfpars; jj++)
         fitpars.info[ ii ][ jj ] = fitpars.info[ ii ][ jj ] - (hgtrns[ ii ][ jj ] / lambda)
                                    + (datrns[ ii ][ jj ] / deltgam) + (vvtrns[ ii ][ jj ] * lambda);
}


// Convenience function:  is a double value NAN (Not A valid Number)?
bool US_FitWorker::isNan(double value) {
   if (value != value)
      return true; // NAN:  one case where val!=val
   double aval = qAbs(value); // mark NAN if beyond float range
   return (aval < dflt_min || aval > dflt_max);
}
