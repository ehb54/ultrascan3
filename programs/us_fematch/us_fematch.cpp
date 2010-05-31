//! \file us_fe_match.cpp

#include <QApplication>

#include <uuid/uuid.h>

#include "us_fematch.h"
#include "us_license_t.h"
#include "us_license.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_matrix.h"
#include "us_constants.h"
#include "us_analyte_gui.h"

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
   parametraLayout = new QGridLayout();
   controlsLayout  = new QGridLayout();
   buttonLayout    = new QHBoxLayout();

   leftLayout->addLayout( analysisLayout  );
   leftLayout->addLayout( runInfoLayout   );
   leftLayout->addLayout( parameterLayout );
   leftLayout->addLayout( parametraLayout );
   leftLayout->addLayout( controlsLayout  );
   leftLayout->addStretch();
   leftLayout->addLayout( buttonLayout    );

   // Analysis buttons
   pb_load      = us_pushbutton( tr( "Load Data" ) );
   pb_details   = us_pushbutton( tr( "Run Details" ) );
   ck_edit      = new QCheckBox( tr( "Latest Data Edit" ) );
   pb_distrib   = us_pushbutton( tr( "s20,W Distribution" ) );
   pb_loadmodel = us_pushbutton( tr( "Load Model" ) );
   pb_simumodel = us_pushbutton( tr( "Simulate Model" ) );
   pb_view      = us_pushbutton( tr( "View Data Report" ) );
   pb_save      = us_pushbutton( tr( "Save Data" ) );

   connect( pb_load, SIGNAL( clicked() ), SLOT( load() ) );
   connect( pb_details, SIGNAL( clicked() ), SLOT( details() ) );

   ck_edit     ->setChecked( true );
   ck_edit     ->setFont( pb_load->font() );
   ck_edit     ->setPalette( US_GuiSettings::normalColor() );
   connect( ck_edit, SIGNAL( stateChanged( int ) ),
            this,    SLOT  ( set_edit_last( int ) ) );

   pb_load     ->setEnabled( true );
   pb_details  ->setEnabled( false );
   ck_edit     ->setEnabled( true );
   pb_distrib  ->setEnabled( false );
   pb_loadmodel->setEnabled( false );
   pb_simumodel->setEnabled( false );
   pb_view     ->setEnabled( false );
   pb_save     ->setEnabled( false );

   analysisLayout->addWidget( pb_load,      0, 0 );
   analysisLayout->addWidget( pb_details,   0, 1 );
   analysisLayout->addWidget( ck_edit,      1, 0 );
   analysisLayout->addWidget( pb_distrib,   1, 1 );
   analysisLayout->addWidget( pb_loadmodel, 2, 0 );
   analysisLayout->addWidget( pb_simumodel, 2, 1 );
   analysisLayout->addWidget( pb_view,      3, 0 );
   analysisLayout->addWidget( pb_save,      3, 1 );

   // Run info
   QLabel* lb_info    = us_banner( tr( "Information for this Run" ) );
   QLabel* lb_triples = us_banner( tr( "Cell / Channel / Wavelength" ) );
   QLabel* lb_id      = us_label ( tr( "Run ID / Edit ID:" ) );
   QLabel* lb_temp    = us_label ( tr( "Avg Temperature:" ) );

   le_id      = us_lineedit();
   le_id->setReadOnly( true );
   le_temp    = us_lineedit();
   le_temp->setReadOnly( true );

   QFont font( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() );
   QFontMetrics fm( font );
   int fontHeight = fm.lineSpacing();

   te_desc    = us_textedit();
   te_desc->setMaximumHeight( fontHeight * 2 + 12 );  // Add for border
   te_desc->setReadOnly( true );

   lw_triples = us_listwidget();
   lw_triples->setMaximumHeight( fontHeight * 4 + 12 );


   runInfoLayout->addWidget( lb_info   , 0, 0, 1, 4 );
   runInfoLayout->addWidget( lb_id     , 1, 0, 1, 2 );
   runInfoLayout->addWidget( le_id     , 1, 2, 1, 2 );
   runInfoLayout->addWidget( lb_temp   , 2, 0, 1, 2 );
   runInfoLayout->addWidget( le_temp   , 2, 2, 1, 2 );
   runInfoLayout->addWidget( te_desc   , 3, 0, 2, 4 );
   runInfoLayout->addWidget( lb_triples, 5, 0, 1, 4 );
   runInfoLayout->addWidget( lw_triples, 6, 0, 5, 4 );

   // Parameters

   pb_density   = us_pushbutton( tr( "Density"   ) );
   le_density   = us_lineedit( "0.998234" );
   pb_viscosity = us_pushbutton( tr( "Viscosity" ) );
   le_viscosity = us_lineedit( "1.001940" );
   pb_vbar      = us_pushbutton( tr( "Vbar"   ) );
   le_vbar      = us_lineedit( "0.7200" );
   pb_showmodel = us_pushbutton( tr( "Show Model #"   ) );
   lb_rmsd      = us_label     ( tr( "RMSD:"  ) );
   le_rmsd      = us_lineedit( "0" );
   le_sedcoeff  = us_lineedit( "" );
   le_difcoeff  = us_lineedit( "" );
   le_partconc  = us_lineedit( "" );
   le_variance  = us_lineedit( "0.0" );
   le_moweight  = us_lineedit( "" );
   ct_component = us_counter( 3, 1, 50, 1 );
   ct_component->setMinimumWidth( 170 );

   connect( pb_density, SIGNAL( clicked() ), SLOT( get_buffer() ) );
   connect( pb_viscosity, SIGNAL( clicked() ), SLOT( get_buffer() ) );
   connect( pb_vbar, SIGNAL( clicked() ), SLOT( get_vbar() ) );
   le_rmsd->setReadOnly( true );

   density   = 0.998234;
   viscosity = 1.001940;
   vbar      = 0.72;

   QLabel* lb_experiment   = us_banner( tr( "Experimental Parameters (at 20" ) 
      + DEGC + "):" );
   QLabel* lb_sedcoeff     = us_label ( tr( "Sedimentation Coefficient:" ) );
   QLabel* lb_difcoeff     = us_label ( tr( "Diffusion Coefficient:" ) );
   QLabel* lb_partconc     = us_label ( tr( "Partial Concentration:" ) );
   QLabel* lb_variance     = us_label ( tr( "Variance:" ) );
   QLabel* lb_moweight     = us_label ( tr( "Molecular Weight, f/f0:" ) );
   QLabel* lb_component    = us_label ( tr( "Component:" ) );
           lb_simpoints    = us_label ( tr( "Simulation Points:" ) );
           lb_bldvolume    = us_label ( tr( "Band-loading Volume:" ) );
           lb_parameter    = us_label ( tr( "Parameter:" ) );

   cb_mesh      = us_comboBox();
   cb_mesh->addItem( tr( "Adaptive Space Mesh (ASTFEM)" )   );
   cb_mesh->addItem( tr( "Claverie Mesh" ) );
   cb_mesh->addItem( tr( "Moving Hat Mesh" ) );
   cb_mesh->addItem( tr( "File: \"$ULTRASCAN/mesh.dat\"" ) );
   cb_grid      = us_comboBox();
   cb_grid->addItem( tr( "Adaptive Time Grid" )   );
   cb_grid->addItem( tr( "Constant Time Grid" ) );
   
   ct_simpoints = us_counter( 3, 0, 500, 1 );
   ct_bldvolume = us_counter( 3, 0, 1, 0.001 );
   ct_parameter = us_counter( 3, 1, 50, 1 );
   ct_modelnbr  = us_counter( 2, 1, 50, 1 );
   ct_simpoints->setValue( 200 );
   ct_bldvolume->setValue( 0.015 );
   ct_parameter->setValue( 0 );
   ct_modelnbr ->setValue( 0 );
   ct_simpoints->setMinimumWidth( 170 );

   // Scan Controls

   QLabel* lb_scan         = us_banner( tr( "Scan Control"       ) ); 

   QLabel* lb_from         = us_label ( tr( "From:" ) );
   QLabel* lb_to           = us_label ( tr( "to:"   ) );

   pb_exclude = us_pushbutton( tr( "Exclude Scan Range" ) );
   pb_exclude->setEnabled( false );
   connect( pb_exclude, SIGNAL( clicked() ), SLOT( exclude() ) );

   ct_from            = us_counter( 2, 0, 0 );
   ct_to              = us_counter( 2, 0, 0 );
   
   connect( ct_from, SIGNAL( valueChanged( double ) ),
                     SLOT  ( exclude_from( double ) ) );
   connect( ct_to,   SIGNAL( valueChanged( double ) ),
                     SLOT  ( exclude_to  ( double ) ) );

   // Plots
   plotLayout1 = new US_Plot( data_plot1,
            tr( "Experimental Data" ),
            tr( "Radius in cm" ),
            tr( "OD Difference" ) );

   gb_modelsim         = new QGroupBox( 
      tr( "Simulate data using parameters from"
          " model or from Monte Carlo statistics" ) );
   QRadioButton* rb_curmod = new QRadioButton( tr( "Current Model" ) );
   QRadioButton* rb_mode   = new QRadioButton( tr( "Mode" ) );
   QRadioButton* rb_mean   = new QRadioButton( tr( "Mean" ) );
   QRadioButton* rb_median = new QRadioButton( tr( "Median" ) );
   gb_modelsim ->setFont( pb_load->font() );
   gb_modelsim ->setPalette( US_GuiSettings::normalColor() );
   QHBoxLayout* msbox = new QHBoxLayout();
   msbox->addWidget( rb_curmod );
   msbox->addWidget( rb_mode   );
   msbox->addWidget( rb_mean   );
   msbox->addWidget( rb_median );
   msbox->setSpacing       ( 0 );
   gb_modelsim->setLayout( msbox );
   rb_curmod->setChecked( true );

   plotLayout2 = new US_Plot( data_plot2,
            tr( "Velocity Data" ),
            tr( "Radius (in cm)" ),
            tr( "Absorbance" ) );

   data_plot1->setCanvasBackground( Qt::black );
   data_plot2->setCanvasBackground( Qt::black );
   data_plot1->setMinimumSize( 600, 400 );
   data_plot2->setMinimumSize( 600, 300 );

   // Standard buttons
   pb_reset = us_pushbutton( tr( "Reset" ) );
   pb_help  = us_pushbutton( tr( "Help"  ) );
   pb_close = us_pushbutton( tr( "Close" ) );
   pb_reset->setEnabled( false );

   buttonLayout->addWidget( pb_reset );
   connect( pb_reset, SIGNAL( clicked() ), SLOT( reset() ) );
   buttonLayout->addWidget( pb_help  );
   buttonLayout->addWidget( pb_close );
   connect( pb_close, SIGNAL( clicked() ), SLOT( close() ) );

   connect( pb_help, SIGNAL( clicked() ),
            this,    SLOT(   help() ) );

   parameterLayout->addWidget( lb_experiment   , 0, 0, 1, 4 );
   parameterLayout->addWidget( pb_density      , 1, 0, 1, 1 );
   parameterLayout->addWidget( le_density      , 1, 1, 1, 1 );
   parameterLayout->addWidget( pb_viscosity    , 1, 2, 1, 1 );
   parameterLayout->addWidget( le_viscosity    , 1, 3, 1, 1 );
   parameterLayout->addWidget( pb_vbar         , 2, 0, 1, 1 );
   parameterLayout->addWidget( le_vbar         , 2, 1, 1, 1 );
   parameterLayout->addWidget( lb_rmsd         , 2, 2, 1, 1 );
   parameterLayout->addWidget( le_rmsd         , 2, 3, 1, 1 );
   parameterLayout->addWidget( lb_sedcoeff     , 3, 0, 1, 2 );
   parameterLayout->addWidget( le_sedcoeff     , 3, 2, 1, 2 );
   parameterLayout->addWidget( lb_difcoeff     , 4, 0, 1, 2 );
   parameterLayout->addWidget( le_difcoeff     , 4, 2, 1, 2 );
   parameterLayout->addWidget( lb_partconc     , 5, 0, 1, 2 );
   parameterLayout->addWidget( le_partconc     , 5, 2, 1, 2 );
   parameterLayout->addWidget( lb_variance     , 6, 0, 1, 2 );
   parameterLayout->addWidget( le_variance     , 6, 2, 1, 2 );
   parameterLayout->addWidget( lb_moweight     , 7, 0, 1, 2 );
   parameterLayout->addWidget( le_moweight     , 7, 2, 1, 2 );
   parameterLayout->addWidget( lb_component    , 8, 0, 1, 2 );
   parameterLayout->addWidget( ct_component    , 8, 2, 1, 2 );

   parametraLayout->addWidget( lb_simpoints    , 10, 0, 1, 2 );
   parametraLayout->addWidget( ct_simpoints    , 10, 2, 1, 2 );
   parametraLayout->addWidget( lb_bldvolume    , 11, 0, 1, 2 );
   parametraLayout->addWidget( ct_bldvolume    , 11, 2, 1, 2 );
   parametraLayout->addWidget( lb_parameter    , 12, 0, 1, 2 );
   parametraLayout->addWidget( ct_parameter    , 12, 2, 1, 2 );
   parametraLayout->addWidget( pb_showmodel    , 13, 0, 1, 2 );
   parametraLayout->addWidget( ct_modelnbr     , 13, 2, 1, 2 );
   parametraLayout->addWidget( cb_mesh         , 14, 0, 1, 4 );
   parametraLayout->addWidget( cb_grid         , 15, 0, 1, 4 );

   ct_simpoints->setEnabled( true  );
   ct_bldvolume->setEnabled( true  );
   ct_parameter->setEnabled( true  );
   pb_showmodel->setEnabled( false );
   ct_modelnbr ->setEnabled( false );
   cb_mesh     ->setEnabled( true  );
   cb_grid     ->setEnabled( true  );

   set_ra_visible( false );

   parametraLayout->setEnabled( false );

   controlsLayout->addWidget( lb_scan           , 0, 0, 1, 4 );
   controlsLayout->addWidget( lb_from           , 1, 0, 1, 1 );
   controlsLayout->addWidget( ct_from           , 1, 1, 1, 1 );
   controlsLayout->addWidget( lb_to             , 1, 2, 1, 1 );
   controlsLayout->addWidget( ct_to             , 1, 3, 1, 1 );
   controlsLayout->addWidget( pb_exclude        , 2, 0, 1, 4 );

   rightLayout->addLayout( plotLayout1 );
   rightLayout->addWidget( gb_modelsim );
   rightLayout->addLayout( plotLayout2 );
   rightLayout->setStretchFactor( plotLayout1, 4 );
   rightLayout->setStretchFactor( plotLayout2, 2 );

   mainLayout->addLayout( leftLayout  );
   mainLayout->addLayout( rightLayout );
   mainLayout->setStretchFactor( leftLayout, 2 );
   mainLayout->setStretchFactor( rightLayout, 5 );

qDebug() << "leftLayout width" << leftLayout->geometry().width();
qDebug() << "ct_simpoints size" << ct_simpoints->size();
   dataLoaded = false;

}

// load data
void US_FeMatch::load( void )
{
   QString file;

   dataLoaded = false;
   dataLatest = ck_edit->isChecked();

   if ( dataLatest )
   {  // will be getting latest edit, so let user choose directory
      workingDir = QFileDialog::getExistingDirectory( this,
            tr( "Raw Data Directory" ),
            US_Settings::resultDir(),
            QFileDialog::DontResolveSymlinks | QFileDialog::ShowDirsOnly );
qDebug() << "LATEST edit: wdir" << workingDir;
   }

   else
   {  // will be getting specific edit, so user may choose a file
      file       = QFileDialog::getOpenFileName( this,
            tr( "Raw Data Directory & Specific Edit" ),
            US_Settings::resultDir(),
            tr( "Edit/Data Files (*.xml *.auc);; All Files( *)" ) );
      workingDir   = QFileInfo( file ).absolutePath();
qDebug() << "SPECIFIC edit: file" << file;
qDebug() << "SPECIFIC edit: wdir" << workingDir;
      if ( file.contains( ".auc" ) )
         dataLatest    = true;
   }

qDebug() << "dataLatest:" << dataLatest;

   if ( workingDir.isEmpty() )
      return;

   pb_details->disconnect( );                        // reset details connect
   pb_details->setEnable( true );
   connect( pb_details, SIGNAL( clicked() ),
            this,       SLOT(   details() ) );

   dataLoaded = true;

   update( 0 );

   //le_temp->setText( t );                            // set avg temp text
}

// details
void US_FeMatch::details( void )
{
//   US_RunDetails2* dialog
//      = new US_RunDetails2( rawList, runID, workingDir, triples );
//   dialog->move( this->pos() + QPoint( 100, 100 ) );
//   dialog->exec();
//   qApp->processEvents();
//   delete dialog;
}

// update
void US_FeMatch::update( int row )
{
   //QString ccw = lw_triples.at( row )->text();
}


// data plot
void US_FeMatch::data_plot( void )
{
   data_plot2->detachItems();

   // let AnalysisBase do the lower plot
   //US_AnalysisBase2::data_plot();
}

// save the enhanced data
void US_FeMatch::save_data( void )
{ 
qDebug() << "save_data";

}

// update density
void US_FeMatch::update_density(  double new_dens )
{
   density    = new_dens;
}

// update viscosity
void US_FeMatch::update_viscosity( double new_visc )
{
   viscosity  = new_visc;
}

// open dialog and get buffer information
void US_FeMatch::get_buffer( void )
{
   US_BufferGui* bdiag = new US_BufferGui( true );
   connect( bdiag, SIGNAL( valueChanged( double, double ) ),
            this,  SLOT  ( updateBuffer( double, double ) ) );
   bdiag->exec();
   qApp->processEvents();
}

// update buffer
void US_FeMatch::update_buffer( double new_dens, double new_visc )
{
   density    = new_dens;
   viscosity  = new_visc;

   le_density  ->setText( QString::number( density,   'f', 6 ) );
   le_viscosity->setText( QString::number( viscosity, 'f', 6 ) );
}

// open dialog and get vbar information
void US_FeMatch::get_vbar( void )
{
   US_AnalyteGui* vdiag = new US_AnalyteGui( -1, true );
   connect( vdiag, SIGNAL( valueChanged( double ) ),
             this,  SLOT  ( update_vbar ( double ) ) );
   vdiag->exec();
   qApp->processEvents();
}

// update vbar
void US_FeMatch::update_vbar( double new_vbar )
{
   vbar = new_vbar;
   le_vbar->setText( QString::number( new_vbar, 'f', 4 ) );
}

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

void US_FeMatch::exclude( void )
{
   double from = ct_from->value();
   double to   = ct_to  ->value();

}

void US_FeMatch::set_ra_visible( bool visible )
{
   lb_simpoints->setVisible( visible );
   ct_simpoints->setVisible( visible );
   lb_bldvolume->setVisible( visible );
   ct_bldvolume->setVisible( visible );
   lb_parameter->setVisible( visible );
   ct_parameter->setVisible( visible );
   pb_showmodel->setVisible( visible );
   ct_modelnbr ->setVisible( visible );
   cb_mesh     ->setVisible( visible );
   cb_grid     ->setVisible( visible );

   gb_modelsim ->setVisible( visible );
}

void US_FeMatch::set_edit_last( int state )
{
   dataLatest = ( state == Qt::Checked );
set_ra_visible( !dataLatest );
}

