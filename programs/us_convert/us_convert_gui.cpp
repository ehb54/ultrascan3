#include <QApplication>

#include "us_license_t.h"
#include "us_license.h"
#include "us_util.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_run_details2.h"
#include "us_plot.h"
#include "us_math2.h"
#include "us_convert_gui.h"
#include "us_convertio.h"
#include "us_experiment_gui.h"
#include "us_solution_gui.h"
#include "us_db2.h"
#include "us_passwd.h"
#include "us_intensity.h"
#include "us_get_dbrun.h"
#include "us_investigator.h"
#include "us_constants.h"
#include "us_report.h"
#include "us_gui_util.h"

#ifdef WIN32
  #include <float.h>
  #define isnan _isnan
#endif

int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );

   #include "main1.inc"

   // License is OK.  Start up.
   
   US_ConvertGui w;
   w.show();                   //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication
}

US_ConvertGui::US_ConvertGui() : US_Widgets()
{
   ExpData.invID = US_Settings::us_inv_ID();

   // Ensure data directories are there
   QDir dir;
   dir.mkpath( US_Settings::dataDir()    );
   dir.mkpath( US_Settings::resultDir()  );
   dir.mkpath( US_Settings::tmpDir()     );
   dir.mkpath( US_Settings::reportDir()  );
   dir.mkpath( US_Settings::archiveDir() );

   setWindowTitle( tr( "Convert Legacy Raw Data" ) );
   setPalette( US_GuiSettings::frameColor() );

   // Very light gray, for read-only line edits
   QPalette gray = US_GuiSettings::editColor();
   gray.setColor( QPalette::Base, QColor( 0xe0, 0xe0, 0xe0 ) );

   QGridLayout* settings = new QGridLayout;

   int row = 0;

   // First row
   QStringList DB = US_Settings::defaultDB();
   if ( DB.isEmpty() ) DB << "Undefined";
   QLabel* lb_DB = us_banner( tr( "Database: " ) + DB.at( 0 ) );
   lb_DB->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
   settings->addWidget( lb_DB, row++, 0, 1, 4 );

   // Investigator
   if ( US_Settings::us_inv_level() > 2 )
   {
      QPushButton* pb_investigator = us_pushbutton( tr( "Select Investigator" ) );
      connect( pb_investigator, SIGNAL( clicked() ), SLOT( sel_investigator() ) );
      settings->addWidget( pb_investigator, row, 0, 1, 2 );
   }
   else
   {
      QLabel* lb_investigator = us_label( tr( "Investigator:" ) );
      settings->addWidget( lb_investigator, row, 0, 1, 2 );
   }
      
   le_investigator = us_lineedit( tr( "Not Selected" ) );
   le_investigator->setReadOnly( true );
   settings->addWidget( le_investigator, row++, 2, 1, 2 );

   // Radio buttons
   disk_controls = new US_Disk_DB_Controls( US_Disk_DB_Controls::Default );
   connect( disk_controls, SIGNAL( changed       ( bool ) ),
                           SLOT  ( source_changed( bool ) ) );
   settings->addLayout( disk_controls, row++, 0, 1, 4 );
   save_diskDB = US_Disk_DB_Controls::Default;

   // Display Run ID
   QLabel* lb_runID = us_label( tr( "Run ID:" ) );
   // Add this later, after tabs:settings->addWidget( lb_runID, row, 0 );
   lb_runID->setVisible( false ); // for now

   le_runID = us_lineedit( "", 1 );
   le_runID ->setMinimumWidth( 225 );
   // Add this later, after tabs: settings->addWidget( le_runID, row++, 1 );
   le_runID ->setPalette ( gray );                // This one is read-only
   le_runID ->setReadOnly( true );
   le_runID ->setVisible ( false );  // for now

   // Load the run
   QLabel* lb_run = us_banner( tr( "Load the Run" ) );
   settings->addWidget( lb_run, row++, 0, 1, 4 );

   // Pushbuttons to load and reload data
   pb_import = us_pushbutton( tr( "Import Legacy Run from HD" ) );
   connect( pb_import, SIGNAL( clicked() ), SLOT( import() ) );
   settings->addWidget( pb_import, row, 0, 1, 2 );

   // External program to enter experiment information
   pb_editRuninfo = us_pushbutton( tr( "Edit Run Information" ) );
   connect( pb_editRuninfo, SIGNAL( clicked() ), SLOT( editRuninfo() ) );
   settings->addWidget( pb_editRuninfo, row++, 2, 1, 2 );
   pb_editRuninfo->setEnabled( false );

   // load US3 data ( that perhaps has been done offline )
   pb_loadUS3 = us_pushbutton( tr( "Load US3 Run" ), true );
   connect( pb_loadUS3, SIGNAL( clicked() ), SLOT( loadUS3() ) );
   settings->addWidget( pb_loadUS3, row, 0, 1, 2 );

   // Run details
   pb_details = us_pushbutton( tr( "Run Details" ), false );
   connect( pb_details, SIGNAL( clicked() ), SLOT( runDetails() ) );
   settings->addWidget( pb_details, row++, 2, 1, 2 );

   // Set the wavelength tolerance for c/c/w determination
   QLabel* lb_tolerance = us_label( tr( "Separation Tolerance:" ) );
   settings->addWidget( lb_tolerance, row, 0, 1, 2 );

   ct_tolerance = us_counter ( 2, 0.0, 100.0, 5.0 ); // #buttons, low, high, start_value
   ct_tolerance->setStep( 1 );
   ct_tolerance->setMinimumWidth( 160 );
   settings->addWidget( ct_tolerance, row++, 2, 1, 2 );
   // connect whenever the user has changed it
   connect( ct_tolerance, SIGNAL( valueChanged         ( double ) ),
                          SLOT  ( toleranceValueChanged( double ) ) );

   // Change Run ID
   QGridLayout* textBoxes = new QGridLayout();

   QLabel* lb_runID2 = us_label( tr( "Run ID:" ) );
   textBoxes->addWidget( lb_runID2, row, 0 );

   le_runID2 = us_lineedit( "", 1 );
   le_runID2 ->setMinimumWidth( 225 );
   textBoxes->addWidget( le_runID2, row++, 1, 1, 3 );

   // Directory
   QLabel* lb_dir = us_label( tr( "Directory:" ) );
   textBoxes->addWidget( lb_dir, row, 0 );

   le_dir = us_lineedit( "", 1 );
   textBoxes->addWidget( le_dir, row++, 1, 1, 3 );
   le_dir ->setPalette ( gray );
   le_dir ->setReadOnly( true );

   // Description
   lb_description = us_label( tr( "Description:" ), -1 );
   lb_description ->setMaximumWidth( 175 );
   textBoxes->addWidget( lb_description, row, 0 );

   le_description = us_lineedit( "", 1 );
   connect( le_description, SIGNAL( editingFinished   () ),
                            SLOT  ( changeDescription () ) );
   textBoxes->addWidget( le_description, row++, 1, 1, 3 );

   settings->addLayout( textBoxes, row++, 0, 1, 4 );

   // Cell / Channel / Wavelength
   QGridLayout* ccw = new QGridLayout();

   lb_triple = us_banner( tr( "Cell / Channel / Wavelength" ), -1 );
   ccw->addWidget( lb_triple, row++, 0, 1, 4 );

   lw_triple = us_listwidget();
   lw_triple->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed );
   lw_triple->setMaximumWidth ( 120 );
   connect( lw_triple, SIGNAL( itemClicked ( QListWidgetItem* ) ),
                       SLOT  ( changeTriple( QListWidgetItem* ) ) );
   ccw->addWidget( lw_triple, row, 0, 6, 1 );

   QLabel* lb_ccwinfo = us_label( tr( "Enter Associated c/c/w Info:" ) );
   ccw->addWidget( lb_ccwinfo, row++, 1, 1, 4 );

   // Set up centerpiece drop-down
   cb_centerpiece = new US_SelectBox( this );
   centerpieceInfo();
   cb_centerpiece -> load();
   connect( cb_centerpiece, SIGNAL( currentIndexChanged( int ) ), 
                            SLOT  ( getCenterpieceIndex( int ) ) );
   ccw->addWidget( cb_centerpiece, row++, 1, 1, 3 );

   // External program to enter solution information
   pb_solution = us_pushbutton( tr( "Manage Solutions" ), false );
   connect( pb_solution, SIGNAL( clicked() ), SLOT( getSolutionInfo() ) );
   ccw->addWidget( pb_solution, row, 1 );

   pb_applyAll = us_pushbutton( tr( "Apply to All" ), false );
   connect( pb_applyAll, SIGNAL( clicked() ), SLOT( tripleApplyAll() ) );
   ccw->addWidget( pb_applyAll, row++, 2 );

   // Defining data subsets
   pb_define = us_pushbutton( tr( "Define Subsets" ), false );
   connect( pb_define, SIGNAL( clicked() ), SLOT( define_subsets() ) );
   ccw->addWidget( pb_define, row, 1 );

   pb_process = us_pushbutton( tr( "Process Subsets" ) , false );
   connect( pb_process, SIGNAL( clicked() ), SLOT( process_subsets() ) );
   ccw->addWidget( pb_process, row++, 2 );

   // Choosing reference channel
   pb_reference = us_pushbutton( tr( "Define Reference Scans" ), false );
   connect( pb_reference, SIGNAL( clicked() ), SLOT( define_reference() ) );
   ccw->addWidget( pb_reference, row, 1 );

   pb_cancelref = us_pushbutton( tr( "Undo Reference Scans" ), false );
   connect( pb_cancelref, SIGNAL( clicked() ), SLOT( cancel_reference() ) );
   ccw->addWidget( pb_cancelref, row++, 2 );

   // Define intensity profile
   pb_intensity = us_pushbutton( tr( "Show Intensity Profile" ) , false );
   connect( pb_intensity, SIGNAL( clicked() ), SLOT( show_intensity() ) );
   ccw->addWidget( pb_intensity, row, 1 );

   // drop scan
   pb_dropScan = us_pushbutton( tr( "Drop Current c/c/w" ), false );
   connect( pb_dropScan, SIGNAL( clicked() ), SLOT( drop_reference() ) );
   ccw->addWidget( pb_dropScan, row++, 2 );

   QLabel* lb_solution = us_label( tr( "Solution:" ) );
   ccw->addWidget( lb_solution, row, 0 );

   le_solutionDesc = us_lineedit( "", 1 );
   le_solutionDesc ->setPalette ( gray );
   le_solutionDesc ->setReadOnly( true );
   ccw->addWidget( le_solutionDesc, row++, 1, 1, 3 );

   settings->addLayout( ccw, row++, 0, 1, 4 );

   // Scan Controls
   QLabel* lb_scan = us_banner( tr( "Scan Controls" ) );
   settings->addWidget( lb_scan, row++, 0, 1, 4 );

   // Scan focus from
   QLabel* lb_from = us_label( tr( "Scan Focus from:" ), -1 );
   lb_from->setAlignment( Qt::AlignVCenter | Qt::AlignRight );
   settings->addWidget( lb_from, row, 0, 1, 2 );

   ct_from = us_counter ( 3, 0.0, 0.0 ); // Update range upon load
   ct_from->setStep( 1 );
   settings->addWidget( ct_from, row++, 2, 1, 2 );

   // Scan focus to
   QLabel* lb_to = us_label( tr( "Scan Focus to:" ), -1 );
   lb_to->setAlignment( Qt::AlignVCenter | Qt::AlignRight );
   settings->addWidget( lb_to, row, 0, 1, 2 );

   ct_to = us_counter ( 3, 0.0, 0.0 ); // Update range upon load
   ct_to->setStep( 1 );
   settings->addWidget( ct_to, row++, 2, 1, 2 );

   // Exclude and Include pushbuttons
   pb_exclude = us_pushbutton( tr( "Exclude Scan(s)" ), false );
   connect( pb_exclude, SIGNAL( clicked() ), SLOT( exclude_scans() ) );
   settings->addWidget( pb_exclude, row, 0, 1, 2 );

   pb_include = us_pushbutton( tr( "Include All" ), false );
   connect( pb_include, SIGNAL( clicked() ), SLOT( include() ) );
   settings->addWidget( pb_include, row++, 2, 1, 2 );
   pb_include ->setEnabled( false );

   // Standard pushbuttons
   QPushButton* pb_reset = us_pushbutton( tr( "Reset" ) );
   connect( pb_reset, SIGNAL( clicked() ), SLOT( resetAll() ) );
   settings->addWidget( pb_reset, row, 0 );

   QPushButton* pb_help = us_pushbutton( tr( "Help" ) );
   connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );
   settings->addWidget( pb_help, row, 1 );

   pb_saveUS3 = us_pushbutton( tr( "Save" ) );
   connect( pb_saveUS3, SIGNAL( clicked() ), SLOT( saveUS3() ) );
   settings->addWidget( pb_saveUS3, row, 2 );

   QPushButton* pb_close = us_pushbutton( tr( "Close" ) );
   connect( pb_close, SIGNAL( clicked() ), SLOT( close() ) );
   settings->addWidget( pb_close, row++, 3 );

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
   picker ->setRubberBand    ( QwtPicker::VLineRubberBand );
   picker->setMousePattern   ( QwtEventPattern::MouseSelect1,
                               Qt::LeftButton, Qt::ControlModifier );

   // Instructions ( missing to do items )
   QVBoxLayout* todo = new QVBoxLayout();

   QLabel* lb_todoinfo = us_banner( tr( "Instructions ( to do list )" ), -1 );
   todo->addWidget( lb_todoinfo );

   lw_todoinfo = us_listwidget();
   lw_todoinfo->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed );
   lw_todoinfo->setMaximumHeight ( 90 );
   lw_todoinfo->setSelectionMode( QAbstractItemView::NoSelection );
   todo->addWidget( lw_todoinfo );

   // Now let's assemble the page
   
   QVBoxLayout* left     = new QVBoxLayout;

   left->addLayout( settings );
   
   QVBoxLayout* right    = new QVBoxLayout;
   
   right->addLayout( plot );
   right->addLayout( todo );

   QHBoxLayout* main = new QHBoxLayout( this );
   main->setSpacing         ( 2 );
   main->setContentsMargins ( 2, 2, 2, 2 );

   main->addLayout( left );
   main->addLayout( right );

   main->setStretch( 0, 2 );
   main->setStretch( 1, 4 );

   reset();
}

void US_ConvertGui::reset( void )
{
   lw_triple       ->clear();

   le_dir          ->setText( "" );

   le_description  ->setText( "" );
   le_runID        ->setText( "" );
   le_runID2       ->setText( "" );
   le_solutionDesc ->setText( "" );

   pb_import     ->setEnabled( true  );
   pb_loadUS3    ->setEnabled( true  );
   pb_exclude    ->setEnabled( false );
   pb_include    ->setEnabled( false );
   pb_details    ->setEnabled( false );
   pb_intensity  ->setEnabled( false );
   pb_cancelref  ->setEnabled( false );
   pb_dropScan   ->setEnabled( false );
   pb_solution   ->setEnabled( false );
   pb_editRuninfo ->setEnabled( false );
   pb_applyAll   ->setEnabled( false );
   pb_saveUS3    ->setEnabled( false );

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
   show_plot_progress = true;
   ExpData.rpms.clear();

   // Erase the todo list
   lw_todoinfo->clear();
   lw_todoinfo->addItem( "Load or import some AUC data" );
   
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
   referenceDefined = false;

   // Display investigator
   ExpData.invID = US_Settings::us_inv_ID();

   QString number = ( ExpData.invID > 0 )
      ?  QString::number( ExpData.invID ) + ": "
      : "";

   le_investigator->setText( number + US_Settings::us_inv_name() );
}

void US_ConvertGui::resetAll( void )
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
   subsets.clear();
   reference_start = 0;
   reference_end   = 0;

   ct_tolerance    ->setMinValue(   0.0 );
   ct_tolerance    ->setMaxValue( 100.0 );
   ct_tolerance    ->setValue   (   5.0 );
   ct_tolerance    ->setStep( 1 );
   scanTolerance   = 5.0;
}

// Function to select the current investigator
void US_ConvertGui::sel_investigator( void )
{
   US_Investigator* inv_dialog = new US_Investigator( true, ExpData.invID );

   connect( inv_dialog,
      SIGNAL( investigator_accepted( int ) ),
      SLOT  ( assign_investigator  ( int ) ) );

   inv_dialog->exec();
}

// Function to assign the selected investigator as current
void US_ConvertGui::assign_investigator( int invID )
{
   ExpData.invID = invID;

   QString number = ( invID > 0 )
                    ? QString::number( invID ) + ": "
                    : "";

   le_investigator->setText( number + US_Settings::us_inv_name() );

}

// Function to change the data source (disk/db)
void US_ConvertGui::source_changed( bool )
{
   QStringList DB = US_Settings::defaultDB();

   if ( DB.size() < 5 )
   {
      QMessageBox::warning( this,
         tr( "Attention" ),
         tr( "There is no default database set." ) );
   }

   // Did we switch from disk to db?
   if ( disk_controls->db() && ! save_diskDB )
   {
      // Make sure these are explicitly chosen
      ExpData.operatorID = 0;
      ExpData.rotorID    = 0;
      ExpData.calibrationID = 0;
   }

   save_diskDB = disk_controls->db();

   // Reinvestigate whether to enable the buttons
   enableControls();
}

void US_ConvertGui::update_disk_db( bool db )
{
   save_diskDB = disk_controls->db();

   ( db ) ? disk_controls->set_db() : disk_controls->set_disk();
}

// User changed the dataset separation tolerance
void US_ConvertGui::toleranceValueChanged( double )
{
   toleranceChanged = true;
   scanTolerance    = ct_tolerance->value();
   reimport();
}

// User pressed the import data button
void US_ConvertGui::import( QString dir )
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

   checkTemperature();          // Check to see if temperature varied too much

   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );

   // Initialize exclude list
   init_excludes();
   
   plot_current();

   QApplication::restoreOverrideCursor();

   connect( ct_from, SIGNAL( valueChanged ( double ) ),
                     SLOT  ( focus_from   ( double ) ) );

   connect( ct_to  , SIGNAL( valueChanged ( double ) ),
                     SLOT  ( focus_to     ( double ) ) );

   saveStatus = NOT_SAVED;

   // Ok to enable some buttons now
   enableControls();

   if ( runType == "RI" )
   {
      referenceDefined = false;
      pb_reference->setEnabled( true );
   }
}

// User pressed the reimport data button
// Legacy data is already supposed to be present
void US_ConvertGui::reimport( void )
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
   QApplication::restoreOverrideCursor();

   connect( ct_from, SIGNAL( valueChanged ( double ) ),
                     SLOT  ( focus_from   ( double ) ) );

   connect( ct_to  , SIGNAL( valueChanged ( double ) ),
                     SLOT  ( focus_to     ( double ) ) );

   // Ok to enable some buttons now
   enableControls();

   if ( runType == "RI" )
   {
      referenceDefined = false;
      pb_reference->setEnabled( true );
   }
}

// Enable the common dialog controls when there is data
void US_ConvertGui::enableControls( void )
{
   if ( allData.size() == 0 )
      reset();

   else
   {
      // Ok to enable some buttons now
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
         pb_reference->setEnabled( ! referenceDefined );
   
      if ( subsets.size() < 1 )
      {
         // Allow user to define subsets, if he hasn't already
         pb_define   ->setEnabled( true );
      } 

      // Disable load buttons if there is data
      pb_import        ->setEnabled( false );
      pb_loadUS3       ->setEnabled( false );
      
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
         
      enableSaveBtn();
   }
}

// Enable or disable the runID control
void US_ConvertGui::enableRunIDControl( bool setEnable )
{
   QPalette gray = US_GuiSettings::editColor();
   gray.setColor( QPalette::Base, QColor( 0xe0, 0xe0, 0xe0 ) );

   if ( setEnable )
   {
      le_runID2->setPalette ( US_GuiSettings::normalColor() );
      le_runID2->setReadOnly( false );
      connect( le_runID2, SIGNAL( editingFinished() ),
                          SLOT  ( runIDChanged   () ) );
   }

   else
   {
      le_runID2->disconnect();
      le_runID2->setPalette ( gray );
      le_runID2->setReadOnly( true );
   }
         
}

// Reset the boundaries on the scan controls
void US_ConvertGui::enableScanControls( void )
{
   ct_from->disconnect();
   ct_from->setMinValue( 0.0 );
   ct_from->setMaxValue(  allData[ currentTriple ].scanData.size() 
                        - allExcludes[ currentTriple ].size() );
   ct_from->setValue   ( 0 );

   ct_to  ->disconnect();
   ct_to  ->setMinValue( 0.0 );
   ct_to  ->setMaxValue(  allData[ currentTriple ].scanData.size() 
                        - allExcludes[ currentTriple ].size() );
   ct_to  ->setValue   ( 0 );

   connect( ct_from, SIGNAL( valueChanged ( double ) ),
                     SLOT  ( focus_from   ( double ) ) );

   connect( ct_to  , SIGNAL( valueChanged ( double ) ),
                     SLOT  ( focus_to     ( double ) ) );

}

// Enable the "save" button, if appropriate
// Let's use the same logic to populate the todo list too
void US_ConvertGui::enableSaveBtn( void )
{
   lw_todoinfo->clear();
   int count = 0;
   bool completed = true;
   cb_centerpiece->setLogicalIndex( triples[ currentTriple ].centerpiece );

   if ( allData.size() == 0 )
   {
      count++;
      lw_todoinfo->addItem( QString::number( count ) + ": Load or import some AUC data" );
      completed = false;
   }

   // Do we have any triples?
   if ( triples.size() == 0 )
   {
      count++;
      lw_todoinfo->addItem( QString::number( count ) + ": Load or import some AUC data" );
      completed = false;
   }
   
   // Is the run info defined?
   QRegExp rx( "^[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}$" );

   // Not checking operator on disk -- defined as "Local"
   if ( ( ExpData.rotorID == 0 )              ||
        ( ExpData.calibrationID == 0 )        ||
        ( ExpData.labID == 0 )                ||
        ( ExpData.instrumentID == 0 )         ||
        ( ExpData.label.isEmpty() )           ||
        ( ! rx.exactMatch( ExpData.project.projectGUID ) ) )
   {
      count++;
      lw_todoinfo->addItem( QString::number( count ) + ": Edit run information" );
      completed = false;
   }
   
   // Have we filled out all the c/c/w info?
   // Check GUIDs, because solutionID's may not be present yet.
   foreach ( US_Convert::TripleInfo triple, triples )
   {
      if ( triple.excluded ) continue;
   
      if ( ! rx.exactMatch( triple.solution.solutionGUID ) )
      {
         count++;
         lw_todoinfo->addItem( QString::number( count ) + 
                               ": Select solution for triple " +
                               triple.tripleDesc );
         completed = false;
      }
   }

   foreach ( US_Convert::TripleInfo triple, triples )
   {
      if ( triple.excluded ) continue;
   
      if ( triple.centerpiece == 0 )
      {
         count++;
         lw_todoinfo->addItem( QString::number( count ) + 
                               ": Select centerpiece for triple " +
                               triple.tripleDesc );
         completed = false;
      }
   }
  
   if ( disk_controls->db() )
   {
      // Verify connectivity
      US_Passwd pw;
      QString masterPW = pw.getPasswd();
      US_DB2 db( masterPW );
   
      if ( db.lastErrno() != US_DB2::OK )
      {
         count++;
         lw_todoinfo->addItem( QString::number( count ) +
                               ": Verify database connectivity" );
         completed = false;
      }
   
      // Information is there, but we need to see if the runID exists in the 
      // DB. If we didn't load it from there, then we shouldn't be able to sync
      int recStatus = ExpData.checkRunID( &db );
   
      // if a record is found but saveStatus == BOTH, then we are editing that record
      if ( ( recStatus == US_DB2::OK ) && ( saveStatus != BOTH ) ) // ||
           // ( ! ExpData.syncOK ) ) 
      {
         count++;
         lw_todoinfo->addItem( QString::number( count ) +
                               ": Select a different runID" );
         completed = false;
      }
   
      // Not checking operator on disk -- defined as "Local"
      if ( ExpData.operatorID == 0 )
      {
         count++;
         lw_todoinfo->addItem( QString::number( count ) + ": Select operator in run information" );
         completed = false;
      }

   }

   // This can go on the todo list, but should not prevent user from saving
   if ( ( runType == "RI" ) && ( ! referenceDefined ) )
   {
      count++;
      lw_todoinfo->addItem( QString::number( count ) + 
                            ": Define reference scans" );
   }

   // If we made it here, user can save
   pb_saveUS3 ->setEnabled( completed );
}

// Process when the user changes the runID
void US_ConvertGui::runIDChanged( void )
{
   // See if we need to update the runID
   QRegExp rx( "^[A-Za-z0-9_-]{1,80}$" );
   QString new_runID = le_runID2->text();
      
   if ( rx.indexIn( new_runID ) >= 0 )
   {
      runID = new_runID;
      plot_titles();
   }

   // If the runID has changed, a number of other things need to change too,
   // for instance GUID's.
   ExpData.clear();
   foreach( US_Convert::TripleInfo triple, triples )
      triple.clear();
   le_runID2->setText( runID );
   le_runID ->setText( runID );
   ExpData.runID = runID;

   saveStatus = NOT_SAVED;

   // Set the directory too
   QDir resultDir( US_Settings::resultDir() );
   currentDir = resultDir.absolutePath() + "/" + runID + "/";
   le_dir ->setText( currentDir );
}

// Function to generate a new guid for experiment, and associate with DB
void US_ConvertGui::editRuninfo( void )
{
   if ( saveStatus == NOT_SAVED )
   {
      // First time for this data, so clear ExpData out
      ExpData.clear();
   
      // Create a new GUID for the experiment as a whole
      ExpData.expGUID = US_Util::new_guid();

   }

   getExpInfo( );
}

// Function to load US3 data
void US_ConvertGui::loadUS3( QString dir )
{
   if ( disk_controls->db() )
      loadUS3DB();

   else if ( dir.isEmpty() )
      loadUS3Disk();

   else
      loadUS3Disk( dir );

   checkTemperature();          // Check to see if temperature varied too much
}

void US_ConvertGui::loadUS3Disk( void )
{
   // Ask for data directory
   QString dir = QFileDialog::getExistingDirectory( this, 
         tr("US3 Raw Data Directory"),
         US_Settings::resultDir(),
         QFileDialog::DontResolveSymlinks );
   
   // Restore area beneath dialog
   qApp->processEvents();

   if ( dir.isEmpty() ) return; 
   
   dir.replace( "\\", "/" );  // WIN32 issue
   if ( dir.right( 1 ) != "/" ) dir += "/"; // Ensure trailing /

   loadUS3Disk( dir );
}

void US_ConvertGui::loadUS3Disk( QString dir )
{
   resetAll();

   // Check the runID
   QStringList components =  dir.split( "/", QString::SkipEmptyParts );  
   QString new_runID = components.last();
      
   QRegExp rx( "^[A-Za-z0-9_-]{1,80}$" );
   if ( rx.indexIn( new_runID ) < 0 )
   {
      QMessageBox::warning( this,
            tr( "Bad runID Name" ),
            tr( "The runID name may consist only of alphanumeric\n"  
                "characters, the underscore, and the hyphen." ) );
      return;
   }

   // Set the runID and directory
   runID = new_runID;
   le_runID ->setText( runID );
   le_runID2->setText( runID );
   le_dir   ->setText( dir );
   currentDir  = QString( dir );

   // Reload the AUC data
   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
   int status = US_Convert::readUS3Disk( dir, allData, triples, runType );
   QApplication::restoreOverrideCursor();

   if ( status == US_Convert::NODATA )
   {
      QMessageBox::warning( this,
            tr( "No Files Found" ),
            tr( "There were no files of the form *.auc\n"  
                "found in the specified directory." ) );
      return;
   }

   if ( status == US_Convert::NOAUC )
   {
      QMessageBox::warning( this,
         tr( "UltraScan Error" ),
         tr( "Could not read data file.\n" ) );
      return;
   }

   // Now try to read the xml file
   ExpData.clear();
   status = ExpData.readFromDisk( triples, runType, runID, dir );

   if ( status == US_Convert::CANTOPEN )
   {
      QString readFile = runID      + "." 
                       + runType    + ".xml";
      QMessageBox::information( this,
            tr( "Error" ),
            tr( "US3 run data ok, but unable to assocate run with DB.\n " ) +
            tr( "Cannot open read file: " ) + dir + readFile );
   }

   else if ( status == US_Convert::BADXML )
   {
      QString readFile = runID      + "." 
                       + runType    + ".xml";
      QMessageBox::information( this,
            tr( "Error" ),
            tr( "US3 run data ok, but there is an error in association with DB.\n" ) +
            tr( "Improper XML in read file: " ) + dir + readFile );
   }

   else if ( status != US_Convert::OK )
   {
      QMessageBox::information( this,
            tr( "Error" ),
            tr( "Unknown error: " ) + status );
   }

   // Now that we have the experiment, let's read the rest of the
   //  solution and project information
   status = ExpData.project.readFromDisk( ExpData.project.projectGUID );

   // Error reporting 
   if ( status == US_DB2::NO_PROJECT ) 
   { 
      QMessageBox::information( this, 
            tr( "Attention" ), 
            tr( "The project was not found.\n" 
                "Please select an existing project and try again.\n" ) ); 
   } 
   
   else if ( status != US_DB2::OK ) 
   { 
      QMessageBox::information( this, 
            tr( "Disk Read Problem" ), 
            tr( "Could not read data from the disk.\n" 
                "Disk status: " ) + QString::number( status ) ); 
   }

   // and clear it out
   if ( status != US_DB2::OK )
      ExpData.project.clear();

   // Now the solutions
   for ( int i = 0; i < triples.size(); i++ )
   {
      status = triples[ i ].solution.readFromDisk( triples[ i ].solution.solutionGUID );

      // Error reporting
      if ( status == US_DB2::NO_SOLUTION )
      {
         QMessageBox::information( this,
               tr( "Attention" ),
               tr( "A solution this run refers to was not found, or could not be read.\n"
                   "Please select an existing solution and try again.\n" ) );
      }
      
      else if ( status == US_DB2::NO_BUFFER )
      {
         QMessageBox::information( this,
               tr( "Attention" ),
               tr( "The buffer this solution refers to was not found.\n"
                   "Please restore and try again.\n" ) );
      }
      
      else if ( status == US_DB2::NO_ANALYTE )
      {
         QMessageBox::information( this,
               tr( "Attention" ),
               tr( "One of the analytes this solution refers to was not found.\n"
                   "Please restore and try again.\n" ) );
      }
      
      else if ( status != US_DB2::OK )
      {
         QMessageBox::information( this, 
               tr( "Disk Read Problem" ), 
               tr( "Could not read data from the disk.\n" 
                   "Disk status: " ) + QString::number( status ) ); 
      }

      // Just clear it out
      if ( status != US_DB2::OK )
         triples[ i ].solution.clear();
   }

   // Now read the RI Profile, if it exists
   if ( runType == "RI" )
   {
      referenceDefined = false;
      pb_reference->setEnabled( true );

      status = ExpData.readRIDisk( runID, dir );

      if ( status == US_Convert::CANTOPEN )
      {
         QString readFile = runID      + "." 
                          + "RIProfile.xml";
         QMessageBox::information( this,
               tr( "Error" ),
               tr( "US3 run data ok, but unable to read intensity profile.\n " ) +
               tr( "Cannot open read file: " ) + dir + readFile );
      }
      
      else if ( status == US_Convert::BADXML )
      {
         QString readFile = runID      + "." 
                          + "RIProfile.xml";
         QMessageBox::information( this,
               tr( "Error" ),
               tr( "US3 run data ok, but unable to read intensity profile.\n " ) +
               tr( "Improper XML in read file: " ) + dir + readFile );
      }
      
      else if ( status != US_Convert::OK )
      {
         QMessageBox::information( this,
               tr( "Error" ),
               tr( "Unknown error: " ) + status );
      }

      else
      {
         // Enable intensity plot
         pb_intensity->setEnabled( true );
         
         referenceDefined = true;
         pb_reference->setEnabled( false );
         pb_cancelref ->setEnabled( true );
      }

   }

   if ( allData.size() == 0 ) return;

   // Update triple information on screen
   setTripleInfo();
   init_excludes();

   le_solutionDesc  -> setText( triples[ currentTriple ].solution.solutionDesc  );

   // Restore description
   le_description->setText( allData[ 0 ].description );
   saveDescription = QString( allData[ 0 ].description );

   // Reset maximum scan control values
   enableScanControls();

   // The centerpiece combo box
   cb_centerpiece->setLogicalIndex( triples[ currentTriple ].centerpiece );

   // Redo plot
   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
   plot_current();
   QApplication::restoreOverrideCursor();

   connect( ct_from, SIGNAL( valueChanged ( double ) ),
                     SLOT  ( focus_from   ( double ) ) );

   connect( ct_to  , SIGNAL( valueChanged ( double ) ),
                     SLOT  ( focus_to     ( double ) ) );

   // Ok to enable some buttons now
   enableControls();
   if ( ! disk_controls->db() ) 
      pb_saveUS3  ->setEnabled( true );

   pb_details     ->setEnabled( true );
   pb_solution    ->setEnabled( true );
   cb_centerpiece ->setEnabled( true );
   pb_editRuninfo ->setEnabled( true );

   // some things one can't do from here
   ct_tolerance   ->setEnabled( false );

   enableRunIDControl( false );

   if ( runType == "RA" && subsets.size() < 1 )
   {
      // Allow user to define subsets, if he hasn't already
      pb_define   ->setEnabled( true );
   } 

   saveStatus = ( ExpData.expID == 0 ) ? HD_ONLY : BOTH;
   //pb_editRuninfo  ->setEnabled ( saveStatus == HD_ONLY );

   // Read it in ok, so ok to sync with DB
   ExpData.syncOK = true;

   enableSaveBtn();
}

// Function to load an experiment from the DB
void US_ConvertGui:: loadUS3DB( void )
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

   // Restore area beneath dialog
   qApp->processEvents();

   // Now that we have the runID, let's copy the DB info to HD
   QDir        readDir( US_Settings::resultDir() );
   QString     dirname = readDir.absolutePath() + "/" + runID + "/";

   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
   QString status = US_ConvertIO::readDBExperiment( runID, dirname, &db );
   QApplication::restoreOverrideCursor();

   if ( status  != QString( "" ) )
   {
      QMessageBox::information( this,
             tr( "Error" ),
             status + "\n" );
      return;
   }

   // and load it
   loadUS3Disk( dirname );

   saveStatus = BOTH;         // override from loadUS3Disk()
   ExpData.syncOK = true;     // since we just read it from there

   enableControls();
}

void US_ConvertGui::getExpInfo( void )
{
   ExpData.runID = le_runID -> text();

   if ( disk_controls->db() )
   {
      // Then we're working in DB, so verify connectivity
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
     
      // Check if the run ID already exists in the DB
      int recStatus = ExpData.checkRunID( &db );
     
      // if saveStatus == BOTH, then we are editing the record from the database
      if ( ( recStatus == US_DB2::OK ) && ( saveStatus != BOTH ) ) 
      {
         QMessageBox::information( this,
                tr( "Error" ),
                tr( "The current runID already exists in the database. To edit that "
                    "information, load it from the database to start with.\n" ) );
         return;
      }
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

   int dbdisk = ( disk_controls->db() ) ? US_Disk_DB_Controls::DB
                                        : US_Disk_DB_Controls::Disk;

   US_ExperimentGui* expInfo = new US_ExperimentGui( true,    // signal_wanted
                                                     ExpData,
                                                     dbdisk );

   connect( expInfo, SIGNAL( updateExpInfoSelection( US_Experiment& ) ),
            this   , SLOT  ( updateExpInfo         ( US_Experiment& ) ) );

   connect( expInfo, SIGNAL( cancelExpInfoSelection() ),
            this   , SLOT  ( cancelExpInfo         () ) );

   connect( expInfo, SIGNAL( use_db        ( bool ) ),
                     SLOT  ( update_disk_db( bool ) ) );

   expInfo->exec();
}

// Updating after user has selected info from experiment dialog
void US_ConvertGui::updateExpInfo( US_Experiment& d )
{
   // Update local copy
   ExpData = d;

   if ( this->saveStatus == NOT_SAVED )
      this->saveStatus = EDITING;        // don't delete the data!

   enableControls();
}

void US_ConvertGui::cancelExpInfo( void )
{
   // Don't clear out the data, just don't save anything new

   enableControls();
}

void US_ConvertGui::getSolutionInfo( void )
{
   ExpData.runID = le_runID -> text();

   int dbdisk = ( disk_controls->db() ) ? US_Disk_DB_Controls::DB
                                        : US_Disk_DB_Controls::Disk;

   US_Solution solution = triples[ currentTriple ].solution;

   US_SolutionGui* solutionInfo = new US_SolutionGui( ExpData.expID,
                                                      1,      // channelID (figure out later ?? )
                                                      true,   // signal wanted
                                                      dbdisk, // data source
                                                      solution );

   connect( solutionInfo, SIGNAL( updateSolutionGuiSelection( US_Solution ) ),
            this,         SLOT  ( updateSolutionInfo        ( US_Solution ) ) );

   connect( solutionInfo, SIGNAL( cancelSolutionGuiSelection() ),
            this,         SLOT  ( cancelSolutionInfo        () ) );

   connect( solutionInfo, SIGNAL( use_db        ( bool ) ),
                          SLOT  ( update_disk_db( bool ) ) );

   solutionInfo->exec();
}

// Updating after user has selected info from experiment dialog
void US_ConvertGui::updateSolutionInfo( US_Solution s )
{
   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );

   // Update local copy
   triples[ currentTriple ].solution = s;

   le_solutionDesc  ->setText( triples[ currentTriple ].solution.solutionDesc  );

   plot_current();

   QApplication::restoreOverrideCursor();

   enableControls();
}

void US_ConvertGui::cancelSolutionInfo( void )
{
   enableControls();
}

// Function to copy the current triple's data to all triples
void US_ConvertGui::tripleApplyAll( void )
{
   US_Convert::TripleInfo triple = triples[ currentTriple ];

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

void US_ConvertGui::runDetails( void )
{
   // Create data structures for US_RunDetails2
   QStringList tripleDescriptions;
   QVector< US_DataIO2::RawData >  currentData;
   for (int i = 0; i < triples.size(); i++ )
   {
      if ( triples[ i ].excluded ) continue;

      // Only pass non-excluded triples
      currentData << allData[ i ];
      tripleDescriptions << triples[ i ].tripleDesc;
   }

   US_RunDetails2* dialog
      = new US_RunDetails2( currentData, runID, currentDir, tripleDescriptions );
   dialog->exec();
   qApp->processEvents();
   delete dialog;
}

void US_ConvertGui::changeDescription( void )
{
   allData[ currentTriple ].description = le_description->text();
   allData[ currentTriple ].description = allData[ currentTriple ].description.trimmed();
}

void US_ConvertGui::changeTriple( QListWidgetItem* )
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

void US_ConvertGui::setTripleInfo( void )
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

void US_ConvertGui::checkTemperature( void )
{
   // Temperature check 
   double dt = 0.0; 
  
   foreach( US_DataIO2::RawData triple, allData ) 
   { 
       double temp_spread = triple.temperature_spread(); 
       dt = ( temp_spread > dt ) ? temp_spread : dt; 
   } 
  
   if ( dt > US_Settings::tempTolerance() ) 
   { 
      QMessageBox::warning( this, 
            tr( "Temperature Problem" ), 
            tr( "The temperature in this run varied over the course\n" 
                "of the run to a larger extent than allowed by the\n" 
                "current threshold (" ) 
                + QString::number( US_Settings::tempTolerance(), 'f', 1 ) 
                + " " + DEGC + tr( "). The accuracy of experimental\n" 
                "results may be affected significantly." ) ); 
   } 
}

void US_ConvertGui::getCenterpieceIndex( int )
{
   triples[ currentTriple ].centerpiece = cb_centerpiece->getLogicalID();

   enableSaveBtn();
}

void US_ConvertGui::focus_from( double scan )
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

void US_ConvertGui::focus_to( double scan )
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

void US_ConvertGui::focus( int from, int to )
{
   if ( from == 0 && to == 0 )
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
void US_ConvertGui::init_excludes( void )
{
   allExcludes.clear();
   US_Convert::Excludes x;
   for ( int i = 0; i < allData.size(); i++ )
      allExcludes << x;
}

// Function to exclude user-selected scans
void US_ConvertGui::exclude_scans( void )
{
   US_Convert::Excludes excludes = allExcludes[ currentTriple ];

   // Scans actually start at index 0
   int scanStart = (int)ct_from->value() - 1;
   int scanEnd   = (int)ct_to  ->value() - 1;

   // Sometimes the user leaves this at 0
   scanStart = ( scanStart < 0 ) ? 0 : scanStart;
   int scanCount = scanEnd - scanStart + 1;

   // Find the first scan to exclude, but account for the already excluded
   int scanNdx = 0;
   while ( scanNdx < scanStart )
   {
      if ( excludes.contains( scanNdx ) )
         scanStart++;

      scanNdx++;
   }

   // Now we have the starting point, so exclude some scans
   int excluded = 0;
   while ( excluded < scanCount )
   {
      if ( ! excludes.contains( scanNdx ) )
      {
         excluded++;
         allExcludes[ currentTriple ] << scanNdx;
      }

      scanNdx++;
   }

   enableScanControls();

   replot();
}

void US_ConvertGui::include( void )
{
   allExcludes[ currentTriple ].excludes.clear();
   enableScanControls();

   pb_include->setEnabled( false );
   pb_exclude->setEnabled( false );

   replot();
}

// User pressed the define subsets button while processing equil-abs data
void US_ConvertGui::define_subsets( void )
{
   subsets.clear();
   countSubsets = 1;      // We start with one big subset

   pb_define  ->setEnabled( false );
   pb_process ->setEnabled( true );

   connect( picker, SIGNAL( cMouseUp( const QwtDoublePoint& ) ),
                    SLOT  ( cClick  ( const QwtDoublePoint& ) ) );

   step = SPLIT;
}

void US_ConvertGui::process_subsets( void )
{
   if ( subsets.size() < 1 )
   {
      // Not enough clicks to work with
      subsets.clear();
      pb_process ->setEnabled( true );
      return;
   }

   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );

   // Add the top and bottom boundaries
   subsets << 5.7
           << 7.3;

   // Let's make sure the points are in sorted order
   for ( int i = 0; i < subsets.size() - 1; i++ )
      for ( int j = i + 1; j < subsets.size(); j++ )
         if ( subsets[ i ] > subsets[ j ] )
         {
            double temp  = subsets[ i ];
            subsets[ i ] = subsets[ j ];
            subsets[ j ] = temp;
         }

   pb_process ->setEnabled( false );
   picker   ->disconnect();

   // Now let's split the triple
   US_Convert::splitRAData( allData, triples, currentTriple, subsets );

   // We don't need this any more
   subsets.clear();

   // Reinitialize some things
   setTripleInfo();
   init_excludes();
   enableControls();
   
   plot_current();
   QApplication::restoreOverrideCursor();
}

// User pressed the Define reference button while analyzing intensity data
void US_ConvertGui::define_reference( void )
{
   connect( picker, SIGNAL( cMouseUp( const QwtDoublePoint& ) ),
                    SLOT  ( cClick  ( const QwtDoublePoint& ) ) );

   reference_start = 0.0;
   reference_end   = 0.0;
   pb_reference ->setEnabled( false );

   step = REFERENCE;
}

// Select starting point of reference scan in intensity data
void US_ConvertGui::start_reference( const QwtDoublePoint& p )
{
   reference_start   = p.x();

   draw_vline( reference_start );
   data_plot->replot();
}

// Select end point of reference scan in intensity data
void US_ConvertGui::process_reference( const QwtDoublePoint& p )
{
   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );

   // Just in case we get a second click message right away
   if ( fabs( p.x() - reference_start ) < 0.005 ) return;

   reference_end = p.x();
   draw_vline( reference_end );
   data_plot->replot();
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
   QApplication::restoreOverrideCursor();

   pb_reference  ->setEnabled( false );
   referenceDefined = true;
   enableSaveBtn();
}

// Process a control-click on the plot window
void US_ConvertGui::cClick( const QwtDoublePoint& p )
{
   switch ( step )
   {
      case SPLIT :
         if ( countSubsets < 4 )
         {
             // process equil-abs data
             draw_vline( p.x() );
             subsets << p.x();
             countSubsets++;
         }

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

void US_ConvertGui::PseudoCalcAvg( void )
{
   if ( referenceDefined ) return;             // Average calculation has already been done

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

      if ( count > 0 )
         ExpData.RIProfile << sum / count;

      else
         ExpData.RIProfile << 1.0;    // See the log10 function, later

   }
   
   // Now average around excluded values
   int lastGood  = 0;
   int countBad  = 0;
   for ( int i = 0; i < ExpData.RIProfile.size(); i++ )
   {
      // In case there are adjacent excluded scans...
      if ( allExcludes[ currentTriple ].contains( i ) )
         countBad++;

      // Calculate average of before and after for intervening values
      else if ( countBad > 0 )
      {
         double newAvg = ( ExpData.RIProfile[ lastGood ] + ExpData.RIProfile[ i ] ) / 2.0;
         for ( int k = lastGood + 1; k < i; k++ )
            ExpData.RIProfile[ k ] = newAvg;

         countBad = 0;
      }

      // Normal situation -- value is not excluded
      else
         lastGood = i;

   }

   // Now calculate the pseudo-absorbance
   for ( int i = 0; i < allData.size(); i++ )
   {
      US_DataIO2::RawData* currentData = &allData[ i ];

      for ( int j = 0; j < currentData->scanData.size(); j++ )
      {
         US_DataIO2::Scan* s = &currentData->scanData[ j ];

         for ( int k = 0; k < s->readings.size(); k++ )
         {
            US_DataIO2::Reading* r = &s->readings[ k ];

            // Protect against possible inf's and nan's, if a reading 
            // evaluates to 0 or wherever log function is undefined or -inf
            if ( r->value < 1.0 ) r->value = 1.0;
            r->value = log10(ExpData.RIProfile[ j ] / r->value );
         }
      }

      // Let's mark pseudo-absorbance as different from RI data,
      //  since it needs some different processing in some places
      isPseudo = true;
   }

   // Enable intensity plot
   referenceDefined = true;
   pb_intensity->setEnabled( true );
   pb_reference  ->setEnabled( false );
   pb_cancelref ->setEnabled( true );
}

// Bring up a graph window showing the intensity profile
void US_ConvertGui::show_intensity( void )
{
   US_Intensity* dialog
      = new US_Intensity( runID, triples[0].tripleDesc, 
                        ( const QVector< double > ) ExpData.RIProfile );
   dialog->exec();
   qApp->processEvents();
}

void US_ConvertGui::cancel_reference( void )
{
   // Do the inverse operation and retrieve raw intensity data
   for ( int i = 0; i < allData.size(); i++ )
   {
      US_DataIO2::RawData* currentData = &allData[ i ];

      for ( int j = 0; j < currentData->scanData.size(); j++ )
      {
         US_DataIO2::Scan* s = &currentData->scanData[ j ];

         for ( int k = 0; k < s->readings.size(); k++ )
         {
            US_DataIO2::Reading* r = &s->readings[ k ];

            r->value = ExpData.RIProfile[ j ] / pow( 10, r->value );
         }
      }
   }

   referenceDefined = false;
   ExpData.RIProfile.clear();
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

   enableSaveBtn();
}

void US_ConvertGui::drop_reference( void )
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

// Function to save US3 data
void US_ConvertGui::saveUS3( void )
{
   if ( disk_controls->db() )
      saveUS3DB();

   else
      saveUS3Disk();

}

int US_ConvertGui::saveUS3Disk( void )
{
   if ( allData[ 0 ].scanData.empty() ) return US_Convert::NODATA; 

   QDir        writeDir( US_Settings::resultDir() );
   QString     dirname = writeDir.absolutePath() + "/" + runID + "/";

   if ( saveStatus == NOT_SAVED  && 
        writeDir.exists( runID ) )
   {
        QMessageBox::information( this,
              tr( "Error" ),
              tr( "The write directory,  " ) + dirname + 
              tr( " already exists. Please change run ID to a unique value." ) );
        return US_Convert::DUP_RUNID;
   }

   if ( ! writeDir.exists( runID ) )
   {
      if ( ! writeDir.mkpath( dirname ) )
      {
         QMessageBox::information( this,
               tr( "Error" ),
               tr( "Cannot write to " ) + writeDir.absolutePath() );
         return US_Convert::CANTOPEN;
      }
   }

   int status;

   // Check to see if we have all the data to write
   if ( ! ExpData.syncOK )
   {
      status = QMessageBox::information( this,
               tr( "Warning" ),
               tr( "The run has not yet been associated with the database. "    ) +
               tr( "Click 'OK' to proceed anyway, or click 'Cancel' "           ) +
               tr( "and then click on the 'Edit Run Information'"              ) +
               tr( "button to enter this information first.\n\n "               ),
               tr( "&OK" ), tr( "&Cancel" ),
               0, 0, 1 );
      if ( status != 0 ) return US_Convert::NOT_WRITTEN;
   }

   // Write the data
   bool saveGUIDs = saveStatus != NOT_SAVED ;
   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
   status = US_Convert::saveToDisk( allData, triples, 
                                    allExcludes, runType, runID, dirname, saveGUIDs );
   QApplication::restoreOverrideCursor();

   // Now try to write the xml file
   status = ExpData.saveToDisk( triples, runType, runID, dirname );

   // How many files should have been written?
   int fileCount = 0;
   for ( int i = 0; i < triples.size(); i++ )
      if ( ! triples[ i ].excluded ) fileCount++;

   // Now try to communicate status
   if ( status == US_Convert::CANTOPEN )
   {
      QString writeFile = runID      + "." 
                        + runType    + ".xml";
      QMessageBox::information( this,
            tr( "Error" ),
            tr( "Cannot open write file: " ) + dirname + writeFile );
   }

   else if ( status == US_Convert::NOXML )
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

   else if ( status == US_Convert::PARTIAL_XML )
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

   else if ( status != US_Convert::OK )
   {
      QMessageBox::information( this,
            tr( "Error" ),
            tr( "Error: " ) + status );
      return( status );
   }

   // Save or delete RI intensity profile
   if ( runType == "RI" )
   {
      if ( referenceDefined )
      {
         status = ExpData.saveRIDisk( runID, dirname ); 
         
         if ( status == US_Convert::CANTOPEN )
         {
            QString writeFile = runID      + "." 
                              + "RIProfile.xml";
            QMessageBox::information( this,
                  tr( "Error" ),
                  tr( "Cannot open write file: " ) + dirname + writeFile );
         }
         
         else if ( status != US_Convert::OK )
         {
            QMessageBox::information( this,
                  tr( "Error" ),
                  tr( "Error: " ) + status );
            return( status );
         
         }
      }

      else
      {
         // Maybe the profile has been deleted, so let's
         // delete the xml file and avoid confusion later
         QDir d( dirname );
         QString filename = runID      + "." 
                          + "RIProfile.xml";
         if ( d.exists( filename ) && ! d.remove( filename ) )
            qDebug() << "Unable to remove file" << filename;

      }
   }

   // Status is OK
   QMessageBox::information( this,
         tr( "Success" ),
         QString::number( fileCount ) + " " + 
         runID + tr( " files written." ) );
  
   if ( saveStatus == NOT_SAVED )
      saveStatus = HD_ONLY;

   // If we are here, it's ok to sync with DB
   ExpData.syncOK = true;

   enableRunIDControl( false );

   // Save the main plots
   QString dir    = US_Settings::reportDir() + "/" + runID;
   if ( ! QDir( dir ).exists() )      // make sure the directory exists
      QDir().mkdir( dir );
   int save_currentTriple = currentTriple;
   data_plot->setVisible( false );
   for ( int i = 0; i < triples.size(); i++ )
   {
      currentTriple = i;
      QString triple = US_Util::compressed_triple( triples[ i ].tripleDesc );
      QString filename = dir + "/cnvt." + triple + ".raw.svg";

      // Redo current plot and write it to a file
      plot_current();
      int status = US_GuiUtil::save_plot( filename, data_plot );
      if ( status != 0 )
         qDebug() << filename << "plot not saved";
   }

   // Restore original plot
   currentTriple = save_currentTriple;
   plot_current();
   data_plot->setVisible( true );

   return( US_Convert::OK );
}

void US_ConvertGui::saveUS3DB( void )
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

      return;
   }

   // Ok, let's make sure they know what'll happen
   if ( saveStatus == BOTH )
   {
     int status = QMessageBox::information( this,
              tr( "Warning" ),
              tr( "This will overwrite the raw data currently in the " ) +
              tr( "database, and all existing edit profiles, models "  ) +
              tr( "and noise data will be deleted too. Proceed? "      ),
              tr( "&OK" ), tr( "&Cancel" ),
              0, 0, 1 );
     if ( status != 0 ) return;
   }

   else
   {
     int status = QMessageBox::information( this,
              tr( "Warning" ),
              tr( "Once this data is written to the DB you will not "  ) +
              tr( "be able to make changes to it without erasing the " ) +
              tr( "edit profiles, models and noise files too. Proceed? " ),
              tr( "&OK" ), tr( "&Cancel" ),
              0, 0, 1 );
     if ( status != 0 ) return;
   }

   // First check some of the data with the DB
   int status = US_ConvertIO::checkDiskData( ExpData, triples, &db );

   if ( status == US_DB2::NO_PERSON )    // Investigator or operator doesn't exist
   {
      QMessageBox::information( this,
            tr( "Error" ),
            tr( "This investigator or instrument operator was not found \n" ) +
            tr( "in the database.\n") );
      return;
   }

   if ( status == US_DB2::BADGUID )
   {
      QMessageBox::information( this,
            tr( "Error" ),
            tr( "Bad GUID format.\n" ) +
            tr( "Please click on Edit Run Information and re-select hardware.\n") );
      return;
   }

   if ( status == US_DB2::NO_ROTOR )
   {
      QMessageBox::information( this,
            tr( "Error" ),
            tr( "Don't recognize the rotor configuration.\n" ) +
            tr( "Please click on Edit Run Information and re-select hardware.\n") );
      return;
   }

   if ( status != US_DB2::OK && status != US_DB2::NO_RAWDATA )
   {
      // US_DB2::OK means we're updating; US_DB2::NO_RAWDATA means it's new
      QMessageBox::information( this,
            tr( "Error" ),
            db.lastError() + " (" + db.lastErrno() + ")\n" );
      return;
   }

   // Save updated files and prepare to transfer to DB
   status = saveUS3Disk();
   if ( status != US_Convert::OK )
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

   if ( ExpData.checkRunID( &db ) == US_DB2::OK && ( saveStatus != BOTH ) )
   {
      // Then the user is trying to overwrite a runID that is already in the DB
      QMessageBox::warning( this,
            tr( "Duplicate runID" ),
            tr( "This runID already exists in the database. To edit that "  
                "run information, load it from there to begin with.\n" ) );
      return;
   }

   // If saveStatus == BOTH already, then it came from the db to begin with
   // and it should be updated. Otherwise, there shouldn't be any database
   // records with this runID found
   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
   status = ExpData.saveToDB( ( saveStatus == BOTH ), &db );
   QApplication::restoreOverrideCursor();

   if ( status == US_DB2::NO_PROJECT )
   {
      QMessageBox::warning( this,
            tr( "Project missing" ),
            tr( "The project associated with this experiment could not be "
                "updated or added to the database.\n" ) );
      return;
   }

   else if ( status == US_DB2::DUPFIELD )
   {
      QMessageBox::warning( this,
            tr( "Duplicate runID" ),
            tr( "The runID already exists in the database.\n" ) );
      return;
   }

   else if ( status == US_DB2::ERROR )
   {
      // This is what happens in the case of RI data, and the xml is bad
      QMessageBox::warning( this,
            tr( "Bad RI XML" ),
            tr( "There was a problem with the xml data read from the database.\n" ) );
   }

   else if ( status != US_DB2::OK )
   {
      QMessageBox::warning( this,
            tr( "Problem saving experiment information" ),
            tr( "MySQL Error : " ) + db.lastError() + " (" + QString::number( status ) + ")" );
      return;
   }

   // If the data came from the database in the first place,
   // then this function erases all the edit profiles, models 
   // and noise files in the database too. However, if one
   // changes most of the things here ( solution, rotor, etc. )
   // it would invalidate the data anyway.
   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
   QString writeStatus = US_ConvertIO::writeRawDataToDB( ExpData, triples, dir, &db );
   QApplication::restoreOverrideCursor();

   if ( ! writeStatus.isEmpty() )
   {
      QMessageBox::warning( this,
            tr( "Problem saving experiment" ),
            tr( "Unspecified database error: " ) + writeStatus );
      return;
   }

   QMessageBox::information( this, tr( "Record saved" ),
         tr( "The run information has been saved to the DB successfully \n" ) );

   saveStatus = BOTH;
   enableRunIDControl( false );

   saveReportsToDB();
}

void US_ConvertGui::saveReportsToDB( void )
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

      return;
   }

   // Get a list of report files produced by us_convert
   QString dir = US_Settings::reportDir() + "/" + runID;
   QDir d( dir, "cnvt*", QDir::Name, QDir::Files | QDir::Readable );
   d.makeAbsolute();
   QStringList files = d.entryList( QDir::Files );

   // Add any run details files to it
   QDir d2( dir, "rundetail*", QDir::Name, QDir::Files | QDir::Readable );
   d2.makeAbsolute();
   files << d2.entryList( QDir::Files );

   // Create US_Report object
   QString now = QDateTime::currentDateTime().toString();
   US_Report myReport;
   myReport.reset();
   myReport.runID = runID;
   myReport.title = runID + " Report";
   myReport.html  = "<p>Report created " + now + "</p>";

   // Save all us_convert report files to the DB
   QString errorMsg = "";
   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
   foreach( QString file, files )
   {
      // Edit data ID is not known yet, so use 1. It goes in the report document
      //   table itself, so we're not overwriting anything.
      US_Report::Status status = myReport.saveDocumentFromFile( dir, file, &db, 1 );
      if ( status != US_Report::REPORT_OK )
      {
         errorMsg += file + " was not saved to report database; error code: "
                          + QString::number( status ) + "\n";
         qDebug() << "US_ConvertGui.saveDocumentFromFile error: " 
                  << db.lastError() << db.lastErrno();
      }
   }
   QApplication::restoreOverrideCursor();

   if ( ! errorMsg.isEmpty() )
   {
      QMessageBox::warning( this,
            tr( "Problem saving reports to DB" ),
            errorMsg );
   }

}

bool US_ConvertGui::read( void )
{
   // Ask for data directory
   QString dir = QFileDialog::getExistingDirectory( this, 
         tr( "Raw Data Directory" ),
         US_Settings::dataDir(),
         QFileDialog::DontResolveSymlinks );

   // Restore area beneath dialog
   qApp->processEvents();

   if ( dir.isEmpty() ) return( false ); 

   dir.replace( "\\", "/" );  // WIN32 issue

   return( read( dir ) );
}

bool US_ConvertGui::read( QString dir )
{
   // Get legacy file names
   QDir d( dir, "*", QDir::Name, QDir::Files | QDir::Readable );
   d.makeAbsolute();
   if ( dir.right( 1 ) != "/" ) dir += "/"; // Ensure trailing /

   // See if we need to fix the runID
   QStringList components = dir.split( "/", QString::SkipEmptyParts );
   QString new_runID = components.last();
   QRegExp rx( "[^A-Za-z0-9_-]" );

   int pos = 0;
   while ( ( pos = rx.indexIn( new_runID ) ) != -1 )
      new_runID.replace( pos, 1, "_" );         // Replace 1 char at position pos

   // Set the runID and directory
   runID = new_runID;
   le_runID ->setText( runID );
   le_runID2->setText( runID );
   le_dir   ->setText( dir );
   currentDir  = QString( dir );

   oldRunType = runType;            // let's see if the runType changes

   // Read the data
   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
   US_Convert::readLegacyData( dir, legacyData, runType );
   QApplication::restoreOverrideCursor();

   if ( legacyData.size() == 0 ) return( false );

   // if runType has changed, let's clear out xml data too
   if ( oldRunType != runType ) ExpData.clear();

   return( true );
}

bool US_ConvertGui::convert( void )
{
   double tolerance = (double)ct_tolerance->value() + 0.05;    // to stay between wl numbers

   // Convert the data
   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
   US_Convert::convertLegacyData( legacyData, allData, triples, 
                                  runType, tolerance );
   QApplication::restoreOverrideCursor();

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

bool US_ConvertGui::centerpieceInfo( void )
{
   if ( disk_controls->db() )
      return centerpieceInfoDB();

   else
      return centerpieceInfoDisk();
}

// Function to get abstractCenterpiece names from DB
bool US_ConvertGui::centerpieceInfoDB( void )
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

// Function to get abstractCenterpiece names from disk
// Right now use function that gets info from flat file,
// but later change to xml
bool US_ConvertGui::centerpieceInfoDisk( void )
{
   // First figure out the xml file name, and try to open it
   //QString home = qApp->applicationDirPath().remove( QRegExp( "/bin$" ) );
   QString home = US_Settings::appBaseDir();
   QFile f( home + "/etc/abstractCenterpieces.xml");

   if ( ! f.open( QIODevice::ReadOnly ) ) return false;

   QList<listInfo> options;
   QXmlStreamReader xml( &f );
   while ( ! xml.atEnd() )
   {
      xml.readNext();

      if ( xml.isStartElement() )
      {
         if ( xml.name() == "abstractCenterpiece" )
         {
            struct listInfo option;
            QXmlStreamAttributes a = xml.attributes();
            option.ID   = a.value("id").toString();
            option.text = a.value("name").toString();
            options << option;
         }
      }
   }

   bool error = xml.hasError();
   f.close();

   if ( error ) return false;

   cb_centerpiece->clear();
   if ( options.size() > 0 )
   {
      // Let's sort them so they come up like the DB
      for ( int i = 0; i < options.size() - 1; i++ )
         for ( int j = i + 1; j < options.size(); j++ )
            if ( options[ i ].text > options[ j ].text )
               options.swap( i, j );

      cb_centerpiece->addOptions( options );
   }

   return true;
}

void US_ConvertGui::plot_current( void )
{
   US_DataIO2::RawData currentData = allData[ currentTriple ];

   if ( currentData.scanData.empty() ) return;

   plot_titles();

   // Plot current data for cell / channel / wavelength triple
   plot_all();
   
   // Set the Scan spin boxes
   enableScanControls();
}

void US_ConvertGui::plot_titles( void )
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
      title = "Radial Absorbance Data\nRun ID: " + runID + "\n"  +
              "Cell: " + cell + " Wavelength: " + wl;
   }

   else if ( ( strncmp( currentData.type, "RI", 2 ) == 0 ) && isPseudo )
   {
      title = "Pseudo Absorbance Data\nRun ID: " + runID + "\n"  +
              "Cell: " + cell + " Wavelength: " + wl;
   }

   else if ( strncmp( currentData.type, "RI", 2 ) == 0 )
   {
      title = "Radial Intensity Data\nRun ID: " + runID + "\n"  +
              "Cell: " + cell + " Wavelength: " + wl;
      yLegend = "Radial Intensity";
   }

   else if ( strncmp( currentData.type, "IP", 2 ) == 0 )
   {
      title = "Interference Data\nRun ID: " + runID + "\n"  +
              "Cell: " + cell + " Wavelength: " + wl;
      yLegend = "Fringes";
   }

   else if ( strncmp( currentData.type, "FI", 2 ) == 0 )
   {
      title = "Fluorescence Intensity Data\nRun ID: " + runID + "\n"  +
              "Cell: " + cell + " Wavelength: " + wl;
      yLegend = "Fluorescence Intensity";
   }
      
   else if ( strncmp( currentData.type, "WA", 2 ) == 0 )
   {
      title = "Wavelength Data\nRun ID: " + runID + "\n"  +
              "Cell: " + cell + " Radius: " + wl;
      xLegend = "Wavelength";
      yLegend = "Value";
   }

   else if ( strncmp( currentData.type, "WI", 2 ) == 0 )
   {
      title = "Wavelength Intensity Data\nRun ID: " + runID + "\n"  +
              "Cell: " + cell + " Radius: " + wl;
      xLegend = "Wavelength";
      yLegend = "Value";
   }

   else
      title = "File type not recognized";
   
   data_plot->setTitle( title );
   data_plot->setAxisTitle( QwtPlot::yLeft, yLegend );
   data_plot->setAxisTitle( QwtPlot::xBottom, xLegend );

}

void US_ConvertGui::plot_all( void )
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
      US_Convert::Excludes currentExcludes = allExcludes[ currentTriple ];
      if ( currentExcludes.contains( i ) ) continue;
      US_DataIO2::Scan* s = &currentData.scanData[ i ];

//qDebug() << "readings.size = " << size;
      for ( int j = 0; j < size; j++ )
      {
         r[ j ] = currentData.radius( j );
         v[ j ] = s->readings  [ j ].value;

//qDebug() << "(r, v) = ( " << r[j] << ", " << v[j] << ")";
         if ( v[ j ] > 1.0e99 || isnan( v[ j ] ) )
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

void US_ConvertGui::replot( void )
{
  plot_all();
}

void US_ConvertGui::set_colors( const QList< int >& focus )
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

void US_ConvertGui::draw_vline( double radius )
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

void US_ConvertGui::db_error( const QString& error )
{
   QMessageBox::warning( this, tr( "Database Problem" ),
         tr( "Database returned the following error: \n" ) + error );
}

