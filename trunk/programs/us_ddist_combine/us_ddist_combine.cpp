//! \file us_ddist_combine.cpp

#include <QApplication>

#include "us_ddist_combine.h"
#include "us_select_runid.h"
#include "us_license_t.h"
#include "us_license.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_gui_util.h"
#include "us_matrix.h"
#include "us_constants.h"
#include "us_passwd.h"
#include "us_report.h"
#include "us_util.h"
#include "us_model.h"
#include "qwt_legend.h"

// Main program
int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );

   #include "main1.inc"

   // License is OK.  Start up.
   
   US_DDistr_Combine w;
   w.show();                   //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication
}

// US_DDistr_Combine class constructor
US_DDistr_Combine::US_DDistr_Combine() : US_Widgets()
{
   // set up the GUI
   setWindowTitle( tr( "Combined Discrete Distributions:" ) );
   setPalette( US_GuiSettings::frameColor() );
   dbg_level     = US_Settings::us_debug();
   xtype         = 0;

   QBoxLayout*  mainLayout   = new QHBoxLayout( this );
   QGridLayout* leftLayout   = new QGridLayout;
   QVBoxLayout* rightLayout  = new QVBoxLayout;
   mainLayout ->setSpacing        ( 2 );
   mainLayout ->setContentsMargins( 2, 2, 2, 2 );
   leftLayout ->setSpacing        ( 0 );
   leftLayout ->setContentsMargins( 0, 1, 0, 1 );
   rightLayout->setSpacing        ( 0 );
   rightLayout->setContentsMargins( 0, 1, 0, 1 );

   dkdb_cntrls             = new US_Disk_DB_Controls(
         US_Settings::default_data_location() );
   QPushButton* pb_loadda  = us_pushbutton( tr( "Select Run ID(s)"  ) );
                pb_saveda  = us_pushbutton( tr( "Save Data"  ) );
                pb_resetd  = us_pushbutton( tr( "Reset Data" ) );
                pb_resetp  = us_pushbutton( tr( "Reset Plot" ) );
   QPushButton* pb_help    = us_pushbutton( tr( "Help"       ) );
   QPushButton* pb_close   = us_pushbutton( tr( "Close"      ) );

   pb_saveda->setEnabled( false );
   pb_resetd->setEnabled( false );
   pb_resetp->setEnabled( false );

   QLabel* lb_distrtype  = us_banner( tr( "Select Distribution Type(s):" ) );
   QLabel* lb_plottype   = us_banner( tr( "Select Plot Type:" ) );
   QLabel* lb_runinfo    = us_banner( tr( "Information for this Run:" ) );
   QLabel* lb_runid      = us_label ( tr( "Current Run ID:" ) );
   QLabel* lb_svproj     = us_label ( tr( "Save Plot under Project:" ) );
   QLabel* lb_runids     = us_banner( tr( "Run IDs:" ) );
   QLabel* lb_models     = us_banner( tr( "Distributions:" ) );

   QLayout* lo_2dsa     = us_checkbox( tr( "2DSA" ),       ck_2dsa,     true  );
   QLayout* lo_2dsamc   = us_checkbox( tr( "2DSA-MC" ),    ck_2dsamc,   false );
   QLayout* lo_2dsamw   = us_checkbox( tr( "2DSA-MW" ),    ck_2dsamw,   false );
   QLayout* lo_2dsamcmw = us_checkbox( tr( "2DSA-MC-MW" ), ck_2dsamcmw, false );
   QLayout* lo_ga       = us_checkbox( tr( "GA" ),         ck_ga,       false );
   QLayout* lo_gamc     = us_checkbox( tr( "GA-MC" ),      ck_gamc,     false );
   QLayout* lo_gamw     = us_checkbox( tr( "GA-MW" ),      ck_gamw,     false );
   QLayout* lo_gamcmw   = us_checkbox( tr( "GA-MC-MW" ),   ck_gamcmw,   false );
   QLayout* lo_2dsafm   = us_checkbox( tr( "2DSA-FM" ),    ck_2dsafm,   false );
   QLayout* lo_2dsagl   = us_checkbox( tr( "2DSA-GL" ),    ck_2dsagl,   false );
   QLayout* lo_gagl     = us_checkbox( tr( "GA-GL" ),      ck_gagl,     false );
   QLayout* lo_dtall    = us_checkbox( tr( "All" ),        ck_dtall,    false );

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

   le_runid      = us_lineedit( "(current run ID)", -1, true );
   cmb_svproj    = us_comboBox();
   lw_runids     = us_listwidget();
   lw_models     = us_listwidget();
   te_status     = us_textedit();
   us_setReadOnly( te_status, true );
   te_status->setTextColor( Qt::blue );

   int row = 0;
   leftLayout->addLayout( dkdb_cntrls,  row++, 0, 1, 8 );
   leftLayout->addWidget( pb_loadda,    row,   0, 1, 4 );
   leftLayout->addWidget( pb_saveda,    row++, 4, 1, 4 );
   leftLayout->addWidget( pb_resetd,    row,   0, 1, 4 );
   leftLayout->addWidget( pb_resetp,    row++, 4, 1, 4 );
   leftLayout->addWidget( pb_help,      row,   4, 1, 2 );
   leftLayout->addWidget( pb_close,     row++, 6, 1, 2 );
   leftLayout->addWidget( lb_distrtype, row++, 0, 1, 8 );
   leftLayout->addLayout( lo_2dsa,      row,   0, 1, 2 );
   leftLayout->addLayout( lo_2dsamc,    row,   2, 1, 2 );
   leftLayout->addLayout( lo_2dsamw,    row,   4, 1, 2 );
   leftLayout->addLayout( lo_2dsamcmw,  row++, 6, 1, 2 );
   leftLayout->addLayout( lo_ga,        row,   0, 1, 2 );
   leftLayout->addLayout( lo_gamc,      row,   2, 1, 2 );
   leftLayout->addLayout( lo_gamw,      row,   4, 1, 2 );
   leftLayout->addLayout( lo_gamcmw,    row++, 6, 1, 2 );
   leftLayout->addLayout( lo_2dsafm,    row,   0, 1, 2 );
   leftLayout->addLayout( lo_2dsagl,    row,   2, 1, 2 );
   leftLayout->addLayout( lo_gagl,      row,   4, 1, 2 );
   leftLayout->addLayout( lo_dtall,     row++, 6, 1, 6 );
   leftLayout->addWidget( lb_plottype,  row++, 0, 1, 8 );
   leftLayout->addLayout( lo_pltsw,     row,   0, 1, 2 );
   leftLayout->addLayout( lo_pltMW,     row,   2, 1, 2 );
   leftLayout->addLayout( lo_pltDw,     row,   4, 1, 2 );
   leftLayout->addLayout( lo_pltff0,    row++, 6, 1, 2 );
   leftLayout->addLayout( lo_pltvb,     row,   0, 1, 8 );
   leftLayout->addLayout( lo_pltMWl,    row++, 2, 1, 8 );
   leftLayout->addWidget( lb_runinfo,   row++, 0, 1, 8 );
   leftLayout->addWidget( lb_runid,     row,   0, 1, 3 );
   leftLayout->addWidget( le_runid,     row++, 3, 1, 5 );
   leftLayout->addWidget( lb_svproj,    row,   0, 1, 3 );
   leftLayout->addWidget( cmb_svproj,   row++, 3, 1, 5 );
   leftLayout->addWidget( lb_runids,    row++, 0, 1, 8 );
   leftLayout->addWidget( lw_runids,    row,   0, 1, 8 );
   row    += 2;
   leftLayout->addWidget( lb_models,    row++, 0, 1, 8 );
   //leftLayout->setRowStretch( row, 0 );
   leftLayout->addWidget( lw_models,    row,   0, 5, 8 );
   row    += 5;
   leftLayout->setRowStretch( row, 1 );
   leftLayout->addWidget( te_status,    row++, 0, 1, 8 );

   connect( dkdb_cntrls, SIGNAL( changed( bool ) ),
            this,    SLOT( update_disk_db( bool ) ) );

   connect( pb_loadda, SIGNAL( clicked()    ),
            this,      SLOT(   load()       ) );
   connect( pb_saveda, SIGNAL( clicked()    ),
            this,      SLOT(   save()       ) );
   connect( pb_resetd, SIGNAL( clicked()    ),
            this,      SLOT(   reset_data() ) );
   connect( pb_resetp, SIGNAL( clicked()    ),
            this,      SLOT(   reset_plot() ) );
   connect( pb_help,   SIGNAL( clicked()    ),
            this,      SLOT(   help()       ) );
   connect( pb_close,  SIGNAL( clicked()    ),
            this,      SLOT(   close()      ) );

   connect( ck_2dsa,     SIGNAL( stateChanged    ( int ) ),
            this,        SLOT(   methodChanged   ( int ) ) );
   connect( ck_2dsamc,   SIGNAL( stateChanged    ( int ) ),
            this,        SLOT(   methodChanged   ( int ) ) );
   connect( ck_2dsamw,   SIGNAL( stateChanged    ( int ) ),
            this,        SLOT(   methodChanged   ( int ) ) );
   connect( ck_2dsamcmw, SIGNAL( stateChanged    ( int ) ),
            this,        SLOT(   methodChanged   ( int ) ) );
   connect( ck_2dsafm,   SIGNAL( stateChanged    ( int ) ),
            this,        SLOT(   methodChanged   ( int ) ) );
   connect( ck_ga,       SIGNAL( stateChanged    ( int ) ),
            this,        SLOT(   methodChanged   ( int ) ) );
   connect( ck_gamc,     SIGNAL( stateChanged    ( int ) ),
            this,        SLOT(   methodChanged   ( int ) ) );
   connect( ck_gamw,     SIGNAL( stateChanged    ( int ) ),
            this,        SLOT(   methodChanged   ( int ) ) );
   connect( ck_gamcmw,   SIGNAL( stateChanged    ( int ) ),
            this,        SLOT(   methodChanged   ( int ) ) );
   connect( ck_dtall,    SIGNAL( stateChanged    ( int ) ),
            this,        SLOT(   allMethodChanged( int ) ) );

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

   connect( lw_runids,   SIGNAL( currentRowChanged( int ) ),
            this,        SLOT(   runid_select(      int ) ) );
   connect( lw_models,   SIGNAL( currentRowChanged( int ) ),
            this,        SLOT(   model_select(      int ) ) );

   QBoxLayout* plot = new US_Plot( data_plot1,
         tr( "Discrete s20,W Distributions" ),
         tr( "Sedimentation Coefficient x 1e+13 (corr. for 20,W)" ),
         tr( "Relative Concentration" ) );

   data_plot1->setMinimumSize( 560, 400 );
   data_plot1->setAxisScale( QwtPlot::xBottom, 1.0,  10.0 );
   data_plot1->setAxisScale( QwtPlot::yLeft,   0.0, 100.0 );
   QwtPlotGrid* grid = us_grid( data_plot1 );
   grid->enableXMin( true );
   grid->enableYMin( true );
   grid->setMajPen( QPen( US_GuiSettings::plotMajGrid(), 0, Qt::DashLine ) );
   grid->setMinPen( QPen( US_GuiSettings::plotMinGrid(), 0, Qt::DotLine  ) );

   QwtLegend *legend = new QwtLegend;
   legend->setFrameStyle( QFrame::Box | QFrame::Sunken );
   data_plot1->insertLegend( legend, QwtPlot::BottomLegend  ); 

   rightLayout->addLayout( plot );

   mainLayout ->addLayout( leftLayout     );
   mainLayout ->addLayout( rightLayout    );
   mainLayout ->setStretchFactor( leftLayout,  2 );
   mainLayout ->setStretchFactor( rightLayout, 3 );

   le_runid   ->setText( "(current run ID)" );
   cmb_svproj ->addItem( "(project name for plot save)" );

   adjustSize();
   int hh  = lb_svproj->height();
   //int ww  = lb_svproj->width() / 3;
   int ww  = lb_svproj->width() / 6;
   lw_runids  ->setMinimumHeight( hh * 2 );
   lw_runids  ->setMaximumHeight( hh * 4 );
   lw_models  ->setMinimumHeight( hh * 5 );
   //cmb_svproj ->setMinimumWidth ( ww * 5 );
   cmb_svproj ->setMinimumWidth ( ww * 2 );
   for ( int ii = 0; ii < 8; ii++ )
      leftLayout ->setColumnMinimumWidth( ii, ww );
   leftLayout ->setColumnStretch     ( 0, 1  );
   leftLayout ->setColumnStretch     ( 1, 1  );
   te_status  ->setMaximumHeight( ( hh * 3 ) / 2 );

   adjustSize();
   resize( 1180, 580 );
   reset_data();
}

// Load data
void US_DDistr_Combine::load( void )
{
   QStringList runids;
   QString     runid;
   te_status->setText( tr( "Building list of selectable run IDs..." ) );
   qApp->processEvents();

   // Open a dialog and get the runID(s)
   US_SelectRunid srdiag( dkdb_cntrls->db(), runids, aDescrs );
   connect( &srdiag,      SIGNAL( changed( bool ) ),
            this,    SLOT( update_disk_db( bool ) ) );
   srdiag.exec();

   int nrunids = runids.count();
   int nsprojs = cmb_svproj->count();
   if ( nrunids < 1 )   return;

   update_distros();
//*DEBUG*
if(dbg_level>0)
{
 DbgLv(1) << "Selected runIDs[0]" << runids[0] << "count" << nrunids;
 DbgLv(1) << "Selected models  count" << aDescrs.count();
 for(int ii=0;ii<aDescrs.count();ii++)
 {
  QString mrun=aDescrs[ii].section("\t",0,0);
  QString mgid=aDescrs[ii].section("\t",1,1).left(9)+"...";
  QString mdes=aDescrs[ii].section("\t",2,2);
  bool iter=mdes.contains("-MC_0");
  if(iter&&!mdes.contains("_mc0001")) continue;
  mdes="..."+mdes.section(".",1,-1);
  DbgLv(1) << "  ii" << ii << "mrun" << mrun << "mgid" << mgid
   << "mdes" << mdes << "iter" << iter;
 }
}
//*DEBUG*
   te_status->setText( tr( "Added: %1 run(s), %2 implied models." )
         .arg( nrunids ).arg( aDescrs.count() ) );

   if ( nsprojs == 1 )
   {
      cmb_svproj->clear();
   }
   else
   {
      cmb_svproj->removeItem( nsprojs - 1 );
   }

   for ( int ii = 0; ii < nrunids; ii++ )
   {  // Add run IDs to list and to project combo box
      lw_runids->addItem( runids[ ii ] );
      cmb_svproj->addItem( runids[ ii ] );
   }

   cmb_svproj->addItem( "All" );
   le_runid->setText( runids[ 0 ] );
   pb_resetd->setEnabled( true );
   QStringList methods;

   for ( int ii = 0; ii < distros.count(); ii++ )
   {  // Build a list of unique methods of distros
      QString method = distros[ ii ].mdescr.section( ".", -1, -1 )
                                           .section( "_", -3, -3 );
      if ( ! methods.contains( method ) )
         methods << method;
   }

   bool hv_2dsa     = methods.contains( "2DSA"       );
   bool hv_2dsamc   = methods.contains( "2DSA-MC"    );
   bool hv_2dsamw   = methods.contains( "2DSA-MW"    );
   bool hv_2dsamcmw = methods.contains( "2DSA-MC-MW" );
   bool hv_ga       = methods.contains( "GA"         );
   bool hv_gamc     = methods.contains( "GA-MC"      );
   bool hv_gamw     = methods.contains( "GA-MW"      );
   bool hv_gamcmw   = methods.contains( "GA-MC-MW"   );
   bool hv_2dsafm   = methods.contains( "2DSA-FM"    );
   bool hv_2dsagl   = methods.contains( "2DSA-GL"    );
   bool hv_gagl     = methods.contains( "GA-GL"      );
   bool hv_dtall    = methods.size() > 0;

   ck_2dsa    ->setEnabled( hv_2dsa     );
   ck_2dsamc  ->setEnabled( hv_2dsamc   );
   ck_2dsamw  ->setEnabled( hv_2dsamw   );
   ck_2dsamcmw->setEnabled( hv_2dsamcmw );
   ck_ga      ->setEnabled( hv_ga       );
   ck_gamc    ->setEnabled( hv_gamc     );
   ck_gamw    ->setEnabled( hv_gamw     );
   ck_gamcmw  ->setEnabled( hv_gamcmw   );
   ck_2dsafm  ->setEnabled( hv_2dsafm   );
   ck_2dsagl  ->setEnabled( hv_2dsagl   );
   ck_gagl    ->setEnabled( hv_gagl     );
   ck_dtall   ->setEnabled( hv_dtall    );

   ck_2dsa    ->setChecked( hv_2dsa     );
   ck_2dsamc  ->setChecked( hv_2dsamc   );
   ck_2dsamw  ->setChecked( hv_2dsamw   );
   ck_2dsamcmw->setChecked( hv_2dsamcmw );
   ck_ga      ->setChecked( hv_ga       );
   ck_gamc    ->setChecked( hv_gamc     );
   ck_gamw    ->setChecked( hv_gamw     );
   ck_gamcmw  ->setChecked( hv_gamcmw   );
   ck_2dsafm  ->setChecked( hv_2dsafm   );
   ck_2dsagl  ->setChecked( hv_2dsagl   );
   ck_gagl    ->setChecked( hv_gagl     );
   ck_dtall   ->setChecked( hv_dtall    );
}

// Reset data: remove all loaded data and clear plots
void US_DDistr_Combine::reset_data( void )
{
   distros.clear();
   distIDs.clear();
   aDescrs.clear();

   lw_runids  ->clear();
   lw_models  ->clear();
   le_runid   ->clear();
   cmb_svproj ->clear();

   reset_plot();

   pb_resetd->setEnabled( false );
   pb_resetp->setEnabled( false );
DbgLv(1) << "main size" << size();
}

// Reset plot:  Clear plots and lists of plotted data
void US_DDistr_Combine::reset_plot( void )
{
   data_plot1->detachItems();
   data_plot1->clear();
   data_plot1->replot();

   pdistrs.clear();
   pdisIDs.clear();
   pb_saveda->setEnabled( false );

   lw_models  ->setCurrentRow( -1 );
}

// Plot all data
void US_DDistr_Combine::plot_data( void )
{
DbgLv(1) << "pDa:  xtype" << xtype;
   data_plot1->detachItems();
   data_grid = us_grid ( data_plot1 );
   data_grid->enableXMin( true );
   data_grid->enableYMin( true );
   data_grid->setMajPen(
         QPen( US_GuiSettings::plotMajGrid(), 0, Qt::DashLine ) );
   data_grid->setMinPen(
         QPen( US_GuiSettings::plotMinGrid(), 0, Qt::DotLine  ) );

   QString titleY = tr( "Relative Concentration" );
DbgLv(1) << "pDa:  titleY" << titleY;
   QString titleP;
   QString titleX;

   if      ( rb_pltsw->isChecked() )
   {
      titleP = tr( "Discrete s20,W Distributions" );
      titleX = tr( "Sedimentation Coefficient x 1.e+13 (20,W)" );
   }
   else if ( rb_pltMW->isChecked() )
   {
      titleP = tr( "Discrete Molecular Weight Distributions" );
      titleX = tr( "Molecular Weight (Dalton)" );
   }
   else if ( rb_pltDw->isChecked() )
   {
      titleP = tr( "Discrete D20,W Distributions" );
      titleX = tr( "Diffusion Coefficient (20,W)" );
   }
   else if ( rb_pltff0->isChecked() )
   {
      titleP = tr( "Discrete Frictional Ratio Distributions" );
      titleX = tr( "Frictional Ratio (f/f0)" );
   }
   else if ( rb_pltvb->isChecked() )
   {
      titleP = tr( "Discrete Vbar Distributions" );
      titleX = tr( "Vbar (Specific Density)" );
   }
   else if ( rb_pltMWl->isChecked() )
   {
      titleP = tr( "Discrete Log of Molecular Weight Distributions" );
      titleX = tr( "Molecular Weight (Log, Dalton)" );
   }
DbgLv(1) << "pDa:  titleP" << titleP;
DbgLv(1) << "pDa:  titleX" << titleX;
   data_plot1->setTitle    ( titleP );
   data_plot1->setAxisTitle( QwtPlot::xBottom, titleX );
   data_plot1->setAxisTitle( QwtPlot::yLeft,   titleY );

   for ( int ii = 0; ii < pdistrs.size(); ii++ )
      plot_distr( pdistrs[ ii ], pdisIDs[ ii ] );
}

// Add a single distribution to the plot
void US_DDistr_Combine::plot_distr( DistrDesc ddesc, QString distrID )
{
   int  ndispt = ddesc.xvals.size();
   double* xx  = ddesc.xvals.data();
   double* yy  = ddesc.yvals.data();
DbgLv(1) << "pDi:  ndispt" << ndispt << "ID" << distrID.left(20);

   QwtPlotCurve* data_curv = us_curve( data_plot1, distrID );

   data_curv->setData ( xx, yy, ndispt );
   data_curv->setPen  ( QPen( QBrush( ddesc.color ), 3.0, Qt::SolidLine ) );
   data_curv->setStyle( QwtPlotCurve::Sticks );
   data_curv->setItemAttribute( QwtPlotItem::Legend, true );

   data_plot1->setAxisAutoScale( QwtPlot::xBottom );
   data_plot1->setAxisAutoScale( QwtPlot::yLeft );
   data_plot1->enableAxis      ( QwtPlot::xBottom, true );
   data_plot1->enableAxis      ( QwtPlot::yLeft,   true );

   if ( pdistrs.size() < 2 )
   {  // Set up grid if first distribution in plot
      data_grid = us_grid ( data_plot1 );
      data_grid->enableXMin( true );
      data_grid->enableYMin( true );
      data_grid->setMajPen(
            QPen( US_GuiSettings::plotMajGrid(), 0, Qt::DashLine ) );
      data_grid->setMinPen(
            QPen( US_GuiSettings::plotMinGrid(), 0, Qt::DotLine  ) );
   }

   data_plot1->replot();
}

// Save the plot data
void US_DDistr_Combine::save( void )
{
   QString oproj    = cmb_svproj->currentText();
   QString runID    = ( oproj == "All" ) ? pdistrs[ 0 ].runID : oproj;
   QString fdir     = US_Settings::reportDir() + "/" + runID;
   QString mdescr   = pdistrs[ 0 ].mdescr;
   QString annode   = mdescr.section( ".", -1, -1 ).section( "_", -3, -3 );
   QString trnode   = "0Z9999";
   QString sanode1  = "combo-distrib-s";
   QString sanode2  = "combo-vcdat-s";
   QString sanode3  = "combo-listincl-s";

   if ( xtype == 1 )
   {
      sanode1       = "combo-distrib-mw";
      sanode2       = "combo-vcdat-mw";
      sanode3       = "combo-listincl-mw";
   }
   else if ( xtype == 2 )
   {
      sanode1       = "combo-distrib-d";
      sanode2       = "combo-vcdat-d";
      sanode3       = "combo-listincl-d";
   }
   else if ( xtype == 3 )
   {
      sanode1       = "combo-distrib-ff0";
      sanode2       = "combo-vcdat-ff0";
      sanode3       = "combo-listincl-ff0";
   }
   else if ( xtype == 4 )
   {
      sanode1       = "combo-distrib-vbar";
      sanode2       = "combo-vcdat-vbar";
      sanode3       = "combo-listincl-vbar";
   }
   else if ( xtype == 5 )
   {
      sanode1       = "combo-distrib-mwl";
      sanode2       = "combo-vcdat-mwl";
      sanode3       = "combo-listincl-mwl";
   }
   QString fnamsvg  = annode + "." + trnode + "." + sanode1 + ".svg"; 
   QString fnampng  = annode + "." + trnode + "." + sanode1 + ".png"; 
   QString fnamdat  = annode + "." + trnode + "." + sanode2 + ".dat"; 
   QString fnamlst  = annode + "." + trnode + "." + sanode3 + ".rpt"; 
   QString plotFile = fdir + "/" + fnamsvg;
   QString dataFile = fdir + "/" + fnamdat;
   QString listFile = fdir + "/" + fnamlst;
   QStringList prunids;
   QList< int > prndxs;
   QString svmsg   = tr( "Saved:\n    " ) + fnampng + "\n    "
                                          + fnamsvg + "\n    "
                                          + fnamdat + "\n    "
                                          + fnamlst + "\n";
   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );

   // Look for multiple run IDs
   for ( int ii = 0; ii < pdistrs.size(); ii++ )
   {
      QString prun     = pdistrs[ ii ].runID;
      if ( oproj == "All" )
      {  // If save-plot project is "All", save lists of runids and indexes
         if ( ! prunids.contains( prun ) )
         {
            prunids << prun;
            prndxs  << ii;
         }
      }

      else if ( prun == runID )
      {  // If save-plot project matches current run, save it and its index
         prunids << prun;
         prndxs  << ii;
         break;
      }
   }

   int     iruns   = 0;
   int     nruns   = prunids.size();

   while( iruns < nruns )
   {
      if ( ! QFile( fdir ).exists() )
      {  // If need be, create runID directory
         QDir().mkpath( fdir );
      }

      // Save plot file as SVG and as PNG; write data and list files
      write_plot( plotFile, data_plot1 );
      write_data( dataFile, listFile, iruns );
      svmsg += tr( "in directory:" ) + "\n    " + fdir + "\n";

      if ( dkdb_cntrls->db() )
      {
         US_Passwd    pw;
         US_DB2       db( pw.getPasswd() );
         int          idEdit = 0;
         int          kl     = pdistrs.size() - 1;

         QString trfirst  = pdistrs[ 0  ].mdescr.section( ".", -2, -2 );
         QString trlast   = pdistrs[ kl ].mdescr.section( ".", -2, -2 );
         QString trdesc   = "Combined Analyses (" + trfirst
            + "..." + trlast + ")";

         QString editID;         // Edit ID for matching experiment,triple
         QString eeditID;        // First edit ID from experiment match
         // Get test triple to match file part and investigator
         QString trip1 = "";
         QString invID = QString::number( US_Settings::us_inv_ID() );
         // Query for the experiment ID matching the run ID
         QStringList  query;
         query << "get_experiment_info_by_runID" << runID << invID;
         db.query( query );
         db.next();
         QString expID = db.value( 1 ).toString();
DbgLv(1) << "SV:  runID expID" << runID << expID;
         // Query for the raw ID in experiment matching a triple
         QString rawID;
         query.clear();
         query << "get_rawDataIDs" << expID;
         db.query( query );
         while ( db.next() )
         {
                    rawID  = db.value( 0 ).toString();
            QString efname = db.value( 2 ).toString();
DbgLv(1) << "SV:   rawID" << rawID << "efname" << efname << "trip1" << trip1;
            // Save rawID when we have found a triple match
            if ( efname.contains( trip1 ) )
               break;
         }
         // Query edit IDs for raw ID and look for triple match
         query.clear();
         query << "get_editedDataIDs" << rawID;
         db.query( query );
         while ( db.next() )
         {
            QString aeditID = db.value( 0 ).toString();
            QString efname  = db.value( 2 ).toString();
            if ( eeditID.isEmpty() )
               eeditID      = aeditID;  // Save 1st valid from experiment
DbgLv(1) << "SV:     editID" << eeditID << "raw exp trip1 fname"
 << rawID << expID << trip1 << efname;
            if ( efname.contains( trip1 ) )
            {  // Keep saving editID from last triple match
               editID       = aeditID;
DbgLv(1) << "SV:        Name-Trip MATCH:  editID" << editID;
            }
         }

         if ( ! editID.isEmpty() )
         {  // Use edit ID from last matching triple
            idEdit              = editID.toInt();
         }
         else
         {  // Or fall back to one from first valid edit in experiment
            idEdit              = eeditID.toInt();
         }
DbgLv(1) << "SV: editID idEdit" << editID << idEdit << "  eeditID" << eeditID;

         // Add or update report documents in the database
         QStringList rfiles;
         rfiles << fnamsvg << fnampng << fnamdat << fnamlst;
         int st = reportDocsFromFiles( runID, fdir, rfiles, &db,
                                       idEdit, trdesc );

DbgLv(1) << "SV:runID" << runID << "idEdit" << idEdit
 << "fnamlst" << fnamlst << "trdesc" << trdesc;
         if ( iruns == ( nruns - 1 ) )
         {  // Append message line after last run save
            if ( st == 0 )
               svmsg += tr( "\nThe files were also saved to the database" );
            else
               svmsg += tr( "\n*ERROR* in saving files to the database" );
         }
      }  // END:  database

      if ( ++iruns >= nruns )  break;

      runID         = prunids[ iruns ];
      fdir          = US_Settings::reportDir() + "/" + runID;
      plotFile      = fdir + "/" + fnamsvg;
      dataFile      = fdir + "/" + fnamdat;
      listFile      = fdir + "/" + fnamlst;
   }  // END:  runs loop

   QApplication::restoreOverrideCursor();

   // Report saved files
   QMessageBox::information( this, tr( "Combo Distro Plot File Save" ), svmsg );
}

// RunID selected
void US_DDistr_Combine::runid_select( int row )
{
DbgLv(1) << "RunIDSel:row" << row;
   if ( row < 0 )  return;

   // Get selected run item and its ID
   QListWidgetItem* item = lw_runids->item( row );
   runID    = item->text();
DbgLv(1) << "RunIDSel:runID" << runID << "distrsize" << distros.size();
   le_runid ->setText( runID );

   // Check for filtering by method
   bool mfilter = ! ck_dtall->isChecked();
   QStringList methods;

   if ( mfilter )
   {
      if ( ck_2dsa    ->isChecked() )  methods << "2DSA";
      if ( ck_2dsamc  ->isChecked() )  methods << "2DSA-MC";
      if ( ck_2dsamw  ->isChecked() )  methods << "2DSA-MW";
      if ( ck_2dsamcmw->isChecked() )  methods << "2DSA-MC-MW";
      if ( ck_ga      ->isChecked() )  methods << "GA";
      if ( ck_gamc    ->isChecked() )  methods << "GA-MC";
      if ( ck_gamw    ->isChecked() )  methods << "GA-MW";
      if ( ck_gamcmw  ->isChecked() )  methods << "GA-MC-MW";
      if ( ck_2dsafm  ->isChecked() )  methods << "2DSA-FM";
      if ( ck_2dsagl  ->isChecked() )  methods << "2DSA-GL";
      if ( ck_gagl    ->isChecked() )  methods << "GA-GL";
   }

   lw_models ->clear();

   for ( int ii = 0; ii < distros.size(); ii++ )
   {
DbgLv(1) << "RunIDSel:  ii runID" << ii << distros[ii].runID;
      if ( distros[ ii ].runID == runID )
      {  // Only (possibly) add item with matching run ID
         QString mdesc = distros[ ii ].mdescr;

         if ( mfilter )
         {  // If method-filtering, skip any item whose method is not checked
            QString meth = mdesc.section( ".", -1, -1 ).section( "_", -3, -3 );
            if ( ! methods.contains( meth ) )  continue;
         }

         lw_models ->addItem( distribID( mdesc ) );
      }
   }

   if ( pdistrs.size() == 0 )
   {
      cmb_svproj->setCurrentIndex( cmb_svproj->findText( runID ) );
   }
}

// Model distribution selected
void US_DDistr_Combine::model_select( int row )
{
DbgLv(1) << "ModelSel:row" << row;
   if ( row < 0 )  return;
   QListWidgetItem* item    = lw_models ->item( row );
   QString          distrID = item->text();
   int              mdx     = distro_by_descr( distrID );
DbgLv(1) << "ModelSel: model" << distrID << "mdx" << mdx;
   DistrDesc*       ddesc   = &distros[ mdx ];

   if ( ! pdisIDs.contains( distrID ) )
   {  // If this distro not yet filled out, do so now

      fill_in_desc( distros[ mdx ], pdistrs.size() );

      pdistrs << *ddesc;     // Add to list of plotted distros
      pdisIDs << distrID;    // Add to list of IDs of plotted distros
   }

   plot_data();

   pb_saveda->setEnabled( true );
   pb_resetd->setEnabled( true );
   pb_resetp->setEnabled( true );

   te_status->setText( tr( "Count of plotted distributions: %1." )
         .arg( pdistrs.count() ) );
}

// Assign color for a distribution
void US_DDistr_Combine::setColor( DistrDesc& ddesc, int distx )
{
   possibleColors();              // Make sure possible colors exist

   int ncolors     = colors.size();
   int color_index = distx;

   while ( color_index >= ncolors )
      color_index -= ncolors;

DbgLv(1) << "sC:  color_index" << color_index;
   ddesc.color = colors[ color_index ];
   return;
}

// Generate list of colors if need be
void US_DDistr_Combine::possibleColors()
{
   if ( colors.size() > 0 )
      return;

   colors  << QColor( 255,   0,   0 );
   colors  << QColor(   0, 255,   0 );
   colors  << QColor(   0,   0, 255 );
   colors  << QColor( 255, 255,   0 );
   colors  << QColor( 255,   0, 255 );
   colors  << QColor(   0, 255, 255 );
   colors  << QColor( 122,   0, 255 );
   colors  << QColor(   0, 255, 122 );
   colors  << QColor(   0, 122, 255 );
   colors  << QColor( 255, 122,   0 );
   colors  << QColor( 122, 255,   0 );
   colors  << QColor(  80,   0, 255 );
   colors  << QColor( 255,   0,  80 );
   colors  << QColor(  80,   0, 255 );
   colors  << QColor( 255,   0,  80 );
   colors  << QColor(   0, 255,  80 );
   colors  << QColor(   0,  80, 255 );
   colors  << QColor(  80, 255,   0 );
   colors  << QColor( 255,  80,  40 );
   colors  << QColor(  40, 255,  40 );
   colors  << QColor(  40,  40, 255 );
   return;
}

// Return a distribution ID string that is a shortened model description
QString US_DDistr_Combine::distribID( QString mdescr )
{
   QString runID   = mdescr.section( ".",  0, -3 );
           runID   = runID.length() < 13 ? runID : runID.left( 12 ) + "(++)";
   QString triple  = mdescr.section( ".", -2, -2 );
   QString iterID  = mdescr.section( ".", -1, -1 );
   QString andate  = iterID.section( "_",  1,  1 );
   QString method  = iterID.section( "_",  2,  2 );
           iterID  = ( method != "2DSA-FM" )
                   ? iterID.section( "_", -2, -2 )
                   : iterID.section( "_", -1, -1 );
   QString distrID = runID + "." + triple + "." + andate
                           + "_" + method + "_" + iterID;

   return distrID;
}

// Reset Disk_DB control whenever data source is changed in any dialog
void US_DDistr_Combine::update_disk_db( bool isDB )
{
   isDB ? dkdb_cntrls->set_db() : dkdb_cntrls->set_disk();
DbgLv(1) << "Upd_Dk_Db isDB" << isDB;

   reset_data();
}

// Fill in a distribution description object with model and values
void US_DDistr_Combine::fill_in_desc( DistrDesc& ddesc, int distx )
{
   if ( ddesc.xvals.size() > 0  &&
        ddesc.model.components.size() > 0  &&
        ddesc.xtype == xtype )
      return;

   // The distribution record is at least partially uninitialized
   QVector< double > mxvals;
   QVector< double > myvals;
   QString mdescr = ddesc.mdescr;
   ddesc.iters    = mdescr.contains( "-MC_0" ) ? 1 : 0;
DbgLv(1) << "FID: mdescr" << mdescr << "iters" << ddesc.iters;
   bool    isDB   = dkdb_cntrls->db();
   US_Passwd pw;
   US_DB2* db     = isDB ? new US_DB2( pw.getPasswd() ) : 0;
   int     ncomps = ddesc.model.components.size();
DbgLv(1) << "FID:  ncomps" << ncomps;

   // Read in the (first) model
   if ( ncomps == 0 )
   {  // Model not yet loaded, so load it now
      ddesc.model.load( isDB, ddesc.mGUID, db );
      int niters = 1;

      if ( ddesc.iters > 0 )
      {  // An MC model, so concatenate the remaining MC iteration models
         QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
         te_status->setText(
               tr( "Building a composite of Monte Carlo distributions..." ) );
         qApp->processEvents();
         US_Model model2;
         int     kk     = mdescr.lastIndexOf( "_mc0001" );
         QString pdescr = mdescr.left( kk );  // Description prefix

         for ( int jj = 0; jj < aDescrs.count(); jj++ )
         {
            QString aGUID  = aDescrs[ jj ].section( "\t", 1, 1 );
            QString adescr = aDescrs[ jj ].section( "\t", 2, 2 );

            // Test for the same description prefix, but not iteration 1
            if ( adescr.startsWith( pdescr )  &&  ddesc.mGUID != aGUID )
            {  // We've found a model that is an iteration:  add it in
               model2.load( isDB, aGUID, db );
               ddesc.model.components << model2.components;
               niters++;
            }
         }
         QApplication::restoreOverrideCursor();
      }

      ncomps = ddesc.model.components.size();     // Composite components
DbgLv(1) << "FID:  (2)ncomps" << ncomps;

      // Scale the composite components
      scale_montecarlo( ddesc.model, niters );
      ncomps = ddesc.model.components.size();     // Adjusted components

   }
DbgLv(1) << "FID:  (3)ncomps" << ncomps;

   // Build the X,Y vectors with values at every component point
   for ( int jj = 0; jj < ncomps; jj++ )
   {
      myvals << ddesc.model.components[ jj ].signal_concentration;
      double                  xval = ddesc.model.components[ jj ].s * 1.e+13;
      if ( xtype == 1 )       xval = ddesc.model.components[ jj ].mw;
      else if ( xtype == 2 )  xval = ddesc.model.components[ jj ].D;
      else if ( xtype == 3 )  xval = ddesc.model.components[ jj ].f_f0;
      else if ( xtype == 4 )  xval = ddesc.model.components[ jj ].vbar20;
      else if ( xtype == 5 )  xval = log( ddesc.model.components[ jj ].mw );

      mxvals << xval;
   }

   ddesc.xtype     = xtype;
DbgLv(1) << "FID:   xtype" << xtype << "mxval.size" << mxvals.size();

   // Now build the plot vectors:  composite values at every 2% point
   double xmin     = mxvals[ 0 ];
   double xmax     = xmin;

   for ( int jj = 1; jj < ncomps; jj++ )
   {  // Scan for X limits
      xmin = qMin( xmin, mxvals[ jj ] );
      xmax = qMax( xmax, mxvals[ jj ] );
   }

   double xinc  = ( xmax - xmin ) / 49.0;
          xmin -= ( xinc * 0.01 );
          xmax += ( xinc * 0.01 );
          xinc  = ( xmax - xmin ) / 49.0;
   double xval  = xmin;
DbgLv(1) << "FID:    xmin xmax xinc" << xmin << xmax << xinc;
   ddesc.xvals.fill( 0.0, 50 );
   ddesc.yvals.fill( 0.0, 50 );

   for ( int jj = 0; jj < 50; jj++ )
   {  // Fill in the X values at each 2% point
DbgLv(1) << "FID:      X-fill jj xval" << jj << xval;
      ddesc.xvals[ jj ]  = xval;
      xval              += xinc;
   }

   for ( int jj = 0; jj < ncomps; jj++ )
   {  // Sum in Y values around each X point
      xval               = mxvals[ jj ];
      int kk             = qRound( ( xval - xmin ) / xinc );
DbgLv(1) << "FID:      jj xval kk" << jj << xval << kk;
      double yval        = myvals[ jj ];
      ddesc.yvals[ kk ] += yval;
   }
int kk = ddesc.xvals.size()-1;
DbgLv(1) << "FID:Distro runid" << ddesc.runID;
DbgLv(1) << "FID:  0 X Y" << ddesc.xvals[0] << ddesc.yvals[0];
DbgLv(1) << "FID:  kk X Y" << ddesc.xvals[kk] << ddesc.yvals[kk];

   setColor( ddesc, distx );
}

// Write data and list report files
void US_DDistr_Combine::write_data( QString& dataFile, QString& listFile,
      int& irun )
{
   if ( irun > 0 )
   {  // After first/only time:  just make a copy of the files
      QFile( dat1File ).copy( dataFile );
      QFile( lis1File ).copy( listFile );
      return;
   }

   // First/only time through:  compute the data and create files
   QStringList pdlong;
   QString line;
   dat1File = dataFile;
   lis1File = listFile;

   QFile dfile( dataFile );

   if ( ! dfile.open( QIODevice::WriteOnly | QIODevice::Text ) )
   {
      qDebug() << "***Error opening output file" << dataFile;
      return;
   }

   QTextStream tsd( &dfile );

   int nplots = pdistrs.size();
   int maxnvl = 0;
   line       = "";
      
   for ( int ii = 0; ii < nplots; ii++ )
   {  // Accumulate long descriptions and build header line
      maxnvl     = qMax( maxnvl, pdistrs[ ii ].xvals.size() );
      QString pd = pdisIDs[ ii ];

      pdlong << pdistrs[ ii ].mdescr;

DbgLv(1) << "WrDa:  plot" << ii << "pd" << pd;
      line      += pd + ".X " + pd + ".Y"; // X,Y header entries for contributor
      if ( ii < ( nplots - 1 ) )
         line     += "  ";
      else
         line     += "\n";
   }
   tsd << line;                             // Write header line

DbgLv(1) << "WrDa: maxnvl" << maxnvl << "nplots" << nplots;
   char  valfm1[] = "%12.5f %10.5f";
   char  valfm2[] = "%13.4e %9.5f";
   char* valfmt   = valfm1;
   if ( xtype == 1  ||  xtype == 2  || xtype == 5 )
         valfmt   = valfm2;                // Formatting for "MW"/"D"/"MWlog"

   for ( int jj = 0; jj < maxnvl; jj++ )
   {  // Build and write xvalue+concentration data line
      line       = "";
      for ( int ii = 0; ii < nplots; ii++ )
      {  // Add each X,Y data pair
         int nvals   = pdistrs[ ii ].xvals.size();
         double* xx  = pdistrs[ ii ].xvals.data();
         double* yy  = pdistrs[ ii ].yvals.data();
         int kk      = qMin( jj, ( nvals - 1 ) );
         double xval = xx[ kk ];
         double yval = yy[ kk ];

         line       += QString().sprintf( valfmt, xval, yval );
      }
      line       += "\n";
      tsd << line;                           // Write data line
//DbgLv(1) << "WrDa:   jj" << jj << " line written";
   }

   dfile.close();

   // Write list-of-included file
   QFile lfile( listFile );
   if ( ! lfile.open( QIODevice::WriteOnly | QIODevice::Text ) )
   {
      qDebug() << "***Error opening output file" << listFile;
      return;
   }
   QTextStream tsl( &lfile );

   for ( int ii = 0; ii < nplots; ii++ )
   {  // Build and write each long-description line
      line       = pdlong[ ii ] + "\n";
      tsl << line;
   }

   lfile.close();

   return;
}

// Save report documents from files
int US_DDistr_Combine::reportDocsFromFiles( QString& runID, QString& fdir,
   QStringList& files, US_DB2* db, int& idEdit, QString& trdesc )
{
 DbgLv(1) << "rDFF: runID fdir files0" << runID << fdir << files[0];
 DbgLv(1) << "rDFF:  idEdit trdesc" << idEdit << trdesc;
   int ostat      = 0;
   US_Report    freport;
   freport.runID  = runID;

   for ( int ii = 0; ii < files.size(); ii++ )
   {
      QString fname = files[ ii ];
      int st = freport.saveDocumentFromFile( fdir, fname, db, idEdit, trdesc );

      ostat = ( st == US_Report::REPORT_OK ) ? ostat : st;
   }

//*DEBUG*
   if ( dbg_level > 0 )
   {
      int status = freport.readDB( runID, db );
      DbgLv(1) << "DFF:report readDB status" << status << "ID" << freport.ID;
      DbgLv(1) << "DFF:  report triples size" << freport.triples.size();
      for ( int ii = 0; ii < freport.triples.size(); ii++ )
      {
         int ndoc = freport.triples[ii].docs.size();
         DbgLv(1) << "DFF:  triple" << ii << "docssize" << ndoc
            << "ID" << freport.triples[ii].tripleID
            << "triple" << freport.triples[ii].triple;
         int jj   = ndoc - 1;
         if ( ndoc > 0 )
         {
            DbgLv(1) << "DFF:    doc" << 0
               << "ID" << freport.triples[ii].docs[0].documentID
               << "label" << freport.triples[ii].docs[0].label;
            DbgLv(1) << "DFF:    doc" << jj
               << "ID" << freport.triples[ii].docs[jj].documentID
               << "label" << freport.triples[ii].docs[jj].label;
         }
      }
      QString fname = files[0];
      QString tripl( "0/Z/9999" );
      int ndx = freport.findTriple( tripl );
      DbgLv(1) << "DFF:triple" << tripl << "ndx" << ndx;
      if ( ndx >= 0 )
      {
         int ndoc = freport.triples[ndx].docs.size();
         DbgLv(1) << "DFF:  triple" << ndx << "docs size" << ndoc
            << "ID" << freport.triples[ndx].tripleID
            << "triple" << freport.triples[ndx].triple;
         if ( ndoc > 0 )
         {
            DbgLv(1) << "DFF:    doc" << 0
               << "ID" << freport.triples[ndx].docs[0].documentID
               << "label" << freport.triples[ndx].docs[0].label;
            int jj   = ndoc - 1;
            DbgLv(1) << "DFF:    doc" << jj
               << "ID" << freport.triples[ndx].docs[jj].documentID
               << "label" << freport.triples[ndx].docs[jj].label;
         }
      }
   }
//*DEBUG*

   return ostat;
}

// Get the index to a distro using the model description
int US_DDistr_Combine::distro_by_descr( QString& mdesc )
{
   int index = -1;

   for ( int ii = 0; ii < distros.count(); ii++ )
   {
      if ( mdesc == distribID( distros[ ii ].mdescr ) )
      {
         index  = ii;
         break;
      }
   }

   return index;
}

// Get the index to a distro using the model GUID
int US_DDistr_Combine::distro_by_mguid( QString& mguid )
{
   int index = -1;

   for ( int ii = 0; ii < distros.count(); ii++ )
   {
      if ( mguid == distros[ ii ].mGUID )
      {
         index  = ii;
         break;
      }
   }

   return index;
}

// Get the next index to a distro that has a matching run ID
int US_DDistr_Combine::distro_by_runid( QString& runid, int first )
{
   int index = -1;

   for ( int ii = first; ii < distros.count(); ii++ )
   {
      if ( runid == distros[ ii ].runID )
      {
         index  = ii;
         break;
      }
   }

   return index;
}

// Update distributions list objects from new run models
void US_DDistr_Combine::update_distros()
{
   for ( int ii = 0; ii < aDescrs.count(); ii++ )
   {
      QString mrun = aDescrs[ii].section( "\t", 0, 0 );
      QString mgid = aDescrs[ii].section( "\t", 1, 1 );
      QString mdes = aDescrs[ii].section( "\t", 2, 2 );

      DistrDesc dd;
      dd.runID     = mrun;
      dd.mGUID     = mgid;
      dd.mdescr    = mdes;
      dd.iters     = mdes.contains( "-MC_0" ) ? 1 : 0;
      if ( dd.iters != 0  &&  ! mdes.contains( "_mc0001" ) )  continue;
      if ( distro_by_mguid( mgid ) >= 0 )                     continue;

      dd.xvals.clear();
      dd.yvals.clear();

      distros << dd;
   }

   qSort( distros );

   return;
}

// Update Distributions list when a method check box is changed
void US_DDistr_Combine::methodChanged( int state )
{
   if ( state == Qt::Unchecked )
      ck_dtall->setChecked( false );

   list_distributions();
}

// Update Distributions list when the All method check box is changed
void US_DDistr_Combine::allMethodChanged( int state )
{
   if ( state == Qt::Checked )
   {
      ck_2dsa    ->setChecked( ck_2dsa    ->isEnabled() );
      ck_2dsamc  ->setChecked( ck_2dsamc  ->isEnabled() );
      ck_2dsamw  ->setChecked( ck_2dsamw  ->isEnabled() );
      ck_2dsamcmw->setChecked( ck_2dsamcmw->isEnabled() );
      ck_2dsafm  ->setChecked( ck_2dsafm  ->isEnabled() );
      ck_ga      ->setChecked( ck_ga      ->isEnabled() );
      ck_gamc    ->setChecked( ck_gamc    ->isEnabled() );
      ck_gamw    ->setChecked( ck_gamw    ->isEnabled() );
      ck_gamcmw  ->setChecked( ck_gamcmw  ->isEnabled() );
   }

   list_distributions();
}

// Change the contents of the distributions list based on method filtering
void US_DDistr_Combine::list_distributions()
{
   runid_select( lw_runids->currentRow() );
}

// React to a change in the X type of plots
void US_DDistr_Combine::changedPlotX( bool on_state )
{
   if ( ! on_state )  return;

DbgLv(1) << "changedPlotX" << on_state;
   bool x_is_sw    = rb_pltsw ->isChecked();
   bool x_is_MW    = rb_pltMW ->isChecked();
   bool x_is_Dw    = rb_pltDw ->isChecked();
   bool x_is_ff0   = rb_pltff0->isChecked();
   bool x_is_vb    = rb_pltvb ->isChecked();
   bool x_is_MWl   = rb_pltMWl->isChecked();
        xtype      = 0;

   if ( x_is_sw )
   {
DbgLv(1) << "  PX=Sed.Coeff";
      xtype           = 0;
   }

   else if ( x_is_MW )
   {
DbgLv(1) << "  PX=Molec.Wt.";
      xtype           = 1;
   }

   else if ( x_is_Dw )
   {
DbgLv(1) << "  PX=Diff.Coeff";
      xtype           = 2;
   }

   else if ( x_is_ff0 )
   {
DbgLv(1) << "  PX=f/f0";
      xtype           = 3;
   }

   else if ( x_is_vb )
   {
DbgLv(1) << "  PX=Vbar";
      xtype           = 4;
   }

   else if ( x_is_MWl )
   {
DbgLv(1) << "  PX=Molec.Wt.log";
      xtype           = 5;
   }

   int npdis    = pdistrs.size();
   if ( npdis > 0 )
   {  // Re-do plot distros to account for X-type change
      QList< DistrDesc >  wdistros;
      DistrDesc           ddist;
      DistrDesc*          pddist;

      for ( int ii = 0; ii < npdis; ii++ )
      {  // Build rudimentary plot distros without value arrays
         pddist       = &pdistrs[ ii ];
         ddist.runID  = pddist->runID;
         ddist.mGUID  = pddist->mGUID;
         ddist.mdescr = pddist->mdescr;
         ddist.iters  = pddist->iters;
         ddist.xtype  = xtype;
         ddist.model  = pddist->model;

         wdistros << ddist;
      }

      pdistrs.clear();
      pdisIDs.clear();

      for ( int ii = 0; ii < npdis; ii++ )
      {
         pddist       = &wdistros[ ii ];
         fill_in_desc( *pddist, ii );

         pdistrs << *pddist;
         pdisIDs << distribID( pddist->mdescr );
      }

      plot_data();
   }
}

// Scale Monte Carlo concentrations
void US_DDistr_Combine::scale_montecarlo( US_Model& model, int niters )
{
   double scfactor = 1.0 / (double)niters;
   QStringList sklist;
   QStringList skvals;
   QVector< US_Model::SimulationComponent > comps;
   int         ncomps = model.components.size();

   for ( int ii = 0; ii < ncomps; ii++ )
   {  // Build list of values and list of unique values
      comps << model.components[ ii ];
      QString skval = QString().sprintf( "%9.4e %9.4e",
            comps[ ii ].s, comps[ ii ].f_f0 );
      sklist << skval;

      if ( ! skvals.contains( skval ) )
         skvals << skval;
   }

   int nlist = sklist.size();
   int nuniq = skvals.size();
DbgLv(1) << "scMC: niters ncomps nlist nuniq"
 << niters << ncomps << nlist << nuniq;

   // Clear original components and sort the unique s,f_f0 values
   model.components.clear();
   skvals.sort();

   for ( int ii = 0; ii < nuniq; ii++ )
   {  // Loop to sum on each unique s,f/f0 point
      QString skval = skvals[ ii ];
      double  conc  = 0.0;
      int     kk    = 0;

      for ( int jj = 0; jj < nlist; jj++ )
      {  // Loop in full list, summing all matches to this point
         if ( skval == sklist[ jj ] )
         {  // Sum concentration and keep last index
            conc    += comps[ jj ].signal_concentration;
            kk       = jj;
         }
      }

      // Replace concentration with sum scaled by number of iterations
      comps[ kk ].signal_concentration = conc * scfactor;
      // Store this adjusted component in the new vector
      model.components << comps[ kk ];
   }
}

