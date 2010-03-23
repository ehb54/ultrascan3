//! \file us_fit_meniscus.cpp
#include "us_fit_meniscus.h"
#include "us_gui_settings.h"
#include "us_math.h"
#include "us_matrix.h"

#include "qwt_plot_marker.h"

US_FitMeniscus::US_FitMeniscus() : US_Widgets()
{
   setWindowTitle( tr( "Fit Meniscus from 2DSA Data" ) );
   setPalette( US_GuiSettings::frameColor() );

   QGridLayout* main = new QGridLayout( this );
   main->setSpacing         ( 2 );
   main->setContentsMargins ( 2, 2, 2, 2 );

   int row = 0;

   te_data = new US_Editor( US_Editor::LOAD, false );
   connect( te_data, SIGNAL( US_EditorLoadComplete() ), 
                     SLOT  ( plot_data()             ) );
   
   QFontMetrics fm( QFont( US_GuiSettings::fontFamily(), 
                           US_GuiSettings::fontSize()   ) );
   
   te_data->setMinimumHeight( fm.height() * 20 );
   te_data->setFixedWidth ( fm.width( '0' ) * 22 );

   main->addWidget( te_data, row, 0, 20, 1 );

   QBoxLayout* plot = new US_Plot( meniscus_plot, 
         tr( "Meniscus Fit" ),
         tr( "Radius" ), tr( "2DSA Meniscus RMSD Value" ) );
   
   us_grid( meniscus_plot );
   
   meniscus_plot->setMinimumSize( 400, 400 );
   meniscus_plot->setAxisScale( QwtPlot::xBottom, 5.7, 6.8 );

   main->addLayout( plot, row, 1, 20, 1 );
   row += 20;

   QBoxLayout* misc = new QHBoxLayout;

   QLabel* lb_order = us_label( tr( "Fit Order:" ) );
   misc->addWidget( lb_order );

   sb_order = new QSpinBox();
   sb_order->setRange( 2, 9 );
   sb_order->setValue( 2 );
   sb_order->setPalette( US_GuiSettings::editColor() );
   connect( sb_order, SIGNAL( valueChanged( int ) ), SLOT( plot_data( int ) ) );
   misc->addWidget( sb_order );

   QLabel* lb_fit = us_label( tr( "Meniscus at minimum:" ) );
   misc->addWidget( lb_fit );

   le_fit = us_lineedit( "" );
   le_fit->setReadOnly( true );
   misc->addWidget( le_fit );

   QLabel* lb_rms_error = us_label( tr( "RMS Error:" ) );
   misc->addWidget( lb_rms_error );
   
   le_rms_error = us_lineedit( "" );
   le_rms_error->setReadOnly( true );
   misc->addWidget( le_rms_error );

   main->addLayout( misc, row++, 0, 1, 2 );

   // Button rows

   QBoxLayout* buttons = new QHBoxLayout;

   QPushButton* pb_plot = us_pushbutton( tr( "Plot" ) );
   connect( pb_plot, SIGNAL( clicked() ), SLOT( plot_data() ) );
   buttons->addWidget( pb_plot );

   QPushButton* pb_reset = us_pushbutton( tr( "Reset" ) );
   connect( pb_reset, SIGNAL( clicked() ), SLOT( reset() ) );
   buttons->addWidget( pb_reset );

   QPushButton* pb_help = us_pushbutton( tr( "Help" ) );
   connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );
   buttons->addWidget( pb_help );

   QPushButton* pb_accept = us_pushbutton( tr( "Close" ) );
   connect( pb_accept, SIGNAL( clicked() ), SLOT( close() ) );
   buttons->addWidget( pb_accept );

   main->addLayout( buttons, row, 0, 1, 2 );
}

void US_FitMeniscus::reset( void )
{
   meniscus_plot->clear();
   meniscus_plot->replot();
   
   te_data->e   ->setPlainText( "" );
   sb_order     ->setValue( 2 );
   le_fit       ->setText( "" );
   le_rms_error ->setText( "" );
}

void US_FitMeniscus::plot_data( int )
{
   plot_data();
}

void US_FitMeniscus::plot_data( void )
{
   meniscus_plot->clear();

   QString contents = te_data->e->toPlainText();
   contents.replace( QRegExp( "[^0-9eE\\.\\n\\+\\-]+" ), " " );

   QStringList lines = contents.split( "\n", QString::SkipEmptyParts );
   QStringList parsed;

   double* radius_values = new double[ lines.size() ];
   double* rmsd_values   = new double[ lines.size() ];
   
   int     count = 0;

   double  minx = 1e20;
   double  maxx = 0.0;

   double  miny = 1e20;
   double  maxy = 0.0;

   // Remove any non-data lines and put values in arrays
   for ( int i = 0; i < lines.size(); i++ )
   {
      QStringList values = lines[ i ].split( ' ', QString::SkipEmptyParts );
qDebug() << values;
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
         parsed << QString::number( radius_values[ count ], 'e', 6 ) + ", " +
                   QString::number( rmsd_values  [ count ], 'e', 6 ); 

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
//qDebug() << "sb_order->value()" << order;
   if ( ! US_Matrix::lsfit( c, radius_values, rmsd_values, count, order + 1 ) )
   {
      QMessageBox::warning( this,
            tr( "Data Problem" ),
            tr( "The data is inadequate for this fit order" ) );
      
      le_fit      ->clear();
      le_rms_error->clear();
      meniscus_plot->replot();

      delete [] radius_values;
      delete [] rmsd_values;
      
      return;  
   }

//   for ( int i = 0; i < order + 1; i++ ) qDebug() << QString::number( c[ i ], 'e', 6 );
//   qDebug() << "------";


   int fit_count = (int) ( ( maxx - minx + 2 * overscan ) / 0.001 );

   double* fit_x = new double[ fit_count ];
   double* fit_y = new double[ fit_count ];
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

   le_rms_error->setText( QString::number( sqrt( rms_err / count ), 'e', 3 ) );

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
   delete [] fit_x;
   delete [] fit_y;
   delete [] radius_values;
   delete [] rmsd_values;
}

