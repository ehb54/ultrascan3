//! file us_globalequil.cpp
#include <QApplication>
#include "us_globalequil.h"
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

   QBoxLayout* mainLayout = new QHBoxLayout( this );
   mainLayout->setSpacing         ( 2 );
   mainLayout->setContentsMargins ( 2, 2, 2, 2 );

   // Left Column
   QVBoxLayout* leftLayout     = new QVBoxLayout;
   QVBoxLayout* rightLayout    = new QVBoxLayout;

   QGridLayout* dataSelLayout  = new QGridLayout;
   QGridLayout* scnListLayout  = new QGridLayout;
   QGridLayout* modlFitLayout  = new QGridLayout;
   QGridLayout* paramLayout    = new QGridLayout;
   QGridLayout* statusLayout   = new QGridLayout;

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

   QLabel*      lb_equiscns  = us_banner( tr( "List of available Equilibrium"
                                              " Scans:" ) );
   QTextEdit*   te_equiscns  = us_textedit();
   QFont font( US_GuiSettings::fontFamily(),
               US_GuiSettings::fontSize(),
               QFont::Bold );
   QFontMetrics fm( font );
   int fontHeight = fm.lineSpacing();
   te_equiscns->setMaximumHeight( fontHeight * 15 + 12 );

   row     = 0;
   scnListLayout->addWidget( lb_equiscns, row++, 0, 1, 2 );
   scnListLayout->addWidget( te_equiscns, row,   0, 5, 2 );
   row    += 5;

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


   // Right Column

   // Simulation plot

   QBoxLayout* plot = new US_Plot( equilibrium_plot, 
         tr( "Experiment Equilibrium Data" ),
         tr( "Radius" ),
         tr( "Absorbance (280 nm)" ) );
   us_grid( equilibrium_plot );
   
   equilibrium_plot->setMinimumSize( 600, 400 );
   equilibrium_plot->setAxisScale( QwtPlot::yLeft  , 0.1, 0.601 );
   equilibrium_plot->setAxisScale( QwtPlot::xBottom, 5.9, 6.2 );

   QLabel*      lb_status    = us_label(    tr( "Status/Information:" ) );
                le_status    = us_lineedit( tr( "Please select an edited"
                                                " Equilibrium Dataset with"
                                                " \"Load Experiment\"" ) );
   QLabel*      lb_currmodl  = us_label(    tr( "Current Model:"      ) );
                le_currmodl  = us_lineedit( tr( "-- none selected --" ) );
   QLabel*      lb_mxfringe  = us_label(    tr( "Max. OD/Fringe:"     ) );
                le_mxfringe  = us_lineedit( tr( "0.90" ) );
   QLabel*      lb_mxfnotes  = us_label(    tr( "(set to zero to inactivate"
                                                " high conc. limits)"  ) );
   le_status  ->setAlignment( Qt::AlignCenter );
   le_currmodl->setAlignment( Qt::AlignCenter );

   row     = 0;
   statusLayout ->addWidget( lb_status,   row,   0, 1, 2 );
   statusLayout ->addWidget( le_status,   row++, 2, 1, 4 );
   statusLayout ->addWidget( lb_currmodl, row,   0, 1, 2 );
   statusLayout ->addWidget( le_currmodl, row++, 2, 1, 4 );
   statusLayout ->addWidget( lb_mxfringe, row,   0, 1, 2 );
   statusLayout ->addWidget( le_mxfringe, row,   2, 1, 1 );
   statusLayout ->addWidget( lb_mxfnotes, row++, 3, 1, 3 );

   rightLayout->addLayout( plot );
   rightLayout->addLayout( statusLayout );
   rightLayout->setStretchFactor( plot,        10 );
   rightLayout->setStretchFactor( statusLayout, 2 );

   mainLayout->addLayout( leftLayout  ); 
   mainLayout->addLayout( rightLayout ); 
   mainLayout->setStretchFactor( leftLayout,   3 );
   mainLayout->setStretchFactor( rightLayout,  5 );
}

void US_GlobalEquil::load( void )
{
qDebug() << "LOAD()";
pb_details ->setEnabled( true );
pb_view    ->setEnabled( true );
pb_unload  ->setEnabled( true );
pb_scdiags ->setEnabled( true );
pb_ckscfit ->setEnabled( true );
pb_conchist->setEnabled( true );
pb_resetsl ->setEnabled( true );
pb_selModel->setEnabled( true );
pb_modlCtrl->setEnabled( true );
pb_fitcntrl->setEnabled( true );
pb_loadFit ->setEnabled( true );
pb_monCarlo->setEnabled( true );
pb_floatPar->setEnabled( true );
pb_initPars->setEnabled( true );

   excludedScans.clear();
   dataList     .clear();
   rawList      .clear();
   triples      .clear();

   dataLoaded  = false;
   buffLoaded  = false;
   dataLatest  = ck_edlast->isChecked();
   dbdisk      = dkdb_cntrls->db() ? US_Disk_DB_Controls::DB
                                   : US_Disk_DB_Controls::Disk;

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
qDebug() << " workingDir" << workingDir;
qDebug() << " dataList size" << dataList.size();
for ( int jj=0;jj<triples.size();jj++ ) qDebug() << "  " << triples[jj];

   qApp->processEvents();

   edata       = &dataList[ 0 ];
   dataLoaded  = true;
}

void US_GlobalEquil::details( void )
{
qDebug() << "DETAILS()";
   QString runID  = edata->runID;

   US_RunDetails2 dialog( rawList, runID, workingDir, triples );
   dialog.exec();
   qApp->processEvents();
}

void US_GlobalEquil::view_report( void )
{ qDebug() << "VIEW_REPORT()"; }
void US_GlobalEquil::unload( void )
{ qDebug() << "UNLOAD()"; }
void US_GlobalEquil::scan_diags( void )
{ qDebug() << "SCAN_DIAGS()"; }
void US_GlobalEquil::check_scan_fit( void )
{ qDebug() << "CHECK_SCAN_FIT()"; }
void US_GlobalEquil::conc_histogram( void )
{ qDebug() << "CONC_HISTOGRAM()"; }
void US_GlobalEquil::reset_scan_lims( void )
{ qDebug() << "RESET_SCAN_LIMS()"; }
void US_GlobalEquil::load_model( void )
{ qDebug() << "LOAD_MODEL()"; }
void US_GlobalEquil::new_project_name( const QString& newpname )
{ qDebug() << "NEW_PROJECT_NAME()" << newpname; }
void US_GlobalEquil::select_model( void )
{ qDebug() << "SELECT_MODEL()"; }
void US_GlobalEquil::model_control( void )
{ qDebug() << "MODEL_CONTROL()"; }
void US_GlobalEquil::fitting_control( void )
{ qDebug() << "FITTING_CONTROL()"; }
void US_GlobalEquil::load_fit( void )
{ qDebug() << "LOAD_FIT()"; }
void US_GlobalEquil::monte_carlo( void )
{ qDebug() << "MONTE_CARLO()"; }
void US_GlobalEquil::float_params( void )
{ qDebug() << "FLOAT_PARAMS()"; }
void US_GlobalEquil::init_params( void )
{ qDebug() << "INIT_PARAMS()"; }
void US_GlobalEquil::scan_select( double newscan )
{ qDebug() << "SCAN_SELECT()" << newscan; }

void US_GlobalEquil::close_all( void )
{
qDebug() << "CLOSE_ALL()";
   close();
}

void US_GlobalEquil::update_disk_db( bool dbaccess )
{
   if ( dbaccess )
   {
      dkdb_cntrls->set_db();
      dbdisk    = US_Disk_DB_Controls::DB;
   }

   else
   {
      dkdb_cntrls->set_disk();
      dbdisk    = US_Disk_DB_Controls::Disk;
   }
}

