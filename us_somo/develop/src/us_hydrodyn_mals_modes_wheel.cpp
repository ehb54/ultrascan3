#include "../include/us_hydrodyn.h"
#include "../include/us_hydrodyn_mals.h"

#define TSO QTextStream(stdout)

void US_Hydrodyn_Mals::wheel_dec() 
{
   pb_wheel_dec->setEnabled( qwtw_wheel->value() > qwtw_wheel->minimum() );
   qwtw_wheel->setValue( qwtw_wheel->value() - qwtw_wheel->singleStep() );
}

void US_Hydrodyn_Mals::wheel_inc() 
{
   pb_wheel_inc->setEnabled( qwtw_wheel->value() < qwtw_wheel->maximum() );
   qwtw_wheel->setValue( qwtw_wheel->value() + qwtw_wheel->singleStep() );
}

void US_Hydrodyn_Mals::wheel_enables( bool enable )
{
   qwtw_wheel    ->setEnabled( enable );   
   pb_wheel_dec  ->setEnabled( enable && qwtw_wheel->value() > qwtw_wheel->minimum() );
   pb_wheel_inc  ->setEnabled( enable && qwtw_wheel->value() < qwtw_wheel->maximum() );
}

void US_Hydrodyn_Mals::adjust_wheel( double pos )
{
   if ( !qwtw_wheel->isEnabled() ) {
      wheel_enables( false );
      return;
   }

   wheel_enables();

   // TSO << QString( "adjust_wheel pos %1 focus %2 mode %3\n" ).arg( pos ).arg( (unsigned long)le_last_focus ).arg( current_mode );
   
   switch ( current_mode )
   {

   case MODE_TESTIQ :
      {
         if ( le_testiq_q_start->hasFocus() )
         {
            le_last_focus = le_testiq_q_start;
         }
         if ( le_testiq_q_end->hasFocus() )
         {
            le_last_focus = le_testiq_q_end;
         }

         if ( !le_last_focus )
         {
            // cout << "aw: no last focus in testiq mode\n";
            return;
         }

         le_last_focus->setText( QString( "%1" ).arg( pos ) );

         lbl_wheel_pos->setText( QString( "%1" ).arg( pos ) );
      }
      break;

   case MODE_GUINIER :
      {
         if ( !le_last_focus && cb_guinier_scroll->isChecked() )
         {
            // us_qdebug( QString( "guinier scroll is checked value %1" ).arg( pos ) );
            lbl_wheel_pos->setText( QString( "%1" ).arg( guinier_names[ pos ] ) );
            guinier_scroll_highlight( pos );
         } else {

            if ( le_guinier_q_start->hasFocus() )
            {
               le_last_focus = le_guinier_q_start;
            }
            if ( le_guinier_q_end->hasFocus() )
            {
               le_last_focus = le_guinier_q_end;
            }
            if ( le_guinier_q2_start->hasFocus() )
            {
               le_last_focus = le_guinier_q2_start;
            }
            if ( le_guinier_q2_end->hasFocus() )
            {
               le_last_focus = le_guinier_q2_end;
            }
            if ( le_guinier_delta_start->hasFocus() )
            {
               le_last_focus = le_guinier_delta_start;
            }
            if ( le_guinier_delta_end->hasFocus() )
            {
               le_last_focus = le_guinier_delta_end;
            }
            if ( le_guinier_rg_t_start->hasFocus() )
            {
               le_last_focus = le_guinier_rg_t_start;
            }
            if ( le_guinier_rg_t_end->hasFocus() )
            {
               le_last_focus = le_guinier_rg_t_end;
            }
            if ( le_guinier_rg_rg_start->hasFocus() )
            {
               le_last_focus = le_guinier_rg_rg_start;
            }
            if ( le_guinier_rg_rg_end->hasFocus() )
            {
               le_last_focus = le_guinier_rg_rg_end;
            }
            if ( le_guinier_mw_t_start->hasFocus() )
            {
               le_last_focus = le_guinier_mw_t_start;
            }
            if ( le_guinier_mw_t_end->hasFocus() )
            {
               le_last_focus = le_guinier_mw_t_end;
            }
            if ( le_guinier_mw_mw_start->hasFocus() )
            {
               le_last_focus = le_guinier_mw_mw_start;
            }
            if ( le_guinier_mw_mw_end->hasFocus() )
            {
               le_last_focus = le_guinier_mw_mw_end;
            }

            if ( !le_last_focus )
            {
               // us_qdebug( "aw: no last focus in guinier mode\n" );
               return;
            }

            le_last_focus->setText( QString( "%1" ).arg( pos ) );

            lbl_wheel_pos->setText( QString( "%1" ).arg( pos ) );
         }
      }
      break;

   case MODE_SCALE :
      {
         if ( !le_last_focus && cb_scale_scroll->isChecked() )
         {
            // us_qdebug( QString( "scale scroll is checked value %1" ).arg( pos ) );
            lbl_wheel_pos->setText( QString( "%1" ).arg( scale_scroll_selected[ pos ] ) );
            scale_scroll_highlight( pos );
         } else {

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
      }
      break;

   case MODE_PM :
      {
         if ( le_pm_q_start->hasFocus() )
         {
            le_last_focus = le_pm_q_start;
         }
         if ( le_pm_q_end->hasFocus() )
         {
            le_last_focus = le_pm_q_end;
         }

         if ( !le_last_focus )
         {
            // cout << "aw: no last focus in pm mode\n";
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
            qwtw_wheel->setRange( f_qs[ wheel_file ][ 0 ], f_qs[ wheel_file ].back()); qwtw_wheel->setSingleStep( ( f_qs[ wheel_file ].back() - f_qs[ wheel_file ][ 0 ] ) / UHSH_WHEEL_RES );
            connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
            return;
         }

         le_last_focus->setText( QString( "%1" ).arg( pos ) );

         lbl_wheel_pos->setText( QString( "%1" ).arg( pos ) );

      }
      break;

   case MODE_WYATT :
      {
         if ( le_wyatt_start->hasFocus() )
         {
            // cout << "aw: wyatt start focus\n";
            le_last_focus = le_wyatt_start;
         }
         if ( le_wyatt_end->hasFocus() )
         {
            // cout << "aw: wyatt end focus\n";
            le_last_focus = le_wyatt_end;
         }
         if ( cb_wyatt_2->isChecked() )
         {
            if ( le_wyatt_start2->hasFocus() )
            {
               // cout << "aw: wyatt start focus\n";
               le_last_focus = le_wyatt_start2;
            }
            if ( le_wyatt_end2->hasFocus() )
            {
               // cout << "aw: wyatt end focus\n";
               le_last_focus = le_wyatt_end2;
            }
         }
         if ( !le_last_focus )
         {
            // cout << "aw: no last focus in wyatt mode\n";
            return;
         }

         le_last_focus->setText( QString( "%1" ).arg( pos ) );

         lbl_wheel_pos->setText( QString( "%1" ).arg( pos ) );
      }
      break;

   case MODE_BLANKS :
      {
         if ( le_baseline_end_s->hasFocus() )
         {
            // cout << "aw: baseline end_s focus\n";
            le_last_focus = le_baseline_end_s;
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
         // le_last_focus->setText( QString( "%1" ).arg( pos ) );

         // lbl_wheel_pos->setText( QString( "%1" ).arg( pos ) );

         le_last_focus->setText( QString( "%1" ).arg( f_qs[ wheel_file ][ (int) pos ] ) );

         lbl_wheel_pos->setText( QString( "%1" ).arg( f_qs[ wheel_file ][ (int) pos ] ) );
      }
      break;

   case MODE_BASELINE :
      {
         if ( baseline_test_mode ) {
            return baseline_test_pos( pos );
         }

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

         if ( le_baseline_width->hasFocus() )
         {
            // cout << "aw: baseline end_e focus\n";
            le_last_focus = le_baseline_width;
            le_last_focus->setText( QString( "%1" ).arg( pos ) );

            lbl_wheel_pos->setText( QString( "%1" ).arg( pos ) );
            return;
         }

         if ( !le_last_focus )
         {
            // cout << "aw: no last focus in baseline mode\n";
            return;
         }
         // le_last_focus->setText( QString( "%1" ).arg( pos ) );

         // lbl_wheel_pos->setText( QString( "%1" ).arg( pos ) );

         le_last_focus->setText( QString( "%1" ).arg( f_qs[ wheel_file ][ (int) pos ] ) );

         lbl_wheel_pos->setText( QString( "%1" ).arg( f_qs[ wheel_file ][ (int) pos ] ) );
      }
      break;

   case MODE_GGAUSSIAN :
      {
         if ( current_mode == MODE_GGAUSSIAN )
         {
            if ( cb_ggauss_scroll->isChecked() ) {
               return ggauss_scroll_highlight( pos );
            }
            // us_qdebug( QString( "gg adjust_wheel value %1" ).arg( pos ) );
            lbl_gauss_fit ->setText( "?" );
            ggaussian_last_pfit_P    .clear( );
            ggaussian_last_pfit_N    .clear( );
            ggaussian_last_pfit_C    .clear( );
            ggaussian_last_pfit_S    .clear( );
            ggaussian_last_gg        .clear( );
            ggaussian_last_gg_t      .clear( );
            ggaussian_last_ggig      .clear( );
            ggaussian_last_I         .clear( );
            ggaussian_last_e         .clear( );
            ggaussian_pts_chi2       .clear( );
            ggaussian_pts_pfit       .clear( );
            ggaussian_pts_chi2_marked.clear( );
            ggaussian_pts_pfit_marked.clear( );

            ggqfit_plot->detachItems( QwtPlotItem::Rtti_PlotCurve ); ggqfit_plot->detachItems( QwtPlotItem::Rtti_PlotMarker );;
            ggqfit_plot   ->replot();
            plot_errors->detachItems( QwtPlotItem::Rtti_PlotCurve ); plot_errors->detachItems( QwtPlotItem::Rtti_PlotMarker );;
            if ( !suppress_replot )
            {
               plot_errors      ->replot();
            }
            plot_errors_grid  .clear( );
            plot_errors_target.clear( );
            plot_errors_fit   .clear( );
            plot_errors_errors.clear( );
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
               disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
               qwtw_wheel->setRange( f_qs[ wheel_file ][ 0 ], f_qs[ wheel_file ].back()); qwtw_wheel->setSingleStep( ( f_qs[ wheel_file ].back() - f_qs[ wheel_file ][ 0 ] ) / UHSH_WHEEL_RES );
               qwtw_wheel->setValue( le_gauss_pos->text().toDouble() );
               connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
               return;
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
#if QT_VERSION < 0x040000
         plot_dist->setCurveData( wheel_curve, 
                                  /* cb_guinier->isChecked() ? (double *)&(plotted_q2[p][0]) : */
                                  (double *)&( offset_q[ 0 ] ),
                                  (double *)&( f_Is[ wheel_file ][ 0 ] ),
                                  offset_q.size()
                                  );
#else
         wheel_curve->setSamples(
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
   default : us_qdebug( "adjust wheel called in invalid mode" ); break;
   }
}

void US_Hydrodyn_Mals::wheel_cancel( bool from_wheel_save )
{
   errors_were_on = plot_errors->isVisible();
   hide_widgets( plot_errors_widgets, true );
   cb_plot_errors_group->setChecked( true );

   clear_smoothed(); // for now always on cancel or save
   clear_oldstyle(); // for now always on cancel or save
   
   disable_all();
   switch ( current_mode )
   {
   case MODE_GGAUSSIAN :
      {
         pb_cormap->setText( us_tr( "PVP Analysis" ) );
         f_gaussians = org_f_gaussians;
         gaussians = org_gaussians;
         gauss_delete_markers();
         if ( cb_ggauss_scroll->isChecked() ) {
            cb_ggauss_scroll->setChecked( false );
            disable_updates = true;
            lb_files->clearSelection();
            for ( int i = 0; i < (int) ggaussian_selected_file_index.size(); ++i ) {
               lb_files->item( ggaussian_selected_file_index[ i ])->setSelected( true );
            }
            disable_updates = false;
            plot_files();
         } else {
            if ( !suppress_replot )
            {
               plot_dist->replot();
            }
         }
      } 
      break;

   case MODE_GAUSSIAN :
      {
         gaussians = org_gaussians;
         gauss_delete_markers();
         gauss_delete_gaussians();
         plotted_gaussians.clear( );
         plotted_gaussian_sum.clear( );
         if ( plotted_curves.count( wheel_file ) &&
              f_pos.count( wheel_file ) )
         {
#if QT_VERSION < 0x040000
            plot_dist->setCurvePen( plotted_curves[ wheel_file ], QPen( plot_colors[ f_pos[ wheel_file ] % plot_colors.size()], use_line_width, SolidLine));
#else
            plotted_curves[ wheel_file ]->setPen( QPen( plot_colors[ f_pos[ wheel_file ] % plot_colors.size() ], use_line_width, Qt::SolidLine ) );
#endif
         }
      } 
      break;

   case MODE_WYATT :
      {
         le_wyatt_start  ->setText( QString( "%1" ).arg( org_wyatt_start   ) );
         le_wyatt_end    ->setText( QString( "%1" ).arg( org_wyatt_end     ) );
         le_wyatt_start2 ->setText( QString( "%1" ).arg( org_wyatt_start2  ) );
         le_wyatt_end2   ->setText( QString( "%1" ).arg( org_wyatt_end2    ) );
         cb_wyatt_2      ->setChecked( org_wyatt_2 );
         gauss_delete_markers();
         for ( unsigned int i = 0; i < ( unsigned int ) plotted_wyatt.size(); i++ )
         {
#if QT_VERSION < 0x040000
            plot_dist->removeCurve( plotted_wyatt[ i ] );
#else
            plotted_wyatt[ i ]->detach();
#endif
         }
         //          if ( plotted_curves.count( wheel_file ) &&
         //               f_pos.count( wheel_file ) )
         //          {
         //             QwtSymbol symbol;
         //             symbol.setStyle( QwtSymbol::None );

         // #if QT_VERSION < 0x040000
         //             plot_dist->setCurvePen( plotted_curves[ wheel_file ], QPen( plot_colors[ f_pos[ wheel_file ] % plot_colors.size()], use_line_width, Qt::SolidLine));
         //             plot_dist->setCurveSymbol( plotted_curves[ wheel_file ], symbol );
         // #else
         //             plotted_curves[ wheel_file ]->setPen( QPen( plot_colors[ f_pos[ wheel_file ] % plot_colors.size() ], use_line_width, Qt::SolidLine ) );
         //             plotted_curves[ wheel_file ]->setSymbol( new QwtSymbol( symbol.style(), symbol.brush(), symbol.pen(), symbol.size() ) );
         // #endif
         //          }
         plot_files();
      }
      break;

   case MODE_BLANKS :
      {
         // us_qdebug( "wheel cancel in blanks" );
         le_baseline_end_s  ->setText( QString( "%1" ).arg( org_baseline_end_s   ) );
         le_baseline_end_e  ->setText( QString( "%1" ).arg( org_baseline_end_e   ) );
         gauss_delete_markers();
         remove_files( blanks_created_set );
         remove_files( blanks_temporary_load_set );
         if ( blanks_axis_y_was_log != axis_y_log ) {
            axis_y();
         }
         set_selected( blanks_save_selected_set );
         if ( cb_eb->isChecked() ) {
            cb_eb->setChecked( false );
         }
         rescale();
         if ( blanks_in_baseline_mode &&
              blanks_cormap_parameters.size() &&
              blanks_cormap_pvaluepairs.size() &&
              blanks_brookesmap_sliding_results.size() ) {
            baseline_start( from_wheel_save );
            return;
         }
      }
      break;

   case MODE_BASELINE :
      {
         // us_qdebug( "wheel cancel in baseline" );
         if ( baseline_test_mode ) {
            lbl_wheel_pos_below->setText( "" );
            baseline_test_mode = false;
            remove_files( baseline_test_added_set );
            set_selected( baseline_selected_set );
            baseline_init_markers();
            replot_baseline( "cancel from baseline test" );
            return baseline_enables();
         }

         disconnect( le_baseline_start_s, SIGNAL( textChanged( const QString & ) ), 0, 0 );
         le_baseline_start_s->setText( QString( "%1" ).arg( org_baseline_start_s ) );
         connect( le_baseline_start_s, SIGNAL( textChanged( const QString & ) ), SLOT( baseline_start_s_text( const QString & ) ) );

         disconnect( le_baseline_start, SIGNAL( textChanged( const QString & ) ), 0, 0 );
         le_baseline_start  ->setText( QString( "%1" ).arg( org_baseline_start   ) );
         connect( le_baseline_start, SIGNAL( textChanged( const QString & ) ), SLOT( baseline_start_text( const QString & ) ) );

         disconnect( le_baseline_start_e, SIGNAL( textChanged( const QString & ) ), 0, 0 );
         le_baseline_start_e->setText( QString( "%1" ).arg( org_baseline_start_e ) );
         connect( le_baseline_start_e, SIGNAL( textChanged( const QString & ) ), SLOT( baseline_start_e_text( const QString & ) ) );

         disconnect( le_baseline_end_s, SIGNAL( textChanged( const QString & ) ), 0, 0 );
         le_baseline_end_s  ->setText( QString( "%1" ).arg( org_baseline_end_s   ) );
         connect( le_baseline_end_s, SIGNAL( textChanged( const QString & ) ), SLOT( baseline_end_s_text( const QString & ) ) );

         disconnect( le_baseline_end, SIGNAL( textChanged( const QString & ) ), 0, 0 );
         le_baseline_end    ->setText( QString( "%1" ).arg( org_baseline_end     ) );
         connect( le_baseline_end, SIGNAL( textChanged( const QString & ) ), SLOT( baseline_end_text( const QString & ) ) );

         disconnect( le_baseline_end_e, SIGNAL( textChanged( const QString & ) ), 0, 0 );
         le_baseline_end_e  ->setText( QString( "%1" ).arg( org_baseline_end_e   ) );
         connect( le_baseline_end_e, SIGNAL( textChanged( const QString & ) ), SLOT( baseline_end_e_text( const QString & ) ) );

         gauss_delete_markers();

         if ( plotted_curves.count( wheel_file ) &&
              f_pos.count( wheel_file ) )
         {
#if QT_VERSION < 0x040000
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
#if QT_VERSION < 0x040000
         plot_dist->setCurveData( wheel_curve, 
                                  /* cb_guinier->isChecked() ? (double *)&(plotted_q2[p][0]) : */
                                  (double *)&( f_qs[ wheel_file ][ 0 ] ),
                                  (double *)&( f_Is[ wheel_file ][ 0 ] ),
                                  f_qs[ wheel_file ].size()
                                  );
#else
         wheel_curve->setSamples(
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
         scale_selected_names.clear( );
         scale_q.clear( );
         scale_I.clear( );
         scale_e.clear( );
         gauss_delete_markers();
         if ( scale_last_created.size() )
         {
            set_selected        ( scale_last_created );
            set_created_selected( scale_last_created );
         }
         plot_files();
         if ( testiq_active )
         {
            remove_files( testiq_created_scale_names );
            set_selected( testiq_original_selection );
            testiq_created_scale_names.clear( );
            testiq_original_selection.clear( );
            mode_select( MODE_TESTIQ );
            return wheel_cancel();
         }
         mode_select( MODE_NORMAL );
         rescale();
      }
      break;

   case MODE_TESTIQ :
      {
         testiq_created_names.clear( );
         testiq_created_q    .clear( );
         testiq_created_I    .clear( );
         testiq_created_e    .clear( );

         for ( int i = 0; i < (int) rb_testiq_gaussians.size(); ++i )
         {
#if QT_VERSION < 0x040000
            hbl_testiq_gaussians->removeWidget( rb_testiq_gaussians[ i ] );
#else
            hbl_testiq_gaussians->removeWidget( rb_testiq_gaussians[ i ] );
#endif
            delete rb_testiq_gaussians[ i ];
         }
         rb_testiq_gaussians.clear( );

         gauss_delete_markers();
         mode_select( MODE_NORMAL );
         plot_files();
         rescale();
         testiq_active = false;
      }
      break;

   case MODE_GUINIER :
      {
         guinier_q           .clear( );
         guinier_q2          .clear( );
         guinier_I           .clear( );
         guinier_e           .clear( );
         guinier_x           .clear( );
         guinier_y           .clear( );
         guinier_a           .clear( );
         guinier_b           .clear( );
         guinier_colors      .clear( );

         guinier_report      .clear( );
         guinier_delete_markers();
         guinier_markers.clear( );
         guinier_curves.clear( );
         guinier_errorbar_curves.clear( );
         plot_dist->show();
         if ( testiq_active )
         {
            mode_select( MODE_TESTIQ );
            return wheel_cancel();
         }
         mode_select( MODE_NORMAL );

         // plot_files();
         // rescale();
      }
      break;

   case MODE_PM :
      {
         gauss_delete_markers();
         mode_select( MODE_NORMAL );
         plot_files();
         rescale();
      }
      break;

   case MODE_RGC :
      break;

   default : us_qdebug( "wheel cancel called in invalid mode" ); break;
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

   wheel_enables( false );
   pb_wheel_save         ->setEnabled( false );
   pb_wheel_cancel       ->setEnabled( false );

   running               = false;

   disable_all();
   update_enables();
}

void US_Hydrodyn_Mals::wheel_save()
{
   clear_smoothed(); // for now always on cancel or save
   clear_oldstyle(); // for now always on cancel or save

   switch ( current_mode )
   {
   case MODE_SCALE :
      {
         // us_qdebug( "wheel save mode scale not yet" );
         wheel_cancel( true );
         return;
      }
      break;

   case MODE_TESTIQ :
      {
         // us_qdebug( "wheel save mode testiq not yet" );
         wheel_cancel( true );
         return;
      }
      break;

   case MODE_GUINIER :
      {
         // us_qdebug( "wheel save mode scale not yet" );
         wheel_cancel( true );
         return;
      }
      break;

   case MODE_PM :
      {
         us_qdebug( "wheel save mode pm not yet" );
         wheel_cancel( true );
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

         wheel_cancel( true );
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
            
         wheel_cancel( true );
         return;
      }
      break;

   case MODE_WYATT :
      {
         org_wyatt_start   = le_wyatt_start  ->text().toDouble();
         org_wyatt_end     = le_wyatt_end    ->text().toDouble();
         org_wyatt_start2  = le_wyatt_start2 ->text().toDouble();
         org_wyatt_end2    = le_wyatt_end2   ->text().toDouble();
         org_wyatt_2       = cb_wyatt_2      ->isChecked();
         wheel_cancel( true );
         return;
      }
      break;

   case MODE_BLANKS :
      {
         blanks_last_q_range = f_qs[ wheel_file ];
         blanks_end_s  = le_baseline_end_s  ->text().toDouble();
         blanks_end_e  = le_baseline_end_e  ->text().toDouble();
         default_blanks     = blanks_selected;
         default_blanks_set = blanks_selected_set;
         default_blanks_files.clear( );
         blanks_cormap_parameters  = blanks_last_cormap_parameters;
         blanks_cormap_pvaluepairs = blanks_last_cormap_pvaluepairs;
         // do sliding window analysis
         blanks_brookesmap_sliding_results.clear( );
         if ( blanks_cormap_parameters.size() &&
              blanks_cormap_pvaluepairs.size() ) {
            US_Hydrodyn_Saxs_Cormap_Cluster_Analysis ca;
            if ( ca.sliding( 
                            blanks_cormap_pvaluepairs, 
                            blanks_cormap_parameters, 
                            blanks_brookesmap_sliding_results, 
                            this,
                            progress ) ) {
               // us_qdebug( "sliding_results" );
               // for ( map < QString, double >::iterator it = blanks_brookesmap_sliding_results.begin();
               //       it != blanks_brookesmap_sliding_results.end();
               //       ++it ) {
               //    cout << "\t" << it->first << " => " << it->second << endl;
               // }
            }
         }

         for ( int i = 0; i < ( int ) blanks_selected.size(); ++i ) {
            if ( f_name.count( blanks_selected[ i ] ) ) {
               default_blanks_files.push_back( f_name[ blanks_selected[ i ] ] );
            } else {
               editor_msg( "red", us_tr( "Internal error: mode_blanks closing, missing blanks selected" ) );
               default_blanks      .clear( );
               default_blanks_set  .clear( );
               default_blanks_files.clear( );
               break;
            }
         }
         wheel_cancel( true );
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
         baseline_ready_to_apply = true;
         wheel_cancel( true );
         return;
      }
      break;

   case MODE_TIMESHIFT :
      {
         wheel_enables( false );
         pb_wheel_save         ->setEnabled( false );
         pb_wheel_cancel       ->setEnabled( false );
         lbl_wheel_pos->setText( QString( "%1" ).arg( qwtw_wheel->value() ) );

         // save time adjusted selected as new
         map < QString, bool > current_files;

         int wheel_pos = -1;

         for ( int i = 0; i < (int)lb_files->count(); i++ )
         {
            current_files[ lb_files->item( i )->text() ] = true;
            if ( lb_files->item( i )->text() == wheel_file )
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

         lb_created_files->addItem( save_name );
         lb_created_files->scrollToItem( lb_created_files->item( lb_created_files->count() - 1 ) );
         lb_files->addItem( save_name );
         lb_files->scrollToItem( lb_files->item( lb_files->count() - 1 ) );
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

         conc_files.insert( save_name );
         editor_msg( "gray", QString( "Created %1\n" ).arg( save_name ) );

         lb_files->item( f_pos[ save_name ])->setSelected( true );
         if ( wheel_pos != -1 )
         {
            lb_files->item( wheel_pos)->setSelected( false );
         }

         if ( !suppress_replot )
         {
            plot_dist->replot();
         }

         wheel_enables( false );
         pb_wheel_save         ->setEnabled( false );
         pb_wheel_cancel       ->setEnabled( false );

         // timeshift_mode        = false;
         mode_select( MODE_NORMAL );

         running               = false;

         disable_all();
         if ( QMessageBox::Yes == QMessageBox::question(
                                                        this,
                                                        windowTitle() + us_tr( ": Timeshift : set concentration file" ),
                                                        us_tr("Would you like to *set* the timeshifted concentration file?" ),
                                                        QMessageBox::Yes, 
                                                        QMessageBox::No | QMessageBox::Default
                                                        ) )
         {
            return set_conc_file( save_name );
         }
         update_enables();
      }
      break;
   default : us_qdebug( "wheel save called in invalid mode" ); break;
   }
}

void US_Hydrodyn_Mals::wheel_pressed() {
   // qDebug() << "wheel_pressed()";
   wheel_is_pressed = true;
}

void US_Hydrodyn_Mals::wheel_released() {
   // qDebug() << "wheel_released()";
   wheel_is_pressed = false;
}
