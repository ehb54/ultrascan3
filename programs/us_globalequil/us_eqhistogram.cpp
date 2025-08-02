//! \file us_eqhistogram.cpp

#include "us_eqhistogram.h"
#include "us_gui_settings.h"
#include "us_settings.h"
#include "us_math2.h"
#if QT_VERSION < 0x050000
#define setSamples(a,b,c)  setData(a,b,c)
#define setSymbol(a)       setSymbol(*a)
#endif

#ifdef ARRAY_SIZE
#undef ARRAY_SIZE
#endif
#define ARRAY_SIZE 50

// Main constructor
US_EqHistogram::US_EqHistogram( double od_limit,
                                QVector< EqScanFit >& scanfits,
                                QWidget* parent,
                                Qt::WindowFlags f )
 :US_WidgetsDialog( parent, f ),
   od_limit ( od_limit ),
   scanfits ( scanfits )
{
   setAttribute  ( Qt::WA_DeleteOnClose );
   setPalette    ( US_GuiSettings::frameColor() );

   // Main layout
   QVBoxLayout* main = new QVBoxLayout( this );
   main->setContentsMargins( 2, 2, 2, 2 );
   main->setSpacing        ( 2 );

   // Analyze data to determine type of histogram
   double min_extinc = 1.0e28;
   double max_extinc = -1.0;
   bool   first_scan = true;
   bool   multi_lamd = false;
   bool   ext_define = false;
   bool   plot_conc  = false;
   int    first_lamd = 0;

   for ( int ii = 0; ii < scanfits.size(); ii++ )
   {  // Accumulate information about wavelengths and extinctions
      if ( ! scanfits[ ii ].scanFit )  continue;

      int    lambda  = scanfits[ ii ].wavelen;
      double extincv = scanfits[ ii ].extincts[ 0 ];

      if ( first_scan )
      {
         first_lamd = lambda;
         first_scan = false;
      }

      else if ( first_lamd != lambda )
         multi_lamd = true;

      if ( extincv != 1.0 )
      {
         ext_define = true;
         min_extinc = std::min( min_extinc, extincv );
         max_extinc = std::max( max_extinc, extincv );
      }
   }

   // If extinctions exist, allow the option to plot concentrations
   if ( ext_define )
   {  // Extinctions are defined
      QString msgstr;

      if ( multi_lamd )
      {  // There are multiple wavelengths
         msgstr  = tr( "You appear to be fitting multiple wavelengths\n"
                       "for which different extinction coefficients are\n"
                       "available. Would you like to correct the optical\n"
                       "density profile for extinction, and display real\n"
                       "concentations?\n\n" );
      }

      else
      {  // There is a single wavelength
         msgstr  = tr( "You have extinction coefficients defined for\n"
                       "your scans. Would you like to correct the optical\n"
                       "density profile for extinction, and display real\n"
                       "concentations?\n\n" );
      }

      msgstr += tr( "NOTE: This program only corrects for the 1st component." );

      QMessageBox msgBox;
      msgBox.setWindowTitle( "Display Concentrations?" );
      msgBox.setText( msgstr );
      msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::No );

      if ( msgBox.exec() == QMessageBox::Yes )
         plot_conc = true;    // Plot-Molar-Concentration selected

      else
         plot_conc = false;   // Plot-Optical-Density selected
   }

   double xplot[ ARRAY_SIZE ];
   double yplot[ ARRAY_SIZE ];

   double tmp_od_lim = ( od_limit == 0.0 ) ? 1.0 : od_limit;
   double bin_low    = 0.01 / ( 1.2 * max_extinc );
   double bin_high   = tmp_od_lim / ( 1.2 * min_extinc );
   double x_incr     = 1.0 / (double)ARRAY_SIZE;
   double x_valu     = tmp_od_lim * x_incr;

   if ( plot_conc )
   {  // initial X value and X increment for Concentration
      x_valu       = bin_low;
      x_incr      *= ( bin_high - bin_low );
   }

   else
   {  // initial X value (as above) and X increment for Density
      x_incr       = x_valu;
   }

   for ( int jj = 0; jj < ARRAY_SIZE; jj++ )
   {  // Build X-values ramp and initialize Y's to zero
      xplot[ jj ]  = x_valu;
      x_valu      += x_incr;
      yplot[ jj ]  = 0.0;
   }

   for ( int ii = 0; ii < scanfits.size(); ii++ )
   {  // Build histogram data for each scan
      EqScanFit* scnf = &scanfits[ ii ];

      if ( ! scnf->scanFit )  continue;

      int    strtx   = scnf->start_ndx;
      int    stopx   = scnf->stop_ndx;
      int    stopn   = stopx + 1;
      double scl_ext = plot_conc ?
         ( 1.0 / ( scnf->pathlen * scnf->extincts[ 0 ] ) ) : 1.0;
      double xpres   = xplot[ 0 ] - x_incr;
      double xlast;
int nx=scnf->xvs.size();
qDebug() << "UH:  ii" << ii << " strt,stop,nx" << strtx << stopx << nx
   << " t_odlim scex" << tmp_od_lim << scl_ext << " yvs,e"
   << scnf->yvs[strtx] << scnf->yvs[stopx];

      for ( int jj = 0; jj < ARRAY_SIZE; jj++ )
      {  // Update histogram points for this scan
         xlast   = xpres;           // Lower limit for histogram bar
         xpres   = xplot[ jj ];     // Upper limit for histogram bar
         xpres   = std::min( xpres, tmp_od_lim );

         for ( int kk = strtx; kk < stopn; kk++ )  // test in radius range
         {  // Add to histogram point where Y's are within OD range
            double xtest   = scnf->yvs[ kk ] * scl_ext;

            if ( xtest > xlast  &&  xtest <= xpres )  // Y value in bar range,
               yplot[ jj ] += 1.0;                    //  so bump bar
         }
if ( jj < 3 || (jj+4) > ARRAY_SIZE )
qDebug() << "UH:    jj" << jj << " xp" << xpres << " yplot[jj]" << yplot[jj];
      }
   }

   // Create the histogram plot titles
   QString odtype = tr( "Optical Density" );
   QString mctype = tr( "Molar Concentration" );
   QString htitle = odtype + tr( " Histogram" );
   QString hxaxis = odtype + tr( " Bin" );
   QString hyaxis = tr( "Frequency" );
   QString wtitle = tr( "GlobalEquil " ) + htitle;
   int     points = ARRAY_SIZE;

   if ( plot_conc )
   {
      htitle.replace( odtype, mctype );
      hxaxis.replace( odtype, mctype );
      wtitle.replace( odtype, mctype );
   }

qDebug() << "UH: wtitle" << wtitle;
   setWindowTitle( wtitle );

   // Create the plot
   hplot = new US_Plot( hist_plot, htitle, hxaxis, hyaxis );
   QwtPlotGrid* grid = us_grid( hist_plot );
   grid->enableYMin( true );
   grid->enableY   ( true );

   hist_plot->setMinimumSize( 600, 400 );
   hist_plot->setAxisScale( QwtPlot::xBottom, 0.0,   1.0 );
   hist_plot->setAxisAutoScale( QwtPlot::yLeft );

   // Build a "curve" consisting of bars
   QwtPlotCurve* hcurve = us_curve( hist_plot, "Histogram Bar" );
   hcurve->setPen( QPen( QBrush( Qt::red ), 6.0 ) );
   hcurve->setStyle( QwtPlotCurve::Sticks );
qDebug() << "UH: x0 x1 xm xn" << xplot[0] << xplot[1] << xplot[48] << xplot[49];
qDebug() << "UH: y0 y1 ym yn" << yplot[0] << yplot[1] << yplot[48] << yplot[49];
   hcurve->setSamples( xplot, yplot, points );

   // Add a "curve" of circles at the max point of each bar
   QwtPlotCurve* pcurve = us_curve( hist_plot, "Histogram MaxPoints" );
   pcurve->setStyle( QwtPlotCurve::NoCurve );
   QwtSymbol* sym = new QwtSymbol;
   sym->setStyle( QwtSymbol::Ellipse );
   sym->setPen  ( QPen( Qt::blue ) );
   sym->setBrush( QBrush( Qt::yellow ) );
   sym->setSize ( 12 );
   pcurve->setSymbol ( sym );
   pcurve->setSamples( xplot, yplot, points );

   // Display the plot
   hist_plot->replot();

   main->addLayout( hplot );

   adjustSize();
}

