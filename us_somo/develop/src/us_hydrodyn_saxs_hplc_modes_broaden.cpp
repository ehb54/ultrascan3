#include "../include/us3_defines.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_hydrodyn_saxs_hplc.h"
#include "../include/us_band_broaden.h"

#define TSO QTextStream(stdout)

US_Band_Broaden ubb;

void US_Hydrodyn_Saxs_Hplc::broaden() {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden() start\n";
   broaden_org_selected = all_selected_files_set();

   disable_all();

   ubb.clear();

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

   set_selected( broaden_names );

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
   }
   set_selected( broaden_org_selected );
   mode_select( MODE_NORMAL );
}

void US_Hydrodyn_Saxs_Hplc::broaden_enables() {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_enables() start\n";

   le_broaden_tau_start         -> setEnabled( true );
   le_broaden_tau               -> setEnabled( true );
   le_broaden_tau_end           -> setEnabled( true );
   le_broaden_deltat_start      -> setEnabled( true );
   le_broaden_deltat            -> setEnabled( true );
   le_broaden_deltat_end        -> setEnabled( true );
   le_broaden_kernel_end        -> setEnabled( true );
   le_broaden_kernel_deltat     -> setEnabled( true );
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

void US_Hydrodyn_Saxs_Hplc::broaden_fit() {
   TSO << "US_Hydrodyn_Saxs_Hplc::broaden_fit() start\n";
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

   set_selected( broaden_names );
   
   // compute RMSD or Chi2

   // plot broadened plot
}
