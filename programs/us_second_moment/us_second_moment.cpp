//! \file us_second_moment.cpp

#include <QApplication>

#include "us_second_moment.h"
#include "us_license_t.h"
#include "us_license.h"
#include "us_math.h"

//! \brief Main program for us_convert. Loads translators and starts
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

US_SecondMoment::US_SecondMoment() : US_AnalysisBase()
{
   setWindowTitle( tr( "Second Moment Analysis" ) );

   smPoints   = NULL;
   smSeconds  = NULL;
   te_results = NULL;

   connect( pb_help,  SIGNAL( clicked() ), SLOT( help() ) );
   connect( pb_view,  SIGNAL( clicked() ), SLOT( view() ) );
   connect( pb_save,  SIGNAL( clicked() ), SLOT( save() ) );
}

US_SecondMoment::~US_SecondMoment()
{
   if ( smPoints   != NULL ) delete [] smPoints;
   if ( smSeconds  != NULL ) delete [] smSeconds;
   if ( te_results != NULL ) 
   {
      qDebug() << "Closing te_results";
      te_results->close();
   }
}

void US_SecondMoment::data_plot( void )
{
   US_AnalysisBase::data_plot();

   int                    index  = lw_triples->currentRow();
   US_DataIO::editedData* d      = &dataList[ index ];

   int    scanCount = d->scanData.size();
   int    exclude   = 0;
   double position  = ct_boundaryPos    ->value() / 100.0;
   double range     = ct_boundaryPercent->value() / 100.0;

   for ( int i = 0; i < scanCount; i++ )
   {
      if ( excludedScans.contains( i ) ) continue;
      
      double test_y = d->scanData[ i ].plateau * position;
      
      if ( d->scanData[ i ].readings[ 0 ].value > test_y ) exclude++;
   }

   le_skipped->setText( QString::number( exclude ) );


   // Draw plot
   data_plot1->clear();
   us_grid( data_plot1 );

   data_plot1->setTitle( tr( "Run " ) + d->runID + tr( ": Cell " ) + d->cell
             + " (" + d->wavelength + tr( " nm) - Second Moment Plot" ) );

   data_plot1->setAxisTitle( QwtPlot::xBottom, tr( "Scan Number" ) );
   data_plot1->setAxisTitle( QwtPlot::yLeft  , 
         tr( "Corrected Sed. Coeff. (1e-13 s)" ) );

   // Calculate solution parameters
   struct solution_data solution;

   solution.density   = le_density  ->text().toDouble();
   solution.viscosity = le_viscosity->text().toDouble();
   solution.vbar      = le_vbar     ->text().toDouble();
   
   double sum = 0;
   int    count;
   
   for ( int i = 0; i < scanCount; i++ ) sum += d->scanData[ i ].temperature;
   
   double avgTemp  = sum / scanCount;
   solution.vbar20 = US_Math::adjust_vbar( solution.vbar, avgTemp );
   US_Math::data_correction( avgTemp, solution );

   if ( smPoints  != NULL ) delete [] smPoints;
   if ( smSeconds != NULL ) delete [] smSeconds;

   smPoints  = new double[ scanCount ];
   smSeconds = new double[ scanCount ];

   // Calculate the 2nd moment
   for ( int i = 0; i < scanCount; i++ )
   {
      double sum1  = 0.0;
      double sum2  = 0.0;
      count        = 0;

      // The span is the boundary portion that is going to be analyzed (in
      // percent)

      double span   = d->scanData[ i ].plateau * range;
      double test_y = d->scanData[ i ].plateau * position;

      while ( d->scanData[ i ].readings[ count ].value < test_y ) count++;

      int points = d->scanData[ i ].readings.size();

      while ( count < points )
      {   
         double value  = d->scanData[ i ].readings[ count ].value;
         double radius = d->scanData[ i ].readings[ count ].d.radius;
         
         if ( value >= test_y + span ) break;
      
         sum1 += value * sq( radius );
         sum2 += value;
         count++;
      }

      smPoints [ i ] = sqrt( sum1 / sum2 ); // second moment points in cm
      
      double omega = d->scanData[ i ].rpm * M_PI / 30.0;

      // second moment s
      smSeconds[ i ] = 1.0e13 * solution.correction            * 
                       log( smPoints[ i ] / d->meniscus )      /
                       ( sq( omega ) * d->scanData[ i ].seconds );
   }

   double* x = new double[ scanCount ];
   double* y = new double[ scanCount ];
   
   // Sedimentation coefficients from all scans that have not cleared the
   // meniscus form a separate plot that will be plotted in red, and will not
   // be included in the line fit:
    
   QwtPlotCurve* curve;
   QwtSymbol     sym;
   
   count = 0;
   
   // Curve 1
   for ( int i = 0; i < exclude; i++ )
   {
      if ( excludedScans.contains( i ) ) continue;
      
      x[ count ] = (double)( i + 1 );
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
   curve->setData  ( x, y, count );

   // Curve 2
   count          = 0;
   double average = 0.0;

   for ( int i = exclude; i < scanCount; i++ )
   {
      if ( excludedScans.contains( i ) ) continue;
      
      x[ count ] = (double)( i + 1 );
      y[ count ] = smSeconds[ i ];
      average   += smSeconds[ i ];
      count++;
   }

   sym.setPen  ( QPen  ( Qt::blue  ) );
   sym.setBrush( QBrush( Qt::white ) );
   
   curve = us_curve( data_plot1, tr( "Cleared Sedimentation Coefficients" ) );
   curve->setSymbol( sym );
   curve->setData( x, y, count );
   
   // Curve 3

   x[ 0 ] = 0.0;
   x[ 1 ] = (double)( scanCount - excludedScans.size() );
   y[ 0 ] = average / count;
   y[ 1 ] = y[ 0 ];

   curve = us_curve( data_plot1, tr( "Average" ) );
   curve->setPen( QPen( Qt::green ) );
   curve->setData( x, y, 2 );

   data_plot1->setAxisScale   ( QwtPlot::xBottom, 0.0, x[ 1 ] + 0.25, 1.0 );
   data_plot1->setAxisMaxMinor( QwtPlot::xBottom, 0 );
   data_plot1->replot();

   delete [] x;
   delete [] y;
}

void US_SecondMoment::reset( void )
{
   US_AnalysisBase::reset();

}



void US_SecondMoment::view( void )
{
   // Create US_Editor
   if ( te_results == NULL )
   {
      qDebug() << "Creating US_Editor";
      te_results = new US_Editor( US_Editor::DEFAULT, true );
      connect( te_results, SIGNAL( destroyed( QObject* ) ),
                           SLOT  ( edit_done( QObject* ) ) );
      te_results->resize( 400, 300 );
      QPoint p = g.global_position();
      te_results->move( p.x() + 30, p.y() + 30 );

      te_results->show();
   }

   // Add results to window
   te_results->e->setPlainText( results() );
}

void US_SecondMoment::save( void )
{
   // Write data
   // Write results
   // Tell user

}

void US_SecondMoment::write_data( void )
{
}

QString US_SecondMoment::results( void )
{
   QString s = "testing";


   return s;

}
