#include "../include/us3_defines.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_hydrodyn_saxs_hplc.h"
#include "../include/us_pm.h"

// --- PM ----

void US_Hydrodyn_Saxs_Hplc::pm()
{
   le_last_focus = (mQLineEdit *) 0;

   bool any_selected = false;

   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      if ( lb_files->isSelected( i ) )
      {
         if ( !any_selected )
         {
            wheel_file = lb_files->text( i );
            any_selected = true;
            break;
         }
      }
   }

   if ( !any_selected )
   {
      editor_msg( "red", tr( "Internal error: no files selected in pm mode" ) );
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

   if ( le_pm_q_start->text().isEmpty() ||
        le_pm_q_start->text() == "0" ||
        le_pm_q_start->text().toDouble() < f_qs[ wheel_file ][ 0 ] )
   {
      disconnect( le_pm_q_start, SIGNAL( textChanged( const QString & ) ), 0, 0 );
      le_pm_q_start->setText( QString( "%1" ).arg( f_qs[ wheel_file ][ 0 ] ) );
      connect( le_pm_q_start, SIGNAL( textChanged( const QString & ) ), SLOT( pm_q_start_text( const QString & ) ) );
   }

   if ( le_pm_q_end->text().isEmpty() ||
        le_pm_q_end->text() == "0" ||
        le_pm_q_end->text().toDouble() > f_qs[ wheel_file ].back() )
   {
      disconnect( le_pm_q_end, SIGNAL( textChanged( const QString & ) ), 0, 0 );
      le_pm_q_end->setText( QString( "%1" ).arg( f_qs[ wheel_file ].back() ) );
      connect( le_pm_q_end, SIGNAL( textChanged( const QString & ) ), SLOT( pm_q_end_text( const QString & ) ) );
   }

   disable_all();
   mode_select( MODE_PM );
   pb_rescale     ->setEnabled( true );
   pb_axis_x      ->setEnabled( true );
   pb_axis_y      ->setEnabled( true );

   if ( f_errors.count( wheel_file ) && f_errors[ wheel_file ].size() == f_qs[ wheel_file ].size() )
   {
      cb_pm_sd->setChecked( true );
      cb_pm_sd->show();
   } else {
      cb_pm_sd->setChecked( false );
      cb_pm_sd->hide();
   }
   cb_pm_q_logbin->hide();

   running       = true;

   pm_enables();

   plotted_markers.clear();
   gauss_add_marker( le_pm_q_start  ->text().toDouble(), Qt::red, tr( "Start" ) );
   gauss_add_marker( le_pm_q_end    ->text().toDouble(), Qt::red, tr( "End"   ), Qt::AlignLeft | Qt::AlignTop );
   plot_dist->replot();
}

void US_Hydrodyn_Saxs_Hplc::pm_enables()
{
   bool shapes_selected = false;
#ifdef QT4
   for ( int i = 0; i < (int) bg_pm_shape->buttons().size(); ++i )
   {
      if ( ((QRadioButton *)bg_pm_shape->buttons()[ i ]->isChecked() ) )
      {
         shapes_selected = true;
      }
   }
#else
   for ( int i = 0; i < (int) bg_pm_shape->count(); ++i )
   {
      if ( ((QRadioButton *)bg_pm_shape->find( i ))->isChecked() )
      {
         shapes_selected = true;
      }
   }
#endif
   pb_wheel_start        ->setEnabled( false );
   pb_wheel_cancel       ->setEnabled( true );
   le_pm_q_start         ->setEnabled( true );
   le_pm_q_end           ->setEnabled( true );
   le_pm_q_pts           ->setEnabled( true );
   le_pm_grid_size       ->setEnabled( true );
   le_pm_samp_e_dens     ->setEnabled( true );
   le_pm_buff_e_dens     ->setEnabled( true );
   pb_pm_q_reset         ->setEnabled( true );
   pb_pm_run             ->setEnabled( shapes_selected );

   model_enables();
}

void US_Hydrodyn_Saxs_Hplc::pm_run()
{
   disable_all();
   qApp->processEvents();

   map < QString, QString > run_params;
   map < QString, vector < double > > run_vectors;

   vector < double > use_q;
   vector < double > use_I;
   vector < double > use_e;

   double q_min = le_pm_q_start->text().toDouble();
   double q_max = le_pm_q_end  ->text().toDouble();

   int pts     = le_pm_q_pts->text().toInt();

   int pts_cnt = 0;

   qDebug( "pm_run 0" );
   if ( cb_pm_sd->isChecked() )
   {
      for ( int i = 0; i < (int) f_qs[ wheel_file ].size(); ++i )
      {
         if ( f_qs[ wheel_file ][ i ] >= q_min &&
              f_qs[ wheel_file ][ i ] <= q_max )
         {
            if ( !(pts_cnt++ % pts ) )
            {
               use_q.push_back( f_qs    [ wheel_file ][ i ] );
               use_I.push_back( f_Is    [ wheel_file ][ i ] );
               use_e.push_back( f_errors[ wheel_file ][ i ] );
            }
         }
      }
   } else {
      for ( int i = 0; i < (int) f_qs[ wheel_file ].size(); ++i )
      {
         if ( f_qs[ wheel_file ][ i ] >= q_min &&
              f_qs[ wheel_file ][ i ] <= q_max )
         {
            if ( !(pts_cnt++ % pts ) )
            {
               use_q.push_back( f_qs[ wheel_file ][ i ] );
               use_I.push_back( f_Is[ wheel_file ][ i ] );
            }
         }
      }
   }      

   if ( !use_q.size() )
   {
      editor_msg( "red", tr( "PM: no points left in curve" ) );
      pm_enables();
      return;
   }

   qDebug( "pm_run 1" );
   editor_msg( "blue", QString( tr( "PM: actual q points used %1 q range [%2:%3]" ) ).arg( use_q.size() ).arg( use_q[ 0 ] ).arg( use_q.back() ) );

   run_params[ "pmgridsize"           ] = le_pm_grid_size  ->text();
   run_params[ "pmminq"               ] = le_pm_q_start    ->text();
   run_params[ "pmmaxq"               ] = le_pm_q_end      ->text();

   run_params[ "pmbufferedensity"     ] = le_pm_buff_e_dens->text();
   run_params[ "pmrayleighdrho"       ] = le_pm_samp_e_dens->text();
   
   run_params[ "pmapproxmaxdimension" ] = "true";

   run_params[ "pmoutname"            ] = wheel_file + "_pm";
   
   qDebug( "pm_run 2" );

   if ( rb_pm_shape_sphere->isChecked() )
   {
      run_vectors[ "pmtypes" ].push_back( US_PM::SPHERE );
   }
   if ( rb_pm_shape_spheroid->isChecked() )
   {
      run_vectors[ "pmtypes" ].push_back( US_PM::SPHEROID );
   }
   if ( rb_pm_shape_ellipsoid->isChecked() )
   {
      run_vectors[ "pmtypes" ].push_back( US_PM::ELLIPSOID );
   }
   if ( rb_pm_shape_cylinder->isChecked() )
   {
      run_vectors[ "pmtypes" ].push_back( US_PM::CYLINDER );
   }
   if ( rb_pm_shape_torus->isChecked() )
   {
      run_vectors[ "pmtypes" ].push_back( US_PM::TORUS );
   }
   
   run_vectors[ "pmq" ] = use_q;
   run_vectors[ "pmi" ] = use_I;
   if ( cb_pm_sd->isChecked() )
   {
      run_vectors[ "pme" ] = use_e;
   }

   //   run_params[ "pmbestmd0" ] = "";
   run_params[ "pmbestga" ] = "";

   qDebug( "pm_run 3" );
   US_Saxs_Util usu;
   map < QString, vector < double > > produced_q;
   map < QString, vector < double > > produced_I;
   map < QString, QString >           produced_model;
   if ( !usu.run_pm( 
                    produced_q, 
                    produced_I,
                    produced_model,
                    run_params, 
                    run_vectors
                     ) )
   {
      editor_msg( "red", usu.errormsg );
   }
   qDebug( "pm_run 4" );
   for ( map < QString, vector < double > >::iterator it = produced_q.begin();
         it != produced_q.end();
         ++it )
   {
      add_plot( it->first, it->second, produced_I[ it->first ], false, false );
      lb_model_files->insertItem( last_created_file );
      models[ last_created_file ] = produced_model[ it->first ];
      models_not_saved.insert( last_created_file );
   }
   qDebug( "pm_run 5" );
   pm_enables();
   // add to produced
}

void US_Hydrodyn_Saxs_Hplc::pm_q_reset()
{
   le_pm_q_start->setText( QString( "%1" ).arg( f_qs[ wheel_file ][ 0 ]   ) );
   le_pm_q_end  ->setText( QString( "%1" ).arg( f_qs[ wheel_file ].back() ) );
}

void US_Hydrodyn_Saxs_Hplc::pm_grid_size_text( const QString & )
{
}

void US_Hydrodyn_Saxs_Hplc::pm_buff_e_dens_text( const QString & )
{
}

void US_Hydrodyn_Saxs_Hplc::pm_samp_e_dens_text( const QString & )
{
}

void US_Hydrodyn_Saxs_Hplc::pm_q_pts_text( const QString & )
{
}

void US_Hydrodyn_Saxs_Hplc::pm_q_start_text( const QString & text )
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
   pm_enables();
}

void US_Hydrodyn_Saxs_Hplc::pm_q_end_text( const QString & text )
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
   pm_enables();
}

void US_Hydrodyn_Saxs_Hplc::pm_q_start_focus( bool hasFocus )
{
   if ( hasFocus )
   {
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      qwtw_wheel->setRange( f_qs[ wheel_file ][ 0 ], f_qs[ wheel_file ].back(), 
                            ( f_qs[ wheel_file ].back() - f_qs[ wheel_file ][ 0 ] ) / UHSH_WHEEL_RES );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setValue( le_pm_q_start->text().toDouble() );
      qwtw_wheel->setEnabled( true );
   }
}

void US_Hydrodyn_Saxs_Hplc::pm_q_end_focus( bool hasFocus )
{
   if ( hasFocus )
   {
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      qwtw_wheel->setRange( f_qs[ wheel_file ][ 0 ], f_qs[ wheel_file ].back(), 
                            ( f_qs[ wheel_file ].back() - f_qs[ wheel_file ][ 0 ] ) / UHSH_WHEEL_RES );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setValue( le_pm_q_end->text().toDouble() );
      qwtw_wheel->setEnabled( true );
   }
}

// --- RGC ---

void US_Hydrodyn_Saxs_Hplc::rgc()
{

   disable_all();
   mode_select( MODE_RGC );
   running       = true;
   rgc_enables();
}

void US_Hydrodyn_Saxs_Hplc::rgc_enables()
{
   pb_rgc                ->setEnabled( false );
   pb_wheel_cancel       ->setEnabled( true );

   if ( rb_rgc_shape_ellipsoid->isChecked() )
   {
      le_rgc_axis_b->show();
      le_rgc_axis_c->show();
   } else {
      if ( rb_rgc_shape_oblate->isChecked() ||
           rb_rgc_shape_prolate->isChecked() )
      {
         le_rgc_axis_b->show();
         le_rgc_axis_c->hide();
      } else {
         le_rgc_axis_b->hide();
         le_rgc_axis_c->hide();
      }
   }
}

void US_Hydrodyn_Saxs_Hplc::rgc_shape()
{
   if ( rb_rgc_shape_ellipsoid->isChecked() )
   {
      lbl_rgc_axis->setText( tr( " Relative axes 1 > b > c:" ) );
   } else {
      if ( rb_rgc_shape_oblate->isChecked() ||
           rb_rgc_shape_prolate->isChecked() )
      {
         lbl_rgc_axis->setText( tr( " Relative axis 1 > b:" ) );
      } else {
         lbl_rgc_axis->setText( "" );
      }
   }
   rgc_enables();
   rgc_calc_rg();
}

void US_Hydrodyn_Saxs_Hplc::rgc_calc_rg()
{
   
   double V = 0e0;
   double rho = le_rgc_rho->text().toDouble();
   if ( rho )
   {
      V = ( 1e3 * le_rgc_mw->text().toDouble() / AVOGADRO ) / rho;
   }
   // qDebug( QString( "volume %1" ).arg( V ) );

   double abc4o3pi = M_PI * 4e0 / 3e0;

   if ( rb_rgc_shape_ellipsoid->isChecked() )
   {
      abc4o3pi *= le_rgc_axis_b->text().toDouble() * le_rgc_axis_c->text().toDouble();
   } else {
      if ( rb_rgc_shape_oblate->isChecked() )
      {
         abc4o3pi *= le_rgc_axis_b->text().toDouble();
      } else {
         if ( rb_rgc_shape_prolate->isChecked() )
         {
            abc4o3pi *= le_rgc_axis_b->text().toDouble() * le_rgc_axis_b->text().toDouble();
         } 
      }
   }

   double extent_a = 0e0;
   if ( abc4o3pi )
   {
      extent_a = pow( V / abc4o3pi, (1e0/3e0) );
   }

   disconnect( le_rgc_vol, SIGNAL( textChanged( const QString & ) ), 0, 0 );
   le_rgc_vol->setText( QString( "%1" ).arg( V * 1e8 * 1e8 * 1e8 ) );
   connect( le_rgc_vol, SIGNAL( textChanged( const QString & ) ), SLOT( rgc_vol_text( const QString & ) ) );

   // qDebug( QString( "extent_a %1" ).arg( extent_a ) );
           
   // ? is this really correct or should it be 2/5 instead of 3/5
   double Rg = sqrt( 3e0/5e0 ) * extent_a;
   le_rgc_rg->setText( QString( "%1" ).arg( Rg * 1e8 ) );

   double extent_b = extent_a;
   double extent_c = extent_a;

   if ( rb_rgc_shape_ellipsoid->isChecked() )
   {
      extent_b *= le_rgc_axis_b->text().toDouble();
      extent_c *= le_rgc_axis_c->text().toDouble();
   } else {
      if ( rb_rgc_shape_oblate->isChecked() )
      {
         extent_c *= le_rgc_axis_b->text().toDouble();
      } else  {
         if ( rb_rgc_shape_prolate->isChecked() )
         {
            extent_b *= le_rgc_axis_b->text().toDouble();
            extent_c *= le_rgc_axis_b->text().toDouble();
         }
      }
   }
   
   le_rgc_extents->setText( QString( "%1 : %2 : %3" ).arg( extent_a * 1e8 ).arg( extent_b * 1e8 ).arg( extent_c * 1e8 ) );
}

void US_Hydrodyn_Saxs_Hplc::rgc_mw_text( const QString & )
{
   rgc_calc_rg();
}

void US_Hydrodyn_Saxs_Hplc::rgc_vol_text( const QString & )
{
   double V = le_rgc_vol->text().toDouble();
   double rho = 0e0; 
   if ( V )
   {
      rho = ( 1e3 * le_rgc_mw->text().toDouble() / AVOGADRO ) / V;
   }

   disconnect( le_rgc_rho, SIGNAL( textChanged( const QString & ) ), 0, 0 );
   le_rgc_rho->setText( QString( "%1" ).arg( rho ) );
   connect( le_rgc_rho, SIGNAL( textChanged( const QString & ) ), SLOT( rgc_rho_text( const QString & ) ) );
   
   rgc_calc_rg();
}

void US_Hydrodyn_Saxs_Hplc::rgc_rho_text( const QString & )
{
   rgc_calc_rg();
}

void US_Hydrodyn_Saxs_Hplc::rgc_axis_b_text( const QString &, bool do_recompute )
{
   if ( le_rgc_axis_b->text().toDouble() < le_rgc_axis_c->text().toDouble() )
   {
      le_rgc_axis_c->setText( le_rgc_axis_b->text() );
   }
   if ( do_recompute )
   {
      rgc_calc_rg();
   }
}

void US_Hydrodyn_Saxs_Hplc::rgc_axis_c_text( const QString &, bool do_recompute )
{
   if ( le_rgc_axis_b->text().toDouble() < le_rgc_axis_c->text().toDouble() )
   {
      le_rgc_axis_b->setText( le_rgc_axis_c->text() );
   }
   if ( do_recompute )
   {
      rgc_calc_rg();
   }
}

void US_Hydrodyn_Saxs_Hplc::rgc_rg_text( const QString & )
{
}

// --- TESTIQ ----

class testiq_sortable_double {
public:
   double       x;
   double       Imin;
   double       Imax;
   QString      index;
   bool operator < (const testiq_sortable_double& objIn) const
   {
      return x < objIn.x;
   }
};


void US_Hydrodyn_Saxs_Hplc::testiq()
{
   le_last_focus = (mQLineEdit *) 0;

   bool any_selected = false;

   testiq_selected    .clear();

   bool do_rescale = false;


   if ( current_mode == MODE_SCALE )
   {
      remove_files( testiq_created_scale_names );
      set_selected( testiq_original_selection );
      testiq_created_scale_names.clear();
      testiq_original_selection.clear();
      plot_files();
      do_rescale = true;
   }

   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      if ( lb_files->isSelected( i ) )
      {
         if ( !any_selected )
         {
            wheel_file = lb_files->text( i );
         }
         testiq_selected.insert( lb_files->text( i ) );
         any_selected = true;
         if ( !plotted_curves.count( lb_files->text( i ) ) )
         {
            editor_msg( "red", QString( tr( "Internal error: testiq selected %1, but no plotted curve found" ) ).arg( lb_files->text( i ) ) );
            return;
         }
      }
   }

   if ( !any_selected )
   {
      editor_msg( "red", tr( "Internal error: no files selected in testiq mode" ) );
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

   if ( le_testiq_q_start->text().isEmpty() ||
        le_testiq_q_start->text() == "0" ||
        le_testiq_q_start->text().toDouble() < f_qs[ wheel_file ][ 0 ] )
   {
      disconnect( le_testiq_q_start, SIGNAL( textChanged( const QString & ) ), 0, 0 );
      le_testiq_q_start->setText( QString( "%1" ).arg( f_qs[ wheel_file ][ 0 ] ) );
      connect( le_testiq_q_start, SIGNAL( textChanged( const QString & ) ), SLOT( testiq_q_start_text( const QString & ) ) );
   }

   if ( le_testiq_q_end->text().isEmpty() ||
        le_testiq_q_end->text() == "0" ||
        le_testiq_q_end->text().toDouble() > f_qs[ wheel_file ].back() )
   {
      disconnect( le_testiq_q_end, SIGNAL( textChanged( const QString & ) ), 0, 0 );
      le_testiq_q_end->setText( QString( "%1" ).arg( f_qs[ wheel_file ].back() ) );
      connect( le_testiq_q_end, SIGNAL( textChanged( const QString & ) ), SLOT( testiq_q_end_text( const QString & ) ) );
   }

   disable_all();
   // plotted_markers.clear();
   if ( current_mode == MODE_GUINIER ||
        current_mode == MODE_SCALE )
   {
      gauss_delete_markers();
      plotted_markers.clear();
      plot_dist->show();
   } else {
      testiq_it_selected = "";
      
      // figure out which testiq_it to select
      {
         list < testiq_sortable_double > tsdl;
         double avg_i = 0e0;
         double max_i = 0e0;

         for ( set < QString >::iterator it = testiq_selected.begin();
               it != testiq_selected.end();
               ++it )
         {
            QString this_file = *it;
            if ( f_Is.count( this_file ) )
            {
               testiq_sortable_double tsd;
               tsd.index = this_file;
               tsd.x = 0e0;
               tsd.Imin = 1e99;
               tsd.Imax = -1e99;

               for ( int j = 0; j < (int) f_Is[ this_file ].size(); ++j )
               {
                  double t = f_Is[ this_file ][ j ];
                  tsd.x += t;
                  if ( tsd.Imin > t )
                  {
                     tsd.Imin = t;
                  }
                  if ( tsd.Imax < t )
                  {
                     tsd.Imax = t;
                  }
               }
               tsdl.push_back( tsd );
               avg_i += tsd.x;
               if ( max_i < tsd.x )
               {
                  max_i = tsd.x;
               }
            }
         }
         if ( tsdl.size() )
         {
            avg_i /= (double) tsdl.size();
         }
         double thresh = (max_i - avg_i) * 0.70 + avg_i;
         // qDebug( QString( "testiq thresh %1 max_i %2 avg_i %3" ).arg( thresh ).arg( max_i ).arg( avg_i ) );

         tsdl.sort();
         tsdl.reverse();
         
         for ( list < testiq_sortable_double >::iterator it = tsdl.begin();
               it != tsdl.end();
               ++it )
         {
            // qDebug( QString( "it loop x %1 index %2 Imin %3 Imax %4" ).arg( it->x ).arg( it->index ).arg( it->Imin ).arg( it->Imax ) );
            if ( it->x < thresh )
            {
               testiq_it_selected      = it->index;
               testiq_it_selected_Imin = it->Imin;
               testiq_it_selected_Imax = it->Imax;
               break;
            }
         }
         if ( testiq_it_selected.isEmpty() && tsdl.size() > 1 )
         {
            testiq_it_selected      = tsdl.begin()->index;
            testiq_it_selected_Imin = tsdl.begin()->Imin;
            testiq_it_selected_Imax = tsdl.begin()->Imax;
         }
      }
   }

   // qDebug( QString( "testitsel %1 min %2 max %3" ).arg( testiq_it_selected ).arg( testiq_it_selected_Imin ).arg( testiq_it_selected_Imax ) );

   mode_select( MODE_TESTIQ );

   gauss_add_marker( le_testiq_q_start  ->text().toDouble(), Qt::red, tr( "Start") );
   gauss_add_marker( le_testiq_q_end    ->text().toDouble(), Qt::red, tr( "End"  ), Qt::AlignLeft | Qt::AlignTop );

   if (
       !unified_ggaussian_ok ||
       !f_gaussians.count( wheel_file ) ||
       !f_gaussians[ wheel_file ].size() ||
       !ggaussian_compatible( testiq_selected, false ) )
   {
      lbl_testiq_gaussians   ->hide();
      rb_testiq_from_i_t     ->hide();
      rb_testiq_from_i_t     ->setChecked( true );
   } else {
      if ( testiq_active )
      {
         for ( int i = 0; i < (int) rb_testiq_gaussians.size(); ++i )
         {
            rb_testiq_gaussians[ i ]->show();
         }
      } else {
         for ( int i = 0; i < (int) f_gaussians[ wheel_file ].size(); i += gaussian_type_size )
         {
            QRadioButton * rb = new QRadioButton( this );
            rb->setPalette( PALET_NORMAL );
            AUTFBACK( rb );
            rb->setFont   ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
            rb->setText   ( QString( "%1 " ).arg( ( i / gaussian_type_size ) + 1 ) );
            rb->setChecked( false );
            rb->setEnabled( true );
            rb->show();
            rb_testiq_gaussians .push_back( rb );
            hbl_testiq_gaussians->addWidget( rb );
            bg_testiq_gaussians ->insert( rb );
            connect( rb, SIGNAL( clicked() ), SLOT( testiq_gauss_line() ) );
         }
         rb_testiq_from_i_t->setChecked( true );
      }
      
      for ( int i = 0; i < (int) unified_ggaussian_gaussians_size; i++ )
      {
         gauss_add_marker( unified_ggaussian_params[ common_size * i ], Qt::blue, QString( "%1" ).arg( i + 1 ) );
      }
      testiq_gauss_line();
   }         

   pb_rescale     ->setEnabled( true );
   pb_axis_x      ->setEnabled( true );
   pb_axis_y      ->setEnabled( true );

   running       = true;
   testiq_active = true;

   testiq_enables();

   // qDebug( suppress_replot ? "suppress_replot on" : "suppress_replot off" );
   if ( do_rescale )
   {
      // qDebug( "testiq mode scale rescale" );
      rescale();
   } else {
      plot_dist->replot();
   }
}

void US_Hydrodyn_Saxs_Hplc::testiq_gauss_line()
{
   int line_width = use_line_width < 3 ? ( use_line_width + 1 ) : use_line_width;

   for ( int i = 0; i < (int) rb_testiq_gaussians.size(); ++i )
   {
      QPen use_pen = QPen( rb_testiq_gaussians[ i ]->isChecked() ? Qt::magenta : Qt::blue, line_width, Qt::DashDotDotLine );
         
#ifdef QT4
      plotted_markers[ 2 + i ]->setLinePen( use_pen );
#else
      plot_dist->setMarkerPen( plotted_markers[ 2 + i ], use_pen );
#endif
   }
   plot_dist->replot();
   testiq_enables();
}      

void US_Hydrodyn_Saxs_Hplc::testiq_enables()
{
   pb_wheel_start         ->setEnabled( false );
   pb_wheel_cancel        ->setEnabled( true );
   le_testiq_q_start      ->setEnabled( true );
   le_testiq_q_end        ->setEnabled( true );
   pb_testiq_testset      ->setEnabled( le_testiq_q_start->text().toDouble() + .9999e0 <= le_testiq_q_end->text().toDouble() );
   pb_guinier             ->setEnabled( le_testiq_q_start->text().toDouble() + .9999e0 <= le_testiq_q_end->text().toDouble() );
   pb_scale               ->setEnabled( le_testiq_q_start->text().toDouble() + .9999e0 <= le_testiq_q_end->text().toDouble() );
   if ( rb_testiq_from_i_t->isChecked() )
   {
      cb_testiq_from_gaussian->setChecked( false );
      cb_testiq_from_gaussian->hide();
   } else {
      cb_testiq_from_gaussian->show();
   }
}

bool US_Hydrodyn_Saxs_Hplc::testiq_make()
{
   // todo: make the iq to vectors <> 
   // used by guinier_testiq() and testiq_testset
   testiq_created_names.clear();
   testiq_created_t    .clear();
   testiq_created_q    .clear();
   testiq_created_I    .clear();
   testiq_created_e    .clear();

   if ( cb_testiq_from_gaussian->isVisible() && !rb_testiq_from_i_t->isChecked() )
   {
      return create_i_of_q( testiq_selected, le_testiq_q_start->text().toDouble(), le_testiq_q_end->text().toDouble() );
   } else {
      return create_i_of_q_ng( testiq_selected, le_testiq_q_start->text().toDouble(), le_testiq_q_end->text().toDouble() );
   }
}

void US_Hydrodyn_Saxs_Hplc::testiq_visrange()
{
   disconnect( le_testiq_q_start, SIGNAL( textChanged( const QString & ) ), 0, 0 );
   disconnect( le_testiq_q_end, SIGNAL( textChanged( const QString & ) ), 0, 0 );
   if ( plot_dist_zoomer )
   {
#ifdef QT4
      le_testiq_q_start->setText( QString( "%1" ).arg( plot_dist_zoomer->zoomRect().left() ) );
      le_testiq_q_end  ->setText( QString( "%1" ).arg( plot_dist_zoomer->zoomRect().right() ) );
#else
      le_testiq_q_start->setText( QString( "%1" ).arg( plot_dist_zoomer->zoomRect().x1() ) );
      le_testiq_q_end  ->setText( QString( "%1" ).arg( plot_dist_zoomer->zoomRect().x2() ) );
#endif
   } else {
      le_testiq_q_start->setText( QString( "%1" ).arg( f_qs[ wheel_file ][ 0 ] ) );
      le_testiq_q_end  ->setText( QString( "%1" ).arg( f_qs[ wheel_file ].back() ) );
   }      
   connect( le_testiq_q_start, SIGNAL( textChanged( const QString & ) ), SLOT( testiq_q_start_text( const QString & ) ) );
   connect( le_testiq_q_end, SIGNAL( textChanged( const QString & ) ), SLOT( testiq_q_end_text( const QString & ) ) );
#ifndef QT4
   plot_dist->setMarkerPos( plotted_markers[ 0 ], le_testiq_q_start->text().toDouble(), 0e0 );
   plot_dist->setMarkerPos( plotted_markers[ 1 ], le_testiq_q_end  ->text().toDouble(), 0e0 );
#else
   plotted_markers[ 0 ]->setXValue( le_testiq_q_start->text().toDouble() );
   plotted_markers[ 1 ]->setXValue( le_testiq_q_end  ->text().toDouble() );
#endif
   if ( le_testiq_q_start->hasFocus() &&
        qwtw_wheel->value() != le_testiq_q_start->text().toDouble() )
   {
      qwtw_wheel->setValue( le_testiq_q_start->text().toDouble() );
   }
   if ( le_testiq_q_end->hasFocus() &&
        qwtw_wheel->value() != le_testiq_q_end->text().toDouble() )
   {
      qwtw_wheel->setValue( le_testiq_q_end->text().toDouble() );
   }
   plot_dist->replot();
   testiq_enables();
}

void US_Hydrodyn_Saxs_Hplc::testiq_testset()
{
   if ( !testiq_make() )
   {
      return;
   }
   // todo: add to plot
   for ( int i = 0; i < (int) testiq_created_names.size(); ++i )
   {
      QString this_file = testiq_created_names[ i ];
      add_plot( this_file, 
                testiq_created_q[ this_file ],
                testiq_created_I[ this_file ],
                testiq_created_e[ this_file ],
                false, false );
   }
   testiq_enables();
}

void US_Hydrodyn_Saxs_Hplc::testiq_q_start_text( const QString & text )
{
   if ( current_mode != MODE_TESTIQ )
   {
      return;
   }
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
   testiq_enables();
}

void US_Hydrodyn_Saxs_Hplc::testiq_q_end_text( const QString & text )
{
   if ( current_mode != MODE_TESTIQ )
   {
      return;
   }
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
   testiq_enables();
}

void US_Hydrodyn_Saxs_Hplc::testiq_q_start_focus( bool hasFocus )
{
   if ( current_mode != MODE_TESTIQ )
   {
      return;
   }
   if ( hasFocus )
   {
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      qwtw_wheel->setRange( f_qs[ wheel_file ][ 0 ], f_qs[ wheel_file ].back(), 
                            ( f_qs[ wheel_file ].back() - f_qs[ wheel_file ][ 0 ] ) / UHSH_WHEEL_RES );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setValue( le_testiq_q_start->text().toDouble() );
      qwtw_wheel->setEnabled( true );
   }
}

void US_Hydrodyn_Saxs_Hplc::testiq_q_end_focus( bool hasFocus )
{
   if ( current_mode != MODE_TESTIQ )
   {
      return;
   }
   if ( hasFocus )
   {
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      qwtw_wheel->setRange( f_qs[ wheel_file ][ 0 ], f_qs[ wheel_file ].back(), 
                            ( f_qs[ wheel_file ].back() - f_qs[ wheel_file ][ 0 ] ) / UHSH_WHEEL_RES );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setValue( le_testiq_q_end->text().toDouble() );
      qwtw_wheel->setEnabled( true );
   }
}

// --- guinier ---

void US_Hydrodyn_Saxs_Hplc::guinier()
{
   le_last_focus = (mQLineEdit *) 0;

   bool any_selected = false;

   guinier_names       .clear();
   guinier_q           .clear();
   guinier_q2          .clear();
   guinier_I           .clear();
   guinier_e           .clear();
   guinier_x           .clear();
   guinier_y           .clear();
   guinier_a           .clear();
   guinier_b           .clear();
   guinier_colors      .clear();

   guinier_it_t        .clear();
   guinier_it_I        .clear();

   if ( current_mode == MODE_TESTIQ )
   {
      if ( !testiq_make() )
      {
         return;
      }
      for ( int i = 0; i < (int) rb_testiq_gaussians.size(); ++i )
      {
         rb_testiq_gaussians[ i ]->hide();
      }

      guinier_mint = -1e0;
      guinier_maxt = 1e0;

      if ( !testiq_it_selected.isEmpty() &&
           f_qs.count( testiq_it_selected ) )
      {
         guinier_it_t      = f_qs[ testiq_it_selected ];
         guinier_it_I      = f_Is[ testiq_it_selected ];
         guinier_it_Imin   = testiq_it_selected_Imin;
         guinier_it_Imax   = testiq_it_selected_Imax;
         guinier_it_Irange = testiq_it_selected_Imax - testiq_it_selected_Imin;
         guinier_mint      = guinier_it_t[ 0 ] - 1;
         guinier_maxt      = guinier_it_t.back() + 1;
      }

      for ( int i = 0; i < (int) testiq_created_names.size(); ++i )
      {
         any_selected = true;
         QString this_file = testiq_created_names[ i ];
         guinier_names.push_back( this_file ); 
         guinier_t[ this_file ] = testiq_created_t[ this_file ];
         guinier_q[ this_file ] = testiq_created_q[ this_file ];
         guinier_I[ this_file ] = testiq_created_I[ this_file ];
         guinier_e[ this_file ] = testiq_created_e[ this_file ];
         for ( int j = 0; j < (int) guinier_q[ this_file ].size(); ++j )
         {
            guinier_q2[ this_file ].push_back( guinier_q[ this_file ][ j ] * guinier_q[ this_file ][ j ] );
         }
         guinier_colors[ this_file ] = plot_colors[ i % plot_colors.size() ];

         if ( !i )
         {
            guinier_minq  = guinier_q [ this_file ][ 0 ];
            guinier_maxq  = guinier_q [ this_file ].back();
            guinier_minq2 = guinier_q2[ this_file ][ 0 ];
            guinier_maxq2 = guinier_q2[ this_file ].back();
         } else {
            if ( guinier_minq > guinier_q [ this_file ][ 0 ] )
            {
               guinier_minq = guinier_q [ this_file ][ 0 ];
            }
            if ( guinier_maxq < guinier_q [ this_file ].back() )
            {
               guinier_maxq = guinier_q [ this_file ].back();
            }
            if ( guinier_minq2 > guinier_q2[ this_file ][ 0 ] )
            {
               guinier_minq2 = guinier_q2[ this_file ][ 0 ];
            }
            if ( guinier_maxq2 < guinier_q2[ this_file ].back() )
            {
               guinier_maxq2 = guinier_q2[ this_file ].back();
            }
         }            
      }
      if ( !any_selected )
      {
         editor_msg( "red", tr( "Internal error: no files selected in Guinier mode" ) );
         return;
      }
   } else {
      double pos = 0e0;
      guinier_mint = -1e0;
      guinier_maxt = 1e0;
      for ( int i = 0; i < lb_files->numRows(); i++ )
      {
         if ( lb_files->isSelected( i ) )
         {
            if ( !any_selected )
            {
               wheel_file = lb_files->text( i );
               any_selected = true;
            }
            QString this_file = lb_files->text( i );
            guinier_names.push_back( this_file ); 
            guinier_t[ this_file ] = ++pos;
            guinier_q[ this_file ] = f_qs[ this_file ];
            guinier_I[ this_file ] = f_Is[ this_file ];
            guinier_e[ this_file ] = f_errors[ this_file ];
            for ( int j = 0; j < (int) f_qs[ this_file ].size(); ++j )
            {
               guinier_q2[ this_file ].push_back( f_qs[ this_file ][ j ] * f_qs[ this_file ][ j ] );
            }

            if ( !plotted_curves.count( this_file ) )
            {
               editor_msg( "red", QString( tr( "Internal error: guinier selected %1, but no plotted curve found" ) ).arg( this_file ) );
               return;
            }

            if ( !guinier_q[ this_file ].size() )
            {
               editor_msg( "red", QString( tr( "Internal error: guinier selected %1, but no data for curve found" ) ).arg( this_file ) );
               return;
            }
            
            guinier_colors[ this_file ] = plot_colors[ f_pos[ this_file ] % plot_colors.size() ];

            if ( !i )
            {
               guinier_minq  = guinier_q [ this_file ][ 0 ];
               guinier_maxq  = guinier_q [ this_file ].back();
               guinier_minq2 = guinier_q2[ this_file ][ 0 ];
               guinier_maxq2 = guinier_q2[ this_file ].back();
            } else {
               if ( guinier_minq > guinier_q [ this_file ][ 0 ] )
               {
                  guinier_minq = guinier_q [ this_file ][ 0 ];
               }
               if ( guinier_maxq < guinier_q [ this_file ].back() )
               {
                  guinier_maxq = guinier_q [ this_file ].back();
               }
               if ( guinier_minq2 > guinier_q2[ this_file ][ 0 ] )
               {
                  guinier_minq2 = guinier_q2[ this_file ][ 0 ];
               }
               if ( guinier_maxq2 < guinier_q2[ this_file ].back() )
               {
                  guinier_maxq2 = guinier_q2[ this_file ].back();
               }
            }            
         }
      }

      guinier_maxt = pos + 1;

      if ( !any_selected )
      {
         editor_msg( "red", tr( "Internal error: no files selected in guinier mode" ) );
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
   }

   if ( le_guinier_q_start->text().isEmpty() ||
        le_guinier_q_start->text() == "0" ||
        le_guinier_q_start->text().toDouble() < guinier_minq )
   {
      disconnect( le_guinier_q_start, SIGNAL( textChanged( const QString & ) ), 0, 0 );
      le_guinier_q_start->setText( QString( "%1" ).arg( guinier_minq ) );
      connect( le_guinier_q_start, SIGNAL( textChanged( const QString & ) ), SLOT( guinier_q_start_text( const QString & ) ) );
   }

   if ( le_guinier_q_end->text().isEmpty() ||
        le_guinier_q_end->text() == "0" ||
        le_guinier_q_end->text().toDouble() > guinier_maxq )
   {
      disconnect( le_guinier_q_end, SIGNAL( textChanged( const QString & ) ), 0, 0 );
      le_guinier_q_end->setText( QString( "%1" ).arg( guinier_maxq > 0.05 ? 0.05 : guinier_maxq ) );
      connect( le_guinier_q_end, SIGNAL( textChanged( const QString & ) ), SLOT( guinier_q_end_text( const QString & ) ) );
   }

   disconnect( le_guinier_q2_start, SIGNAL( textChanged( const QString & ) ), 0, 0 );
   le_guinier_q2_start->setText( QString( "%1" ).arg( le_guinier_q_start->text().toDouble() * le_guinier_q_start->text().toDouble() ) );
   connect( le_guinier_q2_start, SIGNAL( textChanged( const QString & ) ), SLOT( guinier_q2_start_text( const QString & ) ) );

   disconnect( le_guinier_q2_end, SIGNAL( textChanged( const QString & ) ), 0, 0 );
   le_guinier_q2_end->setText( QString( "%1" ).arg( le_guinier_q_end->text().toDouble() * le_guinier_q_end->text().toDouble() ) );
   connect( le_guinier_q2_end, SIGNAL( textChanged( const QString & ) ), SLOT( guinier_q2_end_text( const QString & ) ) );

   disconnect( le_guinier_delta_start, SIGNAL( textChanged( const QString & ) ), 0, 0 );
   le_guinier_delta_start->setText( QString( "%1" ).arg( 5e-5 ) );
   connect( le_guinier_delta_start, SIGNAL( textChanged( const QString & ) ), SLOT( guinier_delta_start_text( const QString & ) ) );

   disconnect( le_guinier_delta_end, SIGNAL( textChanged( const QString & ) ), 0, 0 );
   le_guinier_delta_end->setText( QString( "%1" ).arg( 5e-5 ) );
   connect( le_guinier_delta_end, SIGNAL( textChanged( const QString & ) ), SLOT( guinier_delta_end_text( const QString & ) ) );

   if ( guinier_plot_rg_zoomer )
   {
      delete guinier_plot_rg_zoomer;
      guinier_plot_rg_zoomer = (ScrollZoomer *) 0;
   }

   disable_all();
   mode_select( MODE_GUINIER );
   plot_dist->hide();
   if ( testiq_active )
   {
      pb_testiq->setEnabled( true );
   }
   ShowHide::hide_widgets( guinier_errors_widgets );
   ShowHide::hide_widgets( guinier_rg_widgets );
   
   running       = true;

   guinier_replot();
   guinier_plot->replot();
   guinier_residuals( true );
   guinier_plot_errors->replot();
   guinier_enables();
}

void US_Hydrodyn_Saxs_Hplc::guinier_add_marker( 
                                               QwtPlot * plot,
                                               double pos, 
                                               QColor color, 
                                               QString text, 
#ifndef QT4
                                               int 
#else
                                               Qt::Alignment
#endif
                                               align )
{
#ifndef QT4
   long marker = plot->insertMarker();
   plot->setMarkerLineStyle ( marker, QwtMarker::VLine );
   plot->setMarkerPos       ( marker, pos, 0e0 );
   plot->setMarkerLabelAlign( marker, align );
   plot->setMarkerPen       ( marker, QPen( color, 2, DashDotDotLine));
   plot->setMarkerFont      ( marker, QFont("Helvetica", 11, QFont::Bold));
   plot->setMarkerLabelText ( marker, text );
#else
   QwtPlotMarker * marker = new QwtPlotMarker;
   marker->setLineStyle       ( QwtPlotMarker::VLine );
   marker->setLinePen         ( QPen( color, 2, Qt::DashDotDotLine ) );
   marker->setLabelOrientation( Qt::Horizontal );
   marker->setXValue          ( pos );
   marker->setLabelAlignment  ( align );
   {
      QwtText qwtt( text );
      qwtt.setFont( QFont("Helvetica", 11, QFont::Bold ) );
      marker->setLabel           ( qwtt );
   }
   marker->attach             ( plot );
#endif
   guinier_markers.push_back( marker );
}   

void US_Hydrodyn_Saxs_Hplc::guinier_sd()
{
   guinier_replot();
   guinier_plot->replot();
   guinier_analysis();
   guinier_residuals();
   guinier_plot_errors->replot();
   guinier_enables();
}

void US_Hydrodyn_Saxs_Hplc::guinier_plot_rg_toggle()
{
   if ( guinier_rg_widgets[ 0 ]->isVisible() )
   {
      ShowHide::hide_widgets( guinier_rg_widgets );
   } else {
      ShowHide::hide_widgets( guinier_rg_widgets, false );
   }
}

void US_Hydrodyn_Saxs_Hplc::guinier_residuals( bool reset )
{
   if ( reset )
   {
      guinier_plot_errors->clear();
#ifdef QT4
      guinier_plot_errors->detachItems( QwtPlotItem::Rtti_PlotMarker );
#else
      guinier_plot_errors->removeMarkers();
#endif
      guinier_error_curves.clear();
      if ( guinier_markers.size() == 4 )
      {
         guinier_markers.resize( 2 );
         guinier_add_marker( guinier_plot_errors, le_guinier_q2_start  ->text().toDouble(), Qt::red, tr( "Start") );
         guinier_add_marker( guinier_plot_errors, le_guinier_q2_end    ->text().toDouble(), Qt::red, tr( "End"  ), Qt::AlignLeft | Qt::AlignTop );
      }

      vector < double > x( 2 );
      vector < double > y( 2 );
      x[ 0 ] = guinier_minq2;
      x[ 1 ] = guinier_maxq2;

#ifdef QT4
      QwtPlotCurve *curve = new QwtPlotCurve( "base" );
      curve->setStyle( QwtPlotCurve::Lines );
#else
      long curve;
      curve = guinier_plot_errors->insertCurve( "base" );
      guinier_plot_errors->setCurveStyle( curve, QwtCurve::Lines );
#endif

#ifdef QT4
      curve->setPen( QPen( Qt::red, use_line_width, Qt::SolidLine ) );
      curve->setData(
                     (double *)&x[ 0 ],
                     (double *)&y[ 0 ],
                     2
                     );
      curve->attach( guinier_plot_errors );
#else
      guinier_plot_errors->setCurvePen( curve, QPen( Qt::darkRed, use_line_width, Qt::SolidLine ) );
      guinier_plot_errors->setCurveData( curve,
                                         (double *)&x[ 0 ],
                                         (double *)&y[ 0 ],
                                         2
                                         );
#endif
   }
   
   double tmp;
   double ltmp;

#ifdef QT4
   QwtPlotCurve * curve;
#else
   long           curve;
#endif

   double emin = 1e99;
   double emax = 0e0;

   double minq2 = le_guinier_q2_start->text().toDouble() - le_guinier_delta_start->text().toDouble();
   double maxq2 = le_guinier_q2_end  ->text().toDouble() + le_guinier_delta_end  ->text().toDouble();


   for ( map < QString, vector <double > >::iterator it = guinier_q2.begin();
         it != guinier_q2.end();
         ++it )
   {
      if ( !guinier_a.count( it->first ) )
      {
         // qDebug( "no guinier a line" );
         if ( guinier_error_curves.count( it->first ) )
         {
            return guinier_residuals( true );
         }
         continue;
      }

      // vector < double > * q = & guinier_q[ it->first ];
      vector < double > * I = & guinier_I[ it->first ];
      vector < double > * er = & guinier_e[ it->first ];

      int pts = (int) it->second.size();

      if ( rb_guinier_resid_sd->isChecked() && (int) er->size() != pts )
      {
         if ( guinier_error_curves.count( it->first ) )
         {
            return guinier_residuals( true );
         }
         continue;
      }

      QPen use_pen = QPen( guinier_colors[ it->first ], use_line_width, Qt::SolidLine );

      double a = guinier_a[ it->first ];
      double b = guinier_b[ it->first ];

      vector < double > e( pts );

      if ( rb_guinier_resid_diff->isChecked() )
      {
         for ( int i = 0; i < (int) pts; ++i )
         {
            tmp = (*I)[ i ];
            ltmp = log( tmp > 0e0 ? tmp : 1e-99 );
            e[ i ] =
               ltmp - ( a + b * it->second[ i ] )
               ;
         }
      }
         
      if ( rb_guinier_resid_sd->isChecked() )
      {
         for ( int i = 0; i < (int) pts; ++i )
         {
            if ( (*er)[ i ] == 0e0 )
            {
               e[ i ] = 0e0;
            } else {
               tmp = (*I)[ i ];
               ltmp = log( tmp > 0e0 ? tmp : 1e-99 );
               e[ i ] = (*I)[ i ] * ( ltmp - ( a + b * it->second[ i ] ) ) / (*er)[ i ];
            }
         }
      }

      if ( rb_guinier_resid_pct->isChecked() )
      {
         for ( int i = 0; i < (int) pts; ++i )
         {
            tmp = (*I)[ i ];
            ltmp = log( tmp > 0e0 ? tmp : 1e-99 );
            e[ i ] =
               100e0 * ( ltmp - ( a + b * it->second[ i ] ) ) / fabs( ltmp );
            ;
         }
      }

      for ( int i = 0; i < (int) pts; ++i )
      {
         if ( it->second[ i ] >= minq2 &&
              it->second[ i ] <= maxq2 )
         {
            if ( emin > e[ i ] )
            {
               emin = e[ i ];
            }
            if ( emax < e[ i ] )
            {
               emax = e[ i ];
            }
         }
      }

      if ( guinier_error_curves.count( it->first ) )
      {
         curve = guinier_error_curves[ it->first ];
      } else {
#ifdef QT4
         curve = new QwtPlotCurve( "errors" );
         curve->setStyle( QwtPlotCurve::Lines );
         curve->setPen( use_pen );
         curve->setStyle( QwtPlotCurve::Sticks );
         curve->attach( guinier_plot_errors );
#else
         curve = guinier_plot_errors->insertCurve( "errors" );
         guinier_plot_errors->setCurveStyle( curve, QwtCurve::Lines );
         guinier_plot_errors->setCurvePen( curve, use_pen );
         guinier_plot_errors->curve( curve )->setStyle( QwtCurve::Sticks );
#endif
         guinier_error_curves[ it->first ] = curve;
      }
         
#ifdef QT4
      curve->setData(
                     (double *)&it->second[ 0 ],
                     (double *)&e[ 0 ],
                     pts
                     );
#else
      guinier_plot_errors->setCurveData( curve,
                                         (double *)&it->second[ 0 ],
                                         (double *)&e[ 0 ],
                                         pts
                                         );
#endif
   }

   emax = fabs( emin ) > fabs( emax ) ? fabs( emin ) : fabs( emax );
   emax *= 1.1;
   guinier_plot_errors->setAxisScale( QwtPlot::yLeft, -emax, +emax );
}

void US_Hydrodyn_Saxs_Hplc::guinier_residuals_update()
{
   if ( rb_guinier_resid_diff->isChecked() )
   {
      guinier_plot_errors->setAxisTitle( QwtPlot::yLeft,tr( "ln(I(q)) - Guinier line" ) );
   }
   if ( rb_guinier_resid_sd->isChecked() )
   {
      guinier_plot_errors->setAxisTitle( QwtPlot::yLeft,tr( "(ln(I(q)) - Guinier line) / S.D." ) );
   }
   if ( rb_guinier_resid_pct->isChecked() )
   {
      guinier_plot_errors->setAxisTitle( QwtPlot::yLeft,tr( "% difference [100*(ln(I(q)) - Guinier)/ln(I(q))]" ) );
   }
   guinier_residuals();
   guinier_plot_errors->replot();
}

void US_Hydrodyn_Saxs_Hplc::guinier_analysis()
{
   US_Saxs_Util * usu = ((US_Hydrodyn *)us_hydrodyn)->saxs_util;

   double a;
   double b;
   double siga;
   double sigb;
   double chi2;
   double Rg;
   double I0;
   double smin;
   double smax;
   double sRgmin;
   double sRgmax;

   double qstart      = le_guinier_q_start->text().toDouble();
   double qend        = le_guinier_q_end  ->text().toDouble();
   double q2endvis    = le_guinier_q2_end ->text().toDouble() + le_guinier_delta_end->text().toDouble();
   // double sRgmaxlimit = le_guinier_qrgmax ->text().toDouble();

   // int points_min = 2;

   // bool any_sd_off = false;
   QString this_log;

   editor_msg( "black", "\n" );

   int    count  = 0;

   double i0_avg = 0e0;
   double i0_min = 1e99;
   double i0_max = -1e99;

   double rg_avg = 0e0;
   double rg_min = 1e99;
   double rg_max = -1e99;

   double qrg_avg = 0e0;
   double qrg_min = 1e99;
   double qrg_max = -1e99;

   double rg_sd_min = 1e99;
   double rg_sd_max = -1e99;

   guinier_plot_rg->clear();
   QwtSymbol rg_sym;
   rg_sym.setStyle( QwtSymbol::Diamond );
   rg_sym.setSize( 10 );

   double posmin = 1e99;
   double posmax = -1e99;

   vector < double > rg_x;
   vector < double > rg_y;

   // for ( map < QString, vector <double > >::iterator it = guinier_q2.begin();
   //       it != guinier_q2.end();
   //       ++it )
   // {
   for ( int i = 0; i < (int) guinier_names.size(); ++i )
   {
      QString this_name = guinier_names[ i ];
      double pos = guinier_t[ this_name ];
      if ( posmin > pos )
      {
         posmin = pos;
      }
      if ( posmax < pos )
      {
         posmax = pos;
      }

      guinier_x[ this_name ].clear();
      guinier_y[ this_name ].clear();
      guinier_a.erase( this_name );
      guinier_b.erase( this_name );

      usu->wave["hplc"].q.clear();
      usu->wave["hplc"].r.clear();
      usu->wave["hplc"].s.clear();

      bool use_SD_weighting = cb_guinier_sd->isChecked();
      if ( guinier_e[ this_name ].size() != guinier_q[ this_name ].size() )
      {
         // any_sd_off = true;
         // editor_msg( "dark red", QString( tr( "Notice: SD weighting of Guinier fit is off for %1 since SDs are not fully present" ) )
         //             .arg( this_name ) );
         use_SD_weighting = false;
      } else {
         for ( int j = 0; j < (int) guinier_q[ this_name ].size(); j++ )
         {
            if ( guinier_q[ this_name ][ j ] >= qstart &&
                 guinier_q[ this_name ][ j ] <= qend )
            {
               if ( guinier_e[ this_name ][ j ] <= 0e0 )
               {
                  // any_sd_off = true;
                  // editor_msg( "dark red", QString( tr( "Notice: SD weighting of Guinier fit is off for %1 since at least one SD is zero or negative in the selected q range" ) )
                  //             .arg( this_name) );
                  use_SD_weighting = false;
                  break;
               }
            }
         }
      }

      for ( int j = 0; j < (int) guinier_q[ this_name ].size(); j++ )
      {
         if ( guinier_q[ this_name ][ j ] >= qstart &&
              guinier_q[ this_name ][ j ] <= qend )
         {
            usu->wave[ "hplc" ].q.push_back( guinier_q[ this_name ][ j ] );
            usu->wave[ "hplc" ].r.push_back( guinier_I[ this_name ][ j ] );
            if ( use_SD_weighting )
            {
               usu->wave[ "hplc" ].s.push_back( guinier_e[ this_name ][ j ] );
            }
         }
      }

      unsigned int pstart = 0;
      unsigned int pend   = usu->wave[ "hplc" ].q.size() ? usu->wave[ "hplc" ].q.size() - 1 : 0;

      if ( 
          !usu->guinier_plot(
                             "hplcrg",
                             "hplc"
                             )   ||
          !usu->guinier_fit(
                            this_log,
                            "hplcrg", 
                            pstart,
                            pend,
                            a,
                            b,
                            siga,
                            sigb,
                            chi2,
                            Rg,
                            I0,
                            smax, // don't know why these are flipped
                            smin,
                            sRgmin,
                            sRgmax
                            ) )
      {
         // editor->append(QString("Error performing Guinier analysis on %1\n" + usu->errormsg + "\n")
         // .arg(qsl_plotted_iq_names[i]));
         editor_msg( "dark red", QString( "%1 could not compute Rg" ).arg( this_name ) );
      } else {
         // editor_msg( "blue", 
         //             QString( "%1 Rg %2 I0 %3 points %4 qRgmax %5" )
         //             .arg( this_name )
         //             .arg( Rg )
         //             .arg( I0 )
         //             .arg( sRgmax ) 
         //             );
         if ( !isnan( Rg ) )
         {
            if ( !count )
            {
               qrg_min   = sRgmax;
               qrg_max   = sRgmax;
               rg_min    = Rg;
               rg_max    = Rg;
               i0_min    = I0;
               i0_max    = I0;
               rg_sd_min = Rg;
               rg_sd_max = Rg;
            } else {
               if ( qrg_min > sRgmax )
               {
                  qrg_min = sRgmax;
               }
               if ( qrg_max < sRgmax )
               {
                  qrg_max = sRgmax;
               }
               if ( rg_min > Rg )
               {
                  rg_min = Rg;
               }
               if ( rg_max < Rg )
               {
                  rg_max = Rg;
               }
               if ( i0_min > I0 )
               {
                  i0_min = I0;
               }
               if ( i0_max < I0 )
               {
                  i0_max = I0;
               }
               if ( isnan( siga ) || (unsigned int) usu->wave[ "hplc" ].q.size() < 3 )
               {
                  if ( rg_sd_min > Rg )
                  {
                     rg_sd_min = Rg;
                  }
                  if ( rg_sd_max < Rg )
                  {
                     rg_sd_max = Rg;
                  }
               } else {
                  if ( rg_sd_min > Rg - siga )
                  {
                     rg_sd_min = Rg - siga;
                  }
                  if ( rg_sd_max < Rg + siga )
                  {
                     rg_sd_max = Rg + siga;
                  }
               }
            }

            qrg_avg += sRgmax;
            rg_avg  += Rg;
            i0_avg  += I0;

            count++;
         }

         QString report =
            QString("%1:%2 ").arg( pos ).arg( this_name ) +
            QString( "" )
            .sprintf(
                     "Rg %.1f (%.1f) (A) I(0) %.2e (%.2e) qRg [%.3f,%.3f] pts %u chi^2 %.2e r-chi %.2e\n"
                     , Rg
                     , sigb
                     , I0
                     , siga
                     , sRgmin
                     , sRgmax
                     , (unsigned int) usu->wave[ "hplc" ].q.size()
                     , chi2
                     , sqrt( chi2 / usu->wave[ "hplc" ].q.size() )
                     ) +
            tr( use_SD_weighting ? "SD  on" : "SD OFF" )
            ;

         guinier_a[ this_name ] = a;
         guinier_b[ this_name ] = b;
         guinier_x[ this_name ].push_back( guinier_q2[ this_name ][ 0 ] );
         guinier_x[ this_name ].push_back( q2endvis );
         guinier_y[ this_name ].push_back( exp( a + b * guinier_q2[ this_name ][ 0 ] ) );
         guinier_y[ this_name ].push_back( exp( a + b * q2endvis ) );

         if ( !isnan( Rg ) )
         {
            if ( !guinier_fit_lines.count( this_name ) )
            {
#ifdef QT4
               QwtPlotCurve *curve = new QwtPlotCurve( "fl:" + this_name );
               curve->setStyle ( QwtPlotCurve::Lines );
               curve->setPen( QPen( guinier_colors[ this_name ], use_line_width, Qt::SolidLine ) );
               curve->attach( guinier_plot );
#else
               long curve = guinier_plot->insertCurve( "fl:" + this_name );
               guinier_plot->setCurveStyle ( curve, QwtCurve::Lines );
               guinier_plot->setCurvePen( curve, QPen( guinier_colors[ this_name ], use_line_width, SolidLine ) );
#endif
               guinier_fit_lines[ this_name ] = curve;
            }
#ifdef QT4
            guinier_fit_lines[ this_name ]->setData(
                                                    (double *)&( guinier_x[ this_name ][ 0 ] ),
                                                    (double *)&( guinier_y[ this_name ][ 0 ] ),
                                                    2
                                                    );
#else
            guinier_plot->setCurveData( guinier_fit_lines[ this_name ],
                                        (double *)&( guinier_x[ this_name ][ 0 ] ),
                                        (double *)&( guinier_y[ this_name ][ 0 ] ),
                                        2
                                        );
#endif
            {
               rg_sym.setBrush( guinier_colors[ this_name ] );
#ifdef QT4
               QwtPlotCurve *curve = new QwtPlotCurve( this_name );
               curve->setStyle ( QwtPlotCurve::NoCurve );
               curve->setSymbol( rg_sym );
               curve->setData( & pos, & Rg, 1 );
               curve->attach( guinier_plot_rg );
#else
               long curve = guinier_plot_rg->insertCurve( this_name );
               guinier_plot_rg->setCurveStyle ( curve, QwtCurve::NoCurve );
               guinier_plot_rg->setCurveSymbol( curve, rg_sym );
               guinier_plot_rg->setCurveData( curve, & pos, & Rg, 1 );
#endif
               rg_x .push_back( pos );
               rg_y .push_back( Rg );
            }

            {
               double x[2];
               double y[2];
               
               x[ 0 ] = x[ 1 ] = pos;
               y[ 0 ] = Rg - sigb;
               y[ 1 ] = Rg + sigb;

               QPen use_pen = QPen( guinier_colors[ this_name ], use_line_width, Qt::SolidLine );
#ifdef QT4
               QwtPlotCurve *curve = new QwtPlotCurve( this_name + "_sd" );
               curve->setStyle ( QwtPlotCurve::Lines );
               curve->setData( x, y, 2 );
               curve->setPen( use_pen );
               curve->attach( guinier_plot_rg );
#else
               long curve = guinier_plot_rg->insertCurve( this_name + "_sd" );
               guinier_plot_rg->setCurveStyle ( curve, QwtCurve::Lines );
               guinier_plot_rg->setCurveData( curve, x, y, 2 );
               guinier_plot_rg->setCurvePen( curve, use_pen );
#endif
               rg_x .push_back( pos );
               rg_y .push_back( Rg );
            }
         }

         editor_msg( "dark blue", report );
         // qDebug( QString( "guinier a %1 b %2" ).arg( a ).arg( b ) );
         // US_Vector::printvector2( "guinier x,y:", guinier_x[ this_name ], guinier_y[ this_name ] );
      }
   }
   QString msg;
   switch ( count )
   {
   case 0 : msg = ""; break;
   case 1 : msg = QString( "q*Rg max %1   Rg %2   I0 %3" ).arg( qrg_avg ).arg( rg_avg ).arg( i0_avg ); break;
   default :
      {
         double countinv = 1e0 / (double) count;
         qrg_avg *= countinv;
         rg_avg  *= countinv;
         i0_avg  *= countinv;
         msg = QString( "Avg. %1 " ).arg( count );
         if ( count != ( int ) guinier_q2.size() )
         {
            msg += QString( "of %1 " ).arg( guinier_q2.size() );
         }
         msg += QString( "" )
            .sprintf(
                     "curves  q*Rg max %.3f [%.3f:%.3f]  Rg %.1f [%.1f:%.1f]  I0 %.2e [%.2e:%.2e]"
                     , qrg_avg
                     , qrg_min
                     , qrg_max
                     , rg_avg
                     , rg_min
                     , rg_max
                     , i0_avg
                     , i0_min
                     , i0_max
                     )
            ;
      }
      break;
   }
   lbl_guinier_stats->setText( msg );

   if ( rg_x.size() > 1 )
   {
      QPen use_pen = QPen( Qt::cyan, use_line_width, Qt::DotLine );
#ifdef QT4
      QwtPlotCurve * curve = new QwtPlotCurve( "rgline" );
      curve->setStyle( QwtPlotCurve::Lines );
      curve->setData(
                     (double *)&(rg_x[0]),
                     (double *)&(rg_y[0]),
                     rg_x.size() );
      curve->setPen( use_pen );
      curve->attach( guinier_plot_rg );
#else
      long curve = guinier_plot_rg->insertCurve( "rgline" );
      guinier_plot_rg->setCurveStyle( curve, QwtCurve::Lines );
      guinier_plot_rg->setCurveData( curve,
                                     (double *)&(rg_x[0]),
                                     (double *)&(rg_y[0]),
                                     rg_x.size() );
      guinier_plot_rg->setCurvePen( curve, use_pen );
#endif
   }
   double use_min = posmin - 1e0;
   double use_max = posmax + 1e0;
   double space = 0.05 * ( rg_sd_max - rg_sd_min ) ;
   // qDebug( QString( "space %1 rg_max %2 rg_sd_max %3 rg_min %4 rg_sd_min %5 count %6" )
   //         .arg( space )
   //         .arg( rg_max )
   //         .arg( rg_sd_max )
   //         .arg( rg_min )
   //         .arg( rg_sd_min )
   //         .arg( count )
   //         );

   if ( rg_x.size() > 1 && guinier_it_t.size() && space > 0e0 )
   {
      // qDebug( QString( "testiq in rg plot ok use minmax %1 %2" ).arg( use_min ).arg( use_max ) );
      vector < double > x;
      vector < double > y;
      vector < double > y2 = guinier_it_I;

      double ymin = 1e99;
      double ymax = -1e99;

      if ( cb_guinier_lock_rg_range->isChecked() )
      {
         use_min = le_guinier_rg_t_start->text().toDouble();
         use_max = le_guinier_rg_t_end->text().toDouble();
      }

      for ( int i = 0; i < (int) guinier_it_t.size(); ++i )
      {
         if ( guinier_it_t[ i ] >= use_min &&
              guinier_it_t[ i ] <= use_max )
         {
            x.push_back( guinier_it_t[ i ] );
            y.push_back( guinier_it_I[ i ] );
            if ( ymin > guinier_it_I[ i ] )
            {
               ymin = guinier_it_I[ i ];
            } 
            if ( ymax < guinier_it_I[ i ] )
            {
               ymax = guinier_it_I[ i ];
            } 
         }
      }
      // US_Vector::printvector2( "ref x,y", x, y );

      if ( ymax > ymin )
      {
         if ( cb_guinier_lock_rg_range->isChecked() )
         {
            double mint = le_guinier_rg_rg_start->text().toDouble();
            double maxt = le_guinier_rg_rg_end->text().toDouble();
            double scale = ( maxt - mint ) / ( ymax - ymin );
            for ( int i = 0; i < (int) y2.size(); ++i )
            {
               y2[ i ] -= ymin;
               y2[ i ] *= scale;
               y2[ i ] += mint;
            }
         } else {
            double scale = ( rg_sd_max - rg_sd_min ) / ( ymax - ymin );
            for ( int i = 0; i < (int) y2.size(); ++i )
            {
               y2[ i ] -= ymin;
               y2[ i ] *= scale;
               y2[ i ] += rg_sd_min;
            }
         }

         // qDebug( QString( "testiq in rg plot scale %1 rg_min %2 rg_max %3 ymin %4 ymax %5 " ).arg( scale ).arg( rg_min ).arg( rg_max ).arg( ymin ).arg( ymax ) );

         // for ( int i = 0; i < (int) y.size(); ++i )
         // {
         //    y[ i ] -= ymin;
         //    y[ i ] *= scale;
         //    y[ i ] += rg_min;
         // }

         // QPen use_pen = QPen( plot_colors[ f_pos[ testiq_it_selected ] % plot_colors.size() ], use_line_width, Qt::SolidLine );
         QPen use_pen = QPen( Qt::green, use_line_width, Qt::SolidLine );
#ifdef QT4
         QwtPlotCurve * curve = new QwtPlotCurve( "refitline" );
         curve->setStyle( QwtPlotCurve::Lines );
         curve->setData(
                        (double *)&(guinier_it_t[0]),
                        (double *)&(y2[0]),
                        guinier_it_t.size() );
         curve->setPen( use_pen );
         curve->attach( guinier_plot_rg );
#else
         long curve = guinier_plot_rg->insertCurve( "rgline" );
         guinier_plot_rg->setCurveStyle( curve, QwtCurve::Lines );
         guinier_plot_rg->setCurveData( curve,
                                        (double *)&(guinier_it_t[0]),
                                        (double *)&(y2[0]),
                                        guinier_it_t.size() );
         guinier_plot_rg->setCurvePen( curve, use_pen );
#endif
      }
   }


   {
      if ( cb_guinier_lock_rg_range->isChecked() )
      {
         guinier_plot_rg->setAxisScale( QwtPlot::xBottom, le_guinier_rg_t_start->text().toDouble(), le_guinier_rg_t_end->text().toDouble() );
         guinier_plot_rg->setAxisScale( QwtPlot::yLeft  , le_guinier_rg_rg_start->text().toDouble(), le_guinier_rg_rg_end->text().toDouble() );
      } else {
         guinier_plot_rg->setAxisScale( QwtPlot::xBottom, use_min, use_max );
         guinier_plot_rg->setAxisScale( QwtPlot::yLeft  , rg_sd_min - space, rg_sd_max + space );
         
         disconnect( le_guinier_rg_t_start , SIGNAL( textChanged( const QString & ) ), 0, 0 );
         disconnect( le_guinier_rg_t_end   , SIGNAL( textChanged( const QString & ) ), 0, 0 );
         disconnect( le_guinier_rg_rg_start, SIGNAL( textChanged( const QString & ) ), 0, 0 );
         disconnect( le_guinier_rg_rg_end  , SIGNAL( textChanged( const QString & ) ), 0, 0 );
         le_guinier_rg_t_start ->setText( QString( "%1" ).arg( use_min ) );
         le_guinier_rg_t_end   ->setText( QString( "%1" ).arg( use_max ) );
         le_guinier_rg_rg_start->setText( QString( "%1" ).arg( rg_sd_min - space ) );
         le_guinier_rg_rg_end  ->setText( QString( "%1" ).arg( rg_sd_max + space ) );
         connect( le_guinier_rg_t_start, SIGNAL( textChanged( const QString & ) ), SLOT( guinier_rg_t_start_text( const QString & ) ) );
         connect( le_guinier_rg_t_end, SIGNAL( textChanged( const QString & ) ), SLOT( guinier_rg_t_end_text( const QString & ) ) );
         connect( le_guinier_rg_rg_start, SIGNAL( textChanged( const QString & ) ), SLOT( guinier_rg_rg_start_text( const QString & ) ) );
         connect( le_guinier_rg_rg_end, SIGNAL( textChanged( const QString & ) ), SLOT( guinier_rg_rg_end_text( const QString & ) ) );
      }

      if ( guinier_plot_rg_zoomer )
      {
          delete guinier_plot_rg_zoomer;
      }
      guinier_plot_rg_zoomer = new ScrollZoomer(guinier_plot_rg->canvas());
      guinier_plot_rg_zoomer->setRubberBandPen(QPen(Qt::yellow, 0, Qt::DotLine));
#ifndef QT4
      guinier_plot_rg_zoomer->setCursorLabelPen(QPen(Qt::yellow));
#endif
   }
      
   guinier_plot_rg->replot();
}

void US_Hydrodyn_Saxs_Hplc::guinier_delete_markers()
{
#ifndef QT4
   guinier_plot       ->removeMarkers();
   guinier_plot_errors->removeMarkers();
#else
   guinier_plot       ->detachItems( QwtPlotItem::Rtti_PlotMarker );
   guinier_plot_errors->detachItems( QwtPlotItem::Rtti_PlotMarker );
#endif
}

void US_Hydrodyn_Saxs_Hplc::guinier_range( 
                                          double minq2, 
                                          double maxq2,
                                          double minI, 
                                          double maxI
                                           )
{
   guinier_plot       ->setAxisScale( QwtPlot::xBottom, minq2, maxq2 );
   guinier_plot_errors->setAxisScale( QwtPlot::xBottom, minq2, maxq2 );
   guinier_plot       ->setAxisScale( QwtPlot::yLeft  , minI * 0.9e0 , maxI * 1.1e0 );

   if ( guinier_plot_zoomer )
   {
      delete guinier_plot_zoomer;
      guinier_plot_zoomer = (ScrollZoomer *) 0;
   }

   if ( !guinier_plot_zoomer )
   {
      // puts( "redoing zoomer" );
      guinier_plot_zoomer = new ScrollZoomer(guinier_plot->canvas());
      guinier_plot_zoomer->setRubberBandPen(QPen(Qt::yellow, 0, Qt::DotLine));
#ifndef QT4
      guinier_plot_zoomer->setCursorLabelPen(QPen(Qt::yellow));
#endif
      // connect( guinier_plot_zoomer, SIGNAL( zoomed( const QwtDoubleRect & ) ), SLOT( plot_zoomed( const QwtDoubleRect & ) ) );
   }

   if ( guinier_plot_errors_zoomer )
   {
      delete guinier_plot_errors_zoomer;
      guinier_plot_errors_zoomer = (ScrollZoomer *) 0;
   }

   if ( !guinier_plot_errors_zoomer )
   {
      // puts( "redoing zoomer" );
      guinier_plot_errors_zoomer = new ScrollZoomer(guinier_plot_errors->canvas());
      guinier_plot_errors_zoomer->setRubberBandPen(QPen(Qt::yellow, 0, Qt::DotLine));
#ifndef QT4
      guinier_plot_errors_zoomer->setCursorLabelPen(QPen(Qt::yellow));
#endif
      // connect( guinier_plot_zoomer, SIGNAL( zoomed( const QwtDoubleRect & ) ), SLOT( plot_zoomed( const QwtDoubleRect & ) ) );
   }
}

void US_Hydrodyn_Saxs_Hplc::guinier_range()
{
   double minI  = 1e99;
   double maxI  = 0e0;
   double minq2 = le_guinier_q2_start->text().toDouble() - le_guinier_delta_start->text().toDouble();
   double maxq2 = le_guinier_q2_end  ->text().toDouble() + le_guinier_delta_end  ->text().toDouble();

   if ( minq2 < guinier_minq2 )
   {
      minq2 = guinier_minq2;
   }
   if ( maxq2 > guinier_maxq2 )
   {
      maxq2 = guinier_maxq2;
   }

   for ( map < QString, vector <double > >::iterator it = guinier_q2.begin();
         it != guinier_q2.end();
         ++it )
   {
      unsigned int q_points = it->second.size();

      for ( unsigned int i = 0; i < q_points; i++ )
      {
         double I = guinier_I [ it->first ][ i ];
         if ( I > 0e0 &&
              guinier_q2[ it->first ][ i ] >= minq2 &&
              guinier_q2[ it->first ][ i ] <= maxq2 )
         {
            if ( minI > I )
            {
               minI = I;
            }
            if ( maxI < I )
            {
               maxI = I;
            }
         }
      }
   }
   
   guinier_range( minq2, maxq2, minI, maxI );
}

void US_Hydrodyn_Saxs_Hplc::guinier_replot()
{
   guinier_curves.clear();
   guinier_markers.clear();
   guinier_fit_lines.clear();
   guinier_plot->clear();
   guinier_add_marker( guinier_plot,        le_guinier_q2_start  ->text().toDouble(), Qt::red, tr( "Start") );
   guinier_add_marker( guinier_plot,        le_guinier_q2_end    ->text().toDouble(), Qt::red, tr( "End"  ), Qt::AlignLeft | Qt::AlignTop );
   guinier_add_marker( guinier_plot_errors, le_guinier_q2_start  ->text().toDouble(), Qt::red, tr( "Start") );
   guinier_add_marker( guinier_plot_errors, le_guinier_q2_end    ->text().toDouble(), Qt::red, tr( "End"  ), Qt::AlignLeft | Qt::AlignTop );

   double minI  = 1e99;
   double maxI  = 0e0;
   double minq2 = le_guinier_q2_start->text().toDouble() - le_guinier_delta_start->text().toDouble();
   double maxq2 = le_guinier_q2_end  ->text().toDouble() + le_guinier_delta_end  ->text().toDouble();

   if ( minq2 < guinier_minq2 )
   {
      minq2 = guinier_minq2;
   }
   if ( maxq2 > guinier_maxq2 )
   {
      maxq2 = guinier_maxq2;
   }

   QwtSymbol sym;
   sym.setStyle(QwtSymbol::Diamond);
   sym.setSize(6);
   sym.setBrush(Qt::white);

   for ( map < QString, vector <double > >::iterator it = guinier_q2.begin();
         it != guinier_q2.end();
         ++it )
   {
      // plot each curve
      sym.setPen( QPen( guinier_colors[ it->first ] ) );
#ifndef QT4
      long curve = guinier_plot->insertCurve( it->first );
      guinier_plot->setCurveStyle ( curve, QwtCurve::NoCurve );
      guinier_plot->setCurveSymbol( curve, sym );
#else
      QwtPlotCurve *curve = new QwtPlotCurve( it->first );
      curve->setStyle ( QwtPlotCurve::NoCurve );
      curve->setSymbol( sym );
#endif
      guinier_curves[ it->first ] = curve;
      unsigned int q_points = it->second.size();

      vector < double > q;
      vector < double > I;
      vector < double > e;
      bool use_error = ( guinier_q[ it->first ].size() == guinier_e[ it->first ].size() );
      for ( unsigned int i = 0; i < q_points; i++ )
      {
         if ( guinier_I [ it->first ][ i ] > 0e0 )
         {
            q.push_back( guinier_q2[ it->first ][ i ] );
            I.push_back( guinier_I [ it->first ][ i ] );
            if ( guinier_q2[ it->first ][ i ] >= minq2 &&
                 guinier_q2[ it->first ][ i ] <= maxq2 )
            {
               if ( minI > I.back() )
               {
                  minI = I.back();
               }
               if ( maxI < I.back() )
               {
                  maxI = I.back();
               }
            }
            if ( use_error )
            {
               e.push_back( guinier_e[ it->first ][ i ] );
            }
         }
      }
      q_points = ( unsigned int )q.size();
      QColor use_qc = guinier_colors[ it->first ];
#ifndef QT4
      guinier_plot->setCurveData( curve, 
                                  (double *)&( q[ 0 ] ),
                                  (double *)&( I[ 0 ] ),
                                  q_points
                                  );
      guinier_plot->setCurvePen( curve, QPen( use_qc, use_line_width, SolidLine));
#else
      curve->setData(
                     (double *)&( q[ 0 ] ),
                     (double *)&( I[ 0 ] ),
                     q_points
                     );

      curve->setPen( QPen( use_qc, use_line_width, Qt::SolidLine ) );
      curve->attach( guinier_plot );
#endif

      if ( use_error )
      {
         vector < double > x( 2 );
         vector < double > y( 2 );
         QString ebname = "eb:" + it->first;
#ifdef QT4
         QPen use_pen = QPen( use_qc, use_line_width, Qt::SolidLine );
#else
         QPen use_pen = QPen( use_qc, use_line_width, Qt::SolidLine );
#endif
         for ( int i = 0; i < ( int ) q_points; ++i )
         {
            x[ 0 ] = x[ 1 ] = q[ i ];
            y[ 0 ] = I[ i ] - e[ i ];
            y[ 1 ] = I[ i ] + e[ i ];
#ifdef QT4
            QwtPlotCurve * curve = new QwtPlotCurve( ebname );
            curve->setStyle( QwtPlotCurve::Lines );
            curve->setData(
                           (double *)&(x[0]),
                           (double *)&(y[0]),
                           2 );
            curve->setPen( use_pen );
            curve->attach( guinier_plot );
#else
            long curve = guinier_plot->insertCurve( ebname );
            guinier_plot->setCurveStyle( curve, QwtCurve::Lines );
            guinier_plot->setCurveData( curve,
                                        (double *)&(x[0]),
                                        (double *)&(y[0]),
                                        2 );
            guinier_plot->setCurvePen( curve, use_pen );
#endif
         }
      }
   }         

   guinier_analysis();
   guinier_residuals();
   guinier_range( minq2, maxq2, minI, maxI );
}

void US_Hydrodyn_Saxs_Hplc::guinier_enables()
{
   pb_guinier             -> setEnabled( false );
   pb_errors              -> setEnabled( true );
   pb_wheel_cancel        -> setEnabled( true );
   le_guinier_q_start     -> setEnabled( true );
   le_guinier_q_end       -> setEnabled( true );
   le_guinier_q2_start    -> setEnabled( true );
   le_guinier_q2_end      -> setEnabled( true );
   le_guinier_delta_start -> setEnabled( true );
   le_guinier_delta_end   -> setEnabled( true );
   le_guinier_qrgmax      -> setEnabled( true );
   pb_guinier_plot_rg     -> setEnabled( guinier_q.size() > 1 );
   le_guinier_rg_t_start  -> setEnabled( true );
   le_guinier_rg_t_end    -> setEnabled( true );
   le_guinier_rg_rg_start -> setEnabled( true );
   le_guinier_rg_rg_end   -> setEnabled( true );
}

void US_Hydrodyn_Saxs_Hplc::guinier_qrgmax_text( const QString & )
{
}

void US_Hydrodyn_Saxs_Hplc::guinier_q_start_text( const QString & text )
{
   if ( current_mode != MODE_GUINIER )
   {
      return;
   }
   if ( guinier_markers.size() != 4 )
   {
      editor_msg( "red", QString( "internal error: guinier_q_start_text markers issue size %1" ).arg( guinier_markers.size() ) );
      return;
   }
#ifndef QT4
   guinier_plot       ->setMarkerPos( guinier_markers[ 0 ], text.toDouble() * text.toDouble(), 0e0 );
   guinier_plot_errors->setMarkerPos( guinier_markers[ 2 ], text.toDouble() * text.toDouble(), 0e0 );
#else
   guinier_markers[ 0 ]->setXValue( text.toDouble() * text.toDouble() );
   guinier_markers[ 2 ]->setXValue( text.toDouble() * text.toDouble() );
#endif
   if ( qwtw_wheel->value() != text.toDouble() )
   {
      qwtw_wheel->setValue( text.toDouble() );
      disconnect( le_guinier_q2_start, SIGNAL( textChanged( const QString & ) ), 0, 0 );
      le_guinier_q2_start->setText( QString( "%1" ).arg( text.toDouble() * text.toDouble() ) );
      connect   ( le_guinier_q2_start, SIGNAL( textChanged( const QString & ) ), SLOT( guinier_q2_start_text( const QString & ) ) );
   }
   if ( text.toDouble() > le_guinier_q_end->text().toDouble() )
   {
      le_guinier_q_end->setText( text );
   } else {
      guinier_analysis();
      guinier_residuals();
      guinier_range();
      guinier_plot->replot();
      guinier_plot_errors->replot();
      guinier_enables();
   }
}

#define UHSH_G_WHEEL_RES ( 25e0 * UHSH_WHEEL_RES )

void US_Hydrodyn_Saxs_Hplc::guinier_q_end_text( const QString & text )
{
   if ( current_mode != MODE_GUINIER )
   {
      return;
   }
   if ( guinier_markers.size() != 4 )
   {
      editor_msg( "red", QString( "internal error: guinier_q_end_text markers issue size %1" ).arg( guinier_markers.size() ) );
      return;
   }
#ifndef QT4
   guinier_plot       ->setMarkerPos( guinier_markers[ 1 ], text.toDouble() * text.toDouble(), 0e0 );
   guinier_plot_errors->setMarkerPos( guinier_markers[ 3 ], text.toDouble() * text.toDouble(), 0e0 );
#else
   guinier_markers[ 1 ]->setXValue( text.toDouble() * text.toDouble() );
   guinier_markers[ 3 ]->setXValue( text.toDouble() * text.toDouble() );
#endif
   if ( qwtw_wheel->value() != text.toDouble() )
   {
      qwtw_wheel->setValue( text.toDouble() );
      disconnect( le_guinier_q2_end, SIGNAL( textChanged( const QString & ) ), 0, 0 );
      le_guinier_q2_end->setText( QString( "%1" ).arg( text.toDouble() * text.toDouble() ) );
      connect   ( le_guinier_q2_end, SIGNAL( textChanged( const QString & ) ), SLOT( guinier_q2_end_text( const QString & ) ) );
   }
   if ( text.toDouble() < le_guinier_q_start->text().toDouble() )
   {
      le_guinier_q_start->setText( text );
   } else {
      guinier_analysis();
      guinier_residuals();
      guinier_range();
      guinier_plot->replot();
      guinier_plot_errors->replot();
      guinier_enables();
   }
}

void US_Hydrodyn_Saxs_Hplc::guinier_q_start_focus( bool hasFocus )
{
   if ( current_mode != MODE_GUINIER )
   {
      return;
   }
   if ( hasFocus )
   {
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      qwtw_wheel->setRange( guinier_minq, guinier_maxq,
                            ( guinier_maxq - guinier_minq ) / UHSH_G_WHEEL_RES );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setValue( le_guinier_q_start->text().toDouble() );
      qwtw_wheel->setEnabled( true );
   }
}

void US_Hydrodyn_Saxs_Hplc::guinier_q_end_focus( bool hasFocus )
{
   if ( current_mode != MODE_GUINIER )
   {
      return;
   }
   if ( hasFocus )
   {
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      qwtw_wheel->setRange( guinier_minq, guinier_maxq,
                            ( guinier_maxq - guinier_minq ) / UHSH_G_WHEEL_RES );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setValue( le_guinier_q_end->text().toDouble() );
      qwtw_wheel->setEnabled( true );
   }
}

void US_Hydrodyn_Saxs_Hplc::guinier_q2_start_text( const QString & text )
{
   if ( current_mode != MODE_GUINIER )
   {
      return;
   }
   if ( guinier_markers.size() != 4 )
   {
      editor_msg( "red", QString( "internal error: guinier_q2_start_text markers issue size %1" ).arg( guinier_markers.size() ) );
      return;
   }
#ifndef QT4
   guinier_plot       ->setMarkerPos( guinier_markers[ 0 ], text.toDouble(), 0e0 );
   guinier_plot_errors->setMarkerPos( guinier_markers[ 2 ], text.toDouble(), 0e0 );
#else
   guinier_markers[ 0 ]->setXValue( text.toDouble() );
   guinier_markers[ 2 ]->setXValue( text.toDouble() );
#endif
   if ( qwtw_wheel->value() != text.toDouble() )
   {
      qwtw_wheel->setValue( text.toDouble() );
      disconnect( le_guinier_q_start, SIGNAL( textChanged( const QString & ) ), 0, 0 );
      le_guinier_q_start->setText( QString( "%1" ).arg( sqrt( text.toDouble() ) ) );
      connect   ( le_guinier_q_start, SIGNAL( textChanged( const QString & ) ), SLOT( guinier_q_start_text( const QString & ) ) );
   }

   guinier_residuals();
   guinier_range();
   guinier_plot->replot();
   guinier_plot_errors->replot();
   guinier_enables();
}

void US_Hydrodyn_Saxs_Hplc::guinier_q2_end_text( const QString & text )
{
   if ( current_mode != MODE_GUINIER )
   {
      return;
   }
   if ( guinier_markers.size() != 4 )
   {
      editor_msg( "red", QString( "internal error: guinier_q2_end_text markers issue size %1" ).arg( guinier_markers.size() ) );
      return;
   }
#ifndef QT4
   guinier_plot       ->setMarkerPos( guinier_markers[ 1 ], text.toDouble(), 0e0 );
   guinier_plot_errors->setMarkerPos( guinier_markers[ 3 ], text.toDouble(), 0e0 );
#else
   guinier_markers[ 1 ]->setXValue( text.toDouble() );
   guinier_markers[ 3 ]->setXValue( text.toDouble() );
#endif
   if ( qwtw_wheel->value() != text.toDouble() )
   {
      qwtw_wheel->setValue( text.toDouble() );
      disconnect( le_guinier_q_end, SIGNAL( textChanged( const QString & ) ), 0, 0 );
      le_guinier_q_end->setText( QString( "%1" ).arg( sqrt( text.toDouble() ) ) );
      connect   ( le_guinier_q_end, SIGNAL( textChanged( const QString & ) ), SLOT( guinier_q_end_text( const QString & ) ) );
   }

   guinier_residuals();
   guinier_range();
   guinier_plot->replot();
   guinier_plot_errors->replot();
   guinier_enables();
}

void US_Hydrodyn_Saxs_Hplc::guinier_q2_start_focus( bool hasFocus )
{
   if ( current_mode != MODE_GUINIER )
   {
      return;
   }
   if ( hasFocus )
   {
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      qwtw_wheel->setRange( guinier_minq2, guinier_maxq2,
                            ( guinier_maxq2 - guinier_minq2 ) / UHSH_G_WHEEL_RES );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setValue( le_guinier_q2_start->text().toDouble() );
      qwtw_wheel->setEnabled( true );
   }
}

void US_Hydrodyn_Saxs_Hplc::guinier_q2_end_focus( bool hasFocus )
{
   if ( current_mode != MODE_GUINIER )
   {
      return;
   }
   if ( hasFocus )
   {
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      qwtw_wheel->setRange( guinier_minq2, guinier_maxq2,
                            ( guinier_maxq2 - guinier_minq2 ) / UHSH_G_WHEEL_RES );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setValue( le_guinier_q2_end->text().toDouble() );
      qwtw_wheel->setEnabled( true );
   }
}

void US_Hydrodyn_Saxs_Hplc::guinier_delta_start_text( const QString & text )
{
   if ( current_mode != MODE_GUINIER )
   {
      return;
   }
   if ( qwtw_wheel->value() != text.toDouble() )
   {
      qwtw_wheel->setValue( text.toDouble() );
   }

   guinier_residuals();
   guinier_range();
   guinier_plot->replot();
   guinier_plot_errors->replot();
   guinier_enables();
}

void US_Hydrodyn_Saxs_Hplc::guinier_delta_end_text( const QString & text )
{
   if ( current_mode != MODE_GUINIER )
   {
      return;
   }
   if ( qwtw_wheel->value() != text.toDouble() )
   {
      qwtw_wheel->setValue( text.toDouble() );
   }

   guinier_residuals();
   guinier_range();
   guinier_plot->replot();
   guinier_plot_errors->replot();
   guinier_enables();
}

void US_Hydrodyn_Saxs_Hplc::guinier_delta_start_focus( bool hasFocus )
{
   if ( current_mode != MODE_GUINIER )
   {
      return;
   }
   if ( hasFocus )
   {
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      qwtw_wheel->setRange( 0e0, guinier_maxq2, guinier_maxq2 / UHSH_G_WHEEL_RES );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setValue( le_guinier_delta_start->text().toDouble() );
      qwtw_wheel->setEnabled( true );
   }
}

void US_Hydrodyn_Saxs_Hplc::guinier_delta_end_focus( bool hasFocus )
{
   if ( current_mode != MODE_GUINIER )
   {
      return;
   }
   if ( hasFocus )
   {
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      qwtw_wheel->setRange( 0e0, guinier_maxq2, guinier_maxq2 / UHSH_G_WHEEL_RES );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setValue( le_guinier_delta_end->text().toDouble() );
      qwtw_wheel->setEnabled( true );
   }
}

void US_Hydrodyn_Saxs_Hplc::guinier_rg_t_start_text( const QString & text )
{
   if ( current_mode != MODE_GUINIER )
   {
      return;
   }
   guinier_plot_rg->setAxisScale( QwtPlot::xBottom, text.toDouble(), le_guinier_rg_t_end->text().toDouble() );
   guinier_plot_rg->replot();
   if ( guinier_plot_rg_zoomer )
   {
      delete guinier_plot_rg_zoomer;
   }
   guinier_plot_rg_zoomer = new ScrollZoomer(guinier_plot_rg->canvas());
   guinier_plot_rg_zoomer->setRubberBandPen(QPen(Qt::yellow, 0, Qt::DotLine));
#ifndef QT4
   guinier_plot_rg_zoomer->setCursorLabelPen(QPen(Qt::yellow));
#endif
}

void US_Hydrodyn_Saxs_Hplc::guinier_rg_t_end_text( const QString & text )
{
   if ( current_mode != MODE_GUINIER )
   {
      return;
   }
   guinier_plot_rg->setAxisScale( QwtPlot::xBottom, le_guinier_rg_t_start->text().toDouble(), text.toDouble() );
   guinier_plot_rg->replot();
   if ( guinier_plot_rg_zoomer )
   {
      delete guinier_plot_rg_zoomer;
   }
   guinier_plot_rg_zoomer = new ScrollZoomer(guinier_plot_rg->canvas());
   guinier_plot_rg_zoomer->setRubberBandPen(QPen(Qt::yellow, 0, Qt::DotLine));
#ifndef QT4
   guinier_plot_rg_zoomer->setCursorLabelPen(QPen(Qt::yellow));
#endif
}

void US_Hydrodyn_Saxs_Hplc::guinier_rg_t_start_focus( bool hasFocus )
{
   if ( current_mode != MODE_GUINIER )
   {
      return;
   }
   if ( hasFocus )
   {
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      qwtw_wheel->setRange( guinier_mint, guinier_maxt,
                            ( guinier_maxt - guinier_mint ) / UHSH_G_WHEEL_RES );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setValue( le_guinier_rg_t_start->text().toDouble() );
      qwtw_wheel->setEnabled( true );
   }
}

void US_Hydrodyn_Saxs_Hplc::guinier_rg_t_end_focus( bool hasFocus )
{
   if ( current_mode != MODE_GUINIER )
   {
      return;
   }
   if ( hasFocus )
   {
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      qwtw_wheel->setRange( guinier_mint, guinier_maxt,
                            ( guinier_maxt - guinier_mint ) / UHSH_G_WHEEL_RES );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setValue( le_guinier_rg_t_end->text().toDouble() );
      qwtw_wheel->setEnabled( true );
   }
}

void US_Hydrodyn_Saxs_Hplc::guinier_rg_rg_start_text( const QString & text )
{
   if ( current_mode != MODE_GUINIER )
   {
      return;
   }
   guinier_plot_rg->setAxisScale( QwtPlot::yLeft, text.toDouble(), le_guinier_rg_rg_end->text().toDouble() );
   guinier_plot_rg->replot();
   if ( guinier_plot_rg_zoomer )
   {
      delete guinier_plot_rg_zoomer;
   }
   guinier_plot_rg_zoomer = new ScrollZoomer(guinier_plot_rg->canvas());
   guinier_plot_rg_zoomer->setRubberBandPen(QPen(Qt::yellow, 0, Qt::DotLine));
#ifndef QT4
   guinier_plot_rg_zoomer->setCursorLabelPen(QPen(Qt::yellow));
#endif
}

void US_Hydrodyn_Saxs_Hplc::guinier_rg_rg_end_text( const QString & text )
{
   if ( current_mode != MODE_GUINIER )
   {
      return;
   }
   guinier_plot_rg->setAxisScale( QwtPlot::yLeft, le_guinier_rg_rg_start->text().toDouble(), text.toDouble() );
   guinier_plot_rg->replot();
   if ( guinier_plot_rg_zoomer )
   {
      delete guinier_plot_rg_zoomer;
   }
   guinier_plot_rg_zoomer = new ScrollZoomer(guinier_plot_rg->canvas());
   guinier_plot_rg_zoomer->setRubberBandPen(QPen(Qt::yellow, 0, Qt::DotLine));
#ifndef QT4
   guinier_plot_rg_zoomer->setCursorLabelPen(QPen(Qt::yellow));
#endif
}

#define UHSH_MAX_RG 1000

void US_Hydrodyn_Saxs_Hplc::guinier_rg_rg_start_focus( bool hasFocus )
{
   if ( current_mode != MODE_GUINIER )
   {
      return;
   }
   if ( hasFocus )
   {
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      qwtw_wheel->setRange( 0, UHSH_MAX_RG,
                            UHSH_MAX_RG  / UHSH_G_WHEEL_RES );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setValue( le_guinier_rg_rg_start->text().toDouble() );
      qwtw_wheel->setEnabled( true );
   }
}

void US_Hydrodyn_Saxs_Hplc::guinier_rg_rg_end_focus( bool hasFocus )
{
   if ( current_mode != MODE_GUINIER )
   {
      return;
   }
   if ( hasFocus )
   {
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      qwtw_wheel->setRange( 0, UHSH_MAX_RG,
                            UHSH_MAX_RG  / UHSH_G_WHEEL_RES );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setValue( le_guinier_rg_rg_end->text().toDouble() );
      qwtw_wheel->setEnabled( true );
   }
}

// --- baseline ---
void US_Hydrodyn_Saxs_Hplc::baseline_start()
{
   org_baseline_start_s     = le_baseline_start_s    ->text().toDouble();
   org_baseline_start       = le_baseline_start      ->text().toDouble();
   org_baseline_start_e     = le_baseline_start_e    ->text().toDouble();
   org_baseline_end_s       = le_baseline_end_s      ->text().toDouble();
   org_baseline_end         = le_baseline_end        ->text().toDouble();
   org_baseline_end_e       = le_baseline_end_e      ->text().toDouble();
   org_baseline_start_zero  = cb_baseline_start_zero ->isChecked();

   le_last_focus = (mQLineEdit *) 0;
   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      if ( lb_files->isSelected( i ) )
      {
         wheel_file = lb_files->text( i );
         break;
      }
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

#ifndef QT4
   plot_dist->setCurvePen( plotted_curves[ wheel_file ], QPen( Qt::cyan, use_line_width, SolidLine));
#else
   plotted_curves[ wheel_file ]->setPen( QPen( Qt::cyan, use_line_width, Qt::SolidLine ) );
#endif

   // baseline_mode = true;
   mode_select( MODE_BASELINE );
   running       = true;
   qwtw_wheel->setRange( f_qs[ wheel_file ][ 0 ], 
                         f_qs[ wheel_file ].back(), 
                         ( f_qs[ wheel_file ].back() - f_qs[ wheel_file ][ 0 ] ) / UHSH_WHEEL_RES );

   double q_len       = f_qs[ wheel_file ].back() - f_qs[ wheel_file ][ 0 ];
   double q_len_delta = q_len * 0.05;

   if ( le_baseline_start_s->text().isEmpty() ||
        le_baseline_start_s->text() == "0" ||
        le_baseline_start_s->text().toDouble() < f_qs[ wheel_file ][ 0 ] )
   {
      disconnect( le_baseline_start_s, SIGNAL( textChanged( const QString & ) ), 0, 0 );
      le_baseline_start_s->setText( QString( "%1" ).arg( f_qs[ wheel_file ][ 0 ] ) );
      connect( le_baseline_start_s, SIGNAL( textChanged( const QString & ) ), SLOT( baseline_start_s_text( const QString & ) ) );
   }

   if ( le_baseline_start->text().isEmpty() ||
        le_baseline_start->text() == "0" ||
        le_baseline_start->text().toDouble() < f_qs[ wheel_file ][ 0 ] )
   {
      disconnect( le_baseline_start, SIGNAL( textChanged( const QString & ) ), 0, 0 );
      le_baseline_start->setText( QString( "%1" ).arg( f_qs[ wheel_file ][ 0 ] + q_len_delta ) );
      connect( le_baseline_start, SIGNAL( textChanged( const QString & ) ), SLOT( baseline_start_text( const QString & ) ) );
   }

   if ( le_baseline_start_e->text().isEmpty() ||
        le_baseline_start_e->text() == "0" ||
        le_baseline_start_e->text().toDouble() < f_qs[ wheel_file ][ 0 ] )
   {
      disconnect( le_baseline_start_e, SIGNAL( textChanged( const QString & ) ), 0, 0 );
      le_baseline_start_e->setText( QString( "%1" ).arg( f_qs[ wheel_file ][ 0 ] + 2e0 * q_len_delta) );
      connect( le_baseline_start_e, SIGNAL( textChanged( const QString & ) ), SLOT( baseline_start_e_text( const QString & ) ) );
   }

   if ( le_baseline_end_s->text().isEmpty() ||
        le_baseline_end_s->text() == "0" ||
        le_baseline_end_s->text().toDouble() > f_qs[ wheel_file ].back() )
   {
      disconnect( le_baseline_end_s, SIGNAL( textChanged( const QString & ) ), 0, 0 );
      le_baseline_end_s->setText( QString( "%1" ).arg( f_qs[ wheel_file ].back() - 2e0 * q_len_delta ) );
      connect( le_baseline_end_s, SIGNAL( textChanged( const QString & ) ), SLOT( baseline_end_s_text( const QString & ) ) );
   }

   if ( le_baseline_end->text().isEmpty() ||
        le_baseline_end->text() == "0" ||
        le_baseline_end->text().toDouble() > f_qs[ wheel_file ].back() )
   {
      disconnect( le_baseline_end, SIGNAL( textChanged( const QString & ) ), 0, 0 );
      le_baseline_end->setText( QString( "%1" ).arg( f_qs[ wheel_file ].back() - q_len_delta ) );
      connect( le_baseline_end, SIGNAL( textChanged( const QString & ) ), SLOT( baseline_end_text( const QString & ) ) );
   }

   if ( le_baseline_end_e->text().isEmpty() ||
        le_baseline_end_e->text() == "0" ||
        le_baseline_end_e->text().toDouble() > f_qs[ wheel_file ].back() )
   {
      disconnect( le_baseline_end_e, SIGNAL( textChanged( const QString & ) ), 0, 0 );
      le_baseline_end_e->setText( QString( "%1" ).arg( f_qs[ wheel_file ].back() ) );
      connect( le_baseline_end_e, SIGNAL( textChanged( const QString & ) ), SLOT( baseline_end_e_text( const QString & ) ) );
   }

   baseline_init_markers();
   replot_baseline();
   disable_all();
   baseline_enables();
}

void US_Hydrodyn_Saxs_Hplc::baseline_enables()
{
   pb_baseline_start      ->setEnabled( false );
   pb_wheel_cancel        ->setEnabled( true );
   pb_wheel_save          ->setEnabled( 
                                    le_baseline_start_s->text().toDouble() != org_baseline_start_s ||
                                    le_baseline_start  ->text().toDouble() != org_baseline_start   ||
                                    le_baseline_start_e->text().toDouble() != org_baseline_start_e ||
                                    le_baseline_end_s  ->text().toDouble() != org_baseline_end_s   ||
                                    le_baseline_end    ->text().toDouble() != org_baseline_end     ||   
                                    le_baseline_end_e  ->text().toDouble() != org_baseline_end_e   ||
                                    cb_baseline_start_zero->isChecked()    != org_baseline_start_zero
                                    );
   cb_baseline_start_zero ->setEnabled( true );
   if ( cb_baseline_start_zero->isChecked() )
   {
      le_baseline_start_s->hide();
      le_baseline_start  ->hide();
      le_baseline_start_e->hide();
   } else {
      le_baseline_start_s->show();
      le_baseline_start  ->show();
      le_baseline_start_e->show();
   }

   le_baseline_start_s    ->setEnabled( !cb_baseline_start_zero->isChecked() );
   le_baseline_start      ->setEnabled( !cb_baseline_start_zero->isChecked() );
   le_baseline_start_e    ->setEnabled( !cb_baseline_start_zero->isChecked() );
   le_baseline_end_s      ->setEnabled( true );
   le_baseline_end        ->setEnabled( true );
   le_baseline_end_e      ->setEnabled( true );
   qwtw_wheel             ->setEnabled( 
                                    le_baseline_start_s->hasFocus() || 
                                    le_baseline_start  ->hasFocus() || 
                                    le_baseline_start_e->hasFocus() || 
                                    le_baseline_end_s  ->hasFocus() ||
                                    le_baseline_end    ->hasFocus() ||
                                    le_baseline_end_e  ->hasFocus()
                                    );
   pb_rescale             ->setEnabled( true );
   pb_view                ->setEnabled( true );
}

void US_Hydrodyn_Saxs_Hplc::set_baseline_start_zero()
{
   if ( cb_baseline_start_zero->isChecked() &&
        ( le_baseline_start_s->hasFocus() ||
          le_baseline_start  ->hasFocus() ||
          le_baseline_start_e->hasFocus() ) )
   {
      le_last_focus = (mQLineEdit *)0;
   }

   for ( unsigned int i = 0; i < ( unsigned int ) plotted_baseline.size(); i++ )
   {
#ifndef QT4
      plot_dist->removeCurve( plotted_baseline[ i ] );
#else
      plotted_baseline[ i ]->detach();
#endif
   }

   baseline_init_markers();
   replot_baseline();
   plot_dist->replot();
   baseline_enables();
}

static QColor start_color( 255, 165, 0 );
static QColor end_color  ( 255, 160, 122 );

void US_Hydrodyn_Saxs_Hplc::baseline_init_markers()
{
   gauss_delete_markers();

   plotted_markers.clear();
   plotted_baseline.clear();

   if ( !cb_baseline_start_zero->isChecked() )
   {
      gauss_add_marker( le_baseline_start_s->text().toDouble(), Qt::magenta, tr( "\nLFS\nStart"   ) );
      gauss_add_marker( le_baseline_start  ->text().toDouble(), Qt::red,     tr( "Start"          ) );
      gauss_add_marker( le_baseline_start_e->text().toDouble(), Qt::magenta, tr( "\n\n\nLFS\nEnd" ) );
   }
   gauss_add_marker( le_baseline_end_s  ->text().toDouble(), Qt::magenta, tr( "\nLFE\nStart"   ), Qt::AlignLeft | Qt::AlignTop );
   gauss_add_marker( le_baseline_end    ->text().toDouble(), Qt::red,     tr( "End"            ), Qt::AlignLeft | Qt::AlignTop );
   gauss_add_marker( le_baseline_end_e  ->text().toDouble(), Qt::magenta, tr( "\n\n\nLFE\nEnd" ), Qt::AlignLeft | Qt::AlignTop );

   if ( !suppress_replot )
   {
      plot_dist->replot();
   }
}

void US_Hydrodyn_Saxs_Hplc::replot_baseline()
{
   // cout << "replot baseline\n";
   // compute & plot baseline
   // baseline_slope =
   // baseline_intercept =
   // plot over range from start to end
   // also find closest f_qs[ wheel_file ] to start/end
   unsigned int before_start = 0;
   unsigned int after_start  = 1;
   unsigned int before_end   = 0;
   unsigned int after_end    = 1;

   double start_s = le_baseline_start_s->text().toDouble();
   double start   = le_baseline_start  ->text().toDouble();
   double start_e = le_baseline_start_e->text().toDouble();
   double end_s   = le_baseline_end_s  ->text().toDouble();
   double end     = le_baseline_end    ->text().toDouble();
   double end_e   = le_baseline_end_e  ->text().toDouble();

   vector < double > start_q;
   vector < double > start_I;

   vector < double > end_q;
   vector < double > end_I;

   {
      unsigned int i = 0;
      if ( cb_baseline_start_zero->isChecked() )
      {
         start_q.push_back( f_qs[ wheel_file ][ i ] );
         start = start_q[ 0 ];
         start_I.push_back( 0e0 );
      } else {
         if ( f_qs[ wheel_file ][ i ] >= start_s &&
              f_qs[ wheel_file ][ i ] <= start_e )
         {
            start_q.push_back( f_qs[ wheel_file ][ i ] );
            start_I.push_back( f_Is[ wheel_file ][ i ] );
         }
      }
      if ( f_qs[ wheel_file ][ i ] >= end_s &&
           f_qs[ wheel_file ][ i ] <= end_e )
      {
         end_q.push_back( f_qs[ wheel_file ][ i ] );
         end_I.push_back( f_Is[ wheel_file ][ i ] );
      }
   }

   for ( unsigned int i = 1; i < f_qs[ wheel_file ].size(); i++ )
   {
      if ( !cb_baseline_start_zero->isChecked() &&
           f_qs[ wheel_file ][ i ] >= start_s &&
           f_qs[ wheel_file ][ i ] <= start_e )
      {
         start_q.push_back( f_qs[ wheel_file ][ i ] );
         start_I.push_back( f_Is[ wheel_file ][ i ] );
      }
      if ( f_qs[ wheel_file ][ i ] >= end_s &&
           f_qs[ wheel_file ][ i ] <= end_e )
      {
         end_q.push_back( f_qs[ wheel_file ][ i ] );
         end_I.push_back( f_Is[ wheel_file ][ i ] );
      }

      if ( f_qs[ wheel_file ][ i - 1 ] <= start &&
           f_qs[ wheel_file ][ i     ] >= start )
      {
         before_start = i - 1;
         after_start  = i;
      }
      if ( f_qs[ wheel_file ][ i - 1 ] <= end &&
           f_qs[ wheel_file ][ i     ] >= end )
      {
         before_end = i - 1;
         after_end  = i;
      }
   }

   bool   set_start = ( start_q.size() > 1 );
   bool   set_end   = ( end_q  .size() > 1 );
   double start_intercept = 0e0;
   double start_slope     = 0e0;
   double end_intercept   = 0e0;
   double end_slope       = 0e0;

   double start_y;
   double end_y;

   double siga;
   double sigb;
   double chi2;

   if ( set_start && set_end )
   {
      // linear fit on each
      usu->linear_fit( start_q, start_I, start_intercept, start_slope, siga, sigb, chi2 );
      usu->linear_fit( end_q  , end_I  , end_intercept  , end_slope  , siga, sigb, chi2 );

      // find intercepts for baseline

      start_y = start_intercept + start_slope * start;
      end_y   = end_intercept   + end_slope   * end;
   } else {
      if ( set_start )
      {
         usu->linear_fit( start_q, start_I, start_intercept, start_slope, siga, sigb, chi2 );

         start_y = start_intercept + start_slope * start;

         double end_t;

         if ( f_qs[ wheel_file ][ after_end  ] != f_qs[ wheel_file ][ before_end ] )
         {
            end_t = ( f_qs[ wheel_file ][ after_end ] - end )
               / ( f_qs[ wheel_file ][ after_end  ] -
                   f_qs[ wheel_file ][ before_end ] );
         } else {
            end_t = 0.5e0;
         }

         end_y = 
            ( end_t ) * f_Is[ wheel_file ][ before_end ] +
            ( 1e0 - end_t ) * f_Is[ wheel_file ][ after_end ];
      } else {
         if ( set_end )
         {
            usu->linear_fit( end_q, end_I, end_intercept, end_slope, siga, sigb, chi2 );

            end_y = end_intercept + end_slope * end;

            double start_t;
            if ( f_qs[ wheel_file ][ after_start  ] != f_qs[ wheel_file ][ before_start ] )
            {
               start_t = 
                  ( f_qs[ wheel_file ][ after_start ] - start )
                  / ( f_qs[ wheel_file ][ after_start  ] -
                      f_qs[ wheel_file ][ before_start ] );
            } else {
               start_t = 0.5e0;
            }

            start_y = 
               ( start_t ) * f_Is[ wheel_file ][ before_start ] +
               ( 1e0 - start_t ) * f_Is[ wheel_file ][ after_start ];
            
         } else {
            // for now, we are going to do this way for all conditions

            double start_t;
            double end_t;

            if ( f_qs[ wheel_file ][ after_start  ] != f_qs[ wheel_file ][ before_start ] )
            {
               start_t = 
                  ( f_qs[ wheel_file ][ after_start ] - start )
                  / ( f_qs[ wheel_file ][ after_start  ] -
                      f_qs[ wheel_file ][ before_start ] );
            } else {
               start_t = 0.5e0;
            }
      
            if ( f_qs[ wheel_file ][ after_end  ] != f_qs[ wheel_file ][ before_end ] )
            {
               end_t = ( f_qs[ wheel_file ][ after_end ] - end )
                  / ( f_qs[ wheel_file ][ after_end  ] -
                      f_qs[ wheel_file ][ before_end ] );
            } else {
               end_t = 0.5e0;
            }

            start_y = 
               ( start_t ) * f_Is[ wheel_file ][ before_start ] +
               ( 1e0 - start_t ) * f_Is[ wheel_file ][ after_start ];

            end_y = 
               ( end_t ) * f_Is[ wheel_file ][ before_end ] +
               ( 1e0 - end_t ) * f_Is[ wheel_file ][ after_end ];
         }
      }
   }

   if ( cb_baseline_start_zero->isChecked() )
   {
      start_y = 0e0;
   }

   baseline_slope     = ( end_y - start_y ) / ( end - start );
   baseline_intercept = 
      ( ( start_y + end_y ) -
        baseline_slope * ( start + end ) ) * 5e-1;

   vector < double > x( 2 );
   vector < double > y( 2 );

   x[ 0 ] = f_qs[ wheel_file ][ 0 ];
   x[ 1 ] = f_qs[ wheel_file ].back();

   y[ 0 ] = baseline_slope * x[ 0 ] + baseline_intercept;
   y[ 1 ] = baseline_slope * x[ 1 ] + baseline_intercept;

   // remove any baseline curves

   for ( unsigned int i = 0; i < ( unsigned int ) plotted_baseline.size(); i++ )
   {
#ifndef QT4
      plot_dist->removeCurve( plotted_baseline[ i ] );
#else
      plotted_baseline[ i ]->detach();
#endif
   }
   plotted_baseline.clear();

   // the baseline
   {
      // cout << QString( "baseline slope %1 intercept %2\n" ).arg( baseline_slope ).arg( baseline_intercept );
      // printvector( "baseline x", x );
      // printvector( "baseline y", y );

#ifndef QT4
      long curve;
      curve = plot_dist->insertCurve( "baseline" );
      plot_dist->setCurveStyle( curve, QwtCurve::Lines );
#else
      QwtPlotCurve *curve = new QwtPlotCurve( "baseline" );
      curve->setStyle( QwtPlotCurve::Lines );
#endif

      plotted_baseline.push_back( curve );

#ifndef QT4
      plot_dist->setCurvePen( curve, QPen( Qt::green , use_line_width, Qt::DashLine ) );
      plot_dist->setCurveData( plotted_baseline[ 0 ],
                               (double *)&x[ 0 ],
                               (double *)&y[ 0 ],
                               2
                               );
#else
      curve->setPen( QPen( Qt::green, use_line_width, Qt::DashLine ) );
      plotted_baseline[ 0 ]->setData(
                                     (double *)&x[ 0 ],
                                     (double *)&y[ 0 ],
                                     2
                                     );
      curve->attach( plot_dist );
#endif
   }
   if ( set_start )
   {
      y[ 0 ] = start_slope * f_qs[ wheel_file ][ 0 ] + start_intercept;
      y[ 1 ] = start_slope * f_qs[ wheel_file ].back() + start_intercept;

      // cout << QString( "start slope %1 intercept %2\n" ).arg( start_slope ).arg( start_intercept );
      // printvector( "start x", x );
      // printvector( "start y", y );

#ifndef QT4
      long curve;
      curve = plot_dist->insertCurve( "baseline s" );
      plot_dist->setCurveStyle( curve, QwtCurve::Lines );
#else
      QwtPlotCurve *curve = new QwtPlotCurve( "baseline s" );
      curve->setStyle( QwtPlotCurve::Lines );
#endif

      plotted_baseline.push_back( curve );

#ifndef QT4
      plot_dist->setCurvePen( curve, QPen( start_color, use_line_width, Qt::DashLine ) );
      plot_dist->setCurveData( plotted_baseline.back(),
                               (double *)&x[ 0 ],
                               (double *)&y[ 0 ],
                               2
                               );
#else
      curve->setPen( QPen( start_color, use_line_width, Qt::DashLine ) );
      plotted_baseline.back()->setData(
                                       (double *)&x[ 0 ],
                                       (double *)&y[ 0 ],
                                       2
                                     );
      curve->attach( plot_dist );
#endif
   }

   if ( set_end )
   {
      y[ 0 ] = end_slope * f_qs[ wheel_file ][ 0 ] + end_intercept;
      y[ 1 ] = end_slope * f_qs[ wheel_file ].back() + end_intercept;

      // cout << QString( "end slope %1 intercept %2\n" ).arg( end_slope ).arg( end_intercept );
      // printvector( "end x", x );
      // printvector( "end y", y );

#ifndef QT4
      long curve;
      curve = plot_dist->insertCurve( "baseline e" );
      plot_dist->setCurveStyle( curve, QwtCurve::Lines );
#else
      QwtPlotCurve *curve = new QwtPlotCurve( "baseline e" );
      curve->setStyle( QwtPlotCurve::Lines );
#endif

      plotted_baseline.push_back( curve );

#ifndef QT4
      plot_dist->setCurvePen( curve, QPen( end_color, use_line_width, Qt::DashLine ) );
      plot_dist->setCurveData( plotted_baseline.back(),
                               (double *)&x[ 0 ],
                               (double *)&y[ 0 ],
                               2
                               );
#else
      curve->setPen( QPen( end_color, use_line_width, Qt::DashLine ) );
      plotted_baseline.back()->setData(
                                       (double *)&x[ 0 ],
                                       (double *)&y[ 0 ],
                                       2
                                     );
      curve->attach( plot_dist );
#endif
   }

   if ( !suppress_replot )
   {
      plot_dist->replot();
   }
}

void US_Hydrodyn_Saxs_Hplc::baseline_apply()
{
   int smoothing = 0;
   bool integral = ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "hplc_bl_integral" ] == "true";
   if ( integral && U_EXPT )
   {
      smoothing = ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "hplc_bl_smooth" ].toInt();
   }
   bool save_bl = ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "hplc_bl_save" ] == "true";
   unsigned int reps = 0;
   if ( integral )
   {
      reps = ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "hplc_bl_reps" ].toInt();
   }
   
   baseline_apply( all_selected_files(), integral, smoothing, save_bl, reps );
}

void US_Hydrodyn_Saxs_Hplc::baseline_apply( QStringList files, bool integral, int smoothing, bool save_bl, unsigned int reps )
{
   map < QString, bool > current_files;
   for ( int i = 0; i < (int)lb_files->numRows(); i++ )
   {
      current_files[ lb_files->text( i ) ] = true;
   }

   map < QString, bool > select_files;

   if ( integral && save_bl && files.size() > 3 )
   {
      switch ( QMessageBox::warning(this, 
                                    caption(),
                                    QString( tr( "Please note:\n\n"
                                                 "You have selected to produce separate baseline curves and have %1 files selected\n"
                                                 "This will generate upto %2 separate baseline curves in addition to your %3 baseline subtracted files\n"
                                                 "What would you like to do?\n" ) )
                                    .arg( files.size() )
                                    .arg( (int) reps * (int)  files.size() )
                                    .arg( files.size() ),
                                    tr( "&Generate them anyway" ), 
                                    tr( "&Temporarily turn off production of separate baseline curves" ), 
                                    tr( "&Quit" ), 
                                    0, // Stop == button 0
                                    0 // Escape == button 0
                                    ) )
      {
      case 0 : // generate them anyway
         break;
      case 1 : // turn off production
         save_bl = false;
         break;
      case 2 : // quit
         return;
         break;
      }
   }

   double start_s = le_baseline_start_s->text().toDouble();
   double start   = le_baseline_start  ->text().toDouble();
   double start_e = le_baseline_start_e->text().toDouble();
   double end_s   = le_baseline_end_s  ->text().toDouble();
   double end     = le_baseline_end    ->text().toDouble();
   double end_e   = le_baseline_end_e  ->text().toDouble();

   // redo this to compute from best linear fit over ranges
   QStringList messages;

   for ( unsigned int i = 0; i < ( unsigned int ) files.size(); i++ )
   {

      unsigned int before_start = 0;
      unsigned int after_start  = 1;
      unsigned int before_end   = 0;
      unsigned int after_end    = 1;

      vector < double > start_q;
      vector < double > start_I;

      vector < double > end_q;
      vector < double > end_I;


      {
         unsigned int j = 0;
         if ( cb_baseline_start_zero->isChecked() )
         {
            start_q.push_back( f_qs[ files[ i ] ][ j ] );
            start = start_q[ 0 ];
            start_I.push_back( 0e0 );
         } else {
            if ( f_qs[ files[ i ] ][ j ] >= start_s &&
                 f_qs[ files[ i ] ][ j ] <= start_e )
            {
               start_q.push_back( f_qs[ files[ i ] ][ j ] );
               start_I.push_back( f_Is[ files[ i ] ][ j ] );
            }
         }
         if ( f_qs[ files[ i ] ][ j ] >= end_s &&
              f_qs[ files[ i ] ][ j ] <= end_e )
         {
            end_q.push_back( f_qs[ files[ i ] ][ j ] );
            end_I.push_back( f_Is[ files[ i ] ][ j ] );
         }
      }

      for ( unsigned int j = 1; j < f_qs[ files[ i ] ].size(); j++ )
      {
         if ( !cb_baseline_start_zero->isChecked() &&
              f_qs[ files[ i ] ][ j ] >= start_s &&
              f_qs[ files[ i ] ][ j ] <= start_e )
         {
            start_q.push_back( f_qs[ files[ i ] ][ j ] );
            start_I.push_back( f_Is[ files[ i ] ][ j ] );
         }
         if ( f_qs[ files[ i ] ][ j ] >= end_s &&
              f_qs[ files[ i ] ][ j ] <= end_e )
         {
            end_q.push_back( f_qs[ files[ i ] ][ j ] );
            end_I.push_back( f_Is[ files[ i ] ][ j ] );
         }

         if ( f_qs[ files[ i ] ][ j - 1 ] <= start &&
              f_qs[ files[ i ] ][ j     ] >= start )
         {
            before_start = j - 1;
            after_start  = j;
         }
         if ( f_qs[ files[ i ] ][ j - 1 ] <= end &&
              f_qs[ files[ i ] ][ j     ] >= end )
         {
            before_end = j - 1;
            after_end  = j;
         }
      }

      bool   set_start = ( start_q.size() > 1 );
      bool   set_end   = ( end_q  .size() > 1 );
      double start_intercept = 0e0;
      double start_slope     = 0e0;
      double end_intercept   = 0e0;
      double end_slope       = 0e0;

      double start_y;
      double end_y;

      double siga;
      double sigb;
      double chi2;

      if ( set_start && set_end )
      {
         // linear fit on each
         usu->linear_fit( start_q, start_I, start_intercept, start_slope, siga, sigb, chi2 );
         usu->linear_fit( end_q  , end_I  , end_intercept  , end_slope  , siga, sigb, chi2 );

         // find intercepts for baseline

         start_y = start_intercept + start_slope * start;
         end_y   = end_intercept   + end_slope   * end;
      } else {
         if ( set_start )
         {
            usu->linear_fit( start_q, start_I, start_intercept, start_slope, siga, sigb, chi2 );

            start_y = start_intercept + start_slope * start;

            double end_t;

            if ( f_qs[ files[ i ] ][ after_end  ] != f_qs[ files[ i ] ][ before_end ] )
            {
               end_t = ( f_qs[ files[ i ] ][ after_end ] - end )
                  / ( f_qs[ files[ i ] ][ after_end  ] -
                      f_qs[ files[ i ] ][ before_end ] );
            } else {
               end_t = 0.5e0;
            }

            end_y = 
               ( end_t ) * f_Is[ files[ i ] ][ before_end ] +
               ( 1e0 - end_t ) * f_Is[ files[ i ] ][ after_end ];
         } else {
            if ( set_end )
            {
               usu->linear_fit( end_q, end_I, end_intercept, end_slope, siga, sigb, chi2 );

               end_y = end_intercept + end_slope * end;

               double start_t;
               if ( f_qs[ files[ i ] ][ after_start  ] != f_qs[ files[ i ] ][ before_start ] )
               {
                  start_t = 
                     ( f_qs[ files[ i ] ][ after_start ] - start )
                     / ( f_qs[ files[ i ] ][ after_start  ] -
                         f_qs[ files[ i ] ][ before_start ] );
               } else {
                  start_t = 0.5e0;
               }

               start_y = 
                  ( start_t ) * f_Is[ files[ i ] ][ before_start ] +
                  ( 1e0 - start_t ) * f_Is[ files[ i ] ][ after_start ];
            
            } else {
               // for now, we are going to do this way for all conditions

               double start_t;
               double end_t;

               if ( f_qs[ files[ i ] ][ after_start  ] != f_qs[ files[ i ] ][ before_start ] )
               {
                  start_t = 
                     ( f_qs[ files[ i ] ][ after_start ] - start )
                     / ( f_qs[ files[ i ] ][ after_start  ] -
                         f_qs[ files[ i ] ][ before_start ] );
               } else {
                  start_t = 0.5e0;
               }
      
               if ( f_qs[ files[ i ] ][ after_end  ] != f_qs[ files[ i ] ][ before_end ] )
               {
                  end_t = ( f_qs[ files[ i ] ][ after_end ] - end )
                     / ( f_qs[ files[ i ] ][ after_end  ] -
                         f_qs[ files[ i ] ][ before_end ] );
               } else {
                  end_t = 0.5e0;
               }

               start_y = 
                  ( start_t ) * f_Is[ files[ i ] ][ before_start ] +
                  ( 1e0 - start_t ) * f_Is[ files[ i ] ][ after_start ];

               end_y = 
                  ( end_t ) * f_Is[ files[ i ] ][ before_end ] +
                  ( 1e0 - end_t ) * f_Is[ files[ i ] ][ after_end ];
            }
         }
      }

      if ( cb_baseline_start_zero->isChecked() )
      {
         start_y = 0e0;
      }

      vector < double > bl_I = f_Is[ files[ i ] ];
      int ext = 0;
      QString bl_name = files[ i ];


      if ( integral )
      {
         double delta_bl = end_y - start_y;

         if ( smoothing )
         {
            US_Saxs_Util usu;
            if ( !usu.smooth( f_Is[ files[ i ] ], bl_I, smoothing ) )
            {
               bl_I = f_Is[ files[ i ] ];
               messages << QString( tr( "Error: smoothing error on %1" ) ).arg( files[ i ] );
               editor_msg( "red", messages.back() );
            }
         }
         vector < double > new_I = f_Is[ files[ i ] ];
         vector < double > last_bl( new_I.size() );
         for ( unsigned int j = 0; j < ( unsigned int ) bl_I.size(); j++ )
         {
            last_bl[ j ] = start_y;
         }

         unsigned int this_reps = 0;
         double I_tot;

         double alpha = 0e0;
         double alpha_epsilon = 
            ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "hplc_bl_alpha" ) ?
            ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "hplc_bl_alpha" ].toDouble() : 5e-3;

         double last_alpha = 0e0;
         vector < double > bl = last_bl;

         do {
            last_alpha = alpha;
            this_reps++;
            I_tot = 0e0;

            // note: this "if" could be separated into 2 loops
            // removing one of the condition checks

            for ( unsigned int j = 0; j < ( unsigned int ) bl_I.size(); j++ )
            {
               if ( f_qs[ files[ i ] ][ j ] >= start &&
                    f_qs[ files[ i ] ][ j ] <= end )
               {
                  I_tot += bl_I[ j ] - bl[ j ];
               }
            }

            if ( I_tot > 0e0 )
            {
               alpha = delta_bl / I_tot;

               editor_msg( "dark blue", QString( tr( "iteration %1 delta_Bl %2 Itot %3 alpha %4" ) )
                           .arg( this_reps )
                           .arg( delta_bl ).arg( I_tot ).arg( alpha ) );

               vector < double > D( bl.size() );

               for ( unsigned int j = 0; j < bl_I.size(); j++ )
               {
                  if ( f_qs[ files[ i ] ][ j ] >= start &&
                       f_qs[ files[ i ] ][ j ] <= end )
                  {
                     D[ j ] = alpha * ( f_Is[ files[ i ] ][ j ] - bl[ j ] );
                  }
               }
                  
               for ( unsigned int j = 0; j < bl_I.size(); j++ )
               {
                  if ( f_qs[ files[ i ] ][ j ] < start )
                  {
                     bl[ j ] = start_y;
                  } else {
                     if ( f_qs[ files[ i ] ][ j ] <= end )
                     {
                        bl[ j ] = start_y;
                        for ( unsigned int k = 0; k <= j ; ++k )
                        {
                           bl[ j ] += D[ k ];
                        }
                     } else {
                        bl[ j ] = end_y;
                     }
                  }
                  new_I[ j ] = f_Is[ files[ i ] ][ j ] - bl[ j ];
               }
            } else {
               for ( unsigned int j = 0; j < bl_I.size(); j++ )
               {
                  new_I[ j ] = f_Is[ files[ i ] ][ j ] - bl[ j ];
               }
               messages << QString( tr( "Warning: the integral of %1 was less than or equal to zero => constant baseline" ) ).arg( files[ i ] );
               editor_msg( "dark red", messages.back() );
            }

            if ( save_bl )
            {
               add_plot( QString( "BI_%1-%2" ).arg( files[ i ] ).arg( this_reps ), f_qs[ files[ i ] ], bl, true, false );
            }

         } while ( this_reps < reps && alpha > 0e0 && ( fabs( alpha - last_alpha ) / alpha ) > alpha_epsilon );

         bl_I = new_I;

         //          cout << QString( 
         //                          "delta_bl   %1\n"
         //                          "integral_I %2\n"
         //                           )
         //             .arg( delta_bl )
         //             .arg( integral_I )
         //             ;

         bl_name += QString( "-bi%1-%2s" ).arg( delta_bl, 0, 'g', 6 ).arg( alpha, 0, 'g', 6 ).replace( ".", "_" );
         while ( current_files.count( bl_name ) )
         {
            bl_name = files[ i ] + QString( "-bi%1-%2s-%3" ).arg( delta_bl, 0, 'g', 6 ).arg( alpha, 0, 'g', 6 ).arg( ++ext ).replace( ".", "_" );
         }

      } else {

         baseline_slope     = ( end_y - start_y ) / ( end - start );
         baseline_intercept = 
            ( ( start_y + end_y ) -
              baseline_slope * ( start + end ) ) * 5e-1;

         for ( unsigned int j = 0; j < bl_I.size(); j++ )
         {
            bl_I[ j ] -= baseline_slope * f_qs[ files[ i ] ][ j ] + baseline_intercept;
         }
         bl_name += QString( "-bl%1-%2s" ).arg( baseline_slope, 0, 'g', 8 ).arg( baseline_intercept, 0, 'g', 8 ).replace( ".", "_" );
         while ( current_files.count( bl_name ) )
         {
            bl_name = files[ i ] + QString( "-bl%1-%2s-%3" ).arg( baseline_slope, 0, 'g', 8 ).arg( baseline_intercept, 0, 'g', 8 ).arg( ++ext ).replace( ".", "_" );
         }
      }

      select_files[ bl_name ] = true;

      lb_created_files->insertItem( bl_name );
      lb_created_files->setBottomItem( lb_created_files->numRows() - 1 );
      lb_files->insertItem( bl_name );
      lb_files->setBottomItem( lb_files->numRows() - 1 );
      created_files_not_saved[ bl_name ] = true;
   
      f_pos       [ bl_name ] = f_qs.size();
      f_qs_string [ bl_name ] = f_qs_string[ files[ i ] ];
      f_qs        [ bl_name ] = f_qs       [ files[ i ] ];
      f_Is        [ bl_name ] = bl_I;
      f_errors    [ bl_name ] = f_errors   [ files[ i ] ];
      f_is_time   [ bl_name ] = f_is_time  [ files[ i ] ];
      f_psv       [ bl_name ] = f_psv.count( files[ i ] ) ? f_psv[ files[ i ] ] : 0e0;
      f_I0se      [ bl_name ] = f_I0se.count( files[ i ] ) ? f_I0se[ files[ i ] ] : 0e0;
      f_conc      [ bl_name ] = f_conc.count( files[ i ] ) ? f_conc[ files[ i ] ] : 0e0;
      {
         vector < double > tmp;
         f_gaussians  [ bl_name ] = tmp;
      }
      editor_msg( "gray", QString( "Created %1\n" ).arg( bl_name ) );
   }

   if ( messages.size() )
   {
      QStringList qsl;
      for ( int i = 0; i < (int)messages.size() && i < 15; i++ )
      {
         qsl << messages[ i ];
      }

      if ( qsl.size() < messages.size() )
      {
         qsl << QString( tr( "... and %1 more not listed" ) ).arg( messages.size() - qsl.size() );
      }

      QMessageBox::warning(
                           this,
                           this->caption() + tr(": Baseline apply" ),
                           QString( tr( "Please note:\n\n"
                                        "These errors were produced when performing the baseline correction\n"
                                        "%1\n"
                                        ) )
                           .arg( qsl.join( "\n" ) ),
                           QMessageBox::Ok | QMessageBox::Default,
                           QMessageBox::NoButton
                           );
   }

   disable_updates = true;

   if ( select_files.size() != 1 )
   {
      lb_files->clearSelection();
   }

   for ( int i = 0; i < (int)lb_files->numRows(); i++ )
   {
      if ( select_files.count( lb_files->text( i ) ) )
      {
         lb_files->setSelected( i, true );
      }
   }

   disable_updates = false;
   rescale();
   plot_files();
   update_enables();
}

void US_Hydrodyn_Saxs_Hplc::baseline_start_s_text( const QString & text )
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
   replot_baseline();
   if ( !suppress_replot )
   {
      plot_dist->replot();
   }
   baseline_enables();
}

void US_Hydrodyn_Saxs_Hplc::baseline_start_text( const QString & text )
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
   replot_baseline();
   if ( !suppress_replot )
   {
      plot_dist->replot();
   }
   baseline_enables();
}

void US_Hydrodyn_Saxs_Hplc::baseline_start_e_text( const QString & text )
{
#ifndef QT4
   plot_dist->setMarkerPos( plotted_markers[ 2 ], text.toDouble(), 0e0 );
#else
   plotted_markers[ 2 ]->setXValue( text.toDouble() );
#endif
   if ( qwtw_wheel->value() != text.toDouble() )
   {
      qwtw_wheel->setValue( text.toDouble() );
   }
   replot_baseline();
   plot_dist->replot();
   baseline_enables();
}

void US_Hydrodyn_Saxs_Hplc::baseline_end_s_text( const QString & text )
{
   int pos = cb_baseline_start_zero->isChecked() ? 0 : 3;
#ifndef QT4
   plot_dist->setMarkerPos( plotted_markers[ pos ], text.toDouble(), 0e0 );
#else
   plotted_markers[ pos ]->setXValue( text.toDouble() );
#endif
   if ( qwtw_wheel->value() != text.toDouble() )
   {
      qwtw_wheel->setValue( text.toDouble() );
   }
   replot_baseline();
   if ( !suppress_replot )
   {
      plot_dist->replot();
   }
   baseline_enables();
}

void US_Hydrodyn_Saxs_Hplc::baseline_end_text( const QString & text )
{
   int pos = cb_baseline_start_zero->isChecked() ? 1 : 4;
#ifndef QT4
   plot_dist->setMarkerPos( plotted_markers[ pos ], text.toDouble(), 0e0 );
#else
   plotted_markers[ pos ]->setXValue( text.toDouble() );
#endif
   if ( qwtw_wheel->value() != text.toDouble() )
   {
      qwtw_wheel->setValue( text.toDouble() );
   }
   replot_baseline();
   if ( !suppress_replot )
   {
      plot_dist->replot();
   }
   baseline_enables();
}

void US_Hydrodyn_Saxs_Hplc::baseline_end_e_text( const QString & text )
{
   int pos = cb_baseline_start_zero->isChecked() ? 2 : 5;
#ifndef QT4
   plot_dist->setMarkerPos( plotted_markers[ pos ], text.toDouble(), 0e0 );
#else
   plotted_markers[ pos ]->setXValue( text.toDouble() );
#endif
   if ( qwtw_wheel->value() != text.toDouble() )
   {
      qwtw_wheel->setValue( text.toDouble() );
   }
   replot_baseline();
   if ( !suppress_replot )
   {
      plot_dist->replot();
   }
   baseline_enables();
}


void US_Hydrodyn_Saxs_Hplc::baseline_start_s_focus( bool hasFocus )
{
   // cout << QString( "baseline_start_s_focus %1\n" ).arg( hasFocus ? "true" : "false" );
   if ( hasFocus )
   {
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      qwtw_wheel->setRange( f_qs[ wheel_file ][ 0 ], f_qs[ wheel_file ].back(), 
                            ( f_qs[ wheel_file ].back() - f_qs[ wheel_file ][ 0 ] ) / UHSH_WHEEL_RES );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setValue( le_baseline_start_s->text().toDouble() );
      qwtw_wheel->setEnabled( true );
   }
}

void US_Hydrodyn_Saxs_Hplc::baseline_start_focus( bool hasFocus )
{
   // cout << QString( "baseline_start_focus %1\n" ).arg( hasFocus ? "true" : "false" );
   if ( hasFocus )
   {
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      qwtw_wheel->setRange( f_qs[ wheel_file ][ 0 ], f_qs[ wheel_file ].back(), 
                            ( f_qs[ wheel_file ].back() - f_qs[ wheel_file ][ 0 ] ) / UHSH_WHEEL_RES );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setValue( le_baseline_start->text().toDouble() );
      qwtw_wheel->setEnabled( true );
   }
}

void US_Hydrodyn_Saxs_Hplc::baseline_start_e_focus( bool hasFocus )
{
   // cout << QString( "baseline_start_e_focus %1\n" ).arg( hasFocus ? "true" : "false" );
   if ( hasFocus )
   {
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      qwtw_wheel->setRange( f_qs[ wheel_file ][ 0 ], f_qs[ wheel_file ].back(), 
                            ( f_qs[ wheel_file ].back() - f_qs[ wheel_file ][ 0 ] ) / UHSH_WHEEL_RES );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setValue( le_baseline_start_e->text().toDouble() );
      qwtw_wheel->setEnabled( true );
   }
}

void US_Hydrodyn_Saxs_Hplc::baseline_end_s_focus( bool hasFocus )
{
   // cout << QString( "baseline_end_s_focus %1\n" ).arg( hasFocus ? "true" : "false" );
   if ( hasFocus )
   {
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      qwtw_wheel->setRange( f_qs[ wheel_file ][ 0 ], 
                            f_qs[ wheel_file ].back(), 
                            ( f_qs[ wheel_file ].back() - f_qs[ wheel_file ][ 0 ] ) / UHSH_WHEEL_RES );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setValue( le_baseline_end_s->text().toDouble() );
      qwtw_wheel->setEnabled( true );
   }
}

void US_Hydrodyn_Saxs_Hplc::baseline_end_focus( bool hasFocus )
{
   // cout << QString( "baseline_end_focus %1\n" ).arg( hasFocus ? "true" : "false" );
   if ( hasFocus )
   {
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      qwtw_wheel->setRange( f_qs[ wheel_file ][ 0 ], 
                            f_qs[ wheel_file ].back(), 
                            ( f_qs[ wheel_file ].back() - f_qs[ wheel_file ][ 0 ] ) / UHSH_WHEEL_RES );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setValue( le_baseline_end->text().toDouble() );
      qwtw_wheel->setEnabled( true );
   }
}

void US_Hydrodyn_Saxs_Hplc::baseline_end_e_focus( bool hasFocus )
{
   // cout << QString( "baseline_end_e_focus %1\n" ).arg( hasFocus ? "true" : "false" );
   if ( hasFocus )
   {
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      qwtw_wheel->setRange( f_qs[ wheel_file ][ 0 ], 
                            f_qs[ wheel_file ].back(), 
                            ( f_qs[ wheel_file ].back() - f_qs[ wheel_file ][ 0 ] ) / UHSH_WHEEL_RES );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setValue( le_baseline_end_e->text().toDouble() );
      qwtw_wheel->setEnabled( true );
   }
}

// --- SCALE ----

void US_Hydrodyn_Saxs_Hplc::scale()
{
   le_last_focus = (mQLineEdit *) 0;

   bool any_selected = false;

   scale_selected    .clear();
   scale_q           .clear();
   scale_I           .clear();
   scale_e           .clear();
   scale_last_created.clear();

   if ( current_mode == MODE_TESTIQ )
   {
      if ( !testiq_make() ||
           !testiq_created_names.size() )
      {
         return;
      }
      for ( int i = 0; i < (int) rb_testiq_gaussians.size(); ++i )
      {
         rb_testiq_gaussians[ i ]->hide();
      }
      testiq_original_selection = all_selected_files_set();
      testiq_created_scale_names.clear();
      gauss_delete_markers();
      plotted_markers.clear();
      for ( int i = 0; i < (int) testiq_created_names.size(); ++i )
      {
         QString this_file = testiq_created_names[ i ];
         add_plot( this_file,
                   testiq_created_q[ this_file ],
                   testiq_created_I[ this_file ],
                   testiq_created_e[ this_file ],
                   false,
                   false );

         testiq_created_scale_names.insert( last_created_file );
      }
      set_selected( testiq_created_scale_names );
      plot_files();
      rescale();
   } 

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

   if ( !scale_q.count( wheel_file ) )
   {
      editor_msg( "red", QString( tr( "Internal error: %1 not found in data" ) ).arg( wheel_file ) );
      return;
   }

   if ( scale_q[ wheel_file ].size() < 2 )
   {
      editor_msg( "red", QString( tr( "Internal error: %1 almost empty data" ) ).arg( wheel_file ) );
      return;
   }

   if ( !scale_I.count( wheel_file ) )
   {
      editor_msg( "red", QString( tr( "Internal error: %1 not found in y data" ) ).arg( wheel_file ) );
      return;
   }

   if ( !scale_I[ wheel_file ].size() )
   {
      editor_msg( "red", QString( tr( "Internal error: %1 empty y data" ) ).arg( wheel_file ) );
      return;
   }

   if ( le_scale_q_start->text().isEmpty() ||
        le_scale_q_start->text() == "0" ||
        le_scale_q_start->text().toDouble() < scale_q[ wheel_file ][ 0 ] ||
        le_scale_q_start->text().toDouble() > scale_q[ wheel_file ].back()
        )
   {
      disconnect( le_scale_q_start, SIGNAL( textChanged( const QString & ) ), 0, 0 );
      le_scale_q_start->setText( QString( "%1" ).arg( scale_q[ wheel_file ][ 0 ] ) );
      connect( le_scale_q_start, SIGNAL( textChanged( const QString & ) ), SLOT( scale_q_start_text( const QString & ) ) );
   }

   if ( le_scale_q_end->text().isEmpty() ||
        le_scale_q_end->text() == "0" ||
        le_scale_q_end->text().toDouble() < scale_q[ wheel_file ][ 0 ] ||
        le_scale_q_end->text().toDouble() > scale_q[ wheel_file ].back() )
   {
      disconnect( le_scale_q_end, SIGNAL( textChanged( const QString & ) ), 0, 0 );
      le_scale_q_end->setText( QString( "%1" ).arg( scale_q[ wheel_file ].back() ) );
      connect( le_scale_q_end, SIGNAL( textChanged( const QString & ) ), SLOT( scale_q_end_text( const QString & ) ) );
   }

   // qDebug( QString( "scale: q: [%1:%2] wheelfile %3 [%4:%5]" )
   //         .arg( le_scale_q_start->text() )
   //         .arg( le_scale_q_end->text() )
   //         .arg( wheel_file )
   //         .arg( scale_q[ wheel_file ][ 0 ] )
   //         .arg( scale_q[ wheel_file ].back() ) 
   //         );

   disable_all();
   mode_select( MODE_SCALE );

   if ( testiq_active )
   {
      pb_testiq->setEnabled( true );
   }
   pb_rescale     ->setEnabled( true );
   pb_axis_x      ->setEnabled( true );
   pb_axis_y      ->setEnabled( true );

   scale_applied = false;

   running       = true;

   scale_enables();

   plotted_markers.clear();
   gauss_add_marker( le_scale_q_start  ->text().toDouble(), Qt::red, tr( "Start") );
   gauss_add_marker( le_scale_q_end    ->text().toDouble(), Qt::red, tr( "End"  ), Qt::AlignLeft | Qt::AlignTop );
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

   msg += QString( "\nScale %1 files from q: %2 %3\n" ).arg( scale_selected.size() ).arg( q_min ).arg( q_max );

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
   le_scale_q_start->setText( QString( "%1" ).arg( scale_q[ wheel_file ][ 0 ]   ) );
   le_scale_q_end  ->setText( QString( "%1" ).arg( scale_q[ wheel_file ].back() ) );
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
   if ( current_mode != MODE_SCALE )
   {
      return;
   }
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
   if ( current_mode != MODE_SCALE )
   {
      return;
   }
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
   if ( current_mode != MODE_SCALE )
   {
      return;
   }
   if ( hasFocus && scale_q.count( wheel_file ) )
   {
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      qwtw_wheel->setRange( scale_q[ wheel_file ][ 0 ], scale_q[ wheel_file ].back(), 
                            ( scale_q[ wheel_file ].back() - scale_q[ wheel_file ][ 0 ] ) / UHSH_WHEEL_RES );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setValue( le_scale_q_start->text().toDouble() );
      qwtw_wheel->setEnabled( true );
   }
}

void US_Hydrodyn_Saxs_Hplc::scale_q_end_focus( bool hasFocus )
{
   if ( current_mode != MODE_SCALE )
   {
      return;
   }
   if ( hasFocus && scale_q.count( wheel_file ) )
   {
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      qwtw_wheel->setRange( scale_q[ wheel_file ][ 0 ], scale_q[ wheel_file ].back(), 
                            ( scale_q[ wheel_file ].back() - scale_q[ wheel_file ][ 0 ] ) / UHSH_WHEEL_RES );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setValue( le_scale_q_end->text().toDouble() );
      qwtw_wheel->setEnabled( true );
   }
}


void US_Hydrodyn_Saxs_Hplc::adjust_wheel( double pos )
{
   // cout << QString("pos is now %1 wheel step is %2\n").arg(pos, 0, 'f', 8 ).arg( qwtw_wheel->step() );
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

         if ( !le_last_focus )
         {
            // cout << "aw: no last focus in guinier mode\n";
            return;
         }

         le_last_focus->setText( QString( "%1" ).arg( pos ) );

         lbl_wheel_pos->setText( QString( "%1" ).arg( pos ) );
      }
      break;

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
         plot_files();
         if ( testiq_active )
         {
            remove_files( testiq_created_scale_names );
            set_selected( testiq_original_selection );
            testiq_created_scale_names.clear();
            testiq_original_selection.clear();
            mode_select( MODE_TESTIQ );
            return wheel_cancel();
         }
         mode_select( MODE_NORMAL );
         rescale();
      }
      break;

   case MODE_TESTIQ :
      {
         testiq_created_names.clear();
         testiq_created_q    .clear();
         testiq_created_I    .clear();
         testiq_created_e    .clear();

         for ( int i = 0; i < (int) rb_testiq_gaussians.size(); ++i )
         {
            hbl_testiq_gaussians->remove( rb_testiq_gaussians[ i ] );
            delete rb_testiq_gaussians[ i ];
         }
         rb_testiq_gaussians.clear();

         gauss_delete_markers();
         plotted_markers.clear();
         mode_select( MODE_NORMAL );
         plot_files();
         rescale();
         testiq_active = false;
      }
      break;

   case MODE_GUINIER :
      {
         guinier_q           .clear();
         guinier_q2          .clear();
         guinier_I           .clear();
         guinier_e           .clear();
         guinier_x           .clear();
         guinier_y           .clear();
         guinier_a           .clear();
         guinier_b           .clear();
         guinier_colors      .clear();

         guinier_delete_markers();
         guinier_markers.clear();
         guinier_curves.clear();
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
         plotted_markers.clear();
         mode_select( MODE_NORMAL );
         plot_files();
         rescale();
      }
      break;

   case MODE_RGC :
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
         // qDebug( "wheel save mode scale not yet" );
         wheel_cancel();
         return;
      }
      break;

   case MODE_TESTIQ :
      {
         // qDebug( "wheel save mode testiq not yet" );
         wheel_cancel();
         return;
      }
      break;

   case MODE_GUINIER :
      {
         // qDebug( "wheel save mode scale not yet" );
         wheel_cancel();
         return;
      }
      break;

   case MODE_PM :
      {
         qDebug( "wheel save mode pm not yet" );
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
