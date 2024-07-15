#include "../include/us_hydrodyn.h"
#include "../include/us_hydrodyn_mals_saxs.h"
#include "../include/us_hydrodyn_mals_saxs_baseline_best.h"
//Added by qt3to4:
#include <QPixmap>

// --- blanks ---
#define IB_END_OFS       5
#define IB_MIN_RANGE     10
#define IB_DEFAULT_RANGE 20
#define IB_MIN_FRAMES    20

void US_Hydrodyn_Mals_Saxs::blanks_start()
{
   // like baseline except without middle line and simulated i(t)...
   org_baseline_end_s       = le_baseline_end_s      ->text().toDouble();
   org_baseline_end_e       = le_baseline_end_e      ->text().toDouble();

   blanks_in_baseline_mode = current_mode == MODE_BASELINE;

   blanks_save_selected     = all_selected_files();
   blanks_save_selected_set = all_selected_files_set();

   blanks_last_cormap_parameters         .clear( );
   blanks_last_cormap_pvaluepairs        .clear( );
   blanks_last_brookesmap_sliding_results.clear( );

   blanks_last_cormap_run_end_s = -M_PI;
   blanks_last_cormap_run_end_e = -M_PI;

   blanks_temporary_load_set.clear( );
   bool use_default_blanks = false;
   if ( default_blanks.size() &&
        blanks_save_selected_set != default_blanks_set // not same files!
        ) {
      // either use (load) these if selected are not also all !time or ask
      use_default_blanks = true;
      if ( blanks_save_selected.size() > 1 ) {
         bool files_compatible = compatible_files( blanks_save_selected );
         bool files_are_time   = type_files      ( blanks_save_selected );
         if ( !files_are_time && files_compatible ) {
            switch ( QMessageBox::question(this, 
                                           windowTitle() + us_tr( " : Blanks analysis" )
                                           ,us_tr( 
                                               "There are available blanks data and selected files that could be blanks\n"
                                               "Which would you like to analyze?"
                                                )
                                           ,us_tr( us_tr( "&Available blanks data" ) )
                                           ,us_tr( us_tr( "Currently &selected files" ) )
                                           ,us_tr( "&Cancel" )
                                           ,0 // 
                                           ,-1 // 
                                           ) )
            {
            case -1 : // escape
               blanks_in_baseline_mode ? baseline_enables() : update_enables();
               return;
               break;
            case 0 : // available blanks data
               break;
            case 1 : // currently selected files
               use_default_blanks = false;
               break;
            case 2 : // cancel
               blanks_in_baseline_mode ? baseline_enables() : update_enables();
               return;
               break;
            }
         }
      }         
   }

   if ( use_default_blanks ) {
      // us_qdebug( "use default blanks" );
      for ( int i = 0; i < (int) default_blanks.size(); ++i ) {
         if ( !f_qs.count( default_blanks[ i ] ) ) {
            // us_qdebug( QString( "loading %1" ).arg( default_blanks_files[ i ] ) );
            if ( !load_file( default_blanks_files[ i ] ) ) {
               editor_msg( "red", QString( us_tr( "Error: could not load blank file %1" ) ).arg( default_blanks_files[ i ] ) );
               remove_files( blanks_temporary_load_set );
               blanks_in_baseline_mode ? baseline_enables() : update_enables();
               return;
            } else {
               blanks_temporary_load_set.insert( default_blanks[ i ] );
               lb_files->addItem( default_blanks[ i ] );
            }
         }
      }
      // now all blanks loaded
      set_selected( default_blanks_set );
   } else {
      // us_qdebug( "Not use default blanks" );
      if ( blanks_in_baseline_mode ) {
         return baseline_start();
         // QMessageBox::warning( this, windowTitle() + us_tr( " : Blanks analysis" ),
         //                       us_tr( "You would now be asked to load files, but this is not yet implemented\n"
         //                           "You *can* exit and load blanks, run blanks analysis & 'keep' and then try again\n" )
         //                       ,QMessageBox::Ok | QMessageBox::Default
         //                       ,QMessageBox::NoButton
         //                       );
         // baseline_enables();
      }
   }
 
   blanks_selected     = all_selected_files();
   blanks_selected_set = all_selected_files_set();
   blanks_created.clear( );
   blanks_created_q.clear( );
   blanks_created_set.clear( );

   if ( blanks_selected.size() < IB_MIN_FRAMES ) {
      QMessageBox::warning( this, windowTitle() + us_tr( " : Blanks analysis" ),
                            QString( us_tr( "You must have minimum of %1 frames for blanks" ) ).arg( IB_MIN_FRAMES )
                            ,QMessageBox::Ok | QMessageBox::Default
                            ,QMessageBox::NoButton
                            );
      // return current_mode != MODE_NORMAL ? wheel_cancel() : update_enables();
      if ( current_mode != MODE_NORMAL ) {
         current_mode = MODE_BLANKS;
         blanks_axis_y_was_log = false;
         return wheel_cancel();
      }
      return update_enables();
      // return blanks_in_baseline_mode ? baseline_enables() : update_enables();
   }
      

   {
      vector < vector < double > > grids;

      for ( int i = 0; i < (int) blanks_selected.size(); ++i )
      {
         QString this_file = blanks_selected[ i ];
         if ( f_qs.count( this_file ) &&
              f_Is.count( this_file ) &&
              f_qs[ this_file ].size() &&
              f_Is[ this_file ].size() )
         {
            grids.push_back( f_qs[ this_file ] );
         }
      }

      vector < double > v_union = US_Vector::vunion( grids );
      vector < double > v_int   = US_Vector::intersection( grids );

      bool any_differences = v_union != v_int;
      
      if ( any_differences )
      {
         editor_msg( "red", us_tr( "Blanks analysis: curves must be on the same grid, try 'Crop Common' first." ) );
         update_enables();
         return;
      }
   }
   
   // make some i(t)'s

   {
      // find common q 
      QStringList files = blanks_selected;

      QString head = qstring_common_head( files, true );
      QString tail = qstring_common_tail( files, true );

      // map: [ timestamp ][ q value ] = intensity

      map < double, map < double , double > > I_values;
      map < double, map < double , double > > e_values;

      map < double, bool > used_q;
      list < double >      ql;

      QRegExp rx_cap( "(\\d+)_(\\d+)" );
      QRegExp rx_clear_nonnumeric( "^(\\d*_?\\d+)([^0-9_]|_[a-zA-Z])" );

      {
         QString     * qs;
         double      * q;
         double      * I;
         double      * e;
         unsigned int size;
         double        qv;

         map < double , double > * Ivp;
         map < double , double > * evp;

         for ( unsigned int i = 0; i < ( unsigned int ) files.size(); ++i )
         {
      
            qs = &( files[ i ] );

            if ( !f_qs.count( *qs ) )
            {
               editor_msg( "red", QString( us_tr( "Internal error: request to use %1, but not found in data" ) ).arg( *qs ) );
            } else {
               QString tmp = qs->mid( head.length() );
               tmp = tmp.mid( 0, tmp.length() - tail.length() );
               if ( rx_clear_nonnumeric.indexIn( tmp ) != -1 )
               {
                  tmp = rx_clear_nonnumeric.cap( 1 );
               }

               if ( rx_cap.indexIn( tmp ) != -1 )
               {
                  tmp = rx_cap.cap( 1 ) + "." + rx_cap.cap( 2 );
               }
               double timestamp = tmp.toDouble();
#ifdef DEBUG_LOAD_REORDER
               us_qdebug( QString( "%1 tmp %2 value %3" ).arg( *qs ).arg( tmp ).arg( timestamp ) );
#endif

               Ivp = &(I_values[ timestamp ]);
               evp = &(e_values[ timestamp ]);

               q = &(f_qs[ *qs ][ 0 ]);
               I = &(f_Is[ *qs ][ 0 ]);
               size =  ( unsigned int ) f_qs[ *qs ].size();

               if ( f_errors[ *qs ].size() )
               {
                  e = &(f_errors[ *qs ][ 0 ]);
                  for ( unsigned int j = 0; j < size; j++ )
                  {
                     qv = q[ j ];
                     (*Ivp)[ qv ] = I[ j ];
                     (*evp)[ qv ] = e[ j ];
                     if ( !used_q.count( qv ) )
                     {
                        ql.push_back( qv );
                        used_q[ qv ] = true;
                     }
                  }
               } else {
                  for ( unsigned int j = 0; j < size; j++ )
                  {
                     qv = q[ j ];
                     (*Ivp)[ qv ] = I[ j ];
                     if ( !used_q.count( qv ) )
                     {
                        ql.push_back( qv );
                        used_q[ qv ] = true;
                     }
                  }
               }
            }
         }
      }

      ql.sort();

      vector < double > q;
      vector < QString > q_qs;
      for ( list < double >::iterator it = ql.begin();
            it != ql.end();
            it++ )
      {
         q.push_back( *it );
      }

      map < QString, bool > current_files;
      for ( int i = 0; i < lb_files->count(); ++i )
      {
         current_files[ lb_files->item( i )->text() ] = true;
      }
      QStringList created_files;
      set < QString > created_files_set;
   
      {
         unsigned int size = ( unsigned int )q.size();

         for ( unsigned int i = 0; i < size; ++i )
         {
            double qv = q[ i ];
            QString basename = QString( "%1_It_q%2" ).arg( head ).arg( qv );
            basename.replace( ".", "_" );
      
            unsigned int ext = 0;
            QString fname = basename + tail;
            while ( current_files.count( fname ) )
            {
               fname = basename + QString( "-%1" ).arg( ++ext ) + tail;
            }
            // editor_msg( "gray", fname );

            vector < double  > t;
            vector < QString > t_qs;
            vector < double  > I;
            vector < double  > e;

            for ( map < double, map < double , double > >::iterator it = I_values.begin();
                  it != I_values.end();
                  it++ )
            {
               t   .push_back( it->first );
               t_qs.push_back( QString( "" ).sprintf( "%.8f", it->first ) );
               if ( it->second.count( qv ) )
               {
                  I.push_back( it->second[ qv ] );
               } else {
                  I.push_back( 0e0 );
               }
               if ( e_values.count( it->first ) &&
                    e_values[ it->first ].count( qv ) )
               {
                  e.push_back( e_values[ it->first ][ qv ] );
               } else {
                  e.push_back( 0e0 );
               }
            }

            // created_files_not_saved[ fname ] = true;
            blanks_created  .push_back( fname );
            blanks_created_q.push_back( qv );
   
            f_pos       [ fname ] = f_qs.size();
            f_qs_string [ fname ] = t_qs;
            f_qs        [ fname ] = t;
            f_Is        [ fname ] = I;
            f_errors    [ fname ] = e;
            f_is_time   [ fname ] = true;
            f_conc      [ fname ] = 0e0;
            f_psv       [ fname ] = 0e0;
            f_I0se      [ fname ] = 0e0;
            {
               vector < double > tmp;
               f_gaussians  [ fname ] = tmp;
            }
            created_files << fname;
            blanks_created_set.insert( fname );
            
         }      
         lb_created_files->addItems( created_files );
         lb_files->addItems( created_files );
         lb_created_files->scrollToItem( lb_created_files->item( lb_created_files->count() - 1 ) );
         lb_files->scrollToItem( lb_files->item( lb_files->count() - 1 ) );
      }

      // now select created files
      blanks_axis_y_was_log = axis_y_log;
      if ( axis_y_log )
      {
         axis_y();
      }
      set_selected( blanks_created_set );
      rescale();
   }

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
      editor_msg( "red", QString( us_tr( "Internal error (bm): %1 not found in data" ) ).arg( wheel_file ) );
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

   int q_size = (int) f_qs[ wheel_file ].size();

   wheel_double_to_pos.clear( );
   for ( int i = 0; i < q_size; ++i ) {
      wheel_double_to_pos[ f_qs[ wheel_file ][ i ] ] = i;
   }

   disconnect( le_baseline_end_s, SIGNAL( textChanged( const QString & ) ), 0, 0 );
   le_baseline_end_s->setText( QString( "%1" ).arg( blanks_end_s ) );
   connect( le_baseline_end_s, SIGNAL( textChanged( const QString & ) ), SLOT( baseline_end_s_text( const QString & ) ) );

   disconnect( le_baseline_end_e, SIGNAL( textChanged( const QString & ) ), 0, 0 );
   le_baseline_end_e->setText( QString( "%1" ).arg( blanks_end_e ) );
   connect( le_baseline_end_e, SIGNAL( textChanged( const QString & ) ), SLOT( baseline_end_e_text( const QString & ) ) );

   bool reset_range = f_qs[ wheel_file ] != blanks_last_q_range;

   if ( reset_range ||
        le_baseline_end_s->text().isEmpty() ||
        le_baseline_end_s->text() == "0" ||
        le_baseline_end_s->text().toDouble() > f_qs[ wheel_file ].back() ||
        le_baseline_end_s->text().toDouble() < f_qs[ wheel_file ].front() )
   {
      disconnect( le_baseline_end_s, SIGNAL( textChanged( const QString & ) ), 0, 0 );
      le_baseline_end_s->setText( QString( "%1" ).arg( f_qs[ wheel_file ].front()) );
      connect( le_baseline_end_s, SIGNAL( textChanged( const QString & ) ), SLOT( baseline_end_s_text( const QString & ) ) );
   }

   if ( reset_range ||
        le_baseline_end_e->text().isEmpty() ||
        le_baseline_end_e->text() == "0" ||
        le_baseline_end_e->text().toDouble() < f_qs[ wheel_file ].front() ||
        le_baseline_end_e->text().toDouble() > f_qs[ wheel_file ].back() )
   {
      disconnect( le_baseline_end_e, SIGNAL( textChanged( const QString & ) ), 0, 0 );
      le_baseline_end_e->setText( QString( "%1" ).arg( f_qs[ wheel_file ].back() ) );
      connect( le_baseline_end_e, SIGNAL( textChanged( const QString & ) ), SLOT( baseline_end_e_text( const QString & ) ) );
   }

   mode_select( MODE_BLANKS );
   baseline_init_markers();
   // we still need to make i(t)... likely data for this also
   // replot_baseline();
   disable_all();
   blanks_enables();
}

void US_Hydrodyn_Mals_Saxs::blanks_enables()
{
   pb_bb_cm_inc           ->setEnabled( true );
   pb_blanks_params       ->setEnabled( true );
   pb_cormap              ->setEnabled( true );
   pb_baseline_start      ->setEnabled( false );
   pb_wheel_cancel        ->setEnabled( true );
   pb_wheel_save          ->setEnabled( 
                                       blanks_last_cormap_parameters.size() &&
                                       blanks_last_cormap_pvaluepairs.size() >= IB_MIN_FRAMES &&
                                       le_baseline_end_s->text().toDouble() == blanks_last_cormap_run_end_s &&
                                       le_baseline_end_e->text().toDouble() == blanks_last_cormap_run_end_e 
                                       // le_baseline_end_s->text().toDouble() != org_baseline_end_s ||
                                       // le_baseline_end_e->text().toDouble() != org_baseline_end_e 
                                        );
   le_baseline_end_s      ->setEnabled( true );
   le_baseline_end_e      ->setEnabled( true );

   wheel_enables(
#if QT_VERSION > 0x050000
                 le_last_focus == le_baseline_end_s ||
                 le_last_focus == le_baseline_end_e
#else
                 le_baseline_end_s  ->hasFocus() ||
                 le_baseline_end_e  ->hasFocus()
#endif
                 );
   pb_rescale             ->setEnabled( true );
   pb_rescale_y           ->setEnabled( true );
   pb_view                ->setEnabled( true );
}

// --- baseline ---
void US_Hydrodyn_Mals_Saxs::baseline_start( bool from_blanks_mode_save )
{
   if ( axis_y_log )
   {
      axis_y();
   }

   if ( !from_blanks_mode_save ) {
      cb_baseline_fix_width->setChecked( false );
   }

   {
      bool use_i_power = 
         ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "mals_saxs_bl_i_power" ) &&
         ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_saxs_bl_i_power" ].toDouble() != 1e0;

      double i_power = 1e0;

      if ( use_i_power ) {
         i_power = ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_saxs_bl_i_power" ].toDouble();
         switch ( QMessageBox::warning(this, 
                                       windowTitle() + us_tr( " : Baseline" ),
                                       QString( us_tr( "Please note:\n\n"
                                                    "You are using a non-unity intensity exponent of %1 for integral baseline correction.\n"
                                                    "The model of intensity proportional deposits is based on a unity intensity exponent.\n"
                                                    "It is possible that a specific capillary fouling mechanism may be correctly approximated with a non-unity exponent.\n"
                                                    "We urge you to use a non-unity intensity exponent with care.\n"
                                                    "What would you like to do?\n" ) )
                                       .arg( i_power ),
                                       us_tr( "&Continue" ), 
                                       us_tr( "&Quit" )
                                       ) )
         {
         case 0 : // continue
            break;
         case 1 : // quit
            return;
            break;
         }
      }
   }

   {
      int reps = ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_saxs_bl_reps" ].toInt();
      if ( reps < 5 )
      {
         switch ( QMessageBox::warning(this, 
                                       windowTitle() + us_tr( " : Baseline" ),
                                       QString( us_tr( "Please note:\n\n"
                                                    "You have selected the integral baseline method with maximum iterations of %1\n"
                                                    "It is recommended that a minimum of 5 iterations be used to approach convergence\n"
                                                    "What would you like to do?\n" ) )
                                       .arg( reps ),
                                       us_tr( "&Continue" ), 
                                       us_tr( "&Quit" )
                                       ) )
         {
         case 0 : // continue
            break;
         case 1 : // quit
            return;
            break;
         }
      }
   }

   org_baseline_start_s     = le_baseline_start_s    ->text().toDouble();
   org_baseline_start       = le_baseline_start      ->text().toDouble();
   org_baseline_start_e     = le_baseline_start_e    ->text().toDouble();
   org_baseline_end_s       = le_baseline_end_s      ->text().toDouble();
   org_baseline_end         = le_baseline_end        ->text().toDouble();
   org_baseline_end_e       = le_baseline_end_e      ->text().toDouble();
   org_baseline_start_zero  = cb_baseline_start_zero ->isChecked();

   baseline_best_ok               = false;
   baseline_best_last_window_size = -1;

   baseline_test_mode             = false;

   baseline_selected     = all_selected_files();
   baseline_selected_set = all_selected_files_set();
   baseline_multi    = baseline_selected.size() > 1;
   baseline_integral = ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_saxs_bl_integral" ] == "true";
   baseline_ready_to_apply = false;

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

   if ( f_qs[ wheel_file ].size() < IB_MIN_RANGE * 2 ) {
      QMessageBox::warning(this
                           ,windowTitle() + us_tr( " : Baseline" )
                           ,QString( us_tr( "You must have a minimum of %1 frames in your data to perform baseline correction\n" ) )
                           .arg( IB_MIN_RANGE * 2 )
                           ,QMessageBox::Ok | QMessageBox::Default
                           ,QMessageBox::NoButton
                           );
      return;
   }


#if QT_VERSION < 0x040000
   plot_dist->setCurvePen( plotted_curves[ wheel_file ], QPen( Qt::cyan, use_line_width, SolidLine));
#else
   plotted_curves[ wheel_file ]->setPen( QPen( Qt::cyan, use_line_width, Qt::SolidLine ) );
#endif

   double default_baseline_end_s = f_qs[ wheel_file ].back();
   double default_baseline_end   = f_qs[ wheel_file ].back();
   double default_baseline_end_e = f_qs[ wheel_file ].back();

   int q_size = (int) f_qs[ wheel_file ].size();

   wheel_double_to_pos.clear( );
   for ( int i = 0; i < q_size; ++i ) {
      wheel_double_to_pos[ f_qs[ wheel_file ][ i ] ] = i;
   }

   baseline_max_window_size = (int) f_qs[ wheel_file ].size();

   bool do_blanks_start         = false;
   bool force_endpoints_default = false;

   baseline_hb_only_mode = false;

   if ( baseline_integral ) {
      bool show_determination_info = true;
      if ( q_size < IB_MIN_FRAMES ) {
         QMessageBox::warning( this
                               ,windowTitle() + us_tr( " : Baseline" )
                               ,us_tr( QString( "Insufficient frames (%1) for integral baseline analysis.\n"
                                             "A minimum of %2 frames are required" )
                                    .arg( q_size )
                                    .arg( IB_MIN_FRAMES ) )
                               ,QMessageBox::Ok | QMessageBox::Default
                               ,QMessageBox::NoButton );
         return current_mode != MODE_NORMAL ? wheel_cancel() : update_enables();
      }
      if ( !from_blanks_mode_save || !blanks_brookesmap_sliding_results.size() ) {
         switch ( QMessageBox::question( this
                                         ,windowTitle() + us_tr( " : Utilize blanks in baseline analysis" )
                                         ,us_tr( "Do you wish to utilize reference blanks for baseline analysis?\n"
                                              "Without reference blanks, only the adjusted Holm-Bonferroni method can be used." )
                                         ,us_tr( "&Yes, utilize blanks" )
                                         ,us_tr( "&No, proceed without blanks" )
                                         ,QString()
                                         ) ) {
         case -1 : // escape
            return current_mode != MODE_NORMAL ? wheel_cancel() : update_enables();
            break;
         case 0 : 
            {
               switch ( QMessageBox::question( this
                                               ,windowTitle() + us_tr( " : Baseline mode blanks selection" )
                                               ,QString( us_tr( "You must have blanks processed to proceed\n"
                                                             "\n"
                                                             "The blanks should be non-averaged buffer frames (whose average value was then subtracted from the elution frames)\n"
                                                             "If you do not have the buffers corresponding to their present elution, then the preloaded blanks should have been chosen in a similar way:\n"
                                                             "The blanks do not have to be *the* blanks used for the present elution, even if it is strongly recommended,\n"
                                                             "but at least blanks collected on the same beamline with a similar set-up (distance, pixel size, energy).\n"
                                                             "\n"
                                                             "You have the following options" ) )
                                               ,us_tr( "Load blanks" )
                                               ,us_tr( "Load previous analysis" )
                                               ,( blanks_brookesmap_sliding_results.size() &&
                                                  blanks_cormap_pvaluepairs.size() >= IB_MIN_FRAMES  ) ?
                                               // maybe should be qstring with currently analyzed name
                                               QString( us_tr( "Accept currently analyzed blanks%1" ) )
                                               .arg( blanks_cormap_parameters.count( "name" ) ? 
                                                     QString( "\n%1" ).arg( blanks_cormap_parameters[ "name" ] ) : QString( "" ) )
                                               : QString()
                                               ,( blanks_brookesmap_sliding_results.size() &&
                                                  blanks_cormap_pvaluepairs.size() >= IB_MIN_FRAMES ) ? 2 : 0
                                               ) ) {
               case -1 : // escape
                  return current_mode != MODE_NORMAL ? wheel_cancel() : update_enables();
                  break;
               case 0 : // load blanks
                  {
                     QStringList                  new_blanks;
                     QStringList                  new_blanks_files;
                     set < QString >              new_blanks_set;

                     QString use_dir = QDir::current().canonicalPath();
                     ((US_Hydrodyn  *)us_hydrodyn)->select_from_directory_history( use_dir, this );

                     new_blanks_files = QFileDialog::getOpenFileNames( this , windowTitle() + us_tr( " : Select blanks files" ) , use_dir , "dat files [foxs / other] (*.dat);;"
                                                                      "All files (*);;"
                                                                      "ssaxs files (*.ssaxs);;"
                                                                      "txt files [specify q, I, sigma columns] (*.txt)" );

         
                     if ( !new_blanks_files.size() ) {
                        return current_mode != MODE_NORMAL ? wheel_cancel() : update_enables();
                     }


                     for ( int i = 0; i < (int) new_blanks_files.size(); ++i ) {
                        QString basename = QFileInfo( new_blanks_files[ i ] ).completeBaseName();
                        new_blanks << basename;
                        new_blanks_set.insert( basename );
                     }

                     default_blanks       = new_blanks;
                     default_blanks_files = new_blanks_files;
                     default_blanks_set   = new_blanks_set;

                     do_blanks_start = true;
                     show_determination_info = false;
                  }
                  break;
               case 1 : // load previous analysis
                  {
                     QString use_dir = QDir::current().canonicalPath();
                     ((US_Hydrodyn  *)us_hydrodyn)->select_from_directory_history( use_dir, this );

                     QString filename = QFileDialog::getOpenFileName( this , windowTitle() , use_dir , "*.csv *.CSV" );

   
                     if ( filename.isEmpty() ) {
                        return current_mode != MODE_NORMAL ?  wheel_cancel() : update_enables();
                     }
                     ((US_Hydrodyn *)us_hydrodyn)->add_to_directory_history( filename );

                     map < QString, QString >       load_parameters;
                     vector < vector < double > >   load_pvaluepairs;
                     vector < vector < double > >   load_adjpvaluepairs;
                     vector < QString >             load_selected_files;

                     if (
                         !US_Hydrodyn_Saxs_Cormap::load_csv(
                                                            filename,
                                                            load_parameters,
                                                            load_pvaluepairs,
                                                            load_adjpvaluepairs,
                                                            load_selected_files,
                                                            this
                                                            ) 
                         ) {
                        return current_mode != MODE_NORMAL ?  wheel_cancel() : update_enables();
                     }

                     if ( !load_parameters.count( "isblanks" ) ) {
                        QMessageBox::warning( this, windowTitle() + us_tr( " : Baseline" ),
                                              QString( us_tr( "The loaded analysis does not appear to be a blanks analysis" ) )
                                              ,QMessageBox::Ok | QMessageBox::Default
                                              ,QMessageBox::NoButton
                                              );
                        return current_mode != MODE_NORMAL ?  wheel_cancel() : update_enables();
                     }
                  

                     blanks_cormap_parameters  = load_parameters;
                     blanks_cormap_pvaluepairs = load_pvaluepairs;

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
                        } else {
                           blanks_cormap_parameters.clear( );
                           blanks_cormap_pvaluepairs.clear( );
                           return current_mode != MODE_NORMAL ?  wheel_cancel() : update_enables();
                        }
                     }
                  }            
                  break;
               case 2 : // accept currently analyzed blanks
                  // us_qdebug( "accept currently analyzed blanks" );
                  break;
               }
            }
            break;
         case 1 :
            {
               // without blanks
               baseline_hb_only_mode = true;
               blanks_cormap_parameters.clear( );
               blanks_cormap_pvaluepairs.clear( );
            }
            break;
         }
      }
         
      baseline_max_window_size = 
         baseline_hb_only_mode
         ? (int) f_qs[ wheel_file ].size()
         : (int) blanks_cormap_pvaluepairs.size()
         ;

      if ( baseline_max_window_size > (int) f_qs[ wheel_file ].size() ) {
         baseline_max_window_size = (int) f_qs[ wheel_file ].size();
      }

      if ( show_determination_info ) {
         if ( baseline_max_window_size < IB_MIN_FRAMES ) {
            QMessageBox::warning( this, windowTitle() + us_tr( " : Baseline" ),
                                  QString( us_tr( "You must have minimum of %1 frames for blanks" ) ).arg( IB_MIN_FRAMES )
                                  ,QMessageBox::Ok | QMessageBox::Default
                                  ,QMessageBox::NoButton
                                  );
            return current_mode != MODE_NORMAL ?  wheel_cancel() : update_enables();
         } else {
            QMessageBox::information( this
                                      ,windowTitle() + us_tr( " : Baseline" )
                                      ,us_tr( "The first step of the integral baseline procedure is to determine a constant final baseline." )
                                      ,QMessageBox::Ok | QMessageBox::Default
                                      ,QMessageBox::NoButton
                                      );
         }
      }

      // int use_default_range = IB_DEFAULT_RANGE > baseline_max_window_size ? baseline_max_window_size : IB_DEFAULT_RANGE;

      // us_qdebug( QString( "use default range %1 IB_DEFAULT_RANGE %2 baseline_max_window_size %3 q_size %4" )
      //         .arg( use_default_range )
      //         .arg( IB_DEFAULT_RANGE )
      //         .arg( baseline_max_window_size )
      //         .arg( q_size ) );

      // int pos = q_size - use_default_range - IB_END_OFS;
      int pos = ( q_size * 3 ) / 4;
      if ( pos < 0 ) {
         pos = 0;
      }
      default_baseline_end_s = f_qs[ wheel_file ][ pos ];

      // us_qdebug( QString( "default_baseline_end_s %1 pos %2" )
      //         .arg( default_baseline_end_s )
      //         .arg( pos ) );

      // pos = q_size - IB_END_OFS - 1;
      pos += IB_DEFAULT_RANGE - 1;
      if ( pos >= q_size ) {
         pos = q_size - 1;
      }
      if ( pos < 0 ) {
         pos = 0;
      }
      default_baseline_end_e = f_qs[ wheel_file ][ pos ];

      // us_qdebug( QString( "default_baseline_end_e %1 pos %2" )
      //         .arg( default_baseline_end_e )
      //         .arg( pos ) );
      force_endpoints_default = true;
   }      

   // baseline_mode = true;
   mode_select( MODE_BASELINE );
   running       = true;

   // qwtw_wheel->setRange( f_qs[ wheel_file ][ 0 ],
   //                       f_qs[ wheel_file ].back(), 
   //                       ( f_qs[ wheel_file ].back() - f_qs[ wheel_file ][ 0 ] ) / UHSH_WHEEL_RES );

   // double q_len       = f_qs[ wheel_file ].back() - f_qs[ wheel_file ][ 0 ];
   // double q_len_delta = q_len * 0.05;
   int pos_len_delta = q_size / 20;

   qwtw_wheel->setRange( 0, (double) f_qs[ wheel_file ].size() - 1); qwtw_wheel->setSingleStep( 1 );

   if ( baseline_integral ) {
      if ( force_endpoints_default ||
           le_baseline_end_s->text().isEmpty() ||
           le_baseline_end_s->text() == "0" ||
           le_baseline_end_s->text().toDouble() > f_qs[ wheel_file ].back() )
      {
         disconnect( le_baseline_end_s, SIGNAL( textChanged( const QString & ) ), 0, 0 );
         le_baseline_end_s->setText( QString( "%1" ).arg( default_baseline_end_s ) );
         connect( le_baseline_end_s, SIGNAL( textChanged( const QString & ) ), SLOT( baseline_end_s_text( const QString & ) ) );
      }

      if ( force_endpoints_default ||
           le_baseline_end->text().isEmpty() ||
           le_baseline_end->text() == "0" ||
           le_baseline_end->text().toDouble() > f_qs[ wheel_file ].back() )
      {
         disconnect( le_baseline_end, SIGNAL( textChanged( const QString & ) ), 0, 0 );
         le_baseline_end->setText( QString( "%1" ).arg( default_baseline_end ) );
         connect( le_baseline_end, SIGNAL( textChanged( const QString & ) ), SLOT( baseline_end_text( const QString & ) ) );
      }

      if ( force_endpoints_default ||
           le_baseline_end_e->text().isEmpty() ||
           le_baseline_end_e->text() == "0" ||
           le_baseline_end_e->text().toDouble() > f_qs[ wheel_file ].back() )
      {
         disconnect( le_baseline_end_e, SIGNAL( textChanged( const QString & ) ), 0, 0 );
         le_baseline_end_e->setText( QString( "%1" ).arg( default_baseline_end_e ) );
         connect( le_baseline_end_e, SIGNAL( textChanged( const QString & ) ), SLOT( baseline_end_e_text( const QString & ) ) );
      }
      disconnect( le_baseline_width, SIGNAL( textChanged( const QString & ) ), 0, 0 );
      le_baseline_width->setText( QString( "%1" )
                                  .arg( input_double_to_pos( le_baseline_end_e->text().toDouble() ) -
                                        input_double_to_pos( le_baseline_end_s->text().toDouble() ) + 1 ) );
      connect( le_baseline_width, SIGNAL( textChanged( const QString & ) ), SLOT( baseline_width_text( const QString & ) ) );
   } else {

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
         // le_baseline_start->setText( QString( "%1" ).arg( f_qs[ wheel_file ][ 0 ] + q_len_delta ) );
         le_baseline_start->setText( QString( "%1" ).arg( f_qs[ wheel_file ][ 0 + pos_len_delta < q_size - 1 ?
                                                                              0 + pos_len_delta : 0  ] ) );
         connect( le_baseline_start, SIGNAL( textChanged( const QString & ) ), SLOT( baseline_start_text( const QString & ) ) );
      }

      if ( le_baseline_start_e->text().isEmpty() ||
           le_baseline_start_e->text() == "0" ||
           le_baseline_start_e->text().toDouble() < f_qs[ wheel_file ][ 0 ] )
      {
         disconnect( le_baseline_start_e, SIGNAL( textChanged( const QString & ) ), 0, 0 );
         // le_baseline_start_e->setText( QString( "%1" ).arg( f_qs[ wheel_file ][ 0 ] + 2e0 * q_len_delta) );
         le_baseline_start_e->setText( QString( "%1" ).arg( f_qs[ wheel_file ][ 0 + 2 * pos_len_delta < q_size - 1 ?
                                                                                0 + 2 * pos_len_delta : 0 ] ) );
         connect( le_baseline_start_e, SIGNAL( textChanged( const QString & ) ), SLOT( baseline_start_e_text( const QString & ) ) );
      }

      if ( le_baseline_end_s->text().isEmpty() ||
           le_baseline_end_s->text() == "0" ||
           le_baseline_end_s->text().toDouble() > f_qs[ wheel_file ].back() )
      {
         disconnect( le_baseline_end_s, SIGNAL( textChanged( const QString & ) ), 0, 0 );
         // le_baseline_end_s->setText( QString( "%1" ).arg( f_qs[ wheel_file ].back() - 2e0 * q_len_delta ) );
         le_baseline_end_s->setText( QString( "%1" ).arg( f_qs[ wheel_file ][ q_size - 1 - 2 * pos_len_delta > 0 ?
                                                                              q_size - 1 - 2 * pos_len_delta : 0 ] ) );
         connect( le_baseline_end_s, SIGNAL( textChanged( const QString & ) ), SLOT( baseline_end_s_text( const QString & ) ) );
      }

      if ( le_baseline_end->text().isEmpty() ||
           le_baseline_end->text() == "0" ||
           le_baseline_end->text().toDouble() > f_qs[ wheel_file ].back() )
      {
         disconnect( le_baseline_end, SIGNAL( textChanged( const QString & ) ), 0, 0 );
         // le_baseline_end->setText( QString( "%1" ).arg( f_qs[ wheel_file ].back() - q_len_delta ) );
         le_baseline_end->setText( QString( "%1" ).arg( f_qs[ wheel_file ][ q_size - 1 - pos_len_delta > 0 ?
                                                                            q_size - 1 - pos_len_delta : 0 ] ) );
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
   }

   if ( baseline_integral ) {
      cb_baseline_start_zero->setChecked( true );
      cb_baseline_start_zero->hide();
   } else {
      cb_baseline_start_zero->setChecked( false );
      cb_baseline_start_zero->show();
      QMessageBox::warning(this, 
                           windowTitle(),
                           QString( us_tr( "Please note:\n\n"
                                        "You are utilizing the linear baseline method.\n"
                                        "This method compensates for a systematic drift and can distort/hide the effects of capillary fouling.\n"
                                        "The linear baseline correction does not have an explicit physical basis and should be applied with care\n"
                                        "and only in the case of \"small-amplitude\" corrections as compared to the experimental uncertainty.\n" ) ),
                           QMessageBox::Ok | QMessageBox::Default,
                           QMessageBox::NoButton
                           );
   }
   baseline_init_markers();
   replot_baseline( "baseline_start" );
   disable_all();
   if ( do_blanks_start ) {
      blanks_start();
   } else {
      baseline_enables();
   }
}

void US_Hydrodyn_Mals_Saxs::baseline_enables()
{
   pb_pp->setEnabled( true );

   if ( current_mode == MODE_BLANKS ) {
      return blanks_enables();
   }

   if ( baseline_test_mode ) {
      disable_all();
      ShowHide::hide_widgets( wheel_below_widgets, always_hide_widgets, false );
      pb_wheel_cancel        ->setEnabled( true );
      pb_line_width          ->setEnabled( true );
      pb_color_rotate        ->setEnabled( true );
      pb_rescale             ->setEnabled( true );
      pb_rescale_y           ->setEnabled( true );
      cb_eb                  ->setEnabled( true );
      pb_pp                  ->setEnabled( true );
      wheel_enables();
      return;
   }

   ShowHide::hide_widgets( wheel_below_widgets, always_hide_widgets );

   pb_line_width          ->setEnabled( true );
   pb_color_rotate        ->setEnabled( true );

   cb_eb                  ->setEnabled( false );
   pb_blanks_start        ->setEnabled( baseline_multi );
   pb_bb_cm_inc           ->setEnabled( true );
   pb_baseline_best       ->setEnabled( baseline_integral );
   pb_cormap              ->setEnabled( baseline_multi );
   pb_baseline_start      ->setEnabled( false );
   pb_baseline_test       ->setEnabled( baseline_multi && 
                                        le_baseline_end_s->text().toDouble() < le_baseline_end_e->text().toDouble() );
   pb_wheel_cancel        ->setEnabled( true );
   if ( baseline_integral ) {
      pb_wheel_save          ->setEnabled( false );
      pb_baseline_apply      ->setEnabled( 
                                          baseline_best_ok &&
                                          baseline_best_last_window_size == le_baseline_width->text().toInt() 
                                           );
   } else {
      pb_wheel_save          ->setEnabled(
                                          le_baseline_start_s->text().toDouble() != org_baseline_start_s ||
                                          le_baseline_start  ->text().toDouble() != org_baseline_start   ||
                                          le_baseline_start_e->text().toDouble() != org_baseline_start_e ||
                                          le_baseline_end_s  ->text().toDouble() != org_baseline_end_s   ||
                                          le_baseline_end    ->text().toDouble() != org_baseline_end     ||   
                                          le_baseline_end_e  ->text().toDouble() != org_baseline_end_e   ||
                                          cb_baseline_start_zero->isChecked()    != org_baseline_start_zero
                                          );
   }
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

   if ( !baseline_integral ) {
      cb_baseline_fix_width ->hide();
      le_baseline_width     ->hide();
   }

   le_baseline_width      ->setEnabled( !cb_baseline_fix_width->isChecked() );

   le_baseline_start_s    ->setEnabled( !cb_baseline_start_zero->isChecked() );
   le_baseline_start      ->setEnabled( !cb_baseline_start_zero->isChecked() );
   le_baseline_start_e    ->setEnabled( !cb_baseline_start_zero->isChecked() );
   le_baseline_end_s      ->setEnabled( true );
   le_baseline_end        ->setEnabled( true );
   le_baseline_end_e      ->setEnabled( true );
   wheel_enables(
#if QT_VERSION > 0x050000
                 le_last_focus == le_baseline_start_s || 
                 le_last_focus == le_baseline_start   || 
                 le_last_focus == le_baseline_start_e || 
                 le_last_focus == le_baseline_end_s   ||
                 le_last_focus == le_baseline_end     ||
                 le_last_focus == le_baseline_end_e   ||
                 le_last_focus == le_baseline_width  
#else
                 le_baseline_start_s->hasFocus() || 
                 le_baseline_start  ->hasFocus() || 
                 le_baseline_start_e->hasFocus() || 
                 le_baseline_end_s  ->hasFocus() ||
                 le_baseline_end    ->hasFocus() ||
                 le_baseline_end_e  ->hasFocus() ||
                 le_baseline_width  ->hasFocus()
#endif
                 );
   pb_rescale             ->setEnabled( true );
   pb_rescale_y           ->setEnabled( true );
   pb_view                ->setEnabled( true );
}

void US_Hydrodyn_Mals_Saxs::set_baseline_start_zero()
{
   if ( !cb_baseline_start_zero->isChecked() )
   {
      // QMessageBox::warning(
      //                      this,
      //                      this->windowTitle() + us_tr(": Baseline setup: nonzero starting offset" ),
      //                      us_tr(
      //                         "Having a non zero starting offset likely means problems with buffer subtraction which should be corrected before *any* analysis is done.\n"
      //                         ),
      //                      QMessageBox::Ok | QMessageBox::Default,
      //                      QMessageBox::NoButton
      //                      );
   }      

   if ( cb_baseline_start_zero->isChecked() &&
        ( le_baseline_start_s->hasFocus() ||
          le_baseline_start  ->hasFocus() ||
          le_baseline_start_e->hasFocus() ) )
   {
      le_last_focus = (mQLineEdit *)0;
   }

   for ( unsigned int i = 0; i < ( unsigned int ) plotted_baseline.size(); i++ )
   {
#if QT_VERSION < 0x040000
      plot_dist->removeCurve( plotted_baseline[ i ] );
#else
      plotted_baseline[ i ]->detach();
#endif
   }

   baseline_init_markers();
   replot_baseline( "set baseline start zero" );
   plot_dist->replot();
   baseline_enables();
}

static QColor start_color( 255, 165, 0 );
static QColor end_color  ( 255, 160, 122 );

void US_Hydrodyn_Mals_Saxs::baseline_init_markers()
{
   gauss_delete_markers();

   plotted_baseline.clear( );

   if ( current_mode == MODE_BLANKS ) {
      gauss_add_marker( le_baseline_end_s  ->text().toDouble(), Qt::magenta, us_tr( "\nStart" ), Qt::AlignRight | Qt::AlignTop );
      gauss_add_marker( le_baseline_end_e  ->text().toDouble(), Qt::magenta, us_tr( "\nEnd"   ), Qt::AlignLeft  | Qt::AlignTop );
   } else {
      if ( baseline_integral ) {
         gauss_add_marker( le_baseline_end_s  ->text().toDouble(), Qt::red    ,
                           us_tr( "\n  & Analysis;\nEnd Integral\n   Bas. calc." ), Qt::AlignLeft | Qt::AlignTop );
         gauss_add_marker( le_baseline_end_s  ->text().toDouble(), Qt::magenta,
                           us_tr( "Start Bas. Window"                               ), Qt::AlignLeft | Qt::AlignTop );
         gauss_add_marker( le_baseline_end    ->text().toDouble(), Qt::red,
                           us_tr( "\n\n\n\n\n\nEnd Bas.\nAnalysis"             ), Qt::AlignLeft | Qt::AlignTop );
         gauss_add_marker( le_baseline_end_e  ->text().toDouble(), Qt::magenta,
                           us_tr( "\n\n\n\nEnd Bas.\n  Window"                     ), Qt::AlignLeft | Qt::AlignTop );
      } else {
         if ( !cb_baseline_start_zero->isChecked() )
         {
            gauss_add_marker( le_baseline_start_s->text().toDouble(), Qt::magenta, us_tr( "\nLFS  \nStart"   ) );
            gauss_add_marker( le_baseline_start  ->text().toDouble(), Qt::red,     us_tr( "Start"          ) );
            gauss_add_marker( le_baseline_start_e->text().toDouble(), Qt::magenta, us_tr( "\n\n\nLFS\nEnd" ) );
            gauss_add_marker( le_baseline_end_s  ->text().toDouble(), Qt::magenta, us_tr( "\n  LFE\nStart"   ), Qt::AlignLeft | Qt::AlignTop );
            gauss_add_marker( le_baseline_end    ->text().toDouble(), Qt::red,     us_tr( "End"            ), Qt::AlignLeft | Qt::AlignTop );
            gauss_add_marker( le_baseline_end_e  ->text().toDouble(), Qt::magenta, us_tr( "\n\n\nLFE\nEnd" ), Qt::AlignLeft | Qt::AlignTop );
         } else {
            gauss_add_marker( le_baseline_end_s  ->text().toDouble(), Qt::magenta, us_tr( "\nStart"   ), Qt::AlignLeft | Qt::AlignTop );
            gauss_add_marker( le_baseline_end    ->text().toDouble(), Qt::red,     us_tr( "End"            ), Qt::AlignLeft | Qt::AlignTop );
            gauss_add_marker( le_baseline_end_e  ->text().toDouble(), Qt::magenta, us_tr( "\n\nEnd" ), Qt::AlignLeft | Qt::AlignTop );
         }
      }
   }

   if ( !suppress_replot )
   {
      plot_dist->replot();
   }
}

void US_Hydrodyn_Mals_Saxs::replot_baseline_integral()
{
   for ( int i = 0; i < (int) plotted_baseline.size(); ++i ) {
#if QT_VERSION < 0x040000
      plot_dist->removeCurve( plotted_baseline[ i ] );
#else
      plotted_baseline[ i ]->detach();
#endif
   }
   plotted_baseline.clear( );

   double end_s   = le_baseline_end_s  ->text().toDouble();
   double end_e   = le_baseline_end_e  ->text().toDouble();

   vector < double > end_q;
   vector < double > end_I;
   vector < double > end_errors;

   int wheel_pts = (int) f_qs[ wheel_file ].size();

   bool use_errors = f_errors[ wheel_file ].size() == f_qs[ wheel_file ].size();
   {
      unsigned int i = 0;
      if ( f_qs[ wheel_file ][ i ] >= end_s &&
           f_qs[ wheel_file ][ i ] <= end_e )
      {
         end_q.push_back( f_qs[ wheel_file ][ i ] );
         end_I.push_back( f_Is[ wheel_file ][ i ] );
         if ( use_errors ) {
            end_errors.push_back( f_errors[ wheel_file ][ i ] );
         }
      }
   }

   if ( use_errors ) {
      for ( int i = 1; i < wheel_pts; ++i ) {
         if ( f_qs[ wheel_file ][ i ] >= end_s &&
              f_qs[ wheel_file ][ i ] <= end_e ) {
            end_q.push_back( f_qs[ wheel_file ][ i ] );
            end_I.push_back( f_Is[ wheel_file ][ i ] );
            end_errors.push_back( f_errors[ wheel_file ][ i ] );
         }
      }
   } else {
      for ( int i = 1; i < wheel_pts; ++i ) {
         if ( f_qs[ wheel_file ][ i ] >= end_s &&
              f_qs[ wheel_file ][ i ] <= end_e ) {
            end_q.push_back( f_qs[ wheel_file ][ i ] );
            end_I.push_back( f_Is[ wheel_file ][ i ] );
         }
      }
   }

   if ( use_errors && !is_nonzero_vector( end_errors ) ) {
      use_errors = false;
   }

   int end_pts = (int) end_q.size();

   // compute sd weighted average over start -> end

   double avg_bl = 0e0;

   if ( use_errors ) {
      double invsd2;
      double sum_invsd2 = 0e0;;
      for ( int i = 0; i < end_pts; i++ ) {
         invsd2 = 1e0 / end_errors[ i ];
         invsd2 *= invsd2;
         avg_bl += end_I[ i ] * invsd2;
         sum_invsd2 += invsd2;
      }
      avg_bl /= sum_invsd2;
   } else {
      for ( int i = 0; i < end_pts; i++ ) {
         avg_bl += end_I[ i ];
      }
      avg_bl /= (double) end_pts;
   }

   // plot avg_bl

   if ( end_pts ) {
      double x[ 2 ];
      double y[ 2 ];

      x[ 0 ] = f_qs[ wheel_file ].front();
      x[ 1 ] = f_qs[ wheel_file ].back();

      y[ 0 ] = avg_bl;
      y[ 1 ] = avg_bl;

#if QT_VERSION < 0x040000
      long curve;
      curve = plot_dist->insertCurve( "avg buffer" );
      plot_dist->setCurveStyle( curve, QwtCurve::Lines );
#else
      QwtPlotCurve *curve = new QwtPlotCurve( "avg buffer" );
      curve->setStyle( QwtPlotCurve::Lines );
#endif

      plotted_baseline.push_back( curve );

#if QT_VERSION < 0x040000
      plot_dist->setCurvePen( curve, QPen( end_color, use_line_width, Qt::DashLine ) );
      plot_dist->setCurveData( plotted_baseline.back(), x, y, 2 );
#else
      curve->setPen( QPen( end_color, use_line_width, Qt::DashLine ) );
      plotted_baseline.back()->setSamples( x, y, 2 );
      curve->attach( plot_dist );
#endif
   }

   if ( !suppress_replot )
   {
      plot_dist->replot();
   }
   // us_qdebug( QString( "replot_baseline_integral() end current plotted_baseline curves size %1" ).arg( plotted_baseline.size() ) );
}

void US_Hydrodyn_Mals_Saxs::replot_baseline( QString /* qs */ )
{
   // us_qdebug( QString( "replot_baseline( %1 ) current plotted_baseline curves size %2 %3" )
   //         .arg( qs )
   //         .arg( plotted_baseline.size() ).arg( baseline_integral ? "integral mode" : "!integral mode" ) );

   if ( current_mode == MODE_BLANKS ) {
      return;
   }
   if ( baseline_integral ) {
      return replot_baseline_integral();
   }

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
#if QT_VERSION < 0x040000
      plot_dist->removeCurve( plotted_baseline[ i ] );
#else
      plotted_baseline[ i ]->detach();
#endif
   }
   plotted_baseline.clear( );

   // the baseline
   {
      // cout << QString( "baseline slope %1 intercept %2\n" ).arg( baseline_slope ).arg( baseline_intercept );
      // printvector( "baseline x", x );
      // printvector( "baseline y", y );

#if QT_VERSION < 0x040000
      long curve;
      curve = plot_dist->insertCurve( "baseline" );
      plot_dist->setCurveStyle( curve, QwtCurve::Lines );
#else
      QwtPlotCurve *curve = new QwtPlotCurve( "baseline" );
      curve->setStyle( QwtPlotCurve::Lines );
#endif

      plotted_baseline.push_back( curve );

#if QT_VERSION < 0x040000
      plot_dist->setCurvePen( curve, QPen( Qt::green , use_line_width, Qt::DashLine ) );
      plot_dist->setCurveData( plotted_baseline[ 0 ],
                               (double *)&x[ 0 ],
                               (double *)&y[ 0 ],
                               2
                               );
#else
      curve->setPen( QPen( Qt::green, use_line_width, Qt::DashLine ) );
      plotted_baseline[ 0 ]->setSamples(
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

#if QT_VERSION < 0x040000
      long curve;
      curve = plot_dist->insertCurve( "baseline s" );
      plot_dist->setCurveStyle( curve, QwtCurve::Lines );
#else
      QwtPlotCurve *curve = new QwtPlotCurve( "baseline s" );
      curve->setStyle( QwtPlotCurve::Lines );
#endif

      plotted_baseline.push_back( curve );

#if QT_VERSION < 0x040000
      plot_dist->setCurvePen( curve, QPen( start_color, use_line_width, Qt::DashLine ) );
      plot_dist->setCurveData( plotted_baseline.back(),
                               (double *)&x[ 0 ],
                               (double *)&y[ 0 ],
                               2
                               );
#else
      curve->setPen( QPen( start_color, use_line_width, Qt::DashLine ) );
      plotted_baseline.back()->setSamples(
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

#if QT_VERSION < 0x040000
      long curve;
      curve = plot_dist->insertCurve( "baseline e" );
      plot_dist->setCurveStyle( curve, QwtCurve::Lines );
#else
      QwtPlotCurve *curve = new QwtPlotCurve( "baseline e" );
      curve->setStyle( QwtPlotCurve::Lines );
#endif

      plotted_baseline.push_back( curve );

#if QT_VERSION < 0x040000
      plot_dist->setCurvePen( curve, QPen( end_color, use_line_width, Qt::DashLine ) );
      plot_dist->setCurveData( plotted_baseline.back(),
                               (double *)&x[ 0 ],
                               (double *)&y[ 0 ],
                               2
                               );
#else
      curve->setPen( QPen( end_color, use_line_width, Qt::DashLine ) );
      plotted_baseline.back()->setSamples(
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

void US_Hydrodyn_Mals_Saxs::baseline_apply()
{
   int smoothing = 0;
   bool integral = ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_saxs_bl_integral" ] == "true";
   if ( integral )
   {
      smoothing = ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_saxs_bl_smooth" ].toInt();
   }
   // bool save_bl = ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_saxs_bl_save" ] == "true";
   bool save_bl = false;
   unsigned int reps = 0;
   if ( integral )
   {
      reps = ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_saxs_bl_reps" ].toInt();
   }
   
   disable_all();
   baseline_apply( all_selected_files(), integral, smoothing, save_bl, reps );
   current_mode == MODE_BASELINE ? baseline_enables() : update_enables();
}

void US_Hydrodyn_Mals_Saxs::baseline_apply( QStringList files, 
                                            bool integral, 
                                            int smoothing, 
                                            bool save_bl, 
                                            unsigned int reps,
                                            bool do_replot,
                                            bool quiet)
{
   baseline_apply_created.clear( );

   map < QString, bool > current_files;
   for ( int i = 0; i < (int)lb_files->count(); i++ )
   {
      current_files[ lb_files->item( i )->text() ] = true;
   }

   map < QString, bool > select_files;

   if ( integral ) {
      check_zi_window( files, us_tr( "You may have already noticed this message when making I(t).\n\n" ) );
   }
      
   if ( integral && save_bl && files.size() > 3 )
   {
      switch ( QMessageBox::warning(this, 
                                    windowTitle() + us_tr( " : Baseline Apply" ),
                                    QString( us_tr( "Please note:\n\n"
                                                 "You have selected to produce separate baseline curves and have %1 files selected\n"
                                                 "This will generate upto %2 separate baseline curves in addition to your %3 baseline subtracted files\n"
                                                 "What would you like to do?\n" ) )
                                    .arg( files.size() )
                                    .arg( (int) reps * (int)  files.size() )
                                    .arg( files.size() ),
                                    us_tr( "&Generate them anyway" ), 
                                    us_tr( "&Temporarily turn off production of separate baseline curves" ), 
                                    us_tr( "&Quit" ), 
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

   // if ( integral && reps < 5 && !quiet )
   // {
   //    switch ( QMessageBox::warning(this, 
   //                                  windowTitle() + us_tr( " : Baseline Apply" ),
   //                                  QString( us_tr( "Please note:\n\n"
   //                                               "You have selected the integral baseline method with maximum iterations of %1\n"
   //                                               "It is recommended that a minimum of 5 iterations be used to approach convergence\n"
   //                                               "What would you like to do?\n" ) )
   //                                  .arg( reps ),
   //                                  us_tr( "&Continue anyway" ), 
   //                                  us_tr( "&Quit" )
   //                                  ) )
   //    {
   //    case 0 : // continue
   //       break;
   //    case 1 : // quit
   //       return;
   //       break;
   //    }
   // }

   double start_s = le_baseline_start_s->text().toDouble();
   double start   = le_baseline_start  ->text().toDouble();
   double start_e = le_baseline_start_e->text().toDouble();
   double end_s   = le_baseline_end_s  ->text().toDouble();
   double end     = le_baseline_end    ->text().toDouble();
   double end_e   = le_baseline_end_e  ->text().toDouble();

   if ( integral ) {
      end = end_s;
   }

   // redo this to compute from best linear fit over ranges
   QStringList messages;

   bool test_alt_integral_bl = ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_saxs_bl_save" ] == "true";
   test_alt_integral_bl = true;
   // if ( test_alt_integral_bl ) {
   //    editor_msg( "red", us_tr( "Using alternative baseline integral strategy" ) );
   // }

   bool use_start_region = 
      false &&
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "mals_saxs_bl_start_region" ) &&
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_saxs_bl_start_region" ].toInt() > 0
      ;

   bool use_i_power = 
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "mals_saxs_bl_i_power" ) &&
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_saxs_bl_i_power" ].toDouble() != 1e0 &&
      started_in_expert_mode
      ;

   double i_power = 1e0;

   if ( use_i_power ) {
      i_power = ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_saxs_bl_i_power" ].toDouble();
      editor_msg( "darkred", QString( us_tr( "Using power of intensity (%1) for distribution of delta baseline" ) ).arg( i_power ) );
      // if ( !quiet ) {
      //    switch ( QMessageBox::warning(this, 
      //                                  windowTitle() + us_tr( " : Baseline Apply" ),
      //                                  QString( us_tr( "Please note:\n\n"
      //                                               "You are using a non-unity intensity exponent of %1 for integral baseline correction.\n"
      //                                               "The model of intesity proportional deposits is based on a unity intensity exponent.\n"
      //                                               "It is possible that a specific capillary fouling mechanism may be approximated corrected with a non-unity exponent.\n"
      //                                               "We urge you to use a non-unity intensity exponent with care.\n"
      //                                               "What would you like to do?\n" ) )
      //                                  .arg( i_power ),
      //                                  us_tr( "&Continue" ), 
      //                                  us_tr( "&Quit" )
      //                                  ) )
      //    {
      //    case 0 : // continue
      //       break;
      //    case 1 : // quit
      //       return;
      //       break;
      //    }
      // }
   }

   for ( unsigned int i = 0; i < ( unsigned int ) files.size(); i++ )
   {
      if ( !quiet ) {
         progress->setValue( i ); progress->setMaximum( files.size() );
         qApp->processEvents();
      }

      unsigned int before_start = 0;
      unsigned int after_start  = 1;
      unsigned int before_end   = 0;
      unsigned int after_end    = 1;

      vector < double > start_q;
      vector < double > start_I;

      vector < double > end_q;
      vector < double > end_I;
      vector < double > end_errors;

      bool use_errors = f_errors[ files[ i ] ].size() == f_qs[ files[ i ] ].size() && is_nonzero_vector( f_errors[ files[ i ] ] );

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
            if ( use_errors ) {
               end_errors.push_back( f_errors[ files[ i ] ][ j ] );
            }
         }
      }

      if ( use_errors ) {
         for ( unsigned int j = 1; j < f_qs[ files[ i ] ].size(); j++ ) {
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
               end_errors.push_back( f_errors[ files[ i ] ][ j ] );
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
      } else {
         for ( unsigned int j = 1; j < f_qs[ files[ i ] ].size(); j++ ) {
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
      }

      if ( use_errors && !is_nonzero_vector( end_errors ) ) {
         use_errors = false;
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


      if ( integral ) {
         {
            // recompute start_y
            if ( use_start_region ) {
               int start_region_frames = ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_saxs_bl_start_region" ].toInt();

               double avg_bl = 0e0;

               vector < double > start_I;
               vector < double > start_errors;


               if ( use_errors ) {
                  for ( int j = 0; j < (int) f_qs[ files[ i ] ].size() && j < start_region_frames; ++j ) {
                     start_I     .push_back( f_Is    [ files[ i ] ][ j ] );
                     start_errors.push_back( f_errors[ files[ i ] ][ j ] );
                  }
               } else {
                  for ( int j = 0; j < (int) f_qs[ files[ i ] ].size() && j < start_region_frames; ++j ) {
                     start_I.push_back( f_Is[ files[ i ] ][ j ] );
                  }
               }                  
               
               int start_pts = (int) start_I.size();

               // compute sd weighted average over start -> end


               if ( use_errors ) {
                  double invsd2;
                  double sum_invsd2 = 0e0;;
                  for ( int i = 0; i < start_pts; i++ ) {
                     invsd2 = 1e0 / start_errors[ i ];
                     invsd2 *= invsd2;
                     avg_bl += start_I[ i ] * invsd2;
                     sum_invsd2 += invsd2;
                  }
                  avg_bl /= sum_invsd2;
               } else {
                  for ( int i = 0; i < start_pts; i++ ) {
                     avg_bl += start_I[ i ];
                  }
                  avg_bl /= (double) start_pts;
               }

               start_y = avg_bl;

               editor_msg( "darkgreen", 
                           QString( "Using start region size %1 for baseline with avg value %2" )
                           .arg( start_region_frames ) 
                           .arg( start_y ) 
                           );
            }
         }

         // recompute end_y
         {
            double avg_bl = 0e0;

            int end_pts = (int) end_q.size();

            // compute sd weighted average over start -> end


            if ( use_errors ) {
               double invsd2;
               double sum_invsd2 = 0e0;;
               for ( int i = 0; i < end_pts; i++ ) {
                  invsd2 = 1e0 / end_errors[ i ];
                  invsd2 *= invsd2;
                  avg_bl += end_I[ i ] * invsd2;
                  sum_invsd2 += invsd2;
               }
               avg_bl /= sum_invsd2;
            } else {
               for ( int i = 0; i < end_pts; i++ ) {
                  avg_bl += end_I[ i ];
               }
               avg_bl /= (double) end_pts;
            }

            end_y = avg_bl;
         }

         // us_qdebug( QString( "integral baseline apply, start_y %1 end_y %2" ).arg( start_y ).arg( end_y ) );

         double delta_bl = end_y - start_y;

         if ( smoothing )
         {
            US_Saxs_Util usu;
            if ( !usu.smooth( f_Is[ files[ i ] ], bl_I, smoothing ) )
            {
               bl_I = f_Is[ files[ i ] ];
               messages << QString( us_tr( "Error: smoothing error on %1" ) ).arg( files[ i ] );
               editor_msg( "red", messages.back() );
            } else {
               if ( save_bl )
               {
                  QString this_file = QString( "SM%1_%2" ).arg( smoothing ).arg( files[ i ] );
                  baseline_apply_created.push_back( this_file );
                  add_plot( this_file, f_qs[ files[ i ] ], bl_I, true, false );
               }
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

         // note: we have replaced alpha with gamma to avoid overloading with CorMap analysis alpha
         double alpha = 0e0;
         double alpha_epsilon = 
            ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "mals_saxs_bl_epsilon" ) ?
            ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_saxs_bl_epsilon" ].toDouble() : 5e-3;

         double last_alpha = 0e0;
         vector < double > bl = last_bl;

         if ( delta_bl < 0e0 ) {
            // messages << QString( us_tr( "Notice: the overall change in baseline of %1 is negative (%2), so a constant baseline correction will be applied" ) ).arg( files[ i ] ).arg( delta_bl );
            messages << QString( us_tr( "Notice: the overall change in baseline of %1 is negative (%2), so no baseline correction will be applied" ) ).arg( files[ i ] ).arg( delta_bl );
            editor_msg( "dark red", messages.back() );
            new_I = f_Is[ files[ i ] ];

            // for ( int i = 0; i < (int) new_I.size(); ++i ) {
            //    new_I[ i ] -= delta_bl;
            // }
            alpha = 0e0;
         } else {
            do {
               last_alpha = alpha;
               this_reps++;
               I_tot = 0e0;

               // note: this "if" could be separated into 2 loops
               // removing one of the condition checks

               if ( test_alt_integral_bl ) {
                  for ( unsigned int j = 0; j < ( unsigned int ) bl_I.size(); j++ )
                  {
                     if ( f_qs[ files[ i ] ][ j ] >= start &&
                          f_qs[ files[ i ] ][ j ] <= end )
                     {
                        if ( bl_I[ j ] > bl[ j ] ) {
                           I_tot += pow( bl_I[ j ] - bl[ j ], i_power );
                        }
                     }
                  }

                  if ( I_tot > 0e0 && delta_bl > 0e0 )
                  {
                     alpha = delta_bl / I_tot;

                     if ( reps > 1 && fabs( alpha - last_alpha ) / alpha < alpha_epsilon ) {
                        break;
                     }
                     editor_msg( "dark blue", 
                                 QString( us_tr( "iteration %1 delta_Bl %2 Itot %3 gamma %4 %5%6" ) )
                                 .arg( this_reps )
                                 .arg( delta_bl )
                                 .arg( I_tot )
                                 .arg( alpha )
                                 .arg( smoothing ? QString( " smoothing %1" ).arg( smoothing ) : QString( "" ) )
                                 .arg( i_power != 1e0 ? QString( " I exponent %1" ).arg( i_power ) : QString( "" ) )
                                 );

                     vector < double > D( bl.size() );

                     for ( unsigned int j = 0; j < bl_I.size(); j++ )
                     {
                        if ( f_qs[ files[ i ] ][ j ] >= start &&
                             f_qs[ files[ i ] ][ j ] <= end )
                        {
                           if ( bl_I[ j ]  > bl[ j ] ) {
                              D[ j ] = alpha * ( pow( bl_I[ j ] - bl[ j ], i_power ) );
                           } else {
                              D[ j ] = 0;
                           }
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
                     if ( I_tot < 0e0 && delta_bl < 0e0 )
                     {
                        messages << QString( us_tr( "Warning: the overall change in baseline of %1 is negative (%2) and the integral is less than zero, so no baseline correction will be applied" ) ).arg( files[ i ] ).arg( delta_bl );
                        editor_msg( "dark red", messages.back() );
                        new_I = f_Is[ files[ i ] ];
                        alpha = 0e0;
                     } else {
                        if ( I_tot <= 0e0 && delta_bl > 0e0 )
                        {
                           messages << QString( us_tr( "Warning: the integral of %1 was less than or equal to zero, so no baseline correction will be applied" ) ).arg( files[ i ] );
                           editor_msg( "dark red", messages.back() );
                           new_I = f_Is[ files[ i ] ];
                           alpha = 0e0;
                        } else {
                           if ( delta_bl < 0e0 && I_tot > 0e0 )
                           {
                              messages << QString( us_tr( "Warning: the overall change in baseline of %1 is negative (%2), so no baseline correction will be applied" ) ).arg( files[ i ] ).arg( delta_bl );
                              editor_msg( "dark red", messages.back() );
                              new_I = f_Is[ files[ i ] ];
                              alpha = 0e0;
                           }
                        }
                     }
                  }
               } else {
                  for ( unsigned int j = 0; j < ( unsigned int ) bl_I.size(); j++ )
                  {
                     if ( f_qs[ files[ i ] ][ j ] >= start &&
                          f_qs[ files[ i ] ][ j ] <= end )
                     {
                        I_tot += bl_I[ j ] - bl[ j ];
                     }
                  }

                  if ( I_tot > 0e0 && delta_bl > 0e0 )
                  {
                     alpha = delta_bl / I_tot;

                     editor_msg( "dark blue", 
                                 QString( us_tr( "iteration %1 delta_Bl %2 Itot %3 gamma %4%5" ) )
                                 .arg( this_reps )
                                 .arg( delta_bl )
                                 .arg( I_tot )
                                 .arg( alpha )
                                 .arg( smoothing ? QString( " smoothing %1" ).arg( smoothing ) : QString( "" ) )
                                 );

                     vector < double > D( bl.size() );

                     for ( unsigned int j = 0; j < bl_I.size(); j++ )
                     {
                        if ( f_qs[ files[ i ] ][ j ] >= start &&
                             f_qs[ files[ i ] ][ j ] <= end )
                        {
                           // D[ j ] = alpha * ( f_Is[ files[ i ] ][ j ] - bl[ j ] );
                           D[ j ] = alpha * ( bl_I[ j ] - bl[ j ] );
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
                     if ( I_tot < 0e0 && delta_bl < 0e0 )
                     {
                        messages << QString( us_tr( "Warning: the overall change in baseline of %1 is negative (%2) and the integral is less than zero, so no baseline correction will be applied" ) ).arg( files[ i ] ).arg( delta_bl );
                        editor_msg( "dark red", messages.back() );
                        new_I = f_Is[ files[ i ] ];
                        alpha = 0e0;
                     } else {
                        if ( I_tot <= 0e0 && delta_bl > 0e0 )
                        {
                           messages << QString( us_tr( "Warning: the integral of %1 was less than or equal to zero, so no baseline correction will be applied" ) ).arg( files[ i ] );
                           editor_msg( "dark red", messages.back() );
                           new_I = f_Is[ files[ i ] ];
                           alpha = 0e0;
                        } else {
                           if ( delta_bl < 0e0 && I_tot > 0e0 )
                           {
                              messages << QString( us_tr( "Warning: the overall change in baseline of %1 is negative (%2), so no baseline correction will be applied" ) ).arg( files[ i ] ).arg( delta_bl );
                              editor_msg( "dark red", messages.back() );
                              new_I = f_Is[ files[ i ] ];
                              alpha = 0e0;
                           }
                        }
                     }
                  }
               }

               if ( save_bl )
               {
                  QString this_file = QString( "BI_%1-%2" ).arg( files[ i ] ).arg( this_reps );
                  baseline_apply_created.push_back( this_file );
                  add_plot( this_file, f_qs[ files[ i ] ], bl, true, false );
               }

            } while ( this_reps < reps && alpha > 0e0 && ( fabs( alpha - last_alpha ) / alpha ) >= alpha_epsilon );
         }

         bl_I = new_I;

         //          cout << QString( 
         //                          "delta_bl   %1\n"
         //                          "integral_I %2\n"
         //                           )
         //             .arg( delta_bl )
         //             .arg( integral_I )
         //             ;

         QString bl_name_add = use_i_power && test_alt_integral_bl ? QString( "-ip%1" ).arg( i_power ) : QString( "" );

         bl_name += QString( "-bi%1-%2s%3" )
            .arg( delta_bl, 0, 'g', 6 )
            .arg( alpha, 0, 'g', 6 ).replace( ".", "_" )
            .arg( bl_name_add )
            ;

         while ( current_files.count( bl_name ) ) {
            bl_name = files[ i ] + QString( "-bi%1-%2s%3-%4" )
               .arg( delta_bl, 0, 'g', 6 )
               .arg( alpha, 0, 'g', 6 )
               .arg( bl_name_add )
               .arg( ++ext ).replace( ".", "_" )
               ;
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

      lb_created_files->addItem( bl_name );
      lb_created_files->scrollToItem( lb_created_files->item( lb_created_files->count() - 1 ) );
      lb_files->addItem( bl_name );
      lb_files->scrollToItem( lb_files->item( lb_files->count() - 1 ) );
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
      editor_msg( "dark gray", QString( "Created %1\n" ).arg( bl_name ) );
      baseline_apply_created.push_back( bl_name );
   }

   if ( messages.size() && !quiet )
   {
      QStringList qsl;
      for ( int i = 0; i < (int)messages.size() && i < 15; i++ )
      {
         qsl << messages[ i ];
      }

      if ( qsl.size() < messages.size() )
      {
         qsl << QString( us_tr( "... and %1 more not listed" ) ).arg( messages.size() - qsl.size() );
      }

      QMessageBox::warning(
                           this,
                           this->windowTitle() + us_tr(": Baseline apply" ),
                           QString( us_tr( "Please note:\n\n"
                                        "These warnings were produced when performing the baseline correction\n"
                                        "%1\n"
                                        ) )
                           .arg( qsl.join( "\n" ) ),
                           QMessageBox::Ok | QMessageBox::Default,
                           QMessageBox::NoButton
                           );
   }

   if ( current_mode == MODE_BASELINE &&
        !baseline_test_mode ) {
      wheel_save();
   }

   if ( !quiet ) {
      progress->reset();
   }

   if ( do_replot ) {
      disable_updates = true;

      if ( select_files.size() != 1 )
      {
         lb_files->clearSelection();
      }

      for ( int i = 0; i < (int)lb_files->count(); i++ )
      {
         if ( select_files.count( lb_files->item( i )->text() ) )
         {
            lb_files->item( i)->setSelected( true );
         }
      }

      disable_updates = false;
      rescale();
      plot_files();
      update_enables();
   }
}

void US_Hydrodyn_Mals_Saxs::set_baseline_fix_width() {
   // us_qdebug( "set baseline fix width" );
   baseline_enables();
}

int US_Hydrodyn_Mals_Saxs::input_double_to_pos( double d ) {
   if ( wheel_double_to_pos.count( d ) ) {
      return wheel_double_to_pos[ d ];
   }
   // bisect size and find closest match

   int last_start = 0;
   int last_end   = (int) f_qs[ wheel_file ].size() - 1;

   int pos;

   do {
      pos = ( last_start + last_end ) / 2;
      if ( f_qs[ wheel_file ][ pos ] > d ) {
         last_start = pos;
      } else {
         if ( f_qs[ wheel_file ][ pos ] < d ) {
            last_end = pos;
         }
      }
   } while ( pos != last_start && pos != last_end );
   return pos;
}

void US_Hydrodyn_Mals_Saxs::baseline_start_s_text( const QString & text )
{
#if QT_VERSION < 0x040000
   plot_dist->setMarkerPos( plotted_markers[ 0 ], text.toDouble(), 0e0 );
#else
   plotted_markers[ 0 ]->setXValue( text.toDouble() );
#endif
   // if ( !wheel_is_pressed && qwtw_wheel->value() != text.toDouble() )
   // {
   //    qwtw_wheel->setValue( text.toDouble() );
   // }
   double dpos = (double) input_double_to_pos( text.toDouble() );
   if ( !wheel_is_pressed && qwtw_wheel->value() != dpos ) {
      qwtw_wheel->setValue( dpos );
   }
   replot_baseline( "baseline_start_s_text" );
   if ( !suppress_replot )
   {
      plot_dist->replot();
   }
   baseline_enables();
}

void US_Hydrodyn_Mals_Saxs::baseline_start_text( const QString & text )
{
#if QT_VERSION < 0x040000
   plot_dist->setMarkerPos( plotted_markers[ 1 ], text.toDouble(), 0e0 );
#else
   plotted_markers[ 1 ]->setXValue( text.toDouble() );
#endif
   // if ( !wheel_is_pressed && qwtw_wheel->value() != text.toDouble() )
   // {
   //    qwtw_wheel->setValue( text.toDouble() );
   // }
   double dpos = (double) input_double_to_pos( text.toDouble() );
   if ( !wheel_is_pressed && qwtw_wheel->value() != dpos ) {
      qwtw_wheel->setValue( dpos );
   }
   replot_baseline( "baseline_start_text" );
   if ( !suppress_replot )
   {
      plot_dist->replot();
   }
   baseline_enables();
}

void US_Hydrodyn_Mals_Saxs::baseline_start_e_text( const QString & text )
{
#if QT_VERSION < 0x040000
   plot_dist->setMarkerPos( plotted_markers[ 2 ], text.toDouble(), 0e0 );
#else
   plotted_markers[ 2 ]->setXValue( text.toDouble() );
#endif
   // if ( !wheel_is_pressed && qwtw_wheel->value() != text.toDouble() )
   // {
   //    qwtw_wheel->setValue( text.toDouble() );
   // }
   double dpos = (double) input_double_to_pos( text.toDouble() );
   if ( !wheel_is_pressed && qwtw_wheel->value() != dpos ) {
      qwtw_wheel->setValue( dpos );
   }
   replot_baseline( "baseline_start_e_text" );
   plot_dist->replot();
   baseline_enables();
}

void US_Hydrodyn_Mals_Saxs::baseline_end_s_text( const QString & text )
{
   int pos = cb_baseline_start_zero->isChecked() ? 0 : 3;

   if ( current_mode == MODE_BLANKS || baseline_integral ) {
      pos = 0;
   }

#if QT_VERSION < 0x040000
   plot_dist->setMarkerPos( plotted_markers[ pos ], text.toDouble(), 0e0 );
#else
   plotted_markers[ pos ]->setXValue( text.toDouble() );
#endif

   if ( current_mode != MODE_BLANKS && baseline_integral ) {
#if QT_VERSION < 0x040000
      plot_dist->setMarkerPos( plotted_markers[ pos + 1 ], text.toDouble(), 0e0 );
#else
      plotted_markers[ pos + 1 ]->setXValue( text.toDouble() );
#endif
   }

   // if ( !wheel_is_pressed && qwtw_wheel->value() != text.toDouble() )
   // {
   //    qwtw_wheel->setValue( text.toDouble() );
   // }
   double dpos = (double) input_double_to_pos( text.toDouble() );
   if ( le_baseline_end_s->hasFocus() ) {
      if ( !wheel_is_pressed && qwtw_wheel->value() != dpos ) {
         qwtw_wheel->setValue( dpos );
      }
   }
   if ( current_mode != MODE_BLANKS && baseline_integral ) {
      int pos_s = input_double_to_pos( le_baseline_end_s->text().toDouble() );
      int pos_e = input_double_to_pos( le_baseline_end_e->text().toDouble() );

      if ( cb_baseline_fix_width->isChecked() ) {
         int q_size = (int) f_qs[ wheel_file ].size();
         int blw = le_baseline_width->text().toInt();
         if ( blw > q_size - 1 ) {
            blw = q_size - 1;
            disconnect( le_baseline_width, SIGNAL( textChanged( const QString & ) ), 0, 0 );
            le_baseline_width->setText( QString( "%1" ).arg( blw ) );
            connect( le_baseline_width, SIGNAL( textChanged( const QString & ) ), SLOT( baseline_width_text( const QString & ) ) );
         }
         int new_pos_e = pos_s + blw - 1;
         int new_pos_s = pos_s;
         if ( new_pos_e >= q_size ) {
            new_pos_e = q_size - 1;
            new_pos_s = new_pos_e - blw + 1;
         }
         disconnect( le_baseline_end_e, SIGNAL( textChanged( const QString & ) ), 0, 0 );
         le_baseline_end_e->setText( QString( "%1" ).arg( f_qs[ wheel_file ][ new_pos_e ] ) );
         connect( le_baseline_end_e, SIGNAL( textChanged( const QString & ) ), SLOT( baseline_end_e_text( const QString & ) ) );
#if QT_VERSION < 0x040000
         plot_dist->setMarkerPos( plotted_markers[ 3 ], f_qs[ wheel_file ][ new_pos_e ], 0e0 );
#else
         plotted_markers[ 3 ]->setXValue(  f_qs[ wheel_file ][ new_pos_e ] );
#endif
         if ( new_pos_s != pos_s ) {
            // disconnect( le_baseline_end_s, SIGNAL( textChanged( const QString & ) ), 0, 0 );
            // le_baseline_end_s->setText( QString( "%1" ).arg( f_qs[ wheel_file ][ new_pos_s ] ) );
            // connect( le_baseline_end_s, SIGNAL( textChanged( const QString & ) ), SLOT( baseline_end_s_text( const QString & ) ) );
            if ( !wheel_is_pressed && qwtw_wheel->value() != new_pos_s ) {
               qwtw_wheel->setValue( new_pos_s );
            }
#if QT_VERSION < 0x040000
            plot_dist->setMarkerPos( plotted_markers[ pos ], f_qs[ wheel_file ][ new_pos_s ], 0e0 );
            plot_dist->setMarkerPos( plotted_markers[ pos + 1 ], f_qs[ wheel_file ][ new_pos_s ], 0e0 );
#else
            plotted_markers[ pos ]->setXValue(  f_qs[ wheel_file ][ new_pos_s ] );
            plotted_markers[ pos + 1 ]->setXValue(  f_qs[ wheel_file ][ new_pos_s ] );
#endif
         }
      } else {
         if ( pos_e - IB_MIN_RANGE + 1 < pos_s ) {
            pos_e = pos_s + IB_MIN_RANGE - 1;
            disconnect( le_baseline_end_e, SIGNAL( textChanged( const QString & ) ), 0, 0 );
            le_baseline_end_e->setText( QString( "%1" ).arg( f_qs[ wheel_file ][ pos_e ] ) );
            connect( le_baseline_end_e, SIGNAL( textChanged( const QString & ) ), SLOT( baseline_end_e_text( const QString & ) ) );
#if QT_VERSION < 0x040000
            plot_dist->setMarkerPos( plotted_markers[ 3 ], f_qs[ wheel_file ][ pos_e ], 0e0 );
#else
            plotted_markers[ 3 ]->setXValue(  f_qs[ wheel_file ][ pos_e ] );
#endif
         } else {
            if ( pos_e > pos_s + baseline_max_window_size - 1 ) {
               pos_e = pos_s + baseline_max_window_size - 1;
               disconnect( le_baseline_end_e, SIGNAL( textChanged( const QString & ) ), 0, 0 );
               le_baseline_end_e->setText( QString( "%1" ).arg( f_qs[ wheel_file ][ pos_e ] ) );
               connect( le_baseline_end_e, SIGNAL( textChanged( const QString & ) ), SLOT( baseline_end_e_text( const QString & ) ) );
#if QT_VERSION < 0x040000
               plot_dist->setMarkerPos( plotted_markers[ 3 ], f_qs[ wheel_file ][ pos_e ], 0e0 );
#else
               plotted_markers[ 3 ]->setXValue(  f_qs[ wheel_file ][ pos_e ] );
#endif
            }
         }

         le_baseline_width->setText( QString( "%1" ).arg( pos_e - pos_s + 1 ) );
      }
   }
   replot_baseline( "baseline_end_s_text" );
   if ( !suppress_replot )
   {
      plot_dist->replot();
   }
   baseline_enables();
}

void US_Hydrodyn_Mals_Saxs::baseline_end_text( const QString & text )
{
   int pos = cb_baseline_start_zero->isChecked() ? 1 : 4;
   if ( baseline_integral ) {
      pos++;
   }
#if QT_VERSION < 0x040000
   plot_dist->setMarkerPos( plotted_markers[ pos ], text.toDouble(), 0e0 );
#else
   plotted_markers[ pos ]->setXValue( text.toDouble() );
#endif
   // if ( !wheel_is_pressed && qwtw_wheel->value() != text.toDouble() )
   // {
   //    qwtw_wheel->setValue( text.toDouble() );
   // }
   double dpos = (double) input_double_to_pos( text.toDouble() );
   if ( le_baseline_end->hasFocus() ) {
      if ( !wheel_is_pressed && qwtw_wheel->value() != dpos ) {
         qwtw_wheel->setValue( dpos );
      }
   }
   replot_baseline( "baseline_end_text" );
   if ( !suppress_replot )
   {
      plot_dist->replot();
   }
   baseline_enables();
}

void US_Hydrodyn_Mals_Saxs::baseline_end_e_text( const QString & text )
{
   int pos = cb_baseline_start_zero->isChecked() ? 2 : 5;
   if ( baseline_integral ) {
      pos++;
   }
   if ( current_mode == MODE_BLANKS ) {
      pos = 1;
   }

#if QT_VERSION < 0x040000
   plot_dist->setMarkerPos( plotted_markers[ pos ], text.toDouble(), 0e0 );
#else
   plotted_markers[ pos ]->setXValue( text.toDouble() );
#endif
   // if ( !wheel_is_pressed && qwtw_wheel->value() != text.toDouble() )
   // {
   //    qwtw_wheel->setValue( text.toDouble() );
   // }
   double dpos = (double) input_double_to_pos( text.toDouble() );
   if ( le_baseline_end_e->hasFocus() ) {
      if ( !wheel_is_pressed && qwtw_wheel->value() != dpos ) {
         qwtw_wheel->setValue( dpos );
      }
   }
   if ( current_mode != MODE_BLANKS && baseline_integral ) {
      int pos_s = input_double_to_pos( le_baseline_end_s->text().toDouble() );
      int pos_e = input_double_to_pos( le_baseline_end_e->text().toDouble() );

      if ( cb_baseline_fix_width->isChecked() ) {
         int blw = le_baseline_width->text().toInt();
         if ( blw > (int) f_qs[ wheel_file ].size() - 1 ) {
            blw = f_qs[ wheel_file ].size() - 1;
            disconnect( le_baseline_width, SIGNAL( textChanged( const QString & ) ), 0, 0 );
            le_baseline_width->setText( QString( "%1" ).arg( blw ) );
            connect( le_baseline_width, SIGNAL( textChanged( const QString & ) ), SLOT( baseline_width_text( const QString & ) ) );
         }
         int new_pos_s = pos_e - blw + 1;
         int new_pos_e = pos_e;
         if ( new_pos_s < 0 ) {
            new_pos_s = 0;
            new_pos_e = blw;
         }
         disconnect( le_baseline_end_s, SIGNAL( textChanged( const QString & ) ), 0, 0 );
         le_baseline_end_s->setText( QString( "%1" ).arg( f_qs[ wheel_file ][ new_pos_s ] ) );
         connect( le_baseline_end_s, SIGNAL( textChanged( const QString & ) ), SLOT( baseline_end_s_text( const QString & ) ) );
#if QT_VERSION < 0x040000
         plot_dist->setMarkerPos( plotted_markers[ 0 ], f_qs[ wheel_file ][ new_pos_s ], 0e0 );
         plot_dist->setMarkerPos( plotted_markers[ 1 ], f_qs[ wheel_file ][ new_pos_s ], 0e0 );
#else
         plotted_markers[ 0 ]->setXValue(  f_qs[ wheel_file ][ new_pos_s ] );
         plotted_markers[ 1 ]->setXValue(  f_qs[ wheel_file ][ new_pos_s ] );
#endif
         if ( new_pos_e != pos_e ) {
            // disconnect( le_baseline_end_e, SIGNAL( textChanged( const QString & ) ), 0, 0 );
            // le_baseline_end_e->setText( QString( "%1" ).arg( f_qs[ wheel_file ][ new_pos_e ] ) );
            // connect( le_baseline_end_e, SIGNAL( textChanged( const QString & ) ), SLOT( baseline_end_s_text( const QString & ) ) );
            if ( !wheel_is_pressed && qwtw_wheel->value() != new_pos_e ) {
               qwtw_wheel->setValue( new_pos_e );
            }
#if QT_VERSION < 0x040000
            plot_dist->setMarkerPos( plotted_markers[ pos ], f_qs[ wheel_file ][ new_pos_e ], 0e0 );
#else
            plotted_markers[ pos ]->setXValue(  f_qs[ wheel_file ][ new_pos_e ] );
#endif
         }
      } else {
         if ( pos_s > pos_e - IB_MIN_RANGE + 1 ) {
            pos_s = pos_e - IB_MIN_RANGE + 1;
            disconnect( le_baseline_end_s, SIGNAL( textChanged( const QString & ) ), 0, 0 );
            le_baseline_end_s->setText( QString( "%1" ).arg( f_qs[ wheel_file ][ pos_s ] ) );
            connect( le_baseline_end_s, SIGNAL( textChanged( const QString & ) ), SLOT( baseline_end_s_text( const QString & ) ) );
#if QT_VERSION < 0x040000
            plot_dist->setMarkerPos( plotted_markers[ 0 ], f_qs[ wheel_file ][ pos_s ], 0e0 );
            plot_dist->setMarkerPos( plotted_markers[ 1 ], f_qs[ wheel_file ][ pos_s ], 0e0 );
#else
            plotted_markers[ 0 ]->setXValue(  f_qs[ wheel_file ][ pos_s ] );
            plotted_markers[ 1 ]->setXValue(  f_qs[ wheel_file ][ pos_s ] );
#endif
         } else {
            if ( pos_s < pos_e - baseline_max_window_size + 1 ) {
               pos_s = pos_e - baseline_max_window_size + 1;
               disconnect( le_baseline_end_s, SIGNAL( textChanged( const QString & ) ), 0, 0 );
               le_baseline_end_s->setText( QString( "%1" ).arg( f_qs[ wheel_file ][ pos_s ] ) );
               connect( le_baseline_end_s, SIGNAL( textChanged( const QString & ) ), SLOT( baseline_end_s_text( const QString & ) ) );
#if QT_VERSION < 0x040000
               plot_dist->setMarkerPos( plotted_markers[ 0 ], f_qs[ wheel_file ][ pos_s ], 0e0 );
               plot_dist->setMarkerPos( plotted_markers[ 1 ], f_qs[ wheel_file ][ pos_s ], 0e0 );
#else
               plotted_markers[ 0 ]->setXValue(  f_qs[ wheel_file ][ pos_s ] );
               plotted_markers[ 1 ]->setXValue(  f_qs[ wheel_file ][ pos_s ] );
#endif
            }
         }
            
         le_baseline_width->setText( QString( "%1" ).arg( pos_e - pos_s + 1 ) );
      }
   }
   replot_baseline( "baseline_end_e_text" );
   if ( !suppress_replot )
   {
      plot_dist->replot();
   }
   baseline_enables();
}

void US_Hydrodyn_Mals_Saxs::baseline_start_s_focus( bool hasFocus )
{
   // cout << QString( "baseline_start_s_focus %1\n" ).arg( hasFocus ? "true" : "false" );
   if ( hasFocus )
   {
      le_last_focus = le_baseline_start_s;
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      // qwtw_wheel->setRange( f_qs[ wheel_file ][ 0 ], f_qs[ wheel_file ].back(),
      //                       ( f_qs[ wheel_file ].back() - f_qs[ wheel_file ][ 0 ] ) / UHSH_WHEEL_RES );
      qwtw_wheel->setRange( 0, (double) f_qs[ wheel_file ].size() - 1); qwtw_wheel->setSingleStep( 1 );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      // qwtw_wheel->setValue( le_baseline_start_s->text().toDouble() );
      qwtw_wheel->setValue( (double) input_double_to_pos( le_baseline_start_s->text().toDouble() ) );
      wheel_enables();
   }
}

void US_Hydrodyn_Mals_Saxs::baseline_start_focus( bool hasFocus )
{
   // cout << QString( "baseline_start_focus %1\n" ).arg( hasFocus ? "true" : "false" );
   if ( hasFocus )
   {
      le_last_focus = le_baseline_start;
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      // qwtw_wheel->setRange( f_qs[ wheel_file ][ 0 ], f_qs[ wheel_file ].back(),
      //                       ( f_qs[ wheel_file ].back() - f_qs[ wheel_file ][ 0 ] ) / UHSH_WHEEL_RES );
      qwtw_wheel->setRange( 0, (double) f_qs[ wheel_file ].size() - 1); qwtw_wheel->setSingleStep( 1 );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      // qwtw_wheel->setValue( le_baseline_start->text().toDouble() );
      qwtw_wheel->setValue( (double) input_double_to_pos( le_baseline_start->text().toDouble() ) );
      wheel_enables();
   }
}

void US_Hydrodyn_Mals_Saxs::baseline_start_e_focus( bool hasFocus )
{
   // cout << QString( "baseline_start_e_focus %1\n" ).arg( hasFocus ? "true" : "false" );
   if ( hasFocus )
   {
      le_last_focus = le_baseline_start_e;
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      // qwtw_wheel->setRange( f_qs[ wheel_file ][ 0 ], f_qs[ wheel_file ].back(),
      //                       ( f_qs[ wheel_file ].back() - f_qs[ wheel_file ][ 0 ] ) / UHSH_WHEEL_RES );
      qwtw_wheel->setRange( 0, (double) f_qs[ wheel_file ].size() - 1); qwtw_wheel->setSingleStep( 1 );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      // qwtw_wheel->setValue( le_baseline_start_e->text().toDouble() );
      qwtw_wheel->setValue( (double) input_double_to_pos( le_baseline_start_e->text().toDouble() ) );
      wheel_enables();
   }
}

void US_Hydrodyn_Mals_Saxs::baseline_end_s_focus( bool hasFocus )
{
   // cout << QString( "baseline_end_s_focus %1\n" ).arg( hasFocus ? "true" : "false" );
   if ( hasFocus )
   {
      le_last_focus = le_baseline_end_s;
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      // qwtw_wheel->setRange( f_qs[ wheel_file ][ 0 ],
      //                       f_qs[ wheel_file ].back(), 
      //                       ( f_qs[ wheel_file ].back() - f_qs[ wheel_file ][ 0 ] ) / UHSH_WHEEL_RES );
      qwtw_wheel->setRange( 0, (double) f_qs[ wheel_file ].size() - 1); qwtw_wheel->setSingleStep( 1 );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      // qwtw_wheel->setValue( le_baseline_end_s->text().toDouble() );
      qwtw_wheel->setValue( (double) input_double_to_pos( le_baseline_end_s->text().toDouble() ) );
      wheel_enables();
   }
}

void US_Hydrodyn_Mals_Saxs::baseline_end_focus( bool hasFocus )
{
   // cout << QString( "baseline_end_focus %1\n" ).arg( hasFocus ? "true" : "false" );
   if ( hasFocus )
   {
      le_last_focus = le_baseline_end;
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      // qwtw_wheel->setRange( f_qs[ wheel_file ][ 0 ],
      //                       f_qs[ wheel_file ].back(), 
      //                       ( f_qs[ wheel_file ].back() - f_qs[ wheel_file ][ 0 ] ) / UHSH_WHEEL_RES );
      qwtw_wheel->setRange( 0, (double) f_qs[ wheel_file ].size() - 1); qwtw_wheel->setSingleStep( 1 );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      // qwtw_wheel->setValue( le_baseline_end->text().toDouble() );
      qwtw_wheel->setValue( (double) input_double_to_pos( le_baseline_end->text().toDouble() ) );
      wheel_enables();
   }
}

void US_Hydrodyn_Mals_Saxs::baseline_end_e_focus( bool hasFocus )
{
   // cout << QString( "baseline_end_e_focus %1\n" ).arg( hasFocus ? "true" : "false" );
   if ( hasFocus )
   {
      le_last_focus = le_baseline_end_e;
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      // qwtw_wheel->setRange( f_qs[ wheel_file ][ 0 ],
      //                       f_qs[ wheel_file ].back(), 
      //                       ( f_qs[ wheel_file ].back() - f_qs[ wheel_file ][ 0 ] ) / UHSH_WHEEL_RES );
      qwtw_wheel->setRange( 0, (double) f_qs[ wheel_file ].size() - 1); qwtw_wheel->setSingleStep( 1 );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      // qwtw_wheel->setValue( le_baseline_end_e->text().toDouble() );
      qwtw_wheel->setValue( (double) input_double_to_pos( le_baseline_end_e->text().toDouble() ) );
      wheel_enables();
   }
}

void US_Hydrodyn_Mals_Saxs::baseline_width_text( const QString & text )
{
   int blw = text.toInt();
   // us_qdebug( QString( "baseline_width_text %1 %2" ).arg( blw ).arg( le_baseline_width->hasFocus() ? "hasFocus" : "!hasFocus" ) );
   if ( blw < 1 ) {
      le_baseline_width->setText( "1" );
      return;
   }

   if ( le_baseline_width->hasFocus() ) {
      if ( !wheel_is_pressed && qwtw_wheel->value() != (double) blw ) {
         qwtw_wheel->setValue( (double) blw );
      }

      if ( current_mode != MODE_BLANKS && baseline_integral ) {
         int pos_s = input_double_to_pos( le_baseline_end_s->text().toDouble() );

         int q_size = (int) f_qs[ wheel_file ].size();
         if ( blw > q_size - 1 ) {
            blw = q_size - 1;
            disconnect( le_baseline_width, SIGNAL( textChanged( const QString & ) ), 0, 0 );
            le_baseline_width->setText( QString( "%1" ).arg( blw ) );
            connect( le_baseline_width, SIGNAL( textChanged( const QString & ) ), SLOT( baseline_width_text( const QString & ) ) );
         }
         int new_pos_e = pos_s + blw - 1;
         int new_pos_s = pos_s;
         if ( new_pos_e >= q_size ) {
            new_pos_e = q_size - 1;
            new_pos_s = new_pos_e - blw + 1;
         }
         // us_qdebug( QString( "baseline width new pos s %1, e %2" ).arg( new_pos_s ).arg( new_pos_e ) );
         disconnect( le_baseline_end_e, SIGNAL( textChanged( const QString & ) ), 0, 0 );
         le_baseline_end_e->setText( QString( "%1" ).arg( f_qs[ wheel_file ][ new_pos_e ] ) );
         connect( le_baseline_end_e, SIGNAL( textChanged( const QString & ) ), SLOT( baseline_end_e_text( const QString & ) ) );
#if QT_VERSION < 0x040000
         plot_dist->setMarkerPos( plotted_markers[ 3 ], f_qs[ wheel_file ][ new_pos_e ], 0e0 );
#else
         plotted_markers[ 3 ]->setXValue(  f_qs[ wheel_file ][ new_pos_e ] );
#endif
         if ( new_pos_s != pos_s ) {
            disconnect( le_baseline_end_s, SIGNAL( textChanged( const QString & ) ), 0, 0 );
            le_baseline_end_s->setText( QString( "%1" ).arg( f_qs[ wheel_file ][ new_pos_s ] ) );
            connect( le_baseline_end_s, SIGNAL( textChanged( const QString & ) ), SLOT( baseline_end_s_text( const QString & ) ) );
#if QT_VERSION < 0x040000
            plot_dist->setMarkerPos( plotted_markers[ 0 ], f_qs[ wheel_file ][ new_pos_s ], 0e0 );
            plot_dist->setMarkerPos( plotted_markers[ 1 ], f_qs[ wheel_file ][ new_pos_s ], 0e0 );
#else
            plotted_markers[ 0 ]->setXValue(  f_qs[ wheel_file ][ new_pos_s ] );
            plotted_markers[ 1 ]->setXValue(  f_qs[ wheel_file ][ new_pos_s ] );
#endif
         }
      }
   }
   replot_baseline( "baseline_width_text" );
   if ( !suppress_replot )
   {
      plot_dist->replot();
   }
   baseline_enables();
}

void US_Hydrodyn_Mals_Saxs::baseline_width_focus( bool hasFocus )
{
   if ( hasFocus ) {
      le_last_focus = le_baseline_width;
      // us_qdebug( QString( "baseline_width focus blw currently %1" ).arg( le_baseline_width->text() ) );
      disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      qwtw_wheel->setRange( IB_MIN_RANGE, baseline_max_window_size - 1); qwtw_wheel->setSingleStep( 1 );
      connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setValue( le_baseline_width->text().toDouble() );
      wheel_enables();
   }
}

void US_Hydrodyn_Mals_Saxs::baseline_best() {
   disable_all();

   // us_qdebug( "baseline best" );
   // run cormap on full system

   map < QString, QString > parameters;

   if ( !baseline_hb_only_mode ) {
      if (
          blanks_cormap_parameters.count( "decimate" ) &&
          blanks_cormap_parameters[ "decimate" ] != "0" &&
          blanks_cormap_parameters[ "decimate" ] != "1" 
          ) {
         parameters[ "decimate" ] = blanks_cormap_parameters[ "decimate" ];
      } else {
         parameters[ "decimate" ] = "1";
      }
   }

   bool hb_mode = false;

   if ( blanks_cormap_parameters.count( "hb" ) ||
        baseline_hb_only_mode ) {
      parameters[ "hb" ] = "true";
      hb_mode = true;
   }

   if ( blanks_cormap_parameters.count( "alpha" ) ) {
      // us_qdebug( QString( "got alpha %1 from blanks" ).arg( blanks_cormap_parameters[ "alpha" ] ) );
      parameters[ "alpha" ] = blanks_cormap_parameters[ "alpha" ];
   }

   // if ( baseline_hb_only_mode ) {
   //    ask_to_decimate( parameters );
   // }

   parameters[ "baseline_use_end" ] = "true";
   // parameters[ "close"            ] = "true";

   // if cormap start/end is the same, maybe don't redo

   cormap( parameters );

   baseline_cormap_parameters  = baseline_last_cormap_parameters;
   baseline_cormap_pvaluepairs = baseline_last_cormap_pvaluepairs;
   baseline_cormap_run_end_s   = baseline_last_cormap_run_end_s;
   baseline_cormap_run_end_e   = baseline_last_cormap_run_end_e;

   baseline_best_last_window_size = le_baseline_width->text().toInt();

   baseline_last_cormap_parameters .clear( );
   baseline_last_cormap_pvaluepairs.clear( );

   // do sliding analysis of each region and from end_s to end with window and plot avg red cluster size

   map < QString, double > baseline_hb_brookesmap_sliding_results;

   if ( baseline_cormap_parameters.size() &&
        baseline_cormap_pvaluepairs.size() ) {
      US_Hydrodyn_Saxs_Cormap_Cluster_Analysis ca;
      baseline_cormap_parameters[ "sliding_minimum_size"      ] = le_baseline_width->text();
      baseline_cormap_parameters[ "sliding_maximum_size"      ] = le_baseline_width->text();
      baseline_cormap_parameters[ "sliding_baseline_mode"     ] = "true";
      baseline_cormap_parameters[ "sliding_baseline_pos_base" ] = QString( "%1" ).arg( input_double_to_pos ( baseline_last_cormap_run_end_s ) );
      
      if ( ca.sliding( 
                      baseline_cormap_pvaluepairs, 
                      baseline_cormap_parameters, 
                      baseline_brookesmap_sliding_results, 
                      baseline_hb_brookesmap_sliding_results, 
                      this,
                      progress
                      ) ) {
         // us_qdebug( "sliding_results" );
         // for ( map < QString, double >::iterator it = baseline_brookesmap_sliding_results.begin();
         //       it != baseline_brookesmap_sliding_results.end();
         //       ++it ) {
         //    cout << "\t" << it->first << " => " << it->second << endl;
         // }
         // plot of results
         {
            map < QString, QString >                best_parameters;
            map < QString, double >                 best_dparameters;
            map < QString, vector < double > >      best_vdparameters;

            if ( !hb_mode ) {
               int     width = le_baseline_width->text().toInt();
               QString tag   = QString( "%1:Red cluster size average" ).arg( width );
               if ( blanks_brookesmap_sliding_results.count( tag ) ) {
                  best_dparameters[ "blanksaverage" ] = blanks_brookesmap_sliding_results[ tag ];
                  QString tagsd = QString( "%1 sd:Red cluster size average" ).arg( width );
                  if ( blanks_brookesmap_sliding_results.count( tagsd ) ) {
                     best_dparameters[ "blanksaveragesd" ] = blanks_brookesmap_sliding_results[ tagsd ];
                  }
               }
            }

            double cormap_maxq = ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "mals_saxs_cormap_maxq" ) ?
               ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_saxs_cormap_maxq" ].toDouble() : 0.05;

            {
               QString head = qstring_common_head( baseline_selected, true );
               QString tail = qstring_common_tail( baseline_selected, true );
               head = head.replace( QRegExp( "_q0_" ), "" );
               best_parameters[ "name" ] = QString( "%1%2" )
                  .arg( head )
                  .arg( tail )
                  ;

                  // .arg( baseline_selected.size() )
            }

            best_dparameters[ "width"  ] = baseline_best_last_window_size;
            best_parameters[ "xlegend" ] = QString( us_tr( "Start Frame (window width %1 frames)" ) ).arg( baseline_best_last_window_size );
            best_parameters[ "title"   ] = QString( us_tr( 
                                                       hb_mode 
                                                       ? "Baseline avg. I(q) and red pair % by start frame (window width %1 frames)." 
                                                       : "Baseline avg. I(q) and average red cluster size by start frame\n(window width %1 frames)." 
                                                        ) ).arg( baseline_best_last_window_size );

            if ( !hb_mode ) {
               best_parameters[ "hb_title"   ] = QString( us_tr( "Baseline avg. I(q) and red pair % by start frame\n(window width %1 frames)." ) ).arg( baseline_best_last_window_size );
            }

            if ( baseline_cormap_parameters.count( "decimate" ) ) {
               int decimate = baseline_cormap_parameters[ "decimate" ].toInt();
               if ( decimate > 1 ) {
                  QString nth_tag;
                  switch ( decimate ) {
                  case 2 : nth_tag = "nd"; break;
                  case 3 : nth_tag = "rd"; break;
                  default : nth_tag = "th"; break;
                  }
               
                  best_parameters[ "title" ] += QString( us_tr( " Only every %1%2 q value selected." ) )
                     .arg( decimate ).arg( nth_tag );
                  best_parameters[ "hb_title" ] += QString( us_tr( " Only every %1%2 q value selected." ) )
                     .arg( decimate ).arg( nth_tag );
               }
            }
            
            QRegExp rx_cap( 
                           hb_mode 
                           ? "^(\\d+):% red pairs$"
                           : "^(\\d+):Red cluster size average$"
                            );
            
            int q_size = (int) f_qs[ wheel_file ].size();

            map < double, double > plotdata;
            map < double, double > hb_plotdata;

            for ( map < QString, double >::iterator it = baseline_brookesmap_sliding_results.begin();
                  it != baseline_brookesmap_sliding_results.end();
                  ++it ) {
               if ( rx_cap.indexIn( it->first ) != -1 ) {
                  int pos = rx_cap.cap( 1 ).toInt();
                  if ( pos > 0 && pos < q_size ) {
                     plotdata[ f_qs[ wheel_file ][ pos ] ] = it->second;
                  }
               }
            }

            if ( !hb_mode ) {
               QRegExp rx_cap( "^(\\d+):% red pairs$" );
            
               for ( map < QString, double >::iterator it = baseline_hb_brookesmap_sliding_results.begin();
                     it != baseline_hb_brookesmap_sliding_results.end();
                     ++it ) {
                  if ( rx_cap.indexIn( it->first ) != -1 ) {
                     int pos = rx_cap.cap( 1 ).toInt();
                     if ( pos > 0 && pos < q_size ) {
                        hb_plotdata[ f_qs[ wheel_file ][ pos ] ] = it->second;
                     }
                  }
               }
            }
               
            {
               vector < double > x;
               vector < double > y;
               vector < double > gx;
               vector < double > gy;

               double ybest     = 1e99;
               double ybestposx = -1;

               vector < double > hb_x;
               vector < double > hb_y;

               double hb_ybest     = 1e99;
               double hb_ybestposx = -1;

               double green_limit = best_dparameters.count( "blanksaverage" ) ? 
                  best_dparameters[ "blanksaverage" ] : 1e0;
               if ( best_dparameters.count(  "blanksaveragesd" ) ) {
                  green_limit += best_dparameters[ "blanksaveragesd" ];
               }
               if ( hb_mode ) {
                  green_limit = 0e0;
               }
               
               for ( map < double, double >::iterator it = plotdata.begin();
                     it != plotdata.end();
                     ++it ) {
                  if ( ybest > it->second ) {
                     ybest     = it->second;
                     ybestposx = it->first;
                  }
               }

               if ( ybestposx != -1 ) {
                  vector < double > yx;
                  vector < double > yy;
                  for ( map < double, double >::iterator it = plotdata.begin();
                        it != plotdata.end();
                        ++it ) {
                     if ( ybest == it->second ) {
                        yx.push_back( it->first );
                        yy.push_back( it->second );
                        best_vdparameters[ "bestpos" ].push_back( it->first );
                     }
                  }
                  if ( ybest <= green_limit ) {
                     best_vdparameters[ "wx" ] = yx;
                     best_vdparameters[ "wy" ] = yy;
                  } else {
                     best_vdparameters[ "yx" ] = yx;
                     best_vdparameters[ "yy" ] = yy;
                  }
               }

               for ( map < double, double >::iterator it = plotdata.begin();
                     it != plotdata.end();
                     ++it ) {
                  x.push_back( it->first );
                  y.push_back( it->second );
                  if ( it->second <= green_limit &&
                       !hb_mode ) {
                     gx.push_back( it->first );
                     gy.push_back( it->second );
                  }
               }

               // us_qdebug( QString( "ybest %1 pos %2" ).arg( ybest ).arg( ybestposx ) );

               // US_Vector::printvector2( "sliding analysis data", x, y );
               // US_Vector::printvector2( "sliding analysis gy results", gx, gy );

               best_vdparameters[ "x" ] = x;
               best_vdparameters[ "y" ] = y;

               if ( gx.size() ) {
                  best_vdparameters[ "gx" ] = gx;
                  best_vdparameters[ "gy" ] = gy;
               }

               // us_qdebug( QString( "ybest %1 pos %2" ).arg( ybest ).arg( ybestposx ) );
               // if ( ybestposx != -1 ) {
               //    best_dparameters[ "bestpos" ] = ybestposx;
               //    us_qdebug( "set as best_dparameters" );
               // }

               if ( !hb_mode ) {
                  for ( map < double, double >::iterator it = hb_plotdata.begin();
                        it != hb_plotdata.end();
                        ++it ) {
                     if ( hb_ybest > it->second ) {
                        hb_ybest     = it->second;
                        hb_ybestposx = it->first;
                     }
                  }

                  if ( hb_ybestposx != -1 ) {
                     vector < double > hb_yx;
                     vector < double > hb_yy;
                     for ( map < double, double >::iterator it = hb_plotdata.begin();
                           it != hb_plotdata.end();
                           ++it ) {
                        if ( hb_ybest == it->second ) {
                           hb_yx.push_back( it->first );
                           hb_yy.push_back( it->second );
                           best_vdparameters[ "hb_bestpos" ].push_back( it->first );
                        }
                     }
                     best_vdparameters[ "hb_wx" ] = hb_yx;
                     best_vdparameters[ "hb_wy" ] = hb_yy;

                  }

                  for ( map < double, double >::iterator it = hb_plotdata.begin();
                        it != hb_plotdata.end();
                        ++it ) {
                     hb_x.push_back( it->first );
                     hb_y.push_back( it->second );
                  }

                  // us_qdebug( QString( "ybest %1 pos %2" ).arg( ybest ).arg( ybestposx ) );

                  // US_Vector::printvector2( "sliding analysis data", x, y );
                  // US_Vector::printvector2( "sliding analysis gy results", gx, gy );

                  best_vdparameters[ "hb_x" ] = hb_x;
                  best_vdparameters[ "hb_y" ] = hb_y;

                  // us_qdebug( QString( "ybest %1 pos %2" ).arg( hb_ybest ).arg( hb_ybestposx ) );
                  // if ( hb_ybestposx != -1 ) {
                  //    best_dparameters[ "hb_bestpos" ] = hb_ybestposx;
                  //    us_qdebug( "set as best_dparameters" );
                  // }
               }
            }

            if ( true ) { // new way
               // compute sd weighted I(q)  average value for each frame group

               // sum up q for each frame in anaysis range

               map < int, double > msumq;
               map < int, double > msum2q;
               map < int, double > msumsdinv2;
               map < int, double > msumqmaxq;
               map < int, double > msum2qmaxq;
               map < int, double > msumsdinv2maxq;
               map < int, int >    pts;
               map < int, int >    ptsmaxq;

               double cormap_maxq = ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "mals_saxs_cormap_maxq" ) ?
                  ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_saxs_cormap_maxq" ].toDouble() : 0.05;

               QRegExp rx_capqv( "_It_q(\\d+_\\d+)" );

               bool use_errors = true;
               for ( int i = 0; i < (int) baseline_selected.size(); ++i ) {
                  if ( !f_errors.count( baseline_selected[ i ] ) ||
                       f_errors[ baseline_selected[ i ] ].size() != f_Is[ baseline_selected[ i ] ].size() ||
                       !is_nonzero_vector( f_errors[ baseline_selected[ i ] ] ) ) {
                     use_errors = false;
                     break;
                  }
               }

               double sdinv2;

               if ( use_errors ) {
                  for ( int i = 0; i < (int) baseline_selected.size(); ++i ) {
                     if ( rx_capqv.indexIn( baseline_selected[ i ] ) == -1 ) {
                        editor_msg( "red", 
                                    QString( us_tr( "Baseline Find best region: Could not extract q value from file name %1" ) )
                                    .arg( baseline_selected[ i ] ) );
                        baseline_enables();
                        return;
                     }
                     double qv = rx_capqv.cap( 1 ).replace( "_", "." ).toDouble();

                     for ( int j = 0; j < (int) f_qs[ baseline_selected[ i ] ].size(); ++j ) {
                        sdinv2 = 1e0 / f_errors[ baseline_selected[ i ] ][ j ];
                        sdinv2 *= sdinv2;
                        msumsdinv2[ j ] += sdinv2;
                        msumq     [ j ] += f_Is[ baseline_selected[ i ] ][ j ] * sdinv2;
                        pts       [ j ]++;
                        if ( qv <= cormap_maxq ) {
                           msumsdinv2maxq[ j ] += sdinv2;
                           msumqmaxq     [ j ] += f_Is[ baseline_selected[ i ] ][ j ] * sdinv2;
                           ptsmaxq       [ j ]++;
                        }
                     }
                  }
                  for ( map < int, double >::iterator it = msumq.begin();
                        it != msumq.end();
                        ++it ) {
                     msumq[ it->first ] /= msumsdinv2[ it->first ];
                  }

                  for ( map < int, double >::iterator it = msumqmaxq.begin();
                        it != msumqmaxq.end();
                        ++it ) {
                     msumqmaxq[ it->first ] /= msumsdinv2maxq[ it->first ];
                  }
                  // now msumq[ j ] and msumqmaxq[ j ] contains sd weighted xbar for each frame
               } else {
                  for ( int i = 0; i < (int) baseline_selected.size(); ++i ) {
                     if ( rx_capqv.indexIn( baseline_selected[ i ] ) == -1 ) {
                        editor_msg( "red", 
                                    QString( us_tr( "Baseline Find best region: Could not extract q value from file name %1" ) )
                                    .arg( baseline_selected[ i ] ) );
                        baseline_enables();
                        return;
                     }
                     double qv = rx_capqv.cap( 1 ).replace( "_", "." ).toDouble();

                     for ( int j = 0; j < (int) f_qs[ baseline_selected[ i ] ].size(); ++j ) {
                        msumq [ j ] += f_Is[ baseline_selected[ i ] ][ j ];
                        msum2q[ j ] += f_Is[ baseline_selected[ i ] ][ j ] * f_Is[ baseline_selected[ i ] ][ j ];
                        pts   [ j ]++;
                        if ( qv <= cormap_maxq ) {
                           msumqmaxq [ j ] += f_Is[ baseline_selected[ i ] ][ j ];
                           msum2qmaxq[ j ] += f_Is[ baseline_selected[ i ] ][ j ] * f_Is[ baseline_selected[ i ] ][ j ];
                           ptsmaxq   [ j ]++;
                        }
                     }
                  }
                  for ( map < int, double >::iterator it = msumq.begin();
                        it != msumq.end();
                        ++it ) {
                     msumq[ it->first ] /= (double) pts[ it->first ];
                  }

                  for ( map < int, double >::iterator it = msumqmaxq.begin();
                        it != msumqmaxq.end();
                        ++it ) {
                     msumqmaxq[ it->first ] /= (double) ptsmaxq[ it->first ];
                  }
                  // now msumq[ j ] and msumqmaxq[ j ] contain the average intensity
               }                  

               // now sum groups

               double minsumqmaxqy   = 1e99;
               double minsumqmaxqysd = 0e0;

               map < int, double > msumqx;
               map < int, double > msumqy;
               map < int, double > msum2qy;
               map < int, double > msumqysd;
               map < int, double > msumqmaxqy;
               map < int, double > msum2qmaxqy;
               map < int, double > msumqmaxqysd;

               for ( map < QString, double >::iterator it = baseline_brookesmap_sliding_results.begin();
                     it != baseline_brookesmap_sliding_results.end();
                     ++it ) {
                  if ( rx_cap.indexIn( it->first ) != -1 ) {
                     int pos = rx_cap.cap( 1 ).toInt();
                     msumqx     [ pos ] = f_qs[ wheel_file ][ pos ];
                     msumqy     [ pos ] = 0e0;
                     msum2qy    [ pos ] = 0e0;
                     msumqmaxqy [ pos ] = 0e0;
                     msum2qmaxqy[ pos ] = 0e0;
                     for ( int i = 0; i <  baseline_best_last_window_size ; ++i ) {
                        int upos = i + pos;
                        if ( upos < q_size ) {
                           msumqy     [ pos ] += msumq    [ upos ];
                           msum2qy    [ pos ] += msumq    [ upos ] * msumq    [ upos ];
                           msumqmaxqy [ pos ] += msumqmaxq[ upos ];
                           msum2qmaxqy[ pos ] += msumqmaxq[ upos ] * msumqmaxq[ upos ];
                        }
                     }
                     double countinv   = 1e0 / (double) baseline_best_last_window_size;
                     if ( baseline_best_last_window_size > 1 ) {
                        double countm1inv = 1e0 / ((double) baseline_best_last_window_size - 1 );
                        msumqysd    [ pos ] = sqrt( countm1inv * ( msum2qy    [ pos ] - countinv * msumqy    [ pos ] * msumqy    [ pos ] ) );
                        msumqmaxqysd[ pos ] = sqrt( countm1inv * ( msum2qmaxqy[ pos ] - countinv * msumqmaxqy[ pos ] * msumqmaxqy[ pos ] ) );
                     }
                     msumqy    [ pos ] *= countinv;
                     msumqmaxqy[ pos ] *= countinv;
                     if ( minsumqmaxqy > msumqmaxqy[ pos ] ) {
                        minsumqmaxqy    = msumqmaxqy[ pos ];
                        minsumqmaxqysd  = msumqmaxqysd.count( pos ) ? msumqmaxqysd[ pos ] : 0e0;
                     }
                  }
               }

               vector < double > sumqx;
               vector < double > sumqy;
               vector < double > sumqysdp;
               vector < double > sumqysdm;
               vector < double > sumqmaxqy;
               vector < double > sumqmaxqysdp;
               vector < double > sumqmaxqysdm;

               double sd;
               double sdmaxq;

               for ( map < int, double >::iterator it = msumqx.begin();
                     it != msumqx.end();
                     ++it ) {
                  sumqx    .push_back( it->second );
                  sumqy    .push_back( msumqy    [ it->first ] );
                  sumqmaxqy.push_back( msumqmaxqy[ it->first ] );
                  
                  sd     = msumqysd    .count( it->first ) ? msumqysd    [ it->first ] : 0e0;
                  sdmaxq = msumqmaxqysd.count( it->first ) ? msumqmaxqysd[ it->first ] : 0e0;

                  sumqysdm .push_back( msumqy    [ it->first ] - sd );
                  sumqysdp .push_back( msumqy    [ it->first ] + sd );

                  sumqmaxqysdm .push_back( msumqmaxqy    [ it->first ] - sdmaxq );
                  sumqmaxqysdp .push_back( msumqmaxqy    [ it->first ] + sdmaxq );
               }

               best_dparameters[ "minsumqmaxqy"   ] = minsumqmaxqy;
               best_dparameters[ "minsumqmaxqysd" ] = minsumqmaxqysd;

               for ( int i = 0; i < (int) sumqx.size(); ++i ) {
                  if ( minsumqmaxqy == sumqmaxqy[ i ] ) {
                     best_vdparameters[ "bestsumqmaxqypos" ].push_back( sumqx[ i ] );
                  }
               }

               if ( blanks_cormap_parameters.count( "blanks_avg_maxq_sd" ) ) {
                  best_dparameters[ "blanks_avg_maxq_sd" ] = blanks_cormap_parameters[ "blanks_avg_maxq_sd" ].toDouble();
               }

               best_vdparameters[ "sumqx"        ] = sumqx;
               // best_vdparameters[ "sumqy"     ] = sumqy;
               best_vdparameters[ "sumqmaxqy"    ] = sumqmaxqy;
               best_vdparameters[ "sumqmaxqysdm" ] = sumqmaxqysdm;
               best_vdparameters[ "sumqmaxqysdp" ] = sumqmaxqysdp;

               // for ( map < int, double >::iterator it = msumq.begin();
               //       it != msumq.end();
               //       ++it ) {
               //    cout << QString( "msumq %1 %2" ).arg( f_qs[ wheel_file ][ it->first ] ).arg( it->second );
               //    if ( msumqmaxq.count( it->first ) ) {
               //       cout << QString( " msumqmaxq %1" ).arg( msumqmaxq[ it->first ] );
               //    }
               //    cout << endl;
               // }
               // US_Vector::printvector3( "sumqx, sumqy, sumqmaxqy", sumqx, sumqy, sumqmaxqy );
            }                        

            best_parameters[ "msg" ] = "~~_darkblue_~~";
            
            if ( hb_mode ) {
               best_parameters[ "msg" ] += "Holm-Bonferroni adjustment is active\n";
               best_parameters[ "hb"  ] = "true";
            }
               
            // store common strings in tmp_msg
            QString tmp_msg;

            if ( blanks_cormap_parameters.count( "alpha" ) ) {
               tmp_msg += QString( "Alpha is %1\n\n" ).arg( blanks_cormap_parameters[ "alpha" ].toDouble() * 0.2 );
            }

            if ( true ) {
               if ( best_dparameters.count( "minsumqmaxqy" ) ) {
                  if ( false && best_dparameters.count( "minsumqmaxqysd" ) ) {
                     if ( best_dparameters[ "minsumqmaxqy" ] > best_dparameters[ "minsumqmaxqysd" ] ) {
                        if ( best_dparameters[ "minsumqmaxqy" ] > 5 * best_dparameters[ "minsumqmaxqysd" ] ) {
                           tmp_msg +=
                              us_tr( "Integral baseline correction is recommended\n\n" );
                        } else {
                           if ( best_dparameters[ "minsumqmaxqy" ] > 3 * best_dparameters[ "minsumqmaxqysd" ] ) {
                              tmp_msg +=
                                 us_tr( "Integral baseline correction is of borderline need\n\n" );
                           } else {
                              tmp_msg +=
                                 us_tr( "Integral baseline correction is of questionable use\n\n" );
                           }
                        }
                     } else {
                        if ( best_dparameters[ "minsumqmaxqy" ] < -5 * best_dparameters[ "minsumqmaxqysd" ] ) {
                           tmp_msg +=
                              us_tr( "It appears there is an issue with buffer subtraction resulting in negative data\n" ) +
                              us_tr( "Integral baseline correction is not appropriate for this situation\n" ) +
                              us_tr( "Perhaps a linear baseline correction would help, but we recommend you first verify the buffer subtraction\n\n" );
                        } else {
                           tmp_msg += us_tr( "Integral baseline correction is not recommended\n\n" );
                        }
                     }
                  } else {
                     // this is the only choice
                     if ( best_dparameters[ "minsumqmaxqy" ] > 0 ) {
                        tmp_msg +=
                           best_vdparameters.count( "yx" ) ?
                           us_tr( "Integral baseline correction would be possible, but no steady state end region has been identified\n\n" ) 
                           :
                           us_tr( "Integral baseline correction is possible\n\n" );
                     } else {
                        tmp_msg += us_tr( "Integral baseline correction is not recommended\n\n" );
                     }
                  }                     
               }
            }
               
            tmp_msg +=
               QString( 
                       "Baseline Analysis:\n"
                       "  Name                                                 %1\n"
                       "  q sampling interval                                  %2\n" 
                       "  Cormap Analysis maximum q [A^-1]                     %3\n" 
                        )
               .arg( best_parameters[ "name" ] )
               .arg( baseline_cormap_parameters.count( "decimate" ) ? QString( "%1" ).arg( baseline_cormap_parameters[ "decimate" ] ) : QString( "none" ) )
               .arg( cormap_maxq )
               ;

            best_dparameters[ "cormap_maxq" ] = QString( "%1" ).arg( cormap_maxq ).toDouble();

            // accumulate common strings 
            best_parameters[ "hb_msg" ] += best_parameters[ "msg" ] + "Holm-Bonferroni adjustment values\n" + tmp_msg;
            best_parameters[ "msg" ]    += tmp_msg;

            if( best_vdparameters.count( "bestpos" ) ) {
               if ( best_vdparameters[ "bestpos" ].size() > 1 ) {
                  best_parameters[ "msg" ] +=
                     QString(
                             hb_mode 
                             ? "  Best start positions based on red pair %             %1"
                             : "  Best start positions based on avg. red cluster size  %1"
                             )
                     .arg( best_vdparameters[ "bestpos" ].front() )
                     ;
                  for ( int i = 1; i < (int) best_vdparameters[ "bestpos" ].size(); ++i ) {
                     best_parameters[ "msg" ] += QString( ",%1" ).arg( best_vdparameters[ "bestpos" ][ i ] );
                  }
                  best_parameters[ "msg" ] += "\n";
               } else {
                  best_parameters[ "msg" ] +=
                     QString(
                             best_vdparameters.count( "yx" )
                             ? "~~_darkyellow2_~~  Best start positions based on avg. red cluster size  %1~~_darkblue_~~"
                             : ( 
                                hb_mode 
                                ? "  Best start position based on red pair %              %1\n"
                                : "  Best start position based on avg. red cluster size   %1\n"
                                 )
                             )
                     .arg( best_vdparameters[ "bestpos" ].front() )
                  ;
               }
            }

            if( best_vdparameters.count( "hb_bestpos" ) ) {
               if ( best_vdparameters[ "hb_bestpos" ].size() > 1 ) {
                  best_parameters[ "hb_msg" ] +=
                     QString(
                             "  Best start positions based on red pair %             %1"
                             )
                     .arg( best_vdparameters[ "hb_bestpos" ].front() )
                     ;
                  for ( int i = 1; i < (int) best_vdparameters[ "hb_bestpos" ].size(); ++i ) {
                     best_parameters[ "hb_msg" ] += QString( ",%1" ).arg( best_vdparameters[ "hb_bestpos" ][ i ] );
                  }
                  best_parameters[ "hb_msg" ] += "\n";
               } else {
                  best_parameters[ "hb_msg" ] +=
                     QString(
                             ( 
                              "  Best start position based on red pair %              %1\n"
                               )
                             )
                     .arg( best_vdparameters[ "hb_bestpos" ].front() )
                  ;
               }
            }

            { // common for both msg & hb_msg 
               QString tmp_msg;

               if( best_vdparameters.count( "bestsumqmaxqypos" ) ) {
                  if ( best_vdparameters[ "bestqmaxqypos" ].size() > 1 ) {
                     tmp_msg +=
                        QString(
                                "~~_darkorange3_~~  Lowest average intensity positions                   %1~~_darkblue_~~"
                                )
                        .arg( best_vdparameters[ "bestsumqmaxqypos" ].front() )
                        ;
                     for ( int i = 1; i < (int) best_vdparameters[ "bestsumqmaxqypos" ].size(); ++i ) {
                        tmp_msg += QString( ",%1" ).arg( best_vdparameters[ "bestsumqmaxqypos" ][ i ] );
                     }
                     tmp_msg += "\n";
                  } else {
                     tmp_msg +=
                        QString(
                                "~~_darkorange3_~~  Lowest average intensity position                    %1~~_darkblue_~~"
                                )
                        .arg( best_vdparameters[ "bestsumqmaxqypos" ].front() )
                        ;
                  }
               }
               best_parameters[ "msg"    ] += tmp_msg;
               best_parameters[ "hb_msg" ] += tmp_msg;
            }

            if ( hb_mode ) {
               best_parameters[ "msg" ] +=
                  QString(
                          "  Used window width                                    %4\n"
                          )
                  .arg( baseline_best_last_window_size )
                  ;
            } else {
               best_parameters[ "hb_msg" ] +=
                  QString(
                          "  Used window width                                    %4\n"
                          )
                  .arg( baseline_best_last_window_size )
                  ;
               best_parameters[ "msg" ] +=
                  QString(
                          "\n"
                          "\n"
                          "Blanks frames:\n"
                          "  Name                                                 %1\n"
                          "  q sampling interval                                  %2\n"
                          "  Maximum window width                                 %3\n"
                          "  Used window width                                    %4\n"
                          "  Number of sliding windows of this width averaged     %5\n"
                          )
                  .arg( blanks_cormap_parameters.count( "name" ) ?
                        blanks_cormap_parameters[ "name" ] : QString( us_tr( "unknown" ) ) )
                  .arg( blanks_cormap_parameters.count( "decimate" ) ?
                        blanks_cormap_parameters[ "decimate" ] : QString( us_tr( "none" )) )
                  .arg( blanks_cormap_parameters.count( "frames" ) ?
                        blanks_cormap_parameters[ "frames" ] : QString( us_tr( "unknown" ) ) )
                  .arg( baseline_best_last_window_size )
                  .arg( blanks_brookesmap_sliding_results.count( QString( "%1 count" ).arg( baseline_best_last_window_size ) ) ?
                        QString( "%1" ).arg( blanks_brookesmap_sliding_results[ QString( "%1 count" ).arg( baseline_best_last_window_size ) ] ) :
                        QString( "unknown" ) )
                  ;
            }


            if ( best_dparameters.count( "blanksaverage" ) ) {
               best_parameters[ "msg" ] +=
                  QString( "  Used window red cluster average size                 %1\n" ).arg( best_dparameters[ "blanksaverage" ] );
               if ( best_dparameters.count( "blanksaveragesd" ) ) {
                  best_parameters[ "msg" ] +=
                     QString( "  Used window red cluster average size sd              %1\n" ).arg( best_dparameters[ "blanksaveragesd" ] );
               }
            }


            best_parameters[ "global_width"  ] = "800";
            best_parameters[ "global_height" ] = "600";

            if ( best_vdparameters[ "x" ].size() ) {
               US_Hydrodyn_Mals_Saxs_Baseline_Best * uhsbb = new US_Hydrodyn_Mals_Saxs_Baseline_Best( us_hydrodyn,
                                                                                                      best_parameters,
                                                                                                      best_dparameters,
                                                                                                      best_vdparameters
                                                                                                      );
               uhsbb->show();
               baseline_best_ok = true;
               cb_baseline_fix_width->setChecked( true );
            } else {
               us_qdebug( "some error" );
            }
         }
      } else {
         baseline_cormap_parameters .clear( );
         baseline_cormap_pvaluepairs.clear( );
         baseline_brookesmap_sliding_results.clear( );
      }
   }
   baseline_enables();
}

void US_Hydrodyn_Mals_Saxs::baseline_test()
{
   // QMessageBox::warning(this, 
   //                      windowTitle() + us_tr( " : Test baseline" ),
   //                      us_tr( "Test baseline is not currently functional" ), 
   //                      QMessageBox::Ok | QMessageBox::Default,
   //                      QMessageBox::NoButton
   //                      );
   // run a temporary baseline_apply over a "scroll mode" (maybe open up listbox ?)
   // save & remove files as needed 
   // include full iteration plot and plot original & created curve

   wheel_enables( false );

   le_baseline_end_s     ->clearFocus();
   le_baseline_end       ->clearFocus();
   le_baseline_end_e     ->clearFocus();
   le_baseline_width     ->clearFocus();
   le_last_focus = (mQLineEdit *) 0;

   baseline_test_mode = true;

   // clear markers

   gauss_delete_markers();
   baseline_enables();
   wheel_enables( false );
   qwtw_wheel->setRange( 0, baseline_selected.size() - 1); qwtw_wheel->setSingleStep( 1 );
   qwtw_wheel->setValue( 0 );
   wheel_enables();
   lbl_wheel_pos->setText( "" );
   {
      QPixmap pm;
      lbl_wheel_Pcolor->setPixmap( pm );
   }
   baseline_test_pos( 0 );
}

void US_Hydrodyn_Mals_Saxs::baseline_test_pos( int pos ) {
   // us_qdebug( QString( "baseline_test_pos( %1 ) %2" ).arg( pos )
   //         .arg( 
   //              ( pos >= 0 && pos < (int) baseline_selected.size() ) ?  baseline_selected[ pos ] : QString( "outta limits" ) ) );

   remove_files( baseline_test_added_set );
   baseline_test_added_set.clear( );

   if ( pos < 0 || pos >= (int) baseline_selected.size() ) {
      lbl_wheel_pos_below->setText("" );
      return;
   }

   lbl_wheel_pos_below->setText( baseline_selected[ pos ] );

   set < QString > select_set;

   select_set.insert( baseline_selected[ pos ] );

   // create new files via apply baseline and add to baseline_test_added_set & select_set

   QStringList qsl;
   qsl << baseline_selected[ pos ];
   // us_qdebug( QString( "smooth %1" ).arg( ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_saxs_bl_smooth" ].toInt() ) );

   baseline_apply( qsl
                   ,true
                   ,( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_saxs_bl_smooth" ].toInt()
                   ,true
                   ,( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_saxs_bl_reps" ].toInt()
                   ,false
                   ,true );

   for ( int i = 0; i < (int) baseline_apply_created.size(); ++i ) {
      baseline_test_added_set.insert( baseline_apply_created[ i ] );
      select_set             .insert( baseline_apply_created[ i ] );
   }

   set_selected( select_set );
   // rescale();
}
