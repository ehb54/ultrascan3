//! \file us_fit_worker.cpp

#include "us_fit_worker.h"
#include "us_settings.h"
#include "us_gui_settings.h"

// Construct fit worker thread
US_FitWorker::US_FitWorker( US_EqMath* emath, FitCtrlPar& fcpars,
      QObject* parent )
 : QThread( parent ),
   emath      ( emath ),
   fcpars     ( fcpars )
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
   mxiters   = fcpars.mxiters;
   tolerance = fcpars.fittoler;
   k_iter    = 0;
   nlsmeth   = fcpars.nlsmeth;
   modelx    = fcpars.modelx;
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
void US_FitWorker::fit_iterations()
{
DbgLv(1) << "FWk: fit_iterations";
DbgLv(1) << "FWk:  mxiters" << fcpars.mxiters;
}

// Private slot to run iterations:  Levenberg-Marquardt
void US_FitWorker::fit_iters_LM()
{
}

// Private slot to run iterations:  Modified Gauss-Newton
void US_FitWorker::fit_iters_MGN()
{
}

// Private slot to run iterations:  Hybrid Method
void US_FitWorker::fit_iters_HM()
{
}

// Private slot to run iterations:  Quasi-Newton
void US_FitWorker::fit_iters_QN()
{
}

// Private slot to run iterations:  Generalized Linear Least Squares
void US_FitWorker::fit_iters_GLLS()
{
}

// Private slot to run iterations:  NonNegative Least Squares
void US_FitWorker::fit_iters_NNLS()
{
}


