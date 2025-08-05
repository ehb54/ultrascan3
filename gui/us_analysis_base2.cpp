//! \file us_analysis_base2.cpp

#include <QtSvg>

#include "us_analysis_base2.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_gui_util.h"
#include "us_run_details2.h"
#include "us_analyte_gui.h"
#include "us_buffer_gui.h"
#include "us_data_loader.h"
#include "us_noise_loader.h"
#include "us_loadable_noise.h"
#include "us_db2.h"
#include "us_passwd.h"
#include "us_solution_vals.h"
#include "us_solution_gui.h"
#include "us_report.h"
#if QT_VERSION < 0x050000
#define setSamples(a,b,c) setData(a,b,c)
#define setMinimum(a)  setMinValue(a)
#define setMaximum(a)  setMaxValue(a)
#endif

US_AnalysisBase2::US_AnalysisBase2() : US_Widgets()
{
   setPalette( US_GuiSettings::frameColor() );

   mainLayout      = new QGridLayout( this );
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
            tr( "Plot 1 Title" ),
            tr( "X-Axis Title" ),
            tr( "Y-Axis Title" ) );

   data_plot1->setMinimumSize( 600, 150 );

   plotLayout2 = new US_Plot( data_plot2,
            tr( "Plot 2 Title" ),
            tr( "X-Axis Title" ),
            tr( "Y-Axis Title" ),
            true, ".*in range", "rainbow" );

   data_plot2->setMinimumSize( 600, 150 );

   rightLayout->addLayout( plotLayout1 );
   rightLayout->addLayout( plotLayout2 );

   mainLayout->addLayout( leftLayout,  0, 0 );
   mainLayout->addLayout( rightLayout, 0, 1 );
   mainLayout->setColumnStretch( 0, 0 );
   mainLayout->setColumnStretch( 1, 99 );

   // Analysis buttons
   pb_load    = us_pushbutton( tr( "Load Experiment" ) );
   connect( pb_load, SIGNAL( clicked() ), SLOT( load() ) );
   pb_details = us_pushbutton( tr( "Run Details" ) );
   QLayout* lo_edlast =
                us_checkbox(   tr( "Latest Data Edit" ), ck_edlast, true ); 
   connect( pb_details, SIGNAL( clicked() ), SLOT( details() ) );

   disk_controls = new US_Disk_DB_Controls;

   pb_view    = us_pushbutton( tr( "View Data Report" ) );
   pb_save    = us_pushbutton( tr( "Save Data" ) );

   pb_details->setEnabled( false );
   pb_view   ->setEnabled( false );
   pb_save   ->setEnabled( false );

   int row = 0;
   analysisLayout->addWidget( pb_load,       row,   0, 1, 1 );
   analysisLayout->addWidget( pb_details,    row++, 1, 1, 1 );
   analysisLayout->addLayout( lo_edlast,     row,   0, 1, 1 );
   analysisLayout->addLayout( disk_controls, row++, 1, 1, 1 );
   analysisLayout->addWidget( pb_view,       row,   0, 1, 1 );
   analysisLayout->addWidget( pb_save,       row++, 1, 1, 1 );

   // Standard buttons
   pb_reset = us_pushbutton( tr( "Reset" ) );
   pb_help  = us_pushbutton( tr( "Help"  ) );
   pb_close = us_pushbutton( tr( "Close" ) );

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

   le_id      = us_lineedit( "", 0, true );
   le_temp    = us_lineedit( "", 0, true );

   te_desc    = us_textedit();
   lw_triples = us_listwidget();

   QFont        font( US_GuiSettings::fontFamily(), 
                      US_GuiSettings::fontSize() );
   QFontMetrics fm  ( font );

   int fontHeight = fm.lineSpacing();

   te_desc   ->setMaximumHeight( fontHeight * 2 + 12 );  // Add for border
   lw_triples->setMaximumHeight( fontHeight * 6 + 12 );

   us_setReadOnly( te_desc, true );

   row = 0;
   runInfoLayout->addWidget( lb_info   , row++, 0, 1, 2 );
   runInfoLayout->addWidget( lb_id     , row,   0 );
   runInfoLayout->addWidget( le_id     , row++, 1 );
   runInfoLayout->addWidget( lb_temp   , row,   0 );
   runInfoLayout->addWidget( le_temp   , row++, 1 );
   runInfoLayout->addWidget( te_desc   , row,   0, 2, 2 );
   row += 2;
   runInfoLayout->addWidget( lb_triples, row++, 0, 1, 2 );
   runInfoLayout->addWidget( lw_triples, row++, 0, 4, 2 );

   // Parameters

   pb_solution  = us_pushbutton( tr( "Solution" ) );
   connect( pb_solution, SIGNAL( clicked() ), SLOT( get_solution() ) );

   QLabel* lb_density   = us_label( tr( "Density (20" ) + DEGC + ")" );
   QLabel* lb_viscosity = us_label( tr( "Viscosity (20" ) + DEGC + ")" );
   QLabel* lb_vbar      = us_label( tr( "Vbar (20" ) + DEGC + ")" );
   QLabel* lb_skipped   = us_label( tr( "Non-Cleared:" ) );

   density      = DENS_20W;
   viscosity    = VISC_20W;
   vbar         = TYPICAL_VBAR;
   manual       = false;

   le_solution  = us_lineedit( tr( "(Experiment's solution)" ), 0, true );
   le_density   = us_lineedit( QString::number( density,   'f', 6 ), 0, true );
   le_viscosity = us_lineedit( QString::number( viscosity, 'f', 5 ), 0, true );
   le_vbar      = us_lineedit( QString::number( vbar,      'f', 5 ), 0, true );
   le_skipped   = us_lineedit( "0", 0, true );

   pb_solution ->setEnabled ( false );

   row = 0;
   parameterLayout->addWidget( pb_solution , row,   0 );
   parameterLayout->addWidget( le_solution , row++, 1, 1, 3 );
   parameterLayout->addWidget( lb_density  , row,   0 );
   parameterLayout->addWidget( le_density  , row,   1 );
   parameterLayout->addWidget( lb_viscosity, row,   2 );
   parameterLayout->addWidget( le_viscosity, row++, 3 );
   parameterLayout->addWidget( lb_vbar     , row,   0 );
   parameterLayout->addWidget( le_vbar     , row,   1 );
   parameterLayout->addWidget( lb_skipped  , row,   2 );
   parameterLayout->addWidget( le_skipped  , row++, 3 );

   // Analysis Controls
   QLabel* lb_analysis     = us_banner( tr( "Analysis Controls"  ) ); 
   QLabel* lb_smoothing    = us_label ( tr( "Data Smoothing:"    ) );
   QLabel* lb_boundPercent = us_label ( tr( "% of Boundary:"     ) ); 
   QLabel* lb_boundPos     = us_label ( tr( "Boundary Pos. (%):" ) ); 

   QLabel* lb_from         = us_label ( tr( "Scan focus from:" ) );
   QLabel* lb_to           = us_label ( tr( "to:"   ) );

   pb_exclude = us_pushbutton( tr( "Exclude Scan Range" ) );
   pb_exclude->setEnabled( false );
   connect( pb_exclude, SIGNAL( clicked() ), SLOT( exclude() ) );

   pb_reset_exclude = us_pushbutton( tr( "Reset Scan Range" ) );
   pb_reset_exclude->setEnabled( false );
   connect( pb_reset_exclude, SIGNAL( clicked() ), SLOT( reset_excludes() ) );

   ct_smoothing = us_counter( 2,  1,  50,  1 );
   ct_smoothing->setSingleStep( 1.0 );
   connect( ct_smoothing, SIGNAL( valueChanged( double ) ),
                          SLOT  ( smoothing   ( double ) ) );

   ct_boundaryPercent = us_counter( 3, 10, 100, 90 );
   ct_boundaryPos     = us_counter( 3,  0,  10,  5 );
   ct_boundaryPercent->setSingleStep( 0.1 );
   ct_boundaryPos    ->setSingleStep( 0.1 );
   connect( ct_boundaryPercent, SIGNAL( valueChanged( double ) ),
                                SLOT  ( boundary_pct( double ) ) );
   connect( ct_boundaryPos,     SIGNAL( valueChanged( double ) ),
                                SLOT  ( boundary_pos( double ) ) );
   
   ct_from            = us_counter( 3, 0, 0 );
   ct_to              = us_counter( 3, 0, 0 );
   
   connect( ct_from, SIGNAL( valueChanged( double ) ),
                     SLOT  ( exclude_from( double ) ) );
   connect( ct_to,   SIGNAL( valueChanged( double ) ),
                     SLOT  ( exclude_to  ( double ) ) );

   row = 0;
   controlsLayout->addWidget( lb_from           , row,   0, 1, 1 );
   controlsLayout->addWidget( ct_from           , row++, 1, 1, 1 );
   controlsLayout->addWidget( lb_to             , row,   0, 1, 1 );
   controlsLayout->addWidget( ct_to             , row++, 1, 1, 1 );
   controlsLayout->addWidget( pb_exclude        , row,   0, 1, 1 );
   controlsLayout->addWidget( pb_reset_exclude  , row++, 1, 1, 1 );
   controlsLayout->addWidget( lb_analysis       , row++, 0, 1, 2 );
   controlsLayout->addWidget( lb_smoothing      , row,   0, 1, 1 );
   controlsLayout->addWidget( ct_smoothing      , row++, 1, 1, 1 );
   controlsLayout->addWidget( lb_boundPercent   , row,   0, 1, 1 );
   controlsLayout->addWidget( ct_boundaryPercent, row++, 1, 1, 1 );
   controlsLayout->addWidget( lb_boundPos       , row,   0, 1, 1 );
   controlsLayout->addWidget( ct_boundaryPos    , row++, 1, 1, 1 );
   controlsLayout->setColumnStretch( 0, 1 );
   controlsLayout->setColumnStretch( 1, 1 );

   dataLoaded = false;
   buffLoaded = false;

   dfilter    = "";
   etype_filt = "velocity";

   setMaximumSize( qApp->desktop()->size() - QSize( 60, 60 ) );
   reset();
//qDebug() << "AB2: desktop size" << qApp->desktop()->size();
//qDebug() << "AB2: max main size" << maximumSize();
}

void US_AnalysisBase2::update_disk_db( bool db )
{
   ( db ) ? disk_controls->set_db() : disk_controls->set_disk();
}

void US_AnalysisBase2::load( void )
{
   // Determine the edit ID
   dataLoaded = false;
   dataList     .clear();
   rawList      .clear();
   excludedScans.clear();
   triples      .clear();
   savedValues  .clear();

   lw_triples->disconnect();
   lw_triples->clear();
   ct_from   ->disconnect();
   ct_from   ->setValue( 0 );


   bool edlast = ck_edlast->isChecked();
   int  dbdisk = ( disk_controls->db() ) ? US_Disk_DB_Controls::DB
                                         : US_Disk_DB_Controls::Disk;
   QString description;

   US_DataLoader* dialog = new US_DataLoader(
         edlast, dbdisk, rawList, dataList, triples, description, etype_filt );

   connect( dialog, SIGNAL( changed( bool ) ), SLOT( update_disk_db( bool ) ) );
   connect( dialog, SIGNAL( progress    ( const QString ) ), 
                    SLOT  ( set_progress( const QString ) ) );

   if ( dialog->exec() != QDialog::Accepted ) return;

   if ( disk_controls->db() )
      directory = tr( "(database)" );

   else
   {
      directory = description.section( description.left( 1 ), 4, 4 );
      directory = directory.left( directory.lastIndexOf( "/" ) );
   }

   for ( int ii=0; ii < triples.size(); ii++ )
      lw_triples->addItem( triples.at( ii ) );

   int nscans  = dataList[ 0 ].scanCount();

   // Save original readings values for each scan
   for ( int ii = 0; ii < nscans; ii++ )
      savedValues << dataList[ 0 ].scanData[ ii ].rvalues;

   noiflags.fill( -1,            dataList.size() );
   allExcls.fill( excludedScans, dataList.size() );
   rinoises.fill( US_Noise(),    dataList.size() );
   tinoises.fill( US_Noise(),    dataList.size() );

   connect( lw_triples, SIGNAL( currentRowChanged( int ) ), 
                        SLOT  ( new_triple       ( int ) ) );
   lw_triples->setCurrentRow( 0 );

   // Enable other buttons
   pb_solution->setEnabled( true );
   pb_details ->setEnabled( true );
   pb_view    ->setEnabled( true );
   pb_save    ->setEnabled( true );
   pb_exclude ->setEnabled( true );

   connect( ct_from, SIGNAL( valueChanged( double ) ),
                     SLOT  ( exclude_from( double ) ) );

   dataLoaded = true;
   emit dataAreLoaded();
   qApp->processEvents();
}

void US_AnalysisBase2::update( int selection )
{
   US_DataIO::EditedData* d = &dataList[ selection ];
   int scanCount = d->scanData.size();
   runID         = d->runID;
   le_id->setText( runID + " / " + d->editID );

   double avTemp = d->average_temperature();

   le_temp->setText( QString::number( avTemp, 'f', 1 ) + " " + DEGC );

   te_desc->setText( d->description );

   excludedScans = allExcls[ selection ];

   ct_from->setMaximum( scanCount - excludedScans.size() );
   ct_from->setSingleStep( 1.0 );
   ct_to  ->setMaximum( scanCount - excludedScans.size() );
   ct_to  ->setSingleStep( 1.0 );

   // Set up solution/buffer values implied from experimental data
   QString solID;
   QString bufID;
   QString bguid;
   QString bdesc;
   QString bdens  = le_density  ->text();
   QString bvisc  = le_viscosity->text();
   QString svbar  = le_vbar     ->text();
   QString bcomp  = "";
   QString bmanu  = "0";
   QString errmsg = "";
   US_Passwd pw;
   US_DB2*   dbP  = ( disk_controls->db() ) ?
                    new US_DB2( pw.getPasswd() ) : 0;

   bool    bufin  = US_SolutionVals::values( dbP, d, solID, svbar, bdens,
                                             bvisc, bcomp, bmanu, errmsg );

   if ( bufin )
   {
      buffLoaded  = false;

      le_density  ->setText( bdens );
      le_viscosity->setText( bvisc );
      density     = bdens.toDouble();
      viscosity   = bvisc.toDouble();
      manual      = ( !bmanu.isEmpty()  &&  bmanu == "1" );
      buffLoaded  = true;
      solution_rec.buffer.manual = manual;

      if ( solID.isEmpty() )
      {
         QMessageBox::warning( this, tr( "Solution/Buffer Values Fetch" ),
            tr( "Empty solution ID value!" ) );
      }

      else if ( solID.length() < 36  &&  dbP != NULL )
      {  // Have DB solution ID
         solution_rec.readFromDB( solID.toInt(), dbP );
      }

      else
      {  // Have Local solution GUID
         solution_rec.readFromDisk( solID );
      }

      le_solution ->setText( solution_rec.solutionDesc );
      vbar         = US_Math2::calcCommonVbar( solution_rec, 20.0 );
      svbar        = QString::number( vbar );
      le_vbar     ->setText( svbar );
   }

   else
   {
      QMessageBox::warning( this, tr( "Solution/Buffer Values Fetch" ),
         errmsg );
      solution_rec.commonVbar20 = vbar;
      le_solution ->setText( tr( "( ***Undefined*** )" ) );
   }

   if ( dbP != NULL )
   {
      delete dbP;
   }

   data_plot();
}

// Report data set details
void US_AnalysisBase2::details( void )
{
   US_RunDetails2* dialog
      = new US_RunDetails2( rawList, runID, directory, triples );
   dialog->exec();
   qApp->processEvents();
   delete dialog;
}

void US_AnalysisBase2::data_plot( void )
{
   int                     row  = lw_triples->currentRow();
   US_DataIO::EditedData* d     = &dataList[ row ];

   QString                        dataType = tr( "Absorbance" );
   if ( d->dataType == "RI"  &&
        !has_intensity_profile( d->runID, disk_controls->db() ) )
                                  dataType = tr( "Intensity" );
   if ( d->dataType == "WI" )     dataType = tr( "Intensity" );
   if ( d->dataType == "IP" )     dataType = tr( "Interference" );
   if ( d->dataType == "FI" )     dataType = tr( "Fluorescence" );


   dataPlotClear( data_plot2 );
   QString header = tr( "Velocity Data for\n") + d->runID + "  ("
         + d->cell + "/" + d->channel + "/" + d->wavelength + ")";
   data_plot2->setTitle( header );

   header = dataType + tr( " at " ) + d->wavelength + tr( " nm" );
   data_plot2->setAxisTitle( QwtPlot::yLeft, header );

   header = tr( "Radius (cm) " );
   data_plot2->setAxisTitle( QwtPlot::xBottom, header );

   us_grid( data_plot2 );

   int     scan_number = 0;
   int     from        = (int)ct_from->value();
   int     to          = (int)ct_to  ->value();

   int     scanCount   = d->scanCount();
   int     points      = d->pointCount();
   double  boundaryPct = ct_boundaryPercent->value() / 100.0;
   boundaryPct = ct_boundaryPercent->isEnabled() ? boundaryPct : 9.0;
   double  positionPct = ct_boundaryPos    ->value() / 100.0;
   double  baseline    = calc_baseline();

   QVector< double > rvec( points );
   QVector< double > vvec( points );
   double* r           = rvec.data();
   double* v           = vvec.data();

   // Calculate basic parameters for other functions
   time_correction    = US_Math2::time_correction( dataList );

   solution.density   = le_density  ->text().toDouble();
   solution.viscosity = le_viscosity->text().toDouble();
   solution.vbar20    = le_vbar     ->text().toDouble();
   solution.manual    = manual;
   double avgTemp     = d->average_temperature();
   solution.vbar      = US_Math2::calcCommonVbar( solution_rec, avgTemp );
   QList< QColor > mcolors;
   int nmcols         = plotLayout2->map_colors( mcolors );

   US_Math2::data_correction( avgTemp, solution );

   // Draw curves
   for ( int i = 0; i < scanCount; i++ )
   {
      if ( excludedScans.contains( i ) ) continue;

      scan_number++;
      bool highlight = scan_number >= from  &&  scan_number <= to;

      US_DataIO::Scan*  s = &d->scanData[ i ];

      double range       = s->plateau - baseline;
      double lower_limit = baseline    + range * positionPct;
      double upper_limit = lower_limit + range * boundaryPct;

      int j     = 0;
      int count = 0;

      // Plot each scan in (up to) three segments: below, in, and above
      // the specified boundaries
      while (  j < points  &&  s->rvalues[ j ] < lower_limit )
      {
         r[ count ] = d->xvalues[ j ];
         v[ count ] = s->rvalues[ j ];
         j++;
         count++;
      }

      QString       title; 
      QwtPlotCurve* c;

      if ( count > 1 )
      {
         title = tr( "Curve " ) + QString::number( i ) + tr( " below range" );
         c     = us_curve( data_plot2, title );

         if ( highlight )
            c->setPen( QPen( Qt::red ) );
         else
            c->setPen( QPen( Qt::cyan ) );
         
         c->setSamples( r, v, count );
      }

      count = 0;

      while (   j < points && s->rvalues[ j ] < upper_limit )
      {
         r[ count ] = d->xvalues[ j ];
         v[ count ] = s->rvalues[ j ];
         j++;
         count++;
      }

      if ( count > 1 )
      {
         title = tr( "Curve " ) + QString::number( i ) + tr( " in range" );
         c = us_curve( data_plot2, title );

         if ( highlight )
            c->setPen( QPen( Qt::red ) );
         else if ( nmcols > 0 )
            c->setPen( QPen( mcolors[ i % nmcols ] ) );
         else
            c->setPen( QPen( US_GuiSettings::plotCurve() ) );
         
         c->setSamples( r, v, count );
      }

      count = 0;

      while ( j < points )
      {
         r[ count ] = d->xvalues[ j ];
         v[ count ] = s->rvalues[ j ];
         j++;
         count++;
      }

      if ( count > 1 )
      {
         title = tr( "Curve " ) + QString::number( i ) + tr( " above range" );
         c = us_curve( data_plot2, title );

         if ( highlight )
            c->setPen( QPen( Qt::red ) );
         else
            c->setPen( QPen( Qt::cyan ) );
        
         c->setSamples( r, v, count );
      }
   }

   data_plot2->replot();

   return;
}

void US_AnalysisBase2::boundary_pct( double percent )
{
   ct_boundaryPos->disconnect();
   ct_boundaryPos->setMaximum( 100.0 - percent );

   ct_boundaryPos->setValue( ( 100.0 - percent ) / 2.0 );

   connect( ct_boundaryPos, SIGNAL( valueChanged( double ) ),
                            SLOT  ( boundary_pos( double ) ) );
   data_plot();
}

void US_AnalysisBase2::boundary_pos( double percent )
{
   ct_boundaryPercent->disconnect();
   ct_boundaryPercent->setMaximum( 100.0 - percent );

   connect( ct_boundaryPercent, SIGNAL( valueChanged( double ) ),
                                SLOT  ( boundary_pct( double ) ) );
   data_plot();
}

void US_AnalysisBase2::exclude_from( double from )
{
   double to = ct_to->value();

   if ( to < from )
   {
      ct_to->disconnect();
      ct_to->setValue( from );

      connect( ct_to,   SIGNAL( valueChanged( double ) ),
                        SLOT  ( exclude_to  ( double ) ) );
   }

   data_plot();
}

void US_AnalysisBase2::exclude_to( double to )
{
   double from = ct_from->value();

   if ( from > to )
   {
      ct_from->disconnect();
      ct_from->setValue( to );

      connect( ct_from, SIGNAL( valueChanged( double ) ),
                        SLOT  ( exclude_from( double ) ) );
   }

   data_plot();
}

void US_AnalysisBase2::exclude( void )
{
   double from = ct_from->value();
   double to   = ct_to  ->value();

   int                     displayedScan = 1; 
   int                     index         = lw_triples->currentRow();
   US_DataIO::EditedData*  d             = &dataList[ index ];
   int                     totalScans    = d->scanData.size();
   
   for( int i = 0; i < totalScans; i++ )
   {
      if ( excludedScans.contains( i ) ) continue;
      
      if ( displayedScan >= from  &&  displayedScan <= to ) excludedScans << i;
   
      displayedScan++;
   }

   ct_to->setValue( 0 );  // Resets both counters and replots

   ct_from->setMaximum( totalScans - excludedScans.size() );
   ct_to  ->setMaximum( totalScans - excludedScans.size() );

   allExcls[ index ] = excludedScans;
   pb_reset_exclude->setEnabled( true );
   data_plot();
}

void US_AnalysisBase2::reset_excludes( void )
{
   int                     index      = lw_triples->currentRow();
   US_DataIO::EditedData*  d          = &dataList[ index ];
   int                     totalScans = d->scanData.size();

   excludedScans.clear();
   le_skipped->setText( "0" );

   ct_from->setMaximum( totalScans );
   ct_to  ->setMaximum( totalScans );

   if ( ct_to->value() != 0 )
      ct_to ->setValue( 0 );
   else
      data_plot();

   pb_reset_exclude->setEnabled( false );
   allExcls[ index ] = excludedScans;
}

void US_AnalysisBase2::smoothing( double smoothCount )
{
   if ( ! dataLoaded ) return;

   int smoothPoints = (int) smoothCount;

   // Restore saved data
   int                     index  = lw_triples->currentRow();
   US_DataIO::EditedData*  dat    = &dataList[ index ];

   for ( int ii = 0; ii < dat->scanCount(); ii++ )
      dat->scanData[ ii ].rvalues  = savedValues[ ii ];
   
   // Smooth the data
   if ( smoothPoints > 1 )
   {
      QVector< double > xwvec( smoothPoints );
      QVector< double > ywvec( smoothPoints );
      x_weights = xwvec.data();
      y_weights = ywvec.data();
                  
      // Divide the count into 2 standard deviations
      double increment  = 2.0 / smoothCount;
      int    scanPoints = dat->pointCount();

      // Only calculate half a Gaussian curve, since the other side is symmetric
      for ( int jj = 0; jj < smoothPoints; jj++ ) 
      {
         x_weights[ jj ] = increment * jj;

         // Use a standard deviation of 0.7 to narrow the spread and multiply
         // by 0.7 to scale the result as an empirical weighting factor
         
         // Standard deviation = 0.7, mean = 0.0, point = 0.0;
         y_weights[ jj ] = 
            0.7 * US_Math2::normal_distribution( 0.7, 0.0, x_weights[ jj ] );
      }

      // For each scan
      for ( int ii = 0; ii < dat->scanData.size(); ii++ )
      {
         US_DataIO::Scan* scn  = &dat->scanData[ ii ];
         
         // Loop over all border point centers
         for ( int jj = 0; jj < smoothPoints; jj++ )
         {
            scn->rvalues[ jj ] = smooth_point( ii, jj, -1, smoothPoints );
         }
      
         // Now deal with all non-border points
         for ( int jj = smoothPoints; jj < scanPoints - smoothPoints - 1; jj++ )
         {
            scn->rvalues[ jj ] = smooth_point( ii, jj,  0, smoothPoints );
         }

         // Finally the points on the right border
         for ( int jj = scanPoints - smoothPoints - 1; jj < scanPoints; jj++ )
         {
            scn->rvalues[ jj ]
               = smooth_point( ii, jj,  1, smoothPoints, scanPoints );
         }
      } // END: scan loop
   
   } // END: smoothPoints > 1
   
   data_plot();
}

double US_AnalysisBase2::smooth_point( 
      int scan, int point, int type, int smoothPoints, int scanPoints )
{
   // type ==  0 means no reflection
   // type ==  1 means to reflect on the right
   // type == -1 means to reflect on the left

   double  sum      = 0.0;
   double  sum_y    = 0.0;
   int     start;
   int     stop;
   int     direction;

   // Sum all applicable points left of center
   if ( type == -1 ) // reflect left
   {
      start     = point + 1;
      stop      = point + smoothPoints;
      direction = 1;
   }
   else
   {
      start     = point - 1;
      stop      = point - smoothPoints;
      direction = -1;
   }

   // This is a bit complex because the test for leaving the loop
   // is different if we are incrementing or decrementing.
   
   int position = 0;
   int k        = start;
   
   while ( true )
   {
      position++;
      double value = savedValues[ scan ][ k ];

      if ( type ==  -1 )
      {
         if ( point - position < 0 ) // we need a reflected value
         {
            double dy = savedValues[ scan ][ k ] - savedValues[ scan ][ point ];
            value     = savedValues[ scan ][ point ] - dy;
         }
         else
            value     = savedValues[ scan ][ point - position ];
      }
      
      sum   += value * y_weights[ position ];
      sum_y +=         y_weights[ position ];

      if ( type == -1 )
      {
         if ( k > stop ) break;
      }
      else
      {
         if ( k <= stop ) break;
      }

      k += direction;
   }

   // Add the center point
   sum   += savedValues[ scan ][ point ] * y_weights[ 0 ];
   sum_y +=                                y_weights[ 0 ];
   
   // Sum all applicable points right of center
   if ( type == 1 ) // reflect right
   {
      start     = point - 1;
      stop      = point - smoothPoints;
      direction = -1;
   }
   else
   {
      start     = point + 1;
      stop      = point + smoothPoints;
      direction = 1;
   }

   position = 0;
   k        = start;

   while( true )
   {
      position++;
      double value = savedValues[ scan ][ k ];

      if ( type == 1 )
      {
         if ( point + position >= scanPoints ) // Need reflection
         {
            double dy = savedValues[ scan ][ k ] 
                      - savedValues[ scan ][ point ];
            value     = savedValues[ scan ][ point ] - dy;
         }
         else
            value     = savedValues[ scan ][ point + position ];
      }

      sum   += value * y_weights[ position ];
      sum_y +=         y_weights[ position ];

      if ( type == 1 )
      {
         if ( k <= stop ) break;
      }
      else
      {
         if ( k > stop ) break;
      }

      k += direction;
   }

   // Normalize by the sum of all weights that were used 
   return sum / sum_y;
}

void US_AnalysisBase2::reset( void )
{
   if ( ! dataLoaded ) return;

   excludedScans.clear();

   int index    = lw_triples->currentRow();
   density      = DENS_20W;
   viscosity    = VISC_20W;
   vbar         = TYPICAL_VBAR;
   manual       = false;

   le_density  ->setText( QString::number( density,   'f', 6 ) );
   le_viscosity->setText( QString::number( viscosity, 'f', 5 ) );
   le_vbar     ->setText( QString::number( vbar,      'f', 5 ) );
   le_skipped  ->setText( "0" );

   // Restore saved data
   if ( dataList.size() > 0 )
   {
      US_DataIO::EditedData*  dat    = &dataList[ index ];
   
      for ( int ii = 0; ii < dat->scanData.size(); ii++ )
      {
         dat->scanData[ ii ].rvalues = savedValues[ ii ];
      }
   }

   ct_from           ->disconnect();
   ct_to             ->disconnect();
   ct_smoothing      ->disconnect();
   ct_boundaryPercent->disconnect();
   ct_boundaryPos    ->disconnect();
   
   ct_from           ->setValue( 0 );
   ct_to             ->setValue( 0 );
   ct_smoothing      ->setValue( 1 );
   ct_boundaryPercent->setValue( 90 );
   ct_boundaryPos    ->setValue( 5 );

   connect( ct_from,            SIGNAL( valueChanged( double ) ),
                                SLOT  ( exclude_from( double ) ) );
                                
   connect( ct_to,              SIGNAL( valueChanged( double ) ),
                                SLOT  ( exclude_to  ( double ) ) );

   connect( ct_boundaryPercent, SIGNAL( valueChanged( double ) ),
                                SLOT  ( boundary_pct( double ) ) );

   connect( ct_boundaryPos,     SIGNAL( valueChanged( double ) ),
                                SLOT  ( boundary_pos( double ) ) );
   
   connect( ct_smoothing,       SIGNAL( valueChanged( double ) ),
                                SLOT  ( smoothing   ( double ) ) );

   allExcls[ index ] = excludedScans;

   update( index );
}

void US_AnalysisBase2::new_triple( int index )
{
   // Save the data for the new triple
   US_DataIO::EditedData*  dat = &dataList[ index ];
 
   // Test for noise data to substract from the experiment; apply if any
   load_noise( index );

   savedValues.clear();

   for ( int ii = 0; ii < dat->scanCount(); ii++ )
   {
      savedValues << dat->scanData[ ii ].rvalues;
   }

   // Update GUI elements and plot for selected triple
   update( index );

   // Make sure we have a reports directory for this runID
   QString repdir = US_Settings::reportDir() + "/" + dat->runID;
   QDir dir;
   if ( ! dir.exists( repdir ) )  dir.mkpath( repdir );
}

double US_AnalysisBase2::calc_baseline( void ) const
{
   int    row   = lw_triples->currentRow();
   const US_DataIO::Scan*
          scan  = &dataList[ row ].scanData.last();
   int    point = US_DataIO::index( dataList[ row ].xvalues, 
                                    dataList[ row ].baseline );
          point = ( point < 5 ) ? 5 : point;
   double sum   = 0.0;
   
   for ( int j = point - 5;  j < point + 6; j++ )
      sum += scan->rvalues[ j ];

   return sum / 11.0;
}

// String to accomplish line identation
QString US_AnalysisBase2::indent( const int spaces ) const
{
   return QString( " " ).leftJustified( spaces, ' ' );
}

// Table row HTML with 2 columns
QString US_AnalysisBase2::table_row( const QString& s1, const QString& s2 ) const
{
   return ( indent( 6 ) + "<tr><td>" + s1 + "</td><td>" + s2 + "</td></tr>\n" );
}

// Table row HTML with 3 columns
QString US_AnalysisBase2::table_row( const QString& s1, const QString& s2, 
                                     const QString& s3 ) const
{
   return ( indent( 6 ) + "<tr><td>" + s1 + "</td><td>" + s2 + "</td><td>"
            + s3 + "</td></tr>\n" );
}

// Table row HTML with 4 columns
QString US_AnalysisBase2::table_row( const QString& s1, const QString& s2, 
                                     const QString& s3, const QString& s4 ) const
{
   return ( indent( 6 ) + "<tr><td>" + s1 + "</td><td>" + s2 + "</td><td>"
            + s3 + "</td><td>" + s4 + "</td></tr>\n" );
}

// Table row HTML with 5 columns
QString US_AnalysisBase2::table_row( const QString& s1, const QString& s2, 
                                     const QString& s3, const QString& s4, 
                                     const QString& s5 ) const
{
   return ( indent( 6 ) + "<tr><td>" + s1 + "</td><td>" + s2 + "</td><td>"
            + s3 + "</td><td>" + s4 + "</td><td>" + s5 + "</td></tr>\n" );
}

// Table row HTML with 7 columns
QString US_AnalysisBase2::table_row( const QString& s1, const QString& s2, 
                                     const QString& s3, const QString& s4, 
                                     const QString& s5, const QString& s6, 
                                     const QString& s7 ) const
{
   return ( indent( 6 ) + "<tr><td>" + s1 + "</td><td>" + s2 + "</td><td>"
            + s3 + "</td><td>" + s4 + "</td><td>" + s5 + "</td><td>"
            + s6 + "</td><td>" + s7 + "</td></tr>\n" );
}

// Compose HTML header string
QString US_AnalysisBase2::html_header( const QString& title,
      const QString& head1 ) const
{
   QString ss = QString( "<?xml version=\"1.0\"?>\n" );
   ss  += "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\"\n";
   ss  += "                      \"http://www.w3.org/TR/xhtml1/DTD"
          "/xhtml1-strict.dtd\">\n";
   ss  += "<html xmlns=\"http://www.w3.org/1999/xhtml\""
          " xml:lang=\"en\" lang=\"en\">\n";
   ss  += "  <head>\n";
   ss  += "    <title> " + title + " </title>\n";
   ss  += "    <meta http-equiv=\"Content-Type\" content="
          "\"text/html; charset=iso-8859-1\"/>\n";
   ss  += "    <style type=\"text/css\" >\n";
   ss  += "      td { padding-right: 1em; }\n";
   ss  += "      body { background-color: white; }\n";
   ss  += "    </style>\n";
   ss  += "  </head>\n  <body>\n";
   ss  += "    <h1>" + head1 + "</h1>\n";
   return ss;
}

// Compose HTML header string
QString US_AnalysisBase2::html_header( const QString& title,
      const QString& head1, US_DataIO::EditedData* edata ) const
{
   QString ss = QString( "<?xml version=\"1.0\"?>\n" );
   ss  += "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\"\n";
   ss  += "                      \"http://www.w3.org/TR/xhtml1/DTD"
          "/xhtml1-strict.dtd\">\n";
   ss  += "<html xmlns=\"http://www.w3.org/1999/xhtml\""
          " xml:lang=\"en\" lang=\"en\">\n";
   ss  += "  <head>\n";
   ss  += "    <title> " + title + " </title>\n";
   ss  += "    <meta http-equiv=\"Content-Type\" content="
          "\"text/html; charset=iso-8859-1\"/>\n";
   ss  += "    <style type=\"text/css\" >\n";
   ss  += "      td { padding-right: 1em; }\n";
   ss  += "      body { background-color: white; }\n";
   ss  += "    </style>\n";
   ss  += "  </head>\n  <body>\n";
   ss  += "    <h1>" + head1 + "</h1>\n";
   ss  += indent( 4 ) + tr( "<h2>Data Report for Run \"" ) + edata->runID;
   ss  += "\",<br/>\n" + indent( 4 ) + "&nbsp;" + tr( " Cell " ) + edata->cell;
   ss  += tr( ", Channel " ) + edata->channel;
   ss  += tr( ", Wavelength " ) + edata->wavelength;
   ss  += ",<br/>\n" + indent( 4 ) + "&nbsp;" + tr( " Edited Dataset " );
   ss  += edata->editID + "</h2>\n";
 
   return ss;
}

QString US_AnalysisBase2::run_details( void ) const
{
   int                           index  = lw_triples->currentRow();
   const US_DataIO::EditedData*  d      = &dataList[ index ];

   QString s = "\n" + indent( 4 )
        + tr( "<h3>Detailed Run Information:</h3>\n" )
        + indent( 4 ) + "<table>\n"
        + table_row( tr( "Cell Description:" ), d->description )
        + table_row( tr( "Data Directory:"   ), directory )
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
   int minutes = (int)time_correction / 60;
   int seconds = (int)time_correction % 60;

   QString m   = ( minutes == 1 ) ? tr( " minute " ) : tr( " minutes " );
   QString sec = ( seconds == 1 ) ? tr( " second"  ) : tr( " seconds"  );

   s += table_row( tr( "Time Correction:" ), 
                   QString::number( minutes ) + m +
                   QString::number( seconds ) + sec );

   double duration = rawList.last().scanData.last().seconds;

   int hours = (int) duration / 3600;
   minutes   = (int) duration / 60 - hours * 60;
   seconds   = (int) duration % 60;
   QString                        dataType = tr( "Absorbance:" );
   if ( d->dataType == "RI" )     dataType = tr( "Intensity:" );
   if ( d->dataType == "WI" )     dataType = tr( "Intensity:" );
   if ( d->dataType == "IP" )     dataType = tr( "Interference:" );
   if ( d->dataType == "FI" )     dataType = tr( "Fluorescence:" );

   QString h;
   h   = ( hours   == 1 ) ? tr( " hour "   ) : tr( " hours " );
   m   = ( minutes == 1 ) ? tr( " minute " ) : tr( " minutes " );
   sec = ( seconds == 1 ) ? tr( " second"  ) : tr( " seconds" );

   s += table_row( tr( "Run Duration:" ),
                   QString::number( hours   ) + h + 
                   QString::number( minutes ) + m + 
                   QString::number( seconds ) + sec );

   // Wavelength, baseline, meniscus, range
   s += table_row( tr( "Wavelength:" ), d->wavelength + " nm" )  +
        table_row( tr( "Baseline " ) + dataType,
                   QString::number( calc_baseline(), 'f', 6 ) + " OD" ) + 
        table_row( tr( "Meniscus Position:" ),           
                   QString::number( d->meniscus, 'f', 3 ) + " cm" );

   int    rrx   =  d->xvalues.size() - 1;
   double left  =  d->xvalues[ 0   ];
   double right =  d->xvalues[ rrx ];

   s += table_row( tr( "Edited Data starts at:"  ), 
                   QString::number( left,  'f', 3 ) + " cm" ) +
        table_row( tr( "Edited Data stops at:"   ), 
                   QString::number( right, 'f', 3 ) + " cm" );
   s += indent( 4 ) + "</table>\n";
   return s;
}

QString US_AnalysisBase2::hydrodynamics( void ) const
{
   // Set up hydrodynamics values
   US_Math2::SolutionData solution = this->solution;
   solution.vbar20    = le_vbar     ->text().toDouble();
   solution.density   = le_density  ->text().toDouble();
   solution.viscosity = le_viscosity->text().toDouble();
   solution.manual    = manual;
   double avgTemp     = le_temp     ->text().section( " ", 0, 0 ).toDouble();
   solution.vbar      = US_Math2::calcCommonVbar( (US_Solution&)solution_rec, avgTemp );
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
                   QString::number( solution.vbar, 'f', 4 ) + " ccm/g" ) +
        table_row( tr( "Vbar corrected for 20 " ) + MLDEGC + ":",
                   QString::number( solution.vbar20, 'f', 4 ) + " ccm/g" ) +
        table_row( tr( "Buoyancy (Water, 20 " ) + MLDEGC + "):",
                   QString::number( solution.buoyancyw, 'f', 6 ) ) +
        table_row( tr( "Buoyancy (absolute):" ),
                   QString::number( solution.buoyancyb, 'f', 6 ) ) +
        table_row( tr( "Correction Factor (s):" ),
                   QString::number( solution.s20w_correction, 'f', 6 ) ) + 
        table_row( tr( "Correction Factor (D):" ),
                   QString::number( solution.D20w_correction, 'f', 6 ) ) + 
        indent( 4 ) + "</table>\n";

   return s;
}

QString US_AnalysisBase2::analysis( const QString& extra ) const
{
   QString s = "\n" + indent( 4 ) + tr( "<h3>Data Analysis Settings:</h3>\n" )
               + indent( 4 ) + "<table>\n";

   s += table_row( tr( "Smoothing Frame:" ),
                   QString::number( (int)ct_smoothing->value() ) );
   s += table_row( tr( "Analyzed Boundary:" ),
                   QString::number( (int)ct_boundaryPercent->value() ) + " %" );
   s += table_row( tr( "Boundary Position:" ),
                   QString::number( (int)ct_boundaryPos->value() ) + " %" );
   s += table_row( tr( "Early Non-Cleared Scans:" ),
                   le_skipped->text() + " scans" );
   s += extra;
   
   s += indent( 4 ) + "</table>\n";

   return s;
}

QString US_AnalysisBase2::scan_info( void ) const
{
   int                           index  = lw_triples->currentRow();
   const US_DataIO::EditedData*  d      = &dataList[ index ];

   QString s = "\n" + indent( 4 ) + tr( "<h3>Scan Information:</h3>\n" )
               + indent( 4 ) + "<table>\n"; 
         
   s += table_row( tr( "Scan" ), tr( "Corrected Time" ), 
                   tr( "Plateau Concentration" ),
                   tr( "Seconds" ), tr( "Omega^2T" ) );

   for ( int ii = 0; ii < d->scanData.size(); ii++ )
   {
      if ( excludedScans.contains( ii ) ) continue;

      QString s1;
      QString s2;
      QString s3;
      QString s4;
      QString s5;

      double od    = d->scanData[ ii ].plateau;
      double time  = d->scanData[ ii ].seconds;
      double omg2t = d->scanData[ ii ].omega2t;
      int    ctime = (int)( time - time_correction ); 

      s1 = QString::asprintf( "%4d",             ii + 1 );
      s2 = QString::asprintf( "%4d min %2d sec", ctime / 60, ctime % 60 );
      s3 = QString::asprintf( "%.6f OD",         od ); 
      s4 = QString::asprintf( "%5d",             (int)time ); 
      s5 = QString::asprintf( "%.5e",            omg2t ); 

      s += table_row( s1, s2, s3, s4, s5 );
   }

   s += indent( 4 ) + "</table>\n";
   
   return s;
}

bool US_AnalysisBase2::mkdir( const QString& baseDir, const QString& subdir )
{
   QDir folder( baseDir );

   if ( folder.exists( subdir ) ) return true;
           
   if ( folder.mkdir( subdir ) ) return true;
   
   QMessageBox::warning( this,
      tr( "File error" ),
      tr( "Could not create the directory:\n" ) + baseDir + "/" + subdir );
   
   return false;
}

// Slot to give load-data progress feedback
void US_AnalysisBase2::set_progress( const QString message )
{
   te_desc->setHtml( "<b>" + message + " ...</b>" );
   qApp->processEvents();
}

// Load noise record(s) if there are any and user so chooses, then apply
void US_AnalysisBase2::load_noise( int index )
{
   US_DataIO::EditedData*  edata = &dataList[ index ];
//qDebug() << "AB2: load_noise index noif" << index << noiflags[index];

   if ( noiflags[ index ] >= 0 )
   { // If noise already applied, ask user: retain? reselect?
      bool retain = query_noise_retain();

      if ( retain )  // Do nothing if user wants applied noise retained
         return;
      else           // Otherwise, back out noise and fall thru to re-select
         back_out_noise( index );
   }

   noiflags[ index ] = 0; // Initially flag no noise to subtract

   QStringList mieGUIDs;  // List of GUIDs of models-in-edit
   QStringList nieGUIDs;  // List of GUIDS:type:index of noises-in-edit

   te_desc->setHtml( tr( "<b>Scanning noise for %1 ...</b>" )
      .arg( triples[ index ] ) );
   qApp->processEvents();
   US_LoadableNoise lnoise;
   bool loadDB = disk_controls->db();
   bool local  = ! loadDB;
   int  noisdf = US_Settings::noise_dialog();
   int  nenois = lnoise.count_noise( local, edata, NULL, mieGUIDs, nieGUIDs );
   te_desc->setHtml( tr( "<b>%1 noise(s) found for %2</b>" )
      .arg( nenois ).arg( triples[ index ] ) );
   qApp->processEvents();

//for (int jj=0;jj<nenois;jj++)
// qDebug() << " jj nieG" << jj << nieGUIDs.at(jj);

   if ( nenois > 0 )
   {  // There is/are noise(s):  ask user if she wants to load
      US_Passwd pw;
      US_DB2* dbP  = local ? NULL : new US_DB2( pw.getPasswd() );

      if ( nenois > 1  &&  noisdf == 0 )
      {  // Noise exists and noise-dialog flag set to "Auto-load"
         QString descn = nieGUIDs.at( 0 );
         QString noiID = descn.section( ":", 0, 0 );
         QString typen = descn.section( ":", 1, 1 );
         QString mdlx1 = descn.section( ":", 2, 2 );

         if ( typen == "ti" )
            ti_noise.load( loadDB, noiID, dbP );

         else
            ri_noise.load( loadDB, noiID, dbP );

         descn         = nieGUIDs.at( 1 );
         QString mdlx2 = descn.section( ":", 2, 2 );
         int kenois    = ( mdlx1 == mdlx2 ) ? 2 : 1;

         if ( kenois == 2 )
         {  // Second noise from same model:  get it, too
            noiID         = descn.section( ":", 0, 0 );
            typen         = descn.section( ":", 1, 1 );

            if ( typen == "ti" )
               ti_noise.load( loadDB, noiID, dbP );

            else
               ri_noise.load( loadDB, noiID, dbP );
         }
      }

      else if ( nenois > 1  &&  noisdf > 0 )
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

      // Subtract noise from the experiment

      for ( int ii = 0; ii < nscans; ii++ )
      {
         int    iin    = min( ii, ( nrinois - 1 ) );
         double rinoi  = ( nrinois > 0 ) ? ri_noise.values[ iin ] : 0.0;
         US_DataIO::Scan*  escan = &edata->scanData[ ii ];

         for ( int jj = 0; jj < npoints; jj++ )
         {
            int    jjn    = min( jj, ( ntinois - 1 ) );
            double tinoi  = ( ntinois > 0 ) ? ti_noise.values[ jjn ] : 0.0;

            escan->rvalues[ jj ] = edata->value( ii, jj ) - rinoi - tinoi;
         }

         int plx        = US_DataIO::index( edata->xvalues, edata->plateau );
         escan->plateau = escan->rvalues[ plx ];
      }

      // Keep track of noise applied to this triple and save each
      noiflags[ index ] = min( nrinois, 1 ) + 2 * min( ntinois, 1 );

      rinoises[ index ] = nrinois > 0 ? ri_noise : US_Noise();
      tinoises[ index ] = ntinois > 0 ? ti_noise : US_Noise();

      if ( dbP != NULL )
      {
         delete dbP;
      }
   }  // End:  query for desired noise

   else                            // Flag that there was no noise to apply
      noiflags[ index ] = -1;
}

// Get solution parameters via US_SolutionGui
void US_AnalysisBase2::get_solution()
{
   if ( ! dataLoaded )
      return;

   int dbdisk = ( disk_controls->db() ) ? US_Disk_DB_Controls::DB
                                        : US_Disk_DB_Controls::Disk;
   int expID  = 0;
   QString runID = dataList[ lw_triples->currentRow() ].runID;

   if ( disk_controls->db() )
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
void US_AnalysisBase2::updateSolution( US_Solution solution_sel )
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
qDebug() << "updSolu: manual" << bmanu;
   
   if ( disk_controls->db() )
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
   vbar         = US_Math2::calcCommonVbar( solution_rec, 20.0 );
   svbar        = QString::number( vbar );
   manual       = ( !bmanu.isEmpty()  &&  bmanu == "1" );
   solution_rec.buffer.manual = manual;

   le_density  ->setText( bdens );
   le_viscosity->setText( bvisc );
   le_vbar     ->setText( svbar );
   le_solution ->setText( solution_rec.solutionDesc );
qDebug() << "updSolu:  reread manual" << manual << bmanu;
}

// Query whether user wants to retain already-applied noise
bool US_AnalysisBase2::query_noise_retain( )
{
   QString msg = tr(
         "Noise has previously been applied to this triple.<br/>"
         "Do you want to retain the previous noise selection?<br/>"
         "<ul><li><b>Yes</b> to retain the applied noise selection;</li>"
         "<li><b>No </b> to apply a new noise selection.</li></ul>" );

   QMessageBox msgBox( this );
   msgBox.setWindowTitle( tr( "Noise Already Applied" ) );
   msgBox.setTextFormat ( Qt::RichText );
   msgBox.setText       ( msg );
   msgBox.addButton     ( QMessageBox::No  );
   msgBox.addButton     ( QMessageBox::Yes );
   msgBox.setDefaultButton( QMessageBox::Yes );

   bool retain = ( msgBox.exec() == QMessageBox::Yes );

   return retain;
}

// Back out applied noise in preparation for possible reselection of noise
void US_AnalysisBase2::back_out_noise( int index )
{
   int noif     = noiflags[ index ];

   // Add noise back into data
   ri_noise     = ( ( noif & 1 ) != 0 ) ? rinoises[ index ] : US_Noise();
   ti_noise     = ( ( noif & 2 ) != 0 ) ? tinoises[ index ] : US_Noise();
   ri_noise.apply_to_data( dataList[ index ], false );
   ti_noise.apply_to_data( dataList[ index ], false );
}

// Copy report files to the database
void US_AnalysisBase2::reportFilesToDB( QStringList& files )
{
   US_Passwd   pw;
   US_DB2      db( pw.getPasswd() );
   US_DB2*     dbP  = &db;
   QStringList query;
   US_DataIO::EditedData* edata = &dataList[ lw_triples->currentRow() ];
   QString tripdesc = edata->description;
   QString pfdir    = QString( files[ 0 ] ).section( "/", 0, -2 );

   // Get the ID of the EditedData DB record associated with the report
   query << "get_editID" << edata->editGUID;
   db.query( query );
   db.next();
   int     idEdit = db.value( 0 ).toString().toInt();

   // Set the runID for the report
   US_Report freport;
   freport.runID  = runID;
qDebug() << "base2:rptFtoDB: idEdit" << idEdit << "runID" << runID;

   // Write all report records to the database
   int st = freport.saveFileDocuments( pfdir, files, dbP,
                                       idEdit, tripdesc );

   if ( st != US_DB2::OK )
   {
      qDebug() << "*ERROR* saveFileDocuments, status" << st;
   }
}

// Write a general dataset information HTML report file
bool US_AnalysisBase2::write_dset_report( QString& dsfname )
{
   QFile f_rep( dsfname );

   bool is_ok = f_rep.open( QIODevice::WriteOnly | QIODevice::Truncate );

   if ( ! is_ok )
      return is_ok;

   QTextStream ts( &f_rep );

   int                    index  = lw_triples->currentRow();
   US_DataIO::EditedData* edata  = &dataList[ index ];

   QString title = "US_Analysis_Base";
   QString head1 = tr( "General Data Set Information" );

   ts << html_header( title, head1, edata );
   ts << run_details();
   ts << hydrodynamics();
   ts << scan_info();
   ts << indent( 2 ) + "</body>\n</html>\n";

   f_rep.close();
   return is_ok;
}

// Update report files list, including adding PNG for each SVGZ
void US_AnalysisBase2::update_filelist( QStringList& flist,
                                        const QString fname )
{
   flist << fname;

   if ( fname.contains( ".svg" ) )
      flist << QString( fname ).section( ".", 0, -2 ) + ".png";
}

// Determine if a run has an intensity profile
bool US_AnalysisBase2::has_intensity_profile( const QString& runID,
                                              const bool in_db )
{
   QString ripxml;

   if ( in_db )
   {  // Data from DB:  get any RIProfile from experiment record
      US_Passwd pw;
      US_DB2 db( pw.getPasswd() );
      QStringList query;
      query << "get_experiment_info_by_runID" << runID
            << QString::number( US_Settings::us_inv_ID() );
      db.query( query );
      if ( db.lastErrno() == US_DB2::NOROWS )
         return false;                      // No DB records means no profile
      db.next();
      ripxml   = db.value( 16 ).toString(); // Otherwise get profile text
   }

   else
   {  // Data local:  read in any RIProfile from an XML file
      QString filename = US_Settings::resultDir() + "/" + runID + "/"
                         + runID + ".RIProfile.xml";

      QFile fi( filename );

      if ( !fi.open( QIODevice::ReadOnly | QIODevice::Text ) )
         return false;              // No file means no intensity profile

      ripxml  = fi.readAll();       // Otherwise read profile text
      fi.close();
   }

   return ! ripxml.isEmpty();
}

