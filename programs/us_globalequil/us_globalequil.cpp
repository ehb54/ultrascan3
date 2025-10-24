//! file us_globalequil.cpp
#include <QApplication>
#include "us_images.h"
#include "us_license_t.h"
#include "us_license.h"
#include "us_data_loader.h"
#include "us_gui_settings.h"
#include "us_gui_util.h"
#include "us_run_details2.h"
#include "us_settings.h"
#include "us_constants.h"
#include "us_editor.h"
#include "us_math2.h"
#include "us_db2.h"
#include "us_passwd.h"
#include "us_solution_vals.h"
#include "us_simparms.h"
#include "us_globalequil.h"
#include "us_model_select.h"
#include "us_eqreporter.h"
#if QT_VERSION < 0x050000
#define setSamples(a,b,c)  setData(a,b,c)
#define setSymbol(a)       setSymbol(*a)
#endif

// main program
int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );

   #include "main1.inc"

   // License is OK.  Start up.
   
   US_GlobalEquil w;
   w.show();                   //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication
}

// US_GlobalEquil class constructor
US_GlobalEquil::US_GlobalEquil() : US_Widgets( true )
{
   setWindowTitle( tr( "Global Equilibrium Analysis" ) );
   setPalette( US_GuiSettings::frameColor() );
   dbg_level  = US_Settings::us_debug();

   QBoxLayout* mainLayout = new QHBoxLayout( this );
   mainLayout->setSpacing         ( 2 );
   mainLayout->setContentsMargins ( 2, 2, 2, 2 );

   // Layouts
   QVBoxLayout* leftLayout     = new QVBoxLayout;
   QVBoxLayout* rightLayout    = new QVBoxLayout;

   QGridLayout* dataSelLayout  = new QGridLayout;
   QGridLayout* scnListLayout  = new QGridLayout;
   QGridLayout* modlFitLayout  = new QGridLayout;
   QGridLayout* paramLayout    = new QGridLayout;
   QGridLayout* statusLayout   = new QGridLayout;

   // Data Selection elements
   QLabel*      lb_datasel   = us_banner(     tr( "Data Selection"      ) ); 
   QPushButton* pb_loadExp   = us_pushbutton( tr( "Load Experiment"     ) );
                pb_details   = us_pushbutton( tr( "Run Details"         ) );
   QLayout*     lo_edlast    = us_checkbox(   tr( "Lastest Data Edit"   ),
                                              ck_edlast, true );
                dkdb_cntrls  = new US_Disk_DB_Controls(
                                  US_Settings::default_data_location() );
                pb_view      = us_pushbutton( tr( "View Report"         ) );
                pb_unload    = us_pushbutton( tr( "Unload all Data"     ) );
                pb_scdiags   = us_pushbutton( tr( "Scan Diagnostics"    ) );
                pb_ckscfit   = us_pushbutton( tr( "Check Scans for Fit" ) );
                pb_conchist  = us_pushbutton( tr( "Conc. Histogram"     ) );
                pb_resetsl   = us_pushbutton( tr( "Reset Scan Limits"   ) );
   QLabel*      lb_prjname   = us_label(      tr( "Project Name:"       ) );
                le_prjname   = us_lineedit();

   pb_details ->setEnabled( false );
   pb_view    ->setEnabled( false );
   pb_unload  ->setEnabled( false );
   pb_scdiags ->setEnabled( false );
   pb_ckscfit ->setEnabled( false );
   pb_conchist->setEnabled( false );
   pb_resetsl ->setEnabled( false );
   le_prjname ->setText( tr( "SampleFit" ) );

   connect( pb_loadExp,  SIGNAL( clicked()       ),
                         SLOT(   load()          ) );
   connect( pb_details,  SIGNAL( clicked()       ),
                         SLOT(   details()       ) );
   connect( pb_view,     SIGNAL( clicked()       ),
                         SLOT(   view_report()   ) );
   connect( pb_unload,   SIGNAL( clicked()       ),
                         SLOT(   unload()        ) );
   connect( pb_scdiags,  SIGNAL( clicked()       ),
                         SLOT(   scan_diags()    ) );
   connect( pb_ckscfit,  SIGNAL( clicked()       ),
                         SLOT(  check_scan_fit() ) );
   connect( pb_conchist, SIGNAL( clicked()       ),
                         SLOT(  conc_histogram() ) );
   connect( pb_resetsl,  SIGNAL( clicked()       ),
                         SLOT(   reset_scan_lims() ) );
   connect( le_prjname,  SIGNAL( textChanged(    const QString& ) ),
                         SLOT( new_project_name( const QString& ) ) );

   int row = 0;
   dataSelLayout->addWidget( lb_datasel,  row++, 0, 1, 2 );
   dataSelLayout->addWidget( pb_loadExp,  row,   0, 1, 1 );
   dataSelLayout->addWidget( pb_details,  row++, 1, 1, 1 );
   dataSelLayout->addLayout( lo_edlast,   row,   0, 1, 1 );
   dataSelLayout->addLayout( dkdb_cntrls, row++, 1, 1, 1 );
   dataSelLayout->addWidget( pb_unload,   row,   0, 1, 1 );
   dataSelLayout->addWidget( pb_view,     row++, 1, 1, 1 );
   dataSelLayout->addWidget( pb_scdiags,  row,   0, 1, 1 );
   dataSelLayout->addWidget( pb_ckscfit,  row++, 1, 1, 1 );
   dataSelLayout->addWidget( pb_conchist, row,   0, 1, 1 );
   dataSelLayout->addWidget( pb_resetsl,  row++, 1, 1, 1 );
   dataSelLayout->addWidget( lb_prjname,  row,   0, 1, 1 );
   dataSelLayout->addWidget( le_prjname,  row++, 1, 1, 1 );

   // Equilibrium Scans Table
   QLabel*      lb_equiscns  = us_banner( tr( "List of available Equilibrium"
                                              " Scans:" ) );
   tw_equiscns = new QTableWidget( 15, 5, this );
   tw_equiscns->setPalette( US_GuiSettings::editColor() );
   QFont font( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize(),
         QFont::Bold );
   QFontMetrics fm( font );
   int rowHgt = fm.lineSpacing();
   tw_equiscns->setMaximumHeight( rowHgt * 60 + 12 );
   tw_equiscns->setRowHeight( 0, rowHgt );
   tw_equiscns->setFont(
      QFont( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() ) );

   QPixmap bapix = US_Images::getImage( US_Images::ARROW_BLUE );
   blue_arrow    = QIcon( bapix );
   green_arrow   = US_Images::getIcon( US_Images::ARROW_GREEN );
   red_arrow     = US_Images::getIcon( US_Images::ARROW_RED   );
   iconw         = bapix.width();
DbgLv(1) << "BlueArrowIcon isNull" << blue_arrow.isNull();
DbgLv(1) << " RedArrowIcon isNull" << red_arrow.isNull();

   row     = 0;
   scnListLayout->addWidget( lb_equiscns, row++, 0, 1, 2 );
   scnListLayout->addWidget( tw_equiscns, row,   0, 5, 2 );
   row    += 5;
   lb_equiscns->setMaximumHeight( lb_prjname->height() );

   // Model Fit elements
   QLabel*      lb_mfitinfo  = us_banner(     tr( "Model and Fitting"
                                                  " Information:" ) );
                pb_selModel  = us_pushbutton( tr( "Select Model"        ) );
                pb_modlCtrl  = us_pushbutton( tr( "Model Control"         ) );
                pb_fitcntrl  = us_pushbutton( tr( "Fitting Control"     ) );
                pb_loadFit   = us_pushbutton( tr( "Load Fit"            ) );
                pb_monCarlo  = us_pushbutton( tr( "Monte Carlo"         ) );

   pb_selModel->setEnabled( false );
   pb_modlCtrl->setEnabled( false );
   pb_fitcntrl->setEnabled( false );
   pb_monCarlo->setEnabled( false );

   connect( pb_selModel, SIGNAL( clicked()       ),
                         SLOT(   select_model()    ) );
   connect( pb_modlCtrl, SIGNAL( clicked()       ),
                         SLOT(   model_control()   ) );
   connect( pb_fitcntrl, SIGNAL( clicked()       ),
                         SLOT(   fitting_control() ) );
   connect( pb_loadFit,  SIGNAL( clicked()       ),
                         SLOT(   load_fit()        ) );
   connect( pb_monCarlo, SIGNAL( clicked()       ),
                         SLOT(   monte_carlo()     ) );

   row     = 0;
   modlFitLayout->addWidget( lb_mfitinfo, row++, 0, 1, 2 );
   modlFitLayout->addWidget( pb_selModel, row,   0, 1, 1 );
   modlFitLayout->addWidget( pb_modlCtrl, row++, 1, 1, 1 );
   modlFitLayout->addWidget( pb_loadFit,  row,   0, 1, 1 );
   modlFitLayout->addWidget( pb_fitcntrl, row++, 1, 1, 1 );
   modlFitLayout->addWidget( pb_monCarlo, row,   1, 1, 1 );

   // Parameter Information elements
   QLabel*      lb_parminfo  = us_banner( tr( "Parameter Information:"    ) );
                pb_floatPar  = us_pushbutton( tr( "Float Parameters"      ) );
                pb_initPars  = us_pushbutton( tr( "Initialize Parameters" ) );
   QPushButton* pb_help      = us_pushbutton( tr( "Help"                  ) );
   QPushButton* pb_close     = us_pushbutton( tr( "Close"                 ) );
   QLabel*      lb_scselect  = us_label(      tr( "Scan Selector:"      ) );
                ct_scselect  = us_counter( 2, 0, 50, 1 );

   pb_floatPar->setEnabled( false );
   pb_floatPar->setVisible( false );
   pb_initPars->setEnabled( false );
   ct_scselect->setSingleStep(  1.0 );
   ct_scselect->setValue     ( 0.0 );

   connect( pb_floatPar, SIGNAL( clicked()              ),
                         SLOT(   float_params()         ) );
   connect( pb_initPars, SIGNAL( clicked()              ),
                         SLOT(   init_params()          ) );
   connect( pb_help,     SIGNAL( clicked()              ),
                         SLOT(   help()                 ) );
   connect( pb_close,    SIGNAL( clicked()              ),
                         SLOT(   close_all()            ) );
   connect( ct_scselect, SIGNAL( valueChanged( double ) ),
                         SLOT(   scan_select(  double ) ) );

   row     = 0;
   paramLayout  ->addWidget( lb_parminfo, row++, 0, 1, 4 );
   paramLayout  ->addWidget( pb_floatPar, row,   0, 1, 2 );
   paramLayout  ->addWidget( pb_initPars, row++, 2, 1, 2 );
   paramLayout  ->addWidget( pb_help,     row,   0, 1, 2 );
   paramLayout  ->addWidget( pb_close,    row++, 2, 1, 2 );
   paramLayout  ->addWidget( lb_scselect, row,   0, 1, 2 );
   paramLayout  ->addWidget( ct_scselect, row++, 2, 1, 2 );
   
   leftLayout->addLayout( dataSelLayout );
   leftLayout->addLayout( scnListLayout );
   leftLayout->addLayout( modlFitLayout );
   leftLayout->addLayout( paramLayout   );
   leftLayout->setStretchFactor( scnListLayout, 10 );
   leftLayout->setStretchFactor( modlFitLayout,  1 );

   // Equilibrium Data Plot
   eplot = new US_Plot( equil_plot, 
         tr( "Experiment Equilibrium Data" ),
         tr( "Radius (cm)" ),
         tr( "Absorbance (280 nm)" ) );
   us_grid( equil_plot );
   
   equil_plot->setMinimumSize( 600, 400 );
   equil_plot->setAxisScale( QwtPlot::yLeft  , 0.1, 0.601 );
   equil_plot->setAxisScale( QwtPlot::xBottom, 5.8, 7.2 );

   // Status elements
   QLabel*      lb_status    = us_label(    tr( "Status/Information:" ) );
                te_status    = us_textedit();
   te_status->setWordWrapMode( QTextOption::WordWrap );
   te_status->setText( tr( "Please select an edited Equilibrium"
                           " Dataset with \"Load Experiment\"" ) );
   QLabel*      lb_currmodl  = us_label(    tr( "Current Model:"      ) );
                le_currmodl  = us_lineedit( tr( "-- none selected --" ) );
   QLabel*      lb_mxfringe  = us_label(    tr( "Max. OD/Fringe:"     ) );
                le_mxfringe  = us_lineedit( tr( "0.90" ) );
   QLabel*      lb_mxfnotes  = us_label(    tr( "(set to zero to inactivate"
                                                " high conc. limits)"  ) );
   te_status  ->setAlignment( Qt::AlignCenter );
   le_currmodl->setAlignment( Qt::AlignCenter );
   us_setReadOnly( te_status,   true );
   us_setReadOnly( le_currmodl, true );
   te_status  ->setMinimumHeight( rowHgt * 2 + 12 );
   te_status  ->setFixedHeight(   rowHgt * 2 + 12 );

   row     = 0;
   statusLayout ->addWidget( lb_status,   row,   0, 1, 2 );
   statusLayout ->addWidget( te_status,   row++, 2, 1, 4 );
   statusLayout ->addWidget( lb_currmodl, row,   0, 1, 2 );
   statusLayout ->addWidget( le_currmodl, row++, 2, 1, 4 );
   statusLayout ->addWidget( lb_mxfringe, row,   0, 1, 2 );
   statusLayout ->addWidget( le_mxfringe, row,   2, 1, 1 );
   statusLayout ->addWidget( lb_mxfnotes, row++, 3, 1, 3 );

   connect( le_mxfringe, SIGNAL( textChanged(    const QString& ) ),
                         SLOT( od_limit_changed( const QString& ) ) );

   rightLayout->addLayout( eplot );
   rightLayout->addLayout( statusLayout );
   rightLayout->setStretchFactor( eplot,       10 );
   rightLayout->setStretchFactor( statusLayout, 2 );

   mainLayout->addLayout( leftLayout  ); 
   mainLayout->addLayout( rightLayout ); 
   mainLayout->setStretchFactor( leftLayout,   3 );
   mainLayout->setStretchFactor( rightLayout,  5 );

   emodctrl     = 0;
   efitctrl     = 0;
   ereporter    = 0;
   emath        = 0;
   ehisto       = 0;

   model_widget = false;
   fit_widget   = false;
   signal_mc    = true;
   floated_pars = false;
   show_msgs    = true;
}

// Public slot to respond to a new scan selection, signalled externally
void US_GlobalEquil::new_scan( int newscan )
{
   signal_mc = false;               // Avoid circular new-scan signals

   scan_select( (double)newscan );  // Act like scan counter was changed here

   signal_mc = true;                // Re-enable external signalling
}

// Load equilibrium data
void US_GlobalEquil::load( void )
{
   excludedScans.clear();
   dataList  .clear();
   rawList   .clear();
   triples   .clear();
   ds_vbar20s.clear();
   ds_densits.clear();
   ds_viscos .clear();
   ds_solIDs .clear();

   modelx      = 0;
   models << "";
   dataLoaded  = false;
   buffLoaded  = false;
   dataLatest  = ck_edlast->isChecked();
   dbdisk      = dkdb_cntrls->db() ? US_Disk_DB_Controls::DB
                                   : US_Disk_DB_Controls::Disk;

   // Open a dialog to select and load data
   US_DataLoader* dialog = new US_DataLoader( dataLatest, dbdisk, rawList,
         dataList, triples, workingDir, QString( "equilibrium" ) );

   connect( dialog, SIGNAL( changed(      bool ) ),
            this,   SLOT( update_disk_db( bool ) ) );

   if ( dialog->exec() != QDialog::Accepted )  return;

   US_DB2* dbP   = NULL;

   if ( dkdb_cntrls->db() )
   {
      US_Passwd pw;
      dbP        = new US_DB2( pw.getPasswd() );

      workingDir = tr( "(database)" );
   }

   else
   {
      workingDir = workingDir.section( workingDir.left( 1 ), 4, 4 );
      workingDir = workingDir.left( workingDir.lastIndexOf( "/" ) );
   }

   qApp->processEvents();
   runfit.dbdisk       = dbdisk;

   if ( dataList[ 0 ].expType != "Equilibrium" )
   {
      QMessageBox::critical( this, tr( "Non-Equilibrium Data" ),
         tr( "The selected data is not of type \"Equilibrium\"!\n"
             "Please select Equilibrium data." ) );
      unload();
      return;
   }

   // Count the total scans available in the data sets and get vbars,densities
   ntscns     = 0;

   for ( int jd = 0; jd < dataList.size(); jd++ )
   {
      ntscns += dataList[ jd ].scanData.size();

      QString s_vbar = QString::number( TYPICAL_VBAR );
      QString s_dens = QString::number( DENS_20W );
      QString s_visc;
      QString s_comp;
      QString s_manu;
      QString s_emsg;
      QString solID;
      US_SolutionVals::values( dbP, &dataList[ jd ], solID,
                               s_vbar, s_dens, s_visc, s_comp, s_manu, s_emsg );
      ds_vbar20s << s_vbar.toDouble();
      ds_densits << s_dens.toDouble();
      ds_viscos  << s_visc.toDouble();
      ds_solIDs  << solID;
   }
DbgLv(1) << "  jd vbar20 density" << 0 << ds_vbar20s[0] << ds_densits[0];
int nn=dataList.size()-1;
DbgLv(1) << "  jd vbar20 density" << nn << ds_vbar20s[nn] << ds_densits[nn];

   // Get Centerpiece bottom and rotor coefficients for calc_bottom
   US_SimulationParameters simparams;
   simparams.initFromData( dbP, dataList[ 0 ] );
   runfit.bottom_pos   = simparams.bottom_position;
   runfit.rcoeffs[ 0 ] = simparams.rotorcoeffs[ 0 ];
   runfit.rcoeffs[ 1 ] = simparams.rotorcoeffs[ 1 ];

   // Build the table of available scans
   QStringList headers;
   QFont font( US_GuiSettings::fontFamily(),
               US_GuiSettings::fontSize(), QFont::Bold );
   QFontMetrics fm( font );
   int rowHgt   = fm.lineSpacing();
   QString hdr1 = tr( "Scan" );
   QString hdr2 = tr( "CCW Triple" );
   QString hdr3 = tr( "Speed" );
   QString hdr4 = tr( "scan of set" );
   int     whd1 = fm.horizontalAdvance( hdr1 + "W" );
   int     whd2 = fm.horizontalAdvance( hdr2 + "W" );
   int     whd3 = fm.horizontalAdvance( hdr3 + "W" );
   int     whd4 = fm.horizontalAdvance( hdr4 + "W" );
   int     whd0 = ( iconw * 3 ) / 2;

   headers << "" << hdr1 << hdr2 << hdr3 << hdr4;
   tw_equiscns->setMaximumHeight( rowHgt * 60 + 12 );
   tw_equiscns->setRowCount( ntscns );
   tw_equiscns->setHorizontalHeaderLabels( headers );
   tw_equiscns->verticalHeader()->hide();
   tw_equiscns->setShowGrid( false );
   tw_equiscns->setSelectionBehavior( QAbstractItemView::SelectRows );
   tw_equiscns->setColumnWidth( 0, whd0 );
   tw_equiscns->setColumnWidth( 1, whd1 );
   tw_equiscns->setColumnWidth( 2, whd2 );
   tw_equiscns->setColumnWidth( 3, whd3 );
   tw_equiscns->setColumnWidth( 4, whd4 );
   tw_equiscns->setMinimumWidth(  160 );
   tw_equiscns->setMinimumHeight( 160 );

   scedits .resize( ntscns );
   scanfits.clear();

   int jsscn = 0;

   for ( int jd = 0; jd < dataList.size(); jd++ )
   {  // Loop to get scans from the data sets
      edata          = &dataList[ jd ];
      QString triple = triples[ jd ];
      QString tdesc  = edata->description;

      for ( int jr = 0; jr < edata->speedData.size(); jr++ )
      {  // Add a table entry for each speed step of each triple
         QTableWidgetItem* item;
         double  drpm   = edata->speedData[ jr ].speed;
         int     iscn   = edata->speedData[ jr ].first_scan;
         int     kscn   = edata->speedData[ jr ].scan_count;
         sRadLo         = edata->speedData[ jr ].dataLeft;
         sRadHi         = edata->speedData[ jr ].dataRight;

         for ( int js = iscn; js < iscn + kscn; js++ )
         {
            // Build a table entry with 5 column items
            item = new QTableWidgetItem( blue_arrow, "" );
            item->setToolTip( tdesc );
//if(iscn==3) item=new QTableWidgetItem( green_arrow, "" );
//if(iscn==5) item=new QTableWidgetItem( red_arrow, "" );
            tw_equiscns->setItem( jsscn, 0, item );

            item = new QTableWidgetItem( QString::number( jsscn + 1 ) );
            item->setFlags( item->flags() ^ Qt::ItemIsEditable );
            item->setToolTip( tdesc );
            tw_equiscns->setItem( jsscn, 1, item );

            item = new QTableWidgetItem( triple );
            item->setFlags( item->flags() ^ Qt::ItemIsEditable );
            item->setToolTip( tdesc );
            tw_equiscns->setItem( jsscn, 2, item );

            item = new QTableWidgetItem( QString::number( drpm ) );
            item->setFlags( item->flags() ^ Qt::ItemIsEditable );
            item->setToolTip( tdesc );
            tw_equiscns->setItem( jsscn, 3, item );

            item = new QTableWidgetItem( QString::number( js ) );
            item->setFlags( item->flags() ^ Qt::ItemIsEditable );
            item->setToolTip( tdesc );
            tw_equiscns->setItem( jsscn, 4, item );

            tw_equiscns->setRowHeight( jsscn, rowHgt );

            // Build a scan edit record
            scedits[ jsscn ].dsindex  = jd;
            scedits[ jsscn ].speedx   = jr;
            scedits[ jsscn ].scannbr  = js;
            scedits[ jsscn ].rad_lo   = sRadLo;
            scedits[ jsscn ].rad_hi   = sRadHi;
            scedits[ jsscn ].edited   = false;
DbgLv(1) << "  jsscn jd js" << jsscn << jd << js
 << "  sRadLo sRadHi" << sRadLo << sRadHi;

            jsscn++;
         }
      }
   }

   od_limit = 0.9;

DbgLv(1) << " LD: setup_runfit";
   setup_runfit();       // Build the runfit data structure
DbgLv(1) << " LD: assign_scanfit";
   assign_scanfit();     // Build a vector of scanfit data structures

DbgLv(1) << " LD: update_limit";
   update_limit( 0.9 );  // Possibly modify data ranges by OD limit

   // Reset the range of the scan counter to scans available
   ct_scselect->setRange( 1.0, (double)jsscn );
   ct_scselect->setSingleStep( 1.0 );

   connect( tw_equiscns, SIGNAL( itemDoubleClicked( QTableWidgetItem* ) ),
            this,        SLOT(   doubleClickedItem( QTableWidgetItem* ) ) );
   connect( tw_equiscns, SIGNAL( itemSelectionChanged( ) ),
            this,        SLOT(   itemRowChanged( )       ) );

   te_status->setText( tr( "To edit (exclude points):  Ctrl-click-hold,"
                           " move, and release mouse button in the plot area;"
                           " then release Ctrl key." ) );

   edata       = &dataList[ 0 ];

   if ( edata->dataType != "RA" )
      le_mxfringe->setText( "0.0" );   // If not absorbance, change OD limit

DbgLv(1) << "eData rvalues size" << edata->scanData[0].rvalues.size();
   dataLoaded  = true;
   pb_details ->setEnabled( true );
   pb_unload  ->setEnabled( true );
   pb_selModel->setEnabled( true );
}

// Open a dialog to display details of the data selected
void US_GlobalEquil::details( void )
{
   US_RunDetails2 dialog( rawList, edata->runID, workingDir, triples );
   dialog.exec();
   qApp->processEvents();
}

void US_GlobalEquil::view_report( void )
{ DbgLv(1) << "VIEW_REPORT()"; }

// Unload all data
void US_GlobalEquil::unload( void )
{
   dataList.clear();
   rawList .clear();
   triples .clear();
   scedits .clear();

   tw_equiscns->disconnect();
   tw_equiscns->clear();

   dataPlotClear( equil_plot );
   equil_plot->setTitle( tr( "Experiment Equilibrium Data" ) );

   setup_runfit();

   dataLoaded  = false;
   pb_details ->setEnabled( false );
   pb_unload  ->setEnabled( false );
   pb_selModel->setEnabled( false );
   pb_scdiags ->setEnabled( false );

   connect( tw_equiscns, SIGNAL( itemDoubleClicked( QTableWidgetItem* ) ),
            this,        SLOT(   doubleClickedItem( QTableWidgetItem* ) ) );
   connect( tw_equiscns, SIGNAL( itemSelectionChanged( ) ),
            this,        SLOT(   itemRowChanged( )       ) );
}

// Generate and display scan diagnostics
void US_GlobalEquil::scan_diags( void )
{
DbgLv(1) << "SCAN_DIAGS()";
   if ( ! dataLoaded )                  // Don't bother if no data yet
      return;

   if ( ereporter == 0 )                // Create reporter (1st time)
      ereporter  = new US_EqReporter( dataList, scedits, scanfits,
                                      runfit, this );

   ereporter->scan_diagnostics();       // Generate and display report

   // Modify scans list icons based on new fit flags
   for ( int jes = 0; jes < scedits.size(); jes++ )
   {
      if ( scanfits[ jes ].scanFit )    // Mark scan as fit/non-excluded
         tw_equiscns->item( jes, 0 )->setIcon( green_arrow );

      else                              // Mark scan as non-fit/excluded
         tw_equiscns->item( jes, 0 )->setIcon( blue_arrow );
   }

   pb_conchist->setEnabled( true );     // Enable buttons now useable
   pb_modlCtrl->setEnabled( true );
   pb_initPars->setEnabled( true );
}

void US_GlobalEquil::check_scan_fit( void )
{
   runfit.modlname     = models[ modelx ];
DbgLv(1) << "CHECK_SCAN_FIT()";
   if ( ereporter == 0 )                // Create reporter (1st time)
      ereporter  = new US_EqReporter( dataList, scedits, scanfits,
                                      runfit, this );
DbgLv(1) << " CkScFit:  sz - sced scnf" << scedits.size() << scanfits.size();

                                        // Generate and display report
   bool crit = ereporter->check_scan_fit( modelx );
DbgLv(1) << " CkScFit: modelx crit" << modelx << crit;

   pb_fitcntrl->setEnabled( !crit );    // Fit Control if no critical errors
}

void US_GlobalEquil::conc_histogram( void )
{
DbgLv(1) << "CONC_HISTOGRAM()";
   ehisto = new US_EqHistogram( od_limit, scanfits, this, Qt::WindowFlags() );
   ehisto->show();
}

// Reset the scan limits
void US_GlobalEquil::reset_scan_lims( void )
{
DbgLv(1) << "RESET_SCAN_LIMS()";
   sscanx  = tw_equiscns->currentRow();
   int jdx = scedits[ sscanx ].dsindex;
   int jrx = scedits[ sscanx ].speedx;

   scedits[ sscanx ].rad_lo = dataList[ jdx ].speedData[ jrx ].dataLeft;
   scedits[ sscanx ].rad_hi = dataList[ jdx ].speedData[ jrx ].dataRight;
   scedits[ sscanx ].edited = false;

   edata_plot();
   pb_resetsl->setEnabled( false );
}

void US_GlobalEquil::load_model( void )
{ DbgLv(1) << "LOAD_MODEL()"; }
void US_GlobalEquil::new_project_name( const QString& newpname )
{
DbgLv(1) << "NEW_PROJECT_NAME()" << newpname;
   runfit.projname     = newpname;
}

// Select the model
void US_GlobalEquil::select_model( void )
{
DbgLv(1) << "SELECT_MODEL()";
   US_ModelSelect* mdiag = new US_ModelSelect( modelx, models, aud_params );

   mdiag->exec();

int na=aud_params.size();
DbgLv(1) << "  modelx" << modelx << " nbr aud params" << na;
   if ( modelx >= 0 )
   {
      modelname  = models[ modelx ];
      le_currmodl->setText( modelname );
      pb_scdiags ->setEnabled( true );
DbgLv(1) << "   model" << modelname;
if(na==1) DbgLv(1) << "   par1: "   << aud_params[0];
if(na==2) DbgLv(1) << "   par1-2: " << aud_params[0] << aud_params[1];
if(na==4) DbgLv(1) << "   par1-4: " << aud_params[0] << aud_params[1]
   << aud_params[2] << aud_params[3];

      setup_runfit();
      assign_scanfit();

//DbgLv(1) << "     fix_all  model_widget" << model_widget;
      //fix_all();

      if ( model_widget )
         emodctrl->new_components();

      runfit.modlname     = modelname;
   }
}

void US_GlobalEquil::model_control( void )
{
DbgLv(1) << "MODEL_CONTROL()";
   sscanx       = tw_equiscns->currentRow();

   if ( sscanx < 0 )   scan_select( 1.0 );

   if ( model_widget )
   {
      emodctrl->raise();
      emodctrl->new_components();
   }

   else
   {
      model_widget = true;
      sscann       = sscanx + 1;

      emodctrl = new US_EqModelControl(
            scanfits, runfit, edata, modelx, models, model_widget, sscann );

      connect( emodctrl, SIGNAL( update_scan( int ) ),
               this,     SLOT(   new_scan(    int ) ) );

      emodctrl->show();
   }
}

void US_GlobalEquil::fitting_control( void )
{
DbgLv(1) << "FITTING_CONTROL()";
   if ( fit_widget )
   {
      efitctrl->raise();
   }

   else
   {
      fit_widget   = true;
      sscanx       = tw_equiscns->currentRow();
      sscann       = sscanx + 1;

      if ( emath == 0 )
         emath        = new US_EqMath( dataList, scedits, scanfits, runfit );

      runfit.modlname = models[ modelx ];

      if ( ereporter == 0 )                // Create reporter (1st time)
         ereporter    = new US_EqReporter( dataList, scedits, scanfits,
                                           runfit, this );

      efitctrl     = new US_EqFitControl( scanfits, runfit, edata, emath,
                                          ereporter, modelx, models,
                                          fit_widget, sscann );

      efitctrl->show();
   }
}

void US_GlobalEquil::load_fit( void )
{ DbgLv(1) << "LOAD_FIT()"; }
void US_GlobalEquil::monte_carlo( void )
{ DbgLv(1) << "MONTE_CARLO()"; }

void US_GlobalEquil::float_params( void )
{
DbgLv(1) << "FLOAT_PARAMS()";
   float_all();

   if ( model_widget )
      emodctrl->set_float( true );
}

// Initialize parameters and (if need be) open a model control dialog
void US_GlobalEquil::init_params( void )
{
DbgLv(1) << "INIT_PARAMS()";
   // Insure we have an object for doing needed calculations
   if ( emath == 0 )
      emath  = new US_EqMath( dataList, scedits, scanfits, runfit );

   // Count fitted scans and save the index to the first such
   int  fitx      = -1;
   bool update_mw = true;

   for ( int ii = 0; ii < scanfits.size(); ii++ )
      if ( scanfits[ ii ].scanFit  &&  fitx < 0 )
         fitx     = ii;

DbgLv(1) << "IP: fitx" << fitx;
DbgLv(1) << "IP:  points xvs0 xvsN" << scanfits[0].points
   << scanfits[0].xvs[0] << scanfits[0].xvs[scanfits[0].points-1];
   if ( fitx < 0 )
   {
      QMessageBox::warning( this, tr( "Scan Fits" ),
         tr( "There are no scans to fit!\n\n"
             "Please select one or more scans to be fitted." ) );
      return;
   }

   if ( runfit.mw_vals[ 0 ] <= 0.0 )
   {  // Calculate the molecular weight value the 1st time
      runfit.mw_vals[ 0 ] = emath->linesearch();
      runfit.mw_rngs[ 0 ] = runfit.mw_vals[ 0 ] * 0.2;
   }

   else
   {  // Thereafter, ask if user wants to calculate a new MW
      QMessageBox msgBox;
      msgBox.setWindowTitle( tr( "Molecular Weight" ) );
      msgBox.setText(
         tr( "Do you want to use the currently defined molecular"
             " weight for the parameter\n"
             "initialization or calculate a newly initialized"
             " molecular weight?" ) );
      msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::No 
                               | QMessageBox::Cancel );
      msgBox.setButtonText( QMessageBox::Yes,
            tr( "New Molecular Weight" ) );
      msgBox.setButtonText( QMessageBox::No,
            tr( "Current Molecular Weight" ) );

      switch( msgBox.exec() )
      {
         case QMessageBox::Yes:
         case QMessageBox::Default:
         default:
            runfit.mw_vals[ 0 ] = emath->linesearch();
            runfit.mw_rngs[ 0 ] = runfit.mw_vals[ 0 ] * 0.2;
            update_mw = true;
            break;

         case QMessageBox::No:
            emath->calc_testParameter( runfit.mw_vals[ 0 ] );
            runfit.mw_rngs[ 0 ] = runfit.mw_vals[ 0 ] * 0.2;
            update_mw = false;
            break;
         case QMessageBox::Cancel:
            update_mw = false;
            break;
      }
   }
DbgLv(1) << "IP: update_mw" << update_mw;

   // Initialize parameters
DbgLv(1) << "IP: em init_params call";
   emath->init_params( modelx, update_mw, ds_vbar20s, aud_params );

   // Display the model control dialog
   model_control();

   // Enable buttons that are now appropriate
   //pb_floatPar->setEnabled( true );
   pb_ckscfit ->setEnabled( true );
}

// Respond to a change in the selected scan
void US_GlobalEquil::scan_select( double newscan )
{
   sscann   = (int)newscan;
   sscanx   = sscann - 1;
DbgLv(1) << "SCAN_SELECT()" << sscann << sscanx;

   tw_equiscns->setCurrentCell( sscanx, 1 );   // Select the table row

   QString triple = tw_equiscns->item( sscanx, 2 )->text();
   double  drpm   = tw_equiscns->item( sscanx, 3 )->text().toDouble();
//DbgLv(1) << " Clicked: triple" << triple << "rpm" << drpm;
   int     jdx    = -1;
   int     jrx    = -1;

   bool    found  = findData( triple, drpm, jdx, jrx );
//DbgLv(1) << " Clicked:   found" << found << " jdx jsx" << jdx << jsx;

   if ( found )  edata_plot();   // Change the plot to the newly selected scan

   ct_scselect->disconnect();
   ct_scselect->setValue( newscan );              // Set the scan nbr counter
   connect( ct_scselect, SIGNAL( valueChanged( double ) ),
                         SLOT(   scan_select(  double ) ) );

DbgLv(1) << " GE:ClItem: signal_mc model_widget" << signal_mc << model_widget;
   if ( signal_mc )
   {  // Signalling of model control is enabled
      if ( model_widget )
      {  // If a model control is up, have it reset the scan
         emodctrl->new_scan( sscann );
      }
   }

   pb_resetsl ->setEnabled( scedits[ sscanx ].edited );

DbgLv(1) << " GE:ScSel: signal_mc model_widget" << signal_mc << model_widget;
   if ( signal_mc )
   {  // Signalling of model control is enabled
      if ( model_widget )
      {  // If a model control is up, have it reset the scan
         emodctrl->new_scan( sscann );
      }
   }
}

// Close all opened children, then close main
void US_GlobalEquil::close_all( void )
{
//DbgLv(1) << "CLOSE_ALL()";
   if ( model_widget )    emodctrl->close();
   if ( fit_widget )      efitctrl->close();
   if ( emath != 0 )      delete emath;
   if ( ereporter != 0 )  delete ereporter;

   close();
}

// Update Disk/DB selection
void US_GlobalEquil::update_disk_db( bool dbaccess )
{
   if ( dbaccess )
   {  // Database
      dkdb_cntrls->set_db();
      dbdisk    = US_Disk_DB_Controls::DB;
   }

   else
   {  // Local Disk
      dkdb_cntrls->set_disk();
      dbdisk    = US_Disk_DB_Controls::Disk;
   }

   runfit.dbdisk       = dbdisk;
}

// Respond to a change in the row selected
void US_GlobalEquil::itemRowChanged( )
{
DbgLv(1) << "itemRowChanged";
   scan_select( (double)( tw_equiscns->currentRow() + 1 ) );
}

// Respond to a table row being double-clicked
void US_GlobalEquil::doubleClickedItem( QTableWidgetItem* item )
{
   int row    = item->row();
   bool fit   = ! scanfits[ row ].scanFit;  // reverse fit setting
   bool excl  = scanfits[ row ].autoExcl;
   scanfits[ row ].scanFit = fit;

DbgLv(1) << "TableItemDoubleClicked row col" << row << item->column();
   if ( ! fit  )                     // Mark scan as non-fit
      tw_equiscns->item( row, 0 )->setIcon( blue_arrow );

   else if ( ! excl )                // Mark scan as fit/non-excluded
      tw_equiscns->item( row, 0 )->setIcon( green_arrow );

   else                              // Mark scan as fit/excluded
      tw_equiscns->item( row, 0 )->setIcon( red_arrow );
}


// Find the data (triple and speed step) corresponding to a scan selection
bool US_GlobalEquil::findData( QString trip, double drpm, int& jdx, int& jrx )
{
   bool found = false;
   jdx    = -1;
   jrx    = -1;

   while ( ++jdx < dataList.size() )
   {  // Search the data set list
      jrx  = -1;

      if ( trip == triples[ jdx ] )
      {  // If we are at the right triple, examine the speed data

         while ( ++jrx < dataList[ jdx ].speedData.size() )
         {  // Search the speed steps in this triple

            if ( dataList[ jdx ].speedData[ jrx ].speed == drpm )
            {  // Right speed in right triple:  get data and mark found
               edata  = &dataList[ jdx ];
               spdata = &edata->speedData[ jrx ];
               found  = true;
               break;
            }
         }
      }

      if ( found )  break;  // Break out of data list search when data found
   }

   return found;
}

// Plot equilibrium data as ellipses in radius,absorbance plane
void US_GlobalEquil::edata_plot()
{
   sscanx      = tw_equiscns->currentRow();

   if ( sscanx < 0 )
   {
      sscanx  = 0;
      tw_equiscns->setCurrentCell( sscanx, 1 );
      edata   = &dataList[ 0 ];
      spdata  = &edata->speedData[ 0 ];
   }

   sscann      = sscanx + 1;
   int iscan   = spdata->first_scan;
   int kscan   = spdata->scan_count;
   int jscan   = tw_equiscns->item( sscanx, 4 )->text().toInt();

   if ( jscan < iscan  ||  jscan >= ( iscan + kscan ) )
   {
      QMessageBox::warning( this, tr( "Scan Problem" ),
         tr( "Scan %1 is not within speed data scan range: %2 for %3" )
         .arg( jscan ).arg( iscan ).arg( kscan ) );
      return;
   }

   int     nrpts   = edata->pointCount();
   double  drpm    = spdata->speed;
   double  radl    = spdata->dataLeft;
   double  radr    = spdata->dataRight;
DbgLv(1) << "EdataPlot: radl radr" << radl << radr;
   QString runID   = edata->runID;
   QString editID  = edata->editID;
   QString cell    = edata->cell;
   QString chan    = edata->channel;
   QString waveln  = edata->wavelength;

   // Initialize the plot and its titles
   dataPlotClear( equil_plot );
   equil_plot->setTitle(
      tr( "Run: " ) + runID + tr( "  Edit: " ) + editID + "\n" +
      tr( "Cell " ) + cell + tr( ",  Channel " ) + chan +
      tr( ",  " ) + QString::number( drpm ) + tr( " rpm,  Scan " ) +
      QString::number( sscann ) );
   equil_plot->setAxisTitle( QwtPlot::yLeft,
      tr( "Absorbance (" ) + waveln + tr( " nm)" ) );

   // Set up the grid
   QwtPlotGrid* grid = us_grid( equil_plot );
   grid->enableYMin( true );
   grid->enableY   ( true );
   grid->setMajorPen( QPen( US_GuiSettings::plotMajGrid(), 0, Qt::DashLine ) );
   grid->setMinorPen( QPen( US_GuiSettings::plotMinGrid(), 0, Qt::DotLine  ) );

   // Set up the picker for mouse down, moves and up
   QwtPlotPicker* pick = new US_PlotPicker( equil_plot );
   pick->setRubberBand( QwtPicker::CrossRubberBand );
   connect( pick, SIGNAL( cMouseDown(   const QwtDoublePoint& ) ),
                  SLOT(   pMouseDown(  const QwtDoublePoint& ) ) );
   connect( pick, SIGNAL( cMouseUp(     const QwtDoublePoint& ) ),
                  SLOT(   pMouseUp(    const QwtDoublePoint& ) ) );
   connect( pick, SIGNAL( cMouseDrag(   const QwtDoublePoint& ) ),
                  SLOT(   pMouseMoved( const QwtDoublePoint& ) ) );

   if ( scedits[ sscanx ].edited )
   {
      radl    = scedits[ sscanx ].rad_lo;
      radr    = scedits[ sscanx ].rad_hi;
   }

   else
   {
      mDown   = false;
      mMoved  = false;
      mLowerH = false;
      sRadLo  = 0.0;
      sRadHi  = 0.0;
   }
DbgLv(1) << "EdataPlot: radl radr" << radl << radr
   << " edited" << scedits[sscanx].edited;

   // Accumulate data points
   rvec.fill( 0.0, nrpts );
   vvec.fill( 0.0, nrpts );
   double* ra  = rvec.data();
   double* va  = vvec.data();

   int isc     = jscan - 1;
   int count   = 0;
   double rlo  = 9e+10;
   double rhi  = -9e+10;
   double vlo  = 9e+10;
   double vhi  = -9e+10;
   int    krpt = qMin( nrpts, scanfits[ sscanx ].stop_ndx + 1 );

   for ( int jj = 0; jj < krpt; jj++ )
   {
      double rv = edata->radius( jj );

      if ( rv >= radl  &&  rv <= radr )
      {
         double vv     = edata->value( isc, jj );
         ra[ count   ] = rv;
         va[ count++ ] = vv;
         rlo           = qMin( rlo, rv );
         rhi           = qMax( rhi, rv );
         vlo           = qMin( vlo, vv );
         vhi           = qMax( vhi, vv );
      }
   }

   cRadLo = radl;
   cRadHi = radr;
DbgLv(1) << "EdataPlot: cRadLo cRadHi" << cRadLo << cRadHi;
DbgLv(1) << "EdataPlot: dr0 drn" << edata->radius(0) << edata->radius(nrpts-1);
DbgLv(1) << "EdataPlot:  ra0 rak" << ra[0] << ra[count-1];
DbgLv(1) << "EdataPlot:  va0 vak" << va[0] << va[count-1];
DbgLv(1) << "EdataPlot:   count" << count;
   vecknt = count;

   double rpad = ( rhi - rlo ) * 0.05;  // pad range 5 percent beyond low,high
   double vpad = ( vhi - vlo ) * 0.05;
   rlo -= rpad;
   rhi += rpad;
   vlo -= vpad;
   vhi += vpad;

   // Set the scale and plot data points as ellipses
   equil_plot->setAxisScale( QwtPlot::xBottom, rlo, rhi );
   equil_plot->setAxisScale( QwtPlot::yLeft,   vlo, vhi );

   QwtSymbol* sym = new QwtSymbol;
   sym->setStyle( QwtSymbol::Ellipse );
   sym->setPen  ( QPen( Qt::blue ) );
   sym->setBrush( QBrush( Qt::yellow ) );
   sym->setSize ( 10 );

   QwtPlotCurve* curve = us_curve( equil_plot, "Equil Data" );
   curve->setStyle( QwtPlotCurve::NoCurve );
   curve->setSymbol ( sym );
   curve->setSamples( ra, va, count );

   equil_plot->replot();
}

// Re-draw curve after editing to show yellow and red ellipses
void US_GlobalEquil::edited_plot( void )
{
   // Scan data points to find where radius crosses current mouse position
   double* ru  = rvec.data();
   double* vu  = vvec.data();
   double* re  = ru;
   double* ve  = vu;

   int countu  = 0;
   int counte  = 0;

   for ( int jj = 0; jj < vecknt; jj++ )
   {
      if ( ru[ jj ] > sRadMv )
      {  // Save count to position where radius is beyond mouse position
         countu   = jj;
         break;
      }
   }

   if ( countu == 0 )
      return;

   // Detach previous plot curve(s)
   equil_plot->detachItems( QwtPlotItem::Rtti_PlotCurve );

   // Set up symbols and curves for unedited and edited arrays
   QwtSymbol* symu = new QwtSymbol;
   symu->setStyle( QwtSymbol::Ellipse );
   symu->setPen  ( QPen( Qt::blue ) );
   symu->setBrush( QBrush( Qt::yellow ) );  // Unedited yellow
   symu->setSize ( 10 );
   QwtSymbol* syme = new QwtSymbol;
   syme->setStyle( QwtSymbol::Ellipse );
   syme->setPen  ( QPen( Qt::white ) );
   syme->setBrush( QBrush( Qt::red ) );     // Edited red
   syme->setSize ( 10 );

   QwtPlotCurve* curvu = us_curve( equil_plot, "Equil Data" );
   curvu->setStyle ( QwtPlotCurve::NoCurve );
   curvu->setSymbol( symu );
   QwtPlotCurve* curve = us_curve( equil_plot, "Edited Data" );
   curve->setStyle ( QwtPlotCurve::NoCurve );
   curve->setSymbol( syme );

   // Set edited/unedited division based on lower/upper half mouse position
   if ( mLowerH )
   {  // Mouse in lower half:  edited at beginning; unedited at division
      counte   = countu;
      countu   = vecknt - counte;
      ru       = ru + counte; 
      vu       = vu + counte; 
   }

   else
   {  // Mouse in upper half:  unedited at beginning; edited at division
      counte   = vecknt - countu;
      re       = ru + countu; 
      ve       = vu + countu; 
   }

   curvu->setSamples( ru, vu, countu );
   curve->setSamples( re, ve, counte );

   equil_plot->replot();
}

// Respond to mouse button down
void US_GlobalEquil::pMouseDown( const QwtDoublePoint& p )
{
   mMoved = false;
   mDown  = true;
   sRadLo = p.x();
   double RadMid = ( cRadLo + cRadHi ) * 0.5;
   mLowerH = sRadLo < RadMid;
}

// Respond to mouse button up (after move)
void US_GlobalEquil::pMouseUp( const QwtDoublePoint& p )
{
   // If mouse never moved, ignore release; otherwise reset mouse condition
   if ( ! mMoved )
      return;

   mMoved = false;
   mDown  = false;

   // Set new edited range based on where the mouse is positioned
   if ( mLowerH )
   {  // Lower half:  range-to-keep is from current position to end
      sRadLo = p.x();
      sRadHi = cRadHi;
   }

   else
   {  // Upper half:  range-to-keep is from beginning to current position
      sRadLo = cRadLo;
      sRadHi = p.x();
   }

   // Save the radius range of the edited scan
   scedits[ sscanx ].edited  = true;
   scedits[ sscanx ].rad_lo  = sRadLo;
   scedits[ sscanx ].rad_hi  = sRadHi;

   // Turn off any zoom that might have been on
   eplot->btnZoom->setDown   ( false );
   eplot->btnZoom->setChecked( false );

   // Re-draw the full edited plot
   edata_plot();

   pb_resetsl->setEnabled( true );
}

// Respond to mouse button being moved - redraw curve with edited points
void US_GlobalEquil::pMouseMoved( const QwtDoublePoint& p )
{
   if ( ! mDown )
      return;

   mMoved  = true;      // Flag that we are moving the mouse
   sRadMv  = p.x();     // Save the current mouse radius position

   if ( sRadMv > cRadHi  ||  sRadMv < cRadLo )
      return;

   edited_plot();       // Re-draw plot curves showing edited points
}

// Create the Scan Fits vector of parameters
void US_GlobalEquil::assign_scanfit()
{
   if ( scanfits.size() > 0 )
      return;           // Only create base scanfits array one time

   EqScanFit   scanfit;
   QStringList channs;
   int         ncomp = qMax( 1, runfit.nbr_comps );
   int         mcomp = qMax( 4, ncomp );
   int         nintg = mcomp + runfit.nbr_assocs;
   
   for ( int jes = 0; jes < scedits.size(); jes++ )
   {
      int    jdx   = scedits[ jes ].dsindex;
      int    jrx   = scedits[ jes ].speedx;
      int    jsx   = scedits[ jes ].scannbr - 1;
      double radlo = scedits[ jes ].rad_lo;
      double radhi = scedits[ jes ].rad_hi;
      edata        = &dataList[ jdx ];
      US_DataIO::Scan* dscan = &edata->scanData[ jsx ];
      QString trip = triples[ jdx ];
      QString chan = trip.section( "/", 1, 1 ).simplified();

      if ( ! channs.contains( chan ) )
         channs << chan;    // build list of unique channels

      scanfit.scanFit    = false;
      scanfit.autoExcl   = false;
      scanfit.limsModd   = false;
      scanfit.points     = edata->pointCount();
      scanfit.nbr_posr   = 0;
      scanfit.nbr_negr   = 0;
      scanfit.runs       = 0;
      scanfit.start_ndx  = index_radius( edata, radlo );
      scanfit.stop_ndx   = index_radius( edata, radhi );
      scanfit.density    = ds_densits[ jdx ];
      scanfit.viscosity  = ds_viscos [ jdx ];
      scanfit.tempera    = dscan->temperature;
      scanfit.pathlen    = 1.2;
      scanfit.meniscus   = edata->meniscus;
      scanfit.baseline   = radlo;
      scanfit.baseln_ndx = scanfit.start_ndx;
      scanfit.baseln_rng = radlo * 0.2;
      scanfit.baseln_fit = true;
      scanfit.baseln_bnd = true;
      scanfit.rpm        = (int)edata->speedData[ jrx ].speed;
      scanfit.cell       = trip.section( "/", 0, 0 ).simplified().toInt();
      scanfit.channel    = channs.indexOf( chan ) + 1;
      scanfit.wavelen    = trip.section( "/", 2, 2 ).simplified().toInt();
      scanfit.runID      = edata->runID;
      scanfit.descript   = edata->description;

      scanfit.xvs.resize( scanfit.points );
      scanfit.yvs.resize( scanfit.points );
      scanfit.amp_vals.fill(   0.0, mcomp );
      scanfit.amp_ndxs.fill(     0, mcomp );
      scanfit.amp_rngs.fill(   0.0, mcomp );
      scanfit.amp_fits.fill(  true, mcomp );
      scanfit.amp_bnds.fill( false, mcomp );
      scanfit.extincts.fill(   1.0, nintg );
      scanfit.integral.fill(   0.0, nintg );

      for ( int jj = 0; jj < scanfit.points; jj++ )
      {
         scanfit.xvs[ jj ] = edata->radius( jj );
         scanfit.yvs[ jj ] = edata->value( jsx, jj );
      }

      scanfit.stop_ndx   = index_od_limit( scanfit, od_limit );

      scanfits << scanfit;
   }
DbgLv(1) << "AsnSF: points xvs0 xvsN" << scanfit.points
   << scanfit.xvs[0] << scanfit.xvs[scanfit.points-1];
}

// Set up global Run Fit parameters profile
void US_GlobalEquil::setup_runfit()
{
   runfit.nbr_comps    = 1;
   runfit.nbr_assocs   = 0;
   runfit.nbr_runs     = 0;
   runfit.runs_percent = 0.0;
   runfit.runs_expect  = 0.0;
   runfit.runs_vari    = 0.0;
   runfit.projname     = le_prjname->text();
   modelx              = qMax( modelx, 0 );
DbgLv(1) << " sRF: modelx" << modelx;
   runfit.modlname     = models[ modelx ];
DbgLv(1) << " sRF: modlname" << runfit.modlname;

   switch ( modelx )
   {
      case 0:
         runfit.nbr_comps    = 1;
         runfit.nbr_assocs   = 0;
         break;
      case 1:
         runfit.nbr_comps    = 2;
         runfit.nbr_assocs   = 0;
         break;
      case 2:
         runfit.nbr_comps    = 3;
         runfit.nbr_assocs   = 0;
         break;
      case 3:
         runfit.nbr_comps    = (int)aud_params[ 0 ];
         runfit.nbr_assocs   = 1;
         break;
      case 4:
      case 5:
      case 6:
      case 7:
      case 8:
      case 9:
      case 10:
         runfit.nbr_comps    = 1;
         runfit.nbr_assocs   = 1;
         break;
      case 11:
      case 12:
      case 13:
         runfit.nbr_comps    = 1;
         runfit.nbr_assocs   = 2;
         break;
      case 14:
      case 16:
      case 17:
      case 19:
         runfit.nbr_comps    = 2;
         runfit.nbr_assocs   = 1;
         break;
      case 15:
         runfit.nbr_comps    = 2;
         runfit.nbr_assocs   = 2;
      case 18:
         runfit.nbr_comps    = 2;
         runfit.nbr_assocs   = 0;
      default:
         break;
   }
DbgLv(1) << " sRF: ncomps nassocs" << runfit.nbr_comps << runfit.nbr_assocs;

   double dvval  = TYPICAL_VBAR;

   if ( ds_vbar20s.size() > 0 )
      dvval      = ds_vbar20s[ 0 ];

   double dvrng  = dvval * 0.2;
   runfit.mw_vals  .fill(   0.0, runfit.nbr_comps );
   runfit.mw_ndxs  .fill(     0, runfit.nbr_comps );
   runfit.mw_rngs  .fill(   0.0, runfit.nbr_comps );
   runfit.mw_fits  .fill(  true, runfit.nbr_comps );
   runfit.mw_bnds  .fill( false, runfit.nbr_comps );
   runfit.vbar_vals.fill( dvval, runfit.nbr_comps );
   runfit.vbar_ndxs.fill(     0, runfit.nbr_comps );
   runfit.vbar_rngs.fill( dvrng, runfit.nbr_comps );
   runfit.vbar_fits.fill(  true, runfit.nbr_comps );
   runfit.vbar_bnds.fill( false, runfit.nbr_comps );
   runfit.viri_vals.fill(   0.0, runfit.nbr_comps );
   runfit.viri_ndxs.fill(     0, runfit.nbr_comps );
   runfit.viri_rngs.fill(   0.0, runfit.nbr_comps );
   runfit.viri_fits.fill( false, runfit.nbr_comps );
   runfit.viri_bnds.fill( false, runfit.nbr_comps );

   for ( int ii = 0; ii < 4; ii++ )
   {
      runfit.eq_vals[ ii ] = 0.0;
      runfit.eq_ndxs[ ii ] = 0;
      runfit.eq_rngs[ ii ] = 0.0;
      runfit.eq_fits[ ii ] = false;
      runfit.eq_bnds[ ii ] = false;
      runfit.stoichs[ ii ] = 1.0;
   }
}

// Determine the index in the radius vector of a given radius
int US_GlobalEquil::index_radius( US_DataIO::EditedData* edat, double radius )
{
   int l_index = edat->pointCount() - 1;
   int r_index = -1;

   while ( ++r_index < l_index )
   {
      if ( radius <= edat->radius( r_index ) )
         break;
   }

   return r_index;
}

// React to change in OD limit
void US_GlobalEquil::od_limit_changed( const QString& newlim )
{
   update_limit( newlim.toDouble() );
}

// Change stop index based on updated OD limit
void US_GlobalEquil::update_limit( double odlim )
{
   od_limit = odlim;

   if ( scanfits.size() > 0  &&  od_limit != 0.0 )
   {
      for ( int ii = 0; ii < scanfits.size(); ii++ )
      {
         EqScanFit* sfit = &scanfits[ ii ];
         int    jdx      = scedits[ ii ].dsindex;
         double radhi    = scedits[ ii ].rad_hi;
         sfit->stop_ndx  = index_radius( &dataList[ jdx ], radhi );
         sfit->stop_ndx  = index_od_limit( scanfits[ ii], odlim );
      }
   }

   else
   {
      for ( int ii = 0; ii < scanfits.size(); ii++ )
      {
         int    jdx      = scedits[ ii ].dsindex;
         double radhi    = scedits[ ii ].rad_hi;
         scanfits[ ii ].stop_ndx = index_radius( &dataList[ jdx ], radhi );
      }
   }
}

// Find the index in a scan of the od_limit point
int US_GlobalEquil::index_od_limit( EqScanFit& scanfit, double odlim )
{
   int stopx    = scanfit.stop_ndx;

   for ( int jj = scanfit.start_ndx; jj < scanfit.stop_ndx + 1; jj++ )
   {
      if ( scanfit.yvs[ jj ] > odlim )
      {
         stopx = qMax( jj - 1, scanfit.start_ndx );
         break;
      }
   }

   return stopx;
}

// Set fit parameters to floated state
void US_GlobalEquil::float_all()
{
   for ( int jj = 0; jj < runfit.nbr_comps; jj++ )
      runfit.mw_fits[ jj ] = true;

   for ( int jj = 0; jj < runfit.nbr_assocs; jj++ )
      runfit.eq_fits[ jj ] = true;

   for ( int ii = 0; ii < scanfits.size(); ii++ )
   {
      scanfits[ ii ].baseln_fit = true;

      for ( int jj = 0; jj < runfit.nbr_comps; jj++ )
         scanfits[ ii ].amp_fits[ jj ] = true;
   }
}
// Set fit parameters to fixed (locked) state
void US_GlobalEquil::fix_all()
{
   for ( int jj = 0; jj < runfit.nbr_comps; jj++ )
      runfit.mw_fits[ jj ] = false;

   for ( int jj = 0; jj < runfit.nbr_assocs; jj++ )
      runfit.eq_fits[ jj ] = false;

   for ( int ii = 0; ii < scanfits.size(); ii++ )
   {
      scanfits[ ii ].baseln_fit = false;

      for ( int jj = 0; jj < runfit.nbr_comps; jj++ )
         scanfits[ ii ].amp_fits[ jj ] = false;
   }
}

