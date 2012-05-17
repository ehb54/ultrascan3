//! \file us_ga_init.cpp

#include <QApplication>

#include "us_ga_init.h"
#include "us_select_edits.h"
#include "us_model_loader.h"
#include "us_license_t.h"
#include "us_license.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_math2.h"
#include "us_matrix.h"
#include "us_constants.h"
#include "us_passwd.h"
#include "us_report.h"

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

// qSort LessThan method for Solute sort
bool distro_lessthan( const Solute &solu1, const Solute &solu2 )
{  // TRUE iff  (s1<s2) || (s1==s2 && k1<k2)
   return ( solu1.s < solu2.s ) ||
          ( ( solu1.s == solu2.s ) && ( solu1.k < solu2.k ) );
}

const double epsilon = 0.0005;   // equivalence magnitude ratio radius

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

   // series of rows: most of them label on left, counter/box on right
   lb_info1      = us_banner( tr( "Genetic Algorithm Controls" ) );

   lb_nisols     = us_label( tr( "Number of Initial Solutes:" ) );
   lb_nisols->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );

   ct_nisols     = us_counter( 3, 0.0, 1000.0, 0.0 );
   ct_nisols->setStep( 1 );
   ct_nisols->setEnabled( true );
   connect( ct_nisols, SIGNAL( valueChanged(  double ) ),
            this,      SLOT(   update_nisols( double ) ) );

   lb_wsbuck     = us_label( tr( "Width of s Bucket:" ) );
   lb_wsbuck->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );

   ct_wsbuck     = us_counter( 3, 0.0, 10.0, 0.0 );
   ct_wsbuck->setStep( 1 );
   connect( ct_wsbuck, SIGNAL( valueChanged(  double ) ),
            this,      SLOT(   update_wsbuck( double ) ) );

   lb_hfbuck     = us_label( tr( "Height of f/f0 Bucket:" ) );
   lb_hfbuck->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );

   ct_hfbuck     = us_counter( 3, 0.0, 1.0, 0.0 );
   ct_hfbuck->setStep( 1 );
   connect( ct_hfbuck, SIGNAL( valueChanged(  double ) ),
            this,      SLOT(   update_hfbuck( double ) ) );

   lb_info2      = us_banner( tr( "Pseudo-3D Controls" ) );

   lb_resolu     = us_label( tr( "Pseudo-3D Resolution:" ) );
   lb_resolu->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );

   ct_resolu     = us_counter( 3, 0.0, 100.0, 90.0 );
   ct_resolu->setStep( 1 );
   connect( ct_resolu, SIGNAL( valueChanged( double ) ),
            this,      SLOT( update_resolu( double ) ) );

   lb_xreso      = us_label( tr( "X Resolution:" ) );
   lb_xreso->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );

   ct_xreso      = us_counter( 3, 10.0, 1000.0, 0.0 );
   ct_xreso->setStep( 1 );
   connect( ct_xreso,  SIGNAL( valueChanged( double ) ),
            this,      SLOT( update_xreso( double ) ) );

   lb_yreso      = us_label( tr( "Y Resolution:" ) );
   lb_yreso->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );

   ct_yreso      = us_counter( 3, 10.0, 1000.0, 0.0 );
   ct_yreso->setStep( 1 );
   connect( ct_yreso,  SIGNAL( valueChanged( double ) ),
            this,      SLOT( update_yreso( double ) ) );

   lb_zfloor     = us_label( tr( "Z Floor Percent:" ) );
   lb_zfloor->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );

   ct_zfloor     = us_counter( 3, 0.0, 50.0, 1.0 );
   ct_zfloor->setStep( 1 );
   connect( ct_zfloor, SIGNAL( valueChanged( double ) ),
            this,      SLOT( update_zfloor( double ) ) );

   lb_autlim     = us_label( tr( "Automatic Plot Limits" ) );
   lb_autlim->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );

   us_checkbox( tr( "(unselect to override)             " ), cb_autlim, true );
   connect( cb_autlim, SIGNAL( clicked() ),
            this,       SLOT( select_autolim() ) );

   lb_plfmin     = us_label( tr( "Plot Limit f/f0 Min:" ) );
   lb_plfmin->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );

   ct_plfmin     = us_counter( 3, 0.5, 50.0, 0.0 );
   ct_plfmin->setStep( 1 );
   connect( ct_plfmin, SIGNAL( valueChanged( double ) ),
            this,        SLOT( update_plfmin( double ) ) );
   
   lb_plfmax     = us_label( tr( "Plot Limit f/f0 Max:" ) );
   lb_plfmax->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
   
   ct_plfmax     = us_counter( 3, 1.0, 50.0, 1.0 );
   ct_plfmax->setStep( 1 );
   ct_plfmax->setValue( 1.34567e+01 );
   connect( ct_plfmax, SIGNAL( valueChanged( double ) ),
         this,         SLOT( update_plfmax( double ) ) );

   lb_plsmin     = us_label( tr( "Plot Limit s Min:" ) );
   lb_plsmin->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );

   ct_plsmin     = us_counter( 3, -10000.0, 10000.0, 0.0 );
   ct_plsmin->setStep( 1 );
   connect( ct_plsmin, SIGNAL( valueChanged( double ) ),
            this,        SLOT( update_plsmin( double ) ) );

   lb_plsmax     = us_label( tr( "Plot Limit s Max:" ) );
   ct_plsmax     = us_counter( 3, 0.0, 10000.0, 0.0 );
   ct_plsmax->setStep( 1 );
   connect( ct_plsmax, SIGNAL( valueChanged( double ) ),
            this,        SLOT( update_plsmax( double ) ) );

   lw_sbin_data = us_listwidget( );
   lw_sbin_data->installEventFilter( this );
   connect( lw_sbin_data, SIGNAL( clicked(       const QModelIndex& ) ),
            this,         SLOT(   sclick_sbdata( const QModelIndex& ) ) );
   connect( lw_sbin_data, SIGNAL( doubleClicked( const QModelIndex& ) ),
            this,         SLOT(   dclick_sbdata( const QModelIndex& ) ) );
   connect( lw_sbin_data, SIGNAL( currentRowChanged( int )            ),
            this,         SLOT(   newrow_sbdata(     int )            ) );

   us_checkbox( tr( "Plot f/f0 VS s" ), cb_plot_s, true );
   connect( cb_plot_s,  SIGNAL( clicked() ),
            this,       SLOT( select_plot_s() ) );

   us_checkbox( tr( "Plot f/f0 VS mw" ), cb_plot_mw, false );
   connect( cb_plot_mw, SIGNAL( clicked() ),
            this,       SLOT( select_plot_mw() ) );

   dkdb_cntrls   = new US_Disk_DB_Controls(
         US_Settings::default_data_location() );
   connect( dkdb_cntrls, SIGNAL( changed( bool ) ),
            this,   SLOT( update_disk_db( bool ) ) );

   pb_prefilt    = us_pushbutton( tr( "Select PreFilter" ) );
   pb_prefilt->setEnabled( true );
   connect( pb_prefilt, SIGNAL( clicked() ),
            this,       SLOT(   select_prefilt() ) );

   le_prefilt    = us_lineedit  ( "", -1, true );

   pb_lddistr    = us_pushbutton( tr( "Load Distribution" ) );
   pb_lddistr->setEnabled( true );
   connect( pb_lddistr, SIGNAL( clicked() ),
            this,       SLOT(   load_distro() ) );

   us_checkbox( tr( "1-Dimensional Plot" ), cb_1dplot, false );
   connect( cb_1dplot,  SIGNAL( clicked() ),
            this,       SLOT(   select_plot1d() ) );

   pb_ldcolor    = us_pushbutton( tr( "Load Color File" ) );
   pb_ldcolor->setEnabled( true );
   connect( pb_ldcolor, SIGNAL( clicked() ),
            this,       SLOT(   load_color() ) );

   us_checkbox( tr( "2-Dimensional Plot" ), cb_2dplot, false );
   connect( cb_2dplot, SIGNAL( clicked() ),
            this,       SLOT(  select_plot2d() ) );

   pb_refresh    = us_pushbutton( tr( "Refresh Plot" ) );
   pb_refresh->setEnabled(  false );
   connect( pb_refresh, SIGNAL( clicked() ),
            this,       SLOT(   replot_data() ) );

   us_checkbox( tr( "Pseudo 3-D Plot" ),    cb_3dplot, true  );
   connect( cb_3dplot, SIGNAL( clicked() ),
            this,       SLOT(  select_plot3d() ) );

   pb_mandrsb    = us_pushbutton( tr( "Manually Draw Bins" ) );
   pb_mandrsb->setEnabled( false );
   connect( pb_mandrsb, SIGNAL( clicked() ),
            this,       SLOT(   manDrawSb() ) );

   pb_ckovrlp    = us_pushbutton( tr( "Check for Bin Overlaps" ) );
   pb_ckovrlp->setEnabled( false );
   connect( pb_ckovrlp, SIGNAL( clicked()       ),
            this,       SLOT(   checkOverlaps() ) );

   pb_autassb    = us_pushbutton( tr( "Autoassign Solute Bins" ) );
   pb_autassb->setEnabled( false );
   connect( pb_autassb, SIGNAL( clicked() ),
            this,       SLOT(   autoAssignSb() ) );

   pb_resetsb    = us_pushbutton( tr( "Reset Solute Bins" ) );
   pb_resetsb->setEnabled( false );
   connect( pb_resetsb, SIGNAL( clicked() ),
            this,       SLOT( resetSb() ) );

   pb_reset      = us_pushbutton( tr( "Reset" ) );
   pb_reset->setEnabled( false );
   connect( pb_reset,   SIGNAL( clicked() ),
            this,       SLOT(   reset() ) );

   pb_view       = us_pushbutton( tr( "View Statistics" ) );
   pb_view ->setEnabled( false );
   connect( pb_view,    SIGNAL( clicked() ),
            this,       SLOT(   view()  ) );

   pb_save       = us_pushbutton( tr( "Save" ) );
   pb_save->setEnabled( false );
   connect( pb_save,    SIGNAL( clicked() ),
            this,       SLOT(   save() ) );

   pb_help       = us_pushbutton( tr( "Help" ) );
   pb_help->setEnabled( true );
   connect( pb_help,    SIGNAL( clicked() ),
            this,       SLOT(   help() ) );

   pb_close      = us_pushbutton( tr( "Close" ) );
   pb_close->setEnabled( true );
   connect( pb_close,   SIGNAL( clicked() ),
            this,       SLOT(   close() ) );

   QPalette pa( lb_info1->palette() );
   te_status    = us_textedit( );
   te_status->setPalette( pa );
   te_status->setTextBackgroundColor( pa.color( QPalette::Window ) );
   te_status->setTextColor( pa.color( QPalette::WindowText ) );
   dfilname     = "(NONE)";
   stcmline     = tr( "Color Map:  the default w-cyan-magenta-red-black" );
   stdiline     = tr( "The distribution was loaded from the file:" );
   stdfline     = "  " + dfilname;
   stnpline     = tr( "The number of distribution points is 0." );
   te_status->setText( stcmline + "\n" + stdiline + "\n"
         + stdfline + "\n" + stnpline );

   int s_row = 0;
   spec->addWidget( lb_info1,     s_row++, 0, 1, 4 );
   spec->addWidget( lb_nisols,    s_row,   0, 1, 2 );
   spec->addWidget( ct_nisols,    s_row++, 2, 1, 2 );
   spec->addWidget( lb_wsbuck,    s_row,   0, 1, 2 );
   spec->addWidget( ct_wsbuck,    s_row++, 2, 1, 2 );
   spec->addWidget( lb_hfbuck,    s_row,   0, 1, 2 );
   spec->addWidget( ct_hfbuck,    s_row++, 2, 1, 2 );
   spec->addWidget( lb_info2,     s_row++, 0, 1, 4 );
   spec->addWidget( lb_resolu,    s_row,   0, 1, 2 );
   spec->addWidget( ct_resolu,    s_row++, 2, 1, 2 );
   spec->addWidget( lb_xreso,     s_row,   0, 1, 2 );
   spec->addWidget( ct_xreso,     s_row++, 2, 1, 2 );
   spec->addWidget( lb_yreso,     s_row,   0, 1, 2 );
   spec->addWidget( ct_yreso,     s_row++, 2, 1, 2 );
   spec->addWidget( lb_zfloor,    s_row,   0, 1, 2 );
   spec->addWidget( ct_zfloor,    s_row++, 2, 1, 2 );
   spec->addWidget( lb_autlim,    s_row,   0, 1, 2 );
   spec->addWidget( cb_autlim,    s_row++, 2, 1, 2 );
   spec->addWidget( lb_plfmin,    s_row,   0, 1, 2 );
   spec->addWidget( ct_plfmin,    s_row++, 2, 1, 2 );
   spec->addWidget( lb_plfmax,    s_row,   0, 1, 2 );
   spec->addWidget( ct_plfmax,    s_row++, 2, 1, 2 );
   spec->addWidget( lb_plsmin,    s_row,   0, 1, 2 );
   spec->addWidget( ct_plsmin,    s_row++, 2, 1, 2 );
   spec->addWidget( lb_plsmax,    s_row,   0, 1, 2 );
   spec->addWidget( ct_plsmax,    s_row++, 2, 1, 2 );
   spec->addWidget( lw_sbin_data, s_row++, 0, 1, 4 );
   spec->addWidget( cb_plot_s,    s_row,   0, 1, 2 );
   spec->addWidget( cb_plot_mw,   s_row++, 2, 1, 2 );
   spec->addLayout( dkdb_cntrls,  s_row++, 0, 1, 4 );
   spec->addWidget( pb_prefilt,   s_row,   0, 1, 2 );
   spec->addWidget( le_prefilt,   s_row++, 2, 1, 2 );
   spec->addWidget( pb_lddistr,   s_row,   0, 1, 2 );
   spec->addWidget( cb_1dplot,    s_row++, 2, 1, 2 );
   spec->addWidget( pb_ldcolor,   s_row,   0, 1, 2 );
   spec->addWidget( cb_2dplot,    s_row++, 2, 1, 2 );
   spec->addWidget( pb_refresh,   s_row,   0, 1, 2 );
   spec->addWidget( cb_3dplot,    s_row++, 2, 1, 2 );
   spec->addWidget( pb_mandrsb,   s_row,   0, 1, 2 );
   spec->addWidget( pb_ckovrlp,   s_row++, 2, 1, 2 );
   spec->addWidget( pb_autassb,   s_row,   0, 1, 2 );
   spec->addWidget( pb_resetsb,   s_row++, 2, 1, 2 );
   spec->addWidget( pb_reset,     s_row,   0, 1, 2 );
   spec->addWidget( pb_save,      s_row++, 2, 1, 2 );
   spec->addWidget( pb_view,      s_row,   0, 1, 2 );
   spec->addWidget( pb_help,      s_row,   2, 1, 1 );
   spec->addWidget( pb_close,     s_row++, 3, 1, 1 );
   spec->addWidget( te_status,    s_row++, 0, 1, 4 );

   // set up plot component window on right side
   xa_title_s  = tr( "Sedimentation Coefficient corrected for water at 20" )
      + DEGC;
   xa_title_mw = tr( "Molecular Weight (Dalton)" );
   xa_title    = xa_title_s;

   QBoxLayout* plot = new US_Plot( data_plot, 
      tr( "Distribution Data" ), xa_title, ya_title ); 
   rght->addLayout( plot );
   QBoxLayout* txed = new QHBoxLayout;
   te_pctl_help  = us_textedit( );
   te_pctl_help->setText( tr(
      "Please load a sedimentation coefficient or molecular weight"
      " distribution to initialize the genetic algorithm s-value or vw-value"
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

   data_plot->setCanvasBackground( Qt::darkBlue );

   // put layouts together for overall layout
   left->addLayout( spec );

   main->addLayout( left );
   main->addLayout( rght );
   main->setStretchFactor( left, 3 );
   main->setStretchFactor( spec, 3 );
   main->setStretchFactor( rght, 5 );

   // set up variables and initial state of GUI
   soludata   = new US_SoluteData();
   sdistro    = &s_distro;
   plot_dim   = 3;          // default plot dimension
   plot_s     = true;       // default s/MW X type
   rbtn_click = false;      // default right-button clicked
   mfilter    = "";         // default model list filter
   runsel     = true;       // default prefilter type
   latest     = true;       // default edit prefilter type
   pfilts.clear();          // default prefilter edits list

   reset();
}

// reset the GUI
void US_GA_Initialize::reset( void )
{
   data_plot->detachItems( );
   data_plot->replot();
   pick       = new US_PlotPicker( data_plot );
 
   lw_sbin_data->clear();
   soludata->clearBuckets();
   sxset      = 0;

   minmax     = false;
   zoom       = false;
   cb_1dplot->setChecked(  plot_dim == 1 );  
   cb_2dplot->setChecked(  plot_dim == 2 );
   cb_3dplot->setChecked(  plot_dim == 3 );
   cb_plot_s->setChecked(  plot_s );
   cb_plot_mw->setChecked( !plot_s );

   nisols     = 0;
   nibuks     = 0;
   wsbuck     = 0.0;
   hfbuck     = 0.0;
   ct_nisols->setValue( (double)nisols );
   ct_wsbuck->setRange( 0, 200, 0.1 );
   ct_hfbuck->setRange( 0, 50, 0.01 );
   ct_wsbuck->setValue( wsbuck );
   ct_hfbuck->setValue( hfbuck );

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
   ct_plfmin->setRange( 0.5, 50, 0.01 );
   ct_plfmin->setValue( plfmin );
   ct_plfmin->setEnabled( false );
   ct_plfmax->setRange( 1.0, 50, 0.01 );
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
   colormap   = new QwtLinearColorMap( Qt::white, Qt::black );
   colormap->addColorStop( 0.10, Qt::cyan );
   colormap->addColorStop( 0.50, Qt::magenta );
   colormap->addColorStop( 0.80, Qt::red );
   cmapname   = tr( "Default Color Map: w-cyan-magenta-red-black" );

   monte_carlo = false;
   pb_reset->setEnabled(   false );
   cb_plot_s->setEnabled(  true );
   cb_plot_mw->setEnabled( true );
}

// save the GA data
void US_GA_Initialize::save( void )
{ 
   int novlps = soludata->countOverlaps();

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

   if ( ! plot_s  &&  ! monte_carlo )
   {  // MW & not Monte Carlo:  there can be no output file
      QMessageBox::information( this,
         tr( "No Files Output" ),
         tr( "No files will be saved, since buckets are not s vs. f/f0\n"
             "and input models were not Monte Carlo.\n\n"
             "To output a gadistro file, pick buckets in s vs. f/f0." ) );
      return;
   }

   QString runid = run_name.section( ".", 0, -2 );
   QString trpid = run_name.section( ".", -1, -1 );
   QString fdir  = US_Settings::resultDir() + "/" + runid;
   QString fndat = "gainit." + trpid + ".gadistro.dat";
   QString fnsta = "gainit." + trpid + ".ga.stats";
   QString fname = fdir + "/" + fndat;
   QString fdir2 = US_Settings::reportDir() + "/" + runid;
   QString fnst2 = "gainit." + trpid + ".ga_stats.rpt";
   QString fnam2 = fdir2 + "/" + fnst2;

   QDir dirp( US_Settings::resultDir() );

   if ( ! dirp.exists( fdir ) )
      dirp.mkpath( fdir );

   if ( plot_s )
      soludata->saveGAdata( fname );

   if ( monte_carlo )
   {  // if Monte Carlo, build up and analyze data, then report

      soludata->buildDataMC( plot_s );             // build it

      fname         = fdir + "/" + fnsta;

      soludata->reportDataMC( fname, mc_iters );   // report it

      // Copy the statistics file to the report directory
      QFile( fnam2 ).remove();
      QFile( fname ).copy( fnam2 );
      pb_view   ->setEnabled( true );
   }

   // Report on files saved
   QString msg = tr( "Saved:\n" );
   if ( plot_s )
      msg     += "    " + fndat + "\n";

   if ( monte_carlo )
   {
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

         freport.saveDocumentFromFile( fdir2, fnst2, dbP, idEdit );

         msg     += tr( "\n\nThe report file was also saved to the database" );
      }
   }

   else
   {
      msg     += tr( "in directory:\n    " ) + fdir;
   }

   QMessageBox::information( this, tr( "Distro/Stats File Save" ), msg );
}

// Manually draw solute bins
void US_GA_Initialize::manDrawSb( void )
{
   QColor cblack( Qt::black );
   QColor cwhite( Qt::white );

   // create a new plot picker to draw rectangles around solute points
   delete pick;
   pick      = new US_PlotPicker( data_plot );

   // make sure rubber band and tracker show up against background
   QColor bg = data_plot->canvasBackground();
   int csum  = bg.red() + bg.green() + bg.blue();
   pickpen   = new QPen( ( csum > 600 ) ? cblack : cwhite );

   pick->setRubberBandPen( *pickpen );
   pick->setTrackerPen(    *pickpen );
   pick->setRubberBand(     QwtPicker::RectRubberBand );
   pick->setSelectionFlags( QwtPicker::RectSelection
                          | QwtPicker::DragSelection );

   // set up to capture position and dimensions of solute bin
   connect( pick, SIGNAL(  mouseDown( const QwtDoublePoint& ) ),
            this, SLOT( getMouseDown( const QwtDoublePoint& ) ) );
   connect( pick, SIGNAL( mouseUp(    const QwtDoublePoint& ) ),
            this, SLOT( getMouseUp(   const QwtDoublePoint& ) ) );

   pb_ckovrlp->setEnabled( false );

   wsbuck       = ( plsmax - plsmin ) / 20.0;
   hfbuck       = ( plfmax - plfmin ) / 20.0;
   hfbuck       = qMax( hfbuck, 0.1 );
   double rmax  = wsbuck * 10.0;
   double rinc  = pow( 10.0, (double)( (int)( log10( rmax ) - 3.0 ) ) );
   ct_wsbuck->disconnect( );
   ct_wsbuck->setRange( 0.0, rmax, rinc );
   ct_wsbuck->setValue( wsbuck );
   ct_hfbuck->setValue( hfbuck );
   connect( ct_wsbuck, SIGNAL( valueChanged(  double ) ),
            this,      SLOT(   update_wsbuck( double ) ) );
}

// Check for bin overlaps
void US_GA_Initialize::checkOverlaps( void )
{
   int novlps = soludata->countOverlaps();

   if ( novlps == 0 )
   {
      QMessageBox::information( this,
         tr( "No Bins Overlap" ),
         tr( "No bin overlaps were found, so you\n"
             "may proceed to saving this GA data." ) );
      pb_save->setEnabled( true );
   }

   else
   {
      QString msg = ( novlps == 1 ) ?
         tr( "There is one case of overlap between bins.\n" ) :
         tr( "%1 pairs of bins overlap one another.\n" ).arg( novlps );
      QMessageBox::warning( this,
         tr( "Bin Overlaps" ),
         msg + tr( "You must correct this condition so that no\n"
                   "bins overlap, before you can save GA data." ) );
      pb_save->setEnabled( false );
   }
}

// Auto assign solute bins
void US_GA_Initialize::autoAssignSb( void )
{
   nisols      = ( nisols == 0 ) ? sdistro->size() : nisols;
   pc1         = NULL;
   lw_sbin_data->clear();
   soludata->clearBuckets();
   erase_buckets( true );

   nibuks      = soludata->autoCalcBins( nisols, wsbuck, hfbuck );

   for ( int jj = 0; jj < nibuks; jj++ )
   {  // draw the auto-assigned buckets and add lines to list widget
      QRectF rect = soludata->bucketRect( jj );
      pc1         = drawBucketRect( jj, rect );

      lw_sbin_data->addItem( soludata->bucketLine( jj ) );
   }

   data_plot->replot();
   pb_resetsb->setEnabled( true );
   pb_save   ->setEnabled( true );
   pb_ckovrlp->setEnabled( true );
}

// Reset solute bins
void US_GA_Initialize::resetSb( void )
{
   ct_nisols->setValue( 0.0 );
   lw_sbin_data->clear();     // clear solute bucket data
   soludata->clearBuckets();
   sxset    = 0;

   erase_buckets( true );     // erase bucket rectangles from plot and delete

   nibuks   = 0;
   pb_save   ->setEnabled( false );
   pb_view   ->setEnabled( false );
   pb_ckovrlp->setEnabled( false );

   data_plot->replot();
}

// (re)plot data
void US_GA_Initialize::replot_data()
{
   if ( sdistro->isEmpty()  || sdistro->size() == 0 )
      return;

   resetSb();
   ya_title = cnstvbar ?
              tr( "Frictional Ratio f/f0" ) :
              tr( "Vbar at 20" ) + DEGC;

   if ( plot_dim == 1 )
   {
      plot_1dim();
   }
   else if ( plot_dim == 2 )
   {
      plot_2dim();
   }
   else
   {
      plot_3dim();
   }

   setBucketPens();
}

// plot data 1-D
void US_GA_Initialize::plot_1dim( void )
{
   data_plot->detachItems();

   data_plot->setCanvasBackground( Qt::black );
   pick->setTrackerPen( QColor( Qt::white ) );

   sdistro       = plot_s ? &s_distro : &w_distro;

   int     dsize = sdistro->size();
   QVector< double > xvec( dsize );
   QVector< double > yvec( dsize );
   double* x     = xvec.data();
   double* y     = yvec.data();
   double  sval  = sdistro->at( 0 ).s;
   double  smin  = sval;
   double  smax  = sval;
   double  cval  = sdistro->at( 0 ).c;
   double  cmin  = cval;
   double  cmax  = cval;
   int     nn    = 1;
   x[ 0 ]        = sval;
   y[ 0 ]        = cval;

   for ( int jj = 1; jj < dsize; jj++ )
   {
      double svpr = sval;
      double cvpr = cval;
      sval        = sdistro->at( jj ).s;
      cval        = sdistro->at( jj ).c;

      if ( equivalent( sval, svpr, epsilon ) )
      {  // effectively equal s values: sum c values
         cval       += cvpr;
         x[ nn - 1 ] = ( svpr + sval ) * 0.5;
         y[ nn - 1 ] = cval;
      }

      else
      {  // new s value:  save c value and bump count
         x[ nn   ]   = sval;
         y[ nn++ ]   = cval;
      }

      smin        = ( smin < sval ) ? smin : sval;
      smax        = ( smax > sval ) ? smax : sval;
      cmin        = ( cmin < cval ) ? cmin : cval;
      cmax        = ( cmax > cval ) ? cmax : cval;
   }

   if ( dsize == 1 )
   {
      smin       *= 0.95;
      smax       *= 1.05;
      cmin       *= 0.95;
      cmax       *= 1.05;
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
   data_curv->setData( x, y, nn );
   data_curv->setPen( QPen( Qt::yellow, 3, Qt::SolidLine ) );
   data_curv->setStyle( QwtPlotCurve::Sticks );

   data_plot->setAxisAutoScale( QwtPlot::xBottom );
   data_plot->setAxisAutoScale( QwtPlot::yLeft );
   data_plot->enableAxis( QwtPlot::yRight, false );
   data_plot->setAxisTitle( QwtPlot::xBottom, xa_title );
   data_plot->setAxisTitle( QwtPlot::yLeft,   tr( "Partial Concentration" ) );
   data_plot->setAxisScale( QwtPlot::xBottom, smin, smax );
   data_plot->setAxisScale( QwtPlot::yLeft,   cmin, cmax );

   data_plot->replot();

   pb_reset->setEnabled( true );
   pb_autassb->setEnabled( false );
}

// plot data 2-D
void US_GA_Initialize::plot_2dim( void )
{
   data_plot->detachItems();

   data_plot->setCanvasBackground( Qt::black );
   pick->setTrackerPen( QColor( Qt::white ) );

   sdistro       = plot_s ? &s_distro : &w_distro;

   int     dsize = sdistro->size();
   double* x     = new double[ dsize ];
   double* y     = new double[ dsize ];
   double  smin  = 1.0e30;
   double  smax  = -1.0e30;
   double  fmin  = 1.0e30;
   double  fmax  = -1.0e30;

   for ( int jj = 0; jj < dsize; jj++ )
   {
      double sval = sdistro->at( jj ).s;
      double fval = sdistro->at( jj ).k;
      x[ jj ]     = sval;
      y[ jj ]     = fval;
      smin        = ( smin < sval ) ? smin : sval;
      smax        = ( smax > sval ) ? smax : sval;
      fmin        = ( fmin < fval ) ? fmin : fval;
      fmax        = ( fmax > fval ) ? fmax : fval;
   }

   if ( dsize == 1 )
   {
      smin       *= 0.95;
      smax       *= 1.05;
      fmin       *= 0.95;
      fmax       *= 1.05;
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
   data_plot->enableAxis( QwtPlot::yRight, false );
   data_plot->setAxisTitle( QwtPlot::xBottom, xa_title );
   data_plot->setAxisTitle( QwtPlot::yLeft,   ya_title );
   data_plot->setAxisTitle( QwtPlot::yRight,  tr( "Partial Concentration" ) );
   data_plot->axisTitle( QwtPlot::yRight ).setFont(
         data_plot->axisTitle( QwtPlot::yLeft ).font() );
   data_plot->setAxisScale( QwtPlot::xBottom, smin, smax );
   data_plot->setAxisScale( QwtPlot::yLeft,   fmin, fmax );

   data_plot->replot();

   pb_reset->setEnabled( true );
   pb_autassb->setEnabled( !monte_carlo );
}

// plot data 3-D
void US_GA_Initialize::plot_3dim( void )
{
   data_plot->detachItems();
   QColor bg = colormap->color1();

   data_plot->setCanvasBackground( bg );

   int csum  = bg.red() + bg.green() + bg.blue();
   pick->setTrackerPen( QPen( ( csum > 600 ) ? QColor( Qt::black ) :
                                               QColor( Qt::white ) ) );

   QString tstr = run_name + "\n" + analys_name + "\n (" + method + ")";
   data_plot->setTitle( tstr );

   // set up spectrogram data
   d_spectrogram = new QwtPlotSpectrogram();
   d_spectrogram->setData( US_SpectrogramData() );
   d_spectrogram->setColorMap( *colormap );

   US_SpectrogramData& spec_dat = (US_SpectrogramData&)d_spectrogram->data();

   QwtDoubleRect drect;

   if ( auto_lim )
      drect = QwtDoubleRect( 0.0, 0.0, 0.0, 0.0 );
   else
      drect = QwtDoubleRect( plsmin, plfmin,
            ( plsmax - plsmin ), ( plfmax - plfmin ) );

   sdistro       = plot_s ? &s_distro : &w_distro;
   spec_dat.setRastRanges( xreso, yreso, resolu, zfloor, drect );
   spec_dat.setRaster( sdistro );

   d_spectrogram->attach( data_plot );

   // set color map and axis settings
   QwtScaleWidget *rightAxis = data_plot->axisWidget( QwtPlot::yRight );
   rightAxis->setColorBarEnabled( true );
   rightAxis->setColorMap( spec_dat.range(), d_spectrogram->colorMap() );
   data_plot->setAxisTitle( QwtPlot::xBottom, xa_title );
   data_plot->setAxisTitle( QwtPlot::yLeft,   ya_title );
   data_plot->setAxisTitle( QwtPlot::yRight,  tr( "Partial Concentration" ) );
   data_plot->axisTitle( QwtPlot::yRight ).setFont(
         data_plot->axisTitle( QwtPlot::yLeft ).font() );
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

   data_plot->replot();

   pb_reset->setEnabled( true );
   pb_autassb->setEnabled( !monte_carlo );
}

// update pseudo-3d resolution factor
void US_GA_Initialize::update_resolu( double dval )
{
   resolu = dval;
}

// update raster x resolution
void US_GA_Initialize::update_xreso( double dval )
{
   xreso  = dval;
}

// update raster y resolution
void US_GA_Initialize::update_yreso( double dval )
{
   yreso  = dval;
}

// update Z (frequency) floor percent
void US_GA_Initialize::update_zfloor( double dval )
{
   zfloor = dval;
}

// update number of initial solutes
void US_GA_Initialize::update_nisols( double dval )
{
   nisols    = qRound( dval );
}

// update width in s of buckets
void US_GA_Initialize::update_wsbuck( double dval )
{
   wsbuck    = dval;
}

// update height in f/f0 of buckets
void US_GA_Initialize::update_hfbuck( double dval )
{
   hfbuck    = dval;
}

// update plot limit s min
void US_GA_Initialize::update_plsmin( double dval )
{
   plsmin    = dval;
}

// update plot limit s max
void US_GA_Initialize::update_plsmax( double dval )
{
   plsmax    = dval;

   if ( ! plot_s )
   {  // For MW, use logarithmic steps
      double rinc = qMax( pow( 10.0, qRound( log10( dval ) ) - 2.0 ), 10.0 );
      ct_plsmin->setRange( -10.0, 1.0E+5, rinc );
      ct_plsmax->setRange(   0.0, 1.0E+8, rinc );
   }
}

// update plot limit f/f0 min
void US_GA_Initialize::update_plfmin( double dval )
{
   plfmin    = dval;
}

// update plot limit f/f0 max
void US_GA_Initialize::update_plfmax( double dval )
{
   plfmax    = dval;
}

// select automatic plot limits
void US_GA_Initialize::select_autolim()
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

   else if ( plot_s )
   {
      ct_plsmin->setRange( -10000.0, 10000.0, 0.01 );
      ct_plsmax->setRange( 0.0, 10000.0, 0.01 );
   }

   else
   {
      ct_plsmin->setRange( -10.0, 1.0E+8, 1.0E+3 );
      ct_plsmax->setRange(   0.0, 1.0E+8, 1.0E+3 );
   }

   wsbuck       = ( plsmax - plsmin ) / 20.0;
   hfbuck       = ( plfmax - plfmin ) / 20.0;
   double rmax  = wsbuck * 10.0;
   double rinc  = pow( 10.0, (double)( (int)( log10( rmax ) - 3.0 ) ) );
   ct_wsbuck->disconnect( );
   ct_wsbuck->setRange( 0.0, rmax, rinc );
   ct_wsbuck->setValue( wsbuck );
   ct_hfbuck->setValue( hfbuck );
   connect( ct_wsbuck, SIGNAL( valueChanged(  double ) ),
            this,      SLOT(   update_wsbuck( double ) ) );
}

// select 1-dimensional plot
void US_GA_Initialize::select_plot1d()
{
   plot_dim   = 1;
   cb_2dplot->disconnect();
   cb_3dplot->disconnect();
   cb_2dplot->setChecked(  false );
   cb_3dplot->setChecked(  false );

   cb_1dplot->setEnabled(  false );
   cb_2dplot->setEnabled(  true );
   cb_3dplot->setEnabled(  true );

   connect( cb_2dplot,  SIGNAL( clicked() ),
            this,       SLOT( select_plot2d() ) );
   connect( cb_3dplot,  SIGNAL( clicked() ),
            this,       SLOT( select_plot3d() ) );

   replot_data();

   pb_mandrsb->setEnabled( false );
   pb_autassb->setEnabled( false );
}

// select 2-dimensional plot
void US_GA_Initialize::select_plot2d()
{
   plot_dim   = 2;
   cb_1dplot->disconnect();
   cb_3dplot->disconnect();
   cb_1dplot->setChecked( false );
   cb_3dplot->setChecked( false );

   cb_1dplot->setEnabled( true );
   cb_2dplot->setEnabled( false );
   cb_3dplot->setEnabled( true );

   connect( cb_1dplot,  SIGNAL( clicked() ),
            this,       SLOT( select_plot1d() ) );
   connect( cb_3dplot,  SIGNAL( clicked() ),
            this,       SLOT( select_plot3d() ) );

   replot_data();

   pb_mandrsb->setEnabled( true );
   pb_autassb->setEnabled( !monte_carlo );
}

// select 3-dimensional plot
void US_GA_Initialize::select_plot3d()
{
   plot_dim   = 3;
   cb_1dplot->disconnect();
   cb_2dplot->disconnect();
   cb_3dplot->disconnect();
   cb_1dplot->setChecked( false );
   cb_2dplot->setChecked( false );

   cb_1dplot->setEnabled( true );
   cb_2dplot->setEnabled( true );
   cb_3dplot->setEnabled( false );

   connect( cb_1dplot,  SIGNAL( clicked() ),
            this,       SLOT( select_plot1d() ) );
   connect( cb_2dplot,  SIGNAL( clicked() ),
            this,       SLOT( select_plot2d() ) );
   connect( cb_3dplot,  SIGNAL( clicked() ),
            this,       SLOT( select_plot3d() ) );

   replot_data();

   pb_mandrsb->setEnabled( true );
   pb_autassb->setEnabled( !monte_carlo );
}

// select S (sed. coeff.) for horizontal axis
void US_GA_Initialize::select_plot_s()
{
   plot_s    = cb_plot_s->isChecked();
   cb_plot_mw->setChecked( !plot_s );
   xa_title  = plot_s ? xa_title_s : xa_title_mw;
   data_plot->setAxisTitle( QwtPlot::xBottom, xa_title );

   set_limits();

   replot_data();

   lb_wsbuck->setText( tr( "Width of s Bucket:" ) );
   lb_plsmin->setText( tr( "Plot Limit s Min:" ) );
   lb_plsmax->setText( tr( "Plot Limit s Max:" ) );

   wsbuck       = ( plsmax - plsmin ) / 20.0;
   hfbuck       = ( plfmax - plfmin ) / 20.0;
   double rmax  = wsbuck * 10.0;
   double rinc  = pow( 10.0, (double)( (int)( log10( rmax ) - 3.0 ) ) );
   ct_wsbuck->disconnect();
   ct_wsbuck->setRange( 0.0, rmax, rinc );
   ct_wsbuck->setValue( wsbuck );
   ct_hfbuck->setValue( hfbuck );
   connect( ct_wsbuck, SIGNAL( valueChanged(  double ) ),
            this,      SLOT(   update_wsbuck( double ) ) );
}

// select MW (mol.wt.) for horizontal axis
void US_GA_Initialize::select_plot_mw()
{
   plot_s    = !cb_plot_mw->isChecked();
   cb_plot_s->setChecked( plot_s );
   xa_title  = plot_s ? xa_title_s : xa_title_mw;
   data_plot->setAxisTitle( QwtPlot::xBottom, xa_title );

   set_limits();

   replot_data();

   lb_wsbuck->setText( tr( "Width of mw Bucket:" ) );
   lb_plsmin->setText( tr( "Plot Limit mw Min:" ) );
   lb_plsmax->setText( tr( "Plot Limit mw Max:" ) );

   wsbuck       = ( plsmax - plsmin ) / 20.0;
   hfbuck       = ( plfmax - plfmin ) / 20.0;
   double rmax  = wsbuck * 10.0;
   double rinc  = pow( 10.0, (double)( (int)( log10( rmax ) - 3.0 ) ) );
   ct_wsbuck->disconnect();
   ct_wsbuck->setRange( 0.0, rmax, rinc );
   ct_wsbuck->setValue( wsbuck );
   ct_hfbuck->setValue( hfbuck );
   connect( ct_wsbuck, SIGNAL( valueChanged(  double ) ),
            this,      SLOT(   update_wsbuck( double ) ) );
}

// load the solute distribution from a file or from DB
void US_GA_Initialize::load_distro()
{
   Solute          sol_s;
   Solute          sol_w;
   US_Model        model;
   QString         mdesc;
   bool            loadDB = dkdb_cntrls->db();

   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
   US_ModelLoader dialog( loadDB, mfilter, model, mdesc, pfilts );
   dialog.move( this->pos() + QPoint( 200, 200 ) );

   connect( &dialog, SIGNAL(   changed( bool ) ),
            this, SLOT( update_disk_db( bool ) ) );
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
   run_name     = mdesc.section( ".",  0, -3 );
   QString asys = mdesc.section( ".", -2, -2 );
   analys_name  = asys.section( "_", 0, 1 ) + "_"
                + asys.section( "_", 3, 4 );

   monte_carlo  = model.monteCarlo;
   mc_iters     = monte_carlo ? aiters.toInt() : 1;
   editGUID     = model.editGUID;
DbgLv(1) << "MC" << monte_carlo << " iters" << mc_iters;

   s_distro.clear();
   w_distro.clear();

   QString tstr = run_name+ "\n" + analys_name + "\n (" + method + ")";
   data_plot->setTitle( tstr );
   cnstvbar = model.constant_vbar();

   // read in and set distribution s,c,k,d values
   if ( model.analysis != US_Model::COFS )
   {
      for ( int jj = 0; jj < model.components.size(); jj++ )
      {
         US_Model::calc_coefficients( model.components[ jj ] );

         sol_s.s  = model.components[ jj ].s * 1.0e13;
         sol_s.c  = model.components[ jj ].signal_concentration;
         sol_s.k  = cnstvbar ?
                    model.components[ jj ].f_f0 :
                    model.components[ jj ].vbar20;
         sol_s.d  = model.components[ jj ].D;
         sol_s.w  = model.components[ jj ].mw;
         sol_w.s  = sol_s.w;
         sol_w.c  = sol_s.c;
         sol_w.k  = sol_s.k;
         sol_w.d  = sol_s.d;
         sol_w.w  = sol_s.s;
//DbgLv(2) << "Solute jj s w k c d" << jj << sol_s.s << sol_w.s << sol_s.k
//   << sol_s.c << sol_s.d << " vb" << model.components[jj].vbar20;

         s_distro.append( sol_s );
         w_distro.append( sol_w );
      }

      // sort and reduce distributions
      psdsiz    = s_distro.size();
      sort_distro( s_distro, true );
      sort_distro( w_distro, true );
DbgLv(1) << "Solute psdsiz sdsiz wdsiz" << psdsiz << s_distro.size()
 << w_distro.size();
for ( int jj=0;jj<qMin(s_distro.size(),w_distro.size());jj++ ) {
 DbgLv(2) << " jj" << jj << " s k d w" << s_distro[jj].s << s_distro[jj].k
    << s_distro[jj].d << s_distro[jj].w << " w k d s" << w_distro[jj].s
    << w_distro[jj].k << w_distro[jj].d << w_distro[jj].w; }
   }

DbgLv(1) << "Constant-Vbar" << cnstvbar;
   if ( cnstvbar )
   {
      lb_plfmin->setText( tr( "Plot Limit f/f0 Min:" ) );
      lb_plfmax->setText( tr( "Plot Limit f/f0 Max:" ) );
      lb_hfbuck->setText( tr( "Height of f/f0 Bucket:" ) );
      ct_plfmin->setRange( 0.5, 50, 0.01 );
      ct_plfmax->setRange( 1.0, 50, 0.01 );
      ct_hfbuck->setRange( 0, 50, 0.01 );
   }

   else
   {
      lb_plfmin->setText( tr( "Plot Limit Vbar Min:" ) );
      lb_plfmax->setText( tr( "Plot Limit Vbar Max:" ) );
      lb_hfbuck->setText( tr( "Height of Vbar Bucket:" ) );
      ct_plfmin->setRange( 0.1, 1.5, 0.001 );
      ct_plfmax->setRange( 0.2, 1.5, 0.001 );
      ct_hfbuck->setRange( 0, 1.5, 0.001 );
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

      if ( plot_s )
      {
         ct_plsmin->setRange( -10000.0, 10000.0, 0.01 );
         ct_plsmax->setRange( 0.0, 10000.0, 0.01 );
      }
      else
      {
         ct_plsmin->setRange( -10.0, 1.0E+8, 1.0E+3 );
         ct_plsmax->setRange(   0.0, 1.0E+8, 1.0E+3 );
      }
   }
   data_plot->setAxisScale( QwtPlot::xBottom, plsmin, plsmax );
   data_plot->setAxisScale( QwtPlot::yLeft,   plfmin, plfmax );

   pb_resetsb->setEnabled( true );

   nisols       = s_distro.size();
   dfilname     = "(" + mfnam + ") " + mdesc;
   stdfline     = "  " + dfilname;
   stnpline     = tr( "The number of distribution points is %1" ).arg( nisols );

   if ( nisols != psdsiz )
      stnpline    += tr( "\n  (reduced from %1)" ).arg( psdsiz );

   te_status->setText( stcmline + "\n" + stdiline + "\n"
         + stdfline + "\n" + stnpline );

   replot_data();

   soludata->setDistro( sdistro );

   nibuks       = 0;
   wsbuck       = ( plsmax - plsmin ) / 20.0;
   hfbuck       = ( plfmax - plfmin ) / 20.0;
   ct_wsbuck->setValue( wsbuck );
   ct_hfbuck->setValue( hfbuck );
   ct_nisols->setValue( double( nisols ) );
   ct_wsbuck->setEnabled(  true );
   ct_hfbuck->setEnabled(  true );
   pb_refresh->setEnabled( true );
   pb_mandrsb->setEnabled( plot_dim != 1 );
}

// load the color map from a file
void US_GA_Initialize::load_color()
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

   stcmline  = cmapname;
   te_status->setText( stcmline + "\n" + stdiline + "\n"
         + stdfline + "\n" + stnpline );
}

// set plot x,y limits
void US_GA_Initialize::set_limits()
{
   double fmin = 1.0e30;
   double fmax = -1.0e30;
   double smin = 1.0e30;
   double smax = -1.0e30;
   double rdif;

   resetSb();

   if ( plot_s )
   {
      sdistro     = &s_distro;
      xa_title    = xa_title_s;
   }

   else
   {
      sdistro     = &w_distro;
      xa_title    = xa_title_mw;
   }

   soludata->setDistro( sdistro );

   data_plot->setAxisTitle( QwtPlot::xBottom, xa_title );

   // find min,max for S distributions
   for ( int jj = 0; jj < sdistro->size(); jj++ )
   {
      double sval = sdistro->at( jj ).s;
      double fval = sdistro->at( jj ).k;
      smin        = ( smin < sval ) ? smin : sval;
      smax        = ( smax > sval ) ? smax : sval;
      fmin        = ( fmin < fval ) ? fmin : fval;
      fmax        = ( fmax > fval ) ? fmax : fval;
   }

   // adjust minima, maxima
   rdif      = ( smax - smin ) / 10.0;
   smin     -= rdif;
   smax     += rdif;
   smin      = ( smin < 0.0 ) ? 0.0 : smin;
   rdif      = ( fmax - fmin ) / 10.0;

   if ( sdistro->size() == 1 )
   {
      rdif      = smin * 0.05;
      smin     -= rdif;
      smax     += rdif;
      rdif      = fmin * 0.05;
      fmin     -= rdif;
      fmax     += rdif;
   }

   if ( rdif == 0.0 )
   {
      QMessageBox::warning( this,
         tr( "Constant f/f0 ; Varying Vbar" ),
         tr( "NOTE: This model has constant f/f0 values.\n"
             "It cannot currently be used for GA." ) );

      rdif      = 0.5;
      update_plfmin( fmin - rdif );
      update_plfmax( fmax + rdif );
      update_plsmin( smin );
      update_plsmax( smax );

      auto_lim  = false;
      cb_autlim->setChecked( auto_lim );
      select_autolim();

      return;
   }

   fmin     -= rdif;
   fmax     += rdif;

   if ( sdistro->size() > 0 )
   {
      double rmax  = smax * 10.0;
      double rinc  = pow( 10.0, (double)( (int)( log10( rmax ) - 3.0 ) ) );

      ct_plsmax->setRange( 0.0, rmax, rinc );
      ct_plsmin->setRange( -( smax / 50.0 ), rmax, rinc );
   }

   if ( auto_lim )
   {
      // set auto limits
      smax       += ( ( smax - smin ) / 20.0 );
      smin       -= ( ( smax - smin ) / 20.0 );
      fmax       += ( ( fmax - fmin ) / 20.0 );
      fmin       -= ( ( fmax - fmin ) / 20.0 );
      fmin        = ( fmin < 0.1 ) ? 0.1 : fmin;

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
   }
   te_pctl_help->setText( tr(
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
      " item and responding/defaulting Yes in the resulting dialog." ) );
}

// Sort distribution solute list by s,k values and optionally reduce
void US_GA_Initialize::sort_distro( QList< Solute >& listsols,
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
             kdup    = max( kdup, ++jdup );
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

// highlight solute bin rectangle in red; previous in yellow or blue
void US_GA_Initialize::highlight_solute( QwtPlotCurve* bc1 )
{
   if ( bc1 == NULL )
      return;

   if ( nibuks > 0  &&  pc1 != NULL )
   {  // re-color previous bucket yellow or blue
      pc1->setPen( pbukpen );
   }

   // current bucket borders drawn in red
   bc1->setPen( cbukpen );

   pc1       = bc1;         // save previous bucket curve
   return;
}

// find bucket curve by solute index, then highlight
void US_GA_Initialize::highlight_solute( int sx )
{
   highlight_solute( bucketCurveAt( sx ) );
   return;
}

// mouse down:  save of first point
void US_GA_Initialize::getMouseDown( const QwtDoublePoint& p )
{
   p1     = p;              // save the first rubberband point
}

// mouse up:  draw bucket rectangle
void US_GA_Initialize::getMouseUp( const QwtDoublePoint& p )
{
   double        tx[2];
   double        ty[2];
   QwtPlotCurve* bc1;

   p2     = p;              // save the second rubberband point

   // draw the bucket rectangle
   bc1    = drawBucketRect( nibuks, p1, p2 );

   // highlight it (and turn off highlight for previous)
   highlight_solute( bc1 );

   data_plot->replot();

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
   QPointF tpt( ( tx[0] + tx[1] ) / 2.0, ( ty[0] + ty[1] ) / 2.0 );
   QPointF& bucp = tpt;
   int sx        = soludata->findNearestPoint( bucp );

   if ( sx >= 0 )
   {  // for solute point nearest to rectangle midpoint
      Solute sol  = sdistro->at( sx );
      qreal bucc  = sol.c;  // get concentrate value

      // add the bucket entry and add a text box entry
      soludata->appendBucket( bucr, bucp, bucc, 2 );

      QString txt = soludata->bucketLine( -3 );
      lw_sbin_data->addItem( txt );
   }

   // bump solute bins count
   nibuks++;
   ct_nisols->setValue( (double)nibuks );

   if ( nibuks > 0 )
      pb_save   ->setEnabled( true );
   if ( nibuks > 1 )
      pb_ckovrlp->setEnabled( true );
}

// draw a bucket rectangle by index and top-left,bottom-right points
QwtPlotCurve* US_GA_Initialize::drawBucketRect( int sx,
      QPointF pt1, QPointF pt2 )
{
   double        tx[5];
   double        ty[5];
   QwtPlotCurve* bc1;

   tx[0]  = pt1.x();        // set 5 points needed to draw rectangle
   ty[0]  = pt1.y();
   tx[1]  = pt2.x();
   ty[1]  = pt1.y();
   tx[2]  = pt2.x();
   ty[2]  = pt2.y();
   tx[3]  = pt1.x();
   ty[3]  = pt2.y();
   tx[4]  = pt1.x();
   ty[4]  = pt1.y();

   // create the bucket rectangle curve
   bc1    = us_curve( data_plot, QString( "bucket border %1" ).arg( sx ) );
   bc1->setPen(   pbukpen );
   bc1->setStyle( QwtPlotCurve::Lines );
   bc1->setData(  tx, ty, 5 );

   return bc1;
}

// draw a bucket rectangle by index and rectangle
QwtPlotCurve* US_GA_Initialize::drawBucketRect( int sx, QRectF rect )
{
   return drawBucketRect( sx, rect.topLeft(), rect.bottomRight() );
}

// solute bin list row selected by arrow key
void US_GA_Initialize::newrow_sbdata( int /*row*/ )
{
   sclick_sbdata( lw_sbin_data->currentIndex() );
}

// solute bin list row clicked: highlight bucket
void US_GA_Initialize::sclick_sbdata( const QModelIndex& mx )
{
   int sx      = mx.row();

   highlight_solute( sx );
   data_plot->replot();

   if ( rbtn_click )
   {
      rbtn_click   = false;
      lw_sbin_data->disconnect();
      int binx    = sx + 1;
      QMessageBox msgBox;
      QString msg = tr( "Are you sure you want to delete solute bin %1 ?" )
         .arg( binx );
      msgBox.setWindowTitle( tr( "GA_Init Solute Bin Delete" ) );
      msgBox.setText( msg );
      msgBox.setStandardButtons( QMessageBox::No | QMessageBox::Yes );
      msgBox.setDefaultButton( QMessageBox::Yes );
      if ( msgBox.exec() == QMessageBox::Yes )
      {
         removeSoluteBin( sx );
      }
      connect( lw_sbin_data, SIGNAL( clicked(       const QModelIndex& ) ),
               this,         SLOT(   sclick_sbdata( const QModelIndex& ) ) );
      connect( lw_sbin_data, SIGNAL( doubleClicked( const QModelIndex& ) ),
               this,         SLOT(   dclick_sbdata( const QModelIndex& ) ) );
      connect( lw_sbin_data, SIGNAL( currentRowChanged( int )            ),
               this,         SLOT(   newrow_sbdata(     int )            ) );
   }

   else if ( monte_carlo  &&  sx != sxset )
   {
      QRectF rect  = soludata->bucketRect( sx );
      ct_wsbuck->setValue( rect.width() );
      ct_hfbuck->setValue( rect.height() );
   }
   sxset        = sx;
}

// solute bin list row double-clicked:  change bucket values
void US_GA_Initialize::dclick_sbdata( const QModelIndex& mx )
{
   int sx      = mx.row();
   QwtPlotCurve* bc1;
   QPointF pt0;

   if ( !monte_carlo )
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
   sxset       = sx;
   qreal x1    = pt0.x() - wsbuck / 2.0;
   qreal y1    = pt0.y() + hfbuck / 2.0;
   qreal x2    = pt0.x() + wsbuck / 2.0;
   qreal y2    = pt0.y() - hfbuck / 2.0;
   QPointF pt1( x1, y1 );
   QPointF pt2( x2, y2 );
   QRectF  brect( pt1, pt2 );

   pc1->detach();                  // erase old rectangle for this bucket

   changeBucketRect( sx, brect );  // change bucket rectangle

   bc1         = drawBucketRect( sx, pt1, pt2 );  // draw a new rectangle

   pc1         = bc1;              // save previous bucket curve
   rbtn_click  = false;

   data_plot->replot();

   return;
}

// change the rectangle (vertices) for a bucket
void US_GA_Initialize::changeBucketRect( int sx, QRectF& rect )
{
   bucket abuck  = soludata->bucketAt( sx );

   QPointF bpnt  = soludata->bucketPoint( sx, true );
   qreal bconc   = soludata->bucketAt( sx ).conc;
   int bstat     = 0;

   soludata->setBucket( sx, rect, bpnt, bconc, bstat );

   QString line  = soludata->bucketLine( sx );
   lw_sbin_data->item( sx )->setText( line );

   return;
}

// find the plot curve in the list of curves
QwtPlotCurve* US_GA_Initialize::bucketCurveAt( int sx )
{
   // get title of desired bucket curve and list of all items
   QString ctext         = QString( "bucket border %1" ).arg( sx );
   QwtPlotItemList ilist = data_plot->itemList();

   for ( int jj = 0; jj < ilist.size(); jj++ )
   {  // test each item for Curve type and matching title
      QwtPlotCurve* bc1 = (QwtPlotCurve*)ilist.at( jj );

      if ( bc1->rtti() == QwtPlotItem::Rtti_PlotCurve )
      {  // right type,  so check title
         QString itext = bc1->title().text();

         if ( itext.compare( ctext ) == 0 )
         {  // this is the one we want, return a pointer to the curve
            return bc1;
         }
      }
   }

   return (QwtPlotCurve*)NULL;
}

// erase all bucket curves (option to completely delete )
void US_GA_Initialize::erase_buckets( bool delflag )
{

   for ( int jj = 0; jj < nibuks; jj++ )
   {
      QwtPlotCurve* bc1 = bucketCurveAt( jj );
      if ( bc1 != NULL )
      {
         bc1->detach();

         if ( delflag )
            delete bc1;
      }
   }
   nibuks   = 0;

   data_plot->replot();

   return;
}
// erase all bucket curves (from plot only)
void US_GA_Initialize::erase_buckets( )
{
   erase_buckets( false );
   return;
}

// filter events to catch right-mouse-button-click on list widget
bool US_GA_Initialize::eventFilter( QObject *obj, QEvent *e )
{
   if ( obj == lw_sbin_data  &&
        e->type() == QEvent::ContextMenu )
   {
      rbtn_click = true;
      return false;
   }
   else
   {
      return US_Widgets::eventFilter( obj, e );
   }
}

// remove a solute bin from data, plot, and list
void US_GA_Initialize::removeSoluteBin( int sx )
{
   QList< QString > lines;
   QList< QwtPlotCurve* > curves;
   QwtPlotCurve* bc1;

   int bsize = soludata->bucketsCount();

   // remove the solute bin
   soludata->removeBucketAt( sx );

   // create a new set of lines for the List Widget and new list of curves

   for ( int jj = 0; jj < bsize; jj++ )
   {
      pc1    = bucketCurveAt( jj );
      if ( jj < sx )
      {  // before removed item, just save the old line and curve
         lines.append( lw_sbin_data->item( jj )->text() );
         curves.append( pc1 );
      }

      else if ( jj > sx )
      {  // after removed item, compose new line,title from bucket data
         int kk = jj - 1;
         lines.append( soludata->bucketLine( kk ) );
         pc1->setTitle( QString( "bucket border %1" ).arg( kk ) );
         curves.append( pc1 );
      }

      else
      {  // completely remove the curve itself
         delete pc1;
         pc1    = bucketCurveAt( ( jj > 0 ) ? (jj-1) : 0 );
      }
   }

   // replace the List Widget contents and redraw bin rectangles
   lw_sbin_data->clear();
   erase_buckets();
   nibuks    = bsize - 1;
   ct_nisols->setValue( (double)nibuks );

   for ( int jj = 0; jj < nibuks; jj++ )
   {
      // add the Solute Bin line back in the List Widget
      lw_sbin_data->addItem( lines.at( jj ) );

      // redraw the bin rectangle
      bc1    = curves.at( jj );
      bc1->attach( data_plot );
      highlight_solute( jj );
      pc1    = bc1;
   }

   if ( nibuks > 0 )
   {  // highlight the next bucket if there is one
      highlight_solute( ( sx < nibuks ) ? sx : ( nibuks - 1) );
   }

   data_plot->replot();

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
   pfilts.clear();

   US_SelectEdits sediag( dkdb_cntrls->db(), runsel, latest, pfilts );
   sediag.move( this->pos() + QPoint( 200, 200 ) );
   sediag.exec();

   int nedits = pfilts.size();
   QString pfmsg;

   if ( nedits == 0 )
      pfmsg = tr( "(none chosen)" );

   else if ( runsel )
      pfmsg = tr( "Run ID prefilter - %1 edit(s)" ).arg( nedits );

   else if ( latest )
      pfmsg = tr( "%1 Latest-Edit prefilter(s)" ).arg( nedits );

   else
      pfmsg = tr( "%1 total Edit prefilter(s)" ).arg( nedits );

   le_prefilt->setText( pfmsg );
}

// View the statistics file produced in a file editor
void US_GA_Initialize::view( )
{
qDebug() << "VIEW";
   QString runid = run_name.section( ".",  0, -2 );
   QString trpid = run_name.section( ".", -1, -1 );
   QString fdir  = US_Settings::resultDir() + "/" + runid;
   QString fnsta = "gainit." + trpid + ".ga.stats";
   QString fname = fdir + "/" + fnsta;
qDebug() << "VIEW fname" << fname;

   QFile filei( fname );
   if ( filei.open( QIODevice::ReadOnly | QIODevice::Text ) )
   {
qDebug() << "VIEW OPENED";
      QTextStream ts( &filei );
      QString rtext = ts.readAll();
      filei.close();

      US_Editor* eddiag = new US_Editor( US_Editor::DEFAULT, true );
      eddiag->setWindowTitle( tr( "Statistics File Contents" ) );
      eddiag->move( this->pos() + QPoint( 30, 30 ) );
      eddiag->resize( 600, 700 );
      eddiag->e->setFont( US_Widgets::fixedFont() );
      eddiag->e->setPlainText( rtext );
qDebug() << "VIEW eddiag SHOW";
      eddiag->show();
   }

   else
   {
qDebug() << "VIEW OPEN ERROR" << fname;
      QMessageBox::critical( this, tr( "File Read Error" ),
         tr( "Unable to open file\n\"%1\"\nfor read" ).arg( fname ) );
   }
}

