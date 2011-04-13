//! \file us_distrib_plot.cpp

#include <QtSvg>

#include "us_distrib_plot.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_math2.h"

#include <qwt_legend.h>

US_DistribPlot::US_DistribPlot( const QList< double >& divfracs,
      const QList< double >& divsedcs )
   : US_WidgetsDialog( 0, 0 )
{

   setWindowTitle( tr( "van Holde - Weischet Distribution Plot" ) );
   setPalette( US_GuiSettings::frameColor() );

   QGridLayout* main = new QGridLayout( this );
   main->setSpacing        ( 2 );
   main->setContentsMargins( 2, 2, 2, 2 );

   bfracs    = divfracs;
   dsedcs    = divsedcs;

   plotType  = DISTR;
   plotTypeH = COMBO;
   divsCount = bfracs.size();
   nSensit   = 50;
   nSmooth   = 30;
   dbg_level = US_Settings::us_debug();

   int row   = 0;

   // Plot Rows
   QBoxLayout* plot = new US_Plot( data_plot,
        tr( "Sedimentation Coefficient Distribution" ),
        tr( "Sedimentation Coefficient" ),
        tr( "Boundary Fraction" ) );

   data_plot->setCanvasBackground( Qt::black );
   data_plot->setMinimumSize( 600, 500 );
   data_plot->setAxisScale( QwtPlot::yLeft,   0.0, 100.0, 20.0 );
   data_plot->setAxisScale( QwtPlot::xBottom, 0.0,   6.9,  1.0 );

   QwtPlotGrid* grid = us_grid( data_plot );
   grid->enableXMin( true );
   grid->enableYMin( true );
   grid->setMajPen( QPen( US_GuiSettings::plotMajGrid(), 0, Qt::DashLine ) );
   grid->setMinPen( QPen( US_GuiSettings::plotMinGrid(), 0, Qt::DotLine ) );

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
   ct_sensitivity->setStep( 1.0 );
   ct_sensitivity->setValue( 50.0 );
   main->addWidget( pb_histogram,   row,   0, 1, 1 );
   main->addWidget( lb_sensitivity, row,   1, 1, 1 );
   main->addWidget( ct_sensitivity, row++, 2, 1, 1, Qt::AlignLeft );

   // Control Row 3
   pb_envelope    = us_pushbutton( tr( "Hide Envelope" ) );
   lb_smoothing   = us_label(      tr( "Smoothing:" ) );
   ct_smoothing   = us_counter( 2, 10, 100, 1 );
   pb_envelope-> setEnabled( false );
   ct_smoothing->setEnabled( false );
   ct_smoothing->setStep( 1.0 );
   ct_smoothing->setValue( 30.0 );
   main->addWidget( pb_envelope,    row,   0, 1, 1 );
   main->addWidget( lb_smoothing,   row,   1, 1, 1 );
   main->addWidget( ct_smoothing,   row++, 2, 1, 1, Qt::AlignLeft );

   show_plot();

   connect( pb_plot_type,   SIGNAL( clicked()    ),
            this,           SLOT  ( type_plot()  ) );
   connect( pb_help,        SIGNAL( clicked()    ),
            this,           SLOT  ( help()       ) );
   connect( pb_close,       SIGNAL( clicked()    ),
            this,           SLOT  ( close()      ) );
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
   QSvgGenerator generator;
   QSvgGenerator generator2;

   // Set up, generate distribution plot and save it to a file
   QwtPlotGrid* grid = us_grid( data_plot );
   grid->enableXMin( true );
   grid->enableYMin( true );
   grid->setMajPen( QPen( US_GuiSettings::plotMajGrid(), 0, Qt::DashLine ) );
   grid->setMinPen( QPen( US_GuiSettings::plotMinGrid(), 0, Qt::DotLine ) );
   plotType = DISTR;

   plot_distrib();

   generator.setSize( data_plot->size() );
   generator.setFileName( plot1File );
   data_plot->print( generator );

   // Set up, generate combined histogram plot and save it to a file
   data_plot->detachItems();
   grid = us_grid( data_plot );
   grid->enableXMin( true );
   grid->enableYMin( true );
   grid->setMajPen( QPen( US_GuiSettings::plotMajGrid(), 0, Qt::DashLine ) );
   grid->setMinPen( QPen( US_GuiSettings::plotMinGrid(), 0, Qt::DotLine ) );
   plotType = COMBO;

   plot_combined();

   generator2.setSize( data_plot->size() );
   generator2.setFileName( plot2File );
   data_plot->print( generator2 );
}

// clear plot and execute appropriate new plot
void US_DistribPlot::show_plot( void )
{

   data_plot->detachItems();

   QwtPlotGrid* grid = us_grid( data_plot );
   grid->enableXMin( true );
   grid->enableYMin( true );
   grid->setMajPen( QPen( US_GuiSettings::plotMajGrid(), 0, Qt::DashLine ) );
   grid->setMinPen( QPen( US_GuiSettings::plotMinGrid(), 0, Qt::DotLine ) );


   if ( plotType == DISTR )
   {
      plot_distrib();
   }

   else if ( plotType == COMBO )
   {
      plot_combined();
   }

   else if ( plotType == HISTO  ||  plotType == NONE )
   {
      plot_histogram();
   }

   else if ( plotType == ENVEL )
   {
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
   // Set up the titles and axes
   data_plot->setTitle( 
      tr( "Sedimentation Coefficient Distribution" ) );
   data_plot->setAxisTitle( QwtPlot::yLeft,
      tr( "Boundary Fraction" ) );
   data_plot->setAxisTitle( QwtPlot::xBottom,
      tr( "Sedimentation Coefficient" ) );
   data_plot->setAxisScale( QwtPlot::yLeft,   0.0, 100.0, 20.0 );
 
   // create the x,y arrays of sedcoeffs,boundfracs

   QVector< double > xv( divsCount );
   QVector< double > yv( divsCount );
   double* xx = xv.data();
   double* yy = yv.data();
   double maxx = 0.0;
   double minx = 100.0;

   for ( int jj = 0; jj < divsCount; jj++ )
   {
      xx[ jj ] = dsedcs.at( jj );
      yy[ jj ] = bfracs.at( jj );
      maxx     = max( maxx, xx[ jj ] );
      minx     = min( minx, xx[ jj ] );
   }

   maxx     = (double)( (int)( maxx / 2.0 ) + 1 ) * 2.0;
   maxx     = ( ( maxx - minx ) < 2.0 ) ? ( maxx + 1.0 ) : maxx;
   data_plot->setAxisScale( QwtPlot::xBottom, 0.0,  maxx,  1.0 );

   // first draw the yellow line through points
   dcurve  = us_curve( data_plot, tr( "Distrib Line" ) );
   dcurve->setPen( QPen( QBrush( Qt::yellow ), 1.0 ) );
   dcurve->setData( xx, yy, divsCount );

   // then draw the symbols at each point
   dcurve  = us_curve( data_plot, tr( "Distrib Points" ) );
   QwtSymbol  sym;
   sym.setStyle( QwtSymbol::Ellipse );
   sym.setPen  ( QPen( Qt::blue ) );
   sym.setBrush( QBrush( Qt::white ) );
   sym.setSize ( 8 );
   dcurve->setStyle( QwtPlotCurve::NoCurve );
   dcurve->setSymbol( sym );
   dcurve->setData  ( xx, yy, divsCount );

   data_plot->replot();
}

// plot histogram
void US_DistribPlot::plot_histogram( void )
{
   double* xx;
   double* yy;
   double  maxx;
   double  maxy;
   int     npoints;

   // Set up the titles and axes
   data_plot->setTitle( 
      tr( "Sedimentation Coefficient Histogram" ) );
   data_plot->setAxisTitle( QwtPlot::yLeft,
      tr( "Relative Concentration" ) );
   data_plot->setAxisTitle( QwtPlot::xBottom,
      tr( "Sedimentation Coefficient" ) );
 
   // Calculate histogram data
   npoints  = histo_data( &xx, &yy );
   maxx     = xx[ 0 ];
   maxy     = yy[ 0 ];

   for ( int jj = 1; jj < npoints; jj++ )
   {
      maxx     = max( maxx, xx[ jj ] );
      maxy     = max( maxy, yy[ jj ] );
   }

   maxx     = (double)( (int)( maxx / 2.0 ) + 1 ) * 2.0;
   maxy     = (double)( (int)( maxy / 2.0 ) + 1 ) * 2.0;
   data_plot->setAxisScale( QwtPlot::yLeft,   0.0,  maxy,  5.0 );
   data_plot->setAxisScale( QwtPlot::xBottom, 0.0,  maxx,  1.0 );

DbgLv(2) << "HISTO_DAT:" << npoints;
for(int jj=0;jj<npoints;jj++) DbgLv(2) << jj << xx[jj] << yy[jj];

   // Draw curve of histogram sticks
   hcurve  = us_curve( data_plot, tr( "Histogram Bar" ) );
   hcurve->setPen( QPen( QBrush( Qt::red ), 5.0 ) );
   hcurve->setStyle( QwtPlotCurve::Sticks );
   hcurve->setData( xx, yy, npoints );


   if ( plotType == HISTO  ||  plotType == NONE )
      data_plot->replot();

   delete [] xx;
   delete [] yy;
}

// plot envelope
void US_DistribPlot::plot_envelope( void )
{
   double* xx;
   double* yy;
   double  maxx;
   double  maxy;
   int     npoints;

   if ( plotType == ENVEL )
   { // if envelope only, must set titles and axes (otherwise handled by histo)
      data_plot->setTitle( 
         tr( "Sedimentation Coefficient Histogram" ) );
      data_plot->setAxisTitle( QwtPlot::yLeft,
         tr( "Relative Concentration" ) );
      data_plot->setAxisTitle( QwtPlot::xBottom,
         tr( "Sedimentation Coefficient" ) );

      // calculate histogram data in order to use its maximum Y
      npoints  = histo_data( &xx, &yy );
      maxx     = xx[ 0 ];
      maxy     = yy[ 0 ];

      for ( int jj = 1; jj < npoints; jj++ )
      {
         maxx     = max( maxx, xx[ jj ] );
         maxy     = max( maxy, yy[ jj ] );
      }

      maxx     = (double)( (int)( maxx / 2.0 ) + 1 ) * 2.0;
      maxy     = (double)( (int)( maxy / 2.0 ) + 1 ) * 2.0;
      data_plot->setAxisScale( QwtPlot::yLeft,   0.0,  maxy,  5.0 );
      data_plot->setAxisScale( QwtPlot::xBottom, 0.0,  maxx,  1.0 );

      delete [] xx;       // free up work arrays for re-use below
      delete [] yy;
   }

   // Calculate envelope data
   npoints  = envel_data( &xx, &yy );

   // Draw a cyan line through points
   ecurve  = us_curve( data_plot, tr( "Envelope Line" ) );
   ecurve->setPen( QPen( QBrush( Qt::cyan ), 3.0 ) );
   ecurve->setData( xx, yy, npoints );

   data_plot->replot();

   delete [] xx;
   delete [] yy;
}

// plot combined histogram and envelope
void US_DistribPlot::plot_combined( void )
{
   plot_histogram();
   plot_envelope();
}

// generate histogram data
int US_DistribPlot::histo_data( double** xxP, double** yyP )
{
   int     steps;
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

   double* bink = new double[ steps ];           // bin count array
   double* sval = new double[ steps ];           // sedcoeff array

   for ( int jj = 0; jj < steps; jj++ )
   {  // accumulate histogram values
      int kbin     = 0;
      sed_lo       = sed_bin * (double)jj;       // low bin sedcoeff
      sed_hi       = sed_lo + sed_bin;           // high bin sedcoeff
      sval[ jj ]   = ( sed_lo + sed_hi ) * 0.5;  // mid bin sedcoeff

      for ( int kk = 0; kk < divsCount; kk++ )
      {  // accumulate count of sedcoeff values in current bin
         sedc         = dsedcs.at( kk );

         if ( sedc >= sed_lo  &&  sedc < sed_hi )
            kbin++;
      }

      bink[ jj ]   = (double)kbin;             // current bin count
   }

   *xxP         = new double[ steps ];         // allocate return arrays
   *yyP         = new double[ steps ];

   for ( int jj = 0; jj < steps; jj++ )
   {  // copy internal arrays to ones returned
      (*xxP)[ jj ] = sval[ jj ];
      (*yyP)[ jj ] = bink[ jj ];
   }

   delete [] bink;                             // clean up
   delete [] sval;

   return steps;                               // return arrays' size
}

// generate envelope data
int US_DistribPlot::envel_data( double** xxP, double** yyP )
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

   if ( array <= steps )
   {  // insure envelope array size bigger than histogram array size
      array        = steps + 1;
   }

   double  bink   = 0.0;
   double  sval   = 0.0;
   double  pisqr  = sqrt( M_PI * 2.0 );
   double* xval   = new double[ array ];
   double* yval   = new double[ array ];
   double  scale  = max_step / (double)array;

   for ( int jj = 0; jj < array; jj++ )
   {  // initialize envelope values
      xval[ jj ]   = scale * (double)jj;
      yval[ jj ]   = 0.0;
   }

   sigma        = sed_bin * 0.02 * (double)nSmooth;

   for ( int jj = 0; jj < steps; jj++ )
   {  // calculate histogram values and envelope values based on them
      int kbin     = 0;
      sed_lo       = sed_bin * (double)jj;
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
         }
      }
   }

   for ( int kk = 0; kk < array; kk++ )
   {  // accumulate the envelope values sum
      env_sum     += yval[ kk ];
   }

   env_sum     *= xval[ 1 ];            // sum times X increment
   scale        = his_sum / env_sum;    // normalizing scale factor
DbgLv(2) << "ED: hsum esum scale " << his_sum << env_sum << scale;

   for ( int kk = 0; kk < array; kk++ )
   {  // normalize Y values
      yval[ kk ]  *= scale;
   }

   *xxP         = new double[ array ];  // allocate X,Y arrays
   *yyP         = new double[ array ];

   for ( int jj = 0; jj < array; jj++ )
   {  // copy internal arrays to the returned ones
      (*xxP)[ jj ] = xval[ jj ];
      (*yyP)[ jj ] = yval[ jj ];
   }

   delete [] xval;                      // clean up
   delete [] yval;

   return array;                        // return arrays' size
}


