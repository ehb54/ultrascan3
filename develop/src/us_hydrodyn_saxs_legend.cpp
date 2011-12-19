#include "../include/us_hydrodyn_saxs.h"

void US_Hydrodyn_Saxs::plot_saxs_clicked( long key )
{
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
      double avg_std_dev_pct = 0e0;
      for ( unsigned int i = 0; i < plotted_I_error[ pos ].size(); i++ )
      {
         avg_std_dev_pct += 100.0 * plotted_I_error[ pos ][ i ] / plotted_I[ pos ][ i ];
      }
      avg_std_dev_pct /= (double) plotted_I_error[ pos ].size();
      editor_msg( "black", 
                  QString( tr( "Errors present %1 points average % error %2" ) )
                  .arg( plotted_I_error[ pos ].size() )
                  .arg( avg_std_dev_pct ) );
   } else {
      editor_msg( "black", tr( "No errors present" ) );
   }
}

void US_Hydrodyn_Saxs::plot_pr_clicked( long key )
{
   cout << QString( "plot_pr_clicked %1\n" ).arg( key );
}

void US_Hydrodyn_Saxs::saxs_legend()
{
   if ( plot_saxs->autoLegend() )
   {
      plot_saxs->setAutoLegend( false );
      plot_saxs->enableLegend ( false, -1 );
   } else {
      plot_saxs->setAutoLegend( true );
      plot_saxs->enableLegend ( true, -1 );
   }
}

void US_Hydrodyn_Saxs::pr_legend()
{
   if ( plot_pr->autoLegend() )
   {
      plot_pr->setAutoLegend( false );
      plot_pr->enableLegend ( false, -1 );
   } else {
      plot_pr->setAutoLegend( true );
      plot_pr->enableLegend ( true, -1 );
   }
}
