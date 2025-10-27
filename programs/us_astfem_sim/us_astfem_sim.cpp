//! \file us_astfem_sim.cpp

#include <QApplication>

#include "us_license.h"
#include "us_license_t.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_gui_util.h"
#include "us_astfem_sim.h"
#include "us_sim_params_gui.h"
#include "us_math2.h"
#include "us_astfem_math.h"
#include "us_defines.h"
#include "us_clipdata.h"
#include "us_rotor_gui.h"
#include "us_db2.h"
#include "us_gui_util.h"
#include "us_model_gui.h"
#include "us_buffer_gui.h"
#include "us_util.h"
#include "us_astfem_rsa.h"
#include "us_lamm_astfvm.h"
#include "us_time_state.h"

#if QT_VERSION < 0x050000
#define setSamples(a,b,c)  setData(a,b,c)
#endif

//! \brief Main program for US_Astfem_Sim.  Loads translators and starts
//! the class US_Astfem_Sim.
int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );
   QApplication::setApplicationName("us_astfem_sim");
   QApplication::setApplicationDisplayName("US Astfem Simulation Module");
   QApplication::setApplicationVersion( US_Version );
   QApplication::setOrganizationDomain("https://ultrascan.aucsoltions.com");
   QApplication::setOrganizationName("AUC Solutions, LLC");
   #include "main1.inc"

   // License is OK. Start up.
   US_Astfem_Sim w;

   QCommandLineParser parser;
   auto help_option = QCommandLineOption({"help", "h", "?"},
      "Display help on commandline options");
   parser.addOption(help_option);
   auto version_option = parser.addVersionOption();
   auto model_option = QCommandLineOption("model",
      "Load model from file path, GUID or DB ID",
      "model");
   parser.addOption(model_option);
   auto buffer_option = QCommandLineOption("buffer",
      "Load buffer from file path, GUID or DB ID",
      "buffer");
   parser.addOption(buffer_option);
   auto sim_parameters_option = QCommandLineOption("simparams",
      "Load simulation parameters from file path",
      "simparams");
   parser.addOption(sim_parameters_option);
   auto rotor_option = QCommandLineOption("rotor",
      "Load rotor from file path, GUID or DB ID",
      "rotor");
   parser.addOption(rotor_option);
   auto movie_option = QCommandLineOption("movie",
      "Show movie of simulation");
   parser.addOption(movie_option);
   auto time_correction_option = QCommandLineOption("timecorr",
      "Use time correction");
   parser.addOption(time_correction_option);
   auto start_option = QCommandLineOption("start",
      "Start simulation automatically");
   parser.addOption(start_option);
   auto save_option = QCommandLineOption("save",
      "Save simulation data to file path",
      "save");
   parser.addOption(save_option);
   auto close_option = QCommandLineOption("close",
      "Close application if no errors occurred");
   parser.addOption(close_option);
   auto ignore_db_option = QCommandLineOption("no-db",
      "Ignore any database preferences and only use locally available data");
   parser.addOption(ignore_db_option);
   auto errors_option = QCommandLineOption("errors-cl",
      "Force errors to console and don't open any sort of gui");
   parser.addOption(errors_option);

   QMap<QString, QString> args;
   int cli_parsing_result = -1; //!< -1 not finished, 0 headless, 1 gui needed, 2 error
   // parser had a problem parsing cli arguments
   if ( !parser.parse( QApplication::arguments() ) )
   {
      // print error message to console via QTextStream
      QTextStream(stderr) << qUtf8Printable(parser.errorText()) << Qt::endl;
      // exit QApplication with error code
      QApplication::exit( 1 );
      return 1;
   }
   // parser detected version option
   if ( parser.isSet( version_option ) )
   {
      QTextStream(stdout) << QString::asprintf( "%s (%s)\nVersion %s\n\n",
         qUtf8Printable(QApplication::applicationDisplayName() ),
         qUtf8Printable( QApplication::applicationName() ),
         qUtf8Printable( QApplication::applicationVersion() )) << Qt::endl;
      QApplication::exit( 0 );
      return 0;
   }
   // parser detected help option (help or help-all)
   if ( parser.isSet( help_option ) )
   {
      QTextStream(stdout) << qUtf8Printable( parser.helpText() ) << Qt::endl;
      QApplication::exit( 0 );
      return 0;
   }

   // parse command specific commands

   // parse ignore db
   int default_data_location = US_Settings::default_data_location();
   if ( parser.isSet( ignore_db_option ) )
   {
      US_Settings::set_default_data_location( 2 );
   }
   // parse model
   if ( parser.isSet( model_option ) && !parser.value( model_option ).isEmpty() )
   {
      args["model"] = parser.value( model_option );
   }
   else
   {
      cli_parsing_result = qMax( cli_parsing_result, 1 );
   }
   // parse buffer
   if ( parser.isSet( buffer_option ) && !parser.value( buffer_option ).isEmpty() )
   {
      args["buffer"] = parser.value( buffer_option );
   }
   else
   {
      cli_parsing_result = qMax( cli_parsing_result, 1 );
   }
   // parse simulation parameters
   if ( parser.isSet( sim_parameters_option ) && !parser.value( sim_parameters_option ).isEmpty() )
   {
      args["simparams"] = parser.value( sim_parameters_option );
   }
   else
   {
      cli_parsing_result = qMax( cli_parsing_result, 1 );
   }
   // parse rotor
   if ( parser.isSet( rotor_option ) && !parser.value( rotor_option ).isEmpty() )
   {
      args["rotor"] = parser.value( rotor_option );
   }
   else
   {
      cli_parsing_result = qMax( cli_parsing_result, 1 );
   }
   // parse movie
   if ( parser.isSet( movie_option ) )
   {
      args["movie"] = "true";
   }
   // parse time correction
   if ( parser.isSet( time_correction_option ) )
   {
      args["timecorr"] = "true";
   }
   // parse start
   if ( parser.isSet( start_option ) )
   {
      args["start"] = "true";
   }
   // parse errors
   if ( parser.isSet( errors_option ) )
   {
      args["errors-cl"] = "true";
   }
   // parse save
   if ( parser.isSet( save_option ) && !parser.value( save_option ).isEmpty() )
   {
      args["save"] = parser.value( save_option ).replace("\\", "/");
   }
   else
   {
      cli_parsing_result = qMax( cli_parsing_result, 1 );
   }
   // parse close
   if ( parser.isSet( close_option ) )
   {
      args["close"] = "true";
   }
   else
   {
      cli_parsing_result = qMax( cli_parsing_result, 1 );
   }
   cli_parsing_result = qMax( cli_parsing_result, 0 );
   int init_status = w.init_from_args(args);
   if ( default_data_location != US_Settings::default_data_location() && parser.isSet( ignore_db_option ) )
   {
      // revert the previously changed default data location
      US_Settings::set_default_data_location( default_data_location );
   }
   // Only show GUI if needed
   if ( init_status != 0 && !args.contains( "errors-cl" )) {
      w.show();
      return QApplication::exec();
   }

   return init_status;
}

// US_Astfem_Sim constructor
US_Astfem_Sim::US_Astfem_Sim( QWidget* p, Qt::WindowFlags f )
   : US_Widgets( true, p, f )
{
   dbg_level           = US_Settings::us_debug();
   tmst_tfpath         = "";

   setWindowTitle( "UltraScan3 Simulation Module" );
   setPalette( US_GuiSettings::frameColor() );
   init_simparams();
   meniscus_ar                 = 5.8 + simparams.bottom_position - 7.2;
   stopFlag            = false;
   movieFlag           = false;
   save_movie          = false;
   time_correctionFlag = false;
   imagedir            = US_Settings::tmpDir() + "/movies";

   // Initialize buffer density to default water @ 20C:
   buffer.density      = DENS_20W;
   buffer.viscosity    = VISC_20W;
   rotor               = US_Rotor::Rotor();
   rotor_calibration   = US_Rotor::RotorCalibration();
   astfem              = nullptr;
   astfvm              = nullptr;

   clean_etc_dir();

   QGridLayout* main     = new QGridLayout( this );
   main->setSpacing( 2 );
   main->setContentsMargins ( 2, 2, 2, 2 );
   main->setColumnStretch( 1, 1 );

   pb_changeModel        = us_pushbutton( tr( "Model Control") );
   pb_buffer             = us_pushbutton( tr( "Define Buffer"),         false );
   pb_simParms           = us_pushbutton( tr( "Simulation Parameters"), false );
   pb_rotor              = us_pushbutton( tr( "Select rotor"),          false );
   pb_start              = us_pushbutton( tr( "Start Simulation" ),     false );
   pb_stop               = us_pushbutton( tr( "Stop Simulation" ),      false );
   pb_saveSim            = us_pushbutton( tr( "Save Simulation" ),      false );
   QPushButton* pb_help  = us_pushbutton( tr( "Help" )  );
   QPushButton* pb_close = us_pushbutton( tr( "Close" ) );
   QPalette pa( pb_close->palette() );
   te_status             = us_textedit();
   te_status->setPalette( pa );
   te_status->setTextBackgroundColor( pa.color( QPalette::Window ) );
   te_status->setTextColor(           pa.color( QPalette::WindowText ) );
   QFontMetrics fm( te_status->font() );
   te_status->setMaximumHeight( fm.lineSpacing() * 25 / 2 );

   QGridLayout* movie      = us_checkbox( "Show Movie", ck_movie, movieFlag );
   QGridLayout* lo_svmovie = us_checkbox( "Save Movie", ck_savemovie, false );
   QGridLayout* timeCorr   = us_checkbox( "Use Time Correction", ck_timeCorr,
                                           time_correctionFlag );
   QBoxLayout* buttonbox   = new QVBoxLayout;
   QBoxLayout* timeSpeed   = new QHBoxLayout;
   QBoxLayout* completion  = new QHBoxLayout;


   buttonbox->addWidget( pb_changeModel );
   buttonbox->addWidget( pb_buffer );
   buttonbox->addWidget( pb_simParms );
   buttonbox->addWidget( pb_rotor );
   buttonbox->addLayout( movie );
   buttonbox->addLayout( lo_svmovie );
   buttonbox->addLayout( timeCorr );
   buttonbox->addWidget( pb_start );
   buttonbox->addWidget( pb_stop );
   buttonbox->addWidget( pb_saveSim );
   buttonbox->addWidget( pb_help );
   buttonbox->addWidget( pb_close );
   buttonbox->addWidget( te_status );
   buttonbox->addStretch();

   connect( pb_changeModel,SIGNAL( clicked()        ),
            this,          SLOT(   new_model()      ) );
   connect( pb_buffer,     SIGNAL( clicked()        ),
            this,          SLOT(   new_buffer()     ) );
   connect( pb_simParms,   SIGNAL( clicked()        ),
            this,          SLOT(   sim_parameters() ) );
   connect( pb_rotor,      SIGNAL( clicked()        ),
            this,          SLOT(   select_rotor() ) );
   connect( ck_savemovie,  SIGNAL( toggled          ( bool ) ),
            this,          SLOT(   update_save_movie( bool ) ) );
   connect( ck_timeCorr,   SIGNAL( clicked()          ),
            this,          SLOT(   update_time_corr() ) );
   connect( pb_start,      SIGNAL( clicked()          ),
            this,          SLOT(   start_simulation() ) );
   connect( pb_stop,       SIGNAL( clicked()          ),
            this,          SLOT(   stop_simulation()  ) );
   connect( pb_saveSim,    SIGNAL( clicked()    ),
            this,          SLOT(   save_scans() ) );
   connect( pb_help,       SIGNAL( clicked()    ),
            this,          SLOT(   help()       ) );
   connect( pb_close,      SIGNAL( clicked()    ),
            this,          SLOT(   close()      ) );

   main->addLayout( buttonbox, 0, 0 );


   // Right Column
   QBoxLayout* plot = new QVBoxLayout;

   // Simulation Plot
   plot1            = new US_Plot( moviePlot,
                                   tr( "Simulation Window" ),
                                   tr( "Radius (cm)" ),
                                   tr( "Concentration" ) );
   us_grid  ( moviePlot );
   moviePlot->setMinimumSize( 600, 275);
   moviePlot->setAxisScale( QwtPlot::yLeft, 0.0, 2.0 );
   moviePlot->setAxisScale( QwtPlot::xBottom, simparams.meniscus, simparams.bottom );

   plot->addLayout( plot1 );


   QLabel* lb_time  = us_label( tr( "Time [s]:" ) );
   lcd_time         = us_lcd( 7, 0 );
   QLabel* lb_speed = us_label( tr( "Current Speed [rpm]:" ) );
   lcd_speed        = us_lcd( 6, 0 );
   QLabel* lb_scan = us_label( tr( "Current Scan:" ) );
   lcd_scan        = us_lcd( 5, 0 );
   lb_time->setAlignment ( Qt::AlignCenter );
   lb_speed->setAlignment( Qt::AlignCenter );
   lb_scan->setAlignment(Qt::AlignCenter);

   timeSpeed->addWidget( lb_time );
   timeSpeed->addWidget( lcd_time );
   timeSpeed->addWidget( lb_speed );
   timeSpeed->addWidget( lcd_speed );
   timeSpeed->addWidget( lb_scan );
   timeSpeed->addWidget( lcd_scan );

   // Saved Scans
   plot2              = new US_Plot( scanPlot,
                                     tr( "Saved Scans" ),
                                     tr( "Radius (cm)" ),
                                     tr( "Concentration" ),
                                     true, "Concentration", "rainbow" );
   QwtPlotGrid* grid2 = us_grid  ( scanPlot );
   grid2->enableX(    true );
   grid2->enableY(    true );
   scanPlot->setMinimumSize( 600, 275);
   scanPlot->setAxisScale( QwtPlot::yLeft,   0.0, 2.0 );
   scanPlot->setAxisScale( QwtPlot::xBottom, simparams.meniscus, simparams.bottom );

   lb_component  = us_label( tr( "Component:" ) );
   lcd_component = us_lcd  ( 7, 0 );
   lb_progress   = us_label( tr( "% Completed:" ) );
   progress_value = 0;
   progress_maximum = 100;
   progress      = us_progressBar( 0, progress_maximum, progress_value );

   lb_component->setAlignment ( Qt::AlignCenter );
   lb_progress->setAlignment ( Qt::AlignCenter );

   completion->addWidget( lb_component );
   completion->addWidget( lcd_component );
   completion->addWidget( lb_progress );
   completion->addWidget( progress );

   plot->addLayout( timeSpeed );
   plot->addLayout( plot2 );
   plot->addLayout( completion );

   main->addLayout( plot, 0, 1 );
   // initialize variables
   total_conc = 0.0;
   times_comp = 0.0; // gets updated by change_status
   icomponent = 0;
   ncomponent = 0; // gets updated by change_status
   curve_count = 0;
   image_count = 0;


   change_status();
}

// Initialize simulation from command line arguments
int US_Astfem_Sim::init_from_args( const QMap<QString, QString>& flags ) {
   // check if model is to be loaded
   bool gui_needed = !flags.contains("close");
   bool error_occured = false;
   bool loaded_model = false;
   bool loaded_buffer = false;
   bool loaded_simparams = false;
   bool loaded_rotor = false;
   bool errors_to_cl = flags.contains("errors-cl");
   // load model
   if ( flags.contains("model") && flags["model"].length() > 0 ) {
      US_Model temp_model = US_Model();
      QString model_id = flags["model"];
      US_ModelGui* dialog = new US_ModelGui( system  );
      bool success = dialog->load_model( model_id, temp_model );
      dialog->close();
      if ( !success ) {
         if ( errors_to_cl )
         {
            // print error message to command line and exit
            qDebug() << "Error loading model " << model_id;
            exit( 2 );
         }
         gui_needed = true;
         error_occured = true;
      }
      else {
         change_model(temp_model);
         loaded_model = true;
      }
      delete dialog;
   }
   // load buffer
   if ( flags.contains("buffer") && flags["buffer"].length() > 0 ) {
      QString load_id = flags["buffer"];
      US_BufferGui* dialog = new US_BufferGui( true, buffer, US_Disk_DB_Controls::Default );
      bool success = dialog->load_buffer( load_id, buffer );
      dialog->close();
      if ( !success ) {
         if ( errors_to_cl )
         {
            // print error message to command line and exit
            qDebug() << "Error loading buffer " << load_id;
            exit( 2 );
         }
         gui_needed = true;
         error_occured = true;
      }
      else {
         change_buffer(buffer);
         loaded_buffer = true;
      }
      delete dialog;
   }
   // load simulation parameters if needed
   if ( flags.contains("simparams") && flags["simparams"].length() > 0 ) {
      QString load_id = flags["simparams"];
      US_SimParamsGui* dialog = new US_SimParamsGui( simparams );
      bool success = dialog->load_params( load_id, simparams );
      dialog->close();
      if ( !success ) {
         if ( errors_to_cl )
         {
            // print error message to command line and exit
            qDebug() << "Error loading simparams " << load_id;
            exit( 2 );
         }
         gui_needed = true;
         error_occured = true;
      }
      else {
         set_parameters( );
         loaded_simparams = true;
      }
      delete dialog;
   }
   // load rotor if needed
   if ( flags.contains("rotor") && flags["rotor"].length() > 0 )
   {
      US_Rotor::Rotor rotor;
      US_Rotor::RotorCalibration calibration;
      US_Disk_DB_Controls*    disk_controls;
      QString rotor_id = flags["rotor"];
      disk_controls     = new US_Disk_DB_Controls( US_Disk_DB_Controls::Default );
      int dbdisk = ( disk_controls->db() ) ? US_Disk_DB_Controls::DB
                                            : US_Disk_DB_Controls::Disk;
      US_RotorGui* rotorInfo = new US_RotorGui( true,    // signal_wanted
                                                 dbdisk,
                                                 rotor, calibration );
      double coeff1 = 0.0;
      double coeff2 = 0.0;
      bool status = rotorInfo->load_rotor( rotor_id, coeff1, coeff2 );

      if ( status )
      {
         this->rotor = rotorInfo->currentRotor;
         this->rotor_calibration = rotorInfo->currentCalibration;
         simparams.rotorcoeffs[0]   = coeff1;
         simparams.rotorcoeffs[1]   = coeff2;
         simparams.rotorCalID = QString::number( rotorInfo->currentCalibration.ID );
         loaded_rotor = true;
         rotorInfo->close();
      }
      else {
         rotorInfo->close();
         if ( errors_to_cl )
         {
            // print error message to command line and exit
            qDebug() << "Error loading rotor " << rotor_id;
            exit( 2 );
         }
         gui_needed = true;
         error_occured = true;
      }
      delete rotorInfo;
      delete disk_controls;
   }

   // set movie flag if needed
   if ( flags.contains("movie") && ck_movie != nullptr )
   {
      ck_movie->setChecked( true );
      movieFlag = true;
   }
   // set timecorr flag if needed
   if ( flags.contains("timecorr") && ck_timeCorr != nullptr )
   {
      ck_timeCorr->setChecked( true );
      time_correctionFlag = true;
   }
   // check save directory
   if ( flags.contains("save") && flags["save"].length() > 0 )
   {
      // check if path is accessible and writable
      QString save_path = flags["save"];
      QDir dir( save_path );
      if ( !dir.exists() ) {
         // path does not exist
         if ( errors_to_cl )
         {
            // print error message to command line and exit
            qDebug() << "Error save path doesn't exist " << save_path;
            exit( 2 );
         }
         error_occured = true;
         gui_needed = true;
      }
      // check if writeable
      QFile file(dir.filePath( "tmp.txt" ) );
      if ( !file.open(QIODevice::WriteOnly ) )
      {
         if ( errors_to_cl )
         {
            // print error message to command line and exit
            qDebug() << "Error save path isn't writeable " << save_path;
            exit( 2 );
         }
         // path is not writeable
         error_occured = true;
         gui_needed = true;
      }
      else
      {
         file.close();
         if ( file.exists() )
         {
            file.remove();
         }
      }
   }


   if ( !error_occured && loaded_model && loaded_buffer && loaded_simparams && loaded_rotor  )
   {
      // no error yet
      if ( flags.contains("start") ) {
         // start simulation
         start_simulation();
         if ( flags.contains( "save" ) && flags["save"].length() > 0 )
         {
            // check if path is accessible and writable
            QString save_path = flags["save"];
            save_simulation( save_path, true );
         }
      }
   }
   else
   {
      gui_needed = true;
   }
   if ( flags.contains( "close" ) && !gui_needed && !error_occured )
   {
      // close GUI
      gui_needed = false;
   }

   if ( error_occured ) {
      return 2;
   }
   if ( gui_needed )
   {
      return 1;
   }
   return 0;
}

// Initialize simulation parameters
void US_Astfem_Sim::init_simparams( void )
{
   US_SimulationParameters::SpeedProfile sp;
   QString rotor_calibr = "0";
   double  rpm          = 45000.0; // Initialized rotor speed

   // set up bottom start and rotor coefficients from hardware file
   simparams.setHardware( NULL, rotor_calibr, 0, 0 );

   // calculate bottom from rpm, channel bottom pos., rotor coefficients
   double bottom        = US_AstfemMath::calc_bottom( rpm, simparams.bottom_position,
                                                    simparams.rotorcoeffs );
   double menisc_curr   = 5.8 + bottom - simparams.bottom_position;

   simparams.mesh_radius.clear();
   simparams.speed_step .clear();

   sp.duration_hours    = 2;         // Initialized experiment duration hours
   sp.duration_minutes  = 30.0;      // Initialized experiment duration minutes
   sp.delay_hours       = 0;    // Initialized time for accelerating from current rotor speed to next rotor speed in hours
   sp.delay_minutes     = 20.0; // Initialized time for accelerating from current rotor speed to next rotor speed in minutes
   sp.rotorspeed        = (int)rpm;  // Initialized rotor speed
   sp.avg_speed         = rpm;       // Initialized average speed
   sp.set_speed         = (int)rpm;  // Initialized set speed
   sp.scans             = 30;        // Initialized number of scans
   sp.acceleration      = 400;       // Acceleration speed of the rotor
   sp.acceleration_flag = true;      // Flag used for acceleration zone
   sp.delay_minutes     =( double)(sp.rotorspeed/(60.0*sp.acceleration));// Minimum delay ie. time to accelerate the rotor
   simparams.speed_step << sp;

   simparams.simpoints         = 200;    // Initialized number of radial grid points
   simparams.radial_resolution = 0.001;  // Increment in radial experimental grid
   simparams.meshType          = US_SimulationParameters::ASTFEM; // Used for solver option
   simparams.gridType          = US_SimulationParameters::MOVING; // Used for grid option
   simparams.meniscus          = menisc_curr;    // Meniscus for simulation
   simparams.bottom            = bottom;         // Bottom for simulation
   simparams.rnoise            = 0.0;
   simparams.lrnoise           = 0.0;
   simparams.tinoise           = 0.0;
   simparams.rinoise           = 0.0;
   simparams.baseline          = 0.0;
   simparams.band_volume       = 0.015;
   simparams.rotorCalID        = rotor_calibr;
   simparams.band_forming      = false;
}

void US_Astfem_Sim::new_model( void )
{
   system = US_Model();
   US_ModelGui* dialog = new US_ModelGui( system );
   connect( dialog, SIGNAL( valueChanged( US_Model ) ),
                    SLOT  ( change_model( US_Model ) ) );
   dialog->exec();
}

void US_Astfem_Sim::change_model( US_Model m )
{
   system      = m;
   pb_buffer  ->setEnabled( true );
   pb_simParms->setEnabled( true );
   pb_rotor->setEnabled(true);
   // set default of FVM if model is non-ideal
   if ( system.components[ 0 ].sigma != 0.0  ||
        system.components[ 0 ].delta != 0.0  ||
        system.coSedSolute           >= 0    ||
        buffer.compressibility       >  0.0 )
      simparams.meshType = US_SimulationParameters::ASTFVM;

   else  // normal (ideal) default
      simparams.meshType = US_SimulationParameters::ASTFEM;

   change_status();

   ncomponent  = system.components.size();
   icomponent  = 1;
}

void US_Astfem_Sim::new_buffer( void )
{
   US_BufferGui* dialog = new US_BufferGui( true, buffer );

   connect( dialog, SIGNAL( valueChanged ( US_Buffer ) ),
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

// Report on new simulation parameters
void US_Astfem_Sim::change_status()
{
   QStringList mtyps;
   mtyps << "ASTFEM" << "CLAVERIE" << "MOVING_HAT" << "USER" << "ASTFVM";
   QString simtype = mtyps[ (int)simparams.meshType ];

   int    dhrs  = simparams.speed_step[ 0 ].duration_hours;
   double dmns  = simparams.speed_step[ 0 ].duration_minutes;
   int    scns  = simparams.speed_step[ 0 ].scans;
   double spdls = simparams.speed_step[ 0 ].rotorspeed;
   int    nspd  = simparams.speed_step.count();

   for ( int ii = 1; ii < nspd; ii++ )
   {
      dhrs     += simparams.speed_step[ ii ].duration_hours;
      dmns     += simparams.speed_step[ ii ].duration_minutes;
      scns     += simparams.speed_step[ ii ].scans;
      spdls     = simparams.speed_step[ ii ].rotorspeed;
   }

   if ( dmns > 59 )
   {
      int khrs = dmns / 60;
      dmns    -= ( khrs * 60.0 );
      dhrs    += khrs;
   }
   total_scans  = scns;
   times_comp   = dhrs * 3600.0 + dmns * 60.0;
   ncomponent   = system.components.size();
DbgLv(1) << "ASIM: chg_stat: ncomponent" << ncomponent << "times_comp" << times_comp;
   times_comp   = qMax( 100.0, times_comp );
   ncomponent   = qMax( 1, ncomponent );
   int maxts    = ncomponent * scns;
DbgLv(1) << "ASIM: chg_stat:  times_comp" << times_comp << "maxts" << maxts;
   progress->setRange( 1, maxts );
DbgLv(1) << "ASIM: chg_stat:  times_comp" << times_comp << "maxts" << maxts;

   te_status->setText( tr(
      "Model:\n  %1\n"
      "Buffer (density/viscosity/compress.):\n  %2 / %3 / %4\n"
      "SimParams (type/duration/scans):\n  %5 / %6 h %7 m / %8\n"
      "Speeds:  %9    Last speed:  %10" )
      .arg( system.description ).arg( buffer.density ).arg( buffer.viscosity )
      .arg( buffer.compressibility ).arg( simtype )
      .arg( dhrs ).arg( dmns ).arg( scns )
      .arg( nspd ).arg( spdls ) );
}

void US_Astfem_Sim::select_rotor(void)
{
   US_Rotor::Rotor rotor;
   US_Rotor::RotorCalibration calibration;

   US_Disk_DB_Controls*    disk_controls;
   disk_controls     = new US_Disk_DB_Controls( US_Disk_DB_Controls::Default );
   int dbdisk = ( disk_controls->db() ) ? US_Disk_DB_Controls::DB
                                        : US_Disk_DB_Controls::Disk;
DbgLv(1) << "dbdisk_from_us_astfem_sim" << dbdisk;

   US_RotorGui* rotorInfo = new US_RotorGui( true,    // signal_wanted
                                             dbdisk,
                                             rotor, calibration );

   connect( rotorInfo, SIGNAL( RotorCalibrationSelected( US_Rotor::Rotor&, US_Rotor::RotorCalibration& ) ),
                       SLOT  ( assignRotor             ( US_Rotor::Rotor&, US_Rotor::RotorCalibration& ) ) );
   rotorInfo->exec();
DbgLv(1) << "simparams_rotorcoeffs" << simparams.rotorcoeffs[0] << simparams.rotorcoeffs[1];
}

void US_Astfem_Sim::assignRotor( US_Rotor::Rotor& rot, US_Rotor::RotorCalibration& calibration )
{
DbgLv(1) << "assignrotor is called" << calibration.coeff1 << calibration.coeff2;
   simparams.rotorcoeffs[0]   = calibration.coeff1;
   simparams.rotorcoeffs[1]   = calibration.coeff2;
   simparams.rotorCalID       = QString::number( calibration.ID );
   rotor = rot;
DbgLv(1) << "simparams_assign" << simparams.rotorcoeffs[0] << simparams.rotorcoeffs[1];
}

void US_Astfem_Sim::sim_parameters( void )
{
   simparams.meniscus = meniscus_ar;
   simparams.bottom   = simparams.bottom_position;
DbgLv(1) << "SimPar:MAIN:simp: nspeed" << simparams.speed_step.count()
 << "speed0" << simparams.speed_step[0].rotorspeed;

   US_SimParamsGui* dialog = new US_SimParamsGui( simparams );

   connect( dialog, SIGNAL( complete() ), SLOT( set_parameters() ) );

   dialog->exec();
}

void US_Astfem_Sim::set_parameters( void )
{
   meniscus_ar   = simparams.meniscus; // Meniscus at rest
   simparams.sim = true;
   pb_start  ->setEnabled( true );
DbgLv(1) << "SimPar:MAIN:SetP:  nspeed" << simparams.speed_step.count()
 << "speed0" << simparams.speed_step[0].rotorspeed << "meniscus_ar" << meniscus_ar;
simparams.debug();
DbgLv(1) << "==SimPar:MAIN:SetP";

   // Initialize all-speed raw data
   sim_data_all.xvalues .clear();
   sim_data_all.scanData.clear();
   sim_data_all.type[0]    = 'R';
   sim_data_all.type[1]    = 'A';

   QString guid = US_Util::new_guid();
   US_Util::uuid_parse( guid, (uchar*)sim_data_all.rawGUID );

   sim_data_all.cell        = 1;
   sim_data_all.channel     = 'S';
   sim_data_all.description = "Simulation";

   int points    = qRound( ( simparams.bottom - simparams.meniscus ) /
                            simparams.radial_resolution ) + 1;
   // int points    = qCeil( ( simparams.bottom - simparams.meniscus ) /
   //                          simparams.radial_resolution ) + 1;

   sim_data_all.xvalues.resize( points ); // Sets the size of radial grid points array

   for ( int jp = 0; jp < points; jp++ )
   {   // Calculate the radial grid points
      sim_data_all.xvalues[ jp ] = simparams.meniscus
                                   + jp * simparams.radial_resolution;
   }
DbgLv(1) << "SimPar:MAIN:SetP:  points" << points << "rad0 radn"
 << sim_data_all.xvalues[0] << sim_data_all.xvalues[points-1];

//   sim_data_all.xvalues[ points - 1 ] = simparams.bottom;
//DbgLv(1) << "SimPar:MAIN:SetP:  points" << points << "rad0 radn"
// << sim_data_all.xvalues[0] << sim_data_all.xvalues[points-1];

   // Fill in speed steps with scan times and omega^2t; build raw data
   double time0   = 0.0;
   double time1   = 0.0;
   double time2   = 0.0;
   double w2tsum  = 0.0;
   double p_speed = 0.0;
   double s_speed = 0.0;
   double c_speed = 0.0;
   int nstep      = simparams.speed_step.count();

   double previous_speed = 0.0;
   double previous_time  = 0.0;
   double previous_w2t   = 0.0;
   for ( int jj = 0; jj < nstep; jj++ ) {
      US_SimulationParameters::SpeedProfile* sp = &simparams.speed_step[ jj ];
      if ( nstep > 0  &&  sp->set_speed < 1.0 )
      {  // For multi-speed, insure values for set and average speeds
         s_speed        = qRound( sp->rotorspeed * 0.01 ) * 100.0;
         sp->set_speed  = s_speed;
         sp->avg_speed  = sp->rotorspeed;
      }
      const double target_speed = ( sp->set_speed == 0.0 ) ? sp->rotorspeed : sp->set_speed;
      const double delay    = qRound(sp->delay_hours    * 3600.0 + sp->delay_minutes    * 60.0);
      const double duration = qRound(sp->duration_hours * 3600.0 + sp->duration_minutes * 60.0);
      const double scanning_time = duration - delay;
      const double time_between_scans = scanning_time / static_cast<double>( sp->scans - 1 );
      US_DataIO::Scan scandata;
      scandata.temperature = simparams.temperature;
      scandata.rpm         = target_speed;
      scandata.wavelength  = system.wavelength;
      scandata.plateau     = 0.0;
      scandata.delta_r     = simparams.radial_resolution;
      scandata.rvalues     .fill( 0.0, points   );
      for (int s = 0; s < sp->scans; s++) {
         scandata.seconds = static_cast<double>(qRound(previous_time + delay + time_between_scans * s));
         scandata.omega2t = US_AstfemMath::calc_omega2t(previous_w2t, previous_speed, previous_time,
            target_speed, sp->acceleration, scandata.seconds);
         if (s == 0) {
            sp->time_first = scandata.seconds;
            sp->w2t_first  = scandata.omega2t;
         }
         else if ( s == sp->scans - 1) {
            sp->time_last  = scandata.seconds;
            sp->w2t_last   = scandata.omega2t;
         }
         DbgLv( 1 ) << "SimPar:MAIN:SetP: js time omega2t " << s << scandata.seconds << scandata.omega2t;
         sim_data_all.scanData << scandata;
      }
      previous_speed = target_speed;
      previous_time  = sp->time_last;
      previous_w2t   = sp->w2t_last;
   }

   for ( int jd = 0; jd < 0; jd++ )
   {
      US_SimulationParameters::SpeedProfile* sp = &simparams.speed_step[ jd ];
      time0          = time2;
      p_speed        = s_speed;
      c_speed        = p_speed;
DbgLv(1) << "SimPar:MAIN:SetP:   sset" << jd << "time1 time2" << time1 << time2;
      s_speed        = sp->set_speed;

      if ( nstep > 0  &&  s_speed < 1.0 )
      {  // For multi-speed, insure values for set and average speeds
         s_speed        = qRound( sp->rotorspeed * 0.01 ) * 100.0;
         sp->set_speed  = s_speed;
         sp->avg_speed  = sp->rotorspeed;
      }

      s_speed        = ( s_speed == 0.0 ) ? sp->rotorspeed : s_speed;
      double accel   = sp->acceleration;
      double dlay    = sp->delay_hours    * 3600.0 + sp->delay_minutes    * 60.0;
      double durat   = sp->duration_hours * 3600.0 + sp->duration_minutes * 60.0;
      time1          = qRound( time0 + dlay  );
      time2          = qRound( time0 + durat );
      double c_time  = time0;
      sp->time_first = time1;
      sp->time_last  = time2;
      double timeinc = ( time2 - time1 ) / (double)( sp->scans - 1 );
DbgLv(1) << "SimPar:MAIN:SetP:   sset" << jd << "time1 time2" << time1 << time2
 << "timeinc" << timeinc << "scans" << sp->scans << " c,s speed" << c_speed << s_speed;

DbgLv(1) << "SimPar:MAIN:SetP: accel-end:  time omega2t" << c_time << w2tsum;
      double w2tinc  = sq( s_speed * M_PI / 30.0 );
      double accel_time = 0.0;
      if ( sp->acceleration_flag && sp->acceleration != 0 )
      {
         accel_time = (s_speed - p_speed) / accel;
         double angular_accel = accel * M_PI / 30.0;

         double angular_previous_speed = p_speed * M_PI / 30.0;
         // add contribution of acceleration to w2tsum
         w2tsum += sq( angular_accel * accel_time ) * accel_time +
            angular_accel * angular_previous_speed * sq(accel_time) +
               sq(angular_previous_speed) * accel_time;
      }
      // add the contribution between end of acceleration and first scan
      w2tsum += w2tinc * (time1-time0-accel_time);
      DbgLv(1) << "SimPar:MAIN:SetP: 1st scan:   time omega2t" << c_time << w2tsum << "w2tinc" << w2tinc;
      c_speed = s_speed;
      sp->time_first = static_cast<int>(time1);
      sp->w2t_first  = w2tsum;
      w2tinc         = timeinc * sq( c_speed * M_PI / 30.0 );
      c_time         = time1 - timeinc;
      w2tsum         = w2tsum - w2tinc;
      DbgLv( 1 ) << "SimPar:MAIN:SetP: c_time w2tsum w2tinc timeinc" << c_time << w2tsum << w2tinc << timeinc << "time1" << time1 ;
      US_DataIO::Scan scandata;
      scandata.temperature = simparams.temperature;
      scandata.rpm         = c_speed;
      scandata.omega2t     = w2tsum;
      scandata.wavelength  = system.wavelength;
      scandata.plateau     = 0.0;
      scandata.delta_r     = simparams.radial_resolution;
      scandata.rvalues     .fill( 0.0, points   );

      for ( int js = 0; js < sp->scans; js++ )
      {  // Save scan times and omega2ts
         c_time           += timeinc;
         w2tsum           += w2tinc;
         int itime         = (int)qRound( c_time );
         scandata.seconds  = (double)itime;
         scandata.omega2t  = w2tsum;
         DbgLv( 2 ) << "SimPar:MAIN:SetP: js time omega2t " << js << scandata.seconds << scandata.omega2t;
         sim_data_all.scanData << scandata;
      }
      sp->time_last  = time2;
      sp->w2t_last   = w2tsum;
DbgLv(1) << "SimPar:MAIN:SetP: last scan:  time omega2t" << time2  << w2tsum;
   }  // End: loop to fill sim_data_all with times and omega2t's

   // Create a timestate for this speed profile set
   if ( !tmst_tfpath.isEmpty()  ||
        QFile( tmst_tfpath ).exists() )
   {  // Remove any previous temporary TMST
      QString tdef_tfpath = QString( tmst_tfpath )
                            .replace( ".tmst", ".xml" );
      QFile( tmst_tfpath ).remove();
      QFile( tdef_tfpath ).remove();
   }

   tmst_tfpath          = US_Settings::tmpDir() + "/p"
                          + QString::number( getpid() ) + "t"
                          + QDateTime::currentDateTime().toUTC()
                            .toString( "yyMMddhhmmss" )
                          + ".time_state.tmst";

   US_AstfemMath::writetimestate( tmst_tfpath, simparams, sim_data_all );

   simparams.simSpeedsFromTimeState( tmst_tfpath );
//*DEBUG*
DbgLv(1) << "ASIM: === speed steps from SSprof ===";
simparams.speedstepsFromSSprof();
simparams.debug();
DbgLv(1) << "ASIM: === speed steps from SSprof ===";
//*DEBUG*

   // Report simulation parameters
   change_status();
}

void US_Astfem_Sim::stop_simulation( void )
{
   stopFlag  = true;

   if ( astfem )
      astfem->setStopFlag( stopFlag );

   if ( astfvm )
      astfvm->setStopFlag( stopFlag );
}

// Adjusts meniscus and bottom position based on rotor coefficients
void US_Astfem_Sim::adjust_limits( double speed )
{
   double stretch_value        = stretch( simparams.rotorcoeffs, speed );
   af_params.current_meniscus  = meniscus_ar               + stretch_value;
   af_params.current_bottom    = simparams.bottom_position + stretch_value;
DbgLv(1) << "ASIM: adjlim: stretch currmen currbott"
 << stretch_value << af_params.current_meniscus << af_params.current_bottom;
}

// Calculates stretch for rotor coefficients array and rpm
double US_Astfem_Sim::stretch( double* rotorcoeffs, double speed )
{
   return ( rotorcoeffs[ 0 ] * speed
          + rotorcoeffs[ 1 ] * sq( speed ) );
}

// Start simulation
void US_Astfem_Sim::start_simulation( void )
{
//DbgLv(1) << "start_simulation is called";
   double current_time;  // Used for current time
   double delay;         // Acceleration time of the rotor from one speed to other
   double increment;     // Used to update omega_2_t in experimental grid
   double duration;      // Used for time left after acceleration zone

   int size_cv    = system.components.size();// Number of components in the system
   QVector< bool >  reactVec( size_cv );     // Boolean array showing reaction case between system

   //moviePlot->autoRefresh();

   moviePlot->replot();
   curve_count    = 0;
   image_count    = 0;
   int nstep      = simparams.speed_step.size();
   double rpm     = simparams.speed_step[ 0 ].rotorspeed; // Rotor speed for first speed step
   dataPlotClear( scanPlot );
   scanPlot ->setAxisScale( QwtPlot::xBottom, simparams.meniscus, simparams.bottom );
   scanPlot ->replot();

   pb_stop   ->setEnabled( true  );
   pb_start  ->setEnabled( false );
   pb_saveSim->setEnabled( false );

   // The astfem/astfvm simulation routines expects a dataset structure that
   // is initialized with a time and radius grid, and all concentration points
   // need to be set to zero. Each speed is a separate mfem_data set.
   sim_datas.resize( nstep );
DbgLv(1) << "start_simulation is called, steps:" << nstep;
   // Experimental grid setting starts from here
   for ( int jd = 0; jd < nstep; jd++ )
   {
      sim_datas[ jd ].xvalues .clear();
      sim_datas[ jd ].scanData.clear();
      sim_datas[ jd ].type[0]    = 'R';
      sim_datas[ jd ].type[1]    = 'A';

      QString guid = US_Util::new_guid();
      US_Util::uuid_parse( guid, (uchar*)sim_datas[ jd ].rawGUID );

      sim_datas[ jd ].cell        = 1;
      sim_datas[ jd ].channel     = 'S';
      sim_datas[ jd ].description = "Simulation";

      simparams.meniscus      = meniscus_ar;
      simparams.bottom        = simparams.bottom_position;

      // Update meniscus and bottom
      adjust_limits( (double)simparams.speed_step[ jd ].rotorspeed );
DbgLv(1) << "start_simulation is called" << af_params.current_meniscus << af_params.current_bottom
 << "step=" << jd << simparams.speed_step[jd].rotorspeed;

      // Number of radial grid points on the experimental grid; number of scans
      int points = qRound( ( af_params.current_bottom - af_params.current_meniscus ) /
                           simparams.radial_resolution ) + 1;
      // int points = qCeil( ( af_params.current_bottom - af_params.current_meniscus ) /
      //                     simparams.radial_resolution ) + 1;
      int nscans = simparams.speed_step[ jd ].scans;

      sim_datas[ jd ].xvalues.resize( points ); // Sets the size of radial grid points array

      for ( int jp = 0; jp < points; jp++ )
      {   // Calculate the radial grid points
         sim_datas[ jd ].xvalues[ jp ] = af_params.current_meniscus
                                         + jp * simparams.radial_resolution;
//DbgLv(1) << "radial_values" << sim_datas[jd].xvalues[jp] << jd;
      }
DbgLv(1) << "astfem_radial_ranges" << sim_datas[jd].xvalues[0] << sim_datas[jd].xvalues[points-1]
 << af_params.current_meniscus << af_params.current_bottom << jd;
//      sim_datas[ jd ].xvalues[ points - 1 ] = af_params.current_bottom;

      // Set the total size of scans for simulation
      sim_datas[ jd ].scanData.resize( nscans );

      int terpsize    = ( points + 7 ) / 8;

      // For each scan set the informations for each scan on
      // the experimental grid.
      int scan_index = 0;
      for ( int js = 0; js < nscans; js++ )
      {
         sim_datas[ jd ].scanData[ js ] = sim_data_all.scanData[ scan_index++ ];
         sim_datas[ jd ].scanData[ js ].wavelength  = system.wavelength;
         sim_datas[ jd ].scanData[ js ].plateau     = 0.0;
         sim_datas[ jd ].scanData[ js ].delta_r     = simparams.radial_resolution;
         sim_datas[ jd ].scanData[ js ].rvalues     .fill( 0.0, points   );
         sim_datas[ jd ].scanData[ js ].interpolated.fill( 0,   terpsize );
      }
   }

   // Initialize delay
   delay        = simparams.speed_step[ 0 ].delay_hours * 3600.0
                + simparams.speed_step[ 0 ].delay_minutes * 60.0;

   current_time        = 0.0;   // Initial time
   duration            = 0.0;   // Initialize total duration
   increment           = 0.0;   // Initial increment in time
//   int    scan_number  = 0;   // Counter for each scan and initialized to zero
   double acc_time;
//for ( int i =0; i< sim_datas.scanData.size();i++ )
// DbgLv(1)<<"time="<<sim_datas.scanData[i].seconds<<"omega2t="<< sim_datas.scanData[i].omega2t;

   lb_progress->setText( tr( "% Completed:" ) );
//   progress->setRange( 1, system.components.size() );
   progress->reset();
   lcd_component->display( 0 );
   lcd_scan->display(0);
   stopFlag         = false;

   simparams.mesh_radius.clear();
   simparams.sim    = true;
   simparams.firstScanIsConcentration = false;

   // Here we simulate on simulation grid and get
   // back the scans on desired grid i.e. either
   // experimental or simulation
   if ( simparams.meshType != US_SimulationParameters::ASTFVM )
   {  // the normal case:  ASTFEM (finite element)
      if ( system.associations.size() > 0 )
         lb_component->setText( tr( "RA Step:"   ) );
      else
         lb_component->setText( tr( "Component:" ) );

      if ( astfem )
      {
         astfem->disconnect();
         delete( astfem );
      }

      // make sure the selected model is adjusted for the selected temperature
      // and buffer conditions:
      // make a copy of the original system to correct s and D for visc and dens.
      // save original model unmodified, pass the density/viscosity corrected data
      // to astfem_rsa for simulating experimental space:
      //
      US_Model system_corrected = system;
      for ( int jc = 0; jc < system_corrected.components.size(); jc++ )
      {
         US_Math2::SolutionData sol_data;
         sol_data.density   = buffer.density;
         sol_data.viscosity = buffer.viscosity;
         sol_data.manual    = buffer.manual;
         double temp = simparams.temperature;
         sol_data.vbar20 = system_corrected.components.at(jc).vbar20;
         sol_data.vbar   = US_Math2::adjust_vbar20(sol_data.vbar20, temp);

         US_Math2::data_correction( simparams.temperature, sol_data );

         system_corrected.components[ jc ].s /= sol_data.s20w_correction;
         system_corrected.components[ jc ].D /= sol_data.D20w_correction;
      }

      astfem = new US_Astfem_RSA( system_corrected, simparams );

      connect( astfem, SIGNAL( new_scan( QVector< double >*, double* ) ),
                       SLOT( update_movie_plot( QVector< double >*, double* ) ) );
      connect( astfem, SIGNAL( current_component( int ) ),
                       SLOT  ( update_progress  ( int ) ) );
      connect( astfem, SIGNAL( new_time   ( double ) ),
                       SLOT  ( update_time( double ) ) );
      connect( astfem, SIGNAL( current_speed( int ) ),
                       SLOT  ( update_speed ( int ) ) );
      connect( astfem, SIGNAL( calc_progress( int ) ),
                       SLOT  ( show_progress( int ) ) );
      connect( astfem, SIGNAL( calc_done( void ) ),
                       SLOT  ( calc_over( void ) ) );

      astfem->set_movie_flag( ck_movie->isChecked() );
//      astfem->setStopFlag( stopFlag );
      astfem->set_buffer( buffer );
      astfem->set_debug_flag( dbg_level );
//DbgLv(1) << "after_calculate" << sim_datas.size();

      double bottom_ar    = simparams.bottom_position;
      simparams.meniscus  = meniscus_ar;
      simparams.bottom    = bottom_ar;
      int kscan           = sim_data_all.scanCount();
      int kpoint          = sim_data_all.pointCount();

      // Initialize reading values to zero for all scans (all speeds)
      for ( int js = 0; js < kscan; js++ )
         sim_data_all.scanData[ js ].rvalues.fill( 0.0, kpoint );

      // Set the radius values in data sets
      int points          = qRound( ( simparams.bottom - simparams.meniscus ) /
                                 simparams.radial_resolution ) + 1;
      // int points          = qCeil( ( simparams.bottom - simparams.meniscus ) /
      //                              simparams.radial_resolution ) + 1;
      sim_data_all.xvalues.resize( points );

      for ( int jd = 0; jd < nstep; jd++ )
      {  // Set radius values for current speed's dataset
         double stretch_fac  = stretch( simparams.rotorcoeffs,
                                        simparams.speed_step[ jd ].rotorspeed );
         simparams.meniscus  = meniscus_ar + stretch_fac;
         simparams.bottom    = bottom_ar   + stretch_fac;
         double radval       = simparams.meniscus;
         sim_datas[ jd ].xvalues.resize( points );

         for ( int jp = 0; jp < points; jp++ )
         {
            sim_datas[ jd ].xvalues[ jp ] = radval;
            // Set radius value in composite dataset (for multi-speed)
            if ( jd == 0 )
               sim_data_all.xvalues[ jp ] = radval;
            radval                       += simparams.radial_resolution;
         }
DbgLv(1) << "out:astfem_radial_ranges" << sim_datas[jd].xvalues[0] << sim_datas[jd].xvalues[points-1]
 << af_params.current_meniscus << af_params.current_bottom << jd;
      }

      // Set meniscus and bottom for (composite?) dataset to 1st speed range
      simparams.meniscus  = sim_data_all.xvalues[ 0 ];
      simparams.bottom    = sim_data_all.xvalues[ points - 1 ];

      // Compute the simulation dataset
      astfem->calculate( sim_data_all );
      calc_over();

      int ks              = 0;

      // Copy generated simulation dataset to any separate speed datasets
      for ( int jd = 0; jd < nstep; jd++ )
      {
         int nscans          = sim_datas[ jd ].scanCount();
         for ( int js = 0; js < nscans; js++, ks++ )
         {
            sim_datas[ jd ].scanData[ js ] = sim_data_all.scanData[ ks ];
         }
      }
   }
   else
   {
      if ( system.associations.size() > 0 )
         lb_component->setText( tr( "RA Step:"   ) );
      else
         lb_component->setText( tr( "Component:" ) );

      if ( astfvm )
      {
         astfvm->disconnect();
         delete( astfvm );
      }

      astfvm = new US_LammAstfvm( system, simparams );
      astfvm->set_buffer(buffer);
      connect( astfvm, SIGNAL( new_scan( QVector< double >*, double* ) ),
                       SLOT( update_movie_plot( QVector< double >*, double* ) ) );
      connect( astfvm, SIGNAL( comp_progress( int ) ),
                       SLOT  ( update_progress  ( int ) ) );
      connect( astfvm, SIGNAL( new_time   ( double ) ),
                       SLOT  ( update_time( double ) ) );
      connect( astfvm, SIGNAL( current_speed( int ) ),
                       SLOT  ( update_speed ( int ) ) );
      connect( astfvm, SIGNAL( calc_progress( int ) ),
                       SLOT  ( show_progress( int ) ) );
      connect( astfvm, SIGNAL( calc_done( void ) ),
                       SLOT  ( calc_over( void ) ) );
      double bottom_ar    = simparams.bottom_position;
      simparams.meniscus  = meniscus_ar;
      simparams.bottom    = bottom_ar;
      int kscan           = sim_data_all.scanCount();
      int kpoint          = sim_data_all.pointCount();

      // Initialize reading values to zero for all scans (all speeds)
      for ( int js = 0; js < kscan; js++ )
         sim_data_all.scanData[ js ].rvalues.fill( 0.0, kpoint );

      // Set the radius values in data sets
      int points          = qRound( ( simparams.bottom - simparams.meniscus ) /
                                  simparams.radial_resolution ) + 1;
      //int points          = qCeil( ( simparams.bottom - simparams.meniscus ) /
      //                             simparams.radial_resolution ) + 1;
      sim_data_all.xvalues.resize( points );
      for ( int jd = 0; jd < nstep; jd++ )
      {  // Set radius values for current speed's dataset
         double stretch_fac  = stretch( simparams.rotorcoeffs,
                                        simparams.speed_step[ jd ].rotorspeed );
         simparams.meniscus  = meniscus_ar + stretch_fac;
         simparams.bottom    = bottom_ar   + stretch_fac;
         double radval       = simparams.meniscus;
         for ( int jp = 0; jp < points; jp++ )
         {
            sim_datas[ jd ].xvalues[ jp ] = radval;
            // Set radius value in composite dataset (for multi-speed)
            if ( jd == 0 )
               sim_data_all.xvalues[ jp ] = radval;
            radval                       += simparams.radial_resolution;
         }
         DbgLv(1) << "out:astfem_radial_ranges" << sim_datas[jd].xvalues[0] << sim_datas[jd].xvalues[points-1]
                  << af_params.current_meniscus << af_params.current_bottom << jd;
      }

      // Set meniscus and bottom for (composite?) dataset to 1st speed range
      simparams.meniscus  = sim_data_all.xvalues[ 0 ];
      simparams.bottom    = sim_data_all.xvalues[ points - 1 ];
      astfvm->calculate( sim_data_all );
      calc_over();
      int ks              = 0;
      // Copy generated simulation dataset to any separate speed datasets
      for ( int jd = 0; jd < nstep; jd++ )
      {
         int nscans          = sim_datas[ jd ].scanCount();
         for ( int js = 0; js < nscans; js++, ks++ )
         {
            sim_datas[ jd ].scanData[ js ] = sim_data_all.scanData[ ks ];
         }
      }
   }

   finish();
}

void US_Astfem_Sim::finish( void )
{
   total_conc = 0.0;

   for ( int ii = 0; ii < system.components.size(); ii++ )
   {
      if ( ii != system.coSedSolute )
         total_conc += system.components[ ii ].signal_concentration;
   }

//DbgLv(1) << "FIN: comp size" << system.components.size();
//DbgLv(1) << "FIN:  total_conc" << total_conc;
   ri_noise();
   baseline();
   random_noise();
   ti_noise();

   // If we didn't interrupt, we need to set to 100 % complete at end of run
   if ( ! stopFlag )
   {
      //update_progress( progress->maximum() );
      update_progress( ncomponent );
DbgLv(1) << "FIN:  progress maxsize" << progress->maximum();
   }

   int jex  = simparams.speed_step.count() - 1;
   update_speed( (int)simparams.speed_step[ jex ].rotorspeed );

   stopFlag = false;
   if (movieFlag){scanPlot->detachItems();scanPlot->replot();}
   for ( int  i = 0; i < simparams.speed_step.size(); i++ )
   plot(i);

   pb_stop   ->setEnabled( false  );
   pb_start  ->setEnabled( true );
   pb_saveSim->setEnabled( true );

   if ( astfem )
   {
      delete astfem;
      astfem      = NULL;
   }

   if ( astfvm )
   {
      delete astfvm;
      astfvm      = NULL;
   }
}

void US_Astfem_Sim::ri_noise( void )
{
   csv_data_ri.clear();
   if ( simparams.rinoise == 0.0 ) return;


   QStringList header;
   header << "Time (s)" << "RI noise (OD)";

   // Add radially invariant noise
   for ( int jd = 0; jd < simparams.speed_step.size(); jd++ )
   {
      QVector<QVector<double>> csv_data;
      QVector<double> tv;
      QVector<double> rv;
      for ( int ks = 0; ks < sim_datas[ jd ].scanData.size(); ks++ )
      {
         double rinoise = US_Math2::box_muller( 0, total_conc * simparams.rinoise / 100 );
         tv << sim_datas[ jd ].scanData[ ks ].seconds;
         rv << rinoise;

         for ( int mp = 0; mp < sim_datas[ jd ].pointCount(); mp++ )
         {
            sim_datas[ jd ].scanData[ ks ].rvalues[ mp ] += rinoise;
         }
      }
      csv_data << tv;
      csv_data << rv;
      US_CSV_Data csv;
      csv.setData( header, csv_data );
      csv_data_ri << csv;
   }
}

void US_Astfem_Sim::baseline( void )
{
   if ( simparams.baseline == 0.0 ) return;

   // Add a constant baseline offset
   for ( int jd = 0; jd < simparams.speed_step.size(); jd++ )
   {
       for ( int ks = 0; ks < sim_datas[ jd ].scanData.size(); ks++ )
       {
           for ( int mp = 0; mp < sim_datas[ jd ].pointCount(); mp++ )
              sim_datas[ jd ].scanData[ ks ].rvalues[ mp ] += simparams.baseline;
       }
   }
}

void US_Astfem_Sim::random_noise( void )
{
   if ( simparams.rnoise == 0.0 && simparams.lrnoise == 0.0) return;
   // Add random noise
   if ( simparams.rnoise != 0.0 && simparams.lrnoise != 0.0)
   {  for ( int jd = 0; jd < simparams.speed_step.size(); jd++ )
      {
          for ( int j = 0; j < sim_datas[ jd ].scanData.size(); j++ )
          {
              for ( int k = 0; k < sim_datas[ jd ].pointCount(); k++ )
              {
                  sim_datas[ jd ].scanData[ j ].rvalues[ k ] +=
                  US_Math2::box_muller( 0, total_conc * simparams.rnoise / 100 ) + // based on total concentration
                  US_Math2::box_muller( 0, sim_datas[ jd ].scanData[ j ].rvalues[ k ] * simparams.lrnoise / 100 ); // based on local concentration
              }//'k' loop
          }//'j' loop
       }//'i' loop
   }
   if ( simparams.rnoise != 0.0  &&  simparams.lrnoise == 0.0 )
   {
      for ( int jd = 0; jd < simparams.speed_step.size(); jd++)
      {
          for ( int ks = 0; ks < sim_datas[ jd ].scanData.size(); ks++ )
          {
              for ( int mp = 0; mp < sim_datas[ jd ].pointCount(); mp++ )
              {
                  sim_datas[ jd ].scanData[ ks ].rvalues[ mp ] +=
                  US_Math2::box_muller( 0, total_conc * simparams.rnoise / 100 ); // based on total concentration
              }
          }
       }
   }
   if ( simparams.rnoise == 0.0 && simparams.lrnoise != 0.0)
   {  for ( int jd = 0; jd < simparams.speed_step.size(); jd++ )
      {
          for ( int ks = 0; ks < sim_datas[ jd ].scanData.size(); ks++ )
          {
              for ( int mp = 0; mp < sim_datas[ jd ].pointCount(); mp++ )
              {
                  sim_datas[ jd ].scanData[ ks ].rvalues[ mp ] +=
                  US_Math2::box_muller( 0, sim_datas[ jd ].scanData[ ks ].rvalues[ mp ] * simparams.lrnoise / 100 ); // based on local concentration
              }
          }
      }
   }
}

void US_Astfem_Sim::ti_noise( void )
{
   csv_data_ti.clear();
   if ( simparams.tinoise == 0.0 ) return;

   QVector< double > tinoise;
   // all speed steps are assumed to have the same number of the radial points of a single scan
   int points = sim_datas[0].pointCount();
   tinoise.resize( points );

   double val = US_Math2::box_muller( 0, total_conc * simparams.tinoise / 100 );
   for ( int mp = 0; mp < points; mp++ )
   {
      val += US_Math2::box_muller( 0, total_conc * simparams.tinoise / 100 );
      tinoise[ mp ] = val;
   }

   // Add time invariant noise
   for ( int jd = 0; jd < simparams.speed_step.size(); jd++ )
   {
      // int points = sim_datas[ jd ].pointCount();
      for ( int ks = 0; ks < sim_datas[ jd ].scanData.size(); ks++ )
      {
         for ( int mp = 0; mp < points; mp++ )
         {
            sim_datas[ jd ].scanData[ ks ].rvalues[ mp ] += tinoise[ mp ];
         }
      }
   }

   // save the TI into a csv file
   QDir dir( US_Settings::resultDir() );
   QStringList header{"Radial Points (cm)", "TI noise (OD)"};
   QVector<QVector<double>> csv_data;
   csv_data << sim_datas[0].xvalues;
   csv_data << tinoise;
   csv_data_ti.setData( header, csv_data );
}

void US_Astfem_Sim::save_scans( void )
{
DbgLv(1) << "ASIM:svscn: IN";
   QString odir        = QFileDialog::getExistingDirectory( this,
         tr( "Select a directory for the simulated data:" ),
         US_Settings::importDir() );
   if ( odir.isEmpty() ) {
      return;
   }
   QDir target_dir( odir );
   QDir parent_target_dir( odir );
   parent_target_dir.cdUp();
   QDir ultrascan_user_dir( US_Settings::workBaseDir() );
   // protect all ~/ultrascan/* locations
   if ( parent_target_dir.absolutePath() == ultrascan_user_dir.absolutePath() ) {
      QMessageBox::critical( this, tr( "Error" ), tr( "Cannot save at this location." ) );
      return;
   }
   if ( target_dir.exists() && !target_dir.isEmpty() ) {
      // Ask the user if really everything should be deleted
      QString text = tr( "The directory <b>%1</b> is not empty. Do you want to risk overwriting the content?" )
                     .arg( target_dir.absolutePath() );
      QMessageBox::StandardButton response = QMessageBox::question(
         this,
         tr( "Confirm" ),
         text,
         QMessageBox::Yes | QMessageBox::No );
      if ( response != QMessageBox::Yes ) {
         return;
      }
      target_dir.removeRecursively();
      target_dir.mkpath( target_dir.absolutePath() );
   }
   save_simulation( odir );
}

bool US_Astfem_Sim::save_simulation( QString odir, bool supress_dialog )
{
   QString run_id      = odir.section( "/", -1, -1 );
   int nstep           = simparams.speed_step.size();
   QString xdef_tfpath = QString( tmst_tfpath ).replace( ".tmst", ".xml" );
   bool have_tmst      = ( QFile( tmst_tfpath ).exists()  &&
                           QFile( xdef_tfpath ).exists() );

   if ( ! odir.isEmpty() )
   {  // The user gave a directory name, save in openAUC format
      odir               = odir.replace( "\\", "/" );
      simparams.sim      = true;

      if ( nstep == 1 )
      {  // Single-speed case
         QDir dir ( odir );

         save_xla( odir, sim_datas[ 0 ], 0, supress_dialog );

         // Create a timestate in the same directory
         QString tmst_fbase = run_id  + ".time_state.tmst";
         QString tmst_fpath = odir + "/" + tmst_fbase;
         QString xdef_fpath = QString( tmst_fpath ).replace( ".tmst", ".xml" );
DbgLv(1) << "ASIM:svscn: 1-speed file paths"  << odir << tmst_fpath;

         if ( have_tmst )
         {  // Copy temporary timestate files to imports subdirectory
            QFile::remove( tmst_fpath );
            QFile::remove( xdef_fpath );
            QFile::copy(   tmst_tfpath, tmst_fpath );
            QFile::copy(   xdef_tfpath, xdef_fpath );
         }

         else
         {  // Create timestate file pair in imports subdirectory
            US_AstfemMath::writetimestate( tmst_fpath, simparams, sim_datas[ 0 ] );
         }

         // Save TI noises
         if ( csv_data_ti.rowCount() > 0 ) {
            csv_data_ti.setFilePath( dir.absoluteFilePath( "ASTFEM_TI_NOISE.csv" ) );
            save_csv_noise( csv_data_ti );
         }
         // Save RI noises
         if ( !csv_data_ri.isEmpty() ) {
            csv_data_ri[ 0 ].setFilePath( dir.absoluteFilePath( "ASTFEM_RI_NOISE.csv" ) );
            save_csv_noise( csv_data_ri[ 0 ] );
         }

      }  // End:  single-speed case

      else
      {  // Multi-speed_case
DbgLv(1) << "ASIM:svscn: m-speed  have_tmst" << have_tmst;
         if ( have_tmst )
         {  // Copy temporary timestate file pairs to each speed's subdirectory
            //  and save AUC data there
            for ( int jd = 0; jd < nstep; jd++ )
            {
// x  x  x  x  x  x  x
               int ispeed          = simparams.speed_step[ jd ].rotorspeed;
               QString spsufx      = QString::asprintf( "-%05d", ispeed );
               QString run_id1     =  run_id + spsufx;
               QString odir1       =  odir   + spsufx;
               QString tmst_fpath1 =  odir1 + "/" + run_id1 + ".time_state.tmst";
               QString xdef_fpath1 =  odir1 + "/" + run_id1 + ".time_state.xml";
               QDir().mkpath( odir1 );

               save_xla( odir1, sim_datas[ jd ], jd, supress_dialog );

               QFile::remove( tmst_fpath1 );
               QFile::remove( xdef_fpath1 );
               QFile::copy(   tmst_tfpath, tmst_fpath1 );
               QFile::copy(   xdef_tfpath, xdef_fpath1 );
            }
         }

         else
         {  // Create timestate file pairs in first speed's subdirectory,
            //  copy them to the other each speed's subdirectory,
            //  and save AUC data in all
            int ispeed          = simparams.speed_step[ 0 ].rotorspeed;
            QString spsufx      = QString::asprintf( "-%05d", ispeed );
            QString run_id1     =  run_id + spsufx;
            QString odir1       =  odir   + spsufx;
            QString tmst_fpath1 =  odir1 + "/" + run_id1 + ".time_state.tmst";
            QString xdef_fpath1 =  odir1 + "/" + run_id1 + ".time_state.xml";
            QDir().mkpath( odir1 );

            US_AstfemMath::writetimestate( tmst_fpath1, simparams, sim_datas[ 0 ] );

            for ( int jd = 0; jd < nstep; jd++ )
            {
               ispeed              = simparams.speed_step[ jd ].rotorspeed;
               spsufx              = QString::asprintf( "-%05d", ispeed );
               QString run_id2     =  run_id + spsufx;
               QString odir2       =  odir   + spsufx;
               QDir().mkpath( odir2 );

               save_xla( odir2, sim_datas[ jd ], jd );

               if ( jd > 0 )
               {
                  QString tmst_fpath2 =  odir2 + "/" + run_id2 + ".time_state.tmst";
                  QString xdef_fpath2 =  odir2 + "/" + run_id2 + ".time_state.xml";

                  QFile::remove( tmst_fpath2 );
                  QFile::remove( xdef_fpath2 );
                  QFile::copy(   tmst_fpath1, tmst_fpath2 );
                  QFile::copy(   xdef_fpath1, xdef_fpath2 );
               }
            }
         }

         // Save TI, RI noises

         for ( int ii = 0; ii < nstep; ii++ )
         {
            int ispeed          = simparams.speed_step[ ii ].rotorspeed;
            QString spsufx      = QString::asprintf( "-%05d", ispeed );
            QString odir1       =  odir   + spsufx;
            QDir dir ( odir1 );
            csv_data_ti.setFilePath( dir.absoluteFilePath( "ASTFEM_TI_NOISE.csv" ) );
            csv_data_ri[ ii ].setFilePath( dir.absoluteFilePath( "ASTFEM_RI_NOISE.csv" ) );
            save_csv_noise( csv_data_ti );
            save_csv_noise( csv_data_ri[ ii ] );
         }

//*DEBUG*
int kscn=sim_data_all.scanCount();
DbgLv(1) << "ASIM:svscn:  all_data scan count" << kscn << sim_data_all.scanData.count();
for(int ss=0; ss<kscn; ss++ )
 DbgLv(1) << "ASIM:svscn:  scan" << (ss+1) << "time" << sim_data_all.scanData[ss].seconds;
//*DEBUG*
      }  // End:  multi-speed case
   }  // End:  output directory specified
   if ( !supress_dialog )
   {
      return true;
   }
   // write experiment file
   QRegularExpression rx( "^[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}$" );
   if ( odir.right( 1 ) != "/" )
   {
      odir += "/"; // Ensure trailing /
   }
   QString runType = QString( QChar( sim_datas[0].type[ 0 ] ) )
                     + QString( QChar( sim_datas[0].type[ 1 ] ) );
   QString writeFile = run_id      + "."
                     + runType    + ".xml";
   QFile file( odir + writeFile );
   QString experiment_GUID = US_Util::new_guid();
   if ( !file.open( QIODevice::WriteOnly | QIODevice::Text) )
   {
      return false ;
   }

   QXmlStreamWriter xml;
   xml.setDevice( &file );
   xml.setAutoFormatting( true );

   xml.writeStartDocument();
   xml.writeDTD("<!DOCTYPE US_Scandata>");
   xml.writeStartElement("US_Scandata");
   xml.writeAttribute("version", "1.0");

   // elements
   // experiment
   xml.writeStartElement( "experiment" );
   xml.writeAttribute   ( "id",   QString::number( 0 ) );
   xml.writeAttribute   ( "guid", experiment_GUID );
   xml.writeAttribute   ( "type",  QString( "velocity" ) );
   xml.writeAttribute   ( "runID", run_id );
   // investigator
   xml.writeStartElement( "investigator" );
   xml.writeAttribute   ( "id", QString::number( US_Settings::us_inv_ID(  ) ) );
   xml.writeAttribute   ( "guid", QString( "" ) );
   xml.writeEndElement  ();

   xml.writeStartElement( "name" );
   xml.writeAttribute   ( "value", US_Settings::us_inv_name(  ) );
   xml.writeEndElement  ();

   xml.writeStartElement( "project" );
   xml.writeAttribute   ( "id", QString::number( 0 ) );
   xml.writeAttribute   ( "guid", "" );
   xml.writeAttribute   ( "desc", "Simulation" );
   xml.writeEndElement  ();
   int lab_id = 1;
   int instrument_id = 1;
   QString instrument_serial = "";

   xml.writeStartElement( "lab" );
   xml.writeAttribute   ( "id",   QString::number( 1 ) );
   xml.writeEndElement  ();

   xml.writeStartElement( "instrument" );
   xml.writeAttribute   ( "id",     QString::number( 1 ) );
   xml.writeAttribute   ( "serial", "" );
   xml.writeEndElement  ();

   xml.writeStartElement( "operator" );
   xml.writeAttribute   ( "id", QString::number( 0 ) );
   xml.writeAttribute   ( "guid", "" );
   xml.writeEndElement  ();

   xml.writeStartElement( "rotor" );
   xml.writeAttribute   ( "id",     QString::number( rotor.ID   ) );
   xml.writeAttribute   ( "guid",   rotor.GUID );
   xml.writeAttribute   ( "serial", rotor.serialNumber );
   xml.writeAttribute   ( "name", rotor.name );
   xml.writeEndElement  ();

   xml.writeStartElement( "calibration" );
   xml.writeAttribute   ( "id",     simparams.rotorCalID );
   xml.writeAttribute   ( "coeff1", QString::number( simparams.rotorcoeffs[0] ) );
   xml.writeAttribute   ( "coeff2", QString::number( simparams.rotorcoeffs[1] ) );
   xml.writeAttribute( "date", "2019-01-01" );
   xml.writeEndElement  ();

   int     psolID    = -1;
   QString psolGUID  = "";
   QString psolDesc  = "";
   qDebug() << "  EsTD: triples loop" << sim_datas.size();
   // find if a solution for this already exists
   US_Solution sol = US_Solution();
   sol.buffer = buffer;
   sol.analyteInfo.clear();
   for ( int i = 0; i < system.components.size(); i++ )
   {
      auto comp = system.components[ i ];
      auto analyte = US_Analyte();
      analyte.extinction.clear();
      analyte.type = static_cast<US_Analyte::analyte_t>(comp.analyte_type);
      analyte.vbar20 = comp.vbar20;
      analyte.mw = comp.mw;
      analyte.description = comp.name;
      analyte.analyteGUID = comp.analyteGUID;
      // analyte.sequence has no corresponding SimulationComponent property
      analyte.grad_form = system.coSedSolute == i;
      analyte.extinction[system.wavelength] = comp.extinction;
      auto analyteInfo = US_Solution::AnalyteInfo();
      analyteInfo.analyte = analyte;
      analyteInfo.amount = comp.signal_concentration;
      if ( i > 0 )
      {
         psolDesc += " ";
      }
      psolDesc += comp.name;
      sol.analyteInfo.append(analyteInfo);
   }
   psolDesc += " | " + buffer.description;
   sol.solutionDesc = psolDesc;
   sol.saveToDisk(  );


   // loop through the following for c/c/w combinations
   for ( int trx = 0; trx < sim_datas.size(); trx++ )
   {
      auto trp = sim_datas[ trx ];

      QString triple     = trp.description;
      QStringList parts  = triple.split(" / ");

      QString cell       = QString::number( trp.cell );
      QString channel    = QString( trp.channel );
      QString wl         = QString::number( trp.scanData.first().wavelength );
      wl     = ( trp.scanData.first().wavelength < 99 ) ? "123" : wl;

      xml.writeStartElement( "dataset" );
      xml.writeAttribute   ( "id",      QString::number( trx + 1 ) );
      xml.writeAttribute   ( "guid",    US_Util::uuid_unparse(reinterpret_cast<uchar*>(trp.rawGUID)) );
      xml.writeAttribute   ( "cell",    cell );
      xml.writeAttribute   ( "channel", channel );

      if ( runType == "WA" )
      {
         xml.writeAttribute( "radius", wl );
      }
      else
      {
         xml.writeAttribute( "wavelength", wl );
      }

      xml.writeStartElement( "centerpiece" );
      xml.writeAttribute   ( "id", QString::number( 1 ) );
      xml.writeEndElement  ();

      xml.writeStartElement( "solution" );
      xml.writeAttribute   ( "id",   QString::number( sol.solutionID ) );
      xml.writeAttribute   ( "guid", sol.solutionGUID );
      xml.writeAttribute   ( "desc", sol.solutionDesc );
      xml.writeEndElement  ();

      xml.writeEndElement  ();
   }

   for ( int jj = 0; jj < simparams.speed_step.count(); jj++ )
   {
      US_SimulationParameters::speedstepToXml( xml, &simparams.speed_step[ jj ] );
   }

   xml.writeStartElement( "opticalSystem" );
   xml.writeAttribute   ( "value", runType  );
   xml.writeEndElement  ();

   xml.writeStartElement( "date" );
   xml.writeAttribute   ( "value", "" );
   xml.writeEndElement  ();

   xml.writeStartElement( "runTemp" );
   xml.writeAttribute   ( "value", QString::number(simparams.temperature) );
   xml.writeEndElement  ();

   xml.writeTextElement ( "label", run_id );
   xml.writeTextElement ( "comments", "Auto exported" );
   xml.writeTextElement ( "protocolGUID", "" );

   xml.writeEndElement(); // US_Scandata
   xml.writeEndDocument();
   QString editGUID = US_Util::new_guid();
   QString rawGUID = US_Util::uuid_unparse(reinterpret_cast<uchar*>(sim_datas[0].rawGUID));
   QString cell       = QString::number( sim_datas[0].cell );
   QString channel    = QString( sim_datas[0].channel );
   QString wl         = QString::number( sim_datas[0].scanData.first().wavelength );
   wl     = ( sim_datas[0].scanData.first().wavelength < 99 ) ? "123" : wl;

   QString now  =  QDateTime::currentDateTime()
                      .toUTC().toString( "yyMMddhhmm" );
   QString fname = run_id + "." + now + "." + runType + "." + cell + "." + channel + "." + wl + ".xml";
   QFile efo( odir + fname );


   if ( ! efo.open( QFile::WriteOnly | QFile::Text ) )
   {
      QMessageBox::information( this,
            tr( "File write error" ),
            tr( "Could not open the file\n" ) + odir + fname
            + tr( "\n for writing.  Check your permissions." ) );
      return 1;
   }

   xml.setDevice( &efo );
   xml.setAutoFormatting( true );

   xml.setAutoFormatting( true );
   xml.writeStartDocument();
   xml.writeDTD         ( "<!DOCTYPE UltraScanEdits>" );
   xml.writeStartElement( "experiment" );
   xml.writeAttribute   ( "type", "Velocity" );

   // Write identification
   xml.writeStartElement( "identification" );

   xml.writeStartElement( "runid" );
   xml.writeAttribute   ( "value", run_id );
   xml.writeEndElement  ();

   xml.writeStartElement( "editGUID" );
   xml.writeAttribute   ( "value", US_Util::new_guid() );
   xml.writeEndElement  ();

   xml.writeStartElement( "rawDataGUID" );
   xml.writeAttribute   ( "value", rawGUID );
   xml.writeEndElement  ();

   xml.writeEndElement  ();  // identification




DbgLv(1) << "EDT:WrXml:  waveln" << wl;

   xml.writeStartElement( "run" );
   xml.writeAttribute   ( "cell",       cell    );
   xml.writeAttribute   ( "channel",    channel );
   xml.writeAttribute   ( "wavelength", wl  );


   // Write meniscus, range, plateau, baseline, odlimit
   xml.writeStartElement( "parameters" );


   xml.writeStartElement( "meniscus" );
   xml.writeAttribute   ( "radius",
      QString::number( af_params.current_meniscus, 'f', 8 ) );
   xml.writeEndElement  ();
   xml.writeStartElement( "bottom" );
   xml.writeAttribute   ( "radius",
      QString::number( af_params.current_bottom, 'f', 8 ) );
   xml.writeEndElement  ();

   xml.writeStartElement( "data_range" );
   xml.writeAttribute   ( "left",
      QString::number( af_params.current_meniscus + 0.0005,  'f', 8 ) );
   xml.writeAttribute   ( "right",
      QString::number( af_params.current_bottom - 0.1, 'f', 8 ) );
   xml.writeEndElement  ();

   xml.writeStartElement( "plateau" );
   xml.writeAttribute   ( "radius",
      QString::number( af_params.current_bottom - 0.3,  'f', 8 ) );
   xml.writeEndElement  ();

   xml.writeStartElement( "baseline" );
   xml.writeAttribute   ( "radius",
      QString::number( af_params.current_meniscus + 0.0055, 'f', 8 ) );
   xml.writeEndElement  ();
   double maxc        = 0.0;
   int    total_scans = sim_datas[0].scanCount();
   int    old_points  = sim_datas[0].pointCount();

   for ( int ii = 0; ii < total_scans; ii++ )
   {  // Accumulate the maximum computed OD value
      for ( int kk = 0; kk < old_points; kk++ )
         maxc = qMax( maxc, sim_datas[0].value( ii, kk ) );
   }
   xml.writeStartElement( "od_limit" );
   xml.writeAttribute   ( "value",
      QString::number( maxc,  'f', 8 ) );
   xml.writeEndElement  ();


   xml.writeEndElement  ();  // parameters

   xml.writeEndElement  ();  // run
   xml.writeEndElement  ();  // experiment
   xml.writeEndDocument ();

   efo.close();

   return true;
}

void US_Astfem_Sim::save_csv_noise( US_CSV_Data &csv )
{
   if ( csv.columnCount() != 2 ) return;
   csv.writeFile( "," );
}

// slot to update progress and lcd based on current component
void US_Astfem_Sim::update_progress( int component )
{
   if ( component == -1 )
   {  // -1 component flags reset to maximum
//      progress->setValue( progress->maximum() );
      lcd_component->setMode( QLCDNumber::Hex );
      lcd_component->display( "rA " );
   }

   else if ( component < 0 )
   {  // other negative component flags set maximum
//      progress->setRange( 1, -component );
      lcd_component->setMode( QLCDNumber::Dec );
      lcd_component->display( 0 );
   }

   else
   {  // normal call sets progress value
//      progress->setValue( component );
      icomponent         = component;
      lcd_component->setMode( QLCDNumber::Dec );
      lcd_component->display( icomponent );
      scanPlot->detachItems();
      scanPlot->replot();
      curve_count = 0;
   }
}

void US_Astfem_Sim::save_xla( const QString& dirname, US_DataIO::RawData sim_data, int i1, bool supress_dialog )
{

DbgLv(1) << "save_xla_is_called";
//   double mrad      = simparams.meniscus;
//   double brad      = simparams.bottom;
   simparams.meniscus = meniscus_ar;
   simparams.bottom   = simparams.bottom_position;

   adjust_limits( (double)simparams.speed_step[ i1 ].rotorspeed );
//   double mrad      = sim_data.xvalues[ 0 ];
//   double brad      = sim_data.xvalues[ sim_data.xvalues.size()-1 ];

   double mrad      = af_params.current_meniscus;
   double brad      = af_params.current_bottom;
   double grid_res  = simparams.radial_resolution;

DbgLv(1) << "save_xla_meniscus_bottom" <<  mrad << brad;

   // Add 30 points in front of meniscus
   int    points      = (int)( ( brad - mrad ) / grid_res ) + 31;

   double maxc        = 0.0;
   int    total_scans = sim_data.scanCount();
   int    old_points  = sim_data.pointCount();

   for ( int ii = 0; ii < total_scans; ii++ )
   {  // Accumulate the maximum computed OD value
      for ( int kk = 0; kk < old_points; kk++ )
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
DbgLv(1) << "Sim:SV: slope intercept timecorr" << slope << intercept
 << timecorr << "x0 xn y0 yn" << xtmp[0] << xtmp[total_scans-1]
 << ytmp[0] << ytmp[total_scans-1] << "total_scans" << total_scans;

   for ( int jc = 0; jc < system.components.count(); jc++ )
   {
      US_Model::SimulationComponent* sc = &system.components[ jc ];
      double conc     = sc->signal_concentration;
      double sval     = sc->s;
      s1plat         += ( conc * exp( oterm * sval ) );
   }

   double dthresh  = maxc;
DbgLv(1) << "Sim:SV: maxc" << maxc << "s1plat" << s1plat
   << "dthresh" << dthresh << "total_conc" << total_conc;
   double maxrad   = brad;
   s1plat          = qMin( s1plat, ( dthresh * 0.5 ) );
DbgLv(1) << "Sim:SV: reset s1plat" << s1plat;
   if ( !supress_dialog )
   {
      US_ClipData* cd = new US_ClipData( dthresh, maxrad, mrad, total_conc );

      if ( ! cd->exec() )
      {
         maxrad         = brad;
         dthresh        = maxc;
      }
   } else
   {
      dthresh = total_conc * 100.0;
   }


   // If the overall maximum reading exceeds the threshold,
   //  limit OD values in all scans to the threshold
   if ( maxc > dthresh )
   {
      int nchange     = 0;  // Total threshold changes
      int nmodscn     = 0;  // Modified scans


      for ( int js = 0; js < total_scans; js++ )
      {  // Examine each scan
         int kchange  = 0;  // Changes in a scan

         for ( int jp = 0; jp < old_points; jp++ )
         {  // Examine each reading point in a scan

            if ( sim_data.value( js, jp ) > dthresh )
            {  // Set a high value to the threshold and bump counts
               sim_data.setValue( js, jp, dthresh );
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
      if (!supress_dialog)
      {
         QMessageBox::information( this,
      tr( "OD Values Threshold Limited" ),
      tr( "%1 readings in %2 scans were reset\n"
          "to a threshold value of %3 .\n"
          "The pre-threshold-limit maximum OD\n"
          "value was %4 ." )
      .arg( nchange ).arg( nmodscn ).arg( dthresh ).arg( maxc ) );
      }
   }


   points             = (int)( ( maxrad - mrad ) / grid_res ) + 31;

   progress->setRange( 1, total_scans );
   progress->reset();

   QVector< double > tconc_v( points );
   double* temp_conc  = tconc_v.data();
   double  rad        = mrad - 30.0 * grid_res;
   sim_data.xvalues.resize( points );
   lb_progress->setText( "Writing..." );

   for ( int jp = 0; jp < points; jp++ )
   {
      sim_data.xvalues[ jp ] = rad;
      rad  += grid_res;
   }

   for ( int js = 0; js < total_scans; js++ )
   {
      US_DataIO::Scan* scan = &sim_data.scanData[ js ];

      for ( int jp = 30; jp < points; jp++ )
      {  // Position the computed concentration values after the first 30
         temp_conc[ jp ] = scan->rvalues[ jp - 30 ];
      }

      for ( int jp = 0; jp < 30; jp++ )
      {  // Zero the first 30 points
         temp_conc[ jp ] = 0.0;
      }

      temp_conc[ 30 ] = s1plat * 2.0;   // Put a spike at the meniscus

      scan->rvalues.resize( points );

      for ( int jp = 0; jp < points; jp++ )
      {  // Store the values: first 30 then computed values
         scan->rvalues[ jp ] = temp_conc[ jp ];
      }

      progress->setValue( ( js + 1 ) );
//DbgLv(2) << "WD:sc secs" << scan->seconds;
//if ( ii == 0 || (ii+1) == total_scans ) {
//DbgLv(2) << "WD:S0:c00" << scan->rvalues[0];
//DbgLv(2) << "WD:S0:c01" << scan->rvalues[1];
//DbgLv(2) << "WD:S0:c30" << scan->rvalues[30];
//DbgLv(2) << "WD:S0:cn1" << scan->rvalues[points-2];
//DbgLv(2) << "WD:S0:cnn" << scan->rvalues[points-1]; }
   }

   QString run_id    = dirname.section( "/", -1, -1 );

DbgLv(1) << "Sim:SV:  run_id_from_save_xla" << run_id;
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
DbgLv(1) << "Sim:SV: after_write_rawdata" << ofname;
   progress->setValue( total_scans );
   lb_progress->setText( tr( "Completed" ) );
}


//int   scan_count = exp_data[speed_step].scanCount();
//          points     = exp_data[speed_step].pointCount();
void US_Astfem_Sim::plot( int step )
{
   if (movieFlag)scanPlot->detachItems();// remove plots from movie
   QList< QColor > mcolors;
   int nmcols     = plot2->map_colors( mcolors );
  // dataPlotClear( scanPlot );
   double min_y_axis = 0.0;
   double max_y_axis = total_conc * 2.0;

   // Set plot scale for band-forming
   if ( simparams.band_forming )
   {
      min_y_axis = total_conc;
   }
   // adjust the plotting for the baseline offset if defined
   if ( simparams.baseline != 0.0)
   {
      min_y_axis += simparams.baseline;
      max_y_axis += simparams.baseline;
   }
   // For co-sedimenting solutes axis are auto-scaled
   if ( system.coSedSolute >= 0 )
   {
      scanPlot->setAxisAutoScale( QwtPlot::yLeft   );
   }
   else
   {
      scanPlot->setAxisScale( QwtPlot::yLeft, min_y_axis, max_y_axis );
   }

   QwtPlotGrid* grid2 = us_grid( scanPlot );
   grid2->enableX(    true );
   grid2->enableY(    true );

   // Walk scan index through previous steps
   int scanx        = 0;
   for ( int jj = 0; jj < step; jj++ )
      scanx           += sim_datas[ jj ].scanCount();

   // Plot the simulation
   if ( ! stopFlag )
    {
      int   scan_count = sim_datas[ step ].scanCount();
      int   points     = sim_datas[ step ].pointCount();
      int*  curve      = new int[ scan_count ];

      double*  x;
      double** y;

      x = new double  [ points ];
      y = new double* [ scan_count ];

      for ( int j = 0; j < points; j++ )
         x[ j ] = sim_datas[ step ].xvalues[ j ];

      for ( int j = 0; j < scan_count; j++ )
         y[ j ] = new double [ points ];

      for ( int j = 0; j < scan_count; j++ )
      {
         for ( int k = 0; k < sim_datas[step].scanData[j].rvalues.size(); k++ )
            y[ j ][ k ] = sim_datas[ step ].value( j, k );
      }

      for ( int j = 0; j < scan_count; j++ )
      {
         QString title = "Concentration" + QString::number( scanx+1 );
         QwtPlotCurve* plotCurve = new QwtPlotCurve( title );

         if ( nmcols > 0 )
            plotCurve->setPen ( QPen( mcolors[ scanx % nmcols ] ) );
         else
            plotCurve->setPen ( QPen( Qt::yellow ) );
         plotCurve->attach    ( scanPlot );
         plotCurve->setSamples( x, y[ j ], points );
         scanx++;
      }

      delete [] x;

      for ( int j = 0; j < scan_count; j++ ) delete [] y[ j ];
      delete [] y;

      delete [] curve;
      scanPlot->replot();
    }
}

// slot to update lcd based on current component
void US_Astfem_Sim::update_component( int component )
{
   lcd_component->setMode( QLCDNumber::Dec );
   lcd_component->display( component );
   scanPlot->detachItems();
   scanPlot->replot();
   curve_count = 0;
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

   progress   ->setRange( 1, steps );
   progress   ->reset();
   lb_progress->setText( tr( "Calculating..." ) );
}

// slot to set progress to maximum
void US_Astfem_Sim::calc_over( void )
{
//   progress   ->setRange( 1, progress_maximum );
//   progress   ->setValue  ( progress_value );
   progress_value   = progress->maximum();
   //progress_value   = ncomponent * 100;
   progress   ->setValue  ( progress_value );
   lb_progress->setText( progress_text );
DbgLv(1) << "ASIM: over: progmax" << progress_value << "progress_text" << progress_text;
}

// slot to update movie plot
void US_Astfem_Sim::update_movie_plot( QVector< double >* x, double* c )
{
   //moviePlot->detachItems();
   double total_c = 0.0;
   double yscale  = 0.0;

   if ( simparams.meshType != US_SimulationParameters::ASTFVM  ||
        system.coSedSolute < 0 )
   {  // Get total concentration of all components
      for ( int ii = 0; ii < system.components.size(); ii++ )
         total_c += system.components[ ii ].signal_concentration;

      yscale         = total_c * 3.0;
   }
   else
   {  // Get total concentration of non-cosed components
      for ( int ii = 0; ii < system.components.size(); ii++ )
      {
         if ( ii != system.coSedSolute )
            total_c += system.components[ ii ].signal_concentration;
      }

      yscale         = total_c * 7.0;
   }
   QwtPlotGrid* grid2 = us_grid( moviePlot );
   grid2->enableX(    true );
   grid2->enableY(    true );
   //moviePlot->setAxisScale( QwtPlot::yLeft, 0, yscale );
   moviePlot->setAxisAutoScale( QwtPlot::yLeft );
   moviePlot->setAxisScale( QwtPlot::xBottom, simparams.meniscus, simparams.bottom );

   double* r = new double [ x->size() ];

   for ( int i = 0; i < x->size(); i++ ) r[ i ] = (*x)[ i ];

   QwtPlotCurve* curve =
      new QwtPlotCurve( "Scan Number " + QString::number( ++curve_count ) );

   curve->setPen    ( QPen( US_GuiSettings::plotCurve(), 3 ) );
   curve->setSamples(r, c, x->size() );
   curve->attach ( moviePlot );

   moviePlot->replot();

   if ( save_movie )
   {
      QPixmap pmap;
      image_count++;
      imageName = imagedir + QString::asprintf( "frame%05d.png", image_count );
      US_GuiUtil::save_png( imageName, moviePlot );
   }
   if(dbg_level>0){
      bool x_needed = sim_datas[ 0 ].xvalues.isEmpty();
      // dataPlotClear( scanPlot );
      QList< QColor > mcolors;
      int nmcols     = plot2->map_colors( mcolors );



      // Set plot scale
      if ( simparams.band_forming ) {
          scanPlot->setAxisAutoScale(QwtPlot::yLeft);
      }
      else if ( system.coSedSolute >= 0 )
      {
         scanPlot->setAxisAutoScale( QwtPlot::yLeft );
      }

      else
      {
         scanPlot->setAxisAutoScale( QwtPlot::yLeft );
      }

      QwtPlotGrid* grid2 = us_grid( scanPlot );
      grid2->enableX(    true );
      grid2->enableY(    true );


      // Walk scan index through previous steps
      int scanx        = curve_count;


      // Plot the simulation scan
      QString title = "Concentration" + QString::number( scanx );
      QwtPlotCurve* plotCurve = new QwtPlotCurve( title );

      if ( nmcols > 0 )
         plotCurve->setPen ( QPen( mcolors[ scanx % nmcols ] ) );
      else
         plotCurve->setPen ( QPen( Qt::yellow ) );
      plotCurve->setSamples( r, c, x->size() );
      plotCurve->attach    ( scanPlot );
      scanPlot->replot();
      scanPlot->detachItems();
   }
   qApp->processEvents();
//int k=x->size()-1;
//int h=x->size()/2;
//DbgLv(1) << "UpdMovie: r0 rh rn c0 ch cn" << r[0] << r[h] << r[k]
//   << c[0] << c[h] << c[k];

   delete [] r;
   moviePlot->detachItems();

}

// slot to update movie plot
void US_Astfem_Sim::update_save_movie( bool ckd )
{
   save_movie   = ckd;

   if ( save_movie )
   {
      imagedir   = QFileDialog::getExistingDirectory( this,
            tr( "Select or create a movie frames directory" ),
            US_Settings::tmpDir() );
      if ( imagedir.isEmpty() )
      {
         imagedir   = US_Settings::tmpDir() + "/movies";
         QDir().mkpath( imagedir );
      }

      if ( ! imagedir.endsWith( "/" ) )
         imagedir   = imagedir + "/";
   }

}

void US_Astfem_Sim::update_time( double time )
{
   static int kstep = 0;
   double dtime = qRound( time );
   lcd_time ->display( dtime );
   curve_count++;
   lcd_scan->display(curve_count);
   int tstep    = (int)qRound( dtime * 100.0 / times_comp );
//   tstep        = ( icomponent - 1 ) * 100 + tstep;
//   if ( tstep != kstep )
//   {
//      show_progress( tstep );
//DbgLv(1) << "ASIM: upd_tm: tstep" << tstep << "dtime" << dtime << "icomponent" << icomponent
// << "times_comp" << times_comp;
//   }
   kstep        = tstep;
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
   qDebug() << "lrnoise " << simparams.lrnoise;
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
#if 0
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
#endif
}

void US_Astfem_Sim::dump_mfem_scan( US_DataIO::Scan& /*ms*/ )
{
#if 0
   qDebug() << "mfem_scan----";
   qDebug() << "time " << ms.time;
   qDebug() << "omega_s_t " << ms.omega_s_t;
   qDebug() << "rpm " << ms.rpm;
   qDebug() << "temperature " << ms.temperature;
   qDebug() << "time " << ms.time;
   qDebug() << "conc list size " << ms.conc.size();
   //qDebug() << "conc " << ms.conc;
#endif
}

