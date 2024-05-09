//! \file us_pseudo3d_combine.cpp

#include <QApplication>

#include "us_pseudo3d_combine.h"
#include "us_spectrodata.h"
#include "us_remove_distros.h"
#include "us_select_runs.h"
#include "us_model.h"
#include "us_license_t.h"
#include "us_license.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_gui_util.h"
#include "us_math2.h"
#include "us_matrix.h"
#include "us_sleep.h"
#include "us_passwd.h"
#include "us_report.h"
#include "us_constants.h"
#if QT_VERSION < 0x050000
#define setSamples(a,b,c) setData(a,b,c)
#define setMinimum(a)     setMinValue(a)
#define setMaximum(a)     setMaxValue(a)
#endif

#define PA_TMDIS_MS 0   // default Plotall time per distro in milliseconds



// qSort LessThan method for S_Solute sort
bool distro_lessthan( const S_Solute &solu1, const S_Solute &solu2 )
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
   ct_resolu->setSingleStep( 1 );
   connect( ct_resolu, SIGNAL( valueChanged( double ) ),
            this,      SLOT( update_resolu( double ) ) );

   QLabel* lb_xreso      = us_label( tr( "X Resolution:" ) );
   lb_xreso->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );

   ct_xreso      = us_counter( 3, 10.0, 1000.0, 0.0 );
   ct_xreso->setSingleStep( 1 );
   connect( ct_xreso,  SIGNAL( valueChanged( double ) ),
            this,      SLOT( update_xreso( double ) ) );

   QLabel* lb_yreso      = us_label( tr( "Y Resolution:" ) );
   lb_yreso->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );

   ct_yreso      = us_counter( 3, 10.0, 1000.0, 0.0 );
   ct_yreso->setSingleStep( 1 );
   connect( ct_yreso,  SIGNAL( valueChanged( double ) ),
            this,      SLOT( update_yreso( double ) ) );

   QLabel* lb_zfloor     = us_label( tr( "Z Visibility Percent:" ) );
   lb_zfloor->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );

   ct_zfloor     = us_counter( 3, 50.0, 150.0, 1.0 );
   ct_zfloor->setSingleStep( 1 );
   connect( ct_zfloor, SIGNAL( valueChanged( double ) ),
            this,      SLOT( update_zfloor( double ) ) );

   us_checkbox( tr( "Autoscale X and Y" ), ck_autosxy, true );
   connect( ck_autosxy, SIGNAL( clicked() ),
            this,       SLOT( select_autosxy() ) );

   us_checkbox( tr( "Autoscale Z" ), ck_autoscz, true );
   connect( ck_autoscz, SIGNAL( clicked() ),
            this,       SLOT( select_autoscz() ) );

   us_checkbox( tr( "Continuous Loop" ), ck_conloop, true );
   connect( ck_conloop, SIGNAL( clicked() ),
            this,       SLOT( select_conloop() ) );

   us_checkbox( tr( "Z as Percentage" ), ck_zpcent,  false );

   us_checkbox( tr( "Save Plot(s)"    ), ck_savepl,  false );
   us_checkbox( tr( "Local Save Only" ), ck_locsave, true  );

   lb_plt_kmin   = us_label( tr( "Plot Limit f/f0 Minimum:" ) );
   lb_plt_kmin->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );

   ct_plt_kmin   = us_counter( 3, 0.5, 50.0, 1.0 );
   ct_plt_kmin->setSingleStep( 1 );
   connect( ct_plt_kmin, SIGNAL( valueChanged( double ) ),
            this,        SLOT( update_plot_kmin( double ) ) );

   lb_plt_kmax   = us_label( tr( "Plot Limit f/f0 Maximum:" ) );
   lb_plt_kmax->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );

   ct_plt_kmax   = us_counter( 3, 1.0, 50.0, 4.0 );
   ct_plt_kmax->setSingleStep( 1 );
   connect( ct_plt_kmax, SIGNAL( valueChanged( double ) ),
            this,        SLOT( update_plot_kmax( double ) ) );

   lb_plt_smin   = us_label( tr( "Plot Limit s Minimum:" ) );
   lb_plt_smin->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );

   ct_plt_smin   = us_counter( 3, -100000.0, 100000.0, 1.0 );
   ct_plt_smin->setSingleStep( 1 );
   connect( ct_plt_smin, SIGNAL( valueChanged( double ) ),
            this,        SLOT( update_plot_smin( double ) ) );

   lb_plt_smax   = us_label( tr( "Plot Limit s Maximum:" ) );
   lb_plt_smax->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );

   ct_plt_smax   = us_counter( 3, -100.0, 100000.0, 10.0 );
   ct_plt_smax->setSingleStep( 1 );
   connect( ct_plt_smax, SIGNAL( valueChanged( double ) ),
            this,        SLOT( update_plot_smax( double ) ) );

   QLabel* lb_plt_dlay   = us_label( tr( "Plot Loop Delay Seconds:" ) );
   lb_plt_dlay->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );

   ct_plt_dlay   = us_counter( 3, 0.0, 30.0, 0.0 );
   ct_plt_dlay->setSingleStep( 0.1 );
//   QSettings settings( "UltraScan3", "UltraScan" );
//   patm_dlay     = settings.value( "slideDelay", PA_TMDIS_MS ).toInt();
   patm_dlay     = PA_TMDIS_MS;
   ct_plt_dlay->setValue( (double)( patm_dlay ) / 1000.0 );

   QLabel* lb_curr_distr = us_label( tr( "Current Distro:" ) );
   lb_curr_distr->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );

   ct_curr_distr = us_counter( 3, 0.0, 10.0, 0.0 );
   ct_curr_distr->setSingleStep( 1 );
   connect( ct_curr_distr, SIGNAL( valueChanged     ( double ) ),
            this,          SLOT(   update_curr_distr( double ) ) );

   te_distr_info = us_textedit();
   te_distr_info->setText    ( tr( "Run:  runID.triple (method)\n" )
            + tr( "    analysisID" ) );
   us_setReadOnly( te_distr_info, true );

   le_cmap_name  = us_lineedit(
         tr( "Default Color Map: w-cyan-magenta-red-black" ), -1, true );
   te_distr_info->setMaximumHeight( le_cmap_name->height() * 2 );

           plot_x      = 0;
           plot_y      = 1;
   QLabel* lb_x_axis   = us_label( tr( "Plot X:" ) );
   QLabel* lb_y_axis   = us_label( tr( "Plot Y:" ) );
           bg_x_axis   = new QButtonGroup( this );
           bg_y_axis   = new QButtonGroup( this );
   QGridLayout*  gl_x_s    = us_radiobutton( tr( "s"   ), rb_x_s,    true  );
   QGridLayout*  gl_x_ff0  = us_radiobutton( tr( "ff0" ), rb_x_ff0,  false );
   QGridLayout*  gl_x_mw   = us_radiobutton( tr( "mw"  ), rb_x_mw,   false );
   QGridLayout*  gl_x_vbar = us_radiobutton( tr( "vbar"), rb_x_vbar, false );
   QGridLayout*  gl_x_D    = us_radiobutton( tr( "D"   ), rb_x_D,    false );
   QGridLayout*  gl_x_f    = us_radiobutton( tr( "f"   ), rb_x_f,    false );
   QGridLayout*  gl_y_s    = us_radiobutton( tr( "s"   ), rb_y_s,    false );
   QGridLayout*  gl_y_ff0  = us_radiobutton( tr( "ff0" ), rb_y_ff0,  true  );
   QGridLayout*  gl_y_mw   = us_radiobutton( tr( "mw"  ), rb_y_mw,   false );
   QGridLayout*  gl_y_vbar = us_radiobutton( tr( "vbar"), rb_y_vbar, false );
   QGridLayout*  gl_y_D    = us_radiobutton( tr( "D"   ), rb_y_D,    false );
   QGridLayout*  gl_y_f    = us_radiobutton( tr( "f"   ), rb_y_f,    false );
   bg_x_axis->addButton( rb_x_s,    ATTR_S );
   bg_x_axis->addButton( rb_x_ff0,  ATTR_K );
   bg_x_axis->addButton( rb_x_mw,   ATTR_W );
   bg_x_axis->addButton( rb_x_vbar, ATTR_V );
   bg_x_axis->addButton( rb_x_D,    ATTR_D );
   bg_x_axis->addButton( rb_x_f,    ATTR_F );
   bg_y_axis->addButton( rb_y_s,    ATTR_S );
   bg_y_axis->addButton( rb_y_ff0,  ATTR_K );
   bg_y_axis->addButton( rb_y_mw,   ATTR_W );
   bg_y_axis->addButton( rb_y_vbar, ATTR_V );
   bg_y_axis->addButton( rb_y_D,    ATTR_D );
   bg_y_axis->addButton( rb_y_f,    ATTR_F );
   rb_x_s   ->setChecked( true  );
   rb_y_s   ->setEnabled( false );
   rb_y_ff0 ->setChecked( true  );
   rb_x_ff0 ->setEnabled( false );
   rb_x_s   ->setToolTip( tr( "Set X axis to Sedimentation Coefficient" ) );
   rb_x_ff0 ->setToolTip( tr( "Set X axis to Frictional Ratio"          ) );
   rb_x_mw  ->setToolTip( tr( "Set X axis to Molecular Weight"          ) );
   rb_x_vbar->setToolTip( tr( "Set X axis to Partial Specific Volume"   ) );
   rb_x_D   ->setToolTip( tr( "Set X axis to Diffusion Coefficient"     ) );
   rb_x_f   ->setToolTip( tr( "Set X axis to Frictional Coefficient"    ) );
   rb_y_s   ->setToolTip( tr( "Set Y axis to Sedimentation Coefficient" ) );
   rb_y_ff0 ->setToolTip( tr( "Set Y axis to Frictional Ratio"          ) );
   rb_y_mw  ->setToolTip( tr( "Set Y axis to Molecular Weight"          ) );
   rb_y_vbar->setToolTip( tr( "Set Y axis to Partial Specific Volume"   ) );
   rb_y_D   ->setToolTip( tr( "Set Y axis to Diffusion Coefficient"     ) );
   rb_y_f   ->setToolTip( tr( "Set Y axis to Frictional Coefficient"    ) );
   connect( bg_x_axis,  SIGNAL( buttonReleased( int ) ),
            this,       SLOT  ( select_x_axis ( int ) ) );
   connect( bg_y_axis,  SIGNAL( buttonReleased( int ) ),
            this,       SLOT  ( select_y_axis ( int ) ) );

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
   spec->addWidget( lb_info1,      s_row++, 0, 1, 8 );
   spec->addWidget( lb_resolu,     s_row,   0, 1, 4 );
   spec->addWidget( ct_resolu,     s_row++, 4, 1, 4 );
   spec->addWidget( lb_xreso,      s_row,   0, 1, 4 );
   spec->addWidget( ct_xreso,      s_row++, 4, 1, 4 );
   spec->addWidget( lb_yreso,      s_row,   0, 1, 4 );
   spec->addWidget( ct_yreso,      s_row++, 4, 1, 4 );
   spec->addWidget( lb_zfloor,     s_row,   0, 1, 4 );
   spec->addWidget( ct_zfloor,     s_row++, 4, 1, 4 );
   spec->addWidget( ck_autosxy,    s_row,   0, 1, 4 );
   spec->addWidget( ck_autoscz,    s_row++, 4, 1, 4 );
   spec->addWidget( ck_conloop,    s_row,   0, 1, 4 );
   spec->addWidget( ck_zpcent,     s_row++, 4, 1, 4 );
   spec->addWidget( ck_savepl,     s_row,   0, 1, 4 );
   spec->addWidget( ck_locsave,    s_row++, 4, 1, 4 );
   spec->addWidget( lb_plt_kmin,   s_row,   0, 1, 4 );
   spec->addWidget( ct_plt_kmin,   s_row++, 4, 1, 4 );
   spec->addWidget( lb_plt_kmax,   s_row,   0, 1, 4 );
   spec->addWidget( ct_plt_kmax,   s_row++, 4, 1, 4 );
   spec->addWidget( lb_plt_smin,   s_row,   0, 1, 4 );
   spec->addWidget( ct_plt_smin,   s_row++, 4, 1, 4 );
   spec->addWidget( lb_plt_smax,   s_row,   0, 1, 4 );
   spec->addWidget( ct_plt_smax,   s_row++, 4, 1, 4 );
   spec->addWidget( lb_plt_dlay,   s_row,   0, 1, 4 );
   spec->addWidget( ct_plt_dlay,   s_row++, 4, 1, 4 );
   spec->addWidget( lb_curr_distr, s_row,   0, 1, 4 );
   spec->addWidget( ct_curr_distr, s_row++, 4, 1, 4 );
   spec->addWidget( te_distr_info, s_row,   0, 2, 8 ); s_row += 2;
   spec->addWidget( le_cmap_name,  s_row++, 0, 1, 8 );
   spec->addWidget( lb_x_axis,     s_row,   0, 1, 2 );
   spec->addLayout( gl_x_s,        s_row,   2, 1, 1 );
   spec->addLayout( gl_x_ff0,      s_row,   3, 1, 1 );
   spec->addLayout( gl_x_mw,       s_row,   4, 1, 1 );
   spec->addLayout( gl_x_vbar,     s_row,   5, 1, 1 );
   spec->addLayout( gl_x_D,        s_row,   6, 1, 1 );
   spec->addLayout( gl_x_f,        s_row++, 7, 1, 1 );
   spec->addWidget( lb_y_axis,     s_row,   0, 1, 2 );
   spec->addLayout( gl_y_s,        s_row,   2, 1, 1 );
   spec->addLayout( gl_y_ff0,      s_row,   3, 1, 1 );
   spec->addLayout( gl_y_mw,       s_row,   4, 1, 1 );
   spec->addLayout( gl_y_vbar,     s_row,   5, 1, 1 );
   spec->addLayout( gl_y_D,        s_row,   6, 1, 1 );
   spec->addLayout( gl_y_f,        s_row++, 7, 1, 1 );
   spec->addWidget( pb_pltall,     s_row,   0, 1, 4 );
   spec->addWidget( pb_stopplt,    s_row++, 4, 1, 4 );
   spec->addWidget( pb_refresh,    s_row,   0, 1, 4 );
   spec->addWidget( pb_reset,      s_row++, 4, 1, 4 );
   spec->addLayout( dkdb_cntrls,   s_row++, 0, 1, 8 );
   spec->addWidget( le_prefilt,    s_row++, 0, 1, 8 );
   spec->addWidget( pb_prefilt,    s_row,   0, 1, 4 );
   spec->addWidget( pb_ldcolor,    s_row++, 4, 1, 4 );
   spec->addWidget( pb_lddistr,    s_row,   0, 1, 4 );
   spec->addWidget( pb_rmvdist,    s_row++, 4, 1, 4 );
   spec->addWidget( pb_help,       s_row,   0, 1, 4 );
   spec->addWidget( pb_close,      s_row++, 4, 1, 4 );

   // Set up plot component window on right side
   xa_title    = anno_title( ATTR_S );
   ya_title    = anno_title( ATTR_K );
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

void US_Pseudo3D_Combine::reset_auto( void )
{
  reset();
}

void US_Pseudo3D_Combine::reset( void )
{
   dataPlotClear( data_plot );
   data_plot->replot();
 
   need_save  = false;

   plot_x     = ATTR_S;
   plot_y     = ATTR_K;
   resolu     = 90.0;
   ct_resolu->setRange( 1.0, 100.0 );
   ct_resolu->setSingleStep( 1.0 );
   ct_resolu->setValue( resolu );  

   xreso      = 300.0;
   yreso      = 300.0;
   ct_xreso->setRange( 10.0, 1000.0 );
   ct_xreso->setSingleStep( 1.0 );
   ct_xreso->setValue( (double)xreso );
   ct_yreso->setRange( 10.0, 1000.0 );
   ct_yreso->setSingleStep( 1.0 );
   ct_yreso->setValue( (double)yreso );

   zfloor     = 100.0;
   ct_zfloor->setRange( 50.0, 150.0 );
   ct_zfloor->setSingleStep( 1.0 );
   ct_zfloor->setValue( (double)zfloor );

   auto_sxy   = true;
   ck_autosxy->setChecked( auto_sxy );
   auto_scz   = true;
   ck_autoscz->setChecked( auto_scz );
   cont_loop  = false;
   ck_conloop->setChecked( cont_loop );
   ck_savepl ->setChecked( false     );
   ck_locsave->setChecked( true      );

   plt_kmin   = 0.8;
   plt_kmax   = 4.2;
   ct_plt_kmin->setRange( 0.0, 50.0 );
   ct_plt_kmin->setSingleStep( 0.01 );
   ct_plt_kmin->setValue( plt_kmin );
   ct_plt_kmin->setEnabled( false );
   ct_plt_kmax->setRange( 1.0, 50.0 );
   ct_plt_kmax->setSingleStep( 0.01 );
   ct_plt_kmax->setValue( plt_kmax );
   ct_plt_kmax->setEnabled( false );

   plt_smin   = 1.0;
   plt_smax   = 10.0;
   ct_plt_smin->setRange( -1000.0, 100000.0 );
   ct_plt_smin->setSingleStep( 0.01 );
   ct_plt_smin->setValue( plt_smin );
   ct_plt_smin->setEnabled( false );
   ct_plt_smax->setRange(  -100.0, 100000.0 );
   ct_plt_smax->setSingleStep( 0.01 );
   ct_plt_smax->setValue( plt_smax );
   ct_plt_smax->setEnabled( false );

   curr_distr = 0;
   ct_curr_distr->setRange( 1.0, 1.0 );
   ct_curr_distr->setSingleStep( 1.0 );
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
   int syssiz = system.size();

   if ( syssiz < 1 )
      return;

   if ( curr_distr < 0  ||  curr_distr >= syssiz )
   {   // current distro index somehow out of valid range
      qDebug() << "curr_distr=" << curr_distr
         << "  ( sys.size()=" << syssiz << " )";
      syssiz--;
      curr_distr     = qBound( curr_distr, 0, syssiz );
   }

   zpcent   = ck_zpcent->isChecked();

   // Get current distro and (if need be) rebuild XY distro
   DisSys* tsys   = (DisSys*)&system.at( curr_distr );
   QList< S_Solute >* sol_d = &tsys->sk_distro;

   build_xy_distro();

   if ( zpcent )
   {
      data_plot->setAxisTitle( QwtPlot::yRight,
         tr( "Percent of Total Concentration" ) );
      sol_d    = &tsys->xy_distro_zp;
   }

   else
   {
      data_plot->setAxisTitle( QwtPlot::yRight,
         tr( "Partial Concentration" ) );
      sol_d    = &tsys->xy_distro;
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

   // Set up spectrogram data
   QwtPlotSpectrogram* d_spectrogram = new QwtPlotSpectrogram();
#if QT_VERSION < 0x050000
   d_spectrogram->setData( US_SpectrogramData() );
   d_spectrogram->setColorMap( *colormap );
   US_SpectrogramData& spec_dat = (US_SpectrogramData&)d_spectrogram->data();
#else
   US_SpectrogramData* rdata = new US_SpectrogramData();
   d_spectrogram->setData( rdata );
//   d_spectrogram->setColorMap( (QwtColorMap*)colormap );
   d_spectrogram->setColorMap( ColorMapCopy( colormap ) );
   US_SpectrogramData& spec_dat = (US_SpectrogramData&)*(d_spectrogram->data());
#endif
   QwtDoubleRect drect;

   if ( auto_sxy )
      drect = QwtDoubleRect( 0.0, 0.0, 0.0, 0.0 );

   else
   {
      drect = QwtDoubleRect( plt_smin, plt_kmin,
            ( plt_smax - plt_smin ), ( plt_kmax - plt_kmin ) );
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
         QList< S_Solute >* sol_z  = zpcent ? &tsys->sk_distro_zp
                                            : &tsys->sk_distro;

         for ( int jj = 0; jj < sol_z->size(); jj++ )
         {
            double zval = sol_z->at( jj ).c;
            plt_zmin    = qMin( plt_zmin, zval );
            plt_zmax    = qMax( plt_zmax, zval );
         }
      }
   }

   spec_dat.setRastRanges( xreso, yreso, resolu, zfloor, drect );
   spec_dat.setZRange( 0.0, plt_zmax );
   spec_dat.setRaster( sol_d );

   d_spectrogram->attach( data_plot );

   // set color map and axis settings
   QwtScaleWidget *rightAxis = data_plot->axisWidget( QwtPlot::yRight );
   rightAxis->setColorBarEnabled( true );

   xa_title    = anno_title( plot_x );
   ya_title    = anno_title( plot_y );
   data_plot->setAxisTitle( QwtPlot::xBottom, xa_title );
   data_plot->setAxisTitle( QwtPlot::yLeft,   ya_title );

   if ( auto_sxy )
   { // Auto scale x and y
      data_plot->setAxisAutoScale( QwtPlot::yLeft   );
      data_plot->setAxisAutoScale( QwtPlot::xBottom );
   }
   else
   { // Manual limits on x and y
      double lStep = data_plot->axisStepSize( QwtPlot::yLeft   );
      double bStep = data_plot->axisStepSize( QwtPlot::xBottom );
      data_plot->setAxisScale( QwtPlot::xBottom, plt_smin, plt_smax, bStep );
      data_plot->setAxisScale( QwtPlot::yLeft,   plt_kmin, plt_kmax, lStep );
   }

#if QT_VERSION < 0x050000
   rightAxis->setColorMap( QwtDoubleInterval( 0.0, plt_zmax ),
      d_spectrogram->colorMap() );
#else
//   rightAxis->setColorMap( QwtInterval( plt_zmin, plt_zmax ),
//      (QwtColorMap*)d_spectrogram->colorMap() );
   rightAxis->setColorMap( QwtInterval( 0.0, plt_zmax ),
                           ColorMapCopy( colormap ) );
#endif
//   data_plot->setAxisScale( QwtPlot::yRight,  plt_zmin, plt_zmax );
   data_plot->setAxisScale( QwtPlot::yRight,  0.0, plt_zmax );

   data_plot->replot();

   //QString dtext = te_distr_info->toPlainText().section( "\n", 0, 1 );
   QString dtext  = tr( "Run:  " ) + tsys->run_name
         + " (" + tsys->method + ")\n    " + tsys->analys_name;

   bool sv_plot = ck_savepl->isChecked()  &&
                  ( ( looping  &&  !ck_conloop->isChecked() ) || !looping );
DbgLv(2) << "(1) sv_plot" << sv_plot << "looping" << looping;

DbgLv(2) << "(3)   need_save sv_plot" << need_save << sv_plot;
   //if ( need_save  &&  sv_plot )
   if ( sv_plot )
   {  // Automatically save plot image in a PNG file
      const QString s_attrs[] = { "s", "ff0", "MW", "vbar", "D", "f" };
#if QT_VERSION > 0x050000
      QPixmap plotmap = ((QWidget*)data_plot)->grab();
#else
      QSize pmsize    = data_plot->size();
      QPixmap plotmap = QPixmap::grabWidget( data_plot, 0, 0,
                                             pmsize.width(), pmsize.height() );
#endif

      QString runid   = tsys->run_name.section( ".",  0, -2 );
      QString triple  = tsys->run_name.section( ".", -1, -1 );
      QString report  = QString( "pseudo3d_" ) + s_attrs[ plot_x ]
         + "_" + s_attrs[ plot_y ];

      QString ofdir   = US_Settings::reportDir() + "/" + runid;
      QDir dirof( ofdir );
      if ( !dirof.exists( ) )
         QDir( US_Settings::reportDir() ).mkdir( runid );
      QString ofname = tsys->method + "." + triple + "." + report + ".png";
      QString ofpath = ofdir + "/" + ofname;

      plotmap.save( ofpath );
      dtext          = dtext + tr( "\nPLOT %1 SAVED to local" )
         .arg( curr_distr + 1 );

      if ( dkdb_cntrls->db()  &&  !ck_locsave->isChecked() )
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
DbgLv(1) << "upd_curr_distr" << curr_distr;

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

   build_xy_distro();

   set_limits();

   plot_data();
}

void US_Pseudo3D_Combine::update_plot_smin( double dval )
{
   plt_smin = dval;
DbgLv(1) << "plt_smin" << plt_smin;
}

void US_Pseudo3D_Combine::update_plot_smax( double dval )
{
   plt_smax = dval;
DbgLv(1) << "plt_smax" << plt_smax;
}

void US_Pseudo3D_Combine::update_plot_kmin( double dval )
{
   plt_kmin = dval;
}

void US_Pseudo3D_Combine::update_plot_kmax( double dval )
{
   plt_kmax = dval;
}

void US_Pseudo3D_Combine::select_autosxy()
{
   auto_sxy   = ck_autosxy->isChecked();
   ct_plt_kmin->setEnabled( !auto_sxy );
   ct_plt_kmax->setEnabled( !auto_sxy );
   ct_plt_smin->setEnabled( !auto_sxy );
   ct_plt_smax->setEnabled( !auto_sxy );

   set_limits();
}

void US_Pseudo3D_Combine::select_autoscz()
{
   auto_scz   = ck_autoscz->isChecked();

   set_limits();
}

void US_Pseudo3D_Combine::select_conloop()
{
   cont_loop  = ck_conloop->isChecked();
   DisSys* tsys   = (DisSys*)&system.at( curr_distr );
   QString dtext  = tr( "Run:  " ) + tsys->run_name
         + " (" + tsys->method + ")\n    " + tsys->analys_name;

   if ( cont_loop )
   {
      pb_pltall->setText( tr( "Plot All Distros in a Loop" ) );
      dtext          = dtext +
         tr( "\nWith continuous loop, plot files are not saved." );
   }
   else
      pb_pltall->setText( tr( "Plot All Distros" ) );

   te_distr_info->setText( dtext );
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
   
   need_save  = false;

   for ( int jj = 0; jj < models.count(); jj++ )
   {  // load each selected distribution model
      load_distro( models[ jj ], mdescs[ jj ] );
   }

   curr_distr = system.size() - 1;
   need_save  = ck_savepl->isChecked()  &&  !cont_loop;
   ct_curr_distr->setEnabled( true );
   ct_curr_distr->setValue( curr_distr + 1 );
   pb_rmvdist->setEnabled( models.count() > 0 );

   update_curr_distr( (double)system.size() );
}

//Modified copy for GMP
void US_Pseudo3D_Combine::load_distro_auto( QString invID_passed, QStringList m_t_r_id )
{
   // get a model description or set of descriptions for distribution data
   QList< US_Model > models;
   QStringList       mdescs;
   bool              loadDB = dkdb_cntrls->db();

   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
   
   US_ModelLoader dialog( true, mfilter, models, mdescs, pfilts, invID_passed );

   dialog. accepted_auto ( m_t_r_id );

   qDebug() << "In load_distro(): mdescs -- " << mdescs;
   
   need_save  = false;
   QApplication::restoreOverrideCursor();
   
   for ( int jj = 0; jj < models.count(); jj++ )
   {  // load each selected distribution model
      load_distro( models[ jj ], mdescs[ jj ] );
   }

   curr_distr = system.size() - 1;
   need_save  = ck_savepl->isChecked()  &&  !cont_loop;
   ct_curr_distr->setEnabled( true );
   ct_curr_distr->setValue( curr_distr + 1 );
   pb_rmvdist->setEnabled( models.count() > 0 );

   update_curr_distr( (double)system.size() );
}

//return pointer to data_plot
QwtPlot* US_Pseudo3D_Combine::rp_data_plot()
{
  return data_plot;
}

void US_Pseudo3D_Combine::load_distro( US_Model model, QString mdescr )
{
   DisSys      tsys;
   S_Solute    sol_sk;
   S_Solute    sol_xy;

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
   if ( model.global == US_Model::GLOBAL )
   {
      tsys.method       = QString( mdesc ).section( '.', 2, 2 )
                                          .section( '_', 2, 2 );
   }
   tsys.editGUID     = model.editGUID;
   tsys.plot_x       = plot_x;
   tsys.plot_y       = plot_y;
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
   int nsolmc  = model.components.size();

   // read in and set distribution s,k,c,... values
   if ( tsys.distro_type != (int)US_Model::COFS )
   {
      double tot_conc = 0.0;

      for ( int jj = 0; jj < nsolmc; jj++ )
      {
         sol_sk.s  = model.components[ jj ].s * 1.0e13;
         sol_sk.k  = model.components[ jj ].f_f0;
         sol_sk.c  = model.components[ jj ].signal_concentration;
         sol_sk.w  = model.components[ jj ].mw;
         sol_sk.v  = model.components[ jj ].vbar20;
         sol_sk.d  = model.components[ jj ].D * 1.0e7;
         sol_sk.f  = model.components[ jj ].f;

         sol_xy    = sol_sk;
         sol_xy.s  = ( plot_x == ATTR_S ) ? sol_sk.s : sol_xy.s;
         sol_xy.s  = ( plot_x == ATTR_K ) ? sol_sk.k : sol_xy.s;
         sol_xy.s  = ( plot_x == ATTR_W ) ? sol_sk.w : sol_xy.s;
         sol_xy.s  = ( plot_x == ATTR_V ) ? sol_sk.v : sol_xy.s;
         sol_xy.s  = ( plot_x == ATTR_D ) ? sol_sk.d : sol_xy.s;
         sol_xy.s  = ( plot_x == ATTR_F ) ? sol_sk.f : sol_xy.s;
         sol_xy.k  = ( plot_y == ATTR_S ) ? sol_sk.s : sol_xy.k;
         sol_xy.k  = ( plot_y == ATTR_K ) ? sol_sk.k : sol_xy.k;
         sol_xy.k  = ( plot_y == ATTR_W ) ? sol_sk.w : sol_xy.k;
         sol_xy.k  = ( plot_y == ATTR_V ) ? sol_sk.v : sol_xy.k;
         sol_xy.k  = ( plot_y == ATTR_D ) ? sol_sk.d : sol_xy.k;
         sol_xy.k  = ( plot_y == ATTR_F ) ? sol_sk.f : sol_xy.k;

         tsys.sk_distro << sol_sk;
         tsys.xy_distro << sol_xy;

         plt_zmin_co = qMin( plt_zmin_co, sol_sk.c );
         plt_zmax_co = qMax( plt_zmax_co, sol_sk.c );
         tot_conc   += sol_sk.c;
      }
DbgLv(1) << "LD: zmin zmax totconc" << plt_zmin_co << plt_zmax_co << tot_conc;

      // sort and reduce distributions
      sort_distro( tsys.sk_distro, false );
      sort_distro( tsys.xy_distro, true  );
      int nsolsk = tsys.sk_distro.size();
      int nsolxy = tsys.xy_distro.size();
DbgLv(1) << "LD: nsolsk nsolxy nsolmc" << nsolsk << nsolxy << nsolmc;
      tsys.sk_distro_zp.clear();
      tsys.xy_distro_zp.clear();

      // Create Z-as-percentage version of distributions

      for ( int jj = 0; jj < nsolmc; jj++ )
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

         if ( jj < nsolxy )
         {
            sol_xy      = tsys.xy_distro[ jj ];
            cozpc       = sol_xy.c * 100.0 / tot_conc;
            sol_xy.c    = cozpc;
            plt_zmin_zp = qMin( plt_zmin_zp, cozpc );
            plt_zmax_zp = qMax( plt_zmax_zp, cozpc );
            tsys.xy_distro_zp << sol_xy;
         }
      }
DbgLv(1) << "LD: zminzp zmaxzp" << plt_zmin_zp << plt_zmax_zp;
   }

   // update current distribution record
   system.append( tsys );
   int jd     = system.size();
   curr_distr = jd - 1;
   ct_curr_distr->setRange( 1.0, jd );
   ct_curr_distr->setSingleStep( 1.0 );
   ct_curr_distr->setValue( jd );
   ct_curr_distr->setEnabled( true );

   if ( auto_sxy )
   {
DbgLv(1) << "LD:  auto_sxy call set_limits";
      set_limits();
DbgLv(1) << "LD:  auto_sxy  rtn fr set_limits";
      ct_plt_kmin->setEnabled( false );
      ct_plt_kmax->setEnabled( false );
      ct_plt_smin->setEnabled( false );
      ct_plt_smax->setEnabled( false );
   }
   else
   {
      plt_smin    = ct_plt_smin->value();
      plt_smax    = ct_plt_smax->value();
      plt_kmin    = ct_plt_kmin->value();
      plt_kmax    = ct_plt_kmax->value();
DbgLv(1) << "LD:  non-auto_sxy call set_limits";
      set_limits();
DbgLv(1) << "LD:  non-auto_sxy  rtn fr set_limits";
   }
   data_plot->setAxisScale( QwtPlot::xBottom, plt_smin, plt_smax );
   data_plot->setAxisScale( QwtPlot::yLeft,   plt_kmin, plt_kmax );

   pb_pltall ->setEnabled( true );
   pb_refresh->setEnabled( true );
   pb_reset  ->setEnabled( true );

   if ( cont_loop )
      pb_pltall->setText( tr( "Plot All Distros in a Loop" ) );
   else
      pb_pltall->setText( tr( "Plot All Distros" ) );
DbgLv(1) << "LD: RETURN";
}

void US_Pseudo3D_Combine::load_color()
{
   QString filter = tr( "Color Map files (*cm-*.xml);;" )
         + tr( "Any XML files (*.xml);;" )
         + tr( "Any files (*)" );

   // get an xml file name for the color map
   QString fname = QFileDialog::getOpenFileName( this,
      tr( "Load Color Map File" ),
      US_Settings::etcDir(), filter, 0, 0 );

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

   plot_data();
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

   need_save  = ck_savepl->isChecked()  &&  !cont_loop;
}

// Stop the distros-plotting loop
void US_Pseudo3D_Combine::stop()
{
   looping    = false;
   need_save  = ck_savepl->isChecked()  &&  !cont_loop;
}

void US_Pseudo3D_Combine::set_limits()
{
   double smin = 1.0e30;
   double smax = -1.0e30;
   double kmin = 1.0e30;
   double kmax = -1.0e30;
   double sinc;
   double kinc;
   xa_title    = anno_title( plot_x );
   ya_title    = anno_title( plot_y );

   data_plot->setAxisTitle( QwtPlot::xBottom, xa_title );
   data_plot->setAxisTitle( QwtPlot::yLeft,   ya_title );

   if ( system.size() < 1 )
      return;

   // find min,max for X,Y distributions
   for ( int ii = 0; ii < system.size(); ii++ )
   {
      DisSys* tsys = (DisSys*)&system.at( ii );

      for ( int jj = 0; jj < tsys->xy_distro.size(); jj++ )
      {
         double sval = tsys->xy_distro.at( jj ).s;
         double kval = tsys->xy_distro.at( jj ).k;
         smin        = qMin( smin, sval );
         smax        = qMax( smax, sval );
         kmin        = qMin( kmin, kval );
         kmax        = qMax( kmax, kval );
      }
   }

   // adjust minima, maxima
   sinc      = ( smax - smin ) / 10.0;
   kinc      = ( kmax - kmin ) / 10.0;
   sinc      = ( sinc <= 0.0 ) ? ( smin * 0.05 ) : sinc;
   kinc      = ( kinc <= 0.0 ) ? ( kmin * 0.05 ) : kinc;
DbgLv(1) << "SL: real smin smax kmin kmax" << smin << smax << kmin << kmax;
   smin     -= sinc;
   smax     += sinc;
   kmin     -= kinc;
   kmax     += kinc;
DbgLv(1) << "SL: adjusted smin smax kmin kmax" << smin << smax << kmin << kmax;

   if ( auto_sxy )
   {  // Set auto limits on X and Y
      sinc        = pow( 10.0, qFloor( log10( smax ) ) - 3.0 );
      kinc        = pow( 10.0, qFloor( log10( kmax ) ) - 3.0 );
      if ( qAbs( ( smax - smin ) / smax ) < 0.001 )
      {  // Put padding around virtually constant value
         smin     -= sinc;
         smax     += sinc;
      }
      if ( qAbs( ( kmax - kmin ) / kmax ) < 0.001 )
      {  // Put padding around virtually constant value
         kmin     -= kinc;
         kmax     += kinc;
      }
      // Make sure limits are nearest reasonable values
      smin        = qFloor( smin / sinc ) * sinc;
      smax        = qFloor( smax / sinc ) * sinc + sinc;
      smin        = ( plot_x != ATTR_S ) ? qMax( smin, 0.0 ) : smin;
      smin        = ( plot_x == ATTR_K ) ? qMax( smin, 0.5 ) : smin;
      kmin        = qFloor( kmin / kinc ) * kinc;
      kmax        = qFloor( kmax / kinc ) * kinc + kinc;

DbgLv(1) << "SL: setVal kmin kmax" << kmin << kmax;
      ct_plt_smin->setValue( smin );
      ct_plt_smax->setValue( smax );
      ct_plt_kmin->setValue( kmin );
      ct_plt_kmax->setValue( kmax );

      plt_smin    = smin;
      plt_smax    = smax;
      plt_kmin    = kmin;
      plt_kmax    = kmax;
   }
   else
   {
      plt_smin    = ct_plt_smin->value();
      plt_smax    = ct_plt_smax->value();
      plt_kmin    = ct_plt_kmin->value();
      plt_kmax    = ct_plt_kmax->value();
   }
DbgLv(1) << "SL: plt_smin _smax _kmin _kmax" << plt_smin << plt_smax
 << plt_kmin << plt_kmax;
}

// Sort distribution solute list by s,k values and optionally reduce
void US_Pseudo3D_Combine::sort_distro( QList< S_Solute >& listsols,
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
      S_Solute sol1;
      S_Solute sol2;
      QList< S_Solute > reduced;
      QList< S_Solute >::iterator jj = listsols.begin();
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
      need_save  = ck_savepl->isChecked()  &&  !cont_loop;
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
   QString pfmsg;
   int nruns  = 0;
   pfilts.clear();

   US_SelectRuns srdiag( dkdb_cntrls->db(), pfilts );
   srdiag.move( this->pos() + QPoint( 200, 200 ) );
   connect( &srdiag, SIGNAL( dkdb_changed  ( bool ) ),
            this,    SLOT  ( update_disk_db( bool ) ) );

   if ( srdiag.exec() == QDialog::Accepted )
      nruns         = pfilts.size();
   else
      pfilts.clear();

   if ( nruns == 0 )
      pfmsg = tr( "(no prefilter)" );

   else if ( nruns > 1 )
      pfmsg = tr( "RunID prefilter - %1 runs: " ).arg( nruns )
              + QString( pfilts[ 0 ] ).left( 20 ) + "*, ...";

   else
      pfmsg = tr( "RunID prefilter - 1 run: " )
              + QString( pfilts[ 0 ] ).left( 20 ) + " ...";

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
      ct_curr_distr->setRange( 1, jd );
      ct_curr_distr->setSingleStep( 1.0 );
      ct_curr_distr->setValue( 1 );
      ct_curr_distr->setEnabled( true );
   }

   plot_data();
}

// Select coordinate for horizontal axis: copy for GMP
void US_Pseudo3D_Combine::select_x_axis_auto( int ival )
{
  select_x_axis( ival );
}

// Select coordinate for vertical axis: copy for GMP
void US_Pseudo3D_Combine::select_y_axis_auto( int ival )
{
  select_y_axis( ival );
}

// Select coordinate for horizontal axis
void US_Pseudo3D_Combine::select_x_axis( int ival )
{
   const QString xlabs[] = {      "s", "f/f0",  "MW", "vbar", "D", "f"  };
   const double  xvlos[] = {      1.0,   1.0,   2e+4,  0.60, 1e-8, 1e-8 };
   const double  xvhis[] = {     10.0,   4.0,   1e+5,  0.80, 1e-7, 1e-7 };
   const double  xmins[] = { -100000.0,  1.0,    0.0,  0.01, 1e-9, 1e-9 };
   const double  xmaxs[] = {  100000.0, 50.0,  1e+10,  3.00, 1e-5, 1e-5 };
   const double  xincs[] = {     0.01,  0.01, 1000.0,  0.01, 1e-9, 1e-9 };

   plot_x     = ival;

   qDebug() << "Pseudo3D: x_axis changed: ival,  xlabs[ plot_x ] -- " << ival <<  xlabs[ plot_x ];

   lb_plt_smin->setText( tr( "Plot Limit " ) + xlabs[ plot_x ]
                       + tr( " Minimum:" ) );
   lb_plt_smax->setText( tr( "Plot Limit " ) + xlabs[ plot_x ]
                       + tr( " Maximum:" ) );
   ct_plt_smin->setRange( xmins[ plot_x ], xmaxs[ plot_x ] );
   ct_plt_smax->setRange( xmins[ plot_x ], xmaxs[ plot_x ] );
   ct_plt_smin->setSingleStep( xincs[ plot_x ] );
   ct_plt_smax->setSingleStep( xincs[ plot_x ] );
   ct_plt_smin->setValue( xvlos[ plot_x ] );
   ct_plt_smax->setValue( xvhis[ plot_x ] );

   rb_y_s   ->setEnabled( plot_x != ATTR_S );
   rb_y_ff0 ->setEnabled( plot_x != ATTR_K );
   rb_y_mw  ->setEnabled( plot_x != ATTR_W );
   rb_y_vbar->setEnabled( plot_x != ATTR_V );
   rb_y_D   ->setEnabled( plot_x != ATTR_D );
   rb_y_f   ->setEnabled( plot_x != ATTR_F );

   build_xy_distro();

   set_limits();

   plot_data();
}

// Select coordinate for vertical axis
void US_Pseudo3D_Combine::select_y_axis( int ival )
{
   const QString ylabs[] = {      "s", "f/f0",  "MW", "vbar",    "D", "f"  };
   const double  yvlos[] = {      1.0,   1.0,   2e+4,  0.60,     0.0, 1e-8 };
   const double  yvhis[] = {     10.0,   4.0,   1e+5,  0.80,    30.0, 1e-7 };
   const double  ymins[] = { -100000.0,  1.0,    0.0,  0.01,     0.0, 1e-9 };
   const double  ymaxs[] = {  100000.0, 50.0,  1e+10,  3.00,100000.0, 1e-5 };
   const double  yincs[] = {     0.01,  0.01, 1000.0,  0.01,    0.01, 1e-9 };

   plot_y     = ival;
qDebug() << "select-y: plot_y" << plot_y;

   qDebug() << "Pseudo3D: y_axis changed: ival,  ylabs[ plot_y ] -- " << ival <<  ylabs[ plot_y ];

   lb_plt_kmin->setText( tr( "Plot Limit " ) + ylabs[ plot_y ]
                       + tr( " Minimum:" ) );
   lb_plt_kmax->setText( tr( "Plot Limit " ) + ylabs[ plot_y ]
                       + tr( " Maximum:" ) );
qDebug() << "  ylab" << ylabs[plot_y];
   ct_plt_kmin->setRange( ymins[ plot_y ], ymaxs[ plot_y ] );
   ct_plt_kmax->setRange( ymins[ plot_y ], ymaxs[ plot_y ] );
   ct_plt_kmin->setSingleStep( yincs[ plot_y ] );
   ct_plt_kmax->setSingleStep( yincs[ plot_y ] );
   ct_plt_kmin->setValue( yvlos[ plot_y ] );
   ct_plt_kmax->setValue( yvhis[ plot_y ] );
qDebug() << "  yval-lo val-hi" << yvlos[plot_y] << yvhis[plot_y];

   rb_x_s   ->setEnabled( plot_y != ATTR_S );
   rb_x_ff0 ->setEnabled( plot_y != ATTR_K );
   rb_x_mw  ->setEnabled( plot_y != ATTR_W );
   rb_x_vbar->setEnabled( plot_y != ATTR_V );
   rb_x_D   ->setEnabled( plot_y != ATTR_D );
   rb_x_f   ->setEnabled( plot_y != ATTR_F );

   build_xy_distro();

   set_limits();

   plot_data();
}

// Re-generate the XY version of the current distribution
void US_Pseudo3D_Combine::build_xy_distro()
{
   if ( system.size() < 1 )
      return;
   DisSys* tsys     = (DisSys*)&system.at( curr_distr );
   if ( tsys->plot_x == plot_x  && tsys->plot_y == plot_y )
      return;

   tsys->xy_distro.clear();
   tsys->plot_x     = plot_x;
   tsys->plot_y     = plot_y;
   double tot_conc  = 0.0;
   int    nsolsk    = tsys->sk_distro.size();

   // Create solute list with specified x,y
   for ( int ii = 0; ii < nsolsk; ii++ )
   {
      S_Solute sol_sk  = tsys->sk_distro[ ii ];
      S_Solute sol_xy  = sol_sk;

      sol_xy.s  = ( plot_x == ATTR_S ) ? sol_sk.s : sol_xy.s;
      sol_xy.s  = ( plot_x == ATTR_K ) ? sol_sk.k : sol_xy.s;
      sol_xy.s  = ( plot_x == ATTR_W ) ? sol_sk.w : sol_xy.s;
      sol_xy.s  = ( plot_x == ATTR_V ) ? sol_sk.v : sol_xy.s;
      sol_xy.s  = ( plot_x == ATTR_D ) ? sol_sk.d : sol_xy.s;
      sol_xy.s  = ( plot_x == ATTR_F ) ? sol_sk.f : sol_xy.s;
      sol_xy.k  = ( plot_y == ATTR_S ) ? sol_sk.s : sol_xy.k;
      sol_xy.k  = ( plot_y == ATTR_K ) ? sol_sk.k : sol_xy.k;
      sol_xy.k  = ( plot_y == ATTR_W ) ? sol_sk.w : sol_xy.k;
      sol_xy.k  = ( plot_y == ATTR_V ) ? sol_sk.v : sol_xy.k;
      sol_xy.k  = ( plot_y == ATTR_D ) ? sol_sk.d : sol_xy.k;
      sol_xy.k  = ( plot_y == ATTR_F ) ? sol_sk.f : sol_xy.k;
      tot_conc += sol_sk.c;
      tsys->xy_distro << sol_xy;
   }

   // Sort and possibly reduce XY distro
   sort_distro( tsys->xy_distro, true );

   // Create Z-as-percentage version of xy distribution
   int    nsolxy = tsys->xy_distro.size();
DbgLv(1) << "Bld: nsolsk nsolxy" << nsolsk << nsolxy;
   tsys->xy_distro_zp.clear();

   for ( int ii = 0; ii < nsolxy; ii++ )
   {
      S_Solute sol_xy  = tsys->xy_distro[ ii ];
      double cozpc     = sol_xy.c * 100.0 / tot_conc;
      sol_xy.c         = cozpc;
      plt_zmin_zp      = qMin( plt_zmin_zp, cozpc );
      plt_zmax_zp      = qMax( plt_zmax_zp, cozpc );

      tsys->xy_distro_zp << sol_xy;
   }
}

// Set annotation title for a plot index
QString US_Pseudo3D_Combine::anno_title( int pltndx )
{
   QString a_title;

   if      ( pltndx == ATTR_S )
      a_title  = tr( "Sedimentation Coefficient (1e-13)"
                     " for water at 20" ) + DEGC;
   else if ( pltndx == ATTR_K )
      a_title  = tr( "Frictional Ratio f/f0" );
   else if ( pltndx == ATTR_W )
      a_title  = tr( "Molecular Weight (Dalton)" );
   else if ( pltndx == ATTR_V )
      a_title  = tr( "Vbar at 20" ) + DEGC;
   else if ( pltndx == ATTR_D )
      a_title  = tr( "Diffusion Coefficient (1e-7)" );
   else if ( pltndx == ATTR_F )
      a_title  = tr( "Frictional Coefficient" );

   return a_title;
}

// Make a ColorMap copy and return a pointer to the new ColorMap
QwtLinearColorMap* US_Pseudo3D_Combine::ColorMapCopy( QwtLinearColorMap* colormap )
{
   QVector< double >  cstops   = colormap->colorStops();
   int                lstop    = cstops.count() - 1;
   QwtInterval        csvals( 0.0, 1.0 );
   QwtLinearColorMap* cmapcopy = new QwtLinearColorMap( colormap->color1(),
                                                        colormap->color2() );

   for ( int jj = 1; jj < lstop; jj++ )
   {
      QColor scolor = colormap->color( csvals, cstops[ jj ] );
      cmapcopy->addColorStop( cstops[ jj ], scolor );
   }

   return cmapcopy;
}

