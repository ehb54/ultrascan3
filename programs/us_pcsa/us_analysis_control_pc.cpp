//! \file us_analysis_control_pc.cpp

#include "us_pcsa.h"
#include "us_analysis_control_pc.h"
#include "us_adv_analysis_pc.h"
#include "us_simparms.h"
#include "us_rpscan.h"
#include "us_settings.h"
#include "us_passwd.h"
#include "us_db2.h"
#include "us_gui_settings.h"
#include "us_memory.h"

// constructor:  pcsa analysis controls widget
US_AnalysisControlPc::US_AnalysisControlPc(
   QList< US_SolveSim::DataSet* >& dsets, QWidget* p )
   : US_WidgetsDialog( p, 0 ), dsets( dsets )
{
   parentw        = p;
   processor      = 0;
   dbg_level      = US_Settings::us_debug();
   varimin        = 9e+99;
   bmndx          = -1;
   mlnplotd       = 0;
   fitpars        = QString();
   ctypes << CTYPE_SL << CTYPE_IS << CTYPE_DS << CTYPE_HL << CTYPE_2O
          << CTYPE_ALL;
   attr_x         = 0;
   attr_y         = 1;
   attr_z         = 3;
   sol_type       = ( attr_x << 6 ) + ( attr_y << 3 ) + attr_z;

   if ( parentw )
   {  // Get pointers to needed objects from the main
      US_pcsa* mainw = (US_pcsa*)parentw;
      edata          = mainw->mw_editdata();
      sdata          = mainw->mw_simdata();
      rdata          = mainw->mw_resdata();
      model          = mainw->mw_model();
      ti_noise       = mainw->mw_ti_noise();
      ri_noise       = mainw->mw_ri_noise();
      mw_stattext    = mainw->mw_status_text();
      mw_modstats    = mainw->mw_model_stats();
      mw_mrecs       = mainw->mw_mrecs();
      mw_mrecs_mc    = mainw->mw_mrecs_mc();
      mw_baserss     = mainw->mw_base_rss();
DbgLv(1) << "AnaC: edata scans" << edata->scanData.size();
   }
   else
   {  // Aarrggg! No pointer back to parent!!!
      DbgLv(0) << "*ERROR* AnalysisControl has no pointer back to Main PCSA";
      QMessageBox::critical( this, tr( "Parent Pointer NULL!" ),
         tr( "*ERROR* AnalysisControl has no pointer back to Main PCSA" ) );
      close();
      return;
   }

   setObjectName( "US_AnalysisControlPc" );
   setAttribute( Qt::WA_DeleteOnClose, true );
   setPalette( US_GuiSettings::frameColor() );
   setFont( QFont( US_GuiSettings::fontFamily(),
                   US_GuiSettings::fontSize() ) );
   QFontMetrics fmet( font() );

   // lay out the GUI
   setWindowTitle(
      tr( "Parametrically Constrained Spectrum Analysis Controls" ) );

   mainLayout      = new QHBoxLayout( this );
   controlsLayout  = new QGridLayout( );
   optimizeLayout  = new QGridLayout( );

   mainLayout->setSpacing        ( 2 );
   mainLayout->setContentsMargins( 2, 2, 2, 2 );

   mainLayout->addLayout( controlsLayout );
   mainLayout->addLayout( optimizeLayout  );

   QLabel* lb_fitting      = us_banner( tr( "Fitting Controls:" ) );
   QLabel* lb_curvtype     = us_label(  tr( "Curve Type:"       ) );
   QLabel* lb_x_type       = us_label(  tr( "X Axis Type:"      ) );
   QLabel* lb_y_type       = us_label(  tr( "Y Axis Type:"      ) );
   QLabel* lb_z_type       = us_label(  tr( "Z Axis Type:"      ) );
   QLabel* lb_z_value      = us_label(  tr( "Z Value/Coeffs."   ) );
   QLabel* lb_range_x      = us_label(  tr( "X range:"          ) );
   QLabel* lb_range_y      = us_label(  tr( "Y range:"          ) );
   QLabel* lb_varcount     = us_label(  tr( "Variations Count:" ) );
   QLabel* lb_gfiters      = us_label(  tr( "Grid Fit Iterations:" ) );
   QLabel* lb_gfthresh     = us_label(  tr( "Threshold Delta-RMSD Ratio:" ) );
   QLabel* lb_cresolu      = us_label(  tr( "Curve Resolution Points:" ) );
   QLabel* lb_lmmxcall     = us_label(  tr( "Maximum L-M Evaluate Calls:" ) );
   QLabel* lb_tralpha      = us_label(  tr( "Regularization Parameter:"  ) );
   QLabel* lb_thrdcnt      = us_label(  tr( "Thread Count:" ) );
   QLabel* lb_minvari      = us_label(  tr( "Best Model Variance:" ) );
   QLabel* lb_minrmsd      = us_label(  tr( "Best Model RMSD:" ) );
   QLabel* lb_status       = us_label(  tr( "Status:" ) );
           bg_x_axis       = new QButtonGroup( this );
           bg_y_axis       = new QButtonGroup( this );
   QGridLayout* gl_x_s     = us_radiobutton( tr( "s"    ), rb_x_s,    true  );
   QGridLayout* gl_x_ff0   = us_radiobutton( tr( "f/f0" ), rb_x_ff0,  false );
   QGridLayout* gl_x_mw    = us_radiobutton( tr( "mw"   ), rb_x_mw,   false );
   QGridLayout* gl_x_vbar  = us_radiobutton( tr( "vbar" ), rb_x_vbar, false );
   QGridLayout* gl_x_D     = us_radiobutton( tr( "D"    ), rb_x_D,    false );
   QGridLayout* gl_y_s     = us_radiobutton( tr( "s"    ), rb_y_s,    false );
   QGridLayout* gl_y_ff0   = us_radiobutton( tr( "f/f0" ), rb_y_ff0,  true  );
   QGridLayout* gl_y_mw    = us_radiobutton( tr( "mw"   ), rb_y_mw,   false );
   QGridLayout* gl_y_vbar  = us_radiobutton( tr( "vbar" ), rb_y_vbar, false );
   QGridLayout* gl_y_D     = us_radiobutton( tr( "D"    ), rb_y_D,    false );
   le_z_func    = us_lineedit( "0.0", -1, false );
   bg_x_axis->addButton( rb_x_s,    ATTR_S );
   bg_x_axis->addButton( rb_x_ff0,  ATTR_K );
   bg_x_axis->addButton( rb_x_mw,   ATTR_W );
   bg_x_axis->addButton( rb_x_vbar, ATTR_V );
   bg_x_axis->addButton( rb_x_D,    ATTR_D );
   bg_y_axis->addButton( rb_y_s,    ATTR_S );
   bg_y_axis->addButton( rb_y_ff0,  ATTR_K );
   bg_y_axis->addButton( rb_y_mw,   ATTR_W );
   bg_y_axis->addButton( rb_y_vbar, ATTR_V );
   bg_y_axis->addButton( rb_y_D,    ATTR_D );
   rb_x_s   ->setChecked( true  );
   rb_y_ff0 ->setChecked( true  );
   rb_y_s   ->setEnabled( false );

   QLabel* lb_statinfo     = us_banner( tr( "Status Information:" ) );

   pb_pltlines = us_pushbutton( tr( "Plot Model Lines"              ), true );
   pb_startfit = us_pushbutton( tr( "Start Fit"                     ), true );
   pb_scanregp = us_pushbutton( tr( "Perform Regularization Scan"   ), false );
   pb_finalmdl = us_pushbutton( tr( "Recompute Best Model"          ), false );
   pb_stopfit  = us_pushbutton( tr( "Stop Fit"                      ), false );
   pb_plot     = us_pushbutton( tr( "Plot Results"                  ), false );
   pb_save     = us_pushbutton( tr( "Save Results"                  ), false );
   pb_advanaly = us_pushbutton( tr( "Advanced Controls"             ), true );
   pb_help     = us_pushbutton( tr( "Help" ) );
   pb_close    = us_pushbutton( tr( "Close" ) );
   te_status   = us_textedit();
   us_setReadOnly( te_status, true );

   QLayout* lo_lmalpha     =
      us_checkbox( tr( "Regularize in L-M Fits"        ), ck_lmalpha  );
   QLayout* lo_fxalpha     =
      us_checkbox( tr( "Regularize in Grid Fits"       ), ck_fxalpha  );
   QLayout* lo_tinois      =
      us_checkbox( tr( "Fit Time-Invariant Noise"      ), ck_tinoise  );
   QLayout* lo_rinois      =
      us_checkbox( tr( "Fit Radially-Invariant Noise"  ), ck_rinoise  );
   ck_fxalpha ->setEnabled( false );
   ck_lmalpha ->setEnabled( false );

   int nthr     = US_Settings::threads();
   nthr         = ( nthr > 1 ) ? nthr : QThread::idealThreadCount();
DbgLv(1) << "idealThrCout" << nthr;
   ct_lolimitx  = us_counter( 2, -10000, 10000,    1 );
   ct_uplimitx  = us_counter( 3, -10000, 10000,   10 );
   ct_lolimity  = us_counter( 2,      1,     8,    1 );
   ct_uplimity  = us_counter( 3,      1,   100,    5 );
   ct_varcount  = us_counter( 2,      1,   200,    6 );
   ct_gfiters   = us_counter( 2,      1,    20,    3 );
   ct_gfthresh  = us_counter( 3,  1.e-6, 1.e-2, 1e-4 );
   ct_lmmxcall  = us_counter( 2,      0,   200,    0 );
   ct_cresolu   = us_counter( 2,     20,   501,  100 );
   ct_thrdcnt   = us_counter( 2,      1,    64, nthr );
   ct_tralpha   = us_counter( 3,      0,   100,    0 );
   ct_lolimitx->setSingleStep(  0.1 );
   ct_uplimitx->setSingleStep(  0.1 );
   ct_lolimity->setSingleStep( 0.01 );
   ct_uplimity->setSingleStep( 0.01 );
   ct_varcount->setSingleStep(    1 );
   ct_gfiters ->setSingleStep(    1 );
   ct_gfthresh->setSingleStep( 1.e-6 );
   ct_lmmxcall->setSingleStep(    1 );
   ct_cresolu ->setSingleStep(    1 );
   ct_tralpha ->setSingleStep( 0.001 );
   ct_thrdcnt ->setSingleStep(    1 );
   cb_curvtype = us_comboBox();
   cb_curvtype->addItem( "Straight Line" );
   cb_curvtype->addItem( "Increasing Sigmoid" );
   cb_curvtype->addItem( "Decreasing Sigmoid" );
   cb_curvtype->addItem( "Horizontal Line [ C(s) ]" );
   cb_curvtype->addItem( "Second-Order Power Law" );
   cb_curvtype->setCurrentIndex( 1 );
   cb_z_type   = us_comboBox();
   cb_z_type  ->addItem( "vbar" );
   cb_z_type  ->addItem( "f/f0" );
   cb_z_type  ->addItem( "mw"   );
   cb_z_type  ->setCurrentIndex( 0 );

   le_minvari   = us_lineedit( "0.000e-05", -1, true );
   le_minrmsd   = us_lineedit( "0.009000" , -1, true );

   b_progress   = us_progressBar( 0, 100, 0 );

   int row       = 0;
   controlsLayout->addWidget( lb_fitting,    row++, 0, 1, 6 );
   controlsLayout->addWidget( lb_curvtype,   row,   0, 1, 2 );
   controlsLayout->addWidget( cb_curvtype,   row++, 2, 1, 4 );
   controlsLayout->addWidget( lb_x_type,     row,   0, 1, 1 );
   controlsLayout->addLayout( gl_x_s,        row,   1, 1, 1 );
   controlsLayout->addLayout( gl_x_ff0,      row,   2, 1, 1 );
   controlsLayout->addLayout( gl_x_mw,       row,   3, 1, 1 );
   controlsLayout->addLayout( gl_x_vbar,     row,   4, 1, 1 );
   controlsLayout->addLayout( gl_x_D,        row++, 5, 1, 1 );
   controlsLayout->addWidget( lb_y_type,     row,   0, 1, 1 );
   controlsLayout->addLayout( gl_y_s,        row,   1, 1, 1 );
   controlsLayout->addLayout( gl_y_ff0,      row,   2, 1, 1 );
   controlsLayout->addLayout( gl_y_mw,       row,   3, 1, 1 );
   controlsLayout->addLayout( gl_y_vbar,     row,   4, 1, 1 );
   controlsLayout->addLayout( gl_y_D,        row++, 5, 1, 1 );
   controlsLayout->addWidget( lb_z_type,     row,   0, 1, 1 );
   controlsLayout->addWidget( cb_z_type,     row,   1, 1, 1 );
   controlsLayout->addWidget( lb_z_value,    row,   2, 1, 2 );
   controlsLayout->addWidget( le_z_func,     row++, 4, 1, 2 );
   controlsLayout->addWidget( lb_range_x,    row,   0, 1, 1 );
   controlsLayout->addWidget( ct_lolimitx,   row,   1, 1, 2 );
   controlsLayout->addWidget( ct_uplimitx,   row++, 3, 1, 3 );
   controlsLayout->addWidget( lb_range_y,    row,   0, 1, 1 );
   controlsLayout->addWidget( ct_lolimity,   row,   1, 1, 2 );
   controlsLayout->addWidget( ct_uplimity,   row++, 3, 1, 3 );
   controlsLayout->addWidget( lb_varcount,   row,   0, 1, 3 );
   controlsLayout->addWidget( ct_varcount,   row++, 3, 1, 3 );
   controlsLayout->addWidget( lb_gfiters,    row,   0, 1, 3 );
   controlsLayout->addWidget( ct_gfiters,    row++, 3, 1, 3 );
   controlsLayout->addWidget( lb_gfthresh,   row,   0, 1, 3 );
   controlsLayout->addWidget( ct_gfthresh,   row++, 3, 1, 3 );
   controlsLayout->addWidget( lb_cresolu,    row,   0, 1, 3 );
   controlsLayout->addWidget( ct_cresolu,    row++, 3, 1, 3 );
   controlsLayout->addWidget( lb_lmmxcall,   row,   0, 1, 3 );
   controlsLayout->addWidget( ct_lmmxcall,   row++, 3, 1, 3 );
   controlsLayout->addWidget( lb_tralpha,    row,   0, 1, 3 );
   controlsLayout->addWidget( ct_tralpha,    row++, 3, 1, 3 );
   controlsLayout->addWidget( lb_thrdcnt,    row,   0, 1, 3 );
   controlsLayout->addWidget( ct_thrdcnt,    row++, 3, 1, 3 );
   controlsLayout->addLayout( lo_lmalpha,    row,   0, 1, 3 );
   controlsLayout->addWidget( pb_startfit,   row++, 3, 1, 3 );
   controlsLayout->addLayout( lo_fxalpha,    row,   0, 1, 3 );
   controlsLayout->addWidget( pb_stopfit,    row++, 3, 1, 3 );
   controlsLayout->addLayout( lo_tinois,     row,   0, 1, 3 );
   controlsLayout->addWidget( pb_scanregp,   row++, 3, 1, 3 );
   controlsLayout->addLayout( lo_rinois,     row,   0, 1, 3 );
   controlsLayout->addWidget( pb_finalmdl,   row++, 3, 1, 3 );
   controlsLayout->addWidget( pb_plot,       row,   0, 1, 3 );
   controlsLayout->addWidget( pb_save,       row++, 3, 1, 3 );
   controlsLayout->addWidget( pb_advanaly,   row++, 0, 1, 6 );
   controlsLayout->addWidget( pb_pltlines,   row,   0, 1, 2 );
   controlsLayout->addWidget( pb_help,       row,   2, 1, 2 );
   controlsLayout->addWidget( pb_close,      row++, 4, 1, 2 );
   controlsLayout->addWidget( lb_status,     row,   0, 1, 1 );
   controlsLayout->addWidget( b_progress,    row++, 1, 1, 5 );
   QLabel* lb_optspace1    = us_banner( "" );
   controlsLayout->addWidget( lb_optspace1,  row,   0, 1, 6 );
   controlsLayout->setRowStretch( row, 2 );

   row           = 0;
   optimizeLayout->addWidget( lb_minvari,    row,   0, 1, 2 );
   optimizeLayout->addWidget( le_minvari,    row++, 2, 1, 2 );
   optimizeLayout->addWidget( lb_minrmsd,    row,   0, 1, 2 );
   optimizeLayout->addWidget( le_minrmsd,    row++, 2, 1, 2 );
   optimizeLayout->addWidget( lb_statinfo,   row++, 0, 1, 4 );
   optimizeLayout->addWidget( te_status,     row,   0, 8, 4 );
   le_minrmsd->setMinimumWidth( lb_minrmsd->width() );
   te_status ->setMinimumWidth( lb_minrmsd->width()*4 );
   row    += 8;

   edata         = &dsets[ 0 ]->run_data;
   double vbar   = dsets[ 0 ]->vbar20;
   le_z_func->setText( QString::number( vbar ) );

   QLabel* lb_optspace     = us_banner( "" );
   optimizeLayout->addWidget( lb_optspace,   row,   0, 1, 4 );

   optimize_options();

   fitpars_connect( true );

   connect( ct_tralpha,  SIGNAL( valueChanged( double ) ),
            this,        SLOT(   set_alpha   ()         ) );

   connect( pb_pltlines, SIGNAL( clicked()    ),
            this,        SLOT(   plot_lines() ) );
   connect( pb_startfit, SIGNAL( clicked()    ),
            this,        SLOT(   fit_final()  ) );
   connect( pb_scanregp, SIGNAL( clicked()    ),
            this,        SLOT(   scan_alpha() ) );
   connect( pb_finalmdl, SIGNAL( clicked()    ),
            this,        SLOT(   final_only() ) );
   connect( pb_stopfit,  SIGNAL( clicked()    ),
            this,        SLOT(   stop_fit()   ) );
   connect( pb_plot,     SIGNAL( clicked()    ),
            this,        SLOT(   plot()       ) );
   connect( pb_save,     SIGNAL( clicked()    ),
            this,        SLOT(   save()       ) );
   connect( pb_advanaly, SIGNAL( clicked()    ),
            this,        SLOT(   advanced()   ) );
   connect( pb_help,     SIGNAL( clicked()    ),
            this,        SLOT(   help()       ) );
   connect( pb_close,    SIGNAL( clicked()    ),
            this,        SLOT(   close_all()  ) );

   pb_pltlines->setEnabled( false );
   compute();

//DbgLv(1) << "Pre-resize AC size" << size();
   int  fwidth   = fmet.maxWidth();
   int  rheight  = ct_lolimitx->height();
   int  cminw    = fwidth * 7;
   int  csizw    = cminw + fwidth;
   ct_lolimitx->setMinimumWidth( cminw );
   ct_uplimitx->setMinimumWidth( cminw );
   ct_lolimity->setMinimumWidth( cminw );
   ct_uplimity->setMinimumWidth( cminw );
   ct_varcount->setMinimumWidth( cminw );
   ct_gfiters ->setMinimumWidth( cminw );
   ct_gfthresh->setMinimumWidth( cminw );
   ct_lmmxcall->setMinimumWidth( cminw );
   ct_cresolu ->setMinimumWidth( cminw );
   ct_thrdcnt ->setMinimumWidth( cminw );
   ct_lolimitx->resize( csizw, rheight );
   ct_uplimitx->resize( csizw, rheight );
   ct_lolimity->resize( csizw, rheight );
   ct_uplimity->resize( csizw, rheight );
   ct_varcount->resize( csizw, rheight );
   ct_gfiters ->resize( csizw, rheight );
   ct_gfthresh->resize( csizw, rheight );
   ct_lmmxcall->resize( csizw, rheight );
   ct_cresolu ->resize( csizw, rheight );
   ct_thrdcnt ->resize( csizw, rheight );

   resize( 710, 440 );
   qApp->processEvents();

//DbgLv(1) << "Post-resize AC size" << size();
}

// enable/disable optimize counters based on chosen method
void US_AnalysisControlPc::optimize_options()
{
   bool use_noise = ( ct_tralpha->value() == 0.0 );
   ck_tinoise ->setEnabled( use_noise );
   ck_rinoise ->setEnabled( use_noise );

   adjustSize();
   qApp->processEvents();
}

// uncheck optimize options other than one just checked
void US_AnalysisControlPc::uncheck_optimize( int /*ckflag*/ )
{
}

// start fit button clicked
void US_AnalysisControlPc::start()
{
   US_pcsa* mainw = (US_pcsa*)parentw;
   mainw->analysis_done( -1 );        // Reset counters to zero

   // Make sure that ranges are reasonable
   if ( ( ct_uplimitx->value() - ct_lolimitx->value() ) < 0.0  ||
        ( ct_uplimity->value() - ct_lolimity->value() ) < 0.0 )
   {
      QString msg = 
         tr( "The \"s\" or \"f/f0\" ranges are inconsistent.\n"
             "Please re-check the limits and correct them\n"
             "before again clicking \"Start Fit\"." );

      QMessageBox::critical( this, tr( "Limits Inconsistent!" ), msg );
      return;
   }

   // Adjust some other parameters if Variations Count is 1.
   int    nvar   = (int)ct_varcount->value();
   if ( nvar == 1 )
   {
      ct_thrdcnt->setValue( 1.0 );
      ct_gfiters->setValue( 1.0 );
   }

   // Start a processing object if need be
   if ( processor == 0 )
   {
      need_fit    = true;
      need_final  = true;
      dsets[ 0 ]->solute_type  = sol_type;
      dsets[ 0 ]->zcoeffs[ 0 ] = le_z_func->text().section( " ", 0, 0 )
                                                  .toDouble();
      dsets[ 0 ]->zcoeffs[ 1 ] = 0.0;
      dsets[ 0 ]->zcoeffs[ 2 ] = 0.0;
      dsets[ 0 ]->zcoeffs[ 3 ] = 0.0;
DbgLv(1) << "AnaC: (A)zcoeff0" << dsets[0]->zcoeffs[0];

      processor   = new US_pcsaProcess( dsets, this );
   }

   else
   {
      processor->disconnect();
      if ( need_fit )
      {
         processor->stop_fit();
         processor->clear_memory();
      }
   }

   // Check that fit as parameterized will not exceed memory
   sdata      ->scanData.clear();
   rdata      ->scanData.clear();
   mw_mrecs   ->clear();
   mw_mrecs_mc->clear();
   mrecs       .clear();

   if ( memory_check() )
      return;

   // Check implied grid size does not exceed limits
   set_solute_type();
   QString smsg;
   double s_max  = ct_uplimitx->value() * 1.0e-13;

   if ( attr_x == US_ZSolute::ATTR_S  &&
        US_SolveSim::checkGridSize( dsets, s_max, smsg ) )
   {
      QMessageBox::critical( this,
         tr( "Implied Grid Size is Too Large!" ), smsg );
      return;
   }

   // Set up for the start of fit processing
   varimin       = 9e+99;
   bmndx         = -1;
   le_minvari  ->setText( "0.000e-05" );
   le_minrmsd  ->setText( "0.0000" );

   int    typ    = ctypes[ cb_curvtype->currentIndex() ];
   double slo    = ct_lolimitx->value();
   double sup    = ct_uplimitx->value();
   double klo    = ct_lolimity->value();
   double kup    = ct_uplimity->value();
   int    nthr   = (int)ct_thrdcnt ->value();
   int    noif   = ( ck_tinoise->isChecked() ? 1 : 0 ) +
                   ( ck_rinoise->isChecked() ? 2 : 0 );
   int    res    = (int)ct_cresolu ->value();
   double gfthr  = ct_gfthresh->value();
   int    gfits  = (int)ct_gfiters ->value();
   int    lmmxc  = (int)ct_lmmxcall->value();
   double alpha  = ct_tralpha ->value();

   // Alpha-scan completed:  test if we need to re-fit
DbgLv(1) << "AnaC: (1)need_fit" << need_fit;
   need_fit      = ( fitpars_string() != fitpars );
DbgLv(1) << "AnaC: (2)need_fit" << need_fit;

   if ( need_fit )
   {  // Not resuming after an Alpha scan
      if ( ! need_final )
         alpha         = -99.0;             // Flag Alpha scan
      else if ( ck_fxalpha->isChecked()  &&  alpha != 0.0 )
         alpha         = -alpha - 1.0;      // Flag use-alpha-for-fixed-fits
      else if ( ck_lmalpha->isChecked()  &&  alpha != 0.0 )
         alpha         = -alpha;            // Flag use-alpha-for-LM-fits

      mrecs.clear();
   }
DbgLv(1) << "AnaC: need_fit need_fnl" << need_fit << need_final
 << "alpha" << alpha;

   ti_noise->values.clear();
   ri_noise->values.clear();
   ti_noise->count = 0;
   ri_noise->count = 0;

   nctotal         = 10000;

   connect( processor, SIGNAL( progress_update(   double ) ),
            this,      SLOT(   update_progress(   double ) ) );
   connect( processor, SIGNAL( message_update(    QString, bool ) ),
            this,      SLOT(   progress_message(  QString, bool ) ) );
   connect( processor, SIGNAL( stage_complete(    int, int )  ),
            this,      SLOT(   reset_steps(       int, int )  ) );
   connect( processor, SIGNAL( process_complete(  int  ) ),
            this,      SLOT(   completed_process( int  ) ) );

   // Begin or resume the fit
   pb_startfit->setEnabled( false );
   pb_scanregp->setEnabled( false );
   pb_finalmdl->setEnabled( false );
   pb_stopfit ->setEnabled( true  );
   pb_plot    ->setEnabled( false );
DbgLv(1) << "(2)pb_plot-Enabled" << pb_plot->isEnabled();
   pb_save    ->setEnabled( false );
   qApp->processEvents();

   if ( need_fit )
      processor->start_fit( slo, sup, klo, kup, nvar, res, typ,
                            nthr, noif, lmmxc, gfits, gfthr, alpha );

   else if ( need_final )
      processor->final_fit( alpha );

   qApp->processEvents();
}

// stop fit button clicked
void US_AnalysisControlPc::stop_fit()
{
DbgLv(1) << "AC:SF:StopFit";
   b_progress->reset();

   pb_startfit->setEnabled( true );
   pb_scanregp->setEnabled( true );
   pb_finalmdl->setEnabled( true );
   pb_stopfit ->setEnabled( false );
   pb_plot    ->setEnabled( false );
   pb_save    ->setEnabled( false );
   qApp->processEvents();
DbgLv(1) << "(3)pb_plot-Enabled" << pb_plot->isEnabled();

   US_pcsa* mainw = (US_pcsa*)parentw;
   mainw->analysis_done( -1 );   // Reset counters to zero

   if ( processor != 0 )
   {
DbgLv(1) << "AC:SF: processor stopping...";
      processor->stop_fit();
      processor->clear_memory();
DbgLv(1) << "AC:SF: processor stopped";
   }

   qApp->processEvents();
}

// plot button clicked
void US_AnalysisControlPc::plot()
{
   *mw_mrecs       = mrecs;
   *model          = mrecs[ 0 ].model;
   US_pcsa* mainw = (US_pcsa*)parentw;
   mainw->analysis_done( 1 );     // Plot in main
}

// advanced controls button clicked
void US_AnalysisControlPc::advanced()
{
   US_pcsa* mainw = (US_pcsa*)parentw;
DbgLv(1) << "AC:advanced";
DbgLv(1) << "AC:advanced  mrecs.size" << mrecs.size();
if(mrecs.size()>0)
 DbgLv(1) << "AC:advanced mrecs0 p1 p2" << mrecs[0].par1 << mrecs[0].par2
  << "ctype" << mrecs[0].ctype;
   int    nthr   = (int)ct_thrdcnt ->value();

   US_AdvAnalysisPc* aadiag = new US_AdvAnalysisPc( &mrecs, nthr,
                                                    dsets[ 0 ], this );

   if ( aadiag->exec() == QDialog::Accepted )
   {
DbgLv(1) << "AC:advanced dialog exec() return - ACCEPTED";
      int      state  = aadiag->advanced_results( &mrecs_mc );
      bool     mrsupd = ( ( state & 3 ) != 0 );
      bool     mmcupd = ( ( state & 4 ) != 0 );
      int      ncsols = mrsupd ? mrecs[ 0 ].csolutes.size() : 0;
      double   rmsdf  = mrsupd ? mrecs[ 0 ].rmsd : 0.0;
      double   varif  = mrsupd ? mrecs[ 0 ].variance : 0.0;
      int      mciter = mmcupd ? mrecs_mc.size() : 0;
DbgLv(1) << "AC:advanced dialog state=" << state << "mainw" << mainw;
DbgLv(1) << "AC:adv:(1)rmsd" << mrecs[0].rmsd;

      if ( ck_tinoise->isChecked()  ||  ck_rinoise->isChecked() )
      {
         QMessageBox::warning( this, tr( "Problematic combination!" ),
            tr( "The combination of one or two noise fits selected and "
                "Advanced Controls chosen is problematic, since model "
                "records only specify fitting curves and not noise "
                "parameters.\n\n"
                "You may proceed, but the safest course is to click on "
                "the Recompute Best Model button to re-fit with noises.\n\n"
                "Any Save Results where noise is involved is better "
                "executed *before* Advanced Controls." ) );
      }

      // Update model recs where possible and appropriate
      if ( mrsupd )
      {
         *mw_mrecs       = mrecs;
         *model          = mrecs[ 0 ].model;
         *sdata          = mrecs[ 0 ].sim_data;
         *rdata          = mrecs[ 0 ].residuals;
         *mw_mrecs_mc    = mrecs_mc;
      }
DbgLv(1) << "AC:adv:(2)rmsd" << mrecs[0].rmsd;

      if ( mmcupd )
      {
         *mw_mrecs_mc    = mrecs_mc;
      }
DbgLv(1) << "AC:advanced: mrec0 sols" << mrecs[0].csolutes.size()
 << "mrecs size" << mrecs.size() << "mrecs_mc size" << mrecs_mc.size()
 << "model compsize" << model->components.size();

      if ( mmcupd )
      {  // Report new BFM from MonteCarlo
         QString fmsg = tr(
            "\nA newer best model has been created by %1 Monte Carlo\n"
            "     iterations  ( %2-solute, with RMSD = %3 )" )
            .arg( mciter ).arg( ncsols ).arg( rmsdf );

         progress_message( fmsg, true );
         le_minvari->setText( QString::number( varif ) );
         le_minrmsd->setText( QString::number( rmsdf ) );
      }

      else if ( mrsupd )
      {  // Report new BFM from other advanced controls action
         QString fmsg = tr(
            "\nA newer best model has been created from Advanced Controls\n"
            "    action  ( %1-solute, with RMSD = %2 )" )
            .arg( ncsols ).arg( rmsdf );

         progress_message( fmsg, true );
         le_minvari->setText( QString::number( varif ) );
         le_minrmsd->setText( QString::number( rmsdf ) );
      }
DbgLv(1) << "AC:adv:(3)rmsd" << mrecs[0].rmsd;

      if ( state != 0 )
      {  // Where advanced controls have made changes, copy them
         pb_startfit->setEnabled( true  );
         pb_scanregp->setEnabled( true  );
         pb_finalmdl->setEnabled( true  );
         pb_stopfit ->setEnabled( false );
         pb_plot    ->setEnabled( true  );
         pb_save    ->setEnabled( true  );
         pb_pltlines->setEnabled( true  );
         need_fit      = false;
         need_final    = true;
         bmndx         = mrecs[ 0 ].taskx;
DbgLv(1) << "AC:adv:(4)rmsd" << mrecs[0].rmsd;

         if ( processor == 0 )
         {

            processor   = new US_pcsaProcess( dsets, this );
         }

         dsets[ 0 ]->solute_type  = sol_type;
         dsets[ 0 ]->zcoeffs[ 0 ] = le_z_func->text().section( " ", 0, 0 )
                                                     .toDouble();
         dsets[ 0 ]->zcoeffs[ 1 ] = 0.0;
         dsets[ 0 ]->zcoeffs[ 2 ] = 0.0;
         dsets[ 0 ]->zcoeffs[ 3 ] = 0.0;

DbgLv(1) << "AC: (B)stype" << sol_type << "zcoeff0" << dsets[0]->zcoeffs[0];
DbgLv(1) << "AC:advanced: put_mrecs";
DbgLv(1) << "AC:adv:putm: rmsd" << mrecs[0].rmsd;
         processor->put_mrecs( mrecs );
DbgLv(1) << "AC:advanced: get_results";
         processor->get_results( sdata, rdata, model, ti_noise, ri_noise,
               bmndx, *mw_modstats, mrecs );

         ctype         = mrecs[ 0 ].ctype;
         int v_ctype   = mrecs[ 0 ].v_ctype;
         int nmrecs    = mrecs.size();
         int nmtsks    = ( mrecs[ 0 ].taskx == mrecs[ 1 ].taskx )
                       ? ( nmrecs - 1 ) : nmrecs;
         nmtsks        = ( mrecs[ 1 ].taskx == mrecs[ 2 ].taskx )
                       ? ( nmrecs - 1 ) : nmrecs;
         int strec     = nmrecs - nmtsks;
         nypts         = ( v_ctype != CTYPE_ALL ) ? nmtsks : ( nmtsks / 3 );
         nypts         = ( ctype != CTYPE_HL  &&  ctype != CTYPE_2O )
                       ? qRound( sqrt( (double)nypts ) ) : nypts;
         nypts         = ( ctype != CTYPE_2O ) ? nypts
                       : qRound( pow( (double)nypts, 0.33333 ) );
         nlpts         = mrecs[ strec ].isolutes.size();
         xmin          = mrecs[ strec ].xmin;
         xmax          = mrecs[ strec ].xmax;
         ymin          = mrecs[ strec ].ymin;
         ymax          = mrecs[ strec ].ymax;
         int stype     = mrecs[ 0 ].stype;
         int attr_x    = ( stype >> 6 ) & 7;
         int attr_y    = ( stype >> 3 ) & 7;
         double xscl   = ( attr_x == US_ZSolute::ATTR_S ) ? 1.0e+13 : 1.0;
         double yscl   = ( attr_y == US_ZSolute::ATTR_S ) ? 1.0e+13 : 1.0;

         for ( int ii = strec; ii < nmrecs; ii++ )
         {
            QVector< US_ZSolute >* isolutes = &mrecs[ ii ].isolutes;
            nlpts         = isolutes->size();
            ymin          = qMin( ymin, mrecs[ ii ].str_y );
            ymax          = qMax( ymax, mrecs[ ii ].end_y );

            for ( int jj = 0; jj < nlpts; jj++ )
            {
               double xval   = (*isolutes)[ jj ].x * xscl;
               double yval   = (*isolutes)[ jj ].y * yscl;
               xmin          = qMin( xmin, xval );
               xmax          = qMax( xmax, xval );
               ymin          = qMin( ymin, yval );
               ymax          = qMax( ymax, yval );
            }
         }

         fitpars_connect( false );

         cb_curvtype->setCurrentIndex( ctypes.indexOf( ctype ) );
         ct_lolimitx->setValue( xmin  );
         ct_uplimitx->setValue( xmax  );
         ct_lolimity->setValue( ymin  );
         ct_uplimity->setValue( ymax  );
         ct_varcount->setValue( nypts );
         ct_cresolu ->setValue( nlpts );

         fitpars       = fitpars_string();
         fitpars_connect( true );

         mainw->analysis_done( 0 );   // Tell main that new records are done
      }
   }
else
DbgLv(1) << "AC:advanced dialog exec() return - CANCELED";
   delete aadiag;
}

// save button clicked
void US_AnalysisControlPc::save()
{
   US_pcsa* mainw = (US_pcsa*)parentw;
DbgLv(1) << "AC:save: model components size" << model->components.size();
   *mw_mrecs       = mrecs;
   if ( mw_mrecs_mc->size() == 0 )
      *model          = mrecs[ 0 ].model;
   else
      *model          = mw_mrecs_mc->at( 0 ).model;
   mainw->analysis_done( 2 );         // Save in main
}

// Close all windows
void US_AnalysisControlPc::close_all()
{
DbgLv(1) << "AC:close: mlnplotd" << mlnplotd;
   if ( (QObject*)mlnplotd != (QObject*)0 )
      mlnplotd->close();

   if ( processor != 0 )
      delete processor;

   accept();
}

// Public close slot
void US_AnalysisControlPc::close()
{
   close_all();
}

// Adjust s-limit ranges when x-limit value changes
void US_AnalysisControlPc::xlim_change()
{
   double loval  = ct_lolimitx->value();
   double upval  = ct_uplimitx->value();
   double limlo  = -1.e6;
   double limup  = 1.e6;
   if ( loval > upval )   // Insure lower value less than upper
      ct_lolimitx->setValue( upval );

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

   ct_lolimitx->setRange( limlo, upval );
   ct_uplimitx->setRange( loval, limup );
   ct_lolimitx->setSingleStep( 0.1 );
   ct_uplimitx->setSingleStep( 0.1 );
}

// Set k-upper-limit to lower when y grid points == 1
void US_AnalysisControlPc::ylim_change()
{
   compute();
}

// Handle change in resolution count
void US_AnalysisControlPc::reso_change()
{
DbgLv(1) << "RESO_CHANGE: need_fit" << need_fit
 << "reso" << ct_cresolu ->value();
   if ( need_fit )
      compute();
}

// Handle change in curve type
void US_AnalysisControlPc::type_change()
{
   ctypex  = cb_curvtype->currentIndex();
   ctype   = ctypes[ ctypex ];
   bmndx   = -1;
DbgLv(1) << "TYPE_CHANGE: ctypex" << ctypex << "ctype" << ctype;

   if ( ctype == CTYPE_HL )
      ct_varcount->setValue( 100 );
   else if ( ctype == CTYPE_2O )
      ct_varcount->setValue( 5 );
   else
      ct_varcount->setValue( 6 );
}

// Set regularization factor alpha
void US_AnalysisControlPc::set_alpha()
{
   bool regular   = ( ct_tralpha->value() != 0.0 );
   ck_tinoise ->setEnabled( !regular );
   ck_rinoise ->setEnabled( !regular );
   ck_lmalpha ->setEnabled( regular );
   ck_fxalpha ->setEnabled( regular );

   if ( regular )
   {
      pb_finalmdl->setText( tr( "Regularize Current Model" ) );
      ck_tinoise->setChecked( false );
      ck_rinoise->setChecked( false );
   }

   else
   {
      pb_finalmdl->setText( tr( "Recompute Best Model"     ) );
   }

   if ( ct_tralpha->value() >= 10.0 )
      ct_tralpha->setSingleStep( 0.01 );
   else
      ct_tralpha->setSingleStep( 0.001 );
}

// Slot to handle progress update
void US_AnalysisControlPc::update_progress( double variance )
{
   ncsteps ++;

   if ( ncsteps > nctotal )
   {
      nctotal  = ( nctotal * 11 ) / 10;
      b_progress->setMaximum( nctotal );
   }

   b_progress->setValue( ncsteps );
DbgLv(2) << "UpdPr: ncs nts vari" << ncsteps << nctotal << variance;
   double rmsd = sqrt( variance );
   if ( variance < varimin )
   {
      varimin = variance;
      le_minvari->setText( QString::number( varimin ) );
      le_minrmsd->setText( QString::number( rmsd    ) );
   }
}

// slot to handle updated progress message
void US_AnalysisControlPc::progress_message( QString pmsg, bool append )
{
   QString amsg;

   if ( append )
   {  // append to existing progress message
      amsg   = te_status->toPlainText() + pmsg;
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
void US_AnalysisControlPc::reset_steps( int kcs, int nct )
{
DbgLv(1) << "AC:cs: prmx nct kcs" << b_progress->maximum() << nct << kcs;
   ncsteps      = kcs;
   nctotal      = nct;

   b_progress->setMaximum( nctotal );
   b_progress->setValue(   ncsteps );

   qApp->processEvents();
}

// slot to handle completed processing
void US_AnalysisControlPc::completed_process( int stage )
{
   US_pcsa* mainw = (US_pcsa*)parentw;
DbgLv(1) << "AC:cp: stage" << stage;

   if ( stage == 7 )
   { // If an alpha scan can now be done, report L-M info and mark scan-ready
      US_ModelRecord mrec;
      processor->get_mrec( mrec );
      double vari   = mrec.variance;
      double rmsd   = mrec.rmsd;
      le_minvari->setText( QString::number( vari ) );
      le_minrmsd->setText( QString::number( rmsd ) );
DbgLv(1) << "AC:cp: mrec fetched";

      // Assume we can compute the final and save current fit parameters
      need_fit      = false;
      need_final    = true;
      fitpars       = fitpars_string();
      return;
   }

   if ( stage == 8 )
   { // If starting L-M, turn off Stop Fit
      pb_stopfit->setEnabled( false );
      return;
   }

   if ( stage == 6 )
   {  // If stopped because of memory usage, execute stop_fit
      stop_fit();
      return;
   }

   reset_steps( nctotal, nctotal );
   QStringList modelstats;
   mrecs.clear();

   processor->get_results( sdata, rdata, model, ti_noise, ri_noise, bmndx,
         modelstats, mrecs );
DbgLv(1) << "AC:cp: RES: ti,ri counts" << ti_noise->count << ri_noise->count;
DbgLv(1) << "AC:cp: RES: bmndx" << bmndx;

   plot_lines();

   if ( stage == 9 )
   {
      *mw_modstats    = modelstats;
      *mw_mrecs       = mrecs;

DbgLv(1) << "AC:cp: main done -2";
      mainw->analysis_done( -2 );     // Update RMSD in main

DbgLv(1) << "AC:cp: main done 0";
      mainw->analysis_done(  0 );     // Tell main that records are done

      pb_startfit->setEnabled( true  );
      pb_scanregp->setEnabled( true  );
      pb_finalmdl->setEnabled( true  );
      pb_stopfit ->setEnabled( false );
      pb_plot    ->setEnabled( true  );
      pb_save    ->setEnabled( true  );
      pb_pltlines->setEnabled( true  );
DbgLv(1) << "(1)pb_plot-Enabled" << pb_plot->isEnabled();
      need_fit      = false;
      need_final    = false;
      fitpars       = fitpars_string();

      double vari   = mrecs[ 0 ].variance;
      double rmsd   = mrecs[ 0 ].rmsd;
      le_minvari->setText( QString::number( vari ) );
      le_minrmsd->setText( QString::number( rmsd ) );
   }
}

// slot to compute model lines
void US_AnalysisControlPc::compute()
{
   double parlims[ 12 ];

   set_solute_type();

   ctypex  = cb_curvtype->currentIndex();
   ctype   = ctypes[ ctypex ];
   xmin    = ct_lolimitx->value();
   xmax    = ct_uplimitx->value();
   ymin    = ct_lolimity->value();
   ymax    = ct_uplimity->value();
   nypts   = (int)ct_varcount->value();
   nlpts   = (int)ct_cresolu ->value();
   mrecs.clear();
   parlims[ 0 ]   = -1.0;
   parlims[ 4 ]   = sol_type;
   parlims[ 5 ]   = le_z_func->text().section( " ", 0, 0 ).toDouble();
   int    nlmodl  = nypts * nypts;
DbgLv(1) << "AC:CM: ctype" << ctype << "stype" << sol_type
 << US_ModelRecord::stype_text(sol_type);

   if ( ctype == CTYPE_SL )
   {
      US_ModelRecord::compute_slines( xmin, xmax, ymin, ymax, nypts, nlpts,
            parlims, mrecs );
   }
   else if ( ctype == CTYPE_IS  ||  ctype == CTYPE_DS )
   {
      US_ModelRecord::compute_sigmoids( ctype, xmin, xmax, ymin, ymax,
            nypts, nlpts, parlims, mrecs );
   }
   else if ( ctype == CTYPE_HL )
   {
      nlmodl         = nypts;

      US_ModelRecord::compute_hlines( xmin, xmax, ymin, ymax, nypts, nlpts,
            parlims, mrecs );
   }
   else if ( ctype == CTYPE_2O )
   {
      nlmodl        *= nypts;

      US_ModelRecord::compute_2ndorder( xmin, xmax, ymin, ymax,
            nypts, nlpts, parlims, mrecs );
   }
DbgLv(1) << "AC:CM: mrecs0.sol0 x y z c"
 << mrecs[0].isolutes[0].x << mrecs[0].isolutes[0].y
 << mrecs[0].isolutes[0].z << mrecs[0].isolutes[0].c;

   QString amsg   =
      tr( "The number of test models is %1,\n" ).arg( nlmodl );
   if ( ctype == CTYPE_2O )
   {
      amsg       += tr( " derived from the cube of %1 variation points,\n" )
                    .arg( nypts );
   }
   else if ( ctype != 8 )
   {
      amsg       += tr( " derived from the square of %1 variation points,\n" )
                    .arg( nypts );
   }
   amsg          += tr( " with each curve model consisting of %1 points." )
                    .arg( nlpts );
   te_status  ->setText( amsg );

   bmndx          = -1;
   need_fit       = ( fitpars != fitpars_string() );
DbgLv(1) << "AC:CM: need_fit" << need_fit;
   pb_pltlines->setEnabled( true );
   pb_scanregp->setEnabled( !need_fit );
   pb_finalmdl->setEnabled( !need_fit );
}

// slot to launch a plot dialog showing model lines
void US_AnalysisControlPc::plot_lines()
{
   ctypex   = cb_curvtype->currentIndex();
   ctype    = ( mrecs[ 0 ].v_ctype == CTYPE_ALL )
              ? CTYPE_ALL : ctypes[ ctypex ];
   xmin     = ct_lolimitx->value();
   xmax     = ct_uplimitx->value();
   ymin     = ct_lolimity->value();
   ymax     = ct_uplimity->value();
   nlpts    = (int)ct_cresolu ->value();
   nypts    = (int)ct_varcount->value();
   sol_type = ( attr_x << 6 ) + ( attr_y << 3 ) + attr_z;

   mrecs[ 0 ].stype  = sol_type;

DbgLv(1) << "PL: mlnplotd" << mlnplotd;
   if ( mlnplotd != 0 )
      mlnplotd->close();
DbgLv(1) << "PL:  mlnplotd closed";

   mlnplotd = new US_MLinesPlot( ymin, ymax, xmin, xmax, ctype,
                                 nypts, nlpts, bmndx );

   connect( mlnplotd, SIGNAL( destroyed( QObject* ) ),
            this,     SLOT  ( closed   ( QObject* ) ) );

DbgLv(1) << "PL:  new mlnplotd" << mlnplotd << "sol_type" << sol_type;

   if ( bmndx >= 0 )
   {
DbgLv(1) << "PL:    mlp: setModel bmndx" << bmndx;
      mlnplotd->setModel( model, mrecs );
   }
   else
   {
DbgLv(1) << "PL:    mlp: setModel model=0";
      mlnplotd->setModel( 0, mrecs );
   }

DbgLv(1) << "PL:    mlp: call plot_data";
   mlnplotd->plot_data();
DbgLv(1) << "PL:    mlp:  rtn fr plot_data";
   mlnplotd->setVisible( true );

   QString filepath = US_Settings::tmpDir() + "/PCSA."
                      + edata->cell + edata->channel + edata->wavelength
                      + ".mlines."
                      + QString::number( getpid() ) + ".png";
#if QT_VERSION > 0x050000
   QPixmap pixmap   = ((QWidget*)mlnplotd)->grab();
#else
   QPixmap pixmap   = QPixmap::grabWidget( mlnplotd, 0, 0,
                         mlnplotd->width(), mlnplotd->height() );
#endif
DbgLv(0) << "PLOTLINE: mlines filepath" << filepath;
DbgLv(0) << "PLOTLINE: mlines w h" << pixmap.width() << pixmap.height();
   pixmap.save( filepath );
}

// Private slot to mark a child widget as closed, if it has been destroyed
void US_AnalysisControlPc::closed( QObject* o )
{
   QString oname = o->objectName();

   if ( oname.contains( "MLinesPlot" ) )
      mlnplotd    = 0;
}

// Set flags and start fit where fits and final computation are needed
void US_AnalysisControlPc::fit_final( void )
{
   need_fit    = true;
   need_final  = true;
   fitpars     = QString();

   start();
}

// Set flags and open the dialog to do an Alpha scan
void US_AnalysisControlPc::scan_alpha( void )
{
   US_ModelRecord mrec;
   need_fit        = false;
   need_final      = false;

   int    nthr     = (int)ct_thrdcnt ->value();
   int    klpts    = (int)ct_cresolu ->value();
   double alpha    = 0.0;
DbgLv(1) << "AC:sa: nthr klp nlp" << nthr << klpts << nlpts;

   if ( klpts != nlpts )
   {  // Must recompute final LM model since resol. points value has changed
      recompute_mrec();
      mrec            = mrecs[ 0 ];
      nlpts           = klpts;
   }

   else if ( processor != 0 )
      processor->get_mrec( mrec );

   US_RpScan* rpscand = new US_RpScan( dsets, mrec, nthr, alpha );
DbgLv(1) << "AC:sa: RpScan created";

   if ( rpscand->exec() == QDialog::Accepted )
   {
DbgLv(1) << "AC:sa: alpha fetched" << alpha;
      ct_tralpha ->setValue( alpha );
   }

   pb_finalmdl->setEnabled( true  );
DbgLv(1) << "AC:sa: RpScan deleting";
   delete rpscand;
   rpscand   = NULL;
DbgLv(1) << "AC:sa: RpScan deleted";

   // Assume we can compute the final and save current fit parameters
   fitpars     = fitpars_string();
   nlpts       = (int)ct_cresolu ->value();
}

// Set flags and start fit where only final computation is needed
void US_AnalysisControlPc::final_only( void )
{
   need_fit    = false;
   need_final  = true;
   int klpts   = (int)ct_cresolu ->value();
DbgLv(1) << "AC:fo: klp nlp" << klpts << nlpts;

   if ( klpts != nlpts )
   {  // Must recompute final LM model since resol. points value has changed
      recompute_mrec();
      nlpts           = klpts;
   }

   start();
}

// Compose a string showing the current settings for fit parameters
QString US_AnalysisControlPc::fitpars_string()
{
   int    typ    = cb_curvtype->currentIndex();
   double xlo    = ct_lolimitx->value();
   double xup    = ct_uplimitx->value();
   double ylo    = ct_lolimity->value();
   double yup    = ct_uplimity->value();
   int    nvar   = (int)ct_varcount->value();
   int    noif   = ( ck_tinoise->isChecked() ? 1 : 0 ) +
                   ( ck_rinoise->isChecked() ? 2 : 0 );


   return QString().sprintf( "%d %.5e %.5e %.5e %.5e %d %d",
            typ, xlo, xup, ylo, yup, nvar, noif );
}

// Re-Connect or disconnect fit parameters controls
void US_AnalysisControlPc::fitpars_connect( bool reconn )
{
   if ( reconn )
   {  // Reconnect controls
      connect( bg_x_axis,   SIGNAL( buttonReleased( int )    ),
               this,        SLOT(   select_x_axis ( int )    ) );
      connect( bg_y_axis,   SIGNAL( buttonReleased( int )    ),
               this,        SLOT(   select_y_axis ( int )    ) );
      connect( cb_curvtype, SIGNAL( activated     ( int )    ),
               this,        SLOT(   type_change()            ) );
      connect( ct_lolimitx, SIGNAL( valueChanged  ( double ) ),
               this,        SLOT(   xlim_change()            ) );
      connect( ct_uplimitx, SIGNAL( valueChanged  ( double ) ),
               this,        SLOT(   xlim_change()            ) );
      connect( ct_lolimity, SIGNAL( valueChanged  ( double ) ),
               this,        SLOT(   ylim_change()            ) );
      connect( ct_uplimity, SIGNAL( valueChanged  ( double ) ),
               this,        SLOT(   ylim_change()            ) );
      connect( cb_z_type,   SIGNAL( activated     ( int )    ),
               this,        SLOT(   ztype_change  ( int )    ) );
      connect( ct_varcount, SIGNAL( valueChanged  ( double ) ),
               this,        SLOT(   compute()                ) );
      connect( ct_cresolu,  SIGNAL( valueChanged  ( double ) ),
               this,        SLOT(   reso_change()            ) );
   }

   else
   {  // Disconnect controls
      bg_x_axis  ->disconnect();
      bg_y_axis  ->disconnect();
      cb_curvtype->disconnect();
      ct_lolimitx->disconnect();
      ct_uplimitx->disconnect();
      ct_lolimity->disconnect();
      ct_uplimity->disconnect();
      cb_z_type  ->disconnect();
      ct_varcount->disconnect();
      ct_cresolu ->disconnect();
   }
}

// Recompute the top model record after change to resolution points
void US_AnalysisControlPc::recompute_mrec()
{
   US_ModelRecord mrec  = mrecs[ 0 ];
int nn=mrec.isolutes.size()-1;
int mm=mrec.isolutes.size()/2;
DbgLv(1) << "AC:RM: mrec0 solsize" << mrec.isolutes.size()
 << "s0 x,y" << mrec.isolutes[0].x << mrec.isolutes[0].y
 << "sm x,y" << mrec.isolutes[mm].x << mrec.isolutes[mm].y
 << "sn x,y" << mrec.isolutes[nn].x << mrec.isolutes[nn].y;
   mrec.isolutes.clear();
   US_ZSolute isol;
   xmin          = ct_lolimitx->value();
   xmax          = ct_uplimitx->value();
   ymin          = ct_lolimity->value();
   ymax          = ct_uplimity->value();
   nlpts         = (int)ct_cresolu ->value();
   ctypex        = cb_curvtype->currentIndex();
   ctype         = ctypes[ ctypex ];
   mrec.ctype    = ctype;
   mrec.xmin     = xmin;
   mrec.xmax     = xmax;
   mrec.ymin     = ymin;
   mrec.ymax     = ymax;
   double str_y  = mrec.str_y;
   double end_y  = mrec.end_y;
   double par1   = mrec.par1;
   double par2   = mrec.par2;
   double par3   = mrec.par3;
   double prng   = (double)( nlpts - 1 );
   double xrng   = xmax - xmin;
   int stype     = mrec.stype;
   int attr_x    = ( stype >> 6 ) & 7;
   int attr_y    = ( stype >> 3 ) & 7;
   double xscl   = ( attr_x == US_ZSolute::ATTR_S ) ? 1.0e-13 : 1.0;
   double yscl   = ( attr_y == US_ZSolute::ATTR_S ) ? 1.0e-13 : 1.0;
   isol.z        = le_z_func->text().section( " ", 0, 0 ).toDouble();

   if ( ctype == CTYPE_SL )
   {
      double xval   = xmin;
      double xinc   = xrng / prng;
      double yval   = str_y;
      double yinc   = ( end_y - str_y ) / prng;

      for ( int kk = 0; kk < nlpts; kk++ )
      { // Loop over points on a line
         isol.x      = xval * xscl;
         isol.y      = yval * yscl;
         mrec.isolutes << isol;
         xval       += xinc;
         yval       += yinc;
      } // END: points-per-line loop
   }

   else if ( ctype == CTYPE_IS  ||  ctype == CTYPE_DS )
   {
      double xrng   = xmax - xmin;
      double ystr   = ( ctype == CTYPE_IS ) ? ymin : ymax;
      double ydif   = ymax - ymin;
      ydif          = ( ctype == CTYPE_IS ) ? ydif : -ydif;
      double xoff   = 0.0;
      double xoinc  = 1.0 / prng;
      double p1rt   = sqrt( 2.0 * par1 );

      for ( int kk = 0; kk < nlpts; kk++ )
      { // Loop over points on a sigmoid curve
         double xval  = xmin + xoff * xrng;
         double efac  = 0.5 * erf( ( xoff - par2 ) / p1rt ) + 0.5;
         double yval  = ystr + ydif * efac;
         isol.x       = xval * xscl;
         isol.y       = yval * yscl;
         mrec.isolutes << isol;
         xoff        += xoinc;
      } // END: points-on-curve loop
   }

   else if ( ctype == CTYPE_HL )
   {
      double xval   = xmin;
      double xinc   = xrng / prng;
      double yval   = end_y;

      for ( int kk = 0; kk < nlpts; kk++ )
      { // Loop over points on a line
         isol.x        = xval * xscl;
         isol.y        = yval * yscl;
         mrec.isolutes << isol;
         xval         += xinc;
      } // END: points-per-line loop
   }

   else if ( ctype == CTYPE_2O )
   {
      double xval   = xmin;
      double xinc   = xrng / prng;

      for ( int ll = 0; ll < nlpts; ll++ )
      { // Loop over points on a power law curve
         isol.x        = xval * xscl;
         double yval   = par1 * pow( xval, par2 ) + par3;
         isol.y        = yval * yscl;
         mrec.isolutes << isol;
         xval         += xinc;
      } // END: points-per-line loop
   }

   mrecs[ 0 ] = mrec;
nn=mrec.isolutes.size()-1;
mm=mrec.isolutes.size()/2;
DbgLv(1) << "AC:RM: NEW mrec0 solsize" << mrec.isolutes.size()
 << "s0 x,y" << mrec.isolutes[0].x << mrec.isolutes[0].y
 << "sm x,y" << mrec.isolutes[mm].x << mrec.isolutes[mm].y
 << "sn x,y" << mrec.isolutes[nn].x << mrec.isolutes[nn].y;
   if ( processor != 0 )
      processor->put_mrec( mrec );
}

int US_AnalysisControlPc::memory_check()
{
   const double mb_fact = ( 1024. * 1024. );
   const double x_fact  = 19.25;
   const double y_fact  = 2.43;
   const int    pc_ava  = 90;
   const int    nxdata  = 4;
   int    baserss  = *mw_baserss;
   if ( baserss == 0 )
   {
      baserss         = qRound( (double)US_Memory::rss_now() / 1024. );
      *mw_baserss     = baserss;
   }
   int    status   = 0;
   int    nscans   = edata->scanCount();
   int    npoints  = edata->pointCount();
   long int    idsize   = (long) nscans * npoints * sizeof( double );
   int    nsols    = (int)ct_cresolu ->value();
   int    nthrds   = (int)ct_thrdcnt ->value();
   double dsize    = (double)idsize * (double)( nsols + nxdata ) / mb_fact;
   double msize    = ( x_fact + dsize * y_fact ) * (double)nthrds;
   int    memneed  = baserss + qRound( msize );
   int    memtot, memava, memuse;
   int    mempca   = US_Memory::memory_profile( &memava, &memtot, &memuse );
   int    memsafe  = ( memava * pc_ava ) / 100;
int idsz = qRound(dsize);
int imsz = qRound(msize);
DbgLv(1) << "MEMck: memtot,ava,use,safe,need" << memtot << memava << memuse
 << memsafe << memneed << "dsz msz ns nt" << idsz << imsz << nsols << nthrds;

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
                          "Curve Resolution Points and/or\n"
                          "Thread Count." ) );
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
                             "Curve Resolution Points and/or\n"
                             "Thread Count.\n\nProceed?" ) );
         msgBox.addButton( QMessageBox::No );
         msgBox.addButton( QMessageBox::Yes );
         msgBox.setDefaultButton( QMessageBox::No );

         if ( msgBox.exec() == QMessageBox::No )
            status          = 2;
      }
   }

   return status;
}

// Select the coordinate for the horizontal axis
void US_AnalysisControlPc::select_x_axis( int ival )
{
qDebug() << "SelX: ival" << ival;
   attr_x         = ival;
   fitpars_connect( false );
qDebug() << "SelX:  attr_x attr_y" << attr_x << attr_y;

   if ( attr_x == attr_y )
   {  // If new X same as current Y, change Y
      attr_y         = ( attr_y < ATTR_D ) ? ( attr_y + 1 ) : ATTR_S;
qDebug() << "SelX:  NEW attr_y" << attr_y;

      if ( attr_y == ATTR_S )
         rb_y_s   ->setChecked( true );
      else if ( attr_y == ATTR_K )
         rb_y_ff0 ->setChecked( true );
      else if ( attr_y == ATTR_W )
         rb_y_mw  ->setChecked( true );
      else if ( attr_y == ATTR_V )
         rb_y_vbar->setChecked( true );
      else if ( attr_y == ATTR_D )
         rb_y_D   ->setChecked( true );

      adjust_xyz( 2 );
   }

   // Disable the Y that matches X
   rb_y_s   ->setEnabled( attr_x != ATTR_S );
   rb_y_ff0 ->setEnabled( attr_x != ATTR_K );
   rb_y_mw  ->setEnabled( attr_x != ATTR_W );
   rb_y_vbar->setEnabled( attr_x != ATTR_V );
   rb_y_D   ->setEnabled( attr_x != ATTR_D );

   // Change Z selection if it duplicates X or Y
   if ( attr_z == attr_x  ||  attr_z == attr_y )
   {
      if ( attr_x != ATTR_V  &&  attr_y != ATTR_V )
      {
         cb_z_type  ->setCurrentIndex( 0 );
         attr_z         = ATTR_V;
      }
      else if ( attr_x != ATTR_K  &&  attr_y != ATTR_K )
      {
         cb_z_type  ->setCurrentIndex( 1 );
         attr_z         = ATTR_K;
      }
      else if ( attr_x != ATTR_W  &&  attr_y != ATTR_W )
      {
         cb_z_type  ->setCurrentIndex( 2 );
         attr_z         = ATTR_W;
      }

      adjust_xyz( 0 );
   }

   // Adjust XYZ ranges and values
   adjust_xyz( 1 );
   fitpars_connect( true  );
}

// Select the coordinate for the vertical axis
void US_AnalysisControlPc::select_y_axis( int ival )
{
qDebug() << "SelY: ival" << ival;
   attr_y         = ival;
   fitpars_connect( false );

   // Change Z selection if it duplicates X or Y
   if ( attr_z == attr_x  ||  attr_z == attr_y )
   {
      if ( attr_x != ATTR_V  &&  attr_y != ATTR_V )
      {
         cb_z_type  ->setCurrentIndex( 0 );
         attr_z         = ATTR_V;
      }
      else if ( attr_x != ATTR_K  &&  attr_y != ATTR_K )
      {
         cb_z_type  ->setCurrentIndex( 1 );
         attr_z         = ATTR_K;
      }
      else if ( attr_x != ATTR_W  &&  attr_y != ATTR_W )
      {
         cb_z_type  ->setCurrentIndex( 2 );
         attr_z         = ATTR_W;
      }

      adjust_xyz( 0 );
   }

   // Adjust XYZ ranges and values
   adjust_xyz( 2 );
   fitpars_connect( true  );
}

// Set Z type
void US_AnalysisControlPc::ztype_change( int newx )
{
   attr_z         = ATTR_V;
   attr_z         = ( newx == 1 ) ? ATTR_K : attr_z;
   attr_z         = ( newx == 2 ) ? ATTR_W : attr_z;

   fitpars_connect( false );
   adjust_xyz( 0 );
   fitpars_connect( true  );
}

// Adjust X,Y,Z ranges and default values based on type
void US_AnalysisControlPc::adjust_xyz( const int chg_ndx )
{
   //               Types:        "s", "f/f0",  "MW", "vbar", "D"
   const double  vllos[] = {      1.0,   1.0,   2e+4,  0.60, 1e-8 };
   const double  vlhis[] = {     10.0,   4.0,   1e+5,  0.80, 1e-7 };
   const double  vdefs[] = {      5.0,   1.5,   5e+4,  0.72, 5e-8 };
   const double  vmins[] = { -10000.0,   1.0,    0.0,  0.01, 1e-9 };
   const double  vmaxs[] = {  10000.0,  50.0,  1e+10,  3.00, 1e-5 };
   const double  vincs[] = {     0.01,  0.01, 1000.0,  0.01, 1e-9 };

   // Get X,Y,Z ranges and values
   double xlow    = vllos[ attr_x ];
   double xhigh   = vlhis[ attr_x ];
   double xmin    = vmins[ attr_x ];
   double xmax    = vmaxs[ attr_x ];
   double xinc    = vincs[ attr_x ];
   double ylow    = vllos[ attr_y ];
   double yhigh   = vlhis[ attr_y ];
   double ymin    = vmins[ attr_y ];
   double ymax    = vmaxs[ attr_y ];
   double yinc    = vincs[ attr_y ];
   bmndx          = -1;

   // Set ranges and values in GUI elements
   ct_lolimitx->setRange( xmin, xmax );
   ct_uplimitx->setRange( xmin, xmax );
   ct_lolimity->setRange( ymin, ymax );
   ct_uplimity->setRange( ymin, ymax );
   ct_lolimitx->setSingleStep( xinc );
   ct_uplimitx->setSingleStep( xinc );
   ct_lolimity->setSingleStep( yinc );
   ct_uplimity->setSingleStep( yinc );

   if ( chg_ndx == 1 )
   {  // X has just changed
      ct_lolimitx->setValue( xlow  );
      ct_uplimitx->setValue( xhigh );
   }
   else if ( chg_ndx == 2 )
   {  // Y has just changed
      ct_lolimity->setValue( ylow  );
      ct_uplimity->setValue( yhigh );
   }
   else
   {  // Z has just changed
      double zdef    = ( attr_z == US_ZSolute::ATTR_V )
                       ? dsets[ 0 ]->vbar20 
                       : vdefs[ attr_z ];
      le_z_func->setText( QString::number( zdef ) );
   }

   compute();
}

// Set attribute indecies and the composite solute type
void US_AnalysisControlPc::set_solute_type()
{
   attr_x         = bg_x_axis->checkedId();
   attr_y         = bg_y_axis->checkedId();
   int zt_ndx     = cb_z_type->currentIndex();
   attr_z         = ATTR_V;
   attr_z         = ( zt_ndx == 1 ) ? ATTR_K : attr_z;
   attr_z         = ( zt_ndx == 2 ) ? ATTR_W : attr_z;

   sol_type       = ( attr_x << 6 ) + ( attr_y << 3 ) + attr_z;
DbgLv(1) << "AC:SST: ztx x,y,z" << zt_ndx << attr_x << attr_y << attr_z
 << "sol_type" << sol_type << US_ModelRecord::stype_text(sol_type);

   dsets[ 0 ]->solute_type  = sol_type;
   dsets[ 0 ]->zcoeffs[ 0 ] = le_z_func->text().section( " ", 0, 0 )
                                               .toDouble();
   dsets[ 0 ]->zcoeffs[ 1 ] = 0.0;
   dsets[ 0 ]->zcoeffs[ 2 ] = 0.0;
   dsets[ 0 ]->zcoeffs[ 3 ] = 0.0;
}

