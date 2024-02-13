#include "../include/us3_defines.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_hydrodyn_mals_saxs.h"
#include "../include/us_hydrodyn_mals_saxs_scale_trend.h"
#include "../include/us_pm.h"
#include <QPixmap>

#define TSO QTextStream(stdout)

// --- scale pairs ---

void US_Hydrodyn_Mals_Saxs::scale_pair()
{
   TSO << "scale_pairs()\n";
   le_last_focus = (mQLineEdit *) 0;

   // verify selected files and setup scale_pair_names
   // must match time and one must end in _common
   // all istar or ihashq already verfied by _gui

   disable_all();

   scale_pair_names        .clear();
   scale_pair_org_selected .clear();
   scale_pair_time_to_names.clear();
   scale_pair_times        .clear();
   scale_pair_qgrids       .clear();
   scale_pair_save_names.clear();

   QStringList names = all_selected_files();
   scale_pair_original_scale    = le_scale_pair_scale   ->text().toDouble();
   scale_pair_original_sd_scale = le_scale_pair_sd_scale->text().toDouble();

   // somewhat duplicated code in ::common_time() && ::join_by_time
   {
      map < vector < double >, QStringList > qgrid_to_names;

      for ( auto const & name : names ) {
         scale_pair_org_selected.insert( name );

         if ( !f_qs.count( name ) || !f_Is.count( name ) ) {
            QMessageBox::critical( this,
                                   windowTitle() + us_tr( ": Scale" ),
                                   QString( us_tr( "Internal error: %1 has no q-values!" ) ).arg( name )
                                   );
            return update_enables();
         }
         qgrid_to_names[ f_qs[ name ] ].push_back( name );
      }
      
      if ( qgrid_to_names.size() != 2 ) {
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
                                windowTitle() + us_tr( ": Scale" ),
                                QString( us_tr(
                                               "The %1 curves selected do not have exactly two unique q-grids\n\n"
                                               "%2 q-grid(s) found\n"
                                               )
                                         )
                                .arg( names.size() )
                                .arg( qgrid_to_names.size() )
                                + detail
                                );
         return update_enables();
      }      

      scale_pair_names =
         {
            qgrid_to_names.begin()->second
            ,(--qgrid_to_names.end())->second
         };

      scale_pair_qgrids.push_back( qgrid_to_names.begin()->first );
      scale_pair_qgrids.push_back( (--qgrid_to_names.end())->first );
   }
   
   // get time grids for each

   {
      vector < vector < double >> time_grids =
         {
            get_time_grid_from_namelist( scale_pair_names[0] )
            ,get_time_grid_from_namelist( scale_pair_names[1] )
         };

      if ( time_grids[0] != time_grids[1] ) {
         QMessageBox::critical( this,
                                windowTitle() + us_tr( ": Scale" ),
                                QString( us_tr(
                                               "The %1 curves selected do not have the same times\n\n"
                                               )
                                         )
                                .arg( names.size() )
                                );
         return update_enables();
      }      

      scale_pair_times = time_grids[0];
   }
   
   // build scale_pair_time_to_names

   scale_pair_mals_set = f_qs[ scale_pair_names[0][0] ].front() < f_qs[ scale_pair_names[1][0] ].front() ? 0 : 1;
   scale_pair_saxs_set = 1 - scale_pair_mals_set;

   for ( int i = 0; i < (int) scale_pair_times.size(); ++i ) {
      scale_pair_time_to_names[ scale_pair_times[i] ].push_back( scale_pair_names[scale_pair_mals_set][i] );
      scale_pair_time_to_names[ scale_pair_times[i] ].push_back( scale_pair_names[scale_pair_saxs_set][i] );
   }

   // using errors?

   scale_pair_use_errors = true;
   {
      QString missing_errors_name;
      
      for ( auto const & name : scale_pair_names[ scale_pair_mals_set ] ) {
         if ( !f_errors.count( name )
              || f_errors[ name ].size() != scale_pair_qgrids[ scale_pair_mals_set ].size()
              || is_zero_vector( f_errors[ name ] ) ) {
            scale_pair_use_errors = false;
            missing_errors_name = name;
         }
      }

      if ( scale_pair_use_errors ) {
         for ( auto const & name : scale_pair_names[ scale_pair_saxs_set ] ) {
            if ( !f_errors.count( name )
                 || f_errors[ name ].size() != scale_pair_qgrids[ scale_pair_saxs_set ].size()
                 || is_zero_vector( f_errors[ name ] ) ) {
               scale_pair_use_errors = false;
               missing_errors_name = name;
            }
         }
      }

      if ( !scale_pair_use_errors ) {
         QMessageBox::warning( this,
                               windowTitle() + us_tr( ": Scale Fit" ),
                               QString( us_tr( "File(s) without SDs defined or having zero value SDs found\n"
                                               "First found in:\n%1\n"
                                               "Errors will be disabled for fitting"
                                               ) )
                               .arg( missing_errors_name )
                               );
      }         
   }

   scale_pair_created_init();

   mode_select( MODE_SCALE_PAIR );

   running       = true;

   // set wheel range etc.

   scale_pair_time_pos      = 0;

   qwtw_wheel->setRange     ( 0, scale_pair_times.size() - 1 );
   qwtw_wheel->setSingleStep( 1 );
   qwtw_wheel->setValue     ( scale_pair_time_pos );
   
   // set le_scale_pair_q1,2 start,end to grid limits
   scale_pair_markers.clear();

   scale_pair_add_marker( plot_dist, le_scale_pair_q1_start  ->text().toDouble(), scale_pair_color_q1, us_tr( "Start"), Qt::AlignRight | Qt::AlignBottom );
   scale_pair_add_marker( plot_dist, le_scale_pair_q1_end    ->text().toDouble(), scale_pair_color_q1, us_tr( "End"  ), Qt::AlignLeft  | Qt::AlignBottom );
   scale_pair_add_marker( plot_dist, le_scale_pair_q2_start  ->text().toDouble(), scale_pair_color_q2, us_tr( "Start"), Qt::AlignRight | Qt::AlignBottom );
   scale_pair_add_marker( plot_dist, le_scale_pair_q2_end    ->text().toDouble(), scale_pair_color_q2, us_tr( "End"  ), Qt::AlignLeft  | Qt::AlignBottom );

   scale_pair_q1_min = f_qs[ scale_pair_names[scale_pair_mals_set][0] ].front();
   scale_pair_q1_max = f_qs[ scale_pair_names[scale_pair_mals_set][0] ].back();
   scale_pair_q2_min = f_qs[ scale_pair_names[scale_pair_saxs_set][0] ].front();
   scale_pair_q2_max = f_qs[ scale_pair_names[scale_pair_saxs_set][0] ].back();

   le_scale_pair_q1_start->setText( QString( "%1" ).arg( scale_pair_q1_min ) );
   le_scale_pair_q1_end  ->setText( QString( "%1" ).arg( scale_pair_q1_max ) );
   le_scale_pair_q2_start->setText( QString( "%1" ).arg( scale_pair_q2_min ) );
   le_scale_pair_q2_end  ->setText( QString( "%1" ).arg( scale_pair_q2_max ) );

   scale_pair_scroll_highlight(0);
   lbl_wheel_pos->setText("");
   scale_pair_enables();
}

void US_Hydrodyn_Mals_Saxs::scale_pair_enables()
{
   // mostly disable except cancel & wheel
   TSO << "scale_pair_enables()\n";
   qwtw_wheel                           ->setEnabled( true );
   pb_wheel_cancel                      ->setEnabled( true );
   pb_wheel_save                        ->setEnabled( le_scale_pair_scale->text().toDouble() != scale_pair_original_scale ||
                                                      le_scale_pair_sd_scale->text().toDouble() != scale_pair_original_sd_scale );
   pb_axis_x                            ->setEnabled( true );
   pb_axis_y                            ->setEnabled( true );
   le_scale_pair_time                   ->setEnabled( true );
   cb_eb                                ->setEnabled( true );
   cb_dots                              ->setEnabled( true );
   pb_color_rotate                      ->setEnabled( true );
   pb_line_width                        ->setEnabled( true );
   pb_q_exclude_vis                     ->setEnabled( true );
   pb_q_exclude_left                    ->setEnabled( true );
   pb_q_exclude_right                   ->setEnabled( true );
   pb_q_exclude_clear                   ->setEnabled( q_exclude.size() > 0 );
   
   pb_scale_pair_fit                    ->setEnabled( true );
   pb_scale_pair_minimize               ->setEnabled( true );
   pb_scale_pair_create_scaled_curves   ->setEnabled( ( le_scale_pair_scale->text() != "1" ||
                                                        ( scale_pair_use_errors && le_scale_pair_sd_scale->text() != "1" ) )
                                                      && le_scale_pair_scale->text().toDouble() > 0
                                                      && le_scale_pair_sd_scale->text().toDouble() > 0
                                                      );

   pb_scale_pair_reset                  ->setEnabled( le_scale_pair_scale->text() != "1" ||
                                                      le_scale_pair_sd_scale->text() != "1" );
   rb_scale_pair_fit_method_p2          ->setEnabled( true );
   rb_scale_pair_fit_method_p3          ->setEnabled( true );
   rb_scale_pair_fit_method_p4          ->setEnabled( true );
   le_scale_pair_q1_start               ->setEnabled( true );
   le_scale_pair_q1_end                 ->setEnabled( true );
   le_scale_pair_q2_start               ->setEnabled( true );
   le_scale_pair_q2_end                 ->setEnabled( true );
   le_scale_pair_scale                  ->setEnabled( true );
   le_scale_pair_sd_scale               ->setEnabled( scale_pair_use_errors );

   wheel_enables( true );
}

void US_Hydrodyn_Mals_Saxs::scale_pair_scroll_highlight( int pos )
{
   // show the plot
   qDebug() << "scale_pair_scroll_highlight( " <<  pos << " )";
   if ( pos >= (int) scale_pair_times.size() ) {
      qDebug() << "error: scale_pair_scroll_highlight( " <<  pos << " ) out of range, ignored!";
      return;
   }
   
   scale_pair_time_pos = pos;

   double time = scale_pair_times[ pos ];
   le_scale_pair_time->setText( QString( "%1" ).arg( time ) );

   set < QString > toplot;
   toplot = {
      scale_pair_created_time_to_name[ time ] // always mals_set
      // scale_pair_time_to_names[ time ][ scale_pair_mals_set ] (previously unscaled)
      ,scale_pair_time_to_names[ time ][ scale_pair_saxs_set ]
   };

   set_selected( toplot );
}

void US_Hydrodyn_Mals_Saxs::scale_pair_add_marker( 
                                                  QwtPlot * plot,
                                                  double pos, 
                                                  QColor color, 
                                                  QString text, 
                                                  Qt::Alignment
                                                  align ) {
   QwtPlotMarker * marker = new QwtPlotMarker;
   marker->setLineStyle        ( QwtPlotMarker::VLine );
   marker->setLinePen          ( QPen( color, 2, Qt::DashDotDotLine ) );
   marker->setLabelOrientation ( Qt::Horizontal );
   marker->setXValue           ( pos );
   marker->setLabelAlignment   ( align );
   {
      QwtText qwtt             ( text );
      qwtt.setFont             ( QFont("Helvetica", 11, QFont::Bold ) );
      marker->setLabel         ( qwtt );
   }
   marker->attach              ( plot );
   scale_pair_markers.push_back( marker );
}   

void US_Hydrodyn_Mals_Saxs::scale_pair_delete_markers() {
   plot_dist   ->detachItems( QwtPlotItem::Rtti_PlotMarker );
   scale_pair_markers.clear();
}

void US_Hydrodyn_Mals_Saxs::scale_pair_set_fit_method_p2() {
   qDebug() << "scale_pair_set_fit_method_p2()";
   scale_pair_current_fit_method = SCALE_PAIR_FIT_METHOD_P2;
}

void US_Hydrodyn_Mals_Saxs::scale_pair_set_fit_method_p3() {
   qDebug() << "scale_pair_set_fit_method_p3()";
   scale_pair_current_fit_method = SCALE_PAIR_FIT_METHOD_P3;
}

void US_Hydrodyn_Mals_Saxs::scale_pair_set_fit_method_p4() {
   qDebug() << "scale_pair_set_fit_method_p3()";
   scale_pair_current_fit_method = SCALE_PAIR_FIT_METHOD_P4;
}

#define UHMS_SCALE_WHEEL_RES 10000
// #define UHMS_SCALE_WHEEL_RES 1e-6

void US_Hydrodyn_Mals_Saxs::scale_pair_q1_start_text( const QString & text ) {
   TSO << QString( "scale_pair_q1_start_text( %1 )\n" ).arg( text );
   if ( current_mode != MODE_SCALE_PAIR ) {
      return;
   }
   if ( scale_pair_markers.size() != 4 ) {
      editor_msg( "red", QString( "internal error: scale_pair_q1_start_text markers issue size %1" ).arg( scale_pair_markers.size() ) );
      return;
   }
   
   lbl_wheel_pos->setText( text );
   scale_pair_markers[ 0 ]->setXValue( text.toDouble() );
   plot_dist->replot();

   if ( !wheel_is_pressed && qwtw_wheel->value() != text.toDouble() ) {
      qwtw_wheel->setValue( text.toDouble() );
   }

   disconnect( le_scale_pair_q1_start, SIGNAL( textChanged( const QString & ) ), 0, 0 );
   le_scale_pair_q1_start->setText( QString( "%1" ).arg( text.toDouble() ) );
   connect   ( le_scale_pair_q1_start, SIGNAL( textChanged( const QString & ) ), SLOT( scale_pair_q1_start_text( const QString & ) ) );

   if ( text.toDouble() > le_scale_pair_q1_end->text().toDouble() ) {
      le_scale_pair_q1_end->setText( text );
   } else {
      TSO << QString( "scale_pair_q1_start_text() calls any analysis etc()\n" );
   }
}

void US_Hydrodyn_Mals_Saxs::scale_pair_q1_end_text( const QString & text ) {
   // TSO << QString( "scale_pair_q1_end_text( %1 )\n" ).arg( text );
   if ( current_mode != MODE_SCALE_PAIR ) {
      return;
   }
   if ( scale_pair_markers.size() != 4 ) {
      editor_msg( "red", QString( "internal error: scale_pair_q1_end_text markers issue size %1" ).arg( scale_pair_markers.size() ) );
      return;
   }
   lbl_wheel_pos->setText( text );
   scale_pair_markers[ 1 ]->setXValue( text.toDouble() );
   plot_dist->replot();

   if ( !wheel_is_pressed && qwtw_wheel->value() != text.toDouble() ) {
      qwtw_wheel->setValue( text.toDouble() );
   }

   disconnect( le_scale_pair_q1_end, SIGNAL( textChanged( const QString & ) ), 0, 0 );
   le_scale_pair_q1_end->setText( QString( "%1" ).arg( text.toDouble() ) );
   connect   ( le_scale_pair_q1_end, SIGNAL( textChanged( const QString & ) ), SLOT( scale_pair_q1_end_text( const QString & ) ) );

   if ( text.toDouble() < le_scale_pair_q1_start->text().toDouble() ) {
      le_scale_pair_q1_start->setText( text );
   } else {
      TSO << QString( "scale_pair_q1_end_text() calls any analysis etc\n" );
   }
}

void US_Hydrodyn_Mals_Saxs::scale_pair_q1_start_focus( bool hasFocus ) {
   qDebug() << "scale_pair_q1_start_focus()";
   if ( current_mode != MODE_SCALE_PAIR ) {
      return;
   }
   if ( hasFocus ) {
      le_last_focus = le_scale_pair_q1_start;
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      qwtw_wheel->setRange( scale_pair_q1_min, scale_pair_q1_max);
      qwtw_wheel->setSingleStep( ( scale_pair_q1_max - scale_pair_q1_min ) / UHMS_SCALE_WHEEL_RES );
      // qwtw_wheel->setSingleStep( UHMS_SCALE_WHEEL_RES );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setValue( le_scale_pair_q1_start->text().toDouble() );
      wheel_enables();
   }
}

void US_Hydrodyn_Mals_Saxs::scale_pair_q1_end_focus( bool hasFocus )
{
   qDebug() << "scale_pair_q1_end_focus()";
   if ( current_mode != MODE_SCALE_PAIR ) {
      return;
   }
   if ( hasFocus )
   {
      le_last_focus = le_scale_pair_q1_end;
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      qwtw_wheel->setRange( scale_pair_q1_min, scale_pair_q1_max);
      qwtw_wheel->setSingleStep( ( scale_pair_q1_max - scale_pair_q1_min ) / UHMS_SCALE_WHEEL_RES );
      // qwtw_wheel->setSingleStep( UHMS_SCALE_WHEEL_RES );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setValue( le_scale_pair_q1_end->text().toDouble() );
      wheel_enables();
   }
}

void US_Hydrodyn_Mals_Saxs::scale_pair_q2_start_text( const QString & text ) {
   TSO << QString( "scale_pair_q2_start_text( %1 )\n" ).arg( text );
   if ( current_mode != MODE_SCALE_PAIR ) {
      return;
   }
   if ( scale_pair_markers.size() != 4 ) {
      editor_msg( "red", QString( "internal error: scale_pair_q2_start_text markers issue size %1" ).arg( scale_pair_markers.size() ) );
      return;
   }
   
   lbl_wheel_pos->setText( text );
   scale_pair_markers[ 2 ]->setXValue( text.toDouble() );
   plot_dist->replot();

   if ( !wheel_is_pressed && qwtw_wheel->value() != text.toDouble() ) {
      qwtw_wheel->setValue( text.toDouble() );
   }

   disconnect( le_scale_pair_q2_start, SIGNAL( textChanged( const QString & ) ), 0, 0 );
   le_scale_pair_q2_start->setText( QString( "%1" ).arg( text.toDouble() ) );
   connect   ( le_scale_pair_q2_start, SIGNAL( textChanged( const QString & ) ), SLOT( scale_pair_q2_start_text( const QString & ) ) );

   if ( text.toDouble() > le_scale_pair_q2_end->text().toDouble() ) {
      le_scale_pair_q2_end->setText( text );
   } else {
      TSO << QString( "scale_pair_q2_start_text() calls any analysis etc()\n" );
   }
}

void US_Hydrodyn_Mals_Saxs::scale_pair_q2_end_text( const QString & text ) {
   // TSO << QString( "scale_pair_q2_end_text( %1 )\n" ).arg( text );
   if ( current_mode != MODE_SCALE_PAIR ) {
      return;
   }
   if ( scale_pair_markers.size() != 4 ) {
      editor_msg( "red", QString( "internal error: scale_pair_q2_end_text markers issue size %1" ).arg( scale_pair_markers.size() ) );
      return;
   }
   lbl_wheel_pos->setText( text );
   scale_pair_markers[ 3 ]->setXValue( text.toDouble() );
   plot_dist->replot();

   if ( !wheel_is_pressed && qwtw_wheel->value() != text.toDouble() ) {
      qwtw_wheel->setValue( text.toDouble() );
   }

   disconnect( le_scale_pair_q2_end, SIGNAL( textChanged( const QString & ) ), 0, 0 );
   le_scale_pair_q2_end->setText( QString( "%1" ).arg( text.toDouble() ) );
   connect   ( le_scale_pair_q2_end, SIGNAL( textChanged( const QString & ) ), SLOT( scale_pair_q2_end_text( const QString & ) ) );

   if ( text.toDouble() < le_scale_pair_q2_start->text().toDouble() ) {
      le_scale_pair_q2_start->setText( text );
   } else {
      TSO << QString( "scale_pair_q2_end_text() calls any analysis etc\n" );
   }
}

void US_Hydrodyn_Mals_Saxs::scale_pair_q2_start_focus( bool hasFocus ) {
   qDebug() << "scale_pair_q2_start_focus()";
   if ( current_mode != MODE_SCALE_PAIR ) {
      return;
   }
   if ( hasFocus ) {
      le_last_focus = le_scale_pair_q2_start;
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      qwtw_wheel->setRange( scale_pair_q2_min, scale_pair_q2_max);
      qwtw_wheel->setSingleStep( ( scale_pair_q2_max - scale_pair_q2_min ) / UHMS_SCALE_WHEEL_RES );
      // qwtw_wheel->setSingleStep( UHMS_SCALE_WHEEL_RES );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setValue( le_scale_pair_q2_start->text().toDouble() );
      wheel_enables();
   }
}

void US_Hydrodyn_Mals_Saxs::scale_pair_q2_end_focus( bool hasFocus )
{
   qDebug() << "scale_pair_q2_end_focus()";
   if ( current_mode != MODE_SCALE_PAIR ) {
      return;
   }
   if ( hasFocus )
   {
      le_last_focus = le_scale_pair_q2_end;
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      qwtw_wheel->setRange( scale_pair_q2_min, scale_pair_q2_max);
      qwtw_wheel->setSingleStep( ( scale_pair_q2_max - scale_pair_q2_min ) / UHMS_SCALE_WHEEL_RES );
      // qwtw_wheel->setSingleStep( UHMS_SCALE_WHEEL_RES );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setValue( le_scale_pair_q2_end->text().toDouble() );
      wheel_enables();
   }
}

void US_Hydrodyn_Mals_Saxs::scale_pair_time_focus( bool hasFocus ) {
   qDebug() << "scale_pair_time_focus";
   if ( current_mode != MODE_SCALE_PAIR ) {
      return;
   }
   if ( hasFocus ) {
      qDebug() << "scale_pair_time_focus : hasFocus";
      le_last_focus = (mQLineEdit *)0;
      qwtw_wheel->setValue     ( scale_pair_time_pos );
      qwtw_wheel->setRange     ( 0, scale_pair_times.size() - 1 );
      qwtw_wheel->setSingleStep( 1 );
      lbl_wheel_pos->setText   ( "" );
   }
}

void US_Hydrodyn_Mals_Saxs::scale_pair_minimize() {
   qDebug() << "scale_pair_minimize()";
}

void US_Hydrodyn_Mals_Saxs::scale_pair_fit() {
   qDebug() << "scale_pair_fit()";
   disable_all();
#warning probably should be in ::disable_all() but for some reason we commented it out there, probably should check all wheel usage and set as appropriate

   qwtw_wheel            ->setEnabled( false );

   // collect q grid positions

   set < double > q1;
   set < double > q2;
   set < double > qs;

   double q1_min = le_scale_pair_q1_start->text().toDouble();
   double q1_max = le_scale_pair_q1_end  ->text().toDouble();
   double q2_min = le_scale_pair_q2_start->text().toDouble();
   double q2_max = le_scale_pair_q2_end  ->text().toDouble();

   for ( auto const & q : scale_pair_qgrids[scale_pair_mals_set] ) {
      if ( q >= q1_min && q <= q1_max && !q_exclude.count( q ) ) {
         q1.insert( q );
         qs.insert( q );
      }
   }
   
   for ( auto const & q : scale_pair_qgrids[scale_pair_saxs_set] ) {
      if ( q >= q2_min && q <= q2_max && !q_exclude.count( q ) ) {
         q2.insert( q );
         qs.insert( q );
      }
   }
   
   if ( !qs.size() ) {
      QMessageBox::critical( this,
                             windowTitle() + us_tr( ": Scale Fit" ),
                             us_tr(
                                   "There are no non-excluded points selected for fitting!"
                                   )
                             );
      return scale_pair_enables();
   }

   {
      int minimum_pts_req = 2;
      switch ( scale_pair_current_fit_method ) {
      case SCALE_PAIR_FIT_METHOD_P2 :
         minimum_pts_req = 3;
         break;
      case SCALE_PAIR_FIT_METHOD_P3 :
         minimum_pts_req = 4;
         break;
      case SCALE_PAIR_FIT_METHOD_P4 :
         minimum_pts_req = 5;
         break;
      default:
         QMessageBox::critical( this,
                                windowTitle() + us_tr( ": Scale Fit" ),
                                us_tr(
                                      "Internal error : unexpected fit method"
                                      )
                                );
         return scale_pair_enables();
         break;
      }

      if ( (int) qs.size() < minimum_pts_req ) {
         QMessageBox::critical( this,
                                windowTitle() + us_tr( ": Scale Fit" ),
                                QString( us_tr(
                                               "There are insufficient (%1) non-excluded points selected.\n"
                                               "The current fitting method requires a minimum of %2 points."
                                               )
                                         )
                                .arg( qs.size() )
                                .arg( minimum_pts_req )
                                );
         return scale_pair_enables();
      }

      if ( q1.size() + q2.size() != qs.size() ) {
         set < double > intersect;
         set_intersection( q1.begin(), q1.end(), q2.begin(), q2.end(), inserter(intersect, intersect.begin()) );

         QString detail;
         for ( auto const & q : intersect ) {
            detail += QString( " %1" ).arg( q );
         }
      
         QMessageBox::warning( this,
                               windowTitle() + us_tr( ": Scale Fit" ),
                               us_tr( "The data has duplicate q value(s):\n" )
                               + detail
                               + us_tr( "\nThe MALS set values will be used" )
                               );
         
         for ( auto const & q : q2 ) {
            q1.erase( q );
         }
      }
   }

   // get the names

   double time = le_scale_pair_time->text().toDouble();
   if ( !scale_pair_time_to_names.count( time ) ) {
      QMessageBox::critical( this,
                             windowTitle() + us_tr( ": Scale Fit" ),
                             QString( us_tr(
                                            "Internal error : files for time %1 not found"
                                            )
                                      )
                             .arg( time )
                             );
      return scale_pair_enables();
   }
      
   QString name1 = scale_pair_time_to_names[ time ][ scale_pair_mals_set ];
   QString name2 = scale_pair_time_to_names[ time ][ scale_pair_saxs_set ];

   vector < double > q;
   vector < double > I;
   vector < double > e;

   if ( scale_pair_use_errors ) {
      for ( int i = 0; i < (int) scale_pair_qgrids[ scale_pair_mals_set ].size(); ++i ) {
         if ( q1.count( scale_pair_qgrids[ scale_pair_mals_set ][ i ] ) ) {
            q.push_back( scale_pair_qgrids[ scale_pair_mals_set ][ i ] );
            I.push_back( f_Is[ name1 ][ i ] );
            e.push_back( f_errors[ name1 ][ i ] );
         }
      }
   
      for ( int i = 0; i < (int) scale_pair_qgrids[ scale_pair_saxs_set ].size(); ++i ) {
         if ( q2.count( scale_pair_qgrids[ scale_pair_saxs_set ][ i ] ) ) {
            q.push_back( scale_pair_qgrids[ scale_pair_saxs_set ][ i ] );
            I.push_back( f_Is[ name2 ][ i ] );
            e.push_back( f_errors[ name2 ][ i ] );
         }
      }
   
      US_Vector::printvector3( "data for fitting, q,I,e", q, I, e );
   } else {
      for ( int i = 0; i < (int) scale_pair_qgrids[ scale_pair_mals_set ].size(); ++i ) {
         if ( q1.count( scale_pair_qgrids[ scale_pair_mals_set ][ i ] ) ) {
            q.push_back( scale_pair_qgrids[ scale_pair_mals_set ][ i ] );
            I.push_back( f_Is[ name1 ][ i ] );
         }
      }
   
      for ( int i = 0; i < (int) scale_pair_qgrids[ scale_pair_saxs_set ].size(); ++i ) {
         if ( q2.count( scale_pair_qgrids[ scale_pair_saxs_set ][ i ] ) ) {
            q.push_back( scale_pair_qgrids[ scale_pair_saxs_set ][ i ] );
            I.push_back( f_Is[ name2 ][ i ] );
         }
      }
   
      US_Vector::printvector2( "data for fitting, q,I", q, I );
   }      

   return scale_pair_enables();
}

void US_Hydrodyn_Mals_Saxs::scale_pair_scale( const QString & text ) {
   qDebug() << "scale_pair_scale( " << text << " )";
   scale_pair_created_update();
   scale_pair_enables();
}

void US_Hydrodyn_Mals_Saxs::scale_pair_scale_focus( bool hasFocus ) {
   qDebug() << "scale_pair_scale_focus()";
   if ( hasFocus ) {
      // do our focus bits
      qDebug() << "scale_pair_time_focus : hasFocus";
      le_last_focus = le_scale_pair_scale;
      qwtw_wheel->setValue     ( le_scale_pair_scale->text().toDouble() );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setRange     ( 0.01, 10 );
      qwtw_wheel->setSingleStep( 0.01 );
      lbl_wheel_pos->setText   ( le_scale_pair_scale->text() );
   }
}

void US_Hydrodyn_Mals_Saxs::scale_pair_sd_scale( const QString & text ) {
   qDebug() << "scale_pair_sd_scale( " << text << " )";
   scale_pair_created_update();
   scale_pair_enables();
}

void US_Hydrodyn_Mals_Saxs::scale_pair_created_init() {
   qDebug() << "scale_pair_created_init()";
   // setup duplicate mals_set curves

   scale_pair_created_names.clear();
   scale_pair_created_time_to_name.clear();
   scale_pair_created_q.clear();
   scale_pair_created_I.clear();
   scale_pair_created_e.clear();
   
   for ( auto const & time_to_names : scale_pair_time_to_names ) {
      double  time = time_to_names.first;
      QString name = time_to_names.second[scale_pair_mals_set];

      QString scaled_name = name + "_scaled_temp";
      
      vector < double > q = f_qs[ name ];
      vector < double > I = f_Is[ name ];

      if ( scale_pair_use_errors ) {
         vector < double > e = f_errors[ name ];
         add_plot( scaled_name, q, I, e, false, false );
      } else {
         add_plot( scaled_name, q, I, false, false );
      }

#warning should add all the MALS units to MALS_SAXS & copy from here, possibly extend add_plot with a ref name to centralize

      scale_pair_created_names.insert( last_created_file );
      scale_pair_created_time_to_name[ time ] = last_created_file;
   }
}

void US_Hydrodyn_Mals_Saxs::scale_pair_created_update() {
   qDebug() << "scale_pair_created_update()";
   // called when scale and/or sd is updated
   // recreate & initially & when scale and/or sd is updated

   int qgrid_size             = (int) scale_pair_qgrids[ scale_pair_mals_set ].size();
   double scale_pair_scale    = le_scale_pair_scale->text().toDouble();
      
   if ( scale_pair_use_errors ) {
      double scale_pair_sd_scale = le_scale_pair_sd_scale->text().toDouble();
      for ( auto const & scaled_time_to_name : scale_pair_created_time_to_name ) {
         double  time        = scaled_time_to_name.first;
         QString scaled_name = scaled_time_to_name.second;
         QString source_name = scale_pair_time_to_names[ time ][ scale_pair_mals_set ];

         for ( int i = 0; i < qgrid_size; ++i ) {
            f_Is    [ scaled_name ][ i ] = f_Is    [ source_name ][ i ] * scale_pair_scale;
            f_errors[ scaled_name ][ i ] = f_errors[ source_name ][ i ] * scale_pair_scale * scale_pair_sd_scale;
         }
      }
   } else {
      for ( auto const & scaled_time_to_name : scale_pair_created_time_to_name ) {
         double  time        = scaled_time_to_name.first;
         QString scaled_name = scaled_time_to_name.second;
         QString source_name = scale_pair_time_to_names[ time ][ scale_pair_mals_set ];

         for ( int i = 0; i < qgrid_size; ++i ) {
            f_Is     [ scaled_name ][ i ] = f_Is    [ source_name ][ i ] * scale_pair_scale;
         }
      }
   }

   plot_files();
}        

void US_Hydrodyn_Mals_Saxs::scale_pair_created_remove() {
   // remove the temporary created curves
   remove_files( scale_pair_created_names );
   scale_pair_created_names.clear();
   scale_pair_created_time_to_name.clear();
   scale_pair_created_q.clear();
   scale_pair_created_I.clear();
   scale_pair_created_e.clear();
}

void US_Hydrodyn_Mals_Saxs::scale_pair_create_scaled_curves() {
#warning save headers

   qDebug() << "scale_pair_create_scaled_curves()";
   disable_all();

   scale_pair_save_names.clear();

   int qgrid_size             = (int) scale_pair_qgrids[ scale_pair_mals_set ].size();
   double scale_pair_scale    = le_scale_pair_scale->text().toDouble();
   double scale_pair_sd_scale = le_scale_pair_sd_scale->text().toDouble();

   if ( scale_pair_use_errors ) {
      for ( auto const & scaled_time_to_name : scale_pair_time_to_names ) {
         QString source_name = scaled_time_to_name.second[scale_pair_mals_set];
         QString scaled_name = source_name;
         scaled_name = scaled_name.replace( "_common", "" );
         scaled_name += QString( "_Im%1_SDm%2_common" )
            .arg( scale_pair_scale )
            .arg( scale_pair_sd_scale )
            ;

         vector < double > I = f_Is[ source_name ];
         vector < double > e = f_errors[ source_name ];

         for ( int i = 0; i < qgrid_size; ++i ) {
            I[ i ] *= scale_pair_scale;
            e[ i ] *= scale_pair_scale * scale_pair_sd_scale;
         }

         add_plot( scaled_name, scale_pair_qgrids[ scale_pair_mals_set ], I, e, false, false );
         scale_pair_save_names.insert( last_created_file );
         scale_pair_org_selected.erase( source_name );
         scale_pair_org_selected.insert( last_created_file );
      }
   } else {
      for ( auto const & scaled_time_to_name : scale_pair_time_to_names ) {
         QString source_name = scaled_time_to_name.second[scale_pair_mals_set];
         QString scaled_name = source_name;
         scaled_name = scaled_name.replace( "_common", "" );
         scaled_name += QString( "_Im%1_common" )
            .arg( scale_pair_scale )
            ;

         vector < double > I = f_Is[ source_name ];

         for ( int i = 0; i < qgrid_size; ++i ) {
            I[ i ] *= scale_pair_scale;
         }

         add_plot( scaled_name, scale_pair_qgrids[ scale_pair_mals_set ], I, false, false );
         scale_pair_save_names.insert( last_created_file );
         scale_pair_org_selected.erase( source_name );
         scale_pair_org_selected.insert( last_created_file );
      }
   }      

   return scale_pair_enables();
}

void US_Hydrodyn_Mals_Saxs::scale_pair_reset() {
   qDebug() << "scale_pair_reset()";
   le_scale_pair_scale->setText( "1" );
   le_scale_pair_sd_scale->setText( "1" );
}
