//! \file us_run_details2.cpp

#include "us_run_details2.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_math2.h"
#include "us_constants.h"
#include "us_util.h"
#include "us_gui_util.h"

#if QT_VERSION < 0x050000
#define setSamples(a,b,c) setData(a,b,c)
#define setSymbol(a)      setSymbol(*a)
#endif

#include <qwt_legend.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_curve.h>
#include <qwt_scale_widget.h>

US_RunDetails2::US_RunDetails2( const QVector< US_DataIO::RawData >& data, 
                                const QString&                       runID, 
                                const QString&                       dataDir, 
                                const QStringList&                   cell_ch_wl )
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
   main->addWidget( lw_rpm, row, 2, 6, 2 );

   lw_triples = us_listwidget();
   lw_triples->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Minimum );
   lw_triples->setMinimumSize( 100, 50 );
   main->addWidget( lw_triples, row, 4, 6, 2 );

   le_runID = us_lineedit();
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
   QLabel* lb_firstScan = us_label( tr( "Time of Scan 1:" ) );
   main->addWidget( lb_firstScan, row, 0 );

   le_firstScan = us_lineedit();
   le_firstScan->setReadOnly( true );
   main->addWidget( le_firstScan, row++, 1 );

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

   connect( lw_rpm,     SIGNAL( itemClicked     ( QListWidgetItem* ) ),
            this,       SLOT  ( show_rpm_details( QListWidgetItem* ) ) );
}

US_RunDetails2::~US_RunDetails2()
{
   // Before leaving, save a combined plot for each triple automatically
   plotType = COMBINED;
   QString dir    = US_Settings::reportDir() + "/" + le_runID->text();
   if ( ! QDir( dir ).exists() )      // make sure the directory exists
      QDir().mkdir( dir );
   // int save_currentRow = lw_triples->currentRow();
   data_plot->setVisible( false );
   for ( int i = 0; i < triples.size(); i++ )
   {
      QString triple = US_Util::compressed_triple( triples[ i ] );
      QString filename = dir + "/rundetail." + triple + ".rundetail.svgz";

      // Calculate the current plot and write it to a file
      lw_triples->setCurrentRow( i );
      update( i );
      int status = US_GuiUtil::save_plot( filename, data_plot );
      if ( status != 0 )
         qDebug() << filename << "plot not saved";
   }

   // Don't need to restore original view because we're leaving
}

void US_RunDetails2::setup( void )
{
   // Set length of run
   double last   = 0.0;
   double first  = dataList[ 0 ].scanData[ 0 ].seconds;
   US_DataIO::RawData data;

   foreach( data, dataList )
   {
      last          = qMax( last,  data.scanData.last().seconds );
      first         = qMin( first, data.scanData.first().seconds );
   }
   
   last          = qRound( last );
   first         = qFloor( first );

   //run length in secs
   run_length = last - first;
   
   int  hours    = (int)qFloor( last / 3600.0 );
   int  mins     = (int)qRound( ( last - hours * 3600.0 ) / 60.0 );

   QString hh    = ( hours == 1 ) ? tr( "hr" ) : tr( "hrs" );
	        hh    = "h";
   QString wks   = QString().sprintf( "%d %s %02d m", hours, hh.toLatin1().data(), mins );
   int fmins     = (int)qFloor( first / 60.0 );
   int fsecs     = first - fmins * 60.0;
   QString mm    = ( fmins == 1 ) ? tr( "min" ) : tr( "mins" );

   QString scan1time = QString().sprintf( "%d m %02d s", fmins, fsecs );
//*DEBUG*
data=dataList[0];
double s1tim=data.scanData[0].seconds;
double s1omg=data.scanData[0].omega2t;
double s1rpm=data.scanData[0].rpm;
qDebug() << "dtails: ds 1, scan 1: secs,omg2t,rpm" << s1tim << s1omg << s1rpm;
//*DEBUG*
 
   le_runLen->setText( wks );
   le_firstScan->setText( scan1time );

   // Set Time Correction
   double correction = 0.0;
   int    scanCount  = 0;

   US_DataIO::Scan scan;
   
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

   le_timeCorr->setText( wks.sprintf( "%d m %02d s", minutes, seconds ) );

   int ss_reso         = 100;
   // If debug_text so directs, change set_speed_resolution
   QStringList dbgtxt = US_Settings::debug_text();
   for ( int ii = 0; ii < dbgtxt.count(); ii++ )
   {  // If debug text modifies ss_reso, apply it
      if ( dbgtxt[ ii ].startsWith( "SetSpeedReso" ) )
         ss_reso       = QString( dbgtxt[ ii ] ).section( "=", 1, 1 ).toInt();
   }

   // Set rpm list widget
   int i = 0;

   foreach( data, dataList )
   {
      int scanNumber = 1;
      foreach( scan, data.scanData )
      {
         // Round to closest 100 rpm (or other resolution)
         int rpm  = (int)qRound( scan.rpm / (double)ss_reso ) * ss_reso;
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
   scanCount_per_dataType .clear();
   for ( int i = 0; i < triples.size(); i++ )
   {
      int scans = dataList[ i ].scanData.size();
      lw_triples->addItem( triples[ i ] + wks.sprintf( " -- %d scans", scans ) );

      //determine also dataType:
      char chtype[ 3 ] = { 'R', 'A', '\0' };
      strncpy( chtype, dataList[ i ].type, 2 );
      QString dataType = QString( chtype ).left( 2 );
      qDebug() << "[US_RunDetails] dataType -- " << dataType;

      scanCount_per_dataType[ dataType ] << QString::number( scans );
   }

   //removeDupl.
   for( int i=0; i<scanCount_per_dataType.keys().size(); ++ i )
     {
       scanCount_per_dataType[ scanCount_per_dataType.keys()[i] ].removeDuplicates();
       qDebug() << "[US_RunDetails] dataType--scanCount: "
		<< scanCount_per_dataType.keys()[i] << " -- " << scanCount_per_dataType[ scanCount_per_dataType.keys()[i] ];
     }
   lw_triples->addItem( wks.sprintf( "All scans -- %d scans", scanCount ) );    

   // Set triple to indicate All Data
   lw_triples->setCurrentRow( triples.size() );

   show_all_data();
}

void US_RunDetails2::show_all_data( void )
{
   le_desc->setText( "" );

   US_DataIO::RawData triple;
   US_DataIO::Scan    scan;
   double             temp      = 0.0;
   double             rpm       = 0.0;
   int                scanCount = 0;

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
   rpm  = qRound( rpm / 100.0 );  // Round to closest 100 rpm
   le_rotorSpeed->setText( QString::number( (int)rpm * 100 ) + " RPM" );

   rpm_av        = (int)rpm * 100;
   scan_count_av = (int)scanCount/(int)dataList.size();

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
   double prior_seconds = 0.0;

   for ( int i = 0; i < scanCount; i++ )
   {
      x[ i ] = i + 1;
      t[ i ] = values[ i ].temperature;
      r[ i ] = values[ i ].rpm / 1000.0;

      
      if ( i > 0 )
         m[ i ] = ( values[ i ].seconds - prior_seconds ) / 60.0;

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

   if ( plotType == TEMPERATURE  || plotType == COMBINED )
   {
      QwtPlotCurve* c1 = us_curve( data_plot, tr( "Temperature" ) );
      QwtSymbol*  sym1 = new QwtSymbol;
      sym1->setStyle( QwtSymbol::Ellipse );
      sym1->setPen  ( QPen( Qt::yellow ) );
      sym1->setBrush( Qt::white );
      sym1->setSize ( 6 );
      c1->setPen    ( QPen( QBrush( Qt::yellow ), 2 ) );
      c1->setSymbol ( sym1 );
      c1->setSamples( x, t, count );
   }

   if ( plotType == RPM  || plotType == COMBINED )
   {
      QwtPlotCurve* c2 = us_curve( data_plot, tr( "RPM" ) );
      QwtSymbol*  sym2 = new QwtSymbol;
      sym2->setStyle( QwtSymbol::Ellipse );
      sym2->setPen  ( QPen( Qt::green  ) );
      sym2->setBrush( Qt::white );
      sym2->setSize ( 6 );
      c2->setPen    ( QPen( QBrush( Qt::green ), 2 ) );
      c2->setSymbol ( sym2 );
      c2->setSamples( x, r, count );
   }

   if ( plotType == INTERVAL  || plotType == COMBINED )
   {
      QwtPlotCurve* c3 = us_curve( data_plot, tr( "Scan Time Deltas" ) );
      QwtSymbol*  sym3 = new QwtSymbol;
      sym3->setStyle( QwtSymbol::Ellipse );
      sym3->setPen  ( QPen( Qt::red    ) );
      sym3->setBrush( Qt::white );
      sym3->setSize ( 6 );
      if ( plotType == COMBINED ) c3->setYAxis( QwtPlot::yRight );
      c3->setPen    ( QPen( QBrush( Qt::red ), 2 ) );
      c3->setSymbol ( sym3 );
      c3->setSamples( &x[ 1 ], &m[ 1 ], count - 1 );
   }

   if ( data_plot->legend() == NULL )
   {
      QwtLegend* legend = new QwtLegend;
      data_plot->insertLegend( legend, QwtPlot::BottomLegend );
      legend->setFrameStyle( QFrame::Box | QFrame::Sunken );

#if QT_VERSION < 0x050000
      QList< QWidget* > items = legend->legendItems();

      QFont font = items[ 0 ]->font();
      font.setPointSize( US_GuiSettings::fontSize() );

      QWidget* item;
      foreach( item, items ) item->setFont( font );

      data_plot->insertLegend( legend, QwtPlot::BottomLegend );
#else
      QFont lfont( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() );
      data_plot->setFont( lfont );
      legend   ->setFont( lfont );
#endif
   }

   data_plot->replot();
}

void US_RunDetails2::update( int index )
{
   lw_rpm->clearSelection();

   if ( lw_triples->currentItem()->text().contains( tr( "All" ) ) )
   {
      show_all_data();
      return;
   }

   const US_DataIO::RawData* data      = &dataList[ index ];
   int                       scanCount = data->scanData.size();

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
   rpm  = qRound( rpm / 100.0 );  // Round to closest 100 rpm
   le_rotorSpeed->setText( QString::number( (int)rpm * 100 ) + " RPM" );

   double maxTemp = -1.0e99;
   double minTemp =  1.0e99;

   QVector< double > x( scanCount );
   QVector< double > t( scanCount );
   QVector< double > r( scanCount );
   QVector< double > m( scanCount );
   double prior_seconds = 0.0;

   for ( int i = 0; i < scanCount; i++ )
   {
      const US_DataIO::Scan* s = &data->scanData[ i ];

      x[ i ] = i + 1;
      t[ i ] = s->temperature;
      maxTemp = max( maxTemp, s->temperature );
      minTemp = min( minTemp, s->temperature );

      r[ i ] = s->rpm / 1000.0;

      if ( i > 0 )  m[ i ] = ( s->seconds - prior_seconds ) / 60.0;

      prior_seconds = s->seconds;
   }

   // Determine temperature variation
   double dt = data->temperature_spread();;

   check_temp( dt );

   draw_plot( x.constData(), t.constData(), r.constData(), m.constData(), scanCount );
}

void US_RunDetails2::show_rpm_details( QListWidgetItem* item )
{
qDebug() << "show_rpm_details";
   QString sspeed  = item->text();
   QString msg     = tr( "The following scans have been measured at " )
                   + sspeed + ":\n\n";

qDebug() << " srd: msg" << msg;
   QStringList sl  = sspeed.split( " " );
   sspeed          = sl[ 0 ];
   int     rpm     = sspeed.toInt();
qDebug() << " srd: sl" << sl << "rpm" << rpm << "np" << sl.count();

   sl              = map.values( rpm );
   qSort( sl ); // contains cell / channel / wavelength / scan
                //  ( or cell / channel / scan  for MWL data )

   QString triple  = triples[ 0 ];
   bool    isMwl   = triple.split( " / " ).size() < 3;
   QString pcellCh = "0/Z";

   foreach( triple, triples )
   {
      QList< int > scans;
      QString     value;

      foreach( value, sl )
      {
//qDebug() << " srd:  triple" << triple << "value" << value;
         if ( value.startsWith( triple ) )
         {
            QStringList components = value.split( " / " );
            int         lvx        = components.size() - 1;
            scans << components[ lvx ].toInt();
         }
      }
      
      if ( scans.size() == 0 ) continue;
      
      QStringList cellChWl = triple.split( " / " );
      QString     cellCh   = cellChWl[ 0 ] + "/" + cellChWl[ 1 ];
qDebug() << " srd: ccw" << cellChWl << "ccwsz" << cellChWl.count();
qDebug() << " srd:  pcellCh cellCh isMwl" << pcellCh << cellCh << isMwl;

      if ( cellChWl.size() > 2 )
      {  // Normal (c/c/w/s) data
         msg    += tr( "Cell: "         ) + cellChWl[ 0 ] 
                +  tr( ", Channel: "    ) + cellChWl[ 1 ]
                +  tr( ", Wavelength: " ) + cellChWl[ 2 ]
                +  tr( ", Scan Count: " );
         isMwl   = ( isMwl  ||  cellCh == pcellCh );
qDebug() << " srd:   isMwl" << isMwl;
      }

      else
      {  // MWL (c/c/s) data
         msg    += tr( "Cell: "         ) + cellChWl[ 0 ] 
                +  tr( ", Channel: "    ) + cellChWl[ 1 ]
                +  tr( ", Scan Count: " );
      }

      msg    += QString::number( scans.size() ) + "\n";
      pcellCh = cellCh;
   }

   bool have_avg  = false;
   QString runID  = le_runID->text();
   QString fname  = US_Settings::resultDir() + "/" + runID + "/"
                    + runID + ".RI.xml";
   QFile   filei( fname );
   if ( filei.open( QIODevice::ReadOnly | QIODevice::Text ) )
   {
      QTextStream texti( &filei );
      QString xtext  = texti.readAll();
      have_avg       = xtext.contains( "avg_speed" );
      filei.close();
   }

   if ( have_avg )
   {
      msg    += moreSpeedInfo( rpm );
   }

#if 1
   QMessageBox::information( this,
         tr( "Speed Information" ), msg );
#endif
#if 0
   msg += tr( "\nYou may \"Close\" this dialog or click \"Details\""
              " to open a text dialog showing speed details"
              " for all scans" );
   QMessageBox msgbox;
   msgbox.setIcon( QMessageBox::Information );
   msgbox.setText( tr( "Speed Information" ) );
   msgbox.setInformativeText( msg );
   msgbox.addButton( tr( "Details"  ), QMessageBox::AcceptRole );
   msgbox.addButton( tr( "Close"    ), QMessageBox::RejectRole );
   msgbox.adjustSize();
   int sbutton = msgbox.exec();
qDebug() << "select button" << sbutton << "Rej(C) Acc(D)"
   << QMessageBox::RejectRole << QMessageBox::AcceptRole;

   if ( sbutton == QMessageBox::AcceptRole )
   {
QMessageBox::information( this, "DEBUG", "you clicked DETAILS" );
   }
else
QMessageBox::information( this, "DEBUG", "you clicked CLOSE" );
#endif
   lw_rpm->clearSelection();
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

// Read any speedsteps from run xml and create speed details message
QString US_RunDetails2::moreSpeedInfo( double rpm )
{
   QString msg    = "";
   QString runID  = le_runID->text();
   QString fname  = US_Settings::resultDir() + "/" + runID + "/"
                    + runID + ".RI.xml";
   QFile   filei( fname );
qDebug() << " srd:mSI: rpm fname" << rpm << fname;
   double  rpm_s  = 0.0;
   double  rpm_a  = 0.0;
   double  rpm_d  = 0.0;

   if ( filei.open( QIODevice::ReadOnly | QIODevice::Text ) )
   {
      QXmlStreamReader xmli( &filei );

      while ( ! xmli.atEnd() )
      {
         xmli.readNext();
qDebug() << " srd:mSI:  xml name" << xmli.name();

         if ( xmli.isStartElement()  &&   xmli.name() == "speedstep" )
         {
            QXmlStreamAttributes attr = xmli.attributes();
            double  rpm_x  = attr.value( "rotorspeed"   ).toString().toDouble();
            QString msg    = "";
qDebug() << " srd:mSI:   rpm_x" << rpm_x;

            if ( qAbs( rpm_x - rpm ) < 200.0 )
            {
               rpm_s       = attr.value( "set_speed"    ).toString().toDouble();
               rpm_a       = attr.value( "avg_speed"    ).toString().toDouble();
               rpm_d       = attr.value( "speed_stddev" ).toString().toDouble();
qDebug() << " srd:mSI:M:  rpm_s rpm_a rpm_d" << rpm_s << rpm_a << rpm_d;
               break;
            }
         }
      }

      filei.close();
   }
qDebug() << " srd:mSI:    rpm_s rpm_a rpm_d" << rpm_s << rpm_a << rpm_d;

   if ( rpm_a != 0.0 )
   {
      int rpm_fe     = qRound( rpm_a );
      msg            = tr( "\nSpeed step additional information:"
                           "\n   Set Speed = %1 ;"
                           "\n   Average Speed = %2 ;"
                           "\n   Speed Standard Deviation = %3 ;"
                           "\n   Finite Element Rotor Speed = %4 ." )
                       .arg( rpm_s ).arg( rpm_a ).arg( rpm_d ).arg( rpm_fe );
   }
else
 msg="\n*** MWL but no extended speedstep values ***";
   
   return msg;
}

//get main params
QMap< QString, QString>  US_RunDetails2::get_params_public()
{
  QMap< QString, QString> parms;

  parms[ "RPM" ]       = QString::number( rpm_av );
  parms[ "Time" ]      = QString::number( run_length );
  //parms[ "ScanCount" ] = QString::number( scan_count_av );

  QString scanCount_str;
  for( int i=0; i<scanCount_per_dataType.keys().size(); ++ i )
    {
      QString key = scanCount_per_dataType.keys()[i];
      scanCount_str += key + ":" + scanCount_per_dataType[ key ][0] + ",";
    }
  scanCount_str. chop(1);
  parms[ "ScanCount" ] = scanCount_str;
  
  return parms;
}
 
