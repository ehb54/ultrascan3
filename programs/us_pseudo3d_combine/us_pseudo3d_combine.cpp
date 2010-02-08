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

int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );

   #include "main1.inc"

   // License is OK.  Start up.
   
   US_Pseudo3D_Combine w;
   w.show();                   //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication
}

US_Pseudo3D_Combine::US_Pseudo3D_Combine() : US_Widgets()
{
   // set up the GUI

   setWindowTitle( tr( "Combine Pseudo-3D Distribution Overlays" ) );
   setPalette( US_GuiSettings::frameColor() );

   QHBoxLayout* main = new QHBoxLayout( this );
   QVBoxLayout* left = new QVBoxLayout();
   QGridLayout* spec = new QGridLayout();

   int s_row = 0;

   lb_info1      = us_banner( tr( "Pseudo-3D Plotting Controls" ) );
   spec->addWidget( lb_info1, s_row++, 0, 1, 2 );

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

   lb_plt_fmin   = us_label( tr( "Plot Limit f/f0 min:" ) );
   lb_plt_fmin->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
   spec->addWidget( lb_plt_fmin, s_row, 0 );

   ct_plt_fmin   = us_counter( 3, 0.0, 50.0, 0.0 );
   ct_plt_fmin->setStep( 1 );
   spec->addWidget( ct_plt_fmin, s_row++, 1 );

   lb_plt_fmax   = us_label( tr( "Plot Limit f/f0 max:" ) );
   lb_plt_fmax->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
   spec->addWidget( lb_plt_fmax, s_row, 0 );

   ct_plt_fmax   = us_counter( 3, 1.0, 50.0, 1.0 );
   ct_plt_fmax->setStep( 1 );
   spec->addWidget( ct_plt_fmax, s_row++, 1 );

   lb_plt_smin   = us_label( tr( "Plot Limit s min:" ) );
   lb_plt_smin->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
   spec->addWidget( lb_plt_smin, s_row, 0 );

   ct_plt_smin   = us_counter( 3, 0.0, 10000.0, 0.0 );
   ct_plt_smin->setStep( 1 );
   spec->addWidget( ct_plt_smin, s_row++, 1 );

   lb_plt_smax   = us_label( tr( "Plot Limit s max:" ) );
   lb_plt_smax->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
   spec->addWidget( lb_plt_smax, s_row, 0 );

   ct_plt_smax   = us_counter( 3, 0.0, 10000.0, 0.0 );
   ct_plt_smax->setStep( 1 );
   spec->addWidget( ct_plt_smax, s_row++, 1 );

   lb_curr_distr = us_label( tr( "Current Distro:" ) );
   lb_curr_distr->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
   spec->addWidget( lb_curr_distr, s_row, 0 );

   ct_curr_distr = us_counter( 3, 0.0, 10.0, 0.0 );
   ct_curr_distr->setStep( 1 );
   spec->addWidget( ct_curr_distr, s_row++, 1 );

   le_distr_info = us_lineedit( tr( "Run xxx.1 (2DSA)" ) );
   le_distr_info->setReadOnly( true );
   spec->addWidget( le_distr_info, s_row++, 0, 1, 2 );

   us_checkbox( tr( "Plot f/f0 vs s" ), cb_plot_s, true );
   spec->addWidget( cb_plot_s, s_row, 0 );

   us_checkbox( tr( "Plot f/f0 vs MW" ), cb_plot_mw, false );
   spec->addWidget( cb_plot_mw, s_row++, 1 );

   pb_pltall     = us_pushbutton( tr( "Plot all Distros" ) );
   pb_pltall->setEnabled( true );
   spec->addWidget( pb_pltall, s_row, 0 );

   pb_stopplt    = us_pushbutton( tr( "Stop Plotting Loop" ) );
   pb_stopplt->setEnabled( false );
   spec->addWidget( pb_stopplt, s_row++, 1 );

   pb_refresh    = us_pushbutton( tr( "Refresh Pseudo-3D Plot" ) );
   pb_refresh->setEnabled( true );
   spec->addWidget( pb_refresh, s_row, 0 );

   pb_reset      = us_pushbutton( tr( "Reset" ) );
   pb_reset->setEnabled( true );
   spec->addWidget( pb_reset, s_row++, 1 );

   pb_lddistr    = us_pushbutton( tr( "Load Distribution" ) );
   pb_lddistr->setEnabled( true );
   spec->addWidget( pb_lddistr, s_row, 0 );

   pb_ldcolor    = us_pushbutton( tr( "Load Color File" ) );
   pb_ldcolor->setEnabled( true );
   spec->addWidget( pb_ldcolor, s_row++, 1 );

   pb_print      = us_pushbutton( tr( "Print" ) );
   pb_print->setEnabled( true );
   spec->addWidget( pb_print, s_row, 0 );

   pb_save       = us_pushbutton( tr( "Save" ) );
   pb_save->setEnabled( false );
   spec->addWidget( pb_save, s_row++, 1 );

   pb_help       = us_pushbutton( tr( "Help" ) );
   pb_help->setEnabled( true );
   spec->addWidget( pb_help, s_row, 0 );

   pb_close      = us_pushbutton( tr( "Close" ) );
   pb_close->setEnabled( true );
   spec->addWidget( pb_close, s_row++, 1 );

   progress      = us_progressBar( 0, 100, 0 );
   progress->reset();
   progress->setVisible( true );
   spec->addWidget( progress, s_row++, 0, 1, 2 );
 
   QString degrsign( "°" );
   QString xaxistitle =
      tr( "Sedimentation Coefficient corrected for water at 20" );
   if ( degrsign.length() == 2 )
      xaxistitle = xaxistitle.append( degrsign.at( 1 ) ) + "C";
   else
      xaxistitle = xaxistitle + degrsign + "C";

   QBoxLayout* plot = new US_Plot( data_plot, 
      tr( "Pseudo-3D Distribution Data" ),
      xaxistitle,
      tr( "Frictional Ratio f/f0" ) );

   data_plot->setMinimumSize( 600, 600 );

   data_plot->enableAxis( QwtPlot::xBottom, true );
   data_plot->enableAxis( QwtPlot::yLeft,   true );

   pick = new US_PlotPicker( data_plot );
   pick->setRubberBand( QwtPicker::VLineRubberBand );

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
   ct_plt_smin->setRange( 0.0, 10000.0, 0.01 );
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
{}
void US_Pseudo3D_Combine::select_plot_s()
{}
void US_Pseudo3D_Combine::select_plot_mw()
{}
void US_Pseudo3D_Combine::load_distro()
{}
void US_Pseudo3D_Combine::load_distro( const QString& fname )
{
   QString filename = fname;
}
void US_Pseudo3D_Combine::load_color()
{}
void US_Pseudo3D_Combine::save()
{}
void US_Pseudo3D_Combine::stop()
{}
void US_Pseudo3D_Combine::print()
{}
void US_Pseudo3D_Combine::set_limits()
{}
void US_Pseudo3D_Combine::setup_GUI()
{}
