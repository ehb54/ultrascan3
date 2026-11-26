//! \file us_distrib_stick.cpp
#include "us_distrib_stick.h"
#include "us_gui_settings.h"
#include "us_gui_util.h"
#include "us_plot.h"

#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>

void US_DistribStick::plot( US_Widgets* owner, QwtPlot* data_plot1, US_DataIO::EditedData* edata, US_Model& model_used, int type )
{
   QString pltitle =
      QObject::tr( "Run " ) + edata->runID + QObject::tr( " :\nCell " ) + edata->cell + " (" + edata->wavelength + " nm)";
   QString xatitle;
   QString yatitle = QObject::tr( "Rel. Concentr." );

   if ( type == 0 )
   {
      pltitle = pltitle + QObject::tr( "\ns20,W Distribution" );
      xatitle = QObject::tr( "Corrected Sedimentation Coefficient" );
   }

   else if ( type == 1 )
   {
      pltitle = pltitle + QObject::tr( "\nMW Distribution" );
      xatitle = QObject::tr( "Molecular Weight (Dalton)" );
   }

   else if ( type == 2 )
   {
      pltitle = pltitle + QObject::tr( "\nD20,W Distribution" );
      xatitle = QObject::tr( "D20,W (cm^2/sec)" );
   }

   dataPlotClear( data_plot1 );

   data_plot1->setTitle( pltitle );
   data_plot1->setAxisTitle( QwtPlot::yLeft, yatitle );
   data_plot1->setAxisTitle( QwtPlot::xBottom, xatitle );

   QwtPlotGrid*    data_grid = owner->us_grid( data_plot1 );
   QwtPlotCurve*   data_curv = owner->us_curve( data_plot1, "distro" );


   int             dsize = model_used.components.size();
   QVector<double> vecx( dsize );
   QVector<double> vecy( dsize );
   double*         xx   = vecx.data();
   double*         yy   = vecy.data();
   double          xmin = 1.0e30;
   double          xmax = -1.0e30;
   double          ymin = 1.0e30;
   double          ymax = -1.0e30;
   double          xval;
   double          yval;
   double          rdif;

   for ( int jj = 0; jj < dsize; jj++ )
   {
      xval   = ( type == 0 ) ? model_used.components[jj].s
                             : ( ( type == 1 ) ? model_used.components[jj].mw : model_used.components[jj].D );
      yval   = model_used.components[jj].signal_concentration;
      xx[jj] = xval;
      yy[jj] = yval;
      xmin   = qMin( xval, xmin );
      xmax   = qMax( xval, xmax );
      ymin   = qMin( yval, ymin );
      ymax   = qMax( yval, ymax );
   }

   rdif = ( xmax - xmin ) / 20.0;
   xmin -= rdif;
   xmax += rdif;
   rdif = ( ymax - ymin ) / 20.0;
   ymin -= rdif;
   ymax += rdif;
   xmin = ( type == 0 ) ? xmin : qMax( xmin, 0.0 );
   ymin = qMax( ymin, 0.0 );

   data_grid->enableYMin( true );
   data_grid->enableY( true );
   data_grid->setMajorPen( QPen( US_GuiSettings::plotMinGrid(), 0, Qt::DotLine ) );

   data_curv->setSamples( xx, yy, dsize );
   data_curv->setPen( QPen( Qt::yellow, 3, Qt::SolidLine ) );
   data_curv->setStyle( QwtPlotCurve::Sticks );

   data_plot1->setAxisAutoScale( QwtPlot::xBottom );
   data_plot1->setAxisAutoScale( QwtPlot::yLeft );
   data_plot1->setAxisScale( QwtPlot::xBottom, xmin, xmax );
   data_plot1->setAxisScale( QwtPlot::yLeft, ymin, ymax );

   data_plot1->replot();
}
