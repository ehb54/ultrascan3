#include "../include/us_hydrodyn.h"
#include "../include/us_hydrodyn_saxs_hplc.h"

void US_Hydrodyn_Saxs_Hplc::scale()
{
   le_last_focus = (mQLineEdit *) 0;

   bool any_selected = false;

   scale_selected    .clear();
   scale_q           .clear();
   scale_I           .clear();
   scale_e           .clear();
   scale_last_created.clear();

   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      if ( lb_files->isSelected( i ) )
      {
         if ( !any_selected )
         {
            wheel_file = lb_files->text( i );
         }
         scale_selected.insert( lb_files->text( i ) );
         any_selected = true;
         scale_q[ lb_files->text( i ) ] = f_qs[ lb_files->text( i ) ];
         scale_I[ lb_files->text( i ) ] = f_Is[ lb_files->text( i ) ];
         scale_e[ lb_files->text( i ) ] = f_errors[ lb_files->text( i ) ];
         if ( !plotted_curves.count( lb_files->text( i ) ) )
         {
            editor_msg( "red", QString( tr( "Internal error: scale selected %1, but no plotted curve found" ) ).arg( lb_files->text( i ) ) );
            return;
         }
      }
   }

   if ( !any_selected )
   {
      editor_msg( "red", tr( "Internal error: no files selected in scale mode" ) );
      return;
   }

   if ( !f_qs.count( wheel_file ) )
   {
      editor_msg( "red", QString( tr( "Internal error: %1 not found in data" ) ).arg( wheel_file ) );
      return;
   }

   if ( f_qs[ wheel_file ].size() < 2 )
   {
      editor_msg( "red", QString( tr( "Internal error: %1 almost empty data" ) ).arg( wheel_file ) );
      return;
   }

   if ( !f_Is.count( wheel_file ) )
   {
      editor_msg( "red", QString( tr( "Internal error: %1 not found in y data" ) ).arg( wheel_file ) );
      return;
   }

   if ( !f_Is[ wheel_file ].size() )
   {
      editor_msg( "red", QString( tr( "Internal error: %1 empty y data" ) ).arg( wheel_file ) );
      return;
   }

   if ( le_scale_q_start->text().isEmpty() ||
        le_scale_q_start->text() == "0" ||
        le_scale_q_start->text().toDouble() < f_qs[ wheel_file ][ 0 ] )
   {
      disconnect( le_scale_q_start, SIGNAL( textChanged( const QString & ) ), 0, 0 );
      le_scale_q_start->setText( QString( "%1" ).arg( f_qs[ wheel_file ][ 0 ] ) );
      connect( le_scale_q_start, SIGNAL( textChanged( const QString & ) ), SLOT( scale_q_start_text( const QString & ) ) );
   }

   if ( le_scale_q_end->text().isEmpty() ||
        le_scale_q_end->text() == "0" ||
        le_scale_q_end->text().toDouble() > f_qs[ wheel_file ].back() )
   {
      disconnect( le_scale_q_end, SIGNAL( textChanged( const QString & ) ), 0, 0 );
      le_scale_q_end->setText( QString( "%1" ).arg( f_qs[ wheel_file ].back() ) );
      connect( le_scale_q_end, SIGNAL( textChanged( const QString & ) ), SLOT( scale_q_end_text( const QString & ) ) );
   }

   disable_all();
   mode_select( MODE_SCALE );
   pb_rescale     ->setEnabled( true );

   scale_applied = false;

   running       = true;

   scale_enables();

   plotted_markers.clear();
   gauss_add_marker( le_scale_q_start  ->text().toDouble(), Qt::red, tr( "Start") );
   gauss_add_marker( le_scale_q_end    ->text().toDouble(), Qt::red, tr( "End"  ) );
   plot_dist->replot();
}

void US_Hydrodyn_Saxs_Hplc::scale_enables()
{
   pb_wheel_start        ->setEnabled( false );
   pb_wheel_cancel       ->setEnabled( true );
   rb_scale_high         ->setEnabled( true );
   rb_scale_low          ->setEnabled( true );
   le_scale_q_start      ->setEnabled( true );
   le_scale_q_end        ->setEnabled( true );
   pb_scale_q_reset      ->setEnabled( true );
   pb_scale_reset        ->setEnabled( true );
   pb_scale_apply        ->setEnabled( true );
   pb_scale_create       ->setEnabled( scale_applied );
}

QString US_Hydrodyn_Saxs_Hplc::scale_get_target( bool do_msg )
{
   double min_i = 0e0;
   double max_i = 0e0;
   double tot_i = 0e0;

   double q_min = le_scale_q_start->text().toDouble();
   double q_max = le_scale_q_end  ->text().toDouble();

   QString msg;

   if ( !scale_selected.size() )
   {
      editor_msg( "red",  tr( "Scale: internal error, no selected files" ) );
      return "";
   }

   if ( q_min >= q_max )
   {
      editor_msg( "red",  tr( "Error: Scale mode: minimum q value is greator or equal to maximum q value" ) );
      return "";
   }

   QString max_file;
   QString min_file;

   bool first_one = true;
   for ( set < QString >::iterator it = scale_selected.begin();
         it != scale_selected.end();
         ++it )
   {
      QString file = *it;
      double this_i = tot_intensity( file, q_min, q_max );
      if ( first_one || min_i > this_i )
      {
         first_one = false;
         min_i     = this_i;
         min_file  = file;
      }
      if ( first_one || max_i < this_i )
      {
         first_one = false;
         max_i     = this_i;
         max_file  = file;
      }
      tot_i += this_i;
   }

   msg += QString( "\nScale %2 files from q: %2 %3\n" ).arg( scale_selected.size() ).arg( q_min ).arg( q_max );

   msg += QString( 
                  "minimum total I : %1 %2\n" 
                  "maximum total I : %3 %4\n"
                  "average total I : %5\n" 
                   )
      .arg( min_file )
      .arg( min_i )
      .arg( max_file )
      .arg( max_i )
      .arg( tot_i / scale_selected.size() )
      ;

   if ( do_msg )
   {
      editor_msg( "blue", msg );
   }

   return rb_scale_low->isChecked() ? min_file : max_file;
}

void US_Hydrodyn_Saxs_Hplc::scale_apply()
{
   QString target = scale_get_target( true );
   if ( target.isEmpty() )
   {
      scale_enables();
      return;
   }

   editor_msg( "blue", QString( tr( "scaling target is %1" ) ).arg( target ) );

   double q_min = le_scale_q_start->text().toDouble();
   double q_max = le_scale_q_end  ->text().toDouble();

   for ( set < QString >::iterator it = scale_selected.begin();
         it != scale_selected.end();
         ++it )
   {
      if ( q_min < scale_q[ *it ][ 0 ] )
      {
         q_min = scale_q[ *it ][ 0 ];
      }
      if ( q_max > scale_q[ *it ].back() )
      {
         q_max = scale_q[ *it ].back();
      }
   }

   le_scale_q_start->setText( QString( "%1" ).arg( q_min ) );
   le_scale_q_end  ->setText( QString( "%1" ).arg( q_max ) );

   US_Saxs_Util * usu              = ((US_Hydrodyn *)us_hydrodyn)->saxs_util;
   saxs_options * our_saxs_options = &(((US_Hydrodyn *)us_hydrodyn)->saxs_options);

   vector < double > target_q;
   vector < double > target_I;
   vector < double > target_e;

   bool target_has_e = 
      ( ( scale_q[ target ].size() == scale_e[ target ].size() ) &&
        usu->is_nonzero_vector( scale_e[ target ] ) );
      
   double       avg_std_dev_frac        = 0e0;
   unsigned int avg_std_dev_point_count = 0;

   for ( int i = 0; i < (int) scale_q[ target ].size(); ++i )
   {
      if ( scale_q[ target ][ i ] >= q_min &&
           scale_q[ target ][ i ] <= q_max )
      {
         target_q.push_back( scale_q[ target ][ i ] );
         target_I.push_back( scale_I[ target ][ i ] );
         if ( target_has_e )
         {
            target_e.push_back( scale_e[ target ][ i ] );
            if ( scale_I[ target ][ i ] )
            {
               avg_std_dev_frac += scale_e[ target ][ i ] / scale_I[ target ][ i ];
               avg_std_dev_point_count++;
            }
         }
      }
   }            

   if ( avg_std_dev_point_count )
   {
      avg_std_dev_frac /= ( double ) avg_std_dev_point_count;
   }

   if ( !target_q.size() )
   {
      editor_msg( "red", QString( tr( "Error: Scale 'apply': the target curve for scaling has no points in the selected q range" ) ) );
      scale_enables();
      return;
   }

   bool do_chi2_fitting        = our_saxs_options->iqq_scale_chi2_fitting;
   bool do_scale_linear_offset = our_saxs_options->iqq_scale_linear_offset;

   QString notices;

   if ( our_saxs_options->ignore_errors && target_has_e )
   {
      notices += tr( "Ignoring experimental errors in target\n" );
      do_chi2_fitting = false;
   }

   if ( do_chi2_fitting && !target_has_e )
   {
      notices += tr( "\nChi^2 fitting requested, but target data has no or some zero standard deviation data. Chi^2 fitting not used" );
      do_chi2_fitting = false;
   }

   if ( our_saxs_options->iqq_scale_nnls && do_chi2_fitting && our_saxs_options->iqq_scale_chi2_fitting )
   {
      notices += tr( "\nChi^2 fitting is currently not compatable with NNLS scaling, Chi^2 fitting not used" );
      do_chi2_fitting = false;
   }

   if ( our_saxs_options->iqq_scale_nnls && do_scale_linear_offset )
   {
      notices += tr("\nScale with linear offset is not compatable with NNLS scaling" );
      do_scale_linear_offset = false;
   }

   if ( !our_saxs_options->iqq_scale_nnls && do_scale_linear_offset )
   {
      notices +=  tr("\nScale with linear offset is not currently implemented");
      do_scale_linear_offset = false;
   }

   if ( !notices.isEmpty() )
   {
      editor_msg( "dark red", "Scale 'apply' notices:" + notices );
   }         
   notices = "";

   if ( do_chi2_fitting )
   {
      editor_msg( "dark blue", tr(  "Chi^2 fitting\n" ) );
   }

   map < QString, double > k;
   map < QString, double > chi2;

   for ( set < QString >::iterator it = scale_selected.begin();
         it != scale_selected.end();
         ++it )
   {
      if ( *it != target )
      {
         vector < double > source_I;
         if ( !usu->interpolate( source_I, target_q, scale_q[ *it ], scale_I[ *it ] ) )
         {
            editor_msg( "red",
                        QString( tr( "Error: Scale 'apply': %1 could not interpolate to target on specified q range: %2" ) )
                        .arg( *it )
                        .arg( usu->errormsg ) );
            scale_enables();
            return;
         }

         k   [ *it ] = 1e0;
         chi2[ *it ] = 9e99;

         if ( our_saxs_options->iqq_scale_nnls )
         {
            usu->nnls_fit( 
                          source_I, 
                          target_I, 
                          k   [ *it ], 
                          chi2[ *it ]
                           );
         } else {
            if ( do_chi2_fitting )
            {
               usu->scaling_fit( 
                                source_I, 
                                target_I, 
                                target_e,
                                k   [ *it ], 
                                chi2[ *it ]
                                 );
            } else {
               usu->scaling_fit( 
                                source_I, 
                                target_I, 
                                k   [ *it ], 
                                chi2[ *it ]
                                 );
            }
         }
      }
   }

   QString fit_msg = "";

   for ( set < QString >::iterator it = scale_selected.begin();
         it != scale_selected.end();
         ++it )
   {
      if ( *it != target )
      {
         bool source_has_e = ( scale_q[ *it ].size() == scale_e[ target ].size() );
         for ( int i = 0; i < (int) scale_I[ *it ].size(); ++i )
         {
            scale_I[ *it ][ i ] *= k[ *it ];
            if ( source_has_e )
            {
               scale_e[ *it ][ i ] *= k[ *it ];
            }
         }
         fit_msg += QString( "%1 : " ).arg( *it );
         if ( do_chi2_fitting )
         {
            // usu.calc_chisq_prob( 0.5 * target_I.size() - ( do_scale_linear_offset ? 2 : 1 ),
            // 0.5 * chi2[ *it ],
            // chi2_prob );
            fit_msg += 
               QString( "chi^2=%1 df=%2 nchi=%3" )
               .arg( chi2[ *it ], 6 )
               .arg( target_I.size() - ( do_scale_linear_offset ? 2 : 1 ) )
               .arg( sqrt( chi2[ *it ] / ( target_I.size() - ( do_scale_linear_offset ? 2 : 1 ) ) ), 5 )
               ;
            if ( avg_std_dev_frac )
            {
               fit_msg += QString( " sdf=%1 nchi*sdf=%2 " )
                  .arg( avg_std_dev_frac ) 
                  .arg( avg_std_dev_frac * sqrt( chi2[ *it ] / ( target_I.size() - ( do_scale_linear_offset ? 2 : 1 ) ) ), 5 );
            }
         } else {
            fit_msg += QString( "RMSD = %1" ).arg( chi2[ *it ], 5 );
         }
         fit_msg += "\n";
      }         
   }

   editor_msg( "dark blue", fit_msg );

   scale_replot();
   scale_applied_q_min  = q_min;
   scale_applied_q_max  = q_max;
   scale_applied_target = target;
   scale_applied        = true;

   scale_enables();
}

void US_Hydrodyn_Saxs_Hplc::scale_q_reset()
{
   le_scale_q_start->setText( QString( "%1" ).arg( f_qs[ wheel_file ][ 0 ]   ) );
   le_scale_q_end  ->setText( QString( "%1" ).arg( f_qs[ wheel_file ].back() ) );
}

void US_Hydrodyn_Saxs_Hplc::scale_reset()
{
   for ( set < QString >::iterator it = scale_selected.begin();
         it != scale_selected.end();
         ++it )
   {
      scale_q[ *it ] = f_qs    [ *it ];
      scale_I[ *it ] = f_Is    [ *it ];
      scale_e[ *it ] = f_errors[ *it ];
   }
   scale_replot();
   scale_applied = false;
   scale_enables();
}

void US_Hydrodyn_Saxs_Hplc::scale_replot()
{
   for ( set < QString >::iterator it = scale_selected.begin();
         it != scale_selected.end();
         ++it )
   {
#ifndef QT4
      plot_dist->setCurveData( plotted_curves[ *it ], 
                               (double *)&( scale_q[ *it ][ 0 ] ),
                               (double *)&( scale_I[ *it ][ 0 ] ),
                               scale_q[ *it ].size()
                               );
#else
      plotted_curves[ *it ]->setData(
                           (double *)&( scale_q[ *it ][ 0 ] ),
                           (double *)&( scale_I[ *it ][ 0 ] ),
                           scale_q[ *it ].size()
                           );
#endif
   }
   plot_dist->replot();
}

void US_Hydrodyn_Saxs_Hplc::scale_create()
{
   scale_last_created.clear();

   for ( set < QString >::iterator it = scale_selected.begin();
         it != scale_selected.end();
         ++it )
   {
      QString name = 
         QString( "%1_scaled_q%2_%3" )
         .arg( *it )
         .arg( scale_applied_q_min, 5 )
         .arg( scale_applied_q_max, 5 )
         .replace( ".", "_" );

      if ( cb_scale_save_intp->isChecked() && *it != scale_applied_target )
      {
         name += "_interp";

         double q_min = scale_q[ scale_applied_target ][ 0 ];
         double q_max = scale_q[ scale_applied_target ].back();
         if ( q_min < scale_q[ *it ][ 0 ] )
         {
            q_min = scale_q[ *it ][ 0 ];
         }
         if ( q_max > scale_q[ *it ].back() )
         {
            q_max = scale_q[ *it ].back();
         }

         vector < double > target_q;

         for ( int i = 0; i < (int) scale_q[ scale_applied_target ].size(); ++i )
         {
            if ( scale_q[ scale_applied_target ][ i ] >= q_min &&
                 scale_q[ scale_applied_target ][ i ] <= q_max )
            {
               target_q.push_back( scale_q[ scale_applied_target ][ i ] );
            }
         }            

         vector < double > source_I;
         vector < double > source_e;
         if ( !usu->interpolate( source_I, target_q, scale_q[ *it ], scale_I[ *it ] ) )
         {
            editor_msg( "red",
                        QString( tr( "Error: Scale 'apply': %1 could not interpolate to target on specified q range: %2" ) )
                        .arg( *it )
                        .arg( usu->errormsg ) );
            scale_enables();
            return;
         }

         if ( scale_e[ *it ].size() == scale_q[ *it ].size() )
         {
            if ( !usu->interpolate( source_e, target_q, scale_q[ *it ], scale_e[ *it ] ) )
            {
               editor_msg( "red",
                           QString( tr( "Error: Scale 'apply': %1 could not interpolate to target on specified q range: %2" ) )
                           .arg( *it )
                           .arg( usu->errormsg ) );
               scale_enables();
               return;
            }
         }

         add_plot( 
                  name,
                  target_q, 
                  source_I, 
                  source_e, 
                  f_is_time[ *it ], 
                  false );

      } else {
         add_plot( 
                  name,
                  scale_q[ *it ], 
                  scale_I[ *it ], 
                  scale_e[ *it ], 
                  f_is_time[ *it ], 
                  false );

      }
      scale_last_created.insert( last_created_file );
   }      

   scale_applied = false;
   scale_enables();
}

void US_Hydrodyn_Saxs_Hplc::scale_q_start_text( const QString & text )
{
#ifndef QT4
   plot_dist->setMarkerPos( plotted_markers[ 0 ], text.toDouble(), 0e0 );
#else
   plotted_markers[ 0 ]->setXValue( text.toDouble() );
#endif
   if ( qwtw_wheel->value() != text.toDouble() )
   {
      qwtw_wheel->setValue( text.toDouble() );
   }
   plot_dist->replot();
   scale_enables();
}

void US_Hydrodyn_Saxs_Hplc::scale_q_end_text( const QString & text )
{
#ifndef QT4
   plot_dist->setMarkerPos( plotted_markers[ 1 ], text.toDouble(), 0e0 );
#else
   plotted_markers[ 1 ]->setXValue( text.toDouble() );
#endif
   if ( qwtw_wheel->value() != text.toDouble() )
   {
      qwtw_wheel->setValue( text.toDouble() );
   }
   plot_dist->replot();
   scale_enables();
}

void US_Hydrodyn_Saxs_Hplc::scale_q_start_focus( bool hasFocus )
{
   if ( hasFocus )
   {
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      qwtw_wheel->setRange( f_qs[ wheel_file ][ 0 ], f_qs[ wheel_file ].back(), 
                            ( f_qs[ wheel_file ].back() - f_qs[ wheel_file ][ 0 ] ) / UHSH_WHEEL_RES );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setValue( le_scale_q_start->text().toDouble() );
      qwtw_wheel->setEnabled( true );
   }
}

void US_Hydrodyn_Saxs_Hplc::scale_q_end_focus( bool hasFocus )
{
   if ( hasFocus )
   {
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      qwtw_wheel->setRange( f_qs[ wheel_file ][ 0 ], f_qs[ wheel_file ].back(), 
                            ( f_qs[ wheel_file ].back() - f_qs[ wheel_file ][ 0 ] ) / UHSH_WHEEL_RES );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setValue( le_scale_q_end->text().toDouble() );
      qwtw_wheel->setEnabled( true );
   }
}

void US_Hydrodyn_Saxs_Hplc::guinier()
{
   qDebug( "not yet" );
}

void US_Hydrodyn_Saxs_Hplc::adjust_wheel( double pos )
{
   // cout << QString("pos is now %1 wheel step is %2\n").arg(pos, 0, 'f', 8 ).arg( qwtw_wheel->step() );
   switch ( current_mode )
   {

   case MODE_SCALE :
      {
         if ( le_scale_q_start->hasFocus() )
         {
            le_last_focus = le_scale_q_start;
         }
         if ( le_scale_q_end->hasFocus() )
         {
            le_last_focus = le_scale_q_end;
         }

         if ( !le_last_focus )
         {
            // cout << "aw: no last focus in scale mode\n";
            return;
         }

         le_last_focus->setText( QString( "%1" ).arg( pos ) );

         lbl_wheel_pos->setText( QString( "%1" ).arg( pos ) );
      }
      break;

   case MODE_GAUSSIAN :
      {
         if ( le_gauss_pos->hasFocus() )
         {
            // cout << "aw: pos focus\n";
            le_last_focus = le_gauss_pos;
         }
         if ( le_gauss_pos_width->hasFocus() )
         {
            // cout << "aw: pos width focus\n";
            le_last_focus = le_gauss_pos_width;
         }
         if ( le_gauss_pos_height->hasFocus() )
         {
            // cout << "aw: pos height focus\n";
            le_last_focus = le_gauss_pos_height;
         }
         if ( le_gauss_pos_dist1->hasFocus() )
         {
            // cout << "aw: pos dist1 focus\n";
            le_last_focus = le_gauss_pos_dist1;
         }
         if ( le_gauss_pos_dist2->hasFocus() )
         {
            // cout << "aw: pos dist2 focus\n";
            le_last_focus = le_gauss_pos_dist2;
         }
         if ( le_gauss_fit_start->hasFocus() )
         {
            // cout << "aw: fit start focus\n";
            le_last_focus = le_gauss_fit_start;
         }
         if ( le_gauss_fit_end->hasFocus() )
         {
            // cout << "aw: fit end focus\n";
            le_last_focus = le_gauss_fit_end;
         }

         if ( !le_last_focus )
         {
            le_last_focus = le_gauss_pos;
            // cout << "aw: pos focus, since no last\n";
            disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
            qwtw_wheel->setRange( f_qs[ wheel_file ][ 0 ], 
                                  f_qs[ wheel_file ].back(), 
                                  ( f_qs[ wheel_file ].back() - f_qs[ wheel_file ][ 0 ] ) / UHSH_WHEEL_RES );
            connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
            return;
         }

         le_last_focus->setText( QString( "%1" ).arg( pos ) );

         lbl_wheel_pos->setText( QString( "%1" ).arg( pos ) );

      }
      break;

   case MODE_BASELINE :
      {
         if ( le_baseline_start_s->hasFocus() )
         {
            // cout << "aw: baseline start_s focus\n";
            le_last_focus = le_baseline_start_s;
         }
         if ( le_baseline_start->hasFocus() )
         {
            // cout << "aw: baseline start focus\n";
            le_last_focus = le_baseline_start;
         }
         if ( le_baseline_start_e->hasFocus() )
         {
            // cout << "aw: baseline start_e focus\n";
            le_last_focus = le_baseline_start_e;
         }

         if ( le_baseline_end_s->hasFocus() )
         {
            // cout << "aw: baseline end_s focus\n";
            le_last_focus = le_baseline_end_s;
         }
         if ( le_baseline_end->hasFocus() )
         {
            // cout << "aw: baseline end focus\n";
            le_last_focus = le_baseline_end;
         }
         if ( le_baseline_end_e->hasFocus() )
         {
            // cout << "aw: baseline end_e focus\n";
            le_last_focus = le_baseline_end_e;
         }

         if ( !le_last_focus )
         {
            // cout << "aw: no last focus in baseline mode\n";
            return;
         }

         le_last_focus->setText( QString( "%1" ).arg( pos ) );

         lbl_wheel_pos->setText( QString( "%1" ).arg( pos ) );
      }
      break;

   case MODE_GGAUSSIAN :
      {
         if ( current_mode == MODE_GGAUSSIAN )
         {
            lbl_gauss_fit ->setText( "?" );
            plot_errors      ->clear();
            if ( !suppress_replot )
            {
               plot_errors      ->replot();
            }
            plot_errors_grid  .clear();
            plot_errors_target.clear();
            plot_errors_fit   .clear();
            plot_errors_errors.clear();
            pb_ggauss_rmsd->setEnabled( true );

            if ( le_gauss_pos->hasFocus() )
            {
               // cout << "aw: pos focus\n";
               le_last_focus = le_gauss_pos;
            }
            if ( cb_fix_width->isChecked() &&
                 le_gauss_pos_width->hasFocus() )
            {
               // cout << "aw: pos width focus\n";
               le_last_focus = le_gauss_pos_width;
            }
            if ( cb_fix_dist1->isChecked() &&
                 le_gauss_pos_dist1->hasFocus() )
            {
               // cout << "aw: pos dist1 focus\n";
               le_last_focus = le_gauss_pos_dist1;
            }
            if ( cb_fix_dist2->isChecked() &&
                 le_gauss_pos_dist2->hasFocus() )
            {
               // cout << "aw: pos dist2 focus\n";
               le_last_focus = le_gauss_pos_dist2;
            }
            if ( le_gauss_fit_start->hasFocus() )
            {
               // cout << "aw: fit start focus\n";
               le_last_focus = le_gauss_fit_start;
            }
            if ( le_gauss_fit_end->hasFocus() )
            {
               // cout << "aw: fit end focus\n";
               le_last_focus = le_gauss_fit_end;
            }

            if ( !le_last_focus )
            {
               // cout << "aw: pos focus, since no last\n";
               le_last_focus = le_gauss_pos;
            }

            le_last_focus->setText( QString( "%1" ).arg( pos ) );

            lbl_wheel_pos->setText( QString( "%1" ).arg( pos ) );
            
         } 
      }
      break;

   case MODE_TIMESHIFT :
      {
         // timeshift mode
         // adjust selected time
         pb_wheel_save->setEnabled( pos != 0e0 );

         lbl_wheel_pos->setText( QString( "%1" ).arg( pos ) );

         vector < double > offset_q = f_qs[ wheel_file ];
         for ( unsigned int i = 0; i < ( unsigned int ) offset_q.size(); i++ )
         {
            offset_q[ i ] += pos;
         }
#ifndef QT4
         plot_dist->setCurveData( wheel_curve, 
                                  /* cb_guinier->isChecked() ? (double *)&(plotted_q2[p][0]) : */
                                  (double *)&( offset_q[ 0 ] ),
                                  (double *)&( f_Is[ wheel_file ][ 0 ] ),
                                  offset_q.size()
                                  );
#else
         wheel_curve->setData(
                              /* cb_guinier->isChecked() ?
                                 (double *)&(plotted_q2[p][0]) : */
                              (double *)&( offset_q[ 0 ] ),
                              (double *)&( f_Is[ wheel_file ][ 0 ] ),
                              offset_q.size()
                              );
#endif
         if ( !suppress_replot )
         {
            plot_dist->replot();
         }
      }
      break;
   default : qDebug( "adjust wheel called in invalid mode" ); break;
   }
}

void US_Hydrodyn_Saxs_Hplc::wheel_cancel()
{
   errors_were_on = plot_errors->isVisible();
   hide_widgets( plot_errors_widgets, true );

   disable_all();
   switch ( current_mode )
   {
   case MODE_GGAUSSIAN :
      {
         f_gaussians = org_f_gaussians;
         gaussians = org_gaussians;
         gauss_delete_markers();
         plotted_markers.clear();

         if ( !suppress_replot )
         {
            plot_dist->replot();
         }
      } 
      break;

   case MODE_GAUSSIAN :
      {
         gaussians = org_gaussians;
         gauss_delete_markers();
         plotted_markers.clear();
         gauss_delete_gaussians();
         plotted_gaussians.clear();
         plotted_gaussian_sum.clear();
         if ( plotted_curves.count( wheel_file ) &&
              f_pos.count( wheel_file ) )
         {
#ifndef QT4
            plot_dist->setCurvePen( plotted_curves[ wheel_file ], QPen( plot_colors[ f_pos[ wheel_file ] % plot_colors.size()], use_line_width, SolidLine));
#else
            plotted_curves[ wheel_file ]->setPen( QPen( plot_colors[ f_pos[ wheel_file ] % plot_colors.size() ], use_line_width, Qt::SolidLine ) );
#endif
         }
      } 
      break;

   case MODE_BASELINE :
      {
         le_baseline_start_s->setText( QString( "%1" ).arg( org_baseline_start_s ) );
         le_baseline_start  ->setText( QString( "%1" ).arg( org_baseline_start   ) );
         le_baseline_start_e->setText( QString( "%1" ).arg( org_baseline_start_e ) );
         le_baseline_end_s  ->setText( QString( "%1" ).arg( org_baseline_end_s   ) );
         le_baseline_end    ->setText( QString( "%1" ).arg( org_baseline_end     ) );
         le_baseline_end_e  ->setText( QString( "%1" ).arg( org_baseline_end_e   ) );
         gauss_delete_markers();
         plotted_markers.clear();
         for ( unsigned int i = 0; i < ( unsigned int ) plotted_baseline.size(); i++ )
         {
#ifndef QT4
            plot_dist->removeCurve( plotted_baseline[ i ] );
#else
            plotted_baseline[ i ]->detach();
#endif
         }
         if ( plotted_curves.count( wheel_file ) &&
              f_pos.count( wheel_file ) )
         {
#ifndef QT4
            plot_dist->setCurvePen( plotted_curves[ wheel_file ], QPen( plot_colors[ f_pos[ wheel_file ] % plot_colors.size()], use_line_width, SolidLine));
#else
            plotted_curves[ wheel_file ]->setPen( QPen( plot_colors[ f_pos[ wheel_file ] % plot_colors.size() ], use_line_width, Qt::SolidLine ) );
#endif
         }
      }
      break;

   case MODE_TIMESHIFT :
      {
         lbl_wheel_pos->setText( QString( "%1" ).arg( 0 ) );
#ifndef QT4
         plot_dist->setCurveData( wheel_curve, 
                                  /* cb_guinier->isChecked() ? (double *)&(plotted_q2[p][0]) : */
                                  (double *)&( f_qs[ wheel_file ][ 0 ] ),
                                  (double *)&( f_Is[ wheel_file ][ 0 ] ),
                                  f_qs[ wheel_file ].size()
                                  );
#else
         wheel_curve->setData(
                              /* cb_guinier->isChecked() ?
                                 (double *)&(plotted_q2[p][0]) : */
                              (double *)&( f_qs[ wheel_file ][ 0 ] ),
                              (double *)&( f_Is[ wheel_file ][ 0 ] ),
                              f_qs[ wheel_file ].size()
                              );
#endif
      }
      break;

   case MODE_SCALE :
      {
         scale_q.clear();
         scale_I.clear();
         scale_e.clear();
         gauss_delete_markers();
         plotted_markers.clear();
         if ( scale_last_created.size() )
         {
            set_selected        ( scale_last_created );
            set_created_selected( scale_last_created );
         }
         mode_select( MODE_NORMAL );
         plot_files();
         rescale();
      }
      break;

   default : qDebug( "wheel cancel called in invalid mode" ); break;
   }

   if ( !suppress_replot )
   {
      plot_dist->replot();
   }

   // gaussian_mode         = false;
   // ggaussian_mode        = false;
   // baseline_mode         = false;
   // timeshift_mode        = false;
   mode_select( MODE_NORMAL );

   qwtw_wheel            ->setEnabled( false );
   pb_wheel_save         ->setEnabled( false );
   pb_wheel_cancel       ->setEnabled( false );

   running               = false;

   disable_all();
   update_enables();
}

void US_Hydrodyn_Saxs_Hplc::wheel_save()
{
   switch ( current_mode )
   {
   case MODE_SCALE :
      {
         qDebug( "wheel save mode scale not yet" );
         wheel_cancel();
         return;
      }
      break;

   case  MODE_GGAUSSIAN :
      {
         // org_gaussians = gaussians;
         // ? f_gaussians[ wheel_file ] = gaussians;
         if ( unified_ggaussian_ok )
         {
            unified_ggaussian_to_f_gaussians();
            /*
              if ( cb_fix_width->isChecked() )
              {
              for ( unsigned int i = 0; i < ( unsigned int ) unified_ggaussian_files.size(); i++ )
              {
              vector < double > g;
              unsigned int  index = 2 * unified_ggaussian_gaussians_size + i * unified_ggaussian_gaussians_size;

              for ( unsigned int j = 0; j < unified_ggaussian_gaussians_size; j++ )
              {
              g.push_back( unified_ggaussian_params[ index + j + 0 ] );
              g.push_back( unified_ggaussian_params[ 2 * j + 0 ] );
              g.push_back( unified_ggaussian_params[ 2 * j + 1 ] );
              }
              f_gaussians[ unified_ggaussian_files[ i ] ] = g;
              }
              } else {
              for ( unsigned int i = 0; i < ( unsigned int ) unified_ggaussian_files.size(); i++ )
              {
              vector < double > g;
              unsigned int  index = unified_ggaussian_gaussians_size + i * 2 * unified_ggaussian_gaussians_size;

              for ( unsigned int j = 0; j < unified_ggaussian_gaussians_size; j++ )
              {
              g.push_back( unified_ggaussian_params[ index + 2 * j + 0 ] );
              g.push_back( unified_ggaussian_params[ j ] );
              g.push_back( unified_ggaussian_params[ index + 2 * j + 1 ] );
              }
              f_gaussians[ unified_ggaussian_files[ i ] ] = g;
              }
              } 
            */           

            org_f_gaussians = f_gaussians;
            org_gaussians   = f_gaussians[ wheel_file ];
         }               

         wheel_cancel();
         return;
      }
      break;

   case MODE_GAUSSIAN :
      {
         org_gaussians = gaussians;
         if ( f_gaussians.count( wheel_file ) )
         {
            f_gaussians[ wheel_file ] = gaussians;
         }
         double tot_area = 0e0;
         vector < double > g_area;
         for ( unsigned int g = 0; g < (unsigned int) gaussians.size(); g += gaussian_type_size )
         {
            g_area.push_back( gaussians[ g + 0 ] * gaussians[ g + 2 ] * M_SQRT2PI );
            tot_area += g_area.back();
         }

         for ( unsigned int g = 0; g < (unsigned int) gaussians.size(); g += gaussian_type_size )
         {
            editor_msg( "darkblue",
                        QString( "Gaussian %1: center %2 height %3 width %4 area %5 % of total %6\n" ) 
                        .arg( (g/ gaussian_type_size) + 1 )
                        .arg( gaussians[ g + 1 ] )
                        .arg( gaussians[ g + 0 ] )
                        .arg( gaussians[ g + 2 ] )
                        .arg( g_area[ g/gaussian_type_size ] )
                        .arg( tot_area != 0e0 ? 100e0 * g_area[ g/gaussian_type_size ] / tot_area : 0e0 )
                        );
         }
            
         wheel_cancel();
         return;
      }
      break;

   case MODE_BASELINE :
      {
         org_baseline_start_s = le_baseline_start_s->text().toDouble();
         org_baseline_start   = le_baseline_start  ->text().toDouble();
         org_baseline_start_e = le_baseline_start_e->text().toDouble();
         org_baseline_end_s   = le_baseline_end_s  ->text().toDouble();
         org_baseline_end     = le_baseline_end    ->text().toDouble();
         org_baseline_end_e   = le_baseline_end_e  ->text().toDouble();
         wheel_cancel();
         return;
      }
      break;

   case MODE_TIMESHIFT :
      {
         qwtw_wheel            ->setEnabled( false );
         pb_wheel_save         ->setEnabled( false );
         pb_wheel_cancel       ->setEnabled( false );
         lbl_wheel_pos->setText( QString( "%1" ).arg( qwtw_wheel->value() ) );

         // save time adjusted selected as new
         map < QString, bool > current_files;

         int wheel_pos = -1;

         for ( int i = 0; i < (int)lb_files->numRows(); i++ )
         {
            current_files[ lb_files->text( i ) ] = true;
            if ( lb_files->text( i ) == wheel_file )
            {
               wheel_pos = i;
            }
         }

         QString save_name = wheel_file + QString( "_ts%1" ).arg( qwtw_wheel->value() ).replace( ".", "_" );

         int ext = 0;
         while ( current_files.count( save_name ) )
         {
            save_name = wheel_file + QString( "_ts%1-%2" ).arg( qwtw_wheel->value() ).arg( ++ext ).replace( ".", "_" );
         }
   
         // cout << QString( "new name is %1\n" ).arg( save_name );

         lb_created_files->insertItem( save_name );
         lb_created_files->setBottomItem( lb_created_files->numRows() - 1 );
         lb_files->insertItem( save_name );
         lb_files->setBottomItem( lb_files->numRows() - 1 );
         created_files_not_saved[ save_name ] = true;

         f_pos       [ save_name ] = f_qs.size();
         f_qs        [ save_name ] = f_qs        [ wheel_file ];
         f_qs_string [ save_name ] = f_qs_string [ wheel_file ];

         for ( unsigned int i = 0; i < ( unsigned int ) f_qs[ save_name ].size(); i++ )
         {
            f_qs       [ save_name ][ i ] += qwtw_wheel->value();
            f_qs_string[ save_name ][ i ] = QString( "%1" ).arg( f_qs[ save_name ][ i ] );
         }

         f_Is        [ save_name ] = f_Is        [ wheel_file ];
         f_errors    [ save_name ] = f_errors    [ wheel_file ];
         f_is_time   [ save_name ] = true;
         f_conc      [ save_name ] = f_conc.count( wheel_file ) ? f_conc[ wheel_file ] : 0e0;
         f_psv       [ save_name ] = f_psv .count( wheel_file ) ? f_psv [ wheel_file ] : 0e0;
         f_I0se      [ save_name ] = f_I0se .count( wheel_file ) ? f_I0se [ wheel_file ] : 0e0;
         {
            vector < double > tmp;
            f_gaussians  [ save_name ] = tmp;
         }

         lb_files->setSelected( f_pos[ save_name ], true );
         if ( wheel_pos != -1 )
         {
            lb_files->setSelected( wheel_pos, false );
         }

         if ( !suppress_replot )
         {
            plot_dist->replot();
         }

         qwtw_wheel            ->setEnabled( false );
         pb_wheel_save         ->setEnabled( false );
         pb_wheel_cancel       ->setEnabled( false );

         // timeshift_mode        = false;
         mode_select( MODE_NORMAL );

         running               = false;

         disable_all();
         update_enables();
      }
      break;
   default : qDebug( "wheel save called in invalid mode" ); break;
   }
}
