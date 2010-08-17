#include <QApplication>

#include <uuid/uuid.h>

#include "us_license_t.h"
#include "us_license.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_run_details2.h"
#include "us_plot.h"
#include "us_math2.h"
#include "us_buffer_gui.h"
#include "us_analyte_gui.h"
#include "us_convert.h"
#include "us_db2.h"
#include "us_passwd.h"
#include "us_process_convert.h"
#include "us_convertio.h"

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

   // Pushbuttons to load and reload data
   QPushButton* pb_load = us_pushbutton( tr( "Load Legacy Data" ) );
   connect( pb_load, SIGNAL( clicked() ), SLOT( load() ) );
   settings->addWidget( pb_load, row, 0 );

   pb_reload = us_pushbutton( tr( "Reset Data Parameters" ) );
   connect( pb_reload, SIGNAL( clicked() ), SLOT( reload() ) );
   settings->addWidget( pb_reload, row++, 1 );

   // External program to enter experiment information
   QBoxLayout* expButtons = new QHBoxLayout;

   pb_expinfo = us_pushbutton( tr( "New Experiment" ) );
   connect( pb_expinfo, SIGNAL( clicked() ), SLOT( newExpInfo() ) );
   expButtons->addWidget( pb_expinfo );
   pb_expinfo->setEnabled( false );

   pb_editExpinfo = us_pushbutton( tr( "Edit Experiment" ) );
   connect( pb_editExpinfo, SIGNAL( clicked() ), SLOT( editExpInfo() ) );
   expButtons->addWidget( pb_editExpinfo );
   pb_editExpinfo->setEnabled( false );

   settings->addLayout( expButtons, row, 0 );

   pb_details = us_pushbutton( tr( "Run Details" ), false );
   connect( pb_details, SIGNAL( clicked() ), SLOT( runDetails() ) );
   settings->addWidget( pb_details, row++, 1 );

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
   ccw->addWidget( lb_triple, row++, 0, 1, 4 );

   lw_triple = us_listwidget();
   lw_triple->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed );
   lw_triple->setMaximumHeight( 120 );
   lw_triple->setMaximumWidth ( 120 );
   connect( lw_triple, SIGNAL( itemClicked ( QListWidgetItem* ) ),
                       SLOT  ( changeTriple( QListWidgetItem* ) ) );
   ccw->addWidget( lw_triple, row, 0, 4, 1 );

   QLabel* lb_ccwinfo = us_label( tr( "Enter Associated c/c/w Info:" ) );
   ccw->addWidget( lb_ccwinfo, row++, 1, 1, 3 );

   // Set up centerpiece drop-down
   centerpieceInfo();
   cb_centerpiece = us_comboBox();
   cb_centerpiece -> addItems( centerpieceTypes );
   connect( cb_centerpiece, SIGNAL( currentIndexChanged( int ) ), 
                            SLOT  ( getCenterpieceIndex( int ) ) );
   ccw->addWidget( cb_centerpiece, row, 1, 1, 2 );

   pb_applyAll = us_pushbutton( tr( "Apply to All" ), false );
   connect( pb_applyAll, SIGNAL( clicked() ), SLOT( ccwApplyAll() ) );
   ccw->addWidget( pb_applyAll, row++, 3 );

   pb_buffer = us_pushbutton( tr( "Buffer" ), false );
   connect( pb_buffer, SIGNAL( clicked() ), SLOT( selectBuffer() ) );
   ccw->addWidget( pb_buffer, row, 1 );

   le_bufferInfo = us_lineedit( "", 1 );
   le_bufferInfo ->setPalette ( gray );
   le_bufferInfo ->setReadOnly( true );
   ccw->addWidget( le_bufferInfo, row++, 2, 1, 2 );

   pb_analyte = us_pushbutton( tr( "Analyte" ), false );
   connect( pb_analyte, SIGNAL( clicked() ), SLOT( selectAnalyte() ) );
   ccw->addWidget( pb_analyte, row, 1 );

   le_analyteInfo = us_lineedit( "", 1 );
   le_analyteInfo ->setPalette ( gray );
   le_analyteInfo ->setReadOnly( true );
   ccw->addWidget( le_analyteInfo, row++, 2, 1, 2 );

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

   // drop scan
   pb_dropScan = us_pushbutton( tr( "Drop Current c/c/w" ), false );
   connect( pb_dropScan, SIGNAL( clicked() ), SLOT( drop_reference() ) );
   settings->addWidget( pb_dropScan, row, 0 );

   // Write pushbuttons
   pb_savetoHD = us_pushbutton( tr( "Save to HD" ), false );
   connect( pb_savetoHD, SIGNAL( clicked() ), SLOT( savetoHD() ) );
   settings->addWidget( pb_savetoHD, row++, 1 );

   // load US3 data ( that perhaps has been done offline )
   pb_loadUS3 = us_pushbutton( tr( "Load US3 Data" ), true );
   connect( pb_loadUS3, SIGNAL( clicked() ), SLOT( loadUS3() ) );
   settings->addWidget( pb_loadUS3, row, 0 );

   // sync data with DB
   pb_syncDB = us_pushbutton( tr( "Sync with DB" ), false );
   connect( pb_syncDB, SIGNAL( clicked() ), SLOT( syncDB() ) );
   settings->addWidget( pb_syncDB, row++, 1 );

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
   lw_triple     ->clear();

   le_dir        ->setText( "" );

   le_description->setText( "" );
   le_runID      ->setText( "" );
   le_bufferInfo ->setText( "" );
   le_analyteInfo->setText( "" );

   pb_exclude    ->setEnabled( false );
   pb_include    ->setEnabled( false );
   pb_savetoHD   ->setEnabled( false );
   pb_details    ->setEnabled( false );
   pb_cancelref  ->setEnabled( false );
   pb_dropScan   ->setEnabled( false );
   pb_buffer     ->setEnabled( false );
   pb_analyte    ->setEnabled( false );
   pb_applyAll   ->setEnabled( false );
   pb_expinfo    ->setEnabled( false );
   pb_editExpinfo->setEnabled( false );
   pb_syncDB     ->setEnabled( false );

   pb_reload     ->setEnabled( true  );
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
   RP_averaged        = false;
   show_plot_progress = true;
   ExpData.triples.clear();
   ExpData.rpms.clear();

   data_plot      ->detachItems();
   picker         ->disconnect();
   data_plot      ->setAxisScale( QwtPlot::xBottom, 5.7, 7.3 );
   data_plot      ->setAxisScale( QwtPlot::yLeft  , 0.0, 1.5 );
   grid           = us_grid( data_plot );
   data_plot      ->replot();

   pb_define      ->setEnabled( false );
   pb_process     ->setEnabled( false );
   step           = NONE;

   enableRunIDControl( true );

   toleranceChanged = false;
   this->editing    = false;

   pb_reference   ->setEnabled( false );
}

void US_Convert::resetAll( void )
{
   reset();

   ExpData.clear();
   ss_limits.clear();
   reference_start = 0;
   reference_end   = 0;

   ct_tolerance    ->setMinValue(   0.0 );
   ct_tolerance    ->setMaxValue( 100.0 );
   ct_tolerance    ->setValue   (   5.0 );
   ct_tolerance    ->setStep( 1 );
}

// User changed the dataset separation tolerance
void US_Convert::toleranceValueChanged( double )
{
   toleranceChanged = true;
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
      tripleMap.clear();

      le_bufferInfo ->setText( "" );
      le_analyteInfo->setText( "" );
      pb_applyAll   ->setEnabled( false );
   
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
      pb_buffer      ->setEnabled( true );
      pb_analyte     ->setEnabled( true );
      cb_centerpiece ->setEnabled( true );
      pb_expinfo     ->setEnabled( true );
      pb_editExpinfo ->setEnabled( true );
      pb_dropScan    ->setEnabled( true );

      if ( runType == "RI" )
         pb_reference->setEnabled( true );
   
      else if ( runType == "RA" && ss_limits.size() < 2 )
      {
         // Allow user to define subsets, if he hasn't already
         pb_define   ->setEnabled( true );
      } 

      enableRunIDControl( ! this->editing );
         
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

void US_Convert::enableCCWControls( void )
{
   int ndx = findTripleIndex();

   // The centerpiece combo box
   cb_centerpiece->setCurrentIndex( ExpData.triples[ ndx ].centerpiece );

   // Let's calculate if we're eligible to copy this triple info to all
   US_ExpInfo::TripleInfo triple = ExpData.triples[ ndx ];
   pb_applyAll  -> setEnabled( false );
   if ( tripleMap.size()   > 1 && 
        triple.analyteID   > 0 &&
        triple.bufferID    > 0 )
   {
      pb_applyAll ->setEnabled( true );
      // pb_dropCurrent ->setEnabled( true );
   }
   
}

// Enable the "sync with DB" button, if appropriate
void US_Convert::enableSyncDB( void )
{
   // Have we made connection with the db?
   if ( ExpData.invID == 0 ||
        ExpData.triples.size() == 0 )
   {
      pb_syncDB ->setEnabled( false );
      return;
   }

   // If editing, we should have an expID
   if ( this->editing      && 
        ExpData.expID == 0 )
   {
      pb_syncDB ->setEnabled( false );
      return;
   }

   // Have we filled out all the c/c/w info?
   for ( int i = 0; i < tripleMap.size(); i++ )
   {
      int ndx = tripleMap[ i ];
      US_ExpInfo::TripleInfo triple = ExpData.triples[ ndx ];
      if ( triple.bufferID  == 0 ||
           triple.analyteID == 0 )
      {
         pb_syncDB ->setEnabled( false );
         return;
      }
   }

   // Information is there, but are we connected to the db? 
   US_Passwd pw;
   QString masterPW = pw.getPasswd();
   US_DB2 db( masterPW );

   if ( ! db.isConnected() )
   {
      pb_syncDB ->setEnabled( false );
      return;
   }

   // If we made it here, user can sync with DB
   pb_syncDB ->setEnabled( true );
}

// Process when the user changes the runID
void US_Convert::runIDChanged( void )
{
   // See if we need to update the runID
   QRegExp rx( "^[A-Za-z0-9_]{1,20}$" );
   QString new_runID = le_runID->text();
      
   if ( rx.indexIn( new_runID ) >= 0 )
   {
      runID = new_runID;
      plot_titles();
   }

   le_runID->setText( runID );
   ExpData.runID = runID;

   // Set the directory too
   QDir resultDir( US_Settings::resultDir() );
   currentDir = resultDir.absolutePath() + "/" + runID + "/";
   le_dir ->setText( currentDir );
}

void US_Convert::newExpInfo( void )
{
   // Verify connectivity
   US_Passwd pw;
   QString masterPW = pw.getPasswd();
   US_DB2 db( masterPW );

   if ( db.lastErrno() != US_DB2::OK )
   {
      QMessageBox::information( this,
             tr( "Error" ),
             tr( "This is the dialog where you associate your experiment " ) +
             tr( "with information in a database. Please verify your "     ) +
             tr( "connection parameters before proceeding. You can still " ) +
             tr( "create your auc file and do this later.\n" ) );
      return;
   }

   // Create a new GUID for the experiment as a whole
   uuid_t uuid;
   char uuidc[ 37 ];

   uuid_generate( uuid );
   uuid_unparse( (unsigned char*)uuid, uuidc );

   ExpData.expGUID = QString( uuidc );

   getExpInfo( false );
}

void US_Convert:: editExpInfo( void )
{
   // Verify connectivity
   US_Passwd pw;
   QString masterPW = pw.getPasswd();
   US_DB2 db( masterPW );

   if ( db.lastErrno() != US_DB2::OK )
   {
      QMessageBox::information( this,
             tr( "Error" ),
             tr( "This is the dialog where you associate your experiment " ) +
             tr( "with information in a database. Please verify your "     ) +
             tr( "connection parameters before proceeding. You can still " ) +
             tr( "create your auc file and do this later.\n" ) );
      return;
   }

   // Preview if invID and expID are in the database before proceeding
   QStringList q( "get_experiment_info_by_runID" );
   q << le_runID -> text();
   db.query( q );
   db.next();

   if ( db.lastErrno() == US_DB2::NOROWS )
   {
      QMessageBox::information( this,
             tr( "Error" ),
             tr( "The current run ID is not found in the database;\n" ) +
             tr( "Click the New Experiment button to add it.\n" ) );
      return;
   }

   // Ok, edit the experiment info
   getExpInfo( true );
}

void US_Convert::getExpInfo( bool editing )
{
   this->editing = editing;

   ExpData.runID = le_runID -> text();

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

   US_ExpInfo* expInfo = new US_ExpInfo( ExpData, editing );

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
   this->editing = false;
   enableControls();
}

void US_Convert::runDetails( void )
{
   US_RunDetails2* dialog
      = new US_RunDetails2( allData, runID, currentDir, triples );
   dialog->exec();
   qApp->processEvents();
   delete dialog;
}

void US_Convert::changeTriple( QListWidgetItem* )
{
   int row       = lw_triple->currentRow();
   currentTriple = tripleMap[ row ];
   int ndx       = findTripleIndex();

   le_dir         -> setText( currentDir );
   le_description -> setText( saveDescription );

   le_bufferInfo  -> setText( ExpData.triples[ ndx ].bufferDesc  );
   le_analyteInfo -> setText( ExpData.triples[ ndx ].analyteDesc );

   // Reset maximum scan control values
   enableScanControls();

   // Reset apply all button
   enableCCWControls();

   // Redo plot
   plot_current();
}

void US_Convert::setTripleInfo( void )
{
   // Load them into the list box
   lw_triple->clear();
   foreach ( int ndx, tripleMap )
      lw_triple->addItem( triples[ ndx ] );

   QListWidgetItem* item = lw_triple->item( 0 );   // select the item at row 0
   lw_triple->setItemSelected( item, true );
   currentTriple = tripleMap[ 0 ];
}

int US_Convert::findTripleIndex ( void )
{
   // See if the current triple has been added already
   for (int i = 0; i < tripleMap.size(); i++ )
   {
      if ( tripleMap[ i ] == currentTriple )
         return i;
   }

   // Not found, so let's pick the first one
   currentTriple = tripleMap[ 0 ];
   return 0;
}

void US_Convert::getCenterpieceIndex( int ndx )
{
   ExpData.triples[ findTripleIndex() ].centerpiece = ndx;
}

void US_Convert::ccwApplyAll( void )
{
   US_ExpInfo::TripleInfo triple = ExpData.triples[ findTripleIndex() ];

   // Copy selected fields only
   for ( int i = 0; i < ExpData.triples.size(); i++ )
   {
      ExpData.triples[ i ].centerpiece = triple.centerpiece;
      ExpData.triples[ i ].bufferID    = triple.bufferID;
      ExpData.triples[ i ].bufferGUID  = triple.bufferGUID;
      ExpData.triples[ i ].bufferDesc  = triple.bufferDesc;
      ExpData.triples[ i ].analyteID   = triple.analyteID;
      ExpData.triples[ i ].analyteGUID = triple.analyteGUID;
      ExpData.triples[ i ].analyteDesc = triple.analyteDesc;
   }

   enableControls();
   enableSyncDB();
}

// Create a dialog to request a buffer selection
void US_Convert::selectBuffer( void )
{
   US_BufferGui* buffer_dialog = new US_BufferGui( ExpData.invID, true );         // Ask for a signal

   connect( buffer_dialog, SIGNAL( valueBufferID ( const QString& ) ),
            this,          SLOT  ( assignBuffer  ( const QString& ) ) );

   buffer_dialog->exec();
}

// Get information about selected buffer
void US_Convert::assignBuffer( const QString& bufferID )
{
   int ndx = findTripleIndex();

   ExpData.triples[ ndx ].bufferID = bufferID.toInt();

   // Now get the corresponding description 
   US_Passwd pw;
   QString masterPW = pw.getPasswd();
   US_DB2 db( masterPW );

   if ( db.lastErrno() != US_DB2::OK )
   {
      db_error( db.lastError() );
      return;
   }

   QStringList q( "get_buffer_info" );
   q << bufferID;
   db.query( q );

   if ( db.next() )
   {
      ExpData.triples[ ndx ].bufferGUID = db.value( 0 ).toString();
      ExpData.triples[ ndx ].bufferDesc = db.value( 1 ).toString();
      le_bufferInfo -> setText( db.value( 1 ).toString() );
   }

   enableControls();
   enableSyncDB();
   enableCCWControls();
}

// Create dialog to request an analyte selection
void US_Convert::selectAnalyte( void )
{
   US_AnalyteGui* analyte_dialog = new US_AnalyteGui( ExpData.invID, true ); 

   connect( analyte_dialog, SIGNAL( valueChanged  ( US_Analyte ) ),
            this,           SLOT  ( assignAnalyte ( US_Analyte ) ) );

   analyte_dialog->exec();
}

// Get information about selected analyte
void US_Convert::assignAnalyte( US_Analyte data )
{
   US_Passwd pw;
   QString masterPW = pw.getPasswd();
   US_DB2 db( masterPW );

   if ( db.lastErrno() != US_DB2::OK )
   {
      db_error( db.lastError() );
      return;
   }

   // Get analyteID
   QStringList q( "get_analyteID" );
   q << data.analyteGUID;

   db.query( q );
   if ( db.lastErrno() != US_DB2::OK ) return;

   db.next();
   QString analyteID = db.value( 0 ).toString();

   int ndx = findTripleIndex();
   ExpData.triples[ ndx ].analyteID = analyteID.toInt();

   // Now get the corresponding description 
   q.clear();
   q << "get_analyte_info";
   q << analyteID;
   db.query( q );

   if ( db.next() )
   {
      ExpData.triples[ ndx ].analyteGUID = db.value( 0 ).toString();
      ExpData.triples[ ndx ].analyteDesc = db.value( 4 ).toString();
      le_analyteInfo -> setText( db.value( 4 ).toString() );
   }

   enableControls();
   enableSyncDB();
   enableCCWControls();
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

void US_Convert::init_excludes( void )
{
   allExcludes.clear();
   Excludes x;
   for ( int i = 0; i < allData.size(); i++ )
      allExcludes << x;
}

void US_Convert::exclude_scans( void )
{
   int scanStart = (int)ct_from->value();
   int scanEnd   = (int)ct_to  ->value();

   for ( int i = scanStart - 1; i < scanEnd; i++ )
   {
      if ( ! allExcludes[ currentTriple ].contains( i ) )
         allExcludes[ currentTriple ] << i;
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
   RP_calc_avg();

   // Now that we have the averages, let's replot
   RP_reference_triple = currentTriple;

   // Default to displaying the first non-reference triple
   for ( int i = 0; i < allData.size(); i++ )
   {
      if ( i != RP_reference_triple )
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

void US_Convert::RP_calc_avg( void )
{
   if ( RP_averaged ) return;             // Average calculation has already been done

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

      int lastGood = j;
      int countBad = 0;
      //while ( referenceData.x[ j ].radius < reference_end && j < ref_size )
      while ( referenceData.radius( j ) < reference_end && j < ref_size )
      {
         // Check against excluded values
         if ( allExcludes[ currentTriple ].contains( j ) )
            countBad++;

         // Calculate average of before and after for intervening values
         else if ( countBad > 0 )
         {
            sum += ( ( s.readings[ lastGood ].value + s.readings[ j ].value ) / 2.0 )
                   * countBad;
            countBad = 0;
         }

         // Normal situation -- value is not excluded
         else
         {
            sum += s.readings[ j ].value;
            lastGood = j;
         }

         count++;
         j++;
      }
      RP_averages << sum / count;
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

            r->value = log10(RP_averages[ j ] / r->value );
         }
      }
      strncpy( currentData->type, "RP", 2);
   }

   RP_averaged = true;
   pb_cancelref ->setEnabled( true );
}

void US_Convert::cancel_reference( void )
{
   RP_averaged = false;
   allData     = RIData;
   RIData.clear();

   RP_averages.clear();
   reference_start = 0.0;
   reference_end   = 0.0;

   tripleMap.clear();
   for ( int i = 0; i < triples.size(); i++ )
      tripleMap << i;

   setTripleInfo();

   pb_reference  ->setEnabled( true );
   pb_cancelref  ->setEnabled( false );
   pb_dropScan   ->setEnabled( false );
   currentTriple = 0;
   lw_triple->setCurrentRow( currentTriple );

   plot_current();
}

void US_Convert::drop_reference( void )
{
   int currentRow = lw_triple->currentRow();
   tripleMap.removeAt( currentRow );

   setTripleInfo();           // Resets currentTriple
   int ndx       = findTripleIndex();

   le_dir         -> setText( currentDir );
   le_description -> setText( saveDescription );

   le_bufferInfo  -> setText( ExpData.triples[ ndx ].bufferDesc  );
   le_analyteInfo -> setText( ExpData.triples[ ndx ].analyteDesc );

   // Reset maximum scan control values
   enableScanControls();

   // Reset apply all button
   enableCCWControls();

   // Redo plot
   plot_current();
}

int US_Convert::savetoHD( void )
{
   if ( allData[ 0 ].scanData.empty() ) return NODATA; 

   QDir        writeDir( US_Settings::resultDir() );
   QString     dirname = writeDir.absolutePath() + "/" + runID + "/";

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
               tr( "Experiment information has not been defined yet. "    ) +
               tr( "Click 'OK' to proceed anyway, or click 'Cancel' "     ) +
               tr( "and then click on the 'Enter Experiment Information'" ) +
               tr( "button to enter this information first.\n\n "         ),
               tr( "&OK" ), tr( "&Cancel" ),
               0, 0, 1 );
      if ( status != 0 ) return NOT_WRITTEN;
   }

   if ( ExpData.triples.size() != triples.size() )
   {
      status = QMessageBox::information( this,
               tr( "Warning" ),
               tr( "A buffer and/or analyte has not been defined yet. " )  +
               tr( "Click 'OK' to proceed anyway, or click 'Cancel' "   )  +
               tr( "and enter the current c/c/w info for each cell, "   )  +
               tr( "channel, and wavelength combination first.\n\n "    ),
               tr( "&OK" ), tr( "&Cancel" ),
               0, 0, 1 );
      if ( status != 0 ) return NOT_WRITTEN;
   }

   // Write the data
   US_ProcessConvert* dialog 
      = new US_ProcessConvert( this );
      dialog->writeConvertedData( status, allData, ExpData, triples, 
                                  tripleMap, allExcludes, runType, runID, dirname );
   delete dialog;

   // Now try to communicate status
   if ( status == NOXML )
   {
      // Main xml data is missing
      QMessageBox::information( this,
            tr( "Warning" ),
            tr( "The experiment information file was not written. " ) +
            tr( "Please click on the " ) +
            tr( "'Enter Experiment Information' button \n\n " )    +
            QString::number( triples.size() ) + " "                + 
            runID + tr( " files written." ) );
      return( status );
   }

   else if ( status == PARTIAL_XML )
   {
      // xml data is missing for one or more triples
      QMessageBox::information( this,
            tr( "Warning" ),
            tr( "Buffer and/or analyte information is incomplete. Please click on the " ) +
            tr( "'Enter Current c/c/w Info' button for each "  ) +
            tr( "cell, channel, and wavelength combination \n\n " ) +
            QString::number( triples.size() ) + " "                + 
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
         QString::number( tripleMap.size() ) + " " + 
         runID + tr( " files written." ) );
  
   enableRunIDControl( false );

   return( OK );
}

void US_Convert::loadUS3( void )
{
   // Ask for data directory
   QString dir = QFileDialog::getExistingDirectory( this, 
         tr("US3 Raw Data Directory"),
         US_Settings::resultDir(),
         QFileDialog::DontResolveSymlinks );

   if ( dir.isEmpty() ) return; 

   dir.replace( "\\", "/" );  // WIN32 issue
   if ( dir.right( 1 ) != "/" ) dir += "/"; // Ensure trailing /

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
      dialog->reloadUS3Data( dir, allData, ExpData, triples, tripleMap, runType, runID );
   delete dialog;

   if ( allData.size() == 0 ) return;

   // Update triple information on screen
   setTripleInfo();
   int ndx       = findTripleIndex();

   le_bufferInfo  -> setText( ExpData.triples[ ndx ].bufferDesc  );
   le_analyteInfo -> setText( ExpData.triples[ ndx ].analyteDesc );

   // Restore description
   le_description->setText( allData[ 0 ].description );
   saveDescription = QString( allData[ 0 ].description );

   // Reset maximum scan control values
   enableScanControls();

   // Reset apply all button
   enableCCWControls();

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
   pb_buffer      ->setEnabled( true );
   pb_analyte     ->setEnabled( true );
   cb_centerpiece ->setEnabled( true );
   pb_expinfo     ->setEnabled( true );
   pb_editExpinfo ->setEnabled( true );

   // some things one can't do from here
   pb_reload      ->setEnabled( false );
   ct_tolerance   ->setEnabled( false );

   enableRunIDControl( false );

   if ( runType == "RI" )
      pb_reference->setEnabled( true );

   else if ( runType == "RA" && ss_limits.size() < 2 )
   {
      // Allow user to define subsets, if he hasn't already
      pb_define   ->setEnabled( true );
   } 

   enableSyncDB();
}

void US_Convert::syncDB( void )
{
   QString error;

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
   if ( this->editing )
      error = US_ConvertIO::updateDBExperiment( ExpData, tripleMap, dir );

   else
      error = US_ConvertIO::newDBExperiment( ExpData, tripleMap, dir );

   if ( ! error.isNull() )
   {
      db_error( error );
      return;
   }

   QMessageBox::information( this, tr( "Record saved" ),
         tr( "The experiment information has been synchronized with the DB successfully \n" ) );

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

   reset();
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
   // We need to set the default tolerances before converting
   if ( runType == "WA" )
   {
      // First of all, wavelength triples are ccr.
      lb_triple   ->setText( tr( "Cell / Channel / Radius" ) );
   
      if ( runType != oldRunType )
      {
         // We only need to adjust these if the runType has changed
         ct_tolerance->setMinimumWidth( 160 );
         ct_tolerance->setNumButtons  ( 3 );
         ct_tolerance->setRange       ( 0.0, 10.0 );
         ct_tolerance->setStep        ( 0.001 );
         ct_tolerance->setValue       ( 0.1 );
      }

   }
   else
   {
      // Most triples are ccw
      lb_triple   ->setText( tr( "Cell / Channel / Wavelength" ) );
   
      if ( runType != oldRunType )
      {
         // We only need to adjust these if the runType has changed
         ct_tolerance->setMinimumWidth( 160 );
         ct_tolerance->setNumButtons  ( 2 );
         ct_tolerance->setRange       ( 0.0, 100.0 );
         ct_tolerance->setStep        ( 1.0 );
         ct_tolerance->setValue       ( 5.0 );
      }

   }

   double tolerance = (double)ct_tolerance->value() + 0.05;    // to stay between wl numbers

   // Convert the data
   US_ProcessConvert* dialog 
      = new US_ProcessConvert( this );
      dialog->convertLegacyData( legacyData, allData, triples, 
                                 runType, tolerance, ss_limits );
   delete dialog;

   if ( allData.size() == 0 ) return( false );

   // Make sure all the parallel structures are initialized too
   tripleMap.clear();
   ExpData.triples.clear();
   ExpData.rpms.clear();
   for ( int i = 0; i < triples.size(); i++ )
   {
      US_ExpInfo::TripleInfo d;
      d.tripleID = i;
      ExpData.triples << d;
      tripleMap << i;
   }

   le_description->setText( allData[ 0 ].description );
   saveDescription = QString( allData[ 0 ].description ); 

   currentTriple = tripleMap[ 0 ];     // Now let's show the user the first one
   return( true );
}

bool US_Convert::centerpieceInfo( void )
{
   QString home = qApp->applicationDirPath().remove( QRegExp( "/bin$" ) );
   QFile f( home + "/etc/centerpiece.dat" );
   if ( ! f.open( QIODevice::ReadOnly | QIODevice::Text ) ) return false;
   QTextStream ts( &f );

   centerpieceTypes.clear();
   while ( ! ts.atEnd() )
   {
      // Read a line at a time
      QString line      = ts.readLine();

      // Make sure we skip the comments
      if ( line[ 0 ] != '#' ) 
      {
         QStringList parts = line.split(" ", QString::SkipEmptyParts );
         QString material = parts[ 1 ].toAscii();
         QString channels = QString::number( parts[ 2 ].toInt() * 2 );

         int pl_index = parts[ 2 ].toInt() + 4;           // bottom position for each channel
         QString pathlength = parts[ pl_index ].toAscii();

         QString c = material   + ", "
                   + channels   + " channels, "
                   + pathlength + "cm";

         centerpieceTypes << c;
      }
   }

   f.close();

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

   QString triple         = triples[ currentTriple ];
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

   else if ( strncmp( currentData.type, "RP", 2 ) == 0 )
   {
      title = "Pseudo Absorbance Data\nRun ID: "
            + runID + " Cell: " + cell + " Wavelength: " + wl;
   }

   else if ( strncmp( currentData.type, "IP", 2 ) == 0 )
   {
      title = "Interference Data\nRun ID: "
            + runID + " Cell: " + cell + " Wavelength: " + wl;
      yLegend = "Fringes";
   }

   else if ( strncmp( currentData.type, "RI", 2 ) == 0 )
   {
      title = "Radial Intensity Data\nRun ID: "
            + runID + " Cell: " + cell + " Wavelength: " + wl;
      yLegend = "Radial Intensity";
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
