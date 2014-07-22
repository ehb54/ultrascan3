//! \file us_astfem_sim.cpp

#include <QApplication>

#include "us_license.h"
#include "us_license_t.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_astfem_sim.h"
#include "us_simulationparameters.h"
#include "us_math2.h"
#include "us_defines.h"
#include "us_clipdata.h"
#include "us_gui_util.h"
#include "us_model_gui.h"
#include "us_buffer_gui.h"
#include "us_util.h"
#include "us_lamm_astfvm.h"

/*! \brief Main program for US_Astfem_Sim.  Loads translators and starts
    the class US_Astfem_Sim.
*/
int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );
  
   #include "main1.inc"

   // License is OK.  Start up.
   US_Astfem_Sim w;
   w.show();                   //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication
}

US_Astfem_Sim::US_Astfem_Sim( QWidget* p, Qt::WindowFlags f ) 
   : US_Widgets( true, p, f )
{
   dbg_level           = US_Settings::us_debug();

   setWindowTitle( "UltraScan3 Simulation Module" );
   setPalette( US_GuiSettings::frameColor() );
   init_simparams();

   stopFlag            = false;
   movieFlag           = false;
   time_correctionFlag = false;

   astfem_rsa          = NULL;
   astfvm              = NULL;

   clean_etc_dir();

   QGridLayout* main = new QGridLayout( this );
   main->setSpacing( 2 );
   main->setContentsMargins ( 2, 2, 2, 2 );
   main->setColumnStretch( 1, 1 );
   
   QBoxLayout* buttonbox = new QVBoxLayout;

   QPushButton* pb_new = us_pushbutton( tr( "Model Control") );
   connect( pb_new, SIGNAL( clicked() ), SLOT( new_model() ) );
   buttonbox->addWidget( pb_new );

   pb_buffer   = us_pushbutton( tr( "Define Buffer"), false );
   connect( pb_buffer, SIGNAL( clicked() ), SLOT( new_buffer() ) );
   buttonbox->addWidget( pb_buffer );

   pb_simParms = us_pushbutton( tr( "Simulation Parameters"), false );
   connect ( pb_simParms, SIGNAL( clicked() ), SLOT( sim_parameters() ) );
   buttonbox->addWidget( pb_simParms );

   QGridLayout* movie = us_checkbox( "Show Movie", ck_movie, movieFlag );
   buttonbox->addLayout( movie );

   QGridLayout* lo_svmovie = us_checkbox( "Save Movie", ck_savemovie, false );
   connect( ck_savemovie, SIGNAL( toggled          ( bool ) ),
                          SLOT(   update_save_movie( bool ) ) );
   buttonbox->addLayout( lo_svmovie );

   QGridLayout* timeCorr = us_checkbox( "Use Time Correction", ck_timeCorr,
         time_correctionFlag );
   
   connect( ck_timeCorr, SIGNAL( clicked() ), SLOT( update_time_corr() ) );
   buttonbox->addLayout( timeCorr );

   pb_start = us_pushbutton( tr( "Start Simulation" ), false );
   connect( pb_start, SIGNAL( clicked() ), SLOT( start_simulation() ) );
   buttonbox->addWidget( pb_start );

   pb_stop = us_pushbutton( tr( "Stop Simulation" ), false );
   connect( pb_stop, SIGNAL( clicked() ), SLOT( stop_simulation() ) );
   buttonbox->addWidget( pb_stop );

   pb_saveSim = us_pushbutton( tr( "Save Simulation" ), false );
   connect( pb_saveSim, SIGNAL( clicked() ), SLOT( save_scans() ) );
   buttonbox->addWidget( pb_saveSim );

   QPushButton* pb_help = us_pushbutton( tr( "Help" ) );
   connect( pb_help, SIGNAL( clicked() ), SLOT( help()) );
   buttonbox->addWidget( pb_help );

   QPushButton* pb_close = us_pushbutton( tr( "Close" ) );
   buttonbox->addWidget( pb_close );
   connect( pb_close, SIGNAL( clicked() ), SLOT( close()) );

   QPalette pa( pb_close->palette() );
   te_status   = us_textedit();
   te_status->setPalette( pa );
   te_status->setTextBackgroundColor( pa.color( QPalette::Window ) );
   te_status->setTextColor(           pa.color( QPalette::WindowText ) );
   QFontMetrics fm( te_status->font() );
   te_status->setMaximumHeight( fm.lineSpacing() * 17 / 2 );

   change_status();

   buttonbox->addWidget( te_status );
   buttonbox->addStretch();

   main->addLayout( buttonbox, 0, 0 );

   // Right Column
   QBoxLayout* plot = new QVBoxLayout;

   // Simulation Plot
   plot1 = new US_Plot( moviePlot, tr( "Simulation Window" ), 
         tr( "Radius (cm)" ), tr( "Concentration" ) );
   us_grid  ( moviePlot );
   moviePlot->setMinimumSize( 600, 275);
   moviePlot->setAxisScale( QwtPlot::yLeft, 0.0, 2.0 );
   moviePlot->setAxisScale( QwtPlot::xBottom, 5.8, 7.2 );

   plot->addLayout( plot1 );

   QBoxLayout* timeSpeed = new QHBoxLayout;

   QLabel* lb_time = us_label( tr( "Time( in seconds):" ) );
   lb_time->setAlignment ( Qt::AlignCenter );
   timeSpeed->addWidget( lb_time );

   lcd_time = us_lcd( 5, 0 );
   timeSpeed->addWidget( lcd_time );
   
   QLabel* lb_speed = us_label( tr( "Current Speed:" ) );
   lb_speed->setAlignment ( Qt::AlignCenter );
   timeSpeed->addWidget( lb_speed );

   lcd_speed = us_lcd( 5, 0 );
   timeSpeed->addWidget( lcd_speed );
   
   plot->addLayout( timeSpeed );

   // Saved Scans
   plot2 = new US_Plot( scanPlot, tr( "Saved Scans" ), 
         tr( "Radius (cm)" ), tr( "Concentration" ) );
   QwtPlotGrid* grid2 = us_grid  ( scanPlot );
   grid2->enableX(    true );
   grid2->enableY(    true );
   scanPlot->setMinimumSize( 600, 275);
   scanPlot->setAxisScale( QwtPlot::yLeft,   0.0, 2.0 );
   scanPlot->setAxisScale( QwtPlot::xBottom, 5.8, 7.2 );
   plot->addLayout( plot2 );

   QBoxLayout* completion = new QHBoxLayout;

   lb_component  = us_label( tr( "Component:" ) );
   lb_component->setAlignment ( Qt::AlignCenter );
   completion->addWidget( lb_component );

   lcd_component = us_lcd( 7, 0 );
   completion->addWidget( lcd_component );

   lb_progress   = us_label( tr( "% Completed:" ) );
   lb_progress->setAlignment ( Qt::AlignCenter );
   completion->addWidget( lb_progress );

   progress = us_progressBar( 0, 100, 0 );
   completion->addWidget( progress );

   plot->addLayout( completion );

   main->addLayout( plot, 0, 1 );
}

void US_Astfem_Sim::init_simparams( void )
{
   US_SimulationParameters::SpeedProfile sp;
   QString rotor_calibr = "0";
   double  rpm          = 45000.0;

   // set up bottom start and rotor coefficients from hardware file
   simparams.setHardware( NULL, rotor_calibr, 0, 0 );

   // calculate bottom from rpm, channel bottom pos., rotor coefficients
   double bottom = US_AstfemMath::calc_bottom( rpm,
         simparams.bottom_position, simparams.rotorcoeffs );
   bottom        = (double)( qRound( bottom * 1000.0 ) ) * 0.001;
qDebug() << "ASim:InSP: rotor_calibr" << rotor_calibr;
qDebug() << "ASim:InSP:  rpm" << rpm;
qDebug() << "ASim:InSP:  bottom_position" << simparams.bottom_position;
qDebug() << "ASim:InSP:  rotorcoeffs" << simparams.rotorcoeffs[0] << simparams.rotorcoeffs[1];
qDebug() << "ASim:InSP:  bottom" << bottom << simparams.bottom;

   simparams.mesh_radius.clear();
   simparams.speed_step .clear();

   sp.duration_hours    = 5;
   sp.duration_minutes  = 30.0;
   sp.delay_hours       = 0;
   sp.delay_minutes     = 20.0;
   sp.rotorspeed        = (int)rpm;
   sp.scans             = 30;
   sp.acceleration      = 400;
   sp.acceleration_flag = true;

   simparams.speed_step << sp;

   simparams.simpoints         = 200;
   simparams.radial_resolution = 0.001;
   simparams.meshType          = US_SimulationParameters::ASTFEM;
   simparams.gridType          = US_SimulationParameters::MOVING;
   simparams.meniscus          = 5.8;
   simparams.bottom            = bottom;
   simparams.rnoise            = 0.0;
   simparams.tinoise           = 0.0;
   simparams.rinoise           = 0.0;
   simparams.band_volume       = 0.015;
   simparams.rotorCalID        = rotor_calibr;
   simparams.band_forming      = false;
}

void US_Astfem_Sim::new_model( void )
{
   US_ModelGui* dialog = new US_ModelGui( system );
   connect( dialog, SIGNAL( valueChanged( US_Model ) ), 
                    SLOT  ( change_model( US_Model ) ) );
   dialog->exec();
}

void US_Astfem_Sim::change_model( US_Model m )
{
   system = m;
   pb_buffer  ->setEnabled( true );
   pb_simParms->setEnabled( true );

   // set default of FVM if model is non-ideal
   if ( system.components[ 0 ].sigma != 0.0  ||
        system.components[ 0 ].delta != 0.0  ||
        system.coSedSolute           >= 0    ||
        buffer.compressibility       >  0.0 )
      simparams.meshType = US_SimulationParameters::ASTFVM;

   else  // normal (ideal) default
      simparams.meshType = US_SimulationParameters::ASTFEM;

   change_status();
}

void US_Astfem_Sim::new_buffer( void )
{
   US_BufferGui* dialog = new US_BufferGui( true, buffer );

   connect( dialog, SIGNAL( valueChanged( US_Buffer ) ), 
                    SLOT  ( change_buffer( US_Buffer ) ) );

   dialog->exec();
   qApp->processEvents();
}

void US_Astfem_Sim::change_buffer( US_Buffer b )
{
   buffer = b;

   if ( buffer.compressibility  >  0.0 )
      simparams.meshType = US_SimulationParameters::ASTFVM;

   change_status();
}

void US_Astfem_Sim::change_status()
{
   QStringList mtyps;
   mtyps << "ASTFEM" << "CLAVERIE" << "MOVING_HAT" << "USER" << "ASTFVM";
   QString simtype = mtyps[ (int)simparams.meshType ];

   int    dhrs  = simparams.speed_step[ 0 ].duration_hours;
   double dmns  = simparams.speed_step[ 0 ].duration_minutes;
   int    scns  = simparams.speed_step[ 0 ].scans;

   for ( int ii = 1; ii < simparams.speed_step.size(); ii++ )
   {
      dhrs     += simparams.speed_step[ ii ].duration_hours;
      dmns     += simparams.speed_step[ ii ].duration_minutes;
      scns     += simparams.speed_step[ ii ].scans;
   }

   if ( dmns > 59 )
   {
      int khrs = dmns / 60;
      dmns    -= ( khrs * 60.0 );
      dhrs    += khrs;
   }

   te_status->setText( tr(
      "Model:\n  %1\n"
      "Buffer (density/viscosity/compress.):\n  %2 / %3 / %4\n"
      "SimParams (type/duration/scans):\n  %5 / %6 h %7 m / %8" )
      .arg( system.description ).arg( buffer.density ).arg( buffer.viscosity )
      .arg( buffer.compressibility ).arg( simtype )
      .arg( simparams.speed_step[ 0 ].duration_hours )
      .arg( simparams.speed_step[ 0 ].duration_minutes )
      .arg( simparams.speed_step[ 0 ].scans ) );
}

void US_Astfem_Sim::sim_parameters( void )
{
   working_simparams = simparams;

   US_SimulationParametersGui* dialog = 
      new US_SimulationParametersGui( working_simparams );
   
   connect( dialog, SIGNAL( complete() ), SLOT( set_parameters() ) );

   dialog->exec();
}

void US_Astfem_Sim::set_parameters( void )
{
   simparams = working_simparams;

   pb_start  ->setEnabled( true );

   change_status();
}

void US_Astfem_Sim::stop_simulation( void )
{
   stopFlag  = true;

   if ( astfem_rsa )
      astfem_rsa->setStopFlag( stopFlag );

   if ( astfvm )
      astfvm    ->setStopFlag( stopFlag );
}

void US_Astfem_Sim::start_simulation( void )
{

   moviePlot->clear();
   moviePlot->replot();
   curve_count = 0;
   image_count = 0;
   double rpm  = simparams.speed_step[ 0 ].rotorspeed;

   scanPlot->clear();
	scanPlot->setAxisAutoScale( QwtPlot::xBottom );
   scanPlot->replot();

   pb_stop   ->setEnabled( true  );
   pb_start  ->setEnabled( false );
   pb_saveSim->setEnabled( false );

   // The astfem/astfvm simulation routines expects a dataset structure that
   // is initialized with a time and radius grid, and all concentration points
   // need to be set to zero. Each speed is a separate mfem_data set.
   sim_data.xvalues .clear();
   sim_data.scanData.clear();

   sim_data.type[0]    = 'R';
   sim_data.type[1]    = 'A';

   QString guid = US_Util::new_guid();
   US_Util::uuid_parse( guid, (uchar*)sim_data.rawGUID );
   
   sim_data.cell        = 1;
   sim_data.channel     = 'S';
   sim_data.description = "Simulation";
   
   int points = qRound( ( simparams.bottom - simparams.meniscus ) / 
                          simparams.radial_resolution ) + 1;

   sim_data.xvalues.resize( points );

   for ( int i = 0; i < points; i++ ) 
      sim_data.xvalues[ i ] = simparams.meniscus
                              + i * simparams.radial_resolution;

   int total_scans = 0;

   for ( int i = 0; i < simparams.speed_step.size(); i++ ) 
      total_scans += simparams.speed_step[ i ].scans;

   sim_data.scanData.resize( total_scans );
   int terpsize    = ( points + 7 ) / 8;

   for ( int i = 0; i < total_scans; i++ ) 
   {
      US_DataIO::Scan* scan = &sim_data.scanData[ i ];

      scan->temperature = simparams.temperature;
      scan->rpm         = rpm;
      scan->omega2t     = 0.0;
      scan->wavelength  = system.wavelength;
      scan->plateau     = 0.0;
      scan->delta_r     = simparams.radial_resolution;

      scan->rvalues     .fill( 0.0, points   );
      scan->interpolated.fill( 0,   terpsize );
   }

   // Set the time for the scans
   double w2t_sum      = 0.0;
   double delay        = simparams.speed_step[ 0 ].delay_hours * 3600.0
                       + simparams.speed_step[ 0 ].delay_minutes * 60.0;
   double current_time = delay;
//   double current_time = 0.0;
   double duration;
   double increment    = 0.0;
   int    scan_number  = 0;

   for ( int ii = 0; ii < simparams.speed_step.size(); ii++ )
   {
      // To get the right time, we need to take the last scan time from the
      // previous speed step and add the delays to it

      // First time point of this speed step in secs
      US_SimulationParameters::SpeedProfile* sp = &simparams.speed_step[ ii ];
      double w2t     = sq( sp->rotorspeed * M_PI / 30.0 );

      delay          = sp->delay_hours    * 3600. + sp->delay_minutes    * 60.;
      duration       = sp->duration_hours * 3600. + sp->duration_minutes * 60.;
//      increment      = ( duration - delay ) / (double)( sp->scans - 1 );
      increment      = duration / (double)( sp->scans - 1 );
      if ( ii == 0 )
         w2t_sum        = current_time * w2t;
      double w2t_inc = increment * w2t;
//      current_time  += ( delay - increment );
DbgLv(2) << "SIM curtime dur incr" << current_time << duration << increment;

      for ( int jj = 0; jj < sp->scans; jj++ )
      {
         US_DataIO::Scan* scan = &sim_data.scanData[ scan_number ];
//         current_time += increment;
         scan->seconds = (double)qRound( current_time );
         scan->omega2t = w2t_sum;
         w2t_sum      += w2t_inc;
         current_time += increment;

         scan_number++;
DbgLv(2) << "SIM   scan time" << scan_number << scan->seconds;
      }

      int j1           = scan_number - sp->scans;
      int j2           = scan_number - 1;
      sp->w2t_first    = sim_data.scanData[ j1 ].omega2t;
      sp->w2t_last     = sim_data.scanData[ j2 ].omega2t;
      sp->time_first   = sim_data.scanData[ j1 ].seconds;
      sp->time_last    = sim_data.scanData[ j2 ].seconds;
   }

   lb_progress->setText( tr( "% Completed:" ) );
   progress->setMaximum( system.components.size() ); 
   progress->reset();
   lcd_component->display( 0 );

   stopFlag  = false;

   simparams.mesh_radius.clear();
   simparams.firstScanIsConcentration = false;

   // Run the simulation

   if ( simparams.meshType != US_SimulationParameters::ASTFVM )
   {  // the normal case:  ASTFEM (finite element)

      if ( system.associations.size() > 0 )
         lb_component->setText( tr( "RA Step:"   ) );
      else
         lb_component->setText( tr( "Component:" ) );

      astfem_rsa = new US_Astfem_RSA( system, simparams );

      connect( astfem_rsa, SIGNAL( new_scan( QVector< double >*, double* ) ), 
                    SLOT( update_movie_plot( QVector< double >*, double* ) ) );
      connect( astfem_rsa, SIGNAL( current_component( int ) ), 
                           SLOT  ( update_progress  ( int ) ) );
      connect( astfem_rsa, SIGNAL( new_time   ( double ) ), 
                           SLOT  ( update_time( double ) ) );
      connect( astfem_rsa, SIGNAL( current_speed( int ) ), 
                           SLOT  ( update_speed ( int ) ) );
      connect( astfem_rsa, SIGNAL( calc_progress( int ) ), 
                           SLOT  ( show_progress( int ) ) );
      connect( astfem_rsa, SIGNAL( calc_done( void ) ), 
                           SLOT  ( calc_over( void ) ) );

      astfem_rsa->set_movie_flag( ck_movie->isChecked() );
 
      astfem_rsa->setTimeInterpolation( false ); 
      astfem_rsa->setTimeCorrection( time_correctionFlag );
      astfem_rsa->setStopFlag( stopFlag );
   
      astfem_rsa->set_debug_flag( dbg_level );
      astfem_rsa->calculate( sim_data );

      if ( dbg_level > 0 )
      {
         double cmin=99999.9;
         double cmax=-999999.9;
         int ilo=0;
         int ihi=0;
         int jlo=0;
         int jhi=0;
         int nscn=sim_data.scanCount();
         int ncvl=sim_data.pointCount();

         for ( int ii=0; ii<nscn; ii++ )
         {
            double t0=sim_data.scanData[ii].seconds;
            double t1=(ii==0)?sim_data.scanData[1].seconds
                             :sim_data.scanData[ii-1].seconds;
            if ( ii==0 || (ii+1)==nscn || (ii*2)==nscn )
               DbgLv(2) << "  Scan" << ii << "  Time" << t0 << t1;
            for ( int jj=0; jj<ncvl; jj++ )
            {
              double cval = sim_data.value(ii,jj);
              if ( cval < cmin ) { ilo=ii; jlo=jj; cmin=cval; }
              if ( cval > cmax ) { ihi=ii; jhi=jj; cmax=cval; }
              if ( ii==0 || (ii+1)==nscn || (ii*2)==nscn )
              {
                 if ( jj<10 || (jj+11)>ncvl ||
                    ((jj*2)>(ncvl-10)&&(jj*2)<(ncvl+11)) )
                    DbgLv(2) << "    C index value" << jj << cval;
              }
            }
         }
         DbgLv(1) << "SIM data min conc ilo jlo" << cmin << ilo << jlo;
         DbgLv(1) << "SIM data max conc ihi jhi" << cmax << ihi << jhi;
         DbgLv(1) << "SIM:fem: m b  s D  rpm" << simparams.meniscus << simparams.bottom
            << system.components[0].s << system.components[0].D
            << simparams.speed_step[0].rotorspeed;
      }
   }

   else
   {  // special case:  ASTFVM (finite volume)

      // create ASTFVM solver
      astfvm = new US_LammAstfvm( system, simparams, this );

      // set up to report progress
      connect( astfvm, SIGNAL( calc_start( int ) ), 
                       SLOT  ( start_calc( int ) ) );

      connect( astfvm, SIGNAL( calc_progress( int ) ), 
                       SLOT  ( show_progress( int ) ) );

      connect( astfvm, SIGNAL( calc_done( void ) ), 
                       SLOT  ( calc_over( void ) ) );

      connect( astfvm, SIGNAL( new_scan( QVector< double >*, double* ) ), 
                    SLOT( update_movie_plot( QVector< double >*, double* ) ) );
      connect( astfvm, SIGNAL( new_time   ( double ) ), 
                       SLOT  ( update_time( double ) ) );

      // initialize LCD with component "1"
      lcd_component->setMode( QLCDNumber::Dec );
      lcd_component->display( 1 );

      astfvm->set_buffer( buffer );
      astfvm->setMovieFlag( ck_movie->isChecked() );

      // solve using ASTFVM
      int rc = astfvm->calculate( sim_data );

      if ( rc != 0 )
      {  // report on multiple non-ideal conditions
         QMessageBox::information( this, 
               tr( "Non-Ideal Simulation Error" ), 
               tr( "Unable to create simulation.\n"
                   "Multiple non-ideal conditions exist.\n"
                   "Edit the ambiguous model." ) ); 
         return;
      }

      // on completion, set LCD display to components count
      lcd_component->display( system.components.size() ); 
   }

   if ( dbg_level > 0 )
   {
      DbgLv(1) << "SIM data simp.temperature" << simparams.temperature;
      double cmin=99999.9;
      double cmax=-999999.9;
      int ilo=0;
      int ihi=0;
      int jlo=0;
      int jhi=0;
      int nscn=sim_data.scanCount();
      int npts=sim_data.pointCount();
      for ( int ii=0; ii<nscn; ii++ )
      {
         double t0=sim_data.scanData[ii].seconds;
         double t1=(ii==0)?sim_data.scanData[1].seconds
                          :sim_data.scanData[ii-1].seconds;
         if ( ii==0 || (ii+1)==nscn || (ii*2)==nscn )
            DbgLv(2) << "  Scan" << ii << "  Time" << t0 << t1
                     << "temp" << sim_data.scanData[ii].temperature;
         for ( int jj=0; jj<npts; jj++ )
         {
            double cval = sim_data.value(ii,jj);
            if ( cval < cmin ) { ilo=ii; jlo=jj; cmin=cval; }
            if ( cval > cmax ) { ihi=ii; jhi=jj; cmax=cval; }
            if ( ii==0 || (ii+1)==nscn || (ii*2)==nscn )
            {
               if ( jj<10 || (jj+11)>npts ||
                   ((jj*2)>(npts-10)&&(jj*2)<(npts+11)) )
                  DbgLv(2) << "    C index value" << jj << cval;
            }
         }
      }
      DbgLv(1) << "SIM data min conc ilo jlo" << cmin << ilo << jlo;
      DbgLv(1) << "SIM data max conc ihi jhi" << cmax << ihi << jhi;
      DbgLv(1) << "SIM:fem: m b  s D  rpm" << simparams.meniscus
         << simparams.bottom << system.components[0].s
         << system.components[0].D << simparams.speed_step[0].rotorspeed;
   }

   finish();
}

void US_Astfem_Sim::finish( void )
{
   total_conc = 0.0;

   for ( int i = 0; i < system.components.size(); i++ )
      total_conc += system.components[ i ].signal_concentration;

//DbgLv(1) << "FIN: comp size" << system.components.size();
//DbgLv(1) << "FIN:  total_conc" << total_conc;
   ri_noise();
   random_noise();
   ti_noise();

   // If we didn't interrupt, we need to set to 100 % complete at end of run
   if ( ! stopFlag )
   {
      update_progress( progress->maximum() );
//DbgLv(1) << "FIN:  progress maxsize" << progress->maximum();
   }

   stopFlag = false;

   plot();

   pb_stop   ->setEnabled( false  );
   pb_start  ->setEnabled( true );
   pb_saveSim->setEnabled( true );

   if ( astfem_rsa )
   {
      delete astfem_rsa;
      astfem_rsa  = NULL;
   }

   if ( astfvm )
   {
      delete astfvm;
      astfvm      = NULL;
   }
}

void US_Astfem_Sim::ri_noise( void )
{
   if ( simparams.rinoise == 0.0 ) return;

   // Add radially invariant noise
   for ( int j = 0; j < sim_data.scanData.size(); j++ )
   {
      double rinoise = 
         US_Math2::box_muller( 0, total_conc * simparams.rinoise / 100 );

      for ( int k = 0; k < sim_data.pointCount(); k++ )
         sim_data.scanData[ j ].rvalues[ k ] += rinoise;
   }
}

void US_Astfem_Sim::random_noise( void )
{
   if ( simparams.rnoise == 0.0 ) return;
   // Add random noise

   for ( int j = 0; j < sim_data.scanData.size(); j++ )
   {
      for ( int k = 0; k < sim_data.pointCount(); k++ )
      {
         sim_data.scanData[ j ].rvalues[ k ] 
            += US_Math2::box_muller( 0, total_conc * simparams.rnoise / 100 );
      }
   }
}

void US_Astfem_Sim::ti_noise( void )
{
   if ( simparams.tinoise == 0.0 ) return;

   // Add time invariant noise
   int points = sim_data.pointCount();
   QVector< double > tinoise;
   tinoise.resize( points );
      
   double val = US_Math2::box_muller( 0, total_conc * simparams.tinoise / 100 );
   
   for ( int k = 0; k < points; k++ )
   {
      val += US_Math2::box_muller( 0, total_conc * simparams.tinoise / 100 );
      tinoise[ k ] = val;
   }

   for ( int j = 0; j < sim_data.scanData.size(); j++ )
   {
      for ( int k = 0; k < points; k++ )
         sim_data.scanData[ j ].rvalues[ k ] += tinoise[ k ];
   }
}

void US_Astfem_Sim::plot( void )
{
   scanPlot->detachItems();

   // Set plot scale
   if ( simparams.band_forming )
      scanPlot->setAxisScale( QwtPlot::yLeft, 0, total_conc );

   else if ( system.coSedSolute >= 0 )
   {
      scanPlot->setAxisAutoScale( QwtPlot::xBottom );
      scanPlot->setAxisAutoScale( QwtPlot::yLeft   );
   }

   else
   {
      scanPlot->setAxisScale( QwtPlot::yLeft, 0, total_conc * 2.0 );
   }

   QwtPlotGrid* grid2 = us_grid( scanPlot );
   grid2->enableX(    true );
   grid2->enableY(    true );

   // Plot the simulation
   if ( ! stopFlag )
   {
      int   scan_count = sim_data.scanCount();
      int   points     = sim_data.pointCount();
      int*  curve      = new int[ scan_count ];
   
      double*  x;
      double** y;

      x = new double  [ points ];
      y = new double* [ scan_count ];

      for ( int j = 0; j < points; j++ )
         x[ j ] = sim_data.xvalues[ j ];

      for ( int j = 0; j < scan_count; j++ )
         y[ j ] = new double [ points ];
      
      for ( int j = 0; j < scan_count; j++ )
      {
         for ( int k = 0; k < points; k++ )
            y[ j ][ k ] = sim_data.value( j, k );
      }

      for ( int j = 0; j < scan_count; j++ )
      {
         QString title = "Concentration" + QString::number( j );
         QwtPlotCurve* plotCurve = new QwtPlotCurve( title );
         
         plotCurve->setData( x, y[ j ], points );
         plotCurve->setPen( QPen( Qt::yellow ) );
         plotCurve->attach( scanPlot );
      }

      delete [] x;
      
      for ( int j = 0; j < scan_count; j++ ) delete [] y[ j ];
      delete [] y;

      delete [] curve;
      scanPlot->replot();
   }
}

void US_Astfem_Sim::save_scans( void )
{
   QString fn = QFileDialog::getExistingDirectory( this,
         tr( "Select a directory for the simulated data:" ),
         US_Settings::importDir() );

   // The user gave a directory name, save in openAUC format

   if ( ! fn.isEmpty() )
   {
      fn = fn.replace( "\\", "/" );
      save_xla( fn );
   }
}

void US_Astfem_Sim::save_xla( const QString& dirname )
{
   double brad      = simparams.bottom;
   double mrad      = simparams.meniscus;
   double grid_res  = simparams.radial_resolution;

   // Add 30 points in front of meniscus                                                               
   int    points      = (int)( ( brad - mrad ) / grid_res ) + 31; 
   
   double maxc        = 0.0;
   int    total_scans = sim_data.scanCount();
   int    old_points  = sim_data.pointCount();
   int    kk          = old_points - 1;

   for ( int ii = 0; ii < total_scans; ii++ )
   {  // Accumulate the maximum computed OD value
      maxc = qMax( maxc, sim_data.value( ii, kk ) );
   }

   // Compute a data threshold that is scan 1's plateau reading times 3
   QVector< double > xtmpVec( total_scans );
   QVector< double > ytmpVec( total_scans );
   double *xtmp    = xtmpVec.data();
   double *ytmp    = ytmpVec.data();
   double intercept;
   double slope;
   double sigma;
   double correl;

   for ( int ii = 0; ii < total_scans; ii++ )
   {  // Build time,omega2t points
      xtmp[ ii ]      = sim_data.scanData[ ii ].seconds;
      ytmp[ ii ]      = sim_data.scanData[ ii ].omega2t;
   }

   // Fit to time,omega2t and use fit to compute the time correction
   US_Math2::linefit( &xtmp, &ytmp, &slope, &intercept, &sigma,
                      &correl, total_scans );

   double timecorr = -intercept / slope;
   double s20wcorr = -2.0;
   double omega    = sim_data.scanData[ 0 ].rpm * M_PI / 30.0;
   double oterm    = ( sim_data.scanData[ 0 ].seconds - timecorr )
                     * omega * omega * s20wcorr;
   double s1plat   = 0.0;

   for ( int jj = 0; jj < system.components.count(); jj++ )
   {
      US_Model::SimulationComponent* sc = &system.components[ jj ];
      double conc     = sc->signal_concentration;
      double sval     = sc->s;
      s1plat         += ( conc * exp( oterm * sval ) );
   }

   double dthresh  = s1plat * 3.0;

   // If the overall maximum reading exceeds the threshold,
   //  limit OD values in all scans to the threshold
   if ( maxc > dthresh )
   {
      int nchange     = 0;  // Total threshold changes
      int nmodscn     = 0;  // Modified scans


      for ( int ii = 0; ii < total_scans; ii++ )
      {  // Examine each scan
         int kchange  = 0;  // Changes in a scan

         for ( int jj = 0; jj < old_points; jj++ )
         {  // Examine each reading point in a scan

            if ( sim_data.value( ii, jj ) > dthresh )
            {  // Set a high value to the threshold and bump counts
               sim_data.setValue( ii, jj, dthresh );
               nchange++;   // Bump the count of total threshold changes
               kchange++;   // Bump the count of threshold changes in this scan
            }
         }

         // Bump the count of scans in which a threshold limit was applied
         if ( kchange > 0 )
            nmodscn++;
      }
DbgLv(1) << "Sim:SV: OD-Limit nchange nmodscn" << nchange << nmodscn
 << "maxc dthresh" << maxc << dthresh;

      // Report that some readings were threshold-limited
      QMessageBox::information( this,
            tr( "OD Values Threshold Limited" ),
            tr( "%1 readings in %2 scans were reset\n"
                "to a threshold value of %3 ,\n"
                "3 times the scan 1 plateau value.\n"
                "The pre-threshold-limit maximum OD\n"
                "value was %4 ." )
            .arg( nchange ).arg( nmodscn ).arg( dthresh ).arg( maxc ) );
   }

   //US_ClipData* cd = new US_ClipData( maxc, b, m, total_conc );
   //if ( ! cd->exec() ) return;
   
   progress->setMaximum( total_scans );
   progress->reset();
 
   QVector< double > tconc_v( points );
   double* temp_conc   = tconc_v.data();
   double  rad         = mrad - 30.0 * grid_res;
   sim_data.xvalues.resize( points );
   lb_progress->setText( "Writing..." );
   
   for ( int jj = 0; jj < points; jj++ )
   {
      sim_data.xvalues[ jj ] = rad;
      rad  += grid_res;
   }

   for ( int ii = 0; ii < total_scans; ii++ )
   {
      US_DataIO::Scan* scan = &sim_data.scanData[ ii ];

      for ( int jj = 30; jj < points; jj++ )
      {  // Position the computed concentration values after the first 30
         temp_conc[ jj ] = scan->rvalues[ jj - 30 ];
      }

      for ( int jj = 0; jj < 30; jj++ )
      {  // Zero the first 30 points
         temp_conc[ jj ] = 0.0;
      }

      temp_conc[ 30 ] = s1plat * 2.0;   // Put a spike at the meniscus

      scan->rvalues.resize( points );

      for ( int jj = 0; jj < points; jj++ )
      {  // Store the values: first 30 then computed values
         scan->rvalues[ jj ] = temp_conc[ jj ];
      }

      progress->setValue( ( ii + 1 ) );
//DbgLv(2) << "WD:sc secs" << scan->seconds;
//if ( ii == 0 || (ii+1) == total_scans ) {
//DbgLv(2) << "WD:S0:c00" << scan->rvalues[0];
//DbgLv(2) << "WD:S0:c01" << scan->rvalues[1];
//DbgLv(2) << "WD:S0:c30" << scan->rvalues[30];
//DbgLv(2) << "WD:S0:cn1" << scan->rvalues[points-2];
//DbgLv(2) << "WD:S0:cnn" << scan->rvalues[points-1]; }
   }

   QString run_id    = dirname.section( "/", -1, -1 );
   QString stype     = QString( QChar( sim_data.type[ 0 ] ) )
                     + QString( QChar( sim_data.type[ 1 ] ) );
   QString schann    = QString( QChar( sim_data.channel ) );
   int     cell      = sim_data.cell;
   int     wvlen     = qRound( sim_data.scanData[ 0 ].wavelength );
           wvlen     = ( wvlen < 99 ) ? 123 : wvlen;
   QString ofname    = QString( "%1/%2.%3.%4.%5.%6.auc" )
      .arg( dirname ).arg( run_id ).arg( stype ).arg( cell )
      .arg( schann  ).arg( wvlen  );
   
   US_DataIO::writeRawData( ofname, sim_data );

   progress->setValue( total_scans );
   lb_progress->setText( tr( "Completed" ) );
   
   pb_saveSim->setEnabled( false );
}

// slot to update progress and lcd based on current component
void US_Astfem_Sim::update_progress( int component )
{
   if ( component == -1 )
   {  // -1 component flags reset to maximum
      progress->setValue( progress->maximum() );
      lcd_component->setMode( QLCDNumber::Hex );
      lcd_component->display( "rA " );
   }

   else if ( component < 0 )
   {  // other negative component flags set maximum
      progress->setMaximum( -component );
      lcd_component->setMode( QLCDNumber::Dec );
      lcd_component->display( 0 );
   }

   else
   {  // normal call sets progress value
      progress->setValue( component );
      lcd_component->setMode( QLCDNumber::Dec );
      lcd_component->display( component );
   }
}

// slot to update lcd based on current component
void US_Astfem_Sim::update_component( int component )
{
   lcd_component->setMode( QLCDNumber::Dec );
   lcd_component->display( component );
}

// slot to update progress by time step
void US_Astfem_Sim::show_progress( int time_step )
{
   progress->setValue( time_step );
}

// slot to initialize progress and set maximum steps
void US_Astfem_Sim::start_calc( int steps )
{
   progress_text    = lb_progress->text();
   progress_maximum = progress->maximum();
   progress_value   = progress->value();

   progress   ->setMaximum( steps );
   progress   ->reset();
   lb_progress->setText( tr( "Calculating..." ) );
}

// slot to set progress to maximum
void US_Astfem_Sim::calc_over( void )
{
   progress   ->setMaximum( progress_maximum );
   progress   ->setValue  ( progress_value );
   lb_progress->setText( progress_text );
}

// slot to update movie plot
void US_Astfem_Sim::update_movie_plot( QVector< double >* x, double* c )
{
   moviePlot->clear();
   double total_c = 0.0;
   double yscale  = 0.0;

   if ( simparams.meshType != US_SimulationParameters::ASTFVM  ||
        system.coSedSolute < 0 )
   {  // Get total concentration of all components
      for ( int ii = 0; ii < system.components.size(); ii++ )
         total_c += system.components[ ii ].signal_concentration;

      yscale         = total_c * 2.0;
   }
   else
   {  // Get total concentration of non-cosed components
      for ( int ii = 0; ii < system.components.size(); ii++ )
      {
         if ( ii != system.coSedSolute )
            total_c += system.components[ ii ].signal_concentration;
      }

      yscale         = total_c * 4.0;
   }

   moviePlot->setAxisScale( QwtPlot::yLeft, 0, yscale );
	moviePlot->setAxisAutoScale( QwtPlot::xBottom );
   
   double* r = new double [ x->size() ];
   
   for ( int i = 0; i < x->size(); i++ ) r[ i ] = (*x)[ i ]; 

   QwtPlotCurve* curve = 
      new QwtPlotCurve( "Scan Number " + QString::number( curve_count++ ) );

   curve->setPen( QPen( Qt::yellow ) );
   curve->setData( r, c, x->size() );
   curve->attach( moviePlot );
   
   moviePlot->replot();

   if ( save_movie )
   {
      QPixmap pmap;
      image_count++;
      imageName = imagedir + QString().sprintf( "frame%05d.png", image_count );
      US_GuiUtil::save_png( imageName, moviePlot );
   }

   qApp->processEvents();
//int k=x->size()-1;
//int h=x->size()/2;
//DbgLv(1) << "UpdMovie: r0 rh rn c0 ch cn" << r[0] << r[h] << r[k]
//   << c[0] << c[h] << c[k];
   
   delete [] r;
}

// slot to update movie plot
void US_Astfem_Sim::update_save_movie( bool ckd )
{
DbgLv(1) << "SIM: update_save_movie" << ckd;
   save_movie   = ckd;

   if ( save_movie )
   {
      imagedir   = QFileDialog::getExistingDirectory( this,
            tr( "Select or create a movie frames directory" ),
            US_Settings::tmpDir() );
DbgLv(1) << "SIM:  upd_sv_movie: imagedir" << imagedir;

      if ( imagedir.isEmpty() )
      {
         imagedir   = US_Settings::tmpDir() + "/movies";
         QDir().mkpath( imagedir );
      }

      if ( ! imagedir.endsWith( "/" ) )
         imagedir   = imagedir + "/";
DbgLv(1) << "SIM:  upd_sv_movie:   imagedir" << imagedir;
   }
}

void US_Astfem_Sim::dump_system( void )
{
   qDebug() << "description" <<system.description;
   qDebug() << "modelGUID" << system.modelGUID;
   qDebug() << "component vector size" << system.components.size();
   for ( int i = 0; i < system.components.size(); i++ ) 
   {
      qDebug() << "component " << i + 1;
      dump_simComponent( system.components[ i ] );
   }
   qDebug() << "association vector size" << system.associations.size();
   for ( int i = 0; i < system.associations.size(); i++ )
   {
      qDebug() << "Association vector " << i + 1;
      dump_association( system.associations[ i ] );
   }
}

void US_Astfem_Sim::dump_simComponent( US_Model::SimulationComponent& sc )
{
   qDebug() << "molar_concentration" << sc.molar_concentration;
   qDebug() << "signal_concentration" << sc.signal_concentration;
   qDebug() << "vbar20" << sc.vbar20;
   qDebug() << "mw" << sc.mw;
   qDebug() << "s" << sc.s;
   qDebug() << "D" << sc.D;
   qDebug() << "f" << sc.f;
   qDebug() << "f_f0" << sc.f_f0;
   qDebug() << "extinction" << sc.extinction;
   qDebug() << "axial_ratio" << sc.axial_ratio;
   qDebug() << "sigma" << sc.sigma;
   qDebug() << "delta" << sc.delta;
   qDebug() << "oligomer" << sc.oligomer;
   qDebug() << "shape" << sc.shape;
   qDebug() << "name" << sc.name;
   qDebug() << "analyte_type" << sc.analyte_type;
   qDebug() << "mfem_initial:";
   dump_mfem_initial( sc.c0 );
}

void US_Astfem_Sim::dump_mfem_initial( US_Model::MfemInitial& mfem )
{
   qDebug() << "radius list size " << mfem.radius.size();
//   qDebug() << "radius list" << mfem.radius;
   qDebug() << "concentration list size " << mfem.concentration.size();
//   qDebug() << "concentration list" << mfem.concentration;
}

void US_Astfem_Sim::dump_association( US_Model::Association& as )
{
   qDebug() << "k_d" << as.k_d;
   qDebug() << "k_off" << as.k_off;
   qDebug() << "rcomps list size " << as.rcomps.size();
   qDebug() << "rcomps list " << as.rcomps;
   qDebug() << "stoichs list size " << as.stoichs.size();
   qDebug() << "stoichs list " << as.stoichs;
}

void US_Astfem_Sim::dump_simparms( void )
{
   qDebug() << "simparams";
   qDebug() << "mesh_radius list size " << simparams.mesh_radius.size();
//   qDebug() << "mesh_radius list " << simparams.mesh_radius;;
   qDebug() << "speed profile list size " << simparams.speed_step.size();
   for ( int i = 0; i < simparams.speed_step.size(); i++ )
      dump_ss( simparams.speed_step[ i ] );
   qDebug() << "simpoints " << simparams.simpoints;
   qDebug() << "meshType " << simparams.meshType;
   qDebug() << "gridType " << simparams.gridType;
   qDebug() << "radial_resolution " << simparams.radial_resolution;
   qDebug() << "meniscus " << simparams.meniscus;
   qDebug() << "bottom " << simparams.bottom;
   qDebug() << "rnoise " << simparams.rnoise;
   qDebug() << "tinoise " << simparams.tinoise;
   qDebug() << "rinoise " << simparams.rinoise;
   qDebug() << "rotorCalID " << simparams.rotorCalID;
   qDebug() << "band_forming " << simparams.band_forming;
   qDebug() << "band_volume " << simparams.band_volume;
   qDebug() << "firstScanIsConcentration "
      << simparams.firstScanIsConcentration;
}

void US_Astfem_Sim::dump_ss( US_SimulationParameters::SpeedProfile& sp )
{
   qDebug() << "speed profile";
   qDebug() << "duration_hours " << sp.duration_hours;
   qDebug() << "duration_minutes " << sp.duration_minutes;
   qDebug() << "delay_hours " << sp.delay_hours;
   qDebug() << "delay_minutes " << sp.delay_minutes;
   qDebug() << "scans " << sp.scans;
   qDebug() << "acceleration " << sp.acceleration;
   qDebug() << "rotorspeed " << sp.rotorspeed;
   qDebug() << "acceleration_flag " << sp.acceleration_flag;
}

void US_Astfem_Sim::dump_astfem_data( void )
{
   /*
   qDebug() << "astfem_data---- list size " << astfem_data.size();
   for ( int j = 0; j < astfem_data.size(); j++ ) 
   {
      qDebug() << "id " << astfem_data[ j ].id;
      qDebug() << "cell " << astfem_data[ j ].cell;
      qDebug() << "channel " << astfem_data[ j ].channel;
      qDebug() << "wavelength " << astfem_data[ j ].wavelength;
      qDebug() << "rpm " << astfem_data[ j ].rpm;
      qDebug() << "s20w_correction " << astfem_data[ j ].s20w_correction;
      qDebug() << "D20w_correction " << astfem_data[ j ].D20w_correction;
      qDebug() << "viscosity " << astfem_data[ j ].viscosity;
      qDebug() << "density " << astfem_data[ j ].density;
      qDebug() << "vbar " << astfem_data[ j ].vbar;
      qDebug() << "avg_temperature " << astfem_data[ j ].avg_temperature;
      qDebug() << "vbar20 " << astfem_data[ j ].vbar20;
      qDebug() << "meniscus " << astfem_data[ j ].meniscus;
      qDebug() << "bottom " << astfem_data[ j ].bottom;
      qDebug() << "radius list size " << astfem_data[ j ].radius.size();
      //qDebug() << "radius list " << astfem_data[ j ].radius;;
      qDebug() << "scan list size " << astfem_data[ j ].scan.size();
      for ( int i = 0; i < astfem_data[ j ].scan.size(); i++ ) 
         dump_mfem_scan( astfem_data[ j ].scan [ i ] );
   }
   */
}

void US_Astfem_Sim::dump_mfem_scan( US_DataIO::Scan& /*ms*/ )
{
   /*
   qDebug() << "mfem_scan----";
   qDebug() << "time " << ms.time;
   qDebug() << "omega_s_t " << ms.omega_s_t;
   qDebug() << "rpm " << ms.rpm;
   qDebug() << "temperature " << ms.temperature;
   qDebug() << "time " << ms.time;
   qDebug() << "conc list size " << ms.conc.size();
   //qDebug() << "conc " << ms.conc;
   */
}
 
