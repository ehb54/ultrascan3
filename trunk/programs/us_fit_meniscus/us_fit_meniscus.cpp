//! \file us_fit_meniscus.cpp
#include <QApplication>

#include "us_fit_meniscus.h"
#include "us_license_t.h"
#include "us_license.h"
#include "us_gui_settings.h"
#include "us_settings.h"
#include "us_db2.h"
#include "us_passwd.h"
#include "us_investigator.h"
#include "us_math2.h"
#include "us_matrix.h"
#include "us_model.h"
#include "us_noise.h"

//! \brief Main program for US_FitMeniscus. Loads translators and starts
//         the class US_FitMeniscus.

int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );

   #include "main1.inc"

   // License is OK.  Start up.
   
   US_FitMeniscus w;
   w.show();                   //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication
}

US_FitMeniscus::US_FitMeniscus() : US_Widgets()
{
   setWindowTitle( tr( "Fit Meniscus from 2DSA Data" ) );
   setPalette( US_GuiSettings::frameColor() );
   dbg_level = US_Settings::us_debug();

   // Main layout
   QBoxLayout*  mainLayout   = new QVBoxLayout( this );
   mainLayout->setSpacing         ( 2 );
   mainLayout->setContentsMargins ( 2, 2, 2, 2 );

   // Component layouts
   QHBoxLayout* topLayout    = new QHBoxLayout;
   QHBoxLayout* bottomLayout = new QHBoxLayout;
   QGridLayout* leftLayout   = new QGridLayout;
   QGridLayout* rightLayout  = new QGridLayout;
   QGridLayout* cntrlsLayout = new QGridLayout;

   // Lay out the meniscus,rmsd text box
   te_data = new US_Editor( US_Editor::LOAD, false,
         "results/2dsa-fm*.dat;;*.dat;;*.*" );
   connect( te_data, SIGNAL( US_EditorLoadComplete( QString ) ), 
                     SLOT  ( file_loaded(           QString ) ) );
   
   QFontMetrics fm( te_data->e->font() ); 

   te_data->setMinimumHeight( fm.height() * 20 );
   te_data->setMinimumWidth ( fm.width( "11 :  6.34567, 0.00567890 " ) );
   te_data->e->setToolTip( tr( "Loaded, editable meniscus,rmsd table" ) );

   leftLayout->addWidget( te_data, 0, 0, 20, 1 );

   // Lay out the plot
   QBoxLayout* plot = new US_Plot( meniscus_plot, 
         tr( "Meniscus Fit" ),
         tr( "Radius" ), tr( "2DSA Meniscus RMSD Value" ) );
   
   us_grid( meniscus_plot );
   
   meniscus_plot->setMinimumSize( 400, 400 );
   meniscus_plot->setAxisScale( QwtPlot::xBottom, 5.7, 6.8 );
   meniscus_plot->setToolTip( tr( "Fitted meniscus,rmsd plot" ) );

   rightLayout->addLayout( plot, 0, 1, 20, 1 );

   // Lay out the controls
   QLabel* lb_status    = us_label( tr( "Status:"    ) );
   
   le_status    = us_lineedit( tr( "No data loaded" ) );
   le_status->setReadOnly( true );
   le_status->setToolTip(
         tr( "Results of the last action performed" ) );

   QLabel* lb_order = us_label( tr( "Fit Order:" ) );

   sb_order = new QSpinBox();
   sb_order->setRange( 2, 9 );
   sb_order->setValue( 2 );
   sb_order->setPalette( US_GuiSettings::editColor() );
   sb_order->setToolTip( tr( "Order of fitting curve" ) );
   connect( sb_order, SIGNAL( valueChanged( int ) ), SLOT( plot_data( int ) ) );

   QLabel* lb_fit = us_label( tr( "Meniscus at minimum:" ) );

   le_fit = us_lineedit( "" );
   le_fit->setReadOnly( false );
   le_fit->setToolTip(
         tr( "Selected-minimum/Editable meniscus radius value" ) );

   QLabel* lb_rms_error = us_label( tr( "RMS Error:" ) );
   
   le_rms_error = us_lineedit( "" );
   le_rms_error->setReadOnly( true );
   le_rms_error->setToolTip(
         tr( "RMS error of curve to meniscus,rmsd points" ) );

   dkdb_cntrls            = new US_Disk_DB_Controls(
         US_Settings::default_data_location() );
   connect( dkdb_cntrls, SIGNAL( changed( bool )        ),
            this,        SLOT(   update_disk_db( bool ) ) );

   pb_update = us_pushbutton( tr( "Update Edit" ) );
   connect( pb_update, SIGNAL( clicked() ), SLOT( edit_update() ) );
   pb_update->setEnabled( false );
   pb_update->setToolTip(
         tr( "Update edit record with meniscus; remove non-chosen models" ) );

   pb_scandb = us_pushbutton( tr( "Scan Database" ) );
   connect( pb_scandb, SIGNAL( clicked() ), SLOT( scan_dbase() ) );
   pb_scandb->setEnabled( dkdb_cntrls->db() );
   pb_scandb->setToolTip(
         tr( "Scan fit-meniscus models in DB; create local table files" ) );

   pb_plot   = us_pushbutton( tr( "Plot" ) );
   connect( pb_plot, SIGNAL( clicked() ), SLOT( plot_data() ) );
   pb_plot->setToolTip(
         tr( "Plot,analyze meniscus,rmsd from current text" ) );

   pb_reset  = us_pushbutton( tr( "Reset" ) );
   connect( pb_reset, SIGNAL( clicked() ), SLOT( reset() ) );
   pb_reset->setToolTip(
         tr( "Clear text,plot and various other controls" ) );

   QPushButton* pb_help   = us_pushbutton( tr( "Help" ) );
   connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );
   pb_help->setToolTip(
         tr( "Open a dialog with detailed documentation" ) );

   QPushButton* pb_accept = us_pushbutton( tr( "Close" ) );
   connect( pb_accept, SIGNAL( clicked() ), SLOT( close() ) );
   pb_accept->setToolTip(
         tr( "Close this dialog and exit the program" ) );

   // Do detailed layout of the controls
   int row = 0;
   cntrlsLayout->addWidget( lb_status,    row,    0, 1,  1 );
   cntrlsLayout->addWidget( le_status,    row++,  1, 1, 15 );
   cntrlsLayout->addWidget( lb_order,     row,    0, 1,  2 );
   cntrlsLayout->addWidget( sb_order,     row,    2, 1,  1 );
   cntrlsLayout->addWidget( lb_fit,       row,    3, 1,  5 );
   cntrlsLayout->addWidget( le_fit,       row,    8, 1,  3 );
   cntrlsLayout->addWidget( lb_rms_error, row,   11, 1,  2 );
   cntrlsLayout->addWidget( le_rms_error, row++, 13, 1,  3 );
   cntrlsLayout->addLayout( dkdb_cntrls,  row,    0, 1,  6 );
   cntrlsLayout->addWidget( pb_update,    row,    6, 1,  5 );
   cntrlsLayout->addWidget( pb_scandb,    row++, 11, 1,  5 );
   cntrlsLayout->addWidget( pb_plot,      row,    0, 1,  4 );
   cntrlsLayout->addWidget( pb_reset,     row,    4, 1,  4 );
   cntrlsLayout->addWidget( pb_help,      row,    8, 1,  4 );
   cntrlsLayout->addWidget( pb_accept,    row,   12, 1,  4 );

   // Define final layout
   topLayout   ->addLayout( leftLayout   );
   topLayout   ->addLayout( rightLayout  );
   topLayout   ->setStretchFactor( leftLayout,  1 );
   topLayout   ->setStretchFactor( rightLayout, 2 );
   bottomLayout->addLayout( cntrlsLayout );

   mainLayout  ->addLayout( topLayout    );
   mainLayout  ->addLayout( bottomLayout );
   mainLayout  ->setStretchFactor( topLayout,    2 );
   mainLayout  ->setStretchFactor( bottomLayout, 0 );
}

// Clear the plot, m-r table text, and other elements
void US_FitMeniscus::reset( void )
{
   meniscus_plot->clear();
   meniscus_plot->replot();
   
   te_data->e   ->setPlainText( "" );
   sb_order     ->setValue( 2 );
   le_fit       ->setText( "" );
   le_rms_error ->setText( "" );
}

// Plot the data
void US_FitMeniscus::plot_data( int )
{
   plot_data();
}

// Plot the data
void US_FitMeniscus::plot_data( void )
{
   meniscus_plot->clear();

   QString contents = te_data->e->toPlainText();
   contents.replace( QRegExp( "[^0-9eE\\.\\n\\+\\-]+" ), " " );

   QStringList lines = contents.split( "\n", QString::SkipEmptyParts );
   QStringList parsed;

   QVector< double > vradi( lines.size() );
   QVector< double > vrmsd( lines.size() );
   double* radius_values = vradi.data();
   double* rmsd_values   = vrmsd.data();
   
   int     count = 0;

   double  minx = 1e20;
   double  maxx = 0.0;

   double  miny = 1e20;
   double  maxy = 0.0;

   // Remove any non-data lines and put values in arrays
   for ( int ii = 0; ii < lines.size(); ii++ )
   {
      QStringList values = lines[ ii ].split( ' ', QString::SkipEmptyParts );

      if ( values.size() > 1 ) 
      {
         if ( values.size() > 2 ) values.removeFirst();
         
         double radius = values[ 0 ].toDouble();
         if ( radius < 5.7  || radius > 7.3 ) continue;

         radius_values[ count ] = radius;
         rmsd_values  [ count ] = values[ 1 ].toDouble();

         // Find min and max
         minx = min( minx, radius_values[ count ] );
         maxx = max( maxx, radius_values[ count ] );

         miny = min( miny, rmsd_values[ count ] );
         maxy = max( maxy, rmsd_values[ count ] );

         // Reformat
         //parsed << QString::number( radius_values[ count ], 'e', 6 ) + ", " +
         //          QString::number( rmsd_values  [ count ], 'e', 6 ); 
         parsed << QString().sprintf( "%2d : ", ii + 1 ) +
                   QString::number( radius_values[ count ], 'f', 5 ) + ", " +
                   QString::number( rmsd_values  [ count ], 'f', 8 ); 

         count++;
      }
   }

   if ( count < 3 ) return;

   te_data->e->setPlainText( parsed.join( "\n" ) );

   double overscan = ( maxx - minx ) * 0.10;  // 10% overscan

   meniscus_plot->setAxisScale( QwtPlot::xBottom, 
         minx - overscan, maxx + overscan );
    
   // Adjust y axis to scale all the data
   double dy = fabs( maxy - miny ) / 10.0;

   meniscus_plot->setAxisScale( QwtPlot::yLeft, miny - dy, maxy + dy );

   raw_curve = us_curve( meniscus_plot, tr( "Raw Data" ) ); 
   raw_curve->setPen( QPen( Qt::yellow ) );

   raw_curve->setData( radius_values, rmsd_values, count );

   // Do the fit and get the minimum

   double c[ 10 ];

   int order = sb_order->value();

   if ( ! US_Matrix::lsfit( c, radius_values, rmsd_values, count, order + 1 ) )
   {
      QMessageBox::warning( this,
            tr( "Data Problem" ),
            tr( "The data is inadequate for this fit order" ) );
      
      le_fit      ->clear();
      le_rms_error->clear();
      meniscus_plot->replot();

      return;  
   }

   int fit_count = (int) ( ( maxx - minx + 2 * overscan ) / 0.001 );

   QVector< double > vfitx( fit_count );
   QVector< double > vfity( fit_count );
   double* fit_x = vfitx.data();
   double* fit_y = vfity.data();
   double  x     = minx - overscan;
   double minimum;

   for ( int i = 0; i < fit_count; i++, x += 0.001 )
   {
      fit_x[ i ] = x;
      fit_y[ i ] = c[ 0 ];

      for ( int j = 1; j <= order; j++ ) 
         fit_y[ i ] += c[ j ] * pow( x, j );
   }

   // Calculate Root Mean Square Error
   double rms_err = 0.0;

   for ( int i = 0; i < count; i++ )
   {
      double x = radius_values[ i ];
      double y = rmsd_values  [ i ];

      double y_calc = c[ 0 ];
      
      for ( int j = 1; j <= order; j++ )  
         y_calc += c[ j ] * pow( x, j );
      
      rms_err += sq ( fabs ( y_calc - y ) );
   }

   le_rms_error->setText( QString::number( sqrt( rms_err / count ), 'e', 5 ) );

   // Find the minimum
   if ( order == 2 )
   {
      // Take the derivitive and get the minimum
      // c1 + 2 * c2 * x = 0
      minimum = - c[ 1 ] / ( 2.0 * c[ 2 ] );
   }
   else
   {
      // Find the zero of the derivitive
      double dxdy  [ 9 ];
      double d2xdy2[ 8 ];

      // First take the derivitive
      for ( int i = 0; i < order; i++ ) 
         dxdy[ i ] = c[ i + 1 ] * ( i + 1 );

      // And we'll need the 2nd derivitive
      for ( int i = 0; i < order - 1; i++ ) 
         d2xdy2[ i ] = dxdy[ i + 1 ] * ( i + 1 );

      // We'll do a quadratic fit for the initial estimate
      double q[ 3 ];
      US_Matrix::lsfit( q, radius_values, rmsd_values, count, 3 );
      minimum = - q[ 1 ] / ( 2.0 * q[ 2 ] );

      const double epsilon = 1.0e-4;

      int    k = 0;
      double f;
      double f_prime;
      do
      {
        // f is the 1st derivitive
        f = dxdy[ 0 ];
        for ( int i = 1; i < order; i++ ) f += dxdy[ i ] * pow( minimum, i );

        // f_prime is the 2nd derivitive
        f_prime = d2xdy2[ 0 ];
        for ( int i = 1; i < order - 1; i++ ) 
           f_prime += d2xdy2[ i ] * pow( minimum, i );

        if ( fabs( f ) < epsilon ) break;
        if ( k++ > 10 ) break;

        // Get the next estimate
        minimum -= f / f_prime;

      } while ( true );
   }

   fit_curve = us_curve( meniscus_plot, tr( "Fitted Data" ) ); 
   fit_curve->setPen( QPen( Qt::red ) );
   fit_curve->setData( fit_x, fit_y, fit_count );
   
   // Plot the minimum

   minimum_curve = us_curve( meniscus_plot, tr( "Minimum Pointer" ) ); 
   minimum_curve->setPen( QPen( QBrush( Qt::cyan ), 3.0 ) );

   double radius_min[ 2 ];
   double rmsd_min  [ 2 ];

   radius_min[ 0 ] = minimum;
   radius_min[ 1 ] = minimum;

   rmsd_min  [ 0 ] = miny - 1.0 * dy;
   rmsd_min  [ 0 ] = miny + 2.0 * dy;

   minimum_curve->setData( radius_min, rmsd_min, 2 );

   // Put the minimum in the line edit box also
   le_fit->setText( QString::number( minimum, 'f', 5 ) );

   // Add the marker label -- bold, font size default + 1, lines 3 pixels wide
   QPen markerPen( QBrush( Qt::white ), 3.0 );
   markerPen.setWidth( 3 );
   
   QwtPlotMarker* pm = new QwtPlotMarker();
   QwtText        label( QString::number( minimum, 'f', 5 ) );
   QFont          font( pm->label().font() );

   font.setBold( true );
   font.setPointSize( font.pointSize() + 1 );
   label.setFont( font );

   pm->setValue( minimum, miny + 3.0 * dy );
   pm->setSymbol( QwtSymbol( QwtSymbol::Cross, 
            QBrush( Qt::white ), markerPen, QSize( 9, 9 ) ) );
   pm->setLabel( label );
   pm->setLabelAlignment( Qt::AlignTop );

   pm->attach( meniscus_plot );

   meniscus_plot->replot();
}

// Update an edit file with a new meniscus radius value
void US_FitMeniscus::edit_update( void )
{
   QString fn = filedir + "/" + fname_edit;
   QFile filei( fn );
   QString edtext;

   if ( ! filei.open( QIODevice::ReadOnly | QIODevice::Text ) )
   {
      return;
   }

   QTextStream ts( &filei );
   while ( !ts.atEnd() )
      edtext += ts.readLine() + "\n";
   filei.close();

   int mlsx = edtext.indexOf( "<meniscus radius=" );
   int meqx = edtext.indexOf( "=\"", mlsx );
   int mvsx = meqx + 2;
   int mvcn = edtext.indexOf( "\"",  mvsx + 1 ) - mvsx;

   edtext   = edtext.replace( mvsx, mvcn, le_fit->text() );
   int mlnn = edtext.indexOf( ">", mlsx ) - mlsx + 1;

   QFile fileo( fn );

   if ( ! fileo.open( QIODevice::WriteOnly | QIODevice::Text ) )
      return;

   QTextStream tso( &fileo );
   tso << edtext;
   fileo.close();

   QString msg = tr( "In file directory\n    " ) + filedir + " ,\n" +
                 tr( "file\n    " ) + fname_edit + "\n" +
                 tr( "has been modified with the line:\n    " ) + 
                 edtext.mid( mlsx, mlnn );
   
   // If using DB, update the edit record there

   if ( dkdb_cntrls->db() )
   {
      update_db_edit( edtext, fn, msg );
   }

   msg += tr( "\n\nDo you want to remove all fit-meniscus models\n"
              "(and associated noises) except for the one\n"
              "that has the minimum RMSD value?" );

   int response = QMessageBox::question( this, tr( "Edit File Updated" ),
         msg, QMessageBox::Yes, QMessageBox::Cancel );

   if ( response == QMessageBox::Yes )
   {
DbgLv(1) << " call Remove Models";
      remove_models();
   }
else DbgLv(1) << " NO Models removed";
}

// Slot for handling a loaded file:  set the name of loaded,edit files
void US_FitMeniscus::file_loaded( QString fn )
{
   filedir    = fn.section( "/",  0, -2 );
   fname_load = fn.section( "/", -1, -1 );

   QString edittrip = fname_load.section( ".", -3, -3 );
   QString editID   = edittrip.section( "-", 0, 0 ).mid( 1 );
   QString tripnode = edittrip.section( "-", 1, 1 );
   QString runID    = filedir.section( "/", -1, -1 );
   QString tripl    = tripnode.left( 1 ) + "." +
                      tripnode.mid( 1, 1 ) + "." +
                      tripnode.mid( 2 );
   QStringList edtfilt;
   edtfilt << runID + "." + editID + ".*." + tripl + ".xml";

   fname_edit = "";

   QStringList edtfiles = QDir( filedir ).entryList(
         edtfilt, QDir::Files, QDir::Name );
DbgLv(1) << "EDITFILT" << edtfilt;
   if ( edtfiles.size() >= 1 )
   {
      fname_edit = edtfiles.at( 0 );
      pb_update->setEnabled( true );
   }

   else
   {  // Could not find edit file, so try the database
   }
DbgLv(1) << " fname_edit" << fname_edit;

   plot_data();

   le_status->setText( tr( "Data loaded:  " ) + runID + "/" + fname_load );
}

// Scan the database for models to use to write local fit table files
void US_FitMeniscus::scan_dbase()
{
   US_Passwd pw;                   // DB password
   US_DB2 db( pw.getPasswd() );    // DB control
   QStringList query;              // DB query string list
   QStringList modIDs;             // List of FM model IDs
   QStringList modGIs;             // List of FM model GUIDs
   QStringList mdescs;             // List of FM model descriptions
   QStringList medIDs;             // List of FM model edit IDs
   QStringList medGIs;             // List of FM model edit GUIDs
   QList< double > mvaris;         // List of FM model variance values
   QList< double > mmenis;         // List of FM model meniscus values
   QStringList mfnams;             // List of FM model fit file names
   QStringList ufnams;             // List of unique model fit file names
   QStringList uantms;             // List of unique model fit analysis times

   int         nfmods = 0;         // Number of fit-meniscus models
   int         nfsets = 0;         // Number of fit-meniscus analysis sets
   int         nfrpls = 0;         // Number of fit file replacements
   int         nfadds = 0;         // Number of fit file additions
   int         nfexss = 0;         // Number of fit files left as they existed

   QString     invID = QString::number( US_Settings::us_inv_ID() );

   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );

   // Scan the database and find fit-meniscus models

   le_status->setText(
         tr( "Scanning DB fit-meniscus models ..." ) );
   query << "get_model_desc" << invID;
   db.query( query );

   while( db.next() )
   {
      QString modelID    = db.value( 0 ).toString();
      QString modelGUID  = db.value( 1 ).toString();
      QString descript   = db.value( 2 ).toString();
      double  variance   = db.value( 3 ).toString().toDouble();
      double  meniscus   = db.value( 4 ).toString().toDouble();
      QString editGUID   = db.value( 5 ).toString();
      QString editID     = db.value( 6 ).toString();
      QString ansysID    = descript.section( '.', -2, -2 );
      QString iterID     = ansysID .section( '_',  4,  4 );
DbgLv(1) << "DbSc:   modelID vari meni" << modelID << variance << meniscus;

      if ( iterID.length() == 10  &&  iterID.contains( "-m6" ) )
      {  // Model from meniscus fit, so save information
DbgLv(1) << "DbSc:    *FIT* " << descript;
         modIDs << modelID;
         modGIs << modelGUID;
         mdescs << descript;
         medIDs << editID;
         medGIs << editGUID;
         mvaris << variance;
         mmenis << meniscus;

         // Format and save the potential fit table file name
         QString runID      = descript.section( '.',  0, -4 );
         QString tripleID   = descript.section( '.', -3, -3 );
         QString editLabel  = ansysID .section( '_',  0,  0 );
         QString ftfname    = runID + "/2dsa-fm." + editLabel +
                              "-" + tripleID + ".fit.dat";
         mfnams << ftfname;
DbgLv(1) << "DbSc:      ftfname" << ftfname;
      }
else DbgLv(1) << "DbSc:    iterID" << iterID;
   }

   nfmods     = modIDs.size();
DbgLv(1) << "Number of FM models found: " << nfmods;

   // Scan local files to see what fit table files already exist

   le_status->setText(
         tr( "Comparing to existing local meniscus,rmsd table files ..." ) );

   for ( int ii = 0; ii < nfmods; ii++ )
   {  // Find unique file names in order to create sets
      QString ftfname    = mfnams.at( ii );
      QString antime     = mdescs.at( ii ).section( '.', -2, -2 )
                                          .section( '_',  1,  1 );

      if ( ! ufnams.contains( ftfname )  &&  ! uantms.contains( antime ) )
      {  // This is a new occurence of a file name and analysis time
         ufnams << ftfname;
         uantms << antime;
      }
   }

   nfsets     = ufnams.size();
   int kfsets = nfsets;
   QString rdir = US_Settings::resultDir().replace( "\\", "/" ) + "/";
DbgLv(1) << "Number of FM analysis sets: " << nfsets;
   QString fnamesv;

   for ( int ii = 0; ii < kfsets; ii++ )
   {  // Find out for each set whether a corresponding fit file exists
      QString ftfname    = ufnams.at( ii );

      if ( mfnams.count( ftfname ) == 1 )
      {  // Not really a set; single fit model after previous fm run
         nfsets--;
         continue;
      }

      QString ftfpath    = rdir + ftfname;
      QFile   ftfile( ftfpath );

      if ( ftfile.exists() )
      {  // File exists, so we must check the need to replace it
         QString ftfpath    = rdir + ftfname;
         QDateTime fdate    = QFileInfo( ftfile ).lastModified().toUTC();
         int       jj       = mfnams.indexOf( ftfname );
         QString   modelID  = modIDs.at( jj );
         query.clear();
         query << "get_model_info" << modelID;
         db.next();
         QDateTime rdate    = db.value( 6 ).toDateTime().toUTC();

         if ( rdate > fdate )
         {  // DB record is later than file, so must replace file
            nfrpls++;
            ftfile.remove();
         }

         else
         {  // DB record is younger than file, so leave file as is;
            nfexss++;
            continue;
         }
      }

      else
      {  // File does not exist, so we definitely need to create it
         nfadds++;
         QString ftfpath    = QString( rdir + ftfname ).section( "/", 0, -2 );
         QDir().mkpath( ftfpath );
      }

      if ( ! ftfile.open( QIODevice::WriteOnly | QIODevice::Text ) )
      {  // Problem!!!
         qDebug() << "*ERROR* Unable to open file" << ftfname;
         continue;
      }

      // Creating a new or replacement file:  build list of meniscus,rmsd pairs
      int       jfirst   = mfnams.indexOf( ftfname );
      int       jlast    = mfnams.lastIndexOf( ftfname ) + 1;
      QStringList mrpairs;

      for ( int jj = jfirst; jj < jlast; jj++ )
      {  // First build the pairs list
         mrpairs << QString::number( mmenis.at( jj ),         'f', 6 ) + " "
                  + QString::number( sqrt( mvaris.at( jj ) ), 'e', 6 ); 
      }

      mrpairs.sort();
      QTextStream ts( &ftfile );

      // Output the pairs to the file
      for ( int jj = 0; jj < mrpairs.size(); jj++ )
         ts << mrpairs.at( jj ) + "\n";

      ftfile.close();

      fnamesv = fnamesv.isEmpty() ? ftfname : fnamesv;
   }

DbgLv(1) << "Number of FM REPLACE  sets: " << nfrpls;
DbgLv(1) << "Number of FM ADD      sets: " << nfadds;
DbgLv(1) << "Number of FM EXISTING sets: " << nfexss;

   // Report
   QString msg = tr( "Scan complete: " );

   if ( nfadds == 1  ||  nfrpls == 1 )
   {
      msg += tr( "File %1" ).arg( fnamesv );

      if ( nfrpls == 0 )
         msg += tr( " added." );

      else if ( nfadds == 0 )
         msg += tr( " updated." );

      else
         msg += tr( " last added or replaced." );
   }

   else if ( nfadds == 0  &&  nfrpls == 0 )
   {
      msg += tr( "No new fit files were created." );
   }

   else
   {
      msg += tr( "File %1 was the last added or replaced." ).arg( fnamesv );
   }

   le_status->setText( msg );
   QApplication::restoreOverrideCursor();
}

// Reset state of database scan button based on DB/Disk choice
void US_FitMeniscus::update_disk_db( bool isDB )
{
   pb_scandb->setEnabled( isDB );
}

// Update the DB edit record with a new meniscus value
void US_FitMeniscus::update_db_edit( QString edtext, QString efilepath,
      QString& msg )
{
   int     elnx     = edtext.indexOf( "<editGUID " );
   int     esvx     = edtext.indexOf( "\"", elnx ) + 1;
   int     nvch     = edtext.indexOf( "\"", esvx ) - esvx;
   QString edGUID   = edtext.mid( esvx, nvch );
DbgLv(1) << "updDbEd: edGUID" << edGUID;

   US_Passwd pw;
   US_DB2 db( pw.getPasswd() );
   QStringList query;
   query << "get_editID" << edGUID;
   db.query( query );
   db.next();
   int     idEdit   = db.value( 0 ).toString().toInt();
DbgLv(1) << "updDbEd: idEdit" << idEdit;
   db.writeBlobToDB( efilepath, "upload_editData", idEdit );

   msg += tr( "\n\nThe meniscus value was also updated for the"
              "\ncorresponding edit record in the database." );
   return;
}

// Remove f-m models (and associated noise) except for the single chosen one
void US_FitMeniscus::remove_models()
{
   QString srchRun  = filedir.section( "/", -1, -1 );
   QString srchEdit = fname_load.section( ".", -3, -3 );
   QString srchTrip = srchEdit.section( "-", 1, 1 );
           srchEdit = srchEdit.section( "-", 0, 0 );
//DbgLv(1) << "RmvMod: scn1 srchRun srchEdit srchTrip"
// << srchRun << srchEdit << srchTrip;

   // Scan models files; get list of fit-meniscus type matching run/edit/triple
   QStringList modfilt;
   modfilt << "M*.xml";
   QString     moddir   = US_Settings::dataDir() + "/models";
   QStringList modfiles = QDir( moddir ).entryList(
         modfilt, QDir::Files, QDir::Name );
   moddir               = moddir + "/";

   QStringList     lmodFnams;             // local model full path file names
   QStringList     lmodGUIDs;             // Local model GUIDs
   QList< double > lmodVaris;             // Local variance values
   QList< double > lmodMenis;             // Local meniscus values
   QStringList     lmodDescs;             // Local descriptions

   QStringList     dmodIDs;               // Database model IDs
   QStringList     dmodGUIDs;             // Database model GUIDs
   QList< double > dmodVaris;             // Database variance values
   QList< double > dmodMenis;             // Database meniscus values
   QStringList     dmodDescs;             // Database descriptions
   int nlmods           = 0;
   int ndmods           = 0;
   int nlnois           = 0;
   int ndnois           = 0;
   int lArTime          = 0;
   int dArTime          = 0;
   int lkModx           = -1;
   int dkModx           = -1;

   for ( int ii = 0; ii < modfiles.size(); ii++ )
   {
      QString modfname   = modfiles.at( ii );
      QString modpath    = moddir + modfname;
      US_Model model;
      
      if ( model.load( modpath ) != US_DB2::OK )
         continue;    // Can't use if can't load

      QString descript   = model.description;
      QString runID      = descript.section( '.',  0, -4 );
      QString tripID     = descript.section( '.', -3, -3 );
      QString anRunID    = descript.section( '.', -2, -2 );
      QString editLabl   = anRunID .section( '_',  0,  0 );
//DbgLv(1) << "RmvMod:  scn1 ii runID editLabl tripID"
// << ii << runID << editLabl << tripID;

      if ( runID != srchRun  ||  editLabl != srchEdit  ||  tripID != srchTrip )
         continue;    // Can't use if from a different runID or edit or triple

      QString iterID     = anRunID .section( '_',  4,  4 );
//DbgLv(1) << "RmvMod:    iterID" << iterID;

      if ( iterID.length() != 10  ||  ! iterID.contains( "-m" ) )
         continue;    // Can't use if not a fit-meniscus type

      // Probably a file from the right set, but let's check for other sets
      int     arTime     = anRunID .section( '_',  1,  1 ).mid( 1 ).toInt();

      if ( arTime > lArTime )
      {  // If first set or new one younger than previous, start lists
         lmodFnams.clear();
         lmodGUIDs.clear();
         lmodVaris.clear();
         lmodMenis.clear();
         lmodDescs.clear();
         lArTime            = arTime;
      }

      else if ( arTime < lArTime )
      {  // If new one older than previous, skip it
         continue;
      }

      lmodFnams << modpath;             // Save full path file name
      lmodGUIDs << model.modelGUID;     // Save model GUID
      lmodVaris << model.variance;      // Save variance
      lmodMenis << model.meniscus;      // Save meniscus
      lmodDescs << model.description;   // Save description
   }

   nlmods         = lmodFnams.size();
DbgLv(1) << "RmvMod: nlmods" << nlmods;
   double minVari = 99e+10;

   for ( int ii = 0; ii < nlmods; ii++ )
   {  // Scan to identify model in set with lowest variance
DbgLv(1) << "low Vari scan: ii vari meni desc" << ii << lmodVaris.at(ii)
 << lmodMenis.at(ii) << lmodDescs.at(ii).right( 22 );
      if ( lmodVaris.at( ii ) < minVari )
      {
         minVari        = lmodVaris.at( ii );
         lkModx         = ii;
//DbgLv(1) << "low Vari scan:   minVari lkModx" << minVari << lkModx;
      }
   }
DbgLv(1) << "RmvMod:  minVari lkModx" << minVari << lkModx;

   // Make a list of f-m models that match for DB, if possible
   if ( dkdb_cntrls->db() )
   {
      QString   invID = QString::number( US_Settings::us_inv_ID() );
      US_Passwd pw;
      US_DB2 db( pw.getPasswd() );
      QStringList query;

      query << "get_model_desc" << invID;
      db.query( query );

      while( db.next() )
      {
         QString modelID    = db.value( 0 ).toString();
         QString modelGUID  = db.value( 1 ).toString();
         QString descript   = db.value( 2 ).toString();
         double  variance   = db.value( 3 ).toString().toDouble();
         double  meniscus   = db.value( 4 ).toString().toDouble();
         QString runID      = descript.section( '.',  0, -4 );
         QString tripID     = descript.section( '.', -3, -3 );
         QString anRunID    = descript.section( '.', -2, -2 );
         QString editLabl   = anRunID .section( '_',  0,  0 );
//DbgLv(1) << "RmvMod:  scn1 ii runID editLabl tripID"
// << ii << runID << editLabl << tripID;

         if ( runID != srchRun  ||  editLabl != srchEdit
          ||  tripID != srchTrip )
         continue;    // Can't use if from a different runID or edit or triple

         QString iterID     = anRunID .section( '_',  4,  4 );
//DbgLv(1) << "RmvMod:    iterID" << iterID;

         if ( iterID.length() != 10  ||  ! iterID.contains( "-m" ) )
            continue;    // Can't use if not a fit-meniscus type

         // Probably a file from the right set, but let's check for other sets
         int     arTime     = anRunID .section( '_',  1,  1 ).mid( 1 ).toInt();

         if ( arTime > dArTime )
         {  // If first set or new one younger than previous, start lists
            dmodIDs  .clear();
            dmodGUIDs.clear();
            dmodVaris.clear();
            dmodMenis.clear();
            dmodDescs.clear();
            dArTime            = arTime;
         }

         else if ( arTime < dArTime )
         {  // If new one older than previous, skip it
            continue;
         }

         dmodIDs   << modelID;             // Save model DB ID
         dmodGUIDs << modelGUID;           // Save model GUID
         dmodVaris << variance;            // Save variance
         dmodMenis << meniscus;            // Save meniscus
         dmodDescs << descript;            // Save description
DbgLv(1) << "RmvMod:  scn2 ii dmodDesc" << descript; 
      }

      ndmods         = dmodIDs.size();
DbgLv(1) << "RmvMod: ndmods" << ndmods;
      double minVari = 99e+10;

      for ( int ii = 0; ii < ndmods; ii++ )
      {  // Scan to identify model in set with lowest variance
DbgLv(1) << "low Vari scan: ii vari meni desc" << ii << dmodVaris.at(ii)
 << dmodMenis.at(ii) << dmodDescs.at(ii).right( 22 );
         if ( dmodVaris.at( ii ) < minVari )
         {
            minVari        = dmodVaris.at( ii );
            dkModx         = ii;
DbgLv(1) << "low Vari scan:   minVari dkModx" << minVari << dkModx;
         }
      }

      // Now, compare the findings for local versus database
      if ( nlmods == ndmods  ||  ( ndmods > 0 && nlmods == 0 ) )
      {
         int    jj      = 0;
         int    kk      = ndmods - 1;
         int    nmatch  = 0;
         int    nmatfo  = 0;

         while ( jj < nlmods )
         {
            if ( dmodGUIDs[ jj ] == lmodGUIDs[ kk ]  &&
                 dmodDescs[ jj ] == lmodDescs[ kk ] )
               nmatch++;

            else if ( dmodGUIDs[ jj ] == lmodGUIDs[ jj ]  &&
                      dmodDescs[ jj ] == lmodDescs[ jj ]  &&
                      dmodGUIDs[ kk ] == lmodGUIDs[ kk ]  &&
                      dmodDescs[ kk ] == lmodDescs[ kk ] )
               nmatfo++;

            jj++;
            kk--;
         }

         if ( nmatch == nlmods )
         {  // As expected, database records are in reverse order
DbgLv(1) << "++local/dbase match, but are in reverse order";
            // Reverse the order of DB records
            jj             = 0;
            kk             = ndmods - 1;
            dkModx         = kk - dkModx;
            while ( jj < kk )
            {
               QString modelID    = dmodIDs  [ jj ];
               QString modelGUID  = dmodGUIDs[ jj ];
               double  variance   = dmodVaris[ jj ];
               double  meniscus   = dmodMenis[ jj ];
               QString descript   = dmodDescs[ jj ];
               dmodIDs  [ jj ]    = dmodIDs  [ kk ];
               dmodGUIDs[ jj ]    = dmodGUIDs[ kk ];
               dmodVaris[ jj ]    = dmodVaris[ kk ];
               dmodMenis[ jj ]    = dmodMenis[ kk ];
               dmodDescs[ jj ]    = dmodDescs[ kk ];
               dmodIDs  [ kk ]    = modelID;
               dmodGUIDs[ kk ]    = modelGUID;
               dmodVaris[ kk ]    = variance;
               dmodMenis[ kk ]    = meniscus;
               dmodDescs[ kk ]    = descript;
               jj++;
               kk--;
            }
         }

         else if ( nmatfo == ndmods  ||  nlmods == 0 )
         {  // Also OK if they match and are in the same order
DbgLv(1) << "++local/dbase match, and in the same order (or local only)";
         }

         else
         {  // Not good if they do not match
DbgLv(1) << "**local/dbase DO NOT MATCH";
DbgLv(1) << "  nmatch nmo ndmods nlms" << nmatch << nmatfo << ndmods << nlmods;
            return;
         }
      }

      else if ( nlmods == 0 )
      {  // It is OK if there are no local records, when DB ones were found
DbgLv(1) << "++only dbase records exist";
      }

      else if ( ndmods == 0 )
      {  // It is OK if there are only local records, when local ones found
DbgLv(1) << "++only local records exist";
      }

      else
      {  // Non-zero local & DB, but they do not match
DbgLv(1) << "**local/dbase DO NOT MATCH in count";
DbgLv(1) << "  nlmods ndmods" << nlmods << ndmods;
         return;
      }
   }

DbgLv(1) << "  nlmods ndmods" << nlmods << ndmods;
   if ( ndmods > 0  ||  nlmods > 0 )
   {  // There are models to scan, so build a list of models,noises to remove
      QStringList     rmodIDs;
      QStringList     rmodDescs;
      QStringList     rmodFnams;
      QStringList     rnoiIDs;
      QStringList     rnoiFnams;
      QStringList     rnoiDescs;
      QStringList     nieDescs;
      QStringList     nieIDs;
      QStringList     nieFnams;
      int             nlrmod = 0;
      int             ndrmod = 0;
      int             nlrnoi = 0;
      int             ndrnoi = 0;
      int             ntmods = ( ndmods > 0 ) ? ndmods : nlmods;
      int             ikModx = ( ndmods > 0 ) ? dkModx : lkModx;
DbgLv(1) << "  ntmods ikModx" << ntmods << ikModx;

      QString modDesc    = "";

      for ( int jj = 0; jj < ntmods; jj++ )
      {  // Build the list of model files and DB ids for removal
         if ( jj != ikModx )
         {
            int itix;
            int irix;
            QString fname;
            QString mDesc;
            QString mID;
            QString tiDesc;
            QString riDesc;
            QString noiID; 
            QString noiFname;

            if ( nlmods > 0 )
            {
               fname  = lmodFnams[ jj ];
               mDesc  = lmodDescs[ jj ];
               nlrmod++;
               if ( ndmods == 0 )
                  mID    = "-1";
            }

            if ( ndmods > 0 )
            {
               mID    = dmodIDs  [ jj ];
               mDesc  = dmodDescs[ jj ];
               ndrmod++;
               if ( nlmods == 0 )
                  fname  = "";
            }
            rmodIDs   << mID;
            rmodFnams << fname;
            rmodDescs << mDesc;

            if ( modDesc.isEmpty() )
            {
               modDesc = mDesc;   // Save 1st model's description
DbgLv(1) << "RmvMod: 1st rmv-mod: jj modDesc" << jj << modDesc;
               // Build noises-in-edit lists for database and local
               noises_in_edit( modDesc, nieDescs, nieIDs, nieFnams );
            }

            tiDesc = QString( mDesc ).replace( ".model", ".ti_noise" );
            riDesc = QString( mDesc ).replace( ".model", ".ri_noise" );
            itix   = nieDescs.indexOf( tiDesc );
            irix   = nieDescs.indexOf( riDesc );

            if ( itix >= 0 )
            {  // There is a TI noise to remove
               noiID    = nieIDs  [ itix ];
               noiFname = nieFnams[ itix ];

               if ( noiID != "-1" )
                  ndrnoi++;

               if ( ! noiFname.isEmpty() )
                  nlrnoi++;
               else
                  noiFname = "";

               rnoiIDs   << noiID;
               rnoiFnams << noiFname;
               rnoiDescs << tiDesc;
            }

            if ( irix >= 0 )
            {  // There is an RI noise to remove
               noiID    = nieIDs  [ irix ];
               noiFname = nieFnams[ irix ];

               if ( noiID != "-1" )
                  ndrnoi++;

               if ( ! noiFname.isEmpty() )
                  nlrnoi++;
               else
                  noiFname = "";

               rnoiIDs   << noiID;
               rnoiFnams << noiFname;
               rnoiDescs << riDesc;
            }
         }
      }

      nlnois             = nlrnoi + ( nlrnoi > nlrmod ? 2 : 1 );
      ndnois             = ndrnoi + ( ndrnoi > ndrmod ? 2 : 1 );
DbgLv(1) << "RmvMod: nlrmod ndrmod nlrnoi ndrnoi"
 << nlrmod << ndrmod << nlrnoi << ndrnoi;
      QString msg = tr( "%1 local model files;\n"
                        "%2 database model files;\n"
                        "%3 local noise files;\n"
                        "%4 database noise files;\n"
                        "have been identified for removal.\n\n"
                        "Do you really want to delete them?" )
         .arg( nlrmod ).arg( ndrmod ).arg( nlrnoi ).arg( ndrnoi );

      int response = QMessageBox::question( this,
            tr( "Remove Models and Noises?" ),
            msg, QMessageBox::Yes, QMessageBox::Cancel );

      if ( response == QMessageBox::Yes )
      {
         US_Passwd pw;
         US_DB2* dbP;
         if ( dkdb_cntrls->db() )  dbP = new US_DB2( pw.getPasswd() );
         QStringList query;
         QString recID;
         QString recFname;
         QString recDesc;
DbgLv(1) << " Remove Models and Noises";
         for ( int ii = 0; ii < rmodIDs.size(); ii++ )
         {
            recID    = rmodIDs  [ ii ];
            recDesc  = rmodDescs[ ii ];
            recFname = rmodFnams[ ii ];
DbgLv(1) << "  Delete: " << recID << recFname.section("/",-1,-1) << recDesc;

            if ( ! recFname.isEmpty() )
            {
               QFile recf( recFname );
               if ( recf.exists() )
               {
                  if ( recf.remove() )
{ DbgLv(1) << "     local file removed"; }
                  else { qDebug() << "*ERROR* removing" << recFname; }
               }
               else qDebug() << "*ERROR* does not exist:" << recFname;
            }

            if ( recID != "-1" )
            {
               query.clear();
               query << "delete_model" << recID;
               int stat = dbP->statusQuery( query );
               if ( stat != 0 )
                  qDebug() << "delete_model error" << stat;
else DbgLv(1) << "     DB record deleted";
            }
         }

         for ( int ii = 0; ii < rnoiIDs.size(); ii++ )
         {
            recID    = rnoiIDs  [ ii ];
            recDesc  = rnoiDescs[ ii ];
            recFname = rnoiFnams[ ii ];
DbgLv(1) << "  Delete: " << recID << recFname.section("/",-1,-1) << recDesc;

            if ( ! recFname.isEmpty() )
            {
               QFile recf( recFname );
               if ( recf.exists() )
               {
                  if ( recf.remove() )
{ DbgLv(1) << "     local file removed"; }
                  else { qDebug() << "*ERROR* removing" << recFname; }
               }
               else qDebug() << "*ERROR* does not exist:" << recFname;
            }

            if ( recID != "-1" )
            {
               query.clear();
               query << "delete_noise" << recID;
               int stat = dbP->statusQuery( query );
               if ( stat != 0 )
                  qDebug() << "delete_model error" << stat;
else DbgLv(1) << "     DB record deleted";
            }
         }
      }
   }

   else
   {  // No models were found!!! (huh!!!)
DbgLv(1) << "**NO local/dbase models-to-remove were found!!!!";
   }

   return;
}

// Create lists of information for noises that match a sample model from a set
void US_FitMeniscus::noises_in_edit( QString modDesc, QStringList& nieDescs,
      QStringList& nieIDs, QStringList& nieFnams )
{
   QString msetBase = modDesc.section( ".", 0, -3 ) + "." + 
                      modDesc.section( ".", -2, -2 ).section( "_", 0, 3 );
   QString srchTlab = msetBase.section( ".", -2, -2 );
   QString srchTrip = srchTlab.left( 1 ) + "." + srchTlab.mid( 1, 1 ) + "." +
                      srchTlab.mid( 2 ) + ".xml";
   QString srchRun  = msetBase.section( ".", 0, -3 ) + "." +
                      msetBase.section( ".", -1, -1 )
                      .section( "_", 0, 0 ).mid( 1 );
DbgLv(1) << "NIE: msetBase" << msetBase;
   QStringList query;
   QString     fname;
   QString     noiID;
   int         nlnois = 0;
   bool        usesDB = dkdb_cntrls->db();

   QStringList noifilt;
   noifilt << "N*.xml";
   QString     noidir   = US_Settings::dataDir() + "/noises";
   QStringList noifiles = QDir( noidir ).entryList(
         noifilt, QDir::Files, QDir::Name );
   noidir               = noidir + "/";
DbgLv(1) << "NIE: noise-files-size" << noifiles.size();

   for ( int ii = 0; ii < noifiles.size(); ii++ )
   {
      QString noiFname   = noifiles.at( ii );
      QString noiPath    = noidir + noiFname;
      US_Noise noise;
      
      if ( noise.load( noiPath ) != US_DB2::OK )
         continue;    // Can't use if can't load

      QString noiDesc    = noise.description;
DbgLv(1) << "NIE:  ii noiDesc" << ii << noiDesc;

      if ( ! noiDesc.startsWith( msetBase ) )
         continue;    // Can't use if not from the model set

      nlnois++;

      nieDescs << noiDesc;
      nieFnams << noiFname;
DbgLv(1) << "NIE:     noiFname" << noiFname;

      if ( ! usesDB )
         nieIDs   << "-1";
   }

   if ( usesDB )
   {
      US_Passwd pw;
      US_DB2 db( pw.getPasswd() );
      QStringList nIDs;
      QString invID = QString::number( US_Settings::us_inv_ID() );

      QStringList edtIDs;
      QStringList edtNams;

      query.clear();
      query << "all_editedDataIDs" << invID;
      db.query( query );

      while( db.next() )
      {
         QString edtID    = db.value( 0 ).toString();
         QString edtName  = db.value( 2 ).toString();

         if ( edtName.startsWith( srchRun )  &&
              edtName.endsWith(   srchTrip ) )
         {
            edtIDs  << edtID;
            edtNams << edtName;
DbgLv(1) << "NIE:  edtID edtName" << edtID << edtName;
         }
      }
DbgLv(1) << "NIE: edtIDs-size" << edtIDs.size();
if ( edtIDs.size() > 0 ) DbgLv(1) << "NIE: edtName0" << edtNams[0];

      query.clear();
      query << "get_noise_desc" << invID;
      db.query( query );

      while( db.next() )
      {
         QString noiID    = db.value( 0 ).toString();
         QString edtID    = db.value( 2 ).toString();
DbgLv(1) << "NIE:  noiID edtID" << noiID << edtID;

         if ( edtIDs.contains( edtID ) )
            nIDs << noiID;
      }
DbgLv(1) << "NIE: nIDs-size" << nIDs.size() << "msetBase" << msetBase;

      for ( int ii = 0; ii < nIDs.size(); ii++ )
      {
         QString noiID      = nIDs[ ii ];
         US_Noise noise;
      
         if ( noise.load( noiID, &db ) != US_DB2::OK )
            continue;    // Can't use if can't load

         QString noiDesc    = noise.description;
DbgLv(1) << "NIE:  ii noiID noiDesc" << ii << noiID << noiDesc;

         if ( ! noiDesc.startsWith( msetBase ) )
            continue;    // Can't use if not from the model set

         nieIDs   << noiID;

         if ( nlnois == 0 )
         {
            nieFnams << "";
            nieDescs << noiDesc;
         }
      }
   }
DbgLv(1) << "NIE: usesDB" << usesDB << "nlnois" << nlnois
 << "nieDescs-size" << nieDescs.size() << "nieIDs-size" << nieIDs.size();

   return;
}

