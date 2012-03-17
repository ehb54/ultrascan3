#include "../include/us_hydrodyn_saxs.h"
#include <qwt_legend.h>

#ifndef QT4
void US_Hydrodyn_Saxs::plot_saxs_clicked( long key )
{
   int pos = -1;

   for ( int i = 0; i < (int)plotted_Iq.size(); i++ )
   {
      if ( key == plotted_Iq[ i ] )
      {
         pos   = i;
      }
   }
   if ( pos < 0 )
   {
      editor_msg( "red",
            tr( "Internal error: plot_saxs_clicked: curve not found\n" ) );
      return;
   }
   editor_msg( "black", 
               QString( tr( "Curve information: " 
                            "Name: %1\n"
                            "q: [%2:%3] %4 points" ) )
               .arg( qsl_plotted_iq_names[ pos ] )
               .arg( plotted_q[ pos ][ 0 ] )
               .arg( plotted_q[ pos ].back() )
               .arg( plotted_q[ pos ].size() ) );
   if ( is_nonzero_vector( plotted_I_error[ pos ] ) )
   {
      {
         double avg_std_dev_pct = 0e0;
         for ( unsigned int i = 0; i < plotted_I_error[ pos ].size(); i++ )
         {
            avg_std_dev_pct += 100.0 * plotted_I_error[ pos ][ i ] / plotted_I[ pos ][ i ];
         }
         avg_std_dev_pct /= (double) plotted_I_error[ pos ].size();
         editor_msg( "black", 
                     QString( tr( "Errors present %1 points, s.d. average %2 %" ) )
                     .arg( plotted_I_error[ pos ].size() )
                     .arg( avg_std_dev_pct ) );
      }
      if ( plot_saxs_zoomer != 0 )
      {
         double       minx            = plot_saxs_zoomer->zoomRect().x1();
         double       maxx            = plot_saxs_zoomer->zoomRect().x2();
         double       avg_std_dev_pct = 0e0;
         int          count           = 0;

         for ( int i = 0; i < (int)plotted_I_error[ pos ].size(); i++ )
         {
            if ( plotted_q[ pos ][ i ] >= minx &&
                 plotted_q[ pos ][ i ] <= maxx )
            {
               avg_std_dev_pct += 100.0 * plotted_I_error[ pos ][ i ] / plotted_I[ pos ][ i ];
               count++;
            }
         }
         avg_std_dev_pct /= (double) count;
         editor_msg( "black", 
                     QString( tr( "Currently visible %1 points in (%2:%3), s.d. average %4 %" ) )
                     .arg( count )
                     .arg( minx )
                     .arg( maxx )
                     .arg( avg_std_dev_pct ) );
      }
   } else {
      editor_msg( "black", tr( "No errors present" ) );
   }
}
#else
void US_Hydrodyn_Saxs::plot_saxs_clicked( QwtPlotItem* pitem )
{
   QwtPlotCurve* pcurve = (QwtPlotCurve*)pitem;
   int csize = pcurve->dataSize();

   if ( csize < 1 )
   {
      editor_msg( "red", "Internal error: plot_saxs: curve not found\n" );
      return;
   }

   editor_msg( "black", 
               QString( tr( "Curve information: " 
                            "Name: %1\n"
                            "q: [%2:%3] [%4:%5] %6 points" ) )
               .arg( pcurve->title().text() )
               .arg( pcurve->minXValue() ).arg( pcurve->maxXValue() )
               .arg( pcurve->minYValue() ).arg( pcurve->maxYValue() )
               .arg( csize ) );
}
#endif

#ifndef QT4
void US_Hydrodyn_Saxs::plot_pr_clicked( long key )
{
   cout << QString( "plot_pr_clicked %1\n" ).arg( key );
}
#else
void US_Hydrodyn_Saxs::plot_pr_clicked( QwtPlotItem* pitem )
{
   QwtPlotCurve* pcurve = (QwtPlotCurve*)pitem;
   int csize = pcurve->dataSize();

   if ( csize < 1 )
   {
      editor_msg( "red", "Internal error: plot_pr: curve not found\n" );
      return;
   }

   editor_msg( "black", 
               QString( tr( "Curve information: " 
                            "Name: %1\n"
                            "q: [%2:%3] [%4:%5] %6 points" ) )
               .arg( pcurve->title().text() )
               .arg( pcurve->minXValue() ).arg( pcurve->maxXValue() )
               .arg( pcurve->minYValue() ).arg( pcurve->maxYValue() )
               .arg( csize ) );
}
#endif

void US_Hydrodyn_Saxs::saxs_legend()
{
#ifndef QT4
   if ( plot_saxs->autoLegend() )
   {
      plot_saxs->setAutoLegend( false );
      plot_saxs->enableLegend ( false, -1 );
   } else {
      plot_saxs->setAutoLegend( true );
      plot_saxs->enableLegend ( true, -1 );
   }
#else
   bool legvi = true;
   QwtPlotItemList ilist = plot_saxs->itemList();
   for ( int ii = 0; ii < ilist.size(); ii++ )
   {
      QwtPlotItem* plitem = ilist[ ii ];
      if ( plitem->rtti() != QwtPlotItem::Rtti_PlotCurve )
         continue;
      bool legon = plitem->testItemAttribute( QwtPlotItem::Legend );
      plitem->setItemAttribute( QwtPlotItem::Legend, !legon );
      legvi = !legon;
   }
   plot_saxs->legend()->setVisible( legvi );
#endif
}

void US_Hydrodyn_Saxs::pr_legend()
{
#ifndef QT4
   if ( plot_pr->autoLegend() )
   {
      plot_pr->setAutoLegend( false );
      plot_pr->enableLegend ( false, -1 );
   } else {
      plot_pr->setAutoLegend( true );
      plot_pr->enableLegend ( true, -1 );
   }
#else
   bool legvi = true;
   QwtPlotItemList ilist = plot_pr->itemList();
   for ( int ii = 0; ii < ilist.size(); ii++ )
   {
      QwtPlotItem* plitem = ilist[ ii ];
      if ( plitem->rtti() != QwtPlotItem::Rtti_PlotCurve )
         continue;
      bool legon = plitem->testItemAttribute( QwtPlotItem::Legend );
      plitem->setItemAttribute( QwtPlotItem::Legend, !legon );
      legvi = !legon;
   }
   plot_pr->legend()->setVisible( legvi );
#endif
}
