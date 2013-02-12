//! \file us_analysis_control.cpp

#include "us_1dsa.h"
#include "us_analysis_control.h"
#include "us_settings.h"
#include "us_passwd.h"
#include "us_db2.h"
#include "us_gui_settings.h"
#include "us_memory.h"

#include <qwt_legend.h>

// constructor:  1dsa analysis controls widget
US_AnalysisControl::US_AnalysisControl( QList< US_SolveSim::DataSet* >& dsets,
    QWidget* p ) : US_WidgetsDialog( p, 0 ), dsets( dsets )
{
   parentw        = p;
   processor      = 0;
   dbg_level      = US_Settings::us_debug();
   varimin        = 9e+99;
   bmndx          = -1;

   setObjectName( "US_AnalysisControl" );
   setAttribute( Qt::WA_DeleteOnClose, true );
   setPalette( US_GuiSettings::frameColor() );
   setFont( QFont( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() ) );

   // lay out the GUI
   setWindowTitle( tr( "1-D Spectrum Analysis Controls" ) );

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
   QLabel* lb_thrdcnt      = us_label(  tr( "Thread Count:" ) );
   QLabel* lb_minvari      = us_label(  tr( "Minimum Variance:" ) );
   QLabel* lb_minrmsd      = us_label(  tr( "Minimum RMSD:" ) );
   QLabel* lb_status       = us_label(  tr( "Status:" ) );

   QLabel* lb_statinfo     = us_banner( tr( "Status Information:" ) );

   pb_pltlines             = us_pushbutton( tr( "Plot Model Lines" ), true  );
   pb_strtfit              = us_pushbutton( tr( "Start Fit" ),        true  );
   pb_stopfit              = us_pushbutton( tr( "Stop Fit" ),         false );
   pb_plot                 = us_pushbutton( tr( "Plot Results" ),     false );
   pb_save                 = us_pushbutton( tr( "Save Results" ),     false );
   QPushButton* pb_help    = us_pushbutton( tr( "Help" ) );
   QPushButton* pb_close   = us_pushbutton( tr( "Close" ) );
   te_status               = us_textedit();
   us_setReadOnly( te_status, true );

   QLayout* lo_tinois      =
      us_checkbox( tr( "Fit Time-Invariant Noise"     ), ck_tinoise );
   QLayout* lo_rinois      =
      us_checkbox( tr( "Fit Radially-Invariant Noise" ), ck_rinoise );

   int nthr     = US_Settings::threads();
   nthr         = ( nthr > 1 ) ? nthr : QThread::idealThreadCount();
DbgLv(1) << "idealThrCout" << nthr;
   ct_lolimits  = us_counter( 3, -10000, 10000,    1 );
   ct_uplimits  = us_counter( 3, -10000, 10000,   10 );
   ct_lolimitk  = us_counter( 3,      1,     8,    1 );
   ct_uplimitk  = us_counter( 3,      1,    20,    4 );
   ct_incremk   = us_counter( 3,   0.01,     2, 0.25 );
   ct_varcount  = us_counter( 2,      3,   200,   16 );
   ct_cresolu   = us_counter( 2,     20,   200,  100 );
   ct_thrdcnt   = us_counter( 2,      1,    64, nthr );
   ct_lolimits->setStep(  0.1 );
   ct_uplimits->setStep(  0.1 );
   ct_lolimitk->setStep( 0.01 );
   ct_uplimitk->setStep( 0.01 );
   ct_incremk ->setStep( 0.01 );
   ct_varcount->setStep(    1 );
   ct_cresolu ->setStep(    1 );
   ct_thrdcnt ->setStep(    1 );
   cmb_curvtype = us_comboBox();
   cmb_curvtype->addItem( "Straight Line" );
   cmb_curvtype->addItem( "Increasing Sigmoid" );
   cmb_curvtype->addItem( "Decreasing Sigmoid" );
   cmb_curvtype->setCurrentIndex( 0 );

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
   controlsLayout->addWidget( lb_thrdcnt,    row,   0, 1, 2 );
   controlsLayout->addWidget( ct_thrdcnt,    row++, 2, 1, 2 );
   controlsLayout->addLayout( lo_tinois,     row,   0, 1, 2 );
   controlsLayout->addLayout( lo_rinois,     row++, 2, 1, 2 );
   controlsLayout->addWidget( pb_pltlines,   row++, 2, 1, 2 );
   controlsLayout->addWidget( pb_strtfit,    row,   0, 1, 2 );
   controlsLayout->addWidget( pb_stopfit,    row++, 2, 1, 2 );
   controlsLayout->addWidget( pb_plot,       row,   0, 1, 2 );
   controlsLayout->addWidget( pb_save,       row++, 2, 1, 2 );
   controlsLayout->addWidget( pb_help,       row,   0, 1, 2 );
   controlsLayout->addWidget( pb_close,      row++, 2, 1, 2 );
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
   optimizeLayout->addWidget( te_status,     row,   0, 2, 4 );
   le_minrmsd->setMinimumWidth( lb_minrmsd->width() );
   te_status ->setMinimumWidth( lb_minrmsd->width()*4 );
   row    += 6;

   QLabel* lb_optspace     = us_banner( "" );
   optimizeLayout->addWidget( lb_optspace,   row,   0, 1, 4 );
   optimizeLayout->setRowStretch( row, 2 );

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
            this,        SLOT(   compute()              ) );

   connect( pb_pltlines, SIGNAL( clicked()    ),
            this,        SLOT(   plot_lines() ) );
   connect( pb_strtfit,  SIGNAL( clicked()    ),
            this,        SLOT(   start()      ) );
   connect( pb_stopfit,  SIGNAL( clicked()    ),
            this,        SLOT(   stop_fit()   ) );
   connect( pb_plot,     SIGNAL( clicked()    ),
            this,        SLOT(   plot()       ) );
   connect( pb_save,     SIGNAL( clicked()    ),
            this,        SLOT(   save()       ) );
   connect( pb_help,     SIGNAL( clicked()    ),
            this,        SLOT(   help()       ) );
   connect( pb_close,    SIGNAL( clicked()    ),
            this,        SLOT(   close()      ) );

   lb_incremk ->setVisible( true  );
   ct_incremk ->setVisible( true  );
   lb_varcount->setVisible( false );
   ct_varcount->setVisible( false );
   edata          = &dsets[ 0 ]->run_data;

   compute();

//DbgLv(2) << "Pre-resize AC size" << size();
   resize( 710, 440 );
//DbgLv(2) << "Post-resize AC size" << size();
}

// enable/disable optimize counters based on chosen method
void US_AnalysisControl::optimize_options()
{
   ck_tinoise ->setEnabled( true );
   ck_rinoise ->setEnabled( true );

   adjustSize();
}

// uncheck optimize options other than one just checked
void US_AnalysisControl::uncheck_optimize( int /*ckflag*/ )
{
}

// start fit button clicked
void US_AnalysisControl::start()
{
   if ( parentw )
   {  // Get pointers to needed objects from the main
      US_1dsa* mainw = (US_1dsa*)parentw;
      edata          = mainw->mw_editdata();
      sdata          = mainw->mw_simdata();
      rdata          = mainw->mw_resdata();
      model          = mainw->mw_model();
      ti_noise       = mainw->mw_ti_noise();
      ri_noise       = mainw->mw_ri_noise();
      mw_stattext    = mainw->mw_status_text();
      mw_modstats    = mainw->mw_model_stats();

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

      QMessageBox::critical( this, tr( "Limits Inconsistent!" ), msg );
      return;
   }

   // Start a processing object if need be
   if ( processor == 0 )
      processor   = new US_1dsaProcess( dsets, this );

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

   // Begin the fit

   processor->start_fit( slo, sup, klo, kup, kin, res, typ, nthr, noif );

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
      US_1dsa* mainw = (US_1dsa*)parentw;
      mainw->analysis_done( -1 );
   }
DbgLv(1) << "AC:SF: analysis done";

   qApp->processEvents();
}

// plot button clicked
void US_AnalysisControl::plot()
{
   US_1dsa* mainw = (US_1dsa*)parentw;
   mainw->analysis_done( 1 );
}

// save button clicked
void US_AnalysisControl::save()
{
   US_1dsa* mainw = (US_1dsa*)parentw;
   mainw->analysis_done( 2 );
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

// slot to handle progress update
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
   QStringList modelstats;

   processor->get_results( sdata, rdata, model, ti_noise, ri_noise, bmndx,
         modelstats, mrecs );
DbgLv(1) << "AC:cp: RES: ti,ri counts" << ti_noise->count << ri_noise->count;
DbgLv(1) << "AC:cp: RES: bmndx" << bmndx;

   plot_lines();

   US_DataIO2::Scan* rscan0 = &rdata->scanData[ 0 ];
   int    mmitnum  = (int)rscan0->seconds;

   US_1dsa* mainw = (US_1dsa*)parentw;

   if ( alldone )
   {
      *mw_modstats    = modelstats;

      mainw->analysis_done( -2 );

      mainw->analysis_done(  0 );

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
   nkpts   = (int)ct_varcount->value();
   if ( ctype > 0 )
      nkpts       = qRound( ( fmax - fmin ) / finc ) + 1;

   US_MLinesPlot* mlnplotd = new US_MLinesPlot( fmin, fmax, finc, smin, smax,
                                                nlpts, bmndx, nkpts, ctype );

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

   QString filepath = US_Settings::tmpDir() + "/1DSA."
                      + edata->cell + edata->channel + edata->wavelength
                      + ".mlines."
                      + QString::number( getpid() ) + ".png";
   QPixmap pixmap   = QPixmap::grabWidget( mlnplotd, 0, 0,
                         mlnplotd->width(), mlnplotd->height() );
qDebug() << "PLOTLINE: mlines filepath" << filepath;
qDebug() << "PLOTLINE: mlines w h" << pixmap.width() << pixmap.height();
   pixmap.save( filepath );
}

