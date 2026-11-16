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
#include <QFileDialog>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QDir>
#include <QMessageBox>
#include <qwt_symbol.h>

US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot::US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot(
                                                                                void *                          us_hydrodyn,
                                                                                QString                         y_axis_title,
                                                                                double                          merge_q,
                                                                                int                             fit_broaden,
                                                                                int                             model,
                                                                                vector < double >               reg_q,
                                                                                vector < vector < double > >    reg_x,
                                                                                vector < vector < double > >    reg_y,
                                                                                vector < vector < double > >    reg_e,
                                                                                vector < double >               reg_a,
                                                                                vector < double >               reg_b,
                                                                                vector < double >               reg_c,
                                                                                vector < double >               reg_siga,
                                                                                double                          excl_conc,
                                                                                vector < double >               reg_excl_y,
                                                                                QString                         excl_name,
                                                                                vector < vector < int > >       reg_ci,
                                                                                vector < vector < double > >    reg_sx,
                                                                                vector < vector < double > >    reg_sy,
                                                                                vector < vector < double > >    reg_se,
                                                                                vector < vector < int > >       reg_sci,
                                                                                US_Hydrodyn_Saxs_Extrap_C0_Pending pending,
                                                                                void *                          saxs_window,
                                                                                QWidget *                       p,
                                                                                const char *
                                                                                ) : QFrame( p )
{
   this->us_hydrodyn  = us_hydrodyn;
   this->y_axis_title = y_axis_title;
   this->merge_q      = merge_q;
   this->fit_broaden  = fit_broaden;
   this->model        = model;
   this->reg_q        = reg_q;
   this->reg_x        = reg_x;
   this->reg_y        = reg_y;
   this->reg_e        = reg_e;
   this->reg_a        = reg_a;
   this->reg_b        = reg_b;
   this->reg_c        = reg_c;
   this->reg_siga     = reg_siga;
   this->excl_conc    = excl_conc;
   this->reg_excl_y   = reg_excl_y;
   this->excl_name    = excl_name;
   this->reg_ci       = reg_ci;
   this->reg_sx       = reg_sx;
   this->reg_sy       = reg_sy;
   this->reg_se       = reg_se;
   this->reg_sci      = reg_sci;
   this->pending      = pending;
   this->saxs_window  = saxs_window;
   pending_open       = ( saxs_window != 0 );

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

   QString info_txt =
      ( merge_q > 0e0 )
      ? us_tr( "Each point is one input curve at its concentration; the line is the linear "
               "fit whose c=0 intercept is the extrapolated value. Below the merging point "
               "the output is this intercept; at/above it the reference curve is used.\n"
               "Drag the wheel to scroll q; use Prev/Next to step one q at a time." )
      : us_tr( "Each point is one input curve at its concentration; the line is the linear "
               "fit whose intercept at c=0 is the extrapolated value.\n"
               "Drag the wheel to scroll q; use Prev/Next to step one q at a time." );
   if ( fit_broaden > 1 )
   {
      info_txt += QString( us_tr( "\nFit broadening on: the fit line's slope is smoothed over a %1-point q-window." ) )
         .arg( fit_broaden );
   }
   if ( excl_conc > 0e0 && !excl_name.isEmpty() )
   {
      info_txt += QString( us_tr( "\nOutlier QC: curve \"%1\" (conc %2) was excluded from the fit and is shown as a red x." ) )
         .arg( excl_name ).arg( excl_conc );
   }
   {
      int n_spk = 0;
      for ( int i = 0; i < (int) reg_sx.size(); i++ )
      {
         n_spk += (int) reg_sx[ i ].size();
      }
      if ( n_spk )
      {
         info_txt += QString( us_tr( "\nSpike check: %1 point(s) were fitted at their neighbours' local-fit value (cyan); "
                                     "their measured values are shown in magenta." ) ).arg( n_spk );
      }
   }
   if ( pending_open )
   {
      info_txt += us_tr( "\nKeep adds the extrapolated curve to the SAS plot (closing this window does the same); "
                         "Discard drops it. Save CSV writes these per-q fits to a file." );
   }
   lbl_info = new QLabel( info_txt, this );
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

   pb_csv = new QPushButton( us_tr( "Save CSV" ), this );
   pb_csv->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1 ) );
   pb_csv->setMinimumHeight( minHeight1 );
   pb_csv->setPalette( PALET_PUSHB );
   pb_csv->setToolTip( us_tr( "Write the per-q fits to a CSV file: for every q the points used (per curve: value on the\n"
                              "fit axis, its error, and whether it was replaced as a spike), the fit coefficients, and the\n"
                              "extrapolated value with its error." ) );
   connect( pb_csv, SIGNAL( clicked() ), SLOT( save_csv() ) );

   pb_discard = new QPushButton( us_tr( "Discard" ), this );
   pb_discard->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1 ) );
   pb_discard->setMinimumHeight( minHeight1 );
   pb_discard->setPalette( PALET_PUSHB );
   pb_discard->setToolTip( us_tr( "Close this window WITHOUT adding the extrapolated curve to the SAS plot." ) );
   connect( pb_discard, SIGNAL( clicked() ), SLOT( discard() ) );

   pb_keep = new QPushButton( pending_open ? us_tr( "Keep" ) : us_tr( "Close" ), this );
   pb_keep->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1 ) );
   pb_keep->setMinimumHeight( minHeight1 );
   pb_keep->setPalette( PALET_PUSHB );
   pb_keep->setToolTip( us_tr( "Close this window and add the extrapolated curve to the SAS plot (then, if chosen, save it)." ) );
   connect( pb_keep, SIGNAL( clicked() ), SLOT( keep() ) );
   if ( !pending_open )
   {
      pb_discard->hide();
   }

   QHBoxLayout * hbl_wheel = new QHBoxLayout; hbl_wheel->setContentsMargins( 4, 0, 4, 0 ); hbl_wheel->setSpacing( 6 );
   hbl_wheel->addWidget( pb_prev );
   hbl_wheel->addWidget( qwtw_wheel );
   hbl_wheel->addWidget( pb_next );

   QHBoxLayout * hbl_bottom = new QHBoxLayout; hbl_bottom->setContentsMargins( 0, 0, 0, 0 ); hbl_bottom->setSpacing( 0 );
   hbl_bottom->addWidget( pb_help );
   hbl_bottom->addWidget( pb_csv );
   hbl_bottom->addWidget( pb_discard );
   hbl_bottom->addWidget( pb_keep );

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

   // include the QC-excluded curve's point (drawn below as a red x) in the autoscale so it
   // stays visible -- seeing where the rejected point sits is the purpose of showing it
   bool have_excl = ( excl_conc > 0e0 && i < (int) reg_excl_y.size() && !us_isnan( reg_excl_y[ i ] ) );
   if ( have_excl )
   {
      if ( reg_excl_y[ i ] < y_min ) y_min = reg_excl_y[ i ];
      if ( reg_excl_y[ i ] > y_max ) y_max = reg_excl_y[ i ];
   }
   // likewise the measured values of replaced spikes (drawn in magenta below)
   int n_spk_here = ( i < (int) reg_sx.size() ) ? (int) reg_sx[ i ].size() : 0;
   for ( int j = 0; j < n_spk_here; j++ )
   {
      double ej = ( j < (int) reg_se[ i ].size() ) ? reg_se[ i ][ j ] : 0e0;
      if ( reg_sy[ i ][ j ] - ej < y_min )
      {
         y_min = reg_sy[ i ][ j ] - ej;
      }
      if ( reg_sy[ i ][ j ] + ej > y_max )
      {
         y_max = reg_sy[ i ][ j ] + ej;
      }
      if ( reg_sx[ i ][ j ] > x_max )
      {
         x_max = reg_sx[ i ][ j ];
         x_hi  = 1.06 * x_max;
         x_lo  = -0.04 * x_max;
      }
   }

   // fitted curve across the full x-range (so the c=0 intercept is visible). A straight
   // line for the additive/reciprocal fits; a sampled parabola for the 2nd-order virial.
   double c_coef = ( i < (int) reg_c.size() ) ? reg_c[ i ] : 0e0;
   {
      const int NS = 60;
      vector < double > lx( NS ), ly( NS );
      for ( int k = 0; k < NS; k++ )
      {
         double xx = x_lo + ( x_hi - x_lo ) * (double) k / ( NS - 1 );
         lx[ k ] = xx;
         ly[ k ] = a + b * xx + c_coef * xx * xx;
         if ( ly[ k ] < y_min ) y_min = ly[ k ];
         if ( ly[ k ] > y_max ) y_max = ly[ k ];
      }
      QwtPlotCurve * curve = new QwtPlotCurve( "fit" );
      curve->setStyle( QwtPlotCurve::Lines );
      curve->setPen( QPen( Qt::green, 1, Qt::SolidLine ) );
      curve->setSamples( &lx[ 0 ], &ly[ 0 ], NS );
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

   // measured values of points replaced in this q's fit as single-point spikes: magenta, with their
   // error bars, so it is visible why each was replaced (the fit used the cyan point at the same c)
   for ( int j = 0; j < n_spk_here; j++ )
   {
      double ej = ( j < (int) reg_se[ i ].size() ) ? reg_se[ i ][ j ] : 0e0;
      if ( ej > 0e0 )
      {
         double ex[ 2 ] = { reg_sx[ i ][ j ], reg_sx[ i ][ j ] };
         double ey[ 2 ] = { reg_sy[ i ][ j ] - ej, reg_sy[ i ][ j ] + ej };
         QwtPlotCurve * curve = new QwtPlotCurve( "spike errorbar" );
         curve->setStyle( QwtPlotCurve::Lines );
         curve->setPen( QPen( Qt::magenta, 1, Qt::SolidLine ) );
         curve->setSamples( ex, ey, 2 );
         curve->attach( plot );
         plot_curves.push_back( curve );
      }
      QwtPlotMarker * marker = new QwtPlotMarker;
      marker->setSymbol( new QwtSymbol( QwtSymbol::Ellipse, QBrush( Qt::magenta ), QPen( Qt::magenta ), QSize( 9, 9 ) ) );
      marker->setValue( reg_sx[ i ][ j ], reg_sy[ i ][ j ] );
      marker->attach( plot );
      plot_markers.push_back( marker );
   }

   // QC-excluded curve's point at this q: a red x (the same convention as removed Guinier
   // points), not part of the fit -- so the user can see what was rejected and where it sat
   if ( have_excl )
   {
      QwtPlotMarker * marker = new QwtPlotMarker;
      marker->setSymbol( new QwtSymbol( QwtSymbol::XCross, QBrush( Qt::red ), QPen( Qt::red, 2 ), QSize( 12, 12 ) ) );
      marker->setValue( excl_conc, reg_excl_y[ i ] );
      marker->attach( plot );
      plot_markers.push_back( marker );
   }

   // result marker: for Zimm this is the c=0 intercept; for absolute-scale it is the merged
   // (level) value. Draw its error bar + a highlighted diamond at x=0.
   {
      double ix[ 2 ] = { 0e0, 0e0 };
      double iy[ 2 ] = { a - siga, a + siga };
      if ( siga > 0e0 )
      {
         QwtPlotCurve * curve = new QwtPlotCurve( "result errorbar" );
         curve->setStyle( QwtPlotCurve::Lines );
         curve->setPen( QPen( Qt::red, 2, Qt::SolidLine ) );
         curve->setSamples( ix, iy, 2 );
         curve->attach( plot );
         plot_curves.push_back( curve );
      }

      QwtPlotMarker * marker = new QwtPlotMarker;
      marker->setValue( 0e0, a );
      marker->setSymbol( new QwtSymbol( QwtSymbol::Diamond, QBrush( Qt::red ), QPen( Qt::red ), QSize( 11, 11 ) ) );
      marker->attach( plot );
      plot_markers.push_back( marker );

      // vertical reference line at c=0 (marks where the fit extrapolates to); white so the
      // red intercept error bar drawn on top of it stays clearly visible
      QwtPlotMarker * vline = new QwtPlotMarker;
      vline->setLineStyle( QwtPlotMarker::VLine );
      vline->setLinePen( QPen( Qt::white, 1, Qt::DashDotLine ) );
      vline->setXValue( 0e0 );
      vline->attach( plot );
      plot_markers.push_back( vline );
   }

   double y_pad = 0.06 * ( y_max - y_min );
   if ( y_pad <= 0e0 )
   {
      y_pad = 0.5 * ( qAbs( y_max ) + 1e0 );
   }
   plot->setAxisScale( QwtPlot::xBottom, x_lo, x_hi );
   plot->setAxisScale( QwtPlot::yLeft, y_min - y_pad, y_max + y_pad );

   bool at_ref = ( merge_q > 0e0 && reg_q[ i ] >= merge_q );
   QString mode_note;
   if ( merge_q > 0e0 )
   {
      mode_note = at_ref
         ? us_tr( "  -- output: reference curve (q >= merge point)" )
         : us_tr( "  -- output: extrapolated intercept" );
   }

   plot->setTitle( QString( us_tr( "q = %1  (point %2 of %3)%4" ) )
                   .arg( reg_q[ i ], 0, 'g', 6 )
                   .arg( i + 1 )
                   .arg( (int) reg_q.size() )
                   .arg( mode_note ) );

   // goodness of fit at this q: R^2 (fraction of variance explained) and, when the data
   // carry error bars, the reduced chi^2 of the points about the plotted fit
   double ss_res = 0e0, ss_tot = 0e0, chi2 = 0e0, ybar_s = 0e0;
   int    nfit = (int) x.size(), have_err = 0;
   for ( int j = 0; j < nfit; j++ ) { ybar_s += y[ j ]; }
   if ( nfit ) { ybar_s /= (double) nfit; }
   for ( int j = 0; j < nfit; j++ )
   {
      double yp = a + b * x[ j ] + c_coef * x[ j ] * x[ j ];
      double r  = y[ j ] - yp;
      ss_res += r * r;
      ss_tot += ( y[ j ] - ybar_s ) * ( y[ j ] - ybar_s );
      double ej = ( j < (int) e.size() ) ? e[ j ] : 0e0;
      if ( ej > 0e0 ) { chi2 += ( r / ej ) * ( r / ej ); have_err++; }
   }
   int    nparam   = ( c_coef != 0e0 ) ? 3 : 2;
   double r2       = ( ss_tot > 0e0 ) ? 1e0 - ss_res / ss_tot : 1e0;
   double red_chi2 = ( have_err > nparam ) ? chi2 / ( have_err - nparam ) : 0e0;

   // for the reciprocal/virial models the plotted intercept is u = 1/I0; report both
   QString fit_desc;
   if ( model >= 1 )
   {
      double I0  = ( a != 0e0 ) ? 1e0 / a : 0e0;
      double I0e = ( a != 0e0 ) ? siga / ( a * a ) : 0e0;
      fit_desc = QString( us_tr( "u(c=0) = %1 ± %2   \342\206\222  I(0) = 1/u = %3 ± %4" ) )
                 .arg( a, 0, 'g', 6 ).arg( siga, 0, 'g', 4 )
                 .arg( I0, 0, 'g', 6 ).arg( I0e, 0, 'g', 4 );
   }
   else
   {
      fit_desc = QString( us_tr( "I(0) = %1 ± %2" ) ).arg( a, 0, 'g', 6 ).arg( siga, 0, 'g', 4 );
   }
   QString qof = ( have_err > nparam )
      ? QString( us_tr( "R\302\262 = %1   reduced \317\207\302\262 = %2" ) ).arg( r2, 0, 'f', 4 ).arg( red_chi2, 0, 'g', 4 )
      : QString( us_tr( "R\302\262 = %1" ) ).arg( r2, 0, 'f', 4 );

   lbl_results->setText( QString( us_tr( "q = %1    %2    %3    n = %4 curves%5%6" ) )
                         .arg( reg_q[ i ], 0, 'g', 6 )
                         .arg( fit_desc )
                         .arg( qof )
                         .arg( (int) x.size() )
                         .arg( n_spk_here ? QString( us_tr( "   (%1 spike replaced)" ) ).arg( n_spk_here ) : QString( "" ) )
                         .arg( at_ref ? us_tr( "   [taken from reference]" ) : QString( "" ) ) );

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

static QString us_extrap_c0_csv_quote( const QString & s )
{
   QString t = s;
   t.replace( "\"", "\"\"" );
   return "\"" + t + "\"";
}

void US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot::save_csv()
{
   US_Hydrodyn_Saxs * saxs = (US_Hydrodyn_Saxs *) saxs_window;
   QString use_dir = ( saxs && !saxs->our_saxs_options->path_load_saxs_curve.isEmpty() )
      ? saxs->our_saxs_options->path_load_saxs_curve : QDir::currentPath();
   QString base = pending.base_name.isEmpty() ? QString( "extrap_c0" ) : pending.base_name;
   QString fn = QFileDialog::getSaveFileName( this,
                                              us_tr( "Save the per-q fits as CSV" ),
                                              use_dir + "/" + base + "_fits.csv",
                                              "*.csv" );
   if ( fn.isEmpty() )
   {
      return;
   }
   if ( QFileInfo( fn ).suffix().isEmpty() )
   {
      fn += ".csv";
   }
   QFile f( fn );
   if ( !f.open( QIODevice::WriteOnly | QIODevice::Text ) )
   {
      QMessageBox::warning( this, windowTitle(),
                            QString( us_tr( "Could not open %1 for writing:\n%2" ) ).arg( fn ).arg( f.errorString() ) );
      return;
   }

   // the output curve by q (the same grid values the fits were made on)
   map < double, int > out_at;
   for ( int k = 0; k < (int) pending.q.size(); k++ )
   {
      out_at[ pending.q[ k ] ] = k;
   }
   vector < int > cols;                          // curves in the fit
   for ( int c = 0; c < (int) pending.curve_names.size(); c++ )
   {
      if ( c < (int) pending.curve_concs.size() && pending.curve_concs[ c ] > 0e0 )
      {
         cols.push_back( c );
      }
   }
   bool have_excl = ( excl_conc > 0e0 && !excl_name.isEmpty() );

   QTextStream ts( &f );
   ts << "# US-SOMO extrapolation to zero concentration: per-q fits\n";
   ts << "# curve," << us_extrap_c0_csv_quote( base ) << "\n";
   ts << "# model," << us_extrap_c0_csv_quote( pending.model_desc ) << "\n";
   ts << "# fit axis," << us_extrap_c0_csv_quote( y_axis_title ) << "\n";
   ts << "# switchover q," << ( merge_q > 0e0 ? QString::number( merge_q, 'g', 8 ) : QString( "none" ) ) << "\n";
   ts << "# fit_* columns are the per-q fit on the fit axis (for c/I fits the intercept is 1/I(0));"
         " output_I and output_sd are the extrapolated curve as added (after any splice and scale);"
         " per curve: value on the fit axis used in the fit, its error, status (fit = measured value used;"
         " replaced = a single-point spike fitted at its neighbours' local-fit value; blank = not available"
         " at this q) and, for a replaced point, its measured value\n";
   for ( int k = 0; k < (int) cols.size(); k++ )
   {
      ts << "# input," << us_extrap_c0_csv_quote( pending.curve_names[ cols[ k ] ] )
         << ",conc," << QString::number( pending.curve_concs[ cols[ k ] ], 'g', 8 ) << "\n";
   }
   if ( have_excl )
   {
      ts << "# qc_discarded," << us_extrap_c0_csv_quote( excl_name ) << ",conc," << QString::number( excl_conc, 'g', 8 ) << "\n";
   }
   QStringList hdr;
   hdr << "q" << "fit_intercept" << "fit_intercept_sd" << "fit_slope" << "fit_quadratic" << "n_fit"
       << "output_I" << "output_sd";
   for ( int k = 0; k < (int) cols.size(); k++ )
   {
      QString nm = pending.curve_names[ cols[ k ] ];
      hdr << us_extrap_c0_csv_quote( nm + " y" ) << us_extrap_c0_csv_quote( nm + " sd" )
          << us_extrap_c0_csv_quote( nm + " status" ) << us_extrap_c0_csv_quote( nm + " measured" );
   }
   if ( have_excl )
   {
      hdr << us_extrap_c0_csv_quote( excl_name + " y (qc_discarded)" );
   }
   ts << hdr.join( "," ) << "\n";

   for ( int i = 0; i < (int) reg_q.size(); i++ )
   {
      QStringList row;
      row << QString::number( reg_q[ i ], 'g', 10 )
          << QString::number( reg_a[ i ], 'g', 10 )
          << QString::number( i < (int) reg_siga.size() ? reg_siga[ i ] : 0e0, 'g', 10 )
          << QString::number( reg_b[ i ], 'g', 10 )
          << QString::number( i < (int) reg_c.size() ? reg_c[ i ] : 0e0, 'g', 10 )
          << QString::number( (int) reg_x[ i ].size() );
      map < double, int >::iterator it = out_at.find( reg_q[ i ] );
      if ( it != out_at.end() )
      {
         int k = it->second;
         row << QString::number( pending.I[ k ], 'g', 10 )
             << QString::number( k < (int) pending.I_err.size() ? pending.I_err[ k ] : 0e0, 'g', 10 );
      }
      else
      {
         row << "" << "";
      }
      for ( int k = 0; k < (int) cols.size(); k++ )
      {
         int     c = cols[ k ];
         QString yv, ev, st, mv;
         for ( int j = 0; i < (int) reg_ci.size() && j < (int) reg_ci[ i ].size(); j++ )
         {
            if ( reg_ci[ i ][ j ] == c )
            {
               yv = QString::number( reg_y[ i ][ j ], 'g', 10 );
               ev = ( j < (int) reg_e[ i ].size() ) ? QString::number( reg_e[ i ][ j ], 'g', 10 ) : QString( "" );
               st = "fit";
            }
         }
         for ( int j = 0; i < (int) reg_sci.size() && j < (int) reg_sci[ i ].size(); j++ )
         {
            if ( reg_sci[ i ][ j ] == c )
            {
               mv = QString::number( reg_sy[ i ][ j ], 'g', 10 );
               st = "replaced";
            }
         }
         row << yv << ev << st << mv;
      }
      if ( have_excl )
      {
         row << ( ( i < (int) reg_excl_y.size() && !us_isnan( reg_excl_y[ i ] ) )
                  ? QString::number( reg_excl_y[ i ], 'g', 10 ) : QString( "" ) );
      }
      ts << row.join( "," ) << "\n";
   }
   f.close();
   if ( saxs )
   {
      saxs->editor_msg( "black", QString( us_tr( "Saved the per-q fits of \"%1\" to %2\n" ) ).arg( base ).arg( fn ) );
   }
}

void US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot::discard()
{
   if ( pending_open )
   {
      pending_open = false;
      US_Hydrodyn_Saxs * saxs = (US_Hydrodyn_Saxs *) saxs_window;
      if ( saxs )
      {
         saxs->editor_msg( "dark red",
                           QString( us_tr( "Discarded the extrapolated curve \"%1\" (not added to the plot).\n" ) )
                           .arg( pending.base_name ) );
      }
   }
   close();
}

void US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot::keep()
{
   close();                                      // closeEvent adds the curve
}

void US_Hydrodyn_Saxs_Iqq_Extrap_C0_Regplot::closeEvent( QCloseEvent * e )
{
   // closing the viewer keeps the curve; only Discard drops it
   if ( pending_open )
   {
      pending_open = false;
      US_Hydrodyn_Saxs * saxs = (US_Hydrodyn_Saxs *) saxs_window;
      if ( saxs )
      {
         saxs->extrap_c0_commit( pending.base_name, pending.q, pending.I, pending.I_err, pending.conc_tag,
                                 pending.psv, pending.i0se, pending.hdr_extra, pending.offer_save, pending.msg_tail );
      }
   }
   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}
