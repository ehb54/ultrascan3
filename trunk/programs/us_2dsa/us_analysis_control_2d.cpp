//! \file us_analysis_control_2d.cpp

#include "us_2dsa.h"
#include "us_analysis_control_2d.h"
#include "us_adv_analysis_2d.h"
#include "us_settings.h"
#include "us_passwd.h"
#include "us_db2.h"
#include "us_gui_settings.h"
#include "us_memory.h"

#include <qwt_legend.h>

// constructor:  2dsa analysis controls widget
US_AnalysisControl2D::US_AnalysisControl2D( QList< SS_DATASET* >& dsets,
   bool& loadDB, QWidget* p ) : US_WidgetsDialog( p, 0 ),
   dsets( dsets ), loadDB( loadDB )
{
   parentw        = p;
   processor      = 0;
   dbg_level      = US_Settings::us_debug();
   grtype         = US_2dsaProcess::UGRID;
   baserss        = 0;

   setObjectName( "US_AnalysisControl2D" );
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
   QLabel* lb_gridreps     = us_label(  tr( "Grid Repetitions:" ) );
   QLabel* lb_menisrng     = us_label(  tr( "Meniscus Fit Range (cm):" ) );
   QLabel* lb_menispts     = us_label(  tr( "Meniscus Grid Points:" ) );
   QLabel* lb_mciters      = us_label(  tr( "Monte Carlo Iterations:" ) );
   QLabel* lb_iters        = us_label(  tr( "Maximum Iterations:" ) );
   QLabel* lb_statinfo     = us_banner( tr( "Status Information:" ) );

   pb_strtfit              = us_pushbutton( tr( "Start Fit" ),    true  );
   pb_stopfit              = us_pushbutton( tr( "Stop Fit" ),     false );
   pb_plot                 = us_pushbutton( tr( "Plot Results" ), false );
   pb_save                 = us_pushbutton( tr( "Save Results" ), false );
   pb_ldmodel              = us_pushbutton( tr( "Load Model"   ), false );
   QPushButton* pb_help    = us_pushbutton( tr( "Help" ) );
   QPushButton* pb_close   = us_pushbutton( tr( "Close" ) );
   QPushButton* pb_advance = us_pushbutton( tr( "Advanced Analysis Controls" ));
   te_status               = us_textedit();
   us_setReadOnly( te_status, true );

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
   ct_lolimits  = us_counter( 3, -10000, 10000,   1 );
   ct_uplimits  = us_counter( 3, -10000, 10000,  10 );
   ct_nstepss   = us_counter( 3,      1,  1000,  64 );
   ct_lolimitk  = us_counter( 3,      1,     8,   1 );
   ct_uplimitk  = us_counter( 3,      1,    20,   4 );
   ct_nstepsk   = us_counter( 3,      1,  1000,  64 );
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

   le_estmemory = us_lineedit( tr( "100 MB" ), -1, true );
   le_iteration = us_lineedit( "0",            -1, true );
   le_oldvari   = us_lineedit( "0.000e-05",    -1, true );
   le_newvari   = us_lineedit( "0.000e-05",    -1, true );
   le_improve   = us_lineedit( "0.000e-08",    -1, true );
   le_gridreps  = us_lineedit(
      tr( "8  ( -> 64 64-point subgrids )" ),  -1, true );

   b_progress   = us_progressBar( 0, 100, 0 );

   QLayout*  lo_iters   =
      us_checkbox( tr( "Use Iterative Method"    ), ck_iters,  false );
   QLayout*  lo_unifgr  =
      us_checkbox( tr( "Uniform Grid"            ), ck_unifgr, true  );
   QLayout*  lo_custgr  =
      us_checkbox( tr( "Custom Grid"             ), ck_custgr, false );
   QLayout*  lo_menisc  =
      us_checkbox( tr( "Float Meniscus Position" ), ck_menisc, false );
   QLayout*  lo_mcarlo  =
      us_checkbox( tr( "Monte Carlo Iterations"  ), ck_mcarlo, false );


   ct_iters     = us_counter( 2,    1,   16,    1 );
   ct_menisrng  = us_counter( 3, 0.01, 0.65, 0.03 );
   ct_menispts  = us_counter( 2,    3,   21,   10 );
   ct_mciters   = us_counter( 3,    3, 2000,   20 );
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
   QLabel* lb_optspace1    = us_banner( "" );
   controlsLayout->addWidget( lb_optspace1,  row,   0, 1, 4 );
   controlsLayout->setRowStretch( row, 2 );

   row           = 0;
   optimizeLayout->addWidget( lb_optimiz,    row++, 0, 1, 4 );
   optimizeLayout->addLayout( lo_unifgr,     row++, 0, 1, 4 );
   optimizeLayout->addWidget( lb_gridreps,   row,   0, 1, 1 );
   optimizeLayout->addWidget( le_gridreps,   row++, 1, 1, 3 );
   optimizeLayout->addLayout( lo_custgr,     row,   0, 1, 2 );
   optimizeLayout->addWidget( pb_ldmodel,    row++, 2, 1, 4 );
   optimizeLayout->addLayout( lo_menisc,     row++, 0, 1, 4 );
   optimizeLayout->addWidget( lb_menisrng,   row,   0, 1, 2 );
   optimizeLayout->addWidget( ct_menisrng,   row++, 2, 1, 2 );
   optimizeLayout->addWidget( lb_menispts,   row,   0, 1, 2 );
   optimizeLayout->addWidget( ct_menispts,   row++, 2, 1, 2 );
   optimizeLayout->addLayout( lo_mcarlo,     row++, 0, 1, 4 );
   optimizeLayout->addWidget( lb_mciters,    row,   0, 1, 2 );
   optimizeLayout->addWidget( ct_mciters,    row++, 2, 1, 2 );
   optimizeLayout->addWidget( pb_advance,    row++, 0, 1, 4 );
   optimizeLayout->addLayout( lo_iters,      row++, 0, 1, 4 );
   optimizeLayout->addWidget( lb_iters,      row,   0, 1, 2 );
   optimizeLayout->addWidget( ct_iters,      row++, 2, 1, 2 );
   optimizeLayout->addWidget( lb_statinfo,   row++, 0, 1, 4 );
   optimizeLayout->addWidget( te_status,     row,   0, 2, 4 );
   row    += 6;

   QLabel* lb_optspace     = us_banner( "" );
   optimizeLayout->addWidget( lb_optspace,   row,   0, 1, 2 );
   optimizeLayout->setRowStretch( row, 2 );

   lb_constff0 ->setVisible( false );
   ct_constff0 ->setVisible( false );

   ck_unifgr->setChecked( true  );
   ck_custgr->setChecked( false );
   ck_iters ->setEnabled( true  );
   ct_iters ->setEnabled( false );

   optimize_options();

   connect( ck_unifgr, SIGNAL( toggled( bool ) ),
            this,  SLOT( checkUniGrid(  bool ) ) );
   connect( ck_custgr, SIGNAL( toggled( bool ) ),
            this,  SLOT( checkCusGrid(  bool ) ) );
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
   connect( pb_ldmodel, SIGNAL( clicked()   ),
            this,       SLOT(  load_model() ) );
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
void US_AnalysisControl2D::optimize_options()
{
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
void US_AnalysisControl2D::uncheck_optimize( int ckflag )
{
   if ( ckflag >  3 ) ck_unifgr->setChecked( false );
   if ( ckflag == 3 ) ck_menisc->setChecked( false );
   if ( ckflag == 2 ) ck_mcarlo->setChecked( false );
}

// handle uniform grid checked
void US_AnalysisControl2D::checkUniGrid(  bool checked )
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
      ct_thrdcnt ->setValue  ( nthr );
      ck_varvbar ->setEnabled( true );
      ct_constff0->setEnabled( true );
      grtype       = US_2dsaProcess::UGRID;
   }

   ck_custgr ->disconnect();
   ck_custgr ->setChecked( ! checked );
   pb_ldmodel->setEnabled( ! checked );
   connect( ck_custgr, SIGNAL( toggled( bool ) ),
            this,  SLOT( checkCusGrid(  bool ) ) );
}

// Handle custom grid checked
void US_AnalysisControl2D::checkCusGrid(  bool checked )
{
   ck_unifgr ->setChecked( ! checked );
   ck_varvbar->setEnabled( ! checked );

   if ( checked )
   {
      ct_thrdcnt ->setEnabled( true  );
      ck_tinoise ->setEnabled( true );
      ck_rinoise ->setEnabled( true );
      ct_constff0->setEnabled( false );
   }
}

// handle float meniscus position checkec
void US_AnalysisControl2D::checkMeniscus( bool checked )
{
   if ( checked )
      uncheck_optimize( 2 );

   optimize_options();
}

// handle Monte Carlo checked
void US_AnalysisControl2D::checkMonteCar( bool checked )
{
   if ( checked )
   {
      uncheck_optimize( 3 );
   }

   optimize_options();
}

// handle local uniform grid checked
void US_AnalysisControl2D::checkLocalUni( bool checked )
{
   if ( checked ) { uncheck_optimize( 4 ); optimize_options(); }
}

// handle random local grid checked
void US_AnalysisControl2D::checkRandLoc(  bool checked )
{
   if ( checked ) { uncheck_optimize( 5 ); optimize_options(); }
}

// handle solute coalescing checked
void US_AnalysisControl2D::checkSoluCoal( bool checked )
{
   if ( checked ) { uncheck_optimize( 6 ); optimize_options(); }
}

// handle clip lowest conc. solute checked
void US_AnalysisControl2D::checkClipLow(  bool checked )
{
   if ( checked ) { uncheck_optimize( 7 ); optimize_options(); }
}

// handle Regularization checked
void US_AnalysisControl2D::checkRegular(  bool checked )
{
   if ( checked ) { uncheck_optimize( 8 ); optimize_options(); }
}

// handle iterations checked
void US_AnalysisControl2D::checkIterate(  bool checked )
{
   ct_iters->setEnabled( checked );
   ct_iters->setValue( ( checked ? 3 : 1 ) );
}

// handle vary-vbar checked
void US_AnalysisControl2D::checkVaryVbar(  bool checked )
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
      ct_lolimitk->setMinValue( 1.0  );
      ct_lolimitk->setMaxValue( 8.0  );
      ct_lolimitk->setStep    ( 0.01 );
      ct_lolimitk->setValue   ( 1.0  );
      ct_uplimitk->setMinValue( 1.0  );
      ct_uplimitk->setMaxValue( 20.0 );
      ct_uplimitk->setStep    ( 0.01 );
      ct_uplimitk->setValue   ( 4.0  );
   }
}

// start fit button clicked
void US_AnalysisControl2D::start()
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
      mw_baserss     = mainw->mw_base_rss();
      baserss        = *mw_baserss;

      if ( baserss == 0 )
      {
         baserss        = qRound( (double)US_Memory::rss_now() / 1024. );
         *mw_baserss    = baserss;
      }

      mainw->analysis_done( -1 );   // reset counters to zero
DbgLv(1) << "AnaC: edata scans, baserss" << edata->scanData.size() << baserss;
DbgLv(1) << "AnaC: edata" << edata;
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
      double lrdata = edata->xvalues[ 0 ];

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

   // Insure that max RPM and S-value imply a reasonable grid size
   double       s_max = ct_uplimits->value() * 1e-13;
   QString      smsg;

   if ( US_SolveSim::checkGridSize( dsets, s_max, smsg ) )
   {
      QMessageBox::critical( this,
         tr( "Implied Grid Size is Too Large!" ),
         smsg );
      return;
   }

DbgLv(1) << "AnaC:St:MEM (1)rssnow,proc" << US_Memory::rss_now() << processor;
   // Start a processing object if need be
   if ( processor == 0 )
      processor   = new US_2dsaProcess( dsets, this );

   else
   {
      processor->disconnect();
      processor->stop_fit();
      processor->clear_data();
   }
DbgLv(1) << "AnaC:St:MEM (2)rssnow" << US_Memory::rss_now();

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
   int    nthr   = (int)ct_thrdcnt->value();
   int    noif   = ( ck_tinoise->isChecked() ? 1 : 0 ) +
                   ( ck_rinoise->isChecked() ? 2 : 0 );

   int    ngrr   = ( grtype < 0 ) ? grtype
                 : US_Math2::best_grid_reps( nss, nks );

   ct_nstepss->setValue( nss );
   ct_nstepsk->setValue( nks );

   ti_noise->values.clear();
   ri_noise->values.clear();
   ti_noise->count = 0;
   ri_noise->count = 0;

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
   nctotal       = 10000;

   // Check memory and possibly abort fit if too much needed
   if ( memory_check() != 0 )
      return;

   // Begin the fit
   processor->set_iters( mxiter, mciter, mniter, vtoler, menrng, cff0, ngrr );

   processor->start_fit( slo, sup, nss, klo, kup, nks,
         ngrr, nthr, noif );

   pb_strtfit->setEnabled( false );
   pb_stopfit->setEnabled( true  );
   pb_plot   ->setEnabled( false );
   pb_save   ->setEnabled( false );
}

// stop fit button clicked
void US_AnalysisControl2D::stop_fit()
{
DbgLv(1) << "AC:SF:StopFit";
   if ( processor != 0 )
   {
DbgLv(1) << "AC:SF: processor stopping...";
      processor->disconnect();
      processor->stop_fit();
DbgLv(1) << "AC:SF: processor stopped";
      delete processor;
DbgLv(1) << "AC:SF: processor deleted";
   }

   //delete processor;
   processor = 0;
//DbgLv(1) << "AC:SF: processor deleted";

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

// Load Model button clicked
void US_AnalysisControl2D::load_model()
{
   QString  mdesc( "" );
   QString  mfilter( "" );
   US_Model cusmodel;
   US_2dsa* mainw  = NULL;

   if ( parentw )
   {
      mainw          = (US_2dsa*)parentw;
      mfilter        = QString( "CustomGrid" );
      mainw->analysis_done( -1 );
   }

   US_ModelLoader dialog( loadDB, mfilter, cusmodel, mdesc, "" );

   if ( dialog.exec() == QDialog::Accepted )
   {
      int     nsol   = cusmodel.components.size();
      int     nsubg  = cusmodel.subGrids;
      int     sgsize = nsol / nsubg;

      if ( sgsize > 150 )
      {  // Implied subgrid size too large:  change subgrid count
         int ksubg      = nsubg;
         int kssiz      = sgsize;
         nsubg          = ( nsol / 100 + 1 ) | 1;
         sgsize         = nsol / nsubg;
         DbgLv(0) << "Subgrid count adjusted from" << ksubg << "to" << nsubg;
         DbgLv(0) << "Subgrid size adjusted from" << kssiz << "to" << sgsize;
         cusmodel.subGrids = nsubg;
      }

      QString amsg   =
         tr( "Grid from loaded model\n  ( " )
         + QString::number( nsol ) + tr( " solutes, " )
         + QString::number( nsubg ) + tr( " subgrids )" );
      te_status  ->setText( amsg );

      if ( parentw )
      {
         model          = mainw->mw_model();
         *model         = cusmodel;
      }

      dsets[ 0 ]->model = cusmodel;
      grtype            = -1;
      bool cnst_ff0     = ! cusmodel.constant_vbar();
      ck_varvbar ->setChecked( cnst_ff0 );

      if ( cnst_ff0 )
         ct_constff0->setValue( cusmodel.components[ 0 ].f_f0 );
   }
}

// plot button clicked
void US_AnalysisControl2D::plot()
{
   US_2dsa* mainw = (US_2dsa*)parentw;
   mainw->analysis_done( 1 );
}

// save button clicked
void US_AnalysisControl2D::save()
{
   US_2dsa* mainw = (US_2dsa*)parentw;
   mainw->analysis_done( 2 );
}

// Close button clicked
void US_AnalysisControl2D::close_all()
{
   close();
}

// Reset memory estimate when grid steps, threads or repetitions changes
void US_AnalysisControl2D::grid_change()
{
   int    nsteps = (int)ct_nstepss ->value();         // # steps s
   int    nstepk = (int)ct_nstepsk ->value();         // # steps k
   int    nthrd  = (int)ct_thrdcnt ->value();         // # threads
   int    nscan  = edata->scanCount();                // # scans
   int    nconc  = edata->pointCount();               // # concentrations
   int    ntconc = nconc * nscan;                     // # total readings
   double megas  = sq( 1024.0 );

   int    ngrrep = US_Math2::best_grid_reps( nsteps, nstepk );

   int    ngstep = nsteps * nstepk;                   // # grid steps

   if ( parentw )
   {  // Get the starting base rss memory of this dataset and parameters
      US_2dsa* mainw = (US_2dsa*)parentw;
      mw_baserss     = mainw->mw_base_rss();
      baserss        = *mw_baserss;

      if ( baserss == 0 )
      {
         baserss        = qRound( (double)US_Memory::rss_now() / 1024. );
         *mw_baserss    = baserss;
      }
   }
   const double x_fact  = 17.20;
   const double y_fact  = 2.28;
   const int    nxdata  = 4;
   int    nsstep = ( nsteps / ngrrep ) * ( nstepk / ngrrep );
   int    noif   = ( ck_tinoise->isChecked() ? 1 : 0 ) +
                   ( ck_rinoise->isChecked() ? 2 : 0 );
   int    ndatas = nsstep + nxdata + noif;
   double mdata  = ( (double)ntconc * ndatas * sizeof( double ) ) / megas;
   double tdata  = x_fact + mdata * y_fact;
   memneed       = baserss + qRound( tdata * (double)nthrd );
DbgLv(1) << "GC:  baserss tdata mdata ndatas nthrd" << baserss
 << qRound(tdata) << qRound(mdata) << ndatas << nthrd << "memneed" << memneed;

   int memava, memtot;
   US_Memory::memory_profile( &memava, &memtot );

   le_estmemory->setText( tr( "%1 MB  (of %2 MB total real)" )
         .arg( memneed ).arg( memtot ) );

DbgLv(1) << "GC:  ngrrep nsteps nstepk" << ngrrep << nsteps << nstepk;

   // Output a message documenting the grid and subgrid dimensions
//   int nss       = nsteps / ngrrep;
//   int nsk       = nstepk / ngrrep;
   int nss       = nsteps / ngrrep;
   int nsk       = nstepk / ngrrep;
   int nspts     = nss * nsk;
   int nsubg     = sq( ngrrep );
   QString gmsg = tr( "Total grid is approximately %1 points (%2 x %3).\n" )
      .arg( ngstep ).arg( nsteps ).arg( nstepk );
   gmsg += tr( "Subgrid repetitions is %1 subgrids (%2 ^ 2)\n"
               "  with a maximum of %3 points each (%4 x %5)." )
      .arg( nsubg ).arg( ngrrep ).arg( nspts ).arg( nss ).arg( nsk );
   te_status  ->setText( gmsg );
   le_gridreps->setText( tr( "%1   ( -> %2 %3-point subgrids )" )
         .arg( ngrrep ).arg( nsubg ).arg( nspts ) );
}

// Adjust s-limit ranges when s-limit value changes
void US_AnalysisControl2D::slim_change()
{
   double loval  = ct_lolimits->value();
   double upval  = ct_uplimits->value();
   double limlo  = -1.e6;
   double limup  = 1.e6;
   if ( loval > upval )   // Insure lower value less than upper
      ct_lolimits->setValue( upval );

   if ( loval < -1.e5  ||  upval > 1.e5 )
   {  // For larger magnitudes, adjust ranges
      if ( loval < -1.e6  ||  upval > 1.e6 )
      {
         if ( loval < -1.e7  ||  upval > 1.e7 )
         {
            limlo         = -1.e19;
            limup         = 1.e19;
         }
         else
         {
            limlo         = -1.e8;
            limup         = 1.e8;
         }
      }
      else
      {
         limlo         = -1.e7;
         limup         = 1.e7;
      }
   }

   ct_lolimits->setRange( limlo, upval );
   ct_uplimits->setRange( loval, limup );
   ct_lolimits->setStep( 0.1 );
   ct_uplimits->setStep( 0.1 );
}

// Set k-upper-limit to lower when k grid points == 1
void US_AnalysisControl2D::klim_change()
{
   if ( ct_nstepsk->value() == 1.0 )
   {
      ct_uplimitk->setValue( ct_lolimitk->value() );
   }
}

// Test for k-steps==1 when k-step value changes
void US_AnalysisControl2D::kstep_change()
{
   if ( ct_nstepsk->value() == 1.0 )
   {  // Set up for C(s) parameters
      ct_uplimitk->setValue( ct_lolimitk->value() );
      ct_thrdcnt ->setValue( 1.0 );
      ct_uplimitk->setEnabled( false );
      ct_thrdcnt ->setEnabled( false );
   }

   else if ( ct_uplimitk->value() == ct_lolimitk->value() )
   {  // Set up for normal 2dsa parameters
      ct_uplimitk->setValue( 4.0 );
      int nthr     = US_Settings::threads();
      nthr         = ( nthr > 1 ) ? nthr : QThread::idealThreadCount();
      ct_thrdcnt ->setValue( nthr );
      ct_uplimitk->setEnabled( true );
      ct_thrdcnt ->setEnabled( true );
   }
}

// slot to handle progress update
void US_AnalysisControl2D::update_progress( int ksteps )
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
void US_AnalysisControl2D::progress_message( QString pmsg, bool append )
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
void US_AnalysisControl2D::reset_steps( int kcs, int nct )
{
DbgLv(1) << "AC:cs: prmx nct kcs" << b_progress->maximum() << nct << kcs;
   ncsteps      = kcs;
   nctotal      = nct;

   b_progress->setMaximum( nctotal );
   b_progress->setValue(   ncsteps );

   qApp->processEvents();
}

// slot to handle completed processing
void US_AnalysisControl2D::completed_process( int stage )
{
   bool alldone = ( stage == 9 );
DbgLv(1) << "AC:cp: stage alldone" << stage << alldone;

   b_progress->setValue( nctotal );
   qApp->processEvents();

   if ( stage == 6 )
   {  // If stopped because of memory usage, execute Stop Fit
      stop_fit();
      return;
   }

   processor->get_results( sdata, rdata, model, ti_noise, ri_noise );
DbgLv(1) << "AC:cp: RES: ti,ri counts" << ti_noise->count << ri_noise->count;
//DBG-DATA
#if 1
if (dbg_level>0)
{
 double dtot=0.0;
 double ntot=0.0;
 double stot=0.0;
 int nnoi=ti_noise->count;
 int knoi=ti_noise->values.count();
 for (int ii=0; ii<edata->scanCount(); ii++ )
  for (int jj=0; jj<edata->pointCount(); jj++ )
  {
   dtot += edata->value(ii,jj);
   stot += sdata->value(ii,jj);
  }
 for (int jj=0; jj<knoi; jj++ )
  ntot += ti_noise->values[jj];
 DbgLv(1) << "AC:cp DTOT" << dtot << "edata" << edata << "NTOT" << ntot
  << "nnoi knoi" << nnoi << knoi << "STOT" << stot;
}
#endif
//DBG-DATA

   US_DataIO::Scan* rscan0 = &rdata->scanData[ 0 ];
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

   if ( ck_custgr->isChecked() )
      model->description = model->description + QString( " CUSTOMGRID" );

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
void US_AnalysisControl2D::advanced()
{
   US_SimulationParameters* sparms = &dsets[ 0 ]->simparams;
DbgLv(1) << "Adv sparms.bf sect" << sparms->band_forming << sparms->cp_sector;

   US_AdvAnalysis2D* aadiag = new US_AdvAnalysis2D( sparms, loadDB, this );
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
         int     nsol   = modpar.components.size();
         int     nsubg  = modpar.subGrids;
         int     sgsize = nsol / nsubg;

         if ( sgsize > 150 )
         {  // Implied subgrid size too large:  change subgrid count
            int ksubg      = nsubg;
            int kssiz      = sgsize;
            nsubg          = ( nsol / 100 + 1 ) | 1;
            sgsize         = nsol / nsubg;
            DbgLv(0) << "Subgrid count adjusted from" << ksubg << "to" << nsubg;
            DbgLv(0) << "Subgrid size adjusted from" << kssiz << "to" << sgsize;
            modpar.subGrids = nsubg;
         }

         QString amsg   = ( grtype == (-1 )
            ? tr( "Grid from loaded model\n  ( " )
            : tr( "Grid and signal ratios from loaded model\n\n  ( " ) )
            + QString::number( nsol ) + tr( " solutes, " )
            + QString::number( nsubg ) + tr( " subgrids )" );
         te_status  ->setText( amsg );

         US_2dsa* mainw = (US_2dsa*)parentw;
         model          = mainw->mw_model();
         *model         = modpar;
         dsets[ 0 ]->model = modpar;

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
            ct_thrdcnt ->setEnabled( true  );
            ck_varvbar ->setEnabled( false );
            ck_varvbar ->setChecked( ! model->constant_vbar() );
            ct_constff0->setEnabled( false );
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

// Output warning if need be about memory needs, return continue flag
int US_AnalysisControl2D::memory_check( )
{
   const int pc_ava = 90;
   int status   = 0;
   int memava, memtot, memuse;
   int mempca   = US_Memory::memory_profile( &memava, &memtot, &memuse );
   int memsafe  = ( memava * pc_ava ) / 100;

   if ( memneed > memsafe  ||  mempca < 20 )
   {
      QString title   = tr( "High Memory Usage" );
      QString memp    = tr( "\n\nMemory Profile --\n"
                            "  Total:  %1 MB\n"
                            "  Available:  %2 MB\n"
                            "  Used:  %3 MB\n"
                            "  Estimated Need:  %4 MB\n\n" )
         .arg( memtot ).arg( memava ).arg( memuse ).arg( memneed );

      if ( memneed > memtot )
      {
         QMessageBox::critical( this, title,
             tr( "Memory needed for this fit exceeds total available." )
             + memp + tr( "This fit will not proceed.\n"
                          "Re-parameterize the fit with adjusted\n"
                          "Grid Refinements and/or Thread Count." ) );
         status          = 1;
      }

      else
      {
         QMessageBox msgBox( this );
         msgBox.setWindowTitle( title );
         msgBox.setText( tr( "Memory needed for this fit is a\n"
                             "high percentage of the available memory." )
                        + memp +
                         tr( "You may proceed if you wish (\"Yes\")\n"
                             "Or you may stop this fit (\"No\")\n"
                             "then re-parameterize the fit with adjusted\n"
                             "Grid Refinements and/or Thread Count.\n\nProceed?" ) );
         msgBox.addButton( QMessageBox::No );
         msgBox.addButton( QMessageBox::Yes );
         msgBox.setDefaultButton( QMessageBox::No );

         if ( msgBox.exec() == QMessageBox::No )
            status          = 2;
      }
   }

   return status;
}

