#include "../include/us3_defines.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_revision.h"
#include "../include/us_hydrodyn_saxs_hplc.h"
#include "../include/us_hydrodyn_saxs_hplc_ciq.h"
#include "../include/us_hydrodyn_saxs_hplc_dctr.h"
#include "../include/us_hydrodyn_saxs_hplc_p3d.h"
#include "../include/us_hydrodyn_saxs_hplc_fit.h"
#include "../include/us_hydrodyn_saxs_hplc_fit_global.h"
#include "../include/us_hydrodyn_saxs_hplc_nth.h"
#include "../include/us_hydrodyn_saxs_hplc_options.h"
#include "../include/us_hydrodyn_saxs_hplc_svd.h"
#include "../include/us_hydrodyn_saxs_hplc_movie.h"
#include "../include/us_lm.h"
// #include "../include/us_svd.h"
#ifdef QT4
#include <qwt_scale_engine.h>
#endif

// note: this program uses cout and/or cerr and this should be replaced

static std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const QString& str) { 
   return os << qPrintable(str);
}

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
      parameters[ "gaussians" ] = QString( "%1" ).arg( f_gaussians[ files[ 0 ] ].size() / gaussian_type_size );

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

   for ( unsigned int i = 0; i < (unsigned int) files.size(); i++ )
   {
      vector < vector < double > > tmp_v;
      vector < double >            tmp_sum;
      vector < double >            tmp_area;
      double                       tmp_area_sum = 0e0;

      bool any_accumulated        = false;

      for ( unsigned int j = 0; j < ( unsigned int ) f_gaussians[ files[ i ] ].size(); j += gaussian_type_size )
      {
         if ( g_to_plot.count( j / gaussian_type_size ) )
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

   printf( "opt_repeak gmax %g filepeak %g\n" , gmax , peak );

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
         for ( unsigned int i = 0; i < ( unsigned int ) gaussians.size(); i += gaussian_type_size )
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

   if ( peak >= ( unsigned int ) f_gaussians[ conc_file ].size() / gaussian_type_size )
   {
      editor_msg( "red", QString( tr( "Internal error: conc_curve(): Gaussian requested (%1) exceedes available (%2)" ) )
                  .arg( peak + 1 )
                  .arg( f_gaussians[ conc_file ].size() / gaussian_type_size ) );
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

   vector < double > tmp_g( gaussian_type_size );
   tmp_g[ 0 ] = f_gaussians[ conc_file ][ 0 + peak * gaussian_type_size ];
   tmp_g[ 1 ] = f_gaussians[ conc_file ][ 1 + peak * gaussian_type_size ];
   tmp_g[ 2 ] = f_gaussians[ conc_file ][ 2 + peak * gaussian_type_size ];
   if ( dist1_active )
   {
      tmp_g[ 3 ] = f_gaussians[ conc_file ][ 3 + peak * gaussian_type_size ];
      if ( dist2_active )
      {
         tmp_g[ 4 ] = f_gaussians[ conc_file ][ 4 + peak * gaussian_type_size ];
      }
   }

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

bool US_Hydrodyn_Saxs_Hplc::adjacent_select( Q3ListBox *lb, QString match )
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
      repeak_name.replace( ".", "_" );
      while ( current_files.count( repeak_name ) )
      {
         repeak_name = files[ i ] + QString( "-rp%1-%2" ).arg( scale, 0, 'g', 8 ).arg( ++ext ).replace( ".", "_" );
         repeak_name.replace( ".", "_" );
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
      f_I0se      [ repeak_name ] = f_I0se .count( files[ i ] ) ? f_I0se [ files[ i ] ] : 0e0;
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
               .arg( v_union.size() ? QString( "%1" ).arg( v_union[ 0 ] ) : QString( "empty" ) )
               .arg( v_union.size() ? QString( "%1" ).arg( v_union.back() ) : QString( "empty" ) )
               .arg( v_union.size() )
               .arg( v_int.size() ? QString( "%1" ).arg( v_int[ 0 ] ) : QString( "empty" ) )
               .arg( v_int.size() ? QString( "%1" ).arg( v_int.back() ) : QString( "empty" ) )
               .arg( v_int.size() )
               );

   bool any_differences = v_union != v_int;

   if ( !any_differences )
   {
      editor_msg( "black", tr( "Crop common: no differences between selected grids" ) );
      return;
   }

   if ( !v_int.size() )
   {
      editor_msg( "black", tr( "Crop common: grids have no common points" ) );
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

void US_Hydrodyn_Saxs_Hplc::to_saxs()
{
   // copy selected to saxs window
   if ( !activate_saxs_window() )
   {
      return;
   }
   update_csv_conc();
   map < QString, double > concs = current_concs();

   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      if ( lb_files->isSelected( i ) )
      {
         QString this_file = lb_files->text( i );
         if ( f_qs.count( this_file ) &&
              f_Is.count( this_file ) )
         {
            if ( f_errors.count( this_file ) &&
                 f_errors[ this_file ].size() )
            {
               saxs_window->plot_one_iqq( f_qs    [ this_file ],
                                          f_Is    [ this_file ],
                                          f_errors[ this_file ],
                                          this_file );
            } else {
               saxs_window->plot_one_iqq( f_qs    [ this_file ],
                                          f_Is    [ this_file ],
                                          this_file );
            }
            double use_conc;
            if ( concs.count( this_file ) && concs[ this_file ] != 0e0 )
            {
               use_conc = concs[ this_file ];
            } else {
               if ( f_conc.count( this_file ) && f_conc[ this_file ] != 0e0 ) 
               {
                  use_conc = f_conc[ this_file ];
               } else {
                  use_conc = ((US_Hydrodyn *)us_hydrodyn)->saxs_options.conc;
               }
            }
            saxs_window->update_conc_csv( 
                                         saxs_window->qsl_plotted_iq_names.back(), 
                                         use_conc,
                                         ( f_psv .count( this_file ) && f_psv [ this_file ] != 0e0 ) ? f_psv [ this_file ] : ((US_Hydrodyn *)us_hydrodyn)->saxs_options.psv,
                                         ( f_I0se.count( this_file ) && f_I0se[ this_file ] != 0e0 ) ? f_I0se[ this_file ] : ((US_Hydrodyn *)us_hydrodyn)->saxs_options.I0_exp
                                         );
         } else {
            editor_msg( "red", QString( tr( "Internal error: requested %1, but not found in data" ) ).arg( this_file ) );
         }
      }
   }
}

void US_Hydrodyn_Saxs_Hplc::avg( QStringList files )
{
   // create average of selected

   vector < QString > avg_qs_string;
   vector < double >  avg_qs;
   vector < double >  avg_Is;
   vector < double >  avg_Is2;

   QStringList selected_files;

   unsigned int selected_count = 0;

   update_csv_conc();
   map < QString, double > concs = current_concs();
   double avg_conc = 0e0;
   double avg_psv  = 0e0;
   double avg_I0se = 0e0;

   // copies for potential cropping:

   map < QString, vector < QString > > t_qs_string;
   map < QString, vector < double > >  t_qs;
   map < QString, vector < double > >  t_Is;
   map < QString, vector < double > >  t_errors;

   bool first = true;
   bool crop  = false;
   unsigned int min_q_len = 0;

   bool all_nonzero_errors = true;

   for ( int i = 0; i < (int)files.size(); i++ )
   {
      QString this_file = files[ i ];

      t_qs_string[ this_file ] = f_qs_string[ this_file ];
      t_qs       [ this_file ] = f_qs       [ this_file ];
      t_Is       [ this_file ] = f_Is       [ this_file ];
      if ( f_errors[ this_file ].size() )
      {
         t_errors [ this_file ] = f_errors[ this_file ];
      } else {
         all_nonzero_errors = false;
      }

      if ( first )
      {
         first = false;
         min_q_len = t_qs[ this_file ].size();
      } else {
         if ( min_q_len > t_qs[ this_file ].size() )
         {
            min_q_len = t_qs[ this_file ].size();
            crop = true;
         } else {
            if ( min_q_len != t_qs[ this_file ].size() )
            {
               crop = true;
            }
         }  
      }
   }

   if ( crop )
   {
      editor_msg( "dark red", QString( tr( "Notice: averaging requires cropping to %1 points" ) ).arg( min_q_len ) );
      for ( map < QString, vector < double > >::iterator it = t_qs.begin();
            it != t_qs.end();
            it++ )
      {
         t_qs_string[ it->first ].resize( min_q_len );
         t_qs       [ it->first ].resize( min_q_len );
         t_Is       [ it->first ].resize( min_q_len );
         if ( t_errors[ it->first ].size() )
         {
            t_errors   [ it->first ].resize( min_q_len );
         } 
      }
   } 

   if ( all_nonzero_errors )
   {
      for ( map < QString, vector < double > >::iterator it = t_qs.begin();
            it != t_qs.end();
            it++ )
      {
         if ( t_errors[ it->first ].size() )
         {
            if ( all_nonzero_errors &&
                 !is_nonzero_vector ( t_errors[ it->first ] ) )
            {
               all_nonzero_errors = false;
               break;
            }
         } else {
            all_nonzero_errors = false;
            break;
         }
      }      
   }

   first = true;

   vector < double > sum_weight;
   vector < double > sum_weight2;

   if ( all_nonzero_errors )
   {
      editor_msg( "black" ,
                  tr( "Notice: using standard deviation in mean calculation" ) );
   } else {
      editor_msg( "black" ,
                  tr( "Notice: NOT using standard deviation in mean calculation, since some sd's were zero or missing" ) );
   }

   for ( int i = 0; i < (int)files.size(); i++ )
   {
      QString this_file = files[ i ];

      selected_count++;
      selected_files << this_file;
      if ( all_nonzero_errors )
      {
         for ( int j = 0; j < (int)t_Is[ this_file ].size(); j++ )
         {
            t_Is[ this_file ][ j ] /= t_errors[ this_file ][ j ] * t_errors[ this_file ][ j ];
         }
      }         

      if ( first )
      {
         first = false;
         avg_qs_string = t_qs_string[ this_file ];
         avg_qs        = t_qs       [ this_file ];
         avg_Is        = t_Is       [ this_file ];
         avg_Is2       .resize( t_Is[ this_file ].size() );
         sum_weight    .resize( avg_qs.size() );
         sum_weight2   .resize( avg_qs.size() );
         for ( int j = 0; j < (int)t_Is[ this_file ].size(); j++ )
         {
            double weight    = all_nonzero_errors ? 1e0 / ( t_errors[ this_file ][ j ] * t_errors[ this_file ][ j ] ) : 1e0;
            sum_weight[ j ]  = weight;
            sum_weight2[ j ] = weight * weight;
            avg_Is2[ j ]     = t_Is[ this_file ][ j ] * t_Is[ this_file ][ j ];
         }
         avg_conc = 
            concs.count( this_file ) ?
            concs[ this_file ] :
            0e0;
         avg_psv  = f_psv.count( this_file ) ? f_psv[ this_file ] : 0e0;
         avg_I0se = f_I0se.count( this_file ) ? f_I0se[ this_file ] : 0e0;
      } else {
         if ( avg_qs.size() != t_qs[ this_file ].size() )
         {
            editor_msg( "red", tr( "Error: incompatible grids, the files selected do not have the same number of points" ) );
            return;
         }
         for ( int j = 0; j < (int)t_Is[ this_file ].size(); j++ )
         {
            if ( fabs( avg_qs[ j ] - t_qs[ this_file ][ j ] ) > Q_VAL_TOL )
            {
               editor_msg( "red", tr( "Error: incompatible grids, the q values differ between selected files" ) );
               cout << QString( "val1 %1 val2 %2 fabs diff %3\n" ).arg( avg_qs[ j ] ).arg(  t_qs[ this_file ][ j ]  ).arg(fabs( avg_qs[ j ] - t_qs[ this_file ][ j ] ) );
               return;
            }
            avg_Is [ j ]     += t_Is[ this_file ][ j ];
            double weight    = all_nonzero_errors ? 1e0 / ( t_errors[ this_file ][ j ] * t_errors[ this_file ][ j ] ) : 1e0;
            sum_weight[ j ]  += weight;
            sum_weight2[ j ] += weight * weight;
            avg_Is2[ j ]     += t_Is[ this_file ][ j ] * t_Is[ this_file ][ j ];
         }
         avg_conc +=
            concs.count( this_file ) ?
            concs[ this_file ] :
            0e0;
         avg_psv  += f_psv.count( this_file ) ? f_psv[ this_file ] : 0e0;
         avg_I0se += f_I0se.count( this_file ) ? f_I0se[ this_file ] : 0e0;
      }            
   }

   if ( selected_count < 2 )
   {
      editor_msg( "red", tr( "Error: not at least 2 files selected so there is nothing to average" ) );
      return;
   }      

   vector < double > avg_sd( avg_qs.size() );
   for ( int i = 0; i < (int)avg_qs.size(); i++ )
   {
      avg_Is[ i ] /= sum_weight[ i ];

      double sum = 0e0;
      for ( int j = 0; j < (int)files.size(); j++ )
      {
         QString this_file = files[ j ];
         double weight  = all_nonzero_errors ? 1e0 / ( t_errors[ this_file ][ i ] * t_errors[ this_file ][ i ] ) : 1e0;
         double invweight  = all_nonzero_errors ? ( t_errors[ this_file ][ i ] * t_errors[ this_file ][ i ] ) : 1e0;
         sum += weight * ( invweight * t_Is[ this_file ][ i ] - avg_Is[ i ] ) * ( invweight *  t_Is[ this_file ][ i ] - avg_Is[ i ] );
      }
      // sum *= sum_weight[ i ] / ( sum_weight[ i ] * sum_weight[ i ] - sum_weight2[ i ] );
      avg_sd[ i ] = sqrt( sum / ( ( (double) files.size() - 1e0 ) * ( sum_weight[ i ]  / (double) files.size() ) ) );
   }

   avg_conc /= files.size();
   avg_psv  /= files.size();
   avg_I0se /= files.size();

   // determine name
   // find common header & tail substrings

   QString head = qstring_common_head( selected_files, true );
   QString tail = qstring_common_tail( selected_files, true );

   int ext = 0;

   if ( !head.isEmpty() &&
        !head.contains( QRegExp( "_$" ) ) )
   {
      head += "_";
   }
   if ( !tail.isEmpty() &&
        !tail.contains( QRegExp( "^_" ) ) )
   {
      tail = "_" + tail;
   }

   QString avg_name = head + "avg" + tail;

   map < QString, bool > current_files;
   for ( int i = 0; i < (int)lb_files->numRows(); i++ )
   {
      current_files[ lb_files->text( i ) ] = true;
   }

   while ( current_files.count( avg_name ) )
   {
      avg_name = head + QString( "avg-%1" ).arg( ++ext ) + tail;
   }

   lb_created_files->insertItem( avg_name );
   lb_created_files->setBottomItem( lb_created_files->numRows() - 1 );
   lb_files->insertItem( avg_name );
   lb_files->setBottomItem( lb_files->numRows() - 1 );
   created_files_not_saved[ avg_name ] = true;
   
   f_pos       [ avg_name ] = f_qs.size();
   f_qs_string [ avg_name ] = avg_qs_string;
   f_qs        [ avg_name ] = avg_qs;
   f_Is        [ avg_name ] = avg_Is;
   f_errors    [ avg_name ] = avg_sd;
   f_is_time   [ avg_name ] = false;
   f_conc      [ avg_name ] = avg_conc;
   f_psv       [ avg_name ] = avg_psv;
   f_I0se      [ avg_name ] = avg_I0se;

   {
      vector < double > tmp;
      f_gaussians  [ avg_name ] = tmp;
   }
   
   // we could check if it has changed and then delete
   // if ( plot_dist_zoomer )
   // {
   // delete plot_dist_zoomer;
   // plot_dist_zoomer = (ScrollZoomer *) 0;
   // }
   update_csv_conc();
   for ( int i = 0; i < (int)csv_conc.data.size(); i++ )
   {
      if ( csv_conc.data[ i ].size() > 1 &&
           csv_conc.data[ i ][ 0 ] == avg_name )
      {
         csv_conc.data[ i ][ 1 ] = QString( "%1" ).arg( avg_conc );
      }
   }

   if ( conc_widget )
   {
      conc_window->refresh( csv_conc );
   }
   update_enables();
}

void US_Hydrodyn_Saxs_Hplc::conc_avg( QStringList files )
{
   // create average of selected
   vector < QString > avg_qs_string;
   vector < double >  avg_qs;
   vector < double >  avg_Is;
   vector < double >  avg_Is2;

   QStringList selected_files;

   unsigned int selected_count = 0;

   update_csv_conc();
   map < QString, double > concs = current_concs();
   map < QString, double > inv_concs;

   for ( map < QString, double >::iterator it = concs.begin();
         it != concs.end();
         it++ )
   {
      if ( it->second != 0e0 )
      {
         inv_concs[ it->first ] = 1e0 / it->second;
      }
   }

   double avg_conc = 0e0;
   double avg_psv  = 0e0;
   double avg_I0se = 0e0;

   vector < double > nIs;

   double tot_conc;
   double tot_conc2;

   // copies for potential cropping:

   map < QString, vector < QString > > t_qs_string;
   map < QString, vector < double > >  t_qs;
   map < QString, vector < double > >  t_Is;
   map < QString, vector < double > >  t_errors;

   bool first = true;
   bool crop  = false;
   unsigned int min_q_len = 0;

   bool all_nonzero_errors = true;

   for ( int i = 0; i < (int)files.size(); i++ )
   {
      QString this_file = files[ i ];
      t_qs_string[ this_file ] = f_qs_string[ this_file ];
      t_qs       [ this_file ] = f_qs       [ this_file ];
      t_Is       [ this_file ] = f_Is       [ this_file ];
      if ( f_errors[ this_file ].size() )
      {
         t_errors [ this_file ] = f_errors[ this_file ];
      } else {
         all_nonzero_errors = false;
      }

      if ( first )
      {
         first = false;
         min_q_len = t_qs[ this_file ].size();
      } else {
         if ( min_q_len > t_qs[ this_file ].size() )
         {
            min_q_len = t_qs[ this_file ].size();
            crop = true;
         } else {
            if ( min_q_len != t_qs[ this_file ].size() )
            {
               crop = true;
            }
         }  
      }
   }

   if ( crop )
   {
      editor_msg( "dark red", QString( tr( "Notice: averaging requires cropping to %1 points" ) ).arg( min_q_len ) );
      for ( map < QString, vector < double > >::iterator it = t_qs.begin();
            it != t_qs.end();
            it++ )
      {
         t_qs_string[ it->first ].resize( min_q_len );
         t_qs       [ it->first ].resize( min_q_len );
         t_Is       [ it->first ].resize( min_q_len );
         if ( t_errors[ it->first ].size() )
         {
            t_errors   [ it->first ].resize( min_q_len );
         }
      }
   }

   if ( all_nonzero_errors )
   {
      for ( map < QString, vector < double > >::iterator it = t_qs.begin();
            it != t_qs.end();
            it++ )
      {
         if ( t_errors[ it->first ].size() )
         {
            if ( all_nonzero_errors &&
                 !is_nonzero_vector ( t_errors[ it->first ] ) )
            {
               all_nonzero_errors = false;
               break;
            }
         } else {
            all_nonzero_errors = false;
            break;
         }
      }      
   }

   first = true;

   vector < double > sum_weight;
   vector < double > sum_weight2;

   if ( all_nonzero_errors )
   {
      editor_msg( "black" ,
                  tr( "Notice: using standard deviation in mean calculation" ) );
   } else {
      editor_msg( "black" ,
                  tr( "Notice: NOT using standard deviation in mean calculation, since some sd's were zero or missing" ) );
   }

   for ( int i = 0; i < (int)files.size(); i++ )
   {
      QString this_file = files[ i ];

      selected_count++;
      selected_files << this_file;

      if ( all_nonzero_errors )
      {
         for ( int j = 0; j < (int)t_Is[ this_file ].size(); j++ )
         {
            t_Is[ this_file ][ j ] /= t_errors[ this_file ][ j ] * t_errors[ this_file ][ j ];
         }
      }         

      if ( !inv_concs.count( this_file ) )
      {
         editor_msg( "red", QString( tr( "Error: found zero or no concentration for %1" ) ).arg( this_file ) );
         return;
      }
      if ( first )
      {
         first = false;
         tot_conc  = inv_concs[ this_file ];
         tot_conc2 = tot_conc * tot_conc;
         avg_qs_string = t_qs_string[ this_file ];
         avg_qs        = t_qs       [ this_file ];
         nIs           = t_Is       [ this_file ];
         sum_weight    .resize( avg_qs.size() );
         sum_weight2   .resize( avg_qs.size() );
         for ( int j = 0; j < (int)nIs.size(); j++ )
         {
            nIs[ j ] *= inv_concs[ this_file ];
         }
         
         avg_Is        = nIs;
         avg_Is2       .resize( nIs.size() );
         for ( int j = 0; j < (int)nIs.size(); j++ )
         {
            double weight    = all_nonzero_errors ? 1e0 / ( t_errors[ this_file ][ j ] * t_errors[ this_file ][ j ] ) : 1e0;
            sum_weight[ j ]  = weight;
            sum_weight2[ j ] = weight * weight;
            avg_Is2[ j ] = nIs[ j ] * nIs[ j ];
         }
         avg_conc = 
            concs.count( this_file ) ?
            concs[ this_file ] :
            0e0;
         avg_psv  = f_psv.count( this_file ) ? f_psv[ this_file ] : 0e0;
         avg_I0se = f_I0se.count( this_file ) ? f_I0se[ this_file ] : 0e0;
      } else {
         if ( avg_qs.size() != t_qs[ this_file ].size() )
         {
            editor_msg( "red", tr( "Error: incompatible grids, the files selected do not have the same number of points" ) );
            return;
         }
         tot_conc  += inv_concs[ this_file ];
         tot_conc2 += inv_concs[ this_file ] * inv_concs[ this_file ];
         nIs       = t_Is     [ this_file ];
         for ( int j = 0; j < (int)nIs.size(); j++ )
         {
            if ( fabs( avg_qs[ j ] - t_qs[ this_file ][ j ] ) > Q_VAL_TOL )
            {
               cout << QString( "val1 %1 val2 %2 fabs diff %3\n" ).arg( avg_qs[ j ] ).arg(  t_qs[ this_file ][ j ]  ).arg(fabs( avg_qs[ j ] - t_qs[ this_file ][ j ] ) );
               editor_msg( "red", tr( "Error: incompatible grids, the q values differ between selected files" ) );
               return;
            }
            nIs[ j ] *= inv_concs[ this_file ];
            avg_Is [ j ] += nIs[ j ];
            double weight    = all_nonzero_errors ? 1e0 / ( t_errors[ this_file ][ j ] * t_errors[ this_file ][ j ] ) : 1e0;
            sum_weight[ j ]  += weight;
            sum_weight2[ j ] += weight * weight;
            avg_Is2[ j ] += nIs[ j ] * nIs[ j ];
         }
         avg_conc +=
            concs.count( this_file ) ?
            concs[ this_file ] :
            0e0;
         avg_psv  += f_psv.count( this_file ) ? f_psv[ this_file ] : 0e0;
         avg_I0se += f_I0se.count( this_file ) ? f_I0se[ this_file ] : 0e0;
      }            
   }

   if ( selected_count < 2 )
   {
      editor_msg( "red", tr( "Error: not at least 2 files selected so there is nothing to average" ) );
      return;
   }      

   vector < double > avg_sd( avg_qs.size() );

   avg_conc /= files.size();
   avg_psv  /= files.size();
   avg_I0se /= files.size();

   for ( int i = 0; i < (int)avg_qs.size(); i++ )
   {
      avg_Is[ i ] /= sum_weight[ i ];

      double sum = 0e0;
      for ( int j = 0; j < (int)files.size(); j++ )
      {
         QString this_file = files[ j ];
         double weight    = all_nonzero_errors ? 1e0 / ( t_errors[ this_file ][ i ] * t_errors[ this_file ][ i ] ) : 1e0;
         double invweight = all_nonzero_errors ? ( t_errors[ this_file ][ i ] * t_errors[ this_file ][ i ] ) : 1e0;
         sum += weight 
            * ( invweight * inv_concs[ this_file ] * t_Is[ this_file ][ i ] - avg_Is[ i ] ) 
            * ( invweight * inv_concs[ this_file ] * t_Is[ this_file ][ i ] - avg_Is[ i ] );
      }
      // sum *= sum_weight[ i ] / ( sum_weight[ i ] * sum_weight[ i ] - sum_weight2[ i ] );
      avg_sd[ i ] = avg_conc * sqrt( sum / ( ( (double) files.size() - 1e0 ) * ( sum_weight[ i ]  / (double) files.size() ) ) );

      avg_Is[ i ] *= avg_conc;
   }

   // determine name
   // find common header & tail substrings

   QString head = qstring_common_head( selected_files, true );
   QString tail = qstring_common_tail( selected_files, true );

   unsigned int ext = 0;

   if ( !head.isEmpty() &&
        !head.contains( QRegExp( "_$" ) ) )
   {
      head += "_";
   }
   if ( !tail.isEmpty() &&
        !tail.contains( QRegExp( "^_" ) ) )
   {
      tail = "_" + tail;
   }

   QString avg_name = head + "cnavg" + tail;

   map < QString, bool > current_files;
   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      current_files[ lb_files->text( i ) ] = true;
   }

   while ( current_files.count( avg_name ) )
   {
      avg_name = head + QString( "cnavg-%1" ).arg( ++ext ) + tail;
   }

   lb_created_files->insertItem( avg_name );
   lb_created_files->setBottomItem( lb_created_files->numRows() - 1 );
   lb_files->insertItem( avg_name );
   lb_files->setBottomItem( lb_files->numRows() - 1 );
   created_files_not_saved[ avg_name ] = true;
   
   f_pos       [ avg_name ] = f_qs.size();
   f_qs_string [ avg_name ] = avg_qs_string;
   f_qs        [ avg_name ] = avg_qs;
   f_Is        [ avg_name ] = avg_Is;
   f_errors    [ avg_name ] = avg_sd;
   f_is_time   [ avg_name ] = false;
   f_conc      [ avg_name ] = avg_conc;
   f_psv       [ avg_name ] = avg_psv;
   f_I0se      [ avg_name ] = avg_I0se;
   {
      vector < double > tmp;
      f_gaussians  [ avg_name ] = tmp;
   }
   
   // we could check if it has changed and then delete
   // if ( plot_dist_zoomer )
   // {
   // delete plot_dist_zoomer;
   // plot_dist_zoomer = (ScrollZoomer *) 0;
   // }
   update_csv_conc();
   // cout << QString( "trying to set csv_conc to conc %1 for %2\n" ).arg( avg_conc ).arg( avg_name );
   for ( unsigned int i = 0; i < csv_conc.data.size(); i++ )
   {
      if ( csv_conc.data[ i ].size() > 1 &&
           csv_conc.data[ i ][ 0 ] == avg_name )
      {
         csv_conc.data[ i ][ 1 ] = QString( "%1" ).arg( avg_conc );
         // cout << QString( "Found & set csv_conc to conc %1 for %2\n" ).arg( avg_conc ).arg( avg_name );
      }
   }

   if ( conc_widget )
   {
      conc_window->refresh( csv_conc );
   }
   update_enables();
}

void US_Hydrodyn_Saxs_Hplc::baseline_apply()
{
   int smoothing = 0;
   bool integral = ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "hplc_bl_integral" ] == "true";
   if ( integral )
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

   double start_s = le_baseline_start_s->text().toDouble();
   double start   = le_baseline_start  ->text().toDouble();
   double start_e = le_baseline_start_e->text().toDouble();
   double end_s   = le_baseline_end_s  ->text().toDouble();
   double end     = le_baseline_end    ->text().toDouble();
   double end_e   = le_baseline_end_e  ->text().toDouble();

   // redo this to compute from best linear fit over ranges

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
         if ( f_qs[ files[ i ] ][ j ] >= start_s &&
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
      }

      for ( unsigned int j = 1; j < f_qs[ files[ i ] ].size(); j++ )
      {
         if ( f_qs[ files[ i ] ][ j ] >= start_s &&
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
               editor_msg( "red", QString( tr( "Error: smoothing error on %1" ) ).arg( files[ i ] ) );
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
               editor_msg( "dark red", QString( tr( "Warning: the integral of %1 was less than or equal to zero => constant baseline" ) ).arg( files[ i ] ) );
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

   disable_updates = true;

   lb_files->clearSelection();

   for ( int i = 0; i < (int)lb_files->numRows(); i++ )
   {
      if ( select_files.count( lb_files->text( i ) ) )
      {
         lb_files->setSelected( i, true );
      }
   }

   disable_updates = false;
   plot_files();
   update_enables();
}

void US_Hydrodyn_Saxs_Hplc::select_nth()
{
   map < QString, QString > parameters;

   parameters[ "hplc_nth_contains"   ] = 
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "hplc_nth_contains" ) ?
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "hplc_nth_contains" ] : "";

   US_Hydrodyn_Saxs_Hplc_Nth *hplc_nth = 
      new US_Hydrodyn_Saxs_Hplc_Nth(
                                   this,
                                   & parameters,
                                   this );
   US_Hydrodyn::fixWinButtons( hplc_nth );
   hplc_nth->exec();
   delete hplc_nth;

   if ( !parameters.count( "go" ) )
   {
      return;
   }

   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "hplc_nth_contains" ] =
      parameters.count( "hplc_nth_contains" ) ? parameters[ "hplc_nth_contains" ] : "";

   disable_updates = true;
   lb_files->clearSelection();
   for ( int i = 0; i < lb_files->numRows(); ++i )
   {
      if ( parameters.count( QString( "%1" ).arg( i ) ) )
      {
         lb_files->setSelected( i, true );
      }
   }
   disable_updates = false;
   plot_files();
   update_enables();
}

void US_Hydrodyn_Saxs_Hplc::update_enables()
{
   if ( running )
   {
      cout << "update_enables return (running)\n";
      return;
   }
   // cout << "update_enables\n";

   // cout << "US_Hydrodyn_Saxs_Hplc::update_enables()\n";
   // cout << QString("saxs_window->qsl_plotted_iq_names.size() %1\n").arg(saxs_window->qsl_plotted_iq_names.size());

   pb_add_files          ->setEnabled( true );
   pb_regex_load         ->setEnabled( true );
   pb_options            ->setEnabled( true );

   lb_files              ->setEnabled( true );
   lb_created_files      ->setEnabled( true );

   unsigned int files_selected_count                      = 0;
   // unsigned int non_hplc_non_empty_files_selected_count = 0;
   // unsigned int last_selected_pos                         = 0;

   map < QString, bool > selected_map;

   QStringList selected_files;

   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      if ( lb_files->isSelected( i ) )
      {
         selected_files << lb_files->text( i );
         selected_map[ lb_files->text( i ) ] = true;
         // last_selected_pos = i;
         last_selected_file = lb_files->text( i );
         files_selected_count++;
         //          if ( lb_files->text( i ) != lbl_hplc->text() &&
         //               lb_files->text( i ) != lbl_empty->text() )
         //          {
         //             non_hplc_non_empty_files_selected_count++;
         //          }
      }
   }

   bool files_compatible = compatible_files( selected_files );
   bool files_are_time   = type_files      ( selected_files );

   lbl_selected->setText( QString( tr( "%1 of %2 files selected" ) )
                          .arg( files_selected_count )
                          .arg( lb_files->numRows() ) );

   unsigned int files_created_selected_not_saved_count = 0;
   unsigned int files_created_selected_count           = 0;
   unsigned int files_created_selected_not_shown_count = 0;
   map < QString, bool > created_selected_map;

   QString last_created_selected_file;

   for ( int i = 0; i < lb_created_files->numRows(); i++ )
   {
      if ( lb_created_files->isSelected( i ) )
      {
         last_created_selected_file = lb_created_files->text( i );
         created_selected_map[ lb_created_files->text( i ) ] = true;
         files_created_selected_count++;
         if ( !selected_map.count( lb_created_files->text( i ) ) )
         {
            files_created_selected_not_shown_count++;
         } 
         if ( created_files_not_saved.count( lb_created_files->text( i ) ) )
         {
            files_created_selected_not_saved_count++;
         }
      }
   }

   lbl_selected_created->setText( QString( tr( "%1 of %2 files selected" ) )
                                  .arg( files_created_selected_count )
                                  .arg( lb_created_files->numRows() ) );

   unsigned int files_selected_not_created           = 0;
   for ( map < QString, bool >::iterator it = selected_map.begin();
         it != selected_map.end();
         it++ )
   {
      if ( !created_selected_map.count( it->first ) )
      {
         files_selected_not_created++;
      }
   }

   pb_wheel_start        ->setEnabled( files_selected_count > 0 && files_compatible && files_are_time );
   pb_gauss_start        ->setEnabled( files_selected_count == 1 && files_are_time );
   pb_ggauss_start       ->setEnabled( files_selected_count > 1 && files_are_time && gaussians.size() );
   cb_sd_weight          ->setEnabled( files_selected_count && files_are_time && gaussians.size() );
   cb_fix_width          ->setEnabled( files_selected_count && files_are_time && gaussians.size() && U_EXPT );
   cb_fix_dist1          ->setEnabled( files_selected_count && files_are_time && gaussians.size() && U_EXPT );
   cb_fix_dist2          ->setEnabled( files_selected_count && files_are_time && gaussians.size() && U_EXPT );
   pb_baseline_start     ->setEnabled( files_selected_count == 1 && files_are_time );
   pb_baseline_apply     ->setEnabled( files_selected_count && 
                                       files_are_time && 
                                       le_baseline_start->text().toDouble() < le_baseline_end->text().toDouble() );

   pb_similar_files      ->setEnabled( files_selected_count == 1 );
   pb_conc               ->setEnabled( lb_files->numRows() > 0 );
   pb_clear_files        ->setEnabled( files_selected_count > 0 );
   pb_conc_avg           ->setEnabled( all_selected_have_nonzero_conc() && files_compatible && !files_are_time );
   pb_normalize          ->setEnabled( all_selected_have_nonzero_conc() && files_compatible && !files_are_time );
   pb_add                ->setEnabled( files_selected_count > 1 && files_compatible );
   pb_avg                ->setEnabled( files_selected_count > 1 && files_compatible && !files_are_time );
   pb_smooth             ->setEnabled( files_selected_count );
   pb_repeak             ->setEnabled( files_selected_count > 1 && files_compatible && files_are_time );
   pb_svd                ->setEnabled( files_selected_count > 1 && files_compatible && !files_are_time );
   pb_create_i_of_t      ->setEnabled( files_selected_count > 1 && files_compatible && !files_are_time );
   pb_create_i_of_q      ->setEnabled( files_selected_count > 1 && files_compatible && files_are_time /* && gaussians.size() */ );
   pb_conc_file          ->setEnabled( files_selected_count == 1 );
   pb_detector           ->setEnabled( true );

   //                                        );
   //    pb_set_hplc           ->setEnabled( files_selected_count == 1 && 
   //                                        lb_files->text( last_selected_pos ) != lbl_hplc->text() &&
   //                                        lb_files->text( last_selected_pos ) != lbl_empty ->text() &&
   //                                        lb_files->text( last_selected_pos ) != lbl_signal->text()
   //                                        );
   //    pb_set_signal         ->setEnabled( files_selected_count == 1 && 
   //                                        lb_files->text( last_selected_pos ) != lbl_hplc->text() &&
   //                                        lb_files->text( last_selected_pos ) != lbl_empty ->text() &&
   //                                        lb_files->text( last_selected_pos ) != lbl_signal->text() );
   //    pb_set_empty          ->setEnabled( files_selected_count == 1 && 
   //                                        lb_files->text( last_selected_pos ) != lbl_hplc->text() &&
   //                                        lb_files->text( last_selected_pos ) != lbl_empty ->text() &&
   //                                        lb_files->text( last_selected_pos ) != lbl_signal->text() );
   pb_select_all         ->setEnabled( lb_files->numRows() > 0 );
   pb_select_nth         ->setEnabled( lb_files->numRows() > 2 );
   pb_invert             ->setEnabled( lb_files->numRows() > 0 );
   pb_line_width         ->setEnabled( files_selected_count );
   pb_color_rotate       ->setEnabled( files_selected_count );
   //    pb_join               ->setEnabled( files_selected_count == 2 && files_compatible && !files_are_time );
   // pb_adjacent           ->setEnabled( lb_files->numRows() > 1 );
   pb_to_saxs            ->setEnabled( files_selected_count && files_compatible && !files_are_time );
   pb_view               ->setEnabled( files_selected_count && files_selected_count <= 10 );
   pb_movie              ->setEnabled( files_selected_count > 1 );
   pb_rescale            ->setEnabled( files_selected_count > 0 );

   pb_select_all_created ->setEnabled( lb_created_files->numRows() > 0 );
   pb_invert_all_created ->setEnabled( lb_created_files->numRows() > 0 );
   pb_adjacent_created   ->setEnabled( lb_created_files->numRows() > 1 );
   pb_remove_created     ->setEnabled( files_created_selected_count > 0 );
   pb_save_created_csv   ->setEnabled( files_created_selected_count > 0 && files_compatible );
   pb_save_created       ->setEnabled( files_created_selected_not_saved_count > 0 );

   pb_show_created       ->setEnabled( files_created_selected_not_shown_count > 0 );
   pb_show_only_created  ->setEnabled( files_created_selected_count > 0 &&
                                       files_selected_not_created > 0 );

   pb_select_vis       ->setEnabled( 
                                    files_selected_count &&
                                    plot_dist_zoomer && 
                                    plot_dist_zoomer->zoomRect() != plot_dist_zoomer->zoomBase() 
                                    );
   pb_remove_vis       ->setEnabled( 
                                    files_selected_count &&
                                    plot_dist_zoomer && 
                                    plot_dist_zoomer->zoomRect() != plot_dist_zoomer->zoomBase() 
                                    );
   pb_crop_common      ->setEnabled( files_selected_count && files_compatible );

   pb_crop_vis         ->setEnabled( 
                                    files_selected_count &&
                                    plot_dist_zoomer && 
                                    plot_dist_zoomer->zoomRect() != plot_dist_zoomer->zoomBase()
                                    );
   pb_crop_zero         ->setEnabled( 
                                    files_selected_count
                                    );
   pb_crop_left        ->setEnabled( 
                                    files_selected_count &&
                                    plot_dist_zoomer && 
                                    plot_dist_zoomer->zoomRect() != plot_dist_zoomer->zoomBase()
                                    );
   pb_crop_undo        ->setEnabled( crop_undos.size() );
   pb_crop_right       ->setEnabled( 
                                    files_selected_count &&
                                    plot_dist_zoomer && 
                                    plot_dist_zoomer->zoomRect() != plot_dist_zoomer->zoomBase()
                                    );
   pb_legend           ->setEnabled( lb_files->numRows() && files_selected_count <= 20 );
   pb_axis_x           ->setEnabled( lb_files->numRows() );
   pb_axis_y           ->setEnabled( lb_files->numRows() );
   // cb_guinier          ->setEnabled( files_selected_count );
   legend_set();

   pb_stack_push_all   ->setEnabled( lb_files->numRows() );
   pb_stack_push_sel   ->setEnabled( files_selected_count );
   pb_stack_pcopy      ->setEnabled( files_selected_count  && clipboard.files.size() );
   pb_stack_copy       ->setEnabled( files_selected_count );
   pb_stack_paste      ->setEnabled( clipboard.files.size() );
   pb_stack_drop       ->setEnabled( stack_data.size() );
   pb_stack_join       ->setEnabled( stack_data.size() );
   pb_stack_rot_up     ->setEnabled( stack_data.size() > 1 );
   pb_stack_rot_down   ->setEnabled( stack_data.size() > 1 );
   pb_stack_swap       ->setEnabled( stack_data.size() );

   pb_ref              ->setEnabled( files_selected_count == 1 && !files_are_time && !lbl_conc_file->text().isEmpty() );
   if ( !suppress_replot && plot_ref->isVisible() && !pb_ref->isEnabled() )
   {
      plot_ref->hide();
   }

   if ( files_selected_count == 2 && files_compatible )
   {
      pb_errors           ->setEnabled( true );
   } else {
      pb_errors           ->setEnabled( false );
      hide_widgets( plot_errors_widgets, true );
   }

   pb_save_state       ->setEnabled( ( lb_files->numRows() || stack_data.size() ) && !files_created_selected_not_saved_count );

   pb_p3d              ->setEnabled( files_selected_count > 1 && files_compatible && files_are_time );

   {
      QString title;
      if ( !files_compatible )
      {
         title = tr( "q [1/Angstrom] or Time [a.u.]" );
      } else {
         if ( type_files( selected_files ) )
         {
            title = tr( "Time [a.u.]" );
         } else {
            title = tr( "q [1/Angstrom]" );
         }
      }
      if ( axis_x_log )
      {
         plot_dist->setAxisTitle(QwtPlot::xBottom,  title + tr(" (log scale)") );
      } else {
         plot_dist->setAxisTitle(QwtPlot::xBottom,  title );
      }
   }
   {
      QString title;
      if ( !files_compatible )
      {
         title = tr( "Intensity [a.u.]" );
      } else {
         if ( type_files( selected_files ) )
         {
            title = tr( "I(t) [a.u.]" );
         } else {
            title = tr( "I(q) [a.u.]" );
         }
      }

      if ( axis_y_log )
      {
         plot_dist->setAxisTitle(QwtPlot::yLeft, title + tr( " (log scale)") );
      } else {
         plot_dist->setAxisTitle(QwtPlot::yLeft, title );
      }
   }
}

void US_Hydrodyn_Saxs_Hplc::axis_y()
{
   QStringList selected_files;
   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      if ( lb_files->isSelected( i ) )
      {
         selected_files << lb_files->text( i );
      }
   }

   bool files_compatible = compatible_files( selected_files );
   QString title;
   if ( !files_compatible )
   {
      title = tr( "Intensity [a.u.]" );
   } else {
      if ( type_files( selected_files ) )
      {
         title = tr( "I(t) [a.u.]" );
      } else {
         title = tr( "I(q) [a.u.]" );
      }
   }

   axis_y_log = !axis_y_log;

   if ( axis_y_log )
   {
      plot_dist->setAxisTitle(QwtPlot::yLeft, title + tr( " (log scale)") );
#ifndef QT4
      plot_dist->setAxisOptions(QwtPlot::yLeft, QwtAutoScale::Logarithmic);
#else
      plot_dist->setAxisScaleEngine(QwtPlot::yLeft, new QwtLog10ScaleEngine);
#endif
   } else {
      plot_dist->setAxisTitle(QwtPlot::yLeft, title );
#ifndef QT4
      plot_dist->setAxisOptions(QwtPlot::yLeft, QwtAutoScale::None);
#else
      // actually need to test this, not sure what the correct version is
      plot_dist->setAxisScaleEngine(QwtPlot::yLeft, new QwtLinearScaleEngine );
#endif
   }
   if ( plot_dist_zoomer )
   {
      plot_dist_zoomer->zoom ( 0 );
      delete plot_dist_zoomer;
      plot_dist_zoomer = (ScrollZoomer *) 0;
   }
   plot_files();
   if ( !suppress_replot )
   {
      plot_dist->replot();
   }
}

void US_Hydrodyn_Saxs_Hplc::axis_x()
{
   QStringList selected_files;
   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      if ( lb_files->isSelected( i ) )
      {
         selected_files << lb_files->text( i );
      }
   }

   bool files_compatible = compatible_files( selected_files );
   QString title;
   if ( !files_compatible )
   {
      title = tr( "q [1/Angstrom] or Time [a.u.]" );
   } else {
      if ( type_files( selected_files ) )
      {
         title = tr( "Time [a.u.]" );
      } else {
         title = tr( "q [1/Angstrom]" );
      }
   }

   axis_x_log = !axis_x_log;
   if ( axis_x_log )
   {
      plot_dist->setAxisTitle(QwtPlot::xBottom,  title + tr(" (log scale)") );
#ifndef QT4
      plot_dist->setAxisOptions(QwtPlot::xBottom, QwtAutoScale::Logarithmic);
#else
      plot_dist->setAxisScaleEngine(QwtPlot::xBottom, new QwtLog10ScaleEngine);
#endif
   } else {
      plot_dist->setAxisTitle(QwtPlot::xBottom,  title );
#ifndef QT4
      plot_dist->setAxisOptions(QwtPlot::xBottom, QwtAutoScale::None);
#else
      // actually need to test this, not sure what the correct version is
      plot_dist->setAxisScaleEngine(QwtPlot::xBottom, new QwtLinearScaleEngine );
#endif
   }
   if ( !suppress_replot )
   {
      plot_dist->replot();
   }
}

void US_Hydrodyn_Saxs_Hplc::options()
{
   map < QString, QString > parameters;

   if ( U_EXPT )
   {
      parameters[ "expert_mode" ] = "true";
   }
   
   parameters[ "hplc_bl_linear"   ] = ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "hplc_bl_linear"   ];
   parameters[ "hplc_bl_integral" ] = ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "hplc_bl_integral" ];
   parameters[ "hplc_bl_save"     ] = ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "hplc_bl_save"     ];
   parameters[ "hplc_bl_smooth"   ] = ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "hplc_bl_smooth"   ];
   parameters[ "hplc_bl_reps"     ] = ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "hplc_bl_reps"     ];
   parameters[ "hplc_bl_alpha"    ] = ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "hplc_bl_alpha"    ];

   parameters[ "hplc_csv_transposed" ] = 
      (( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "hplc_csv_transposed" ) ?
      (( US_Hydrodyn * ) us_hydrodyn )->gparams[ "hplc_csv_transposed" ] : "false";
   
   parameters[ "gaussian_type" ] = QString( "%1" ).arg( gaussian_type );
   US_Hydrodyn_Saxs_Hplc_Options *sho = 
      new US_Hydrodyn_Saxs_Hplc_Options( & parameters, this );
   US_Hydrodyn::fixWinButtons( sho );
   sho->exec();
   delete sho;

   if ( !parameters.count( "ok" ) )
   {
      return;
   }

   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "hplc_bl_linear"   ] = parameters[ "hplc_bl_linear"   ];
   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "hplc_bl_integral" ] = parameters[ "hplc_bl_integral" ];
   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "hplc_bl_save"     ] = parameters[ "hplc_bl_save"     ];
   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "hplc_bl_smooth"   ] = parameters[ "hplc_bl_smooth"   ];
   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "hplc_bl_reps"     ] = parameters[ "hplc_bl_reps"     ];
   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "hplc_bl_alpha"    ] = parameters[ "hplc_bl_alpha"    ];

   // maybe ask (warn) here if gaussian data structures have data

   if ( gaussian_type != (gaussian_types)( parameters[ "gaussian_type" ].toInt() ) )
   {
      gaussian_type = (gaussian_types)( parameters[ "gaussian_type" ].toInt() );
      unified_ggaussian_ok = false;
      f_gaussians.clear();
      gaussians.clear();
      org_gaussians.clear();
      org_f_gaussians.clear();
      update_gauss_mode();
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "hplc_gaussian_type" ] = QString( "%1" ).arg( gaussian_type );
   }

   (( US_Hydrodyn * ) us_hydrodyn )->gparams[ "hplc_csv_transposed" ] = parameters[ "hplc_csv_transposed" ];

   pb_save_created_csv->setText( (( US_Hydrodyn * ) us_hydrodyn )->gparams[ "hplc_csv_transposed" ] == "true" ?
                                 tr( "Save CSV Tr" ) : tr( " Save CSV " ) );

   if ( gaussian_type != (gaussian_types)( parameters[ "hplc_csv_transposed" ].toInt() ) )
   {
      gaussian_type = (gaussian_types)( parameters[ "gaussian_type" ].toInt() );
      unified_ggaussian_ok = false;
      f_gaussians.clear();
      gaussians.clear();
      org_gaussians.clear();
      org_f_gaussians.clear();
      update_gauss_mode();
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "hplc_gaussian_type" ] = QString( "%1" ).arg( gaussian_type );
   }
   update_enables();
}

void US_Hydrodyn_Saxs_Hplc::hide_files()
{
   if ( U_EXPT )
   {
      hide_widgets( files_expert_widgets, files_widgets[ 0 ]->isVisible() );
   }
   hide_widgets( files_widgets, files_widgets[ 0 ]->isVisible() );
      
   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "hplc_files_widgets" ]         = files_widgets        [ 0 ]->isVisible() ? "visible" : "hidden";
}

void US_Hydrodyn_Saxs_Hplc::hide_created_files()
{
   if ( U_EXPT )
   {
      hide_widgets( created_files_expert_widgets, created_files_widgets[ 0 ]->isVisible() );
   }
   hide_widgets( created_files_widgets, created_files_widgets[ 0 ]->isVisible() );

   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "hplc_created_files_widgets" ] = created_files_widgets[ 0 ]->isVisible() ? "visible" : "hidden";
}

void US_Hydrodyn_Saxs_Hplc::hide_editor()
{
   hide_widgets( editor_widgets, editor_widgets[ 0 ]->isVisible() );

   if ( editor_widgets[ 0 ]->isVisible() )
   {
      lbl_editor->setPalette( PALET_LABEL );
      AUTFBACK( lbl_editor );
   }

   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "hplc_editor_widgets" ] = editor_widgets[ 0 ]->isVisible() ? "visible" : "hidden";
}

void US_Hydrodyn_Saxs_Hplc::svd()
{
   disable_all();

   vector < QString > selected_files;

   // int m = 0; // rows
   int n = 0; // cols

   {
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
               selected_files    .push_back( this_file );
               grids.push_back( f_qs[ this_file ] );
               if ( !n )
               {
                  // m = (int) f_qs[ this_file ].size();
               }
               ++n;
            }
         }
      }

      vector < double > v_union = US_Vector::vunion( grids );
      vector < double > v_int   = US_Vector::intersection( grids );

      bool any_differences = v_union != v_int;

      if ( any_differences )
      {
         editor_msg( "red", tr( "SVD: curves must be on the same grid, try 'Crop Common' first." ) );
         update_enables();
         return;
      }
   }

   update_enables();

   new US_Hydrodyn_Saxs_Hplc_Svd(
                                 this,
                                 selected_files
                                 );
}

void US_Hydrodyn_Saxs_Hplc::line_width()
{
   use_line_width++;
   if ( use_line_width > 5 )
   {
      use_line_width = 1;
   }
   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "hplc_line_width" ] = QString( "%1" ).arg( use_line_width );
   plot_files();
}

void US_Hydrodyn_Saxs_Hplc::movie()
{
   disable_all();

   US_Hydrodyn_Saxs_Hplc_Movie *shm = 
      new US_Hydrodyn_Saxs_Hplc_Movie( this );
   US_Hydrodyn::fixWinButtons( shm );
   shm->exec();
   delete shm;

   update_enables();
}

void US_Hydrodyn_Saxs_Hplc::ref()
{
   if ( plot_ref->isVisible() || lbl_conc_file->text().isEmpty() )
   {
      plot_ref->hide();
   } else {
      disable_all();
      if ( !f_qs.count( lbl_conc_file->text() ) )
      {
         plot_ref->hide();
         editor_msg( "dark red", tr( "Warning: Reference plot selected but no concentration file found" ) );
         update_enables();
         return;
      }
      QStringList files = all_selected_files();
      if ( !f_time.count( last_selected_file ) )
      {
         plot_ref->hide();
         editor_msg( "dark red", tr( "Warning: no time known for this curve" ) );
         update_enables();
         return;
      }

      if ( !plot_ref->isVisible() )
      {
         plot_ref->show();
      }
      update_ref();
      update_enables();
   }
}

void US_Hydrodyn_Saxs_Hplc::update_ref()
{
   if ( !plot_ref->isVisible() )
   {
      return;
   }
   if ( !f_time.count( last_selected_file ) )
   {
      plot_ref->hide();
      return;
   }

#ifndef QT4
   plot_ref->removeMarkers();
#else
   plot_ref->detachItems( QwtPlotItem::Rtti_PlotMarker );
#endif
   
   QColor color( "red" );

#ifndef QT4
   ref_marker = plot_ref->insertMarker();
   plot_ref->setMarkerLineStyle ( ref_marker, QwtMarker::VLine );
   plot_ref->setMarkerPos       ( ref_marker, f_time[ last_selected_file ], 0e0 );
   plot_ref->setMarkerLabelAlign( ref_marker, Qt::AlignLeft | Qt::AlignTop );
   plot_ref->setMarkerPen       ( ref_marker, QPen( color, 2, DashDotDotLine));
   plot_ref->setMarkerFont      ( ref_marker, QFont("Helvetica", 11, QFont::Bold));
   plot_ref->setMarkerLabelText ( ref_marker, QString( "%1" ).arg( f_time[ last_selected_file ] ) );
#else
   QwtPlotMarker * ref_marker = new QwtPlotMarker;
   ref_marker->setLineStyle       ( QwtPlotMarker::VLine );
   ref_marker->setLinePen         ( QPen( color, 2, Qt::DashDotDotLine ) );
   ref_marker->setLabelOrientation( Qt::Horizontal );
   ref_marker->setXValue          ( f_time[ last_selected_file ] );
   ref_marker->setLabelAlignment  ( Qt::AlignRight | Qt::AlignTop );
   {
      QwtText qwtt( QString( "%1" ).arg( f_time[ last_selected_file ] ) );
      qwtt.setFont( QFont("Helvetica", 11, QFont::Bold ) );
      ref_marker->setLabel           ( qwtt );
   }
   ref_marker->attach             ( plot_ref );
#endif
   if ( !suppress_replot )
   {
      plot_ref->replot();
   }
}

// add f_time ref everywhere (copyies etc, maybe search for extc

