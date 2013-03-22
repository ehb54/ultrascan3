#include "../include/us_hydrodyn.h"
#include "../include/us_revision.h"
#include "../include/us_hydrodyn_saxs_hplc.h"
#include "../include/us_hydrodyn_saxs_hplc_ciq.h"
#include "../include/us_hydrodyn_saxs_hplc_dctr.h"
#include "../include/us_hydrodyn_saxs_hplc_p3d.h"
#include "../include/us_hydrodyn_saxs_hplc_fit.h"
#include "../include/us_hydrodyn_saxs_hplc_fit_global.h"
#include "../include/us_lm.h"
#ifdef QT4
#include <qwt_scale_engine.h>
#endif

// note: this program uses cout and/or cerr and this should be replaced

#define SLASH QDir::separator()
#define Q_VAL_TOL 5e-6
#define UHSH_VAL_DEC 8
#define UHSH_UV_CONC_FACTOR 1e0

void US_Hydrodyn_Saxs_Hplc::create_i_of_q( QStringList files )
{
   // for each selected file
   // extract q grid from file names
   editor_msg( "dark blue", tr( "Starting: Make I(q)" ) );

   update_csv_conc();

   {
      QStringList tmp_files;
      for ( unsigned int i = 0; i < files.size(); i++ )
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

   if ( !ggaussian_compatible( false ) )
   {
      editor_msg( "red", tr( "NOTICE: Some files selected have Gaussians with varying centers or a different number of Gaussians or centers, Please enter \"Global Gaussians\" with these files selected and then \"Keep\" before pressing \"Make I(q)\"" ) );
      return;
   }

   QRegExp rx_q     ( "_q(\\d+_\\d+)" );
   QRegExp rx_bl    ( "-bl(.\\d*_\\d+(|e.\\d+))-(.\\d*_\\d+(|e.\\d+))s" );

   vector < QString > q_string;
   vector < double  > q;
   vector < double  > bl_slope;
   vector < double  > bl_intercept;

   bool         any_bl = false;
   unsigned int bl_count = 0;

   // get q and bl

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
      progress->setProgress( i, files.size() * 2 );
      qApp->processEvents();
      if ( rx_q.search( files[ i ] ) == -1 )
      {
         editor_msg( "red", QString( tr( "Error: Can not find q value in file name for %1" ) ).arg( files[ i ] ) );
         progress->reset();
         update_enables();
         return;
      }
      ql.push_back( rx_q.cap( 1 ).replace( "_", "." ).toDouble() );
      if ( used_q.count( ql.back() ) )
      {
         editor_msg( "red", QString( tr( "Error: Duplicate q value in file name for %1" ) ).arg( files[ i ] ) );
         progress->reset();
         update_enables();
         return;
      }
      used_q[ ql.back() ] = true;
         
      if ( rx_bl.search( files[ i ] ) == -1 )
      {
         bl_slope    .push_back( 0e0 );
         bl_intercept.push_back( 0e0 );
      } else {
         // cout << QString( "bl_cap 1 <%1>\n" ).arg( rx_bl.cap( 1 ) );
         // cout << QString( "bl_cap 2 <%1>\n" ).arg( rx_bl.cap( 3 ) );
         bl_slope    .push_back( rx_bl.cap( 1 ).replace( "_", "." ).toDouble() );
         bl_intercept.push_back( rx_bl.cap( 3 ).replace( "_", "." ).toDouble() );
         cout << QString( "bl for file %1 slope %2 intercept %3\n" ).arg( i ).arg( bl_slope.back(), 0, 'g', 8 ).arg( bl_intercept.back(), 0, 'g', 8 ).ascii();
         bl_count++;
         any_bl = true;
      }

      if ( !f_qs.count( files[ i ] ) )
      {
         editor_msg( "red", QString( tr( "Internal error: request to use %1, but not found in data" ) ).arg( files[ i ] ) );
      } else {
         for ( unsigned int j = 0; j < ( unsigned int ) f_qs[ files[ i ] ].size(); j++ )
         {
            I_values[ f_qs[ files[ i ] ][ j ] ][ ql.back() ] = f_Is[ files[ i ] ][ j ];
            if ( use_errors && f_errors[ files[ i ] ].size() == f_qs[ files[ i ] ].size() )
            {
               e_values[ f_qs[ files[ i ] ][ j ] ][ ql.back() ] = f_errors[ files[ i ] ][ j ];
            } else {
               if ( use_errors )
               {
                  use_errors = false;
                  editor_msg( "dark red", QString( tr( "Notice: missing errors, first noticed in %1, so no errors at all" ) )
                              .arg( files[ i ] ) );
               }
            }
            if ( !used_t.count( f_qs[ files[ i ] ][ j ] ) )
            {
               tl.push_back( f_qs[ files[ i ] ][ j ] );
               used_t[ f_qs[ files[ i ] ][ j ] ] = true;
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
                  if ( isnan( f_errors[ files[ i ] ][ j ] ) || f_errors[ files[ i ] ][ j ] == 0e0 )
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
         qsl_list_no_errors << QString( tr( "... and %1 more not listed" ) ).arg( qsl_no_errors.size() - qsl_list_no_errors.size() );
      }
      qs_no_errors = qsl_list_no_errors.join( "\n" );
      
      QStringList qsl_list_zero_points;
      for ( unsigned int i = 0; i < ( unsigned int ) qsl_zero_points.size() && i < 24 - used; i++ )
      {
         qsl_list_zero_points << qsl_zero_points[ i ];
      }
      if ( qsl_list_zero_points.size() < qsl_zero_points.size() )
      {
         qsl_list_zero_points << QString( tr( "... and %1 more not listed" ) ).arg( qsl_zero_points.size() - qsl_list_zero_points.size() );
      }
      qs_zero_points = qsl_list_zero_points.join( "\n" );
   }

   bool save_gaussians;

   vector < double > conv;
   vector < double > psv ;
   
   double conc_repeak = 1e0;
   
   bool normalize_by_conc = false;
   bool conc_ok           = false;

   bool sd_avg_local  = false;
   bool sd_drop_zeros = false;
   bool sd_keep_zeros = false;
   bool sd_set_pt1pct = false;
   bool save_sum      = false;

   bool sd_from_difference = false;
   {
      map < QString, QString > parameters;
      bool no_conc = false;
      if ( bl_count )
      {
         parameters[ "baseline" ] = 
            QString( tr( "Add back the baselines when making I(q).  Baselines were found for %1 of the %2 curves" ) )
            .arg( bl_count )
            .arg( files.size() );
      }

      parameters[ "gaussians" ] = QString( "%1" ).arg( f_gaussians[ files[ 0 ] ].size() / 3 );

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
         parameters[ "error" ] = QString( tr( "Concentration controls disabled: no concentration file set" ) );
         no_conc = true;
      } else {
         if ( !f_gaussians.count( lbl_conc_file->text() ) )
         {
            parameters[ "error" ] = QString( tr( "Concentration controls disabled: no Gaussians defined for concentration file" ) );
            no_conc = true;
         } else {
            if ( f_gaussians[ lbl_conc_file->text() ].size() / 3  != f_gaussians[ files[ 0 ] ].size() / 3 )
            {
               parameters[ "error" ] = 
                  QString( tr( "Concentration controls disabled: Concentration file Gaussian count (%1)\n does not match global curves Gaussian count (%2)" ) )
                  .arg( f_gaussians[ lbl_conc_file->text() ].size() )
                  .arg( f_gaussians[ files[ 0 ] ].size() / 3 )
                  ;
               no_conc = true;
            }
         }
      }

      if ( !no_conc )
      {
         QRegExp rx_repeak( "-rp(.\\d*_\\d+(|e.\\d+))" );
         if ( rx_repeak.search( lbl_conc_file->text() ) != -1 )
         {
            conc_repeak = rx_repeak.cap( 1 ).replace( "_", "." ).toDouble();
            if ( conc_repeak == 0e0 )
            {
               conc_repeak = 1e0;
               editor_msg( "red", tr( "Error: concentration repeak scaling value extracted is 0, turning off back scaling" ) );
            }
         }
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

      //       cout << "parameters b4 ciq:\n";
      //       for ( map < QString, QString >::iterator it = parameters.begin();
      //             it != parameters.end();
      //             it++ )
      //       {
      //          cout << QString( "%1:%2\n" ).arg( it->first ).arg( it->second );
      //       }
      //       cout << "end parameters b4 ciq:\n";

      US_Hydrodyn_Saxs_Hplc_Ciq *hplc_ciq = 
         new US_Hydrodyn_Saxs_Hplc_Ciq(
                                       this,
                                       & parameters,
                                       this );
      US_Hydrodyn::fixWinButtons( hplc_ciq );
      hplc_ciq->exec();
      delete hplc_ciq;
      
      //       cout << "parameters:\n";
      //       for ( map < QString, QString >::iterator it = parameters.begin();
      //             it != parameters.end();
      //             it++ )
      //       {
      //          cout << QString( "%1:%2\n" ).arg( it->first ).arg( it->second );
      //       }
      //       cout << "end parameters:\n";

      if ( bl_count && ( !parameters.count( "add_baseline" ) || parameters[ "add_baseline" ] == "false" ) )
      {
         bl_count = 0;
         cout << "ciq: bl off\n";
      }

      if ( parameters.count( "save_as_pct_iq" ) && parameters[ "save_as_pct_iq" ] == "true" )
      {
         save_gaussians = false;
         cout << "ciq: save_gaussians false\n";
      } else {
         save_gaussians = true;
         cout << "ciq: save_gaussians true\n";
      }
      if ( parameters.count( "save_sum" ) && parameters[ "save_sum" ] == "true" )
      {
         save_sum = true;
      }
      if ( parameters.count( "sd_source" ) && parameters[ "sd_source" ] == "difference" )
      {
         sd_from_difference = true;
         cout << "ciq: sd_from_difference true\n";
      } else {
         sd_from_difference = false;
         cout << "ciq: sd_from_difference false\n";
      }
      if ( !parameters.count( "go" ) )
      {
         progress->reset();
         update_enables();
         return;
      }

      if ( !no_conc )
      {
         for ( unsigned int i = 0; i < ( unsigned int ) f_gaussians[ files[ 0 ] ].size() / 3; i++ )
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

      cout << QString( "sd_avg_local  %1\n"
                       "sd_drop_zeros %2\n"
                       "sd_set_pt1pct %3\n"
                       "sd_keep_zeros %4\n" )
         .arg( sd_avg_local  ? "true" : "false" )
         .arg( sd_drop_zeros ? "true" : "false" )
         .arg( sd_set_pt1pct ? "true" : "false" )
         .arg( sd_keep_zeros ? "true" : "false" )
         ;
   }

   //    if ( bl_count &&
   //         QMessageBox::question(this, 
   //                               this->caption(),
   //                               QString( tr( "Baselines were found for %1 of the %2 curves\n"
   //                                            "Do you want to add back the baselines when making I(q)?" ) )
   //                               .arg( bl_count )
   //                               .arg( files.size() ),
   //                               tr( "&Yes" ),
   //                               tr( "&No" ),
   //                               QString::null,
   //                               0,
   //                               1
   //                               ) == 1 )
   //    {
   //       cout << "not using baselines\n";
   //       bl_count = 0;
   //    }

   //    bool save_gaussians = 
   //         QMessageBox::question(this, 
   //                               this->caption(),
   //                               tr( "Save as Gaussians or a percent of the original I(q)?" ),
   //                               tr( "&Gaussians" ),
   //                               tr( "Percent of &I(q)" ),
   //                               QString::null,
   //                               0,
   //                               1
   //                               ) == 0;
   running = true;

   vector < vector < double > > concs;

   if ( normalize_by_conc || conc_ok )
   {
      // test, produce conc curves for each gaussian
      for ( unsigned int i = 0; i < ( unsigned int )conv.size(); i++ )
      {
         // add_plot( QString( "conc_g_per_ml_peak%1" ).arg( i + 1 ), tv, conc_curve( tv, i, conv[ i ] ), true, false );
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
            vector < double > tmp_g(3);
            QString conc_file = lbl_conc_file->text();
            tmp_g[ 0 ] = f_gaussians[ conc_file ][ 0 + i * 3 ] * detector_conv / ( conc_repeak * conv[ i ] );
            tmp_g[ 1 ] = f_gaussians[ conc_file ][ 1 + i * 3 ];
            tmp_g[ 2 ] = f_gaussians[ conc_file ][ 2 + i * 3 ];

            concs.push_back( compute_gaussian( tv, tmp_g ) );
            add_plot( QString( "conc_g_per_ml_peak%1" ).arg( i + 1 ), tv, concs.back(), true, false );
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


   for ( unsigned int i = 0; i < files.size(); i++ )
   {
      vector < vector < double > > tmp_v;
      vector < double >            tmp_sum;
      vector < double >            tmp_area;
      double                       tmp_area_sum = 0e0;

      for ( unsigned int j = 0; j < ( unsigned int ) f_gaussians[ files[ i ] ].size(); j += 3 )
      {
         vector < double > tmp_g(3);
         tmp_g[ 0 ] = f_gaussians[ files[ i ] ][ 0 + j ];
         tmp_g[ 1 ] = f_gaussians[ files[ i ] ][ 1 + j ];
         tmp_g[ 2 ] = f_gaussians[ files[ i ] ][ 2 + j ];

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

         // add_plot( QString( "fg_%1_g%2" ).arg( i ).arg( j / 3 ), tv, tmp, true, false );

      }
      fg.push_back( tmp_v );
      fs.push_back( tmp_sum );

      for ( unsigned int j = 0; j < ( unsigned int ) tmp_area.size(); j++ )
      {
         tmp_area[ j ] /= tmp_area_sum;
      }
         
      g_area    .push_back( tmp_area );
      g_area_sum.push_back( tmp_area_sum );
      US_Vector::printvector( QString( "areas file %1 (sum %2)" ).arg( i ).arg( tmp_area_sum, 0, 'g', 8 ), tmp_area );
      // add_plot( QString( "fg_%1_gsum" ).arg( i ), tv, tmp_sum, true, false );
   }

   US_Vector::printvector( "area sums", g_area_sum );

   // build up resulting curves

   // for each time, tv[ t ] 
   unsigned int num_of_gauss = ( unsigned int ) gaussians.size() / 3;
   // cout << QString( "num of gauss %1\n" ).arg( num_of_gauss );

   bool reported_gs0 = false;

   for ( unsigned int t = 0; t < tv.size(); t++ )
   {
      progress->setProgress( files.size() + t, files.size() + tv.size() );
      // for each gaussian 
      vector < double > gsI;
      vector < double > gse;
      vector < double > gsG;
      // vector < double > gsI_recon;
      // vector < double > gsG_recon;

      // for "sd by differece"
      vector < vector < double > > used_pcts;
      vector < QString >           used_names;

      for ( unsigned int g = 0; g < num_of_gauss; g++ )
      {
         // build up an I(q)
         double conc_factor = 0e0;
         double norm_factor = 1e0;
         QString qs_fwhm;
         if ( ( conc_ok || normalize_by_conc ) && concs[ g ][ t ] > 0e0 )
         {
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

            vector < double > tmp_g(3);
            QString conc_file = lbl_conc_file->text();
            tmp_g[ 0 ] = f_gaussians[ conc_file ][ 0 + g * 3 ] * detector_conv / ( conc_repeak * conv[ g ] );
            tmp_g[ 1 ] = f_gaussians[ conc_file ][ 1 + g * 3 ];
            tmp_g[ 2 ] = f_gaussians[ conc_file ][ 2 + g * 3 ];

            double center = tmp_g[ 1 ];
            double width  = tmp_g[ 2 ];
            double fwhm   = 2.354820045e0 * width;
            //             cout << QString( "peak %1 center %2 fwhm %3 t %4 tv[t] %5\n" )
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
            .arg( any_bl   ? "_bs" : "" )
            .arg( bl_count ? "ba" : "" )
            .arg( g + 1 )
            .arg( normalize_by_conc ? 
                  QString( "%1_cn%2" )
                  .arg( qs_fwhm )
                  .arg( conc_factor, 0, 'g', 6 ) : QString( "" ) )
            .arg( tv[ t ] )
            .replace( ".", "_" )
            ;

         // cout << QString( "name %1\n" ).arg( name );

         // now go through all the files to pick out the I values and errors and distribute amoungst the various gaussian peaks
         // we could also reassemble the original sum of gaussians curves as a comparative

         vector < double > I;
         vector < double > e;
         vector < double > G;

         // vector < double > I_recon;
         // vector < double > G_recon;

         vector < double > this_used_pcts;

         for ( unsigned int i = 0; i < ( unsigned int ) files.size(); i++ )
         {
            if ( !I_values.count( tv[ t ] ) )
            {
               editor_msg( "dark red", QString( tr( "Notice: I values missing frame/time = %1" ) ).arg( tv[ t ] ) );
               //                running = false;
               //                update_enables();
               //                return;
               continue;
            }

            if ( !I_values[ tv[ t ] ].count( qv[ i ] ) )
            {
               editor_msg( "red", QString( tr( "Notice: I values missing q = %1" ) ).arg( qv[ i ] ) );
               //                running = false;
               //                update_enables();
               //                return;
               continue;
            }

            double tmp_I       = I_values[ tv[ t ] ][ qv[ i ] ];
            double tmp_e       = 0e0;
            double tmp_G       = fg[ i ][ g ][ t ];

            double frac_of_gaussian_sum;
            if ( fs[ i ][ t ] == 0e0 )
            {
               if ( !reported_gs0 )
               {
                  cout << QString( "Notice: file %1 t %2 gaussian sum is zero (further instances ignored)\n" ).arg( i ).arg( t );
                  reported_gs0 = true;
               }
               frac_of_gaussian_sum = 1e0 / ( double ) num_of_gauss;
            } else {
               frac_of_gaussian_sum = tmp_G / fs[ i ][ t ];
            }

            if ( use_errors )
            {
               if ( !e_values.count( tv[ t ] ) )
               {
                  editor_msg( "red", QString( tr( "Internal error: error values missing t %1" ) ).arg( tv[ t ] ) );
                  running = false;
                  update_enables();
                  progress->reset();
                  return;
               }

               if ( !e_values[ tv[ t ] ].count( qv[ i ] ) )
               {
                  editor_msg( "red", QString( tr( "Internal error: error values missing q %1" ) ).arg( qv[ i ] ) );
                  running = false;
                  update_enables();
                  progress->reset();
                  return;
               }

               tmp_e = e_values[ tv[ t ] ][ qv[ i ] ];
            }
            
            tmp_I *= frac_of_gaussian_sum;
            tmp_e *= frac_of_gaussian_sum;

            if ( sd_from_difference )
            {
               this_used_pcts.push_back( frac_of_gaussian_sum );
            }

            // double tmp_I_recon = tmp_I;
            // double tmp_G_recon = tmp_G;

            if ( bl_count )
            {
               double pct_area = 1e0 / ( double ) num_of_gauss; // g_area[ i ][ g ];
               double ofs = ( bl_intercept[ i ] + tv[ t ] * bl_slope[ i ] ) * pct_area;
               tmp_I += ofs;
               tmp_G += ofs;
               // tmp_I_recon += ofs;
               // tmp_G_recon += ofs;
            }

            I      .push_back( tmp_I );
            e      .push_back( tmp_e );
            G      .push_back( tmp_G );
            // I_recon.push_back( tmp_I_recon );
            // G_recon.push_back( tmp_G_recon );
         } // for each file
         
         if ( g )
         {
            for ( unsigned int m = 0; m < ( unsigned int ) qv.size(); m++ )
            {
               gsI[ m ]       += I[ m ];
               gse[ m ]       += e[ m ];
               gsG[ m ]       += G[ m ];
               // gsI_recon[ m ] += I_recon[ m ];
               // gsG_recon[ m ] += G_recon[ m ];
            }
         } else {
            gsI       = I;
            gsG       = G;
            gse       = e;
            // gsI_recon = I_recon;
            // gsG_recon = G_recon;
         }

         // add to csv conc stuff?

         lb_created_files->insertItem( name );
         lb_created_files->setBottomItem( lb_created_files->numRows() - 1 );
         lb_files->insertItem( name );
         lb_files->setBottomItem( lb_files->numRows() - 1 );
         created_files_not_saved[ name ] = true;
   
         vector < QString > use_qv_string = qv_string;
         vector < double  > use_qv        = qv;
         vector < double  > use_I         = save_gaussians ? G : I;
         vector < double  > use_e         = e;

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
            used_names.push_back( name );
            used_pcts .push_back( this_used_pcts );
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
                  editor_msg( "dark red", tr( "Warning: too many adjacent S.D. zeros, could not set all S.D.'s to non-zero values" ) );
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

         f_pos       [ name ] = f_qs.size();
         f_qs_string [ name ] = use_qv_string;
         f_qs        [ name ] = use_qv;
         f_Is        [ name ] = use_I;
         f_errors    [ name ] = use_e;
         f_is_time   [ name ] = false;
         f_conc      [ name ] = conc_factor;
         f_psv       [ name ] = psv.size() > g ? psv[ g ] : 0e0;

         {
            vector < double > tmp;
            f_gaussians  [ name ] = tmp;
         }
      } // for each gaussian

      if ( sd_from_difference )
      {
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

            for ( unsigned int j = 0; j < ( unsigned int ) use_e.size(); j++ )
            {
               use_e[ j ] *= used_pcts[ i ][ j ];
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
                  editor_msg( "dark red", tr( "Warning: too many adjacent S.D. zeros, could not set all S.D.'s to non-zero values" ) );
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
            
            f_qs_string[ used_names[ i ] ] = use_qv_string;
            f_qs       [ used_names[ i ] ] = use_qv;
            f_Is       [ used_names[ i ] ] = use_I;
            f_errors   [ used_names[ i ] ] = use_e;
         }
      }         
      if ( save_sum )
      {
         if ( save_gaussians )
         {
            add_plot( QString( "sumG_T%1" ).arg( pad_zeros( tv[ t ], (int) tv.size() ) ), qv, gsG, gse, false, false );
         } else {
            add_plot( QString( "sumI_T%1" ).arg( pad_zeros( tv[ t ], (int) tv.size() ) ), qv, gsI, gse, false, false );
         }
      }

      // add_plot( QString( "sumIr_T%1" ).arg( pad_zeros( tv[ t ], (int) tv.size() ) ), qv, gsI_recon, gse, false, false );
      // add_plot( QString( "sumGr_T%1" ).arg( pad_zeros( tv[ t ], (int) tv.size() ) ), qv, gsG_recon, gse, false, false );
   } // for each q value

   editor_msg( "dark blue", tr( "Finished: Make I(q)" ) );
   progress->setProgress( 1, 1 );
   running = false;
   update_enables();
}

