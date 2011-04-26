//! \file us_run_details2.cpp

#include "us_run_details2.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_math2.h"
#include "us_constants.h"

#include <qwt_legend.h>

#define round(x) floor( (x) + 0.5 )

US_RunDetails2::US_RunDetails2( const QVector< US_DataIO2::RawData >& data, 
                                const QString&                        runID, 
                                const QString&                        dataDir, 
                                const QStringList&                    cell_ch_wl )
   : US_WidgetsDialog( 0, 0 ), dataList( data ), triples( cell_ch_wl )
{
   setWindowTitle( tr( "Details for Raw Data" ) );
   setPalette( US_GuiSettings::frameColor() );

   QGridLayout* main = new QGridLayout( this );
   main->setSpacing        ( 2 );
   main->setContentsMargins( 2, 2, 2, 2 );

   plotType  = TEMPERATURE;
   temp_warn = true;
   int row  = 0;

   // Plot Rows
   QBoxLayout* plot = new US_Plot( data_plot,
        tr( "Parameter Variation Throughout Run" ),
        tr( "Scan Number" ), 
        tr( "RPM * 1000 / Temperature " ) + DEGC );

   data_plot->setMinimumSize( 400, 200 );
   data_plot->enableAxis( QwtPlot::yRight );

   // Copy font for right axis from left axis
   QwtText axisTitle = data_plot->axisTitle( QwtPlot::yLeft );
   axisTitle.setText( tr( "Time between Scans (min)" ) );
   data_plot->setAxisTitle( QwtPlot::yRight, axisTitle );

   QwtPlotGrid* grid = us_grid( data_plot );
   grid->enableXMin( false );

   main->addLayout( plot, row, 0, 5, 6 );
   row += 6;

   // Row
   QLabel* lb_dir = us_label( tr( "Data Directory:" ) );
   main->addWidget( lb_dir, row, 0 );

   QLineEdit* le_dir = us_lineedit();
   le_dir->setReadOnly( true );
   le_dir->setText( dataDir );
   main->addWidget( le_dir, row++, 1, 1, 5 );

   // Row
   QLabel* lb_desc = us_label( tr( "Description:" ) );
   main->addWidget( lb_desc, row, 0 );

   le_desc = us_lineedit();
   le_desc->setReadOnly( true );
   le_desc->setText( dataDir );
   main->addWidget( le_desc, row++, 1, 1, 5 );

   // Row
   QLabel* lb_runID = us_label( tr( "Run Identification:" ) );
   main->addWidget( lb_runID, row, 0 );

   lw_rpm = us_listwidget();
   lw_rpm->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Minimum );
   lw_rpm->setMinimumSize( 100, 50 );
   main->addWidget( lw_rpm, row, 2, 5, 2 );

   lw_triples = us_listwidget();
   lw_triples->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Minimum );
   lw_triples->setMinimumSize( 100, 50 );
   main->addWidget( lw_triples, row, 4, 5, 2 );

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
   QLabel* lb_rotorSpeed = us_label( tr( "Avg. Rotor Speed:" ) );
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

   QHBoxLayout* buttons = new QHBoxLayout();

   QPushButton* pb_temp = us_pushbutton( tr( "Temperature" ) );
   connect( pb_temp, SIGNAL( clicked() ), SLOT( plot_temp() ) );
   buttons->addWidget( pb_temp );

   QPushButton* pb_rpm = us_pushbutton( tr( "RPM" ) );
   connect( pb_rpm, SIGNAL( clicked() ), SLOT( plot_rpm() ) );
   buttons->addWidget( pb_rpm );

   QPushButton* pb_interval = us_pushbutton( tr( "Interval" ) );
   connect( pb_interval, SIGNAL( clicked() ), SLOT( plot_interval() ) );
   buttons->addWidget( pb_interval );

   QPushButton* pb_all = us_pushbutton( tr( "Combined" ) );
   connect( pb_all, SIGNAL( clicked() ), SLOT( plot_combined() ) );
   buttons->addWidget( pb_all );

   QPushButton* pb_close = us_pushbutton( tr( "Close" ) );
   connect( pb_close, SIGNAL( clicked() ), SLOT( close() ) );
   buttons->addWidget( pb_close );

   main->addLayout( buttons, row++, 2, 1, 4 );

   timer = new QTimer();
   connect( timer, SIGNAL( timeout() ), SLOT( update_timer() ) );

   setup();
   connect( lw_triples, SIGNAL( currentRowChanged( int ) ),
                        SLOT  ( update           ( int ) ) );

   connect( lw_rpm    , SIGNAL( currentRowChanged( int ) ),
                        SLOT  ( show_rpm_details ( int ) ) );
}

void US_RunDetails2::setup( void )
{
   // Set length of run
   double              last = 0.0;
   US_DataIO2::RawData data;

   foreach( data, dataList )
      last = max( last, data.scanData.last().seconds );
   
   last       = round( last );
   int  hours = (int)floor( last / 3600.0 );
   int  mins  = (int) round( ( last - hours * 3600.0 ) / 60.0 );

   QString s; 
   QString h = ( hours == 1 ) ? tr( "hour" ) : tr( "hours" );
 
   le_runLen->setText( 
         s.sprintf( "%d %s %02d min", hours, h.toAscii().data(), mins ) );

   // Set Time Correction
   double correction = 0.0;
   int    scanCount  = 0;

   US_DataIO2::Scan scan;
   
   foreach( data, dataList )
   {
      foreach( scan, data.scanData )
      {
         double omega = ( M_PI / 30.0 ) * scan.rpm;
         correction  += scan.seconds - scan.omega2t / sq( omega );
         scanCount++;
      }
   }

   correction /= scanCount;
   int minutes = (int) correction / 60;
   int seconds = (int) correction % 60;

   le_timeCorr->setText( s.sprintf( "%d min %02d sec", minutes, seconds ) );

   // Set rpm list widget
   int i = 0;

   foreach( data, dataList )
   {
      int scanNumber = 1;
      foreach( scan, data.scanData )
      {
         // Round to closest 100 rpm
         int rpm  = (int)round( scan.rpm / 100.0 ) * 100;
         map.insert( rpm, triples[ i ] + " / " + QString::number( scanNumber ) );
         scanNumber++;
      }

      i++;
   }

   QList< int > rpms = map.uniqueKeys();
   qSort( rpms );
   QStringList  s_rpms;
   int          rpm;

   foreach( rpm, rpms ) s_rpms << QString::number( rpm ) + " RPM";
   lw_rpm->addItems( s_rpms );

   // Set triples + scans
   for ( int i = 0; i < triples.size(); i++ )
   {
      int scans = dataList[ i ].scanData.size();
      lw_triples->addItem( triples[ i ] + s.sprintf( " -- %d scans", scans ) );    
   }

   lw_triples->addItem( s.sprintf( "All scans -- %d scans", scanCount ) );    

   // Set triple to indicate All Data
   lw_triples->setCurrentRow( triples.size() );

   show_all_data();
}

void US_RunDetails2::show_all_data( void )
{
   le_desc->setText( "" );

   US_DataIO2::RawData triple;
   US_DataIO2::Scan    scan;
   double              temp      = 0.0;
   double              rpm       = 0.0;
   int                 scanCount = 0;

   // Note that these are not weighted averages 
   foreach( triple, dataList )
   {
      foreach( scan, triple.scanData )
      {
         temp += scan.temperature;
         rpm  += scan.rpm;
         scanCount++;
      }
   }

   // Set average temperature
   le_avgTemp->setText( QString::number( temp / scanCount, 'f', 1 ) 
         + " " + DEGC );

   // Set average rpm
   rpm /= scanCount;             // Get average
   rpm  = round( rpm / 100.0 );  // Round to closest 100 rpm
   le_rotorSpeed->setText( QString::number( (int)rpm * 100 ) + " RPM" );

   // Determine temperature variation
   double dt = 0.0;

   foreach( triple, dataList )
   {
      double temp_spread = triple.temperature_spread();
      dt = ( temp_spread > dt ) ? temp_spread : dt;
   }

   check_temp( dt );

   // Plot the data
   // First, put all data in a list and sort by time
   QList< graphValue > values;

   foreach( triple, dataList )
   {
      foreach( scan, triple.scanData )
      {
         values << graphValue( scan.seconds, scan.rpm, scan.temperature );
      }
   }

   qSort( values );
         
   QVector< double > x( scanCount );
   QVector< double > t( scanCount );
   QVector< double > r( scanCount );
   QVector< double > m( scanCount );

   for ( int i = 0; i < scanCount; i++ )
   {
      x[ i ] = i + 1;
      t[ i ] = values[ i ].temperature;
      r[ i ] = values[ i ].rpm / 1000.0;

      double prior_seconds;
      
      if ( i > 0 ) m[ i ] = ( values[ i ].seconds - prior_seconds ) / 60.0;

      prior_seconds = values[ i ].seconds;
   }

   draw_plot( x.constData(), t.constData(), r.constData(), m.constData(), scanCount );
}

void US_RunDetails2::check_temp( double dt )
{
   if ( dt <= US_Settings::tempTolerance() )
   {
      lb_red  ->setPalette( QPalette( QColor( 0x55, 0, 0 ) ) ); // Dark Red
      lb_green->setPalette( QPalette( Qt::green ) );
   }
   else
   {
      lb_red  ->setPalette( QPalette( Qt::red ) ); 
      lb_green->setPalette( QPalette( QColor( 0, 0x44, 0 ) ) ); // Dark Green

      if ( ! timer->isActive() ) timer->start( 1000 );
   }
}

void US_RunDetails2::update_timer( void )
{
   static bool           bright = true;
   static const QPalette red( Qt::red );
   static const QPalette darkRed( QColor( 0x55, 0, 0 ) );
   
   if ( bright )
      lb_red->setPalette( darkRed );
   else
      lb_red->setPalette( red );

   bright = ! bright;
}

void US_RunDetails2::draw_plot( const double* x, const double* t, 
      const double* r, const double* m, int count )
{
   // Set up the axes and titles
   QwtText axisTitle = data_plot->axisTitle( QwtPlot::yLeft );
   
   data_plot->setAxisTitle    ( QwtPlot::yRight, axisTitle );
   data_plot->enableAxis      ( QwtPlot::yRight, false );
   data_plot->setAxisAutoScale( QwtPlot::yLeft );
   data_plot->setAxisScale    ( QwtPlot::xBottom, 1.0, count );
   data_plot->setAxisMaxMinor ( QwtPlot::xBottom, 0 );
   
   switch( plotType )
   {
      case TEMPERATURE:
         axisTitle.setText( tr( "Temperature " ) + DEGC );
         data_plot->setAxisTitle( QwtPlot::yLeft, axisTitle );
         break;

      case RPM:
         axisTitle.setText( tr( "RPM * 1000" ) );
         data_plot->setAxisTitle( QwtPlot::yLeft, axisTitle );
         break;

      case INTERVAL:
         axisTitle.setText( tr( "Time between Scans (min)" ) );
         data_plot->setAxisTitle( QwtPlot::yLeft, axisTitle );
         break;

      default:
         axisTitle.setText( 
               tr( "RPM * 1000 / Temperature " ) + DEGC );
         data_plot->setAxisTitle( QwtPlot::yLeft, axisTitle );

         axisTitle.setText( tr( "Time between Scans (min)" ) );
         data_plot->setAxisTitle( QwtPlot::yRight, axisTitle );
         
         data_plot->enableAxis  ( QwtPlot::yRight, true );
         data_plot->setAxisScale( QwtPlot::yLeft, 0.0, 60.0 );
         break;

   }

   data_plot->detachItems( QwtPlotItem::Rtti_PlotCurve );

   QwtSymbol sym;

   sym.setStyle( QwtSymbol::Ellipse );
   sym.setPen  ( QPen( Qt::yellow ) );
   sym.setBrush( Qt::white );
   sym.setSize ( 6 );

   if ( plotType == TEMPERATURE  || plotType == COMBINED )
   {
      QwtPlotCurve* c1 = us_curve( data_plot, tr( "Temperature" ) );
      c1->setPen   ( QPen( QBrush( Qt::yellow ), 2 ) );
      c1->setSymbol( sym );
      c1->setData  ( x, t, count );
   }

   sym.setPen( QColor( Qt::green ) );

   if ( plotType == RPM  || plotType == COMBINED )
   {
      QwtPlotCurve* c2 = us_curve( data_plot, tr( "RPM" ) );
      c2->setPen       ( QPen( QBrush( Qt::green ), 2 ) );
      c2->setSymbol    ( sym );
      c2->setData      ( x, r, count );
   }

   sym.setPen( QColor( Qt::red ) );

   if ( plotType == INTERVAL  || plotType == COMBINED )
   {
      QwtPlotCurve* c3 = us_curve( data_plot, tr( "Scan Time Deltas" ) );
      if ( plotType == COMBINED ) c3->setYAxis( QwtPlot::yRight );
      c3->setPen       ( QPen( QBrush( Qt::red ), 2 ) );
      c3->setSymbol    ( sym );
      c3->setData      ( &x[ 1 ], &m[ 1 ], count - 1 );
   }

   if ( data_plot->legend() == NULL )
   {
      QwtLegend* legend = new QwtLegend;
      data_plot->insertLegend( legend, QwtPlot::BottomLegend );
      legend->setFrameStyle( QFrame::Box | QFrame::Sunken );
      
      QList< QWidget* > items = legend->legendItems();

      QFont font = items[ 0 ]->font();
      font.setPointSize( US_GuiSettings::fontSize() );

      QWidget* item;
      foreach( item, items ) item->setFont( font );

      data_plot->insertLegend( legend, QwtPlot::BottomLegend );
   }

   data_plot->replot();
}

void US_RunDetails2::update( int index )
{
   if ( lw_triples->currentItem()->text().contains( tr( "All" ) ) )
   {
      show_all_data();
      return;
   }

   const US_DataIO2::RawData* data      = &dataList[ index ];
   int                        scanCount = data->scanData.size();

   le_desc->setText( data->description );

   double temp = 0.0;
   double rpm  = 0.0;

   for ( int i = 0; i < scanCount; i++ )
   {
      temp += data->scanData[ i ].temperature;
      rpm  += data->scanData[ i ].rpm;
   }

   // Set average temperature
   le_avgTemp->setText( QString::number( temp / scanCount, 'f', 1 ) 
         + " " + DEGC );

   // Set average rpm
   rpm /= scanCount;             // Get average
   rpm  = round( rpm / 100.0 );  // Round to closest 100 rpm
   le_rotorSpeed->setText( QString::number( (int)rpm * 100 ) + " RPM" );

   double maxTemp = -1.0e99;
   double minTemp =  1.0e99;

   QVector< double > x( scanCount );
   QVector< double > t( scanCount );
   QVector< double > r( scanCount );
   QVector< double > m( scanCount );

   for ( int i = 0; i < scanCount; i++ )
   {
      const US_DataIO2::Scan* s = &data->scanData[ i ];

      x[ i ] = i + 1;
      t[ i ] = s->temperature;
      maxTemp = max( maxTemp, s->temperature );
      minTemp = min( minTemp, s->temperature );

      r[ i ] = s->rpm / 1000.0;

      double prior_seconds;
      
      if ( i > 0 )  m[ i ] = ( s->seconds - prior_seconds ) / 60.0;

      prior_seconds = s->seconds;
   }

   // Determine temperature variation
   double dt = data->temperature_spread();;

   check_temp( dt );

   draw_plot( x.constData(), t.constData(), r.constData(), m.constData(), scanCount );
}

void US_RunDetails2::show_rpm_details( int /* index */ )
{
   QString msg = tr( "The following scans have been measured at " )
               + lw_rpm->currentItem()->text() + ":\n\n";

   QStringList sl  = lw_rpm->currentItem()->text().split( " " ); 
   int         rpm = sl[ 0 ].toInt();

   sl = map.values( rpm );
   qSort( sl ); // contains cell / channel / wavelength / scan

   QString triple;

   foreach( triple, triples )
   {
      QList< int > scans;
      QString     value;

      foreach( value, sl )
      {
         if ( value.startsWith( triple ) )
         {
            QStringList components = value.split( " / " );
            scans << components[ 3 ].toInt();
         }
      }
      
      if ( scans.size() == 0 ) continue;
      
      QStringList cellChWl = triple.split( " / " );

      msg += tr( "Cell: "         ) + cellChWl[ 0 ] 
          +  tr( ", Channel: "    ) + cellChWl[ 1 ]
          +  tr( ", Wavelength: " ) + cellChWl[ 2 ]
          +  tr( ", Scan Count: " );
         

      msg += QString::number( scans.size() ) + "\n";
   }

   QMessageBox::information( this,
         tr( "Speed Information" ), msg );
}

void US_RunDetails2::plot_temp( void )
{
   plotType = TEMPERATURE;
   update( lw_triples->currentRow() );
}

void US_RunDetails2::plot_rpm( void )
{
   plotType = RPM;
   update( lw_triples->currentRow() );
}

void US_RunDetails2::plot_interval( void )
{
   plotType = INTERVAL;
   update( lw_triples->currentRow() );
}

void US_RunDetails2::plot_combined( void )
{
   plotType = COMBINED;
   update( lw_triples->currentRow() );
}
