//! \file us_analysis_control.cpp

#include "us_2dsa.h"
#include "us_analysis_control.h"
#include "us_adv_analysis.h"
#include "us_settings.h"
#include "us_gui_settings.h"

#include <qwt_legend.h>

// constructor:  2dsa analysis controls widget
US_AnalysisControl::US_AnalysisControl( US_DataIO2::EditedData* dat_exp,
    QWidget* p ) : US_WidgetsDialog( p, 0 )
{
   edata          = dat_exp;
   parentw        = p;
   processor      = 0;
   dbg_level      = US_Settings::us_debug();

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
   QLabel* lb_lolimitk     = us_label(  tr( "Lower Limit (f/f0):" ) );
   QLabel* lb_uplimitk     = us_label(  tr( "Upper Limit (f/f0):" ) );
   QLabel* lb_nstepsk      = us_label(  tr( "Number Grid Points (f/f0):" ) );
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

   int nthr     = US_Settings::threads();
   nthr         = ( nthr > 1 ) ? nthr : QThread::idealThreadCount();
DbgLv(1) << "idealThrCout" << nthr;
   ct_lolimits  = us_counter( 3,  0.1,    5,   1 );
   ct_uplimits  = us_counter( 3,    5,   10,  10 );
   ct_nstepss   = us_counter( 3,    1, 1000, 100 );
   ct_lolimitk  = us_counter( 3, 0.01,    3,   1 );
   ct_uplimitk  = us_counter( 3,    3,    6,   4 );
   ct_nstepsk   = us_counter( 3,    1, 1000,  40 );
   ct_thrdcnt   = us_counter( 2,    1,   64, nthr );
   ct_lolimits->setStep(  0.1 );
   ct_uplimits->setStep(  0.1 );
   ct_nstepss ->setStep(    1 );
   ct_lolimitk->setStep( 0.01 );
   ct_uplimitk->setStep( 0.01 );
   ct_nstepsk ->setStep(    1 );
   ct_thrdcnt ->setStep(    1 );

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


   ct_iters     = us_counter( 2,    1,    8,    1 );
   ct_grrefine  = us_counter( 2,    1,   20,    7 );
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
   controlsLayout->addLayout( lo_autoplt,    row++, 0, 1, 2 );
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

   connect( ct_nstepss,  SIGNAL( valueChanged( double ) ),
            this,        SLOT(   grid_change()          ) );
   connect( ct_nstepsk,  SIGNAL( valueChanged( double ) ),
            this,        SLOT(   grid_change()          ) );
   connect( ct_grrefine, SIGNAL( valueChanged( double ) ),
            this,        SLOT(   grid_change()          ) );
   connect( ct_thrdcnt,  SIGNAL( valueChanged( double ) ),
            this,        SLOT(   grid_change()          ) );

   connect( pb_strtfit, SIGNAL( clicked()   ),
            this,       SLOT(   start()     ) );
   connect( pb_stopfit, SIGNAL( clicked()   ),
            this,       SLOT(   stop_fit()  ) );
   connect( pb_plot,    SIGNAL( clicked()   ),
            this,       SLOT(   plot()      ) );
   connect( pb_help,    SIGNAL( clicked()   ),
            this,       SLOT(   help()      ) );
   connect( pb_close,   SIGNAL( clicked()   ),
            this,       SLOT(   close_all() ) );
   connect( pb_advance, SIGNAL( clicked()   ),
            this,       SLOT(   advanced()  ) );

   grid_change();

   // initialize simulation parameters from data
   sparms         = new US_SimulationParameters();
   sparms->initFromData( NULL, *edata );
if ( dbg_level > 0 )
 sparms->save_simparms( US_Settings::appBaseDir() + "/etc/sp_2dsa.xml" );

DbgLv(1) << "Pre-resize AC size" << size();
   resize( 710, 440 );
DbgLv(1) << "Post-resize AC size" << size();
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

// start fit button clicked
void US_AnalysisControl::start()
{
   if ( processor == 0 )
      processor   = new US_2dsaProcess( edata, sparms, this );

   else
      processor->disconnect();

   if ( parentw )
   {
      US_2dsa* mainw = (US_2dsa*)parentw;
      edata          = mainw->mw_editdata();
      sdata          = mainw->mw_simdata();
      rdata          = mainw->mw_resdata();
      model          = mainw->mw_model();
      ti_noise       = mainw->mw_ti_noise();
      ri_noise       = mainw->mw_ri_noise();
      mw_stattext    = mainw->mw_status_text();
DbgLv(1) << "AnaC: edata scans" << edata->scanData.size();
   }

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

   processor->set_iters( mxiter, mciter, mniter, vtoler, menrng );

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
}

// close button clicked
void US_AnalysisControl::close_all()
{
   close();
}

// reset memory estimate when grid steps, threads or repetitions changes
void US_AnalysisControl::grid_change()
{
   int    nsteps = (int)ct_nstepss ->value();         // # steps s
   int    nstepk = (int)ct_nstepsk ->value();         // # steps k
   int    ngrrep = (int)ct_grrefine->value();         // # repetitions
   int    nthrd  = (int)ct_thrdcnt ->value();         // # threads
   long   ngstep = nsteps * nstepk;                   // # grid steps
   long   nsstep = ( nsteps / ngrrep + 1 )
                 * ( nstepk / ngrrep + 1 );           // # subgrid steps
   //int    nscan  = edata->scanData.size();            // # scans
   //int    nconc  = edata->x.size();                   // # concentrations
   //int    ntconc = nconc * nscan;                     // # total readings
   long   szsol  = sizeof( Solute );                  // size Solute
   long   szval  = sizeof( double );                  // size vector value
   long   szgso  = ngstep * szsol;                    // size grid solutes
   long   szsso  = nsstep * szsol * nthrd;            // size subg solutes
   long   szmat  = sq( ngstep / 5 ) * szval;          // size matrices
   if ( ck_tinoise->isChecked() || ck_rinoise->isChecked() )
      szmat        *= 2L;
//DbgLv(1) << "GC: ngst nsst ngrr nthr" << ngstep << nsstep << ngrrep << nthrd;
//DbgLv(1) << "GC:  szsol szval szgso szsso szmat" << szsol << szval << szgso
//   << szsso << szmat;
   double mbase  = 38.0;
   double mgfac  = 0.250 / 1024.0;
   double msfac  = 2.400 / 1024.0;
   double mmfac  = 0.010 / 1024.0;
   double mgrid  = (double)szgso * mgfac;
   double msubg  = (double)szsso * msfac;
   double mmatr  = (double)szmat * mmfac;
   int    megs   = qRound( mbase + mgrid + msubg + mmatr );
//DbgLv(1) << "GC:  mgrid msubg mmatr megs" << mgrid << msubg << mmatr << megs;

   le_estmemory->setText( QString::number( megs ) + " MB" );
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
   {
      amsg   = te_status->toPlainText() + "\n" + pmsg;
   }

   else
   {
      //int kk = le_iteration->text().toInt() + 1;
      //amsg   = tr( "Iteration %1" ).arg( kk ) + ":\n" + pmsg;
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
   //int    iternum  = le_iteration->text().toInt() + 1;
   int    iternum  = (int)rscan0->rpm;
   int    mmitnum  = (int)rscan0->seconds;
   double varinew  = rscan0->delta_r;
   double meniscus = rscan0->plateau;
   double variold  = le_newvari  ->text().toDouble();
   double vimprov  = variold - varinew;
   le_oldvari  ->setText( QString::number( variold ) );
   le_newvari  ->setText( QString::number( varinew ) );
   le_improve  ->setText( QString::number( vimprov ) );

   if ( mmitnum == 0 )   // simple refinement iteration (no MC/Meniscus)
      le_iteration->setText( QString::number( iternum ) );

   else if ( ck_menisc->isChecked() )
   {  // Meniscus
      model->description = QString( "MMITER=%1 VARI=%2 MENISCUS=%3" )
                           .arg( mmitnum ).arg( varinew ).arg( meniscus );
      le_iteration->setText( QString::number( iternum  ) + "   ( Model " +
                             QString::number( mmitnum  ) + " , Meniscus " +
                             QString::number( meniscus ) + " )" );
   }

   else
   {  // Monte Carlo
      model->description = QString( "MMITER=%1 VARI=%2 " )
                           .arg( mmitnum ).arg( varinew );
      le_iteration->setText( QString::number( iternum ) + "   ( MC Model " +
                             QString::number( mmitnum ) + " )" );
   }

   US_2dsa* mainw = (US_2dsa*)parentw;

   if ( alldone )
   {
      if ( parentw )
      {
         if ( mmitnum > 0 )
            mainw->analysis_done( -2 );

         mainw->analysis_done( ck_autoplt->isChecked() ? 1 : 0 );
      }

      pb_strtfit->setEnabled( true  );
      pb_stopfit->setEnabled( false );
      pb_plot   ->setEnabled( true  );
      pb_save   ->setEnabled( true  );
   }

   else if ( mmitnum > 0  &&  stage > 0 )
   {  // signal main to update lists of models,noises
      if ( parentw )
         mainw->analysis_done( -2 );
   }
}

// slot to handle advanced analysis controls
void US_AnalysisControl::advanced()
{
   US_AdvAnalysis* aadiag = new US_AdvAnalysis( sparms, this );
   if ( aadiag->exec() == QDialog::Accepted )
   {
      int    grtype = 0;
      double grpar1 = 0.0;
      double grpar2 = 0.0;
      double grpar3 = 0.0;
      bool   men    = false;
      double mepar1 = 0.0;
      double mepar2 = 0.0;
      bool   reg    = false;
      double repar1 = 0.0;
      aadiag->get_parameters( grtype, grpar1, grpar2, grpar3,
                              men,    mepar1, mepar2,
                              reg,    repar1 );
DbgLv(1) << "Adv ACCEPT";
DbgLv(1) << "Adv grtype par123" << grtype << grpar1 << grpar2 << grpar3;
DbgLv(1) << "Adv men    par12 " << men    << mepar1 << mepar2;
DbgLv(1) << "Adv reg    par1  " << reg    << repar1;
   }
else
DbgLv(1) << "Adv REJECT";

   qApp->processEvents();

   delete aadiag;
}

