//! \file us_dcdt.cpp

#include <QApplication>
#include <QtSvg>

#include "us_dcdt.h"
#include "us_license_t.h"
#include "us_license.h"
#include "us_settings.h"
#include "us_gui_settings.h"

//! \brief Main program for us_dcdt. Loads translators and starts
//         the class US_Dcdt.

int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );

   #include "main1.inc"

   // License is OK.  Start up.
   
   US_Dcdt w;
   w.show();                   //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication
}

US_Dcdt::US_Dcdt() : US_AnalysisBase2()
{
   setWindowTitle( tr( "Time Derivative - dC/dt Analysis" ) );

   te_results = NULL;
   sMax       = 1000.0;

   //QLabel*       lb_aux    = us_banner( tr( "dC/dt Auxiliary Controls" ) );
   QLabel*       lb_sValue = us_label( tr( "S-value Cutoff:" ) );
   ct_sValue               = us_counter( 3, 0, 20, 20 );
   ct_sValue->setStep( 0.1 );
   connect( ct_sValue, SIGNAL( valueChanged ( double ) ), 
                       SLOT  ( sMaxChanged  ( double ) ) );
   
   QLabel*       lb_graph  = us_label( tr( "Graph Selection" ) );
   lb_graph->setAlignment ( Qt::AlignCenter );

   QGridLayout* rb_layout1 = us_radiobutton( tr( "x:radius" ), rb_radius,false);
   QGridLayout* rb_layout2 = us_radiobutton( tr( "x:S" ),       rb_sed, false );
   QGridLayout* rb_layout3 = us_radiobutton( tr( "Average S" ), rb_avg, true );

   QButtonGroup* group = new QButtonGroup( this );
   group->addButton( rb_radius, 0 );
   group->addButton( rb_sed   , 1 );
   group->addButton( rb_avg   , 2 );
   graphType = 2;
   connect( group, SIGNAL( buttonClicked( int ) ), SLOT( set_graph( int ) ) );

   QBoxLayout* rb_layout0 = new QHBoxLayout();
   rb_layout0->addLayout( rb_layout1 );
   rb_layout0->addLayout( rb_layout2 );
   rb_layout0->addLayout( rb_layout3 );

   int row = 5;
   QLabel*       lb_bPos   = us_label( tr( "Boundary Pos (%):" ) );
   controlsLayout->addWidget( lb_bPos,        row,   0, 1, 1 );
   controlsLayout->addWidget( ct_boundaryPos, row++, 1, 1, 1 );

   //controlsLayout->addWidget( lb_aux,     row++, 0, 1, 2 );

   controlsLayout->addWidget( lb_sValue,  row,   0, 1, 1 );
   controlsLayout->addWidget( ct_sValue,  row++, 1, 1, 1 );
   
   controlsLayout->addWidget( lb_graph,   row++, 0, 1, 3 );
   controlsLayout->addLayout( rb_layout0, row++, 0, 1, 3 );

   connect( pb_help,  SIGNAL( clicked() ), SLOT( help() ) );
   connect( pb_view,  SIGNAL( clicked() ), SLOT( view() ) );
   connect( pb_save,  SIGNAL( clicked() ), SLOT( save() ) );

   ct_boundaryPos    ->setMaxValue( 90.0 );
   ct_boundaryPercent->disconnect();
   ct_boundaryPercent->setValue   ( 9000.0 );
   ct_boundaryPercent->setEnabled ( false );
   ct_boundaryPercent->setVisible ( false );

   qApp->processEvents();
}

void US_Dcdt::exclude( void )
{
   if ( ! dataLoaded ) return;
   US_AnalysisBase2::exclude();
   
   data_plot();
}

void US_Dcdt::reset_excludes( void )
{
   if ( ! dataLoaded ) return;
   US_AnalysisBase2::reset_excludes();
   
   data_plot();
}

void US_Dcdt::reset( void )
{
   if ( ! dataLoaded ) return;

   US_AnalysisBase2::reset();

   ct_sValue->disconnect();
   ct_sValue->setValue( sMax );
   rb_avg   ->click();
   connect( ct_sValue, SIGNAL( valueChanged ( double ) ), 
                       SLOT  ( sMaxChanged  ( double ) ) );
   qApp->processEvents();
}

void US_Dcdt::sMaxChanged( double /* value */ )
{
   data_plot();
}

void US_Dcdt::set_graph( int button )
{
   if ( graphType == button ) return;

   graphType = button;
   data_plot();
}

void US_Dcdt::data_plot( void )
{
   US_AnalysisBase2::data_plot();

   int                     index  = lw_triples->currentRow();
   US_DataIO2::EditedData* d      = &dataList[ index ];

   int     scanCount   = d->scanData.size();
   int     skipped     = 0;
   double  positionPct = ct_boundaryPos    ->value() / 100.0;
   double  baseline    = calc_baseline();
/*
   for ( int i = 0; i < scanCount; i++ )
   {
      if ( excludedScans.contains( i ) ) continue;
      
      double range  = d->scanData[ i ].plateau - baseline;
      double test_y = baseline + range * positionPct;
      
      if ( d->scanData[ i ].readings[ 0 ].value > test_y ) skipped++;
   }
*/

   le_skipped->setText( QString::number( skipped ) );

   if ( scanCount - skipped - excludedScans.count() < 4 )
   {
      QMessageBox::warning( this,
            tr( "Attention" ),
            tr( "You must have at least four non-skipped scans "
                "for this analysis." ) );
      return;
   }

   int points = d->scanData[ 0 ].readings.size();

   // Create the new arrays
   dcdt   .clear();
   sValues.clear();
   avgDcdt.resize( points );
   avgS   .resize( points );

   previousScanCount = scanCount;

   for ( int i = 0; i < scanCount; i++ )
   {
      dcdt    << QVector< double >( points );
      sValues << QVector< double >( points ); 
   }

   arraySizes.fill( 0, scanCount );
   arrayStart.fill( 0, scanCount );

   // Calculate dcdt and sValues
   while ( excludedScans.contains( skipped ) ) skipped++;

   int    previous = skipped;
   int    count    = 0;
   double s_max    = 0.0;

   for ( int i = skipped + 1; i < scanCount; i++ )
   {
      if ( excludedScans.contains( i ) ) continue;

      US_DataIO2::Scan* thisScan = &d->scanData[ i ];
      US_DataIO2::Scan* prevScan = &d->scanData[ previous ];

      // These limits are for thisScan only:
      double range       = thisScan->plateau - baseline;
      double lower_limit = baseline + range * positionPct;
      double dt          = thisScan->seconds - prevScan->seconds;
      double plateau     = thisScan->plateau;
      double prevPlateau = prevScan->plateau;

      double meniscus    = d->meniscus;
      double omega       = thisScan->rpm * M_PI / 30.0;

      int    size        = 0;
      bool   started     = false;

      for ( int j = 0; j < points; j++ )
      {
         double currentV  = thisScan->readings[ j ].value;
         double previousV = prevScan->readings[ j ].value;

         if ( currentV < lower_limit ) continue;

         if ( ! started )
         {
            started = true;
            arrayStart[ count ] = j;
         }

         double dC             = previousV / prevPlateau - currentV / plateau;
         dcdt[ count ][ size ] = dC / dt;
         double radius         = d->radius( j );

         sValues[ count ][ size ] = 
            solution.s20w_correction * 1.0e13 * log( radius / meniscus ) /
            ( sq( omega ) * ( prevScan->seconds + dt / 2.0 ) );

         s_max = max( s_max, sValues[ count ][ size ] );

         size++;
      }

      arraySizes[ count ] = size;

      count++;
      previous = i;
   }

   ct_sValue->setMaxValue( ceil( s_max ) );

   if ( s_max < sMax )
   {
      if ( ct_sValue->value() > s_max )
      {
         ct_sValue->disconnect();
         ct_sValue->setValue( ceil( s_max ) );

         connect( ct_sValue, SIGNAL( valueChanged ( double ) ), 
                             SLOT  ( sMaxChanged  ( double ) ) );
         qApp->processEvents();
      }

      sMax = s_max;
   }

   double s_min = 9.9e10;

   for ( int i = 0; i < count; i++ )
   {
      for ( int j = 0; j < arraySizes[ i ]; j++ )
      {
         s_min = min( s_min, sValues[ i ][ j ] );
      }
   }

   // Figure the total points to plot
   double increment = ( s_max - s_min ) / arrayLength;

   // Assign new equally spaced s-values for the x-axis
   for ( int i = 0; i < arrayLength; i++ ) 
      avgS[ i ] = s_min + i * increment;

   // For each new s value, find the corresponding g*(s) through
   // linear interpolation and add up, then average by the number of
   // total scans included.
   
   for ( int j = 0; j < arrayLength; j++ ) 
   {
      avgDcdt[ j ] = 0.0;

      for ( int i = 0; i < count; i++ )
      {
         int k = 1;

         // Find index where svalue for the scan first exceeds 
         // the point on the x-axis
         while ( sValues[ i ][ k ] < avgS[ j ] ) 
         {
            k++;
            if ( k == arraySizes[ i ] ) goto next; // Skip rest of scans
         }

         // Interpolate and apply y = mx + b
         double m = ( dcdt   [ i ][ k ] - dcdt   [ i ][ k - 1 ] ) /
                    ( sValues[ i ][ k ] - sValues[ i ][ k - 1 ] );
         
         double b = dcdt[ i ][ k ] - m * sValues[ i ][ k ];
         
         avgDcdt[ j ] += m * avgS[ j ] + b;
      }

next: avgDcdt[ j ] /= ( count - 1 ); 
   }

   // Draw plot
   data_plot1->clear();
   us_grid( data_plot1 );

   data_plot1->setTitle( tr( "Time Derivative (dC/dt)\n" ) +
         tr( "Run " ) + d->runID + tr( ": Cell " ) + d->cell
         + " (" + d->wavelength + tr( " nm)" ) );

   data_plot1->setAxisTitle( QwtPlot::yLeft  , tr( "g<sup>*</sup>(s)" ) );

   QVector< double > x( points );
   QwtPlotCurve*     curve;
   QwtText           title = QwtText( tr( "<b>Sedimentation Coefficient x "
                                          "10<sup>13</sup> sec</b>" ) );
         
   data_plot1->setAxisTitle( QwtPlot::xBottom, title );

   // Draw the desired plot
   switch ( graphType )
   {
      case 0:  // Radius Plot
         data_plot1->setAxisTitle( QwtPlot::xBottom, tr( "Radius (cm)" ) );

         for ( int i = 0; i < count; i++ )
         {
            int size = 0;
            int end  = arraySizes[ i ] + arrayStart[ i ];

            for ( int j = arrayStart[ i ]; j < end; j++ ) 
               x[ size++ ] = d->x[ j ].radius;

            curve = us_curve( data_plot1, 
                  tr( "Scan " ) + QString::number( i + 1 ) );

            curve->setData( x.data(), dcdt[ i ].data(), arraySizes[ i ] );
         }

         data_plot1->setAxisAutoScale( QwtPlot::xBottom );
         break;

      case 1:  // S-value plot
         for ( int i = 0; i < count; i++ )
         {
            curve = us_curve( data_plot1, 
                  tr( "Scan " ) + QString::number( i + 1 ) );

            curve->setData( sValues[ i ].data(), dcdt[ i ].data(), 
                            arraySizes[ i ] );
         }
         
         data_plot1->setAxisScale( QwtPlot::xBottom, 0.0, ct_sValue->value() );
         break;

      case 2:  // Average dC/dt or g*(s) plot
         data_plot1->setAxisTitle( QwtPlot::yLeft, 
                                   tr( "Average g<sup>*</sup>(s)" ) );

         curve = us_curve( data_plot1, tr( "Average g*(s)" ) );
         curve->setData( avgS.data(), avgDcdt.data(), arrayLength );

         data_plot1->setAxisScale( QwtPlot::xBottom, 0.0, ct_sValue->value() );
         break;
   }

   qApp->processEvents();
   data_plot1->replot();
}

// Write report html to stream
void US_Dcdt::write_report( QTextStream& ts )
{
   int                     index  = lw_triples->currentRow();
   US_DataIO2::EditedData* edata  = &dataList[ index ];

   QString title = "US_dCdt";
   QString head1 = tr( "Time - Derivative (dC/dt) Analysis" );

   ts << html_header( title, head1, edata );
   ts << run_details();
   ts << hydrodynamics();
   ts << analysis( "" );
   ts << scan_info();
   ts << indent( 2 ) + "</body>\n</html>\n";
}

void US_Dcdt::view( void )
{
   // Create report string
   QString mtext;
   QTextStream ts( &mtext );

   write_report( ts );

   // Create US_Editor and display report
   if ( te_results == NULL )
   {
      te_results = new US_Editor( US_Editor::DEFAULT, true, QString(), this );
      te_results->resize( 600, 700 );
      QPoint p = g.global_position();
      te_results->move( p.x() + 30, p.y() + 30 );
      te_results->e->setFont( QFont( US_GuiSettings::fontFamily(),
                                     US_GuiSettings::fontSize() ) );
   }

   te_results->e->setHtml( mtext );
   te_results->show();
}

void US_Dcdt::save( void )
{
   int                     index  = lw_triples->currentRow();
   US_DataIO2::EditedData* d      = &dataList[ index ];
   QString                 dir    = US_Settings::reportDir();

   if ( ! mkdir( dir, d->runID ) ) return;

   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
   // Note: d->runID is both directory and first segment of file name
   QString filebase = dir + "/" + d->runID + "/dcdt."
      + QString( triples.at( index ) ).replace( " / ", "" ) + ".";
   
   QString htmlFile  = filebase + "report.html";
   QString plot1File = filebase + "velocity.svg";
   QString plot2File = filebase + "x-to-radius.svg";
   QString plot3File = filebase + "x-to-S.svg";
   QString plot4File = filebase + "average-S.svg";
   QString textFile  = filebase + "average-S.dat";

   // Write main report file
   // Write report
   QFile f_rep( htmlFile );

   if ( ! f_rep.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
   {
      QMessageBox::warning( this,
            tr( "IO Error" ),
            tr( "Could not open\n" ) + htmlFile + "\n" +
                tr( "\nfor writing" ) );
      QApplication::restoreOverrideCursor();
      return;
   }

   QTextStream ts( &f_rep );
   write_report( ts );
   f_rep.close();

   // Save current plot type
   int saveGraph = graphType;

   // Write plots, starting with velocity scans
   data_plot();
   write_plot( plot1File, data_plot2 );

   // Set current plot type to radius
   graphType = 0;
   data_plot();
   write_plot( plot2File, data_plot1 );

   // Set current plot type to Svalues
   graphType = 1;
   data_plot();
   write_plot( plot3File, data_plot1 );

   // Set current plot type to average-S
   graphType = 2;
   data_plot();
   write_plot( plot4File, data_plot1 );
      
   // Restore plot type
   graphType = saveGraph;

   // Write dcdt analysis average data
   QFile dcdt_data( textFile );
   
   if ( ! dcdt_data.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
   {
      QMessageBox::warning( this,
            tr( "IO Error" ),
            tr( "Could not open\n" ) + textFile + "\n" +
                tr( "\nfor writing" ) );
      QApplication::restoreOverrideCursor();
      return;
   }

   QTextStream ts_data( &dcdt_data );

   for ( int i = 0; i < arrayLength; i++ )
      ts_data << avgS   [ i ] << "\t" << avgDcdt[ i ] << endl;

   dcdt_data.close();

   QStringList repfiles;
   repfiles << htmlFile << plot1File << plot2File << plot3File 
            << plot4File << textFile;

   // Tell user
   htmlFile  = htmlFile .mid( htmlFile .lastIndexOf( "/" ) + 1 );
   plot1File = plot1File.mid( plot1File.lastIndexOf( "/" ) + 1 );
   plot2File = plot2File.mid( plot2File.lastIndexOf( "/" ) + 1 );
   plot3File = plot3File.mid( plot3File.lastIndexOf( "/" ) + 1 );
   plot4File = plot4File.mid( plot4File.lastIndexOf( "/" ) + 1 );
   textFile  = textFile .mid( textFile .lastIndexOf( "/" ) + 1 );

   QString     wmsg = tr( "Wrote:\n  " )
      + htmlFile  + "\n  " + plot1File + "\n  " + plot2File + "\n  "
      + plot3File + "\n  " + plot4File + "\n  " + textFile;

   if ( disk_controls->db() )
   {  // Write report files also to the database
      reportFilesToDB( repfiles );

      wmsg += tr( "\n\nReport files were also saved to the database." );
   }

   QApplication::restoreOverrideCursor();
   QMessageBox::warning( this, tr( "Success" ), wmsg );
}

