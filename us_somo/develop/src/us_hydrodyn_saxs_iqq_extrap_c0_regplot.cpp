#include "../include/us3_defines.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_hydrodyn_saxs.h"
#include "../include/us_saxs_util.h"
#include "../include/us_hydrodyn_saxs_iqq_extrap_c0_regplot.h"
//Added by qt3to4:
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCloseEvent>
#include <QFrame>
#include <QLabel>
#include <qwt_symbol.h>

US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot::US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot(
                                                                                void *                          us_hydrodyn,
                                                                                QString                         y_axis_title,
                                                                                bool                            merge_mode,
                                                                                vector < double >               reg_q,
                                                                                vector < vector < double > >    reg_x,
                                                                                vector < vector < double > >    reg_y,
                                                                                vector < vector < double > >    reg_e,
                                                                                vector < double >               reg_a,
                                                                                vector < double >               reg_b,
                                                                                vector < double >               reg_siga,
                                                                                QWidget *                       p,
                                                                                const char *
                                                                                ) : QFrame( p )
{
   this->us_hydrodyn  = us_hydrodyn;
   this->y_axis_title = y_axis_title;
   this->merge_mode   = merge_mode;
   this->reg_q        = reg_q;
   this->reg_x        = reg_x;
   this->reg_y        = reg_y;
   this->reg_e        = reg_e;
   this->reg_a        = reg_a;
   this->reg_b        = reg_b;
   this->reg_siga     = reg_siga;

   cur_index   = 0;
   plot_zoomer = (ScrollZoomer *) 0;

   USglobal = new US_Config();
   // top-level window, not a child widget embedded in the SAS window
   setWindowFlags( Qt::Window );
   setPalette( PALET_FRAME );
   setWindowTitle( us_tr( "US-SOMO: SAXS: Extrapolation to zero concentration - per-q regression" ) );

   setupGUI();

   global_Xpos += 30;
   global_Ypos += 30;
   setGeometry( global_Xpos, global_Ypos, 700, 560 );

   set_index( 0 );
}

US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot::~US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot()
{
}

void US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot::setupGUI()
{
   int minHeight1 = 30;

   lbl_info = new QLabel(
                         merge_mode
                         ? us_tr( "Each point is one input curve at its concentration; the line is the "
                                  "inverse-variance weighted merge (the Primus result) at this q.\n"
                                  "Drag the wheel to scroll q; use Prev/Next to step one q at a time." )
                         : us_tr( "Each point is one input curve at its concentration; the line is the linear "
                                  "fit whose intercept at c=0 is the extrapolated value.\n"
                                  "Drag the wheel to scroll q; use Prev/Next to step one q at a time." ),
                         this );
   lbl_info->setAlignment( Qt::AlignCenter | Qt::AlignVCenter );
   lbl_info->setMinimumHeight( minHeight1 );
   lbl_info->setPalette( PALET_LABEL );
   AUTFBACK( lbl_info );
   lbl_info->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );

   usp_plot = new US_Plot( plot, "", "", "", this );
   connect( (QWidget *)plot->titleLabel(), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_plot( const QPoint & ) ) );
   ((QWidget *)plot->titleLabel())->setContextMenuPolicy( Qt::CustomContextMenu );
   connect( (QWidget *)plot->axisWidget( QwtPlot::yLeft ), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_plot( const QPoint & ) ) );
   ((QWidget *)plot->axisWidget( QwtPlot::yLeft ))->setContextMenuPolicy( Qt::CustomContextMenu );
   connect( (QWidget *)plot->axisWidget( QwtPlot::xBottom ), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_plot( const QPoint & ) ) );
   ((QWidget *)plot->axisWidget( QwtPlot::xBottom ))->setContextMenuPolicy( Qt::CustomContextMenu );

   grid = new QwtPlotGrid;
   grid->enableXMin( true );
   grid->enableYMin( true );
   plot->setPalette( PALET_NORMAL );
   AUTFBACK( plot );
   grid->setMajorPen( QPen( USglobal->global_colors.major_ticks, 0, Qt::DotLine ) );
   grid->setMinorPen( QPen( USglobal->global_colors.minor_ticks, 0, Qt::DotLine ) );
   grid->attach( plot );

   plot->setAxisTitle( QwtPlot::xBottom, us_tr( "concentration [mg/mL]" ) );
   plot->setAxisTitle( QwtPlot::yLeft, y_axis_title );
   plot->setAxisFont( QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   plot->setAxisFont( QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   plot->setCanvasBackground( USglobal->global_colors.plot );

   double wheel_max = reg_q.size() ? (double)( reg_q.size() - 1 ) : 0e0;
   qwtw_wheel = new QwtWheel( this );
   qwtw_wheel->setOrientation( Qt::Horizontal );
   qwtw_wheel->setRange( 0, wheel_max );
   // continuous/fine so dragging scrolls smoothly across the whole q range (the
   // plot snaps to the nearest q); Prev/Next do the integer stepping
   qwtw_wheel->setSingleStep( wheel_max > 0e0 ? wheel_max / 1000e0 : 1e0 );
   qwtw_wheel->setMass( 1e0 );                 // inertia: a flick keeps spinning
   qwtw_wheel->setTracking( true );
   qwtw_wheel->setMinimumHeight( minHeight1 );
   qwtw_wheel->setMinimumWidth( 200 );
   qwtw_wheel->setToolTip( us_tr( "Drag to scroll smoothly through the q grid" ) );
   connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( wheel_changed( double ) ) );

   lbl_results = new QLabel( "", this );
   lbl_results->setAlignment( Qt::AlignCenter | Qt::AlignVCenter );
   lbl_results->setMinimumHeight( minHeight1 );
   lbl_results->setPalette( PALET_LABEL );
   AUTFBACK( lbl_results );
   lbl_results->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold ) );

   pb_prev = new QPushButton( us_tr( "Prev" ), this );
   pb_prev->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1 ) );
   pb_prev->setMinimumHeight( minHeight1 );
   pb_prev->setPalette( PALET_PUSHB );
   connect( pb_prev, SIGNAL( clicked() ), SLOT( prev() ) );

   pb_next = new QPushButton( us_tr( "Next" ), this );
   pb_next->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1 ) );
   pb_next->setMinimumHeight( minHeight1 );
   pb_next->setPalette( PALET_PUSHB );
   connect( pb_next, SIGNAL( clicked() ), SLOT( next() ) );

   pb_help = new QPushButton( us_tr( "Help" ), this );
   pb_help->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1 ) );
   pb_help->setMinimumHeight( minHeight1 );
   pb_help->setPalette( PALET_PUSHB );
   connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );

   pb_close = new QPushButton( us_tr( "Close" ), this );
   pb_close->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1 ) );
   pb_close->setMinimumHeight( minHeight1 );
   pb_close->setPalette( PALET_PUSHB );
   connect( pb_close, SIGNAL( clicked() ), SLOT( cancel() ) );

   QHBoxLayout * hbl_wheel = new QHBoxLayout; hbl_wheel->setContentsMargins( 4, 0, 4, 0 ); hbl_wheel->setSpacing( 6 );
   hbl_wheel->addWidget( pb_prev );
   hbl_wheel->addWidget( qwtw_wheel );
   hbl_wheel->addWidget( pb_next );

   QHBoxLayout * hbl_bottom = new QHBoxLayout; hbl_bottom->setContentsMargins( 0, 0, 0, 0 ); hbl_bottom->setSpacing( 0 );
   hbl_bottom->addWidget( pb_help );
   hbl_bottom->addWidget( pb_close );

   QVBoxLayout * background = new QVBoxLayout( this ); background->setContentsMargins( 0, 0, 0, 0 ); background->setSpacing( 0 );
   background->addWidget( lbl_info );
   background->addWidget( plot );
   background->addLayout( hbl_wheel );
   background->addWidget( lbl_results );
   background->addLayout( hbl_bottom );
}

void US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot::clear_plot_items()
{
   for ( int i = 0; i < (int) plot_curves.size(); i++ )
   {
      plot_curves[ i ]->detach();
      delete plot_curves[ i ];
   }
   plot_curves.clear();

   for ( int i = 0; i < (int) plot_markers.size(); i++ )
   {
      plot_markers[ i ]->detach();
      delete plot_markers[ i ];
   }
   plot_markers.clear();
}

int US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot::clamp_index( int i )
{
   if ( i < 0 )
   {
      i = 0;
   }
   if ( i > (int) reg_q.size() - 1 )
   {
      i = (int) reg_q.size() - 1;
   }
   return i;
}

// used by Prev/Next and initial show: snaps the wheel to the exact index
void US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot::set_index( int i )
{
   if ( !reg_q.size() )
   {
      return;
   }
   cur_index = clamp_index( i );

   qwtw_wheel->blockSignals( true );
   qwtw_wheel->setValue( (double) cur_index );
   qwtw_wheel->blockSignals( false );

   update_plot();
}

// driven by dragging the wheel: update the plot to the nearest q WITHOUT snapping
// the wheel back, so it scrolls smoothly (and keeps its inertia)
void US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot::wheel_changed( double val )
{
   if ( !reg_q.size() )
   {
      return;
   }
   int i = clamp_index( (int) qRound( val ) );
   if ( i != cur_index )
   {
      cur_index = i;
      update_plot();
   }
}

void US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot::prev()
{
   set_index( cur_index - 1 );
}

void US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot::next()
{
   set_index( cur_index + 1 );
}

void US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot::update_plot()
{
   clear_plot_items();

   if ( plot_zoomer )
   {
      delete plot_zoomer;
      plot_zoomer = (ScrollZoomer *) 0;
   }

   int i = cur_index;
   if ( i < 0 || i >= (int) reg_q.size() || !reg_x[ i ].size() )
   {
      plot->replot();
      return;
   }

   const vector < double > & x = reg_x[ i ];
   const vector < double > & y = reg_y[ i ];
   const vector < double > & e = reg_e[ i ];
   double a    = reg_a[ i ];
   double b    = reg_b[ i ];
   double siga = reg_siga[ i ];

   double x_max = 0e0;
   for ( int j = 0; j < (int) x.size(); j++ )
   {
      if ( x[ j ] > x_max )
      {
         x_max = x[ j ];
      }
   }
   double x_lo = -0.04 * x_max;
   double x_hi = 1.06 * x_max;

   double y_min = a - siga;
   double y_max = a + siga;
   for ( int j = 0; j < (int) y.size(); j++ )
   {
      double ej = ( j < (int) e.size() ) ? e[ j ] : 0e0;
      double lo = y[ j ] - ej;
      double hi = y[ j ] + ej;
      if ( lo < y_min ) y_min = lo;
      if ( hi > y_max ) y_max = hi;
   }
   if ( a < y_min ) y_min = a;
   if ( a > y_max ) y_max = a;

   // fitted line across the full x-range (so the c=0 intercept is visible)
   {
      double lx[ 2 ] = { x_lo, x_hi };
      double ly[ 2 ] = { a + b * x_lo, a + b * x_hi };
      QwtPlotCurve * curve = new QwtPlotCurve( "fit" );
      curve->setStyle( QwtPlotCurve::Lines );
      curve->setPen( QPen( Qt::green, 1, Qt::SolidLine ) );
      curve->setSamples( lx, ly, 2 );
      curve->attach( plot );
      plot_curves.push_back( curve );
   }

   // per-point error bars
   for ( int j = 0; j < (int) x.size(); j++ )
   {
      double ej = ( j < (int) e.size() ) ? e[ j ] : 0e0;
      if ( ej <= 0e0 )
      {
         continue;
      }
      double ex[ 2 ] = { x[ j ], x[ j ] };
      double ey[ 2 ] = { y[ j ] - ej, y[ j ] + ej };
      QwtPlotCurve * curve = new QwtPlotCurve( "errorbar" );
      curve->setStyle( QwtPlotCurve::Lines );
      curve->setPen( QPen( Qt::darkCyan, 1, Qt::SolidLine ) );
      curve->setSamples( ex, ey, 2 );
      curve->attach( plot );
      plot_curves.push_back( curve );
   }

   // data points
   {
      QwtPlotCurve * curve = new QwtPlotCurve( "data" );
      curve->setStyle( QwtPlotCurve::NoCurve );
      curve->setSymbol( new QwtSymbol( QwtSymbol::Ellipse, QBrush( Qt::cyan ), QPen( Qt::cyan ), QSize( 8, 8 ) ) );
      curve->setSamples( (double *) &( x[ 0 ] ), (double *) &( y[ 0 ] ), x.size() );
      curve->attach( plot );
      plot_curves.push_back( curve );
   }

   // result marker: for Zimm this is the c=0 intercept; for Primus it is the merged
   // (level) value. Draw its error bar + a highlighted diamond at x=0.
   {
      double ix[ 2 ] = { 0e0, 0e0 };
      double iy[ 2 ] = { a - siga, a + siga };
      if ( siga > 0e0 )
      {
         QwtPlotCurve * curve = new QwtPlotCurve( "result errorbar" );
         curve->setStyle( QwtPlotCurve::Lines );
         curve->setPen( QPen( Qt::red, 1, Qt::SolidLine ) );
         curve->setSamples( ix, iy, 2 );
         curve->attach( plot );
         plot_curves.push_back( curve );
      }

      QwtPlotMarker * marker = new QwtPlotMarker;
      marker->setValue( 0e0, a );
      marker->setSymbol( new QwtSymbol( QwtSymbol::Diamond, QBrush( Qt::red ), QPen( Qt::red ), QSize( 11, 11 ) ) );
      marker->attach( plot );
      plot_markers.push_back( marker );

      // vertical reference line at c=0 -- only meaningful for the Zimm extrapolation
      if ( !merge_mode )
      {
         QwtPlotMarker * vline = new QwtPlotMarker;
         vline->setLineStyle( QwtPlotMarker::VLine );
         vline->setLinePen( QPen( Qt::red, 0, Qt::DashDotLine ) );
         vline->setXValue( 0e0 );
         vline->attach( plot );
         plot_markers.push_back( vline );
      }
   }

   double y_pad = 0.06 * ( y_max - y_min );
   if ( y_pad <= 0e0 )
   {
      y_pad = 0.5 * ( qAbs( y_max ) + 1e0 );
   }
   plot->setAxisScale( QwtPlot::xBottom, x_lo, x_hi );
   plot->setAxisScale( QwtPlot::yLeft, y_min - y_pad, y_max + y_pad );

   plot->setTitle( QString( us_tr( "q = %1  (point %2 of %3)%4" ) )
                   .arg( reg_q[ i ], 0, 'g', 6 )
                   .arg( i + 1 )
                   .arg( (int) reg_q.size() )
                   .arg( merge_mode ? us_tr( "  -- weighted merge" ) : QString( "" ) ) );

   lbl_results->setText(
                        merge_mode
                        ? QString( us_tr( "q = %1    merged value = %2 ± %3    n = %4 curves" ) )
                          .arg( reg_q[ i ], 0, 'g', 6 )
                          .arg( a, 0, 'g', 6 )
                          .arg( siga, 0, 'g', 4 )
                          .arg( (int) x.size() )
                        : QString( us_tr( "q = %1    intercept (c=0) = %2 ± %3    slope = %4    n = %5 curves" ) )
                          .arg( reg_q[ i ], 0, 'g', 6 )
                          .arg( a, 0, 'g', 6 )
                          .arg( siga, 0, 'g', 4 )
                          .arg( b, 0, 'g', 6 )
                          .arg( (int) x.size() ) );

   plot_zoomer = new ScrollZoomer( plot->canvas() );
   plot_zoomer->setRubberBandPen( QPen( Qt::yellow, 0, Qt::DotLine ) );
   plot_zoomer->setTrackerPen( QPen( Qt::red ) );

   plot->replot();
}

void US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot::usp_config_plot( const QPoint & )
{
   US_PlotChoices * uspc = new US_PlotChoices( usp_plot );
   uspc->exec();
   delete uspc;
}

void US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot::help()
{
   US_Help * online_help;
   online_help = new US_Help( this );
   online_help->show_help( "manual/somo/somo_saxs_extrap_c0.html" );
}

void US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot::cancel()
{
   close();
}

void US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot::closeEvent( QCloseEvent * e )
{
   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}
