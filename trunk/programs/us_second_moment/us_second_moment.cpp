//! \file us_second_moment.cpp

#include <QApplication>

#include "us_second_moment.h"
#include "us_license_t.h"
#include "us_license.h"
#include "us_settings.h"
#include "us_gui_settings.h"

//! \brief Main program. Loads translators and starts
//         the class US_Convert.

int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );

   #include "main1.inc"

   // License is OK.  Start up.
   
   US_SecondMoment w;
   w.show();                   //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication
}

US_SecondMoment::US_SecondMoment() : US_AnalysisBase2()
{
   setWindowTitle( tr( "Second Moment Analysis" ) );

   smPoints   = NULL;
   smSeconds  = NULL;
   te_results = NULL;

   connect( pb_help,  SIGNAL( clicked() ), SLOT( help() ) );
   connect( pb_view,  SIGNAL( clicked() ), SLOT( view() ) );
   connect( pb_save,  SIGNAL( clicked() ), SLOT( save() ) );
}

void US_SecondMoment::data_plot( void )
{
   US_AnalysisBase2::data_plot();

   //time_correction = US_Math::time_correction( dataList );

   int                     index  = lw_triples->currentRow();
   US_DataIO2::EditedData* d      = &dataList[ index ];

   int     scanCount   = d->scanData.size();
   int     exclude     = 0;
   double  boundaryPct = ct_boundaryPercent->value() / 100.0;
   double  positionPct = ct_boundaryPos    ->value() / 100.0;
   double  baseline    = calc_baseline();

   for ( int i = 0; i < scanCount; i++ )
   {
      if ( excludedScans.contains( i ) ) continue;
      
      double range  = d->scanData[ i ].plateau - baseline;
      double test_y = baseline + range * positionPct;
      
      if ( d->scanData[ i ].readings[ 0 ].value > test_y ) exclude++;
   }

   le_skipped->setText( QString::number( exclude ) );

   // Draw plot
   data_plot1->clear();
   us_grid( data_plot1 );

   data_plot1->setTitle( tr( "Run " ) + d->runID + tr( ": Cell " ) + d->cell
             + " (" + d->wavelength + tr( " nm)\nSecond Moment Plot" ) );

   data_plot1->setAxisTitle( QwtPlot::xBottom, tr( "Scan Number" ) );
   data_plot1->setAxisTitle( QwtPlot::yLeft  , 
         tr( "Corrected Sed. Coeff. (1e-13 s)" ) );

   if ( smPoints  != NULL ) delete [] smPoints;
   if ( smSeconds != NULL ) delete [] smSeconds;

   smPoints  = new double[ scanCount ];
   smSeconds = new double[ scanCount ];

   // Calculate the 2nd moment
   for ( int i = 0; i < scanCount; i++ )
   {
      double sum1  = 0.0;
      double sum2  = 0.0;
      int    count = 0;

      // The span is the boundary portion that is going to be analyzed (in
      // percent)

      double range  = ( d->scanData[ i ].plateau - baseline ) * boundaryPct;
      double test_y = range * positionPct;

      while ( d->scanData[ i ].readings[ count ].value - baseline < test_y ) 
         count++;

      int points = d->scanData[ i ].readings.size();

      if ( count == 0 ) count = 1;

      while ( count < points )
      {   
         double value  = d->scanData[ i ].readings[ count ].value - baseline;
         double radius = d->x[ count ].radius;

         if ( value >= test_y + range ) break;
      
         double v0 = d->scanData[ i ].readings[ count - 1 ].value - baseline;
         double dC = value - v0;

         sum1 += dC * sq( radius );
         sum2 += dC;
         count++;
      }

      smPoints [ i ] = sqrt( sum1 / sum2 ); // second moment points in cm

      double omega = d->scanData[ i ].rpm * M_PI / 30.0;

      // second moment s
      smSeconds[ i ] = 
         1.0e13 * solution.s20w_correction * log( smPoints[ i ] / d->meniscus ) /
         ( sq( omega ) * ( d->scanData[ i ].seconds - time_correction ) );
   }

   QVector< double > x( scanCount );
   QVector< double > y( scanCount );
   
   // Sedimentation coefficients from all scans that have not cleared the
   // meniscus form a separate plot that will be plotted in red, and will not
   // be included in the line fit:
    
   QwtPlotCurve* curve;
   QwtSymbol     sym;
   
   int count = 0;

   // Curve 1
   for ( int i = 0; i < exclude; i++ )
   {
      if ( excludedScans.contains( i ) ) continue;
      
      x[ count ] = (double)( count + 1 );
      y[ count ] = smSeconds[ i ];
      count++;
   }

   curve = us_curve( data_plot1, tr( "Non-cleared Sedimentation Coefficients" ) );

   sym.setStyle( QwtSymbol::Ellipse );
   sym.setPen  ( QPen( Qt::white ) );
   sym.setBrush( QBrush( Qt::red ) );
   sym.setSize ( 8 );
   
   curve->setStyle ( QwtPlotCurve::NoCurve );
   curve->setSymbol( sym );
   curve->setData  ( x.data(), y.data(), count );

   // Curve 2
   count          = 0;
   double average = 0.0;

   for ( int i = exclude; i < scanCount; i++ )
   {
      if ( excludedScans.contains( i ) ) continue;
      
      x[ count ] = (double)( count + 1 + exclude );
      y[ count ] = smSeconds[ i ];
      average   += smSeconds[ i ];
      count++;
   }

   average_2nd = (count > 0 ) ? average / count : 0.0;

   sym.setPen  ( QPen  ( Qt::blue  ) );
   sym.setBrush( QBrush( Qt::white ) );
   
   curve = us_curve( data_plot1, tr( "Cleared Sedimentation Coefficients" ) );
   curve->setSymbol( sym );
   curve->setData( x.data(), y.data(), count );
   
   // Curve 3
   x[ 0 ] = 0.0;
   x[ 1 ] = (double)( scanCount - excludedScans.size() );
   y[ 0 ] = average_2nd;
   y[ 1 ] = average_2nd;

   if ( count > 0 )
   {
      curve = us_curve( data_plot1, tr( "Average" ) );
      curve->setPen( QPen( Qt::green ) );
      curve->setData( x.data(), y.data(), 2 );
   }

   data_plot1->setAxisScale   ( QwtPlot::xBottom, 0.0, x[ 1 ] + 0.25, 0.0 );
   data_plot1->setAxisMaxMinor( QwtPlot::xBottom, 0 );

   // Mark excluded
   int from = (int)ct_from->value();
   int to   = (int)ct_to  ->value();
   
   if ( to > 0 )
   {
      int index = 0;

      for ( int i = 0; i < scanCount; i++ )
      {
         if ( excludedScans.contains( i ) ) continue;

         index++;
         if ( index < from                ) continue;
         if ( index >  to                 ) break;

         x[ 0 ] = index;
         x[ 1 ] = x[ 0 ];
         y[ 0 ] = smSeconds[ i ] + 0.5;
         y[ 1 ] = smSeconds[ i ] - 0.5;;

         curve = us_curve( data_plot1, 
               tr( "Scan %1 Exclude Marker" ).arg( index + 1 ) );

         curve->setPen( QPen( QBrush( Qt::red ), 1.0 ) );
         curve->setData( x.data(), y.data(), 2 );
      }
   }

   data_plot1->replot();
}

void US_SecondMoment::exclude( void )
{
   US_AnalysisBase2::exclude();
   pb_reset_exclude->setEnabled( true );
}

void US_SecondMoment::write_report( QTextStream& ts )
{
   int                     index  = lw_triples->currentRow();
   US_DataIO2::EditedData* edata  = &dataList[ index ];

   QString sm_results = 
        table_row( tr( "Average Second Moment S: " ),
                   QString::number( average_2nd, 'f', 5 ) + " s * 10e-13" );

   ts << html_header( QString( "US_Second_Moment" ),
                      QString( "Second Moment Analysis" ),
                      edata );
   ts << run_details();
   ts << hydrodynamics();
   ts << analysis( sm_results );
   ts << scan_info();
   ts << indent( 2 ) + "</body>\n</html>\n";
}

void US_SecondMoment::view( void )
{
   // Write main report as string
   QString rtext;
   QTextStream ts( &rtext );
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

   te_results->e->setHtml( rtext );
   te_results->show();
}

void US_SecondMoment::save( void )
{
   int                     index  = lw_triples->currentRow();
   US_DataIO2::EditedData* d      = &dataList[ index ];
   QString                 dir    = US_Settings::reportDir();

   if ( ! mkdir( dir, d->runID ) ) return;

   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
   // Note: d->runID is both directory and first segment of file name
   QString filebase = dir + "/" + d->runID + "/secmo."
      + QString( triples.at( index ) ).replace( " / ", "" ) + ".";
   
   QString plot1File = filebase + "2ndmoment.svg";
   QString plot2File = filebase + "velocity.svg";
   QString textFile  = filebase + "2ndmoment.csv";
   QString htmlFile  = filebase + "report.html";

   // Write main report
   QFile reportf( htmlFile );

   if ( ! reportf.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
   {
      QMessageBox::warning( this,
            tr( "IO Error" ),
            tr( "Could not open\n" ) + htmlFile + "\n" +
                tr( "\nfor writing" ) );
      QApplication::restoreOverrideCursor();
      return;
   }

   QTextStream ts( &reportf );
   write_report( ts );
   reportf.close();

   // Write plots
   write_plot( plot1File, data_plot1 );
   write_plot( plot2File, data_plot2 );
   
   // Write moment data
   QFile sm_data( textFile );
   if ( ! sm_data.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
   {
      QMessageBox::warning( this,
            tr( "IO Error" ),
            tr( "Could not open\n" ) + textFile + "\n" +
                tr( "\nfor writing" ) );
      QApplication::restoreOverrideCursor();
      return;
   }

   const QString sep( "\",\"" );
   const QString quo( "\"" );
   const QString eln( "\"\n" );
   QTextStream ts_data( &sm_data );

   int scanCount = d->scanData.size();
   int excludes  = le_skipped->text().toInt();
   
   if ( excludes == scanCount )
      ts_data << "No valid scans\n";
   else
   {
      ts_data << quo << "Count" << sep << "Points" << sep << "Seconds" << eln;
      int count = 1;
      for ( int i = excludes; i < scanCount; i++ )
      {
         if ( excludedScans.contains( i ) ) continue;

         QString strK = QString::number( count                  ).simplified();
         QString strP = QString::number( smPoints [ i ], 'f', 5 ).simplified();
         QString strS = QString::number( smSeconds[ i ], 'f', 5 ).simplified();
         ts_data << quo << strK << sep << strP << sep << strS << eln;
         count++;
      }
   }

   sm_data.close();
   QStringList repfiles;
   repfiles << htmlFile << plot1File << plot2File << textFile;

   // Tell user
   htmlFile  = htmlFile .mid( htmlFile .lastIndexOf( "/" ) + 1 );
   plot1File = plot1File.mid( plot1File.lastIndexOf( "/" ) + 1 );
   plot2File = plot2File.mid( plot2File.lastIndexOf( "/" ) + 1 );
   textFile  = textFile .mid( textFile .lastIndexOf( "/" ) + 1 );

   QString wmsg = tr( "Wrote:\n  " ) + htmlFile  + "\n  "
      + plot1File + "\n  " + plot2File + "\n  " + textFile;

   if ( disk_controls->db() )
   {  // Write report files to the database
      reportFilesToDB( repfiles );

      wmsg += tr( "\n\nReport files were also saved to the database." );
   }

   QApplication::restoreOverrideCursor();
   QMessageBox::warning( this, tr( "Success" ), wmsg );
}

