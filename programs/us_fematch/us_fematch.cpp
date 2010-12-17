//! \file us_fe_match.cpp

#include <QApplication>
#include <QtSvg>

#include "us_fematch.h"
#include "us_advanced.h"
#include "us_license_t.h"
#include "us_license.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_matrix.h"
#include "us_constants.h"
#include "us_analyte_gui.h"
#include "us_passwd.h"
#include "us_data_loader.h"
#include "us_util.h"
#include "us_investigator.h"
#include "us_lamm_astfvm.h"

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

   def_local  = false;
   def_local  = US_Settings::debug_match( "LoadLocal" ) ? true : def_local;
   dbg_level  = US_Settings::us_debug();

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
   pb_load      = us_pushbutton( tr( "Load Experiment" ) );
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
   le_temp    = us_lineedit();
   QPalette gray = US_GuiSettings::editColor();
   gray.setColor( QPalette::Base, QColor( 0xe0, 0xe0, 0xe0 ) );
   le_id  ->setReadOnly( true );
   le_temp->setReadOnly( true );
   le_id  ->setPalette(  gray );
   le_temp->setPalette(  gray );

   QFont font( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() );
   QFontMetrics fm( font );
   int fontHeight = fm.lineSpacing();

   te_desc    = us_textedit();
   te_desc->setMaximumHeight( fontHeight * 1 + 12 );  // Add for border
   te_desc->setReadOnly( true );
   te_desc->setPalette(  gray );

   lw_triples = us_listwidget();
   lw_triples->setMaximumHeight( fontHeight * 2 + 12 );

   runInfoLayout->addWidget( lb_info   , 0, 0, 1, 4 );
   runInfoLayout->addWidget( lb_id     , 1, 0, 1, 1 );
   runInfoLayout->addWidget( le_id     , 1, 1, 1, 3 );
   runInfoLayout->addWidget( lb_temp   , 2, 0, 1, 1 );
   runInfoLayout->addWidget( le_temp   , 2, 1, 1, 3 );
   runInfoLayout->addWidget( te_desc   , 3, 0, 2, 4 );
   runInfoLayout->addWidget( lb_triples, 5, 0, 1, 4 );
   runInfoLayout->addWidget( lw_triples, 6, 0, 5, 4 );

   // Parameters

   density      = DENS_20W;
   viscosity    = VISC_20W;
   compress     = 0.0;
   pb_density   = us_pushbutton( tr( "Density"   ) );
   le_density   = us_lineedit( QString::number( density,   'f', 6 ) );
   pb_viscosity = us_pushbutton( tr( "Viscosity" ) );
   le_viscosity = us_lineedit( QString::number( viscosity, 'f', 5 ) );
   pb_vbar      = us_pushbutton( tr( "Vbar"   ) );
   le_vbar      = us_lineedit( "0.7200" );
   pb_compress  = us_pushbutton( tr( "Compressibility" ) );
   le_compress  = us_lineedit( "0.0"     );
   lb_rmsd      = us_label     ( tr( "RMSD:"  ) );
   le_rmsd      = us_lineedit( "0.0" );
   le_variance  = us_lineedit( "0.0" );
   QFontMetrics fme( pb_compress->font() );
   int pwid = fme.width( pb_compress->text() + 6 );
   int lwid = pwid * 3 / 4;
   pb_vbar    ->setMinimumWidth( pwid );
   le_vbar    ->setMinimumWidth( lwid );
   pb_compress->setMinimumWidth( pwid );
   le_compress->setMinimumWidth( lwid );
   le_rmsd    ->setReadOnly( true );
   le_variance->setReadOnly( true );
   le_rmsd    ->setPalette(  gray );
   le_variance->setPalette(  gray );

   connect( le_density,   SIGNAL( returnPressed() ),
            this,         SLOT(   buffer_text()   ) );
   connect( le_viscosity, SIGNAL( returnPressed() ),
            this,         SLOT(   buffer_text()   ) );
   connect( le_compress,  SIGNAL( returnPressed() ),
            this,         SLOT(   buffer_text()   ) );
   connect( le_vbar,      SIGNAL( returnPressed() ),
            this,         SLOT(   vbar_text()     ) );

   QLabel* lb_experiment   = us_banner( tr( "Experimental Parameters (at 20" ) 
      + DEGC + "):" );
   QLabel* lb_variance     = us_label ( tr( "Variance:" ) );

   connect( pb_density,   SIGNAL( clicked() ),
            this,         SLOT( get_buffer() ) );
   connect( pb_viscosity, SIGNAL( clicked() ),
            this,         SLOT( get_buffer() ) );
   connect( pb_vbar,      SIGNAL( clicked() ),
            this,         SLOT( get_vbar() ) );
   connect( pb_compress,  SIGNAL( clicked() ),
            this,         SLOT( get_buffer() ) );

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

   int row  = 0;
   parameterLayout->addWidget( lb_experiment   , row++, 0, 1, 4 );
   parameterLayout->addWidget( pb_density      , row,   0, 1, 1 );
   parameterLayout->addWidget( le_density      , row,   1, 1, 1 );
   parameterLayout->addWidget( pb_viscosity    , row,   2, 1, 1 );
   parameterLayout->addWidget( le_viscosity    , row++, 3, 1, 1 );
   parameterLayout->addWidget( pb_vbar         , row,   0, 1, 1 );
   parameterLayout->addWidget( le_vbar         , row,   1, 1, 1 );
   parameterLayout->addWidget( pb_compress     , row,   2, 1, 1 );
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
   investig   = "USER";
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
   adv_vals[ "bldvolume" ] = "0.015";
   adv_vals[ "parameter" ] = "0";
   adv_vals[ "modelnbr"  ] = "0";
   adv_vals[ "meshtype"  ] = "ASTFEM";
   adv_vals[ "gridtype"  ] = "Moving";
   adv_vals[ "modelsim"  ] = "model";

   sdata          = 0;
}

// public function to get pointer to edit data
US_DataIO2::EditedData*     US_FeMatch::fem_editdata() { return edata;     }

// public function to get pointer to sim data
US_DataIO2::RawData*        US_FeMatch::fem_simdata()  { return sdata;     }

// public function to get pointer to load model
US_Model*                   US_FeMatch::fem_model()    { return &model;    }

// public function to get pointer to TI noise
US_Noise*                   US_FeMatch::fem_ti_noise() { return &ti_noise; }

// public function to get pointer to RI noise
US_Noise*                   US_FeMatch::fem_ri_noise() { return &ri_noise; }

// public function to get pointer to resid bitmap diag
QPointer< US_ResidsBitmap > US_FeMatch::fem_resbmap()  { return rbmapd;    }

// load data
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

   dataLoaded = false;
   buffLoaded = false;
   dataLatest = ck_edit->isChecked();

   US_DataLoader* dialog =
      new US_DataLoader( true, dataLatest, def_local, dfilter, investig );

   if ( dialog->exec() == QDialog::Accepted )
   {
      dialog->settings(  def_local, investig, dfilter );
      QTimer* ld_timer = new QTimer( this );
      connect( ld_timer, SIGNAL( timeout()     ),
                         SLOT( load_progress() ) );
      te_desc->setText( tr( "<b>Loading Experiment Data ... </b>" ) );
      ld_timer->start( 100 );
      dialog->load_edit( dataList,  rawList,  triples );
      ld_timer->stop();
      workingDir = dialog->description();

      if ( def_local )
         workingDir = workingDir.section( workingDir.left( 1 ), 4, 4 )
                      .left( workingDir.lastIndexOf( "/" ) );

      else
         workingDir = tr( "(database)" );

      delete dialog;
   }

   else                     // load was aborted
      return;

   qApp->processEvents();

   QFont font( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() );
   QFontMetrics fm( font );
   int fontHeight = fm.lineSpacing();
   int ntriples   = triples.size();
   lw_triples->setMaximumHeight( fontHeight * min( ntriples, 4 ) + 12 );

   for ( int ii = 0; ii < ntriples; ii++ )
      lw_triples->addItem( triples.at( ii ) );

   edata     = &dataList[ 0 ];
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

   pb_details  ->setEnabled( true );
   pb_loadmodel->setEnabled( true );
   pb_exclude  ->setEnabled( true );
   mfilter     = QString( "=edit" );
   dfilter     = QString( "" );

   ct_from->disconnect();
   ct_from->setValue( 0 );

   connect( ct_from, SIGNAL( valueChanged( double ) ),
            this,    SLOT(   exclude_from( double ) ) );

   bmd_pos    = this->pos() + QPoint( 100, 100 );
   epd_pos    = this->pos() + QPoint( 200, 200 );
   rpd_pos    = this->pos() + QPoint( 300, 300 );

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
void US_FeMatch::update( int drow )
{
   edata          = &dataList[ drow ];
   scanCount      = edata->scanData.size();
   runID          = edata->runID;
   le_id->  setText( runID + " / " + edata->editID );

   double avgTemp = edata->average_temperature();
   le_temp->setText( QString::number( avgTemp, 'f', 1 )
         + " " + DEGC );
   te_desc->setText( edata->description );

   ct_from->setMaxValue( scanCount - excludedScans.size() );
   ct_to  ->setMaxValue( scanCount - excludedScans.size() );
   ct_from->setStep( 1.0 );
   ct_to  ->setStep( 1.0 );

   // set up buffer values implied from experimental data
   QString bufid;
   QString bguid;
   QString bdesc;
   QString bdens = le_density  ->text();
   QString bvisc = le_viscosity->text();
   QString bcomp = le_compress ->text();
   QString svbar = le_vbar     ->text();
   bool    bufin = false;

   if ( def_local )
   {  // data from local disk:  get buffer vals (disk or db)
      bufin  = bufinfo_disk( edata, svbar, bufid, bguid, bdesc );
DbgLv(2) << "L:IL: bufin bdesc" << bufin << bdesc;
      bufin  = bufin ? bufin :
               bufinfo_db(   edata, svbar, bufid, bguid, bdesc );
DbgLv(2) << "L:ID: bufin bdesc" << bufin << bdesc;
      bufin  = bufvals_disk( bufid, bguid, bdesc, bdens, bvisc, bcomp );
DbgLv(2) << "L:VL: bufin bdens" << bufin << bdens;
      bufin  = bufin ? bufin :
               bufvals_db(   bufid, bguid, bdesc, bdens, bvisc, bcomp );
DbgLv(2) << "L:VD: bufin bdens" << bufin << bdens;
   }

   else
   {  // data from db:          get buffer vals (db or disk)
      bufin  = bufinfo_db(   edata, svbar, bufid, bguid, bdesc );
DbgLv(2) << "D:ID: bufin bdesc" << bufin << bdesc;
      bufin  = bufin ? bufin :
               bufinfo_disk( edata, svbar, bufid, bguid, bdesc );
DbgLv(2) << "D:IL: bufin bdesc" << bufin << bdesc;
      bufin  = bufvals_db(   bufid, bguid, bdesc, bdens, bvisc, bcomp );
DbgLv(2) << "D:VD: bufin bdens" << bufin << bdens;
      bufin  = bufin ? bufin :
               bufvals_disk( bufid, bguid, bdesc, bdens, bvisc, bcomp );
DbgLv(2) << "D:VL: bufin bdens" << bufin << bdens;
   }

   if ( bufin )
   {
      buffLoaded  = false;
      le_density  ->setText( bdens );
      le_viscosity->setText( bvisc );
      le_compress ->setText( bcomp );
      le_vbar     ->setText( svbar );
      buffLoaded  = true;
      density     = bdens.toDouble();
      viscosity   = bvisc.toDouble();
      compress    = bcomp.toDouble();
   }

   data_plot();
}


// data plot
void US_FeMatch::data_plot( void )
{
   data_plot2->detachItems();

   if ( !dataLoaded )
      return;

   int drow    = lw_triples->currentRow();
   edata       = &dataList[ drow ];
   data_plot2->setTitle(
      tr( "Velocity Data for " ) + edata->runID );
   data_plot2->setAxisTitle( QwtPlot::yLeft,
      tr( "Absorbance at " ) + edata->wavelength + tr( " nm" ) );
   data_plot2->setAxisTitle( QwtPlot::xBottom,
      tr( "Radius (cm)" ) );

   data_plot2->clear();
   us_grid( data_plot2 );

   int     scan_nbr  = 0;
   int     from      = (int)ct_from->value();
   int     to        = (int)ct_to  ->value();

   int     points    = edata->scanData[ 0 ].readings.size();
   int     count     = points;

   if ( haveSim )
   {
      count     = sdata->scanData[ 0 ].readings.size();
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

   dscan           = &edata->scanData.last();
   int    point    = US_DataIO2::index( *dscan, dataList[ drow ].x,
                        dataList[ drow ].baseline );
   double baseline = 0.0;

   for ( int jj = point - 5; jj <= point + 5; jj++ )
      baseline        = dscan->readings[ jj ].value;

   baseline       /= 11.0;
   double avgTemp  = edata->average_temperature();
   solution.vbar20 = US_Math2::adjust_vbar( solution.vbar, avgTemp );
   US_Math2::data_correction( avgTemp, solution );

   // Draw curves
   for ( int ii = 0; ii < scanCount; ii++ )
   {
      if ( excludedScans.contains( ii ) ) continue;

      scan_nbr++;
      bool highlight = ( scan_nbr >= from  &&  scan_nbr <= to );

      dscan          = &edata->scanData[ ii ];

      double lower_limit = baseline;
      double upper_limit = dscan->plateau;

      int jj    = 0;
      count     = 0;

      // Plot each scan in (up to) three segments: below, in, and above
      // the specified boundaries

      while (  jj < points  &&  dscan->readings[ jj ].value < lower_limit )
      {  // accumulate coordinates of below-baseline points
         r[ count   ] = edata->radius( jj );
         v[ count++ ] = edata->value( ii, jj++ );
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

      while ( jj < points && dscan->readings[ jj ].value < upper_limit )
      {  // accumulate coordinates of curve within baseline-to-plateau
         r[ count   ] = edata->radius( jj );
         v[ count++ ] = edata->value( ii, jj++ );
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
         r[ count   ] = edata->radius( jj );
         v[ count++ ] = edata->value( ii, jj++ );
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
int nconc=sdata->scanData[0].readings.size();
DbgLv(1) << "    sdata ns nc " << nscan << nconc;
DbgLv(1) << "      sdata->x0" << sdata->radius(0);
DbgLv(1) << "      sdata->xN" << sdata->radius(nconc-1);
DbgLv(1) << "      sdata->c00" << sdata->value(0,0);
DbgLv(1) << "      sdata->c0N" << sdata->value(0,nconc-1);
DbgLv(1) << "      sdata->cM0" << sdata->value(nscan-1,0);
DbgLv(1) << "      sdata->cMN" << sdata->value(nscan-1,nconc-1);

      for ( int ii = 0; ii < scanCount; ii++ )
      {
         if ( excludedScans.contains( ii ) ) continue;

         points    = sdata->scanData[ ii ].readings.size();
DbgLv(2) << "      II POINTS" << ii << points;
         count     = 0;
         int jj    = 0;
         double rr = 0.0;
         double vv = 0.0;
         rnoi      = have_ri ? ri_noise.values[ ii ] : 0.0;

         while ( jj < points )
         {  // accumulate coordinates of simulation curve
            tnoi      = have_ti ? ti_noise.values[ jj ] : 0.0;
            rr        = sdata->radius( jj );
            vv        = sdata->value( ii, jj++ ) + rnoi + tnoi;
DbgLv(3) << "       JJ rr vv" << jj << rr << vv;

            if ( rr > rl )
            {
               r[ count   ] = rr;
               v[ count++ ] = vv;
            }
         }
         title   = "SimCurve " + QString::number( ii );
         c       = us_curve( data_plot2, title );
         c->setPen( pen_red );
         c->setData( r, v, count );
DbgLv(1) << "Sim plot scan count" << ii << count
 << "  r0 v0 rN vN" << r[0] << v[0 ] << r[count-1] << v[count-1];
      }
   }

   data_plot2->replot();

   delete [] r;
   delete [] v;

   return;
}

// save the report and image data
void US_FeMatch::save_data( void )
{ 
   QStringList files;
   QString htmlFile;

   // save the report to a file
   write_rep( htmlFile );
   files << htmlFile;

   int     jj        = htmlFile.lastIndexOf( "report." );
   QString basename  = htmlFile.left( jj ).replace( "\\", "/" );
   const QString svgext( ".svg" );
   const QString pngext( ".png" );
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

   // save image files from main window
   write_plot( img01File, data_plot2 );
   files << img01File;

   distrib_plot_resids();
   write_plot( img02File, data_plot1 );
   files << img02File;

   distrib_plot_stick( 0 );
   write_plot( img03File, data_plot1 );
   files << img03File;

   distrib_plot_stick( 1 );
   write_plot( img04File, data_plot1 );
   files << img04File;

   distrib_plot_stick( 2 );
   write_plot( img05File, data_plot1 );
   files << img05File;

   distrib_plot_2d(    3 );
   write_plot( img06File, data_plot1 );
   files << img06File;

   distrib_plot_2d(    4 );
   write_plot( img07File, data_plot1 );
   files << img07File;

   distrib_plot_2d(    5 );
   write_plot( img08File, data_plot1 );
   files << img08File;

   distrib_plot_2d(    6 );
   write_plot( img09File, data_plot1 );
   files << img09File;

   distrib_plot_resids();

   // save 3-d plot
   write_plot( img10File, NULL );
   files << img10File;

   // save residual bitmap
   write_plot( img11File, NULL );
   files << img11File;

   // save any noise plots
   if ( ti_noise.count > 0 )
   {
      if ( resplotd == 0 )
      {
         resplotd = new US_ResidPlot( this );
         resplotd->move( rpd_pos );
         resplotd->show();
      }

      resplotd->set_plot( 1 );
      QwtPlot* nois_plot = resplotd->rp_data_plot2();
      write_plot( img12File, nois_plot );
      files << img12File;
   }

   if ( ri_noise.count > 0 )
   {
      if ( resplotd == 0 )
      {
         resplotd = new US_ResidPlot( this );
         resplotd->move( rpd_pos );
         resplotd->show();
      }

      resplotd->set_plot( 2 );
      QwtPlot* nois_plot = resplotd->rp_data_plot2();
      write_plot( img13File, nois_plot );
      files << img13File;
   }

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

   QMessageBox::information( this, tr( "Successfully Written" ), umsg );
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
   int idPers  = investig.section( ":", 0, 0 ).toInt();
   US_Buffer buff;

   US_BufferGui* bdiag = new US_BufferGui( idPers, true, buff, def_local );
   connect( bdiag, SIGNAL( valueChanged(  US_Buffer ) ),
            this,  SLOT  ( update_buffer( US_Buffer ) ) );
   bdiag->exec();
   qApp->processEvents();
}

// slot to update parameters after buffer dialog
void US_FeMatch::update_buffer( US_Buffer buffer )
{
   bool changed = true;

   // if we still have experiment buffer, allow user to abort changes
   if ( buffLoaded )
      changed = verify_buffer();

   // if experiment buffer is to be overridden, proceed with it
   if ( changed )
   {
      density    = buffer.density;
      viscosity  = buffer.viscosity;
      compress   = buffer.compressibility;

      buffLoaded = false;
      le_density  ->setText( QString::number( density,   'f', 6 ) );
      le_viscosity->setText( QString::number( viscosity, 'f', 5 ) );
      le_compress ->setText( QString::number( compress,  'e', 3 ) );
      qApp->processEvents();
   }
}

// open dialog and get vbar information
void US_FeMatch::get_vbar( void )
{
   int idPers  = investig.section( ":", 0, 0 ).toInt();
   QString aguid = "";

   US_AnalyteGui* vdiag = new US_AnalyteGui( idPers, true, aguid, !def_local );
   connect( vdiag,  SIGNAL( valueChanged( US_Analyte ) ),
             this,  SLOT  ( update_vbar ( US_Analyte ) ) );
   vdiag->exec();
   qApp->processEvents();
}

void US_FeMatch::view_report( )
{
   QString filename;
   QString mtext;
   int     drow   = lw_triples->currentRow();
   edata          = &dataList[ drow ];

   // generate the report file
   write_rep( filename );

   // open it
   QFile   rep_f( filename );
   QString fileexts = tr( "Report files (*_rep*);;" )
      + tr( "RunID files (" ) + edata->runID + "*);;"
      + tr( "All files (*)" );

   if ( rep_f.open( QIODevice::ReadOnly | QIODevice::Text ) )
   {
      QTextStream ts( &rep_f );

      while ( !ts.atEnd() )
         mtext.append( ts.readLine() + "\n" );

      rep_f.close();
   }

   else
   {
      mtext.append( "*ERROR* Unable to open file " + filename );
   }

   // display the report dialog
   US_Editor* editd = new US_Editor( US_Editor::DEFAULT, true, fileexts, this );
   editd->setWindowTitle( tr( "Report:  FE Match Model Simulation" ) );
   editd->move( this->pos() + QPoint( 100, 100 ) );
   editd->resize( 600, 700 );
   editd->e->setFont( QFont( US_GuiSettings::fontFamily(),
                             US_GuiSettings::fontSize() ) );
   editd->e->setHtml( mtext );
   editd->show();
}

// update vbar
void US_FeMatch::update_vbar( US_Analyte analyte )
{
   bool changed = true;

   if ( buffLoaded )
      changed   = verify_vbar();

   if ( changed )
   {
      vbar       = analyte.vbar20;
      buffLoaded = false;
      le_vbar->setText( QString::number( vbar, 'f', 5 ) );
      qApp->processEvents();
   }
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
          edata         = &dataList[ lw_triples->currentRow() ];
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
}

void US_FeMatch::set_ra_visible( bool /*visible*/ )
{
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

   // get pointer to data for use by plot routines
   edata   = &dataList[ lw_triples->currentRow() ];

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
   QString pltitle = tr( "Run " ) + edata->runID + tr( ": Cell " )
      + edata->cell + " (" + edata->wavelength + " nm)";
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

   int     dsize  = model_loaded.components.size();
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

   int     dsize  = model_loaded.components.size();
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
      xval     = ( ( type & 1 ) == 1 ) ? model_loaded.components[ jj ].s :
                                         model_loaded.components[ jj ].mw;
      yval     = ( type < 5          ) ? model_loaded.components[ jj ].f_f0 :
                                         model_loaded.components[ jj ].D;
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

   int     dsize  = edata->scanData[ 0 ].readings.size();
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

   data_plot1->replot();

   delete [] xx;
   delete [] yy;
}

// open dialog with advanced analysis parameters
void US_FeMatch::advanced( )
{
   advdiag = new US_Advanced( &model_loaded, adv_vals, (QWidget*)this );
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

   eplotcd = new US_PlotControl( this, &model );
   eplotcd->move( epd_pos );
   eplotcd->show();
}

// open residual plot dialog
void US_FeMatch::plotres( )
{
   if ( resplotd != 0 )
   {
      rpd_pos  = resplotd->pos();
      resplotd->close();
   }

   resplotd = new US_ResidPlot( this );
   resplotd->move( rpd_pos );
   resplotd->show();
}

// reset excluded scan range
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

// load model data and detect if RA
void US_FeMatch::load_model( )
{
   int      drow    = lw_triples->currentRow();
   QString  mdesc;

   // load model
   US_ModelLoader* dialog = new US_ModelLoader( false, def_local,
      mfilter, investig );
   dialog->move( this->pos() + QPoint( 200, 200 ) );
   dialog->set_edit_guid( dataList[ drow ].editGUID );

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

   qApp->processEvents();

//   if ( model.monteCarlo )
//      adjust_mc_model();
DbgLv(1) << "post-Load m,e,r GUIDs" << model.modelGUID << model.editGUID
   << model.requestGUID;
DbgLv(1) << "post-Load def_local" << def_local;

   model_loaded = model;   // save model exactly as loaded

   if ( model.components.size() == 0 )
   {
      QMessageBox::critical( this, tr( "Empty Model" ),
            tr( "Loaded model has ZERO components!" ) );
      return;
   }

   pb_advanced ->setEnabled( true );
   pb_simumodel->setEnabled( true );

   // see if there are any noise files to load
   load_noise();
}

// adjust model components based on buffer, vbar, and temperature
void US_FeMatch::adjust_model()
{
   model              = model_loaded;

   // build model component correction factors
   double avgTemp     = edata->average_temperature();

   solution.density   = le_density  ->text().toDouble();
   solution.viscosity = le_viscosity->text().toDouble();
   solution.vbar      = le_vbar     ->text().toDouble();
   solution.vbar20    = US_Math2::adjust_vbar( solution.vbar, avgTemp );

   US_Math2::data_correction( avgTemp, solution );

   double scorrec  = 1.0 / solution.s20w_correction;
   double dcorrec  = 1.0 / solution.D20w_correction;

   // fill out components values and adjust s,D based on buffer

   for ( int jj = 0; jj < model.components.size(); jj++ )
   {
      US_Model::SimulationComponent* sc = &model.components[ jj ];

      sc->mw      = 0.0;
      sc->f       = 0.0;
      sc->f_f0    = 0.0;

      model.calc_coefficients( *sc );

      sc->s      *= scorrec;
      sc->D      *= dcorrec;

      if ( sc->extinction > 0.0 )
         sc->molar_concentration = sc->signal_concentration / sc->extinction;
   }
}

// compress and average monte carlo model components
void US_FeMatch::adjust_mc_model()
{
   model_loaded       = model;
   int ncomp          = model.components.size();
//DbgLv(1) << "AMM: ncomp" << ncomp;

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
//if ( ii<3 || (ncomp-ii)<4 )
//DbgLv(1) << "AMM:  ii" << ii << " amx" << amx;
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
      {  // for multiples find average concentration; use modified componenta
         double cconc = mcomp.signal_concentration;
//DbgLv(1) << "AMM:  ii kdup" << ii << kdup << "  cconc0" << cconc;

         for ( int cc = 1; cc < kdup; cc++ )
         {
            int    kk    = mlistx[ ++ii ].section( ':', 2, 2 ).toInt();
            cconc       += model_loaded.components[ kk ].signal_concentration;
         }

         mcomp.signal_concentration = cconc / (double)kdup;
//DbgLv(1) << "AMM:      ii" << ii << " cconc" << cconc;

         model.components << mcomp;
      }
   }
//DbgLv(1) << "AMM:  kcomp" << model.components.size();
}

// load noise record(s) if there are any and user so chooses
void US_FeMatch::load_noise( )
{
   int         dd = lw_triples->currentRow();
   QStringList mieGUIDs;  // list of GUIDs of models-in-edit
   QStringList nimGUIDs;  // list of GUIDs:type:index of noises-in-models
   QStringList nieGUIDs;  // list of GUIDS:type:index of noises-in-edit
   QStringList tmpGUIDs;  // temporary noises-in-model list
   QString     editGUID  = dataList[ dd ].editGUID; // loaded edit GUID
   QString     modelGUID = model.modelGUID;         // loaded model GUID
   QString     lmodlGUID;                           // list model GUID
   QString     lnoisGUID;                           // list noise GUID
   QString     modelIndx;                           // "0001" style model index
DbgLv(1) << "editGUID  " << editGUID;
DbgLv(1) << "modelGUID " << modelGUID;

   // get a list of models tied to the loaded edit
   int nemods  = models_in_edit(  def_local, editGUID, mieGUIDs );

   if ( nemods == 0 )
      return;                 // go no further if no models in edit

   // get a list of noises tied to the loaded model
   int nmnois  = noises_in_model( def_local, modelGUID, nimGUIDs );

   // insure that the loaded model is at the head of the model-in-edit list
   if ( modelGUID != mieGUIDs[ 0 ] )
   {
      if ( ! mieGUIDs.removeOne( modelGUID ) )
      {
         qDebug( "*ERROR* Loaded model not in model-in-edit list!" );
         return;
      }

      mieGUIDs.insert( 0, modelGUID );
   }

   if ( nmnois > 0 )
   {  // if loaded model has noise, put noise in list
      nieGUIDs << nimGUIDs;   // initialize noise-in-edit list
   }

   int nenois  = nmnois;      // initial noise-in-edit count is noises in model

   for ( int ii = 1; ii < nemods; ii++ )
   {  // search through models in edit
      lmodlGUID  = mieGUIDs[ ii ];                    // this model's GUID
      modelIndx  = QString().sprintf( "%4.4d", ii );  // models-in-edit index

      // find the noises tied to this model
      int kenois = noises_in_model( def_local, lmodlGUID, tmpGUIDs );

      if ( kenois > 0 )
      {  // if we have 1 or 2 noises, add to noise-in-edit list
         nenois    += kenois;
         // adjust entry to have the right model-in-edit index
         lnoisGUID  = tmpGUIDs.at( 0 ).section( ":", 0, 1 )
            + ":" + modelIndx;
         nieGUIDs << lnoisGUID;
         if ( kenois > 1 )
         {  // add a second noise to the list
            lnoisGUID  = tmpGUIDs.at( 1 ).section( ":", 0, 1 )
               + ":" + modelIndx;
            nieGUIDs << lnoisGUID;
         }
      }
   }
DbgLv(1) << "nemods nmnois nenois" << nemods << nmnois << nenois;
for (int jj=0;jj<nenois;jj++)
 DbgLv(1) << " jj nieG" << jj << nieGUIDs.at(jj);

   if ( nenois > 0 )
   {  // There is/are noise(s):  ask user if she wants to load
      QMessageBox msgBox;
      QString     msg;

      if ( nenois > 1 )
         msg = tr( "There are noise files. Do you want to load them?" );

      else
         msg = tr( "There is a noise file. Do you want to load it?" );

      msgBox.setWindowTitle( tr( "Edit/Model Associated Noise" ) );
      msgBox.setText( msg );
      msgBox.setStandardButtons( QMessageBox::No | QMessageBox::Yes );
      msgBox.setDefaultButton( QMessageBox::Yes );

      if ( msgBox.exec() == QMessageBox::Yes )
      {  // user said "yes":  load noise
         US_DB2*   dbP = NULL;

         if ( !def_local )
         {
            US_Passwd pw;
            dbP          = new US_DB2( pw.getPasswd() );
         }

         if ( nenois > 1 )
         {  // more than 1:  get choice from noise loader dialog
            US_NoiseLoader* nldiag = new US_NoiseLoader( dbP,
               mieGUIDs, nieGUIDs, ti_noise, ri_noise );
            nldiag->move( this->pos() + QPoint( 200, 200 ) );
            nldiag->exec();
            qApp->processEvents();

            delete nldiag;
         }

         else
         {  // only 1:  just load it
            lnoisGUID     = nieGUIDs.at( 0 );
            QString typen = lnoisGUID.section( ":", 1, 1 );
            lnoisGUID     = lnoisGUID.section( ":", 0, 0 );

            if ( typen == "ti" )
               ti_noise.load( !def_local, lnoisGUID, dbP );

            else
               ri_noise.load( !def_local, lnoisGUID, dbP );
         }

         // noise loaded:  insure that counts jive with data
         int ntinois = ti_noise.values.size();
         int nrinois = ri_noise.values.size();
         int nscans  = dataList[ dd ].scanData.size();
         int npoints = dataList[ dd ].x.size();
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
      }
   }
}

// do model simulation
void US_FeMatch::simulate_model( )
{
   int    drow    = lw_triples->currentRow();
   US_SimulationParameters simparams;
   US_DataIO2::RawData*    rdata   = &rawList[  drow ];
   US_DataIO2::EditedData* edata   = &dataList[ drow ];
   US_DataIO2::Reading     reading;
   int    nscan   = rdata->scanData.size();
   int    nconc   = edata->x.size();
   double radlo   = edata->radius( 0 );
   double radhi   = edata->radius( nconc - 1 );
   double rmsd    = 0.0;
DbgLv(1) << " nscan nconc" << nscan << nconc;
DbgLv(1) << " radlo radhi" << radlo << radhi;
DbgLv(1) << " baseline plateau" << edata->baseline << edata->plateau;

   adjust_model();

   sdata          = new US_DataIO2::RawData();

   // initialize simulation parameters using edited data information
   simparams.initFromData( NULL, *edata );
DbgLv(1) << " initFrDat serial type coeffs" << simparams.rotorSerial
   << simparams.rotorType      << simparams.rotorcoeffs[0]
   << simparams.rotorcoeffs[1] << simparams.rotorcoeffs[2]
   << simparams.rotorcoeffs[3] << simparams.rotorcoeffs[4];

   simparams.meshType          = US_SimulationParameters::ASTFEM;
   simparams.gridType          = US_SimulationParameters::MOVING;
   simparams.radial_resolution = ( radhi - radlo ) / (double)( nconc - 1 );

   QString mtyp = adv_vals[ "meshtype" ];
   QString gtyp = adv_vals[ "gridtype" ];
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

   simparams.band_firstScanIsConcentration   = false;
DbgLv(1) << "  duration_hours  " << simparams.speed_step[0].duration_hours;
DbgLv(1) << "  duration_minutes" << simparams.speed_step[0].duration_minutes;
DbgLv(1) << "  delay_hours  " << simparams.speed_step[0].delay_hours;
DbgLv(1) << "  delay_minutes" << simparams.speed_step[0].delay_minutes;

   // make a simulation copy of the experimental data without actual readings

   //US_AstfemMath::initSimData( *sdata, *edata,
   //      model.components[ 0 ].signal_concentration );
   US_AstfemMath::initSimData( *sdata, *edata, 0.0 );

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
 simparams.save_simparms( US_Settings::appBaseDir() + "/etc/sp_fematch.xml" );

   compress  = le_compress->text().toDouble();

   if ( model.components[ 0 ].sigma == 0.0  &&
        model.components[ 0 ].delta == 0.0  &&
        model.coSedSolute           <  0.0  &&
        compress                    == 0.0 )
   {
      US_Astfem_RSA* astfem_rsa = new US_Astfem_RSA( model, simparams );
   
DbgLv(1) << " afrsa calc";
//astfem_rsa->setTimeCorrection( true );

      astfem_rsa->calculate( *sdata );
   }

   else
   {
      US_LammAstfvm *astfvm     = new US_LammAstfvm( model, simparams );

      astfvm->calculate(     *sdata );
   }

nscan = sdata->scanData.size();
nconc = sdata->x.size();
DbgLv(1) << " afrsa done M N" << nscan << nconc;
DbgLv(1) << "   sdata->x0" << sdata->radius(0);
DbgLv(1) << "   sdata->xN" << sdata->radius(nconc-1);
DbgLv(1) << "   sdata->c00" << sdata->value(0,0);
DbgLv(1) << "   sdata->c0N" << sdata->value(0,nconc-1);
DbgLv(1) << "   sdata->cM0" << sdata->value(nscan-1,0);
DbgLv(1) << "   sdata->cMN" << sdata->value(nscan-1,nconc-1);

   rmsd        = US_AstfemMath::variance( *sdata, *edata );
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

// write the report HTML text file
void US_FeMatch::write_rep( QString& htmlFile )
{
   int drow = lw_triples->currentRow();
   mkdir( US_Settings::reportDir(), edata->runID );

   htmlFile = US_Settings::reportDir() + "/" + edata->runID + "/fem_"
      + text_model( model, 0 ) + "." + edata->cell + wave_index( drow )
      + ".report.html";
   QFile rep_f( htmlFile );
   if ( ! rep_f.open( QIODevice::WriteOnly | QIODevice::Text ) )
      return;

   QTextStream ts( &rep_f );

   ts << "<html><head>\n";
   ts << "<style>td { padding-right: 1em;}</style>\n";
   ts << "</head><body>\n";
   ts << "<h1>" + text_model( model, 9 ) + "</h1>\n";
   ts << tr( "<h2>Data Report for Run \"" ) << edata->runID;
   ts << "\",<br>\n&nbsp;" << tr( " Cell " ) << edata->cell;
   ts << tr( ", Channel " ) << edata->channel;
   ts << tr( ", Wavelength " ) << edata->wavelength;
   ts << ",<br>\n&nbsp;" << tr( " Edited Dataset " );
   ts << edata->editID << "</h2>\n";

   ts << data_details();
   ts << hydrodynamics();
   ts << scan_info();
   ts << distrib_info();
   ts << "</body></html>\n";

   rep_f.close();
}

// format a wavelength index number string
QString US_FeMatch::wave_index( int drow )
{
   QString cwaveln = dataList[ drow ].wavelength;
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

// calculate average baseline absorbance
double US_FeMatch::calc_baseline( int drow ) const
{
   const US_DataIO2::EditedData* dd = &dataList[ drow ];
   const US_DataIO2::Scan* ss = &dd->scanData.last();
   int                     nn = US_DataIO2::index( *ss, dd->x, dd->baseline );
   double                  bl = 0.0;

   for ( int jj = nn - 5; jj < nn + 6; jj++ )
      bl += ss->readings[ jj ].value;

   return ( bl / 11.0 );
}

// model type text string
QString US_FeMatch::text_model( US_Model model, int width )
{
   QString title;

   switch ( (int)model.analysis )
   {
      case (int)US_Model::TWODSA:
         title = ( width == 0 ) ? "2dsa" :
            tr( "2-Dimensional Spectrum Analysis" );
         break;

      case (int)US_Model::TWODSA_MW:
         title = ( width == 0 ) ? "2dsa-mw" :
            tr( "2-Dimensional Spectrum Analysis" );
         break;

      case (int)US_Model::GA:
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

      case (int)US_Model::ONEDSA:
         title = ( width == 0 ) ? "1dsa" :
            tr( "1-Dimensional Spectrum Analysis" );
         break;

      case (int)US_Model::MANUAL:
      default:
         title = ( width == 0 ) ? "2dsa" :
            tr( "2-Dimensional Spectrum Analysis" );
         break;
   }

   if ( width == 0 )
   {  // short title (file node):  add any "ra", "gl" ,... "mc"

      if ( model.associations.size() > 1 )
         title = title + "-ra";

      if ( model.global == US_Model::MENISCUS )
         title = title + "-fm";

      else if ( model.global == US_Model::GLOBAL )
         title = title + "-gl";

      else if ( model.global == US_Model::SUPERGLOBAL )
         title = title + "-sg";

      if ( model.monteCarlo )
         title = title + "-mc";

   }

   else
   {  // long title:  add any suffixes and check need to center
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


// calculate residual absorbance values (data - sim - noise)
void US_FeMatch::calc_residuals()
{
   int     dsize  = edata->scanData[ 0 ].readings.size();
   int     ssize  = sdata->scanData[ 0 ].readings.size();
   double* xx     = new double[ dsize ];
   double* sx     = new double[ ssize ];
   double* sy     = new double[ ssize ];
   double  yval;
   double  sval;
   //double  rl     = edata->radius( 0 );
   //double  vh     = edata->value( scanCount - 1, dsize - 1 );
   double  rmsd   = 0.0;
   double  tnoi   = 0.0;
   double  rnoi   = 0.0;
   bool    ftin   = ti_noise.count > 0;
   bool    frin   = ri_noise.count > 0;

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
   }

   for ( int ii = 0; ii < scanCount; ii++ )
   {
      rnoi     = frin ? ri_noise.values[ ii ] : 0.0;

      for ( int jj = 0; jj < ssize; jj++ )
      {
         sy[ jj ] = sdata->value( ii, jj );
      }

      for ( int jj = 0; jj < dsize; jj++ )
      {
         tnoi          = ftin ? ti_noise.values[ jj ] : 0.0;
         sval          = interp_sval( xx[ jj ], sx, sy, ssize );
         yval          = edata->value( ii, jj ) - sval - rnoi - tnoi;
         //if ( xx[ jj ] < rl )
         //   yval          = 0.0;
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

// slot to make sure all windows and dialogs get closed
void US_FeMatch::close_all()
{
   if ( rbmapd )
      rbmapd->close();

   if ( eplotcd )
      eplotcd->close();

   close();
}

// build a list of models(GUIDs) for a given edit(GUID)
int US_FeMatch::models_in_edit( bool ondisk, QString eGUID, QStringList& mGUIDs )
{
   QString xmGUID;
   QString xeGUID;
   QString xrGUID;
   QStringList reGUIDs;

   mGUIDs.clear();
DbgLv(1) << "MIE: ondisk" << ondisk;

   if ( ondisk )
   {  // Models from local disk files
      QDir    dir;
      QString path = US_Settings::dataDir() + "/models";

      if ( !dir.exists( path ) )
         dir.mkpath( path );

      dir          = QDir( path );

      QStringList filter( "M*.xml" );
      QStringList f_names = dir.entryList( filter, QDir::Files, QDir::Name );

      QXmlStreamAttributes attr;

      for ( int ii = 0; ii < f_names.size(); ii++ )
      {
         QString fname( path + "/" + f_names[ ii ] );
         QFile   m_file( fname );

         if ( !m_file.open( QIODevice::ReadOnly | QIODevice::Text ) )
            continue;

         QXmlStreamReader xml( &m_file );


         while ( ! xml.atEnd() )
         {  // Search XML elements until we find "model"
            xml.readNext();

            if ( xml.isStartElement()  &&  xml.name() == "model" )
            {  // test for desired editGUID
               attr    = xml.attributes();
               xeGUID  = attr.value( "editGUID"    ).toString();
               xmGUID  = attr.value( "modelGUID"   ).toString();
               xrGUID  = attr.value( "requestGUID" ).toString();
               int kmc = attr.value( "monteCarlo"  ).toString().toInt();

               if ( xeGUID != eGUID )
                  continue;

               if ( kmc == 1  &&  xrGUID.length() == 36 )
               {  // treat monte carlo specially
                  if ( reGUIDs.contains( xrGUID ) )
                  {  // already have this request GUID:  skip
                     continue;
                  }
                  reGUIDs << xrGUID;  // this is 1st:  save it for compare
               }

               // save the GUID of each model with a matching edit GUID
               mGUIDs << xmGUID;
            }
         }

         m_file.close();
      }
   }

   else
   {
      US_Passwd pw;
      US_DB2    db( pw.getPasswd() );

      if ( db.lastErrno() != US_DB2::OK )
      {
         qDebug() << "*** DB ERROR: " << db.lastErrno();
         return 0;
      }

      QList< int > mDbIDs;
      QStringList  query;
      QString      invID  = investig.section( ":", 0, 0 );
DbgLv(1) << "MIE(db): invID" << invID;

      query.clear();

      query << "get_model_desc" << invID;
      db.query( query );

      while ( db.next() )
      {  // accumulate from db desc entries matching editGUID;
         xmGUID  = db.value( 1 ).toString();
         xeGUID  = db.value( 3 ).toString();
DbgLv(2) << "MIE(db): xm/xe/e GUID" << xmGUID << xeGUID << eGUID;

         if ( xeGUID == eGUID )
         {
            mGUIDs << xmGUID;
            mDbIDs << db.value( 0 ).toString().toInt();
         }
      }
DbgLv(1) << "MIE(db): pass 1 mGUIDs size" << mGUIDs.size() << mDbIDs.size();

      qSort( mDbIDs );            // sort model db IDs into ascending order

      // Make a pass thru models to exclude MC's beyond first

      for ( int ii = 0; ii < mDbIDs.size(); ii++ )
      {
         query.clear();
         query << "get_model_info" << QString::number( mDbIDs.at( ii ) );
         db.query( query );
         db.next();
         QString mxml = db.value( 2 ).toString();
         int     jj   = mxml.indexOf( "requestGUID="  );
         int     kk   = mxml.indexOf( "monteCarlo=" );
         xrGUID       = ( jj < 0 ) ? "" : mxml.mid( jj ).section( '"', 1, 1 );
         int     kmc  = ( kk < 0 ) ? 0 :
                        mxml.mid( kk ).section( '"', 1, 1 ).toInt();
DbgLv(2) << "MIE(db):  ii kmc rGlen" << ii << kmc << xrGUID.length()
 << " DbID" << mDbIDs.at( ii );

         if ( kmc == 1  &&  xrGUID.length() == 36 )
         {  // treat monte carlo specially

            if ( reGUIDs.contains( xrGUID ) )
            {  // already have this request GUID:  remove this model
               mGUIDs.removeOne( db.value( 0 ).toString() );
DbgLv(2) << "MIE(db):    mGI rmvd" << db.value( 0 ).toString();
            }

            else
            {  // this is 1st:  save it for compare
               reGUIDs << xrGUID;
DbgLv(2) << "MIE(db):    dsc savd" << db.value( 1 ).toString();
            }
         }
      }
DbgLv(1) << "MIE(db): pass 2 mGUIDs size" << mGUIDs.size() << mDbIDs.size();
   }

   return mGUIDs.size();
}

// build a list of noise(GUIDs) for a given model(GUID)
int US_FeMatch::noises_in_model( bool ondisk, QString mGUID,
      QStringList& nGUIDs )
{
   QString xnGUID;
   QString xmGUID;
   QString xntype;

   nGUIDs.clear();
   if ( ondisk )
   {  // Noises from local disk files
      QDir    dir;
      QString path = US_Settings::dataDir() + "/noises";

      if ( !dir.exists( path ) )
         dir.mkpath( path );

      dir          = QDir( path );

      QStringList filter( "N*.xml" );
      QStringList f_names = dir.entryList( filter, QDir::Files, QDir::Name );

      QXmlStreamAttributes attr;

      for ( int ii = 0; ii < f_names.size(); ii++ )
      {
         QString fname( path + "/" + f_names[ ii ] );
         QFile   m_file( fname );

         if ( !m_file.open( QIODevice::ReadOnly | QIODevice::Text ) )
            continue;

         QXmlStreamReader xml( &m_file );


         while ( ! xml.atEnd() )
         {  // Search XML elements until we find "noise"
            xml.readNext();

            if ( xml.isStartElement()  &&  xml.name() == "noise" )
            {  // test for desired editGUID
               attr    = xml.attributes();
               xmGUID  = attr.value( "modelGUID"   ).toString();
               xnGUID  = attr.value( "noiseGUID"   ).toString();
               xntype  = attr.value( "type"        ).toString();

               if ( xmGUID == mGUID )
                  nGUIDs << xnGUID + ":" + xntype + ":0000";
            }
         }

         m_file.close();
      }
   }

   else
   {
      US_Passwd pw;
      US_DB2    db( pw.getPasswd() );

      if ( db.lastErrno() != US_DB2::OK )
         return 0;

      QStringList query;
      QString     invID  = investig.section( ":", 0, 0 );
      QString     xnoiID;
      QString     xmodID;
      QString     modlID;

      query.clear();
      query << "get_modelID" << mGUID;
      db.query( query );
      db.next();
      modlID  = db.value( 0 ).toString();

      query.clear();
      query << "get_noise_desc" << invID;
      db.query( query );

      while ( db.next() )
      {  // accumulate from db desc entries matching editGUID;
         xnoiID  = db.value( 0 ).toString();
         xnGUID  = db.value( 1 ).toString();
         xmodID  = db.value( 3 ).toString();
         xntype  = db.value( 4 ).toString();
         xntype  = xntype.contains( "ri_nois", Qt::CaseInsensitive ) ?
                   "ri" : "ti";

//DbgLv(2) << "NIM(db): xm/xe/e ID" << xnoiID << xmodID << modlID;
         if ( xmodID == modlID )
            nGUIDs << xnGUID + ":" + xntype + ":0000";
      }
   }

   return nGUIDs.size();
}

// get buffer info from DB: ID, GUID, description
bool US_FeMatch::bufinfo_db( US_DataIO2::EditedData* edata,
      QString& svbar, QString& bufId, QString& bufGuid, QString& bufDesc )
{
   bool bufinfo = false;

   QStringList query;
   QString rawGUID  = edata->dataGUID;

   US_Passwd pw;
   US_DB2    db( pw.getPasswd() );

   query << "get_rawDataID_from_GUID" << rawGUID;
   db.query( query );
   if ( db.lastErrno() != US_DB2::OK )
   {
      qDebug() << "***Unable to get raw Data ID from GUID" << rawGUID
         << " lastErrno" << db.lastErrno();
      return bufinfo;
   }
   db.next();
   QString rawID    = db.value( 0 ).toString();
   QString soluID   = db.value( 2 ).toString();
QString expID=db.value(1).toString();
DbgLv(2) << "BInfD: rawGUID rawID expID soluID"
 << rawGUID << rawID << expID << soluID;

   query.clear();
   query << "get_solutionBuffer" << soluID;
   db.query( query );
   if ( db.lastErrno() != US_DB2::OK )
   {
      qDebug() << "***Unable to get solutionBuffer from soluID" << soluID
         << " lastErrno" << db.lastErrno();
      query.clear();
      query << "get_solutionIDs" << expID;
      db.query( query );
      db.next();
      soluID = db.value( 0 ).toString();
      query.clear();
      query << "get_solutionBuffer" << soluID;
      db.query( query );
      if ( db.lastErrno() != US_DB2::OK )
      {
         qDebug() << "***Unable to get solutionBuffer from soluID" << soluID
            << " lastErrno" << db.lastErrno();
      }
      else
         DbgLv(1) << "+++ Got solutionBuffer from soluID" << soluID;
      //return bufinfo;
   }
   db.next();
   QString id       = db.value( 0 ).toString();
   QString guid     = db.value( 1 ).toString();
   QString desc     = db.value( 2 ).toString();
DbgLv(2) << "BInfD: id guid desc" << id << guid << desc;

   if ( !id.isEmpty() )
   {
      bufId         = id;
      bufGuid       = guid.isEmpty() ? bufGuid : guid;
      bufDesc       = desc.isEmpty() ? bufDesc : desc;
      bufinfo       = true;
   }
 
   query.clear();
   query << "get_solution" << soluID;
   db.query( query );

   if ( db.lastErrno() != US_DB2::OK )
   {
      qDebug() << "***Unable to get solution vbar from soluID" << soluID
         << " lastErrno" << db.lastErrno();
   }
   else
   {
      db.next();
      svbar  = db.value( 2 ).toString();
      DbgLv(1) << "+++ Got solution vbar from soluID" << soluID
         << ": " << svbar;
   }

   return bufinfo;
}

// get buffer info from local disk: ID, GUID, description
bool US_FeMatch::bufinfo_disk( US_DataIO2::EditedData* edata,
   QString& svbar, QString& bufId, QString& bufGuid, QString& bufDesc )
{
   bool    bufinfo  = false;
   QString soluGUID = "";

   QString exppath = US_Settings::resultDir() + "/" + edata->runID + "/"
      + edata->runID + "." + edata->dataType + ".xml";

   QFile filei( exppath );
   if ( !filei.open( QIODevice::ReadOnly | QIODevice::Text ) )
      return bufinfo;

DbgLv(2) << "BInfL: runID dType" << edata->runID << edata->dataType;
   QXmlStreamReader xml( &filei );

   while ( ! xml.atEnd() )
   {
      xml.readNext();

      if ( xml.isStartElement() )
      {
         QXmlStreamAttributes ats = xml.attributes();

         if ( xml.name() == "buffer" )
         {
            QString id    = ats.value( "id"   ).toString();
            QString guid  = ats.value( "guid" ).toString();
            QString desc  = ats.value( "desc" ).toString();
       
            if ( ! id.isEmpty()  ||  ! guid.isEmpty() )
            {
               bufId         = id  .isEmpty() ? bufId   : id;
               bufGuid       = guid.isEmpty() ? bufGuid : guid;
               bufDesc       = desc.isEmpty() ? bufDesc : desc;
               bufinfo       = true;
               bufId         = bufId.isEmpty() ? "N/A"  : bufId;
            }
            break;
         }

         else if ( xml.name() == "solution" )
         {
            soluGUID      = ats.value( "guid" ).toString();
DbgLv(2) << "BInfL:   soluGUID" << soluGUID;
         }
      }
   }

   filei.close();

   if ( ! bufinfo  &&  ! soluGUID.isEmpty() )
   {  // no buffer info yet, but solution GUID found:  get buffer from solution
      QString spath = US_Settings::dataDir() + "/solutions";
      QDir    f( spath );
      spath         = spath + "/";
      QStringList filter( "S*.xml" );
      QStringList names = f.entryList( filter, QDir::Files, QDir::Name );
      QString fname;
      QString bdens;
      QString bvisc;
      QString bcomp;

      for ( int ii = 0; ii < names.size(); ii++ )
      {
         fname      = spath + names[ ii ];
         QFile filei( fname );

         if ( !filei.open( QIODevice::ReadOnly | QIODevice::Text ) )
            continue;

         QXmlStreamReader xml( &filei );

         while ( ! xml.atEnd() )
         {
            xml.readNext();

            if ( xml.isStartElement() )
            {
               QXmlStreamAttributes ats = xml.attributes();

               if (  xml.name() == "solution" )
               {
                  QString sguid = ats.value( "guid"         ).toString();
                  if ( sguid != soluGUID )
                     break;
                  svbar         = ats.value( "commonVbar20" ).toString();
                  DbgLv(1) << "+++ Got solution vbar" << svbar
                     << "from file, for GUID" << soluGUID;
               }

               else if (  xml.name() == "buffer" )
               {
                  QString bid   = ats.value( "id"   ).toString();
                  QString bguid = ats.value( "guid" ).toString();
                  QString bdesc = ats.value( "desc" ).toString();
       
                  if ( ! bid.isEmpty()  ||  ! bguid.isEmpty() )
                  {
                     bufId         = bid  .isEmpty() ? bufId   : bid;
                     bufId         = bufId.isEmpty() ? "N/A"   : bufId;
                     bufGuid       = bguid.isEmpty() ? bufGuid : bguid;
                     bufDesc       = bdesc.isEmpty() ? bufDesc : bdesc;
                     bufinfo       = true;
                  }
                  break;
               }
            }
            if ( bufinfo )
               break;
         }
         if ( bufinfo )
            break;
      }
   }

   return bufinfo;
}

// get buffer values from DB: density, viscosity, compressiblity
bool US_FeMatch::bufvals_db( QString& bufId, QString& bufGuid, QString& bufDesc,
      QString& dens, QString& visc, QString& comp )
{
   bool bufvals = false;

   US_Passwd pw;
   US_DB2    db( pw.getPasswd() );

   QStringList query;
   int idBuf     = bufId.isEmpty() ? -1    : bufId.toInt();
   bufId         = ( idBuf < 1  )  ? "N/A" : bufId;

   if ( bufId == "N/A"  &&  ! bufGuid.isEmpty() )
   {
      query.clear();
      query << "get_bufferID" << bufGuid;
      db.query( query );
      if ( db.lastErrno() != US_DB2::OK )
         qDebug() << "***Unable to get bufferID from GUID" << bufGuid
            << " lastErrno" << db.lastErrno();
      db.next();
      bufId         = db.value( 0 ).toString();
      bufId         = bufId.isEmpty() ? "N/A" : bufId;
   }

   if ( bufId != "N/A" )
   {
      query.clear();
      query << "get_buffer_info" << bufId;
      db.query( query );
      if ( db.lastErrno() != US_DB2::OK )
      {
         qDebug() << "***Unable to get buffer info from bufID" << bufId
            << " lastErrno" << db.lastErrno();
         return bufvals;
      }
      db.next();
      QString ddens = db.value( 5 ).toString();
      QString dvisc = db.value( 4 ).toString();
      QString dcomp = db.value( 2 ).toString();
      dens          = ddens.isEmpty() ? dens : ddens;
      visc          = dvisc.isEmpty() ? visc : dvisc;
      comp          = dcomp.isEmpty() ? comp : dcomp;
      bufvals       = true;
   }

   else
   {
      QString invID  = investig.section( ":", 0, 0 );
      query.clear();
      query << "get_buffer_desc" << invID;
      db.query( query );
      if ( db.lastErrno() != US_DB2::OK )
      {
         qDebug() << "***Unable to get buffer desc from invID" << invID
            << " lastErrno" << db.lastErrno();
         return bufvals;
      }

      while ( db.next() )
      {
         QString desc = db.value( 1 ).toString();
         
         if ( desc == bufDesc )
         {
            bufId         = db.value( 0 ).toString();
            break;
         }
      }

      if ( ! bufId.isEmpty() )
      {
         query.clear();
         query << "get_buffer_info" << bufId;
         db.query( query );
         if ( db.lastErrno() != US_DB2::OK )
         {
            qDebug() << "***Unable to get buffer info from bufID" << bufId
               << " lastErrno" << db.lastErrno();
            return bufvals;
         }
         db.next();
         QString ddens = db.value( 5 ).toString();
         QString dvisc = db.value( 4 ).toString();
         QString dcomp = db.value( 2 ).toString();
         dens          = ddens.isEmpty() ? dens : ddens;
         visc          = dvisc.isEmpty() ? visc : dvisc;
         comp          = dcomp.isEmpty() ? comp : dcomp;
         bufvals       = true;
      }
   }

   return bufvals;
}

// get buffer values from local disk: density, viscosity, compressiblity
bool US_FeMatch::bufvals_disk( QString& bufId, QString& bufGuid,
      QString& bufDesc, QString& dens, QString& visc, QString& comp )
{
   bool bufvals  = false;
   bool dfound   = false;
   QString bpath = US_Settings::dataDir() + "/buffers";
   QDir    f( bpath );
   bpath         = bpath + "/";
   QStringList filter( "B*.xml" );
   QStringList names = f.entryList( filter, QDir::Files, QDir::Name );
   QString fname;
   QString bdens;
   QString bvisc;
   QString bcomp;

   for ( int ii = 0; ii < names.size(); ii++ )
   {
      fname      = bpath + names[ ii ];
      QFile filei( fname );

      if ( !filei.open( QIODevice::ReadOnly | QIODevice::Text ) )
         continue;

DbgLv(2) << "  bvL: ii fname" << ii << names[ii];
      QXmlStreamReader xml( &filei );

      while ( ! xml.atEnd() )
      {
         xml.readNext();

         if ( xml.isStartElement()  &&  xml.name() == "buffer" )
         {
            QXmlStreamAttributes ats = xml.attributes();
            QString bid   = ats.value( "id"          ).toString();
            QString bguid = ats.value( "guid"        ).toString();
            QString bdesc = ats.value( "description" ).toString();

            if ( bguid == bufGuid  ||  bid == bufId )
            {
               bdens    = ats.value( "density"         ).toString();
               bvisc    = ats.value( "viscosity"       ).toString();
               bcomp    = ats.value( "compressibility" ).toString();
               dens     = bdens.isEmpty() ? dens : bdens;
               visc     = bvisc.isEmpty() ? visc : bvisc;
               comp     = bcomp.isEmpty() ? comp : bcomp;
               bufvals  = true;
DbgLv(2) << "  bvL:   i/g I/G dens" << bid << bguid << bufId << bufGuid << dens;
            }

            else if ( bdesc == bufDesc )
            {
               bdens    = ats.value( "density"         ).toString();
               bvisc    = ats.value( "viscosity"       ).toString();
               bcomp    = ats.value( "compressibility" ).toString();
               dfound   = true;
            }

            break;
         }
      }

      if ( bufvals )
         break;
   }

DbgLv(2) << "  bvL:    bufvals dfound" << bufvals << dfound;
   if ( ! bufvals  &&  dfound )
   {
      dens     = bdens.isEmpty() ? dens : bdens;
      visc     = bvisc.isEmpty() ? visc : bvisc;
      comp     = bcomp.isEmpty() ? comp : bcomp;
      bufvals  = true;
   }

   return bufvals;
}

// use dialogs to alert user to change in experiment buffer
bool US_FeMatch::verify_buffer( )
{
   bool changed = true;

   if ( buffLoaded )
   {  // only need verify buffer change while experiment values are loaded
      if ( QMessageBox::No == QMessageBox::warning( this,
               tr( "Warning" ),
               tr( "Attention:\n"
                   "You are attempting to override buffer parameters\n"
                   "that have been set from the experimental data!\n\n"
                   "Do you really want to override them?" ),
               QMessageBox::Yes, QMessageBox::No ) )
      {  // "No":  retain loaded values, mark unchanged
         QMessageBox::information( this,
            tr( "Buffer Retained" ),
            tr( "Buffer parameters from the experiment will be retained" ) );
         changed    = false;
      }

      else
      {  // "Yes":  change values,  mark experiment values no longer used
         QMessageBox::information( this,
            tr( "Buffer Overridden" ),
            tr( "Buffer parameters from the experiment will be overridden" ) );
         buffLoaded = false;
      }
   }

   return changed;
}

// slot to respond to text box change to buffer parameter
void US_FeMatch::buffer_text( )
{
   if ( buffLoaded )
   {  // only need verify desire to change while experiment values are loaded
      bool changed = verify_buffer();

      if ( changed )
      {  // "Yes" to change: pick up values as entered and turn off loaded flag
         buffLoaded   = false;
         density      = le_density  ->text().toDouble();
         viscosity    = le_viscosity->text().toDouble();
         compress     = le_compress ->text().toDouble();
      }

      else
      {  // "No" to change:  restore text and insure loaded flag still on
         buffLoaded   = false;
         le_density  ->setText( QString::number( density,   'f', 6 ) );
         le_viscosity->setText( QString::number( viscosity, 'f', 5 ) );
         le_compress ->setText( QString::number( compress,  'e', 3 ) );
         qApp->processEvents();
         buffLoaded   = true;
      }
   }
}

// use dialogs to alert user to change in experiment solution common vbar
bool US_FeMatch::verify_vbar( )
{
   bool changed = true;

   if ( buffLoaded )
   {  // only need verify vbar change while experiment values are loaded
      if ( QMessageBox::No == QMessageBox::warning( this,
               tr( "Warning" ),
               tr( "Attention:\n"
                   "You are attempting to override the vbar parameter\n"
                   "that has been set from the experimental data!\n\n"
                   "Do you really want to override it?" ),
               QMessageBox::Yes, QMessageBox::No ) )
      {  // "No":  retain loaded value, mark unchanged
         QMessageBox::information( this,
            tr( "Vbar Retained" ),
            tr( "Vbar parameter from the experiment will be retained" ) );
         changed    = false;
      }

      else
      {  // "Yes":  change value,  mark experiment values no longer used
         QMessageBox::information( this,
            tr( "Vbar Overridden" ),
            tr( "Vbar parameter from the experiment will be overridden" ) );
         buffLoaded = false;
      }
   }

   qApp->processEvents();
   return changed;
}

// slot to respond to text box change to vbar parameter
void US_FeMatch::vbar_text( )
{
   if ( buffLoaded )
   {  // only need verify desire to change while experiment values are loaded
      bool changed = verify_vbar();
      buffLoaded   = false;

      if ( changed )
      {  // "Yes" to change: pick up values as entered and turn off loaded flag
         vbar         = le_vbar->text().toDouble();
      }

      else
      {  // "No" to change:  restore text and insure loaded flag still on
         le_vbar->setText( QString::number( vbar, 'f', 5 ) );
         qApp->processEvents();
         buffLoaded   = true;
      }
   }
}

QString US_FeMatch::table_row( const QString& s1, const QString& s2 ) const
{
   QString s = "<tr><td>" + s1 + "</td><td>" + s2 + "</td></tr>\n";
   return s;
}

QString US_FeMatch::table_row( const QString& s1, const QString& s2, 
                                     const QString& s3 ) const
{
   QString s = "<tr><td>" + s1 + "</td><td>" + s2 + "</td><td>" + s3 
             + "</td></tr>\n";
   return s;
}

// Table HTML table row string for 5 columns
QString US_FeMatch::table_row( const QString& s1, const QString& s2,
                               const QString& s3, const QString& s4,
                               const QString& s5 ) const
{
   return "<tr><td>" + s1 + "</td><td>" + s2 + "</td><td>" + s3
       + "</td><td>" + s4 + "</td><td>" + s5 + "</td></tr>\n";
}

QString US_FeMatch::data_details( void ) const
{
   int    drow     = lw_triples->currentRow();
   const US_DataIO2::EditedData* d      = &dataList[ drow ];
   double baseline = calc_baseline( drow );

   QString s =  
        tr( "<h3>Detailed Run Information:</h3>\n" ) + "<table>\n" +
        table_row( tr( "Cell Description:" ), d->description )     +
        table_row( tr( "Data Directory:"   ), workingDir )         +
        table_row( tr( "Rotor Speed:"      ),  
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
        table_row( tr( "Baseline Absorbance:" ),
                   QString::number( baseline,    'f', 6 ) + " OD" ) + 
        table_row( tr( "Meniscus Position:     " ),           
                   QString::number( d->meniscus, 'f', 3 ) + " cm" );

   int    rrx   =  d->x.size() - 1;
   double left  =  d->x[ 0   ].radius;
   double right =  d->x[ rrx ].radius;

   s += table_row( tr( "Edited Data starts at: " ), 
                   QString::number( left,  'f', 3 ) + " cm" ) +
        table_row( tr( "Edited Data stops at:  " ), 
                   QString::number( right, 'f', 3 ) + " cm" ) + "</table>\n";
   return s;
}

QString US_FeMatch::hydrodynamics( void ) const
{
   // set up hydrodynamics values
   US_Math2::SolutionData solution = this->solution;
   solution.vbar      = le_vbar     ->text().toDouble();
   solution.density   = le_density  ->text().toDouble();
   solution.viscosity = le_viscosity->text().toDouble();
   double avgTemp     = le_temp     ->text().section( " ", 0, 0 ).toDouble();
   solution.vbar20    = solution.vbar;
   US_Math2::data_correction( avgTemp, solution );

   QString s = tr( "<h3>Hydrodynamic Settings:</h3>\n" ) + 
               "<table>\n";
  
   s += table_row( tr( "Viscosity corrected:" ), 
                   QString::number( solution.viscosity, 'f', 5 ) ) +
        table_row( tr( "Viscosity (absolute):" ),
                   QString::number( solution.viscosity_tb, 'f', 5 ) ) +
        table_row( tr( "Density corrected:" ),
                   QString::number( solution.density, 'f', 6 ) + " g/ccm" ) +
        table_row( tr( "Density (absolute):" ),
                   QString::number( solution.density_tb, 'f', 6 ) + " g/ccm" ) +
        table_row( tr( "Vbar:" ), 
                   QString::number( solution.vbar, 'f', 4 ) + " ccm/g" ) +
        table_row( tr( "Vbar corrected for 20 " ) + MLDEGC + ":",
                   QString::number( solution.vbar20, 'f', 4 ) + " ccm/g" ) +
        table_row( tr( "Buoyancy (Water, 20 " ) + MLDEGC + "): ",
                   QString::number( solution.buoyancyw, 'f', 6 ) ) +
        table_row( tr( "Buoyancy (absolute)" ),
                   QString::number( solution.buoyancyb, 'f', 6 ) ) +
        table_row( tr( "Correction Factor (s):" ),
                   QString::number( solution.s20w_correction, 'f', 6 ) ) + 
        table_row( tr( "Correction Factor (D):" ),
                   QString::number( solution.D20w_correction, 'f', 6 ) ) + 
        "</table>\n";

   return s;
}

QString US_FeMatch::scan_info( void ) const
{
   int                           drow   = lw_triples->currentRow();
   const US_DataIO2::EditedData* d      = &dataList[ drow ];
   double time_correction  = US_Math2::time_correction( dataList );

   QString s = tr( "<h3>Scan Information:</h3>\n" ) +
               "<table>\n"; 
         
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

   s += "</table>";
   
   return s;
}

// Distribution information HTML string
QString US_FeMatch::distrib_info() const
{
   int ncomp      = model_loaded.components.size();
   
   if ( ncomp == 0 )
      return "";

   QString mstr = tr( "<h3>Data Analysis Settings:</h3>\n" ) + "<table>\n";

   mstr += table_row( tr( "Number of Components:" ),
                      QString::number( ncomp ) );
   mstr += table_row( tr( "Residual RMS Deviation:" ),
                      le_rmsd->text()  );

   double sum_mw  = 0.0;
   double sum_s   = 0.0;
   double sum_D   = 0.0;
   double sum_c   = 0.0;

   for ( int ii = 0; ii < ncomp; ii++ )
   {
      double conc = model_loaded.components[ ii ].signal_concentration;
      sum_c      += conc;
      sum_mw     += ( model_loaded.components[ ii ].mw * conc );
      sum_s      += ( model_loaded.components[ ii ].s  * conc );
      sum_D      += ( model_loaded.components[ ii ].D  * conc );
   }

   mstr += table_row( tr( "Weight Average s20,W:" ),
                      QString().sprintf( "%6.4e\n", ( sum_s  / sum_c ) ) );
   mstr += table_row( tr( "Weight Average D20,W:" ),
                      QString().sprintf( "%6.4e\n", ( sum_D  / sum_c ) ) );
   mstr += table_row( tr( "W.A. Molecular Weight:" ),
                      QString().sprintf( "%6.4e\n", ( sum_mw / sum_c ) ) );
   mstr += table_row( tr( "Total Concentration:" ),
                      QString().sprintf( "%6.4e\n", sum_c ) );
   mstr += "</table>\n\n";

   mstr += tr( "<h3>Distribution Information:</h3>\n" ) + "<table>\n";
   mstr += table_row( tr( "Molecular Wt." ), tr( "S 20,W" ), tr( "D 20,W" ),
                      tr( "f / f0" ), tr( "Concentration" ) );

   for ( int ii = 0; ii < ncomp; ii++ )
   {
      double conc = model_loaded.components[ ii ].signal_concentration;
      double perc = 100.0 * conc / sum_c;
      mstr       += table_row(
            QString().sprintf( "%10.4e", model_loaded.components[ ii ].mw   ),
            QString().sprintf( "%10.4e", model_loaded.components[ ii ].s    ),
            QString().sprintf( "%10.4e", model_loaded.components[ ii ].D    ),
            QString().sprintf( "%10.4e", model_loaded.components[ ii ].f_f0 ),
            QString().sprintf( "%10.4e (%5.2f %%)", conc, perc       ) );
   }

   mstr += "</table>";

   return mstr;
}

void US_FeMatch::write_plot( const QString& filename, const QwtPlot* plot )
{
   if ( filename.endsWith( ".svg" ) )
   {  // standard SVG file
      QSvgGenerator generator;
      generator.setSize( plot->size() );
      generator.setFileName( filename );
      plot->print( generator );
   }

   else if ( filename.endsWith( "rbitmap.png" ) )
   {  // special case of rbitmap PNG
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
         qDebug() << "*ERROR* Unable to write file" << filename;
   }

   else if ( filename.endsWith( "3dplot.png" ) )
   {  // special case of 3dplot PNG
      if ( eplotcd == 0 )
      {  // if no 3d plot control up,  create it now
         eplotcd = new US_PlotControl( this, &model );
         eplotcd->move( epd_pos );
         eplotcd->show();
      }

      eplotcd->do_3dplot();

      QGLWidget* dataw = eplotcd->data_3dplot();

      QPixmap pixmap = dataw->renderPixmap( dataw->width(), dataw->height(),
                                            true  );

      if ( ! pixmap.save( filename ) )
         qDebug() << "*ERROR* Unable to write file" << filename;
   }

   else if ( filename.endsWith( ".png" ) )
   {  // general case of PNG
      int     iwid   = plot->width();
      int     ihgt   = plot->height();
      QPixmap pixmap = QPixmap::grabWidget( (QWidget*)plot, 0, 0, iwid, ihgt );

      if ( ! pixmap.save( filename ) )
         qDebug() << "*ERROR* Unable to write file" << filename;
   }
}

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

void US_FeMatch::new_triple( int trow )
{
   update( trow );

   reset();
   data_plot();
}

void US_FeMatch::load_progress()
{
   QString pmsg = te_desc->toPlainText();
   te_desc->setText( "<b>" + pmsg + "*</b>" );
   qApp->processEvents();
}

