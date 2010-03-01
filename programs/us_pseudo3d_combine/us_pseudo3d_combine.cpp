//! \file us_pseudo3d_combine.cpp

#include <QApplication>

#include <uuid/uuid.h>

#include "us_pseudo3d_combine.h"
#include "us_spectrodata.h"
#include "us_license_t.h"
#include "us_license.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_math.h"
#include "us_matrix.h"
#include "us_sleep.h"

#define PA_TMDIS_MS 100   // default Plotall time per distro in milliseconds

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

   lb_zfloor     = us_label( tr( "Z Floor Percent:" ) );
   lb_zfloor->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
   spec->addWidget( lb_zfloor, s_row, 0 );

   ct_zfloor     = us_counter( 3, 0.0, 50.0, 1.0 );
   ct_zfloor->setStep( 1 );
   spec->addWidget( ct_zfloor, s_row++, 1 );
   connect( ct_zfloor, SIGNAL( valueChanged( double ) ),
            this,      SLOT( update_zfloor( double ) ) );

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

   ct_plt_fmin   = us_counter( 3, 1.0, 50.0, 0.0 );
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

   ct_plt_smin   = us_counter( 3, -10.0, 10000.0, 0.0 );
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

   le_cmap_name  = us_lineedit(
         tr( "Default Color Map: w-cyan-magenta-red-black" ) );
   le_cmap_name->setReadOnly( true );
   spec->addWidget( le_cmap_name,  s_row++, 0, 1, 2 );

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
   xa_title_s  = tr( "Sedimentation Coefficient corrected for water at 20" )
      + "<span>&deg;</span>C";
   xa_title_mw = tr( "Molecular Weight (Dalton)" );
   xa_title    = xa_title_s;

   QBoxLayout* plot = new US_Plot( data_plot, 
      tr( "Pseudo-3D Distribution Data" ),
      xa_title,
      tr( "Frictional Ratio f/f0" ) );

   data_plot->setMinimumSize( 600, 600 );

   data_plot->enableAxis( QwtPlot::xBottom, true );
   data_plot->enableAxis( QwtPlot::yLeft,   true );
   data_plot->enableAxis( QwtPlot::yRight,  true );
   data_plot->setAxisScale( QwtPlot::xBottom, 1.0, 40.0 );
   data_plot->setAxisScale( QwtPlot::yLeft,   1.0,  4.0 );
   data_plot->setAxisScale( QwtPlot::yRight,  0.0,  0.2 );
   data_plot->setCanvasBackground( Qt::darkBlue );

   pick = new US_PlotPicker( data_plot );
   pick->setRubberBand( QwtPicker::RectRubberBand );

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

   // default to white-cyan-magenta-red-black color map
   colormap  = new QwtLinearColorMap( Qt::white, Qt::black );
   colormap->addColorStop( 0.10, Qt::cyan );
   colormap->addColorStop( 0.50, Qt::magenta );
   colormap->addColorStop( 0.80, Qt::red );
   cmapname  = tr( "Default Color Map: w-cyan-magenta-red-black" );

   // set the plot-all slide show time delay in milliseconds
   QSettings settings( "UTHSCSA", "UltraScan" );
   patm_dlay = settings.value( "slideDelay", PA_TMDIS_MS ).toInt();
}

// plot the data
void US_Pseudo3D_Combine::plot_data( void )
{
   const char* cdtyp[] = 
   {
      "cofs",       "fe",         "sa2d",       "ga_mc",
      "sa2d_mc",    "ga",         "global",     "sa2d_mw",
      "ga_mw",      "sa2d_mw_mc", "ga_mw_mc",   "global",
      "global_mc"
   };

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
   QList< Solute >& sol_s = tsys->s_distro;
   if ( !plot_s )
      sol_s    = tsys->mw_distro;
   colormap = tsys->colormap;
   cmapname = tsys->cmapname;

   data_plot->clear();
   data_plot->setCanvasBackground( colormap->color1() ); 

   // set up spectrogram data
   QwtPlotSpectrogram *d_spectrogram = new QwtPlotSpectrogram();
   d_spectrogram->setData( US_SpectrogramData() );
   d_spectrogram->setColorMap( *colormap );

   US_SpectrogramData& spec_dat = (US_SpectrogramData&)d_spectrogram->data();

   spec_dat.setRastRanges( xreso, yreso, resolu, zfloor );
   spec_dat.setRaster( sol_s );

   d_spectrogram->attach( data_plot );

   // set color map and axis settings
   QwtScaleWidget *rightAxis = data_plot->axisWidget( QwtPlot::yRight );
   rightAxis->setColorBarEnabled( true );
   rightAxis->setColorMap( spec_dat.range(), d_spectrogram->colorMap() );
   QwtText zTitle( "Frequency" );
   zTitle.setFont( QFont( US_GuiSettings::fontFamily(),
      US_GuiSettings::fontSize(), QFont::Bold ) );
   data_plot->setAxisTitle( QwtPlot::yRight, zTitle );
   data_plot->setAxisScale( QwtPlot::yRight,
      spec_dat.range().minValue(), spec_dat.range().maxValue() );
   data_plot->enableAxis( QwtPlot::yRight );

   if ( auto_lim )
   {   // auto limits
      data_plot->setAxisScale( QwtPlot::yLeft,
         spec_dat.yrange().minValue(), spec_dat.yrange().maxValue() );
      data_plot->setAxisScale( QwtPlot::xBottom,
         spec_dat.xrange().minValue(), spec_dat.xrange().maxValue() );
   }
   else
   {   // manual limits
      data_plot->setAxisScale( QwtPlot::xBottom, plt_smin, plt_smax );
      data_plot->setAxisScale( QwtPlot::yLeft,   plt_fmin, plt_fmax );
   }

   QString tstr = tsys->run_name + "." + tsys->cell +
      tsys->wavelength + "\n" + tsys->method;
   QwtText qwtTitle( tstr );
   if ( tstr.length() > 40 )
   {
      qwtTitle.setFont( QFont( US_GuiSettings::fontFamily(),
         US_GuiSettings::fontSize()+1, QFont::Bold ) );
   }
   data_plot->setTitle( qwtTitle );

   data_plot->replot();

   // automatically save plot image in a PNG file
   QPixmap plotmap( data_plot->size() );
   plotmap.fill( US_GuiSettings::plotColor().color( QPalette::Background ) );
   data_plot->print( plotmap );

   QString ofdir  = US_Settings::reportDir() + "/" + tsys->run_name;
   QDir dirof( ofdir );
   if ( !dirof.exists( ) )
      QDir( US_Settings::reportDir() ).mkdir( tsys->run_name );
   QString celli  = "." + tsys->cell + tsys->wavelength;
   QString methi  = QString( cdtyp[tsys->distro_type-1] );
   methi          = methi + "_pseudo3d_f" + ( plot_s ? "s" : "mw" );
   if ( tsys->distro_type == 12 )
      methi          = methi + ".00";
   QString ofname = ofdir + "/" + methi + celli + ".png";
//qDebug() << "PNG SAVE ofname=" << ofname;

   plotmap.save( ofname );

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
      QString tstr = "Run " + tsys->run_name + "." + tsys->cell +
         tsys->wavelength + " (" + tsys->method + ")";
      le_distr_info->setText( tstr );
      cmapname     = tsys->cmapname;
      le_cmap_name->setText( cmapname );
//qDebug() << "UpdCurrDistr cmapname=" << cmapname;
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
   if ( auto_lim )
   {
      set_limits();
   }
   else
   {
      ct_plt_smin->setRange( -10.0, 10000.0, 0.01 );
      ct_plt_smax->setRange( 0.0, 10000.0, 0.01 );
   }
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
   QString filter =
      tr( "Any Distro files (" ) +
      "*.fe*_dis.* *.cofs*_dis.* *.sa2d*_dis.* *.ga*_dis.* *.global*_dis.*);;"
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
      "ga_mw_mc_dis",    "GA, MW Constrained, Monte Carlo",    "T",
      "global_dis",      "Global Distro",                      "T",
      "global_mc_dis",   "Global MC Distro",                   "T"
   };
   int ncdte = sizeof( cdtyp ) / sizeof( char* );

   DisSys      tsys;
   Solute      sol_s;
   Solute      sol_mw;

   // load current colormap
   tsys.colormap = colormap;
   tsys.cmapname = cmapname;

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

   tstr    = "Run " + tsys.run_name + "." + tsys.cell +
      tsys.wavelength + " (" + tsys.method + ")";
   le_distr_info->setText( tstr );

   tstr    = tsys.run_name + "." + tsys.cell +
      tsys.wavelength + "\n" + tsys.method;
   data_plot->setTitle( tstr );

   // read in and set distribution s,c,k values
   if ( tsys.distro_type > 0 )
   {
      QFile filei( fname );

      if ( filei.open( QIODevice::ReadOnly | QIODevice::Text ) )
      {
         QTextStream ts( &filei );
         QString     s1;
         QStringList l1;
         int         i1  = 1;
         int         i2  = 4;
         int         i3  = 5;
         int         i4  = 6;
         int         mxi = 0;

         if ( !ts.atEnd() )
         {
            s1       = ts.readLine();    // interpret header line
            l1       = s1.split( QRegExp( "\\s+" ) );
            i1       = l1.indexOf( QRegExp( "s_20.*", Qt::CaseInsensitive ) );
            i2       = l1.indexOf( QRegExp( "mw.*",   Qt::CaseInsensitive ) );
            i3       = l1.indexOf( QRegExp( "freq.*", Qt::CaseInsensitive ) );
            i4       = l1.indexOf( QRegExp( "f/f0.*", Qt::CaseInsensitive ) );
#if 0
qDebug() << "Header Line :" << s1;
qDebug() << " l1.size()=" << l1.size();
qDebug() << "  i1=" << i1;
qDebug() << "  i2=" << i2;
qDebug() << "  i3=" << i3;
qDebug() << "  i4=" << i4;
#endif
            mxi      = ( i1 > mxi ) ? i1  : mxi;
            mxi      = ( i2 > mxi ) ? i2  : mxi;
            mxi      = ( i3 > mxi ) ? i3  : mxi;
            mxi      = ( i4 > mxi ) ? i4  : mxi;
            i1       = ( i1 < 0 )   ? mxi : i1;
            i2       = ( i2 < 0 )   ? mxi : i2;
            i3       = ( i3 < 0 )   ? mxi : i3;
            i4       = ( i4 < 0 )   ? mxi : i4;
            mxi++;
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
            if ( l1.empty()  ||  l1.size() < mxi )
            {
               qDebug() << "BLANK/SHORT LINE: size=" << l1.size();
               continue;      // skip this line
            }
            dv1      = l1.at( i1 ).toDouble();  // S_20,W
            dv2      = l1.at( i2 ).toDouble();  // MW
            dv3      = l1.at( i3 ).toDouble();  // Frequency
            dv4      = l1.at( i4 ).toDouble();  // f/f0

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
//qDebug() << "  s_distro size=" << tsys.s_distro.size();

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
   else
   {
      plt_smin    = ct_plt_smin->value();
      plt_smax    = ct_plt_smax->value();
      plt_fmin    = ct_plt_fmin->value();
      plt_fmax    = ct_plt_fmax->value();
      ct_plt_smin->setRange( -10.0, 10000.0, 0.01 );
      ct_plt_smax->setRange( 0.0, 10000.0, 0.01 );
   }
   data_plot->setAxisScale( QwtPlot::xBottom, plt_smin, plt_smax );
   data_plot->setAxisScale( QwtPlot::yLeft,   plt_fmin, plt_fmax );

   pb_pltall->setEnabled(   true );
   pb_refresh->setEnabled(  true );
   pb_reset->setEnabled(    true );
   cb_plot_s->setEnabled(   true );
   cb_plot_mw->setEnabled(  true );

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
//qDebug() << "LoadColor cmapname=" << cmapname;

   // save the map information for the current distribution
   if ( system.size() > curr_distr )
   {
      DisSys* tsys    = (DisSys*)&system.at( curr_distr );
      tsys->colormap  = colormap;
      tsys->cmapname  = cmapname;
   }
}

void US_Pseudo3D_Combine::plotall()
{
   looping   = true;
   pb_stopplt->setEnabled( true );
   curr_distr = 0;
   plot_data();

   patm_id    = startTimer( patm_dlay );

   if ( curr_distr == system.size() )
      curr_distr--;

}

void US_Pseudo3D_Combine::stop()
{
//qDebug() << "\t\t+++STOP+++";
   looping  = false;
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
   else
   {
      plt_smin    = ct_plt_smin->value();
      plt_smax    = ct_plt_smax->value();
      plt_fmin    = ct_plt_fmin->value();
      plt_fmax    = ct_plt_fmax->value();
      ct_plt_smin->setRange( -10.0, 10000.0, 0.01 );
      ct_plt_smax->setRange( 0.0, 10000.0, 0.01 );
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
//qDebug() << "  SD:   reduced size=" << reduced.size();

      if ( reduced.size() < sizi )
      {   // if some reduction happened, replace list with reduced version
         listsols = reduced;
      }
   }
   return;
}

void US_Pseudo3D_Combine::timerEvent( QTimerEvent *event )
{
   int tm_id  = event->timerId();
//qDebug() << "Timer ID:" << tm_id;

   if ( tm_id != patm_id )
   {  // if other than plot loop timer event, pass on to normal handler
      QWidget::timerEvent( event );
      return;
   }

   int syssiz = system.size();
   int maxsiz = syssiz - 1;
   int jdistr = curr_distr + 1;

   if ( jdistr < syssiz  &&  looping )
   {   // if still looping, plot the next distribution
      curr_distr = jdistr;
      plot_data();
   }

   if ( curr_distr >= maxsiz  ||  !looping )
   {  // if at last distro or stop clicked, stop the loop
      killTimer( tm_id );
      pb_stopplt->setEnabled( false );
      curr_distr = ( curr_distr > maxsiz ) ? maxsiz : curr_distr;
   }
   ct_curr_distr->setValue( curr_distr+1 );
}


