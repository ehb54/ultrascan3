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

void US_Hydrodyn_Saxs_Hplc::invert_all_created()
{
   disable_all();
   lb_created_files->invertSelection();
   update_enables();
}

void US_Hydrodyn_Saxs_Hplc::remove_created()
{
   disable_all();
   QStringList files;
   for (int i = 0; i < (int) lb_created_files->numRows(); i++ )
   {
      if ( lb_created_files->isSelected( i ) )
      {
         files << lb_created_files->text( i );
      }
   }
   clear_files( files );
   update_enables();
}

void US_Hydrodyn_Saxs_Hplc::add()
{
   QStringList files = all_selected_files();

   vector < double > sum = f_Is[ files[ 0 ] ];
   vector < double > e   = f_errors[ files[ 0 ] ];

   bool use_errors;

   if ( f_errors.count( files[ 0 ] ) &&
        f_errors[ files[ 0 ] ].size() == f_qs[ files[ 0 ] ].size() )
   {
      use_errors = true;
      e = f_errors[ files[ 0 ] ];
   } else {
      use_errors = false;
   }

   disable_all();

   QString name = tr( "sum_" ) + files[ 0 ];

   for ( unsigned int i = 1; i < ( unsigned int ) files.size(); i++ )
   {
      if ( f_qs[ files[ 0 ] ] != f_qs[ files[ i ] ] )
      {
         editor_msg( "red", QString( tr( "Error: Residuals incompatible grids (comparing %1 and %2). Suggest: Crop Common" ) ).arg( files[ 0 ] ).arg( files[ i ] ) );
         update_enables();
         return;
      }
      
      name += "+" + files[ i ];

      for ( unsigned int j = 0; j < ( unsigned int ) sum.size(); j++ )
      {
         sum[ j ] += f_Is[ files[ i ] ][ j ];
      }

      if ( !f_errors.count( files[ i ] ) ||
           f_errors[ files[ i ] ].size() != f_qs[ files[ i ] ].size() )
      {
         use_errors = false;
      }
      if ( use_errors )
      {
         for ( unsigned int j = 0; j < ( unsigned int ) sum.size(); j++ )
         {
            e[ j ] += f_errors[ files[ i ] ][ j ];
         }
      }         
   }

   if ( use_errors )
   {
      add_plot( name, f_qs[ files[ 0 ] ], sum, e, f_is_time.count( files[ 0 ] ) ? f_is_time[ files[ 0 ] ] : false, false );
   } else {
      add_plot( name, f_qs[ files[ 0 ] ], sum, f_is_time.count( files[ 0 ] ) ? f_is_time[ files[ 0 ] ] : false, false );
   }
   update_enables();
}

bool US_Hydrodyn_Saxs_Hplc::all_have_f_gaussians( QStringList & files )
{
   for ( unsigned int i = 0; i < ( unsigned int )files.size(); i++ )
   {
      if ( !f_gaussians.count( files[ i ] ) ||
           !f_gaussians[ files[ i ] ].size() )
      {
         return false;
      }
   }
   return true;
}

void US_Hydrodyn_Saxs_Hplc::p3d()
{
   disable_all();
   // this is for global gaussians for now

   QStringList files = all_selected_files();

   if ( !all_have_f_gaussians( files ) )
   {
      editor_msg( "red", tr( "Error: Not all files have Gaussians defined" ) );
      update_enables();
      return;
   }
   if ( !ggaussian_compatible( files, false ) )
   {
      editor_msg( "dark red", 
                  cb_fix_width->isChecked() ?
                  tr( "NOTICE: Some files selected have Gaussians with varying centers or widths or a different number of Gaussians or centers or widths that do not match the last Gaussians" )
                  :
                  tr( "NOTICE: Some files selected have Gaussians with varying centers or a different number of Gaussians or centers that do not match the last Gaussians." ) 
                  );
      update_enables();
      return;
   }

   for ( unsigned int i = 1; i < ( unsigned int ) files.size(); i++ )
   {
      if ( f_qs[ files[ 0 ] ] != f_qs[ files[ i ] ] )
      {
         editor_msg( "red", QString( tr( "Error: Incompatible grids (comparing %1 and %2). Suggest: Crop Common" ) ).arg( files[ 0 ] ).arg( files[ i ] ) );
         update_enables();
         return;
      }
   }

   // get q range
   vector < double  > q;
   {
      list < double >      ql;
      map < double, bool > used_q;
      QRegExp rx_q( "_q(\\d+_\\d+)" );
      for ( unsigned int i = 0; i < ( unsigned int ) files.size(); i++ )
      {
         if ( rx_q.search( files[ i ] ) == -1 )
         {
            editor_msg( "red", QString( tr( "Error: Can not find q value in file name for %1" ) ).arg( files[ i ] ) );
            update_enables();
            return;
         }
         ql.push_back( rx_q.cap( 1 ).replace( "_", "." ).toDouble() );

         if ( used_q.count( ql.back() ) )
         {
            editor_msg( "red", QString( tr( "Error: Duplicate q value in file name for %1" ) ).arg( files[ i ] ) );
            update_enables();
            return;
         }
         used_q[ ql.back() ] = true;
      }         
      ql.sort();

      for ( list < double >::iterator it = ql.begin();
            it != ql.end();
            it++ )
      {
         q.push_back( *it );
      }
   }

   map < unsigned int, bool > g_to_plot;

   QString title = caption() + ": Gaussians :";

   {
      map < QString, QString > parameters;
      parameters[ "gaussians" ] = QString( "%1" ).arg( f_gaussians[ files[ 0 ] ].size() / 3 );

      US_Hydrodyn_Saxs_Hplc_P3d *hplc_p3d = 
         new US_Hydrodyn_Saxs_Hplc_P3d(
                                       this,
                                       & parameters,
                                       this );
      US_Hydrodyn::fixWinButtons( hplc_p3d );
      hplc_p3d->exec();
      delete hplc_p3d;

      if ( !parameters.count( "plot" ) )
      {
         update_enables();
         return;
      }

      for ( unsigned int i = 0; i < parameters[ "gaussians" ].toUInt(); i++ )
      {
         if ( parameters.count( QString( "%1" ).arg( i ) ) )
         {
            g_to_plot[ i ] = true;
            title += QString( " %1" ).arg( i + 1 );
         }
      }
   }

   if ( !g_to_plot.size() )
   {
      editor_msg( "dark red", tr( "Plot 3D: no Gaussians selected to plot" ) );
      update_enables();
      return;
   }

   // compute partial (selected) gaussians sums:

   // compute all gaussians over q range

   // vector < vector < vector < double > > > fg; // a vector of the individual gaussians
   vector < vector < double > >            fs; // a vector of the gaussian sums
   vector < vector < double > >            g_area;      // a vector of the gaussian area
   vector < double >                       g_area_sum; // a vector of the gaussian area

   for ( unsigned int i = 0; i < files.size(); i++ )
   {
      vector < vector < double > > tmp_v;
      vector < double >            tmp_sum;
      vector < double >            tmp_area;
      double                       tmp_area_sum = 0e0;

      bool any_accumulated        = false;

      for ( unsigned int j = 0; j < ( unsigned int ) f_gaussians[ files[ i ] ].size(); j += 3 )
      {
         if ( g_to_plot.count( j / 3 ) )
         {
            vector < double > tmp_g(3);
            tmp_g[ 0 ] = f_gaussians[ files[ i ] ][ 0 + j ];
            tmp_g[ 1 ] = f_gaussians[ files[ i ] ][ 1 + j ];
            tmp_g[ 2 ] = f_gaussians[ files[ i ] ][ 2 + j ];

            vector < double > tmp = compute_gaussian( f_qs[ files[ i ] ], tmp_g );
            tmp_v.push_back( tmp );
            if ( any_accumulated )
            {
               for ( unsigned int k = 0; k < tmp.size(); k++ )
               {
                  tmp_sum[ k ] += tmp[ k ];
               }
            } else {
               any_accumulated = true;
               tmp_sum = tmp;
            }

            tmp_area.push_back( tmp_g[ 0 ] * tmp_g[ 2 ] * M_SQRT2PI );
            tmp_area_sum += tmp_area.back();
         }
      }
      // fg.push_back( tmp_v );
      fs.push_back( tmp_sum );

      for ( unsigned int j = 0; j < ( unsigned int ) tmp_area.size(); j++ )
      {
         tmp_area[ j ] /= tmp_area_sum;
      }
         
      g_area    .push_back( tmp_area );
      g_area_sum.push_back( tmp_area_sum );
   }

   // plot 3d
   {
      QString xtitle;
      QString ytitle;
      QString ztitle;

      if ( !QGLFormat::hasOpenGL() )
      {
         editor_msg( "red", tr( "This system has no OpenGL support." ) );
         update_enables();
         return;
      }

      double **data3d;

      unsigned int rows    = ( unsigned int ) fs.size();
      unsigned int columns = ( unsigned int ) fs[ 0 ].size();

      data3d = new double * [rows];

      for ( unsigned int i = 0; i < rows; i++ )
      {
         data3d[ i ] = new double [ columns ];
      }

      double maxI = 0e0;
      for ( unsigned int i = 0; i < rows; i++ )
      {
         // cout << QString( "row %1:" ).arg( i );
         for ( unsigned int j = 0; j < columns; j++ )
         {
            // cout << QString( "%1 " ).arg( data3d[ i ][ j ] );
            if ( maxI < fs[ i ][ j ] )
            {
               maxI = fs[ i ][ j ];
            }
         }
         // cout << endl;
      }

      for ( unsigned int i = 0; i < rows; i++ )
      {
         // cout << QString( "row %1:" ).arg( i );
         for ( unsigned int j = 0; j < columns; j++ )
         {
            data3d[ i ][ j ] = fs[ i ][ j ] / maxI;
            // cout << QString( "%1 " ).arg( data3d[ i ][ j ] );
            if ( maxI < fs[ i ][ j ] )
            {
               maxI = fs[ i ][ j ];
            }
         }
         // cout << endl;
      }

      SA2d_control_variables controlvar_3d;

      controlvar_3d.minx = q[ 0 ];
      controlvar_3d.maxx = q.back();

      controlvar_3d.miny = f_qs[ files[ 0 ] ][ 0 ];
      controlvar_3d.maxy = f_qs[ files[ 0 ] ].back();

      controlvar_3d.xscaling = 1e0 / ( q.back() - q[ 0 ] );
      controlvar_3d.yscaling = 10e0 / ( f_qs[ files[ 0 ] ].back() - f_qs[ files[ 0 ] ][ 0 ] );
      controlvar_3d.zscaling = 1000e0 / maxI;

      controlvar_3d.minx = 0e0;  // f_qs[ files[ 0 ] ][ 0 ];
      controlvar_3d.maxx = 10e0; // f_qs[ files[ 0 ] ].back();
      controlvar_3d.miny = 0;
      controlvar_3d.maxy = 10e0;
      controlvar_3d.xscaling = 1e0;
      controlvar_3d.yscaling = 1e0;
      controlvar_3d.zscaling = 10e0; // 10e0 / maxI > 1e0 ? 10e0 / maxI : 1e0;

      cout << QString( "3d params t %1,%2 scaling %3\n"
                       "          q %4,%5 scaling %6\n"
                       "       maxI %7    scaling %8\n" )
         .arg( controlvar_3d.minx ).arg( controlvar_3d.maxx ).arg( controlvar_3d.xscaling )
         .arg( controlvar_3d.miny ).arg( controlvar_3d.maxy ).arg( controlvar_3d.yscaling )
         .arg( maxI ).arg( controlvar_3d.zscaling );

      controlvar_3d.meshx = rows;
      controlvar_3d.meshy = columns;

      xtitle = "q (relative)";
      ytitle = "Frame (relative)";
      ztitle = "I(q)";

      bool raise = plot3d_flag;
      if ( plot3d_flag )
      {
         plot3d_window->setParameters( title, xtitle, ytitle, ztitle, data3d, &controlvar_3d );
      }
      else
      {
         plot3d_window = new Mesh2MainWindow( &plot3d_flag, title, xtitle, ytitle, ztitle, data3d, &controlvar_3d );
      }

      plot3d_window->dataWidget->coordinates()->setStandardScale();
      plot3d_window->dataWidget->updateGL();

      if ( raise )
      {
         plot3d_window->raise();
      } else {
         plot3d_window->show();
      }

      for ( unsigned int i = 0; i < rows; i++ )
      {
         delete [] data3d[i];
      }
      delete [] data3d;
   }
   update_enables();
}

void US_Hydrodyn_Saxs_Hplc::set_detector()
{
   disable_all();

   {
      map < QString, QString > parameters;
      parameters[ "uv_conv" ] = QString( "%1" ).arg( detector_uv_conv, 0, 'g', 8 );
      parameters[ "ri_conv" ] = QString( "%1" ).arg( detector_ri_conv, 0, 'g', 8 );
      if ( detector_uv )
      {
         parameters[ "uv" ] = "true";
      } else {
         if ( detector_ri )
         {
            parameters[ "ri" ] = "true";
         }
      }

      US_Hydrodyn_Saxs_Hplc_Dctr *hplc_dctr = 
         new US_Hydrodyn_Saxs_Hplc_Dctr(
                                        this,
                                        & parameters,
                                        this );
      US_Hydrodyn::fixWinButtons( hplc_dctr );
      hplc_dctr->exec();
      delete hplc_dctr;

      if ( !parameters.count( "keep" ) )
      {
         update_enables();
         return;
      }

      detector_uv      = ( parameters.count( "uv" ) && parameters[ "uv" ] == "true" ) ? true : false;
      detector_ri      = ( parameters.count( "ri" ) && parameters[ "ri" ] == "true" ) ? true : false;
      detector_ri_conv = parameters[ "ri_conv" ].toDouble();
      detector_uv_conv = parameters[ "uv_conv" ].toDouble();
   }
   update_enables();
}


/*

   for ( int i = 0; i < (int)files.size(); i++ )
   {
      QString this_file = files[ i ];
      selected_map[ this_file ] = true;
      if ( created_files_not_saved.count( this_file ) )
      {
         created_not_saved_list << this_file;
         created_not_saved_map[ this_file ] = true;
      }
   }

   if ( created_not_saved_list.size() )
   {
      QStringList qsl;
      for ( int i = 0; i < (int)created_not_saved_list.size() && i < 15; i++ )
      {
         qsl << created_not_saved_list[ i ];
      }

      if ( qsl.size() < created_not_saved_list.size() )
      {
         qsl << QString( tr( "... and %1 more not listed" ) ).arg( created_not_saved_list.size() - qsl.size() );
      }

      switch ( QMessageBox::warning(this, 
                                    tr( "US-SOMO: SAXS Hplc Remove Files" ),
                                    QString( tr( "Please note:\n\n"
                                                 "These files were created but not saved as .dat files:\n"
                                                 "%1\n\n"
                                                 "What would you like to do?\n" ) )
                                    .arg( qsl.join( "\n" ) ),
                                    tr( "&Save them now" ), 
                                    tr( "&Remove them anyway" ), 
                                    tr( "&Quit from removing files" ), 
                                    0, // Stop == button 0
                                    0 // Escape == button 0
                                    ) )
      {
      case 0 : // save them now
         // set the ones listed to selected
         if ( !save_files( created_not_saved_list ) )
         {
            return;
         }
      case 1 : // just remove them
         break;
      case 2 : // quit
         disable_updates = false;
         return;
         break;
      }

   }
*/

bool US_Hydrodyn_Saxs_Hplc::ggauss_recompute()
{
   unified_ggaussian_q               .clear();
   unified_ggaussian_jumps           .clear();
   unified_ggaussian_I               .clear();
   unified_ggaussian_e               .clear();
   unified_ggaussian_t               .clear();
   unified_ggaussian_param_index     .clear();
   unified_ggaussian_q_start         .clear();
   unified_ggaussian_q_end           .clear();
   
   double q_start = le_gauss_fit_start->text().toDouble();
   double q_end   = le_gauss_fit_end  ->text().toDouble();

   unified_ggaussian_jumps  .push_back( 0e0 );

   bool error_msg = false;

   for ( unsigned int i = 0; i < ( unsigned int ) unified_ggaussian_files.size(); i++ )
   {
      if ( i )
      {
         unified_ggaussian_jumps.push_back( unified_ggaussian_t.back() );
      }
      if ( !f_qs.count( unified_ggaussian_files[ i ] ) ||
           f_qs[ unified_ggaussian_files[ i ] ].size() < 2 )
      {
         editor_msg( "red", QString( tr( "Internal error: %1 has no or empty or insufficient data" ) ).arg( unified_ggaussian_files[ i ] ) );
         return false;
      }

      if ( unified_ggaussian_use_errors && 
           ( !f_errors.count( unified_ggaussian_files[ i ] ) ||
             f_errors[ unified_ggaussian_files[ i ] ].size() != f_qs[ unified_ggaussian_files[ i ] ].size() ) )
      {
         editor_msg( "dark red", QString( tr( "WARNING: %1 has no errors so global errors are off for computing RMSD and global fitting" ) ).arg( unified_ggaussian_files[ i ] ) );
         error_msg = true;
         unified_ggaussian_use_errors = false;
         unified_ggaussian_e.clear();
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
            if ( cb_fix_width->isChecked() )
            {
               unified_ggaussian_param_index .push_back( unified_ggaussian_gaussians_size * ( 2 + i ) );
            } else {
               unified_ggaussian_param_index .push_back( unified_ggaussian_gaussians_size * ( 1 + i * 2 ) );
            }               
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
         editor_msg( "dark red", tr( "WARNING: some errors are zero so global errors are off for computing RMSD and global fitting" ) );
      }
   }

   //    US_Vector::printvector( "q_start", unified_ggaussian_q_start );
   //    US_Vector::printvector( "q_end"  , unified_ggaussian_q_end   );

   pb_ggauss_rmsd->setEnabled( false );
   return true;
}

bool US_Hydrodyn_Saxs_Hplc::create_unified_ggaussian_target( bool do_init )
{
   QStringList files = all_selected_files();
   return create_unified_ggaussian_target( files, do_init );
}

bool US_Hydrodyn_Saxs_Hplc::create_unified_ggaussian_target( QStringList & files, bool do_init )
{
   unified_ggaussian_ok = false;

   org_gaussians = gaussians;
   // US_Vector::printvector( "cugt: org_gauss", org_gaussians );

   unified_ggaussian_params          .clear();

   unified_ggaussian_files           = files;
   unified_ggaussian_curves          = files.size();

   unified_ggaussian_use_errors      = true;

   // for testing
   // unified_ggaussian_use_errors      = false;

   unified_ggaussian_gaussians_size  = ( unsigned int ) gaussians.size() / 3;

   if ( do_init )
   {
      unified_ggaussian_errors_skip = false;
      if ( !initial_ggaussian_fit( files ) )
      {
         progress->reset();
         return false;
      }
   }

   // push back centers first
   for ( unsigned int i = 0; i < ( unsigned int ) gaussians.size(); i += 3 )
   {
      unified_ggaussian_params.push_back( gaussians[ 1 + i ] );
      if ( cb_fix_width->isChecked() )
      {
         unified_ggaussian_params.push_back( gaussians[ 2 + i ] );
      }
   }

   // now push back all the file specific amplitude & widths

   map < QString, bool >    no_errors;
   map < QString, QString > zero_points;
   QStringList              qsl_no_errors;
   QStringList              qsl_zero_points;

   for ( unsigned int i = 0; i < ( unsigned int ) files.size(); i++ )
   {
      if ( !f_gaussians.count( files[ i ] ) )
      {
         editor_msg( "red", QString( tr( "Internal error: %1 does not have a gaussian set" ) ).arg( files[ i ] ) );
         return false;
      }
      if ( f_gaussians[ files[ i ] ].size() != gaussians.size() )
      {
         editor_msg( "red", QString( tr( "Internal error: %1 has an incompatible gaussian set" ) ).arg( files[ i ] ) );
         return false;
      }
      
      for ( unsigned int j = 0; j < ( unsigned int ) f_gaussians[ files[ i ] ].size(); j += 3 )
      {
         unified_ggaussian_params.push_back( f_gaussians[ files[ i ] ][ 0 + j ] ); // height
         if ( !cb_fix_width->isChecked() )
         {
            unified_ggaussian_params.push_back( f_gaussians[ files[ i ] ][ 2 + j ] ); // width
         }
      }

      if ( cb_sd_weight->isChecked() )
      {
         if ( !f_errors.count( files[ i ] ) ||
              f_errors[ files[ i ] ].size() != f_Is[ files[ i ] ].size() )
         {
            cout << QString( "file %1 no errors %2\n" )
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

      QStringList qsl_list_zero_points;
      for ( unsigned int i = 0; i < ( unsigned int ) qsl_zero_points.size() && i < 24 - used; i++ )
      {
         qsl_list_zero_points << qsl_zero_points[ i ];
      }
      if ( qsl_list_zero_points.size() < qsl_zero_points.size() )
      {
         qsl_list_zero_points << QString( tr( "... and %1 more not listed" ) ).arg( qsl_zero_points.size() - qsl_list_zero_points.size() );
      }

      switch ( QMessageBox::warning(this, 
                                    caption() + tr( ": Create unified global Gaussians" ),
                                    QString( tr( "Please note:\n\n"
                                                 "%1"
                                                 "%2"
                                                 "What would you like to do?\n" ) )
                                    .arg( qsl_list_no_errors.size() ?
                                          QString( tr( "These files have no associated errors:\n%1\n\n" ) ).arg( qsl_list_no_errors.join( "\n" ) ) : "" )
                                    .arg( qsl_list_zero_points.size() ?
                                          QString( tr( "These files have zero points:\n%1\n\n" ) ).arg( qsl_list_zero_points.join( "\n" ) ) : "" )
                                    ,
                                    tr( "&Turn off SD weighting" ), 
                                    tr( "Drop &full curves with zero SDs" ), 
                                    qsl_zero_points.size() ? tr( "Drop &points with zero SDs" ) : QString::null, 
                                    0, // Stop == button 0
                                    0 // Escape == button 0
                                    ) )
      {
      case 0 : // turn off sd weighting
         {
            cb_sd_weight->setChecked( false );
            return create_unified_ggaussian_target( files, false );
         }
         break;
      case 1 : // drop zero sd curves
         {
            running = true;
            disable_updates = true;
            for ( int i = 0; i < lb_files->numRows(); i++ )
            {
               if ( zero_points.count( lb_files->text( i ) ) ||
                    no_errors  .count( lb_files->text( i ) ) )
               {
                  lb_files->setSelected( i, false );
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

   progress->setProgress( unified_ggaussian_curves * 1.1, unified_ggaussian_curves * 1.2 );
   qApp->processEvents();
   if ( !ggauss_recompute() )
   {
      progress->reset();
      return false;
   }

   //    US_Vector::printvector( "unified q:", unified_ggaussian_q );
   //    US_Vector::printvector( "unified t:", unified_ggaussian_t );
   //    US_Vector::printvector( "unified I:", unified_ggaussian_I );
   // US_Vector::printvector( "unified params:", unified_ggaussian_params );
   // US_Vector::printvector( "unified param index:", unified_ggaussian_param_index );

   unified_ggaussian_ok = true;
   progress->setProgress( 1, 1 );
   return true;
}

bool US_Hydrodyn_Saxs_Hplc::opt_repeak_gaussians( QString file )
{
   if ( !gaussians.size() )
   {
      return false;
   } 
      
   double peak;
   if ( !get_peak( file, peak ) )
   {
      return false;
   }
   
   double gmax = compute_gaussian_peak( file, gaussians );
   
   double scale = peak / gmax;   

   if ( scale < .5 || scale > 1.5 )
   {
      switch ( QMessageBox::warning(this, 
                                    caption(),
                                    QString( tr( "Please note:\n\n"
                                                 "The current Gaussians should be scaled by %1 to be in the range of this curve.\n"
                                                 "What would you like to do?\n" ) ).arg( scale ),
                                    tr( "&Rescale the Gaussian amplitudes" ), 
                                    tr( "&Do not rescale" ),
                                    QString::null,
                                    0, // Stop == button 0
                                    0 // Escape == button 0
                                    ) )
      {
      case 0 : // rescale the Gaussians
         for ( unsigned int i = 0; i < ( unsigned int ) gaussians.size(); i += 3 )
         {
            gaussians[ 0 + i ] *= scale;
         }
         return true;
         break;
      case 1 : // just ignore them
         return false;
         break;
      }
   }
      
   return false;
}

vector < double > US_Hydrodyn_Saxs_Hplc::conc_curve( vector < double > &t,
                                                     unsigned int peak,
                                                     double conv
                                                     )
{
   vector < double > result;
   QString conc_file = lbl_conc_file->text();
   if ( conc_file.isEmpty() )
   {
      editor_msg( "red", tr( "Internal error: conc_curve(): no concentration file set" ) );
      return result;
   } else {
      if ( !f_gaussians.count( conc_file ) )
      {
         editor_msg( "red", tr( "Internal error: conc_curve(): no Gaussians defined for concentration file" ) );
         return result;
      }
   }

   if ( peak >= ( unsigned int ) f_gaussians[ conc_file ].size() / 3 )
   {
      editor_msg( "red", QString( tr( "Internal error: conc_curve(): Gaussian requested (%1) exceedes available (%2)" ) )
                  .arg( peak + 1 )
                  .arg( f_gaussians[ conc_file ].size() / 3 ) );
      return result;
   }

   if ( !detector_uv && !detector_ri )
   {
      editor_msg( "red", tr( "Internal error: conc_curve(): No detector type set" ) );
      return result;
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
   // US_Vector::printvector( QString( "conc_curve peak %1 conv %2 detector_conv %3" ).arg( peak + 1 ).arg( conv ).arg( detector_conv ), t );

   vector < double > tmp_g(3);
   tmp_g[ 0 ] = f_gaussians[ conc_file ][ 0 + peak * 3 ];
   tmp_g[ 1 ] = f_gaussians[ conc_file ][ 1 + peak * 3 ];
   tmp_g[ 2 ] = f_gaussians[ conc_file ][ 2 + peak * 3 ];

   result = compute_gaussian( t, tmp_g );
   // US_Vector::printvector( "conc curve gaussians", tmp_g );
   // US_Vector::printvector( QString( "conc_curve gaussian before conversion" ), result );

   for ( unsigned int i = 0; i < ( unsigned int ) result.size(); i++ )
   {
      result[ i ] *= detector_conv / conv;
   }

   return result;
}

bool US_Hydrodyn_Saxs_Hplc::adjacent_ok( QString name )
{
   if ( name.contains( "_bsub_a" ) ||
        name.contains( QRegExp( "\\d+$" ) ) )
   {

      return true;
   }
   return false;
}

void US_Hydrodyn_Saxs_Hplc::adjacent()
{
   QString match_name;
   int     match_pos = 0;
   QStringList turn_on;

   disable_all();

   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      if ( lb_files->isSelected( i ) )
      {
         match_name = lb_files->text( i );
         turn_on << match_name;
         match_pos = i;
         break;
      }
   }

   QRegExp rx;

   bool found = false;
   // if we have bsub
   if ( match_name.contains( "_bsub_a" ) )
   {
      found = true;
      rx.setPattern(
                    QString( "^%1" )
                    .arg( match_name )
                    .replace( QRegExp( "_bsub_a.*$" ), "" )
                    .replace( QRegExp( "\\d+$" ), "\\d+" )
                    + 
                    QString( "%1$" )
                    .arg( match_name )
                    .replace( QRegExp( "^.*_bsub" ), "_bsub" ) 
                    );
   }

   if ( !found && match_name.contains( QRegExp( "_cn\\d+.*$" ) ) )
   {
      found = true;
      rx.setPattern(
                    QString( "^%1" )
                    .arg( match_name )
                    .replace( QRegExp( "_cn\\d+.*$" ), "" )
                    );
   }

   if ( !found && match_name.contains( QRegExp( "\\d+$" ) ) )
   {
      found = true;
      rx.setPattern(
                    QString( "^%1" )
                    .arg( match_name )
                    .replace( QRegExp( "\\d+$" ), "" ) 
                    );
   }

   // cout << "rx: " << rx.pattern() << endl;

   unsigned int newly_set = 0;

   if ( found )
   {
      disable_updates = true;
      
      for ( int i = match_pos - 1; i >= 0; i-- )
      {
         if ( lb_files->text( i ).contains( rx ) )
         {
            if ( !lb_files->isSelected( i ) )
            {
               lb_files->setSelected( i, true );
               newly_set++;
            }
         }
      }
      
      for ( int i = match_pos + 1; i < lb_files->numRows(); i++ )
      {
         if ( lb_files->text( i ).contains( rx ) )
         {
            if ( !lb_files->isSelected( i ) )
            {
               lb_files->setSelected( i, true );
               newly_set++;
            }
         }
      }
      
      if ( !newly_set )
      {
         adjacent_select( lb_files, match_name );
         return;
      }
      disable_updates = false;
      update_files();
   } else {
      adjacent_select( lb_files, match_name );
      return;
   }      
   update_enables();
}

void US_Hydrodyn_Saxs_Hplc::adjacent_created()
{
   QString match_name;
   int     match_pos = 0;
   QStringList turn_on;

   disable_all();

   for ( int i = 0; i < lb_created_files->numRows(); i++ )
   {
      if ( lb_created_files->isSelected( i ) )
      {
         match_name = lb_created_files->text( i );
         turn_on << match_name;
         match_pos = i;
         break;
      }
   }

   QRegExp rx;

   bool found = false;
   // if we have bsub
   if ( match_name.contains( "_bsub_a" ) )
   {
      found = true;
      rx.setPattern(
                    QString( "^%1" )
                    .arg( match_name )
                    .replace( QRegExp( "_bsub_a.*$" ), "" )
                    .replace( QRegExp( "\\d+$" ), "\\d+" )
                    + 
                    QString( "%1$" )
                    .arg( match_name )
                    .replace( QRegExp( "^.*_bsub" ), "_bsub" ) 
                    );
   }

   if ( !found && match_name.contains( QRegExp( "_cn\\d+.*$" ) ) )
   {
      found = true;
      rx.setPattern(
                    QString( "^%1" )
                    .arg( match_name )
                    .replace( QRegExp( "_cn\\d+.*$" ), "" )
                    );
   }

   if ( !found && match_name.contains( QRegExp( "\\d+$" ) ) )
   {
      found = true;
      rx.setPattern(
                    QString( "^%1" )
                    .arg( match_name )
                    .replace( QRegExp( "\\d+$" ), "" ) 
                    );
   }

   cout << "rx: " << rx.pattern() << endl;

   unsigned int newly_set = 0;

   if ( found )
   {
      disable_updates = true;
      
      for ( int i = match_pos - 1; i >= 0; i-- )
      {
         if ( lb_created_files->text( i ).contains( rx ) )
         {
            if ( !lb_created_files->isSelected( i ) )
            {
               lb_created_files->setSelected( i, true );
               newly_set++;
            }
         }
      }
      
      for ( int i = match_pos + 1; i < lb_created_files->numRows(); i++ )
      {
         if ( lb_created_files->text( i ).contains( rx ) )
         {
            if ( !lb_created_files->isSelected( i ) )
            {
               lb_created_files->setSelected( i, true );
               newly_set++;
            }
         }
      }

      if ( !newly_set )
      {
         adjacent_select( lb_created_files, match_name );
         return;
      }
      disable_updates = false;
      update_files();
   } else {
      adjacent_select( lb_files, match_name );
      return;
   }      

   update_enables();
}

bool US_Hydrodyn_Saxs_Hplc::adjacent_select( QListBox *lb, QString match )
{
   bool ok;

   static QString last_match;
   if ( match.isEmpty() )
   {
      match = last_match;
   }

   match = QInputDialog::getText(
                                 caption() + tr( ": Select by pattern" ), 
                                 tr( "Regular expression search\n"
                                     "\n"
                                     "Special matches:\n"
                                     " ^ beginning of a line\n"
                                     " $ end of a line\n"
                                     " \\d any digit\n"
                                     " \\d+ one or more digits\n"
                                     " \\d* zero or more digits\n"
                                     " \\d? zero or one digit\n"
                                     " \\d{3} exactly 3 digits\n"
                                     " \\d{1,5} one true five digits\n"
                                     " \\D any non digit\n"
                                     " \\s whitespace\n"
                                     " \\S non-whitespace\n"
                                     " [X-Z] a range of characters\n"
                                     " () group\n"
                                     "e.g.:\n"
                                     " ^([A-B]\\d){2} would match anything that started with A or B followed by a digit twice,\n"
                                     " i.e. A1B2 would match\n\n"
                                     "Enter regular expression pattern:\n"                                     
                                     ), 
                                 QLineEdit::Normal,
                                 match, 
                                 &ok, 
                                 this 
                                 );

   if ( !ok )
   {
      update_enables();
      return false;
   }

   disable_updates = true;
      
   last_match = match;

   QRegExp rx( match );
   bool any_set = false;

   for ( int i = 0; i < lb->numRows(); i++ )
   {
      if ( lb->text( i ).contains( rx ) )
      {
         if ( !lb->isSelected( i ) )
         {
            lb->setSelected( i, true );
            any_set = true;
         }
      }
   }
   disable_updates = false;
   update_files();
   update_enables();
   return any_set;
}

void US_Hydrodyn_Saxs_Hplc::repeak()
{
   QStringList files = all_selected_files();
   repeak( files );
}

void US_Hydrodyn_Saxs_Hplc::repeak( QStringList files )
{
   bool ok;

   QString peak_target = QInputDialog::getItem(
                                               tr( "SOMO: HPLC repeak: enter peak target" ),
                                               tr("Select the peak target file:\n" ),
                                               files, 
                                               0, 
                                               FALSE, 
                                               &ok,
                                               this );
   if ( !ok ) {
      return;
   }

   map < QString, bool > current_files;
   for ( int i = 0; i < (int)lb_files->numRows(); i++ )
   {
      current_files[ lb_files->text( i ) ] = true;
   }

   map < QString, bool > select_files;
   select_files[ peak_target ] = true;

   double peak;
   if ( !get_peak( peak_target, peak ) )
   {
      return;
   }

   // check files for errors
   bool peak_target_has_errors = ( f_errors.count( peak_target ) && 
                                   f_errors[ peak_target ].size() == f_qs[ peak_target ].size() && 
                                   is_nonzero_vector( f_errors[ peak_target ] ) );
   bool any_without_errors = false;
   double avg_sd_mult = 0e0;
   bool match_sd = false;

   if ( peak_target_has_errors )
   {
      unsigned int pts = 0;
      for ( unsigned int i = 0; i < ( unsigned int ) f_errors[ peak_target ].size(); i++ )
      {
         if ( f_Is[ peak_target ][ i ] != 0e0 )
         {
            avg_sd_mult += fabs( f_errors[ peak_target ][ i ] / f_Is[ peak_target ][ i ] );
            pts++;
         }

      }
      avg_sd_mult /= ( double ) pts;

      unsigned int wo_errors_count = 0;
      for ( unsigned int i = 0; i < ( unsigned int ) files.size(); i++ )
      {
         if ( files[ i ] == peak_target )
         {
            continue;
         }
         any_without_errors = ( !f_errors.count( files[ i ] ) || 
                                f_errors[ files[ i ] ].size() != f_qs[ files[ i ] ].size() ||
                                !is_nonzero_vector( f_errors[ peak_target ] ) );
         wo_errors_count++;
      }

      if ( any_without_errors )
      {
         switch ( QMessageBox::question(this, 
                                        caption() + tr( ": repeak" ),
                                        QString( tr( "The target has S.D.'s but %1 of %2 file%3 to repeak do not have S.D.'s at every point\n"
                                                     "What would you like to do?\n" ) )
                                        .arg( wo_errors_count ).arg( files.size() - 1 ).arg( files.size() > 2 ? "s" : "" ),
                                        tr( "&Ignore S.D.'s" ), 
                                        tr( "Match target S.D.% pointwise" ),
                                        tr( "Set S.D.'s to 5 %" ), 
                                        0, // Stop == button 0
                                        0 // Escape == button 0
                                        ) )
         {
         case 0 : // ignore S.D.'s
            any_without_errors = false;
            break;
         case 1 : // keep avg_sd_mult
            match_sd = true;
            break;
         case 2 : // set to 5%
            avg_sd_mult = 0.05;
            break;
         }  
      }       
   }

   for ( unsigned int i = 0; i < ( unsigned int ) files.size(); i++ )
   {
      if ( files[ i ] == peak_target )
      {
         continue;
      }

      double this_peak;
      if ( !get_peak( files[ i ], this_peak ) )
      {
         return;
      }

      double scale = peak / this_peak;

      vector < double > repeak_I = f_Is[ files[ i ] ];
      for ( unsigned int j = 0; j < repeak_I.size(); j++ )
      {
         repeak_I[ j ] *= scale;
      }

      vector < double > repeak_e;
      if ( f_errors.count( files[ i ] ) )
      {
         repeak_e = f_errors[ files[ i ] ];
      }         
      for ( unsigned int j = 0; j < repeak_e.size(); j++ )
      {
         repeak_e[ j ] *= scale;
      }

      if ( any_without_errors && 
           ( repeak_e.size() != repeak_I.size() || !is_nonzero_vector( repeak_e ) ) )
      {
         if ( match_sd )
         {
            map < double, double > fracsd;
            for ( unsigned int j = 0; j < ( unsigned int ) f_qs[ peak_target ].size(); j++ )
            {
               if ( f_Is[ peak_target ][ j ] != 0e0 )
               {
                  fracsd[ floor( f_qs[ peak_target ][ j ] + 5e-1 ) ] = fabs( f_errors[ peak_target ][ j ] / f_Is[ peak_target ][ j ] );
               }
            }
            repeak_e.resize( repeak_I.size() );
            for ( unsigned int j = 0; j < ( unsigned int ) repeak_I.size(); j++ )
            {
               if ( fracsd.count( floor( f_qs[ files[ i ] ][ j ] + 5e-1 ) ) )
               {
                  repeak_e[ j ] = fabs( repeak_I[ j ] * fracsd[ floor(f_qs[ files[ i ] ][ j ] + 5e-1 ) ] );
               } else {
                  if ( f_qs[ files[ i ] ][ j ] < f_qs[ peak_target ][ 0 ] )
                  {
                     repeak_e[ j ] = fabs( repeak_I[ j ] * fracsd[ floor( f_qs[ peak_target ][ 0 ] + 5e-1 ) ] );
                  } else {
                     repeak_e[ j ] = fabs( repeak_I[ j ] * fracsd[ floor( f_qs[ peak_target ].back() + 5e-1 ) ] );
                  }
               }
            }
         } else {
            if ( avg_sd_mult != 0e0 )
            {
               repeak_e.resize( repeak_I.size() );
               for ( unsigned int j = 0; j < repeak_I.size(); j++ )
               {
                  repeak_e[ j ] = repeak_I[ j ] * avg_sd_mult;
               }
            } 
         }
      }

      int ext = 0;
      QString repeak_name = files[ i ] + QString( "-rp%1" ).arg( scale, 0, 'g', 8 ).replace( ".", "_" );
      while ( current_files.count( repeak_name ) )
      {
         repeak_name = files[ i ] + QString( "-rp%1-%2" ).arg( scale, 0, 'g', 8 ).arg( ++ext ).replace( ".", "_" );
      }

      select_files[ repeak_name ] = true;
      lb_created_files->insertItem( repeak_name );
      lb_created_files->setBottomItem( lb_created_files->numRows() - 1 );
      lb_files->insertItem( repeak_name );
      lb_files->setBottomItem( lb_files->numRows() - 1 );
      created_files_not_saved[ repeak_name ] = true;
   
      f_pos       [ repeak_name ] = f_qs.size();
      f_qs_string [ repeak_name ] = f_qs_string[ files[ i ] ];
      f_qs        [ repeak_name ] = f_qs       [ files[ i ] ];
      f_Is        [ repeak_name ] = repeak_I;
      f_errors    [ repeak_name ] = repeak_e;
      f_is_time   [ repeak_name ] = f_is_time  [ files[ i ] ];
      f_conc      [ repeak_name ] = f_conc.count( files[ i ] ) ? f_conc[ files[ i ] ] : 0e0;
      f_psv       [ repeak_name ] = f_psv .count( files[ i ] ) ? f_psv [ files[ i ] ] : 0e0;
      {
         vector < double > tmp;
         f_gaussians  [ repeak_name ] = tmp;
      }
      editor_msg( "gray", QString( "Created %1\n" ).arg( repeak_name ) );
   }

   lb_files->clearSelection();
   for ( int i = 0; i < (int)lb_files->numRows(); i++ )
   {
      if ( select_files.count( lb_files->text( i ) ) )
      {
         lb_files->setSelected( i, true );
      }
   }

   update_enables();
}

void US_Hydrodyn_Saxs_Hplc::crop_common()
{
   map < QString, bool > selected_files;

   vector < vector < double > > grids;

   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      if ( lb_files->isSelected( i ) )
      {
         QString this_file = lb_files->text( i );
         if ( f_qs.count( this_file ) &&
              f_Is.count( this_file ) &&
              f_qs[ this_file ].size() &&
              f_Is[ this_file ].size() )
         {
            selected_files[ this_file ] = true;
            grids.push_back( f_qs[ lb_files->text( i ) ] );
         }
      }
   }

   vector < double > v_union = US_Vector::vunion( grids );
   vector < double > v_int   = US_Vector::intersection( grids );

   editor_msg( "black", 
               QString( tr( "Crop common:\n"
                            "Current selected files have a maximal q-range of (%1:%2) with %3 points\n"
                            "Current selected files have a common  q-range of (%4:%5) with %6 points\n"
                            ) )
               .arg( v_union[ 0 ] )
               .arg( v_union.back() )
               .arg( v_union.size() )
               .arg( v_int[ 0 ] )
               .arg( v_int.back() )
               .arg( v_int.size() )
               );

   bool any_differences = v_union != v_int;

   if ( !any_differences )
   {
      editor_msg( "black", tr( "Crop common: no differences between selected grids" ) );
      return;
   }

   map < double, bool > map_int;
   for ( unsigned int i = 0; i < ( unsigned int )v_int.size(); i++ )
   {
      map_int[ v_int[ i ] ] = true;
   }

   crop_undo_data cud;
   cud.is_left   = false;
   cud.is_common = true;

   for ( map < QString, bool >::iterator it = selected_files.begin();
         it != selected_files.end();
         it++ )
   {
      // save undo data
      cud.f_qs_string[ it->first ] = f_qs_string[ it->first ];
      cud.f_qs       [ it->first ] = f_qs       [ it->first ];
      cud.f_Is       [ it->first ] = f_Is       [ it->first ];
      if ( f_errors.count( it->first ) &&
           f_errors[ it->first ].size() )
      {
         cud.f_errors   [ it->first ] = f_errors   [ it->first ];
      }

      vector < QString > new_q_string;
      vector < double  > new_q;
      vector < double  > new_I;
      vector < double  > new_e;

      for ( unsigned int i = 0; i < f_qs[ it->first ].size(); i++ )
      {
         if ( map_int.count( f_qs[ it->first ][ i ] ) )
         {
            new_q_string.push_back( f_qs_string[ it->first ][ i ] );
            new_q       .push_back( f_qs       [ it->first ][ i ] );
            new_I       .push_back( f_Is       [ it->first ][ i ] );

            if ( f_errors.count( it->first ) &&
                 f_errors[ it->first ].size() )
            {
               new_e       .push_back( f_errors   [ it->first ][ i ] );
            }
         }
      }

      f_qs_string[ it->first ] = new_q_string;
      f_qs       [ it->first ] = new_q;
      f_Is       [ it->first ] = new_I;
      if ( f_errors.count( it->first ) &&
           f_errors[ it->first ].size() )
      {
         f_errors[ it->first ] = new_e;
      }
      to_created( it->first );
   }
   crop_undos.push_back( cud );
   editor_msg( "blue", tr( "Crop common: done" ) );

   update_files();
}
