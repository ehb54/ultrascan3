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

   QStringList names = all_selected_files();

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

   int mals_set = f_qs[ scale_pair_names[0][0] ].front() < f_qs[ scale_pair_names[1][0] ].front() ? 0 : 1;
   int saxs_set = 1 - mals_set;

   for ( int i = 0; i < (int) scale_pair_times.size(); ++i ) {
      scale_pair_time_to_names[ scale_pair_times[i] ].push_back( scale_pair_names[mals_set][i] );
      scale_pair_time_to_names[ scale_pair_times[i] ].push_back( scale_pair_names[saxs_set][i] );
   }

   mode_select( MODE_SCALE_PAIR );

   running       = true;

   // set wheel range etc.

   scale_pair_time_pos      = 0;

   qwtw_wheel->setRange     ( 0, scale_pair_times.size() - 1 );
   qwtw_wheel->setSingleStep( 1 );
   qwtw_wheel->setValue     ( scale_pair_time_pos );
   
   // set le_scale_pair_q1,2 start,end to grid limits
   scale_pair_markers.clear();

   scale_pair_add_marker( plot_dist, le_scale_pair_q1_start  ->text().toDouble(), scale_pair_color_q1, us_tr( "q1 Start") );
   scale_pair_add_marker( plot_dist, le_scale_pair_q1_end    ->text().toDouble(), scale_pair_color_q1, us_tr( "q1 End"  ), Qt::AlignLeft | Qt::AlignTop );
   scale_pair_add_marker( plot_dist, le_scale_pair_q2_start  ->text().toDouble(), scale_pair_color_q2, us_tr( "q2 Start") );
   scale_pair_add_marker( plot_dist, le_scale_pair_q2_end    ->text().toDouble(), scale_pair_color_q2, us_tr( "q2 End"  ), Qt::AlignLeft | Qt::AlignTop );

   scale_pair_q1_min = f_qs[ scale_pair_names[mals_set][0] ].front();
   scale_pair_q1_max = f_qs[ scale_pair_names[mals_set][0] ].back();
   scale_pair_q2_min = f_qs[ scale_pair_names[saxs_set][0] ].front();
   scale_pair_q2_max = f_qs[ scale_pair_names[saxs_set][0] ].back();

   le_scale_pair_q1_start->setText( QString( "%1" ).arg( scale_pair_q1_min ) );
   le_scale_pair_q1_end  ->setText( QString( "%1" ).arg( scale_pair_q1_max ) );
   le_scale_pair_q2_start->setText( QString( "%1" ).arg( scale_pair_q2_min ) );
   le_scale_pair_q2_end  ->setText( QString( "%1" ).arg( scale_pair_q2_max ) );

   scale_pair_scroll_highlight(0);
   scale_pair_enables();
}

void US_Hydrodyn_Mals_Saxs::scale_pair_enables()
{
   // mostly disable except cancel & wheel
   TSO << "scale_pair_enables()\n";
   pb_wheel_cancel                      ->setEnabled( true );
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
   pb_scale_pair_create_scaled_curves   ->setEnabled( true );
   rb_scale_pair_fit_method_p2          ->setEnabled( true );
   rb_scale_pair_fit_method_p3          ->setEnabled( true );
   rb_scale_pair_fit_method_p4          ->setEnabled( true );
   le_scale_pair_q1_start               ->setEnabled( true );
   le_scale_pair_q1_end                 ->setEnabled( true );
   le_scale_pair_q2_start               ->setEnabled( true );
   le_scale_pair_q2_end                 ->setEnabled( true );
   le_scale_pair_scale                  ->setEnabled( true );
   le_scale_pair_sd_scale               ->setEnabled( true );

   wheel_enables( true );
}

void US_Hydrodyn_Mals_Saxs::scale_pair_scroll_highlight( int pos )
{
   // show the plot
   // TSO << "scale_pair_scroll_highlight( " <<  pos << " )\n";
   if ( pos >= (int) scale_pair_times.size() ) {
      qDebug() << "error: scale_pair_scroll_highlight( " <<  pos << " ) out of range, ignored!";
      return;
   }
   
   double time = scale_pair_times[ pos ];
   le_scale_pair_time->setText( QString( "%1" ).arg( time ) );

   set < QString > toplot;
   if ( scale_pair_names.size() == 2 ) {
      toplot = {
         scale_pair_time_to_names[ time ][ 0 ]
         ,scale_pair_time_to_names[ time ][ 1 ]
      };
   } else {
      toplot = {
         scale_pair_time_to_names[ time ][ 0 ]
      };
   }

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

void US_Hydrodyn_Mals_Saxs::scale_pair_sd_scale( const QString & text ) {
   qDebug() << "scale_pair_sd_scale( " << text << " )";
}

void US_Hydrodyn_Mals_Saxs::scale_pair_scale( const QString & text ) {
   qDebug() << "scale_pair_scale( " << text << " )";
}

void US_Hydrodyn_Mals_Saxs::scale_pair_delete_markers() {
   plot_dist   ->detachItems( QwtPlotItem::Rtti_PlotMarker );
   scale_pair_markers.clear();
}

void US_Hydrodyn_Mals_Saxs::scale_pair_set_fit_method_p2() {
   qDebug() << "scale_pair_set_fit_method_p2()";
}

void US_Hydrodyn_Mals_Saxs::scale_pair_set_fit_method_p3() {
   qDebug() << "scale_pair_set_fit_method_p3()";
}

void US_Hydrodyn_Mals_Saxs::scale_pair_set_fit_method_p4() {
   qDebug() << "scale_pair_set_fit_method_p3()";
}
   
void US_Hydrodyn_Mals_Saxs::scale_pair_fit() {
   qDebug() << "scale_pair_fit()";
}

void US_Hydrodyn_Mals_Saxs::scale_pair_reset() {
   qDebug() << "scale_pair_reset()";
}

void US_Hydrodyn_Mals_Saxs::scale_pair_minimize() {
   qDebug() << "scale_pair_minimize()";
}

void US_Hydrodyn_Mals_Saxs::scale_pair_create_scaled_curves() {
   qDebug() << "scale_pair_create_scaled_curves()";
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
      qwtw_wheel->setRange     ( 0, scale_pair_times.size() - 1 );
      qwtw_wheel->setSingleStep( 1 );
      qwtw_wheel->setValue     ( scale_pair_time_pos );
      lbl_wheel_pos->setText   ( "" );
   }
}
