//! \file us_fit_worker.cpp

#include "us_fit_worker.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_sleep.h"

// Construct fit worker thread
US_FitWorker::US_FitWorker( US_EqMath* emath, FitCtrlPar& fitpars,
      QObject* parent )
 : QThread( parent  ),
   emath  ( emath   ),
   fitpars( fitpars )
{
   dbg_level    = US_Settings::us_debug();
   abort        = false;
   paused       = false;

   redefine_work();
}

// Destroy fit worker thread
US_FitWorker::~US_FitWorker()
{
   wait();
}

// Redefine work
void US_FitWorker::redefine_work( )
{
   mxiters   = fitpars.mxiters;
   tolerance = fitpars.fittoler;
   k_iter    = 0;
   nlsmeth   = fitpars.nlsmeth;
   modelx    = fitpars.modelx;
   lambda    = fitpars.lam_start;
   ntpts     = fitpars.ntpts;
   ndsets    = fitpars.ndsets;
   nfpars    = fitpars.nfpars;

   fitpars.k_iter    = 0;
   fitpars.k_step    = 0;
   fitpars.ndecomps  = 0;
   fitpars.nfuncev   = 0;
   fitpars.variance  = 0.0;
   fitpars.std_dev   = 0.0;
   fitpars.improve   = 0.0;
   fitpars.lambda    = lambda;
   fitpars.completed = false;
   fitpars.converged = false;
   fitpars.aborted   = false;
}

// Run fit iterations
void US_FitWorker::run()
{
   fit_iterations();        // do all the work here

   quit();
   exec();

   emit work_complete();    // signal that the thread's work is done
}

// Flag that work should be paused or resumed
void US_FitWorker::flag_paused( bool pauseit )
{
   paused      = pauseit;
}

// Flag that work should be aborted
void US_FitWorker::flag_abort()
{
   abort       = true;
}

// Private slot that does the work:  run fit iterations
int US_FitWorker::fit_iterations()
{
   int    stati   = 0;
   int    stats   = 0;
   variance       = tolerance * 2.0;
   k_iter         = 0;
   double ptscale = 1.0 / (double)fitpars.ntpts;
DbgLv(1) << "FWk: fit_iterations";
DbgLv(1) << "FWk:  mxiters" << mxiters << " tolerance" << tolerance;
   stati          = emath->calc_model( fitpars.guess );
   variance       = emath->calc_residuals() * ptscale;
   old_vari       = variance;

   // Iterate fitting for max iter count or until convergence
   while( k_iter < mxiters  &&  variance > tolerance )
   {
      stati = 0;

      // Do an interation with a selected method
      switch( nlsmeth )
      {
         case 0:             // Levenberg-Marquardt
            stati = fit_iter_LM();
            break;
         case 1:             // Modified Gauss-Newton
            stati = fit_iter_MGN();
            break;
         case 2:             // Hybrid Method
            stati = fit_iter_HM();
            break;
         case 3:             // Quasi-Newton
            stati = fit_iter_QN();
            break;
         case 4:             // Generalized Linear LS
            stati = fit_iter_GLLS();
            break;
         case 5:             // NonNegative LS
            stati = fit_iter_NNLS();
            break;
      }

      k_iter++;
      variance = emath->calc_residuals() * ptscale;

      fitpars.k_iter    = k_iter;
      fitpars.variance  = variance;
      fitpars.std_dev   = sqrt( variance );
      fitpars.improve   = variance - old_vari;
      fitpars.lambda    = lambda;
      old_vari          = variance;
      fitpars.completed = ( k_iter >= mxiters );

      emit work_progress( fitpars.k_step );

      if ( abort )   fitpars.aborted = true;

      if ( fitpars.converged  ||  fitpars.completed  ||  fitpars.aborted )
         break;

      check_paused();
   }

   return stats;
}

// Private slot to run an iteration:  Levenberg-Marquardt
int US_FitWorker::fit_iter_LM()
{
   int    stati    = 0;
   int    lamloop  = 0;
   double ptscale  = 1.0 / (double)fitpars.ntpts;
   double old_vari = variance;
   double new_vari = variance;
   double dd_toler = tolerance * 0.0001;

   // Create the jacobian matrix ( points rows by parameters columns )
   emath->calc_jacobian();

   // Get the parameters by parameters info matrix:  J' * J
   emath->calc_A_transpose_A( fitpars.jacobian, fitpars.info, ntpts, nfpars );

   // Add Lambda to the info matrix diagonal
   for ( int ii = 0; ii < nfpars; ii++ )
      fitpars.info[ ii ][ ii ] += lambda;

   // Compute the B vector:  Jacobian times yDelta vector
   emath->calc_B();

   // LLtranspose matrix is a copy of the info matrix
   for ( int ii = 0; ii < nfpars; ii++ )
      for ( int jj = 0; jj < nfpars; jj++ )
         fitpars.LLtr[ ii ][ jj ] = fitpars.info[ ii ][ jj ];

   fitpars.ndecomps++;
   check_paused();

   // Cholesky decomposition of the LLtr matrix
   stati = US_Matrix::Cholesky_Decomposition( fitpars.LLtr, nfpars )
      ? 0 : -2;
   check_paused();

   // Cholesky Solve:  replace B vector
   stati = US_Matrix::Cholesky_SolveSystem( fitpars.LLtr, fitpars.BB, nfpars )
      ? 0 : -3;
   check_paused();
DbgLv(1) << "FWk:   ChoSS stati" << stati << " LLtr0" << fitpars.LLtr[0][0]
 << "lambda" << lambda << fitpars.lambda
 << "lam start,step" << fitpars.lam_start << fitpars.lam_step;

   // Test guess vector is original guess + B vector
   for ( int ii = 0; ii < nfpars; ii++ )
      fitpars.tguess[ ii ] = fitpars.guess[ ii ] + fitpars.BB[ ii ];
DbgLv(1) << "FWk:   ii tguess guess BB" << 0
 << fitpars.tguess[ 0 ] << fitpars.guess[ 0 ] << fitpars.BB[ 0 ];
DbgLv(1) << "FWk:   ii tguess guess BB" << 1
 << fitpars.tguess[ 1 ] << fitpars.guess[ 1 ] << fitpars.BB[ 1 ];

   // Y-Guess is modified based on test-guess
   stati = emath->calc_model( fitpars.tguess );
   check_paused();

   old_vari  = new_vari;
   // Compute new Y-Delta and determine variance
   new_vari  = emath->calc_residuals() * ptscale;
   double dd = old_vari - new_vari;
   if ( qAbs( dd ) < dd_toler )
      old_vari  = new_vari;

DbgLv(1) << "FWk:    old,new vari" << old_vari << new_vari << dd << dd_toler;

   // Modify guess based on direction of iteration variance change
   if ( new_vari < old_vari )
   {  // Reduction:  guess is test-guess; Lambda reduced
      lambda /= fitpars.lam_step;

      for ( int ii = 0; ii < nfpars; ii++ )
         fitpars.guess[ ii ] = fitpars.tguess[ ii ];
   }

   else if ( new_vari == old_vari )
   {  // No change:  convergence!
      fitpars.converged = true;
   }

   else    // new_vari > old_vari
   {  // Greater variance:  increase Lambda
      for ( int ii = 0; ii < nfpars; ii++ )
         fitpars.info[ ii ][ ii ] -= lambda;

      lamloop++;
      lambda *= pow( (double)fitpars.lam_step, (double)lamloop );

      if ( lambda < 1.0e10 )
      {
         for ( int ii = 0; ii < nfpars; ii++ )
            fitpars.info[ ii ][ ii ] += lambda;
      }

      else
      {
         lambda  = 1.0e6;
         fitpars.converged = true;
      }

      // Y-guess based on current Guess vector
      stati = emath->calc_model( fitpars.guess );
   }

   // Bump step count and compute iteration's variance value
   fitpars.k_step++;
   variance = emath->calc_residuals() * ptscale;

   return stati;
}

// Private slot to run an iteration:  Modified Gauss-Newton
int US_FitWorker::fit_iter_MGN()
{
   int stati = 0;

   return stati;
}

// Private slot to run an iteration:  Hybrid Method
int US_FitWorker::fit_iter_HM()
{
   int stati = 0;

   return stati;
}

// Private slot to run an iteration:  Quasi-Newton
int US_FitWorker::fit_iter_QN()
{
   int stati = 0;

   return stati;
}

// Private slot to run an iteration:  Generalized Linear Least Squares
int US_FitWorker::fit_iter_GLLS()
{
   int stati = 0;

   return stati;
}

// Private slot to run an iteration:  NonNegative Least Squares
int US_FitWorker::fit_iter_NNLS()
{
   int stati = 0;

   return stati;
}


// Private slot to block/resume with paused flag set
void US_FitWorker::check_paused()
{
   while ( paused )
   {
      US_Sleep::msleep( 10 );
      qApp->processEvents();
   }
}

