//! \file us_fematch.cpp

#include <QApplication>
#include <QtSvg>

#include "us_fematch.h"
#include "us_thread_worker.h"
#include "us_license_t.h"
#include "us_license.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_gui_util.h"
#include "us_matrix.h"
#include "us_constants.h"
#include "us_solution_vals.h"
#include "us_solution_gui.h"
#include "us_passwd.h"
#include "us_data_loader.h"
#include "us_util.h"
#include "us_investigator.h"
#include "us_loadable_noise.h"
#include "us_lamm_astfvm.h"
#include "us_report.h"
#include "us_sleep.h"

#define MIN_NTC   25

// main program
int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );

   #include "main1.inc"

   // License is OK.  Start up.
   
   US_FeMatch w;
   w.show();                   //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication
}

// US_FeMatch class constructor
US_FeMatch::US_FeMatch() : US_Widgets()
{
   setObjectName( "US_FeMatch" );

   int local  = US_Settings::default_data_location();
   dbg_level  = US_Settings::us_debug();
   // Insure working etc is populated with color maps
   clean_etc_dir();

   // set up the GUI
   setPalette( US_GuiSettings::frameColor() );
   setWindowTitle(
      tr( "Compare Experimental Data to Sums of Finite Element Solutions" ) );

   mainLayout      = new QHBoxLayout( this );
   mainLayout->setSpacing        ( 2 );
   mainLayout->setContentsMargins( 2, 2, 2, 2 );

   leftLayout      = new QVBoxLayout();
   rightLayout     = new QVBoxLayout();
   
   analysisLayout  = new QGridLayout();
   runInfoLayout   = new QGridLayout();
   parameterLayout = new QGridLayout();
   controlsLayout  = new QGridLayout();
   buttonLayout    = new QHBoxLayout();
   progressLayout  = new QHBoxLayout();

   leftLayout->addLayout( analysisLayout  );
   leftLayout->addLayout( runInfoLayout   );
   leftLayout->addLayout( parameterLayout );
   leftLayout->addLayout( controlsLayout  );
   leftLayout->addStretch();
   leftLayout->addLayout( buttonLayout    );
   leftLayout->addLayout( progressLayout  );

   // Analysis buttons
   dkdb_cntrls  = new US_Disk_DB_Controls( local );
   pb_load      = us_pushbutton( tr( "Load Experiment" ) );
   pb_details   = us_pushbutton( tr( "Run Details" ) );
   QLayout* lo_edit
                = us_checkbox(   tr( "Latest Data Edit" ), ck_edit, true );
   pb_distrib   = us_pushbutton( tr( "s20,W Distribution" ) );
   pb_loadmodel = us_pushbutton( tr( "Load Model" ) );
   pb_simumodel = us_pushbutton( tr( "Simulate Model" ) );
   pb_view      = us_pushbutton( tr( "View Data Report" ) );
   pb_save      = us_pushbutton( tr( "Save Data" ) );
   //ck_edit     ->setChecked( true );

   connect( dkdb_cntrls,  SIGNAL( changed(      bool ) ),
            this,         SLOT( update_disk_db( bool ) ) );
   connect( pb_load,      SIGNAL( clicked() ),
            this,         SLOT(   load() ) );
   connect( pb_details,   SIGNAL( clicked() ),
            this,         SLOT(   details() ) );
   connect( pb_distrib,   SIGNAL( clicked() ),
            this,         SLOT(   distrib_type() ) );
   connect( pb_loadmodel, SIGNAL( clicked() ),
            this,         SLOT(   load_model()  ) );
   connect( pb_simumodel, SIGNAL( clicked() ),
            this,         SLOT(   simulate_model()  ) );
   connect( pb_view,      SIGNAL( clicked() ),
            this,         SLOT(   view_report() ) );
   connect( pb_save,      SIGNAL( clicked() ),
            this,         SLOT(   save_data() ) );

   pb_load     ->setEnabled( true );
   pb_details  ->setEnabled( false );
   ck_edit     ->setEnabled( true );
   pb_distrib  ->setEnabled( false );
   pb_loadmodel->setEnabled( false );
   pb_simumodel->setEnabled( false );
   pb_view     ->setEnabled( false );
   pb_save     ->setEnabled( false );

   QLabel* lb_splot   = us_label ( tr( "Simulation Plot:" ) );
   int row  = 0;
   analysisLayout->addWidget( pb_load,      row,   0 );
   analysisLayout->addWidget( pb_details,   row++, 1 );
   analysisLayout->addLayout( lo_edit,      row,   0 );
   analysisLayout->addLayout( dkdb_cntrls,  row++, 1 );
   analysisLayout->addWidget( lb_splot,     row,   0 );
   analysisLayout->addWidget( pb_distrib,   row++, 1 );
   analysisLayout->addWidget( pb_loadmodel, row,   0 );
   analysisLayout->addWidget( pb_simumodel, row++, 1 );
   analysisLayout->addWidget( pb_view,      row,   0 );
   analysisLayout->addWidget( pb_save,      row++, 1 );

   // Run info
   QLabel* lb_info    = us_banner( tr( "Information for this Run" ) );
   QLabel* lb_triples = us_banner( tr( "Cell / Channel / Wavelength" ) );
   QLabel* lb_id      = us_label ( tr( "Run ID / Edit ID:" ) );
   QLabel* lb_temp    = us_label ( tr( "Average Temperature:" ) );

   le_id      = us_lineedit( "", -1, true );
   le_temp    = us_lineedit( "", -1, true );

   QFont font( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() );
   QFontMetrics fm( font );
   int fontHeight = fm.lineSpacing();

   te_desc    = us_textedit();
   te_desc->setMaximumHeight( fontHeight * 1 + 12 );  // Add for border
   us_setReadOnly( te_desc, true );

   lw_triples = us_listwidget();
   lw_triples->setMaximumHeight( fontHeight * 2 + 12 );

   row      = 0;
   runInfoLayout->addWidget( lb_info   , row++, 0, 1, 4 );
   runInfoLayout->addWidget( lb_id     , row,   0, 1, 1 );
   runInfoLayout->addWidget( le_id     , row++, 1, 1, 3 );
   runInfoLayout->addWidget( lb_temp   , row,   0, 1, 1 );
   runInfoLayout->addWidget( le_temp   , row++, 1, 1, 3 );
   runInfoLayout->addWidget( te_desc   , row,   0, 2, 4 ); row += 2;
   runInfoLayout->addWidget( lb_triples, row++, 0, 1, 4 );
   runInfoLayout->addWidget( lw_triples, row++, 0, 5, 4 );

   // Parameters

   density      = DENS_20W;
   viscosity    = VISC_20W;
   compress     = 0.0;
   manual       = false;
   pb_solution  = us_pushbutton( tr( "Solution" ) );
   QLabel* lb_density   = us_label( tr( "Density" ) );
   QLabel* lb_viscosity = us_label( tr( "Viscosity" ) );
   QLabel* lb_vbar      = us_label( tr( "Vbar" ) );
   QLabel* lb_compress  = us_label( tr( "Compressibility" ) );
   le_solution  = us_lineedit( tr( "(Experiment's solution)" ) );
   le_density   = us_lineedit( QString::number( density,   'f', 6 ) );
   le_viscosity = us_lineedit( QString::number( viscosity, 'f', 5 ) );
   le_vbar      = us_lineedit( "0.7200" );
   le_compress  = us_lineedit( "0.0"     );
   lb_rmsd      = us_label     ( tr( "RMSD:"  ) );
   le_rmsd      = us_lineedit( "0.0", -1, true );
   le_variance  = us_lineedit( "0.0", -1, true );
   QFontMetrics fme( lb_compress->font() );
   int pwid = fme.width( lb_compress->text() + 6 );
   int lwid = pwid * 3 / 4;
   pb_solution->setEnabled( false );
   lb_vbar    ->setMinimumWidth( pwid );
   le_vbar    ->setMinimumWidth( lwid );
   lb_compress->setMinimumWidth( pwid );
   le_compress->setMinimumWidth( lwid );

   QLabel* lb_experiment   = us_banner( tr( "Experimental Parameters (at 20" ) 
      + DEGC + "):" );
   QLabel* lb_variance     = us_label ( tr( "Variance:" ) );

   connect( pb_solution,  SIGNAL( clicked()      ),
            this,         SLOT(   get_solution() ) );

   pb_advanced = us_pushbutton( tr( "Advanced Analysis Controls" ) );
   pb_plot3d   = us_pushbutton( tr( "3D Plot"       ) );
   pb_plotres  = us_pushbutton( tr( "Residual Plot" ) );
   pb_advanced->setEnabled( false );

   connect( pb_advanced, SIGNAL( clicked()  ),
            this,        SLOT(   advanced() ) );
   connect( pb_plot3d,   SIGNAL( clicked()  ),
            this,        SLOT(   plot3d()   ) );
   connect( pb_plotres,  SIGNAL( clicked()  ),
            this,        SLOT(   plotres()  ) );

   pb_plot3d ->setEnabled( false );
   pb_plotres->setEnabled( false );

   density   = DENS_20W;
   viscosity = VISC_20W;
   vbar      = TYPICAL_VBAR;
   compress  = 0.0;
   manual    = false;
//Hardwire compressibility to zero and make read-only, for now
le_compress->setText( "0.0" );
us_setReadOnly( le_compress, true );

   row      = 0;
   parameterLayout->addWidget( lb_experiment   , row++, 0, 1, 4 );
   parameterLayout->addWidget( pb_solution     , row,   0, 1, 1 );
   parameterLayout->addWidget( le_solution     , row++, 1, 1, 3 );
   parameterLayout->addWidget( lb_density      , row,   0, 1, 1 );
   parameterLayout->addWidget( le_density      , row,   1, 1, 1 );
   parameterLayout->addWidget( lb_viscosity    , row,   2, 1, 1 );
   parameterLayout->addWidget( le_viscosity    , row++, 3, 1, 1 );
   parameterLayout->addWidget( lb_vbar         , row,   0, 1, 1 );
   parameterLayout->addWidget( le_vbar         , row,   1, 1, 1 );
   parameterLayout->addWidget( lb_compress     , row,   2, 1, 1 );
   parameterLayout->addWidget( le_compress     , row++, 3, 1, 1 );
   parameterLayout->addWidget( lb_variance     , row,   0, 1, 1 );
   parameterLayout->addWidget( le_variance     , row,   1, 1, 1 );
   parameterLayout->addWidget( lb_rmsd         , row,   2, 1, 1 );
   parameterLayout->addWidget( le_rmsd         , row++, 3, 1, 1 );
   parameterLayout->addWidget( pb_advanced     , row++, 0, 1, 4 );
   parameterLayout->addWidget( pb_plot3d       , row,   0, 1, 2 );
   parameterLayout->addWidget( pb_plotres      , row++, 2, 1, 2 );

   // Scan Controls

   QLabel* lb_scan    = us_banner( tr( "Scan Control"       ) ); 
   QLabel* lb_from    = us_label ( tr( "Scan focus from:" ) );
   ct_from            = us_counter( 3, 0, 500, 1 );
   QLabel* lb_to      = us_label ( tr( "to:"   ) );
   ct_to              = us_counter( 3, 0, 500, 1 );
   pb_exclude         = us_pushbutton( tr( "Exclude Scan Range" ) );
   pb_reset_exclude   = us_pushbutton( tr( "Reset Scan Range" ) );
   ct_from->setValue( 0 );
   ct_to  ->setValue( 0 );

   pb_exclude      ->setEnabled( false );
   pb_reset_exclude->setEnabled( false );

   connect( ct_from,    SIGNAL( valueChanged( double ) ),
            this,       SLOT  ( exclude_from( double ) ) );
   connect( ct_to,      SIGNAL( valueChanged( double ) ),
            this,       SLOT  ( exclude_to  ( double ) ) );
   connect( pb_exclude,       SIGNAL( clicked()        ),
            this,             SLOT  ( exclude()        ) );
   connect( pb_reset_exclude, SIGNAL( clicked()        ),
            this,             SLOT  ( reset_excludes() ) );

   row      = 0;
   controlsLayout->addWidget( lb_scan           , row++, 0, 1, 4 );
   controlsLayout->addWidget( lb_from           , row,   0, 1, 2 );
   controlsLayout->addWidget( ct_from           , row++, 2, 1, 2 );
   controlsLayout->addWidget( lb_to             , row,   0, 1, 2 );
   controlsLayout->addWidget( ct_to             , row++, 2, 1, 2 );
   controlsLayout->addWidget( pb_exclude        , row,   0, 1, 2 );
   controlsLayout->addWidget( pb_reset_exclude  , row++, 2, 1, 2 );
   
   // Plots
   plotLayout1 = new US_Plot( data_plot1,
            tr( "Residuals" ),
            tr( "Radius (cm)" ),
            tr( "OD Difference" ) );

   plotLayout2 = new US_Plot( data_plot2,
            tr( "Velocity Data" ),
            tr( "Radius (cm)" ),
            tr( "Absorbance" ) );

   data_plot1->setCanvasBackground( Qt::black );
   data_plot2->setCanvasBackground( Qt::black );
   data_plot1->setMinimumSize( 560, 240 );
   data_plot2->setMinimumSize( 560, 240 );

   // Standard buttons
   pb_reset    = us_pushbutton( tr( "Reset" ) );
   pb_help     = us_pushbutton( tr( "Help"  ) );
   pb_close    = us_pushbutton( tr( "Close" ) );

   buttonLayout->addWidget( pb_reset    );
   buttonLayout->addWidget( pb_help     );
   buttonLayout->addWidget( pb_close    );

   connect( pb_reset,    SIGNAL( clicked() ),
            this,        SLOT(   reset()     ) );
   connect( pb_close,    SIGNAL( clicked() ),
            this,        SLOT(   close_all() ) );
   connect( pb_help,     SIGNAL( clicked() ),
            this,        SLOT(   help()      ) );

   // Progress label and bar
   QLabel* lb_progress = us_label( tr( "% Complete:" ) );
   progress            = us_progressBar( 0, 100, 0 );
   lb_progress->setAlignment( Qt::AlignCenter );

   progressLayout->addWidget( lb_progress );
   progressLayout->addWidget( progress    );

   rightLayout->addLayout( plotLayout1 );
   rightLayout->addLayout( plotLayout2 );
   rightLayout->setStretchFactor( plotLayout1, 2 );
   rightLayout->setStretchFactor( plotLayout2, 3 );

   mainLayout->addLayout( leftLayout  );
   mainLayout->addLayout( rightLayout );
   mainLayout->setStretchFactor( leftLayout, 3 );
   mainLayout->setStretchFactor( rightLayout, 5 );

   dataLoaded = false;
   buffLoaded = false;
   haveSim    = false;
   mfilter    = "";
   resids.clear();
   rbmapd     = 0;
   eplotcd    = 0;
   resplotd   = 0;
   bmd_pos    = this->pos() + QPoint( 100, 100 );
   epd_pos    = this->pos() + QPoint( 200, 200 );
   rpd_pos    = this->pos() + QPoint( 300, 400 );

   ti_noise.count = 0;
   ri_noise.count = 0;

   adv_vals[ "simpoints" ] = "200";
   adv_vals[ "bndvolume" ] = "0.015";
   adv_vals[ "parameter" ] = "0";
   adv_vals[ "modelnbr"  ] = "0";
   adv_vals[ "meshtype"  ] = "ASTFEM";
   adv_vals[ "gridtype"  ] = "Moving";
   adv_vals[ "modelsim"  ] = "model";

   sdata          = &wsdata;

   setMaximumSize( qApp->desktop()->size() - QSize( 40, 40 ) );
}

// public function to get pointer to edit data
US_DataIO::EditedData*      US_FeMatch::fem_editdata() { return edata;     }

// public function to get pointer to list of excluded scans
QList< int >*               US_FeMatch::fem_excllist() { return &excludedScans;}

// public function to get pointer to sim data
US_DataIO::RawData*         US_FeMatch::fem_simdata()  { return sdata;     }

// public function to get pointer to load model
US_Model*                   US_FeMatch::fem_model()    { return &model;    }

// public function to get pointer to TI noise
US_Noise*                   US_FeMatch::fem_ti_noise() { return &ti_noise; }

// public function to get pointer to RI noise
US_Noise*                   US_FeMatch::fem_ri_noise() { return &ri_noise; }

// public function to get pointer to resid bitmap diag
QPointer< US_ResidsBitmap > US_FeMatch::fem_resbmap()  { return rbmapd;    }

// Load data
void US_FeMatch::load( void )
{
   QString     file;
   QStringList files;
   QStringList parts;
   lw_triples->  disconnect();
   lw_triples->  clear();
   dataList.     clear();
   rawList.      clear();
   excludedScans.clear();
   triples.      clear();
   speed_steps  .clear();

   dataLoaded = false;
   buffLoaded = false;
   haveSim    = false;
   dataLatest = ck_edit->isChecked();
   int local  = dkdb_cntrls->db() ? US_Disk_DB_Controls::DB
                                  : US_Disk_DB_Controls::Disk;

DbgLv(1) << "LD: open dialog";
   US_DataLoader* dialog =
      new US_DataLoader( dataLatest, local, rawList, dataList,
            triples, workingDir, QString( "velocity" ) );

   connect( dialog, SIGNAL( changed(      bool ) ),
            this,   SLOT( update_disk_db( bool ) ) );
   connect( dialog, SIGNAL( progress(     const QString ) ),
            this,   SLOT( set_progress(   const QString ) ) );

DbgLv(1) << "LD: exec dialog";
   if ( dialog->exec() != QDialog::Accepted )  return;
DbgLv(1) << "LD: local" << local << US_Disk_DB_Controls::DB;

   edata     = &dataList[ 0 ];
   runID     = edata->runID;

   // Get speed steps from disk or DB experiment
   if ( local == US_Disk_DB_Controls::DB )
   {  // Fetch the speed steps for the experiment from the database
      workingDir = tr( "(database)" );
DbgLv(1) << "LD:  DB IN  runID" << runID;

      US_Passwd   pw;
      US_DB2*     dbP    = new US_DB2( pw.getPasswd() );
      QStringList query;
      QString     expID;
      int         idExp  = 0;
      query << "get_experiment_info_by_runID"
            << runID
            << QString::number( US_Settings::us_inv_ID() );
      dbP->query( query );

      if ( dbP->lastErrno() == US_DB2::OK )
      {
        dbP->next();
        idExp              = dbP->value( 1 ).toInt();
        US_SimulationParameters::speedstepsFromDB( dbP, idExp, speed_steps );
DbgLv(1) << "SS: ss count" << speed_steps.count() << "idExp" << idExp;
if (speed_steps.count()>0 )
DbgLv(1) << "SS:  ss0 w2tfirst w2tlast timefirst timelast"
   << speed_steps[0].w2t_first << speed_steps[0].w2t_last
   << speed_steps[0].time_first << speed_steps[0].time_last;
      }
   }

   else
   {  // Read run experiment file and parse out speed steps
      workingDir = workingDir.section( workingDir.left( 1 ), 4, 4 );
      workingDir = workingDir.left( workingDir.lastIndexOf( "/" ) );
DbgLv(1) << "LD:  Disk IN  runID" << runID << "workingDir" << workingDir;

      QString expfpath = workingDir + "/" + runID + "."
                       + edata->dataType + ".xml";
DbgLv(1) << "LD: expf path" << expfpath;
      QFile xfi( expfpath )
         ;
      if ( xfi.open( QIODevice::ReadOnly ) )
      {  // Read and parse "<speedstep>" lines in the XML
         QXmlStreamReader xmli( &xfi );

         while ( ! xmli.atEnd() )
         {
            xmli.readNext();

            if ( xmli.isStartElement()  &&  xmli.name() == "speedstep" )
            {
               SP_SPEEDPROFILE  sp;
               US_SimulationParameters::speedstepFromXml( xmli, sp );
               speed_steps << sp;
DbgLv(1) << "LD:  sp: rotspeed" << sp.rotorspeed << "t1" << sp.time_first;
            }
         }

         xfi.close();
      }
   }

   exp_steps  = ( speed_steps.count() > 0 );  // Flag any multi-step experiment

   qApp->processEvents();

   QFont font( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() );
   QFontMetrics fm( font );
   int fontHeight = fm.lineSpacing();
   int ntriples   = triples.size();
   lw_triples->setMaximumHeight( fontHeight * min( ntriples, 4 ) + 12 );

   for ( int ii = 0; ii < ntriples; ii++ )
      lw_triples->addItem( triples.at( ii ) );

   allExcls.fill( excludedScans, ntriples );

   scanCount = edata->scanData.size();
   double avgTemp = edata->average_temperature();

   // set ID, description, and avg temperature text
   le_id  ->setText( edata->runID + " / " + edata->editID );
   te_desc->setText( edata->description );
   le_temp->setText( QString::number( avgTemp, 'f', 1 ) + " " + DEGC );

   lw_triples->setCurrentRow( 0 );
   connect( lw_triples, SIGNAL( currentRowChanged( int ) ),
                        SLOT(   new_triple(        int ) ) );

   dataLoaded = true;
   haveSim    = false;

   update( 0 );

   pb_solution ->setEnabled( true );
   pb_details  ->setEnabled( true );
   pb_loadmodel->setEnabled( true );
   pb_exclude  ->setEnabled( true );
   mfilter     = QString( "=e" );

   ct_from->disconnect();
   ct_from->setValue( 0 );

   connect( ct_from, SIGNAL( valueChanged( double ) ),
            this,    SLOT(   exclude_from( double ) ) );

   bmd_pos    = this->pos() + QPoint( 100, 100 );
   epd_pos    = this->pos() + QPoint( 200, 200 );
   rpd_pos    = this->pos() + QPoint( 300, 300 );

}

// Details
void US_FeMatch::details( void )
{
   US_RunDetails2* dialog
      = new US_RunDetails2( rawList, runID, workingDir, triples );

   dialog->move( this->pos() + QPoint( 100, 100 ) );
   dialog->exec();
   qApp->processEvents();

   delete dialog;
}

// Update based on selected triples row
void US_FeMatch::update( int drow )
{
   edata          = &dataList[ drow ];
   scanCount      = edata->scanData.size();
   runID          = edata->runID;
   haveSim        = false;
   le_id->  setText( runID + " / " + edata->editID );

   double avgTemp = edata->average_temperature();
   le_temp->setText( QString::number( avgTemp, 'f', 1 )
         + " " + DEGC );
   te_desc->setText( edata->description );

   excludedScans  = allExcls[ drow ];

   ct_from->setMaxValue( scanCount - excludedScans.size() );
   ct_to  ->setMaxValue( scanCount - excludedScans.size() );
   ct_from->setStep( 1.0 );
   ct_to  ->setStep( 1.0 );

   // Set up solution values implied from experimental data
   QString solID;
   QString bufid;
   QString bguid;
   QString bdesc;
   QString bdens = le_density  ->text();
   QString bvisc = le_viscosity->text();
   QString bcomp = le_compress ->text();
   QString bmanu = manual ? "1" : "0";
   QString svbar = le_vbar     ->text();
   bool    bufvl = false;

   QString errmsg;
   US_Passwd pw;
   US_DB2* dbP = dkdb_cntrls->db() ?
                 new US_DB2( pw.getPasswd() ) : 0;
   bufvl = US_SolutionVals::values( dbP, edata, solID, svbar, bdens,
                                    bvisc, bcomp, bmanu, errmsg );
//Hardwire compressibility to zero, for now
bcomp="0.0";

   if ( bufvl )
   {
      buffLoaded  = false;
      bcomp       = QString::number( bcomp.toDouble() );
      le_density  ->setText( bdens );
      le_viscosity->setText( bvisc );
      le_compress ->setText( bcomp );
      buffLoaded  = true;
      density     = bdens.toDouble();
      viscosity   = bvisc.toDouble();
      compress    = bcomp.toDouble();
      manual      = ( !bmanu.isEmpty()  &&  bmanu == "1" );

      if ( solID.isEmpty() )
      {
         QMessageBox::warning( this, tr( "Solution/Buffer Fetch" ),
            tr( "Empty solution ID value!" ) );
      }

      else if ( solID.length() < 36  &&  dbP != NULL )
      {
         solution_rec.readFromDB( solID.toInt(), dbP );
      }

      else
      {
         solution_rec.readFromDisk( solID );
      }
   
      le_solution ->setText( solution_rec.solutionDesc );
      vbar          = US_Math2::calcCommonVbar( solution_rec, 20.0 );
      svbar         = QString::number( vbar );
      le_vbar     ->setText( svbar );
   }

   else
   {
      QMessageBox::warning( this, tr( "Solution/Buffer Fetch" ),
            errmsg );
      solution_rec.commonVbar20 = vbar;
      le_solution ->setText( tr( "( ***Undefined*** )" ) );
   }

   ti_noise.count = 0;
   ri_noise.count = 0;

   data_plot();

   if ( dbP != NULL )
   {
      delete dbP;
      dbP = NULL;
   }

   pb_view     ->setEnabled( false );
   pb_save     ->setEnabled( false );
   pb_simumodel->setEnabled( false );
   pb_distrib  ->setEnabled( false );
   pb_advanced ->setEnabled( false );
   pb_plot3d   ->setEnabled( false );
   pb_plotres  ->setEnabled( false );
   pb_distrib  ->setText   ( tr( "s20,W Distribution" ) );
DbgLv(1) << "Fem:Upd: manual" << manual << solution_rec.buffer.manual;

   if ( eplotcd != 0 )
   {
      epd_pos  = eplotcd->pos();
      eplotcd->close();
      eplotcd  = 0;
   }

   if ( resplotd != 0 )
   {
      rpd_pos  = resplotd->pos();
      resplotd->close();
      resplotd = 0;
   }

   if ( rbmapd != 0 )
   {
      bmd_pos  = rbmapd->pos();
      rbmapd->close();
      rbmapd   = 0;
   }
}


// Data plot
void US_FeMatch::data_plot( void )
{
   data_plot2->detachItems();

   if ( !dataLoaded )
      return;

   int drow    = lw_triples->currentRow();
   edata       = &dataList[ drow ];
   QString                            dataType = tr( "Absorbance" );
   if ( edata->dataType == "RI" )     dataType = tr( "Intensity" );
   if ( edata->dataType == "WI" )     dataType = tr( "Intensity" );
   if ( edata->dataType == "IP" )     dataType = tr( "Interference" );
   if ( edata->dataType == "FI" )     dataType = tr( "Fluourescence" );
   data_plot2->setTitle(
      tr( "Velocity Data for " ) + edata->runID );
   data_plot2->setAxisTitle( QwtPlot::yLeft,
      dataType + tr( " at " ) + edata->wavelength + tr( " nm" ) );
   data_plot2->setAxisTitle( QwtPlot::xBottom,
      tr( "Radius (cm)" ) );

   data_plot2->clear();
   us_grid( data_plot2 );

   int     scan_nbr  = 0;
   int     from      = (int)ct_from->value();
   int     to        = (int)ct_to  ->value();

   int     points    = edata->pointCount();
   int     count     = points;

   if ( haveSim )
   {
      count     = sdata->pointCount();
      count     = points > count ? points : count;
   }

   QVector< double > vecr( count );
   QVector< double > vecv( count );
   double* rr        = vecr.data();
   double* vv        = vecv.data();

   QString       title; 
   QwtPlotCurve* cc;
   QPen          pen_red(  Qt::red );
   QPen          pen_plot( US_GuiSettings::plotCurve() );

   // Calculate basic parameters for other functions
   double avgTemp     = edata->average_temperature();
   solution.density   = le_density  ->text().toDouble();
   solution.viscosity = le_viscosity->text().toDouble();
   solution.manual    = manual;
   solution.vbar20    = le_vbar     ->text().toDouble();
   solution.vbar      = US_Math2::calcCommonVbar( solution_rec, avgTemp );

   US_Math2::data_correction( avgTemp, solution );

   dscan           = &edata->scanData.last();
   int    point    = US_DataIO::index( dataList[ drow ].xvalues,
                                       dataList[ drow ].baseline );
   point           = ( point < 5 ) ? 5 : point;
   double baseline = 0.0;

   for ( int jj = point - 5; jj < point + 6; jj++ )
      baseline       += dscan->rvalues[ jj ];

   baseline       /= 11.0;

   // Draw curves
   for ( int ii = 0; ii < scanCount; ii++ )
   {
      if ( excludedScans.contains( ii ) ) continue;

      scan_nbr++;
      bool highlight = ( scan_nbr >= from  &&  scan_nbr <= to );

      dscan          = &edata->scanData[ ii ];

      for ( int jj = 0; jj < points; jj++ )
      {
         rr[ jj ] = edata->radius(     jj );
         vv[ jj ] = edata->value ( ii, jj );
      }

      title = tr( "Curve " ) + QString::number( ii ) + tr( " in range" );
      cc    = us_curve( data_plot2, title );

      if ( highlight )
         cc->setPen( pen_red );
      else
         cc->setPen( pen_plot );
         
      cc->setData( rr, vv, points );
   }

   // Plot simulation
   if ( haveSim )
   {
      double rl = edata->radius( 0 );
      double vh = edata->value( scanCount - 1, points - 1 );
      rl       -= 0.05;
      vh       += ( vh - edata->value( 0, 0 ) ) * 0.05;
      double rnoi = 0.0;
      double tnoi = 0.0;
      bool   have_ri = ri_noise.count > 0;
      bool   have_ti = ti_noise.count > 0;
DbgLv(1) << "  RL" << rl << "  VH" << vh;
int nscan=scanCount;
int nconc=sdata->pointCount();
DbgLv(1) << "    sdata ns nc " << nscan << nconc;
DbgLv(1) << "      sdata->x0" << sdata->radius(0);
DbgLv(1) << "      sdata->xN" << sdata->radius(nconc-1);
DbgLv(1) << "      sdata->c00" << sdata->value(0,0);
DbgLv(1) << "      sdata->c0N" << sdata->value(0,nconc-1);
DbgLv(1) << "      sdata->cM0" << sdata->value(nscan-1,0);
DbgLv(1) << "      sdata->cMN" << sdata->value(nscan-1,nconc-1);
      double rmsd = 0.0;
      int    kpts = 0;

      for ( int ii = 0; ii < scanCount; ii++ )
      {
         if ( excludedScans.contains( ii ) ) continue;

         points    = sdata->pointCount();
DbgLv(2) << "      II POINTS" << ii << points;
         count     = 0;
         double rp = 0.0;
         double vp = 0.0;
         double da = 0.0;
         rnoi      = have_ri ? ri_noise.values[ ii ] : 0.0;

         for ( int jj = 0; jj < points; jj++ )
         {  // accumulate coordinates of simulation curve
            tnoi      = have_ti ? ti_noise.values[ jj ] : 0.0;
            rp        = sdata->radius( jj );
            vp        = sdata->value( ii, jj ) + rnoi + tnoi;
            da        = edata->value( ii, jj );
            rmsd     += sq( da - vp );
            kpts++;
DbgLv(3) << "       JJ rr vv" << jj << rr << vv;

            if ( rp > rl )
            {
               rr[ count   ] = rp;
               vv[ count++ ] = vp;
            }
         }
         title   = "SimCurve " + QString::number( ii );
         cc      = us_curve( data_plot2, title );
         cc->setPen( pen_red );
         cc->setData( rr, vv, count );
DbgLv(1) << "Sim plot scan count" << ii << count
 << "  r0 v0 rN vN" << rr[0] << vv[0 ] << rr[count-1] << vv[count-1];
      }

      rmsd       /= (double)kpts;
DbgLv(1) << "     Sim plot rmsd kpts" << rmsd << kpts;
      le_variance->setText( QString::number( rmsd ) );
      rmsd        = sqrt( rmsd );
      le_rmsd    ->setText( QString::number( rmsd ) );
   }

   else
   {  // No simulation exists yet
      data_plot1->detachItems();
      data_plot1->clear();
      data_plot1->setTitle( tr( "Residuals" ) );
      data_plot1->replot();
   }

   data_plot2->replot();

   return;
}

// Save the report and image data
void US_FeMatch::save_data( void )
{
   if ( eplotcd == 0  ||  eplotcd->data_3dplot() == 0 )
   {  // if no 3d plot control up,  ask what user wants for 3d plot save
      if ( eplotcd == 0 )
         eplotcd = new US_PlotControlFem( this, &model );
      eplotcd->move( epd_pos );
      eplotcd->show();
      eplotcd->do_3dplot();

      QMessageBox mBox;
      mBox.addButton( tr( "Save" ), QMessageBox::RejectRole );
      QPushButton* bSkip = mBox.addButton( tr( "Skip" ),
            QMessageBox::NoRole );
      QPushButton* bEdit = mBox.addButton( tr( "Edit" ),
            QMessageBox::YesRole    );
      mBox.setDefaultButton( bEdit );
      mBox.setWindowTitle  ( tr( "3D Plot Save?" ) );
      mBox.setTextFormat   ( Qt::RichText );
      mBox.setText         ( tr(
         "You selected Save Data without a 3D Plot already displayed.<br>"
         "Do you want a 3D Plot to be saved with the other reports?<ul>"
         "<li><b>Save</b> to include the 3D Plot in the report (AS IS);</li>"
         "<li><b>Skip</b> to exclude the 3D Plot from report saves;</li>"
         "<li><b>Edit</b> to abort save for now so that you can edit<br>"
         "the 3D Plot and re-select Save Data when ready.</li></ul>" ) );
      mBox.setIcon         ( QMessageBox::Information );

      mBox.exec();

      if ( mBox.clickedButton() == bEdit )
      {
         QMessageBox::information( this, tr( "Edit 3D Plot" ),
            tr( "You may now edit the 3D Plot.\n"
                "When it is ready, re-select 'Save Data'" ) );
         eplotcd->do_3dplot();
         return;
      }

      else if ( mBox.clickedButton() == bSkip )
      {
         eplotcd->close();
         eplotcd = NULL;
      }
   }
   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
QDateTime time0=QDateTime::currentDateTime();

   QStringList files;
   int drow = lw_triples->currentRow();
   mkdir( US_Settings::reportDir(), edata->runID );
   QString tripnode = QString( triples.at( drow ) ).replace( " / ", "" );
   QString basename = US_Settings::reportDir() + "/" + edata->runID + "/"
      + text_model( model, 0 ) + "." + tripnode + ".";
   QString htmlFile = basename + "report.html";
   QFile rep_f( htmlFile );
   if ( ! rep_f.open( QIODevice::WriteOnly | QIODevice::Text ) )
      return;

   QTextStream ts( &rep_f );

   // save the report to a file
   write_report( ts );

   rep_f.close();
   update_filelist( files, htmlFile );

   const QString svgext( ".svgz" );
   const QString pngext( ".png" );
   const QString csvext( ".csv" );
   QString img01File = basename + "velocity"   + svgext;
   QString img02File = basename + "residuals"  + pngext;
   QString img03File = basename + "s_distrib"  + svgext;
   QString img04File = basename + "mw_distrib" + svgext;
   QString img05File = basename + "D_distrib"  + svgext;
   QString img06File = basename + "ff0_vs_s"   + svgext;
   QString img07File = basename + "ff0_vs_mw"  + svgext;
   QString img08File = basename + "D_vs_s"     + svgext;
   QString img09File = basename + "D_vs_mw"    + svgext;
   QString img10File = basename + "3dplot"     + pngext;
   QString img11File = basename + "rbitmap"    + pngext;
   QString img12File = basename + "tinoise"    + svgext;
   QString img13File = basename + "rinoise"    + svgext;
   QString mdistFile = basename + "mdistr_tab" + csvext;

   if ( !cnstvb )
   {
      img06File.replace( "ff0", "vbar" );
      img07File.replace( "ff0", "vbar" );
   }
DbgLv(1) << "cnstvb" << cnstvb << "img06File" << img06File;

   // Save image files from main window
   write_plot( img01File, data_plot2 );
   update_filelist( files, img01File );

   int     p1type = type_distrib() - 1;
           p1type = ( p1type < 0 ) ? 9 : p1type;
   QString                   p1file = img02File;
   if ( p1type == 0 )        p1file = img03File;
   else if ( p1type == 1 )   p1file = img04File;
   else if ( p1type == 2 )   p1file = img05File;
   else if ( p1type == 3 )   p1file = img06File;
   else if ( p1type == 4 )   p1file = img07File;
   else if ( p1type == 5 )   p1file = img06File;
   else if ( p1type == 6 )   p1file = img07File;
   else if ( p1type == 7 )   p1file = img08File;
   else if ( p1type == 8 )   p1file = img09File;
DbgLv(1) << "p1type" << p1type << "p1file" << p1file;

   write_plot( p1file, data_plot1 );

   if ( p1type != 9 )
   {
      distrib_plot_resids();
      write_plot( img02File, data_plot1 );
   }
   update_filelist( files, img02File );

   if ( p1type != 0 )
   {
      distrib_plot_stick( 0 );
      write_plot( img03File, data_plot1 );
   }
   update_filelist( files, img03File );

   if ( p1type != 1 )
   {
      distrib_plot_stick( 1 );
      write_plot( img04File, data_plot1 );
   }
   update_filelist( files, img04File );

   if ( p1type != 2 )
   {
      distrib_plot_stick( 2 );
      write_plot( img05File, data_plot1 );
   }
   update_filelist( files, img05File );

   if ( p1type != 3  &&  p1type != 5 )
   {
      distrib_plot_2d( ( cnstvb ? 3 : 5 ) );
      write_plot( img06File, data_plot1 );
   }
   update_filelist( files, img06File );

   if ( p1type != 4  &&  p1type != 6 )
   {
      distrib_plot_2d( ( cnstvb ? 4 : 6 ) );
      write_plot( img07File, data_plot1 );
   }
   update_filelist( files, img07File );

   if ( p1type != 7 )
   {
      distrib_plot_2d(    7 );
      write_plot( img08File, data_plot1 );
   }
   update_filelist( files, img08File );

   if ( p1type != 8 )
   {
      distrib_plot_2d(    8 );
      write_plot( img09File, data_plot1 );
   }
   update_filelist( files, img09File );

DbgLv(1) << "(9)p1type" << p1type;
   // Restore upper plot displayed before saves
   if ( p1type == 9 )       distrib_plot_resids();
   else if ( p1type < 3 )   distrib_plot_stick( p1type );
   else                     distrib_plot_2d( p1type );

   // Save 3-d plot
   if ( eplotcd != 0 )
   {
      write_plot( img10File, NULL );
      update_filelist( files, img10File );
   }

   // save residual bitmap
   write_plot( img11File, NULL );
   update_filelist( files, img11File );

   // save any noise plots
   if ( ti_noise.count > 0 )
   {
      if ( resplotd == 0 )
      {
         resplotd = new US_ResidPlotFem( this );
         resplotd->move( rpd_pos );
         resplotd->show();
      }

      resplotd->set_plot( 1 );
      QwtPlot* nois_plot = resplotd->rp_data_plot2();
      write_plot( img12File, nois_plot );
      update_filelist( files, img12File );
   }

   if ( ri_noise.count > 0 )
   {
      if ( resplotd == 0 )
      {
         resplotd = new US_ResidPlotFem( this );
         resplotd->move( rpd_pos );
         resplotd->show();
      }

      resplotd->set_plot( 2 );
      QwtPlot* nois_plot = resplotd->rp_data_plot2();
      write_plot( img13File, nois_plot );
      update_filelist( files, img13File );
   }

   // Create the model distributions table CSV
   model_table( mdistFile );
   update_filelist( files, mdistFile );

   // report the files created
   QString umsg     = tr( "In directory " )
                    + basename.left( basename.lastIndexOf( "/" ) )
                    + tr( " ,\nwrote:\n" );

   for ( int ii = 0; ii < files.length(); ii++ )
   {
      QString fname = files[ ii ];
      umsg          = umsg + "    "
                           + fname.mid( fname.lastIndexOf( "/" ) + 1 ) + "\n";
   }

QDateTime time1=QDateTime::currentDateTime();
   if ( dkdb_cntrls->db() )
   {  // Copy report files to the database
      reportFilesToDB( files );

      umsg = umsg + tr( "\nFiles were also saved to the database.\n" );
   }
QDateTime time2=QDateTime::currentDateTime();
int etim1=time0.msecsTo(time1);
int etim2=time1.msecsTo(time2);
int etimt=etim1+etim2;
int et1pc=(etim1*100)/etimt;
int et2pc=(etim2*100)/etimt;
DbgLv(1) << "SAVE-FILES: local ms" << etim1 << "=" << et1pc << "%";
DbgLv(1) << "SAVE-FILES: DB    ms" << etim2 << "=" << et2pc << "%";

   QApplication::restoreOverrideCursor();
   QMessageBox::information( this, tr( "Successfully Written" ), umsg );
}

// View the report text
void US_FeMatch::view_report( )
{
   QString mtext;
   QTextStream ts( &mtext );

   // generate the report file
   write_report( ts );

   // display the report dialog
   US_Editor* editd = new US_Editor( US_Editor::DEFAULT, true, "", this );
   editd->setWindowTitle( tr( "Report:  FE Match Model Simulation" ) );
   editd->move( this->pos() + QPoint( 100, 100 ) );
   editd->resize( 780, 700 );
   editd->e->setFont( QFont( US_GuiSettings::fontFamily(),
                             US_GuiSettings::fontSize() ) );
   editd->e->setHtml( mtext );
   editd->show();
}

// Slot to handle a change in Exclude-From
void US_FeMatch::exclude_from( double from )
{
   double to = ct_to->value();

   if ( to < from )
   {
      ct_to->disconnect();
      ct_to->setValue( from );

      connect( ct_to,   SIGNAL( valueChanged( double ) ),
               this,    SLOT  ( exclude_to  ( double ) ) );
   }

   data_plot();
}

// Slot to handle a change in Exclude-To
void US_FeMatch::exclude_to( double to )
{
   double from = ct_from->value();

   if ( from > to )
   {
      ct_from->disconnect();
      ct_from->setValue( to );

      connect( ct_from, SIGNAL( valueChanged( double ) ),
               this,    SLOT  ( exclude_from( double ) ) );
   }

   data_plot();
}

// Exclude scans
void US_FeMatch::exclude( void )
{
   double from          = ct_from->value();
   double to            = ct_to  ->value();
   int    displayedScan = 1;
   int    drow          = lw_triples->currentRow();
          edata         = &dataList[ drow ];
   int    totalScans    = edata->scanData.size();

   for ( int ii = 0; ii < totalScans; ii++ )
   {
      if ( excludedScans.contains( ii ) )
         continue;

      if ( displayedScan >= from  &&  displayedScan <= to )
         excludedScans << ii;

      displayedScan++;
   }

   ct_to->setValue( 0 );   // resets both counters and replots

   ct_from->setMaxValue( totalScans - excludedScans.size() );
   ct_to  ->setMaxValue( totalScans - excludedScans.size() );

   allExcls[ drow ]     = excludedScans;

   data_plot();

   pb_reset_exclude->setEnabled( true );
}

// Respond to click of current type of distribution plot
void US_FeMatch::distrib_type( )
{
   const char* dptyp[] = 
   {
      "s20,w distribution",
      "MW distribution",
      "D20,w distribution",
      "f_f0 vs s20,w",
      "f_f0 vs MW",
      "vbar vs s20,w",
      "vbar vs MW",
      "D20,w vs s20,w",
      "D20,w vs MW",
      "Residuals"
   };
   const int ndptyp = sizeof( dptyp ) / sizeof( dptyp[0] );

   int itype = type_distrib();

   // get pointer to data for use by plot routines
   edata   = &dataList[ lw_triples->currentRow() ];

   // set push button text to next type
   int ii  = itype + 1;
   ii      = ( ii == ndptyp ) ? 0 : ii;
   ii      = (  cnstvb && ii == 5 ) ? 7 : ii;
   ii      = ( !cnstvb && ii == 3 ) ? 5 : ii;
   pb_distrib->setText( QString( dptyp[ ii ] ) );
 
   switch( itype )
   {
      case 0:     // s20,w distribution
      case 1:     // MW distribution
      case 2:     // D20,w distribution
         distrib_plot_stick( itype );  // bar (1-d) plot
         break;
      case 3:     // f_f0 vs s20,w
      case 4:     // f_f0 vs MW
      case 5:     // vbar vs s20,w
      case 6:     // vbar vs MW
      case 7:     // D20,w vs s20,w
      case 8:     // D20,w vs MW
         distrib_plot_2d(    itype );  // 2-d plot
         break;
      case 9:     // Residuals
         distrib_plot_resids();        // residuals plot
         break;
   }
}

// Do stick type distribution plot
void US_FeMatch::distrib_plot_stick( int type )
{
   QString pltitle = tr( "Run " ) + edata->runID + tr( ": Cell " )
      + edata->cell + " (" + edata->wavelength + " nm)";
   QString xatitle;
   QString yatitle = tr( "Rel. Concentr." );

   if ( type == 0 )
   {
      pltitle = pltitle + tr( "\ns20,W Distribution" );
      xatitle = tr( "Corrected Sedimentation Coefficient" );
   }

   else if ( type == 1 )
   {
      pltitle = pltitle + tr( "\nMW Distribution" );
      xatitle = tr( "Molecular Weight (Dalton)" );
   }

   else if ( type == 2 )
   {
      pltitle = pltitle + tr( "\nD20,W Distribution" );
      xatitle = tr( "D20,W (cm^2/sec)" );
   }

   data_plot1->detachItems();

   data_plot1->setTitle(                       pltitle );
   data_plot1->setAxisTitle( QwtPlot::yLeft,   yatitle );
   data_plot1->setAxisTitle( QwtPlot::xBottom, xatitle );
   data_plot1->clear();

   QwtPlotGrid*  data_grid = us_grid(  data_plot1 );
   QwtPlotCurve* data_curv = us_curve( data_plot1, "distro" );

   int     dsize  = model_loaded.components.size();
   QVector< double > vecx( dsize );
   QVector< double > vecy( dsize );
   double* xx     = vecx.data();
   double* yy     = vecy.data();
   double  xmin   = 1.0e30;
   double  xmax   = -1.0e30;
   double  ymin   = 1.0e30;
   double  ymax   = -1.0e30;
   double  xval;
   double  yval;
   double  rdif;

   for ( int jj = 0; jj < dsize; jj++ )
   {
      xval     = ( type == 0 ) ? model_loaded.components[ jj ].s :
               ( ( type == 1 ) ? model_loaded.components[ jj ].mw :
                                 model_loaded.components[ jj ].D );
      yval     = model_loaded.components[ jj ].signal_concentration;
      xx[ jj ] = xval;
      yy[ jj ] = yval;
      xmin     = min( xval, xmin );
      xmax     = max( xval, xmax );
      ymin     = min( yval, ymin );
      ymax     = max( yval, ymax );
   }

   rdif   = ( xmax - xmin ) / 20.0;
   xmin  -= rdif;
   xmax  += rdif;
   rdif   = ( ymax - ymin ) / 20.0;
   ymin  -= rdif;
   ymax  += rdif;
   xmin   = ( type == 0 ) ? xmin : max( xmin, 0.0 );
   ymin   = max( ymin, 0.0 );

   data_grid->enableYMin( true );
   data_grid->enableY(    true );
   data_grid->setMajPen(
      QPen( US_GuiSettings::plotMajGrid(), 0, Qt::DashLine ) );

   data_curv->setData(  xx, yy, dsize );
   data_curv->setPen(   QPen( Qt::yellow, 3, Qt::SolidLine ) );
   data_curv->setStyle( QwtPlotCurve::Sticks );

   data_plot1->setAxisAutoScale( QwtPlot::xBottom );
   data_plot1->setAxisAutoScale( QwtPlot::yLeft   );
   data_plot1->setAxisScale( QwtPlot::xBottom, xmin, xmax );
   data_plot1->setAxisScale( QwtPlot::yLeft,   ymin, ymax );

   data_plot1->replot();
}

// Do 2d type distribution plot
void US_FeMatch::distrib_plot_2d( int type )
{
   QString pltitle = tr( "Run " ) + edata->runID + tr( ": Cell " )
      + edata->cell + " (" + edata->wavelength + " nm)";
   QString yatitle;
   QString xatitle;

   if ( type == 3 )
   {
      pltitle = pltitle + tr( "\nf/f0 vs Sed. Coeff." );
      yatitle = tr( "Frictional Ratio f/f0" );
      xatitle = tr( "Sedimentation Coefficient s20,W" );
   }

   else if ( type == 4 )
   {
      pltitle = pltitle + tr( "\nf/f0 vs Mol. Weight" );
      yatitle = tr( "Frictional Ratio f/f0" );
      xatitle = tr( "Molecular Weight" );
   }

   else if ( type == 5 )
   {
      pltitle = pltitle + tr( "\nVbar vs Sed. Coeff." );
      yatitle = tr( "Vbar at 20" ) + DEGC;
      xatitle = tr( "Sedimentation Coefficient s20,W" );
   }

   else if ( type == 6 )
   {
      pltitle = pltitle + tr( "\nVbar vs Mol. Weight" );
      yatitle = tr( "Vbar at 20" ) + DEGC;
      xatitle = tr( "Molecular Weight" );
   }

   else if ( type == 7 )
   {
      pltitle = pltitle + tr( "\nDiff. Coeff. vs Sed. Coeff." );
      yatitle = tr( "Diff. Coeff. D20,W" );
      xatitle = tr( "Sedimentation Coefficient s20,W" );
   }

   else if ( type == 8 )
   {
      pltitle = pltitle + tr( "\nDiff. Coeff. vs Molecular Weight" );
      yatitle = tr( "Diff. Coeff. D20,W" );
      xatitle = tr( "Molecular Weight" );
   }

   data_plot1->setTitle(                       pltitle );
   data_plot1->setAxisTitle( QwtPlot::yLeft,   yatitle );
   data_plot1->setAxisTitle( QwtPlot::xBottom, xatitle );

   data_plot1->clear();
   data_plot1->detachItems();

   QwtPlotGrid*  data_grid = us_grid(  data_plot1 );
   QwtPlotCurve* data_curv = us_curve( data_plot1, "distro" );
   QwtSymbol     symbol;

   int     dsize  = model_loaded.components.size();
   QVector< double > vecx( dsize );
   QVector< double > vecy( dsize );
   double* xx     = vecx.data();
   double* yy     = vecy.data();
   double  xmin   = 1.0e30;
   double  xmax   = -1.0e30;
   double  ymin   = 1.0e30;
   double  ymax   = -1.0e30;
   double  xval;
   double  yval;
   double  rdif;

   for ( int jj = 0; jj < dsize; jj++ )
   {
      xval     = ( ( type & 1 ) == 1 ) ? model_loaded.components[ jj ].s :
                                         model_loaded.components[ jj ].mw;

      if ( type < 5 )             yval = model_loaded.components[ jj ].f_f0;
      else if ( type < 7 )        yval = model_loaded.components[ jj ].vbar20;
      else                        yval = model_loaded.components[ jj ].D;

      xx[ jj ] = xval;
      yy[ jj ] = yval;
      xmin     = min( xval, xmin );
      xmax     = max( xval, xmax );
      ymin     = min( yval, ymin );
      ymax     = max( yval, ymax );
   }

   rdif   = ( xmax - xmin ) / 20.0;
   xmin  -= rdif;
   xmax  += rdif;
   rdif   = ( ymax - ymin ) / 20.0;
   ymin  -= rdif;
   ymax  += rdif;
   xmin   = ( type & 1 ) == 1 ? xmin : max( xmin, 0.0 );
   ymin   = max( ymin, 0.0 );

   data_grid->enableYMin( true );
   data_grid->enableY(    true );
   data_grid->setMajPen(
      QPen( US_GuiSettings::plotMajGrid(), 0, Qt::DashLine ) );

   symbol.setStyle( QwtSymbol::Ellipse );
   symbol.setPen(   QPen(   Qt::red    ) );
   symbol.setBrush( QBrush( Qt::yellow ) );
   if ( dsize > 100 )
      symbol.setSize(  5 );
   else if ( dsize > 50 )
      symbol.setSize(  8 );
   else if ( dsize > 20 )
      symbol.setSize( 10 );
   else
      symbol.setSize( 12 );

   data_curv->setStyle(  QwtPlotCurve::NoCurve );
   data_curv->setSymbol( symbol );
   data_curv->setData(   xx, yy, dsize );

   data_plot1->setAxisAutoScale( QwtPlot::xBottom );
   data_plot1->setAxisAutoScale( QwtPlot::yLeft   );
   data_plot1->setAxisScale( QwtPlot::xBottom, xmin, xmax );
   data_plot1->setAxisScale( QwtPlot::yLeft,   ymin, ymax );

   data_plot1->replot();
}

// Do residuals type distribution plot
void US_FeMatch::distrib_plot_resids( )
{
   QString pltitle = tr( "Run " ) + edata->runID + tr( ": Cell " )
      + edata->cell + " (" + edata->wavelength + " nm)" + tr( "\nResiduals" );
   QString yatitle = tr( "OD Difference" );
   QString xatitle = tr( "Radius (cm)" );

   data_plot1->setTitle(     pltitle );
   data_plot1->setAxisTitle( QwtPlot::yLeft,   yatitle );
   data_plot1->setAxisTitle( QwtPlot::xBottom, xatitle );

   data_plot1->clear();
   data_plot1->detachItems();

   QwtPlotGrid*  data_grid = us_grid( data_plot1 );
   QwtPlotCurve* data_curv;
   QwtPlotCurve* line_curv = us_curve( data_plot1, "resids zline" );

   int     dsize  = edata->pointCount();
   QVector< double > vecx( dsize );
   QVector< double > vecy( dsize );
   double* xx     = vecx.data();
   double* yy     = vecy.data();
   double  zx[ 2 ];
   double  zy[ 2 ];
   double  xmin   = 1.0e30;
   double  xmax   = -1.0e30;
   double  ymin   = 1.0e30;
   double  ymax   = -1.0e30;
   double  xval;
   double  yval;
   double  rdif;

   for ( int jj = 0; jj < dsize; jj++ )
   { // accumulate x (radius) values and min,max
      xval     = edata->radius( jj );
      xmin     = min( xval, xmin );
      xmax     = max( xval, xmax );
      xx[ jj ] = xval;
   }

   rdif   = ( xmax - xmin ) / 20.0;  // expand grid range slightly
   xmin  -= rdif;
   xmax  += rdif;
   xmin   = max( xmin, 0.0 );

   for ( int ii = 0; ii < scanCount; ii++ )
   {  // accumulate min,max y (residual) values
      for ( int jj = 0; jj < dsize; jj++ )
      {
         yval     = resids[ ii ][ jj ];
         ymin     = min( yval, ymin );
         ymax     = max( yval, ymax );
      }
   }

   rdif   = ( ymax - ymin ) / 20.0;
   ymin  -= rdif;
   ymax  += rdif;

   data_grid->enableYMin( true );
   data_grid->enableY(    true );
   data_grid->setMajPen(
      QPen( US_GuiSettings::plotMajGrid(), 0, Qt::DashLine ) );

   data_plot1->setAxisAutoScale( QwtPlot::xBottom );
   data_plot1->setAxisAutoScale( QwtPlot::yLeft   );
   data_plot1->setAxisScale( QwtPlot::xBottom, xmin, xmax );
   data_plot1->setAxisScale( QwtPlot::yLeft,   ymin, ymax );

   // draw the red zero line
   zx[ 0 ] = xmin;
   zx[ 1 ] = xmax;
   zy[ 0 ] = 0.0;
   zy[ 1 ] = 0.0;
   line_curv->setPen( QPen( Qt::red ) );
   line_curv->setData( zx, zy, 2 );

   for ( int ii = 0; ii < scanCount; ii++ )
   {  // draw residual dots a scan at a time

      for ( int jj = 0; jj < dsize; jj++ )
      {  // get residuals for this scan
         yy[ jj ] = resids[ ii ][ jj ];
      }

      // plot the residual scatter for this scan
      data_curv = us_curve( data_plot1, "resids " +  QString::number( ii ) );
      data_curv->setPen(    QPen( Qt::yellow ) );
      data_curv->setStyle(  QwtPlotCurve::Dots );
      data_curv->setData(   xx, yy, dsize );
   }

   data_plot1->replot();
}

// Open a dialog with advanced analysis parameters
void US_FeMatch::advanced( )
{
   advdiag = new US_AdvancedFem( &model_loaded, adv_vals,
                                 (QWidget*)this );
   advdiag->show();
}

// open 3d plot dialog
void US_FeMatch::plot3d( )
{
   if ( eplotcd != 0 )
   {
      epd_pos  = eplotcd->pos();
      eplotcd->close();
   }

   eplotcd = new US_PlotControlFem( this, &model_loaded );
   eplotcd->move( epd_pos );
   eplotcd->show();
}

// Open a residual plot dialog
void US_FeMatch::plotres( )
{
   if ( resplotd != 0 )
   {
      rpd_pos  = resplotd->pos();
      resplotd->close();
   }

   resplotd = new US_ResidPlotFem( this );
   resplotd->move( rpd_pos );
   resplotd->show();
}

// Load the model data and detect if it is RA
void US_FeMatch::load_model( )
{
   int      drow    = lw_triples->currentRow();
   QString  mdesc;
   pb_simumodel->setEnabled( false );
   progress->reset();

   // load model
   bool loadDB = dkdb_cntrls->db();

#if 0
   if ( dataList[ drow ].channel == "S" )
   {  // Set up for "manual" model list option for simulated data
      if ( ! mfilter.contains( "=m" ) )
         mfilter     = "=m " + mfilter.replace( "=e", "" ).simplified();
   }
#endif

   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );

   US_ModelLoader dialog( loadDB, mfilter, model,
      mdesc, dataList[ drow ].editGUID );

   connect( &dialog, SIGNAL( changed(      bool ) ),
            this,    SLOT( update_disk_db( bool ) ) );

   dialog.move( this->pos() + QPoint( 200, 200 ) );
   QApplication::restoreOverrideCursor();

   if ( dialog.exec() != QDialog::Accepted )
      return;                     // Cancel:  bail out now

   qApp->processEvents();

//   if ( model.monteCarlo )
//      adjust_mc_model();
DbgLv(1) << "post-Load m,e,r GUIDs" << model.modelGUID << model.editGUID
 << model.requestGUID;
DbgLv(1) << "post-Load loadDB" << dkdb_cntrls->db();

   model_loaded = model;   // save model exactly as loaded

   if ( model.components.size() == 0 )
   {
      QMessageBox::critical( this, tr( "Empty Model" ),
            tr( "Loaded model has ZERO components!" ) );
      return;
   }

   ti_noise.count = 0;
   ri_noise.count = 0;

   // see if there are any noise files to load
   if ( ! model.editGUID.isEmpty() )
      load_noise();

   pb_advanced ->setEnabled( true );
   pb_simumodel->setEnabled( true );
}

// Adjust model components based on buffer, vbar, and temperature
void US_FeMatch::adjust_model()
{
   model              = model_loaded;

   // build model component correction factors
   double avgTemp     = edata->average_temperature();
   double vbar20      = le_vbar     ->text().toDouble();

   solution.density   = le_density  ->text().toDouble();
   solution.viscosity = le_viscosity->text().toDouble();
   solution.manual    = manual;
   solution.vbar20    = vbar20;
   solution.vbar      = US_Math2::calcCommonVbar( solution_rec, avgTemp );
DbgLv(1) << "Fem:Adj: manual" << manual << solution.manual << solution_rec.buffer.manual;

   US_Math2::data_correction( avgTemp, solution );

   double scorrec  = solution.s20w_correction;
   double dcorrec  = solution.D20w_correction;
   // Set constant-vbar flag
   cnstvb         = model.constant_vbar();

   US_Math2::SolutionData sd;
   sd.density      = solution.density;
   sd.viscosity    = solution.viscosity;
   sd.vbar20       = solution.vbar20;
   sd.vbar         = solution.vbar;
   sd.manual       = solution.manual;

   // fill out components values and adjust s,D based on buffer

   for ( int jj = 0; jj < model.components.size(); jj++ )
   {
      US_Model::SimulationComponent* sc = &model.components[ jj ];

      if ( sc->vbar20 == 0.0 )
         sc->vbar20  = vbar20;
 double s0_k = sc->f_f0;
 double s0_s = sc->s;
 double s0_D = sc->D;
 double s0_w = sc->mw;
 double s0_b = sc->vbar20;

      sc->mw      = 0.0;
      sc->f       = 0.0;
      sc->D       = 0.0;

      model.calc_coefficients( *sc );
if ( dbg_level > 0 && jj < 2 ) {
 DbgLv(1) << "AdjMo: 0) s" << s0_s << "k" << s0_k << "D" << s0_D
  << "mw" << s0_w << "vbar20" << s0_b << "  jj" << jj;
 double s1_k = sc->f_f0;
 double s1_s = sc->s;
 double s1_D = sc->D;
 double s1_w = sc->mw;
 double s1_b = sc->vbar20;
 DbgLv(1) << "AdjMo:  1) s" << s1_s << "k" << s1_k << "D" << s1_D
  << "mw" << s1_w << "vbar20" << s1_b;
 double D20w = R * K20 / ( AVOGADRO * 18.0 * M_PI *
   pow(s0_k * VISC_20W / 100.0, 3.0 / 2.0) * 
   sqrt(qAbs(s0_s) * s1_b / (2.0 * (1.0 - s1_b * DENS_20W))));
 sc->mw = sc->f = sc->f_f0 = 0.0;
 sc->D = D20w;
 model.calc_coefficients( *sc );
double s2_k = sc->f_f0;
double s2_s = sc->s;
double s2_D = sc->D;
double s2_w = sc->mw;
double s2_b = sc->vbar20;
 DbgLv(1) << "AdjMo:  2) s" << s2_s << "k" << s2_k << "D" << s2_D
  << "mw" << s2_w << "vbar20" << s2_b;
}

      if ( ! cnstvb )
      {
         sd.vbar20   = sc->vbar20;
         sd.vbar     = US_Math2::adjust_vbar( sd.vbar20, avgTemp );
         US_Math2::data_correction( avgTemp, sd );
         scorrec     = sd.s20w_correction;
         dcorrec     = sd.D20w_correction;
      }

      sc->s      /= scorrec;
      sc->D      /= dcorrec;

      if ( sc->extinction > 0.0 )
         sc->molar_concentration = sc->signal_concentration / sc->extinction;
   }

}

// Compress and average monte carlo model components
void US_FeMatch::adjust_mc_model()
{
   model_loaded       = model;
   int ncomp          = model.components.size();
DbgLv(1) << "AMM: ncomp" << ncomp;

   QStringList mlistn;
   QStringList mlistx;

   // build a set of lists that will enable components sorted by s,k
   for ( int ii = 0; ii < ncomp; ii++ )
   {
      double  sval = model.components[ ii ].s;
      double  kval = model.components[ ii ].f_f0;
      int     isv  = qRound( sval * 1.0e+19 );
      int     ikv  = qRound( kval * 1.0e+06 );
      QString amx  = QString().sprintf( "%09i:%09i:%04i", isv, ikv, ii );

      mlistx << amx;
      mlistn << amx.section( ':', 0, 1 );
if ( ii<3 || (ncomp-ii)<4 )
DbgLv(1) << "AMM:  ii" << ii << " amx" << amx;
   }

   // sort the lists
   mlistn.sort();
   mlistx.sort();
   model.components.clear();

   // re-order, compress, and average the model

   for ( int ii = 0; ii < ncomp; ii++ )
   {
      int    jj    = mlistx[ ii ].section( ':', 2, 2 ).toInt();
      int    kdup  = mlistn.count( mlistn[ ii ] );
      US_Model::SimulationComponent mcomp = model_loaded.components[ jj ];

      if ( kdup == 1 )
      {  // if a single version of this component, output it as is
         model.components << mcomp;
      }

      else
      {  // for multiples find average concentration; use modified component
         double cconc = mcomp.signal_concentration;
DbgLv(1) << "AMM:  ii kdup" << ii << kdup << "  cconc0" << cconc;

         for ( int cc = 1; cc < kdup; cc++ )
         {
            int    kk    = mlistx[ ++ii ].section( ':', 2, 2 ).toInt();
            cconc       += model_loaded.components[ kk ].signal_concentration;
         }

         mcomp.signal_concentration = cconc / (double)kdup;
DbgLv(1) << "AMM:      ii" << ii << " cconc" << cconc;

         model.components << mcomp;
      }
   }
DbgLv(1) << "AMM:  kcomp" << model.components.size();
}

// load noise record(s) if there are any and user so chooses
void US_FeMatch::load_noise( )
{
   QStringList mieGUIDs;  // list of GUIDs of models-in-edit
   QStringList nieGUIDs;  // list of GUIDS:type:index of noises-in-edit
   QString     editGUID  = edata->editGUID;         // loaded edit GUID
   QString     modelGUID = model.modelGUID;         // loaded model GUID
DbgLv(1) << "editGUID  " << editGUID;
DbgLv(1) << "modelGUID " << modelGUID;

   te_desc->setText( tr( "<b>Scanning noise for %1 ...</b>" )
         .arg( triples[ lw_triples->currentRow() ] ) );
   qApp->processEvents();
   US_LoadableNoise lnoise;
   bool loadDB = dkdb_cntrls->db();
   int nenois  = lnoise.count_noise( !loadDB, edata, &model,
         mieGUIDs, nieGUIDs );

for (int jj=0;jj<nenois;jj++)
 DbgLv(1) << " jj nieG" << jj << nieGUIDs.at(jj);

   if ( nenois > 0 )
   {  // There is/are noise(s):  ask user if she wants to load
      US_Passwd pw;
      US_DB2* dbP  = loadDB ? new US_DB2( pw.getPasswd() ) : NULL;

      if ( nenois > 1 )
      {  // more than 1:  get choice from noise loader dialog
         US_NoiseLoader* nldiag = new US_NoiseLoader( dbP,
            mieGUIDs, nieGUIDs, ti_noise, ri_noise, edata );
         nldiag->move( this->pos() + QPoint( 200, 200 ) );
         nldiag->exec();
         qApp->processEvents();

         delete nldiag;
      }

      else
      {  // only 1:  just load it
         QString noiID = nieGUIDs.at( 0 );
         QString typen = noiID.section( ":", 1, 1 );
         noiID         = noiID.section( ":", 0, 0 );

         if ( typen == "ti" )
            ti_noise.load( loadDB, noiID, dbP );

         else
            ri_noise.load( loadDB, noiID, dbP );
      }

      // noise loaded:  insure that counts jive with data
      int ntinois = ti_noise.values.size();
      int nrinois = ri_noise.values.size();
      int nscans  = edata->scanCount();
      int npoints = edata->pointCount();
      int npadded = 0;

      if ( ntinois > 0  &&  ntinois < npoints )
      {  // pad out ti noise values to radius count
         int jj      = ntinois;
         while ( jj++ < npoints )
            ti_noise.values << 0.0;
         ti_noise.count = ti_noise.values.size();
         npadded++;
      }

      if ( nrinois > 0  &&  nrinois < nscans )
      {  // pad out ri noise values to scan count
         int jj      = nrinois;
         while ( jj++ < nscans )
            ri_noise.values << 0.0;
         ri_noise.count = ri_noise.values.size();
         npadded++;
      }

      if ( npadded  > 0 )
      {  // let user know that padding occurred
         QString pmsg;

         if ( npadded == 1 )
            pmsg = tr( "The noise file was padded out with zeroes\n"
                       "in order to match the data range." );
         else
            pmsg = tr( "The noise files were padded out with zeroes\n"
                       "in order to match the data ranges." );

         QMessageBox::information( this, tr( "Noise Padded Out" ), pmsg );
      }

      if ( dbP != NULL )
      {
         delete dbP;
         dbP = NULL;
      }
   }
   te_desc->setText( edata->description );
   qApp->processEvents();
}

// Do model simulation
void US_FeMatch::simulate_model( )
{
   double buoy    = 1.0 - vbar * DENS_20W;

   if ( buoy <= 0.0 )
   {
      QMessageBox::critical( this, tr( "Negative Buoyancy Implied" ),
         tr( "The current Vbar value (%1) implies a buoyancy\n"
             "value (%2) that is non-positive.\n\n"
             "Simulate Model cannot proceed with this value. Click on\n"
             "the <Solution> button and change the Vbar value.\n"
             "Note that the Solution may be accepted without being saved." )
             .arg( vbar ).arg( buoy ) );
      return;
   }

   int    drow    = lw_triples->currentRow();
   US_DataIO::RawData*    rdata   = &rawList[  drow ];
   US_DataIO::EditedData* edata   = &dataList[ drow ];
   int    kscan   = rdata->scanCount();
   int    nscan   = edata->scanCount();
   int    nconc   = edata->pointCount();
   double radlo   = edata->radius( 0 );
   double radhi   = edata->radius( nconc - 1 );
DbgLv(1) << " kscan nscan nconc" << kscan << nscan << nconc;
DbgLv(1) << " radlo radhi" << radlo << radhi;
DbgLv(1) << " baseline plateau" << edata->baseline << edata->plateau;

   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
   adjust_model();

   // Initialize simulation parameters using edited data information
   US_Passwd pw;
   US_DB2* dbP = dkdb_cntrls->db() ? new US_DB2( pw.getPasswd() ) : NULL;     

   simparams.initFromData( dbP, *edata, !exp_steps );
DbgLv(1) << " initFrDat rotorCalID coeffs" << simparams.rotorCalID
   << simparams.rotorcoeffs[0] << simparams.rotorcoeffs[1];
simparams.simpoints = adv_vals[ "simpoints" ].toInt();
DbgLv(1) << " simulation points" << simparams.simpoints;

   simparams.meshType          = US_SimulationParameters::ASTFEM;
   simparams.gridType          = US_SimulationParameters::MOVING;
   simparams.radial_resolution = ( radhi - radlo ) / (double)( nconc - 1 );
   simparams.bottom            = simparams.bottom_position;
   if ( exp_steps )
      simparams.speed_step        = speed_steps;

   QString mtyp = adv_vals[ "meshtype"  ];
   QString gtyp = adv_vals[ "gridtype"  ];
   QString bvol = adv_vals[ "bndvolume" ];
DbgLv(1) << "  meshtype" << mtyp;

   if ( mtyp.contains( "Claverie" ) )
      simparams.meshType = US_SimulationParameters::CLAVERIE;
   else if ( mtyp.contains( "Moving Hat" ) )
      simparams.meshType = US_SimulationParameters::MOVING_HAT;
   else if ( mtyp.contains( "File:"      ) )
      simparams.meshType = US_SimulationParameters::USER;
   else if ( mtyp.contains( "ASTFVM"     ) )
      simparams.meshType = US_SimulationParameters::ASTFVM;

   if ( gtyp.contains( "Constant" ) )
      simparams.gridType = US_SimulationParameters::FIXED;

   simparams.firstScanIsConcentration = false;
   double concval1                    = 0.0;

   if ( simparams.band_forming )
   {
      simparams.band_volume = bvol.toDouble();
      //concval1              = 1.0;
      //simparams.firstScanIsConcentration = true;
   }
   else
      simparams.band_volume = 0.0;
DbgLv(1) << "  duration_hours  " << simparams.speed_step[0].duration_hours;
DbgLv(1) << "  duration_minutes" << simparams.speed_step[0].duration_minutes;
DbgLv(1) << "  delay_hours  " << simparams.speed_step[0].delay_hours;
DbgLv(1) << "  delay_minutes" << simparams.speed_step[0].delay_minutes;

   // Make a simulation copy of the experimental data without actual readings

//   US_AstfemMath::initSimData( *sdata, *edata, 0.0 );
   US_AstfemMath::initSimData( *sdata, *edata, concval1 );

   sdata->cell        = rdata->cell;
   sdata->channel     = rdata->channel;
   sdata->description = rdata->description;
DbgLv(1) << "  sdata->description" << sdata->description;
DbgLv(1) << "   sdata->x0" << sdata->radius(0);
DbgLv(1) << "   sdata->xN" << sdata->radius(nconc-1);
DbgLv(1) << "   rdata->c0" << rdata->value(0,0);
DbgLv(1) << "   rdata->cN" << rdata->value(0,nconc-1);
DbgLv(1) << "   edata->c0" << edata->value(0,0);
DbgLv(1) << "   edata->cN" << edata->value(0,nconc-1);
DbgLv(1) << "   sdata->c00" << sdata->value(0,0);
DbgLv(1) << "   sdata->c0N" << sdata->value(0,nconc-1);
DbgLv(1) << "   sdata->cM0" << sdata->value(nscan-1,0);
DbgLv(1) << "   sdata->cMN" << sdata->value(nscan-1,nconc-1);
DbgLv(1) << " afrsa init";
if ( dbg_level > 1 )
 simparams.save_simparms( US_Settings::etcDir() + "/sp_fematch.xml" );

   start_time = QDateTime::currentDateTime();
   int ncomp  = model.components.size();
   compress   = le_compress->text().toDouble();
   progress->setMaximum( ncomp );
   progress->reset();

   nthread    = US_Settings::threads();
   int ntc    = ( ncomp + nthread - 1 ) / nthread;
   nthread    = ( ntc > MIN_NTC ) ? nthread : 1;
DbgLv(1) << " nthread ntc ncomp" << nthread << ntc << ncomp;

   // Do simulation by several possible ways: 1-/Multi-thread, ASTFEM/ASTFVM
   if ( nthread < 2 )
   {  // Do a single-thread calculation
      if ( model.components[ 0 ].sigma == 0.0  &&
           model.components[ 0 ].delta == 0.0  &&
           model.coSedSolute           <  0.0  &&
           compress                    == 0.0 )
      {  // ASTFEM
         US_Astfem_RSA* astfem_rsa = new US_Astfem_RSA( model, simparams );
   
         connect( astfem_rsa, SIGNAL( current_component( int ) ),
                  this,       SLOT(   update_progress(   int ) ) );
DbgLv(1) << " afrsa calc";
//astfem_rsa->setTimeInterpolation( true );
//astfem_rsa->setTimeCorrection( true );

         astfem_rsa->calculate( *sdata );
      }

      else
      {  // ASTFVM
DbgLv(1) << " afvm calc: sigma delta coSed compress"
 << model.components[ 0 ].sigma << model.components[ 0 ].delta
 << model.coSedSolute << compress;
         US_LammAstfvm *astfvm     = new US_LammAstfvm( model, simparams );

         connect( astfvm,     SIGNAL( comp_progress(     int ) ),
                  this,       SLOT(   update_progress(   int ) ) );

         solution_rec.buffer.compressibility = compress;
         solution_rec.buffer.manual          = manual;
         astfvm->set_buffer( solution_rec.buffer );
         astfvm->calculate(     *sdata );
      }

      show_results();
   }

   else
   {  // Do multi-thread calculations
DbgLv(1) << " USING THREADING";
      solution_rec.buffer.compressibility = compress;
      solution_rec.buffer.manual          = manual;
      tsimdats.clear();
      tmodels .clear();
      kcomps  .clear();
      QList< ThreadWorker* >         tworkers;
      QList< QThreadEx* >            wthreads;

      // Build models for each thread
      for ( int ii = 0; ii < ncomp; ii++ )
      {
         if ( ii < nthread )
         {  // First time through per thread:  get initial model and sim data
            tmodels << model;
            tmodels[ ii ].components.clear();
            US_DataIO::RawData sdat = *sdata;
            tsimdats << sdat;
            kcomps   << 0;
         }

         // Partition thread models from round-robin fetch of components
         int jj = ii % nthread;
         tmodels[ jj ].components << model.components[ ii ];
      }

      thrdone   = 0;
      solution_rec.buffer.manual = manual;

      // Build worker threads and begin running
      for ( int ii = 0; ii < nthread; ii++ )
      {
DbgLv(1) << "Thr-Bld ii" << ii << "model comps"
 << tmodels[ii].components.size();
         ThreadWorker* tworker = new ThreadWorker( tmodels[ ii ], simparams,
               tsimdats[ ii ], solution_rec.buffer, ii );
         QThreadEx*    wthread = new QThreadEx();

         tworker->moveToThread( wthread );
         tworkers << tworker;
         wthreads << wthread;

         connect( wthread, SIGNAL( started()         ),
                  tworker, SLOT  ( calc_simulation() ) );

         connect( tworker, SIGNAL( work_progress  ( int, int ) ),
                  this,    SLOT(   thread_progress( int, int ) ) );
         connect( tworker, SIGNAL( work_complete  ( int )      ),
                  this,    SLOT(   thread_complete( int )      ) );

         wthread->start();
      }
DbgLv(1) << "    +++End Of Thr-St loop";

   }
}

// Show simulation and residual when the simulation is complete
void US_FeMatch::show_results( )
{
   long dur_calc = start_time.msecsTo( QDateTime::currentDateTime() );
   DbgLv(0) << dur_calc << "Msecs. for calculations using"
      << nthread << "thread(s)";

   progress->setValue( progress->maximum() );

   int nscan = sdata->scanCount();
   int nconc = sdata->pointCount();
DbgLv(1) << " afrsa done M N" << nscan << nconc;
DbgLv(1) << "   sdata->x0" << sdata->radius(0);
DbgLv(1) << "   sdata->xN" << sdata->radius(nconc-1);
DbgLv(1) << "   sdata->c00" << sdata->value(0,0);
DbgLv(1) << "   sdata->c0N" << sdata->value(0,nconc-1);
DbgLv(1) << "   sdata->cM0" << sdata->value(nscan-1,0);
DbgLv(1) << "   sdata->cMN" << sdata->value(nscan-1,nconc-1);

   double rmsd = 0.0;
   rmsd        = US_AstfemMath::variance( *sdata, *edata, excludedScans );
   le_variance->setText( QString::number( rmsd ) );
   rmsd        = sqrt( rmsd );
   le_rmsd    ->setText( QString::number( rmsd ) );

   haveSim     = true;
   pb_distrib->setEnabled( true );
   pb_view   ->setEnabled( true );
   pb_save   ->setEnabled( true );
   pb_plot3d ->setEnabled( true );
   pb_plotres->setEnabled( true );

   calc_residuals();             // calculate residuals

   distrib_plot_resids();        // plot residuals

   data_plot();                  // re-plot data+simulation

   if ( rbmapd != 0 )
   {
      bmd_pos  = rbmapd->pos();
      rbmapd->close();
   }

   rbmapd = new US_ResidsBitmap( resids );
   rbmapd->move( bmd_pos );
   rbmapd->show();

   plot3d();

   plotres();
   QApplication::restoreOverrideCursor();
}

// Pare down files list by including only the last-edit versions
QStringList US_FeMatch::last_edit_files( QStringList files )
{
   QStringList ofiles;
   QStringList part;
   QString     file;
   QString     test;
   QString     pfile;
   QString     ptest;
   int         nfi   = files.size();

   // if only one in list, we need do no more
   if ( nfi < 2 )
   {
      return files;
   }

   // make sure files list is in ascending alphabetical order
   files.sort();

   // get first file name and its non-editID parts
   file    = files[ 0 ];
   part    = file.split( "." );
   test    = part[ 0 ] + part[ 3 ] + part[ 4 ] + part[ 5 ];

   // skip all but last of any duplicates (differ only in editID)
   for ( int ii = 1; ii < nfi; ii++ )
   {
      pfile   = file;
      ptest   = test;
      file    = files[ ii ];
      part    = file.split( "." );
      test    = part[ 0 ] + part[ 3 ] + part[ 4 ] + part[ 5 ];

      if ( QString::compare( test, ptest ) != 0 )
      {  // differs by more than just edit, so output previous
         ofiles.append( pfile );
      }
   }

   // output the final
   ofiles.append( file );

   return ofiles;
}

// Set values for component at index
void US_FeMatch::component_values( int /*index*/ )
{
#if 0
   le_sedcoeff->setText( QString::number( model.components[ index ].s ) );
   le_difcoeff->setText( QString::number( model.components[ index ].D ) );
   le_partconc->setText(
      QString::number( model.components[ index ].signal_concentration ) );
   le_moweight->setText(
      QString( "%1 kD,  %2" ).arg( model.components[ index ].mw / 1000.0 )
      .arg( model.components[ index ].f_f0 ) );
#endif
}

// Component number changed
void US_FeMatch::comp_number( double cnbr )
{
   component_values( (int)cnbr - 1 );
}

// Interpolate an sdata y (readings) value for a given x (radius)
double US_FeMatch::interp_sval( double xv, double* sx, double* sy, int ssize )
{
   for ( int jj = 1; jj < ssize; jj++ )
   {
      if ( xv == sx[ jj ] )
      {
         return sy[ jj ];
      }

      if ( xv < sx[ jj ] )
      {  // given x lower than array x: interpolate between point and previous
         int    ii = jj - 1;
         double dx = sx[ jj ] - sx[ ii ];
         double dy = sy[ jj ] - sy[ ii ];
         return ( sy[ ii ] + ( xv - sx[ ii ] ) * dy / dx );
      }
   }

   // given x position not found:  interpolate using last two points
   int    jj = ssize - 1;
   int    ii = jj - 1;
   double dx = sx[ jj ] - sx[ ii ];
   double dy = sy[ jj ] - sy[ ii ];
   return ( sy[ ii ] + ( xv - sx[ ii ] ) * dy / dx );
}

// Write the report HTML text stream
void US_FeMatch::write_report( QTextStream& ts )
{
   ts << html_header( "US_Fematch", text_model( model, 2 ), edata );
//   ts << data_details();
//   ts << hydrodynamics();
//   ts << scan_info();
   ts << distrib_info();
   ts << "  </body>\n</html>\n";
}

// Calculate average baseline absorbance
double US_FeMatch::calc_baseline( int drow ) const
{
   const US_DataIO::EditedData* dd = &dataList[ drow ];
   const US_DataIO::Scan*       ss = &dd->scanData.last();
   int     nn    = US_DataIO::index( dd->xvalues, dd->baseline );
   double  bl    = 0.0;

   for ( int jj = nn - 5; jj < nn + 6; jj++ )
      bl += ss->rvalues[ jj ];

   return ( bl / 11.0 );
}

// Model type text string
QString US_FeMatch::text_model( US_Model model, int width )
{
   QString stitle = model.typeText();
   QString title  = stitle;

   if ( width != 0 )
   {  // long title:  add any suffixes and check need to center
      switch ( (int)model.analysis )
      {
         case (int)US_Model::TWODSA:
         case (int)US_Model::TWODSA_MW:
            title = tr( "2-Dimensional Spectrum Analysis" );
            break;

         case (int)US_Model::GA:
         case (int)US_Model::GA_MW:
            title = tr( "Genetic Algorithm Analysis" );
            break;

         case (int)US_Model::COFS:
            title = tr( "C(s) Analysis" );
            break;

         case (int)US_Model::FE:
            title = tr( "Finite Element Analysis" );
            break;

         case (int)US_Model::PCSA:
            title = tr( "Parametrically Constrained Spectrum Analysis " );

            if ( stitle.contains( "-SL" ) )
               title += tr( "(Straight Line)" );

            else if ( stitle.contains( "-IS" ) )
               title += tr( "(Incr. Sigmoid)" );

            else if ( stitle.contains( "-DS" ) )
               title += tr( "(Decr. Sigmoid)" );

            else if ( stitle.contains( "-HL" ) )
               title += tr( "(Horizontal Line)" );

            break;

         case (int)US_Model::MANUAL:
         default:
            title = tr( "2-Dimensional Spectrum Analysis" );
            break;
      }

      if ( model.associations.size() > 1 )
         title = title + " (RA)";

      if ( model.global == US_Model::MENISCUS )
         title = title + " (Menisc.)";

      else if ( model.global == US_Model::GLOBAL )
         title = title + " (Global)";

      else if ( model.global == US_Model::SUPERGLOBAL )
         title = title + " (S.Glob.)";

      if ( model.monteCarlo )
         title = title + " (MC)";

      if ( width > title.length() )
      {  // long title centered:  center it in fixed-length string
         int lent = title.length();
         int lenl = ( width - lent ) / 2;
         int lenr = width - lent - lenl;
         title    = QString( " " ).repeated( lenl ) + title
                  + QString( " " ).repeated( lenr );
      }
   }

   return title;
}


// Calculate residual absorbance values (data - sim - noise)
void US_FeMatch::calc_residuals()
{
   int     dsize  = edata->pointCount();
   int     ssize  = sdata->pointCount();
   QVector< double > vecxx( dsize );
   QVector< double > vecsx( ssize );
   QVector< double > vecsy( ssize );
   double* xx     = vecxx.data();
   double* sx     = vecsx.data();
   double* sy     = vecsy.data();
   double  yval;
   double  sval;
   //double  rl     = edata->radius( 0 );
   //double  vh     = edata->value( scanCount - 1, dsize - 1 );
   double  rmsd   = 0.0;
   double  tnoi   = 0.0;
   double  rnoi   = 0.0;
   bool    ftin   = ti_noise.count > 0;
   bool    frin   = ri_noise.count > 0;
   bool    matchd = ( dsize == ssize );
   int     kpts   = 0;

   QVector< double > resscan;

   resids.clear();
   resscan.resize( dsize );

   for ( int jj = 0; jj < dsize; jj++ )
   {
      xx[ jj ] = edata->radius( jj );
   }

   for ( int jj = 0; jj < ssize; jj++ )
   {
      sx[ jj ] = sdata->radius( jj );
      if ( sx[ jj ] != xx[ jj ] )  matchd = false;
   }

   for ( int ii = 0; ii < scanCount; ii++ )
   {
      bool usescan = !excludedScans.contains( ii );

      rnoi     = frin ? ri_noise.values[ ii ] : 0.0;

      for ( int jj = 0; jj < ssize; jj++ )
      {
         sy[ jj ] = sdata->value( ii, jj );
      }

      for ( int jj = 0; jj < dsize; jj++ )
      { // Calculate the residuals and the RMSD
         tnoi          = ftin ? ti_noise.values[ jj ] : 0.0;

         if ( matchd )
            sval          = sy[ jj ];
         else
            sval          = interp_sval( xx[ jj ], sx, sy, ssize );

         yval          = edata->value( ii, jj ) - sval - rnoi - tnoi;
         //if ( xx[ jj ] < rl )
         //   yval          = 0.0;

         if ( usescan )
         {
            rmsd         += sq( yval );
            kpts++;
         }

         resscan[ jj ] = yval;
      }

      resids.append( resscan );
   }

   rmsd  /= (double)( kpts );
   le_variance->setText( QString::number( rmsd ) );
   rmsd   = sqrt( rmsd );
   le_rmsd    ->setText( QString::number( rmsd ) );
}

// Slot to make sure all windows and dialogs get closed
void US_FeMatch::close_all()
{
   if ( rbmapd )
      rbmapd->close();

   if ( eplotcd )
      eplotcd->close();

   close();
}

// String to accomplish line indentation
QString US_FeMatch::indent( const int spaces ) const
{
   return ( QString( " " ).leftJustified( spaces, ' ' ) );
}

// Table row HTML with 2 columns
QString US_FeMatch::table_row( const QString& s1, const QString& s2 ) const
{
   return( indent( 6 ) + "<tr><td>" + s1 + "</td><td>" + s2 + "</td></tr>\n" );
}

// Table row HTML with 3 columns
QString US_FeMatch::table_row( const QString& s1, const QString& s2, 
                               const QString& s3 ) const
{
   return ( indent( 6 ) + "<tr><td>" + s1 + "</td><td>" + s2 + "</td><td>" + s3 
            + "</td></tr>\n" );
}

// Table row HTML with 5 columns
QString US_FeMatch::table_row( const QString& s1, const QString& s2,
                               const QString& s3, const QString& s4,
                               const QString& s5 ) const
{
   return ( indent( 6 ) + "<tr><td>" + s1 + "</td><td>" + s2 + "</td><td>" + s3
            + "</td><td>" + s4 + "</td><td>" + s5 + "</td></tr>\n" );
}

// Table row HTML with 7 columns
QString US_FeMatch::table_row( const QString& s1, const QString& s2,
                               const QString& s3, const QString& s4,
                               const QString& s5, const QString& s6,
                               const QString& s7 ) const
{
   return ( indent( 6 ) + "<tr><td>" + s1 + "</td><td>" + s2 + "</td><td>"
            + s3 + "</td><td>" + s4 + "</td><td>" + s5 + "</td><td>"
            + s6 + "</td><td>" + s7 + "</td></tr>\n" );
}

// Compose a report HTML header
QString US_FeMatch::html_header( QString title, QString head1,
      US_DataIO::EditedData* edata )
{
   QString s = QString( "<?xml version=\"1.0\"?>\n" );
   s  += "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\"\n";
   s  += "                      \"http://www.w3.org/TR/xhtml1/DTD"
         "/xhtml1-strict.dtd\">\n";
   s  += "<html xmlns=\"http://www.w3.org/1999/xhtml\""
         " xml:lang=\"en\" lang=\"en\">\n";
   s  += "  <head>\n";
   s  += "    <title> " + title + " </title>\n";
   s  += "    <meta http-equiv=\"Content-Type\" content="
         "\"text/html; charset=iso-8859-1\"/>\n";
   s  += "    <style type=\"text/css\" >\n";
   s  += "      td { padding-right: 1em; }\n";
   s  += "      body { background-color: white; }\n";
   s  += "    </style>\n";
   s  += "  </head>\n  <body>\n";
   s  += "    <h1>" + head1 + "</h1>\n";
   s  += indent( 4 ) + tr( "<h2>Data Report for Run \"" ) + edata->runID;
   s  += "\",<br/>\n" + indent( 4 ) + "&nbsp;" + tr( " Cell " ) + edata->cell;
   s  += tr( ", Channel " ) + edata->channel;
   s  += tr( ", Wavelength " ) + edata->wavelength;
   s  += ",<br/>\n" + indent( 4 ) + "&nbsp;" + tr( " Edited Dataset " );
   s  += edata->editID + "</h2>\n";

   return s;
}

// Compose data details text
QString US_FeMatch::data_details( void ) const
{
   int    drow     = lw_triples->currentRow();
   const US_DataIO::EditedData* d      = &dataList[ drow ];
   double baseline = calc_baseline( drow );
   QString                       dataType = tr( "Absorbance" );
   if ( d->dataType == "RI" )    dataType = tr( "Intensity" );
   if ( d->dataType == "WI" )    dataType = tr( "Intensity" );
   if ( d->dataType == "IP" )    dataType = tr( "Interference" );
   if ( d->dataType == "FI" )    dataType = tr( "Fluourescence" );

   QString s =
      "\n" + indent( 4 ) + tr( "<h3>Detailed Run Information:</h3>\n" )
      + indent( 4 ) + "<table>\n"
      + table_row( tr( "Cell Description:" ), d->description )
      + table_row( tr( "Data Directory:"   ), workingDir )
      + table_row( tr( "Rotor Speed:"      ),  
            QString::number( (int)d->scanData[ 0 ].rpm ) + " rpm" );

   // Temperature data
   double sum     =  0.0;
   double maxTemp = -1.0e99;
   double minTemp =  1.0e99;

   for ( int i = 0; i < d->scanData.size(); i++ )
   {
      double t = d->scanData[ i ].temperature;
      sum += t;
      maxTemp = max( maxTemp, t );
      minTemp = min( minTemp, t );
   }

   QString average = QString::number( sum / d->scanData.size(), 'f', 1 );

   s += table_row( tr( "Average Temperature:" ), average + " " + MLDEGC );

   if ( maxTemp - minTemp <= US_Settings::tempTolerance() )
      s += table_row( tr( "Temperature Variation:" ), tr( "Within tolerance" ) );
   else 
      s += table_row( tr( "Temperature Variation:" ), 
                      tr( "(!) OUTSIDE TOLERANCE (!)" ) );

   // Time data
   double  tcorrec  = US_Math2::time_correction( dataList );
   int minutes = (int)tcorrec / 60;
   int seconds = (int)tcorrec % 60;

   QString m   = ( minutes == 1 ) ? tr( " minute " ) : tr( " minutes " );
   QString sec = ( seconds == 1 ) ? tr( " second"  ) : tr( " seconds"  );
   s += table_row( tr( "Time Correction:" ), 
                   QString::number( minutes ) + m +
                   QString::number( seconds ) + sec );

   double duration = rawList.last().scanData.last().seconds;

   int hours = (int) duration / 3600;
   minutes   = (int) duration / 60 - hours * 60;
   seconds   = (int) duration % 60;

   QString h;
   h   = ( hours   == 1 ) ? tr( " hour "   ) : tr( " hours "   );
   m   = ( minutes == 1 ) ? tr( " minute " ) : tr( " minutes " );
   sec = ( seconds == 1 ) ? tr( " second " ) : tr( " seconds " );

   s += table_row( tr( "Run Duration:" ),
                   QString::number( hours   ) + h + 
                   QString::number( minutes ) + m + 
                   QString::number( seconds ) + sec );

   // Wavelength, baseline, meniscus, range
   s += table_row( tr( "Wavelength:" ), d->wavelength + " nm" )  +
        table_row( tr( "Baseline " ) + dataType + ":",
                   QString::number( baseline,    'f', 6 ) + " OD" ) + 
        table_row( tr( "Meniscus Position:     " ),           
                   QString::number( d->meniscus, 'f', 3 ) + " cm" );

   int    rrx   =  d->xvalues.size() - 1;
   double left  =  d->xvalues[ 0   ];
   double right =  d->xvalues[ rrx ];

   s += table_row( tr( "Edited Data starts at: " ), 
                   QString::number( left,  'f', 3 ) + " cm" ) +
        table_row( tr( "Edited Data stops at:  " ), 
                   QString::number( right, 'f', 3 ) + " cm" );
   s += indent( 4 ) + "</table>\n";
   return s;
}

// Compose hydrodynamics portion of report text
QString US_FeMatch::hydrodynamics( void ) const
{
   // set up hydrodynamics values
   double avgTemp     = le_temp     ->text().section( " ", 0, 0 ).toDouble();
   US_Math2::SolutionData solution = this->solution;
   solution.density   = le_density  ->text().toDouble();
   solution.viscosity = le_viscosity->text().toDouble();
   solution.manual    = manual;
   solution.vbar20    = le_vbar     ->text().toDouble();
   solution.vbar      = US_Math2::calcCommonVbar( (US_Solution&)solution_rec,
                                                  avgTemp );

   US_Math2::data_correction( avgTemp, solution );

   QString s = "\n" + indent( 4 ) + tr( "<h3>Hydrodynamic Settings:</h3>\n" )
               + indent( 4 ) + "<table>\n";
  
   s += table_row( tr( "Viscosity corrected:" ), 
                   QString::number( solution.viscosity, 'f', 5 ) ) +
        table_row( tr( "Viscosity (absolute):" ),
                   QString::number( solution.viscosity_tb, 'f', 5 ) ) +
        table_row( tr( "Density corrected:" ),
                   QString::number( solution.density, 'f', 6 ) + " g/ccm" ) +
        table_row( tr( "Density (absolute):" ),
                   QString::number( solution.density_tb, 'f', 6 ) + " g/ccm" ) +
        table_row( tr( "Vbar:" ), 
                   QString::number( solution.vbar, 'f', 6 ) + " ccm/g" ) +
        table_row( tr( "Vbar corrected for 20 " ) + MLDEGC + ":",
                   QString::number( solution.vbar20, 'f', 6 ) + " ccm/g" ) +
        table_row( tr( "Buoyancy (Water, 20 " ) + MLDEGC + "): ",
                   QString::number( solution.buoyancyw, 'f', 6 ) ) +
        table_row( tr( "Buoyancy (absolute)" ),
                   QString::number( solution.buoyancyb, 'f', 6 ) ) +
        table_row( tr( "Correction Factor (s):" ),
                   QString::number( solution.s20w_correction, 'f', 6 ) ) + 
        table_row( tr( "Correction Factor (D):" ),
                   QString::number( solution.D20w_correction, 'f', 6 ) ) + 
        indent( 4 ) + "</table>\n";

   return s;
}

// Compose scan information portion of report text
QString US_FeMatch::scan_info( void ) const
{
   int                          drow   = lw_triples->currentRow();
   const US_DataIO::EditedData* d      = &dataList[ drow ];
   double time_correction  = US_Math2::time_correction( dataList );

   QString s = "\n" + indent( 4 ) + tr( "<h3>Scan Information:</h3>\n" )
               + indent( 4 ) + "<table>\n"; 
         
   s += table_row( tr( "Scan" ), tr( "Corrected Time" ), 
                   tr( "Plateau Concentration" ) );

   for ( int i = 0; i < d->scanData.size(); i++ )
   {
      QString s1;
      QString s2;
      QString s3;

      double od   = d->scanData[ i ].plateau;
      int    time = (int)( d->scanData[ i ].seconds - time_correction ); 

      s1 = s1.sprintf( "%4d",             i + 1 );
      s2 = s2.sprintf( "%4d min %2d sec", time / 60, time % 60 );
      s3 = s3.sprintf( "%.6f OD",         od ); 

      s += table_row( s1, s2, s3 );
   }

   s += indent( 4 ) + "</table>\n";
   
   return s;
}

// Distribution information HTML string
QString US_FeMatch::distrib_info() const
{
   int ncomp      = model_loaded.components.size();
   
   if ( ncomp == 0 )
      return "";

   QString mstr = "\n" + indent( 4 )
                  + tr( "<h3>Data Analysis Settings:</h3>\n" )
                  + indent( 4 ) + "<table>\n";

   mstr += table_row( tr( "Model Analysis:" ),
                      model_loaded.description
                      .section( ".", -2, -2 ).section( "_", 1, -1 ) );
   mstr += table_row( tr( "Number of Components:" ),
                      QString::number( ncomp ) );
   mstr += table_row( tr( "Residual RMS Deviation:" ),
                      le_rmsd->text()  );

   double sum_mw  = 0.0;
   double sum_s   = 0.0;
   double sum_D   = 0.0;
   double sum_c   = 0.0;
   double sum_v   = 0.0;
   double sum_k   = 0.0;
   double mink    = 1e+99;
   double maxk    = -1e+99;
   double minv    = 1e+99;
   double maxv    = -1e+99;

   for ( int ii = 0; ii < ncomp; ii++ )
   {
      double conc = model_loaded.components[ ii ].signal_concentration;
      double kval = model_loaded.components[ ii ].f_f0;
      double vval = model_loaded.components[ ii ].vbar20;
      sum_c      += conc;
      sum_mw     += ( model_loaded.components[ ii ].mw * conc );
      sum_s      += ( model_loaded.components[ ii ].s  * conc );
      sum_D      += ( model_loaded.components[ ii ].D  * conc );
      sum_v      += ( vval * conc );
      sum_k      += ( kval * conc );
      mink        = qMin( kval, mink );
      maxk        = qMax( kval, maxk );
      minv        = qMin( vval, minv );
      maxv        = qMax( vval, maxv );
   }

   mstr += table_row( tr( "Weight Average s20,W:" ),
                      QString().sprintf( "%6.4e", ( sum_s  / sum_c ) ) );
   mstr += table_row( tr( "Weight Average D20,W:" ),
                      QString().sprintf( "%6.4e", ( sum_D  / sum_c ) ) );
   mstr += table_row( tr( "W.A. Molecular Weight:" ),
                      QString().sprintf( "%6.4e", ( sum_mw / sum_c ) ) );
   if ( ! cnstvb )
      mstr += table_row( tr( "Weight Average vbar20:" ),
                         QString::number( ( sum_v / sum_c ) ) );
   else
      mstr += table_row( tr( "Weight Average f/f0:" ),
                         QString::number( ( sum_k / sum_c ) ) );
   mstr += table_row( tr( "Total Concentration:" ),
                      QString().sprintf( "%6.4e", sum_c ) );

   if ( cnstvb )
      mstr += table_row( tr( "Constant vbar20:" ),
                         QString::number( minv ) );
   else
      mstr += table_row( tr( "Constant f/f0:" ),
                         QString::number( mink ) );
   mstr += indent( 4 ) + "</table>\n";

   mstr += "\n" + indent( 4 ) + tr( "<h3>Distribution Information:</h3>\n" );
   mstr += indent( 4 ) + "<table>\n";

   if ( cnstvb )
   {  // Normal constant-vbar distribution
      mstr += table_row( tr( "Molec. Wt." ), tr( "S Apparent" ),
                         tr( "S 20,W" ),     tr( "D Apparent" ),
                         tr( "D 20,W" ),     tr( "f / f0" ),
                         tr( "Concentration" ) );

      for ( int ii = 0; ii < ncomp; ii++ )
      {
         double conc = model_loaded.components[ ii ].signal_concentration;
         double perc = 100.0 * conc / sum_c;
         mstr       += table_row(
               QString().sprintf( "%10.4e",
                  model_loaded.components[ ii ].mw   ),
               QString().sprintf( "%10.4e",
                  model       .components[ ii ].s    ),
               QString().sprintf( "%10.4e",
                  model_loaded.components[ ii ].s    ),
               QString().sprintf( "%10.4e",
                  model       .components[ ii ].D    ),
               QString().sprintf( "%10.4e",
                  model_loaded.components[ ii ].D    ),
               QString().sprintf( "%10.4e",
                  model_loaded.components[ ii ].f_f0 ),
               QString().sprintf( "%10.4e (%5.2f %%)", conc, perc ) );
      }
   }

   else
   {  // Constant-f/f0, varying vbar
      mstr += table_row( tr( "Molec. Wt." ), tr( "S Apparent" ),
                         tr( "S 20,W" ),     tr( "D Apparent" ),
                         tr( "D 20,W" ),     tr( "Vbar20" ),
                         tr( "Concentration" ) );

      for ( int ii = 0; ii < ncomp; ii++ )
      {
         double conc = model_loaded.components[ ii ].signal_concentration;
         double perc = 100.0 * conc / sum_c;
         mstr       += table_row(
               QString().sprintf( "%10.4e",
                  model_loaded.components[ ii ].mw     ),
               QString().sprintf( "%10.4e",
                  model       .components[ ii ].s      ),
               QString().sprintf( "%10.4e",
                  model_loaded.components[ ii ].s      ),
               QString().sprintf( "%10.4e",
                  model       .components[ ii ].D      ),
               QString().sprintf( "%10.4e",
                  model_loaded.components[ ii ].D      ),
               QString().sprintf( "%10.4e",
                  model_loaded.components[ ii ].vbar20 ),
               QString().sprintf( "%10.4e (%5.2f %%)", conc, perc ) );
      }
   }

   mstr += indent( 4 ) + "</table>\n";

   return mstr;
}

// Write out a plot
void US_FeMatch::write_plot( const QString& filename, const QwtPlot* plot )
{
   if ( filename.contains( ".svg" ) )
   {  // Save an SVG file and a PNG copy
      if ( US_GuiUtil::save_plot( filename, plot ) != 0 )
         QMessageBox::warning( this, tr( "File Write Error" ),
            tr( "Unable to write file" ) + filename );
   }

   else if ( filename.endsWith( "rbitmap.png" ) )
   {  // Special case of rbitmap PNG
      if ( rbmapd == 0 )
      {  // if it is not currently displayed, display it
         rbmapd = new US_ResidsBitmap( resids );
         rbmapd->move( bmd_pos );
         rbmapd->show();
         rbmapd->raise();
      }

      else
      {  // if already displayed,  replot and re-activate
         rbmapd->replot( resids );
         rbmapd->raise();
         rbmapd->activateWindow();
      }

      QPixmap pixmap = QPixmap::grabWidget( rbmapd, 0, 0,
                                            rbmapd->width(), rbmapd->height() );

      if ( ! pixmap.save( filename ) )
         QMessageBox::warning( this, tr( "File Write Error" ),
            tr( "Unable to write file" ) + filename );
   }

   else if ( filename.endsWith( "3dplot.png" ) )
   {  // Special case of 3dplot PNG
      if ( eplotcd == 0 )
      {  // if no 3d plot control up,  create it now
         eplotcd = new US_PlotControlFem( this, &model );
         eplotcd->move( epd_pos );
         eplotcd->show();
         eplotcd->do_3dplot();
      }

#if defined(Q_WS_WIN) || defined(Q_WS_MAC)
      US_Plot3D* widgw = eplotcd->widget_3dplot();
      bool ok          = widgw->save_plot( filename, QString( "png" ) );
#else
      QGLWidget* dataw = eplotcd->data_3dplot();
      QPixmap pixmap   = dataw->renderPixmap( dataw->width(), dataw->height(),
                                            true  );
      bool ok          = pixmap.save( filename );
#endif

      if ( ! ok )
         QMessageBox::warning( this, tr( "File Write Error" ),
            tr( "Unable to write file" ) + filename );
   }

   else if ( filename.endsWith( ".png" ) )
   {  // General case of PNG
      if ( US_GuiUtil::save_png( filename, plot ) != 0 )
         QMessageBox::warning( this, tr( "File Write Error" ),
            tr( "Unable to write file" ) + filename );
   }
}

// Create a subdirectory if need be
bool US_FeMatch::mkdir( const QString& baseDir, const QString& subdir )
{
   QDir folder( baseDir );

   if ( folder.exists( subdir ) ) return true;
           
   if ( folder.mkdir( subdir ) ) return true;
   
   QMessageBox::warning( this,
      tr( "File error" ),
      tr( "Could not create the directory:\n" ) + baseDir + "/" + subdir );
   
   return false;
}

// Slot to handle selection of a new triple
void US_FeMatch::new_triple( int trow )
{
   haveSim    = false;

   update( trow );

   data_plot();
}

// Set progress text
void US_FeMatch::set_progress( const QString message )
{
   te_desc->setText( "<b>" + message + " ...</b>" );
}

// Update the disk/DB choice element
void US_FeMatch::update_disk_db( bool isDB )
{
   isDB ?  dkdb_cntrls->set_db() : dkdb_cntrls->set_disk();
}

// Get solution parameters via US_SolutionGui
void US_FeMatch::get_solution()
{
   if ( ! dataLoaded )
      return;

   int dbdisk = ( dkdb_cntrls->db() ) ? US_Disk_DB_Controls::DB
                                      : US_Disk_DB_Controls::Disk;
   int expID  = 0;
   solution_rec.buffer.manual = manual;
   QString runID = dataList[ lw_triples->currentRow() ].runID;

   if ( dkdb_cntrls->db() )
   {
      US_Passwd pw;
      US_DB2 db( pw.getPasswd() );
      QStringList query( "get_experiment_info_by_runID" );
      query << runID << QString::number( US_Settings::us_inv_ID() );
      db.query( query );
      if ( db.lastErrno() != US_DB2::NOROWS )
      {
         db.next();
         expID = db.value( 1 ).toString().toInt();
      }
   }

   US_SolutionGui* soluInfo = new US_SolutionGui( expID, 1, true, dbdisk,
                                                  solution_rec, false );

   connect( soluInfo, SIGNAL( updateSolutionGuiSelection( US_Solution ) ),
            this,     SLOT(   updateSolution(             US_Solution ) ) );

   soluInfo->exec();
}

// Update solution parameters after user has made selections
void US_FeMatch::updateSolution( US_Solution solution_sel )
{
   solution_rec    = solution_sel;

   int bufID       = solution_rec.buffer.bufferID.toInt();
   QString sbufID  = QString::number( bufID );
   QString bufDesc = solution_rec.buffer.description;
   QString bdens   = le_density  ->text();
   QString bvisc   = le_viscosity->text();
   QString svbar   = le_vbar     ->text();
   QString bcmpr   = "";
   QString bmanu   = solution_rec.buffer.manual ? "1" : "0";
   QString errmsg  = "";
   QString bufGUID = solution_rec.buffer.GUID;
   
   if ( dkdb_cntrls->db() )
   {
      US_Passwd pw;
      US_DB2 db( pw.getPasswd() );
      US_SolutionVals::bufvals_db( &db, sbufID, bufGUID, bufDesc,
            bdens, bvisc, bcmpr, bmanu, errmsg );
   }

   else
   {
      US_SolutionVals::bufvals_disk( sbufID, bufGUID, bufDesc,
            bdens, bvisc, bcmpr, bmanu, errmsg );
   }

   density      = bdens.toDouble();
   viscosity    = bvisc.toDouble();
   vbar         = solution_rec.commonVbar20;
   svbar        = QString::number( vbar );
   manual       = ( !bmanu.isEmpty()  &&  bmanu == "1" );
   solution_rec.buffer.manual = manual;

   le_density  ->setText( bdens );
   le_viscosity->setText( bvisc );
   le_vbar     ->setText( svbar );
   le_solution ->setText( solution_rec.solutionDesc );
}

// Update progress bar as each component is completed
void US_FeMatch::update_progress( int icomp )
{
   progress->setValue( icomp );
}

// Reset scan excludes
void US_FeMatch::reset_excludes( void )
{
   int                    index      = lw_triples->currentRow();
   US_DataIO::EditedData* d          = &dataList[ index ];
   int                    totalScans = d->scanData.size();

   excludedScans.clear();

   ct_from->setMaxValue( totalScans );
   ct_to  ->setMaxValue( totalScans );

   if ( ct_to->value() != 0 )
      ct_to ->setValue( 0 );
   else
      data_plot();

   pb_reset_exclude->setEnabled( false );
   allExcls[ index ] = excludedScans;
}

// Reset data set
void US_FeMatch::reset( void )
{
   if ( ! dataLoaded ) return;

   excludedScans.clear();

   density      = DENS_20W;
   viscosity    = VISC_20W;
   vbar         = TYPICAL_VBAR;

   le_solution ->setText( tr( "(Experiment's solution)" ) );
   le_density  ->setText( QString::number( density,   'f', 6 ) );
   le_viscosity->setText( QString::number( viscosity, 'f', 5 ) );
   le_vbar     ->setText( QString::number( vbar,      'f', 5 ) );
   ct_from     ->disconnect();
   ct_to       ->disconnect();
   ct_from     ->setValue( 0 );
   ct_to       ->setValue( 0 );

   connect( ct_from,            SIGNAL( valueChanged( double ) ),
                                SLOT  ( exclude_from( double ) ) );
                                
   connect( ct_to,              SIGNAL( valueChanged( double ) ),
                                SLOT  ( exclude_to  ( double ) ) );

   lw_triples->  disconnect();
   lw_triples->  clear();
   dataList.     clear();
   rawList.      clear();
   excludedScans.clear();
   triples.      clear();
   allExcls.     clear();
   resids.       clear();

   dataLoaded = false;
   buffLoaded = false;
   haveSim    = false;
   mfilter    = "";

   data_plot1->detachItems();
   data_plot2->detachItems();
   data_plot1->clear();
   data_plot2->clear();
   data_plot1->replot();
   data_plot2->replot();

   pb_details  ->setEnabled( false );
   pb_distrib  ->setEnabled( false );
   pb_loadmodel->setEnabled( false );
   pb_simumodel->setEnabled( false );
   pb_view     ->setEnabled( false );
   pb_save     ->setEnabled( false );
   pb_exclude  ->setEnabled( false );
   pb_advanced ->setEnabled( false );
   pb_plot3d   ->setEnabled( false );
   pb_plotres  ->setEnabled( false );
   le_id       ->setText( "" );
   le_temp     ->setText( "" );
   te_desc     ->setText( "" );
}

// Return index to type of distribution plot currently displayed
int US_FeMatch::type_distrib( )
{
   const char* dptyp[] = 
   {
      "s20,w distribution",
      "MW distribution",
      "D20,w distribution",
      "f_f0 vs s20,w",
      "f_f0 vs MW",
      "vbar vs s20,w",
      "vbar vs MW",
      "D20,w vs s20,w",
      "D20,w vs MW",
      "Residuals"
   };
   const int ndptyp = sizeof( dptyp ) / sizeof( dptyp[0] );

   QString curtxt = pb_distrib->text();
   int     itype  = 0;

   for ( int ii = 0; ii < ndptyp; ii++ )
   { // identify text of current push button
      if ( curtxt == QString( dptyp[ ii ] ) )
      { // found:  save index and break
         itype   = ii;
         break;
      }
   }

   return itype;
}

// Copy report files to the database
void US_FeMatch::reportFilesToDB( QStringList& files )
{
   US_Passwd   pw;
   US_DB2      db( pw.getPasswd() );
   US_DB2*     dbP = &db;
   QStringList query;
   US_DataIO::EditedData*  edata = &dataList[ lw_triples->currentRow() ];
   QString     tripdesc          = edata->description;

   // Get the ID of the EditedData DB record associated with the report
   query << "get_editID" << edata->editGUID;
   db.query( query );
   db.next();
   int     idEdit = db.value( 0 ).toString().toInt();

   // Parse the plot files directory and set the runID for the report
   QString pfdir  = files[ 0 ].left( files[ 0 ].lastIndexOf( "/" ) );
   US_Report freport;
   freport.runID  = runID;

   int st = freport.saveFileDocuments( pfdir, files, dbP, idEdit,
                                       tripdesc );

   if ( st != US_DB2::OK )
   {
      qDebug() << "*ERROR* saveFileDocuments, status" << st;
   }
}

// Update progress when thread reports
void US_FeMatch::thread_progress( int thr, int icomp )
{
   int kcomp     = 0;
   kcomps[ thr ] = icomp;
   for ( int ii = 0; ii < nthread; ii++ )
      kcomp += kcomps[ ii ];
   progress->setValue( kcomp );
DbgLv(1) << "THR PROGR thr icomp" << thr << icomp << "kcomp" << kcomp;
}

// Update count of threads completed and colate simulations when all are done
void US_FeMatch::thread_complete( int thr )
{
   thrdone++;
DbgLv(1) << "THR COMPL thr" << thr << "thrdone" << thrdone;

   if ( thrdone >= nthread )
   {  // All threads are done, so sum thread simulation data
      for ( int ii = 0; ii < sdata->scanData.size(); ii++ )
      {
         for ( int jj = 0; jj < sdata->xvalues.size(); jj++ )
         {
            double conc = 0.0;

            for ( int kk = 0; kk < nthread; kk++ )
               conc += tsimdats[ kk ].value( ii, jj );

            sdata->setValue( ii, jj, conc );
         }
      }

      // Then show the results
      show_results();
   }
}

// Write a file containing a model distributions table in CSV format
void US_FeMatch::model_table( QString mdtFile )
{
   // Get the total concentration
   int ncomp      = model_loaded.components.size();
   if ( ncomp == 0 )
      return;
   QFile mdt_f( mdtFile );
   if ( ! mdt_f.open( QIODevice::WriteOnly | QIODevice::Text ) )
      return;
   double sum_c   = 0.0;

   for ( int ii = 0; ii < ncomp; ii++ )
   {
      double conc = model_loaded.components[ ii ].signal_concentration;
      sum_c      += conc;
   }

   // Write out the comma-separated-values text fields
   QTextStream ts( &mdt_f );
   const QString dquote( "\"" );
   const QString comma( "," );
   const QString endln( "\n" );

   // Write the header line
   ts << dquote + "Molec.Weight"   + dquote + comma
       + dquote + "S_Apparent"     + dquote + comma
       + dquote + "S_20_W"         + dquote + comma
       + dquote + "D_Apparent"     + dquote + comma
       + dquote + "D_20_W"         + dquote + comma
       + dquote + "f/f0"           + dquote + comma
       + dquote + "Vbar20"         + dquote + comma
       + dquote + "Concentration"  + dquote + comma
       + dquote + "Conc.Percent"   + dquote + endln;

   for ( int ii = 0; ii < ncomp; ii++ )
   {  // Write each component line
      double conc = model_loaded.components[ ii ].signal_concentration;
      double perc = 100.0 * conc / sum_c;
      ts << dquote + QString().sprintf( "%10.4e",
               model_loaded.components[ ii ].mw   )   + dquote + comma +
            dquote + QString().sprintf( "%10.4e",
               model       .components[ ii ].s    )   + dquote + comma +
            dquote + QString().sprintf( "%10.4e",
               model_loaded.components[ ii ].s    )   + dquote + comma +
            dquote + QString().sprintf( "%10.4e",
               model       .components[ ii ].D    )   + dquote + comma +
            dquote + QString().sprintf( "%10.4e",
               model_loaded.components[ ii ].D    )   + dquote + comma +
            dquote + QString().sprintf( "%10.4e",
               model_loaded.components[ ii ].f_f0 )   + dquote + comma +
            dquote + QString().sprintf( "%10.4e",
               model_loaded.components[ ii ].vbar20 ) + dquote + comma +
            dquote + QString().sprintf( "%10.4e",
               conc )                                 + dquote + comma +
            dquote + QString().sprintf( "%5.2f %%",
               perc )                                 + dquote + endln;
   }
}

// Update report file list, including adding PNG for each SVGZ
void US_FeMatch::update_filelist( QStringList& flist, const QString fname )
{
   flist << fname;

   if ( fname.contains( ".svg" ) )
      flist << QString( fname ).section( ".", 0, -2 ) + ".png";
}

