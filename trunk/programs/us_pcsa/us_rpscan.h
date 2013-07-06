//! \file us_rpscan.h
#ifndef US_RP_SCAN_H
#define US_RP_SCAN_H

#include <QtGui>
#include <QTimer>

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_plot.h"
#include "us_solve_sim.h"
#include "us_plot.h"
#include "us_model_record.h"
#include "us_worker.h"
#include "us_help.h"
#include "qwt_plot_marker.h"

//! \brief A dialog class for Tikhonov regularization parameter scan/plot

class US_RpScan : public US_WidgetsDialog
{
   Q_OBJECT

   public:
      //! \param mr The best model record from initial scan
      US_RpScan( QList< US_SolveSim::DataSet* >&, ModelRecord&, int&,
                 double&, QWidget* p = 0 );

   private:
      QList< US_SolveSim::DataSet* >&  dsets;
      ModelRecord&                     mrec;
      int&                             nthr;
      double&                          alpha;

      QList< WorkerThread* >           wthreads;

      US_Plot*           plotLayout1;

      QwtPlot*           data_plot1;

      QwtPlotCurve*      v_line;
      QwtPlotGrid*       grid;
      QwtPlotMarker*     marker;
      US_PlotPicker*     pick;

      QHBoxLayout*       mainLayout;
      QVBoxLayout*       leftLayout;
      QVBoxLayout*       rightLayout;
      QGridLayout*       pltctrlsLayout;
      QHBoxLayout*       buttonsLayout;

      QLineEdit*         le_mtype;
      QLineEdit*         le_npoints;
      QLineEdit*         le_mdlpar1;
      QLineEdit*         le_mdlpar2;
      QLineEdit*         le_selalpha;
      QLineEdit*         le_stattext;

      QwtCounter*        ct_stralpha;
      QwtCounter*        ct_endalpha;
      QwtCounter*        ct_incalpha;

      int                dbg_level;
      int                nalpha;
      int                nasubm;
      int                nacomp;
      int                lgv;
      int                lgx;

      double             vscl;
      double             xscl;

      QVector< double >  alphas;
      QVector< double >  varias;
      QVector< double >  xnorms;
      QVector< double >  sv_nnls_a;
      QVector< double >  sv_nnls_b;

   protected:
      US_Help       showHelp;
      QProgressBar* b_progress;

   private slots:
      void reject_it  ( void   );
      void accept_it  ( void   );
      void scan       ( void   );
      void plot_data  ( void   );
      void mouse      ( const QwtDoublePoint& );
      void process_job( WorkerThread* );
      void apply_alpha( const double,
                        QVector< double >*, QVector< double >*,
                        const int, const int, const int, double&, double& );

      void help       ( void )
      { showHelp.show_help( "pcsa_rpscan.html" ); };

};
#endif

