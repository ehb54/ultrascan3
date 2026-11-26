#include "../include/us3_defines.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_hydrodyn_saxs_buffer.h"
#include "../include/us_hydrodyn_saxs_buffer_upturn.h"
#include "../include/us_lm.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QCloseEvent>
#include <qwt_symbol.h>
#include <qwt_scale_engine.h>
#include <qwt_legend.h>
#include <algorithm>
#include <cmath>

// the norm lmmin uses (defined in us_lm.cpp; set by lmcurve_fit, which is not used here)
namespace LM
{
   extern thread_local double ( *lm_use_norm )( int n, const double * x );
}

// The model, with parameters transformed so that the fit is unconstrained:
//   p[ 0 ] = ln G   solute I(0)             p[ 3 ] = ln Ra  large-scatterer Rg
//   p[ 1 ] = ln Rg  solute Rg               p[ 4 ] = ln Ba  power-law amplitude
//   p[ 2 ] = ln Ga  large-scatterer I(0)    p[ 5 ] = t, power-law slope Pa = 1 + 3.5 / ( 1 + exp( -t ) )
// I( q ) = G exp( -q^2 Rg^2 / 3 )                                       (solute)
//        + Ga exp( -q^2 Ra^2 / 3 ) + Ba exp( -q^2 Rg^2 / 3 ) ( erf( q Ra / sqrt 6 )^3 / q )^Pa   (large scatterer)
// The large-scatterer term is a Beaucage unified level whose power law is cut off at the solute's size.

#define UPT_NPAR 6

struct us_upturn_data
{
   const double * q;
   const double * I;
   const double * s;
};

static void us_upturn_par( const double * p, double & G, double & Rg, double & Ga, double & Ra, double & Ba, double & Pa )
{
   G  = exp( p[ 0 ] );
   Rg = exp( p[ 1 ] );
   Ga = exp( p[ 2 ] );
   Ra = exp( p[ 3 ] );
   Ba = exp( p[ 4 ] );
   Pa = 1e0 + 3.5e0 / ( 1e0 + exp( -p[ 5 ] ) );
}

static double us_upturn_solute( double q, const double * p )
{
   double G, Rg, Ga, Ra, Ba, Pa;
   us_upturn_par( p, G, Rg, Ga, Ra, Ba, Pa );
   return G * exp( -q * q * Rg * Rg / 3e0 );
}

static double us_upturn_large( double q, const double * p )
{
   double G, Rg, Ga, Ra, Ba, Pa;
   us_upturn_par( p, G, Rg, Ga, Ra, Ba, Pa );
   if ( q <= 0e0 )
   {
      return Ga;
   }
   double x = erf( q * Ra / sqrt( 6e0 ) );
   x = x * x * x / q;
   return Ga * exp( -q * q * Ra * Ra / 3e0 ) + Ba * exp( -q * q * Rg * Rg / 3e0 ) * pow( x, Pa );
}

static void us_upturn_eval( const double * p, int m, const void * data, double * fvec, int * )
{
   const us_upturn_data * d = (const us_upturn_data *) data;
   for ( int i = 0; i < m; i++ )
   {
      double v = us_upturn_solute( d->q[ i ], p ) + us_upturn_large( d->q[ i ], p );
      double r = ( v - d->I[ i ] ) / d->s[ i ];
      fvec[ i ] = std::isfinite( r ) ? r : 1e10;
   }
}

static double us_upturn_chi2( const double * p, const us_upturn_data & d, int m )
{
   vector < double > f( m );
   int info = 0;
   us_upturn_eval( p, m, (const void *) &d, &f[ 0 ], &info );
   double c = 0e0;
   for ( int i = 0; i < m; i++ )
   {
      c += f[ i ] * f[ i ];
   }
   return c;
}

// inverse of a small symmetric matrix by Gauss-Jordan elimination with partial pivoting; false if singular
static bool us_upturn_invert( vector < double > a, int n, vector < double > & inv )
{
   inv.assign( n * n, 0e0 );
   for ( int i = 0; i < n; i++ )
   {
      inv[ i * n + i ] = 1e0;
   }
   for ( int c = 0; c < n; c++ )
   {
      int    piv  = c;
      double best = fabs( a[ c * n + c ] );
      for ( int r = c + 1; r < n; r++ )
      {
         if ( fabs( a[ r * n + c ] ) > best )
         {
            best = fabs( a[ r * n + c ] );
            piv  = r;
         }
      }
      if ( best < 1e-300 )
      {
         return false;
      }
      if ( piv != c )
      {
         for ( int k = 0; k < n; k++ )
         {
            swap( a  [ c * n + k ], a  [ piv * n + k ] );
            swap( inv[ c * n + k ], inv[ piv * n + k ] );
         }
      }
      double d = a[ c * n + c ];
      for ( int k = 0; k < n; k++ )
      {
         a  [ c * n + k ] /= d;
         inv[ c * n + k ] /= d;
      }
      for ( int r = 0; r < n; r++ )
      {
         if ( r != c && a[ r * n + c ] != 0e0 )
         {
            double f = a[ r * n + c ];
            for ( int k = 0; k < n; k++ )
            {
               a  [ r * n + k ] -= f * a  [ c * n + k ];
               inv[ r * n + k ] -= f * inv[ c * n + k ];
            }
         }
      }
   }
   return true;
}

US_Hydrodyn_Saxs_Buffer_Upturn::US_Hydrodyn_Saxs_Buffer_Upturn(
                                                                US_Hydrodyn_Saxs_Buffer * buffer,
                                                                QStringList               files,
                                                                QWidget *                 p
                                                                ) : QFrame( p, Qt::Window )
{
   this->buffer = buffer;
   this->files  = files;
   setAttribute( Qt::WA_DeleteOnClose );
   USglobal = new US_Config();
   setPalette( PALET_FRAME );
   setWindowTitle( us_tr( "US-SOMO: SAXS Data Utility : Upturn correction" ) );

   setupGUI();

   global_Xpos += 30;
   global_Ypos += 30;
   setGeometry( global_Xpos, global_Ypos, 1050, 760 );

   fit();
}

US_Hydrodyn_Saxs_Buffer_Upturn::~US_Hydrodyn_Saxs_Buffer_Upturn()
{
}

QTableWidgetItem * US_Hydrodyn_Saxs_Buffer_Upturn::cell( const QString & text, bool numeric )
{
   QTableWidgetItem * it = new QTableWidgetItem( text );
   it->setFlags( Qt::ItemIsEnabled );
   it->setTextAlignment( numeric ? ( Qt::AlignRight | Qt::AlignVCenter ) : ( Qt::AlignLeft | Qt::AlignVCenter ) );
   return it;
}

void US_Hydrodyn_Saxs_Buffer_Upturn::setupGUI()
{
   int minHeight1 = 24;

   lbl_info = new QLabel( us_tr( "Each curve's low-q region is fitted with a Guinier term for the solute plus a large-scatterer term "
                                 "(Guinier knee and power-law tail), up to the end of the solute's Guinier range. The corrected copy "
                                 "has the large-scatterer term subtracted, up to where it falls below a fraction of the data's errors "
                                 "(the splice); above that the curve is unchanged.\n"
                                 "Where the term is large next to the solute's signal the subtraction depends on the model (orange in the "
                                 "plot): Make corrected copies asks whether to cut those points or keep them, noted in the header. "
                                 "A curve without an upturn is left as it is." ), this );
   lbl_info->setWordWrap( true );
   lbl_info->setPalette( PALET_LABEL );
   AUTFBACK( lbl_info );
   lbl_info->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );

   QLabel * lbl_q_from = new QLabel( us_tr( " Fit from q" ), this );
   QLabel * lbl_qrg    = new QLabel( us_tr( "  up to q Rg =" ), this );
   QLabel * lbl_end    = new QLabel( us_tr( "  Subtract until the term is below" ), this );
   QLabel * lbl_end2   = new QLabel( us_tr( "x SD" ), this );
   QLabel * lbl_rel    = new QLabel( us_tr( "  Reliable while the term is at most" ), this );
   QLabel * lbl_rel2   = new QLabel( us_tr( "x the solute's" ), this );
   QLabel * labels[]   = { lbl_q_from, lbl_qrg, lbl_end, lbl_end2, lbl_rel, lbl_rel2 };
   for ( int k = 0; k < 6; k++ )
   {
      labels[ k ]->setPalette( PALET_NORMAL );
      AUTFBACK( labels[ k ] );
      labels[ k ]->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   }

   le_q_from = new QLineEdit( "", this );
   le_qrg    = new QLineEdit( "1.3", this );
   le_end_sd = new QLineEdit( "0.5", this );
   le_rel    = new QLineEdit( "0.25", this );
   le_q_from->setToolTip( us_tr( "First q of the fit [1/Angstrom]; empty: the curve's first point" ) );
   le_qrg   ->setToolTip( us_tr( "The fit ends at q = this / (the solute's fitted Rg): the end of its Guinier range" ) );
   le_end_sd->setToolTip( us_tr( "The term is subtracted below the q from which it stays under this many of the data's standard deviations\n"
                                 "(but at least up to the end of the fit); above that the curve is unchanged" ) );
   le_rel   ->setToolTip( us_tr( "Below the q where the large-scatterer term exceeds this fraction of the solute's fitted intensity,\n"
                                 "the corrected points depend too much on the model to be trusted" ) );
   QLineEdit * edits[] = { le_q_from, le_qrg, le_end_sd, le_rel };
   for ( int k = 0; k < 4; k++ )
   {
      edits[ k ]->setPalette( PALET_NORMAL );
      AUTFBACK( edits[ k ] );
      edits[ k ]->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
      edits[ k ]->setMinimumHeight( minHeight1 );
      edits[ k ]->setMaximumWidth( 70 );
      connect( edits[ k ], SIGNAL( textChanged( const QString & ) ), SLOT( inputs_changed() ) );
   }

   pb_fit = new QPushButton( us_tr( "Fit" ), this );
   pb_fit->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   pb_fit->setMinimumHeight( minHeight1 );
   pb_fit->setPalette( PALET_PUSHB );
   connect( pb_fit, SIGNAL( clicked() ), SLOT( fit() ) );

   QStringList hdr;
   hdr << us_tr( "Curve" ) << us_tr( "Fit q range" ) << us_tr( "Solute\nI(0)" ) << us_tr( "Solute\nRg [A]" )
       << us_tr( "Large scatterer\nRg [A]" ) << us_tr( "Power-law\nslope" ) << us_tr( "Reduced\nchi^2" )
       << us_tr( "Reliable\nfrom q" ) << us_tr( "Subtracted\nbelow q" ) << us_tr( "Result" );
   t_results = new QTableWidget( (int) files.size(), hdr.size(), this );
   t_results->setHorizontalHeaderLabels( hdr );
   t_results->verticalHeader()->hide();
   t_results->setPalette( PALET_EDIT );
   AUTFBACK( t_results );
   t_results->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   t_results->setEditTriggers( QAbstractItemView::NoEditTriggers );
   t_results->setSelectionBehavior( QAbstractItemView::SelectRows );
   t_results->setSelectionMode( QAbstractItemView::SingleSelection );
   for ( int i = 0; i < (int) files.size(); i++ )
   {
      t_results->setItem( i, 0, cell( files[ i ], false ) );
   }
   t_results->horizontalHeader()->setSectionResizeMode( QHeaderView::ResizeToContents );
   t_results->horizontalHeader()->setStretchLastSection( true );
   t_results->setMaximumHeight( 60 + 26 * min( (int) files.size(), 6 ) );

   cmb_curve = new QComboBox( this );
   cmb_curve->setPalette( PALET_NORMAL );
   AUTFBACK( cmb_curve );
   cmb_curve->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cmb_curve->addItems( files );
   cmb_curve->setToolTip( us_tr( "The curve shown in the plot" ) );
   connect( cmb_curve, SIGNAL( currentIndexChanged( int ) ), SLOT( show_curve( int ) ) );
   connect( t_results, SIGNAL( cellClicked( int, int ) ), cmb_curve, SLOT( setCurrentIndex( int ) ) );

   usp_plot = new US_Plot( plot, "", "", "", this );
   connect( (QWidget *) plot->titleLabel(), SIGNAL( customContextMenuRequested( const QPoint & ) ), SLOT( usp_config_plot( const QPoint & ) ) );
   ( (QWidget *) plot->titleLabel() )->setContextMenuPolicy( Qt::CustomContextMenu );
   plot->setCanvasBackground( USglobal->global_colors.plot );
   plot->setAxisTitle( QwtPlot::xBottom, us_tr( "q [1/Angstrom]" ) );
   plot->setAxisTitle( QwtPlot::yLeft, us_tr( "I(q)" ) );
   plot->setAxisScaleEngine( QwtPlot::xBottom, new QwtLogScaleEngine( 10 ) );
   plot->setAxisScaleEngine( QwtPlot::yLeft, new QwtLogScaleEngine( 10 ) );
   plot->setAxisFont( QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   plot->setAxisFont( QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   plot->insertLegend( new QwtLegend(), QwtPlot::BottomLegend );
   grid = new QwtPlotGrid;
   grid->enableXMin( true );
   grid->enableYMin( true );
   grid->setMajorPen( QPen( USglobal->global_colors.major_ticks, 0, Qt::DotLine ) );
   grid->setMinorPen( QPen( USglobal->global_colors.minor_ticks, 0, Qt::DotLine ) );
   grid->attach( plot );
   plot->setMinimumHeight( 300 );

   lbl_status = new QLabel( "", this );
   lbl_status->setWordWrap( true );
   lbl_status->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_status );
   lbl_status->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );

   pb_help  = new QPushButton( us_tr( "Help" ), this );
   pb_make  = new QPushButton( us_tr( "Make corrected copies" ), this );
   pb_close = new QPushButton( us_tr( "Close" ), this );
   QPushButton * buttons[] = { pb_help, pb_make, pb_close };
   for ( int k = 0; k < 3; k++ )
   {
      buttons[ k ]->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
      buttons[ k ]->setMinimumHeight( minHeight1 );
      buttons[ k ]->setPalette( PALET_PUSHB );
   }
   pb_make->setToolTip( us_tr( "Add a corrected copy (<name>_upc) of each curve with an upturn to the produced data" ) );
   connect( pb_help,  SIGNAL( clicked() ), SLOT( help() ) );
   connect( pb_make,  SIGNAL( clicked() ), SLOT( make() ) );
   connect( pb_close, SIGNAL( clicked() ), SLOT( close() ) );

   QHBoxLayout * hbl_opts = new QHBoxLayout; hbl_opts->setContentsMargins( 0, 0, 0, 0 ); hbl_opts->setSpacing( 2 );
   hbl_opts->addWidget( lbl_q_from );
   hbl_opts->addWidget( le_q_from );
   hbl_opts->addWidget( lbl_qrg );
   hbl_opts->addWidget( le_qrg );
   hbl_opts->addWidget( lbl_end );
   hbl_opts->addWidget( le_end_sd );
   hbl_opts->addWidget( lbl_end2 );
   hbl_opts->addStretch( 1 );
   hbl_opts->addWidget( pb_fit );

   QHBoxLayout * hbl_opts2 = new QHBoxLayout; hbl_opts2->setContentsMargins( 0, 0, 0, 0 ); hbl_opts2->setSpacing( 2 );
   hbl_opts2->addWidget( lbl_rel );
   hbl_opts2->addWidget( le_rel );
   hbl_opts2->addWidget( lbl_rel2 );
   hbl_opts2->addStretch( 1 );
   hbl_opts2->addWidget( cmb_curve, 1 );

   QHBoxLayout * hbl_bottom = new QHBoxLayout; hbl_bottom->setContentsMargins( 0, 0, 0, 0 ); hbl_bottom->setSpacing( 0 );
   hbl_bottom->addWidget( pb_help );
   hbl_bottom->addWidget( pb_make, 1 );
   hbl_bottom->addWidget( pb_close );

   QVBoxLayout * background = new QVBoxLayout( this ); background->setContentsMargins( 0, 0, 0, 0 ); background->setSpacing( 1 );
   background->addWidget( lbl_info );
   background->addLayout( hbl_opts );
   background->addLayout( hbl_opts2 );
   background->addWidget( t_results );
   background->addWidget( plot, 1 );
   background->addWidget( lbl_status );
   background->addLayout( hbl_bottom );
}

void US_Hydrodyn_Saxs_Buffer_Upturn::usp_config_plot( const QPoint & )
{
   US_PlotChoices * uspc = new US_PlotChoices( usp_plot );
   uspc->exec();
   delete uspc;
}

void US_Hydrodyn_Saxs_Buffer_Upturn::inputs_changed()
{
   lbl_status->setText( us_tr( "Settings changed: press Fit to update." ) );
   pb_make->setEnabled( false );
}

// fit one curve; the fit range starts at "Fit from q" and ends at q = ( q Rg ) / Rg, iterated as Rg changes
bool US_Hydrodyn_Saxs_Buffer_Upturn::fit_one( const QString & file, fit_result & r )
{
   r = fit_result();
   const vector < double > & q = buffer->f_qs[ file ];
   const vector < double > & I = buffer->f_Is[ file ];
   int n = (int) q.size();
   bool have_err = buffer->f_errors.count( file ) && (int) buffer->f_errors[ file ].size() == n;
   if ( n < 20 )
   {
      r.msg = us_tr( "too few points" );
      return false;
   }
   vector < double > s( n, 1e0 );
   if ( have_err )
   {
      s = buffer->f_errors[ file ];
      for ( int i = 0; i < n; i++ )
      {
         if ( !( s[ i ] > 0e0 ) )
         {
            have_err = false;
            s.assign( n, 1e0 );
            break;
         }
      }
   }

   bool   ok;
   double q_from = le_q_from->text().toDouble( &ok );
   if ( !ok || le_q_from->text().trimmed().isEmpty() )
   {
      q_from = q[ 0 ];
   }
   double qrg_max = le_qrg->text().toDouble( &ok );
   if ( !ok || qrg_max <= 0e0 )
   {
      r.msg = us_tr( "q Rg is not valid" );
      return false;
   }
   int i0 = 0;
   while ( i0 < n && q[ i0 ] < q_from )
   {
      i0++;
   }
   if ( i0 > n - 12 )
   {
      r.msg = us_tr( "too few points above the first q of the fit" );
      return false;
   }

   double q_to = min( 0.06, q[ n - 1 ] );
   if ( q_to <= q[ i0 + 11 ] )
   {
      q_to = q[ min( n - 1, i0 + 40 ) ];
   }

   vector < double > best;
   double best_chi2 = 1e300;
   int    i1        = i0;
   for ( int iter = 0; iter < 5; iter++ )
   {
      i1 = i0;
      while ( i1 + 1 < n && q[ i1 + 1 ] <= q_to )
      {
         i1++;
      }
      int m = i1 - i0 + 1;
      us_upturn_data d;
      d.q = &q[ i0 ];
      d.I = &I[ i0 ];
      d.s = &s[ i0 ];

      // starting points: the solute from the upper part of the range (Guinier corrected), the
      // large scatterer from the excess at the first point, for a few sizes and slopes
      double Rg0 = qrg_max / q_to;
      vector < double > g0;
      for ( int i = i0 + (int) ( 0.7 * ( m - 1 ) ); i <= i1; i++ )
      {
         g0.push_back( I[ i ] * exp( q[ i ] * q[ i ] * Rg0 * Rg0 / 3e0 ) );
      }
      sort( g0.begin(), g0.end() );
      double G0 = g0.size() ? g0[ g0.size() / 2 ] : I[ i1 ];
      if ( !( G0 > 0e0 ) )
      {
         G0 = fabs( I[ i1 ] ) + 1e-30;
      }
      double q0 = q[ i0 ];
      double E0 = I[ i0 ] - G0 * exp( -q0 * q0 * Rg0 * Rg0 / 3e0 );
      if ( !( E0 > 0e0 ) )
      {
         E0 = 0.01 * G0;
      }

      vector < vector < double > > starts;
      if ( best.size() )
      {
         starts.push_back( best );
      }
      double Ras[] = { 150e0, 400e0, 1000e0 };
      double Pas[] = { 3e0, 4e0 };
      for ( int a = 0; a < 3; a++ )
      {
         for ( int b = 0; b < 2; b++ )
         {
            double Ra0 = Ras[ a ];
            double Pa0 = Pas[ b ];
            double ex  = min( 50e0, q0 * q0 * Ra0 * Ra0 / 3e0 );
            double x   = erf( q0 * Ra0 / sqrt( 6e0 ) );
            x = x * x * x / q0;
            double pw  = exp( -q0 * q0 * Rg0 * Rg0 / 3e0 ) * pow( x, Pa0 );
            vector < double > p( UPT_NPAR );
            p[ 0 ] = log( G0 );
            p[ 1 ] = log( Rg0 );
            p[ 2 ] = log( 0.5 * E0 * exp( ex ) );
            p[ 3 ] = log( Ra0 );
            p[ 4 ] = log( ( pw > 0e0 && std::isfinite( pw ) ) ? 0.5 * E0 / pw : 1e-30 );
            p[ 5 ] = log( ( Pa0 - 1e0 ) / ( 4.5e0 - Pa0 ) );
            starts.push_back( p );
         }
      }

      best_chi2 = 1e300;
      for ( int k = 0; k < (int) starts.size(); k++ )
      {
         vector < double > p = starts[ k ];
         LM::lm_control_struct control = LM::lm_control_double;
         control.printflags = 0;
         control.maxcall    = 300;
         LM::lm_status_struct status;
         LM::lm_use_norm = LM::lm_enorm;
         LM::lmmin( UPT_NPAR, &p[ 0 ], m, (const void *) &d, us_upturn_eval,
                    (const LM::lm_control_struct *) &control, &status, 0 );
         double c2 = us_upturn_chi2( &p[ 0 ], d, m );
         if ( std::isfinite( c2 ) && c2 < best_chi2 )
         {
            best_chi2 = c2;
            best      = p;
         }
      }
      if ( !best.size() )
      {
         r.msg = us_tr( "the fit failed" );
         return false;
      }
      double G, Rg, Ga, Ra, Ba, Pa;
      us_upturn_par( &best[ 0 ], G, Rg, Ga, Ra, Ba, Pa );
      double q_new = qrg_max / Rg;
      q_new = max( q_new, q[ min( n - 1, i0 + 11 ) ] );
      q_new = min( q_new, q[ n - 1 ] );
      if ( fabs( q_new - q_to ) < 0.01 * q_to )
      {
         q_to = q_new;
         break;
      }
      q_to = q_new;
   }
   // the final range
   i1 = i0;
   while ( i1 + 1 < n && q[ i1 + 1 ] <= q_to )
   {
      i1++;
   }
   int m = i1 - i0 + 1;
   us_upturn_data d;
   d.q = &q[ i0 ];
   d.I = &I[ i0 ];
   d.s = &s[ i0 ];
   best_chi2 = us_upturn_chi2( &best[ 0 ], d, m );

   r.ok         = true;
   r.par        = best;
   r.q_fit_from = q[ i0 ];
   r.q_fit_to   = q[ i1 ];
   r.npts       = m;
   r.chi2r      = best_chi2 / (double) max( 1, m - UPT_NPAR );
   us_upturn_par( &best[ 0 ], r.G, r.Rg, r.Ga, r.Ra, r.Ba, r.Pa );

   // the parameters' covariance from the Jacobian of the residuals (central differences), scaled up by
   // the reduced chi^2 when that is above 1 (or by it, without errors)
   vector < double > jac( m * UPT_NPAR );
   vector < double > hstep( UPT_NPAR );
   {
      vector < double > fp( m );
      vector < double > fm( m );
      int info = 0;
      for ( int j = 0; j < UPT_NPAR; j++ )
      {
         vector < double > pp = best;
         vector < double > pm = best;
         hstep[ j ] = 1e-5 * max( 1e0, fabs( best[ j ] ) );
         pp[ j ] += hstep[ j ];
         pm[ j ] -= hstep[ j ];
         us_upturn_eval( &pp[ 0 ], m, (const void *) &d, &fp[ 0 ], &info );
         us_upturn_eval( &pm[ 0 ], m, (const void *) &d, &fm[ 0 ], &info );
         for ( int i = 0; i < m; i++ )
         {
            jac[ i * UPT_NPAR + j ] = ( fp[ i ] - fm[ i ] ) / ( 2e0 * hstep[ j ] );
         }
      }
   }
   vector < double > jtj( UPT_NPAR * UPT_NPAR, 0e0 );
   for ( int a = 0; a < UPT_NPAR; a++ )
   {
      for ( int b = 0; b < UPT_NPAR; b++ )
      {
         double sum = 0e0;
         for ( int i = 0; i < m; i++ )
         {
            sum += jac[ i * UPT_NPAR + a ] * jac[ i * UPT_NPAR + b ];
         }
         jtj[ a * UPT_NPAR + b ] = sum;
      }
   }
   vector < double > cov;
   bool have_cov = us_upturn_invert( jtj, UPT_NPAR, cov );
   if ( !have_cov )
   {
      // a parameter with no effect (e.g. a power law that has vanished): a tiny ridge gives it a huge
      // variance, which its zero derivative then keeps out of the propagated error
      double dmax = 0e0;
      for ( int a = 0; a < UPT_NPAR; a++ )
      {
         dmax = max( dmax, jtj[ a * UPT_NPAR + a ] );
      }
      vector < double > jtj_r = jtj;
      for ( int a = 0; a < UPT_NPAR; a++ )
      {
         jtj_r[ a * UPT_NPAR + a ] += 1e-12 * dmax;
      }
      have_cov = dmax > 0e0 && us_upturn_invert( jtj_r, UPT_NPAR, cov );
   }
   double cov_scale = have_err ? max( 1e0, r.chi2r ) : r.chi2r;

   r.large   .resize( n );
   r.large_sd.resize( n );
   r.solute  .resize( n );
   for ( int i = 0; i < n; i++ )
   {
      r.large [ i ] = us_upturn_large ( q[ i ], &best[ 0 ] );
      r.solute[ i ] = us_upturn_solute( q[ i ], &best[ 0 ] );
      double var = 0e0;
      if ( have_cov )
      {
         double g[ UPT_NPAR ];
         for ( int j = 0; j < UPT_NPAR; j++ )
         {
            vector < double > pp = best;
            vector < double > pm = best;
            pp[ j ] += hstep[ j ];
            pm[ j ] -= hstep[ j ];
            g[ j ] = ( us_upturn_large( q[ i ], &pp[ 0 ] ) - us_upturn_large( q[ i ], &pm[ 0 ] ) ) / ( 2e0 * hstep[ j ] );
         }
         for ( int a = 0; a < UPT_NPAR; a++ )
         {
            for ( int b = 0; b < UPT_NPAR; b++ )
            {
               var += g[ a ] * cov[ a * UPT_NPAR + b ] * g[ b ];
            }
         }
         var *= cov_scale;
      }
      r.large_sd[ i ] = ( var > 0e0 && std::isfinite( var ) ) ? sqrt( var ) : 0e0;
   }

   // an upturn: the term is well above the noise and not negligible next to the solute at the first point
   double noise0 = have_err ? s[ i0 ] : sqrt( r.chi2r );
   r.upturn = r.large[ i0 ] > 3e0 * noise0 && r.large[ i0 ] > 0.05 * r.solute[ i0 ];
   if ( !r.upturn )
   {
      r.msg        = us_tr( "no upturn: left unchanged" );
      r.i_reliable = 0;
      r.i_end      = 0;
      return true;
   }

   // the splice: subtract below the point from which the term stays under the threshold (in the data's
   // standard deviations), but at least up to the end of the fit
   double end_sd = le_end_sd->text().toDouble( &ok );
   if ( !ok || end_sd <= 0e0 )
   {
      end_sd = 0.5;
   }
   int last_big = i0;
   for ( int i = i0; i < n; i++ )
   {
      double noise = have_err ? s[ i ] : sqrt( r.chi2r );
      if ( r.large[ i ] >= end_sd * noise )
      {
         last_big = i;
      }
   }
   r.i_end = min( n, max( last_big + 1, i1 + 1 ) );

   // reliable from the point above which the term stays at most rel times the solute's intensity
   double rel = le_rel->text().toDouble( &ok );
   if ( !ok || rel <= 0e0 )
   {
      rel = 0.25;
   }
   r.i_reliable = i0;
   for ( int i = i0; i < r.i_end; i++ )
   {
      if ( r.large[ i ] > rel * r.solute[ i ] )
      {
         r.i_reliable = i + 1;
      }
   }
   if ( r.i_reliable >= r.i_end )
   {
      r.msg = us_tr( "the upturn dominates the whole fitted range: no reliable correction" );
      r.upturn = false;
      r.ok     = false;
      return false;
   }
   r.msg = r.i_reliable
      ? QString( us_tr( "upturn; %1 point(s) below q %2 not reliable" ) ).arg( r.i_reliable ).arg( q[ r.i_reliable ], 0, 'g', 4 )
      : us_tr( "upturn; corrected throughout" );
   return true;
}

void US_Hydrodyn_Saxs_Buffer_Upturn::fit()
{
   results.clear();
   int n_corr = 0;
   for ( int k = 0; k < (int) files.size(); k++ )
   {
      fit_result r;
      bool ok = fit_one( files[ k ], r );
      results[ files[ k ] ] = r;
      for ( int c = 1; c < t_results->columnCount(); c++ )
      {
         t_results->setItem( k, c, cell( "" ) );
      }
      if ( r.par.size() )
      {
         const vector < double > & q = buffer->f_qs[ files[ k ] ];
         t_results->setItem( k, 1, cell( QString( "%1 - %2" ).arg( r.q_fit_from, 0, 'g', 4 ).arg( r.q_fit_to, 0, 'g', 4 ) ) );
         t_results->setItem( k, 2, cell( QString::number( r.G, 'g', 4 ) ) );
         t_results->setItem( k, 3, cell( QString::number( r.Rg, 'f', 1 ) ) );
         t_results->setItem( k, 4, cell( QString::number( r.Ra, 'f', 0 ) ) );
         t_results->setItem( k, 5, cell( QString::number( r.Pa, 'f', 2 ) ) );
         t_results->setItem( k, 6, cell( QString::number( r.chi2r, 'f', 2 ) ) );
         if ( ok && r.upturn )
         {
            t_results->setItem( k, 7, cell( QString::number( q[ r.i_reliable ], 'g', 4 ) ) );
            t_results->setItem( k, 8, cell( r.i_end < (int) q.size() ? QString::number( q[ r.i_end ], 'g', 4 ) : us_tr( "all" ) ) );
            n_corr++;
         }
      }
      t_results->setItem( k, 9, cell( r.msg, false ) );
   }
   lbl_status->setText( QString( us_tr( "%1 of %2 curve(s) have an upturn to correct." ) ).arg( n_corr ).arg( files.size() ) );
   pb_make->setEnabled( n_corr > 0 );
   show_curve( cmb_curve->currentIndex() );
}

void US_Hydrodyn_Saxs_Buffer_Upturn::clear_plot()
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

void US_Hydrodyn_Saxs_Buffer_Upturn::show_curve( int k )
{
   clear_plot();
   if ( k < 0 || k >= (int) files.size() || !results.count( files[ k ] ) )
   {
      plot->replot();
      return;
   }
   const QString & file = files[ k ];
   const fit_result & r = results[ file ];
   const vector < double > & q = buffer->f_qs[ file ];
   const vector < double > & I = buffer->f_Is[ file ];
   int n = (int) q.size();
   // show up to a few times the splice (or the fit's end)
   double q_show = r.par.size() ? 3e0 * ( r.upturn && r.i_end < n ? q[ r.i_end ] : r.q_fit_to ) : q[ n - 1 ];
   int n_show = 0;
   while ( n_show < n && q[ n_show ] <= q_show )
   {
      n_show++;
   }

   auto add_curve = [ & ]( const QString & title, const vector < double > & x, const vector < double > & y,
                           QPen pen, bool dots ) {
      vector < double > xs;
      vector < double > ys;
      for ( int i = 0; i < (int) x.size(); i++ )
      {
         if ( x[ i ] > 0e0 && y[ i ] > 0e0 && std::isfinite( y[ i ] ) )
         {
            xs.push_back( x[ i ] );
            ys.push_back( y[ i ] );
         }
      }
      if ( !xs.size() )
      {
         return;
      }
      QwtPlotCurve * c = new QwtPlotCurve( title );
      if ( dots )
      {
         c->setStyle( QwtPlotCurve::NoCurve );
         c->setSymbol( new QwtSymbol( QwtSymbol::Ellipse, QBrush( pen.color() ), QPen( pen.color() ), QSize( 3, 3 ) ) );
      }
      else
      {
         c->setStyle( QwtPlotCurve::Lines );
         c->setPen( pen );
      }
      c->setSamples( &xs[ 0 ], &ys[ 0 ], (int) xs.size() );
      c->attach( plot );
      plot_curves.push_back( c );
   };
   auto add_marker = [ & ]( double x, QColor color, const QString & label ) {
      QwtPlotMarker * mk = new QwtPlotMarker;
      mk->setLineStyle( QwtPlotMarker::VLine );
      mk->setLinePen( QPen( color, 1, Qt::DashLine ) );
      mk->setXValue( x );
      mk->setLabel( QwtText( label ) );
      mk->setLabelAlignment( Qt::AlignRight | Qt::AlignTop );
      mk->attach( plot );
      plot_markers.push_back( mk );
   };

   vector < double > xq( q.begin(), q.begin() + n_show );
   vector < double > yI( I.begin(), I.begin() + n_show );

   // the intensity axis spans the data shown; the model terms are drawn only within it
   double y_lo = 1e300;
   double y_hi = 0e0;
   for ( int i = 0; i < n_show; i++ )
   {
      if ( I[ i ] > 0e0 )
      {
         y_lo = min( y_lo, I[ i ] );
         y_hi = max( y_hi, I[ i ] );
      }
   }
   if ( !( y_hi > 0e0 ) )
   {
      y_lo = 1e-3;
      y_hi = 1e0;
   }
   y_lo /= 3e0;
   y_hi *= 3e0;
   auto clip = [ & ]( vector < double > v ) {
      for ( int i = 0; i < (int) v.size(); i++ )
      {
         if ( v[ i ] < y_lo )
         {
            v[ i ] = 0e0;                       // not drawn
         }
      }
      return v;
   };

   add_curve( us_tr( "data" ), xq, yI, QPen( Qt::lightGray ), true );
   if ( r.par.size() )
   {
      vector < double > tot( n_show );
      vector < double > lg( r.large.begin(), r.large.begin() + n_show );
      vector < double > so( r.solute.begin(), r.solute.begin() + n_show );
      for ( int i = 0; i < n_show; i++ )
      {
         tot[ i ] = r.large[ i ] + r.solute[ i ];
      }
      if ( r.upturn )
      {
         // corrected points up to the splice (above it the curve is the data)
         vector < double > xc;
         vector < double > yc;
         vector < double > xu;
         vector < double > yu;
         for ( int i = 0; i < min( n_show, r.i_end ); i++ )
         {
            double v = I[ i ] - r.large[ i ];
            if ( i < r.i_reliable )
            {
               xu.push_back( q[ i ] );
               yu.push_back( v );
            }
            else
            {
               xc.push_back( q[ i ] );
               yc.push_back( v );
            }
         }
         add_curve( us_tr( "corrected" ), xc, yc, QPen( Qt::yellow ), true );
         add_curve( us_tr( "corrected, not reliable" ), xu, yu, QPen( QColor( 255, 140, 0 ) ), true );
      }
      add_curve( us_tr( "fit" ), xq, clip( tot ), QPen( Qt::green, 2 ), false );
      add_curve( us_tr( "large scatterer" ), xq, clip( lg ), QPen( Qt::red, 1, Qt::DashLine ), false );
      add_curve( us_tr( "solute" ), xq, clip( so ), QPen( Qt::cyan, 1, Qt::DashDotLine ), false );
      if ( r.upturn )
      {
         add_marker( q[ r.i_reliable ], Qt::red, us_tr( "reliable" ) );
      }
      bool at_fit_end = r.upturn && r.i_end < n && fabs( q[ r.i_end ] - r.q_fit_to ) < 0.02 * r.q_fit_to;
      if ( r.upturn && r.i_end < n && !at_fit_end )
      {
         add_marker( q[ r.i_end ], Qt::yellow, us_tr( "splice" ) );
      }
      add_marker( r.q_fit_to, Qt::green, at_fit_end ? us_tr( "fit end = splice" ) : us_tr( "fit end" ) );
   }
   plot->setTitle( QwtText( file, QwtText::PlainText ) );
   plot->setAxisAutoScale( QwtPlot::xBottom );
   plot->setAxisScale( QwtPlot::yLeft, y_lo, y_hi );
   plot->replot();
}

void US_Hydrodyn_Saxs_Buffer_Upturn::make()
{
   map < QString, bool > existing;
   for ( int i = 0; i < buffer->lb_files->count(); i++ )
   {
      existing[ buffer->lb_files->item( i )->text() ] = true;
   }
   map < QString, double > concs = buffer->current_concs( true );

   // the points below the reliable limit: proposed to the user, cut or keep
   QStringList unreliable;
   for ( int k = 0; k < (int) files.size(); k++ )
   {
      if ( results.count( files[ k ] ) && results[ files[ k ] ].ok && results[ files[ k ] ].upturn && results[ files[ k ] ].i_reliable > 0 )
      {
         const fit_result & r = results[ files[ k ] ];
         unreliable << QString( us_tr( "%1: %2 point(s) below q %3" ) ).arg( files[ k ] ).arg( r.i_reliable )
            .arg( buffer->f_qs[ files[ k ] ][ r.i_reliable ], 0, 'g', 4 );
      }
   }
   bool cut = true;
   if ( unreliable.size() )
   {
      QMessageBox mb( QMessageBox::Question, windowTitle(),
                      QString( us_tr( "Below these points the large-scatterer term is more than %1 x the solute's intensity, so the "
                                      "corrected values depend on the model (orange in the plot):\n\n%2\n\n"
                                      "Cut them from the corrected copies, or keep them (corrected, and noted in the header)?" ) )
                      .arg( le_rel->text() ).arg( unreliable.join( "\n" ) ),
                      QMessageBox::NoButton, this );
      QPushButton * b_cut  = mb.addButton( us_tr( "Cut them" ), QMessageBox::AcceptRole );
      QPushButton * b_keep = mb.addButton( us_tr( "Keep them, noted" ), QMessageBox::AcceptRole );
      mb.addButton( QMessageBox::Cancel );
      mb.setDefaultButton( b_cut );
      mb.exec();
      if ( mb.clickedButton() == b_cut )
      {
         cut = true;
      }
      else if ( mb.clickedButton() == b_keep )
      {
         cut = false;
      }
      else
      {
         return;
      }
   }
   int n_made = 0;
   for ( int k = 0; k < (int) files.size(); k++ )
   {
      const QString & file = files[ k ];
      if ( !results.count( file ) )
      {
         continue;
      }
      const fit_result & r = results[ file ];
      if ( !r.ok || !r.upturn )
      {
         buffer->editor_msg( "dark blue", QString( us_tr( "Upturn correction: %1: %2" ) ).arg( file ).arg( r.msg ) );
         continue;
      }
      const vector < double > & q  = buffer->f_qs[ file ];
      const vector < double > & I  = buffer->f_Is[ file ];
      const vector < QString > & qs = buffer->f_qs_string[ file ];
      bool have_err = buffer->f_errors.count( file ) && buffer->f_errors[ file ].size() == q.size();
      int n = (int) q.size();

      QString name = file + "_upc";
      int ext = 0;
      while ( existing.count( name ) )
      {
         name = file + QString( "_upc%1" ).arg( ++ext );
      }
      existing[ name ] = true;

      vector < double > nq;
      vector < double > nI;
      vector < double > ne;
      vector < QString > nqs;
      for ( int i = cut ? r.i_reliable : 0; i < n; i++ )
      {
         nq.push_back( q[ i ] );
         nqs.push_back( i < (int) qs.size() ? qs[ i ] : QString::number( q[ i ] ) );
         if ( i < r.i_end )
         {
            nI.push_back( I[ i ] - r.large[ i ] );
            if ( have_err )
            {
               double e = buffer->f_errors[ file ][ i ];
               ne.push_back( sqrt( e * e + r.large_sd[ i ] * r.large_sd[ i ] ) );
            }
         }
         else
         {
            nI.push_back( I[ i ] );
            if ( have_err )
            {
               ne.push_back( buffer->f_errors[ file ][ i ] );
            }
         }
      }

      buffer->lb_created_files->addItem( name );
      buffer->lb_created_files->scrollToItem( buffer->lb_created_files->item( buffer->lb_created_files->count() - 1 ) );
      buffer->lb_files->addItem( name );
      buffer->lb_files->scrollToItem( buffer->lb_files->item( buffer->lb_files->count() - 1 ) );
      buffer->created_files_not_saved[ name ] = true;
      buffer->f_pos      [ name ] = buffer->f_qs.size();
      buffer->f_qs_string[ name ] = nqs;
      buffer->f_qs       [ name ] = nq;
      buffer->f_Is       [ name ] = nI;
      if ( have_err )
      {
         buffer->f_errors[ name ] = ne;
      }
      if ( buffer->f_psv.count( file ) )
      {
         buffer->f_psv[ name ] = buffer->f_psv[ file ];
      }
      if ( buffer->f_I0se.count( file ) )
      {
         buffer->f_I0se[ name ] = buffer->f_I0se[ file ];
      }
      if ( buffer->f_conc.count( file ) )
      {
         buffer->f_conc[ name ] = buffer->f_conc[ file ];
      }
      QString below = cut
         ? QString( us_tr( "; the %1 point(s) below q %2, where the term exceeds %3 x the solute, cut" ) )
         .arg( r.i_reliable ).arg( q[ r.i_reliable ], 0, 'g', 5 ).arg( le_rel->text() )
         : QString( us_tr( "; below q %1, where the term exceeds %2 x the solute, the correction is not reliable" ) )
         .arg( q[ r.i_reliable ], 0, 'g', 5 ).arg( le_rel->text() );
      buffer->f_header[ name ] = QString( us_tr( " Upturn corrected: %1 minus a large-scatterer term (Guinier + power law, Rg %2 A, "
                                                 "slope %3) fitted with a solute Guinier (Rg %4 A) over q %5 to %6, subtracted below q %7%8" ) )
         .arg( file )
         .arg( r.Ra, 0, 'f', 0 )
         .arg( r.Pa, 0, 'f', 2 )
         .arg( r.Rg, 0, 'f', 2 )
         .arg( r.q_fit_from, 0, 'g', 5 )
         .arg( r.q_fit_to, 0, 'g', 5 )
         .arg( r.i_end < n ? QString::number( q[ r.i_end ], 'g', 5 ) : us_tr( "the end" ) )
         .arg( below );

      buffer->update_csv_conc();
      if ( concs.count( file ) && concs[ file ] != 0e0 )
      {
         for ( unsigned int i = 0; i < buffer->csv_conc.data.size(); i++ )
         {
            if ( buffer->csv_conc.data[ i ].size() > 1 && buffer->csv_conc.data[ i ][ 0 ] == name )
            {
               buffer->csv_conc.data[ i ][ 1 ] = QString( "%1" ).arg( concs[ file ] );
            }
         }
      }
      buffer->editor_msg( "black", QString( us_tr( "Upturn correction: %1 made from %2%3" ) ).arg( name ).arg( file ).arg( below ) );
      n_made++;
   }
   if ( buffer->conc_widget )
   {
      buffer->conc_window->refresh( buffer->csv_conc );
   }
   buffer->update_enables();
   lbl_status->setText( QString( us_tr( "%1 corrected cop(ies) added to the produced data." ) ).arg( n_made ) );
}

void US_Hydrodyn_Saxs_Buffer_Upturn::help()
{
   US_Help * online_help = new US_Help( this );
   online_help->show_help( "manual/somo/somo_SAS_data_utility.html" );
}

void US_Hydrodyn_Saxs_Buffer_Upturn::closeEvent( QCloseEvent * e )
{
   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}
