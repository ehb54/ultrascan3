#include "../include/us3_defines.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_hydrodyn_mals_saxs.h"
#include "../include/us_hydrodyn_mals_saxs_scale_trend.h"
#include "../include/us_pm.h"
#include <QPixmap>
#include "../include/us_eigen.h"

static US_Eigen               eigen;
static bool                   minimize_running;
static int                    variable_set;
static int                    fixed_set;
static QString                last_minimize_method;
static QString                last_minimize_weight;
static QString                last_minimize_curve;
static QString                last_minimize_q_ranges;
static double                 last_minimize_sd_scale;
static double                 last_minimize_scale;
static map < double, double > last_minimize_chi2s;

#define TSO QTextStream(stdout)

// --- scale pairs ---

void US_Hydrodyn_Mals_Saxs::scale_pair( bool no_store_original )
{
   TSO << "scale_pairs()\n";
   le_last_focus = (mQLineEdit *) 0;
   minimize_running = false;

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

   scale_pair_minimize_clear();

   QStringList names = all_selected_files();
   if ( !no_store_original ) {
      scale_pair_original_scale    = le_scale_pair_scale   ->text().toDouble();
      scale_pair_original_sd_scale = le_scale_pair_sd_scale->text().toDouble();
   }

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
   variable_set        = cb_scale_pair_scale_saxs->isChecked() ? scale_pair_saxs_set : scale_pair_mals_set;
   fixed_set           = cb_scale_pair_scale_saxs->isChecked() ? scale_pair_mals_set : scale_pair_saxs_set;

   for ( int i = 0; i < (int) scale_pair_times.size(); ++i ) {
      scale_pair_time_to_names[ scale_pair_times[i] ].push_back( scale_pair_names[ scale_pair_mals_set ][i] );
      scale_pair_time_to_names[ scale_pair_times[i] ].push_back( scale_pair_names[ scale_pair_saxs_set ][i] );
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

   lbl_scale_pair_q1_range->setText(
                                    cb_scale_pair_scale_saxs->isChecked()
                                    ? us_tr( " Fit range 1 (fixed): " )
                                    : us_tr( " Fit range 1 (scales): " )
                                    );
   lbl_scale_pair_q2_range->setText(
                                    cb_scale_pair_scale_saxs->isChecked()
                                    ? us_tr( " Fit range 2 (scales): " )
                                    : us_tr( " Fit range 2 (fixed): " )
                                    );

   lbl_scale_pair_sd_scale->setText(
                                    cb_scale_pair_scale_saxs->isChecked()
                                    ? us_tr( " SAXS SD Mult.: " )
                                    : us_tr( " MALS SD Mult.: " )
                                    );
   
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

   scale_pair_q1_min = f_qs[ scale_pair_names[ scale_pair_mals_set ][0] ].front();
   scale_pair_q1_max = f_qs[ scale_pair_names[ scale_pair_mals_set ][0] ].back();
   scale_pair_q2_min = f_qs[ scale_pair_names[ scale_pair_saxs_set ][0] ].front();
   scale_pair_q2_max = f_qs[ scale_pair_names[ scale_pair_saxs_set ][0] ].back();

   // disconnect( le_scale_pair_q1_start, SIGNAL( textChanged( const QString & ) ), 0, 0 );
   // disconnect( le_scale_pair_q1_end, SIGNAL( textChanged( const QString & ) ), 0, 0 );
   if (
       le_scale_pair_q1_start->text().isEmpty() 
       || le_scale_pair_q1_start->text().toDouble() < scale_pair_q1_min
       || le_scale_pair_q1_start->text().toDouble() > scale_pair_q1_max
        ) {
      le_scale_pair_q1_start->setText( QString( "%1" ).arg( scale_pair_q1_min ) );
   }
   if (
       le_scale_pair_q1_end->text().isEmpty() 
       || le_scale_pair_q1_end->text().toDouble() > scale_pair_q1_max
       || le_scale_pair_q1_end->text().toDouble() < scale_pair_q1_min
       || le_scale_pair_q1_end->text().toDouble() == scale_pair_q1_min // caused by the slot
        ) {
      le_scale_pair_q1_end  ->setText( QString( "%1" ).arg( scale_pair_q1_max ) );
   }
   // connect( le_scale_pair_q1_start, SIGNAL( textChanged( const QString & ) ), SLOT( scale_pair_q1_start_text( const QString & ) ) );
   // connect( le_scale_pair_q1_end, SIGNAL( textChanged( const QString & ) ), SLOT( scale_pair_q1_end_text( const QString & ) ) );

   // disconnect( le_scale_pair_q2_start, SIGNAL( textChanged( const QString & ) ), 0, 0 );
   // disconnect( le_scale_pair_q2_end, SIGNAL( textChanged( const QString & ) ), 0, 0 );
   if (
       le_scale_pair_q2_start->text().isEmpty() 
       || le_scale_pair_q2_start->text().toDouble() < scale_pair_q2_min
       || le_scale_pair_q2_start->text().toDouble() > scale_pair_q2_max
        ) {
      le_scale_pair_q2_start->setText( QString( "%1" ).arg( scale_pair_q2_min ) );
   }
   if (
       le_scale_pair_q2_end->text().isEmpty() 
       || le_scale_pair_q2_end->text().toDouble() > scale_pair_q2_max
       || le_scale_pair_q2_end->text().toDouble() < scale_pair_q2_min
       || le_scale_pair_q2_end->text().toDouble() == scale_pair_q2_min // caused by the slot
        ) {
      le_scale_pair_q2_end  ->setText( QString( "%1" ).arg( scale_pair_q2_max ) );
   }
   // connect( le_scale_pair_q2_start, SIGNAL( textChanged( const QString & ) ), SLOT( scale_pair_q2_start_text( const QString & ) ) );
   // connect( le_scale_pair_q2_end, SIGNAL( textChanged( const QString & ) ), SLOT( scale_pair_q2_end_text( const QString & ) ) );

   suppress_plot = true;
   scale_pair_created_update();
   suppress_plot = false;
   scale_pair_scroll_highlight(0);
   lbl_wheel_pos->setText("");
   scale_pair_enables();
}

void US_Hydrodyn_Mals_Saxs::scale_pair_enables()
{
   // qDebug() << "scale_pair_enables() last_minimize_scale " << last_minimize_scale << " scale_pair_scale: " << le_scale_pair_scale->text().toDouble();
   // mostly disable except cancel & wheel
   if ( minimize_running ) {
      TSO << "scale_pair_enables() disabled!\n";
      return;
   }
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
   pb_pp                                ->setEnabled( true );
   
   pb_scale_pair_fit                    ->setEnabled( true );
   pb_scale_pair_minimize               ->setEnabled( true );
   pb_scale_pair_create_scaled_curves   ->setEnabled( ( le_scale_pair_scale->text().toDouble() != 1 ||
                                                        ( scale_pair_use_errors && le_scale_pair_sd_scale->text().toDouble() != 1 ) )
                                                      && le_scale_pair_scale->text().toDouble() > 0
                                                      && le_scale_pair_sd_scale->text().toDouble() > 0
                                                      );

   pb_scale_pair_reset                  ->setEnabled( le_scale_pair_scale->text().toDouble() != 1 ||
                                                      le_scale_pair_sd_scale->text().toDouble() != 1 );
   le_scale_pair_q1_start               ->setEnabled( true );
   le_scale_pair_q1_end                 ->setEnabled( true );
   le_scale_pair_q2_start               ->setEnabled( true );
   le_scale_pair_q2_end                 ->setEnabled( true );
   le_scale_pair_scale                  ->setEnabled( true );
   le_scale_pair_sd_scale               ->setEnabled( scale_pair_use_errors );

   cb_scale_pair_fit_curve              ->setEnabled( true );
   cb_scale_pair_fit_alg                ->setEnabled( true );
   cb_scale_pair_fit_alg_weight         ->setEnabled( true );
   cb_scale_pair_scale_saxs             ->setEnabled( true );

   wheel_enables( true );

   progress->reset();
}

void US_Hydrodyn_Mals_Saxs::scale_pair_scroll_highlight( int pos )
{
   // show the plot
   if ( !minimize_running ) {
      qDebug() << "scale_pair_scroll_highlight( " <<  pos << " )";
   } else {
      return;
   }
   
   if ( pos >= (int) scale_pair_times.size() ) {
      qDebug() << "error: scale_pair_scroll_highlight( " <<  pos << " ) out of range, ignored!";
      return;
   }
   
   scale_pair_time_pos = pos;

   double time = scale_pair_times[ pos ];
   le_scale_pair_time->setText( QString( "%1" ).arg( time ) );

   set < QString > toplot;
   toplot = {
      scale_pair_created_time_to_name[ time ] // always the scaling
      // scale_pair_time_to_names[ time ][ variable_set ] (previously unscaled)
      ,scale_pair_time_to_names[ time ][ fixed_set ]
   };

#warning call fit
   lbl_scale_pair_msg->setText( "" ); 
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
   
   scale_pair_fit_clear( false );

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

   scale_pair_fit_clear( false );

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
   
   scale_pair_fit_clear( false );

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

   scale_pair_fit_clear( false );
   
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
      qwtw_wheel->setRange     ( -DBL_MAX, DBL_MAX );
      qwtw_wheel->setValue     ( scale_pair_time_pos );
      qwtw_wheel->setRange     ( 0, scale_pair_times.size() - 1 );
      qwtw_wheel->setSingleStep( 1 );
      lbl_wheel_pos->setText   ( "" );
   }
}

void US_Hydrodyn_Mals_Saxs::scale_pair_scale( const QString & text ) {
   qDebug() << "scale_pair_scale( " << text << " )";
   // qDebug() << "scale_pair_scale() last_minimize_scale " << last_minimize_scale << " scale_pair_scale: " << le_scale_pair_scale->text().toDouble();
   lbl_wheel_pos->setText   ( le_scale_pair_scale->text() );
   scale_pair_fit_clear( false );
   scale_pair_created_update();
   scale_pair_enables();
}

void US_Hydrodyn_Mals_Saxs::scale_pair_scale_focus( bool hasFocus ) {
   qDebug() << "scale_pair_scale_focus()";
   if ( hasFocus ) {
      // do our focus bits
      qDebug() << "scale_pair_time_focus : hasFocus";
      le_last_focus = le_scale_pair_scale;
      qwtw_wheel->setRange     ( -DBL_MAX, DBL_MAX );
      qwtw_wheel->setValue     ( le_scale_pair_scale->text().toDouble() );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setRange     ( 0.01, 10 );
      qwtw_wheel->setSingleStep( 0.01 );
      lbl_wheel_pos->setText   ( le_scale_pair_scale->text() );
   }
}

void US_Hydrodyn_Mals_Saxs::scale_pair_sd_scale( const QString & text ) {
   qDebug() << "scale_pair_sd_scale( " << text << " )";
   scale_pair_fit_clear( false );
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
      QString name = time_to_names.second[ variable_set ];

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

   int qgrid_size             = (int) scale_pair_qgrids[ variable_set ].size();
   double scale_pair_scale    = le_scale_pair_scale->text().toDouble();
      
   if ( scale_pair_use_errors ) {
      double scale_pair_sd_scale = le_scale_pair_sd_scale->text().toDouble();
      for ( auto const & scaled_time_to_name : scale_pair_created_time_to_name ) {
         double  time        = scaled_time_to_name.first;
         QString scaled_name = scaled_time_to_name.second;
         QString source_name = scale_pair_time_to_names[ time ][ variable_set ];

         for ( int i = 0; i < qgrid_size; ++i ) {
            f_Is    [ scaled_name ][ i ] = f_Is    [ source_name ][ i ] * scale_pair_scale;
            f_errors[ scaled_name ][ i ] = f_errors[ source_name ][ i ] * scale_pair_scale * scale_pair_sd_scale;
         }
      }
   } else {
      for ( auto const & scaled_time_to_name : scale_pair_created_time_to_name ) {
         double  time        = scaled_time_to_name.first;
         QString scaled_name = scaled_time_to_name.second;
         QString source_name = scale_pair_time_to_names[ time ][ variable_set ];

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

void US_Hydrodyn_Mals_Saxs::scale_pair_reset() {
   qDebug() << "scale_pair_reset()";
   le_scale_pair_scale->setText( "1" );
   le_scale_pair_sd_scale->setText( "1" );
   scale_pair_fit_clear();
}

void US_Hydrodyn_Mals_Saxs::scale_pair_fit() {
   qDebug() << "scale_pair_fit()";
   disable_all();
#warning qwtw_wheel probably should be in ::disable_all() but for some reason we commented it out there, probably should check all wheel usage and set as appropriate
   qwtw_wheel            ->setEnabled( false );
   scale_pair_fit_clear();

   // collect q grid positions

   set < double > q1;
   set < double > q2;
   set < double > qs;

   double q1_min = le_scale_pair_q1_start->text().toDouble();
   double q1_max = le_scale_pair_q1_end  ->text().toDouble();
   double q2_min = le_scale_pair_q2_start->text().toDouble();
   double q2_max = le_scale_pair_q2_end  ->text().toDouble();

   for ( auto const & q : scale_pair_qgrids[ scale_pair_mals_set ] ) {
      if ( q >= q1_min && q <= q1_max && !q_exclude.count( q ) ) {
         q1.insert( q );
         qs.insert( q );
      }
   }
   
   for ( auto const & q : scale_pair_qgrids[ scale_pair_saxs_set ] ) {
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

   int degree;

   {
      int minimum_pts_req = 2;
      switch ( (scale_pair_fit_curves) cb_scale_pair_fit_curve->currentData().toInt() ) {
      case SCALE_PAIR_FIT_CURVE_P2 :
         minimum_pts_req = 3;
         degree          = 2;
         break;
      case SCALE_PAIR_FIT_CURVE_P3 :
         minimum_pts_req = 4;
         degree          = 3;
         break;
      case SCALE_PAIR_FIT_CURVE_P4 :
         minimum_pts_req = 5;
         degree          = 4;
         break;
      case SCALE_PAIR_FIT_CURVE_P5 :
         minimum_pts_req = 6;
         degree          = 5;
         break;
      case SCALE_PAIR_FIT_CURVE_P6 :
         minimum_pts_req = 7;
         degree          = 6;
         break;
      case SCALE_PAIR_FIT_CURVE_P7 :
         minimum_pts_req = 8;
         degree          = 7;
         break;
      case SCALE_PAIR_FIT_CURVE_P8 :
         minimum_pts_req = 9;
         degree          = 8;
         break;
      default:
         QMessageBox::critical( this,
                                windowTitle() + us_tr( ": Scale Fit" ),
                                us_tr(
                                      "Internal error : unexpected fit curve"
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

   double time = le_scale_pair_time->text().toDouble();

   // compute globally first
   // US_Timer           us_timers;
   // us_timers.clear_timers();
   // us_timers.init_timer( "polyfit" );

   progress->reset();

   double global_chi2 = 0;
   {
      int pp = 0;
      progress->setMaximum( scale_pair_times.size() + 1 );
      for ( auto const & t : scale_pair_times ) {
         if ( !scale_pair_time_to_names.count( t ) ) {
            QMessageBox::critical( this,
                                   windowTitle() + us_tr( ": Scale Fit" ),
                                   QString( us_tr(
                                                  "Internal error : files for time %1 not found"
                                                  )
                                            )
                                   .arg( t )
                                   );
            return scale_pair_enables();
         }

         if ( t != time ) {
            progress->setValue( pp++ );
            qApp->processEvents();
            double chi2;
            scale_pair_fit_at_time( t, degree, q1, q2, chi2 );
            global_chi2 += chi2;
         }
      }
   }         
      
   progress->setValue( scale_pair_times.size() );

   // get the names
      
   QString name1 = scale_pair_time_to_names[ time ][ scale_pair_mals_set ];
   QString name2 = scale_pair_time_to_names[ time ][ scale_pair_saxs_set ];

   vector < double > q;
   vector < double > I;
   vector < double > e;

   vector < double > coeff;
   vector < double > x;
   vector < double > y;
   double            chi2;

   double scale_pair_scale    = le_scale_pair_scale->text().toDouble();

   bool use_errors;

   switch( (US_Eigen::weight_methods) cb_scale_pair_fit_alg_weight->currentData().toInt() ) {
   case US_Eigen::EIGEN_NO_WEIGHTS :
   case US_Eigen::EIGEN_1_OVER_AMOUNT :
   case US_Eigen::EIGEN_1_OVER_AMOUNT_SQ :
      use_errors = false;
      break;
      
   case US_Eigen::EIGEN_1_OVER_SD :
   case US_Eigen::EIGEN_1_OVER_SD_SQ :
      use_errors = true;
      break;
   default:
      qDebug() << "scale_pair_fit_at_time eigen weight error";
      use_errors = false;
      break;
   }
   
   if ( scale_pair_use_errors && use_errors ) {
      double scale_pair_sd_scale = le_scale_pair_sd_scale->text().toDouble();
      for ( int i = 0; i < (int) scale_pair_qgrids[ scale_pair_mals_set ].size(); ++i ) {
         if ( q1.count( scale_pair_qgrids[ scale_pair_mals_set ][ i ] ) ) {
            q.push_back( scale_pair_qgrids[ scale_pair_mals_set ][ i ] );
            I.push_back( f_Is[ name1 ][ i ] * scale_pair_scale );
            e.push_back( f_errors[ name1 ][ i ] * scale_pair_scale * scale_pair_sd_scale );
         }
      }
   
      for ( int i = 0; i < (int) scale_pair_qgrids[ scale_pair_saxs_set ].size(); ++i ) {
         if ( q2.count( scale_pair_qgrids[ scale_pair_saxs_set ][ i ] ) ) {
            q.push_back( scale_pair_qgrids[ scale_pair_saxs_set ][ i ] );
            I.push_back( f_Is[ name2 ][ i ] );
            e.push_back( f_errors[ name2 ][ i ] );
         }
      }
   
      // US_Vector::printvector3( "data for fitting, q,I,e", q, I, e );
      // us_timers.start_timer( "polyfit" );
      eigen.polyfit( q, I, e, degree, coeff, chi2
                     ,(US_Eigen::fit_methods)    cb_scale_pair_fit_alg->currentData().toInt() 
                     ,(US_Eigen::weight_methods) cb_scale_pair_fit_alg_weight->currentData().toInt()
                     );
      // us_timers.end_timer( "polyfit" );
   } else {
      for ( int i = 0; i < (int) scale_pair_qgrids[ scale_pair_mals_set ].size(); ++i ) {
         if ( q1.count( scale_pair_qgrids[ scale_pair_mals_set ][ i ] ) ) {
            q.push_back( scale_pair_qgrids[ scale_pair_mals_set ][ i ] );
            I.push_back( f_Is[ name1 ][ i ] * scale_pair_scale );
         }
      }
   
      for ( int i = 0; i < (int) scale_pair_qgrids[ scale_pair_saxs_set ].size(); ++i ) {
         if ( q2.count( scale_pair_qgrids[ scale_pair_saxs_set ][ i ] ) ) {
            q.push_back( scale_pair_qgrids[ scale_pair_saxs_set ][ i ] );
            I.push_back( f_Is[ name2 ][ i ] );
         }
      }
   
      // US_Vector::printvector2( "data for fitting, q,I", q, I );
      // us_timers.start_timer( "polyfit" );
      eigen.polyfit( q, I, degree, coeff, chi2
                     ,(US_Eigen::fit_methods)    cb_scale_pair_fit_alg->currentData().toInt() 
                     ,(US_Eigen::weight_methods) cb_scale_pair_fit_alg_weight->currentData().toInt()
                     );
      // us_timers.end_timer( "polyfit" );
   }      

   global_chi2 += chi2;
   
   // us_qdebug( us_timers.list_times() );

   lbl_scale_pair_msg->setText( QString( "nChi^2 %1 average %2 (%3 curves)" )
                                .arg( chi2 / q.size() )
                                .arg( global_chi2 / ( q.size() * scale_pair_times.size() ) )
                                .arg( scale_pair_times.size() )
                                );
   eigen.evaluate_polynomial( coeff, q1_min, q2_max, 100, x, y );
   US_Vector::printvector( "coefficients", coeff );
   // US_Vector::printvector2( "fitting curve", x, y );

   {
      scale_pair_fit_clear( false );
         
      if ( axis_y_log ) {
         vector < double > trimmed_x;
         vector < double > trimmed_y;

         for ( size_t i = 0; i < x.size(); ++i ) {
            if ( y[i] > 0 ) {
               trimmed_x.push_back( x[i] );
               trimmed_y.push_back( y[i] );
            }
         }

         x = trimmed_x;
         y = trimmed_y;
      }

      scale_pair_fit_curve = new QwtPlotCurve( "scale_fit" );
      scale_pair_fit_curve->setStyle( QwtPlotCurve::Lines );
      scale_pair_fit_curve->setSamples(
                        (double *)&( x[ 0 ] ),
                        (double *)&( y[ 0 ] ),
                        x.size() );
      scale_pair_fit_curve->setPen( QPen( QColor( Qt::red ), use_line_width, Qt::SolidLine ) );
      scale_pair_fit_curve->attach( plot_dist );
      plot_dist->replot();
   }

   return scale_pair_enables();
}

bool US_Hydrodyn_Mals_Saxs::scale_pair_fit_at_time( double time
                                                    ,int degree
                                                    ,const set < double > & q1
                                                    ,const set < double > & q2
                                                    ,double & chi2 ) {
   if ( !minimize_running ) {
      qDebug() << "scale_pair_fit_at_time( " << time << " , &chi2 )";
   }
   
   QString name1 = scale_pair_time_to_names[ time ][ scale_pair_mals_set ];
   QString name2 = scale_pair_time_to_names[ time ][ scale_pair_saxs_set ];

   vector < double > q;
   vector < double > I;
   vector < double > e;

   vector < double > coeff;
   vector < double > x;
   vector < double > y;

   double scale_pair_scale    = le_scale_pair_scale->text().toDouble();

   bool use_errors;

   switch( (US_Eigen::weight_methods) cb_scale_pair_fit_alg_weight->currentData().toInt() ) {
   case US_Eigen::EIGEN_NO_WEIGHTS :
   case US_Eigen::EIGEN_1_OVER_AMOUNT :
   case US_Eigen::EIGEN_1_OVER_AMOUNT_SQ :
      use_errors = false;
      break;
      
   case US_Eigen::EIGEN_1_OVER_SD :
   case US_Eigen::EIGEN_1_OVER_SD_SQ :
      use_errors = true;
      break;
   default:
      qDebug() << "scale_pair_fit_at_time eigen weight error";
      use_errors = false;
      break;
   }

   if ( scale_pair_use_errors && use_errors ) {
      double scale_pair_sd_scale = le_scale_pair_sd_scale->text().toDouble();
      for ( int i = 0; i < (int) scale_pair_qgrids[ scale_pair_mals_set ].size(); ++i ) {
         if ( q1.count( scale_pair_qgrids[ scale_pair_mals_set ][ i ] ) ) {
            q.push_back( scale_pair_qgrids[ scale_pair_mals_set ][ i ] );
            I.push_back( f_Is[ name1 ][ i ] * scale_pair_scale );
            e.push_back( f_errors[ name1 ][ i ] * scale_pair_scale * scale_pair_sd_scale );
         }
      }
   
      for ( int i = 0; i < (int) scale_pair_qgrids[ scale_pair_saxs_set ].size(); ++i ) {
         if ( q2.count( scale_pair_qgrids[ scale_pair_saxs_set ][ i ] ) ) {
            q.push_back( scale_pair_qgrids[ scale_pair_saxs_set ][ i ] );
            I.push_back( f_Is[ name2 ][ i ] );
            e.push_back( f_errors[ name2 ][ i ] );
         }
      }
   
      // US_Vector::printvector3( "data for fitting, q,I,e", q, I, e );
      eigen.polyfit( q, I, e, degree, coeff, chi2
                     ,(US_Eigen::fit_methods)    cb_scale_pair_fit_alg->currentData().toInt() 
                     ,(US_Eigen::weight_methods) cb_scale_pair_fit_alg_weight->currentData().toInt()
                     );
   } else {
      for ( int i = 0; i < (int) scale_pair_qgrids[ scale_pair_mals_set ].size(); ++i ) {
         if ( q1.count( scale_pair_qgrids[ scale_pair_mals_set ][ i ] ) ) {
            q.push_back( scale_pair_qgrids[ scale_pair_mals_set ][ i ] );
            I.push_back( f_Is[ name1 ][ i ] * scale_pair_scale );
         }
      }
   
      for ( int i = 0; i < (int) scale_pair_qgrids[ scale_pair_saxs_set ].size(); ++i ) {
         if ( q2.count( scale_pair_qgrids[ scale_pair_saxs_set ][ i ] ) ) {
            q.push_back( scale_pair_qgrids[ scale_pair_saxs_set ][ i ] );
            I.push_back( f_Is[ name2 ][ i ] );
         }
      }
   
      // US_Vector::printvector2( "data for fitting, q,I", q, I );
      eigen.polyfit( q, I, degree, coeff, chi2
                     ,(US_Eigen::fit_methods)    cb_scale_pair_fit_alg->currentData().toInt() 
                     ,(US_Eigen::weight_methods) cb_scale_pair_fit_alg_weight->currentData().toInt()
                     );
   }

   return true;
}

void US_Hydrodyn_Mals_Saxs::scale_pair_minimize() {
   qDebug() << "scale_pair_minimize()";

   disable_all();
   qwtw_wheel            ->setEnabled( false );
   scale_pair_fit_clear();
   scale_pair_minimize_clear();
   
   // collect q grid positions

   set < double > q1;
   set < double > q2;
   set < double > qs;

   double q1_min = le_scale_pair_q1_start->text().toDouble();
   double q1_max = le_scale_pair_q1_end  ->text().toDouble();
   double q2_min = le_scale_pair_q2_start->text().toDouble();
   double q2_max = le_scale_pair_q2_end  ->text().toDouble();

   for ( auto const & q : scale_pair_qgrids[ scale_pair_mals_set ] ) {
      if ( q >= q1_min && q <= q1_max && !q_exclude.count( q ) ) {
         q1.insert( q );
         qs.insert( q );
      }
   }
   
   for ( auto const & q : scale_pair_qgrids[ scale_pair_saxs_set ] ) {
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
      scale_pair_minimize_clear();
      return scale_pair_enables();
   }

   int degree;

   last_minimize_sd_scale = le_scale_pair_sd_scale->text().toDouble();

   {
      int minimum_pts_req = 2;
      switch ( (scale_pair_fit_curves) cb_scale_pair_fit_curve->currentData().toInt() ) {
      case SCALE_PAIR_FIT_CURVE_P2 :
         minimum_pts_req     = 3;
         degree              = 2;
         last_minimize_curve = "2nd_degree_Polynomial";
         break;
      case SCALE_PAIR_FIT_CURVE_P3 :
         minimum_pts_req     = 4;
         degree              = 3;
         last_minimize_curve = "3rd_degree_Polynomial";
         break;
      case SCALE_PAIR_FIT_CURVE_P4 :
         minimum_pts_req     = 5;
         degree              = 4;
         last_minimize_curve = "4th_degree_Polynomial";
         break;
      case SCALE_PAIR_FIT_CURVE_P5 :
         minimum_pts_req     = 6;
         degree              = 5;
         last_minimize_curve = "5th_degree_Polynomial";
         break;
      case SCALE_PAIR_FIT_CURVE_P6 :
         minimum_pts_req     = 7;
         degree              = 6;
         last_minimize_curve = "6th_degree_Polynomial";
         break;
      case SCALE_PAIR_FIT_CURVE_P7 :
         minimum_pts_req     = 8;
         degree              = 7;
         last_minimize_curve = "7th_degree_Polynomial";
         break;
      case SCALE_PAIR_FIT_CURVE_P8 :
         minimum_pts_req     = 9;
         degree              = 8;
         last_minimize_curve = "8th_degree_Polynomial";
         break;
      default:
         QMessageBox::critical( this,
                                windowTitle() + us_tr( ": Scale Fit" ),
                                us_tr(
                                      "Internal error : unexpected fit curve"
                                      )
                                );
         scale_pair_minimize_clear();
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
         scale_pair_minimize_clear();
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

   // initial grid search
   // get range, step dialog

   double scale_start;
   double scale_end;
   double scale_step;

   {
      bool try_again;
      do {
         try_again = false;
         QDialog dialog(this);
         dialog.setWindowTitle( windowTitle() + us_tr( ": Scale Minimize" ) );
         // Use a layout allowing a label next to each field
         dialog.setMinimumWidth( 200 );

         QFormLayout form(&dialog);

         // Add some text above the fields
         form.addRow( new QLabel(
                                 us_tr(
                                       "Minimize parameters\n"
                                       "Fill out the values below and click OK\n"
                                       )
                                 ) );

         // Add the lineEdits with their respective labels
         QList<QLineEdit *> fields;
   
         vector < QString > labels =
            {
               us_tr( "Global scale start :" )
               ,us_tr( "Global scale end :" )
               ,us_tr( "Step size:" )
            };


         vector < double > defaults =
            {
               0.5
               ,1.5
               ,0.01
            };

         for( int i = 0; i < (int) labels.size(); ++i ) {
            QLineEdit *lineEdit = new QLineEdit( &dialog );
            lineEdit->setValidator( new QDoubleValidator(this) );
            form.addRow( labels[i], lineEdit );
            lineEdit->setText( QString( "%1" ).arg( defaults[i] ) );
            fields << lineEdit;
         }

         // Add some standard buttons (Cancel/Ok) at the bottom of the dialog
         QDialogButtonBox buttonBox(
                                    QDialogButtonBox::Ok | QDialogButtonBox::Cancel
                                    ,Qt::Horizontal
                                    ,&dialog
                                    );
         form.addRow(&buttonBox);
         QObject::connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
         QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

         // Show the dialog as modal
         if (dialog.exec() == QDialog::Accepted) {
            // If the user didn't dismiss the dialog, do something with the fields
            scale_start  = fields[0]->text().toDouble();
            scale_end    = fields[1]->text().toDouble();
            scale_step   = fields[2]->text().toDouble();
            if ( scale_step <= 0 ||
                 scale_end <= scale_start ) {
               try_again = true;
            }
         } else {
            scale_pair_minimize_clear();
            return scale_pair_enables();
         }
      } while ( try_again );
   }
   
   progress->reset();
   progress->setMaximum( 1 + ( (scale_end - scale_start) / scale_step ) );
   
   last_minimize_method = us_tr( US_Eigen::qs_fit_method( (US_Eigen::fit_methods) cb_scale_pair_fit_alg->currentData().toInt() ) );
   last_minimize_weight = us_tr( US_Eigen::qs_weight_method( (US_Eigen::weight_methods) cb_scale_pair_fit_alg_weight->currentData().toInt() ) );
   last_minimize_q_ranges =
      QString( "[%1-%2],[%3,%4]" )
      .arg( q1_min )
      .arg( q1_max )
      .arg( q2_min )
      .arg( q2_max )
      ;

   US_Timer           us_timers;
   us_timers.clear_timers();
   us_timers.init_timer( "minimize" );

   double best_scale = -DBL_MAX;
   double best_gchi2 = DBL_MAX;

   minimize_running = true;

   us_timers.start_timer( "minimize" );
   int pp = 0;
   for ( double s = scale_start; s <= scale_end; s += scale_step ) {
      le_scale_pair_scale->setText( QString( "%1" ).arg( s ) );
      progress->setValue( pp++ );
      qApp->processEvents();
      
      double global_chi2 = 0;
      map < double, double > tmp_minimize_chi2s;
      {
         for ( auto const & t : scale_pair_times ) {
            if ( !scale_pair_time_to_names.count( t ) ) {
               QMessageBox::critical( this,
                                      windowTitle() + us_tr( ": Scale Fit" ),
                                      QString( us_tr(
                                                     "Internal error : files for time %1 not found"
                                                     )
                                               )
                                      .arg( t )
                                      );
               minimize_running = false;
               return scale_pair_enables();
            }

            double chi2;
            scale_pair_fit_at_time( t, degree, q1, q2, chi2 );
            global_chi2 += chi2;
            tmp_minimize_chi2s[ t ] = chi2 / qs.size();
         }
      }
      if ( best_gchi2 > global_chi2 ) {
         best_gchi2 = global_chi2;
         best_scale = s;
         last_minimize_chi2s = tmp_minimize_chi2s;
         qDebug() << QString( "scale %1 gChi2 %2 new best\n" ).arg( s ).arg( global_chi2 / ( qs.size() * scale_pair_times.size() ) );
      } else {
         qDebug() << QString( "scale %1 gChi2 %2\n" ).arg( s ).arg( global_chi2 / ( qs.size() * scale_pair_times.size() ) );
      }
   }
   us_timers.end_timer( "minimize" );
   us_qdebug( us_timers.list_times() );
   minimize_running = false;
   le_scale_pair_scale->setText( QString( "%1" ).arg( best_scale ) );
   last_minimize_scale = le_scale_pair_scale->text().toDouble();
   scale_pair_time_focus( true );
   scale_pair_fit();
}

// qt doc says int argument for the signal, but actually QString, just dropping it

void US_Hydrodyn_Mals_Saxs::scale_pair_fit_alg_index() {
   qDebug() << "scale_pair_fit_alg_index()";
   scale_pair_fit_clear();
};


void US_Hydrodyn_Mals_Saxs::scale_pair_fit_alg_weight_index() {
   qDebug() << "scale_pair_fit_alg_weight_index()";
   scale_pair_fit_clear();
}

void US_Hydrodyn_Mals_Saxs::scale_pair_fit_curve_index() {
   qDebug() << "scale_pair_fit_curve_index()";
   scale_pair_fit_clear();
}

void US_Hydrodyn_Mals_Saxs::scale_pair_scale_saxs() {
   qDebug() << "scale_pair_scale_saxs()";
   suppress_plot = true;
   int save_time_pos = scale_pair_time_pos;
   scale_pair_created_remove();
   //   scale_pair_created_init();
   // redisplay
   scale_pair_created_remove();
   scale_pair_delete_markers();
   set_selected( scale_pair_org_selected );
   scale_pair( true );
   suppress_plot = false;
   scale_pair_scroll_highlight( save_time_pos );
}

void US_Hydrodyn_Mals_Saxs::scale_pair_create_scaled_curves() {
#warning save headers

   qDebug() << "scale_pair_create_scaled_curves()";
   // qDebug() << "scale_pair_create_scaled_curves() last_minimize_scale " << last_minimize_scale << " scale_pair_scale: " << le_scale_pair_scale->text().toDouble();

   disable_all();
   scale_pair_save_names.clear();

   int qgrid_size_mals        = (int) scale_pair_qgrids[ scale_pair_mals_set ].size();
   double scale_pair_scale    = le_scale_pair_scale->text().toDouble();
   double scale_pair_sd_scale = le_scale_pair_sd_scale->text().toDouble();

   // remove rounding check since we now set last_minimize scale from lb_scale_pair_scale->text().toDouble() instead of the loop value
   // if ( (int)(200*scale_pair_scale) != (int)(200*last_minimize_scale) ) {
   if ( scale_pair_scale != last_minimize_scale ) {
      scale_pair_minimize_clear();
      switch ( QMessageBox::question(this, 
                                     windowTitle() + us_tr( " : Scale Make Scaled" )
                                     ,us_tr( "The scale has changed since the last 'minimize' or you did not 'minimize' at all.\n"
                                             "You must 'minimize' again to keep fitting header information" )
                                     ,us_tr( "&Continue anyway" )
                                     ,us_tr( "&Quit" )
                                     ) )
      {
      case 0 : 
         break;
      case 1 : 
         return scale_pair_enables();
         break;
      default: 
         break;
      }
   }

   bool create_saxs = false;
   if ( scale_pair_scale != 1 ) {
      switch ( QMessageBox::question(this, 
                                     windowTitle() + us_tr( " : Scale Make Scaled" )
                                     ,us_tr( "How do you want to save the scaled curves?" )
                                     ,us_tr( "Scaled &MALS data" )
                                     ,us_tr( "Inverse scaled &SAXS data" )
                                     ) )
      {
      case 0 : 
         break;
      case 1 : 
         create_saxs = true;
         break;
      default: 
         break;
      }
   }


   if ( create_saxs ) {
      qDebug() << "scale_pair_create_scaled_curves() - will create saxs";
   }

   // cases
   // sd mult != 1 .. always SD MALS
   // create_saxs == false ... just as normal
   // create_saxs == true ... create MALS without scaling only if sd_mult != 1, create SAXS files with inverse scaling

   // MALS saves

   if ( !create_saxs || scale_pair_sd_scale != 1 ) {
      double use_scale_pair_scale = create_saxs ? 1. : scale_pair_scale;
      if ( scale_pair_use_errors ) {
         const QString & suffix     = "_MIm%1_MSDm%2_common";
         for ( auto const & scaled_time_to_name : scale_pair_time_to_names ) {
            QString source_name = scaled_time_to_name.second[ scale_pair_mals_set ];
            QString scaled_name = source_name;
            scaled_name = scaled_name.replace( "_common", "" );
            scaled_name += QString( suffix )
               .arg( use_scale_pair_scale )
               .arg( scale_pair_sd_scale )
               ;

            vector < double > I = f_Is[ source_name ];
            vector < double > e = f_errors[ source_name ];

            for ( int i = 0; i < qgrid_size_mals; ++i ) {
               I[ i ] *= use_scale_pair_scale;
               e[ i ] *= use_scale_pair_scale * scale_pair_sd_scale;
            }

            add_plot( scaled_name, scale_pair_qgrids[ scale_pair_mals_set ], I, e, false, false );
            scale_pair_save_names.insert( last_created_file );
            scale_pair_org_selected.erase( source_name );
            scale_pair_org_selected.insert( last_created_file );
            if ( !last_minimize_curve.isEmpty() ) {
               f_fit_curve[ last_created_file ] = last_minimize_curve;
            } else {
               f_fit_curve.erase( last_created_file );
            }
            if ( !last_minimize_method.isEmpty() ) {
               f_fit_method[ last_created_file ] =
                  QString( "%1:%2" ).arg( last_minimize_method ).arg( last_minimize_weight );
            } else {
               f_fit_method.erase( last_created_file );
            }
            if ( !last_minimize_q_ranges.isEmpty() ) {
               f_fit_q_ranges[ last_created_file ] = last_minimize_q_ranges;
            } else {
               f_fit_q_ranges.erase( last_created_file );
            }
            if ( last_minimize_chi2s.count( scaled_time_to_name.first ) ) {
               f_fit_chi2[ last_created_file ] = last_minimize_chi2s[ scaled_time_to_name.first ];
            } else {
               f_fit_chi2.erase( last_created_file );
            }
            if ( last_minimize_sd_scale != 0 ) {
               f_fit_sd_scale[ last_created_file ] = last_minimize_sd_scale;
            } else {
               f_fit_sd_scale.erase( last_created_file );
            }
         }
      } else {
         const QString & suffix     = "_MIm%1_common";
         for ( auto const & scaled_time_to_name : scale_pair_time_to_names ) {
            QString source_name = scaled_time_to_name.second[ scale_pair_mals_set ];
            QString scaled_name = source_name;
            scaled_name = scaled_name.replace( "_common", "" );
            scaled_name += QString( suffix )
               .arg( use_scale_pair_scale )
               ;

            vector < double > I = f_Is[ source_name ];

            for ( int i = 0; i < qgrid_size_mals; ++i ) {
               I[ i ] *= use_scale_pair_scale;
            }

            add_plot( scaled_name, scale_pair_qgrids[ scale_pair_mals_set ], I, false, false );
            scale_pair_save_names.insert( last_created_file );
            scale_pair_org_selected.erase( source_name );
            scale_pair_org_selected.insert( last_created_file );

            if ( !last_minimize_curve.isEmpty() ) {
               f_fit_curve[ last_created_file ] = last_minimize_curve;
            } else {
               f_fit_curve.erase( last_created_file );
            }
            if ( !last_minimize_method.isEmpty() ) {
               f_fit_method[ last_created_file ] =
                  QString( "%1:%2" ).arg( last_minimize_method ).arg( last_minimize_weight );
            } else {
               f_fit_method.erase( last_created_file );
            }
            if ( !last_minimize_q_ranges.isEmpty() ) {
               f_fit_q_ranges[ last_created_file ] = last_minimize_q_ranges;
            } else {
               f_fit_q_ranges.erase( last_created_file );
            }
            if ( last_minimize_chi2s.count( scaled_time_to_name.first ) ) {
               f_fit_chi2[ last_created_file ] = last_minimize_chi2s[ scaled_time_to_name.first ];
            } else {
               f_fit_chi2.erase( last_created_file );
            }
            if ( last_minimize_sd_scale != 0 ) {
               f_fit_sd_scale[ last_created_file ] = last_minimize_sd_scale;
            } else {
               f_fit_sd_scale.erase( last_created_file );
            }
         }
      }
   }

   // SAXS save

   if ( create_saxs ) {
      qDebug() << "create saxs inverse scaled curves";
      int qgrid_size_saxs         = (int) scale_pair_qgrids[ scale_pair_saxs_set ].size();
      double use_scale_pair_scale = 1. / scale_pair_scale;
      const QString & suffix      = "_SIm%1";
      if ( scale_pair_use_errors ) {
         for ( auto const & scaled_time_to_name : scale_pair_time_to_names ) {
            QString source_name = scaled_time_to_name.second[ scale_pair_saxs_set ];
            QString scaled_name = source_name;
            scaled_name = scaled_name.replace( "_common", "" );
            scaled_name += QString( suffix )
               .arg( use_scale_pair_scale )
               ;

            vector < double > I = f_Is[ source_name ];
            vector < double > e = f_errors[ source_name ];

            for ( int i = 0; i < qgrid_size_saxs; ++i ) {
               I[ i ] *= use_scale_pair_scale;
               e[ i ] *= use_scale_pair_scale;
            }

            add_plot( scaled_name, scale_pair_qgrids[ scale_pair_saxs_set ], I, e, false, false );
            scale_pair_save_names.insert( last_created_file );
            scale_pair_org_selected.erase( source_name );
            scale_pair_org_selected.insert( last_created_file );
            if ( !last_minimize_curve.isEmpty() ) {
               f_fit_curve[ last_created_file ] = last_minimize_curve;
            } else {
               f_fit_curve.erase( last_created_file );
            }
            if ( !last_minimize_method.isEmpty() ) {
               f_fit_method[ last_created_file ] =
                  QString( "%1:%2" ).arg( last_minimize_method ).arg( last_minimize_weight );
            } else {
               f_fit_method.erase( last_created_file );
            }
            if ( !last_minimize_q_ranges.isEmpty() ) {
               f_fit_q_ranges[ last_created_file ] = last_minimize_q_ranges;
            } else {
               f_fit_q_ranges.erase( last_created_file );
            }
            if ( last_minimize_chi2s.count( scaled_time_to_name.first ) ) {
               f_fit_chi2[ last_created_file ] = last_minimize_chi2s[ scaled_time_to_name.first ];
            } else {
               f_fit_chi2.erase( last_created_file );
            }
            if ( last_minimize_sd_scale != 0 ) {
               f_fit_sd_scale[ last_created_file ] = last_minimize_sd_scale;
            } else {
               f_fit_sd_scale.erase( last_created_file );
            }
         }
      } else {
         for ( auto const & scaled_time_to_name : scale_pair_time_to_names ) {
            QString source_name = scaled_time_to_name.second[ scale_pair_saxs_set ];
            QString scaled_name = source_name;
            scaled_name = scaled_name.replace( "_common", "" );
            scaled_name += QString( suffix )
               .arg( use_scale_pair_scale )
               ;

            vector < double > I = f_Is[ source_name ];

            for ( int i = 0; i < qgrid_size_saxs; ++i ) {
               I[ i ] *= use_scale_pair_scale;
            }

            add_plot( scaled_name, scale_pair_qgrids[ scale_pair_saxs_set ], I, false, false );
            scale_pair_save_names.insert( last_created_file );
            scale_pair_org_selected.erase( source_name );
            scale_pair_org_selected.insert( last_created_file );
            if ( !last_minimize_curve.isEmpty() ) {
               f_fit_curve[ last_created_file ] = last_minimize_curve;
            } else {
               f_fit_curve.erase( last_created_file );
            }
            if ( !last_minimize_method.isEmpty() ) {
               f_fit_method[ last_created_file ] =
                  QString( "%1:%2" ).arg( last_minimize_method ).arg( last_minimize_weight );
            } else {
               f_fit_method.erase( last_created_file );
            }
            if ( !last_minimize_q_ranges.isEmpty() ) {
               f_fit_q_ranges[ last_created_file ] = last_minimize_q_ranges;
            } else {
               f_fit_q_ranges.erase( last_created_file );
            }
            if ( last_minimize_chi2s.count( scaled_time_to_name.first ) ) {
               f_fit_chi2[ last_created_file ] = last_minimize_chi2s[ scaled_time_to_name.first ];
            } else {
               f_fit_chi2.erase( last_created_file );
            }
            if ( last_minimize_sd_scale != 0 ) {
               f_fit_sd_scale[ last_created_file ] = last_minimize_sd_scale;
            } else {
               f_fit_sd_scale.erase( last_created_file );
            }
         }
      }
   }

   return scale_pair_enables();
}

void US_Hydrodyn_Mals_Saxs::scale_pair_minimize_clear() {
   qDebug() << "scale_pair_minimize_clear()";
   last_minimize_curve    = "";
   last_minimize_method   = "";
   last_minimize_weight   = "";
   last_minimize_q_ranges = "";
   last_minimize_scale    = 0e0;
   last_minimize_sd_scale = 0e0;
   last_minimize_chi2s.clear();
}   

void US_Hydrodyn_Mals_Saxs::scale_pair_fit_clear( bool replot ) {
   if ( !minimize_running ) {
      qDebug() << "scale_pair_fit_clear( " << ( replot ? "true" : "false" ) << " )";
   }
   if ( scale_pair_fit_curve ) {
      qDebug() << "scale_pair_fit_clear() scale_pair_fit_curve set";
      scale_pair_fit_curve->detach();
      delete scale_pair_fit_curve;
      lbl_scale_pair_msg->setText("");
      scale_pair_fit_curve = (QwtPlotCurve *) 0;
      if ( replot ) {
         plot_dist->replot();
      }
   }
}
