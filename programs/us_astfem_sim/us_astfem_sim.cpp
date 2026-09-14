//! \file us_astfem_sim.cpp

#include <QApplication>

#include "us_license.h"
#include "us_license_t.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_gui_util.h"
#include "us_headless_cli.h"
#include "us_astfem_sim.h"
#include "us_sim_params_gui.h"
#include "us_math2.h"
#include "us_astfem_math.h"
#include "us_sim_inputs.h"
#include "us_hardware.h"
#include "us_defines.h"
#include "us_clipdata.h"
#include "us_rotor_gui.h"
#include "us_db2.h"
#include "us_gui_util.h"
#include "us_model_gui.h"
#include "us_buffer_gui.h"
#include "us_util.h"
#include "us_experiment.h"
#include "us_convert.h"
#include "us_sim_record.h"
#include "us_astfem_rsa.h"
#include "us_lamm_astfvm.h"
#include "us_time_state.h"

//! \brief Main program for US_Astfem_Sim.  Loads translators and starts
//! the class US_Astfem_Sim.
#ifndef US_ASTFEM_SIM_NO_MAIN
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
      "Display command-line help");
   parser.addOption(help_option);
   auto version_option = parser.addVersionOption();
   auto model_option = QCommandLineOption("model",
      "Load a model from a file path, GUID, or database ID",
      "model");
   parser.addOption(model_option);
   auto buffer_option = QCommandLineOption("buffer",
      "Load a buffer from a file path, GUID, or database ID",
      "buffer");
   parser.addOption(buffer_option);
   auto sim_parameters_option = QCommandLineOption("simparams",
      "Load simulation parameters from a file path",
      "simparams");
   parser.addOption(sim_parameters_option);
   auto rotor_option = QCommandLineOption("rotor",
      "Load a rotor from a file path, GUID, or database ID",
      "rotor");
   parser.addOption(rotor_option);
   auto centerpiece_option = QCommandLineOption("centerpiece",
      "Centerpiece-list index used for channel geometry (default: 0). "
      "Overrides the bottom position recovered from --simparams, which is the "
      "only way to select a row of a multi-row centerpiece. Indexes the local "
      "etc/abstractCenterpieces.xml, never the database, so that a given index "
      "means the same thing on every machine",
      "index");
   parser.addOption(centerpiece_option);
   auto centerpiece_channel_option = QCommandLineOption("centerpiece-channel",
      "Row within the centerpiece (default: 0), given either as a channel "
      "letter (A-H, where a channel and its reference share a row, so A and B "
      "are row 0) or as a bare row index",
      "channel");
   parser.addOption(centerpiece_channel_option);
   auto movie_option = QCommandLineOption("movie",
      "Show the simulation as a movie");
   parser.addOption(movie_option);
   auto time_correction_option = QCommandLineOption("timecorr",
      "Use time correction");
   parser.addOption(time_correction_option);
   auto start_option = QCommandLineOption("start",
      "Start simulation automatically");
   parser.addOption(start_option);
   auto save_option = QCommandLineOption("save",
      "Directory to write the run into; its last path component becomes the "
      "run ID. Writes several files, not one: the .auc data, the time-state "
      "pair, the edit file, and any noise CSVs. A multi-speed run instead "
      "writes one sibling directory per speed, suffixed with -<rpm>",
      "dir");
   parser.addOption(save_option);
   auto guid_seed_option = QCommandLineOption("guid-seed",
      "Derive this run's experiment, raw and edit GUIDs from this text "
      "instead of minting random ones, so simulating the same inputs twice "
      "produces the same identities. Use a value unique to the run",
      "text");
   parser.addOption(guid_seed_option);
   auto edit_stamp_option = QCommandLineOption("edit-timestamp",
      "Stamp the edit filename with this yyMMddhhmm instead of the current "
      "clock, so the run's filenames are reproducible too",
      "yyMMddhhmm");
   parser.addOption(edit_stamp_option);
   auto close_option = QCommandLineOption("close",
      "Close application if no errors occurred");
   parser.addOption(close_option);
   auto ignore_db_option = QCommandLineOption("no-db",
      "Ignore database preferences and use only locally available data");
   parser.addOption(ignore_db_option);
   auto errors_option = QCommandLineOption("errors-cl",
      "Write errors to the console without opening the GUI");
   parser.addOption(errors_option);
   auto run_type_option = QCommandLineOption("runtype",
      runTypeOptionHelp(), "runtype");
   parser.addOption(run_type_option);
   auto cell_option = QCommandLineOption("cell", cellOptionHelp(), "cell");
   parser.addOption(cell_option);
   auto channel_option = QCommandLineOption("channel", channelOptionHelp(),
      "channel");
   parser.addOption(channel_option);
   auto noise_seed_option = QCommandLineOption("noise-seed",
      noiseSeedOptionHelp(), "seed");
   parser.addOption(noise_seed_option);

   QMap<QString, QString> args;
   int cli_exit_code = 0;
   if ( handleStandardCliOptions( parser, help_option, version_option, cli_exit_code ) )
      return cli_exit_code;

   // Parse command-specific options.

   // Parse the database setting.
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
   // parse buffer
   if ( parser.isSet( buffer_option ) && !parser.value( buffer_option ).isEmpty() )
   {
      args["buffer"] = parser.value( buffer_option );
   }
   // parse simulation parameters
   if ( parser.isSet( sim_parameters_option ) && !parser.value( sim_parameters_option ).isEmpty() )
   {
      args["simparams"] = parser.value( sim_parameters_option );
   }
   // parse rotor
   if ( parser.isSet( rotor_option ) && !parser.value( rotor_option ).isEmpty() )
   {
      args["rotor"] = parser.value( rotor_option );
   }
   // Parse centerpiece and channel indices.
   if ( parser.isSet( centerpiece_option ) )
   {
      args["centerpiece"] = parser.value( centerpiece_option );
   }
   if ( parser.isSet( centerpiece_channel_option ) )
   {
      args["centerpiece-channel"] = parser.value( centerpiece_channel_option );
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
   // parse run type
   if ( parseRunTypeOption( parser, run_type_option, args, cli_exit_code ) )
      return cli_exit_code;
   // parse the cell/channel triple
   if ( parseTripleOptions( parser, cell_option, channel_option, args,
                            cli_exit_code ) )
      return cli_exit_code;
   // parse the noise seed
   if ( parseNoiseSeedOption( parser, noise_seed_option, args, cli_exit_code ) )
      return cli_exit_code;
   // parse save
   if ( parser.isSet( save_option ) && !parser.value( save_option ).isEmpty() )
   {
      args["save"] = parser.value( save_option ).replace("\\", "/");
   }
   // parse guid-seed
   if ( parser.isSet( guid_seed_option ) && !parser.value( guid_seed_option ).isEmpty() )
   {
      args["guid-seed"] = parser.value( guid_seed_option );
   }
   // parse edit-timestamp
   if ( parser.isSet( edit_stamp_option ) && !parser.value( edit_stamp_option ).isEmpty() )
   {
      args["edit-timestamp"] = parser.value( edit_stamp_option );
   }
   // parse close
   if ( parser.isSet( close_option ) )
   {
      args["close"] = "true";
   }
   int init_status = w.init_from_args(args);
   if ( default_data_location != US_Settings::default_data_location() && parser.isSet( ignore_db_option ) )
   {
      // revert the previously changed default data location
      US_Settings::set_default_data_location( default_data_location );
   }
   // Show the GUI only if needed.
   return showGuiIfNeeded( w, init_status, args );
}
#endif

// US_Astfem_Sim constructor
US_Astfem_Sim::US_Astfem_Sim( QWidget* p, Qt::WindowFlags f )
   : US_Widgets( true, p, f )
{
   dbg_level           = US_Settings::us_debug();
   tmst_tfpath         = "";
   // Only --runtype changes this. The GUI has no control for it, so an
   // interactive run always produces RA, exactly as before.
   run_type            = "RA";

   setWindowTitle( "UltraScan3 Simulation Module" );
   setPalette( US_GuiSettings::frameColor() );
   init_simparams();
   meniscus_ar                 = 5.8 + simparams.bottom_position - 7.2;
   stopFlag            = false;
   sim_failed          = false;
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
   moviePlot->setAxisScale( QwtPlot::xBottom, 5.8, 7.2 );

   plot->addLayout( plot1 );


   QLabel* lb_time  = us_label( tr( "Time( in seconds):" ) );
   lcd_time         = us_lcd( 6, 0 );
   QLabel* lb_speed = us_label( tr( "Current Speed:" ) );
   lcd_speed        = us_lcd( 5, 0 );
   lb_time->setAlignment ( Qt::AlignCenter );
   lb_speed->setAlignment( Qt::AlignCenter );

   timeSpeed->addWidget( lb_time );
   timeSpeed->addWidget( lcd_time );
   timeSpeed->addWidget( lb_speed );
   timeSpeed->addWidget( lcd_speed );

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
   scanPlot->setAxisScale( QwtPlot::xBottom, 5.8, 7.2 );

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

// Initialize the simulation from command-line arguments.
int US_Astfem_Sim::init_from_args( const QMap<QString, QString>& flags ) {
   // check if model is to be loaded
   bool gui_needed = !flags.contains("close");
   bool error_occured = false;
   // Each input is optional. Only an explicitly requested input that fails
   // to load should prevent start_simulation() below.
   bool loaded_model = true;
   bool loaded_buffer = true;
   bool loaded_simparams = true;
   bool loaded_rotor = true;
   bool errors_to_cl = flags.contains("errors-cl");
   // load model
   if ( flags.contains("model") && flags["model"].length() > 0 ) {
      US_Model temp_model = US_Model();
      QString model_id = flags["model"];
      US_ModelGui* dialog = new US_ModelGui( system  );
      bool success = dialog->load_model( model_id, temp_model );
      dialog->close();
      if ( !success ) {
         reportHeadlessLoadFailure( "model", model_id, errors_to_cl,
                                     gui_needed, error_occured );
         loaded_model = false;
      }
      else {
         change_model(temp_model);
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
         reportHeadlessLoadFailure( "buffer", load_id, errors_to_cl,
                                     gui_needed, error_occured );
         loaded_buffer = false;
      }
      else {
         change_buffer(buffer);
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
         reportHeadlessLoadFailure( "simparams", load_id, errors_to_cl,
                                     gui_needed, error_occured );
         loaded_simparams = false;
      }
      else {
         // The file omits bottom_position; mirror the loaded at-rest bottom.
         // An explicit --centerpiece overrides it below.
         simparams.bottom_position = simparams.bottom;
         set_parameters( );
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
         rotorInfo->close();
      }
      else {
         rotorInfo->close();
         reportHeadlessLoadFailure( "rotor", rotor_id, errors_to_cl,
                                     gui_needed, error_occured );
         loaded_rotor = false;
      }
      delete rotorInfo;
      delete disk_controls;
   }

   // A centerpiece selects the row geometry and overrides the loaded bottom.
   // Preserve rotorCalID so its calibration remains in effect.
   if ( flags.contains("centerpiece") || flags.contains("centerpiece-channel") )
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

   // set the output data type if given. Already validated in main(); absent
   // it keeps the constructor's "RA", which is what the GUI always uses.
   // Reproducible identity. Both stay empty unless asked for, so the desktop
   // and every existing caller keep minting fresh GUIDs and stamping the clock.
   if ( flags.contains("guid-seed") )
      guid_seed  = flags["guid-seed"];

   if ( flags.contains("edit-timestamp") )
      edit_stamp = flags["edit-timestamp"];

   if ( flags.contains("runtype") && flags["runtype"].length() == 2 )
   {
      run_type = flags["runtype"];
   }

   // Preserve the historical 1/S defaults unless the CLI overrides them.
   if ( flags.contains("cell") )
      sim_cell    = flags["cell"].toInt();

   if ( flags.contains("channel") )
      sim_channel = flags["channel"].at( 0 ).toLatin1();

   if ( flags.contains("noise-seed") )
      noise_seed  = flags["noise-seed"].toUInt();
   // set movie flag if needed
   if ( flags.contains("movie") )
   {
      ck_movie->setChecked( true );
      movieFlag = true;
   }
   // set timecorr flag if needed
   if ( flags.contains("timecorr") )
   {
      ck_timeCorr->setChecked( true );
      time_correctionFlag = true;
   }
   // check save directory
   if ( flags.contains("save") && flags["save"].length() > 0 )
   {
      // Check whether the path exists and is writable.
      QString save_path = flags["save"];
      QDir dir( save_path );
      if ( !dir.exists() ) {
         if ( errors_to_cl )
         {
            qDebug() << "Error: save directory does not exist:" << save_path;
            exit( 2 );
         }
         error_occured = true;
         gui_needed = true;
      }
      // Check whether a file can be created in the directory.
      QFile file(dir.filePath( "tmp.txt" ) );
      if ( !file.open(QIODevice::WriteOnly ) )
      {
         if ( errors_to_cl )
         {
            qDebug() << "Error: save directory is not writable:" << save_path;
            exit( 2 );
         }
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

         if ( sim_failed )
         {  // Saving here would write a dataset that was never simulated,
            // and report success while doing it.
            DbgLv(0) << "US_Astfem_Sim: simulation failed; nothing was saved";
            error_occured = true;
         }
         else if ( flags.contains( "save" ) && flags["save"].length() > 0 )
         {
            // check if path is accessible and writable
            QString save_path = flags["save"];
            save_simulation( save_path, true, true );
         }
      }
   }
   else
   {
      gui_needed = true;
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

   times_comp   = dhrs * 3600.0 + dmns * 60.0;
   ncomponent   = system.components.size();
DbgLv(1) << "ASIM: chg_stat: ncomponent" << ncomponent << "times_comp" << times_comp;
   times_comp   = qMax( 100.0, times_comp );
   ncomponent   = qMax( 1, ncomponent );
   int maxts    = ncomponent * 100;
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
   sim_data_all.type[0]    = run_type.at( 0 ).toLatin1();
   sim_data_all.type[1]    = run_type.at( 1 ).toLatin1();

   QString guid = US_SimRecord::guid( guid_seed, "raw.all" );
   US_Util::uuid_parse( guid, (uchar*)sim_data_all.rawGUID );

   sim_data_all.cell        = sim_cell;
   sim_data_all.channel     = sim_channel;
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
   double s_speed = 0.0;
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
      const double time_between_scans = ( sp->scans > 1 )
         ? scanning_time / static_cast<double>( sp->scans - 1 )
         : 0.0;
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
            if ( sp->scans == 1 ) {
               sp->time_last = sp->time_first;
               sp->w2t_last  = sp->w2t_first;
            }
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
   sim_failed     = false;
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
   scanPlot ->setAxisAutoScale( QwtPlot::xBottom );
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
      sim_datas[ jd ].type[0]    = run_type.at( 0 ).toLatin1();
      sim_datas[ jd ].type[1]    = run_type.at( 1 ).toLatin1();

      // Each speed step is saved as its own run, so it carries its own raw
      // GUID rather than a view of a shared one.
      QString guid = US_SimRecord::guid( guid_seed,
                                         QString( "raw.%1" ).arg( jd ) );
      US_Util::uuid_parse( guid, (uchar*)sim_datas[ jd ].rawGUID );

      sim_datas[ jd ].cell        = sim_cell;
      sim_datas[ jd ].channel     = sim_channel;
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

   stopFlag         = false;

   simparams.mesh_radius.clear();
   simparams.sim    = true;
   simparams.firstScanIsConcentration = false;

   // ASTFEM computes one composite dataset spanning every speed step; ASTFVM
   // computes sim_datas[ 0 ] and leaves the composite empty.  Noise follows
   // whichever the solver actually filled in.
   noise_to_composite = ( simparams.meshType != US_SimulationParameters::ASTFVM );

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
      if ( astfem->calculate( sim_data_all ) < 0 )
      {  // Ignoring this would go on to save a dataset that was never
         // simulated. The solver has already reported why it failed.
         DbgLv(0) << "US_Astfem_Sim: simulation failed";
         sim_failed  = true;
         return;
      }
      calc_over();
   }
   else
   {
      astfvm = new US_LammAstfvm( system, simparams );

      connect( astfvm, SIGNAL( new_scan( QVector< double >*, double* ) ),
                       SLOT( update_movie_plot( QVector< double >*, double* ) ) );
      connect( astfvm, SIGNAL( current_component( int ) ),
                       SLOT  ( update_progress  ( int ) ) );
      connect( astfvm, SIGNAL( new_time   ( double ) ),
                       SLOT  ( update_time( double ) ) );
      connect( astfvm, SIGNAL( current_speed( int ) ),
                       SLOT  ( update_speed ( int ) ) );
      connect( astfvm, SIGNAL( calc_progress( int ) ),
                       SLOT  ( show_progress( int ) ) );
      connect( astfvm, SIGNAL( calc_done( void ) ),
                       SLOT  ( calc_over( void ) ) );
      astfvm->calculate( sim_datas[ 0 ] );
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

   // Seeding here rather than at startup keeps every draw that contributes to
   // the saved data downstream of the seed, whatever the solver consumed.
   if ( noise_seed != 0 )
      US_Math2::randomize( noise_seed );

   ri_noise();
   baseline();
   random_noise();
   ti_noise();

   if ( noise_to_composite )
      derive_speed_data();

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
   int scan_offset = 0;

   // Add radially invariant noise
   for ( int jd = 0; jd < simparams.speed_step.size(); jd++ )
   {
      QVector<QVector<double>> csv_data;
      QVector<double> tv;
      QVector<double> rv;
      for ( int ks = 0; ks < sim_datas[ jd ].scanData.size(); ks++ )
      {
         double rinoise = US_Math2::box_muller( 0, total_conc * simparams.rinoise / 100 );
         US_DataIO::Scan& scan = noise_scan( jd, scan_offset, ks );
         tv << scan.seconds;
         rv << rinoise;

         for ( int mp = 0; mp < sim_datas[ jd ].pointCount(); mp++ )
         {
            scan.rvalues[ mp ] += rinoise;
         }
      }
      csv_data << tv;
      csv_data << rv;
      US_CSV_Data csv;
      csv.setData( header, csv_data );
      csv_data_ri << csv;
      scan_offset += sim_datas[ jd ].scanData.size();
   }
}

void US_Astfem_Sim::baseline( void )
{
   if ( simparams.baseline == 0.0 ) return;

   // Add a constant baseline offset
   int scan_offset = 0;
   for ( int jd = 0; jd < simparams.speed_step.size(); jd++ )
   {
       for ( int ks = 0; ks < sim_datas[ jd ].scanData.size(); ks++ )
       {
           US_DataIO::Scan& scan = noise_scan( jd, scan_offset, ks );
           for ( int mp = 0; mp < sim_datas[ jd ].pointCount(); mp++ )
              scan.rvalues[ mp ] += simparams.baseline;
       }
       scan_offset += sim_datas[ jd ].scanData.size();
   }
}

void US_Astfem_Sim::random_noise( void )
{
   if ( simparams.rnoise == 0.0 && simparams.lrnoise == 0.0) return;
   // Add random noise
   if ( simparams.rnoise != 0.0 && simparams.lrnoise != 0.0)
   {  int scan_offset = 0;
      for ( int jd = 0; jd < simparams.speed_step.size(); jd++ )
      {
          for ( int j = 0; j < sim_datas[ jd ].scanData.size(); j++ )
          {
              US_DataIO::Scan& scan = noise_scan( jd, scan_offset, j );
              for ( int k = 0; k < sim_datas[ jd ].pointCount(); k++ )
              {
                  scan.rvalues[ k ] +=
                  US_Math2::box_muller( 0, total_conc * simparams.rnoise / 100 ) + // based on total concentration
                  US_Math2::box_muller( 0, scan.rvalues[ k ] * simparams.lrnoise / 100 ); // based on local concentration
              }//'k' loop
          }//'j' loop
          scan_offset += sim_datas[ jd ].scanData.size();
       }//'i' loop
   }
   if ( simparams.rnoise != 0.0  &&  simparams.lrnoise == 0.0 )
   {
      int scan_offset = 0;
      for ( int jd = 0; jd < simparams.speed_step.size(); jd++)
      {
          for ( int ks = 0; ks < sim_datas[ jd ].scanData.size(); ks++ )
          {
              US_DataIO::Scan& scan = noise_scan( jd, scan_offset, ks );
              for ( int mp = 0; mp < sim_datas[ jd ].pointCount(); mp++ )
              {
                  scan.rvalues[ mp ] +=
                  US_Math2::box_muller( 0, total_conc * simparams.rnoise / 100 ); // based on total concentration
              }
          }
          scan_offset += sim_datas[ jd ].scanData.size();
       }
   }
   if ( simparams.rnoise == 0.0 && simparams.lrnoise != 0.0)
   {  int scan_offset = 0;
      for ( int jd = 0; jd < simparams.speed_step.size(); jd++ )
      {
          for ( int ks = 0; ks < sim_datas[ jd ].scanData.size(); ks++ )
          {
              US_DataIO::Scan& scan = noise_scan( jd, scan_offset, ks );
              for ( int mp = 0; mp < sim_datas[ jd ].pointCount(); mp++ )
              {
                  scan.rvalues[ mp ] +=
                  US_Math2::box_muller( 0, scan.rvalues[ mp ] * simparams.lrnoise / 100 ); // based on local concentration
              }
          }
          scan_offset += sim_datas[ jd ].scanData.size();
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
   int scan_offset = 0;
   for ( int jd = 0; jd < simparams.speed_step.size(); jd++ )
   {
      for ( int ks = 0; ks < sim_datas[ jd ].scanData.size(); ks++ )
      {
         US_DataIO::Scan& scan = noise_scan( jd, scan_offset, ks );
         for ( int mp = 0; mp < points; mp++ )
         {
            scan.rvalues[ mp ] += tinoise[ mp ];
         }
      }
      scan_offset += sim_datas[ jd ].scanData.size();
   }

   // save the TI into a csv file
   QDir dir( US_Settings::resultDir() );
   QStringList header{"Radial Points (cm)", "TI noise (OD)"};
   QVector<QVector<double>> csv_data;
   csv_data << sim_datas[0].xvalues;
   csv_data << tinoise;
   csv_data_ti.setData( header, csv_data );
}

// The scan that speed step jd's scan ks contributes to.
//
// ASTFEM simulates one composite dataset covering every speed step, so noise
// is applied there once and the per-speed datasets are derived from the result
// afterward.  Applying it per speed instead would draw a separate noise series
// for each step of what is one continuous experiment.  ASTFVM has no composite
// -- it fills only sim_datas[ 0 ] -- so it keeps the per-speed traversal it has
// always used.
US_DataIO::Scan& US_Astfem_Sim::noise_scan( int jd, int scan_offset, int ks )
{
   return noise_to_composite ? sim_data_all.scanData[ scan_offset + ks ]
                             : sim_datas[ jd ].scanData[ ks ];
}

// Copy the finalized composite scans out to the per-speed datasets that are
// plotted and saved.  ASTFEM only; see noise_scan().
void US_Astfem_Sim::derive_speed_data( void )
{
   int scan_offset = 0;

   for ( int jd = 0; jd < sim_datas.size(); jd++ )
   {
      Q_ASSERT( scan_offset + sim_datas[ jd ].scanData.size()
                <= sim_data_all.scanData.size() );

      for ( int js = 0; js < sim_datas[ jd ].scanData.size(); js++ )
         sim_datas[ jd ].scanData[ js ] =
            sim_data_all.scanData[ scan_offset + js ];

      scan_offset += sim_datas[ jd ].scanData.size();
   }

   Q_ASSERT( scan_offset == sim_data_all.scanData.size() );
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

bool US_Astfem_Sim::save_simulation( QString odir, bool supress_dialog,
                                     bool write_records )
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
      {  // Multi-speed case: each speed step is saved as its own complete run
         //  in a sibling directory suffixed with the rpm
DbgLv(1) << "ASIM:svscn: m-speed  have_tmst" << have_tmst;
         if ( have_tmst )
         {  // Copy temporary timestate file pairs to each speed's subdirectory
            //  and save AUC data there
            for ( int jd = 0; jd < nstep; jd++ )
            {
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

         // Save TI, RI noises beside the data each describes
         for ( int ii = 0; ii < nstep; ii++ )
         {
            int ispeed          = simparams.speed_step[ ii ].rotorspeed;
            QString spsufx      = QString::asprintf( "-%05d", ispeed );
            QDir dir( odir + spsufx );

            if ( csv_data_ti.rowCount() > 0 )
            {
               csv_data_ti.setFilePath(
                  dir.absoluteFilePath( "ASTFEM_TI_NOISE.csv" ) );
               save_csv_noise( csv_data_ti );
            }

            if ( ii < csv_data_ri.size()  &&  csv_data_ri[ ii ].rowCount() > 0 )
            {
               csv_data_ri[ ii ].setFilePath(
                  dir.absoluteFilePath( "ASTFEM_RI_NOISE.csv" ) );
               save_csv_noise( csv_data_ri[ ii ] );
            }
         }
      }  // End:  multi-speed case
   }  // End:  output directory specified
   if ( !write_records )
   {  // The simulated data has been written; the records that describe it have
      // not. Gated separately from supress_dialog, which the two used to share:
      // that made whether a run got an experiment record depend on whether it
      // was allowed to raise a dialog, so the GUI's Save Simulation wrote scan
      // data with no solution, no experiment record and no edit files, leaving
      // nothing able to attribute the data to a sample. Kept off by default so
      // the desktop keeps writing exactly what it always has; the headless
      // caller, whose output is loaded into a database, asks for the records.
      return true;
   }

   // Ensure trailing /
   if ( odir.right( 1 ) != "/" )
   {
      odir += "/";
   }

   QString runType = QString( QChar( sim_datas[0].type[ 0 ] ) )
                     + QString( QChar( sim_datas[0].type[ 1 ] ) );

   // One solution for the run: the model's components in the run's buffer.
   // Shared by every dataset, so it is composed and written once. The
   // composition is shared with us_mwl_species_sim through US_SimRecord.
   QList< US_Model > models;
   QList< double >   wavelengths;
   models      << system;
   wavelengths << system.wavelength;

   US_Solution sol = US_SimRecord::solution( models, wavelengths, buffer );

   // saveToDisk mints one only when this does not already hold a UUID, so
   // seeding it here needs no change to US_Solution.
   if ( ! guid_seed.isEmpty() )
      sol.solutionGUID = US_SimRecord::guid( guid_seed, "solution" );

   if ( sol.analyteInfo.isEmpty() )
      return false;

   sol.saveToDisk();

   // One experiment record and one edit file per dataset, each written beside
   // the data it describes.
   //
   // A single-speed run is one dataset in odir and this writes one of each
   // there.  A multi-speed run has already written one complete run per speed
   // into a sibling directory suffixed with the rpm, each with its own run ID
   // and its own rawGUID: separate runs, not triples of one run.  Both
   // documents name their run in the filename and their data by GUID in the
   // body, so a single set written for the base run ID describes datasets that
   // are not there and matches none of the ones that are, leaving every
   // multi-speed dataset unloadable.
   int     nspeed = simparams.speed_step.count();
   QString now    = edit_stamp.isEmpty()
                    ? QDateTime::currentDateTimeUtc().toString( "yyMMddhhmm" )
                    : edit_stamp;

   for ( int jd = 0; jd < nspeed; jd++ )
   {
      US_DataIO::RawData& sim_data = sim_datas[ jd ];
      QString edir       = odir;
      QString edit_runid = run_id;

      // The speed steps this run covers.  A multi-speed dataset is a run of
      // one speed, so its record carries only that step rather than all of
      // them.
      QVector< SP_SPEEDPROFILE > speed_steps = simparams.speed_step;

      if ( nspeed > 1 )
      {  // The same suffix the save loop above used.  odir carries a trailing
         // separator by this point, so it comes off and goes back on.
         QString spsufx     = QString::asprintf( "-%05d",
                                 simparams.speed_step[ jd ].rotorspeed );
         edir               = odir.left( odir.length() - 1 ) + spsufx + "/";
         edit_runid         = run_id + spsufx;

         speed_steps.clear();
         speed_steps << simparams.speed_step[ jd ];
      }

      // The cell limits follow the rotor speed, and save_xla left af_params
      // holding whichever speed it saved last, so re-derive this one's.
      adjust_limits( (double)simparams.speed_step[ jd ].rotorspeed );

      QString cell       = QString::number( sim_data.cell );
      QString channel    = QString( sim_data.channel );
      double  wavelength = sim_data.scanData.first().wavelength;
      QString wl         = ( wavelength < 99 ) ? QString( "123" )
                                               : QString::number( wavelength );

      // The record.  saveToDisk splits cell, channel and wavelength back out
      // of tripleDesc, so the " / " separator matters.
      US_Experiment experiment = US_SimRecord::experiment( rotor, simparams,
                                                           edit_runid, runType,
                                                           guid_seed );

      US_Convert::TripleInfo triple;
      triple.clear();
      triple.tripleID    = 1;
      triple.tripleDesc  = cell + " / " + channel + " / " + wl;
      triple.excluded    = false;
      triple.centerpiece = 1;
      triple.solution    = sol;
      memcpy( triple.tripleGUID, sim_data.rawGUID, sizeof( triple.tripleGUID ) );

      QList< US_Convert::TripleInfo > triples;
      triples << triple;

      if ( experiment.saveToDisk( triples, runType, edit_runid, edir,
                                  speed_steps ) != US_Convert::OK )
      {
         qDebug() << "Error: could not write the experiment record for"
                  << edit_runid;
         return false;
      }

      // The edit file.
      QString fname = edit_runid + "." + now + "." + runType + "." + cell + "."
                      + channel + "." + wl + ".xml";

DbgLv(1) << "EDT:WrXml:  waveln" << wl;

      US_DataIO::EditValues ev;
      ev.expType    = "Velocity";
      ev.runID      = edit_runid;
      ev.cell       = cell;
      ev.channel    = channel;
      ev.wavelength = wl;
      ev.editGUID   = US_SimRecord::guid( guid_seed, "edit." + edit_runid );
      ev.dataGUID   = US_Util::uuid_unparse(
                         reinterpret_cast<uchar*>( sim_data.rawGUID ) );

      US_SimulationParameters::editRadiiFromCell( ev, af_params.current_meniscus,
                                                      af_params.current_bottom );
      ev.ODlimit    = US_SimRecord::maxOD( sim_data );

      if ( US_DataIO::writeEdits( edir + fname, ev ) != US_DataIO::OK )
      {  // Only the headless caller reaches this block, so report the failure
         // the way the rest of that path does rather than with a modal dialog
         // nothing would be there to dismiss.
         qDebug() << "Error: could not write edit file" << edir + fname;
         return false;
      }
   }

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
      dthresh = total_conc * 2.0;
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
      if ( ! supress_dialog )
      {
         QMessageBox::information( this,
               tr( "OD Values Limited to Threshold" ),
               tr( "%1 readings in %2 scans were reset to the threshold "
                   "value of %3.\nThe maximum OD before thresholding was %4." )
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

      // The interpolation bitmap describes the readings, so it has to grow with
      // them.  None of the simulated points is interpolated, so the added bits
      // are zero.
      scan->interpolated.fill( '\0', ( points + 7 ) / 8 );

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

   int wstat        = US_DataIO::writeRawData( ofname, sim_data );
DbgLv(1) << "Sim:SV: after_write_rawdata" << ofname;

   if ( wstat != US_DataIO::OK )
   {  // Nothing was saved, so do not report completion
      progress->setValue( total_scans );
      lb_progress->setText( tr( "Save failed" ) );
      QMessageBox::critical( this, tr( "Simulation Save Failed" ),
         tr( "The simulated data could not be written to\n%1\n\n%2" )
         .arg( ofname ).arg( US_DataIO::errorString( wstat ) ) );
      return;
   }

   progress->setValue( total_scans );
   lb_progress->setText( tr( "Completed" ) );
}


//int   scan_count = exp_data[speed_step].scanCount();
//          points     = exp_data[speed_step].pointCount();
void US_Astfem_Sim::plot( int step )
{
   QList< QColor > mcolors;
   int nmcols     = plot2->map_colors( mcolors );
  // dataPlotClear( scanPlot );
   double min_y_axis = 0.0;
   double max_y_axis = total_conc * 2.0;

   // Set plot scale for band-forming
   if ( simparams.band_forming )
   {
      max_y_axis = total_conc;
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
      scanPlot->setAxisAutoScale( QwtPlot::xBottom );
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
         for ( int k = 0; k < points; k++ )
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

   moviePlot->setAxisScale( QwtPlot::yLeft, 0, yscale );
   //moviePlot->setAxisAutoScale( QwtPlot::yLeft );
   moviePlot->setAxisAutoScale( QwtPlot::xBottom );

   double* r = new double [ x->size() ];

   for ( int i = 0; i < x->size(); i++ ) r[ i ] = (*x)[ i ];

   QwtPlotCurve* curve =
      new QwtPlotCurve( "Scan Number " + QString::number( ++curve_count ) );

   curve->setPen    ( QPen( Qt::yellow, 3 ) );
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

   int tstep    = (int)qRound( dtime * 100.0 / times_comp );
   tstep        = ( icomponent - 1 ) * 100 + tstep;
   if ( tstep != kstep )
   {
      show_progress( tstep );
DbgLv(1) << "ASIM: upd_tm: tstep" << tstep << "dtime" << dtime << "icomponent" << icomponent
 << "times_comp" << times_comp;
   }
   kstep        = tstep;
}
