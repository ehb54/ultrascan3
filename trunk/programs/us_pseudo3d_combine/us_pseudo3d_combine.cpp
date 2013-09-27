//! \file us_pseudo3d_combine.cpp

#include <QApplication>

#include "us_pseudo3d_combine.h"
#include "us_spectrodata.h"
#include "us_remove_distros.h"
#include "us_select_edits.h"
#include "us_model.h"
#include "us_license_t.h"
#include "us_license.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_math2.h"
#include "us_matrix.h"
#include "us_sleep.h"
#include "us_passwd.h"
#include "us_report.h"
#include "us_constants.h"

#define PA_TMDIS_MS 2000  // default Plotall time per distro in milliseconds

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
   main->setSpacing        ( 2 );
   main->setContentsMargins( 2, 2, 2, 2 );
   left->setSpacing        ( 0 );
   left->setContentsMargins( 0, 1, 0, 1 );
   spec->setSpacing        ( 1 );
   spec->setContentsMargins( 0, 0, 0, 0 );

   int s_row = 0;
   dbg_level = US_Settings::us_debug();
   clean_etc_dir();

   // Top banner
   QLabel* lb_info1      = us_banner( tr( "Pseudo-3D Plotting Controls" ) );

   // Series of rows: most of them label on left, counter/box on right
   QLabel* lb_resolu     = us_label( tr( "Pseudo-3D Resolution:" ) );
   lb_resolu->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );

   ct_resolu     = us_counter( 3, 0.0, 100.0, 90.0 );
   ct_resolu->setStep( 1 );
   connect( ct_resolu, SIGNAL( valueChanged( double ) ),
            this,      SLOT( update_resolu( double ) ) );

   QLabel* lb_xreso      = us_label( tr( "X Resolution:" ) );
   lb_xreso->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );

   ct_xreso      = us_counter( 3, 10.0, 1000.0, 0.0 );
   ct_xreso->setStep( 1 );
   connect( ct_xreso,  SIGNAL( valueChanged( double ) ),
            this,      SLOT( update_xreso( double ) ) );

   QLabel* lb_yreso      = us_label( tr( "Y Resolution:" ) );
   lb_yreso->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );

   ct_yreso      = us_counter( 3, 10.0, 1000.0, 0.0 );
   ct_yreso->setStep( 1 );
   connect( ct_yreso,  SIGNAL( valueChanged( double ) ),
            this,      SLOT( update_yreso( double ) ) );

   QLabel* lb_zfloor     = us_label( tr( "Z Floor Percent:" ) );
   lb_zfloor->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );

   ct_zfloor     = us_counter( 3, 0.0, 50.0, 1.0 );
   ct_zfloor->setStep( 1 );
   connect( ct_zfloor, SIGNAL( valueChanged( double ) ),
            this,      SLOT( update_zfloor( double ) ) );

   us_checkbox( tr( "Autoscale X and Y" ), cb_autosxy, true );
   connect( cb_autosxy, SIGNAL( clicked() ),
            this,       SLOT( select_autosxy() ) );

   us_checkbox( tr( "Autoscale Z" ), cb_autoscz, true );
   connect( cb_autoscz, SIGNAL( clicked() ),
            this,       SLOT( select_autoscz() ) );

   us_checkbox( tr( "Continuous Loop" ), cb_conloop, true );
   connect( cb_conloop, SIGNAL( clicked() ),
            this,       SLOT( select_conloop() ) );

   us_checkbox( tr( "Z as Percentage" ), cb_zpcent, false );

   lb_plt_fmin   = us_label( tr( "Plot Limit f/f0 Minimum:" ) );
   lb_plt_fmin->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );

   ct_plt_fmin   = us_counter( 3, 1.0, 50.0, 0.0 );
   ct_plt_fmin->setStep( 1 );
   connect( ct_plt_fmin, SIGNAL( valueChanged( double ) ),
            this,        SLOT( update_plot_fmin( double ) ) );

   lb_plt_fmax   = us_label( tr( "Plot Limit f/f0 Maximum:" ) );
   lb_plt_fmax->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );

   ct_plt_fmax   = us_counter( 3, 1.0, 50.0, 1.0 );
   ct_plt_fmax->setStep( 1 );
   connect( ct_plt_fmax, SIGNAL( valueChanged( double ) ),
            this,        SLOT( update_plot_fmax( double ) ) );

   lb_plt_smin   = us_label( tr( "Plot Limit s Minimum:" ) );
   lb_plt_smin->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );

   ct_plt_smin   = us_counter( 3, -10.0, 10000.0, 0.0 );
   ct_plt_smin->setStep( 1 );
   connect( ct_plt_smin, SIGNAL( valueChanged( double ) ),
            this,        SLOT( update_plot_smin( double ) ) );

   lb_plt_smax   = us_label( tr( "Plot Limit s Maximum:" ) );
   lb_plt_smax->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );

   ct_plt_smax   = us_counter( 3, 0.0, 10000.0, 0.0 );
   ct_plt_smax->setStep( 1 );
   connect( ct_plt_smax, SIGNAL( valueChanged( double ) ),
            this,        SLOT( update_plot_smax( double ) ) );

   QLabel* lb_plt_dlay   = us_label( tr( "Plot Loop Delay Seconds:" ) );
   lb_plt_dlay->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );

   ct_plt_dlay   = us_counter( 3, 0.1, 30.0, 0.0 );
   ct_plt_dlay->setStep( 0.1 );
   QSettings settings( "UltraScan3", "UltraScan" );
   patm_dlay     = settings.value( "slideDelay", PA_TMDIS_MS ).toInt();
   ct_plt_dlay->setValue( (double)( patm_dlay ) / 1000.0 );

   QLabel* lb_curr_distr = us_label( tr( "Current Distro:" ) );
   lb_curr_distr->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );

   ct_curr_distr = us_counter( 3, 0.0, 10.0, 0.0 );
   ct_curr_distr->setStep( 1 );
   connect( ct_curr_distr, SIGNAL( valueChanged( double ) ),
            this,          SLOT( update_curr_distr( double ) ) );

   te_distr_info = us_textedit();
   te_distr_info->setText    ( tr( "Run:  runID.triple (method)\n" )
            + tr( "    analysisID" ) );
   us_setReadOnly( te_distr_info, true );

   le_cmap_name  = us_lineedit(
         tr( "Default Color Map: w-cyan-magenta-red-black" ), -1, true );
   te_distr_info->setMaximumHeight( le_cmap_name->height() * 2 );

   us_checkbox( tr( "Plot f/f0 vs s" ),  cb_plot_sk, true );
   connect( cb_plot_sk, SIGNAL( clicked() ),
            this,       SLOT( select_plot_sk() ) );

   us_checkbox( tr( "Plot f/f0 vs MW" ), cb_plot_wk, false );
   connect( cb_plot_wk, SIGNAL( clicked() ),
            this,       SLOT( select_plot_wk() ) );

   us_checkbox( tr( "Plot vbar vs s" ),  cb_plot_sv, false );
   connect( cb_plot_sv, SIGNAL( clicked() ),
            this,       SLOT( select_plot_sv() ) );

   us_checkbox( tr( "Plot vbar vs MW" ), cb_plot_wv, false );
   connect( cb_plot_wv, SIGNAL( clicked() ),
            this,       SLOT( select_plot_wv() ) );

   pb_pltall     = us_pushbutton( tr( "Plot All Distros" ) );
   pb_pltall->setEnabled( false );
   connect( pb_pltall,  SIGNAL( clicked() ),
            this,       SLOT( plotall() ) );

   pb_stopplt    = us_pushbutton( tr( "Stop Plotting Loop" ) );
   pb_stopplt->setEnabled( false );
   connect( pb_stopplt, SIGNAL( clicked() ),
            this,       SLOT( stop() ) );

   pb_refresh    = us_pushbutton( tr( "Refresh Pseudo-3D Plot" ) );
   pb_refresh->setEnabled(  false );
   connect( pb_refresh, SIGNAL( clicked() ),
            this,       SLOT( plot_data() ) );

   pb_reset      = us_pushbutton( tr( "Reset" ) );
   pb_reset->setEnabled( true );
   connect( pb_reset,   SIGNAL( clicked() ),
            this,       SLOT( reset() ) );

   dkdb_cntrls   = new US_Disk_DB_Controls(
         US_Settings::default_data_location() );
   connect( dkdb_cntrls, SIGNAL( changed( bool ) ),
            this,   SLOT( update_disk_db( bool ) ) );

   pb_prefilt    = us_pushbutton( tr( "Select PreFilter" ) );

   pb_ldcolor    = us_pushbutton( tr( "Load Color File" ) );
   pb_ldcolor->setEnabled( true );
   connect( pb_ldcolor, SIGNAL( clicked() ),
            this,       SLOT( load_color() ) );

   le_prefilt    = us_lineedit( tr( "" ), -1, true );
   connect( pb_prefilt, SIGNAL( clicked() ),
            this,       SLOT( select_prefilt() ) );

   pb_lddistr    = us_pushbutton( tr( "Load Distribution(s)" ) );
   pb_lddistr->setEnabled( true );
   connect( pb_lddistr, SIGNAL( clicked() ),
            this,       SLOT( load_distro() ) );

   pb_rmvdist    = us_pushbutton( tr( "Remove Distribution(s)" ) );
   pb_rmvdist->setEnabled( true );
   connect( pb_rmvdist, SIGNAL( clicked() ),
            this,       SLOT( remove_distro() ) );

   pb_help       = us_pushbutton( tr( "Help" ) );
   pb_help->setEnabled( true );
   connect( pb_help,    SIGNAL( clicked() ),
            this,       SLOT( help() ) );

   pb_close      = us_pushbutton( tr( "Close" ) );
   pb_close->setEnabled( true );
   connect( pb_close,   SIGNAL( clicked() ),
            this,       SLOT( close() ) );

   QFontMetrics fm( ct_plt_smax->font() );
   ct_plt_smax->adjustSize();
   ct_plt_smax->setMinimumWidth( ct_plt_smax->width() + fm.width( "ABC" ) );

   // Order plot components on the left side
   spec->addWidget( lb_info1,      s_row++, 0, 1, 2 );
   spec->addWidget( lb_resolu,     s_row,   0 );
   spec->addWidget( ct_resolu,     s_row++, 1 );
   spec->addWidget( lb_xreso,      s_row,   0 );
   spec->addWidget( ct_xreso,      s_row++, 1 );
   spec->addWidget( lb_yreso,      s_row,   0 );
   spec->addWidget( ct_yreso,      s_row++, 1 );
   spec->addWidget( lb_zfloor,     s_row,   0 );
   spec->addWidget( ct_zfloor,     s_row++, 1 );
   spec->addWidget( cb_autosxy,    s_row,   0 );
   spec->addWidget( cb_autoscz,    s_row++, 1 );
   spec->addWidget( cb_conloop,    s_row,   0 );
   spec->addWidget( cb_zpcent,     s_row++, 1 );
   spec->addWidget( lb_plt_fmin,   s_row,   0 );
   spec->addWidget( ct_plt_fmin,   s_row++, 1 );
   spec->addWidget( lb_plt_fmax,   s_row,   0 );
   spec->addWidget( ct_plt_fmax,   s_row++, 1 );
   spec->addWidget( lb_plt_smin,   s_row,   0 );
   spec->addWidget( ct_plt_smin,   s_row++, 1 );
   spec->addWidget( lb_plt_smax,   s_row,   0 );
   spec->addWidget( ct_plt_smax,   s_row++, 1 );
   spec->addWidget( lb_plt_dlay,   s_row,   0 );
   spec->addWidget( ct_plt_dlay,   s_row++, 1 );
   spec->addWidget( lb_curr_distr, s_row,   0 );
   spec->addWidget( ct_curr_distr, s_row++, 1 );
   spec->addWidget( te_distr_info, s_row,   0, 2, 2 ); s_row += 2;
   spec->addWidget( le_cmap_name,  s_row++, 0, 1, 2 );
   spec->addWidget( cb_plot_sk,    s_row,   0 );
   spec->addWidget( cb_plot_wk,    s_row++, 1 );
   spec->addWidget( cb_plot_sv,    s_row,   0 );
   spec->addWidget( cb_plot_wv,    s_row++, 1 );
   spec->addWidget( pb_pltall,     s_row,   0 );
   spec->addWidget( pb_stopplt,    s_row++, 1 );
   spec->addWidget( pb_refresh,    s_row,   0 );
   spec->addWidget( pb_reset,      s_row++, 1 );
   spec->addLayout( dkdb_cntrls,   s_row++, 0, 1, 2 );
   spec->addWidget( le_prefilt,    s_row++, 0, 1, 2 );
   spec->addWidget( pb_prefilt,    s_row,   0 );
   spec->addWidget( pb_ldcolor,    s_row++, 1 );
   spec->addWidget( pb_lddistr,    s_row,   0 );
   spec->addWidget( pb_rmvdist,    s_row++, 1 );
   spec->addWidget( pb_help,       s_row,   0 );
   spec->addWidget( pb_close,      s_row++, 1 );

   // Set up plot component window on right side
   xa_title_s  = tr( "Sedimentation Coefficient (1e-13)"
                     " for water at 20" ) + DEGC;
   xa_title_w  = tr( "Molecular Weight (Dalton)" );
   xa_title    = xa_title_s;

   ya_title_k  = tr( "Frictional Ratio f/f0" );
   ya_title_v  = tr( "Vbar at 20" ) + DEGC;
   ya_title    = ya_title_k;
   QBoxLayout* plot = new US_Plot( data_plot, 
      tr( "Pseudo-3D Distribution Data" ), xa_title, ya_title );

   data_plot->setMinimumSize( 600, 600 );

   data_plot->enableAxis( QwtPlot::xBottom, true );
   data_plot->enableAxis( QwtPlot::yLeft,   true );
   data_plot->enableAxis( QwtPlot::yRight,  true );
   data_plot->setAxisScale( QwtPlot::xBottom, 1.0, 40.0 );
   data_plot->setAxisScale( QwtPlot::yLeft,   1.0,  4.0 );
   data_plot->setAxisScale( QwtPlot::yRight,  0.0,  0.2 );
   data_plot->setCanvasBackground( Qt::white );
   QwtText zTitle( "Partial Concentration" );
   zTitle.setFont( QFont( US_GuiSettings::fontFamily(),
      US_GuiSettings::fontSize(), QFont::Bold ) );
   data_plot->setAxisTitle( QwtPlot::yRight, zTitle );

   pick = new US_PlotPicker( data_plot );
   pick->setRubberBand( QwtPicker::RectRubberBand );

   // put layouts together for overall layout
   left->addLayout( spec );
   left->addStretch();

   main->addLayout( left );
   main->addLayout( plot );
   main->setStretchFactor( left, 3 );
   main->setStretchFactor( plot, 5 );

   mfilter    = "";
   plt_zmin   = 1e+8;
   plt_zmax   = -1e+8;
   runsel     = true;
   latest     = true;

   // Set up variables and initial state of GUI

   reset();
}

void US_Pseudo3D_Combine::reset( void )
{
   data_plot->detachItems( QwtPlotItem::Rtti_PlotSpectrogram );
   data_plot->replot();
 
   plot_s     = true;
   plot_k     = true;
   need_save  = true;
   plot_xy    = ( plot_s ? 0 : 1 ) + ( plot_k ? 0 : 2 );
   cb_plot_sk->setChecked( plot_xy == 0 );  
   cb_plot_wk->setChecked( plot_xy == 1 );
   cb_plot_sv->setChecked( plot_xy == 2 );
   cb_plot_wv->setChecked( plot_xy == 3 );

   resolu     = 90.0;
   ct_resolu->setRange( 1, 100, 1 );
   ct_resolu->setValue( resolu );  

   xreso      = 300.0;
   yreso      = 300.0;
   ct_xreso->setRange( 10.0, 1000.0, 1.0 );
   ct_xreso->setValue( (double)xreso );
   ct_yreso->setRange( 10, 1000, 1 );
   ct_yreso->setValue( (double)yreso );

   zfloor     = 5.0;
   ct_zfloor->setRange( 0, 50, 1 );
   ct_zfloor->setValue( (double)zfloor );

   auto_sxy   = true;
   cb_autosxy->setChecked( auto_sxy );
   auto_scz   = true;
   cb_autoscz->setChecked( auto_scz );
   cont_loop  = false;
   cb_conloop->setChecked( cont_loop );

   plt_fmin   = 0.8;
   plt_fmax   = 4.2;
   ct_plt_fmin->setRange( 0.1, 50, 0.01 );
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
   ct_curr_distr->setValue( curr_distr + 1 );
   ct_curr_distr->setEnabled( false );

   // default to white-cyan-magenta-red-black color map
   colormap  = new QwtLinearColorMap( Qt::white, Qt::black );
   colormap->addColorStop( 0.10, Qt::cyan );
   colormap->addColorStop( 0.50, Qt::magenta );
   colormap->addColorStop( 0.80, Qt::red );
   cmapname  = tr( "Default Color Map: w-cyan-magenta-red-black" );

   stop();
   system.clear();
   pfilts.clear();
   pb_pltall ->setEnabled( false );
   pb_refresh->setEnabled( false );
   pb_rmvdist->setEnabled( false );
   le_prefilt->setText( tr( "(no prefilter)" ) );
}

// plot the data
void US_Pseudo3D_Combine::plot_data( void )
{
   if ( curr_distr < 0  ||  curr_distr >= system.size() )
   {   // current distro index somehow out of valid range
      int syssiz = system.size();
      qDebug() << "curr_distr=" << curr_distr
         << "  ( sys.size()=" << syssiz << " )";
      syssiz--;
      curr_distr     = qBound( curr_distr, 0, syssiz );
   }

   zpcent   = cb_zpcent->isChecked();

   // get current distro
   plot_xy        = ( plot_s ? 0 : 1 ) + ( plot_k ? 0 : 2 );
   DisSys* tsys   = (DisSys*)&system.at( curr_distr );
   QList< Solute >* sol_d = &tsys->sk_distro;

   if ( zpcent )
   {
      data_plot->setAxisTitle( QwtPlot::yRight,
         tr( "Percent of Total Concentration" ) );
      if      ( plot_xy == 0 )  sol_d = &tsys->sk_distro_zp;
      else if ( plot_xy == 1 )  sol_d = &tsys->wk_distro_zp;
      else if ( plot_xy == 2 )  sol_d = &tsys->sv_distro_zp;
      else if ( plot_xy == 3 )  sol_d = &tsys->wv_distro_zp;
   }

   else
   {
      data_plot->setAxisTitle( QwtPlot::yRight,
         tr( "Partial Concentration" ) );
      if      ( plot_xy == 0 )  sol_d = &tsys->sk_distro;
      else if ( plot_xy == 1 )  sol_d = &tsys->wk_distro;
      else if ( plot_xy == 2 )  sol_d = &tsys->sv_distro;
      else if ( plot_xy == 3 )  sol_d = &tsys->wv_distro;
   }

   colormap = tsys->colormap;
   cmapname = tsys->cmapname;

   QString tstr = tsys->run_name + "\n" + tsys->analys_name
                  + "\n" + tsys->method;
   data_plot->setTitle( tstr );
   data_plot->detachItems( QwtPlotItem::Rtti_PlotSpectrogram );
   QColor bg   = colormap->color1();
   data_plot->setCanvasBackground( bg );
   int    csum = bg.red() + bg.green() + bg.blue();
   pick->setTrackerPen( QPen( csum > 600 ? QColor( Qt::black ) :
                                           QColor( Qt::white ) ) );

   // set up spectrogram data
   QwtPlotSpectrogram *d_spectrogram = new QwtPlotSpectrogram();
   d_spectrogram->setData( US_SpectrogramData() );
   d_spectrogram->setColorMap( *colormap );
   QwtDoubleRect drect;

   if ( auto_sxy )
      drect = QwtDoubleRect( 0.0, 0.0, 0.0, 0.0 );

   else
   {
      drect = QwtDoubleRect( plt_smin, plt_fmin,
            ( plt_smax - plt_smin ), ( plt_fmax - plt_fmin ) );
   }

   plt_zmin = zpcent ? 100.0 :  1e+8;
   plt_zmax = zpcent ?   0.0 : -1e+8;

   if ( auto_scz )
   {  // Find Z min,max for current distribution
      for ( int jj = 0; jj < sol_d->size(); jj++ )
      {
         double zval = sol_d->at( jj ).c;
         plt_zmin    = qMin( plt_zmin, zval );
         plt_zmax    = qMax( plt_zmax, zval );
      }
   }
   else
   {  // Find Z min,max for all distributions
      for ( int ii = 0; ii < system.size(); ii++ )
      {
         DisSys* tsys = (DisSys*)&system.at( ii );
         QList< Solute >* sol_z  = zpcent ? &tsys->sk_distro_zp
                                          : &tsys->sk_distro;

         for ( int jj = 0; jj < sol_z->size(); jj++ )
         {
            double zval = sol_z->at( jj ).c;
            plt_zmin    = qMin( plt_zmin, zval );
            plt_zmax    = qMax( plt_zmax, zval );
         }
      }
   }

   US_SpectrogramData& spec_dat = (US_SpectrogramData&)d_spectrogram->data();

   spec_dat.setRastRanges( xreso, yreso, resolu, zfloor, drect );
   spec_dat.setZRange( plt_zmin, plt_zmax );
   spec_dat.setRaster( *sol_d );

   d_spectrogram->attach( data_plot );

   // set color map and axis settings
   QwtScaleWidget *rightAxis = data_plot->axisWidget( QwtPlot::yRight );
   rightAxis->setColorBarEnabled( true );
   ya_title   = plot_k ? ya_title_k : ya_title_v;
   data_plot->setAxisTitle( QwtPlot::yLeft,   ya_title );
   plot_xy    = ( plot_s ? 0 : 1 ) + ( plot_k ? 0 : 2 );

   if ( plot_k )
   {
      lb_plt_fmin->setText( tr( "Plot Limit f/f0 Minimum:" ) );
      lb_plt_fmax->setText( tr( "Plot Limit f/f0 Maximum:" ) );
   }
   else
   {
      lb_plt_fmin->setText( tr( "Plot Limit vbar Minimum:" ) );
      lb_plt_fmax->setText( tr( "Plot Limit vbar Maximum:" ) );
   }

   if ( auto_sxy )
   { // Auto scale x and y
      data_plot->setAxisScale( QwtPlot::yLeft,  
         spec_dat.yrange().minValue(), spec_dat.yrange().maxValue() );
      data_plot->setAxisScale( QwtPlot::xBottom,
         spec_dat.xrange().minValue(), spec_dat.xrange().maxValue() );
   }
   else
   { // Manual limits on x and y
      double lStep = data_plot->axisStepSize( QwtPlot::yLeft   );
      double bStep = data_plot->axisStepSize( QwtPlot::xBottom );
      data_plot->setAxisScale( QwtPlot::xBottom, plt_smin, plt_smax, bStep );
      data_plot->setAxisScale( QwtPlot::yLeft,   plt_fmin, plt_fmax, lStep );
   }

   rightAxis->setColorMap( QwtDoubleInterval( plt_zmin, plt_zmax ),
      d_spectrogram->colorMap() );
   data_plot->setAxisScale( QwtPlot::yRight,  plt_zmin, plt_zmax );

   data_plot->replot();

   //QString dtext = te_distr_info->toPlainText().section( "\n", 0, 1 );
   QString dtext  = tr( "Run:  " ) + tsys->run_name
         + " (" + tsys->method + ")\n    " + tsys->analys_name;

   //bool sv_plot = ( looping && cb_conloop->isChecked() ) ? false : true;
   bool sv_plot = need_save;
DbgLv(2) << "(1) sv_plot" << sv_plot << "looping" << looping;

   if ( tsys->method.contains( "-MC" ) )
   {  // Test if some MC should be skipped
      sv_plot   = sv_plot && tsys->monte_carlo;   // Only plot if MC composite
DbgLv(2) << "(2)   sv_plot" << sv_plot;
   }

DbgLv(2) << "(3)   need_save sv_plot" << need_save << sv_plot;
   //if ( need_save  &&  sv_plot )
   if ( sv_plot )
   {  // Automatically save plot image in a PNG file
      QPixmap plotmap( data_plot->size() );
      plotmap.fill( US_GuiSettings::plotColor().color( QPalette::Background ) );

      QString runid  = tsys->run_name.section( ".",  0, -2 );
      QString triple = tsys->run_name.section( ".", -1, -1 );
      QString report = QString( "pseudo3d_" )
         + ( plot_k ? "ff0_" : "vbar_" ) + ( plot_s ? "s" : "MW" );

      QString ofdir  = US_Settings::reportDir() + "/" + runid;
      QDir dirof( ofdir );
      if ( !dirof.exists( ) )
         QDir( US_Settings::reportDir() ).mkdir( runid );
      QString ofname = tsys->method + "." + triple + "." + report + ".png";
      QString ofpath = ofdir + "/" + ofname;

      data_plot->print( plotmap );
      plotmap.save( ofpath );
      dtext          = dtext + tr( "\nPLOT %1 SAVED to local" )
         .arg( curr_distr + 1 );

      if ( dkdb_cntrls->db() )
      {  // Save a copy to the database
QDateTime time0=QDateTime::currentDateTime();
         US_Passwd   pw;
         US_DB2      db( pw.getPasswd() );
         QStringList query;
         query << "get_editID" << tsys->editGUID;
         db.query( query );
         db.next();
         int         idEdit   = db.value( 0 ).toString().toInt();
         US_Report   freport;
         freport.runID        = runid;
         freport.saveDocumentFromFile( ofdir, ofname, &db, idEdit );
QDateTime time1=QDateTime::currentDateTime();
qDebug() << "DB-save: currdist" << curr_distr
 << "svtime:" << time0.msecsTo(time1);
         dtext          = dtext + tr( " and DB" );
      }
   }

   else
      dtext          = dtext + tr( "\n(no plot saved)" );

   te_distr_info->setText( dtext );

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
   xreso  = dval;
}

void US_Pseudo3D_Combine::update_yreso( double dval )
{
   yreso  = dval;
}

void US_Pseudo3D_Combine::update_zfloor( double dval )
{
   zfloor = dval;
}

void US_Pseudo3D_Combine::update_curr_distr( double dval )
{
   curr_distr   = qRound( dval ) - 1;

   if ( curr_distr > (-1)  &&  curr_distr < system.size() )
   {
      DisSys* tsys = (DisSys*)&system.at( curr_distr );
      cmapname     = tsys->cmapname;
      le_cmap_name->setText( cmapname );
      colormap     = tsys->colormap;
      if ( ! looping )
         te_distr_info->setText( tr( "Run:  " ) + tsys->run_name
            + " (" + tsys->method + ")\n    " + tsys->analys_name );
   }

}

void US_Pseudo3D_Combine::update_plot_smin( double dval )
{
   plt_smin = dval;
}

void US_Pseudo3D_Combine::update_plot_smax( double dval )
{
   plt_smax = dval;

   // Use logarithmic steps if MW
   if ( ! plot_s )
   {
      double r10p = double( int( log10( dval ) ) - 2 );
      r10p        = qMax( r10p, 0.0 );
      double r10v = pow( 10.0, r10p + 2.0 );
      if ( ( dval - r10v ) <= 0.0 )
         r10p        = qMax( r10p - 1.0, 0.0 );
      double rinc = qMax( pow( 10.0, r10p ), 1.0 );

      ct_plt_smin->setRange( 0.0, 1.e+6, rinc );
      ct_plt_smax->setRange( 0.0, 1.e+9, rinc );
//DbgLv(1) << "plt_smax" << plt_smax << " rinc" << rinc;
   }
}

void US_Pseudo3D_Combine::update_plot_fmin( double dval )
{
   plt_fmin = dval;
}

void US_Pseudo3D_Combine::update_plot_fmax( double dval )
{
   plt_fmax = dval;
}

void US_Pseudo3D_Combine::select_autosxy()
{
   auto_sxy   = cb_autosxy->isChecked();
   ct_plt_fmin->setEnabled( !auto_sxy );
   ct_plt_fmax->setEnabled( !auto_sxy );
   ct_plt_smin->setEnabled( !auto_sxy );
   ct_plt_smax->setEnabled( !auto_sxy );

   set_limits();
}

void US_Pseudo3D_Combine::select_autoscz()
{
   auto_scz   = cb_autoscz->isChecked();

   set_limits();
}

void US_Pseudo3D_Combine::select_conloop()
{
   cont_loop  = cb_conloop->isChecked();
   DisSys* tsys   = (DisSys*)&system.at( curr_distr );
   QString dtext  = tr( "Run:  " ) + tsys->run_name
         + " (" + tsys->method + ")\n    " + tsys->analys_name;

   if ( cont_loop )
   {
      pb_pltall->setText( tr( "Plot All Distros in a Loop" ) );
      dtext          = dtext +
         tr( "\nWith continuous loop, plot files only saved during 1st pass." );
   }
   else
      pb_pltall->setText( tr( "Plot All Distros" ) );

   te_distr_info->setText( dtext );
}

void US_Pseudo3D_Combine::select_plot_sk()
{
   if ( ! cb_plot_sk->isChecked()  )  return;

   plot_s     = true;
   plot_k     = true;
   plot_xy    = ( plot_s ? 0 : 1 ) + ( plot_k ? 0 : 2 );
   cb_plot_wk->setChecked( false );
   cb_plot_sv->setChecked( false );
   cb_plot_wv->setChecked( false );
   xa_title   = plot_s    ? xa_title_s  : xa_title_w;
   ya_title   = plot_k    ? ya_title_k  : ya_title_v;
   data_plot->setAxisTitle( QwtPlot::xBottom, xa_title );
   data_plot->setAxisTitle( QwtPlot::yLeft,   ya_title );

   set_limits();

   plot_data();
}

void US_Pseudo3D_Combine::select_plot_wk()
{
   if ( ! cb_plot_wk->isChecked()  )  return;

   plot_s     = false;
   plot_k     = true;
   plot_xy    = ( plot_s ? 0 : 1 ) + ( plot_k ? 0 : 2 );
   cb_plot_sk->setChecked( false );
   cb_plot_sv->setChecked( false );
   cb_plot_wv->setChecked( false );
   xa_title   = plot_s    ? xa_title_s  : xa_title_w;
   ya_title   = plot_k    ? ya_title_k  : ya_title_v;
   data_plot->setAxisTitle( QwtPlot::xBottom, xa_title );
   data_plot->setAxisTitle( QwtPlot::yLeft,   ya_title );

   set_limits();

   plot_data();
}

void US_Pseudo3D_Combine::select_plot_sv()
{
   if ( ! cb_plot_sv->isChecked()  )  return;

   plot_s     = true;
   plot_k     = false;
   plot_xy    = ( plot_s ? 0 : 1 ) + ( plot_k ? 0 : 2 );
   cb_plot_sk->setChecked( false );
   cb_plot_wk->setChecked( false );
   cb_plot_wv->setChecked( false );
   xa_title   = plot_s    ? xa_title_s  : xa_title_w;
   ya_title   = plot_k    ? ya_title_k  : ya_title_v;
   data_plot->setAxisTitle( QwtPlot::xBottom, xa_title );
   data_plot->setAxisTitle( QwtPlot::yLeft,   ya_title );

   set_limits();

   plot_data();
}

void US_Pseudo3D_Combine::select_plot_wv()
{
   if ( ! cb_plot_wv->isChecked()  )  return;

   plot_s     = false;
   plot_k     = false;
   plot_xy    = ( plot_s ? 0 : 1 ) + ( plot_k ? 0 : 2 );
   cb_plot_sk->setChecked( false );
   cb_plot_wk->setChecked( false );
   cb_plot_sv->setChecked( false );
   xa_title   = plot_s    ? xa_title_s  : xa_title_w;
   ya_title   = plot_k    ? ya_title_k  : ya_title_v;
   data_plot->setAxisTitle( QwtPlot::xBottom, xa_title );
   data_plot->setAxisTitle( QwtPlot::yLeft,   ya_title );

   set_limits();

   plot_data();
}

void US_Pseudo3D_Combine::load_distro()
{
   // get a model description or set of descriptions for distribution data
   QList< US_Model > models;
   QStringList       mdescs;
   bool              loadDB = dkdb_cntrls->db();

   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
   US_ModelLoader dialog( loadDB, mfilter, models, mdescs, pfilts );
   dialog.move( this->pos() + QPoint( 200, 200 ) );

   connect( &dialog, SIGNAL(   changed( bool ) ),
            this, SLOT( update_disk_db( bool ) ) );
   QApplication::restoreOverrideCursor();

   if ( dialog.exec() != QDialog::Accepted )
      return;  // no selection made

   for ( int jj = 0; jj < models.count(); jj++ )
   {  // load each selected distribution model
      load_distro( models[ jj ], mdescs[ jj ] );
   }

   need_save  = true;
   plot_data();
   pb_rmvdist->setEnabled( models.count() > 0 );
}

void US_Pseudo3D_Combine::load_distro( US_Model model, QString mdescr )
{
   DisSys      tsys;
   Solute      sol_sk;
   Solute      sol_wk;
   Solute      sol_sv;
   Solute      sol_wv;

   model.update_coefficients();          // fill in any missing coefficients

   QString mdesc = mdescr.section( mdescr.left( 1 ), 1, 1 );

   // load current colormap
   tsys.colormap     = colormap;
   tsys.cmapname     = cmapname;

   tsys.run_name     = mdesc.section( ".",  0, -3 );
   QString asys      = mdesc.section( ".", -2, -2 );
   tsys.analys_name  = asys.section( "_",  0, -4 ) + "_"
                     + asys.section( "_", -2, -1 );
   tsys.method       = model.typeText();
   tsys.method       = ( mdesc.contains( "-CG" ) )
                       ? tsys.method.replace( "2DSA", "2DSA-CG" )
                       : tsys.method;
   tsys.editGUID     = model.editGUID;
DbgLv(1) << "LD: method" << tsys.method << "mdesc" << mdesc;

   if ( tsys.method == "Manual"  ||  tsys.method == "CUSTOMGRID" )
   {
      int jj            = mdesc.indexOf( ".model" );
      mdesc             = ( jj < 1 ) ? mdesc : mdesc.left( jj );
      tsys.run_name     = mdesc + ".0Z280";
      tsys.analys_name  = "e00_a00_" + tsys.method + "_local";
DbgLv(1) << "LD:  run_name" << tsys.run_name;
DbgLv(1) << "LD:  analys_name" << tsys.analys_name;
   }

   tsys.distro_type  = (int)model.analysis;
   tsys.monte_carlo  = model.monteCarlo;

   if ( model.monteCarlo )
   {  // Revisit setting if Monte Carlo
      QString miter = mdescr.section( mdescr.left( 1 ), 6 );
      int     kiter = miter.isEmpty() ? 0 : miter.toInt();

      if ( kiter < 2 )
      {  // Turn off flag if not composite MC model (is individual MC)
         tsys.monte_carlo = false;
      }
   }
      

   te_distr_info->setText( tr( "Run:  " ) + tsys.run_name
      + " (" + tsys.method + ")\n    " + tsys.analys_name );
   plt_zmin_co = 1e+8;
   plt_zmax_co = -1e+8;
   plt_zmin_zp = 100.0;
   plt_zmax_zp = 0.0;

   // read in and set distribution s,c,k values
   if ( tsys.distro_type != (int)US_Model::COFS )
   {
      double tot_conc = 0.0;

      for ( int jj = 0; jj < model.components.size(); jj++ )
      {
         sol_sk.s  = model.components[ jj ].s * 1.0e13;
         sol_sk.k  = model.components[ jj ].f_f0;
         sol_sk.c  = model.components[ jj ].signal_concentration;

         sol_wk.s  = model.components[ jj ].mw;
         sol_wk.k  = sol_sk.k;
         sol_wk.c  = sol_sk.c;

         sol_sv.s  = sol_sk.s;
         sol_sv.k  = model.components[ jj ].vbar20;
         sol_sv.c  = sol_sk.c;

         sol_wv.s  = sol_wk.s;
         sol_wv.k  = sol_sv.k;
         sol_wv.c  = sol_sk.c;

         tsys.sk_distro << sol_sk;
         tsys.wk_distro << sol_wk;
         tsys.sv_distro << sol_sv;
         tsys.wv_distro << sol_wv;

         plt_zmin_co = qMin( plt_zmin_co, sol_sk.c );
         plt_zmax_co = qMax( plt_zmax_co, sol_sk.c );
         tot_conc   += sol_sk.c;
      }

      // sort and reduce distributions
      sort_distro( tsys.sk_distro, true );
      sort_distro( tsys.wk_distro, true );
      sort_distro( tsys.sv_distro, true );
      sort_distro( tsys.wv_distro, true );
      int nsolsk = tsys.sk_distro.size();
      int nsolwk = tsys.wk_distro.size();
      int nsolsv = tsys.sv_distro.size();
      int nsolwv = tsys.wv_distro.size();
      tsys.sk_distro_zp.clear();
      tsys.wk_distro_zp.clear();
      tsys.sv_distro_zp.clear();
      tsys.wv_distro_zp.clear();

      // Create Z-as-percentage version of distributions

      for ( int jj = 0; jj < model.components.size(); jj++ )
      {
         double cozpc;

         if ( jj < nsolsk )
         {
            sol_sk      = tsys.sk_distro[ jj ];
            cozpc       = sol_sk.c * 100.0 / tot_conc;
            sol_sk.c    = cozpc;
            plt_zmin_zp = qMin( plt_zmin_zp, cozpc );
            plt_zmax_zp = qMax( plt_zmax_zp, cozpc );
            tsys.sk_distro_zp << sol_sk;
         }

         if ( jj < nsolwk )
         {
            sol_wk      = tsys.wk_distro[ jj ];
            cozpc       = sol_wk.c * 100.0 / tot_conc;
            sol_wk.c    = cozpc;
            plt_zmin_zp = qMin( plt_zmin_zp, cozpc );
            plt_zmax_zp = qMax( plt_zmax_zp, cozpc );
            tsys.wk_distro_zp << sol_wk;
         }

         if ( jj < nsolsv )
         {
            sol_sv      = tsys.sv_distro[ jj ];
            cozpc       = sol_sv.c * 100.0 / tot_conc;
            sol_sv.c    = cozpc;
            plt_zmin_zp = qMin( plt_zmin_zp, cozpc );
            plt_zmax_zp = qMax( plt_zmax_zp, cozpc );
            tsys.sv_distro_zp << sol_sv;
         }

         if ( jj < nsolwv )
         {
            sol_wv      = tsys.wv_distro[ jj ];
            cozpc       = sol_wv.c * 100.0 / tot_conc;
            sol_wv.c    = cozpc;
            plt_zmin_zp = qMin( plt_zmin_zp, cozpc );
            plt_zmax_zp = qMax( plt_zmax_zp, cozpc );
            tsys.wv_distro_zp << sol_wv;
         }
      }
   }

   // update current distribution record
   system.append( tsys );
   int jd     = system.size();
   curr_distr = jd - 1;
   ct_curr_distr->setRange( 1, jd, 1 );
   ct_curr_distr->setValue( jd );
   ct_curr_distr->setEnabled( true );

   if ( auto_sxy )
   {
      set_limits();
      ct_plt_fmin->setEnabled( false );
      ct_plt_fmax->setEnabled( false );
      ct_plt_smin->setEnabled( false );
      ct_plt_smax->setEnabled( false );
   }
   else
   {
      plt_smin    = ct_plt_smin->value();
      plt_smax    = ct_plt_smax->value();
      plt_fmin    = ct_plt_fmin->value();
      plt_fmax    = ct_plt_fmax->value();
      set_limits();
   }
   data_plot->setAxisScale( QwtPlot::xBottom, plt_smin, plt_smax );
   data_plot->setAxisScale( QwtPlot::yLeft,   plt_fmin, plt_fmax );

   pb_pltall ->setEnabled( true );
   pb_refresh->setEnabled( true );
   pb_reset  ->setEnabled( true );
   cb_plot_sk->setEnabled( true );

   if ( cont_loop )
      pb_pltall->setText( tr( "Plot All Distros in a Loop" ) );
   else
      pb_pltall->setText( tr( "Plot All Distros" ) );

}

void US_Pseudo3D_Combine::load_color()
{
   QString filter = tr( "Color Map files (*cm-*.xml);;" )
         + tr( "Any XML files (*.xml);;" )
         + tr( "Any files (*)" );

   // get an xml file name for the color map
   QString fname = QFileDialog::getOpenFileName( this,
      tr( "Load Color Map File" ),
      US_Settings::baseDataDir() + "/etc",
      filter,
      0, 0 );

   if ( fname.isEmpty() )
      return;

   // get the map from the file
   QList< QColor > cmcolor;
   QList< double > cmvalue;

   US_ColorGradIO::read_color_steps( fname, cmcolor, cmvalue );
   colormap  = new QwtLinearColorMap( cmcolor.first(), cmcolor.last() );

   for ( int jj = 1; jj < cmvalue.size() - 1; jj++ )
   {
      colormap->addColorStop( cmvalue.at( jj ), cmcolor.at( jj ) );
   }
   QFileInfo fi( fname );
   cmapname  = tr( "Color Map: " ) + fi.baseName();
   le_cmap_name->setText( cmapname );

   // save the map information for the current distribution
   if ( curr_distr < system.size() )
   {
      DisSys* tsys    = (DisSys*)&system.at( curr_distr );
      tsys->colormap  = colormap;
      tsys->cmapname  = cmapname;
   }
}

// Start a loop of plotting all distros
void US_Pseudo3D_Combine::plotall()
{
   looping    = true;
   pb_stopplt->setEnabled( true );
   curr_distr = 0;
   plot_data();
   patm_dlay  = qRound( ct_plt_dlay->value() * 1000.0 );

   patm_id    = startTimer( patm_dlay );

   if ( curr_distr == system.size() )
      curr_distr--;

   need_save  = true;
}

// Stop the distros-plotting loop
void US_Pseudo3D_Combine::stop()
{
   looping    = false;
   need_save  = true;
}

void US_Pseudo3D_Combine::set_limits()
{
   double fmin = 1.0e30;
   double fmax = -1.0e30;
   double smin = 1.0e30;
   double smax = -1.0e30;
   plot_s      = cb_plot_sk->isChecked() || cb_plot_sv->isChecked();
   plot_k      = cb_plot_sk->isChecked() || cb_plot_wk->isChecked();
   plot_xy     = ( plot_s ? 0 : 1 ) + ( plot_k ? 0 : 2 );
   double rdif;
   int    ii;
   int    jj;

   if ( plot_xy == 0 )
   {
      data_plot->setAxisTitle( QwtPlot::xBottom, xa_title_s );
      data_plot->setAxisTitle( QwtPlot::yLeft,   ya_title_k );

      // find min,max for S,f/f0 distributions
      for ( ii = 0; ii < system.size(); ii++ )
      {
         DisSys* tsys = (DisSys*)&system.at( ii );
         for ( jj = 0; jj < tsys->sk_distro.size(); jj++ )
         {
            double sval = tsys->sk_distro.at( jj ).s;
            double fval = tsys->sk_distro.at( jj ).k;
            smin        = ( smin < sval ) ? smin : sval;
            smax        = ( smax > sval ) ? smax : sval;
            fmin        = ( fmin < fval ) ? fmin : fval;
            fmax        = ( fmax > fval ) ? fmax : fval;
         }
      }
      lb_plt_smin->setText( tr( "Plot Limit s Minimum:" ) );
      lb_plt_smax->setText( tr( "Plot Limit s Maximum:" ) );
      lb_plt_fmin->setText( tr( "Plot Limit f/f0 Minimum:" ) );
      lb_plt_fmax->setText( tr( "Plot Limit f/f0 Maximum:" ) );
   }
   else if ( plot_xy == 1 )
   {
      data_plot->setAxisTitle( QwtPlot::xBottom, xa_title_w );
      data_plot->setAxisTitle( QwtPlot::yLeft,   ya_title_k );

      // find min,max for MW,f/f0 distributions
      for ( ii = 0; ii < system.size(); ii++ )
      {
         DisSys* tsys = (DisSys*)&system.at( ii );
         for ( jj = 0; jj < tsys->wk_distro.size(); jj++ )
         {
            double sval = tsys->wk_distro.at( jj ).s;
            double fval = tsys->wk_distro.at( jj ).k;
            smin        = ( smin < sval ) ? smin : sval;
            smax        = ( smax > sval ) ? smax : sval;
            fmin        = ( fmin < fval ) ? fmin : fval;
            fmax        = ( fmax > fval ) ? fmax : fval;
         }
      }
      lb_plt_smin->setText( tr( "Plot Limit mw Minimum:" ) );
      lb_plt_smax->setText( tr( "Plot Limit mw Maximum:" ) );
      lb_plt_fmin->setText( tr( "Plot Limit f/f0 Minimum:" ) );
      lb_plt_fmax->setText( tr( "Plot Limit f/f0 Maximum:" ) );
   }
   else if ( plot_xy == 2 )
   {
      data_plot->setAxisTitle( QwtPlot::xBottom, xa_title_s );
      data_plot->setAxisTitle( QwtPlot::yLeft,   ya_title_v );

      // find min,max for s,vbar distributions
      for ( ii = 0; ii < system.size(); ii++ )
      {
         DisSys* tsys = (DisSys*)&system.at( ii );
         for ( jj = 0; jj < tsys->sv_distro.size(); jj++ )
         {
            double sval = tsys->sv_distro.at( jj ).s;
            double fval = tsys->sv_distro.at( jj ).k;
            smin        = ( smin < sval ) ? smin : sval;
            smax        = ( smax > sval ) ? smax : sval;
            fmin        = ( fmin < fval ) ? fmin : fval;
            fmax        = ( fmax > fval ) ? fmax : fval;
         }
      }
      lb_plt_smin->setText( tr( "Plot Limit mw Minimum:" ) );
      lb_plt_smax->setText( tr( "Plot Limit mw Maximum:" ) );
      lb_plt_fmin->setText( tr( "Plot Limit vbar Minimum:" ) );
      lb_plt_fmax->setText( tr( "Plot Limit vbar Maximum:" ) );
   }
   else if ( plot_xy == 3 )
   {
      data_plot->setAxisTitle( QwtPlot::xBottom, xa_title_w );
      data_plot->setAxisTitle( QwtPlot::yLeft,   ya_title_v );

      // find min,max for MW,vbar distributions
      for ( ii = 0; ii < system.size(); ii++ )
      {
         DisSys* tsys = (DisSys*)&system.at( ii );
         for ( jj = 0; jj < tsys->wv_distro.size(); jj++ )
         {
            double sval = tsys->wv_distro.at( jj ).s;
            double fval = tsys->wv_distro.at( jj ).k;
            smin        = ( smin < sval ) ? smin : sval;
            smax        = ( smax > sval ) ? smax : sval;
            fmin        = ( fmin < fval ) ? fmin : fval;
            fmax        = ( fmax > fval ) ? fmax : fval;
         }
      }
      lb_plt_smin->setText( tr( "Plot Limit mw Minimum:" ) );
      lb_plt_smax->setText( tr( "Plot Limit mw Maximum:" ) );
      lb_plt_fmin->setText( tr( "Plot Limit vbar Minimum:" ) );
      lb_plt_fmax->setText( tr( "Plot Limit vbar Maximum:" ) );
   }

   // adjust minima, maxima
   rdif      = ( smax - smin ) / 10.0;
   smin     -= rdif;
   smax     += rdif;
   rdif      = ( fmax - fmin ) / 10.0;
   rdif      = ( rdif == 0.0 ) ? 0.1 : rdif;
DbgLv(1) << "SL: real fmin fmax" << fmin << fmax;
   fmin     -= rdif;
   fmax     += rdif;
DbgLv(1) << "SL: adjusted fmin fmax" << fmin << fmax;
   double rmin = smax * 10.0;
   double rinc = pow( 10.0, double( int( log10( smax ) ) - 2 ) );

   if ( auto_sxy )
   {  // Set auto limits on X and Y
      if ( plot_s )
      {
         ct_plt_smax->setRange( 0.0, rmin, rinc );
         ct_plt_smin->setRange( -( smax / 50.0 ), rmin, rinc );
         smax       += ( ( smax - smin ) / 20.0 );
         smin       -= ( ( smax - smin ) / 20.0 );
      }

      else
      {
         rmin      = (double)( qRound( smax / 1000.0 ) ) * 1000.0;
         rinc      = 1000.0;
         ct_plt_smax->setRange( 0.0, rmin, rinc );
         ct_plt_smin->setRange( 0.0, rmin, rinc );
         smax       += ( ( smax - smin ) / 100.0 );
         smin       -= ( ( smax - smin ) / 100.0 );
         smin        = ( smin < 0.0 ) ? 0.0 : smin;
      }

      if ( ( smax - smin ) < 1.0e-100 )
      {
         smin       -= ( smin / 30.0 );
         smax       += ( smax / 30.0 );
      }

      if ( plot_k )
      {
         fmax       += ( ( fmax - fmin ) / 20.0 );
         fmin       -= ( ( fmax - fmin ) / 20.0 );
         fmin        = qMax( fmin, 0.1 );

         if ( ( fmax - fmin ) < 1.0e-3 )
            fmax       += ( fmax / 10.0 );

         fmin        = (double)( (int)( fmin * 10.0 ) ) / 10.0;
         fmax        = (double)( (int)( fmax * 10.0 + 0.5 ) ) / 10.0;
      }

      else
      {
         fmax       += 0.01;
         fmin       -= 0.01;
DbgLv(1) << "SL: auto-adjusted fmin fmax" << fmin << fmax;
      }

      if ( plot_s )
      {
         smin        = (double)( (int)( smin * 10.0 ) ) / 10.0;
         smax        = (double)( (int)( smax * 10.0 + 0.5 ) ) / 10.0;
         if ( smin < 0.0  &&  smin > (-1.0) )
            smin        = 0.0;
      }
      else
      {
         smin        = (double)( (int)( smin / 10.0 ) ) * 10.0;
         smax        = (double)( (int)( smax / 10.0 + 1.5 ) ) * 10.0;
      }
DbgLv(1) << "SL: setVal fmin fmax" << fmin << fmax;
      ct_plt_smin->setValue( smin );
      ct_plt_smax->setValue( smax );
      ct_plt_fmin->setValue( fmin );
      ct_plt_fmax->setValue( fmax );

      plt_smin    = smin;
      plt_smax    = smax;
      plt_fmin    = fmin;
      plt_fmax    = fmax;
   }
   else
   {
      plt_smin    = ct_plt_smin->value();
      plt_smax    = ct_plt_smax->value();
      plt_fmin    = ct_plt_fmin->value();
      plt_fmax    = ct_plt_fmax->value();
      ct_plt_smax->setRange( 0.0, rmin, rinc );

      if ( plot_s )
         ct_plt_smin->setRange( -rmin, rmin, rinc );
      else
         ct_plt_smin->setRange( 0.0, rmin, rinc );

      if ( plot_k )
         ct_plt_fmax->setRange( 1, 50, 0.01 );
      else
         ct_plt_fmax->setRange( 0.1, 2.0, 0.01 );
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
      int kdup = 0;
      int jdup = 0;

      while ( (++jj) != listsols.end() )
      {     // loop to compare each entry to previous
          sol2    = *jj;         // solute entry

          if ( sol1.s != sol2.s  ||  sol1.k != sol2.k )
          {   // not a duplicate, so output to temporary list
             reduced.append( sol2 );
             jdup    = 0;
          }

          else
          {  // duplicate, so sum c value;
             sol2.c += sol1.c;   // sum c value
             sol2.s  = ( sol1.s + sol2.s ) * 0.5;  // average s,k
             sol2.k  = ( sol1.k + sol2.k ) * 0.5;
             reduced.replace( reduced.size() - 1, sol2 );
             kdup    = max( kdup, ++jdup );
          }

          sol1    = sol2;        // save entry for next iteration
      }

      if ( kdup > 0 )
      {   // if some reduction happened, replace list with reduced version
         double sc = 1.0 / (double)( kdup + 1 );
DbgLv(1) << "KDUP" << kdup;
//sc = 1.0;

         for ( int ii = 0; ii < reduced.size(); ii++ )
         {  // first scale c values by reciprocal of maximum replicate count
            reduced[ ii ].c *= sc;
         }

         listsols = reduced;
DbgLv(1) << " reduced-size" << reduced.size();
      }
   }
DbgLv(1) << " sol-size" << listsols.size();
   return;
}

void US_Pseudo3D_Combine::timerEvent( QTimerEvent *event )
{
   int tm_id  = event->timerId();

   if ( tm_id != patm_id )
   {  // if other than plot loop timer event, pass on to normal handler
      QWidget::timerEvent( event );
      return;
   }

   int syssiz = system.size();
   int maxsiz = syssiz - 1;
   int jdistr = curr_distr + 1;

   if ( syssiz > 0  &&  looping )
   {   // If still looping, plot the next distribution
      if ( jdistr > maxsiz )
      {  // If we have passed the end in looping, reset
         jdistr     = 0;

         if ( ! cont_loop )
         {  // If not in continuous loop, turn off looping flag
            jdistr     = curr_distr;
            looping    = false;
         }

         else
         {  // If in continuous loop, turn off save-plot flag
            need_save  = false;
         }
      }
      curr_distr = jdistr;
      plot_data();
   }

   if ( curr_distr > maxsiz  ||  !looping )
   {  // If past last distro or Stop clicked, stop the loop
      killTimer( tm_id );
      pb_stopplt->setEnabled( false );
      curr_distr = ( curr_distr > maxsiz ) ? maxsiz : curr_distr;
      need_save  = true;
   }
   ct_curr_distr->setValue( curr_distr + 1 );
}

// Reset Disk_DB control whenever data source is changed in any dialog
void US_Pseudo3D_Combine::update_disk_db( bool isDB )
{
   isDB ? dkdb_cntrls->set_db() : dkdb_cntrls->set_disk();
}

// Select a prefilter for model distributions list
void US_Pseudo3D_Combine::select_prefilt( void )
{
   pfilts.clear();

   US_SelectEdits sediag( dkdb_cntrls->db(), runsel, latest, pfilts );
   sediag.move( this->pos() + QPoint( 200, 200 ) );
   sediag.exec();

   int nedits    = pfilts.size();
   QString pfmsg;

   if ( nedits == 0 )
      pfmsg = tr( "(no prefilter)" );

   else if ( runsel )
      pfmsg = tr( "Run ID prefilter - %1 edit(s)" ).arg( nedits );

   else if ( latest )
      pfmsg = tr( "%1 Latest-Edit prefilter(s)" ).arg( nedits );

   else
      pfmsg = tr( "%1 total Edit prefilter(s) " ).arg( nedits );

   le_prefilt->setText( pfmsg );
}


// Remove distribution(s) from the models list
void US_Pseudo3D_Combine::remove_distro( void )
{
qDebug() << "Remove Distros";
   US_RemoveDistros rmvd( system );

   if ( rmvd.exec() == QDialog::Accepted )
   {
      int jd     = system.size();

      if ( jd < 1 )
      {
         reset();
         return;
      }

      curr_distr = 0;
      ct_curr_distr->setRange( 1, jd, 1 );
      ct_curr_distr->setValue( 1 );
      ct_curr_distr->setEnabled( true );
   }

   plot_data();
}

