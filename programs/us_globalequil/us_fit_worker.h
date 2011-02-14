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

      void redefine_work( void );
      void run          ( void );
      void flag_paused  ( bool );
      void flag_abort   ( void );

   signals:
      void work_progress( int  );
      void work_complete( void );

   private:
      US_EqMath*  emath;              // EqMath object pointer
      FitCtrlPar& fcpars;             // Fit Control Parameters reference

      int         dbg_level;
      int         mxiters;            // Maximum iterations
      int         k_iter;             // iteration count
      int         nlsmeth;            // NLS method index
      int         modelx;             // model type index

      double      tolerance;          // residual tolerance
      double      residual;           // residual value (sum diffs. squared)

      bool        paused;
      bool        abort;

   private slots:
      void   fit_iterations( void );    // Main work method for fit iterations
      void   fit_iters_LM  ( void );    // Fit iterations - Generalized L LS
      void   fit_iters_MGN ( void );    // Fit iterations - Levenverg-Marquardt
      void   fit_iters_HM  ( void );    // Fit iterations - Hybrid Method
      void   fit_iters_QN  ( void );    // Fit iterations - Quasi-Newton
      void   fit_iters_GLLS( void );    // Fit iterations - Generalized L LS
      void   fit_iters_NNLS( void );    // Fit iterations - NonNegative LS
};
#endif

