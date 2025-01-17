//! \file us_mwl_species_sim.cpp

#include <QApplication>

#include "us_mwl_species_sim.h"
#include "us_data_loader.h"
#include "us_select_runs.h"
#include "us_astfem_rsa.h"
#include "us_astfem_math.h"
#include "us_license_t.h"
#include "us_license.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_util.h"
#include "us_passwd.h"
#include "us_images.h"

#if QT_VERSION < 0x050000
#define setSamples(a,b,c)  setData(a,b,c)
#define setMinimum(a)      setMinValue(a)
#define setMaximum(a)      setMaxValue(a)
#define setSymbol(a)       setSymbol(*a)
#define dPlotClearAll(a) a->clear()
#else
#include "qwt_picker_machine.h"
#define dPlotClearAll(a) a->detachItems(QwtPlotItem::Rtti_PlotItem,true)
#endif

//! \brief Main program. Loads translators and starts
//         the class US_Convert.
int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );

   #include "main1.inc"

   // License is OK.  Start up.
   
   US_MwlSpeciesSim w;
   w.show();                   //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication
}

//! \brief Constructor for US_MwlSpeciesSim class
US_MwlSpeciesSim::US_MwlSpeciesSim() : US_Widgets()
{
   setWindowTitle( tr( "MWL Pre-Fit Species Simulation" ) );
   setPalette( US_GuiSettings::frameColor() );

   dbg_level   = US_Settings::us_debug();
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
   QString cell   = QString().sprintf( "%d", rdata->cell );
   QString wavl   = QString().sprintf( "%d",
                       (int)rdata->scanData[ 0 ].wavelength );
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
   double scn_rng      = (double)( simparams.speed_step[ 0 ].scans - 1 );
   delay               = qRound( delay );
   duration            = qRound( duration );
   double pi_fac       = sq( M_PI / 30.0 );
   double tim_rng      = duration - delay;
   double tim_inc      = tim_rng / scn_rng;
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

   while( tim_val < duration )
   {  // Walk time and omega2t up to the last scan
      tim_val            += tim_inc;
      w2t_val            += w2t_inc;
   }

   simparams.speed_step[ 0 ].time_last   = duration;
   simparams.speed_step[ 0 ].w2t_last    = w2t_val;
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
   QString impdir     = US_Settings::importDir() + "/" + orunid + "/";
   QString cell       = QString( orunid ).section( "-", -1, -1 ).left( 1 );
   QString basefn     = orunid + ".RA." + cell + ".S.xxx.auc";

   QDir().mkpath( impdir );

   for ( int jm = 0; jm < nmodels; jm++ )
   {
      QString mdesc      = models[ jm ].description;
      QString swavl      = mdesc.section( ".", -3, -3 ).mid( 2, 3 ) + ".auc";
      QString fname      = QString( basefn ).replace( "xxx.auc", swavl );
      QString fpath      = impdir + fname;
DbgLv(1) << " svsim: jm" << jm << "fname" << fname;

      QString smsg       = tr( "Saving data: %1" ).arg( swavl );
      te_status->setText( smsg );
      qApp->processEvents();

      US_DataIO::writeRawData( fpath, synData[ jm ] );
   }

   QString smsga      = tr( "All %1 AUC files created\nand saved "
                            "to directory\n%2" ).arg( nmodels ).arg( impdir );
   te_status->setText( smsga );
   qApp->processEvents();

   // Build and save time state
   QString tfname     = orunid + ".time_state.tmst";
   QString tfpath     = impdir + tfname;
DbgLv(1) << " svsim: sc0 time" << synData[0].scanData[0].seconds;

   writeTimeState( tfpath, simparams, synData[ 0 ] );

   smsga             += tr( "\n\nTime State file\n%1\nhas been written" )
                        .arg( tfname );
   te_status->setText( smsga );
   qApp->processEvents();
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
   QString guid       = US_Util::new_guid();
   QString mdesc      = models[ 0 ].description;
   QString triple     = QString( mdesc ).section( ".", -3, -3 );
   QString waveln     = QString( triple ).mid( 2, 3 );
   rdata0.xvalues .clear();
   rdata0.scanData.clear();
   rdata0.type[0]     = 'R';
   rdata0.type[1]     = 'A';
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
   double timeinc     = ( simparams.speed_step[ 0 ].time_last
                        - simparams.speed_step[ 0 ].time_first )
                        / (double)( nscans - 1 );
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
      rdata.scanData[ js ].rvalues.resize( npoint );

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

   // Save the simulated data and plot it
   QString guid       = US_Util::new_guid();
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
   timestate.write_defs( 1.0 );

   return timestate.time_count();
#endif
}

