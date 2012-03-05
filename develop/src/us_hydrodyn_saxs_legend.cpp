#include "../include/us_hydrodyn_saxs.h"

// note: this program uses cout and/or cerr and this should be replaced

void US_Hydrodyn_Saxs::plot_saxs_clicked( long key )
{
#ifndef QT4
   bool found = false;
   unsigned int pos;
   for ( unsigned int i = 0; i < plotted_Iq.size(); i++ )
   {
      if ( key == plotted_Iq[ i ] )
      {
         found = true;
         pos   = i;
      }
   }
   if ( !found )
   {
      editor_msg( "red", "Internal error: plot_saxs_clicked: curve not found\n" );
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
      if ( plot_saxs_zoomer )
      {
         double       minx            = plot_saxs_zoomer->zoomRect().x1();
         double       maxx            = plot_saxs_zoomer->zoomRect().x2();
         double       avg_std_dev_pct = 0e0;
         unsigned int count           = 0;

         for ( unsigned int i = 0; i < plotted_I_error[ pos ].size(); i++ )
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
#else
   editor_msg( "red", tr( "Legend controls not yet supported in US3" ) );
#endif
}

void US_Hydrodyn_Saxs::plot_pr_clicked( long key )
{
   cout << QString( "plot_pr_clicked %1\n" ).arg( key );
}

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
#endif
}
