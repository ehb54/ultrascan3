//! \file us_mwl_species_sim.cpp
//!
//! The US_MwlSpeciesSim implementation, kept separate from the application's
//! main() in main.cpp so that a test can link this class without pulling in a
//! second main(). us_astfem_sim splits us_clipdata.cpp for the same reason.

#include <QApplication>

#include "us_headless_cli.h"
#include "us_mwl_species_sim.h"
#include "us_data_loader.h"
#include "us_select_runs.h"
#include "us_astfem_rsa.h"
#include "us_astfem_math.h"
#include "us_math2.h"
#include "us_sim_inputs.h"
#include "us_hardware.h"
#include "us_license_t.h"
#include "us_license.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_util.h"
#include "us_experiment.h"
#include "us_convert.h"
#include "us_sim_record.h"
#include "us_passwd.h"
#include "us_images.h"

#include "qwt_picker_machine.h"
#define dPlotClearAll(a) a->detachItems(QwtPlotItem::Rtti_PlotItem,true)


//! \brief Constructor for US_MwlSpeciesSim class
US_MwlSpeciesSim::US_MwlSpeciesSim() : US_Widgets()
{
   setWindowTitle( tr( "MWL Pre-Fit Species Simulation" ) );
   setPalette( US_GuiSettings::frameColor() );

   dbg_level   = US_Settings::us_debug();
   // Only --runtype changes this; the GUI has no control for it.
   run_type    = "RA";
   nmodels     = 0;
   tripx       = 0;
   dbload      = ( US_Settings::default_data_location() == 1 );
   stopFlag    = false;
   mfilt       = QString( "2DSA-IT" );
   synData.clear();
   have_p1.clear();

   init_simparams();

   QHBoxLayout* main = new QHBoxLayout( this );
   QGridLayout* left = new QGridLayout();
   QBoxLayout*  plot = new US_Plot( data_plot1,
      tr( "MWL Species Simulation" ),
      tr( "Sedimentation Coefficient x 1e-13" ),
      tr( "Amplitude in OD" ) );

   data_plot1->setTitle( tr( "MWL Species Simulation" ) ),
   data_plot1->setMinimumSize( 600, 300 );

   pb_prefilt  = us_pushbutton( tr( "PreFilter Models"      ) );
   pb_semodels = us_pushbutton( tr( "Select Models"         ) );
   pb_defbuff  = us_pushbutton( tr( "Define Buffer"         ) );
   pb_simparms = us_pushbutton( tr( "Simulation Parameters" ) );
   pb_selrotor = us_pushbutton( tr( "Select Rotor"          ) );
   pb_strtsims = us_pushbutton( tr( "Start Simulations"     ) );
   pb_stopsims = us_pushbutton( tr( "Stop Simulations"      ) );
   pb_savesims = us_pushbutton( tr( "Save Simulations"      ) );
   pb_prev     = us_pushbutton( tr( "Previous"  ) );
   pb_next     = us_pushbutton( tr( "Next"      ) );
   pb_help     = us_pushbutton( tr( "Help"      ) );
   pb_close    = us_pushbutton( tr( "Close"     ) );

   pb_prev->setIcon( US_Images::getIcon( US_Images::ARROW_LEFT  ) );
   pb_next->setIcon( US_Images::getIcon( US_Images::ARROW_RIGHT ) );

   QLabel* lb_specplot  = us_banner( tr ( "Species to Plot:" ) );
   QLabel* lb_inforun   = us_banner( tr ( "Information for this Run:" ) );
   QLabel* lb_runid     = us_label ( tr ( "Run ID:" ) );
   QLabel* lb_triples   = us_label ( tr ( "Triples:" ) );

   le_runid    = us_lineedit( "", -1, true );
   le_triples  = us_lineedit( "", -1, true );
   te_status   = us_textedit();
   us_setReadOnly( le_runid,   true );
   us_setReadOnly( le_triples, true );
   us_setReadOnly( te_status,  true );

   int row     = 3;
   left->addWidget( pb_prefilt,  row,   0, 1, 2 );
   left->addWidget( pb_semodels, row++, 2, 1, 2 );
   left->addWidget( pb_defbuff,  row,   0, 1, 2 );
   left->addWidget( pb_simparms, row++, 2, 1, 2 );
   left->addWidget( pb_selrotor, row,   0, 1, 2 );
   left->addWidget( pb_strtsims, row++, 2, 1, 2 );
   left->addWidget( pb_stopsims, row,   0, 1, 2 );
   left->addWidget( pb_savesims, row++, 2, 1, 2 );
   left->addWidget( lb_specplot, row++, 0, 1, 4 );
   left->addWidget( pb_prev,     row,   0, 1, 2 );
   left->addWidget( pb_next,     row++, 2, 1, 2 );
   left->addWidget( lb_inforun,  row++, 0, 1, 4 );
   left->addWidget( lb_runid,    row,   0, 1, 1 );
   left->addWidget( le_runid,    row++, 1, 1, 3 );
   left->addWidget( lb_triples,  row,   0, 1, 1 );
   left->addWidget( le_triples,  row++, 1, 1, 3 );
   left->addWidget( te_status,   row,   0, 3, 4 );
   row        += 5;
   left->addWidget( pb_help,     row,   0, 1, 2 );
   left->addWidget( pb_close,    row++, 2, 1, 2 );

   pb_strtsims->setEnabled( false );
   pb_stopsims->setEnabled( false );
   pb_savesims->setEnabled( false );
   pb_prev    ->setEnabled( false );
   pb_next    ->setEnabled( false );

   connect( pb_prefilt,  SIGNAL( clicked      () ),
            this,        SLOT  ( pre_filt     () ) );
   connect( pb_semodels, SIGNAL( clicked      () ),
            this,        SLOT  ( select_models() ) );
   connect( pb_defbuff,  SIGNAL( clicked      () ),
            this,        SLOT  ( define_buffer() ) );
   connect( pb_simparms, SIGNAL( clicked      () ),
            this,        SLOT  ( sim_params   () ) );
   connect( pb_selrotor, SIGNAL( clicked      () ),
            this,        SLOT  ( select_rotor () ) );
   connect( pb_strtsims, SIGNAL( clicked      () ),
            this,        SLOT  ( start_sims   () ) );
   connect( pb_stopsims, SIGNAL( clicked      () ),
            this,        SLOT  ( stop_sims    () ) );
   connect( pb_savesims, SIGNAL( clicked      () ),
            this,        SLOT  ( save_sims    () ) );
   connect( pb_prev,     SIGNAL( clicked      () ),
            this,        SLOT  ( prev_plot    () ) );
   connect( pb_next,     SIGNAL( clicked      () ),
            this,        SLOT  ( next_plot    () ) );
   connect( pb_help,     SIGNAL( clicked      () ),
            this,        SLOT  ( help         () ) );
   connect( pb_close,    SIGNAL( clicked      () ),
            this,        SLOT  ( close_all    () ) );

   main->addLayout( left );
   main->addLayout( plot );
   main->setStretchFactor( left, 3 );
   main->setStretchFactor( plot, 5 );
}

// Plot currently selected channel's data
void US_MwlSpeciesSim::data_plot( void )
{
   if ( nmodels > 0 )
   {
      plot_data1();
   }
}

// Plot the AUC data
void US_MwlSpeciesSim::plot_data1( void )
{
   int ndatas    = synData.count();
if(tripx>=0 && tripx<ndatas)
DbgLv(1) << "PlotData1: tripx" << tripx << "have" << have_p1[tripx];
   if ( tripx < 0  ||  tripx >= ndatas  ||  ! have_p1[ tripx ] )
      return;
   dPlotClearAll( data_plot1 );

DbgLv(1) << "PlotData1:  tripx" << tripx;
   US_DataIO::RawData*     rdata = &synData[ tripx ];
   QString cell   = QString::asprintf( "%d", rdata->cell );
   QString wavl   = QString::asprintf( "%d",
(int )rdata->scanData[ 0 ].wavelength );
   QString mdesc  = models[ tripx ].description;
   QString triple = QString( mdesc ).section( ".", -3, -3 );
   QString chani  = QString( triple ).left( 2 );
   QString chano  = QString( "S" );
   QString runid  = "ISSF-" + mdesc.section( ".", 0, -4 )
                    + "-" + chani;
   int irunx      = runid.indexOf( "-run" );
   int lnruni     = runid.length();
   irunx          = ( irunx > 0 ) ? irunx : ( lnruni / 2 );
   QString runid1 = QString( runid ).left( irunx );
   QString runid2 = QString( runid ).mid ( irunx );

   QString dataType = tr( "Optical Density" );

   QString header = tr( "Simulated Raw Data for\n  ")
         + runid1 + "\n" + runid2
         + " ( " + cell + " / " + chano + " / " + wavl + " )";
   data_plot1->setTitle( header );
   data_plot1->setAxisTitle( QwtPlot::yLeft,   tr( "Optical Density" ) );
   data_plot1->setAxisTitle( QwtPlot::xBottom, tr( "Radius (cm) "    ) );
   data_plot1->setAxisScale    ( QwtPlot::yLeft, 0.0, mtconcs[ tripx ] * 2.0 );
   data_plot1->setAxisAutoScale( QwtPlot::xBottom );

   us_grid( data_plot1 );

   int    scan_number = 0;
   int    scanCount   = rdata->scanCount();
   int    points      = rdata->pointCount();

   QVector< double > rvec( points );
   QVector< double > vvec( points );
   double* rr         = rvec.data();
   double* vv         = vvec.data();

   // Draw curves
   for ( int ii = 0; ii < scanCount; ii++ )
   {
      scan_number++;

      US_DataIO::Scan*  rscan = &rdata->scanData[ ii ];

      // Plot each scan

      for ( int jj = 0; jj < points; jj++ )
      {
         rr[ jj ] = rdata->xvalues[ jj ];
         vv[ jj ] = rscan->rvalues[ jj ];
      }

      QString       title = tr( "Curve " ) + QString::number( ii );
      QwtPlotCurve* curv  = us_curve( data_plot1, title );

      curv->setPen( QPen( US_GuiSettings::plotCurve() ) );
         
      curv->setSamples( rr, vv, points );
   }

   data_plot1->replot();

//   te_desc->setText( rdata->description );

   return;
}

// Select a pre-filter run (set of edits) for model filtering
void US_MwlSpeciesSim::pre_filt( void )
{
DbgLv(1) << "SLOT: pre_filt";
   QString pfmsg;
   int nruns  = 0;

   pfilts.clear();

   US_SelectRuns srdiag( dbload, pfilts );

   if ( srdiag.exec() == QDialog::Accepted )
      nruns         = pfilts.size();
   else
      pfilts.clear();

   if ( nruns == 1 )
      pfmsg = tr( "RunID prefilter - 1 run:\n  " )
              + QString( pfilts[ 0 ] ).left( 20 ) + " ...";

   else if ( nruns > 1 )
      pfmsg = tr( "RunID prefilter - %1 runs:\n  " ).arg( nruns )
              + QString( pfilts[ 0 ] ).left( 20 ) + "*, ...";

   else
      pfmsg = tr( "(no prefilter)" );

   te_status->setText( pfmsg );

}

// Select a set of models
void US_MwlSpeciesSim::select_models( void )
{
DbgLv(1) << "SLOT: select_models";
   // Get a set of descriptions for distribution data
DbgLv(1) << "  smdls: call ML dbload" << dbload << "mfilt" << mfilt
 << "pfilts" << pfilts;
   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
   US_ModelLoader dialog( dbload, mfilt, models, mdescs, pfilts );

   QApplication::restoreOverrideCursor();
   qApp->processEvents();

   if ( dialog.exec() != QDialog::Accepted )
      return;  // no selection made

   nmodels        = models.count();

   if ( nmodels < 1 )
      return;

   // Before the totals below are summed, so the clip and the noise sigmas
   // that read them see the scaled amplitudes.
   apply_extinction_scaling();

   mtconcs.fill( 0.0, nmodels );

   QStringList runids;
   QStringList chans;
   QStringList wavelns;

   for ( int jm = 0; jm < nmodels; jm++ )
   {
      // Accumulate runs, channels, wavelengths present in models
      QString mdesc  = models[ jm ].description;
      QString runid  = QString( mdesc ).section( ".",  0, -4 );
      QString triple = QString( mdesc ).section( ".", -3, -3 );
      QString chan   = QString( triple ).left( 2 );
      QString waveln = QString( triple ).mid( 2, 3 );

      if ( ! runids.contains( runid ) )
         runids  << runid;

      if ( ! chans .contains( chan ) )
         chans   << chan;

      if ( ! wavelns.contains( waveln ) )
         wavelns << waveln;
//*DEBUG
if(jm<5 || (jm+5)>nmodels ) {
 DbgLv(1) << "  smdls: jm" << jm << "model.desc"
  << mdesc << "mdescs[jm]" << mdescs[jm];
}
//*DEBUG

      // Compute and save the total concentration in each model
      double tot_conc = 0.0;

      for ( int jc = 0; jc < models[ jm ].components.count(); jc++ )
      {
         tot_conc      += models[ jm ].components[ jc ].signal_concentration;
      }

      mtconcs[ jm ]  = tot_conc;
DbgLv(1) << "  smdls:   jm" << jm << "tot_conc" << tot_conc;
   }

   int nruns      = runids .count();
   int nchans     = chans  .count();
   int nwavls     = wavelns.count();
DbgLv(1) << "  smdls: nmodels" << nmodels << "nruns" << nruns
 << "nchans" << nchans << "nwavls" << nwavls;

   if ( nruns != 1  ||  nchans != 1  ||  nwavls != nmodels )
   {
      qDebug() << "expected: runs chans wavelns" << 1 << 1 << nmodels;
      qDebug() << "have:     runs chans wavelns" << nruns << nchans << nwavls;
   }

   mrunid         = runids[ 0 ];
   orunid         = "ISSF-" + mrunid + "-" + chans[ 0 ];
   QString triple = chans[ 0 ].left( 1 ) + "."
                  + chans[ 0 ].mid( 1, 1 ) + "."
                  + wavelns[ 0 ] + "-" + wavelns[ nwavls - 1 ];
   le_triples->setText( triple );
   le_runid  ->setText( orunid );

   pb_strtsims->setEnabled( true );
}

// Define the buffer for the run
void US_MwlSpeciesSim::define_buffer( void )
{
DbgLv(1) << "SLOT: define_buffer";
   US_BufferGui* dialog = new US_BufferGui( true, buffer );

   connect( dialog, SIGNAL( valueChanged ( US_Buffer ) ),
                    SLOT  ( change_buffer( US_Buffer ) ) );

   dialog->exec();
   qApp->processEvents();
}

// Set parameters based on a newly selected buffer
void US_MwlSpeciesSim::change_buffer( US_Buffer b )
{
   buffer = b;

   if ( buffer.compressibility  >  0.0 )
      simparams.meshType = US_SimulationParameters::ASTFVM;
}

// Open a dialog to select simulation parameters
void US_MwlSpeciesSim::sim_params( void )
{
DbgLv(1) << "SLOT: sim_params";
   US_SimParamsGui* dialog = new US_SimParamsGui( simparams );
   
   connect( dialog, SIGNAL( complete() ), SLOT( set_parameters() ) );

   dialog->exec();
}

// Set simulation parameter as selected in the simparams dialog
void US_MwlSpeciesSim::set_parameters( void )
{
   double rad_precis          = simparams.radial_resolution * 0.1;
   simparams.meniscus         = qRound( simparams.meniscus
                                / rad_precis ) * rad_precis;
   simparams.bottom_position  = qRound( simparams.bottom_position
                                / rad_precis ) * rad_precis;
   simparams.bottom           = simparams.bottom_position;

   double delay        = simparams.speed_step[ 0 ].delay_hours * 3600.0
                       + simparams.speed_step[ 0 ].delay_minutes * 60.0;
   double duration     = simparams.speed_step[ 0 ].duration_hours * 3600.0
                       + simparams.speed_step[ 0 ].duration_minutes * 60.0;
   double rspeed       = (double)simparams.speed_step[ 0 ].rotorspeed;
   int    scans        = simparams.speed_step[ 0 ].scans;
   delay               = qRound( delay );
   duration            = qRound( duration );
   double pi_fac       = sq( M_PI / 30.0 );
   double tim_rng      = duration - delay;
   double tim_inc      = ( scans > 1 ) ? tim_rng / (double)( scans - 1 ) : 0.0;
   double w2t_fac      = sq( rspeed ) * pi_fac;
   double accel        = simparams.speed_step[ 0 ].acceleration;
   double w2t_inc      = tim_inc * w2t_fac;
   double tim_val      = 0.0;
   double w2t_val      = 0.0;
   double aspeed       = 0.0;
   double rpm_inc      = rspeed / qCeil( rspeed / ( accel == 0.0 ? 1.0 : accel ) );

   while ( aspeed < rspeed )
   {  // Walk time and omega2t through acceleration zone
      aspeed             += rpm_inc;
      tim_val            += 1.0;
      w2t_val            += sq( aspeed ) * pi_fac;
   }

   while( tim_val < delay )
   {  // Walk time and omega2t up to the first scan
      tim_val            += 1.0;
      w2t_val            += w2t_fac;
   }

   simparams.speed_step[ 0 ].time_first  = tim_val;
   simparams.speed_step[ 0 ].w2t_first   = w2t_val;

   if ( scans > 1 )
   {
      while( tim_val < duration )
      {  // Walk time and omega2t up to the last scan
         tim_val            += tim_inc;
         w2t_val            += w2t_inc;
      }

      simparams.speed_step[ 0 ].time_last = duration;
      simparams.speed_step[ 0 ].w2t_last  = w2t_val;
   }
   else
   {  // A one-scan run has only its first-scan time and omega-squared-t.
      simparams.speed_step[ 0 ].time_last =
         simparams.speed_step[ 0 ].time_first;
      simparams.speed_step[ 0 ].w2t_last  =
         simparams.speed_step[ 0 ].w2t_first;
   }
   simparams.speed_step[ 0 ].set_speed   = (int)rspeed;
   simparams.speed_step[ 0 ].avg_speed   = rspeed;

//*DEBUG*
DbgLv(1) << "set_params:";
simparams.debug();
//*DEBUG*
}

// Select a rotor
void US_MwlSpeciesSim::select_rotor( void )
{
DbgLv(1) << "SLOT: select_rotor";
     US_Rotor::Rotor rotor;
     US_Rotor::RotorCalibration calibration;

     int dbdisk = dbload ? US_Disk_DB_Controls::DB
                         : US_Disk_DB_Controls::Disk;
    
     US_RotorGui* rotorInfo = new US_RotorGui( true, dbdisk,
                                               rotor, calibration );
   
    connect( rotorInfo, SIGNAL( RotorCalibrationSelected(
                           US_Rotor::Rotor&, US_Rotor::RotorCalibration& ) ),
             this,      SLOT  ( assign_rotor            (
                           US_Rotor::Rotor&, US_Rotor::RotorCalibration& ) ) );

    rotorInfo->exec();
}

// Set stretch coefficients based on a selected rotor
void US_MwlSpeciesSim::assign_rotor( US_Rotor::Rotor& arotor,
                                     US_Rotor::RotorCalibration& calibration )
{
   rotor                      = arotor;
   rotor_calib                = calibration;
   simparams.rotorcoeffs[0]   = rotor_calib.coeff1;
   simparams.rotorcoeffs[1]   = rotor_calib.coeff2;
DbgLv(1) << "assign_rotor: rotor" << rotor.name
 << "coeffs" << simparams.rotorcoeffs[0] << simparams.rotorcoeffs[1];
}

// Scale every model's signal concentration by its extinction coefficient.
//
// Each model here is one wavelength, so the set of models is a sampling of the
// analyte's absorbance spectrum. Without this, every wavelength simulates at
// the amplitude its model file states, which for a set generated from one
// species means every wavelength comes out identical: a multi-wavelength run
// whose wavelengths carry no wavelength-dependent information at all.
//
// The scale factor is normalized against the strongest absorber in the set
// rather than applied raw. Extinction coefficients are molar and run to five
// figures, so multiplying a signal concentration of 1 by 10000 would ask for an
// optical density no instrument produces, and the clip in build_rawdata() would
// flatten it. Normalizing here lets a model file carry recognizable molar
// extinctions while the strongest wavelength keeps the amplitude that file
// asks for and the rest fall below it in the spectrum's proportions.
//
// Extinction defaults to zero (us_model.cpp) and every model written before
// this existed carries zero, so a set with no extinction data takes the early
// return and simulates exactly as it did before. That is what keeps existing
// models, and the GUI paths that load them, unaffected.
void US_MwlSpeciesSim::apply_extinction_scaling( void )
{
   double max_extinc  = 0.0;

   for ( int jm = 0; jm < models.count(); jm++ )
      for ( int jc = 0; jc < models[ jm ].components.count(); jc++ )
         max_extinc      = qMax( max_extinc,
                                 models[ jm ].components[ jc ].extinction );

   if ( max_extinc <= 0.0 )
   {  // No extinction data in this set: leave every amplitude as loaded.
DbgLv(1) << "extinc: no extinction data; amplitudes unscaled";
      return;
   }

   for ( int jm = 0; jm < models.count(); jm++ )
   {
      for ( int jc = 0; jc < models[ jm ].components.count(); jc++ )
      {
         US_Model::SimulationComponent* sc = &models[ jm ].components[ jc ];

         // A component with no extinction of its own, in a set where others
         // have it, absorbs nothing at this wavelength. Scaling it to zero is
         // the honest reading of that, not a degenerate case to guard against.
         sc->signal_concentration *= ( sc->extinction / max_extinc );
      }
   }
DbgLv(1) << "extinc: scaled" << models.count() << "models; max extinction"
 << max_extinc;
}

// Start the simulations for all chosen models
void US_MwlSpeciesSim::start_sims( void )
{
DbgLv(1) << "SLOT: start_sims";
   stopFlag        = false;

   if ( nmodels < 1 )
      return;

   // Create the template rawData for all to be created
   init_rawdata();

   // Create rawData's for all models
   pb_stopsims->setEnabled( true );
   qApp->processEvents();
   QString smsg;
   int kmodels    = 0;
   synData.clear();
   have_p1.clear();

   // Drop the previous run's systematic noise so a re-run draws fresh
   // vectors rather than reusing the ones the last run's wavelengths shared.
   shared_ti.clear();
   shared_ri.clear();

   for ( int jm = 0; jm < nmodels; jm++ )
   {
      tripx            = jm;
      US_Model model   = models[ jm ];
DbgLv(1) << " sims: build from model" << model.description;
      QString mdesc  = model.description;
      QString triple = QString( mdesc ).section( ".", -3, -3 );

      smsg           = tr( "Of %1 models, building for: %2 (%3) ..." )
                       .arg( nmodels ).arg( jm + 1 ).arg( triple );
      te_status->setText( smsg );
      qApp->processEvents();
//sleep( 3 );
      kmodels++;
      if ( stopFlag )
         break;

      build_rawdata();
   }

   if ( kmodels == nmodels )
      smsg           = tr( "Raw Data have been built for all %1 models." )
                       .arg( nmodels );
   else
      smsg           = tr( "Raw Data builds stopped after %1 models." )
                       .arg( kmodels );

   te_status->setText( smsg );
   qApp->processEvents();
   stopFlag        = false;
   pb_stopsims->setEnabled( false );
   pb_prev    ->setEnabled( true );
   pb_next    ->setEnabled( true );
   pb_savesims->setEnabled( true );
}

// Set the STOP flag so building simulations ceases
void US_MwlSpeciesSim::stop_sims( void )
{
DbgLv(1) << "SLOT: stop_sims";
   stopFlag        = true;
}

// Save simulations to an imports directory, along with time state
void US_MwlSpeciesSim::save_sims( void )
{
DbgLv(1) << "SLOT: save_sims";
   save_sims_to( US_Settings::importDir() + "/" + orunid );
}

// Run the simulation headlessly using command-line options.
int US_MwlSpeciesSim::init_from_args( const QMap<QString, QString>& flags )
{
   bool gui_needed      = !flags.contains( "close" );
   bool error_occured   = false;
   bool errors_to_cl    = flags.contains( "errors-cl" );

   // Validated in main(); absent, the constructor's "RA" stands.
   // Reproducible identity. Both stay empty unless asked for, so the desktop
   // and every existing caller keep minting fresh GUIDs and stamping the clock.
   if ( flags.contains( "guid-seed" ) )
      guid_seed  = flags[ "guid-seed" ];

   if ( flags.contains( "edit-timestamp" ) )
      edit_stamp = flags[ "edit-timestamp" ];

   if ( flags.contains( "runtype" ) && flags[ "runtype" ].length() == 2 )
      run_type = flags[ "runtype" ];

   // Each input is optional. Only an explicitly requested input that fails
   // to load should prevent start_sims() below.
   bool loaded_models    = true;
   bool loaded_buffer    = true;
   bool loaded_simparams = true;
   bool loaded_rotor     = true;

   if ( flags.contains( "models" ) && flags[ "models" ].length() > 0 )
   {
      QStringList paths = flags[ "models" ].split( ",", Qt::SkipEmptyParts );
      loaded_models      = load_models_from_paths( paths );
      if ( ! loaded_models )
         reportHeadlessLoadFailure( "models", paths.join( "," ), errors_to_cl,
                                     gui_needed, error_occured );
   }

   if ( flags.contains( "buffer" ) && flags[ "buffer" ].length() > 0 )
   {
      QString load_id     = flags[ "buffer" ];
      US_BufferGui* dialog = new US_BufferGui( true, buffer, US_Disk_DB_Controls::Default );
      bool success         = dialog->load_buffer( load_id, buffer );
      dialog->close();
      if ( ! success )
      {
         reportHeadlessLoadFailure( "buffer", load_id, errors_to_cl,
                                     gui_needed, error_occured );
         loaded_buffer      = false;
      }
      else
      {
         change_buffer( buffer );
      }
      delete dialog;
   }

   if ( flags.contains( "simparams" ) && flags[ "simparams" ].length() > 0 )
   {
      QString load_id      = flags[ "simparams" ];
      US_SimParamsGui* dialog = new US_SimParamsGui( simparams );
      bool success          = dialog->load_params( load_id, simparams );
      dialog->close();
      if ( ! success )
      {
         reportHeadlessLoadFailure( "simparams", load_id, errors_to_cl,
                                     gui_needed, error_occured );
         loaded_simparams    = false;
      }
      else
      {
         // The file omits bottom_position; mirror the loaded at-rest bottom.
         // An explicit --centerpiece overrides it below.
         simparams.bottom_position = simparams.bottom;
         set_parameters();
      }
      delete dialog;
   }

   if ( flags.contains( "rotor" ) && flags[ "rotor" ].length() > 0 )
   {
      US_Rotor::Rotor rotorval;
      US_Rotor::RotorCalibration calibration;
      QString rotor_id      = flags[ "rotor" ];
      US_Disk_DB_Controls* disk_controls = new US_Disk_DB_Controls( US_Disk_DB_Controls::Default );
      int dbdisk = ( disk_controls->db() ) ? US_Disk_DB_Controls::DB
                                            : US_Disk_DB_Controls::Disk;
      US_RotorGui* rotorInfo = new US_RotorGui( true, dbdisk, rotorval, calibration );
      double coeff1 = 0.0;
      double coeff2 = 0.0;
      bool status = rotorInfo->load_rotor( rotor_id, coeff1, coeff2 );

      if ( status )
      {
         rotor                     = rotorInfo->currentRotor;
         rotor_calib                = rotorInfo->currentCalibration;
         simparams.rotorcoeffs[0]   = coeff1;
         simparams.rotorcoeffs[1]   = coeff2;
         simparams.rotorCalID       = QString::number( rotorInfo->currentCalibration.ID );
         rotorInfo->close();
      }
      else
      {
         rotorInfo->close();
         reportHeadlessLoadFailure( "rotor", rotor_id, errors_to_cl,
                                     gui_needed, error_occured );
         loaded_rotor        = false;
      }
      delete rotorInfo;
      delete disk_controls;
   }

   // A centerpiece selects the row geometry and overrides the loaded bottom.
   // Preserve rotorCalID so its calibration remains in effect.
   if ( flags.contains( "centerpiece" ) || flags.contains( "centerpiece-channel" ) )
   {
      int cp = 0;
      int ch = 0;
      QString parse_error;

      if ( ! US_AbstractCenterpiece::parse_index( flags.value( "centerpiece", "0" ), cp, parse_error )
           || ! US_AbstractCenterpiece::parse_channel( flags.value( "centerpiece-channel", "0" ),
                                          ch, parse_error ) )
      {
         reportHeadlessLoadFailure( "centerpiece", parse_error, errors_to_cl,
                                     gui_needed, error_occured );
      }
      else
      {
         QString range_error = US_AbstractCenterpiece::validate( cp, ch );
         if ( ! range_error.isEmpty() )
         {
            reportHeadlessLoadFailure( "centerpiece", range_error, errors_to_cl,
                                        gui_needed, error_occured );
         }
         else if ( ! simparams.setHardware( NULL, simparams.rotorCalID,
                                            cp, ch ) )
         {  // Ignoring this would silently fall back to the 7.2 default bottom
            reportHeadlessLoadFailure( "centerpiece",
               "hardware definitions could not be applied", errors_to_cl,
               gui_needed, error_occured );
         }
      }
   }

   QString save_path;
   if ( flags.contains( "save" ) && flags[ "save" ].length() > 0 )
   {
      save_path            = flags[ "save" ];
      QDir dir( save_path );

      // Accept a new output directory as well as an existing one.
      if ( ! dir.exists()  &&  ! QDir().mkpath( save_path ) )
      {
         reportHeadlessLoadFailure( "save directory (could not create)",
                                     save_path, errors_to_cl,
                                     gui_needed, error_occured );
      }
      else
      {
         QFile file( dir.filePath( "tmp.txt" ) );
         if ( ! file.open( QIODevice::WriteOnly ) )
         {
            reportHeadlessLoadFailure( "save directory (not writable)",
                                        save_path, errors_to_cl,
                                        gui_needed, error_occured );
         }
         else
         {
            file.close();
            file.remove();
         }
      }
   }

   if ( ! error_occured && loaded_models && loaded_buffer && loaded_simparams && loaded_rotor )
   {
      if ( flags.contains( "start" ) )
      {
         start_sims();
         if ( ! save_path.isEmpty()  &&  ! save_sims_to( save_path ) )
         {
            if ( errors_to_cl )
               return 2;
            error_occured       = true;
            gui_needed          = true;
         }
      }
   }
   else
   {
      gui_needed             = true;
   }

   if ( error_occured )
      return 2;
   if ( gui_needed )
      return 1;
   return 0;
}

// Load models from explicit file paths and aggregate them as select_models()
// does, without displaying the selection dialog.
bool US_MwlSpeciesSim::load_models_from_paths( const QStringList& paths )
{
   // mdescs is deliberately not filled here. It belongs to US_ModelLoader,
   // which fills it with composite ";desc;filename;modelGUID;DB_id;editGUID"
   // strings (us_model_loader.cpp:description), not with bare descriptions.
   // Anything this path needs comes from models[].description directly, so
   // leaving the member empty keeps it meaning exactly one thing.
   models.clear();

   for ( const QString& path : paths )
   {
      US_Model m;
      if ( m.load( path ) != IUS_DB2::OK  ||  m.description.isEmpty() )
      {
         qDebug() << "Error loading model file" << path;
         return false;
      }
      models  << m;
   }

   nmodels      = models.count();
   if ( nmodels < 1 )
      return false;

   // Before the totals below are summed, so the clip and the noise sigmas
   // that read them see the scaled amplitudes.
   apply_extinction_scaling();

   mtconcs.fill( 0.0, nmodels );

   QStringList runids;
   QStringList chans;
   QStringList wavelns;

   for ( int jm = 0; jm < nmodels; jm++ )
   {
      QString mdesc  = models[ jm ].description;
      QString runid  = QString( mdesc ).section( ".",  0, -4 );
      QString triple = QString( mdesc ).section( ".", -3, -3 );
      QString chan   = QString( triple ).left( 2 );
      QString waveln = QString( triple ).mid( 2, 3 );

      if ( ! runids.contains( runid ) )
         runids  << runid;

      if ( ! chans .contains( chan ) )
         chans   << chan;

      if ( ! wavelns.contains( waveln ) )
         wavelns << waveln;

      double tot_conc = 0.0;
      for ( int jc = 0; jc < models[ jm ].components.count(); jc++ )
         tot_conc      += models[ jm ].components[ jc ].signal_concentration;

      mtconcs[ jm ]  = tot_conc;
   }

   int nruns      = runids .count();
   int nchans     = chans  .count();
   int nwavls     = wavelns.count();

   if ( nruns != 1  ||  nchans != 1  ||  nwavls != nmodels )
   {
      qDebug() << "Model descriptions must identify one run and one channel,"
                  " with one wavelength per model. Expected counts:"
               << "runs=" << 1 << "channels=" << 1 << "wavelengths=" << nmodels
               << "Actual counts: runs=" << nruns << "channels=" << nchans
               << "wavelengths=" << nwavls;
      return false;
   }

   mrunid         = runids[ 0 ];
   orunid         = "ISSF-" + mrunid + "-" + chans[ 0 ];
   QString triple = chans[ 0 ].left( 1 ) + "."
                  + chans[ 0 ].mid( 1, 1 ) + "."
                  + wavelns[ 0 ] + "-" + wavelns[ nwavls - 1 ];
   le_triples->setText( triple );
   le_runid  ->setText( orunid );

   pb_strtsims->setEnabled( true );

   return true;
}

// Save simulations to the requested directory instead of
// US_Settings::importDir().
bool US_MwlSpeciesSim::save_sims_to( const QString& save_dir )
{
   QString impdir     = save_dir + "/";
   // The generated model description owns the dataset triple.
   QString cell       = model_cell   ( models[ 0 ].description );
   QString channel    = model_channel( models[ 0 ].description );
   QString basefn     = orunid + "." + run_type + "." + cell + "." + channel
                      + ".xxx.auc";

   if ( ! QDir().mkpath( impdir ) )
   {
      qDebug() << "Error: could not create save directory" << impdir;
      return false;
   }

   for ( int jm = 0; jm < nmodels; jm++ )
   {
      QString mdesc      = models[ jm ].description;
      QString swavl      = mdesc.section( ".", -3, -3 ).mid( 2, 3 ) + ".auc";
      QString fname      = QString( basefn ).replace( "xxx.auc", swavl );
      QString fpath      = impdir + fname;
DbgLv(1) << " svsim: jm" << jm << "fname" << fname;

      te_status->setText( tr( "Saving data: %1" ).arg( swavl ) );
      qApp->processEvents();

      int stat           = US_DataIO::writeRawData( fpath, synData[ jm ] );
      if ( stat != US_DataIO::OK )
      {
         qDebug() << "Error: could not write" << fpath << "status" << stat;
         te_status->setText( tr( "Error writing %1" ).arg( fname ) );
         return false;
      }
   }

   QString smsga      = tr( "All %1 AUC files created\nand saved "
                            "to directory\n%2" ).arg( nmodels ).arg( impdir );
   te_status->setText( smsga );
   qApp->processEvents();

   QString tfname     = orunid + ".time_state.tmst";
   QString tfpath     = impdir + tfname;
DbgLv(1) << " svsim: sc0 time" << synData[0].scanData[0].seconds;

   // writeTimeState() reports failure by returning zero time points.
   if ( writeTimeState( tfpath, simparams, synData[ 0 ] ) == 0 )
   {
      qDebug() << "Error: could not write time state" << tfpath;
      return false;
   }

   smsga             += tr( "\n\nTime State file\n%1\nhas been written" )
                        .arg( tfname );
   te_status->setText( smsga );
   qApp->processEvents();

   // A database load needs the experiment and solution records too.
   if ( ! write_experiment_record( impdir, cell, channel ) )
   {
      qDebug() << "Error: could not write the experiment record for" << orunid;
      return false;
   }

   // Analysis programs require one edit file per wavelength.
   if ( ! write_edit_files( impdir, cell, channel ) )
      return false;

   return true;
}

// Write the experiment and solution records a database load needs.
//
// us_astfem_sim has exported these since 2025; this simulator never did, so a
// multi-wavelength archive carried scan data nothing could attribute to a
// sample. US_Experiment::readFromDisk had no <runID>.<type>.xml to rebuild
// triples from, and US_Solution::readFromDisk no record to hydrate chemistry
// from. Both documents come from the same utils writers us_astfem_sim calls,
// so what a multi-wavelength run exports is what a single-wavelength one does.
bool US_MwlSpeciesSim::write_experiment_record( const QString& impdir,
                                                const QString& cell,
                                                const QString& channel )
{
   if ( models.isEmpty()  ||  synData.isEmpty() )
      return false;

   // The record composition is shared with us_astfem_sim: same fields, same
   // fixed values, so what a multi-wavelength run exports is what a
   // single-wavelength one does.
   US_Experiment experiment = US_SimRecord::experiment( rotor, simparams,
                                                        orunid, run_type,
                                                        guid_seed );

   // One solution for the run. Every wavelength simulates the same species in
   // the same buffer, and each analyte collects its extinction at every
   // wavelength the run covers, which is what makes this a spectrum rather
   // than a set of unrelated samples.
   QList< double > wavelengths;

   for ( int jm = 0; jm < nmodels; jm++ )
      wavelengths << model_wavelength( models[ jm ].description ).toDouble();

   US_Solution sol = US_SimRecord::solution( models, wavelengths, buffer );

   // saveToDisk mints one only when this does not already hold a UUID, so
   // seeding it here needs no change to US_Solution.
   if ( ! guid_seed.isEmpty() )
      sol.solutionGUID = US_SimRecord::guid( guid_seed, "solution" );

   if ( sol.analyteInfo.isEmpty() )
      return false;

   sol.saveToDisk();

   // One dataset per wavelength, matching the .auc files written above.
   // saveToDisk splits cell, channel and wavelength back out of tripleDesc,
   // so these have to agree with the names save_sims_to built.
   QList< US_Convert::TripleInfo > triples;

   for ( int jm = 0; jm < nmodels; jm++ )
   {
      US_Convert::TripleInfo triple;
      triple.clear();
      triple.tripleID    = jm + 1;
      // Keep the experiment triple consistent with the AUC filename.
      triple.tripleDesc  = cell + " / " + channel + " / "
                           + model_wavelength( models[ jm ].description );
      triple.excluded    = false;
      triple.centerpiece = 1;
      triple.solution    = sol;
      memcpy( triple.tripleGUID, synData[ jm ].rawGUID,
              sizeof( triple.tripleGUID ) );

      triples << triple;
   }

   return experiment.saveToDisk( triples, run_type, orunid, impdir,
                                 simparams.speed_step ) == US_Convert::OK;
}

// Write an edit XML with stretched cell geometry beside each .auc file.
// The wavelength a per-wavelength model description names. The derived models
// a multi-wavelength run builds carry it in the third-from-last dot-separated
// field, two characters in, as in "...-MWL.e280.model.xml".
QString US_MwlSpeciesSim::model_wavelength( const QString& description )
{
   return description.section( ".", -3, -3 ).mid( 2, 3 );
}

// The generated triple field is <cell><channel><wavelength>, for example
// "1A250".
QString US_MwlSpeciesSim::model_cell( const QString& description )
{
   return description.section( ".", -3, -3 ).left( 1 );
}

QString US_MwlSpeciesSim::model_channel( const QString& description )
{
   return description.section( ".", -3, -3 ).mid( 1, 1 );
}

// On disk, each edit filename carries the plain wavelength of its dataset.
// Band and "@" forms are lookup keys assembled by consumers in memory.
QString US_MwlSpeciesSim::edit_wavelength_field( const QStringList& descriptions,
                                                 int index )
{
   if ( index < 0  ||  index >= descriptions.count() )
      return QString();

   return model_wavelength( descriptions[ index ] );
}

bool US_MwlSpeciesSim::write_edit_files( const QString& impdir,
                                          const QString& cell,
                                          const QString& channel )
{
   QString now        = edit_stamp.isEmpty()
                        ? QDateTime::currentDateTimeUtc().toString( "yyMMddhhmm" )
                        : edit_stamp;
   // Reuse the geometry init_rawdata() built the radial grid from; deriving
   // the stretch again here puts the edit radii outside the data range.
   double  meniscus   = curr_meniscus;
   double  bottom     = curr_bottom;

   if ( bottom <= meniscus )
   {
      QTextStream( stderr ) << "Error: cell geometry is unset; simulations "
         "must run before edit files can be written" << Qt::endl;
      return false;
   }

   // Taken from the models rather than from the mdescs member, which holds
   // US_ModelLoader's composite ";desc;filename;modelGUID;DB_id;editGUID"
   // strings and is empty on the headless path. model_wavelength() parses
   // dot-separated fields of a bare description, so it needs this form.
   QStringList model_descs;

   for ( int jm = 0; jm < nmodels; jm++ )
      model_descs << models[ jm ].description;

   for ( int jm = 0; jm < nmodels; jm++ )
   {
      QString swavl   = model_wavelength( model_descs[ jm ] );
      QString wfield  = edit_wavelength_field( model_descs, jm );
      // Field 2 of this name is what US_DataIO::loadData() uses as dataType,
      // so it must track the tag written into the .auc header.
      QString fname   = orunid + "." + now + "." + run_type + "." + cell
                      + "." + channel + "." + wfield + ".xml";
      US_DataIO::EditValues ev;
      ev.expType    = "Velocity";
      ev.runID      = orunid;
      ev.cell       = cell;
      ev.channel    = channel;
      ev.wavelength = swavl;
      ev.editGUID   = US_SimRecord::guid( guid_seed,
                                          "edit." + swavl );
      ev.dataGUID   = US_Util::uuid_unparse(
         reinterpret_cast<uchar*>( synData[ jm ].rawGUID ) );

      US_SimulationParameters::editRadiiFromCell( ev, meniscus, bottom );
      ev.ODlimit    = US_SimRecord::maxOD( synData[ jm ] );

      if ( US_DataIO::writeEdits( impdir + fname, ev ) != US_DataIO::OK )
      {
         qDebug() << "Error: could not write edit file" << impdir + fname;
         return false;
      }
   }

   return true;
}

// Bump the current plot to the previous channel
void US_MwlSpeciesSim::prev_plot( void )
{
DbgLv(1) << "SLOT: prev_plot";
   int ltripx      = nmodels - 1;
   tripx--;
   tripx           = ( tripx < 0 ) ? ltripx : tripx;

   while ( ! have_p1[ tripx ] )
   {
      tripx--;
      tripx           = ( tripx < 0 ) ? ltripx : tripx;
   }
DbgLv(1) << "PrevPlot: nmodels ltripx tripx" << nmodels << ltripx << tripx;

   plot_data1();
}

// Bump the current plot to the next channel
void US_MwlSpeciesSim::next_plot( void )
{
DbgLv(1) << "SLOT: next_plot";
   int ltripx      = nmodels - 1;
   tripx++;
   tripx           = ( tripx > ltripx ) ? 0 : tripx;

   while ( ! have_p1[ tripx ] )
   {
      tripx++;
      tripx           = ( tripx > ltripx ) ? 0 : tripx;
DbgLv(1) << "NextPlot:   tripx have" << tripx << have_p1[tripx];
   }
DbgLv(1) << "NextPlot: nmodels ltripx tripx" << nmodels << ltripx << tripx;

   plot_data1();
}

// Close the main window and stop the application
void US_MwlSpeciesSim::close_all( void )
{
DbgLv(1) << "SLOT: close_all";
   close();
}

// Set initial values for simulation parameters
void US_MwlSpeciesSim::init_simparams( void )
{
   US_SimulationParameters::SpeedProfile sp;
   QString rotor_calibr = "0";
   double rpm           = 45000.0; // Initialized rotor speed
   double low_delay     = rpm / ( 60.0 * 400.0 );  // Minimum delay

   // Set up bottom start and rotor coefficients from hardware file
   simparams.setHardware( NULL, rotor_calibr, 0, 0 );

   // Calculate bottom from rpm, channel bottom pos., rotor coefficients
   double bottom = US_AstfemMath::calc_bottom( rpm,
                      simparams.bottom_position, simparams.rotorcoeffs );
   bottom        = (double)( qRound( bottom * 1000.0 ) ) * 0.001;


   simparams.mesh_radius.clear();
   simparams.speed_step .clear();

   sp.duration_hours    = 2;        // Initialized experiment duration hours
   sp.duration_minutes  = 30.0;     // Initialized experiment duration minutes
   sp.delay_hours       = 0;        // Initialized accel time in hours
   sp.delay_minutes     = 20.0;     // Initialized accel time in minutes
   sp.rotorspeed        = (int)rpm; // Initialized rotor speed
   sp.avg_speed         = rpm;      // Initialized average speed
   sp.set_speed         = (int)rpm; // Initialized set speed
   sp.scans             = 100;      // Initialized number of scans
   sp.acceleration      = 400;      // Acceleration speed of the rotor
   sp.acceleration_flag = true;     // Flag used for acceleration zone
   sp.delay_minutes     = qMax( sp.delay_minutes, low_delay );  // Insure minimum

   simparams.speed_step << sp;

   simparams.simpoints         = 200;    // Initialized number of radial grid points
   simparams.radial_resolution = 0.001;  // Increment in radial experimental grid
   simparams.meshType          = US_SimulationParameters::ASTFEM;// Used for solver option
   simparams.gridType          = US_SimulationParameters::MOVING;// Used for grid option
   simparams.meniscus          = 5.8;    // Meniscus for simulation
   simparams.bottom            = bottom; // Bottom for simulation
   simparams.bottom_position   = 7.2;
   // Noise defaults to off. A --simparams file, or the simulation-parameters
   // dialog, may raise any of these; build_rawdata() applies whatever is set.
   simparams.rnoise            = 0.0;
   simparams.lrnoise           = 0.0;
   simparams.tinoise           = 0.0;
   simparams.rinoise           = 0.0;
   simparams.band_volume       = 0.015;
   simparams.rotorCalID        = rotor_calibr;
   simparams.band_forming      = false;

   set_parameters();
}

// Initialize the base raw data object
void US_MwlSpeciesSim::init_rawdata( void )
{
   QString guid       = US_SimRecord::guid( guid_seed, "raw.base" );
   QString mdesc      = models[ 0 ].description;
   QString triple     = QString( mdesc ).section( ".", -3, -3 );
   QString waveln     = QString( triple ).mid( 2, 3 );
   rdata0.xvalues .clear();
   rdata0.scanData.clear();
   rdata0.type[0]     = run_type.at( 0 ).toLatin1();
   rdata0.type[1]     = run_type.at( 1 ).toLatin1();
   rdata0.cell        = 1;
   rdata0.channel     = 'S';
   rdata0.description = "Simulation";
   US_Util::uuid_parse( guid, (uchar*)rdata0.rawGUID );
//*DEBUG*
simparams.debug();
//*DEBUG*

   // Update meniscus and bottom
   double speed0      = (double)simparams.speed_step[ 0 ].set_speed;
   double stretch     = speed0 * simparams.rotorcoeffs[ 0 ] +
                        sq( speed0 ) * simparams.rotorcoeffs[ 1 ];
   curr_meniscus      = simparams.meniscus + stretch;
   curr_bottom        = simparams.bottom_position + stretch;
   int nsteps         = simparams.speed_step.count();
   if ( nsteps > 1 )
   {
      qDebug() << "*** NSTEPS=" << nsteps << "--Only single speed reliable!";
   }

   // Number of radial grid points on the experimental grid
   int npoints        = qRound( ( curr_bottom - curr_meniscus ) /
                          simparams.radial_resolution ) + 1;
   double radinc      = simparams.radial_resolution;
   double radval      = curr_meniscus;
   int lptx           = npoints - 1;

   // Sets the size of radial grid points array
   rdata0.xvalues.resize( npoints );

   for ( int ii = 0; ii < lptx; ii++ )
   {  // Calculate the radial grid points
      rdata0.xvalues[ ii ] = radval;
      radval              += radinc;
   }

   rdata0.xvalues[ lptx ] = curr_bottom;

   // Set the total size of scans for simulation
   int nscans         = simparams.speed_step[ 0 ].scans;
   rdata0.scanData.resize( nscans );

   int terpsize       = ( npoints + 7 ) / 8;
   double mwavelen    = waveln.toDouble();
   double timeval     = simparams.speed_step[ 0 ].time_first;
   double timeinc     = ( nscans > 1 )
      ? ( simparams.speed_step[ 0 ].time_last
        - simparams.speed_step[ 0 ].time_first ) / (double)( nscans - 1 )
      : 0.0;
   double w2tval      = simparams.speed_step[ 0 ].w2t_first;
   double w2tinc      = timeinc * pow( simparams.speed_step[ 0 ].rotorspeed * M_PI / 30.0, 2.0 );
DbgLv(1) << "rdata0 tf tl" << simparams.speed_step[0].time_first << simparams.speed_step[0].time_last
 << "w2f w2l" << simparams.speed_step[0].w2t_first << simparams.speed_step[0].w2t_last
 << "timeinc w2tinc" << timeinc << w2tinc;

   // For each scan, set the information for the experimental grid.
   for ( int js = 0; js < nscans; js++ )
   {
      US_DataIO::Scan* scan = &rdata0.scanData[ js ];
      scan->temperature = simparams.temperature;
      scan->rpm         = speed0;
      scan->seconds     = timeval;
      scan->omega2t     = w2tval;
      scan->wavelength  = mwavelen;
      scan->plateau     = 0.0;
      scan->delta_r     = simparams.radial_resolution;
      scan->nz_stddev   = false;
      scan->rvalues     .fill( 0.0, npoints  );
      scan->interpolated.fill( 0,   terpsize );
      timeval          += timeinc;
      w2tval           += w2tinc;
if(js==0  || js==(nscans-1))
 DbgLv(1) << "rdata0 sc seconds" << scan->seconds << "omg2t" << scan->omega2t << "js" << js;
   }
}

// Add radially invariant noise: one offset per scan, applied to every point
// of that scan. The offsets are shared across wavelengths -- a lamp or
// detector fluctuation during scan k moves every wavelength read in that
// scan, so drawing them per wavelength would model N unrelated instruments.
void US_MwlSpeciesSim::add_ri_noise( US_DataIO::RawData& rdata,
                                     double total_conc )
{
   if ( simparams.rinoise == 0.0 ) return;

   int scans = rdata.scanData.size();

   if ( shared_ri.size() != scans )
   {  // First wavelength of the run draws it; the rest reuse it. Every
      // wavelength has the same scan geometry, so the size check only ever
      // fires once.
      shared_ri.resize( scans );

      for ( int ks = 0; ks < scans; ks++ )
         shared_ri[ ks ] = US_Math2::box_muller( 0.0, 1.0 );
   }

   double sigma = total_conc * simparams.rinoise / 100;

   for ( int ks = 0; ks < scans; ks++ )
   {
      double rinoise = shared_ri[ ks ] * sigma;

      for ( int mp = 0; mp < rdata.pointCount(); mp++ )
         rdata.scanData[ ks ].rvalues[ mp ] += rinoise;
   }
}

// Add a constant baseline offset.
void US_MwlSpeciesSim::add_baseline( US_DataIO::RawData& rdata )
{
   if ( simparams.baseline == 0.0 ) return;

   for ( int ks = 0; ks < rdata.scanData.size(); ks++ )
      for ( int mp = 0; mp < rdata.pointCount(); mp++ )
         rdata.scanData[ ks ].rvalues[ mp ] += simparams.baseline;
}

// Add per-point random noise. rnoise scales with the total concentration,
// lrnoise with the local value; both may be active at once.
void US_MwlSpeciesSim::add_random_noise( US_DataIO::RawData& rdata,
                                         double total_conc )
{
   if ( simparams.rnoise == 0.0  &&  simparams.lrnoise == 0.0 ) return;

   for ( int ks = 0; ks < rdata.scanData.size(); ks++ )
   {
      for ( int mp = 0; mp < rdata.pointCount(); mp++ )
      {
         double value = rdata.scanData[ ks ].rvalues[ mp ];
         double noise = 0.0;

         if ( simparams.rnoise != 0.0 )
            noise += US_Math2::box_muller( 0,
                        total_conc * simparams.rnoise / 100 );

         if ( simparams.lrnoise != 0.0 )
            noise += US_Math2::box_muller( 0,
                        value * simparams.lrnoise / 100 );

         rdata.scanData[ ks ].rvalues[ mp ] = value + noise;
      }
   }
}

// Add time invariant noise: one vector over radius, added identically to
// every scan.
//
// Built as a random walk rather than as independent draws, so its spread
// grows with the point count instead of staying at sigma, and varies a lot
// from one run to the next. At simpoints=200 the end-to-end drift is of
// order sqrt(N) times sigma. Keep tinoise well below rnoise or it dominates
// the dataset.
//
// The walk is shared across wavelengths. TI noise is by definition the part
// that does not vary between scans, which means it comes from fixed features
// of the optical path -- a scratch or a speck sits at one radius, and light
// of every wavelength passing through that radius passes through it. Drawing
// a separate walk per wavelength would model N unrelated scratches, and in a
// global fit those partially cancel across datasets, handing the fit a noise
// reduction real data would never give.
void US_MwlSpeciesSim::add_ti_noise( US_DataIO::RawData& rdata,
                                     double total_conc )
{
   if ( simparams.tinoise == 0.0 ) return;

   int points = rdata.pointCount();

   if ( shared_ti.size() != points )
   {  // First wavelength of the run draws it; the rest reuse it. Every
      // wavelength has the same radial geometry, so the size check only ever
      // fires once. Drawn at unit sigma and scaled below, so the magnitude
      // of the noise does not change how many random numbers are consumed.
      shared_ti.resize( points );

      double val = US_Math2::box_muller( 0.0, 1.0 );

      for ( int mp = 0; mp < points; mp++ )
      {
         val += US_Math2::box_muller( 0.0, 1.0 );
         shared_ti[ mp ] = val;
      }
   }

   double sigma = total_conc * simparams.tinoise / 100;

   for ( int ks = 0; ks < rdata.scanData.size(); ks++ )
      for ( int mp = 0; mp < points; mp++ )
         rdata.scanData[ ks ].rvalues[ mp ] += shared_ti[ mp ] * sigma;
}

// Apply all four noise terms to one wavelength's data.
//
// What is shared between wavelengths and what is not:
//
//  - The systematic terms (TI, RI) are drawn once per run and reused, since
//    both come from the instrument rather than from the measurement -- see
//    the comments on add_ti_noise() and add_ri_noise().
//
//  - The random terms (rnoise, lrnoise) are drawn per point per wavelength.
//    That is measurement noise, and it genuinely is independent.
//
//  - Every sigma scales with this wavelength's own total concentration, so a
//    wavelength where the analyte absorbs weakly gets proportionally less
//    noise rather than the same absolute amount. This is a simplification:
//    a fixed optical defect attenuates a roughly constant fraction of the
//    transmitted light, which does not convert to a constant fraction of OD.
//
// Called after clipping and padding, so the meniscus spike and the zeroed
// pad below it carry noise too, as they would in real data.
void US_MwlSpeciesSim::apply_noise( US_DataIO::RawData& rdata,
                                    double total_conc )
{
   // Order matches us_astfem_sim's finish(): lrnoise reads the running
   // value, so anything applied before it feeds into its magnitude.
   add_ri_noise    ( rdata, total_conc );
   add_baseline    ( rdata );
   add_random_noise( rdata, total_conc );
   add_ti_noise    ( rdata, total_conc );
}

// Build the raw data for the current triple (model)
void US_MwlSpeciesSim::build_rawdata( void )
{
   const int npad     = 10;

   // Initialize working RawData with template
   US_DataIO::RawData rdata = rdata0;
   int nscan          = rdata.scanCount();
   int npoint         = rdata.pointCount();
//*DEBUG*
if(tripx<2) {
DbgLv(1) << "rdata: xvals count" << rdata.xvalues.count();
DbgLv(1) << "rdata: scandata count" << rdata.scanData.count();
DbgLv(1) << "rdata: nscan" << nscan;
DbgLv(1) << "rdata: npoints" << npoint;
DbgLv(1) << "rdata: xval0" << rdata.xvalues[0];
DbgLv(1) << "rdata: xvaln" << rdata.xvalues[npoint-1];
DbgLv(1) << "rdata: xindex(menisc)" << rdata.xindex(curr_meniscus);
DbgLv(1) << "rdata:  time sc0" << rdata.scanData[0].seconds;
DbgLv(1) << "rdata:  time scn" << rdata.scanData[nscan-1].seconds;
DbgLv(1) << "rdata:  w2t  sc0" << rdata.scanData[0].omega2t;
DbgLv(1) << "rdata:  w2t  scn" << rdata.scanData[nscan-1].omega2t;
DbgLv(1) << "rdata:  temp sc0" << rdata.scanData[0].temperature;
DbgLv(1) << "rdata:  rpm  sc0" << rdata.scanData[0].rpm;
DbgLv(1) << "rdata:  wvln sc0" << rdata.scanData[0].wavelength;
DbgLv(1) << "rdata:  plat sc0" << rdata.scanData[0].plateau;
DbgLv(1) << "rdata:  dltr sc0" << rdata.scanData[0].delta_r;
DbgLv(1) << "rdata:  temp sc1" << rdata.scanData[1].temperature;
DbgLv(1) << "rdata:  rpm  sc1" << rdata.scanData[1].rpm;
DbgLv(1) << "rdata:  wvln sc1" << rdata.scanData[1].wavelength;
DbgLv(1) << "rdata:  plat sc1" << rdata.scanData[1].plateau;
DbgLv(1) << "rdata:  dltr sc1" << rdata.scanData[1].delta_r;
}
//*DEBUG*

   // Get variables related to specific model
   QString mdesc      = models[ tripx ].description;
   QString triple     = QString( mdesc ).section( ".", -3, -3 );
   QString waveln     = QString( triple ).mid( 2, 3 );
   double mwavelen    = waveln.toDouble();
   rdata.cell         = triple.left( 1 ).toInt();
   rdata.channel      = triple.mid( 1, 1 ).at( 0 ).toLatin1();
   for ( int js = 0; js < nscan; js++ )
      rdata.scanData[ js ].wavelength = mwavelen;

DbgLv(1) << "bldraw:  tripx" << tripx;
   US_Astfem_RSA* astfem = new US_Astfem_RSA( models[ tripx ], simparams );

   astfem->set_debug_flag( dbg_level );
astfem->set_debug_flag( 0 );
   astfem->setTimeInterpolation( true  );
   astfem->setTimeCorrection   ( false );

   astfem->calculate( rdata );
int hs=nscan/2;
int hp=npoint/2;
DbgLv(1) << "bldraw:   hs,hp" << hs << hp << "data0m" << rdata.value(0,hp)
 << "datamm" << rdata.value(hs,hp);

   // Clip maximum value to 3 times total concentration of model
   double tot_conc    = mtconcs[ tripx ];
   double max_od      = tot_conc * 3.0;
DbgLv(1) << "bldraw:  " << tripx << "tot_conc max_od" << tot_conc << max_od;

   for ( int js = 0; js < nscan; js++ )
   {
      for ( int jr = 0; jr < npoint; jr++ )
      {
         if ( rdata.value( js, jr ) > max_od )
            rdata.setValue( js, jr, max_od );    // Limit value to max. OD
      }
DbgLv(1) << "bldraw:   js" << js << "valmm" << rdata.value(js,npoint/2);
   }

   // Pad start of data and set meniscus value spike
   double mspike      = tot_conc * 1.5;
   npoint             = rdata0.pointCount() + npad;
   rdata.xvalues.resize( npoint );
   double radv0       = rdata.xvalues[ 0 ];
   double radinc      = rdata.xvalues[ 1 ] - radv0;
   double radval      = radv0;

   for ( int js = 0; js < nscan; js++ )                // Resize for pad
   {
      rdata.scanData[ js ].rvalues.resize( npoint );

      // The interpolation bitmap describes the readings, so it has to grow
      // with them, exactly as in us_astfem_sim's save path. None of the
      // simulated points is interpolated, so every bit is zero -- which is
      // also why the pad shifting readings down by npad needs no bit shift.
      rdata.scanData[ js ].interpolated.fill( '\0', ( npoint + 7 ) / 8 );
   }

   for ( int jr = npoint - 1; jr >= 0; jr-- )
   {  // Set shifted values, starting at data end
      int kr             = jr - npad;                  // Old value index

      if ( kr > 0 )
      {  // Move old values down by pad amount
         rdata.xvalues[ jr ] = rdata.xvalues[ kr ];
         for ( int js = 0; js < nscan; js++ )
         {
            rdata.setValue( js, jr, rdata.value( js, kr ) );
         }
      }

      else if ( kr == 0 )
      {  // Set the meniscus spike value for each scan
         rdata.xvalues[ jr ] = radval;
         radval             -= radinc;
         for ( int js = 0; js < nscan; js++ )
         {
            rdata.setValue( js, jr, mspike );
         }
      }

      else
      {  // Set values to zero that are below the meniscus
         rdata.xvalues[ jr ] = radval;
         radval             -= radinc;
         for ( int js = 0; js < nscan; js++ )
         {
            rdata.setValue( js, jr, 0.0 );
         }
      }
   }

   // Add noise last, so the pad and the meniscus spike are noisy too
   apply_noise( rdata, tot_conc );

   // Save the simulated data and plot it
   QString guid       = US_SimRecord::guid( guid_seed,
                           QString( "raw.%1" ).arg( synData.count() ) );
   US_Util::uuid_parse( guid, (uchar*)rdata.rawGUID );

   synData << rdata;
   have_p1 << true;

   plot_data1();
}


// Write a pair of TimeState files
int US_MwlSpeciesSim::writeTimeState( const QString&           tmst_fpath,
                                      US_SimulationParameters& simparams,
                                      US_DataIO::RawData&      sim_data )
{
#if 0     // New writetimestate function is in AstfemMath
   simparams.sim      = true;
   US_AstfemMath::writeTimeState( tmst_fpath, simparams, sim_data );
#endif
#if 1     // New writetimestate function is NOT in AstfemMath (duplicate here)
   const bool spsim     = true;
   US_TimeState timestate;
   int nspeed           = simparams.speed_step.size();

   if ( timestate.open_write_data( tmst_fpath, 1.0, 0.0 ) != 0 )
   {
DbgLv(1) << "AMATH: wrts: Unable to open" << tmst_fpath;
      return 0;
   }

   timestate.set_key( "Time",        "I4" );
   timestate.set_key( "RawSpeed",    "F4" );
   timestate.set_key( "SetSpeed",    "I4" );
   timestate.set_key( "Omega2T",     "F4" );
   timestate.set_key( "Temperature", "F4" );
   timestate.set_key( "Step",        "I2" );
   timestate.set_key( "Scan",        "I2" );

   double duration      = 0.0;
   double duration_prev = 0.0;
   double rpm           = 0.0;
   double omega2t       = 0.0;
   double prvs_speed    = 0.0 ;
   int    scan_nbr      = 0;
   double temperature   = sim_data.scanData[ 0 ].temperature;
   int nscans           = sim_data.scanData.size() ;// Used for number of scans
   int t_acc;    // Used for time when accelerated up to the specified rotor speed
   double rate, speed;
   US_SimulationParameters::SpeedProfile* sp;
   US_SimulationParameters::SpeedProfile* sp_prev;
   QList< int > scantimes;

   for ( int ii = 0; ii < nscans; ii++ )
   {  // Accumulate the times at scans
      scantimes << sim_data.scanData[ ii ].seconds;
   }
//DbgLv(1) << " writetimestate : no of scans" << nscans ;

   //if ( simparams.sim == false )
   if ( spsim == false )
   {
      double d   = -2.0/3.0;

      double t2  = simparams.speed_step[0].time_first;
      double w2t = simparams.speed_step[0].w2t_first;
      double c   = pow((simparams.speed_step[0].rotorspeed*M_PI/30.0),2.0);
      double t1  = ( w2t - ( t2 * c ) ) / ( d * c );
      t_acc      = (int)qRound( t1 );
      rate       = (double)( simparams.speed_step[ 0 ].rotorspeed ) / (double)t_acc;
   }

   else
   {
      sp         = &simparams.speed_step[ 0 ];
      rate       = (double) sp->acceleration ;
      t_acc      = (int)qRound( sp->rotorspeed / rate );
      rate       = (double)sp->rotorspeed / (double)t_acc;
//DbgLv(1)<< " rate is given by user : t_acc from timestate" << t_acc << rate;
   }

   int d1     = 0;
   int itime  = 0;

   for ( int stepx = 0; stepx < nspeed; stepx++ )
   {
      sp         = &simparams.speed_step[ stepx ];
      speed      = (double) (sp->rotorspeed) ;

      if ( stepx > 0 )
      {
         sp_prev      = &simparams.speed_step[ stepx - 1 ];
         prvs_speed   = (double) (sp_prev->rotorspeed) ;
      }
      else
         prvs_speed   = 0.0 ;

      if ( stepx == 0 )
      {
         d1           = 0 ;
         rpm         -= rate ;
      }
      else
      {
         duration_prev = duration;

         d1          = (int)duration_prev + 1;

         //if ( simparams.sim == false )
         if ( spsim == false )
            // Use calculated rate when user doesn't know it
            t_acc = ( int)qRound( double( qAbs( speed - prvs_speed ) ) / rate );
         else
            // When user knows acceleration rate
            t_acc = ( int)qRound( double( qAbs( speed - prvs_speed  ) ) / sp->acceleration );

         rate        = (double)( speed - prvs_speed ) / (double)t_acc;
      }

      duration    += ( sp->duration_hours * 3600.0 )
                   + ( sp->duration_minutes * 60.0 );

      for ( int ii = d1; ii <= int(duration); ii++ )
      {
         int tacc = d1 + t_acc  ;

         if ( ii < tacc )
            rpm     += rate ;
         else
            rpm     =  speed ;

         int    set_speed  = qRound( rpm / 100.0 ) * 100;

         omega2t  += pow((rpm*M_PI/30.0),2.0);

         itime       = ii;
         int step    = stepx + 1;
         int scanx   = scantimes.indexOf( itime );
         scan_nbr    = ( scanx < 0 ) ? 0 : ( scanx + 1 );
if(scan_nbr>0)
DbgLv(1) << "wrTS:   scan_nbr" << scan_nbr << "itime" << itime;

         timestate.set_value( "Time",        itime       );
         timestate.set_value( "RawSpeed",    rpm         );
         timestate.set_value( "SetSpeed",    set_speed   );
         timestate.set_value( "Omega2T",     omega2t     );
         timestate.set_value( "Temperature", temperature );
         timestate.set_value( "Step",        step        );
         timestate.set_value( "Scan",        scan_nbr    );
         timestate.flush_record();
      }
   }

   // Pad out TimeState for some seconds beyond last scan time
   duration      += 10;
   d1             = itime + 1;
   int step       = nspeed;
   int set_speed  = qRound( rpm / 100.0 ) * 100;
   scan_nbr       = 0;
   double omg2ti  = pow( ( rpm * M_PI / 30.0 ), 2.0 );

   for ( int ii = d1; ii <= int(duration); ii++ )
   {
      omega2t       += omg2ti;
      itime          = ii ;

      timestate.set_value( "Time",        itime       );
      timestate.set_value( "RawSpeed",    rpm         );
      timestate.set_value( "SetSpeed",    set_speed   );
      timestate.set_value( "Omega2T",     omega2t     );
      timestate.set_value( "Temperature", temperature );
      timestate.set_value( "Step",        step        );
      timestate.set_value( "Scan",        scan_nbr    );

      timestate.flush_record();
   }

   timestate.close_write_data();
   timestate.setImportType( US_TimeState::IMPORT_TYPE::MWRS );
   timestate.setTimeStateType( US_TimeState::TIMESTATE_TYPE::CALCULATED );
   timestate.write_defs( 1.0 );

   return timestate.time_count();
#endif
}
