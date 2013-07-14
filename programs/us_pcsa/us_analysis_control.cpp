//! \file us_analysis_control.cpp

#include "us_pcsa.h"
#include "us_analysis_control.h"
#include "us_adv_analysis.h"
#include "us_simparms.h"
#include "us_rpscan.h"
#include "us_settings.h"
#include "us_passwd.h"
#include "us_db2.h"
#include "us_gui_settings.h"
#include "us_memory.h"

// constructor:  pcsa analysis controls widget
US_AnalysisControl::US_AnalysisControl( QList< US_SolveSim::DataSet* >& dsets,
    QWidget* p ) : US_WidgetsDialog( p, 0 ), dsets( dsets )
{
   parentw        = p;
   processor      = 0;
   dbg_level      = US_Settings::us_debug();
   varimin        = 9e+99;
   bmndx          = -1;
   mlnplotd       = 0;
   fitpars        = QString();

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

   setObjectName( "US_AnalysisControl" );
   setAttribute( Qt::WA_DeleteOnClose, true );
   setPalette( US_GuiSettings::frameColor() );
   setFont( QFont( US_GuiSettings::fontFamily(),
                   US_GuiSettings::fontSize() ) );
   QFontMetrics fmet( font() );

   // lay out the GUI
   setWindowTitle( tr( "Parametrically Constrained Spectrum Analysis Controls" ) );

   mainLayout      = new QHBoxLayout( this );
   controlsLayout  = new QGridLayout( );
   optimizeLayout  = new QGridLayout( );

   mainLayout->setSpacing        ( 2 );
   mainLayout->setContentsMargins( 2, 2, 2, 2 );

   mainLayout->addLayout( controlsLayout );
   mainLayout->addLayout( optimizeLayout  );

   QLabel* lb_fitting      = us_banner( tr( "Fitting Controls:" ) );
   QLabel* lb_curvtype     = us_label(  tr( "Curve Type:" ) );
   QLabel* lb_lolimits     = us_label(  tr( "Lower Limit (s x 1e-13):" ) );
   QLabel* lb_uplimits     = us_label(  tr( "Upper Limit (s x 1e-13):" ) );
           lb_lolimitk     = us_label(  tr( "Lower Limit (f/f0):" ) );
           lb_uplimitk     = us_label(  tr( "Upper Limit (f/f0):" ) );
           lb_incremk      = us_label(  tr( "Increment   (f/f0):" ) );
           lb_varcount     = us_label(  tr( "Variations Count:" ) );
   QLabel* lb_cresolu      = us_label(  tr( "Curve Resolution Points:" ) );
   QLabel* lb_tralpha      = us_label(  tr( "Regularization Parameter:"  ) );
   QLabel* lb_thrdcnt      = us_label(  tr( "Thread Count:" ) );
   QLabel* lb_minvari      = us_label(  tr( "Best Model Variance:" ) );
   QLabel* lb_minrmsd      = us_label(  tr( "Best Model RMSD:" ) );
   QLabel* lb_status       = us_label(  tr( "Status:" ) );

   QLabel* lb_statinfo     = us_banner( tr( "Status Information:" ) );

   pb_pltlines = us_pushbutton( tr( "Plot Model Lines"              ), true );
   pb_startfit = us_pushbutton( tr( "Start Fit"                     ), true );
   pb_scanregp = us_pushbutton( tr( "Perform Regularization Scan"   ), false );
   pb_finalmdl = us_pushbutton( tr( "Regularize Current Model"      ), false );
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
      us_checkbox( tr( "Regularize in Fixed Fits"      ), ck_fxalpha  );
   QLayout* lo_tinois      =
      us_checkbox( tr( "Fit Time-Invariant Noise"      ), ck_tinoise  );
   QLayout* lo_rinois      =
      us_checkbox( tr( "Fit Radially-Invariant Noise"  ), ck_rinoise  );
   ck_fxalpha ->setEnabled( false );
   ck_lmalpha ->setEnabled( false );

   int nthr     = US_Settings::threads();
   nthr         = ( nthr > 1 ) ? nthr : QThread::idealThreadCount();
DbgLv(1) << "idealThrCout" << nthr;
   ct_lolimits  = us_counter( 3, -10000, 10000,    1 );
   ct_uplimits  = us_counter( 3, -10000, 10000,   10 );
   ct_lolimitk  = us_counter( 3,      1,     8,    1 );
   ct_uplimitk  = us_counter( 3,      1,   100,    5 );
   ct_incremk   = us_counter( 3,   0.01,    10, 0.50 );
   ct_varcount  = us_counter( 2,      3,   200,   11 );
   ct_cresolu   = us_counter( 2,     20,   501,  101 );
   ct_thrdcnt   = us_counter( 2,      1,    64, nthr );
   ct_tralpha   = us_counter( 3,      0,   100,    0 );
   ct_lolimits->setStep(  0.1 );
   ct_uplimits->setStep(  0.1 );
   ct_lolimitk->setStep( 0.01 );
   ct_uplimitk->setStep( 0.01 );
   ct_incremk ->setStep( 0.01 );
   ct_varcount->setStep(    1 );
   ct_cresolu ->setStep(    1 );
   ct_tralpha ->setStep( 0.001 );
   ct_thrdcnt ->setStep(    1 );
   cmb_curvtype = us_comboBox();
   cmb_curvtype->addItem( "Straight Line" );
   cmb_curvtype->addItem( "Increasing Sigmoid" );
   cmb_curvtype->addItem( "Decreasing Sigmoid" );
   cmb_curvtype->setCurrentIndex( 1 );

   le_minvari   = us_lineedit( "0.000e-05", -1, true );
   le_minrmsd   = us_lineedit( "0.009000" , -1, true );

   b_progress   = us_progressBar( 0, 100, 0 );

   int row       = 0;
   controlsLayout->addWidget( lb_fitting,    row++, 0, 1, 4 );
   controlsLayout->addWidget( lb_curvtype,   row,   0, 1, 2 );
   controlsLayout->addWidget( cmb_curvtype,  row++, 2, 1, 2 );
   controlsLayout->addWidget( lb_lolimits,   row,   0, 1, 2 );
   controlsLayout->addWidget( ct_lolimits,   row++, 2, 1, 2 );
   controlsLayout->addWidget( lb_uplimits,   row,   0, 1, 2 );
   controlsLayout->addWidget( ct_uplimits,   row++, 2, 1, 2 );
   controlsLayout->addWidget( lb_lolimitk,   row,   0, 1, 2 );
   controlsLayout->addWidget( ct_lolimitk,   row++, 2, 1, 2 );
   controlsLayout->addWidget( lb_uplimitk,   row,   0, 1, 2 );
   controlsLayout->addWidget( ct_uplimitk,   row++, 2, 1, 2 );
   controlsLayout->addWidget( lb_incremk,    row,   0, 1, 2 );
   controlsLayout->addWidget( ct_incremk,    row++, 2, 1, 2 );
   controlsLayout->addWidget( lb_varcount,   row,   0, 1, 2 );
   controlsLayout->addWidget( ct_varcount,   row++, 2, 1, 2 );
   controlsLayout->addWidget( lb_cresolu,    row,   0, 1, 2 );
   controlsLayout->addWidget( ct_cresolu,    row++, 2, 1, 2 );
   controlsLayout->addWidget( lb_tralpha,    row,   0, 1, 2 );
   controlsLayout->addWidget( ct_tralpha,    row++, 2, 1, 2 );
   controlsLayout->addWidget( lb_thrdcnt,    row,   0, 1, 2 );
   controlsLayout->addWidget( ct_thrdcnt,    row++, 2, 1, 2 );
   controlsLayout->addLayout( lo_lmalpha,    row,   0, 1, 2 );
   controlsLayout->addWidget( pb_startfit,   row++, 2, 1, 2 );
   controlsLayout->addLayout( lo_fxalpha,    row,   0, 1, 2 );
   controlsLayout->addWidget( pb_stopfit,    row++, 2, 1, 2 );
   controlsLayout->addLayout( lo_tinois,     row,   0, 1, 2 );
   controlsLayout->addWidget( pb_scanregp,   row++, 2, 1, 2 );
   controlsLayout->addLayout( lo_rinois,     row,   0, 1, 2 );
   controlsLayout->addWidget( pb_finalmdl,   row++, 2, 1, 2 );
   controlsLayout->addWidget( pb_plot,       row,   0, 1, 2 );
   controlsLayout->addWidget( pb_save,       row++, 2, 1, 2 );
   controlsLayout->addWidget( pb_advanaly,   row++, 0, 1, 4 );
   controlsLayout->addWidget( pb_pltlines,   row,   0, 1, 2 );
   controlsLayout->addWidget( pb_help,       row,   2, 1, 1 );
   controlsLayout->addWidget( pb_close,      row++, 3, 1, 1 );
   controlsLayout->addWidget( lb_status,     row,   0, 1, 1 );
   controlsLayout->addWidget( b_progress,    row++, 1, 1, 3 );
   QLabel* lb_optspace1    = us_banner( "" );
   controlsLayout->addWidget( lb_optspace1,  row,   0, 1, 4 );
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

   QLabel* lb_optspace     = us_banner( "" );
   optimizeLayout->addWidget( lb_optspace,   row,   0, 1, 4 );

   optimize_options();

   connect( cmb_curvtype, SIGNAL( activated  ( int )    ),
            this,         SLOT(   compute()             ) );
   connect( ct_lolimits, SIGNAL( valueChanged( double ) ),
            this,        SLOT(   slim_change()          ) );
   connect( ct_uplimits, SIGNAL( valueChanged( double ) ),
            this,        SLOT(   slim_change()          ) );
   connect( ct_lolimitk, SIGNAL( valueChanged( double ) ),
            this,        SLOT(   klim_change()          ) );
   connect( ct_uplimitk, SIGNAL( valueChanged( double ) ),
            this,        SLOT(   klim_change()          ) );
   connect( ct_incremk,  SIGNAL( valueChanged( double ) ),
            this,        SLOT(   klim_change()          ) );
   connect( ct_varcount, SIGNAL( valueChanged( double ) ),
            this,        SLOT(   compute()              ) );
   connect( ct_cresolu,  SIGNAL( valueChanged( double ) ),
            this,        SLOT(   reso_change()          ) );
   connect( ct_tralpha,  SIGNAL( valueChanged( double ) ),
            this,        SLOT(   set_alpha()            ) );

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

   lb_incremk ->setVisible( true  );
   ct_incremk ->setVisible( true  );
   lb_varcount->setVisible( false );
   ct_varcount->setVisible( false );
   edata          = &dsets[ 0 ]->run_data;

   pb_pltlines->setEnabled( false );
   compute();

//DbgLv(2) << "Pre-resize AC size" << size();
   int  fwidth   = fmet.maxWidth();
   int  rheight  = ct_lolimits->height();
   int  cminw    = fwidth * 7;
   int  csizw    = cminw + fwidth;
   ct_lolimits->setMinimumWidth( cminw );
   ct_uplimits->setMinimumWidth( cminw );
   ct_lolimitk->setMinimumWidth( cminw );
   ct_uplimitk->setMinimumWidth( cminw );
   ct_incremk ->setMinimumWidth( cminw );
   ct_varcount->setMinimumWidth( cminw );
   ct_cresolu ->setMinimumWidth( cminw );
   ct_thrdcnt ->setMinimumWidth( cminw );
   ct_lolimits->resize( csizw, rheight );
   ct_uplimits->resize( csizw, rheight );
   ct_lolimitk->resize( csizw, rheight );
   ct_uplimitk->resize( csizw, rheight );
   ct_incremk ->resize( csizw, rheight );
   ct_varcount->resize( csizw, rheight );
   ct_cresolu ->resize( csizw, rheight );
   ct_thrdcnt ->resize( csizw, rheight );

   resize( 710, 440 );
   qApp->processEvents();

//DbgLv(2) << "Post-resize AC size" << size();
}

// enable/disable optimize counters based on chosen method
void US_AnalysisControl::optimize_options()
{
   bool use_noise = ( ct_tralpha->value() == 0.0 );
   ck_tinoise ->setEnabled( use_noise );
   ck_rinoise ->setEnabled( use_noise );

   adjustSize();
   qApp->processEvents();
}

// uncheck optimize options other than one just checked
void US_AnalysisControl::uncheck_optimize( int /*ckflag*/ )
{
}

// start fit button clicked
void US_AnalysisControl::start()
{
   US_pcsa* mainw = (US_pcsa*)parentw;
   mainw->analysis_done( -1 );   // Reset counters to zero

   // Make sure that ranges are reasonable
   if ( ( ct_uplimits->value() - ct_lolimits->value() ) < 0.0  ||
        ( ct_uplimitk->value() - ct_lolimitk->value() ) < 0.0 )
   {
      QString msg = 
         tr( "The \"s\" or \"f/f0\" ranges are inconsistent.\n"
             "Please re-check the limits and correct them\n"
             "before again clicking \"Start Fit\"." );

      QMessageBox::critical( this, tr( "Limits Inconsistent!" ), msg );
      return;
   }

   // Start a processing object if need be
   if ( processor == 0 )
   {
      need_fit    = true;
      need_final  = true;
      processor   = new US_pcsaProcess( dsets, this );
   }

   else
      processor->disconnect();

   // Set up for the start of fit processing
   varimin       = 9e+99;
   bmndx         = -1;
   le_minvari  ->setText( "0.000e-05" );
   le_minrmsd  ->setText( "0.0000" );

   int    typ    = cmb_curvtype->currentIndex();
   double slo    = ct_lolimits->value();
   double sup    = ct_uplimits->value();
   double klo    = ct_lolimitk->value();
   double kup    = ct_uplimitk->value();
   double kin    = ct_incremk ->value();
   int    nthr   = (int)ct_thrdcnt ->value();
   int    nvar   = (int)ct_varcount->value();
   int    noif   = ( ck_tinoise->isChecked() ? 1 : 0 ) +
                   ( ck_rinoise->isChecked() ? 2 : 0 );
   int    res    = (int)ct_cresolu ->value();
   kin           = ( typ == 0 ) ? kin : (double)nvar;
   double alpha  = ct_tralpha  ->value();

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
      processor->start_fit( slo, sup, klo, kup, kin,
                            res, typ, nthr, noif, alpha );

   else if ( need_final )
      processor->final_fit( alpha );

   qApp->processEvents();
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

   qApp->processEvents();
}

// plot button clicked
void US_AnalysisControl::plot()
{
   *mw_mrecs       = mrecs;
   *model          = mrecs[ 0 ].model;
   US_pcsa* mainw = (US_pcsa*)parentw;
   mainw->analysis_done( 1 );
}

// advanced controls button clicked
void US_AnalysisControl::advanced()
{
   US_pcsa* mainw = (US_pcsa*)parentw;
DbgLv(1) << "AC:advanced";
DbgLv(1) << "AC:advanced  mrecs.size" << mrecs.size();
if(mrecs.size()>0)
 DbgLv(1) << "AC:advanced mrecs0 p1 p2" << mrecs[0].par1 << mrecs[0].par2;
   int    nthr   = (int)ct_thrdcnt ->value();

   US_AdvAnalysis* aadiag = new US_AdvAnalysis( &mrecs, nthr, dsets[ 0 ],
                                                this );

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

      // Update model recs where possible and appropriate
      if ( mrsupd  &&  mw_mrecs != 0 )
      {
         *mw_mrecs       = mrecs;
         *model          = mrecs[ 0 ].model;
         *sdata          = mrecs[ 0 ].sim_data;
      }

      if ( mmcupd  &&  mw_mrecs_mc != 0 )
      {
         *mw_mrecs_mc    = mrecs_mc;
      }
DbgLv(1) << "AC:advanced: mrec0 sols" << mrecs[0].csolutes.size()
 << "mrecs size" << mrecs.size() << "mrecs_mc size" << mrecs_mc.size()
 << "model compsize" << model->components.size();

      if ( mmcupd )
      {  // Report new BFM from MonteCarlo
         QString fmsg = tr(
            "\n\nA newer best model has been created by %1 Monte Carlo\n"
            "  iterations  ( %2-solute, with RMSD = %3 )" )
            .arg( mciter ).arg( ncsols ).arg( rmsdf );

         progress_message( fmsg, true );
         le_minvari->setText( QString::number( varif ) );
         le_minrmsd->setText( QString::number( rmsdf ) );
      }

      else if ( mrsupd )
      {  // Report new BFM from other advanced controls action
         QString fmsg = tr(
            "\n\nA newer best model has been created from Advanced Controls\n"
            "  action  ( %1-solute, with RMSD = %2 )" )
            .arg( ncsols ).arg( rmsdf );

         progress_message( fmsg, true );
         le_minvari->setText( QString::number( varif ) );
         le_minrmsd->setText( QString::number( rmsdf ) );
      }
   }
else
DbgLv(1) << "AC:advanced dialog exec() return - CANCELED";
   delete aadiag;
}

// save button clicked
void US_AnalysisControl::save()
{
   US_pcsa* mainw = (US_pcsa*)parentw;
DbgLv(1) << "AC:save: model components size" << model->components.size();
   *mw_mrecs       = mrecs;
   *model          = mrecs[ 0 ].model;
   mainw->analysis_done( 2 );
}

// Close all windows
void US_AnalysisControl::close_all()
{
DbgLv(1) << "AC:close: mlnplotd" << mlnplotd;
   if ( (QObject*)mlnplotd != (QObject*)0 )
      mlnplotd->close();

   accept();
}

// Public close slot
void US_AnalysisControl::close()
{
   close_all();
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

   compute();
}

// Set k-upper-limit to lower when k grid points == 1
void US_AnalysisControl::klim_change()
{
   compute();
}

// Handle change in resolution count
void US_AnalysisControl::reso_change()
{
DbgLv(1) << "RESO_CHANGE: need_fit" << need_fit
 << "reso" << ct_cresolu ->value();
   if ( need_fit )
      compute();
}

// Set regularization factor alpha
void US_AnalysisControl::set_alpha()
{
   bool regular   = ( ct_tralpha->value() != 0.0 );
   ck_tinoise ->setEnabled( !regular );
   ck_rinoise ->setEnabled( !regular );
   ck_lmalpha ->setEnabled( regular );
   ck_fxalpha ->setEnabled( regular );

   if ( regular )
   {
      pb_finalmdl->setText( tr( "Regularize Current Model" ) );
   }

   else
   {
      pb_finalmdl->setText( tr( "Unregularize Current Model" ) );
      ck_tinoise ->setChecked( false );
      ck_rinoise ->setChecked( false );
   }

   if ( ct_tralpha->value() >= 10.0 )
      ct_tralpha->setStep( 0.01 );
   else
      ct_tralpha->setStep( 0.001 );
}

// Slot to handle progress update
void US_AnalysisControl::update_progress( double variance )
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
void US_AnalysisControl::progress_message( QString pmsg, bool append )
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
   US_pcsa* mainw = (US_pcsa*)parentw;
DbgLv(1) << "AC:cp: stage" << stage;

   if ( stage == 7 )
   { // If an alpha scan can now be done, report L-M info and mark scan-ready
      ModelRecord mrec;
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
      mainw->analysis_done( -2 );

DbgLv(1) << "AC:cp: main done 0";
      mainw->analysis_done(  0 );

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
void US_AnalysisControl::compute()
{
   ctype   = cmb_curvtype->currentIndex();
   smin    = ct_lolimits->value();
   smax    = ct_uplimits->value();
   fmin    = ct_lolimitk->value();
   fmax    = ct_uplimitk->value();
   finc    = ct_incremk ->value();
   nlpts   = (int)ct_cresolu ->value();
   nkpts   = qRound( ( fmax - fmin ) / finc ) + 1;
   mrecs.clear();

   if ( ctype == 0 )
   {
      lb_incremk ->setVisible( true  );
      ct_incremk ->setVisible( true  );
      lb_varcount->setVisible( false );
      ct_varcount->setVisible( false );

      ModelRecord::compute_slines( smin, smax, fmin, fmax, finc, nlpts,
            mrecs );
   }
   if ( ctype == 1  ||  ctype == 2 )
   {
      nkpts          = (int)ct_varcount->value();
      lb_incremk ->setVisible( false );
      ct_incremk ->setVisible( false );
      lb_varcount->setVisible( true  );
      ct_varcount->setVisible( true  );

      ModelRecord::compute_sigmoids( ctype, smin, smax, fmin, fmax,
            nkpts, nlpts, mrecs );
   }
   int    nlmodl  = nkpts * nkpts;

   QString amsg   =
      tr( "The number of test models is %1,\n"
          " derived from the square of %2 variation points,\n"
          " with each curve model consisting of %3 points." )
      .arg( nlmodl ).arg( nkpts ).arg( nlpts );
   te_status  ->setText( amsg );

   bmndx          = -1;
   need_fit       = ( fitpars != fitpars_string() );
DbgLv(1) << "CM: need_fit" << need_fit;
   pb_pltlines->setEnabled( true );
   pb_scanregp->setEnabled( !need_fit );
   pb_finalmdl->setEnabled( !need_fit );
}

// slot to launch a plot dialog showing model lines
void US_AnalysisControl::plot_lines()
{
   ctype   = cmb_curvtype->currentIndex();
   smin    = ct_lolimits->value();
   smax    = ct_uplimits->value();
   fmin    = ct_lolimitk->value();
   fmax    = ct_uplimitk->value();
   finc    = ct_incremk ->value();
   nlpts   = (int)ct_cresolu ->value();
   nkpts   = ( ctype > 0 )
             ? (int)ct_varcount->value()
             : qRound( ( fmax - fmin ) / finc ) + 1;

DbgLv(1) << "PL: mlnplotd" << mlnplotd;
   if ( mlnplotd != 0 )
      mlnplotd->close();
DbgLv(1) << "PL:  mlnplotd closed";

   mlnplotd = new US_MLinesPlot( fmin, fmax, finc, smin, smax,
                                 nlpts, bmndx, nkpts, ctype );

   connect( mlnplotd, SIGNAL( destroyed( QObject* ) ),
            this,     SLOT  ( closed   ( QObject* ) ) );

DbgLv(1) << "PL:  new mlnplotd" << mlnplotd;

   if ( bmndx >= 0 )
   {
      mlnplotd->setModel( model, mrecs );
   }
   else
   {
      mlnplotd->setModel( 0, mrecs );
   }

   mlnplotd->plot_data();
   mlnplotd->setVisible( true );

   QString filepath = US_Settings::tmpDir() + "/PCSA."
                      + edata->cell + edata->channel + edata->wavelength
                      + ".mlines."
                      + QString::number( getpid() ) + ".png";
   QPixmap pixmap   = QPixmap::grabWidget( mlnplotd, 0, 0,
                         mlnplotd->width(), mlnplotd->height() );
DbgLv(0) << "PLOTLINE: mlines filepath" << filepath;
DbgLv(0) << "PLOTLINE: mlines w h" << pixmap.width() << pixmap.height();
   pixmap.save( filepath );
}

// Private slot to mark a child widget as closed, if it has been destroyed
void US_AnalysisControl::closed( QObject* o )
{
   QString oname = o->objectName();

   if ( oname.contains( "MLinesPlot" ) )
      mlnplotd    = 0;
}

// Set flags and start fit where fits and final computation are needed
void US_AnalysisControl::fit_final( void )
{
   need_fit    = true;
   need_final  = true;
   fitpars     = QString();

   start();
}

// Set flags and open the dialog to do an Alpha scan
void US_AnalysisControl::scan_alpha( void )
{
   ModelRecord mrec;
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
void US_AnalysisControl::final_only( void )
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
QString US_AnalysisControl::fitpars_string()
{
   int    typ    = cmb_curvtype->currentIndex();
   double slo    = ct_lolimits->value();
   double sup    = ct_uplimits->value();
   double klo    = ct_lolimitk->value();
   double kup    = ct_uplimitk->value();
   double kin    = ct_incremk ->value();
   int    nvar   = (int)ct_varcount->value();
   kin           = ( typ == 0 ) ? kin : (double)nvar;
   int    noif   = ( ck_tinoise->isChecked() ? 1 : 0 ) +
                   ( ck_rinoise->isChecked() ? 2 : 0 );

   return QString().sprintf( "%d %.5f %.5f %.5f %.5f %.5f %d %d",
            typ, slo, sup, klo, kup, kin, nvar, noif );
}

// Recompute the top model record after change to resolution points
void US_AnalysisControl::recompute_mrec()
{
   ModelRecord mrec  = mrecs[ 0 ];
int nn=mrec.isolutes.size()-1;
int mm=mrec.isolutes.size()/2;
DbgLv(1) << "AC:RM: mrec0 solsize" << mrec.isolutes.size()
 << "s0 s,k" << mrec.isolutes[0].s << mrec.isolutes[0].k
 << "sm s,k" << mrec.isolutes[mm].s << mrec.isolutes[mm].k
 << "sn s,k" << mrec.isolutes[nn].s << mrec.isolutes[nn].k;
   mrec.isolutes.clear();
   US_Solute isol;
   smin          = ct_lolimits->value();
   smax          = ct_uplimits->value();
   fmin          = ct_lolimitk->value();
   fmax          = ct_uplimitk->value();
   finc          = ct_incremk ->value();
   nlpts         = (int)ct_cresolu ->value();
   ctype         = cmb_curvtype->currentIndex();
   double str_k  = mrec.str_k;
   double end_k  = mrec.end_k;
   double par1   = mrec.par1;
   double par2   = mrec.par2;
   double prng   = (double)( nlpts - 1 );
   double xrng   = smax - smin;

   if ( ctype == 0 )
   {
      double xval   = smin;
      double xinc   = xrng / prng;
      double kval   = str_k;
      double kinc   = ( end_k - str_k ) / prng;

      for ( int kk = 0; kk < nlpts; kk++ )
      { // Loop over points on a line
         isol.s      = xval * 1.e-13;
         isol.k      = kval;
         mrec.isolutes << isol;
         xval       += xinc;
         kval       += kinc;
      } // END: points-per-line loop
   }

   else if ( ctype == 1  ||  ctype == 2 )
   {
      double xrng   = smax - smin;
      double kstr   = fmin;
      double kdif   = fmax - fmin;
      if ( ctype == 2 )
      {
         kstr          = fmax;
         kdif          = -kdif;
      }

      double xval   = 0.0;
      double xinc   = 1.0 / prng;
      double p1rt   = sqrt( 2.0 * par1 );

      for ( int kk = 0; kk < nlpts; kk++ )
      { // Loop over points on a sigmoid curve
         double sval  = smin + xval * xrng;
         double efac  = 0.5 * erf( ( xval - par2 ) / p1rt ) + 0.5;
         double kval  = kstr + kdif * efac;
         isol.s       = sval * 1.e-13;
         isol.k       = kval;
         mrec.isolutes << isol;
         xval        += xinc;
      } // END: points-on-curve loop
   }

   mrecs[ 0 ] = mrec;
nn=mrec.isolutes.size()-1;
mm=mrec.isolutes.size()/2;
DbgLv(1) << "AC:RM: NEW mrec0 solsize" << mrec.isolutes.size()
 << "s0 s,k" << mrec.isolutes[0].s << mrec.isolutes[0].k
 << "sm s,k" << mrec.isolutes[mm].s << mrec.isolutes[mm].k
 << "sn s,k" << mrec.isolutes[nn].s << mrec.isolutes[nn].k;
   if ( processor != 0 )
      processor->put_mrec( mrec );
}

