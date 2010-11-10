//! \file us_anal_control.cpp

#include "us_2dsa.h"
#include "us_anal_control.h"
#include "us_settings.h"
#include "us_gui_settings.h"

#include <qwt_legend.h>

// constructor:  enhanced plot control widget
US_AnalControl::US_AnalControl( US_DataIO2::EditedData* dat_exp, QWidget* p )
   : US_WidgetsDialog( p, 0 )
{
   edata          = dat_exp;
   parentw        = p;
   processor      = 0;
   dbg_level      = US_Settings::us_debug();

   setObjectName( "US_AnalControl" );
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
   QLabel* lb_threadcnt    = us_label(  tr( "Thread Count:" ) );
   QLabel* lb_estmemory    = us_label(  tr( "Estimated Memory:" ) );
   QLabel* lb_iteration    = us_label(  tr( "Iteration:" ) );
   QLabel* lb_oldvari      = us_label(  tr( "Old Variance:" ) );
   QLabel* lb_newvari      = us_label(  tr( "New Variance:" ) );
   QLabel* lb_improve      = us_label(  tr( "Improvement:" ) );
   QLabel* lb_status       = us_label(  tr( "Status:" ) );

   QLabel* lb_optimiz      = us_banner( tr( "Optimization Methods:" ) );
   QLabel* lb_grrefine     = us_label(  tr( "Grid Refinements:" ) );
   QLabel* lb_repetitl     = us_label(  tr( "Repetitions:" ) );
   QLabel* lb_scfactor     = us_label(  tr( "Scaling Factor:" ) );
   QLabel* lb_scfact2      = us_label(  tr( "Scaling Factor 2:" ) );
   QLabel* lb_repetitr     = us_label(  tr( "Repetitions:" ) );
   QLabel* lb_stddevia     = us_label(  tr( "Standard Deviation:" ) );
   QLabel* lb_coaldist     = us_label(  tr( "Coalescing Distance:" ) );
   QLabel* lb_nbrclips     = us_label(  tr( "# of Clipped Solutes:" ) );
   QLabel* lb_regufact     = us_label(  tr( "Regularization Factor:" ) );

   QPushButton* pb_strtfit = us_pushbutton( tr( "Start Fit" ) );
   QPushButton* pb_plot    = us_pushbutton( tr( "Plot Results" ) );
   QPushButton* pb_save    = us_pushbutton( tr( "Save Results" ) );
   QPushButton* pb_help    = us_pushbutton( tr( "Help" ) );
   QPushButton* pb_close   = us_pushbutton( tr( "Close" ) );

   QLayout* lo_tinois      =
      us_checkbox( tr( "Fit Time-Invariant Noise"  ), ck_tinoise );
   QLayout* lo_rinois      =
      us_checkbox( tr( "Fit Radially-Inv. Noise"   ), ck_rinoise );
   QLayout* lo_autupd      =
      us_checkbox( tr( "Automatically Update Plot" ), ck_autoupd );

   int nthr     = QThread::idealThreadCount();
DbgLv(1) << "idealThrCout" << nthr;
   ct_lolimits  = us_counter( 3,  0.1,    5,   1 );
   ct_uplimits  = us_counter( 3,    5,   10,  10 );
   ct_nstepss   = us_counter( 3,    1, 1000, 150 );
   ct_lolimitk  = us_counter( 3, 0.01,    3,   1 );
   ct_uplimitk  = us_counter( 3,    3,    6,   4 );
   ct_nstepsk   = us_counter( 3,    1, 1000, 150 );
   ct_threadcnt = us_counter( 2,    1,   64, nthr );
   ct_lolimits ->setStep(  0.1 );
   ct_uplimits ->setStep(  0.1 );
   ct_nstepss  ->setStep(    1 );
   ct_lolimitk ->setStep( 0.01 );
   ct_uplimitk ->setStep( 0.01 );
   ct_nstepsk  ->setStep(    1 );
   ct_threadcnt->setStep(    1 );

   le_estmemory = us_lineedit( "100 MB" );
   le_iteration = us_lineedit( "1" );
   le_oldvari   = us_lineedit( "0.000e-05" );
   le_newvari   = us_lineedit( "0.000e-05" );
   le_improve   = us_lineedit( "0.000e-08" );

   b_progress   = us_progressBar( 0, 100, 0 );

   QLayout*  lo_unifgr  =
      us_checkbox( tr( "Uniform Grid"                      ), ck_unifgr, true );
   QLayout*  lo_locugr  =
      us_checkbox( tr( "Local Uniform Grid"                ), ck_locugr );
   QLayout*  lo_ranlgr  =
      us_checkbox( tr( "Random Local Grid"                 ), ck_ranlgr );
   QLayout*  lo_soluco  =
      us_checkbox( tr( "Solute Coalescing"                 ), ck_soluco );
   QLayout*  lo_clipcs  =
      us_checkbox( tr( "Clip Lowest Concentration Solutes" ), ck_clipcs );
   QLayout*  lo_regulz  =
      us_checkbox( tr( "Regularization"                    ), ck_regulz );


   ct_grrefine  = us_counter( 2,    1,   20,    1 );
   ct_repetitl  = us_counter( 2,    1,   20,    1 );
   ct_scfactor  = us_counter( 3, 0.01, 10.0,  0.3 );
   ct_scfact2   = us_counter( 3, 0.01, 10.0,  0.9 );
   ct_repetitr  = us_counter( 2,    1,   20,    1 );
   ct_stddevia  = us_counter( 3, 0.01, 10.0,  0.1 );
   ct_coaldist  = us_counter( 3, 0.01, 10.0,  0.1 );
   ct_nbrclips  = us_counter( 2,    1,   20,    1 );
   ct_regufact  = us_counter( 3, 0.01, 10.0,  0.9 );
   ct_grrefine ->setStep(    1 );
   ct_repetitl ->setStep(    1 );
   ct_scfactor ->setStep( 0.01 );
   ct_scfact2  ->setStep( 0.01 );
   ct_repetitr ->setStep(    1 );
   ct_stddevia ->setStep( 0.01 );
   ct_coaldist ->setStep( 0.01 );
   ct_nbrclips ->setStep(    1 );
   ct_regufact ->setStep( 0.01 );


   controlsLayout->addWidget( lb_fitting,    0, 0, 1, 4 );
   controlsLayout->addWidget( lb_lolimits,   1, 0, 1, 2 );
   controlsLayout->addWidget( ct_lolimits,   1, 2, 1, 2 );
   controlsLayout->addWidget( lb_uplimits,   2, 0, 1, 2 );
   controlsLayout->addWidget( ct_uplimits,   2, 2, 1, 2 );
   controlsLayout->addWidget( lb_nstepss,    3, 0, 1, 2 );
   controlsLayout->addWidget( ct_nstepss,    3, 2, 1, 2 );
   controlsLayout->addWidget( lb_lolimitk,   4, 0, 1, 2 );
   controlsLayout->addWidget( ct_lolimitk,   4, 2, 1, 2 );
   controlsLayout->addWidget( lb_uplimitk,   5, 0, 1, 2 );
   controlsLayout->addWidget( ct_uplimitk,   5, 2, 1, 2 );
   controlsLayout->addWidget( lb_nstepsk,    6, 0, 1, 2 );
   controlsLayout->addWidget( ct_nstepsk,    6, 2, 1, 2 );
   controlsLayout->addWidget( lb_threadcnt,  7, 0, 1, 2 );
   controlsLayout->addWidget( ct_threadcnt,  7, 2, 1, 2 );
   controlsLayout->addLayout( lo_tinois,     8, 0, 1, 2 );
   controlsLayout->addLayout( lo_rinois,     8, 2, 1, 2 );
   controlsLayout->addLayout( lo_autupd,     9, 0, 1, 2 );
   controlsLayout->addWidget( pb_strtfit,    9, 2, 1, 2 );
   controlsLayout->addWidget( pb_plot,      10, 0, 1, 2 );
   controlsLayout->addWidget( pb_save,      10, 2, 1, 2 );
   controlsLayout->addWidget( pb_help,      11, 0, 1, 2 );
   controlsLayout->addWidget( pb_close,     11, 2, 1, 2 );
   controlsLayout->addWidget( lb_estmemory, 12, 0, 1, 2 );
   controlsLayout->addWidget( le_estmemory, 12, 2, 1, 2 );
   controlsLayout->addWidget( lb_iteration, 13, 0, 1, 2 );
   controlsLayout->addWidget( le_iteration, 13, 2, 1, 2 );
   controlsLayout->addWidget( lb_oldvari,   14, 0, 1, 2 );
   controlsLayout->addWidget( le_oldvari,   14, 2, 1, 2 );
   controlsLayout->addWidget( lb_newvari,   15, 0, 1, 2 );
   controlsLayout->addWidget( le_newvari,   15, 2, 1, 2 );
   controlsLayout->addWidget( lb_improve,   16, 0, 1, 2 );
   controlsLayout->addWidget( le_improve,   16, 2, 1, 2 );
   controlsLayout->addWidget( lb_status,    17, 0, 1, 1 );
   controlsLayout->addWidget( b_progress,   17, 1, 1, 3 );

   optimizeLayout->addWidget( lb_optimiz,    0, 0, 1, 2 );
   optimizeLayout->addLayout( lo_unifgr,     1, 0, 1, 2 );
   optimizeLayout->addWidget( lb_grrefine,   2, 0, 1, 1 );
   optimizeLayout->addWidget( ct_grrefine,   2, 1, 1, 1 );
   optimizeLayout->addLayout( lo_locugr,     3, 0, 1, 2 );
   optimizeLayout->addWidget( lb_repetitl,   4, 0, 1, 1 );
   optimizeLayout->addWidget( ct_repetitl,   4, 1, 1, 1 );
   optimizeLayout->addWidget( lb_scfactor,   5, 0, 1, 1 );
   optimizeLayout->addWidget( ct_scfactor,   5, 1, 1, 1 );
   optimizeLayout->addWidget( lb_scfact2,    6, 0, 1, 1 );
   optimizeLayout->addWidget( ct_scfact2,    6, 1, 1, 1 );
   optimizeLayout->addLayout( lo_ranlgr,     7, 0, 1, 2 );
   optimizeLayout->addWidget( lb_repetitr,   8, 0, 1, 1 );
   optimizeLayout->addWidget( ct_repetitr,   8, 1, 1, 1 );
   optimizeLayout->addWidget( lb_stddevia,   9, 0, 1, 1 );
   optimizeLayout->addWidget( ct_stddevia,   9, 1, 1, 1 );
   optimizeLayout->addLayout( lo_soluco,    10, 0, 1, 2 );
   optimizeLayout->addWidget( lb_coaldist,  11, 0, 1, 1 );
   optimizeLayout->addWidget( ct_coaldist,  11, 1, 1, 1 );
   optimizeLayout->addLayout( lo_clipcs,    12, 0, 1, 2 );
   optimizeLayout->addWidget( lb_nbrclips,  13, 0, 1, 1 );
   optimizeLayout->addWidget( ct_nbrclips,  13, 1, 1, 1 );
   optimizeLayout->addLayout( lo_regulz,    14, 0, 1, 2 );
   optimizeLayout->addWidget( lb_regufact,  15, 0, 1, 1 );
   optimizeLayout->addWidget( ct_regufact,  15, 1, 1, 1 );

   le_estmemory->setReadOnly( true );
   le_iteration->setReadOnly( true );
   le_oldvari  ->setReadOnly( true );
   le_newvari  ->setReadOnly( true );
   le_improve  ->setReadOnly( true );
   QPalette gray = US_GuiSettings::editColor();
   gray.setColor( QPalette::Base, QColor( 0xd0, 0xd0, 0xd0 ) );
   le_estmemory->setPalette( gray );
   le_iteration->setPalette( gray );
   le_oldvari  ->setPalette( gray );
   le_newvari  ->setPalette( gray );
   le_improve  ->setPalette( gray );

   ck_unifgr->setChecked( true );

   optimize_options();

   connect( ck_unifgr, SIGNAL( toggled( bool ) ),
            this,  SLOT( checkUniGrid(  bool ) ) );
   connect( ck_locugr, SIGNAL( toggled( bool ) ),
            this,  SLOT( checkLocalUni( bool ) ) );
   connect( ck_ranlgr, SIGNAL( toggled( bool ) ),
            this,  SLOT( checkRandLoc(  bool ) ) );
   connect( ck_soluco, SIGNAL( toggled( bool ) ),
            this,  SLOT( checkSoluCoal( bool ) ) );
   connect( ck_clipcs, SIGNAL( toggled( bool ) ),
            this,  SLOT( checkClipLow(  bool ) ) );
   connect( ck_regulz, SIGNAL( toggled( bool ) ),
            this,  SLOT( checkRegular(  bool ) ) );

   connect( ct_nstepss,  SIGNAL( valueChanged( double ) ),
            this,        SLOT(   grid_change()          ) );
   connect( ct_nstepsk,  SIGNAL( valueChanged( double ) ),
            this,        SLOT(   grid_change()          ) );
   connect( ct_grrefine, SIGNAL( valueChanged( double ) ),
            this,        SLOT(   grid_change()          ) );

   pb_plot->setEnabled( false );
   pb_save->setEnabled( false );

   connect( pb_strtfit, SIGNAL( clicked() ),
            this,       SLOT( start()   ) );
   connect( pb_plot,    SIGNAL( clicked() ),
            this,       SLOT( plot()    ) );
   connect( pb_help,    SIGNAL( clicked() ),
            this,       SLOT( help()      ) );
   connect( pb_close,   SIGNAL( clicked() ),
            this,       SLOT( close_all() ) );

DbgLv(1) << "Pre-adjust size" << size();
   resize( 740, 440 );
DbgLv(1) << "Post-adjust size" << size();
}

// enable/disable optimize counters based on chosen method
void US_AnalControl::optimize_options()
{
   ct_grrefine->setEnabled( ck_unifgr->isChecked() );
   ct_repetitl->setEnabled( ck_locugr->isChecked() );
   ct_scfactor->setEnabled( ck_locugr->isChecked() );
   ct_scfact2 ->setEnabled( ck_locugr->isChecked() );
   ct_repetitr->setEnabled( ck_ranlgr->isChecked() );
   ct_stddevia->setEnabled( ck_ranlgr->isChecked() );
   ct_coaldist->setEnabled( ck_soluco->isChecked() );
   ct_nbrclips->setEnabled( ck_clipcs->isChecked() );
   ct_regufact->setEnabled( ck_regulz->isChecked() );
}

// uncheck optimize options other than one just checked
void US_AnalControl::uncheck_optimize( int ckflag )
{
   if ( ckflag != 1 ) ck_unifgr->setChecked( false );
   if ( ckflag != 2 ) ck_locugr->setChecked( false );
   if ( ckflag != 3 ) ck_ranlgr->setChecked( false );
   if ( ckflag != 4 ) ck_soluco->setChecked( false );
   if ( ckflag != 5 ) ck_clipcs->setChecked( false );
   if ( ckflag != 6 ) ck_regulz->setChecked( false );
}

// handle uniform grid checked
void US_AnalControl::checkUniGrid( bool checked )
{
   if ( checked ) { uncheck_optimize( 1 ); optimize_options(); }
}

// handle local uniform grid checked
void US_AnalControl::checkLocalUni( bool checked )
{
   if ( checked ) { uncheck_optimize( 2 ); optimize_options(); }
}

// handle random local grid checked
void US_AnalControl::checkRandLoc( bool checked )
{
   if ( checked ) { uncheck_optimize( 3 ); optimize_options(); }
}

// handle solute coalescing checked
void US_AnalControl::checkSoluCoal( bool checked )
{
   if ( checked ) { uncheck_optimize( 4 ); optimize_options(); }
}

// handle clip lowest conc. solute checked
void US_AnalControl::checkClipLow( bool checked )
{
   if ( checked ) { uncheck_optimize( 5 ); optimize_options(); }
}

// handle regularization checked
void US_AnalControl::checkRegular( bool checked )
{
   if ( checked ) { uncheck_optimize( 6 ); optimize_options(); }
}

// start fit button clicked
void US_AnalControl::start()
{
   if ( processor == 0 )
      processor   = new US_2dsaProcess( edata, this );

   if ( parentw )
   {
      US_2dsa* mainw = (US_2dsa*)parentw;
      edata          = mainw->mw_editdata();
      sdata          = mainw->mw_simdata();
      rdata          = mainw->mw_resdata();
      model          = mainw->mw_model();
      ti_noise       = mainw->mw_ti_noise();
      ri_noise       = mainw->mw_ri_noise();
      mw_progbar     = mainw->mw_progress_bar();
      mw_stattext    = mainw->mw_status_text();
DbgLv(1) << "AnaC: edata scans" << edata->scanData.size();
   }

   double slo    = ct_lolimits->value();
   double sup    = ct_uplimits->value();
   int    nss    = (int)ct_nstepss->value();
   double klo    = ct_lolimitk->value();
   double kup    = ct_uplimitk->value();
   int    nks    = (int)ct_nstepsk->value();
   int    ngrr   = (int)ct_grrefine->value();
   int    nthr   = (int)ct_threadcnt->value();
   int    noif   = ( ck_tinoise->isChecked() ? 1 : 0 ) +
                   ( ck_rinoise->isChecked() ? 2 : 0 );

   ncsteps       = 0;
   nctotal       = ( nss * nks * 5 ) / 4;
   b_progress->setMaximum( nctotal );
   mw_progbar->setMaximum( nctotal );

   connect( processor, SIGNAL( progress_update( int ) ),
            this,      SLOT(   update_progress( int ) ) );
   connect( processor, SIGNAL( refine_complete( int ) ),
            this,      SLOT(   completed_refine(  int ) ) );
   connect( processor, SIGNAL( message_update( QString ) ),
            this,      SLOT(   progress_message( QString ) ) );
   connect( processor, SIGNAL( subgrids_complete( )      ),
            this,      SLOT(   completed_subgrids()      ) );
   connect( processor, SIGNAL( process_complete()        ),
            this,      SLOT(   completed_process()       ) );

   processor->start_fit( slo, sup, nss, klo, kup, nks,
         ngrr, nthr, noif );

}

// plot button clicked
void US_AnalControl::plot()
{
}

// close button clicked
void US_AnalControl::close_all()
{
   close();
}

// reset memory estimate when grid steps count or repetitions changes
void US_AnalControl::grid_change()
{
   int    nsteps = (int)ct_nstepss ->value();               // # steps s
   int    nstepk = (int)ct_nstepsk ->value();               // # steps k
   int    ngrrep = (int)ct_grrefine->value();               // # repetitions
   long   ntstep = nsteps * nstepk;                         // # total steps
   int    nscan  = edata->scanData.size();                  // # scans
   int    nconc  = edata->x.size();                         // # concentrations
   int    ntconc = nconc * nscan;                           // # total readings
   int    isizm  = sizeof( US_Model );                      // size model
   int    isizc  = sizeof( US_Model::SimulationComponent ); // size sim comps
   int    isizd  = sizeof( US_DataIO2::RawData );           // size base data
   int    isizx  = sizeof( US_DataIO2::XValue );            // size x (radius)
   int    isizs  = sizeof( US_DataIO2::Scan );              // size scan
   int    isizr  = sizeof( US_DataIO2::Reading );           // size conc vector
//DbgLv(1) << "GC: ntstep ntconc ngrrep" << ntstep << ntconc << ngrrep;
//DbgLv(1) << "GC: isizm c d x s r" << isizm << isizc << isizd << isizx
//   << isizs << isizr;
   isizm         = isizm + isizc;         // size model w component
   isizs         = isizs * ntconc;        // size all scans in data
   isizx         = isizx * nconc;         // size x vector in data
   isizr         = isizr * nconc;         // size c vector in data
   isizd         = isizd + isizs + isizx + isizr;  // size one full data
DbgLv(1) << "GC: isizm c d x s r" << isizm << isizc << isizd << isizx
   << isizs << isizr;
   double megs   = ( (double)ntstep * ( (double)isizm / 1024.0 )
                   + (double)ngrrep * ( (double)isizd /  512.0 ) )
                   / 1024.0 + 16.3576;

   le_estmemory->setText( QString().sprintf( "%.1f MB", megs ) );
}

// slot to handle progress update
void US_AnalControl::update_progress( int ksteps )
{
   ncsteps += ksteps;
   b_progress->setValue( ncsteps );
   mw_progbar->setValue( ncsteps );
DbgLv(1) << "UpdPr: ks ncs nts" << ksteps << ncsteps << nctotal;
}

// slot to handle completed refinement step
void US_AnalControl::completed_refine( int /*kctask*/ )
{
   //le_iteration->setText( QString::number( kctask ) );
}

// slot to handle updated progress message
void US_AnalControl::progress_message( QString pmsg )
{
   mw_stattext->setText( tr( "Subgrid solutions being calculated..." )
         + "\n" + pmsg );
}

// slot to handle completed subgrids
void US_AnalControl::completed_subgrids()
{
   ncsteps      = ( nctotal * 4 ) / 5;
   b_progress->setValue( ncsteps );
   mw_progbar->setValue( ncsteps );
}

// slot to handle completed processing
void US_AnalControl::completed_process()
{
   b_progress->setValue( nctotal );
   mw_progbar->setValue( nctotal );
}

