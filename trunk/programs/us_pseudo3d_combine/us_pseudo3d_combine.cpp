//! \file us_pseudo3d_combine.cpp

#include <QApplication>

#include <uuid/uuid.h>

#include "us_pseudo3d_combine.h"
#include "us_license_t.h"
#include "us_license.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_math.h"
#include "us_matrix.h"

// main program
int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );

   #include "main1.inc"

   // License is OK.  Start up.
   
   US_Pseudo3D_Combine w;
   w.show();                   //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication
}

// qSort LessThan method for Solute sort
bool distro_lessthan( const Solute &solu1, const Solute &solu2 )
{  // TRUE iff  (s1<s2) || (s1==s2 && k1<k2)
   return ( solu1.s < solu2.s ) ||
          ( ( solu1.s == solu2.s ) && ( solu1.k < solu2.k ) );
}

// US_Pseudo3D_Combine class constructor
US_Pseudo3D_Combine::US_Pseudo3D_Combine() : US_Widgets()
{
   // set up the GUI

   setWindowTitle( tr( "Combine Pseudo-3D Distribution Overlays" ) );
   setPalette( US_GuiSettings::frameColor() );

   // primary layouts
   QHBoxLayout* main = new QHBoxLayout( this );
   QVBoxLayout* left = new QVBoxLayout();
   QGridLayout* spec = new QGridLayout();

   int s_row = 0;

   // top banner
   lb_info1      = us_banner( tr( "Pseudo-3D Plotting Controls" ) );
   spec->addWidget( lb_info1, s_row++, 0, 1, 2 );

   // series of rows: most of them label on left, counter/box on right
   lb_resolu     = us_label( tr( "Pseudo-3D Resolution:" ) );
   lb_resolu->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
   spec->addWidget( lb_resolu, s_row, 0 );

   ct_resolu     = us_counter( 3, 0.0, 100.0, 90.0 );
   ct_resolu->setStep( 1 );
   spec->addWidget( ct_resolu, s_row++, 1 );
   connect( ct_resolu, SIGNAL( valueChanged( double ) ),
            this,      SLOT( update_resolu( double ) ) );

   lb_xreso      = us_label( tr( "X Resolution:" ) );
   lb_xreso->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
   spec->addWidget( lb_xreso, s_row, 0 );

   ct_xreso      = us_counter( 3, 10.0, 1000.0, 0.0 );
   ct_xreso->setStep( 1 );
   spec->addWidget( ct_xreso, s_row++, 1 );
   connect( ct_xreso,  SIGNAL( valueChanged( double ) ),
            this,      SLOT( update_xreso( double ) ) );

   lb_yreso      = us_label( tr( "Y Resolution:" ) );
   lb_yreso->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
   spec->addWidget( lb_yreso, s_row, 0 );

   ct_yreso      = us_counter( 3, 10.0, 1000.0, 0.0 );
   ct_yreso->setStep( 1 );
   spec->addWidget( ct_yreso, s_row++, 1 );
   connect( ct_yreso,  SIGNAL( valueChanged( double ) ),
            this,      SLOT( update_yreso( double ) ) );

   lb_xpix       = us_label( tr( "X-pixel width:" ) );
   lb_xpix->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
   spec->addWidget( lb_xpix, s_row, 0 );

   ct_xpix       = us_counter( 3, 2.0, 50.0, 2.0 );
   ct_xpix->setStep( 1 );
   spec->addWidget( ct_xpix, s_row++, 1 );
   connect( ct_xpix,   SIGNAL( valueChanged( double ) ),
            this,      SLOT( update_xpix( double ) ) );

   lb_ypix       = us_label( tr( "Y-pixel width:" ) );
   lb_ypix->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
   spec->addWidget( lb_ypix, s_row, 0 );

   ct_ypix       = us_counter( 3, 2.0, 50.0, 2.0 );
   ct_ypix->setStep( 1 );
   spec->addWidget( ct_ypix, s_row++, 1 );
   connect( ct_ypix,   SIGNAL( valueChanged( double ) ),
            this,      SLOT( update_ypix( double ) ) );

   lb_autolim    = us_label( tr( "Automatic Plot Limits" ) );
   lb_autolim->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
   spec->addWidget( lb_autolim, s_row, 0 );

   us_checkbox( tr( "(unselect to override)" ), cb_autolim, true );
   spec->addWidget( cb_autolim, s_row++, 1 );
   connect( cb_autolim, SIGNAL( clicked() ),
            this,       SLOT( select_autolim() ) );

   lb_plt_fmin   = us_label( tr( "Plot Limit f/f0 min:" ) );
   lb_plt_fmin->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
   spec->addWidget( lb_plt_fmin, s_row, 0 );

   ct_plt_fmin   = us_counter( 3, 0.0, 50.0, 0.0 );
   ct_plt_fmin->setStep( 1 );
   spec->addWidget( ct_plt_fmin, s_row++, 1 );
   connect( ct_plt_fmin, SIGNAL( valueChanged( double ) ),
            this,        SLOT( update_plot_fmin( double ) ) );

   lb_plt_fmax   = us_label( tr( "Plot Limit f/f0 max:" ) );
   lb_plt_fmax->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
   spec->addWidget( lb_plt_fmax, s_row, 0 );

   ct_plt_fmax   = us_counter( 3, 1.0, 50.0, 1.0 );
   ct_plt_fmax->setStep( 1 );
   spec->addWidget( ct_plt_fmax, s_row++, 1 );
   connect( ct_plt_fmax, SIGNAL( valueChanged( double ) ),
            this,        SLOT( update_plot_fmax( double ) ) );

   lb_plt_smin   = us_label( tr( "Plot Limit s min:" ) );
   lb_plt_smin->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
   spec->addWidget( lb_plt_smin, s_row, 0 );

   ct_plt_smin   = us_counter( 3, 0.0, 10000.0, 0.0 );
   ct_plt_smin->setStep( 1 );
   spec->addWidget( ct_plt_smin, s_row++, 1 );
   connect( ct_plt_smin, SIGNAL( valueChanged( double ) ),
            this,        SLOT( update_plot_smin( double ) ) );

   lb_plt_smax   = us_label( tr( "Plot Limit s max:" ) );
   lb_plt_smax->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
   spec->addWidget( lb_plt_smax, s_row, 0 );

   ct_plt_smax   = us_counter( 3, 0.0, 10000.0, 0.0 );
   ct_plt_smax->setStep( 1 );
   spec->addWidget( ct_plt_smax, s_row++, 1 );
   connect( ct_plt_smax, SIGNAL( valueChanged( double ) ),
            this,        SLOT( update_plot_smax( double ) ) );

   lb_curr_distr = us_label( tr( "Current Distro:" ) );
   lb_curr_distr->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
   spec->addWidget( lb_curr_distr, s_row, 0 );

   ct_curr_distr = us_counter( 3, 0.0, 10.0, 0.0 );
   ct_curr_distr->setStep( 1 );
   spec->addWidget( ct_curr_distr, s_row++, 1 );
   connect( ct_curr_distr, SIGNAL( valueChanged( double ) ),
            this,          SLOT( update_curr_distr( double ) ) );

   le_distr_info = us_lineedit( tr( "Run xxx.1 (2DSA)" ) );
   le_distr_info->setReadOnly( true );
   spec->addWidget( le_distr_info, s_row++, 0, 1, 2 );

   us_checkbox( tr( "Plot f/f0 vs s" ), cb_plot_s, true );
   spec->addWidget( cb_plot_s, s_row, 0 );
   connect( cb_plot_s,  SIGNAL( clicked() ),
            this,       SLOT( select_plot_s() ) );

   us_checkbox( tr( "Plot f/f0 vs MW" ), cb_plot_mw, false );
   spec->addWidget( cb_plot_mw, s_row++, 1 );
   connect( cb_plot_mw, SIGNAL( clicked() ),
            this,       SLOT( select_plot_mw() ) );

   pb_pltall     = us_pushbutton( tr( "Plot all Distros" ) );
   pb_pltall->setEnabled( true );
   spec->addWidget( pb_pltall, s_row, 0 );
   connect( pb_pltall,  SIGNAL( clicked() ),
            this,       SLOT( plotall() ) );

   pb_stopplt    = us_pushbutton( tr( "Stop Plotting Loop" ) );
   pb_stopplt->setEnabled( false );
   spec->addWidget( pb_stopplt, s_row++, 1 );
   connect( pb_stopplt, SIGNAL( clicked() ),
            this,       SLOT( stop() ) );

   pb_refresh    = us_pushbutton( tr( "Refresh Pseudo-3D Plot" ) );
   pb_refresh->setEnabled( true );
   spec->addWidget( pb_refresh, s_row, 0 );
   connect( pb_refresh, SIGNAL( clicked() ),
            this,       SLOT( refresh() ) );

   pb_reset      = us_pushbutton( tr( "Reset" ) );
   pb_reset->setEnabled( true );
   spec->addWidget( pb_reset, s_row++, 1 );
   connect( pb_reset,   SIGNAL( clicked() ),
            this,       SLOT( reset() ) );

   pb_lddistr    = us_pushbutton( tr( "Load Distribution" ) );
   pb_lddistr->setEnabled( true );
   spec->addWidget( pb_lddistr, s_row, 0 );
   connect( pb_lddistr, SIGNAL( clicked() ),
            this,       SLOT( load_distro() ) );

   pb_ldcolor    = us_pushbutton( tr( "Load Color File" ) );
   pb_ldcolor->setEnabled( true );
   spec->addWidget( pb_ldcolor, s_row++, 1 );
   connect( pb_ldcolor, SIGNAL( clicked() ),
            this,       SLOT( load_color() ) );

   pb_print      = us_pushbutton( tr( "Print" ) );
   pb_print->setEnabled( true );
   spec->addWidget( pb_print, s_row, 0 );
   connect( pb_print,   SIGNAL( clicked() ),
            this,       SLOT( print() ) );

   pb_save       = us_pushbutton( tr( "Save" ) );
   pb_save->setEnabled( false );
   spec->addWidget( pb_save, s_row++, 1 );
   connect( pb_save,    SIGNAL( clicked() ),
            this,       SLOT( save() ) );

   pb_help       = us_pushbutton( tr( "Help" ) );
   pb_help->setEnabled( true );
   spec->addWidget( pb_help, s_row, 0 );
   connect( pb_help,    SIGNAL( clicked() ),
            this,       SLOT( help() ) );

   pb_close      = us_pushbutton( tr( "Close" ) );
   pb_close->setEnabled( true );
   spec->addWidget( pb_close, s_row++, 1 );
   connect( pb_close,   SIGNAL( clicked() ),
            this,       SLOT( close() ) );

   // set up progress bar
   progress      = us_progressBar( 0, 100, 0 );
   progress->reset();
   progress->setVisible( true );
   spec->addWidget( progress, s_row++, 0, 1, 2 );

   // set up plot component window on right side
   QString degrsymb( "°" );
   xa_title_s  = tr( "Sedimentation Coefficient corrected for water at 20" );
   if ( degrsymb.length() == 1 )
      xa_title_s = xa_title_s + degrsymb + "C";
   else
      xa_title_s = xa_title_s.append( degrsymb.at( 1 ) ) + "C";
   xa_title_mw = tr( "Molecular Weight (Dalton)" );
   xa_title    = xa_title_s;

   QBoxLayout* plot = new US_Plot( data_plot, 
      tr( "Pseudo-3D Distribution Data" ),
      xa_title,
      tr( "Frictional Ratio f/f0" ) );

   data_plot->setMinimumSize( 600, 600 );

   data_plot->enableAxis( QwtPlot::xBottom, true );
   data_plot->enableAxis( QwtPlot::yLeft,   true );

   pick = new US_PlotPicker( data_plot );
   pick->setRubberBand( QwtPicker::VLineRubberBand );

   // put layouts together for overall layout
   left->addLayout( spec );
   left->addStretch();
   plot->addStretch();

   main->addLayout( left );
   main->addLayout( plot );

   // set up variables and initial state of GUI

   reset();
}

void US_Pseudo3D_Combine::reset( void )
{
   data_plot->clear();
   data_plot->replot();
 
   minmax     = false;
   zoom       = false;
   plot_s     = true;
   cb_plot_s->setChecked( plot_s );  
   cb_plot_mw->setChecked( !plot_s );

   resolu     = 90.0;
   ct_resolu->setRange( 1, 100, 1 );
   ct_resolu->setValue( resolu );  

   xreso      = 300;
   yreso      = 300;
   ct_xreso->setRange( 10.0, 1000.0, 1.0 );
   ct_xreso->setValue( (double)xreso );
   ct_yreso->setRange( 10, 1000, 1 );
   ct_yreso->setValue( (double)yreso );

   xpix       = 2;
   ypix       = 2;
   ct_xpix->setRange( 2, 50, 1 );
   ct_xpix->setValue( (double)xpix );
   ct_ypix->setRange( 2, 50, 1 );
   ct_ypix->setValue( (double)ypix );

   auto_lim   = true;
   cb_autolim->setChecked( auto_lim );

   plt_fmin   = 1.0;
   plt_fmax   = 4.0;
   ct_plt_fmin->setRange( 0, 50, 0.01 );
   ct_plt_fmin->setValue( plt_fmin );
   ct_plt_fmin->setEnabled( false );
   ct_plt_fmax->setRange( 1, 50, 0.01 );
   ct_plt_fmax->setValue( plt_fmax );
   ct_plt_fmax->setEnabled( false );

   plt_smin   = 1.0;
   plt_smax   = 10.0;
   ct_plt_smin->setRange( -10.0, 10000.0, 0.01 );
   ct_plt_smin->setValue( plt_smin );
   ct_plt_smin->setEnabled( false );
   ct_plt_smax->setRange( 0.0, 10000.0, 0.01 );
   ct_plt_smax->setValue( plt_smax );
   ct_plt_smax->setEnabled( false );

   curr_distr = 0;
   ct_curr_distr->setRange( 1.0, 1.0, 1.0 );
   ct_curr_distr->setValue( curr_distr );
   ct_curr_distr->setEnabled( false );
}

void US_Pseudo3D_Combine::plot_data( void )
{
   data_plot->clear();

   QStringList parsed;

   int     count = 0;

   double  minx = 1e20;
   double  maxx = 0.0;

   double  miny = 1e20;
   double  maxy = 0.0;

   // Remove any non-data lines and put values in arrays

   double overscan = ( maxx - minx ) * 0.10;  // 10% overscan

   data_plot->setAxisScale( QwtPlot::xBottom, 
         minx - overscan, maxx + overscan );
    
   // Adjust y axis to scale all the data
   double dy = fabs( maxy - miny ) / 10.0;

   data_plot->setAxisScale( QwtPlot::yLeft, miny - dy, maxy + dy );

   // Do the fit and get the minimum

   double c[ 10 ];

   int order = 1;

   int fit_count = (int) ( ( maxx - minx + 2 * overscan ) / 0.001 );

   double* fit_x = new double[ fit_count ];
   double* fit_y = new double[ fit_count ];
   double  x     = minx - overscan;
   double minimum;

   for ( int i = 0; i < fit_count; i++, x += 0.001 )
   {
      fit_x[ i ] = x;
      fit_y[ i ] = c[ 0 ];

      for ( int j = 1; j <= order; j++ ) 
         fit_y[ i ] += c[ j ] * pow( x, j );
   }

   // Calculate Root Mean Square Error
   double rms_err = 0.0;

   for ( int i = 0; i < count; i++ )
   {
      double x = 1.0;
      double y = 1.0;

      double y_calc = c[ 0 ];
      
      for ( int j = 1; j <= order; j++ )  
         y_calc += c[ j ] * pow( x, j );
      
      rms_err += sq ( fabs ( y_calc - y ) );
   }

   // Find the minimum
   if ( order == 2 )
   {
      // Take the derivitive and get the minimum
      // c1 + 2 * c2 * x = 0
      minimum = - c[ 1 ] / ( 2.0 * c[ 2 ] );
   }
   else
   {
      // Find the zero of the derivitive
      double dxdy  [ 9 ];
      double d2xdy2[ 8 ];

      // First take the derivitive
      for ( int i = 0; i < order; i++ ) 
         dxdy[ i ] = c[ i + 1 ] * ( i + 1 );

      // And we'll need the 2nd derivitive
      for ( int i = 0; i < order - 1; i++ ) 
         d2xdy2[ i ] = dxdy[ i + 1 ] * ( i + 1 );

      // We'll do a quadratic fit for the initial estimate
      double q[ 3 ];
      minimum = - q[ 1 ] / ( 2.0 * q[ 2 ] );

      const double epsilon = 1.0e-4;

      int    k = 0;
      double f;
      double f_prime;
      do
      {
        // f is the 1st derivitive
        f = dxdy[ 0 ];
        for ( int i = 1; i < order; i++ ) f += dxdy[ i ] * pow( minimum, i );

        // f_prime is the 2nd derivitive
        f_prime = d2xdy2[ 0 ];
        for ( int i = 1; i < order - 1; i++ ) 
           f_prime += d2xdy2[ i ] * pow( minimum, i );

        if ( fabs( f ) < epsilon ) break;
        if ( k++ > 10 ) break;

        // Get the next estimate
        minimum -= f / f_prime;

      } while ( true );
   }

   // Plot the minimum

   double radius_min[ 2 ];
   double rmsd_min  [ 2 ];

   radius_min[ 0 ] = minimum;
   radius_min[ 1 ] = minimum;

   rmsd_min  [ 0 ] = miny - 1.0 * dy;
   rmsd_min  [ 0 ] = miny + 2.0 * dy;

   // Add the marker label -- bold, font size default + 1, lines 3 pixels wide
   QPen markerPen( QBrush( Qt::white ), 3.0 );
   markerPen.setWidth( 3 );
   
   QwtPlotMarker* pm = new QwtPlotMarker();
   QwtText        label( QString::number( minimum, 'f', 5 ) );
   QFont          font( pm->label().font() );

   font.setBold( true );
   font.setPointSize( font.pointSize() + 1 );
   label.setFont( font );

   pm->setValue( minimum, miny + 3.0 * dy );
   pm->setSymbol( QwtSymbol( QwtSymbol::Cross, 
            QBrush( Qt::white ), markerPen, QSize( 9, 9 ) ) );
   pm->setLabel( label );
   pm->setLabelAlignment( Qt::AlignTop );

   pm->attach( data_plot );

   data_plot->replot();
}

void US_Pseudo3D_Combine::plot_data( int )
{
   plot_data();
}

void US_Pseudo3D_Combine::update_resolu( double dval )
{
   resolu = dval;
}

void US_Pseudo3D_Combine::update_xreso( double dval )
{
   xreso  = qRound( dval );
}

void US_Pseudo3D_Combine::update_yreso( double dval )
{
   yreso  = qRound( dval );
}

void US_Pseudo3D_Combine::update_xpix( double dval )
{
   xpix  = qRound( dval );
}

void US_Pseudo3D_Combine::update_ypix( double dval )
{
   ypix  = qRound( dval );
}

void US_Pseudo3D_Combine::update_curr_distr( double dval )
{
   curr_distr = dval;
}

void US_Pseudo3D_Combine::update_plot_smin( double dval )
{
   plt_smin = dval;
}

void US_Pseudo3D_Combine::update_plot_smax( double dval )
{
   plt_smax = dval;
}

void US_Pseudo3D_Combine::update_plot_fmin( double dval )
{
   plt_fmin = dval;
}

void US_Pseudo3D_Combine::update_plot_fmax( double dval )
{
   plt_fmax = dval;
}

void US_Pseudo3D_Combine::plot_3dim()
{}

void US_Pseudo3D_Combine::loop()
{}

void US_Pseudo3D_Combine::select_autolim()
{
   auto_lim   = cb_autolim->isChecked();
   ct_plt_fmin->setEnabled( !auto_lim );
   ct_plt_fmax->setEnabled( !auto_lim );
   ct_plt_smin->setEnabled( !auto_lim );
   ct_plt_smax->setEnabled( !auto_lim );
   ct_curr_distr->setEnabled( !auto_lim );
}

void US_Pseudo3D_Combine::select_plot_s()
{
   plot_s     = cb_plot_s->isChecked();
   cb_plot_mw->setChecked( !plot_s );
   xa_title   = plot_s ? xa_title_s : xa_title_mw;
   data_plot->setAxisTitle( QwtPlot::xBottom, xa_title );
}

void US_Pseudo3D_Combine::select_plot_mw()
{
   plot_s     = !cb_plot_mw->isChecked();
   cb_plot_s->setChecked( plot_s );
   xa_title   = plot_s ? xa_title_s : xa_title_mw;
   data_plot->setAxisTitle( QwtPlot::xBottom, xa_title );
}

void US_Pseudo3D_Combine::load_distro()
{
   QString filter = tr( "Any Distro files (*.*dis.*);;" )
      + tr( "FE files (*.fe_dis.*);;" )
      + tr( "COFS files (*.cofs_dis.*);;" )
      + tr( "2DSA files (*.sa2d_dis.*);;" )
      + tr( "2DSA-MW files (*.sa2d_mw_dis.*);;" )
      + tr( "GA files (*.ga_dis.*);;" )
      + tr( "GA-MW files (*.ga_mw_dis.*);;" )
      + tr( "GA-MW-MC files (*.ga_mw_mc_dis.*);;" )
      + tr( "2DSA-MC files (*.sa2d_mc_dis.*);;" )
      + tr( "2DSA-MW-MC files (*.sa2d_mw_mc_dis.*);;" )
      + tr( "Global files (*.global_dis.*);;" )
      + tr( "Global-MC files (*.global_mc_dis.*);;" )
      + tr( "Any files (*)" );

   // get a file name or set of names for distribution data
   QStringList list = QFileDialog::getOpenFileNames( this,
      tr( "Load Distribution Files(s)" ),
      US_Settings::resultDir(),
      filter,
      0, 0 );

   if ( !list.empty() )
   {
      // load a distribution for each named file

      for ( QStringList::iterator it = list.begin(); it != list.end(); it++ )
      {
         load_distro( *it );
      }
   }
}
void US_Pseudo3D_Combine::load_distro( const QString& fname )
{
   // file type table:  FilePartialName, Method, MonteCarlo
   const char* cdtyp[] =
   {
      "cofs_dis",        "C(s)",                               "F",
      "fe_dis",          "FE",                                 "F",
      "sa2d_dis",        "2DSA",                               "F",
      "ga_mc_dis",       "GA-MC",                              "T",
      "sa2d_mc_dis",     "2DSA-MC",                            "T",
      "ga_dis",          "GA",                                 "F",
      "global_dis",      "Global",                             "F",
      "sa2d_mw_dis",     "2DSA, MW Constrained",               "F",
      "ga_mw_dis",       "GA, MW Constrained",                 "F",
      "sa2d_mw_mc_dis",  "2DSA, MW Constrained, Monte Carlo",  "T",
      "ga_mw_mc_dis",    "GA, MW Constrained, MonteCarlo",     "T",
      "global_dis",      "Global Distro",                      "T",
      "global_mc_dis",   "Global MC Distro",                   "T"
   };
   int ncdte = sizeof( cdtyp ) / sizeof( char* );

   DisSys    tsys;
   Solute    sol_s;
   Solute    sol_mw;

   // load current gradient
   tsys.gradient.clear();
   for ( int jj = 0; jj < gradient.size(); jj++ )
   {
      tsys.gradient.append( gradient.at( jj ) );
   }

   // set values based on file name
qDebug() << "Load Distro, fname=" << fname;
   QFileInfo fi( fname );
   int jj           = fname.lastIndexOf( "." );
   int kk           = fname.length();
   QString tstr     = fname.right( kk - jj - 1 );

   tsys.cell        = tstr.left( 1 );
   tstr             = fname.right( kk - jj - 2 );
   tsys.wavelength  = tstr;
   tsys.run_name    = fi.baseName();
   tsys.distro_type = 0;

   // find type in table and set values accordingly
   for ( jj = 0; jj < ncdte; jj += 3 )
   {
      QString fnp( cdtyp[ jj ] );

      if ( fname.contains( fnp, Qt::CaseInsensitive ) )
      {
         tsys.distro_type = jj / 3 + 1;
         tsys.monte_carlo = QString( cdtyp[ jj+2 ] ).contains( "T" );
         tsys.method      = QString( cdtyp[ jj+1 ] );
         break;
      }
   }

   tstr             = "Run " + tsys.run_name + "." +
      tsys.cell + tsys.wavelength + " (" + tsys.method + ")";
   le_distr_info->setText( tstr );

   // read in and set distribution s,c,k values
   if ( tsys.distro_type > 0 )
   {
      QFile filei( fname );

      if ( filei.open( QIODevice::ReadOnly | QIODevice::Text ) )
      {
         QTextStream ts( &filei );
         QString s1;
         QStringList l1;

         if ( !ts.atEnd() )
         {
            ts.readLine();    // discard header line
         }

         if ( tsys.monte_carlo )
         {  // GA Monte Carlo:  we need the number of MC iterations
            s1       = ts.readLine();    // consume entire first line
            l1       = s1.split( QRegExp( "\\s+" ) );
            mc_iters = l1.at( 0 ).toInt();
         }

         while ( !ts.atEnd() )
         {
            double dv1;
            double dv2;
            double dv3;
            double dv4;
            s1       = ts.readLine();    // consume entire line
            l1       = s1.split( QRegExp( "\\s+" ) );
            if ( l1.empty()  ||  l1.size() < 7 )
            {
               qDebug() << "BLANK LINE: size=" << l1.size();
               continue;      // skip this line
            }
            dv1      = l1.at( 1 ).toDouble();   // s_20,W
            dv2      = l1.at( 4 ).toDouble();   // D_20,W
            dv3      = l1.at( 5 ).toDouble();   // Frequency
            dv4      = l1.at( 6 ).toDouble();   // f/f0

            if ( dv1 == 0.0 )
               break;

            dv1     *= 1.0e13;   // s_20,W properly scaled
            sol_s.s  = dv1;
            sol_s.c  = dv3;
            sol_s.k  = dv4;
            sol_mw.s = dv2;
            sol_mw.c = dv3;
            sol_mw.k = dv4;

            tsys.s_distro.append( sol_s );
            tsys.mw_distro.append( sol_mw );
         }
         filei.close();
      }
qDebug() << "  s_distro size=" << tsys.s_distro.size();

      // sort and reduce distributions
      sort_distro( tsys.s_distro, true );
      sort_distro( tsys.mw_distro, true );
   }

   // update current distribution record
   system.append( tsys );
   int jd     = system.size();
   curr_distr = jd - 1;
   ct_curr_distr->setRange( 1, jd, 1 );
   ct_curr_distr->setValue( jd );
   ct_curr_distr->setEnabled( true );

   if ( auto_lim )
   {
      set_limits();

      ct_plt_fmin->setEnabled( false );
      ct_plt_fmax->setEnabled( false );
      ct_plt_smin->setEnabled( false );
      ct_plt_smax->setEnabled( false );
   }

   pb_print->setEnabled(    true );
   pb_pltall->setEnabled(   true );
   pb_refresh->setEnabled(  true );
   pb_reset->setEnabled(    true );
   cb_plot_s->setEnabled(   true );
   cb_plot_mw->setEnabled(  true );

}

void US_Pseudo3D_Combine::load_color()
{
   QString filter = tr( "Any XML files (*.xml);;" )
         + tr( "Gradient files (*grad*.xml);;" )
         + tr( "Any files (*)" );

   // get an xml file name for the color gradient
   QString fname = QFileDialog::getOpenFileName( this,
      tr( "Load Color Gradient File" ),
      US_Settings::appBaseDir() + "/etc",
      filter,
      0, 0 );

   if ( fname.isEmpty() )
      return;

   // get the gradient from the file
   US_ColorGradIO::read_color_gradient( fname, gradient );

   // save the gradient information for the current distribution
   if ( system.size() > curr_distr )
   {
      DisSys tsys  = system.at( curr_distr );
      tsys.gradient.clear();
      tsys.gradient.append( gradient );
      system.replace( curr_distr, tsys );
   }
}
void US_Pseudo3D_Combine::save()
{}
void US_Pseudo3D_Combine::plotall()
{}
void US_Pseudo3D_Combine::refresh()
{}
void US_Pseudo3D_Combine::stop()
{}
void US_Pseudo3D_Combine::print()
{}

void US_Pseudo3D_Combine::set_limits()
{
   double fmin = 1.0e30;
   double fmax = -1.0e30;
   double smin = 1.0e30;
   double smax = -1.0e30;
   double rdif;
   int    ii;
   int    jj;

   if ( plot_s )
   {
      data_plot->setAxisTitle( QwtPlot::xBottom, xa_title_s );

      // find min,max for S distributions
      for ( ii = 0; ii < system.size(); ii++ )
      {
         DisSys tsys  = system.at( ii );
         for ( jj = 0; jj < tsys.s_distro.size(); jj++ )
         {
            double sval = tsys.s_distro.at( jj ).s;
            double fval = tsys.s_distro.at( jj ).k;
            smin        = ( smin < sval ) ? smin : sval;
            smax        = ( smax > sval ) ? smax : sval;
            fmin        = ( fmin < fval ) ? fmin : fval;
            fmax        = ( fmax > fval ) ? fmax : fval;
         }
      }
   }
   else
   {
      data_plot->setAxisTitle( QwtPlot::xBottom, xa_title_mw );

      // find min,max for MW distributions
      for ( ii = 0; ii < system.size(); ii++ )
      {
         DisSys tsys  = system.at( ii );
         for ( jj = 0; jj < tsys.mw_distro.size(); jj++ )
         {
            double sval = tsys.mw_distro.at( jj ).s;
            double fval = tsys.mw_distro.at( jj ).k;
            smin        = ( smin < sval ) ? smin : sval;
            smax        = ( smax > sval ) ? smax : sval;
            fmin        = ( fmin < fval ) ? fmin : fval;
            fmax        = ( fmax > fval ) ? fmax : fval;
         }
      }
   }

   // adjust minima, maxima
   rdif      = ( smax - smin ) / 10.0;
   smin     -= rdif;
   smax     += rdif;
   smin      = ( smin < 0.0 ) ? 0.0 : smin;
   rdif      = ( fmax - fmin ) / 10.0;
   fmin     -= rdif;
   fmax     += rdif;

   if ( auto_lim )
   {
      // set auto limits
      double rmin = smax * 10.0;
      double rinc = pow( 1.0, (double)( (int)( log10( rmin - 3.0 ) ) ) );

      ct_plt_smax->setRange( 0.0, rmin, rinc );
      ct_plt_smin->setRange( -( smax / 50.0 ), rmin, rinc );

      smax       += ( ( smax - smin ) / 20.0 );
      smin       -= ( ( smax - smin ) / 20.0 );
      fmax       += ( ( fmax - fmin ) / 20.0 );
      fmin       -= ( ( fmax - fmin ) / 20.0 );
      fmin        = ( fmin < 0.0 ) ? 0.0 : fmin;

      if ( ( fmax - fmin ) < 1.0e-3 )
         fmax       += ( fmax / 10.0 );

      if ( ( smax - smin ) < 1.0e-100 )
      {
         smin       -= ( smin / 30.0 );
         smax       += ( smax / 30.0 );
      }

      ct_plt_smin->setValue( smin );
      ct_plt_smax->setValue( smax );
      ct_plt_fmin->setValue( fmin );
      ct_plt_fmax->setValue( fmax );

      plt_smin    = smin;
      plt_smax    = smax;
      plt_fmin    = fmin;
      plt_fmax    = fmax;
   }
}

// Sort distribution solute list by s,k values and optionally reduce
void US_Pseudo3D_Combine::sort_distro( QList< Solute >& listsols,
      bool reduce )
{
   int sizi = listsols.size();

   if ( sizi < 2 )
      return;        // nothing need be done for 1-element list

   // sort distro solute list by s,k values

   qSort( listsols.begin(), listsols.end(), distro_lessthan );

   // check reduce flag

   if ( reduce )
   {     // skip any duplicates in sorted list
      Solute sol1;
      Solute sol2;
      QList< Solute > reduced;
      QList< Solute >::iterator jj = listsols.begin();
      sol1     = *jj;
      reduced.append( *jj );     // output first entry

      while ( (++jj) != listsols.end() )
      {     // loop to compare each entry to previous
          sol2    = *jj;         // solute entry

          if ( ( sol2.s != sol1.s ) || ( sol2.k != sol1.k ) )
          {   // not a duplicate, so output to temporary list
             reduced.append( sol2 );
          }

          sol1    = sol2;        // save entry for next iteration
      }
qDebug() << "  SD:   reduced size=" << reduced.size();

      if ( reduced.size() < sizi )
      {   // if some reduction happened, replace list with reduced version
         listsols = reduced;
      }
   }
   return;
}


