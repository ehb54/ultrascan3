//! \file test_us_distrib_stick_gui.cpp
// US_DistribStick::plot against the distribution plots it replaced in FEMatch
// and the GMP reporter, kept here word for word as they were at 94e74d4f3.

#include "us3_gui_test_main.h"

#include "us_dataIO.h"
#include "us_distrib_stick.h"
#include "us_gui_settings.h"
#include "us_gui_util.h"
#include "us_model.h"
#include "us_plot.h"
#include "us_widgets.h"

#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_scale_div.h>

namespace
{
// The members the old code used, with the programs' names
struct FrozenStick : public US_Widgets
{
      QwtPlot*               data_plot1 = nullptr;
      US_DataIO::EditedData* edata      = nullptr;
      US_Model               model_used;
};

struct FrozenFeMatchStick : public FrozenStick
{
      void distrib_plot_stick( int type );
};

struct FrozenReporterStick : public FrozenStick
{
      void distrib_plot_stick( int type );
};

void FrozenFeMatchStick::distrib_plot_stick( int type )
{
   QString pltitle = tr( "Run " ) + edata->runID + tr( " :\nCell " ) + edata->cell + " (" + edata->wavelength + " nm)";
   QString xatitle;
   QString yatitle = tr( "Rel. Concentr." );

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

   dataPlotClear( data_plot1 );

   data_plot1->setTitle( pltitle );
   data_plot1->setAxisTitle( QwtPlot::yLeft, yatitle );
   data_plot1->setAxisTitle( QwtPlot::xBottom, xatitle );

   QwtPlotGrid*    data_grid = us_grid( data_plot1 );
   QwtPlotCurve*   data_curv = us_curve( data_plot1, "distro" );


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

void FrozenReporterStick::distrib_plot_stick( int type )
{
   QString pltitle = tr( "Run " ) + edata->runID + tr( " :\nCell " ) + edata->cell + " (" + edata->wavelength + " nm)";
   QString xatitle;
   QString yatitle = tr( "Rel. Concentr." );

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

   dataPlotClear( data_plot1 );

   data_plot1->setTitle( pltitle );
   data_plot1->setAxisTitle( QwtPlot::yLeft, yatitle );
   data_plot1->setAxisTitle( QwtPlot::xBottom, xatitle );

   QwtPlotGrid*    data_grid = us_grid( data_plot1 );
   QwtPlotCurve*   data_curv = us_curve( data_plot1, "distro" );


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

// What the plot shows: titles, axis ranges, and each item with its samples and pen
QString describe( QwtPlot* plot )
{
   QStringList out;
   out << plot->title().text() << plot->axisTitle( QwtPlot::yLeft ).text() << plot->axisTitle( QwtPlot::xBottom ).text();
   for ( int axis : { QwtPlot::xBottom, QwtPlot::yLeft } )
   {
      const QwtScaleDiv div = plot->axisScaleDiv( axis );
      out << QString( "axis %1 %2 %3" ).arg( axis ).arg( div.lowerBound(), 0, 'g', 17 ).arg( div.upperBound(), 0, 'g', 17 );
   }
   for ( QwtPlotItem* item : plot->itemList() )
   {
      QString s = QString( "item %1 %2" ).arg( item->rtti() ).arg( item->title().text() );
      if ( auto* curve = dynamic_cast<QwtPlotCurve*>( item ) )
      {
         s += QString( " style %1 pen %2 %3" ).arg( curve->style() ).arg( curve->pen().color().name() ).arg( curve->pen().width() );
         for ( size_t i = 0; i < curve->dataSize(); i++ )
         {
            s += QString( " %1,%2" ).arg( curve->sample( i ).x(), 0, 'g', 17 ).arg( curve->sample( i ).y(), 0, 'g', 17 );
         }
      }
      if ( auto* grid = dynamic_cast<QwtPlotGrid*>( item ) )
      {
         s += QString( " ymin %1 y %2 pen %3 %4 %5" )
                 .arg( grid->yMinEnabled() )
                 .arg( grid->yEnabled() )
                 .arg( grid->majorPen().color().name() )
                 .arg( grid->majorPen().style() )
                 .arg( grid->majorPen().widthF() );
      }
      out << s;
   }
   return out.join( "\n" );
}

US_Model modelOf( int n, double shift )
{
   US_Model m;
   for ( int ii = 0; ii < n; ii++ )
   {
      US_Model::SimulationComponent sc;
      sc.s                    = ( 1.0 + ii * 0.7 + shift ) * 1e-13;
      sc.mw                   = 20000.0 + 15000.0 * ii - shift * 1e4;
      sc.D                    = ( 9.0 - ii * 0.4 ) * 1e-7;
      sc.signal_concentration = 0.05 + 0.02 * ( ( ii * 7 ) % 5 );
      m.components << sc;
   }
   return m;
}
} // namespace

class US_DistribStickTest : public QObject
{
      Q_OBJECT

   private slots:
      // Every type, for models with one and several components and with s below
      // zero, gives the plot both programs drew.
      void matchesBothOldPlots()
      {
         US_DataIO::EditedData edata;
         edata.runID      = "run";
         edata.cell       = "2";
         edata.wavelength = "280";
         int compared     = 0;
         for ( int n : { 1, 2, 7 } )
         {
            for ( double shift : { 0.0, -5.0 } )
            {
               for ( int type : { 0, 1, 2 } )
               {
                  FrozenFeMatchStick  fe;
                  FrozenReporterStick re;
                  US_Widgets          owner;
                  QwtPlot             p0, p1, p2;
                  fe.data_plot1 = &p1;
                  re.data_plot1 = &p2;
                  fe.edata = re.edata = &edata;
                  fe.model_used = re.model_used = modelOf( n, shift );
                  US_Model model                = modelOf( n, shift );

                  fe.distrib_plot_stick( type );
                  re.distrib_plot_stick( type );
                  US_DistribStick::plot( &owner, &p0, &edata, model, type );

                  const QString shared = describe( &p0 );
                  QCOMPARE( shared, describe( &p1 ) );
                  QCOMPARE( shared, describe( &p2 ) );
                  compared++;
               }
            }
         }
         QCOMPARE( compared, 18 );
      }
};

US3_GUI_TEST_MAIN( US_DistribStickTest )

#include "test_us_distrib_stick_gui.moc"
