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
   le_model      = us_lineedit( tr( "No model for triple" ), -1, true );

   connect( pb_dstrpl,  SIGNAL( clicked()       ),
            this,       SLOT(   distr_plot()    ) );
   connect( pb_selegr,  SIGNAL( clicked()       ),
            this,       SLOT(   sel_groups()    ) );
   connect( pb_replot,  SIGNAL( clicked()       ),
            this,       SLOT(   plot_refresh()  ) );
   connect( ck_modelpl, SIGNAL( toggled( bool ) ),
            this,       SLOT(   data_plot()     ) );

   int jr = 2;
   parameterLayout->addWidget( pb_dstrpl,  jr,   0, 1, 2 );
   parameterLayout->addWidget( pb_selegr,  jr++, 2, 1, 2 );
   parameterLayout->addWidget( ck_manrepl, jr,   0, 1, 2 );
   parameterLayout->addWidget( pb_replot,  jr++, 2, 1, 2 );
   parameterLayout->addWidget( ck_modelpl, jr,   0, 1, 2 );
   parameterLayout->addWidget( le_model,   jr++, 2, 1, 2 );

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

   rightLayout->setStretchFactor( plotLayout1, 3 );
   rightLayout->setStretchFactor( plotLayout2, 2 );

   setMaximumSize( qApp->desktop()->size() - QSize( 80, 80 ) );
}

// load data
void US_vHW_Enhanced::load( void )
{
   US_AnalysisBase2::load();

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
   QList< double > bfracs;
   double  pterm    = 100.0 * positPct;
   double  bterm    = 100.0 * boundPct / (double)divsCount;
   double  bfrac;

   bfracs.clear();

   for ( int jj = 0; jj < divsCount; jj++ )
   {
      bfrac     = pterm + bterm * (double)( jj + 1 );
      bfracs.append( bfrac );
   }

   US_DistribPlot* dialog = new US_DistribPlot( bfracs, dseds );
   dialog->move( this->pos() + QPoint( 100, 100 ) );
   dialog->exec();
   delete dialog;
   row        = lw_triples->currentRow();
   saved.replace( row, true );
}

// data plot
void US_vHW_Enhanced::data_plot( void )
{
   double  xmax        = 0.0;
   double  ymax        = 0.0;
   int     count       = 0;
   int     totalCount;

DbgLv(1) << " data_plot: dataLoaded" << dataLoaded << "vbar" << vbar;
   if ( !dataLoaded  ||  vbar <= 0.0 )
      return;

   if ( !forcePlot  &&  ck_manrepl->isChecked() )
      return;

DbgLv(2) << "DP:TM:00: " << QTime::currentTime().toString("hh:mm:ss:zzzz");
   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
   row        = lw_triples->currentRow();
   d          = &dataList[ row ];

   data_plot2->detachItems();
   data_plot2->setAxisAutoScale( QwtPlot::yLeft );
   data_plot2->setAxisAutoScale( QwtPlot::xBottom );
   scanCount  = d->scanData.size();
   valueCount = d->x.size();
   boundPct   = ct_boundaryPercent->value() / 100.0;
   positPct   = ct_boundaryPos    ->value() / 100.0;
   baseline   = calc_baseline();

   QList< double >  scpds;
   int     nskip = 0;
   cpds.clear();
   divsCount  = qRound( ct_division->value() );
   totalCount = scanCount * divsCount;
   QVector< double > pxvec( scanCount  );
   QVector< double > pyvec( totalCount );
   double* ptx   = pxvec.data();   // t,log(p) points
   double* pty   = pyvec.data();
   divfac     = 1.0 / (double)divsCount;
   correc     = solution.s20w_correction * 1.0e13;
	C0         = 0.0;
	Swavg      = 0.0;
   omega      = d->scanData[ 0 ].rpm * M_PI / 30.0;
   plateau    = d->scanData[ 0 ].plateau;

   for ( int ii = 0; ii < scanCount; ii++ )
   {  // count the scans excluded due to position percent
      if ( excludedScans.contains( ii ) ) continue;
      
      s          = &d->scanData[ ii ];
      plateau    = avg_plateau( );
      range      = plateau - baseline;
      basecut    = baseline + range * positPct;
      
      if ( d->scanData[ ii ].readings[ 0 ].value > basecut ) nskip++;
   }
DbgLv(1) << " valueCount  totalCount" << valueCount << totalCount;
DbgLv(1) << "  scanCount  divsCount" << scanCount << divsCount;
   le_skipped->setText( QString::number( nskip ) );

   bool mdl_plats = have_model() &&  ck_modelpl->isChecked();

   if (  mdl_plats )
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

   if ( mdl_plats )
   {  // Calculate all plot points from model plateaus
      calc_points_by_model( ptx, pty );
   }

   else
   {  // Calculate all plot points from fitted plateaus
      calc_points_by_fit( ptx, pty );
   }

   // Draw the vHW extrapolation plot
   data_plot1->clear();
   us_grid( data_plot1 );

   data_plot1->setTitle( tr( "Run " ) + runID + tr( ": Cell " ) + d->cell
             + " (" + d->wavelength + tr( " nm) - vHW Extrapolation Plot" ) );

   data_plot1->setAxisTitle( QwtPlot::xBottom, tr( "(Time)^-0.5" ) );
   data_plot1->setAxisTitle( QwtPlot::yLeft  , 
         tr( "Corrected Sed. Coeff. (1e-13 s)" ) );

   int nxy    = ( scanCount > divsCount ) ? scanCount : divsCount;
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
   for ( int ii = 0; ii < scanCount; ii++ )
   {
      if ( excludedScans.contains( ii ) )
      {
         kk        += divsCount;      // Excluded:  bump to next scan
         continue;
      }
      
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

      for ( int ii = 0; ii < scanCount; ii++ )
      {
         if ( excludedScans.contains( ii ) ) continue;

         kk         = ii * divsCount + jj;  // Sed. coeff. index

         if ( ptx[ ii ] > 0.0  &&  pty[ kk ] > 0.0 )
         {  // Points for scans in a division
            x[ count ] = ptx[ ii ];
            y[ count ] = pty[ kk ];
            count++;
            omega      = d->scanData[ ii ].rpm * M_PI / 30.0;
         }
      }

      if ( count > 1 )
      {  // Fit a line to the scan points in a division
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

         curve  = us_curve( data_plot1, tr( "Fitted Line %1" ).arg( jj ) );
         curve->setPen( QPen( Qt::yellow ) );
         curve->setData( x, y, 2 );
      }
   }
//DbgLv(3) << "DP:TM:15: " << QTime::currentTime().toString("hh:mm:ss:zzzz");

   if ( Swavg == 0.0 )
   {  // If need be, calculate values for report
      Swavg      = slope / ( -2.0 * omega * omega );
     	C0         = exp( intcept );
DbgLv(1) << "Swavg(M): " << Swavg << " C0: " << C0 ;
   }

   // Set scales, then plot the points and lines
   xmax  *= 1.05;
   xmax   = (double)qRound( ( xmax + 0.0009 ) / 0.001 ) * 0.001;
   ymax   = (double)qRound( ( ymax + 0.3900 ) / 0.400 ) * 0.400;
   data_plot1->setAxisScale(     QwtPlot::xBottom, 0.0, xmax, 0.005 );
   data_plot1->setAxisAutoScale( QwtPlot::yLeft );

   count  = 0;

   for ( int ii = 0; ii < scanCount; ii++ )
   {  // Accumulate the points of the back-diffusion cutoff line

      if ( !excludedScans.contains( ii ) )
      {
         x[ count ] = bdrads.at( count );
         y[ count ] = bdcons.at( count );
DbgLv(2) << "   bd x y k " << x[count] << y[count] << count+1;
         count++;
      }
   }

   // Save all sedcoeff values for report files and plots
   aseds.clear();
   for ( int ii = 0; ii < totalCount; ii++ )
      aseds.append( pty[ ii ] );

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
   d          = &dataList[ lw_triples->currentRow() ];
   ts << html_header( "US_vHW_Enhanced",
         tr( "van Holde - Weischet Analysis" ), d );
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

      for ( int jj = 0; jj < ngrp; jj++ )
      {
         ts << table_row(
               QString().sprintf( "%3d:", jj + 1 ),
               QString().sprintf( "%6.2f", groupdat.at( jj ).sed ),
               QString().sprintf( "(%5.2f %%)", groupdat.at( jj ).percent ) );
      }
   }

   ts << indent( 4 ) + "</table>\n\n";

   ts << indent( 4 ) + "<br/><table>\n";
   ts << table_row( tr( "Average S:" ),
                    QString::number( Swavg * 1.0e13 ) );
   ts << table_row( tr( "Initial concentration from plateau fit:" ),
                     QString::number( C0 ) +  tr( " OD/fringes" ) );
   ts << indent( 4 ) + "</table>\n";

   double  sl;
   double  ci;
   double  sig;
   double  cor;
   QVector< double > xvec( scanCount );
   QVector< double > yvec( scanCount );
   double* x  = xvec.data();
   double* y  = yvec.data();
   QString tscn;
   QString tpla;

   ts << scan_info();

   for ( int ii = 0; ii < scanCount; ii++ )
   {  // accumulate time,plateau pairs for line fit
      s            = &d->scanData[ ii ];
      x[ ii ]      = s->seconds;
      y[ ii ]      = scplats[ ii ];
   }

   US_Math2::linefit( &x, &y, &sl, &ci, &sig, &cor, scanCount );

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
   d             = &dataList[ row ];
   QString tripl = QString( triples.at( row ) ).replace( " / ", "" );
   QString basernam  = US_Settings::resultDir() + "/" + d->runID
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
   QString basename  = US_Settings::reportDir() + "/" + d->runID
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
   QList< double > bfracs;
   double  bfrac    = 100.0 * positPct;
   double  bterm    = 100.0 * boundPct / (double)divsCount;
   bfracs.clear();

   for ( int jj = 0; jj < divsCount; jj++ )
   {
      bfrac    += bterm;
      bfracs.append( bfrac );
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
   int j2  = US_DataIO2::index( d->x, d->plateau ); // Index edit plateau radius
   int j1  = max( 0, ( j2 - PA_POINTS ) );          // Point to 20 points before
       j2  = min( valueCount, ( j2 + PA_POINTS + 1 ) ); // and 20 points after

   for ( int jj = j1; jj < j2; jj++ )    // Sum the 41 points centered at
      plato += s->readings[ jj ].value;  //  the scan plateau radial position

   plato /= (double)( j2 - j1 );         // Find and return the average

   return plato;
}

// Get sedimentation coefficient for a given concentration
double US_vHW_Enhanced::sed_coeff( double cconc, double oterm,
      double* radP, int* ndxP )
{
   int    j2   = first_gteq( cconc, s->readings, valueCount );
   double rv0  = -1.0;          // Mark radius excluded
   double sedc = -1.0;
//DbgLv(2) << "sed_coeff: cconc rdv0 rdcn" << cconc << s->readings[0].value
//  << s->readings[valueCount-1].value << " j2" << j2;

   if ( j2 >= 0  &&  oterm >= 0.0 )
   {  // Likely need to interpolate radius from two input values
      int j1      = j2 - 1;

      if ( j2 > 0 )
      {  // Interpolate radius value
         double av1  = s->readings[ j1 ].value;
         double av2  = s->readings[ j2 ].value;
         double rv1  = d->x[ j1 ].radius;
         double rv2  = d->x[ j2 ].radius;
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
      sedc        = correc * log( rv0 / d->meniscus ) / oterm;
DbgLv(2) << "sed_coeff:   rv0" << rv0 << " sedc" << sedc << " oterm" << oterm;
   }

   if ( radP != NULL )    *radP = rv0;
   if ( ndxP != NULL )    *ndxP = j2;
   return sedc;
}

// Calculate division sedimentation coefficient values (fitted line intercepts)
void US_vHW_Enhanced::div_seds( )
{
   QVector< double > rrv( valueCount );
   QVector< double > ccv( valueCount );
   QVector< double > xxv( scanCount );
   QVector< double > yyv( scanCount );
   QVector< double > ppv( scanCount );
   QVector< double > xrv( scanCount );
   QVector< double > yrv( scanCount );
   QVector< int    > llv( scanCount );
   double* rr       = rrv.data();
   double* xx       = xxv.data();
   double* yy       = yyv.data();
   double* pp       = ppv.data();
   double* xr       = xrv.data();
   double* yr       = yrv.data();
   int*    ll       = llv.data();
   int     nscnu    = 0;  // Number used (non-excluded) scans
   int     kscnu    = 0;  // Count of scans of div not affected by diffusion
   double  bdifcsqr = sqrt( bdiff_coef );  // Sqrt( diff_coeff ) used below
   double  pconc;
   double  cconc;
   double  mconc;
   bdtoler          = ct_tolerance->value();
   valueCount       = d->x.size();
   double  bottom   = d->x[ valueCount - 1 ].radius;

   for ( int jj = 0; jj < valueCount; jj++ )
      rr[ jj ] = d->x[ jj ].radius;            // Copy radius values

   // Do division-1 determination of base

//DbgLv(3) << "  DS:TM:01: " << QTime::currentTime().toString("hh:mm:ss:zzzz");
   for ( int ii = 0; ii < scanCount; ii++ )
   {
      double  timecor;   // Time (corrected)
      double  timesqr;   // Square root of corrected time
      double  bdleft;    // Back-diffusion left value
      double  xbdleft;   // Find-root of bdleft
      double  radD;      // Radius of back-diffusion point
      double  omegasq;   // Omega squared

      if ( !excludedScans.contains( ii ) )
      {
         s           = &d->scanData[ ii ];

         omega       = s->rpm * M_PI / 30.0;
         omegasq     = omega * omega;
         timecor     = s->seconds - time_correction;
         timesqr     = sqrt( timecor );
         pconc       = baseline + ( scplats[ ii ] - baseline ) * positPct;

         xx[ nscnu ] = 1.0 / timesqr;
         ll[ nscnu ] = ii;
         pp[ nscnu ] = pconc;               // Analysis baseline

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
            / ( bdiff_sedc * omegasq * ( bottom + d->meniscus ) * timesqr );
         xbdleft     = find_root( bdleft );

         // radD = bottom - ( 2 * find_root(left) * sqrt( diff * time ) )

         radD        = bottom - ( 2.0 * xbdleft * bdifcsqr * timesqr );
         radD        = max( d->x[ 0 ].radius, min( bottom, radD ) );

         int mm      = US_DataIO2::index( d->x, radD );  // Radius's index

         // Accumulate for this scan of this division
         //  the back diffusion limit radius and corresponding concentration
         xr[ nscnu ] = radD;                    // BD Radius
         yr[ nscnu ] = s->readings[ mm ].value; // BD Concentration
DbgLv(1) << "  bottom meniscus bdleft" << bottom << d->meniscus << bdleft;
DbgLv(1) << "  bdifsedc find_root toler" << bdiff_sedc << xbdleft << bdtoler;
DbgLv(2) << "  bdiff_coef bdifcsqr" << bdiff_coef << bdifcsqr;
DbgLv(1) << "BD x,y " << nscnu+1 << radD << yr[nscnu] << "  mm" << mm;
DbgLv(1) << "  ii nscnu" << ii << nscnu+1 << " bdsed radD yr"
 << bdiff_sedc << radD << yr[nscnu];

         nscnu++;                               // Bump scans-used count
      }
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
      double  radD;
//DbgLv(1) << "div_sed div " << jj+1;

      kscnu      = nscnu;

      // Accumulate y values for this division, across used scans

//DbgLv(3) << "  DS:TM:02: " << QTime::currentTime().toString("hh:mm:ss:zzzz");
      for ( int kk = 0; kk < nscnu; kk++ )
      { // Accumulate concentration, sed.coeff. for all scans, this div
         ii         = ll[ kk ];                   // Scan index
         s          = &d->scanData[ ii ];         // Scan pointer
//DbgLv(3) << "  DS:TM:03: " << QTime::currentTime().toString("hh:mm:ss:zzzz");
         pconc      = pp[ kk ];                   // Prev concen (baseline)
         cpij       = cpds.at( ii ).at( jj );     // Partial concen (increment)
         cconc      = pconc + cpij;               // Curr concen (plateau)
         mconc      = ( cconc + pconc ) * 0.5;    // Mid div concentration
         omega      = s->rpm * M_PI / 30.0;       // Omega
         oterm      = ( s->seconds - time_correction ) * omega * omega;
//DbgLv(3) << "  DS:TM:04: " << QTime::currentTime().toString("hh:mm:ss:zzzz");

         pp[ kk ]   = cconc;        // Division mark of concentration for scan
         yy[ kk ]   = sed_coeff( mconc, oterm, &radD, NULL );  // Sed.coeff.

         if ( radD > xr[ kk ] )
         {  // Gone beyond back-diffusion cutoff: exit loop with truncated list
            kscnu      = kk;
//DbgLv(2) << " div kscnu" << jj+1 << kscnu
//   << " radD xrkk" << radD << xr[kk] << " mconc yrkk" << mconc << yr[kk];
//if(kscnu==0) DbgLv(2) << "   pc cc cpij mm" << pconc << cconc << cpij << mm;
            break;
         }
//DbgLv(3) << "  DS:TM:06: " << QTime::currentTime().toString("hh:mm:ss:zzzz");
//if ( kk < 2 || kk > (nscnu-3) )
//DbgLv(2) << "div scn " << jj+1 << ii+1 << " pconc cconc " << pconc << cconc;

      }
//DbgLv(2) << " nscnu pp0 yy0 ppn yyn " << nscnu << yy[0] << pp[0]
//   << yy[nscnu-1] << pp[nscnu-1];

      QVector< double > xtvec;
      ii         = 0;
      for ( int kk = 0; kk < kscnu; kk++ )
      {  // Remove any leading points below meniscus
         if ( yy[ kk ] > 0.0 )
         { // Sed coeff value positive (not from below meniscus)
            if ( kk > ii )
            {
               yy[ ii ] = yy[ kk ];
            }
            ii++;
            xtvec << xx[ kk ];
DbgLv(1) << " KS    ii kk xx" << ii << kk << xx[ kk ];
         }
         else
            DbgLv(1) << "+++ SED<=0.0 div" << jj+1 << "scn" << kk+1;
      }
//DbgLv(3) << "  DS:TM:07: " << QTime::currentTime().toString("hh:mm:ss:zzzz");
int kscsv=kscnu;
      kscnu      = ii;
      double* xt = xtvec.data();

      if ( kscnu > 1 )
      {
         // Calculate and save the division sedcoeff and fitted line slope

         US_Math2::linefit( &xt, &yy, &slope, &dsed, &sigma, &corre, kscnu );
DbgLv(1) << " KS" << kscnu << kscsv << "jj xx0 xxn yy0 yyn" << jj << xt[0]
 << xt[kscnu-1] << yy[0] << yy[kscnu-1] << "slo sed" << slope << dsed;
      }

      else
      {
         ii         = nscnu / 2;
         dsed       = yy[ ii ];
         slope      = 0.0;
         sigma      = 0.0;
         corre      = 0.0;
         slope      = 0.0;
ii=(nscnu<1)?1:nscnu;
DbgLv(1) << "    nscnu" << nscnu << "pp0 yy0 ppn yyn " << pp[0] << yy[0]
   << pp[ii-1] << yy[ii-1];
      }

      dseds.append( dsed );    // Save fitted line intercept (Sed.Coeff.)

      dslos.append( slope );   // Save slope and other fitting values
      dsigs.append( sigma );
      dcors.append( corre );
      dpnts.append( kscnu );
//if((jj&7)==0||jj==(divsCount-1))
// DbgLv(1) << "     div dsed slope " << jj+1 << dsed << slope;
//DbgLv(3) << "  DS:TM:08: " << QTime::currentTime().toString("hh:mm:ss:zzzz");

   }
//DbgLv(3) << "  DS:TM:77: " << QTime::currentTime().toString("hh:mm:ss:zzzz");
DbgLv(1) << " dsed[0] " << dseds.at(0);
DbgLv(1) << " dsed[L] " << dseds.at(divsCount-1);
DbgLv(1) << " D_S: xr0 yr0 " << xr[0] << yr[0];
DbgLv(1) << " D_S: xrN yrN " << xr[nscnu-1] << yr[nscnu-1] << nscnu;

   bdrads.clear();
   bdcons.clear();

   for ( int kk = 0; kk < nscnu; kk++ )
   {  // Save the Radius,Concentration points for back-diffusion cutoff curve
      bdrads.append( xr[ kk ] );
      bdcons.append( yr[ kk ] );
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

   divsCount       = qRound( ct_division->value() );

   for ( int jj = 0; jj < divsCount; jj++ )
   {  // walk thru all division lines to see if within clicked range
      double sed      = dseds.at( jj );         // intercept sed coeff
      double slope    = dslos.at( jj );         // div line slope
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
   QString dirname  = dirres + "/" + d->runID;
   QString dirrept  = dirrep + "/" + d->runID;

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
   ts << "1/sqrt(t)" << fsep;

   for ( int jj = 0; jj < divsCount; jj++ )
   {
      QString line = QString().sprintf( "D%03dSedCoef", jj + 1 );
      ts << line << ( jj < lastDiv ? fsep : eoln );
   }

   // Output data
   for ( int ii = 0; ii < scanCount; ii++ )
   {
      // Each output line begins with reciprocal square root of scan time
      control      = fsep;
      QString dat  = QString().sprintf( "%11.8f",
         ( 1.0 / sqrt( d->scanData[ ii ].seconds - time_correction ) ) );
      ts << dat.simplified() << control;

      // Balance of line is a list of sedimentation coefficient values for
      //  the divisions in the scan 
      for ( int jj = 0; jj < divsCount; jj++ )
      {
         sedc         = aseds.at( kk++ );
         if ( sedc > 0 )
            dat          = QString().sprintf( "%8.5f", sedc ).simplified();
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
   QString filename = US_Settings::resultDir() + "/" + d->runID + "/vHW."
      + QString( triples.at( row ) ).replace( " / ", "" )
      + ".s-c-distrib.csv";
   QFile   res_f( filename );
   double  pterm    = 100.0 * positPct;
   double  bterm    = 100.0 * boundPct / (double)divsCount;
   double  bfrac;
   QString dline;

   if ( !res_f.open( QIODevice::WriteOnly | QIODevice::Text ) )
   {
      return;
   }
DbgLv(1) << "WD: filename " << filename;

   // write the line-fit variables for each division
   QTextStream ts( &res_f );
   ts << d->description << "\n";
   ts << tr( "%Boundary,Points,Slope,Intercept,Sigma,Correlation\n" );

   for ( int jj = 0; jj < divsCount; jj++ )
   {
      bfrac     = pterm + bterm * (double)( jj + 1 );

      dline.sprintf( "%9.2f,%7d,%12.6f,%12.6f,%12.6f,%12.6f\n", bfrac,
         dpnts.at( jj ), dslos.at( jj ), dseds.at( jj ),
         dsigs.at( jj ), dcors.at( jj ) );
      dline.replace( " ", "" );
      ts << dline;
   }

   res_f.close();
}

// write a file of vHW detailed division group model data
void US_vHW_Enhanced::write_model()
{
   QString filename = US_Settings::resultDir() + "/" + d->runID + "/vHW."
      + QString( triples.at( row ) ).replace( " / ", "" )
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
   ts << d->meniscus << "       " << tr( "\t# Meniscus in cm\n" );
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
   d          = &dataList[ row ];

   // Do some calculations handled in AnalysisBase, but needed here
   //  so that model plateau calculations work
   time_correction    = US_Math2::time_correction( dataList );
   solution.density   = le_density  ->text().toDouble();
   solution.viscosity = le_viscosity->text().toDouble();
   solution.vbar20    = le_vbar     ->text().toDouble();
   double avgTemp     = d->average_temperature();
   solution.vbar      = US_Math2::calcCommonVbar( solution_rec, avgTemp );
   US_Math2::data_correction( avgTemp, solution );

   // Do normal analysis triple updating, but suppress plotting for now
   dataLoaded = false;
   US_AnalysisBase2::update( row );
   dataLoaded = true;
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

   if ( have_model() )
      le_model->setText( tr( "A model IS implied for %1" ).arg( tripl ) );
   else
      le_model->setText( tr( "NO model is implied for %1" ).arg( tripl ) );
}

// Calculate the sedimentation coefficient intercept for division 1
double US_vHW_Enhanced::sedcoeff_intercept()
{
   QVector< double > xrvec( scanCount );
   QVector< double > yrvec( scanCount );
   double* xr      = xrvec.data();
   double* yr      = yrvec.data();
   double  sedc    = 0.0;
   int     nscnu   = 0;

   for ( int ii = 0; ii < scanCount; ii++ )
   {  // Accumulate x,y values:  1/sqrt(time), sed_coeff
      s               = &d->scanData[ ii ];

      if ( excludedScans.contains( ii ) ) continue;

      double range    = scplats[ ii ] - baseline;
      double basecut  = baseline + range * positPct;
      double platcut  = basecut  + range * boundPct;
      double cinc     = ( platcut - basecut ) * divfac;
      double mconc    = basecut + cinc * 0.5;
      double omega    = s->rpm * M_PI / 30.0;
      double timecor  = s->seconds - time_correction;
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

   sedc  *= 1.0e-13;
DbgLv(1) << "sedco-intcp   ++SEDC++" << sedc << nscnu;

   return sedc;
}

// Copy temporary files saved by plot dialog to report/results
void US_vHW_Enhanced::copy_data_files( QString plot1File,
      QString plot2File, QString data2File )
{
   QString tempbase   = US_Settings::tmpDir() + "/vHW.temp.";
   QString tplot1File = tempbase + "s-c-distrib.svg";
   QString tplot2File = tempbase + "s-c-histo.svg";
   QString tdata2File = tempbase + "s-c-envelope.dat";
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

   valueCount = d->x.size();
   scanCount  = d->scanData.size();
   divsCount  = qRound( ct_division->value() );
   totalCount = scanCount * divsCount;
   divfac     = 1.0 / (double)divsCount;
   boundPct   = ct_boundaryPercent->value() / 100.0;
   positPct   = ct_boundaryPos    ->value() / 100.0;
   baseline   = calc_baseline();
   correc     = solution.s20w_correction * 1.0e13;
	C0         = 0.0;
	Swavg      = 0.0;
   omega      = d->scanData[ 0 ].rpm * M_PI / 30.0;
   plateau    = d->scanData[ 0 ].plateau;

   // Do first experimental plateau calcs based on horizontal zones

   int     nrelp = 0;
   QVector< double > pxvec( scanCount  );
   QVector< double > pyvec( totalCount );
   QVector< double > sxvec( scanCount  );
   QVector< double > syvec( scanCount  );
   QVector< double > fyvec( scanCount  );
   double* ptx   = pxvec.data();   // t,log(p) points
   double* pty   = pyvec.data();
   double* csx   = sxvec.data();   // count,slope points
   double* csy   = syvec.data();
   double* fsy   = fyvec.data();   // fitted slope

   QList< double > plats;
   QList< int >    isrel;
   QList< int >    isunr;

   // accumulate reliable,unreliable scans and plateaus

   scplats.fill( 0.0, scanCount );
   nrelp    = 0;

DbgLv(1) << " Initial reliable (non-excluded) scan t,log(avg-plat):";
   // Initially reliable plateaus are all average non-excluded
   for ( int ii = 0; ii < scanCount; ii++ )
   {
      if ( excludedScans.contains( ii ) ) continue;

      s             = &d->scanData[ ii ];
      plateau       = avg_plateau();
      scplats[ ii ] = plateau;
      ptx[ nrelp ]  = s->seconds - time_correction;
      pty[ nrelp ]  = log( plateau );

      isrel.append( ii );
DbgLv(1) << ptx[nrelp] << pty[nrelp];
      nrelp++;
   }
//DbgLv(3) << "DP:TM:01: " << QTime::currentTime().toString("hh:mm:ss:zzzz");

   QList< double >  scpds;
   double  slope;
   double  intcp;
   double  sigma;
   double  corre;
   cpds.clear();
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

   for ( int ii = 0; ii < scanCount; ii++ )
   {  // each extrapolated plateau is exp of Y for X of corrected time
      s             = &d->scanData[ ii ];
      double  tc    = s->seconds - time_correction;

      s->plateau    = exp( tc * slope + intcp );
      scplats[ ii ] = s->plateau;
      //scplats[ ii ] = exp( tc * slope + intcp );
DbgLv(1) << " jj scan plateau " << ii << ii+1 << scplats[ii];
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
   double scorr = 1.0 / solution.s20w_correction;
   //double scorr = 1.0;
DbgLv(1) << "Cpl:    scorr" << scorr << solution.s20w_correction;
   d            = &dataList[ row ];
   valueCount   = d->x.size();
   scanCount    = d->scanData.size();
   scplats.fill( 0.0, scanCount );

   for ( int ii = 0; ii < scanCount; ii++ )
   {
      US_DataIO2::Scan* scan = &d->scanData[ ii ];
      double   omega    = scan->rpm * M_PI / 30.0;
      double   oterm    = ( scan->seconds - time_correction ) * omega * omega;
      double   cplat    = 0.0;

      for ( int jj = 0; jj < ncomp; jj++ )
      {
         US_Model::SimulationComponent* sc = &model.components[ jj ];
         double conc    = sc->signal_concentration;
         double sval    = sc->s * scorr;
         cplat         += ( conc / exp( oterm * sval ) );
      }
DbgLv(1) << "Cpl:  scan" << ii << "cplat" << cplat;

      scplats[ ii ]  = cplat;
      scan->plateau  = cplat;
   }

   return true;
}

// Do a manually forced re-plot of the data
void US_vHW_Enhanced::plot_refresh()
{
   forcePlot    = true;
   data_plot();
   forcePlot    = false;
}

// Calculate X,Y extrapolation points from model
void US_vHW_Enhanced::calc_points_by_model( double* ptx, double* pty )
{
   int     totalCount;

   QList< double >  scpds;
   double  cconc;
   double  pconc;
   double  mconc;
   double  cinc;
   double  cinch;
   double  oterm;
   divsCount  = qRound( ct_division->value() );
   totalCount = scanCount * divsCount;
   divfac     = 1.0 / (double)divsCount;
   cpds.clear();

   for ( int ii = 0; ii < scanCount; ii++ )
   {  // Populate partial concentration lists
      range      = scplats[ ii ] - baseline;
      cinc       = range * boundPct * divfac;
      scpds.clear();
      for ( int jj = 0; jj < divsCount; jj++ )
         scpds << cinc;
      cpds << scpds;
   }
   //calc_backdiff_line();
   div_seds();

   for ( int ii = 0; ii < totalCount; ii++ )
      pty[ ii ]   = -1.0;

   int     kk     = 0;                    // Index to sed. coeff. values
   int     kl     = 0;                    // Index/count of live scans
   valueCount     = d->x.size();

   // Calculate the corrected sedimentation coefficients

//DbgLv(3) << "DP:TM:11: " << QTime::currentTime().toString("hh:mm:ss:zzzz");
   for ( int ii = 0; ii < scanCount; ii++ )
   {
      if ( excludedScans.contains( ii ) )
      {
         kk         += divsCount;
         continue;
      }

      s              = &d->scanData[ ii ];
      double  timev  = s->seconds - time_correction;
      double  timex  = 1.0 / sqrt( timev );
      double  bdrad  = bdrads.at( kl );   // Back-diffus cutoff radius for scan
      double  bdcon  = bdcons.at( kl++ ); // Back-diffus cutoff concentration
      double  divrad = 0.0;               // Division radius value
DbgLv(1) << "scn liv" << ii+1 << kl
   << " radius concen time" << bdrad << bdcon << timev;

      ptx[ ii ]  = timex;                 // Save corrected time and accum max

      range      = scplats[ ii ] - baseline;
      cconc      = baseline + range * positPct; // Initial conc for span
      cinc       = range * boundPct * divfac;
      cinch      = cinc * 0.5;
      omega      = s->rpm * M_PI / 30.0;
      oterm      = ( timev > 0.0 ) ? ( timev * omega * omega ) : -1.0;

      for ( int jj = 0; jj < divsCount; jj++ )
      {  // walk through division points; get sed. coeff. by place in readings
         pconc       = cconc;               // Div base
         cconc       = pconc + cinc;        // Absolute concentration
         mconc       = pconc + cinch;       // Mid div concentration

         sedc        = sed_coeff( mconc, oterm, &divrad, NULL );

         if ( divrad > bdrad )
         {  // Mark a point to be excluded by back-diffusion
            sedc        = -1.0;
DbgLv(1) << " *excl* div" << jj+1 << " drad dcon " << divrad << mconc;
         }

         // Y value of point is sedcoeff; accumulate y max
         pty[ kk++ ] = sedc;
      }
   }
//DbgLv(3) << "DP:TM:12: " << QTime::currentTime().toString("hh:mm:ss:zzzz");
}

// Calculate X,Y extrapolation points by plateau fitting
void US_vHW_Enhanced::calc_points_by_fit( double* ptx, double* pty )
{
   int     count       = 0;
   int     totalCount;

   scanCount  = d->scanData.size();
   valueCount = d->x.size();
   boundPct   = ct_boundaryPercent->value() / 100.0;
   positPct   = ct_boundaryPos    ->value() / 100.0;
   baseline   = calc_baseline();

   QList< double >  scpds;
   double  cconc;
   double  pconc;
   double  mconc;
   double  cinc;
   double  cinch;
   double  eterm;
   double  oterm;
   cpds.clear();
   divsCount  = qRound( ct_division->value() );
   totalCount = scanCount * divsCount;
   divfac     = 1.0 / (double)divsCount;
   correc     = solution.s20w_correction * 1.0e13;
   omega      = d->scanData[ 0 ].rpm * M_PI / 30.0;
   plateau    = d->scanData[ 0 ].plateau;

   // Initialize plateau values for components of scans

//DbgLv(3) << "DP:TM:04: " << QTime::currentTime().toString("hh:mm:ss:zzzz");
   for ( int ii = 0; ii < scanCount; ii++ )
   {
      s          = &d->scanData[ ii ];

      range      = scplats[ ii ] - baseline;
      basecut    = baseline + range * positPct;
      platcut    = basecut  + range * boundPct;
      span       = platcut - basecut;
      cconc      = basecut;
      pconc      = basecut;
      mconc      = basecut;
      cinc       = span * divfac;
      cinch      = cinc / 2.0;
      omega      = s->rpm * M_PI / 30.0;
      oterm      = ( s->seconds - time_correction ) * omega * omega;
      eterm      = -2.0 * oterm / correc;
      c0term     = ( C0 - baseline ) * boundPct * divfac;
      sumcpij    = 0.0;

      scpds.clear();                       // clear this scan's Cp list

      if ( excludedScans.contains( ii ) )
      {
         for ( int jj = 0; jj < divsCount; jj++ )
            scpds.append( cinc );

         cpds << scpds;
         continue;
      }

//DbgLv(3) << "DP:TM:05: " << QTime::currentTime().toString("hh:mm:ss:zzzz");
      for ( int jj = 0; jj < divsCount; jj++ )
      {  // Calculate partial plateaus
         pconc      = cconc;              // prev (baseline) div concentration
         cconc     += cinc;               // curr (plateau) div concentration
         mconc      = pconc + cinch;      // mid div concentration

         // Get sedimentation coefficient for concentration
         sedc       = sed_coeff( mconc, oterm );

         // Calculate the partial concentration (cpij) for this division
         cpij       = ( sedc > 0.0 ) ?
                      ( c0term * exp( sedc * eterm ) ) :
                      cinc;
//DbgLv(1) << " scn div cinc cpij " << ii+1 << jj+1 << cinc << cpij;
//DbgLv(1) << "  sedc eterm eso " << sedc << eterm << (eterm*sedc);

         // Update cpij sum and add to divisions list for scan
         sumcpij   += cpij;     // Add to sum of partial concentrations
         scpds.append( cpij );  // Update scan list of partial concentrations
DbgLv(1) << "CPF: scn div" << ii+1 << jj+1 << " cinc cpij" << cinc << cpij
 << " sumcpij sedc" << sumcpij << sedc;
      }
//DbgLv(3) << "DP:TM:06: " << QTime::currentTime().toString("hh:mm:ss:zzzz");

      // Get span-minus-sum_cpij and divide by number of divisions
      sdiff    = ( span - sumcpij ) * divfac;
DbgLv(1) << "   sumcpij span " << sumcpij << span
 << " sumcpij/span " << (sumcpij/span);

      for ( int jj = 0; jj < divsCount; jj++ )
      {  // Spread the difference to each partial plateau concentration
         cpij     = scpds.at( jj ) + sdiff;
         scpds.replace( jj, cpij );
      }

      cpds << scpds;  // Add cpij list to scan's list-of-lists
   }

   // Fit a line to division 1 and use its intercept to calculate the
   //  back diffusion coefficient

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

      // Reset division plateaus

      for ( int ii = 0; ii < scanCount; ii++ )
      {
         if ( excludedScans.contains( ii ) )   continue;

         s        = &d->scanData[ ii ];
         range    = scplats[ ii ] - baseline;
         basecut  = baseline + range * positPct;
         platcut  = basecut  + range * boundPct;
         span     = platcut - basecut;
         sumcpij  = 0.0;
         omega    = s->rpm * M_PI / 30.0;
         oterm    = ( s->seconds - time_correction ) * omega * omega;
         eterm    = -2.0 * oterm / correc;
         c0term   = ( C0 - baseline ) * boundPct * divfac;
         scpds    = cpds.at( ii );  // Scan's list of partial concentrations
         cinc     = scpds.at( 0 );  // Average concentration increment (diff)
         scpds.clear();

         // Split the difference between divisions

         for ( int jj = 0; jj < divsCount; jj++ )
         {  // Recalculate partial concentrations based on sedcoeff intercepts
            sedc     = dseds[ jj ];
            cpij     = ( sedc > 0.0 ) ?
                       ( c0term * exp( sedc * eterm ) ) :
                       cinc;
            scpds.append( cpij );
            sumcpij += cpij;
//DbgLv(1) << "    div " << jj+1 << "  tcdps cpij " << tcpds.at(jj) << cpij;
         }

         // Set to split span-sum difference over each division
         sdiff    = ( span - sumcpij ) * divfac;

         for ( int jj = 0; jj < divsCount; jj++ )
         {  // Spread the difference to each partial plateau concentration
            cpij     = scpds.at( jj ) + sdiff;
            scpds.replace( jj, cpij );
         }

         cpds.replace( ii, scpds ); // Replace scan's list of divison Cp vals

         avgdif  += qAbs( sdiff );  // Sum of difference magnitudes
         count++;
DbgLv(1) << "   iter scn " << iter << ii+1 << " sumcpij span "
   << sumcpij << span << "  sdiff sumabsdif" << sdiff << avgdif;
      }
//DbgLv(3) << "DP:TM:10: " << QTime::currentTime().toString("hh:mm:ss:zzzz");

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
      pty[ ii ]   = -1.0;

   int     kk     = 0;                    // Index to sed. coeff. values
   int     kl     = 0;                    // Index/count of live scans
   valueCount     = d->x.size();

   // Calculate the corrected sedimentation coefficients

//DbgLv(3) << "DP:TM:11: " << QTime::currentTime().toString("hh:mm:ss:zzzz");
   for ( int ii = 0; ii < scanCount; ii++ )
   {
      if ( excludedScans.contains( ii ) )
      {
         kk         += divsCount;
         continue;
      }

      s              = &d->scanData[ ii ];

      double  timev  = s->seconds - time_correction;
      double  timex  = 1.0 / sqrt( timev );
      double  bdrad  = bdrads.at( kl );   // Back-diffus cutoff radius for scan
      double  bdcon  = bdcons.at( kl++ ); // Back-diffus cutoff concentration
      double  divrad = 0.0;               // Division radius value
DbgLv(1) << "scn liv" << ii+1 << kl
   << " radius concen time" << bdrad << bdcon << timev;

      ptx[ ii ]  = timex;                 // Save corrected time and accum max

      range      = scplats[ ii ] - baseline;
      cconc      = baseline + range * positPct; // Initial conc for span
      basecut    = cconc;
      omega      = s->rpm * M_PI / 30.0;
      oterm      = ( timev > 0.0 ) ? ( timev * omega * omega ) : -1.0;
      scpds      = cpds.at( ii );         // List of conc val of divs this scan

      for ( int jj = 0; jj < divsCount; jj++ )
      {  // walk through division points; get sed. coeff. by place in readings
         pconc       = cconc;               // Div base
         cpij        = scpds.at( jj );      // Div partial concentration
         cconc       = pconc + cpij;        // Absolute concentration
         mconc       = pconc + cpij * 0.5;  // Mid div concentration

         sedc        = sed_coeff( mconc, oterm, &divrad, NULL );

         if ( divrad > bdrad )
         {  // Mark a point to be excluded by back-diffusion
            sedc        = -1.0;
DbgLv(1) << " *excl* div" << jj+1 << " drad dcon " << divrad << mconc;
         }

         // Y value of point is sedcoeff; accumulate y max
         pty[ kk++ ] = sedc;
      }
   }
//DbgLv(3) << "DP:TM:12: " << QTime::currentTime().toString("hh:mm:ss:zzzz");

}

// Calculate Back-diffusion line
void US_vHW_Enhanced::calc_backdiff_line()
{
   double  toprad   = d->x[ 0              ].radius;
   double  bottom   = d->x[ valueCount - 1 ].radius;
   double  meniscus = d->meniscus;
   double  radsum   = bottom + meniscus;
   double  bdifcsqr = sqrt( bdiff_coef );  // Sqrt( diff_coeff ) used below
   bdrads.clear();
   bdcons.clear();

   for ( int ii = 0; ii < scanCount; ii++ )
   {
      if ( excludedScans.contains( ii ) )  continue;

      double  timecor;   // Time (corrected)
      double  timesqr;   // Square root of corrected time
      double  bdleft;    // Back-diffusion left value
      double  xbdleft;   // Find-root of bdleft
      double  radD;      // Radius of back-diffusion point
      double  omegasq;   // Omega squared

      s           = &d->scanData[ ii ];

      omega       = s->rpm * M_PI / 30.0;
      omegasq     = omega * omega;
      timecor     = s->seconds - time_correction;
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

      int mm      = US_DataIO2::index( d->x, radD );  // Radius's index

      // Accumulate for this scan of this division
      //  the back diffusion limit radius and corresponding concentration

      bdrads << radD;                    // BD Radius
      bdcons << s->readings[ mm ].value; // BD Concentration
DbgLv(1) << "  bottom meniscus bdleft" << bottom << d->meniscus << bdleft;
DbgLv(1) << "  bdifsedc find_root toler" << bdiff_sedc << xbdleft << bdtoler;
DbgLv(2) << "  bdiff_coef bdifcsqr" << bdiff_coef << bdifcsqr;
DbgLv(1) << "  ii" << ii << " dsed radD yr"
 << bdiff_sedc << radD << bdcons[bdcons.size()-1];
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

   ymax       = 0.0;
   for ( int ii = 0; ii < aseds.size(); ii++ )
      ymax         = qMax( ymax, aseds.at( ii ) );  // Max sed.coeff. value
   frsc       = ( frsc < 1 ) ? 1 : frsc;
   yy[ 0 ]    = 0.5;
   yy[ 1 ]    = ymax - 0.1;
   int ixsc   = 0;

   for ( int ii = 0; ii < scanCount; ii++ )
   { 
      if ( excludedScans.contains( ii ) )  continue;

      ixsc++;
      if ( ixsc < frsc )  continue;
      if ( ixsc > tosc )  break;

      xx[ 0 ]    = 1.0 / sqrt( d->scanData[ ii ].seconds - time_correction );
      xx[ 1 ]    = xx[ 0 ];
      curve      = us_curve( data_plot1,
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
   vert_exclude_lines();
   data_plot1->replot();

   US_AnalysisBase2::data_plot();
}

// Slot to handle exclude-to change
void US_vHW_Enhanced::exclude_to( double to )
{
   double from = ct_from->value();

   if ( from > to )
   {  // Adjust exclude-from if need be
      ct_from->disconnect();
      ct_from->setValue( to );

      connect( ct_from, SIGNAL( valueChanged( double ) ),
                        SLOT  ( exclude_from( double ) ) );
   }

   // Mark upper plot excluded scans, then lower plot ones
   if ( to > 0.0 )
   {
      vert_exclude_lines();
      data_plot1->replot();

      US_AnalysisBase2::data_plot();
   }

   else                     // Special case of to=0 after exclude clicked
      data_plot();
}

