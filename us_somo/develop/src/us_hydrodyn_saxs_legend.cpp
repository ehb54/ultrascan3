#include "../include/us_hydrodyn_saxs.h"
#include <qwt_legend.h>
#include <qwt_text.h>
#include <QFrame>

void US_Hydrodyn_Saxs::plot_saxs_clicked( long 
                                          )
{
}

void US_Hydrodyn_Saxs::plot_saxs_item_clicked(
                                              const QVariant & iteminfo,
                                              int /* index */
                                               )
{
   QwtPlotCurve* pcurve = (QwtPlotCurve *)plot_saxs->infoToItem( iteminfo );
   int csize = pcurve->dataSize();

   if ( csize < 1 )
   {
      editor_msg( "red", "Internal error: plot_saxs: curve not found\n" );
      return;
   }

   editor_msg( "black", 
               QString( us_tr( "Curve information: " 
                            "Name: %1\n"
                            "q: [%2:%3] [%4:%5] %6 points" ) )
               .arg( pcurve->title().text() )
               .arg( pcurve->minXValue() ).arg( pcurve->maxXValue() )
               .arg( pcurve->minYValue() ).arg( pcurve->maxYValue() )
               .arg( csize ) );
}

void US_Hydrodyn_Saxs::plot_pr_clicked( long
                                        )
{
}

void US_Hydrodyn_Saxs::plot_pr_item_clicked( 
                                            const QVariant & iteminfo,
                                            int pitem
                                             )
{
   QwtPlotCurve* pcurve = (QwtPlotCurve *)plot_pr->infoToItem( iteminfo );

   int csize = pcurve->dataSize();

   if ( csize < 1 )
   {
      editor_msg( "red", "Internal error: plot_pr: curve not found\n" );
      return;
   }

   editor_msg( "black", 
               QString( us_tr( "Curve information: " 
                               "Name: %1\n"
                               "q: [%2:%3] [%4:%5] %6 points" ) )
               .arg( pcurve->title().text() )
               .arg( pcurve->minXValue() ).arg( pcurve->maxXValue() )
               .arg( pcurve->minYValue() ).arg( pcurve->maxYValue() )
               .arg( csize ) );
}

void US_Hydrodyn_Saxs::saxs_legend()
{
   saxs_legend_vis = !saxs_legend_vis;
   set_saxs_legend();
}

void US_Hydrodyn_Saxs::set_saxs_legend()
{
   if ( saxs_legend_vis )
   {
      QwtLegend* legend_saxs = new QwtLegend;
      legend_saxs->setDefaultItemMode( QwtLegendData::Clickable );
      legend_saxs->setFrameStyle( QFrame::Box | QFrame::Sunken );
      plot_saxs->insertLegend( legend_saxs, QwtPlot::BottomLegend );
      ((QwtLegend *)plot_saxs->legend())->setDefaultItemMode( QwtLegendData::Clickable );
      connect( (QwtLegend *)plot_saxs->legend(), SIGNAL( clicked( const QVariant &, int ) ),
               SLOT( plot_pr_item_clicked( const QVariant &, int ) ) );
   } else {
      plot_saxs->insertLegend( NULL );
   }
}

void US_Hydrodyn_Saxs::pr_legend()
{
   pr_legend_vis = !pr_legend_vis;
   set_pr_legend();
}

void US_Hydrodyn_Saxs::set_pr_legend()
{
   if ( pr_legend_vis )
   {
      QwtLegend* legend_pr = new QwtLegend;
      legend_pr->setDefaultItemMode( QwtLegendData::Clickable );
      legend_pr->setFrameStyle( QFrame::Box | QFrame::Sunken );
      plot_pr->insertLegend( legend_pr, QwtPlot::BottomLegend );
      ((QwtLegend *)plot_pr->legend())->setDefaultItemMode( QwtLegendData::Clickable );
      connect( (QwtLegend *)plot_pr->legend(), SIGNAL( clicked( const QVariant &, int ) ),
               SLOT( plot_pr_item_clicked( const QVariant &, int ) ) );
   } else {
      plot_pr->insertLegend( NULL );
   }
}
