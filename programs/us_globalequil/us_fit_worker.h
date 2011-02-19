#ifndef US_EQFWORKER_H
#define US_EQFWORKER_H

#include <QtCore>

#include "us_extern.h"
#include "us_dataIO2.h"
#include "us_matrix.h"
#include "us_math2.h"
#include "us_globeq_data.h"
#include "us_eqmath.h"

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug()
#endif

class US_EXTERN US_FitWorker : public QThread
{
	Q_OBJECT

   public:
      US_FitWorker( US_EqMath*, FitCtrlPar&, QObject* parent );
      ~US_FitWorker();

      void redefine_work( void );     // Redefine work parameters
      void run          ( void );     // Run the thread
      void flag_paused  ( bool );     // Set pause flag true/false
      void flag_abort   ( void );     // Set abort flag true

   signals:
      void work_progress( int  );     // Signal work progress step
      void work_complete( void );     // Signal work complete

   private:
      US_EqMath*  emath;              // EqMath object pointer
      FitCtrlPar& fitpars;            // Fit Control Parameters reference

      int         dbg_level;
      int         mxiters;            // Maximum iterations
      int         k_iter;             // iteration count
      int         nlsmeth;            // NLS method index
      int         modelx;             // model type index
      int         ntpts;              // number of total x,y data points
      int         ndsets;             // number of data sets (scans)
      int         nfpars;             // number of fit parameters per point

      double      tolerance;          // fit variance tolerance
      double      variance;           // variance value (sum diffs. squared)
      double      old_vari;           // previous iteration variance
      double      lambda;             // current lambda value

      bool        paused;             // Flag pause/resume
      bool        abort;              // Flag abort
      bool        aborted;            // Flag fitting aborted
      bool        converged;          // Flag fitting converged
      bool        completed;          // Flag fitting completed

   private slots:
      int    fit_iterations( void );    // Main work method for fit iterations
      int    fit_iter_LM  ( void );     // Fit iteration - Levenberg-Marquardt
      int    fit_iter_MGN ( void );     // Fit iteration - Modified Gauss-Newton
      int    fit_iter_HM  ( void );     // Fit iteration - Hybrid Method
      int    fit_iter_QN  ( void );     // Fit iteration - Quasi-Newton
      int    fit_iter_GLLS( void );     // Fit iteration - Generalized L LS
      int    fit_iter_NNLS( void );     // Fit iteration - NonNegative LS
      void   check_paused ( void );

      double linesearch        ( double*, double  );
      double calc_testParameter( double*, double  );
      void   updateQN          ( double*, double* );
      bool   isNan             ( double );

};
#endif

