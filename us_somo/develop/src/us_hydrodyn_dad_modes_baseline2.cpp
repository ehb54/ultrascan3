#include "../include/us3_defines.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_hydrodyn_mals_saxs.h"
#include "../include/us_hydrodyn_mals_saxs_scale_trend.h"
#include "../include/us_pm.h"

#define TSO QTextStream(stdout)

// --- baseline2 ---

void US_Hydrodyn_Dad::baseline2_start() {
   TSO << "baseline2()\n";
   le_last_focus = (mQLineEdit *) 0;

   // verify selected files and setup baseline2_names
   // must match time and one must end in _common
   // all istar or ihashq already verfied by _gui

   disable_all();

   baseline2_names              .clear();
   baseline2_name               = "";
   baseline2_org_selected       .clear();
   baseline2_q_start_save       = le_baseline2_q_start->text();
   baseline2_q_end_save         = le_baseline2_q_end  ->text();

   QStringList names           = all_selected_files();

   if ( names.size() != 1 ) {
      QMessageBox::critical( this,
                             windowTitle() + us_tr( ": Baseline" ),
                             QString( us_tr(
                                            "%1 curves selected, Baseline requires exactly one curve selected\n"
                                            )
                                      )
                             .arg( names.size() )
                             );
      return update_enables();
   }
   
   baseline2_org_selected = all_selected_files_set();
   baseline2_name         = names[0];

   // using errors?

   baseline2_use_errors =
      f_errors.count( baseline2_name )
      && f_qs.count( baseline2_name )
      && f_errors[ baseline2_name ].size() == f_qs[ baseline2_name ].size()
      && !is_zero_vector( f_errors[ baseline2_name ] )
      ;

   // if ( !baseline2_use_errors &&
   //      ( 
   //       (US_Eigen::weight_methods) cb_baseline2_fit_alg_weight->currentData().toInt() == US_Eigen::EIGEN_1_OVER_SD
   //       || (US_Eigen::weight_methods) cb_baseline2_fit_alg_weight->currentData().toInt() == US_Eigen::EIGEN_1_OVER_SD_SQ
   //        ) ) {
   //    cb_baseline2_fit_alg_weight->setCurrentIndex( 0 ); // US_Eigen::EIGEN_NO_WEIGHTS
   // }

   mode_select( MODE_BASELINE2 );
   pb_wheel_save->show();
   
   running       = true;

   // set wheel range etc.

   // set le_baseline2_q1,2 start,end to grid limits
   baseline2_markers.clear();

   baseline2_add_marker( plot_dist, le_baseline2_q_start->text().toDouble(), baseline2_color_q, us_tr( "Start")             , Qt::AlignRight | Qt::AlignBottom );
   baseline2_add_marker( plot_dist, le_baseline2_q_end  ->text().toDouble(), baseline2_color_q, us_tr( "End"  )             , Qt::AlignLeft  | Qt::AlignBottom );

   baseline2_q_min = f_qs[ baseline2_name ].front();
   baseline2_q_max = f_qs[ baseline2_name ].back();

   qDebug() << "baseline2 range " << baseline2_q_min << " " << baseline2_q_max;

   if (
       le_baseline2_q_start->text().isEmpty() 
       || le_baseline2_q_start->text().toDouble() < baseline2_q_min
       || le_baseline2_q_start->text().toDouble() > baseline2_q_max
       ) {
      le_baseline2_q_start->setText( QString( "%1" ).arg( baseline2_q_min ) );
      qDebug() << "baseline2 q start set " << baseline2_q_min;
   }
   if (
       le_baseline2_q_end->text().isEmpty() 
       || le_baseline2_q_end->text().toDouble() > baseline2_q_max
       || le_baseline2_q_end->text().toDouble() <= baseline2_q_min + 0.001 // == caused by the slot, +0.001 for rounding
       ) {
      le_baseline2_q_end  ->setText( QString( "%1" ).arg( baseline2_q_max ) );
      qDebug() << "baseline2 q end set " << baseline2_q_max;
   } else {
      qDebug() << "baseline2 q end NOT set, value " << le_baseline2_q_end->text();
   }

   lbl_wheel_pos->setText("");
   pb_baseline2_create_adjusted_curve   ->setEnabled( false ); // set in curve_update & curve_clear
   baseline2_enables();
}

void US_Hydrodyn_Dad::baseline2_enables()
{
   // qDebug() << "baseline2_enables() last_minimize_scale " << last_minimize_scale << " baseline2_scale: " << le_baseline2_scale->text().toDouble();
   // mostly disable except cancel & wheel
   // if ( minimize_running ) {
   //    TSO << "baseline2_enables() disabled!\n";
   //    return;
   // }
   TSO << "baseline2_enables()\n";
   
   qwtw_wheel                           ->setEnabled( true );
   pb_wheel_cancel                      ->setEnabled( true );
   pb_wheel_save                        ->setEnabled( true );
   pb_axis_x                            ->setEnabled( true );
   pb_axis_y                            ->setEnabled( true );
   cb_eb                                ->setEnabled( true );
   cb_dots                              ->setEnabled( true );
   pb_color_rotate                      ->setEnabled( true );
   pb_line_width                        ->setEnabled( true );
   pb_legend                            ->setEnabled( true );
   pb_q_exclude_vis                     ->setEnabled( true );
   pb_q_exclude_left                    ->setEnabled( true );
   pb_q_exclude_right                   ->setEnabled( true );
   pb_q_exclude_clear                   ->setEnabled( q_exclude.size() > 0 );
   pb_pp                                ->setEnabled( true );
   
   pb_baseline2_fit                     ->setEnabled( baseline2_fit_curve == 0 );

   le_baseline2_q_start                 ->setEnabled( true );
   le_baseline2_q_end                   ->setEnabled( true );

   wheel_enables( true );

   progress->reset();
}

void US_Hydrodyn_Dad::baseline2_scroll_highlight( int pos )
{
   qDebug() << "baseline2_scroll_highlight " << pos;
}

void US_Hydrodyn_Dad::baseline2_add_marker( 
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
   baseline2_markers.push_back( marker );
}   

void US_Hydrodyn_Dad::baseline2_delete_markers() {
   plot_dist   ->detachItems( QwtPlotItem::Rtti_PlotMarker );
   baseline2_markers.clear();
}

#define UHMS_SCALE_WHEEL_RES 10000
// #define UHMS_SCALE_WHEEL_RES 1e-6

void US_Hydrodyn_Dad::baseline2_q_start_text( const QString & text ) {
   TSO << QString( "baseline2_q_start_text( %1 )\n" ).arg( text );
   if ( current_mode != MODE_BASELINE2 ) {
      return;
   }
   if ( baseline2_markers.size() != 2 ) {
      editor_msg( "red", QString( "internal error: baseline2_q_start_text markers issue size %1" ).arg( baseline2_markers.size() ) );
      return;
   }
   
   baseline2_fit_clear( true );

   lbl_wheel_pos->setText( text );
   baseline2_markers[ 0 ]->setXValue( text.toDouble() );
   // plot_dist->replot();

   if ( !wheel_is_pressed && qwtw_wheel->value() != text.toDouble() ) {
      qwtw_wheel->setValue( text.toDouble() );
   }

   disconnect( le_baseline2_q_start, SIGNAL( textChanged( const QString & ) ), 0, 0 );
   le_baseline2_q_start->setText( QString( "%1" ).arg( text.toDouble() ) );
   connect   ( le_baseline2_q_start, SIGNAL( textChanged( const QString & ) ), SLOT( baseline2_q_start_text( const QString & ) ) );

   if ( text.toDouble() > le_baseline2_q_end->text().toDouble() ) {
      le_baseline2_q_end->setText( text );
   } else {
      TSO << QString( "baseline2_q_start_text() calls any analysis etc()\n" );
   }
   baseline2_curve_update();
}

void US_Hydrodyn_Dad::baseline2_q_end_text( const QString & text ) {
   // TSO << QString( "baseline2_q_end_text( %1 )\n" ).arg( text );
   if ( current_mode != MODE_BASELINE2 ) {
      return;
   }
   if ( baseline2_markers.size() != 2 ) {
      editor_msg( "red", QString( "internal error: baseline2_q_end_text markers issue size %1" ).arg( baseline2_markers.size() ) );
      return;
   }

   baseline2_fit_clear( false );

   lbl_wheel_pos->setText( text );
   baseline2_markers[ 1 ]->setXValue( text.toDouble() );
   // plot_dist->replot();

   if ( !wheel_is_pressed && qwtw_wheel->value() != text.toDouble() ) {
      qwtw_wheel->setValue( text.toDouble() );
   }

   disconnect( le_baseline2_q_end, SIGNAL( textChanged( const QString & ) ), 0, 0 );
   le_baseline2_q_end->setText( QString( "%1" ).arg( text.toDouble() ) );
   connect   ( le_baseline2_q_end, SIGNAL( textChanged( const QString & ) ), SLOT( baseline2_q_end_text( const QString & ) ) );

   if ( text.toDouble() < le_baseline2_q_start->text().toDouble() ) {
      le_baseline2_q_start->setText( text );
   } else {
      TSO << QString( "baseline2_q_end_text() calls any analysis etc\n" );
   }
   baseline2_curve_update();
}

void US_Hydrodyn_Dad::baseline2_q_start_focus( bool hasFocus ) {
   qDebug() << "baseline2_q_start_focus()";
   if ( current_mode != MODE_BASELINE2 ) {
      return;
   }
   if ( hasFocus ) {
      le_last_focus = le_baseline2_q_start;
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      qwtw_wheel->setRange( baseline2_q_min, baseline2_q_max);
      qwtw_wheel->setSingleStep( ( baseline2_q_max - baseline2_q_min ) / UHMS_SCALE_WHEEL_RES );
      // qwtw_wheel->setSingleStep( UHMS_SCALE_WHEEL_RES );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setValue( le_baseline2_q_start->text().toDouble() );
      wheel_enables();
   }
}

void US_Hydrodyn_Dad::baseline2_q_end_focus( bool hasFocus )
{
   qDebug() << "baseline2_q_end_focus()";
   if ( current_mode != MODE_BASELINE2 ) {
      return;
   }
   if ( hasFocus )
   {
      le_last_focus = le_baseline2_q_end;
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      qwtw_wheel->setRange( baseline2_q_min, baseline2_q_max);
      qwtw_wheel->setSingleStep( ( baseline2_q_max - baseline2_q_min ) / UHMS_SCALE_WHEEL_RES );
      // qwtw_wheel->setSingleStep( UHMS_SCALE_WHEEL_RES );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setValue( le_baseline2_q_end->text().toDouble() );
      wheel_enables();
   }
}

void US_Hydrodyn_Dad::baseline2_fit() {
   qDebug() << "baseline2_fit()";

   baseline2_curve_update();
   return baseline2_enables();
}

// qt doc says int argument for the signal, but actually QString, just dropping it


void US_Hydrodyn_Dad::baseline2_fit_clear( bool replot ) {
   qDebug() << "baseline2_fit_clear()";

   lbl_baseline2_msg->setText("");
   pb_baseline2_create_adjusted_curve   ->setEnabled( false );

   if ( baseline2_fit_curve ) {
      qDebug() << "baseline2_fit_clear() baseline2_fit_curve set";
      baseline2_fit_curve->detach();
      delete baseline2_fit_curve;
      lbl_baseline2_msg->setText("");
      baseline2_fit_curve = (QwtPlotCurve *) 0;
   }

   if ( replot ) {
      plot_dist->replot();
   }
}

void US_Hydrodyn_Dad::baseline2_curve_update() {
   qDebug() << "baseline2_curve_update()";
   // compute & plot 

   baseline2_fit_clear( false );
      
   double q_start = le_baseline2_q_start->text().toDouble();
   double q_end   = le_baseline2_q_end->text().toDouble();

   baseline2_fit_avg = 0;
   int    points     = 0;
   
   for ( size_t i = 0; i < f_qs[ baseline2_name ].size(); ++i ) {
      double q = f_qs[ baseline2_name ][ i ];
      if ( q >= q_start && q <= q_end && !q_exclude.count( q ) ) {
         baseline2_fit_avg += f_Is[ baseline2_name ][ i ];
         ++points;
      }
   }

   if ( points ) {
      baseline2_fit_avg /= (double) points;

      vector < double > x = { q_start, q_end };
      vector < double > y = { baseline2_fit_avg, baseline2_fit_avg };

      baseline2_fit_curve = new QwtPlotCurve( "baseline fit" );
      baseline2_fit_curve->setStyle( QwtPlotCurve::Lines );

      baseline2_fit_curve->setPen( QPen( Qt::green, use_line_width, Qt::SolidLine ) );
      baseline2_fit_curve->setSamples(
                                      (double *)&x[ 0 ],
                                      (double *)&y[ 0 ],
                                      2
                                      );
      baseline2_fit_curve->attach( plot_dist );
      lbl_baseline2_msg->setText( QString( us_tr( "Fit average value %1" ) ).arg( baseline2_fit_avg ) );
      pb_baseline2_create_adjusted_curve   ->setEnabled( true );
   } else {
      baseline2_fit_avg = 0;
      lbl_baseline2_msg->setText( us_tr( "No data points found in fit range" ) );
   }

   plot_dist->replot();
   
}

void US_Hydrodyn_Dad::baseline2_apply() {
   qDebug() << "baseline2_apply()";
   baseline2_create_adjusted_curve( all_selected_files_set() );
   update_enables();
}

void US_Hydrodyn_Dad::baseline2_create_adjusted_curve() {
   qDebug() << "baseline2_create_adjusted_curve()";
   baseline2_create_adjusted_curve( baseline2_name );
}

void US_Hydrodyn_Dad::baseline2_create_adjusted_curve( const set < QString > & names ) {
   qDebug() << "baseline2_create_adjusted_curve( set < QString > )";
   set < QString > to_select;
   for ( auto const & name : names ) {
      baseline2_create_adjusted_curve( name );
      to_select.insert( last_created_file );
   }
   set_selected( to_select );
}

void US_Hydrodyn_Dad::baseline2_create_adjusted_curve( const QString & name ) {
   qDebug() << "baseline2_create_adjusted_curve( QString )";
   
   if ( !f_qs.count( name ) || !f_Is.count( name ) ) {
      editor_msg( "red", QString( us_tr( "Error: missing data for curve %1" ) ).arg( name ) );
      return;
   }

   double q_start = le_baseline2_q_start->text().toDouble();
   double q_end   = le_baseline2_q_end->text().toDouble();

   double fit_avg = 0;
   int    points  = 0;
   
   for ( size_t i = 0; i < f_qs[ name ].size(); ++i ) {
      double q = f_qs[ name ][ i ];
      if ( q >= q_start && q <= q_end && !q_exclude.count( q ) ) {
         fit_avg += f_Is[ name ][ i ];
         ++points;
      }
   }

   if ( points ) {
      fit_avg /= (double) points;

      QString newname = name + QString( "_blc%1" ).arg( fit_avg ).replace( ".", "_" );

      vector < double > y = f_Is[ name ];
      for ( auto & v : y ) {
         v -= fit_avg;
      }

      if ( f_errors.count( name ) && f_errors[ name ].size() == f_qs[ name ].size() ) {
         add_plot( newname, f_qs[ name ], y, f_errors[ name ], true, false );
      } else {
         add_plot( newname, f_qs[ name ], y, true, false );
      }
   } else {
      editor_msg( "red", QString( us_tr( "Error: no data points found in baseline range for curve %1" ) ).arg( name ) );
      return;
   }
}
