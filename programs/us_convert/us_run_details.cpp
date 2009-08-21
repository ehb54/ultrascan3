//! \file us_run_details.cpp

#include "us_run_details.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_math.h"

#include <qwt_legend.h>

US_RunDetails::US_RunDetails( const QList< rawData >& data, int index,
      const QString& runID, const QString& dataDir, 
      const QStringList& cell_ch_wl )
   : US_WidgetsDialog( 0, 0 ), dataList( data ), triples( cell_ch_wl )
{
   setWindowTitle( tr( "Details for Raw Data" ) );
   setPalette( US_GuiSettings::frameColor() );

   QGridLayout* main = new QGridLayout( this );
   main->setSpacing        ( 2 );
   main->setContentsMargins( 2, 2, 2, 2 );

   int row = 0;

   // Plot Rows
   QBoxLayout* plot = new US_Plot( data_plot,
        tr( "Parameter Variation Throughout Run" ),
        tr( "Scan Number" ), 
        tr( "RPM * 1000 / Temperature " ) + QChar( 176 ) + "C" );

   data_plot->setMinimumSize( 400, 200 );
   main->addLayout( plot, row, 0, 5, 6 );

   data_plot->enableAxis( QwtPlot::yRight );

   // Copy font for right axis from left axis
   QwtText axisTitle = data_plot->axisTitle( QwtPlot::yLeft );
   axisTitle.setText( tr( "Time between Scans (min)" ) );
   data_plot->setAxisTitle( QwtPlot::yRight, axisTitle );

   grid = us_grid( data_plot );
   row += 6;

   // Row
   QLabel* lb_dir = us_label( tr( "Data Directory:" ) );
   main->addWidget( lb_dir, row, 0 );

   QLineEdit* le_dir = us_lineedit();
   le_dir->setReadOnly( true );
   le_dir->setText( dataDir );
   main->addWidget( le_dir, row++, 1, 1, 5 );

   // Row
   QLabel* lb_runID = us_label( tr( "Run Identification:" ) );
   main->addWidget( lb_runID, row, 0 );

   lw_triples = us_listwidget();
   //lw_triples->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Minimum );
   lw_triples->setMinimumSize( 50, 50 );
   main->addWidget( lw_triples, row, 2, 5, 3 );

   QLineEdit* le_runID = us_lineedit();
   le_runID->setReadOnly( true );
   le_runID->setText( runID );
   main->addWidget( le_runID, row++, 1 );

   // Row
   QLabel* lb_runLen = us_label( tr( "Length of Run:" ) );
   main->addWidget( lb_runLen, row, 0 );

   le_runLen = us_lineedit();
   le_runLen->setReadOnly( true );
   main->addWidget( le_runLen, row++, 1 );

   // Row
   QLabel* lb_timeCorr = us_label( tr( "Time Correction:" ) );
   main->addWidget( lb_timeCorr, row, 0 );

   le_timeCorr = us_lineedit();
   le_timeCorr->setReadOnly( true );
   main->addWidget( le_timeCorr, row++, 1 );

   // Row
   QLabel* lb_rotorSpeed = us_label( tr( "Rotor Speed:" ) );
   main->addWidget( lb_rotorSpeed, row, 0 );

   le_rotorSpeed = us_lineedit();
   le_rotorSpeed->setReadOnly( true );
   main->addWidget( le_rotorSpeed, row++, 1 );

   // Row
   QLabel* lb_avgTemp = us_label( tr( "Avg. Temperature:" ) );
   main->addWidget( lb_avgTemp, row, 0 );

   le_avgTemp = us_lineedit();
   le_avgTemp->setReadOnly( true );
   main->addWidget( le_avgTemp, row++, 1 );

   // Row
   QLabel* lb_tempCheck = us_label( tr( "Temperature Check:" ) );
   main->addWidget( lb_tempCheck, row, 0 );

   QHBoxLayout* box1 = new QHBoxLayout;
   box1->setAlignment( Qt::AlignCenter );

   QHBoxLayout* box2 = new QHBoxLayout;
   box2->setAlignment( Qt::AlignCenter );
   box2->setSpacing( 10 );

   lb_green = new QLabel();
   lb_green->setFixedSize(20, 16);
   lb_green->setPalette( QPalette( QColor( 0, 0x44, 0 ) ) ); // Dark Green
   lb_green->setAutoFillBackground( true );
   lb_green->setFrameStyle( QFrame::StyledPanel | QFrame::Sunken );

   lb_red   = new QLabel();
   lb_red->setFixedSize(20, 16);
   lb_red->setPalette( QPalette( QColor( 0x55, 0, 0 ) ) ); // Dark Red
   lb_red->setAutoFillBackground( true );
   lb_red->setFrameStyle( QFrame::StyledPanel | QFrame::Sunken );

   box2->addWidget( lb_green );
   box2->addWidget( lb_red );
   box1->addLayout( box2 );

   main->addLayout( box1, row, 1 );

   QPushButton* pb_close = us_pushbutton( tr( "Close" ) );
   connect( pb_close, SIGNAL( clicked() ), SLOT( close() ) );
   main->addWidget( pb_close, row++, 2, 1, 3 );

   update( index );
}

void US_RunDetails::update( int index )
{
   const rawData* r         = &dataList[ index ];
   int            scanCount = r->scanData.size();

   double         temp      = 0.0;
   double         rpm       = 0.0;

   for ( int i = 0; i < scanCount; i++ )
   {
      temp += r->scanData[ i ].temperature;
      rpm  += r->scanData[ i ].rpm;
   }

   // Set average temperature
   le_avgTemp->setText( QString::number( temp / scanCount, 'f', 1 ) 
         + " " + QChar( 176 ) + "C" );

   // Set average rpm
   rpm /= scanCount;             // Get average
   double omega = ( M_PI / 30.0 ) * rpm;
   rpm  = round( rpm / 100.0 );  // Round to closest 100 rpm
   le_rotorSpeed->setText( QString::number( (int)rpm * 100 ) + " RPM" );

   double correction = 0.0;

   for ( int i = 0; i < scanCount; i++ )
      correction += r->scanData[ i ].seconds 
                  - r->scanData[ i ].omega2t / sq( omega );

   correction /= scanCount;
   int minutes = (int) correction / 60;
   int seconds = (int) correction % 60;

   QString s;
   le_timeCorr->setText( s.sprintf( "%d min %02d sec", minutes, seconds ) );

   double last  = round( r->scanData[ scanCount - 1 ].seconds );
   int    hours = (int)floor( last / 3600.0 );
   int    mins  = (int) round( ( last - hours * 3600.0 ) / 60.0 );
   le_runLen->setText( s.sprintf( "%d hours %02d min", hours, mins ) );

   lw_triples->disconnect();
   lw_triples->clear();

   for ( int i = 0; i < triples.size(); i++ )
   {
      int scans = dataList[ i ].scanData.size();
      lw_triples->addItem( triples[ i ] + s.sprintf( " -- %d scans", scans ) );    
   }

   lw_triples->setCurrentRow( index );
   connect( lw_triples, SIGNAL( currentRowChanged( int ) ),
                        SLOT  ( update           ( int ) ) );

   double maxTemp = -1.0e99;
   double minTemp =  1.0e99;

   double* x = new double[ scanCount ];
   double* t = new double[ scanCount ];
   double* w = new double[ scanCount ];
   double* m = new double[ scanCount ];

   for ( int i = 0; i < scanCount; i++ )
   {
      const scan* s = &r->scanData[ i ];

      x[ i ] = i + 1;
      t[ i ] = s->temperature;
      maxTemp = max( maxTemp, s->temperature );
      minTemp = min( minTemp, s->temperature );

      w[ i ] = s->rpm / 1000.0;

      double prior_seconds;
      
      if ( i > 0 )  m[ i ] = ( s->seconds - prior_seconds ) / 60.0;

      prior_seconds = s->seconds;
   }

   if ( maxTemp - minTemp <= US_Settings::tempTolerance() )
   {
      lb_red  ->setPalette( QPalette( QColor( 0x55, 0, 0 ) ) ); // Dark Red
      lb_green->setPalette( QPalette( Qt::green ) );
   }
   else
   {
      lb_red  ->setPalette( QPalette( Qt::red ) ); 
      lb_green->setPalette( QPalette( QColor( 0, 0x44, 0 ) ) ); // Dark Green

      QMessageBox::warning( this, 
            tr( "Temperature Problem" ),
            tr( "The temperature in this run varied over the course\n"
                "of the run to a larger extent than allowed by the\n"
                "current threshold (" )  
                + QString::number( US_Settings::tempTolerance(), 'f', 1 )
                + " " + QChar( 178 ) + tr( "C). The accuracy of experimental\n"
                "results may be affected significantly." ) );
   }

   data_plot->clear();

   // Draw the plots
   data_plot->setAxisScale( QwtPlot::yLeft, 0.0, 60.0 );
   data_plot->setAxisScale( QwtPlot::xBottom, 1.0, scanCount, 5.0 );

   //QwtScaleDiv* xScale = data_plot->axisScalDiv( QwtPlot::xBottom );
   grid->enableYMin( false );
   //QwtScaleDiv xScale = grid->xScaleDiv();


   QwtSymbol sym;

   sym.setStyle( QwtSymbol::Ellipse);
   sym.setPen  ( QPen( Qt::yellow ) );
   sym.setBrush( Qt::white );
   sym.setSize ( 6 );

   QwtPlotCurve* c1 = new QwtPlotCurve( tr( "Temperature" ) );
   c1->setRenderHint( QwtPlotItem::RenderAntialiased );
   c1->setYAxis     ( QwtPlot::yLeft );
   c1->setPen       ( QPen( QBrush( Qt::yellow ), 2 ) );
   c1->setSymbol    ( sym );
   c1->attach       ( data_plot );
   c1->setData      ( x, t, scanCount );

   sym.setPen( QColor( Qt::green ) );

   QwtPlotCurve* c2 = new QwtPlotCurve( tr( "RPM" ) );
   c2->setRenderHint( QwtPlotItem::RenderAntialiased );
   c2->setYAxis     ( QwtPlot::yLeft );
   c2->setPen       ( QPen( QBrush( Qt::green ), 2 ) );
   c2->setSymbol    ( sym );
   c2->attach       ( data_plot );
   c2->setData      ( x, w, scanCount );

   sym.setPen( QColor( Qt::red ) );

   QwtPlotCurve* c3 = new QwtPlotCurve( tr( "Scan Time Deltas" ) );
   c3->setRenderHint( QwtPlotItem::RenderAntialiased );
   c3->setYAxis     ( QwtPlot::yRight );
   c3->setPen       ( QPen( QBrush( Qt::red ), 2 ) );
   c3->setSymbol    ( sym );
   c3->attach       ( data_plot );
   c3->setData      ( &x[ 1 ], &m[ 1 ], scanCount - 1 );

   QwtLegend* legend = new QwtLegend;
   legend->setFrameStyle( QFrame::Box | QFrame::Sunken );
   data_plot->insertLegend( legend, QwtPlot::BottomLegend );

   data_plot->replot();
   
   delete [] x;
   delete [] t;
   delete [] w;
   delete [] m;
}
