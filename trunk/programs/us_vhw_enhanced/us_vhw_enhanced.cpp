//! \file us_vhw_enhanced.cpp

#include <QApplication>
#include <QtSvg>

#include "us_vhw_enhanced.h"
#include "us_license_t.h"
#include "us_license.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_matrix.h"
#include "us_model.h"
#include "us_passwd.h"
#include "us_constants.h"

#define SEDC_NOVAL   -9999.0

// main program
int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );

   #include "main1.inc"

   // License is OK.  Start up.
   
   US_vHW_Enhanced w;
   w.show();                   //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication
}

// US_vHW_Enhanced class constructor
US_vHW_Enhanced::US_vHW_Enhanced() : US_AnalysisBase2()
{
   // set up the GUI (mostly handled in US_AnalysisBase)

   setWindowTitle( tr( "Enhanced van Holde - Weischet Analysis:" ) );

   dbg_level     = US_Settings::us_debug();
   pb_dstrpl     = us_pushbutton( tr( "Distribution Plot" ) );
   pb_dstrpl->setEnabled( false );
   pb_selegr     = us_pushbutton( tr( "Select Groups" ) );
   groupSel      = false;
   pb_selegr->setEnabled( false );
   us_checkbox( tr( "Plateaus from 2DSA" ), ck_modelpl, true );
   pb_replot     = us_pushbutton( tr( "Refresh Plot" ) );
   pb_replot->setEnabled( false );
   us_checkbox( tr( "Manual-only Replot" ), ck_manrepl, false );
   us_checkbox( tr( "Use Enhanced vHW" ),   ck_vhw_enh, true  );

   connect( pb_dstrpl,  SIGNAL( clicked()       ),
            this,       SLOT(   distr_plot()    ) );
   connect( pb_selegr,  SIGNAL( clicked()       ),
            this,       SLOT(   sel_groups()    ) );
   connect( pb_replot,  SIGNAL( clicked()       ),
            this,       SLOT(   plot_refresh()  ) );
   connect( ck_modelpl, SIGNAL( toggled( bool ) ),
            this,       SLOT(   data_plot()     ) );
   connect( ck_vhw_enh, SIGNAL( toggled( bool ) ),
            this,       SLOT(   data_plot()     ) );

   int jr = 2;
   parameterLayout->addWidget( pb_dstrpl,  jr,   0, 1, 2 );
   parameterLayout->addWidget( pb_selegr,  jr++, 2, 1, 2 );
   parameterLayout->addWidget( ck_manrepl, jr,   0, 1, 2 );
   parameterLayout->addWidget( pb_replot,  jr++, 2, 1, 2 );
   parameterLayout->addWidget( ck_modelpl, jr,   0, 1, 2 );
   parameterLayout->addWidget( ck_vhw_enh, jr++, 2, 1, 2 );

   QLabel* lb_analysis     = us_banner( tr( "Analysis Controls"      ) );
   QLabel* lb_scan         = us_banner( tr( "Scan Control"           ) );
   QLabel* lb_smoothing    = us_label ( tr( "Data Smoothing:"        ) );
   QLabel* lb_boundPercent = us_label ( tr( "% of Boundary:"         ) );
   QLabel* lb_boundPos     = us_label ( tr( "Boundary Position (%):" ) );

   QLabel* lb_from         = us_label ( tr( "Scan focus from:" ) );
   QLabel* lb_to           = us_label ( tr( "to:"   ) );

   lb_tolerance  = us_label( tr( "Back Diffusion Tolerance:" ) );
   lb_tolerance->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );

   ct_tolerance  = us_counter( 3, 0.001, 1.0, 0.001 );
   bdtoler       = 0.001;
   ct_tolerance->setStep( 0.001 );
   ct_tolerance->setEnabled( true );
   connect( ct_tolerance, SIGNAL( valueChanged(   double ) ),
            this,          SLOT(  update_bdtoler( double ) ) );

   lb_division   = us_label( tr( "Divisions:" ) );
   lb_division->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );

   ct_division   = us_counter( 3, 0.0, 1000.0, 50.0 );
   ct_division->setStep( 1 );
   ct_division->setEnabled( true );
   connect( ct_division, SIGNAL( valueChanged(  double ) ),
            this,         SLOT(  update_divis(  double ) ) );

   jr     = 0;
   controlsLayout->addWidget( lb_analysis       , jr++, 0, 1, 4 );
   controlsLayout->addWidget( lb_tolerance      , jr,   0, 1, 2 );
   controlsLayout->addWidget( ct_tolerance      , jr++, 2, 1, 2 );
   controlsLayout->addWidget( lb_division       , jr,   0, 1, 2 );
   controlsLayout->addWidget( ct_division       , jr++, 2, 1, 2 );
   controlsLayout->addWidget( lb_smoothing      , jr,   0, 1, 2 );
   controlsLayout->addWidget( ct_smoothing      , jr++, 2, 1, 2 );
   controlsLayout->addWidget( lb_boundPercent   , jr,   0, 1, 2 );
   controlsLayout->addWidget( ct_boundaryPercent, jr++, 2, 1, 2 );
   controlsLayout->addWidget( lb_boundPos       , jr,   0, 1, 2 );
   controlsLayout->addWidget( ct_boundaryPos    , jr++, 2, 1, 2 );
   controlsLayout->addWidget( lb_scan           , jr++, 0, 1, 4 );
   controlsLayout->addWidget( lb_from           , jr,   0, 1, 2 );
   controlsLayout->addWidget( ct_from           , jr++, 2, 1, 2 );
   controlsLayout->addWidget( lb_to             , jr,   0, 1, 2 );
   controlsLayout->addWidget( ct_to             , jr++, 2, 1, 2 );
   controlsLayout->addWidget( pb_exclude        , jr,   0, 1, 2 );
   controlsLayout->addWidget( pb_reset_exclude  , jr++, 2, 1, 2 );

   connect( pb_help, SIGNAL( clicked() ),
            this,    SLOT(   help() ) );

   dataLoaded = false;
   haveZone   = false;
   forcePlot  = false;
   skipPlot   = false;

   rightLayout->setStretchFactor( plotLayout1, 3 );
   rightLayout->setStretchFactor( plotLayout2, 2 );

   setMaximumSize( qApp->desktop()->size() - QSize( 80, 80 ) );
}

// load data
void US_vHW_Enhanced::load( void )
{
   skipPlot   = true;
   US_AnalysisBase2::load();
   skipPlot   = false;

   if ( ! dataLoaded )
      return;

   if ( dataList[ 0 ].expType == "Equilibrium" )
   {
      QMessageBox::warning( this, tr( "Wrong Data Type" ),
            tr( "You have selected Equilibrium data, which is not\n"
                "appropriate for van Holde - Weischet analysis." ) );

      dataLoaded = false;
      lw_triples->disconnect();
      le_id     ->disconnect();
      le_temp   ->disconnect();
      te_desc   ->disconnect();

      lw_triples->clear();
      le_id     ->clear();
      le_temp   ->clear();
      te_desc   ->clear();

      data_plot1->clear();
      data_plot2->clear();

      dataList.clear();
      rawList .clear();
      triples .clear();
      pb_exclude->setEnabled( false );
      return;
   }

   connect( pb_save,    SIGNAL( clicked() ),
            this,       SLOT(   save_data() ) );
   connect( pb_view,    SIGNAL( clicked() ),
            this,       SLOT(   view_report() ) );

   data_plot1->setCanvasBackground( Qt::black );
   data_plot2->setCanvasBackground( Qt::black );
   int bord = height() - data_plot1->height() - data_plot2->height() + 12;
   int mxht = qApp->desktop()->height() - bord;
   int p1ht = ( mxht * 400 ) / 650;
   int p2ht = mxht - p1ht;
   p1ht     = qMin( p1ht, 400 );
   p2ht     = qMin( p2ht, 250 );
DbgLv(1) << "mxht p1ht p2ht" << mxht << p1ht << p2ht;
   data_plot1->setMinimumSize( 600, p1ht );
   data_plot2->setMinimumSize( 600, p2ht );
   data_plot2->setAxisAutoScale( QwtPlot::yLeft );
   data_plot2->setAxisAutoScale( QwtPlot::xBottom );

   gpick      = new US_PlotPicker( data_plot1 );
   gpick->setSelectionFlags( QwtPicker::PointSelection
                           | QwtPicker::ClickSelection );
   connect( gpick,    SIGNAL( mouseDown(  const QwtDoublePoint& ) ),
            this,       SLOT( groupClick( const QwtDoublePoint& ) ) );
   groupstep   = NONE;

   pb_selegr->setEnabled( true );
   pb_dstrpl->setEnabled( true );
   pb_replot->setEnabled( true );
   haveZone    = false;

   saved.clear();
   for ( int ii = 0; ii < triples.size(); ii++ )
      saved << false;

   update( 0 );
}

// distribution plot
void US_vHW_Enhanced::distr_plot(  void )
{
   QVector< double > bfracs;
   double  bterm    = 100.0 * boundPct / (double)divsCount;
   double  pterm    = 100.0 * positPct + bterm;
   int     npoints  = dseds.size();

   bfracs.resize( npoints );

   for ( int jj = 0; jj < npoints; jj++ )
   {
      bfracs[ jj ] = pterm + bterm * (double)( jj );
   }

   US_DistribPlot* dialog = new US_DistribPlot( bfracs, dseds );
   dialog->move( this->pos() + QPoint( 100, 100 ) );
   dialog->exec();
   delete dialog;
   row          = lw_triples->currentRow();
   saved[ row ] = true;
}

// data plot
void US_vHW_Enhanced::data_plot( void )
{
   double  xmax        = 0.0;
   double  ymax        = 0.0;
   int     count       = 0;
   int     totalCount;

DbgLv(1) << " data_plot: dataLoaded" << dataLoaded << "vbar" << vbar;
   if ( !dataLoaded  ||  vbar <= 0.0  ||  skipPlot )
      return;

   if ( !forcePlot  &&  ck_manrepl->isChecked() )
      return;

DbgLv(2) << "DP:TM:00: " << QTime::currentTime().toString("hh:mm:ss:zzzz");
   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
   row        = lw_triples->currentRow();
   edata      = &dataList[ row ];

   data_plot2->detachItems();
   data_plot2->setAxisAutoScale( QwtPlot::yLeft );
   data_plot2->setAxisAutoScale( QwtPlot::xBottom );
   scanCount  = edata->scanData.size();
   valueCount = edata->x.size();
   boundPct   = ct_boundaryPercent->value() / 100.0;
   positPct   = ct_boundaryPos    ->value() / 100.0;
   baseline   = calc_baseline();

   divsCount  = qRound( ct_division->value() );
   totalCount = scanCount * divsCount;
   QVector< double > pxvec( scanCount  );
   QVector< double > pyvec( totalCount );
   double* ptx   = pxvec.data();   // t,log(p) points
   double* pty   = pyvec.data();
   divfac     = 1.0 / (double)divsCount;
   correc     = solution.s20w_correction * 1.0e13;
   omega      = edata->scanData[ 0 ].rpm * M_PI / 30.0;

   // Get live scans and original plateaus
   live_scans();
DbgLv(1) << " valueCount totalCount" << valueCount << totalCount;
DbgLv(1) << "  scanCount divsCount" << scanCount << divsCount;
DbgLv(1) << "  lscnCount" << lscnCount;

   mo_plats    = ck_modelpl->isChecked() && have_model();
   vhw_enh     = ck_vhw_enh->isChecked();

   if (  mo_plats )
   {  // Calculate plateaus from a model
      model_plateaus();
   }

   else
   {  // Calculate plateaus from fitting to specified values
      fitted_plateaus();
   }

   // Let AnalysisBase do the lower plot
   US_AnalysisBase2::data_plot();

   // Then set up to handle the upper (vHW Extrapolation) plot.
   // Calculate the division-1 sedimentation coefficient intercept and,
   //  from that, the back diffusion coefficient

   bdiff_sedc = sedcoeff_intercept();

   bdiff_coef = back_diff_coeff( bdiff_sedc );

   init_partials();

   if ( vhw_enh )
   {  // Calculate all plot points using enhanced method
      vhw_calcs_enhanced( ptx, pty );
   }

   else
   {  // Calculate all plot points using standard method
      vhw_calcs_standard( ptx, pty );
   }

   // Remove points that are singles in a division
   for ( int jj = 0; jj < divsCount; jj++ )
   {  // Examine each division
      int count = 0;
      for ( int ii = 0; ii < lscnCount; ii++ )
      {  // Count unexcluded scan points in a division
         int    kk = divsCount * ii + jj;
         double xv = ptx[ ii ];
         double yv = pty[ kk ];
         if ( xv > 0.0  &&  yv > 0.0 )  count++;
      }

      if ( count == 1 )
      {  // Mark a division with a single point as having none
         int    kk = jj;
         for ( int ii = 0; ii < lscnCount; ii++, kk += divsCount )
            pty[ kk ] = SEDC_NOVAL;
      }
   }

   // Draw the vHW extrapolation plot
   data_plot1->clear();
   us_grid( data_plot1 );

   data_plot1->setTitle( tr( "Run " ) + runID + tr( ": Cell " )
         + edata->cell + " (" + edata->wavelength
         + tr( " nm) - vHW Extrapolation Plot" ) );

   data_plot1->setAxisTitle( QwtPlot::xBottom, tr( "(Time)^-0.5" ) );
   data_plot1->setAxisTitle( QwtPlot::yLeft  , 
         tr( "Corrected Sed. Coeff. (1e-13 s)" ) );

   int nxy    = ( lscnCount > divsCount ) ? lscnCount : divsCount;
   QVector< double > xvec( nxy );
   QVector< double > yvec( nxy );
   double* x  = xvec.data();
   double* y  = yvec.data();
   
   QwtPlotCurve* curve;
   QwtSymbol     sym;
   sym.setStyle( QwtSymbol::Ellipse );
   sym.setPen  ( QPen( Qt::blue ) );
   sym.setBrush( QBrush( Qt::white ) );
   sym.setSize ( 8 );
 
   int kk     = 0;                    // Index to sed. coeff. values

   // Set points for each division of each scan

//DbgLv(3) << "DP:TM:13: " << QTime::currentTime().toString("hh:mm:ss:zzzz");
   for ( int ii = 0; ii < lscnCount; ii++ )
   {
      count      = 0;
      double xv  = ptx[ ii ];         // Reciprocal square root of time value

      for ( int jj = 0; jj < divsCount; jj++ )
      {
         double yv  = pty[ kk++ ];    // Sed.coeff. values for divs in scan
         if ( xv >= 0.0  &&  yv >= 0.0 )
         {  // Points in a scan
            x[ count ] = xv;
            y[ count ] = yv;
            xmax       = ( xmax > xv ) ? xmax : xv;
            count++;
         }
      }

      if ( count > 0 )
      {  // Plot the points in a scan
         curve = us_curve( data_plot1,
               tr( "Sed Coeff Points, scan %1" ).arg( ii+1 ) );

         curve->setStyle ( QwtPlotCurve::NoCurve );
         curve->setSymbol( sym );
         curve->setData  ( x, y, count );
      }
   }
//DbgLv(3) << "DP:TM:14: " << QTime::currentTime().toString("hh:mm:ss:zzzz");

   double slope   = 0.0;
   double intcept = 0.0;

   // Fit lines for each division to all scan points

   for ( int jj = 0; jj < divsCount; jj++ )
   {  // Walk thru divisions, fitting line to points from all scans
      count          = 0;

      for ( int ii = 0; ii < lscnCount; ii++ )
      {
         kk         = ii * divsCount + jj;  // Sed. coeff. index

         if ( ptx[ ii ] > 0.0  &&  pty[ kk ] > 0.0 )
         {  // Points for scans in a division
            x[ count ] = ptx[ ii ];
            y[ count ] = pty[ kk ];
            count++;
            int js     = liveScans[ ii ];
            omega      = edata->scanData[ js ].rpm * M_PI / 30.0;
         }
      }

      if ( count > 1 )
      {  // Fit a line to the scan points in a division
if(jj<3||jj>(divsCount-9))
DbgLv(1) << "plot2 jj count" << jj << count << "  y0 yn" << y[0] << y[count-1];
         double sigma = 0.0;
         double correl;

         US_Math2::linefit( &x, &y, &slope, &intcept, &sigma, &correl, count );

         x[ 0 ] = 0.0;                      // X from 0.0 to max
         x[ 1 ] = xmax + 0.001;
         y[ 0 ] = intcept;                  // Y from intercept to y at x-max
         y[ 1 ] = y[ 0 ] + x[ 1 ] * slope;

         if ( y[ 1 ] < 0.0 )
         {
            y[ 1 ] = 0.0;
            x[ 1 ] = ( slope != 0.0 ? ( -y[ 0 ] / slope ) : xmax );
         }
if(jj<3||jj>(divsCount-9))
DbgLv(1) << "plot2 jj" << jj << "x0 y0 x1 y1" << x[0] << y[0] << x[1] << y[1];

         curve  = us_curve( data_plot1, tr( "Fitted Line %1" ).arg( jj ) );
         curve->setPen( QPen( Qt::yellow ) );
         curve->setData( x, y, 2 );
      }
   }
//DbgLv(3) << "DP:TM:15: " << QTime::currentTime().toString("hh:mm:ss:zzzz");

   // Set scales, then plot the points and lines
   xmax  *= 1.05;
   xmax   = (double)qRound( ( xmax + 0.0009 ) / 0.001 ) * 0.001;
   ymax   = (double)qRound( ( ymax + 0.3900 ) / 0.400 ) * 0.400;
   data_plot1->setAxisScale(     QwtPlot::xBottom, 0.0, xmax, 0.005 );
   data_plot1->setAxisAutoScale( QwtPlot::yLeft );

   count  = 0;

   for ( int ii = 0; ii < lscnCount; ii++ )
   {  // Accumulate the points of the back-diffusion cutoff line
      x[ count ] = bdrads.at( count );
      y[ count ] = bdcons.at( count );
DbgLv(2) << "   bd x y k " << x[count] << y[count] << count+1;
      count++;
   }

   // Save all sedcoeff values for report files and plots
   aseds.clear();
   for ( int ii = 0; ii < totalCount; ii++ )
      aseds << pty[ ii ];

   // Plot the red back-diffusion cutoff line
   dcurve  = us_curve( data_plot2, tr( "Fitted Line BD" ) );
   dcurve->setPen( QPen( QBrush( Qt::red ), 3.0 ) );
   dcurve->setData( x, y, count );
DbgLv(1) << " DP: xr0 yr0 " << x[0]       << y[0];
DbgLv(1) << " DP: xrN yrN " << x[count-1] << y[count-1] << count;
   data_plot2->replot();

   // Plot any upper plot vertical excluded-scan lines
   vert_exclude_lines(); 
   data_plot1->replot();

DbgLv(2) << "DP:TM:99: " << QTime::currentTime().toString("hh:mm:ss:zzzz");

   QApplication::restoreOverrideCursor();
}

// Write the main report HTML to a stream
void US_vHW_Enhanced::write_report( QTextStream& ts )
{
   edata      = &dataList[ lw_triples->currentRow() ];
   ts << html_header( "US_vHW_Enhanced",
         tr( "van Holde - Weischet Analysis" ), edata );
   ts << run_details();
   ts << hydrodynamics();
   ts << analysis( "" );

   ts << "\n" + indent( 4 ) + tr( "<h3>Selected Groups:</h3>\n" )
         + indent( 4 ) + "<table>\n";
   int ngrp = groupdat.size();

   if ( ngrp == 0 )
      ts << table_row( tr( "Groups Selected:" ), tr( "NONE" ) );

   else
   {
      ts << table_row( tr( "Group:" ), tr( "Average S:" ),
                       tr( "Relative Amount:" ) );

      double tsed = 0.0;

      for ( int jj = 0; jj < ngrp; jj++ )
      {
         ts << table_row(
               QString().sprintf( "%3d:", jj + 1 ),
               QString().sprintf( "%6.2f", groupdat.at( jj ).sed ),
               QString().sprintf( "(%5.2f %%)", groupdat.at( jj ).percent ) );
         tsed += groupdat.at( jj ).sed * groupdat.at( jj ).percent;
      }

      tsed  = tsed / ( (double)ngrp * 100.0 );
      Swavg = ( Swavg > 0.0 ) ? Swavg : ( tsed * 1.0e-13 );
   }

   ts << indent( 4 ) + "</table>\n\n";

   ts << indent( 4 ) + "<br/><table>\n";
   ts << table_row( tr( "Average S:" ),
                    QString::number( Swavg * 1.0e13 ) );
   ts << table_row( tr( "Initial Concentration from plateau fit:" ),
                     QString::number( C0 ) +  tr( " OD/fringes" ) );
   ts << indent( 4 ) + "</table>\n";

   double  sl;
   double  ci;
   double  sig;
   double  cor;
   QVector< double > xvec( lscnCount );
   QVector< double > yvec( lscnCount );
   double* x  = xvec.data();
   double* y  = yvec.data();
   QString tscn;
   QString tpla;

   ts << scan_info();

   for ( int ii = 0; ii < lscnCount; ii++ )
   {  // accumulate time,plateau pairs for line fit
      int js       = liveScans[ ii ];
      dscan        = &edata->scanData[ js ];
      x[ ii ]      = dscan->seconds;
      y[ ii ]      = scPlats[ ii ];
   }

   US_Math2::linefit( &x, &y, &sl, &ci, &sig, &cor, lscnCount );
   C0 = ( C0 == 0.0 ) ? ci : C0;

   ts << "\n" + indent( 4 ) + "<br/><table>\n";
   ts << table_row( tr( "Initial Concentration:   " ),
                    QString::number( ci ) + " OD" );
   ts << table_row( tr( "Correlation Coefficient: " ),
                    QString::number( cor ) );
   ts << table_row( tr( "Standard Deviation:      " ),
                    QString::number( sig ) );
   ts << table_row( tr( "Initial Concentration from exponential fit: " ),
                    QString::number( C0 ) + " OD" );
   ts << indent( 4 ) + "</table>\n";

   ts << "  </body>\n</html>\n";
}

// save the enhanced data
void US_vHW_Enhanced::save_data( void )
{
   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
QDateTime time0=QDateTime::currentDateTime();
   row           = lw_triples->currentRow();
   edata         = &dataList[ row ];
   QString tripl = QString( triples.at( row ) ).replace( " / ", "" );
   QString basernam  = US_Settings::resultDir() + "/" + edata->runID
      + "/vHW." + tripl + ".";
   QStringList files;
   QStringList repfiles;

   // Write results files
   write_vhw();
   write_dis();
   QString data0File = basernam + "extrap.csv";
   QString data1File = basernam + "s-c-distrib.csv";
   QString data2File = basernam + "s-c-envelope.csv";
   files << data0File;
   files << data1File;
   files << data2File;

   if ( groupdat.size() > 0 )
   {
      write_model();
      files << basernam + "fef_model.rpt";
   }

   // Set up to write reports files
   QString basename  = US_Settings::reportDir() + "/" + edata->runID
      + "/vHW." + tripl + ".";
   QString htmlFile  = basename + "report.html";
   QString plot1File = basename + "velocity.svg";
   QString plot2File = basename + "extrap.svg";
   QString plot3File = basename + "s-c-distrib.svg";
   QString plot4File = basename + "s-c-histo.svg";

   // Write the main report file
   QFile rpt_f( htmlFile );

   if ( !rpt_f.open( QIODevice::WriteOnly | QIODevice::Text ) )
      return;

   QTextStream ts( &rpt_f );
   write_report( ts );
   rpt_f.close();

   // Write the velocity and extrapolation plots
   write_plot( plot1File, data_plot2 );
   write_plot( plot2File, data_plot1 );

   // Save distribution and histogram plots
   QVector< double > bfracs;
   double  bterm    = 100.0 * boundPct / (double)divsCount;
   double  bfrac    = 100.0 * positPct;
   int     npoints  = dseds.size();
   bfracs.resize( npoints );

   for ( int jj = 0; jj < npoints; jj++ )
   {
      bfrac       += bterm;
      bfracs[ jj ] = bfrac;
   }

   if ( saved[ row ] == false )
   {  // If no plot dialog was opened for this triple, get files now
      US_DistribPlot* dialog = new US_DistribPlot( bfracs, dseds );
DbgLv(1) << "(P)PLOT ENV save: plot3File" << plot3File;
      dialog->save_plots( plot3File, plot4File );

      delete dialog;
   }

   else
   {  // Otherwise, copy the temporary files created earlier
DbgLv(1) << "(T)PLOT ENV save: plot3File" << plot3File;
      copy_data_files( plot3File, plot4File, data2File );
   }

   files << " ";
   files << htmlFile;
   files << plot1File;
   files << plot2File;
   files << plot3File;
   files << plot4File;
   repfiles << htmlFile;
   repfiles << plot1File;
   repfiles << plot2File;
   repfiles << plot3File;
   repfiles << plot4File;
   repfiles << data0File;
   repfiles << data1File;
   repfiles << data2File;

   // Report files created to the user
   QString wmsg = tr( "In directories\n%1,\n%2;\nwrote:\n\n" )
      .arg( files[ 0 ].left( files[ 0 ].lastIndexOf( "/" ) ) )
      .arg( plot4File.left( plot4File.lastIndexOf( "/" ) ) );

   for ( int ii = 0; ii < files.size(); ii++ )
   {
      QString fname = files[ ii ];
      fname         = fname.mid( fname.lastIndexOf( "/" ) + 1 );
      wmsg += "    " + fname + "\n";
   }

QDateTime time1=QDateTime::currentDateTime();
   if ( disk_controls->db() )
   {  // Also save report files to the database
      reportFilesToDB( repfiles );

      wmsg += tr( "\nFiles were also saved to the database." );
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

   QMessageBox::information( this, tr( "Successfully Written" ), wmsg );
}

// generate result report, pop up dialog and display the report
void US_vHW_Enhanced::view_report( void )
{
   // Generate the main report html text
   QString rtext;
   QTextStream ts( &rtext );
   write_report( ts );

   // Create the report dialog and display the text
   US_Editor* edit = new US_Editor( US_Editor::LOAD, true );
   edit->setWindowTitle( "Results:  van Holde - Weischet Analysis" );
   edit->move( this->pos() + QPoint( 100, 100 ) );
   edit->resize( 700, 600 );
   edit->e->setFont( QFont( US_GuiSettings::fontFamily(),
                            US_GuiSettings::fontSize() ) );
   edit->e->setText( rtext );
   edit->show();
}

// alternate between selecting set-up and clearing vH-W groups
void US_vHW_Enhanced::sel_groups(  void )
{
   if ( groupSel )
   {  // had been Select, now doing Clear
      groupstep = NONE;
      pb_selegr->setText( tr( "Select Groups" ) );

      data_plot1->detachItems( QwtPlotItem::Rtti_PlotMarker );
      data_plot1->replot();
      groupdat.clear();
      groupxy.clear();
   }

   else
   {  // had been Clear, now doing new Select
      groupstep = START;
      pb_selegr->setText( tr( "Clear Groups" ) );
      groupxy.clear();

      QMessageBox::information( this, tr( "Group Selection" ),
            tr( "Please click first ABOVE, then BELOW the intercepts on the\n"
                "Y-axis of the van Holde - Weischet extrapolation plot to\n"
                "define groups and to average the S-values within a group.\n\n"
                "PLEASE NOTE:\n"
                "This algorithm provides for multiple and also overlapping\n"
                "groups, such that the total percentage may exceed 100%" ) );
   }

   groupSel  = !groupSel;    // reverse Select/Clear mode
}

// update density
void US_vHW_Enhanced::update_density(  double dval )
{
   density   = dval;
}

// update viscosity
void US_vHW_Enhanced::update_viscosity( double dval )
{
   viscosity  = dval;
}

// update vbar
void US_vHW_Enhanced::update_vbar(      double  dval )
{
   vbar       = dval;
}


void US_vHW_Enhanced::update_bdtoler(    double dval )
{
   bdtoler   = dval;

   data_plot();
}

void US_vHW_Enhanced::update_divis(      double dval )
{ 
   divsCount = qRound( dval );

   data_plot();
}

// Index to first readings value greater than or equal to given concentration
int US_vHW_Enhanced::first_gteq( double concenv,
      QVector< US_DataIO2::Reading >& readings, int valueCount, int defndx )
{
   int index = defndx;                           // Set return index to default

   for ( int jj = 0; jj < valueCount; jj++ )
   {  // Find index where readings value equals or exceeds given concentration
      if ( readings[ jj ].value >= concenv )
      {
         index     = jj;                         // Set index to found point
         break;                                  // And return with it
      }
   }
   return index;
}

// Index to first greater-or-equal value (with default of -1)
int US_vHW_Enhanced::first_gteq( double concenv,
      QVector< US_DataIO2::Reading >& readings, int valueCount )
{
   return first_gteq( concenv, readings, valueCount, -1 );
}

// Get average scan plateau value for 41 points around user-specified value
double US_vHW_Enhanced::avg_plateau( )
{
   double plato  = 0.0;
   int j2  = US_DataIO2::index( edata->x, edata->plateau ); // Index plat radius
   int j1  = max( 0, ( j2 - PA_POINTS ) );          // Point to 20 points before
       j2  = min( valueCount, ( j2 + PA_POINTS + 1 ) ); // and 20 points after

   for ( int jj = j1; jj < j2; jj++ )        // Sum 41 points centered at the
      plato += dscan->readings[ jj ].value;  //  scan plateau radial position

   plato /= (double)( j2 - j1 );         // Find and return the average

   return plato;
}

// Get sedimentation coefficient for a given concentration
double US_vHW_Enhanced::sed_coeff( double cconc, double oterm,
      double* radP, int* ndxP )
{
   int    j2   = first_gteq( cconc, dscan->readings, valueCount );
   double rv0  = -1.0;          // Mark radius excluded
   double sedc = SEDC_NOVAL;
//DbgLv(2) << "sed_coeff: cconc rdv0 rdcn" << cconc << s->readings[0].value
//  << s->readings[valueCount-1].value << " j2" << j2;

   if ( j2 >= 0  &&  oterm >= 0.0 )
   {  // Likely need to interpolate radius from two input values
      int j1      = j2 - 1;

      if ( j2 > 0 )
      {  // Interpolate radius value
         double av1  = dscan->readings[ j1 ].value;
         double av2  = dscan->readings[ j2 ].value;
         double rv1  = edata->x[ j1 ].radius;
         double rv2  = edata->x[ j2 ].radius;
         double rra  = av2 - av1;
         rra         = ( rra == 0.0 ) ? 0.0 : ( ( rv2 - rv1 ) / rra );
         rv0         = rv1 + ( cconc - av1 ) * rra;
         j2          = ( av2 - cconc ) > ( cconc - av1 ) ? j1 : j2;
      }

      else
      {
         rv0         = -1.0;
         j2          = -1;
      }
   }

   if ( rv0 > 0.0 )
   {  // Use radius and other terms to get corrected sedimentation coeff. value
      sedc        = correc * log( rv0 / edata->meniscus ) / oterm;
DbgLv(2) << "sed_coeff:   rv0" << rv0 << " sedc" << sedc << " oterm" << oterm;
   }

   if ( radP != NULL )    *radP = rv0;
   if ( ndxP != NULL )    *ndxP = j2;
   return sedc;
}

// Calculate division sedimentation coefficient values (fitted line intercepts)
void US_vHW_Enhanced::div_seds( )
{
   QVector< double > xxv( lscnCount );
   QVector< double > yyv( lscnCount );
   QVector< double > xrv( lscnCount );
   QVector< double > yrv( lscnCount );
   double* xx       = xxv.data();
   double* yy       = yyv.data();
   double* xr       = xrv.data();
   double* yr       = yrv.data();
   int     nscnu    = 0;  // Number used (non-excluded) scans
   int     kscnu    = 0;  // Count of scans of div not affected by diffusion
   double  bdifcsqr = sqrt( bdiff_coef );  // Sqrt( diff_coeff ) used below
   double  pconc;
   double  mconc;
   bdtoler          = ct_tolerance->value();
   valueCount       = edata->x.size();
   double  bottom   = edata->x[ valueCount - 1 ].radius;

   // Do division-1 determination of base

//DbgLv(3) << "  DS:TM:01: " << QTime::currentTime().toString("hh:mm:ss:zzzz");
   for ( int ii = 0; ii < lscnCount; ii++ )
   {
      double  timecor;   // Time (corrected)
      double  timesqr;   // Square root of corrected time
      double  bdleft;    // Back-diffusion left value
      double  xbdleft;   // Find-root of bdleft
      double  radD;      // Radius of back-diffusion point
      double  omegasq;   // Omega squared

      int js      = liveScans[ ii ];
DbgLv(1) << "  ii nscnu" << ii << nscnu << " js lscnCount" << js << lscnCount;
      dscan       = &edata->scanData[ js ];
      omega       = dscan->rpm * M_PI / 30.0;
      omegasq     = omega * omega;
      timecor     = dscan->seconds - time_correction;
      timesqr     = sqrt( timecor );
      pconc       = baseline + ( scPlats[ ii ] - baseline ) * positPct;

      xx[ ii ]    = 1.0 / timesqr;    // Save X (reciprocal sqrt(time))

      // Accumulate limits based on back diffusion

//left=tolerance*pow(diff,0.5)/(2*intercept[0]*omega_s*
//  (bottom+run_inf.meniscus[selected_cell])/2
//  *pow(run_inf.time[selected_cell][selected_lambda][i],0.5);
//radD=bottom-(2*find_root(left)
//  *pow((diff*run_inf.time[selected_cell][selected_lambda][i]),0.5));

      // left = tolerance * sqrt( diff )
      //        / ( 2 * intercept[0] * omega^2
      //            * ( bottom + meniscus ) / 2 * sqrt( time ) )

      bdleft      = bdtoler * bdifcsqr
         / ( bdiff_sedc * omegasq * ( bottom + edata->meniscus ) * timesqr );
      xbdleft     = find_root( bdleft );

      // radD = bottom - ( 2 * find_root(left) * sqrt( diff * time ) )

      radD        = bottom - ( 2.0 * xbdleft * bdifcsqr * timesqr );
      radD        = max( edata->x[ 0 ].radius, min( bottom, radD ) );

      int mm      = US_DataIO2::index( edata->x, radD );  // Radius's index

      // Accumulate for this scan of this division
      //  the back diffusion limit radius and corresponding concentration
      xr[ nscnu ] = radD;                         // BD Radius
      yr[ nscnu ] = dscan->readings[ mm ].value;  // BD Concentration
DbgLv(1) << "  bottom meniscus bdleft" << bottom << edata->meniscus << bdleft;
DbgLv(1) << "  bdifsedc find_root toler" << bdiff_sedc << xbdleft << bdtoler;
DbgLv(2) << "  bdiff_coef bdifcsqr" << bdiff_coef << bdifcsqr;
DbgLv(1) << "BD x,y " << nscnu+1 << radD << yr[nscnu] << "  mm" << mm;
DbgLv(1) << "  ii nscnu" << ii << nscnu+1 << " bdsed radD yr"
 << bdiff_sedc << radD << yr[nscnu];

      nscnu++;                               // Bump scans-used count
   }
//DbgLv(3) << "  DS:TM:02: " << QTime::currentTime().toString("hh:mm:ss:zzzz");

   dseds.clear();
   dslos.clear();
   dsigs.clear();
   dcors.clear();
   dpnts.clear();

   for ( int jj = 0; jj < divsCount; jj++ )
   {  // Loop to fit points across scans in a division
      double  dsed;
      double  slope;
      double  sigma;
      double  corre;
      int     ii;
      double  oterm;
      double  radC;
      double  radD;
      double  range;
      double  rngFact  = boundPct * divfac;
      double  rngjFact = rngFact  * jj;
      double  conjFact = positPct + rngjFact;
//DbgLv(1) << "div_sed div " << jj+1;

      kscnu      = nscnu;
DbgLv(1) << "FIT jj" << jj << "nscnu" << nscnu;

      // Accumulate y values for this division, across used scans

//DbgLv(3) << "  DS:TM:02: " << QTime::currentTime().toString("hh:mm:ss:zzzz");
      for ( int kk = 0; kk < nscnu; kk++ )
      { // Accumulate concentration, sed.coeff. for all scans, this div
         ii         = liveScans[ kk ];            // Scan index
         dscan      = &edata->scanData[ ii ];     // Scan pointer
DbgLv(1) << "FIT  kk ii" << kk << ii;
//DbgLv(3) << "  DS:TM:03: " << QTime::currentTime().toString("hh:mm:ss:zzzz");

         if ( vhw_enh )
         {
DbgLv(1) << "FIT   CPijs sz" << CPijs.size();
DbgLv(1) << "FIT   CPijs[0] sz" << CPijs[0].size();
DbgLv(1) << "FIT   mconcs sz" << mconcs.size();
DbgLv(1) << "FIT   mconcs[0] sz" << mconcs[0].size();
            cpij       = CPijs [ kk ][ jj ];    // Partial concen (increment)
            mconc      = mconcs[ kk ][ jj ];    // Mid-div concentration
         }
         else
         {
            range      = scPlats[ kk ] - baseline;    // Scan's range
            pconc      = baseline + range * conjFact; // Base conc. of Div
            cpij       = range * rngFact;       // Partial concentration
            mconc      = pconc + cpij * 0.5;    // Mid-div concentration
         }

DbgLv(1) << "FIT    kk jj" << kk << jj << "mconc cpij" << mconc << cpij;
         omega      = dscan->rpm * M_PI / 30.0;   // Omega
         oterm      = ( dscan->seconds - time_correction ) * omega * omega;
//DbgLv(3) << "  DS:TM:04: " << QTime::currentTime().toString("hh:mm:ss:zzzz");

         yy[ kk ]   = sed_coeff( mconc, oterm, &radC, NULL );  // Sed.coeff.
         radD       = xr[ kk ];

         if ( radC > radD )
         {  // Gone beyond back-diffusion cutoff: exit loop with truncated list
            kscnu      = kk;
            yy[ kk ]   = SEDC_NOVAL;
DbgLv(1) << " div kscnu" << jj+1 << kscnu
   << " radC radD" << radC << radD << " mconc sedcc" << mconc << yy[kk];
//if(kscnu==0) DbgLv(2) << "   pc mc cpij mm" << pconc << mconc << cpij << mm;
            break;
         }
//DbgLv(3) << "  DS:TM:06: " << QTime::currentTime().toString("hh:mm:ss:zzzz");
//if ( kk < 2 || kk > (nscnu-3) )
//DbgLv(2) << "div scn " << jj+1 << ii+1 << " pconc mconc " << pconc << mconc;

      }
//DbgLv(2) << " nscnu yy0 yyn " << nscnu << yy[0] << yy[kscnu-1];

      int kscsv = kscnu;
      QVector< double > xtvec;
      QVector< double > ytvec;
      ii         = 0;

      for ( int kk = 0; kk < kscnu; kk++ )
      {  // Remove any leading points below meniscus
         if ( yy[ kk ] != SEDC_NOVAL )
         { // Sed coeff value not from below meniscus
            //if ( kk > ii )
            //{
            //   yy[ ii ] = yy[ kk ];
            //}
DbgLv(1) << "KS    ii kk" << ii << kk << "xx yy" << xx[kk] << yy[kk];
            ii++;
            xtvec << xx[ kk ];
            ytvec << yy[ kk ];
         }
         else
            DbgLv(1) << "KS +++ SED<=0.0 divjj" << jj << "scnkk ii" << kk << ii;
      }
//DbgLv(3) << "  DS:TM:07: " << QTime::currentTime().toString("hh:mm:ss:zzzz");
      kscnu      = ii;
      double* xt = xtvec.data();
      double* yt = ytvec.data();
DbgLv(1) << " KS" << kscnu << kscsv << "size xtv ytv" << xtvec.size()
   << ytvec.size();

      if ( kscnu > 1 )
      {
         // Calculate and save the division sedcoeff and fitted line slope

         US_Math2::linefit( &xt, &yt, &slope, &dsed, &sigma, &corre, kscnu );
DbgLv(1) << " KS" << kscnu << kscsv << "jj" << jj << "xx0 xxn yy0 yyn" << xt[0]
 << xt[kscnu-1] << yt[0] << yt[kscnu-1] << "slo sed" << slope << dsed;
      }

      else
      {
         dsed       = yyv[ 0 ];
         slope      = 0.0;
         sigma      = 0.0;
         corre      = 0.0;
         slope      = 0.0;
ii=(kscnu<1)?1:kscnu;
DbgLv(1) << "    kscnu" << kscnu << "yy0 yyn " << yy[0] << yy[ii-1];
         continue;
      }

      dseds << dsed;    // Save fitted line intercept (Sed.Coeff.)
      dslos << slope;   // Save slope and other fitting values
      dsigs << sigma;
      dcors << corre;
      dpnts << kscnu;
DbgLv(1) << "JJ" << jj << "DSED" << dsed << "dseds size" << dseds.size();
//DbgLv(3) << "  DS:TM:08: " << QTime::currentTime().toString("hh:mm:ss:zzzz");

   }
//DbgLv(3) << "  DS:TM:77: " << QTime::currentTime().toString("hh:mm:ss:zzzz");
   int kdivs = dseds.size();
int k3=qMax(0,kdivs-3);
int k2=qMax(0,kdivs-2);
int k1=qMax(0,kdivs-1);
DbgLv(1) << " dsed[0]  " << dseds[0] << "kdivs" << kdivs;
DbgLv(1) << " dsed[k3] " << dseds[k3];
DbgLv(1) << " dsed[k2] " << dseds[k2];
DbgLv(1) << " dsed[L]  " << dseds[k1];
DbgLv(1) << " D_S: xr0 yr0 " << xr[0] << yr[0];
DbgLv(1) << " D_S: xrN yrN " << xr[nscnu-1] << yr[nscnu-1] << nscnu;

   bdrads.clear();
   bdcons.clear();

   for ( int kk = 0; kk < nscnu; kk++ )
   {  // Save the Radius,Concentration points for back-diffusion cutoff curve
      bdrads << xr[ kk ];
      bdcons << yr[ kk ];
   }
//DbgLv(3) << "  DS:TM:88: " << QTime::currentTime().toString("hh:mm:ss:zzzz");

   return;
}

// Find root X where evaluated Y is virtually equal to a goal, using a
//  calculation including the inverse complementary error function (erfc).
double US_vHW_Enhanced::find_root( double goal )
{
#ifdef WIN32
#define erfc(x) US_Math2::erfc(x)
#endif

#define _FR_MXKNT 100            // Max find-root iteration count
   double  tolerance = 1.0e-7;   // Min difference tolerance
   double  x1        = 0.0;
   double  x2        = 10.0;
   double  xv        = 5.0;
   double  xdiff     = 2.5;
   double  xsqr      = xv * xv;
   double  rsqr_pi   = 1.0 / sqrt( M_PI );
   double  test      = exp( -xsqr ) * rsqr_pi - ( xv * erfc( xv ) );
           test      = ( goal != 0.0 ) ? test : 0.0;
DbgLv(2) << "      find_root: goal test" << goal << test << " xv" << xv;
//DbgLv(2) << "        erfc(x)" << erfc(xv);
   int     count     = 0;

   // Iterate until the difference between subsequent x value evaluations
   //  is too small to be relevant (or max count reached);

   while ( fabs( test - goal ) > tolerance )
   {
      xdiff  = ( x2 - x1 ) / 2.0;

      if ( test < goal )
      { // At less than goal, adjust top (x2) limit
         x2     = xv;
         xv    -= xdiff;
      }

      else
      { // At greater than goal, adjust bottom (x1) limit
         x1     = xv;
         xv    += xdiff;
      }

      // Then update the test y-value
      xsqr   = xv * xv;
      test   = ( 1.0 + 2.0 * xsqr ) * erfc( xv ) 
         - ( 2.0 * xv * exp( -xsqr ) ) * rsqr_pi;
//DbgLv(2) << "      find_root:  goal test" << goal << test << " x" << xv;

      if ( (++count) > _FR_MXKNT )
         break;
   }
DbgLv(2) << "      find_root:  goal test" << goal << test
   << " xv" << xv << "  count" << count;

   return xv;
}

// Calculate back diffusion coefficient
double US_vHW_Enhanced::back_diff_coeff( double sedc )
{
   double  tempera  = le_temp->text().section( " ", 0, 0 ).toDouble();
   double  RT       = R * ( K0 + tempera );
   double  D1       = AVOGADRO * 0.06 * M_PI * viscosity;
   double  D2       = 0.045 * sedc * vbar * viscosity;
   double  D3       = 1.0 - vbar * density;

   double  bdcoef   = RT / ( D1 * sqrt( D2 / D3 ) );

DbgLv(1) << "BackDiffusion:";
DbgLv(1) << " RT " << RT << " R K0 tempera  " << R << K0 << tempera;
DbgLv(1) << " D1 " << D1 << " viscosity AVO " << viscosity << AVOGADRO; 
DbgLv(1) << " D2 " << D2 << " sedc vbar     " << sedc << vbar;
DbgLv(1) << " D3 " << D3 << " density       " << density;
DbgLv(1) << "  bdiff_coef" << bdcoef << " = RT/(D1*sqrt(D2/D3))";
   return bdcoef;
}

// Handle mouse clicks for selecting vH-W groups
void US_vHW_Enhanced::groupClick( const QwtDoublePoint& p )
{
   QwtPlotMarker* marker;
   QwtText        label;
   GrpInfo        cgrdata;
   QString        gbanner;
   int            ngroup;
DbgLv(1) << "groupClick: step" << groupstep
   << "x y" << p.x() << p.y();

   switch( groupstep )
   {
      case NONE :
         break;

      case START :
         groupstep = END;
         groupxy << p.x() << p.y();      // add start x,y to list
         break;

      case END :
         groupstep = START;
         groupxy << p.x() << p.y();      // add end x,y to list
         ngroup    = groupxy.size() / 4;

         add_group_info( );

         marker    = new QwtPlotMarker;
         cgrdata   = groupdat.at( ngroup-1 );

         gbanner   = tr( "Group %1: %2 (%3%)" )
               .arg( ngroup ).arg( cgrdata.sed ).arg( cgrdata.percent );
         label.setText( gbanner );
         label.setFont( QFont( US_GuiSettings::fontFamily(),
                  -1, QFont::Bold ) );
         label.setColor( Qt::magenta );
         label.setBackgroundBrush( QBrush( QColor( 8, 8, 8, 128 ) ) );

         marker->setValue( 0.0, cgrdata.sed );
         marker->setLabel( label );
         marker->setLabelAlignment( Qt::AlignRight | Qt::AlignVCenter );
         marker->attach( data_plot1 );

         data_plot1->replot();
         break;

      default :
         break;
   }
DbgLv(1) << "groupClick:  nxy val" << groupxy.size();
}

// add to selected group information
void US_vHW_Enhanced::add_group_info( )
{
   int     ngroup  = groupxy.size() / 4;
   int     jg      = ( ngroup - 1 ) * 4;
   GrpInfo grdat;

   grdat.x1        = groupxy.at( jg     ); // save pick coordinates
   grdat.y1        = groupxy.at( jg + 1 );
   grdat.x2        = groupxy.at( jg + 2 );
   grdat.y2        = groupxy.at( jg + 3 );
   grdat.sed       = 0.0;                  // initialize other variables
   grdat.percent   = 0.0;
   grdat.ndivs     = 0;
   grdat.idivs.clear();
   int divsUsed    = dseds.size();

   for ( int jj = 0; jj < divsUsed; jj++ )
   {  // walk thru all division lines to see if within clicked range
      double sed      = dseds[ jj ];            // intercept sed coeff
      double slope    = dslos[ jj ];            // div line slope
      double yh       = sed + slope * grdat.x1; // line intercept w high x
      double yl       = sed + slope * grdat.x2; // line intercept w low x

      if ( yh <= grdat.y1  &&  yl >= grdat.y2 )
      {  // line is in group:  add division to group
         grdat.idivs.append( jj );  // add to div index list
         grdat.sed      += sed;     // accumulate sedcoeff sum for average
         grdat.ndivs++;             // bump included divs count
      }
   }

   // finish average-sed-coeff and percent calculations; add to groups list
   grdat.ndivs     = grdat.ndivs > 0 ? grdat.ndivs : 1;
   grdat.sed      /= (double)grdat.ndivs;
   grdat.percent   = ( (double)grdat.ndivs / (double)divsCount ) * 100.0;

   groupdat.append( grdat );
}

// write a file of vHW extrapolation data
void US_vHW_Enhanced::write_vhw()
{
   QString dirres   = US_Settings::resultDir();
   QString dirrep   = US_Settings::reportDir();
   QString dirname  = dirres + "/" + edata->runID;
   QString dirrept  = dirrep + "/" + edata->runID;

   QDir dirr( dirres );
   QDir dirp( dirrep );

   if ( ! dirr.exists( dirname ) )
      dirr.mkpath( dirname );

   if ( ! dirp.exists( dirrept ) )
      dirp.mkpath( dirrept );

   QString filename = dirname + "/vHW."
      + QString( triples.at( row ) ).replace( " / ", "" ) + ".extrap.csv";

   QFile   res_f( filename );
   double  sedc;
   int     lastDiv   = divsCount - 1;
   int     kk        = 0;
   const QString fsep( "," );
   const QString eoln( "\n" );
   const QString blnk( "\"\"" );
   //QString control   = "\t";
   QString control   = fsep;

   if ( !res_f.open( QIODevice::WriteOnly | QIODevice::Text ) )
   {
      return;
   }
DbgLv(1) << "WV: filename " << filename;

   QTextStream ts( &res_f );

   // Output header line
   ts << "\"1/sqrt(t)\"" << fsep;

   for ( int jj = 0; jj < divsCount; jj++ )
   {
      QString line = QString().sprintf( "\"D%03dSedCoef\"", jj + 1 );
      ts << line << ( jj < lastDiv ? fsep : eoln );
   }

   // Output data
   for ( int ii = 0; ii < lscnCount; ii++ )
   {
      // Each output line begins with reciprocal square root of scan time
      int js       = liveScans[ ii ];
      control      = fsep;
      QString dat  = QString().sprintf( "\"%11.8f\"",
         ( 1.0 / sqrt( edata->scanData[ js ].seconds - time_correction ) ) );
      dat.replace( " ", "" );
      ts << dat << control;

      // Balance of line is a list of sedimentation coefficient values for
      //  the divisions in the scan 
      for ( int jj = 0; jj < divsCount; jj++ )
      {
         sedc         = aseds[ kk++ ];
         if ( sedc > 0 )
         {
            dat          = QString().sprintf( "\"%8.5f\"", sedc );
            dat.replace( " ", "" );
         }
         else
            dat          = blnk;

         ts << dat << ( jj < lastDiv ? control : eoln );
      }
   }

   res_f.close();
}

// write a file of vHW division distribution values
void US_vHW_Enhanced::write_dis()
{
   QString filename = US_Settings::resultDir() + "/" + edata->runID
      + "/vHW." + QString( triples.at( row ) ).replace( " / ", "" )
      + ".s-c-distrib.csv";
   QFile   res_f( filename );
   double  bterm    = 100.0 * boundPct / (double)divsCount;
   double  pterm    = 100.0 * positPct + bterm;
   double  bfrac;
   QString dline;

   if ( !res_f.open( QIODevice::WriteOnly | QIODevice::Text ) )
   {
      return;
   }
DbgLv(1) << "WD: filename " << filename;

   // write the line-fit variables for each division
   QTextStream ts( &res_f );
   ts << edata->description << "\n";
   ts << tr( "\"%Boundary\",\"Points\",\"Slope\",\"Intercept\","
             "\"Sigma\",\"Correlation\"\n" );

   for ( int jj = 0; jj < dseds.size(); jj++ )
   {
      bfrac     = pterm + bterm * (double)( jj );

      dline.sprintf(
         "\"%9.2f\",\"%7d\",\"%12.6f\",\"%12.6f\",\"%12.6f\",\"%12.6f\"\n",
         bfrac, dpnts[ jj ], dslos[ jj ], dseds[ jj ],
         dsigs[ jj ], dcors[ jj ] );
      dline.replace( " ", "" );
      ts << dline;
   }

   res_f.close();
}

// write a file of vHW detailed division group model data
void US_vHW_Enhanced::write_model()
{
   QString filename = US_Settings::resultDir() + "/" + edata->runID
      + "/vHW." + QString( triples.at( row ) ).replace( " / ", "" )
      + ".fef_model.rpt";
   QFile   res_f( filename );
   int     groups   = groupdat.size();

   if ( !res_f.open( QIODevice::WriteOnly | QIODevice::Text ) )
   {
      return;
   }
DbgLv(1) << "WM: filename " << filename;

   QTextStream ts( &res_f );

   ts <<     "*************************************\n";
   ts << tr( "*   Please do not edit this file!   *\n" );
   ts <<     "*************************************\n\n\n";
   ts << "3            " << tr( "\t# Fixed Molecular Weight Distribution\n" );
   ts << groups << "           " << tr( "\t# Number of Components\n" );
   ts << edata->meniscus << "       " << tr( "\t# Meniscus in cm\n" );
   ts << "0.01         " << tr( "\t# Meniscus range in cm\n" );
   ts << "0            " << tr( "\t# Meniscus fitting control\n" );
   ts << baseline << "     " << tr( "\t# Baseline in OD\n" );
   ts << "0.01         " << tr( "\t# Baseline range in OD\n" );
   ts << "1            " << tr( "\t# Baseline fitting control\n" );
   ts << "0.0000       " << tr( "\t# Slope(r) Correction in OD\n" );
   ts << "0.0000       " << tr( "\t# Slope(r) Correction range in OD\n" );
   ts << "0            " << tr( "\t# Slope(r) Correction fitting control\n" );
   ts << "0.0000       " << tr( "\t# Slope(c,r) Correction in OD\n" );
   ts << "0.0000       " << tr( "\t# Slope(c,r) Correction range in OD\n" );
   ts << "0            " << tr( "\t# Slope(c,r) Correction fitting control\n");
   ts << "20           " << tr( "\t# Delta_t in seconds\n" );
   ts << "0.001        " << tr( "\t# Delta_t in cm\n" );

   QString line04 = "1            \t" +
      tr( "# Sedimentation Coefficient fitting control\n" );
   QString line05 = "7.00e-07     \t" +
      tr( "# Diffusion Coefficient in D\n" );
   QString line06 = "7.00e-08     \t" +
      tr( "# Diffusion Coefficient range in D\n" );
   QString line07 = "1            \t" +
      tr( "# Diffusion Coefficient fitting control\n" );
   QString line10 = "0            \t" +
      tr( "# Partial Concentration fitting control\n" );
   QString line11 = "0.0000       \t" + tr( "# Sigma\n" );
   QString line12 = "0.0000       \t" + tr( "# Sigma range\n" );
   QString line13 = "0            \t" + tr( "# Sigma fitting control\n" );
   QString line14 = "0.0000       \t" + tr( "# Delta\n" );
   QString line15 = "0.0000       \t" + tr( "# Delta range\n" );
   QString line16 = "0            \t" + tr( "# Delta fitting control\n" );
   QString line17 = "1            \t" +
      tr( "# Molecular Weight fitting control\n" );
   QString line18 = "0            \t" +
      tr( "# Part. Spec. Volume fitting control\n" );
   double  cterm  = ( C0 - baseline ) / 100.0;

   for ( int ii = 0; ii < groups; ii++ )
   {
      double gsed = groupdat.at( ii ).sed * 1.0e-13;
      double gcon = groupdat.at( ii ).percent * cterm;
      ts << "\n";
      ts << tr( "Parameters for Component " ) << ( ii + 1 ) << ":\n\n";
      ts << gsed << "  "
         << tr( "\t# Sedimentation Coefficient in s\n" );
      ts << gsed / 10.0 << "  "
         << tr( "\t# Sedimentation Coefficient range in s\n" );
      ts << line04 << line05 << line06 << line07;
      ts << gcon << "    "
         << tr( "\t# Partial Concentration in OD\n" );
      ts << gcon / 10.0 << "   "
         << tr( "\t# Partial Concentration range in OD\n" );
      ts << line10 << line11 << line12 << line13;
      ts << line14 << line15 << line16 << line17 << line18;
   }

   res_f.close();
}

// text of minutes,seconds or hours,minutes for a given total seconds value
QString US_vHW_Enhanced::text_time( double seconds, int type )
{
   int mins = (int)( seconds / 60.0 );
   int secs = (int)( seconds - (double)mins * 60.0 );

   if ( type == 0 )
   {  // fixed-field mins,secs text
      QString tmin = QString().sprintf( "%4d", mins );
      QString tsec = QString().sprintf( "%3d", secs );
      return  tr( "%1 min %2 sec" ).arg( tmin ).arg( tsec );
   }

   else if ( type == 1 )
   {  // minutes,seconds text
      return  tr( "%1 minutes(s) %2 second(s)" ).arg( mins ).arg( secs );
   }

   else
   {  // hours,minutes text
      int hrs  = (int)( seconds / 3600.0 );
      mins     = qRound( ( seconds - (double)hrs * 3600.0 ) / 60.0 );
      return  tr( "%1 hour(s) %2 minute(s)" ).arg( hrs ).arg( mins );
   }
}

// text of minutes and seconds for a given total seconds value
QString US_vHW_Enhanced::text_time( double seconds )
{  // default mins,secs text
   return  text_time( seconds, 0 );
}


// pare down files list by including only the last-edit versions
QStringList US_vHW_Enhanced::last_edit_files( QStringList files )
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

void US_vHW_Enhanced::new_triple( int row )
{
   US_AnalysisBase2::new_triple( row );
   haveZone   = false;
}

void US_vHW_Enhanced::update( int row )
{
   haveZone   = false;
   edata      = &dataList[ row ];

   // Do some calculations handled in AnalysisBase, but needed here
   //  so that model plateau calculations work
   time_correction    = US_Math2::time_correction( dataList );
   solution.density   = le_density  ->text().toDouble();
   solution.viscosity = le_viscosity->text().toDouble();
   solution.vbar20    = le_vbar     ->text().toDouble();
   double avgTemp     = edata->average_temperature();
   solution.vbar      = US_Math2::calcCommonVbar( solution_rec, avgTemp );
   US_Math2::data_correction( avgTemp, solution );

   // Do normal analysis triple updating, but suppress plotting for now
   skipPlot   = true;
   US_AnalysisBase2::update( row );
   skipPlot   = false;
DbgLv(1) << " update: vbar" << vbar;

   if ( vbar <= 0.0 )
   {
      QMessageBox::warning( this, tr( "Bad Solution Values" ),
         tr( "The Vbar for this data is not a positive value (vbar=%1).\n"
             "The current data set needs editing of its Solution before\n"
             "van Holde - Weischet analysis can proceed." ).arg( vbar ) );
      return;
   }

   // After triple update has completed, we can proceed with plotting
   forcePlot  = true;
   data_plot();
   forcePlot  = false;

   // Report on whether a model is available
   QString tripl = QString( triples.at( row ) ).replace( " / ", "" );
   QString dmsg  = te_desc->toPlainText() + "\n";

   if ( have_model() )
      dmsg += tr( "A model IS implied for %1" ).arg( tripl );
   else
      dmsg += tr( "NO model is implied for %1" ).arg( tripl );

   te_desc->setText( dmsg );
}

// Calculate the sedimentation coefficient intercept for division 1
double US_vHW_Enhanced::sedcoeff_intercept()
{
   QVector< double > xrvec( lscnCount );
   QVector< double > yrvec( lscnCount );
   double* xr      = xrvec.data();
   double* yr      = yrvec.data();
   double  sedc    = 0.0;
   int     nscnu   = 0;

   for ( int ii = 0; ii < lscnCount; ii++ )
   {  // Accumulate x,y values:  1/sqrt(time), sed_coeff
      int js          = liveScans[ ii ];
      dscan           = &edata->scanData[ js ];

      double range    = scPlats[ ii ] - baseline;
      double basecut  = baseline + range * positPct;
      double platcut  = basecut  + range * boundPct;
      double cinc     = ( platcut - basecut ) * divfac;
      double mconc    = basecut + cinc * 0.5;
      double omega    = dscan->rpm * M_PI / 30.0;
      double timecor  = dscan->seconds - time_correction;
      double oterm    = timecor * omega * omega;

      // Get sedimentation coefficient for concentration
      sedc            = sed_coeff( mconc, oterm );

      if ( sedc > 0.0 )
      {
         xr[ nscnu ] = 1.0 / sqrt( timecor );  // X = inverse sqrt of time
         yr[ nscnu ] = sedc;                   // Y = sedimentation coeff.
         nscnu++;                              // bump count of used scans
      }
DbgLv(2) << " s-i: range baseline basecut platcut mconc" << range << baseline
  << basecut << platcut << mconc;
   }

   double slope;
   double sigma = 0.0;
   double corre;

   // Fit a line. Use the intercept as the sedimentation coefficient
   US_Math2::linefit( &xr, &yr, &slope, &sedc, &sigma, &corre, nscnu );

   if ( sedc <= 0.0 )
   {  // If it dropped below zero, back up line until it crosses to positive
DbgLv(1) << "sedco-intcp **SEDC**" << sedc << "x0 y0 xn yn n"
 << xr[0] << yr[0] << xr[nscnu-1] << yr[nscnu-1] << nscnu; 
DbgLv(1) << "sedco-intcp   slope sigma" << slope << sigma;
      while ( (--nscnu) > 0  &&  sedc <= 0.0 )
      {
         sedc = yr[ nscnu ];
      }
   }

DbgLv(1) << "sedco-intcp   ++SEDC++" << sedc << nscnu;
   sedc  *= 1.0e-13;

   return sedc;
}

// Copy temporary files saved by plot dialog to report/results
void US_vHW_Enhanced::copy_data_files( QString plot1File,
      QString plot2File, QString data2File )
{
   QString tempbase   = US_Settings::tmpDir() + "/vHW.temp.";
   QString tplot1File = tempbase + "s-c-distrib.svg";
   QString tplot2File = tempbase + "s-c-histo.svg";
   QString tdata2File = tempbase + "s-c-envelope.csv";
   QString tplot3File = tempbase + "s-c-distrib.png";
   QString tplot4File = tempbase + "s-c-histo.png";
   QString plot3File  = QString( plot1File ).replace( ".svg", ".png" );
   QString plot4File  = QString( plot2File ).replace( ".svg", ".png" );

   QFile tfp1( tplot1File );
   QFile tfp2( tplot2File );
   QFile tfd2( tdata2File );
   QFile tfp3( tplot3File );
   QFile tfp4( tplot4File );

   if ( tfp1.exists() )
   {
      if ( QFile( plot1File ).remove() )
         DbgLv(1) << "CDF: removed:" << plot1File;
      if ( tfp1.copy( plot1File ) )
         DbgLv(1) << "CDF: copied:" << tplot1File;
   }

   if ( tfp2.exists() )
   {
      if ( QFile( plot2File ).remove() )
         DbgLv(1) << "CDF: removed:" << plot2File;
      if ( tfp2.copy( plot2File ) )
         DbgLv(1) << "CDF: copied:" << tplot2File;
   }

   if ( tfd2.exists() )
   {
      if ( QFile( data2File ).remove() )
         DbgLv(1) << "CDF: removed:" << data2File;
      if ( tfd2.copy( data2File ) )
         DbgLv(1) << "CDF: copied:" << tdata2File;
   }

   if ( tfp3.exists() )
   {
      if ( QFile( plot3File ).remove() )
         DbgLv(1) << "CDF: removed:" << plot3File;
      if ( tfp3.copy( plot3File ) )
         DbgLv(1) << "CDF: copied:" << tplot3File;
   }

   if ( tfp4.exists() )
   {
      if ( QFile( plot4File ).remove() )
         DbgLv(1) << "CDF: removed:" << plot4File;
      if ( tfp4.copy( plot4File ) )
         DbgLv(1) << "CDF: copied:" << tplot4File;
   }
}

// Calculate scan plateau concentrations by fitting initial values 
bool US_vHW_Enhanced::fitted_plateaus()
{
   int     totalCount;
DbgLv(1) << "FITTED_PLATEAUS";
   if ( !dataLoaded  ||  vbar <= 0.0 )
      return false;

   valueCount = edata->x.size();
   scanCount  = edata->scanData.size();
   divsCount  = qRound( ct_division->value() );
   totalCount = scanCount * divsCount;
   divfac     = 1.0 / (double)divsCount;
   boundPct   = ct_boundaryPercent->value() / 100.0;
   positPct   = ct_boundaryPos    ->value() / 100.0;
   baseline   = calc_baseline();
   correc     = solution.s20w_correction * 1.0e13;
	C0         = 0.0;
	Swavg      = 0.0;
   omega      = edata->scanData[ 0 ].rpm * M_PI / 30.0;

   // Do first experimental plateau calcs based on horizontal zones

   int     nrelp = 0;
   QVector< double > pxvec( lscnCount  );
   QVector< double > pyvec( totalCount );
   QVector< double > sxvec( lscnCount  );
   QVector< double > syvec( lscnCount  );
   QVector< double > fyvec( lscnCount  );
   double* ptx   = pxvec.data();   // t,log(p) points
   double* pty   = pyvec.data();
   double* csx   = sxvec.data();   // count,slope points
   double* csy   = syvec.data();
   double* fsy   = fyvec.data();   // fitted slope
   double plateau;

   QList< double > plats;
   QList< int >    isrel;
   QList< int >    isunr;

   // accumulate reliable,unreliable scans and plateaus

   scPlats.fill( 0.0, lscnCount );
   nrelp    = 0;

DbgLv(1) << " Initial reliable (non-excluded) scan t,log(avg-plat):";
   // Initially reliable plateaus are all average non-excluded
   for ( int ii = 0; ii < lscnCount; ii++ )
   {
      int js        = liveScans[ ii ];
      dscan         = &edata->scanData[ js ];
      plateau       = avg_plateau();
      scPlats[ ii ] = plateau;
      ptx[ nrelp ]  = dscan->seconds - time_correction;
      pty[ nrelp ]  = log( plateau );

      isrel.append( ii );
DbgLv(1) << ptx[nrelp] << pty[nrelp];
      nrelp++;
   }
//DbgLv(3) << "DP:TM:01: " << QTime::currentTime().toString("hh:mm:ss:zzzz");

   double  slope;
   double  intcp;
   double  sigma;
   double  corre;
   int    krelp  = nrelp;
   int    jrelp  = nrelp;
   int    kf     = 0;

   // Accumulate count,slope points for line fits to the t,log(p) points

   for ( int jj = 6; jj < nrelp; jj++ )
   {
      US_Math2::linefit( &ptx, &pty, &slope, &intcp, &sigma, &corre, jj );

      csx[ kf ]   = (double)jj;
      csy[ kf++ ] = slope;
DbgLv(1) << "k,slope point: " << kf << jj << slope;
   }

   // Create a 3rd-order polynomial fit to the count,slope curve

   double coefs[ 4 ];

   US_Matrix::lsfit( coefs, csx, csy, kf, 4 );
DbgLv(1) << "3rd-ord poly fit coefs:" << coefs[0] << coefs[1] << coefs[2]
 << coefs[3];

   for ( int jj = 0; jj < kf; jj++ )
   {
      double xx = csx[ jj ];
      double yy = coefs[ 0 ] + coefs[ 1 ] * xx + coefs[ 2 ] * xx * xx
                + coefs[ 3 ] * xx * xx * xx;
      fsy[ jj ] = yy;
DbgLv(1) << " k,fit-slope point: " << jj << xx << yy << " raw slope" << csy[jj];
   }

   // Now find the spot where the derivative of the fit crosses zero
   double prevy = fsy[ 1 ];
   double maxd  = -1e+30;
   double dfac  = 1.0 / qAbs( fsy[ kf - 1 ] - prevy );  // norm. deriv. factor
   double prevd = ( prevy - fsy[ 0 ] ) * dfac;  // normalized derivative
   jrelp        = -1;

//DbgLv(3) << "DP:TM:02: " << QTime::currentTime().toString("hh:mm:ss:zzzz");
DbgLv(1) << "Fitted slopes derivative points:";
   for ( int jj = 2; jj < kf; jj++ )
   {
      double currx = csx[ jj ];
      double curry = fsy[ jj ];
      double currd = ( curry - prevy ) * dfac;  // normalized derivative
//DbgLv(1) << "  k cx cy cd pd" << jj << currx << curry << currd << prevd;
DbgLv(1) << "  k" << jj << " x fslo" << currx << curry << " deriv" << currd;

      if (   currd == 0.0  ||
           ( currd > 0.0  &&  prevd < 0.0 )  ||
           ( currd < 0.0  &&  prevd > 0.0 ) )
      {  // Zero point or zero crossing
         jrelp    = jj - 1;
DbgLv(1) << "    Z-CROSS";
if (dbg_level>=1 ) { for ( int mm = jj + 1; mm < kf; mm++ ) {
 prevy = curry; prevd = currd;
 currx = csx[mm]; curry = fsy[mm];
 currd = ( curry - prevy ) * dfac;
 DbgLv(1) << "  k cx cy cd pd" << mm << currx << curry << currd << prevd;
}}
         break;
      }

      if ( currd > maxd )
      {  // Maximum derivative value
         maxd     = currd;
         jrelp    = jj;
DbgLv(1) << "    MAXD" << maxd;
      }
      prevy      = curry;
      prevd      = currd;
   }

   // Do final fit to the determined number of leading points
   krelp         = (int)csx[ jrelp ];

   US_Math2::linefit( &ptx, &pty, &slope, &intcp, &sigma, &corre, krelp );
DbgLv(1) << " KRELP" << krelp << "   slope intcp sigma"
 << slope << intcp << sigma;
//DbgLv(3) << "DP:TM:03: " << QTime::currentTime().toString("hh:mm:ss:zzzz");

   Swavg      = slope / ( -2.0 * omega * omega );  // Swavg func of slope
	C0         = exp( intcp );                      // C0 func of intercept
DbgLv(1) << "Swavg(c): " << Swavg*correc << " C0: " << C0 ;

   // Determine Cp for all the scans based on fitted line:
   //   y = ax + b, using "a" and "b" determined above.
   // Since y = log( Cp ), we get Cp by exponentiating
   //   the left-hand term.

   for ( int ii = 0; ii < lscnCount; ii++ )
   {  // each extrapolated plateau is exp of Y for X of corrected time
      int js         = liveScans[ ii ];
      dscan          = &edata->scanData[ js ];
      double tc      = dscan->seconds - time_correction;
      dscan->plateau = exp( tc * slope + intcp );
      scPlats[ ii ]  = dscan->plateau;
DbgLv(1) << " jj scan plateau " << ii << ii+1 << scPlats[ii];
   }
   return true;
}

// Calculate scan plateau concentrations from a 2DSA model
bool US_vHW_Enhanced::model_plateaus()
{
   int n_ti_noi = ti_noise.values.size();
   int n_ri_noi = ri_noise.values.size();
   QString modelGUID;
   US_Model model;

   if ( ! ck_modelpl->isChecked() )
      return false;

   if ( n_ti_noi > 0 )
      modelGUID = ti_noise.modelGUID;
   else if ( n_ri_noi > 0 )
      modelGUID = ri_noise.modelGUID;
   else
      return false;

DbgLv(1) << "Cpl: MODEL_PLATEAUS: modelGUID" << modelGUID;
   if ( ! modelGUID.isEmpty()  &&  modelGUID.length() == 36 )
   {
      bool    isDB = disk_controls->db();
      US_Passwd pw;
      US_DB2* db   = isDB ? new US_DB2( pw.getPasswd() ) : 0;

      model.load( isDB, modelGUID, db );
   }
else DbgLv(1) << "Cpl:    MODEL empty   GUIDlen" << modelGUID.length();
   int ncomp    = model.components.size();
DbgLv(1) << "CPl:    ncomp" << ncomp;
   double scorr = -2.0 / solution.s20w_correction;
DbgLv(1) << "Cpl:    scorr" << scorr << solution.s20w_correction;
   edata        = &dataList[ row ];
   valueCount   = edata->x.size();
   scanCount    = edata->scanData.size();
   scPlats.fill( 0.0, lscnCount );
	C0           = 0.0;
	Swavg        = 0.0;

   for ( int ii = 0; ii < lscnCount; ii++ )
   {
      int js            = liveScans[ ii ];
      dscan             = &edata->scanData[ js ];
      double   omega    = dscan->rpm * M_PI / 30.0;
      double   oterm    = ( dscan->seconds - time_correction ) * omega * omega;
               oterm   *= scorr;
      double   cplat    = 0.0;

      for ( int jj = 0; jj < ncomp; jj++ )
      {
         US_Model::SimulationComponent* sc = &model.components[ jj ];
         double conc    = sc->signal_concentration;
         double sval    = sc->s;
         cplat         += ( conc * exp( oterm * sval ) );
      }
DbgLv(1) << "Cpl:  scan" << ii << "cplat" << cplat;

      scPlats[ ii ]  = cplat;
      dscan->plateau = cplat;
      C0            += cplat;
   }

	C0           = ( lscnCount < 1 ) ? 0.0 : ( C0 / (double)lscnCount );
   return true;
}

// Do a manually forced re-plot of the data
void US_vHW_Enhanced::plot_refresh()
{
   forcePlot    = true;
   data_plot();
   forcePlot    = false;
}

// Calculate X,Y extrapolation points using standard method
void US_vHW_Enhanced::vhw_calcs_standard( double* ptx, double* pty )
{
   int     totalCount;
   double  cconc;
   double  pconc;
   double  mconc;
   double  cinc;
   double  cinch;
   double  oterm;
   divsCount  = qRound( ct_division->value() );
   totalCount = scanCount * divsCount;
   divfac     = 1.0 / (double)divsCount;
	C0         = 0.0;
	Swavg      = 0.0;

   div_seds();

   for ( int ii = 0; ii < totalCount; ii++ )
      pty[ ii ]   = SEDC_NOVAL;

   int     kk     = 0;                    // Index to sed. coeff. values
   int     kl     = 0;                    // Index/count of live scans
   valueCount     = edata->x.size();

   // Calculate the corrected sedimentation coefficients

//DbgLv(3) << "DP:TM:11: " << QTime::currentTime().toString("hh:mm:ss:zzzz");
   for ( int ii = 0; ii < lscnCount; ii++ )
   {
      range          = scPlats[ ii ] - baseline;
      cinc           = range * boundPct * divfac;
      int js         = liveScans[ ii ];
      dscan          = &edata->scanData[ js ];
      double  timev  = dscan->seconds - time_correction;
      double  timex  = 1.0 / sqrt( timev );
      double  bdrad  = bdrads.at( kl );   // Back-diffus cutoff radius for scan
      double  bdcon  = bdcons.at( kl++ ); // Back-diffus cutoff concentration
      double  divrad = 0.0;               // Division radius value
DbgLv(1) << "scn liv" << ii+1 << kl
   << " radius concen time" << bdrad << bdcon << timev;

      ptx[ ii ]  = timex;                 // Save corrected time and accum max

      range      = scPlats[ ii ] - baseline;
      cconc      = baseline + range * positPct; // Initial conc for span
      cinc       = range * boundPct * divfac;
      cinch      = cinc * 0.5;
      omega      = dscan->rpm * M_PI / 30.0;
      oterm      = ( timev > 0.0 ) ? ( timev * omega * omega ) : -1.0;

      for ( int jj = 0; jj < divsCount; jj++ )
      {  // walk through division points; get sed. coeff. by place in readings
         pconc       = cconc;               // Div base
         cconc       = pconc + cinc;        // Absolute concentration
         mconc       = pconc + cinch;       // Mid div concentration

         sedc        = sed_coeff( mconc, oterm, &divrad, NULL );

         if ( divrad > bdrad )
         {  // Mark a point to be excluded by back-diffusion
            sedc        = SEDC_NOVAL;
DbgLv(1) << "CPm: *excl* div" << jj+1 << " drad dcon " << divrad << mconc;
         }
DbgLv(1) << "CPm: ii jj" << ii << jj << "kk iSedc" << kk << sedc
   << "mconc divrad bdrad" << mconc << divrad << bdrad;

         // Y value of point is sedcoeff; accumulate y max
         pty[ kk++ ] = sedc;
      }
   }
//DbgLv(3) << "DP:TM:12: " << QTime::currentTime().toString("hh:mm:ss:zzzz");
}

// Calculate X,Y extrapolation points using enhanced method
void US_vHW_Enhanced::vhw_calcs_enhanced( double* ptx, double* pty )
{
   int     count       = 0;
   int     totalCount;
   double  mconc;
   double  eterm;
   double  oterm;

   scanCount  = edata->scanData.size();
   valueCount = edata->x.size();
   boundPct   = ct_boundaryPercent->value() / 100.0;
   positPct   = ct_boundaryPos    ->value() / 100.0;
   baseline   = calc_baseline();

   divsCount  = qRound( ct_division->value() );
   totalCount = scanCount * divsCount;
   divfac     = 1.0 / (double)divsCount;
   correc     = solution.s20w_correction * 1.0e13;
   omega      = edata->scanData[ 0 ].rpm * M_PI / 30.0;

//DbgLv(3) << "DP:TM:04: " << QTime::currentTime().toString("hh:mm:ss:zzzz");

   // Iterate to adjust plateaus until none needed or max iters reached

   int     iter      = 1;
   int     mxiter    = 3;          // maximum iterations
   double  avdthr    = 2.0e-5;     // threshold cp-absavg-diff

//DbgLv(3) << "DP:TM:07: " << QTime::currentTime().toString("hh:mm:ss:zzzz");
   while( iter <= mxiter )
   {
      double avgdif  = 0.0;
      count          = 0;
DbgLv(1) << "iter mxiter " << iter << mxiter;

      // Get division sedimentation coefficient values (intercepts)

//DbgLv(3) << "DP:TM:08: " << QTime::currentTime().toString("hh:mm:ss:zzzz");
      div_seds();

//DbgLv(3) << "DP:TM:09: " << QTime::currentTime().toString("hh:mm:ss:zzzz");
      int divsUsed   = dseds.size();

      // Reset division plateaus

      for ( int ii = 0; ii < lscnCount; ii++ )
      {
         int js   = liveScans[ ii ];
         dscan    = &edata->scanData[ js ];
         omega    = dscan->rpm * M_PI / 30.0;
         oterm    = ( dscan->seconds - time_correction ) * omega * omega;
         eterm    = -2.0 * oterm / correc;
         c0term   = ( C0 - baseline ) * boundPct * divfac;
         span     = ( scPlats[ ii ] - baseline ) * boundPct;
         sumcpij  = 0.0;

         // Split the difference between divisions

         for ( int jj = 0; jj < divsCount; jj++ )
         {  // Recalculate partial concentrations based on sedcoeff intercepts
            sedc     = ( jj < divsUsed ) ? dseds[ jj ] : SEDC_NOVAL;
            cpij     = ( sedc != SEDC_NOVAL ) ?
                       ( c0term * exp( sedc * eterm ) ) :
                       CPijs[ ii ][ jj ];
            CPijs[ ii ][ jj ] = cpij;
            sumcpij += cpij;
//DbgLv(1) << "    div " << jj+1 << "  tcdps cpij " << tcpds.at(jj) << cpij;
         }

         // Set to split span-sum difference over each division
         sdiff    = ( span - sumcpij ) * divfac;

         for ( int jj = 0; jj < divsCount; jj++ )
         {  // Spread the difference to each partial plateau concentration
            CPijs[ ii ][ jj ] += sdiff;
         }

         avgdif  += qAbs( sdiff );  // Sum of difference magnitudes
         count++;
DbgLv(1) << "   iter scn " << iter << ii+1 << " sumcpij span "
   << sumcpij << span << "  sdiff sumabsdif" << sdiff << avgdif;
      }
//DbgLv(3) << "DP:TM:10: " << QTime::currentTime().toString("hh:mm:ss:zzzz");

      // Insure we have mid-division concentrations for newest partials
      update_mid_concs();

      avgdif  /= (double)count;     // Average of difference magnitudes
DbgLv(1) << " iter" << iter << " avg(abs(sdiff))" << avgdif;

      if ( avgdif < avdthr )        // If differences are small, we're done
      {
DbgLv(1) << "   +++ avgdif < avdthr (" << avgdif << avdthr << ") +++";
         break;
      }

      iter++;
   }

   for ( int ii = 0; ii < totalCount; ii++ )
      pty[ ii ]   = SEDC_NOVAL;

   int     kk     = 0;                    // Index to sed. coeff. values
   int     kl     = 0;                    // Index/count of live scans
   valueCount     = edata->x.size();

   // Calculate the corrected sedimentation coefficients

//DbgLv(3) << "DP:TM:11: " << QTime::currentTime().toString("hh:mm:ss:zzzz");
   for ( int ii = 0; ii < lscnCount; ii++ )
   {
      int js         = liveScans[ ii ];
      dscan          = &edata->scanData[ js ];

      double  timev  = dscan->seconds - time_correction;
      double  timex  = 1.0 / sqrt( timev );
      double  bdrad  = bdrads.at( kl );   // Back-diffus cutoff radius for scan
      double  bdcon  = bdcons.at( kl++ ); // Back-diffus cutoff concentration
      double  divrad = 0.0;               // Division radius value
DbgLv(1) << "scn liv" << ii+1 << kl
   << " radius concen time" << bdrad << bdcon << timev;

      ptx[ ii ]  = timex;                 // Save corrected time and accum max

      omega      = dscan->rpm * M_PI / 30.0;
      oterm      = ( timev > 0.0 ) ? ( timev * omega * omega ) : -1.0;

      for ( int jj = 0; jj < divsCount; jj++ )
      {  // walk through division points; get sed. coeff. by place in readings
         mconc        = mconcs[ ii ][ jj ];  // Mid div concentration

         sedc         = sed_coeff( mconc, oterm, &divrad, NULL );

         if ( divrad > bdrad )
         {  // Mark a point to be excluded by back-diffusion
            sedc        = SEDC_NOVAL;
DbgLv(1) << " *excl* div" << jj+1 << " drad dcon " << divrad << mconc;
         }

         // Y value of point is sedcoeff
         pty[ kk++ ] = sedc;
      }
   }
//DbgLv(3) << "DP:TM:12: " << QTime::currentTime().toString("hh:mm:ss:zzzz");

}

// Calculate Back-diffusion line
void US_vHW_Enhanced::calc_backdiff_line()
{
   double  toprad   = edata->x[ 0              ].radius;
   double  bottom   = edata->x[ valueCount - 1 ].radius;
   double  meniscus = edata->meniscus;
   double  radsum   = bottom + meniscus;
   double  bdifcsqr = sqrt( bdiff_coef );  // Sqrt( diff_coeff ) used below
   bdrads.clear();
   bdcons.clear();

   for ( int ii = 0; ii < lscnCount; ii++ )
   {
      double  timecor;   // Time (corrected)
      double  timesqr;   // Square root of corrected time
      double  bdleft;    // Back-diffusion left value
      double  xbdleft;   // Find-root of bdleft
      double  radD;      // Radius of back-diffusion point
      double  omegasq;   // Omega squared

      int js      = liveScans[ ii ];
      dscan       = &edata->scanData[ js ];

      omega       = dscan->rpm * M_PI / 30.0;
      omegasq     = omega * omega;
      timecor     = dscan->seconds - time_correction;
      timesqr     = sqrt( timecor );

      // Accumulate limits based on back diffusion

      // left = tolerance * sqrt( diff )
      //        / ( 2 * intercept[0] * omega^2
      //            * ( bottom + meniscus ) / 2 * sqrt( time ) )

      bdleft      = bdtoler * bdifcsqr
                    / ( bdiff_sedc * omegasq * radsum * timesqr );
      xbdleft     = find_root( bdleft );

      // radD = bottom - ( 2 * find_root(left) * sqrt( diff * time ) )

      radD        = bottom - ( 2.0 * xbdleft * bdifcsqr * timesqr );
      radD        = max( toprad, min( bottom, radD ) );

      int mm      = US_DataIO2::index( edata->x, radD );  // Radius's index

      // Accumulate for this scan of this division
      //  the back diffusion limit radius and corresponding concentration

      bdrads << radD;                        // BD Radius
      bdcons << dscan->readings[ mm ].value; // BD Concentration
DbgLv(1) << "  bottom meniscus bdleft" << bottom << edata->meniscus << bdleft;
DbgLv(1) << "  bdifsedc find_root toler" << bdiff_sedc << xbdleft << bdtoler;
DbgLv(2) << "  bdiff_coef bdifcsqr" << bdiff_coef << bdifcsqr;
DbgLv(1) << "  ii" << ii << " bdrad bdcon" << radD << bdcons[bdcons.size()-1];
   }
}

// Flag whether we have a model to use for finite-element plateau determination
bool US_vHW_Enhanced::have_model()
{
   row           = lw_triples->currentRow();
   ti_noise      = tinoises[ row ];
   ri_noise      = rinoises[ row ];
   int n_ti_noi  = ti_noise.values.size();
   int n_ri_noi  = ri_noise.values.size();
   QString modelGUID;

   if ( n_ti_noi > 0 )
      modelGUID = ti_noise.modelGUID;
   else if ( n_ri_noi > 0 )
      modelGUID = ri_noise.modelGUID;

   return ( ! modelGUID.isEmpty()  &&  modelGUID.length() == 36 );
}

// Plot any upper plot vertical excluded-scan lines
void US_vHW_Enhanced::vert_exclude_lines()
{
   double ymax;
   double xx[ 2 ];
   double yy[ 2 ];
   int frsc   = qRound( ct_from->value() );
   int tosc   = qRound( ct_to  ->value() );

   if ( tosc <= 0 )   return;

   // First remove previously drawn red vertical lines
   QwtPlotItemList list = data_plot1->itemList();
   for ( int ii = 0; ii < list.size(); ii++ )
   {
      QwtPlotItem* curve = list[ ii ];
      if ( curve->title().text().contains( "Exclude Marker" ) )
      {
         curve->detach();
      }
   }

   ymax     = 0.0;
   for ( int ii = 0; ii < aseds.size(); ii++ )
      ymax     = qMax( ymax, aseds[ ii ] );  // Max sed.coeff. value
   frsc     = ( frsc < 1 ) ? 1 : frsc;
   yy[ 0 ]  = 0.5;
   yy[ 1 ]  = ymax - 0.1;

   for ( int ii = 0; ii < lscnCount; ii++ )
   { 
      int ixsc = ii + 1;
      if ( ixsc < frsc )  continue;
      if ( ixsc > tosc )  break;

      int js   = liveScans[ ii ];
      xx[ 0 ]  = 1.0 / sqrt( edata->scanData[ js ].seconds - time_correction );
      xx[ 1 ]  = xx[ 0 ];
      curve    = us_curve( data_plot1,
         tr( "Scan %1 Exclude Marker" ).arg( ixsc ) );
      curve->setPen( QPen( QBrush( Qt::red ), 1.0 ) );
      curve->setData( xx, yy, 2 );
   }
}

// Slot to handle exclude-from change
void US_vHW_Enhanced::exclude_from( double from )
{
   double to = ct_to->value();

   if ( to < from )
   {  // Adjust exclude-to if need be
      ct_to->disconnect();
      ct_to->setValue( from );

      connect( ct_to,   SIGNAL( valueChanged( double ) ),
                        SLOT  ( exclude_to  ( double ) ) );
   }

   // Mark upper plot excluded scans, then lower plot ones
   skipPlot = true;
   vert_exclude_lines();
   data_plot1->replot();

   US_AnalysisBase2::data_plot();
   skipPlot = false;
}

// Slot to handle exclude-to change
void US_vHW_Enhanced::exclude_to( double to )
{
DbgLv(1) << "(1)TO=" << to;
   double from = ct_from->value();

   if ( from > to  ||  from == 0 )
   {  // Adjust exclude-from if need be
DbgLv(1) << "(2)TO=" << to;
      ct_from->disconnect();
      if ( from > to )
         ct_from->setValue( to );
      else if ( to > 0.0 )
         ct_from->setValue( 1.0 );

      connect( ct_from, SIGNAL( valueChanged( double ) ),
                        SLOT  ( exclude_from( double ) ) );
DbgLv(1) << "(3)TO=" << to;
   }

   // Mark upper plot excluded scans, then lower plot ones
   if ( to > 0.0 )
   {
DbgLv(1) << "(4)TO=" << to;
      skipPlot = true;
      vert_exclude_lines();
      data_plot1->replot();

      US_AnalysisBase2::data_plot();
      skipPlot = false;
DbgLv(1) << "(5)TO=" << to;
   }

   else                     // Special case of to=0 after exclude clicked
   {
DbgLv(1) << "(8)TO=" << to;
      data_plot();
DbgLv(1) << "(9)TO=" << to;
   }
}

// Build vectors of live scan information
void US_vHW_Enhanced::live_scans()
{
   row        = lw_triples->currentRow();
   edata      = &dataList[ row ];
   scanCount  = edata->scanData.size();
   lscnCount  = 0;
   liveScans.clear();

   for ( int ii = 0; ii < scanCount; ii++ )
   {  // For non-excluded scans, save scan index and plateau concentration
      if ( ! excludedScans.contains( ii ) )
      {
         liveScans << ii;      // Save original scan index
         lscnCount++;          // Bump count of live scans
      }
   }
}

// Build initial versions of partial-conc and mid-div conc vectors
void US_vHW_Enhanced::init_partials()
{
   double  mconc;              // Mid-division concentration
   double  cinc;               // Constant division concentration increment
   divsCount  = qRound( ct_division->value() );
   divfac     = 1.0 / (double)divsCount;
   CPijs .clear();
   mconcs.clear();

   for ( int ii = 0; ii < lscnCount; ii++ )
   {  // Populate partial concentration lists
      QVector< double >  div_pconcs;
      QVector< double >  div_mconcs;
      range      = scPlats[ ii ] - baseline;    // Range for this scan
      cinc       = range * boundPct * divfac;   // Conc. incr. this scan
      mconc      = baseline + range * positPct + cinc * 0.5; // 1st Mid-div conc
      div_pconcs.clear();
      div_mconcs.clear();

      for ( int jj = 0; jj < divsCount; jj++ )
      {
         div_pconcs << cinc;   // Save partial concentration
         div_mconcs << mconc;  // Save mid-div concentration
         mconc += cinc;        // Bump to next division
      }

      CPijs  << div_pconcs;    // Save a vector for each scan
      mconcs << div_mconcs;
   }
}

// Update mid-division concentrations
void US_vHW_Enhanced::update_mid_concs()
{
   double  bconc;    // Base concentration of a division
   double  mconc;    // Mid concentration of a division
   double  cpij;     // Partial concentration of a division
   mconcs.clear();   // Clear out old mid-division concentrations

   for ( int ii = 0; ii < lscnCount; ii++ )
   {  // Populate mid-division concentration vector of each scan
      QVector< double >  div_mconcs;
      range      = scPlats[ ii ] - baseline;
      bconc      = baseline + range * positPct;  // Division 1 base conc.
      div_mconcs.clear();

      for ( int jj = 0; jj < divsCount; jj++ )
      {  // Determine new concentrations in divisions of the scan
         cpij       = CPijs[ ii ][ jj ];   // Partial concentration of div.
         mconc      = bconc + cpij * 0.5;  // Mid-division concentration
         bconc     += cpij;                // Next division base
         div_mconcs << mconc;              // Save new mid-div concentration
DbgLv(1) << "UPD: mconc cpij" << mconc << cpij << "ii jj" << ii << jj;
      }

      mconcs << div_mconcs;    // Save a vector for each scan
   }
}

