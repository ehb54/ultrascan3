//! \file us_tmst_plot.cpp

#include "us_tmst_plot.h"
#include "us_time_state.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_gui_util.h"
#include "us_math2.h"
#include "us_constants.h"
#include "us_images.h"
#include "us_editor.h"
#if QT_VERSION < 0x050000
#define setSamples(a,b,c) setData(a,b,c)
#endif

#include <qwt_legend.h>

// constructor:  residuals plot widget
US_TmstPlot::US_TmstPlot( QWidget* parent, const QString tspath )
   : US_WidgetsDialog( parent, 0 )
{
   // Begin laying out the GUI
   setObjectName( "US_TmstPlot" );
   setWindowTitle( tr( "Time State Plots" ) );
   setPalette( US_GuiSettings::frameColor() );

   QSize p1size( 600, 240 );
   QSize p2size( 600, 360 );

   dbg_level        = US_Settings::us_debug();
   tmstpath         = tspath;
   QString xdpath   = QString( tmstpath ).replace( ".tmst", ".xml" );
   nfkeys           = 0;
   ndkeys           = 0;
   ntimes           = 0;
   nscans           = 0;
   nplpts           = 0;
   x_scans          = false;
   s_start          = true;
   defvers          = QString( "1.0" );
   imptype          = QString( "(unknown)" );
DbgLv(1) << "TP:mn: tmstpath" << tmstpath;
DbgLv(1) << "TP:mn:   xdpath" << xdpath;

   QHBoxLayout* mainLayout    = new QHBoxLayout( this );
   QVBoxLayout* leftLayout    = new QVBoxLayout();
   QVBoxLayout* rightLayout   = new QVBoxLayout();
   QGridLayout* ctrlsLayout   = new QGridLayout();
   QHBoxLayout* buttonsLayout = new QHBoxLayout();

   mainLayout->setSpacing        ( 2 );
   mainLayout->setContentsMargins( 2, 2, 2, 2 );

   // Control widgets
   QLabel* lb_ctrls       = us_banner( tr( "Time State Plot Controls" ) );
   QLabel* lb_pltkey      = us_label(  tr( "Plot Option:" ) );
           cb_pltkey      = us_comboBox();
           lw_datinfo     = us_listwidget();
   QLabel* lb_info        = us_banner( tr( "Time State Data Summary" ) );
   lw_datinfo->setSelectionMode( QAbstractItemView::NoSelection );

   QPushButton* pb_prev   = us_pushbutton( tr( "Previous" ) );
   QPushButton* pb_next   = us_pushbutton( tr( "Next" ) );
   pb_prev->setIcon( US_Images::getIcon( US_Images::ARROW_LEFT  ) );
   pb_next->setIcon( US_Images::getIcon( US_Images::ARROW_RIGHT ) );
   QPushButton* pb_detail = us_pushbutton( tr( "Data Details" ) );
   QPushButton* pb_close  = us_pushbutton( tr( "Close" ) );

   // Layout controls grid and bottom button row
   int row   = 0;
   ctrlsLayout->addWidget( lb_ctrls,    row++, 0,  1, 4 );
   ctrlsLayout->addWidget( lb_pltkey,   row,   0,  1, 2 );
   ctrlsLayout->addWidget( cb_pltkey,   row++, 2,  1, 2 );
   ctrlsLayout->addWidget( pb_prev,     row,   0,  1, 2 );
   ctrlsLayout->addWidget( pb_next,     row++, 2,  1, 2 );
   ctrlsLayout->addWidget( lb_info,     row++, 0,  1, 4 );
   ctrlsLayout->addWidget( lw_datinfo,  row++, 0, 20, 4 );

   QGridLayout* box1 = us_checkbox( tr( "X axis scans only" ),
                          ck_xscans, x_scans );
   QGridLayout* box2 = us_checkbox( tr( "X start at scan 1" ),
                          ck_sstart, s_start );

   buttonsLayout ->addWidget( pb_detail );
   buttonsLayout ->addWidget( pb_close  );

   // Lay out the right-side plots
   US_Plot* plotLayout1 = new US_Plot( data_plot1,
         tr( "Time Data" ),
         tr( "Scan Number" ),
         tr( "Time (minutes)" ) );

   US_Plot* plotLayout2 = new US_Plot( data_plot2,
         tr( "Time State Data:  Combined" ),
         tr( "Scan Number" ),
         tr( "Time (minutes)" ) );

   data_plot1->setCanvasBackground( Qt::black );
   data_plot2->setCanvasBackground( Qt::black );
   data_plot1->setMinimumSize( p1size );
   data_plot2->setMinimumSize( p2size );

   rightLayout->addLayout( plotLayout1    );
   rightLayout->addLayout( plotLayout2    );

   leftLayout ->addLayout( ctrlsLayout );
   leftLayout ->addStretch();
   leftLayout ->addLayout( box1  );
   leftLayout ->addLayout( box2  );
   leftLayout ->addLayout( buttonsLayout  );

   mainLayout->addLayout( leftLayout  );
   mainLayout->addLayout( rightLayout );
   mainLayout->setStretchFactor( leftLayout,  1 );
   mainLayout->setStretchFactor( rightLayout, 4 );

   connect( cb_pltkey, SIGNAL( currentIndexChanged( int ) ),
            this,      SLOT  ( plot_kdata()               ) );
   connect( pb_prev,   SIGNAL( clicked()   ),
            this,      SLOT  ( prev_plot() ) );
   connect( pb_next,   SIGNAL( clicked()   ),
            this,      SLOT  ( next_plot() ) );
   connect( pb_detail, SIGNAL( clicked()   ),
            this,      SLOT  ( details()   ) );
   connect( pb_close,  SIGNAL( clicked()   ),
            this,      SLOT  ( close()     ) );
   connect( ck_xscans, SIGNAL( toggled( bool ) ),
            this,      SLOT  ( plot_data()     ) );
   connect( ck_sstart, SIGNAL( toggled( bool ) ),
            this,      SLOT  ( plot_data()     ) );

   adjustSize();
   lw_datinfo->setMinimumHeight( lb_pltkey ->height() * 16 );

   data_plot1->resize( p1size );
   data_plot2->resize( p2size );
DbgLv(1) << "TP:mn:   p1size" << p1size << "p2size" << p2size;

   // Read in TimeState data, populate widgets, and do initial plots

   read_data();        // Read in time state data

   plot_data();        // Do an initial set of plots

   resize( p2size );
   adjustSize();
DbgLv(1) << "TP:mn:   resized" << size() << "lw size" << lw_datinfo->size();
}


//public funcitons to access GMP-related parms
QStringList US_TmstPlot::timestamp_data_dkeys( )
{
  return dkeys;
}

QVector< QVector< double > > US_TmstPlot::timestamp_data_dvals( )
{
  return dvals;
}

QMap< QString, double >  US_TmstPlot::timestamp_data_mins ( )
{
  return dmins;
}

QMap< QString, double >  US_TmstPlot::timestamp_data_maxs ( )
{
  return dmaxs;
}

QMap< QString, double >  US_TmstPlot::timestamp_data_avgs ( )
{
  return davgs;
}

QMap < QString, QMap< QString, double >>  US_TmstPlot::timestamp_data_avgs_stdd_first_scan ( )
{
  QMap < QString, QMap< QString, double > > davgs_stdd_first_scan;

  s_start = true;  // X axis starts at scan 1
  
  for ( int i=0; i < dkeys.size(); ++i )
    {
      QString pkey = dkeys[ i ];
      int ky       = dkeys.indexOf( pkey );       // Y data index

      // Accumulate X,Y points
      nplpts       = point_indexes();
      
      //Average:
      double y_avg_val = 0;
      double yy_first  = 0;
      double yy_last   = 0;
      for ( int jj = 0; jj < nplpts; jj++ )
	{
	  int jt       = pntxs[ jj ];
	  //xx[ jj ]     = dvals[ kx ][ jt ];
	  //yy[ jj ]     = dvals[ ky ][ jt ];

	  if ( jj == 0 )
	    yy_first = dvals[ ky ][ jt ];
	  if ( jj ==  nplpts - 1 )
	    yy_last = dvals[ ky ][ jt ];
	  
	  y_avg_val   +=  dvals[ ky ][ jt ];
	}
   
      double scalea_cut  = 1.0 / (double)nplpts;             // Averaging scale
      y_avg_val         *= scalea_cut;

      qDebug() << "[auto] Plot for: " << pkey  << "; First value of Y: " << yy_first;
      qDebug() << "[auto] Plot for: " << pkey  << "; Last value of Y: "  << yy_last;
      qDebug() << "[auto] Plot for: " << pkey  << "; # of points: "      << nplpts;
      qDebug() << "[auto] Plot for: " << pkey  << "; First data index: " << pntxs[ 0 ];
      qDebug() << "[auto] Plot for: " << pkey  << "; Average: "          << y_avg_val;

      //Standard Deviation:
      double std_dev = 0;
      double sum_mean_dev_sq = 0;
      for ( int jj = 0; jj < nplpts; jj++ )
	{
	  int jt       = pntxs[ jj ];

	  sum_mean_dev_sq += (dvals[ ky ][ jt ] - y_avg_val) * (dvals[ ky ][ jt ] - y_avg_val); 
	}
      
      //std_dev = sqrt( sum_mean_dev_sq * scalea_cut );
      sum_mean_dev_sq *= scalea_cut;
      std_dev = pow( double(sum_mean_dev_sq), double(0.5) );
      
      //Fill the Map:
      QMap < QString, double > keys_data;
      keys_data[ "Avg"  ] = y_avg_val;
      keys_data[ "Stdd" ] = std_dev;
      davgs_stdd_first_scan[ pkey ] = keys_data;
    }
  
  
  return davgs_stdd_first_scan;
}


// Plot the data (both key-specific and combined)
void US_TmstPlot::plot_data()
{
   if ( ck_xscans->isChecked() )
   {  // For scans-only, disable scan1-start checkbox and set checked
      ck_sstart->setEnabled( false );
      if ( ! ck_sstart->isChecked() )
      {
         ck_sstart->disconnect();
         ck_sstart->setChecked( true );
         connect( ck_sstart, SIGNAL( toggled( bool ) ),
                  this,      SLOT  ( plot_data()     ) );
      }
   }
   else
   {  // If not scans-only, insure scan1-start checkbox is enabled
      ck_sstart->setEnabled( true );
   }

   plot_kdata();     // Plot data for specific key

   plot_cdata();     // Plot combined data
}

// Plot the data for a specific key
void US_TmstPlot::plot_kdata()
{
   dataPlotClear( data_plot1 );

   QString pkey   = cb_pltkey->currentText();  // Current key
   data_plot1->setTitle( tr( " Time State Data:  " ) + pkey );

   QString ytitle = pkey;                      // Y title = key (+ units?)
   if ( pkey.startsWith( "Time" ) )
      ytitle         = ytitle + "  (mins.)";
   else if ( pkey.contains( "Speed" ) )
      ytitle         = ytitle + "  (RPM)";
   else if ( pkey.contains( "Tempera" ) )
      ytitle         = ytitle + "  (C)";

   data_plot1->setAxisTitle( QwtPlot::yLeft, ytitle );
   us_grid( data_plot1 );                                  // Grid
   US_PlotPicker* pick = new US_PlotPicker( data_plot1 );  // Annotated cursor
   pick->setRubberBand( QwtPicker::RectRubberBand );

   x_scans      = ck_xscans->isChecked();
   s_start      = ck_sstart->isChecked();

   QString xkey = x_scans ? "Scan" : "Time";
   QString xtit = x_scans ? "Scan Number" : "Time (minutes)";
   int kx       = dkeys.indexOf( xkey );       // X data index
   int ky       = dkeys.indexOf( pkey );       // Y data index
   int ks       = dkeys.indexOf( "Scan" );     // Scan number index
   data_plot1->setAxisTitle( QwtPlot::xBottom, xtit );
   QVector< double > xvec( ntimes, 0.0 );      // Initialize X vector
   QVector< double > yvec( ntimes, 0.0 );      // Initialize Y vector

   double* xx   = xvec.data();
   double* yy   = yvec.data();
   double smin  = dmaxs[ xkey  ];
   double smax  = -1.0;
   double ymin  = dmaxs[ pkey ];
   double ymax  = -1.0;

   // Accumulate X,Y points
   nplpts       = point_indexes();

   //double y_avg_val = 0;
   
   for ( int jj = 0; jj < nplpts; jj++ )
   {
      int jt       = pntxs[ jj ];
      xx[ jj ]     = dvals[ kx ][ jt ];
      yy[ jj ]     = dvals[ ky ][ jt ];
      smin         = qMin( smin, xx[ jj ] );
      smax         = qMax( smax, xx[ jj ] );
      ymin         = qMin( ymin, yy[ jj ] );
      ymax         = qMax( ymax, yy[ jj ] );

      //y_avg_val   += yy[ jj ];
   }

   //double scalea_cut  = 1.0 / (double)nplpts;             // Averaging scale
   //y_avg_val         *= scalea_cut;
      
   // qDebug() << "Plot for: " << pkey  << "; First value of Y: " << yy[ 0 ];
   // qDebug() << "Plot for: " << pkey  << "; Last value of Y: "  << yy[ nplpts - 1 ];
   // qDebug() << "Plot for: " << pkey  << "; # of points: "      << nplpts;
   // qDebug() << "Plot for: " << pkey  << "; Average: "          << y_avg_val;
   
DbgLv(1) << "TP:plkd   smin smax" << smin << smax << "xkey" << xkey << "kx ks" << kx << ks;
   data_plot1->setAxisScale( QwtPlot::xBottom, smin, smax );

   if ( ymin < ymax )
   {  // Auto-scale where Y is normal range type
      data_plot1->setAxisAutoScale( QwtPlot::yLeft );
   }

   else
   {  // Add min,max padding where Y is constant
      ymin         = ( ymin > 0.0 ) ? ( ymin * 0.95 ) : ( ymin * 1.05 );
      ymax         = ( ymax > 0.0 ) ? ( ymax * 1.05 ) : ( ymax * 0.95 );
      data_plot1->setAxisScale( QwtPlot::yLeft, ymin, ymax );
   }

   QwtPlotCurve* curv = us_curve( data_plot1, pkey );  // Create a single curve
//   curv->setPen ( QPen( US_GuiSettings::plotCurve(), 2 ) );
   curv->setPen    ( QPen( curve_color( ky ) ) );
   //curv->setSamples( xx, yy, ntimes );
   curv->setSamples( xx, yy, nplpts );

   data_plot1->replot();
}


// Return a pointer to the QwtPlot for the upper plot
QwtPlot* US_TmstPlot::rp_data_plot1( QString p_type )
{
  //set to Temp. OR any other type
  cb_pltkey -> setCurrentIndex(cb_pltkey->findText( p_type ));
  return data_plot1;
}


// Plot the combined data
void US_TmstPlot::plot_cdata()
{
   x_scans      = ck_xscans->isChecked();
   s_start      = ck_sstart->isChecked();

   dataPlotClear( data_plot2 );
   us_grid( data_plot2 );                                     // Grid

   US_PlotPicker* pick = new US_PlotPicker( data_plot2 );     // Annotate cursor
   pick->setRubberBand( QwtPicker::RectRubberBand );

   QwtLegend* legend   = new QwtLegend;                       // Legend
   legend->setFrameStyle( QFrame::Box | QFrame::Sunken );
   legend->setFont( QFont( US_GuiSettings::fontFamily(),
                           US_GuiSettings::fontSize() - 1 ) );
   data_plot2->insertLegend( legend, QwtPlot::BottomLegend );

   QVector< double > xvec( ntimes, 0.0 );                     // Plox xy vectors
   QVector< double > yvec( ntimes, 0.0 );
   double* xx   = xvec.data();
   double* yy   = yvec.data();
   QString xkey = x_scans ? "Scan" : "Time";
   QString xtit = x_scans ? "Scan Number" : "Time (minutes)";
   int kx       = dkeys.indexOf( xkey );                      // X key index
   double smin  = dmaxs[ xkey ];
   double smax  = -1.0;

   // Accumulate X,Y points
   nplpts       = point_indexes();

   for ( int jj = 0; jj < nplpts; jj++ )
   {
      int jt       = pntxs[ jj ];
      xx[ jj ]     = dvals[ kx ][ jt ];
      smin         = qMin( smin, xx[ jj ] );
      smax         = qMax( smax, xx[ jj ] );
   }

   // Set axes scales and title
   int npkeys   = pkeys.count();                              // Plot key count
DbgLv(1) << "TP:plcd:  ndkeys npkeys npltps" << ndkeys << npkeys << nplpts
 << "smin smax" << smin << smax;
   data_plot2->setAxisTitle( QwtPlot::xBottom, xtit );
   data_plot2->setAxisScale( QwtPlot::xBottom, smin, smax );
   data_plot2->setAxisTitle( QwtPlot::yLeft, tr( "Y percent + K offset" ) );

   // Create a curve for each key
   for ( int jk = 0; jk < npkeys; jk++ )
   {
      QString pkey = pkeys[ jk ];                             // Plot key
      int ky       = dkeys.indexOf( pkey );                   // Data key index
      QColor ccolr = curve_color( ky );                       // Curve color
      double ymin  = dmaxs[ pkey ];                           // Data Y min
      double ymax  = -1.0;                                    // Data Y max

      for ( int jj = 0; jj < nplpts; jj++ )
      {  // Get data Y min,max
         int jt       = pntxs[ jj ];
         double yval  = dvals[ ky ][ jt ];
         ymin         = qMin( ymin, yval );
         ymax         = qMax( ymax, yval );
      }
DbgLv(1) << "TP:plcd:   jk ky" << jk << ky
 << "color" << ccolr << "key" << pkey << "ymin ymax" << ymin << ymax;

      if ( ymin == ymax )
      {  // Adjust the range when Y is constant
         ymin         = ( ymin > 0.0 ) ? ( ymin * 0.95 ) : ( ymin * 1.05 );
         ymax         = ( ymax > 0.0 ) ? ( ymax * 1.05 ) : ( ymax * 0.95 );
      }

      // Scale Y range to 100% percent and offset by 10% x key index
      double yrng  = ymax - ymin;                       // Y range
      double yscl  = 100.0 / yrng;                      // Scale to percent
      double yoff  = (double)jk * 100.0 - ymin * yscl;  // Offset by key index
DbgLv(1) << "TP:plcd:   " << pkey << ccolr << "  ymin ymax" << ymin << ymax
 << "yoff" << yoff << "yscl" << yscl;

      // Scale and offset each Y point to fit in common Y axis
      for ( int jj = 0; jj < nplpts; jj++ )
      {
         int jt       = pntxs[ jj ];
         yy[ jj ]     = yoff + dvals[ ky ][ jt ] * yscl;
      }
DbgLv(1) << "TP:plcd:       yy0 yyn" << yy[0] << yy[nplpts-1];

      // Create the colored curve for the current key value type; add to legend
      QwtPlotCurve* curv = us_curve( data_plot2, pkey );
      curv->setPen ( QPen( ccolr, 2 ) );
      //curv->setSamples( xx, yy, ntimes );
      curv->setSamples( xx, yy, nplpts );
      curv->setItemAttribute( QwtPlotItem::Legend, true );
   }

   data_plot2->setAxisScale( QwtPlot::yLeft, 0.0, 100.0 * npkeys );
   data_plot2->replot();
}

// Slot to move to previous key-based plot
void US_TmstPlot::prev_plot()
{
   int pltx   = cb_pltkey->currentIndex() - 1;
   pltx       = ( pltx >= 0 ) ? pltx : ( pkeys.count() - 1 );
   cb_pltkey->setCurrentIndex( pltx );
}

// Slot to move to next key-based plot
void US_TmstPlot::next_plot()
{
   int pltx   = cb_pltkey->currentIndex() + 1;
   pltx       = ( pltx < pkeys.count() ) ? pltx : 0;
   cb_pltkey->setCurrentIndex( pltx );
}

// Slot to pop up a details text dialog
void US_TmstPlot::details()
{
   QString dat_dir  = tmstpath.section( "/",  0, -2 ) + "/";
   QString tsfname  = tmstpath.section( "/", -1, -1 );
   QString xdfname  = QString( tsfname ).replace( ".tmst", ".xml" );
   QString runID    = tsfname .section( ".",  0, -3 );

   // Compose the details text

   QString dtext    = tr( "Time State Details:\n\n" );
   dtext += tr( "Run ID:            %1\n"   ).arg( runID   );
   dtext += tr( "TMST Directory:    %1\n"   ).arg( dat_dir );
   dtext += tr( "TMST File Name:    %1\n"   ).arg( tsfname );
   dtext += tr( "Def. File Name:    %1\n\n" ).arg( xdfname );
   dtext += tr( "Field Key Count:   %1\n"   ).arg( nfkeys  );
   dtext += tr( "Data Key Count:    %1\n"   ).arg( ndkeys  );
   dtext += tr( "Time Value Count:  %1\n"   ).arg( ntimes  );
   dtext += tr( "Scan Number Count: %1\n"   ).arg( nscans  );
   dtext += tr( "Def. Version:      %1\n"   ).arg( defvers );
   dtext += tr( "Import Type:       %1\n"   ).arg( imptype );
   int jl     = ntimes - 1;
   int jm     = ntimes / 2;

   for ( int jk = 0; jk < ndkeys; jk++ )
   {  // Compose a brief summary for each key
      QString dkey = dkeys[ jk ];
      dtext += tr( "\n(%1) Key \"%2\"\n" ).arg( jk + 1 ).arg( dkey );
      dtext += tr( "  Minimum:   %1\n" ).arg( dmins[ dkey ] );
      dtext += tr( "  Maximum:   %1\n" ).arg( dmaxs[ dkey ] );
      dtext += tr( "  Average:   %1\n" ).arg( davgs[ dkey ] );
      dtext += tr( "  First:     %1\n" ).arg( dvals[ jk ][  0 ] );
      dtext += tr( "  Middle:    %1\n" ).arg( dvals[ jk ][ jm ] );
      dtext += tr( "  Last:      %1\n" ).arg( dvals[ jk ][ jl ] );
   }

   // Build a list of start,end scans,times for each step
   int lostep     = dmins[ "Step" ];
   int histep     = dmaxs[ "Step" ];
   int nstep      = histep - lostep + 1;
   int tmdx       = dkeys.indexOf( "Time" );          // Time data index
   int scdx       = dkeys.indexOf( "Scan" );          // Scan number index
   int stpx       = dkeys.indexOf( "Step" );          // Step index
   int spdx       = dkeys.indexOf( "SetSpeed" );      // Speed index
DbgLv(1) << "TP: det: lostep histep nstep" << lostep << histep << nstep
 << "tmdx scdx stpx" << tmdx << scdx << stpx;
   QVector< double >  ststimes;
   QVector< double >  stetimes;
   QVector< int >     stsscans;
   QVector< int >     stescans;
   QVector< int >     stspeeds;
   ststimes.fill( 0.0, nstep );
   stetimes.fill( 0.0, nstep );
   stsscans.fill(   0, nstep );
   stescans.fill(   0, nstep );
   stspeeds.fill(   0, nstep );
   for ( int jt = 0; jt < ntimes; jt++ )
   {
      int scan_v     = dvals[ scdx ][ jt ];
      if ( scan_v > 0 )
      {
         double time_v  = dvals[ tmdx ][ jt ];
         int step_v     = ( histep > 0 ) ? dvals[ stpx ][ jt ] : 0;
         int step_x     = step_v - lostep;
         if ( ststimes[ step_x ] == 0.0 )
         {
            ststimes[ step_x ] = time_v;
            stsscans[ step_x ] = scan_v;
            stspeeds[ step_x ] = dvals[ spdx ][ jt ];
         }
         stetimes[ step_x ] = time_v;
         stescans[ step_x ] = scan_v;
      }
   }
   dtext += tr( "\nSet speed resolution: %1 rpm\n" ).arg( ssreso );
   dtext += tr( "Step Scans and Times in minutes:\n" );
   dtext += tr( "  Step  Speed  Scan range      Time range\n" );
   for ( int js = 0; js < nstep; js++ )
   {
      int stepnum    = lostep + js;
      int ispeed     = stspeeds[ js ];
      dtext += QString().sprintf( "%5d   %5d %5d to %5d  %9.3f to %9.3f\n",
                  stepnum, ispeed, stsscans[ js ], stescans[js],
                  ststimes[ js ], stetimes[ js ] );
DbgLv(1) << "TP: det: step speed" << stepnum << ispeed
 << "scan_0 time_0 scan_n time_n"
 << stsscans[js] << ststimes[js] << stescans[js] << stetimes[js];
   }

   // Pop up a text dialog with the details content

   US_Editor* ediag = new US_Editor( US_Editor::LOAD, true, "", this );
   ediag->setWindowTitle( tr( "Time State Information" ) );
   ediag->move( this->pos() + QPoint( 200, 200 ) );
   ediag->resize( 800, 460 );
   ediag->e->setFont( QFont( "monospace", US_GuiSettings::fontSize() ) );
   ediag->e->setText( dtext );
   ediag->show();
}

// Read in the TimeState data and populate widgets
void US_TmstPlot::read_data()
{
   QStringList  fkeys;
   QStringList  ffmts;
   US_TimeState tsobj;
DbgLv(1) << "TP:rdda: tmstpath" << tmstpath;
   QString xdpath   = QString( tmstpath ).replace( ".tmst", ".xml" );
   QString dat_dir  = tmstpath.section( "/",  0, -2 ) + "/";
   QString tsfname  = tmstpath.section( "/", -1, -1 );
   QString runID    = tsfname .section( ".",  0, -3 );

   if ( tsobj.open_read_data( tmstpath ) != 0 )
   {
DbgLv(1) << "TP:rdda: *ERROR* openread :" << tsobj.last_error_message();
      ndkeys         = 0;
      return;
   }

   tsobj.origin( &defvers, &imptype );     // Def version and import type
   tsobj.field_keys( &fkeys, &ffmts );     // Field keys and formats
   nfkeys         = fkeys.count();         // Field keys count
   ntimes         = tsobj.time_count();    // Time values count
   ssreso         = tsobj.ss_resolution(); // Set speed resolution
   double tscl    = 1.0 / 60.0;            // Time scale, seconds-to-minutes
DbgLv(1) << "TP:rdda: ntimes nfkeys" << ntimes << nfkeys;
   ndkeys         = nfkeys + 1;            // Data keys count (interval added)
   dvals.fill( QVector< double >(), ndkeys );  // Init data vector of vectors

   for ( int jt = 0; jt < ntimes; jt++ )
   {  // Build data vectors for each time position
      tsobj.read_record();

      for ( int jk = 0; jk < nfkeys; jk++ )
      {  // Set the value for each key
         QString fkey   = fkeys[ jk ];
         QString ffmt   = ffmts[ jk ];
         // Get value as double, regardless of file format
         double  fval   = ffmt.contains( "F" )
                          ? tsobj.time_dvalue( fkey )
                          : (double)tsobj.time_ivalue( fkey );
         // Convert any time value from seconds to minutes
         fval          *= ( fkey == "Time" ?  tscl : 1.0 );
//DbgLv(1) << "TP:rdda:  jt jk" << jt << jk << "fkey" << fkey << "ffmt" << ffmt
// << "fval" << fval;

         dvals[ jk + 1 ] << fval;  // Store value (offset for extra field)
      }
   }

   tsobj.close_read_data();

   dkeys << "TimeInterval";     // Insert extra delta-t field
   dkeys << fkeys;              // Then insert from file fields

   double scalea  = 1.0 / (double)ntimes;             // Averaging scale
   int tmdx       = dkeys.indexOf( "Time" );          // Time data index
   int scdx       = dkeys.indexOf( "Scan" );          // Scan number index
   int dtdx       = dkeys.indexOf( "TimeInterval" );  // Delta-t data index

   // Build a list of time indexes to scan points
   for ( int jt = 0; jt < ntimes; jt++ )
   {
      int scan       = (int)dvals[ scdx ][ jt ];
      if ( scan > 0 )  scnxs << jt;
   }

   nscans         = scnxs.count();
DbgLv(1) << "TP:rdda: tmdx" << tmdx << "dtdx" << dtdx
 << "dvals[tmdx]size" << dvals[tmdx].size() << "nscans" << nscans;

   // Build the vector of TimeInterval-between-scans values
   int jt1        = scnxs[ 0 ];
   int jt2        = scnxs[ 1 ];
   double delta_t = ( dvals[ tmdx ][ jt2 ] - dvals[ tmdx ][ jt1 ] );
   for ( int jj = 0; jj < jt1; jj++ )
      dvals[ dtdx ] << delta_t;

   for ( int ii = 1; ii < scnxs.count(); ii++ )
   {
      jt1            = scnxs[ ii - 1 ];
      jt2            = scnxs[ ii ];
      delta_t        = ( dvals[ tmdx ][ jt2 ] - dvals[ tmdx ][ jt1 ] );
      for ( int jj = jt1; jj < jt2; jj++ )
      {
         dvals[ dtdx ] << delta_t;
      }
   }

   for ( int jj = jt2; jj < ntimes; jj++ )
   {
      dvals[ dtdx ] << delta_t;
   }

DbgLv(1) << "TP:rdda:  dvals[dtdx]size" << dvals[dtdx].size() << "ntimes" << ntimes;

   // Now scan values for each key to build minimum,maximum,average
   for ( int jk = 0; jk < ndkeys; jk++ )
   {
      QString dkey   = dkeys[ jk ];
      double dmin    = 1e+99;
      double dmax    = -1e+99;
      double davg    = 0.0;

      for ( int jt = 0; jt < ntimes; jt++ )
      {  // Accumulate min, max, and sum
         double dval    = dvals[ jk ][ jt ];
         if ( dval == 0.0 )  continue;
         dmin           = qMin( dmin, dval );
         dmax           = qMax( dmax, dval );
         davg          += dval;
if(jk==9 &&  dval<0.0)
{
int j2=qMax(1,jt);
int j1=jt-1;
DbgLv(1) << "TP:rdda:      DT" << dval << "jt" << jt << "T2,T1"
 << dvals[1][j2] << dvals[1][j1] << "   +++++++";
}
      }

      davg          *= scalea;    // Scale sum to get average
      dmins[ dkey ]  = dmin;      // Map minimum to key
      dmaxs[ dkey ]  = dmax;      // Map maximum to key
      davgs[ dkey ]  = davg;      // Map average to key
DbgLv(1) << "TP:rdda:  key" << dkey << "min,max,avg" << dmin << dmax << davg;
   }
//*DEBUG*
for (int jk=0;jk<ndkeys;jk++)
{
 QString dkey=dkeys[jk];
DbgLv(1) << "TP:rdda:    key" << dkey << "   min,max,avg"
 << dmins[dkey] << dmaxs[dkey] << davgs[dkey];
}
//*DEBUG*

   // Build plot options comboBox items
   pkeys.clear();
   pkeys << "TimeInterval";

   for ( int jk = 0; jk < ndkeys; jk++ )
   {
      QString dkey   = dkeys[ jk ];

      if ( dkey == "Scan"  ||  dkey == "TimeInterval" )
         continue;

      pkeys << dkey;
   }

   cb_pltkey->addItems( pkeys );

   // Populate status information list widget
   QStringList infotxt;
   infotxt << tr( "Run ID:  %1" ).arg( runID );
   infotxt << tr( "Directory:  %1" ).arg( dat_dir );
   infotxt << tr( "File:  %1" ).arg( tsfname );
   infotxt << tr( "Time Count:  %1" ).arg( ntimes );
   infotxt << tr( "Scan Count:  %1" ).arg( nscans );
   infotxt << tr( "Field Key Count:  %1" ).arg( nfkeys );
   infotxt << tr( "Data Key Count:  %1" ).arg( ndkeys );
   infotxt << tr( "Set Speed Resolution:  %1" ).arg( ssreso );

   for ( int jk = 0; jk < ndkeys; jk++ )
   {
      QString dkey   = dkeys[ jk ];

      infotxt << tr( "  %1 Range:  %2 to %3" ).arg( dkey )
                 .arg( dmins[ dkey ] ).arg( dmaxs[ dkey ] );
   }

   lw_datinfo->clear();
   lw_datinfo->addItems( infotxt );

   return;
}

// Get color for a specific key
QColor US_TmstPlot::curve_color( const int keyx )
{
   const QColor ccolors[] = {   // Curve colors that differ by key
      QColor( 255,   0,   0 ),
      QColor(   0, 255,   0 ),
      QColor(   0,   0, 255 ),
      QColor( 255, 255,   0 ),
      QColor( 255,   0, 255 ),
      QColor(   0, 255, 255 ),
      QColor( 122,   0, 255 ),
      QColor(   0, 255, 122 ),
      QColor(   0, 122, 255 ),
      QColor( 255, 122,   0 ),
      QColor( 122, 255,   0 ),
      QColor(  80,   0, 255 ),
      QColor(   0, 255,  80 ),
      QColor(   0,  80, 255 ),
      QColor( 255,  80,   0 ),
      QColor(  80, 255,   0 ),
      QColor(  40, 255,  40 ),
      QColor(  40,  40, 255 ),
      QColor(  80,  80,  80 ),
      QColor( 122, 122, 122 ) };
   int ncolrs   = sizeof( ccolors ) / sizeof( ccolors[0] );      // Colors count
   int jc       = ( keyx < ncolrs ) ? keyx : ( keyx % ncolrs );  // Color index

   return ccolors[ jc ];
}

// Generate the current plot point data indexes
int US_TmstPlot::point_indexes()
{
   pntxs.clear();

   if ( x_scans )
      pntxs = scnxs;
   else if ( s_start )
   {
      for ( int jj = scnxs[ 0 ]; jj < ntimes; jj++ )
         pntxs << jj;
   }
   else
   {
      for ( int jj = 0; jj < ntimes; jj++ )
         pntxs << jj;
   }

   return pntxs.count();
}

