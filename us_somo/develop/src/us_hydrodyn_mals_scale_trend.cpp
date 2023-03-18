#include "../include/us3_defines.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_hydrodyn_saxs.h"
#include "../include/us_saxs_util.h"
#include "../include/us_hydrodyn_mals_scale_trend.h"
//Added by qt3to4:
#include <QHBoxLayout>
#include <QCloseEvent>
#include <QFrame>
#include <QLabel>
#include <QVBoxLayout>

US_Hydrodyn_Mals_Scale_Trend::US_Hydrodyn_Mals_Scale_Trend(
                                                                       void                     *              us_hydrodyn,
                                                                       map < QString, QString >                parameters,
                                                                       map < QString, vector < vector < double > > > plots,
                                                                       QWidget *                               p,
                                                                       const char *                            
                                                                       ) : QFrame(  p )
{
   this->us_hydrodyn                          = us_hydrodyn;
   this->parameters                           = parameters;
   this->plots                                = plots;

   USglobal = new US_Config();
   setPalette( PALET_FRAME );
   setWindowTitle( us_tr( "US-SOMO: SAXS-HPLC Scale Analysis" ) );

   plot_zoomer = (ScrollZoomer *) 0;

   setupGUI();

   global_Xpos += 30;
   global_Ypos += 30;

   setGeometry( global_Xpos, global_Ypos, 0, 0 );
   analysis();
   update_plot();
}

US_Hydrodyn_Mals_Scale_Trend::~US_Hydrodyn_Mals_Scale_Trend()
{
}

void US_Hydrodyn_Mals_Scale_Trend::setupGUI()
{
   // fix up missing parameters
   {
      QStringList fixup;
      fixup 
         << "title" << "unknown"
         << "xbottom" << "q [Angstrom^-1]"
         << "yleft" << "unknown"
         ;
      
      for ( int i = 0; i < (int) fixup.size(); i += 2 ) {
         if ( !parameters.count( fixup[ i ]) ) {
            parameters[ fixup[ i ] ] = fixup[ i + 1 ];
         }
      }
   }

   int minHeight1  = 30;


   lbl_results = new QLabel      ( "", this );
   lbl_results ->setAlignment    ( Qt::AlignLeft | Qt::AlignVCenter );
   lbl_results ->setMinimumHeight( minHeight1 );
   lbl_results ->setPalette      ( PALET_LABEL );
   AUTFBACK( lbl_results );
   lbl_results ->setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold ) );

   pb_help =  new QPushButton ( us_tr( "Help" ), this );
   pb_help -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1) );
   pb_help -> setMinimumHeight( minHeight1 );
   pb_help -> setPalette      ( PALET_PUSHB );
   connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );

   pb_close =  new QPushButton ( us_tr( "Close" ), this );
   pb_close -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1) );
   pb_close -> setMinimumHeight( minHeight1 );
   pb_close -> setPalette      ( PALET_PUSHB );
   connect( pb_close, SIGNAL( clicked() ), SLOT( cancel() ) );

//   plot = new QwtPlot(this);
   usp_plot = new US_Plot( plot, "", "", "", this );
   connect( (QWidget *)plot->titleLabel(), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_plot( const QPoint & ) ) );
   ((QWidget *)plot->titleLabel())->setContextMenuPolicy( Qt::CustomContextMenu );
   connect( (QWidget *)plot->axisWidget( QwtPlot::yLeft ), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_plot( const QPoint & ) ) );
   ((QWidget *)plot->axisWidget( QwtPlot::yLeft ))->setContextMenuPolicy( Qt::CustomContextMenu );
   connect( (QWidget *)plot->axisWidget( QwtPlot::xBottom ), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_plot( const QPoint & ) ) );
   ((QWidget *)plot->axisWidget( QwtPlot::xBottom ))->setContextMenuPolicy( Qt::CustomContextMenu );
#if QT_VERSION < 0x040000
   plot->enableOutline(true);
   plot->setOutlinePen(Qt::white);
   plot->setOutlineStyle(Qwt::VLine);
   plot->enableGridXMin();
   plot->enableGridYMin();
   plot->setPalette( PALET_NORMAL );
   AUTFBACK( plot );
   plot->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
   plot->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
#else
   grid = new QwtPlotGrid;
   grid->enableXMin( true );
   grid->enableYMin( true );
   plot->setPalette( PALET_NORMAL );
   AUTFBACK( plot );
   grid->setMajorPen( QPen( USglobal->global_colors.major_ticks, 0, Qt::DotLine ) );
   grid->setMinorPen( QPen( USglobal->global_colors.minor_ticks, 0, Qt::DotLine ) );
   grid->attach( plot );
#endif
   plot->setAxisTitle(QwtPlot::xBottom, parameters[ "xbottom" ] );
   plot->setAxisTitle(QwtPlot::yLeft  , parameters[ "yleft"   ] );

#if QT_VERSION < 0x040000
   plot->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   plot->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   plot->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
#if QT_VERSION < 0x040000
   plot->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   plot->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
#if QT_VERSION < 0x040000
   plot->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   plot->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
//    plot->setMargin(USglobal->config_list.margin);
   plot->setTitle( parameters[ "title" ] );
   plot->setCanvasBackground(USglobal->global_colors.plot);

   QVBoxLayout * background = new QVBoxLayout( this ); background->setContentsMargins( 0, 0, 0, 0 ); background->setSpacing( 0 );
   // background->addSpacing(4);

   // background->addWidget( lbl_title );
   // background->addWidget( lbl_credits_1 );
   // background->addSpacing( 4 );
   background->addWidget( plot );

   background->addWidget( lbl_results );

   QHBoxLayout * hbl_bottom = new QHBoxLayout(); hbl_bottom->setContentsMargins( 0, 0, 0, 0 ); hbl_bottom->setSpacing( 0 );
   //   hbl_bottom->addSpacing( 4 );
   hbl_bottom->addWidget ( pb_help );
   //   hbl_bottom->addSpacing( 4 );
   hbl_bottom->addWidget ( pb_close );
   //   hbl_bottom->addSpacing( 4 );

   background->addLayout ( hbl_bottom );
   //   background->addSpacing( 4 );
}

void US_Hydrodyn_Mals_Scale_Trend::cancel()
{
   close();
}

void US_Hydrodyn_Mals_Scale_Trend::help()
{
   US_Help *online_help;
   online_help = new US_Help( this );
   online_help->show_help("manual/somo/mals_scale_trend.html");
}

void US_Hydrodyn_Mals_Scale_Trend::closeEvent( QCloseEvent *e )
{

   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void US_Hydrodyn_Mals_Scale_Trend::update_plot()
{
   us_qdebug( "update plot" );

   if ( plot_zoomer )
   {
      delete plot_zoomer;
      plot_zoomer = (ScrollZoomer *) 0;
   }

   if ( !slopes_x.size() ) {
      close();
      return;
   }

   int use_line_width = 1;

   QwtSymbol sym;
   sym.setSize( use_line_width * 5 + 3 );
   sym.setStyle( QwtSymbol::Diamond );
   sym.setBrush( Qt::cyan );
   sym.setPen  ( QPen( Qt::cyan ) );

   double use_x_min = slopes_x.front();
   double use_x_max = slopes_x.back();
   double use_y_min = 1e99;
   double use_y_max = -1e99;

   {
      QString this_name = QString( "slope of fits" );

      QPen use_pen = QPen( Qt::cyan, use_line_width, Qt::SolidLine );
#if QT_VERSION >= 0x040000
      QwtPlotCurve *curve = new QwtPlotCurve( this_name );
      curve->setStyle ( QwtPlotCurve::NoCurve );
      curve->setSymbol( new QwtSymbol( sym.style(), sym.brush(), sym.pen(), sym.size() ) );
      curve->setSamples  ( & (slopes_x[ 0 ]), &(slopes[ 0 ]), slopes_x.size() );
      curve->attach   ( plot );
#else
      long curve = plot->insertCurve( this_name );
      plot->setCurveStyle ( curve, QwtCurve::NoCurve );
      plot->setCurveSymbol( curve, sym );
      plot->setCurveData  ( curve, & (slopes_x[ 0 ]), &(slopes[ 0 ]), slopes_x.size() );
#endif
   }

   us_qdebug( "scale1" ); 

   for ( int i = 0; i < (int) slopes.size(); ++i ) {
      QString this_name = QString( "slope of fits error bars" );

      double x[ 2 ];
      double y[ 2 ];

      x[ 0 ] = x[ 1 ] = slopes_x[ i ];
      y[ 0 ] = slopes[ i ] - slopes_e[ i ];
      y[ 1 ] = slopes[ i ] + slopes_e[ i ];

      if ( use_y_min > y[ 0 ] ) {
         use_y_min = y[ 0 ];
      }
      if ( use_y_max < y[ 1 ] ) {
         use_y_max = y[ 1 ];
      }

      QPen use_pen = QPen( Qt::darkRed, use_line_width, Qt::SolidLine );
#if QT_VERSION >= 0x040000
      QwtPlotCurve *curve = new QwtPlotCurve( this_name + "_errorbar" );
      curve->setStyle ( QwtPlotCurve::Lines );
      curve->setPen   ( use_pen );
      curve->setSamples  ( x, y, 2 );
      curve->attach   ( plot );
#else
      long curve = plot->insertCurve( this_name + "_errorbar" );
      plot->setCurveStyle ( curve, QwtCurve::Lines );
      plot->setCurvePen   ( curve, use_pen  );
      plot->setCurveData  ( curve, x, y, 2 );
#endif
   }
   us_qdebug( "scale2" ); 

   {
      QString this_name = QString( "fit of slope of fits error bars" );

      double x[ 2 ];
      double y[ 2 ];

      x[ 0 ] = slopes_x.front();
      y[ 0 ] = fit_a + fit_b * x[ 0 ];
      x[ 1 ] = slopes_x.back();
      y[ 1 ] = fit_a + fit_b * x[ 1 ];

      QPen use_pen = QPen( Qt::green, use_line_width, Qt::SolidLine );
#if QT_VERSION >= 0x040000
      QwtPlotCurve *curve = new QwtPlotCurve( this_name + "_fitline" );
      curve->setStyle ( QwtPlotCurve::Lines );
      curve->setPen   ( use_pen );
      curve->setSamples  ( x, y, 2 );
      curve->attach   ( plot );
#else
      long curve = plot->insertCurve( this_name + "_errorbar" );
      plot->setCurveStyle ( curve, QwtCurve::Lines );
      plot->setCurvePen   ( curve, use_pen  );
      plot->setCurveData  ( curve, x, y, 2 );
#endif
   }

   us_qdebug( "scale3" ); 
   {
      QPen use_pen = QPen( Qt::red, use_line_width, Qt::DashDotLine );
      QFont use_font = QFont("Helvetica", 11, QFont::Bold );
#if QT_VERSION >= 0x040000
      Qt::Alignment align;
#else
      int align;
#endif

      if ( use_qmin ) {
         align = Qt::AlignRight | Qt::AlignTop;
         double pos = qmin;
         QString text = "Start";
#if QT_VERSION >= 0x040000
         QwtPlotMarker * marker = new QwtPlotMarker;
         marker->setLineStyle       ( QwtPlotMarker::VLine );
         marker->setLinePen         ( use_pen );
         marker->setLabelOrientation( Qt::Horizontal );
         marker->setXValue          ( pos );
         marker->setLabelAlignment  ( align );
         {
            QwtText qwtt( text );
            qwtt.setFont( use_font );
            marker->setLabel           ( qwtt );
         }
         marker->attach             ( plot );
#else
         long marker = plot->insertMarker();
         plot->setMarkerLineStyle ( marker, QwtMarker::VLine );
         plot->setMarkerPos       ( marker, pos, 0e0 );
         plot->setMarkerLabelAlign( marker, align );
         plot->setMarkerPen       ( marker, use_pen );
         plot->setMarkerFont      ( marker, use_font );
         plot->setMarkerLabelText ( marker, text );
#endif
      }
      if ( use_qmax ) {
         align = Qt::AlignLeft | Qt::AlignTop;
         double pos = qmax;
         QString text = "End";
#if QT_VERSION >= 0x040000
         QwtPlotMarker * marker = new QwtPlotMarker;
         marker->setLineStyle       ( QwtPlotMarker::VLine );
         marker->setLinePen         ( use_pen );
         marker->setLabelOrientation( Qt::Horizontal );
         marker->setXValue          ( pos );
         marker->setLabelAlignment  ( align );
         {
            QwtText qwtt( text );
            qwtt.setFont( use_font );
            marker->setLabel           ( qwtt );
         }
         marker->attach             ( plot );
#else
         long marker = plot->insertMarker();
         plot->setMarkerLineStyle ( marker, QwtMarker::VLine );
         plot->setMarkerPos       ( marker, pos, 0e0 );
         plot->setMarkerLabelAlign( marker, align );
         plot->setMarkerPen       ( marker, use_pen );
         plot->setMarkerFont      ( marker, use_font );
         plot->setMarkerLabelText ( marker, text );
#endif
      }
   }      

   us_qdebug( "scale4" ); 

   plot->setAxisScale( QwtPlot::xBottom, use_x_min, use_x_max );
   plot->setAxisScale( QwtPlot::yLeft  , use_y_min, use_y_max );

   if ( !plot_zoomer )
   {
      plot_zoomer = new ScrollZoomer(plot->canvas());
      plot_zoomer->setRubberBandPen(QPen(Qt::yellow, 0, Qt::DotLine));
      plot_zoomer->setTrackerPen(QPen(Qt::red));
   }

   plot->replot();
}

void US_Hydrodyn_Mals_Scale_Trend::analysis()
{
   us_qdebug( "analysis" );

   // plots map has q, I, maybe:error

   vector < QString > names;

   map < QString, vector < double > > q;
   map < QString, vector < double > > I;
   map < QString, vector < double > > e;

   for ( map < QString, vector < vector < double > > >::iterator it = plots.begin();
         it != plots.end();
         ++it ) {
      names.push_back( it->first );
      q[ it->first ] = it->second[ 0 ];
      I[ it->first ] = it->second[ 1 ];
      e[ it->first ] = it->second[ 2 ];

   //    us_qdebug( US_Vector::qs_vector3( QString( "plot '%1'" ).arg( it->first ),
   //                                   it->second[ 0 ],
   //                                   it->second[ 1 ],
   //                                   it->second[ 2 ] ) );
   }

   if ( !names.size() || !q.count( names[ 0 ] ) || !q[ names[ 0 ] ].size() ) {
      us_qdebug( "scale trend: no data" );
      cancel();
      return;
   }

   int q_size = (int) q[ names[ 0 ] ].size();
   vector < double > q_ref = q[ names[ 0 ] ];

   bool use_errors = true;

   for ( int i = 0; i < (int) names.size(); ++i ) {
      if ( q_ref != q[ names[ i ] ] ) {
         us_qdebug( US_Vector::qs_vector2( 
                                       QString( "scale trend: incompatible grids %1 %2" )
                                       .arg( names[ 0 ] )
                                       .arg( names[ i ] )
                                       , q_ref, q[ names[ i ] ] ) 
                 );
         return;
      }
           
      if ( use_errors && 
           ( !e.count( names[ i ] ) || 
             q_size != (int) e[ names[ i ] ].size() || 
             !US_Saxs_Util::is_nonzero_vector( e[ names[ i ] ] ) )
             ) {
         use_errors = false;
      }

      // us_qdebug( QString( "%1 %2 %3 %4\n" )
      //         .arg( names[ i ] )
      //         .arg( q[ names[ i ] ].size() )
      //         .arg( I[ names[ i ] ].size() )
      //         .arg( e[ names[ i ] ].size() ) );
   }

   // 
   use_qmin = parameters.count( "qmin" );
   use_qmax = parameters.count( "qmax" );
   if ( use_qmin ) {
      qmin = parameters[ "qmin" ].toDouble();
   }
   if ( use_qmax ) {
      qmax = parameters[ "qmax" ].toDouble();
   }

   // setup a linear fit and plot slopes per q value

   slopes_x.clear( );
   slopes  .clear( );
   slopes_e.clear( );


   for ( int i = 0; i < q_size; ++i ) {
      vector < double > x;
      vector < double > y;
      vector < double > s;

      if ( use_qmin && q_ref[ i ] < qmin ) {
         continue;
      }

      if ( use_qmax && q_ref[ i ] > qmax ) {
         continue;
      }

      for ( int j = 0; j < (int) names.size(); ++j ) {
         x.push_back( (double) j );
         y.push_back( I[ names[ j ] ][ i ] );
         if ( use_errors ) {
            s.push_back( e[ names[ j ] ][ i ] );
         }
      }
      // linear fit, push back slope
      double a;
      double b;
      double siga;
      double sigb;
      double chi2;

      // us_qdebug( QString( "%1:" ).arg( q_ref[ i ] ) );
      if ( use_errors ) {
         // us_qdebug( US_Vector::qs_vector3( "xys", x, y, s ) );
         US_Saxs_Util::linear_fit( x, y, s, a, b, siga, sigb, chi2 );
      } else {
         // us_qdebug( US_Vector::qs_vector2( "xy", x, y ) );
         US_Saxs_Util::linear_fit( x, y, a, b, siga, sigb, chi2 );
      }         
      
      slopes_x.push_back( q_ref[ i ] );
      slopes  .push_back( b );
      slopes_e.push_back( sigb );
   }

   US_Saxs_Util::linear_fit( slopes_x, slopes, slopes_e, fit_a, fit_b, fit_siga, fit_sigb, fit_chi2 );

   {
      QString error_msg;
      double r;
      bool r_ok = US_Saxs_Util::pearsonpmcc( slopes_x, slopes, r, error_msg );

      QString msg = 
         QString( "Slope = %1   r = %2" )
         .arg( fit_b )
         .arg( r_ok ?
               QString( "%1" ).arg( r ) :
               error_msg );

      if ( slopes.size() ) {
         double syy = 0e0;

         for ( int i = 0; i < (int) slopes.size(); ++i ) {
            syy += slopes[ i ] * slopes[ i ];
         }

         double rmsd = sqrt( syy ); // / (double) slopes.size();

         msg += QString( "  RMSD = %1" ).arg( rmsd );
      }
      lbl_results->setText( msg );
   }

   // us_qdebug( US_Vector::qs_vector3( "slopes", slopes_x, slopes, slopes_e ) );
}

void US_Hydrodyn_Mals_Scale_Trend::usp_config_plot( const QPoint & ) {
   US_PlotChoices *uspc = new US_PlotChoices( usp_plot );
   uspc->exec();
   delete uspc;
}
