#ifndef US_EQFITCTRL_H
#define US_EQFITCTRL_H

#include "us_extern.h"
#include "us_globeq_data.h"
#include "us_eqmath.h"
#include "us_eqreporter.h"
#include "us_fit_worker.h"
#include "us_widgets_dialog.h"
#include "us_dataIO.h"
#include "us_plot.h"
#include "us_help.h"

class US_EqFitControl : public US_WidgetsDialog
{
	Q_OBJECT
	
	public:
		US_EqFitControl( QVector< EqScanFit >&, EqRunFit&,
         US_DataIO::EditedData*, US_EqMath*, US_EqReporter*,
         int, QStringList, bool&, int& );

      void new_scan( int );
      void new_components( void );
      void set_float( bool );

   signals:
      void update_scan( int );

	private:
      QVector< EqScanFit >&   scanfits;  // Scan Fit vector
      EqRunFit&               runfit;    // Run Fit parameters structure
      US_DataIO::EditedData*  edata;     // Edited Data pointer
      US_EqMath*              emath;     // Equil-Math object pointer
      US_EqReporter*          ereporter; // Equil-Reporter object pointer
      int                     modelx;    // Selected model index
      QStringList             models;    // List of model titles
      bool&                   fWidget;   // Fitting Widget created flag
      int&                    selscan;   // Current selected scan

      QLineEdit*         le_iternbr;
      QLineEdit*         le_varianc;
      QLineEdit*         le_stddev;
      QLineEdit*         le_improve;
      QLineEdit*         le_funceva;
      QLineEdit*         le_decompo;
      QLineEdit*         le_clambda;
      QLineEdit*         le_nbrpars;
      QLineEdit*         le_nbrsets;
      QLineEdit*         le_nbrdpts;
      QLineEdit*         le_lamstrt;
      QLineEdit*         le_lamsize;
      QLineEdit*         le_mxiters;
      QLineEdit*         le_fittolr;
      QLineEdit*         le_status;
      QLineEdit*         le_inform;

      QRadioButton*      rb_pltalld;
      QRadioButton*      rb_pltgrp5;
      QRadioButton*      rb_pltsscn;
      QRadioButton*      rb_lincnsn;
      QRadioButton*      rb_lincnsy;
      QRadioButton*      rb_autocnn;
      QRadioButton*      rb_autocny;

      QCheckBox*         ck_monfitg;

      QComboBox*         cb_nlsalgo;

      QPushButton*       pb_strtfit;
      QPushButton*       pb_pause;
      QPushButton*       pb_resume;
      QPushButton*       pb_savefit;
      QPushButton*       pb_viewrep;
      QPushButton*       pb_resids;
      QPushButton*       pb_ovrlays;
      QPushButton*       pb_help;
      QPushButton*       pb_close;
      QPushButton*       pb_lnvsr2;
      QPushButton*       pb_mwvsr2;
      QPushButton*       pb_mwvscv;
      QPushButton*       pb_cmments;

      QProgressBar*      progress;

      QwtCounter*        ct_plotscn;

      US_Plot*           dplot;
      QwtPlot*           data_plot;
      US_FitWorker*      fitwork;
      FitCtrlPar         fitpars;

      US_Help            showHelp;

      //bool               send_signal;

      int                ntpts;           // Total data points
      int                ndsets;          // Total data sets (scans)
      int                nfpars;          // Total fit parameters
      int                nlsmeth;         // NLS method flag
      int                mxiters;         // Maximum fit iterations
      int                mxspts;          // Maximum points in any scan
      int                ipscnn;          // Start plot scan number
      int                lpscnn;          // Last plot scan number
      int                npscns;          // Number of plot scans
      int                plotgrpf;        // Plot group flag: -1, 0, 5
      int                plottype;        // Plot type 0-4: resids, ...

      QVector< int >     v_dscnx;         // Vector of scans' data indecies
      int*               dscnx;           // Array of scans' data indecies

      double             fittoler;
             
   private slots:
      void start_fit     ( void );
      void pause_fit     ( void );
      void resume_fit    ( void );
      void save_fit      ( void );
      void view_report   ( void );
      void plot_residuals( void );
      void plot_overlays ( void );
      void plot_two      ( void );
      void plot_three    ( void );
      void plot_four     ( void );
      void closed        ( void );
      void new_progress  ( int  );
      void fit_completed ( void );
      void prepare_data  ( void );
      void new_pscan     ( void );

      void help    ( void )
      { showHelp.show_help( "global_equil-fitctrl.html" ); };
};
#endif

