//! \file us_intensity.cpp

#include "us_intensity.h"
#include "us_gui_settings.h"

US_Intensity::US_Intensity( const QVector< double >& data ) 
    : US_WidgetsDialog( 0, 0 ), dataIn( data )
{
   setWindowTitle( tr( "Details for Average Intensity Values" ) );
   setPalette( US_GuiSettings::frameColor() );
   setAttribute( Qt::WA_DeleteOnClose );

   QGridLayout* main = new QGridLayout( this );
   main->setSpacing        ( 2 );
   main->setContentsMargins( 2, 2, 2, 2 );

   int row  = 0;

   // Plot Rows
   QBoxLayout* plot = new US_Plot( data_plot,
        tr( "Intensity Profile" ),
        tr( "Scan Number" ),
        tr( "Intensity" ));

   data_plot->setMinimumSize( 800, 200 );

   QwtPlotGrid* grid = us_grid( data_plot );
   grid->enableXMin( false );

   main->addLayout( plot, row, 0, 5, 6 );
   row += 6;

   QHBoxLayout* buttons = new QHBoxLayout();

   QPushButton* pb_close = us_pushbutton( tr( "Close" ) );
   pb_close->setFixedWidth( 100 );
   connect( pb_close, SIGNAL( clicked() ), SLOT( close() ) );
   buttons->insertStretch( 0, 10 );
   buttons->addWidget( pb_close );

   main->addLayout( buttons, row++, 2, 1, 4 );

   draw_plot( dataIn );
}

void US_Intensity::draw_plot( const QVector< double >& scanData )
{
   // Set up the axes and titles
   QwtText axisTitle = data_plot->axisTitle( QwtPlot::yLeft );
   
   data_plot->setAxisAutoScale( QwtPlot::yLeft );
   data_plot->setAxisScale    ( QwtPlot::xBottom, 1.0, scanData.size() );
   data_plot->setAxisMaxMinor ( QwtPlot::xBottom, 0 );
   data_plot->setAxisTitle    ( QwtPlot::yLeft, "Intensity" );

   QwtSymbol sym;

   sym.setStyle( QwtSymbol::Ellipse );
   sym.setPen  ( QPen( Qt::yellow ) );
   sym.setBrush( Qt::white );
   sym.setSize ( 6 );

   // Get the scan data in the right format
   double* x = new double[ scanData.size() ];
   double* y = new double[ scanData.size() ];

   for ( int i = 0; i < scanData.size(); i++ )
   {
      x[ i ] = i + 1;
      y[ i ] = scanData[ i ];
   }

   QwtPlotCurve* c1 = us_curve( data_plot, tr( "Intensity" ) );
   c1->setPen   ( QPen( QBrush( Qt::yellow ), 2 ) );
   c1->setSymbol( sym );
   c1->setData  ( x, y, scanData.size() );

   data_plot->replot();

   delete [] x;
   delete [] y;
}
