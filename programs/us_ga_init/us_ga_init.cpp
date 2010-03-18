//! \file us_ga_init.cpp

#include <QApplication>

#include <uuid/uuid.h>

#include "us_ga_init.h"
#include "us_spectrodata.h"
#include "us_license_t.h"
#include "us_license.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_math.h"
#include "us_matrix.h"
#include "us_sleep.h"

#define PA_TMDIS_MS 5000   // default Plotall time per distro in milliseconds

// main program
int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );

   #include "main1.inc"

   // License is OK.  Start up.
   
   US_GA_Init w;
   w.show();                   //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication
}

// qSort LessThan method for Solute sort
bool distro_lessthan( const Solute &solu1, const Solute &solu2 )
{  // TRUE iff  (s1<s2) || (s1==s2 && k1<k2)
   return ( solu1.s < solu2.s ) ||
          ( ( solu1.s == solu2.s ) && ( solu1.k < solu2.k ) );
}

// US_GA_Init class constructor
US_GA_Init::US_GA_Init() : US_Widgets()
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

   int s_row = 0;

   // series of rows: most of them label on left, counter/box on right
   lb_info1      = us_banner( tr( "Genetic Algorithm Controls" ) );
   spec->addWidget( lb_info1, s_row++, 0, 1, 2 );

   lb_nisols     = us_label( tr( "Number of Initial Solutes:" ) );
   lb_nisols->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
   spec->addWidget( lb_nisols, s_row, 0 );

   ct_nisols     = us_counter( 3, 0.0, 1000.0, 0.0 );
   ct_nisols->setStep( 1 );
   spec->addWidget( ct_nisols, s_row++, 1 );
   connect( ct_nisols, SIGNAL( valueChanged( double ) ),
            this,      SLOT( update_nisols( double ) ) );

   lb_dafmin     = us_label( tr( "f/f0 minimum:" ) );
   lb_dafmin->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
   spec->addWidget( lb_dafmin, s_row, 0 );

   ct_dafmin       = us_counter( 3, 0.0, 49.0, 1.0 );
   ct_dafmin->setStep( 1 );
   spec->addWidget( ct_dafmin, s_row++, 1 );
   connect( ct_dafmin, SIGNAL( valueChanged( double ) ),
            this,      SLOT( update_dafmin( double ) ) );

   lb_dafmax       = us_label( tr( "f/f0 maximum:" ) );
   lb_dafmax->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
   spec->addWidget( lb_dafmax, s_row, 0 );

   ct_dafmax       = us_counter( 3, 1.0, 50.0, 4.0 );
   ct_dafmax->setStep( 1 );
   spec->addWidget( ct_dafmax, s_row++, 1 );
   connect( ct_dafmax, SIGNAL( valueChanged( double ) ),
            this,      SLOT( update_dafmax( double ) ) );

   lb_wsbuck     = us_label( tr( "Width of s Bucket:" ) );
   lb_wsbuck->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
   spec->addWidget( lb_wsbuck, s_row, 0 );

   ct_wsbuck     = us_counter( 3, 0.0, 1.0E20, 0.0 );
   ct_wsbuck->setStep( 1 );
   spec->addWidget( ct_wsbuck, s_row++, 1 );
   connect( ct_wsbuck, SIGNAL( valueChanged( double ) ),
            this,      SLOT( update_wsbuck( double ) ) );

   lb_wfbuck     = us_label( tr( "Width of f/f0 Bucket:" ) );
   lb_wfbuck->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
   spec->addWidget( lb_wfbuck, s_row, 0 );

   ct_wfbuck     = us_counter( 3, 0.0, 10000.0, 0.0 );
   ct_wfbuck->setStep( 1 );
   spec->addWidget( ct_wfbuck, s_row++, 1 );
   connect( ct_wfbuck, SIGNAL( valueChanged( double ) ),
            this,      SLOT( update_wfbuck( double ) ) );

   lb_info2      = us_banner( tr( "Pseudo-3D Controls" ) );
   spec->addWidget( lb_info2, s_row++, 0, 1, 2 );

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

   lb_autlim     = us_label( tr( "Automatic Plot Limits" ) );
   lb_autlim->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
   spec->addWidget( lb_autlim, s_row, 0 );

   us_checkbox( tr( "(unselect to override)" ), cb_autlim, true );
   spec->addWidget( cb_autlim, s_row++, 1 );
   connect( cb_autlim, SIGNAL( clicked() ),
            this,       SLOT( select_autolim() ) );

   lb_plfmin     = us_label( tr( "Plot Limit f/f0 min:" ) );
   lb_plfmin->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
   spec->addWidget( lb_plfmin, s_row, 0 );

   ct_plfmin     = us_counter( 3, 1.0, 50.0, 0.0 );
   ct_plfmin->setStep( 1 );
   spec->addWidget( ct_plfmin, s_row++, 1 );
   connect( ct_plfmin, SIGNAL( valueChanged( double ) ),
            this,        SLOT( update_plfmin( double ) ) );
   
   lb_plfmax     = us_label( tr( "Plot Limit f/f0 max:" ) );
   lb_plfmax->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
   spec->addWidget( lb_plfmax, s_row, 0 );
   
   ct_plfmax     = us_counter( 3, 1.0, 50.0, 1.0 );
   ct_plfmax->setStep( 1 );
   spec->addWidget( ct_plfmax, s_row++, 1 );
   connect( ct_plfmax, SIGNAL( valueChanged( double ) ),
         this,         SLOT( update_plfmax( double ) ) );

   lb_plsmin     = us_label( tr( "Plot Limit s min:" ) );
   lb_plsmin->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
   spec->addWidget( lb_plsmin, s_row, 0 );

   ct_plsmin     = us_counter( 3, -10.0, 10000.0, 0.0 );
   ct_plsmin->setStep( 1 );
   spec->addWidget( ct_plsmin, s_row++, 1 );
   connect( ct_plsmin, SIGNAL( valueChanged( double ) ),
            this,        SLOT( update_plsmin( double ) ) );

   lb_plsmax     = us_label( tr( "Plot Limit s max:" ) );
   lb_plsmax->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
   spec->addWidget( lb_plsmax, s_row, 0 );

   ct_plsmax     = us_counter( 3, 0.0, 10000.0, 0.0 );
   ct_plsmax->setStep( 1 );
   spec->addWidget( ct_plsmax, s_row++, 1 );
   connect( ct_plsmax, SIGNAL( valueChanged( double ) ),
            this,        SLOT( update_plsmax( double ) ) );

   te_pctl_help = us_textedit( );
   te_pctl_help->setText( tr( "                " ) );
   te_pctl_help->setReadOnly( true );
   spec->addWidget( te_pctl_help, s_row++, 0, 1, 2 );

   pb_lddistr    = us_pushbutton( tr( "Load Distribution" ) );
   pb_lddistr->setEnabled( true );
   spec->addWidget( pb_lddistr, s_row, 0 );
   connect( pb_lddistr, SIGNAL( clicked() ),
            this,       SLOT( load_distro() ) );

   us_checkbox( tr( "1-Dimensional Plot" ), cb_1dplot, true );
   spec->addWidget( cb_1dplot, s_row++, 1 );
   connect( cb_1dplot,  SIGNAL( clicked() ),
            this,       SLOT( select_plot1d() ) );

   pb_ldcolor    = us_pushbutton( tr( "Load Color File" ) );
   pb_ldcolor->setEnabled( true );
   spec->addWidget( pb_ldcolor, s_row, 0 );
   connect( pb_ldcolor, SIGNAL( clicked() ),
            this,       SLOT( load_color() ) );

   us_checkbox( tr( "2-Dimensional Plot" ), cb_2dplot, false );
   spec->addWidget( cb_2dplot, s_row++, 1 );
   connect( cb_2dplot, SIGNAL( clicked() ),
            this,       SLOT( select_plot2d() ) );

   pb_refresh    = us_pushbutton( tr( "Refresh Pseudo-3D Plot" ) );
   pb_refresh->setEnabled(  false );
   spec->addWidget( pb_refresh, s_row, 0 );
   connect( pb_refresh, SIGNAL( clicked() ),
            this,       SLOT( plot_3dim() ) );

   us_checkbox( tr( "Pseudo 3-D Plot" ),    cb_3dplot, false );
   spec->addWidget( cb_3dplot, s_row++, 1 );
   connect( cb_3dplot, SIGNAL( clicked() ),
            this,       SLOT( select_plot3d() ) );

   pb_mandrsb    = us_pushbutton( tr( "Manually Draw Bins" ) );
   pb_mandrsb->setEnabled( false );
   spec->addWidget( pb_mandrsb, s_row, 0 );
   connect( pb_mandrsb, SIGNAL( clicked() ),
            this,       SLOT( mandrawsb() ) );

   pb_shrnksb    = us_pushbutton( tr( "Shrink Solute Bins" ) );
   pb_shrnksb->setEnabled( false );
   spec->addWidget( pb_shrnksb, s_row++, 1 );
   connect( pb_shrnksb, SIGNAL( clicked() ),
            this,       SLOT( shrinksb() ) );

   pb_autassb    = us_pushbutton( tr( "Autoassign Solute Bins" ) );
   pb_autassb->setEnabled( false );
   spec->addWidget( pb_autassb, s_row, 0 );
   connect( pb_autassb, SIGNAL( clicked() ),
            this,       SLOT( autassignsb() ) );

   pb_resetsb    = us_pushbutton( tr( "Reset Solute Bins" ) );
   pb_resetsb->setEnabled( false );
   spec->addWidget( pb_resetsb, s_row++, 1 );
   connect( pb_resetsb, SIGNAL( clicked() ),
            this,       SLOT( resetsb() ) );

   pb_reset      = us_pushbutton( tr( "Reset" ) );
   pb_reset->setEnabled( false );
   spec->addWidget( pb_reset, s_row, 0 );
   connect( pb_reset,   SIGNAL( clicked() ),
            this,       SLOT( reset() ) );

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

   // set up plot component window on right side
   xa_title_s  = tr( "Sedimentation Coefficient corrected for water at 20" )
      + "<span>&deg;</span>C";
   xa_title_mw = tr( "Molecular Weight (Dalton)" );
   xa_title    = xa_title_s;

   QBoxLayout* plot = new US_Plot( data_plot, 
      tr( "Distribution Data" ),
      xa_title,
      tr( "Frictional Ratio f/f0" ) );

   rght->addLayout( plot );
   QBoxLayout* txed = new QHBoxLayout;
   te_sbin_data  = us_textedit( );
   te_sbin_data->setText( tr(
      "Please load a sedimentation coefficient distribution to initialize the"
      " genetic algorithm s-value range. The distribution should have a good"
      " resolution over the sedimentation coefficients. This distribution will"
      " be used to initialize all experiments used in the run, so the"
      " distribution taken from the experiment with the highest speed is"
      " probably the most appropriate distribution. You can use a distribution"
      " from the van Holde - Weischet method, the C(s) method, or 2-D Spectrum"
      " Analysis." ) );
   te_sbin_data->setReadOnly( true );
   txed->addWidget( te_sbin_data );
   rght->addLayout( txed );

   data_plot->setMinimumSize( 600, 600 );

   data_plot->enableAxis( QwtPlot::xBottom, true );
   data_plot->enableAxis( QwtPlot::yLeft,   true );
   data_plot->enableAxis( QwtPlot::yRight,  true );
   data_plot->setAxisScale( QwtPlot::xBottom, 1.0, 40.0 );
   data_plot->setAxisScale( QwtPlot::yLeft,   1.0,  4.0 );

   data_plot->setCanvasBackground( Qt::darkBlue );

   // put layouts together for overall layout
   left->addLayout( spec );
   left->addStretch();
   plot->addStretch();

   main->addLayout( left );
   main->addLayout( rght );

   // set up variables and initial state of GUI
   reset();
}

// reset the GUI
void US_GA_Init::reset( void )
{
   //data_plot->clear();
   data_plot->detachItems( );
   data_plot->replot();
 
   minmax     = false;
   zoom       = false;
   plot_dim   = 1;
   cb_1dplot->setChecked( plot_dim == 1 );  
   cb_2dplot->setChecked( plot_dim == 2 );
   cb_3dplot->setChecked( plot_dim == 3 );

   nisols     = 0;
   dafmin     = 1.0;
   dafmax     = 4.0;
   wsbuck     = 0.0;
   wfbuck     = 0.0;
   ct_nisols->setValue( (double)nisols );
   ct_dafmin->setValue( dafmin );
   ct_dafmax->setValue( dafmax );
   ct_wsbuck->setValue( wsbuck );
   ct_wfbuck->setValue( wfbuck );
   ct_wsbuck->setEnabled( false );
   ct_wfbuck->setEnabled( false );

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
   cb_autlim->setChecked( auto_lim );

   plfmin     = 1.0;
   plfmax     = 4.0;
   ct_plfmin->setRange( 0, 50, 0.01 );
   ct_plfmin->setValue( plfmin );
   ct_plfmin->setEnabled( false );
   ct_plfmax->setRange( 1, 50, 0.01 );
   ct_plfmax->setValue( plfmax );
   ct_plfmax->setEnabled( false );

   plsmin     = 1.0;
   plsmax     = 10.0;
   ct_plsmin->setRange( -10.0, 10000.0, 0.01 );
   ct_plsmin->setValue( plsmin );
   ct_plsmin->setEnabled( false );
   ct_plsmax->setRange( 0.0, 10000.0, 0.01 );
   ct_plsmax->setValue( plsmax );
   ct_plsmax->setEnabled( false );

   // default to white-cyan-magenta-red-black color map
   colormap  = new QwtLinearColorMap( Qt::white, Qt::black );
   colormap->addColorStop( 0.10, Qt::cyan );
   colormap->addColorStop( 0.50, Qt::magenta );
   colormap->addColorStop( 0.80, Qt::red );
   cmapname  = tr( "Default Color Map: w-cyan-magenta-red-black" );

   pb_reset->setEnabled( false );
}

// save the GA data
void US_GA_Init::save( void )
{
}

// manually draw solute bins
void US_GA_Init::mandrawsb( void )
{
   pb_shrnksb->setEnabled( true );

   pick      = new US_PlotPicker( data_plot );
   QColor bg = data_plot->canvasBackground();
   int csum  = bg.red() + bg.green() + bg.blue();
qDebug() << "Canvas BG rgb sum = " << csum;

   if ( csum > 600 )
   {
      pick->setRubberBandPen( QPen( QColor( Qt::black ) ) );
      pick->setTrackerPen(    QPen( QColor( Qt::black ) ) );
qDebug() << "   (c)PEN=black ==" << QColor( Qt::black );
   }
   else
   {
      pick->setRubberBandPen( QPen( QColor( Qt::white ) ) );
      pick->setTrackerPen(    QPen( QColor( Qt::white ) ) );
qDebug() << "   (c)PEN=white ==" << QColor( Qt::white );
   }
qDebug() << "     (c)PICKPEN=" << pick->rubberBandPen().color();
   pick->setRubberBand(     QwtPicker::RectRubberBand );
   pick->setSelectionFlags( QwtPicker::PointSelection
                          | QwtPicker::ClickSelection );
   pick->setSelectionFlags( QwtPicker::RectSelection
                          | QwtPicker::DragSelection );

   connect( pick, SIGNAL(  mouseDown( const QwtDoublePoint& ) ),
            this, SLOT( getMouseDown( const QwtDoublePoint& ) ) );
   connect( pick, SIGNAL( mouseUp(    const QwtDoublePoint& ) ),
            this, SLOT( getMouseUp(   const QwtDoublePoint& ) ) );

}
// shrink solute bins
void US_GA_Init::shrinksb( void )
{
}
// auto assign solute bins
void US_GA_Init::autassignsb( void )
{
   pb_resetsb->setEnabled( true );
   pb_save->setEnabled(    true );
}
// reset solute bins
void US_GA_Init::resetsb( void )
{
}

// plot data 1-D
void US_GA_Init::plot_1dim( void )
{
   //data_plot->clear();
   //data_plot->detachItems( QwtPlotItem::Rtti_PlotSpectrogram );
   data_plot->detachItems();

   data_plot->setCanvasBackground( Qt::black );

   int     dsize = s_distro.size();
   double* x     = new double[ dsize ];
   double* y     = new double[ dsize ];
   double  cmin  = 1.0e30;
   double  cmax  = -1.0e30;
   double  smin  = 1.0e30;
   double  smax  = -1.0e30;

   for ( int jj = 0; jj < dsize; jj++ )
   {
      double sval = s_distro.at( jj ).s;
      double cval = s_distro.at( jj ).c;
      x[ jj ]     = sval;
      y[ jj ]     = cval;
      smin        = ( smin < sval ) ? smin : sval;
      smax        = ( smax > sval ) ? smax : sval;
      cmin        = ( cmin < cval ) ? cmin : cval;
      cmax        = ( cmax > cval ) ? cmax : cval;
   }

   double rdif = ( smax - smin ) / 20.0;
   smin       -= rdif;
   smax       += rdif;
   rdif        = ( cmax - cmin ) / 20.0;
   cmin       -= rdif;
   cmax       += rdif;
   smin        = ( smin > 0.0 ) ? smin : 0.0;
   cmin        = ( cmin > 0.0 ) ? cmin : 0.0;

   QwtPlotGrid* data_grid = us_grid( data_plot );
   data_grid->enableYMin( true );
   data_grid->enableY( true );
   data_grid->setMajPen( QPen( US_GuiSettings::plotMajGrid(),
      0, Qt::DashLine ) );
   data_grid->attach( data_plot );

   QwtPlotCurve *data_curv = us_curve( data_plot, "distro" );
   data_curv->setData( x, y, dsize );
   data_curv->setPen( QPen( Qt::yellow, 3, Qt::SolidLine ) );
   data_curv->setStyle( QwtPlotCurve::Sticks );

   delete [] x;
   delete [] y;
   data_plot->setAxisAutoScale( QwtPlot::xBottom );
   data_plot->setAxisAutoScale( QwtPlot::yLeft );
   data_plot->enableAxis( QwtPlot::yRight, FALSE );
   data_plot->setAxisTitle( QwtPlot::xBottom, xa_title_s );
   data_plot->setAxisTitle( QwtPlot::yLeft,   tr( "Relative Frequency" ) );
   data_plot->setAxisScale( QwtPlot::xBottom, smin, smax );
   data_plot->setAxisScale( QwtPlot::yLeft,   cmin, cmax );

   data_plot->replot();
   pb_reset->setEnabled( true );
}

// plot data 2-D
void US_GA_Init::plot_2dim( void )
{
   //data_plot->clear();
   //data_plot->detachItems( QwtPlotItem::Rtti_PlotSpectrogram );
   data_plot->detachItems();

   data_plot->setCanvasBackground( Qt::black );

   int     dsize = s_distro.size();
   double* x     = new double[ dsize ];
   double* y     = new double[ dsize ];
   double  smin  = 1.0e30;
   double  smax  = -1.0e30;
   double  fmin  = 1.0e30;
   double  fmax  = -1.0e30;

   for ( int jj = 0; jj < dsize; jj++ )
   {
      double sval = s_distro.at( jj ).s;
      double fval = s_distro.at( jj ).k;
      x[ jj ]     = sval;
      y[ jj ]     = fval;
      smin        = ( smin < sval ) ? smin : sval;
      smax        = ( smax > sval ) ? smax : sval;
      fmin        = ( fmin < fval ) ? fmin : fval;
      fmax        = ( fmax > fval ) ? fmax : fval;
   }

   double rdif = ( smax - smin ) / 20.0;
   smin       -= rdif;
   smax       += rdif;
   rdif        = ( fmax - fmin ) / 20.0;
   fmin       -= rdif;
   fmax       += rdif;
   smin        = ( smin > 0.0 ) ? smin : 0.0;
   fmin        = ( fmin > 0.0 ) ? fmin : 0.0;

   QwtPlotGrid* data_grid = us_grid( data_plot );
   data_grid->enableYMin( true );
   data_grid->enableY( true );
   data_grid->setMajPen( QPen( US_GuiSettings::plotMajGrid(),
      0, Qt::DashLine ) );
   data_grid->attach( data_plot );

   QwtPlotCurve *data_curv = us_curve( data_plot, "distro" );
   QwtSymbol symbol;

   symbol.setStyle( QwtSymbol::Ellipse );
   symbol.setPen( QPen( Qt::red ) );
   symbol.setBrush( QBrush( Qt::yellow ) );
   if ( dsize < 100  &&  dsize > 50 )
      symbol.setSize( 8 );
   else if ( dsize < 50  &&  dsize > 20 )
      symbol.setSize( 10 );
   else if ( dsize < 21 )
      symbol.setSize( 12 );
   else if ( dsize > 100 )
      symbol.setSize( 6 );

   data_curv->setStyle( QwtPlotCurve::NoCurve );
   data_curv->setSymbol( symbol );
   data_curv->setData( x, y, dsize );

   delete [] x;
   delete [] y;
   data_plot->setAxisAutoScale( QwtPlot::xBottom );
   data_plot->setAxisAutoScale( QwtPlot::yLeft );
   data_plot->enableAxis( QwtPlot::yRight, FALSE );
   data_plot->setAxisTitle( QwtPlot::xBottom, tr( "s20,W (cm^2/sec)" ) );
   data_plot->setAxisTitle( QwtPlot::yLeft,   tr( "Frictional Ratio f/f0" ) );
   data_plot->setAxisScale( QwtPlot::xBottom, smin, smax );
   data_plot->setAxisScale( QwtPlot::yLeft,   fmin, fmax );

#if 0
   pick      = new US_PlotPicker( data_plot );
   pick->setRubberBand(     QwtPicker::RectRubberBand );
   pick->setSelectionFlags( QwtPicker::RectSelection
                          | QwtPicker::DragSelection );

   connect( pick, SIGNAL(  mouseDown( const QwtDoublePoint& ) ),
            this, SLOT( getMouseDown( const QwtDoublePoint& ) ) );
   connect( pick, SIGNAL( mouseUp(    const QwtDoublePoint& ) ),
            this, SLOT( getMouseUp(   const QwtDoublePoint& ) ) );
#endif

   data_plot->replot();
   pb_reset->setEnabled( true );
}

// plot data 3-D
void US_GA_Init::plot_3dim( void )
{
   data_plot->detachItems();

   data_plot->setCanvasBackground( colormap->color1() ); 
   QString tstr = run_name + "." + cell + wavelength + "\n" + method;
   data_plot->setTitle( tstr );

   // set up spectrogram data
   d_spectrogram = new QwtPlotSpectrogram();
   d_spectrogram->setData( US_SpectrogramData() );
   d_spectrogram->setColorMap( *colormap );

   US_SpectrogramData& spec_dat = (US_SpectrogramData&)d_spectrogram->data();

   spec_dat.setRastRanges( xreso, yreso, resolu, zfloor );
   spec_dat.setRaster( s_distro );

   d_spectrogram->attach( data_plot );

   // set color map and axis settings
   QwtScaleWidget *rightAxis = data_plot->axisWidget( QwtPlot::yRight );
   rightAxis->setColorBarEnabled( true );
   rightAxis->setColorMap( spec_dat.range(), d_spectrogram->colorMap() );
   data_plot->setAxisTitle( QwtPlot::xBottom, xa_title_s );
   data_plot->setAxisTitle( QwtPlot::yRight, "Frequency" );
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
      data_plot->setAxisScale( QwtPlot::xBottom, plsmin, plsmax );
      data_plot->setAxisScale( QwtPlot::yLeft,   plfmin, plfmax );
   }

#if 0
   pick      = new US_PlotPicker( data_plot );
   QColor bg = colormap->color1();
   int csum  = bg.red() + bg.green() + bg.blue();
qDebug() << "BG Colormap rgb sum = " << csum;

   if ( csum > 600 )
   {
      pick->setRubberBandPen( QPen( QColor( Qt::black ) ) );
      pick->setTrackerPen(    QPen( QColor( Qt::black ) ) );
qDebug() << "   PEN=black ==" << QColor( Qt::black );
   }
   else
   {
      pick->setRubberBandPen( QPen( QColor( Qt::white ) ) );
      pick->setTrackerPen(    QPen( QColor( Qt::white ) ) );
qDebug() << "   PEN=white ==" << QColor( Qt::white );
   }
qDebug() << "     PICKPEN=" << pick->rubberBandPen().color();
   pick->setRubberBand(     QwtPicker::RectRubberBand );
   pick->setSelectionFlags( QwtPicker::PointSelection
                          | QwtPicker::ClickSelection );
   pick->setSelectionFlags( QwtPicker::RectSelection
                          | QwtPicker::DragSelection );

   connect( pick, SIGNAL(  mouseDown( const QwtDoublePoint& ) ),
            this, SLOT( getMouseDown( const QwtDoublePoint& ) ) );
   connect( pick, SIGNAL( mouseUp(    const QwtDoublePoint& ) ),
            this, SLOT( getMouseUp(   const QwtDoublePoint& ) ) );
#endif

   data_plot->replot();

   pb_reset->setEnabled( true );
}

void US_GA_Init::update_resolu( double dval )
{
   resolu = dval;
}

void US_GA_Init::update_xreso( double dval )
{
   xreso  = dval;
}

void US_GA_Init::update_yreso( double dval )
{
   yreso  = dval;
}

void US_GA_Init::update_zfloor( double dval )
{
   zfloor = dval;
}

void US_GA_Init::update_nisols( double dval )
{
   nisols    = dval;
}

void US_GA_Init::update_dafmin( double dval )
{
   dafmin    = dval;
}

void US_GA_Init::update_dafmax( double dval )
{
   dafmax    = dval;
}

void US_GA_Init::update_wsbuck( double dval )
{
   wsbuck    = dval;
}

void US_GA_Init::update_wfbuck( double dval )
{
   wfbuck    = dval;
}

void US_GA_Init::update_plsmin( double dval )
{
   plsmin    = dval;
}

void US_GA_Init::update_plsmax( double dval )
{
   plsmax    = dval;
}

void US_GA_Init::update_plfmin( double dval )
{
   plfmin    = dval;
}

void US_GA_Init::update_plfmax( double dval )
{
   plfmax    = dval;
}

void US_GA_Init::select_autolim()
{
   auto_lim   = cb_autlim->isChecked();
   ct_plfmin->setEnabled( !auto_lim );
   ct_plfmax->setEnabled( !auto_lim );
   ct_plsmin->setEnabled( !auto_lim );
   ct_plsmax->setEnabled( !auto_lim );
   if ( auto_lim )
   {
      set_limits();
   }
   else
   {
      ct_plsmin->setRange( -10.0, 10000.0, 0.01 );
      ct_plsmax->setRange( 0.0, 10000.0, 0.01 );
   }
}

void US_GA_Init::select_plot1d()
{
   plot_dim   = 1;
   cb_2dplot->disconnect();
   cb_3dplot->disconnect();
   cb_2dplot->setChecked( false );
   cb_3dplot->setChecked( false );

   cb_1dplot->setEnabled( false );
   cb_2dplot->setEnabled( true );
   cb_3dplot->setEnabled( true );
   pb_refresh->setEnabled( false );

   connect( cb_2dplot,  SIGNAL( clicked() ),
            this,       SLOT( select_plot2d() ) );
   connect( cb_3dplot,  SIGNAL( clicked() ),
            this,       SLOT( select_plot3d() ) );

   if ( s_distro.size() > 0 )
   {
      plot_1dim();
   }
}

void US_GA_Init::select_plot2d()
{
   plot_dim   = 2;
   cb_1dplot->disconnect();
   cb_3dplot->disconnect();
   cb_1dplot->setChecked( false );
   cb_3dplot->setChecked( false );

   cb_1dplot->setEnabled( true );
   cb_2dplot->setEnabled( false );
   cb_3dplot->setEnabled( true );
   pb_refresh->setEnabled( false );

   connect( cb_1dplot,  SIGNAL( clicked() ),
            this,       SLOT( select_plot1d() ) );
   connect( cb_3dplot,  SIGNAL( clicked() ),
            this,       SLOT( select_plot3d() ) );

   if ( s_distro.size() > 0 )
   {
      plot_2dim();
   }

   pb_mandrsb->setEnabled( true );
}

void US_GA_Init::select_plot3d()
{
   plot_dim   = 3;
   cb_1dplot->disconnect();
   cb_2dplot->disconnect();
   cb_1dplot->setChecked( false );
   cb_2dplot->setChecked( false );

   cb_1dplot->setEnabled( true );
   cb_2dplot->setEnabled( true );
   cb_3dplot->setEnabled( false );
   pb_refresh->setEnabled( true );

   connect( cb_1dplot,  SIGNAL( clicked() ),
            this,       SLOT( select_plot1d() ) );
   connect( cb_2dplot,  SIGNAL( clicked() ),
            this,       SLOT( select_plot2d() ) );

   if ( s_distro.size() > 0 )
      plot_3dim();

   pb_mandrsb->setEnabled( true );
}

void US_GA_Init::load_distro()
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
void US_GA_Init::load_distro( const QString& fname )
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

   Solute      sol_s;
   Solute      sol_mw;

   // set values based on file name
qDebug() << "Load Distro, fname=" << fname;
   QFileInfo fi( fname );
   int jj       = fname.lastIndexOf( "." );
   int kk       = fname.length();
   QString tstr = fname.right( kk - jj - 1 );

   cell         = tstr.left( 1 );
   tstr         = fname.right( kk - jj - 2 );
   wavelength   = tstr;
   run_name     = fi.baseName();
   distro_type  = 0;

   // find type in table and set values accordingly
   for ( jj = 0; jj < ncdte; jj += 3 )
   {
      QString fnp( cdtyp[ jj ] );

      if ( fname.contains( fnp, Qt::CaseInsensitive ) )
      {
         distro_type = jj / 3 + 1;
         monte_carlo = QString( cdtyp[ jj+2 ] ).contains( "T" );
         method      = QString( cdtyp[ jj+1 ] );
         break;
      }
   }
   s_distro.clear();
   mw_distro.clear();

   tstr    = run_name + "." + cell + wavelength + "\n" + method;
   data_plot->setTitle( tstr );

   // read in and set distribution s,c,k values
   if ( distro_type > 0 )
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

         if ( monte_carlo )
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

            s_distro.append( sol_s );
            mw_distro.append( sol_mw );
         }
         filei.close();
      }

      // sort and reduce distributions
      sort_distro( s_distro, true );
      sort_distro( mw_distro, true );
   }

   if ( auto_lim )
   {
      set_limits();

      ct_plfmin->setEnabled( false );
      ct_plfmax->setEnabled( false );
      ct_plsmin->setEnabled( false );
      ct_plsmax->setEnabled( false );
   }
   else
   {
      plsmin    = ct_plsmin->value();
      plsmax    = ct_plsmax->value();
      plfmin    = ct_plfmin->value();
      plfmax    = ct_plfmax->value();
      ct_plsmin->setRange( -10.0, 10000.0, 0.01 );
      ct_plsmax->setRange( 0.0, 10000.0, 0.01 );
   }
   data_plot->setAxisScale( QwtPlot::xBottom, plsmin, plsmax );
   data_plot->setAxisScale( QwtPlot::yLeft,   plfmin, plfmax );

   pb_resetsb->setEnabled( true );

   if ( plot_dim == 1 )
   {
      pb_refresh->setEnabled( false );
      plot_1dim();
   }
   else if ( plot_dim == 2 )
   {
      pb_refresh->setEnabled( false );
      plot_2dim();
   }
   else
   {
      pb_refresh->setEnabled( true );
      plot_3dim();
   }
   pb_autassb->setEnabled( true );
}

void US_GA_Init::load_color()
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
}

void US_GA_Init::set_limits()
{
   double fmin = 1.0e30;
   double fmax = -1.0e30;
   double smin = 1.0e30;
   double smax = -1.0e30;
   double rdif;

   {
      data_plot->setAxisTitle( QwtPlot::xBottom, xa_title_s );

      // find min,max for S distributions
      for ( int jj = 0; jj < s_distro.size(); jj++ )
      {
         double sval = s_distro.at( jj ).s;
         double fval = s_distro.at( jj ).k;
         smin        = ( smin < sval ) ? smin : sval;
         smax        = ( smax > sval ) ? smax : sval;
         fmin        = ( fmin < fval ) ? fmin : fval;
         fmax        = ( fmax > fval ) ? fmax : fval;
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

      ct_plsmax->setRange( 0.0, rmin, rinc );
      ct_plsmin->setRange( -( smax / 50.0 ), rmin, rinc );

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

      ct_plsmin->setValue( smin );
      ct_plsmax->setValue( smax );
      ct_plfmin->setValue( fmin );
      ct_plfmax->setValue( fmax );

      plsmin    = smin;
      plsmax    = smax;
      plfmin    = fmin;
      plfmax    = fmax;
   }
   else
   {
      plsmin    = ct_plsmin->value();
      plsmax    = ct_plsmax->value();
      plfmin    = ct_plfmin->value();
      plfmax    = ct_plfmax->value();
      ct_plsmin->setRange( -10.0, 10000.0, 0.01 );
      ct_plsmax->setRange( 0.0, 10000.0, 0.01 );
   }
   te_sbin_data->setText( tr(
      "Now either auto-assign the solute bins, or manually select bins"
      " by clicking first on the lower and then on the upper limit of"
      " the bin range. If you auto-assign the bins you should first"
      " select the number of solute bins you want to use. UltraScan will"
      " space the bins proportional to the integral value of each peak,"
      " such that each bin contains the same integral value."
      " You can select each solute bin from the listbox on the left"
      " and modify the frictional ratio limits by selecting them first"
      " with the respective counters. To change the frictional ratios for"
      " the solutes, change to the desired f/f0 values in the counter, then"
      " double-click on the listbox item to change the selected bin"
      " accordingly." ) );
}

// Sort distribution solute list by s,k values and optionally reduce
void US_GA_Init::sort_distro( QList< Solute >& listsols,
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

      if ( reduced.size() < sizi )
      {   // if some reduction happened, replace list with reduced version
         listsols = reduced;
      }
   }
   return;
}
void US_GA_Init::highlight_solute( int sx )
{
qDebug() << "highlight solute # " << sx;
}
void US_GA_Init::getMouseDown( const QwtDoublePoint& p )
{
   p1   = p;
qDebug() << "MPressed x,y " << p1.x() << "," << p1.y();
}
void US_GA_Init::getMouseUp( const QwtDoublePoint& p )
{
   p2   = p;
qDebug() << "MReleased x,y " << p2.x() << "," << p2.y();
qDebug() << "       (U)PICKPEN=" << pick->rubberBandPen().color();
}

