#include "../include/us_hydrodyn.h"
#include "../include/us_revision.h"
#include "../include/us_hydrodyn_mals_saxs.h"
#include "../include/us_hydrodyn_mals_saxs_ciq.h"
#include "../include/us_hydrodyn_mals_saxs_cistarq.h"
#include "../include/us_hydrodyn_mals_saxs_dctr.h"
#include "../include/us_hydrodyn_mals_saxs_p3d.h"
#include "../include/us_hydrodyn_mals_saxs_fit.h"
#include "../include/us_hydrodyn_mals_saxs_fit_global.h"
#include "../include/us_lm.h"
#if QT_VERSION >= 0x040000
#include <qwt_scale_engine.h>
//Added by qt3to4:
#include <QTextStream>
#endif

#define TSO QTextStream(stdout)

#define SLASH QDir::separator()
#define Q_VAL_TOL 5e-6
// #define UHSH_VAL_DEC 8
#define UHSH_UV_CONC_FACTOR 1e0

// no gaussians version
bool US_Hydrodyn_Mals_Saxs::create_i_of_q_ng( set < QString > & fileset, double t_min, double t_max )
{
   QStringList files;
   for ( set < QString >::iterator it = fileset.begin();
         it != fileset.end();
         ++it )
   {
      files << *it;
   }
   return create_i_of_q_ng( files, t_min, t_max );
}

bool US_Hydrodyn_Mals_Saxs::create_i_of_q_ng( QStringList files, double t_min, double t_max )
{
   bool mode_testiq = ( current_mode == MODE_TESTIQ );

   if ( !mode_testiq )
   {
      QMessageBox::information( this,
                                windowTitle() + us_tr( ": Make I(q)" ),
                                us_tr( "I(q) will be made without Gaussians" )
                                );

      // for each selected file
      // extract q grid from file names
      editor_msg( "dark blue", us_tr( "Starting: Make I(q)" ) );
   }

   QString head = qstring_common_head( files, true );
   head = head.replace( QRegExp( "__It_q\\d*_$" ), "" );
   head = head.replace( QRegExp( "_q\\d*_$" ), "" );
   head = head.replace( QRegularExpression( "[\\[\\]{}]" ), "" );

   QRegExp rx_q     ( "_q(\\d+_\\d+)" );
   QRegExp rx_bl    ( "-bl(.\\d*_\\d+(|e.\\d+))-(.\\d*_\\d+(|e.\\d+))s" );
   QRegExp rx_bi    ( "-bi(.\\d*_\\d+(|e.\\d+))-(.\\d*_\\d+(|e.\\d+))s" );

   vector < QString > q_string;
   vector < double  > q;

   bool         any_bl = false;
   bool         any_bi = false;

   // get q 

   // map: [ timestamp ][ q_value ] = intensity

   map < double, map < double , double > > I_values;
   map < double, map < double , double > > e_values;

   map < double, bool > used_t;
   list < double >      tl;

   map < double, bool > used_q;
   list < double >      ql;

   bool                 use_errors = true;

   disable_all();

   map < QString, bool >    no_errors;
   map < QString, QString > zero_points;
   QStringList              qsl_no_errors;
   QStringList              qsl_zero_points;

   for ( unsigned int i = 0; i < ( unsigned int ) files.size(); i++ )
   {
      progress->setValue( i ); progress->setMaximum( files.size() * 2 );
      qApp->processEvents();
      if ( rx_q.indexIn( files[ i ] ) == -1 )
      {
         editor_msg( "red", QString( us_tr( "Error: Can not find q value in file name for %1" ) ).arg( files[ i ] ) );
         progress->reset();
         update_enables();
         return false;
      }
      ql.push_back( rx_q.cap( 1 ).replace( "_", "." ).toDouble() );
      if ( used_q.count( ql.back() ) )
      {
         editor_msg( "red", QString( us_tr( "Error: Duplicate q value in file name for %1" ) ).arg( files[ i ] ) );
         progress->reset();
         update_enables();
         return false;
      }
      used_q[ ql.back() ] = true;
         
      if ( rx_bl.indexIn( files[ i ] ) != -1 )
      {
         any_bl = true;
      }
      if ( rx_bi.indexIn( files[ i ] ) != -1 )
      {
         any_bi = true;
      }

      if ( !f_qs.count( files[ i ] ) )
      {
         editor_msg( "red", QString( us_tr( "Internal error: request to use %1, but not found in data" ) ).arg( files[ i ] ) );
      } else {
         for ( unsigned int j = 0; j < ( unsigned int ) f_qs[ files[ i ] ].size(); j++ )
         {
            if ( !mode_testiq || ( f_qs[ files[ i ] ][ j ] >= t_min && f_qs[ files[ i ] ][ j ] <= t_max ) )
            {
               I_values[ f_qs[ files[ i ] ][ j ] ][ ql.back() ] = f_Is[ files[ i ] ][ j ];
               if ( use_errors && f_errors[ files[ i ] ].size() == f_qs[ files[ i ] ].size() )
               {
                  e_values[ f_qs[ files[ i ] ][ j ] ][ ql.back() ] = f_errors[ files[ i ] ][ j ];
               } else {
                  if ( use_errors )
                  {
                     use_errors = false;
                     editor_msg( "dark red", QString( us_tr( "Notice: missing errors, first noticed in %1, so no errors at all" ) )
                                 .arg( files[ i ] ) );
                  }
               }
               if ( !used_t.count( f_qs[ files[ i ] ][ j ] ) )
               {
                  tl.push_back( f_qs[ files[ i ] ][ j ] );
                  used_t[ f_qs[ files[ i ] ][ j ] ] = true;
               }
            }
         }

         if ( !f_errors.count( files[ i ] ) ||
              f_errors[ files[ i ] ].size() != f_Is[ files[ i ] ].size() )
         {
            no_errors[ files[ i ] ] = true;
            qsl_no_errors           << files[ i ];
         } else {
            if ( !is_nonzero_vector( f_errors[ files[ i ] ] ) )
            {
               unsigned int zero_pts = 0;
               for ( unsigned int j = 0; j < ( unsigned int ) f_errors[ files[ i ] ].size(); j++ )
               {
                  if ( !mode_testiq || ( f_qs[ files[ i ] ][ j ] >= t_min && f_qs[ files[ i ] ][ j ] <= t_max ) )
                  {
                     if ( us_isnan( f_errors[ files[ i ] ][ j ] ) || f_errors[ files[ i ] ][ j ] == 0e0 )
                     {
                        zero_pts++;
                     }
                  }
               }
               zero_points[ files[ i ] ] = QString( "%1: %2 of %3 points" ).arg( files[ i ] ).arg( zero_pts ).arg( f_errors[ files[ i ] ].size() );
               qsl_zero_points           << zero_points[ files[ i ] ];
            }
         }

      }
   }

   tl.sort();

   vector < double > tv;
   for ( list < double >::iterator it = tl.begin();
         it != tl.end();
         it++ )
   {
      tv.push_back( *it );
   }

   ql.sort();

   vector < double  > qv;
   vector < QString > qv_string;
   for ( list < double >::iterator it = ql.begin();
         it != ql.end();
         it++ )
   {
      qv.push_back( *it );
      qv_string.push_back( QString( "%1" ).arg( *it ) );
   }


   QString qs_no_errors;
   QString qs_zero_points;

   if ( zero_points.size() || no_errors.size() )
   {
      unsigned int used = 0;

      QStringList qsl_list_no_errors;

      for ( unsigned int i = 0; i < ( unsigned int ) qsl_no_errors.size() && i < 12; i++ )
      {
         qsl_list_no_errors << qsl_no_errors[ i ];
         used++;
      }
      if ( qsl_list_no_errors.size() < qsl_no_errors.size() )
      {
         qsl_list_no_errors << QString( us_tr( "... and %1 more not listed" ) ).arg( qsl_no_errors.size() - qsl_list_no_errors.size() );
      }
      qs_no_errors = qsl_list_no_errors.join( "\n" );
      
      QStringList qsl_list_zero_points;
      for ( unsigned int i = 0; i < ( unsigned int ) qsl_zero_points.size() && i < 24 - used; i++ )
      {
         qsl_list_zero_points << qsl_zero_points[ i ];
      }
      if ( qsl_list_zero_points.size() < qsl_zero_points.size() )
      {
         qsl_list_zero_points << QString( us_tr( "... and %1 more not listed" ) ).arg( qsl_zero_points.size() - qsl_list_zero_points.size() );
      }
      qs_zero_points = qsl_list_zero_points.join( "\n" );
   }

   // bool   normalize_by_conc = false;
   bool   conc_ok           = false;

   double conv = 0e0;
   double psv  = 0e0;
   double I0se = 0e0;
   double conc_repeak = 1e0;
   
   vector < double > conc_spline_x;
   vector < double > conc_spline_y;
   vector < double > conc_spline_y2;

   if ( !mode_testiq ) {
      map < QString, QString > parameters;

      bool any_detector = false;
      if ( detector_uv ) {
         parameters[ "uv" ] = "true";
         any_detector = true;
      } else {
         if ( detector_ri ) {
            parameters[ "ri" ] = "true";
            any_detector = true;
         }
      }

      bool use_conc = true;
      if ( lbl_conc_file->text().isEmpty() ) {
         use_conc = false;
      } 

      if ( use_conc &&
           (
            !f_qs.count( lbl_conc_file->text() )
            || !f_Is.count( lbl_conc_file->text() )
            ) ) {
         editor_msg( "red", QString( us_tr( "Internal error: Concentration file %1 set, but associated data is incomplete.  Concentration disabled." ) ).arg( lbl_conc_file->text() ) );
         lbl_conc_file->setText( "" );
         use_conc = false;
      }

      if ( use_conc ) {
         if ( !any_detector ) {
            if ( parameters.count( "error" ) ) {
               parameters[ "error" ] += "\nYou must also select a detector type";
            } else {
               parameters[ "error" ] = "\nYou must select a detector type";
            }
         }
         parameters[ "ngmode" ] = "true";
         parameters[ "gaussians" ] = "1";
         US_Hydrodyn_Mals_Saxs_Ciq *mals_saxs_ciq = 
            new US_Hydrodyn_Mals_Saxs_Ciq(
                                          this,
                                          & parameters,
                                          this );
         US_Hydrodyn::fixWinButtons( mals_saxs_ciq );
         mals_saxs_ciq->exec();
         delete mals_saxs_ciq;

         if ( !parameters.count( "go" ) ) {
            progress->reset();
            update_enables();
            return false;
         }

         conv = parameters.count( "conv 0" ) ? parameters[ "conv 0" ].toDouble() : 0e0;
         psv  = parameters.count( "psv 0" ) ? parameters[ "psv 0" ].toDouble() : 0e0;
         if ( conv == 0e0 ||
              psv == 0e0 ) {
            progress->reset();
            update_enables();
            return false;
         }
            
         conc_ok = true;
         if ( parameters.count( "normalize" ) && parameters[ "normalize" ] == "true" ) {
            // normalize_by_conc = true;
         }

         if ( parameters.count( "I0se" ) ) {
            I0se = parameters[ "I0se" ].toDouble();
         }

         double detector_conv = 0e0;
         if ( detector_uv )
         {
            detector_conv = detector_uv_conv * UHSH_UV_CONC_FACTOR;
         }
         if ( detector_ri )
         {
            detector_conv = detector_ri_conv;
         }
         
         {
            QRegExp rx_repeak( "-rp(.\\d*(_|\\.)\\d+(|e.\\d+))" );
            if ( rx_repeak.indexIn( lbl_conc_file->text() ) != -1 )
            {
               conc_repeak = rx_repeak.cap( 1 ).replace( "_", "." ).toDouble();
               if ( conc_repeak == 0e0 )
               {
                  conc_repeak = 1e0;
                  editor_msg( "red", us_tr( "Error: concentration repeak scaling value extracted is 0, turning off back scaling" ) );
               } else {
                  editor_msg( "dark blue", QString( us_tr( "Notice: concentration scaling repeak value %1" ) ).arg( conc_repeak ) );
               }
            }
         }

         conc_spline_x = f_qs[ lbl_conc_file->text() ];
         conc_spline_y = f_Is[ lbl_conc_file->text() ];
         for ( int i = 0; i < (int) conc_spline_y.size(); ++i ) {
            conc_spline_y[ i ] *= detector_conv / ( conc_repeak * conv );
         }
         usu->natural_spline( conc_spline_x, conc_spline_y, conc_spline_y2 );
      }
   }

   running = true;

   // now for each I(t) distribute the I for each frame 

   // build up resulting curves

   // for each time, tv[ t ] 

   map < QString, bool > current_files;
   if ( !mode_testiq )
   {
      for ( int i = 0; i < (int)lb_files->count(); i++ )
      {
         current_files[ lb_files->item( i )->text() ] = true;
      }
   }

   for ( unsigned int t = 0; t < tv.size(); t++ )
   {
      progress->setValue( files.size() + t ); progress->setMaximum( files.size() + tv.size() );

      // build up an I(q)


      QString name = head + QString( "%1%2" )
         .arg( (any_bl || any_bi) ? "_bs" : "" )
         .arg( pad_zeros( tv[ t ], (int) tv.size() ) )
         .replace( ".", "_" )
         ;

      {
         int ext = 0;
         QString use_name = name;
         while ( current_files.count( use_name ) )
         {
            use_name = name + QString( "-%1" ).arg( ++ext );
         }
         name = use_name;
      }
         
      // TSO << "curve " << t << " tv[t] " << tv[t] << " name " << name << "\n";
      
      // TSO << QString( "name %1\n" ).arg( name );

      // now go through all the files to pick out the I values and errors and distribute amoungst the various gaussian peaks
      // we could also reassemble the original sum of gaussians curves as a comparative

      vector < double > I;
      vector < double > e;
      // vector < double > G;

      // vector < double > I_recon;
      // vector < double > G_recon;

      vector < double > this_used_pcts;
      double conc_factor = 0e0;
      if ( conc_ok ) {
         if ( !usu->apply_natural_spline( conc_spline_x, conc_spline_y, conc_spline_y2, tv[ t ], conc_factor ) ) {
            editor_msg( "red", QString( us_tr( "Error getting concentration from spline for frame %1, concentration set to zero." ) ).arg( tv[ t ] ) );
            conc_factor = 0e0;
         }
      }

      for ( unsigned int i = 0; i < ( unsigned int ) files.size(); i++ )
      {
         if ( !I_values.count( tv[ t ] ) )
         {
            editor_msg( "dark red", QString( us_tr( "Notice: I values missing frame/time = %1" ) ).arg( tv[ t ] ) );
         }

         if ( !I_values[ tv[ t ] ].count( qv[ i ] ) )
         {
            editor_msg( "red", QString( us_tr( "Notice: I values missing q = %1" ) ).arg( qv[ i ] ) );
            continue;
         }

         double tmp_I       = I_values[ tv[ t ] ][ qv[ i ] ];
         double tmp_e       = 0e0;

         if ( use_errors )
         {
            if ( !e_values.count( tv[ t ] ) )
            {
               editor_msg( "red", QString( us_tr( "Internal error: error values missing t %1" ) ).arg( tv[ t ] ) );
               running = false;
               update_enables();
               progress->reset();
               return false;
            }

            if ( !e_values[ tv[ t ] ].count( qv[ i ] ) )
            {
               editor_msg( "red", QString( us_tr( "Internal error: error values missing q %1" ) ).arg( qv[ i ] ) );
               running = false;
               update_enables();
               progress->reset();
               return false;
            }

            tmp_e = e_values[ tv[ t ] ][ qv[ i ] ];
         }
            
         I      .push_back( tmp_I );
         e      .push_back( tmp_e );
      } // for each file
         
      if ( mode_testiq )
      {
         testiq_created_names.push_back( name );
         testiq_created_t[ name ] = tv[ t ];
         testiq_created_q[ name ] = qv;
         testiq_created_I[ name ] = I;
         testiq_created_e[ name ] = e;
      } else {
         lb_created_files->addItem( name );
         lb_created_files->scrollToItem( lb_created_files->item( lb_created_files->count() - 1 ) );
         lb_files->addItem( name );
         lb_files->scrollToItem( lb_files->item( lb_files->count() - 1 ) );
         created_files_not_saved[ name ] = true;
   
         vector < QString > use_qv_string = qv_string;
         vector < double  > use_qv        = qv;
         vector < double  > use_I         = I;
         vector < double  > use_e         = e;

         f_pos       [ name ] = f_qs.size();
         f_qs_string [ name ] = qv_string;
         f_qs        [ name ] = qv;
         f_Is        [ name ] = I;
         f_errors    [ name ] = e;
         f_is_time   [ name ] = false;
         f_conc      [ name ] = conc_ok ? conc_factor : 0e0;
         f_psv       [ name ] = conc_ok ? psv : 0e0;
         f_I0se      [ name ] = conc_ok ? I0se : 0e0;
         f_time      [ name ] = tv[ t ];
         if ( conc_ok && conv ) {
            f_extc      [ name ] = conv;
         }
      }
   } // for each q value

   if ( !mode_testiq )
   {
      editor_msg( "dark blue", us_tr( "Finished: Make I(q)" ) );
      running = false;
   }
   progress->reset();
   update_enables();
   return true;
}


bool US_Hydrodyn_Mals_Saxs::create_i_of_q( set < QString > & fileset, double t_min, double t_max )
{
   QStringList files;
   for ( set < QString >::iterator it = fileset.begin();
         it != fileset.end();
         ++it )
   {
      files << *it;
   }
   return create_i_of_q( files, t_min, t_max );
}

bool US_Hydrodyn_Mals_Saxs::create_i_of_q( QStringList files, double t_min, double t_max )
{
   // us_qdebug( "ciq 0" );
   // for each selected file
   // extract q grid from file names
   bool mode_testiq = ( current_mode == MODE_TESTIQ );
   // bool force_errors_from_org = true;

   if ( !mode_testiq )
   {
      editor_msg( "dark blue", us_tr( "Starting: Make I(q)" ) );

      update_csv_conc();

      QStringList tmp_files;
      for ( unsigned int i = 0; i < (unsigned int) files.size(); i++ )
      {
         if ( files[ i ] != lbl_conc_file->text() )
         {
            tmp_files << files[ i ];
         }
      }
      files = tmp_files;
   }

   QString head = qstring_common_head( files, true );
   head = head.replace( QRegExp( "__It_q\\d*_$" ), "" );
   head = head.replace( QRegExp( "_q\\d*_$" ), "" );
   head = head.replace( QRegularExpression( "[\\[\\]{}]" ), "" );

   if ( !ggaussian_compatible( false ) )
   {
      editor_msg( "red", us_tr( "NOTICE: Some files selected have Gaussians with varying centers or a different number of Gaussians or centers, Please enter \"Global Gaussians\" with these files selected and then \"Keep\" before pressing \"Make I(q)\"" ) );
      return false;
   }

   QRegExp rx_q     ( "_q(\\d+_\\d+)" );
   QRegExp rx_bl    ( "-bl(.\\d*_\\d+(|e.\\d+))-(.\\d*_\\d+(|e.\\d+))s" );
   QRegExp rx_bi    ( "-bi(.\\d*_\\d+(|e.\\d+))-(.\\d*_\\d+(|e.\\d+))s" );

   vector < QString > q_string;
   vector < double  > q;
   vector < double  > bl_slope;
   vector < double  > bl_intercept;

   bool         any_bl = false;
   bool         any_bi = false;
   unsigned int bl_count = 0;
   unsigned int bi_count = 0;

   // get q and bl

   // map: bi_{delta|alpha}[ q_value ] = value

   map < double, double > bi_delta;
   map < double, double > bi_alpha;

   // map: [ timestamp ][ q_value ] = intensity

   map < double, map < double , double > > I_values;
   map < double, map < double , double > > e_values;

   map < double, bool > used_t;
   list < double >      tl;

   map < double, bool > used_q;
   list < double >      ql;

   bool                 use_errors = true;

   disable_all();

   map < QString, bool >    no_errors;
   map < QString, QString > zero_points;
   QStringList              qsl_no_errors;
   QStringList              qsl_zero_points;

   for ( unsigned int i = 0; i < ( unsigned int ) files.size(); i++ )
   {
      progress->setValue( i ); progress->setMaximum( files.size() * 2 );
      qApp->processEvents();
      if ( rx_q.indexIn( files[ i ] ) == -1 )
      {
         editor_msg( "red", QString( us_tr( "Error: Can not find q value in file name for %1" ) ).arg( files[ i ] ) );
         progress->reset();
         update_enables();
         return false;
      }
      ql.push_back( rx_q.cap( 1 ).replace( "_", "." ).toDouble() );
      if ( used_q.count( ql.back() ) )
      {
         editor_msg( "red", QString( us_tr( "Error: Duplicate q value in file name for %1" ) ).arg( files[ i ] ) );
         progress->reset();
         update_enables();
         return false;
      }
      used_q[ ql.back() ] = true;
         
      if ( rx_bl.indexIn( files[ i ] ) == -1 )
      {
         bl_slope    .push_back( 0e0 );
         bl_intercept.push_back( 0e0 );
      } else {
         // TSO << QString( "bl_cap 1 <%1>\n" ).arg( rx_bl.cap( 1 ) );
         // TSO << QString( "bl_cap 2 <%1>\n" ).arg( rx_bl.cap( 3 ) );
         bl_slope    .push_back( rx_bl.cap( 1 ).replace( "_", "." ).toDouble() );
         bl_intercept.push_back( rx_bl.cap( 3 ).replace( "_", "." ).toDouble() );
         // TSO << QString( "bl for file %1 slope %2 intercept %3\n" ).arg( i ).arg( bl_slope.back(), 0, 'g', 8 ).arg( bl_intercept.back(), 0, 'g', 8 ).toLatin1().data();
         bl_count++;
         any_bl = true;
      }

      if ( rx_bi.indexIn( files[ i ] ) == -1 )
      {
         // bi_delta    .push_back( 0e0 );
         // bi_alpha.push_back( 0e0 );
      } else {
         // TSO << QString( "bi_cap 1 <%1>\n" ).arg( rx_bi.cap( 1 ) );
         // TSO << QString( "bi_cap 2 <%1>\n" ).arg( rx_bi.cap( 3 ) );
         // bi_delta .push_back( rx_bi.cap( 1 ).replace( "_", "." ).toDouble() );
         // bi_alpha .push_back( rx_bi.cap( 3 ).replace( "_", "." ).toDouble() );
         // TSO << QString( "bi for file %1 delta  %2 alpha %3\n" ).arg( i ).arg( bi_delta.back(), 0, 'g', 8 ).arg( bi_alpha.back(), 0, 'g', 8 ).toLatin1().data();
         bi_delta[ ql.back() ] = rx_bi.cap( 1 ).replace( "_", "." ).toDouble();
         bi_alpha[ ql.back() ] = rx_bi.cap( 3 ).replace( "_", "." ).toDouble();
         bi_count++;
         any_bi = true;
      }

      if ( !f_qs.count( files[ i ] ) )
      {
         editor_msg( "red", QString( us_tr( "Internal error: request to use %1, but not found in data" ) ).arg( files[ i ] ) );
      } else {
         for ( unsigned int j = 0; j < ( unsigned int ) f_qs[ files[ i ] ].size(); j++ )
         {
            if ( !mode_testiq || ( f_qs[ files[ i ] ][ j ] >= t_min && f_qs[ files[ i ] ][ j ] <= t_max ) )
            {
               I_values[ f_qs[ files[ i ] ][ j ] ][ ql.back() ] = f_Is[ files[ i ] ][ j ];
               if ( use_errors && f_errors[ files[ i ] ].size() == f_qs[ files[ i ] ].size() )
               {
                  e_values[ f_qs[ files[ i ] ][ j ] ][ ql.back() ] = f_errors[ files[ i ] ][ j ];
               } else {
                  if ( use_errors )
                  {
                     use_errors = false;
                     editor_msg( "dark red", QString( us_tr( "Notice: missing errors, first noticed in %1, so no errors at all" ) )
                                 .arg( files[ i ] ) );
                  }
               }
               if ( !used_t.count( f_qs[ files[ i ] ][ j ] ) )
               {
                  tl.push_back( f_qs[ files[ i ] ][ j ] );
                  used_t[ f_qs[ files[ i ] ][ j ] ] = true;
               }
            }
         }

         if ( !f_errors.count( files[ i ] ) ||
              f_errors[ files[ i ] ].size() != f_Is[ files[ i ] ].size() )
         {
            no_errors[ files[ i ] ] = true;
            qsl_no_errors           << files[ i ];
         } else {
            if ( !is_nonzero_vector( f_errors[ files[ i ] ] ) )
            {
               unsigned int zero_pts = 0;
               for ( unsigned int j = 0; j < ( unsigned int ) f_errors[ files[ i ] ].size(); j++ )
               {
                  if ( us_isnan( f_errors[ files[ i ] ][ j ] ) || f_errors[ files[ i ] ][ j ] == 0e0 )
                  {
                     zero_pts++;
                  }
               }
               zero_points[ files[ i ] ] = QString( "%1: %2 of %3 points" ).arg( files[ i ] ).arg( zero_pts ).arg( f_errors[ files[ i ] ].size() );
               qsl_zero_points           << zero_points[ files[ i ] ];
            }
         }

      }
   }

   tl.sort();

   vector < double > tv;
   for ( list < double >::iterator it = tl.begin();
         it != tl.end();
         it++ )
   {
      tv.push_back( *it );
   }


   ql.sort();

   vector < double  > qv;
   vector < QString > qv_string;
   for ( list < double >::iterator it = ql.begin();
         it != ql.end();
         it++ )
   {
      qv.push_back( *it );
      qv_string.push_back( QString( "%1" ).arg( *it ) );
   }


   QString qs_no_errors;
   QString qs_zero_points;

   if ( zero_points.size() || no_errors.size() )
   {
      unsigned int used = 0;

      QStringList qsl_list_no_errors;

      for ( unsigned int i = 0; i < ( unsigned int ) qsl_no_errors.size() && i < 12; i++ )
      {
         qsl_list_no_errors << qsl_no_errors[ i ];
         used++;
      }
      if ( qsl_list_no_errors.size() < qsl_no_errors.size() )
      {
         qsl_list_no_errors << QString( us_tr( "... and %1 more not listed" ) ).arg( qsl_no_errors.size() - qsl_list_no_errors.size() );
      }
      qs_no_errors = qsl_list_no_errors.join( "\n" );
      
      QStringList qsl_list_zero_points;
      for ( unsigned int i = 0; i < ( unsigned int ) qsl_zero_points.size() && i < 24 - used; i++ )
      {
         qsl_list_zero_points << qsl_zero_points[ i ];
      }
      if ( qsl_list_zero_points.size() < qsl_zero_points.size() )
      {
         qsl_list_zero_points << QString( us_tr( "... and %1 more not listed" ) ).arg( qsl_zero_points.size() - qsl_list_zero_points.size() );
      }
      qs_zero_points = qsl_list_zero_points.join( "\n" );
   }

   bool save_gaussians;

   vector < double > conv;
   vector < double > psv ;
   double            I0se = 0e0;
   
   double conc_repeak = 1e0;
   
   bool normalize_by_conc = false;
   bool conc_ok           = false;

   bool sd_avg_local  = false;
   bool sd_drop_zeros = false;
   bool sd_keep_zeros = false;
   bool sd_set_pt1pct = false;
   bool save_sum      = false;

   bool sd_from_difference = false;

   bool conc_to_saxs  = false;

   if ( mode_testiq )
   { 
      save_gaussians = cb_testiq_from_gaussian->isChecked();
      // us_qdebug( QString( "ciq mode testiq save_gaussians %1 %2" ).arg( save_gaussians ? "true" : "false" ).arg( bl_count ) );
      bl_count = 0;
      // us_qdebug( QString( "now ciq mode testiq save_gaussians %1 %2" ).arg( save_gaussians ? "true" : "false" ).arg( bl_count ) );
   } else {
      map < QString, QString > parameters;
      bool no_conc = false;
      if ( bl_count )
      {
         parameters[ "baseline" ] = 
            QString( us_tr( "Add back the baselines when making I(q).  Baselines were found for %1 of the %2 curves" ) )
            .arg( bl_count )
            .arg( files.size() );
      }

      if ( bl_count )
      {
         parameters[ "baseline" ] = 
            QString( us_tr( "Add back the baselines when making I(q).  Baselines were found for %1 of the %2 curves" ) )
            .arg( bl_count )
            .arg( files.size() );
      }

      if ( bi_count )
      {
         parameters[ "integralbaseline" ] = QString( "%1" ).arg( bi_count );
      }

      parameters[ "gaussians" ] = QString( "%1" ).arg( f_gaussians[ files[ 0 ] ].size() / gaussian_type_size );

      bool any_detector = false;
      if ( detector_uv )
      {
         parameters[ "uv" ] = "true";
         any_detector = true;
      } else {
         if ( detector_ri )
         {
            parameters[ "ri" ] = "true";
            any_detector = true;
         }
      }

      if ( lbl_conc_file->text().isEmpty() )
      {
         parameters[ "error" ] = QString( us_tr( "Concentration controls disabled: no concentration file set" ) );
         no_conc = true;
      } else {
         if ( !f_gaussians.count( lbl_conc_file->text() ) )
         {
            parameters[ "error" ] = QString( us_tr( "Concentration controls disabled: no Gaussians defined for concentration file" ) );
            no_conc = true;
         } else {
            if ( f_gaussians[ lbl_conc_file->text() ].size() / gaussian_type_size  != f_gaussians[ files[ 0 ] ].size() / gaussian_type_size )
            {
               parameters[ "error" ] = 
                  QString( us_tr( "Concentration controls disabled: Concentration file Gaussian count (%1)\n does not match global curves Gaussian count (%2)" ) )
                  .arg( f_gaussians[ lbl_conc_file->text() ].size() )
                  .arg( f_gaussians[ files[ 0 ] ].size() / gaussian_type_size )
                  ;
               no_conc = true;
            }
         }
      }

      if ( !no_conc )
      {
         QRegExp rx_repeak( "-rp(.\\d*(_|\\.)\\d+(|e.\\d+))" );
         if ( rx_repeak.indexIn( lbl_conc_file->text() ) != -1 )
         {
            conc_repeak = rx_repeak.cap( 1 ).replace( "_", "." ).toDouble();
            if ( conc_repeak == 0e0 )
            {
               conc_repeak = 1e0;
               editor_msg( "red", us_tr( "Error: concentration repeak scaling value extracted is 0, turning off back scaling" ) );
            } else {
               editor_msg( "dark blue", QString( us_tr( "Notice: concentration scaling repeak value %1" ) ).arg( conc_repeak ) );
            }
         } else {
            editor_msg( "dark red", us_tr( "Notice: no concentration repeak scaling value found" ) );
         }
      } else {
         TSO << "no conc\n";
      }
         
      if ( !any_detector )
      {
         if ( parameters.count( "error" ) )
         {
            parameters[ "error" ] += "\nYou must also select a detector type";
         } else {
            parameters[ "error" ] = "\nYou must select a detector type";
         }
      }            

      parameters[ "no_errors"   ] = qs_no_errors;
      parameters[ "zero_points" ] = qs_zero_points;
      if ( U_EXPT )
      {
         parameters[ "expert_mode" ] = "true";
      }

      //       TSO << "parameters b4 ciq:\n";
      //       for ( map < QString, QString >::iterator it = parameters.begin();
      //             it != parameters.end();
      //             it++ )
      //       {
      //          TSO << QString( "%1:%2\n" ).arg( it->first ).arg( it->second );
      //       }
      //       TSO << "end parameters b4 ciq:\n";

      parameters[ "mals_saxs_cb_makeiq_avg_peaks" ] = ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_saxs_cb_makeiq_avg_peaks" ];
      parameters[ "mals_saxs_makeiq_avg_peaks" ] = ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_saxs_makeiq_avg_peaks" ];
      
      US_Hydrodyn_Mals_Saxs_Ciq *mals_saxs_ciq = 
         new US_Hydrodyn_Mals_Saxs_Ciq(
                                       this,
                                       & parameters,
                                       this );
      US_Hydrodyn::fixWinButtons( mals_saxs_ciq );
      mals_saxs_ciq->exec();
      delete mals_saxs_ciq;
      
      //       TSO << "parameters:\n";
      //       for ( map < QString, QString >::iterator it = parameters.begin();
      //             it != parameters.end();
      //             it++ )
      //       {
      //          TSO << QString( "%1:%2\n" ).arg( it->first ).arg( it->second );
      //       }
      //       TSO << "end parameters:\n";

      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_saxs_cb_makeiq_avg_peaks" ] = parameters[ "mals_saxs_cb_makeiq_avg_peaks" ];
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_saxs_makeiq_avg_peaks" ] = parameters[ "mals_saxs_makeiq_avg_peaks" ];

      if ( bl_count && ( !parameters.count( "add_baseline" ) || parameters[ "add_baseline" ] == "false" ) )
      {
         bl_count = 0;
         TSO << "ciq: bl off\n";
      }

      if ( parameters.count( "save_as_pct_iq" ) && parameters[ "save_as_pct_iq" ] == "true" )
      {
         save_gaussians = false;
         TSO << "ciq: save_gaussians false\n";
      } else {
         save_gaussians = true;
         TSO << "ciq: save_gaussians true\n";
      }
      if ( parameters.count( "save_sum" ) && parameters[ "save_sum" ] == "true" )
      {
         save_sum = true;
      }
      if ( parameters.count( "sd_source" ) && parameters[ "sd_source" ] == "difference" )
      {
         sd_from_difference = true;
         TSO << "ciq: sd_from_difference true\n";
      } else {
         sd_from_difference = false;
         TSO << "ciq: sd_from_difference false\n";
      }
      if ( !parameters.count( "go" ) )
      {
         progress->reset();
         update_enables();
         return false;
      }

      if ( parameters.count( "make_ng" ) &&
           parameters[ "make_ng" ] == "true" )
      {
         progress->reset();
         return create_i_of_q_ng( files );
      }

      if ( !no_conc )
      {
         for ( unsigned int i = 0; i < ( unsigned int ) f_gaussians[ files[ 0 ] ].size() / gaussian_type_size; i++ )
         {
            conv.push_back( parameters.count( QString( "conv %1" ).arg( i ) ) ?
                            parameters[ QString( "conv %1" ).arg( i ) ].toDouble() : 0e0 );
            psv .push_back( parameters.count( QString( "psv %1" ).arg( i ) ) ?
                            parameters[ QString( "psv %1" ).arg( i ) ].toDouble() : 0e0 );
         }
         conc_ok = true;
         if ( parameters.count( "normalize" ) && parameters[ "normalize" ] == "true" )
         {
            normalize_by_conc = true;
         }
      }

      if ( parameters.count( "I0se" ) )
      {
         I0se = parameters[ "I0se" ].toDouble();
      }

      if ( sd_from_difference )
      {
         sd_avg_local  = parameters[ "sd_zero_avg_local_sd"  ] == "true";
         sd_keep_zeros = parameters[ "sd_zero_keep_as_zeros" ] == "true";
         sd_set_pt1pct = parameters[ "sd_zero_set_to_pt1pct" ] == "true";
      } else {
         sd_drop_zeros = parameters[ "zero_drop_points"   ] == "true";
         sd_avg_local  = parameters[ "zero_avg_local_sd"  ] == "true";
         sd_keep_zeros = parameters[ "zero_keep_as_zeros" ] == "true";
      }

      TSO << QString( "sd_avg_local  %1\n"
                       "sd_drop_zeros %2\n"
                       "sd_set_pt1pct %3\n"
                       "sd_keep_zeros %4\n" )
         .arg( sd_avg_local  ? "true" : "false" )
         .arg( sd_drop_zeros ? "true" : "false" )
         .arg( sd_set_pt1pct ? "true" : "false" )
         .arg( sd_keep_zeros ? "true" : "false" )
         ;


      if ( conc_ok && parameters[ "conc_to_saxs" ] == "true" )
      {
         conc_to_saxs = true;
      }
   }

   //    if ( bl_count &&
   //         QMessageBox::question(this, 
   //                               this->windowTitle(),
   //                               QString( us_tr( "Baselines were found for %1 of the %2 curves\n"
   //                                            "Do you want to add back the baselines when making I(q)?" ) )
   //                               .arg( bl_count )
   //                               .arg( files.size() ),
   //                               us_tr( "&Yes" ),
   //                               us_tr( "&No" ),
   //                               QString(),
   //                               0,
   //                               1
   //                               ) == 1 )
   //    {
   //       TSO << "not using baselines\n";
   //       bl_count = 0;
   //    }

   //    bool save_gaussians = 
   //         QMessageBox::question(this, 
   //                               this->windowTitle(),
   //                               us_tr( "Save as Gaussians or a percent of the original I(q)?" ),
   //                               us_tr( "&Gaussians" ),
   //                               us_tr( "Percent of &I(q)" ),
   //                               QString(),
   //                               0,
   //                               1
   //                               ) == 0;
   running = true;

   vector < vector < double > > concs;

   vector < double > conc_gaussians;
   vector < double > ref_gaussians = f_gaussians[ files[ 0 ] ];

   if ( conc_ok )
   {
      conc_gaussians = f_gaussians[ lbl_conc_file->text() ];
   }

   if ( conc_ok && conc_to_saxs )
   {
      vector < double > new_conc_gaussians;

      vector < double > tmp_gc( gaussian_type_size );
      vector < double > tmp_gr( gaussian_type_size );
      vector < double > tmp_gn( gaussian_type_size );

      double conc_area;

      for ( unsigned int i = 0; i < ( unsigned int )conv.size(); i++ )
      {
         tmp_gc[ 0 ] = conc_gaussians[ 0 + i * gaussian_type_size ];
         tmp_gr[ 0 ] = ref_gaussians [ 0 + i * gaussian_type_size ];

         tmp_gc[ 1 ] = conc_gaussians[ 1 + i * gaussian_type_size ];
         tmp_gr[ 1 ] = ref_gaussians [ 1 + i * gaussian_type_size ];

         tmp_gc[ 2 ] = conc_gaussians[ 2 + i * gaussian_type_size ];
         tmp_gr[ 2 ] = ref_gaussians [ 2 + i * gaussian_type_size ];

         if ( dist1_active )
         {
            tmp_gc[ 3 ] = conc_gaussians[ 3 + i * gaussian_type_size ];
            tmp_gr[ 3 ] = ref_gaussians [ 3 + i * gaussian_type_size ];
            if ( dist2_active )
            {
               tmp_gc[ 4 ] = conc_gaussians[ 4 + i * gaussian_type_size ];
               tmp_gr[ 4 ] = ref_gaussians [ 4 + i * gaussian_type_size ];
            }
         }
         
         conc_area = tmp_gc[ 0 ] * tmp_gc[ 2 ] * M_SQRT2PI;

         // keep center, width, distortions:
         tmp_gn = tmp_gr; 

         // compute new amplitude
         tmp_gn[ 0 ] = conc_area / ( M_SQRT2PI * tmp_gn[ 2 ] );

         QString qs = QString( us_tr( "Concentration Gaussian %1: center %2 area %3" ) )
            .arg( i + 1 )
            .arg( tmp_gc[ 1 ] )
            .arg( conc_area );
         editor_msg( "dark blue", qs );
         for ( int j = 0; j < (int) gaussian_type_size; ++j )
         {
            new_conc_gaussians.push_back( tmp_gn[ j ] );
         }
         add_plot( QString( lbl_conc_file->text() + "_sas_adjusted_pk%1" ).arg( i + 1 ), tv, compute_gaussian( tv, tmp_gn ), true, false );
      }
      // us_qdebug( US_Vector::qs_vector3( "conc, ref, new conc", conc_gaussians, ref_gaussians, new_conc_gaussians ) );
      conc_gaussians = new_conc_gaussians;
      add_plot( lbl_conc_file->text() + "_sas_adjusted" , tv, compute_gaussian_sum( tv, conc_gaussians), true, false );
   }

   // if ( conc_ok && conc_to_saxs )
   // {
   //    editor_msg( "red", "Adjusting concentration curve to SAXS optimized values is not yet enabled" );
   //    running = false;
   //    progress->reset();
   //    update_enables();
   //    return false;
   // }

   if ( normalize_by_conc || conc_ok )
   {
      // test, produce conc curves for each gaussian
      for ( unsigned int i = 0; i < ( unsigned int )conv.size(); i++ )
      {
         // add_plot( QString( "conc_mg_per_ml_peak%1" ).arg( i + 1 ), tv, conc_curve( tv, i, conv[ i ] ), true, false );
         // alt method
         {
            double detector_conv = 0e0;
            if ( detector_uv )
            {
               detector_conv = detector_uv_conv * UHSH_UV_CONC_FACTOR;
            }
            if ( detector_ri )
            {
               detector_conv = detector_ri_conv;
            }
            vector < double > tmp_g( gaussian_type_size );
            tmp_g[ 0 ] = conc_gaussians[ 0 + i * gaussian_type_size ] * detector_conv / ( conc_repeak * conv[ i ] );
            tmp_g[ 1 ] = conc_gaussians[ 1 + i * gaussian_type_size ];
            tmp_g[ 2 ] = conc_gaussians[ 2 + i * gaussian_type_size ];
            if ( dist1_active )
            {
               tmp_g[ 3 ] = conc_gaussians[ 3 + i * gaussian_type_size ];
               if ( dist2_active )
               {
                  tmp_g[ 4 ] = conc_gaussians[ 4 + i * gaussian_type_size ];
               }
            }
            concs.push_back( compute_gaussian( tv, tmp_g ) );
            add_plot( QString( "conc_mg_per_ml_peak%1" ).arg( i + 1 ), tv, concs.back(), true, false );
         }
      }
   }

   // now for each I(t) distribute the I for each frame according to the gaussians
   // !!! **              ---- >>>> check for baseline, if present, optionally add back

   // compute all gaussians over q range

   // [ file ][ gaussian ][ time ]
   vector < vector < vector < double > > > fg; // a vector of the individual gaussians
   // [ file ][ time ]
   vector < vector < double > >            fs; // a vector of the gaussian sums
   vector < vector < double > >            g_area;      // a vector of the gaussian area
   vector < double >                       g_area_sum; // a vector of the gaussian area


   for ( unsigned int i = 0; i < (unsigned int) files.size(); i++ )
   {
      vector < vector < double > > tmp_v;
      vector < double >            tmp_sum;
      vector < double >            tmp_area;
      double                       tmp_area_sum = 0e0;

      for ( unsigned int j = 0; j < ( unsigned int ) f_gaussians[ files[ i ] ].size(); j += gaussian_type_size )
      {
         vector < double > tmp_g( gaussian_type_size );
         tmp_g[ 0 ] = f_gaussians[ files[ i ] ][ 0 + j ];
         tmp_g[ 1 ] = f_gaussians[ files[ i ] ][ 1 + j ];
         tmp_g[ 2 ] = f_gaussians[ files[ i ] ][ 2 + j ];
         if ( dist1_active )
         {
            tmp_g[ 3 ] = f_gaussians[ files[ i ] ][ 3 + j ];
            if ( dist2_active )
            {
               tmp_g[ 4 ] = f_gaussians[ files[ i ] ][ 4 + j ];
            }
         }

         // US_Vector::printvector( QString( "file %1 gaussian %2 params" ).arg( files[ i ] ).arg( 1 + ( j / gaussian_type_size ) ), tmp_g );

         vector < double > tmp = compute_gaussian( tv, tmp_g );
         tmp_v.push_back( tmp );
         if ( j )
         {
            for ( unsigned int k = 0; k < tmp.size(); k++ )
            {
               tmp_sum[ k ] += tmp[ k ];
            }
         } else {
            tmp_sum = tmp;
         }

         tmp_area.push_back( tmp_g[ 0 ] * tmp_g[ 2 ] * M_SQRT2PI );
         tmp_area_sum += tmp_area.back();

         // add_plot( QString( "%1_fg_g%2" ).arg( files[ i ] ).arg( 1 + ( j / gaussian_type_size ) ), tv, tmp, true, false );

      }
      fg.push_back( tmp_v );
      fs.push_back( tmp_sum );

      for ( unsigned int j = 0; j < ( unsigned int ) tmp_area.size(); j++ )
      {
         tmp_area[ j ] /= tmp_area_sum;
      }
         
      g_area    .push_back( tmp_area );
      g_area_sum.push_back( tmp_area_sum );
      // US_Vector::printvector( QString( "areas file %1 (sum %2)" ).arg( i ).arg( tmp_area_sum, 0, 'g', 8 ), tmp_area );
      // add_plot( QString( "fg_%1_gsum" ).arg( i ), tv, tmp_sum, true, false );
   }

   // US_Vector::printvector( "area sums", g_area_sum );

   unsigned int num_of_gauss = ( unsigned int ) gaussians.size() / gaussian_type_size;

   if ( mode_testiq && (unsigned int) rb_testiq_gaussians.size() != num_of_gauss )
   {
      editor_msg( "red", QString( us_tr( "Test I(q) internal error: Gaussian count mismatch %1 %2" ) )
                  .arg( rb_testiq_gaussians.size() )
                  .arg( num_of_gauss ) );
      return false;
   }

   if ( !mode_testiq )
   {
      QFile f( ((US_Hydrodyn *)us_hydrodyn)->somo_dir + QDir::separator() + "saxs" + QDir::separator() + "tmp" +  QDir::separator() + "mals_saxs_frac.csv" );
      if ( f.open ( QIODevice::WriteOnly ) )
      {
         QTextStream ts( &f );
         ts << QString( "\"file\",\"time\",\"peak\",\"%\"\n" );
         for ( unsigned int t = 0; t < tv.size(); t++ )
         {
            for ( unsigned int i = 0; i < ( unsigned int ) files.size(); i++ )
            {
               for ( unsigned int g = 0; g < num_of_gauss; g++ )
               {
                  double frac_of_gaussian_sum;
                  if ( fs[ i ][ t ] == 0e0 )
                  {
                     frac_of_gaussian_sum = -1e0;
                  } else {
                     frac_of_gaussian_sum = fg[ i ][ g ][ t ] / fs[ i ][ t ];
                  }
                  ts << QString( "\"%1\",%2,%3,%4\n" )
                     .arg( files[ i ] )
                     .arg( g + 1 )
                     .arg( tv[ t ] )
                     .arg( 100e0 * frac_of_gaussian_sum )
                     ;
               }
            }
         }
         f.close();
      }
   }

   // build up resulting curves

   map < double, map < double , double > > bi_recon;
   // us_qdebug( QString( "here build up resulting curves save sum %1 any_bi %2" )
   //         .arg( save_sum ? "yes" : "no" )
   //         .arg( any_bi ? "yes" : "no" )
   //         );

   if ( save_sum && any_bi ) {

      // us_qdebug( "qv delta alpha" );
      // for ( int i = 0; i < (int) qv.size(); ++i ) {
      //    us_qdebug( QString( "%1 %2 %3" )
      //            .arg( qv[ i ] )
      //            .arg( QString( bi_delta.count( qv[ i ] ) ? QString( "%1" ).arg( bi_delta[ qv[ i ] ] ) : QString( "na" ) ) )
      //            .arg( QString( bi_alpha.count( qv[ i ] ) ? QString( "%1" ).arg( bi_alpha[ qv[ i ] ] ) : QString( "na" ) ) )
      //            );
      // }

      for ( int q = 0; q < (int) qv.size(); ++q ) {
         double bl = 0e0;
         if ( !bi_alpha.count( qv[ q ] ) ) {
            // us_qdebug( QString( "bi_alpha missing for qv %1" ).arg( qv[ q ] ) );
            editor_msg( "red", 
                        QString( us_tr( "Warning: could not find alpha value for integral baseline reconstruction for q %1 A^-1" ) )
                        .arg( qv[ q ] ) );
            bi_alpha[ qv[ q ] ] = 0e0;
         }

         for ( int t = 0; t < (int) tv.size(); ++t ) {
            if ( !I_values.count( tv[ t ] ) ) {
               // us_qdebug( QString( "I_values[%1] missing" ).arg( tv[ t ] ) );
               continue;
            }
            if ( !I_values[ tv[ t ] ].count( qv[ q ] ) ) {
               // // us_qdebug( QString( "I_values[%1][%2] missing" ).arg( tv[ t ] ).arg( qv[ q ] ) );
               continue;
            }
            bl += I_values[ tv[ t ] ][ qv[ q ] ] * bi_alpha[ qv[ q ] ];
            bi_recon[ tv[ t ] ][ qv[ q ] ] += bl;
         }
         // us_qdebug( QString( "bl recomputed %1 delta %2 absdiff %3" )
         //         .arg( bl )
         //         .arg( QString( bi_delta.count( qv[ q ] ) ? QString( "%1" ).arg( bi_delta[ qv[ q ] ] ) : QString( "na" ) ) )
         //         .arg( bi_delta.count( qv[ q ] ) ? fabs( bi_delta[ qv[ q ] ] - bl ) : 1e99 ) 
         //         );
      }
   }      

   // for each time, tv[ t ] 

   // TSO << QString( "num of gauss %1\n" ).arg( num_of_gauss );

   bool reported_gs0 = false;

   map < QString, bool > current_files;
   if ( !mode_testiq )
   {
      for ( int i = 0; i < (int)lb_files->count(); i++ )
      {
         current_files[ lb_files->item( i )->text() ] = true;
      }
   }

   // find the peak of each gaussian and establish limits
   bool use_cutoff = false;
   vector < unsigned int > tc_min( num_of_gauss );
   vector < unsigned int > tc_max( num_of_gauss );

   vector < unsigned int > tcpeak_min( num_of_gauss );
   vector < unsigned int > tcpeak_max( num_of_gauss );

   bool avg_peaks = false;
   vector < set < QString > > avg_peaks_names;

   {
      double cutoff_frac = 
         ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_saxs_cb_makeiq_cutmax_pct" ] == "true" ?
         ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_saxs_makeiq_cutmax_pct" ].toDouble() * 0.01 :
         0e0;
         
      if ( cutoff_frac > 0e0 ) {
         use_cutoff = true;

         for ( unsigned int g = 0; g < num_of_gauss; g++ ) {
            vector < double > tmp_g( gaussian_type_size );
            tmp_g[ 0 ] = f_gaussians[ files[ 0 ] ][ 0 + g * gaussian_type_size ];
            tmp_g[ 1 ] = f_gaussians[ files[ 0 ] ][ 1 + g * gaussian_type_size ];
            tmp_g[ 2 ] = f_gaussians[ files[ 0 ] ][ 2 + g * gaussian_type_size ];
            if ( dist1_active )
            {
               tmp_g[ 3 ] = f_gaussians[ files[ 0 ] ][ 3 + g * gaussian_type_size ];
               if ( dist2_active )
               {
                  tmp_g[ 4 ] = f_gaussians[ files[ 0 ] ][ 4 + g * gaussian_type_size ];
               }
            }
            vector < double > gI = compute_gaussian( tv, tmp_g );
            double max = 0e0;
            for ( int j = 0; j < (int) gI.size(); ++j ) {
               if ( max < gI[ j ] ) {
                  max = gI[ j ];
               }
            }
            double cutmax = cutoff_frac * max;
            tc_min[ g ] = 0;
            tc_max[ g ] = tv.size() - 1;

            {
               int j = 0;
               for ( ; j < (int) gI.size(); ++j ) {
                  if ( gI[ j ] >= cutmax ) {
                     tc_min[ g ] = j;
                     break;
                  }
               }
               for ( ; j < (int) gI.size(); ++j ) {
                  if ( gI[ j ] <= cutmax ) {
                     tc_max[ g ] = j - 1;
                     break;
                  }
               }
            }
            
            editor_msg( "dark blue", QString( us_tr( "Cutoff for Gaussian %1 is [%2,%3]" ) ).arg( g + 1 ).arg( tc_min[ g ] ).arg( tc_max[ g ] ) );
         }
      } 
   }

   {
      avg_peaks = ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_saxs_cb_makeiq_avg_peaks" ] == "true";

      if ( avg_peaks ) {
         double peak_frac = 
            1e0 - ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_saxs_makeiq_avg_peaks" ].toDouble() * 0.01;

         avg_peaks_names.resize( num_of_gauss );
         
         if ( peak_frac > 0e0 ) {
         
            for ( unsigned int g = 0; g < num_of_gauss; g++ ) {
               vector < double > tmp_g( gaussian_type_size );
               tmp_g[ 0 ] = f_gaussians[ files[ 0 ] ][ 0 + g * gaussian_type_size ];
               tmp_g[ 1 ] = f_gaussians[ files[ 0 ] ][ 1 + g * gaussian_type_size ];
               tmp_g[ 2 ] = f_gaussians[ files[ 0 ] ][ 2 + g * gaussian_type_size ];
               if ( dist1_active )
               {
                  tmp_g[ 3 ] = f_gaussians[ files[ 0 ] ][ 3 + g * gaussian_type_size ];
                  if ( dist2_active )
                  {
                     tmp_g[ 4 ] = f_gaussians[ files[ 0 ] ][ 4 + g * gaussian_type_size ];
                  }
               }
               vector < double > gI = compute_gaussian( tv, tmp_g );
               double max = 0e0;
               for ( int j = 0; j < (int) gI.size(); ++j ) {
                  if ( max < gI[ j ] ) {
                     max = gI[ j ];
                  }
               }
               double cutmax = peak_frac * max;
               tcpeak_min[ g ] = 0;
               tcpeak_max[ g ] = tv.size() - 1;

               {
                  int j = 0;
                  for ( ; j < (int) gI.size(); ++j ) {
                     if ( gI[ j ] >= cutmax ) {
                        tcpeak_min[ g ] = j;
                        break;
                     }
                  }
                  for ( ; j < (int) gI.size(); ++j ) {
                     if ( gI[ j ] <= cutmax ) {
                        tcpeak_max[ g ] = j - 1;
                        break;
                     }
                  }
               }
               editor_msg( "dark blue", QString( us_tr( "Cutoff for peak averaging of Gaussian %1 is [%2,%3]" ) ).arg( g + 1 ).arg( tcpeak_min[ g ] ).arg( tcpeak_max[ g ] ) );
            }
         }
      }
   }

   for ( unsigned int t = 0; t < tv.size(); t++ )
   {
      progress->setValue( files.size() + t ); progress->setMaximum( files.size() + tv.size() );
      // for each gaussian 
      vector < double > gsI;
      vector < double > gse;
      vector < double > gsG;
      vector < double > gsI_recon;
      vector < double > gsG_recon;

      // for "sd by differece"
      vector < vector < double > > used_pcts;
      vector < QString >           used_names;

      for ( unsigned int g = 0; g < num_of_gauss; g++ )
      {
         bool skip_create = false;
         bool add_avg_peaks = false;
         if ( mode_testiq && !rb_testiq_gaussians[ g ]->isChecked() )
         {
            continue;
         }
         if ( use_cutoff &&
              ( t < tc_min[ g ] || t > tc_max[ g ] ) ) {
            skip_create = true;
         }
         if ( avg_peaks && t >= tcpeak_min[ g ] && t <= tcpeak_max[ g ] ) {
            add_avg_peaks = true;
         }
         // us_qdebug( QString( "ciq create g loop t %1 g %2 go on" ).arg( t ).arg( g + 1 ) );
         // build up an I(q)
         double conc_factor = 0e0;
         double norm_factor = 1e0;
         QString qs_fwhm;
         if ( ( conc_ok || normalize_by_conc ) && concs[ g ][ t ] > 0e0 )
         {
            // us_qdebug( QString( "ciq conc stuff" ) );
            conc_factor = concs[ g ][ t ];
            norm_factor = 1e0 / conc_factor;

            double detector_conv = 0e0;
            if ( detector_uv )
            {
               detector_conv = detector_uv_conv * UHSH_UV_CONC_FACTOR;
            }
            if ( detector_ri )
            {
               detector_conv = detector_ri_conv;
            }

            vector < double > tmp_g( gaussian_type_size );
            tmp_g[ 0 ] = conc_gaussians[ 0 + g * gaussian_type_size ] * detector_conv / ( conc_repeak * conv[ g ] );
            tmp_g[ 1 ] = conc_gaussians[ 1 + g * gaussian_type_size ];
            tmp_g[ 2 ] = conc_gaussians[ 2 + g * gaussian_type_size ];
            if ( dist1_active )
            {
               tmp_g[ 3 ] = conc_gaussians[ 3 + g * gaussian_type_size ];
               if ( dist2_active )
               {
                  tmp_g[ 4 ] = conc_gaussians[ 4 + g * gaussian_type_size ];
               }
            }

            double center = tmp_g[ 1 ];
            double width  = tmp_g[ 2 ];
            double fwhm   = 2.354820045e0 * width;
            //             TSO << QString( "peak %1 center %2 fwhm %3 t %4 tv[t] %5\n" )
            //                .arg( g + 1 )
            //                .arg( center )
            //                .arg( fwhm )
            //                .arg( t )
            //                .arg( tv[ t ] );
            if ( tv[ t ] >= center - fwhm && tv[ t ] <= center + fwhm )
            {
               qs_fwhm = "_fwhm";
            }
         }

         QString name = head + QString( "%1%2%3_pk%4%5_t%6" )
            .arg( save_gaussians  ? "_G" : "" )
            .arg( (any_bl || any_bi)   ? "_bs" : "" )
            .arg( bl_count ? "ba" : "" )
            .arg( g + 1 )
            .arg( normalize_by_conc ? 
                  QString( "%1_cn%2" )
                  .arg( qs_fwhm )
                  .arg( conc_factor, 0, 'g', 6 ) : QString( "" ) )
            .arg( pad_zeros( tv[ t ], (int) tv.size() ) )
            .replace( ".", "_" )
            ;

         {
            int ext = 0;
            QString use_name = name;
            while ( current_files.count( use_name ) )
            {
               use_name = name + QString( "-%1" ).arg( ++ext );
            }
            name = use_name;
         }

         // TSO << QString( "name %1\n" ).arg( name );

         // now go through all the files to pick out the I values and errors and distribute amoungst the various gaussian peaks
         // we could also reassemble the original sum of gaussians curves as a comparative

         vector < double > I;
         vector < double > e;
         vector < double > G;

         vector < double > I_recon;
         vector < double > G_recon;

         vector < double > this_used_pcts;

         for ( unsigned int i = 0; i < ( unsigned int ) files.size(); i++ )
         {
            if ( !I_values.count( tv[ t ] ) )
            {
               editor_msg( "dark red", QString( us_tr( "Notice: I values missing frame/time = %1" ) ).arg( tv[ t ] ) );
               //                running = false;
               //                update_enables();
               //                return;
               continue;
            }

            if ( !I_values[ tv[ t ] ].count( qv[ i ] ) )
            {
               editor_msg( "red", QString( us_tr( "Notice: I values missing q = %1" ) ).arg( qv[ i ] ) );
               //                running = false;
               //                update_enables();
               //                return;
               continue;
            }

            double tmp_I       = I_values[ tv[ t ] ][ qv[ i ] ];
            double tmp_e       = 0e0;
            double tmp_G       = fg[ i ][ g ][ t ];

            // us_qdebug( QString( "ciq: pulling tmp_G i %1 g %2 t %3" ).arg( i ).arg( g + 1 ).arg( t ) );

            double frac_of_gaussian_sum;
            if ( fs[ i ][ t ] == 0e0 )
            {
               if ( !reported_gs0 )
               {
                  TSO << QString( "Notice: file %1 t %2 gaussian sum is zero (further instances ignored)\n" ).arg( i ).arg( t );
                  reported_gs0 = true;
               }
               frac_of_gaussian_sum = 1e0 / ( double ) num_of_gauss;
            } else {
               frac_of_gaussian_sum = tmp_G / fs[ i ][ t ];
            }

            if ( use_errors )
            {
               // us_qdebug( QString( "ciq: use errors" ) );
               
               if ( !e_values.count( tv[ t ] ) )
               {
                  editor_msg( "red", QString( us_tr( "Internal error: error values missing t %1" ) ).arg( tv[ t ] ) );
                  if ( !mode_testiq )
                  {
                     running = false;
                  }
                  update_enables();
                  progress->reset();
                  return false;
               }

               if ( !e_values[ tv[ t ] ].count( qv[ i ] ) )
               {
                  editor_msg( "red", QString( us_tr( "Internal error: error values missing q %1" ) ).arg( qv[ i ] ) );
                  if ( !mode_testiq )
                  {
                     running = false;
                  }
                  update_enables();
                  progress->reset();
                  return false;
               }

               tmp_e = e_values[ tv[ t ] ][ qv[ i ] ];
            }
            
            tmp_I *= frac_of_gaussian_sum;
            // if ( !force_errors_from_org )
            // {
            // 141212 (change error mode ... propagate original errors)  tmp_e *= frac_of_gaussian_sum;
            // }

            if ( sd_from_difference )
            {
               this_used_pcts.push_back( frac_of_gaussian_sum );
            }

            double tmp_I_recon = tmp_I;
            double tmp_G_recon = tmp_G;

            // add back baseline linear
            if ( any_bl )
            {
               double pct_area = 1e0; // 1e0 / ( double ) num_of_gauss; // g_area[ i ][ g ];
               double ofs = ( bl_intercept[ i ] + tv[ t ] * bl_slope[ i ] ) * pct_area;
               if ( bl_count )
               {
                  tmp_I += ofs;
                  tmp_G += ofs;
               }
               if ( !g )
               {
                  tmp_I_recon += ofs;
                  tmp_G_recon += ofs;
               }
            }

            I      .push_back( tmp_I );
            e      .push_back( tmp_e );
            G      .push_back( tmp_G );
            I_recon.push_back( tmp_I_recon );
            G_recon.push_back( tmp_G_recon );
         } // for each file
         
         if ( !mode_testiq )
         {
            if ( g )
            {
               for ( unsigned int m = 0; m < ( unsigned int ) qv.size(); m++ )
               {
                  gsI[ m ]       += I[ m ];
                  gse[ m ]       += e[ m ];
                  gsG[ m ]       += G[ m ];
                  gsI_recon[ m ] += I_recon[ m ];
                  gsG_recon[ m ] += G_recon[ m ];
               }
            } else {
               gsI       = I;
               gsG       = G;
               gse       = e;
               gsI_recon = I_recon;
               gsG_recon = G_recon;
            }

            // add to csv conc stuff?

            if ( !skip_create ) {
               lb_created_files->addItem( name );
               lb_created_files->scrollToItem( lb_created_files->item( lb_created_files->count() - 1 ) );
               lb_files->addItem( name );
               lb_files->scrollToItem( lb_files->item( lb_files->count() - 1 ) );
               created_files_not_saved[ name ] = true;
               if ( add_avg_peaks ) {
                  avg_peaks_names[ g ].insert( name );
               }
            }
         }

         vector < QString > use_qv_string = qv_string;
         vector < double  > use_qv        = qv;
         vector < double  > use_I         = save_gaussians ? G : I;
         vector < double  > use_e         = e;

         // add_plot( QString( "testG" ),
         //           qv, 
         //           use_I, 
         //           use_e,
         //           false, 
         //           false );

         if ( conc_ok )
         {
            update_csv_conc();

            for ( unsigned int i = 0; i < csv_conc.data.size(); i++ )
            {
               if ( csv_conc.data[ i ].size() > 1 &&
                    csv_conc.data[ i ][ 0 ] == name )
               {
                  csv_conc.data[ i ][ 1 ] = QString( "%1" ).arg( conc_factor );
               }
            }
            if ( conc_widget )
            {
               conc_window->refresh( csv_conc );
            }
         }


         if ( normalize_by_conc && norm_factor != 1e0 )
         {
            for ( unsigned int i = 0; i < use_I.size(); i++ )
            {
               use_I[ i ] *= norm_factor;
            }
         }

         if ( sd_from_difference )
         {
            if ( !skip_create ) {
               used_names.push_back( name );
               used_pcts .push_back( this_used_pcts );
            }
         } else {
            if ( sd_drop_zeros )
            {
               vector < QString > tmp_qv_string;
               vector < double  > tmp_qv       ;
               vector < double  > tmp_I        ; 
               vector < double  > tmp_e        ;
               for ( unsigned int i = 0; i < ( unsigned int ) use_e.size(); i++ )
               {
                  if ( use_e[ i ] > 0e0 )
                  {
                     tmp_qv_string.push_back( use_qv_string[ i ] );
                     tmp_qv       .push_back( use_qv       [ i ] );
                     tmp_I        .push_back( use_I        [ i ] );
                     tmp_e        .push_back( use_e        [ i ] );
                  }
               }
               use_qv_string = tmp_qv_string;
               use_qv        = tmp_qv;
               use_I         = tmp_I;
               use_e         = tmp_e;
            }

            if ( sd_avg_local )
            {
               bool more_zeros;
               unsigned int tries = 0;
               do 
               {
                  more_zeros = false;
                  tries++;
                  for ( unsigned int i = 0; i < ( unsigned int ) use_e.size(); i++ )
                  {
                     if ( use_e[ i ] <= 0e0 )
                     {
                        if ( !i && i < ( unsigned int ) use_e.size() - 1 )
                        {
                           use_e[ i ] = use_e[ i + 1 ];
                           if ( use_e[ i ] <= 0e0 )
                           {
                              more_zeros = true;
                           }
                           continue;
                        }
                        if ( i == ( unsigned int ) use_e.size() - 1 && i > 0 )
                        {
                           use_e[ i ] = use_e[ i - 1 ];
                           if ( use_e[ i ] <= 0e0 )
                           {
                              more_zeros = true;
                           }
                           continue;
                        }
                        if ( i < ( unsigned int ) use_e.size() - 1 && i > 0 )
                        {
                           use_e[ i ] = ( use_e[ i - 1 ] + use_e[ i + 1 ] ) * 5e-1;
                           if ( use_e[ i ] <= 0e0 )
                           {
                              more_zeros = true;
                           }
                           continue;
                        }
                        more_zeros = true;
                     }
                  }
               } while ( more_zeros && tries < 5 );
               if ( more_zeros )
               {
                  editor_msg( "dark red", us_tr( "Warning: too many adjacent S.D. zeros, could not set all S.D.'s to non-zero values" ) );
               }
            }
            
            if ( sd_set_pt1pct )
            {
               for ( unsigned int i = 0; i < ( unsigned int ) use_e.size(); i++ )
               {
                  if ( use_e[ i ] <= 0e0 )
                  {
                     use_e[ i ] = use_I[ i ] * 0.001;
                  }
               }
            }
         }

         if ( mode_testiq )
         {
            testiq_created_names.push_back( name );
            testiq_created_t[ name ] = tv[ t ];
            testiq_created_q[ name ] = use_qv;
            testiq_created_I[ name ] = use_I;
            testiq_created_e[ name ] = use_e;
         } else {
            if ( !skip_create ) {
               f_pos       [ name ] = f_qs.size();
               f_qs_string [ name ] = use_qv_string;
               f_qs        [ name ] = use_qv;
               f_Is        [ name ] = use_I;
               f_errors    [ name ] = use_e;
               f_is_time   [ name ] = false;
               f_conc      [ name ] = conc_factor;
               f_psv       [ name ] = psv.size() > g ? psv[ g ] : 0e0;
               f_I0se      [ name ] = I0se;
               f_time      [ name ] = tv[ t ];
               if ( conv.size() > g )
               {
                  f_extc      [ name ] = conv[ g ];
               }

               {
                  vector < double > tmp;
                  f_gaussians  [ name ] = tmp;
               }
            }
         }
      } // for each gaussian

      if ( sd_from_difference )
      {
         // NEVER CURRENTLY IN TESTIQ MODE ... POSSIBLY LATER
         vector < double >  total_e;
         for ( unsigned int i = 0; i < gsG.size(); i++ )
         {
            total_e.push_back( fabs( gsG[ i ] - gsI[ i ] ) );
         }

         // US_Vector::printvector( "total_e", total_e );

         for ( unsigned int i = 0; i < ( unsigned int ) used_names.size(); i++ )
         {
            vector < QString > use_qv_string = f_qs_string[ used_names[ i ] ];
            vector < double >  use_qv        = f_qs       [ used_names[ i ] ];
            vector < double >  use_I         = f_Is       [ used_names[ i ] ];
            vector < double >  use_e         = total_e;
            
            // US_Vector::printvector( QString( "used_pcts for %1" ).arg( used_names[ i ] ), used_pcts[ i ] );

            if ( use_errors )
            {
               for ( unsigned int j = 0; j < ( unsigned int ) use_e.size(); j++ )
               {
                  use_e[ j ] *= used_pcts[ i ][ j ];
                  use_e[ j ] = sqrt( 
                                    use_e[ j ] * use_e[ j ] +
                                    f_errors[ used_names[ i ] ][ j ] * f_errors[ used_names[ i ] ][ j ] 
                                     );
               }
            } else {
               for ( unsigned int j = 0; j < ( unsigned int ) use_e.size(); j++ )
               {
                  use_e[ j ] *= used_pcts[ i ][ j ];
               }               
            }

            // US_Vector::printvector( "use_e", use_e );

            if ( sd_drop_zeros )
            {
               vector < QString > tmp_qv_string;
               vector < double  > tmp_qv       ;
               vector < double  > tmp_I        ; 
               vector < double  > tmp_e        ;
               for ( unsigned int i = 0; i < ( unsigned int ) use_e.size(); i++ )
               {
                  if ( use_e[ i ] > 0e0 )
                  {
                     tmp_qv_string.push_back( use_qv_string[ i ] );
                     tmp_qv       .push_back( use_qv       [ i ] );
                     tmp_I        .push_back( use_I        [ i ] );
                     tmp_e        .push_back( use_e        [ i ] );
                  }
               }
               use_qv_string = tmp_qv_string;
               use_qv        = tmp_qv;
               use_I         = tmp_I;
               use_e         = tmp_e;
            }

            if ( sd_avg_local )
            {
               bool more_zeros;
               unsigned int tries = 0;
               do 
               {
                  more_zeros = false;
                  tries++;
                  for ( unsigned int i = 0; i < ( unsigned int ) use_e.size(); i++ )
                  {
                     if ( use_e[ i ] <= 0e0 )
                     {
                        if ( !i && i < ( unsigned int ) use_e.size() - 1 )
                        {
                           use_e[ i ] = use_e[ i + 1 ];
                           if ( use_e[ i ] <= 0e0 )
                           {
                              more_zeros = true;
                           }
                           continue;
                        }
                        if ( i == ( unsigned int ) use_e.size() - 1 && i > 0 )
                        {
                           use_e[ i ] = use_e[ i - 1 ];
                           if ( use_e[ i ] <= 0e0 )
                           {
                              more_zeros = true;
                           }
                           continue;
                        }
                        if ( i < ( unsigned int ) use_e.size() - 1 && i > 0 )
                        {
                           use_e[ i ] = ( use_e[ i - 1 ] + use_e[ i + 1 ] ) * 5e-1;
                           if ( use_e[ i ] <= 0e0 )
                           {
                              more_zeros = true;
                           }
                           continue;
                        }
                        more_zeros = true;
                     }
                  }
               } while ( more_zeros && tries < 5 );
               if ( more_zeros )
               {
                  editor_msg( "dark red", us_tr( "Warning: too many adjacent S.D. zeros, could not set all S.D.'s to non-zero values" ) );
               }
            }
            
            if ( sd_set_pt1pct )
            {
               for ( unsigned int i = 0; i < ( unsigned int ) use_e.size(); i++ )
               {
                  if ( use_e[ i ] <= 0e0 )
                  {
                     use_e[ i ] = use_I[ i ] * 0.001;
                  }
               }
            }
            
            if ( mode_testiq )
            {
               testiq_created_q[ used_names[ i ] ] = use_qv;
               testiq_created_I[ used_names[ i ] ] = use_I;
               testiq_created_e[ used_names[ i ] ] = use_e;
            } else {
               f_qs_string[ used_names[ i ] ] = use_qv_string;
               f_qs       [ used_names[ i ] ] = use_qv;
               f_Is       [ used_names[ i ] ] = use_I;
               f_errors   [ used_names[ i ] ] = use_e;
            }
         }
      }         
      if ( !mode_testiq && save_sum )
      {
         if ( save_gaussians )
         {
            add_plot( QString( "sumG%1_T%2" ).arg( (any_bl || any_bi) ? "_bs" : "" ).arg( pad_zeros( tv[ t ], (int) tv.size() ) ), qv, gsG, gse, false, false );
            if ( any_bl )
            {
               add_plot( QString( "sumG_bsba_T%1" ).arg( pad_zeros( tv[ t ], (int) tv.size() ) ), qv, gsG_recon, gse, false, false );
            }
            if ( any_bi )
            {
               vector < double > gsG_bi_recon = gsG_recon;
               if ( bi_recon.count( tv[ t ] ) ) {
                  for ( int q = 0; q < (int) qv.size(); ++q ) {
                     gsG_bi_recon[ q ] += bi_recon[ tv[ t ] ].count( qv[ q ] ) ? bi_recon[ tv[ t ] ][ qv[ q ] ] : 0e0;
                  }
               }
               add_plot( QString( "sumG_bsbai_T%1" ).arg( pad_zeros( tv[ t ], (int) tv.size() ) ), qv, gsG_bi_recon, gse, false, false );
            }               
         } else {
            add_plot( QString( "sumI%1_T%2" ).arg( (any_bl || any_bi) ? "_bs" : "" ).arg( pad_zeros( tv[ t ], (int) tv.size() ) ), qv, gsI, gse, false, false );
            if ( any_bl )
            {
               add_plot( QString( "sumI_bsba_T%1" ).arg( pad_zeros( tv[ t ], (int) tv.size() ) ), qv, gsI_recon, gse, false, false );
            }
            if ( any_bi )
            {
               vector < double > gsI_bi_recon = gsI_recon;
               if ( bi_recon.count( tv[ t ] ) ) {
                  for ( int q = 0; q < (int) qv.size(); ++q ) {
                     gsI_bi_recon[ q ] += bi_recon[ tv[ t ] ].count( qv[ q ] ) ? bi_recon[ tv[ t ] ][ qv[ q ] ] : 0e0;
                  }
               }
               add_plot( QString( "sumI_bsbai_T%1" ).arg( pad_zeros( tv[ t ], (int) tv.size() ) ), qv, gsI_bi_recon, gse, false, false );
            }               
         }
      }

      // add_plot( QString( "sumIr_T%1" ).arg( pad_zeros( tv[ t ], (int) tv.size() ) ), qv, gsI_recon, gse, false, false );
      // add_plot( QString( "sumGr_T%1" ).arg( pad_zeros( tv[ t ], (int) tv.size() ) ), qv, gsG_recon, gse, false, false );
   } // for each q value

   // avg_peaks: select & average & conc normalize peaks

   if ( avg_peaks ) {
      QString peak_tag = QString( "co%1_" ).arg( ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_saxs_makeiq_avg_peaks" ] );
      set < QString > selected_files_set = all_selected_files_set();
      set < QString > final_files_set;
      for ( unsigned int i = 0; i < (unsigned int) avg_peaks_names.size(); ++i ) {
         if ( avg_peaks_names[ i ].size() ) {
            set_selected( avg_peaks_names[ i ] );
            // then average, normalize
            avg( all_selected_files(), QString( peak_tag ) );
            if ( conc_ok ) {
               set < QString > last_selected;
               last_selected.insert( lb_files->item( lb_files->count() - 1 )->text() );
               set_selected( last_selected );
               normalize();
               final_files_set.insert( lb_files->item( lb_files->count() - 1 )->text() );
            } else {
               final_files_set.insert( lb_files->item( lb_files->count() - 1 )->text() );
            }
         }
      }
      if ( final_files_set.size() < 9 &&
           !cb_eb->isChecked() ) {
         cb_eb->setChecked( true );
      }
      use_line_width = 1;
      set_selected( final_files_set );
      if ( !axis_y_log ) {
         axis_y();
      }
      rescale();
   }

   if ( !mode_testiq )
   {
      editor_msg( "dark blue", us_tr( "Finished: Make I(q)" ) );
      running = false;
   }
   progress->reset();
   update_enables();
   return true;
}

bool US_Hydrodyn_Mals_Saxs::create_unified_ggaussian_target( bool do_init, bool only_init_unset )
{
   QStringList files = all_selected_files();
   return create_unified_ggaussian_target( files, do_init, only_init_unset );
}

bool US_Hydrodyn_Mals_Saxs::create_unified_ggaussian_target( QStringList & files, bool do_init, bool only_init_unset )
{
   unified_ggaussian_ok = false;

   org_gaussians = gaussians;
   // US_Vector::printvector( "cugt: org_gauss", org_gaussians );

   unified_ggaussian_params          .clear( );

   unified_ggaussian_files           = files;
   unified_ggaussian_curves          = files.size();
   unified_ggaussian_qvals           .clear( );

   unified_ggaussian_use_errors      = true;

   // for testing
   // unified_ggaussian_use_errors      = false;

   unified_ggaussian_gaussians_size  = ( unsigned int ) gaussians.size() / gaussian_type_size;

   if ( do_init )
   {
      unified_ggaussian_errors_skip = false;
      if ( ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "mals_saxs_cb_gg_smooth" )
           && ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_saxs_cb_gg_smooth" ] == "true"
           && ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "mals_saxs_gg_smooth" )
           && ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_saxs_gg_smooth" ].toInt() > 0 ) {
         editor_msg( "darkred", QString( "Experimental smoothing points %1 will be applied\n" ).arg( ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_saxs_gg_smooth" ].toInt() ) );
      }

      if ( !initial_ggaussian_fit( files, only_init_unset ) )
      {
         progress->reset();
         return false;
      }
   }

   QRegExp rx_q     ( "_q(\\d+_\\d+)" );

   common_size   = 0;
   per_file_size = 0;
   is_common.clear( );
   offset.clear( );
   describe_unified_common   = "";
   describe_unified_per_file = "";

   // height:
   is_common.push_back( false           );  // height always variable
   offset   .push_back( per_file_size++ );  // first variable entry
   describe_unified_per_file += "height ";

   // center
   is_common.push_back( true            );  // center always common
   offset   .push_back( common_size++   );  // first common entry
   describe_unified_common += "center ";

   // width
   if ( cb_fix_width->isChecked() )
   {
      is_common.push_back( true );
      offset   .push_back( common_size++   );  // first common entry
      describe_unified_common += "width ";
   } else {
      is_common.push_back( false );
      offset   .push_back( per_file_size++ );  // first variable entry
      describe_unified_per_file += "width ";
   }

   if ( dist1_active )
   {
      if ( cb_fix_dist1->isChecked() )
      {
         is_common.push_back( true );
         offset   .push_back( common_size++   );  // first common entry
         describe_unified_common += "dist1 ";
      } else {
         is_common.push_back( false );
         offset   .push_back( per_file_size++ );  // first variable entry
         describe_unified_per_file += "dist1 ";
      }
      if ( dist2_active )
      {
         if ( cb_fix_dist2->isChecked() )
         {
            is_common.push_back( true );
            offset   .push_back( common_size++   );  // first common entry
            describe_unified_common += "dist2 ";
         } else {
            is_common.push_back( false );
            offset   .push_back( per_file_size++ );  // first variable entry
            describe_unified_per_file += "dist2 ";
         }
      }
   }

   // push back centers first

   for ( unsigned int i = 0; i < ( unsigned int ) gaussians.size(); i += gaussian_type_size )
   {
      unified_ggaussian_params.push_back( gaussians[ 1 + i ] ); // center
      if ( cb_fix_width->isChecked() )
      {
         unified_ggaussian_params.push_back( gaussians[ 2 + i ] );
      }
      if ( dist1_active && cb_fix_dist1->isChecked() )
      {
         unified_ggaussian_params.push_back( gaussians[ 3 + i ] );
      }
      if ( dist2_active && cb_fix_dist2->isChecked() )
      {
         unified_ggaussian_params.push_back( gaussians[ 4 + i ] );
      }
   }

   // now push back all the file specific amplitude & widths (& dist1, dist2 )

   map < QString, bool >    no_errors;
   map < QString, QString > zero_points;
   QStringList              qsl_no_errors;
   QStringList              qsl_zero_points;

   for ( unsigned int i = 0; i < ( unsigned int ) files.size(); i++ )
   {
      if ( !f_gaussians.count( files[ i ] ) )
      {
         editor_msg( "red", QString( us_tr( "Internal error: %1 does not have a gaussian set" ) ).arg( files[ i ] ) );
         return false;
      }
      if ( f_gaussians[ files[ i ] ].size() != gaussians.size() )
      {
         editor_msg( "red", QString( us_tr( "Internal error: %1 has an incompatible gaussian set" ) ).arg( files[ i ] ) );
         return false;
      }
      
      if ( rx_q.indexIn( files[ i ] ) == -1 )
      {
         editor_msg( "red", QString( us_tr( "Error: Can not find q value in file name for %1" ) ).arg( files[ i ] ) );
         return false;
      }

      unified_ggaussian_qvals.push_back( rx_q.cap( 1 ).replace( "_", "." ).toDouble() );
      

      for ( unsigned int j = 0; j < ( unsigned int ) f_gaussians[ files[ i ] ].size(); j += gaussian_type_size )
      {
         unified_ggaussian_params.push_back( f_gaussians[ files[ i ] ][ 0 + j ] ); // height
         if ( !cb_fix_width->isChecked() )
         {
            unified_ggaussian_params.push_back( f_gaussians[ files[ i ] ][ 2 + j ] ); // width
         }
         if ( dist1_active && !cb_fix_dist1->isChecked() )
         {
            unified_ggaussian_params.push_back( f_gaussians[ files[ i ] ][ 3 + j ] ); // dist1
         }
         if ( dist2_active && !cb_fix_dist2->isChecked() )
         {
            unified_ggaussian_params.push_back( f_gaussians[ files[ i ] ][ 4 + j ] ); // dist2
         }
      }

      if ( cb_sd_weight->isChecked() )
      {
         if ( !f_errors.count( files[ i ] ) ||
              f_errors[ files[ i ] ].size() != f_Is[ files[ i ] ].size() )
         {
            TSO << QString( "file %1 no errors %2\n" )
               .arg( files[ i ] ).arg( f_errors.count( files[ i ] ) ?
                                       QString( "errors %1 vs Is %2" )
                                       .arg( f_errors[ files[ i ] ].size() )
                                       .arg( f_Is[ files[ i ] ].size() )
                                       :
                                       "at all" );
            no_errors[ files[ i ] ] = true;
            qsl_no_errors           << files[ i ];
         } else {
            if ( !is_nonzero_vector( f_errors[ files[ i ] ] ) )
            {
               unsigned int zero_pts = 0;
               for ( unsigned int j = 0; j < ( unsigned int ) f_errors[ files[ i ] ].size(); j++ )
               {
                  if ( us_isnan( f_errors[ files[ i ] ][ j ] ) || f_errors[ files[ i ] ][ j ] == 0e0 )
                  {
                     zero_pts++;
                  }
               }
               zero_points[ files[ i ] ] = QString( "%1: %2 of %3 points" ).arg( files[ i ] ).arg( zero_pts ).arg( f_errors[ files[ i ] ].size() );
               qsl_zero_points           << zero_points[ files[ i ] ];
            }
         }
      }
   }

   if ( zero_points.size() || no_errors.size() )
   {
      unsigned int used = 0;

      QStringList qsl_list_no_errors;

      for ( unsigned int i = 0; i < ( unsigned int ) qsl_no_errors.size() && i < 12; i++ )
      {
         qsl_list_no_errors << qsl_no_errors[ i ];
         used++;
      }
      if ( qsl_list_no_errors.size() < qsl_no_errors.size() )
      {
         qsl_list_no_errors << QString( us_tr( "... and %1 more not listed" ) ).arg( qsl_no_errors.size() - qsl_list_no_errors.size() );
      }

      QStringList qsl_list_zero_points;
      for ( unsigned int i = 0; i < ( unsigned int ) qsl_zero_points.size() && i < 24 - used; i++ )
      {
         qsl_list_zero_points << qsl_zero_points[ i ];
      }
      if ( qsl_list_zero_points.size() < qsl_zero_points.size() )
      {
         qsl_list_zero_points << QString( us_tr( "... and %1 more not listed" ) ).arg( qsl_zero_points.size() - qsl_list_zero_points.size() );
      }

      switch ( QMessageBox::warning(this, 
                                    windowTitle() + us_tr( ": Create unified global Gaussians" ),
                                    QString( us_tr( "Please note:\n\n"
                                                 "%1"
                                                 "%2"
                                                 "What would you like to do?\n" ) )
                                    .arg( qsl_list_no_errors.size() ?
                                          QString( us_tr( "These files have no associated errors:\n%1\n\n" ) ).arg( qsl_list_no_errors.join( "\n" ) ) : "" )
                                    .arg( qsl_list_zero_points.size() ?
                                          QString( us_tr( "These files have points with missing SDs:\n%1\n\n" ) ).arg( qsl_list_zero_points.join( "\n" ) ) : "" )
                                    ,
                                    us_tr( "&Turn off SD weighting" ), 
                                    us_tr( "Drop &full curves with missing SDs" ), 
                                    qsl_zero_points.size() ? us_tr( "Drop &points with missing SDs" ) : QString(), 
                                    0, // Stop == button 0
                                    0 // Escape == button 0
                                    ) )
      {
      case 0 : // turn off sd weighting
         {
            cb_sd_weight->setChecked( false );
            pb_ggauss_rmsd->setText   ( us_tr( "Recompute RMSD" ) );
            return create_unified_ggaussian_target( files, false );
         }
         break;
      case 1 : // drop zero sd curves
         {
            running = true;
            disable_updates = true;
            for ( int i = 0; i < lb_files->count(); i++ )
            {
               if ( zero_points.count( lb_files->item( i )->text() ) ||
                    no_errors  .count( lb_files->item( i )->text() ) )
               {
                  lb_files->item( i)->setSelected( false );
               }
            }
            disable_updates = false;
            running = false;
            update_enables();
            disable_all();
            plot_files();
            QStringList files = all_selected_files();
            return create_unified_ggaussian_target( files, false );
         }
         break;
      case 2 : // drop zero sd points
         unified_ggaussian_errors_skip = true;
         break;
      }
   }

   progress->setValue( unified_ggaussian_curves * 1.1 ); progress->setMaximum( unified_ggaussian_curves * 1.2 );
   qApp->processEvents();
   if ( !ggauss_recompute() )
   {
      progress->reset();
      return false;
   }

   // US_Vector::printvector( "unified qvals:", unified_ggaussian_qvals );
   // US_Vector::printvector( "unified q:", unified_ggaussian_q );
   // US_Vector::printvector( "unified t:", unified_ggaussian_t );
   //    US_Vector::printvector( "unified I:", unified_ggaussian_I );
   // US_Vector::printvector( "unified params:", unified_ggaussian_params );
   // US_Vector::printvector( "unified param index:", unified_ggaussian_param_index );

   unified_ggaussian_ok = true;
   progress->reset();
   return true;
}

bool US_Hydrodyn_Mals_Saxs::ggauss_recompute()
{
   unified_ggaussian_q               .clear( );
   unified_ggaussian_jumps           .clear( );
   unified_ggaussian_I               .clear( );
   unified_ggaussian_e               .clear( );
   unified_ggaussian_t               .clear( );
   unified_ggaussian_param_index     .clear( );
   unified_ggaussian_q_start         .clear( );
   unified_ggaussian_q_end           .clear( );
   
   double q_start = le_gauss_fit_start->text().toDouble();
   double q_end   = le_gauss_fit_end  ->text().toDouble();

   unified_ggaussian_jumps  .push_back( 0e0 );

   bool error_msg = false;

   unsigned int common_size   = 1; // center always fixed 
   
   if ( cb_fix_width->isChecked() )
   {
      common_size++;
   }
   if ( dist1_active && cb_fix_dist1->isChecked() )
   {
      common_size++;
   }
   if ( dist2_active && cb_fix_dist2->isChecked() )
   {
      common_size++;
   }

   unsigned int per_file_size = gaussian_type_size - common_size;
   // TSO << QString( "ggauss_recompute: common_size: %1 per_file_size %2\n" ).arg( common_size ).arg( per_file_size );

   for ( unsigned int i = 0; i < ( unsigned int ) unified_ggaussian_files.size(); i++ )
   {
      if ( i )
      {
         unified_ggaussian_jumps.push_back( unified_ggaussian_t.back() );
      }
      if ( !f_qs.count( unified_ggaussian_files[ i ] ) ||
           f_qs[ unified_ggaussian_files[ i ] ].size() < 2 )
      {
         editor_msg( "red", QString( us_tr( "Internal error: %1 has no or empty or insufficient data" ) ).arg( unified_ggaussian_files[ i ] ) );
         return false;
      }

      if ( unified_ggaussian_use_errors && 
           ( !f_errors.count( unified_ggaussian_files[ i ] ) ||
             f_errors[ unified_ggaussian_files[ i ] ].size() != f_qs[ unified_ggaussian_files[ i ] ].size() ) )
      {
         editor_msg( "dark red", QString( us_tr( "WARNING: %1 has no errors so global errors are off for computing RMSD and global fitting" ) ).arg( unified_ggaussian_files[ i ] ) );
         error_msg = true;
         unified_ggaussian_use_errors = false;
         unified_ggaussian_e.clear( );
      }

      unified_ggaussian_q_start.push_back( unified_ggaussian_t.size() );
      for ( unsigned int j = 0; j < ( unsigned int ) f_qs[ unified_ggaussian_files[ i ] ].size(); j++ )
      {
         if ( f_qs[ unified_ggaussian_files[ i ] ][ j ] >= q_start &&
              f_qs[ unified_ggaussian_files[ i ] ][ j ] <= q_end )
         {
            if ( unified_ggaussian_use_errors && unified_ggaussian_errors_skip &&
                 f_errors[ unified_ggaussian_files[ i ] ][ j ] <= 0e0 )
            {
               continue;
            }
                 
            unified_ggaussian_t           .push_back( unified_ggaussian_t.size() );

            // unified_gguassian_param_index is the base of the variable parameters
            // for this curve

            unified_ggaussian_param_index .push_back( unified_ggaussian_gaussians_size * ( common_size + i * per_file_size ) );
            /* old way
              if ( cb_fix_width->isChecked() )
              {
              unified_ggaussian_param_index .push_back( unified_ggaussian_gaussians_size * ( 2 + i ) );
              } else {
              unified_ggaussian_param_index .push_back( unified_ggaussian_gaussians_size * ( 1 + i * 2 ) );
              } 
            */              
            unified_ggaussian_q           .push_back( f_qs[ unified_ggaussian_files[ i ] ][ j ] );
            unified_ggaussian_I           .push_back( f_Is[ unified_ggaussian_files[ i ] ][ j ] );
            if ( unified_ggaussian_use_errors )
            {
               unified_ggaussian_e        .push_back( f_errors[ unified_ggaussian_files[ i ] ][ j ] );
            }
         }
      }
      unified_ggaussian_q_end.push_back( unified_ggaussian_t.size() );
   }

   if ( !is_nonzero_vector( unified_ggaussian_e ) )
   {
      unified_ggaussian_use_errors = false;
      if ( !error_msg )
      {
         editor_msg( "dark red", us_tr( "WARNING: some errors are zero so global errors are off for computing RMSD and global fitting" ) );
      }
   }

   // US_Vector::printvector( "q_start", unified_ggaussian_q_start );
   // US_Vector::printvector( "q_end"  , unified_ggaussian_q_end   );

   // pb_ggauss_rmsd->setEnabled( false );
   
   return true;
}

// #define GG_DEBUG

bool US_Hydrodyn_Mals_Saxs::compute_f_gaussians( QString file, QWidget *mals_saxs_fit_widget )
{

#if defined(GG_DEBUG)
   TSO << "================================================================================\n";
   TSO << QString( "compute_f_gaussians %1\n" ).arg( file );
   TSO << "--------------------------------------------------------------------------------\n";
#endif

   if (
       ((US_Hydrodyn *)us_hydrodyn)->gparams.count( "mals_saxs_cb_gg_cyclic" ) &&
       ((US_Hydrodyn *)us_hydrodyn)->gparams[ "mals_saxs_cb_gg_cyclic" ] == "true"
       ) {
      editor_msg( "darkRed", us_tr( "Experimental: Global Gaussian Gaussian cyclic fit - active\n" ) );
   }
   // us_qdebug( QString( "compute_f_gaussians %1" ).arg( file ) );
   // take current gaussians & compute for this curve
   // find peak of curve
   // US_Hydrodyn_Mals_Saxs_Fit *fit = (US_Hydrodyn_Mals_Saxs_Fit *)mals_saxs_fit_widget;

   vector < double > save_q;
   vector < double > save_I;
   vector < double > save_e;

   // US_Vector::printvector( QString( "cfg %1 q" ).arg( file ), f_qs[ file ] );
   // US_Vector::printvector( QString( "cfg %1 I" ).arg( file ), f_Is[ file ] );
   // US_Vector::printvector( QString( "cfg %1 e" ).arg( file ), f_errors[ file ] );

   bool saved = false;
   if ( cb_sd_weight->isChecked() &&
        f_errors.count( file ) &&
        f_errors[ file ].size() )
   {
      saved = true;

      save_q = f_qs[ file ];
      save_I = f_Is[ file ];
      save_e = f_errors[ file ];

      f_qs[ file ].clear( );
      f_Is[ file ].clear( );
      f_errors[ file ].clear( );

      for ( unsigned int i = 0; i < ( unsigned int ) save_e.size(); i++ )
      {
         if ( save_e[ i ] > 0e0 )
         {
            f_qs[ file ]    .push_back( save_q[ i ] );
            f_Is[ file ]    .push_back( save_I[ i ] );
            f_errors[ file ].push_back( save_e[ i ] );
         }
      }
      if ( !f_qs[ file ].size() )
      {
         saved = false;
         f_qs[ file ]     = save_q;
         f_Is[ file ]     = save_I;
         f_errors[ file ] = save_e;
      }
      // US_Vector::printvector( QString( "cfg as %1 q" ).arg( file ), f_qs[ file ] );
      // US_Vector::printvector( QString( "cfg as %1 I" ).arg( file ), f_Is[ file ] );
      // US_Vector::printvector( QString( "cfg as %1 e" ).arg( file ), f_errors[ file ] );
   }

   if ( ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "mals_saxs_cb_gg_oldstyle" )
        && ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_saxs_cb_gg_oldstyle" ] == "true" ) {
      bool cyclic_on =
         ((US_Hydrodyn *)us_hydrodyn)->gparams.count( "mals_saxs_cb_gg_cyclic" ) &&
         ((US_Hydrodyn *)us_hydrodyn)->gparams[ "mals_saxs_cb_gg_cyclic" ] == "true";
      bool smooth_on = 
         ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "mals_saxs_cb_gg_smooth" )
         && ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_saxs_cb_gg_smooth" ] == "true"
         && ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "mals_saxs_gg_smooth" )
         && ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_saxs_gg_smooth" ].toInt() > 0
         ;

      if ( cyclic_on || smooth_on ) {
         ((US_Hydrodyn *)us_hydrodyn)->gparams[ "mals_saxs_cb_gg_cyclic" ] = "false";
         ((US_Hydrodyn *)us_hydrodyn)->gparams[ "mals_saxs_cb_gg_smooth" ] = "false";

         // process "oldstyle" fit and save

         vector < double > save_gaussians   = gaussians;
         vector < double > save_f_gaussians = f_gaussians[ file ];

         if ( !compute_f_gaussians_trial( file, mals_saxs_fit_widget ) ) {
            editor_msg( "red", QString( us_tr( "Error: computing Gaussians for %1" ) ).arg( file ) );
            return false;
         }

         vector < double > gsum = compute_gaussian_sum( f_qs[file], f_gaussians[ file ] );

         add_oldstyle( file
                       ,f_qs[ file ]
                       ,gsum );
                       
         gaussians           = save_gaussians;
         f_gaussians[ file ] = save_f_gaussians;
      
         if ( cyclic_on ) {
            ((US_Hydrodyn *)us_hydrodyn)->gparams[ "mals_saxs_cb_gg_cyclic" ] = "true";
         }         
         if ( smooth_on ) {
            ((US_Hydrodyn *)us_hydrodyn)->gparams[ "mals_saxs_cb_gg_smooth" ] = "true";
         }
      }
   }

   if ( ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "mals_saxs_cb_gg_smooth" )
        && ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_saxs_cb_gg_smooth" ] == "true"
        && ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "mals_saxs_gg_smooth" )
        && ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_saxs_gg_smooth" ].toInt() > 0 ) {
      if ( !saved ) {
         saved = true;

         save_q = f_qs[ file ];
         save_I = f_Is[ file ];
         save_e = f_errors[ file ];
      }

      vector < double > save2_q = f_qs[ file ];
      vector < double > save2_I = f_Is[ file ];
      vector < double > save2_e = f_errors[ file ];

      int smoothing = ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "mals_saxs_gg_smooth" ].toInt();

      int best_smoothing = 0;
      double best_pvalue;

      // get initial fit

#if defined(GG_DEBUG)
      TSO << QString( "******** get initial fit %1\n" ).arg( file );
#endif
      if ( !compute_f_gaussians_trial( file, mals_saxs_fit_widget ) ) {
         editor_msg( "red", QString( "Error computing gaussians %1 [a]" ).arg( file ) );
         f_qs[ file ]     = save_q;
         f_Is[ file ]     = save_I;
         f_errors[ file ] = save_e;
         return false;
      }         

      vector < double > bestg         = f_gaussians[ file ];
      vector < double > bestsmoothedI; // = f_Is[ file ];

      // vector < vector < double > > bestgs_debug;

      // bestgs_debug.push_back( bestg );

      // get initial fit p value
      {
         double P;
         QString errormsg;
      
         if ( pvalue( file, P, errormsg ) ) {
            editor_msg( "darkblue", QString( "%1 pvalue %2\n" ).arg( file ).arg( P ) );
         } else {
            editor_msg( "red", QString( "%1 pvalue compute failed %2\n" ).arg( file ).arg( errormsg ) );
            f_qs[ file ]     = save_q;
            f_Is[ file ]     = save_I;
            f_errors[ file ] = save_e;
            return false;
         }
         best_pvalue = P;
      }

      // qDebug() << QString( "gg smoothing %1 s %2 p %3" ).arg( file ).arg( 0 ).arg( best_pvalue );

      // check smoothing for improvement

      for ( int s = 1; s <= smoothing; ++s ) {
         f_qs[ file ]     = save2_q;
         f_Is[ file ]     = save2_I;
         f_errors[ file ] = save2_e;

         vector < double > smoothed_I;

         if ( usu->smooth( f_Is[ file ], smoothed_I, s ) ) {
            f_Is[ file ] = smoothed_I;
         } else {
            editor_msg( "red", QString( "Error smoothing %1 : %2" ).arg( file ).arg( usu->errormsg ) );
            return false;
         }

#if defined(GG_DEBUG)
         TSO << QString( "******** get smoothing fit %1 %2\n" ).arg( s ).arg( file );
#endif

         if ( !compute_f_gaussians_trial( file, mals_saxs_fit_widget ) ) {
            f_qs[ file ]     = save_q;
            f_Is[ file ]     = save_I;
            f_errors[ file ] = save_e;
            editor_msg( "red", QString( "Error computing gaussians %1 [b]" ).arg( file ) );
            return false;
         }         

         // bestgs_debug.push_back( f_gaussians[ file ] );

         // get p value
         {
            double P;
            QString errormsg;
      
            f_qs[ file ]     = save2_q;
            f_Is[ file ]     = save2_I;
            f_errors[ file ] = save2_e;

            if ( pvalue( file, P, errormsg ) ) {
               editor_msg( "darkblue", QString( "%1 pvalue %2\n" ).arg( file ).arg( P ) );
            } else {
               editor_msg( "red", QString( "%1 pvalue compute failed %2\n" ).arg( file ).arg( errormsg ) );
               return false;
            }
            if ( best_pvalue < P ) {
               best_pvalue    = P;
               best_smoothing = s;
               bestg          = f_gaussians[ file ];
               bestsmoothedI  = smoothed_I;

            }
            // qDebug() << QString( "gg smoothing %1 s %2 p %3" ).arg( file ).arg( s ).arg( P );
         }
      }

      f_gaussians[ file ] = bestg;

      if ( best_smoothing > 0 ) {
         // qDebug() << QString( "gg smoothing %1 best s %2 p %3" ).arg( file ).arg( best_smoothing ).arg( best_pvalue );
         add_plot(
                  QString( "%1-sm%2" ).arg( file ).arg( best_smoothing )
                  ,f_qs[ file ]
                  ,bestsmoothedI
                  ,f_errors[ file ]
                  ,true
                  ,false
                  );
         add_smoothed(
                      file
                      ,f_qs[ file ]
                      ,bestsmoothedI
                      ,best_smoothing
                      );
                        
         // add_plot_gaussian( file, QString( "sm%1-g" ).arg( best_smoothing ) );
         // to debug them all
         // vector < double > save_g = f_gaussians[ file ];
         // for ( int i = 0; i < (int) bestgs_debug.size(); ++i ) {
         //    f_gaussians[file] = bestgs_debug[i];
         //    add_plot_gaussian( file, QString( "g_s%1" ).arg( i ) );
         // }
         // f_gaussians[ file ] = save_g;
      }

      f_qs[ file ]     = save_q;
      f_Is[ file ]     = save_I;
      f_errors[ file ] = save_e;

      return true;
   }

#if defined(GG_DEBUG)
   TSO << QString( "******* get final fit %1\n" ).arg( file );
#endif

   if ( !compute_f_gaussians_trial( file, mals_saxs_fit_widget ) ) {
      if ( saved ) {
         f_qs[ file ]     = save_q;
         f_Is[ file ]     = save_I;
         f_errors[ file ] = save_e;
      }         
      return false;
   }         

   return true;
}

bool US_Hydrodyn_Mals_Saxs::initial_ggaussian_fit( QStringList & files, bool only_init_unset ) {
   wheel_file = files[ 0 ];

   // us_qdebug( "creating fit window" );

   US_Hydrodyn_Mals_Saxs_Fit *mals_saxs_fit_window = 
      new US_Hydrodyn_Mals_Saxs_Fit(
                                    this,
                                    false,
                                    this
                                    );
   US_Hydrodyn::fixWinButtons( mals_saxs_fit_window );

   mals_saxs_fit_window->update_mals_saxs = false;

   clear_smoothed();
   clear_oldstyle();

   for ( unsigned int i = 0; i < ( unsigned int ) files.size(); i++ ) {
      progress->setValue( i ); progress->setMaximum( files.size() * 1.2 );
      qApp->processEvents();
#if defined(GG_DEBUG)
      us_qdebug( QString( "------ processing initial gaussian fit %1 ------" ).arg( files[ i ] ) );
#endif
      
      if ( !only_init_unset || !f_gaussians.count( files[ i ] ) ) {
         if ( !compute_f_gaussians( files[ i ], (QWidget *) mals_saxs_fit_window ) ) {
            return false;
         }
      }
   }

   delete mals_saxs_fit_window;

   list_smoothed();
   
   return true;
}

void US_Hydrodyn_Mals_Saxs::add_plot_gaussian( const QString &file, const QString &tag ) {
   add_plot(
            QString( "%1-%2" ).arg( file ).arg( tag )
            ,f_qs[ file ]
            ,compute_gaussian_sum( f_qs[ file ], f_gaussians[ file ] )
            ,true
            ,false
            );
}

bool US_Hydrodyn_Mals_Saxs::compute_f_gaussians_trial( QString file, QWidget *mals_saxs_fit_widget ) {
   // printvector( "cfg: org_gauss", org_gaussians );
#if defined(GG_DEBUG)
   TSO << "--------------------------------------------------------------------------------\n";
   TSO << QString( "compute_f_gaussians_trial %1\n" ).arg( file );
   TSO << "--------------------------------------------------------------------------------\n";
#endif
   double peak;
   if ( !get_peak( file, peak ) ) {
      return false;
   }

   double gmax = compute_gaussian_peak( file, org_gaussians );
   
   double scale = peak / gmax;   

   gauss_max_height = peak * 1.2;
   if ( gaussian_type != GAUSS )
   {
      gauss_max_height *= 20e0;
   }
   // TSO << QString( "compute_f_gaussians():gauss_max_height for %1 is %2\n" ).arg( file ).arg( gauss_max_height );

   // printvector( "cfg: org_gauss 2", org_gaussians );
   gaussians = org_gaussians;
   
   for ( unsigned int i = 0; i < ( unsigned int ) gaussians.size(); i += gaussian_type_size ) {
      gaussians[ 0 + i ] *= scale;
   }

   // US_Vector::printvector( "cfg: gaussians", gaussians );

   // double gmax2 = compute_gaussian_peak( file, gaussians );

   // TSO << QString( "cfg: %1 org_gaussian peak %2, curve peak %3, scaling %4 new gaussian peak %5\n" )
   //    .arg( file )
   //    .arg( gmax )
   //    .arg( peak )
   //    .arg( scale )
   //    .arg( gmax2 )
   //    ;

   // now setup and fit
   wheel_file = file;

   // vector < double > tmp_gs = gaussians;

   US_Hydrodyn_Mals_Saxs_Fit *fit = (US_Hydrodyn_Mals_Saxs_Fit *)mals_saxs_fit_widget;

   fit->redo_settings();
   fit->gaussians_undo.clear( );
   fit->gaussians_undo.push_back( gaussians );

   fit->cb_fix_center    ->setChecked( true );
   fit->cb_fix_width     ->setChecked( true );
   fit->cb_fix_amplitude ->setChecked( false );
   fit->cb_fix_dist1     ->setChecked( true );
   fit->cb_fix_dist2     ->setChecked( true );

   // QTextStream(stdout) << QString( "compute_f_gaussians_trial number of gaussians %1\n" ).arg( fit->cb_fix_curves.size() );
#if defined(GG_DEBUG)
   QTextStream(stdout) << QString( "epsilon is %1\n" ).arg( fit->le_epsilon->text() );
#endif

   if (
       ((US_Hydrodyn *)us_hydrodyn)->gparams.count( "mals_saxs_cb_gg_cyclic" ) &&
       ((US_Hydrodyn *)us_hydrodyn)->gparams[ "mals_saxs_cb_gg_cyclic" ] == "true"
       ) {
      // QTextStream(stdout) << QString( "Experimental: Global Gaussian Gaussian cyclic fit - active - %1\n" ).arg( file );
      for ( int i = 0; i < (int) fit->cb_fix_curves.size(); ++i ) {
         for ( int j = 0; j < (int) fit->cb_fix_curves.size(); ++j ) {
            fit->cb_fix_curves[j]->setChecked( i != j );
         }
#if defined(GG_DEBUG)
         TSO << QString( "******** fit %1 fix %2\n" ).arg( file ).arg( i + 1 );
#endif
         fit->lm();
      }
      for ( int i = 0; i < (int) fit->cb_fix_curves.size(); ++i ) {
         fit->cb_fix_curves[i]->setChecked( false );
      }
   }

   fit->le_epsilon->setText( QString( "%1" ).arg( peak / 1e6 < 0.001 ? peak / 1e6 : 0.001 ) );
      
   // fit initial amplitudes
   // us_qdebug( "call first lm fit in compute_f_gaussians" );
#if defined(GG_DEBUG)
   TSO << QString( "fit %1 fit all centers mobile\n" ).arg( file );
#endif
   fit->lm();
   // us_qdebug( "return from first lm fit in compute_f_gaussians" );
   // US_Vector::printvector2( "compute_f_gaussians: gaussians before, after", tmp_gs, gaussians );
   // printvector( "cfg: after fit gaussians", gaussians );

   // now run "open"

   if ( !cb_fix_width->isChecked() ||
        ( dist1_active && !cb_fix_dist1->isChecked() ) ||
        ( dist2_active && !cb_fix_dist2->isChecked() ) )
   {
#if defined(GG_DEBUG)
      TSO << QString( "fit %1 fit 'open'?\n" ).arg( file );
#endif
      // us_qdebug( "call second lm fit in compute_f_gaussians" );
      fit->cb_fix_width     ->setChecked( cb_fix_width->isChecked() );
      fit->cb_fix_dist1     ->setChecked( cb_fix_dist1->isChecked() );
      fit->cb_fix_dist2     ->setChecked( cb_fix_dist2->isChecked() );
      fit->lm();
      // us_qdebug( "return from second lm fit in compute_f_gaussians" );
      // printvector( "cfg: after fit2 gaussians", gaussians );
   }
   
   f_gaussians[ file ] = gaussians;
   gaussians = org_gaussians;

   return true;
}

void US_Hydrodyn_Mals_Saxs::clear_smoothed() {
   f_qs_smoothed.clear();
   f_Is_smoothed.clear();
   f_best_smoothed_smoothing.clear();
}
   
void US_Hydrodyn_Mals_Saxs::list_smoothed() {
   TSO << "list_smoothed():\n";
   for ( auto it = f_qs_smoothed.begin();
         it != f_qs_smoothed.end();
         ++it ) {
      TSO << QString( "%1 smoothing %2 data points %3\n" )
         .arg( it->first )
         .arg( f_best_smoothed_smoothing[ it->first ] )
         .arg( it->second.size() )
         ;
   }
}
            
void US_Hydrodyn_Mals_Saxs::add_smoothed(
                                         const QString            & name
                                         ,const vector < double > & q
                                         ,const vector < double > & I
                                         ,int                       best_smoothing
                                         ) {
   f_qs_smoothed[ name ]             = q;
   f_Is_smoothed[ name ]             = I;
   f_best_smoothed_smoothing[ name ] = best_smoothing;
}

void US_Hydrodyn_Mals_Saxs::clear_oldstyle() {
   f_qs_oldstyle.clear();
   f_Is_oldstyle.clear();
}
   
void US_Hydrodyn_Mals_Saxs::list_oldstyle() {
   TSO << "list_oldstyle():\n";
   for ( auto it = f_qs_oldstyle.begin();
         it != f_qs_oldstyle.end();
         ++it ) {
      TSO << QString( "%1 oldstyle data points %2\n" )
         .arg( it->first )
         .arg( it->second.size() )
         ;
   }
}
            
void US_Hydrodyn_Mals_Saxs::add_oldstyle(
                                         const QString            & name
                                         ,const vector < double > & q
                                         ,const vector < double > & I
                                         ) {
   f_qs_oldstyle[ name ]             = q;
   f_Is_oldstyle[ name ]             = I;
}


void US_Hydrodyn_Mals_Saxs::create_ihash_t() {
   disable_all();

   QStringList files = all_selected_files();
   create_ihash_t( files );

   // dndc_info( "dndc after create_ihash_t" );
   // extc_info( "extc after create_ihash_t" );

   update_enables();
}

bool US_Hydrodyn_Mals_Saxs::create_ihash_t( QStringList files ) {
   double lambda_cm = mals_saxs_param_lambda * 1e-7; // nm to cm

   double K =
      (
       4
       * M_PI * M_PI
       * mals_saxs_param_n * mals_saxs_param_n
       * mals_saxs_param_g_dndc * mals_saxs_param_g_dndc
       ) / (
            AVOGADRO
            * lambda_cm * lambda_cm * lambda_cm * lambda_cm
            );

   double Kinv = 1.0 / K;

   TSO << "create I#(t) : K is " << K << "\n";

   set < QString > hash_names;
   
   for ( const auto & name : files ) {
      QString hash_name = name;
      hash_name.replace( "_Rt_", "_Ihasht_" );
      if ( !f_Is.count( name )
           || !f_errors.count( name )
           || !f_qs.count( name ) ) {
         editor_msg( "red", QString( "Internal error: missing data for %1\n" ).arg( name ) );
      } else {
         vector < double > hash_I = f_Is[ name ];
         vector < double > hash_e = f_errors[ name ];

         bool use_errors = hash_e.size() == hash_I.size();

         if ( use_errors ) {
            for ( int i = 0; i < (int) hash_I.size(); ++i ) {
               hash_I[i] *= Kinv;
               hash_e[i] *= Kinv;
            }
            add_plot( hash_name, f_qs[ name ], hash_I, hash_e, true, false );
         } else {
            for ( int i = 0; i < (int) hash_I.size(); ++i ) {
               hash_I[i] *= Kinv;
            }
            add_plot( hash_name, f_qs[ name ], hash_I, true, false );
         }
         f_g_dndc[ last_created_file ] = mals_saxs_param_g_dndc;
         hash_names.insert( last_created_file );
      }
   }

   set_selected( hash_names );
   plot_files();
   update_enables();
   return true;
}

void US_Hydrodyn_Mals_Saxs::deselect_conc_file() {
   if ( !lbl_conc_file->text().isEmpty() ) {
      for ( int i = 0; i < lb_files->count(); i++ ) {
         if ( lb_files->item( i )->isSelected() ) {
            if ( lb_files->item( i )->text() == lbl_conc_file->text() ) {
               lb_files->item( i)->setSelected( false );
            }
         }
      }
   }
}

void US_Hydrodyn_Mals_Saxs::create_istar_q() {
   disable_all();

   deselect_conc_file();
   
   QStringList files = all_selected_files();

   gaussians.size() ? (void) create_istar_q( files ) : (void) create_istar_q_ng( files );
   
   update_enables();

   // conc_info( "conc after create_istar_q" );
   // dndc_info( "dndc after create_istar_q" );
   // extc_info( "extc after create_istar_q" );
}

bool US_Hydrodyn_Mals_Saxs::create_istar_q( QStringList files, double t_min, double t_max ) {
   QMessageBox::critical( this,
                             windowTitle() + us_tr( ": Make I*(q)" ),
                             us_tr( "Make I*(q) with Gaussians not currently implemented" )
                             );
   return false;
}

bool US_Hydrodyn_Mals_Saxs::create_istar_q_ng( QStringList files, double t_min, double t_max ) {

   // if ( lbl_conc_file->text().isEmpty() ) {
   QMessageBox::information( this,
                             windowTitle() + us_tr( ": Make I*(q)" ),
                             us_tr( "I*(q) will be made without Gaussians" )
                             );

   // } else {
   //    QMessageBox::critical( this,
   //                           windowTitle() + us_tr( ": Make I*(q)" ),
   //                          us_tr( "I*(q) with concentration curve not currently implemented" )
   //                          );
   //    return false;
   // }
      
   bool use_conc = !lbl_conc_file->text().isEmpty();
   double detector_conv = 0e0;
   if ( detector_uv ) {
      detector_conv = detector_uv_conv * UHSH_UV_CONC_FACTOR;
   }
   if ( detector_ri ) {
      detector_conv = detector_ri_conv;
   }

   if ( use_conc &&
        (
         !f_qs.count( lbl_conc_file->text() )
         || !f_Is.count( lbl_conc_file->text() )
         ) ) {
      editor_msg( "red", QString( us_tr( "Internal error: Concentration file %1 set, but associated data is incomplete.  Concentration disabled." ) ).arg( lbl_conc_file->text() ) );
      lbl_conc_file->setText( "" );
      use_conc = false;
   }

   double use_g_dndc = 0;
   if ( use_conc ) {
      for ( unsigned int i = 0; i < ( unsigned int ) files.size(); i++ ) {
         if ( !f_g_dndc.count( files[i] ) ) {
            QMessageBox::critical( this,
                                   windowTitle() + us_tr( ": Make I*(q)" ),
                                   QString( 
                                           us_tr( "I*(q) with concentration curve, global dn/dc not defined for curve %1" )
                                            )
                                   .arg( files[i] )
                                   );
            return false;
         }

         TSO << QString( "curve %1 g_dndc %2\n" ).arg( files[i] ).arg( f_g_dndc[ files[i] ] );

         if ( !i ) {
            use_g_dndc = f_g_dndc[ files[i] ];
         } else {
            if ( use_g_dndc != f_g_dndc[ files[i] ] ) {
               QMessageBox::critical( this,
                                      windowTitle() + us_tr( ": Make I*(q)" ),
                                      QString( 
                                              us_tr( "I*(q) with concentration curve: global dn/dc inconsistent\n"
                                                     "e.g. curve %1 has value %2 and curve %3 has value %4" )
                                               )
                                      .arg( files[0] )
                                      .arg( use_g_dndc )
                                      .arg( files[i] )
                                      .arg( f_g_dndc[ files[i] ] )
                                      );
               return false;
            }
         }               
      }
   }
      
   bool make_ihashq = false;

   if ( !use_conc && !mals_saxs_param_g_conc ) {
      if ( 1 == QMessageBox::question(this, 
                                      this->windowTitle() + us_tr( ": Make I*(q)" )
                                      ,us_tr(
                                            "Global concentration in Options->MALS_SAXS parameters is zero\n"
                                            "Do you wish to make I#(q)?"
                                            )
                                      ,us_tr( "&Yes, make I#(q)" )
                                      ,us_tr( "&No" )
                                      ,QString()
                                      ,0
                                      ,1
                                      ) ) {
         QMessageBox::critical( this,
                                windowTitle() + us_tr( ": Make I*(q)" ),
                                us_tr(
                                      "To make I*(q) without a concentration curve,\n"
                                      "the global concentration must be defined in Options->MALS_SAXS parameters"
                                      )
                                );
         return false;
      }
      make_ihashq = true;
   }
   if ( !use_conc && !make_ihashq ) {
      QMessageBox::information( this,
                                windowTitle() + us_tr( ": Make I*(q)" ),
                                QString( us_tr( "I*(q) will be created using Global concentration of %1 [mg/mL]\n(set in Options->MALS_SAXS parameters)" ) )
                                .arg( mals_saxs_param_g_conc )
                                );
   }      

   if ( use_conc && !detector_conv ) {
      QMessageBox::critical( this,
                             windowTitle() + us_tr( ": Make I*(q)" ),
                             us_tr(
                                   "The detector properties must be defined Options->Concentraton Detector Properties"
                                   )
                             );
      return false;
   }      

   bool mode_testiq = ( current_mode == MODE_TESTIQ );

   editor_msg( "dark blue", us_tr(
                                  make_ihashq
                                  ? "Starting: Make I#(q) without Gaussians"
                                  : "Starting: Make I*(q) without Gaussians"
                                  ) );

   QString head = qstring_common_head( files, true );

   // TSO << "create_istar_q_ng()  common head: " << head << "\n";

   head = head.replace( QRegularExpression( "_D\\d+_Ihasht_q\\d*_$" ), "" );
   head = head.replace( QRegularExpression( "_q\\d*_$" ), "" );
   head = head.replace( QRegularExpression( "_D$" ), "" );
   head = head.replace( QRegularExpression( "[\\[\\]{}]" ), "" );
   head +=
      make_ihashq
      ? "_MALS_SAXS_Ihashq_t"
      : "_MALS_SAXS_Istarq_t"
      ;
   
   // TSO << "create_istar_q_ng()  head: " << head << "\n";

   QRegExp rx_q     ( "_q(\\d+_\\d+)" );
   QRegExp rx_bl    ( "-bl(.\\d*_\\d+(|e.\\d+))-(.\\d*_\\d+(|e.\\d+))s" );
   QRegExp rx_bi    ( "-bi(.\\d*_\\d+(|e.\\d+))-(.\\d*_\\d+(|e.\\d+))s" );

   vector < QString > q_string;
   vector < double  > q;

   bool         any_bl = false;
   bool         any_bi = false;

   // get q 

   // map: [ timestamp ][ q_value ] = intensity

   map < double, map < double , double > > I_values;
   map < double, map < double , double > > e_values;

   map < double, bool > used_t;
   list < double >      tl;

   map < double, bool > used_q;
   list < double >      ql;

   bool                 use_errors = true;

   disable_all();

   map < QString, bool >    no_errors;
   map < QString, QString > zero_points;
   QStringList              qsl_no_errors;
   QStringList              qsl_zero_points;

   for ( unsigned int i = 0; i < ( unsigned int ) files.size(); i++ ) {
      progress->setValue( i ); progress->setMaximum( files.size() * 2 );
      qApp->processEvents();
      if ( rx_q.indexIn( files[ i ] ) == -1 )
      {
         editor_msg( "red", QString( us_tr( "Error: Can not find q value in file name for %1" ) ).arg( files[ i ] ) );
         progress->reset();
         update_enables();
         return false;
      }
      ql.push_back( rx_q.cap( 1 ).replace( "_", "." ).toDouble() );
      if ( used_q.count( ql.back() ) )
      {
         editor_msg( "red", QString( us_tr( "Error: Duplicate q value in file name for %1" ) ).arg( files[ i ] ) );
         progress->reset();
         update_enables();

         return false;
      }
      used_q[ ql.back() ] = true;
         
      if ( rx_bl.indexIn( files[ i ] ) != -1 )
      {
         any_bl = true;
      }
      if ( rx_bi.indexIn( files[ i ] ) != -1 )
      {
         any_bi = true;
      }

      if ( !f_qs.count( files[ i ] ) )
      {
         editor_msg( "red", QString( us_tr( "Internal error: request to use %1, but not found in data" ) ).arg( files[ i ] ) );
      } else {
         for ( unsigned int j = 0; j < ( unsigned int ) f_qs[ files[ i ] ].size(); j++ )
         {
            if ( !mode_testiq || ( f_qs[ files[ i ] ][ j ] >= t_min && f_qs[ files[ i ] ][ j ] <= t_max ) )
            {
               I_values[ f_qs[ files[ i ] ][ j ] ][ ql.back() ] = f_Is[ files[ i ] ][ j ];
               if ( use_errors && f_errors[ files[ i ] ].size() == f_qs[ files[ i ] ].size() )
               {
                  e_values[ f_qs[ files[ i ] ][ j ] ][ ql.back() ] = f_errors[ files[ i ] ][ j ];
               } else {
                  if ( use_errors )
                  {
                     use_errors = false;
                     editor_msg( "dark red", QString( us_tr( "Notice: missing errors, first noticed in %1, so no errors at all" ) )
                                 .arg( files[ i ] ) );
                  }
               }
               if ( !used_t.count( f_qs[ files[ i ] ][ j ] ) )
               {
                  tl.push_back( f_qs[ files[ i ] ][ j ] );
                  used_t[ f_qs[ files[ i ] ][ j ] ] = true;
               }
            }
         }

         if ( !f_errors.count( files[ i ] ) ||
              f_errors[ files[ i ] ].size() != f_Is[ files[ i ] ].size() )
         {
            no_errors[ files[ i ] ] = true;
            qsl_no_errors           << files[ i ];
         } else {
            if ( !is_nonzero_vector( f_errors[ files[ i ] ] ) )
            {
               unsigned int zero_pts = 0;
               for ( unsigned int j = 0; j < ( unsigned int ) f_errors[ files[ i ] ].size(); j++ )
               {
                  if ( !mode_testiq || ( f_qs[ files[ i ] ][ j ] >= t_min && f_qs[ files[ i ] ][ j ] <= t_max ) )
                  {
                     if ( us_isnan( f_errors[ files[ i ] ][ j ] ) || f_errors[ files[ i ] ][ j ] == 0e0 )
                     {
                        zero_pts++;
                     }
                  }
               }
               zero_points[ files[ i ] ] = QString( "%1: %2 of %3 points" ).arg( files[ i ] ).arg( zero_pts ).arg( f_errors[ files[ i ] ].size() );
               qsl_zero_points           << zero_points[ files[ i ] ];
            }
         }

      }
   }

   tl.sort();

   vector < double > tv;
   for ( list < double >::iterator it = tl.begin();
         it != tl.end();
         it++ )
   {
      tv.push_back( *it );
   }

   ql.sort();

   vector < double  > qv;
   vector < QString > qv_string;
   for ( list < double >::iterator it = ql.begin();
         it != ql.end();
         it++ )
   {
      qv.push_back( *it );
      qv_string.push_back( QString( "%1" ).arg( *it ) );
   }


   QString qs_no_errors;
   QString qs_zero_points;

   if ( zero_points.size() || no_errors.size() )
   {
      unsigned int used = 0;

      QStringList qsl_list_no_errors;

      for ( unsigned int i = 0; i < ( unsigned int ) qsl_no_errors.size() && i < 12; i++ )
      {
         qsl_list_no_errors << qsl_no_errors[ i ];
         used++;
      }
      if ( qsl_list_no_errors.size() < qsl_no_errors.size() )
      {
         qsl_list_no_errors << QString( us_tr( "... and %1 more not listed" ) ).arg( qsl_no_errors.size() - qsl_list_no_errors.size() );
      }
      qs_no_errors = qsl_list_no_errors.join( "\n" );
      
      QStringList qsl_list_zero_points;
      for ( unsigned int i = 0; i < ( unsigned int ) qsl_zero_points.size() && i < 24 - used; i++ )
      {
         qsl_list_zero_points << qsl_zero_points[ i ];
      }
      if ( qsl_list_zero_points.size() < qsl_zero_points.size() )
      {
         qsl_list_zero_points << QString( us_tr( "... and %1 more not listed" ) ).arg( qsl_zero_points.size() - qsl_list_zero_points.size() );
      }
      qs_zero_points = qsl_list_zero_points.join( "\n" );
   }

   // bool   normalize_by_conc = false;
   bool   conc_ok           = false;

   double conv = 0e0;
   double dndc = 0e0;
   double I0se = 0e0;
   double conc_repeak = 1e0;
   
   vector < double > conc_spline_x;
   vector < double > conc_spline_y;
   vector < double > conc_spline_y2;

   if ( !mode_testiq ) {
      map < QString, QString > parameters;

      bool any_detector = false;
      if ( detector_uv ) {
         parameters[ "uv" ] = "true";
         any_detector = true;
      } else {
         if ( detector_ri ) {
            parameters[ "ri" ] = "true";
            any_detector = true;
         }
      }

      if ( use_conc ) {
         if ( !any_detector ) {
            if ( parameters.count( "error" ) ) {
               parameters[ "error" ] += "\nYou must also select a detector type";
            } else {
               parameters[ "error" ] = "\nYou must select a detector type";
            }
         }
         parameters[ "ngmode" ] = "true";
         parameters[ "gaussians" ] = "1";
         parameters[ "default_extc" ] = QString( "%1" ).arg( mals_saxs_param_g_extinction_coef );
         parameters[ "default_dndc" ] = QString( "%1" ).arg( mals_saxs_param_g_dndc );
         US_Hydrodyn_Mals_Saxs_Ciq *mals_saxs_ciq = 
            new US_Hydrodyn_Mals_Saxs_Ciq(
                                     this,
                                     & parameters,
                                     this );
         US_Hydrodyn::fixWinButtons( mals_saxs_ciq );
         mals_saxs_ciq->exec();
         delete mals_saxs_ciq;

         if ( !parameters.count( "go" ) ) {
            // TSO << "mals_saxs_ciq no go\n";
            progress->reset();
            update_enables();
            return false;
         }

         conv = parameters.count( "conv 0" ) ? parameters[ "conv 0" ].toDouble() : 0e0;
         dndc = parameters.count( "dndc 0" ) ? parameters[ "dndc 0" ].toDouble() : 0e0;
         if ( conv == 0e0 ||
              dndc == 0e0 ) {
            // TSO << "mals_saxs_ciq conv or dndc zero\n";
            progress->reset();
            update_enables();
            return false;
         }
            
         conc_ok = true;
         if ( parameters.count( "normalize" ) && parameters[ "normalize" ] == "true" ) {
            // normalize_by_conc = true;
         }

         if ( parameters.count( "I0se" ) ) {
            I0se = parameters[ "I0se" ].toDouble();
         }

         double detector_conv = 0e0;
         if ( detector_uv ) {
            detector_conv = detector_uv_conv * UHSH_UV_CONC_FACTOR;
         }
         if ( detector_ri ) {
            detector_conv = detector_ri_conv;
         }
         
         {
            QRegExp rx_repeak( "-rp(.\\d*(_|\\.)\\d+(|e.\\d+))" );
            if ( rx_repeak.indexIn( lbl_conc_file->text() ) != -1 )
            {
               conc_repeak = rx_repeak.cap( 1 ).replace( "_", "." ).toDouble();
               if ( conc_repeak == 0e0 )
               {
                  conc_repeak = 1e0;
                  editor_msg( "red", us_tr( "Error: concentration repeak scaling value extracted is 0, turning off back scaling" ) );
               } else {
                  editor_msg( "dark blue", QString( us_tr( "Notice: concentration scaling repeak value %1" ) ).arg( conc_repeak ) );
               }
            }
         }

         conc_spline_x = f_qs[ lbl_conc_file->text() ];
         conc_spline_y = f_Is[ lbl_conc_file->text() ];
         for ( int i = 0; i < (int) conc_spline_y.size(); ++i ) {
            conc_spline_y[ i ] *= detector_conv / ( conc_repeak * conv );
         }
         usu->natural_spline( conc_spline_x, conc_spline_y, conc_spline_y2 );
         // TSO << "conc natural spine made\n";
      }
   }

   running = true;

   // now for each I(t) distribute the I for each frame 

   // build up resulting curves

   // for each time, tv[ t ] 

   map < QString, bool > current_files;
   if ( !mode_testiq )
   {
      for ( int i = 0; i < (int)lb_files->count(); i++ )
      {
         current_files[ lb_files->item( i )->text() ] = true;
      }
   }

   set < QString > star_names;

   for ( unsigned int t = 0; t < tv.size(); t++ ) {
      progress->setValue( files.size() + t ); progress->setMaximum( files.size() + tv.size() );
      qApp->processEvents();

      // build up an I(q)

      QString name;

      {
         double whole, frac;
         frac = std::modf( tv[ t ], &whole );
         name = head + QString( "%1%2%3" )
            .arg( (any_bl || any_bi) ? "_bs" : "" )
            .arg( pad_zeros( whole, (int) (tv.back() + .5) ) )
            .arg( QString( "%1" ).arg( frac ).replace( QRegularExpression( "^0" ), "" ) )
            .replace( ".", "_" )
            ;
      }

      {
         int ext = 0;
         QString use_name = name;
         while ( current_files.count( use_name ) )
         {
            use_name = name + QString( "-%1" ).arg( ++ext );
         }
         name = use_name;
      }
         
      // TSO << "curve " << t << " tv[t] " << tv[t] << " name " << name << "\n";
      
      // TSO << QString( "name %1\n" ).arg( name );

      // now go through all the files to pick out the I values and errors and distribute amoungst the various gaussian peaks
      // we could also reassemble the original sum of gaussians curves as a comparative

      vector < double > I;
      vector < double > e;
      // vector < double > G;

      // vector < double > I_recon;
      // vector < double > G_recon;

      vector < double > this_used_pcts;

      // concentration factor with no conc curve
      double conc_factor =
         make_ihashq
         ? 1
         : 1 / (mals_saxs_param_g_conc * 1e-3)
         ;
      
      double store_conc = mals_saxs_param_g_conc;

      if ( use_conc && conc_ok ) {
         if ( !usu->apply_natural_spline( conc_spline_x, conc_spline_y, conc_spline_y2, tv[ t ], conc_factor ) ) {
            editor_msg( "red", QString( us_tr( "Error getting concentration from spline for frame %1, concentration set to zero." ) ).arg( tv[ t ] ) );
            conc_factor = 0e0;
         } else {
            // conc_factor will multiply I# intensities
            // mals_saxs_param_g_dndc ^2 / dndc ^2 used to correct for K when computing I#
            store_conc = conc_factor * 1e-3;
            conc_factor = ( use_g_dndc * use_g_dndc ) / ( dndc * dndc * conc_factor );
         }
      }

      TSO << QString( "conc factor %1  store_conc %2\n" ).arg( conc_factor ).arg( store_conc );

      for ( unsigned int i = 0; i < ( unsigned int ) files.size(); i++ )
      {
         if ( !I_values.count( tv[ t ] ) ) {
            editor_msg( "dark red", QString( us_tr( "Notice: I values missing frame/time = %1, curve creation skipped curve" ) ).arg( tv[ t ] ) );
            continue;
         }

         if ( !I_values[ tv[ t ] ].count( qv[ i ] ) )
         {
            editor_msg( "red", QString( us_tr( "Notice: I values missing q = %1" ) ).arg( qv[ i ] ) );
            continue;
         }

         double tmp_I       = I_values[ tv[ t ] ][ qv[ i ] ] * conc_factor;
         double tmp_e       = 0e0;

         if ( use_errors )
         {
            if ( !e_values.count( tv[ t ] ) )
            {
               editor_msg( "red", QString( us_tr( "Internal error: error values missing t %1" ) ).arg( tv[ t ] ) );
               running = false;
               update_enables();
               progress->reset();
               return false;
            }

            if ( !e_values[ tv[ t ] ].count( qv[ i ] ) )
            {
               editor_msg( "red", QString( us_tr( "Internal error: error values missing q %1" ) ).arg( qv[ i ] ) );
               running = false;
               update_enables();
               progress->reset();
               return false;
            }

            tmp_e = e_values[ tv[ t ] ][ qv[ i ] ] * conc_factor;
         }
            
         I      .push_back( tmp_I );
         e      .push_back( tmp_e );
      } // for each file
         
      if ( mode_testiq )
      {
         testiq_created_names.push_back( name );
         testiq_created_t[ name ] = tv[ t ];
         testiq_created_q[ name ] = qv;
         testiq_created_I[ name ] = I;
         testiq_created_e[ name ] = e;
      } else {
         lb_created_files->addItem( name );
         lb_created_files->scrollToItem( lb_created_files->item( lb_created_files->count() - 1 ) );
         lb_files->addItem( name );
         lb_files->scrollToItem( lb_files->item( lb_files->count() - 1 ) );
         created_files_not_saved[ name ] = true;
   
         vector < QString > use_qv_string = qv_string;
         vector < double  > use_qv        = qv;
         vector < double  > use_I         = I;
         vector < double  > use_e         = e;

         f_pos       [ name ] = f_qs.size();
         f_qs_string [ name ] = qv_string;
         f_qs        [ name ] = qv;
         f_Is        [ name ] = I;
         f_errors    [ name ] = e;
         f_is_time   [ name ] = false;
         f_conc      [ name ] = store_conc * 1e6;
         // f_psv       [ name ] = conc_ok ? dndc : 0e0;
         f_I0se      [ name ] = conc_ok ? I0se : 0e0;
         f_time      [ name ] = tv[ t ];
         // if ( conc_ok && conv ) {
         if ( conv ) {
            f_extc      [ name ] = conv;
         }
         // if ( conc_ok && dndc ) {
         if ( dndc ) {
            f_dndc      [ name ] = dndc;
         }
         f_conc_units[ name ] = "mg/mL";

         if ( !conc_ok ) { // no uv curve
#warning what about loaded curve data?
#warning check if mals_saxs_param_g_extinction_coef even used?
            if ( f_extc.count( name ) ) {
               f_extc.erase( name );
            }
            // f_extc      [ name ] = mals_saxs_param_g_extinction_coef;
            f_g_dndc    [ name ] = mals_saxs_param_g_dndc;
            f_dndc      [ name ] = mals_saxs_param_g_dndc;
            f_conc      [ name ] = mals_saxs_param_g_conc;
         }
         star_names.insert( name );
      }
   } // for each q value

   if ( !mode_testiq )
   {
      editor_msg( "dark blue", us_tr( "Finished: Make I(q)" ) );
      running = false;
   }
   progress->reset();
   if ( star_names.size() ) {
      set_selected( star_names );
   }
   update_enables();
   return true;
}
