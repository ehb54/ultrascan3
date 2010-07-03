//! \file us_fe_match.cpp

#include <QApplication>

#include <uuid/uuid.h>

#include "us_fematch.h"
#include "us_resids_bitmap.h"
#include "us_plot_control.h"
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
   controlsLayout  = new QGridLayout();
   buttonLayout    = new QHBoxLayout();

   leftLayout->addLayout( analysisLayout  );
   leftLayout->addLayout( runInfoLayout   );
   leftLayout->addLayout( parameterLayout );
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
   ck_edit     ->setChecked( true );
   ck_edit     ->setFont( pb_load->font() );
   ck_edit     ->setPalette( US_GuiSettings::normalColor() );

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
   QLabel* lb_temp    = us_label ( tr( "Average Temperature:" ) );

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
   le_rmsd      = us_lineedit( "0.0" );
   le_variance  = us_lineedit( "0.0" );
   le_sedcoeff  = us_lineedit( "" );
   le_difcoeff  = us_lineedit( "" );
   le_partconc  = us_lineedit( "" );
   le_moweight  = us_lineedit( "" );
   ct_component = us_counter( 3, 1, 50, 1 );
   ct_component->setMinimumWidth( 170 );
   ct_component->setStep( 1.0 );

   QLabel* lb_experiment   = us_banner( tr( "Experimental Parameters (at 20" ) 
      + DEGC + "):" );
   QLabel* lb_variance     = us_label ( tr( "Variance:" ) );
   QLabel* lb_sedcoeff     = us_label ( tr( "Sedimentation Coefficient:" ) );
   QLabel* lb_difcoeff     = us_label ( tr( "Diffusion Coefficient:" ) );
   QLabel* lb_partconc     = us_label ( tr( "Partial Concentration:" ) );
   QLabel* lb_moweight     = us_label ( tr( "Molecular Weight, f/f0:" ) );
   QLabel* lb_component    = us_label ( tr( "Component:" ) );
           lb_simpoints    = us_label ( tr( "Simulation Points:" ) );
           lb_bldvolume    = us_label ( tr( "Band-loading Volume:" ) );
           lb_parameter    = us_label ( tr( "Parameter:" ) );

   cb_mesh      = us_comboBox();
   cb_mesh->addItem( tr( "Adaptive Space Time Mesh (ASTFEM)" )   );
   cb_mesh->addItem( tr( "Claverie Mesh" ) );
   cb_mesh->addItem( tr( "Moving Hat Mesh" ) );
   cb_mesh->addItem( tr( "File: \"$ULTRASCAN/mesh.dat\"" ) );
   cb_mesh->addItem( tr( "Adaptive Space Volume Mesh (ASVFEM)" )   );
   cb_grid      = us_comboBox();
   cb_grid->addItem( tr( "Moving Time Grid" )   );
   cb_grid->addItem( tr( "Constant Time Grid" ) );
   
   ct_simpoints = us_counter( 3, 0, 500, 1 );
   ct_bldvolume = us_counter( 3, 0, 1, 0.001 );
   ct_parameter = us_counter( 3, 1, 50, 1 );
   ct_modelnbr  = us_counter( 2, 1, 50, 1 );

   connect( pb_density,   SIGNAL( clicked() ),
            this,         SLOT( get_buffer() ) );
   connect( pb_viscosity, SIGNAL( clicked() ),
            this,         SLOT( get_buffer() ) );
   connect( pb_vbar,      SIGNAL( clicked() ),
            this,         SLOT( get_vbar() ) );
   connect( ct_component, SIGNAL( valueChanged( double ) ),
            this,         SLOT  ( comp_number(  double ) ) );
   le_rmsd->setReadOnly( true );

   density   = 0.998234;
   viscosity = 1.001940;
   vbar      = 0.72;

   ct_simpoints->setValue( 200 );
   ct_bldvolume->setValue( 0.015 );
   ct_parameter->setValue( 0 );
   ct_modelnbr ->setValue( 0 );
   ct_simpoints->setMinimumWidth( 170 );

   ct_simpoints->setEnabled( true  );
   ct_bldvolume->setEnabled( true  );
   ct_parameter->setEnabled( true  );
   pb_showmodel->setEnabled( false );
   ct_modelnbr ->setEnabled( false );
   cb_mesh     ->setEnabled( true  );
   cb_grid     ->setEnabled( true  );

   parameterLayout->addWidget( lb_experiment   , 0, 0, 1, 4 );
   parameterLayout->addWidget( pb_density      , 1, 0, 1, 1 );
   parameterLayout->addWidget( le_density      , 1, 1, 1, 1 );
   parameterLayout->addWidget( pb_viscosity    , 1, 2, 1, 1 );
   parameterLayout->addWidget( le_viscosity    , 1, 3, 1, 1 );
   parameterLayout->addWidget( pb_vbar         , 2, 0, 1, 1 );
   parameterLayout->addWidget( le_vbar         , 2, 1, 1, 1 );
   parameterLayout->addWidget( lb_variance     , 3, 0, 1, 1 );
   parameterLayout->addWidget( le_variance     , 3, 1, 1, 1 );
   parameterLayout->addWidget( lb_rmsd         , 3, 2, 1, 1 );
   parameterLayout->addWidget( le_rmsd         , 3, 3, 1, 1 );
   parameterLayout->addWidget( lb_sedcoeff     , 4, 0, 1, 2 );
   parameterLayout->addWidget( le_sedcoeff     , 4, 2, 1, 2 );
   parameterLayout->addWidget( lb_difcoeff     , 5, 0, 1, 2 );
   parameterLayout->addWidget( le_difcoeff     , 5, 2, 1, 2 );
   parameterLayout->addWidget( lb_partconc     , 6, 0, 1, 2 );
   parameterLayout->addWidget( le_partconc     , 6, 2, 1, 2 );
   parameterLayout->addWidget( lb_moweight     , 7, 0, 1, 2 );
   parameterLayout->addWidget( le_moweight     , 7, 2, 1, 2 );
   parameterLayout->addWidget( lb_component    , 8, 0, 1, 2 );
   parameterLayout->addWidget( ct_component    , 8, 2, 1, 2 );

   parameterLayout->addWidget( lb_simpoints    , 10, 0, 1, 2 );
   parameterLayout->addWidget( ct_simpoints    , 10, 2, 1, 2 );
   parameterLayout->addWidget( lb_bldvolume    , 11, 0, 1, 2 );
   parameterLayout->addWidget( ct_bldvolume    , 11, 2, 1, 2 );
   parameterLayout->addWidget( lb_parameter    , 12, 0, 1, 2 );
   parameterLayout->addWidget( ct_parameter    , 12, 2, 1, 2 );
   parameterLayout->addWidget( pb_showmodel    , 13, 0, 1, 2 );
   parameterLayout->addWidget( ct_modelnbr     , 13, 2, 1, 2 );
   parameterLayout->addWidget( cb_mesh         , 14, 0, 1, 4 );
   parameterLayout->addWidget( cb_grid         , 15, 0, 1, 4 );

   // Scan Controls

   QLabel* lb_scan    = us_banner( tr( "Scan Control"       ) ); 
   QLabel* lb_from    = us_label ( tr( "From:" ) );
   ct_from            = us_counter( 2, 0, 500, 1 );
   QLabel* lb_to      = us_label ( tr( "to:"   ) );
   ct_to              = us_counter( 2, 0, 500, 1 );
   pb_exclude         = us_pushbutton( tr( "Exclude Scan Range" ) );
   ct_from->setValue( 0 );
   ct_to  ->setValue( 0 );

   pb_exclude->setEnabled( false );

   connect( ct_from,    SIGNAL( valueChanged( double ) ),
            this,       SLOT  ( exclude_from( double ) ) );
   connect( ct_to,      SIGNAL( valueChanged( double ) ),
            this,       SLOT  ( exclude_to  ( double ) ) );
   connect( pb_exclude, SIGNAL( clicked() ),
            this,       SLOT  ( exclude() ) );

   controlsLayout->addWidget( lb_scan           , 0, 0, 1, 4 );
   controlsLayout->addWidget( lb_from           , 1, 0, 1, 1 );
   controlsLayout->addWidget( ct_from           , 1, 1, 1, 1 );
   controlsLayout->addWidget( lb_to             , 1, 2, 1, 1 );
   controlsLayout->addWidget( ct_to             , 1, 3, 1, 1 );
   controlsLayout->addWidget( pb_exclude        , 2, 0, 1, 4 );

   // Plots
   plotLayout1 = new US_Plot( data_plot1,
            tr( "Experimental Data" ),
            tr( "Radius (cm) " ),
            tr( "OD Difference" ) );

   gb_modelsim         = new QGroupBox( 
      tr( "Simulate data using parameters from"
          " model or from Monte Carlo statistics" ) );
   gb_modelsim->setFlat( true );
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
            tr( "Radius (cm) " ),
            tr( "Absorbance" ) );

   data_plot1->setCanvasBackground( Qt::black );
   data_plot2->setCanvasBackground( Qt::black );
   data_plot1->setMinimumSize( 600, 360 );
   data_plot2->setMinimumSize( 600, 280 );

   // Standard buttons
   pb_reset = us_pushbutton( tr( "Reset" ) );
   pb_help  = us_pushbutton( tr( "Help"  ) );
   pb_close = us_pushbutton( tr( "Close" ) );

   buttonLayout->addWidget( pb_reset );
   buttonLayout->addWidget( pb_help  );
   buttonLayout->addWidget( pb_close );

   connect( pb_reset, SIGNAL( clicked() ),
            this,     SLOT(   reset() ) );
   connect( pb_close, SIGNAL( clicked() ),
            this,     SLOT(   close_all() ) );
   connect( pb_help,  SIGNAL( clicked() ),
            this,     SLOT(   help() ) );

   rightLayout->addLayout( plotLayout1 );
   rightLayout->addWidget( gb_modelsim );
   rightLayout->addLayout( plotLayout2 );
   rightLayout->setStretchFactor( plotLayout1, 4 );
   rightLayout->setStretchFactor( plotLayout2, 2 );

   mainLayout->addLayout( leftLayout  );
   mainLayout->addLayout( rightLayout );
   mainLayout->setStretchFactor( leftLayout, 2 );
   mainLayout->setStretchFactor( rightLayout, 5 );

   set_ra_visible( false );

   dataLoaded = false;
   haveSim    = false;
   def_local  = true;
   mfilter    = "";
   investig   = "USER";
   resids.clear();
   rbmapd     = 0;
   eplotcd    = 0;
   rbd_pos    = this->pos() + QPoint( 100, 100 );
   epd_pos    = this->pos() + QPoint( 200, 200 );
}

// load data
void US_FeMatch::load( void )
{
   QString     file;
   QStringList files;
   QStringList parts;

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

   // insure we have a .auc file
   workingDir.replace( "\\", "/" );
   QDir wdir( workingDir );
   files    = wdir.entryList( QStringList() << "*.auc",
         QDir::Files | QDir::Readable, QDir::Name );

   if ( files.size() == 0 )
   {
      QMessageBox::warning( this,
            tr( "No Files Found" ),
            tr( "There were no files of the form *.auc\n"
                "found in the specified directory." ) );
      return;
   }

   // Look for cell / channel / wavelength combinations
   lw_triples->clear();
   dataList.clear();
   rawList.clear();
   excludedScans.clear();
   triples.clear();

   // Read all data
   if ( workingDir.right( 1 ) != "/" )
      workingDir += "/"; // Ensure trailing '/'

   if ( dataLatest )
   {  // build list of files with latest edit ID
      files    = wdir.entryList( QStringList() << "*.*.*.*.*.*.xml",
         QDir::Files | QDir::Readable, QDir::Name );
      files    = last_edit_files( files );
   }

   else
   {  // build list of files with edit ID matching selected file
      editID   = file.section( ".", 1, 1 );
      file     = "*." + editID + ".*.*.*.*.xml";
      files    = wdir.entryList( QStringList() << file,
         QDir::Files | QDir::Readable, QDir::Name );
   }

   for ( int ii = 0; ii < files.size(); ii++ )
   {  // load all data in directory; get triples
      file     = files[ ii ];
      parts    = file.split( "." );
 
      // load edit data (xml) and raw data (auc)
      int result = US_DataIO2::loadData( workingDir, file, dataList, rawList );

      if ( result != US_DataIO2::OK )
      {
         QMessageBox::warning( this,
            tr( "UltraScan Error" ),
            tr( "Could not read edit file.\n" ) 
            + US_DataIO2::errorString( result ) + "\n"
            + workingDir + file );
         return;
      }

      QString t = parts[ 3 ] + " / " + parts[ 4 ] + " / " + parts[ 5 ];
      runID     = parts[ 0 ];
      editID    = parts[ 1 ];

      if ( ! triples.contains( t ) )
      {  // update ListWidget with cell / channel / wavelength triple
         triples << t;
         lw_triples->addItem( t );
      } 
   }

   lw_triples->setCurrentRow( 0 );

   d         = &dataList[ 0 ];
   scanCount = d->scanData.size();
   double avgTemp = average_temperature();

   // set ID, description, and avg temperature text
   le_id  ->setText( d->runID + " / " + d->editID );
   te_desc->setText( d->description );
   le_temp->setText( QString::number( avgTemp, 'f', 1 ) + " " + DEGC );

   lw_triples->setCurrentRow( 0 );

   dataLoaded = true;
   haveSim    = false;

   update( 0 );

   pb_details  ->setEnabled( true );
   pb_loadmodel->setEnabled( true );
   pb_exclude  ->setEnabled( true );
   //mfilter     = dataList[ 0 ].runID;
   mfilter     = QString( "=edit" );

   ct_from->disconnect();
   ct_from->setValue( 0 );

   connect( ct_from, SIGNAL( valueChanged( double ) ),
            this,    SLOT(   exclude_from( double ) ) );

   rbd_pos    = this->pos() + QPoint( 100, 100 );
   epd_pos    = this->pos() + QPoint( 200, 200 );
}

// details
void US_FeMatch::details( void )
{
   US_RunDetails2* dialog
      = new US_RunDetails2( rawList, runID, workingDir, triples );

   dialog->move( this->pos() + QPoint( 100, 100 ) );
   dialog->exec();
   qApp->processEvents();

   delete dialog;
}

// update based on selected triples row
void US_FeMatch::update( int row )
{
   d              = &dataList[ row ];
   scanCount      = d->scanData.size();
   le_id->  setText( d->runID + " / " + d->editID );

   le_temp->setText( QString::number( average_temperature(), 'f', 1 )
         + " " + DEGC );
   te_desc->setText( d->description );

   ct_from->setMaxValue( scanCount - excludedScans.size() );
   ct_to  ->setMaxValue( scanCount - excludedScans.size() );
   ct_from->setStep( 1.0 );
   ct_to  ->setStep( 1.0 );

   data_plot();
}


// data plot
void US_FeMatch::data_plot( void )
{
   data_plot2->detachItems();

   if ( !dataLoaded )
      return;

   int row     = lw_triples->currentRow();
   d           = &dataList[ row ];
   data_plot2->setTitle(
      tr( "Velocity Data for " ) + d->runID );
   data_plot2->setAxisTitle( QwtPlot::yLeft,
      tr( "Absorbance at " ) + d->wavelength + tr( " nm" ) );
   data_plot2->setAxisTitle( QwtPlot::xBottom,
      tr( "Radius (cm)" ) );

   data_plot2->clear();
   us_grid( data_plot2 );

   int     scan_nbr  = 0;
   int     from      = (int)ct_from->value();
   int     to        = (int)ct_to  ->value();

   int     points    = d->scanData[ 0 ].readings.size();
   int     count     = points;

   if ( haveSim )
   {
      count     = sdata.scanData[ 0 ].readings.size();
qDebug() << "R,V points count" << points << count;
      count     = points > count ? points : count;
   }

   double* r         = new double[ count ];
   double* v         = new double[ count ];

   QString       title; 
   QwtPlotCurve* c;
   QPen          pen_red(  Qt::red );
   QPen          pen_cyan( Qt::cyan );
   QPen          pen_plot( US_GuiSettings::plotCurve() );

   // Calculate basic parameters for other functions
   solution.density   = le_density  ->text().toDouble();
   solution.viscosity = le_viscosity->text().toDouble();
   solution.vbar      = le_vbar     ->text().toDouble();

   s               = &d->scanData.last();
   int    point    = US_DataIO2::index( *s, dataList[ row ].x,
                        dataList[ row ].baseline );
   double baseline = 0.0;

   for ( int jj = point - 5; jj <= point + 5; jj++ )
      baseline        = s->readings[ jj ].value;

   baseline       /= 11.0;
//qDebug() << "baseline(c)" << baseline;
   double avgTemp  = average_temperature();
   solution.vbar20 = US_Math2::adjust_vbar( solution.vbar, avgTemp );
   US_Math2::data_correction( avgTemp, solution );

   // Draw curves
   for ( int ii = 0; ii < scanCount; ii++ )
   {
      if ( excludedScans.contains( ii ) ) continue;

      scan_nbr++;
      bool highlight = ( scan_nbr >= from  &&  scan_nbr <= to );

      US_DataIO2::Scan* s = &d->scanData[ ii ];

      double lower_limit = baseline;
      double upper_limit = s->plateau;

      int jj    = 0;
      count     = 0;

      // Plot each scan in (up to) three segments: below, in, and above
      // the specified boundaries

      while (  jj < points  &&  s->readings[ jj ].value < lower_limit )
      {  // accumulate coordinates of below-baseline points
         r[ count ] = d->x       [ jj ].radius;
         v[ count ] = s->readings[ jj ].value;
         jj++;
         count++;
      }

      if ( count > 1 )
      {  // plot portion of curve below baseline
         title = tr( "Curve " ) + QString::number( ii ) + tr( " below range" );
         c     = us_curve( data_plot2, title );

         if ( highlight )
            c->setPen( pen_red );
         else
            c->setPen( pen_cyan );
         
         c->setData( r, v, count );
      }

      count = 0;

      while ( jj < points && s->readings[ jj ].value < upper_limit )
      {  // accumulate coordinates of curve within baseline-to-plateau
         r[ count ] = d->x       [ jj ].radius;
         v[ count ] = s->readings[ jj ].value;
         jj++;
         count++;
      }

      if ( count > 1 )
      {  // plot portion of curve within baseline-to-plateau
         title = tr( "Curve " ) + QString::number( ii ) + tr( " in range" );
         c     = us_curve( data_plot2, title );

         if ( highlight )
            c->setPen( pen_red );
         else
            c->setPen( pen_plot );
         
         c->setData( r, v, count );
      }

      count = 0;

      while ( jj < points )
      {  // accumulate coordinates of curve portion above plateau
         r[ count ] = d->x       [ jj ].radius;
         v[ count ] = s->readings[ jj ].value;
         jj++;
         count++;
      }

      if ( count > 1 )
      {  // plot portion of curve above plateau
         title = tr( "Curve " ) + QString::number( ii ) + tr( " above range" );
         c     = us_curve( data_plot2, title );

         if ( highlight )
            c->setPen( pen_red );
         else
            c->setPen( pen_cyan );
        
         c->setData( r, v, count );
      }
   }

   // plot simulation
   if ( haveSim )
   {
      double rl = d->radius( 0 );
      double vh = d->value( scanCount - 1, points - 1 );
      rl       -= 0.05;
      vh       += ( vh - d->value( 0, 0 ) ) * 0.05;
qDebug() << "  RL" << rl << "  VH" << vh;
int nscan=scanCount;
int nconc=sdata.scanData[0].readings.size();
qDebug() << "    sdata ns nc " << nscan << nconc;
qDebug() << "      sdata.x0" << sdata.radius(0);
qDebug() << "      sdata.xN" << sdata.radius(nconc-1);
qDebug() << "      sdata.c00" << sdata.value(0,0);
qDebug() << "      sdata.c0N" << sdata.value(0,nconc-1);
qDebug() << "      sdata.cM0" << sdata.value(nscan-1,0);
qDebug() << "      sdata.cMN" << sdata.value(nscan-1,nconc-1);

      for ( int ii = 0; ii < scanCount; ii++ )
      {
         if ( excludedScans.contains( ii ) ) continue;

         points    = sdata.scanData[ ii ].readings.size();
//qDebug() << "      II POINTS" << ii << points;
         count     = 0;
         int jj    = 0;
         double rr = 0.0;
         double vv = 0.0;

         while ( jj < points )
         {  // accumulate coordinates of simulation curve
            rr         = sdata.radius( jj );
            vv         = sdata.value( ii, jj++ );
//qDebug() << "       JJ rr vv" << jj << rr << vv;

            if ( rr > rl  &&  vv < vh )
            {
               r[ count ] = rr;
               v[ count ] = vv;
               count++;
            }
         }
         title   = "SimCurve " + QString::number( ii );
         c       = us_curve( data_plot2, title );
         c->setPen( pen_red );
         c->setData( r, v, count );
qDebug() << "Sim plot scan count" << ii << count
 << "  r0 v0 rN vN" << r[0] << v[0 ] << r[count-1] << v[count-1];
      }
   }

   data_plot2->replot();

   delete [] r;
   delete [] v;

   return;
}

// save the enhanced data
void US_FeMatch::save_data( void )
{ 
qDebug() << "save_data";
   write_res();

   write_cofs();
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
   US_BufferGui* bdiag = new US_BufferGui( -1, true );
   connect( bdiag, SIGNAL( valueChanged(  double, double ) ),
            this,  SLOT  ( update_buffer( double, double ) ) );
   bdiag->exec();
   qApp->processEvents();
}

// update buffer
void US_FeMatch::update_buffer( double new_dens, double new_visc )
{
   density    = new_dens;
   viscosity  = new_visc;
qDebug() << "upd_buf dens visc" << density << viscosity;

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

void US_FeMatch::view_report( )
{
   QString mtext;
   int     row    = lw_triples->currentRow();
   d              = &dataList[ row ];

   // generate the report file
   write_res();

   // open it
   QString filename = US_Settings::resultDir() + "/" + d->runID + "."
      + text_model( model, 0 ) + "_res." + d->cell + wave_index( row );
   QFile   res_f( filename );
   QString fileexts = tr( "Result files (*_res*);;" )
      + tr( "RunID files (" ) + d->runID + "*);;"
      + tr( "All files (*)" );

   if ( res_f.open( QIODevice::ReadOnly | QIODevice::Text ) )
   {
      QTextStream ts( &res_f );

      while ( !ts.atEnd() )
         mtext.append( ts.readLine() + "\n" );

      res_f.close();
   }

   else
   {
      mtext.append( "*ERROR* Unable to open file " + filename );
   }

   // display the report dialog
   US_Editor* editd = new US_Editor( US_Editor::LOAD, true, fileexts );
   editd->setWindowTitle( tr( "Results:  FE Match Model Simulation" ) );
   editd->move( this->pos() + QPoint( 100, 100 ) );
   editd->resize( 600, 500 );
   editd->e->setFont( QFont( "monospace", US_GuiSettings::fontSize() ) );
   editd->e->setText( mtext );
   editd->show();
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
   double from          = ct_from->value();
   double to            = ct_to  ->value();
   int    displayedScan = 1;
          d             = &dataList[ lw_triples->currentRow() ];
   int    totalScans    = d->scanData.size();

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
}

void US_FeMatch::set_ra_visible( bool visible )
{
   lb_simpoints->setVisible( visible );  // visibility of RA experimental pars
   ct_simpoints->setVisible( visible );
   lb_bldvolume->setVisible( visible );
   ct_bldvolume->setVisible( visible );
   lb_parameter->setVisible( visible );
   ct_parameter->setVisible( visible );
   pb_showmodel->setVisible( visible );
   ct_modelnbr ->setVisible( visible );
   cb_mesh     ->setVisible( visible );
   cb_grid     ->setVisible( visible );

   gb_modelsim ->setVisible( visible );  // visibility model simulate group box
}

// respond to click of current type of distribution plot
void US_FeMatch::distrib_type( )
{
   const char* dptyp[] = 
   {
      "s20,w distribution",
      "MW distribution",
      "D20,w distribution",
      "f_f0 vs s20,w",
      "f_f0 vs MW",
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
//qDebug() << "distrib_type" << itype;

   // get pointer to data for use by plot routines
   d       = &dataList[ lw_triples->currentRow() ];

   // set push button text to next type
   int ii  = itype + 1;
   ii      = ( ii == ndptyp ) ? 0 : ii;
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
      case 5:     // D20,w vs s20,w
      case 6:     // D20,w vs MW
         distrib_plot_2d(    itype );  // 2-d plot
         break;
      case 7:     // Residuals
         distrib_plot_resids();        // residuals plot
         break;
   }
}

// do stick type distribution plot
void US_FeMatch::distrib_plot_stick( int type )
{
   QString pltitle = tr( "Run " ) + d->runID + tr( ": Cell " )
      + d->cell + " (" + d->wavelength + " nm)";
   QString xatitle;
   QString yatitle = tr( "Relative Concentration" );

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
   QwtPlotGrid*  data_grid = us_grid( data_plot1 );
   QwtPlotCurve* data_curv = us_curve( data_plot1, "distro" );

   int     dsize  = model.components.size();
   double* xx     = new double[ dsize ];
   double* yy     = new double[ dsize ];
   double  xmin   = 1.0e30;
   double  xmax   = -1.0e30;
   double  ymin   = 1.0e30;
   double  ymax   = -1.0e30;
   double  xval;
   double  yval;
   double  rdif;

   for ( int jj = 0; jj < dsize; jj++ )
   {
      xval     = ( type == 0 ) ? model.components[ jj ].s :
               ( ( type == 1 ) ? model.components[ jj ].mw :
                                 model.components[ jj ].D );
      yval     = model.components[ jj ].signal_concentration;
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
   xmin   = max( xmin, 0.0 );
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

   delete [] xx;
   delete [] yy;
}

// do 2d type distribution plot
void US_FeMatch::distrib_plot_2d( int type )
{
   QString pltitle = tr( "Run " ) + d->runID + tr( ": Cell " )
      + d->cell + " (" + d->wavelength + " nm)";
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
      pltitle = pltitle + tr( "\nDiff. Coeff. vs Sed. Coeff." );
      yatitle = tr( "Diffusion Coefficent D20,W" );
      xatitle = tr( "Sedimentation Coefficient s20,W" );
   }

   else if ( type == 6 )
   {
      pltitle = pltitle + tr( "\nDiff. Coeff. vs Molecular Weight" );
      yatitle = tr( "Diffusion Coefficent D20,W" );
      xatitle = tr( "Molecular Weight" );
   }

   data_plot1->setTitle(                       pltitle );
   data_plot1->setAxisTitle( QwtPlot::yLeft,   yatitle );
   data_plot1->setAxisTitle( QwtPlot::xBottom, xatitle );

   data_plot1->clear();
   data_plot1->detachItems();

   QwtPlotGrid*  data_grid = us_grid( data_plot1 );
   QwtPlotCurve* data_curv = us_curve( data_plot1, "distro" );
   QwtSymbol     symbol;

   int     dsize  = model.components.size();
   double* xx     = new double[ dsize ];
   double* yy     = new double[ dsize ];
   double  xmin   = 1.0e30;
   double  xmax   = -1.0e30;
   double  ymin   = 1.0e30;
   double  ymax   = -1.0e30;
   double  xval;
   double  yval;
   double  rdif;

   for ( int jj = 0; jj < dsize; jj++ )
   {
      xval     = ( ( type & 1 ) == 1 ) ? model.components[ jj ].s :
                                         model.components[ jj ].mw;
      yval     = ( type < 5          ) ? model.components[ jj ].f_f0 :
                                         model.components[ jj ].D;
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
   xmin   = max( xmin, 0.0 );
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

   delete [] xx;
   delete [] yy;
}

// do residuals type distribution plot
void US_FeMatch::distrib_plot_resids( )
{
   QString pltitle = tr( "Run " ) + d->runID + tr( ": Cell " )
      + d->cell + " (" + d->wavelength + " nm)" + tr( "\nResiduals" );
   QString yatitle = tr( "OD Difference" );
   QString xatitle = tr( "Radius in cm" );

   data_plot1->setTitle(     pltitle );
   data_plot1->setAxisTitle( QwtPlot::yLeft,   yatitle );
   data_plot1->setAxisTitle( QwtPlot::xBottom, xatitle );

   data_plot1->clear();
   data_plot1->detachItems();

   QwtPlotGrid*  data_grid = us_grid( data_plot1 );
   QwtPlotCurve* data_curv;
   QwtPlotCurve* line_curv = us_curve( data_plot1, "resids zline" );

   int     dsize  = d->scanData[ 0 ].readings.size();
   double* xx     = new double[ dsize ];
   double* yy     = new double[ dsize ];
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
      xval     = d->radius( jj );
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
         yval     = min( yval, 0.1 );
         yval     = max( yval, -0.1 );
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
         yval     = resids[ ii ][ jj ];
         yval     = min( yval, 0.1 );
         yval     = max( yval, -0.1 );
         yy[ jj ] = yval;
      }

      // plot the residual scatter for this scan
      data_curv = us_curve( data_plot1, "resids " +  QString::number( ii ) );
      data_curv->setPen(    QPen( Qt::yellow ) );
      data_curv->setStyle(  QwtPlotCurve::Dots );
      data_curv->setData(   xx, yy, dsize );
   }
qDebug() << " dsize" << dsize;
qDebug() << "  drN" << yy[dsize-1];

   data_plot1->replot();

   delete [] xx;
   delete [] yy;
}

// reset excluded scan range
void US_FeMatch::reset( )
{
//DEBUG: for now, use reset button to toggle RA visibility
bool visible=lb_simpoints->isVisible();
qDebug() << "debug isRA" << !visible;
set_ra_visible( !visible );
adjustSize(); 

   if ( !dataLoaded )
      return;

   excludedScans.clear();

   ct_from->disconnect();
   ct_to  ->disconnect();
   ct_from->setValue( 0 );
   ct_to  ->setValue( 0 );

   connect( ct_from, SIGNAL( valueChanged( double ) ),
            this,    SLOT  ( exclude_from( double ) ) );
   connect( ct_to,   SIGNAL( valueChanged( double ) ),
            this,    SLOT  ( exclude_to  ( double ) ) );

   data_plot();
}

// load model data and detect if RA
void US_FeMatch::load_model( )
{
   bool     visible = lb_simpoints->isVisible();  // current RA visibility
   bool     isRA;                                 // is model RA?
   QString  mdesc;

   // load model
   US_ModelLoader* dialog = new US_ModelLoader( false, def_local,
      mfilter, investig );
   dialog->move( this->pos() + QPoint( 200, 200 ) );
   dialog->set_edit_guid( dataList[ 0 ].editGUID );

   if ( dialog->exec() == QDialog::Accepted )
   {
      mfilter       = dialog->search_filter();     // next search filter
      investig      = dialog->investigator_text(); // next investigator
      mdesc         = dialog->description( 0 );

      dialog->load_model( model, 0 );              // load selected model

      if ( mdesc.section( mdesc.left( 1 ), 2, 2 ).isEmpty() )
         def_local     = false;  // empty filename:      default to db next
      else
         def_local     = true;   // non-empty filename:  default to local next

      delete dialog;
   }

   else
      return;                     // Cancel:  bail out now

   double avgTemp  = average_temperature();
double Vd=le_viscosity->text().toDouble();
qDebug() << "ViscD ViscM D/M" << Vd << model.viscosity << Vd/model.viscosity;

   if ( model.viscosity != 0.0 )
   {
      viscosity  = model.viscosity;
      le_viscosity->setText( QString::number( viscosity, 'f', 6 ) );
   }

   if ( model.density != 0.0 )
   {
      density    = model.density;
      le_density  ->setText( QString::number( density,   'f', 6 ) );
   }

   solution.density   = le_density  ->text().toDouble();
   solution.viscosity = le_viscosity->text().toDouble();
   solution.vbar      = le_vbar     ->text().toDouble();
   solution.vbar20    = US_Math2::adjust_vbar( solution.vbar, avgTemp );

   US_Math2::data_correction( avgTemp, solution );

   double scorrec  = 1.0 / solution.correction;
   double dcorrec  = ( ( K0 + avgTemp ) * 100.0 * VISC_20W )
      / ( K20 * solution.viscosity );

   int    ncomp    = model.components.size();       // components count
   int    nassoc   = model.associations.size();     // associations count
   isRA        = ( nassoc > 1 );                    // RA if #assocs > 1
   double s20w;
   double D20w;
   double vbar20;
   double mw;
   double vol;
   double f0;
   double fv;
   double f_f0;
   double rad_sphere;
   US_Model::SimulationComponent* sc;
qDebug() << "dialog mfilter" << mfilter;
qDebug() << "dialog investig" << investig;
qDebug() << "dialog mdesc" << mdesc;
qDebug() << "dialog model.desc" << model.description;
qDebug() << "dialog ncomp" << ncomp;
qDebug() << "dialog nassoc" << nassoc;
qDebug() << "dialog isRA" << isRA;
//DEBUG: for now, use Load Model button to toggle RA visibility
//isRA=!visible;
//qDebug() << "debug isRA" << isRA;
qDebug() << "scorrec dcorrec" << scorrec << dcorrec;
qDebug() << "  viscosity" << solution.viscosity << solution.viscosity_tb;

   // fill out components values

   for ( int jj = 0; jj < ncomp; jj++ )
   {
      sc         = &model.components[ jj ];
      s20w       = fabs( sc->s );
      D20w       = sc->D;
      vbar20     = sc->vbar20;
      vbar20     = ( vbar20 > 0.0 ) ? vbar20 : TYPICAL_VBAR;
      mw         = ( s20w / D20w ) * ( R * K20 ) / ( 1.0 - vbar20 * DENS_20W );
      vol        = vbar20 * mw / AVOGADRO;
      rad_sphere = pow( ( vol * 0.75 ) / M_PI, 1.0 / 3.0 );
      f0         = rad_sphere * 6.0 * M_PI * VISC_20W;
      fv         = ( mw * ( 1.0 - vbar20 * DENS_20W ) ) / ( s20w * AVOGADRO );
      f_f0       = fv / f0;
      sc->s     *= scorrec;
      sc->D     *= dcorrec;
if(jj==0)
qDebug() << "  s20w s" << s20w << sc->s << "  D20w D" << D20w << sc->D;

      sc->vbar20 = vbar20;
      sc->mw     = mw;
      sc->f      = fv;
      sc->f_f0   = f_f0;

      if ( sc->extinction > 0.0 )
         sc->molar_concentration = sc->signal_concentration / sc->extinction;
   }

   ct_component->setMaxValue( (double)ncomp );

   // set values for component 1
   component_values( 0 );

   if ( ( isRA && !visible )  ||  ( !isRA && visible ) )
   {  // new RA visibility state out of sync:  change it
      set_ra_visible( isRA );  // change visible components based on RA

      adjustSize();            // adjust overall size to visible components
   }

   pb_simumodel->setEnabled( true );
}

// do model simulation
void US_FeMatch::simulate_model( )
{
   int    row     = lw_triples->currentRow();
   US_SimulationParameters simparams;
   US_DataIO2::RawData*    rdata   = &rawList[  row ];
   US_DataIO2::EditedData* edata   = &dataList[ row ];
   US_DataIO2::Reading     reading;
   int    nscan   = rdata->scanData.size();
   int    nconc   = edata->x.size();
   double radlo   = edata->radius( 0 );
   //double radlo   = edata->meniscus;
   double radhi   = edata->radius( nconc - 1 );
   double time1   = rdata->scanData[ 0         ].seconds;
   double time2   = rdata->scanData[ nscan - 1 ].seconds;
   //double tcorrec = US_Math2::time_correction( dataList );
   //time1         -= tcorrec;
   //time2         -= tcorrec;
qDebug() << " nscan nconc" << nscan << nconc;
qDebug() << " radlo radhi" << radlo << radhi;
qDebug() << " baseline plateau" << edata->baseline << edata->plateau;

   // initialize simulation parameters using raw data information
   simparams.simpoints         = 200;
   simparams.meshType          = US_SimulationParameters::ASTFEM;
   simparams.gridType          = US_SimulationParameters::MOVING;
   simparams.radial_resolution = ( radhi - radlo ) / (double)( nconc - 1 );
   //simparams.meniscus          = radlo;
   simparams.meniscus          = edata->meniscus;
   simparams.bottom            = radhi;
//simparams.bottom            = 6.95;
   simparams.rnoise            = 0.0;
   simparams.tinoise           = 0.0;
   simparams.rinoise           = 0.0;
   simparams.rotor             = 1;
   simparams.band_forming      = false;
   simparams.band_volume       = 0.015;
qDebug() << "  rad_reso" << simparams.radial_resolution;
qDebug() << "   meniscus bottom" << simparams.meniscus << simparams.bottom;

   simparams.band_firstScanIsConcentration   = false;
   //simparams.band_firstScanIsConcentration   = true;
   simparams.mesh_radius.clear();
   simparams.speed_step .clear();
   US_SimulationParameters::SpeedProfile sp;
   sp.duration_hours    = (int)( time2 / 3600.0 );
   sp.duration_minutes  = (int)( time2 / 60.0 ) - ( sp.duration_hours * 60 );
   sp.delay_hours       = (int)( time1 / 3600.0 );
   sp.delay_minutes     = ( time1 / 60.0 ) - ( (double)sp.delay_hours * 60.0 );
   sp.scans             = nscan;
   sp.acceleration      = 400;
   sp.rotorspeed        = rdata->scanData[ 0 ].rpm;
   sp.acceleration_flag = false;
   simparams.speed_step << sp;
qDebug() << "  duration_hours  " << sp.duration_hours;
qDebug() << "  duration_minutes" << sp.duration_minutes;
qDebug() << "  delay_hours  " << sp.delay_hours;
qDebug() << "  delay_minutes" << sp.delay_minutes;

   // make a simulation copy of the experimental data without actual readings
   sdata.type[0]     = rdata->type[0];
   sdata.type[1]     = rdata->type[1];

   for ( int jj = 0; jj < 16; jj++ )
      sdata.rawGUID[ jj ] = rdata->rawGUID[ jj ];

   sdata.cell        = rdata->cell;
   sdata.channel     = rdata->channel;
   sdata.description = rdata->description;
qDebug() << "  sdata.description" << sdata.description;
   sdata.x.resize( nconc );

   for ( int jj = 0; jj < nconc; jj++ )
   {
      sdata.x[ jj ]     = edata->x[ jj ];
   }
qDebug() << "   sdata.x0" << sdata.radius(0);
qDebug() << "   sdata.xN" << sdata.radius(nconc-1);
qDebug() << "   rdata.cN" << rdata->value(0,0);
qDebug() << "   rdata.cN" << rdata->value(0,nconc-1);

   // use same concentration value for all of first scan
   reading.value     = model.components[ 0 ].signal_concentration;
   reading.stdDev    = 0.0;
   sdata.scanData.clear();

   for ( int ii = 0; ii < nscan; ii++ )
   {  // initialize readings for all sim data scans
      US_DataIO2::Scan sscan = edata->scanData[ ii ];
      //sscan.seconds -= tcorrec;

      for ( int jj = 0; jj < nconc; jj++ )
      {
         sscan.readings[ jj ] = reading;
      }

      sdata.scanData.append( sscan );
      // set values to zero for 2nd and subsequent scans
      reading.value     = 0.0;
   }

qDebug() << "   sdata.c00" << sdata.value(0,0);
qDebug() << "   sdata.c0N" << sdata.value(0,nconc-1);
qDebug() << "   sdata.cM0" << sdata.value(nscan-1,0);
qDebug() << "   sdata.cMN" << sdata.value(nscan-1,nconc-1);
qDebug() << " afrsa init";

   US_Astfem_RSA* astfem_rsa = new US_Astfem_RSA( model, simparams );
   
qDebug() << " afrsa calc";
//astfem_rsa->setTimeCorrection( true );

   astfem_rsa->calculate( sdata );

nscan = sdata.scanData.size();
nconc = sdata.x.size();
qDebug() << " afrsa done M N" << nscan << nconc;
qDebug() << "   sdata.x0" << sdata.radius(0);
qDebug() << "   sdata.xN" << sdata.radius(nconc-1);
qDebug() << "   sdata.c00" << sdata.value(0,0);
qDebug() << "   sdata.c0N" << sdata.value(0,nconc-1);
qDebug() << "   sdata.cM0" << sdata.value(nscan-1,0);
qDebug() << "   sdata.cMN" << sdata.value(nscan-1,nconc-1);

   haveSim     = true;
   pb_distrib->setEnabled( true );
   pb_view   ->setEnabled( true );
   pb_save   ->setEnabled( true );

   calc_residuals();             // calculate residuals

   distrib_plot_resids();        // plot residuals

   data_plot();                  // re-plot data+simulation

   if ( rbmapd )
   {
      rbd_pos  = rbmapd->pos();
      rbmapd->close();
   }

   rbmapd = new US_ResidsBitmap( resids );
   rbmapd->move( rbd_pos );
   rbmapd->show();

   if ( eplotcd )
   {
      epd_pos  = eplotcd->pos();
      eplotcd->close();
   }

   eplotcd = new US_PlotControl();
   eplotcd->move( epd_pos );
   eplotcd->show();
}

// pare down files list by including only the last-edit versions
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

// set values for component at index
void US_FeMatch::component_values( int index )
{
   le_sedcoeff->setText( QString::number( model.components[ index ].s ) );
   le_difcoeff->setText( QString::number( model.components[ index ].D ) );
   le_partconc->setText(
      QString::number( model.components[ index ].signal_concentration ) );
   le_moweight->setText(
      QString( "%1 kD,  %2" ).arg( model.components[ index ].mw / 1000.0 )
      .arg( model.components[ index ].f_f0 ) );
}

// component number changed
void US_FeMatch::comp_number( double cnbr )
{
   component_values( (int)cnbr - 1 );
}

// interpolate an sdata y (readings) value for a given x (radius)
double US_FeMatch::interp_sval( double xv, double* sx, double* sy, int ssize )
{
   for ( int jj = 1; jj < ssize; jj++ )
   {
      if ( xv < sx[ jj ] )
      {  // given x lower than array x: interpolate between point and previous
         double dx = sx[ jj ] - sx[ jj - 1 ];
         double dy = sy[ jj ] - sy[ jj - 1 ];
         return ( sy[ jj ] + ( xv - sx[ jj - 1 ] ) * dy / dx );
      }
   }

   // given x position not found:  interpolate using last two points
   int    jj = ssize - 1;
   double dx = sx[ jj ] - sx[ jj - 1 ];
   double dy = sy[ jj ] - sy[ jj - 1 ];
   return ( sy[ jj ] + ( xv - sx[ jj - 1 ] ) * dy / dx );
}

// write the results text file
void US_FeMatch::write_res()
{
   int     row      = lw_triples->currentRow();
   d                = &dataList[ row ];
   QString filename = US_Settings::resultDir() + "/" + d->runID + "."
      + text_model( model, 0 ) + "_res." + d->cell + wave_index( row );
   QFile   res_f( filename );

   if ( !res_f.open( QIODevice::WriteOnly | QIODevice::Text ) )
   {
      return;
   }

   s                = &d->scanData[ 0 ];
   int     vcount   = s->readings.size();
   int     scount   = d->scanData.size();
   int     ccount   = model.components.size();
   QString t20d     = QString( "20" ) + DEGC;
   QString stars    = QString( "*" ).repeated( 60 );
   double  tcorrec  = US_Math2::time_correction( dataList );
   double  baseline = calc_baseline( lw_triples->currentRow() );

   QTextStream ts( &res_f );

   ts << stars << "\n";
   ts << "*" << text_model( model, 58 ) << "*\n";
   ts << stars << "\n\n\n";
   ts << tr( "Data Report for Run \"" ) << d->runID
      << tr( "\",\n Cell " ) << d->cell << tr( ", Channel " ) << d->channel
      << tr( ", Wavelength " ) << d->wavelength
      << tr( ", Edited Dataset " ) << d->editID << "\n\n";

   ts << tr( "Detailed Run Information:\n\n" );
   ts << tr( "Cell Description:        " ) << d->description << "\n";
   ts << tr( "Raw Data Directory:      " ) << workingDir << "\n";
   ts << tr( "Rotor Speed:             " ) << s->rpm << " rpm\n";
   ts << tr( "Average Temperature:     " ) << le_temp->text() << "\n";
   ts << tr( "Temperature Variation:   Within Tolerance\n" );
   ts << tr( "Time Correction:         " ) << text_time( tcorrec, 1 ) << "\n";
   ts << tr( "Run Duration:            " )
      << text_time( d->scanData[ scount - 1 ].seconds, 2 ) << "\n";
   ts << tr( "Wavelength:              " ) << d->wavelength << " nm\n";
   ts << tr( "Baseline Absorbance:     " ) << baseline << " OD\n";
   ts << tr( "Meniscus Position:       " ) << d->meniscus << " cm\n";
   ts << tr( "Edited Data starts at:   " ) << d->radius( 0 ) << " cm\n";
   ts << tr( "Edited Data stops at:    " )
      << d->radius( vcount - 1 ) << " cm\n\n\n";

   ts << tr( "Hydrodynamic Settings:\n\n" );
   ts << tr( "Viscosity correction:    " ) << solution.viscosity << "\n";
   ts << tr( "Viscosity (absolute):    " ) << solution.viscosity_tb << "\n";
   ts << tr( "Density correction:      " ) << solution.density << " g/ccm\n";
   ts << tr( "Density (absolute):      " )
      << solution.density_tb << " g/ccm\n";
   ts << tr( "Vbar:                    " ) << solution.vbar << " ccm/g\n";
   ts << tr( "Vbar corrected for " ) << t20d << ": "
      << solution.vbar20 << " ccm/g\n";
   ts << tr( "Buoyancy (Water, " ) << t20d << "):  "
      << solution.buoyancyw << "\n";
   ts << tr( "Buoyancy (absolute):     " ) << solution.buoyancyb << "\n";
   ts << tr( "Correction Factor:       " ) << solution.correction << "\n\n\n";

   ts << tr( "Data Analysis Settings:\n\n" );
   ts << tr( "Number of Components:    " ) << ccount << "\n";
   ts << tr( "Residual RMS Deviation:  " ) << le_rmsd->text() << "\n\n";

   double sum_mw   = 0.0;
   double sum_s    = 0.0;
   double sum_D    = 0.0;
   double sum_c    = 0.0;
   double ctime    = 0.0;

   for ( int jj = 0; jj < ccount; jj++ )
   {
      double conc;
      conc     = model.components[ jj ].signal_concentration;
      sum_c   += conc;
      sum_mw  += model.components[ jj ].mw * conc;
      sum_s   += model.components[ jj ].s  * conc;
      sum_D   += model.components[ jj ].D  * conc;
   }

   ts << tr( "Weight Averages:\n\n" );
   ts << tr( "Weight Average s20,W:    " )
      << QString().sprintf( "%6.4e\n", (sum_s  / sum_c ) );
   ts << tr( "Weight Average D20,W:    " )
      << QString().sprintf( "%6.4e\n", (sum_D  / sum_c ) );
   ts << tr( "W.A. Molecular Weight:   " )
      << QString().sprintf( "%6.4e\n", (sum_mw / sum_c ) );
   ts << tr( "Total Concentration:     " )
      << QString().sprintf( "%6.4e\n", sum_c ) << "\n\n";

   ts << tr( "Distribution Information:\n\n" );
   ts << tr( "Molecular Weight    " )
      << tr( "S 20,W         " )
      << tr( "D 20,W         " )
      << tr( "Concentration\n" );

   for ( int jj = 0; jj < ccount; jj++ )
   {
      double conc;
      double perc;
      conc     = model.components[ jj ].signal_concentration;
      perc     = 100.0 * conc / sum_c;
      ts << QString().sprintf( " %12.5e  %14.5e %14.5e %14.5e  (%5.2f",
         model.components[ jj ].mw, model.components[ jj ].s,
         model.components[ jj ].D,  conc, perc ) << " %)\n";
   }

   ts << tr( "\n\nScan Information:\n\n" );
   ts << tr( "Scan" )
      << tr( "     Corrected Time" )
      << tr( "  Plateau Concentration" )
      << tr( "  (Ed,Sim Omega_s_t)\n" );

   for ( int ii = 0; ii < scount; ii++ )
   {
      s         = &d->scanData[ ii ];
      ctime     = s->seconds - tcorrec;
      ts << QString().sprintf( "%4i:", ( ii + 1 ) );
      ts << "   " << text_time( ctime, 0 );
      ts << QString().sprintf( "%14.6f OD  (%9.3e, %9.3e)\n",
            s->plateau, s->omega2t, sdata.scanData[ ii ].omega2t );
   }

   ts << "\n";

   res_f.close();
}

// write the results text file
void US_FeMatch::write_cofs()
{
   int    row      = lw_triples->currentRow();
   d               = &dataList[ row ];
   int    ccount   = model.components.size();
   double avgTemp  = average_temperature();
   double scorrec  = 1.0 / solution.correction;
   double dcorrec  = ( ( K0 + avgTemp ) * 100.0 * VISC_20W )
      / ( K20 * solution.viscosity );

   QString filename = US_Settings::resultDir() + "/" + d->runID + "."
      + text_model( model, 0 ) + "_dis." + d->cell + wave_index( row );
   QFile   res_f( filename );

   if ( !res_f.open( QIODevice::WriteOnly | QIODevice::Text ) )
   {
      return;
   }

   QTextStream ts( &res_f );

   ts << tr( "S_apparent" )
      << tr( "  S_20,W    " )
      << tr( "  D_apparent" )
      << tr( "  D_20,W    " )
      << tr( "  MW        " )
      << tr( "  Frequency " )
      << tr( "  f/f0(20,W)\n" );

   for ( int jj = 0; jj < ccount; jj++ )
   {
      ts << QString().sprintf(
         "%10.4e %11.4e %11.4e %11.4e %11.4e %11.4e %11.4e",
         model.components[ jj ].s,  model.components[ jj ].s / scorrec,
         model.components[ jj ].D,  model.components[ jj ].D / dcorrec,
         model.components[ jj ].mw, model.components[ jj ].signal_concentration,
         model.components[ jj ].f_f0 )
         << "\n";
   }

   res_f.close();
}

// format a wavelength index number string
QString US_FeMatch::wave_index( int row )
{
   QString cwaveln = dataList[ row ].wavelength;
   QStringList wavelns;

   wavelns << dataList[ 0 ].wavelength;  // start list of wavelengths

   for ( int jj = 1; jj < dataList.size(); jj++ )
   {  // add to list of unique wavelength strings
      QString dwaveln = dataList[ jj ].wavelength;

      if ( !wavelns.contains( dwaveln ) )
         wavelns << dwaveln;
   }

   wavelns.sort();                       // sort wavelengths

   // return string representation of index of current wavelength
   return QString::number( wavelns.indexOf( cwaveln ) + 1 );
}

// text of minutes,seconds or hours,minutes for a given total seconds value
QString US_FeMatch::text_time( double seconds, int type )
{
   int mins = (int)( seconds / 60.0 );
   int secs = (int)( seconds - (double)mins * 60.0 );

   if ( type == 0 )
   {  // fixed-field mins,secs text
      QString tmin = QString().sprintf( "%4d", mins );
      QString tsec = QString().sprintf( "%3d", secs );
      return tr( "%1 min %2 sec" ).arg( tmin ).arg( tsec );
   }

   else if ( type == 1 )
   {  // minutes,seconds text
      return tr( "%1 minute(s) %2 second(s)" ).arg( mins ).arg( secs );
   }

   else
   {  // hours,minutes text
      int hrs   = (int)( seconds / 3600.0 );
      mins      = qRound( ( seconds - (double)hrs * 3600.0 ) / 60.0 );
      return tr( "%1 hour(s) %2 minute(s)" ).arg( hrs ).arg( mins );
   }
}

// calculate average baseline absorbance
double US_FeMatch::calc_baseline( int row )
{
                           d  = &dataList[ row ];
   const US_DataIO2::Scan* ss = &d->scanData.last();
   int                     nn = US_DataIO2::index( *ss, d->x, d->baseline );
   double                  bl = 0.0;

   for ( int jj = nn - 5; jj < nn + 6; jj++ )
      bl += ss->readings[ jj ].value;

   return ( bl / 11.0 );
}

// model type text string
QString US_FeMatch::text_model( US_Model model, int width )
{
   QString title;

   switch ( (int)model.type )
   {
      case (int)US_Model::TWODSA:
         title = ( width == 0 ) ? "sa2d" :
            tr( "2-Dimensional Spectrum Analysis" );
         break;

      case (int)US_Model::TWODSA_MW:
         title = ( width == 0 ) ? "sa2d-mw" :
            tr( "2-Dimensional Spectrum Analysis" );
         break;

      case (int)US_Model::GA:
      case (int)US_Model::GA_RA:
         title = ( width == 0 ) ? "ga" :
            tr( "Genetic Algorithm Analysis" );
         break;

      case (int)US_Model::GA_MW:
         title = ( width == 0 ) ? "ga-mw" :
            tr( "Genetic Algorithm Analysis" );
         break;

      case (int)US_Model::COFS:
         title = ( width == 0 ) ? "cofs" :
            tr( "C(s) Analysis" );
         break;

      case (int)US_Model::FE:
         title = ( width == 0 ) ? "fe" :
            tr( "Finite Element Analysis" );
         break;

      case (int)US_Model::GLOBAL:
         title = ( width == 0 ) ? "global" :
            tr( "Global Algorithm Analysis" );
         break;

      case (int)US_Model::ONEDSA:
         title = ( width == 0 ) ? "sa1d" :
            tr( "1-Dimensional Spectrum Analysis" );
         break;

      case (int)US_Model::MANUAL:
      default:
         title = ( width == 0 ) ? "sa2d" :
            tr( "2-Dimensional Spectrum Analysis" );
         break;
   }

   if ( width == 0 )
   {  // short title (file node):  add any "ra" or "mc"

      if ( model.associations.size() > 1 )
         title = title + "-ra";

      if ( model.iterations > 1 )
         title = title + "-mc";

   }

   else if ( width > title.length() )
   {  // long title centered:  center it in fixed-length string
      int lent = title.length();
      int lenl = ( width - lent ) / 2;
      int lenr = width - lent - lenl;
      title    = QString( " " ).repeated( lenl ) + title
               + QString( " " ).repeated( lenr );
   }

   return title;
}


// calculate residual absorbance values (data - sim)
void US_FeMatch::calc_residuals()
{
   int     dsize  = d->scanData[ 0 ].readings.size();
   int     ssize  = sdata.scanData[ 0 ].readings.size();
   double* xx     = new double[ dsize ];
   double* sx     = new double[ ssize ];
   double* sy     = new double[ ssize ];
   double  yval;
   double  sval;
   QVector< double > resscan;
   double rmsd    = 0.0;
   resids.clear();
   resscan.resize( dsize );

   for ( int jj = 0; jj < dsize; jj++ )
   {
      xx[ jj ] = d->radius( jj );
   }

   for ( int jj = 0; jj < ssize; jj++ )
   {
      sx[ jj ] = sdata.radius( jj );
   }

   for ( int ii = 0; ii < scanCount; ii++ )
   {

      for ( int jj = 0; jj < ssize; jj++ )
      {
         sy[ jj ] = sdata.value( ii, jj );
      }

      for ( int jj = 0; jj < dsize; jj++ )
      {
         sval          = interp_sval( xx[ jj ], sx, sy, ssize );
         yval          = sval - d->value( ii, jj );
         rmsd         += sq( yval );
         resscan[ jj ] = yval;
      }

      resids.append( resscan );
   }

   rmsd  /= (double)( scanCount * dsize );
   le_variance->setText( QString::number( rmsd ) );
   rmsd   = sqrt( rmsd );
   le_rmsd    ->setText( QString::number( rmsd ) );

   delete [] xx;
   delete [] sx;
   delete [] sy;
}

// calculate average temperature across scans
double US_FeMatch::average_temperature()
{
   double avgTemp  = 0.0;

   for ( int ii = 0; ii < scanCount; ii++ )
      avgTemp += d->scanData[ ii ].temperature;

   avgTemp        /= (double)scanCount;
   return avgTemp;
}

// slot to make sure all windows and dialogs get closed
void US_FeMatch::close_all()
{
   if ( rbmapd )
      rbmapd->close();

   if ( eplotcd )
      eplotcd->close();

   close();
}

