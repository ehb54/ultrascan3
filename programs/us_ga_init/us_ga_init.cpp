//! \file us_ga_init.cpp

#include <QApplication>
#include <qchar.h>
#include <qglobal.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qnamespace.h>
#include <qpen.h>
#include <qvector.h>
#include <QSignalBlocker>
#include <qwt_plot_spectrogram.h>

#include "us_ga_init.h"
#include "us_select_runs.h"
#include "us_model_loader.h"
#include "us_license_t.h"
#include "us_license.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_gui_util.h"
#include "us_math2.h"
#include "us_matrix.h"
#include "us_constants.h"
#include "us_passwd.h"
#include "us_report.h"
#include "qwt_picker_machine.h"
#include "us_solutedata.h"
#define canvasBackground() canvasBackground().color();

// main program
int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );

   #include "main1.inc"

   // License is OK.  Start up.
   
   US_GA_Initialize w;
   w.show();                   //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication
}

// LessThan method for S_Solute sort
bool distro_lessthan( const S_Solute &solu1, const S_Solute &solu2 )
{  // TRUE iff  (s1<s2) || (s1==s2 && k1<k2)
   return ( solu1.s < solu2.s ) ||
          ( ( solu1.s == solu2.s ) && ( solu1.k < solu2.k ) );
}

// US_GA_Initialize class constructor
US_GA_Initialize::US_GA_Initialize() : US_Widgets()
{
   // set up the GUI

   setWindowTitle( tr( "Genetic Algorithm Initialization Control Window" ) );
   setPalette( US_GuiSettings::frameColor() );

   // primary layouts
   QHBoxLayout* main  = new QHBoxLayout( this );
   QVBoxLayout* left  = new QVBoxLayout();
   QVBoxLayout* rght  = new QVBoxLayout();
   QGridLayout* spec  = new QGridLayout();
   main->setSpacing        ( 2 );
   main->setContentsMargins( 2, 2, 2, 2 );
   left->setSpacing        ( 0 );
   left->setContentsMargins( 0, 1, 0, 1 );
   spec->setSpacing        ( 1 );
   spec->setContentsMargins( 0, 0, 0, 0 );
   rght->setSpacing        ( 0 );
   rght->setContentsMargins( 0, 1, 0, 1 );

   dbg_level = US_Settings::us_debug();

   // Insure working etc is populated with color maps
   clean_etc_dir( true );

   // series of rows: most of them label on left, counter/box on right
   lb_info1      = us_banner( tr( "Genetic Algorithm Controls" ) );

   lb_wxbuck     = us_label( tr( "Width of s Bucket:" ) );
   lb_wxbuck->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );

   ct_wxbuck     = us_counter( 3, 0.0, 10.0, 0.0 );
   ct_wxbuck->setSingleStep( 1 );

   lb_hybuck     = us_label( tr( "Height of f/f0 Bucket:" ) );
   lb_hybuck->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );

   ct_hybuck     = us_counter( 3, 0.0, 1.0, 0.0 );
   ct_hybuck->setSingleStep( 1 );

   lb_info2      = us_banner( tr( "Pseudo-3D Controls" ) );

   lb_resolu     = us_label( tr( "Pseudo-3D Resolution:" ) );
   lb_resolu->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );

   ct_resolu     = us_counter( 3, 0.0, 100.0, 90.0 );
   ct_resolu->setSingleStep( 1 );
   connect( ct_resolu, &QwtCounter::valueChanged,
            this,      &US_GA_Initialize::update_resolu );

   lb_xreso      = us_label( tr( "X Resolution:" ) );
   lb_xreso->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );

   ct_xreso      = us_counter( 3, 10.0, 1000.0, 0.0 );
   ct_xreso->setSingleStep( 1 );
   connect( ct_xreso,  &QwtCounter::valueChanged,
            this,      &US_GA_Initialize::update_xreso );

   lb_yreso      = us_label( tr( "Y Resolution:" ) );
   lb_yreso->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );

   ct_yreso      = us_counter( 3, 10.0, 1000.0, 0.0 );
   ct_yreso->setSingleStep( 1 );
   connect( ct_yreso,  &QwtCounter::valueChanged,
            this,      &US_GA_Initialize::update_yreso );

   lb_zfloor     = us_label( tr( "Z Visibility Percent:" ) );
   lb_zfloor->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );

   ct_zfloor     = us_counter( 3, 50.0, 150.0, 1.0 );
   ct_zfloor->setSingleStep( 1 );
   connect( ct_zfloor, &QwtCounter::valueChanged,
            this,      &US_GA_Initialize::update_zfloor );

   us_checkbox( tr( "Manually Set the X and Y Limits" ), ck_manlim, true );
   ck_manlim->setStyleSheet( "QCheckBox {background-color: white;}" );
   connect( ck_manlim, &QAbstractButton::clicked,
            this,      &US_GA_Initialize::select_manlim );

   lb_plxmin     = us_label( tr( "Min. s [S]:" ) );
   lb_plxmin->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
   le_plxmin     = us_lineedit();
   connect( le_plxmin, &QLineEdit::returnPressed,
            this,      &US_GA_Initialize::update_plxmin );

   lb_plxmax     = us_label( tr( "Max. s [S]:" ) );
   lb_plxmax->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
   le_plxmax= us_lineedit();
   connect( le_plxmax, &QLineEdit::returnPressed,
            this,      &US_GA_Initialize::update_plxmax );

   lb_plymin     = us_label( tr( "Min. f/f0:" ) );
   lb_plymin->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
   le_plymin     = us_lineedit();
   connect( le_plymin, &QLineEdit::returnPressed,
            this,      &US_GA_Initialize::update_plymin );
   
   lb_plymax     = us_label( tr( "Max. f/f0:" ) );
   lb_plymax->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
   le_plymax     = us_lineedit();
   connect( le_plymax, &QLineEdit::returnPressed,
            this,      &US_GA_Initialize::update_plymax );

           attr_x      = 0;
           attr_y      = 1;
           attr_z      = 3;
   QLabel* lb_x_axis   = us_label( tr( "X Axis:" ) );
   QLabel* lb_y_axis   = us_label( tr( "Y Axis:" ) );
           bg_x_axis   = new QButtonGroup( this );
           bg_y_axis   = new QButtonGroup( this );
   QGridLayout* gl_x_s    = us_radiobutton( tr( "s"    ), rb_x_s,    true  );
   QGridLayout* gl_x_ff0  = us_radiobutton( tr( "ff0"  ), rb_x_ff0,  false );
   QGridLayout* gl_x_mw   = us_radiobutton( tr( "mw"   ), rb_x_mw,   false );
   QGridLayout* gl_x_vbar = us_radiobutton( tr( "vbar" ), rb_x_vbar, false );
   QGridLayout* gl_x_D    = us_radiobutton( tr( "D"    ), rb_x_D,    false );
   QGridLayout* gl_x_rh   = us_radiobutton( tr( "Rh"   ), rb_x_rh,   false );
   QGridLayout* gl_y_s    = us_radiobutton( tr( "s"    ), rb_y_s,    false );
   QGridLayout* gl_y_ff0  = us_radiobutton( tr( "ff0"  ), rb_y_ff0,  true  );
   QGridLayout* gl_y_mw   = us_radiobutton( tr( "mw"   ), rb_y_mw,   false );
   QGridLayout* gl_y_vbar = us_radiobutton( tr( "vbar" ), rb_y_vbar, false );
   QGridLayout* gl_y_D    = us_radiobutton( tr( "D"    ), rb_y_D,    false );
   QGridLayout* gl_y_rh   = us_radiobutton( tr( "Rh"   ), rb_y_rh,   false );
   bg_x_axis->addButton( rb_x_s,    ATTR_S );
   bg_x_axis->addButton( rb_x_ff0,  ATTR_K );
   bg_x_axis->addButton( rb_x_mw,   ATTR_W );
   bg_x_axis->addButton( rb_x_vbar, ATTR_V );
   bg_x_axis->addButton( rb_x_D,    ATTR_D );
   bg_x_axis->addButton( rb_x_rh,   ATTR_R );
   bg_y_axis->addButton( rb_y_s,    ATTR_S );
   bg_y_axis->addButton( rb_y_ff0,  ATTR_K );
   bg_y_axis->addButton( rb_y_mw,   ATTR_W );
   bg_y_axis->addButton( rb_y_vbar, ATTR_V );
   bg_y_axis->addButton( rb_y_D,    ATTR_D );
   bg_y_axis->addButton( rb_y_rh,   ATTR_R );
   rb_x_s   ->setChecked( true  );
   rb_y_s   ->setEnabled( false );
   rb_y_ff0 ->setChecked( true  );
   rb_x_ff0 ->setEnabled( false );
   rb_x_s   ->setToolTip( tr( "Set X axis to Sedimentation Coefficient" ) );
   rb_x_ff0 ->setToolTip( tr( "Set X axis to Frictional Ratio"          ) );
   rb_x_mw  ->setToolTip( tr( "Set X axis to Molecular Weight"          ) );
   rb_x_vbar->setToolTip( tr( "Set X axis to Partial Specific Volume"   ) );
   rb_x_D   ->setToolTip( tr( "Set X axis to Diffusion Coefficient"     ) );
   rb_y_s   ->setToolTip( tr( "Set Y axis to Sedimentation Coefficient" ) );
   rb_y_ff0 ->setToolTip( tr( "Set Y axis to Frictional Ratio"          ) );
   rb_y_mw  ->setToolTip( tr( "Set Y axis to Molecular Weight"          ) );
   rb_y_vbar->setToolTip( tr( "Set Y axis to Partial Specific Volume"   ) );
   rb_y_D   ->setToolTip( tr( "Set Y axis to Diffusion Coefficient"     ) );
   connect( bg_x_axis, &QButtonGroup::idReleased, this, &US_GA_Initialize::select_x_axis );
   connect( bg_y_axis, &QButtonGroup::idReleased, this, &US_GA_Initialize::select_y_axis );

   lb_info3      = us_banner( tr( "Bucket List" ) );
   lw_sbin_data  = us_listwidget( );
   connect( lw_sbin_data, &QAbstractItemView::pressed,
            this,         qOverload< const QModelIndex& >( &US_GA_Initialize::sclick_sbdata ) );
   // connect( lw_sbin_data, &QAbstractItemView::doubleClicked,
   //          this,         &US_GA_Initialize::dclick_sbdata );
   connect( lw_sbin_data, &QListWidget::currentRowChanged,
            this,         &US_GA_Initialize::highlight_bucket );

   QLabel* lb_nsols = us_label( tr( "Number Solutes:" ) );
   lb_nsols->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );

   QLabel* lb_nbuks = us_label( tr( "Number Buckets:" ) );
   lb_nsols->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );

   le_nsols = us_lineedit( "", -1, true );
   le_nbuks = us_lineedit( "", -1, true );

   lb_info4      = us_banner( tr( "Data Controls" ) );

   dkdb_cntrls   = new US_Disk_DB_Controls(
         US_Settings::default_data_location() );
   connect( dkdb_cntrls, &US_Disk_DB_Controls::changed,
            this,   &US_GA_Initialize::update_disk_db );

   pb_prefilt    = us_pushbutton( tr( "Select PreFilter" ) );
   connect( pb_prefilt, &QAbstractButton::clicked,
            this,       &US_GA_Initialize::select_prefilt );

   le_prefilt    = us_lineedit  ( "", -1, true );

   pb_lddistr    = us_pushbutton( tr( "Load Distribution" ) );
   connect( pb_lddistr, &QAbstractButton::clicked,
            this,       &US_GA_Initialize::load_distro );

   pb_ldcolor    = us_pushbutton( tr( "Load Color File" ) );
   connect( pb_ldcolor, &QAbstractButton::clicked,
            this,       &US_GA_Initialize::load_color );

   pb_mandrsb    = us_pushbutton( tr( "Manually Draw Bins" ) );
   pb_mandrsb->setCheckable( true );
   
   connect( pb_mandrsb, &QAbstractButton::toggled,
            this,       &US_GA_Initialize::manDrawSb );

   pb_autassb    = us_pushbutton( tr( "Autoassign Solute Bins" ) );
   connect( pb_autassb, &QAbstractButton::clicked,
            this,       &US_GA_Initialize::autoAssignSb );

   pb_resetsb    = us_pushbutton( tr( "Reset Solute Bins" ) );
   connect( pb_resetsb, &QAbstractButton::clicked,
            this,       &US_GA_Initialize::resetSb );

   pb_reset      = us_pushbutton( tr( "Reset" ) );
   connect( pb_reset,   &QAbstractButton::clicked,
            this,       &US_GA_Initialize::reset );

   pb_view       = us_pushbutton( tr( "View Statistics" ) );
   connect( pb_view,    &QAbstractButton::clicked,
            this,       &US_GA_Initialize::view );

   pb_save       = us_pushbutton( tr( "Save" ) );
   connect( pb_save,    &QAbstractButton::clicked,
            this,       &US_GA_Initialize::save );

   pb_loadsb     = us_pushbutton( tr( "Load Solute Bins" ) );
   connect( pb_loadsb,  &QAbstractButton::clicked,
            this,       &US_GA_Initialize::load_bins );

   pb_help       = us_pushbutton( tr( "Help" ) );
   connect( pb_help,    &QAbstractButton::clicked,
            this,       &US_GA_Initialize::help );

   pb_close      = us_pushbutton( tr( "Close" ) );
   connect( pb_close,   &QAbstractButton::clicked,
            this,       &QWidget::close );

   te_status    = us_textedit( );
   te_status->setTextBackgroundColor( Qt::white );
   te_status->setTextColor( Qt::blue );
   dfilname     = "(NONE)";
   stcmline     = tr( "Color Map:  the default w-cyan-magenta-red-black" );
   stdiline     = tr( "The distribution was loaded from the file:" );
   stdfline     = "  " + dfilname;
   stfxline     = tr( "Components fixed attribute is \"vbar\"." );
   stnpline     = tr( "The number of distribution points is 0." );
   te_status->setText( stcmline + "\n" + stdiline + "\n"
         + stdfline + "\n" + stfxline + "\n" + stnpline );

   int s_row = 0;
   spec->addWidget( lb_info1,     s_row++, 0, 1, 8 );
   spec->addWidget( lb_wxbuck,    s_row,   0, 1, 4 );
   spec->addWidget( ct_wxbuck,    s_row++, 4, 1, 4 );
   spec->addWidget( lb_hybuck,    s_row,   0, 1, 4 );
   spec->addWidget( ct_hybuck,    s_row++, 4, 1, 4 );
   spec->addWidget( lb_info2,     s_row++, 0, 1, 8 );
   spec->addWidget( lb_resolu,    s_row,   0, 1, 4 );
   spec->addWidget( ct_resolu,    s_row++, 4, 1, 4 );
   spec->addWidget( lb_xreso,     s_row,   0, 1, 4 );
   spec->addWidget( ct_xreso,     s_row++, 4, 1, 4 );
   spec->addWidget( lb_yreso,     s_row,   0, 1, 4 );
   spec->addWidget( ct_yreso,     s_row++, 4, 1, 4 );
   spec->addWidget( lb_zfloor,    s_row,   0, 1, 4 );
   spec->addWidget( ct_zfloor,    s_row++, 4, 1, 4 );
   spec->addWidget( lb_x_axis,    s_row,   0, 1, 2 );
   spec->addLayout( gl_x_s,       s_row,   2, 1, 1 );
   spec->addLayout( gl_x_ff0,     s_row,   3, 1, 1 );
   spec->addLayout( gl_x_mw,      s_row,   4, 1, 1 );
   spec->addLayout( gl_x_vbar,    s_row,   5, 1, 1 );
   spec->addLayout( gl_x_D,       s_row,   6, 1, 1 );
   spec->addLayout( gl_x_rh,      s_row++, 7, 1, 1 );
   spec->addWidget( lb_y_axis,    s_row,   0, 1, 2 );
   spec->addLayout( gl_y_s,       s_row,   2, 1, 1 );
   spec->addLayout( gl_y_ff0,     s_row,   3, 1, 1 );
   spec->addLayout( gl_y_mw,      s_row,   4, 1, 1 );
   spec->addLayout( gl_y_vbar,    s_row,   5, 1, 1 );
   spec->addLayout( gl_y_D,       s_row,   6, 1, 1 );
   spec->addLayout( gl_y_rh,      s_row++, 7, 1, 1 );
   spec->addWidget( ck_manlim,    s_row++, 0, 1, 8, Qt::AlignCenter );
   spec->addWidget( lb_plxmin,    s_row,   0, 1, 2 );
   spec->addWidget( le_plxmin,    s_row,   2, 1, 2 );
   spec->addWidget( lb_plxmax,    s_row,   4, 1, 2 );
   spec->addWidget( le_plxmax,    s_row++, 6, 1, 2 );
   spec->addWidget( lb_plymin,    s_row,   0, 1, 2 );
   spec->addWidget( le_plymin,    s_row,   2, 1, 2 );
   spec->addWidget( lb_plymax,    s_row,   4, 1, 2 );
   spec->addWidget( le_plymax,    s_row++, 6, 1, 2 );
   spec->addWidget( lb_info3,     s_row++, 0, 1, 8 );
   spec->addWidget( lw_sbin_data, s_row++, 0, 1, 8 );
   spec->addWidget( lb_nsols,     s_row,   0, 1, 2 );
   spec->addWidget( le_nsols,     s_row,   2, 1, 2 );
   spec->addWidget( lb_nbuks,     s_row,   4, 1, 2 );
   spec->addWidget( le_nbuks,     s_row++, 6, 1, 2 );
   spec->addWidget( lb_info4,     s_row++, 0, 1, 8 );
   spec->addLayout( dkdb_cntrls,  s_row++, 0, 1, 8 );
   spec->addWidget( le_prefilt,   s_row++, 0, 1, 8 );
   spec->addWidget( pb_prefilt,   s_row,   0, 1, 4 );
   spec->addWidget( pb_lddistr,   s_row++, 4, 1, 4 );
   spec->addWidget( pb_mandrsb,   s_row,   0, 1, 4 );
   spec->addWidget( pb_autassb,   s_row++, 4, 1, 4 );
   spec->addWidget( pb_resetsb,   s_row,   0, 1, 4 );
   spec->addWidget( pb_loadsb,    s_row++, 4, 1, 4 );
   spec->addWidget( pb_ldcolor,   s_row,   0, 1, 4 );
   spec->addWidget( pb_view,      s_row++, 4, 1, 4 );
   spec->addWidget( pb_reset,     s_row,   0, 1, 4 );
   spec->addWidget( pb_save,      s_row++, 4, 1, 4 );
   spec->addWidget( pb_help,      s_row,   0, 1, 4 );
   spec->addWidget( pb_close,     s_row++, 4, 1, 4 );
   spec->addWidget( te_status,    s_row++, 0, 1, 8 );

   // set up plot component window on right side
   xa_title    = tr( "Sedimentation Coefficient corrected for water at 20" )
      + DEGC;
   ya_title    = tr( "Frictional Ratio f/f0" );

   QBoxLayout* plot = new US_Plot( data_plot, 
      tr( "Distribution Data" ), xa_title, ya_title ); 
   rght->addLayout( plot );
   QBoxLayout* txed = new QHBoxLayout;
   te_pctl_help  = us_textedit( );
   te_pctl_help->setText( tr(
      "Please load a sedimentation coefficient or molecular weight"
      " distribution to initialize the genetic algorithm s-value or mw-value"
      " range. The distribution should have a good resolution over the"
      " sedimentation or weight coefficients. This distribution will be "
      " used to initialize all experiments used in the run, so the"
      " distribution taken from the experiment with the highest speed is"
      " probably the most appropriate distribution. You can use a distribution"
      " from the van Holde - Weischet method, the C(s) method, or 2-D Spectrum"
      " Analysis. You may also load a Monte Carlo distribution." ) );
   us_setReadOnly( te_pctl_help, true );
   txed->addWidget( te_pctl_help );
   rght->addLayout( txed );
   rght->setStretchFactor( plot, 4 );
   rght->setStretchFactor( txed, 1 );

   data_plot->setAutoDelete( true );
   data_plot->setMinimumSize( 600, 600 );

   data_plot->enableAxis( QwtPlot::xBottom, true );
   data_plot->enableAxis( QwtPlot::yLeft,   true );
   data_plot->enableAxis( QwtPlot::yRight,  true );
   data_plot->setAxisScale( QwtPlot::xBottom, 1.0, 40.0 );
   data_plot->setAxisScale( QwtPlot::yLeft,   1.0,  4.0 );

   data_plot->setCanvasBackground( Qt::white );

   // put layouts together for overall layout
   left->addLayout( spec );

   main->addLayout( left );
   main->addLayout( rght );
   main->setStretchFactor( left, 3 );
   main->setStretchFactor( spec, 3 );
   main->setStretchFactor( rght, 5 );

   // set up variables and initial state of GUI
   soludata   = new US_SoluteData();
   attr_x     = 0;          // default X type: s
   attr_y     = 1;          // default Y type: f/f0
   attr_z     = 3;          // default Z (fixed) type:  vbar
   mfilter    = "";         // default model list filter
   runsel     = true;       // default prefilter type
   latest     = true;       // default edit prefilter type
   is_saved   = false;      // files have been saved
   pfilts.clear();          // default prefilter edits list
   binfpath   = US_Settings::resultDir();
   pick       = new US_PlotPicker( data_plot );
   pickpen    = new QPen();
   pick->setStateMachine(  new QwtPickerDragRectMachine() );
   pick->setRubberBandPen( *pickpen );
   pick->setTrackerPen(    *pickpen );
   pick->setRubberBand(    QwtPicker::RectRubberBand );
   connect( pick, qOverload<const QRectF&>( &QwtPlotPicker::selected ),
            this, [this]( const QRectF& region ) {
      getMouseDown( region.topLeft() );
      getMouseUp( region.bottomRight() );
   } );
   
   reset();
}

// reset the GUI
void US_GA_Initialize::reset( void )
{
   pb_mandrsb->setChecked( false );
   manbuks = false;
   sk_distro.clear();
   xy_distro.clear();
   pfilts.clear();
   le_prefilt->clear();
   erase_curves( false );
   resetSb();
   pick->setEnabled( false );
 
   minmax     = false;
   zoom       = false;
   ct_wxbuck->setRange( 0, 200 );
   ct_hybuck->setRange( 0,  50 );
   ct_wxbuck->setSingleStep( 0.1 );
   ct_hybuck->setSingleStep( 0.01 );
   ct_wxbuck->setValue( 0.0 );
   ct_hybuck->setValue( 0.0 );

   resolu     = 90.0;
   ct_resolu->setRange( 1, 100 );
   ct_resolu->setSingleStep( 1 );
   ct_resolu->setValue( resolu );  

   xreso      = 300.0;
   yreso      = 300.0;
   ct_xreso->setRange( 10.0, 1000.0 );
   ct_xreso->setSingleStep( 1.0 );
   ct_xreso->setValue( (double)xreso );
   ct_yreso->setRange( 10, 1000 );
   ct_yreso->setSingleStep( 1.0 );
   ct_yreso->setValue( (double)yreso );

   zfloor     = 100.0;
   ct_zfloor->setRange( 50, 150 );
   ct_zfloor->setSingleStep( 1.0 );
   ct_zfloor->setValue( (double)zfloor );

   attr_x = ATTR_S;
   plxmin = 1.0;
   plxmax = 10.0;
   rb_x_s->setChecked( true );

   attr_y = ATTR_K;
   plymin = 1.0;
   plymax = 4.0;
   rb_y_ff0->setChecked( true );
   select_x_axis( ATTR_S );
   select_y_axis( ATTR_K );
   attr_z = ATTR_V;
   
   le_plxmin->clear();
   le_plxmax->clear();
   le_plymin->clear();
   le_plymax->clear();
   le_plxmin->setReadOnly( true );
   le_plxmax->setReadOnly( true );
   le_plymin->setReadOnly( true );
   le_plymax->setReadOnly( true );

   limit_key.clear();
   manual_limits.clear();
   ck_manlim->setChecked( false );

   update_plot_labels();
   data_plot->setAxisScale( QwtPlot::xBottom, plxmin, plxmax );
   data_plot->setAxisScale( QwtPlot::yLeft,   plymin, plymax );
   data_plot->replot();

   manual_limits.clear();

   le_nsols->clear();
   le_nbuks->clear();

   // default to white-cyan-magenta-red-black color map
   colormap   = new QwtLinearColorMap( Qt::white, Qt::black );
   colormap->addColorStop( 0.10, Qt::cyan );
   colormap->addColorStop( 0.50, Qt::magenta );
   colormap->addColorStop( 0.80, Qt::red );
   cmapname   = tr( "Default Color Map: w-cyan-magenta-red-black" );

   monte_carlo = false;
   is_saved   = false;
   ct_wxbuck->setEnabled( true );
   ct_hybuck->setEnabled( true );
   pb_autassb->setEnabled( true );
}

// save the GA data
void US_GA_Initialize::save( void )
{
   if ( sk_distro.isEmpty() ) {
      return;
   }

   if ( attr_x == ATTR_R || attr_y == ATTR_R || attr_z == ATTR_R )
   {
      QMessageBox::warning( this, "Unsupported GA attribute",
         "Hydrodynamic radius is not supported by the GA boundary file format." );
      return;
   }

   // Sort the buckets in increasing-s order
   soludata->sortBuckets();
   fill_lw_bins();
   plot_buckets();
   is_saved = false;

   // Test for overlaps
   int novlps = soludata->countOverlaps();
DbgLv(1) << "SAVE novlps" << novlps;

   if ( novlps > 0 )
   {
      QString msg = ( novlps == 1 ) ?
         tr( "There is one case of overlap between bins.\n" ) :
         tr( "%1 pairs of bins overlap one another.\n" ).arg( novlps );
      QMessageBox::warning( this,
         tr( "Bin Overlaps" ),
         msg + tr( "You must correct this condition so that\n"
                   "no bins overlap, before you can save GA data." ) );
      return;
   }

   if ( attr_x != ATTR_V  &&  attr_y != ATTR_V  &&  attr_z != ATTR_V )
   {  // None of X,Y,fixed is vbar:  there can be no output file
      QMessageBox::information( this,
         tr( "No Files Output" ),
         tr( "No files will be saved, since neither X,Y nor fixed attribute"
             " is vbar.\n\n"
             "To output a gadistro file, insure that one of X,Y,fixed"
             " is vbar." ) );
      return;
   }

   QString runid = run_name.section( ".", 0, -2 );
   if ( runid.startsWith( "Global-" ) )
      runid         = runid.mid( 7 );
   QString trpid = run_name.section( ".", -1, -1 );
   QString fdir  = US_Settings::resultDir() + "/" + runid;
   QString fndat = "gainit." + trpid + ".gadistro.dat";
   QString fnsta = "gainit." + trpid + ".sol_integ.stats";
   QString fname = fdir + "/" + fndat;
   QString fdir2 = US_Settings::reportDir() + "/" + runid;
   QString fnst2 = method + "." + trpid + ".statistics.rpt";
   QString fnam2 = fdir2 + "/" + fnst2;

   QDir dirp( US_Settings::resultDir() );

   if ( ! dirp.exists( fdir ) )
      dirp.mkpath( fdir );

   if ( ! dirp.exists( fdir2 ) )
      dirp.mkpath( fdir2 );

DbgLv(1) << "SAVE ax,y,z" << attr_x << attr_y << attr_z;
   double fixval = 0.0;
   fixval        = ( attr_z == ATTR_S ) ? sk_distro[ 0 ].s : fixval;
   fixval        = ( attr_z == ATTR_K ) ? sk_distro[ 0 ].k : fixval;
   fixval        = ( attr_z == ATTR_W ) ? sk_distro[ 0 ].w : fixval;
   fixval        = ( attr_z == ATTR_D ) ? sk_distro[ 0 ].d : fixval;

   soludata->saveGAdata( fname, attr_x, attr_y, attr_z, fixval );

   // Report on files saved
   QString msg = tr( "Saved:\n    " ) + fndat + "\n";

   if ( manbuks )
   {  // if manual buckets, build up and analyze data, then report

DbgLv(1) << "SAVE call buildMC";
      soludata->buildDataMC();                     // build it

      fname         = fdir + "/" + fnsta;

DbgLv(1) << "SAVE call reportMC";
      soludata->reportDataMC( fname, mc_iters );   // report it
      is_saved      = true;

      // Copy the statistics file to the report directory
      QFile( fnam2 ).remove();
      QFile( fname ).copy( fnam2 );
      // pb_view   ->setEnabled( true );

      msg     += "    " + fnsta + "\n";
      msg     += tr( "in directory:\n    " ) + fdir + tr( "\n\nand\n" );
      msg     += "    " + fnst2 + "\n";
      msg     += tr( "in directory:\n    " ) + fdir2;

      if ( dkdb_cntrls->db() )
      {  // Write statistics report to database
         US_Passwd   pw;
         US_DB2      db( pw.getPasswd() );
         US_DB2*     dbP = &db;
         QStringList query;

         query << "get_editID" << editGUID;
         db.query( query );
         db.next();
         int         idEdit = db.value( 0 ).toString().toInt();
         US_Report   freport;
         freport.runID      = runid;

         int status =
            freport.saveDocumentFromFile( fdir2, fnst2, dbP, idEdit );

         if ( status == US_Report::REPORT_OK )
            msg  += tr( "\n\nThe report file was also saved to the database" );
         else
            msg  += tr( "\n\n*ERROR* attempting save of report to database." );
      }
   }

   else
   {
DbgLv(1) << "SAVE fnst2" << fnst2;
      msg     += tr( "in directory:\n    " ) + fdir;
   }

   QMessageBox::information( this, tr( "Distro/Stats File Save" ), msg );
}

// Manually draw solute bins
void US_GA_Initialize::manDrawSb( void )
{
   if ( sk_distro.isEmpty() ) {
      pick->setEnabled( false );
      pb_mandrsb->setChecked( false );
      return;
   }

   if ( !pb_mandrsb->isChecked() )
   {
      pick->setEnabled( false );
      return;
   }
   pick->setEnabled( true );
   QColor cblack( Qt::black );
   QColor cwhite( Qt::white );
   // make sure rubber band and tracker show up against background
   QColor bg    = data_plot->canvasBackground();
   int csum     = bg.red() + bg.green() + bg.blue();
   pickpen->setColor( ( csum > 600 ) ? cblack : cwhite );
   pick->setRubberBandPen( *pickpen );
   pick->setTrackerPen(    *pickpen );
DbgLv(1) << "manDr: bg" << bg << "csum" << csum;
DbgLv(1) << "manDr:   pickpen" << pickpen->brush().color();
DbgLv(1) << "manDr:    cblack" << cblack << "cwhite" << cwhite;
}

// Auto assign solute bins
void US_GA_Initialize::autoAssignSb( void )
{
   if ( sk_distro.isEmpty() ) {
      return;
   }

   if ( rb_x_rh->isChecked() || rb_y_rh->isChecked() )
   {
      QMessageBox::information( this, "Information", "Autoassign is disabled if Rh selected.");
      return;
   }

   if ( soludata->bucketsCount() > 0 )
   {
      QString msg = "The solute bins list is not empty. " 
                    "Rebuilding the bins will delete all existing ones.\n"
                    "Do you want to continue?";
      int res = QMessageBox::question( this, "Auto Find Solute Bins", msg );
      if ( res != QMessageBox::Yes ) {
         return;
      }
   }
   resetSb();
   int nisols    = xy_distro.size();
   double wxbuck = ct_wxbuck->value();
   double hybuck = ct_hybuck->value();
   soludata->autoCalcBins( nisols, wxbuck, hybuck );
   manbuks      = false;
   is_saved     = false;
   fill_lw_bins();
   plot_buckets();
   if ( lw_sbin_data->count() > 0 )
   {
      lw_sbin_data->setCurrentRow(0);
   }
}

void US_GA_Initialize::fill_lw_bins()
{
   const QSignalBlocker blocker( lw_sbin_data );
   lw_sbin_data->clear();
   int nbuks = soludata->bucketsCount();
   for ( int ii = 0; ii < nbuks; ii++ )
   {
      lw_sbin_data->addItem( soludata->bucketLine( ii ) );
   }
   le_nbuks->clear();
   if ( nbuks > 0 )
   {
      le_nbuks->setText( QString::number( nbuks ) );
   }
}

// Reset solute bins
void US_GA_Initialize::resetSb( void )
{
   le_nbuks->clear();
   soludata->clearBuckets();
   is_saved = false;
   fill_lw_bins();
   plot_buckets();
}

// (re)plot data
void US_GA_Initialize::replot_data()
{
   if ( xy_distro.isEmpty() )
      return;

   plot_3dim();
   setBucketPens();
   plot_buckets();
}

void US_GA_Initialize::erase_curves( bool buckets_only )
{
   if ( buckets_only )
   {
      for ( int ii = 0; ii < bucket_curves.size(); ii++ )
      {
         bucket_curves[ ii ]->detach();
         delete bucket_curves[ ii ];
       }
   }
   else
   {
      dataPlotClear( data_plot );
   }
   bucket_curves.clear();
   data_plot->replot();
}

void US_GA_Initialize::plot_buckets()
{
   erase_curves( true );

   for ( int ii = 0; ii < soludata->bucketsCount(); ii++ )
   {
      QRectF r = soludata->bucketRect( ii );
      const double x[] = {
        r.left(), r.right(), r.right(), r.left(), r.left()
      };
      const double y[] = {
        r.top(), r.top(), r.bottom(), r.bottom(), r.top()
      };

      auto curve = us_curve( data_plot, QString( "bucket border %1" ).arg( ii ) );
      curve->setPen( pbukpen );
      curve->setStyle( QwtPlotCurve::Lines );
      curve->setSamples( x, y, 5 );
      curve->attach( data_plot );
      bucket_curves << curve;
   }
   highlight_bucket( lw_sbin_data->currentRow() );
}

// plot data 3-D
void US_GA_Initialize::plot_3dim( void )
{
   erase_curves( false );
   QColor cblack( Qt::black );
   QColor cwhite( Qt::white );
   QColor bg     = colormap->color1();

   data_plot->setCanvasBackground( bg );

   int csum      = bg.red() + bg.green() + bg.blue();
   pick->setTrackerPen( QPen( ( csum > 600 ) ? cblack : cwhite ) );
DbgLv(1) << "pl3d: bg" << bg << "csum" << csum;
DbgLv(1) << "pl3d:   pickpen" << pick->trackerPen().brush().color();
DbgLv(1) << "pl3d:    cblack" << cblack << "cwhite" << cwhite;

   QString tstr  = run_name + "\n" + analys_name + "\n (" + method + ")";
   data_plot->setTitle( tstr );

   // set up spectrogram data
   QwtPlotSpectrogram* d_spectrogram = new QwtPlotSpectrogram();
   US_SpectrogramData* spec_dat      = new US_SpectrogramData();
   d_spectrogram->setColorMap( ColorMapCopy( colormap ) );
   d_spectrogram->setData    ( spec_dat );
   d_spectrogram->attach( data_plot );

   QRectF drect = QRectF( plxmin, plymin,
                        ( plxmax - plxmin ), ( plymax - plymin ) );

   double zmax = xy_distro.at( 0 ).c;
   double zmin = 0.0;

   for ( int jj = 0; jj < xy_distro.size(); jj++ )
   {
      zmin  = qMin( zmin, xy_distro.at( jj ).c );
      zmax  = qMax( zmax, xy_distro.at( jj ).c );
   }

   spec_dat->setRastRanges( xreso, yreso, resolu, zfloor, drect );
   spec_dat->setZRange( zmin, zmax );
   spec_dat->setRaster( &xy_distro );

   // Set color map and axis settings
   QwtScaleWidget *rightAxis = data_plot->axisWidget( QwtPlot::yRight );
   rightAxis    ->setColorBarEnabled( true );
   rightAxis    ->setColorMap( spec_dat->range(),
                               ColorMapCopy( colormap ) );
   d_spectrogram->setColorMap( ColorMapCopy( colormap ) );
   data_plot->setAxisTitle( QwtPlot::xBottom, xa_title );
   data_plot->setAxisTitle( QwtPlot::yLeft,   ya_title );
   data_plot->setAxisTitle( QwtPlot::yRight,  tr( "Partial Concentration" ) );
   data_plot->axisTitle( QwtPlot::yRight ).setFont(
   data_plot->axisTitle( QwtPlot::yLeft ).font() );
   data_plot->setAxisScale( QwtPlot::yRight,
   spec_dat->range().minValue(), spec_dat->range().maxValue() );
   data_plot->enableAxis( QwtPlot::yRight );

   data_plot->setAxisScale( QwtPlot::xBottom, plxmin, plxmax );
   data_plot->setAxisScale( QwtPlot::yLeft,   plymin, plymax );
   data_plot->replot();

}

// update pseudo-3d resolution factor
void US_GA_Initialize::update_resolu( double dval )
{
   resolu = dval;
   replot_data();
}

// update raster x resolution
void US_GA_Initialize::update_xreso( double dval )
{
   xreso  = dval;
   replot_data();
}

// update raster y resolution
void US_GA_Initialize::update_yreso( double dval )
{
   yreso  = dval;
   replot_data();
}

// update Z (frequency) floor percent
void US_GA_Initialize::update_zfloor( double dval )
{
   zfloor = dval;
   replot_data();
}

double US_GA_Initialize::str2double( const QString& str, double val )
{
  bool ok;
  double newval = str.toDouble( &ok );
  if (ok)
  {
    return newval;
  } else {
    QMessageBox::warning(this, "Warning!", "Please enter a number!");
    return val;
  }
}

// update plot limit s min
void US_GA_Initialize::update_plxmin()
{
  double val = str2double( le_plxmin->text(), plxmin );
  if (val >= plxmax) {
    QString msg = "Minimum of the x-axis must be less than the maximum of it!";
    QMessageBox::warning(this, "Error!", msg);
  } else {
    plxmin = val;
  }
  if ( manual_limits.contains( limit_key ) )
  {
    manual_limits[ limit_key ][0] = plxmin;
  }

  replot_data();
}

// update plot limit s max
void US_GA_Initialize::update_plxmax()
{
  double val = str2double( le_plxmax->text(), plxmax);
  if (val <= plxmin) {
    QString msg = "Maximum of the x-axis must be greater than the minimum of it!";
    QMessageBox::warning(this, "Error!", msg);
  } else {
    plxmax= val;
  }
  le_plxmax->setText( QString::number( plxmax ) );
  if ( manual_limits.contains( limit_key ) )
  {
    manual_limits[ limit_key ][1] = plxmax;
  }

  replot_data();
}

// update plot limit f/f0 min
void US_GA_Initialize::update_plymin()
{
  double val = str2double( le_plymin->text(), plymin );
  if (val >= plymax) {
    QString msg = "Minimum of the y-axis must be less than the maximum of it!";
    QMessageBox::warning(this, "Error!", msg);
  } else {
    plymin = val;
  }
  le_plymin->setText( QString::number( plymin ) );
  le_plxmax->setText( QString::number( plxmax ) );
  if ( manual_limits.contains( limit_key ) )
  {
    manual_limits[ limit_key ][2] = plymin;
  }

  replot_data();
}


// update plot limit f/f0 max
void US_GA_Initialize::update_plymax()
{
  double val = str2double( le_plymax->text(), plymax);
  if (val <= plymin) {
    QString msg = "Maximum of the y-axis must be greater than the minimum of it!";
    QMessageBox::warning(this, "Error!", msg);
  } else {
    plymax= val;
  }
  le_plymax->setText( QString::number( plymax ) );
  le_plxmax->setText( QString::number( plxmax ) );
  if ( manual_limits.contains( limit_key ) )
  {
    manual_limits[ limit_key ][3] = plymax;
  }

  replot_data();
}

// load the solute distribution from a file or from DB
void US_GA_Initialize::load_distro()
{
   S_Solute        sol_sk;
   S_Solute        sol_xy;
   US_Model        model;
   QString         mdesc;
   bool            loadDB = dkdb_cntrls->db();
   double          smin   = 1e+39;
   double          smax   = 1e-39;
   double          kmin   = 1e+39;
   double          kmax   = 1e-39;
   double          wmin   = 1e+39;
   double          wmax   = 1e-39;
   double          vmin   = 1e+39;
   double          vmax   = 1e-39;
   double          dmin   = 1e+39;
   double          dmax   = 1e-39;

   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
   US_ModelLoader dialog( loadDB, mfilter, model, mdesc, pfilts );
   dialog.move( this->pos() + QPoint( 200, 200 ) );

   connect( &dialog, &US_ModelLoader::changed,
            this, &US_GA_Initialize::update_disk_db );
   QApplication::restoreOverrideCursor();

   QString         mfnam;
   QString         sep;
   QString         aiters;

   if ( dialog.exec() != QDialog::Accepted )
      return;


DbgLv(1) << "LOAD ACCEPT  Description:\n " << mdesc;
   sep       = mdesc.left( 1 );
   mfnam     = mdesc.section( sep, 2, 2 );
   aiters    = mdesc.section( sep, 6, 6 );

   if ( mfnam.isEmpty() )
   {  // From db:  make ID the "filename"
      mfnam     = "db ID " + mdesc.section( sep, 4, 4 );
   }

   else
   {  // From disk:  use base file name
      mfnam     = QFileInfo( mfnam ).baseName();
   }

   if ( model.components.size() < 1 )
   {
DbgLv(0) << "  NO Model components";
      return;
   }

   // insure all model coefficient properties are set
   if ( ! model.update_coefficients() )
   {
      DbgLv(0) << "*** Unable to recalculate coefficient values ***";
   }

   // parse model information from its description
   mdesc        = mdesc.section( sep, 1, 1 );
   method       = model.typeText();
   method       = mdesc.contains( "-CG" )
                  ? method.replace( "2DSA", "2DSA-CG" ) : method;
   run_name     = mdesc.section( ".",  0, -3 );
   QString asys = mdesc.section( ".", -2, -2 );
   analys_name  = asys.section( "_", 0, 1 ) + "_"
                + asys.section( "_", 3, 4 );

   if ( method == "Manual"  ||  method == "CUSTOMGRID" )
   {
      int jj       = mdesc.indexOf( ".model" );
      mdesc        = ( jj < 1 ) ? mdesc : mdesc.left( jj );
      run_name     = mdesc + ".0Z280";
      analys_name  = "e00_a00_" + method + "_local";
   }

   monte_carlo  = model.monteCarlo;
   mc_iters     = monte_carlo ? aiters.toInt() : 1;
   editGUID     = model.editGUID;
DbgLv(1) << "MC" << monte_carlo << " iters" << mc_iters;

   sk_distro.clear();
   xy_distro.clear();

   resetSb();

   QString tstr = run_name+ "\n" + analys_name + "\n (" + method + ")";
   data_plot->setTitle( tstr );

   // read in and set distribution s,c,k,d values
   if ( model.analysis != US_Model::COFS )
   {
      for ( int jj = 0; jj < model.components.size(); jj++ )
      {
         US_Model::calc_coefficients( model.components[ jj ] );

         sol_sk.s  = model.components[ jj ].s * 1.0e13;
         sol_sk.k  = model.components[ jj ].f_f0;
         sol_sk.c  = model.components[ jj ].signal_concentration;
         sol_sk.w  = model.components[ jj ].mw;
         sol_sk.v  = model.components[ jj ].vbar20;
         sol_sk.d  = model.components[ jj ].D;
         sol_sk.f  = model.components[ jj ].f;
         sol_sk.r  = model.components[ jj ].f / ( 6e-9 * M_PI * VISC_20W );

         sol_xy    = sol_sk;
         sol_xy.s  = ( attr_x == ATTR_S ) ? sol_sk.s : sol_xy.s;
         sol_xy.s  = ( attr_x == ATTR_K ) ? sol_sk.k : sol_xy.s;
         sol_xy.s  = ( attr_x == ATTR_W ) ? sol_sk.w : sol_xy.s;
         sol_xy.s  = ( attr_x == ATTR_V ) ? sol_sk.v : sol_xy.s;
         sol_xy.s  = ( attr_x == ATTR_D ) ? sol_sk.d : sol_xy.s;
         sol_xy.s  = ( attr_x == ATTR_R ) ? sol_sk.r : sol_xy.s;
         sol_xy.k  = ( attr_y == ATTR_S ) ? sol_sk.s : sol_xy.k;
         sol_xy.k  = ( attr_y == ATTR_K ) ? sol_sk.k : sol_xy.k;
         sol_xy.k  = ( attr_y == ATTR_W ) ? sol_sk.w : sol_xy.k;
         sol_xy.k  = ( attr_y == ATTR_V ) ? sol_sk.v : sol_xy.k;
         sol_xy.k  = ( attr_y == ATTR_D ) ? sol_sk.d : sol_xy.k;
         sol_xy.k  = ( attr_y == ATTR_R ) ? sol_sk.r : sol_xy.k;
         sol_xy.si = sol_sk.s;
         sol_xy.ki = sol_sk.k;

         smin      = qMin( smin, sol_sk.s );
         smax      = qMax( smax, sol_sk.s );
         kmin      = qMin( kmin, sol_sk.k );
         kmax      = qMax( kmax, sol_sk.k );
         wmin      = qMin( wmin, sol_sk.w );
         wmax      = qMax( wmax, sol_sk.w );
         vmin      = qMin( vmin, sol_sk.v );
         vmax      = qMax( vmax, sol_sk.v );
         dmin      = qMin( dmin, sol_sk.d );
         dmax      = qMax( dmax, sol_sk.d );
//DbgLv(2) << "Solute jj s w k c d" << jj << sol_s.s << sol_w.s << sol_s.k
//   << sol_s.c << sol_s.d << " vb" << model.components[jj].vbar20;

         sk_distro << sol_sk;
         xy_distro << sol_xy;
      }

      // sort and reduce distributions
      psdsiz    = xy_distro.size();
      sort_distro( sk_distro, false );
      sort_distro( xy_distro, true  );
DbgLv(1) << "Solute psdsiz sdsiz xdsiz" << psdsiz << sk_distro.size()
 << xy_distro.size();
for ( int jj=0;jj<sk_distro.size();jj++ ) {
 DbgLv(2) << " jj" << jj << " s k" << sk_distro[jj].s << sk_distro[jj].k
    << " w v" << sk_distro[jj].w << sk_distro[jj].v; }
   }

   // Determine which attribute is fixed
   if (      equivalent( vmin, vmax, 0.001 ) )
      attr_z    = ATTR_V;
   else if ( equivalent( kmin, kmax, 0.001 ) )
      attr_z    = ATTR_K;
   else if ( equivalent( smin, smax, 0.001 ) )
      attr_z    = ATTR_S;
   else if ( equivalent( wmin, wmax, 0.001 ) )
      attr_z    = ATTR_W;
   else if ( equivalent( dmin, dmax, 0.001 ) )
      attr_z    = ATTR_D;

   if ( attr_x != ATTR_V  &&  attr_y != ATTR_V  &&  attr_z != ATTR_V )
   {  // No attribute is vbar, so use vbar as the default X or Y
      int attrv = ATTR_V;

      if ( attr_y == attr_z )
      {
         rb_y_vbar->setChecked( true  );
         select_y_axis( attrv );
      }
      else
      {
         rb_x_vbar->setChecked( true  );
         select_x_axis( attrv );
      }
   }

   QStringList attrs;
   attrs << "s" << "f/f0" << "MW" << "vbar" << "D" << "f";
   QString s_attr = attrs[ attr_z ];

   le_nsols->setText( QString::number( sk_distro.size() ) );
   set_limits();

   data_plot->setAxisScale( QwtPlot::xBottom, plxmin, plxmax );
   data_plot->setAxisScale( QwtPlot::yLeft,   plymin, plymax );
   data_plot->replot();

   // nisols       = sdistro->size();
   s_attr       = "\"" + s_attr + "\".";
   dfilname     = "(" + mfnam + ") " + mdesc;
   stdfline     = "  " + dfilname;
   stfxline     = tr( "The components fixed attribute is " ) + s_attr;
   // stnpline     = tr( "The number of distribution points is %1" )
   //                .arg( nisols );
   // if ( nisols != psdsiz )
   //    stnpline    += tr( "\n  (reduced from %1)" ).arg( psdsiz );

   te_status->setText( stcmline + "\n" + stdiline + "\n"
         + stdfline + "\n" + stfxline + "\n" + stnpline + "." );

   replot_data();

   soludata->setDistro( &xy_distro, attr_x, attr_y, attr_z );

   ct_wxbuck->setDisabled( monte_carlo );
   ct_hybuck->setDisabled( monte_carlo );
   pb_autassb->setDisabled( monte_carlo );
}

// load the color map from a file
void US_GA_Initialize::load_color()
{
   QString filter = tr( "Color Map files (*cm-*.xml);;" )
         + tr( "Any XML files (*.xml);;" )
         + tr( "Any files (*)" );

   // get an xml file name for the color map
   QString fname = QFileDialog::getOpenFileName( this,
      tr( "Load Color Map File" ),
      US_Settings::etcDir(), filter, nullptr );

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

   stcmline  = cmapname;
   te_status->setText( stcmline + "\n" + stdiline + "\n"
         + stdfline + "\n" + stfxline + "\n" + stnpline );

   replot_data();
}

// set plot x,y limits
void US_GA_Initialize::set_limits()
{
   soludata->setDistro( &xy_distro, attr_x, attr_y, attr_z );
   
   // find min,max for S distributions
   double smin = +1.0e99;
   double smax = -1.0e99;
   double kmin = +1.0e99;
   double kmax = -1.0e99;
   double sinc;
   double kinc;
   for ( int jj = 0; jj < xy_distro.size(); jj++ )
   {
      double sval = xy_distro.at( jj ).s;
      double kval = xy_distro.at( jj ).k;
      smin        = qMin( smin, sval );
      smax        = qMax( smax, sval );
      kmin        = qMin( kmin, kval );
      kmax        = qMax( kmax, kval );
   }
DbgLv(1) << "SL: distr skmin,max" << smin << smax << kmin << kmax;

   // adjust minima, maxima
   sinc      = ( smax - smin ) / 10.0;
   kinc      = ( kmax - kmin ) / 10.0;
DbgLv(1) << "SL: adj sinc kinc" << sinc << kinc;
   sinc      = ( sinc == 0.0 || xy_distro.size() == 1 ) ? ( smin * 0.05 ) : sinc;
   kinc      = ( kinc == 0.0 || xy_distro.size() == 1 ) ? ( kmin * 0.05 ) : kinc;
DbgLv(1) << "SL:  adj sinc kinc" << sinc << kinc;

   smin     -= sinc;
   smax     += sinc;
   kmin     -= kinc;
   kmax     += kinc;
DbgLv(1) << "SL: adj smin,max" << smin << smax
 << "kmin,max" << kmin << kmax;

   sinc        = pow( 10.0, qFloor( log10( smax ) ) - 3.0 );
   kinc        = pow( 10.0, qFloor( log10( kmax ) ) - 3.0 );
DbgLv(1) << "SL: aut min,inc" << smax << sinc << kmax << kinc;

   // Make x,y limits multiples of reasonable values
   if ( equivalent( smin, smax, 0.001 ) )
   {
      smin       -= sinc;
      smax       += sinc;
   }
   if ( equivalent( kmin, kmax, 0.001 ) )
   {
      kmin       -= kinc;
      kmax       += kinc;
   }
   smin        = qFloor( smin / sinc ) * sinc;
   smin        = ( smin < 0.0 ) ? ( smin - sinc ) : smin;
   smax        = qFloor( smax / sinc ) * sinc + sinc;
   smin        = ( attr_x != ATTR_S ) ?  qMax( smin, 0.0 ) : smin;
   smin        = ( attr_x == ATTR_K ) ?  qMax( smin, 0.8 ) : smin;
   kmin        = qFloor( kmin / kinc ) * kinc;
   kmax        = qFloor( kmax / kinc ) * kinc + kinc;
   kmin        = ( attr_y != ATTR_S ) ?  qMax( kmin, 0.0 ) : kmin;
   kmin        = ( attr_y == ATTR_K ) ?  qMax( kmin, 0.8 ) : kmin;
DbgLv(1) << "SL: auto smin,max,inc" << smin << smax << sinc
 << "kmin,max,inc" << kmin << kmax << kinc;

   plxmin = smin;
   plxmax = smax;
   plymin = kmin;
   plymax = kmax;
   if ( ck_manlim->isChecked() )
   {
      if ( manual_limits.contains( limit_key) )
      {
         auto vec = manual_limits.value( limit_key );
         plxmin = vec.at( 0 );
         plxmax = vec.at( 1 );
         plymin = vec.at( 2 );
         plymax = vec.at( 3 );
      } else
      {
         QVector< double > vec;
         vec << plxmin << plxmax << plymin << plymax;
         manual_limits.insert( limit_key, vec );
      }
      le_plxmin->setReadOnly( false );
      le_plxmax->setReadOnly( false );
      le_plymin->setReadOnly( false );
      le_plymax->setReadOnly( false );
   }
   else
   {
      le_plxmin->setReadOnly( true );
      le_plxmax->setReadOnly( true );
      le_plymin->setReadOnly( true );
      le_plymax->setReadOnly( true );
   }

   le_plxmin->setText( QString::number( plxmin ) );
   le_plxmax->setText( QString::number( plxmax ) );
   le_plymin->setText( QString::number( plymin ) );
   le_plymax->setText( QString::number( plymax ) );

   // Set bucket width,height values
   double wxbuck       = ( smax - smin ) * 0.1;
   double hybuck       = ( kmax - kmin ) * 0.2;
   int    spwr  = qRound( log10( wxbuck ) );
   int    kpwr  = qRound( log10( hybuck ) );
   smax         = pow( 10.0, spwr + 3 );
   kmax         = pow( 10.0, kpwr + 3 );
   sinc         = pow( 10.0, spwr - 3 );
   kinc         = pow( 10.0, kpwr - 3 );
   //wxbuck       = qRound( wxbuck / sinc ) * sinc;
   ct_wxbuck->setRange( 0.0, smax );
   ct_wxbuck->setSingleStep( sinc );
   ct_wxbuck->setValue( wxbuck );
   //hybuck       = qRound( hybuck / kinc ) * kinc;
   ct_hybuck->setRange( 0.0, kmax );
   ct_hybuck->setSingleStep( kinc );
   ct_hybuck->setValue( hybuck );

   // Update help and status text
   QString hmsg = tr(
      "Now either auto-assign the solute bins, or manually select bins"
      " by clicking on a bin vertex, then moving and releasing on the"
      " other vertex. If you auto-assign the bins you should first"
      " select the number of solute bins you want to use. UltraScan will"
      " space the bins proportional to the integral value of each peak,"
      " such that each bin contains the same integral value."
      " You can select each solute bin from the listbox on the left and"
      " modify its size by first changing the bucket dimensions with the"
      " respective counters, then double-click on the listbox item."
      " You may remove a bin by right-mouse-button clicking on the listbox"
      " item and responding/defaulting Yes in the resulting dialog." );

   if ( attr_x != ATTR_V  &&  attr_y != ATTR_V  &&  attr_z != ATTR_V )
   {
      hmsg         = hmsg + tr( "\n\nNO SAVE ENABLED:  None of X "
                                "or Y or the fixed-attribute is VBAR." );
   }

   if ( monte_carlo )
   {
      hmsg         = hmsg + tr( "\n\nNO AUTOASSIGN ENABLED:  "
                                "Distribution is from a Monte-Carlo." );
   }

   te_pctl_help->setText( hmsg );

DbgLv(1) << "SL: autoassn xmin,xmax,ymin,ymax" << plxmin << plxmax
 << plymin << plymax;

   int kisols   = xy_distro.size();
   stnpline     = tr( "The number of distribution points is %1." )
                  .arg( kisols );
   te_status->setText( stcmline + "\n" + stdiline + "\n"
         + stdfline + "\n" + stfxline + "\n" + stnpline );

}

// Sort distribution solute list by s,k values and optionally reduce
void US_GA_Initialize::sort_distro( QList< S_Solute >& listsols,
      bool reduce )
{
   int sizi = listsols.size();

   if ( sizi < 2 )
      return;        // nothing need be done for 1-element list

   // sort distro solute list by s,k values

   std::sort( listsols.begin(), listsols.end(), distro_lessthan );

   // check reduce flag

   if ( reduce )
   {     // skip any duplicates in sorted list
      S_Solute sol1;
      S_Solute sol2;
      QList< S_Solute > reduced;
      sol1     = listsols.at( 0 );
      reduced.append( sol1 );    // output first entry
      int kdup = 0;
      int jdup = 0;

      for ( int jj = 1; jj < sizi; jj++ )
      {     // loop to compare each entry to previous
          sol2    = listsols.at( jj );  // solute entry

          if ( sol1.s != sol2.s  ||  sol1.k != sol2.k )
          {   // not a duplicate, so output to temporary list
             reduced.append( sol2 );
             jdup    = 0;
          }

          else
          {   // duplicate:  sum c value
DbgLv(1) << "DUP: sval svpr jj" << sol1.s << sol2.s << jj;
             kdup    = qMax( kdup, ++jdup );
             qreal f = (qreal)( jdup + 1 );
             sol2.c += sol1.c;   // sum c value
             sol2.s  = ( sol1.s * jdup + sol2.s ) / f;  // average s,k
             sol2.k  = ( sol1.k * jdup + sol2.k ) / f;
             reduced.replace( reduced.size() - 1, sol2 );
          }

          sol1    = sol2;        // save entry for next iteration
      }

      if ( reduced.size() < sizi )
      {   // if some reduction happened, replace list with reduced version
         //double sc = 1.0 / (double)( kdup + 1 );

         //for ( int ii = 0; ii < reduced.size(); ii++ )
         //   reduced[ ii ].c *= sc;

         listsols = reduced;
      }
   }
   return;
}

// set bucket pens for previous and current bin
void US_GA_Initialize::setBucketPens()
{
   QPen penCR( QColor( Qt::red ),   1, Qt::SolidLine );
   QPen penCY( QColor( Qt::yellow), 1, Qt::SolidLine );
   QPen penCB( QColor( Qt::blue),   1, Qt::SolidLine );

   // current is always red
   cbukpen   = penCR;

   // previous is blue for light background, yellow for dark
   QColor bg = data_plot->canvasBackground();
   int csum  = bg.red() + bg.green() + bg.blue();
   pbukpen   = ( csum > 600 ) ? penCB : penCY;

   return;
}

// find bucket curve by solute index, then highlight
void US_GA_Initialize::highlight_bucket( int sx )
{
   for ( int ii = 0; ii < bucket_curves.size(); ii++ )
   {
      if ( ii == sx )
      {
         bucket_curves[ ii ]->setPen( cbukpen );
      }
      else
      {
         bucket_curves[ ii ]->setPen( pbukpen );
      }
   }
   data_plot->replot();
}

// mouse down:  save of first point
void US_GA_Initialize::getMouseDown( const QPointF& p )
{
   if ( !pb_mandrsb->isChecked() ) {
      return;
   }

   if ( sk_distro.isEmpty() ) {
      return;
   }
   p1     = p;              // save the first rubberband point
}

// mouse up:  draw bucket rectangle
void US_GA_Initialize::getMouseUp( const QPointF& p )
{
   if ( !pb_mandrsb->isChecked() ) {
      return;
   }

   if ( sk_distro.isEmpty() ) {
      return;
   }
   
   double        tx[2];
   double        ty[2];

   p2     = p;              // save the second rubberband point

   // construct and save a bucket entry
   tx[0]  = p1.x();         // upper,left and lower,right points
   ty[0]  = p1.y();
   tx[1]  = p2.x();
   ty[1]  = p2.y();

   if ( tx[0] > tx[1] )     // insure properly ordered
   {
      tx[0]  = p2.x();
      tx[1]  = p1.x();
   }

   if ( ty[0] > ty[1] )
   {
      ty[0]  = p2.y();
      ty[1]  = p1.y();
   }

   // create bucket rectangle, solute point, and concentration value
   QRectF bucr( QPointF( tx[0], ty[1] ), QPointF( tx[1], ty[0] ) );
   if ( tx[0] == tx[1] || ty[0] == ty[1] )
      return;

   QPointF tpt( ( tx[0] + tx[1] ) / 2.0, ( ty[0] + ty[1] ) / 2.0 );
   QPointF& bucp = tpt;
   int sx        = soludata->findNearestPoint( bucp );

   if ( sx >= 0 )
   {  // for solute point nearest to rectangle midpoint
      S_Solute sol  = xy_distro.at( sx );
      qreal bucc    = sol.c;  // get concentrate value

      // add the bucket entry and add a text box entry
      bucket candidate = soludata->createBucket( bucr, bucp, bucc, 2 );
      const QRectF region( QPointF( candidate.x_min, candidate.y_min ),
                           QPointF( candidate.x_max, candidate.y_max ) );
      for ( int ii = 0; ii < soludata->bucketsCount(); ++ii )
      {
         if ( soludata->bucketRect( ii ).normalized().intersects( region.normalized() ) )
         {
            QMessageBox::warning( this, "Warning!", "The selected bin overlaps an existing bin." );
            return;
         }
      }
      soludata->appendBucket( candidate );
   }

   if ( sx < 0 )
      return;
   manbuks = true;
   fill_lw_bins();
   plot_buckets();
   if (lw_sbin_data->count() > 0)
   {
      lw_sbin_data->setCurrentRow(lw_sbin_data->count() - 1);
   }
   manbuks  = true;
   is_saved = false;
}

// solute bin list row clicked: highlight bucket
void US_GA_Initialize::sclick_sbdata( const QModelIndex& mx )
{
   int  sx     = mx.row();
   if ( sx < 0 || sx >= soludata->bucketsCount() )
      return;
   bool global = monte_carlo || manbuks;
   bool rtbtn  = ( QApplication::mouseButtons() == Qt::RightButton );
DbgLv(1) << "gain: sclick_sbd: sx" << sx << "rtbtn" << rtbtn;

   highlight_bucket( sx );

   if ( rtbtn )
   {  // Handle right-mouse-button click:  delete bin
      QString msg = tr( "Are you sure you want to delete solute bin %1 ?" ).arg( sx + 1 );
      int ret = QMessageBox::question( this,"GA_Init Solute Bin Delete", msg );
      if ( ret == QMessageBox::No)
      {
         return;
      }
      else
      {
         const QSignalBlocker blocker( lw_sbin_data );
         delete lw_sbin_data->takeItem( sx );
         soludata->removeBucketAt( sx );
         is_saved = false;
         bucket_curves[ sx ]->detach();
         delete bucket_curves[ sx ];
         bucket_curves.removeAt( sx );
      }

      data_plot->replot();
      int nibuks = soludata->bucketsCount();
      int rx      = qMin( sx, ( nibuks - 1 ) );
      if ( rx >= 0 )
      {
         lw_sbin_data->setCurrentRow( rx );
         highlight_bucket( rx );
         data_plot->replot();
      }
   }

   else if ( global )
   {
      QRectF rect = soludata->bucketRect( sx );
      ct_wxbuck->setValue( rect.width() );
      ct_hybuck->setValue( rect.height() );
   }
}

// solute bin list row double-clicked:  change bucket values
void US_GA_Initialize::dclick_sbdata( const QModelIndex& mx )
{
   int sx      = mx.row();
   if ( sx < 0 || sx >= soludata->bucketsCount() )
      return;
   bool global = monte_carlo || manbuks;
   QPointF pt0;

   if ( !global )
   {
      pt0         = soludata->bucketPoint( sx, false );
   }
   else
   {
      QRectF rect = soludata->bucketRect( sx );
      QPointF ptl = rect.topLeft();
      QPointF pbr = rect.bottomRight();
      pt0         = QPointF( ( ptl.x() + pbr.x() ) / 2.0, 
                             ( ptl.y() + pbr.y() ) / 2.0 );
   }

   double wxbuck = ct_wxbuck->value();
   double hybuck = ct_hybuck->value();
   qreal x1    = pt0.x() - wxbuck / 2.0;
   qreal y1    = pt0.y() + hybuck / 2.0;
   qreal x2    = pt0.x() + wxbuck / 2.0;
   qreal y2    = pt0.y() - hybuck / 2.0;
   QPointF pt1( x1, y1 );
   QPointF pt2( x2, y2 );
   QRectF  brect( pt1, pt2 );

   if ( wxbuck <= 0.0 || hybuck <= 0.0 )
      return;

   changeBucketRect( sx, brect );  // change bucket rectangle
   plot_buckets();
   highlight_bucket( sx );

   return;
}

// change the rectangle (vertices) for a bucket
void US_GA_Initialize::changeBucketRect( int sx, QRectF& rect )
{
   if ( sx < 0 || sx >= soludata->bucketsCount() )
      return;

   QPointF bpnt  = soludata->bucketPoint( sx, true );
   qreal bconc   = soludata->bucketAt( sx ).conc;
   int bstat     = 0;

   bucket candidate = soludata->createBucket( rect, bpnt, bconc, bstat );
   const QRectF region( QPointF( candidate.x_min, candidate.y_min ),
                        QPointF( candidate.x_max, candidate.y_max ) );
   for ( int ii = 0; ii < soludata->bucketsCount(); ++ii )
   {
      if ( ii != sx && soludata->bucketRect( ii ).normalized().intersects( region.normalized() ) )
      {
         QMessageBox::warning( this, "Warning!", "The resized bin overlaps an existing bin." );
         return;
      }
   }
   soludata->setBucket( sx, rect, bpnt, bconc, bstat );
   is_saved = false;

   QString line  = soludata->bucketLine( sx );
   lw_sbin_data->item( sx )->setText( line );

   return;
}

// Flag whether two values are effectively equal within a given epsilon
bool US_GA_Initialize::equivalent( double a, double b, double eps )
{
   return ( qAbs( ( a - b ) / a ) <= eps );
}

// Reset Disk_DB control whenever data source is changed in any dialog
void US_GA_Initialize::update_disk_db( bool isDB )
{
   isDB ? dkdb_cntrls->set_db() : dkdb_cntrls->set_disk();
}
// Select a prefilter for model distribution list
void US_GA_Initialize::select_prefilt( void )
{
   QString pfmsg;
   int nruns = 0;
   pfilts.clear();

   US_SelectRuns srdiag( dkdb_cntrls->db(), pfilts );
   srdiag.move( this->pos() + QPoint( 200, 200 ) );
   connect( &srdiag, &US_SelectRuns::dkdb_changed,
            this,    &US_GA_Initialize::update_disk_db );

   if ( srdiag.exec() == QDialog::Accepted )
      nruns      = pfilts.size();
   else
      pfilts.clear();

   if ( nruns == 0 )
      pfmsg = tr( "(none chosen)" );

   else if ( nruns > 1 )
      pfmsg = tr( "RunID prefilter - %1 run(s)" ).arg( nruns );

   else
      pfmsg = tr( "RunID prefilter - 1 run: " ) +
              QString( pfilts[ 0 ] ).left( 8 ) + "...";

DbgLv(1) << "PreFilt: pfilts[0]" << ((nruns>0)?pfilts[0]:"(none)");
   le_prefilt->setText( pfmsg );
}

// View the statistics file produced in a file editor
void US_GA_Initialize::view( )
{
   if ( sk_distro.isEmpty() ) {
      return;
   }

DbgLv(1) << "VIEW";
   QString runid = run_name.section( ".",  0, -2 );
   if ( runid.startsWith( "Global-" ) )
      runid         = runid.mid( 7 );
   QString trpid = run_name.section( ".", -1, -1 );
   QString fdir  = US_Settings::resultDir() + "/" + runid;
   QString fnsta = "gainit." + trpid + ".sol_integ.stats";
   QString fname = fdir + "/" + fnsta;
DbgLv(1) << "VIEW fname" << fname;

   if ( ! is_saved )
   {  // No save was done, so generate report file
      soludata->buildDataMC();                     // build it
      soludata->reportDataMC( fname, mc_iters );   // report it
      is_saved      = true;
   }

   QFile filei( fname );
   if ( filei.open( QIODevice::ReadOnly | QIODevice::Text ) )
   {
DbgLv(1) << "VIEW OPENED";
      QTextStream ts( &filei );
      QString rtext = ts.readAll();
      filei.close();

      US_Editor* eddiag = new US_Editor( US_Editor::DEFAULT, true );
      eddiag->setWindowTitle( tr( "Statistics File Contents" ) );
      eddiag->move( this->pos() + QPoint( 30, 30 ) );
      eddiag->resize( 760, 700 );
      eddiag->e->setFont( US_Widgets::fixedFont() );
      eddiag->e->setPlainText( rtext );
DbgLv(1) << "VIEW eddiag SHOW";
      eddiag->show();
   }

   else
   {
DbgLv(1) << "VIEW OPEN ERROR" << fname;
      QMessageBox::critical( this, tr( "File Read Error" ),
         tr( "Unable to open file\n\"%1\"\nfor read" ).arg( fname ) );
   }
}

void US_GA_Initialize::select_manlim()
{
   if ( sk_distro.empty() )
   {
      return;
   }

   build_xy_distro();
   set_limits();
   replot_data();
}

// Select the coordinate for the horizontal axis
void US_GA_Initialize::select_x_axis( int ival )
{
   attr_x = ival;
   rb_y_s   ->setEnabled( attr_x != ATTR_S );
   rb_y_ff0 ->setEnabled( attr_x != ATTR_K );
   rb_y_mw  ->setEnabled( attr_x != ATTR_W );
   rb_y_vbar->setEnabled( attr_x != ATTR_V );
   rb_y_D   ->setEnabled( attr_x != ATTR_D );
   rb_y_rh  ->setEnabled( attr_x != ATTR_R );
   update_plot_labels();
   QString label_x = anno_label( attr_x );
   QString label_y = anno_label( attr_y );
   limit_key = QString( "%1-%2" ).arg( label_x ).arg( label_y );

   if ( sk_distro.empty() )
   {
      return;
   }

   build_xy_distro();
   resetSb();
   set_limits();
   replot_data();
}

// Select the coordinate for the vertical axis
void US_GA_Initialize::select_y_axis( int ival )
{
   attr_y = ival;
   rb_x_s   ->setEnabled( attr_y != ATTR_S );
   rb_x_ff0 ->setEnabled( attr_y != ATTR_K );
   rb_x_mw  ->setEnabled( attr_y != ATTR_W );
   rb_x_vbar->setEnabled( attr_y != ATTR_V );
   rb_x_D   ->setEnabled( attr_y != ATTR_D );
   rb_x_rh  ->setEnabled( attr_y != ATTR_R );
   update_plot_labels();
   QString label_x = anno_label( attr_x );
   QString label_y = anno_label( attr_y );
   limit_key = QString( "%1-%2" ).arg( label_x ).arg( label_y );

   if ( sk_distro.empty() )
   {
      return;
   }

   build_xy_distro();
   resetSb();
   set_limits();
   replot_data();
}

// Re-generate the XY version of the current distribution
void US_GA_Initialize::build_xy_distro()
{
   S_Solute sol_sk;
   S_Solute sol_xy;
   xy_distro.clear();

   for ( int jj = 0; jj < sk_distro.size(); jj++ )
   {
      sol_sk    = sk_distro[ jj ];
      sol_xy    = sol_sk;
      sol_xy.si = sol_sk.s;
      sol_xy.ki = sol_sk.k;

      sol_xy.s  = ( attr_x == ATTR_S ) ? sol_sk.s : sol_xy.s;
      sol_xy.s  = ( attr_x == ATTR_K ) ? sol_sk.k : sol_xy.s;
      sol_xy.s  = ( attr_x == ATTR_W ) ? sol_sk.w : sol_xy.s;
      sol_xy.s  = ( attr_x == ATTR_V ) ? sol_sk.v : sol_xy.s;
      sol_xy.s  = ( attr_x == ATTR_D ) ? sol_sk.d : sol_xy.s;
      sol_xy.s  = ( attr_x == ATTR_R ) ? sol_sk.r : sol_xy.s;
      sol_xy.k  = ( attr_y == ATTR_S ) ? sol_sk.s : sol_xy.k;
      sol_xy.k  = ( attr_y == ATTR_K ) ? sol_sk.k : sol_xy.k;
      sol_xy.k  = ( attr_y == ATTR_W ) ? sol_sk.w : sol_xy.k;
      sol_xy.k  = ( attr_y == ATTR_V ) ? sol_sk.v : sol_xy.k;
      sol_xy.k  = ( attr_y == ATTR_D ) ? sol_sk.d : sol_xy.k;
      sol_xy.k  = ( attr_y == ATTR_R ) ? sol_sk.r : sol_xy.k;

      xy_distro << sol_xy;
   }

   sort_distro( xy_distro, true );
}

void US_GA_Initialize::update_plot_labels()
{
   const QStringList labels = {"s", "f/f0", "MW", "Vbar", "D", "f", "Rh" };
   QString xlabel = labels.at( attr_x );
   QString ylabel = labels.at( attr_y );
   lb_plxmin->setText( tr( "Min. %1: " ).arg( xlabel ) );
   lb_plxmax->setText( tr( "Max. %1: " ).arg( xlabel ) );
   lb_plymin->setText( tr( "Min. %1: " ).arg( ylabel ) );
   lb_plymax->setText( tr( "Max. %1: " ).arg( ylabel ) );
   lb_wxbuck->setText( tr( "Width of "   ) + xlabel + tr( " Bucket:"  ) );
   lb_hybuck->setText( tr( "Height of "  ) + ylabel + tr( " Bucket:"  ) );
   xa_title = anno_title( attr_x );
   ya_title = anno_title( attr_y );
   data_plot->setAxisTitle( QwtPlot::xBottom, xa_title );
   data_plot->setAxisTitle( QwtPlot::yLeft,   ya_title );
}

// Set annotation title for a plot index
QString US_GA_Initialize::anno_title( int pltndx )
{
   QString a_title;

   if      ( pltndx == ATTR_S )
      a_title  = tr( "Sedimentation Coefficient (Sv)" );
   else if ( pltndx == ATTR_K )
      a_title  = tr( "Frictional Ratio f/f0" );
   else if ( pltndx == ATTR_W )
      a_title  = tr( "Molecular Weight (Da)" );
   else if ( pltndx == ATTR_V )
      a_title  = tr( "Vbar (ml / g)" );
   else if ( pltndx == ATTR_D )
      a_title  = tr( "Diffusion Coefficient (cm^2 / s)" );
   else if ( pltndx == ATTR_R )
      a_title  = tr( "Hydrodynamic Radius (nm)" );

   return a_title;
}

// Set annotation label for an attribute
QString US_GA_Initialize::anno_label( int id )
{
   QString title;

   switch ( id )
   {
   case ATTR_S:
      title = "s";
      break;
   case ATTR_K:
      title = "ff0";
      break;
   case ATTR_W:
      title = "mw";
      break;
   case ATTR_V:
      title = "vbar";
      break;
   case ATTR_D:
      title = "D";
      break;
   case ATTR_R:
      title = "Rh";
      break;
   }
   return title;
}

// Load bins from a saved gadistro file
void US_GA_Initialize::load_bins( void )
{
   if ( sk_distro.isEmpty() ) {
      return;
   }

DbgLv(1) << "gain: load_bins()";

   // Get a distro file
   QString filter = tr( "Bin GaDistro files (*gadistro.dat);;" )
         + tr( "Any DAT files (*.dat);;" )
         + tr( "Any files (*)" );
   QString fname  = QFileDialog::getOpenFileName( this,
      tr( "Load Solute Bin (GaDistro) File" ),
      binfpath, filter );

   if ( fname.isEmpty() )
      return;

   // Load the solute bins and plot them
   binfpath       = QString( fname ).section( "/", 0, -2 );
   int xparam, yparam, zparam;
   US_SoluteData sdata;
   if ( !sdata.loadGAdata( fname, &xparam, &yparam, &zparam ) )
   {
      QMessageBox::warning( this, "Warning!", "Wrong file format!" );
      return;
   }
DbgLv(1) << "gain:  ld_b : fname attr_x attr_y attr_z"
 << fname << attr_x << attr_y << attr_z;
   if ( xparam == attr_x && yparam == attr_y )
   {
      attr_x = xparam;
      attr_y = yparam;
      attr_z = zparam;
   }
   else
   {
      QString msg = "Plot X and Y parameters don't match with the file:\n";
      msg += tr("X : Plot=%1, File=%2\n").arg( anno_label( attr_x ), anno_label( xparam ) );
      msg += tr("Y : Plot=%1, File=%2\n").arg( anno_label( attr_y ), anno_label( yparam ) );
      QMessageBox::warning( this, "Warning!", msg );
      return;
   }

   resetSb();
   manbuks  = true;
   is_saved = false;
   soludata->loadGAdata( fname, &xparam, &yparam, &zparam );
   soludata->sortBuckets();
   fill_lw_bins();
   plot_buckets();
}

// Make a ColorMap copy and return a pointer to the new ColorMap
QwtLinearColorMap* US_GA_Initialize::ColorMapCopy( QwtLinearColorMap* colormap )
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

