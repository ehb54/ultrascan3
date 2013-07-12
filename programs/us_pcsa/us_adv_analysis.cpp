//! \file us_adv_analysis.cpp

#include "us_pcsa.h"
#include "us_adv_analysis.h"
#include "us_settings.h"
#include "us_gui_settings.h"

// constructor:  enhanced plot control widget
US_AdvAnalysis::US_AdvAnalysis( QVector< ModelRecord >* p_mrs, const int nth,
    US_SolveSim::DataSet* ds0, QWidget* p ) : US_WidgetsDialog( p, 0 )
{
   p_mrecs        = p_mrs;
   nthr           = nth;
   dset0          = ds0;
   parentw        = p;
   dbg_level      = US_Settings::us_debug();
DbgLv(1) << "AA: IN";

   setObjectName( "US_AdvAnalysis" );
   setPalette( US_GuiSettings::frameColor() );
   setFont( QFont( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() ) );
   setAttribute( Qt::WA_DeleteOnClose, false );

   // lay out the GUI
   setWindowTitle( tr( "PCSA Advanced Controls" ) );

   // Define the layouts
   mainLayout      = new QHBoxLayout( this );
   finmodelLayout  = new QGridLayout( );
   mreclistLayout  = new QGridLayout( );

   mainLayout->setSpacing        ( 2 );
   mainLayout->setContentsMargins( 2, 2, 2, 2 );

   mainLayout->addLayout( finmodelLayout  );
   mainLayout->addLayout( mreclistLayout  );
   mainLayout->setStretchFactor( finmodelLayout, 1 );
   mainLayout->setStretchFactor( mreclistLayout, 1 );
DbgLv(1) << "AA: define GUI elements";

   // Define banners and labels
   QLabel* lb_fitctrl   = us_banner( tr( "Fitting Controls:" ) );
   QLabel* lb_bfmstat   = us_banner( tr( "Best Final Model Status:" ) );
   QLabel* lb_mrecctrl  = us_banner( tr( "Final & Model Records Controls:" ) );
   QLabel* lb_mrecstat  = us_banner( tr( "Model Records Status:" ) );
   QLabel* lb_curvtype  = us_label ( tr( "Curve Type:" ) );
   QLabel* lb_s_lower   = us_label ( tr( "Lower Limit (s x 1e-13):" ) );
   QLabel* lb_s_upper   = us_label ( tr( "Upper Limit (s x 1e-13):" ) );
           lb_sigmpar1  = us_label ( tr( "Sigmoid Par 1:" ) );
           lb_sigmpar2  = us_label ( tr( "Sigmoid Par 2:" ) );
           lb_k_lower   = us_label ( tr( "Lower Limit (f/f0):" ) );
           lb_k_upper   = us_label ( tr( "Upper Limit (f/f0):" ) );
   QLabel* lb_crpoints  = us_label ( tr( "Curve Resolution Points:" ) );
   QLabel* lb_mciters   = us_label ( tr( "Monte Carlo Iterations:" ) );
   QLabel* lb_progress  = us_label ( tr( "Progress:" ) );
   QLabel* lb_space1    = us_banner( "" );
   QLabel* lb_space2    = us_banner( "" );

   // Define buttons
   pb_loadmrl   = us_pushbutton( tr( "Load Model Records"  ) );
   pb_storemrl  = us_pushbutton( tr( "Store Model Records" ) );
   pb_loadbfm   = us_pushbutton( tr( "Load Final Model"    ) );
   pb_storebfm  = us_pushbutton( tr( "Store Final Model"   ) );
   pb_resetbfm  = us_pushbutton( tr( "Reset Final Model"   ) );
   pb_resetmrl  = us_pushbutton( tr( "Reset Model Records" ) );
   pb_buildbfm  = us_pushbutton( tr( "Build Final Model"   ) );
   pb_mciters   = us_pushbutton( tr( "Start Monte Carlo"   ) );
   pb_help      = us_pushbutton( tr( "Help"   ) );
   pb_cancel    = us_pushbutton( tr( "Cancel" ) );
   pb_accept    = us_pushbutton( tr( "Accept" ) );

   // Define counters
   ct_s_lower   = us_counter( 3, -10000, 10000,   1 );
   ct_s_upper   = us_counter( 3, -10000, 10000,  10 );
   ct_sigmpar1  = us_counter( 3,  0.001,   0.5, 0.2 );
   ct_sigmpar2  = us_counter( 3,    0.0,   1.0, 0.1 );
   ct_k_lower   = us_counter( 3,      1,     8,   1 );
   ct_k_upper   = us_counter( 3,      1,   100,   5 );
   ct_crpoints  = us_counter( 2,     20,   501, 101 );
   ct_mciters   = us_counter( 2,      1,   100,  20 );
   ct_s_lower ->setStep(   0.1 );
   ct_s_upper ->setStep(   0.1 );
   ct_k_lower ->setStep(  0.01 );
   ct_k_upper ->setStep(  0.01 );
   ct_sigmpar1->setStep( 0.001 );
   ct_sigmpar2->setStep( 0.001 );
   ct_crpoints->setStep(     1 );
   ct_mciters ->setStep(     1 );

   // Define combo box
   cb_curvtype  = us_comboBox();
   cb_curvtype->setMaxVisibleItems( 3 );
   cb_curvtype->addItem( tr( "Straight Line"      ) );
   cb_curvtype->addItem( tr( "Increasing Sigmoid" ) );
   cb_curvtype->addItem( tr( "Decreasing Sigmoid" ) );
   cb_curvtype->setCurrentIndex( 1 );
 
   // Define status text boxes and progress bar
   te_bfmstat   = us_textedit();
   te_mrecstat  = us_textedit();
   b_progress   = us_progressBar( 0, 100, 0 );
   us_setReadOnly( te_bfmstat,  true );
   us_setReadOnly( te_mrecstat, true );

   // Lay out the left side, of BFM controls and status
DbgLv(1) << "AA: populate finmodelLayout";
   int row      = 0;
   finmodelLayout->addWidget( lb_fitctrl,    row++, 0, 1, 6 );
   finmodelLayout->addWidget( lb_curvtype,   row,   0, 1, 3 );
   finmodelLayout->addWidget( cb_curvtype,   row++, 3, 1, 3 );
   finmodelLayout->addWidget( lb_s_lower,    row,   0, 1, 3 );
   finmodelLayout->addWidget( ct_s_lower,    row++, 3, 1, 3 );
   finmodelLayout->addWidget( lb_s_upper,    row,   0, 1, 3 );
   finmodelLayout->addWidget( ct_s_upper,    row++, 3, 1, 3 );
   finmodelLayout->addWidget( lb_sigmpar1,   row,   0, 1, 3 );
   finmodelLayout->addWidget( ct_sigmpar1,   row++, 3, 1, 3 );
   finmodelLayout->addWidget( lb_sigmpar2,   row,   0, 1, 3 );
   finmodelLayout->addWidget( ct_sigmpar2,   row++, 3, 1, 3 );
   finmodelLayout->addWidget( lb_k_lower,    row,   0, 1, 3 );
   finmodelLayout->addWidget( ct_k_lower,    row++, 3, 1, 3 );
   finmodelLayout->addWidget( lb_k_upper,    row,   0, 1, 3 );
   finmodelLayout->addWidget( ct_k_upper,    row++, 3, 1, 3 );
   finmodelLayout->addWidget( lb_crpoints,   row,   0, 1, 3 );
   finmodelLayout->addWidget( ct_crpoints,   row++, 3, 1, 3 );
   finmodelLayout->addWidget( lb_mciters,    row,   0, 1, 3 );
   finmodelLayout->addWidget( ct_mciters,    row++, 3, 1, 3 );

   finmodelLayout->addWidget( lb_bfmstat,    row++, 0, 1, 6 );
   finmodelLayout->addWidget( te_bfmstat,    row,   0, 8, 6 );
   row         += 8;
   finmodelLayout->addWidget( lb_space1,     row,   0, 1, 6 );

   // Lay out the right side, of Model Record controls and status
DbgLv(1) << "AA: populate mreclistLayout";
   row          = 0;
   mreclistLayout->addWidget( lb_mrecctrl,   row++, 0, 1, 6 );
   mreclistLayout->addWidget( pb_loadmrl,    row,   0, 1, 3 );
   mreclistLayout->addWidget( pb_storemrl,   row++, 3, 1, 3 );
   mreclistLayout->addWidget( pb_loadbfm,    row,   0, 1, 3 );
   mreclistLayout->addWidget( pb_storebfm,   row++, 3, 1, 3 );
   mreclistLayout->addWidget( pb_resetbfm,   row,   0, 1, 3 );
   mreclistLayout->addWidget( pb_resetmrl,   row++, 3, 1, 3 );
   mreclistLayout->addWidget( pb_buildbfm,   row,   0, 1, 3 );
   mreclistLayout->addWidget( pb_mciters,    row++, 3, 1, 3 );

   mreclistLayout->addWidget( lb_mrecstat,   row++, 0, 1, 6 );
   mreclistLayout->addWidget( te_mrecstat,   row,   0, 8, 6 );
   mreclistLayout->setRowStretch( row, 2 );
   row         += 8;
   mreclistLayout->addWidget( lb_progress,   row,   0, 1, 1 );
   mreclistLayout->addWidget( b_progress,    row++, 1, 1, 5 );
   mreclistLayout->addWidget( pb_help,       row,   0, 1, 2 );
   mreclistLayout->addWidget( pb_cancel,     row,   2, 1, 2 );
   mreclistLayout->addWidget( pb_accept,     row++, 4, 1, 2 );
   mreclistLayout->addWidget( lb_space2,     row,   0, 1, 6 );

   // Set defaults and status values based on the initial model records
   mrecs        = *p_mrecs;
   nmrecs       = mrecs.size();
   mciters      = 0;
   kciters      = 0;
   bfm0_exists  = false;
   mrs0_exists  = false;
   bfm_new      = false;
   mrs_new      = false;
   mc_done      = false;
   mc_running   = false;
   ctype        = 1;
   nisols       = 0;
   ncsols       = ( nmrecs > 0 ) ? mrecs[ 0 ].csolutes.size() : 0;

   if ( ncsols > 0 )
   {  // We are starting with models already computed
      mrec         = mrecs[ 0 ];
      nisols       = mrec.isolutes.size();
      ncsols       = mrec.csolutes.size();
      int ks       = nisols - 1;
      double str_s = ( nisols > 0 ) ? mrec.isolutes[ 0  ].s * 1.e13 : 1.0;
      double end_s = ( nisols > 0 ) ? mrec.isolutes[ ks ].s * 1.e13 : 10.0;
      ctype        = ( mrec.par1 == mrec.str_k ) ? 0 :
                     ( ( mrec.str_k < mrec.end_k ) ? 1 : 2 );

      ct_s_lower ->setValue( str_s );
      ct_s_upper ->setValue( end_s );
      ct_k_lower ->setValue( mrec.str_k );
      ct_k_upper ->setValue( mrec.end_k );
      ct_sigmpar1->setValue( mrec.par1  );
      ct_sigmpar2->setValue( mrec.par2  );
DbgLv(1) << "AA: mr p1 p2  m0 p1 p2" << mrec.par1 << mrec.par2 << mrecs[0].par1
 << mrecs[0].par2 << "  typ ni nc" << ctype << nisols << ncsols;

      te_bfmstat ->setText(
         tr( "An initial best final model, with RMSD of %1,\n"
             "  has been read.\n" ).arg( mrec.rmsd ) );

      te_mrecstat->setText(
         tr( "An initial model records list, with %1 fits,\n"
             "  has been read." ).arg( nmrecs ) );

      mrec0        = mrec;          // Save initial model records
      mrecs0       = mrecs;
      bfm0_exists  = true;
      mrs0_exists  = true;
   }

   else
   {  // We are starting with models not yet computed
      mrec         = ( nmrecs > 0 ) ? mrecs[ 0 ] : mrec;
      mrec0        = mrec;          // Save initial model records
      mrecs0       = mrecs;
DbgLv(1) << "AA: nmrecs" << nmrecs << "ncsols" << ncsols;
      te_bfmstat ->setText(
         tr( "No initial best final model has been read" ) );
      te_mrecstat->setText(
         tr( "No initial model records list has been read" ) );
   }

   // Define connections
DbgLv(1) << "AA: connect buttons";
   connect( cb_curvtype, SIGNAL( currentIndexChanged( int ) ),
            this,        SLOT(   curvtypeChanged    ( int ) ) );

   connect( ct_s_lower,  SIGNAL( valueChanged ( double ) ),
            this,        SLOT(   slowerChanged( double ) ) );
   connect( ct_s_upper,  SIGNAL( valueChanged ( double ) ),
            this,        SLOT(   supperChanged( double ) ) );
   connect( ct_sigmpar1, SIGNAL( valueChanged ( double ) ),
            this,        SLOT(   sipar1Changed( double ) ) );
   connect( ct_sigmpar2, SIGNAL( valueChanged ( double ) ),
            this,        SLOT(   sipar2Changed( double ) ) );
   connect( ct_k_lower,  SIGNAL( valueChanged ( double ) ),
            this,        SLOT(   klowerChanged( double ) ) );
   connect( ct_k_upper,  SIGNAL( valueChanged ( double ) ),
            this,        SLOT(   kupperChanged( double ) ) );
   connect( ct_crpoints, SIGNAL( valueChanged ( double ) ),
            this,        SLOT(   pointsChanged( double ) ) );
   connect( ct_mciters,  SIGNAL( valueChanged ( double ) ),
            this,        SLOT(   mciterChanged( double ) ) );

   connect( pb_loadmrl,  SIGNAL( clicked()          ),
            this,        SLOT(   load_mrecs()       ) );
   connect( pb_storemrl, SIGNAL( clicked()          ),
            this,        SLOT(   store_mrecs()      ) );
   connect( pb_loadbfm,  SIGNAL( clicked()          ),
            this,        SLOT(   load_bfm()         ) );
   connect( pb_storebfm, SIGNAL( clicked()          ),
            this,        SLOT(   store_bfm()        ) );
   connect( pb_resetbfm, SIGNAL( clicked()          ),
            this,        SLOT(   reset_bfm()        ) );
   connect( pb_resetmrl, SIGNAL( clicked()          ),
            this,        SLOT(   reset_mrecs()      ) );
   connect( pb_buildbfm, SIGNAL( clicked()          ),
            this,        SLOT(   build_bfm()        ) );
   connect( pb_mciters,  SIGNAL( clicked()          ),
            this,        SLOT(   start_montecarlo() ) );

   connect( pb_help,     SIGNAL( clicked() ),
            this,        SLOT(   help()    ) );
   connect( pb_cancel,   SIGNAL( clicked() ),
            this,        SLOT(   cancel()  ) );
   connect( pb_accept,   SIGNAL( clicked() ),
            this,        SLOT(   select()  ) );

   curvtypeChanged( ctype );
#if 0
DbgLv(1) << "Pre-adjust size" << size();
   adjustSize();
DbgLv(1) << "Post-adjust size" << size();
#endif
   resize( 780, 400 );
DbgLv(1) << "Post-resize size" << size();
   qApp->processEvents();
DbgLv(1) << "Post-processEvents";
}

// Return state flag from advanced actions and, possibly, MC models
int US_AdvAnalysis::advanced_results( QVector< ModelRecord >* p_mrecsmc )
{
   // Set state flag reflecting new-bfm, new-mrs, montecarlo
   int state    = bfm_new ?           msk_bfnew   : 0;
   state        = mrs_new ? ( state | msk_mrnew ) : state;
   state        = mc_done ? ( state | msk_mcarl ) : state;
DbgLv(1) << "advanced_results - state=" << state;

   if ( mc_done  &&  p_mrecsmc != 0 )
   {  // If MonteCarlo was done, return its model records
      *p_mrecsmc   = mrecs_mc;
   }

   return state;
}

// Accept button clicked
void US_AdvAnalysis::select()
{
   if ( mrs_new  &&  p_mrecs != 0 )
   {  // If model records are new, return them to the caller
      *p_mrecs     = mrecs;
   }

   accept();
   close();
}

// Cancel button clicked
void US_AdvAnalysis::cancel()
{
   reject();
   close();
}

// Slot to handle a change in curve type
void US_AdvAnalysis::curvtypeChanged( int ivalue )
{
DbgLv(1) << "curvtypeChanged" << ivalue;
   ctype          = ivalue;
   bool is_line   = ( ctype == 0 );
   bool is_sigm   = ! is_line;

   lb_sigmpar1->setVisible( is_sigm );
   lb_sigmpar2->setVisible( is_sigm );
   lb_k_lower ->setVisible( is_line );
   lb_k_upper ->setVisible( is_line );
   ct_sigmpar1->setVisible( is_sigm );
   ct_sigmpar2->setVisible( is_sigm );
   ct_k_lower ->setVisible( is_line );
   ct_k_upper ->setVisible( is_line );
}

// Slot to handle a change in S lower bound
void US_AdvAnalysis::slowerChanged( double value )
{
DbgLv(1) << "slowerChanged" << value;
}

// Slot to handle a change in S upper bound
void US_AdvAnalysis::supperChanged( double value )
{
DbgLv(1) << "supperChanged" << value;
}

// Slot to handle a change in sigmoid par 1
void US_AdvAnalysis::sipar1Changed( double value )
{
DbgLv(1) << "sipar1Changed" << value;
}

// Slot to handle a change in sigmoid par 2
void US_AdvAnalysis::sipar2Changed( double value )
{
DbgLv(1) << "sipar2Changed" << value;
}

// Slot to handle a change in K(f/f0) lower bound
void US_AdvAnalysis::klowerChanged( double value )
{
DbgLv(1) << "klowerChanged" << value;
}

// Slot to handle a change in K(f/f0) upper bound
void US_AdvAnalysis::kupperChanged( double value )
{
DbgLv(1) << "kupperChanged" << value;
}

// Slot to handle a change in curve points
void US_AdvAnalysis::pointsChanged( double value )
{
DbgLv(1) << "pointsChanged" << value;
}

// Slot to handle a change in monte carlo iterations
void US_AdvAnalysis::mciterChanged( double value )
{
DbgLv(1) << "mciterChanged" << value;
   mciters           = (int)value;
}

// Slot to load a model records list from disk
void US_AdvAnalysis::load_mrecs()
{
DbgLv(1) << "load_mrecs";
under_construct( "Load Model Records" );
}

// Slot to store a model records list to disk
void US_AdvAnalysis::store_mrecs()
{
DbgLv(1) << "store_mrecs";
under_construct( "Store Model Records" );
}

// Slot to load a best final model from disk
void US_AdvAnalysis::load_bfm()
{
DbgLv(1) << "load_bfm";
under_construct( "Load Final Model" );
}

// Slot to store a best final model to disk
void US_AdvAnalysis::store_bfm()
{
DbgLv(1) << "store_bfm";
under_construct( "Store Final Model" );
}

// Slot to reset the best final model to its initial state
void US_AdvAnalysis::reset_bfm()
{
DbgLv(1) << "reset_bfm";
}

// Slot to reset the list of model records to its initial state
void US_AdvAnalysis::reset_mrecs()
{
DbgLv(1) << "reset_mrecs";
}

// Slot to build a new best final model from specified fitting controls
void US_AdvAnalysis::build_bfm()
{
DbgLv(1) << "build_bfm";
under_construct( "Build Final Model" );
}

// Slot to start and process monte carlo iterations
void US_AdvAnalysis::start_montecarlo()
{
DbgLv(1) << "start_montecarlo";
   wdata          = dset0->run_data;
   edata          = &wdata;
   mciters        = (int)ct_mciters->value();
   QString sline1 = tr( "%1 Monte Carlo iterations are being computed...\n" )
                    .arg( mciters );
   ksiters        = 0;
   kciters        = 0;
   ModelRecord          mrec_mc = mrec;
   US_SolveSim::DataSet dset    = *dset0;
   QList< US_SolveSim::DataSet* > dsets;
   dsets << &dset;
   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
   b_progress->reset();
   b_progress->setMaximum( mciters );

   if ( nthr == 1 )
   {  // Monte Carlo with single thread
      for ( int iter = 0; iter < mciters; iter++ )
      {
         ksiters++;
         US_SolveSim::Simulation sim_vals;
         sim_vals.solutes          = mrec.isolutes;
         dsets[ 0 ]->run_data      = wdata;

         US_SolveSim* solvesim     = new US_SolveSim( dsets, 0, false );

         solvesim->calc_residuals( 0, 1, sim_vals );

         kciters          = ksiters;
         mrec_mc.variance = sim_vals.variance;
         mrec_mc.rmsd     = sqrt( sim_vals.variance );
         mrec_mc.csolutes = sim_vals.solutes;
         ncsols           = mrec_mc.csolutes.size();
DbgLv(1) << "  kciters" << kciters << "rmsd" << mrec_mc.rmsd
 << "ncsols" << ncsols;

         mrecs_mc << mrec_mc;

         // Set up new data modified by a gaussian distribution
         if ( iter == 0 )
         {
            set_gaussians( sim_vals );
         }

         // Except on last iteration, we must create new "experiment" data
         if ( kciters < mciters )
         {
            apply_gaussians();
         }

         te_bfmstat ->setText( sline1 +
            tr( "%1 are completed ( last:  %2-solute,  RMSD=%3 )" )
            .arg( kciters ).arg( ncsols ).arg( mrec_mc.rmsd ) );
         b_progress->setValue( kciters );
         qApp->processEvents();
      }

      // Complete the MonteCarlo process
      b_progress->setValue( mciters );
      qApp->processEvents();

      montecarlo_done();

      QApplication::restoreOverrideCursor();
      qApp->processEvents();
   }

   else
   {  // Monte Carlo in threads
      sline1 = tr( "%1 Monte Carlo iterations are being"
                   " computed in %2 threads...\n" )
                    .arg( mciters ).arg( nthr );

      // Do the first iteration computation and set gaussians
      US_SolveSim::Simulation sim_vals;
      sim_vals.solutes          = mrec.isolutes;
      dsets[ 0 ]->run_data      = wdata;
      ksiters++;

      US_SolveSim* solvesim     = new US_SolveSim( dsets, 0, false );

      solvesim->calc_residuals( 0, 1, sim_vals );

      kciters          = ksiters;
      mrec_mc.variance = sim_vals.variance;
      mrec_mc.rmsd     = sqrt( sim_vals.variance );
      mrec_mc.csolutes = sim_vals.solutes;
      ncsols           = mrec_mc.csolutes.size();
DbgLv(1) << "  kciters" << kciters << "rmsd" << mrec_mc.rmsd
 << "ncsols" << ncsols;
      te_bfmstat ->setText( sline1 +
         tr( "%1 are completed ( last:  %2-solute,  RMSD=%3 )" )
         .arg( kciters ).arg( ncsols ).arg( mrec_mc.rmsd ) );
      b_progress->setValue( kciters );
      qApp->processEvents();

      mrecs_mc << mrec_mc;
      set_gaussians( sim_vals );

      WorkPacket    wtbase;
      wtbase.par1      = mrec.par1;
      wtbase.par2      = mrec.par2;
      wtbase.str_k     = mrec.str_k;
      wtbase.end_k     = mrec.end_k;
      wtbase.isolutes  = mrec.isolutes;
      wtbase.csolutes.clear();
      wtbase.noisf     = 0;
      wtbase.dsets     = dsets;
      wtbase.depth     = 0;
      US_SolveSim::DataSet wkdset  = *(dsets[ 0 ]);
      wkdsets.clear();

      for ( int jt = 0; jt < nthr; jt++ )
      {  // Build up a list of datasets for each thread
         wkdsets << wkdset;
      }

      // Start the next few iterations in available threads
      for ( int jt = 0; jt < nthr; jt++ )
      {
         apply_gaussians();

         WorkPacket wtask       = wtbase;
         US_SolveSim::Simulation        sim_vals;
         sim_vals.solutes       = mrec.isolutes;

         wtask.thrn             = jt + 1;
         wtask.taskx            = jt + 1;
         wtask.str_k            = mrec.str_k;
         wtask.end_k            = mrec.end_k;
         wtask.sim_vals         = sim_vals;
         wtask.dsets[ 0 ]       = &wkdsets[ jt ];

         wtask.dsets[ 0 ]->run_data = wdata;

         WorkerThread* wthrd    = new WorkerThread( this );
         connect( wthrd, SIGNAL( work_complete( WorkerThread* ) ),
                  this,  SLOT  ( process_job  ( WorkerThread* ) ) );

         wthrd->define_work( wtask );

         wthrd->start();

         ksiters++;
DbgLv(1) << "    ksiters" << ksiters << "dsets[0]" << wtask.dsets[0];
      }

   }

}

// Set gaussian distribution: sigmas and iteration 1 simulation data
void US_AdvAnalysis::set_gaussians( US_SolveSim::Simulation& sim_vals )
{
   int nscans   = edata->scanCount();
   int npoints  = edata->pointCount();
   US_DataIO::RawData*   sdata = &sim_vals.sim_data;
   US_DataIO::RawData*   rdata = &sim_vals.residuals;

   sigmas.clear();

   for ( int ss = 0; ss < nscans; ss++ )
   {
      QVector< double > vv( npoints );

      for ( int rr = 0; rr < npoints; rr++ )
      {
         vv[ rr ]   = qAbs( rdata->value( ss, rr ) );
      }

//      US_Math2::gaussian_smoothing( vv, 5 );
      sigmas << vv;
   }

   sdata1       = *sdata;
}

// Apply gaussians: add in random variations of sigmas to base simulation
void US_AdvAnalysis::apply_gaussians()
{
   int nscans   = edata->scanCount();
   int npoints  = edata->pointCount();
   int kk       = 0;

   for ( int ss = 0; ss < nscans; ss++ )
   {
      for ( int rr = 0; rr < npoints; rr++ )
      {
         double svari  = US_Math2::box_muller( 0.0, sigmas[ kk++ ] );
         wdata.setValue( ss, rr, ( sdata1.value( ss, rr ) + svari ) );
      }
   }
}

// Process the completion of an MC worker thread
void US_AdvAnalysis::process_job( WorkerThread* wthr )
{
   WorkPacket wresult;
   wthr->get_result( wresult );
   kciters++;

   ModelRecord mrec_mc = mrecs_mc[ 0 ];
   mrec_mc.variance = wresult.sim_vals.variance;
   mrec_mc.rmsd     = sqrt( mrec_mc.variance );
   mrec_mc.csolutes = wresult.sim_vals.solutes;
   ncsols           = mrec_mc.csolutes.size();
DbgLv(1) << "  kciters" << kciters << "rmsd" << mrec_mc.rmsd
 << "ncsols" << ncsols;
   QString sline1 = tr( "%1 Monte Carlo iterations are being"
                         " computed in %2 threads...\n" )
                    .arg( mciters ).arg( nthr );
   te_bfmstat ->setText( sline1 +
      tr( "%1 are completed ( last:  %2-solute,  RMSD=%3 )" )
      .arg( kciters ).arg( ncsols ).arg( mrec_mc.rmsd ) );
   b_progress->setValue( kciters );
   qApp->processEvents();

   mrecs_mc << mrec_mc;

   if ( kciters == mciters )
   {  // Complete the MonteCarlo process
      b_progress->setValue( mciters );

      montecarlo_done();

      QApplication::restoreOverrideCursor();
      qApp->processEvents();
   }

   else if ( ksiters < mciters )
   {  // Submit a new task
//DbgLv(1) << "    ksiters" << ksiters << "   apply_gaussians";
      apply_gaussians();
//DbgLv(1) << "    ksiters" << ksiters << "    retn: apply_gaussians";

      WorkerThread* wthrd        = wthr;
      WorkPacket    wtask        = wresult;
      wtask.dsets[ 0 ]->run_data = wdata;
      wtask.taskx                = ksiters;

      wthrd->disconnect();
      connect( wthrd, SIGNAL( work_complete( WorkerThread* ) ),
               this,  SLOT  ( process_job  ( WorkerThread* ) ) );

//DbgLv(1) << "    ksiters" << ksiters << "     define,start";
      wthrd->define_work( wtask );
      wthrd->start();

      ksiters++;
DbgLv(1) << "    ksiters" << ksiters << "dsets[0]" << wtask.dsets[0];
   }
}

// Complete model records and the final model after monte carlo completion
void US_AdvAnalysis::montecarlo_done( void )
{
DbgLv(1) << "==montecarlo_done()==";
   te_bfmstat ->setText( te_bfmstat->toPlainText() +
      tr( "\nBuilding MC models and final composite..." ) );
   int     nccsol   = 0;
   QVector< US_Solute > compsols;
   QStringList sortlst;
   US_Model::SimulationComponent zcomponent;
   zcomponent.vbar20   = dset0->vbar20;

   // Build individual models and append all solutes to one composite
   for ( int jmc = 0; jmc < mciters; jmc++ )
   {
      mrec             = mrecs_mc[ jmc ];
      QVector< US_Solute > csolutes = mrec.csolutes;
      int     nsols    = csolutes.size();
      model.components.resize( nsols );

      for ( int cc = 0; cc < nsols; cc++ )
      {
         // Get component values and sorting string
         double  sol_s    = csolutes[ cc ].s;
         double  sol_k    = csolutes[ cc ].k;
         double  sol_c    = csolutes[ cc ].c;
         QString sol_id   = QString().sprintf( "%.4f:%.4f:%d",
            sol_s * 1.e13, sol_k, nccsol++ );
DbgLv(1) << "MCD: cc" << cc << "sol_id" << sol_id;

         // Save unsorted/summed solute and sorting string
         US_Solute compsol;
         compsol.s        = sol_s;
         compsol.k        = sol_k;
         compsol.c        = sol_c;
         compsols << compsol;
         sortlst  << sol_id;

         // Build the model component
         model.components[ cc ]                       = zcomponent;
         model.components[ cc ].s                     = sol_s;
         model.components[ cc ].f_f0                  = sol_k;
         model.components[ cc ].signal_concentration  = sol_c;
         model.components[ cc ].name = QString().sprintf( "SC%04d", cc + 1 );
         model.calc_coefficients( model.components[ cc ] );
      }

      // Save the individual MC model
      model.monteCarlo = true;
      mrec.model       = model;
      mrecs_mc[ jmc ]  = mrec;
   }

   // Now sort the solute id strings to create sorted composite
   qSort( sortlst );
   US_Solute pcompsol;
   US_Solute ccompsol;
   QString pskmatch  = QString();
   mrec.csolutes.clear();
   double  cnorm     = 1.0 / (double)mciters;
   ncsols            = 0;

   for ( int cc = 0; cc < nccsol; cc++ )
   {
      QString sol_id    = sortlst[ cc ];
      QString skmatch   = sol_id.section( ":", 0, 1 );
      int     ccin      = sol_id.section( ":", 2, 2 ).toInt();
      ccompsol          = compsols[ ccin ];

      if ( skmatch != pskmatch )
      {  // New s,k combination:  output previous component, start new sum
         if ( cc > 0 )
         {
            pcompsol.c    *= cnorm;
            mrec.csolutes << pcompsol;
            ncsols++;
         }

         pcompsol       = ccompsol;
         pskmatch       = skmatch;
DbgLv(1) << "MCD: cc ccin ncsols" << cc << ccin << ncsols;
      }

      else
      {  // Identical s,k to previous:  sum concentration;
         pcompsol.c    += ccompsol.c;
      }
   }

   // Output last component
   pcompsol.c    *= cnorm;
   mrec.csolutes << pcompsol;
   ncsols++;
   US_Model modela;

   model          = mrec.model;           // Model to pass back to main
   model.components.resize( ncsols );
   modela         = model;                // Model for application (corrected)
   double sfactor = 1.0 / dset0->s20w_correction;
   double dfactor = 1.0 / dset0->D20w_correction;

   // Build the model that goes along with the new composite model record
   for ( int cc = 0; cc < ncsols; cc++ )
   {
      model.components[ cc ]                       = zcomponent;
      model.components[ cc ].s                     = mrec.csolutes[ cc ].s;
      model.components[ cc ].f_f0                  = mrec.csolutes[ cc ].k;
      model.components[ cc ].signal_concentration  = mrec.csolutes[ cc ].c;
      model.components[ cc ].name = QString().sprintf( "SC%04d", cc + 1 );
      model.calc_coefficients( model.components[ cc ] );

      modela.components[ cc ]                      = model.components[ cc ];
      modela.components[ cc ].s                   *= sfactor;
      modela.components[ cc ].D                   *= dfactor;
   }

   // Do a fit with the composite model and get the RMSD
                       edata   = &dset0->run_data;
   US_DataIO::RawData* sdata   = &mrec.sim_data;
   US_AstfemMath::initSimData( *sdata, *edata, 0.0 );

   US_Astfem_RSA astfem_rsa( modela, dset0->simparams );
   astfem_rsa.calculate( *sdata );

   int    nscans  = edata->scanCount();
   int    npoints = edata->pointCount();
   double varia   = 0.0;

   for ( int ss = 0; ss < nscans; ss++ )
      for ( int rr = 0; rr < npoints; rr++ )
         varia   += sq( ( edata->value( ss, rr ) - sdata->value( ss, rr ) ) );

   varia         /= (double)( nscans * npoints );
   mrec.variance  = varia;
   mrec.rmsd      = sqrt( varia );
   mrec.model     = model;
   mrecs[ 0 ]     = mrec;

   // Report MC completion status
   te_bfmstat ->setText( te_bfmstat->toPlainText() +
      tr( "\nMC models and final %1-solute composite model are built." )
      .arg( ncsols ) );

   te_mrecstat->setText(
      tr( "A best final %1-solute model ( RMSD = %2 )\n"
          "  now occupies the top curve model records list spot." )
      .arg( ncsols ).arg( mrec.rmsd ) );

   bfm_new        = true;
   mrs_new        = true;
   mc_done        = true;
}

// Pop up an under-construction message dialog
void US_AdvAnalysis::under_construct( QString proc )
{
   QMessageBox::information( this, tr( "UNDER CONSTRUCTION" ),
      tr( "Implementation of <b>%1</b> is not yet complete." ).arg( proc ) );
}

