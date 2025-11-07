//! \file us_eqfit_control.cpp

#include "us_eqfit_control.h"
#include "us_fit_worker.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_gui_util.h"
#include "us_constants.h"
#include "us_math2.h"
#include "qwt_plot_marker.h"

// Main constructor with references to parameters from main GlobalEquil class
US_EqFitControl::US_EqFitControl(
      QVector< EqScanFit >&   a_scanfits,
      EqRunFit&               a_runfit,
      US_DataIO::EditedData*  a_edata,
      US_EqMath*              a_emath,
      US_EqReporter*          a_ereporter,
      int                     a_modelx,
      QStringList             a_models,
      bool&                   a_fWidget,
      int&                    a_selscan )
 : US_WidgetsDialog( nullptr, Qt::WindowFlags() ),
   scanfits   ( a_scanfits ),
   runfit     ( a_runfit ),
   edata      ( a_edata ),
   emath      ( a_emath ),
   ereporter  ( a_ereporter ),
   modelx     ( a_modelx ),
   models     ( a_models ),
   fWidget    ( a_fWidget ),
   selscan    ( a_selscan )
{
qDebug() << "EFC: IN";
   setAttribute  ( Qt::WA_DeleteOnClose );
   setWindowTitle( tr( "Equilibrium Fitting Control Window" ) );
   setPalette    ( US_GuiSettings::frameColor() );
   setMinimumSize( 200, 100 );

   // Main layout
   QBoxLayout* main  = new QHBoxLayout( this );
   main->setContentsMargins( 2, 2, 2, 2 );
   main->setSpacing        ( 2 );

   // Two primary layouts
   QBoxLayout*  lsideLayout  = new QVBoxLayout;
   QBoxLayout*  rsideLayout  = new QVBoxLayout;

   // Primary left-side layouts
   QGridLayout* eqfitLayout  = new QGridLayout;
   QGridLayout* gplotLayout  = new QGridLayout;
   QGridLayout* ftuneLayout  = new QGridLayout;

   // Primary right-side layouts
   dplot = new US_Plot( data_plot,
         tr( "Residuals" ),
         tr( "Radius^2 - Radius(ref)^2 (cm)" ),
         tr( "Optical Density Difference" ) );
   QGridLayout* statsLayout  = new QGridLayout;

   // Fitting Control layout
   QLabel*  lb_fbanner  = us_banner(
         tr( "Nonlinear Least Squares\n"
             "Global Equilibrium Fitting Control" ) );
   QLabel*  lb_iternbr  = us_label( tr( "Iteration Number:" ) );
            le_iternbr  = us_lineedit( "0" );
            pb_strtfit  = us_pushbutton( tr( "Fit" ) );
   QLabel*  lb_varianc  = us_label( tr( "Variance:" ) );
            le_varianc  = us_lineedit( "0" );
            pb_pause    = us_pushbutton( tr( "Pause" ) );
   QLabel*  lb_stddev   = us_label( tr( "Std.Deviation:" ) );
            le_stddev   = us_lineedit( "0" );
            pb_resume   = us_pushbutton( tr( "Resume" ) );
   QLabel*  lb_improve  = us_label( tr( "Improvement:" ) );
            le_improve  = us_lineedit( "0" );
            pb_savefit  = us_pushbutton( tr( "Save Fit" ) );
   QLabel*  lb_funceva  = us_label( tr( "Function Evaluations:" ) );
            le_funceva  = us_lineedit( "0" );
            pb_viewrep  = us_pushbutton( tr( "View Report" ) );
   QLabel*  lb_decompo  = us_label( tr( "Decompositions:" ) );
            le_decompo  = us_lineedit( "0" );
            pb_resids   = us_pushbutton( tr( "Residuals" ) );
   QLabel*  lb_clambda  = us_label( tr( "Current Lambda:" ) );
            le_clambda  = us_lineedit( "10000" );
            pb_ovrlays  = us_pushbutton( tr( "Overlays" ) );
   QLabel*  lb_nbrpars  = us_label( tr( "# of Parameters:" ) );
            le_nbrpars  = us_lineedit( "0" );
   QLabel*  lb_nbrsets  = us_label( tr( "# of Datasets in Fit:" ) );
            le_nbrsets  = us_lineedit( "0" );
            pb_help     = us_pushbutton( tr( "Help" ) );
   QLabel*  lb_nbrdpts  = us_label( tr( "# of Datapoints in Fit:" ) );
            le_nbrdpts  = us_lineedit( "0" );
            pb_close    = us_pushbutton( tr( "Close" ) );

   int row = 0;
   eqfitLayout->addWidget( lb_fbanner,  row,   0, 2, 6 );
   row    += 2;
   eqfitLayout->addWidget( lb_iternbr,  row,   0, 1, 2 );
   eqfitLayout->addWidget( le_iternbr,  row,   2, 1, 2 );
   eqfitLayout->addWidget( pb_strtfit,  row++, 4, 1, 2 );
   eqfitLayout->addWidget( lb_varianc,  row,   0, 1, 2 );
   eqfitLayout->addWidget( le_varianc,  row,   2, 1, 2 );
   eqfitLayout->addWidget( pb_pause,    row++, 4, 1, 2 );
   eqfitLayout->addWidget( lb_stddev,   row,   0, 1, 2 );
   eqfitLayout->addWidget( le_stddev,   row,   2, 1, 2 );
   eqfitLayout->addWidget( pb_resume,   row++, 4, 1, 2 );
   eqfitLayout->addWidget( lb_improve,  row,   0, 1, 2 );
   eqfitLayout->addWidget( le_improve,  row,   2, 1, 2 );
   eqfitLayout->addWidget( pb_savefit,  row++, 4, 1, 2 );
   eqfitLayout->addWidget( lb_funceva,  row,   0, 1, 2 );
   eqfitLayout->addWidget( le_funceva,  row,   2, 1, 2 );
   eqfitLayout->addWidget( pb_viewrep,  row++, 4, 1, 2 );
   eqfitLayout->addWidget( lb_decompo,  row,   0, 1, 2 );
   eqfitLayout->addWidget( le_decompo,  row,   2, 1, 2 );
   eqfitLayout->addWidget( pb_resids,   row++, 4, 1, 2 );
   eqfitLayout->addWidget( lb_clambda,  row,   0, 1, 2 );
   eqfitLayout->addWidget( le_clambda,  row,   2, 1, 2 );
   eqfitLayout->addWidget( pb_ovrlays,  row++, 4, 1, 2 );
   eqfitLayout->addWidget( lb_nbrpars,  row,   0, 1, 2 );
   eqfitLayout->addWidget( le_nbrpars,  row++, 2, 1, 2 );
   eqfitLayout->addWidget( lb_nbrsets,  row,   0, 1, 2 );
   eqfitLayout->addWidget( le_nbrsets,  row,   2, 1, 2 );
   eqfitLayout->addWidget( pb_help,     row++, 4, 1, 2 );
   eqfitLayout->addWidget( lb_nbrdpts,  row,   0, 1, 2 );
   eqfitLayout->addWidget( le_nbrdpts,  row,   2, 1, 2 );
   eqfitLayout->addWidget( pb_close,    row++, 4, 1, 2 );

   connect( pb_strtfit, SIGNAL( clicked()    ),
            this,       SLOT(   start_fit()  ) );
   connect( pb_pause,   SIGNAL( clicked()    ),
            this,       SLOT(   pause_fit()  ) );
   connect( pb_resume,  SIGNAL( clicked()    ),
            this,       SLOT(   resume_fit() ) );
   connect( pb_savefit, SIGNAL( clicked()    ),
            this,       SLOT(   save_fit()   ) );
   connect( pb_viewrep, SIGNAL( clicked()        ),
            this,       SLOT(   view_report()    ) );
   connect( pb_resids,  SIGNAL( clicked()        ),
            this,       SLOT(   plot_residuals() ) );
   connect( pb_ovrlays, SIGNAL( clicked()        ),
            this,       SLOT(   plot_overlays()  ) );
   connect( pb_close,   SIGNAL( clicked() ),
            this,       SLOT(   closed()  ) );
   connect( pb_help,    SIGNAL( clicked() ),
            this,       SLOT(   help()    ) );
   pb_pause  ->setEnabled( false );
   pb_resume ->setEnabled( false );
   pb_savefit->setEnabled( false );
   pb_viewrep->setEnabled( false );
   pb_resids ->setEnabled( false );
   pb_ovrlays->setEnabled( false );

   // Graph Plotting Controls layout
   QLabel*  lb_gbanner = us_banner(
         tr( "Graph Plotting Controls" ) );
   QGridLayout* lo_pltalld = us_radiobutton( tr( "All Data" ),
         rb_pltalld, true );
   QGridLayout* lo_pltgrp5 = us_radiobutton( tr( "Groups of 5" ),
         rb_pltgrp5, false );
   QGridLayout* lo_pltsscn = us_radiobutton( tr( "Single Scan" ),
         rb_pltsscn, false );
   QButtonGroup* plotgrp   = new QButtonGroup( this );
   plotgrp->addButton( rb_pltalld );
   plotgrp->addButton( rb_pltgrp5 );
   plotgrp->addButton( rb_pltsscn );
   QLabel*  lb_plotscn  = us_label( tr( "Scan (start):" ) );
            ct_plotscn  = us_counter( 2, 0,  1, 20 );
   QLayout* lo_monfitg  = us_checkbox( tr( "Monitor Fit Graphically" ),
         ck_monfitg, true );
   
   row     = 0;
   gplotLayout->addWidget( lb_gbanner,  row++, 0, 1, 6 );
   gplotLayout->addLayout( lo_pltalld,  row,   0, 1, 2 );
   gplotLayout->addLayout( lo_pltgrp5,  row,   2, 1, 2 );
   gplotLayout->addLayout( lo_pltsscn,  row++, 4, 1, 2 );
   gplotLayout->addWidget( lb_plotscn,  row,   0, 1, 3 );
   gplotLayout->addWidget( ct_plotscn,  row++, 3, 1, 3 );
   gplotLayout->addLayout( lo_monfitg,  row++, 2, 1, 4 );

   connect( ct_plotscn, SIGNAL( valueChanged(double) ), SLOT( new_pscan() ) );
   connect( rb_pltalld, SIGNAL( toggled( bool ) ),      SLOT( new_pscan() ) );
   connect( rb_pltsscn, SIGNAL( toggled( bool ) ),      SLOT( new_pscan() ) );
   plottype = -1;

   // NLSQ Fit Tuning controls
   QLabel*  lb_tbanner = us_banner(
         tr( "NLSQ Fit Tuning Controls" ) );
   QLabel*  lb_lincnst  = us_label( tr( "Linear Constraints:" ) );
   QGridLayout*  lo_lincnsn = us_radiobutton( tr( "No"  ), rb_lincnsn, true );
   QGridLayout*  lo_lincnsy = us_radiobutton( tr( "Yes" ), rb_lincnsy, false );
   QHBoxLayout*  lo_lincbox = new QHBoxLayout;
   QButtonGroup* lcnsgrp    = new QButtonGroup( this );
   lo_lincbox->setSpacing        (  0 );
   lo_lincbox->setContentsMargins( 0, 0, 0, 0 );
   lo_lincbox->addLayout( lo_lincnsn );
   lo_lincbox->addLayout( lo_lincnsy );
   lcnsgrp->addButton( rb_lincnsn );
   lcnsgrp->addButton( rb_lincnsy );
   lcnsgrp->setExclusive( true );
   QLabel*  lb_autocnv  = us_label( tr( "Autoconverge:" ) );
   QGridLayout*  lo_autocnn = us_radiobutton( tr( "No"  ), rb_autocnn, true );
   QGridLayout*  lo_autocny = us_radiobutton( tr( "Yes" ), rb_autocny, false );
   QHBoxLayout*  lo_autcbox = new QHBoxLayout;
   QButtonGroup* autcgrp    = new QButtonGroup( this );
   lo_autcbox->setSpacing        (  0 );
   lo_autcbox->setContentsMargins( 0, 0, 0, 0 );
   lo_autcbox->addLayout( lo_autocnn );
   lo_autcbox->addLayout( lo_autocny );
   autcgrp->addButton( rb_autocnn );
   autcgrp->addButton( rb_autocny );
   autcgrp->setExclusive( true );
   QLabel*  lb_nlsalgo  = us_label( tr( "NLS Algorithm:" ) );
            cb_nlsalgo  = us_comboBox();
   QLabel*  lb_lamstrt  = us_label( tr( "Lambda Start:" ) );
            le_lamstrt  = us_lineedit( "10000" );
            pb_lnvsr2   = us_pushbutton( tr( "Ln(C) vs R^2" ) );
   QLabel*  lb_lamsize  = us_label( tr( "Lambda Step Size:" ) );
            le_lamsize  = us_lineedit( "10" );
            pb_mwvsr2   = us_pushbutton( tr( "MW vs R^2" ) );
   QLabel*  lb_mxiters  = us_label( tr( "Maximum Iterations:" ) );
            le_mxiters  = us_lineedit( "1000" );
            pb_mwvscv   = us_pushbutton( tr( "MW vs C" ) );
   QLabel*  lb_fittolr  = us_label( tr( "Fit Tolerance:" ) );
            le_fittolr  = us_lineedit( "1.0000e-12" );
            pb_cmments  = us_pushbutton( tr( "Comments" ) );
   QLabel*  lb_fprogres = us_label( tr( "Fitting Progress:" ) );
            progress    = us_progressBar( 0, 100, 0 );

   row     = 0;
   ftuneLayout->addWidget( lb_tbanner,  row++, 0, 1, 6 );
   ftuneLayout->addWidget( lb_lincnst,  row,   0, 1, 2 );
   ftuneLayout->addLayout( lo_lincbox,  row++, 2, 1, 4 );
   ftuneLayout->addWidget( lb_autocnv,  row,   0, 1, 2 );
   ftuneLayout->addLayout( lo_autcbox,  row++, 2, 1, 4 );
   ftuneLayout->addWidget( lb_nlsalgo,  row,   0, 1, 2 );
   ftuneLayout->addWidget( cb_nlsalgo,  row++, 2, 1, 4 );
   ftuneLayout->addWidget( lb_lamstrt,  row,   0, 1, 2 );
   ftuneLayout->addWidget( le_lamstrt,  row,   2, 1, 2 );
   ftuneLayout->addWidget( pb_lnvsr2,   row++, 4, 1, 2 );
   ftuneLayout->addWidget( lb_lamsize,  row,   0, 1, 2 );
   ftuneLayout->addWidget( le_lamsize,  row,   2, 1, 2 );
   ftuneLayout->addWidget( pb_mwvsr2,   row++, 4, 1, 2 );
   ftuneLayout->addWidget( lb_mxiters,  row,   0, 1, 2 );
   ftuneLayout->addWidget( le_mxiters,  row,   2, 1, 2 );
   ftuneLayout->addWidget( pb_mwvscv,   row++, 4, 1, 2 );
   ftuneLayout->addWidget( lb_fittolr,  row,   0, 1, 2 );
   ftuneLayout->addWidget( le_fittolr,  row,   2, 1, 2 );
   ftuneLayout->addWidget( pb_cmments,  row++, 4, 1, 2 );
   ftuneLayout->addWidget( lb_fprogres, row,   0, 1, 2 );
   ftuneLayout->addWidget( progress,    row++, 2, 1, 4 );

   cb_nlsalgo->addItem( tr( "Levenberg-Marquardt" ) );
   cb_nlsalgo->addItem( tr( "Modified Gauss-Newton" ) );
   cb_nlsalgo->addItem( tr( "Hybrid Method" ) );
   cb_nlsalgo->addItem( tr( "Quasi-Newton Method" ) );
   cb_nlsalgo->addItem( tr( "Generalized Linear LS" ) );
   cb_nlsalgo->addItem( tr( "NonNegative constrained LS" ) );
   connect( pb_lnvsr2,  SIGNAL( clicked()    ),
            this,       SLOT(   plot_two()   ) );
   connect( pb_mwvsr2,  SIGNAL( clicked()    ),
            this,       SLOT(   plot_three() ) );
   connect( pb_mwvscv,  SIGNAL( clicked()    ),
            this,       SLOT(   plot_four()  ) );

   // Status layout
   QLabel*  lb_status   = us_label( tr( "Status:" ) );
            le_status   = us_lineedit(
                  tr( "Waiting for Input - Please click on \"Fit\" to start"
                      " the fitting process..." ) );
   QLabel*  lb_inform   = us_label( tr( "Information:" ) );
            le_inform   = us_lineedit();

   row     = 0;
   statsLayout->addWidget( lb_status,   row,   0, 1, 2 );
   statsLayout->addWidget( le_status,   row++, 2, 1, 8 );
   statsLayout->addWidget( lb_inform,   row,   0, 1, 2 );
   statsLayout->addWidget( le_inform,   row++, 2, 1, 8 );


   lsideLayout->addLayout( eqfitLayout );
   lsideLayout->addLayout( gplotLayout );
   lsideLayout->addLayout( ftuneLayout );
   lsideLayout->addStretch();

   rsideLayout->addLayout( dplot       );
   rsideLayout->addLayout( statsLayout );
   rsideLayout->setStretchFactor( dplot,       1 );
   rsideLayout->setStretchFactor( statsLayout, 0 );

   main->addLayout( lsideLayout );
   main->addLayout( rsideLayout );
   main->setStretchFactor( lsideLayout, 2 );
   main->setStretchFactor( rsideLayout, 4 );

   // Resize to fit elements added
   setMinimumSize( 1000, 600 );

   adjustSize();

   // Initialize for fit
   nlsmeth      = 0;
   cb_nlsalgo->setCurrentIndex( nlsmeth );

   emath->init_fit( modelx, nlsmeth, fitpars );
qDebug() << "EFC: init_fit return";
   nfpars       = fitpars.nfpars;
   ndsets       = fitpars.ndsets;
   ntpts        = fitpars.ntpts;

   le_nbrpars->setText( QString::number( nfpars ) );
   le_nbrsets->setText( QString::number( ndsets ) );
   le_nbrdpts->setText( QString::number( ntpts  ) );
   ct_plotscn->setRange( 1, ndsets );
   ct_plotscn->setSingleStep(  1 );
   ct_plotscn->setValue( 1 );
}

// Close
void US_EqFitControl::closed()
{
   reject();
   fWidget = false;
   close();
}

// Start the fitting computations
void US_EqFitControl::start_fit()
{
qDebug() << "START_FIT";
   const char* cmeth[] = { "L-M", "M G-N", "Hybrid", "Q-N", "GLLS", "NNLS" };

   if ( pb_strtfit->text().contains( tr( "Abort" ) ) )
   {
      fitwork->flag_abort( );

      pb_strtfit->setText( tr( "Fit" ) );
      pb_pause  ->setEnabled( false );
      pb_resume ->setEnabled( false );
      pb_savefit->setEnabled( false );
      pb_close  ->setEnabled( true  );

      le_inform->setText( tr( "Fitting ABORTED by user request!" ) );
      return;
   }

   nlsmeth   = cb_nlsalgo->currentIndex();
   le_status->setText( tr( "Equilibrium " ) + QString( cmeth[ nlsmeth ] ) +
         tr( " fitting for " ) + models[ modelx ] + "..." );
   le_inform->setText( tr( "Fitting iterations have begun" ) );

   emath->init_fit( modelx, nlsmeth, fitpars );

   mxiters   = le_mxiters->text().toInt();
   fittoler  = le_fittolr->text().toDouble();
   ntpts     = fitpars.ntpts;
   ndsets    = fitpars.ndsets;
   nfpars    = fitpars.nfpars;

   fitpars.nlsmeth   = nlsmeth;
   fitpars.modelx    = modelx;
   fitpars.mxiters   = mxiters;
   fitpars.mxsteps   = mxiters;
   fitpars.lam_start = le_lamstrt->text().toInt();
   fitpars.lam_step  = le_lamsize->text().toInt();
   fitpars.fittoler  = fittoler;
   fitpars.lincnstr  = rb_lincnsy->isChecked();
   fitpars.autocnvg  = rb_autocny->isChecked();

   fitwork   = new US_FitWorker( emath, fitpars, this );

   pb_strtfit->setText( tr( "Abort Fit" ) );
   pb_pause  ->setEnabled( true  );
   pb_resume ->setEnabled( false );
   pb_savefit->setEnabled( false );
   pb_close  ->setEnabled( false );

   connect( fitwork, SIGNAL( work_progress( int ) ),
            this,    SLOT(   new_progress ( int ) ) );
   connect( fitwork, SIGNAL( work_complete()      ),
            this,    SLOT(   fit_completed()      ) );
   progress->setRange( 1, mxiters );
   progress->reset();

   fitwork->start();

}
// Pause the fitting computations
void US_EqFitControl::pause_fit()
{
qDebug() << "PAUSE_FIT";
   fitwork->flag_paused( true );
   pb_pause  ->setEnabled( false );
   pb_resume ->setEnabled( true  );
   pb_strtfit->setEnabled( false );
}

// Resume the fitting computations
void US_EqFitControl::resume_fit()
{
qDebug() << "RESUME_FIT";
   fitwork->flag_paused( false );
   pb_pause  ->setEnabled( true  );
   pb_resume ->setEnabled( false );
   pb_strtfit->setEnabled( true  );
}

// Save the computed fit to a file
void US_EqFitControl::save_fit()
{
qDebug() << "SAVE_FIT";
}
// View a report on the fit
void US_EqFitControl::view_report()
{
qDebug() << "VIEW_REPORT";
   bool showgui = true;
   bool writerf = true;
   QString filename;

   emath->calc_runs();
   emath->calc_integral();

   ereporter->fit_report( fitpars, showgui,
         writerf, filename );
qDebug() << " V_REP filename" << filename;
}

// Plot residuals for scan(s)
void US_EqFitControl::plot_residuals()
{
qDebug() << "PLOT_RESIDUALS";
   plottype = 0;

   // Prepare the data, such as y_delta and indexes
   prepare_data();
qDebug() << "PL_R: mxspts" << mxspts << "ipscnn" << ipscnn;

   QVector< double > v_xplot;
   QVector< double > v_yplot;
   v_xplot.fill( 0.0, mxspts );
   v_yplot.fill( 0.0, mxspts );
   double* xplot  = v_xplot.data();
   double* yplot  = v_yplot.data();
   double* ydelta = fitpars.y_delta;
   double  xpzero[ 2 ];
   double  ypzero[ 2 ];
   double  yoffs  = 0.0;
   double  yoffi  = ( plotgrpf == 5 ) ? 0.03 : 0.0;
qDebug() << " mxspts ntpts" << mxspts << ntpts;
qDebug() << "  ydelta0" << ydelta[0] << " ydeltan" << ydelta[ntpts-1];

   dataPlotClear( data_plot );
   data_plot->setTitle( tr( "Residuals" ) );
   data_plot->setAxisTitle( QwtPlot::yLeft,
      tr( "Optical Density Difference" ) );
   data_plot->setAxisTitle( QwtPlot::xBottom,
      tr( "Radius^2 - Radius(ref)^2 (cm)" ) );
   QwtPlotGrid* grid = us_grid( data_plot );
   grid->enableYMin( true );
   grid->enableY   ( true );
   grid->setMajorPen( QPen( US_GuiSettings::plotMajGrid(), 0, Qt::DashLine ) );
   grid->setMinorPen( QPen( US_GuiSettings::plotMinGrid(), 0, Qt::DotLine  ) );

   QwtSymbol* sym = new QwtSymbol;
   sym->setStyle( QwtSymbol::Ellipse );
   sym->setPen  ( QPen  ( Qt::blue   ) );
   sym->setBrush( QBrush( Qt::yellow ) );
   sym->setSize ( plotgrpf < 0 ? 8 : 5 );

   QPen lnpen( QBrush( Qt::green ), 1 );
   QPen zlpen( QBrush( Qt::red   ), 2 );
   double xmin = 0.0;
   double xmax = 0.0;
   double xpad = 0.2;
   int    sns[ 5 ];
   int    ksn  = 0;

   for ( int ii = 0; ii < scanfits.size(); ii++ )
   {
      EqScanFit* scnf = &scanfits[ ii ];
      int scnn      = ii + 1;

//qDebug() << "RPlot: ii scnn" << ii << scnn << " ipsc lpsc" << ipscnn << lpscnn;
      if ( ! scnf->scanFit  ||  scnn < ipscnn  ||  scnn > lpscnn )
         continue;

      int    jdsx   = dscnx[ ii ];
      int    jvxy   = scnf->start_ndx;
      int    nspts  = scnf->stop_ndx - jvxy + 1;
      double xmsq   = sq( scnf->xvs[ jvxy ] );
//qDebug() << "RPlot: ii scnn" << ii << scnn << " jdsx jvsy" << jdsx << jvxy
//   << " nspts" << nspts;

      for ( int jj = 0; jj < nspts; jj++ )
      {
         xplot[ jj ]   = sq( scnf->xvs[ jvxy ] ) - xmsq;
//qDebug() << "RPlot:  jj jvxy" << jj << jvxy << "xval" << scnf->xvs[jvxy]; 
         yplot[ jj ]   = ydelta[ jdsx ] + yoffs;
//qDebug() << "RPlot:   jdsx" << jdsx << "yval" << ydelta[jdsx];
         jvxy++;
         jdsx++;
      }
//qDebug() << "RPlot:  jdsx jvxy" << jdsx << jvxy;

      xmax   = qMax( xmax, xplot[ nspts - 1 ] );
      xmin   = qMin( xmin, -xpad );
      QwtPlotCurve* lcurve = us_curve( data_plot,
            QString( "RLine-%1" ).arg( scnn ) );

      lcurve->setStyle  ( QwtPlotCurve::Lines );
      lcurve->setPen    ( lnpen );
      lcurve->setSamples( xplot, yplot, nspts );

      QwtPlotCurve* scurve = us_curve( data_plot,
            QString( "RSymb-%1" ).arg( scnn ) );
      scurve->setStyle  ( QwtPlotCurve::NoCurve );
      scurve->setSymbol ( sym );
      scurve->setSamples( xplot, yplot, nspts );

      yoffs        += yoffi;

      if ( plotgrpf == 5 )  sns[ ksn++ ] = scnn;
   }

   xmax  += xpad;
   data_plot->setAxisScale( QwtPlot::xBottom, xmin, xmax );
   data_plot->setAxisAutoScale( QwtPlot::yLeft );

   if ( plotgrpf == 5 )
   {
      QFont lbfont( US_GuiSettings::fontFamily(), -1, QFont::Bold );
      double yofmx = yoffs;
      yoffs        = 0.0;
      ksn          = 0;
      xpad        *= 0.5;
      xpzero[ 0 ]  = xmin;
      xpzero[ 1 ]  = xmax - xpad;
      xmax        += xpad * 0.5;

      while ( yoffs < yofmx )
      {
         int scnn    = sns[ ksn++ ];
         QwtPlotCurve* zcurve = us_curve( data_plot,
               QString( "RZero-%1" ).arg( scnn ) );
         ypzero[ 0 ] = yoffs;
         ypzero[ 1 ] = yoffs;
         zcurve->setStyle  ( QwtPlotCurve::Lines );
         zcurve->setPen    ( zlpen );
         zcurve->setSamples( xpzero, ypzero, 2 );
         QwtPlotMarker* marker = new QwtPlotMarker;
         QwtText        mlabel;
         mlabel.setText( QString::number( scnn ) );
         mlabel.setFont( lbfont );
         mlabel.setColor( Qt::red );
         mlabel.setBackgroundBrush( QBrush( Qt::white ) );
         marker->setValue( xmax, yoffs );
         marker->setLabel( mlabel );
         marker->setLabelAlignment( Qt::AlignLeft | Qt::AlignVCenter );
         marker->attach( data_plot );

         yoffs      += yoffi;
      }
   }

   else
   {
      QwtPlotCurve* zcurve = us_curve( data_plot, QString( "RZero-0" ) );
      xpzero[ 0 ] = xmin;
      xpzero[ 1 ] = xmax;
      ypzero[ 0 ] = 0.0;
      ypzero[ 1 ] = 0.0;
      zcurve->setStyle  ( QwtPlotCurve::Lines );
      zcurve->setPen    ( zlpen );
      zcurve->setSamples( xpzero, ypzero, 2 );
   }

   data_plot->replot();
}

// Plot overlays for scan(s)
void US_EqFitControl::plot_overlays()
{
qDebug() << "PLOT_OVERLAYS";
   plottype = 1;

   // Prepare the data, such as indexes and counts
   prepare_data();
qDebug() << "PL_O: mxspts" << mxspts << "ipscnn" << ipscnn;

   QVector< double > v_xplot;
   v_xplot.fill( 0.0, mxspts );
   double* xplot  = v_xplot.data();
   double* yraw   = fitpars.y_raw;
   double* yguess = fitpars.y_guess;
   double* ypraw  = yraw;
   double* ypfit  = yguess;
qDebug() << " mxspts ntpts" << mxspts << ntpts;
qDebug() << "  yguess0" << yguess[0] << " yguessn" << yguess[ntpts-1];

   dataPlotClear( data_plot );
   data_plot->setTitle( ( npscns == 1 ) ?
       tr( "Overlays for fitted Scan %1" ).arg( ipscnn ) :
       tr( "Overlays for fitted Scans %1 - %2" ).arg( ipscnn ).arg( lpscnn ) );
   data_plot->setAxisTitle( QwtPlot::yLeft,
      tr( "Optical Density Difference" ) );
   data_plot->setAxisTitle( QwtPlot::xBottom,
      tr( "Radius^2 - Radius(ref)^2 (cm)" ) );
   QwtPlotGrid* grid = us_grid( data_plot );
   grid->enableYMin( true );
   grid->enableY   ( true );
   grid->setMajorPen( QPen( US_GuiSettings::plotMajGrid(), 0, Qt::DashLine ) );
   grid->setMinorPen( QPen( US_GuiSettings::plotMinGrid(), 0, Qt::DotLine  ) );

   QwtSymbol* sym = new QwtSymbol;
   sym->setStyle( QwtSymbol::Ellipse );
   sym->setPen  ( QPen  ( Qt::blue   ) );
   sym->setBrush( QBrush( Qt::yellow ) );
   sym->setSize ( 4 );

   QPen lnpen( QBrush( Qt::red ), 1 );
   double xmin = 0.0;
   double xmax = 0.0;
   double xpad = 0.05;

   for ( int ii = 0; ii < scanfits.size(); ii++ )
   {
      EqScanFit* scnf = &scanfits[ ii ];
      int scnn      = ii + 1;

//qDebug() << "OPlot: ii scnn" << ii << scnn << " ipsc lpsc" << ipscnn << lpscnn;
      if ( ! scnf->scanFit  ||  scnn < ipscnn  ||  scnn > lpscnn )
         continue;

      int    jdsx   = dscnx[ ii ];
      int    jvxy   = scnf->start_ndx;
      int    nspts  = scnf->stop_ndx - jvxy + 1;
      double xmsq   = sq( scnf->xvs[ jvxy ] );
      ypraw         = yraw   + jdsx;
      ypfit         = yguess + jdsx;
//qDebug() << "RPlot: ii scnn" << ii << scnn << " jdsx jvsy" << jdsx << jvxy
//   << " nspts" << nspts;

      for ( int jj = 0; jj < nspts; jj++,jvxy++ )
      {
         xplot[ jj ]   = sq( scnf->xvs[ jvxy ] ) - xmsq;
//qDebug() << "RPlot:  jj jvxy" << jj << jvxy << "xval" << scnf->xvs[jvxy]; 
      }
//qDebug() << "RPlot:  jdsx jvxy" << jdsx << jvxy;

      xmax   = qMax( xmax, xplot[ nspts - 1 ] );
      xmin   = qMin( xmin, -xpad );

      QwtPlotCurve* scurve = us_curve( data_plot,
            QString( "RSymb-%1" ).arg( scnn ) );
      scurve->setStyle  ( QwtPlotCurve::NoCurve );
      scurve->setSymbol ( sym );
      scurve->setSamples( xplot, ypraw, nspts );  // Plot raw symbols

      QwtPlotCurve* lcurve = us_curve( data_plot,
            QString( "RLine-%1" ).arg( scnn ) );
      lcurve->setStyle  ( QwtPlotCurve::Lines );
      lcurve->setPen    ( lnpen );
      lcurve->setSamples( xplot, ypfit, nspts );  // Plot fitted line
   }

   xmax  += xpad;
   data_plot->setAxisScale( QwtPlot::xBottom, xmin, xmax );
   data_plot->setAxisAutoScale( QwtPlot::yLeft );

   data_plot->replot();
}

// Plot Ln(C) vs R^2 for scan(s)
void US_EqFitControl::plot_two()
{
qDebug() << "PLOT_TWO";
   plottype = 2;
}

// Plot Ln(C) vs R^2 for scan(s)
void US_EqFitControl::plot_three()
{
qDebug() << "PLOT_THREE";
   plottype = 3;
}

// Plot Ln(C) vs R^2 for scan(s)
void US_EqFitControl::plot_four()
{
qDebug() << "PLOT_FOUR";
   plottype = 4;
}

// Update progress bar
void US_EqFitControl::new_progress( int step )
{
qDebug() << "NEW_PROGRESS" << step;
   progress->setValue( step );

   le_clambda->setText( QString::number( fitpars.lambda   ) );
   le_iternbr->setText( QString::number( fitpars.k_iter   ) );
   le_varianc->setText( QString::number( fitpars.variance ) );
   le_stddev ->setText( QString::number( fitpars.std_dev  ) );
   le_improve->setText( QString::number( fitpars.improve  ) );
   le_funceva->setText( QString::number( fitpars.nfuncev  ) );
   le_decompo->setText( QString::number( fitpars.ndecomps ) );
   le_inform ->setText(
      tr( "Iteration %1 has completed." ).arg( fitpars.k_iter ) );

   if ( ck_monfitg->isChecked() )
   {
      plot_residuals();
   }
}

// React to completion of fit
void US_EqFitControl::fit_completed()
{
qDebug() << "FIT_COMPLETED";
   pb_strtfit->setText( tr( "Fit" ) );

   QString iinform;

   if ( fitpars.aborted )
      iinform = tr( "Fitting ABORTED!" );

   else if ( fitpars.converged )
      iinform = tr( "Fitting CONVERGED!" );

   else if ( fitpars.completed )
      iinform = tr( "Fitting iterations COMPLETED!" );

   iinform = iinform + " ( " + fitpars.infomsg + " )";
   le_inform ->setText( iinform );
   pb_close  ->setEnabled( true );
   pb_savefit->setEnabled( true );
   pb_viewrep->setEnabled( true );
   pb_resids ->setEnabled( true );
   pb_ovrlays->setEnabled( true );

   if ( ck_monfitg->isChecked()  &&  ! fitpars.aborted )
   {
      plot_residuals();
   }
}

// Prepare data for plots:  get y_delta and data indecies,counts
void US_EqFitControl::prepare_data()
{
   double* ydelta = fitpars.y_delta;
   double* yguess = fitpars.y_guess;
   double* yraw   = fitpars.y_raw;
   mxspts     = 0;
   ipscnn     = (int)ct_plotscn->value();
   lpscnn     = 0;
   int kpscns = 0;
   int jpscnn = 0;
   int dssx   = 0;
   int ntscns = scanfits.size();
   int niscns = 0;
   int liscnn = 0;
   v_dscnx.fill( 0, ntscns );
   dscnx      = v_dscnx.data();

   for ( int ii = 0; ii < ntscns; ii++ )
   {
      EqScanFit* scnf = &scanfits[ ii ];
      int scann   = ii + 1;

      if ( ! scnf->scanFit )  continue;

      niscns++;                           // Bump included-scans count
      dscnx[ ii ] = dssx;                 // Save scan's start data index
      liscnn      = scann;                // Last included scan number
      int nspts   = scnf->stop_ndx - scnf->start_ndx + 1;
      mxspts      = qMax( mxspts, nspts ); // Maximum data points in a scan

      if ( scann >= ipscnn )
         kpscns++;

      if ( kpscns == 1 )
         ipscnn   = scann;                // Initial plot scan number

      if ( kpscns > 0  &&  kpscns < 6 )
         lpscnn   = scann;                // Last plot scan number

      if ( kpscns == 6 )
         jpscnn   = scann;                // Next group-of-5 start scan number
//qDebug() << "PrD: sn ipsc lpsc jpsc kpsc" << scann << ipscnn << lpscnn
//   << jpscnn << kpscns;

      for ( int jj = 0; jj < nspts; jj++ )
      {  // Insure we have freshly calculated deltas for scan data
         ydelta[ dssx ] = yraw[ dssx ] - yguess[ dssx ];
         dssx++;                          // Scan's data index
      }
   }

   plotgrpf   = 0;
   npscns     = niscns;

   if ( rb_pltalld->isChecked() )
   {  // Plot all
      plotgrpf   = 0;                 // Flag plot-all
      npscns     = niscns;            // Plot scans is total included scans
      lpscnn     = liscnn;            // Last plot scan is last included
      jpscnn     = 1;
   }

   else if ( rb_pltgrp5->isChecked() )
   {  // Plot in group of 5
      plotgrpf   = 5;                 // Flag plot-5
      npscns     = qMin( kpscns, 5 );  // Plot scans is at-most 5
      jpscnn     = qMax( npscns, ( jpscnn - ipscnn ) );
   }

   else if ( rb_pltsscn->isChecked() )
   {  // Plot single scan
      plotgrpf   = -1;                // Flag plot-1
      npscns     = 1;                 // Plot scans is one
      lpscnn     = ipscnn;            // Last plot scan is same as first
      jpscnn     = 1;
   }

   ct_plotscn->setRange( 1, liscnn );
   ct_plotscn->setSingleStep(  jpscnn );
   ct_plotscn->disconnect();
   ct_plotscn->setValue( ipscnn );
   connect( ct_plotscn, SIGNAL( valueChanged( double ) ), SLOT( new_pscan() ) );
//qDebug() << "PREP_DATA ipscnn lpscnn liscnn jpscnn" << ipscnn << lpscnn
//   << liscnn << jpscnn;
}

// New plot scan number selected
void US_EqFitControl::new_pscan()
{
qDebug() << "NEW_PSCAN plottype" << plottype << ct_plotscn->value();
   switch ( plottype )
   {
      case 0:
         plot_residuals();
         break;
      case 1:
         plot_overlays();
         break;
      case 2:
         plot_two();
         break;
      case 3:
         plot_three();
         break;
      case 4:
         plot_four();
         break;
      default:
         break;
   }
}

