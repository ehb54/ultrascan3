//! \file us_export.cpp

#include <QApplication>

#include "us_export.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_gui_util.h"
#include "us_constants.h"
#include "us_passwd.h"
#include "us_load_auc.h"
#include "us_util.h"
#include "us_investigator.h"
#include "us_report.h"
#include "us_license.h"
#include "us_license_t.h"
#include "us_sleep.h"
#include "us_matrix.h"
#if QT_VERSION < 0x050000
#define setSamples(a,b,c)  setData(a,b,c)
#endif

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
   rawList.      clear();
   triples.      clear();

   dataLoaded    = false;
   bool isLocal  = !dkdb_cntrls->db();

   US_LoadAUC* dialog = 
      new US_LoadAUC( isLocal, rawList, triples, workingDir ); 

   connect( dialog, SIGNAL( changed       ( bool ) ),
            this,   SLOT(   update_disk_db( bool ) ) );

   if ( dialog->exec() != QDialog::Accepted )  return;

   qApp->processEvents();
   QFont font( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() );
   QFontMetrics fm( font );
   int fontHeight = fm.lineSpacing();
   int ntriples   = triples.size();
   lw_triples->setMaximumHeight( fontHeight * min( ntriples, 8 ) + 12 );

   for ( int ii = 0; ii < ntriples; ii++ )
      lw_triples->addItem( triples.at( ii ) );

   rdata            = &rawList [ 0 ];
   scanCount        = rdata->scanCount();
   double  avgTemp  = rdata->average_temperature();
   runID            = workingDir.section( "/", -1, -1 );

   rDataStrings( rdata, rawDtype, rawCell, rawChann, rawWaveln );

   // set ID, description, and avg temperature text
   le_id  ->setText( runID );
   te_desc->setText( rdata->description );
   le_temp->setText( QString::number( avgTemp, 'f', 1 ) + " " + DEGC );
   if ( ntriples > 1 )
      te_stat->setText( tr( "%1 input %2 triples" )
            .arg( ntriples ).arg( rawDtype ) );
   else
      te_stat->setText( tr( "1 input %1 triple" ).arg( rawDtype ) );

   lw_triples->setCurrentRow( 0 );
   connect( lw_triples, SIGNAL( currentRowChanged( int ) ),
                        SLOT(   new_triple(        int ) ) );

   if ( rawDtype == "RI" )
   {  // Possibly convert Pseudo Absorbance to Intensity
      QVector< double > RIProfile;
      QMessageBox msgBox( this );
      msgBox.setTextFormat   ( Qt::RichText );

      // Get any RI Profile
      int nrip = getRIProfile( RIProfile );

      if ( nrip == 0 )
      { // No RI Profile:  export RA
         msgBox.setWindowTitle  ( tr( "RA Export Type" ) );
         QString mtxt = tr( "The input is <b>RI</b> data,<br>&nbsp;&nbsp;"
                            "but <b>no RI Profile</b> exists!<br><br>"
                            "So, <b>Pseudo Absorbance (RA)</b><br>&nbsp;&nbsp;"
                            "data will be exported." );
         msgBox.setText         ( mtxt );
         msgBox.addButton       ( QMessageBox::Ok );
         msgBox.exec();
         rawDtype = "RA";
      }

      else
      { // RI Profile exists:  ask user RA/RI preference
         QString ybtn = tr( "Absorbance" );
         QString nbtn = tr( "Intensity" );
         QString mtxt = tr( "For <b>RI</b> data, you may export values as<br>"
                            "<b>Intensity</b> or "
                            "<b>Pseudo Absorbance</b>.<br><br>"
                            "Which export type do you want?" );
         msgBox.setWindowTitle  ( tr( "RI Export Type" ) );
         msgBox.setText         ( mtxt );
         QPushButton* pb_abs = msgBox.addButton( ybtn, QMessageBox::YesRole );
         msgBox.addButton       ( nbtn, QMessageBox::NoRole  );
         msgBox.setDefaultButton( pb_abs );
         msgBox.exec();

         if ( msgBox.clickedButton() == pb_abs )
         { // RA chosen:  no conversion necessary
DbgLv(1) << "RI Export: YES : Absorbance";
            rawDtype = "RA";
         }

         else
         { // RI chosen:  convert data to Intensity
DbgLv(1) << "RI Export: NO  : Intensity";
            convertToIntensity( RIProfile );
         }
      }

      if ( rawDtype == "RA" )
      { // Export type changed to RA:  modify data headers
         for ( int kk = 0; kk < rawList.size(); kk++ )
         {
            rawList[ kk ].type[ 1 ] = 'A';
         }
      }
   }
DbgLv(1) << "Chosen/Forced export rawDtype" << rawDtype;

   dataLoaded = true;

   update( 0 );

   pb_details->setEnabled( true );
   pb_view   ->setEnabled( true );
   delete dialog;
}

// Details
void US_ExportLegacy::details( void )
{
   QString         workDir = dkdb_cntrls->db() ? tr( "(database)" )
                                               : workingDir;
   US_RunDetails2* dialog  = new US_RunDetails2( rawList, runID,
                                                 workDir, triples );

   dialog->move( this->pos() + QPoint( 100, 100 ) );
   dialog->exec();
   qApp->processEvents();

   delete dialog;
}

// Update based on selected triples row
void US_ExportLegacy::update( int drow )
{
   rdata          = &rawList [ drow ];
   scanCount      = rdata->scanCount();
   double avgTemp = rdata->average_temperature();

   le_id  ->setText( runID );
   le_temp->setText( QString::number( avgTemp, 'f', 1 ) + " " + DEGC );
   te_desc->setText( rdata->description );

   data_plot();

   pb_view->setEnabled( true );
   pb_save->setEnabled( true );
}


// Data plot
void US_ExportLegacy::data_plot( void )
{
   dataPlotClear( data_plot2 );

   if ( !dataLoaded )
      return;

   int drow    = lw_triples->currentRow();
   rdata       = &rawList [ drow ];
   rDataStrings( rdata, rawDtype, rawCell, rawChann, rawWaveln );
   QString                     dataType = tr( "Absorbance" );
   if ( rawDtype == "RI" )     dataType = tr( "Intensity" );
   if ( rawDtype == "WI" )     dataType = tr( "Intensity" );
   if ( rawDtype == "IP" )     dataType = tr( "Interference" );
   if ( rawDtype == "FI" )     dataType = tr( "Fluourescence" );
   data_plot2->setTitle(
      tr( "Velocity Data for " ) + runID );
   data_plot2->setAxisTitle( QwtPlot::yLeft,
      dataType + tr( " at " ) + rawWaveln + tr( " nm" ) );
   data_plot2->setAxisTitle( QwtPlot::xBottom,
      tr( "Radius (cm)" ) );

   us_grid( data_plot2 );

   valueCount        = rdata->pointCount();

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

      curve->setPen    ( pen_plot );
      curve->setSamples( rr, vv, valueCount );
   }

   data_plot2->replot();
}

// Save the report and image data
void US_ExportLegacy::export_data( void )
{
   QString rawDtyp2;
   rDataStrings( rdata, rawDtyp2, rawCell, rawChann, rawWaveln );
   QStringList files;
   QString     legadir( US_Settings::importDir() );

   // Insure that */imports/runid exists
   mkdir( legadir, runID );
   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );

   if ( rawDtype == "RI"  ||  rawDtype == "WI" )
   {  // Export intensity
      exp_intensity( files );
   }
   else if ( rawDtype == "IP" )
   {  // Export interference
      exp_interference( files );
   }
   else
   {  // Export most types of data
      exp_mosttypes( files );
   }

   QApplication::restoreOverrideCursor();

   // Report the files created
   int nfiles    = files.count();

   QString umsg  = tr( "In directory \"" ) + legadir + "\",\n"
                 + tr( "   in subdirectory \"" ) + runID + "\",\n"
                 + tr( "   %1 files were written:\n" ).arg( nfiles );
   umsg += files[ 0 ] + "\n...\n" + files[ nfiles - 1 ] + "\n";

   QMessageBox::information( this, tr( "Successfully Written" ), umsg );

   // Give a summary in the status box
   int ntriples   = triples.size();
   umsg.clear();
   if ( ntriples > 1 )
      umsg = tr( "From %1 input %2 triples, legacy output\n" )
         .arg( ntriples ).arg( rawDtype );
   else
      umsg = tr( "From 1 input %1 triple, legacy output\n" ).arg( rawDtype );

   umsg += tr( "was written to %1 files." ).arg( nfiles );

   te_stat->setText( umsg );
}

// Export most types of data (1 channel per file set, 3 columns per point)
void US_ExportLegacy::exp_mosttypes( QStringList& files )
{
  qDebug() << "Abs export";
   // Get data pointers and output directory
   rdata       = &rawList [ 0 ];
   QString legadir( US_Settings::importDir() );
   mkdir( legadir, runID );
   QString odirname = legadir + "/" + runID + "/";
   QString ofname;
   QString ofpath;
   int     ntriples = triples.size();

   // Determine data type
   US_DataIO::Scan* dscan = &rdata->scanData[ 0 ];
   QString ddesc    = rdata->description + "\n";
   QString htype    = QString( "U" );
           htype    = ( rawDtype == "RA" ) ? "R" : htype;
           htype    = ( rawDtype == "RI" ) ? "I" : htype;
           htype    = ( rawDtype == "IP" ) ? "P" : htype;
           htype    = ( rawDtype == "FI" ) ? "F" : htype;
           htype    = ( rawDtype == "WA" ) ? "W" : htype;
           htype    = ( rawDtype == "WI" ) ? "V" : htype;
DbgLv(1) << "rawDtype" << rawDtype << "htype" << htype;
   // Get first scan header information
   bool    wldata   = ( QString( rawDtype ).left( 1 ) == "W" );
   int     hcell    = rdata->cell;
   double  htemp    = dscan->temperature;
   int     hrpm     = qRound( dscan->rpm );
   int     hsecs    = qRound( dscan->seconds );
   double  homeg    = dscan->omega2t;
   double  hradi    = dscan->wavelength;
   int     hwavl    = qRound( dscan->wavelength );
   int     hcoun    = 3;
   QString oline;
   int     nscan    = rdata->scanCount();
   int     nvalu    = rdata->pointCount();
   QString fext     = "." + rawDtype + QString::number( hcell );
   bool    channdir = false;

   QString tripa;
   
   if ( htype == "R" )
   { // For RA, determine if pseudo absorbance needing channel subdirectories
      for ( int drow = 0; drow < ntriples; drow++ )
      {
         QString chann  = triples[ drow ].section( "/", 1, 1 ).simplified();
DbgLv(1) << " drow chann" << drow << chann;
         if ( chann != "A" )
         {
            channdir   = true;
            break;
         }
      }
   }

   for ( int drow = 0; drow < ntriples; drow++ )
   {  // Output a set of files for each input triple
      rdata  = &rawList [ drow ];               // Current data
      nscan  = rdata->scanCount();

      tripa  = triples[ drow ];
      
      ddesc  = rdata->description + "\n";
      hcell  = rdata->cell;
      fext   = "." + rawDtype + QString::number( hcell );
      QString chann    = triples[ drow ].section( "/", 1, 1 ).simplified();

      if ( channdir )
      { // For pseudo absorbance, output to channel subdirectory
         QString odirchan = runID + "_channel" + chann;
         odirname         = legadir + "/" + odirchan + "/";
         mkdir( legadir, odirchan );
      }


      //ALEXEY: create sub-dir based on triple name:
      QString tripleName = tripa;
      tripleName.replace(" / ","");
      mkdir( odirname, tripleName );
      QString odirname_triple =  odirname + tripleName + "/";
      //////////////////////////////////////////////////////////////////
      
      for ( int ii = 0; ii < nscan; ii++ )
      {  // Output a file for each scan
         ofname = chann + QString().sprintf( "%05i", ( ii + 1 ) ) + fext;

	 //ofpath = odirname + ofname;            // Full path file name for scan
	 ofpath = odirname_triple + ofname;            // Full path output file
	 
         dscan  = &rdata->scanData[ ii ];       // Scan pointer
         htemp  = dscan->temperature;           // Temperature
         hrpm   = dscan->rpm;                   // RPM
         hsecs  = qRound( dscan->seconds );     // Seconds as integer
         homeg  = dscan->omega2t;               // Omega^2 * T
         hwavl  = qRound( dscan->wavelength );  // Wavelength as integer
         hradi  = dscan->wavelength;            // Radius (possibly)
         // Format most of header line
         oline  = htype
                  + QString().sprintf( "%2i%5.1f%6i %07i%11.4E",
                  hcell, htemp, hrpm, hsecs, homeg ).replace( "E+", "E" );
         // Complete header line, using radius if Wavelength data
         oline  = oline + ( wldata
                  ? QString().sprintf( "%6.3f %i\n", hradi, hcoun )
                  : QString().sprintf( "%4i %i\n",   hwavl, hcoun ) );
	 DbgLv(1) << "OFNAME, ofpath " << ofname << ofpath;

         QFile legfile( ofpath );

         if ( ! legfile.open( QIODevice::WriteOnly | QIODevice::Text ) )
         {
            qDebug() << "*ERROR* Opening file" << ofpath;
            continue;
         }

         QTextStream ts( &legfile );
         ts << ddesc;                           // Write description line
         ts << oline;                           // Write header line
DbgLv(1) << "  LINE:" << QString(ddesc).replace("\n","");
DbgLv(1) << "  LINE:" << QString(oline).replace("\n","");
//DbgLv(1) << "  LINE: nv ii sizes" << nvalu << ii << rdata->scanCount()
//   << rdata->pointCount() << "nscan" << nscan;

         for ( int jj = 0; jj < nvalu; jj++ )
         {  // Output a line for each data point
            double  radi  = rdata->radius ( jj );
            double  valu  = rdata->value  ( ii, jj );
            double  stdd  = rdata->std_dev( ii, jj );
            // Format data line:  Radius Value StdDev
            QString oline = QString().sprintf( "%9.4f %12.5E %13.5E\n",
               radi, valu, stdd )
               .replace( "E+", "E+00" ).replace( "E-", "E-00" );
if (jj < 3  || jj > (nvalu-4))
 DbgLv(1) << "  LINE:" << QString(oline).replace("\n","");

            ts << oline;                        // Write data line
         }  // END: values loop

         files << ofname;                       // Save scan file name
         legfile.close();                       // Close file
      }  // END: scan loop
   }

   qDebug() << "Abs export Finished";
}

// Special export of intensity data (2 channels at a time)
void US_ExportLegacy::exp_intensity( QStringList& files )
{
   // Get 1st triple data pointer and output directory
   rdata       = &rawList [ 0 ];
   QString legadir( US_Settings::importDir() );
   mkdir( legadir, runID );
   QString odirname = legadir + "/" + runID + "/";
   QString ofname;
   QString ofpath;
   int     ntriples = triples.size();

   // Get data type
   US_DataIO::Scan* dscan = &rdata->scanData[ 0 ];
   QString ddesc    = rdata->description + "\n";
   QString htype    = QString( "U" );
           htype    = ( rawDtype == "RA" ) ? "R" : htype;
           htype    = ( rawDtype == "RI" ) ? "I" : htype;
           htype    = ( rawDtype == "IP" ) ? "P" : htype;
           htype    = ( rawDtype == "FI" ) ? "F" : htype;
           htype    = ( rawDtype == "WA" ) ? "W" : htype;
           htype    = ( rawDtype == "WI" ) ? "V" : htype;
DbgLv(1) << "rawDtype" << rawDtype << "htype" << htype;
   bool    wldata   = ( QString( rawDtype ).left( 1 ) == "W" );
   int     hcell    = rdata->cell;
   double  htemp    = dscan->temperature;
   int     hrpm     = qRound( dscan->rpm );
   int     hsecs    = qRound( dscan->seconds );
   double  homeg    = dscan->omega2t;
   double  hradi    = dscan->wavelength;
   int     hwavl    = qRound( dscan->wavelength );
   int     hcoun    = 3;
   int     nscan    = rdata->scanCount();
   int     nvalu    = rdata->pointCount();
   QString fext     = "." + rawDtype + QString::number( hcell );
   QString tripa;
   QString oline;
   bool    twofer;
   bool    bfirst;
   US_DataIO::RawData* rdat2 = rdata;

   for ( int drow = 0; drow < ntriples; drow++ )
   {  // Output a set of files for each input triple
      rdata  = &rawList [ drow ];
      nscan  = rdata->scanCount();
      ddesc  = rdata->description + "\n";
      hcell  = rdata->cell;
      fext   = "." + rawDtype + QString::number( hcell );

      tripa  = triples[ drow ];
                
      twofer = tripa.contains( "A" );
      bfirst = false;
      if ( twofer )
      {  // We have channel A, test if we have a B next
         int     erow  = drow + 1;          // Next triple index
         QString tripa = triples[ drow ];   // First triple in pair
         if ( erow < ntriples  && 
              triples[ erow ].contains( "B" )  &&
              QString( triples[ erow ] ).replace( "B", "A" ) == tripa )
         {  // We have a B that goes with this A:  set up to do 2-at-a-time
            rdat2  = &rawList[ erow ];
            drow   = erow;
         }

         else  // No next triple or not B, so only do A on this pass
            twofer = false;
      }
      else     // We are at a B, so set to output "Radius 0.0 Value-B"
         bfirst = true;

      //ALEXEY: create sub-dir based on triple name:
      QString tripleName = tripa;
      tripleName.replace(" / ","");
      mkdir( odirname, tripleName );
      QString odirname_triple =  odirname + tripleName + "/";
      //////////////////////////////////////////////////////////////////
      
      for ( int ii = 0; ii < nscan; ii++ )
      {  // Output a file for each scan
         ofname = QString().sprintf( "%05i", ( ii + 1 ) ) + fext;

	 //ofpath = odirname + ofname;            // Full path output file

	 ofpath = odirname_triple + ofname;            // Full path output file
	 
         dscan  = &rdata->scanData[ ii ];       // Current scan pointer
         htemp  = dscan->temperature;           // Temperature
         hrpm   = dscan->rpm;                   // RPM
         hsecs  = qRound( dscan->seconds );     // Seconds as integer
         homeg  = dscan->omega2t;               // Omega^2*T
         hwavl  = qRound( dscan->wavelength );  // Wavelength as integer
         hradi  = dscan->wavelength;            // Radius (possibly)
         // Format most of header line
         oline  = htype
                  + QString().sprintf( "%2i%5.1f%6i %07i%11.4E",
                  hcell, htemp, hrpm, hsecs, homeg ).replace( "E+", "E" );
         // Complete header line, using radius if Wavelength data
         oline  = oline + ( wldata
                  ? QString().sprintf( "%6.3f %i\n", hradi, hcoun )
                  : QString().sprintf( "%4i %i\n",   hwavl, hcoun ) );
DbgLv(1) << "OFNAME" << ofname;
DbgLv(1) << "Ofpath" << ofpath;
         

         QFile legfile( ofpath );

         if ( ! legfile.open( QIODevice::WriteOnly | QIODevice::Text ) )
         {
            qDebug() << "*ERROR* Opening file" << ofpath;
            continue;
         }

         QTextStream ts( &legfile );
         ts << ddesc;               // Description line
         ts << oline;               // Header line
DbgLv(1) << "  LINE:" << QString(ddesc).replace("\n","");
DbgLv(1) << "  LINE:" << QString(oline).replace("\n","");

         for ( int jj = 0; jj < nvalu; jj++ )
         {  // Output a line for each data point
            double  radi  = rdata->radius( jj );
            double  valu  = rdata->value ( ii, jj );
            double  stdd  = twofer ? rdat2->value( ii, jj ) : 0.0;
                    stdd  = bfirst ? valu : stdd;
                    valu  = bfirst ? 0.0  : valu;
            // Format line mostly as Radius,Value-A,Value-B
            QString oline = QString().sprintf( "%9.4f %12.5E %13.5E\n",
               radi, valu, stdd )
               .replace( "E+", "E+00" ).replace( "E-", "E-00" );
if (jj < 3  || jj > (nvalu-4))
 DbgLv(1) << "  LINE:" << QString(oline).replace("\n","");

            ts << oline;            // Data line
         }  // END: values loop

         files << ofname;           // Save just-output file name
         legfile.close();           // Close the file
      }  // END: scan loop
   }
}

// Special export of interference data (2 columns per point)
void US_ExportLegacy::exp_interference( QStringList& files )
{
   // Get 1st triple data pointers and output directory
   rdata       = &rawList [ 0 ];
   QString legadir( US_Settings::importDir() );
   mkdir( legadir, runID );
   QString odirname = legadir + "/" + runID + "/";
   QString ofname;
   QString ofpath;
   int     ntriples = triples.size();

   // Get data type
   US_DataIO::Scan* dscan = &rdata->scanData[ 0 ];
   QString ddesc    = rdata->description + "\n";
   QString htype    = QString( "U" );
           htype    = ( rawDtype == "RA" ) ? "R" : htype;
           htype    = ( rawDtype == "RI" ) ? "I" : htype;
           htype    = ( rawDtype == "IP" ) ? "P" : htype;
           htype    = ( rawDtype == "FI" ) ? "F" : htype;
           htype    = ( rawDtype == "WA" ) ? "W" : htype;
           htype    = ( rawDtype == "WI" ) ? "V" : htype;
DbgLv(1) << "rawDtype" << rawDtype << "htype" << htype;
   bool    wldata   = ( QString( rawDtype ).left( 1 ) == "W" );
   int     hcell    = rdata->cell;
   double  htemp    = dscan->temperature;
   int     hrpm     = qRound( dscan->rpm );
   int     hsecs    = qRound( dscan->seconds );
   double  homeg    = dscan->omega2t;
   double  hradi    = dscan->wavelength;
   int     hwavl    = qRound( dscan->wavelength );
   int     hcoun    = 3;
   QString oline;
   int     nscan    = rdata->scanCount();
   int     nvalu    = rdata->pointCount();
   QString fext     = "." + rawDtype + QString::number( hcell );

   for ( int drow = 0; drow < ntriples; drow++ )
   {  // Output a set of files for each input triple
      rdata  = &rawList [ drow ];
      nscan  = rdata->scanCount();
      ddesc  = rdata->description + "\n";
      hcell  = rdata->cell;
      fext   = "." + rawDtype + QString::number( hcell );

      for ( int ii = 0; ii < nscan; ii++ )
      {  // Output a file for each scan
         ofname = QString().sprintf( "%05i", ( ii + 1 ) ) + fext;
         ofpath = odirname + ofname;            // Full path output file
         dscan  = &rdata->scanData[ ii ];       // Current scan pointer
         htemp  = dscan->temperature;           // Temperature
         hrpm   = dscan->rpm;                   // RPM
         hsecs  = qRound( dscan->seconds );     // Seconds as integer
         homeg  = dscan->omega2t;               // Omega^2*T
         hwavl  = qRound( dscan->wavelength );  // Wavelength as integer
         hradi  = dscan->wavelength;            // Radius (possibly)
         // Format most of header line
         oline  = htype
                  + QString().sprintf( "%2i%5.1f%6i %07i%11.4E",
                  hcell, htemp, hrpm, hsecs, homeg ).replace( "E+", "E" );
         // Complete header line, using radius if Wavelength data
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
         ts << ddesc;                           // Write description line
         ts << oline;                           // Write header line
DbgLv(1) << "  LINE:" << QString(ddesc).replace("\n","");
DbgLv(1) << "  LINE:" << QString(oline).replace("\n","");

         for ( int jj = 0; jj < nvalu; jj++ )
         {  // Output a line for each data point
            double  radi  = rdata->radius( jj );
            double  valu  = rdata->value ( ii, jj );
            // Format the data line:  Radius Value-A
            QString oline = QString().sprintf( "%9.4f %12.5E\n", radi, valu );
if (jj < 3  || jj > (nvalu-4))
 DbgLv(1) << "  LINE:" << QString(oline).replace("\n","");

            ts << oline;                        // Write data line
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

   // Generate the report file
   write_report( ts );

   // Display the report dialog
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
   ts << html_header( "US_ExportLegacy", "Legacy Export", rdata );
   ts << data_details();
   ts << "  </body>\n</html>\n";
}

// String to accomplish line indentation
QString US_ExportLegacy::indent( int spaces ) const
{
   return ( QString( " " ).leftJustified( spaces, ' ' ) );
}

// Compose data details text
QString US_ExportLegacy::data_details( void )
{
   QString                    dataType = tr( "Unknown" );
   if ( rawDtype == "RA" )    dataType = tr( "Radial Absorbance" );
   if ( rawDtype == "RI" )    dataType = tr( "Radial Intensity" );
   if ( rawDtype == "WA" )    dataType = tr( "Wavelength Absorbance" );
   if ( rawDtype == "WI" )    dataType = tr( "Wavelength Intensity" );
   if ( rawDtype == "IP" )    dataType = tr( "Interference" );
   if ( rawDtype == "FI" )    dataType = tr( "Fluourescence Intensity" );
   dataType       = dataType + "  (" + rawDtype + ")";
   QString expDir = US_Settings::importDir() + "/" + runID;

   QString s =
      "\n" + indent( 4 ) + tr( "<h3>Detailed Run Information:</h3>\n" )
      + indent( 4 ) + "<table>\n"
      + table_row( tr( "Cell Description:" ), rdata->description )
      + table_row( tr( "Data Directory:"   ), workingDir )
      + table_row( tr( "Export Directory:" ), expDir )
      + table_row( tr( "Data Type:"        ), dataType )
      + table_row( tr( "Rotor Speed:"      ),  
            QString::number( (int)rdata->scanData[ 0 ].rpm ) + " rpm" );

   // Temperature data
   double sum     =  0.0;
   double maxTemp = -1.0e99;
   double minTemp =  1.0e99;

   for ( int i = 0; i < rdata->scanCount(); i++ )
   {
      double t = rdata->scanData[ i ].temperature;
      sum += t;
      maxTemp = max( maxTemp, t );
      minTemp = min( minTemp, t );
   }

   QString average = QString::number( sum / rdata->scanCount(), 'f', 1 );

   s += table_row( tr( "Average Temperature:" ), average + " " + MLDEGC );

   if ( maxTemp - minTemp <= US_Settings::tempTolerance() )
      s += table_row( tr( "Temperature Variation:" ), tr( "Within tolerance" ) );
   else 
      s += table_row( tr( "Temperature Variation:" ), 
                      tr( "(!) OUTSIDE TOLERANCE (!)" ) );

   // Time data
   double tcorrec  = time_correction();
   int    minutes  = (int)tcorrec / 60;
   int    seconds  = (int)tcorrec % 60;

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

   s += table_row( tr( "Wavelength:" ), rawWaveln + " nm" );

   return s;
}

// Create a subdirectory if need be
bool US_ExportLegacy::mkdir( QString& baseDir, QString& subdir )
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
   rawList.      clear();
   triples.      clear();

   dataLoaded = false;

   dataPlotClear( data_plot2 );
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
      US_DataIO::RawData* rdata )
{
   rDataStrings( rdata, rawDtype, rawCell, rawChann, rawWaveln );

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
   s  += indent( 4 ) + tr( "<h2>Data Report for Run \"" ) + runID;
   s  += "\",<br/>\n" + indent( 4 ) + "&nbsp;" + tr( " Cell " ) + rawCell;
   s  += tr( ", Channel " ) + rawChann;
   s  += tr( ", Wavelength " ) + rawWaveln + "<br/></h2>\n";

   return s;
}

// Compose some strings from RawData that exist for EditedData
void US_ExportLegacy::rDataStrings( US_DataIO::RawData* rdata,
   QString& Dtype, QString& Cell, QString& Chann, QString& Waveln )
{
   Dtype    = QString( QChar( rdata->type[ 0 ] ) )
            + QString( QChar( rdata->type[ 1 ] ) );
   Cell     = QString::number( rdata->cell );
   Chann    = QString( QChar( rdata->channel ) );
   Waveln   = QString::number( rdata->scanData[ 0 ].wavelength );
}

// Compute time correction
double US_ExportLegacy::time_correction()
{
   int size  = rawList[ 0 ].scanCount();

   for ( int ii = 1; ii < rawList.size(); ii++ )
      size += rawList[ ii ].scanCount();

   int count = 0;

   QVector< double > vecx( size );
   QVector< double > vecy( size );
   double* x = vecx.data();
   double* y = vecy.data();
   
   double c[ 2 ];  // Looking for a linear fit

   for ( int ii = 0; ii < rawList.size(); ii++ )
   {
      US_DataIO::RawData* d = &rawList[ ii ];

      for ( int jj = 0; jj < d->scanCount(); jj++ )
      {
         if ( d->scanData[ jj ].omega2t > 9.99999e10 ) break;

         x[ count ] = d->scanData[ jj ].omega2t;
         y[ count ] = d->scanData[ jj ].seconds;
         count++;
      }
   }

   US_Matrix::lsfit( c, x, y, count, 2 );

   return c[ 0 ]; // Return the time value corresponding to zero omega2t
}

// Get RI Profile if it exists
int US_ExportLegacy::getRIProfile( QVector< double >& RIProfile )
{
   QString ripxml;
   bool    isDB  = dkdb_cntrls->db();
   runID         = workingDir.section( "/", -1, -1 );

   if ( isDB )
   {  // Data from DB:  get any RIProfile from experiment record
      US_Passwd pw;
      US_DB2 db( pw.getPasswd() );
      QStringList query;
      query << "get_experiment_info_by_runID"
            << runID
            << QString::number( US_Settings::us_inv_ID() );
      db.query( query );
      if ( db.lastErrno() == US_DB2::NOROWS )
         return 0;
      db.next();
      ripxml   = db.value( 16 ).toString();
   }

   else
   {  // Data local:  read in any RIProfile from an XML file
      QString filename = workingDir + "/" + runID + ".RIProfile.xml";

      QFile fi( filename );

      if ( !fi.open( QIODevice::ReadOnly | QIODevice::Text ) )
         return 0;

      ripxml  = fi.readAll();
      fi.close();
   }

   if ( ripxml.isEmpty() )
      return 0;

   parseRIProfile( ripxml, RIProfile );

   return RIProfile.size();
}

// Convert pseudo absorbance to intensity if RIProfile exists
void US_ExportLegacy::convertToIntensity( QVector< double >& RIProfile )
{
   int ntriples = rawList.size();
   int lrip     = RIProfile.size() - 1;
   if ( lrip < 0 )
      return;
DbgLv(1) << "CnvPA: ntrip lrip" << ntriples << lrip;

   for ( int kk = 0; kk < ntriples; kk++ )
   { // Loop to convert each data set to Intensity
      rdata            = &rawList[ kk ];
      scanCount        = rdata->scanCount();
      valueCount       = rdata->pointCount();
DbgLv(1) << "CnvPA:  kk scns vals" << kk << scanCount << valueCount;

      for ( int ii = 0; ii < scanCount; ii++ )
      { // Convert each scan using a RIProfile term
         US_DataIO::Scan* rscan = &rdata->scanData[ ii ];
         double rip    = RIProfile[ qMin( ii, lrip ) ];
DbgLv(1) << "CnvPA:   ii rip" << ii << rip;
DbgLv(1) << "CnvPA:    aval0" << rscan->rvalues[0]
 << "pow" << pow(10.0,rscan->rvalues[0]);

         for ( int jj = 0; jj < valueCount; jj++ )
         {
            double aval          = rscan->rvalues[ jj ];
            rscan->rvalues[ jj ] = rip / pow( 10.0, aval );
         }
DbgLv(1) << "CnvPA:    ival0" << rscan->rvalues[0];
      }
   }
}

// Parse RI Profile values from XML
void US_ExportLegacy::parseRIProfile( QString& ripxml,
                                      QVector< double >& RIProfile )
{
   RIProfile.clear();
   QXmlStreamReader xml( ripxml );
   QXmlStreamAttributes atts;

   while ( ! xml.atEnd() )
   {
      xml.readNext();

      if ( xml.isStartElement()  &&  xml.name() == "RI" )
      {
         atts   = xml.attributes();
         RIProfile << atts.value( "value" ).toString().toDouble();
      }
   }
int nprof=RIProfile.size();
DbgLv(1) << "ParsRIProf: nprof" << RIProfile.size();
if(nprof>0) DbgLv(1) << "ParsRIProf: RIP0" << RIProfile[0];
if(nprof>1) DbgLv(1) << "ParsRIProf: RIPn" << RIProfile[nprof-1];
}

