#include <QApplication>

#include "us_license_t.h"
#include "us_license.h"
#include "us_util.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_run_details2.h"
#include "us_mwlr_viewer.h"
#include "us_plot.h"
#include "us_math2.h"
#include "us_db2.h"
#include "us_passwd.h"
#include "us_investigator.h"
#include "us_constants.h"
#include "us_report.h"
#include "us_gui_util.h"
#include "us_util.h"
#include "us_editor.h"

#ifdef WIN32
#include <float.h>
#define isnan _isnan
#endif

int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );

   #include "main1.inc"

   // License is OK.  Start up.
   
   US_MwlRawViewer w;
   w.show();                   //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication
}

US_MwlRawViewer::US_MwlRawViewer() : US_Widgets()
{

   setWindowTitle( tr( "Multi-Wavelength Raw Data Viewer" ) );
   setPalette( US_GuiSettings::frameColor() );
   QFont font( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() );
   QFontMetrics fm( font );
   int fhigh    = fm.lineSpacing();

   QGridLayout* settings = new QGridLayout;

   lavgg        = 10;

   // Load the run
   QLabel*      lb_run       = us_banner( tr( "Load the Run" ) );

   // Define the GUI elements
                pb_import    = us_pushbutton( tr( "Import MWL Run" ) );
                pb_details   = us_pushbutton( tr( "Run Details" ), false );

   QLabel*      lb_dir       = us_label( tr( "Directory" ), -1 );
                le_dir       = us_lineedit( "", -1, true );

   QLabel*      lb_runID     = us_label( tr( "Run ID:" ), -1 );
                le_runID     = us_lineedit( "", -1, true );

   QLabel*      lb_cellchn   = us_label( tr( "Cell / Channel" ), -1 );
                lw_cellchn   = us_listwidget();
   lw_cellchn->resize( fhigh * 5, lw_cellchn->width() );

   QLabel*      lb_iwavcnt   = us_label( tr( "Input Wavelengths:" ) );
                le_iwavcnt   = us_lineedit( "1118", -1, true );

   QLabel*      lb_owavcnt   = us_label( tr( "Output Wavelengths:" ) );
                le_owavcnt   = us_lineedit( "112",  -1, true );

   QLabel*      lb_avggcnt   = us_label( tr( "Lambda Average:" ) );
                ct_avggcnt   = us_counter( 2, 1, 100, 10 );
   ct_avggcnt->setStep( 1.0 );

   QLabel*      lb_wvlrange  = us_label( tr( "Wavelength Range:" ) );
                le_wvlrange  = us_lineedit( "250.0 to 650.1", -1, true );

   QLabel*      lb_radpts    = us_label( tr( "Radius Points:" ) );
                le_radpts    = us_lineedit( "281", -1, true );

   QLabel*      lb_radstart  = us_label( tr( "Radius Start:" ) );
                le_radstart  = us_lineedit( "6.5", -1, true );

   QLabel*      lb_radstep   = us_label( tr( "Radius Step:" ) );
                le_radstep   = us_lineedit( "0.005", -1, true );

   QLabel*      lb_nbrscans  = us_label( tr( "Scans per Wavelength:" ) );
                le_nbrscans  = us_lineedit( "20", -1, true );

   QLabel*      lb_pltwavln  = us_label( tr( "Wavelength to Plot:" ) );
                cmb_pltwavln = us_comboBox();

                pb_prev      = us_pushbutton( tr( "Previous Wavelength" ) );
                pb_next      = us_pushbutton( tr( "Next Wavelength" ) );

   // Standard pushbuttons
                pb_reset     = us_pushbutton( tr( "Reset" ) );
   QPushButton* pb_help      = us_pushbutton( tr( "Help" ) );
                pb_saveUS3   = us_pushbutton( tr( "Save" ) );
   QPushButton* pb_close     = us_pushbutton( tr( "Close" ) );

   connect( pb_details,   SIGNAL( clicked()    ),
            this,         SLOT(   runDetails() ) );
   connect( lw_cellchn,   SIGNAL( currentRowChanged ( int  ) ),
            this,         SLOT  ( changeCellCh(            ) ) );
   connect( ct_avggcnt,   SIGNAL( valueChanged( double     ) ),
            this,         SLOT  ( changeLambda( double     ) ) );
   connect( cmb_pltwavln, SIGNAL( currentIndexChanged( int ) ),
            this,         SLOT  ( changeWaveln(            ) ) );
   connect( pb_import,    SIGNAL( clicked()  ),
            this,         SLOT(   import()   ) );
   connect( pb_prev,      SIGNAL( clicked()  ),
            this,         SLOT(   prevWvpl() ) );
   connect( pb_next,      SIGNAL( clicked()  ),
            this,         SLOT(   nextWvpl() ) );
   connect( pb_reset,     SIGNAL( clicked()  ),
            this,         SLOT(   resetAll() ) );
   connect( pb_help,      SIGNAL( clicked()  ),
            this,         SLOT(    help()    ) );
   connect( pb_saveUS3,   SIGNAL( clicked()  ),
            this,         SLOT(   saveUS3()  ) );
   connect( pb_close,     SIGNAL( clicked()  ),
            this,         SLOT(   close()    ) );

   // Do the left-side layout
   int row = 0;
   settings->addWidget( lb_run,        row++, 0, 1, 4 );
   settings->addWidget( lb_dir,        row++, 0, 1, 4 );
   settings->addWidget( le_dir,        row++, 0, 1, 4 );
   settings->addWidget( lb_runID,      row,   0, 1, 1 );
   settings->addWidget( le_runID,      row++, 1, 1, 3 );
   settings->addWidget( pb_import,     row,   0, 1, 2 );
   settings->addWidget( pb_details,    row++, 2, 1, 2 );
   settings->addWidget( lb_cellchn,    row,   0, 1, 2 );
   settings->addWidget( lw_cellchn,    row++, 2, 1, 2 );
   settings->addWidget( lb_iwavcnt,    row,   0, 1, 2 );
   settings->addWidget( le_iwavcnt,    row++, 2, 1, 2 );
   settings->addWidget( lb_owavcnt,    row,   0, 1, 2 );
   settings->addWidget( le_owavcnt,    row++, 2, 1, 2 );
   settings->addWidget( lb_avggcnt,    row,   0, 1, 2 );
   settings->addWidget( ct_avggcnt,    row++, 2, 1, 2 );
   settings->addWidget( lb_wvlrange,   row,   0, 1, 2 );
   settings->addWidget( le_wvlrange,   row++, 2, 1, 2 );
   settings->addWidget( lb_radpts,     row,   0, 1, 2 );
   settings->addWidget( le_radpts,     row++, 2, 1, 2 );
   settings->addWidget( lb_radstart,   row,   0, 1, 2 );
   settings->addWidget( le_radstart,   row++, 2, 1, 2 );
   settings->addWidget( lb_radstep,    row,   0, 1, 2 );
   settings->addWidget( le_radstep,    row++, 2, 1, 2 );
   settings->addWidget( lb_nbrscans,   row,   0, 1, 2 );
   settings->addWidget( le_nbrscans,   row++, 2, 1, 2 );
   settings->addWidget( lb_pltwavln,   row,   0, 1, 2 );
   settings->addWidget( cmb_pltwavln,  row++, 2, 1, 2 );
   settings->addWidget( pb_prev,       row,   0, 1, 2 );
   settings->addWidget( pb_next,       row++, 2, 1, 2 );
   settings->addWidget( pb_reset,      row,   0, 1, 1 );
   settings->addWidget( pb_help,       row,   1, 1, 1 );
   settings->addWidget( pb_saveUS3,    row,   2, 1, 1 );
   settings->addWidget( pb_close,      row++, 3, 1, 1 );

   // Plot layout for the right side of window
   QBoxLayout* plot = new US_Plot( data_plot,
                                   tr( "Intensity Data" ),
                                   tr( "Radius (in cm)" ), 
                                   tr( "Intensity" ) );

   data_plot->setMinimumSize( 600, 400 );

   data_plot->enableAxis( QwtPlot::xBottom, true );
   data_plot->enableAxis( QwtPlot::yLeft  , true );

   data_plot->setAxisScale( QwtPlot::xBottom, 5.7, 7.3 );
   data_plot->setAxisScale( QwtPlot::yLeft  , 0.0, 1.5 );

   picker = new US_PlotPicker( data_plot );
   picker ->setRubberBand    ( QwtPicker::VLineRubberBand );
   picker->setMousePattern   ( QwtEventPattern::MouseSelect1,
                               Qt::LeftButton, Qt::ControlModifier );

   // Now let's assemble the page
   
   QVBoxLayout* left     = new QVBoxLayout;

   left->addLayout( settings );
   
   QVBoxLayout* right    = new QVBoxLayout;
   
   right->addLayout( plot );

   QHBoxLayout* main = new QHBoxLayout( this );
   main->setSpacing         ( 2 );
   main->setContentsMargins ( 2, 2, 2, 2 );

   main->addLayout( left );
   main->addLayout( right );

   main->setStretch( 0, 2 );
   main->setStretch( 1, 4 );

   reset();
   adjustSize();
   lw_cellchn->resize( fhigh * 5, lw_cellchn->width() );
}

void US_MwlRawViewer::reset( void )
{
   lw_cellchn->disconnect();
   lw_cellchn->clear();
   le_dir    ->setText( "" );
   le_runID  ->setText( "" );

   pb_import ->setEnabled( true  );
   pb_saveUS3->setEnabled( false );
   pb_details->setEnabled( false );

   // Clear any data structures
   orig_wvlns.clear();
   orig_reads.clear();
   curr_wvlns.clear();
   curr_reads.clear();

   data_plot->detachItems();
   picker   ->disconnect();
   data_plot->setAxisScale( QwtPlot::xBottom, 5.7, 7.3 );
   data_plot->setAxisScale( QwtPlot::yLeft  , 0.0, 1.5 );
   grid          = us_grid( data_plot );
   data_plot->replot();
   connect( lw_cellchn,   SIGNAL( currentRowChanged ( int  ) ),
            this,         SLOT  ( changeCellCh(            ) ) );
}

void US_MwlRawViewer::resetAll( void )
{
   int status = QMessageBox::information( this,
            tr( "New Data Warning" ),
            tr( "This will erase all data currently on the screen, and " 
                "reset the program to its starting condition. No hard-drive "
                "data or database information will be affected. Proceed? " ),
            tr( "&OK" ), tr( "&Cancel" ),
            0, 0, 1 );
   if ( status != 0 ) return;

   reset();

   runID           = "";
   data_plot->setTitle( tr( "Intensity Data" ) );
}

// User pressed the import data button
void US_MwlRawViewer::import( QString dir )
{
   bool success = false;

   if ( dir.isEmpty() )
      success = read();                // Read the legacy data

   else
      success = read( dir );

qDebug() << "IMP: read success" << success;
}


// Enable the common dialog controls when there is data
void US_MwlRawViewer::enableControls( void )
{
   if ( orig_reads.size() == 0 )
      reset();

   else
   {
      // Ok to enable some buttons now
      pb_details     ->setEnabled( true );

      // Disable load buttons if there is data
      pb_import        ->setEnabled( false );
      pb_loadUS3       ->setEnabled( false );
      
      // Let's calculate if we're eligible to copy this triple info to all
      // or to save it
      // We have to check against GUID's, because solutions won't have
      // solutionID's yet if they are created as needed offline
      QRegExp rx( "^[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}$" );

      enableSaveBtn();
   }
}

// Enable the "save" button, if appropriate
// Let's use the same logic to populate the todo list too
void US_MwlRawViewer::enableSaveBtn( void )
{
   bool completed = true;

   // Have we filled out all the c/c/w info?
   // Check GUIDs, because solutionID's may not be present yet.
  
   // If we made it here, user can save
   pb_saveUS3 ->setEnabled( completed );
}

// Function to load US3 data
void US_MwlRawViewer::loadUS3( QString dir )
{
   if ( dir.isEmpty() )
      loadUS3Disk();

   else
      loadUS3Disk( dir );
}

void US_MwlRawViewer::loadUS3Disk( void )
{
   // Ask for data directory
   QString dir = QFileDialog::getExistingDirectory( this, 
         tr( "US3 Raw MWL Data Directory" ),
         US_Settings::resultDir(),
         QFileDialog::DontResolveSymlinks );
   
   // Restore area beneath dialog
   qApp->processEvents();

   if ( dir.isEmpty() ) return; 
   
   dir.replace( "\\", "/" );  // WIN32 issue
   if ( dir.right( 1 ) != "/" ) dir += "/"; // Ensure trailing /

   loadUS3Disk( dir );
}

void US_MwlRawViewer::loadUS3Disk( QString dir )
{
   int status = 0;
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
   runID       = new_runID;
   le_runID ->setText( runID );
   le_dir   ->setText( dir );
   currentDir  = QString( dir );

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

   // Ok to enable some buttons now
   enableControls();

   pb_details     ->setEnabled( true );

   enableSaveBtn();
}

void US_MwlRawViewer::runDetails( void )
{
   // Loop to insure all wavelengths have been averaged
   int saveccc   = currCellCh;

   for ( currCellCh = 0; currCellCh < ncellch; currCellCh++ )
      for ( int wavx = 0; wavx < nwaveln; wavx++ )
         averageWavlen( wavx );

   currCellCh    = saveccc;

   // Initialize statistics for data
   double ofdmax  = -1e99;
   double o90max  = -1e99;
   double ow1max  = -1e99;
   double ow2max  = -1e99;
   double ow3max  = -1e99;
   double afdmax  = -1e99;
   double a90max  = -1e99;
   double aw1max  = -1e99;
   double aw2max  = -1e99;
   double aw3max  = -1e99;
   double ofdmin  = 1e99;
   double o90min  = 1e99;
   double ow1min  = 1e99;
   double ow2min  = 1e99;
   double ow3min  = 1e99;
   double afdmin  = 1e99;
   double a90min  = 1e99;
   double aw1min  = 1e99;
   double aw2min  = 1e99;
   double aw3min  = 1e99;
   double ofdavg  = 0.0;
   double o90avg  = 0.0;
   double ow1avg  = 0.0;
   double ow2avg  = 0.0;
   double ow3avg  = 0.0;
   double afdavg  = 0.0;
   double a90avg  = 0.0;
   double aw1avg  = 0.0;
   double aw2avg  = 0.0;
   double aw3avg  = 0.0;
   int    ofdknt  = 0;
   int    o90knt  = 0;
   int    ow1knt  = 0;
   int    ow2knt  = 0;
   int    ow3knt  = 0;
   int    afdknt  = 0;
   int    a90knt  = 0;
   int    aw1knt  = 0;
   int    aw2knt  = 0;
   int    aw3knt  = 0;
   bool   wv1     = false;
   bool   wv2     = false;
   bool   wv3     = false;
   int    awvlo   = 0;
   int    awvmd   = nwaveln / 2;
   int    awvhi   = nwaveln - 1;
   int    owvlo   = 0;
   int    owvhi   = nwlorig - 1;
   double wvvmid  = curr_wvlns[ nwaveln / 2 ];
   int    owvmd   = orig_wvlns.indexOf( wvvmid );
   int    rplo    = ( npoints * 5 ) / 100;
   int    rphi    = npoints - rplo;
   double rdata   = 0.0;
   int    wvx     = 0;
   int    rpx     = 0;

   // Accumulate statistics for original data
   for ( int ii = 0; ii < ntriplo; ii++ )
   {
      wv1      = ( wvx == owvlo );
      wv2      = ( wvx == owvmd );
      wv3      = ( wvx == owvhi );
      rpx      = 0;

      for ( int jj = 0; jj < nradpt; jj++ )
      {
         rdata    = orig_reads[ ii ][ jj ];
         ofdmin   = qMin( ofdmin, rdata );
         ofdmax   = qMax( ofdmax, rdata );
         ofdavg  += rdata;
         ofdknt++;
         if ( rpx > rplo  &&  rpx < rphi )
         {
            o90min   = qMin( o90min, rdata );
            o90max   = qMax( o90max, rdata );
            o90avg  += rdata;
            o90knt++;
            if ( wv1 )
            {
               ow1min   = qMin( ow1min, rdata );
               ow1max   = qMax( ow1max, rdata );
               ow1avg  += rdata;
               ow1knt++;
            }
            if ( wv2 )
            {
               ow2min   = qMin( ow2min, rdata );
               ow2max   = qMax( ow2max, rdata );
               ow2avg  += rdata;
               ow2knt++;
            }
            if ( wv3 )
            {
               ow3min   = qMin( ow3min, rdata );
               ow3max   = qMax( ow3max, rdata );
               ow3avg  += rdata;
               ow3knt++;
            }
         }
         rpx++;
         if ( rpx >= npoints )
            rpx      = 0;
      }
      wvx++;
      if ( wvx >= nwlorig )
         wvx      = 0;
   }

   // Accumulate statistics for averaged data
   wvx     = 0;
   rpx     = 0;

   for ( int ii = 0; ii < ntripls; ii++ )
   {
      wv1      = ( wvx == awvlo );
      wv2      = ( wvx == awvmd );
      wv3      = ( wvx == awvhi );
      rpx      = 0;

      for ( int jj = 0; jj < nradpt; jj++ )
      {
         rdata    = curr_reads[ ii ][ jj ];
         afdmin   = qMin( afdmin, rdata );
         afdmax   = qMax( afdmax, rdata );
         afdavg  += rdata;
         afdknt++;
         if ( rpx > rplo  &&  rpx < rphi )
         {
            a90min   = qMin( a90min, rdata );
            a90max   = qMax( a90max, rdata );
            a90avg  += rdata;
            a90knt++;
            if ( wv1 )
            {
               aw1min   = qMin( aw1min, rdata );
               aw1max   = qMax( aw1max, rdata );
               aw1avg  += rdata;
               aw1knt++;
            }
            if ( wv2 )
            {
               aw2min   = qMin( aw2min, rdata );
               aw2max   = qMax( aw2max, rdata );
               aw2avg  += rdata;
               aw2knt++;
            }
            if ( wv3 )
            {
               aw3min   = qMin( aw3min, rdata );
               aw3max   = qMax( aw3max, rdata );
               aw3avg  += rdata;
               aw3knt++;
            }
         }
         rpx++;
         if ( rpx >= npoints )
            rpx      = 0;
      }
      wvx++;
      if ( wvx >= nwaveln )
         wvx      = 0;
   }

   // Now build the report text string
   ofdavg        /= (double)ofdknt;
   o90avg        /= (double)o90knt;
   ow1avg        /= (double)ow1knt;
   ow2avg        /= (double)ow2knt;
   ow3avg        /= (double)ow3knt;
   afdavg        /= (double)afdknt;
   a90avg        /= (double)a90knt;
   aw1avg        /= (double)aw1knt;
   aw2avg        /= (double)aw2knt;
   aw3avg        /= (double)aw3knt;
   double s1tem   = mwl_headers[ 0 ].temperature;
   double s1rot   = mwl_headers[ 0 ].rotor_speed;
   double s1omg   = mwl_headers[ 0 ].omega2t;
   double s1etm   = mwl_headers[ 0 ].elaps_time;
   int    lfx     = nfiles - 1;
   double s2tem   = mwl_headers[ lfx ].temperature;
   double s2rot   = mwl_headers[ lfx ].rotor_speed;
   double s2omg   = mwl_headers[ lfx ].omega2t;
   double s2etm   = mwl_headers[ lfx ].elaps_time;
   double owvmin  = orig_wvlns[ 0 ];
   double owvmax  = orig_wvlns[ nwlorig - 1 ];
   double owvain  = ( owvmax - owvmin ) / (double)( nwlorig - 1 );
   double awvain  = ( owvmax - owvmin ) / (double)( nwaveln - 1 );
   double wavv1   = owvmin;
   double wavv2   = orig_wvlns[ owvmd ];
   double wavv3   = owvmax;
   QString ffname = mwl_fnames[ 0 ];
   QString lfname = mwl_fnames[ lfx ];
   QString msg = tr( "Multi-Wavelength Statistics for RunID %1,\n" )
      .arg( runID );
   msg += tr( " from Directory %1\n\n" ).arg( currentDir );
   msg += tr( "General Data Set Values and Counts.\n" );
   msg += tr( "   First File Name:              %1\n" ).arg( ffname ); 
   msg += tr( "   Last File Name:               %1\n" ).arg( lfname ); 
   msg += tr( "   Scans:                        %1\n" ).arg( nscan );
   msg += tr( "   Radius Data Points:           %1\n" ).arg( npoints );
   msg += tr( "Values for 1st Cell/Channel Scan 1.\n" );
   msg += tr( "   Temperature                   %1\n" ).arg( s1tem );
   msg += tr( "   Omega^2T                      %1\n" ).arg( s1omg );
   msg += tr( "   Elapsed Time                  %1\n" ).arg( s1etm );
   msg += tr( "   RotorSpeed                    %1\n" ).arg( s1rot );
   msg += tr( "Values for Last Cell/Channel Scan %1.\n" ).arg( nscan );
   msg += tr( "   Temperature                   %1\n" ).arg( s2tem );
   msg += tr( "   Omega^2T                      %1\n" ).arg( s2omg );
   msg += tr( "   Elapsed Time                  %1\n" ).arg( s2etm );
   msg += tr( "   RotorSpeed                    %1\n" ).arg( s2rot );
   msg += tr( "Original Data Wavelengths.\n" );
   msg += tr( "   Count of Wavelengths:         %1\n" ).arg( nwlorig );
   msg += tr( "   Minimum:                      %1\n" ).arg( owvmin );
   msg += tr( "   Maximum:                      %1\n" ).arg( owvmax );
   msg += tr( "   Average Increment:            %1\n" ).arg( owvain );
   msg += tr( "%1-point Averaged Data Wavelengths.\n" ).arg( lavgg );
   msg += tr( "   Count of Wavelengths:         %1\n" ).arg( nwaveln );
   msg += tr( "   Minimum:                      %1\n" ).arg( owvmin );
   msg += tr( "   Maximum:                      %1\n" ).arg( owvmax );
   msg += tr( "   Average Increment:            %1\n" ).arg( awvain );
   msg += tr( "\nOriginal Full Intensity Data.\n" );
   msg += tr( "   Minimum:                      %1\n" ).arg( ofdmin );
   msg += tr( "   Maximum:                      %1\n" ).arg( ofdmax );
   msg += tr( "   Average:                      %1\n" ).arg( ofdavg );
   msg += tr( "%1-point Averaged Full Intensity Data.\n" ).arg( lavgg );
   msg += tr( "   Minimum:                      %1\n" ).arg( afdmin );
   msg += tr( "   Maximum:                      %1\n" ).arg( afdmax );
   msg += tr( "   Average:                      %1\n" ).arg( afdavg );
   msg += tr( "Original Middle 90% Intensity Data.\n" );
   msg += tr( "   Minimum:                      %1\n" ).arg( o90min );
   msg += tr( "   Maximum:                      %1\n" ).arg( o90max );
   msg += tr( "   Average:                      %1\n" ).arg( o90avg );
   msg += tr( "%1-point Averaged Middle 90% Intensity Data.\n" ).arg( lavgg );
   msg += tr( "   Minimum:                      %1\n" ).arg( a90min );
   msg += tr( "   Maximum:                      %1\n" ).arg( a90max );
   msg += tr( "   Average:                      %1\n" ).arg( a90avg );
   msg += tr( "%1 nm Original Middle 90% Intensity Data.\n" )
      .arg( wavv1 );
   msg += tr( "   Minimum:                      %1\n" ).arg( ow1min );
   msg += tr( "   Maximum:                      %1\n" ).arg( ow1max );
   msg += tr( "   Average:                      %1\n" ).arg( ow1avg );
   msg += tr( "%1 nm %2-point Averaged Middle 90% Intensity Data.\n" )
      .arg( wavv1 ).arg( lavgg );
   msg += tr( "   Minimum:                      %1\n" ).arg( aw1min );
   msg += tr( "   Maximum:                      %1\n" ).arg( aw1max );
   msg += tr( "   Average:                      %1\n" ).arg( aw1avg );
   msg += tr( "%1 nm Original Middle 90% Intensity Data.\n" )
      .arg( wavv2 );
   msg += tr( "   Minimum:                      %1\n" ).arg( ow2min );
   msg += tr( "   Maximum:                      %1\n" ).arg( ow2max );
   msg += tr( "   Average:                      %1\n" ).arg( ow2avg );
   msg += tr( "%1 nm %2-point Averaged Middle 90% Intensity Data.\n" )
      .arg( wavv2 ).arg( lavgg );
   msg += tr( "   Minimum:                      %1\n" ).arg( aw2min );
   msg += tr( "   Maximum:                      %1\n" ).arg( aw2max );
   msg += tr( "   Average:                      %1\n" ).arg( aw2avg );
   msg += tr( "%1 nm Original Middle 90% Intensity Data.\n" )
      .arg( wavv3 );
   msg += tr( "   Minimum:                      %1\n" ).arg( ow3min );
   msg += tr( "   Maximum:                      %1\n" ).arg( ow3max );
   msg += tr( "   Average:                      %1\n" ).arg( ow3avg );
   msg += tr( "%1 nm %2-point Averaged Middle 90% Intensity Data.\n" )
      .arg( wavv3 ).arg( lavgg );
   msg += tr( "   Minimum:                      %1\n" ).arg( aw3min );
   msg += tr( "   Maximum:                      %1\n" ).arg( aw3max );
   msg += tr( "   Average:                      %1\n" ).arg( aw3avg );

   // Open the dialog and display the report text
   US_Editor* editd = new US_Editor( US_Editor::DEFAULT, true );
   editd->setWindowTitle( tr( "Multi-Wavelength Data Statistics" ) );
   editd->move( pos() + QPoint( 200, 200 ) );
   editd->resize( 600, 500 );
   editd->e->setFont( QFont( US_Widgets::fixedFont().family(),
                             US_GuiSettings::fontSize() ) );
   editd->e->setText( msg );
   editd->show();
}

void US_MwlRawViewer::changeCellCh( void )
{
qDebug() << "changeCellCh";
   // Match the description to find the correct triple in memory
   QString cellch = lw_cellchn->currentItem()->text();
   QString cell   = cellch.section( "/", 0, 0 ).simplified();
   QString chan   = cellch.section( "/", 1, 1 ).simplified();
   int     icell  = cell.toInt() - 1;
   int     ichan  = QString( "ABCDEFGH" ).indexOf( chan );
   currCellCh     = icell * nchan + ichan;
   
   ct_avggcnt->setValue( lavgg );
   changeLambda( lavgg );
}

void US_MwlRawViewer::setCellChInfo( void )
{
   // Load them into the list box
   lw_cellchn->clear();
   currCellCh     = -1;          // indicates that it hasn't been selected yet

   QListWidgetItem* item = lw_cellchn->item( 0 );   // select the item at row 0
   lw_cellchn->setItemSelected( item, true );
}

// Function to save US3 data
void US_MwlRawViewer::saveUS3( void )
{
      saveUS3Disk();
}

int US_MwlRawViewer::saveUS3Disk( void )
{

   QDir        writeDir( US_Settings::resultDir() );
   QString     dirname = writeDir.absolutePath() + "/" + runID + "/";

   if ( ! writeDir.exists( runID ) )
   {
      if ( ! writeDir.mkpath( dirname ) )
      {
         QMessageBox::information( this,
               tr( "Error" ),
               tr( "Cannot write to " ) + writeDir.absolutePath() );
         return 1;
      }
   }

   // How many files should have been written?
   int fileCount = 0;

   // Status is OK
   QMessageBox::information( this,
         tr( "Success" ),
         QString::number( fileCount ) + " " + 
         runID + tr( " files written." ) );
  
   // Save the main plots
   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
   QString dir    = US_Settings::reportDir() + "/" + runID;
   if ( ! QDir( dir ).exists() )      // make sure the directory exists
      QDir().mkdir( dir );
   int saveccc    = currCellCh;
   data_plot->setVisible( false );

   // Make sure directory is empty
   QDir d( dir );
   QStringList rmvfilt( "*.svg" );
   QStringList rmvfiles = d.entryList( rmvfilt, QDir::Files, QDir::Name );
   QStringList rmvfilt2( "*.png" );
   rmvfiles << d.entryList( rmvfilt2, QDir::Files, QDir::Name );
   for ( int ii = 0; ii < rmvfiles.size(); ii++ )
      if ( ! d.remove( rmvfiles[ ii ] ) )
         qDebug() << "Unable to remove file" << rmvfiles[ ii ];

   // Restore original plot
   currCellCh     = saveccc;
   plot_current();
   data_plot->setVisible( true );
   QApplication::restoreOverrideCursor();

   return( 0 );
}

bool US_MwlRawViewer::read( void )
{
   // Ask for data directory
   QString dir = QFileDialog::getExistingDirectory( this, 
         tr( "Raw Data Directory" ),
         US_Settings::dataDir() + "/mwl",
         QFileDialog::DontResolveSymlinks );

   // Restore area beneath dialog
   qApp->processEvents();

   if ( dir.isEmpty() ) return( false ); 

   dir.replace( "\\", "/" );  // WIN32 issue

   return( read( dir ) );
}

bool US_MwlRawViewer::read( QString dir )
{
   // Get mwl file names
   QDir dirdir( dir, "*", QDir::Name, QDir::Files | QDir::Readable );
   dirdir.makeAbsolute();
   if ( dir.right( 1 ) != "/" ) dir += "/"; // Ensure trailing /

   // See if we need to fix the runID
   QStringList components = dir.split( "/", QString::SkipEmptyParts );
   QString new_runID = components.last();
   QRegExp rx( "[^A-Za-z0-9_-]" );

   int pos = 0;
   bool runID_changed = false;
   while ( ( pos = rx.indexIn( new_runID ) ) != -1 )
   {
      new_runID.replace( pos, 1, "_" );         // Replace 1 char at pos
      runID_changed = true;
   }

   // Let the user know if the runID name has changed
   if ( runID_changed )
   {
      QMessageBox::warning( this,
            tr( "RunID Name Changed" ),
            tr( "The runID name has been changed. It may consist only "
                "of alphanumeric \ncharacters, the underscore, and the "
                "hyphen. New runID: " ) + new_runID );
   }

   // Set the runID and directory
   runID       = new_runID;
   le_runID->setText( runID );
   le_dir  ->setText( dir );
   currentDir  = QString( dir );

   // Read the data
   QStringList mwrfiles = dirdir.entryList( QStringList( "*.mwrs" ),
         QDir::Files, QDir::Name );
   QStringList cells;
   QStringList chans;
   nscan    = -1;
   mwrfiles.sort();
   nfiles   = mwrfiles.size();
qDebug() << "RD: mwr nfiles" << nfiles;

   for ( int ii = 0; ii < nfiles; ii++ )
   {
      QString mwrfname = mwrfiles.at( ii );
      QString mwrfpath = dir + "/" + mwrfname;
      QString acell    = mwrfname.section( ".", -5, -5 );
      QString chann    = mwrfname.section( ".", -4, -4 );
      QString adesc    = mwrfname.section( ".", -3, -3 );
      QString ascan    = mwrfname.section( ".", -2, -2 );
      if ( !cells.contains( acell ) )  cells << acell;
      if ( !chans.contains( chann ) )  chans << chann;
      int     scann    = ascan.toInt();
              nscan    = qMax( nscan, scann );

      mwl_fnames << mwrfname;
      mwl_fpaths << mwrfpath;
   }

   nscan++;
   ncell    = cells.size();
   nchan    = chans.size();
   ncellch  = ncell * nchan;
qDebug() << "RD: cells size" << ncell;
qDebug() << "RD: chans size" << nchan;
qDebug() << "RD: nscan" << nscan;
   cells   .sort();
   chans   .sort();
   lw_cellchn->clear();
   cellchans  .clear();

   for ( int ii = 0; ii < ncell; ii++ )
   {
      for ( int jj = 0; jj < nchan; jj++ )
      {
         QString celchn = cells[ ii ] + " / " + chans[ jj ];
qDebug() << "RD: ii jj celchn" << ii << jj << celchn;
         lw_cellchn->addItem( celchn );
         cellchans << celchn;
      }
   }

qDebug() << "RD:  le_ns Text" << le_nbrscans->text();
   le_nbrscans->setText( QString::number( nscan ) );

   orig_wvlns.clear();
qDebug() << "RD:  le_ns setText return";

   // Read in all the headers
   for ( int ii = 0; ii < nfiles; ii++ )
   {
      QString mwlfp = mwl_fpaths[ ii ];
      QString mwlfn = mwl_fnames[ ii ];
qDebug() << "RD:   ii" << ii << "filename" << mwlfn;
      QFile fi( mwlfp );
      if ( ! fi.open( QIODevice::ReadOnly ) )
      {
         qDebug() << "*ERROR* Unable to open" << mwlfn;
         qDebug() << mwlfp;
         return ( false );
      }
      QDataStream ds( &fi );
      MwlHeader   hd;
      read_header( ds, hd );
      mwl_headers << hd;
qDebug() << "RD:     hd: cell chan icell ichann npoints nwaveln" << hd.cell
 << hd.channel << hd.icell << hd.ichann << hd.npoints << hd.nwaveln;

      if ( ii == 0 )
      { // At the first file, read in the wavelengths
         nwlorig     = hd.nwaveln;
         npoints     = hd.npoints;
         ntriplo     = nwlorig * ncellch;  // Number triples
         nradpt      = npoints * nscan;    // Number radius points per triple
         nwaveln     = nwlorig;
         ntripls     = ntriplo;

         read_wavelns( ds, orig_wvlns, nwlorig );
int mm=nwlorig/2;
int nn=nwlorig-1;
qDebug() << "RD:     wv: wv0 wvm wvn "
 << orig_wvlns[0] << orig_wvlns[mm] << orig_wvlns[nn];

         // And initialize the data vector
qDebug() << "RD:     nradpt ntripls" << nradpt << ntripls;
         QVector< double > wave_reads( nradpt, 0.0 );
         orig_reads.reserve( ntripls );
         for ( int tx = 0; tx < ntripls; tx++ )
            orig_reads << wave_reads;
      }
      else
      { // Otherwise, skip over wavelengths
         ds.skipRawData( hd.nwaveln * 4 );
      }

      int ccx   = hd.icell * nchan + hd.ichann;
      int tripx = ccx * nwlorig;
      int scnx  = mwlfn.section( ".", -2, -2 ).toInt() * npoints;
qDebug() << "RD: ccx scnx tripx" << ccx << scnx << tripx;

      // Now read in data

      for ( int wavx = 0; wavx < nwaveln; wavx++ )
      {
         read_rdata( ds, orig_reads[ tripx ], scnx, npoints );
if(wavx<3||(wavx+4)>nwaveln)
qDebug() << "RD:      wavx tripx" << wavx << tripx << "scnx" << scnx;
         tripx++;
      }
   }

   double wvlo  = orig_wvlns[ 0 ];
   double wvhi  = orig_wvlns[ nwlorig - 1 ];
   double rdlo  = mwl_headers[ 0 ].radius_start;
   double rdinc = mwl_headers[ 0 ].radius_step;
   lavgg        = 10;
   nwaveln      = qRound( (double)nwlorig / (double) lavgg );
   le_iwavcnt ->setText( QString::number( nwlorig ) );
   le_owavcnt ->setText( QString::number( nwaveln ) );
   le_wvlrange->setText( QString::number( wvlo ) + " to " +
                         QString::number( wvhi ) );
   le_radpts  ->setText( QString::number( npoints ) );
   le_radstart->setText( QString::number( rdlo  ) );
   le_radstep ->setText( QString::number( rdinc ) );
   le_nbrscans->setText( QString::number( nscan ) );

   lw_cellchn->setCurrentRow( 0 );
   changeCellCh( );

   return( true );
}

bool US_MwlRawViewer::convert( void )
{
   return ( true );
}

void US_MwlRawViewer::plot_current( void )
{
   if ( orig_reads.size() == 0 )
      return;

   plot_titles();

   plot_all();
}

void US_MwlRawViewer::plot_titles( void )
{
   QString cellch   = lw_cellchn->currentItem()->text();
   int     wvx      = cmb_pltwavln->currentIndex();
   double  wavl     = curr_wvlns[ wvx ];
   QString waveln   = QString::number( wavl );
   QString wl       = QString::number( qRound( wavl ) );
   QString triple   = cellch + " / " + wl;

   // Plot Title and legends
   QString title    = "Radial Intensity Data\nRun ID: " + runID +
              "    Triple: " + triple;
   QString xLegend  = "Radius (in cm)";
   QString yLegend  = "Radial Intensity at " + waveln + " nm";

   data_plot->setTitle( title );
   data_plot->setAxisTitle( QwtPlot::yLeft,   yLegend );
   data_plot->setAxisTitle( QwtPlot::xBottom, xLegend );

}

void US_MwlRawViewer::plot_all( void )
{
   data_plot->detachItems();
   grid           = us_grid( data_plot );

   QVector< double > rvec( npoints );
   QVector< double > vvec( npoints );
   double* rr     = rvec.data();
   double* vv     = vvec.data();

   double  radstr = mwl_headers[ 0 ].radius_start;
   double  radinc = mwl_headers[ 0 ].radius_step;
   int     wvx    = cmb_pltwavln->currentIndex();
   int     trx    = currCellCh * nwaveln + wvx;
   double  radval = radstr;
   double  readvl = 0.0;
   int     rdx    = 0;

   for ( int ii = 0; ii < nscan; ii++ )
   {
      radval         = radstr;

      for ( int jj = 0; jj < npoints; jj++ )
      {
         readvl         = curr_reads[ trx ][ rdx++ ];
         rr[ jj ]       = radval;
         vv[ jj ]       = readvl;
         radval        += radinc;
      }

      QString title = tr( "Raw Data at scan " ) + QString::number( ii );

      QwtPlotCurve* curv = us_curve( data_plot, title );
      curv->setData( rr, vv, npoints );

   }

   data_plot->setAxisAutoScale( QwtPlot::yLeft   );
   data_plot->setAxisAutoScale( QwtPlot::xBottom );
   
   data_plot->replot();
   pb_details->setEnabled( true );
}

void US_MwlRawViewer::read_header( QDataStream& ds, MwlHeader& head )
{
   char cbuf[ 28 ];

   //ds.readRawData( cbuf, 26 );
   ds.readRawData( cbuf, 24 );

   head.cell         = QChar( '0' | cbuf[ 0 ] );
   head.channel      = QChar( cbuf[ 1 ] );
   head.icell        = QString( head.cell ).toInt() - 1;
   head.ichann       = QString( "ABCDEFGH" ).indexOf( head.channel );
   head.nscans       = hword( cbuf + 2 );
   head.rotor_speed  = (double)( hword( cbuf + 4 ) );
   head.temperature  = (double)( hword( cbuf + 6 ) ) / 10.0;
   head.omega2t      = dword( cbuf + 8 );
   head.elaps_time   = iword( cbuf + 12 );
   head.npoints      = hword( cbuf + 16 );
   head.radius_start = (double)( hword( cbuf + 18 ) ) / 1000.0;
   head.radius_step  = (double)( hword( cbuf + 20 ) ) / 10000.0;
   //head.nwaveln      = iword( cbuf + 22 );
   head.nwaveln      = hword( cbuf + 22 );
}

int US_MwlRawViewer::hword( char* cc )
{
   unsigned char* cbuf = (unsigned char*)cc;
   int j0 = (int)cbuf[ 0 ] & 255;
   int j1 = (int)cbuf[ 1 ] & 255;
   return ( ( j0 << 8 ) | j1 );
}

int US_MwlRawViewer::iword( char* cbuf )
{
   int j0 = hword( cbuf );
   int j1 = hword( cbuf + 2 );
   return ( ( j0 << 16 ) | j1 );
}

float US_MwlRawViewer::fword( char* cbuf )
{
   int    ivalue = iword( cbuf );
   int*   iptr   = &ivalue;
   float* fptr   = (float*)iptr;
   return *fptr;
}

double US_MwlRawViewer::dword( char* cbuf )
{
   float fvalue  = fword( cbuf );
   return (double)fvalue;
}

void US_MwlRawViewer::read_wavelns( QDataStream& ds,
      QVector< double >& wavelns, int& nwvpts )
{
   char cbuf[ 4 ];

   wavelns.reserve( nwvpts );

   for ( int ii = 0; ii < nwvpts; ii++ )
   {
      //ds.readRawData( cbuf, 4 );
      //int    ivalue = iword( cbuf );
      //double wavln  = (double)ivalue / 10.0;
      ds.readRawData( cbuf, 2 );
      double wavln  = (double)hword( cbuf );
      wavelns << wavln;
   }
}

void US_MwlRawViewer::read_rdata( QDataStream& ds,
      QVector< double >& reads, int& scnx, int& npoints )
{
   char cbuf[ 4 ];
   int  kk      = scnx;

   for ( int ii = 0; ii < npoints; ii++ )
   {
      ds.readRawData( cbuf, 4 );
      int    ivalue = iword( cbuf );
      double rreadv = (double)ivalue / 1000.0;
      reads[ kk++ ] = rreadv;
   }
}
 
void US_MwlRawViewer::changeLambda( double value )
{
qDebug() << "changeLambda";
   cmb_pltwavln->disconnect();

   lavgg       = value;
   nwaveln     = qRound( (double)nwlorig / lavgg );
   le_owavcnt->setText( QString::number( nwaveln ) );
   curr_wvlns.fill( 0.0, nwaveln );
   int pknt    = cmb_pltwavln->count();
   int prex    = cmb_pltwavln->currentIndex();
   int curx    = ( pknt == 0 ) ? ( nwaveln / 2 ) : ( prex * nwaveln / pknt );
   cmb_pltwavln->clear();

   double winc = (double)( nwlorig - 1 ) / (double)( nwaveln - 1 );
   double wcur = 0.0;

   for ( int kk = 0; kk < nwaveln; kk++ )
   { // Construct wavelength array at lambda increment from original
      int ii           = qRound( wcur );
      curr_wvlns[ kk ] = orig_wvlns[ ii ];
      QString citem    = QString::number( curr_wvlns[ kk ] );
      wcur            += winc;

      cmb_pltwavln->addItem( citem );
   }

   // Initially construct the reduced data, unaveraged
   ntripls     = nwaveln * ncellch;  // Number triples
   curr_lavgs.resize( ntripls );
   curr_reads.resize( ntripls );
qDebug() << "  chgLmb nwaveln ntripls" << nwaveln << ntripls;
   int wvx     = 0;
   int ccx     = 0;

   for ( int kk = 0; kk < ntripls; kk++ )
   {
      double wavl = curr_wvlns[ wvx ];
      int    wvxo = orig_wvlns.indexOf( wavl );
      int    trxo = ccx * nwaveln + wvxo;
      int    trx  = ccx * nwaveln + wvx;

      curr_lavgs[ trx ]  = 0;
      curr_reads[ trx ]  = orig_reads[ trxo ];

      if ( (++wvx) >= nwaveln )
      {
         wvx         = 0;
         ccx++;
      }
   }

   // Select wavelength in current list that is closest to previous selection
qDebug() << "  chgLmb wavln-curx" << curx;
   connect( cmb_pltwavln, SIGNAL( currentIndexChanged( int ) ),
            this,         SLOT  ( changeWaveln(            ) ) );

   cmb_pltwavln->setCurrentIndex( curx );
   if ( curx == prex )
      changeWaveln();
}
 
void US_MwlRawViewer::changeWaveln( void )
{
qDebug() << "changeWaveln";
   int    wvx    = cmb_pltwavln->currentIndex();

   // Average at the current wavelength if not already done
   averageWavlen( wvx );

   // Plot what we have
   plot_current();

   pb_prev->setEnabled( ( wvx > 0 ) );
   pb_next->setEnabled( ( wvx < ( nwaveln -1 ) ) );
}

void US_MwlRawViewer::prevWvpl( void )
{
   int wvx = cmb_pltwavln->currentIndex() - 1;

   if ( wvx < 1 )
   {
      wvx     = 0;
      pb_prev->setEnabled( false );
   }

   cmb_pltwavln->setCurrentIndex( wvx );
}

void US_MwlRawViewer::nextWvpl( void )
{
   int wvx = cmb_pltwavln->currentIndex() + 1;

   if ( ( wvx + 2 ) > nwaveln )
   {
      wvx     = nwaveln - 1;
      pb_next->setEnabled( false );
   }

   cmb_pltwavln->setCurrentIndex( wvx );
}

void US_MwlRawViewer::averageWavlen( int wvx )
{
   double wavl   = curr_wvlns[ wvx ];
   int    trx    = currCellCh * nwaveln + wvx;
   int    wvxo   = orig_wvlns.indexOf( wavl );
   int    navgg  = curr_lavgs[ trx ];

   if ( navgg != lavgg )
   { // Not currently averaged, so do so
      int    wvlo   = qMax( ( wvxo - lavgg / 2 ), 0       );
      int    wvhi   = qMin( ( wvlo + lavgg     ), nwlorig );
      int    trxos  = currCellCh * nwlorig + wvlo;
      double wcsum  = (double)( wvhi - wvlo );

      for ( int ii = 0; ii < nradpt; ii++ )
      {
         int    trxo   = trxos;
         double wvsum  = 0.0;
         for ( int jj = wvlo; jj < wvhi; jj++ )
         {
            wvsum        += orig_reads[ trxo++ ][ ii ];
         }

         curr_reads[ trx ][ ii ] = wvsum / wcsum;
      }
      
      curr_lavgs[ trx ]   = lavgg;
   }
}

