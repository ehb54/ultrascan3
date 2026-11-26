#include "../include/us3_defines.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_hydrodyn_saxs_buffer.h"
#include "../include/us_hydrodyn_saxs_buffer_dilutions.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QCloseEvent>
#include <algorithm>
#include <cmath>

// table columns
#define DIL_USE      0
#define DIL_CURVE    1
#define DIL_STATED   2
#define DIL_SOURCE   3
#define DIL_RATIO    4
#define DIL_REVISED  5
#define DIL_DIFF     6
#define DIL_RATIO_C  7
#define DIL_CHECK    8
#define DIL_OFFSET   9
#define DIL_CHI2R   10

US_Hydrodyn_Saxs_Buffer_Dilutions::US_Hydrodyn_Saxs_Buffer_Dilutions(
                                                                      US_Hydrodyn_Saxs_Buffer * buffer,
                                                                      QStringList               files,
                                                                      QWidget *                 p
                                                                      ) : QFrame( p, Qt::Window )
{
   this->buffer = buffer;
   filling      = false;
   setAttribute( Qt::WA_DeleteOnClose );
   USglobal = new US_Config();
   setPalette( PALET_FRAME );
   setWindowTitle( us_tr( "US-SOMO: SAXS Data Utility : Check dilutions" ) );

   // the stated concentration: the Concentrations table, else the Conc of the file header
   map < QString, double > concs = buffer->current_concs( true );
   for ( int i = 0; i < (int) files.size(); i++ )
   {
      if ( concs.count( files[ i ] ) && concs[ files[ i ] ] > 0e0 )
      {
         stated     [ files[ i ] ] = concs[ files[ i ] ];
         stated_from[ files[ i ] ] = "table";
      }
      else if ( buffer->f_conc.count( files[ i ] ) && buffer->f_conc[ files[ i ] ] > 0e0 )
      {
         stated     [ files[ i ] ] = buffer->f_conc[ files[ i ] ];
         stated_from[ files[ i ] ] = "file";
      }
   }
   // highest concentration first; curves without one last, in the order given
   vector < pair < double, QString > > order;
   for ( int i = 0; i < (int) files.size(); i++ )
   {
      order.push_back( pair < double, QString >( stated.count( files[ i ] ) ? -stated[ files[ i ] ] : 1e0, files[ i ] ) );
   }
   stable_sort( order.begin(), order.end(),
                []( const pair < double, QString > & a, const pair < double, QString > & b ) { return a.first < b.first; } );
   for ( int i = 0; i < (int) order.size(); i++ )
   {
      this->files << order[ i ].second;
   }

   fit_q_from = 0e0;
   fit_q_to   = 0e0;

   setupGUI();

   global_Xpos += 30;
   global_Ypos += 30;
   setGeometry( global_Xpos, global_Ypos, 1100, 500 );

   fit();
}

US_Hydrodyn_Saxs_Buffer_Dilutions::~US_Hydrodyn_Saxs_Buffer_Dilutions()
{
}

void US_Hydrodyn_Saxs_Buffer_Dilutions::setupGUI()
{
   int minHeight1 = 24;

   lbl_info = new QLabel( us_tr( "Each curve is fitted over the q range as ratio x reference + offset; the ratio times the reference's "
                                 "concentration is the revised concentration. The reference (by default the most concentrated curve) is "
                                 "taken as correct, so all the revised concentrations scale with its concentration: choose the curve whose "
                                 "concentration is best known. Use a q range where interactions play no role (high q); the ratio over the check "
                                 "range should agree, and when it does not the difference is not just the concentration.\n"
                                 "Type a stated concentration where there is none (or to change one), then press Fit. Make the new series "
                                 "copies the curves, unchanged, each with its stated or its revised concentration (Use revised)." ), this );
   lbl_info->setWordWrap( true );
   lbl_info->setAlignment( Qt::AlignLeft | Qt::AlignVCenter );
   lbl_info->setPalette( PALET_LABEL );
   AUTFBACK( lbl_info );
   lbl_info->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );

   // default ranges from the q range all the curves cover
   double q_lo = -1e99;
   double q_hi = 1e99;
   for ( int i = 0; i < (int) files.size(); i++ )
   {
      const vector < double > & q = buffer->f_qs[ files[ i ] ];
      if ( q.size() )
      {
         q_lo = max( q_lo, q.front() );
         q_hi = min( q_hi, q.back() );
      }
   }
   double fit_from = ( q_hi > 0.2 ) ? max( q_lo, 0.15 ) : q_lo + 0.5 * ( q_hi - q_lo );
   double chk_from = max( q_lo, 0.03 );
   QString s_fit_from = QString::number( fit_from, 'g', 4 );
   QString s_fit_to   = QString::number( q_hi, 'g', 6 );
   QString s_chk_from = chk_from < fit_from ? QString::number( chk_from, 'g', 4 ) : QString( "" );
   QString s_chk_to   = chk_from < fit_from ? s_fit_from : QString( "" );

   QLabel * lbl_ref    = new QLabel( us_tr( " Reference:" ), this );
   QLabel * lbl_q      = new QLabel( us_tr( "  Fit over q from" ), this );
   QLabel * lbl_q_to   = new QLabel( us_tr( "to" ), this );
   QLabel * lbl_qc     = new QLabel( us_tr( "  Check range: q from" ), this );
   QLabel * lbl_qc_to  = new QLabel( us_tr( "to" ), this );
   QLabel * labels[]   = { lbl_ref, lbl_q, lbl_q_to, lbl_qc, lbl_qc_to };
   for ( int k = 0; k < 5; k++ )
   {
      labels[ k ]->setPalette( PALET_NORMAL );
      AUTFBACK( labels[ k ] );
      labels[ k ]->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   }

   cmb_ref = new QComboBox( this );
   cmb_ref->setPalette( PALET_NORMAL );
   AUTFBACK( cmb_ref );
   cmb_ref->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cmb_ref->addItems( files );
   cmb_ref->setToolTip( us_tr( "The curve whose concentration is taken as correct (default: the most concentrated);\n"
                               "all the revised concentrations scale with it, so choose the curve whose concentration is best known" ) );
   connect( cmb_ref, SIGNAL( currentIndexChanged( int ) ), SLOT( inputs_changed() ) );

   le_q_from  = new QLineEdit( s_fit_from, this );
   le_q_to    = new QLineEdit( s_fit_to, this );
   le_qc_from = new QLineEdit( s_chk_from, this );
   le_qc_to   = new QLineEdit( s_chk_to, this );
   QLineEdit * edits[] = { le_q_from, le_q_to, le_qc_from, le_qc_to };
   for ( int k = 0; k < 4; k++ )
   {
      edits[ k ]->setPalette( PALET_NORMAL );
      AUTFBACK( edits[ k ] );
      edits[ k ]->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
      edits[ k ]->setMinimumHeight( minHeight1 );
      edits[ k ]->setMaximumWidth( 90 );
      connect( edits[ k ], SIGNAL( textChanged( const QString & ) ), SLOT( inputs_changed() ) );
   }
   le_q_from ->setToolTip( us_tr( "The ratios are fitted over this q range [1/Angstrom]" ) );
   le_q_to   ->setToolTip( us_tr( "The ratios are fitted over this q range [1/Angstrom]" ) );
   le_qc_from->setToolTip( us_tr( "The ratios are fitted again over this q range, as a check (leave empty for none)" ) );
   le_qc_to  ->setToolTip( us_tr( "The ratios are fitted again over this q range, as a check (leave empty for none)" ) );

   cb_offset = new QCheckBox( us_tr( "Offset" ), this );
   cb_offset->setChecked( true );
   cb_offset->setPalette( PALET_NORMAL );
   AUTFBACK( cb_offset );
   cb_offset->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   cb_offset->setToolTip( us_tr( "Also fit a constant offset (e.g. from an imperfect buffer subtraction)" ) );
   connect( cb_offset, SIGNAL( clicked() ), SLOT( inputs_changed() ) );

   pb_fit = new QPushButton( us_tr( "Fit" ), this );
   pb_fit->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   pb_fit->setMinimumHeight( minHeight1 );
   pb_fit->setPalette( PALET_PUSHB );
   connect( pb_fit, SIGNAL( clicked() ), SLOT( fit() ) );

   QStringList hdr;
   hdr << us_tr( "Use\nrevised" ) << us_tr( "Curve" ) << us_tr( "Stated conc.\n[mg/ml]" ) << us_tr( "Stated\nfrom" )
       << us_tr( "Ratio to\nreference" ) << us_tr( "Revised conc.\n[mg/ml]" ) << us_tr( "Difference" )
       << us_tr( "Ratio over\ncheck range" ) << us_tr( "Check vs.\nfit" ) << us_tr( "Offset" ) << us_tr( "Reduced\nchi^2" );
   filling = true;
   t_results = new QTableWidget( (int) files.size(), hdr.size(), this );
   t_results->setHorizontalHeaderLabels( hdr );
   t_results->verticalHeader()->hide();
   t_results->setPalette( PALET_EDIT );
   AUTFBACK( t_results );
   t_results->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
   t_results->setEditTriggers( QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed | QAbstractItemView::AnyKeyPressed );
   t_results->setSelectionMode( QAbstractItemView::SingleSelection );
   for ( int i = 0; i < (int) files.size(); i++ )
   {
      QTableWidgetItem * use = new QTableWidgetItem();
      use->setFlags( Qt::ItemIsEnabled );
      use->setCheckState( Qt::Unchecked );
      t_results->setItem( i, DIL_USE, use );
      t_results->setItem( i, DIL_CURVE, cell( files[ i ], false ) );
      QTableWidgetItem * st = cell( stated.count( files[ i ] ) ? QString( "%1" ).arg( stated[ files[ i ] ] ) : QString( "" ) );
      st->setFlags( Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable );
      st->setToolTip( us_tr( "Double-click to type the stated (theoretical) concentration" ) );
      t_results->setItem( i, DIL_STATED, st );
      t_results->setItem( i, DIL_SOURCE, cell( stated_from.count( files[ i ] )
                                               ? ( stated_from[ files[ i ] ] == "table" ? us_tr( "table" ) : us_tr( "file" ) )
                                               : us_tr( "none: enter it" ), false ) );
   }
   filling = false;
   t_results->horizontalHeader()->setSectionResizeMode( QHeaderView::ResizeToContents );
   t_results->horizontalHeader()->setStretchLastSection( true );
   connect( t_results, SIGNAL( cellChanged( int, int ) ), SLOT( table_changed( int, int ) ) );

   lbl_status = new QLabel( "", this );
   lbl_status->setWordWrap( true );
   lbl_status->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_status );
   lbl_status->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );

   pb_help  = new QPushButton( us_tr( "Help" ), this );
   pb_make  = new QPushButton( us_tr( "Make the new series" ), this );
   pb_close = new QPushButton( us_tr( "Close" ), this );
   QPushButton * buttons[] = { pb_help, pb_make, pb_close };
   for ( int k = 0; k < 3; k++ )
   {
      buttons[ k ]->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1 ) );
      buttons[ k ]->setMinimumHeight( minHeight1 );
      buttons[ k ]->setPalette( PALET_PUSHB );
   }
   pb_make->setToolTip( us_tr( "Add to the Produced Data a copy of each curve (<name>_dc), its intensities unchanged, with its revised\n"
                               "concentration where Use revised is checked and its stated one otherwise; Save writes them with that\n"
                               "concentration in the file header" ) );
   connect( pb_help,  SIGNAL( clicked() ), SLOT( help() ) );
   connect( pb_make,  SIGNAL( clicked() ), SLOT( make_series() ) );
   connect( pb_close, SIGNAL( clicked() ), SLOT( close() ) );

   QHBoxLayout * hbl_opts = new QHBoxLayout; hbl_opts->setContentsMargins( 0, 0, 0, 0 ); hbl_opts->setSpacing( 2 );
   hbl_opts->addWidget( lbl_ref );
   hbl_opts->addWidget( cmb_ref, 1 );
   hbl_opts->addWidget( lbl_q );
   hbl_opts->addWidget( le_q_from );
   hbl_opts->addWidget( lbl_q_to );
   hbl_opts->addWidget( le_q_to );
   hbl_opts->addWidget( lbl_qc );
   hbl_opts->addWidget( le_qc_from );
   hbl_opts->addWidget( lbl_qc_to );
   hbl_opts->addWidget( le_qc_to );
   hbl_opts->addWidget( cb_offset );
   hbl_opts->addWidget( pb_fit );

   QHBoxLayout * hbl_bottom = new QHBoxLayout; hbl_bottom->setContentsMargins( 0, 0, 0, 0 ); hbl_bottom->setSpacing( 0 );
   hbl_bottom->addWidget( pb_help );
   hbl_bottom->addWidget( pb_make, 1 );
   hbl_bottom->addWidget( pb_close );

   QVBoxLayout * background = new QVBoxLayout( this ); background->setContentsMargins( 0, 0, 0, 0 ); background->setSpacing( 1 );
   background->addWidget( lbl_info );
   background->addLayout( hbl_opts );
   background->addWidget( t_results, 1 );
   background->addWidget( lbl_status );
   background->addLayout( hbl_bottom );
}

QTableWidgetItem * US_Hydrodyn_Saxs_Buffer_Dilutions::cell( const QString & text, bool numeric )
{
   QTableWidgetItem * it = new QTableWidgetItem( text );
   it->setFlags( Qt::ItemIsEnabled );
   it->setTextAlignment( numeric ? ( Qt::AlignRight | Qt::AlignVCenter ) : ( Qt::AlignLeft | Qt::AlignVCenter ) );
   return it;
}

// least squares I_file( q ) = ratio * I_ref( q ) + offset over the reference's q points in [ q_from, q_to ],
// the file's intensity (and error) linearly interpolated onto them. With errors on both curves the line is
// fitted with errors in both coordinates (York et al., Am. J. Phys. 72, 367 (2004); through the origin without
// the offset), each point weighted by 1 / ( sd_file^2 + ratio^2 sd_ref^2 ): weighting a least squares fit of
// the file on the reference alone biases the ratio towards 0 when the reference is noisy (a dilute reference).
// Fitting the reference to the file then gives the inverse ratio. Without errors, unweighted least squares.
// ratio_sd is the ratio's standard error, scaled up by sqrt( reduced chi^2 ) when that is above 1.
bool US_Hydrodyn_Saxs_Buffer_Dilutions::fit_ratio( const QString & file, const QString & ref, double q_from, double q_to,
                                                   bool offset, double & ratio, double & ratio_sd, double & off,
                                                   double & chi2r, int & npts, QString & err )
{
   ratio    = 0e0;
   ratio_sd = 0e0;
   off      = 0e0;
   chi2r    = 0e0;
   npts     = 0;
   const vector < double > & qr = buffer->f_qs[ ref ];
   const vector < double > & Ir = buffer->f_Is[ ref ];
   const vector < double > & qf = buffer->f_qs[ file ];
   const vector < double > & If = buffer->f_Is[ file ];
   bool have_err =
      buffer->f_errors.count( ref )  && buffer->f_errors[ ref ].size()  == qr.size() &&
      buffer->f_errors.count( file ) && buffer->f_errors[ file ].size() == qf.size();
   if ( qf.size() < 2 || qr.size() < 2 )
   {
      err = us_tr( "too few points" );
      return false;
   }

   vector < double > x;
   vector < double > y;
   vector < double > sx;
   vector < double > sy;
   for ( int i = 0; i < (int) qr.size(); i++ )
   {
      double q = qr[ i ];
      if ( q < q_from || q > q_to || q < qf.front() || q > qf.back() )
      {
         continue;
      }
      int j = (int) ( upper_bound( qf.begin(), qf.end(), q ) - qf.begin() );
      if ( j >= (int) qf.size() )
      {
         j = (int) qf.size() - 1;
      }
      if ( j < 1 )
      {
         j = 1;
      }
      double t  = ( qf[ j ] != qf[ j - 1 ] ) ? ( q - qf[ j - 1 ] ) / ( qf[ j ] - qf[ j - 1 ] ) : 0e0;
      double yv = If[ j - 1 ] + t * ( If[ j ] - If[ j - 1 ] );
      x.push_back( Ir[ i ] );
      y.push_back( yv );
      if ( have_err )
      {
         const vector < double > & er = buffer->f_errors[ ref ];
         const vector < double > & ef = buffer->f_errors[ file ];
         sx.push_back( er[ i ] );
         sy.push_back( ef[ j - 1 ] + t * ( ef[ j ] - ef[ j - 1 ] ) );
      }
   }
   npts = (int) x.size();
   int nparam = offset ? 2 : 1;
   if ( npts < nparam + 2 )
   {
      err = QString( us_tr( "only %1 point(s) in the q range" ) ).arg( npts );
      return false;
   }

   double sum_x = 0e0;
   double sum_y = 0e0;
   for ( int i = 0; i < npts; i++ )
   {
      sum_x += x[ i ];
      sum_y += y[ i ];
   }
   ratio = ( sum_x != 0e0 ) ? sum_y / sum_x : 1e0;

   // a first estimate: weighted least squares of the file on the reference
   double Sw = 0e0, Sx = 0e0, Sy = 0e0, Sxx = 0e0, Sxy = 0e0;
   for ( int it = 0; it < ( have_err ? 6 : 1 ); it++ )
   {
      Sw = Sx = Sy = Sxx = Sxy = 0e0;
      for ( int i = 0; i < npts; i++ )
      {
         double w = 1e0;
         if ( have_err )
         {
            double v = sy[ i ] * sy[ i ] + ratio * ratio * sx[ i ] * sx[ i ];
            w = ( v > 0e0 ) ? 1e0 / v : 0e0;
         }
         Sw  += w;
         Sx  += w * x[ i ];
         Sy  += w * y[ i ];
         Sxx += w * x[ i ] * x[ i ];
         Sxy += w * x[ i ] * y[ i ];
      }
      if ( offset )
      {
         double det = Sw * Sxx - Sx * Sx;
         if ( det == 0e0 )
         {
            err = us_tr( "singular fit" );
            return false;
         }
         ratio = ( Sw * Sxy - Sx * Sy ) / det;
         off   = ( Sxx * Sy - Sx * Sxy ) / det;
      }
      else
      {
         if ( Sxx == 0e0 )
         {
            err = us_tr( "singular fit" );
            return false;
         }
         ratio = Sxy / Sxx;
         off   = 0e0;
      }
   }
   double var = offset ? Sw / ( Sw * Sxx - Sx * Sx ) : 1e0 / Sxx;

   if ( have_err )
   {
      // errors in both coordinates (York et al. 2004): the weights W, the weighted means (0 through the origin)
      // and beta for a given ratio; the next ratio is sum( W beta V ) / sum( W beta U )
      vector < double > W( npts );
      vector < double > beta( npts );
      double Xb = 0e0;
      double Yb = 0e0;
      auto york_step = [ & ]( double b, double & b_out ) -> bool {
         double sw  = 0e0;
         double swx = 0e0;
         double swy = 0e0;
         for ( int i = 0; i < npts; i++ )
         {
            double v = sy[ i ] * sy[ i ] + b * b * sx[ i ] * sx[ i ];
            W[ i ] = ( v > 0e0 ) ? 1e0 / v : 0e0;
            sw  += W[ i ];
            swx += W[ i ] * x[ i ];
            swy += W[ i ] * y[ i ];
         }
         if ( sw == 0e0 )
         {
            return false;
         }
         Xb = offset ? swx / sw : 0e0;
         Yb = offset ? swy / sw : 0e0;
         double num = 0e0;
         double den = 0e0;
         for ( int i = 0; i < npts; i++ )
         {
            double U = x[ i ] - Xb;
            double V = y[ i ] - Yb;
            beta[ i ] = W[ i ] * ( U * sy[ i ] * sy[ i ] + b * V * sx[ i ] * sx[ i ] );
            num += W[ i ] * beta[ i ] * V;
            den += W[ i ] * beta[ i ] * U;
         }
         if ( den == 0e0 )
         {
            return false;
         }
         b_out = num / den;
         return true;
      };
      bool converged = false;
      for ( int it = 0; it < 200 && !converged; it++ )
      {
         double b_next;
         if ( !york_step( ratio, b_next ) )
         {
            err = us_tr( "singular fit" );
            return false;
         }
         converged = b_next == ratio || fabs( b_next - ratio ) <= 1e-12 * fabs( b_next );
         ratio = b_next;
      }
      double b_next;
      if ( !converged || !york_step( ratio, b_next ) )
      {
         err = us_tr( "the fit did not converge" );
         return false;
      }
      off = Yb - ratio * Xb;
      // the variance of the ratio, from the adjusted reference values Xb + beta
      double sw   = 0e0;
      double swxa = 0e0;
      for ( int i = 0; i < npts; i++ )
      {
         sw   += W[ i ];
         swxa += W[ i ] * ( Xb + beta[ i ] );
      }
      double xm   = offset ? swxa / sw : 0e0;
      double swu2 = 0e0;
      for ( int i = 0; i < npts; i++ )
      {
         double u = Xb + beta[ i ] - xm;
         swu2 += W[ i ] * u * u;
      }
      if ( swu2 <= 0e0 )
      {
         err = us_tr( "singular fit" );
         return false;
      }
      var = 1e0 / swu2;
   }

   double chi2 = 0e0;
   for ( int i = 0; i < npts; i++ )
   {
      double w = 1e0;
      if ( have_err )
      {
         double v = sy[ i ] * sy[ i ] + ratio * ratio * sx[ i ] * sx[ i ];
         w = ( v > 0e0 ) ? 1e0 / v : 0e0;
      }
      double r = y[ i ] - ratio * x[ i ] - off;
      chi2 += w * r * r;
   }
   chi2r = chi2 / (double) ( npts - nparam );
   // without errors the weights are 1 and the variance comes from the scatter of the residuals
   double scale = have_err ? max( 1e0, chi2r ) : chi2r;
   ratio_sd = sqrt( var * scale );
   if ( !have_err )
   {
      chi2r = 0e0;
   }
   return true;
}

void US_Hydrodyn_Saxs_Buffer_Dilutions::inputs_changed()
{
   lbl_status->setText( us_tr( "Settings changed: press Fit to update the table." ) );
   pb_make->setEnabled( false );
}

// a stated concentration typed in by the user
void US_Hydrodyn_Saxs_Buffer_Dilutions::table_changed( int row, int col )
{
   if ( filling || col != DIL_STATED || row < 0 || row >= (int) files.size() )
   {
      return;
   }
   filling = true;
   t_results->setItem( row, DIL_SOURCE, cell( us_tr( "entered" ), false ) );
   filling = false;
   stated_from[ files[ row ] ] = "entered";
   inputs_changed();
}

// the stated concentrations as the table shows them; empty or invalid ones are highlighted
void US_Hydrodyn_Saxs_Buffer_Dilutions::read_stated()
{
   filling = true;
   for ( int i = 0; i < (int) files.size(); i++ )
   {
      QTableWidgetItem * it = t_results->item( i, DIL_STATED );
      bool ok = false;
      double v = it ? it->text().trimmed().toDouble( &ok ) : 0e0;
      if ( ok && v > 0e0 )
      {
         stated[ files[ i ] ] = v;
         it->setBackground( QBrush() );
         it->setToolTip( us_tr( "Double-click to type the stated (theoretical) concentration" ) );
      }
      else
      {
         stated.erase( files[ i ] );
         if ( it )
         {
            it->setBackground( QColor( 255, 200, 200 ) );
            it->setToolTip( us_tr( "No concentration: double-click and type the theoretical value" ) );
         }
      }
   }
   filling = false;
}

void US_Hydrodyn_Saxs_Buffer_Dilutions::fit()
{
   read_stated();
   fit_ratio_val.clear();
   fit_implied.clear();
   fit_ref = cmb_ref->currentText();
   pb_make->setEnabled( false );

   bool ok_from;
   bool ok_to;
   fit_q_from = le_q_from->text().toDouble( &ok_from );
   fit_q_to   = le_q_to  ->text().toDouble( &ok_to );
   int n_missing = 0;
   for ( int i = 0; i < (int) files.size(); i++ )
   {
      n_missing += stated.count( files[ i ] ) ? 0 : 1;
   }
   QString missing_note = n_missing
      ? QString( us_tr( " %1 curve(s) have no stated concentration (red): double-click to type the theoretical value, to see it "
                        "next to the revised one." ) ).arg( n_missing )
      : QString( "" );
   if ( !stated.count( fit_ref ) )
   {
      lbl_status->setText( QString( us_tr( "The reference %1 has no concentration: double-click its stated concentration and "
                                           "type it, then press Fit." ) ).arg( fit_ref ) + missing_note );
      return;
   }
   if ( !ok_from || !ok_to || fit_q_to <= fit_q_from )
   {
      lbl_status->setText( us_tr( "The fit q range is not valid." ) );
      return;
   }
   bool ok_cf;
   bool ok_ct;
   double qc_from = le_qc_from->text().toDouble( &ok_cf );
   double qc_to   = le_qc_to  ->text().toDouble( &ok_ct );
   bool check = ok_cf && ok_ct && qc_to > qc_from;
   bool offset = cb_offset->isChecked();
   double c_ref = stated[ fit_ref ];

   filling = true;
   int n_flag    = 0;
   int n_fit     = 0;
   int npts_used = 0;
   for ( int i = 0; i < (int) files.size(); i++ )
   {
      const QString & f = files[ i ];
      for ( int c = DIL_RATIO; c < t_results->columnCount(); c++ )
      {
         t_results->setItem( i, c, cell( "" ) );
      }
      t_results->item( i, DIL_USE )->setCheckState( Qt::Unchecked );
      t_results->item( i, DIL_USE )->setFlags( Qt::ItemIsEnabled );
      if ( f == fit_ref )
      {
         fit_implied[ f ] = c_ref;
         t_results->setItem( i, DIL_RATIO, cell( "1" ) );
         t_results->setItem( i, DIL_REVISED, cell( QString( "%1" ).arg( c_ref ) ) );
         t_results->setItem( i, DIL_DIFF, cell( us_tr( "reference" ) ) );
         continue;
      }
      double ratio, ratio_sd, off, chi2r;
      int    npts;
      QString err;
      if ( !fit_ratio( f, fit_ref, fit_q_from, fit_q_to, offset, ratio, ratio_sd, off, chi2r, npts, err ) )
      {
         t_results->setItem( i, DIL_DIFF, cell( err, false ) );
         continue;
      }
      n_fit++;
      npts_used = npts;
      fit_ratio_val[ f ] = ratio;
      double implied = ratio * c_ref;
      fit_implied[ f ] = implied;
      t_results->setItem( i, DIL_RATIO, cell( QString( "%1 \302\261 %2" ).arg( ratio, 0, 'g', 5 ).arg( ratio_sd, 0, 'g', 2 ) ) );
      t_results->setItem( i, DIL_REVISED, cell( QString::number( implied, 'g', 5 ) ) );
      if ( stated.count( f ) )
      {
         t_results->setItem( i, DIL_DIFF, cell( QString( "%1%2 %" ).arg( implied / stated[ f ] >= 1e0 ? "+" : "" )
                                                .arg( 100e0 * ( implied / stated[ f ] - 1e0 ), 0, 'f', 1 ) ) );
      }
      t_results->setItem( i, DIL_OFFSET, cell( QString::number( off, 'g', 3 ) ) );
      t_results->setItem( i, DIL_CHI2R, cell( chi2r > 0e0 ? QString::number( chi2r, 'f', 2 ) : QString( "" ) ) );
      bool flagged = false;
      if ( check )
      {
         double r2, r2_sd, off2, chi2r2;
         int    npts2;
         QString err2;
         if ( fit_ratio( f, fit_ref, qc_from, qc_to, offset, r2, r2_sd, off2, chi2r2, npts2, err2 ) )
         {
            double rel = r2 / ratio - 1e0;
            double tol = 0.02 + 3e0 * sqrt( ratio_sd * ratio_sd + r2_sd * r2_sd ) / fabs( ratio );
            flagged = fabs( rel ) > tol;
            t_results->setItem( i, DIL_RATIO_C, cell( QString( "%1 \302\261 %2" ).arg( r2, 0, 'g', 5 ).arg( r2_sd, 0, 'g', 2 ) ) );
            t_results->setItem( i, DIL_CHECK, cell( QString( "%1%2 %" ).arg( rel >= 0e0 ? "+" : "" ).arg( 100e0 * rel, 0, 'f', 1 ) ) );
            if ( flagged )
            {
               n_flag++;
               for ( int c = DIL_RATIO_C; c <= DIL_CHECK; c++ )
               {
                  t_results->item( i, c )->setBackground( QColor( 255, 200, 200 ) );
                  t_results->item( i, c )->setToolTip( us_tr( "The ratio depends on the q range: over the check range, interactions (or\n"
                                                              "something other than the concentration) differ between this curve and the\n"
                                                              "reference. The revised value comes from the fit range, where that is assumed\n"
                                                              "not to happen." ) );
               }
            }
         }
         else
         {
            t_results->setItem( i, DIL_CHECK, cell( err2, false ) );
         }
      }
      // revised by default; a disagreement over the check range is shown, the choice is the user's
      t_results->item( i, DIL_USE )->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled );
      t_results->item( i, DIL_USE )->setCheckState( Qt::Checked );
   }
   filling = false;
   QString st = QString( us_tr( "Reference %1 (%2 mg/ml); fitted over q %3 to %4 (%5 points each)%6." ) )
      .arg( fit_ref ).arg( c_ref ).arg( fit_q_from ).arg( fit_q_to ).arg( npts_used )
      .arg( check ? QString( us_tr( ", checked over q %1 to %2" ) ).arg( qc_from ).arg( qc_to ) : QString( "" ) );
   if ( n_flag )
   {
      st += QString( us_tr( " %1 curve(s) give a different ratio over the check range (red): over that range their difference "
                            "from the reference is not just the concentration." ) ).arg( n_flag );
      if ( 2 * n_flag >= n_fit && n_fit > 1 )
      {
         st += us_tr( " As most curves disagree, the reference itself probably shows interactions over the check range "
                      "(its revised values from the fit range are not affected by that)." );
      }
   }
   lbl_status->setText( st + missing_note );
   pb_make->setEnabled( true );
}

// the new series: a copy of every curve, intensities unchanged, with the revised or the stated concentration
void US_Hydrodyn_Saxs_Buffer_Dilutions::make_series()
{
   map < QString, double > chosen;
   map < QString, bool >   revised;
   QStringList summary;
   QStringList skipped;
   for ( int i = 0; i < (int) files.size(); i++ )
   {
      const QString & f = files[ i ];
      bool use_rev = t_results->item( i, DIL_USE )->checkState() == Qt::Checked && fit_implied.count( f );
      if ( f == fit_ref )
      {
         use_rev = false;
      }
      double c = use_rev ? fit_implied[ f ] : ( stated.count( f ) ? stated[ f ] : 0e0 );
      if ( !( c > 0e0 ) )
      {
         skipped << f;
         continue;
      }
      chosen [ f ] = c;
      revised[ f ] = use_rev;
      summary << QString( "%1: %2 mg/ml (%3)" ).arg( f ).arg( c, 0, 'g', 6 )
         .arg( f == fit_ref ? us_tr( "reference" ) : use_rev ? us_tr( "revised" ) : us_tr( "stated" ) );
   }
   if ( !chosen.size() )
   {
      lbl_status->setText( us_tr( "No curve has a concentration to use." ) );
      return;
   }
   QString q = QString( us_tr( "Make a new series of %1 curve(s), their intensities unchanged, with these concentrations?\n\n%2" ) )
      .arg( chosen.size() ).arg( summary.join( "\n" ) );
   if ( skipped.size() )
   {
      q += QString( us_tr( "\n\nWithout a concentration, not copied: %1" ) ).arg( skipped.join( ", " ) );
   }
   if ( QMessageBox::question( this, windowTitle(), q, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes ) != QMessageBox::Yes )
   {
      return;
   }

   map < QString, bool > existing;
   for ( int i = 0; i < buffer->lb_files->count(); i++ )
   {
      existing[ buffer->lb_files->item( i )->text() ] = true;
   }
   int n_made = 0;
   for ( int i = 0; i < (int) files.size(); i++ )
   {
      const QString & f = files[ i ];
      if ( !chosen.count( f ) )
      {
         continue;
      }
      QString name = f + "_dc";
      int ext = 0;
      while ( existing.count( name ) )
      {
         name = f + QString( "_dc%1" ).arg( ++ext );
      }
      existing[ name ] = true;

      buffer->lb_created_files->addItem( name );
      buffer->lb_created_files->scrollToItem( buffer->lb_created_files->item( buffer->lb_created_files->count() - 1 ) );
      buffer->lb_files->addItem( name );
      buffer->lb_files->scrollToItem( buffer->lb_files->item( buffer->lb_files->count() - 1 ) );
      buffer->created_files_not_saved[ name ] = true;
      buffer->f_pos      [ name ] = buffer->f_qs.size();
      buffer->f_qs_string[ name ] = buffer->f_qs_string[ f ];
      buffer->f_qs       [ name ] = buffer->f_qs[ f ];
      buffer->f_Is       [ name ] = buffer->f_Is[ f ];
      if ( buffer->f_errors.count( f ) )
      {
         buffer->f_errors[ name ] = buffer->f_errors[ f ];
      }
      if ( buffer->f_psv.count( f ) )
      {
         buffer->f_psv[ name ] = buffer->f_psv[ f ];
      }
      if ( buffer->f_I0se.count( f ) )
      {
         buffer->f_I0se[ name ] = buffer->f_I0se[ f ];
      }
      buffer->f_conc[ name ] = chosen[ f ];
      QString how;
      if ( f == fit_ref )
      {
         how = QString( us_tr( "the reference of the dilution check, concentration %1 mg/ml kept" ) ).arg( chosen[ f ], 0, 'g', 6 );
      }
      else if ( revised[ f ] )
      {
         how = QString( us_tr( "concentration revised to %1 mg/ml (stated %2) from its intensity ratio %3 to %4 at %5 mg/ml over q %6 to %7" ) )
            .arg( chosen[ f ], 0, 'g', 6 )
            .arg( stated.count( f ) ? QString( "%1" ).arg( stated[ f ] ) : us_tr( "none" ) )
            .arg( fit_ratio_val[ f ], 0, 'g', 6 ).arg( fit_ref ).arg( stated[ fit_ref ] ).arg( fit_q_from ).arg( fit_q_to );
      }
      else
      {
         how = QString( us_tr( "stated concentration %1 mg/ml kept (revised value %2 not used)" ) ).arg( chosen[ f ], 0, 'g', 6 )
            .arg( fit_implied.count( f ) ? QString::number( fit_implied[ f ], 'g', 6 ) : us_tr( "none" ) );
      }
      buffer->f_header[ name ] = QString( us_tr( " Dilution checked: %1, %2" ) ).arg( f ).arg( how );

      buffer->update_csv_conc();
      for ( unsigned int r = 0; r < buffer->csv_conc.data.size(); r++ )
      {
         if ( buffer->csv_conc.data[ r ].size() > 1 && buffer->csv_conc.data[ r ][ 0 ] == name )
         {
            buffer->csv_conc.data[ r ][ 1 ] = QString::number( chosen[ f ], 'g', 6 );
         }
      }
      buffer->editor_msg( "black", QString( us_tr( "Check dilutions: %1 made from %2, %3" ) ).arg( name ).arg( f ).arg( how ) );
      n_made++;
   }
   buffer->update_csv_conc();
   if ( buffer->conc_widget )
   {
      buffer->conc_window->refresh( buffer->csv_conc );
   }
   buffer->update_enables();
   lbl_status->setText( QString( us_tr( "%1 curve(s) added to the Produced Data with their concentrations; Save there writes them "
                                        "with the concentration in the file header." ) ).arg( n_made ) );
}

void US_Hydrodyn_Saxs_Buffer_Dilutions::help()
{
   US_Help * online_help = new US_Help( this );
   online_help->show_help( "manual/somo/somo_SAS_data_utility.html" );
}

void US_Hydrodyn_Saxs_Buffer_Dilutions::closeEvent( QCloseEvent * e )
{
   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}
