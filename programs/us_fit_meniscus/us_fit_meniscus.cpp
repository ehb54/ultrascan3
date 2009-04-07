//! \file us_fit_meniscus.cpp
#include "us_fit_meniscus.h"
#include "us_gui_settings.h"
#include "us_math.h"
#include "us_matrix.h"

US_FitMeniscus::US_FitMeniscus() : US_Widgets()
{
   setWindowTitle( tr( "Fit Meniscus from 2DSA Data" ) );
   setPalette( US_GuiSettings::frameColor() );

   QGridLayout* main = new QGridLayout( this );
   main->setSpacing         ( 2 );
   main->setContentsMargins ( 2, 2, 2, 2 );

   int row = 0;

   te_data = new US_Editor( US_Editor::LOAD, false );
   
   QFontMetrics fm( QFont( US_GuiSettings::fontFamily(), 
                           US_GuiSettings::fontSize()   ) );
   
   te_data->setMinimumHeight( fm.height() * 20 );
   te_data->setFixedWidth ( fm.width( '0' ) * 20 );

   main->addWidget( te_data, row, 0, 20, 1 );

   QBoxLayout* plot = new US_Plot( meniscus_plot, 
         tr( "Meniscus Fit" ),
         tr( "Radius" ), tr( "2DSA Meniscus RMSD Value" ) );
   
   us_grid( meniscus_plot );
   
   meniscus_plot->setMinimumSize( 400, 400 );
   meniscus_plot->setAxisScale( QwtPlot::xBottom, 5.7, 6.8 );

   main->addLayout( plot, row, 1, 20, 1 );
   row += 20;

   fit_order = QUADRATIC;

   QRadioButton* rb_quadratic;
   QRadioButton* rb_cubic;

   QGridLayout* quadratic = 
      us_radiobutton( tr( "Quadratic Fit" ), rb_quadratic, true );
   
   QGridLayout* cubic     = 
      us_radiobutton( tr( "Cubic Fit"     ), rb_cubic );

   rb_cubic->setFixedWidth( rb_quadratic->width() );

   QButtonGroup* fitGroup = new QButtonGroup;
   fitGroup->addButton( rb_quadratic, QUADRATIC );
   fitGroup->addButton( rb_cubic    , CUBIC     );
   connect( fitGroup, SIGNAL( buttonClicked( int ) ),
                      SLOT  ( fit_type     ( int ) ) );

   QBoxLayout* misc = new QHBoxLayout;

   misc->addLayout( quadratic ); 
   misc->addLayout( cubic     ); 

   QLabel* lb_fit = us_label( tr( "Meniscus at minimum:" ) );
   misc->addWidget( lb_fit );

   le_fit = us_lineedit( "" );
   le_fit->setReadOnly( true );
   misc->addWidget( le_fit );

   main->addLayout( misc, row++, 0, 1, 2 );

   // Button rows

   QBoxLayout* buttons = new QHBoxLayout;

   QPushButton* pb_reset = us_pushbutton( tr( "Plot" ) );
   connect( pb_reset, SIGNAL( clicked() ), SLOT( plot_data() ) );
   buttons->addWidget( pb_reset );

   QPushButton* pb_help = us_pushbutton( tr( "Help" ) );
   connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );
   buttons->addWidget( pb_help );

   QPushButton* pb_accept = us_pushbutton( tr( "Close" ) );
   connect( pb_accept, SIGNAL( clicked() ), SLOT( close() ) );
   buttons->addWidget( pb_accept );

   main->addLayout( buttons, row, 0, 1, 2 );
}

void US_FitMeniscus::fit_type( int new_order )
{
   fit_order = new_order;
   plot_data();
}

void US_FitMeniscus::plot_data( void )
{
   meniscus_plot->clear();

   QString contents = te_data->e->toPlainText();
   contents.remove( QRegExp( "[^0-9\\.,\\n]" ) );

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
      QStringList values = lines[ i ].split( ',', QString::SkipEmptyParts );
      if ( values.size() > 1 ) 
      {
         radius_values[ count ] = values[ 0 ].toDouble();
         rmsd_values  [ count ] = values[ 1 ].toDouble();

         // Find min and max
         minx = min( minx, radius_values[ count ] );
         maxx = max( maxx, radius_values[ count ] );

         miny = min( miny, rmsd_values[ count ] );
         maxy = max( maxy, rmsd_values[ count ] );

         // Reformat
         parsed << QString::number( radius_values[ count ], 'f', 4 ) + ", " + 
                   QString::number( rmsd_values  [ count ], 'f', 4 ); 

         count++;
      }
   }

   te_data->e->setPlainText( parsed.join( "\n" ) );

   double overscan = ( maxx - minx ) * 0.10;  // 10% overscan

   meniscus_plot->setAxisScale( QwtPlot::xBottom, 
         minx - overscan, maxx + overscan );
   
   meniscus_plot->setAxisScale( QwtPlot::yLeft  , miny - 1.0  , maxy + 1.0   );

   raw_curve = us_curve( meniscus_plot, tr( "Raw Data" ) ); 
   raw_curve->setPen( QPen( Qt::yellow ) );

   raw_curve->setData( radius_values, rmsd_values, count );

   // Do the fit and get the minimum

   double c[ 4 ];

   int order = ( fit_order == QUADRATIC ) ? 2 : 3;

   US_Matrix::lsfit( c, radius_values, rmsd_values, count, order + 1 );

   int fit_count = (int) ( ( maxx - minx + 2 * overscan ) / 0.001 );

   double* fit_x = new double[ fit_count ];
   double* fit_y = new double[ fit_count ];
   double x      = minx - overscan;
   double minimum;

   if (  fit_order == QUADRATIC )
   {
      for ( int i = 0; i < fit_count; i++, x += 0.001 )
      {
         fit_x[ i ] = x;
         fit_y[ i ] = c[ 0 ] + c[ 1 ] * x + c[ 2 ] * sq( x );
      }

      // Take the derivitive and get the minimum
      // c1 + 2 * c2 * x = 0
      minimum = - c[ 1 ] / ( 2.0 * c[ 2 ] );

   }
   else
   {
      for ( int i = 0; i < fit_count; i++, x += 0.001 )
      {
         fit_x[ i ] = x;
         fit_y[ i ] = c[ 0 ] 
                    + c[ 1 ] * x 
                    + c[ 2 ] * x * x
                    + c[ 3 ] * x * x * x;
      }

      // Take the derivitive and get the minimum
      // c1 + 2 * c2 * x + 3 * c3 * x^2 = 0

      double a = 3 * c[ 3 ];
      double b = 2 * c[ 2 ];
      double d =     c[ 1 ];  // c is already used

      double discriminant = sq( b ) - 4 * a * d;

      double root1 = ( - b + sqrt ( discriminant ) ) / ( 2 * a );
      double root2 = ( - b - sqrt ( discriminant ) ) / ( 2 * a );

      if ( root1 < maxx  && root1 > minx ) 
         minimum = root1;
      else
         minimum = root2;
   }

   fit_curve = us_curve( meniscus_plot, tr( "Fitted Data" ) ); 
   fit_curve->setPen( QPen( Qt::red ) );
   fit_curve->setData( fit_x, fit_y, fit_count );


   // Plot the minimum

   minimum_curve = us_curve( meniscus_plot, tr( "Minimum Pointer" ) ); 
   minimum_curve->setPen( QPen( Qt::cyan ) );

   double radius_min[ 2 ];
   double rmsd_min  [ 2 ];

   radius_min[ 0 ] = minimum;
   radius_min[ 1 ] = minimum;

   rmsd_min  [ 0 ] = miny - 1.0;
   rmsd_min  [ 0 ] = miny + 2.0;

   minimum_curve->setData( radius_min, rmsd_min, 2 );

   meniscus_plot->replot();

   le_fit->setText( QString::number( minimum, 'f', 8 ) );
}

