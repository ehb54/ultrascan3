#include "../include/us3_defines.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_hydrodyn_saxs_buffer.h"
#include "../include/us_hydrodyn_saxs_buffer_frames.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QSplitter>
#include <QScrollArea>
#include <QFontMetrics>
#include <QScreen>
#include <QGuiApplication>
#include <QIntValidator>
#include <QDoubleValidator>
#include <QCloseEvent>
#include <QDir>
#include <qwt_symbol.h>
#include <qwt_scale_engine.h>
#include <algorithm>
#include <cmath>
#include <set>

// the q grids of two frames are the same when their q differ by less than this (as the Data Utility's averages)
#define UHSBF_Q_TOL 5e-6

// the upper 0.1% point of a reduced chi^2 with n degrees of freedom (Wilson-Hilferty)
static double us_frames_chi2_limit( int n )
{
   if ( n < 1 )
   {
      return 0e0;
   }
   double a = 2e0 / ( 9e0 * n );
   double c = 1e0 - a + 3.090232e0 * sqrt( a );
   return c * c * c;
}

static double us_frames_median( vector < double > v )
{
   if ( !v.size() )
   {
      return 0e0;
   }
   sort( v.begin(), v.end() );
   size_t n = v.size();
   return n % 2 ? v[ n / 2 ] : 0.5e0 * ( v[ n / 2 - 1 ] + v[ n / 2 ] );
}

static QString us_frames_num( double x )
{
   return QString::number( x, 'g', 4 );
}

US_Hydrodyn_Saxs_Buffer_Frames::US_Hydrodyn_Saxs_Buffer_Frames(
                                                                US_Hydrodyn_Saxs_Buffer * buffer,
                                                                QStringList               files,
                                                                QWidget *                 p
                                                                ) : QFrame( p, Qt::Window )
{
   this->buffer = buffer;
   setAttribute( Qt::WA_DeleteOnClose );
   USglobal = new US_Config();
   setPalette( PALET_FRAME );
   setWindowTitle( us_tr( "US-SOMO: SAXS Data Utility : Buffer frames" ) );
   plot_zoomer = (ScrollZoomer *) 0;
   le_wheel    = (mQLineEdit *) 0;
   log_y       = false;
   suppress    = true;
   It_ok       = false;

   // the frames in frame order: by their number when the names are numbered frames, else as selected
   QStringList numbers;
   numbered = buffer->frame_numbers( files, numbers );
   vector < int > order( files.size() );
   for ( int i = 0; i < (int) files.size(); i++ )
   {
      order[ i ] = i;
   }
   if ( numbered )
   {
      stable_sort( order.begin(), order.end(),
                   [ & ]( int a, int b ) { return numbers[ a ].toULongLong() < numbers[ b ].toULongLong(); } );
   }
   for ( int k = 0; k < (int) order.size(); k++ )
   {
      this->files << files[ order[ k ] ];
      frame.push_back( numbered ? (double) numbers[ order[ k ] ].toULongLong() : (double) ( k + 1 ) );
      frame_text << ( numbered ? numbers[ order[ k ] ] : QString::number( k + 1 ) );
   }

   // sd on every point of every frame, and one q grid, for the chi^2 measures
   have_sd   = true;
   same_grid = true;
   const vector < double > & q0 = buffer->f_qs[ this->files[ 0 ] ];
   double q_lo = 1e300;
   double q_hi = 0e0;
   for ( int k = 0; k < (int) this->files.size(); k++ )
   {
      const QString & f = this->files[ k ];
      const vector < double > & q = buffer->f_qs[ f ];
      if ( !buffer->f_errors.count( f ) || buffer->f_errors[ f ].size() != q.size() )
      {
         have_sd = false;
      } else {
         for ( int i = 0; i < (int) q.size(); i++ )
         {
            if ( !( buffer->f_errors[ f ][ i ] > 0e0 ) )
            {
               have_sd = false;
               break;
            }
         }
      }
      if ( q.size() != q0.size() || buffer->f_Is[ f ].size() != q.size() )
      {
         same_grid = false;
      } else {
         for ( int i = 0; i < (int) q.size(); i++ )
         {
            if ( fabs( q[ i ] - q0[ i ] ) > UHSBF_Q_TOL )
            {
               same_grid = false;
               break;
            }
         }
      }
      if ( q.size() )
      {
         q_lo = min( q_lo, q[ 0 ] );
         q_hi = max( q_hi, q.back() );
      }
   }

   setupGUI();

   // by default the low q, where the elution of the sample shows first
   le_q_from->setText( QString::number( q_lo, 'g', 6 ) );
   le_q_to  ->setText( QString::number( q_hi > 0.05 && q_lo < 0.05 ? 0.05 : q_hi, 'g', 6 ) );
   compute_It();

   global_Xpos += 30;
   global_Ypos += 30;
   // no larger than the Data Utility window it is opened from, which is fitted to the part of the screen that is
   // seen, nor than the screen: the plots share the height left by the fields and the text
   int want_w = 1050;
   int want_h = 860;
   int x0     = global_Xpos;
   int y0     = global_Ypos;
   {
      QRect     du     = buffer->frameGeometry();
      QScreen * screen = QGuiApplication::screenAt( du.center() );
      if ( !screen )
      {
         screen = QGuiApplication::primaryScreen();
      }
      if ( screen )
      {
         // room for this window's frame: about 20 pixels in width, 40 in height with its title
         QRect area = screen->availableGeometry();
         if ( buffer->isVisible() && du.width() > 400 && du.height() > 300 && area.intersects( du ) )
         {
            area = area.intersected( du );
         }
         int max_w = area.width() - 20;
         int max_h = area.height() - 40;
         if ( max_w > 400 && want_w > max_w )
         {
            want_w = max_w;
         }
         if ( max_h > 200 && want_h > max_h )
         {
            want_h = max_h;
         }
         x0 = qMax( area.x() + 10, qMin( x0, area.x() + area.width() - want_w - 10 ) );
         y0 = qMax( area.y() + 30, qMin( y0, area.y() + area.height() - want_h - 10 ) );
      }
   }
   setGeometry( x0, y0, want_w, want_h );

   suppress = false;
   suggest();
   if ( !r1.ok )
   {
      // no suggestion: the first tenth of the frames
      int n1 = max( 2, (int) this->files.size() / 10 );
      set_region_text( le_r1_from, le_r1_to, 0, min( n1, (int) this->files.size() ) - 1 );
      regions_changed();
   }
   replot_It( true );
}

US_Hydrodyn_Saxs_Buffer_Frames::~US_Hydrodyn_Saxs_Buffer_Frames()
{
}

void US_Hydrodyn_Saxs_Buffer_Frames::setupGUI()
{
   int minHeight1 = 24;
   QFont font( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 );

   lbl_info = new QLabel( us_tr( "I(t) of the selected frames: each frame's mean intensity over the q range, against its frame number. "
                                 "Set the buffer frames before the elution (green markers) and, if the signal returns to baseline, "
                                 "after it (cyan markers): type the frame numbers, or click a frame field and move its marker with the wheel. "
                                 "For each region its I(t)'s average, sd and reduced chi^2 and its drift are given, and each frame is "
                                 "compared over its whole curve with the mean of the region's other frames (lower plot). With both regions, "
                                 "a line fitted to the I(t) of both (red, dashed) shows whether the signal returns to its level before the "
                                 "elution. The chosen frames can then be selected in the Data Utility, for Average, Set buffer and Subtract." ), this );
   lbl_info->setWordWrap( true );
   lbl_info->setPalette( PALET_LABEL );
   AUTFBACK( lbl_info );
   lbl_info->setFont( font );

   auto label = [ & ]( const QString & text ) {
      QLabel * l = new QLabel( text, this );
      l->setPalette( PALET_NORMAL );
      AUTFBACK( l );
      l->setFont( font );
      return l;
   };
   auto button = [ & ]( const QString & text ) {
      QPushButton * b = new QPushButton( text, this );
      b->setFont( font );
      b->setMinimumHeight( minHeight1 );
      b->setPalette( PALET_PUSHB );
      return b;
   };
   auto frame_edit = [ & ]( const QString & tip ) {
      mQLineEdit * le = new mQLineEdit( this );
      le->setPalette( PALET_NORMAL );
      AUTFBACK( le );
      le->setFont( font );
      le->setMinimumHeight( minHeight1 );
      le->setMaximumWidth( 90 );
      le->setValidator( new QIntValidator( 0, 2147483647, le ) );
      le->setToolTip( tip );
      connect( le, SIGNAL( textChanged( const QString & ) ), SLOT( regions_changed() ) );
      connect( le, &mQLineEdit::focussed, this, [ this, le ]( bool has_focus ) {
            if ( has_focus )
            {
               wheel_target( le );
            }
         } );
      return le;
   };

   auto stat_edit = [ & ]( const QString & tip ) {
      QLineEdit * le = new QLineEdit( this );
      le->setPalette( PALET_NORMAL );
      AUTFBACK( le );
      le->setFont( font );
      le->setMinimumHeight( minHeight1 );
      le->setMaximumWidth( 90 );
      le->setReadOnly( true );
      le->setToolTip( tip );
      return le;
   };
   QString tip_avg  = us_tr( "Average I(t) of the region's frames (1/sd^2 weighted when they have sd, as the Data Utility's Average)" );
   QString tip_sd   = us_tr( "Standard deviation of the region's I(t) values about their average: the scatter of one frame's I(t)\n"
                             "(with sd, sqrt( n sum w ( I(t) - average )^2 / ( ( n - 1 ) sum w ) ), w = 1/sd^2; without, the sample sd)" );
   QString tip_chi2 = us_tr( "Reduced chi^2 of the region's I(t) values about their average, with each frame's I(t) sd:\n"
                             "sum ( ( I(t) - average ) / sd )^2 / ( n - 1 ), about 1 when I(t) is flat within its sd" );
   le_r1_avg  = stat_edit( tip_avg );
   le_r1_sd   = stat_edit( tip_sd );
   le_r1_chi2 = stat_edit( tip_chi2 );
   le_r2_avg  = stat_edit( tip_avg );
   le_r2_sd   = stat_edit( tip_sd );
   le_r2_chi2 = stat_edit( tip_chi2 );

   lbl_q = label( us_tr( " I(t): mean intensity from q" ) );
   le_q_from = new QLineEdit( "", this );
   le_q_to   = new QLineEdit( "", this );
   QLineEdit * qedits[] = { le_q_from, le_q_to };
   for ( int k = 0; k < 2; k++ )
   {
      qedits[ k ]->setPalette( PALET_NORMAL );
      AUTFBACK( qedits[ k ] );
      qedits[ k ]->setFont( font );
      qedits[ k ]->setMinimumHeight( minHeight1 );
      qedits[ k ]->setMaximumWidth( 90 );
      qedits[ k ]->setValidator( new QDoubleValidator( qedits[ k ] ) );
      connect( qedits[ k ], SIGNAL( textChanged( const QString & ) ), SLOT( q_changed() ) );
   }
   le_q_from->setToolTip( us_tr( "Lowest q of the I(t) [1/Angstrom]" ) );
   le_q_to  ->setToolTip( us_tr( "Highest q of the I(t) [1/Angstrom]; low q shows the elution of the sample first" ) );
   pb_log_y = button( us_tr( "Log Y" ) );
   connect( pb_log_y, SIGNAL( clicked() ), SLOT( log_y_toggle() ) );

   lbl_r1     = label( us_tr( " Buffer frames before the elution: from" ) );
   le_r1_from = frame_edit( us_tr( "First buffer frame before the elution" ) );
   le_r1_to   = frame_edit( us_tr( "Last buffer frame before the elution" ) );
   pb_suggest = button( us_tr( "Suggest" ) );
   pb_suggest->setToolTip( us_tr( "Suggest the buffer frames: I(t)'s baseline level and noise are estimated (first from the first fifth of the\n"
                                  "frames); the elution starts where I(t) is more than 3 noise sd above the baseline for 3 frames in a row;\n"
                                  "the region before it ends 3 frames earlier and goes back while I(t) stays within 4 sd of the baseline.\n"
                                  "After the largest peak, the second region starts 2 frames after I(t) is back within 4 sd for good, if it is." ) );
   connect( pb_suggest, SIGNAL( clicked() ), SLOT( suggest() ) );

   cb_r2 = new QCheckBox( us_tr( " After the elution: from" ), this );
   cb_r2->setPalette( PALET_NORMAL );
   AUTFBACK( cb_r2 );
   cb_r2->setFont( font );
   cb_r2->setToolTip( us_tr( "A second region of buffer frames, after the signal has returned to baseline, compared with the first" ) );
   connect( cb_r2, SIGNAL( clicked() ), SLOT( regions_changed() ) );
   le_r2_from = frame_edit( us_tr( "First buffer frame after the elution" ) );
   le_r2_to   = frame_edit( us_tr( "Last buffer frame after the elution" ) );

   pb_wheel_dec = button( "<" );
   pb_wheel_inc = button( ">" );
   pb_wheel_dec->setMaximumWidth( 2 * minHeight1 );
   pb_wheel_inc->setMaximumWidth( 2 * minHeight1 );
   connect( pb_wheel_dec, SIGNAL( clicked() ), SLOT( wheel_dec() ) );
   connect( pb_wheel_inc, SIGNAL( clicked() ), SLOT( wheel_inc() ) );
   qwtw_wheel = new QwtWheel( this );
   qwtw_wheel->setMinimumHeight( minHeight1 );
   qwtw_wheel->setRange( 0, max( 1, (int) files.size() - 1 ) );
   qwtw_wheel->setSingleStep( 1 );
   connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
   lbl_wheel = label( us_tr( "Click a frame field to move its marker" ) );
   pb_wheel_dec->setEnabled( false );
   pb_wheel_inc->setEnabled( false );
   qwtw_wheel  ->setEnabled( false );

   usp_plot = new US_Plot( plot, "", "", "", this );
   connect( (QWidget *) plot->titleLabel(), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_plot( const QPoint & ) ) );
   ( (QWidget *) plot->titleLabel() )->setContextMenuPolicy( Qt::CustomContextMenu );
   plot->setCanvasBackground( USglobal->global_colors.plot );
   plot->setAxisTitle( QwtPlot::xBottom, numbered ? us_tr( "Frame" ) : us_tr( "Position in the selection" ) );
   plot->setAxisTitle( QwtPlot::yLeft, us_tr( "I(t)" ) );
   plot->setAxisFont( QwtPlot::yLeft, font );
   plot->setAxisFont( QwtPlot::xBottom, font );
   plot->setMinimumHeight( 120 );

   usp_plot_chi2 = new US_Plot( plot_chi2, "", "", "", this );
   connect( (QWidget *) plot_chi2->titleLabel(), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_plot_chi2( const QPoint & ) ) );
   ( (QWidget *) plot_chi2->titleLabel() )->setContextMenuPolicy( Qt::CustomContextMenu );
   plot_chi2->setCanvasBackground( USglobal->global_colors.plot );
   plot_chi2->setAxisTitle( QwtPlot::xBottom, numbered ? us_tr( "Frame" ) : us_tr( "Position in the selection" ) );
   plot_chi2->setAxisTitle( QwtPlot::yLeft, us_tr( "Reduced chi^2 vs. the others" ) );
   plot_chi2->setAxisFont( QwtPlot::yLeft, font );
   plot_chi2->setAxisFont( QwtPlot::xBottom, font );
   plot_chi2->setMinimumHeight( 80 );

   for ( mQwtPlot * pl : { plot, plot_chi2 } )
   {
      QwtPlotGrid * grid = new QwtPlotGrid;
      grid->enableXMin( true );
      grid->enableYMin( true );
      grid->setMajorPen( QPen( USglobal->global_colors.major_ticks, 0, Qt::DotLine ) );
      grid->setMinorPen( QPen( USglobal->global_colors.minor_ticks, 0, Qt::DotLine ) );
      grid->attach( pl );
   }

   lbl_stats = new QLabel( "", this );
   lbl_stats->setWordWrap( true );
   lbl_stats->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_stats );
   lbl_stats->setFont( font );
   lbl_stats->setTextInteractionFlags( Qt::TextSelectableByMouse );
   // the text scrolls: it starts at 3 lines, at least 2, and its height is set by the handle above it
   sa_stats = new QScrollArea( this );
   sa_stats->setWidget( lbl_stats );
   sa_stats->setWidgetResizable( true );
   sa_stats->setFrameShape( QFrame::NoFrame );
   sa_stats->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
   {
      int line = QFontMetrics( font ).lineSpacing();
      sa_stats->setMinimumHeight( 2 * line + 4 );
      stats_start_height = 3 * line + 4;
   }

   pb_help        = button( us_tr( "Help" ) );
   pb_select_r1   = button( us_tr( "Select the frames before the elution" ) );
   pb_select_both = button( us_tr( "Select the frames of both regions" ) );
   pb_close       = button( us_tr( "Close" ) );
   pb_select_r1  ->setToolTip( us_tr( "Select these frames in the Data Utility's file list (and nothing else)" ) );
   pb_select_both->setToolTip( us_tr( "Select the frames of both regions in the Data Utility's file list (and nothing else)" ) );
   connect( pb_help,        SIGNAL( clicked() ), SLOT( help() ) );
   connect( pb_select_r1,   SIGNAL( clicked() ), SLOT( select_r1() ) );
   connect( pb_select_both, SIGNAL( clicked() ), SLOT( select_both() ) );
   connect( pb_close,       SIGNAL( clicked() ), SLOT( close() ) );

   QHBoxLayout * hbl_q = new QHBoxLayout; hbl_q->setContentsMargins( 0, 0, 0, 0 ); hbl_q->setSpacing( 2 );
   hbl_q->addWidget( lbl_q );
   hbl_q->addWidget( le_q_from );
   hbl_q->addWidget( label( us_tr( "to" ) ) );
   hbl_q->addWidget( le_q_to );
   hbl_q->addWidget( label( us_tr( "[1/Angstrom]" ) ) );
   hbl_q->addStretch( 1 );
   hbl_q->addWidget( pb_log_y );

   // the two regions, their frame fields and their I(t) statistics in aligned columns
   QGridLayout * gl_r = new QGridLayout; gl_r->setContentsMargins( 0, 0, 0, 0 ); gl_r->setHorizontalSpacing( 2 ); gl_r->setVerticalSpacing( 1 );
   {
      QWidget * row_widgets[ 2 ][ 10 ] = {
         { lbl_r1, le_r1_from, label( us_tr( "to" ) ), le_r1_to,
           label( us_tr( "  Average" ) ), le_r1_avg, label( us_tr( " SD" ) ), le_r1_sd, label( us_tr( " chi^2" ) ), le_r1_chi2 },
         { cb_r2,  le_r2_from, label( us_tr( "to" ) ), le_r2_to,
           label( us_tr( "  Average" ) ), le_r2_avg, label( us_tr( " SD" ) ), le_r2_sd, label( us_tr( " chi^2" ) ), le_r2_chi2 }
      };
      for ( int row = 0; row < 2; row++ )
      {
         for ( int col = 0; col < 10; col++ )
         {
            gl_r->addWidget( row_widgets[ row ][ col ], row, col );
         }
      }
      gl_r->setColumnStretch( 10, 1 );
      gl_r->addWidget( pb_suggest, 0, 11 );
   }

   QHBoxLayout * hbl_wheel = new QHBoxLayout; hbl_wheel->setContentsMargins( 0, 0, 0, 0 ); hbl_wheel->setSpacing( 2 );
   hbl_wheel->addWidget( lbl_wheel, 1 );
   hbl_wheel->addWidget( pb_wheel_dec );
   hbl_wheel->addWidget( qwtw_wheel, 1 );
   hbl_wheel->addWidget( pb_wheel_inc );

   // the I(t) and chi^2 plots share their height, 3 to 1 to start with, by the handle between them; the regions'
   // text below them keeps the height set by the handle above it, so that a shorter or taller window changes the
   // plots only (until they are at their minimum)
   qs_plots = new QSplitter( Qt::Vertical, this );
   qs_plots->addWidget( plot );
   qs_plots->addWidget( plot_chi2 );
   qs_plots->addWidget( sa_stats );
   qs_plots->setChildrenCollapsible( false );
   qs_plots->setStretchFactor( 0, 3 );
   qs_plots->setStretchFactor( 1, 1 );
   qs_plots->setStretchFactor( 2, 0 );
   qs_plots->setSizes( QList < int >() << 3000 << 1000 << stats_start_height );

   QHBoxLayout * hbl_bottom = new QHBoxLayout; hbl_bottom->setContentsMargins( 0, 0, 0, 0 ); hbl_bottom->setSpacing( 0 );
   hbl_bottom->addWidget( pb_help );
   hbl_bottom->addWidget( pb_select_r1, 1 );
   hbl_bottom->addWidget( pb_select_both, 1 );
   hbl_bottom->addWidget( pb_close );

   QVBoxLayout * background = new QVBoxLayout( this ); background->setContentsMargins( 0, 0, 0, 0 ); background->setSpacing( 1 );
   background->addWidget( lbl_info );
   background->addLayout( hbl_q );
   background->addLayout( gl_r );
   background->addLayout( hbl_wheel );
   background->addWidget( qs_plots, 1 );
   background->addLayout( hbl_bottom );
}

void US_Hydrodyn_Saxs_Buffer_Frames::usp_config_plot( const QPoint & )
{
   US_PlotChoices * uspc = new US_PlotChoices( usp_plot );
   uspc->exec();
   delete uspc;
}

void US_Hydrodyn_Saxs_Buffer_Frames::usp_config_plot_chi2( const QPoint & )
{
   US_PlotChoices * uspc = new US_PlotChoices( usp_plot_chi2 );
   uspc->exec();
   delete uspc;
}

// each frame's mean intensity over the q range, and its sd from the points' sd
void US_Hydrodyn_Saxs_Buffer_Frames::compute_It()
{
   int n = (int) files.size();
   It   .assign( n, 0e0 );
   It_sd.assign( n, 0e0 );
   It_ok = true;
   double qa = le_q_from->text().toDouble();
   double qb = le_q_to  ->text().toDouble();
   for ( int k = 0; k < n; k++ )
   {
      const QString & f = files[ k ];
      const vector < double > & q = buffer->f_qs[ f ];
      const vector < double > & I = buffer->f_Is[ f ];
      bool sd = buffer->f_errors.count( f ) && buffer->f_errors[ f ].size() == q.size();
      double sum  = 0e0;
      double sum2 = 0e0;
      int    m    = 0;
      for ( int i = 0; i < (int) q.size() && i < (int) I.size(); i++ )
      {
         if ( q[ i ] >= qa && q[ i ] <= qb )
         {
            sum += I[ i ];
            if ( sd )
            {
               sum2 += buffer->f_errors[ f ][ i ] * buffer->f_errors[ f ][ i ];
            }
            m++;
         }
      }
      if ( !m )
      {
         It_ok = false;
         continue;
      }
      It   [ k ] = sum / m;
      It_sd[ k ] = sqrt( sum2 ) / m;
   }
}

// the index of the frame a field names, the nearest one when it is not a frame; -1 when empty
int US_Hydrodyn_Saxs_Buffer_Frames::frame_index( const QString & text )
{
   bool ok = false;
   double x = text.trimmed().toDouble( &ok );
   if ( !ok || !frame.size() )
   {
      return -1;
   }
   int best = 0;
   for ( int k = 1; k < (int) frame.size(); k++ )
   {
      if ( fabs( frame[ k ] - x ) < fabs( frame[ best ] - x ) )
      {
         best = k;
      }
   }
   return best;
}

bool US_Hydrodyn_Saxs_Buffer_Frames::region_range( QLineEdit * le_from, QLineEdit * le_to, int & i0, int & i1 )
{
   i0 = frame_index( le_from->text() );
   i1 = frame_index( le_to->text() );
   return i0 >= 0 && i1 >= i0;
}

void US_Hydrodyn_Saxs_Buffer_Frames::set_region_text( QLineEdit * le_from, QLineEdit * le_to, int i0, int i1 )
{
   bool was = suppress;
   suppress = true;
   le_from->setText( frame_text[ i0 ] );
   le_to  ->setText( frame_text[ i1 ] );
   suppress = was;
}

// a straight line I(t) = a + b ( frame - x0 ) fitted to the frames idx (at least 3), weighted by 1/sd^2 when the
// frames have sd; its reduced chi^2 (n - 2 degrees of freedom) and the variance of b: from the frames' sd, larger
// when I(t) scatters more than they say, or, without sd, from the scatter
bool US_Hydrodyn_Saxs_Buffer_Frames::fit_line( const vector < int > & idx, double x0,
                                               double & a, double & b, double & chi2r, double & var_b )
{
   int n = (int) idx.size();
   if ( n < 3 )
   {
      return false;
   }
   double S   = 0e0;
   double Sx  = 0e0;
   double Sy  = 0e0;
   double Sxx = 0e0;
   double Sxy = 0e0;
   for ( int k : idx )
   {
      double w = have_sd && It_sd[ k ] > 0e0 ? 1e0 / ( It_sd[ k ] * It_sd[ k ] ) : 1e0;
      double x = frame[ k ] - x0;
      S   += w;
      Sx  += w * x;
      Sy  += w * It[ k ];
      Sxx += w * x * x;
      Sxy += w * x * It[ k ];
   }
   double D = S * Sxx - Sx * Sx;
   if ( !( D > 0e0 ) )
   {
      return false;
   }
   b = ( S * Sxy - Sx * Sy ) / D;
   a = ( Sxx * Sy - Sx * Sxy ) / D;
   double chi2 = 0e0;
   for ( int k : idx )
   {
      double w   = have_sd && It_sd[ k ] > 0e0 ? 1e0 / ( It_sd[ k ] * It_sd[ k ] ) : 1e0;
      double res = It[ k ] - a - b * ( frame[ k ] - x0 );
      chi2 += w * res * res;
   }
   chi2r = chi2 / ( n - 2 );
   var_b = S / D * ( have_sd ? max( 1e0, chi2r ) : chi2r );
   return true;
}

// the average of I(t) over the region, its sd and reduced chi^2 about that average, and its drift across the
// region from a straight line fitted to it (all weighted by 1/sd^2 when the frames have sd); each frame against
// the (1/sd^2 weighted) mean of the region's other frames, over the whole curve:
// chi^2 = sum_q ( I_k - mean_others )^2 / ( sd_k^2 + var( mean_others ) ) / points
void US_Hydrodyn_Saxs_Buffer_Frames::compute_region( int i0, int i1, region_stats & r )
{
   r    = region_stats();
   r.i0 = i0;
   r.i1 = i1;
   r.n  = i1 - i0 + 1;
   if ( !It_ok || r.n < 1 )
   {
      return;
   }

   vector < int > idx;
   double S  = 0e0;
   double Sy = 0e0;
   for ( int k = i0; k <= i1; k++ )
   {
      double w = have_sd && It_sd[ k ] > 0e0 ? 1e0 / ( It_sd[ k ] * It_sd[ k ] ) : 1e0;
      S  += w;
      Sy += w * It[ k ];
      idx.push_back( k );
   }
   r.mean   = Sy / S;
   r.line_a = r.mean;
   r.line_b = 0e0;
   if ( r.n >= 2 )
   {
      double ss = 0e0;
      for ( int k = i0; k <= i1; k++ )
      {
         double w = have_sd && It_sd[ k ] > 0e0 ? 1e0 / ( It_sd[ k ] * It_sd[ k ] ) : 1e0;
         double d = It[ k ] - r.mean;
         ss += w * d * d;
      }
      r.it_sd = sqrt( r.n * ss / ( ( r.n - 1 ) * S ) );
      if ( have_sd )
      {
         r.it_chi2r_m = ss / ( r.n - 1 );
      }
   }
   double chi2r;
   double var_b;
   if ( fit_line( idx, frame[ i0 ], r.line_a, r.line_b, chi2r, var_b ) )
   {
      if ( have_sd )
      {
         r.it_chi2r = chi2r;
      }
      double span = frame[ i1 ] - frame[ i0 ];
      r.drift    = r.line_b * span;
      r.drift_sd = sqrt( var_b ) * span;
   }
   r.ok = true;

   if ( !have_sd || !same_grid || r.n < 2 )
   {
      return;
   }
   int nq = (int) buffer->f_qs[ files[ i0 ] ].size();
   vector < double > sum_w ( nq, 0e0 );
   vector < double > sum_wI( nq, 0e0 );
   for ( int k = i0; k <= i1; k++ )
   {
      const vector < double > & I = buffer->f_Is    [ files[ k ] ];
      const vector < double > & e = buffer->f_errors[ files[ k ] ];
      for ( int i = 0; i < nq; i++ )
      {
         double w = 1e0 / ( e[ i ] * e[ i ] );
         sum_w [ i ] += w;
         sum_wI[ i ] += w * I[ i ];
      }
   }
   r.avg    .resize( nq );
   r.avg_var.resize( nq );
   for ( int i = 0; i < nq; i++ )
   {
      r.avg    [ i ] = sum_wI[ i ] / sum_w[ i ];
      r.avg_var[ i ] = 1e0 / sum_w[ i ];
   }
   r.n_q        = nq;
   r.chi2_limit = us_frames_chi2_limit( nq );
   for ( int k = i0; k <= i1; k++ )
   {
      const vector < double > & I = buffer->f_Is    [ files[ k ] ];
      const vector < double > & e = buffer->f_errors[ files[ k ] ];
      double c = 0e0;
      for ( int i = 0; i < nq; i++ )
      {
         double v  = e[ i ] * e[ i ];
         double w  = 1e0 / v;
         double sw = sum_w[ i ] - w;
         double m  = ( sum_wI[ i ] - w * I[ i ] ) / sw;
         double d  = I[ i ] - m;
         c += d * d / ( v + 1e0 / sw );
      }
      c /= nq;
      r.frame_chi2r.push_back( c );
      if ( c > r.chi2_limit )
      {
         r.outliers.push_back( k );
      }
   }
   r.have_chi2 = true;
}

// with both regions: a straight line fitted to the I(t) of all their frames, from the first to the last
void US_Hydrodyn_Saxs_Buffer_Frames::compute_joint()
{
   joint = joint_fit();
   if ( !It_ok || !cb_r2->isChecked() || !r1.ok || !r2.ok )
   {
      return;
   }
   set < int > frames_set;
   for ( int k = r1.i0; k <= r1.i1; k++ )
   {
      frames_set.insert( k );
   }
   for ( int k = r2.i0; k <= r2.i1; k++ )
   {
      frames_set.insert( k );
   }
   vector < int > idx( frames_set.begin(), frames_set.end() );
   joint.n  = (int) idx.size();
   joint.k0 = idx.front();
   joint.k1 = idx.back();
   double var_b;
   if ( !fit_line( idx, frame[ joint.k0 ], joint.a, joint.b, joint.chi2r, var_b ) )
   {
      return;
   }
   double span     = frame[ joint.k1 ] - frame[ joint.k0 ];
   joint.change    = joint.b * span;
   joint.change_sd = sqrt( var_b ) * span;
   joint.ok        = true;
}

void US_Hydrodyn_Saxs_Buffer_Frames::set_region_boxes( const region_stats & r, QLineEdit * le_avg, QLineEdit * le_sd, QLineEdit * le_chi2 )
{
   le_avg ->setText( r.ok ? us_frames_num( r.mean ) : QString() );
   le_sd  ->setText( r.ok && r.n >= 2 ? us_frames_num( r.it_sd ) : QString() );
   le_chi2->setText( r.ok && r.n >= 2 && have_sd ? us_frames_num( r.it_chi2r_m ) : QString() );
}

QString US_Hydrodyn_Saxs_Buffer_Frames::region_text( const QString & name, const region_stats & r )
{
   if ( !r.ok )
   {
      return QString( us_tr( "%1: not set (from and to must be frames, from not after to)." ) ).arg( name );
   }
   QString t = QString( us_tr( "%1: frames %2 to %3 (%4 frame%5), I(t) mean %6" ) )
      .arg( name )
      .arg( frame_text[ r.i0 ] )
      .arg( frame_text[ r.i1 ] )
      .arg( r.n )
      .arg( r.n == 1 ? "" : "s" )
      .arg( us_frames_num( r.mean ) );
   if ( r.n >= 3 )
   {
      t += QString( us_tr( ", drift across it %1% +/- %2%" ) )
         .arg( us_frames_num( 100e0 * r.drift / r.mean ) )
         .arg( us_frames_num( 100e0 * r.drift_sd / fabs( r.mean ) ) );
      if ( r.drift_sd > 0e0 )
      {
         t += QString( us_tr( " (%1 sd)" ) ).arg( QString::number( fabs( r.drift ) / r.drift_sd, 'f', 1 ) );
      }
      if ( have_sd )
      {
         t += QString( us_tr( ", reduced chi^2 of I(t) about its line %1" ) ).arg( us_frames_num( r.it_chi2r ) );
      }
   }
   t += ".";
   if ( r.have_chi2 )
   {
      double lo = *min_element( r.frame_chi2r.begin(), r.frame_chi2r.end() );
      double hi = *max_element( r.frame_chi2r.begin(), r.frame_chi2r.end() );
      t += QString( us_tr( "\n   Each frame against the mean of the region's other frames, over its whole curve (%1 points): "
                           "reduced chi^2 %2 to %3 (median %4), 0.1% limit %5: " ) )
         .arg( r.n_q )
         .arg( us_frames_num( lo ) )
         .arg( us_frames_num( hi ) )
         .arg( us_frames_num( us_frames_median( r.frame_chi2r ) ) )
         .arg( us_frames_num( r.chi2_limit ) );
      if ( !r.outliers.size() )
      {
         t += us_tr( "all frames agree within their sd." );
      } else {
         QStringList out;
         for ( int k : r.outliers )
         {
            out << QString( "%1 (%2)" ).arg( frame_text[ k ] ).arg( us_frames_num( r.frame_chi2r[ k - r.i0 ] ) );
         }
         t += QString( us_tr( "%1 frame%2 above it: %3." ) )
            .arg( r.outliers.size() )
            .arg( r.outliers.size() == 1 ? "" : "s" )
            .arg( out.join( ", " ) );
      }
   }
   return t;
}

void US_Hydrodyn_Saxs_Buffer_Frames::regions_changed()
{
   if ( suppress )
   {
      return;
   }
   le_r2_from->setEnabled( cb_r2->isChecked() );
   le_r2_to  ->setEnabled( cb_r2->isChecked() );

   int a0, a1, b0, b1;
   r1 = region_stats();
   r2 = region_stats();
   if ( region_range( le_r1_from, le_r1_to, a0, a1 ) )
   {
      compute_region( a0, a1, r1 );
   }
   if ( cb_r2->isChecked() && region_range( le_r2_from, le_r2_to, b0, b1 ) )
   {
      compute_region( b0, b1, r2 );
   }
   compute_joint();
   set_region_boxes( r1, le_r1_avg, le_r1_sd, le_r1_chi2 );
   set_region_boxes( r2, le_r2_avg, le_r2_sd, le_r2_chi2 );
   le_r2_avg ->setEnabled( cb_r2->isChecked() );
   le_r2_sd  ->setEnabled( cb_r2->isChecked() );
   le_r2_chi2->setEnabled( cb_r2->isChecked() );

   QStringList lines;
   if ( !It_ok )
   {
      lines << us_tr( "Some frames have no points in the q range of the I(t)." );
   }
   if ( !note.isEmpty() )
   {
      lines << note;
   }
   if ( !have_sd )
   {
      lines << us_tr( "Not every frame has sd: the frames are not compared with each other." );
   } else if ( !same_grid ) {
      lines << us_tr( "The frames are not on one q grid: they are not compared with each other." );
   }
   lines << region_text( us_tr( "Before the elution" ), r1 );
   if ( cb_r2->isChecked() )
   {
      lines << region_text( us_tr( "After the elution" ), r2 );
      if ( r1.ok && r2.ok )
      {
         bool overlap = !( r2.i0 > r1.i1 || r1.i0 > r2.i1 );
         QString t = QString( us_tr( "The two regions: I(t) after / before %1" ) ).arg( us_frames_num( r2.mean / r1.mean ) );
         if ( r1.have_chi2 && r2.have_chi2 && !overlap )
         {
            double c = 0e0;
            for ( int i = 0; i < r1.n_q; i++ )
            {
               double d = r1.avg[ i ] - r2.avg[ i ];
               c += d * d / ( r1.avg_var[ i ] + r2.avg_var[ i ] );
            }
            c /= r1.n_q;
            t += QString( us_tr( "; their average curves differ by a reduced chi^2 of %1 over %2 points (0.1% limit %3): %4" ) )
               .arg( us_frames_num( c ) )
               .arg( r1.n_q )
               .arg( us_frames_num( r1.chi2_limit ) )
               .arg( c > r1.chi2_limit ? us_tr( "they differ" ) : us_tr( "they agree within their sd" ) );
         }
         if ( overlap )
         {
            t += us_tr( "; the regions overlap" );
         }
         lines << t + ".";
      }
      if ( joint.ok )
      {
         QString t = QString( us_tr( "Line fitted to the I(t) of both regions (red, dashed): from frame %1 to frame %2 it changes by %3% +/- %4%" ) )
            .arg( frame_text[ joint.k0 ] )
            .arg( frame_text[ joint.k1 ] )
            .arg( us_frames_num( 100e0 * joint.change / joint.a ) )
            .arg( us_frames_num( 100e0 * joint.change_sd / fabs( joint.a ) ) );
         if ( joint.change_sd > 0e0 )
         {
            t += QString( us_tr( " (%1 sd)" ) ).arg( QString::number( fabs( joint.change ) / joint.change_sd, 'f', 1 ) );
         }
         if ( have_sd )
         {
            t += QString( us_tr( ", reduced chi^2 of I(t) about it %1 over %2 frames (each region about its own line: %3, %4)" ) )
               .arg( us_frames_num( joint.chi2r ) )
               .arg( joint.n )
               .arg( r1.n >= 3 ? us_frames_num( r1.it_chi2r ) : QString( "-" ) )
               .arg( r2.n >= 3 ? us_frames_num( r2.it_chi2r ) : QString( "-" ) );
         }
         lines << t + ".";
      }
   }
   lbl_stats->setText( lines.join( "\n" ) );

   pb_select_r1  ->setEnabled( r1.ok );
   pb_select_both->setEnabled( r1.ok && r2.ok );

   replot_It( false );
   replot_chi2();
}

void US_Hydrodyn_Saxs_Buffer_Frames::q_changed()
{
   if ( suppress )
   {
      return;
   }
   compute_It();
   regions_changed();
   replot_It( true );
}

void US_Hydrodyn_Saxs_Buffer_Frames::replot_It( bool rescale )
{
   plot->detachItems( QwtPlotItem::Rtti_PlotCurve );
   plot->detachItems( QwtPlotItem::Rtti_PlotMarker );

   int n = (int) files.size();
   if ( rescale )
   {
      if ( plot_zoomer )
      {
         delete plot_zoomer;
         plot_zoomer = (ScrollZoomer *) 0;
      }
      if ( log_y )
      {
         plot->setAxisScaleEngine( QwtPlot::yLeft, new QwtLogScaleEngine( 10 ) );
      } else {
         plot->setAxisScaleEngine( QwtPlot::yLeft, new QwtLinearScaleEngine );
      }
      double y_lo = 1e300;
      double y_hi = -1e300;
      for ( int k = 0; k < n; k++ )
      {
         if ( !log_y || It[ k ] > 0e0 )
         {
            y_lo = min( y_lo, It[ k ] );
            y_hi = max( y_hi, It[ k ] );
         }
      }
      if ( y_lo > y_hi )
      {
         y_lo = log_y ? 1e-3 : 0e0;
         y_hi = 1e0;
      }
      if ( log_y )
      {
         y_lo /= 1.2e0;
         y_hi *= 1.2e0;
      } else {
         double pad = 0.05e0 * ( y_hi > y_lo ? y_hi - y_lo : fabs( y_hi ) + 1e-30 );
         y_lo -= pad;
         y_hi += pad;
      }
      plot->setAxisScale( QwtPlot::xBottom, frame[ 0 ] - 0.5e0, frame[ n - 1 ] + 0.5e0 );
      plot->setAxisScale( QwtPlot::yLeft, y_lo, y_hi );
   }

   if ( It_ok )
   {
      auto add_points = [ & ]( const QString & title, const vector < int > & idx, QColor color, int size, bool line ) {
         vector < double > x;
         vector < double > y;
         for ( int k : idx )
         {
            if ( !log_y || It[ k ] > 0e0 )
            {
               x.push_back( frame[ k ] );
               y.push_back( It[ k ] );
            }
         }
         if ( !x.size() )
         {
            return;
         }
         QwtPlotCurve * c = new QwtPlotCurve( title );
         c->setStyle( line ? QwtPlotCurve::Lines : QwtPlotCurve::NoCurve );
         c->setPen( QPen( color, 1 ) );
         c->setSymbol( new QwtSymbol( QwtSymbol::Ellipse, QBrush( color ), QPen( color ), QSize( size, size ) ) );
         c->setSamples( &x[ 0 ], &y[ 0 ], (int) x.size() );
         c->attach( plot );
      };
      auto add_line = [ & ]( const region_stats & r, QColor color ) {
         if ( !r.ok || r.n < 2 )
         {
            return;
         }
         double x[ 2 ] = { frame[ r.i0 ], frame[ r.i1 ] };
         double y[ 2 ] = { r.line_a, r.line_a + r.line_b * ( frame[ r.i1 ] - frame[ r.i0 ] ) };
         if ( log_y && ( y[ 0 ] <= 0e0 || y[ 1 ] <= 0e0 ) )
         {
            return;
         }
         QwtPlotCurve * c = new QwtPlotCurve( us_tr( "drift" ) );
         c->setStyle( QwtPlotCurve::Lines );
         c->setPen( QPen( color, 2 ) );
         c->setSamples( x, y, 2 );
         c->attach( plot );
      };
      auto add_marker = [ & ]( int k, QColor color, const QString & label ) {
         QwtPlotMarker * mk = new QwtPlotMarker;
         mk->setLineStyle( QwtPlotMarker::VLine );
         mk->setLinePen( QPen( color, 2, Qt::DashLine ) );
         mk->setXValue( frame[ k ] );
         QwtText qwtt( label );
         qwtt.setColor( color );
         mk->setLabel( qwtt );
         mk->setLabelAlignment( Qt::AlignRight | Qt::AlignTop );
         mk->attach( plot );
      };

      vector < int > all( n );
      for ( int k = 0; k < n; k++ )
      {
         all[ k ] = k;
      }
      add_points( us_tr( "I(t)" ), all, Qt::lightGray, 3, true );
      const region_stats * regions[] = { &r1, &r2 };
      QColor colors[] = { Qt::green, Qt::cyan };
      for ( int j = 0; j < 2; j++ )
      {
         const region_stats & r = *regions[ j ];
         if ( !r.ok )
         {
            continue;
         }
         vector < int > idx;
         for ( int k = r.i0; k <= r.i1; k++ )
         {
            idx.push_back( k );
         }
         add_points( j ? us_tr( "after the elution" ) : us_tr( "before the elution" ), idx, colors[ j ], 5, false );
         add_line( r, colors[ j ] );
         if ( r.outliers.size() )
         {
            add_points( us_tr( "not agreeing" ), r.outliers, Qt::red, 7, false );
         }
         add_marker( r.i0, colors[ j ], j ? "2" : "1" );
         add_marker( r.i1, colors[ j ], j ? "2" : "1" );
      }
      if ( joint.ok )
      {
         double x[ 2 ] = { frame[ joint.k0 ], frame[ joint.k1 ] };
         double y[ 2 ] = { joint.a, joint.a + joint.change };
         if ( !log_y || ( y[ 0 ] > 0e0 && y[ 1 ] > 0e0 ) )
         {
            QwtPlotCurve * c = new QwtPlotCurve( us_tr( "fit to both regions" ) );
            c->setStyle( QwtPlotCurve::Lines );
            c->setPen( QPen( Qt::red, 2, Qt::DashLine ) );
            c->setSamples( x, y, 2 );
            c->attach( plot );
         }
      }
   }

   if ( rescale )
   {
      plot_zoomer = new ScrollZoomer( plot->canvas() );
      plot_zoomer->setRubberBandPen( QPen( Qt::yellow, 0, Qt::DotLine ) );
      plot_zoomer->setTrackerPen( QPen( Qt::red ) );
   }
   plot->replot();
}

void US_Hydrodyn_Saxs_Buffer_Frames::replot_chi2()
{
   plot_chi2->detachItems( QwtPlotItem::Rtti_PlotCurve );
   plot_chi2->detachItems( QwtPlotItem::Rtti_PlotMarker );
   int n = (int) files.size();
   plot_chi2->setAxisScale( QwtPlot::xBottom, frame[ 0 ] - 0.5e0, frame[ n - 1 ] + 0.5e0 );

   double y_hi  = 0e0;
   double limit = 0e0;
   const region_stats * regions[] = { &r1, &r2 };
   QColor colors[] = { Qt::green, Qt::cyan };
   for ( int j = 0; j < 2; j++ )
   {
      const region_stats & r = *regions[ j ];
      if ( !r.ok || !r.have_chi2 )
      {
         continue;
      }
      vector < double > x;
      vector < double > y;
      for ( int k = r.i0; k <= r.i1; k++ )
      {
         x.push_back( frame[ k ] );
         y.push_back( r.frame_chi2r[ k - r.i0 ] );
         y_hi = max( y_hi, y.back() );
      }
      limit = r.chi2_limit;
      QwtPlotCurve * c = new QwtPlotCurve( j ? us_tr( "after the elution" ) : us_tr( "before the elution" ) );
      c->setStyle( QwtPlotCurve::NoCurve );
      c->setSymbol( new QwtSymbol( QwtSymbol::Ellipse, QBrush( colors[ j ] ), QPen( colors[ j ] ), QSize( 5, 5 ) ) );
      c->setSamples( &x[ 0 ], &y[ 0 ], (int) x.size() );
      c->attach( plot_chi2 );
   }
   if ( limit > 0e0 )
   {
      QwtPlotMarker * one = new QwtPlotMarker;
      one->setLineStyle( QwtPlotMarker::HLine );
      one->setLinePen( QPen( Qt::lightGray, 1, Qt::DotLine ) );
      one->setYValue( 1e0 );
      one->attach( plot_chi2 );
      QwtPlotMarker * lim = new QwtPlotMarker;
      lim->setLineStyle( QwtPlotMarker::HLine );
      lim->setLinePen( QPen( Qt::red, 1, Qt::DashLine ) );
      lim->setYValue( limit );
      QwtText qwtt( us_tr( "0.1% limit" ) );
      qwtt.setColor( Qt::red );
      lim->setLabel( qwtt );
      lim->setLabelAlignment( Qt::AlignLeft | Qt::AlignTop );
      lim->attach( plot_chi2 );
      plot_chi2->setAxisScale( QwtPlot::yLeft, 0e0, 1.1e0 * max( y_hi, limit ) );
   } else {
      plot_chi2->setAxisScale( QwtPlot::yLeft, 0e0, 2e0 );
   }
   plot_chi2->replot();
}

// the baseline level and noise of I(t) over frames lo..hi: the median, and 1.4826 x the median absolute deviation
// (at least the frames' own I(t) sd)
static void us_frames_level( const vector < double > & It, const vector < double > & It_sd, bool have_sd,
                             int lo, int hi, double & level, double & noise )
{
   vector < double > v( It.begin() + lo, It.begin() + hi + 1 );
   level = us_frames_median( v );
   vector < double > dev;
   for ( double x : v )
   {
      dev.push_back( fabs( x - level ) );
   }
   noise = 1.4826e0 * us_frames_median( dev );
   if ( have_sd )
   {
      vector < double > s( It_sd.begin() + lo, It_sd.begin() + hi + 1 );
      noise = max( noise, us_frames_median( s ) );
   }
   if ( !( noise > 0e0 ) )
   {
      noise = 1e-6 * fabs( level ) + 1e-300;
   }
}

bool US_Hydrodyn_Saxs_Buffer_Frames::suggest_regions( int & a0, int & a1, int & b0, int & b1, QString & msg )
{
   int n = (int) It.size();
   a0 = a1 = b0 = b1 = -1;
   if ( !It_ok || n < 6 )
   {
      msg = us_tr( "too few frames" );
      return false;
   }
   double level;
   double noise;
   us_frames_level( It, It_sd, have_sd, 0, max( 3, n / 5 ) - 1, level, noise );
   int e = -1;
   for ( int pass = 0; pass < 2; pass++ )
   {
      // the elution starts where I(t) is more than 3 sd above the baseline for 3 frames in a row
      e = -1;
      for ( int i = 0; i + 2 < n; i++ )
      {
         if ( It[ i ] > level + 3e0 * noise && It[ i + 1 ] > level + 3e0 * noise && It[ i + 2 ] > level + 3e0 * noise )
         {
            e = i;
            break;
         }
      }
      if ( e < 0 )
      {
         msg = us_tr( "no elution found: I(t) does not rise above its baseline" );
         return false;
      }
      // the region ends 3 frames before and goes back while I(t) stays within 4 sd of the baseline
      a1 = e - 4;
      while ( a1 >= 0 && fabs( It[ a1 ] - level ) > 4e0 * noise )
      {
         a1--;
      }
      a0 = a1;
      while ( a0 > 0 && fabs( It[ a0 - 1 ] - level ) <= 4e0 * noise )
      {
         a0--;
      }
      if ( a1 < 0 || a1 - a0 + 1 < 2 )
      {
         msg = us_tr( "no stable frames before the elution" );
         a0 = a1 = -1;
         return false;
      }
      us_frames_level( It, It_sd, have_sd, a0, a1, level, noise );
   }

   // after the largest peak, the frames from which I(t) is back within 4 sd of the baseline for good
   int p = e;
   for ( int i = e; i < n; i++ )
   {
      if ( It[ i ] > It[ p ] )
      {
         p = i;
      }
   }
   int back = -1;
   for ( int i = n - 1; i > p; i-- )
   {
      if ( fabs( It[ i ] - level ) <= 4e0 * noise )
      {
         back = i;
      } else {
         break;
      }
   }
   if ( back >= 0 && n - 1 - ( back + 2 ) + 1 >= 3 )
   {
      b0 = back + 2;
      b1 = n - 1;
      msg = us_tr( "Suggested: the buffer frames before the elution and after it, where I(t) is back at the baseline." );
   } else {
      msg = us_tr( "Suggested: the buffer frames before the elution; I(t) does not return to its baseline after it." );
   }
   return true;
}

void US_Hydrodyn_Saxs_Buffer_Frames::suggest()
{
   int a0, a1, b0, b1;
   QString msg;
   if ( suggest_regions( a0, a1, b0, b1, msg ) )
   {
      set_region_text( le_r1_from, le_r1_to, a0, a1 );
      if ( b0 >= 0 )
      {
         cb_r2->setChecked( true );
         set_region_text( le_r2_from, le_r2_to, b0, b1 );
      } else {
         cb_r2->setChecked( false );
      }
      note = msg;
   } else {
      note = QString( us_tr( "No suggestion: %1." ) ).arg( msg );
   }
   regions_changed();
}

void US_Hydrodyn_Saxs_Buffer_Frames::log_y_toggle()
{
   log_y = !log_y;
   pb_log_y->setText( log_y ? us_tr( "Lin Y" ) : us_tr( "Log Y" ) );
   replot_It( true );
}

void US_Hydrodyn_Saxs_Buffer_Frames::wheel_target( mQLineEdit * le )
{
   le_wheel = le;
   int k = frame_index( le->text() );
   qwtw_wheel->blockSignals( true );
   qwtw_wheel->setRange( 0, max( 1, (int) files.size() - 1 ) );
   qwtw_wheel->setSingleStep( 1 );
   qwtw_wheel->setValue( k < 0 ? 0 : k );
   qwtw_wheel->blockSignals( false );
   qwtw_wheel  ->setEnabled( true );
   pb_wheel_dec->setEnabled( true );
   pb_wheel_inc->setEnabled( true );
   QString which =
      le == le_r1_from ? us_tr( "the first frame before the elution" ) :
      le == le_r1_to   ? us_tr( "the last frame before the elution" ) :
      le == le_r2_from ? us_tr( "the first frame after the elution" ) :
      us_tr( "the last frame after the elution" );
   lbl_wheel->setText( QString( us_tr( "The wheel moves %1" ) ).arg( which ) );
}

void US_Hydrodyn_Saxs_Buffer_Frames::adjust_wheel( double v )
{
   if ( !le_wheel )
   {
      return;
   }
   int k = (int) lround( v );
   k = max( 0, min( (int) files.size() - 1, k ) );
   le_wheel->setText( frame_text[ k ] );
}

void US_Hydrodyn_Saxs_Buffer_Frames::wheel_dec()
{
   qwtw_wheel->setValue( qwtw_wheel->value() - 1e0 );
}

void US_Hydrodyn_Saxs_Buffer_Frames::wheel_inc()
{
   qwtw_wheel->setValue( qwtw_wheel->value() + 1e0 );
}

// select the chosen frames, and nothing else, in the Data Utility's file list
void US_Hydrodyn_Saxs_Buffer_Frames::select_frames( bool both )
{
   set < QString > chosen;
   const region_stats * regions[] = { &r1, &r2 };
   for ( int j = 0; j < ( both ? 2 : 1 ); j++ )
   {
      const region_stats & r = *regions[ j ];
      if ( !r.ok )
      {
         continue;
      }
      for ( int k = r.i0; k <= r.i1; k++ )
      {
         chosen.insert( files[ k ] );
      }
   }
   buffer->disable_updates = true;
   buffer->lb_files->clearSelection();
   for ( int i = 0; i < buffer->lb_files->count(); i++ )
   {
      if ( chosen.count( buffer->lb_files->item( i )->text() ) )
      {
         buffer->lb_files->item( i )->setSelected( true );
      }
   }
   buffer->disable_updates = false;
   buffer->update_files();
   buffer->editor_msg( "black", QString( us_tr( "Buffer frames: %1 frames selected (%2)" ) )
                       .arg( chosen.size() )
                       .arg( both ? us_tr( "before and after the elution" ) : us_tr( "before the elution" ) ) );
}

void US_Hydrodyn_Saxs_Buffer_Frames::select_r1()
{
   select_frames( false );
}

void US_Hydrodyn_Saxs_Buffer_Frames::select_both()
{
   select_frames( true );
}

void US_Hydrodyn_Saxs_Buffer_Frames::help()
{
   US_Help * online_help = new US_Help( this );
   online_help->show_URL( "file://" +
                          QDir::toNativeSeparators( USglobal->config_list.help_dir + "/manual/somo/somo_SAS_data_utility.html" ) +
                          "#Frames" );
}

void US_Hydrodyn_Saxs_Buffer_Frames::closeEvent( QCloseEvent * e )
{
   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}
