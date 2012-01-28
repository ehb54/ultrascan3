//! \file us_analysis_control.cpp

#include "us_2dsa.h"
#include "us_analysis_control.h"
#include "us_adv_analysis.h"
#include "us_settings.h"
#include "us_passwd.h"
#include "us_db2.h"
#include "us_gui_settings.h"
#include "us_memory.h"

#include <qwt_legend.h>

// constructor:  2dsa analysis controls widget
US_AnalysisControl::US_AnalysisControl( QList< US_SolveSim::DataSet* >& dsets,
    QWidget* p ) : US_WidgetsDialog( p, 0 ), dsets( dsets )
{
   parentw        = p;
   processor      = 0;
   dbg_level      = US_Settings::us_debug();
   grtype         = US_2dsaProcess::UGRID;

   setObjectName( "US_AnalysisControl" );
   setAttribute( Qt::WA_DeleteOnClose, true );
   setPalette( US_GuiSettings::frameColor() );
   setFont( QFont( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() ) );

   // lay out the GUI
   setWindowTitle( tr( "2-D Spectrum Analysis Controls" ) );

   mainLayout      = new QHBoxLayout( this );
   controlsLayout  = new QGridLayout( );
   optimizeLayout  = new QGridLayout( );

   mainLayout->setSpacing        ( 2 );
   mainLayout->setContentsMargins( 2, 2, 2, 2 );

   mainLayout->addLayout( controlsLayout );
   mainLayout->addLayout( optimizeLayout  );

   QLabel* lb_fitting      = us_banner( tr( "Fitting Controls:" ) );
   QLabel* lb_lolimits     = us_label(  tr( "Lower Limit (s x 1e-13):" ) );
   QLabel* lb_uplimits     = us_label(  tr( "Upper Limit (s):" ) );
   QLabel* lb_nstepss      = us_label(  tr( "Number Grid Points (s):" ) );
           lb_lolimitk     = us_label(  tr( "Lower Limit (f/f0):" ) );
           lb_uplimitk     = us_label(  tr( "Upper Limit (f/f0):" ) );
           lb_nstepsk      = us_label(  tr( "Number Grid Points (f/f0):" ) );
           lb_constff0     = us_label(  tr( "Constant f/f0:"      ) );
   QLabel* lb_thrdcnt      = us_label(  tr( "Thread Count:" ) );
   QLabel* lb_estmemory    = us_label(  tr( "Estimated Memory:" ) );
   QLabel* lb_iteration    = us_label(  tr( "Completed Iteration:" ) );
   QLabel* lb_oldvari      = us_label(  tr( "Old Variance:" ) );
   QLabel* lb_newvari      = us_label(  tr( "New Variance:" ) );
   QLabel* lb_improve      = us_label(  tr( "Improvement:" ) );
   QLabel* lb_status       = us_label(  tr( "Status:" ) );

   QLabel* lb_optimiz      = us_banner( tr( "Optimization Methods:" ) );
   QLabel* lb_grrefine     = us_label(  tr( "Grid Refinements:" ) );
   QLabel* lb_menisrng     = us_label(  tr( "Meniscus Fit Range (cm):" ) );
   QLabel* lb_menispts     = us_label(  tr( "Meniscus Grid Points:" ) );
   QLabel* lb_mciters      = us_label(  tr( "Monte Carlo Iterations:" ) );
   QLabel* lb_iters        = us_label(  tr( "Maximum Iterations:" ) );
   QLabel* lb_statinfo     = us_banner( tr( "Status Information:" ) );

   pb_strtfit              = us_pushbutton( tr( "Start Fit" ),    true  );
   pb_stopfit              = us_pushbutton( tr( "Stop Fit" ),     false );
   pb_plot                 = us_pushbutton( tr( "Plot Results" ), false );
   pb_save                 = us_pushbutton( tr( "Save Results" ), false );
   QPushButton* pb_help    = us_pushbutton( tr( "Help" ) );
   QPushButton* pb_close   = us_pushbutton( tr( "Close" ) );
   QPushButton* pb_advance = us_pushbutton( tr( "Advanced Analysis Controls" ));
   te_status               = us_textedit();

   QLayout* lo_tinois      =
      us_checkbox( tr( "Fit Time-Invariant Noise"     ), ck_tinoise );
   QLayout* lo_rinois      =
      us_checkbox( tr( "Fit Radially-Invariant Noise" ), ck_rinoise );
   QLayout* lo_autoplt     =
      us_checkbox( tr( "Automatically Plot"           ), ck_autoplt );
   QLayout* lo_varvbar     =
      us_checkbox( tr( "Vary Vbar with Constant f/f0" ), ck_varvbar );

   int nthr     = US_Settings::threads();
   nthr         = ( nthr > 1 ) ? nthr : QThread::idealThreadCount();
DbgLv(1) << "idealThrCout" << nthr;
   ct_lolimits  = us_counter( 3, -10000, 10000,  1 );
   ct_uplimits  = us_counter( 3, -10000, 10000 ,10 );
   ct_nstepss   = us_counter( 3,      1,  1000,  60 );
   ct_lolimitk  = us_counter( 3,      1,     8,   1 );
   ct_uplimitk  = us_counter( 3,      1,    10,   4 );
   ct_nstepsk   = us_counter( 3,      1,  1000,  60 );
   ct_thrdcnt   = us_counter( 2,      1,    64, nthr );
   ct_constff0  = us_counter( 3,      1,    10,   1  );
   ct_lolimits->setStep(  0.1 );
   ct_uplimits->setStep(  0.1 );
   ct_nstepss ->setStep(    1 );
   ct_lolimitk->setStep( 0.01 );
   ct_uplimitk->setStep( 0.01 );
   ct_nstepsk ->setStep(    1 );
   ct_thrdcnt ->setStep(    1 );
   ct_constff0->setStep( 0.01 );

   le_estmemory = us_lineedit( "100 MB" );
   le_iteration = us_lineedit( "0" );
   le_oldvari   = us_lineedit( "0.000e-05" );
   le_newvari   = us_lineedit( "0.000e-05" );
   le_improve   = us_lineedit( "0.000e-08" );

   b_progress   = us_progressBar( 0, 100, 0 );

   QLayout*  lo_iters   =
      us_checkbox( tr( "Use Iterative Method"              ), ck_iters  );
   QLayout*  lo_unifgr  =
      us_checkbox( tr( "Uniform Grid"                      ), ck_unifgr, true );
   QLayout*  lo_menisc  =
      us_checkbox( tr( "Float Meniscus Position"           ), ck_menisc );
   QLayout*  lo_mcarlo  =
      us_checkbox( tr( "Monte Carlo Iterations"            ), ck_mcarlo );


   ct_iters     = us_counter( 2,    1,   16,    1 );
   ct_grrefine  = us_counter( 2,    1,   20,    6 );
   ct_menisrng  = us_counter( 3, 0.01, 0.65, 0.03 );
   ct_menispts  = us_counter( 2,    3,   21,   10 );
   ct_mciters   = us_counter( 3,    3, 2000,   20 );
   ct_grrefine ->setStep(    1 );
   ct_menisrng ->setStep( 0.01 );
   ct_menispts ->setStep(    1 );
   ct_mciters  ->setStep(    1 );
   ct_iters    ->setStep(    1 );

   int row       = 0;
   controlsLayout->addWidget( lb_fitting,    row++, 0, 1, 4 );
   controlsLayout->addWidget( lb_lolimits,   row,   0, 1, 2 );
   controlsLayout->addWidget( ct_lolimits,   row++, 2, 1, 2 );
   controlsLayout->addWidget( lb_uplimits,   row,   0, 1, 2 );
   controlsLayout->addWidget( ct_uplimits,   row++, 2, 1, 2 );
   controlsLayout->addWidget( lb_nstepss,    row,   0, 1, 2 );
   controlsLayout->addWidget( ct_nstepss,    row++, 2, 1, 2 );
   controlsLayout->addWidget( lb_lolimitk,   row,   0, 1, 2 );
   controlsLayout->addWidget( ct_lolimitk,   row++, 2, 1, 2 );
   controlsLayout->addWidget( lb_uplimitk,   row,   0, 1, 2 );
   controlsLayout->addWidget( ct_uplimitk,   row++, 2, 1, 2 );
   controlsLayout->addWidget( lb_nstepsk,    row,   0, 1, 2 );
   controlsLayout->addWidget( ct_nstepsk,    row++, 2, 1, 2 );
   controlsLayout->addWidget( lb_thrdcnt,    row,   0, 1, 2 );
   controlsLayout->addWidget( ct_thrdcnt,    row++, 2, 1, 2 );
   controlsLayout->addLayout( lo_tinois,     row,   0, 1, 2 );
   controlsLayout->addLayout( lo_rinois,     row++, 2, 1, 2 );
   controlsLayout->addLayout( lo_autoplt,    row,   0, 1, 2 );
   controlsLayout->addLayout( lo_varvbar,    row++, 2, 1, 2 );
   controlsLayout->addWidget( lb_constff0,   row,   0, 1, 2 );
   controlsLayout->addWidget( ct_constff0,   row++, 2, 1, 2 );
   controlsLayout->addWidget( pb_strtfit,    row,   0, 1, 2 );
   controlsLayout->addWidget( pb_stopfit,    row++, 2, 1, 2 );
   controlsLayout->addWidget( pb_plot,       row,   0, 1, 2 );
   controlsLayout->addWidget( pb_save,       row++, 2, 1, 2 );
   controlsLayout->addWidget( pb_help,       row,   0, 1, 2 );
   controlsLayout->addWidget( pb_close,      row++, 2, 1, 2 );
   controlsLayout->addWidget( lb_estmemory,  row,   0, 1, 2 );
   controlsLayout->addWidget( le_estmemory,  row++, 2, 1, 2 );
   controlsLayout->addWidget( lb_iteration,  row,   0, 1, 2 );
   controlsLayout->addWidget( le_iteration,  row++, 2, 1, 2 );
   controlsLayout->addWidget( lb_oldvari,    row,   0, 1, 2 );
   controlsLayout->addWidget( le_oldvari,    row++, 2, 1, 2 );
   controlsLayout->addWidget( lb_newvari,    row,   0, 1, 2 );
   controlsLayout->addWidget( le_newvari,    row++, 2, 1, 2 );
   controlsLayout->addWidget( lb_improve,    row,   0, 1, 2 );
   controlsLayout->addWidget( le_improve,    row++, 2, 1, 2 );
   controlsLayout->addWidget( lb_status,     row,   0, 1, 1 );
   controlsLayout->addWidget( b_progress,    row++, 1, 1, 3 );

   row           = 0;
   optimizeLayout->addWidget( lb_optimiz,    row++, 0, 1, 2 );
   optimizeLayout->addLayout( lo_unifgr,     row++, 0, 1, 2 );
   optimizeLayout->addWidget( lb_grrefine,   row,   0, 1, 1 );
   optimizeLayout->addWidget( ct_grrefine,   row++, 1, 1, 1 );
   optimizeLayout->addLayout( lo_menisc,     row++, 0, 1, 2 );
   optimizeLayout->addWidget( lb_menisrng,   row,   0, 1, 1 );
   optimizeLayout->addWidget( ct_menisrng,   row++, 1, 1, 1 );
   optimizeLayout->addWidget( lb_menispts,   row,   0, 1, 1 );
   optimizeLayout->addWidget( ct_menispts,   row++, 1, 1, 1 );
   optimizeLayout->addLayout( lo_mcarlo,     row++, 0, 1, 2 );
   optimizeLayout->addWidget( lb_mciters,    row,   0, 1, 1 );
   optimizeLayout->addWidget( ct_mciters,    row++, 1, 1, 1 );
   optimizeLayout->addWidget( pb_advance,    row++, 0, 1, 2 );
   optimizeLayout->addLayout( lo_iters,      row++, 0, 1, 2 );
   optimizeLayout->addWidget( lb_iters,      row,   0, 1, 1 );
   optimizeLayout->addWidget( ct_iters,      row++, 1, 1, 1 );
   optimizeLayout->addWidget( lb_statinfo,   row++, 0, 1, 2 );
   optimizeLayout->addWidget( te_status,     row,   0, 2, 2 );
   row    += 2;

   QLabel* lb_optspace     = us_banner( "" );
   optimizeLayout->addWidget( lb_optspace,   row,   0, 1, 2 );
   optimizeLayout->setRowStretch( row, 2 );

   le_estmemory->setReadOnly( true );
   le_iteration->setReadOnly( true );
   le_oldvari  ->setReadOnly( true );
   le_newvari  ->setReadOnly( true );
   le_improve  ->setReadOnly( true );
   te_status   ->setReadOnly( true );
   QPalette gray = US_GuiSettings::editColor();
   gray.setColor( QPalette::Base, QColor( 0xe0, 0xe0, 0xe0 ) );
   le_estmemory->setPalette( gray );
   le_iteration->setPalette( gray );
   le_oldvari  ->setPalette( gray );
   le_newvari  ->setPalette( gray );
   le_improve  ->setPalette( gray );
   te_status   ->setPalette( gray );

   lb_constff0 ->setVisible( false );
   ct_constff0 ->setVisible( false );

   ck_unifgr->setChecked( true  );
   ck_iters ->setEnabled( true  );
   ct_iters ->setEnabled( false );

   optimize_options();

   connect( ck_unifgr, SIGNAL( toggled( bool ) ),
            this,  SLOT( checkUniGrid(  bool ) ) );
   connect( ck_menisc, SIGNAL( toggled( bool ) ),
            this,  SLOT( checkMeniscus( bool ) ) );
   connect( ck_mcarlo, SIGNAL( toggled( bool ) ),
            this,  SLOT( checkMonteCar( bool ) ) );
   connect( ck_iters,  SIGNAL( toggled( bool ) ),
            this,  SLOT( checkIterate(  bool ) ) );
   connect( ck_varvbar, SIGNAL( toggled( bool ) ),
            this,  SLOT( checkVaryVbar( bool ) ) );

   connect( ct_nstepss,  SIGNAL( valueChanged( double ) ),
            this,        SLOT(   grid_change()          ) );
   connect( ct_nstepsk,  SIGNAL( valueChanged( double ) ),
            this,        SLOT(   grid_change()          ) );
   connect( ct_grrefine, SIGNAL( valueChanged( double ) ),
            this,        SLOT(   grid_change()          ) );
   connect( ct_thrdcnt,  SIGNAL( valueChanged( double ) ),
            this,        SLOT(   grid_change()          ) );
   connect( ct_lolimits, SIGNAL( valueChanged( double ) ),
            this,        SLOT(   slim_change()          ) );
   connect( ct_uplimits, SIGNAL( valueChanged( double ) ),
            this,        SLOT(   slim_change()          ) );
   connect( ct_lolimitk, SIGNAL( valueChanged( double ) ),
            this,        SLOT(   klim_change()          ) );
   connect( ct_nstepsk,  SIGNAL( valueChanged( double ) ),
            this,        SLOT(   kstep_change()         ) );

   connect( pb_strtfit, SIGNAL( clicked()   ),
            this,       SLOT(   start()     ) );
   connect( pb_stopfit, SIGNAL( clicked()   ),
            this,       SLOT(   stop_fit()  ) );
   connect( pb_plot,    SIGNAL( clicked()   ),
            this,       SLOT(   plot()      ) );
   connect( pb_save,    SIGNAL( clicked()   ),
            this,       SLOT(   save()      ) );
   connect( pb_help,    SIGNAL( clicked()   ),
            this,       SLOT(   help()      ) );
   connect( pb_close,   SIGNAL( clicked()   ),
            this,       SLOT(   close_all() ) );
   connect( pb_advance, SIGNAL( clicked()   ),
            this,       SLOT(   advanced()  ) );

   edata          = &dsets[ 0 ]->run_data;

   grid_change();

//DbgLv(2) << "Pre-resize AC size" << size();
   resize( 710, 440 );
//DbgLv(2) << "Post-resize AC size" << size();
}

// enable/disable optimize counters based on chosen method
void US_AnalysisControl::optimize_options()
{
   ct_grrefine->setEnabled( ck_unifgr->isChecked() );
   ct_menisrng->setEnabled( ck_menisc->isChecked() );
   ct_menispts->setEnabled( ck_menisc->isChecked() );
   ct_mciters ->setEnabled( ck_mcarlo->isChecked() );

   ck_tinoise ->setEnabled( ! ck_mcarlo->isChecked() );
   ck_rinoise ->setEnabled( ! ck_mcarlo->isChecked() );

   if ( ck_mcarlo->isChecked() )
   {
      ck_tinoise ->setChecked( false );
      ck_rinoise ->setChecked( false ); 
   }

   ct_menisrng->adjustSize();
   adjustSize();
}

// uncheck optimize options other than one just checked
void US_AnalysisControl::uncheck_optimize( int ckflag )
{
   if ( ckflag >  3 ) ck_unifgr->setChecked( false );
   if ( ckflag == 3 ) ck_menisc->setChecked( false );
   if ( ckflag == 2 ) ck_mcarlo->setChecked( false );
}

// handle uniform grid checked
void US_AnalysisControl::checkUniGrid(  bool checked )
{
   if ( checked )
      uncheck_optimize( 1 );
   
   optimize_options();
   
   ct_lolimits->setEnabled( checked );
   ct_uplimits->setEnabled( checked );
   ct_nstepss ->setEnabled( checked );
   ct_lolimitk->setEnabled( checked );
   ct_uplimitk->setEnabled( checked );
   ct_nstepsk ->setEnabled( checked );
   ct_thrdcnt ->setEnabled( checked );
   ck_tinoise ->setEnabled( checked );
   ck_rinoise ->setEnabled( checked );
   if ( checked )
   {
      int nthr     = US_Settings::threads();
      nthr         = ( nthr > 1 ) ? nthr : QThread::idealThreadCount();
      ct_thrdcnt ->setValue  ( nthr    );
   }
}

// handle float meniscus position checkec
void US_AnalysisControl::checkMeniscus( bool checked )
{
   if ( checked )
      uncheck_optimize( 2 );

   optimize_options();
}

// handle Monte Carlo checked
void US_AnalysisControl::checkMonteCar( bool checked )
{
   if ( checked )
      uncheck_optimize( 3 );

   optimize_options();
}

// handle local uniform grid checked
void US_AnalysisControl::checkLocalUni( bool checked )
{
   if ( checked ) { uncheck_optimize( 4 ); optimize_options(); }
}

// handle random local grid checked
void US_AnalysisControl::checkRandLoc(  bool checked )
{
   if ( checked ) { uncheck_optimize( 5 ); optimize_options(); }
}

// handle solute coalescing checked
void US_AnalysisControl::checkSoluCoal( bool checked )
{
   if ( checked ) { uncheck_optimize( 6 ); optimize_options(); }
}

// handle clip lowest conc. solute checked
void US_AnalysisControl::checkClipLow(  bool checked )
{
   if ( checked ) { uncheck_optimize( 7 ); optimize_options(); }
}

// handle Regularization checked
void US_AnalysisControl::checkRegular(  bool checked )
{
   if ( checked ) { uncheck_optimize( 8 ); optimize_options(); }
}

// handle iterations checked
void US_AnalysisControl::checkIterate(  bool checked )
{
   ct_iters->setEnabled( checked );
   ct_iters->setValue( ( checked ? 3 : 1 ) );
}

// handle vary-vbar checked
void US_AnalysisControl::checkVaryVbar(  bool checked )
{
   lb_constff0->setVisible( checked );
   ct_constff0->setVisible( checked );

   if ( checked )
   {
      double vblo = dsets[ 0 ]->vbar20 - 0.02;
      double vbhi = dsets[ 0 ]->vbar20 + 0.02;
      vblo        = (double)( (int)( vblo * 1000.0 )     ) * 0.001;
      vbhi        = (double)( (int)( vbhi * 1000.0 ) + 1 ) * 0.001;
      lb_lolimitk->setText( tr( "Lower Limit (vbar):" ) );
      lb_uplimitk->setText( tr( "Upper Limit (vbar):" ) );
      lb_nstepsk ->setText( tr( "Number Grid Points (vbar):" ) );
      ct_lolimitk->setMinValue( 0.025 );
      ct_lolimitk->setMaxValue( 1.500 );
      ct_lolimitk->setStep    ( 0.001 );
      ct_lolimitk->setValue   ( vblo  );
      ct_uplimitk->setMinValue( 0.025 );
      ct_uplimitk->setMaxValue( 1.500 );
      ct_uplimitk->setStep    ( 0.001 );
      ct_uplimitk->setValue   ( vbhi  );
      ct_constff0->setValue   ( 2.000 );
   }

   else
   {
      lb_lolimitk->setText( tr( "Lower Limit (f/f0):" ) );
      lb_uplimitk->setText( tr( "Upper Limit (f/f0):" ) );
      lb_nstepsk ->setText( tr( "Number Grid Points (f/f0):" ) );
      ct_lolimitk->setMinValue( 1.0 );
      ct_lolimitk->setMaxValue( 8.0 );
      ct_lolimitk->setStep    ( 0.01 );
      ct_lolimitk->setValue   ( 1.0 );
      ct_uplimitk->setMinValue( 1.0 );
      ct_uplimitk->setMaxValue( 8.0 );
      ct_uplimitk->setStep    ( 0.01 );
      ct_uplimitk->setValue   ( 4.0 );
   }
}

// start fit button clicked
void US_AnalysisControl::start()
{
   if ( parentw )
   {  // Get pointers to needed objects from the main
      US_2dsa* mainw = (US_2dsa*)parentw;
      edata          = mainw->mw_editdata();
      sdata          = mainw->mw_simdata();
      rdata          = mainw->mw_resdata();
      model          = mainw->mw_model();
      ti_noise       = mainw->mw_ti_noise();
      ri_noise       = mainw->mw_ri_noise();
      mw_stattext    = mainw->mw_status_text();

      mainw->analysis_done( -1 );   // reset counters to zero
DbgLv(1) << "AnaC: edata scans" << edata->scanData.size();
   }

   // Make sure that ranges are reasonable
   if ( ( ct_uplimits->value() - ct_lolimits->value() ) < 0.0  ||
        ( ct_uplimitk->value() - ct_lolimitk->value() ) < 0.0 )
   {
      QString msg = 
         tr( "The \"s\" or \"f/f0\" ranges are inconsistent.\n"
             "Please re-check the limits and correct them\n"
             "before again clicking \"Start Fit\"." );

      if ( ck_varvbar->isChecked() )
         msg = msg.replace( "f/f0", "vbar" );

      QMessageBox::critical( this, tr( "Limits Inconsistent!" ), msg );
      return;
   }

   // Make sure that any fit-meniscus is reasonable
   if ( ck_menisc->isChecked() )
   {
      double menrng = ct_menisrng->value();
      double bmenis = edata->meniscus;
      double hmenis = bmenis + menrng * 0.5;
      double lrdata = edata->x[ 0 ].radius;

      if ( hmenis >= lrdata )
      {
         QMessageBox::critical( this, tr( "Meniscus-Data Overlap!" ),
            tr( "The highest meniscus (%1), implied in the range given,\n"
                "equals or exceeds the low data range radius (%2).\n\n"
                "You must either quit this program and re-edit the data\n"
                "to have a low radius value farther from the meniscus;\n"
                "or change the fit-meniscus range given here." )
                .arg( hmenis ).arg( lrdata ) );
         return;
      }
   }

   // Start a processing object if need be
   if ( processor == 0 )
      processor   = new US_2dsaProcess( dsets, this );

   else
      processor->disconnect();

   // Set up for the start of fit processing
   le_iteration->setText( "0" );
   le_oldvari  ->setText( "0.000e-05" );
   le_newvari  ->setText( "0.000e-05" );
   le_improve  ->setText( "0.000e-08" );

   double slo    = ct_lolimits->value();
   double sup    = ct_uplimits->value();
   int    nss    = (int)ct_nstepss->value();
   double klo    = ct_lolimitk->value();
   double kup    = ct_uplimitk->value();
   int    nks    = (int)ct_nstepsk->value();
   int    ngrr   = (int)ct_grrefine->value();
   int    nthr   = (int)ct_thrdcnt->value();
   int    noif   = ( ck_tinoise->isChecked() ? 1 : 0 ) +
                   ( ck_rinoise->isChecked() ? 2 : 0 );
   ti_noise->values.clear();
   ri_noise->values.clear();
   ti_noise->count = 0;
   ri_noise->count = 0;

   ngrr            = ( grtype < 0 ) ? grtype : ngrr;

   nctotal         = 10000;

   connect( processor, SIGNAL( progress_update(   int ) ),
            this,      SLOT(   update_progress(   int ) ) );
   connect( processor, SIGNAL( message_update(    QString, bool ) ),
            this,      SLOT(   progress_message(  QString, bool ) ) );
   connect( processor, SIGNAL( stage_complete(    int, int )  ),
            this,      SLOT(   reset_steps(       int, int )  ) );
   connect( processor, SIGNAL( process_complete(  int  ) ),
            this,      SLOT(   completed_process( int  ) ) );

   int    mxiter = (int)ct_iters->value();
   int    mniter = ck_menisc->isChecked() ?
                   (int)ct_menispts->value() : 0;
   int    mciter = ck_mcarlo->isChecked() ?
                   (int)ct_mciters ->value() : 0;
   double vtoler = 1.0e-12;
   double menrng = ct_menisrng->value();
   double cff0   = ck_varvbar->isChecked() ? ct_constff0->value() : 0.0;

   // Begin the fit
   processor->set_iters( mxiter, mciter, mniter, vtoler, menrng, cff0 );

   processor->start_fit( slo, sup, nss, klo, kup, nks,
         ngrr, nthr, noif );

   pb_strtfit->setEnabled( false );
   pb_stopfit->setEnabled( true  );
   pb_plot   ->setEnabled( false );
   pb_save   ->setEnabled( false );
}

// stop fit button clicked
void US_AnalysisControl::stop_fit()
{
DbgLv(1) << "AC:SF:StopFit";
   if ( processor != 0 )
   {
DbgLv(1) << "AC:SF: processor stopping...";
      processor->stop_fit();
DbgLv(1) << "AC:SF: processor stopped";
   }

   //delete processor;
   processor = 0;
DbgLv(1) << "AC:SF: processor deleted";

   qApp->processEvents();
   b_progress->reset();

   pb_strtfit->setEnabled( true  );
   pb_stopfit->setEnabled( false );
   pb_plot   ->setEnabled( false );
   pb_save   ->setEnabled( false );
   qApp->processEvents();

   if ( parentw )
   {
      US_2dsa* mainw = (US_2dsa*)parentw;
      mainw->analysis_done( -1 );
   }

   qApp->processEvents();
}

// plot button clicked
void US_AnalysisControl::plot()
{
   US_2dsa* mainw = (US_2dsa*)parentw;
   mainw->analysis_done( 1 );
}

// save button clicked
void US_AnalysisControl::save()
{
   US_2dsa* mainw = (US_2dsa*)parentw;
   mainw->analysis_done( 2 );
}

// Close button clicked
void US_AnalysisControl::close_all()
{
   close();
}

// Reset memory estimate when grid steps, threads or repetitions changes
void US_AnalysisControl::grid_change()
{
   int    nsteps = (int)ct_nstepss ->value();         // # steps s
   int    nstepk = (int)ct_nstepsk ->value();         // # steps k
   int    ngrrep = (int)ct_grrefine->value();         // # repetitions
   int    nthrd  = (int)ct_thrdcnt ->value();         // # threads
   int    ngstep = nsteps * nstepk;                   // # grid steps
   int    nsstep = ( nsteps / ngrrep + 1 )
                 * ( nstepk / ngrrep + 1 );           // # subgrid steps
   int    nscan  = edata->scanData.size();            // # scans
   int    nconc  = edata->x.size();                   // # concentrations
   int    ntconc = nconc * nscan;                     // # total readings
   int    szread = sizeof( US_DataIO2::Reading ) * ntconc;
   int    szscan = sizeof( US_DataIO2::Scan ) * nscan;
   int    szedat = sizeof( US_DataIO2::EditedData );
   int    szsol  = sizeof( US_Solute );               // size Solute
   int    szval  = sizeof( double );                  // size vector value
   long   szgso  = ngstep * szsol;                    // size grid solutes
   long   szsso  = nsstep * szsol * nthrd;            // size subg solutes
   long   szdat  = szread + szscan + szedat;          // size data
   long   szmat  = szval * ( ntconc + 2 );            // size matrix
   if ( ck_tinoise->isChecked() || ck_rinoise->isChecked() )
      szmat        *= 2L;
DbgLv(1) << "GC: ngst nsst ngrr nthr" << ngstep << nsstep << ngrrep << nthrd;
DbgLv(1) << "GC:  szsol szval szgso szsso szmat szdat"
 << szsol << szval << szgso << szsso << szmat << szdat;
   nsstep        = ( ntconc < 50000 ) ?
                   qMax( nsstep, 100 ) :
                   qMax( nsstep,  80 );
   double stepf  = (double)( nsstep * nthrd );
   double mbase  = (double)US_Memory::rss_now() / 1024.0;
   double megas  = sq( 1024.0 );
   double mgrid  = (double)szgso * 1.0 / megas;
   double msubg  = (double)szsso * 1.0 / megas;
   double mmatr  = (double)szmat * 1.1 * stepf / megas;
   double mdata  = (double)szdat * 1.6 * stepf / megas;
   int    megs   = qRound( mbase + mgrid + msubg + mmatr + mdata );
DbgLv(1) << "GC:  mbase mgrid msubg mmatr mdata"
 << mbase << mgrid << msubg << mmatr << mdata << " megs" << megs;

   le_estmemory->setText( QString::number( megs ) + " MB" );
}

// Reset s-limit step sizes when s-limit value changes
void US_AnalysisControl::slim_change()
{
   double maglim = qAbs( ct_uplimits->value() - ct_lolimits->value() );
   double lostep = ct_lolimits->step();
   double upstep = ct_uplimits->step();

   if ( maglim > 50.0 )
   {
      if ( lostep != 10.0  ||  upstep != 10.0 )
      {
         ct_lolimits->setStep( 10.0 );
         ct_uplimits->setStep( 10.0 );
      }
   }

   else
   {
      if ( lostep != 0.1  ||  upstep != 0.1 )
      {
         ct_lolimits->setStep( 0.1 );
         ct_uplimits->setStep( 0.1 );
      }
   }
}


// Set k-upper-limit to lower when k grid points == 1
void US_AnalysisControl::klim_change()
{
   if ( ct_nstepsk->value() == 1.0 )
   {
      ct_uplimitk->setValue( ct_lolimitk->value() );
   }
}

// Test for k-steps==1 when k-step value changes
void US_AnalysisControl::kstep_change()
{
   if ( ct_nstepsk->value() == 1.0 )
   {  // Set up for C(s) parameters
      ct_uplimitk->setValue( ct_lolimitk->value() );
      ct_thrdcnt ->setValue( 1.0 );
      ct_grrefine->setValue( 1.0 );
      ct_uplimitk->setEnabled( false );
      ct_thrdcnt ->setEnabled( false );
      ct_grrefine->setEnabled( false );
   }

   else if ( ct_uplimitk->value() == ct_lolimitk->value() )
   {  // Set up for normal 2dsa parameters
      ct_uplimitk->setValue( 4.0 );
      int nthr     = US_Settings::threads();
      nthr         = ( nthr > 1 ) ? nthr : QThread::idealThreadCount();
      ct_thrdcnt ->setValue( nthr );
      ct_grrefine->setValue( 6.0  );
      ct_uplimitk->setEnabled( true );
      ct_thrdcnt ->setEnabled( true );
      ct_grrefine->setEnabled( true );
   }
}

// slot to handle progress update
void US_AnalysisControl::update_progress( int ksteps )
{
   ncsteps += ksteps;

   if ( ncsteps > nctotal )
   {
      nctotal  = ( nctotal * 11 ) / 10;
      b_progress->setMaximum( nctotal );
   }

   b_progress->setValue( ncsteps );
DbgLv(2) << "UpdPr: ks ncs nts" << ksteps << ncsteps << nctotal;
}

// slot to handle updated progress message
void US_AnalysisControl::progress_message( QString pmsg, bool append )
{
   QString amsg;

   if ( append )
   {  // append to existing progress message
      amsg   = te_status->toPlainText() + "\n" + pmsg;
   }

   else
   {  // create a new progress message
      amsg   = pmsg;
   }

   mw_stattext->setText( amsg );
   te_status  ->setText( amsg );

   qApp->processEvents();
}

// Slot to handle resetting progress
void US_AnalysisControl::reset_steps( int kcs, int nct )
{
DbgLv(1) << "AC:cs: prmx nct kcs" << b_progress->maximum() << nct << kcs;
   ncsteps      = kcs;
   nctotal      = nct;

   b_progress->setMaximum( nctotal );
   b_progress->setValue(   ncsteps );

   qApp->processEvents();
}

// slot to handle completed processing
void US_AnalysisControl::completed_process( int stage )
{
   bool alldone = ( stage == 9 );
DbgLv(1) << "AC:cp: stage alldone" << stage << alldone;

   b_progress->setValue( nctotal );
   qApp->processEvents();

   processor->get_results( sdata, rdata, model, ti_noise, ri_noise );
DbgLv(1) << "AC:cp: RES: ti,ri counts" << ti_noise->count << ri_noise->count;

   US_DataIO2::Scan* rscan0 = &rdata->scanData[ 0 ];
   int    iternum  = (int)rscan0->rpm;
   int    mmitnum  = (int)rscan0->seconds;
   double varinew  = rscan0->delta_r;
   double meniscus = rscan0->plateau;
   double variold  = le_newvari  ->text().toDouble();
   double vimprov  = variold - varinew;
   le_oldvari  ->setText( QString::number( variold ) );
   le_newvari  ->setText( QString::number( varinew ) );
   le_improve  ->setText( QString::number( vimprov ) );

   if ( mmitnum == 0 )
   {  // simple refinement iteration (no MC/Meniscus)
      le_iteration->setText( QString::number( iternum ) );
      model->description = QString( "MMITER=%1 VARI=%2 " )
                           .arg( mmitnum ).arg( varinew );
   }

   else if ( ck_menisc->isChecked() )
   {  // Meniscus
      model->global      = US_Model::MENISCUS;
      model->description = QString( "MMITER=%1 VARI=%2 MENISCUS=%3" )
                           .arg( mmitnum ).arg( varinew ).arg( meniscus );
      le_iteration->setText( QString::number( iternum  ) + "   ( Model " +
                             QString::number( mmitnum  ) + " , Meniscus " +
                             QString::number( meniscus ) + " )" );
   }

   else
   {  // Monte Carlo
      model->monteCarlo  = true;
      model->description = QString( "MMITER=%1 VARI=%2 " )
                           .arg( mmitnum ).arg( varinew );
      le_iteration->setText( QString::number( iternum ) + "   ( MC Model " +
                             QString::number( mmitnum ) + " )" );
   }

   US_2dsa* mainw = (US_2dsa*)parentw;

   if ( alldone )
   {
      mainw->analysis_done( -2 );

      mainw->analysis_done( ck_autoplt->isChecked() ? 1 : 0 );

      pb_strtfit->setEnabled( true  );
      pb_stopfit->setEnabled( false );
      pb_plot   ->setEnabled( true  );
      pb_save   ->setEnabled( true  );
   }

   else if ( mmitnum > 0  &&  stage > 0 )
   {  // signal main to update lists of models,noises
      mainw->analysis_done( -2 );
   }
}

// slot to handle advanced analysis controls
void US_AnalysisControl::advanced()
{
   US_SimulationParameters* sparms = &dsets[ 0 ]->simparams;
DbgLv(1) << "Adv sparms.bf sect" << sparms->band_forming << sparms->cp_sector;

   US_AdvAnalysis* aadiag = new US_AdvAnalysis( sparms, this );
   if ( aadiag->exec() == QDialog::Accepted )
   {
             grtype = US_2dsaProcess::UGRID;
      double grpar1 = 0.0;
      double grpar2 = 0.0;
      double grpar3 = 0.0;
      bool   reg    = false;
      double repar1 = 0.0;
      US_Model modpar;

      aadiag->get_parameters( grtype, grpar1, grpar2, grpar3,
                              modpar, reg,    repar1 );
DbgLv(1) << "Adv ACCEPT";
DbgLv(1) << "Adv grtype par123" << grtype << grpar1 << grpar2 << grpar3;
DbgLv(1) << "Adv modpar size  " << modpar.components.size();
DbgLv(1) << "Adv reg    par1  " << reg    << repar1;
DbgLv(1) << "Adv BANDVOL"  << sparms->band_volume << " MUL" << sparms->cp_width;
      if ( grtype < 0 )
      {
         US_2dsa* mainw = (US_2dsa*)parentw;
         model          = mainw->mw_model();
         *model         = modpar;
         dsets[ 0 ]->model = modpar;
         int     nsol   = model->components.size();
         QString amsg   = ( grtype == (-1 )
            ? tr( "Grid from loaded model" )
            : tr( "Grid and signal ratios from loaded model\n" ) )
            + "  ( " + QString::number( nsol ) + tr( " solutes )" );
         te_status  ->setText( amsg );

         if ( nsol > 0 )
         {
            uncheck_optimize( 4 );
            optimize_options();
            ct_lolimits->setEnabled( false );
            ct_uplimits->setEnabled( false );
            ct_nstepss ->setEnabled( false );
            ct_lolimitk->setEnabled( false );
            ct_uplimitk->setEnabled( false );
            ct_nstepsk ->setEnabled( false );
            ct_thrdcnt ->setEnabled( false );
            ck_tinoise ->setEnabled( false );
            ck_rinoise ->setEnabled( false );
            ct_thrdcnt ->setValue  (  1    );
         }

         else
             grtype = US_2dsaProcess::UGRID;
      }
   }
else
DbgLv(1) << "Adv REJECT";

   qApp->processEvents();

   delete aadiag;
}

