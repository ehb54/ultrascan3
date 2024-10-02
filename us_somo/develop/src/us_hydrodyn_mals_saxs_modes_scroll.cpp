#include "../include/us3_defines.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_hydrodyn_mals_saxs.h"
#include "../include/us_hydrodyn_mals_saxs_scale_trend.h"
#include "../include/us_pm.h"
#include <QPixmap>

#define TSO QTextStream(stdout)

// --- scroll pairs ---

void US_Hydrodyn_Mals_Saxs::scroll_pair()
{
   TSO << "scroll_pairs()\n";
   le_last_focus = (mQLineEdit *) 0;

   // verify selected files and setup scroll_pair_names
   // must match time and one must end in _common
   // all istar or ihashq already verfied by _gui

   disable_all();

   scroll_pair_names        .clear();
   scroll_pair_org_selected .clear();
   scroll_pair_time_to_names.clear();
   scroll_pair_times        .clear();

   QStringList names = all_selected_files();

   // somewhat duplicated code in ::common_time() && ::join_by_time
   {
      map < vector < double >, QStringList > qgrid_to_names;

      for ( auto const & name : names ) {
         scroll_pair_org_selected.insert( name );

         if ( !f_qs.count( name ) || !f_Is.count( name ) ) {
            QMessageBox::critical( this,
                                   windowTitle() + us_tr( ": Scroll" ),
                                   QString( us_tr( "Internal error: %1 has no q-values!" ) ).arg( name )
                                   );
            return update_enables();
         }
         qgrid_to_names[ f_qs[ name ] ].push_back( name );
      }
      
      if ( qgrid_to_names.size() > 2 || !qgrid_to_names.size()) {
         QString detail = "\n";
         int pos = 0;
         for ( auto & it : qgrid_to_names ) {
            detail +=
               QString( us_tr( "grid %1 first (of %2) file name: %3\n\n" ) )
               .arg( ++pos )
               .arg( it.second.size() )
               .arg( it.second.front() )
               ;
         }         
         
         QMessageBox::critical( this,
                                windowTitle() + us_tr( ": Scroll" ),
                                QString( us_tr(
                                               "The %1 curves selected have more than two unique q-grids\n\n"
                                               "%2 q-grid(s) found\n"
                                               )
                                         )
                                .arg( names.size() )
                                .arg( qgrid_to_names.size() )
                                + detail
                                );
         return update_enables();
      }      

      if ( qgrid_to_names.size() == 1 ) {
         scroll_pair_names =
            {
               qgrid_to_names.begin()->second
            };
      } else {
         scroll_pair_names =
            {
               qgrid_to_names.begin()->second
               ,(--qgrid_to_names.end())->second
            };
      }
   }
   
   // get time grids for each

   {
      if ( scroll_pair_names.size() == 2 ) {
         vector < vector < double >> time_grids =
            {
               get_time_grid_from_namelist( scroll_pair_names[0] )
               ,get_time_grid_from_namelist( scroll_pair_names[1] )
            };

         if ( time_grids[0] != time_grids[1] ) {
            QMessageBox::critical( this,
                                   windowTitle() + us_tr( ": Scroll" ),
                                   QString( us_tr(
                                                  "The %1 curves selected do not have the same times\n\n"
                                                  )
                                            )
                                   .arg( names.size() )
                                   );
            return update_enables();
         }      

         scroll_pair_times = time_grids[0];
      } else {
         scroll_pair_times = get_time_grid_from_namelist( scroll_pair_names[0] );
      }
   }
   
   // build scroll_pair_time_to_names

   if ( scroll_pair_names.size() == 2 ) {
      // assume MALS has lower minq

      int mals_set = f_qs[ scroll_pair_names[0][0] ].front() < f_qs[ scroll_pair_names[1][0] ].front() ? 0 : 1;
      int saxs_set = 1 - mals_set;

      for ( int i = 0; i < (int) scroll_pair_times.size(); ++i ) {
         scroll_pair_time_to_names[ scroll_pair_times[i] ].push_back( scroll_pair_names[mals_set][i] );
         scroll_pair_time_to_names[ scroll_pair_times[i] ].push_back( scroll_pair_names[saxs_set][i] );
      }
   } else {
      for ( int i = 0; i < (int) scroll_pair_times.size(); ++i ) {
         scroll_pair_time_to_names[ scroll_pair_times[i] ].push_back( scroll_pair_names[0][i] );
      }
   }

   mode_select( MODE_SCROLL_PAIR );

   running       = true;

   // set wheel range etc.

   qwtw_wheel->setRange     ( 0, scroll_pair_times.size() - 1);
   qwtw_wheel->setSingleStep( 1 );
   qwtw_wheel->setValue     ( 0 );
   
   scroll_pair_scroll_highlight(0);
   scroll_pair_enables();
}

void US_Hydrodyn_Mals_Saxs::scroll_pair_enables()
{
   // mostly disable except cancel & wheel
   TSO << "scroll_pair_enables()\n";
   pb_wheel_cancel        -> setEnabled( true );
   pb_axis_x              -> setEnabled( true );
   pb_axis_y              -> setEnabled( true );
   cb_eb                  -> setEnabled( true );
   cb_dots                -> setEnabled( true );
   pb_color_rotate        -> setEnabled( true );
   pb_line_width          -> setEnabled( true );
   pb_q_exclude_vis       -> setEnabled( true );
   pb_q_exclude_left      -> setEnabled( true );
   pb_q_exclude_right     -> setEnabled( true );
   pb_q_exclude_clear     -> setEnabled( q_exclude.size() > 0 );
   
   wheel_enables( true );
}

void US_Hydrodyn_Mals_Saxs::scroll_pair_scroll_highlight( int pos )
{
   // show the plot
   // TSO << "scroll_pair_scroll_highlight( " <<  pos << " )\n";
   if ( pos >= (int) scroll_pair_times.size() ) {
      qDebug() << "error: scroll_pair_scroll_highlight( " <<  pos << " ) out of range, ignored!";
      return;
   }
   
   double time = scroll_pair_times[ pos ];
   lbl_wheel_pos->setText( QString( "%1" ).arg( time ) );

   set < QString > toplot;
   if ( scroll_pair_names.size() == 2 ) {
      toplot = {
         scroll_pair_time_to_names[ time ][ 0 ]
         ,scroll_pair_time_to_names[ time ][ 1 ]
      };
      lbl_wheel_pos_below->setText(
                                   QString( "<center>%1<br>%2</center>" )
                                   .arg( scroll_pair_time_to_names[ time ][ 0 ] )
                                   .arg( scroll_pair_time_to_names[ time ][ 1 ] )
                                   );
   } else {
      toplot = {
         scroll_pair_time_to_names[ time ][ 0 ]
      };
      lbl_wheel_pos_below->setText(
                                   QString( "<center>%1</center>" )
                                   .arg( scroll_pair_time_to_names[ time ][ 0 ] )
                                   );
   }

   set_selected( toplot );
}
