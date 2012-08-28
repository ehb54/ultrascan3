//! \file us_export.cpp

#include <QApplication>

#include "us_export.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_gui_util.h"
#include "us_constants.h"
#include "us_passwd.h"
#include "us_data_loader.h"
#include "us_util.h"
#include "us_investigator.h"
#include "us_report.h"
#include "us_license.h"
#include "us_license_t.h"
#include "us_sleep.h"

#define MIN_NTC   25

// main program
int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );

   #include "main1.inc"

   // License is OK.  Start up.
   
   US_ExportLegacy w;
   w.show();                   //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication
}

// US_ExportLegacy class constructor
US_ExportLegacy::US_ExportLegacy() : US_Widgets()
{
   setObjectName( "US_ExportLegacy" );

   int local  = US_Settings::default_data_location();
   dbg_level  = US_Settings::us_debug();

   // set up the GUI
   setPalette( US_GuiSettings::frameColor() );
   setWindowTitle( tr( "Export Data in Legacy (Beckman) Format" ) );

   mainLayout      = new QHBoxLayout( this );
   mainLayout->setSpacing        ( 2 );
   mainLayout->setContentsMargins( 2, 2, 2, 2 );

   leftLayout      = new QVBoxLayout();
   rightLayout     = new QVBoxLayout();
   
   analysisLayout  = new QGridLayout();
   runInfoLayout   = new QGridLayout();
   buttonLayout    = new QHBoxLayout();

   leftLayout->addLayout( analysisLayout  );
   leftLayout->addLayout( runInfoLayout   );
   leftLayout->addStretch();
   leftLayout->addLayout( buttonLayout    );

   // Analysis buttons
   dkdb_cntrls  = new US_Disk_DB_Controls( local );
   pb_load      = us_pushbutton( tr( "Load Data" ) );
   pb_details   = us_pushbutton( tr( "Run Details" ) );
   pb_save      = us_pushbutton( tr( "Export Data" ) );
   pb_view      = us_pushbutton( tr( "View Data Report" ) );

   connect( dkdb_cntrls,  SIGNAL( changed(      bool ) ),
            this,         SLOT( update_disk_db( bool ) ) );
   connect( pb_load,      SIGNAL( clicked()     ),
            this,         SLOT(   load()        ) );
   connect( pb_details,   SIGNAL( clicked()     ),
            this,         SLOT(   details()     ) );
   connect( pb_save,      SIGNAL( clicked()     ),
            this,         SLOT(   export_data() ) );
   connect( pb_view,      SIGNAL( clicked()     ),
            this,         SLOT(   view_report() ) );

   pb_load     ->setEnabled( true );
   pb_details  ->setEnabled( false );
   pb_save     ->setEnabled( false );
   pb_view     ->setEnabled( false );

   int row  = 0;
   analysisLayout->addLayout( dkdb_cntrls, row++, 0, 1, 2 );
   analysisLayout->addWidget( pb_load,     row,   0, 1, 1 );
   analysisLayout->addWidget( pb_details,  row++, 1, 1, 1 );
   analysisLayout->addWidget( pb_save,     row,   0, 1, 1 );
   analysisLayout->addWidget( pb_view,     row++, 1, 1, 1 );

   // Run info
   QLabel* lb_info    = us_banner( tr( "Information for this Run" ) );
   QLabel* lb_triples = us_banner( tr( "Cell / Channel / Wavelength" ) );
   QLabel* lb_id      = us_label ( tr( "Run ID:" ) );
   QLabel* lb_temp    = us_label ( tr( "Avg. Temp.:" ) );
   QLabel* lb_stat    = us_banner( tr( "Export Status" ) );

   le_id      = us_lineedit( "", -1, true );
   le_temp    = us_lineedit( "", -1, true );

   QFont font( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() );
   QFontMetrics fm( font );
   int fontHeight = fm.lineSpacing();

   te_desc    = us_textedit();
   te_desc->setMaximumHeight( fontHeight * 3 + 12 );  // Add for border
   us_setReadOnly( te_desc, true );

   te_stat    = us_textedit();
   te_stat->setMaximumHeight( fontHeight * 3 + 12 );  // Add for border
   te_stat->setTextColor( Qt::blue );
   us_setReadOnly( te_stat, true );

   lw_triples = us_listwidget();
   lw_triples->setMaximumHeight( fontHeight * 8 + 12 );

   row      = 0;
   runInfoLayout->addWidget( lb_info   , row++, 0, 1, 4 );
   runInfoLayout->addWidget( lb_id     , row,   0, 1, 1 );
   runInfoLayout->addWidget( le_id     , row++, 1, 1, 3 );
   runInfoLayout->addWidget( lb_temp   , row,   0, 1, 1 );
   runInfoLayout->addWidget( le_temp   , row++, 1, 1, 3 );
   runInfoLayout->addWidget( te_desc   , row,   0, 3, 4 ); row += 3;
   runInfoLayout->addWidget( lb_triples, row++, 0, 1, 4 );
   runInfoLayout->addWidget( lw_triples, row++, 0, 8, 4 ); row += 8;
   runInfoLayout->addWidget( lb_stat   , row++, 0, 1, 4 );
   runInfoLayout->addWidget( te_stat   , row,   0, 1, 4 ); row += 3;

   // Plots
   plotLayout2 = new US_Plot( data_plot2,
            tr( "Velocity Data" ),
            tr( "Radius (cm)" ),
            tr( "Absorbance" ) );

   data_plot2->setCanvasBackground( Qt::black );
   data_plot2->setMinimumSize( 560, 240 );

   // Standard buttons
   pb_reset    = us_pushbutton( tr( "Reset" ) );
   pb_help     = us_pushbutton( tr( "Help"  ) );
   pb_close    = us_pushbutton( tr( "Close" ) );

   buttonLayout->addWidget( pb_reset    );
   buttonLayout->addWidget( pb_help     );
   buttonLayout->addWidget( pb_close    );

   connect( pb_reset,    SIGNAL( clicked() ),
            this,        SLOT(   reset()   ) );
   connect( pb_close,    SIGNAL( clicked() ),
            this,        SLOT(   close()   ) );
   connect( pb_help,     SIGNAL( clicked() ),
            this,        SLOT(   help()    ) );

   rightLayout->addLayout( plotLayout2 );
   rightLayout->setStretchFactor( plotLayout2, 3 );

   mainLayout->addLayout( leftLayout  );
   mainLayout->addLayout( rightLayout );
   mainLayout->setStretchFactor( leftLayout, 3 );
   mainLayout->setStretchFactor( rightLayout, 5 );

   dataLoaded = false;

   adjustSize();
   setMaximumSize( qApp->desktop()->size() - QSize( 40, 40 ) );
}

// Load data
void US_ExportLegacy::load( void )
{
   QString     file;
   QStringList files;
   QStringList parts;
   lw_triples->  disconnect();
   lw_triples->  clear();
   dataList.     clear();
   rawList.      clear();
   triples.      clear();

   dataLoaded = false;
   int local  = dkdb_cntrls->db() ? US_Disk_DB_Controls::DB
                                  : US_Disk_DB_Controls::Disk;

   US_DataLoader* dialog =
      new US_DataLoader( true, local, rawList, dataList,
            triples, workingDir, QString( "velocity" ) );

   connect( dialog, SIGNAL( changed(      bool ) ),
            this,   SLOT( update_disk_db( bool ) ) );

   if ( dialog->exec() != QDialog::Accepted )  return;

   if ( ! dkdb_cntrls->db() )
   {
      workingDir = workingDir.section( workingDir.left( 1 ), 4, 4 );
      workingDir = workingDir.left( workingDir.lastIndexOf( "/" ) );
   }

   else
      workingDir = tr( "(database)" );

   qApp->processEvents();

   QFont font( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() );
   QFontMetrics fm( font );
   int fontHeight = fm.lineSpacing();
   int ntriples   = triples.size();
   lw_triples->setMaximumHeight( fontHeight * min( ntriples, 8 ) + 12 );

   for ( int ii = 0; ii < ntriples; ii++ )
      lw_triples->addItem( triples.at( ii ) );

   edata     = &dataList[ 0 ];
   scanCount = edata->scanData.size();
   double avgTemp = edata->average_temperature();

   // set ID, description, and avg temperature text
   le_id  ->setText( edata->runID );
   te_desc->setText( edata->description );
   le_temp->setText( QString::number( avgTemp, 'f', 1 ) + " " + DEGC );
   if ( ntriples > 1 )
      te_stat->setText( tr( "%1 input triples" ).arg( ntriples ) );
   else
      te_stat->setText( tr( "1 input triple" ) );

   lw_triples->setCurrentRow( 0 );
   connect( lw_triples, SIGNAL( currentRowChanged( int ) ),
                        SLOT(   new_triple(        int ) ) );

   dataLoaded = true;

   update( 0 );

   pb_details->setEnabled( true );
   pb_view   ->setEnabled( true );
}

// Details
void US_ExportLegacy::details( void )
{
   US_RunDetails2* dialog
      = new US_RunDetails2( rawList, runID, workingDir, triples );

   dialog->move( this->pos() + QPoint( 100, 100 ) );
   dialog->exec();
   qApp->processEvents();

   delete dialog;
}

// Update based on selected triples row
void US_ExportLegacy::update( int drow )
{
   edata          = &dataList[ drow ];
   rdata          = &rawList [ drow ];
   scanCount      = edata->scanData.size();
   runID          = edata->runID;
   le_id->  setText( runID );

   double avgTemp = edata->average_temperature();
   le_temp->setText( QString::number( avgTemp, 'f', 1 )
         + " " + DEGC );
   te_desc->setText( edata->description );

   data_plot();

   pb_view->setEnabled( true );
   pb_save->setEnabled( true );
}


// Data plot
void US_ExportLegacy::data_plot( void )
{
   data_plot2->detachItems();

   if ( !dataLoaded )
      return;

   int drow    = lw_triples->currentRow();
   edata       = &dataList[ drow ];
   rdata       = &rawList [ drow ];
   QString                            dataType = tr( "Absorbance" );
   if ( edata->dataType == "RI" )     dataType = tr( "Intensity" );
   if ( edata->dataType == "WI" )     dataType = tr( "Intensity" );
   if ( edata->dataType == "IP" )     dataType = tr( "Interference" );
   if ( edata->dataType == "FI" )     dataType = tr( "Fluourescence" );
   data_plot2->setTitle(
      tr( "Velocity Data for " ) + edata->runID );
   data_plot2->setAxisTitle( QwtPlot::yLeft,
      dataType + tr( " at " ) + edata->wavelength + tr( " nm" ) );
   data_plot2->setAxisTitle( QwtPlot::xBottom,
      tr( "Radius (cm)" ) );

   data_plot2->clear();
   us_grid( data_plot2 );

   valueCount        = rdata->x.size();

   QVector< double > vecr( valueCount );
   QVector< double > vecv( valueCount );
   double* rr        = vecr.data();
   double* vv        = vecv.data();

   QString       title; 
   QwtPlotCurve* curve;
   QPen          pen_plot( US_GuiSettings::plotCurve() );

   // Draw curves
   for ( int ii = 0; ii < scanCount; ii++ )
   {
      for ( int jj = 0; jj < valueCount; jj++ )
      {  // accumulate coordinates of curve within baseline-to-plateau
         rr[ jj ] = rdata->radius( jj );
         vv[ jj ] = rdata->value( ii, jj );
      }

      // Plot scan curve
      title = tr( "Curve " ) + QString::number( ii );
      curve = us_curve( data_plot2, title );

      curve->setPen( pen_plot );
         
      curve->setData( rr, vv, valueCount );
   }

   data_plot2->replot();
}

// Save the report and image data
void US_ExportLegacy::export_data( void )
{
   QStringList files;
   QString     dtype    = QString( QChar( rdata->type[ 0 ] ) )
                        + QString( QChar( rdata->type[ 1 ] ) );
   QString     legadir( US_Settings::dataDir() + "/legacy" );

   // Insure that */data/legacy/runid exists
   mkdir( legadir, edata->runID );
   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );

   if ( QString( dtype ).mid( 1 ) == "I" )
   {  // Export intensity
      exp_intensity( files );
   }
   else if ( dtype == "IP" )
   {  // Export interference
      exp_interference( files );
   }
   else
   {  // Export most types of data
      exp_mosttypes( files );
   }

   QApplication::restoreOverrideCursor();

   // report the files created
   int nfiles    = files.count();

   QString umsg  = tr( "In directory \"" ) + legadir + "\",\n"
                 + tr( "   in subdirectory \"" ) + edata->runID + "\",\n"
                 + tr( "   %1 files were written:\n" ).arg( nfiles );
   umsg += files[ 0 ] + "\n...\n" + files[ nfiles - 1 ] + "\n";

//*TEMP
//umsg += "\nNO - THE FILE WRITER IS CURRENTLY INCOMPLETE";
//*TEMP

   QMessageBox::information( this, tr( "Successfully Written" ), umsg );
   //QMessageBox::information( this, tr( "NOT Successfully Written" ), umsg );

   int ntriples   = triples.size();
   umsg.clear();
   if ( ntriples > 1 )
      umsg = tr( "From %1 input triples, legacy output\n" ).arg( ntriples );
   else
      umsg = tr( "From 1 input triple, legacy output\n" );

   umsg += tr( "was written to %1 files." ).arg( nfiles );

   te_stat->setText( umsg );
}

// Export most types of data (1 channel per file set, 3 columns per point)
void US_ExportLegacy::exp_mosttypes( QStringList& files )
{
   edata       = &dataList[ 0 ];
   rdata       = &rawList [ 0 ];
   QString legadir( US_Settings::dataDir() + "/legacy" );
   mkdir( legadir, edata->runID );
   QString odirname = legadir + "/" + edata->runID + "/";
   QString ofname   = "00001.RA1";
   QString ofpath   = odirname + ofname;
   int     ntriples = triples.size();

   US_DataIO2::Scan* dscan = &rdata->scanData[ 0 ];
   QString ddesc    = rdata->description + "\n";
   QString dtype    = QString( QChar( rdata->type[ 0 ] ) )
                    + QString( QChar( rdata->type[ 1 ] ) );
   QString htype    = QString( "U" );
           htype    = ( dtype == "RA" ) ? "R" : htype;
           htype    = ( dtype == "RI" ) ? "I" : htype;
           htype    = ( dtype == "IP" ) ? "P" : htype;
           htype    = ( dtype == "FI" ) ? "F" : htype;
           htype    = ( dtype == "WA" ) ? "W" : htype;
           htype    = ( dtype == "WI" ) ? "V" : htype;
DbgLv(1) << "dtype" << dtype << "htype" << htype;
   bool    wldata   = ( QString( dtype ).left( 1 ) == "W" );
   int     hcell    = rdata->cell;
   double  htemp    = dscan->temperature;
   int     hrpm     = qRound( dscan->rpm );
   int     hsecs    = qRound( dscan->seconds );
   double  homeg    = dscan->omega2t;
   double  hradi    = dscan->wavelength;
   int     hwavl    = qRound( dscan->wavelength );
   int     hcoun    = 3;
   QString oline;
   int     nscan    = rdata->scanData.size();
   int     nvalu    = rdata->x.size();
   QString fext     = "." + dtype + QString::number( hcell );

   for ( int drow = 0; drow < ntriples; drow++ )
   {  // Output a set of files for each input triple
      edata  = &dataList[ drow ];
      rdata  = &rawList [ drow ];
      hcell  = rdata->cell;
      fext   = "." + dtype + QString::number( hcell );
      for ( int ii = 0; ii < nscan; ii++ )
      {  // Output a file for each scan
         ofname = QString().sprintf( "%05i", ( ii + 1 ) ) + fext;
         ofpath = odirname + ofname;
         dscan  = &rdata->scanData[ ii ];
         htemp  = dscan->temperature;
         hrpm   = dscan->rpm;
         hsecs  = qRound( dscan->seconds );
         homeg  = dscan->omega2t;
         hwavl  = qRound( dscan->wavelength );
         hradi  = dscan->wavelength;
         oline  = htype
                  + QString().sprintf( "%2i%5.1f%6i %07i%11.4E",
                  hcell, htemp, hrpm, hsecs, homeg ).replace( "E+", "E" );
         oline  = oline + ( wldata
                  ? QString().sprintf( "%6.3f %i\n", hradi, hcoun )
                  : QString().sprintf( "%4i %i\n",   hwavl, hcoun ) );
DbgLv(1) << "OFNAME" << ofname;

         QFile legfile( ofpath );

         if ( ! legfile.open( QIODevice::WriteOnly | QIODevice::Text ) )
         {
            qDebug() << "*ERROR* Opening file" << ofpath;
            continue;
         }

         QTextStream ts( &legfile );
         ts << ddesc;
         ts << oline;
DbgLv(1) << "  LINE:" << QString(ddesc).replace("\n","");
DbgLv(1) << "  LINE:" << QString(oline).replace("\n","");

         for ( int jj = 0; jj < nvalu; jj++ )
         {  // Output a line for each data point
            double  radi  = rdata->radius( jj );
            double  valu  = rdata->value ( ii, jj );
            double  stdd  = dscan->readings[ jj ].stdDev;
            QString oline = QString().sprintf( "%9.4f %12.5E %13.5E\n",
               radi, valu, stdd )
               .replace( "E+", "E+00" ).replace( "E-", "E-00" );
if (jj < 3  || jj > (nvalu-4))
 DbgLv(1) << "  LINE:" << QString(oline).replace("\n","");

            ts << oline;
         }  // END: values loop

         files << ofname;
         legfile.close();
      }  // END: scan loop
   }
}

// Special export of intensity data (2 channels at a time)
void US_ExportLegacy::exp_intensity( QStringList& files )
{
   edata       = &dataList[ 0 ];
   rdata       = &rawList [ 0 ];
   QString legadir( US_Settings::dataDir() + "/legacy" );
   mkdir( legadir, edata->runID );
   QString odirname = legadir + "/" + edata->runID + "/";
   QString ofname   = "00001.RA1";
   QString ofpath   = odirname + ofname;
   int     ntriples = triples.size();

   US_DataIO2::Scan* dscan = &rdata->scanData[ 0 ];
   QString ddesc    = rdata->description + "\n";
   QString dtype    = QString( QChar( rdata->type[ 0 ] ) )
                    + QString( QChar( rdata->type[ 1 ] ) );
   QString htype    = QString( "U" );
           htype    = ( dtype == "RA" ) ? "R" : htype;
           htype    = ( dtype == "RI" ) ? "I" : htype;
           htype    = ( dtype == "IP" ) ? "P" : htype;
           htype    = ( dtype == "FI" ) ? "F" : htype;
           htype    = ( dtype == "WA" ) ? "W" : htype;
           htype    = ( dtype == "WI" ) ? "V" : htype;
DbgLv(1) << "dtype" << dtype << "htype" << htype;
   bool    wldata   = ( QString( dtype ).left( 1 ) == "W" );
   int     hcell    = rdata->cell;
   double  htemp    = dscan->temperature;
   int     hrpm     = qRound( dscan->rpm );
   int     hsecs    = qRound( dscan->seconds );
   double  homeg    = dscan->omega2t;
   double  hradi    = dscan->wavelength;
   int     hwavl    = qRound( dscan->wavelength );
   int     hcoun    = 3;
   int     nscan    = rdata->scanData.size();
   int     nvalu    = rdata->x.size();
   QString fext     = "." + dtype + QString::number( hcell );
   QString oline;
   bool    twofer;
   US_DataIO2::RawData* rdat2 = rdata;

   for ( int drow = 0; drow < ntriples; drow++ )
   {  // Output a set of files for each input triple
      edata  = &dataList[ drow ];
      rdata  = &rawList [ drow ];
      hcell  = rdata->cell;
      fext   = "." + dtype + QString::number( hcell );
      twofer = triples[ drow ].contains( "A" );
      if ( twofer )
      {
         int erow = drow + 1;
         if ( erow < ntriples  && 
              triples[ erow ].contains( "B" ) )
         {
            rdat2  = &rawList[ erow ];
            drow   = erow;
         }

         else
            twofer = false;
      }

      for ( int ii = 0; ii < nscan; ii++ )
      {  // Output a file for each scan
         ofname = QString().sprintf( "%05i", ( ii + 1 ) ) + fext;
         ofpath = odirname + ofname;
         dscan  = &rdata->scanData[ ii ];
         htemp  = dscan->temperature;
         hrpm   = dscan->rpm;
         hsecs  = qRound( dscan->seconds );
         homeg  = dscan->omega2t;
         hwavl  = qRound( dscan->wavelength );
         hradi  = dscan->wavelength;
         oline  = htype
                  + QString().sprintf( "%2i%5.1f%6i %07i%11.4E",
                  hcell, htemp, hrpm, hsecs, homeg ).replace( "E+", "E" );
         oline  = oline + ( wldata
                  ? QString().sprintf( "%6.3f %i\n", hradi, hcoun )
                  : QString().sprintf( "%4i %i\n",   hwavl, hcoun ) );
DbgLv(1) << "OFNAME" << ofname;

         QFile legfile( ofpath );

         if ( ! legfile.open( QIODevice::WriteOnly | QIODevice::Text ) )
         {
            qDebug() << "*ERROR* Opening file" << ofpath;
            continue;
         }

         QTextStream ts( &legfile );
         ts << ddesc;
         ts << oline;
DbgLv(1) << "  LINE:" << QString(ddesc).replace("\n","");
DbgLv(1) << "  LINE:" << QString(oline).replace("\n","");

         for ( int jj = 0; jj < nvalu; jj++ )
         {  // Output a line for each data point
            double  radi  = rdata->radius( jj );
            double  valu  = rdata->value ( ii, jj );
            double  stdd  = twofer ? rdat2->value( ii, jj ) : 0.0;
            QString oline = QString().sprintf( "%9.4f %12.5E %13.5E\n",
               radi, valu, stdd )
               .replace( "E+", "E+00" ).replace( "E-", "E-00" );
if (jj < 3  || jj > (nvalu-4))
 DbgLv(1) << "  LINE:" << QString(oline).replace("\n","");

            ts << oline;
         }  // END: values loop

         files << ofname;
         legfile.close();
      }  // END: scan loop
   }
}

// Special export of interference data (2 columns per point)
void US_ExportLegacy::exp_interference( QStringList& files )
{
   edata       = &dataList[ 0 ];
   rdata       = &rawList [ 0 ];
   QString legadir( US_Settings::dataDir() + "/legacy" );
   mkdir( legadir, edata->runID );
   QString odirname = legadir + "/" + edata->runID + "/";
   QString ofname   = "00001.RA1";
   QString ofpath   = odirname + ofname;
   int     ntriples = triples.size();

   US_DataIO2::Scan* dscan = &rdata->scanData[ 0 ];
   QString ddesc    = rdata->description + "\n";
   QString dtype    = QString( QChar( rdata->type[ 0 ] ) )
                    + QString( QChar( rdata->type[ 1 ] ) );
   QString htype    = QString( "U" );
           htype    = ( dtype == "RA" ) ? "R" : htype;
           htype    = ( dtype == "RI" ) ? "I" : htype;
           htype    = ( dtype == "IP" ) ? "P" : htype;
           htype    = ( dtype == "FI" ) ? "F" : htype;
           htype    = ( dtype == "WA" ) ? "W" : htype;
           htype    = ( dtype == "WI" ) ? "V" : htype;
DbgLv(1) << "dtype" << dtype << "htype" << htype;
   bool    wldata   = ( QString( dtype ).left( 1 ) == "W" );
   int     hcell    = rdata->cell;
   double  htemp    = dscan->temperature;
   int     hrpm     = qRound( dscan->rpm );
   int     hsecs    = qRound( dscan->seconds );
   double  homeg    = dscan->omega2t;
   double  hradi    = dscan->wavelength;
   int     hwavl    = qRound( dscan->wavelength );
   int     hcoun    = 3;
   QString oline;
   int     nscan    = rdata->scanData.size();
   int     nvalu    = rdata->x.size();
   QString fext     = "." + dtype + QString::number( hcell );

   for ( int drow = 0; drow < ntriples; drow++ )
   {  // Output a set of files for each input triple
      edata  = &dataList[ drow ];
      rdata  = &rawList [ drow ];
      hcell  = rdata->cell;
      fext   = "." + dtype + QString::number( hcell );
      for ( int ii = 0; ii < nscan; ii++ )
      {  // Output a file for each scan
         ofname = QString().sprintf( "%05i", ( ii + 1 ) ) + fext;
         ofpath = odirname + ofname;
         dscan  = &rdata->scanData[ ii ];
         htemp  = dscan->temperature;
         hrpm   = dscan->rpm;
         hsecs  = qRound( dscan->seconds );
         homeg  = dscan->omega2t;
         hwavl  = qRound( dscan->wavelength );
         hradi  = dscan->wavelength;
         oline  = htype
                  + QString().sprintf( "%2i%5.1f%6i %07i%11.4E",
                  hcell, htemp, hrpm, hsecs, homeg ).replace( "E+", "E" );
         oline  = oline + ( wldata
                  ? QString().sprintf( "%6.3f %i\n", hradi, hcoun )
                  : QString().sprintf( "%4i %i\n",   hwavl, hcoun ) );
DbgLv(1) << "OFNAME" << ofname;

         QFile legfile( ofpath );

         if ( ! legfile.open( QIODevice::WriteOnly | QIODevice::Text ) )
         {
            qDebug() << "*ERROR* Opening file" << ofpath;
            continue;
         }

         QTextStream ts( &legfile );
         ts << ddesc;
         ts << oline;
DbgLv(1) << "  LINE:" << QString(ddesc).replace("\n","");
DbgLv(1) << "  LINE:" << QString(oline).replace("\n","");

         for ( int jj = 0; jj < nvalu; jj++ )
         {  // Output a line for each data point
            double  radi  = rdata->radius( jj );
            double  valu  = rdata->value ( ii, jj );
            QString oline = QString().sprintf( "%9.4f %12.5E\n", radi, valu );
if (jj < 3  || jj > (nvalu-4))
 DbgLv(1) << "  LINE:" << QString(oline).replace("\n","");

            ts << oline;
         }  // END: values loop

         files << ofname;
         legfile.close();
      }  // END: scan loop
   }
}

// View the report text
void US_ExportLegacy::view_report( )
{
   QString mtext;
   QTextStream ts( &mtext );

   // generate the report file
   write_report( ts );

   // display the report dialog
   US_Editor* editd = new US_Editor( US_Editor::DEFAULT, true, "", this );
   editd->setWindowTitle( tr( "Report:  FE Match Model Simulation" ) );
   editd->move( this->pos() + QPoint( 100, 100 ) );
   editd->resize( 740, 700 );
   editd->e->setFont( QFont( US_GuiSettings::fontFamily(),
                             US_GuiSettings::fontSize() ) );
   editd->e->setHtml( mtext );
   editd->show();
}

// Write the report HTML text stream
void US_ExportLegacy::write_report( QTextStream& ts )
{
   int drow = lw_triples->currentRow();
   QString hline = edata->runID + "    " + triples.at( drow );
   ts << html_header( "US_ExportLegacy", "Legacy Export", edata );
   ts << data_details();
   ts << "  </body>\n</html>\n";
}

// String to accomplish line indentation
QString US_ExportLegacy::indent( const int spaces ) const
{
   return ( QString( " " ).leftJustified( spaces, ' ' ) );
}

// Compose data details text
QString US_ExportLegacy::data_details( void ) const
{
   int    drow     = lw_triples->currentRow();
   const US_DataIO2::EditedData* d = &dataList[ drow ];
   QString                       dataType = tr( "Absorbance" );
   if ( d->dataType == "RI" )    dataType = tr( "Intensity" );
   if ( d->dataType == "WI" )    dataType = tr( "Intensity" );
   if ( d->dataType == "IP" )    dataType = tr( "Interference" );
   if ( d->dataType == "FI" )    dataType = tr( "Fluourescence" );

   QString s =
      "\n" + indent( 4 ) + tr( "<h3>Detailed Run Information:</h3>\n" )
      + indent( 4 ) + "<table>\n"
      + table_row( tr( "Cell Description:" ), d->description )
      + table_row( tr( "Data Directory:"   ), workingDir )
      + table_row( tr( "Rotor Speed:"      ),  
            QString::number( (int)d->scanData[ 0 ].rpm ) + " rpm" );

   // Temperature data
   double sum     =  0.0;
   double maxTemp = -1.0e99;
   double minTemp =  1.0e99;

   for ( int i = 0; i < d->scanData.size(); i++ )
   {
      double t = d->scanData[ i ].temperature;
      sum += t;
      maxTemp = max( maxTemp, t );
      minTemp = min( minTemp, t );
   }

   QString average = QString::number( sum / d->scanData.size(), 'f', 1 );

   s += table_row( tr( "Average Temperature:" ), average + " " + MLDEGC );

   if ( maxTemp - minTemp <= US_Settings::tempTolerance() )
      s += table_row( tr( "Temperature Variation:" ), tr( "Within tolerance" ) );
   else 
      s += table_row( tr( "Temperature Variation:" ), 
                      tr( "(!) OUTSIDE TOLERANCE (!)" ) );

   // Time data
   double  tcorrec  = US_Math2::time_correction( dataList );
   int minutes = (int)tcorrec / 60;
   int seconds = (int)tcorrec % 60;

   QString m   = ( minutes == 1 ) ? tr( " minute " ) : tr( " minutes " );
   QString sec = ( seconds == 1 ) ? tr( " second"  ) : tr( " seconds"  );
   s += table_row( tr( "Time Correction:" ), 
                   QString::number( minutes ) + m +
                   QString::number( seconds ) + sec );

   double duration = rawList.last().scanData.last().seconds;

   int hours = (int) duration / 3600;
   minutes   = (int) duration / 60 - hours * 60;
   seconds   = (int) duration % 60;

   QString h;
   h   = ( hours   == 1 ) ? tr( " hour "   ) : tr( " hours "   );
   m   = ( minutes == 1 ) ? tr( " minute " ) : tr( " minutes " );
   sec = ( seconds == 1 ) ? tr( " second " ) : tr( " seconds " );

   s += table_row( tr( "Run Duration:" ),
                   QString::number( hours   ) + h + 
                   QString::number( minutes ) + m + 
                   QString::number( seconds ) + sec );

   s += table_row( tr( "Wavelength:" ), d->wavelength + " nm" );

   return s;
}

// Create a subdirectory if need be
bool US_ExportLegacy::mkdir( const QString& baseDir, const QString& subdir )
{
   // Make sure */ultrascan/data/legacy exists
   QDir().mkpath( baseDir );

   QDir folder( baseDir );

   // Report subdirectory already exists
   if ( folder.exists( subdir ) ) return true;

   // Create the subdirectory and report if successful
   if ( folder.mkdir( subdir ) ) return true;

   // Otherwise, report a problem
   QMessageBox::warning( this,
      tr( "File error" ),
      tr( "Could not create the directory:\n" ) + baseDir + "/" + subdir );
   
   return false;
}

// Slot to handle selection of a new triple
void US_ExportLegacy::new_triple( int trow )
{
   update( trow );

   data_plot();
}

// Update the disk/DB choice element
void US_ExportLegacy::update_disk_db( bool isDB )
{
   isDB ?  dkdb_cntrls->set_db() : dkdb_cntrls->set_disk();
}

// Reset data set
void US_ExportLegacy::reset( void )
{
   if ( ! dataLoaded ) return;

   lw_triples->  disconnect();
   lw_triples->  clear();
   dataList.     clear();
   rawList.      clear();
   triples.      clear();

   dataLoaded = false;

   data_plot2->detachItems();
   data_plot2->clear();
   data_plot2->replot();

   pb_details  ->setEnabled( false );
   pb_view     ->setEnabled( false );
   pb_save     ->setEnabled( false );
   le_id       ->setText( "" );
   le_temp     ->setText( "" );
   te_desc     ->setText( "" );
   te_stat     ->setText( "" );
}

// Table row HTML with 2 columns
QString US_ExportLegacy::table_row( const QString& s1, const QString& s2 ) const
{
   return( indent( 6 ) + "<tr><td>" + s1 + "</td><td>" + s2 + "</td></tr>\n" );
}

// Compose a report HTML header
QString US_ExportLegacy::html_header( QString title, QString head1,
      US_DataIO2::EditedData* edata )
{
   QString s = QString( "<?xml version=\"1.0\"?>\n" );
   s  += "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\"\n";
   s  += "                      \"http://www.w3.org/TR/xhtml1/DTD"
         "/xhtml1-strict.dtd\">\n";
   s  += "<html xmlns=\"http://www.w3.org/1999/xhtml\""
         " xml:lang=\"en\" lang=\"en\">\n";
   s  += "  <head>\n";
   s  += "    <title> " + title + " </title>\n";
   s  += "    <meta http-equiv=\"Content-Type\" content="
         "\"text/html; charset=iso-8859-1\"/>\n";
   s  += "    <style type=\"text/css\" >\n";
   s  += "      td { padding-right: 1em; }\n";
   s  += "      body { background-color: white; }\n";
   s  += "    </style>\n";
   s  += "  </head>\n  <body>\n";
   s  += "    <h1>" + head1 + "</h1>\n";
   s  += indent( 4 ) + tr( "<h2>Data Report for Run \"" ) + edata->runID;
   s  += "\",<br/>\n" + indent( 4 ) + "&nbsp;" + tr( " Cell " ) + edata->cell;
   s  += tr( ", Channel " ) + edata->channel;
   s  += tr( ", Wavelength " ) + edata->wavelength + "<br/></h2>\n";

   return s;
}

