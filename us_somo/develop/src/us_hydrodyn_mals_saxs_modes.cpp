#include "../include/us3_defines.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_hydrodyn_mals_saxs.h"
#include "../include/us_hydrodyn_mals_saxs_scale_trend.h"
#include "../include/us_pm.h"
//Added by qt3to4:
#include <QPixmap>

// --- PM ----

void US_Hydrodyn_Mals_Saxs::pm()
{
   le_last_focus = (mQLineEdit *) 0;

   bool any_selected = false;

   for ( int i = 0; i < lb_files->count(); i++ )
   {
      if ( lb_files->item( i )->isSelected() )
      {
         if ( !any_selected )
         {
            wheel_file = lb_files->item( i )->text();
            any_selected = true;
            break;
         }
      }
   }

   if ( !any_selected )
   {
      editor_msg( "red", us_tr( "Internal error: no files selected in pm mode" ) );
      return;
   }

   if ( !f_qs.count( wheel_file ) )
   {
      editor_msg( "red", QString( us_tr( "Internal error: %1 not found in data" ) ).arg( wheel_file ) );
      return;
   }

   if ( f_qs[ wheel_file ].size() < 2 )
   {
      editor_msg( "red", QString( us_tr( "Internal error: %1 almost empty data" ) ).arg( wheel_file ) );
      return;
   }

   if ( !f_Is.count( wheel_file ) )
   {
      editor_msg( "red", QString( us_tr( "Internal error: %1 not found in y data" ) ).arg( wheel_file ) );
      return;
   }

   if ( !f_Is[ wheel_file ].size() )
   {
      editor_msg( "red", QString( us_tr( "Internal error: %1 empty y data" ) ).arg( wheel_file ) );
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
   pb_rescale_y   ->setEnabled( true );
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

   plotted_markers.clear( );
   gauss_add_marker( le_pm_q_start  ->text().toDouble(), Qt::red, us_tr( "Start" ) );
   gauss_add_marker( le_pm_q_end    ->text().toDouble(), Qt::red, us_tr( "End"   ), Qt::AlignLeft | Qt::AlignTop );
   plot_dist->replot();
}

void US_Hydrodyn_Mals_Saxs::pm_enables()
{
   bool shapes_selected = false;
#if QT_VERSION >= 0x040000
   for ( QList < QAbstractButton* >::iterator it = bg_pm_shape->buttons().begin();
         it != bg_pm_shape->buttons().end();
         ++it ) {
      if ( ((QRadioButton*)*it)->isChecked() ) {
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
   pb_timeshift          ->setEnabled( false );
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

void US_Hydrodyn_Mals_Saxs::pm_run()
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

   us_qdebug( "pm_run 0" );
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
      editor_msg( "red", us_tr( "PM: no points left in curve" ) );
      pm_enables();
      return;
   }

   us_qdebug( "pm_run 1" );
   editor_msg( "blue", QString( us_tr( "PM: actual q points used %1 q range [%2:%3]" ) ).arg( use_q.size() ).arg( use_q[ 0 ] ).arg( use_q.back() ) );

   run_params[ "pmgridsize"           ] = le_pm_grid_size  ->text();
   run_params[ "pmminq"               ] = le_pm_q_start    ->text();
   run_params[ "pmmaxq"               ] = le_pm_q_end      ->text();

   run_params[ "pmbufferedensity"     ] = le_pm_buff_e_dens->text();
   run_params[ "pmrayleighdrho"       ] = le_pm_samp_e_dens->text();
   
   run_params[ "pmapproxmaxdimension" ] = "true";

   run_params[ "pmoutname"            ] = wheel_file + "_pm";
   
   us_qdebug( "pm_run 2" );

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

   us_qdebug( "pm_run 3" );
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
   us_qdebug( "pm_run 4" );
   for ( map < QString, vector < double > >::iterator it = produced_q.begin();
         it != produced_q.end();
         ++it )
   {
      add_plot( it->first, it->second, produced_I[ it->first ], false, false );
      lb_model_files->addItem( last_created_file );
      models[ last_created_file ] = produced_model[ it->first ];
      models_not_saved.insert( last_created_file );
   }
   us_qdebug( "pm_run 5" );
   pm_enables();
   // add to produced
}

void US_Hydrodyn_Mals_Saxs::pm_q_reset()
{
   le_pm_q_start->setText( QString( "%1" ).arg( f_qs[ wheel_file ][ 0 ]   ) );
   le_pm_q_end  ->setText( QString( "%1" ).arg( f_qs[ wheel_file ].back() ) );
}

void US_Hydrodyn_Mals_Saxs::pm_grid_size_text( const QString & )
{
}

void US_Hydrodyn_Mals_Saxs::pm_buff_e_dens_text( const QString & )
{
}

void US_Hydrodyn_Mals_Saxs::pm_samp_e_dens_text( const QString & )
{
}

void US_Hydrodyn_Mals_Saxs::pm_q_pts_text( const QString & )
{
}

void US_Hydrodyn_Mals_Saxs::pm_q_start_text( const QString & text )
{
#if QT_VERSION < 0x040000
   plot_dist->setMarkerPos( plotted_markers[ 0 ], text.toDouble(), 0e0 );
#else
   plotted_markers[ 0 ]->setXValue( text.toDouble() );
#endif
   if ( !wheel_is_pressed && qwtw_wheel->value() != text.toDouble() )
   {
      qwtw_wheel->setValue( text.toDouble() );
   }
   plot_dist->replot();
   pm_enables();
}

void US_Hydrodyn_Mals_Saxs::pm_q_end_text( const QString & text )
{
#if QT_VERSION < 0x040000
   plot_dist->setMarkerPos( plotted_markers[ 1 ], text.toDouble(), 0e0 );
#else
   plotted_markers[ 1 ]->setXValue( text.toDouble() );
#endif
   if ( !wheel_is_pressed && qwtw_wheel->value() != text.toDouble() )
   {
      qwtw_wheel->setValue( text.toDouble() );
   }
   plot_dist->replot();
   pm_enables();
}

void US_Hydrodyn_Mals_Saxs::pm_q_start_focus( bool hasFocus )
{
   if ( hasFocus )
   {
      le_last_focus = le_pm_q_start;
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      qwtw_wheel->setRange( f_qs[ wheel_file ][ 0 ], f_qs[ wheel_file ].back()); qwtw_wheel->setSingleStep( ( f_qs[ wheel_file ].back() - f_qs[ wheel_file ][ 0 ] ) / UHSH_WHEEL_RES );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setValue( le_pm_q_start->text().toDouble() );
      wheel_enables();
   }
}

void US_Hydrodyn_Mals_Saxs::pm_q_end_focus( bool hasFocus )
{
   if ( hasFocus )
   {
      le_last_focus = le_pm_q_end;
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      qwtw_wheel->setRange( f_qs[ wheel_file ][ 0 ], f_qs[ wheel_file ].back()); qwtw_wheel->setSingleStep( ( f_qs[ wheel_file ].back() - f_qs[ wheel_file ][ 0 ] ) / UHSH_WHEEL_RES );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setValue( le_pm_q_end->text().toDouble() );
      wheel_enables();
   }
}

// --- RGC ---

void US_Hydrodyn_Mals_Saxs::rgc()
{

   disable_all();
   mode_select( MODE_RGC );
   running       = true;
   rgc_enables();
}

void US_Hydrodyn_Mals_Saxs::rgc_enables()
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

void US_Hydrodyn_Mals_Saxs::rgc_shape()
{
   if ( rb_rgc_shape_ellipsoid->isChecked() )
   {
      lbl_rgc_axis->setText( us_tr( " Relative axes 1 > b > c:" ) );
   } else {
      if ( rb_rgc_shape_oblate->isChecked() ||
           rb_rgc_shape_prolate->isChecked() )
      {
         lbl_rgc_axis->setText( us_tr( " Relative axis 1 > b:" ) );
      } else {
         lbl_rgc_axis->setText( "" );
      }
   }
   rgc_enables();
   rgc_calc_rg();
}

void US_Hydrodyn_Mals_Saxs::rgc_calc_rg()
{
   
   double V = 0e0;
   double rho = le_rgc_rho->text().toDouble();
   if ( rho )
   {
      V = ( 1e3 * le_rgc_mw->text().toDouble() / AVOGADRO ) / rho;
   }
   // us_qdebug( QString( "volume %1" ).arg( V ) );

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

   // us_qdebug( QString( "extent_a %1" ).arg( extent_a ) );
           
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

   le_rgc_g_qrange->setText( QString( "%1 : %2" )
                             .arg( 1.3e0 / ( 2e0 * Rg * 1e8 ), 0, 'g', 3 )
                             .arg( 1.3e0 / ( Rg * 1e8 ), 0, 'g', 3 ) );
}

void US_Hydrodyn_Mals_Saxs::rgc_mw_text( const QString & )
{
   rgc_calc_rg();
}

void US_Hydrodyn_Mals_Saxs::rgc_vol_text( const QString & )
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

void US_Hydrodyn_Mals_Saxs::rgc_rho_text( const QString & )
{
   rgc_calc_rg();
}

void US_Hydrodyn_Mals_Saxs::rgc_axis_b_text( const QString &, bool do_recompute )
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

void US_Hydrodyn_Mals_Saxs::rgc_axis_c_text( const QString &, bool do_recompute )
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

void US_Hydrodyn_Mals_Saxs::rgc_rg_text( const QString & )
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

void US_Hydrodyn_Mals_Saxs::testiq()
{
   le_last_focus = (mQLineEdit *) 0;

   bool any_selected = false;

   testiq_selected    .clear( );

   bool do_rescale = false;

   plot_errors->detachItems( QwtPlotItem::Rtti_PlotCurve ); plot_errors->detachItems( QwtPlotItem::Rtti_PlotMarker );;
   plot_errors->replot();
   hide_widgets( plot_errors_widgets, true );

   if ( current_mode == MODE_SCALE )
   {
      remove_files( testiq_created_scale_names );
      set_selected( testiq_original_selection );
      testiq_created_scale_names.clear( );
      testiq_original_selection.clear( );
      axis_x( true, true );
      plot_files();
      do_rescale = true;
   }

   for ( int i = 0; i < lb_files->count(); i++ )
   {
      if ( lb_files->item( i )->isSelected() )
      {
         if ( !any_selected )
         {
            wheel_file = lb_files->item( i )->text();
         }
         testiq_selected.insert( lb_files->item( i )->text() );
         any_selected = true;
         if ( !plotted_curves.count( lb_files->item( i )->text() ) )
         {
            editor_msg( "red", QString( us_tr( "Internal error: testiq selected %1, but no plotted curve found" ) ).arg( lb_files->item( i )->text() ) );
            return;
         }
      }
   }

   if ( !any_selected )
   {
      editor_msg( "red", us_tr( "Internal error: no files selected in testiq mode" ) );
      return;
   }

   if ( !f_qs.count( wheel_file ) )
   {
      editor_msg( "red", QString( us_tr( "Internal error: %1 not found in data" ) ).arg( wheel_file ) );
      return;
   }

   if ( f_qs[ wheel_file ].size() < 2 )
   {
      editor_msg( "red", QString( us_tr( "Internal error: %1 almost empty data" ) ).arg( wheel_file ) );
      return;
   }

   if ( !f_Is.count( wheel_file ) )
   {
      editor_msg( "red", QString( us_tr( "Internal error: %1 not found in y data" ) ).arg( wheel_file ) );
      return;
   }

   if ( !f_Is[ wheel_file ].size() )
   {
      editor_msg( "red", QString( us_tr( "Internal error: %1 empty y data" ) ).arg( wheel_file ) );
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
   // plotted_markers.clear( );
   if ( current_mode == MODE_GUINIER ||
        current_mode == MODE_SCALE )
   {
      gauss_delete_markers();
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
         // us_qdebug( QString( "testiq thresh %1 max_i %2 avg_i %3" ).arg( thresh ).arg( max_i ).arg( avg_i ) );

         tsdl.sort();
         tsdl.reverse();
         
         for ( list < testiq_sortable_double >::iterator it = tsdl.begin();
               it != tsdl.end();
               ++it )
         {
            // us_qdebug( QString( "it loop x %1 index %2 Imin %3 Imax %4" ).arg( it->x ).arg( it->index ).arg( it->Imin ).arg( it->Imax ) );
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

   // us_qdebug( QString( "testitsel %1 min %2 max %3" ).arg( testiq_it_selected ).arg( testiq_it_selected_Imin ).arg( testiq_it_selected_Imax ) );

   mode_select( MODE_TESTIQ );

   gauss_add_marker( le_testiq_q_start  ->text().toDouble(), Qt::red, us_tr( "Start") );
   gauss_add_marker( le_testiq_q_end    ->text().toDouble(), Qt::red, us_tr( "End"  ), Qt::AlignLeft | Qt::AlignTop );

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
#if QT_VERSION < 0x040000
            bg_testiq_gaussians ->insert( rb );
#else
            bg_testiq_gaussians ->addButton( rb );
#endif
            connect( rb, SIGNAL( clicked() ), SLOT( testiq_gauss_line() ) );
         }
         rb_testiq_from_i_t->setChecked( true );
      }
      
      for ( int i = 0; i < (int) unified_ggaussian_gaussians_size; i++ )
      {
         gauss_add_marker( unified_ggaussian_params[ (vector<double>::size_type) common_size * i ], Qt::blue, QString( "%1" ).arg( i + 1 ) );
      }
      testiq_gauss_line();
   }         

   running       = true;
   testiq_active = true;

   testiq_enables();

   // us_qdebug( suppress_replot ? "suppress_replot on" : "suppress_replot off" );
   if ( do_rescale )
   {
      // us_qdebug( "testiq mode scale rescale" );
      rescale();
   } else {
      plot_dist->replot();
   }
}

void US_Hydrodyn_Mals_Saxs::testiq_gauss_line()
{
   int line_width = use_line_width < 3 ? ( use_line_width + 1 ) : use_line_width;

   for ( int i = 0; i < (int) rb_testiq_gaussians.size(); ++i )
   {
      QPen use_pen = QPen( rb_testiq_gaussians[ i ]->isChecked() ? Qt::magenta : Qt::blue, line_width, Qt::DashDotDotLine );
         
#if QT_VERSION >= 0x040000
      plotted_markers[ 2 + i ]->setLinePen( use_pen );
#else
      plot_dist->setMarkerPen( plotted_markers[ 2 + i ], use_pen );
#endif
   }
   plot_dist->replot();
   testiq_enables();
}      

void US_Hydrodyn_Mals_Saxs::testiq_enables()
{
   pb_timeshift           ->setEnabled( false );
   pb_wheel_cancel        ->setEnabled( true );
   le_testiq_q_start      ->setEnabled( true );
   le_testiq_q_end        ->setEnabled( true );
   pb_testiq_visrange     ->setEnabled( true );
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
   pb_rescale     ->setEnabled( true );
   pb_rescale_y   ->setEnabled( true );
   pb_axis_x      ->setEnabled( true );
   pb_axis_y      ->setEnabled( true );
}

bool US_Hydrodyn_Mals_Saxs::testiq_make()
{
   // todo: make the iq to vectors <> 
   // used by guinier_testiq() and testiq_testset
   testiq_created_names.clear( );
   testiq_created_t    .clear( );
   testiq_created_q    .clear( );
   testiq_created_I    .clear( );
   testiq_created_e    .clear( );

   if ( cb_testiq_from_gaussian->isVisible() && !rb_testiq_from_i_t->isChecked() )
   {
      return create_i_of_q( testiq_selected, le_testiq_q_start->text().toDouble(), le_testiq_q_end->text().toDouble() );
   } else {
      return create_i_of_q_ng( testiq_selected, le_testiq_q_start->text().toDouble(), le_testiq_q_end->text().toDouble() );
   }
}

void US_Hydrodyn_Mals_Saxs::testiq_visrange()
{
   disconnect( le_testiq_q_start, SIGNAL( textChanged( const QString & ) ), 0, 0 );
   disconnect( le_testiq_q_end, SIGNAL( textChanged( const QString & ) ), 0, 0 );
   if ( plot_dist_zoomer )
   {
#if QT_VERSION >= 0x040000
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
#if QT_VERSION < 0x040000
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

void US_Hydrodyn_Mals_Saxs::testiq_testset()
{
   if ( !testiq_make() )
   {
      testiq_enables();
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

void US_Hydrodyn_Mals_Saxs::testiq_q_start_text( const QString & text )
{
   if ( current_mode != MODE_TESTIQ )
   {
      return;
   }
#if QT_VERSION < 0x040000
   plot_dist->setMarkerPos( plotted_markers[ 0 ], text.toDouble(), 0e0 );
#else
   plotted_markers[ 0 ]->setXValue( text.toDouble() );
#endif
   if ( !wheel_is_pressed && qwtw_wheel->value() != text.toDouble() )
   {
      qwtw_wheel->setValue( text.toDouble() );
   }
   plot_dist->replot();
   testiq_enables();
}

void US_Hydrodyn_Mals_Saxs::testiq_q_end_text( const QString & text )
{
   if ( current_mode != MODE_TESTIQ )
   {
      return;
   }
#if QT_VERSION < 0x040000
   plot_dist->setMarkerPos( plotted_markers[ 1 ], text.toDouble(), 0e0 );
#else
   plotted_markers[ 1 ]->setXValue( text.toDouble() );
#endif
   if ( !wheel_is_pressed && qwtw_wheel->value() != text.toDouble() )
   {
      qwtw_wheel->setValue( text.toDouble() );
   }
   plot_dist->replot();
   testiq_enables();
}

void US_Hydrodyn_Mals_Saxs::testiq_q_start_focus( bool hasFocus )
{
   if ( current_mode != MODE_TESTIQ )
   {
      return;
   }
   if ( hasFocus )
   {
      le_last_focus = le_testiq_q_start;
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      qwtw_wheel->setRange( f_qs[ wheel_file ][ 0 ], f_qs[ wheel_file ].back()); qwtw_wheel->setSingleStep( ( f_qs[ wheel_file ].back() - f_qs[ wheel_file ][ 0 ] ) / UHSH_WHEEL_RES );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setValue( le_testiq_q_start->text().toDouble() );
      wheel_enables();
   }
}

void US_Hydrodyn_Mals_Saxs::testiq_q_end_focus( bool hasFocus )
{
   if ( current_mode != MODE_TESTIQ )
   {
      return;
   }
   if ( hasFocus )
   {
      le_last_focus = le_testiq_q_end;
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      qwtw_wheel->setRange( f_qs[ wheel_file ][ 0 ], f_qs[ wheel_file ].back()); qwtw_wheel->setSingleStep( ( f_qs[ wheel_file ].back() - f_qs[ wheel_file ][ 0 ] ) / UHSH_WHEEL_RES );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setValue( le_testiq_q_end->text().toDouble() );
      wheel_enables();
   }
}

// --- wyatt ---
void US_Hydrodyn_Mals_Saxs::wyatt_start()
{
   if ( axis_y_log )
   {
      axis_y();
   }

   org_wyatt_start       = le_wyatt_start      ->text().toDouble();
   org_wyatt_end         = le_wyatt_end        ->text().toDouble();
   org_wyatt_start2      = le_wyatt_start2     ->text().toDouble();
   org_wyatt_end2        = le_wyatt_end2       ->text().toDouble();
   org_wyatt_2           = cb_wyatt_2          ->isChecked();

   le_last_focus = (mQLineEdit *) 0;
   for ( int i = 0; i < lb_files->count(); i++ )
   {
      if ( lb_files->item( i )->isSelected() )
      {
         wheel_file = lb_files->item( i )->text();
         break;
      }
   }
   if ( !f_qs.count( wheel_file ) )
   {
      editor_msg( "red", QString( us_tr( "Internal error: %1 not found in data" ) ).arg( wheel_file ) );
      return;
   }

   if ( f_qs[ wheel_file ].size() < 2 )
   {
      editor_msg( "red", QString( us_tr( "Internal error: %1 almost empty data" ) ).arg( wheel_file ) );
      return;
   }

   if ( !f_Is.count( wheel_file ) )
   {
      editor_msg( "red", QString( us_tr( "Internal error: %1 not found in y data" ) ).arg( wheel_file ) );
      return;
   }

   if ( !f_Is[ wheel_file ].size() )
   {
      editor_msg( "red", QString( us_tr( "Internal error: %1 empty y data" ) ).arg( wheel_file ) );
      return;
   }

#if QT_VERSION < 0x040000
   plot_dist->setCurvePen( plotted_curves[ wheel_file ], QPen( Qt::cyan, use_line_width, SolidLine));
#else
   plotted_curves[ wheel_file ]->setPen( QPen( Qt::cyan, use_line_width, Qt::SolidLine ) );
#endif

   {
      QwtSymbol symbol;
      symbol.setStyle( QwtSymbol::Diamond );
      symbol.setSize( 1 + use_line_width * 2 );
      symbol.setPen  ( QPen( Qt::cyan, use_line_width, Qt::SolidLine ) );
      symbol.setBrush( Qt::cyan );

#if QT_VERSION < 0x040000
      plot_dist->setCurveStyle ( plotted_curves[ wheel_file ], QwtCurve::NoCurve );
      plot_dist->setCurveSymbol( plotted_curves[ wheel_file ], symbol );
#else
      plotted_curves[ wheel_file ]->setStyle( QwtPlotCurve::NoCurve );
# if QT_VERSION < 0x050000
      plotted_curves[ wheel_file ]->setSymbol( new QwtSymbol( symbol.style(), symbol.brush(), symbol.pen(), symbol.size() ) );
# else
      plotted_curves[ wheel_file ]->setSymbol( new QwtSymbol( symbol.style(), symbol.brush(), symbol.pen(), symbol.size() ) );
# endif
#endif
   }

   mode_select( MODE_WYATT );
   running       = true;
   qwtw_wheel->setRange( f_qs[ wheel_file ][ 0 ], f_qs[ wheel_file ].back()); qwtw_wheel->setSingleStep( ( f_qs[ wheel_file ].back() - f_qs[ wheel_file ][ 0 ] ) / UHSH_WHEEL_RES );

   double q_len       = f_qs[ wheel_file ].back() - f_qs[ wheel_file ][ 0 ];
   double q_len_delta = q_len * 0.1;

   if ( le_wyatt_start->text().isEmpty() ||
        le_wyatt_start->text().toDouble() < f_qs[ wheel_file ][ 0 ] )
   {
      disconnect( le_wyatt_start, SIGNAL( textChanged( const QString & ) ), 0, 0 );
      le_wyatt_start->setText( QString( "%1" ).arg( f_qs[ wheel_file ][ 0 ]) );
      connect( le_wyatt_start, SIGNAL( textChanged( const QString & ) ), SLOT( wyatt_start_text( const QString & ) ) );
   }

   if ( le_wyatt_end->text().isEmpty() ||
        le_wyatt_end->text() == "0" ||
        le_wyatt_end->text().toDouble() > f_qs[ wheel_file ].back() )
   {
      disconnect( le_wyatt_end, SIGNAL( textChanged( const QString & ) ), 0, 0 );
      le_wyatt_end->setText( QString( "%1" ).arg( f_qs[ wheel_file ][ 0 ] + q_len_delta ) );
      connect( le_wyatt_end, SIGNAL( textChanged( const QString & ) ), SLOT( wyatt_end_text( const QString & ) ) );
   }

   if ( le_wyatt_start2->text().isEmpty() ||
        le_wyatt_start2->text() == "0" ||
        le_wyatt_start2->text().toDouble() < f_qs[ wheel_file ][ 0 ] )
   {
      disconnect( le_wyatt_start2, SIGNAL( textChanged( const QString & ) ), 0, 0 );
      le_wyatt_start2->setText( QString( "%1" ).arg( f_qs[ wheel_file ].back() - q_len_delta ) );
      connect( le_wyatt_start2, SIGNAL( textChanged( const QString & ) ), SLOT( wyatt_start2_text( const QString & ) ) );
   }

   if ( le_wyatt_end2->text().isEmpty() ||
        le_wyatt_end2->text() == "0" ||
        le_wyatt_end2->text().toDouble() > f_qs[ wheel_file ].back() )
   {
      disconnect( le_wyatt_end2, SIGNAL( textChanged( const QString & ) ), 0, 0 );
      le_wyatt_end2->setText( QString( "%1" ).arg( f_qs[ wheel_file ].back() ) );
      connect( le_wyatt_end2, SIGNAL( textChanged( const QString & ) ), SLOT( wyatt_end2_text( const QString & ) ) );
   }

   wyatt_init_markers();
   replot_wyatt();
   disable_all();
   wyatt_enables();
}

void US_Hydrodyn_Mals_Saxs::wyatt_2()
{
   wyatt_init_markers();
   replot_wyatt();
   wyatt_enables();
}

void US_Hydrodyn_Mals_Saxs::wyatt_enables()
{
   pb_wyatt_start         ->setEnabled( false );
   pb_wheel_cancel        ->setEnabled( true );
   pb_wheel_save          ->setEnabled( 
                                       le_wyatt_start  ->text().toDouble() != org_wyatt_start   ||
                                       le_wyatt_end    ->text().toDouble() != org_wyatt_end     ||
                                       cb_wyatt_2      ->isChecked()       != org_wyatt_2       ||
                                       ( cb_wyatt_2->isChecked() && (
                                                                     le_wyatt_start2 ->text().toDouble() != org_wyatt_start   ||
                                                                     le_wyatt_end2   ->text().toDouble() != org_wyatt_end ) )
                                        );

   le_wyatt_start         ->setEnabled( true );
   le_wyatt_end           ->setEnabled( true );
   cb_wyatt_2             ->setEnabled( true );
   le_wyatt_start2        ->setEnabled( cb_wyatt_2->isChecked() );
   le_wyatt_end2          ->setEnabled( cb_wyatt_2->isChecked() );
   wheel_enables          (
#if QT_VERSION > 0x050000
                           le_last_focus == le_wyatt_start   || 
                           le_last_focus == le_wyatt_end     ||
                           le_last_focus == le_wyatt_start2  || 
                           le_last_focus == le_wyatt_end2   
#else
                           le_wyatt_start  ->hasFocus() || 
                           le_wyatt_end    ->hasFocus() ||
                           le_wyatt_start2 ->hasFocus() || 
                           le_wyatt_end2   ->hasFocus()
#endif
                           );
   pb_rescale             ->setEnabled( true );
   pb_rescale_y           ->setEnabled( true );
   pb_view                ->setEnabled( true );
   if ( cb_wyatt_2->isChecked() )
   {
      le_wyatt_start2->show();
      le_wyatt_end2  ->show();
   } else {
      le_wyatt_start2->hide();
      le_wyatt_end2  ->hide();
   }
}

void US_Hydrodyn_Mals_Saxs::wyatt_init_markers()
{
   gauss_delete_markers();

   plotted_wyatt.clear( );

   gauss_add_marker( le_wyatt_start  ->text().toDouble(), Qt::red,     us_tr( "Start"          ) );
   gauss_add_marker( le_wyatt_end    ->text().toDouble(), Qt::red,     us_tr( "End"            ), Qt::AlignLeft | Qt::AlignTop );

   if ( cb_wyatt_2->isChecked() )
   {
      gauss_add_marker( le_wyatt_start2  ->text().toDouble(), Qt::magenta,    us_tr( "Start"          ) );
      gauss_add_marker( le_wyatt_end2    ->text().toDouble(), Qt::magenta,    us_tr( "End"            ), Qt::AlignLeft | Qt::AlignTop );
   }

   if ( !suppress_replot )
   {
      plot_dist->replot();
   }
}

void US_Hydrodyn_Mals_Saxs::wyatt_apply()
{
   wyatt_apply( all_selected_files() );
}

void US_Hydrodyn_Mals_Saxs::wyatt_apply( const QStringList & files )
{
   for ( int i = 0; i < (int) files.size(); ++i )
   {

      if ( cb_wyatt_2->isChecked() )
      {
         vector < double > wyatt_q;
         vector < double > wyatt_q2;
         vector < double > wyatt_I;
         vector < double > wyatt_y;

         double wyatt_error = 
            wyatt_errors( files[ i ],
                          le_wyatt_start->text().toDouble(),
                          le_wyatt_end->text().toDouble(),
                          wyatt_q,
                          wyatt_I,
                          wyatt_y );
         
         double wyatt_error2 = 
            wyatt_errors( files[ i ],
                          le_wyatt_start2->text().toDouble(),
                          le_wyatt_end2->text().toDouble(),
                          wyatt_q2,
                          wyatt_I,
                          wyatt_y );

         if ( wyatt_error < 0e0 && wyatt_error2 < 0e0 )
         {
            editor_msg( "red", QString( us_tr( "SD errors failed for %1" ) ).arg( files[ i ] ) );
         } else {
            double use_e;
            if ( wyatt_error < 0e0 || wyatt_error2 < 0e0 )
            {
               editor_msg( "dark red", QString( us_tr( "SD errors failed range %1:%2 for %3" ) )
                           .arg( wyatt_error < 0e0 ? le_wyatt_start->text().toDouble() : le_wyatt_start2->text().toDouble())
                           .arg( wyatt_error < 0e0 ? le_wyatt_end->text().toDouble() : le_wyatt_end2->text().toDouble())
                           .arg( files[ i ] ) );
               use_e = wyatt_error < 0e0 ? wyatt_error2 : wyatt_error;
            } else {
               double frac1 = (double) wyatt_q.size() / ( (double) wyatt_q.size() +  (double) wyatt_q2.size() );
               double frac2 = 1e0 - frac1;

               use_e = sqrt( frac1 * wyatt_error * wyatt_error + frac2 * wyatt_error2 * wyatt_error2 );
            }
            if ( !constant_e( files[ i ], use_e ) )
            {
               editor_msg( "red", QString( us_tr( "setting SD errors failed for %1" ) ).arg( files[ i ] ) );
            }
         }
      } else {
         double wyatt_error = 
            wyatt_errors( files[ i ],
                          le_wyatt_start->text().toDouble(),
                          le_wyatt_end->text().toDouble() );


         if ( wyatt_error < 0e0 ||
              !constant_e( files[ i ], wyatt_error ) )
         {
            editor_msg( "red", QString( us_tr( "SD errors failed for %1" ) ).arg( files[ i ] ) );
         }
      }
   }
   update_enables();
}

void US_Hydrodyn_Mals_Saxs::wyatt_start_text( const QString & text )
{
   int pos = 0;
#if QT_VERSION < 0x040000
   plot_dist->setMarkerPos( plotted_markers[ pos ], text.toDouble(), 0e0 );
#else
   plotted_markers[ pos ]->setXValue( text.toDouble() );
#endif
   if ( !wheel_is_pressed && qwtw_wheel->value() != text.toDouble() )
   {
      qwtw_wheel->setValue( text.toDouble() );
   }
   replot_wyatt();
   if ( !suppress_replot )
   {
      plot_dist->replot();
   }
   wyatt_enables();
}

void US_Hydrodyn_Mals_Saxs::wyatt_end_text( const QString & text )
{
   int pos = 1;
#if QT_VERSION < 0x040000
   plot_dist->setMarkerPos( plotted_markers[ pos ], text.toDouble(), 0e0 );
#else
   plotted_markers[ pos ]->setXValue( text.toDouble() );
#endif
   if ( !wheel_is_pressed && qwtw_wheel->value() != text.toDouble() )
   {
      qwtw_wheel->setValue( text.toDouble() );
   }
   replot_wyatt();
   if ( !suppress_replot )
   {
      plot_dist->replot();
   }
   wyatt_enables();
}

void US_Hydrodyn_Mals_Saxs::wyatt_start_focus( bool hasFocus )
{
   // cout << QString( "wyatt_start_focus %1\n" ).arg( hasFocus ? "true" : "false" );
   if ( hasFocus )
   {
      le_last_focus = le_wyatt_start;
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      qwtw_wheel->setRange( f_qs[ wheel_file ][ 0 ], f_qs[ wheel_file ].back()); qwtw_wheel->setSingleStep( ( f_qs[ wheel_file ].back() - f_qs[ wheel_file ][ 0 ] ) / UHSH_WHEEL_RES );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setValue( le_wyatt_start->text().toDouble() );
      wheel_enables();
   }
}

void US_Hydrodyn_Mals_Saxs::wyatt_end_focus( bool hasFocus )
{
   // cout << QString( "wyatt_end_focus %1\n" ).arg( hasFocus ? "true" : "false" );
   if ( hasFocus )
   {
      le_last_focus = le_wyatt_end;
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      qwtw_wheel->setRange( f_qs[ wheel_file ][ 0 ], f_qs[ wheel_file ].back()); qwtw_wheel->setSingleStep( ( f_qs[ wheel_file ].back() - f_qs[ wheel_file ][ 0 ] ) / UHSH_WHEEL_RES );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setValue( le_wyatt_end->text().toDouble() );
      wheel_enables();
   }
}

void US_Hydrodyn_Mals_Saxs::wyatt_start2_text( const QString & text )
{
   int pos = 2;
#if QT_VERSION < 0x040000
   plot_dist->setMarkerPos( plotted_markers[ pos ], text.toDouble(), 0e0 );
#else
   plotted_markers[ pos ]->setXValue( text.toDouble() );
#endif
   if ( !wheel_is_pressed && qwtw_wheel->value() != text.toDouble() )
   {
      qwtw_wheel->setValue( text.toDouble() );
   }
   replot_wyatt();
   if ( !suppress_replot )
   {
      plot_dist->replot();
   }
   wyatt_enables();
}

void US_Hydrodyn_Mals_Saxs::wyatt_end2_text( const QString & text )
{
   int pos = 3;
#if QT_VERSION < 0x040000
   plot_dist->setMarkerPos( plotted_markers[ pos ], text.toDouble(), 0e0 );
#else
   plotted_markers[ pos ]->setXValue( text.toDouble() );
#endif
   if ( !wheel_is_pressed && qwtw_wheel->value() != text.toDouble() )
   {
      qwtw_wheel->setValue( text.toDouble() );
   }
   replot_wyatt();
   if ( !suppress_replot )
   {
      plot_dist->replot();
   }
   wyatt_enables();
}

void US_Hydrodyn_Mals_Saxs::wyatt_start2_focus( bool hasFocus )
{
   // us_qdebug( QString( "wyatt_start2_focus %1\n" ).arg( hasFocus ? "true" : "false" ) );
   if ( hasFocus )
   {
      le_last_focus = le_wyatt_start2;
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      qwtw_wheel->setRange( f_qs[ wheel_file ][ 0 ], f_qs[ wheel_file ].back()); qwtw_wheel->setSingleStep( ( f_qs[ wheel_file ].back() - f_qs[ wheel_file ][ 0 ] ) / UHSH_WHEEL_RES );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setValue( le_wyatt_start2->text().toDouble() );
      wheel_enables();
   }
}

void US_Hydrodyn_Mals_Saxs::wyatt_end2_focus( bool hasFocus )
{
   // us_qdebug(  QString( "wyatt_end2_focus %1\n" ).arg( hasFocus ? "true" : "false" ) );
   if ( hasFocus )
   {
      le_last_focus = le_wyatt_end2;
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      qwtw_wheel->setRange( f_qs[ wheel_file ][ 0 ], f_qs[ wheel_file ].back()); qwtw_wheel->setSingleStep( ( f_qs[ wheel_file ].back() - f_qs[ wheel_file ][ 0 ] ) / UHSH_WHEEL_RES );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setValue( le_wyatt_end2->text().toDouble() );
      wheel_enables();
   }
}

void US_Hydrodyn_Mals_Saxs::replot_wyatt()
{
   // us_qdebug( "replot_wyatt()" );

   // remove any wyatt curves

   for ( unsigned int i = 0; i < ( unsigned int ) plotted_wyatt.size(); i++ )
   {
#if QT_VERSION < 0x040000
      plot_dist->removeCurve( plotted_wyatt[ i ] );
#else
      plotted_wyatt[ i ]->detach();
#endif
   }

   vector < double > wyatt_q;
   vector < double > wyatt_I;
   vector < double > wyatt_y;

   double wyatt_error =  wyatt_errors( wheel_file,
                                       le_wyatt_start->text().toDouble(),
                                       le_wyatt_end->text().toDouble(),
                                       wyatt_q,
                                       wyatt_I,
                                       wyatt_y
                                       );

   // the wyatt curve

   if ( wyatt_error >= 0e0 ) 
   {
#if QT_VERSION < 0x040000
      long curve;
      curve = plot_dist->insertCurve( "wyatt" );
      plot_dist->setCurveStyle( curve, QwtCurve::Lines );
#else
      QwtPlotCurve *curve = new QwtPlotCurve( "wyatt" );
      curve->setStyle( QwtPlotCurve::Lines );
#endif

      plotted_wyatt.push_back( curve );

#if QT_VERSION < 0x040000
      plot_dist->setCurvePen( curve, QPen( Qt::green , use_line_width, Qt::SolidLine ) );
      plot_dist->setCurveData( curve,
                               (double *)&wyatt_q[ 0 ],
                               (double *)&wyatt_y[ 0 ],
                               wyatt_q.size()
                               );
#else
      curve->setPen( QPen( Qt::green, use_line_width, Qt::SolidLine ) );
      curve->setSamples(
                     (double *)&wyatt_q[ 0 ],
                     (double *)&wyatt_y[ 0 ],
                     wyatt_q.size()
                     );
      curve->attach( plot_dist );
#endif
   }

   QString msg = 
      QString( us_tr( "SD t range %1:%2 SD %3" ) )
      .arg( le_wyatt_start->text().toDouble() )
      .arg( le_wyatt_end->text().toDouble() )
      .arg( wyatt_error );

   if ( cb_wyatt_2->isChecked() )
   {
      
      double wyatt_error2 = -1e0;

      vector < double > wyatt_q2;
      vector < double > wyatt_I2;
      vector < double > wyatt_y2;

      wyatt_error2 = wyatt_errors( wheel_file,
                                   le_wyatt_start2->text().toDouble(),
                                   le_wyatt_end2->text().toDouble(),
                                   wyatt_q2,
                                   wyatt_I2,
                                   wyatt_y2
                                   );

      if ( wyatt_error2 >= 0e0 ) 
      {
#if QT_VERSION < 0x040000
         long curve;
         curve = plot_dist->insertCurve( "wyatt2" );
         plot_dist->setCurveStyle( curve, QwtCurve::Lines );
#else
         QwtPlotCurve *curve = new QwtPlotCurve( "wyatt2" );
         curve->setStyle( QwtPlotCurve::Lines );
#endif

         plotted_wyatt.push_back( curve );

#if QT_VERSION < 0x040000
         plot_dist->setCurvePen( curve, QPen( Qt::yellow , use_line_width, Qt::SolidLine ) );
         plot_dist->setCurveData( curve,
                                  (double *)&wyatt_q2[ 0 ],
                                  (double *)&wyatt_y2[ 0 ],
                                  wyatt_q2.size()
                                  );
#else
         curve->setPen( QPen( Qt::yellow, use_line_width, Qt::SolidLine ) );
         curve->setSamples(
                        (double *)&wyatt_q2[ 0 ],
                        (double *)&wyatt_y2[ 0 ],
                        wyatt_q2.size()
                        );
         curve->attach( plot_dist );
#endif
      }
      double use_e;

      if ( wyatt_error < 0e0 && wyatt_error2 < 0e0 )
      {
         editor_msg( "red", QString( us_tr( "SD errors failed" ) ) );
      } else {
         if ( wyatt_error < 0e0 || wyatt_error2 < 0e0 )
         {
            use_e = wyatt_error < 0e0 ? wyatt_error2 : wyatt_error;
         } else {
            double frac1 = (double) wyatt_q.size() / ( (double) wyatt_q.size() +  (double) wyatt_q2.size() );
            double frac2 = 1e0 - frac1;

            use_e = sqrt( frac1 * wyatt_error * wyatt_error + frac2 * wyatt_error2 * wyatt_error2 );
         }

         msg += 
            QString( us_tr( " t range2 %1:%2 SD %3  overall SD %4" ) )
            .arg( le_wyatt_start2->text().toDouble() )
            .arg( le_wyatt_end2->text().toDouble() )
                         .arg( wyatt_error2 )
                         .arg( use_e );
      }
      editor_msg( wyatt_error < 0e0 || wyatt_error2 < 0e0 ? "red" : "dark blue", msg );
   } else {
      editor_msg( wyatt_error < 0e0 ? "red" : "dark blue", msg );
   }

   if ( !suppress_replot )
   {
      plot_dist->replot();
   }
}

// --- SCALE ----

void US_Hydrodyn_Mals_Saxs::scale()
{
   le_last_focus = (mQLineEdit *) 0;

   bool any_selected = false;

   scale_selected      .clear( );
   scale_selected_names.clear( );
   scale_q             .clear( );
   scale_I             .clear( );
   scale_e             .clear( );
   scale_last_created  .clear( );
   scale_spline_x      .clear( );
   scale_spline_y      .clear( );
   scale_spline_y2     .clear( );

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
      testiq_created_scale_names.clear( );
      gauss_delete_markers();
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
      axis_x( true, true );
      axis_y( true, true );
      plot_files();
      rescale();
   } 

   for ( int i = 0; i < lb_files->count(); i++ )
   {
      if ( lb_files->item( i )->isSelected() )
      {
         if ( !any_selected )
         {
            wheel_file = lb_files->item( i )->text();
         }
         scale_selected.insert( lb_files->item( i )->text() );
         scale_selected_names.push_back( lb_files->item( i )->text() );
         any_selected = true;
         scale_q[ lb_files->item( i )->text() ] = f_qs[ lb_files->item( i )->text() ];
         scale_I[ lb_files->item( i )->text() ] = f_Is[ lb_files->item( i )->text() ];
         scale_e[ lb_files->item( i )->text() ] = f_errors[ lb_files->item( i )->text() ];
         if ( !plotted_curves.count( lb_files->item( i )->text() ) )
         {
            editor_msg( "red", QString( us_tr( "Internal error: scale selected %1, but no plotted curve found" ) ).arg( lb_files->item( i )->text() ) );
            return;
         }
      }
   }

   if ( !any_selected )
   {
      editor_msg( "red", us_tr( "Internal error: no files selected in scale mode" ) );
      return;
   }

   if ( !scale_q.count( wheel_file ) )
   {
      editor_msg( "red", QString( us_tr( "Internal error: %1 not found in data" ) ).arg( wheel_file ) );
      return;
   }

   if ( scale_q[ wheel_file ].size() < 2 )
   {
      editor_msg( "red", QString( us_tr( "Internal error: %1 almost empty data" ) ).arg( wheel_file ) );
      return;
   }

   if ( !scale_I.count( wheel_file ) )
   {
      editor_msg( "red", QString( us_tr( "Internal error: %1 not found in y data" ) ).arg( wheel_file ) );
      return;
   }

   if ( !scale_I[ wheel_file ].size() )
   {
      editor_msg( "red", QString( us_tr( "Internal error: %1 empty y data" ) ).arg( wheel_file ) );
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

   // us_qdebug( QString( "scale: q: [%1:%2] wheelfile %3 [%4:%5]" )
   //         .arg( le_scale_q_start->text() )
   //         .arg( le_scale_q_end->text() )
   //         .arg( wheel_file )
   //         .arg( scale_q[ wheel_file ][ 0 ] )
   //         .arg( scale_q[ wheel_file ].back() ) 
   //         );

   disable_all();
   cb_scale_scroll->setChecked( false );
   scale_scroll_pos = -1;

   mode_select( MODE_SCALE );

   if ( testiq_active )
   {
      pb_testiq->setEnabled( true );
   }

   scale_applied = false;

   running       = true;

   scale_enables();

   plotted_markers.clear( );
   gauss_add_marker( le_scale_q_start  ->text().toDouble(), Qt::red, us_tr( "Start") );
   gauss_add_marker( le_scale_q_end    ->text().toDouble(), Qt::red, us_tr( "End"  ), Qt::AlignLeft | Qt::AlignTop );
   plot_dist->replot();

   // if ( plot_errors_zoomer )
   // {
   //    delete plot_errors_zoomer;
   //    plot_errors_zoomer = (ScrollZoomer *) 0;
   // }

   scale_plotted_errors.clear( );
   plot_errors->detachItems( QwtPlotItem::Rtti_PlotCurve ); plot_errors->detachItems( QwtPlotItem::Rtti_PlotMarker );;
   plot_errors->replot();
   {
      QPixmap pm;
      lbl_wheel_Pcolor->setPixmap( pm );
   }
}

void US_Hydrodyn_Mals_Saxs::scale_scroll_highlight( int pos )
{
   // if ( pos >= (int) scale_scroll_selected.size() )
   // {
   //    pos = (int) scale_scroll_selected.size() - 1;
   // }

   lbl_wheel_pos->setText( "" );
   lbl_wheel_pos_below->setText( scale_scroll_selected[ pos ] );

#if QT_VERSION < 0x040000
   plot_dist->setCurveStyle( plotted_curves[ scale_scroll_selected[ scale_scroll_pos ] ], QwtCurve::NoCurve );
   plot_dist->setCurveStyle( plotted_curves[ scale_scroll_selected[ pos ] ], QwtCurve::Lines );
   if ( scale_plotted_errors.count( scale_scroll_selected[ scale_scroll_pos ] ) )
   {
      plot_errors->setCurveStyle( scale_plotted_errors[ scale_scroll_selected[ scale_scroll_pos ] ], QwtCurve::NoCurve );
   }
   if ( scale_plotted_errors.count( scale_scroll_selected[ pos ] ) )
   {
      plot_errors->setCurveStyle( scale_plotted_errors[ scale_scroll_selected[ pos ] ], QwtCurve::Lines );
   }
#else
   plotted_curves[ scale_scroll_selected[ scale_scroll_pos ] ]->setStyle( QwtPlotCurve::NoCurve );
   plotted_curves[ scale_scroll_selected[ pos ] ]->setStyle( QwtPlotCurve::Lines );
   if ( scale_plotted_errors.count( scale_scroll_selected[ scale_scroll_pos ] ) )
   {
      scale_plotted_errors[ scale_scroll_selected[ scale_scroll_pos ] ]->setStyle( QwtPlotCurve::NoCurve );
   }
   if ( scale_plotted_errors.count( scale_scroll_selected[ pos ] ) )
   {
      scale_plotted_errors[ scale_scroll_selected[ pos ] ]->setStyle( QwtPlotCurve::Lines );
   }
#endif
   plot_dist->replot();
   scale_scroll_pos = pos;
   plot_errors->replot();
}

void US_Hydrodyn_Mals_Saxs::scale_scroll()
{
   // us_qdebug( "--- scale_scroll() ---" );
   if ( le_last_focus && !cb_scale_scroll->isChecked() )
   {
      le_last_focus->clearFocus();
      cb_scale_scroll->setChecked( true );
   }

   if ( cb_scale_scroll->isChecked() )
   {
      ShowHide::hide_widgets( wheel_below_widgets, always_hide_widgets, false );
      le_last_focus = ( mQLineEdit * )0;
      scale_scroll_selected.clear( );
      for ( set < QString >::iterator it = scale_selected.begin();
            it != scale_selected.end();
            ++it )
      {
         scale_scroll_selected.push_back( *it );
#if QT_VERSION < 0x040000
         plot_dist->setCurveStyle( plotted_curves[ *it ], QwtCurve::NoCurve );
         if ( scale_plotted_errors.count( *it ) ) {
            plot_errors->setCurveStyle( scale_plotted_errors[ *it ], QwtCurve::NoCurve );
         }
#else
         plotted_curves[ *it ]->setStyle( QwtPlotCurve::NoCurve );
         if ( scale_plotted_errors.count( *it ) ) {
            scale_plotted_errors[ *it ]->setStyle( QwtPlotCurve::NoCurve );
         }
#endif
      }
      if ( scale_scroll_pos < 0 )
      {
         scale_scroll_pos = 0;
      } 
      if ( scale_scroll_pos >= (int) scale_scroll_selected.size() )
      {
         scale_scroll_pos = (int) scale_scroll_selected.size() - 1;
      }

      wheel_enables( false );
      qwtw_wheel->setRange( 0, scale_scroll_selected.size() - 1); qwtw_wheel->setSingleStep( 1 );
      qwtw_wheel->setValue( scale_scroll_pos );
      lbl_wheel_pos->setText( QString( "%1" ).arg( scale_scroll_selected[ scale_scroll_pos ] ) );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      wheel_enables();
      scale_scroll_highlight( scale_scroll_pos );
   } else {
      ShowHide::hide_widgets( wheel_below_widgets, always_hide_widgets );
      // go thru all displayed curves, turn on
      for ( set < QString >::iterator it = scale_selected.begin();
            it != scale_selected.end();
            ++it )
      {
#if QT_VERSION < 0x040000
         plot_dist->setCurveStyle( plotted_curves[ *it ], QwtCurve::Lines );
         if ( scale_plotted_errors.count( *it ) )
         {
            plot_errors->setCurveStyle( scale_plotted_errors[ *it ], QwtCurve::Lines );
         }
#else
         plotted_curves[ *it ]->setStyle( QwtPlotCurve::Lines );
         if ( scale_plotted_errors.count( *it ) )
         {
             scale_plotted_errors[ *it ]->setStyle( QwtPlotCurve::Lines );
         }
#endif
      }
      lbl_wheel_pos->setText( "" );
      le_last_focus = ( mQLineEdit * )0;
      wheel_enables( false );
      plot_dist->replot();
      plot_errors->replot();
   }
   scale_enables();
}

void US_Hydrodyn_Mals_Saxs::scale_update_plot_errors()
{
   // us_qdebug( "--- scale_update_plot_errors() ---" );

   if ( !scale_applied || !scale_spline_x.size() )
   {
      scale_plotted_errors.clear( );
      plot_errors->detachItems( QwtPlotItem::Rtti_PlotCurve ); plot_errors->detachItems( QwtPlotItem::Rtti_PlotMarker );;
      plot_errors->replot();
      return;
   }

   plot_errors->detachItems( QwtPlotItem::Rtti_PlotCurve ); plot_errors->detachItems( QwtPlotItem::Rtti_PlotMarker );;

   double q_min = scale_q[ scale_applied_target ][ 0 ];
   double q_max = scale_q[ scale_applied_target ].back();

   // double qs_min = scale_spline_x[ 0 ];
   // double qs_max = scale_spline_x.back();

   vector < double > this_q;
   vector < double > this_diff;
   double    y;

   // us_qdebug( US_Vector::qs_vector2( QString( "spline avg target %1" ).arg( scale_applied_target ), scale_spline_x, scale_spline_y ) );

   double diff_ymin = 1e99;
   double diff_ymax = -1e99;

   // us_qdebug( QString( "q  min %1 max %2" ).arg( q_min ).arg( q_max ) );
   // us_qdebug( QString( "qs min %1 max %2" ).arg( qs_min ).arg( qs_max ) );

   bool hide = cb_scale_scroll->isChecked();

   for ( set < QString >::iterator it = scale_selected.begin();
         it != scale_selected.end();
         ++it )
   {
      this_q   .clear( );
      this_diff.clear( );

      for ( int i = 0; i < (int) scale_q[ *it ].size(); ++i )
      {
         if ( scale_q[ *it ][ i ] >= q_min &&
              scale_q[ *it ][ i ] <= q_max )
         {
            if ( usu->apply_natural_spline( scale_spline_x,
                                            scale_spline_y,
                                            scale_spline_y2,
                                            scale_q[ *it ][ i ],
                                            y ) )
            {
               this_q   .push_back( scale_q[ *it ][ i ] );
               this_diff.push_back( y - scale_I[ *it ][ i ] );
               if ( diff_ymin > this_diff.back() )
               {
                  diff_ymin = this_diff.back();
               }
               if ( diff_ymax < this_diff.back() )
               {
                  diff_ymax = this_diff.back();
               }
            }
         }
      }

#if QT_VERSION >= 0x040000
      QwtPlotCurve *curve = new QwtPlotCurve( *it );
      curve->setStyle( hide ? QwtPlotCurve::NoCurve : QwtPlotCurve::Lines );
      curve->setPen( QPen( plot_colors[ f_pos[ *it ] % plot_colors.size() ], use_line_width, Qt::SolidLine ) );
      curve->setSamples(
                     (double *)&this_q   [ 0 ],
                     (double *)&this_diff[ 0 ],
                     this_q.size()
                     );
      curve->attach( plot_errors );
#else
      long curve;
      curve = plot_errors->insertCurve( *it );
      plot_errors->setCurveStyle( curve, hide ? QwtPlotCurve::NoCurve : QwtPlotCurve::Lines );
      plot_errors->setCurvePen( curve, QPen( plot_colors[ f_pos[ *it ] % plot_colors.size() ], use_line_width, Qt::SolidLine ) );
      plot_errors->setCurveData( curve,
                                 (double *)&this_q   [ 0 ],
                                 (double *)&this_diff[ 0 ],
                                 this_q.size()
                                 );
#endif
      scale_plotted_errors[ *it ] = curve;
      // us_qdebug( US_Vector::qs_vector2( QString( "diff plot %1" ).arg( *it ), this_q, this_diff ) );
   }

   plot_errors->setAxisScale( QwtPlot::yLeft  , diff_ymin * 1.1e0 , diff_ymax * 1.1e0 );

   if ( plot_dist_zoomer )
   {
#if QT_VERSION < 0x040000
      double minx = plot_dist_zoomer->zoomRect().x1();
      double maxx = plot_dist_zoomer->zoomRect().x2();
#else
      double minx = plot_dist_zoomer->zoomRect().left();
      double maxx = plot_dist_zoomer->zoomRect().right();
#endif
      plot_errors->setAxisScale( QwtPlot::xBottom, minx, maxx );
   } else {
      plot_errors->setAxisScale( QwtPlot::xBottom, q_min, q_max );
   }

//    if ( plot_errors_zoomer )
//    {
//       delete plot_errors_zoomer;
//       plot_errors_zoomer = (ScrollZoomer *) 0;
//    }

//    plot_errors_zoomer = new ScrollZoomer(plot_errors->canvas());
//    plot_errors_zoomer->setRubberBandPen(QPen(Qt::yellow, 0, Qt::DotLine));
//    plot_errors_zoomer->setTrackerPen(QPen(Qt::red));
   // connect( plot_errors_zoomer, SIGNAL( zoomed( const QRectF & ) ), SLOT( plot_zoomed( const QRectF & ) ) );

   if ( hide )
   {
      scale_scroll_highlight( scale_scroll_pos );
   } else {
      plot_errors->replot();
   }
}

void US_Hydrodyn_Mals_Saxs::scale_enables()
{
   pb_errors             ->setEnabled( true );
   pb_cormap             ->setEnabled( true );
   pb_timeshift          ->setEnabled( false );
   pb_wheel_cancel       ->setEnabled( true );
   rb_scale_high         ->setEnabled( true );
   rb_scale_low          ->setEnabled( true );
   le_scale_q_start      ->setEnabled( true );
   le_scale_q_end        ->setEnabled( true );
   pb_scale_q_reset      ->setEnabled( true );
   pb_scale_reset        ->setEnabled( true );
   pb_scale_apply        ->setEnabled( true );
   pb_scale_create       ->setEnabled( scale_applied );
   cb_scale_scroll       ->setEnabled( true );
   pb_rescale            ->setEnabled( true );
   pb_rescale_y          ->setEnabled( true );
   pb_axis_x             ->setEnabled( true );
   pb_axis_y             ->setEnabled( true );
   pb_pp                 ->setEnabled( true );

   ShowHide::hide_widgets( wheel_below_widgets, always_hide_widgets, !cb_scale_scroll->isChecked() );
}

QString US_Hydrodyn_Mals_Saxs::scale_get_target( bool do_msg )
{
   double min_i = 0e0;
   double max_i = 0e0;
   double tot_i = 0e0;

   double q_min = le_scale_q_start->text().toDouble();
   double q_max = le_scale_q_end  ->text().toDouble();

   QString msg;

   if ( !scale_selected.size() )
   {
      editor_msg( "red",  us_tr( "Scale: internal error, no selected files" ) );
      return "";
   }

   if ( q_min >= q_max )
   {
      editor_msg( "red",  us_tr( "Error: Scale mode: minimum q value is greator or equal to maximum q value" ) );
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

void US_Hydrodyn_Mals_Saxs::scale_apply()
{
   // us_qdebug( "---- scale apply ----" );

   QString target = scale_get_target( true );
   if ( target.isEmpty() )
   {
      scale_enables();
      return;
   }

   editor_msg( "blue", QString( us_tr( "scaling target is %1" ) ).arg( target ) );

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
               avg_std_dev_frac += 
                  ( scale_e[ target ][ i ] * scale_e[ target ][ i ] ) /
                  ( scale_I[ target ][ i ] * scale_I[ target ][ i ] );
               avg_std_dev_point_count++;
            }
         }
      }
   }            

   if ( !avg_std_dev_point_count ) {
      avg_std_dev_point_count = 1;
   }

   avg_std_dev_frac = sqrt( avg_std_dev_frac / ( double ) avg_std_dev_point_count );

   if ( !target_q.size() )
   {
      editor_msg( "red", QString( us_tr( "Error: Scale 'apply': the target curve for scaling has no points in the selected q range" ) ) );
      scale_enables();
      return;
   }

   bool do_chi2_fitting        = our_saxs_options->iqq_scale_chi2_fitting;
   bool do_scale_linear_offset = our_saxs_options->iqq_scale_linear_offset;

   QString notices;

   if ( our_saxs_options->ignore_errors && target_has_e )
   {
      notices += us_tr( "Ignoring experimental errors in target\n" );
      do_chi2_fitting = false;
   }

   if ( do_chi2_fitting && !target_has_e )
   {
      notices += us_tr( "\nChi^2 fitting requested, but target data has no or some zero standard deviation data. Chi^2 fitting not used" );
      do_chi2_fitting = false;
   }

   if ( our_saxs_options->iqq_scale_nnls && do_chi2_fitting && our_saxs_options->iqq_scale_chi2_fitting )
   {
      notices += us_tr( "\nChi^2 fitting is currently not compatable with NNLS scaling, Chi^2 fitting not used" );
      do_chi2_fitting = false;
   }

   if ( our_saxs_options->iqq_scale_nnls && do_scale_linear_offset )
   {
      notices += us_tr("\nScale with linear offset is not compatable with NNLS scaling" );
      do_scale_linear_offset = false;
   }

   if ( !our_saxs_options->iqq_scale_nnls && do_scale_linear_offset )
   {
      notices +=  us_tr("\nScale with linear offset is not currently implemented");
      do_scale_linear_offset = false;
   }

   if ( !notices.isEmpty() )
   {
      editor_msg( "dark red", "Scale 'apply' notices:" + notices );
   }         
   notices = "";

   if ( do_chi2_fitting )
   {
      editor_msg( "dark blue", us_tr(  "Chi^2 fitting\n" ) );
   }

   map < QString, double > k;
   map < QString, double > chi2;

   vector < double > average_scaled_I = scale_I[ target ];
   vector < int    > average_scaled_I_count( average_scaled_I.size(), 1 );

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
                        QString( us_tr( "Error: Scale 'apply': %1 could not interpolate to target on specified q range: %2" ) )
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

         {
            int max = (int) average_scaled_I.size();
            max = (int) scale_I[ *it ].size() < max ? (int) scale_I[ *it ].size() : max;
            for ( int i = 0; i < max; ++i )
            {
               average_scaled_I      [ i ] += scale_I[ *it ][ i ] * k[ *it ];
               average_scaled_I_count[ i ]++;
            }
         }
      }
   }

   for ( int i = 0; i < (int) average_scaled_I.size(); ++i )
   {
      average_scaled_I[ i ] /= (double) average_scaled_I_count[ i ];
   }

   // us_qdebug( US_Vector::qs_vector2( "scale_q[ target ], average_scaled_I", scale_q[ target ], average_scaled_I ) );

   scale_spline_x = scale_q[ target ];
   scale_spline_y = average_scaled_I;

   usu->natural_spline( scale_spline_x, scale_spline_y, scale_spline_y2 );

   QString fit_msg = "";

   for ( set < QString >::iterator it = scale_selected.begin();
         it != scale_selected.end();
         ++it )
   {
      if ( *it != target )
      {
         bool source_has_e = ( scale_q[ *it ].size() == scale_e[ *it ].size() );
         for ( int i = 0; i < (int) scale_I[ *it ].size(); ++i )
         {
            scale_I[ *it ][ i ] *= k[ *it ];
            if ( source_has_e )
            {
               scale_e[ *it ][ i ] *= k[ *it ];
            }
         }
         // us_qdebug( US_Vector::qs_vector2( QString( "%1 scale_q, scale_I" ).arg( *it ), scale_q[ *it ], scale_I[ *it ] ) );
         fit_msg += QString( "%1 : scale %2 " ).arg( *it ).arg( k[ *it ] );
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
               fit_msg += QString( " r_sigma=%1 nchi*r_sigma=%2 " )
                  .arg( avg_std_dev_frac ) 
                  .arg( avg_std_dev_frac * sqrt( chi2[ *it ] / ( target_I.size() - ( do_scale_linear_offset ? 2 : 1 ) ) ), 5 );
            }
         } else {
            fit_msg += QString( "RMSD = %1" ).arg( chi2[ *it ], 5 );
         }
         fit_msg += "\n";
      // } else {
      //    us_qdebug( US_Vector::qs_vector2( QString( "%1 scale_q, scale_I" ).arg( *it ), scale_q[ *it ], scale_I[ *it ] ) );
      }         
   }

   editor_msg( "dark blue", fit_msg );

   scale_replot();
   scale_applied_q_min  = q_min;
   scale_applied_q_max  = q_max;
   scale_applied_target = target;
   scale_applied        = true;

   // setup for scale analysis
   {
      // vector < double > fit_q_range;
      // for ( int i = 0; i < (int) scale_q[ scale_applied_target ].size(); ++i ) {
      //    if ( scale_q[ scale_applied_target ][ i ] >= q_min &&
      //         scale_q[ scale_applied_target ][ i ] <= q_max ) {
      //       fit_q_range.push_back( scale_q[ scale_applied_target ][ i ] );
      //    }
      // }

      map < QString, QString > parameters;
      map < QString, vector < vector < double > > > plots; // name, x, y and possibly error
      parameters[ "title"  ] = us_tr( "Scale analysis" );
      parameters[ "yleft"  ] = us_tr( "q fit slope and slope error" );
      parameters[ "qmin"   ] = QString( "%1").arg( q_min );
      parameters[ "qmax"   ] = QString( "%1").arg( q_max );

      // build up scaled interpolated set

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

         {
            QColor tmp = plot_colors[ f_pos[ *it ] % plot_colors.size() ];
            parameters[ QString( "_color:%1" ).arg( name ) ] = QString( "%1" ).arg( (double) ( tmp.red() * 256 + tmp.green() ) * 256 + tmp.blue() );
         }

         if ( cb_scale_save_intp->isChecked() && *it != scale_applied_target )
         {
            double q_min = scale_q[ scale_applied_target ][ 0 ];
            double q_max = scale_q[ scale_applied_target ].back();
            if ( q_min < scale_q[ *it ][ 0 ] )
            {
               q_min = scale_q[ *it ][ 0 ];
               parameters[ "qmin"   ] = QString( "%1").arg( q_min );
            }
            if ( q_max > scale_q[ *it ].back() )
            {
               q_max = scale_q[ *it ].back();
               parameters[ "qmax"   ] = QString( "%1").arg( q_max );
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
                           QString( us_tr( "Error: Scale 'apply': %1 could not interpolate to target on specified q range: %2" ) )
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
                              QString( us_tr( "Error: Scale 'apply': %1 could not interpolate to target on specified q range: %2" ) )
                              .arg( *it )
                              .arg( usu->errormsg ) );
                  scale_enables();
                  return;
               }
            }

            plots[ name ].push_back( target_q );
            plots[ name ].push_back( target_I );
            plots[ name ].push_back( target_e );

         } else {
            plots[ name ].push_back( scale_q[ *it ] );
            plots[ name ].push_back( scale_I[ *it ] );
            plots[ name ].push_back( scale_e[ *it ] );
         }
      }
      
      // US_Hydrodyn_Mals_Saxs_Scale_Trend * uhshst = new US_Hydrodyn_Mals_Saxs_Scale_Trend( us_hydrodyn,
      //                                                                                     parameters,
      //                                                                                     plots );
      // uhshst->show();
   }

   scale_update_plot_errors();
   scale_enables();
}

void US_Hydrodyn_Mals_Saxs::scale_q_reset()
{
   le_scale_q_start->setText( QString( "%1" ).arg( scale_q[ wheel_file ][ 0 ]   ) );
   le_scale_q_end  ->setText( QString( "%1" ).arg( scale_q[ wheel_file ].back() ) );
}

void US_Hydrodyn_Mals_Saxs::scale_reset()
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

   scale_spline_x    .clear( );
   scale_spline_y    .clear( );
   scale_spline_y2   .clear( );

   plot_errors->detachItems( QwtPlotItem::Rtti_PlotCurve ); plot_errors->detachItems( QwtPlotItem::Rtti_PlotMarker );;
   plot_errors->replot();
}

void US_Hydrodyn_Mals_Saxs::scale_replot()
{
   for ( set < QString >::iterator it = scale_selected.begin();
         it != scale_selected.end();
         ++it )
   {
#if QT_VERSION < 0x040000
      plot_dist->setCurveData( plotted_curves[ *it ], 
                               (double *)&( scale_q[ *it ][ 0 ] ),
                               (double *)&( scale_I[ *it ][ 0 ] ),
                               scale_q[ *it ].size()
                               );
#else
      plotted_curves[ *it ]->setSamples(
                           (double *)&( scale_q[ *it ][ 0 ] ),
                           (double *)&( scale_I[ *it ][ 0 ] ),
                           scale_q[ *it ].size()
                           );
#endif
   }
   plot_dist->replot();
}

void US_Hydrodyn_Mals_Saxs::scale_create()
{
   scale_last_created.clear( );

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
                        QString( us_tr( "Error: Scale 'apply': %1 could not interpolate to target on specified q range: %2" ) )
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
                           QString( us_tr( "Error: Scale 'apply': %1 could not interpolate to target on specified q range: %2" ) )
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

void US_Hydrodyn_Mals_Saxs::scale_q_start_text( const QString & text )
{
   if ( current_mode != MODE_SCALE )
   {
      return;
   }
#if QT_VERSION < 0x040000
   plot_dist->setMarkerPos( plotted_markers[ 0 ], text.toDouble(), 0e0 );
#else
   plotted_markers[ 0 ]->setXValue( text.toDouble() );
#endif
   if ( !wheel_is_pressed && qwtw_wheel->value() != text.toDouble() )
   {
      qwtw_wheel->setValue( text.toDouble() );
   }
   plot_dist->replot();
   scale_enables();
}

void US_Hydrodyn_Mals_Saxs::scale_q_end_text( const QString & text )
{
   if ( current_mode != MODE_SCALE )
   {
      return;
   }
#if QT_VERSION < 0x040000
   plot_dist->setMarkerPos( plotted_markers[ 1 ], text.toDouble(), 0e0 );
#else
   plotted_markers[ 1 ]->setXValue( text.toDouble() );
#endif
   if ( !wheel_is_pressed && qwtw_wheel->value() != text.toDouble() )
   {
      qwtw_wheel->setValue( text.toDouble() );
   }
   plot_dist->replot();
   scale_enables();
}

void US_Hydrodyn_Mals_Saxs::scale_q_start_focus( bool hasFocus )
{
   if ( current_mode != MODE_SCALE )
   {
      return;
   }
   if ( hasFocus && scale_q.count( wheel_file ) )
   {
      le_last_focus = le_scale_q_start;
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      qwtw_wheel->setRange( scale_q[ wheel_file ][ 0 ], scale_q[ wheel_file ].back()); qwtw_wheel->setSingleStep( ( scale_q[ wheel_file ].back() - scale_q[ wheel_file ][ 0 ] ) / UHSH_WHEEL_RES );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setValue( le_scale_q_start->text().toDouble() );
      wheel_enables();
   }
}

void US_Hydrodyn_Mals_Saxs::scale_q_end_focus( bool hasFocus )
{
   if ( current_mode != MODE_SCALE )
   {
      return;
   }
   if ( hasFocus && scale_q.count( wheel_file ) )
   {
      le_last_focus = le_scale_q_end;
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      qwtw_wheel->setRange( scale_q[ wheel_file ][ 0 ], scale_q[ wheel_file ].back()); qwtw_wheel->setSingleStep( ( scale_q[ wheel_file ].back() - scale_q[ wheel_file ][ 0 ] ) / UHSH_WHEEL_RES );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setValue( le_scale_q_end->text().toDouble() );
      wheel_enables();
   }
}

// --- ggaussian (partial ... move more from mals_saxs.cpp

void US_Hydrodyn_Mals_Saxs::ggauss_start()
{
   plot_errors->detachItems( QwtPlotItem::Rtti_PlotCurve ); plot_errors->detachItems( QwtPlotItem::Rtti_PlotMarker );;
   // if ( plot_errors_zoomer )
   // {
   //    delete plot_errors_zoomer;
   //    plot_errors_zoomer = (ScrollZoomer *) 0;
   // }

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
   if ( ggqfit_plot_zoomer )
   {
      delete ggqfit_plot_zoomer;
      ggqfit_plot_zoomer = (ScrollZoomer *) 0;
   }

   le_last_focus = (mQLineEdit *) 0;
   pb_gauss_fit->setText( us_tr( "Global Fit" ) );
   pb_cormap     ->setText( us_tr( "Show PVP map" ) );
   ggauss_scroll_save_group = cb_plot_errors_group->isChecked();
      
   disable_all();

   ggaussian_selected_file_index.clear( );

   for ( int i = 0; i < lb_files->count(); i++ )
   {
      if ( lb_files->item( i )->isSelected() )
      {
         ggaussian_selected_file_index.push_back( i );
      }
   }

   int no_gaussian_count = ggaussian_sel_no_gaussian_count();

   QTextStream( stdout ) << "ggaussian_sel_no_gaussian_count() " << no_gaussian_count << Qt::endl;
   QTextStream( stdout ) << "total selected " << ggaussian_selected_file_index.size() << Qt::endl;

   bool not_compatible = !ggaussian_compatible();

   QString msg_addendum = ""; 

   if ( not_compatible ) {
      if ( cb_fix_width->isChecked() )
      {
         msg_addendum += " or widths";
      }
      if ( dist1_active && cb_fix_dist1->isChecked() )
      {
         msg_addendum += " or distortion-1s";
      }
      if ( dist2_active && cb_fix_dist2->isChecked() )
      {
         msg_addendum += " or distortion-2s";
      }
      QString msg = QString( us_tr( "NOTICE: Some files selected have Gaussians with varying centers%1 or a different number of Gaussians or centers that do not match the last Gaussians, these will be reset to the last Gaussian settings" ) )
         .arg( msg_addendum );

      editor_msg( "dark red", msg );
   }

   if ( !cb_sd_weight->isChecked() )
   {
      switch ( QMessageBox::question(this, 
                                     windowTitle() + us_tr( ": Global Gaussians" ),
                                     QString( us_tr( "Note: SD weighting is currently turned off.\n" ) ),
                                     us_tr( "&Turn on" ), 
                                     us_tr( "&Leave off" ),
                                     QString(),
                                     0, // Stop == button 0
                                     0 // Escape == button 0
                                     ) ) {
      case 0 : // Turn on
         cb_sd_weight->setChecked( true );
         break;
      case 1 : // Leave off
         break;
      }       
   }      

   pb_ggauss_rmsd->setText( QString( us_tr( "Recompute %1" ).arg( cb_sd_weight->isChecked() ? "nChi^2" : "RMSD" ) ) );

   org_f_gaussians = f_gaussians;

   {
      if ( not_compatible ) {
         QMessageBox::information(
                                  this, 
                                  windowTitle() + us_tr( ": Global Gaussians" ),
                                  QString( us_tr( "NOTICE: Some files selected have Gaussians with varying centers%1 or\na different number of Gaussians or centers that do not match the last Gaussians.\n" ) ) .arg( msg_addendum )
                                  + "\n" + us_tr( "Global Gaussians will be reinitialized\n" ) );
         if ( !create_unified_ggaussian_target( true, false ) ) {
            update_enables();
            return;
         }
      } else {                                        
         if ( no_gaussian_count ) {
            if ( no_gaussian_count == (int) ggaussian_selected_file_index.size() ) {
               QMessageBox::information(
                                        this, 
                                        windowTitle() + us_tr( ": Global Gaussians" ),
                                        msg_addendum + "\n" + us_tr( "Global Gaussians will be initialized\n" ) );
         
               if ( !create_unified_ggaussian_target( true, false ) ) {
                  update_enables();
                  return;
               }
            } else {
               switch ( QMessageBox::question(this, 
                                              windowTitle() + us_tr( ": Global Gaussians" ),
                                              QString( us_tr( "%1 selected curves do not have defined Gaussians and must be initialized" ) ).arg( no_gaussian_count ),
                                              us_tr( "&Reinitialize only curves with missing Gaussians" ),
                                              us_tr( "&Reinitialize all curves with last used Gaussians" ),
                                              QString(),
                                              0, // Stop == button 0
                                              0 // Escape == button 0
                                              ) ) {
               case 0 : // Reinitialize only curves with missing Gaussians
                  if ( !create_unified_ggaussian_target( true, true ) ) {
                     update_enables();
                     return;
                  }
                  break;
               case 1 : // Reinitialize all curves
                  if ( !create_unified_ggaussian_target( true, false ) ) {
                     update_enables();
                     return;
                  }
                  break;
               }
            }
         } else {
            bool do_rescale = false;

            switch ( QMessageBox::question(this, 
                                           windowTitle() + us_tr( ": Global Gaussians" ),
                                           QString( us_tr(
                                                          "Rescale and refit the Gaussian amplitudes?\n\n"
                                                          "Answer \"Yes\" if you want to re-fit the amplitudes using the last single curve Gaussian known"
                                                          " [the most recent of a Gaussian file load or single Gaussian \"Keep\"].\n\n"
                                                          "Answer \"No\" if you want to maintain the current values." ) ),

                                           QMessageBox::Yes | QMessageBox::No,
                                           QMessageBox::No
                                           ) ) {
            case QMessageBox::Yes : // Yes, rescale
               do_rescale = true;
               break;
            case QMessageBox::No : // No do not rescale
            default :
               do_rescale = false;
               break;
            }       
   
            if ( !create_unified_ggaussian_target( do_rescale, false ) ) {
               update_enables();
               return;
            }
         }
      }
   }

   // add_ggaussian_curve( "unified_ggaussian_target", unified_ggaussian_I );
   // add_ggaussian_curve( "unified_ggaussian_sum",    compute_ggaussian_gaussian_sum() );

   // ggaussian_mode = true;
   mode_select( MODE_GGAUSSIAN );
   ShowHide::hide_widgets( ggqfit_widgets, always_hide_widgets );
   cb_ggauss_scroll->setChecked( false );

   lbl_gauss_fit->setText( QString( "%1" ).arg( ggaussian_rmsd(), 0, 'g', 5 ) );
   wheel_file = unified_ggaussian_files[ 0 ];

   get_peak( wheel_file, gauss_max_height );
   gauss_max_height *= 1.2;
   if ( gaussian_type != GAUSS )
   {
      gauss_max_height *= 20e0;
   }
      
   // org_f_gaussians = f_gaussians;

   running        = true;

   if ( gaussian_pos && gaussian_pos >= unified_ggaussian_gaussians_size )
   {
      gaussian_pos = unified_ggaussian_gaussians_size - 1;
   }
   
   gauss_init_markers();
   disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
   qwtw_wheel->setRange( f_qs[ wheel_file ][ 0 ], f_qs[ wheel_file ].back()); qwtw_wheel->setSingleStep( ( f_qs[ wheel_file ].back() - f_qs[ wheel_file ][ 0 ] ) / UHSH_WHEEL_RES );
   // qwtw_wheel->setValue( unified_ggaussian_params[ gaussian_pos ] );
   // us_qdebug( QString( "setting wheel pos to %1 gaussian pos %2" ).arg( unified_ggaussian_params[ gaussian_pos ] ).arg( gaussian_pos ) );
   update_gauss_pos();
   qwtw_wheel->setValue( le_gauss_pos->text().toDouble() );
   us_qdebug( QString( "setting wheel pos to %1 gaussian pos %2" ).arg( le_gauss_pos->text() ).arg( gaussian_pos ) );
   le_gauss_pos->setFocus();
   connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );

   check_fit_range();

   if ( errors_were_on )
   {
      hide_widgets( plot_errors_widgets, false );
      if ( !unified_ggaussian_use_errors )
      {
         cb_plot_errors_sd->setChecked( false );
         cb_plot_errors_sd->hide();
      }
      emit do_resize_plots();
   }

   ggaussian_enables();
}

void US_Hydrodyn_Mals_Saxs::ggauss_scroll()
{
   if ( cb_ggauss_scroll->isChecked() ) {
      // le_last_focus = (mQLineEdit *) 0;
      cb_ggauss_scroll_p_green ->setEnabled( true );
      cb_ggauss_scroll_p_yellow->setEnabled( true );
      cb_ggauss_scroll_p_red   ->setEnabled( true );
      cb_ggauss_scroll_smoothed->setEnabled( true );
      cb_ggauss_scroll_oldstyle->setEnabled( true );
         
      disable_updates = true;
      gauss_delete_markers();
      lb_files->clearSelection();
      disable_updates = false;
      // plot_files();
      ggauss_scroll_save_group = cb_plot_errors_group->isChecked();
      cb_plot_errors_group->setChecked( false );
      ggauss_scroll_set_selected();

      // // clear plot & leave markers
      // gauss_delete_markers();
      // plot_dist->detachItems( QwtPlotItem::Rtti_PlotCurve ); plot_dist->detachItems( QwtPlotItem::Rtti_PlotMarker );;
      // gauss_init_markers();
      // plot_dist->replot();
   } else {
      cb_ggauss_scroll_p_green ->setEnabled( false );
      cb_ggauss_scroll_p_yellow->setEnabled( false );
      cb_ggauss_scroll_p_red   ->setEnabled( false );
      cb_ggauss_scroll_smoothed->setEnabled( false );
      cb_ggauss_scroll_oldstyle->setEnabled( false );
      // restore to ggaussian plot mode
      disable_updates = true;
      gauss_delete_markers();
      lb_files->clearSelection();
      for ( int i = 0; i < (int) ggaussian_selected_file_index.size(); ++i ) {
         lb_files->item( ggaussian_selected_file_index[ i ])->setSelected( true );
      }
      disable_updates = false;
      suppress_replot = true;
      plot_files();
      suppress_replot = false;
      gauss_init_markers();
      plot_dist->replot();

      // if ( plot_errors_zoomer )
      // {
      //    delete plot_errors_zoomer;
      //    plot_errors_zoomer = (ScrollZoomer *) 0;
      // }
      // if ( plot_errors->isVisible() ) {
      //    cb_plot_errors_group->show();
      // }
      cb_plot_errors_group->setChecked( ggauss_scroll_save_group );
      cb_plot_errors_group->setChecked( true );
      ggaussian_rmsd();
   }
   ggaussian_enables();
}
   
void US_Hydrodyn_Mals_Saxs::ggauss_scroll_p_green() 
{
   us_qdebug( "ggauss_scroll_p_green()" );
   ggauss_scroll_set_selected();
}

void US_Hydrodyn_Mals_Saxs::ggauss_scroll_p_red() 
{
   us_qdebug( "ggauss_scroll_p_yellow()" );
   ggauss_scroll_set_selected();
}

void US_Hydrodyn_Mals_Saxs::ggauss_scroll_p_yellow() 
{
   us_qdebug( "ggauss_scroll_p_yellow()" );
   ggauss_scroll_set_selected();
}

void US_Hydrodyn_Mals_Saxs::ggauss_scroll_smoothed() 
{
   us_qdebug( "ggauss_scroll_smoothed()" );
   ggauss_scroll_set_selected();
}

static int ggauss_scroll_highlight_last_pos;

void US_Hydrodyn_Mals_Saxs::ggauss_scroll_oldstyle() 
{
   us_qdebug( "ggauss_scroll_oldstyle()" );
   ggauss_scroll_highlight( ggauss_scroll_highlight_last_pos );
}

void US_Hydrodyn_Mals_Saxs::ggauss_scroll_set_selected()
{
   us_qdebug( "ggauss_scroll_set_selected()" );
   // build up set by pvalue 
   ggauss_scroll_set.clear( );

   if ( !unified_ggaussian_ok ) {
      editor_msg( "red", us_tr( "Internal error (ggauss_scroll_set_selected): Global Gaussian mode, but unified Global Gaussians are not ok." ) );
      ggaussian_enables();
      return;
   }
            
   if ( 
       (int) unified_ggaussian_files.size() != (int) ggaussian_last_pfit_P.size() ||
       (int) unified_ggaussian_files.size() != (int) ggaussian_last_pfit_N.size() ||
       (int) unified_ggaussian_files.size() != (int) ggaussian_last_pfit_C.size() ||
       (int) unified_ggaussian_files.size() != (int) ggaussian_last_pfit_S.size() ||
       (int) unified_ggaussian_files.size() != (int) ggaussian_last_chi2.size() ||
       lbl_gauss_fit->text() == "?" ||
       pb_ggauss_rmsd->isEnabled()
        ){
      if ( ggauss_recompute() ) {
         lbl_gauss_fit->setText( QString( "%1" ).arg( ggaussian_rmsd(), 0, 'g', 5 ) );
         pb_ggauss_rmsd->setEnabled( false );
      } else {
         editor_msg( "red", us_tr( "Internal error (ggauss_scroll_set_selected): building global Gaussians" ) );
         ggaussian_enables();
         return;
      }
   }
   
   if ( (int) unified_ggaussian_files.size() != (int) ggaussian_last_pfit_P.size() ) {
      editor_msg( "red", us_tr( "Internal error (ggauss_scroll_set_selected): P set does not match Gaussian size" ) );
      ggaussian_enables();
      return;
   }

   if ( (int) unified_ggaussian_files.size() != (int) ggaussian_last_chi2.size() ) {
      editor_msg( "red", us_tr( "Internal error (ggauss_scroll_set_selected): chi2 set does not match Gaussian size" ) );
      ggaussian_enables();
      return;
   }

   int fcount = (int) unified_ggaussian_files.size();

   for ( int i = 0; i < fcount; ++i ) {
      if ( cb_ggauss_scroll_smoothed->isChecked() ) {
         if ( !f_best_smoothed_smoothing.count( unified_ggaussian_files[ i ] ) ) {
            continue;
         }
      }
      if ( cb_ggauss_scroll_oldstyle->isChecked() ) {
         if ( !f_qs_oldstyle.count( unified_ggaussian_files[ i ] ) ) {
            continue;
         }
      }
      if ( ggaussian_last_pfit_P[ i ] >= 0.05 ) {
         if ( cb_ggauss_scroll_p_green->isChecked() ) {
            ggauss_scroll_set.push_back( i );
         }
      } else {
         if ( ggaussian_last_pfit_P[ i ] >= 0.01 ) {
            if ( cb_ggauss_scroll_p_yellow->isChecked() ) {
               ggauss_scroll_set.push_back( i );
            }
         } else {
            if ( cb_ggauss_scroll_p_red->isChecked() ) {
               ggauss_scroll_set.push_back( i );
            }
         }
      }
   }

   // for ( int i = 0; i < (int) ggauss_scroll_set.size(); ++i ) {
   //    us_qdebug( QString( "active %1 file %2" ).arg( ggauss_scroll_set[ i ] ).arg( unified_ggaussian_files[ ggauss_scroll_set [ i ] ] ) );
   // }

   if ( ggauss_scroll_set.size() ) {
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      qwtw_wheel->setRange( 0, ggauss_scroll_set.size() - 1); qwtw_wheel->setSingleStep( 1 );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setValue( 0 );
      ggauss_scroll_highlight( 0 );
   } else {
      QPixmap pm;
      lbl_wheel_Pcolor->setPixmap( pm );
      lbl_wheel_pos_below->setText( us_tr( "-- no curves match current selection --" ) );
      disable_updates = true;
      gauss_delete_markers();
      lb_files->clearSelection();
      disable_updates = false;
      suppress_replot = true;
      plot_files();
      gauss_init_markers();
      suppress_replot = false;
      plot_dist->replot();

      // clear highlighted ggqfit_plot for chi2 and pfit
      {
#if QT_VERSION < 0x050000
         QwtSymbol sym;
#else
         const QwtSymbol * sym;
#endif
         // clear any highlighted symbols
#if QT_VERSION >= 0x040000
# if QT_VERSION < 0x050000
         for ( set < QwtPlotMarker * >::iterator it = ggaussian_pts_chi2_marked.begin();
               it != ggaussian_pts_chi2_marked.end();
               it++ ) {
            sym = (*it)->symbol();
            sym.setSize( use_line_width * 3 + 1 );
            (*it)->setSymbol( new QwtSymbol( sym.style(), sym.brush(), sym.pen(), sym.size() ) );
         }
         for ( set < QwtPlotMarker * >::iterator it = ggaussian_pts_pfit_marked.begin();
               it != ggaussian_pts_pfit_marked.end();
               it++ ) {
            sym = (*it)->symbol();
            sym.setSize( use_line_width * 2 + 1 );
            (*it)->setSymbol( new QwtSymbol( sym.style(), sym.brush(), sym.pen(), sym.size() ) );
         }
# else
         for ( set < QwtPlotMarker * >::iterator it = ggaussian_pts_chi2_marked.begin();
               it != ggaussian_pts_chi2_marked.end();
               it++ ) {
            sym = (*it)->symbol();
            (*it)->setSymbol( new QwtSymbol( sym->style(), sym->brush(), sym->pen(), QSize( use_line_width * 3 + 1, use_line_width * 3 + 1 ) ) );
         }
         for ( set < QwtPlotMarker * >::iterator it = ggaussian_pts_pfit_marked.begin();
               it != ggaussian_pts_pfit_marked.end();
               it++ ) {
            sym = (*it)->symbol();
            (*it)->setSymbol( new QwtSymbol( sym->style(), sym->brush(), sym->pen(), QSize( use_line_width * 2 + 1, use_line_width * 2 + 1 ) ) );
         }
# endif
#else
         for ( set < long >::iterator it = ggaussian_pts_chi2_marked.begin();
               it != ggaussian_pts_chi2_marked.end();
               it++ ) {
            sym = ggqfit_plot->markerSymbol( *it );
            sym.setSize( use_line_width * 3 + 1 );
            ggqfit_plot->setMarkerSymbol( *it, sym );
         }
         for ( set < long >::iterator it = ggaussian_pts_pfit_marked.begin();
               it != ggaussian_pts_pfit_marked.end();
               it++ ) {
            sym = ggqfit_plot->markerSymbol( *it );
            sym.setSize( use_line_width * 2 + 1 );
            ggqfit_plot->setMarkerSymbol( *it, sym );
         }
#endif
         ggaussian_pts_chi2_marked.clear( );
         ggaussian_pts_pfit_marked.clear( );
      }
      ggqfit_plot->replot();
   }
   ggaussian_enables();
}

void US_Hydrodyn_Mals_Saxs::ggauss_scroll_highlight( int pos )
{
   us_qdebug( QString( "ggauss_scroll_highlight %1 " ).arg( pos ) );
   ggauss_scroll_highlight_last_pos = pos;
   lbl_wheel_pos->setText( "" );
   if ( pos >= (int) ggauss_scroll_set.size()  || pos < 0 ) {
      editor_msg( "red", us_tr( "Internal error ggauss_scroll_highlight: pos >= size or < 0" ) );
      lbl_wheel_pos_below->setText( "" );
      {
         QPixmap pm;
         lbl_wheel_Pcolor->setPixmap( pm );
      }
      disable_updates = true;
      gauss_delete_markers();
      lb_files->clearSelection();
      disable_updates = false;
      suppress_replot = true;
      plot_files();
      gauss_init_markers();
      suppress_replot = false;
      plot_dist->replot();
      return;
   }
      
   QPixmap pm;

#if QT_VERSION < 0x040000
   if ( ggaussian_last_pfit_P[ ggauss_scroll_set [ pos ] ] >= 0.05 ) {
      pm.convertFromImage( 
                          qi_green->smoothScale( 
                                                lbl_wheel_Pcolor->width() / 2
                                                ,lbl_wheel_Pcolor->height() / 2
                                                ,Qt::KeepAspectRatio 
                                                 ) );
   } else {
      if ( ggaussian_last_pfit_P[ ggauss_scroll_set [ pos ] ] >= 0.01 ) {
         pm.convertFromImage( 
                             qi_yellow->smoothScale( 
                                                    lbl_wheel_Pcolor->width() / 2
                                                    ,lbl_wheel_Pcolor->height() / 2
                                                    ,Qt::KeepAspectRatio 
                                                     ) );
      } else {
         pm.convertFromImage( 
                             qi_red->smoothScale( 
                                                 lbl_wheel_Pcolor->width() / 2
                                                 ,lbl_wheel_Pcolor->height() / 2
                                                 ,Qt::KeepAspectRatio 
                                                  ) );
      }
   }
#else
   if ( ggaussian_last_pfit_P[ ggauss_scroll_set [ pos ] ] >= 0.05 ) {
      pm.convertFromImage( 
                          qi_green->scaled( 
                                           lbl_wheel_Pcolor->width() / 2
                                           ,lbl_wheel_Pcolor->height() / 2
                                           ,Qt::KeepAspectRatio
                                           ,Qt::SmoothTransformation

                                            ) );
   } else {
      if ( ggaussian_last_pfit_P[ ggauss_scroll_set [ pos ] ] >= 0.01 ) {
         pm.convertFromImage( 
                             qi_yellow->scaled( 
                                               lbl_wheel_Pcolor->width() / 2
                                               ,lbl_wheel_Pcolor->height() / 2
                                               ,Qt::KeepAspectRatio 
                                               ,Qt::SmoothTransformation
                                                ) );
      } else {
         pm.convertFromImage( 
                             qi_red->scaled( 
                                            lbl_wheel_Pcolor->width() / 2
                                            ,lbl_wheel_Pcolor->height() / 2
                                            ,Qt::KeepAspectRatio 
                                            ,Qt::SmoothTransformation
                                             ) );
      }
   }
#endif
   lbl_wheel_Pcolor->setPixmap( pm );

   lbl_wheel_pos_below->setText( 
                                QString( "%1 %2 %3 P %4%5" )
                                .arg( unified_ggaussian_files[ ggauss_scroll_set [ pos ] ] )
                                .arg( unified_ggaussian_use_errors && cb_sd_weight->isChecked() ? "nChi^2" : "RMSD" )
                                .arg( ggaussian_last_chi2[ ggauss_scroll_set [ pos ] ], 0, 'g', 4 )
                                .arg( ggaussian_last_pfit_P[ ggauss_scroll_set [ pos ] ], 0, 'f', 4 )
                                .arg( f_best_smoothed_smoothing.count( unified_ggaussian_files[ ggauss_scroll_set [ pos ] ] )
                                      ? QString( "\nSmoothing points %1" ).arg( f_best_smoothed_smoothing[ unified_ggaussian_files[ ggauss_scroll_set [ pos ] ] ] )
                                      : QString( "" ) )
                                 );

   disable_updates = true;
   gauss_delete_markers();
   lb_files->clearSelection();
   gauss_delete_markers();
   lb_files->item( ggaussian_selected_file_index[ ggauss_scroll_set [ pos ] ])->setSelected( true );
   lb_files->setCurrentItem( lb_files->item( ggaussian_selected_file_index[ ggauss_scroll_set [ pos ] ] ) );
   lb_files->scrollToItem( lb_files->currentItem() );
   disable_updates = false;
   suppress_replot = true;
   plot_files();
   // if smoothed version present add curve
   
   if ( f_best_smoothed_smoothing.count( unified_ggaussian_files[ ggauss_scroll_set [ pos ] ] ) ) {
      QTextStream( stdout ) << QString( "found smoothed curved for %1\n" ).arg( unified_ggaussian_files[ ggauss_scroll_set [ pos ] ] );

      QPen use_pen = QPen( QColor( "#ffb29b" ), use_line_width + 1, Qt::DashDotDotLine );
      QwtPlotCurve * curve = new QwtPlotCurve( "gg_scroll_gaussian_smoothed" );
      curve->setStyle( QwtPlotCurve::Lines );
      curve->setSamples(
                        (double *)&(f_qs_smoothed[ unified_ggaussian_files[ ggauss_scroll_set [ pos ] ] ][ 0 ]),
                        (double *)&(f_Is_smoothed[ unified_ggaussian_files[ ggauss_scroll_set [ pos ] ] ][ 0 ]),
                        f_qs_smoothed[ unified_ggaussian_files[ ggauss_scroll_set [ pos ] ] ].size()
                        );
      curve->setPen( use_pen );
      curve->attach( plot_dist );
   }      

   // if oldstyle version present and checkbox set, add curve
   if ( cb_ggauss_scroll_oldstyle->isChecked() &&
        f_qs_oldstyle.count( unified_ggaussian_files[ ggauss_scroll_set [ pos ] ] ) ) {
      QTextStream( stdout ) << QString( "found oldstyle curved for %1\n" ).arg( unified_ggaussian_files[ ggauss_scroll_set [ pos ] ] );

      QPen use_pen = QPen( QColor( "#ffffff" ), use_line_width + 1, Qt::DashLine );
      QwtPlotCurve * curve = new QwtPlotCurve( "gg_scroll_gaussian_oldstyle" );
      curve->setStyle( QwtPlotCurve::Lines );
      curve->setSamples(
                        (double *)&(f_qs_oldstyle[ unified_ggaussian_files[ ggauss_scroll_set [ pos ] ] ][ 0 ]),
                        (double *)&(f_Is_oldstyle[ unified_ggaussian_files[ ggauss_scroll_set [ pos ] ] ][ 0 ]),
                        f_qs_oldstyle[ unified_ggaussian_files[ ggauss_scroll_set [ pos ] ] ].size()
                        );
      curve->setPen( use_pen );
      curve->attach( plot_dist );
   }      

   // add gaussian curve ...

   {
      QPen use_pen = QPen( Qt::yellow, use_line_width, Qt::DashDotLine );

#if QT_VERSION >= 0x040000
      QwtPlotCurve * curve = new QwtPlotCurve( "gg_scroll_gaussian" );
      curve->setStyle( QwtPlotCurve::Lines );
      curve->setSamples(
                     (double *)&(ggaussian_last_gg_t[ ggauss_scroll_set [ pos ] ][ 0 ]),
                     (double *)&(ggaussian_last_gg[ ggauss_scroll_set [ pos ] ][ 0 ]),
                     ggaussian_last_gg[ ggauss_scroll_set [ pos ] ].size() );
      curve->setPen( use_pen );
      curve->attach( plot_dist );
#else
      long curve = plot_dist->insertCurve( "gg_scroll_gaussian" );
      plot_dist->setCurveStyle( curve, QwtCurve::Lines );
      plot_dist->setCurveData( curve,
                               (double *)&(ggaussian_last_gg_t[ ggauss_scroll_set [ pos ] ][ 0 ]),
                               (double *)&(ggaussian_last_gg[ ggauss_scroll_set [ pos ] ][ 0 ]),
                               ggaussian_last_gg[ ggauss_scroll_set [ pos ] ].size() );
      plot_dist->setCurvePen( curve, use_pen );
#endif
   }
   
   // and individual gaussians

   {
      QPen use_pen = QPen( Qt::green, use_line_width, Qt::DashDotLine );
      for ( int i = 0; i < (int) ggaussian_last_ggig[ ggauss_scroll_set [ pos ] ].size(); ++i ) {
         
#if QT_VERSION >= 0x040000
         QwtPlotCurve * curve = new QwtPlotCurve( "gg_scroll_gaussian_individual" );
         curve->setStyle( QwtPlotCurve::Lines );
         curve->setSamples(
                        (double *)&(ggaussian_last_gg_t[ ggauss_scroll_set [ pos ] ][ 0 ]),
                        (double *)&(ggaussian_last_ggig[ ggauss_scroll_set [ pos ] ][ i ][ 0 ]),
                        ggaussian_last_gg_t[ ggauss_scroll_set [ pos ] ].size() );
         curve->setPen( use_pen );
         curve->attach( plot_dist );
#else
         long curve = plot_dist->insertCurve( "gg_scroll_gaussian_individual" );
         plot_dist->setCurveStyle( curve, QwtCurve::Lines );
         plot_dist->setCurveData( curve,
                                  (double *)&(ggaussian_last_gg_t[ ggauss_scroll_set [ pos ] ][ 0 ]),
                                  (double *)&(ggaussian_last_ggig[ ggauss_scroll_set [ pos ] ][ i ][ 0 ]),
                                  ggaussian_last_gg_t[ ggauss_scroll_set [ pos ] ].size() );
         plot_dist->setCurvePen( curve, use_pen );
#endif
      }
   }         

   suppress_replot = false;
   gauss_init_markers();
   // #if QT_VERSION >= 0x040000
   //    legend_set();
   // #endif
   //    plot_dist->replot();
    
   // update plot errors
   {
      vector < double > errors;
      bool use_errors = is_nonzero_vector( ggaussian_last_e[ ggauss_scroll_set [ pos ] ] );
      if ( plot_errors->isVisible() ) {
         hide_widgets( plot_errors_widgets, false );
         cb_plot_errors_group->hide();
         if ( !use_errors ) {
            disconnect( cb_plot_errors_sd, SIGNAL( clicked() ), 0, 0 );
            cb_plot_errors_sd->setChecked( false );
            connect( cb_plot_errors_sd, SIGNAL( clicked() ), SLOT( set_plot_errors_sd() ) );
            cb_plot_errors_sd->hide();
         }
      }
         
      // if ( plot_errors_zoomer )
      // {
      //    delete plot_errors_zoomer;
      //    plot_errors_zoomer = (ScrollZoomer *) 0;
      // }
      
      update_plot_errors(
                         ggaussian_last_gg_t[ ggauss_scroll_set [ pos ] ],
                         ggaussian_last_I[ ggauss_scroll_set [ pos ] ],
                         ggaussian_last_gg[ ggauss_scroll_set [ pos ] ],
                         use_errors ? ggaussian_last_e[ ggauss_scroll_set [ pos ] ] : errors,
                         plot_colors[ ggaussian_selected_file_index[ ggauss_scroll_set [ pos ] ] % plot_colors.size() ]
                         );
      // redo_plot_errors();
   }

   // clear highlighted ggqfit_plot for chi2 and pfit
   {
#if QT_VERSION < 0x050000
         QwtSymbol sym;
#else
         const QwtSymbol * sym;
#endif
      // clear any highlighted symbols
#if QT_VERSION >= 0x040000
# if QT_VERSION < 0x050000
      for ( set < QwtPlotMarker * >::iterator it = ggaussian_pts_chi2_marked.begin();
            it != ggaussian_pts_chi2_marked.end();
            it++ ) {
         sym = (*it)->symbol();
         sym.setSize( use_line_width * 3 + 1 );
         (*it)->setSymbol( new QwtSymbol( sym.style(), sym.brush(), sym.pen(), sym.size() ) );
      }
      for ( set < QwtPlotMarker * >::iterator it = ggaussian_pts_pfit_marked.begin();
            it != ggaussian_pts_pfit_marked.end();
            it++ ) {
         sym = (*it)->symbol();
         sym.setSize( use_line_width * 2 + 1 );
         (*it)->setSymbol( new QwtSymbol( sym.style(), sym.brush(), sym.pen(), sym.size() ) );
      }
# else
      for ( set < QwtPlotMarker * >::iterator it = ggaussian_pts_chi2_marked.begin();
            it != ggaussian_pts_chi2_marked.end();
            it++ ) {
         sym = (*it)->symbol();
         (*it)->setSymbol( new QwtSymbol( sym->style(), sym->brush(), sym->pen(), QSize( use_line_width * 3 + 1, use_line_width * 3 + 1 ) ) );
      }
      for ( set < QwtPlotMarker * >::iterator it = ggaussian_pts_pfit_marked.begin();
            it != ggaussian_pts_pfit_marked.end();
            it++ ) {
         sym = (*it)->symbol();
         (*it)->setSymbol( new QwtSymbol( sym->style(), sym->brush(), sym->pen(), QSize( use_line_width * 2 + 1, use_line_width * 2 + 1 ) ) );
      }
# endif
#else
      for ( set < long >::iterator it = ggaussian_pts_chi2_marked.begin();
            it != ggaussian_pts_chi2_marked.end();
            it++ ) {
         sym = ggqfit_plot->markerSymbol( *it );
         sym.setSize( use_line_width * 3 + 1 );
         ggqfit_plot->setMarkerSymbol( *it, sym );
      }
      for ( set < long >::iterator it = ggaussian_pts_pfit_marked.begin();
            it != ggaussian_pts_pfit_marked.end();
            it++ ) {
         sym = ggqfit_plot->markerSymbol( *it );
         sym.setSize( use_line_width * 2 + 1 );
         ggqfit_plot->setMarkerSymbol( *it, sym );
      }
#endif
      ggaussian_pts_chi2_marked.clear( );
      ggaussian_pts_pfit_marked.clear( );
   }

   // update ggqfit_plot for chi2 and pfit
   {
      // highlight selected

#if QT_VERSION < 0x050000
      QwtSymbol sym;
#else
      const QwtSymbol * sym;
#endif
#if QT_VERSION >= 0x040000
# if QT_VERSION < 0x050000
      if ( ggaussian_pts_chi2.size() ) {
         QwtPlotMarker * marker = ggaussian_pts_chi2[ ggauss_scroll_set [ pos ] ];
         sym = marker->symbol();
         sym.setSize( use_line_width * 7 + 1 );
         marker->setSymbol( new QwtSymbol( sym.style(), sym.brush(), sym.pen(), sym.size() ) );
         ggaussian_pts_chi2_marked.insert( marker );
      }

      if ( ggaussian_pts_pfit.size() ) {
         QwtPlotMarker * marker = ggaussian_pts_pfit[ ggauss_scroll_set [ pos ] ];
         sym = marker->symbol();
         sym.setSize( use_line_width * 6 + 1 );
         marker->setSymbol( new QwtSymbol( sym.style(), sym.brush(), sym.pen(), sym.size() ) );
         ggaussian_pts_pfit_marked.insert( marker );
      }
# else
      if ( ggaussian_pts_chi2.size() ) {
         QwtPlotMarker * marker = ggaussian_pts_chi2[ ggauss_scroll_set [ pos ] ];
         sym = marker->symbol();
         marker->setSymbol( new QwtSymbol( sym->style(), sym->brush(), sym->pen(), QSize( use_line_width * 7 + 1, use_line_width * 7 + 1 ) ) );
         ggaussian_pts_chi2_marked.insert( marker );
      }

      if ( ggaussian_pts_pfit.size() ) {
         QwtPlotMarker * marker = ggaussian_pts_pfit[ ggauss_scroll_set [ pos ] ];
         sym = marker->symbol();
         marker->setSymbol( new QwtSymbol( sym->style(), sym->brush(), sym->pen(), QSize( use_line_width * 6 + 1, use_line_width * 6 + 1 ) ) );
         ggaussian_pts_pfit_marked.insert( marker );
      }
# endif
#else
      if ( ggaussian_pts_chi2.size() ) {
         long marker = ggaussian_pts_chi2[ ggauss_scroll_set [ pos ] ];
         sym = ggqfit_plot->markerSymbol( marker );
         sym.setSize( use_line_width * 7 + 1 );
         ggqfit_plot->setMarkerSymbol( marker, sym );
         ggaussian_pts_chi2_marked.insert( marker );
      }

      if ( ggaussian_pts_pfit.size() ) {
         long marker = ggaussian_pts_pfit[ ggauss_scroll_set [ pos ] ];
         sym = ggqfit_plot->markerSymbol( marker );
         sym.setSize( use_line_width * 6 + 1 );
         ggqfit_plot->setMarkerSymbol( marker, sym );
         ggaussian_pts_pfit_marked.insert( marker );
      }
#endif
      ggqfit_plot->replot();
   }               
}

void US_Hydrodyn_Mals_Saxs::ggaussian_enables()
{
   if ( !f_best_smoothed_smoothing.size() ) {
      cb_ggauss_scroll_smoothed->hide();
      cb_ggauss_scroll_smoothed->setChecked( false );
   } else {
      cb_ggauss_scroll_smoothed->show();
   }

   if ( !f_qs_oldstyle.size() ) {
      cb_ggauss_scroll_oldstyle->hide();
      cb_ggauss_scroll_oldstyle->setChecked( false );
   } else {
      cb_ggauss_scroll_oldstyle->show();
   }
   
   if ( cb_ggauss_scroll->isChecked() ) {
      disable_all();
      wheel_enables       ( ggauss_scroll_set.size() );
      cb_eb               ->setEnabled( true );
      pb_line_width       ->setEnabled( true );
      pb_color_rotate     ->setEnabled( true );
      ShowHide::hide_widgets( wheel_below_widgets, always_hide_widgets, false );
   } else {
      cb_eb               ->setEnabled( false );
      pb_line_width       ->setEnabled( false );
      pb_color_rotate     ->setEnabled( false );
      pb_gauss_prev       ->setEnabled( unified_ggaussian_gaussians_size > 1 && gaussian_pos > 0 );
      pb_gauss_next       ->setEnabled( unified_ggaussian_gaussians_size > 1 && gaussian_pos < unified_ggaussian_gaussians_size - 1 );
      cb_sd_weight        ->setEnabled( unified_ggaussian_gaussians_size && le_gauss_fit_start->text().toDouble() < le_gauss_fit_end->text().toDouble() );
      pb_gauss_fit        ->setEnabled( unified_ggaussian_gaussians_size && le_gauss_fit_start->text().toDouble() < le_gauss_fit_end->text().toDouble() );
      le_gauss_pos        ->setEnabled( unified_ggaussian_gaussians_size && gaussian_pos < unified_ggaussian_gaussians_size );
      le_gauss_pos_width  ->setEnabled( cb_fix_width->isChecked() && unified_ggaussian_gaussians_size && gaussian_pos < unified_ggaussian_gaussians_size );
      le_gauss_pos_dist1  ->setEnabled( cb_fix_dist1->isChecked() && unified_ggaussian_gaussians_size && gaussian_pos < unified_ggaussian_gaussians_size );
      le_gauss_pos_dist2  ->setEnabled( cb_fix_dist2->isChecked() && unified_ggaussian_gaussians_size && gaussian_pos < unified_ggaussian_gaussians_size );
      le_gauss_fit_start  ->setEnabled( unified_ggaussian_gaussians_size && gaussian_pos < unified_ggaussian_gaussians_size );
      le_gauss_fit_end    ->setEnabled( unified_ggaussian_gaussians_size && gaussian_pos < unified_ggaussian_gaussians_size );
      wheel_enables       ( unified_ggaussian_gaussians_size && gaussian_pos < unified_ggaussian_gaussians_size );
      pb_ggauss_results   ->setEnabled( unified_ggaussian_ok );
      pb_gauss_save       ->setEnabled( unified_ggaussian_ok );
      pb_ggauss_as_curves ->setEnabled( unified_ggaussian_ok );
      pb_view             ->setEnabled( unified_ggaussian_curves <= 10 );
      pb_cormap           ->setEnabled( unified_ggaussian_ok );
      ShowHide::hide_widgets( wheel_below_widgets, always_hide_widgets );
      // if ( le_last_focus && qwtw_wheel->isEnabled() ) {
      //    le_last_focus->setFocus();
      // }
   }

   pb_ggauss_start     ->setEnabled( false );
   pb_wheel_cancel     ->setEnabled( true );
   pb_rescale          ->setEnabled( true );
   pb_rescale_y        ->setEnabled( true );
   pb_wheel_save       ->setEnabled( unified_ggaussian_gaussians_size );
   pb_errors           ->setEnabled( true );
   pb_ggqfit           ->setEnabled( true );
   cb_ggq_plot_chi2    ->setEnabled( true );
   cb_ggq_plot_P       ->setEnabled( true );
   pb_pp               ->setEnabled( true );
}

// --- timeshift ---

void US_Hydrodyn_Mals_Saxs::timeshift()
{
   wheel_file = select_conc_file( "Timeshift" );
   if ( wheel_file.isEmpty() )
   {
      update_enables();
      return;
   }

   {
      set < QString > set_to_select = all_selected_files_set();
      set_to_select.insert( wheel_file );
      set_selected( set_to_select );
   }

   QStringList selected_files;
   lbl_wheel_pos->setText( QString( "%1" ).arg( 0 ) );

   selected_files = all_selected_files();

   // bool ok;

   // wheel_file = US_Static::getItem(
   //                                    us_tr( "SOMO: HPLC timeshift: select file" ),
   //                                    us_tr("Select the curve to timeshift:\n" ),
   //                                    selected_files, 
   //                                    0, 
   //                                    false, 
   //                                    &ok,
   //                                    this );
   // if ( !ok ) {
   //    return;
   // }


   // gaussian_mode  = false;
   // ggaussian_mode = false;
   // baseline_mode  = false;

   if ( !plotted_curves.count( wheel_file ) )
   {
      editor_msg( "red", QString( us_tr( "Internal error: request to timeshift %1, but not found in data" ) ).arg( wheel_file ) );
      return;
   }

   wheel_curve           = plotted_curves[ wheel_file ];

   running               = true;

   disable_all();
   // timeshift_mode = true;
   mode_select( MODE_TIMESHIFT );

   plot_errors->detachItems( QwtPlotItem::Rtti_PlotCurve ); plot_errors->detachItems( QwtPlotItem::Rtti_PlotMarker );;
   
   pb_rescale            ->setEnabled( true );
   pb_rescale_y          ->setEnabled( true );
   pb_timeshift          ->setEnabled( false );
   pb_wheel_cancel       ->setEnabled( true );
   wheel_enables();
   // compute a good range
   {
      // double minq_conc = f_qs[ wheel_file ][ 0 ];
      // double maxq_conc = f_qs[ wheel_file ].back();
      // double range_conc = maxq_conc - minq_conc;

      double minq_ref  = f_qs[ selected_files[ 0 ] ][ 0 ];
      double maxq_ref  = f_qs[ selected_files[ 0 ] ].back();
      double range_ref  = maxq_ref  - minq_ref;

      // double minq_w    = minq_ref - maxq_conc;
      // double maxq_w    = maxq_ref - minq_conc;
      // qwtw_wheel->setRange( minq_w, maxq_w); qwtw_wheel->setSingleStep( (maxq_w - minq_w ) / UHSH_WHEEL_RES );
      // qwtw_wheel            ->setValue  ( ( minq_w + maxq_w ) * 5e-1 );

      double peak;
      double pos_ref;
      double pos_conc;

      get_peak( wheel_file, peak, pos_conc );
      get_peak( selected_files[ 0 ], peak, pos_ref );
      double center = pos_ref - pos_conc;

      double minq_w = center - .25 * range_ref;
      double maxq_w = center + .25 * range_ref;

      qwtw_wheel->setRange( minq_w, maxq_w); qwtw_wheel->setSingleStep( (maxq_w - minq_w ) / UHSH_WHEEL_RES );
      qwtw_wheel            ->setValue  ( center );
   }
}

void US_Hydrodyn_Mals_Saxs::timescale()
{
   timescale( all_selected_files() );
}

void US_Hydrodyn_Mals_Saxs::timescale( const QStringList & files )
{
   bool ok;
   double res = US_Static::getDouble(
                                        us_tr( "US-SOMO: MALS_SAXS: Time scale" ),
                                        us_tr( "Enter a multiplier to scale the time" )
                                        ,
                                        1e0,
                                        1e-5,
                                        1e7,
                                        6,
                                        &ok,
                                        this
                                        );
   if ( !ok ) {
      // user pressed Cancel
      update_enables();
      return;
   }

   set < QString > existing_created;
   for ( int i = 0; i < lb_created_files->count(); ++i )
   {
      existing_created.insert( lb_created_files->item( i )->text() );
   }

   for ( int i = 0; i < (int) files.size(); ++i )
   {
      for ( int j = 0; j < (int) f_qs[ files[ i ] ].size(); ++j )
      {
         f_qs       [ files[ i ] ][ j ] *= res;
         f_qs_string[ files[ i ] ][ j ] = QString( "%1" ).arg( f_qs[ files[ i ] ][ j ] );
      }
      if ( !existing_created.count( files[ i ] ) )
      {
         lb_created_files->addItem( files[ i ] );
         lb_created_files->scrollToItem( lb_created_files->item( lb_created_files->count() - 1 ) );
      }
      created_files_not_saved[ files[ i ] ] = true;
   }

   rescale();
   //   plot_files();
   update_enables();
}

void US_Hydrodyn_Mals_Saxs::ggqfit()
{
   ShowHide::hide_widgets( ggqfit_widgets, always_hide_widgets, ggqfit_widgets[ 0 ]->isVisible() );
}
