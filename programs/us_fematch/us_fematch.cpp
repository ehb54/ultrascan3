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

// main program
int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );

   #include "main1.inc"

   // License is OK.  Start up.
   
   US_FeMatch w;
qDebug() << "MAIN create w";
   w.show();                   //!< \memberof QWidget
qDebug() << "MAIN show w";
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
   controlsLayout  = new QGridLayout();
   buttonLayout    = new QHBoxLayout();

   leftLayout->addLayout( analysisLayout  );
   leftLayout->addLayout( runInfoLayout   );
   leftLayout->addLayout( parameterLayout );
   leftLayout->addLayout( controlsLayout  );
   leftLayout->addStretch();
   leftLayout->addLayout( buttonLayout    );

   // Plots
   plotLayout1 = new US_Plot( data_plot1,
            tr( "Experimental Data" ),
            tr( "Radius in cm" ),
            tr( "OD Difference" ) );

   plotLayout2 = new US_Plot( data_plot2,
            tr( "Velocity Data" ),
            tr( "Radius (in cm)" ),
            tr( "Absorbance" ) );

   data_plot1->setCanvasBackground( Qt::black );
   data_plot2->setCanvasBackground( Qt::black );
   data_plot1->setMinimumSize( 600, 400 );
   data_plot2->setMinimumSize( 600, 300 );

   rightLayout->addLayout( plotLayout1 );
   rightLayout->addLayout( plotLayout2 );

   mainLayout->addLayout( leftLayout  );
   mainLayout->addLayout( rightLayout );

   // Analysis buttons
   pb_load    = us_pushbutton( tr( "Load Data" ) );
   connect( pb_load, SIGNAL( clicked() ), SLOT( load() ) );
   pb_details = us_pushbutton( tr( "Run Details" ) );
   connect( pb_details, SIGNAL( clicked() ), SLOT( details() ) );
   pb_view    = us_pushbutton( tr( "View Data Report" ) );
   pb_save    = us_pushbutton( tr( "Save Data" ) );
   pb_distrib = us_pushbutton( tr( "s20,W Distribution" ) );

   pb_details->setEnabled( false );
   pb_view   ->setEnabled( false );
   pb_save   ->setEnabled( false );
   pb_distrib->setEnabled( false );
   QLabel* lb_distrib = us_label ( "" );

   analysisLayout->addWidget( pb_load,    0, 0 );
   analysisLayout->addWidget( pb_details, 0, 1 );
   analysisLayout->addWidget( pb_view,    1, 0 );
   analysisLayout->addWidget( pb_save,    1, 1 );
   analysisLayout->addWidget( lb_distrib, 2, 0 );
   analysisLayout->addWidget( pb_distrib, 2, 1 );

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
   connect( pb_density, SIGNAL( clicked() ), SLOT( get_buffer() ) );
   
   pb_viscosity = us_pushbutton( tr( "Viscosity" ) );
   connect( pb_viscosity, SIGNAL( clicked() ), SLOT( get_buffer() ) );
   
   pb_vbar      = us_pushbutton( tr( "Vbar"   ) );
   connect( pb_vbar, SIGNAL( clicked() ), SLOT( get_vbar() ) );

   pb_loadmodel = us_pushbutton( tr( "Load Model" ) );
   pb_simumodel = us_pushbutton( tr( "Simulate Model" ) );
   lb_rmsd      = us_label     ( tr( "RMSD:"  ) );
   pb_loadmodel->setEnabled( false );
   pb_simumodel->setEnabled( false );

   density   = 0.998234;
   viscosity = 1.001940;
   vbar      = 0.72;

   QLabel* lb_experiment   = us_banner( tr( "Experimental Parameters (at 20" ) 
      + DEGC + "):" );
   QLabel* lb_sedcoeff     = us_label ( tr( "Sedmentation Coeff.:" ) );
   QLabel* lb_difcoeff     = us_label ( tr( "Diffusion Coeff.:" ) );
   QLabel* lb_partconc     = us_label ( tr( "Partial Concentration:" ) );
   QLabel* lb_variance     = us_label ( tr( "Variance:" ) );
   QLabel* lb_moweight     = us_label ( tr( "Mol.Weight,f/f0.:" ) );
   QLabel* lb_component    = us_label ( tr( "Component:" ) );
   
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



   le_density   = us_lineedit( "0.998234" );
   le_viscosity = us_lineedit( "1.001940" );
   le_vbar      = us_lineedit( "0.7200" );
   le_rmsd      = us_lineedit( "0" );
   le_sedcoeff  = us_lineedit( "" );
   le_difcoeff  = us_lineedit( "" );
   le_partconc  = us_lineedit( "" );
   le_variance  = us_lineedit( "0.0" );
   le_moweight  = us_lineedit( "" );
   ct_component = us_counter( 3, 1, 50, 1 );
   le_rmsd->setReadOnly( true );

qDebug() << "lb_experiment" << lb_experiment->text();

   parameterLayout->addWidget( lb_experiment   , 0, 0, 1, 4 );
   parameterLayout->addWidget( pb_density      , 1, 0, 1, 1 );
   parameterLayout->addWidget( le_density      , 1, 1, 1, 1 );
   parameterLayout->addWidget( pb_viscosity    , 1, 2, 1, 1 );
   parameterLayout->addWidget( le_viscosity    , 1, 3, 1, 1 );
   parameterLayout->addWidget( pb_vbar         , 2, 0, 1, 1 );
   parameterLayout->addWidget( le_vbar         , 2, 1, 1, 1 );
   parameterLayout->addWidget( lb_rmsd         , 2, 2, 1, 1 );
   parameterLayout->addWidget( le_rmsd         , 2, 3, 1, 1 );
   parameterLayout->addWidget( pb_loadmodel    , 3, 0, 1, 2 );
   parameterLayout->addWidget( pb_simumodel    , 3, 2, 1, 2 );
   parameterLayout->addWidget( lb_sedcoeff     , 4, 0, 1, 2 );
   parameterLayout->addWidget( le_sedcoeff     , 4, 2, 1, 2 );
   parameterLayout->addWidget( lb_difcoeff     , 5, 0, 1, 2 );
   parameterLayout->addWidget( le_difcoeff     , 5, 2, 1, 2 );
   parameterLayout->addWidget( lb_partconc     , 6, 0, 1, 2 );
   parameterLayout->addWidget( le_partconc     , 6, 2, 1, 2 );
   parameterLayout->addWidget( lb_variance     , 7, 0, 1, 2 );
   parameterLayout->addWidget( le_variance     , 7, 2, 1, 2 );
   parameterLayout->addWidget( lb_moweight     , 8, 0, 1, 2 );
   parameterLayout->addWidget( le_moweight     , 8, 2, 1, 2 );
   parameterLayout->addWidget( lb_component    , 9, 0, 1, 2 );
   parameterLayout->addWidget( ct_component    , 9, 2, 1, 2 );

   controlsLayout->addWidget( lb_scan           , 0, 0, 1, 4 );
   controlsLayout->addWidget( lb_from           , 1, 0, 1, 1 );
   controlsLayout->addWidget( ct_from           , 1, 1, 1, 1 );
   controlsLayout->addWidget( lb_to             , 1, 2, 1, 1 );
   controlsLayout->addWidget( ct_to             , 1, 3, 1, 1 );
   controlsLayout->addWidget( pb_exclude        , 2, 0, 1, 4 );

   connect( pb_help, SIGNAL( clicked() ),
            this,    SLOT(   help() ) );

   dataLoaded = false;

qDebug() << "CONSTRUCTOR return";
}

// load data
void US_FeMatch::load( void )
{
   dataLoaded = false;
   // query the directory where .auc and .xml file are
   workingDir = QFileDialog::getExistingDirectory( this,
         tr( "Raw Data Directory" ),
         US_Settings::resultDir(),
         QFileDialog::DontResolveSymlinks | QFileDialog::ShowDirsOnly );

   if ( workingDir.isEmpty() )
      return;

   data_plot1->setCanvasBackground( Qt::black );
   data_plot2->setCanvasBackground( Qt::black );
   data_plot1->setMinimumSize( 600, 400 );
   data_plot2->setMinimumSize( 600, 300 );

   pb_details->disconnect( );                        // reset details connect
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
void US_FeMatch::update_density(  double dval )
{
   density   = dval;
}

// update viscosity
void US_FeMatch::update_viscosity( double dval )
{
   viscosity  = dval;
}

// update vbar
void US_FeMatch::update_vbar(      double  dval )
{
   vbar       = dval;
}

