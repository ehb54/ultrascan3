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
            this,         SLOT( load() ) );
   connect( pb_details,   SIGNAL( clicked() ),
            this,         SLOT( details() ) );
   connect( ck_edit,      SIGNAL( stateChanged( int ) ),
            this,         SLOT  ( set_edit_last( int ) ) );
   connect( pb_distrib,   SIGNAL( clicked() ),
            this,         SLOT(   distr_type()  ) );
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

   connect( pb_density,   SIGNAL( clicked() ),
            this,         SLOT( get_buffer() ) );
   connect( pb_viscosity, SIGNAL( clicked() ),
            this,         SLOT( get_buffer() ) );
   connect( pb_vbar,      SIGNAL( clicked() ),
            this,         SLOT( get_vbar() ) );
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
   data_plot1->setMinimumSize( 600, 400 );
   data_plot2->setMinimumSize( 600, 300 );

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
            this,     SLOT(   close() ) );
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
   //scanCount = d->scanData.size();

   // set ID, description, and avg temperature text
   le_id  ->setText( d->runID + " / " + d->editID );
   te_desc->setText( d->description );
   double tempera = d->scanData[ 0 ].temperature;

   for ( int jj = 1; jj < d->scanData.size(); jj++ )
      tempera += d->scanData[ jj ].temperature;

   tempera  /= (double)( d->scanData.size() );
   le_temp->setText( QString::number( tempera, 'f', 1 ) + " " + DEGC );

#if 0
   savedValues.clear();

   for ( int ii = 0; ii < scanCount; ii++ )
   {  // save the data to be used in any smoothing
      s          = &d->scanData[ ii ];
      valueCount = s->readings.size();
      QVector< double > v;
      v.resize( valueCount );

      for ( int jj = 0; jj < valueCount; jj++ )
      {
         v[ jj ] = s->readings[ jj ].value;
      }

      savedValues << v;
   }
#endif
   lw_triples->setCurrentRow( 0 );

   dataLoaded = true;

   update( 0 );

   pb_details  ->setEnabled( true );
   pb_loadmodel->setEnabled( true );
   pb_distrib  ->setEnabled( true );
   pb_exclude  ->setEnabled( true );

   ct_from->disconnect();
   ct_from->setValue( 0 );

   connect( ct_from, SIGNAL( valueChanged( double ) ),
            this,    SLOT(   exclude_from( double ) ) );
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

// update
void US_FeMatch::update( int row )
{
   d              = &dataList[ row ];
   int scanCount  = d->scanData.size();
   le_id->setText( d->runID + "/" + d->editID );

   double avt = 0.0;

   for ( int ii = 0; ii < scanCount; ii++ )
   {
      avt       += d->scanData[ ii ].temperature;
   }

   avt       /= (double)scanCount;

   le_temp->setText( QString::number( avt, 'f', 1 ) + " " + DEGC );
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

   int     scanCount = d->scanData.size();
   int     points    = d->scanData[ 0 ].readings.size();
   double* r         = new double[ points ];
   double* v         = new double[ points ];

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
   double avgTemp  = 0.0;

   for ( int ii = 0; ii < scanCount; ii++ )
      avgTemp += d->scanData[ ii ].temperature;

   avgTemp        /= (double)scanCount;
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
      int count = 0;

      // Plot each scan in (up to) three segments: below, in, and above
      // the specified boundaries
      while (  jj < points  &&  s->readings[ jj ].value < lower_limit )
      {
         r[ count ] = d->x       [ jj ].radius;
         v[ count ] = s->readings[ jj ].value;
         jj++;
         count++;
      }
//qDebug() << "BELOW count" << count;

      QString       title; 
      QwtPlotCurve* c;

      if ( count > 1 )
      {
         title = tr( "Curve " ) + QString::number( ii ) + tr( " below range" );
         c     = us_curve( data_plot2, title );

         if ( highlight )
            c->setPen( QPen( Qt::red ) );
         else
            c->setPen( QPen( Qt::cyan ) );
         
         c->setData( r, v, count );
      }

      count = 0;

      while ( jj < points && s->readings[ jj ].value < upper_limit )
      {
         r[ count ] = d->x       [ jj ].radius;
         v[ count ] = s->readings[ jj ].value;
         jj++;
         count++;
      }
//qDebug() << "IN count" << count;

      if ( count > 1 )
      {
         title = tr( "Curve " ) + QString::number( ii ) + tr( " in range" );
         c = us_curve( data_plot2, title );

         if ( highlight )
            c->setPen( QPen( Qt::red ) );
         else
            c->setPen( QPen( US_GuiSettings::plotCurve() ) );
         
         c->setData( r, v, count );
      }

      count = 0;

      while ( jj < points )
      {
         r[ count ] = d->x       [ jj ].radius;
         v[ count ] = s->readings[ jj ].value;
         jj++;
         count++;
      }
//qDebug() << "ABOVE count" << count;

      if ( count > 1 )
      {
         title = tr( "Curve " ) + QString::number( ii ) + tr( " above range" );
         c = us_curve( data_plot2, title );

         if ( highlight )
            c->setPen( QPen( Qt::red ) );
         else
            c->setPen( QPen( Qt::cyan ) );
        
         c->setData( r, v, count );
      }
   }

   data_plot2->replot();

   delete [] r;
   delete [] v;
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

void US_FeMatch::view_report( )
{
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
}

void US_FeMatch::distrib_type( )
{
}

void US_FeMatch::reset( )
{
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

void US_FeMatch::load_model( )
{
//DEBUG: for now, use Load Model button to toggle RA visibility
bool visible = lb_simpoints->isVisible();
set_ra_visible( !visible );

   adjustSize();
}

void US_FeMatch::simulate_model( )
{
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

