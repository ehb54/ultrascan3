//! file us_globalequil.cpp
#include <QApplication>
#include "us_globalequil.h"
#include "us_model_select.h"
#include "us_images.h"
#include "us_license_t.h"
#include "us_license.h"
#include "us_data_loader.h"
#include "us_gui_settings.h"
#include "us_run_details2.h"
#include "us_settings.h"
#include "us_constants.h"
#include "us_math2.h"

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

   QPalette gray = US_GuiSettings::editColor();
   gray.setColor( QPalette::Base, QColor( 0xe0, 0xe0, 0xe0 ) );

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
   modlFitLayout->addWidget( pb_monCarlo, row,   0, 1, 1 );

   // Parameter Information elements
   QLabel*      lb_parminfo  = us_banner( tr( "Parameter Information:"    ) );
                pb_floatPar  = us_pushbutton( tr( "Float Parameters"      ) );
                pb_initPars  = us_pushbutton( tr( "Initialize Parameters" ) );
   QPushButton* pb_help      = us_pushbutton( tr( "Help"                  ) );
   QPushButton* pb_close     = us_pushbutton( tr( "Close"                 ) );
   QLabel*      lb_scselect  = us_label(      tr( "Scan Selector:"      ) );
                ct_scselect  = us_counter( 2, 0, 50, 1 );

   pb_floatPar->setEnabled( false );
   pb_initPars->setEnabled( false );
   ct_scselect->setStep(  1.0 );
   ct_scselect->setValue( 0.0 );

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
   te_status  ->setPalette( gray );
   le_currmodl->setPalette( gray );
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

   rightLayout->addLayout( eplot );
   rightLayout->addLayout( statusLayout );
   rightLayout->setStretchFactor( eplot,       10 );
   rightLayout->setStretchFactor( statusLayout, 2 );

   mainLayout->addLayout( leftLayout  ); 
   mainLayout->addLayout( rightLayout ); 
   mainLayout->setStretchFactor( leftLayout,   3 );
   mainLayout->setStretchFactor( rightLayout,  5 );
}

// Load equilibrium data
void US_GlobalEquil::load( void )
{
DbgLv(1) << "LOAD()";
   excludedScans.clear();
   dataList     .clear();
   rawList      .clear();
   triples      .clear();

   dataLoaded  = false;
   buffLoaded  = false;
   dataLatest  = ck_edlast->isChecked();
   dbdisk      = dkdb_cntrls->db() ? US_Disk_DB_Controls::DB
                                   : US_Disk_DB_Controls::Disk;

   // Open a dialog to select and load data
   US_DataLoader* dialog = new US_DataLoader( dataLatest, dbdisk, rawList,
         dataList, triples, workingDir );

   connect( dialog, SIGNAL( changed(      bool ) ),
            this,   SLOT( update_disk_db( bool ) ) );

   if ( dialog->exec() != QDialog::Accepted )  return;

   if ( dkdb_cntrls->db() )
      workingDir = tr( "(database)" );

   else
   {
      workingDir = workingDir.section( workingDir.left( 1 ), 4, 4 );
      workingDir = workingDir.left( workingDir.lastIndexOf( "/" ) );
   }
DbgLv(1) << " workingDir" << workingDir;
DbgLv(1) << " dataList size" << dataList.size();
//for ( int jj=0;jj<triples.size();jj++ ) DbgLv(1) << "  " << triples[jj];

   qApp->processEvents();

   // Count the scans available in the data sets
   ntscns     = 0;

   for ( int jd = 0; jd < dataList.size(); jd++ )
      ntscns += dataList[ jd ].scanData.size();

   // Build the table of available scans
   QFont font( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize(),
         QFont::Bold );
   QFontMetrics fm( font );
   int rowHgt   = fm.lineSpacing();
   tw_equiscns->setMaximumHeight( rowHgt * 60 + 12 );
   tw_equiscns->setRowCount( ntscns );
   QString hdr1 = tr( "Scan" );
   QString hdr2 = tr( "CCW Triple" );
   QString hdr3 = tr( "Speed" );
   QString hdr4 = tr( "scan of set" );
   int     whd1 = fm.width( hdr1 + "W" );
   int     whd2 = fm.width( hdr2 + "W" );
   int     whd3 = fm.width( hdr3 + "W" );
   int     whd4 = fm.width( hdr4 + "W" );
   int     whd0 = ( iconw * 3 ) / 2;
DbgLv(1) << "rowHgt" << rowHgt;
DbgLv(1) << "hdr" << hdr1 << " width" << whd1;
DbgLv(1) << "hdr" << hdr2 << " width" << whd2;
DbgLv(1) << "hdr" << hdr3 << " width" << whd3;
DbgLv(1) << "hdr" << hdr4 << " width" << whd4;
DbgLv(1) << "hdr" << " 0" << " width" << whd0;
   QStringList headers;

   headers << "" << hdr1 << hdr2 << hdr3 << hdr4;
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
      sRadLo         = edata->radius( 0 ); 
      sRadHi         = edata->radius( edata->x.size() - 1 );

      for ( int jr = 0; jr < edata->speedData.size(); jr++ )
      {  // Add a table entry for each speed step of each triple
         QTableWidgetItem* item;
         double  drpm   = edata->speedData[ jr ].speed;
         int     iscn   = edata->speedData[ jr ].first_scan;
         int     kscn   = edata->speedData[ jr ].scan_count;

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
            scedits[ jsscn ].sscanx   = js;
            scedits[ jsscn ].rad_lo   = sRadLo;
            scedits[ jsscn ].rad_hi   = sRadHi;
            scedits[ jsscn ].edited   = false;
DbgLv(1) << "  jsscn jd js" << jsscn << jd << js
 << "  sRadLo sRadHi" << sRadLo << sRadHi;

            jsscn++;
         }
      }
   }

   assign_scanfit();
   setup_runfit();

   // Reset the range of the scan counter to scans available
   ct_scselect->setRange( 1.0, (double)jsscn, 1.0 );

   connect( tw_equiscns, SIGNAL( itemClicked( QTableWidgetItem* ) ),
            this,        SLOT(   clickedItem( QTableWidgetItem* ) ) );

   te_status->setText( tr( "To edit (exclude points):  Ctrl-click-hold,"
                           " move, and release mouse button in the plot area;"
                           " then release Ctrl key." ) );

   edata       = &dataList[ 0 ];
DbgLv(1) << "eData readings size" << edata->scanData[0].readings.size();
   dataLoaded  = true;
   pb_details ->setEnabled( true );
   pb_unload  ->setEnabled( true );
   pb_selModel->setEnabled( true );

}

// Open a dialog to display details of the data selected
void US_GlobalEquil::details( void )
{
DbgLv(1) << "DETAILS()";
   QString runID  = edata->runID;

   US_RunDetails2 dialog( rawList, runID, workingDir, triples );
   dialog.exec();
   qApp->processEvents();
}

void US_GlobalEquil::view_report( void )
{ DbgLv(1) << "VIEW_REPORT()"; }

// Unload all data
void US_GlobalEquil::unload( void )
{
DbgLv(1) << "UNLOAD()";
   dataList.clear();
   rawList .clear();
   triples .clear();
   scedits .clear();

   tw_equiscns->clear();

   equil_plot->detachItems();
   equil_plot->setTitle( tr( "Experiment Equilibrium Data" ) );

   assign_scanfit();
   setup_runfit();

   dataLoaded  = false;
   pb_details ->setEnabled( false );
   pb_unload  ->setEnabled( false );
   pb_selModel->setEnabled( false );
   pb_scdiags ->setEnabled( false );
}
void US_GlobalEquil::scan_diags( void )
{ DbgLv(1) << "SCAN_DIAGS()"; }
void US_GlobalEquil::check_scan_fit( void )
{ DbgLv(1) << "CHECK_SCAN_FIT()"; }
void US_GlobalEquil::conc_histogram( void )
{ DbgLv(1) << "CONC_HISTOGRAM()"; }

// Reset the scan limits
void US_GlobalEquil::reset_scan_lims( void )
{
DbgLv(1) << "RESET_SCAN_LIMS()";
   for ( int js = 0; js < ntscns; js++ )
   {
      int jdx = scedits[ js ].dsindex;
      int jrx = scedits[ js ].speedx;

      scedits[ js ].rad_lo = dataList[ jdx ].speedData[ jrx ].dataLeft;
      scedits[ js ].rad_hi = dataList[ jdx ].speedData[ jrx ].dataRight;
   }

   edata_plot();
   pb_resetsl->setEnabled( false );
}

void US_GlobalEquil::load_model( void )
{ DbgLv(1) << "LOAD_MODEL()"; }
void US_GlobalEquil::new_project_name( const QString& newpname )
{ DbgLv(1) << "NEW_PROJECT_NAME()" << newpname; }

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

      if ( modelx == 3 )
         setup_runfit();
   }
}

void US_GlobalEquil::model_control( void )
{ DbgLv(1) << "MODEL_CONTROL()"; }
void US_GlobalEquil::fitting_control( void )
{ DbgLv(1) << "FITTING_CONTROL()"; }
void US_GlobalEquil::load_fit( void )
{ DbgLv(1) << "LOAD_FIT()"; }
void US_GlobalEquil::monte_carlo( void )
{ DbgLv(1) << "MONTE_CARLO()"; }
void US_GlobalEquil::float_params( void )
{ DbgLv(1) << "FLOAT_PARAMS()"; }
void US_GlobalEquil::init_params( void )
{ DbgLv(1) << "INIT_PARAMS()"; }

// Respond to a change in the selected scan
void US_GlobalEquil::scan_select( double newscan )
{
   int jrow = (int)newscan - 1;
//DbgLv(1) << "SCAN_SELECT()" << newscan << jrow;

   tw_equiscns->setCurrentCell( jrow, 0 );     // Select the table row

   clickedItem( tw_equiscns->currentItem() );  // Draw the appropriate plot
}

void US_GlobalEquil::close_all( void )
{
//DbgLv(1) << "CLOSE_ALL()";
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
}

// Respond to a table row being clicked
void US_GlobalEquil::clickedItem( QTableWidgetItem* item )
{
   int row = item->row();
//DbgLv(1) << "TableItemClicked row col" << row << item->column();

   QTableWidget* tabw = item->tableWidget();

   QString triple = tabw->item( row, 2 )->text();
   double  drpm   = tabw->item( row, 3 )->text().toDouble();
//DbgLv(1) << " Clicked: triple" << triple << "rpm" << drpm;
   int     jdx    = -1;
   int     jrx    = -1;

   bool    found  = findData( triple, drpm, jdx, jrx );
//DbgLv(1) << " Clicked:   found" << found << " jdx jsx" << jdx << jsx;

   if ( found )  edata_plot();

   ct_scselect->disconnect();
   ct_scselect->setValue( (double)( row + 1 ) );
   connect( ct_scselect, SIGNAL( valueChanged( double ) ),
                         SLOT(   scan_select(  double ) ) );
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
   int     jsscn   = tw_equiscns->currentRow();

   if ( jsscn < 0 )
   {
      jsscn  = 0;
      tw_equiscns->setCurrentCell( jsscn, 0 );
      edata  = &dataList[ 0 ];
      spdata = &edata->speedData[ 0 ];
   }

   int     iscan   = spdata->first_scan;
   int     kscan   = spdata->scan_count;
   int     jscan   = tw_equiscns->item( jsscn, 4 )->text().toInt();

   if ( jscan < iscan  ||  jscan >= ( iscan + kscan ) )
   {
      qDebug() << "Scan " << jscan << " not within speed data scan range:"
         << iscan << " for" << kscan;
      return;
   }

   int     nrpts   = edata->x.size();
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
   equil_plot->detachItems();
   equil_plot->setTitle(
      tr( "Run: " ) + runID + tr( "  Edit: " ) + editID + "\n" +
      tr( "Cell " ) + cell + tr( ",  Channel " ) + chan +
      tr( ",  " ) + QString::number( drpm ) + tr( " rpm,  Scan " ) +
      QString::number( jsscn + 1 ) );
   equil_plot->setAxisTitle( QwtPlot::yLeft,
      tr( "Absorbance (" ) + waveln + tr( " nm)" ) );

   // Set up the grid
   QwtPlotGrid* grid = us_grid( equil_plot );
   grid->enableYMin( true );
   grid->enableY   ( true );
   grid->setMajPen( QPen( US_GuiSettings::plotMajGrid(), 0, Qt::DashLine ) );
   grid->setMinPen( QPen( US_GuiSettings::plotMinGrid(), 0, Qt::DotLine  ) );

   // Set up the picker for mouse down, moves and up
   QwtPlotPicker* pick = new US_PlotPicker( equil_plot );
   pick->setRubberBand( QwtPicker::CrossRubberBand );
   connect( pick, SIGNAL( cMouseDown(   const QwtDoublePoint& ) ),
                  SLOT(   pMouseDown(  const QwtDoublePoint& ) ) );
   connect( pick, SIGNAL( cMouseUp(     const QwtDoublePoint& ) ),
                  SLOT(   pMouseUp(    const QwtDoublePoint& ) ) );
   connect( pick, SIGNAL( cMouseDrag(   const QwtDoublePoint& ) ),
                  SLOT(   pMouseMoved( const QwtDoublePoint& ) ) );

   if ( scedits[ jsscn ].edited )
   {
      radl    = scedits[ jsscn ].rad_lo;
      radr    = scedits[ jsscn ].rad_hi;
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
   << " edited" << scedits[jsscn].edited;

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

   for ( int jj = 0; jj < nrpts; jj++ )
   {
      double rv = edata->radius( jj );

      if ( rv >= radl  &&  rv <= radr )
      {
         double vv     = edata->value( isc, jj );
         ra[ count   ] = rv;
         va[ count++ ] = vv;
         rlo           = min( rlo, rv );
         rhi           = max( rhi, rv );
         vlo           = min( vlo, vv );
         vhi           = max( vhi, vv );
      }
   }

   cRadLo = rlo;
   cRadHi = rhi;
DbgLv(1) << "EdataPlot:  cRadLo cRadHi" << cRadLo << cRadHi;
DbgLv(1) << "EdataPlot:  dr0 drn" << edata->radius(0) << edata->radius(nrpts-1);
DbgLv(1) << "EdataPlot:   ra0 rak" << ra[0] << ra[count-1];
DbgLv(1) << "EdataPlot:   va0 vak" << va[0] << va[count-1];
DbgLv(1) << "EdataPlot:    count" << count;
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

   QwtSymbol sym;
   sym.setStyle( QwtSymbol::Ellipse );
   sym.setPen  ( QPen( Qt::blue ) );
   sym.setBrush( QBrush( Qt::yellow ) );
   sym.setSize ( 10 );

   QwtPlotCurve* curve = us_curve( equil_plot, "Equil Data" );
   curve->setStyle( QwtPlotCurve::NoCurve );
   curve->setSymbol( sym );
   curve->setData( ra, va, count );

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
   QwtSymbol symu;
   symu.setStyle( QwtSymbol::Ellipse );
   symu.setPen  ( QPen( Qt::blue ) );
   symu.setBrush( QBrush( Qt::yellow ) );  // Unedited yellow
   symu.setSize ( 10 );
   QwtSymbol syme;
   syme.setStyle( QwtSymbol::Ellipse );
   syme.setPen  ( QPen( Qt::white ) );
   syme.setBrush( QBrush( Qt::red ) );     // Edited red
   syme.setSize ( 10 );

   QwtPlotCurve* curvu = us_curve( equil_plot, "Equil Data" );
   curvu->setStyle( QwtPlotCurve::NoCurve );
   curvu->setSymbol( symu );
   QwtPlotCurve* curve = us_curve( equil_plot, "Edited Data" );
   curve->setStyle( QwtPlotCurve::NoCurve );
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

   curvu->setData( ru, vu, countu );
   curve->setData( re, ve, counte );

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

DbgLv(1) << "pMouseDown pos" << p.x() << p.y();
}

// Respond to mouse button up (after move)
void US_GlobalEquil::pMouseUp( const QwtDoublePoint& p )
{
DbgLv(1) << "pMouseUp pos" << p.x() << p.y();
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
   int jsscn = tw_equiscns->currentRow();
   scedits[ jsscn ].edited  = true;
   scedits[ jsscn ].rad_lo  = sRadLo;
   scedits[ jsscn ].rad_hi  = sRadHi;
DbgLv(1) << "pMouseUp sRadLo sRadHi" << sRadLo << sRadHi
   << " cRadLo cRadHi" << cRadLo << cRadHi << " mLowerH" << mLowerH;

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
//DbgLv(1) << "pMouseMoved pos" << p.x() << p.y();
}

void US_GlobalEquil::assign_scanfit()
{
   EqScanFit scanfit;
   scanfits.clear();
   QStringList channs;
   
   for ( int jes = 0; jes < scedits.size(); jes++ )
   {
      int    jed   = scedits[ jes ].dsindex;
      int    jer   = scedits[ jes ].speedx;
      int    jds   = scedits[ jes ].sscanx - 1;
      edata        = &dataList[ jed ];
      QString trip = triples[ jed ];
      QString chan = trip.section( "/", 1, 1 ).simplified();

      if ( ! channs.contains( chan ) )
         channs << chan;

      scanfit.scanFit    = false;
      scanfit.autoExcl   = false;
      scanfit.limsModd   = false;
      scanfit.points     = edata->x.size();
      scanfit.nbr_posr   = 0;
      scanfit.nbr_negr   = 0;
      scanfit.runs       = 0;
      scanfit.start_ndx  = 0;
      scanfit.stop_ndx   = scanfit.points - 1;
      scanfit.cell       = trip.section( "/", 0, 0 ).simplified().toInt();
      scanfit.channel    = channs.indexOf( chan ) + 1;
      scanfit.lambda     = trip.section( "/", 2, 2 ).simplified().toInt();
      scanfit.meniscus   = edata->meniscus;
      scanfit.baseline   = edata->baseline;
      scanfit.pathlen    = 1.2;
      scanfit.density    = DENS_20W;
      scanfit.tempera    = edata->scanData[  jds ].temperature;
      scanfit.rpm        = edata->speedData[ jer ].speed;

      scanfit.xvs     .clear();
      scanfit.yvs     .clear();
      scanfit.amp_vals.clear();
      scanfit.amp_ndxs.clear();
      scanfit.amp_rngs.clear();
      scanfit.amp_fits.clear();
      scanfit.amp_bnds.clear();
      scanfit.extincts.clear();
      scanfit.integral.clear();

      scanfits << scanfit;
   }
}

void US_GlobalEquil::setup_runfit()
{
   runfit.nbr_comps    = 1;
   runfit.nbr_assocs   = 0;
   runfit.nbr_runs     = 0;
   runfit.runs_percent = 0.0;
   runfit.runs_expect  = 0.0;
   runfit.runs_vari    = 0.0;

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
         runfit.nbr_comps    = (int)aud_params[ 1 ];
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

   runfit.mw_vals  .fill(   0.0, runfit.nbr_comps );
   runfit.mw_ndxs  .fill(     0, runfit.nbr_comps );
   runfit.mw_rngs  .fill(   0.0, runfit.nbr_comps );
   runfit.mw_fits  .fill( false, runfit.nbr_comps );
   runfit.mw_bnds  .fill( false, runfit.nbr_comps );
   runfit.vbar_vals.fill(  0.72, runfit.nbr_comps );
   runfit.vbar_ndxs.fill(     0, runfit.nbr_comps );
   runfit.vbar_rngs.fill( 0.144, runfit.nbr_comps );
   runfit.vbar_fits.fill( false, runfit.nbr_comps );
   runfit.vbar_bnds.fill( false, runfit.nbr_comps );
   runfit.viri_vals.fill(   0.0, runfit.nbr_comps );
   runfit.viri_ndxs.fill(     0, runfit.nbr_comps );
   runfit.viri_rngs.fill(   0.0, runfit.nbr_comps );
   runfit.viri_fits.fill( false, runfit.nbr_comps );
   runfit.viri_bnds.fill( false, runfit.nbr_comps );

   for ( int ii = 0; ii < runfit.nbr_assocs; ii++ )
   {
      runfit.eq_vals[ ii ] = 0.0;
      runfit.eq_ndxs[ ii ] = 0;
      runfit.eq_rngs[ ii ] = 0.0;
      runfit.eq_fits[ ii ] = false;
      runfit.eq_bnds[ ii ] = false;
   }
}

