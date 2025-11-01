//! \file us_distrib_plot.cpp

#include "us_distrib_plot.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_gui_util.h"
#include "us_math2.h"
#include <qwt_legend.h>

US_DistribPlot::US_DistribPlot( QVector< double >& divfracs,
   QVector< double >& divsedcs, const double tconc )
   : US_WidgetsDialog( nullptr, Qt::WindowFlags() ), bfracs( divfracs ), dsedcs( divsedcs )
{

   setWindowTitle( tr( "van Holde - Weischet Distribution Plot" ) );
   setPalette( US_GuiSettings::frameColor() );

   QGridLayout* main = new QGridLayout( this );
   main->setSpacing        ( 2 );
   main->setContentsMargins( 2, 2, 2, 2 );

   tot_conc  = tconc;
   plotType  = DISTR;
   plotTypeH = COMBO;
   divsCount = bfracs.size();
   nSensit   = 20;
   nSmooth   = 100;
   dbg_level = US_Settings::us_debug();
DbgLv(1) << "DisPl: divsCount" << divsCount;
DbgLv(1) << "DisPl: isedsSize " << dsedcs.size() << divsedcs.size();
DbgLv(1) << "DisPl: fra0 fran" << bfracs[0] << bfracs[divsCount-1];
DbgLv(1) << "DisPl: sed0 sedn" << dsedcs[0] << dsedcs[divsCount-1];
DbgLv(1) << "DisPl: sed1 sed-" << dsedcs[1] << dsedcs[divsCount-2];
DbgLv(1) << "DisPl: sed2 sed-" << dsedcs[2] << dsedcs[divsCount-3];
//DbgLv(1) << "DisPl: ised0 isedn" << divsedcs[0] << divsedcs[divsCount-1];

   int row   = 0;

   // Plot Rows
   QBoxLayout* plot = new US_Plot( data_plot,
        tr( "Sedimentation Coefficient Distribution" ),
        tr( "Sedimentation Coefficient" ),
        tr( "Boundary Fraction" ) );

   data_plot->setCanvasBackground( Qt::black );
   data_plot->setMinimumSize( 600, 500 );
   data_plot->setAxisScale( QwtPlot::yLeft,   0.0, 100.0, 20.0 );
   //data_plot->setAxisScale( QwtPlot::xBottom, 0.0,   6.9,  1.0 );
   data_plot->setAxisAutoScale( QwtPlot::xBottom );

   QwtPlotGrid* grid = us_grid( data_plot );
   grid->enableXMin( true );
   grid->enableYMin( true );
   grid->setMajorPen( QPen( US_GuiSettings::plotMajGrid(), 0, Qt::DashLine ) );
   grid->setMinorPen( QPen( US_GuiSettings::plotMinGrid(), 0, Qt::DotLine ) );
   US_PlotPicker* pick = new US_PlotPicker( data_plot );
   pick->setTrackerPen( QColor( Qt::white ) );

   main->addLayout( plot, row, 0, 15, 4 );
   row += 15;

   // Control Row 1
   pb_plot_type   = us_pushbutton( tr( "Histogram" ) );
   pb_help        = us_pushbutton( tr( "Help" ) );
   pb_close       = us_pushbutton( tr( "Close" ) );
   main->addWidget( pb_plot_type,   row,   0, 1, 1 );
   main->addWidget( pb_help,        row,   1, 1, 1 );
   main->addWidget( pb_close,       row++, 2, 1, 1 );

   // Control Row 2
   pb_histogram   = us_pushbutton( tr( "Hide Histogram" ) );
   lb_sensitivity = us_label(      tr( "Sensitivity:" ) );
   ct_sensitivity = us_counter( 2, 10, 100, 1 );
   pb_histogram->  setEnabled( false );
   ct_sensitivity->setEnabled( false );
   ct_sensitivity->setSingleStep( 1.0 );
   ct_sensitivity->setValue( nSensit );
   main->addWidget( pb_histogram,   row,   0, 1, 1 );
   main->addWidget( lb_sensitivity, row,   1, 1, 1 );
   main->addWidget( ct_sensitivity, row++, 2, 1, 1, Qt::AlignLeft );

   // Control Row 3
   pb_envelope    = us_pushbutton( tr( "Hide Envelope" ) );
   lb_smoothing   = us_label(      tr( "Smoothing:" ) );
   ct_smoothing   = us_counter( 2, 10, 100, 1 );
   pb_envelope-> setEnabled( false );
   ct_smoothing->setEnabled( false );
   ct_smoothing->setSingleStep( 1.0 );
   ct_smoothing->setValue( nSmooth );
   main->addWidget( pb_envelope,    row,   0, 1, 1 );
   main->addWidget( lb_smoothing,   row,   1, 1, 1 );
   main->addWidget( ct_smoothing,   row++, 2, 1, 1, Qt::AlignLeft );

   show_plot();

   connect( pb_plot_type,   SIGNAL( clicked()    ),
            this,           SLOT  ( type_plot()  ) );
   connect( pb_help,        SIGNAL( clicked()    ),
            this,           SLOT  ( help()       ) );
   connect( pb_close,       SIGNAL( clicked()        ),
            this,           SLOT  ( save_and_close() ) );
   connect( pb_histogram,   SIGNAL( clicked()    ),
            this,           SLOT  ( hide_histo() ) );
   connect( pb_envelope,    SIGNAL( clicked()    ),
            this,           SLOT  ( hide_envel() ) );
   connect( ct_sensitivity, SIGNAL( valueChanged(  double ) ),
            this,           SLOT  ( change_sensit( double ) ) );
   connect( ct_smoothing,   SIGNAL( valueChanged(  double ) ),
            this,           SLOT  ( change_smooth( double ) ) );
}

// Generate distribution,histogram plots and save the SVG files
void US_DistribPlot::save_plots( QString& plot1File, QString& plot2File )
{
   // Set up, generate distribution plot and save it to a file
   dataPlotClear( data_plot );
   QwtPlotGrid* grid = us_grid( data_plot );
   grid->enableXMin( true );
   grid->enableYMin( true );
   grid->setMajorPen( QPen( US_GuiSettings::plotMajGrid(), 0, Qt::DashLine ) );
   grid->setMinorPen( QPen( US_GuiSettings::plotMinGrid(), 0, Qt::DotLine ) );
   plotType = DISTR;

   plot_distrib();

   US_GuiUtil::save_plot( plot1File, data_plot );

   // Set up, generate combined histogram plot and save it to a file
   dataPlotClear( data_plot );
   grid = us_grid( data_plot );
   grid->enableXMin( true );
   grid->enableYMin( true );
   grid->setMajorPen( QPen( US_GuiSettings::plotMajGrid(), 0, Qt::DashLine ) );
   grid->setMinorPen( QPen( US_GuiSettings::plotMinGrid(), 0, Qt::DotLine ) );
   plotType = COMBO;

   plot_combined();

   US_GuiUtil::save_plot( plot2File, data_plot );

   QString runID = plot1File.section( "/", -2, -2 );

   if ( runID == "tmp" )  return;

   // Also save the envelope data with present parameters
   QString dat2File = US_Settings::resultDir() + "/" + runID + "/"
      + plot1File.section( "/", -1, -1 ).section( ".", 0, 1 )
      + ".s-c-envelope.csv";

   save_data_file( dat2File );
}

// clear plot and execute appropriate new plot
void US_DistribPlot::show_plot( void )
{
   dataPlotClear( data_plot );

   QwtPlotGrid* grid = us_grid( data_plot );
   grid->enableXMin( true );
   grid->enableYMin( true );
   grid->setMajorPen( QPen( US_GuiSettings::plotMajGrid(), 0, Qt::DashLine ) );
   grid->setMinorPen( QPen( US_GuiSettings::plotMinGrid(), 0, Qt::DotLine ) );


   if ( plotType == DISTR )
   {
      data_plot->setTitle( tr( "G(s) Distribution" ) );
      plot_distrib();
   }

   else if ( plotType == COMBO )
   {
      data_plot->setTitle( tr( "g(s) Distribution" ) );
      plot_combined();
   }

   else if ( plotType == HISTO  ||  plotType == NONE )
   {
      data_plot->setTitle( tr( "g(s) Distribution" ) );
      plot_histogram();
   }

   else if ( plotType == ENVEL )
   {
      data_plot->setTitle( tr( "g(s) Distribution" ) );
      plot_envelope();
   }

}

// change type of plot to distribution or histogram and replot
void US_DistribPlot::type_plot( void )
{
   if ( plotType == DISTR )
   {  // change to histogram plot
      plotType  = plotTypeH;
      pb_plot_type->setText( tr( "Distribution" ) );
      pb_histogram  ->setEnabled( true );
      pb_envelope   ->setEnabled( true );
      ct_sensitivity->setEnabled( true );
      ct_smoothing  ->setEnabled( true );
   }

   else
   {  // change to distribution plot
      plotType  = DISTR;
      pb_plot_type->setText( tr( "Histogram" ) );
      pb_histogram  ->setEnabled( false );
      pb_envelope   ->setEnabled( false );
      ct_sensitivity->setEnabled( false );
      ct_smoothing  ->setEnabled( false );
   }

   show_plot();
}

// flip-flop between hide/show histogram and replot
void US_DistribPlot::hide_histo( void )
{
   bool isHis = ( plotType == HISTO  ||  plotType == COMBO );
   bool isEnv = ( plotType == ENVEL  ||  plotType == COMBO );

   if ( isHis )
   {  // turn off histogram
      plotType  = isEnv ? ENVEL : NONE;
      pb_histogram->setText( tr( "Show Histogram" ) );
   }

   else
   {  // turn on histogram
      plotType  = isEnv ? COMBO : HISTO;
      pb_histogram->setText( tr( "Hide Histogram" ) );
   }

   plotTypeH = plotType;

   show_plot();
}

// flip-flop between hide/show envelope and replot
void US_DistribPlot::hide_envel( void )
{
   bool isHis = ( plotType == HISTO  ||  plotType == COMBO );
   bool isEnv = ( plotType == ENVEL  ||  plotType == COMBO );

   if ( isEnv )
   {  // turn off envelope
      plotType  = isHis ? HISTO : NONE;
      pb_envelope->setText( tr( "Show Envelope" ) );
   }

   else
   {  // turn on envelope
      plotType  = isHis ? COMBO : ENVEL;
      pb_envelope->setText( tr( "Hide Envelope" ) );
   }

   plotTypeH = plotType;

   show_plot();
}

// get changed sensitivity value and replot
void US_DistribPlot::change_sensit( double value )
{
   nSensit   = (int)value;

   show_plot();
}

// get changed smoothing value and replot
void US_DistribPlot::change_smooth( double value )
{
   nSmooth   = (int)value;

   show_plot();
}

// plot distribution points
void US_DistribPlot::plot_distrib( void )
{
   // Set up the axes
   data_plot->setAxisTitle( QwtPlot::yLeft,
      tr( "Boundary Fraction" ) );
   data_plot->setAxisTitle( QwtPlot::xBottom,
      tr( "Sedimentation Coefficient" ) );
   data_plot->setAxisScale( QwtPlot::yLeft,   0.0, 100.0, 20.0 );
 
   // create the x,y arrays of sedcoeffs,boundfracs

   double* xx = dsedcs.data();
   double* yy = bfracs.data();
   double maxx = 0.0;
   double minx = 100.0;
//   double xinc = 1.0;
   double rngx = 100.0;

   for ( int jj = 0; jj < divsCount; jj++ )
   {
      maxx     = max( maxx, xx[ jj ] );
      minx     = min( minx, xx[ jj ] );
   }

   rngx     = maxx - minx;
   minx     = minx - rngx * 0.1;
   maxx     = maxx + rngx * 0.1;
//   xinc     = ( rngx < 15.0 ) ? xinc : ( xinc * 5.0 );
//   xinc     = ( rngx >  1.0 ) ? xinc : ( xinc * 0.2 );
//   data_plot->setAxisScale( QwtPlot::xBottom, minx, maxx, xinc );
   data_plot->setAxisAutoScale( QwtPlot::xBottom );

   // first draw the yellow line through points
   dcurve  = us_curve( data_plot, tr( "Distrib Line" ) );
   dcurve->setPen    ( QPen( QBrush( Qt::yellow ), 1.0 ) );
   dcurve->setSamples( xx, yy, divsCount );

   // then draw the symbols at each point
   dcurve  = us_curve( data_plot, tr( "Distrib Points" ) );
   QwtSymbol* sym = new QwtSymbol;
   sym->setStyle( QwtSymbol::Ellipse );
   sym->setPen  ( QPen( Qt::blue ) );
   sym->setBrush( QBrush( Qt::white ) );
   sym->setSize ( 8 );
   dcurve->setStyle  ( QwtPlotCurve::NoCurve );
   dcurve->setSymbol ( sym );
   dcurve->setSamples( xx, yy, divsCount );

   data_plot->replot();
}

// plot histogram
void US_DistribPlot::plot_histogram( void )
{
   QVector< double > xvec;
   QVector< double > yvec;
   double  minx = dsedcs[ 0 ];
   double  maxx = minx;;
//   double  xinc = 1.0;
   double  xval;
   double  rngx;
   int     npoints;

   // Set up the axes
   data_plot->setAxisTitle( QwtPlot::yLeft,
      tr( "Relative Concentration" ) );
   data_plot->setAxisTitle( QwtPlot::xBottom,
      tr( "Sedimentation Coefficient" ) );
 
   // Get scale from envelope data
   npoints     = envel_data( xvec, yvec );
   double* xx  = xvec.data();
   double* yy  = yvec.data();
   double esum = 0.0;

   for ( int jj = 0; jj < npoints; jj++ )
      esum    += yy[ jj ];

   double  yscl = tot_conc / esum;
 
   // Calculate histogram data
   npoints  = histo_data( xvec, yvec );
   xx         = xvec.data();
   yy         = yvec.data();

   for ( int jj = 0; jj < divsCount; jj++ )
   {
      xval     = dsedcs.at( jj );
      minx     = qMin( minx, xval );
      maxx     = qMax( maxx, xval );
   }

   rngx     = maxx - minx;
   minx     = minx - rngx * 0.10;
   maxx     = maxx + rngx * 0.10;
//   xinc     = ( rngx < 15.0 ) ? xinc : ( xinc * 5.0 );
//   xinc     = ( rngx >  1.0 ) ? xinc : ( xinc * 0.2 );
//   data_plot->setAxisScale(     QwtPlot::xBottom, minx, maxx, xinc );
   data_plot->setAxisAutoScale( QwtPlot::yLeft );
   data_plot->setAxisAutoScale( QwtPlot::xBottom );

#if 0
   double hsum = 0.0;
   for ( int jj = 0; jj < npoints; jj++ )
      hsum        += yy[ jj ];
   yscl         = tot_conc / hsum;
#endif
DbgLv(2) << "HISTO_DAT:" << npoints;
for(int jj=0;jj<npoints;jj++) DbgLv(2) << jj << xx[jj] << yy[jj];
   // Scale Y points so envelope integration equals total concentration
   for ( int jj = 0; jj < npoints; jj++ )
      yy[ jj ]    *= yscl;

   // Draw curve of histogram sticks
   hcurve  = us_curve( data_plot, tr( "Histogram Bar" ) );
   hcurve->setPen    ( QPen( QBrush( Qt::red ), 5.0 ) );
   hcurve->setStyle  ( QwtPlotCurve::Sticks );
   hcurve->setSamples( xx, yy, npoints );


   if ( plotType == HISTO  ||  plotType == NONE )
   {
      data_plot->replot();
   }
}

// plot envelope
void US_DistribPlot::plot_envelope( void )
{
   QVector< double > xvec;
   QVector< double > yvec;
   double* xx;
   double* yy;
   double  minx = dsedcs[ 0 ];
   double  maxx = minx;
//   double  xinc = 1.0;
   double  xval;
   double  rngx;
   int     npoints;

   for ( int jj = 0; jj < divsCount; jj++ )
   {
      xval     = dsedcs.at( jj );
      minx     = qMin( minx, xval );
      maxx     = qMax( maxx, xval );
   }

   rngx     = maxx - minx;
   minx     = minx - rngx * 0.10;
   maxx     = maxx + rngx * 0.10;

   if ( plotType == ENVEL )
   { // if envelope only, must set up axes (otherwise handled by histo)
      data_plot->setAxisTitle( QwtPlot::yLeft,
         tr( "Relative Concentration" ) );
      data_plot->setAxisTitle( QwtPlot::xBottom,
         tr( "Sedimentation Coefficient" ) );

      // calculate histogram data in order to use its maximum Y
      npoints  = histo_data( xvec, yvec );
      xx       = xvec.data();
      yy       = yvec.data();
//      xinc     = ( rngx < 15.0 ) ? xinc : ( xinc * 5.0 );
//      xinc     = ( rngx >  1.0 ) ? xinc : ( xinc * 0.2 );
//      data_plot->setAxisScale(     QwtPlot::xBottom, minx, maxx, xinc );
      data_plot->setAxisAutoScale( QwtPlot::yLeft );
      data_plot->setAxisAutoScale( QwtPlot::xBottom );
   }

   // Calculate envelope data
   npoints  = envel_data( xvec, yvec );
   xx       = xvec.data();
   yy       = yvec.data();

   // Scale Y points so integration equals total concentration
   double  esum = 0.0;

   for ( int jj = 0; jj < npoints; jj++ )
      esum    += yy[ jj ];

   double  yscl = tot_conc / esum;

   for ( int jj = 0; jj < npoints; jj++ )
      yy[ jj ] *= yscl;

   // Draw a cyan line through points
   ecurve  = us_curve( data_plot, tr( "Envelope Line" ) );
   ecurve->setPen    ( QPen( QBrush( Qt::cyan ), 3.0 ) );
   ecurve->setSamples( xx, yy, npoints );

   data_plot->replot();
}

// plot combined histogram and envelope
void US_DistribPlot::plot_combined( void )
{
   plot_histogram();
   plot_envelope();
}

// generate histogram data
int US_DistribPlot::histo_data( QVector< double >& xvec,
                                QVector< double >& yvec )
{
   int     steps;
   int     stoff    = 0;
   double  max_cept = 1.0e-6;
   double  min_cept = 1.0e+6;
   double  sed_bin  = dsedcs.at( 0 );
   double  div_scl  = (double)nSensit * (double)divsCount * 0.01;
   double  max_step;
   double  sed_lo;
   double  sed_hi;
   double  sedc;

   for ( int jj = 0; jj < divsCount; jj++ )
   {  // get min,max intercept sedimentation coefficients
      max_cept   = max( max_cept, dsedcs.at( jj ) );
      min_cept   = min( min_cept, dsedcs.at( jj ) );
   }

   // calculate values based on range and sensitivity
   sed_bin      = ( max_cept - min_cept ) / div_scl;
   max_step     = max_cept * 4.0 / 3.0;
   steps        = (int)( max_step / sed_bin );
   stoff        = (int)( min_cept / sed_bin ) - 1;
   stoff        = qMax( stoff, 0 );
   steps       -= stoff;

   xvec.fill( 0.0, steps );
   yvec.fill( 0.0, steps );
   double* sval = xvec.data();                   // sedcoeff array
   double* bink = yvec.data();                   // bin count array

   for ( int jj = 0; jj < steps; jj++ )
   {  // accumulate histogram values
      int kbin     = 0;
      sed_lo       = sed_bin * (double)( jj + stoff ); // low bin sedcoeff
      sed_hi       = sed_lo + sed_bin;                 // high bin sedcoeff
      sval[ jj ]   = ( sed_lo + sed_hi ) * 0.5;        // mid bin sedcoeff

      for ( int kk = 0; kk < divsCount; kk++ )
      {  // accumulate count of sedcoeff values in current bin
         sedc         = dsedcs.at( kk );

         if ( sedc >= sed_lo  &&  sedc < sed_hi )
            kbin++;
      }

      bink[ jj ]   = (double)kbin;             // current bin count
   }

   return steps;                               // return arrays' size
}

// Generate envelope data
int US_DistribPlot::envel_data( QVector< double >& xvec,
                                QVector< double >& yvec )
{
   int     steps;
   int     array    = 300;
   double  max_cept = 1.0e-6;
   double  min_cept = 1.0e+6;
   double  sed_bin  = dsedcs.at( 0 );
   double  his_sum  = 0.0;
   double  env_sum  = 0.0;
   double  div_scl  = (double)nSensit * (double)divsCount * 0.01;
   double  max_step;
   double  sigma;
   double  sed_lo;
   double  sed_hi;
   double  sedc;

   for ( int jj = 0; jj < divsCount; jj++ )
   {  // get min,max intercept sedimentation coefficients
      max_cept   = max( max_cept, dsedcs.at( jj ) );
      min_cept   = min( min_cept, dsedcs.at( jj ) );
   }

   // calculate values based on range and sensitivity
   sed_bin      = ( max_cept - min_cept ) / div_scl;
   max_step     = max_cept * 4.0 / 3.0;
   steps        = (int)( max_step / sed_bin );
DbgLv(2) << "ED:  steps" << steps << "sed_bin" << sed_bin;

   if ( array <= steps )
   {  // insure envelope array size bigger than histogram array size
      array        = steps + 1;
   }

   xvec.fill( 0.0, array );
   yvec.fill( 0.0, array );
   double  bink   = 0.0;
   double  sval   = 0.0;
   double  pisqr  = sqrt( M_PI * 2.0 );
   double* xval   = xvec.data();
   double* yval   = yvec.data();
   double  scale  = max_step / (double)array;
DbgLv(2) << "ED:  max_step array scale" << max_step << array << scale;

   for ( int jj = 0; jj < array; jj++ )
   {  // initialize envelope values
      xval[ jj ]   = scale * (double)( jj );
      yval[ jj ]   = 0.0;
   }

   sigma        = sed_bin * 0.02 * (double)nSmooth;
DbgLv(2) << "ED:  sed_bin sigma" << sed_bin << sigma;

   for ( int jj = 0; jj < steps; jj++ )
   {  // calculate histogram values and envelope values based on them
      int kbin     = 0;
      sed_lo       = sed_bin * (double)( jj );
      sed_hi       = sed_lo + sed_bin;
      sval         = ( sed_lo + sed_hi ) * 0.5;

      for ( int kk = 0; kk < divsCount; kk++ )
      {  // count sedcoeffs within current step range
         sedc         = dsedcs.at( kk );

         if ( sedc >= sed_lo  &&  sedc < sed_hi )
            kbin++;
      }

      bink         = (double)kbin;
      his_sum     += ( bink * sed_bin );  // bump histogram sum

      if ( kbin > 0 )
      {  // if non-empty bin, update envelope Y values
         for ( int kk = 0; kk < array; kk++ )
         {
            double xdif  = ( xval[ kk ] - sval ) / sigma;
            yval[ kk ]  += ( ( bink / ( sigma * pisqr ) )
               * exp( -( xdif * xdif ) / 2.0 ) );
DbgLv(2) << "ED:  kk" << kk << "xdif bink yval" << xdif << bink << yval[kk];
         }
      }
DbgLv(2) << "ED:    jj" << jj << "sval his_sum" << sval << his_sum;
   }

   for ( int kk = 0; kk < array; kk++ )
   {  // accumulate the envelope values sum
      env_sum     += yval[ kk ];
   }

   env_sum     *= scale;                // sum times X increment
   scale        = his_sum / env_sum;    // normalizing scale factor
DbgLv(2) << "ED: hsum esum scale " << his_sum << env_sum << scale;

   for ( int kk = 0; kk < array; kk++ )
   {  // normalize Y values
      yval[ kk ]  *= scale;
   }

   return array;                        // return arrays' size
}

// Save plots and data to temporary files and close
void US_DistribPlot::save_and_close()
{
   QString basename   = US_Settings::tmpDir() + "/vHW.temp.";
   QString tplot1File = basename + "s-c-distrib.svg";
   QString tplot2File = basename + "s-c-histo.svg";
   QString tdata2File = basename + "s-c-envelope.csv";

   save_plots(     tplot1File, tplot2File );
   save_data_file( tdata2File );

   close();
}

// Save envelope data file
void US_DistribPlot::save_data_file( QString data2File )
{
   QVector< double > hseds;
   QVector< double > hfrqs;
   QVector< double > eseds;
   QVector< double > efrqs;

   int nhpts  = histo_data( hseds, hfrqs );
   int nepts  = envel_data( eseds, efrqs );
   double tconc = tot_conc;

#if 0
   if ( tconc < 0.01 )
   {  // If concentration too small, bump it to 1.0e0 range
      double tcpwr = qFloor( log10( tconc ) );
      tconc        = tconc * pow( 10.0, -tcpwr );
DbgLv(1) << "SaveDat: tot_conc tconc" << tot_conc << tconc
 << "tcpwr" << tcpwr;
   }
#endif
DbgLv(1) << "SaveDat: file" << data2File << "nhpts nepts" << nhpts << nepts;

   QFile datf( data2File );

   if ( ! datf.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
      return;

   QTextStream ts( &datf );

   ts << tr( "\"S-value(Envelope)\",\"Frequency(E)\","
             "\"S-value(Histogram)\",\"Frequency(H)\","
             "\"TotalConcentration\"\n" );

   for ( int ii = 0; ii < nepts; ii++ )
   {
      QString line;
      if ( ii < nhpts )
         line = QString::asprintf( "\"%.6f\",\"%.6f\",\"%.6f\",\"%9.2f\",\"%9.4e\"\n",
eseds[ ii ], efrqs[ ii ], hseds[ ii ], hfrqs[ ii ],
tconc );
      else
         line = QString::asprintf( "\"%.6f\",\"%.6f\",\"\",\"\",\"\"\n",
eseds[ ii ], efrqs[ ii ] );

      line.replace( " ", "" );
      ts << line;
   }
DbgLv(1) << "SaveDat:   file written";

   datf.close();
}

