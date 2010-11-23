#include <QApplication>

#include <uuid/uuid.h>

#include "us_license_t.h"
#include "us_license.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_run_details2.h"
#include "us_plot.h"
#include "us_math2.h"
#include "us_convert.h"
#include "us_solution_gui.h"
#include "us_db2.h"
#include "us_passwd.h"
#include "us_process_convert.h"
#include "us_convertio.h"
#include "us_intensity.h"
#include "us_get_dbrun.h"

int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );

   #include "main1.inc"

   // License is OK.  Start up.
   
   US_Convert w;
   w.show();                   //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication
}

US_Convert::US_Convert() : US_Widgets()
{
   setWindowTitle( tr( "Convert Legacy Raw Data" ) );
   setPalette( US_GuiSettings::frameColor() );

   // Very light gray, for read-only line edits
   QPalette gray = US_GuiSettings::editColor();
   gray.setColor( QPalette::Base, QColor( 0xe0, 0xe0, 0xe0 ) );

   QGridLayout* settings = new QGridLayout;

   int row = 0;

   // Set the wavelength tolerance for c/c/w determination
   QLabel* lb_tolerance = us_label( tr( "Dataset Separation Tolerance:" ) );
   settings->addWidget( lb_tolerance, row, 0 );

   ct_tolerance = us_counter ( 2, 0.0, 100.0, 5.0 ); // #buttons, low, high, start_value
   ct_tolerance->setStep( 1 );
   ct_tolerance->setMinimumWidth( 160 );
   settings->addWidget( ct_tolerance, row++, 1 );
   connect( ct_tolerance, SIGNAL( valueChanged         ( double ) ),      // if the user has changed it
                          SLOT  ( toleranceValueChanged( double ) ) );

   pb_details = us_pushbutton( tr( "Run Details" ), false );
   connect( pb_details, SIGNAL( clicked() ), SLOT( runDetails() ) );
   settings->addWidget( pb_details, row++, 0, 1, 2 );

   // Pushbuttons to load and reload data
   pb_load = us_pushbutton( tr( "Import Legacy Run from HD" ) );
   connect( pb_load, SIGNAL( clicked() ), SLOT( load() ) );
   settings->addWidget( pb_load, row, 0 );

   // load US3 data ( that perhaps has been done offline )
   pb_loadUS3HD = us_pushbutton( tr( "Load US3 Run from HD" ), true );
   connect( pb_loadUS3HD, SIGNAL( clicked() ), SLOT( loadUS3HD() ) );
   settings->addWidget( pb_loadUS3HD, row++, 1 );

   // External program to enter experiment information
   pb_editRuninfo = us_pushbutton( tr( "Edit Run Information" ) );
   connect( pb_editRuninfo, SIGNAL( clicked() ), SLOT( editRuninfo() ) );
   settings->addWidget( pb_editRuninfo, row, 0 );
   pb_editRuninfo->setEnabled( false );

   pb_loadUS3DB = us_pushbutton( tr( "Load US3 Run from DB" ) );
   connect( pb_loadUS3DB, SIGNAL( clicked() ), SLOT( loadUS3DB() ) );
   settings->addWidget( pb_loadUS3DB, row++, 1 );

   // Change Run ID
   QLabel* lb_runID = us_label( tr( "Run ID:" ) );
   settings->addWidget( lb_runID, row, 0 );

   le_runID = us_lineedit( "", 1 );
   le_runID ->setMinimumWidth( 225 );
   settings->addWidget( le_runID, row++, 1 );

   // Directory
   QLabel* lb_dir = us_label( tr( "Directory:" ) );
   settings->addWidget( lb_dir, row++, 0, 1, 2 );

   le_dir = us_lineedit( "", 1 );
   settings->addWidget( le_dir, row++, 0, 1, 2 );
   le_dir ->setPalette ( gray );
   le_dir ->setReadOnly( true );

   // Description
   lb_description = us_label( tr( "Description:" ), -1 );
   settings->addWidget( lb_description, row++, 0, 1, 2 );

   le_description = us_lineedit( "", 1 );
   settings->addWidget( le_description, row++, 0, 1, 2 );

   // Cell / Channel / Wavelength
   QGridLayout* ccw = new QGridLayout();

   lb_triple = us_banner( tr( "Cell / Channel / Wavelength" ), -1 );
   ccw->addWidget( lb_triple, row++, 0, 1, 3 );

   lw_triple = us_listwidget();
   lw_triple->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed );
   lw_triple->setMaximumHeight( 110 );
   lw_triple->setMaximumWidth ( 120 );
   connect( lw_triple, SIGNAL( itemClicked ( QListWidgetItem* ) ),
                       SLOT  ( changeTriple( QListWidgetItem* ) ) );
   ccw->addWidget( lw_triple, row, 0, 3, 1 );

   QLabel* lb_ccwinfo = us_label( tr( "Enter Associated c/c/w Info:" ) );
   ccw->addWidget( lb_ccwinfo, row++, 1, 1, 2 );

   // Set up centerpiece drop-down
   cb_centerpiece = new US_SelectBox( this );
   centerpieceInfo();
   cb_centerpiece -> load();
   connect( cb_centerpiece, SIGNAL( currentIndexChanged( int ) ), 
                            SLOT  ( getCenterpieceIndex( int ) ) );
   ccw->addWidget( cb_centerpiece, row++, 1, 1, 2 );

   // External program to enter solution information
   pb_solution = us_pushbutton( tr( "Manage Solutions" ), false );
   connect( pb_solution, SIGNAL( clicked() ), SLOT( getSolutionInfo() ) );
   ccw->addWidget( pb_solution, row, 1 );

   pb_applyAll = us_pushbutton( tr( "Apply to All" ), false );
   connect( pb_applyAll, SIGNAL( clicked() ), SLOT( tripleApplyAll() ) );
   ccw->addWidget( pb_applyAll, row++, 2 );

   QLabel* lb_solution = us_label( tr( "Solution:" ) );
   ccw->addWidget( lb_solution, row, 0 );

   le_solutionDesc = us_lineedit( "", 1 );
   le_solutionDesc ->setPalette ( gray );
   le_solutionDesc ->setReadOnly( true );
   ccw->addWidget( le_solutionDesc, row++, 1, 1, 2 );

   settings->addLayout( ccw, row++, 0, 1, 2 );

   // Scan Controls
   QLabel* lb_scan = us_banner( tr( "Scan Controls" ) );
   settings->addWidget( lb_scan, row++, 0, 1, 2 );

   // Scan focus from
   QLabel* lb_from = us_label( tr( "Scan Focus from:" ), -1 );
   lb_from->setAlignment( Qt::AlignVCenter | Qt::AlignRight );
   settings->addWidget( lb_from, row, 0 );

   ct_from = us_counter ( 2, 0.0, 0.0 ); // Update range upon load
   ct_from->setStep( 1 );
   settings->addWidget( ct_from, row++, 1 );

   // Scan focus to
   QLabel* lb_to = us_label( tr( "Scan Focus to:" ), -1 );
   lb_to->setAlignment( Qt::AlignVCenter | Qt::AlignRight );
   settings->addWidget( lb_to, row, 0 );

   ct_to = us_counter ( 2, 0.0, 0.0 ); // Update range upon load
   ct_to->setStep( 1 );
   settings->addWidget( ct_to, row++, 1 );

   // Exclude and Include pushbuttons
   pb_exclude = us_pushbutton( tr( "Exclude Scan(s)" ), false );
   connect( pb_exclude, SIGNAL( clicked() ), SLOT( exclude_scans() ) );
   settings->addWidget( pb_exclude, row, 0 );

   pb_include = us_pushbutton( tr( "Include All" ), false );
   connect( pb_include, SIGNAL( clicked() ), SLOT( include() ) );
   settings->addWidget( pb_include, row++, 1 );
   pb_include ->setEnabled( false );

   // Defining data subsets
   pb_define = us_pushbutton( tr( "Define Subsets" ), false );
   connect( pb_define, SIGNAL( clicked() ), SLOT( define_subsets() ) );
   settings->addWidget( pb_define, row, 0 );

   pb_process = us_pushbutton( tr( "Process Subsets" ) , false );
   connect( pb_process, SIGNAL( clicked() ), SLOT( process_subsets() ) );
   settings->addWidget( pb_process, row++, 1 );

   // Choosing reference channel
   pb_reference = us_pushbutton( tr( "Define Reference Scans" ), false );
   connect( pb_reference, SIGNAL( clicked() ), SLOT( define_reference() ) );
   settings->addWidget( pb_reference, row, 0 );

   pb_cancelref = us_pushbutton( tr( "Undo Reference Scans" ), false );
   connect( pb_cancelref, SIGNAL( clicked() ), SLOT( cancel_reference() ) );
   settings->addWidget( pb_cancelref, row++, 1 );

   // Define intensity profile
   pb_intensity = us_pushbutton( tr( "Show Intensity Profile" ) , false );
   connect( pb_intensity, SIGNAL( clicked() ), SLOT( show_intensity() ) );
   settings->addWidget( pb_intensity, row, 0 );

   // drop scan
   pb_dropScan = us_pushbutton( tr( "Drop Current c/c/w" ), false );
   connect( pb_dropScan, SIGNAL( clicked() ), SLOT( drop_reference() ) );
   settings->addWidget( pb_dropScan, row++, 1 );

   // Write pushbuttons
   pb_savetoHD = us_pushbutton( tr( "Save US3 Run to HD" ), false );
   connect( pb_savetoHD, SIGNAL( clicked() ), SLOT( savetoHD() ) );
   settings->addWidget( pb_savetoHD, row, 0 );

   // sync data with DB
   pb_savetoDB = us_pushbutton( tr( "Save US3 Run to DB" ), false );
   connect( pb_savetoDB, SIGNAL( clicked() ), SLOT( savetoDB() ) );
   settings->addWidget( pb_savetoDB, row++, 1 );

   // Standard pushbuttons
   QBoxLayout* buttons = new QHBoxLayout;

   QPushButton* pb_reset = us_pushbutton( tr( "Reset" ) );
   connect( pb_reset, SIGNAL( clicked() ), SLOT( resetAll() ) );
   buttons->addWidget( pb_reset );

   QPushButton* pb_help = us_pushbutton( tr( "Help" ) );
   connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );
   buttons->addWidget( pb_help );

   QPushButton* pb_accept = us_pushbutton( tr( "Close" ) );
   connect( pb_accept, SIGNAL( clicked() ), SLOT( close() ) );
   buttons->addWidget( pb_accept );

   // Plot layout for the right side of window
   QBoxLayout* plot = new US_Plot( data_plot,
                                   tr( "Absorbance Data" ),
                                   tr( "Radius (in cm)" ), 
                                   tr( "Absorbance" ) );

   data_plot->setMinimumSize( 600, 400 );

   data_plot->enableAxis( QwtPlot::xBottom, true );
   data_plot->enableAxis( QwtPlot::yLeft  , true );

   data_plot->setAxisScale( QwtPlot::xBottom, 5.7, 7.3 );
   data_plot->setAxisScale( QwtPlot::yLeft  , 0.0, 1.5 );

   picker = new US_PlotPicker( data_plot );
   picker ->setRubberBand( QwtPicker::VLineRubberBand );

   // Now let's assemble the page
   
   QVBoxLayout* left     = new QVBoxLayout;

   left->addLayout( settings );
   left->addLayout( buttons );
   
   QHBoxLayout* main = new QHBoxLayout( this );
   main->setSpacing         ( 2 );
   main->setContentsMargins ( 2, 2, 2, 2 );

   main->addLayout( left );
   main->addLayout( plot );

   main->setStretch( 0, 2 );
   main->setStretch( 1, 4 );
}

void US_Convert::reset( void )
{
   lw_triple       ->clear();

   le_dir          ->setText( "" );

   le_description  ->setText( "" );
   le_runID        ->setText( "" );
   le_solutionDesc ->setText( "" );

//   cb_centerpiece  ->load();

   pb_load       ->setEnabled( true  );
   pb_loadUS3HD  ->setEnabled( true  );
   pb_loadUS3DB  ->setEnabled( true  );
   pb_exclude    ->setEnabled( false );
   pb_include    ->setEnabled( false );
   pb_savetoHD   ->setEnabled( false );
   pb_details    ->setEnabled( false );
   pb_intensity  ->setEnabled( false );
   pb_cancelref  ->setEnabled( false );
   pb_dropScan   ->setEnabled( false );
   pb_solution   ->setEnabled( false );
   pb_editRuninfo ->setEnabled( false );
   pb_savetoDB    ->setEnabled( false );
   pb_applyAll    ->setEnabled( false );

//   pb_reload     ->setEnabled( true  );
   ct_tolerance  ->setEnabled( true  );

   cb_centerpiece->setEnabled( false );

   ct_from       ->disconnect();
   ct_from       ->setMinValue( 0 );
   ct_from       ->setMaxValue( 0 );
   ct_from       ->setValue   ( 0 );

   ct_to         ->disconnect();
   ct_to         ->setMinValue( 0 );
   ct_to         ->setMaxValue( 0 );
   ct_to         ->setValue   ( 0 );

   // Clear any data structures
   legacyData.clear();
   allExcludes.clear();
   triples.clear();
   allData.clear();
   Pseudo_averaged    = false;
   show_plot_progress = true;
   ExpData.rpms.clear();

   data_plot     ->detachItems();
   picker        ->disconnect();
   data_plot     ->setAxisScale( QwtPlot::xBottom, 5.7, 7.3 );
   data_plot     ->setAxisScale( QwtPlot::yLeft  , 0.0, 1.5 );
   grid          = us_grid( data_plot );
   data_plot     ->replot();

   pb_define     ->setEnabled( false );
   pb_process    ->setEnabled( false );
   step          = NONE;

   enableRunIDControl( true );

   toleranceChanged = false;
   saveStatus       = NOT_SAVED;
   isPseudo         = false;

   pb_reference   ->setEnabled( false );
}

void US_Convert::resetAll( void )
{
   int status = QMessageBox::information( this,
            tr( "Warning" ),
            tr( "This will erase all data currently on the screen, and reset "    ) +
            tr( "the program to its starting condition. No hard-drive data  "     ) +
            tr( "or database information will be affected. Proceed? "             ),
            tr( "&OK" ), tr( "&Cancel" ),
            0, 0, 1 );
   if ( status != 0 ) return;

   reset();

   ExpData.clear();
   ss_limits.clear();
   reference_start = 0;
   reference_end   = 0;

   ct_tolerance    ->setMinValue(   0.0 );
   ct_tolerance    ->setMaxValue( 100.0 );
   ct_tolerance    ->setValue   (   5.0 );
   ct_tolerance    ->setStep( 1 );
   scanTolerance   = 5.0;
}

// User changed the dataset separation tolerance
void US_Convert::toleranceValueChanged( double )
{
   toleranceChanged = true;
   scanTolerance    = ct_tolerance->value();
   reload();
}

// User pressed the load data button
void US_Convert::load( QString dir )
{
   bool success = false;

   if ( dir.isEmpty() )
      success = read();                // Read the legacy data

   else
      success = read( dir );

   if ( ! success ) return;

/*
   // Display the data that was read
   for ( int i = 0; i < legacyData.size(); i++ )
   {
      US_DataIO2::BeckmanRawScan d = legacyData[ i ];

      qDebug() << d.description;
      qDebug() << d.type         << " "
               << d.cell         << " "
               << d.temperature  << " "
               << d.rpm          << " "
               << d.seconds      << " "
               << d.omega2t      << " "
               << d.t.wavelength << " "
               << d.count;

      for ( int j = 0; j < d.readings.size(); j++ )
      {
         if ( i != 2 ) continue; // only the 2nd set for now

         US_DataIO2::RawReading r = d.readings[ j ];

         QString line = QString::number(r.d.radius, 'f', 4 )    + " "
                      + QString::number(r.value, 'E', 5 )       + " "
                      + QString::number(r.stdDev, 'E', 5 );
         qDebug() << line;
      }
   }

*/

   // Define default tolerances before converting
   scanTolerance = ( runType == "WA" ) ? 0.1 : 5.0;
   ct_tolerance->setValue( scanTolerance );

   // Figure out all the triple combinations and convert data
   success = convert();

   if ( ! success ) return;

   setTripleInfo();

   // Initialize exclude list
   init_excludes();
   
   plot_current();

   connect( ct_from, SIGNAL( valueChanged ( double ) ),
                     SLOT  ( focus_from   ( double ) ) );

   connect( ct_to  , SIGNAL( valueChanged ( double ) ),
                     SLOT  ( focus_to     ( double ) ) );

   saveStatus = NOT_SAVED;

   // Ok to enable some buttons now
   enableControls();
}

// User pressed the reload data button
// Legacy data is already supposed to be present
void US_Convert::reload( void )
{
   if ( toleranceChanged )
   {
      // If the tolerance has changed, we need to reconvert the data
      toleranceChanged = false;
      bool success = false;

      triples.clear();

      le_solutionDesc ->setText( "" );
   
      // Figure out all the triple combinations and convert data
      success = convert();
   
      if ( ! success ) return;
   
      setTripleInfo();
   }

   // Initialize exclude list
   init_excludes();
   
   // In this case the runType is not changing
   oldRunType = runType;
   
   pb_include->setEnabled( false );
   pb_exclude->setEnabled( false );
   plot_current();

   connect( ct_from, SIGNAL( valueChanged ( double ) ),
                     SLOT  ( focus_from   ( double ) ) );

   connect( ct_to  , SIGNAL( valueChanged ( double ) ),
                     SLOT  ( focus_to     ( double ) ) );

   // Ok to enable some buttons now
   enableControls();

}

// Enable the common dialog controls when there is data
void US_Convert::enableControls( void )
{
   if ( allData.size() == 0 )
      reset();

   else
   {
      // Ok to enable some buttons now
      pb_savetoHD    ->setEnabled( true );
      pb_details     ->setEnabled( true );
      pb_solution    ->setEnabled( true );
      cb_centerpiece ->setEnabled( true );
      pb_editRuninfo ->setEnabled( true );

      // Ok to drop scan if not the only one
      int currentScanCount = 0;
      for ( int i = 0; i < triples.size(); i++ )
         if ( ! triples[ i ].excluded ) currentScanCount++;
      pb_dropScan    ->setEnabled( currentScanCount > 1 );

      if ( runType == "RI" )
         pb_reference->setEnabled( true );
   
      else if ( runType == "RA" && ss_limits.size() < 2 )
      {
         // Allow user to define subsets, if he hasn't already
         pb_define   ->setEnabled( true );
      } 

      // Disable load buttons if there is data
      pb_load        ->setEnabled( false );
      pb_loadUS3HD   ->setEnabled( false );
      pb_loadUS3DB   ->setEnabled( false );
      
      // Most triples are ccw
      lb_triple   ->setText( tr( "Cell / Channel / Wavelength" ) );
      ct_tolerance->setMinimumWidth( 160 );
      ct_tolerance->setNumButtons  ( 2 );
      ct_tolerance->setRange       ( 0.0, 100.0 );
      ct_tolerance->setStep        ( 1.0 );
      ct_tolerance->setValue       ( scanTolerance );

      // Default tolerances are different for wavelength data
      if ( runType == "WA" )
      {
         // First of all, wavelength triples are ccr.
         lb_triple   ->setText( tr( "Cell / Channel / Radius" ) );
         ct_tolerance->setMinimumWidth( 160 );
         ct_tolerance->setNumButtons  ( 3 );
         ct_tolerance->setRange       ( 0.0, 10.0 );
         ct_tolerance->setStep        ( 0.001 );
         ct_tolerance->setValue       ( scanTolerance );
      }

      // Let's calculate if we're eligible to copy this triple info to all
      // or to save it
      // We have to check against GUID's, because solutions won't have
      // solutionID's yet if they are created as needed offline
      QRegExp rx( "^[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}$" );

      pb_applyAll     -> setEnabled( false );
      if ( triples.size() > 1 && rx.exactMatch( triples[ currentTriple ].solution.solutionGUID ) )
      {   
         pb_applyAll  -> setEnabled( true );
      }

      enableRunIDControl( saveStatus == NOT_SAVED );
         
      enableSyncDB();
   }
}

// Enable or disable the runID control
void US_Convert::enableRunIDControl( bool setEnable )
{
   QPalette gray = US_GuiSettings::editColor();
   gray.setColor( QPalette::Base, QColor( 0xe0, 0xe0, 0xe0 ) );

   if ( setEnable )
   {
      le_runID->setPalette ( US_GuiSettings::normalColor() );
      le_runID->setReadOnly( false );
      connect( le_runID, SIGNAL( editingFinished() ),
                         SLOT  ( runIDChanged   () ) );
   }

   else
   {
      le_runID->disconnect();
      le_runID->setPalette ( gray );
      le_runID->setReadOnly( true );
   }
         
}

// Reset the boundaries on the scan controls
void US_Convert::enableScanControls( void )
{
   ct_from->disconnect();
   ct_from->setMinValue( 0.0 );
   ct_from->setMaxValue(  allData[ currentTriple ].scanData.size() );
   ct_from->setValue   ( 0 );

   ct_to  ->disconnect();
   ct_to  ->setMinValue( 0.0 );
   ct_to  ->setMaxValue(  allData[ currentTriple ].scanData.size() );
   ct_to  ->setValue   ( 0 );

   connect( ct_from, SIGNAL( valueChanged ( double ) ),
                     SLOT  ( focus_from   ( double ) ) );

   connect( ct_to  , SIGNAL( valueChanged ( double ) ),
                     SLOT  ( focus_to     ( double ) ) );

}

// Enable the "sync with DB" button, if appropriate
void US_Convert::enableSyncDB( void )
{
   // Have we made connection with the db?
   if ( ExpData.invID == 0 ||
        triples.size() == 0 )
   {
      pb_savetoDB ->setEnabled( false );
      return;
   }

   // Have we filled out all the c/c/w info?
   // Check GUIDs, because solutionID's may not be present yet.
   QRegExp rx( "^[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}$" );
   for ( int i = 0; i < triples.size(); i++ )
   {
      TripleInfo triple = triples[ i ];
      if ( triple.excluded ) continue;

      if ( ! rx.exactMatch( triple.solution.solutionGUID ) )
      {
         pb_savetoDB ->setEnabled( false );
         return;
      }
   }

   // We have to have saved it to the HD before, to have auc files
   if ( saveStatus == NOT_SAVED )
   {
      pb_savetoDB ->setEnabled( false );
      return;
   }

   // Information is there, but we need to see if the runID exists in the 
   // DB. If we didn't load it from there, then we shouldn't be able to sync
   int recStatus = US_ConvertIO::checkRunID( ExpData.runID );
   if ( recStatus == -1 )
   {
      // We can't sync to the DB if we can't connect to it
      pb_savetoDB ->setEnabled( false );
      return;
   }

   // if a record is found but saveStatus == BOTH, then we are editing that record
   else if ( ( recStatus > 0 ) && ( saveStatus != BOTH ) ) 
   {
      pb_savetoDB ->setEnabled( false );
      return;
   }

   // If we made it here, user can sync with DB
   pb_savetoDB ->setEnabled( true );
}

// Process when the user changes the runID
void US_Convert::runIDChanged( void )
{
   // See if we need to update the runID
   QRegExp rx( "^[A-Za-z0-9_-]{1,20}$" );
   QString new_runID = le_runID->text();
      
   if ( rx.indexIn( new_runID ) >= 0 )
   {
      runID = new_runID;
      plot_titles();
   }

   // If the runID has changed, a number of other things need to change too,
   // for instance GUID's.
   ExpData.clear();
   foreach( TripleInfo triple, triples )
      triple.clear();
   le_runID->setText( runID );
   ExpData.runID = runID;

   saveStatus = NOT_SAVED;

   // Set the directory too
   QDir resultDir( US_Settings::resultDir() );
   currentDir = resultDir.absolutePath() + "/" + runID + "/";
   le_dir ->setText( currentDir );
}

// Function to generate a new guid for experiment, and associate with DB
void US_Convert::editRuninfo( void )
{
   // Verify connectivity
   US_Passwd pw;
   QString masterPW = pw.getPasswd();
   US_DB2 db( masterPW );

   if ( db.lastErrno() != US_DB2::OK )
   {
      QMessageBox::information( this,
             tr( "Error" ),
             tr( "This is the dialog where you associate the run information " ) +
             tr( "with the database. Please verify your "     ) +
             tr( "connection parameters before proceeding. You can still " ) +
             tr( "create your auc file and do this later.\n" ) );
      return;
   }

   if ( saveStatus == NOT_SAVED )
   {
      // First time for this data, so clear ExpData out
      ExpData.clear();
   
      // Create a new GUID for the experiment as a whole
      uuid_t uuid;
      char uuidc[ 37 ];
   
      uuid_generate( uuid );
      uuid_unparse( (unsigned char*)uuid, uuidc );
   
      ExpData.expGUID = QString( uuidc );

      saveStatus = EDITING;
   }

   getExpInfo( );
}

void US_Convert::loadUS3HD( void )
{
   // Ask for data directory
   QString dir = QFileDialog::getExistingDirectory( this, 
         tr("US3 Raw Data Directory"),
         US_Settings::resultDir(),
         QFileDialog::DontResolveSymlinks );

   if ( dir.isEmpty() ) return; 

   dir.replace( "\\", "/" );  // WIN32 issue
   if ( dir.right( 1 ) != "/" ) dir += "/"; // Ensure trailing /

   loadUS3HD( dir );
}

void US_Convert::loadUS3HD( QString dir )
{
   resetAll();

   // Set the runID and directory
   QStringList components =  dir.split( "/", QString::SkipEmptyParts );  
   runID    = components.last();
   le_runID ->setText( runID );
   le_dir   ->setText( dir );
   currentDir  = QString( dir );

   // Reload the data
   US_ProcessConvert* dialog 
      = new US_ProcessConvert( this );
      dialog->reloadUS3Data( dir, allData, ExpData, triples, runType, runID );
   delete dialog;

   if ( allData.size() == 0 ) return;

   // Update triple information on screen
   setTripleInfo();

   le_solutionDesc  -> setText( triples[ currentTriple ].solution.solutionDesc  );

   // Restore description
   le_description->setText( allData[ 0 ].description );
   saveDescription = QString( allData[ 0 ].description );

   // Reset maximum scan control values
   enableScanControls();

   // The centerpiece combo box
   cb_centerpiece->setLogicalIndex( triples[ currentTriple ].centerpiece );

   // Redo plot
   init_excludes();
   plot_current();

   connect( ct_from, SIGNAL( valueChanged ( double ) ),
                     SLOT  ( focus_from   ( double ) ) );

   connect( ct_to  , SIGNAL( valueChanged ( double ) ),
                     SLOT  ( focus_to     ( double ) ) );

   // Ok to enable some buttons now
   enableControls();
   pb_savetoHD    ->setEnabled( true );
   pb_details     ->setEnabled( true );
   pb_solution    ->setEnabled( true );
   cb_centerpiece ->setEnabled( true );
   pb_editRuninfo ->setEnabled( true );

   // some things one can't do from here
   ct_tolerance   ->setEnabled( false );

   enableRunIDControl( false );

   if ( runType == "RI" )
      pb_reference->setEnabled( true );

   else if ( runType == "RA" && ss_limits.size() < 2 )
   {
      // Allow user to define subsets, if he hasn't already
      pb_define   ->setEnabled( true );
   } 

   saveStatus = ( ExpData.expID == 0 ) ? HD_ONLY : DB_SYNC;
   //pb_editRuninfo  ->setEnabled ( saveStatus == HD_ONLY );

   enableSyncDB();
}

// Function to load an experiment from the DB
void US_Convert:: loadUS3DB( void )
{
   // Verify connectivity
   US_Passwd pw;
   QString masterPW = pw.getPasswd();
   US_DB2 db( masterPW );

   if ( db.lastErrno() != US_DB2::OK )
   {
      QMessageBox::information( this,
             tr( "Error" ),
             tr( "Error making the DB connection.\n" ) );
      return;
   }

   // Present a dialog to ask user which experiment to load
   QString runID;
   US_GetDBRun dialog( runID );
   if ( dialog.exec() == QDialog::Rejected )
      return;

   if ( runID == QString( "" ) )
      return;

   // Now that we have the runID, let's copy the DB info to HD
   QDir        readDir( US_Settings::resultDir() );
   QString     dirname = readDir.absolutePath() + "/" + runID + "/";
   QString status = US_ConvertIO::readDBExperiment( runID, dirname );
   if ( status  != QString( "" ) )
   {
      QMessageBox::information( this,
             tr( "Error" ),
             status + "\n" );
      return;
   }

   // and load it
   loadUS3HD( dirname );
   enableControls();

   saveStatus = BOTH;         // override from loadUS3HD()
}

void US_Convert::getExpInfo( void )
{
   ExpData.runID = le_runID -> text();

   // Check if the run ID already exists in the DB
   int recStatus = US_ConvertIO::checkRunID( ExpData.runID );
   if ( recStatus == -1 )
   {
      QMessageBox::information( this,
             tr( "Error" ),
             tr( "Error making the DB connection.\n" ) );
      return;
   }

   // if saveStatus == BOTH, then we are editing the record from the database
   else if ( ( recStatus > 0 ) && ( saveStatus != BOTH ) ) 
   {
      QMessageBox::information( this,
             tr( "Error" ),
             tr( "The current runID already exists in the database. To edit that "
                 "information, load it from the database to start with.\n" ) );
      return;
   }

   // OK, proceed

   // Calculate average temperature
   double sum = 0.0;
   double count = 0.0;
   for ( int i = 0; i < allData.size(); i++ )
   {
      US_DataIO2::RawData raw = allData[ i ];
      for ( int j = 0; j < raw.scanData.size(); j++ )
         sum += raw.scanData[ j ].temperature;

      count += raw.scanData.size();
   }

   // Load information we're passing
   ExpData.runTemp       = QString::number( sum / count );
   char* optSysPtr       = ExpData.opticalSystem.data();
   strncpy( optSysPtr, allData[ 0 ].type, 2 );
   optSysPtr[ 2 ] = '\0';

   // A list of unique rpms
   ExpData.rpms.clear();
   for ( int i = 0; i < allData.size(); i++ )
   {
      US_DataIO2::RawData raw = allData[ i ];
      for ( int j = 0; j < raw.scanData.size(); j++ )
      {
         if ( ! ExpData.rpms.contains( raw.scanData[ j ].rpm ) )
            ExpData.rpms << raw.scanData[ j ].rpm;
      }
   }

   // Now sort the rpm list.  Use a modified bubble sort;
   // the list might already be almost ordered
   bool done = false;
   while ( !done )
   {
      done = true;
      for ( int i = 0; i < ExpData.rpms.size() - 1; i++ )
      {
         if ( ExpData.rpms[ i ] > ExpData.rpms[ i + 1 ] )
         {
            ExpData.rpms.swap( i, i + 1 );
            done = false;
         }
      }
   }

   US_ExpInfo* expInfo = new US_ExpInfo( ExpData );

   connect( expInfo, SIGNAL( updateExpInfoSelection( US_ExpInfo::ExperimentInfo& ) ),
            this   , SLOT  ( updateExpInfo         ( US_ExpInfo::ExperimentInfo& ) ) );

   connect( expInfo, SIGNAL( cancelExpInfoSelection() ),
            this   , SLOT  ( cancelExpInfo         () ) );

   expInfo->exec();
}

// Updating after user has selected info from experiment dialog
void US_Convert::updateExpInfo( US_ExpInfo::ExperimentInfo& d )
{
   // Update local copy
   ExpData = d;

   enableControls();
}

void US_Convert::cancelExpInfo( void )
{
   ExpData.clear();
   this->saveStatus = NOT_SAVED;
   enableControls();
}

void US_Convert::getSolutionInfo( void )
{
   ExpData.runID = le_runID -> text();

   US_Solution solution = triples[ currentTriple ].solution;

   US_SolutionGui* solutionInfo = new US_SolutionGui( ExpData.invID,
                                                      ExpData.expID,
                                                      1,      // channelID (figure out later ?? )
                                                      true,   // signal wanted
                                                      solution );

   connect( solutionInfo, SIGNAL( updateSolutionGuiSelection( US_Solution& ) ),
            this,         SLOT  ( updateSolutionInfo        ( US_Solution& ) ) );

   connect( solutionInfo, SIGNAL( cancelSolutionGuiSelection() ),
            this,         SLOT  ( cancelSolutionInfo        () ) );

   solutionInfo->exec();
}

// Updating after user has selected info from experiment dialog
void US_Convert::updateSolutionInfo( US_Solution& s )
{
   // Update local copy
   triples[ currentTriple ].solution = s;

   le_solutionDesc  ->setText( triples[ currentTriple ].solution.solutionDesc  );

   plot_current();

   enableControls();
}

void US_Convert::cancelSolutionInfo( void )
{
   enableControls();
}

// Function to copy the current triple's data to all triples
void US_Convert::tripleApplyAll( void )
{
   TripleInfo triple = triples[ currentTriple ];

   // Copy selected fields only
   for ( int i = 0; i < triples.size(); i++ )
   {
      if ( triples[ i ].excluded ) continue;

      triples[ i ].centerpiece  = triple.centerpiece;
      triples[ i ].solution     = triple.solution;
   }

   QMessageBox::information( this, tr( "C/c/w Apply to All" ),
         tr( "The current c/c/w information has been copied to all\n" ) );

   plot_current();

   enableControls();
}

void US_Convert::runDetails( void )
{
   // Create a triples structure for US_RunDetails2
   QStringList tripleDescriptions;
   for (int i = 0; i < triples.size(); i++ )
   {
      if ( triples[ i ].excluded ) continue;

      tripleDescriptions << triples[ i ].tripleDesc;
   }

   US_RunDetails2* dialog
      = new US_RunDetails2( allData, runID, currentDir, tripleDescriptions );
   dialog->exec();
   qApp->processEvents();
   delete dialog;
}

void US_Convert::changeTriple( QListWidgetItem* )
{
   // Match the description to find the correct triple in memory
   QString triple = lw_triple->currentItem()->text();
   currentTriple = 0;
   for ( int i = 0; i < triples.size(); i++ )
   {
      if ( triple == triples[ i ].tripleDesc )
         currentTriple = i;
   }
   
   le_dir          -> setText( currentDir );
   le_description  -> setText( allData[ currentTriple ].description );
   le_solutionDesc -> setText( triples[ currentTriple ].solution.solutionDesc  );
   
   // Reset maximum scan control values
   enableScanControls();
   
   // The centerpiece combo box
   cb_centerpiece->setLogicalIndex( triples[ currentTriple ].centerpiece );
   
   // Redo plot
   plot_current();
}

void US_Convert::setTripleInfo( void )
{
   // Load them into the list box
   lw_triple->clear();
   currentTriple = -1;          // indicates that it hasn't been selected yet
   for (int i = 0; i < triples.size(); i++ )
   {
      if ( triples[ i ].excluded ) continue;

      lw_triple->addItem( triples[ i ].tripleDesc );
      if ( currentTriple == -1 ) currentTriple = i;
   }

   QListWidgetItem* item = lw_triple->item( 0 );   // select the item at row 0
   lw_triple->setItemSelected( item, true );
}

void US_Convert::getCenterpieceIndex( int )
{
   triples[ currentTriple ].centerpiece = cb_centerpiece->getLogicalID();
}

void US_Convert::focus_from( double scan )
{
   int from = (int)scan;
   int to   = (int)ct_to->value();

   if ( from > to )
   {
      ct_to->disconnect();
      ct_to->setValue( scan );
      to = from;
      
      connect( ct_to, SIGNAL( valueChanged ( double ) ),
                      SLOT  ( focus_to     ( double ) ) );
   }

   focus( from, to );
}

void US_Convert::focus_to( double scan )
{
   int to   = (int)scan;
   int from = (int)ct_from->value();

   if ( from > to )
   {
      ct_from->disconnect();
      ct_from->setValue( scan );
      from = to;
      
      connect( ct_from, SIGNAL( valueChanged ( double ) ),
                        SLOT  ( focus_from   ( double ) ) );
   }

   focus( from, to );
}

void US_Convert::focus( int from, int to )
{
   if ( from == 0 )
   {
      pb_exclude->setEnabled( false );
      pb_include->setEnabled( false );
   }
   else
   {
      pb_exclude->setEnabled( true );
      pb_include->setEnabled( true );
   }

   QList< int > focus;  // We don't care if -1 is in the list
   for ( int i = from - 1; i <= to - 1; i++ ) focus << i;  

   set_colors( focus );

}

// Function to initialize the excluded scan count
void US_Convert::init_excludes( void )
{
   allExcludes.clear();
   Excludes x;
   for ( int i = 0; i < allData.size(); i++ )
      allExcludes << x;
}

// Function to exclude user-selected scans
void US_Convert::exclude_scans( void )
{
   int scanStart = (int)ct_from->value();
   int scanEnd   = (int)ct_to  ->value();

   // Create a new list temporarily, to avoid accounting for them twice
   QList< int > newExcludes;
   newExcludes.clear();

   for ( int i = scanStart - 1; i < scanEnd; i++ )
   {
      // Find all lower-numbered excluded scans, to account for them
      int excludeCount = 0;
      for ( int j = 0; j < scanStart; j++ )
         if ( allExcludes[ currentTriple ].contains( j ) )
            excludeCount++;

      // Make a list of current exclusions, accounting for previous ones
      newExcludes << ( i + excludeCount );
   }

   // Now add new excludes to existing ones
   for ( int i = 0; i < newExcludes.size(); i++ )
   {
      if ( ! allExcludes[ currentTriple ].contains( newExcludes[ i ] ) )
         allExcludes[ currentTriple ] << newExcludes[ i ];
   }

   enableScanControls();

   replot();
}

void US_Convert::include( void )
{
   init_excludes();
   enableScanControls();

   pb_include->setEnabled( false );
   pb_exclude->setEnabled( false );

   replot();
}

// User pressed the define subsets button while processing equil-abs data
void US_Convert::define_subsets( void )
{
   ss_limits.clear();

   pb_process ->setEnabled( true );

   connect( picker, SIGNAL( cMouseUp( const QwtDoublePoint& ) ),
                    SLOT  ( cClick  ( const QwtDoublePoint& ) ) );

   step = SPLIT;

}

void US_Convert::process_subsets( void )
{
   pb_process ->setEnabled( false );
   pb_define  ->setEnabled( false );
   picker   ->disconnect();

   if ( ss_limits.size() < 2 )
   {
      // Not enough clicks to work with
      ss_limits.clear();
      pb_process ->setEnabled( true );
      return;
   }

   // Let's make sure the points are in sorted order
   for ( int i = 0; i < ss_limits.size() - 1; i++ )
      for ( int j = i + 1; j < ss_limits.size(); j++ )
         if ( ss_limits[ i ] > ss_limits[ j ] )
         {
            double temp = ss_limits[ i ];
            ss_limits[ i ] = ss_limits[ j ];
            ss_limits[ j ] = temp;
         }

   // Let's make sure all the data is included somewhere
   ss_limits[ 0 ] = 5.7;
   ss_limits[ ss_limits.size() - 1 ] = 7.3;

   // Now that we know we're subdividing, let's reconvert the file
   reset();
   load( currentDir );

   // We don't need this any more, and it interferes with subsequent
   //  loads of RA data. 
   ss_limits.clear();
}

// User pressed the Define reference button while analyzing intensity data
void US_Convert::define_reference( void )
{
   connect( picker, SIGNAL( cMouseUp( const QwtDoublePoint& ) ),
                    SLOT  ( cClick  ( const QwtDoublePoint& ) ) );

   reference_start = 0.0;
   reference_end   = 0.0;
   pb_reference ->setEnabled( false );

   step = REFERENCE;
}

// Select starting point of reference scan in intensity data
void US_Convert::start_reference( const QwtDoublePoint& p )
{
   reference_start   = p.x();

   draw_vline( reference_start );
   data_plot->replot();
}

// Select end point of reference scan in intensity data
void US_Convert::process_reference( const QwtDoublePoint& p )
{
   // Just in case we get a second click message right away
   if ( fabs( p.x() - reference_start ) < 0.005 ) return;

   reference_end = p.x();
   draw_vline( reference_end );
   data_plot->replot();

   pb_reference  ->setEnabled( false );
   picker        ->disconnect();

   // Double check if min < max
   if ( reference_start > reference_end )
   {
      double temp     = reference_start;
      reference_start = reference_end;
      reference_end   = temp;
   }

   // Calculate the averages for all triples
   PseudoCalcAvg();

   // Now that we have the averages, let's replot
   Pseudo_reference_triple = currentTriple;

   // Default to displaying the first non-reference triple
   for ( int i = 0; i < allData.size(); i++ )
   {
      if ( i != Pseudo_reference_triple )
      {
         currentTriple = i;
         break;
      }
   }

   lw_triple->setCurrentRow( currentTriple );
   plot_current();
}

// Process a control-click on the plot window
void US_Convert::cClick( const QwtDoublePoint& p )
{
   switch ( step )
   {
      case SPLIT :
         // process equil-abs data
         draw_vline( p.x() );
         ss_limits << p.x();
         break;

      case REFERENCE :
         // process reference scan
         if ( reference_start == 0.0 )
            start_reference( p );

         else
            process_reference( p );
     
      default :
         break;

   }
}

void US_Convert::PseudoCalcAvg( void )
{
   if ( Pseudo_averaged ) return;             // Average calculation has already been done

   US_DataIO2::RawData referenceData = allData[ currentTriple ];
   int ref_size = referenceData.scanData[ 0 ].readings.size();

   for ( int i = 0; i < referenceData.scanData.size(); i++ )
   {
      US_DataIO2::Scan s = referenceData.scanData[ i ];

      int j      = 0;
      int count  = 0;
      double sum = 0.0;
      //while ( referenceData.x[ j ].radius < reference_start && j < ref_size )
      while ( referenceData.radius( j ) < reference_start && j < ref_size )
         j++;

      //while ( referenceData.x[ j ].radius < reference_end && j < ref_size )
      while ( referenceData.radius( j ) < reference_end && j < ref_size )
      {
         sum += s.readings[ j ].value;
         count++;
         j++;
      }

      Pseudo_averages << sum / count;
   }
   
   // Now average around excluded values
   int lastGood  = 0;
   int countBad  = 0;
   for ( int i = 0; i < Pseudo_averages.size(); i++ )
   {
      // In case there are adjacent excluded scans...
      if ( allExcludes[ currentTriple ].contains( i ) )
         countBad++;

      // Calculate average of before and after for intervening values
      else if ( countBad > 0 )
      {
         double newAvg = ( Pseudo_averages[ lastGood ] + Pseudo_averages[ i ] ) / 2.0;
         for ( int k = lastGood + 1; k < i; k++ )
            Pseudo_averages[ k ] = newAvg;

         countBad = 0;
      }

      // Normal situation -- value is not excluded
      else
         lastGood = i;

   }

   // Now calculate the pseudo-absorbance
   RIData = allData;

   for ( int i = 0; i < allData.size(); i++ )
   {
      US_DataIO2::RawData* currentData = &allData[ i ];

      for ( int j = 0; j < currentData->scanData.size(); j++ )
      {
         US_DataIO2::Scan* s = &currentData->scanData[ j ];

         for ( int k = 0; k < s->readings.size(); k++ )
         {
            US_DataIO2::Reading* r = &s->readings[ k ];

            r->value = log10(Pseudo_averages[ j ] / r->value );
         }
      }

      // Let's mark pseudo-absorbance as different from RI data,
      //  since it needs some different processing in some places
      isPseudo = true;
   }

   // Enable intensity plot
   pb_intensity->setEnabled( true );

   Pseudo_averaged = true;
   pb_cancelref ->setEnabled( true );
}

// Bring up a graph window showing the intensity profile
void US_Convert::show_intensity( void )
{
   US_Intensity* dialog
      = new US_Intensity( ( const QVector< double > ) Pseudo_averages );
   dialog->exec();
   qApp->processEvents();
   delete dialog;
}

void US_Convert::cancel_reference( void )
{
   Pseudo_averaged = false;
   allData     = RIData;
   RIData.clear();

   Pseudo_averages.clear();
   reference_start = 0.0;
   reference_end   = 0.0;

   for ( int i = 0; i < triples.size(); i++ )
      triples[ i ].excluded = false;

   setTripleInfo();

   pb_reference  ->setEnabled( true );
   pb_cancelref  ->setEnabled( false );
   pb_intensity  ->setEnabled( false );
   currentTriple = 0;
   lw_triple->setCurrentRow( currentTriple );

   plot_current();
}

void US_Convert::drop_reference( void )
{
   triples[ currentTriple ].excluded = true;
   setTripleInfo();           // Resets currentTriple

   le_dir          -> setText( currentDir );
   le_description  -> setText( saveDescription );
   le_solutionDesc -> setText( triples[ currentTriple ].solution.solutionDesc  );

   enableControls();

   // Reset maximum scan control values
   enableScanControls();

   // The centerpiece combo box
   cb_centerpiece->setLogicalIndex( triples[ currentTriple ].centerpiece );

   // Redo plot
   plot_current();
}

int US_Convert::savetoHD( void )
{
   if ( allData[ 0 ].scanData.empty() ) return NODATA; 

   QDir        writeDir( US_Settings::resultDir() );
   QString     dirname = writeDir.absolutePath() + "/" + runID + "/";

   if ( saveStatus == NOT_SAVED && writeDir.exists( runID ) )
   {
        QMessageBox::information( this,
              tr( "Error" ),
              tr( "The write directory,  " ) + dirname + 
              tr( " already exists. Please change run ID to a unique value." ) );
        return DUP_RUNID;
   }

   if ( ! writeDir.exists( runID ) )
   {
      if ( ! writeDir.mkdir( runID ) )
      {
         QMessageBox::information( this,
               tr( "Error" ),
               tr( "Cannot write to " ) + writeDir.absolutePath() );
         return CANTOPEN;
      }
   }

   int status;

   // Check to see if we have all the data to write
   if ( ExpData.invID == 0 )
   {
      status = QMessageBox::information( this,
               tr( "Warning" ),
               tr( "The run has not yet been associated with the database. "    ) +
               tr( "Click 'OK' to proceed anyway, or click 'Cancel' "           ) +
               tr( "and then click on the 'Edit Run Information'"              ) +
               tr( "button to enter this information first.\n\n "               ),
               tr( "&OK" ), tr( "&Cancel" ),
               0, 0, 1 );
      if ( status != 0 ) return NOT_WRITTEN;
   }

   // Write the data
   bool saveGUIDs = saveStatus != NOT_SAVED && saveStatus != EDITING;
   US_ProcessConvert* dialog 
      = new US_ProcessConvert( this );
      dialog->writeConvertedData( status, allData, ExpData, triples, 
                                  allExcludes, runType, runID, dirname, saveGUIDs );
   delete dialog;

   // How many files should have been written?
   int fileCount = 0;
   for ( int i = 0; i < triples.size(); i++ )
      if ( ! triples[ i ].excluded ) fileCount++;

   // Now try to communicate status
   if ( status == NOXML )
   {
      // Main xml data is missing
      QMessageBox::information( this,
            tr( "Warning" ),
            tr( "The run information file was not written. " ) +
            tr( "Please click on the " ) +
            tr( "'Associate Run with DB' button \n\n " )    +
            QString::number( fileCount ) + " "                + 
            runID + tr( " files written." ) );
      return( status );
   }

   else if ( status == PARTIAL_XML )
   {
      // xml data is missing for one or more triples
      QMessageBox::information( this,
            tr( "Warning" ),
            tr( "Solution information is incomplete. Please click on the " ) +
            tr( "'Manage Solutions' button for each "  ) +
            tr( "cell, channel, and wavelength combination \n\n " ) +
            QString::number( fileCount ) + " "                + 
            runID + tr( " files written." ) );
      return( status );
   }

   else if ( status != OK )
   {
      // Error from writeData.
      // Try to delete the file and tell the user
      return( status );
   }

   // Status is OK
   QMessageBox::information( this,
         tr( "Success" ),
         QString::number( fileCount ) + " " + 
         runID + tr( " files written." ) );
  
   if ( saveStatus == NOT_SAVED || saveStatus == EDITING )
      saveStatus = HD_ONLY;

   enableRunIDControl( false );

   return( OK );
}

void US_Convert::savetoDB( void )
{
   int status = savetoHD();
   if ( status != OK )
      return;

   QString error = QString( "" );

   // Get the directory where the auc files are
   QDir        resultDir( US_Settings::resultDir() );
   QString     dir = resultDir.absolutePath() + "/" + ExpData.runID + "/";

   if ( ! resultDir.exists( ExpData.runID ) )
   {
      QMessageBox::information( this,
            tr( "Error" ),
            tr( "Cannot read from " ) + dir ); // aucDir.absolutePath() );
      return;
   }

   QStringList nameFilters = QStringList( "*.auc" );

   QDir readDir( dir );

   QStringList files =  readDir.entryList( nameFilters, 
         QDir::Files | QDir::Readable, QDir::Name );

   if ( files.size() == 0 )
   {
      QMessageBox::warning( this,
            tr( "No Files Found" ),
            tr( "There were no files of the form *.auc\n"  
                "found in the specified directory." ) );
      return;
   }

   // Save it to the db and return
   int expID = US_ConvertIO::checkRunID( ExpData.runID );
   if ( expID == -1 )
      error =  "Error making DB connection";

   else if ( expID == 0 )
   {
      // No database records with this runID found
      error = US_ConvertIO::newDBExperiment( ExpData, triples, dir );
   }

   // If saveStatus == BOTH already, then it came from the db to begin with, so it's
   // ok to update it
   else if ( saveStatus == BOTH )
   {
      ExpData.expID = expID;
      error = US_ConvertIO::updateDBExperiment( ExpData, triples, dir );
   }

   else
   {
      // User is trying to overwrite a runID that is already in the DB
      QMessageBox::warning( this,
            tr( "Duplicate runID" ),
            tr( "This runID already exists in the database. To edit that "  
                "run information, load it from there to begin with.\n" ) );
      return;
   }

   if ( error != QString( "" ) )
   {
      db_error( error );
      return;
   }

   QMessageBox::information( this, tr( "Record saved" ),
         tr( "The run information has been saved to the DB successfully \n" ) );

   saveStatus = BOTH;
   enableRunIDControl( false );

}

bool US_Convert::read( void )
{
   // Ask for data directory
   QString dir = QFileDialog::getExistingDirectory( this, 
         tr( "Raw Data Directory" ),
         US_Settings::dataDir(),
         QFileDialog::DontResolveSymlinks );

   if ( dir.isEmpty() ) return( false ); 

   dir.replace( "\\", "/" );  // WIN32 issue

   return( read( dir ) );
}

bool US_Convert::read( QString dir )
{
   // Get legacy file names
   QDir d( dir, "*", QDir::Name, QDir::Files | QDir::Readable );
   d.makeAbsolute();
   if ( dir.right( 1 ) != "/" ) dir += "/"; // Ensure trailing /

   // Set the runID and directory
   QStringList components = dir.split( "/", QString::SkipEmptyParts );
   runID    = components.last();
   le_runID ->setText( runID );
   le_dir   ->setText( dir );
   currentDir  = QString( dir );

   oldRunType = runType;            // let's see if the runType changes

   // Read the data
   US_ProcessConvert* dialog 
      = new US_ProcessConvert( this );
      dialog->readLegacyData( dir, legacyData, runType );
   delete dialog;

   if ( legacyData.size() == 0 ) return( false );

   // if runType has changed, let's clear out xml data too
   if ( oldRunType != runType ) ExpData.clear();

   return( true );
}

bool US_Convert::convert( void )
{
   double tolerance = (double)ct_tolerance->value() + 0.05;    // to stay between wl numbers

   // Convert the data
   US_ProcessConvert* dialog 
      = new US_ProcessConvert( this );
      dialog->convertLegacyData( legacyData, allData, triples, 
                                 runType, tolerance, ss_limits );
   delete dialog;

   if ( allData.size() == 0 ) return( false );

   le_description->setText( allData[ 0 ].description );
   saveDescription = QString( allData[ 0 ].description ); 

   // Now let's show the user the first one
   currentTriple = -1;
   for ( int i = 0; i < triples.size(); i++ )
   {
      if ( triples[ i ].excluded ) continue;

      if ( currentTriple == -1 ) currentTriple = i;
   }

   return( true );
}

bool US_Convert::centerpieceInfo( void )
{
   // Verify connectivity
   US_Passwd pw;
   QString masterPW = pw.getPasswd();
   US_DB2 db( masterPW );

   if ( db.lastErrno() != US_DB2::OK )
   {
      QMessageBox::information( this,
             tr( "Error" ),
             tr( "Database connectivity error" ) );

      return( false );
   }

   QStringList q( "get_abstractCenterpiece_names" );
   db.query( q );

   QList<listInfo> options;
   while ( db.next() )
   {
      struct listInfo option;
      option.ID      = db.value( 0 ).toString();
      option.text    = db.value( 1 ).toString();
      options << option;
   }

   cb_centerpiece->clear();
   if ( options.size() > 0 )
      cb_centerpiece->addOptions( options );

   return true;
}

void US_Convert::plot_current( void )
{
   US_DataIO2::RawData currentData = allData[ currentTriple ];

   if ( currentData.scanData.empty() ) return;

   plot_titles();

   // Plot current data for cell / channel / wavelength triple
   plot_all();
   
   // Set the Scan spin boxes
   enableScanControls();
}

void US_Convert::plot_titles( void )
{
   US_DataIO2::RawData currentData = allData[ currentTriple ];

   QString triple         = triples[ currentTriple ].tripleDesc;
   QStringList parts      = triple.split(" / ");

   QString     cell       = parts[ 0 ];
   QString     channel    = parts[ 1 ];
   QString     wl         = parts[ 2 ];

   // Plot Title and legends
   QString title;
   QString xLegend = "Radius (in cm)";
   QString yLegend = "Absorbance";

   if ( strncmp( currentData.type, "RA", 2 ) == 0 )
   {
      title = "Radial Absorbance Data\nRun ID: "
            + runID + " Cell: " + cell + " Wavelength: " + wl;
   }

   else if ( ( strncmp( currentData.type, "RI", 2 ) == 0 ) && isPseudo )
   {
      title = "Pseudo Absorbance Data\nRun ID: "
            + runID + " Cell: " + cell + " Wavelength: " + wl;
   }

   else if ( strncmp( currentData.type, "RI", 2 ) == 0 )
   {
      title = "Radial Intensity Data\nRun ID: "
            + runID + " Cell: " + cell + " Wavelength: " + wl;
      yLegend = "Radial Intensity";
   }

   else if ( strncmp( currentData.type, "IP", 2 ) == 0 )
   {
      title = "Interference Data\nRun ID: "
            + runID + " Cell: " + cell + " Wavelength: " + wl;
      yLegend = "Fringes";
   }

   else if ( strncmp( currentData.type, "FI", 2 ) == 0 )
   {
      title = "Fluorescence Intensity Data\nRun ID: "
            + runID + " Cell: " + cell + " Wavelength: " + wl;
      yLegend = "Fluorescence Intensity";
   }
      
   else if ( strncmp( currentData.type, "WA", 2 ) == 0 )
   {
      title = "Wavelength Data\nRun ID: "
            + runID + " Cell: " + cell + " Radius: " + wl;
      xLegend = "Wavelength";
      yLegend = "Value";
   }

   else if ( strncmp( currentData.type, "WI", 2 ) == 0 )
   {
      title = "Wavelength Intensity Data\nRun ID: "
            + runID + " Cell: " + cell + " Radius: " + wl;
      xLegend = "Wavelength";
      yLegend = "Value";
   }

   else
      title = "File type not recognized";
   
   data_plot->setTitle( title );
   data_plot->setAxisTitle( QwtPlot::yLeft, yLegend );
   data_plot->setAxisTitle( QwtPlot::xBottom, xLegend );

}

void US_Convert::plot_all( void )
{
   US_DataIO2::RawData currentData = allData[ currentTriple ];

   data_plot->detachItems();
   grid = us_grid( data_plot );

   int size = currentData.scanData[ 0 ].readings.size();

   double* r = new double[ size ];
   double* v = new double[ size ];

   double maxR = -1.0e99;
   double minR =  1.0e99;
   double maxV = -1.0e99;
   double minV =  1.0e99;

   for ( int i = 0; i < currentData.scanData.size(); i++ )
   {
      Excludes currentExcludes = allExcludes[ currentTriple ];
      if ( currentExcludes.contains( i ) ) continue;
      US_DataIO2::Scan* s = &currentData.scanData[ i ];

      for ( int j = 0; j < size; j++ )
      {
         r[ j ] = currentData.radius( j );
         v[ j ] = s->readings  [ j ].value;

         if ( v[ j ] > 1.0e99 )
         {
            // For some reason v[j] is going off the scale
            // Don't know why, but filter out for now
            qDebug() << "(r, v) = ( " << r[j] << ", " << v[j] << ")"
                     << " (minR, maxR) = ( " << minR << ", " << maxR << ")" << endl
                     << " (minV, maxV) = ( " << minV << ", " << maxV << ")" << endl;
            continue;
         }

         maxR = max( maxR, r[ j ] );
         minR = min( minR, r[ j ] );
         maxV = max( maxV, v[ j ] );
         minV = min( minV, v[ j ] );
      }

      QString title = tr( "Raw Data at " )
         + QString::number( s->seconds ) + tr( " seconds" );

      QwtPlotCurve* c = us_curve( data_plot, title );
      c->setData( r, v, size );

   }

   // Reset the scan curves within the new limits
   double padR = ( maxR - minR ) / 30.0;
   double padV = ( maxV - minV ) / 30.0;
   
   data_plot->setAxisScale( QwtPlot::yLeft  , minV - padV, maxV + padV );
   data_plot->setAxisScale( QwtPlot::xBottom, minR - padR, maxR + padR );
   
   show_plot_progress = false;
   data_plot->replot();
 
   delete [] r;
   delete [] v;
}

void US_Convert::replot( void )
{
  plot_all();
}

void US_Convert::set_colors( const QList< int >& focus )
{
   // Get pointers to curves
   QwtPlotItemList        list = data_plot->itemList();
   QList< QwtPlotCurve* > curves;
  
   for ( int i = 0; i < list.size(); i++ )
   {
      if ( list[ i ]->title().text().contains( "Raw" ) )
         curves << dynamic_cast< QwtPlotCurve* >( list[ i ] );
   }
  
   QPen   p   = curves[ 0 ]->pen();
   QBrush b   = curves[ 0 ]->brush();
   QColor std = US_GuiSettings::plotCurve();
   
   // Mark these scans in red
   for ( int i = 0; i < curves.size(); i++ )
   {
      if ( focus.contains( i ) )
      {
         p.setColor( Qt::red );
      }
      else
      {
         p.setColor( std );
         b.setColor( std );
      }

      curves[ i ]->setPen  ( p );
      curves[ i ]->setBrush( b );
   }

   data_plot->replot();
}

void US_Convert::draw_vline( double radius )
{
   double r[ 2 ];

   r[ 0 ] = radius;
   r[ 1 ] = radius;
   QwtScaleDiv* y_axis = data_plot->axisScaleDiv( QwtPlot::yLeft );

   double padding = ( y_axis->upperBound() - y_axis->lowerBound() ) / 30.0;

   double v[ 2 ];
   v [ 0 ] = y_axis->upperBound() - padding;
   v [ 1 ] = y_axis->lowerBound() + padding;

   QwtPlotCurve* v_line = us_curve( data_plot, "V-Line" );
   v_line->setData( r, v, 2 );

   QPen pen = QPen( QBrush( Qt::white ), 2.0 );
   v_line->setPen( pen );

   data_plot->replot();
}

void US_Convert::db_error( const QString& error )
{
   QMessageBox::warning( this, tr( "Database Problem" ),
         tr( "Database returned the following error: \n" ) + error );
}

// Initializations
US_Convert::TripleInfo::TripleInfo()
{
   clear();
}

void US_Convert::TripleInfo::clear( void )
{
   tripleID     = 0;
   tripleDesc   = QString( "" );
   description  = QString( "" );
   excluded     = false;
   centerpiece  = 0;
   memset( tripleGUID, 0, 16 );
   tripleFilename = QString( "" );
   solution.clear();
}

void US_Convert::TripleInfo::show( void )
{
   char uuidc[ 37 ];
   uuid_unparse( (unsigned char*)tripleGUID, uuidc );
   
   qDebug() << "tripleID     = " << tripleID     << '\n'
            << "tripleDesc   = " << tripleDesc   << '\n'
            << "description  = " << description  << '\n'
            << "centerpiece  = " << centerpiece  << '\n'
            << "tripleGUID   = " << QString( uuidc )  << '\n'
            << "tripleFilename = " << tripleFilename << '\n'
            << "solutionID   = " << QString::number( solution.solutionID ) << '\n'
            << "solutionGUID = " << solution.solutionGUID << '\n'
            << "solutionDesc = " << solution.solutionDesc;

   if ( excluded ) qDebug() << "excluded";
}
