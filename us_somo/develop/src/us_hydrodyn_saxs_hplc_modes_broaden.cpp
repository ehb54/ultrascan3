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
#if !defined( BROADEN_SCALE_FIT )
static double bblm_scale;
#endif
static bool   bblm_ref_has_errors;
static vector < bool >   bblm_fit_param;
static bool   shutting_down;

static unordered_map < double,
                       unordered_map < double,
                                       unordered_map < double,
                                                       vector < double > > > > bblm_broadened_conc_I_cache;
#define BBLM_INTERP_PAD 1e6
#define BROADEN_LM_ENORM
/// BROADEN_NO_CHI2 turns off SD in curves for testing
// #define BROADEN_NO_CHI2
// #define BROADEN_NO_BASELINE
#define BROADEN_SCALE_FIT

static double bblm_fit( double t, const double *par ) {
   // qDebug() << "bblm_fit 0";
   static US_Saxs_Util usu;

   size_t pos       = 0;
   size_t pos_fixed = 0;
   size_t pos_param = 0;
   double sigma     = bblm_fit_param[ pos_param++ ] ? par[pos++] : bblm_fixed_params[ pos_fixed++ ];
   double tau       = bblm_fit_param[ pos_param++ ] ? par[pos++] : bblm_fixed_params[ pos_fixed++ ];
   // double xi_1      = bblm_fit_param[ pos_param++ ] ? par[pos++] : bblm_fixed_params[ pos_fixed++ ];
   // double xi_2      = bblm_fit_param[ pos_param++ ] ? par[pos++] : bblm_fixed_params[ pos_fixed++ ];
   double deltat    = bblm_fit_param[ pos_param++ ] ? par[pos++] : bblm_fixed_params[ pos_fixed++ ];
   double baseline  = bblm_fit_param[ pos_param++ ] ? par[pos++] : bblm_fixed_params[ pos_fixed++ ];
#if defined( BROADEN_SCALE_FIT )
   double scale     = bblm_fit_param[ pos_param++ ] ? par[pos++] : bblm_fixed_params[ pos_fixed++ ];
#endif

   if ( sigma < 0 ) {
      return DBL_MAX;
   }
   if ( tau < 0 ) {
      return DBL_MAX;
   }
#if defined( BROADEN_SCALE_FIT )
   if ( scale <= 0 ) {
      return DBL_MAX;
   }
#endif
   
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

      if ( bblm_ref_has_errors ) {
         if ( (size_t) t >= bblm_ref_errors_mult.size() ) {
            qDebug() << QString( "--> error, t value %1 outside of errors range" ).arg( t );
            return DBL_MAX;
         }
#if defined( BROADEN_SCALE_FIT )
         // scale first, then add the baseline -- this is consistent with what we do in broaden_done()
         return scale * bblm_broadened_conc_I_cache[ tau ][ sigma ][ deltat ][ (size_t) t ] + baseline * bblm_ref_errors_mult[ t ];
      } else {
         return scale * bblm_broadened_conc_I_cache[ tau ][ sigma ][ deltat ][ (size_t) t ] + baseline;
#else
         return bblm_broadened_conc_I_cache[ tau ][ sigma ][ deltat ][ (size_t) t ] + baseline * bblm_scale * bblm_ref_errors_mult[ t ];
      } else {
         return bblm_broadened_conc_I_cache[ tau ][ sigma ][ deltat ][ (size_t) t ] + baseline * bblm_scale;
#endif
      }
   }

   if ( !uhsh->broaden_compute_one_no_ui(
                                         tau
                                         ,sigma
                                         ,0 // xi_1
                                         ,0 // xi_2                                         
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

#if defined( BROADEN_SCALE_FIT )
   // the I is unscaled when we are doing a scaling fit
#else
   for ( auto & I : bblm_conc_I ) {
      I *= bblm_scale;
   }
#endif
   
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

   if ( bblm_ref_has_errors ) {
      if ( (size_t) t >= bblm_ref_errors_mult.size() ) {
         qDebug() << QString( "--> error, t value %1 outside of errors range" ).arg( t );
         return DBL_MAX;
      }
#if defined( BROADEN_SCALE_FIT )
      // scale first, then add the baseline -- this is consistent with what we do in broaden_done()
      return scale * new_conc_I[ (size_t) t ] + baseline * bblm_ref_errors_mult[ t ];
   } else {
      return scale * new_conc_I[ (size_t) t ] + baseline;
#else
      return new_conc_I[ (size_t) t ] + baseline * bblm_scale * bblm_ref_errors_mult[ t ];
   } else {
      return new_conc_I[ (size_t) t ] + baseline * bblm_scale;
#endif
   }
}

#if !defined( BROADEN_SCALE_FIT )
static double broaden_scale;
#endif

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

   gauss_delete_markers();

   disable_all();

   ubb.clear();

   running               = false;
   shutting_down         = false;

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

#if defined( BROADEN_NO_CHI2 )
#warning BROADEN_NO_CHI2 is defined, SDs not used
   qDebug() << "BROADEN_NO_CHI2 is defined, SDs not used";
   broaden_ref_has_errors = false;
#endif

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
   broaden_compute_one();
   gauss_add_marker( le_broaden_fit_range_start->text().toDouble(),
                     Qt::red,
                     "Start",
                     Qt::AlignRight | Qt::AlignTop );
   
   gauss_add_marker( le_broaden_fit_range_end->text().toDouble(),
                     Qt::red,
                     "End",
                     Qt::AlignLeft | Qt::AlignTop );
   
   broaden_enables();
}

void US_Hydrodyn_Saxs_Hplc::broaden_done( bool save ) {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_done() start\n";

   ubb.clear();
   shutting_down = true;
   gauss_delete_markers();
   
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
                                                // ,le_broaden_xi_1->text().toDouble()
                                                // ,le_broaden_xi_2->text().toDouble()
                                                ,0
                                                ,le_broaden_kernel_end->text().toDouble()
                                                ,le_broaden_kernel_deltat->text().toDouble()
                                                );
      if ( !broadened.size() ) {
         lbl_broaden_msg->setText( QString( "Error: %1" ).arg( ubb.errormsg ) );
      } else {
         lbl_broaden_msg->setText( QString( "Broadened vector size: %1" ).arg( broadened.size() ) );

         // setup plot curve

         QString header =
            QString( "sigma:%1 tau:%2 xi:(%3,%4) baseline:%5 scale:%6 delta_t:%7 kernel_end:%8 kernel_delta_t:%9 fit_range:[%10:%11]" )
            .arg( le_broaden_sigma->text() )
            .arg( le_broaden_tau->text() )
            .arg( le_broaden_xi_1->text() )
            .arg( le_broaden_xi_2->text() )
            .arg( le_broaden_baseline->text() )
            .arg( le_broaden_scale->text() )
            .arg( le_broaden_deltat->text() )
            .arg( le_broaden_kernel_end->text() )
            .arg( le_broaden_kernel_deltat->text() )
            .arg( le_broaden_fit_range_start->text() )
            .arg( le_broaden_fit_range_end->text() )
            ;

         QString fname = 
#if defined( BROADEN_SCALE_FIT )
            QString( "%1_Sigma%2_Tau%3" )
            .arg( broaden_names[ 0 ] )
            .arg( le_broaden_sigma->text().toDouble(), 0, 'g', 3 )
            .arg( le_broaden_tau->text().toDouble(), 0, 'g', 3 )
#else
            QString( "%1_sigma_%2_tau%3_b%4_dt%5_kte%6_ktdelta%7" )
            .arg( broaden_names[ 0 ] )
            .arg( le_broaden_sigma->text() )
            .arg( le_broaden_tau->text() )
            .arg( le_broaden_baseline->text() )
            .arg( le_broaden_deltat->text() )
            .arg( le_broaden_kernel_end->text() )
            .arg( le_broaden_kernel_deltat->text() )
#endif
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
            f_header[ last_created_file ] = QString( " Broadening target %1 %2" ).arg( broaden_names[ 1 ] ).arg( header );
            f_errors.erase( last_created_file );

#if defined( BROADEN_SCALE_FIT )
            double baseline        = le_broaden_baseline       ->text().toDouble();
            double scale           = le_broaden_scale          ->text().toDouble();

            QString name = last_created_file + "_scaled";

            for ( auto & I : I_interp ) {
               I *= scale;
               I += baseline;
            }

            add_plot( name, f_qs[ broaden_names[ 1 ] ], I_interp, true, false );
            conc_files.insert( last_created_file );
            f_header[ last_created_file ] = QString( " Broadening target %1 %2 scaled" ).arg( broaden_names[ 1 ] ).arg( header );
            f_errors.erase( last_created_file );
#else
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
#endif
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
   if ( shutting_down ) {
      return;
   }

   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_enables() start\n";

   cb_broaden_sigma             -> setEnabled( true );
   le_broaden_sigma_start       -> setEnabled( true );
   le_broaden_sigma             -> setEnabled( true );
   le_broaden_sigma_end         -> setEnabled( true );
   le_broaden_sigma_delta       -> setEnabled( true );
   cb_broaden_tau               -> setEnabled( true );
   le_broaden_tau_start         -> setEnabled( true );
   le_broaden_tau               -> setEnabled( true );
   le_broaden_tau_end           -> setEnabled( true );
   le_broaden_tau_delta         -> setEnabled( true );
   cb_broaden_xi_1              -> setEnabled( true );
   le_broaden_xi_1              -> setEnabled( true );
   cb_broaden_xi_2              -> setEnabled( true );
   le_broaden_xi_2              -> setEnabled( true );
   cb_broaden_deltat            -> setEnabled( true );
   le_broaden_deltat_start      -> setEnabled( true );
   le_broaden_deltat            -> setEnabled( true );
   le_broaden_deltat_end        -> setEnabled( true );
   le_broaden_deltat_delta      -> setEnabled( true );
#if defined( BROADEN_NO_BASELINE )
   cb_broaden_baseline          -> setEnabled( false );
   le_broaden_baseline          -> setEnabled( false );
   le_broaden_baseline          -> setText( "0" );
#else 
   cb_broaden_baseline          -> setEnabled( true );
   le_broaden_baseline          -> setEnabled( true );
#endif
#if defined( BROADEN_SCALE_FIT )
   cb_broaden_scale             -> setEnabled( true );
   le_broaden_scale             -> setEnabled( true );
#endif
   le_broaden_kernel_end        -> setEnabled( true );
   le_broaden_kernel_deltat     -> setEnabled( true );
   cb_broaden_repeak            -> setEnabled( true );
   le_broaden_fit_range_start   -> setEnabled( true );
   le_broaden_fit_range_end     -> setEnabled( true );
   cb_broaden_kernel_type       -> setEnabled( true );
   pb_broaden_minimize          -> setEnabled( true );
   pb_broaden_reset             -> setEnabled( true );
   pb_broaden_scale_compute     -> setEnabled( broaden_names.size() == 3 );

   {
      double fit_range_start = le_broaden_fit_range_start->text().toDouble();
      double fit_range_end   = le_broaden_fit_range_end  ->text().toDouble();
      pb_broaden_fit               -> setEnabled(
                                                 ( cb_broaden_tau->isChecked()
                                                   || cb_broaden_sigma->isChecked()
                                                   || cb_broaden_deltat->isChecked()
                                                   || cb_broaden_baseline->isChecked()
#if defined( BROADEN_SCALE_FIT )
                                                   || cb_broaden_scale->isChecked()
#endif
                                                   ) &&
                                                 fit_range_start < fit_range_end
                                                 );
   }

   pb_wheel_cancel              -> setEnabled( true );
   pb_wheel_save                -> setEnabled( broaden_names.size() == 3 );

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

   cb_broaden_sigma    ->setChecked( true );
   le_broaden_sigma    ->setText( "0" );

   cb_broaden_tau      ->setChecked( true );
   le_broaden_tau      ->setText( "0" );

   cb_broaden_xi_1       ->setChecked( false );
   le_broaden_xi_1       ->setText( "0" );
   
   cb_broaden_xi_2       ->setChecked( false );
   le_broaden_xi_2       ->setText( "0" );
   
   cb_broaden_deltat   ->setChecked( true );
   le_broaden_deltat   ->setText( "0" );

   cb_broaden_baseline ->setChecked( false );
   le_broaden_baseline ->setText( "0" );

#if defined( BROADEN_SCALE_FIT )
   cb_broaden_scale    ->setChecked( true );
   le_broaden_scale    ->setText( "1" );
   broaden_compute_one();
   broaden_scale_compute();
#else
   broaden_compute_one();
#endif

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
   if ( shutting_down ) {
      return;
   }
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
   if ( shutting_down ) {
      return;
   }

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

void US_Hydrodyn_Saxs_Hplc::set_broaden_xi_1() {
   TSO << "US_Hydrodyn_Saxs_Hplc::set_broaden_xi_1() start\n";
   broaden_enables();
}

void US_Hydrodyn_Saxs_Hplc::broaden_xi_1_text( const QString & ) {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_xi_1_text() start\n";
}

void US_Hydrodyn_Saxs_Hplc::broaden_xi_1_focus( bool hasFocus ) {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_xi_1_focus() start\n";
   if ( shutting_down ) {
      return;
   }
   broaden_enables();
   if ( !hasFocus ) {
      broaden_compute_one();
   }
}

void US_Hydrodyn_Saxs_Hplc::set_broaden_xi_2() {
   TSO << "US_Hydrodyn_Saxs_Hplc::set_broaden_xi_2() start\n";
   broaden_enables();
}

void US_Hydrodyn_Saxs_Hplc::broaden_xi_2_text( const QString & ) {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_xi_2_text() start\n";
}

void US_Hydrodyn_Saxs_Hplc::broaden_xi_2_focus( bool hasFocus ) {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_xi_2_focus() start\n";
   if ( shutting_down ) {
      return;
   }
   broaden_enables();
   if ( !hasFocus ) {
      broaden_compute_one();
   }
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
   if ( shutting_down ) {
      return;
   }
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
   if ( shutting_down ) {
      return;
   }
   broaden_enables();
   if ( !hasFocus ) {
      broaden_compute_one();
   }
}

void US_Hydrodyn_Saxs_Hplc::set_broaden_scale() {
   TSO << "US_Hydrodyn_Saxs_Hplc::set_broaden_scale() start\n";
   broaden_enables();
}

void US_Hydrodyn_Saxs_Hplc::broaden_scale_text( const QString & ) {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_scale_text() start\n";
}

void US_Hydrodyn_Saxs_Hplc::broaden_scale_focus( bool hasFocus ) {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_scale_focus() start\n";
   if ( shutting_down ) {
      return;
   }
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
   if ( shutting_down ) {
      return;
   }
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
   if ( shutting_down ) {
      return;
   }
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
   if ( shutting_down ) {
      return;
   }
   broaden_enables();
   if ( !hasFocus ) {
      if ( plotted_markers.size() > 1 ) {
         qDebug() << "set marker 0 " << le_broaden_fit_range_start->text();
         plotted_markers[ 0 ]->setXValue( le_broaden_fit_range_start->text().toDouble() );
         plot_dist->replot();
      } else {
         TSO << "US_Hydrodyn_Saxs_Hplc::broaden_fit_range_focus() no marker\n";
      }
      broaden_compute_one();
   }
}

void US_Hydrodyn_Saxs_Hplc::broaden_fit_range_end_text( const QString & ) {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_fit_range_end_text() start\n";
}

void US_Hydrodyn_Saxs_Hplc::broaden_fit_range_end_focus( bool hasFocus ) {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_fit_range_end_focus() start\n";
   if ( shutting_down ) {
      return;
   }
   broaden_enables();
   if ( !hasFocus ) {
      if ( plotted_markers.size() > 1 ) {
         qDebug() << "set marker 1 " << le_broaden_fit_range_end->text();
         plotted_markers[ 1 ]->setXValue( le_broaden_fit_range_end->text().toDouble() );
         plot_dist->replot();
      } else {
         TSO << "US_Hydrodyn_Saxs_Hplc::broaden_fit_range_focus() no marker\n";
      }
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
                                                      ,double /* xi_1 */ // inactive for now
                                                      ,double /* xi_2 */ // inactive for now
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

void US_Hydrodyn_Saxs_Hplc::broaden_compute_one( bool details ) {

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

#if defined( BROADEN_SCALE_FIT )
   // only the "repeaked" display will have the baseline offset
#else
   // baseline offset
   {
      double baseline = le_broaden_baseline->text().toDouble();
      for ( auto & I : broadened ) {
         I += baseline;
      }
   }
#endif
   
   // deltat offset
   vector < double > ts = f_qs[ broaden_names[ 0 ] ];
   double deltat = le_broaden_deltat->text().toDouble();
   for ( auto & t : ts ) {
      t += deltat;
   }

   // setup plot curve

   QString fname =
#if defined( BROADEN_SCALE_FIT )
      QString( "%1_tau%2_sigma%3_b%4_rp%5_dt%6_kte%7_ktdelta%8" )
      .arg( broaden_names[ 0 ] )
      .arg( le_broaden_tau->text() )
      .arg( le_broaden_sigma->text() )
      .arg( le_broaden_baseline->text() )
      .arg( le_broaden_scale->text() )
      .arg( le_broaden_deltat->text() )
      .arg( le_broaden_kernel_end->text() )
      .arg( le_broaden_kernel_deltat->text() )
#else
      QString( "%1_tau%2_sigma%3_b%4_dt%5_kte%6_ktdelta%7" )
      .arg( broaden_names[ 0 ] )
      .arg( le_broaden_tau->text() )
      .arg( le_broaden_sigma->text() )
      .arg( le_broaden_baseline->text() )
      .arg( le_broaden_deltat->text() )
      .arg( le_broaden_kernel_end->text() )
      .arg( le_broaden_kernel_deltat->text() )
#endif
      ;
   
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
#if defined( BROADEN_SCALE_FIT )
         // scale
         double broaden_scale = le_broaden_scale->text().toDouble();
#endif
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

   if ( details ) {
      QString q_val_str = "n/a";
      {
         QRegExp rx_q     ( "It_q(\\d+_\\d+)" );
         if ( rx_q.indexIn( broaden_names[ 1 ] ) != -1 ) {
            TSO << QString( "broaden_names[1] %1 rxcap ok %2\n" ).arg( broaden_names[1] ).arg( rx_q.cap( 1 ) );
            q_val_str =  rx_q.cap( 1 ).replace( "_", "." );
         } else {
            TSO << QString( "broaden_names[1] %1 rxcap FAILED\n" ).arg( broaden_names[1] );
         }
      }

#if defined( BROADEN_SCALE_FIT )
      editor_msg( "darkblue"
                  ,QString( "Broaden - last fit:\n%1, %2, %3, %4, %5, %6, %7, %8, %9\n%10, %11, %12, %13, %14, %15, %16, %17, %18\n" )
                  .arg( UNICODE_TAU )
                  .arg( UNICODE_SIGMA )
                  .arg( UNICODE_DELTA_QS + "t" )
                  .arg( "baseline" )
                  .arg( "scale" )
                  .arg( "target area" )
                  .arg( "broadened area" )
                  .arg( "q" )
                  .arg( broaden_ref_has_errors ? "nChi^2" : "RMSD" )
                  .arg( le_broaden_tau->text() )
                  .arg( le_broaden_sigma->text() )
                  .arg( le_broaden_deltat->text() )
                  .arg( le_broaden_baseline->text() )
                  .arg( le_broaden_scale->text() )
                  .arg( broaden_names.size() > 2 ? QString( "%1" ).arg( discrete_area_under_curve( f_qs[ broaden_names[ 1 ] ] , f_Is[ broaden_names[ 1 ] ], fit_range_start, fit_range_end ) ) : "n/a" )
                  .arg( broaden_names.size() > 2 ? QString( "%1" ).arg( discrete_area_under_curve( f_qs[ broaden_names[ 2 ] ] , f_Is[ broaden_names[ 2 ] ], fit_range_start, fit_range_end ) ) : "n/a" )
                  .arg( q_val_str )
                  .arg( fit )
                  );
#else
      editor_msg( "darkblue"
                  ,QString( "Broaden - last fit:\n%1, %2, %3, %4, %5, %6, %7, %8\n%9, %10, %11, %12, %13, %14, %15, %16\n" )
                  .arg( UNICODE_TAU )
                  .arg( UNICODE_SIGMA )
                  .arg( UNICODE_DELTA_QS + "t" )
                  .arg( "baseline" )
                  .arg( "target area" )
                  .arg( "broadened area" )
                  .arg( "q" )
                  .arg( broaden_ref_has_errors ? "nChi^2" : "RMSD" )
                  .arg( le_broaden_tau->text() )
                  .arg( le_broaden_sigma->text() )
                  .arg( le_broaden_deltat->text() )
                  .arg( le_broaden_baseline->text() )
                  .arg( broaden_names.size() > 2 ? QString( "%1" ).arg( discrete_area_under_curve( f_qs[ broaden_names[ 1 ] ] , f_Is[ broaden_names[ 1 ] ], fit_range_start, fit_range_end ) ) : "n/a" )
                  .arg( broaden_names.size() > 2 ? QString( "%1" ).arg( discrete_area_under_curve( f_qs[ broaden_names[ 2 ] ] , f_Is[ broaden_names[ 2 ] ], fit_range_start, fit_range_end ) ) : "n/a" )
                  .arg( q_val_str )
                  .arg( fit )
                  );
#endif
   }
}

void US_Hydrodyn_Saxs_Hplc::broaden_lm_fit( bool final_refinement_only ) {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_lm_fit() start\n";

   uhsh = (US_Hydrodyn_Saxs_Hplc *)this;

   bblm_broadened_conc_I_cache.clear();

   bblm_init_params.clear();
   bblm_fixed_params.clear();

#if defined( BROADEN_SCALE_FIT )
   vector < bool > fit_param( 5, false );
#else
   vector < bool > fit_param( 4, false );
#endif
   bblm_fit_param = fit_param;

   vector < double > init_params_min;
   
   if ( cb_broaden_sigma->isChecked() ) {
      bblm_init_params.push_back( le_broaden_sigma->text().toDouble() );
      bblm_fit_param[ 1 ] = true;
      init_params_min.push_back( 0 );
   } else {
      bblm_fixed_params.push_back( le_broaden_sigma->text().toDouble() );
   }

   if ( cb_broaden_tau->isChecked() ) {
      bblm_init_params.push_back( le_broaden_tau->text().toDouble() );
      bblm_fit_param[ 0 ] = true;
      init_params_min.push_back( 0 );
   } else {
      bblm_fixed_params.push_back( le_broaden_tau->text().toDouble() );
   }

   if ( cb_broaden_deltat->isChecked() ) {
      bblm_init_params.push_back( le_broaden_deltat->text().toDouble() );
      bblm_fit_param[ 2 ] = true;
      init_params_min.push_back( -DBL_MAX );
   } else {
      bblm_fixed_params.push_back( le_broaden_deltat->text().toDouble() );
   }

   if ( cb_broaden_baseline->isChecked() ) {
      bblm_init_params.push_back( le_broaden_baseline->text().toDouble() );
      bblm_fit_param[ 3 ] = true;
      init_params_min.push_back( -DBL_MAX );
   } else {
      bblm_fixed_params.push_back( le_broaden_baseline->text().toDouble() );
   }

#if defined( BROADEN_SCALE_FIT )
   if ( cb_broaden_scale->isChecked() ) {
      bblm_init_params.push_back( le_broaden_scale->text().toDouble() );
      bblm_fit_param[ 4 ] = true;
      init_params_min.push_back( DBL_MIN );
   } else {
      bblm_fixed_params.push_back( le_broaden_scale->text().toDouble() );
   }
#endif

   // original loss
   double original_loss = broaden_compute_loss();
   US_Vector::printvector( QString( "broaden_fit original loss %1, params:" ).arg( original_loss ), broaden_params() );
      
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

   US_Vector::printvector2( "padded bblm org", bblm_org_conc_t,  bblm_org_conc_I, 8, 8 );

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

#if defined( BROADEN_SCALE_FIT )
   double area_ref  = discrete_area_under_curve( bblm_ref_t, bblm_ref_I, fit_range_start, fit_range_end );
   double area_conc = discrete_area_under_curve( bblm_org_conc_t, bblm_org_conc_I, fit_range_start, fit_range_end );
   if ( area_conc <= 0 ) {
      editor_msg( "red", "concentraton curve has negative or zero area in the fitting range fitting\n" );
      return;
   }

   bblm_scale = area_ref / area_conc;
#endif

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
      
   #define RANDOM_RESTARTS 0

   for ( size_t i = 0; i < RANDOM_RESTARTS; ++i ) {
      vector < double > this_init( params_size );
      for ( size_t j = 0; j < params_size; ++j ) {
         this_init[ j ] = (double) ( rand() / (double) RAND_MAX );
      }
      init_params.push_back( this_init );
      auto rv = rand();
      US_Vector::printvector( QString( "randoms rand() %1 RAND_MAX %2 rand()/RAND_MAX %3 " ).arg( rv ).arg( RAND_MAX ).arg( rv / (double) RAND_MAX ), this_init );
   }

#if defined( BROADEN_SCALE_FIT )
   if ( 0 ) {
      // remove any init_params violating a preset minimum value
      vector < vector < double > > cleaned_init_params;
      for ( auto const & this_params : init_params ) {
         bool ok = true;
         for ( size_t i = 0; i < params_size; ++i ) {
            if ( this_params[ i ] < init_params_min[ i ] ) {
               ok = false;
               break;
            }
         }
         if ( ok ) {
            cleaned_init_params.push_back( this_params );
         }
      }
      init_params = cleaned_init_params;
   } else {
      // set to minimum value
      for ( auto & this_params : init_params ) {
         for ( size_t i = 0; i < params_size; ++i ) {
            if ( this_params[ i ] < init_params_min[ i ] ) {
               this_params[ i ] = init_params_min[ i ] ;
            }
         }
      }
   }

#endif

   #define REFITS 5
   size_t do_refits = REFITS;
   
   progress->setMaximum( init_params.size() + REFITS );
   size_t pos = 0;

   if ( final_refinement_only ) {
      init_params.clear();
      init_params.push_back( bblm_init_params );
      do_refits = 0;
   }
      
   vector < double > best_params = init_params[ 0 ];
   double best_fit = DBL_MAX;

   {
      set < vector < double > > already_run;
      for ( auto this_params : init_params ) {
         US_Vector::printvector( "init_params:", this_params );
      }

      for ( auto this_params : init_params ) {
         progress->setValue( pos++ );
         qApp->processEvents();
      
         US_Vector::printvector( "--> lmcurve_fit_rmsd init", this_params );

         if ( already_run.count( this_params ) ) {
            TSO << "already run, skipping\n";
            continue;
         }
            
         already_run.insert( this_params );

#if defined( BROADEN_LM_ENORM )
#warning BROADEN_LM_ENORM  is defined, alternate norm for LM fitting
         // LM ENORM version
         LM::lmcurve_fit
#else
         // RMSD LM version (originally used)
         LM::lmcurve_fit_rmsd
#endif
            (
             ( int )      this_params.size(),
             ( double * ) &( this_params[ 0 ] ),
             ( int )      t.size(),
             ( double * ) &( t[ 0 ] ),
             ( double * ) &( y[ 0 ] ),
             bblm_fit,
             (const LM::lm_control_struct *)&control,
             &status
             );

         US_Vector::printvector( QString( "lmcurve_fit_rmsd fnorm %1" ).arg( status.fnorm ), this_params );

         if ( best_fit > status.fnorm ) {
            best_fit    = status.fnorm;
            best_params = this_params;
         }
      }

      // rerun from final best fit

      for ( size_t i = 0; i < do_refits; ++i ) {
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

         US_Vector::printvector( QString( "lmcurve_fit_rmsd fnorm %1" ).arg( status.fnorm ), this_params );

         if ( best_fit > status.fnorm ) {
            best_fit    = status.fnorm;
            best_params = this_params;
         }
      }
   }

   progress->reset();

   bblm_broadened_conc_I_cache.clear();

   US_Vector::printvector( QString( "*** BEST after lmcurve_fit_rmsd fnorm %1, best_params" ).arg( best_fit ), best_params );

   {
      size_t pos = 0;

      if ( cb_broaden_sigma->isChecked() ) {
         le_broaden_sigma   ->setText( QString( "%1" ).arg( best_params[ pos++ ], 0, 'g', 8 ) );
      }

      if ( cb_broaden_tau->isChecked() ) {
         le_broaden_tau     ->setText( QString( "%1" ).arg( best_params[ pos++ ], 0, 'g', 8 ) );
      }

      if ( cb_broaden_deltat->isChecked() ) {
         le_broaden_deltat  ->setText( QString( "%1" ).arg( best_params[ pos++ ], 0, 'g', 8 ) );
      }

      if ( cb_broaden_baseline->isChecked() ) {
         le_broaden_baseline  ->setText( QString( "%1" ).arg( best_params[ pos++ ], 0, 'g', 8 ) );
      }

#if defined( BROADEN_SCALE_FIT )
      if ( cb_broaden_scale->isChecked() ) {
         le_broaden_scale  ->setText( QString( "%1" ).arg( best_params[ pos++ ], 0, 'g', 8 ) );
      }
#endif
   }

   // double loss = bblm_fit( 0, &( bblm_init_params[ 0 ] ) );

   // qDebug() 
   //    << " result: " << loss
   //    ;

   // final loss
   broaden_compute_one();
   double final_loss = broaden_compute_loss();
   US_Vector::printvector( QString( "broaden_fit original loss %1 final loss %2 : params" ).arg( original_loss ).arg( final_loss ), broaden_params() );

   if ( final_loss > original_loss ) {
      editor_msg( "darkred", "No improvement\n" );
      TSO << "No improvement!!\n";
      // reset to original params!
      size_t pos = 0;

      if ( cb_broaden_sigma->isChecked() ) {
         le_broaden_sigma   ->setText( QString( "%1" ).arg( bblm_init_params[ pos++ ], 0, 'g', 8 ) );
      }

      if ( cb_broaden_tau->isChecked() ) {
         le_broaden_tau     ->setText( QString( "%1" ).arg( bblm_init_params[ pos++ ], 0, 'g', 8 ) );
      }

      if ( cb_broaden_deltat->isChecked() ) {
         le_broaden_deltat  ->setText( QString( "%1" ).arg( bblm_init_params[ pos++ ], 0, 'g', 8 ) );
      }

      if ( cb_broaden_baseline->isChecked() ) {
         le_broaden_baseline->setText( QString( "%1" ).arg( bblm_init_params[ pos++ ], 0, 'g', 8 ) );
      }

#if defined( BROADEN_SCALE_FIT )
      if ( cb_broaden_scale->isChecked() ) {
         le_broaden_scale->setText( QString( "%1" ).arg( bblm_init_params[ pos++ ], 0, 'g', 8 ) );
      }
#endif
      
   }

   // scan individual params

   if ( !final_refinement_only ) {
      set < QCheckBox * > cb_active;
      set < QCheckBox * > cb_all;

      cb_all.insert( cb_broaden_sigma );
      if ( cb_broaden_sigma->isChecked() ) {
         cb_active.insert( cb_broaden_sigma );
      }

      cb_all.insert( cb_broaden_tau );
      if ( cb_broaden_tau->isChecked() ) {
         cb_active.insert( cb_broaden_tau );
      }

      cb_all.insert( cb_broaden_deltat );
      if ( cb_broaden_deltat->isChecked() ) {
         cb_active.insert( cb_broaden_deltat );
      }

      cb_all.insert( cb_broaden_baseline );
      if ( cb_broaden_baseline->isChecked() ) {
         cb_active.insert( cb_broaden_baseline );
      }

#if defined( BROADEN_SCALE_FIT )
      cb_all.insert( cb_broaden_scale );
      if ( cb_broaden_scale->isChecked() ) {
         cb_active.insert( cb_broaden_scale );
      }
#endif

      if ( cb_active.size() > 1 ) {
         for ( auto const cb : cb_active ) {
            for ( auto const cb_off : cb_all ) {
               cb_off->setChecked( cb_off == cb );
            }
            TSO << "single fit running\n";
            broaden_lm_fit();
         }
         // restore checkboxes when done
         for ( auto const cb : cb_active ) {
            cb->setChecked( true );
         }         
      }
   }
   // & a final refinement
   if ( !final_refinement_only ) {
      broaden_lm_fit( true );
   }
}

void US_Hydrodyn_Saxs_Hplc::broaden_fit() {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_fit() start\n";
   editor_msg( "darkblue", us_tr( "Broaden fit starting\n" ) );

   running = true;
   disable_all();

   broaden_lm_fit();

   broaden_compute_one( true );

   running = false;
   broaden_enables();
   progress->reset();
   editor_msg( "darkblue", us_tr( "Broaden fit finished\n" ) );
   
}

#if defined( INCLUDE_UNUSED_FUNCTIONS )
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
#endif

double US_Hydrodyn_Saxs_Hplc::broaden_compute_loss() {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_compute_loss() start\n";
   // crop curves to fit range
   // interpolate conc curve to data
   // compute chi^2 or rmsd

   if ( broaden_names.size() < 3 ) {
      editor_msg( "red", "broaden_compute_loss() called with no recomputed curve\n" );
      return DBL_MAX;
   }

   bool repeak_set = cb_broaden_repeak->isChecked();

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

#if defined( BROADEN_SCALE_FIT )
   double broaden_scale = le_broaden_scale->text().toDouble();
   double baseline = le_broaden_baseline  ->text().toDouble();
   for ( auto & I : new_conc_I ) {
      I *= broaden_scale;
      I += baseline;
   }
#else
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
#endif

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

void US_Hydrodyn_Saxs_Hplc::broaden_scale_compute() {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_scale_compute() start\n";
   // recompute scale based upon area fit, store in le_broaden_scale

   if ( broaden_names.size() < 3 ) {
      editor_msg( "red", "broaden_scale_compute() called with no broadened curve\n" );
      return;
   }

   bool repeak_set = cb_broaden_repeak->isChecked();

   if ( repeak_set ) {
      cb_broaden_repeak->setChecked( false );
      broaden_compute_one();
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
      if ( repeak_set ) {
         cb_broaden_repeak->setChecked( repeak_set );
         broaden_compute_one();
      }
      return;
   }

   static US_Saxs_Util usu;

   vector < double > new_conc_I;

   if ( !usu.linear_interpolate( conc_t, conc_I, ref_t, new_conc_I ) ) {
      editor_msg( "red", QString( "Error while fitting: interpolation error: %1\n" ).arg( usu.errormsg ) );
      if ( repeak_set ) {
         cb_broaden_repeak->setChecked( repeak_set );
         broaden_compute_one();
      }
      return;
   }

   // scale area

   double area_ref  = discrete_area_under_curve( ref_t, ref_I );
   double area_conc = discrete_area_under_curve( ref_t, new_conc_I );

   if ( area_conc <= 0 ) {
      editor_msg( "red", QString( "Error: area under broadened curve restricted to fitting range is not positive" ) );
      if ( repeak_set ) {
         cb_broaden_repeak->setChecked( repeak_set );
         broaden_compute_one();
      }
      return;
   }
  
#if defined( BROADEN_SCALE_FIT )
   double broaden_scale = area_ref / area_conc;
#else
   broaden_scale = area_ref / area_conc;
#endif
   le_broaden_scale->setText( QString( "%1" ).arg( broaden_scale, 0, 'g', 8 ) );
   cb_broaden_repeak->setChecked( repeak_set );
   broaden_compute_one();
}

vector < double > US_Hydrodyn_Saxs_Hplc::broaden_params() {
   return {
      le_broaden_sigma->text().toDouble()
      ,le_broaden_tau->text().toDouble()
      ,le_broaden_deltat->text().toDouble()
      ,le_broaden_baseline->text().toDouble()
#if defined( BROADEN_SCALE_FIT )
      ,le_broaden_scale->text().toDouble()
#endif
   };
}
