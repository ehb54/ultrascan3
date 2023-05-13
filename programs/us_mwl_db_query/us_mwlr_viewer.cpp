#include <QApplication>

#include "us_mwlr_viewer.h"
#include "us_mwl_run.h"
#include "us_mwl_pltctrl.h"
#include "us_license_t.h"
#include "us_license.h"
#include "us_util.h"
#include "us_settings.h"
#include "us_gui_settings.h"
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
#include "us_images.h"
#if QT_VERSION < 0x050000
#define setSamples(a,b,c)  setData(a,b,c)
#define setMinimum(a)      setMinValue(a)
#define setMaximum(a)      setMaxValue(a)
#endif

#ifdef Q_OS_WIN
#include <float.h>
#define isnan _isnan
#endif

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug()
#endif

int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );

   #include "main1.inc"

   // License is OK.  Start up.
   
   US_MwlRawViewer ww;
   ww.show();                  //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication
}

US_MwlRawViewer::US_MwlRawViewer() : US_Widgets()
{
   const QChar chlamb( 955 );

   setWindowTitle( tr( "Multi-Wavelength Raw Data Viewer" ) );
   setPalette( US_GuiSettings::frameColor() );

   QGridLayout* settings = new QGridLayout;

   navgrec      = 10;
   is_wrecs     = true;
   dbg_level    = US_Settings::us_debug();
   p3d_ctld     = NULL;
   p3d_pltw     = NULL;
   QFont sfont( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() - 1 );
   QFontMetrics fmet( sfont );
   int fwid     = fmet.maxWidth();
   int lwid     = fwid * 4;
   int swid     = lwid + fwid;

   // Load controls     
   QLabel*      lb_run      = us_banner( tr( "Load the Run" ) );
                pb_loadMwl  = us_pushbutton( tr( "Load Raw MWL Data" ) );
                pb_loadAUC  = us_pushbutton( tr( "Load US3 MWL Data" ) );
                pb_reset    = us_pushbutton( tr( "Reset Data" ) );
                pb_details  = us_pushbutton( tr( "Data Details" ), false );

   QLabel*      lb_dir      = us_label( tr( "Directory" ), -1 );
                le_dir      = us_lineedit( "", -1, true );

   QLabel*      lb_runID    = us_label( tr( "Run ID:" ), -1 );
                le_runID    = us_lineedit( "", -1, true );

   QLabel*      lb_cellchn  = us_label( tr( "Cell / Channel" ), -1 );
                cb_cellchn  = us_comboBox();

   int rhgt     = le_runID->height();
   QLabel*      lb_recavg   = us_label( tr( "Record Average:" ), -1 );
                ct_recavg   = us_counter( 1, 1, 100, 1 );
   ct_recavg->setSingleStep( 1.0 );
   ct_recavg->setFont( sfont );
   ct_recavg->setMinimumWidth( lwid );
   ct_recavg->resize( rhgt, swid );
   ct_recavg->setValue( navgrec );

   // Plot Range controls     
   QLabel*      lb_prcntls  = us_banner( tr( "Plot Range Controls" ) );
   QLabel*      lb_rstart   = us_label( tr( "Radius Start:"   ), -1 );
                cb_rstart   = us_comboBox();
   QLabel*      lb_rend     = us_label( tr( "Radius End:"     ), -1 );
                cb_rend     = us_comboBox();
   QLabel*      lb_lstart   = us_label( tr( "%1 Start:"   ).arg( chlamb ), -1 );
                cb_lstart   = us_comboBox();
   QLabel*      lb_lend     = us_label( tr( "%1 End:"     ).arg( chlamb ), -1 );
                cb_lend     = us_comboBox();
                lb_pltrec   = us_label( tr( "Plot (W nm)" ) );
                cb_pltrec   = us_comboBox();

                pb_prev     = us_pushbutton( tr( "Previous" ) );
                pb_next     = us_pushbutton( tr( "Next" ) );
   pb_prev->setIcon( US_Images::getIcon( US_Images::ARROW_LEFT  ) );
   pb_next->setIcon( US_Images::getIcon( US_Images::ARROW_RIGHT ) );
   us_checkbox( tr( "X-axis Wavelength" ), ck_xwavlen, false );

   // Scan controls     
   QLabel*      lb_scanctl  = us_banner( tr( "Scan Control" ) );
   QLabel*      lb_from     = us_label( tr( "From:" ) );
   QLabel*      lb_to       = us_label( tr( "To:" ) );
                ct_from     = us_counter( 2, 0, 500, 1 );
                ct_to       = us_counter( 2, 0, 500, 1 );
                pb_exclude  = us_pushbutton( tr( "Exclude Scan Range" ) );
                pb_include  = us_pushbutton( tr( "Include All Scans"  ) );
   ct_from  ->setFont( sfont );
   ct_from  ->setMinimumWidth( lwid );
   ct_from  ->resize( rhgt, swid );
   ct_to    ->setFont( sfont );
   ct_to    ->setMinimumWidth( lwid );
   ct_to    ->resize( rhgt, swid );
   ct_from  ->setValue( 0 );
   ct_to    ->setValue( 0 );
   ct_from  ->setSingleStep( 1 );
   ct_to    ->setSingleStep( 1 );

   // Advanced Plotting controls
   QLabel*      lb_advplot  = us_banner( tr( "Advanced Plotting Control" ) );
                pb_plot2d   = us_pushbutton( tr( "Refresh 2D Plot" ) );
                pb_movie2d  = us_pushbutton( tr( "Show 2D Movie"   ) );
                pb_plot3d   = us_pushbutton( tr( "Plot 3D"         ) );
                pb_movie3d  = us_pushbutton( tr( "Show 3D Movie"   ) );
                pb_svplot   = us_pushbutton( tr( "Save Plot(s)"    ) );
                pb_svmovie  = us_pushbutton( tr( "Save Movie(s)"   ) );
   us_checkbox( tr( "Hold 3D Movie Colors Constant" ), ck_hcolorc, false );

   // Status and standard pushbuttons
   QLabel*      lb_status   = us_banner( tr( "Status" ) );
                le_status   = us_lineedit( tr( "(no data loaded)" ), -1, true );
   QPalette stpal;
   stpal.setColor( QPalette::Text, Qt::white );
   stpal.setColor( QPalette::Base, Qt::blue  );
   le_status->setPalette( stpal );

   QPushButton* pb_help     = us_pushbutton( tr( "Help" ) );
   QPushButton* pb_close    = us_pushbutton( tr( "Close" ) );

   // Signals and Slots
   connect( pb_loadMwl,   SIGNAL( clicked()      ),
            this,         SLOT  ( load_mwl_raw() ) );
   connect( pb_loadAUC,   SIGNAL( clicked()      ),
            this,         SLOT  ( load_auc_mwl() ) );
   connect( pb_reset,     SIGNAL( clicked()  ),
            this,         SLOT  ( resetAll() ) );
   connect( pb_details,   SIGNAL( clicked()    ),
            this,         SLOT  ( runDetails() ) );
   connect( cb_cellchn,   SIGNAL( currentIndexChanged( int ) ),
            this,         SLOT  ( changeCellCh( )            ) );
   connect( cb_rstart,    SIGNAL( currentIndexChanged( int ) ),
            this,         SLOT  ( changeRadius( )            ) );
   connect( cb_rend,      SIGNAL( currentIndexChanged( int ) ),
            this,         SLOT  ( changeRadius( )            ) );
   connect( cb_lstart,    SIGNAL( currentIndexChanged( int ) ),
            this,         SLOT  ( changeLambda( )            ) );
   connect( cb_lend,      SIGNAL( currentIndexChanged( int ) ),
            this,         SLOT  ( changeLambda( )            ) );
   connect( ct_recavg,    SIGNAL( valueChanged( double     ) ),
            this,         SLOT  ( changeAverage()            ) );
   connect( cb_pltrec,    SIGNAL( currentIndexChanged( int ) ),
            this,         SLOT  ( changeRecord( )            ) );
   connect( ck_xwavlen,   SIGNAL( toggled      ( bool ) ),
            this,         SLOT  ( changeRectype( bool ) ) );
   connect( pb_prev,      SIGNAL( clicked()  ),
            this,         SLOT  ( prevPlot() ) );
   connect( pb_next,      SIGNAL( clicked()  ),
            this,         SLOT  ( nextPlot() ) );
   connect( ct_from,      SIGNAL( valueChanged( double ) ),
            this,         SLOT  ( exclude_from( double ) ) );
   connect( ct_to,        SIGNAL( valueChanged( double ) ),
            this,         SLOT  ( exclude_to  ( double ) ) );
   connect( pb_exclude,   SIGNAL( clicked()       ),
            this,         SLOT  ( exclude_scans() ) );
   connect( pb_include,   SIGNAL( clicked()       ),
            this,         SLOT  ( include_scans() ) );
   connect( pb_plot2d,    SIGNAL( clicked()       ),
            this,         SLOT  ( changeCellCh()  ) );
   connect( pb_movie2d,   SIGNAL( clicked()       ),
            this,         SLOT  ( show_2d_movie() ) );
   connect( pb_plot3d,    SIGNAL( clicked()       ),
            this,         SLOT  ( plot_3d()       ) );
   connect( pb_movie3d,   SIGNAL( clicked()       ),
            this,         SLOT  ( show_3d_movie() ) );
   connect( pb_svplot,    SIGNAL( clicked()       ),
            this,         SLOT  ( save_plot()     ) );
   connect( pb_svmovie,   SIGNAL( clicked()       ),
            this,         SLOT  ( save_movie()    ) );
   connect( pb_help,      SIGNAL( clicked()  ),
            this,         SLOT  ( help()     ) );
   connect( pb_close,     SIGNAL( clicked()  ),
            this,         SLOT  ( close()    ) );

   // Do the left-side layout
   int row = 0;
   settings->addWidget( lb_run,        row++, 0, 1, 8 );
   settings->addWidget( lb_dir,        row++, 0, 1, 8 );
   settings->addWidget( le_dir,        row++, 0, 1, 8 );
   settings->addWidget( lb_runID,      row,   0, 1, 2 );
   settings->addWidget( le_runID,      row++, 2, 1, 6 );
   settings->addWidget( pb_loadMwl,    row,   0, 1, 4 );
   settings->addWidget( pb_loadAUC,    row++, 4, 1, 4 );
   settings->addWidget( pb_reset,      row,   0, 1, 4 );
   settings->addWidget( pb_details,    row++, 4, 1, 4 );
   settings->addWidget( lb_cellchn,    row,   0, 1, 4 );
   settings->addWidget( cb_cellchn,    row++, 4, 1, 4 );
   settings->addWidget( lb_prcntls,    row++, 0, 1, 8 );
   settings->addWidget( lb_rstart,     row,   0, 1, 2 );
   settings->addWidget( cb_rstart,     row,   2, 1, 2 );
   settings->addWidget( lb_rend,       row,   4, 1, 2 );
   settings->addWidget( cb_rend,       row++, 6, 1, 2 );
   settings->addWidget( lb_lstart,     row,   0, 1, 2 );
   settings->addWidget( cb_lstart,     row,   2, 1, 2 );
   settings->addWidget( lb_lend,       row,   4, 1, 2 );
   settings->addWidget( cb_lend,       row++, 6, 1, 2 );
   settings->addWidget( lb_recavg,     row,   0, 1, 2 );
   settings->addWidget( ct_recavg,     row,   2, 1, 2 );
   settings->addWidget( ck_xwavlen,    row++, 4, 1, 4 );
   settings->addWidget( lb_pltrec,     row,   0, 1, 2 );
   settings->addWidget( cb_pltrec,     row,   2, 1, 2 );
   settings->addWidget( pb_prev,       row,   4, 1, 2 );
   settings->addWidget( pb_next,       row++, 6, 1, 2 );
   settings->addWidget( lb_advplot,    row++, 0, 1, 8 );
   settings->addWidget( pb_plot2d,     row,   0, 1, 4 );
   settings->addWidget( pb_movie2d,    row++, 4, 1, 4 );
   settings->addWidget( pb_plot3d,     row,   0, 1, 4 );
   settings->addWidget( pb_movie3d,    row++, 4, 1, 4 );
   settings->addWidget( pb_svplot,     row,   0, 1, 4 );
   settings->addWidget( pb_svmovie,    row++, 4, 1, 4 );
   settings->addWidget( ck_hcolorc,    row++, 0, 1, 8 );
   settings->addWidget( lb_scanctl,    row++, 0, 1, 8 );
   settings->addWidget( lb_from,       row,   0, 1, 1 );
   settings->addWidget( ct_from,       row,   1, 1, 3 );
   settings->addWidget( lb_to,         row,   4, 1, 1 );
   settings->addWidget( ct_to,         row++, 5, 1, 3 );
   settings->addWidget( pb_exclude,    row,   0, 1, 4 );
   settings->addWidget( pb_include,    row++, 4, 1, 4 );
   settings->addWidget( lb_status,     row++, 0, 1, 8 );
   settings->addWidget( le_status,     row++, 0, 1, 8 );
   settings->addWidget( pb_help,       row,   0, 1, 4 );
   settings->addWidget( pb_close,      row++, 4, 1, 4 );

   // Plot layout for the right side of window
   QBoxLayout* plot = new US_Plot( data_plot,
                                   tr( "Intensity Data" ),
                                   tr( "Radius (in cm)" ), 
                                   tr( "Intensity" ) );

   data_plot->setMinimumSize( 600, 400 );

   data_plot->enableAxis( QwtPlot::xBottom, true );
   data_plot->enableAxis( QwtPlot::yLeft  , true );

   data_plot->setAxisScale( QwtPlot::xBottom, 5.8, 7.2 );
   data_plot->setAxisScale( QwtPlot::yLeft  , 0.0, 1.5 );

   picker = new US_PlotPicker( data_plot );
   picker->setRubberBand     ( QwtPicker::VLineRubberBand );
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
}

void US_MwlRawViewer::reset( void )
{
   cb_cellchn->disconnect();
   cb_cellchn->clear();
   le_dir    ->setText( "" );
   le_runID  ->setText( "" );

   pb_loadMwl->setEnabled( true );
   pb_loadAUC->setEnabled( true );
   pb_details->setEnabled( false );
   cb_cellchn->setEnabled( false );
   cb_rstart ->setEnabled( false );
   cb_rend   ->setEnabled( false );
   cb_lstart ->setEnabled( false );
   cb_lend   ->setEnabled( false );
   ct_recavg ->setEnabled( false );
   ck_xwavlen->setEnabled( false );
   cb_pltrec ->setEnabled( false );
   pb_prev   ->setEnabled( false );
   pb_next   ->setEnabled( false );
   ct_from   ->setEnabled( false );
   ct_to     ->setEnabled( false );
   pb_exclude->setEnabled( false );
   pb_include->setEnabled( false );
   pb_reset  ->setEnabled( false );
   ct_from   ->setEnabled( false );
   ct_to     ->setEnabled( false );
   pb_exclude->setEnabled( false );
   pb_include->setEnabled( false );
   pb_plot2d ->setEnabled( false );
   pb_movie2d->setEnabled( false );
   pb_plot3d ->setEnabled( false );
   pb_movie3d->setEnabled( false );
   pb_svplot ->setEnabled( false );
   pb_svmovie->setEnabled( false );
   ck_hcolorc->setEnabled( false );

   // Clear any data structures
   lambdas   .clear();
   radii     .clear();
   allData   .clear();
   excludes  .clear();
   curr_adata.clear();
   curr_cdata.clear();
   prev_cdata.clear();
   curr_recxs.clear();
   prev_recxs.clear();

   dataPlotClear( data_plot );
   picker   ->disconnect();
   data_plot->setAxisScale( QwtPlot::xBottom, 5.8, 7.2 );
   data_plot->setAxisScale( QwtPlot::yLeft  , 0.0, 1.5 );
   grid          = us_grid( data_plot );
   data_plot->replot();
   connect( cb_cellchn,   SIGNAL( currentIndexChanged( int ) ),
            this,         SLOT  ( changeCellCh(            ) ) );

   last_xmin       = -1.0;
   last_xmax       = -1.0;
   last_ymin       = -1.0;
   last_ymax       = -1.0;

   mwl_data.clear();
   le_status->setText( tr( "(no data loaded)" ) );
}

void US_MwlRawViewer::resetAll( void )
{
   if ( allData.size() > 0 )
   {
      int status = QMessageBox::information( this,
               tr( "New Data Warning" ),
               tr( "This will erase all data currently on the screen, and " 
                   "reset the program to its starting condition. No hard-drive "
                   "data or database information will be affected. Proceed? " ),
               tr( "&OK" ), tr( "&Cancel" ),
               0, 0, 1 );
      if ( status != 0 ) return;
   }

   reset();

   runID           = "";
   data_plot->setTitle( tr( "Intensity Data" ) );
}


// Enable the common dialog controls based on the presence of data
void US_MwlRawViewer::enableControls( void )
{
   if ( allData.size() == 0 )
   {  // If no data yet, just reset
      reset();
      return;
   }

   // Enable and disable controls now
   pb_loadMwl->setEnabled( false );
   pb_loadAUC->setEnabled( false );
   pb_reset  ->setEnabled( true );
   pb_details->setEnabled( true );
   cb_cellchn->setEnabled( true );
   cb_rstart ->setEnabled( true );
   cb_rend   ->setEnabled( true );
   cb_lstart ->setEnabled( true );
   cb_lend   ->setEnabled( true );
   ct_recavg ->setEnabled( true );
   ck_xwavlen->setEnabled( true );
   cb_pltrec ->setEnabled( true );
   pb_prev   ->setEnabled( true );
   pb_next   ->setEnabled( true );
   pb_plot2d ->setEnabled( true );
   pb_movie2d->setEnabled( true );
   ct_from   ->setEnabled( true );
   ct_to     ->setEnabled( true );
   pb_exclude->setEnabled( true );
   pb_plot3d ->setEnabled( true );
   pb_svplot ->setEnabled( true );
   pb_svmovie->setEnabled( true );
   ck_hcolorc->setEnabled( true );

   ncellch     = cellchans.size();
   nlambda     = lambdas  .size();
   nscan       = allData[ 0 ].scanCount();
   npoint      = allData[ 0 ].pointCount();
   ntpoint     = nscan * npoint;
   QStringList slrads;
   QStringList sllmbs;

   for ( int jj = 0; jj < npoint; jj++ )
      slrads << QString().sprintf( "%.5f", radii[ jj ] );

   for ( int jj = 0; jj < nlambda; jj++ )
      sllmbs << QString::number( lambdas[ jj ] );

   connect_ranges( false );
   cb_cellchn->clear();
   cb_rstart ->clear();
   cb_rend   ->clear();
   cb_lstart ->clear();
   cb_lend   ->clear();
   cb_pltrec ->clear();

   cb_cellchn->addItems( cellchans );
   cb_rstart ->addItems( slrads );
   cb_rend   ->addItems( slrads );
   cb_lstart ->addItems( sllmbs );
   cb_lend   ->addItems( sllmbs );
   cb_pltrec ->addItems( sllmbs );

   cb_cellchn->setCurrentIndex( 0 );
   cb_rstart ->setCurrentIndex( 0 );
   cb_rend   ->setCurrentIndex( npoint - 1 );
   cb_lstart ->setCurrentIndex( 0 );
   cb_lend   ->setCurrentIndex( nlambda - 1 );
   connect_ranges( true );

   have_rngs  = false;
   compute_ranges( );

   ct_from   ->setMaximum( nscan );
   ct_to     ->setMaximum( nscan );
   cb_cellchn->setCurrentIndex( 0 );
DbgLv(1) << "EnableControls: nlambda" << nlambda;

   cb_pltrec ->setCurrentIndex( nlambda / 2 );
   qApp->processEvents();

   changeCellCh();                          // Force a plot initialize
}

// Load MWL raw (.mwrs) data
void US_MwlRawViewer::load_mwl_raw( )
{
   // Ask for data directory
   QString dir = "";
   US_MwlRun lddiag( dir, true );
   if ( lddiag.exec() == QDialog::Rejected )
      return;

   // Restore area beneath dialog
   qApp->processEvents();
   if ( dir.isEmpty() ) return;
   dir.replace( "\\", "/" );  // WIN32 issue

   // Get mwl file names
   QDir dirdir( dir, "*", QDir::Name, QDir::Files | QDir::Readable );
   dirdir.makeAbsolute();
   if ( dir.right( 1 ) != "/" ) dir += "/"; // Ensure trailing /

   // See if we need to fix the runID
   QStringList components = dir.split( "/", QString::SkipEmptyParts );
   QString runType   = "RI";
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
   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor) );
   le_status->setText( tr( "Reading Raw MWL data ..." ) );

   mwl_data.import_data( currentDir, le_status );

   // Build the AUC equivalent
   mwl_data.build_rawData( allData );

   QApplication::restoreOverrideCursor();
   QApplication::restoreOverrideCursor();
   mwl_fnames = dirdir.entryList( QStringList( "*.mwrs" ),
                                  QDir::Files, QDir::Name );
   mwl_fnames.sort();
   ntriple     = mwl_fnames.size();
   runID       = new_runID;
   ncellch     = mwl_data.cellchannels( cellchans );
   radii.clear();
   radii << allData[ 0 ].xvalues;
   nscan       = allData[ 0 ].scanCount();
   npoint      = allData[ 0 ].pointCount();

DbgLv(1) << "RD: mwr ntriple" << ntriple;
DbgLv(1) << "RD: ncellch" << ncellch;
DbgLv(1) << "RD: nscan" << nscan << "npoint" << npoint;
DbgLv(1) << "RD:   rvS rvE" << radii[0] << radii[npoint-1];
   cb_cellchn->disconnect();
   cb_cellchn->clear();
   cb_cellchn->addItems( cellchans );
   connect( cb_cellchn,   SIGNAL( currentIndexChanged( int ) ),
            this,         SLOT  ( changeCellCh(            ) ) );

   nlambda      = mwl_data.lambdas_raw( lambdas );
   int wvlo     = lambdas[ 0 ];
   int wvhi     = lambdas[ nlambda - 1 ];
DbgLv(1) << "RD: nwl wvlo wvhi" << nlambda << wvlo << wvhi;
   ntriple      = nlambda * ncellch;  // Number triples
   ntpoint      = npoint  * nscan;    // Number radius points per triple
DbgLv(1) << "RD: allData size" << allData.size();

   // Ok to enable some buttons now
   enableControls();
}

// Load US3 AUC multi-wavelength data
void US_MwlRawViewer::load_auc_mwl( )
{
   int status = 0;
   resetAll();

   QString dir = "";
   US_MwlRun lddiag( dir, false );
   if ( lddiag.exec() == QDialog::Rejected )
      return;
   
   // Restore area beneath dialog
   qApp->processEvents();

   if ( dir.isEmpty() ) return; 
   
   dir.replace( "\\", "/" );                 // WIN32 issue
   if ( dir.right( 1 ) != "/" ) dir += "/";  // Ensure trailing '/'
   
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

   // Load the AUC data
   le_status->setText( tr( "Reading AUC Mwl data ..." ) );
   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor) );
   QDir dirdir( dir, "*", QDir::Name, QDir::Files | QDir::Readable );
   dirdir.makeAbsolute();
   if ( dir.right( 1 ) != "/" ) dir += "/"; // Ensure trailing /
   mwl_fnames = dirdir.entryList( QStringList( "*.auc" ),
                                  QDir::Files, QDir::Name );
   mwl_fnames.sort();
   ntriple     = mwl_fnames.size();
   runID       = new_runID;
   cellchans.clear();
   lambdas  .clear();
DbgLv(1) << "RD: mwr ntriple" << ntriple;

   for ( int ii = 0; ii < ntriple; ii++ )
   {
      QString mwrfname = mwl_fnames.at( ii );
      QString mwrfpath = currentDir + "/" + mwrfname;
      US_DataIO::RawData  rdata;

      US_DataIO::readRawData( mwrfpath, rdata );

      allData << rdata;

      QString celchn   = QString::number( rdata.cell ) + " / " +
                         QString( rdata.channel );
      int lambda       = qRound( rdata.scanData[ 0 ].wavelength );

      if ( ! cellchans.contains( celchn ) )
         cellchans << celchn;

      if ( ! lambdas.contains( lambda ) )
         lambdas   << lambda;

      le_status->setText( tr( "Data in for triple %1 of %2" )
                          .arg( ii + 1 ).arg( ntriple ) );
      qApp->processEvents();
   }

   radii.clear();
   radii << allData[ 0 ].xvalues;

   ncellch     = cellchans .size();
   nlambda     = lambdas   .size();
   nscan       = allData[ 0 ].scanCount();
   npoint      = allData[ 0 ].pointCount();
   ntpoint     = nscan * npoint;
DbgLv(1) << "RD: mwr ncellch nlambda nscan npoint"
 << ncellch << nlambda << nscan << npoint;
DbgLv(1) << "RD:   rvS rvE" << radii[0] << radii[npoint-1];
   le_status->setText( tr( "All %1 raw AUCs have been loaded." )
                       .arg( ntriple ) );
   QApplication::restoreOverrideCursor();
   QApplication::restoreOverrideCursor();
   qApp->processEvents();

   ct_from->setMaximum( nscan );
   ct_to  ->setMaximum( nscan );

   // Ok to enable some buttons now
   enableControls();
}

// Display detailed information about the data
void US_MwlRawViewer::runDetails( void )
{
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
   int    awvmd   = nlambda / 2;
   int    awvhi   = nlambda - 1;
   int    owvlo   = 0;
   int    owvhi   = nlambda - 1;
   double wvvmid  = lambdas[ nlambda / 2 ];
   int    owvmd   = lambdas.indexOf( wvvmid );
   int    rplo    = ( npoint * 5 ) / 100;
   int    rphi    = npoint - rplo;
   double rdata   = 0.0;
   int    wvx     = 0;
   int    rpx     = 0;
DbgLv(1) << "DDet: nlambda ntriple ntpoint" << nlambda << ntriple << ntpoint;
   le_status->setText( tr( "Computing data statistics..." ) );
   qApp->processEvents();
   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor) );

   // Accumulate statistics for original data
   for ( int ii = 0; ii < ntriple; ii++ )
   {
      wv1      = ( wvx == owvlo );
      wv2      = ( wvx == owvmd );
      wv3      = ( wvx == owvhi );
      rpx      = 0;
      int scx  = 0;

      for ( int jj = 0; jj < ntpoint; jj++ )
      {
         rdata    = allData[ ii ].reading( scx, rpx );
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
         if ( rpx >= npoint )
            rpx      = 0;
      }
      wvx++;
      if ( wvx >= nlambda )
         wvx      = 0;
   }
DbgLv(1) << "DDet:  ow3min ow3avg" << ow3min << ow3avg;

   // Accumulate statistics for averaged data
   double avsc    = 1.0;
   int    nha1    = navgrec / 2;
   int    nha2    = navgrec - nha1;
   wvx     = 0;
   rpx     = 0;

   for ( int ii = 0; ii < ntriple; ii++ )
   {
      wv1      = ( wvx == awvlo );
      wv2      = ( wvx == awvmd );
      wv3      = ( wvx == awvhi );
      rpx      = 0;
      int scx  = 0;
      int wvxs = wvx - nha1;
      int wvxe = wvx + nha2;
      int chof = ( ii / nlambda ) * nlambda;
      wvxs     = qMax( wvxs, 0 );
      wvxe     = qMin( wvxe, nlambda );
      wvxe     = qMax( wvxe, ( wvxs + 1 ) );
      wvxs    += chof;
      wvxe    += chof;
      avsc     = 1.0 / (double)( wvxe - wvxs );

      for ( int jj = 0; jj < ntpoint; jj++ )
      {
         rdata    = 0.0;
         for ( int kk = wvxs; kk < wvxe; kk++ )
            rdata   += allData[ kk ].reading( scx, rpx );
         rdata   *= avsc;
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
         if ( rpx >= npoint )
            rpx      = 0;
      }
      wvx++;
      if ( wvx >= nlambda )
         wvx      = 0;
   }
DbgLv(1) << "DDet:  aw3min aw3avg" << aw3min << aw3avg;
   le_status->setText( tr( "Data statistics have been computed." ) );
   qApp->processEvents();
   QApplication::restoreOverrideCursor();
   QApplication::restoreOverrideCursor();

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
   int    lx      = ntriple - 1;
DbgLv(1) << "DDet:  aw3knt aw3avg" << aw3knt << aw3avg << "lx" << lx;
   double s1tem   = allData[  0 ].scanData[ 0 ].temperature;
   double s1rot   = allData[  0 ].scanData[ 0 ].rpm;
   double s1omg   = allData[  0 ].scanData[ 0 ].omega2t;
   double s1etm   = allData[  0 ].scanData[ 0 ].seconds;
   double s2tem   = allData[ lx ].scanData[ 0 ].temperature;
   double s2rot   = allData[ lx ].scanData[ 0 ].rpm;
   double s2omg   = allData[ lx ].scanData[ 0 ].omega2t;
   double s2etm   = allData[ lx ].scanData[ 0 ].seconds;
   double owvmin  = lambdas[ 0 ];
   double owvmax  = lambdas[ nlambda - 1 ];
DbgLv(1) << "DDet:  owvmin owvmax" << owvmin << owvmax;
   double owvain  = ( owvmax - owvmin ) / (double)( nlambda - 1 );
   double awvain  = ( owvmax - owvmin ) / (double)( nlambda - 1 );
   double wavv1   = owvmin;
   double wavv2   = lambdas[ owvmd ];
   double wavv3   = owvmax;
DbgLv(1) << "DDet:  owvmd wavv2" << owvmd << wavv2;
   QString ffname = mwl_fnames[  0 ];
DbgLv(1) << "DDet:  ffname" << ffname;
   lx             = mwl_fnames.count() - 1;
   QString lfname = mwl_fnames[ lx ];
DbgLv(1) << "DDet:  lfname" << lfname;
   QString msg = tr( "Multi-Wavelength Statistics for RunID %1,\n" )
      .arg( runID );
   msg += tr( " from Directory %1\n\n" ).arg( currentDir );
   msg += tr( "General Data Set Values and Counts.\n" );
   msg += tr( "   First File Name:              %1\n" ).arg( ffname ); 
   msg += tr( "   Last File Name:               %1\n" ).arg( lfname ); 
   msg += tr( "   Scans:                        %1\n" ).arg( nscan );
   msg += tr( "   Radius Data Points:           %1\n" ).arg( npoint );
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
   msg += tr( "   Count of Wavelengths:         %1\n" ).arg( nlambda );
   msg += tr( "   Minimum:                      %1\n" ).arg( owvmin );
   msg += tr( "   Maximum:                      %1\n" ).arg( owvmax );
   msg += tr( "   Average Increment:            %1\n" ).arg( owvain );
   msg += tr( "%1-point Averaged Data Wavelengths.\n" ).arg( navgrec );
   msg += tr( "   Count of Wavelengths:         %1\n" ).arg( klambda );
   msg += tr( "   Minimum:                      %1\n" ).arg( owvmin );
   msg += tr( "   Maximum:                      %1\n" ).arg( owvmax );
   msg += tr( "   Average Increment:            %1\n" ).arg( awvain );
   msg += tr( "\nOriginal Full Intensity Data.\n" );
   msg += tr( "   Minimum:                      %1\n" ).arg( ofdmin );
   msg += tr( "   Maximum:                      %1\n" ).arg( ofdmax );
   msg += tr( "   Average:                      %1\n" ).arg( ofdavg );
   msg += tr( "%1-point Averaged Full Intensity Data.\n" ).arg( navgrec );
   msg += tr( "   Minimum:                      %1\n" ).arg( afdmin );
   msg += tr( "   Maximum:                      %1\n" ).arg( afdmax );
   msg += tr( "   Average:                      %1\n" ).arg( afdavg );
   msg += tr( "Original Middle 90% Intensity Data.\n" );
   msg += tr( "   Minimum:                      %1\n" ).arg( o90min );
   msg += tr( "   Maximum:                      %1\n" ).arg( o90max );
   msg += tr( "   Average:                      %1\n" ).arg( o90avg );
   msg += tr( "%1-point Averaged Middle 90% Intensity Data.\n" ).arg( navgrec );
   msg += tr( "   Minimum:                      %1\n" ).arg( a90min );
   msg += tr( "   Maximum:                      %1\n" ).arg( a90max );
   msg += tr( "   Average:                      %1\n" ).arg( a90avg );
   msg += tr( "%1 nm Original Middle 90% Intensity Data.\n" )
      .arg( wavv1 );
   msg += tr( "   Minimum:                      %1\n" ).arg( ow1min );
   msg += tr( "   Maximum:                      %1\n" ).arg( ow1max );
   msg += tr( "   Average:                      %1\n" ).arg( ow1avg );
   msg += tr( "%1 nm %2-point Averaged Middle 90% Intensity Data.\n" )
      .arg( wavv1 ).arg( navgrec );
   msg += tr( "   Minimum:                      %1\n" ).arg( aw1min );
   msg += tr( "   Maximum:                      %1\n" ).arg( aw1max );
   msg += tr( "   Average:                      %1\n" ).arg( aw1avg );
   msg += tr( "%1 nm Original Middle 90% Intensity Data.\n" )
      .arg( wavv2 );
   msg += tr( "   Minimum:                      %1\n" ).arg( ow2min );
   msg += tr( "   Maximum:                      %1\n" ).arg( ow2max );
   msg += tr( "   Average:                      %1\n" ).arg( ow2avg );
   msg += tr( "%1 nm %2-point Averaged Middle 90% Intensity Data.\n" )
      .arg( wavv2 ).arg( navgrec );
   msg += tr( "   Minimum:                      %1\n" ).arg( aw2min );
   msg += tr( "   Maximum:                      %1\n" ).arg( aw2max );
   msg += tr( "   Average:                      %1\n" ).arg( aw2avg );
   msg += tr( "%1 nm Original Middle 90% Intensity Data.\n" )
      .arg( wavv3 );
   msg += tr( "   Minimum:                      %1\n" ).arg( ow3min );
   msg += tr( "   Maximum:                      %1\n" ).arg( ow3max );
   msg += tr( "   Average:                      %1\n" ).arg( ow3avg );
   msg += tr( "%1 nm %2-point Averaged Middle 90% Intensity Data.\n" )
      .arg( wavv3 ).arg( navgrec );
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
   // Match the description to find the correct triple in memory
   QString cellch = cb_cellchn->currentText();
DbgLv(1) << "chgCC:  cellch" << cellch << "last_xmin" << last_xmin;
   if ( allData.size() < 1 )
      return;

   if ( last_xmin < 0.0 )
   {  // If first time plotting any data, detect actual data bounds
      have_rngs     = false;
      compute_ranges();

      int trxs      = trpxs + lmbxs;
      int trxe      = trpxs + lmbxe;

      if ( is_wrecs )
      {  // For wavelength records, X bounds are radius bounds
         last_xmin     = allData[ trxs ].xvalues[ radxs ];
         last_xmax     = last_xmin;
         US_DataIO::RawData* edata = &allData[ trxs ];

         for ( int rdx = radxs; rdx < radxe; rdx++ )
         {
            last_xmin  = qMin( last_xmin, edata->xvalues[ rdx ] );
            last_xmax  = qMax( last_xmax, edata->xvalues[ rdx ] );
         }
      }

      else
      {  // For radius records, X bounds are lambda bounds
         last_xmin     = lambdas[ lmbxs ];
         last_xmax     = lambdas[ lmbxe ];
      }

      last_ymin     = allData[ trxs ].reading( 0, 0 );
      last_ymax     = last_ymin;

      for ( int trx = trxs; trx < trxe; trx++ )
      {  // Accumulate Y bounds, the amplitude bounds
         US_DataIO::RawData* edata = &allData[ trx ];

         for ( int scx = 0; scx < nscan; scx++ )
         {
            for ( int rdx = radxs; rdx < radxe; rdx++ )
            {
               last_ymin  = qMin( last_ymin, edata->reading( scx, rdx ) );
               last_ymax  = qMax( last_ymax, edata->reading( scx, rdx ) );
            }
         }
      }
DbgLv(1) << "chgCC: trxs trxe" << trxs << trxe;
   }

   else
   {  // After first time, detect what has been already set
#if QT_VERSION < 0x050000
      QwtScaleDiv* sdx = data_plot->axisScaleDiv( QwtPlot::xBottom );
      QwtScaleDiv* sdy = data_plot->axisScaleDiv( QwtPlot::yLeft   );
#else
      QwtScaleDiv* sdx = (QwtScaleDiv*)&data_plot->axisScaleDiv( QwtPlot::xBottom );
      QwtScaleDiv* sdy = (QwtScaleDiv*)&data_plot->axisScaleDiv( QwtPlot::yLeft   );
#endif
      last_xmin      = sdx->lowerBound();
      last_xmax      = sdx->upperBound();
      last_ymin      = sdy->lowerBound();
      last_ymax      = sdy->upperBound();
   }
DbgLv(1) << "chgCC:  xmin xmax ymin ymax"
 << last_xmin << last_xmax << last_ymin << last_ymax;

   curr_cdata.clear();
   prev_cdata.clear();
   curr_recxs.clear();
   prev_recxs.clear();

   have_rngs  = false;
   changeRecord();
}

// Plot the current data record
void US_MwlRawViewer::plot_current( void )
{
   if ( allData.size() == 0 )
      return;

   plot_titles();     // Set the titles

   plot_all();        // Plot the data
}

// Compose plot titles for the current record
void US_MwlRawViewer::plot_titles( void )
{
   QString cellch   = cb_cellchn ->currentText();
   QString cell     = cellch.section( "/", 0, 0 ).simplified();
   QString chan     = cellch.section( "/", 1, 1 ).simplified();
   QString prec     = cb_pltrec->currentText();

   // Plot Title and legends
   QString title    = "Radial Intensity Data\nRun ID: " + runID +
                      "\n    Cell: " + cell + "  Channel: " + chan;
   QString xLegend;
   QString yLegend  = "Radial Intensity at " + prec;

   if ( is_wrecs )
   {
      title           += QString( "  Wavelength: " ) + prec;
      xLegend          = QString( "Radius (in cm)" );
      yLegend         += QString( " nm" );
   }

   else
   {
      title           += QString( "  Radius: " ) + prec;
      xLegend          = QString( "Wavelength (in nm)" );
      yLegend         += QString( " cm" );
   }

   data_plot->setTitle( title );
   data_plot->setAxisTitle( QwtPlot::yLeft,   yLegend );
   data_plot->setAxisTitle( QwtPlot::xBottom, xLegend );

}

// Draw scan curves for the current plot record
void US_MwlRawViewer::plot_all( void )
{
   dataPlotClear( data_plot );
   grid           = us_grid( data_plot );

   // Make sure ranges are set up, then build an averaged data vector
   compute_ranges();

   build_avg_data();

DbgLv(1) << "PltA: kpoint" << kpoint << "datsize" << curr_adata.size();
   // Build the X,Y vectors
   QVector< double > rvec( kpoint );
   QVector< double > vvec( kpoint );
   double* rr     = rvec.data();
   double* vv     = vvec.data();

   int     scan_from = (int)ct_from->value();
   int     scan_to   = (int)ct_to  ->value();
   int     scan_nbr  = 0;
   QPen    pen_red ( Qt::red );
   QPen    pen_plot( US_GuiSettings::plotCurve() );
   int     rdx       = 0;

   for ( int ptx = 0; ptx < kpoint; ptx++ )   // One-time build of X vector
      rr[ ptx ]       = pltxvals[ ptx ];

   for ( int scnx = 0; scnx < nscan; scnx++ ) // Build Y vector for each scan
   {
      if ( excludes.contains( scnx ) )  continue;

      for ( int ptx = 0; ptx < kpoint; ptx++ )
      {
         vv[ ptx ]       = curr_adata[ rdx++ ];
      }

      scan_nbr++;
      QString       title = tr( "Raw Data at scan " )
                            + QString::number( scan_nbr );
      QwtPlotCurve* curv  = us_curve( data_plot, title );

      if ( scan_nbr > scan_to  ||  scan_nbr < scan_from )
         curv->setPen( pen_plot );            // Normal pen
      else
         curv->setPen( pen_red  );            // Scan-focus pen

      curv->setSamples( rr, vv, kpoint );     // Build a scan curve
//DbgLv(1) << "PltA:   scx" << scx << "rr0 vv0 rrn vvn"
// << rr[0] << rr[kpoint-1] << vv[0] << vv[kpoint-1];
   }

DbgLv(1) << "PltA: last_xmin" << last_xmin;
   if ( last_xmin < 0.0 )
   {  // If first time, use auto scale to set plot ranges
      data_plot->setAxisAutoScale( QwtPlot::yLeft   );
      data_plot->setAxisAutoScale( QwtPlot::xBottom );
   }

   else
   {  // After first time, use the same plot ranges as set before
      data_plot->setAxisScale( QwtPlot::xBottom, last_xmin, last_xmax );
      data_plot->setAxisScale( QwtPlot::yLeft  , last_ymin, last_ymax );
   }

   // Draw the plot
   data_plot->replot();

   // Pick up the actual bounds plotted (including any Config changes)
#if QT_VERSION < 0x050000
   QwtScaleDiv* sdx = data_plot->axisScaleDiv( QwtPlot::xBottom );
   QwtScaleDiv* sdy = data_plot->axisScaleDiv( QwtPlot::yLeft   );
#else
   QwtScaleDiv* sdx = (QwtScaleDiv*)&data_plot->axisScaleDiv( QwtPlot::xBottom );
   QwtScaleDiv* sdy = (QwtScaleDiv*)&data_plot->axisScaleDiv( QwtPlot::yLeft   );
#endif
   last_xmin      = sdx->lowerBound();
   last_xmax      = sdx->upperBound();
   last_ymin      = sdy->lowerBound();
   last_ymax      = sdy->upperBound();
DbgLv(1) << "PltA: xlo xhi" << last_xmin << last_xmax
 << "ylo yhi" << last_ymin << last_ymax;
}

// Slot to handle a change in start or end radius
void US_MwlRawViewer::changeRadius()
{
DbgLv(1) << "chgRadius";
   have_rngs  = false;
   compute_ranges();                        // Recompute ranges

   if ( !is_wrecs )
   {  // For radius records, we need to reset the plot record list
      cb_pltrec->disconnect();
      cb_pltrec->clear();

      for ( int rdx = radxs; rdx < radxe; rdx++ )
      {  // Build the list of radii that are within range
         QString citem    = QString().sprintf( "%.3f", radii[ rdx ] );

         cb_pltrec->addItem( citem );
      }

      connect( cb_pltrec,    SIGNAL( currentIndexChanged( int ) ),
               this,         SLOT  ( changeRecord( )            ) );

      recx           = ( radxe - radxs ) / 2;
      cb_pltrec->setCurrentIndex( recx );
   }

   build_avg_data();                        // Rebuild initial averaged data
}

// Slot to handle a change in start or end lambda
void US_MwlRawViewer::changeLambda()
{
DbgLv(1) << "chgLambda";
   have_rngs  = false;
   compute_ranges();                        // Recompute ranges

   if ( is_wrecs )
   {  // For wavelength records, we need to reset the plot record list
      cb_pltrec->disconnect();
      cb_pltrec->clear();

      for ( int wvx = lmbxs; wvx < lmbxe; wvx++ )
      {  // Build the list of wavelengths that are within range
         QString citem    = QString::number( lambdas[ wvx ] );

         cb_pltrec->addItem( citem );
      }

      connect( cb_pltrec,    SIGNAL( currentIndexChanged( int ) ),
               this,         SLOT  ( changeRecord( )            ) );

      recx           = ( lmbxe - lmbxs ) / 2;
      cb_pltrec->setCurrentIndex( recx );
   }

   build_avg_data();                        // Rebuild initial averaged data
}
 
// Slot to handle a change in the plot record
void US_MwlRawViewer::changeRecord( void )
{
   recx           = cb_pltrec->currentIndex();
DbgLv(1) << "chgRec: recx" << recx;
   bool plt_one   = ! le_status->text().contains( tr( "saving" ) );

   // Insure we have averaged data for this record
   build_avg_data();

   // Plot what we have
   plot_current();

   // Update status text (if not part of movie save) and set prev/next arrows
   if ( plt_one )
      le_status->setText( lb_pltrec->text() + ": " + cb_pltrec->currentText() );
   pb_prev  ->setEnabled( ( recx > 0 ) );
   pb_next  ->setEnabled( ( recx < ( krecs -1 ) ) );
}

// Slot to handle a change in the plot record type
void US_MwlRawViewer::changeRectype( bool wlnrec )
{
DbgLv(1) << "chgRtype: wlnrec" << wlnrec;
   have_rngs  = false;
   is_wrecs   = ! wlnrec;

   compute_ranges();                        // Recompute ranges for new type
   prev_recxs.clear();                      // Reset averaging components
   prev_cdata.clear();
   changeCellCh();                          // Force a plot initialize

   if ( is_wrecs )
   {  // Change record button title and reset for lambda record
      lb_pltrec->setText( tr( "Plot (W nm)" ) );
      changeLambda();                       // Change to lambda records
      last_xmin  = rad_start;               // Change X limits
      last_xmax  = rad_end;
   }
   else
   {  // Change record button title and reset for radius record
      lb_pltrec->setText( tr( "Plot (R cm)" ) );
      changeRadius();                       // Change to radius records
      last_xmin  = (double)lmb_start;       // Change X limits
      last_xmax  = (double)lmb_end;
   }

   // Since X has changed, reset the plot X scale
   data_plot->setAxisScale( QwtPlot::xBottom, last_xmin, last_xmax );

   plot_current();                          // Plot initial record of this type
}

// Slot to handle a click to go to the previous record
void US_MwlRawViewer::prevPlot( void )
{
   int pltrx      = cb_pltrec->currentIndex() - 1;

   if ( pltrx < 1 )
   {
      pltrx          = 0;
      pb_prev->setEnabled( false );
   }

#if QT_VERSION < 0x050000
   QwtScaleDiv* sdx = data_plot->axisScaleDiv( QwtPlot::xBottom );
   QwtScaleDiv* sdy = data_plot->axisScaleDiv( QwtPlot::yLeft   );
#else
   QwtScaleDiv* sdx = (QwtScaleDiv*)&data_plot->axisScaleDiv( QwtPlot::xBottom );
   QwtScaleDiv* sdy = (QwtScaleDiv*)&data_plot->axisScaleDiv( QwtPlot::yLeft   );
#endif
   last_xmin      = sdx->lowerBound();
   last_xmax      = sdx->upperBound();
   last_ymin      = sdy->lowerBound();
   last_ymax      = sdy->upperBound();

   cb_pltrec->setCurrentIndex( pltrx );
}

// Slot to handle a click to go to the next record
void US_MwlRawViewer::nextPlot( void )
{
   int pltrx      = cb_pltrec->currentIndex() + 1;
   int nitems     = cb_pltrec->count();

   if ( ( pltrx + 2 ) > nitems )
   {
      pltrx          = nitems - 1;
      pb_next->setEnabled( false );
   }

#if QT_VERSION < 0x050000
   QwtScaleDiv* sdx = data_plot->axisScaleDiv( QwtPlot::xBottom );
   QwtScaleDiv* sdy = data_plot->axisScaleDiv( QwtPlot::yLeft   );
#else
   QwtScaleDiv* sdx = (QwtScaleDiv*)&data_plot->axisScaleDiv( QwtPlot::xBottom );
   QwtScaleDiv* sdy = (QwtScaleDiv*)&data_plot->axisScaleDiv( QwtPlot::yLeft   );
#endif
   last_xmin      = sdx->lowerBound();
   last_xmax      = sdx->upperBound();
   last_ymin      = sdy->lowerBound();
   last_ymax      = sdy->upperBound();

   cb_pltrec->setCurrentIndex( pltrx );
}

// Slot to handle a change in the number of averaging points
void US_MwlRawViewer::changeAverage()
{
DbgLv(1) << "chgAvg:";
   prev_recxs.clear();
   prev_cdata.clear();
   changeRecord();
}

// Build the component data vectors for averaging.
//  The vector of data record readings vectors spans a window of records
//  around the current record. Its size is usually the number of averaging
//  points, but is less at either end of the records range.
void US_MwlRawViewer::build_cmp_data()
{
   compute_ranges();                    // Insure we have current plot ranges

   QVector< double > fillv;
   curr_recxs.clear();
   curr_cdata.clear();
   navgrec       = ct_recavg->value();  // Usual number of averaging records
   int kavgc     = navgrec;             // Actual count
   int kavgh     = kavgc / 2;           // Half count
   int arxs      = recx - kavgh;        // Record start index
   int arxe      = recx + kavgh;        // Record end index
   arxs          = qMax( arxs, 0 );                // Start at least 0
   arxe          = qMin( arxe, krecs - 1 ) + 1;    // End at most range count
   arxe          = qMax( arxe, ( arxs + 1 ) );
   kavgc         = arxe - arxs;         // Actual averaging count
   curr_cdata.fill( fillv, kavgc );     // Initialize component datas
DbgLv(1) << "BldC: kavgc" << kavgc << "arxs arxe" << arxs << arxe;
   int jrx       = 0;

   for ( int krx = arxs; krx < arxe; krx++, jrx++ )
   {  // Build or move component records
      int prx       = prev_recxs.indexOf( krx );
DbgLv(1) << "BldC:   krx" << krx << "prx" << prx;

      if ( prx < 0 )
      {  // Previous components missing this one, so build a new one
         build_rec_data( krx, curr_cdata[ jrx ] );
      }
      else
      {  // Previous components had this one, so just copy to new location
         curr_cdata[ jrx ] = prev_cdata[ prx ];
      }

      curr_recxs << krx;                // Keep components indexes
   }

   prev_recxs = curr_recxs;             // Preserve indexes for next time
   prev_cdata = curr_cdata;             // Preserve data for next time

}

// Build the averaged data vector for the current plot record
//   Detect the actual number of component records, sum at each data point,
//   then obtain the averaged point.
void US_MwlRawViewer::build_avg_data()
{
   build_cmp_data();                     // Build components for averaging

   int kavgc     = curr_recxs.size();    // Actual count of components
   double avgscl = 1.0 / (double)kavgc;  // Averaging scale factor
DbgLv(1) << "BldA: kavgc" << kavgc << "ktpoint" << ktpoint;
DbgLv(1) << "BldA:  cd size" << curr_cdata.size();
DbgLv(1) << "BldA:  cd0 size" << curr_cdata[0].size();
int nn=curr_cdata.size()-1;
DbgLv(1) << "BldA:  cdn size" << curr_cdata[nn].size();

   curr_adata.clear();                   // Initial averaged data

   for ( int ptx = 0; ptx < ktpoint; ptx++ )
   {  // Compute the average at each data point
      double dsum   = 0.0;

      for ( int rcx = 0; rcx < kavgc; rcx++ )
      {  // Build a data point sum across all components
         dsum       += curr_cdata[ rcx ][ ptx ];
if(ptx==(ktpoint/2))
DbgLv(1) << "BldA:   ptx rcx" << ptx << rcx << "cdat dsum"
 << curr_cdata[rcx][ptx] << dsum;
      }

      curr_adata << ( dsum * avgscl );   // Store the average data point
   }
DbgLv(1) << "BldA:  curr_adata size" << curr_adata.size();
}

// Build a record data vector.
//   This routine is called when necessary to build an averaging component
//   at a specified record. The output flat vector of doubles has successive
//   data points across the current X range, for non-excluded scans.
void US_MwlRawViewer::build_rec_data( const int recx,
                                      QVector< double >& rdata )
{
   compute_ranges();                     // Compute record and data ranges

   rdata.clear();                        // Initialize the data vector
   kscan      = 0;                       // Count of actual produced scans

DbgLv(1) << "BldR:     recx" << recx;
   if ( is_wrecs )
   {  // Build data values for wavelength record with x-axis radius
      krecs      = klambda;              // Count of wavelength records
      kpoint     = kradii;               // Count of radius points per scan
      int trpx   = trpxs + lmbxs + recx; // Triple index of record
DbgLv(1) << "BldR:      trx rxs rxe nscan" << trpx << radxs << radxe << nscan;

      for ( int scnx = 0; scnx < nscan; scnx++ )
      {  // Examine all scans, but only store for included ones
         if ( excludes.contains( scnx ) )  continue;

         for ( int radx = radxs; radx < radxe; radx++ )
         {  // Save a data point for each radius (input=triple is constant)
            rdata << allData[ trpx ].reading( scnx, radx );
         }

         kscan++;                        // Bump count of produced scans
      }
DbgLv(1) << "BldR:      kscan" << kscan << "rd size" << rdata.size();
   }

   else
   {  // Build data values for radius record with x-axis wavelength
      krecs      = kradii;               // Count of radius records
      kpoint     = klambda;              // Count of wavelength points per scan
      int wavxs  = trpxs + lmbxs;        // Start wavelength index in triples
      int wavxe  = trpxs + lmbxe;        // End wavelength index in triples

      for ( int scnx = 0; scnx < nscan; scnx++ )
      {  // Examine all scans, but only store for included ones
         if ( excludes.contains( scnx ) )  continue;

         for ( int wavx = wavxs; wavx < wavxe; wavx++ )
         {  // Save a data point for each wavelength (radius=record is constant)
            rdata << allData[ wavx ].reading( scnx, recx );
         }

         kscan++;                        // Bump count of produced scans
      }
   }

   ktpoint    = kscan * kpoint;          // Count of total data points
}

// Compute the plot range indexes implied by current settings
void US_MwlRawViewer::compute_ranges()
{
   if ( have_rngs )         // If we just did this computation, return now
      return;

   ccx        = cb_cellchn->currentIndex();            // Cell/Channel index
   rad_start  = cb_rstart ->currentText().toDouble();  // Radius start
   rad_end    = cb_rend   ->currentText().toDouble();  // Radius end
   lmb_start  = cb_lstart ->currentText().toInt();     // Lambda start
   lmb_end    = cb_lend   ->currentText().toInt();     // Lambda end
   recx       = cb_pltrec ->currentIndex();            // Plot record index
   lmbxs      = lambdas.indexOf( lmb_start );          // Lambda start index
   lmbxe      = lambdas.indexOf( lmb_end   ) + 1;      // Lambda end index
   radxs      = dvec_index( radii, rad_start );        // Radius start index
   radxe      = dvec_index( radii, rad_end   ) + 1;    // Radius end index
DbgLv(1) << "cmpR:  rS rE rxS rxE" << rad_start << rad_end << radxs << radxe;
DbgLv(1) << "cmpR:   rvS rvE" << radii[radxs] << radii[radxe-1];
   klambda    = lmbxe - lmbxs;                         // Count of plot lambdas
   kradii     = radxe - radxs;                         // Count of plot radii
   kscan      = nscan - excludes.size();               // Count included scans
   trpxs      = ccx * nlambda;                         // Start triple index
   pltxvals.clear();
   if ( is_wrecs )
   {
      krecs      = klambda;                            // Count of plot recs 
      kpoint     = kradii;                             // Count of plot points
      for ( int jj = radxs; jj < radxe; jj++ )
         pltxvals << radii[ jj ];                      // Plot X values
int jx=pltxvals.count()-1;
DbgLv(1) << "cmpR:  pxS pxE" << pltxvals[0] << pltxvals[jx];
   }
   else
   {
      krecs      = kradii;                             // Count of plot recs 
      kpoint     = klambda;                            // Count of plot points
      for ( int jj = lmbxs; jj < lmbxe; jj++ )
         pltxvals << (double)lambdas[ jj ];            // Plot X values
   }
   ktpoint    = kscan * kpoint;                        // Total plot data points
   have_rngs  = true;                                  // Mark ranges computed
DbgLv(1) << "cmpR:  is_wrecs" << is_wrecs << "kpoint" << kpoint;
}

// Connect or Disconnect plot-range related controls
void US_MwlRawViewer::connect_ranges( bool conn )
{
   if ( conn )
   {  // Connect the range-related controls
      connect( cb_cellchn, SIGNAL( currentIndexChanged( int ) ),
               this,       SLOT  ( changeCellCh(            ) ) );
      connect( cb_rstart,  SIGNAL( currentIndexChanged( int ) ),
               this,       SLOT  ( changeRadius(            ) ) );
      connect( cb_rend,    SIGNAL( currentIndexChanged( int ) ),
               this,       SLOT  ( changeRadius(            ) ) );
      connect( cb_lstart,  SIGNAL( currentIndexChanged( int ) ),
               this,       SLOT  ( changeLambda(            ) ) );
      connect( cb_lend,    SIGNAL( currentIndexChanged( int ) ),
               this,       SLOT  ( changeLambda(            ) ) );
      connect( cb_pltrec,  SIGNAL( currentIndexChanged( int ) ),
               this,       SLOT  ( changeRecord(            ) ) );
   }

   else
   {  // Disconnect the range-related controls
      cb_cellchn->disconnect();
      cb_rstart ->disconnect();
      cb_rend   ->disconnect();
      cb_lstart ->disconnect();
      cb_lend   ->disconnect();
      cb_pltrec ->disconnect();
   }
}

// Slot to handle a change in scan exclude "from" value
void US_MwlRawViewer::exclude_from( double sfr )
{
   int scan_from  = (int)sfr;
   int scan_to    = (int)ct_to  ->value();

   if ( scan_to < scan_from )
   {
      ct_to  ->disconnect();
      ct_to  ->setValue( scan_from );

      connect( ct_to,        SIGNAL( valueChanged( double ) ),
               this,         SLOT  ( exclude_to  ( double ) ) );
   }

   plot_current();
}

// Slot to handle a change in scan exclude "to" value
void US_MwlRawViewer::exclude_to( double sto )
{
   int scan_to    = (int)sto;
   int scan_from  = (int)ct_from->value();

   if ( scan_from > scan_to )
   {
      ct_from->disconnect();
      ct_from->setValue( scan_to );

      connect( ct_from,      SIGNAL( valueChanged( double ) ),
               this,         SLOT  ( exclude_from( double ) ) );
   }

   plot_current();
}

// Slot to handle click of Exclude Scan Range
void US_MwlRawViewer::exclude_scans()
{
   int scan_from  = (int)ct_from->value();
   int scan_to    = (int)ct_to  ->value();
   int scan_knt   = 1;

   for ( int scnx = 0; scnx < nscan; scnx++ )
   {
      if ( excludes.contains( scnx ) )  continue;

      if ( scan_knt >= scan_from  &&  scan_knt <= scan_to )
         excludes << scnx;

      scan_knt++;
   }

   qSort( excludes );
   curr_cdata.clear();
   prev_cdata.clear();
   curr_recxs.clear();
   prev_recxs.clear();
   kscan      = nscan - excludes.count();
DbgLv(1) << "Excl: kscan" << kscan;
   ct_from   ->disconnect();
   ct_to     ->disconnect();
   ct_from   ->setMaximum( kscan );
   ct_to     ->setMaximum( kscan );
   connect( ct_from,      SIGNAL( valueChanged( double ) ),
            this,         SLOT  ( exclude_from( double ) ) );
   connect( ct_to,        SIGNAL( valueChanged( double ) ),
            this,         SLOT  ( exclude_to  ( double ) ) );
   ct_to     ->setValue( 0 );
   pb_include->setEnabled( true );
}

// Slot to handle click of Include All (restore of all scans)
void US_MwlRawViewer::include_scans()
{
   excludes.clear();

   kscan      = nscan;
   ktpoint    = kscan * kpoint;
DbgLv(1) << "Incl: nscan" << nscan << "kscn ecnt" << kscan << excludes.count();
   curr_cdata.clear();                 // Reset averaging data
   prev_cdata.clear();
   curr_recxs.clear();
   prev_recxs.clear();
   ct_to     ->setValue( 0 );
   changeRecord();                     // Force replot

   ct_from   ->disconnect();
   ct_to     ->disconnect();
   ct_from   ->setMaximum( kscan );
   ct_to     ->setMaximum( kscan );
   connect( ct_from,      SIGNAL( valueChanged( double ) ),
            this,         SLOT  ( exclude_from( double ) ) );
   connect( ct_to,        SIGNAL( valueChanged( double ) ),
            this,         SLOT  ( exclude_to  ( double ) ) );
   ct_to     ->setValue( 0 );
   pb_include->setEnabled( false );
}

// Slot to show a 2-D movie
void US_MwlRawViewer::show_2d_movie()
{
DbgLv(1) << "Show 2D Movie";
   // Loop to plot each record in the current cell
   int krecs       = cb_pltrec->count();
   int svrec       = recx;                  // Save currently plotted record
   changeCellCh();                          // Force save of scales

   for ( int prx = 0; prx < krecs; prx++ )
   {
      cb_pltrec->setCurrentIndex( prx );    // Plot each record in the range
      qApp->processEvents();
   }

   cb_pltrec->setCurrentIndex( svrec );     // Restore previous plot record
   qApp->processEvents();
}

// Slot to open a dialog for 3-D plotting
void US_MwlRawViewer::plot_3d()
{
DbgLv(1) << "Plt3D";
   build_xyz_data( xyzdat );

DbgLv(1) << "Plt3D:  open MPC";
   if ( p3d_ctld == NULL )
   {
      p3d_pltw     = NULL;
      p3d_ctld     = new US_MwlPlotControl( this, &xyzdat );
      p3d_ctld->show();
      // Position near the upper right corner of the desktop
//      int cx       = qApp->desktop()->width() - p3d_ctld->width() - 40;
      int cx       = 40;
      int cy       = 40;
      p3d_ctld->move( cx, cy );
      connect( p3d_ctld, SIGNAL( has_closed()     ),
               this,     SLOT  ( p3dctrl_closed() ) );
   }

   else
   {
      p3d_ctld->setFocus();
      p3d_ctld->do_3dplot();

      p3d_pltw     = p3d_ctld->widget_3dplot();

      if ( p3d_pltw != NULL )
      {
         p3d_pltw->reloadData( &xyzdat );

         int scan_nbr   = live_scan( );
         QString ptitle = tr( "MWL 3-D Plot, Scan " )
                        + QString::number( scan_nbr );

         p3d_pltw->setPlotTitle( ptitle );
         p3d_pltw->replot();
      }
   }

   pb_movie3d->setEnabled( true );
}

// Slot to show a 3-D movie
void US_MwlRawViewer::show_3d_movie()
{
   bool hold_color = ck_hcolorc->isChecked();

//DbgLv(1) << "sh3M: Show 3D Movie";
   if ( p3d_ctld == NULL )
   {  // This should not happen, but disallow Show 3D Movie if no plot opened
      return;
   }

   p3d_pltw     = p3d_ctld->widget_3dplot();    // Main 3D plot window

   if ( p3d_pltw == NULL )
   {  // If the dialog is opened
      QMessageBox::warning( this,
            tr( "3-D Plot Window Not Opened" ),
            tr( "You cannot start a 3-D movie until you have first"
                " opened the small plot control dialog by clicking"
                " on the \"Plot 3D\" button in this program's main"
                " window. From that control dialog you must click"
                " on the \"3D Plot\" button to open the 3-D plotting"
                " window.\n\nYou should insure that scale and"
                " orientation are correct for the current plot, as"
                " these will be in force for each movie frame."
                " You may then again click on \"Show 3D Movie\"." ) );
      return;
   }

   // Determine the range of scans that will be in the movie
   int fscnx    = -1;
   int lscnx    = -1;
   int krscan   = 0;

   live_scan( &fscnx, &lscnx, &krscan );
DbgLv(1) << "sh3M:  fscnx lscnx krscan" << fscnx << lscnx << krscan;

   QString statmsg  = ( nscan == krscan )
                    ? tr( "Of %1 scans, showing:   Scan " ).arg( nscan )
                    : tr( "Of %1 in-range, included scans (%2 total),"
                          " showing:   Scan " ).arg( krscan ).arg( nscan );
   QString ptbase   = tr( "MWL 3-D Plot, Scan SSS" );
   QString ptitle   = tr( "MWL 3-D Plot, Scan 1" );
   QString str_scan;

   // Loop to show movie frames for each included scan that is in range
   for ( int scnx = fscnx; scnx <= lscnx; scnx++ )
   {
      if ( excludes.contains( scnx ) )  continue;

      build_xyz_data( xyzdat, scnx );    // Build data for this scan

      p3d_pltw->reloadData( &xyzdat );   // Load the data in the plot window
//      p3d_ctld->do_3dplot();             // Do the plot
      str_scan         = QString::number( ( scnx + 1 ) );
      ptitle           = QString( ptbase ).replace( "SSS", str_scan );
      p3d_pltw->setPlotTitle( ptitle );  // Reset the title for scan_nbr
      p3d_pltw->replot( hold_color );    // Do the plot
DbgLv(1) << "sh3M:    scnx ptitle" << scnx << ptitle;

      le_status->setText( statmsg + str_scan );
      qApp->processEvents();
   }
}

// Slot to save the current plot
void US_MwlRawViewer::save_plot()
{
DbgLv(1) << "Save Plot";
   QString savedir = US_Settings::reportDir() + "/" + runID;
   QDir().mkpath( savedir );
   savedir         = savedir.replace( "\\", "/" ) + "/";
   QString fname2d = runID + ( ck_xwavlen->isChecked()
                   ? ".radRec_RRRRR_2D.png"
                   : ".lmbRec_RRRRR_2D.png" );
   QString fname3d = runID + ".Scan_SSSS_3D.png";
   p3d_pltw        = ( p3d_ctld == NULL ) ? NULL : p3d_ctld->widget_3dplot();
   int nfiles      = ( p3d_pltw != NULL ) ? 2 : 1;

   if ( nfiles == 2 )
   {  // If there is a 3D window, first save a PNG of that window
      int scan_fr     = (int)ct_from->value();
      int scan_to     = (int)ct_to  ->value();
      scan_fr         = ( scan_to < 1 ) ?     1 : scan_fr;
      int scan_knt    = 0;
      int scan_nbr    = 1;

      for ( int scnx = 0; scnx < nscan; scnx++ )
      {
         if ( excludes.contains( scnx ) )   continue;
         scan_knt++;            // Non-excluded count
         if ( scan_knt < scan_fr )          continue;
         scan_nbr        = scnx + 1;
         break;
      }

      p3d_pltw->replot();                // Do the plot
      QString s_scan  = QString().sprintf( "%04d", scan_nbr );
      fname3d         = fname3d.replace( "SSSS", s_scan  );
      QString fpath3d = savedir + fname3d;

      p3d_pltw->save_plot( fpath3d, QString( "png" ) );
   }

   // Always save a PNG of the 2-D plot
   QString ccr     = cb_cellchn->currentText().remove( " / " );
   QString rec_str = ccr + cb_pltrec->currentText().remove( "." );
   fname2d         = fname2d.replace( "RRRRR", rec_str );
   QString fpath2d = savedir + fname2d;

   US_GuiUtil::save_png( fpath2d, data_plot );

   // Report the file(s) saved
   QString mtitle  = ( nfiles == 1 )
                   ? tr( "Plot File Saved" )
                   : tr( "Plot Files Saved" );
   QString msg     = tr( "In the directory\n     %1,\n\n" ).arg( savedir );
   if ( nfiles == 1 )
      msg            += tr( "File\n    %1 was saved." ).arg( fname2d );
   else
      msg            += tr( "Files\n    %1 ;  and\n    %2\nwere saved." )
                        .arg( fname3d ).arg( fname2d );

   QMessageBox::information( this, mtitle, msg );
}

// Slot to save the current cell's movie(s)
void US_MwlRawViewer::save_movie()
{
   bool save_2d   = true;
   bool save_3d   = ( p3d_pltw != NULL );

DbgLv(1) << "Save Movie";
   if ( save_3d )
   {  // If there is a 3D window, ask the user which movies to run
      QMessageBox msgBox( this );
      msgBox.setWindowTitle( tr( "Multiple Possible Movie Saves" ) );
      msgBox.setTextFormat( Qt::RichText );
      msgBox.setText(
         tr( "A 3-D plot window is opened, so you may save both a "
             "3-D movie and a 2-D one; or you may choose to save "
             "only one.<br/><br/>Save both?<ul>"
             "<li><b>Yes</b> to save both movies;</li>"
             "<li><b>No</b> to save just the 3-D movie;</li>"
             "<li><b>Cancel</b> to cancel any save in order "
             "to edit/close the 3-D window.</li></ul>" ) );
      msgBox.addButton( QMessageBox::Yes );
      msgBox.addButton( QMessageBox::No );
      msgBox.addButton( QMessageBox::Cancel );
      msgBox.setDefaultButton( QMessageBox::Yes );
      int stat = msgBox.exec();

      save_3d        = ( stat != QMessageBox::Cancel );
      save_2d        = ( stat == QMessageBox::Yes );
   }

   if ( save_3d )
      save_3d_movie();

   if ( save_2d )
      save_2d_movie();
}

// Slot to save the current cell's 2-D movie
void US_MwlRawViewer::save_2d_movie()
{
DbgLv(1) << "Save 2D Movie";
   // Loop to plot each record in the cell and save an image to file
   int krecs       = cb_pltrec->count();
   int svrec       = recx;                  // Save currently plotted record
   QStringList fnames;
   QString savedir = US_Settings::reportDir() + "/" + runID;
   QDir().mkpath( savedir );
   savedir         = savedir.replace( "\\", "/" ) + "/";
   QString bfname  = runID + ( ck_xwavlen->isChecked()
                   ? ".radRec_RRRRR_2D_frame_XXXXX.png"
                   : ".lmbRec_RRRRR_2D_frame_XXXXX.png" );
   QString ccr     = cb_cellchn->currentText().remove( " / " );
   QString bstat   = tr( "Of %1 records, saving record " ).arg( krecs );
   le_status->setText( bstat );

   for ( int prx = 0; prx < krecs; prx++ )
   {
      cb_pltrec->setCurrentIndex( prx );    // Plot each record in the range
      qApp->processEvents();

      QString rec_str = ccr + cb_pltrec->currentText().remove( "." );
      QString frm_str = QString().sprintf( "%05d", ( prx + 1 ) );
      QString fname   = QString( bfname ).replace( "RRRRR", rec_str )
                                         .replace( "XXXXX", frm_str );
      QString fpath   = savedir + fname;

      le_status->setText( bstat + rec_str + ", frame " + frm_str );

      US_GuiUtil::save_png( fpath, data_plot );
      fnames << fname;
   }

   cb_pltrec->setCurrentIndex( svrec );     // Restore previous plot record
   qApp->processEvents();

   QMessageBox::information( this, tr( "Frame Files Saved" ),
      tr( "In the directory\n     %1,\n\n%2 2-D movie frame files"
          " were saved:\n     %3\n     ...\n     %4 ." )
      .arg( savedir ).arg( krecs ).arg( fnames[ 0 ] )
      .arg( fnames[ krecs - 1 ] ) );
}

// Slot to save the current cell's 3-D movie
void US_MwlRawViewer::save_3d_movie()
{
DbgLv(1) << "Save 3-D Movie";
   bool hold_color = ck_hcolorc->isChecked();

   if ( p3d_ctld == NULL )
   {  // This should not happen, but disallow Save 3D Movie if no plot opened
      return;
   }

   p3d_pltw     = p3d_ctld->widget_3dplot();    // Main 3D plot window

   if ( p3d_pltw == NULL )
   {  // If the plot window is not opened, explain what to do
      QMessageBox::warning( this,
            tr( "3-D Plot Window Not Opened" ),
            tr( "You cannot save a 3-D movie until you have first"
                " opened a 3-D plotting window by clicking on the"
                " \"3D Plot\" button in the small plot control"
                " dialog.\n\nYou should insure that scale and"
                " orientation are correct for the current plot, as"
                " these will be in force for each movie frame."
                " You may then again click on \"Save Movie(s)\"." ) );
      return;
   }

   // Determine the range of scans that will be in the movie
   int fscnx;
   int lscnx;
   int krscan;

   live_scan( &fscnx, &lscnx, &krscan );

   // Get the save directory and base file name for saved image files
   QStringList ffnames;
   QString imgtype( "png" );
   QString ptbase   = tr( "MWL 3-D Plot, Scan SSS" );
   QString statmsg  = tr( "Of %1 included in-range scans,"
                          " saving:   Scan " ).arg( krscan );
   QString savedir  = US_Settings::reportDir() + "/" + runID;
   QDir().mkpath( savedir );
   savedir          = savedir.replace( "\\", "/" ) + "/";
   QString bfname   = runID + ".Scan_SSSS_3D_frame_XXXX.png";
   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor) );
   int kframe       = 0;

   // Loop to save movie frames for each included scan that is in range
   for ( int scnx = fscnx; scnx <= lscnx; scnx++ )
   {
      if ( excludes.contains( scnx ) )  continue;

      build_xyz_data( xyzdat, scnx );    // Build data for this scan

      p3d_pltw->reloadData( &xyzdat );   // Load the data in the plot window

      kframe++;
      int scan_nbr     = scnx + 1;
      QString t_scan   = QString::number( scan_nbr );
      QString ptitle   = QString( ptbase ).replace( "SSS", t_scan );
      p3d_pltw->setPlotTitle( ptitle );  // Reset the title for scan_nbr
      p3d_pltw->replot( hold_color );    // Do the plot

      le_status->setText( statmsg + t_scan );
      qApp->processEvents();

      // Create a frame file for just-completed plot and save its name
      QString s_scan   = QString().sprintf( "%04d", scan_nbr );
      QString s_frame  = QString().sprintf( "%04d", kframe   );
      QString fname    = QString( bfname ).replace( "SSSS", s_scan  )
                                          .replace( "XXXX", s_frame );
      QString fpath    = savedir + fname;

      p3d_pltw->save_plot( fpath, imgtype );

      ffnames << fname;
      qApp->processEvents();
   }

   QApplication::restoreOverrideCursor();
   QApplication::restoreOverrideCursor();

   // Report the frame files saved
   QMessageBox::information( this,
      tr( "Frame Files Saved" ),
      tr( "In the directory\n     %1,\n\n%2 3-D movie frame files"
          " were saved:\n     %3\n     ...\n     %4 ." )
      .arg( savedir ).arg( kframe ).arg( ffnames[ 0 ] )
      .arg( ffnames[ kframe - 1 ] ) );
}

// Utility to find an index in a QVector<double> to a value epsilon match
int US_MwlRawViewer::dvec_index( QVector< double >& dvec, const double dval )
{
   const double eps   = 1.e-3;

   int indx    = dvec.indexOf( dval );   // Try to find an exact match

   if ( indx < 0 )
   {  // If no exact match was found, look for a match within epsilon

      for ( int jj = 0; jj < dvec.size(); jj++ )
      {  // Search doubles vector
         double ddif     = qAbs( dvec[ jj ] - dval );

         if ( ddif < eps )
         {  // If vector value matches within epsilon, break and return
            indx            = jj;
            break;
         }
      }
   }

   return indx;
}

// Build XYZ data vector for 3D plot
int US_MwlRawViewer::build_xyz_data( QVector< QVector3D >& xyzd, int scan )
{
   // Insure we have ranges set
   compute_ranges();

   // Initialize counters
   int k3dtot   = 0;
   int scnx     = scan;
   int scan_fr  = (int)ct_from->value();
   int scan_to  = (int)ct_to  ->value();
   scan_fr      = ( scan_to < 1 ) ?     1 : scan_fr;
   scan_to      = ( scan_to < 1 ) ? kscan : scan_to;
   int scan_knt = 1;
   if ( excludes.count() > 0 )  qSort( excludes );

   // Get index of scan for which to build
   if ( scan < 0 )
   {  // If no scan given, compute index of first included scan in given range
      for ( scnx = 0; scnx < nscan; scnx++ )
      {
         if ( excludes.contains( scnx ) )  continue;

         if ( scan_knt >= scan_fr )
            break;

         scan_knt++;
      }
   }

   else
   {  // If scan given, insure it is included and in given range
      for ( scnx = 0; scnx < nscan; scnx++ )
      {
         if ( excludes.contains( scnx ) )  continue;

         if ( scan_knt >= scan_fr  &&  scan_knt <= scan_to  &&  scnx == scan )
            break;

         scan_knt++;
      }
   }

   if ( scnx >= nscan )
   {
      qDebug() << "BldXYZ: *ERROR* scan invalid:" << scan << nscan;
      return k3dtot;
   }

   QVector< int    > lmb3d;
   QVector< double > rad3d;

   // Build the list of lambda to plot
   for ( int lmbx = lmbxs; lmbx < lmbxe; lmbx++ )
      lmb3d << lambdas[ lmbx ];

   // Build the list of radii to plot
   for ( int radx = radxs; radx < radxe; radx++ )
      rad3d << radii[ radx ];

   k3dlamb      = lmb3d.count();                          // Lambda plot count
   k3drads      = rad3d.count();                          // Radius plot count
DbgLv(1) << "Bxyz: lambda count" << k3dlamb << "range"
 << lmb3d[0] << lmb3d[k3dlamb-1];
DbgLv(1) << "Bxyz: radius count" << k3drads << "range"
 << rad3d[0] << rad3d[k3drads-1];
   int nhavg    = navgrec / 2;                            // Half avg. points
   int wvx      = lmbxs;                                  // True lambda index
DbgLv(1) << "Bxyz:  navg" << navgrec << "trpxs wvx" << trpxs << wvx;
   xyzd.clear();

   // Now build the data points
   for ( int klx = 0; klx < k3dlamb; klx++, wvx++ )
   {  // Outer loop is lambdas
      int lambda   = lmb3d[ klx ];                        // Lambda value
      int wvxs     = qMax( ( wvx  - nhavg   ), 0 );       // Start for avg.
      int wvxe     = qMin( ( wvxs + navgrec ), nlambda ); // End for avg.
      int kavgc    = wvxe - wvxs;                         // True avg. count
      int trxs     = trpxs + wvxs;                        // Triple start
      int trxe     = trpxs + wvxe;                        // Triple end
      int rdx      = radxs;                               // True radius index
      double yval  = (double)lambda;                      // Y is lambda

      for ( int krx = 0; krx < k3drads; krx++, rdx++ )
      {
         double xval  = rad3d[ krx ];                     // X is radius
         double zval  = 0.0;                              // Initial Z sum

         for ( int trx = trxs; trx < trxe; trx++ )        // WvLn range Z sum
            zval        += allData[ trx ].reading( scnx, rdx );

         zval        /= (double)kavgc;                    // Averaged Z value

         xyzd << QVector3D( xval, yval, zval );           // Store X,Y,Z point
DbgLv(2) << "Bxyz:    k" << xyzd.count() << "x y z" << xval << yval << zval;
      }
   }

   k3dtot       = xyzd.count();
int i=k3drads-1;
int j=i+1;
int k=j+1;
int h=i/2;
int m=k3dtot-2;
int n=m+1;
if(k>=k3dtot) return k3dtot;
DbgLv(1) << "Bxyz: k3dtot" << k3dtot << "k3l k3r" << k3dlamb << k3drads;
DbgLv(1) << "Bxyz:  xyz0" << xyzd[0] << 0;
DbgLv(1) << "Bxyz:  xyz1" << xyzd[1] << 1;
DbgLv(1) << "Bxyz:  xyzh" << xyzd[h] << h;
DbgLv(1) << "Bxyz:  xyzi" << xyzd[i] << i;
DbgLv(1) << "Bxyz:  xyzj" << xyzd[j] << j;
DbgLv(1) << "Bxyz:  xyzk" << xyzd[k] << k;
DbgLv(1) << "Bxyz:  xyzm" << xyzd[m] << m;
DbgLv(1) << "Bxyz:  xyzn" << xyzd[n] << n;
DbgLv(1) << "cmpR:  pxS pxE" << pltxvals[0] << pltxvals[pltxvals.count()-1];

   return k3dtot;
}

// Slot to handle the close of the 3D plot control dialog
void US_MwlRawViewer::p3dctrl_closed()
{
   p3d_ctld     = NULL;
   p3d_pltw     = NULL;

   pb_movie3d->setEnabled( false );
}

// Utility to return first live scan number; plus optionally indexes and count
int US_MwlRawViewer::live_scan( int* fsP, int* lsP, int* ksP )
{
   int fscnx    = -1;
   int lscnx    = -1;
   int krscan   = 0;
   int scan_fr  = (int)ct_from->value();
   int scan_to  = (int)ct_to  ->value();
   scan_fr      = ( scan_to < 1 ) ?     1 : scan_fr;
   scan_to      = ( scan_to < 1 ) ? nscan : scan_to;
   int scan_knt = 0;

   for ( int scnx = 0; scnx < nscan; scnx++ )
   {
      if ( excludes.contains( scnx ) )  continue;

      scan_knt++;               // Non-excluded count

      if ( scan_knt < scan_fr  ||  scan_knt > scan_to )  continue;

      krscan++;                 // In-range count

      if ( fscnx < 0 )
         fscnx        = scnx;   // First in-range
      lscnx        = scnx;      // Last in-range
   }

   if ( fsP != NULL )     *fsP = fscnx;
   if ( lsP != NULL )     *lsP = lscnx;
   if ( ksP != NULL )     *ksP = krscan;

   return ( fscnx + 1 );
}

