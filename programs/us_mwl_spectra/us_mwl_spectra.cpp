//! \file us_mwl_spectra.cpp
#include <QApplication>

#include "us_mwl_spectra.h"
#include "us_mwls_pltctl.h"
#include "us_license_t.h"
#include "us_license.h"
#include "us_select_runs.h"
#include "us_util.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_plot.h"
#include "us_math2.h"
#include "us_db2.h"
#include "us_passwd.h"
#include "us_investigator.h"
#include "us_constants.h"
#include "us_report.h"
#include "us_gui_util.h"
#include "us_util.h"
#include "us_sleep.h"
#include "us_editor.h"
#include "us_images.h"
#if QT_VERSION < 0x050000
#define setSamples(a,b,c)  setData(a,b,c)
#endif

#ifdef Q_OS_WIN
#include <float.h>
#define isnan _isnan
#endif

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug()
#endif

int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );

   #include "main1.inc"

   // License is OK.  Start up.
   
   US_MwlSpectra ww;
   ww.show();                  //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication
}

US_MwlSpectra::US_MwlSpectra() : US_Widgets()
{
   const QChar chlamb( 955 );

   setWindowTitle( tr( "Multi-Wavelength S Spectra Viewer" ) );
   setPalette( US_GuiSettings::frameColor() );

   QGridLayout* settings = new QGridLayout;

   nsmooth      = 3;
   dbg_level    = US_Settings::us_debug();
   mfilter      = "";
   p3d_ctld     = NULL;
   p3d_pltw     = NULL;
   runID        = "";
   xaxis        = "Sed. Coeff.";
   xlegend      = "s (x 10^13)";
   QFont sfont( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() - 1 );
   QFontMetrics fmet( sfont );
   int fwid     = fmet.maxWidth();
   int lwid     = fwid * 3;

   // Disk/DB controls
   dkdb_cntrls  = new US_Disk_DB_Controls(
                     US_Settings::default_data_location() );

   // Load controls     
   QLabel*      lb_run      = us_banner    ( tr( "Load the Models" ) );
                pb_prefilt  = us_pushbutton( tr( "Select PreFilter" ) );
                pb_loaddis  = us_pushbutton( tr( "Load Distributions" ) );
                pb_reset    = us_pushbutton( tr( "Reset Data" ) );
                pb_details  = us_pushbutton( tr( "Data Details" ), false );

   int rhgt     = pb_prefilt->height();
   QLabel*      lb_smooth   = us_label( tr( "%1 Gaussian Smooth Points:" )
                                        .arg( chlamb),
                                        -1 );
                ct_smooth   = us_counter( 1, 1, 100, 1 );
   ct_smooth->setSingleStep( 1.0 );
   ct_smooth->setFont( sfont );
   ct_smooth->setMinimumWidth( fwid );
   ct_smooth->resize( rhgt, lwid );
   ct_smooth->setValue( nsmooth );

    QButtonGroup* sel_plt  = new QButtonGroup( this );
    QGridLayout* lo_pltsw  = us_radiobutton( tr( "s20,W" ), rb_pltsw,    true  );
    QGridLayout* lo_pltMW  = us_radiobutton( tr( "MW"    ), rb_pltMW,    false );
    QGridLayout* lo_pltDw  = us_radiobutton( tr( "D20,W" ), rb_pltDw,    false );
    QGridLayout* lo_pltff0 = us_radiobutton( tr( "f/f0"  ), rb_pltff0,   false );
    QGridLayout* lo_pltvb  = us_radiobutton( tr( "vbar"  ), rb_pltvb,    false );
    QGridLayout* lo_pltMWl = us_radiobutton( tr( "MWlog" ), rb_pltMWl,   false );
    sel_plt->addButton( rb_pltsw,  0 );
    sel_plt->addButton( rb_pltMW,  1 );
    sel_plt->addButton( rb_pltDw,  2 );
    sel_plt->addButton( rb_pltff0, 3 );
    sel_plt->addButton( rb_pltvb,  4 );
    sel_plt->addButton( rb_pltMWl, 5 );

   
   // Plot Range controls     
   QLabel*      lb_prcntls  = us_banner( tr( "Plot Controls" ) );
   QLabel*      lb_sstart   = us_label( tr( "X Start:"   ), -1 );
                cb_sstart   = us_comboBox();
   QLabel*      lb_send     = us_label( tr( "X End:"     ), -1 );
                cb_send     = us_comboBox();
   QLabel*      lb_lstart   = us_label( tr( "%1 Start:"   ).arg( chlamb ), -1 );
                cb_lstart   = us_comboBox();
   QLabel*      lb_lend     = us_label( tr( "%1 End:"     ).arg( chlamb ), -1 );
                cb_lend     = us_comboBox();
                lb_pltrec   = us_label( tr( "%1:" ).arg(xlegend) );
                cb_pltrec   = us_comboBox();
   cb_pltrec ->addItem( "1.00" );
   cb_pltrec ->addItem( "1.10" );

                pb_prev     = us_pushbutton( tr( "Previous" ) );
                pb_next     = us_pushbutton( tr( "Next" ) );
   pb_prev->setIcon( US_Images::getIcon( US_Images::ARROW_LEFT  ) );
   pb_next->setIcon( US_Images::getIcon( US_Images::ARROW_RIGHT ) );

   // Advanced Plotting controls
   QLabel*      lb_advplot  = us_banner( tr( "Advanced Plotting Control" ) );
   QLayout*     lo_srngsum  = us_checkbox  ( tr( "X Range Sum" ).arg(xaxis),
                                             ck_srngsum, false );
                pb_svdata   = us_pushbutton( tr( "Save Data"       ) );
                pb_plot2d   = us_pushbutton( tr( "Refresh 2D Plot" ) );
                pb_movie2d  = us_pushbutton( tr( "Show 2D Movie"   ) );
                pb_plot3d   = us_pushbutton( tr( "Plot 3D"         ) );
                pb_svplot   = us_pushbutton( tr( "Save Plot(s)"    ) );
                pb_svmovie  = us_pushbutton( tr( "Save Movie"      ) );
   QLabel*      lb_delay    = us_label( tr( "Delay" ) );
                ct_delay    = us_counter( 1, 0.1, 10.0, 0.1 );
   ct_delay ->setSingleStep( 0.1 );
   ct_delay ->setFont( sfont );
   ct_delay ->setMinimumWidth( fwid );
   ct_delay ->resize( rhgt, lwid );
   ct_delay ->setValue( 0.5 );

   // Status and standard pushbuttons
   QLabel*      lb_status   = us_banner( tr( "Status" ) );
                le_status   = us_lineedit( tr( "(no data loaded)" ), -1, true );
   QPalette stpal;
   stpal.setColor( QPalette::Text, Qt::white );
   stpal.setColor( QPalette::Base, Qt::blue  );
   le_status->setPalette( stpal );

   QPushButton* pb_help     = us_pushbutton( tr( "Help" ) );
   QPushButton* pb_close    = us_pushbutton( tr( "Close" ) );

   // Signals and Slots
   connect( pb_prefilt,   SIGNAL( clicked()        ),
            this,         SLOT  ( select_prefilt() ) );
   connect( pb_loaddis,   SIGNAL( clicked()        ),
            this,         SLOT  ( load_distro()    ) );
   connect( pb_reset,     SIGNAL( clicked()  ),
            this,         SLOT  ( resetAll() ) );
   connect( pb_details,   SIGNAL( clicked()    ),
            this,         SLOT  ( runDetails() ) );
   connect( cb_sstart,    SIGNAL( currentIndexChanged( int ) ),
            this,         SLOT  ( changeSedcoeff( )            ) );
   connect( cb_send,      SIGNAL( currentIndexChanged( int ) ),
            this,         SLOT  ( changeSedcoeff( )            ) );
   connect( cb_lstart,    SIGNAL( currentIndexChanged( int ) ),
            this,         SLOT  ( changeLambda( )            ) );
   connect( cb_lend,      SIGNAL( currentIndexChanged( int ) ),
            this,         SLOT  ( changeLambda( )            ) );
   connect( ct_smooth,    SIGNAL( valueChanged( double     ) ),
            this,         SLOT  ( changeSmooth()             ) );
   connect( cb_pltrec,    SIGNAL( currentIndexChanged( int ) ),
            this,         SLOT  ( changeRecord( )            ) );
   connect( pb_prev,      SIGNAL( clicked()  ),
            this,         SLOT  ( prevPlot() ) );
   connect( pb_next,      SIGNAL( clicked()  ),
            this,         SLOT  ( nextPlot() ) );
   connect( ck_srngsum,   SIGNAL( clicked()       ),
            this,         SLOT  ( sum_check()     ) );
   connect( pb_svdata,    SIGNAL( clicked()       ),
            this,         SLOT  ( save_data()     ) );
   connect( pb_plot2d,    SIGNAL( clicked()       ),
            this,         SLOT  ( changeRecord()  ) );
   connect( pb_movie2d,   SIGNAL( clicked()       ),
            this,         SLOT  ( show_2d_movie() ) );
   connect( pb_plot3d,    SIGNAL( clicked()       ),
            this,         SLOT  ( plot_3d()       ) );
   connect( pb_svplot,    SIGNAL( clicked()       ),
            this,         SLOT  ( save_plot()     ) );
   connect( pb_svmovie,   SIGNAL( clicked()       ),
            this,         SLOT  ( save_movie()    ) );
   connect( pb_help,      SIGNAL( clicked()  ),
            this,         SLOT  ( help()     ) );
   connect( pb_close,     SIGNAL( clicked()  ),
            this,         SLOT  ( close()    ) );
    connect( rb_pltsw,    SIGNAL( toggled     ( bool ) ),
             this,        SLOT(   changedPlotX( bool ) ) );
    connect( rb_pltMW,    SIGNAL( toggled     ( bool ) ),
             this,        SLOT(   changedPlotX( bool ) ) );
    connect( rb_pltDw,    SIGNAL( toggled     ( bool ) ),
             this,        SLOT(   changedPlotX( bool ) ) );
    connect( rb_pltff0,   SIGNAL( toggled     ( bool ) ),
             this,        SLOT(   changedPlotX( bool ) ) );
    connect( rb_pltvb,    SIGNAL( toggled     ( bool ) ),
             this,        SLOT(   changedPlotX( bool ) ) );
    connect( rb_pltMWl,   SIGNAL( toggled     ( bool ) ),
             this,        SLOT(   changedPlotX( bool ) ) );

   // Do the left-side layout
   int row = 0;
   settings->addLayout( dkdb_cntrls,   row++, 0, 1, 8 );
   settings->addWidget( lb_run,        row++, 0, 1, 8 );
   settings->addWidget( pb_prefilt,    row,   0, 1, 4 );
   settings->addWidget( pb_loaddis,    row++, 4, 1, 4 );
   settings->addWidget( pb_reset,      row,   0, 1, 4 );
   settings->addWidget( pb_details,    row++, 4, 1, 4 );
   settings->addWidget( lb_prcntls,    row++, 0, 1, 8 );
   settings->addLayout( lo_pltsw,     row,   0, 1, 2 );
   settings->addLayout( lo_pltMW,     row,   2, 1, 2 );
   settings->addLayout( lo_pltDw,     row,   4, 1, 2 );
   settings->addLayout( lo_pltff0,    row++, 6, 1, 2 );
   settings->addLayout( lo_pltvb,     row,   0, 1, 2 );
   settings->addLayout( lo_pltMWl,    row++, 2, 1, 2 );
   settings->addWidget( lb_sstart,     row,   0, 1, 2 );
   settings->addWidget( cb_sstart,     row,   2, 1, 2 );
   settings->addWidget( lb_send,       row,   4, 1, 2 );
   settings->addWidget( cb_send,       row++, 6, 1, 2 );
   settings->addWidget( lb_lstart,     row,   0, 1, 2 );
   settings->addWidget( cb_lstart,     row,   2, 1, 2 );
   settings->addWidget( lb_lend,       row,   4, 1, 2 );
   settings->addWidget( cb_lend,       row++, 6, 1, 2 );
   settings->addWidget( lb_smooth,     row,   0, 1, 4 );
   settings->addWidget( ct_smooth,     row++, 4, 1, 2 );
   settings->addWidget( lb_pltrec,     row,   0, 1, 2 );
   settings->addWidget( cb_pltrec,     row,   2, 1, 2 );
   settings->addWidget( pb_prev,       row,   4, 1, 2 );
   settings->addWidget( pb_next,       row++, 6, 1, 2 );
   settings->addWidget( lb_advplot,    row++, 0, 1, 8 );
   settings->addLayout( lo_srngsum,    row,   0, 1, 4 );
   settings->addWidget( pb_svdata,     row++, 4, 1, 4 );
   settings->addWidget( pb_plot2d,     row,   0, 1, 4 );
   settings->addWidget( pb_movie2d,    row++, 4, 1, 4 );
   settings->addWidget( pb_plot3d,     row,   0, 1, 4 );
   settings->addWidget( lb_delay,      row,   4, 1, 2 );
   settings->addWidget( ct_delay,      row++, 6, 1, 2 );
   settings->addWidget( pb_svplot,     row,   0, 1, 4 );
   settings->addWidget( pb_svmovie,    row++, 4, 1, 4 );
   settings->addWidget( lb_status,     row++, 0, 1, 8 );
   settings->addWidget( le_status,     row++, 0, 1, 8 );
   settings->addWidget( pb_help,       row,   0, 1, 4 );
   settings->addWidget( pb_close,      row++, 4, 1, 4 );

   // Plot layout for the right side of window
   QBoxLayout* plot = new US_Plot( data_plot,
                                   tr( "S Record Spectrum Data"
                                       "\nS (x 10^13) :  1.12" ),
                                   tr( "Wavelength (nm)" ), 
                                   tr( "Concentration (OD)" ) );

   data_plot->setMinimumSize( 600, 400 );

   data_plot->enableAxis( QwtPlot::xBottom, true );
   data_plot->enableAxis( QwtPlot::yLeft  , true );

   data_plot->setAxisScale( QwtPlot::xBottom, 230, 450 );
   data_plot->setAxisScale( QwtPlot::yLeft  , 0.0, 1.5 );

   picker = new US_PlotPicker( data_plot );
   picker->setRubberBand     ( QwtPicker::VLineRubberBand );
   picker->setMousePattern   ( QwtEventPattern::MouseSelect1,
                               Qt::LeftButton, Qt::ControlModifier );

   // Now let's assemble the page

   QVBoxLayout* left     = new QVBoxLayout;
   QVBoxLayout* right    = new QVBoxLayout;
   QHBoxLayout* main     = new QHBoxLayout( this );

   left ->addLayout( settings );
   right->addLayout( plot );

   main->setSpacing         ( 2 );
   main->setContentsMargins ( 2, 2, 2, 2 );
   main->addLayout( left );
   main->addLayout( right );
   main->setStretch( 0, 2 );
   main->setStretch( 1, 4 );

   reset();
   adjustSize();
}

// Completely reset GUI element states and all data
void US_MwlSpectra::reset( void )
{
   pb_prefilt->setEnabled( true );
   pb_loaddis->setEnabled( true );
   pb_details->setEnabled( false );
   rb_pltsw  ->setEnabled( false );
   rb_pltMW  ->setEnabled( false );
   rb_pltDw  ->setEnabled( false );
   rb_pltff0 ->setEnabled( false );
   rb_pltvb  ->setEnabled( false );
   rb_pltMWl ->setEnabled( false );
   cb_sstart ->setEnabled( false );
   cb_send   ->setEnabled( false );
   cb_lstart ->setEnabled( false );
   cb_lend   ->setEnabled( false );
   ct_smooth ->setEnabled( false );
   cb_pltrec ->setEnabled( false );
   pb_prev   ->setEnabled( false );
   pb_next   ->setEnabled( false );
   pb_reset  ->setEnabled( false );
   ck_srngsum->setEnabled( false );
   pb_svdata ->setEnabled( false );
   pb_plot2d ->setEnabled( false );
   pb_movie2d->setEnabled( false );
   pb_plot3d ->setEnabled( false );
   pb_svplot ->setEnabled( false );
   pb_svmovie->setEnabled( false );

   // Clear any data structures
   lambdas     .clear();
   sedcoes     .clear();
   loadedmodels.clear();
   dataPlotClear( data_plot );
   picker   ->disconnect();
   data_plot->setAxisScale( QwtPlot::xBottom, 230, 450 );
   data_plot->setAxisScale( QwtPlot::yLeft  , 0.0, 1.5 );
   grid          = us_grid( data_plot );
   data_plot->replot();

   last_xmin       = -1.0;
   last_xmax       = -1.0;
   last_ymin       = -1.0;
   last_ymax       = -1.0;

   le_status->setText( tr( "(no distributions loaded)" ) );
}

// Slot to reset
void US_MwlSpectra::resetAll( void )
{
   if ( mdlxyz.count() > 0 )
   {
      int status = QMessageBox::information( this,
               tr( "New Data Warning" ),
               tr( "This will erase all data currently on the screen, and " 
                   "reset the program to its starting condition. No hard-drive "
                   "data or database information will be affected. Proceed? " ),
               tr( "&OK" ), tr( "&Cancel" ),
               0, 0, 1 );
      if ( status != 0 ) return;
   }

   reset();

   runID           = "";
   m_tpart         = "";
   m_apart         = "";
   data_plot->setTitle( tr( "S Spectrum Data"
                            "\ns (10^13) :  1.00" ) );
}


// Enable the common dialog controls based on the presence of data
void US_MwlSpectra::enableControls( void )
{
   if ( mdlxyz.count() == 0 )
   {  // If no data yet, just reset
      reset();
      return;
   }

   // Enable and disable controls now
   pb_prefilt->setEnabled( false );
   pb_loaddis->setEnabled( false );
   rb_pltsw  ->setEnabled( true );
   rb_pltMW  ->setEnabled( true );
   rb_pltDw  ->setEnabled( true );
   rb_pltff0 ->setEnabled( true );
   rb_pltvb  ->setEnabled( true );
   rb_pltMWl ->setEnabled( true );
   pb_reset  ->setEnabled( true );
   pb_details->setEnabled( true );
   cb_sstart ->setEnabled( true );
   cb_send   ->setEnabled( true );
   cb_lstart ->setEnabled( true );
   cb_lend   ->setEnabled( true );
   ct_smooth ->setEnabled( true );
   cb_pltrec ->setEnabled( true );
   pb_prev   ->setEnabled( true );
   pb_next   ->setEnabled( true );
   ck_srngsum->setEnabled( true );
   pb_svdata ->setEnabled( true );
   pb_plot2d ->setEnabled( true );
   pb_movie2d->setEnabled( true );
   pb_plot3d ->setEnabled( true );
   pb_svplot ->setEnabled( true );
   pb_svmovie->setEnabled( true );

   nlambda     = lambdas .count();
   nsedcos     = sedcoes .count();
   ntpoint     = nlambda * nsedcos;
   QStringList slscos;
   QStringList sllmbs;

   for ( int jj = 0; jj < nsedcos; jj++ )
      slscos << QString::number( sedcoes[ jj ] );

   for ( int jj = 0; jj < nlambda; jj++ )
      sllmbs << QString::number( lambdas[ jj ] );

   connect_ranges( false );
   cb_sstart ->clear();
   cb_send   ->clear();
   cb_lstart ->clear();
   cb_lend   ->clear();
   cb_pltrec ->clear();

   cb_sstart ->addItems( slscos );
   cb_send   ->addItems( slscos );
   cb_lstart ->addItems( sllmbs );
   cb_lend   ->addItems( sllmbs );
   cb_pltrec ->addItems( slscos );

   cb_sstart ->setCurrentIndex( 0 );
   cb_send   ->setCurrentIndex( nsedcos - 1 );
   cb_lstart ->setCurrentIndex( 0 );
   cb_lend   ->setCurrentIndex( nlambda - 1 );
   connect_ranges( true );

   have_rngs    = false;
   compute_ranges( );

   // Force a plot initialize
   cb_pltrec ->setCurrentIndex( nlambda / 2 );
   qApp->processEvents();
}

// Select raw/edit run(s) as model prefilter
void US_MwlSpectra::select_prefilt( )
{
   pfilts.clear();

   US_SelectRuns srdiag( dkdb_cntrls->db(), pfilts );
   srdiag.move( this->pos() + QPoint( 200, 200 ) );
   connect( &srdiag, SIGNAL( dkdb_changed  ( bool ) ),
            this,    SLOT  ( update_disk_db( bool ) ) );

   if ( srdiag.exec() != QDialog::Accepted )
      pfilts.clear();
}

// Load model distribution(s)
void US_MwlSpectra::load_distro( )
{
   // Get models and model descriptions
   QList< US_Model > models;
   bool loadDB = dkdb_cntrls->db();

   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
   US_ModelLoader lddiag( loadDB, mfilter, models, mdescs, pfilts );
   lddiag.move( this->pos() + QPoint( 200, 200 ) );

   connect( &lddiag, SIGNAL( changed       ( bool ) ),
            this,    SLOT  ( update_disk_db( bool ) ) );
   QApplication::restoreOverrideCursor();

   if ( lddiag.exec() != QDialog::Accepted )
      return;   // No selection made

   // Load all models and build preliminary vectors

   mdlxyz .clear();
   xyzdat .clear();
   lambdas.clear();
   sedcoes.clear();
   wl_min        = 1e+39;
   wl_max        = 1e-39;
   x_min        = 1e+39;
   x_max        = 1e-39;
   co_min        = 1e+39;
   co_max        = 1e-39;
   nipoint       = 0;
   nnpoint       = 0;
   ntpoint       = 0;

   resetAll();

   for ( int jj = 0; jj < models.count(); jj++ )
   {  // Load each selected distribution model
      loadedmodels << models[ jj ];
      load_distro( models[ jj ], mdescs[ jj ]);
   }

   // Sort points and build normalized concentrations

   qSort( sedcoes );
   qSort( lambdas );
   nsedcos       = sedcoes.count();
   nlambda       = lambdas.count();
   nipoint       = mdlxyz .count();
DbgLv(1) << "LD: nlambda" << nlambda << "nsedcos" << nsedcos
  << "nipoint" << nipoint;
   nc_max        = 0;
   cn_max        = 0.0;
   double scalen = 1.0 / (double)models.count();  // Normalizing scale factor

   for ( int jj = 0; jj < nsedcos; jj++ )
   {
      double sedco  = sedcoes[ jj ];
DbgLv(1) << "LD:  jj" << jj << "sedco" << sedco;

      for ( int ii = 0; ii < nlambda; ii++ )
      {
         double waveln = (double)lambdas[ ii ];
DbgLv(1) << "LD:    ii" << ii << "waveln" << waveln;
         double conc   = 0.0;
         double csum   = 0.0;
         int nconcs    = 0;

         for ( int kk = 0; kk < nipoint; kk++ )
         {
if(kk<2||(kk+3)>nipoint)
DbgLv(1) << "LD:      kk" << kk << "X,Y" << mdlxyz[kk].x() << mdlxyz[kk].y();
            if ( dvirt_equal( mdlxyz[ kk ].y(), sedco )  &&
                 dvirt_equal( mdlxyz[ kk ].x(), waveln ) )
            {
               nconcs++;
               csum          += mdlxyz[ kk ].z();
            }
         }

         if ( nconcs > 0 )
         {  // Normalize concentration by dividing sum by number of models
            conc          = csum * scalen;
            nc_max        = qMax( nc_max, nconcs );
            cn_max        = qMax( cn_max, conc );
            nnpoint++;
if(nconcs>5||conc>1000.0)
DbgLv(1) << "LD: **nc_max cn_max" << nc_max << cn_max << "nconcs conc"
 << nconcs << conc << "jj,ii" << jj << ii << "s wl" << sedco << waveln;
         }

         else
         {
            conc           = 0.0;
         }

         xyzdat << QVector3D( waveln, sedco, conc );
DbgLv(1) << "LD:     nconcs" << nconcs;
      }
   }

   ntpoint       = xyzdat  .count();
DbgLv(1) << "LD: nipoint" << nipoint << "nnpoint" << nnpoint << "ntpoint"
 << ntpoint << "nc_max" << nc_max << "cn_max" << cn_max;
   int kdx       = 0;

   // Create the 2-D concentration vectors for each sed.coeff.
   for ( int jj = 0; jj < nsedcos; jj++ )
   {
      QVector< double > cvect;

      for ( int ii = 0; ii < nlambda; ii++ )
      {
         cvect << xyzdat[ kdx++ ].z();
      }

      // Save the concentration vector for lambdas of this sedcoeff
      concdat << cvect;
   }
DbgLv(1) << "LD: concdat size" << concdat.size() << nsedcos;

   // Ok to enable some buttons now
   enableControls();
}

// Load distributions from a single model
void US_MwlSpectra::load_distro( const US_Model model, const QString mdescr )
{
   QString mdesc = mdescr.section( mdescr.left( 1 ), 1, 1 );
   int lambda    = qRound( model.wavelength );

   if ( lambda < 1 )
   {  // If model has no wavelength, get it from the description
      mdesc         = mdesc.endsWith( "model" ) ? mdesc : model.description;
      lambda        = mdesc.section( ".", -3, -3 ).mid( 2 ).toInt();
   }

   if ( runID.isEmpty() )
   {
      runID         = mdesc.section( ".",  0, -4 );
      m_tpart       = mdesc.section( ".", -3, -3 );
      m_apart       = mdesc.section( ".", -2, -2 );
   }

   double waveln = (double)lambda;
   wl_min        = qMin( wl_min, waveln );
   wl_max        = qMax( wl_max, waveln );
   if ( ! lambdas.contains( lambda ) )
      lambdas << lambda;

   for ( int jj = 0; jj < model.components.size(); jj++ )
   {
      double xc     = model.components[ jj ].s * 1e+13;
      double conc   = model.components[ jj ].signal_concentration;
      x_min         = qMin( x_min, xc );
      x_max         = qMax( x_max, xc );
      co_min        = qMin( co_min, conc );
      co_max        = qMax( co_max, conc );

      mdlxyz  << QVector3D( waveln, xc, conc );
if(conc>1000.0)
DbgLv(1) << "LD: **co_max" << co_max << "conc" << conc << "jj" << jj
 << "lambda xc" << lambda << xc << "mdesc" << mdesc;

      if ( ! sedcoes.contains( xc ) )
         sedcoes << xc;
   }
}

// Display detailed information about the data
void US_MwlSpectra::runDetails( void )
{
   // Set base values
   int nmodels     = mdescs.count();
   int lmx         = nmodels - 1;
   QString fmd     = mdescs[   0 ].section( mdescs[   0 ].left( 1 ), 1, 1 );
   QString lmd     = mdescs[ lmx ].section( mdescs[ lmx ].left( 1 ), 1, 1 );

   // Accumulate statistics for whole, selected sedcoeffs, selected lambdas
   double wln1     = (double)lambdas[ 0 ];
   double wln2     = (double)lambdas[ nlambda / 4 ];
   double wln3     = (double)lambdas[ nlambda / 2 ];
   double wln4     = (double)lambdas[ ( nlambda * 3 ) / 4 ];
   double wln5     = (double)lambdas[ nlambda - 1 ];
   double sed1     = sedcoes[ 0 ];
   double sed2     = sedcoes[ nsedcos / 4 ];
   double sed3     = sedcoes[ nsedcos / 2 ];
   double sed4     = sedcoes[ ( nsedcos * 3 ) / 4 ];
   double sed5     = sedcoes[ nsedcos - 1 ];
   QVector< int    > istaa;
   QVector< double > dstaa;
   QVector< double > lmbsa;
   QVector< double > sedsa;
   QVector< double > consa;
   QVector< int    > istaw1;
   QVector< double > dstaw1;
   QVector< double > lmbsw1;
   QVector< double > sedsw1;
   QVector< double > consw1;
   QVector< int    > istaw2;
   QVector< double > dstaw2;
   QVector< double > lmbsw2;
   QVector< double > sedsw2;
   QVector< double > consw2;
   QVector< int    > istaw3;
   QVector< double > dstaw3;
   QVector< double > lmbsw3;
   QVector< double > sedsw3;
   QVector< double > consw3;
   QVector< int    > istaw4;
   QVector< double > dstaw4;
   QVector< double > lmbsw4;
   QVector< double > sedsw4;
   QVector< double > consw4;
   QVector< int    > istaw5;
   QVector< double > dstaw5;
   QVector< double > lmbsw5;
   QVector< double > sedsw5;
   QVector< double > consw5;
   QVector< int    > istas1;
   QVector< double > dstas1;
   QVector< double > lmbss1;
   QVector< double > sedss1;
   QVector< double > conss1;
   QVector< int    > istas2;
   QVector< double > dstas2;
   QVector< double > lmbss2;
   QVector< double > sedss2;
   QVector< double > conss2;
   QVector< int    > istas3;
   QVector< double > dstas3;
   QVector< double > lmbss3;
   QVector< double > sedss3;
   QVector< double > conss3;
   QVector< int    > istas4;
   QVector< double > dstas4;
   QVector< double > lmbss4;
   QVector< double > sedss4;
   QVector< double > conss4;
   QVector< int    > istas5;
   QVector< double > dstas5;
   QVector< double > lmbss5;
   QVector< double > sedss5;
   QVector< double > conss5;

   istaa .fill( 0, 3 );
   dstaa .fill( 0.0, 10 );
   istaw1.fill( 0, 3 );
   dstaw1.fill( 0.0, 10 );
   istaw2.fill( 0, 3 );
   dstaw2.fill( 0.0, 10 );
   istaw3.fill( 0, 3 );
   dstaw3.fill( 0.0, 10 );
   istaw4.fill( 0, 3 );
   dstaw4.fill( 0.0, 10 );
   istaw5.fill( 0, 3 );
   dstaw5.fill( 0.0, 10 );
   istas1.fill( 0, 3 );
   dstas1.fill( 0.0, 10 );
   istas2.fill( 0, 3 );
   dstas2.fill( 0.0, 10 );
   istas3.fill( 0, 3 );
   dstas3.fill( 0.0, 10 );
   istas4.fill( 0, 3 );
   dstas4.fill( 0.0, 10 );
   istas5.fill( 0, 3 );
   dstas5.fill( 0.0, 10 );

   for ( int ii = 0; ii < mdlxyz.count(); ii++ )
   {
      double wlnv     = mdlxyz[ ii ].x();
      double sedv     = mdlxyz[ ii ].y();
      double conv     = mdlxyz[ ii ].z();

      if ( conv == 0.0 )  continue;

      bld_stats( wlnv, sedv, conv, istaa, dstaa, lmbsa, sedsa, consa );

      if ( dvirt_equal( wlnv, wln1 ) )
      {
         bld_stats( wlnv, sedv, conv, istaw1, dstaw1, lmbsw1, sedsw1, consw1 );
      }

      else if ( dvirt_equal( wlnv, wln2 ) )
      {
         bld_stats( wlnv, sedv, conv, istaw2, dstaw2, lmbsw2, sedsw2, consw2 );
      }

      else if ( dvirt_equal( wlnv, wln3 ) )
      {
         bld_stats( wlnv, sedv, conv, istaw3, dstaw3, lmbsw3, sedsw3, consw3 );
      }

      else if ( dvirt_equal( wlnv, wln4 ) )
      {
         bld_stats( wlnv, sedv, conv, istaw4, dstaw4, lmbsw4, sedsw4, consw4 );
      }

      else if ( dvirt_equal( wlnv, wln5 ) )
      {
         bld_stats( wlnv, sedv, conv, istaw5, dstaw5, lmbsw5, sedsw5, consw5 );
      }

      if ( dvirt_equal( sedv, sed1 ) )
      {
         bld_stats( wlnv, sedv, conv, istas1, dstas1, lmbss1, sedss1, conss1 );
      }

      else if ( dvirt_equal( sedv, sed2 ) )
      {
         bld_stats( wlnv, sedv, conv, istas2, dstas2, lmbss2, sedss2, conss2 );
      }

      else if ( dvirt_equal( sedv, sed3 ) )
      {
         bld_stats( wlnv, sedv, conv, istas3, dstas3, lmbss3, sedss3, conss3 );
      }

      else if ( dvirt_equal( sedv, sed4 ) )
      {
         bld_stats( wlnv, sedv, conv, istas4, dstas4, lmbss4, sedss4, conss4 );
      }

      else if ( dvirt_equal( sedv, sed5 ) )
      {
         bld_stats( wlnv, sedv, conv, istas5, dstas5, lmbss5, sedss5, conss5 );
      }
   }

   // Complete statistical values
   final_stats( istaa,  dstaa,  lmbsa,  sedsa,  consa );
   final_stats( istaw1, dstaw1, lmbsw1, sedsw1, consw1 );
   final_stats( istaw2, dstaw2, lmbsw2, sedsw2, consw2 );
   final_stats( istaw3, dstaw3, lmbsw3, sedsw3, consw3 );
   final_stats( istaw4, dstaw4, lmbsw4, sedsw4, consw4 );
   final_stats( istaw5, dstaw5, lmbsw5, sedsw5, consw5 );
   final_stats( istas1, dstas1, lmbss1, sedss1, conss1 );
   final_stats( istas2, dstas2, lmbss2, sedss2, conss2 );
   final_stats( istas3, dstas3, lmbss3, sedss3, conss3 );
   final_stats( istas4, dstas4, lmbss4, sedss4, conss4 );
   final_stats( istas5, dstas5, lmbss5, sedss5, conss5 );

   // Construct the report text
   QString msg     = tr( "Multi-Wavelength Statistics for RunID %1 --\n\n" )
      .arg( runID );
   msg += tr( "General Models Values and Counts.\n" );
   msg += tr( "   First Model Description:      %1\n" ).arg( fmd );
   msg += tr( "   Last Model Description:       %1\n" ).arg( lmd );
   msg += tr( "   Models Loaded:                %1\n" ).arg( nmodels ); 
   msg += tr( "   %1 Coefficients:              %2\n" ).arg( xaxis,nsedcos );
   msg += tr( "   Wavelengths:                  %1\n" ).arg( nlambda );
   msg += tr( "   Total Loaded Points:          %1\n" ).arg( nipoint );
   msg += tr( "\nNormalized Composite Input Grid.\n" );
   msg += tr( "   Points after Normalizing:     %1\n" ).arg( nnpoint );
   msg += tr( "   Maximum Single-Bin Points:    %1\n" ).arg( nc_max );
   msg += tr( "   Maximum Concentration (OD):   %1\n" ).arg( cn_max );
   msg += tr( "Padded Full S x Lambda Grid.\n" );
   msg += tr( "   Grid Points:                  %1\n" ).arg( ntpoint );
   msg += tr( "   Minimum %1 Value:             %2\n" ).arg( xaxis,x_min );
   msg += tr( "   Maximum %1 Value:             %2\n" ).arg( xaxis,x_max );
   msg += tr( "   Minimum Wavelength (nm):      %1\n" ).arg( wl_min );
   msg += tr( "   Maximum Wavelength (nm):      %1\n" ).arg( wl_max );
   msg += tr( "   Minimum Concentration (OD):   %1\n" ).arg( co_min );
   msg += tr( "   Maximum Concentration (OD):   %1\n" ).arg( co_max );
   msg += tr( "Current Plotting Controls.\n" );
   msg += tr( "   Start %1 value:               %2\n" ).arg( xaxis,sed_start );
   msg += tr( "   End %1 value:                 %2\n" ).arg( xaxis,sed_end );
   msg += tr( "   Start Wavelength:             %1\n" ).arg( lmb_start );
   msg += tr( "   End Wavelength:               %1\n" ).arg( lmb_end );

   msg += tr( "\nStatistical Details for the Full Model set.\n" );
   msg += tr( "   Total Non-Zero Count:         %1\n" ).arg( istaa[ 0 ] );
   msg += tr( "   Count of Wavelengths:         %1\n" ).arg( istaa[ 1 ] );
   msg += tr( "   Count of %1 values:           %2\n" ).arg( xaxis,istaa[ 2 ] );
   msg += tr( "   Minimum Concentration:        %1\n" ).arg( dstaa[ 0 ] );
   msg += tr( "   Maximum Concentration:        %1\n" ).arg( dstaa[ 1 ] );
   msg += tr( "   Mean Concentration:           %1\n" ).arg( dstaa[ 4 ] );
   msg += tr( "   Median Concentration:         %1\n" ).arg( dstaa[ 9 ] );
   msg += tr( "   Mean Wavelength:              %1\n" ).arg( dstaa[ 2 ] );
   msg += tr( "   Median Wavelength:            %1\n" ).arg( dstaa[ 7 ] );
   msg += tr( "   Weighted Avg. Wavelength:     %1\n" ).arg( dstaa[ 5 ] );
   msg += tr( "   Mean %1:                      %2\n" ).arg( xaxis,dstaa[ 3 ] );
   msg += tr( "   Median %1:                    %2\n" ).arg( xaxis,dstaa[ 8 ] );
   msg += tr( "   Weighted Avg. %1:             %2\n" ).arg( xaxis,dstaa[ 6 ] );
   msg += tr( "\nStatistical Details for Wavelength %1 .\n" ).arg( wln1 );
   msg += tr( "   Total Non-Zero Count:         %1\n" ).arg( istaw1[ 0 ] );
   msg += tr( "   Count of Wavelengths:         %1\n" ).arg( istaw1[ 1 ] );
   msg += tr( "   Count of %1:                  %2\n" ).arg( xaxis,istaw1[ 2 ] );
   msg += tr( "   Minimum Concentration:        %1\n" ).arg( dstaw1[ 0 ] );
   msg += tr( "   Maximum Concentration:        %1\n" ).arg( dstaw1[ 1 ] );
   msg += tr( "   Mean Concentration:           %1\n" ).arg( dstaw1[ 4 ] );
   msg += tr( "   Median Concentration:         %1\n" ).arg( dstaw1[ 9 ] );
   msg += tr( "   Mean Wavelength:              %1\n" ).arg( dstaw1[ 2 ] );
   msg += tr( "   Median Wavelength:            %1\n" ).arg( dstaw1[ 7 ] );
   msg += tr( "   Weighted Avg. Wavelength:     %1\n" ).arg( dstaw1[ 5 ] );
   msg += tr( "   Mean %1:                      %2\n" ).arg( xaxis,dstaw1[ 3 ] );
   msg += tr( "   Median %1:                    %2\n" ).arg( xaxis,dstaw1[ 8 ] );
   msg += tr( "   Weighted Avg. %1:             %2\n" ).arg( xaxis,dstaw1[ 6 ] );
   msg += tr( "\nStatistical Details for Wavelength %1 .\n" ).arg( wln2 );
   msg += tr( "   Total Non-Zero Count:         %1\n" ).arg( istaw2[ 0 ] );
   msg += tr( "   Count of Wavelengths:         %1\n" ).arg( istaw2[ 1 ] );
   msg += tr( "   Count of %1:                  %2\n" ).arg( xaxis,istaw2[ 2 ] );
   msg += tr( "   Minimum Concentration:        %1\n" ).arg( dstaw2[ 0 ] );
   msg += tr( "   Maximum Concentration:        %1\n" ).arg( dstaw2[ 1 ] );
   msg += tr( "   Mean Concentration:           %1\n" ).arg( dstaw2[ 4 ] );
   msg += tr( "   Median Concentration:         %1\n" ).arg( dstaw2[ 9 ] );
   msg += tr( "   Mean Wavelength:              %1\n" ).arg( dstaw2[ 2 ] );
   msg += tr( "   Median Wavelength:            %1\n" ).arg( dstaw2[ 7 ] );
   msg += tr( "   Weighted Avg. Wavelength:     %1\n" ).arg( dstaw2[ 5 ] );
   msg += tr( "   Mean %1:                      %2\n" ).arg( xaxis,dstaw2[ 3 ] );
   msg += tr( "   Median %1:                    %2\n" ).arg( xaxis,dstaw2[ 8 ] );
   msg += tr( "   Weighted Avg. %1:             %2\n" ).arg( xaxis,dstaw2[ 6 ] );
   msg += tr( "\nStatistical Details for Wavelength %1 .\n" ).arg( wln3 );
   msg += tr( "   Total Non-Zero Count:         %1\n" ).arg( istaw3[ 0 ] );
   msg += tr( "   Count of Wavelengths:         %1\n" ).arg( istaw3[ 1 ] );
   msg += tr( "   Count of %1:                  %2\n" ).arg( xaxis,istaw3[ 2 ] );
   msg += tr( "   Minimum Concentration:        %1\n" ).arg( dstaw3[ 0 ] );
   msg += tr( "   Maximum Concentration:        %1\n" ).arg( dstaw3[ 1 ] );
   msg += tr( "   Mean Concentration:           %1\n" ).arg( dstaw3[ 4 ] );
   msg += tr( "   Median Concentration:         %1\n" ).arg( dstaw3[ 9 ] );
   msg += tr( "   Mean Wavelength:              %1\n" ).arg( dstaw3[ 2 ] );
   msg += tr( "   Median Wavelength:            %1\n" ).arg( dstaw3[ 7 ] );
   msg += tr( "   Weighted Avg. Wavelength:     %1\n" ).arg( dstaw3[ 5 ] );
   msg += tr( "   Mean %1:                      %2\n" ).arg( xaxis,dstaw3[ 3 ] );
   msg += tr( "   Median %1:                    %2\n" ).arg( xaxis,dstaw3[ 8 ] );
   msg += tr( "   Weighted Avg. %1:             %2\n" ).arg( xaxis,dstaw3[ 6 ] );
   msg += tr( "\nStatistical Details for Wavelength %1 .\n" ).arg( wln4 );
   msg += tr( "   Total Non-Zero Count:         %1\n" ).arg( istaw4[ 0 ] );
   msg += tr( "   Count of Wavelengths:         %1\n" ).arg( istaw4[ 1 ] );
   msg += tr( "   Count of %1:                  %2\n" ).arg( xaxis,istaw4[ 2 ] );
   msg += tr( "   Minimum Concentration:        %1\n" ).arg( dstaw4[ 0 ] );
   msg += tr( "   Maximum Concentration:        %1\n" ).arg( dstaw4[ 1 ] );
   msg += tr( "   Mean Concentration:           %1\n" ).arg( dstaw4[ 4 ] );
   msg += tr( "   Median Concentration:         %1\n" ).arg( dstaw4[ 9 ] );
   msg += tr( "   Mean Wavelength:              %1\n" ).arg( dstaw4[ 2 ] );
   msg += tr( "   Median Wavelength:            %1\n" ).arg( dstaw4[ 7 ] );
   msg += tr( "   Weighted Avg. Wavelength:     %1\n" ).arg( dstaw4[ 5 ] );
   msg += tr( "   Mean %1:                      %2\n" ).arg( xaxis,dstaw4[ 3 ] );
   msg += tr( "   Median %1:                    %2\n" ).arg( xaxis,dstaw4[ 8 ] );
   msg += tr( "   Weighted Avg. %1:             %2\n" ).arg( xaxis,dstaw4[ 6 ] );
   msg += tr( "\nStatistical Details for Wavelength %1 .\n" ).arg( wln5 );
   msg += tr( "   Total Non-Zero Count:         %1\n" ).arg( istaw5[ 0 ] );
   msg += tr( "   Count of Wavelengths:         %1\n" ).arg( istaw5[ 1 ] );
   msg += tr( "   Count of %1:                  %2\n" ).arg( xaxis,istaw5[ 2 ] );
   msg += tr( "   Minimum Concentration:        %1\n" ).arg( dstaw5[ 0 ] );
   msg += tr( "   Maximum Concentration:        %1\n" ).arg( dstaw5[ 1 ] );
   msg += tr( "   Mean Concentration:           %1\n" ).arg( dstaw5[ 4 ] );
   msg += tr( "   Median Concentration:         %1\n" ).arg( dstaw5[ 9 ] );
   msg += tr( "   Mean Wavelength:              %1\n" ).arg( dstaw5[ 2 ] );
   msg += tr( "   Median Wavelength:            %1\n" ).arg( dstaw5[ 7 ] );
   msg += tr( "   Weighted Avg. Wavelength:     %1\n" ).arg( dstaw5[ 5 ] );
   msg += tr( "   Mean %1:                      %2\n" ).arg( xaxis,dstaw5[ 3 ] );
   msg += tr( "   Median %1:                    %2\n" ).arg( xaxis,dstaw5[ 8 ] );
   msg += tr( "   Weighted Avg. %1:             %2\n" ).arg( xaxis,dstaw5[ 6 ] );
   msg += tr( "\nStatistical Details for %1 %2 .\n" ).arg( xaxis,sed1 );
   msg += tr( "   Total Non-Zero Count:         %1\n" ).arg( istas1[ 0 ] );
   msg += tr( "   Count of Wavelengths:         %1\n" ).arg( istas1[ 1 ] );
   msg += tr( "   Count of %1:        %2\n" ).arg( xaxis,istas1[ 2 ] );
   msg += tr( "   Minimum Concentration:        %1\n" ).arg( dstas1[ 0 ] );
   msg += tr( "   Maximum Concentration:        %1\n" ).arg( dstas1[ 1 ] );
   msg += tr( "   Mean Concentration:           %1\n" ).arg( dstas1[ 4 ] );
   msg += tr( "   Median Concentration:         %1\n" ).arg( dstas1[ 9 ] );
   msg += tr( "   Mean Wavelength:              %1\n" ).arg( dstas1[ 2 ] );
   msg += tr( "   Median Wavelength:            %1\n" ).arg( dstas1[ 7 ] );
   msg += tr( "   Weighted Avg. Wavelength:     %1\n" ).arg( dstas1[ 5 ] );
   msg += tr( "   Mean %1:                      %2\n" ).arg( xaxis,dstas1[ 3 ] );
   msg += tr( "   Median %1:                    %2\n" ).arg( xaxis,dstas1[ 8 ] );
   msg += tr( "   Weighted Avg. %1:             %2\n" ).arg( xaxis,dstas1[ 6 ] );
   msg += tr( "\nStatistical Details for %1 %2 .\n" ).arg( xaxis,sed2 );
   msg += tr( "   Total Non-Zero Count:         %1\n" ).arg( istas2[ 0 ] );
   msg += tr( "   Count of Wavelengths:         %1\n" ).arg( istas2[ 1 ] );
   msg += tr( "   Count of %1:        %2\n" ).arg( xaxis,istas2[ 2 ] );
   msg += tr( "   Minimum Concentration:        %1\n" ).arg( dstas2[ 0 ] );
   msg += tr( "   Maximum Concentration:        %1\n" ).arg( dstas2[ 1 ] );
   msg += tr( "   Mean Concentration:           %1\n" ).arg( dstas2[ 4 ] );
   msg += tr( "   Median Concentration:         %1\n" ).arg( dstas2[ 9 ] );
   msg += tr( "   Mean Wavelength:              %1\n" ).arg( dstas2[ 2 ] );
   msg += tr( "   Median Wavelength:            %1\n" ).arg( dstas2[ 7 ] );
   msg += tr( "   Weighted Avg. Wavelength:     %1\n" ).arg( dstas2[ 5 ] );
   msg += tr( "   Mean %1:                      %2\n" ).arg( xaxis,dstas2[ 3 ] );
   msg += tr( "   Median %1:                    %2\n" ).arg( xaxis,dstas2[ 8 ] );
   msg += tr( "   Weighted Avg. %1:             %2\n" ).arg( xaxis,dstas2[ 6 ] );
   msg += tr( "\nStatistical Details for %1 %2 .\n" ).arg( xaxis,sed3 );
   msg += tr( "   Total Non-Zero Count:         %1\n" ).arg( istas3[ 0 ] );
   msg += tr( "   Count of Wavelengths:         %1\n" ).arg( istas3[ 1 ] );
   msg += tr( "   Count of %1:        %2\n" ).arg( xaxis,istas3[ 2 ] );
   msg += tr( "   Minimum Concentration:        %1\n" ).arg( dstas3[ 0 ] );
   msg += tr( "   Maximum Concentration:        %1\n" ).arg( dstas3[ 1 ] );
   msg += tr( "   Mean Concentration:           %1\n" ).arg( dstas3[ 4 ] );
   msg += tr( "   Median Concentration:         %1\n" ).arg( dstas3[ 9 ] );
   msg += tr( "   Mean Wavelength:              %1\n" ).arg( dstas3[ 2 ] );
   msg += tr( "   Median Wavelength:            %1\n" ).arg( dstas3[ 7 ] );
   msg += tr( "   Weighted Avg. Wavelength:     %1\n" ).arg( dstas3[ 5 ] );
   msg += tr( "   Mean %1:                      %2\n" ).arg( xaxis,dstas3[ 3 ] );
   msg += tr( "   Median %1:                    %2\n" ).arg( xaxis,dstas3[ 8 ] );
   msg += tr( "   Weighted Avg. %1:             %2\n" ).arg( xaxis,dstas3[ 6 ] );
   msg += tr( "\nStatistical Details for %1 %2 .\n" ).arg( xaxis,sed4 );
   msg += tr( "   Total Non-Zero Count:         %1\n" ).arg( istas4[ 0 ] );
   msg += tr( "   Count of Wavelengths:         %1\n" ).arg( istas4[ 1 ] );
   msg += tr( "   Count of %1:        %2\n" ).arg( xaxis,istas4[ 2 ] );
   msg += tr( "   Minimum Concentration:        %1\n" ).arg( dstas4[ 0 ] );
   msg += tr( "   Maximum Concentration:        %1\n" ).arg( dstas4[ 1 ] );
   msg += tr( "   Mean Concentration:           %1\n" ).arg( dstas4[ 4 ] );
   msg += tr( "   Median Concentration:         %1\n" ).arg( dstas4[ 9 ] );
   msg += tr( "   Mean Wavelength:              %1\n" ).arg( dstas4[ 2 ] );
   msg += tr( "   Median Wavelength:            %1\n" ).arg( dstas4[ 7 ] );
   msg += tr( "   Weighted Avg. Wavelength:     %1\n" ).arg( dstas4[ 5 ] );
   msg += tr( "   Mean %1:                      %2\n" ).arg( xaxis,dstas4[ 3 ] );
   msg += tr( "   Median %1:                    %2\n" ).arg( xaxis,dstas4[ 8 ] );
   msg += tr( "   Weighted Avg. %1:             %2\n" ).arg( xaxis,dstas4[ 6 ] );
   msg += tr( "\nStatistical Details for %1 %2 .\n" ).arg( xaxis,sed5 );
   msg += tr( "   Total Non-Zero Count:         %1\n" ).arg( istas5[ 0 ] );
   msg += tr( "   Count of Wavelengths:         %1\n" ).arg( istas5[ 1 ] );
   msg += tr( "   Count of %1:        %2\n" ).arg( xaxis,istas5[ 2 ] );
   msg += tr( "   Minimum Concentration:        %1\n" ).arg( dstas5[ 0 ] );
   msg += tr( "   Maximum Concentration:        %1\n" ).arg( dstas5[ 1 ] );
   msg += tr( "   Mean Concentration:           %1\n" ).arg( dstas5[ 4 ] );
   msg += tr( "   Median Concentration:         %1\n" ).arg( dstas5[ 9 ] );
   msg += tr( "   Mean Wavelength:              %1\n" ).arg( dstas5[ 2 ] );
   msg += tr( "   Median Wavelength:            %1\n" ).arg( dstas5[ 7 ] );
   msg += tr( "   Weighted Avg. Wavelength:     %1\n" ).arg( dstas5[ 5 ] );
   msg += tr( "   Mean %1:                      %2\n" ).arg( xaxis,dstas5[ 3 ] );
   msg += tr( "   Median %1:                    %2\n" ).arg( xaxis,dstas5[ 8 ] );
   msg += tr( "   Weighted Avg. %1:             %2\n" ).arg( xaxis,dstas5[ 6 ] );

   // Open the dialog and display the report text
   US_Editor* editd = new US_Editor( US_Editor::DEFAULT, true );
   editd->setWindowTitle( tr( "Multi-Wavelength Spectra Statistics" ) );
   editd->move( pos() + QPoint( 200, 200 ) );
   editd->resize( 600, 500 );
   editd->e->setFont( QFont( US_Widgets::fixedFont().family(),
                             US_GuiSettings::fontSize() ) );
   editd->e->setText( msg );
   editd->show();
}

// Plot the current data record
void US_MwlSpectra::plot_current( void )
{
   if ( mdlxyz.count() == 0 )
      return;

   plot_titles();     // Set the titles

   plot_all();        // Plot the data
}

// Compose plot titles for the current record
void US_MwlSpectra::plot_titles( void )
{
   QString prec     = cb_pltrec->currentText();

   // Plot Title
   QString title = xaxis + tr( " Spectrum\n"
                       "Run ID :  " ) + runID + "\n" + xlegend + " :  " + prec;

   data_plot->setTitle( title );
}

// Draw wavelength,concentration curves for the current sedcoeff plot record
void US_MwlSpectra::plot_all( void )
{
   dataPlotClear( data_plot );
   grid         = us_grid( data_plot );

   // Make sure ranges are set up, then build a smoothed data vector
   have_rngs    = false;
   compute_ranges();

   US_Math2::gaussian_smoothing( pltyvals, nsmooth );

int knz=0;
double cmx=0.0;
for(int ii=0;ii<kpoint;ii++)
{ double cvl=pltyvals[ii];if(cvl!=0.0) knz++; cmx=qMax(cmx,cvl); }
DbgLv(1) << "PltA: kpoint" << kpoint << "knz" << knz << "cmx" << cmx;
   // Point to the X,Y vectors
   double* rr   = pltxvals.data();
   double* vv   = pltyvals.data();

   QPen          pen_plot( US_GuiSettings::plotCurve() );
   QString       title  = tr( "s=%1 sindex=%2" ).arg( sed_plot ).arg( sedxp );
   QwtPlotCurve* curv   = us_curve( data_plot, title );

   curv->setPen( pen_plot );            // Normal pen

   curv->setSamples( rr, vv, kpoint );  // Build a sed.coeff. curve
//DbgLv(1) << "PltA:   scx" << scx << "rr0 vv0 rrn vvn"
// << rr[0] << rr[kpoint-1] << vv[0] << vv[kpoint-1];

DbgLv(1) << "PltA: last_xmin" << last_xmin;
   if ( last_xmin < 0.0 )
   {  // If first time, set scales based on actual values present
      last_xmin    = lmb_start;         // Set X limits
      last_xmax    = lmb_end;
      last_ymin    = 0.0;               // Set Y limits
      last_ymax    = cn_max; 
      data_plot->setAxisScale( QwtPlot::xBottom, last_xmin, last_xmax );
      data_plot->setAxisScale( QwtPlot::yLeft  , last_ymin, last_ymax );
   }

   else
   {  // After first time, use the same plot ranges as set before
      data_plot->setAxisScale( QwtPlot::xBottom, last_xmin, last_xmax );
      data_plot->setAxisScale( QwtPlot::yLeft  , last_ymin, last_ymax );
   }

   if ( ck_srngsum->isChecked() )
   {
      data_plot->setAxisAutoScale( QwtPlot::yLeft );
   }

   // Draw the plot
   data_plot->replot();

   // Pick up the actual bounds plotted (including any Config changes)
#if QT_VERSION < 0x050000
   QwtScaleDiv* sdx = data_plot->axisScaleDiv( QwtPlot::xBottom );
   QwtScaleDiv* sdy = data_plot->axisScaleDiv( QwtPlot::yLeft   );
#else
   QwtScaleDiv* sdx = (QwtScaleDiv*)&data_plot->axisScaleDiv( QwtPlot::xBottom );
   QwtScaleDiv* sdy = (QwtScaleDiv*)&data_plot->axisScaleDiv( QwtPlot::yLeft   );
#endif
   last_xmin      = sdx->lowerBound();
   last_xmax      = sdx->upperBound();
   last_ymin      = sdy->lowerBound();
   last_ymax      = sdy->upperBound();
DbgLv(1) << "PltA: xlo xhi" << last_xmin << last_xmax
 << "ylo yhi" << last_ymin << last_ymax;
}

// Slot to handle a change in start or end sedimentation coefficient
void US_MwlSpectra::changeSedcoeff()
{
DbgLv(1) << "chgSedcoeff";
   // Recompute ranges
   have_rngs    = false;
   compute_ranges();

   // Re-do the list of plot records
   connect_ranges( false );
   cb_pltrec->clear();

   for ( int jj = sedxs; jj < sedxe; jj++ )
      cb_pltrec->addItem( QString::number( sedcoes[ jj ] ) );

   // Recompute ranges
   have_rngs    = false;
   compute_ranges();

   // Reset the current plot record
   sedxp      = dvec_index( sedcoes, sed_plot  );
   sedxp      = qMax( sedxs, qMin( sedxe, sedxp ) );
   connect_ranges( true );
   cb_pltrec->setCurrentIndex( sedxp );

}

// Slot to handle a change in start or end lambda
void US_MwlSpectra::changeLambda()
{
DbgLv(1) << "chgLambda";
   // Recompute ranges
   have_rngs    = false;
   compute_ranges();

   // Reset plot X limits
   last_xmin    = lmb_start;
   last_xmax    = lmb_end;
}
 
// Slot to handle a change in the plot record
void US_MwlSpectra::changeRecord( void )
{
   recx           = cb_pltrec->currentIndex();
DbgLv(1) << "chgRec: recx" << recx;
   bool plt_one   = ! le_status->text().contains( tr( "saving" ) );

   // Plot what we have
   plot_current();

   // Update status text (if not part of movie save) and set prev/next arrows
   if ( plt_one )
      le_status->setText( lb_pltrec->text() + " " + cb_pltrec->currentText() );

   if ( ! ck_srngsum->isChecked() )
   {
      pb_prev  ->setEnabled( ( recx > 0 ) );
      pb_next  ->setEnabled( ( recx < ( cb_pltrec->count() - 1 ) ) );
   }
}

// Slot to handle a click to go to the previous record
void US_MwlSpectra::prevPlot( void )
{
   int pltrx      = cb_pltrec->currentIndex() - 1;

   if ( pltrx < 1 )
   {
      pltrx          = 0;
      pb_prev->setEnabled( false );
   }

#if QT_VERSION < 0x050000
   QwtScaleDiv* sdx = data_plot->axisScaleDiv( QwtPlot::xBottom );
   QwtScaleDiv* sdy = data_plot->axisScaleDiv( QwtPlot::yLeft   );
#else
   QwtScaleDiv* sdx = (QwtScaleDiv*)&data_plot->axisScaleDiv( QwtPlot::xBottom );
   QwtScaleDiv* sdy = (QwtScaleDiv*)&data_plot->axisScaleDiv( QwtPlot::yLeft   );
#endif
   last_xmin      = sdx->lowerBound();
   last_xmax      = sdx->upperBound();
   last_ymin      = sdy->lowerBound();
   last_ymax      = sdy->upperBound();

   cb_pltrec->setCurrentIndex( pltrx );
}

// Slot to handle a click to go to the next record
void US_MwlSpectra::nextPlot( void )
{
   int pltrx      = cb_pltrec->currentIndex() + 1;
   int nitems     = cb_pltrec->count();

   if ( ( pltrx + 2 ) > nitems )
   {
      pltrx          = nitems - 1;
      pb_next->setEnabled( false );
   }

#if QT_VERSION < 0x050000
   QwtScaleDiv* sdx = data_plot->axisScaleDiv( QwtPlot::xBottom );
   QwtScaleDiv* sdy = data_plot->axisScaleDiv( QwtPlot::yLeft   );
#else
   QwtScaleDiv* sdx = (QwtScaleDiv*)&data_plot->axisScaleDiv( QwtPlot::xBottom );
   QwtScaleDiv* sdy = (QwtScaleDiv*)&data_plot->axisScaleDiv( QwtPlot::yLeft   );
#endif
   last_xmin      = sdx->lowerBound();
   last_xmax      = sdx->upperBound();
   last_ymin      = sdy->lowerBound();
   last_ymax      = sdy->upperBound();

   cb_pltrec->setCurrentIndex( pltrx );
}

// Slot to handle a change in the number of smoothing points
void US_MwlSpectra::changeSmooth()
{
DbgLv(1) << "chgSmooth:";
   nsmooth        = ct_smooth->value();

   plot_all();
}

// Compute the plot range indexes implied by current settings
void US_MwlSpectra::compute_ranges()
{
   if ( have_rngs )         // If we just did this computation, return now
      return;

   sed_start  = cb_sstart ->currentText().toDouble();  // Sedcoeff start
   sed_end    = cb_send   ->currentText().toDouble();  // Sedcoeff end
   lmb_start  = cb_lstart ->currentText().toInt();     // Lambda start
   lmb_end    = cb_lend   ->currentText().toInt();     // Lambda end
   sed_plot   = cb_pltrec ->currentText().toDouble();  // Sedcoeff plot record
   lmbxs      = lambdas.indexOf( lmb_start );          // Lambda start index
   lmbxe      = lambdas.indexOf( lmb_end   ) + 1;      // Lambda end index
   sedxs      = dvec_index( sedcoes, sed_start );      // SedCoef start index
   sedxe      = dvec_index( sedcoes, sed_end   ) + 1;  // SedCoef end index
   sedxp      = dvec_index( sedcoes, sed_plot  );      // SedCoef plot rec index
   recx       = cb_pltrec->currentIndex();             // Index in plot sedcos
   klambda    = lmbxe - lmbxs;                         // Count of plot lambdas
   ksedcos    = sedxe - sedxs;                         // Count of plot sedcos
   kpoint     = klambda;                               // Plot x,y points
   pltxvals.clear();
   pltyvals.clear();

   if ( sedxp < 0  ||  recx < 0 )
   {
DbgLv(1) << "cmpR:  (1)sS sE sxS sxE" << sed_start << sed_end << sedxs << sedxe
 << "sxP rx sP" << sedxp << recx << sed_plot;
      sed_plot   = qMax( sed_start, qMin( sed_end, sed_plot ) );
      sedxp      = dvec_index( sedcoes, sed_plot );
      recx       = sedxp - sedxs;
   }
DbgLv(1) << "cmpR:  sS sE sxS sxE" << sed_start << sed_end << sedxs << sedxe
 << "sxP rx sP" << sedxp << recx << sed_plot;

   // Get lambda values in current plot range
   for ( int ii = lmbxs; ii < lmbxe; ii++ )
      pltxvals << (double)lambdas[ ii ];

   if ( ! ck_srngsum->isChecked() )
   {  // Get concentrations from current sedcoeff record, current lambda range
      for ( int ii = lmbxs; ii < lmbxe; ii++ )
         pltyvals << concdat[ sedxp ][ ii ];
   }

   else
   {  // Get sum of concentrations in sedcoeff range, current lambda range
      double ssum;

      for ( int ii = lmbxs; ii < lmbxe; ii++ )
      {
         sedxs      = dvec_index( sedcoes, sed_start );
         sedxe      = dvec_index( sedcoes, sed_end   ) + 1;
         sed_plot   = ( sed_start + sed_end ) * 0.5;
         ssum       = 0.0;

         for ( int jj = sedxs; jj < sedxe; jj++ )
         {
            ssum      += concdat[ jj ][ ii ];
         }

         pltyvals << ssum;
      }
   }

   have_rngs  = true;                                  // Mark ranges computed
}

// Connect or Disconnect plot-range related controls
void US_MwlSpectra::connect_ranges( bool conn )
{
   if ( conn )
   {  // Connect the range-related controls
      connect( cb_sstart,  SIGNAL( currentIndexChanged( int ) ),
               this,       SLOT  ( changeSedcoeff(          ) ) );
      connect( cb_send,    SIGNAL( currentIndexChanged( int ) ),
               this,       SLOT  ( changeSedcoeff(          ) ) );
      connect( cb_lstart,  SIGNAL( currentIndexChanged( int ) ),
               this,       SLOT  ( changeLambda(            ) ) );
      connect( cb_lend,    SIGNAL( currentIndexChanged( int ) ),
               this,       SLOT  ( changeLambda(            ) ) );
      connect( cb_pltrec,  SIGNAL( currentIndexChanged( int ) ),
               this,       SLOT  ( changeRecord(            ) ) );
   }

   else
   {  // Disconnect the range-related controls
      cb_sstart ->disconnect();
      cb_send   ->disconnect();
      cb_lstart ->disconnect();
      cb_lend   ->disconnect();
      cb_pltrec ->disconnect();
   }
}

// Slot to show a 2-D movie
void US_MwlSpectra::show_2d_movie()
{
DbgLv(1) << "Show 2D Movie";
   // Loop to plot each record in the current cell
   int krecs       = cb_pltrec->count();
   int svrec       = recx;                  // Save currently plotted record
   int mdelay      = (int)qRound( ct_delay->value() * 1000.0 );

   for ( int prx = 0; prx < krecs; prx++ )
   {
      cb_pltrec->setCurrentIndex( prx );    // Plot each record in the range
      qApp->processEvents();
      US_Sleep::msleep( mdelay );           // Delay between frames
   }

   cb_pltrec->setCurrentIndex( svrec );     // Restore previous plot record
   qApp->processEvents();
}

// Slot to open a dialog for 3-D plotting
void US_MwlSpectra::plot_3d()
{
DbgLv(1) << "Plt3D";
   // Create a 3D plot version of the data with ranges and smoothing

   compute_ranges();

   p3dxyz.clear();

   // Create the 2-D concentration vectors for each sed.coeff.
   for ( int jj = sedxs; jj < sedxe; jj++ )
   {
      QVector< double > cvect;
      int jd        = jj * nlambda;
      double sedv   = xyzdat[ jd ].y();

      for ( int ii = lmbxs; ii < lmbxe; ii++ )
      {  // Create the concentration vector for this sed.coeff.
         cvect << xyzdat[ jd + ii ].z();
      }

      // Smooth the vector
      US_Math2::gaussian_smoothing( cvect, nsmooth );
      jd           += lmbxs;

      // Output the XYZ points with smoothed concentrations
      for ( int ii = 0; ii < cvect.count(); ii++, jd++ )
      {
         double wlnv  = xyzdat[ jd ].x();
         double conv  = cvect [ ii ];

         p3dxyz << QVector3D( wlnv, sedv, conv );
      }
   }

   // Open a 3D plot control window or reference an opened one, then send data
   if ( p3d_ctld == NULL )
   {
      p3d_pltw     = NULL;
      p3d_ctld     = new US_MwlSPlotControl( this, &p3dxyz );
      p3d_ctld->show();
      // Position inside the main window relative to its upper left
      p3d_ctld->move( x() + 500, y() + 100 );
      connect( p3d_ctld, SIGNAL( has_closed()     ),
               this,     SLOT  ( p3dctrl_closed() ) );
   }

   else
   {
      p3d_ctld->setFocus();
      p3d_ctld->do_3dplot();

      p3d_pltw     = p3d_ctld->widget_3dplot();

      if ( p3d_pltw != NULL )
      {
         p3d_pltw->reloadData( &p3dxyz );

         QString ptitle = tr( "MWL 3-D Plot, Spectra" );

         p3d_pltw->setPlotTitle( ptitle );
         p3d_pltw->replot( false );
      }
   }

}

// Slot to save the current plot
void US_MwlSpectra::save_plot()
{
DbgLv(1) << "Save Plot";
   QString savedir = US_Settings::reportDir() + "/" + runID;
   QDir().mkpath( savedir );
   savedir         = savedir.replace( "\\", "/" ) + "/";
   QString fname2d = runID + ".SSpectra_sedRec_RRRRR_2D.png";
   QString fname3d = runID + ".SSpectra_3D.png";
   p3d_pltw        = ( p3d_ctld == NULL ) ? NULL : p3d_ctld->widget_3dplot();
   int nfiles      = ( p3d_pltw != NULL ) ? 2 : 1;

   if ( nfiles == 2 )
   {  // If there is a 3D window, first save a PNG of that window

      p3d_pltw->replot( false );                // Do the plot
      QString fpath3d = savedir + fname3d;

      p3d_pltw->save_plot( fpath3d, QString( "png" ) );
   }

   // Always save a PNG of the 2-D plot
   QString rec_str = cb_pltrec->currentText().replace( ".", "p" );
   fname2d         = fname2d.replace( "RRRRR", rec_str );
   QString fpath2d = savedir + fname2d;

   US_GuiUtil::save_png( fpath2d, data_plot );

   // Report the file(s) saved
   QString mtitle  = ( nfiles == 1 )
                   ? tr( "Plot File Saved" )
                   : tr( "Plot Files Saved" );
   QString msg     = tr( "In the directory\n     %1,\n\n" ).arg( savedir );
   if ( nfiles == 1 )
      msg            += tr( "File\n    %1 was saved." ).arg( fname2d );
   else
      msg            += tr( "Files\n    %1 ;  and\n    %2\nwere saved." )
                        .arg( fname3d ).arg( fname2d );

   QMessageBox::information( this, mtitle, msg );
}

// Slot to save the current movie
void US_MwlSpectra::save_movie()
{
DbgLv(1) << "Save 2D Movie";
   // Loop to plot each record in the cell and save an image to file
   int krecs       = cb_pltrec->count();
   int svrec       = recx;                  // Save currently plotted record
   QStringList fnames;
   QString savedir = US_Settings::reportDir() + "/" + runID;
   QDir().mkpath( savedir );
   savedir         = savedir.replace( "\\", "/" ) + "/";
   QString bfname  = runID + ".2D_frame_XXXXX.png";
   QString bstat   = tr( "Of %1 records, saving frame " ).arg( krecs );
   le_status->setText( bstat );

   for ( int prx = 0; prx < krecs; prx++ )
   {
      cb_pltrec->setCurrentIndex( prx );    // Plot each record in the range
      qApp->processEvents();

      QString frm_str = QString().sprintf( "%05d", ( prx + 1 ) );
      QString fname   = QString( bfname ).replace( "XXXXX", frm_str );
      QString fpath   = savedir + fname;

      le_status->setText( bstat + frm_str );

      US_GuiUtil::save_png( fpath, data_plot );
      fnames << fname;
   }

   cb_pltrec->setCurrentIndex( svrec );     // Restore previous plot record
   qApp->processEvents();

   QMessageBox::information( this, tr( "Frame Files Saved" ),
      tr( "In the directory\n     %1,\n\n%2 2-D movie frame files"
          " were saved:\n     %3\n     ...\n     %4 ." )
      .arg( savedir ).arg( krecs ).arg( fnames[ 0 ] )
      .arg( fnames[ krecs - 1 ] ) );
}

// Utility to find an index in a QVector<double> to a value epsilon match
int US_MwlSpectra::dvec_index( QVector< double >& dvec, const double dval )
{
   int indx    = dvec.indexOf( dval );   // Try to find an exact match

   if ( indx < 0 )
   {  // If no exact match was found, look for a match within epsilon
      double dmin = dval;

      for ( int jj = 0; jj < dvec.size(); jj++ )
      {  // Search doubles vector
         if ( dvirt_equal( dvec[ jj ], dval ) )
         {  // If vector value matches within epsilon, break and return
            indx            = jj;
            break;
         }

         double ddif = qAbs( dvec[ jj ] - dval );
         if ( ddif < dmin )
         {  // Save index to closest match so far, as a back-up
            indx            = jj;
            dmin            = ddif;
         }
      }
   }

   return indx;
}

// Utility to flag if two doubles are "virtually" equal
bool US_MwlSpectra::dvirt_equal( const double d1, const double d2)
{
   const double eps   = 1.e-4;

   return ( qAbs( ( d1 - d2 ) / d2 ) < eps );
}

// Slot to handle the close of the 3D plot control dialog
void US_MwlSpectra::p3dctrl_closed()
{
   p3d_ctld     = NULL;
   p3d_pltw     = NULL;
}

// Reset Disk/DB controls whenever the data source is changed in any dialog
void US_MwlSpectra::update_disk_db( bool isDB )
{
   if ( isDB )
      dkdb_cntrls->set_db();
   else
      dkdb_cntrls->set_disk();
}

// Utility to compute statistics for all or specific record
void US_MwlSpectra::bld_stats( double wlnv, double sedv, double conv,
      QVector< int >& istats, QVector< double >& dstats,
      QVector< double >& lwlns, QVector< double >& lseds,
      QVector< double >& lcons )
{
   if ( conv == 0.0 )  return;

   // Bump non-zero count and counts of sed,lmb
   istats[ 0 ]++;
   istats[ 1 ] += lwlns.contains( wlnv ) ? 0 : 1;
   istats[ 2 ] += lseds.contains( sedv ) ? 0 : 1;

   // Get concentration min,max and accumulate sums for means
   dstats[ 0 ]  = ( istats[ 0 ] == 1 ) ? conv : qMin( dstats[ 0 ], conv );
   dstats[ 1 ]  = ( istats[ 0 ] == 1 ) ? conv : qMax( dstats[ 1 ], conv );
   dstats[ 2 ] += wlnv;
   dstats[ 3 ] += sedv;
   dstats[ 4 ] += conv;

   // Accumulate weighted sums
   dstats[ 5 ] += ( wlnv * conv );
   dstats[ 6 ] += ( sedv * conv );

   // Add values to record lists
   lwlns << wlnv;
   lseds << sedv;
   lcons << conv;
}

// Utility to finalize statistical computations for a record
void US_MwlSpectra::final_stats( QVector< int >& istats,
      QVector< double >& dstats, QVector< double >& lwlns,
      QVector< double >& lseds, QVector< double >& lcons )
{
   // istats 0-2:  nnz, nwvl, nsed
   // dstats 0-4:  minc, maxc, wmean, smean, cmean,
   //        5-9:  wwavg, swavg, mediw, medis, medic
   
   // Complete averages
   double dnnz        = (double)istats[ 0 ];
   double ctot        = dstats[ 4 ];
   dstats[ 2 ]       /= dnnz;
   dstats[ 3 ]       /= dnnz;
   dstats[ 4 ]       /= dnnz;
   dstats[ 5 ]       /= ctot;
   dstats[ 6 ]       /= ctot;

   // Sort value lists and compute medians
   qSort( lwlns );
   qSort( lseds );
   qSort( lcons );
   dstats[ 7 ]        = lwlns[ lwlns.count() / 2 ];
   dstats[ 8 ]        = lseds[ lseds.count() / 2 ];
   dstats[ 9 ]        = lcons[ lcons.count() / 2 ];
}

// Slot for change in S Range Sum check box state
void US_MwlSpectra::sum_check()
{
   if ( lambdas.count() < 1 )
      return;

   // Enable/Disable elements as related to S Range Sum
   bool sumchkd       = ck_srngsum->isChecked();
   bool notsmck       = ! sumchkd;

   cb_pltrec ->setEnabled( notsmck );
   pb_prev   ->setEnabled( notsmck );
   pb_next   ->setEnabled( notsmck );
   pb_svdata ->setEnabled( sumchkd );
   pb_movie2d->setEnabled( notsmck );
   pb_plot3d ->setEnabled( notsmck );
   ct_delay  ->setEnabled( notsmck );
   pb_svmovie->setEnabled( notsmck );

   if ( sumchkd )
   {  // Set S value to plot to midway point between S limits
      sed_start  = cb_sstart ->currentText().toDouble();
      sed_end    = cb_send   ->currentText().toDouble();
      sed_plot   = ( sed_start + sed_end ) * 0.5;
      sedxs      = dvec_index( sedcoes, sed_start );
      sedxe      = dvec_index( sedcoes, sed_end   );
      sedxp      = dvec_index( sedcoes, sed_plot  );
      sed_plot   = ( sedxp < 0 ) ? sed_plot : sedcoes[ sedxp ]; 
      sedxp      = ( sedxp < 0 ) ? ( ( sedxs + sedxe ) / 2 ) : sedxp;
      cb_pltrec->disconnect();
      cb_pltrec->setCurrentIndex( sedxp );
      connect( cb_pltrec,    SIGNAL( currentIndexChanged( int ) ),
               this,         SLOT  ( changeRecord( )            ) );
      le_status->setText( lb_pltrec->text() + "  "
            + tr( "sum for S from %1 to %2" )
            .arg( sed_start ).arg( sed_end ) );
   }

   plot_current();

   return;
}

// Slot for Save Data button clicked
void US_MwlSpectra::save_data()
{
   if ( lambdas.count() < 1 )
      return;

   // Compose a default CSV file name, based on Sedcoeff range
   QString tpart = m_tpart;   // Model description triple part
   QString apart = m_apart;   // Model description analysis part
   lmb_start     = cb_lstart ->currentText().toInt();     // Lambda start
   lmb_end       = cb_lend   ->currentText().toInt();     // Lambda end
   sed_start     = cb_sstart ->currentText().toDouble();  // Sedcoeff start
   sed_end       = cb_send   ->currentText().toDouble();  // Sedcoeff end
   tpart         = tpart.left( 2 ) + QString( "%1-%2" )
                   .arg( lmb_start ).arg( lmb_end );
   int sedstr    = qRound( sed_start );
   int sedend    = qRound( sed_end   );
   QString spart = QString( "_S%1-%2" ).arg( sedstr ).arg( sedend );
   apart         = apart.section( "_", -3, -3 );
   QString dname = "wavelen-srangesum" + spart + "." + tpart + "."
                   + apart + ".csv";
DbgLv(1) << "SvD: dname" << dname;
   QString rdir  = US_Settings::resultDir().replace( "\\", "/" )
                   + "/" + runID + "/";

   // Open a dialog to allow the file name to be modified
   QString ename = dname;
   bool ok;

   QMessageBox mbox;
   QString msg   = tr( "In the results/run directory<br/>"
                       "&nbsp;&nbsp;%1 ,<br/><br/>"
                       "a new CSV Wavelength-SRangeSum file"
                       " will be created.<br/>You may customize its name"
                       " or use the default provided.<br/>" )
                   .arg( rdir );
   ename         = QInputDialog::getText( this,
         tr( "File Name: Wavelength-vs-SRangeSum CSV" ),
         msg, QLineEdit::Normal, ename, &ok );

   if ( ok  &&  ename != dname )
   {
      dname         = ename.remove( QRegExp( "[^\\w\\d._-]" ) );
   }

   QString dpath = rdir + dname;
DbgLv(1) << "SvD: dpath" << dpath;
   QDir().mkpath( rdir );

   // Now create and write the Wavelength-vs-SRangeSum CSV file
   QFile csvo_f( dpath );
   if ( ! csvo_f.open( QIODevice::WriteOnly | QIODevice::Text ) )
   {
      QMessageBox::critical( this, tr( "CSV File Open ERROR" ),
                             tr( "Output open error:\n%1" ).arg( dpath ) );
DbgLv(1) << "SvD:  *ERROR* out file open failed.";
      return;
   }

   double* rr   = pltxvals.data();
   double* vv   = pltyvals.data();
   int nwavln   = pltxvals.count();
DbgLv(1) << "SvD:  nwaveln" << nwavln << "x0, y0" << rr[0] << vv[0];
   QTextStream ts( &csvo_f );
   const QString dquote( "\"" );
   const QString comma( "," );
   const QString endln( "\n" );
   ts << dquote + "Wavelength(nm)" + dquote + comma
       + dquote + "S-Range-Concentration_Sum" + dquote + endln;

   for ( int ii = 0; ii < nwavln; ii++ )
   {
      ts << dquote + QString().sprintf( "%d", (int)rr[ ii ] ) + dquote + comma
          + dquote + QString().sprintf( "%10.4e", vv[ ii ] ) + dquote + endln;
   }

   csvo_f.close();

   QMessageBox::information( this, tr( "CSV File Saved" ),
                             tr( "%1 wavelength/concentration lines written\n"
                             "to file %2 ." ).arg( nwavln ).arg( dname ) );
   return;
}

// get component x value of given type
double US_MwlSpectra::comp_value(const US_Model::SimulationComponent* sc, int type)
{
    double xval = sc->s;

    switch ( type )
    {  // get appropriate value, based on type
        case 1:                         // x value is sedimentation coefficient
            xval    = sc->s * 1e+13;
            break;

        case 2:                         // x value is molecular weight
            xval    = sc->mw;
            break;

        case 3:                         // x value is diffusion coefficient
            xval    = sc->D * 1e+6;
            break;

        case 4:                         // x value is f/f0
            xval    = sc->f_f0;
            break;

        case 5:                         // x value is vbar20
            xval    = sc->vbar20;
            break;

        case 6:
            xval = log(sc->mw); //x value is log(mw)

        default:
            break;
    }

    return xval;
}

// React to a change in the X type of plots
void US_MwlSpectra::changedPlotX( bool on_state ) {
    if (!on_state) return;

    DbgLv(1) << "changedPlotX" << on_state;
    bool x_is_sw = rb_pltsw->isChecked();
    bool x_is_MW = rb_pltMW->isChecked();
    bool x_is_Dw = rb_pltDw->isChecked();
    bool x_is_ff0 = rb_pltff0->isChecked();
    bool x_is_vb = rb_pltvb->isChecked();
    bool x_is_MWl = rb_pltMWl->isChecked();
    xtype = 0;

    if (x_is_sw) {
        DbgLv(1) << "  PX=Sed.Coeff";
        xtype = 1;
        xaxis = "Sed. Coeff.";
        xlegend = "s (x 10^13)";
    } else if (x_is_MW) {
        DbgLv(1) << "  PX=Molec.Wt.";
        xtype = 2;
        xaxis = "Molec. Wt.";
        xlegend = "mw (x)";
    } else if (x_is_Dw) {
        DbgLv(1) << "  PX=Diff.Coeff";
        xtype = 3;
        xaxis = "Diff. Coeff..";
        xlegend = "D (x 10^6)";
    } else if (x_is_ff0) {
        DbgLv(1) << "  PX=f/f0";
        xtype = 4;
        xaxis = "f/f0";
        xlegend = "f/f0 (x)";
    } else if (x_is_vb) {
        DbgLv(1) << "  PX=Vbar";
        xtype = 5;
        xaxis = "Vbar";
        xlegend = "vbar (x)";
    } else if (x_is_MWl) {
        DbgLv(1) << "  PX=Molec.Wt.log";
        xtype = 6;
        xaxis = "Molec.Wt.log";
        xlegend = "log(mw) (x)";
    }
    mdlxyz .clear();
    xyzdat .clear();
    lambdas.clear();
    sedcoes.clear();
    concdat.clear();
    wl_min        = 1e+39;
    wl_max        = 1e-39;
    x_min        = 1e+39;
    x_max        = 1e-39;
    co_min        = 1e+39;
    co_max        = 1e-39;
    nipoint       = 0;
    nnpoint       = 0;
    ntpoint       = 0;

    DbgLv(1) << "LD: loadedmodels:" << loadedmodels.count() << " mdescs:" << mdescs.count();
    for ( int jj = 0; jj < loadedmodels.count(); jj++ )
    {  // Load each selected distribution model
        load_distro( loadedmodels[ jj ], mdescs[ jj ]);
    }

    // Sort points and build normalized concentrations

    qSort( sedcoes );
    qSort( lambdas );
    nsedcos       = sedcoes.count();
    nlambda       = lambdas.count();
    nipoint       = mdlxyz .count();
    DbgLv(1) << "LD: nlambda" << nlambda << "nsedcos" << nsedcos
             << "nipoint" << nipoint;
    nc_max        = 0;
    cn_max        = 0.0;
    double scalen = 1.0 / (double)loadedmodels.count();  // Normalizing scale factor

    for ( int jj = 0; jj < nsedcos; jj++ )
    {
        double sedco  = sedcoes[ jj ];
        DbgLv(1) << "LD:  jj" << jj << "sedco" << sedco;

        for ( int ii = 0; ii < nlambda; ii++ )
        {
            double waveln = (double)lambdas[ ii ];
            DbgLv(1) << "LD:    ii" << ii << "waveln" << waveln;
            double conc   = 0.0;
            double csum   = 0.0;
            int nconcs    = 0;

            for ( int kk = 0; kk < nipoint; kk++ )
            {
                if(kk<2||(kk+3)>nipoint)
                    DbgLv(1) << "LD:      kk" << kk << "X,Y" << mdlxyz[kk].x() << mdlxyz[kk].y();
                if ( dvirt_equal( mdlxyz[ kk ].y(), sedco )  &&
                     dvirt_equal( mdlxyz[ kk ].x(), waveln ) )
                {
                    nconcs++;
                    csum          += mdlxyz[ kk ].z();
                }
            }

            if ( nconcs > 0 )
            {  // Normalize concentration by dividing sum by number of models
                conc          = csum * scalen;
                nc_max        = qMax( nc_max, nconcs );
                cn_max        = qMax( cn_max, conc );
                nnpoint++;
                if(nconcs>5||conc>1000.0)
                    DbgLv(1) << "LD: **nc_max cn_max" << nc_max << cn_max << "nconcs conc"
                             << nconcs << conc << "jj,ii" << jj << ii << "s wl" << sedco << waveln;
            }

            else
            {
                conc           = 0.0;
            }

            xyzdat << QVector3D( waveln, sedco, conc );
            DbgLv(1) << "LD:     nconcs" << nconcs;
        }
    }

    ntpoint       = xyzdat  .count();
    DbgLv(1) << "LD: nipoint" << nipoint << "nnpoint" << nnpoint << "ntpoint"
             << ntpoint << "nc_max" << nc_max << "cn_max" << cn_max;
    int kdx       = 0;

    // Create the 2-D concentration vectors for each sed.coeff.
    for ( int jj = 0; jj < nsedcos; jj++ )
    {
        QVector< double > cvect;

        for ( int ii = 0; ii < nlambda; ii++ )
        {
            cvect << xyzdat[ kdx++ ].z();
        }

        // Save the concentration vector for lambdas of this sedcoeff
        concdat << cvect;
    }
    DbgLv(1) << "LD: concdat size" << concdat.size() << nsedcos;
    nlambda     = lambdas .count();
    nsedcos     = sedcoes .count();
    ntpoint     = nlambda * nsedcos;
    QStringList slscos;
    QStringList sllmbs;

    for ( int jj = 0; jj < nsedcos; jj++ )
        slscos << QString::number( sedcoes[ jj ] );

    for ( int jj = 0; jj < nlambda; jj++ )
        sllmbs << QString::number( lambdas[ jj ] );

    connect_ranges( false );
    cb_sstart ->clear();
    cb_send   ->clear();
    cb_lstart ->clear();
    cb_lend   ->clear();
    cb_pltrec ->clear();
    lb_pltrec->setText( tr( "%1:" ).arg(xlegend) );
    cb_sstart ->addItems( slscos );
    cb_send   ->addItems( slscos );
    cb_lstart ->addItems( sllmbs );
    cb_lend   ->addItems( sllmbs );
    cb_pltrec ->addItems( slscos );

    cb_sstart ->setCurrentIndex( 0 );
    cb_send   ->setCurrentIndex( nsedcos - 1 );
    cb_lstart ->setCurrentIndex( 0 );
    cb_lend   ->setCurrentIndex( nlambda - 1 );
    connect_ranges( true );

    have_rngs    = false;
    compute_ranges( );

    // Force a plot initialize
    cb_pltrec ->setCurrentIndex( nlambda / 2 );
    qApp->processEvents();
}
