#include "../include/us3_defines.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_hydrodyn_saxs_hplc.h"
#include "../include/us_lm.h"

#define TSO QTextStream(stdout)

static US_Hydrodyn_Saxs_Hplc *uhsh;
static double bblm_kernel_size;
static double bblm_kernel_delta_t;
static vector < double > bblm_org_conc_t;
static vector < double > bblm_org_conc_I;
static vector < double > bblm_ref_t;
static vector < double > bblm_ref_I;
static vector < double > bblm_ref_errors_mult;
static vector < double > bblm_init_params;
static vector < double > bblm_fixed_params;
static size_t bblm_size;
static double bblm_scale;
static bool   bblm_ref_has_errors;
static vector < bool >   bblm_fit_param;

static unordered_map < double,
                       unordered_map < double,
                                       unordered_map < double,
                                                       vector < double > > > > bblm_broadened_conc_I_cache;
#define BBLM_INTERP_PAD 1e6

static double bblm_fit( double t, const double *par ) {
   // qDebug() << "bblm_fit 0";
   static US_Saxs_Util usu;

   size_t pos       = 0;
   size_t pos_fixed = 0;
   double tau       = bblm_fit_param[ 0 ] ? par[pos++] : bblm_fixed_params[ pos_fixed++ ];
   double sigma     = bblm_fit_param[ 1 ] ? par[pos++] : bblm_fixed_params[ pos_fixed++ ];
   double deltat    = bblm_fit_param[ 2 ] ? par[pos++] : bblm_fixed_params[ pos_fixed++ ];
   double baseline  = bblm_fit_param[ 3 ] ? par[pos++] : bblm_fixed_params[ pos_fixed++ ];

   if ( tau < 0 ) {
      return DBL_MAX;
   }
   if ( sigma < 0 ) {
      return DBL_MAX;
   }
   
   vector < double > bblm_conc_I;

   // qDebug() << "bblm_fit 1";

   if ( bblm_broadened_conc_I_cache.count( tau )
        && bblm_broadened_conc_I_cache[ tau ].count( sigma ) 
        && bblm_broadened_conc_I_cache[ tau ][ sigma ].count( deltat ) ) {
      // return point value
      if ( floor( t ) != t
           || t > bblm_broadened_conc_I_cache[ tau ][ sigma ][ deltat ].size()
           || t < 0
           ) {
         qDebug() << QString( "--> error, t value %1 not integer or outside of fitting range" ).arg( t );
         return DBL_MAX;
      }

      return bblm_broadened_conc_I_cache[ tau ][ sigma ][ deltat ][ (size_t) t ] + baseline * bblm_scale;
   }

   if ( !uhsh->broaden_compute_one_no_ui(
                                         tau
                                         ,sigma
                                         ,bblm_kernel_size
                                         ,bblm_kernel_delta_t
                                         ,bblm_org_conc_I
                                         ,bblm_conc_I
                                         ) ) {
      qDebug() << "band_broaden_fit() fit error";
      return DBL_MAX;
   }
   
   // qDebug() << "bblm_fit 2";

   vector < double > bblm_conc_t = bblm_org_conc_t;
   for ( auto & t : bblm_conc_t ) {
      t += deltat;
   }

   for ( auto & I : bblm_conc_I ) {
      I *= bblm_scale;
   }

   vector < double > new_conc_I;

   if ( !usu.linear_interpolate( bblm_conc_t, bblm_conc_I, bblm_ref_t, new_conc_I ) ) {
      uhsh->editor_msg( "red", QString( "Error while fitting: interpolation error: %1\n" ).arg( usu.errormsg ) );
      qDebug() << "interpolation error";
      US_Vector::printvector2( "bblm_conc_t, bblm_conc_I", bblm_conc_t, bblm_conc_I );
      US_Vector::printvector2( "bblm_ref_t, bblm_ref_I", bblm_ref_t, bblm_ref_I );
      exit(-1);
      return DBL_MAX;
   }

   if ( bblm_ref_has_errors ) {
      for ( size_t i = 0; i < bblm_size; ++i ) {
         new_conc_I[ i ] *= bblm_ref_errors_mult[ i ];
      }
   }

   bblm_broadened_conc_I_cache[ tau ][ sigma ][ deltat ] = new_conc_I;

   // US_Vector::printvector2( "bblm_fit after += deltat: bblm_org_conc_t, bblm_conc_t:", bblm_org_conc_t, bblm_conc_t );

   // US_Vector::printvector3( "bblm org conc t, I, conc I", bblm_org_conc_t, bblm_org_conc_I, bblm_conc_I );
   // qDebug() << "bblm_fit 3";
   // US_Vector::printvector3( "bblm ref t, I, errors", bblm_ref_t, bblm_ref_I, bblm_ref_errors );

   // double loss = uhsh->broaden_compute_loss_no_ui(
   //                                                bblm_conc_t
   //                                                ,bblm_conc_I
   //                                                ,bblm_ref_t
   //                                                ,bblm_ref_I
   //                                                ,bblm_ref_errors
   //                                                );
   // qDebug() << "bblm_fit 4";

   if ( floor( t ) != t
        || t > new_conc_I.size()
        || t < 0
        ) {
      qDebug() << QString( "--> error, t value %1 not integer or outside of fitting range" ).arg( t );
      return DBL_MAX;
   }

   return new_conc_I[ (size_t) t ] + baseline * bblm_scale;
}
   

static double broaden_scale;

static double discrete_area_under_curve( const vector < double > & x, const vector < double > & y ) {
   double area = 0;
   size_t x_size = x.size();
   size_t y_size = y.size();

   size_t min_size = x_size;
   if ( min_size > y_size ) {
      min_size = y_size;
   }

   for ( size_t i = 1; i < min_size; ++i ) {
      area += 0.5 * ( y[ i - 1 ] + y[ i ] ) * ( x[ i ] - x[ i - 1 ] );
   }
   return area;
}

static double discrete_area_under_curve( const vector < double > & org_x, const vector < double > & org_y, double start_x, double end_x, QString msg = "" ) {
   double area = 0;

   vector < double > x;
   vector < double > y;

   size_t org_x_size = org_x.size();

   for ( size_t i = 0; i < org_x_size; ++i ) {
      if ( org_x[ i ] >= start_x && org_x[ i ] <= end_x ) {
         x.push_back( org_x[ i ] );
         y.push_back( org_y[ i ] );
      }
   }

   size_t x_size = x.size();
   size_t y_size = y.size();

   size_t min_size = x_size;
   if ( min_size > y_size ) {
      min_size = y_size;
   }

   for ( size_t i = 1; i < min_size; ++i ) {
      area += 0.5 * ( y[ i - 1 ] + y[ i ] ) * ( x[ i ] - x[ i - 1 ] );
   }
   // US_Vector::printvector2( QString( "%1: area %2, x, y" ).arg( msg ).arg( area ), x, y );
   return area;
}

static US_Band_Broaden ubb;
static void tso_vector_info( QString msg, const vector < double > & v ) {
   TSO <<
      QString( "%1 : size %2 range [%3:%4]\n" )
      .arg( msg )
      .arg( v.size() )
      .arg( v.front() )
      .arg( v.back() )
      ;
}

void US_Hydrodyn_Saxs_Hplc::broaden() {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden() start\n";
   broaden_org_selected = all_selected_files_set();

   disable_all();

   ubb.clear();

   running               = false;

   broaden_names.clear();
   broaden_created.clear();

   QStringList this_conc_files;
   QStringList this_ref_files;

   for ( auto const & file : broaden_org_selected ) {
      if ( conc_files.count( file ) ) {
         this_conc_files << file;
      } else {
         this_ref_files  << file;
      }
   }

   TSO <<
      QString( "broaden: this_conc_files %1\n"
               "          this_ref_files %2\n" )
      .arg( this_conc_files.join( " : " ) )
      .arg( this_ref_files.join( " : " ) )
      ;

   if ( !this_conc_files.size() ) {
      qDebug() << "--> no conc files, select conc file!";
      QString conc_file = select_conc_file( "broaden" );
      if ( conc_file.isEmpty() ) {
         QMessageBox::warning( this
                               ,windowTitle() + us_tr( " : Broaden" )
                               ,QString( us_tr( "No concentration file found to broaden\n" ) )
                               ,QMessageBox::Ok
                               ,QMessageBox::NoButton );

         return update_enables();
      }
      this_conc_files << conc_file;
   }

   if ( this_conc_files.size() != 1 || this_ref_files.size() != 1 ) {
      QMessageBox::warning( this
                            ,windowTitle() + us_tr( " : Broaden" )
                            ,QString( us_tr( "Exactly one reference and one concentration file are needed\n" ) )
                            ,QMessageBox::Ok
                            ,QMessageBox::NoButton );
      return update_enables();
   }


   broaden_names
      << this_conc_files[ 0 ]
      << this_ref_files[ 0 ]
      ;
      
   TSO <<
      QString( "broaden: conc file is %1\n"
               "          ref file is %2\n" )
      .arg( broaden_names[ 0 ] )
      .arg( broaden_names[ 1 ] )
      ;

   broaden_ref_has_errors =
      f_errors.count( broaden_names[ 1 ] )
      && f_errors[ broaden_names[ 1 ] ].size() == f_qs[ broaden_names[ 1 ] ].size()
      && is_nonzero_vector( f_errors[ broaden_names[ 1 ] ] )
      ;

   set_selected( broaden_names );

   double fit_range_start = f_qs[ broaden_names[ 0 ] ][ 0 ];
   double fit_range_end   = f_qs[ broaden_names[ 0 ] ].back();

   if ( fit_range_start < f_qs[ broaden_names[ 1 ] ][ 0 ] ) {
      fit_range_start = f_qs[ broaden_names[ 1 ] ][ 0 ];
   }

   if ( fit_range_end > f_qs[ broaden_names[ 1 ] ].back() ) {
      fit_range_end = f_qs[ broaden_names[ 1 ] ].back();
   }

   if ( le_broaden_fit_range_start->text().toDouble() < fit_range_start
        || le_broaden_fit_range_start->text().toDouble() >= fit_range_end
        || le_broaden_fit_range_start->text().isEmpty()
        ) {
      le_broaden_fit_range_start->setText( QString( "%1" ).arg( fit_range_start ) );
   }

   if ( le_broaden_fit_range_end->text().toDouble() <= fit_range_start
        || le_broaden_fit_range_end->text().toDouble() > fit_range_end
        || le_broaden_fit_range_end->text().isEmpty()
        ) {
      le_broaden_fit_range_end->setText( QString( "%1" ).arg( fit_range_end ) );
   }

   mode_select( MODE_BROADEN );
   broaden_enables();
}

void US_Hydrodyn_Saxs_Hplc::broaden_done( bool save ) {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_done() start\n";

   ubb.clear();

   if ( broaden_names.size() > 2 ) {
      set < QString > to_remove = { broaden_names.takeLast() };
      remove_files( to_remove );
   }

   if ( save ) {
      TSO << "US_Hydrodyn_Saxs_Hplc::broaden_done() save\n";
      // recreate and save broadened & broadened interp

      // recreate:
      ubb.clear();

      vector < double > broadened = ubb.broaden(
                                                f_Is[ broaden_names[ 0 ] ]
                                                ,le_broaden_tau->text().toDouble()
                                                ,le_broaden_sigma->text().toDouble()
                                                ,0
                                                ,le_broaden_kernel_end->text().toDouble()
                                                ,le_broaden_kernel_deltat->text().toDouble()
                                                );
      if ( !broadened.size() ) {
         lbl_broaden_msg->setText( QString( "Error: %1" ).arg( ubb.errormsg ) );
      } else {
         lbl_broaden_msg->setText( QString( "Broadened vector size: %1" ).arg( broadened.size() ) );

         // setup plot curve

         QString fname =
            QString( "%1_tau%2_sigma%3_b%4_dt%5_kte%6_ktdelta%7" )
            .arg( broaden_names[ 0 ] )
            .arg( le_broaden_tau->text() )
            .arg( le_broaden_sigma->text() )
            .arg( le_broaden_baseline->text() )
            .arg( le_broaden_deltat->text() )
            .arg( le_broaden_kernel_end->text() )
            .arg( le_broaden_kernel_deltat->text() )
            ;

         vector < double > ts = f_qs[ broaden_names[ 0 ] ];
         double deltat = le_broaden_deltat->text().toDouble();
         for ( auto & t : ts ) {
            t += deltat;
         }
   
         // original grid, only need interpolated to target
         // add_plot( fname, ts, broadened, true, false );
         // broaden_names << last_created_file;
         // conc_files.insert( last_created_file );
         // f_header[ last_created_file ] = QString( " Broadening target %1" ).arg( broaden_names[ 1 ] );
         // f_errors.erase( last_created_file );
         // vector < double > I = f_Is[ last_created_file ];

         vector < double > I = broadened;

         // & interpolated
      
         // pad for interpolation

         ts.insert( ts.begin(),  *(ts.begin()) - BBLM_INTERP_PAD );
         I.insert( I.begin(),  0 );
         ts.push_back( ts.back() + BBLM_INTERP_PAD );
         I.push_back( 0 );

         vector < double > I_interp;
      
         if ( !usu->linear_interpolate( ts, I, f_qs[ broaden_names[ 1 ] ], I_interp ) ) {
            editor_msg( "red", QString( "Error while fitting: interpolation error: %1\n" ).arg( usu->errormsg ) );
         } else {
            add_plot( fname, f_qs[ broaden_names[ 1 ] ], I_interp, true, false );
            broaden_names << last_created_file;
            conc_files.insert( last_created_file );
            f_header[ last_created_file ] = QString( " Broadening target %1" ).arg( broaden_names[ 1 ] );
            f_errors.erase( last_created_file );

            // scale area

            double fit_range_start = le_broaden_fit_range_start->text().toDouble();
            double fit_range_end   = le_broaden_fit_range_end  ->text().toDouble();
            double baseline        = le_broaden_baseline       ->text().toDouble();

            vector < double > conc_t = ts;
            vector < double > conc_I = I;

            // crop only ref, conc is full and will be interpolated to ref
            vector < double > & ref_t = f_qs[ broaden_names[ 1 ] ];
            vector < double > & ref_I = f_Is[ broaden_names[ 1 ] ];

            double area_ref  = discrete_area_under_curve( ref_t, ref_I, fit_range_start, fit_range_end );
            double area_conc = discrete_area_under_curve( conc_t, conc_I, fit_range_start, fit_range_end );

            if ( area_conc <= 0 ) {
               editor_msg( "red", QString( "Error: area under broadened curve restricted to fitting range is not positive" ) );
            } else {
               broaden_scale = area_ref / area_conc;

               QString name = last_created_file + "_scaled";

               for ( auto & I : I_interp ) {
                  I *= broaden_scale;
                  I += baseline;
               }

               add_plot( name, f_qs[ broaden_names[ 1 ] ], I_interp, true, false );
               conc_files.insert( last_created_file );
               f_header[ last_created_file ] = QString( " Broadening target %1" ).arg( broaden_names[ 1 ] );
               f_errors.erase( last_created_file );
            }
         }
      }
   } else {
      TSO << "US_Hydrodyn_Saxs_Hplc::broaden_done() cancel\n";
   }
   set_selected( broaden_org_selected );
   mode_select( MODE_NORMAL );

   wheel_enables( false );
   pb_wheel_save         ->setEnabled( false );
   pb_wheel_cancel       ->setEnabled( false );

   running               = false;

   disable_all();
   update_enables();
}

void US_Hydrodyn_Saxs_Hplc::broaden_enables() {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_enables() start\n";

   cb_broaden_tau               -> setEnabled( true );
   le_broaden_tau_start         -> setEnabled( true );
   le_broaden_tau               -> setEnabled( true );
   le_broaden_tau_end           -> setEnabled( true );
   le_broaden_tau_delta         -> setEnabled( true );
   cb_broaden_sigma             -> setEnabled( true );
   le_broaden_sigma_start       -> setEnabled( true );
   le_broaden_sigma             -> setEnabled( true );
   le_broaden_sigma_end         -> setEnabled( true );
   le_broaden_sigma_delta       -> setEnabled( true );
   cb_broaden_deltat            -> setEnabled( true );
   le_broaden_deltat_start      -> setEnabled( true );
   le_broaden_deltat            -> setEnabled( true );
   le_broaden_deltat_end        -> setEnabled( true );
   le_broaden_deltat_delta      -> setEnabled( true );
   cb_broaden_baseline          -> setEnabled( false );
   le_broaden_baseline          -> setEnabled( false );
   le_broaden_baseline          -> setText( "0" );
   le_broaden_kernel_end        -> setEnabled( true );
   le_broaden_kernel_deltat     -> setEnabled( true );
   cb_broaden_repeak            -> setEnabled( true );
   le_broaden_fit_range_start   -> setEnabled( true );
   le_broaden_fit_range_end     -> setEnabled( true );
   cb_broaden_kernel_type       -> setEnabled( true );
   pb_broaden_minimize          -> setEnabled( true );
   pb_broaden_reset             -> setEnabled( true );

   {
      double fit_range_start = le_broaden_fit_range_start->text().toDouble();
      double fit_range_end   = le_broaden_fit_range_end  ->text().toDouble();
      pb_broaden_fit               -> setEnabled(
                                                 ( cb_broaden_tau->isChecked()
                                                   || cb_broaden_sigma->isChecked()
                                                   || cb_broaden_deltat->isChecked()
                                                   || cb_broaden_baseline->isChecked()
                                                   ) &&
                                                 fit_range_start < fit_range_end
                                                 );
   }

   pb_wheel_cancel              -> setEnabled( true );
   pb_wheel_save                -> setEnabled( broaden_names.size() == 3 );

   // for grid fit
   // {
   //    double tau_start     = le_broaden_tau_start    ->text().toDouble();
   //    double tau_end       = le_broaden_tau_end      ->text().toDouble();
   //    double tau_delta     = le_broaden_tau_delta    ->text().toDouble();

   //    double sigma_start     = le_broaden_sigma_start    ->text().toDouble();
   //    double sigma_end       = le_broaden_sigma_end      ->text().toDouble();
   //    double sigma_delta     = le_broaden_sigma_delta    ->text().toDouble();

   //    double deltat_start    = le_broaden_deltat_start   ->text().toDouble();
   //    double deltat_end      = le_broaden_deltat_end     ->text().toDouble();
   //    double deltat_delta    = le_broaden_deltat_delta   ->text().toDouble();

   //    double fit_range_start = le_broaden_fit_range_start->text().toDouble();
   //    double fit_range_end   = le_broaden_fit_range_end  ->text().toDouble();
   //    pb_broaden_fit               -> setEnabled(
   //                                               (
   //                                                (
   //                                                 tau_delta > 0
   //                                                 && tau_end > tau_start
   //                                                 && tau_start >= 0
   //                                                 )
   //                                                ||
   //                                                (
   //                                                 sigma_delta > 0
   //                                                 && sigma_end > sigma_start
   //                                                 && sigma_start >= 0 
   //                                                 )
   //                                                )
   //                                               &&
   //                                               (
   //                                                deltat_end > deltat_start
   //                                                && deltat_delta > 0
   //                                                )
   //                                               &&
   //                                               fit_range_end > fit_range_start
   //                                               );
   // }

   cb_eb                        -> setEnabled( true );
   cb_dots                      -> setEnabled( true );
   pb_color_rotate              -> setEnabled( true );
   pb_line_width                -> setEnabled( true );
   pb_legend                    -> setEnabled( true );
}

void US_Hydrodyn_Saxs_Hplc::broaden_plot( bool /* replot */ ) {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_plot() start\n";
}

void US_Hydrodyn_Saxs_Hplc::broaden_clear_plot() {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_clear_plot() start\n";
}

void US_Hydrodyn_Saxs_Hplc::broaden_minimize() {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_minimize() start\n";
}

void US_Hydrodyn_Saxs_Hplc::broaden_reset() {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_reset() start\n";

   cb_broaden_tau      ->setChecked( true );
   le_broaden_tau      ->setText( "0" );

   cb_broaden_sigma    ->setChecked( true );
   le_broaden_sigma    ->setText( "0" );

   cb_broaden_deltat   ->setChecked( true );
   le_broaden_deltat   ->setText( "0" );

   cb_broaden_baseline ->setChecked( false );
   le_broaden_baseline ->setText( "0" );

   broaden_compute_one();

   broaden_enables();
}

void US_Hydrodyn_Saxs_Hplc::set_broaden_tau() {
   TSO << "US_Hydrodyn_Saxs_Hplc::set_broaden_tau() start\n";
   broaden_enables();
}

void US_Hydrodyn_Saxs_Hplc::broaden_tau_start_text( const QString & ) {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_tau_start_text() start\n";
}

void US_Hydrodyn_Saxs_Hplc::broaden_tau_start_focus( bool ) {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_tau_start_focus() start\n";
   broaden_enables();
}

void US_Hydrodyn_Saxs_Hplc::broaden_tau_text( const QString & ) {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_tau_text() start\n";
}

void US_Hydrodyn_Saxs_Hplc::broaden_tau_focus( bool hasFocus ) {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_tau_focus() start\n";
   broaden_enables();
   if ( !hasFocus ) {
      broaden_compute_one();
   }
}

void US_Hydrodyn_Saxs_Hplc::broaden_tau_end_text( const QString & ) {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_tau_end_text() start\n";
}

void US_Hydrodyn_Saxs_Hplc::broaden_tau_end_focus( bool ) {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_tau_end_focus() start\n";
   broaden_enables();
}

void US_Hydrodyn_Saxs_Hplc::broaden_tau_delta_text( const QString & ) {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_tau_delta_text() start\n";
}

void US_Hydrodyn_Saxs_Hplc::broaden_tau_delta_focus( bool ) {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_tau_delta_focus() start\n";
   broaden_enables();
}

void US_Hydrodyn_Saxs_Hplc::set_broaden_sigma() {
   TSO << "US_Hydrodyn_Saxs_Hplc::set_broaden_sigma() start\n";
   broaden_enables();
}

void US_Hydrodyn_Saxs_Hplc::broaden_sigma_start_text( const QString & ) {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_sigma_start_text() start\n";
}

void US_Hydrodyn_Saxs_Hplc::broaden_sigma_start_focus( bool ) {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_sigma_start_focus() start\n";
   broaden_enables();
}

void US_Hydrodyn_Saxs_Hplc::broaden_sigma_text( const QString & ) {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_sigma_text() start\n";
}

void US_Hydrodyn_Saxs_Hplc::broaden_sigma_focus( bool hasFocus ) {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_sigma_focus() start\n";
   broaden_enables();
   if ( !hasFocus ) {
      broaden_compute_one();
   }
}

void US_Hydrodyn_Saxs_Hplc::broaden_sigma_end_text( const QString & ) {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_sigma_end_text() start\n";
}

void US_Hydrodyn_Saxs_Hplc::broaden_sigma_end_focus( bool ) {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_sigma_end_focus() start\n";
   broaden_enables();
}

void US_Hydrodyn_Saxs_Hplc::broaden_sigma_delta_text( const QString & ) {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_sigma_delta_text() start\n";
}

void US_Hydrodyn_Saxs_Hplc::broaden_sigma_delta_focus( bool ) {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_sigma_delta_focus() start\n";
   broaden_enables();
}

void US_Hydrodyn_Saxs_Hplc::set_broaden_deltat() {
   TSO << "US_Hydrodyn_Saxs_Hplc::set_broaden_deltat() start\n";
   broaden_enables();
}

void US_Hydrodyn_Saxs_Hplc::broaden_deltat_start_text( const QString & ) {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_deltat_start_text() start\n";
}

void US_Hydrodyn_Saxs_Hplc::broaden_deltat_start_focus( bool ) {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_deltat_start_focus() start\n";
   broaden_enables();
}

void US_Hydrodyn_Saxs_Hplc::broaden_deltat_text( const QString & ) {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_deltat_text() start\n";
}

void US_Hydrodyn_Saxs_Hplc::broaden_deltat_focus( bool hasFocus ) {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_deltat_focus() start\n";
   broaden_enables();
   if ( !hasFocus ) {
      broaden_compute_one();
   }
}

void US_Hydrodyn_Saxs_Hplc::broaden_deltat_end_text( const QString & ) {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_deltat_end_text() start\n";
}

void US_Hydrodyn_Saxs_Hplc::broaden_deltat_end_focus( bool ) {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_deltat_end_focus() start\n";
   broaden_enables();
}

void US_Hydrodyn_Saxs_Hplc::broaden_deltat_delta_text( const QString & ) {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_deltat_delta_text() start\n";
}

void US_Hydrodyn_Saxs_Hplc::broaden_deltat_delta_focus( bool ) {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_deltat_delta_focus() start\n";
   broaden_enables();
}

void US_Hydrodyn_Saxs_Hplc::set_broaden_baseline() {
   TSO << "US_Hydrodyn_Saxs_Hplc::set_broaden_baseline() start\n";
   broaden_enables();
}

void US_Hydrodyn_Saxs_Hplc::broaden_baseline_text( const QString & ) {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_baseline_text() start\n";
}

void US_Hydrodyn_Saxs_Hplc::broaden_baseline_focus( bool hasFocus ) {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_baseline_focus() start\n";
   broaden_enables();
   if ( !hasFocus ) {
      broaden_compute_one();
   }
}

void US_Hydrodyn_Saxs_Hplc::broaden_kernel_end_text( const QString & ) {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_kernel_end_text() start\n";
}

void US_Hydrodyn_Saxs_Hplc::broaden_kernel_end_focus( bool hasFocus ) {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_kernel_end_focus() start\n";
   broaden_enables();
   if ( !hasFocus ) {
      broaden_compute_one();
   }
}

void US_Hydrodyn_Saxs_Hplc::broaden_kernel_deltat_text( const QString & ) {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_kernel_deltat_text() start\n";
}

void US_Hydrodyn_Saxs_Hplc::broaden_kernel_deltat_focus( bool hasFocus ) {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_kernel_deltat_focus() start\n";
   broaden_enables();
   if ( !hasFocus ) {
      broaden_compute_one();
   }
}

void US_Hydrodyn_Saxs_Hplc::broaden_fit_range_start_text( const QString & ) {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_fit_range_start_text() start\n";
}

void US_Hydrodyn_Saxs_Hplc::broaden_fit_range_start_focus( bool hasFocus ) {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_fit_range_start_focus() start\n";
   broaden_enables();
   if ( !hasFocus ) {
      broaden_compute_one();
   }
}

void US_Hydrodyn_Saxs_Hplc::broaden_fit_range_end_text( const QString & ) {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_fit_range_end_text() start\n";
}

void US_Hydrodyn_Saxs_Hplc::broaden_fit_range_end_focus( bool hasFocus ) {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_fit_range_end_focus() start\n";
   broaden_enables();
   if ( !hasFocus ) {
      broaden_compute_one();
   }
}

void US_Hydrodyn_Saxs_Hplc::broaden_repeak_set() {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_repeak_set() start\n";
   broaden_compute_one();
}

void US_Hydrodyn_Saxs_Hplc::broaden_kernel_type_index() {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_kernel_type_index() start\n";
   broaden_compute_one();
}

bool US_Hydrodyn_Saxs_Hplc::broaden_compute_one_no_ui(
                                                      double tau
                                                      ,double sigma
                                                      ,double kernel_size
                                                      ,double kernel_delta_t
                                                      ,const vector < double > & I
                                                      ,vector < double > & broadened
                                                      ) {
#warning need to better support changing kernel type etc in US_Band_Broaden, currently only caching on Tau, so clearing it all for now
   ubb.clear();
   broadened = ubb.broaden(
                           I
                           ,tau
                           ,sigma
                           ,0
                           ,kernel_size
                           ,kernel_delta_t
                           );
   if ( !broadened.size() ) {
      lbl_broaden_msg->setText( QString( "Error: %1" ).arg( ubb.errormsg ) );
      return false;
   }
   return true;
}

void US_Hydrodyn_Saxs_Hplc::broaden_compute_one() {

   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_compute_one() start\n";

   // remove old broadened curve if present
   if ( broaden_names.size() == 3 ) {
      conc_files.erase( broaden_names[2] ); // doesn't remove_files() do this?
      set < QString > tmp_names = { broaden_names[2] };
      remove_files( tmp_names );
      // arg, no resize for QStringList
      // broaden_names.resize( 2 );
      broaden_names.removeAt( 2 );
   }

   // compute broadened curve

#warning need to better support changing kernel type etc in US_Band_Broaden, currently only caching on Tau, so clearing it all for now
   ubb.clear();

   vector < double > broadened = ubb.broaden(
                                             f_Is[ broaden_names[ 0 ] ]
                                             ,le_broaden_tau->text().toDouble()
                                             ,le_broaden_sigma->text().toDouble()
                                             ,0
                                             ,le_broaden_kernel_end->text().toDouble()
                                             ,le_broaden_kernel_deltat->text().toDouble()
                                             );
   if ( !broadened.size() ) {
      lbl_broaden_msg->setText( QString( "Error: %1" ).arg( ubb.errormsg ) );
   } else {
      lbl_broaden_msg->setText( QString( "Broadened vector size: %1" ).arg( broadened.size() ) );
   }      

   // setup plot curve

   QString fname =
      QString( "%1_tau%2_sigma%3_b%4_dt%5_kte%6_ktdelta%7" )
      .arg( broaden_names[ 0 ] )
      .arg( le_broaden_tau->text() )
      .arg( le_broaden_sigma->text() )
      .arg( le_broaden_baseline->text() )
      .arg( le_broaden_deltat->text() )
      .arg( le_broaden_kernel_end->text() )
      .arg( le_broaden_kernel_deltat->text() )
      ;

   vector < double > ts = f_qs[ broaden_names[ 0 ] ];
   double deltat = le_broaden_deltat->text().toDouble();
   for ( auto & t : ts ) {
      t += deltat;
   }
   
   add_plot( fname, ts, broadened, true, false );
   broaden_names << last_created_file;
   conc_files.insert( last_created_file );
   f_header[ last_created_file ] = QString( " Broadening target %1" ).arg( broaden_names[ 1 ] );
   f_errors.erase( last_created_file );

   double fit = broaden_compute_loss();
   if ( fit != DBL_MAX ) {
      lbl_broaden_msg  ->setText(
                                 lbl_broaden_msg->text()
                                 + QString( "  Last fit %1 : %2" )
                                 .arg( broaden_ref_has_errors ? "nChi^2" : "RMSD" )
                                 .arg( fit )
                                 );

      if ( cb_broaden_repeak->isChecked() ) {
         QStringList files;
         files << broaden_names[ 1 ];
         double baseline = le_broaden_baseline       ->text().toDouble();
         QString broadened_not_repeaked = broaden_names.takeLast();
         for ( auto & I : f_Is[ broadened_not_repeaked ] ) {
            I *= broaden_scale;
            I += baseline;
         }
         add_plot(
                  QString( "%1_ab_%2" ).arg( broadened_not_repeaked ).arg( broaden_scale )
                  ,f_qs[ broadened_not_repeaked ]
                  ,f_Is[ broadened_not_repeaked ]
                  ,true
                  ,false
                  );
         
         broaden_names << last_created_file;
         conc_files.insert( last_created_file );
         f_header[ last_created_file ] = QString( " Broadening target %1" ).arg( broaden_names[ 1 ] );
         f_errors.erase( last_created_file );
         set < QString > to_remove = { broadened_not_repeaked };
         remove_files( to_remove );
      }
   } else {
      lbl_broaden_msg  ->setText( lbl_broaden_msg->text() + "  Could not compute fit" );
   }

   set_selected( broaden_names, true, true );
   
   double fit_range_start = le_broaden_fit_range_start->text().toDouble();
   double fit_range_end   = le_broaden_fit_range_end  ->text().toDouble();

   if ( broaden_names.size() > 2 ) {
      lbl_broaden_msg  ->setText(
                                 lbl_broaden_msg->text() +
                                 QString( "\nDiscrete areas target %1, conc. %2, broadened %3" )
                                 .arg( discrete_area_under_curve( f_qs[ broaden_names[ 1 ] ] , f_Is[ broaden_names[ 1 ] ], fit_range_start, fit_range_end, "target" ) )
                                 .arg( discrete_area_under_curve( f_qs[ broaden_names[ 0 ] ] , f_Is[ broaden_names[ 0 ] ], fit_range_start, fit_range_end, "conc") )
                                 .arg( discrete_area_under_curve( f_qs[ broaden_names[ 2 ] ] , f_Is[ broaden_names[ 2 ] ], fit_range_start, fit_range_end, "broadened" ) )
                                 );
   }
}

void US_Hydrodyn_Saxs_Hplc::broaden_lm_fit() {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_lm_fit() start\n";

   editor_msg( "darkblue", us_tr( "Broaden fit starting\n" ) );

   running = true;

   disable_all();

   uhsh = (US_Hydrodyn_Saxs_Hplc *)this;

   bblm_broadened_conc_I_cache.clear();

   bblm_init_params.clear();
   bblm_fixed_params.clear();

   vector < bool > fit_param( 4, false );
   bblm_fit_param = fit_param;
   
   if ( cb_broaden_tau->isChecked() ) {
      bblm_init_params.push_back( le_broaden_tau->text().toDouble() );
      bblm_fit_param[ 0 ] = true;
   } else {
      bblm_fixed_params.push_back( le_broaden_tau->text().toDouble() );
   }

   if ( cb_broaden_sigma->isChecked() ) {
      bblm_init_params.push_back( le_broaden_sigma->text().toDouble() );
      bblm_fit_param[ 1 ] = true;
   } else {
      bblm_fixed_params.push_back( le_broaden_sigma->text().toDouble() );
   }

   if ( cb_broaden_deltat->isChecked() ) {
      bblm_init_params.push_back( le_broaden_deltat->text().toDouble() );
      bblm_fit_param[ 2 ] = true;
   } else {
      bblm_fixed_params.push_back( le_broaden_deltat->text().toDouble() );
   }

   if ( cb_broaden_baseline->isChecked() ) {
      bblm_init_params.push_back( le_broaden_baseline->text().toDouble() );
      bblm_fit_param[ 3 ] = true;
   } else {
      bblm_fixed_params.push_back( le_broaden_baseline->text().toDouble() );
   }
      
   bblm_kernel_size       = le_broaden_kernel_end    ->text().toDouble();
   bblm_kernel_delta_t    = le_broaden_kernel_deltat ->text().toDouble();

   double fit_range_start = le_broaden_fit_range_start->text().toDouble();
   double fit_range_end   = le_broaden_fit_range_end  ->text().toDouble();

   bblm_ref_t             .clear();
   bblm_ref_I             .clear();
   bblm_ref_errors_mult   .clear();
   bblm_ref_has_errors    = broaden_ref_has_errors;
   // bblm_ref_has_errors    = false; // testing

   // crop only ref, conc is full and will be interpolated to ref
   vector < double > & org_ref_t = f_qs[ broaden_names[ 1 ] ];
   vector < double > & org_ref_I = f_Is[ broaden_names[ 1 ] ];

   bblm_org_conc_t = f_qs[ broaden_names[ 0 ] ];
   bblm_org_conc_I = f_Is[ broaden_names[ 0 ] ];

   // pad for interpolation

   bblm_org_conc_t.insert( bblm_org_conc_t.begin(),  *(bblm_org_conc_t.begin()) - BBLM_INTERP_PAD );
   bblm_org_conc_I.insert( bblm_org_conc_I.begin(),  0 );
   bblm_org_conc_t.push_back( bblm_org_conc_t.back() + BBLM_INTERP_PAD );
   bblm_org_conc_I.push_back( 0 );

   // US_Vector::printvector2( "padded bblm org", bblm_org_conc_t,  bblm_org_conc_I );

   size_t org_ref_t_size = org_ref_t.size();

   if ( bblm_ref_has_errors ) {
      vector < double > & org_ref_errors = f_errors[ broaden_names[ 1 ] ];
      for ( size_t i = 0; i < org_ref_t_size; ++i ) {
         if ( org_ref_t[ i ] >= fit_range_start
              && org_ref_t[ i ] <= fit_range_end ) {
            bblm_ref_t.push_back( org_ref_t[ i ] );
            bblm_ref_I.push_back( org_ref_I[ i ] );
            bblm_ref_errors_mult.push_back( 1e0 / org_ref_errors[ i ] );
         }
      }
   } else {
      for ( size_t i = 0; i < org_ref_t_size; ++i ) {
         if ( org_ref_t[ i ] >= fit_range_start
              && org_ref_t[ i ] <= fit_range_end ) {
            bblm_ref_t.push_back( org_ref_t[ i ] );
            bblm_ref_I.push_back( org_ref_I[ i ] );
         }
      }
   }      

   double area_ref  = discrete_area_under_curve( bblm_ref_t, bblm_ref_I, fit_range_start, fit_range_end );
   double area_conc = discrete_area_under_curve( bblm_org_conc_t, bblm_org_conc_I, fit_range_start, fit_range_end );
   if ( area_conc <= 0 ) {
      editor_msg( "red", "concentraton curve has negative or zero area in the fitting range fitting\n" );
      running = false;
      broaden_enables();
      progress->reset();
      return;
   }

   bblm_scale = area_ref / area_conc;

   // divide by sd (or sd^2) for chi2 fitting, after scaling area computation

   bblm_size = bblm_ref_I.size();
   
   if ( bblm_ref_has_errors ) {
      for ( size_t i = 0; i < bblm_size; ++i ) {
         bblm_ref_I[ i ] *= bblm_ref_errors_mult[ i ];
      }
   }

   LM::lm_control_struct control = LM::lm_control_double;
   control.printflags = 0; // 3; // monitor status (+1) and parameters (+2)
   control.epsilon    = 1e-4;
   control.stepbound  = 500;
   control.maxcall    = 500;

   LM::lm_status_struct status;
   vector < double > y = bblm_ref_I;
   vector < double > t( bblm_ref_I.size() );
   iota( t.begin(), t.end(), 0 );

   // US_Vector::printvector2( "t, y", t, y );

   srand(time(0));

   size_t params_size = bblm_init_params.size();
   
   if ( !params_size ) {
      editor_msg( "red", us_tr( "Error: no parameters to fit\n" ) );
      running = false;
      broaden_enables();
      progress->reset();
      return;
   }

   vector < vector < double > > init_params;

   {
      // generate init_params

      size_t permuts = (size_t) pow( 2, (int)params_size );

      for ( size_t i = 0; i < permuts; ++i ) {
         vector < double > this_init( params_size );
         for ( size_t j = 0; j < params_size; ++j ) {
            this_init[ j ] = i & ( 1 << j ) ? 1 : 0;
         }
         init_params.push_back( this_init );
      }

      init_params.push_back( bblm_init_params );
      
      for ( size_t j = 0; j < params_size; ++j ) {
         vector < double > this_init( params_size, 0 );
         this_init[ j ] = bblm_init_params[ j ];
         init_params.push_back( this_init );
      }
   }         
      
   #define RANDOM_RESTARTS 10

   for ( size_t i = 0; i < RANDOM_RESTARTS; ++i ) {
      vector < double > this_init( params_size );
      for ( size_t j = 0; j < params_size; ++j ) {
         this_init[ j ] = (double) ( rand() / (double) RAND_MAX );
      }
      init_params.push_back( this_init );
      auto rv = rand();
      US_Vector::printvector( QString( "randoms rand() %1 RAND_MAX %2 rand()/RAND_MAX %3 " ).arg( rv ).arg( RAND_MAX ).arg( rv / (double) RAND_MAX ), this_init );
   }

   #define REFITS 5

   progress->setMaximum( init_params.size() + REFITS );
   size_t pos = 0;

   vector < double > best_params = init_params[ 0 ];
   double best_fit = DBL_MAX;

   {
      set < vector < double > > already_run;

      for ( auto this_params : init_params ) {
         progress->setValue( pos++ );
         qApp->processEvents();
      
         US_Vector::printvector( "--> lmcurve_fit_rmsd init", this_params );

         if ( already_run.count( this_params ) ) {
            TSO << "already run, skipping\n";
            continue;
         }
            
         already_run.insert( this_params );

         LM::lmcurve_fit_rmsd( ( int )      this_params.size(),
                               ( double * ) &( this_params[ 0 ] ),
                               ( int )      t.size(),
                               ( double * ) &( t[ 0 ] ),
                               ( double * ) &( y[ 0 ] ),
                               bblm_fit,
                               (const LM::lm_control_struct *)&control,
                               &status );

         // US_Vector::printvector( QString( "lmcurve_fit_rmsd fnorm %1" ).arg( status.fnorm ), this_params );

         if ( best_fit > status.fnorm ) {
            best_fit    = status.fnorm;
            best_params = this_params;
         }
      }

      // rerun from final best fit

      for ( size_t i = 0; i < REFITS; ++i ) {
         progress->setValue( pos++ );
         qApp->processEvents();
      
         vector < double > this_params = best_params;

         if ( already_run.count( this_params ) ) {
            TSO << "already run, skipping\n";
            continue;
         }

         already_run.insert( this_params );

         US_Vector::printvector( QString( "--> refinement %1 of %2 lmcurve_fit_rmsd final refinement" ).arg( i+1 ).arg( REFITS ), this_params );

         LM::lmcurve_fit_rmsd( ( int )      this_params.size(),
                               ( double * ) &( this_params[ 0 ] ),
                               ( int )      t.size(),
                               ( double * ) &( t[ 0 ] ),
                               ( double * ) &( y[ 0 ] ),
                               bblm_fit,
                               (const LM::lm_control_struct *)&control,
                               &status );

         // US_Vector::printvector( QString( "lmcurve_fit_rmsd fnorm %1" ).arg( status.fnorm ), this_params );

         if ( best_fit > status.fnorm ) {
            best_fit    = status.fnorm;
            best_params = this_params;
         }
      }
   }

   progress->reset();

   bblm_broadened_conc_I_cache.clear();

   US_Vector::printvector( "bblm_init_param after lmcurve_fit_rmsd", bblm_init_params );

   {
      size_t pos = 0;

      if ( cb_broaden_tau->isChecked() ) {
         le_broaden_tau     ->setText( QString( "%1" ).arg( best_params[ pos++ ], 0, 'g', 6 ) );
      }

      if ( cb_broaden_sigma->isChecked() ) {
         le_broaden_sigma   ->setText( QString( "%1" ).arg( best_params[ pos++ ], 0, 'g', 6 ) );
      }

      if ( cb_broaden_deltat->isChecked() ) {
         le_broaden_deltat  ->setText( QString( "%1" ).arg( best_params[ pos++ ], 0, 'g', 6 ) );
      }

      if ( cb_broaden_baseline->isChecked() ) {
         le_broaden_baseline->setText( QString( "%1" ).arg( best_params[ pos++ ], 0, 'g', 6 ) );
      }
   }

   // double loss = bblm_fit( 0, &( bblm_init_params[ 0 ] ) );

   // qDebug() 
   //    << " result: " << loss
   //    ;

   qDebug() 
      << " broaden_compute_one() call"
      ;
   broaden_compute_one();
   qDebug() 
      << " broaden_compute_one() ok"
      ;

   running = false;
   broaden_enables();
   progress->reset();
   editor_msg( "darkblue", us_tr( "Broaden fit finished\n" ) );
}

void US_Hydrodyn_Saxs_Hplc::broaden_fit() {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_fit() start\n";

   return broaden_lm_fit();

   running = true;

   disable_all();

   // QStringList repeak_target;
   // repeak_target << broaden_names[ 1 ];

   double tau_start     = le_broaden_tau_start    ->text().toDouble();
   double tau_end       = le_broaden_tau_end      ->text().toDouble();
   double tau_delta     = le_broaden_tau_delta    ->text().toDouble();

   double sigma_start     = le_broaden_sigma_start    ->text().toDouble();
   double sigma_end       = le_broaden_sigma_end      ->text().toDouble();
   double sigma_delta     = le_broaden_sigma_delta    ->text().toDouble();

   double deltat_start    = le_broaden_deltat_start   ->text().toDouble();
   double deltat_end      = le_broaden_deltat_end     ->text().toDouble();
   double deltat_delta    = le_broaden_deltat_delta   ->text().toDouble();

   double fit_range_start = le_broaden_fit_range_start->text().toDouble();
   double fit_range_end   = le_broaden_fit_range_end  ->text().toDouble();


   {
      bool can_fit =
         (
          (
           tau_delta > 0
           && tau_end > tau_start
           && tau_start >= 0
           )
          ||
          (
           sigma_delta > 0
           && sigma_end > sigma_start
           && sigma_start >= 0 
           )
          )
         &&
         (
          deltat_end > deltat_start
          && deltat_delta > 0
          )
         &&
         fit_range_end > fit_range_start
         ;

      if ( !can_fit ) {
         editor_msg( "red", "Incorrect fitting range\n" );
         broaden_enables();
         return;
      }
   }

   vector < double > ref_t;
   vector < double > ref_I;
   vector < double > ref_errors;

   // crop only ref, conc is full and will be interpolated to ref
   vector < double > & org_ref_t = f_qs[ broaden_names[ 1 ] ];
   vector < double > & org_ref_I = f_Is[ broaden_names[ 1 ] ];

   size_t org_ref_t_size = org_ref_t.size();

   if ( broaden_ref_has_errors ) {
      vector < double > & org_ref_errors = f_errors[ broaden_names[ 1 ] ];
      for ( size_t i = 0; i < org_ref_t_size; ++i ) {
         if ( org_ref_t[ i ] >= fit_range_start
              && org_ref_t[ i ] <= fit_range_end ) {
            ref_t.push_back( org_ref_t[ i ] );
            ref_I.push_back( org_ref_I[ i ] );
            ref_errors.push_back( org_ref_errors[ i ] );
         }
      }
   } else {
      for ( size_t i = 0; i < org_ref_t_size; ++i ) {
         if ( org_ref_t[ i ] >= fit_range_start
              && org_ref_t[ i ] <= fit_range_end ) {
            ref_t.push_back( org_ref_t[ i ] );
            ref_I.push_back( org_ref_I[ i ] );
         }
      }
   }      

   double best_tau    = tau_start;
   double best_sigma  = sigma_start;
   double best_deltat = deltat_start;
   double best_loss   = DBL_MAX;

   QString summary = "tau,sigma,deltat,loss\n";

   double kernel_size                 = le_broaden_kernel_end->text().toDouble();
   double kernel_delta_t              = le_broaden_kernel_deltat->text().toDouble();

   vector < double > org_conc_t = f_qs[ broaden_names[ 0 ] ];
   vector < double > org_conc_I = f_Is[ broaden_names[ 0 ] ];
   vector < double > conc_I; // broadened

   size_t total_pts =
      ( 1 + ( ( tau_end - tau_start ) / tau_delta ) ) *
      ( 1 + ( ( sigma_end - sigma_start ) / sigma_delta ) ) *
      ( 1 + ( ( deltat_end - deltat_start ) / deltat_delta ) )
      ;

   size_t pt       = 0;

   progress->reset();
   progress->setMaximum( total_pts );

   for ( double tau = tau_start; tau <= tau_end + 1e-10; tau += tau_delta ) {
      progress->setValue( pt );
      for ( double sigma = sigma_start; sigma <= sigma_end + 1e-10; sigma += sigma_delta ) {
         qApp->processEvents();
         for ( double deltat = deltat_start; deltat <= deltat_end + 1e-10; deltat += deltat_delta ) {
            // le_broaden_tau   ->setText( QString( "%1" ).arg( tau ) ); // , 0, 'f', 6 ) );
            // le_broaden_deltat->setText( QString( "%1" ).arg( deltat ) ); // , 0, 'f', 6 ) );
            TSO << QString( "fit progress %1 of %2 %3% tau %4 sigma %5 deltat %6\n" )
               .arg( pt )
               .arg( total_pts )
               .arg( 100 * pt / total_pts )
               .arg( tau )
               .arg( sigma )
               .arg( deltat )
               ;
            pt += 1;

            if ( !broaden_compute_one_no_ui(
                                            tau
                                            ,sigma
                                            ,kernel_size
                                            ,kernel_delta_t
                                            ,org_conc_I
                                            ,conc_I
                                            ) ) {
               editor_msg( "red", "Error: broadening failure" );
               running = false;
               broaden_enables();
               progress->reset();
               return;
            }


            vector < double > conc_t = f_qs[ broaden_names[ 0 ] ];
            for ( auto & t : conc_t ) {
               t += deltat;
            }

            // US_Vector::printvector2( "conc_t, conc_I", conc_t, conc_I );
            // US_Vector::printvector2( "ref_t, ref_I", ref_t, ref_I );

            tso_vector_info( "conc_t", conc_t );
            tso_vector_info( "ref_t", ref_t );

            double this_loss = broaden_compute_loss_no_ui(
                                                          conc_t
                                                          ,conc_I
                                                          ,ref_t
                                                          ,ref_I
                                                          ,ref_errors
                                                          );
            if ( this_loss == DBL_MAX ) {
               editor_msg( "red", "unable to compute loss, stopping fit" );
               running = false;
               broaden_enables();
               progress->reset();
               return;
            }

            summary += QString( "%1,%2,%3,%4\n" ).arg( tau ).arg( sigma ).arg( deltat ).arg( this_loss );
         
            if ( best_loss > this_loss ) {
               best_loss   = this_loss;
               best_tau    = tau;
               best_sigma  = sigma;
               best_deltat = deltat;
               editor_msg( "darkblue"
                           ,QString( "best loss so far tau %1 sigma %2 deltat %3 loss %4\n" )
                           .arg( tau )
                           .arg( sigma )
                           .arg( deltat )
                           .arg( best_loss )
                           );
               qApp->processEvents();
            }
         }
      }
   }

   // {
   //    QString error;
   //    US_File_Util::putcontents( "broaden_fit_summary.csv", summary, error );
   // }

   le_broaden_tau   ->setText( QString( "%1" ).arg( best_tau ) ); // , 0, 'f', 6 ) );
   le_broaden_sigma ->setText( QString( "%1" ).arg( best_sigma ) ); // , 0, 'f', 6 ) );
   le_broaden_deltat->setText( QString( "%1" ).arg( best_deltat ) ); // , 0, 'f', 6 ) );
   lbl_broaden_msg  ->setText(
                              QString( "Last fit %1 : %2" )
                              .arg( broaden_ref_has_errors ? "nChi^2" : "RMSD" )
                              .arg( best_loss )
                              );

   qApp->processEvents();
   
   broaden_compute_one();
   running = false;
   broaden_enables();
   progress->reset();
}

double US_Hydrodyn_Saxs_Hplc::broaden_compute_loss_no_ui(
                                                         const vector < double > & conc_t
                                                         ,const vector < double > & conc_I
                                                         ,const vector < double > & ref_t
                                                         ,const vector < double > & ref_I
                                                         ,const vector < double > & ref_errors
                                                         ) {
   static US_Saxs_Util usu;

   size_t ref_t_size  = ref_t.size();

   vector < double > new_conc_I;

   tso_vector_info( "in loss conc_t", conc_t );
   tso_vector_info( "in loss conc_I", conc_I );
   tso_vector_info( "in loss ref_t", ref_t );

   if ( !usu.linear_interpolate( conc_t, conc_I, ref_t, new_conc_I ) ) {
      editor_msg( "red", QString( "Error while fitting: interpolation error: %1\n" ).arg( usu.errormsg ) );
      US_Vector::printvector2( "conc_t, conc_I", conc_t, conc_I );
      US_Vector::printvector2( "ref_t, ref_I", ref_t, ref_I );
      return DBL_MAX;
   }

   // scale area

   double area_ref  = discrete_area_under_curve( ref_t, ref_I );
   double area_conc = discrete_area_under_curve( ref_t, new_conc_I );

   if ( area_conc <= 0 ) {
      editor_msg( "red", QString( "Error: area under broadened curve restricted to fitting range is not positive" ) );
      return DBL_MAX;
   }
  
   broaden_scale = area_ref / area_conc;
   for ( auto & I : new_conc_I ) {
      I *= broaden_scale;
   }
   TSO << QString( "area_ref %1 area_conc %2 scale %3 area_new_conc %4\n" )
      .arg( area_ref )
      .arg( area_conc )
      .arg( broaden_scale )
      .arg( discrete_area_under_curve( ref_t, new_conc_I ) )
      ;

   // US_Vector::printvector3( "new conc I, ref_i, ref_errors", new_conc_I, ref_I, ref_errors );
   
   if ( broaden_ref_has_errors ) {
      // TSO << "broaden_compute_loss_no_ui() - has errors, compute chi2\n";
      double chi2;
      if ( usu.calc_mychi2( new_conc_I
                            ,ref_I
                            ,ref_errors
                            ,chi2 ) ) {
         // TSO << "broaden_compute_loss_no_ui() - has errors, compute chi2 ok\n";
         return chi2 / ref_t_size;
      } else {
         // TSO << "broaden_compute_loss_no_ui() - has errors, compute chi2 NOT ok\n";
         editor_msg( "red", QString( "Error while fitting: error computing chi^2" ) );
         return DBL_MAX;
      }
   }

   // TSO << "broaden_compute_loss_no_ui() - no errors, compute RMSD\n";

   return usu.calc_rmsd( new_conc_I, ref_I );
}

double US_Hydrodyn_Saxs_Hplc::broaden_compute_loss() {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_compute_loss() start\n";
   // crop curves to fit range
   // interpolate conc curve to data
   // compute chi^2 or rmsd

   if ( broaden_names.size() < 3 ) {
      editor_msg( "red", "broaden_compute_loss() called with no recomputed curve\n" );
      return DBL_MAX;
   }

   // NOTE the reference curve range bits should be calculated once at fit start

   double fit_range_start = le_broaden_fit_range_start->text().toDouble();
   double fit_range_end   = le_broaden_fit_range_end  ->text().toDouble();

   vector < double > ref_t;
   vector < double > ref_I;
   vector < double > ref_errors;

   vector < double > conc_t = f_qs[ broaden_names[ 2 ] ];
   vector < double > conc_I = f_Is[ broaden_names[ 2 ] ];

   // crop only ref, conc is full and will be interpolated to ref
   vector < double > & org_ref_t = f_qs[ broaden_names[ 1 ] ];
   vector < double > & org_ref_I = f_Is[ broaden_names[ 1 ] ];

   size_t org_ref_t_size = org_ref_t.size();

   if ( broaden_ref_has_errors ) {
      vector < double > & org_ref_errors = f_errors[ broaden_names[ 1 ] ];
      for ( size_t i = 0; i < org_ref_t_size; ++i ) {
         if ( org_ref_t[ i ] >= fit_range_start
              && org_ref_t[ i ] <= fit_range_end ) {
            ref_t.push_back( org_ref_t[ i ] );
            ref_I.push_back( org_ref_I[ i ] );
            ref_errors.push_back( org_ref_errors[ i ] );
         }
      }
   } else {
      for ( size_t i = 0; i < org_ref_t_size; ++i ) {
         if ( org_ref_t[ i ] >= fit_range_start
              && org_ref_t[ i ] <= fit_range_end ) {
            ref_t.push_back( org_ref_t[ i ] );
            ref_I.push_back( org_ref_I[ i ] );
         }
      }
   }      
   size_t ref_t_size  = ref_t.size();
   size_t conc_t_size = conc_t.size();

   if ( ref_t_size <= 0
        || conc_t_size <= 0 ) {
      editor_msg( "red", "Error while fitting: no points left after range cropping\n" );
      return DBL_MAX;
   }

   static US_Saxs_Util usu;

   vector < double > new_conc_I;

   if ( !usu.linear_interpolate( conc_t, conc_I, ref_t, new_conc_I ) ) {
      editor_msg( "red", QString( "Error while fitting: interpolation error: %1\n" ).arg( usu.errormsg ) );
      return DBL_MAX;
   }

   // scale area

   double area_ref  = discrete_area_under_curve( ref_t, ref_I );
   double area_conc = discrete_area_under_curve( ref_t, new_conc_I );

   if ( area_conc <= 0 ) {
      editor_msg( "red", QString( "Error: area under broadened curve restricted to fitting range is not positive" ) );
      return DBL_MAX;
   }
  
   broaden_scale = area_ref / area_conc;
   for ( auto & I : new_conc_I ) {
      I *= broaden_scale;
   }

   if ( broaden_ref_has_errors ) {
      double chi2;
      if ( usu.calc_mychi2( new_conc_I
                            ,ref_I
                            ,ref_errors
                            ,chi2 ) ) {
         return chi2 / ref_t_size;
      } else {
         editor_msg( "red", QString( "Error while fitting: error computing chi^2" ) );
         return DBL_MAX;
      }
   }

   return usu.calc_rmsd( new_conc_I, ref_I );
}
