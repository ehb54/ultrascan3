#include <QApplication>

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

   // Pushbuttons to load and reload data
   QPushButton* pb_load = us_pushbutton( tr( "Load Legacy Data" ) );
   connect( pb_load, SIGNAL( clicked() ), SLOT( load() ) );
   settings->addWidget( pb_load, row, 0 );

   QPushButton* pb_reload = us_pushbutton( tr( "Reload Data" ) );
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
   connect( pb_details, SIGNAL( clicked() ), SLOT( details() ) );
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
   connect( lw_triple, SIGNAL( itemDoubleClicked( QListWidgetItem* ) ),
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

   // Write pushbuttons
   pb_writeAll = us_pushbutton( tr( "Write All Data" ), false );
   connect( pb_writeAll, SIGNAL( clicked() ), SLOT( writeAll() ) );
   settings->addWidget( pb_writeAll, row++, 0, 1, 2 );

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
   pb_writeAll   ->setEnabled( false );
   pb_details    ->setEnabled( false );
   pb_cancelref  ->setEnabled( false );
   pb_buffer     ->setEnabled( false );
   pb_analyte    ->setEnabled( false );
   pb_applyAll   ->setEnabled( false );

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
   includes.clear();
   triples.clear();
   allData.clear();
   RP_averaged        = false;
   show_plot_progress = true;
   ExpData.triples.clear();

   data_plot      ->detachItems();
   picker         ->disconnect();
   data_plot      ->setAxisScale( QwtPlot::xBottom, 5.7, 7.3 );
   data_plot      ->setAxisScale( QwtPlot::yLeft  , 0.0, 1.5 );
   grid           = us_grid( data_plot );
   data_plot      ->replot();

   pb_define      ->setEnabled( false );
   pb_process     ->setEnabled( false );
   step           = NONE;

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
      US_DataIO2::beckmanRawScan d = legacyData[ i ];

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
         if ( i != legacyData.size() - 1 ) continue;

         US_DataIO2::reading r = d.readings[ j ];

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
   plot_current();

   connect( ct_from, SIGNAL( valueChanged ( double ) ),
                     SLOT  ( focus_from   ( double ) ) );

   connect( ct_to  , SIGNAL( valueChanged ( double ) ),
                     SLOT  ( focus_to     ( double ) ) );

   // Ok to enable some buttons now
   pb_include     ->setEnabled( true );
   pb_writeAll    ->setEnabled( true );
   pb_details     ->setEnabled( true );
   pb_buffer      ->setEnabled( true );
   pb_analyte     ->setEnabled( true );
   cb_centerpiece ->setEnabled( true );
   pb_expinfo     ->setEnabled( true );
   pb_editExpinfo ->setEnabled( true );

   if ( runType == "RI" )
      pb_reference->setEnabled( true );

   else if ( runType == "RA" && ss_limits.size() < 2 )
   {
      // Allow user to define subsets, if he hasn't already
      pb_define   ->setEnabled( true );
   } 

}

// User pressed the reload data button
// Legacy data is already supposed to be present
void US_Convert::reload( void )
{
   bool success = false;

   triples.clear();

   // In this case the runType is not changing
   oldRunType = runType;

   // Figure out all the triple combinations and convert data
   success = convert();

   if ( ! success ) return;

   setTripleInfo();
   plot_current();

   connect( ct_from, SIGNAL( valueChanged ( double ) ),
                     SLOT  ( focus_from   ( double ) ) );

   connect( ct_to  , SIGNAL( valueChanged ( double ) ),
                     SLOT  ( focus_to     ( double ) ) );

   // Ok to enable some buttons now
   pb_include     ->setEnabled( true );
   pb_writeAll    ->setEnabled( true );
   pb_details     ->setEnabled( true );
   pb_buffer      ->setEnabled( true );
   pb_analyte     ->setEnabled( true );
   cb_centerpiece ->setEnabled( true );
   pb_expinfo     ->setEnabled( true );
   pb_editExpinfo ->setEnabled( true );

   if ( runType == "RI" )
      pb_reference->setEnabled( true );

   else if ( runType == "RA" && ss_limits.size() < 2 )
   {
      // Allow user to define subsets, if he hasn't already
      pb_define   ->setEnabled( true );
   } 

}

bool US_Convert::read( void )
{
   // Ask for data directory
   QString dir = QFileDialog::getExistingDirectory( this, 
         tr( "Raw Data Directory" ),
         US_Settings::dataDir(),
         QFileDialog::DontResolveSymlinks );

/*
Method 2:
   QString filename = QFileDialog::getOpenFileName( this,
         tr( "Raw Data Directory" ),
         US_Settings::dataDir() );

   QFileInfo fileinfo( filename );
   QString dir = fileinfo.absolutePath();
*/

// qDebug() << "Dir = " << dir;

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
   saveDir  = QString( dir );

   oldRunType = runType;            // let's see if the runType changes

   // Read the data
   US_ProcessConvert* dialog 
      = new US_ProcessConvert( this, dir, legacyData, runType );
   delete dialog;

   if ( legacyData.size() == 0 ) return( false );

   // if runType has changed, let's clear out xml data too
   if ( oldRunType != runType ) ExpData.clear();

   return( true );
}

void US_Convert::setTripleInfo( void )
{
   // Load them into the list box
   lw_triple->clear();
   lw_triple->addItems( triples );
   QListWidgetItem* item = lw_triple->item( 0 );   // select the item at row 0
   lw_triple->setItemSelected( item, true );
   currentTriple = 0;
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
      = new US_ProcessConvert( this, legacyData, allData, triples, runType, tolerance, ss_limits );
   delete dialog;

   if ( allData.size() == 0 ) return( false );

   le_description->setText( allData[ 0 ].description );
   saveDescription = QString( allData[ 0 ].description ); 

   currentTriple = 0;     // Now let's show the user the first one
   return( true );
}

void US_Convert::details( void )
{
   US_RunDetails2* dialog
      = new US_RunDetails2( allData, runID, saveDir, triples );
   dialog->exec();
   qApp->processEvents();
   delete dialog;
}

void US_Convert::changeTriple( QListWidgetItem* )
{
   currentTriple = lw_triple->currentRow();
   int ndx       = findTripleIndex();

   le_dir         -> setText( saveDir );
   le_description -> setText( saveDescription );

   le_bufferInfo  -> setText( ExpData.triples[ ndx ].bufferDesc  );
   le_analyteInfo -> setText( ExpData.triples[ ndx ].analyteDesc );

   // Reset maximum scan control values
   reset_scan_ctrls();

   // Reset apply all button
   reset_ccw_ctrls();

   // Redo plot
   plot_current();
}

int US_Convert::writeAll( void )
{
   if ( allData[ 0 ].scanData.empty() ) return NODATA; 

   // See if we need to update the runID
   QRegExp rx( "^[A-Za-z0-9_]{1,20}$" );
   QString new_runID = le_runID->text();
      
   if ( rx.indexIn( new_runID ) >= 0 )
   {
      runID = new_runID;
      plot_titles();
   }

   le_runID->setText( runID );

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
      = new US_ProcessConvert( this, status, allData, ExpData, triples, runType, runID, dirname );
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
         QString::number( triples.size() ) + " " + 
         runID + tr( " files written." ) );

   return( OK );
}

void US_Convert::plot_current( void )
{
   US_DataIO2::RawData currentData = allData[ currentTriple ];

   if ( currentData.scanData.empty() ) return;

   plot_titles();

   // Initialize include list
   init_includes();
   
   // Plot current data for cell / channel / wavelength triple
   plot_all();
   
   // Set the Scan spin boxes
   reset_scan_ctrls();
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

void US_Convert::init_includes( void )
{
   includes.clear();
   for ( int i = 0; i < allData[ currentTriple ].scanData.size(); i++ ) 
      includes << i;
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
      if ( ! includes.contains( i ) ) continue;
      US_DataIO2::Scan* s = &currentData.scanData[ i ];

      for ( int j = 0; j < size; j++ )
      {
         //r[ j ] = currentData.x[ j ].radius;
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
   }
   else
   {
      pb_exclude->setEnabled( true );
   }

   QList< int > focus;  // We don't care if -1 is in the list
   for ( int i = from - 1; i <= to - 1; i++ ) focus << i;  

   set_colors( focus );

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

void US_Convert::exclude_scans( void )
{
   int scanStart = (int)ct_from->value();
   int scanEnd   = (int)ct_to  ->value();

   // Let's remove back to front---the array
   // shifts with each deletion
   // Works when single scan too
   for ( int i = scanEnd - 1; i >= scanStart - 1; i-- )
      includes.removeAt( scanStart - 1 );

   reset_scan_ctrls();

   replot();
}

void US_Convert::include( void )
{
   init_includes();
   reset_scan_ctrls();

   replot();
}

// Reset the boundaries on the scan controls
void US_Convert::reset_scan_ctrls( void )
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

void US_Convert::reset_ccw_ctrls( void )
{
   int ndx = findTripleIndex();

   // The centerpiece combo box
   cb_centerpiece->setCurrentIndex( ExpData.triples[ ndx ].centerpiece );

   // Let's calculate if we're eligible to copy this triple info to all
   US_ExpInfo::TripleInfo triple = ExpData.triples[ ndx ];
   pb_applyAll  -> setEnabled( false );
   if ( triples.size()   > 1 && 
        triple.analyteID > 0 &&
        triple.bufferID  > 0 )
      pb_applyAll ->setEnabled( true );
}

void US_Convert::cClick( const QwtDoublePoint& p )
{
   switch ( step )
   {
      case SPLIT :
         draw_vline( p.x() );
         ss_limits << p.x();
         break;

      case REFERENCE :
         if ( reference_start == 0.0 )
            start_reference( p );

         else
            process_reference( p );
     
      default :
         break;

   }

}

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

/*
   for ( int i = 0; i < ss_limits.size(); i++ )
   {
      qDebug() << "Radius point " << i + 1 << ": " << ss_limits[ i ];
   }
*/

   // Now that we know we're subdividing, let's reconvert the file
   reset();
   load( saveDir );

   // We don't need this any more, and it interferes with subsequent
   //  loads of RA data. 
   ss_limits.clear();
}

void US_Convert::define_reference( void )
{
   connect( picker, SIGNAL( cMouseUp( const QwtDoublePoint& ) ),
                    SLOT  ( cClick  ( const QwtDoublePoint& ) ) );

   reference_start = 0.0;
   reference_end   = 0.0;
   pb_reference ->setEnabled( false );

   step = REFERENCE;
}

void US_Convert::start_reference( const QwtDoublePoint& p )
{
   reference_start   = p.x();

   draw_vline( reference_start );
   data_plot->replot();
}

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

/*
   qDebug() << "Starting Radius: " << reference_start;
   qDebug() << "Ending Radius:   " << reference_end;
*/

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

      //while ( referenceData.x[ j ].radius < reference_end && j < ref_size )
      while ( referenceData.radius( j ) < reference_end && j < ref_size )
      {
         sum += s.readings[ j ].value;
         count++;
         j++;
      }
      RP_averages << sum / count;
   }

/*
   for ( int i = 0; i < referenceData.scanData.size(); i++ )
   {
      qDebug() << "Average " << i + 1 << ": " << RP_averages[ i ];
   }
*/

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

   pb_reference  ->setEnabled( true );
   pb_cancelref  ->setEnabled( false );
   currentTriple = 0;
   lw_triple->setCurrentRow( currentTriple );

   plot_current();
}

void US_Convert::newExpInfo( void )
{
   getExpInfo( false );
}

void US_Convert:: editExpInfo( void )
{
   getExpInfo( true );
}

void US_Convert::getExpInfo( bool editing )
{
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
   ExpData.runTemp = QString::number( sum / count );
   strcpy( ExpData.opticalSystem, allData[ 0 ].type );

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

void US_Convert::updateExpInfo( US_ExpInfo::ExperimentInfo& d )
{
   // Update local copy
   ExpData = d;

   // The runID could have changed
   le_runID      ->setText( ExpData.runID );
}

void US_Convert::cancelExpInfo( void )
{
   ExpData.clear();
}

void US_Convert::selectBuffer( void )
{
   US_BufferGui* buffer_dialog = new US_BufferGui( ExpData.invID, true );         // Ask for a signal

   connect( buffer_dialog, SIGNAL( valueBufferID ( const QString& ) ),
            this,          SLOT  ( assignBuffer  ( const QString& ) ) );

   buffer_dialog->exec();
}

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
      connect_error( db.lastError() );
      return;
   }

   QStringList q( "get_buffer_info" );
   q << ( bufferID );
   db.query( q );

   if ( db.next() )
   {
      ExpData.triples[ ndx ].bufferDesc = db.value( 1 ).toString();
      le_bufferInfo -> setText( db.value( 1 ).toString() );
   }

   reset_ccw_ctrls();
}

void US_Convert::selectAnalyte( void )
{
   US_AnalyteGui* analyte_dialog = new US_AnalyteGui( ExpData.invID, true ); 

   connect( analyte_dialog, SIGNAL( valueChanged  ( US_Analyte ) ),
            this,           SLOT  ( assignAnalyte ( US_Analyte ) ) );

   analyte_dialog->exec();
}

void US_Convert::assignAnalyte( US_Analyte data )
{
   US_Passwd pw;
   QString masterPW = pw.getPasswd();
   US_DB2 db( masterPW );

   if ( db.lastErrno() != US_DB2::OK )
   {
      connect_error( db.lastError() );
      return;
   }

   // Get analyteID
   QStringList q( "get_analyteID" );
   q << data.guid;

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
      ExpData.triples[ ndx ].analyteDesc = db.value( 4 ).toString();
      le_analyteInfo -> setText( db.value( 4 ).toString() );
   }

   reset_ccw_ctrls();
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
      ExpData.triples[ i ].bufferDesc  = triple.bufferDesc;
      ExpData.triples[ i ].analyteID   = triple.analyteID;
      ExpData.triples[ i ].analyteDesc = triple.analyteDesc;
   }
}

int US_Convert::findTripleIndex ( void )
{
   // See if the current triple has been added already
   for (int i = 0; i < ExpData.triples.size(); i++ )
   {
      if ( ExpData.triples[ i ].tripleID == currentTriple )
         return i;
   }

   // Not found, so let's add it
   US_ExpInfo::TripleInfo d;
   d.tripleID = currentTriple;
   ExpData.triples << d;
   return ExpData.triples.size() - 1;
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

void US_Convert::connect_error( const QString& error )
{
   QMessageBox::warning( this, tr( "Connection Problem" ),
         tr( "Could not connect to database \n" ) + error );
}
