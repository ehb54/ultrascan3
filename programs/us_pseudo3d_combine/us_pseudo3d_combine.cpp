//! \file us_pseudo3d_combine.cpp

#include <QApplication>

#include "us_pseudo3d_combine.h"
#include "us_spectrodata.h"
#include "us_model.h"
#include "us_license_t.h"
#include "us_license.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_math2.h"
#include "us_matrix.h"
#include "us_sleep.h"
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

const double epsilon = 0.005;  // equivalence magnitude ratio radius

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

   // Top banner
   QLabel* lb_info1      = us_banner( tr( "Pseudo-3D Plotting Controls" ) );
   spec->addWidget( lb_info1, s_row++, 0, 1, 2 );

   // Series of rows: most of them label on left, counter/box on right
   QLabel* lb_resolu     = us_label( tr( "Pseudo-3D Resolution:" ) );
   lb_resolu->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
   spec->addWidget( lb_resolu, s_row, 0 );

   ct_resolu     = us_counter( 3, 0.0, 100.0, 90.0 );
   ct_resolu->setStep( 1 );
   spec->addWidget( ct_resolu, s_row++, 1 );
   connect( ct_resolu, SIGNAL( valueChanged( double ) ),
            this,      SLOT( update_resolu( double ) ) );

   QLabel* lb_xreso      = us_label( tr( "X Resolution:" ) );
   lb_xreso->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
   spec->addWidget( lb_xreso, s_row, 0 );

   ct_xreso      = us_counter( 3, 10.0, 1000.0, 0.0 );
   ct_xreso->setStep( 1 );
   spec->addWidget( ct_xreso, s_row++, 1 );
   connect( ct_xreso,  SIGNAL( valueChanged( double ) ),
            this,      SLOT( update_xreso( double ) ) );

   QLabel* lb_yreso      = us_label( tr( "Y Resolution:" ) );
   lb_yreso->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
   spec->addWidget( lb_yreso, s_row, 0 );

   ct_yreso      = us_counter( 3, 10.0, 1000.0, 0.0 );
   ct_yreso->setStep( 1 );
   spec->addWidget( ct_yreso, s_row++, 1 );
   connect( ct_yreso,  SIGNAL( valueChanged( double ) ),
            this,      SLOT( update_yreso( double ) ) );

   QLabel* lb_zfloor     = us_label( tr( "Z Floor Percent:" ) );
   lb_zfloor->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
   spec->addWidget( lb_zfloor, s_row, 0 );

   ct_zfloor     = us_counter( 3, 0.0, 50.0, 1.0 );
   ct_zfloor->setStep( 1 );
   spec->addWidget( ct_zfloor, s_row++, 1 );
   connect( ct_zfloor, SIGNAL( valueChanged( double ) ),
            this,      SLOT( update_zfloor( double ) ) );

   QLabel* lb_autolim    = us_label( tr( "Automatic Plot Limits" ) );
   lb_autolim->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
   spec->addWidget( lb_autolim, s_row, 0 );

   us_checkbox( tr( "(unselect to override)" ), cb_autolim, true );
   spec->addWidget( cb_autolim, s_row++, 1 );
   connect( cb_autolim, SIGNAL( clicked() ),
            this,       SLOT( select_autolim() ) );

   lb_plt_fmin   = us_label( tr( "Plot Limit f/f0 Minimum:" ) );
   lb_plt_fmin->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
   spec->addWidget( lb_plt_fmin, s_row, 0 );

   ct_plt_fmin   = us_counter( 3, 1.0, 50.0, 0.0 );
   ct_plt_fmin->setStep( 1 );
   spec->addWidget( ct_plt_fmin, s_row++, 1 );
   connect( ct_plt_fmin, SIGNAL( valueChanged( double ) ),
            this,        SLOT( update_plot_fmin( double ) ) );

   lb_plt_fmax   = us_label( tr( "Plot Limit f/f0 Maximum:" ) );
   lb_plt_fmax->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
   spec->addWidget( lb_plt_fmax, s_row, 0 );

   ct_plt_fmax   = us_counter( 3, 1.0, 50.0, 1.0 );
   ct_plt_fmax->setStep( 1 );
   spec->addWidget( ct_plt_fmax, s_row++, 1 );
   connect( ct_plt_fmax, SIGNAL( valueChanged( double ) ),
            this,        SLOT( update_plot_fmax( double ) ) );

   lb_plt_smin   = us_label( tr( "Plot Limit s Minimum:" ) );
   lb_plt_smin->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
   spec->addWidget( lb_plt_smin, s_row, 0 );

   ct_plt_smin   = us_counter( 3, -10.0, 10000.0, 0.0 );
   ct_plt_smin->setStep( 1 );
   spec->addWidget( ct_plt_smin, s_row++, 1 );
   connect( ct_plt_smin, SIGNAL( valueChanged( double ) ),
            this,        SLOT( update_plot_smin( double ) ) );

   lb_plt_smax   = us_label( tr( "Plot Limit s Maximum:" ) );
   lb_plt_smax->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
   spec->addWidget( lb_plt_smax, s_row, 0 );

   ct_plt_smax   = us_counter( 3, 0.0, 10000.0, 0.0 );
   ct_plt_smax->setStep( 1 );
   spec->addWidget( ct_plt_smax, s_row++, 1 );
   connect( ct_plt_smax, SIGNAL( valueChanged( double ) ),
            this,        SLOT( update_plot_smax( double ) ) );

   QLabel* lb_plt_dlay   = us_label( tr( "Plot Loop Delay Seconds:" ) );
   lb_plt_dlay->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
   spec->addWidget( lb_plt_dlay, s_row, 0 );

   ct_plt_dlay   = us_counter( 3, 0.1, 30.0, 0.0 );
   ct_plt_dlay->setStep( 0.1 );
   spec->addWidget( ct_plt_dlay, s_row++, 1 );
   QSettings settings( "UltraScan3", "UltraScan" );
   patm_dlay     = settings.value( "slideDelay", PA_TMDIS_MS ).toInt();
   ct_plt_dlay->setValue( (double)( patm_dlay ) / 1000.0 );

   us_checkbox( tr( "Continuous Loop" ), cb_conloop, true );
   spec->addWidget( cb_conloop, s_row++, 1 );
   connect( cb_conloop, SIGNAL( clicked() ),
            this,       SLOT( select_conloop() ) );

   QLabel* lb_curr_distr = us_label( tr( "Current Distro:" ) );
   lb_curr_distr->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
   spec->addWidget( lb_curr_distr, s_row, 0 );

   ct_curr_distr = us_counter( 3, 0.0, 10.0, 0.0 );
   ct_curr_distr->setStep( 1 );
   spec->addWidget( ct_curr_distr, s_row++, 1 );
   connect( ct_curr_distr, SIGNAL( valueChanged( double ) ),
            this,          SLOT( update_curr_distr( double ) ) );

   te_distr_info = us_textedit();
   te_distr_info->setText    ( tr( "Run:  runID.triple (method)\n" )
            + tr( "    analysisID" ) );
   te_distr_info->setReadOnly( true );
   spec->addWidget( te_distr_info, s_row,   0, 2, 2 );
   s_row += 2;

   le_cmap_name  = us_lineedit(
         tr( "Default Color Map: w-cyan-magenta-red-black" ) );
   le_cmap_name->setReadOnly( true );
   spec->addWidget( le_cmap_name,  s_row++, 0, 1, 2 );
   te_distr_info->setMaximumHeight( le_cmap_name->height() * 2 );

   us_checkbox( tr( "Plot f/f0 vs s" ), cb_plot_s, true );
   spec->addWidget( cb_plot_s, s_row, 0 );
   connect( cb_plot_s,  SIGNAL( clicked() ),
            this,       SLOT( select_plot_s() ) );

   us_checkbox( tr( "Plot f/f0 vs MW" ), cb_plot_mw, false );
   spec->addWidget( cb_plot_mw, s_row++, 1 );
   connect( cb_plot_mw, SIGNAL( clicked() ),
            this,       SLOT( select_plot_mw() ) );

   pb_pltall     = us_pushbutton( tr( "Plot All Distros" ) );
   pb_pltall->setEnabled( false );
   spec->addWidget( pb_pltall, s_row, 0 );
   connect( pb_pltall,  SIGNAL( clicked() ),
            this,       SLOT( plotall() ) );

   pb_stopplt    = us_pushbutton( tr( "Stop Plotting Loop" ) );
   pb_stopplt->setEnabled( false );
   spec->addWidget( pb_stopplt, s_row++, 1 );
   connect( pb_stopplt, SIGNAL( clicked() ),
            this,       SLOT( stop() ) );

   pb_refresh    = us_pushbutton( tr( "Refresh Pseudo-3D Plot" ) );
   pb_refresh->setEnabled(  false );
   spec->addWidget( pb_refresh, s_row, 0 );
   connect( pb_refresh, SIGNAL( clicked() ),
            this,       SLOT( plot_data() ) );

   pb_reset      = us_pushbutton( tr( "Reset" ) );
   pb_reset->setEnabled( true );
   spec->addWidget( pb_reset, s_row++, 1 );
   connect( pb_reset,   SIGNAL( clicked() ),
            this,       SLOT( reset() ) );

   dkdb_cntrls   = new US_Disk_DB_Controls(
         US_Settings::default_data_location() );
   spec->addLayout( dkdb_cntrls, s_row++, 0, 1, 2 );
   connect( dkdb_cntrls, SIGNAL( changed( bool ) ),
            this,   SLOT( update_disk_db( bool ) ) );

   pb_lddistr    = us_pushbutton( tr( "Load Distribution(s)" ) );
   pb_lddistr->setEnabled( true );
   spec->addWidget( pb_lddistr, s_row, 0 );
   connect( pb_lddistr, SIGNAL( clicked() ),
            this,       SLOT( load_distro() ) );

   pb_ldcolor    = us_pushbutton( tr( "Load Color File" ) );
   pb_ldcolor->setEnabled( true );
   spec->addWidget( pb_ldcolor, s_row++, 1 );
   connect( pb_ldcolor, SIGNAL( clicked() ),
            this,       SLOT( load_color() ) );

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

   // set up plot component window on right side
   xa_title_s  = tr( "Sedimentation Coefficient (1e-13)"
                     " for water at 20" ) + DEGC;
   xa_title_mw = tr( "Molecular Weight (Dalton)" );
   xa_title    = xa_title_s;

   ya_title_ff = tr( "Frictional Ratio f/f0" );
   ya_title_vb = tr( "Vbar at 20" ) + DEGC;
   ya_title    = ya_title_ff;
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

   // Set up variables and initial state of GUI

   reset();
}

void US_Pseudo3D_Combine::reset( void )
{
   data_plot->detachItems();
   data_plot->replot();
 
   cnst_vbar  = true;
   plot_s     = true;
   need_save  = true;
   cb_plot_s->setChecked( plot_s );  
   cb_plot_mw->setChecked( !plot_s );

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

   auto_lim   = true;
   cb_autolim->setChecked( auto_lim );
   cont_loop  = false;
   cb_conloop->setChecked( cont_loop );

   plt_fmin   = 1.0;
   plt_fmax   = 4.0;
   ct_plt_fmin->setRange( 1, 50, 0.01 );
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
   pb_pltall ->setEnabled( false );
   pb_refresh->setEnabled( false );
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

   // get current distro
   DisSys* tsys   = (DisSys*)&system.at( curr_distr );
   QList< Solute >* sol_d = &tsys->s_distro;

   if ( !plot_s )
      sol_d    = &tsys->mw_distro;

   colormap = tsys->colormap;
   cmapname = tsys->cmapname;

   QString tstr = tsys->run_name + "\n" + tsys->analys_name
                  + "\n" + tsys->method;
   data_plot->setTitle( tstr );
   data_plot->detachItems();
   QColor bg   = colormap->color1();
   data_plot->setCanvasBackground( bg );
   int    csum = bg.red() + bg.green() + bg.blue();
   pick->setTrackerPen( QPen( csum > 600 ? QColor( Qt::black ) :
                                           QColor( Qt::white ) ) );
   bool autlim = ( auto_lim && ! looping );

   // set up spectrogram data
   QwtPlotSpectrogram *d_spectrogram = new QwtPlotSpectrogram();
   d_spectrogram->setData( US_SpectrogramData() );
   d_spectrogram->setColorMap( *colormap );
   QwtDoubleRect drect;

   if ( autlim )
      drect = QwtDoubleRect( 0.0, 0.0, 0.0, 0.0 );
   else
      drect = QwtDoubleRect( plt_smin, plt_fmin,
            ( plt_smax - plt_smin ), ( plt_fmax - plt_fmin ) );

   US_SpectrogramData& spec_dat = (US_SpectrogramData&)d_spectrogram->data();

   spec_dat.setRastRanges( xreso, yreso, resolu, zfloor, drect );
   spec_dat.setRaster( *sol_d );

   d_spectrogram->attach( data_plot );

   // set color map and axis settings
   QwtScaleWidget *rightAxis = data_plot->axisWidget( QwtPlot::yRight );
   rightAxis->setColorBarEnabled( true );
   QwtText zTitle( "Partial Concentration" );
   zTitle.setFont( QFont( US_GuiSettings::fontFamily(),
      US_GuiSettings::fontSize(), QFont::Bold ) );
   data_plot->setAxisTitle( QwtPlot::yRight, zTitle );
   data_plot->enableAxis( QwtPlot::yRight );
   ya_title   = cnst_vbar ? ya_title_ff : ya_title_vb;
   data_plot->setAxisTitle( QwtPlot::yLeft,   ya_title );

   if ( cnst_vbar )
   {
      lb_plt_fmin->setText( tr( "Plot Limit f/f0 Minimum:" ) );
      lb_plt_fmax->setText( tr( "Plot Limit f/f0 Maximum:" ) );
      cb_plot_s  ->setText( tr( "Plot f/f0 vs s"  ) );
      cb_plot_mw ->setText( tr( "Plot f/f0 vs MW" ) );
   }
   else
   {
      lb_plt_fmin->setText( tr( "Plot Limit vbar Minimum:" ) );
      lb_plt_fmax->setText( tr( "Plot Limit vbar Maximum:" ) );
      cb_plot_s  ->setText( tr( "Plot vbar vs s"  ) );
      cb_plot_mw ->setText( tr( "Plot vbar vs MW" ) );
   }

   if ( auto_lim  &&  ! looping )
   {   // auto limits and not looping
      rightAxis->setColorMap( spec_dat.range(), d_spectrogram->colorMap() );
      data_plot->setAxisScale( QwtPlot::yLeft,
         spec_dat.yrange().minValue(), spec_dat.yrange().maxValue() );
      data_plot->setAxisScale( QwtPlot::xBottom,
         spec_dat.xrange().minValue(), spec_dat.xrange().maxValue() );
      data_plot->setAxisScale( QwtPlot::yRight,
         spec_dat.range() .minValue(), spec_dat.range() .maxValue() );
   }
   else
   {   // manual limits or looping
      rightAxis->setColorMap( QwtDoubleInterval( plt_zmin, plt_zmax ),
         d_spectrogram->colorMap() );
      data_plot->setAxisScale( QwtPlot::xBottom, plt_smin, plt_smax );
      data_plot->setAxisScale( QwtPlot::yLeft,   plt_fmin, plt_fmax );
      data_plot->setAxisScale( QwtPlot::yRight,  plt_zmin, plt_zmax );
   }

   data_plot->replot();

   if ( need_save )
   {  // automatically save plot image in a PNG file
      QPixmap plotmap( data_plot->size() );
      plotmap.fill( US_GuiSettings::plotColor().color( QPalette::Background ) );

      QString ofdir  = US_Settings::reportDir() + "/"
         + tsys->run_name.section( ".", 0, -2 );
      QDir dirof( ofdir );
      if ( !dirof.exists( ) )
         QDir( US_Settings::reportDir() ).mkdir( tsys->run_name );
      QString celli  = "." + tsys->run_name.section( ".", -1, -1 ) + ".plot";
      QString distr  = QString().sprintf( "%2.2d", ( curr_distr + 1 ) );
      QString methi  = tsys->method;
      methi          = methi + "_pseudo3d_f" + ( plot_s ? "s" : "mw" );
      if ( tsys->distro_type == (int)US_Model::MANUAL )
         methi          = methi + ".00";
      QString ofname = ofdir + "/" + methi + celli + distr + ".png";

      data_plot->print( plotmap );
      plotmap.save( ofname );
   }

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
      te_distr_info->setText( tr( "Run:  " ) + tsys->run_name
         + " (" + tsys->method + ")\n    " + tsys->analys_name );
      cmapname     = tsys->cmapname;
      le_cmap_name->setText( cmapname );
      colormap     = tsys->colormap;
   }

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

void US_Pseudo3D_Combine::select_autolim()
{
   auto_lim   = cb_autolim->isChecked();
   ct_plt_fmin->setEnabled( !auto_lim );
   ct_plt_fmax->setEnabled( !auto_lim );
   ct_plt_smin->setEnabled( !auto_lim );
   ct_plt_smax->setEnabled( !auto_lim );

   set_limits();
}

void US_Pseudo3D_Combine::select_conloop()
{
   cont_loop  = cb_conloop->isChecked();

   if ( cont_loop )
      pb_pltall->setText( tr( "Plot All Distros in a Loop" ) );
   else
      pb_pltall->setText( tr( "Plot All Distros" ) );
}

void US_Pseudo3D_Combine::select_plot_s()
{
   plot_s     = cb_plot_s->isChecked();
   cb_plot_mw->setChecked( !plot_s );
   xa_title   = plot_s    ? xa_title_s  : xa_title_mw;
   data_plot->setAxisTitle( QwtPlot::xBottom, xa_title );
   set_limits();

   plot_data();
}

void US_Pseudo3D_Combine::select_plot_mw()
{
   plot_s     = !cb_plot_mw->isChecked();
   cb_plot_s->setChecked( plot_s );
   xa_title   = plot_s    ? xa_title_s  : xa_title_mw;
   data_plot->setAxisTitle( QwtPlot::xBottom, xa_title );
   set_limits();

   plot_data();
}

void US_Pseudo3D_Combine::load_distro()
{
   // get a model description or set of descriptions for distribution data
   QList< US_Model > models;
   QStringList       mdescs;
   bool              loadDB = dkdb_cntrls->db();

   US_ModelLoader dialog( loadDB, mfilter, models, mdescs );
   dialog.move( this->pos() + QPoint( 200, 200 ) );

   connect( &dialog, SIGNAL(   changed( bool ) ),
            this, SLOT( update_disk_db( bool ) ) );

   if ( dialog.exec() != QDialog::Accepted )
      return;  // no selection made

   for ( int jj = 0; jj < models.count(); jj++ )
   {  // load each selected distribution model
      load_distro( models[ jj ], mdescs[ jj ] );
   }

   plot_data();
}

void US_Pseudo3D_Combine::load_distro( US_Model model, QString mdescr )
{
   DisSys      tsys;
   Solute      sol_s;
   Solute      sol_w;

   model.update_coefficients();          // fill in any missing coefficients

   QString mdesc = mdescr.section( mdescr.left( 1 ), 1, 1 );

   // load current colormap
   tsys.colormap     = colormap;
   tsys.cmapname     = cmapname;

   tsys.run_name     = mdesc.section( ".",  0, -3 );
   QString asys      = mdesc.section( ".", -2, -2 );
   tsys.analys_name  = asys.section( "_", 0, 1 ) + "_"
                     + asys.section( "_", 3, 4 );
   tsys.method       = model.typeText();

   tsys.distro_type  = (int)model.analysis;
   tsys.monte_carlo  = model.monteCarlo;

   te_distr_info->setText( tr( "Run:  " ) + tsys.run_name
      + " (" + tsys.method + ")\n    " + tsys.analys_name );

   // read in and set distribution s,c,k values
   if ( tsys.distro_type != (int)US_Model::COFS )
   {
      for ( int jj = 0; jj < model.components.size(); jj++ )
      {
         double ffval = model.components[ jj ].f_f0;
         double vbval = model.components[ jj ].vbar20;
         sol_s.s  = model.components[ jj ].s * 1.0e13;
         sol_s.c  = model.components[ jj ].signal_concentration;
         sol_s.k  = cnst_vbar ? ffval : vbval;
         sol_w.s  = model.components[ jj ].mw;
         sol_w.c  = sol_s.c;
         sol_w.k  = sol_s.k;

         tsys.s_distro.append(  sol_s );
         tsys.mw_distro.append( sol_w );

         plt_zmin = qMin( plt_zmin, sol_s.c );
         plt_zmax = qMax( plt_zmax, sol_s.c );
      }

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

   // determine whether Y is f/f0 or vbar
   if ( curr_distr == 0 )
   {  // First distribution:  set constant-vbar flag; possibly re-do
      cnst_vbar  = model.constant_vbar();
qDebug() << "cd=0  cnst_vbar" << cnst_vbar;

      if ( ! cnst_vbar )
      {  // Oops!  We need to re-do the distribution using vbar instead of f/f0
         system        .clear();
         tsys.s_distro .clear();
         tsys.mw_distro.clear();
         double ffmin = model.components[ 0 ].f_f0;
         double ffmax = ffmin;

         for ( int jj = 0; jj < model.components.size(); jj++ )
         {
            double vbval = model.components[ jj ].vbar20;
            double ffval = model.components[ jj ].f_f0;
            sol_s.s  = model.components[ jj ].s * 1.0e13;
            sol_s.c  = model.components[ jj ].signal_concentration;
            sol_s.k  = cnst_vbar ? ffval : vbval;
            sol_w.s  = model.components[ jj ].mw;
            sol_w.c  = sol_s.c;
            sol_w.k  = sol_s.k;
            ffmin    = qMin( ffmin, ffval );
            ffmax    = qMax( ffmax, ffval );

            tsys.s_distro.append(  sol_s );
            tsys.mw_distro.append( sol_w );
         }

         // sort and reduce distributions
         sort_distro( tsys.s_distro,  true );
         sort_distro( tsys.mw_distro, true );

         ct_plt_fmin->setMinValue( 0.1   );
         ct_plt_fmin->setMaxValue( 1.5   );
         ct_plt_fmin->setValue   ( ffmin );
         ct_plt_fmax->setMinValue( 0.1   );
         ct_plt_fmax->setMaxValue( 1.5   );
         ct_plt_fmax->setValue   ( ffmax );
         lb_plt_fmin->setText( tr( "Plot Limit vbar Minimum:" ) );
         lb_plt_fmax->setText( tr( "Plot Limit vbar Maximum:" ) );
         system.append( tsys );
      }
   }

   else
   {  // Beyond first:  verify that models are all of the same type
      bool    c_vb_dis  = model.constant_vbar();
      bool    not_same  = false;
      QString msg;

      if ( c_vb_dis && ! cnst_vbar )
      {
         msg = tr( "Model %1 has a constant vbar;\n"
                   "    while vbars in the initial model vary (constant f/f0)\n"
                   "Plots will likely be scaled and annotated inconsistently." )
               .arg( jd );
         not_same   = true;
      }

      else if ( ! c_vb_dis && cnst_vbar )
      {
         msg = tr( "The initial model has a constant vbar;\n"
                   "    while vbars in model %1 vary (constant f/f0).\n"
                   "Plots will likely be scaled and annotated inconsistently." )
               .arg( jd );
         not_same   = true;
      }

      if ( not_same )
      {
         qDebug() << "INCONSISTENT DISTRIBUTIONS LOADED!!!";
         QMessageBox::warning( this, tr( "Inconsistent Distributions" ), msg );
         curr_distr = 0;
         ct_curr_distr->setValue( 1 );
         cb_autolim->setChecked( true  );
         plot_data();
         cb_autolim->setChecked( false );
      }
   }

   if ( auto_lim )
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

   pb_pltall->setEnabled(   true );
   pb_refresh->setEnabled(  true );
   pb_reset->setEnabled(    true );
   cb_plot_s->setEnabled(   true );
   cb_plot_mw->setEnabled(  true );

   if ( cont_loop )
      pb_pltall->setText( tr( "Plot All Distros in a Loop" ) );
   else
      pb_pltall->setText( tr( "Plot All Distros" ) );

}

void US_Pseudo3D_Combine::load_color()
{
   QString filter = tr( "Color Map files (cm*.xml);;" )
         + tr( "Any XML files (*.xml);;" )
         + tr( "Any files (*)" );

   // get an xml file name for the color map
   QString fname = QFileDialog::getOpenFileName( this,
      tr( "Load Color Map File" ),
      US_Settings::appBaseDir() + "/etc",
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
   double rdif;
   int    ii;
   int    jj;

   if ( plot_s )
   {
      data_plot->setAxisTitle( QwtPlot::xBottom, xa_title_s );

      // find min,max for S distributions
      for ( ii = 0; ii < system.size(); ii++ )
      {
         DisSys* tsys = (DisSys*)&system.at( ii );
         for ( jj = 0; jj < tsys->s_distro.size(); jj++ )
         {
            double sval = tsys->s_distro.at( jj ).s;
            double fval = tsys->s_distro.at( jj ).k;
            smin        = ( smin < sval ) ? smin : sval;
            smax        = ( smax > sval ) ? smax : sval;
            fmin        = ( fmin < fval ) ? fmin : fval;
            fmax        = ( fmax > fval ) ? fmax : fval;
         }
      }
      lb_plt_smin->setText( tr( "Plot Limit s Minimum:" ) );
      lb_plt_smax->setText( tr( "Plot Limit s Maximum:" ) );
   }
   else
   {
      data_plot->setAxisTitle( QwtPlot::xBottom, xa_title_mw );

      // find min,max for MW distributions
      for ( ii = 0; ii < system.size(); ii++ )
      {
         DisSys* tsys = (DisSys*)&system.at( ii );
         for ( jj = 0; jj < tsys->mw_distro.size(); jj++ )
         {
            double sval = tsys->mw_distro.at( jj ).s;
            double fval = tsys->mw_distro.at( jj ).k;
            smin        = ( smin < sval ) ? smin : sval;
            smax        = ( smax > sval ) ? smax : sval;
            fmin        = ( fmin < fval ) ? fmin : fval;
            fmax        = ( fmax > fval ) ? fmax : fval;
         }
      }
      lb_plt_smin->setText( tr( "Plot Limit mw Minimum:" ) );
      lb_plt_smax->setText( tr( "Plot Limit mw Maximum:" ) );
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
      }

      if ( ( smax - smin ) < 1.0e-100 )
      {
         smin       -= ( smin / 30.0 );
         smax       += ( smax / 30.0 );
      }

      if ( cnst_vbar )
      {
         fmax       += ( ( fmax - fmin ) / 20.0 );
         fmin       -= ( ( fmax - fmin ) / 20.0 );
         fmin        = ( fmin < 1.0 ) ? 1.0 : fmin;

         if ( ( fmax - fmin ) < 1.0e-3 )
            fmax       += ( fmax / 10.0 );

         fmin        = (double)( (int)( fmin * 10.0 ) ) / 10.0;
         fmax        = (double)( (int)( fmax * 10.0 + 0.5 ) ) / 10.0;
      }

      else
      {
         fmax       += 0.005;
         fmin       -= 0.005;
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

          if ( ! equivalent( sol1.s, sol2.s, epsilon )  ||
               ! equivalent( sol1.k, sol2.k, epsilon ) )
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
qDebug() << "KDUP" << kdup;
sc = 1.0;

         for ( int ii = 0; ii < reduced.size(); ii++ )
         {  // first scale c values by reciprocal of maximum replicate count
            reduced[ ii ].c *= sc;
         }

         listsols = reduced;
      }
   }
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
      {
         jdistr     = 0;
         need_save  = false;

         if ( ! cont_loop )
         {
            jdistr  = curr_distr;
            looping = false;
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
   }
   ct_curr_distr->setValue( curr_distr + 1 );
}

// Flag whether two values are effectively equal within a given epsilon
bool US_Pseudo3D_Combine::equivalent( double a, double b, double eps )
{
   return ( qAbs( ( a - b ) / a ) <= eps );
}

// Reset Disk_DB control whenever data source is changed in any dialog
void US_Pseudo3D_Combine::update_disk_db( bool isDB )
{
   isDB ? dkdb_cntrls->set_db() : dkdb_cntrls->set_disk();
}

