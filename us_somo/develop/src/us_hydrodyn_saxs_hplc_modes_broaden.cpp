#include "../include/us3_defines.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_hydrodyn_saxs_hplc.h"
#include "../include/us_band_broaden.h"

#define TSO QTextStream(stdout)

static US_Band_Broaden ubb;

static bool broaden_ref_has_errors;

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

   if ( save ) {
      TSO << "US_Hydrodyn_Saxs_Hplc::broaden_done() save\n";
   } else {
      TSO << "US_Hydrodyn_Saxs_Hplc::broaden_done() cancel\n";
      if ( broaden_names.size() > 2 ) {
         set < QString > to_remove = { broaden_names.takeLast() };
         remove_files( to_remove );
      }
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

   le_broaden_tau_start         -> setEnabled( true );
   le_broaden_tau               -> setEnabled( true );
   le_broaden_tau_end           -> setEnabled( true );
   le_broaden_tau_delta         -> setEnabled( true );
   le_broaden_deltat_start      -> setEnabled( true );
   le_broaden_deltat            -> setEnabled( true );
   le_broaden_deltat_end        -> setEnabled( true );
   le_broaden_deltat_delta      -> setEnabled( true );
   le_broaden_kernel_end        -> setEnabled( true );
   le_broaden_kernel_deltat     -> setEnabled( true );
   cb_broaden_repeak            -> setEnabled( true );
   le_broaden_fit_range_start   -> setEnabled( true );
   le_broaden_fit_range_end     -> setEnabled( true );
   cb_broaden_kernel_type       -> setEnabled( true );
   pb_broaden_fit               -> setEnabled( true );
   pb_broaden_minimize          -> setEnabled( true );
   pb_broaden_reset             -> setEnabled( true );
   pb_wheel_cancel              -> setEnabled( true );
   pb_wheel_save                -> setEnabled( true );
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
}

void US_Hydrodyn_Saxs_Hplc::broaden_tau_start_text( const QString & ) {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_tau_start_text() start\n";
}

void US_Hydrodyn_Saxs_Hplc::broaden_tau_start_focus( bool ) {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_tau_start_focus() start\n";
}

void US_Hydrodyn_Saxs_Hplc::broaden_tau_text( const QString & ) {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_tau_text() start\n";
}

void US_Hydrodyn_Saxs_Hplc::broaden_tau_focus( bool hasFocus ) {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_tau_focus() start\n";
   if ( !hasFocus ) {
      broaden_compute_one();
   }
}

void US_Hydrodyn_Saxs_Hplc::broaden_tau_end_text( const QString & ) {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_tau_end_text() start\n";
}

void US_Hydrodyn_Saxs_Hplc::broaden_tau_end_focus( bool ) {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_tau_end_focus() start\n";
}

void US_Hydrodyn_Saxs_Hplc::broaden_tau_delta_text( const QString & ) {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_tau_delta_text() start\n";
}

void US_Hydrodyn_Saxs_Hplc::broaden_tau_delta_focus( bool ) {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_tau_delta_focus() start\n";
}

void US_Hydrodyn_Saxs_Hplc::broaden_deltat_start_text( const QString & ) {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_deltat_start_text() start\n";
}

void US_Hydrodyn_Saxs_Hplc::broaden_deltat_start_focus( bool ) {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_deltat_start_focus() start\n";
}

void US_Hydrodyn_Saxs_Hplc::broaden_deltat_text( const QString & ) {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_deltat_text() start\n";
}

void US_Hydrodyn_Saxs_Hplc::broaden_deltat_focus( bool hasFocus ) {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_deltat_focus() start\n";
   if ( !hasFocus ) {
      broaden_compute_one();
   }
}

void US_Hydrodyn_Saxs_Hplc::broaden_deltat_end_text( const QString & ) {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_deltat_end_text() start\n";
}

void US_Hydrodyn_Saxs_Hplc::broaden_deltat_end_focus( bool ) {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_deltat_end_focus() start\n";
}

void US_Hydrodyn_Saxs_Hplc::broaden_deltat_delta_text( const QString & ) {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_deltat_delta_text() start\n";
}

void US_Hydrodyn_Saxs_Hplc::broaden_deltat_delta_focus( bool ) {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_deltat_delta_focus() start\n";
}

void US_Hydrodyn_Saxs_Hplc::broaden_kernel_end_text( const QString & ) {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_kernel_end_text() start\n";
}

void US_Hydrodyn_Saxs_Hplc::broaden_kernel_end_focus( bool hasFocus ) {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_kernel_end_focus() start\n";
   if ( !hasFocus ) {
      broaden_compute_one();
   }
}

void US_Hydrodyn_Saxs_Hplc::broaden_kernel_deltat_text( const QString & ) {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_kernel_deltat_text() start\n";
}

void US_Hydrodyn_Saxs_Hplc::broaden_kernel_deltat_focus( bool hasFocus ) {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_kernel_deltat_focus() start\n";
   if ( !hasFocus ) {
      broaden_compute_one();
   }
}

void US_Hydrodyn_Saxs_Hplc::broaden_fit_range_start_text( const QString & ) {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_fit_range_start_text() start\n";
}

void US_Hydrodyn_Saxs_Hplc::broaden_fit_range_start_focus( bool /* hasFocus */ ) {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_fit_range_start_focus() start\n";
}

void US_Hydrodyn_Saxs_Hplc::broaden_fit_range_end_text( const QString & ) {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_fit_range_end_text() start\n";
}

void US_Hydrodyn_Saxs_Hplc::broaden_fit_range_end_focus( bool /* hasFocus */ ) {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_fit_range_end_focus() start\n";
}

void US_Hydrodyn_Saxs_Hplc::broaden_repeak_set() {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_repeak_set() start\n";
   broaden_compute_one();
}

void US_Hydrodyn_Saxs_Hplc::broaden_kernel_type_index() {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_kernel_type_index() start\n";
   broaden_compute_one();
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
                                             ,(US_Band_Broaden::kernel_type) cb_broaden_kernel_type->currentIndex()
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
      QString( "%1_tau%2_dt%3_kte%4_ktdelta%5_kernel%6.txt" )
      .arg( broaden_names[ 0 ] )
      .arg( le_broaden_tau->text() )
      .arg( le_broaden_deltat->text() )
      .arg( le_broaden_kernel_end->text() )
      .arg( le_broaden_kernel_deltat->text() )
      .arg( US_Band_Broaden::kernel_type_name( (US_Band_Broaden::kernel_type) cb_broaden_kernel_type->currentIndex() ) )
      ;

   vector < double > ts = f_qs[ broaden_names[ 0 ] ];
   double deltat = le_broaden_deltat->text().toDouble();
   for ( auto & t : ts ) {
      t += deltat;
   }
   
   add_plot( fname, ts, broadened, true, false );
   broaden_names << last_created_file;
   conc_files.insert( last_created_file );

   if ( cb_broaden_repeak->isChecked() ) {
      QStringList files;
      files << broaden_names[ 1 ];
      QString broadened_not_repeaked = broaden_names.takeLast();
      repeak( files, true, broadened_not_repeaked );
      broaden_names << last_repeak_name;
      set < QString > to_remove = { broadened_not_repeaked };
      remove_files( to_remove );
   }

   set_selected( broaden_names );
   
   // compute RMSD or Chi2

   // plot broadened plot
}

void US_Hydrodyn_Saxs_Hplc::broaden_fit() {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_fit() start\n";

   running = true;

   disable_all();

   double tau_start       = le_broaden_tau_start      ->text().toDouble();
   double tau_end         = le_broaden_tau_end        ->text().toDouble();
   double tau_delta       = le_broaden_tau_delta      ->text().toDouble();

   double deltat_start    = le_broaden_deltat_start   ->text().toDouble();
   double deltat_end      = le_broaden_deltat_end     ->text().toDouble();
   double deltat_delta    = le_broaden_deltat_delta   ->text().toDouble();

   double fit_range_start = le_broaden_fit_range_start->text().toDouble();
   double fit_range_end   = le_broaden_fit_range_end  ->text().toDouble();

#warning likely best in broaden_enables()
   if ( tau_start >= tau_end
        || tau_delta <= 0
        || deltat_start >= deltat_end
        || deltat_delta <= 0
        || fit_range_start >= fit_range_end
        ) {
      editor_msg( "red", "Incorrect fitting range\n" );
      broaden_enables();
      return;
   }

   double best_tau    = tau_start;
   double best_deltat = deltat_start;
   double best_loss   = DBL_MAX;

   QString summary = "tau,deltat,loss\n";

   for ( double tau = tau_start; tau <= tau_end; tau += tau_delta ) {
      for ( double deltat = deltat_start; deltat <= deltat_end; deltat += deltat_delta ) {
         le_broaden_tau   ->setText( QString( "%1" ).arg( tau ) ); // , 0, 'f', 6 ) );
         le_broaden_deltat->setText( QString( "%1" ).arg( deltat ) ); // , 0, 'f', 6 ) );
         broaden_compute_one();
         double this_loss = broaden_compute_loss();
         if ( this_loss == DBL_MAX ) {
            editor_msg( "red", "unable to compute loss, stopping fit" );
            running = false;
            broaden_enables();
            return;
         }

         summary += QString( "%1,%2,%3\n" ).arg( tau ).arg( deltat ).arg( this_loss );
         
         if ( best_loss > this_loss ) {
            best_loss   = this_loss;
            best_tau    = tau;
            best_deltat = deltat;
         }
      }
   }

   // {
   //    QString error;
   //    US_File_Util::putcontents( "broaden_fit_summary.csv", summary, error );
   // }

   le_broaden_tau   ->setText( QString( "%1" ).arg( best_tau ) ); // , 0, 'f', 6 ) );
   le_broaden_deltat->setText( QString( "%1" ).arg( best_deltat ) ); // , 0, 'f', 6 ) );
   lbl_broaden_msg  ->setText(
                              QString( "Last fit %1 : %2" )
                               .arg( broaden_ref_has_errors ? "nChi^2" : "RMSD" )
                               .arg( best_loss )
                              );

   broaden_compute_one();

   lbl_broaden_msg  ->setText(
                              QString( "Last fit %1 : %2" )
                               .arg( broaden_ref_has_errors ? "nChi^2" : "RMSD" )
                               .arg( best_loss )
                              );

   running = false;
   broaden_enables();
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
