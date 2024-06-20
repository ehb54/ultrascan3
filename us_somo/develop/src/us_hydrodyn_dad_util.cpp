#include "../include/us3_defines.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_revision.h"
#include "../include/us_hydrodyn_dad.h"
#include "../include/us_hydrodyn_dad_ciq.h"
#include "../include/us_hydrodyn_dad_dctr.h"
#include "../include/us_hydrodyn_dad_p3d.h"
#include "../include/us_hydrodyn_dad_fit.h"
#include "../include/us_hydrodyn_dad_fit_global.h"
#include "../include/us_hydrodyn_dad_nth.h"
#include "../include/us_hydrodyn_dad_options.h"
#include "../include/us_hydrodyn_dad_gauss_mode.h"
#include "../include/us_hydrodyn_dad_svd.h"
#include "../include/us_hydrodyn_dad_movie.h"
#include "../include/us_hydrodyn_dad_simulate.h"
#include "../include/us_lm.h"
#include "../include/us_svd.h"
#include "../include/us_plot_zoom.h"
#if QT_VERSION >= 0x040000
#include <qwt_scale_engine.h>
#endif

// note: this program uses cout and/or cerr and this should be replaced

static std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const QString& str) { 
   return os << qPrintable(str);
}

#define SLASH QDir::separator()
#define Q_VAL_TOL 5e-6
// #define UHSH_VAL_DEC 8
#define UHSH_UV_CONC_FACTOR 1e0

void US_Hydrodyn_Dad::invert_all_created()
{
   disable_all();
   set < int > was_selected;
   for ( int i = 0; i < lb_created_files->count(); i++ ) {
      if ( lb_created_files->item( i )->isSelected() ) {
         was_selected.insert( i );
      }
   }

   disable_updates = true;
   for ( int i = 0; i < lb_created_files->count(); i++ ) {
      lb_created_files->item( i)->setSelected( !was_selected.count( i ) );
   }
   disable_updates = false;
   plot_files();
   update_enables();
}

void US_Hydrodyn_Dad::remove_created()
{
   disable_all();
   QStringList files;
   for (int i = 0; i < (int) lb_created_files->count(); i++ )
   {
      if ( lb_created_files->item( i )->isSelected() )
      {
         files << lb_created_files->item( i )->text();
      }
   }
   clear_files( files );
   update_enables();
}

void US_Hydrodyn_Dad::add()
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

   QString name = us_tr( "sum_" ) + files[ 0 ];

   for ( unsigned int i = 1; i < ( unsigned int ) files.size(); i++ )
   {
      if ( f_qs[ files[ 0 ] ] != f_qs[ files[ i ] ] )
      {
         editor_msg( "red", QString( us_tr( "Error: Residuals incompatible grids (comparing %1 and %2). Suggest: Crop Common" ) ).arg( files[ 0 ] ).arg( files[ i ] ) );
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

bool US_Hydrodyn_Dad::all_have_f_gaussians( QStringList & files )
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

void US_Hydrodyn_Dad::p3d()
{
#if QT_VERSION < 0x040000
   disable_all();
   // this is for global gaussians for now

   QStringList files = all_selected_files();

   if ( !all_have_f_gaussians( files ) )
   {
      editor_msg( "red", us_tr( "Error: Not all files have Gaussians defined" ) );
      update_enables();
      return;
   }
   if ( !ggaussian_compatible( files, false ) )
   {
      editor_msg( "dark red", 
                  cb_fix_width->isChecked() ?
                  us_tr( "NOTICE: Some files selected have Gaussians with varying centers or widths or a different number of Gaussians or centers or widths that do not match the last Gaussians" )
                  :
                  us_tr( "NOTICE: Some files selected have Gaussians with varying centers or a different number of Gaussians or centers that do not match the last Gaussians." ) 
                  );
      update_enables();
      return;
   }

   for ( unsigned int i = 1; i < ( unsigned int ) files.size(); i++ )
   {
      if ( f_qs[ files[ 0 ] ] != f_qs[ files[ i ] ] )
      {
         editor_msg( "red", QString( us_tr( "Error: Incompatible grids (comparing %1 and %2). Suggest: Crop Common" ) ).arg( files[ 0 ] ).arg( files[ i ] ) );
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
         if ( rx_q.indexIn( files[ i ] ) == -1 )
         {
            editor_msg( "red", QString( us_tr( "Error: Can not find q value in file name for %1" ) ).arg( files[ i ] ) );
            update_enables();
            return;
         }
         ql.push_back( rx_q.cap( 1 ).replace( "_", "." ).toDouble() );

         if ( used_q.count( ql.back() ) )
         {
            editor_msg( "red", QString( us_tr( "Error: Duplicate q value in file name for %1" ) ).arg( files[ i ] ) );
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

   QString title = windowTitle() + ": Gaussians :";

   {
      map < QString, QString > parameters;
      parameters[ "gaussians" ] = QString( "%1" ).arg( f_gaussians[ files[ 0 ] ].size() / gaussian_type_size );

      US_Hydrodyn_Dad_P3d *dad_p3d = 
         new US_Hydrodyn_Dad_P3d(
                                       this,
                                       & parameters,
                                       this );
      US_Hydrodyn::fixWinButtons( dad_p3d );
      dad_p3d->exec();
      delete dad_p3d;

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
      editor_msg( "dark red", us_tr( "Plot 3D: no Gaussians selected to plot" ) );
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
         editor_msg( "red", us_tr( "This system has no OpenGL support." ) );
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
#endif
}

void US_Hydrodyn_Dad::set_detector()
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

      US_Hydrodyn_Dad_Dctr *dad_dctr = 
         new US_Hydrodyn_Dad_Dctr(
                                        this,
                                        & parameters,
                                        this );
      US_Hydrodyn::fixWinButtons( dad_dctr );
      dad_dctr->exec();
      delete dad_dctr;

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
         qsl << QString( us_tr( "... and %1 more not listed" ) ).arg( created_not_saved_list.size() - qsl.size() );
      }

      switch ( QMessageBox::warning(this, 
                                    us_tr( "US-SOMO: UV-Vis Remove Files" ),
                                    QString( us_tr( "Please note:\n\n"
                                                 "These files were created but not saved as .dat files:\n"
                                                 "%1\n\n"
                                                 "What would you like to do?\n" ) )
                                    .arg( qsl.join( "\n" ) ),
                                    us_tr( "&Save them now" ), 
                                    us_tr( "&Remove them anyway" ), 
                                    us_tr( "&Quit from removing files" ), 
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

bool US_Hydrodyn_Dad::height_wheel_file( double &val, double pos )
{
   if ( current_mode != MODE_GAUSSIAN )
   {
      return false;
   }
   QString errormsg;
   return US_Saxs_Util::static_apply_natural_spline( 
                                                    height_natural_spline_x,
                                                    height_natural_spline_y,
                                                    height_natural_spline_y2,
                                                    pos,
                                                    val,
                                                    errormsg );
}

void US_Hydrodyn_Dad::gauss_match_amplitude()
{
   if ( current_mode != MODE_GAUSSIAN )
   {
      return;
   }
   if ( cb_gauss_match_amplitude->isChecked() )
   {
      gauss_pos_text( le_gauss_pos->text() );
   }
}

bool US_Hydrodyn_Dad::opt_repeak_gaussians( QString file )
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
                                    windowTitle(),
                                    QString( us_tr( "Please note:\n\n"
                                                 "The current Gaussians should be scaled by %1 to be in the range of this curve.\n"
                                                 "What would you like to do?\n" ) ).arg( scale ),
                                    us_tr( "&Rescale the Gaussian amplitudes" ), 
                                    us_tr( "&Do not rescale" ),
                                    QString(),
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

vector < double > US_Hydrodyn_Dad::conc_curve( vector < double > &t,
                                                     unsigned int peak,
                                                     double conv
                                                     )
{
   vector < double > result;
   QString conc_file = lbl_conc_file->text();
   if ( conc_file.isEmpty() )
   {
      editor_msg( "red", us_tr( "Internal error: conc_curve(): no concentration file set" ) );
      return result;
   } else {
      if ( !f_gaussians.count( conc_file ) )
      {
         editor_msg( "red", us_tr( "Internal error: conc_curve(): no Gaussians defined for concentration file" ) );
         return result;
      }
   }

   if ( peak >= ( unsigned int ) f_gaussians[ conc_file ].size() / gaussian_type_size )
   {
      editor_msg( "red", QString( us_tr( "Internal error: conc_curve(): Gaussian requested (%1) exceedes available (%2)" ) )
                  .arg( peak + 1 )
                  .arg( f_gaussians[ conc_file ].size() / gaussian_type_size ) );
      return result;
   }

   if ( !detector_uv && !detector_ri )
   {
      editor_msg( "red", us_tr( "Internal error: conc_curve(): No detector type set" ) );
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

bool US_Hydrodyn_Dad::adjacent_ok( QString name )
{
   if ( name.contains( "_bsub_a" ) ||
        name.contains( QRegExp( "\\d+$" ) ) )
   {

      return true;
   }
   return false;
}

void US_Hydrodyn_Dad::adjacent()
{
   QString match_name;
   int     match_pos = 0;
   QStringList turn_on;

   disable_all();

   for ( int i = 0; i < lb_files->count(); i++ )
   {
      if ( lb_files->item( i )->isSelected() )
      {
         match_name = lb_files->item( i )->text();
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
         if ( lb_files->item( i )->text().contains( rx ) )
         {
            if ( !lb_files->item( i )->isSelected() )
            {
               lb_files->item( i)->setSelected( true );
               newly_set++;
            }
         }
      }
      
      for ( int i = match_pos + 1; i < lb_files->count(); i++ )
      {
         if ( lb_files->item( i )->text().contains( rx ) )
         {
            if ( !lb_files->item( i )->isSelected() )
            {
               lb_files->item( i)->setSelected( true );
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

void US_Hydrodyn_Dad::adjacent_created()
{
   QString match_name;
   int     match_pos = 0;
   QStringList turn_on;

   disable_all();

   for ( int i = 0; i < lb_created_files->count(); i++ )
   {
      if ( lb_created_files->item( i )->isSelected() )
      {
         match_name = lb_created_files->item( i )->text();
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
         if ( lb_created_files->item( i )->text().contains( rx ) )
         {
            if ( !lb_created_files->item( i )->isSelected() )
            {
               lb_created_files->item( i)->setSelected( true );
               newly_set++;
            }
         }
      }
      
      for ( int i = match_pos + 1; i < lb_created_files->count(); i++ )
      {
         if ( lb_created_files->item( i )->text().contains( rx ) )
         {
            if ( !lb_created_files->item( i )->isSelected() )
            {
               lb_created_files->item( i)->setSelected( true );
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
      adjacent_select( lb_created_files, match_name );
      return;
   }      

   update_enables();
}

bool US_Hydrodyn_Dad::adjacent_select( QListWidget *lb, QString match )
{
   bool ok;

   static QString last_match;
   if ( match.isEmpty() )
   {
      match = last_match;
   }

   match = US_Static::getText(
                                 windowTitle() + us_tr( ": Select by pattern" ), 
                                 us_tr( "Regular expression search\n"
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

   for ( int i = 0; i < lb->count(); i++ ) {
      if ( lb->item( i )->text().contains( rx ) ) {
         if ( !lb->item( i )->isSelected() ) {
            lb->item( i)->setSelected( true );
            any_set = true;
         }
      }
   } 

   disable_updates = false;

   update_files();
   update_enables();
   return any_set;
}

QString US_Hydrodyn_Dad::select_conc_file( QString tag )
{
   // make stringlist of set

   if ( conc_files.size() == 0 )
   {
      return "";
   }

   if ( conc_files.size() == 1 )
   {
      return *(conc_files.begin());
   }
   
   set < QString > selected = all_selected_files_set();
   unsigned int    number_of_selected_conc_files = 0;
   QString         last_selected_conc_file;

   QStringList files;
   for ( set < QString >::iterator it = conc_files.begin();
         it != conc_files.end();
         ++it )
   {
      files.push_front( *it );
      if ( selected.count( *it ) )
      {
         last_selected_conc_file = *it;
         number_of_selected_conc_files++;
      }
   }

   if ( number_of_selected_conc_files == 1 )
   {
      return last_selected_conc_file;
   }

   bool ok;

   QString file = US_Static::getItem(
                                        us_tr( "SOMO: " + tag + " : select concentration file" ),
                                        us_tr("Select the concentration file to " + tag + ":\n" ),
                                        files, 
                                        0, 
                                        false, 
                                        &ok,
                                        this );

   if ( !ok || file.isEmpty() ) {
      return "";
   }
   
   editor_msg( "dark blue", QString( us_tr( "Concentration file for %1: %2" ) ).arg( tag ).arg( file ) );
   return file;
}

void US_Hydrodyn_Dad::repeak()
{
   QStringList files = all_selected_files();
   repeak( files );
}

void US_Hydrodyn_Dad::repeak( QStringList files )
{

   QString peak_target = files.front();

   QString use_conc = select_conc_file( "Repeak" );
   if ( use_conc.isEmpty() )
   {
      update_enables();
      return;
   }

   {
      int pos = 1;
      while ( peak_target == use_conc ) {
         if ( (int) files.size() <= pos  ) {
            update_enables();
            return;
         }
         peak_target = files[ pos ];
         ++pos;
      }
   }
   
   set < QString > selected = all_selected_files_set();
   if ( !selected.count( use_conc ) )
   {
      files << use_conc;
   }

   // bool ok;

   // QString peak_target = US_Static::getItem(
   //                                             us_tr( "SOMO: HPLC repeak: enter peak target" ),
   //                                             us_tr("Select the peak target file:\n" ),
   //                                             files, 
   //                                             0, 
   //                                             false, 
   //                                             &ok,
   //                                             this );
   // if ( !ok ) {
   //    return;
   // }

   map < QString, bool > current_files;
   for ( int i = 0; i < (int)lb_files->count(); i++ )
   {
      current_files[ lb_files->item( i )->text() ] = true;
   }

   map < QString, bool > select_files;
   select_files[ peak_target ] = true;

   double peak;
   if ( !get_peak( peak_target, peak, true ) )
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
                                        windowTitle() + us_tr( ": repeak" ),
                                        // QString( us_tr( "The target has SDs but %1 of %2 file%3 to repeak do not have SDs at every point\n"
                                        //              "What would you like to do?\n" ) )
                                        // .arg( wo_errors_count ).arg( files.size() - 1 ).arg( files.size() > 2 ? "s" : "" ),
                                        QString( us_tr( "The repeak target:\n\t%1\nhas SDs, but the concentration file:\n\t%2\nto repeak does not have SDs at every point.\n\n"
                                                     "What would you like to do?\n" ) )
                                        .arg( peak_target ).arg( use_conc ),
                                        us_tr( "&Ignore SDs" ), 
                                        us_tr( "Match target SD % pointwise" ),
                                        us_tr( "Set S.D.'s to 5 %" ), 
                                        0, // Stop == button 0
                                        0 // Escape == button 0
                                        ) )
         {
         case 0 : // ignore SDs
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

   QString last_repeak_name;

   for ( unsigned int i = 0; i < ( unsigned int ) files.size(); i++ )
   {
      if ( files[ i ] == peak_target )
      {
         continue;
      }

      double this_peak;
      if ( !get_peak( files[ i ], this_peak, true ) )
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

      last_repeak_name = repeak_name;

      select_files[ repeak_name ] = true;
      lb_created_files->addItem( repeak_name );
      lb_created_files->scrollToItem( lb_created_files->item( lb_created_files->count() - 1 ) );
      lb_files->addItem( repeak_name );
      lb_files->scrollToItem( lb_files->item( lb_files->count() - 1 ) );
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
      conc_files.insert( repeak_name );
      editor_msg( "gray", QString( "Created %1\n" ).arg( repeak_name ) );
   }

   lb_files->clearSelection();
   for ( int i = 0; i < (int)lb_files->count(); i++ )
   {
      if ( select_files.count( lb_files->item( i )->text() ) )
      {
         lb_files->item( i)->setSelected( true );
      }
   }

   rescale();
   if ( QMessageBox::Yes == QMessageBox::question(
                                                  this,
                                                  windowTitle() + us_tr( ": repeak : set concentration file" ),
                                                  us_tr("Would you like to *set* the repeaked concentration file?" ),
                                                  QMessageBox::Yes, 
                                                  QMessageBox::No | QMessageBox::Default
                                                  ) )
   {
      return set_conc_file( last_repeak_name );
   }
   update_enables();
}

void US_Hydrodyn_Dad::crop_common()
{
   map < QString, bool > selected_files;

   vector < vector < double > > grids;

   for ( int i = 0; i < lb_files->count(); i++ )
   {
      if ( lb_files->item( i )->isSelected() )
      {
         QString this_file = lb_files->item( i )->text();
         if ( f_qs.count( this_file ) &&
              f_Is.count( this_file ) &&
              f_qs[ this_file ].size() &&
              f_Is[ this_file ].size() )
         {
            selected_files[ this_file ] = true;
            grids.push_back( f_qs[ lb_files->item( i )->text() ] );
         }
      }
   }

   vector < double > v_union = US_Vector::vunion( grids );
   vector < double > v_int   = US_Vector::intersection( grids );

   editor_msg( "black", 
               QString( us_tr( "Crop common:\n"
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
      editor_msg( "black", us_tr( "Crop common: no differences between selected grids" ) );
      return;
   }

   if ( !v_int.size() )
   {
      editor_msg( "black", us_tr( "Crop common: grids have no common points" ) );
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
   editor_msg( "blue", us_tr( "Crop common: done" ) );

   update_files();
}

void US_Hydrodyn_Dad::to_saxs()
{
   // copy selected to saxs window
   if ( !activate_saxs_window() )
   {
      return;
   }
   update_csv_conc();
   map < QString, double > concs = current_concs();

   for ( int i = 0; i < lb_files->count(); i++ )
   {
      if ( lb_files->item( i )->isSelected() )
      {
         QString this_file = lb_files->item( i )->text();
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
            editor_msg( "red", QString( us_tr( "Internal error: requested %1, but not found in data" ) ).arg( this_file ) );
         }
      }
   }
   saxs_window->rescale_plot();
   saxs_window->set_guinier();
}

QStringList US_Hydrodyn_Dad::get_frames( QStringList files, QString head, QString tail )
{
   QStringList result;
#if defined( DEBUG_FRAME_NAMES )
   QTextStream(stdout) <<
      QString(
              "get_frames():\n"
              "--> files: %1\n"
              "--> head : %2\n"
              "--> tail : %3\n"
              )
      .arg( files.join( "\n" ) )
      .arg( head )
      .arg( tail )
      ;
#endif
   result = files.replaceInStrings( QRegExp( "^" + QRegExp::escape( head ) ), "" ).replaceInStrings( QRegExp( QRegExp::escape( tail ) + "$" ), "" );
   // us_qdebug( QString( "get frames head %1 tail %2 result %3\n" )
   //         .arg( head )
   //         .arg( tail )
   //         .arg( result.join( "\n" ) )
   //         );
   return result;
}

void US_Hydrodyn_Dad::avg( QStringList files, QString suffix )
{
   // create average of selected

#if defined( DEBUG_FRAME_NAMES )
   QTextStream(stdout) << QString( "avg() of:\n--------\n%1\n--------\n" ).arg( files.join( "\n" ) );
#endif

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
      editor_msg( "dark red", QString( us_tr( "Notice: averaging requires cropping to %1 points" ) ).arg( min_q_len ) );
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
                  us_tr( "Notice: using standard deviation in mean calculation" ) );
   } else {
      editor_msg( "black" ,
                  us_tr( "Notice: NOT using standard deviation in mean calculation, since some sd's were zero or missing" ) );
   }

   for ( int i = 0; i < (int)files.size(); i++ )
   {
      QString this_file = files[ i ];

      selected_count++;
      selected_files << this_file;
      if ( false && all_nonzero_errors )
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
            editor_msg( "red", us_tr( "Error: incompatible grids, the files selected do not have the same number of points" ) );
            return;
         }
         for ( int j = 0; j < (int)t_Is[ this_file ].size(); j++ )
         {
            if ( fabs( avg_qs[ j ] - t_qs[ this_file ][ j ] ) > Q_VAL_TOL )
            {
               editor_msg( "red", us_tr( "Error: incompatible grids, the q values differ between selected files" ) );
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
      editor_msg( "red", us_tr( "Error: not at least 2 files selected so there is nothing to average" ) );
      return;
   }      

   vector < double > avg_sd( avg_qs.size(), 0e0 );
   for ( int i = 0; i < (int)avg_qs.size(); i++ )
   {
      // avg_Is[ i ] /= sum_weight[ i ];
      avg_Is[ i ] /= (double) selected_count;

      if ( false ) {
         // double sum = 0e0;
         double sse = 0e0;
         for ( int j = 0; j < (int) selected_count; j++ )
         {
            QString this_file = files[ j ];
            double se  = all_nonzero_errors ? ( t_errors[ this_file ][ i ] * t_errors[ this_file ][ i ] ) : 0e0;
            // double weight  = all_nonzero_errors ? 1e0 / ( t_errors[ this_file ][ i ] * t_errors[ this_file ][ i ] ) : 1e0;
            // double invweight  = all_nonzero_errors ? ( t_errors[ this_file ][ i ] * t_errors[ this_file ][ i ] ) : 1e0;
            // sum += weight * ( invweight * t_Is[ this_file ][ i ] - avg_Is[ i ] ) * ( invweight *  t_Is[ this_file ][ i ] - avg_Is[ i ] );
            sse += se;
         }
         // sum *= sum_weight[ i ] / ( sum_weight[ i ] * sum_weight[ i ] - sum_weight2[ i ] );
         // avg_sd[ i ] = sqrt( sum / ( ( (double) files.size() - 1e0 ) * ( sum_weight[ i ]  / (double) files.size() ) ) );
         avg_sd[ i ] = sqrt( sse ) / (double) selected_count;
      }
   }

   // get scaling factor for each t_Is -> avg_Is, then scale the SD's by this multiplier for each curve
   // What you can do is average the intensities WITHOUT scaling, but average the SD WITH scaling. The scaling factor for the SD being that which bring all curves to the final average one.

   if ( all_nonzero_errors ) {
      // source_I = each t_Is
      // target_I = avg_Is;

      // add_plot(  "avg_Is", avg_qs, avg_Is, true, false );

      for ( int i = 0; i < (int)selected_count; ++i ) {
         QString this_file = files[ i ];
         vector < double > org_error = t_errors[ this_file ];
         double k;
         double chi2;

         // vector < double > nnIs =  t_Is[ this_file ];
         // double min_nz = 1e-10;
         // for ( int i = 0; i < (int) nnIs.size(); ++i ) {
         //    if ( nnIs[i] > 0 && nnIs[i] < min_nz) {
         //       min_nz = nnIs[i];
         //    }
         // }
         // for ( int i = 0; i < (int) nnIs.size(); ++i ) {

         //    if ( nnIs[i] < 0 ) {
         //       nnIs[i] = min_nz;
         //    }
         // }
         usu->scaling_fit( 
                          t_Is[ this_file ],
                          avg_Is,
                          t_errors[ this_file ],
                          k,
                          chi2 );
         
         // QTextStream(stdout) << QString( "%1 scaling factor %2 chi2 %3\n" ).arg( this_file ).arg( k ).arg( chi2 );
         // {
         //    vector <double> tmp_plot = t_Is[ this_file];
         //    for ( int j = 0; j < (int)tmp_plot.size(); ++j ) {
         //       tmp_plot[ j ] *= k;
         //    }
         //    add_plot(  QString( "%1_scaled" ).arg( this_file ), avg_qs, tmp_plot, true, false );
         // }

         for ( int j = 0; j < (int)avg_qs.size(); ++j ) {
            t_errors[ this_file ][ j ] *= k;
         }

         // cout << US_Vector::qs_vector4( QString( "curve %1 scaling %2" ).arg( this_file ).arg( k ), t_Is[ this_file ], avg_Is, org_error,  t_errors[ this_file ] );
      }            

      for ( int i = 0; i < (int)avg_qs.size(); ++i ) { 
         double sse = 0e0;
         for ( int j = 0; j < (int) selected_count; ++j ) {
            QString this_file = files[ j ];
            double se  = t_errors[ this_file ][ i ] * t_errors[ this_file ][ i ];
            sse += se;
         }
         avg_sd[ i ] = sqrt( sse ) / (double) selected_count;
      }
   } else {
      // recompute errors as SD
      editor_msg( "darkblue", "computing standard deviations from the values themselves" );
      vector < vector < double > > ys;
      vector < double >            y_avg;
      vector < double >            y_sd;
      for ( auto const & I : t_Is ) {
         ys.push_back( I.second );
      }
      QString error_msg;
      if ( !US_Saxs_Util::average( ys, y_avg, y_sd, error_msg ) ) {
         editor_msg( "red", error_msg );
         return;
      }
      avg_sd = y_sd;
   }

   avg_conc /= (double) selected_count;
   avg_psv  /= (double) selected_count;
   avg_I0se /= (double) selected_count;

   // determine name
   // find common header & tail substrings

   QString head = qstring_common_head( selected_files, true );
   QString tail = qstring_common_tail( selected_files, true );
   QStringList framenames = get_frames( selected_files, head, tail );
   QString framename = QString( "f%1_%2_%3_" ).arg( framenames.size() ).arg( framenames.front() ).arg( framenames.back() );

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

   QString avg_name = head + suffix + framename + "avg" + tail;

#if defined( DEBUG_FRAME_NAMES )
   QTextStream(stdout) <<
      QString(
              "--------\n"
              "head      : '%1'\n"
              "suffix    : '%2'\n"
              "framename : '%3'\n"
              "tail      : '%4'\n"
              "avg_name  : '%5'\n"
              "--------\n"
              )
      .arg( head )
      .arg( suffix )
      .arg( framename )
      .arg( tail )
      .arg( avg_name )
      ;
#endif

   map < QString, bool > current_files;
   for ( int i = 0; i < (int)lb_files->count(); i++ )
   {
      current_files[ lb_files->item( i )->text() ] = true;
   }

   while ( current_files.count( avg_name ) )
   {
      avg_name = head + suffix + framename + QString( "avg-%1" ).arg( ++ext ) + tail;
   }

   lb_created_files->addItem( avg_name );
   lb_created_files->scrollToItem( lb_created_files->item( lb_created_files->count() - 1 ) );
   lb_files->addItem( avg_name );
   lb_files->scrollToItem( lb_files->item( lb_files->count() - 1 ) );
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
   f_header    [ avg_name ] = QString( " Frames (%1) %2" ).arg( framenames.size() ).arg( framenames.join( "," ) );
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
   {
      set < QString > to_select = { avg_name };
      set_selected( to_select );
   }
   
   update_enables();
}

void US_Hydrodyn_Dad::conc_avg( QStringList files )
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
      editor_msg( "dark red", QString( us_tr( "Notice: averaging requires cropping to %1 points" ) ).arg( min_q_len ) );
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
                  us_tr( "Notice: using standard deviation in mean calculation" ) );
   } else {
      editor_msg( "black" ,
                  us_tr( "Notice: NOT using standard deviation in mean calculation, since some sd's were zero or missing" ) );
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
         editor_msg( "red", QString( us_tr( "Error: found zero or no concentration for %1" ) ).arg( this_file ) );
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
            editor_msg( "red", us_tr( "Error: incompatible grids, the files selected do not have the same number of points" ) );
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
               editor_msg( "red", us_tr( "Error: incompatible grids, the q values differ between selected files" ) );
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
      editor_msg( "red", us_tr( "Error: not at least 2 files selected so there is nothing to average" ) );
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
   for ( int i = 0; i < lb_files->count(); i++ )
   {
      current_files[ lb_files->item( i )->text() ] = true;
   }

   while ( current_files.count( avg_name ) )
   {
      avg_name = head + QString( "cnavg-%1" ).arg( ++ext ) + tail;
   }

   lb_created_files->addItem( avg_name );
   lb_created_files->scrollToItem( lb_created_files->item( lb_created_files->count() - 1 ) );
   lb_files->addItem( avg_name );
   lb_files->scrollToItem( lb_files->item( lb_files->count() - 1 ) );
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


void US_Hydrodyn_Dad::select_nth()
{
   map < QString, QString > parameters;

   parameters[ "dad_nth_contains"   ] = 
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "dad_nth_contains" ) ?
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_nth_contains" ] : "";

   US_Hydrodyn_Dad_Nth *dad_nth = 
      new US_Hydrodyn_Dad_Nth(
                                   this,
                                   & parameters,
                                   this );
   US_Hydrodyn::fixWinButtons( dad_nth );
   dad_nth->exec();
   delete dad_nth;

   if ( !parameters.count( "go" ) )
   {
      return;
   }

   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_nth_contains" ] =
      parameters.count( "dad_nth_contains" ) ? parameters[ "dad_nth_contains" ] : "";

   disable_updates = true;
   lb_files->clearSelection();
   for ( int i = 0; i < lb_files->count(); ++i )
   {
      if ( parameters.count( QString( "%1" ).arg( i ) ) )
      {
         lb_files->item( i )->setSelected( true );
      }
   }
   disable_updates = false;
   plot_files();
   update_enables();
}

void US_Hydrodyn_Dad::axis_y( bool nochange, bool no_replot )
{

   if ( !nochange ) {
      axis_y_log = !axis_y_log;
   }

   pb_axis_y->setText( axis_y_log ? "Lin Y" : "Log Y" );

   QString title = plot_dist->axisTitle( QwtPlot::yLeft ).text();
   title = title.replace( " (log scale)", "" );

   if ( axis_y_log ) {
      plot_dist->setAxisTitle(QwtPlot::yLeft, title + us_tr( " (log scale)") );
      plot_dist->setAxisScaleEngine(QwtPlot::yLeft, new QwtLogScaleEngine(10));
   } else {
      plot_dist->setAxisTitle(QwtPlot::yLeft, title );
      // actually need to test this, not sure what the correct version is
      plot_dist->setAxisScaleEngine(QwtPlot::yLeft, new QwtLinearScaleEngine );
   }

   // if ( plot_dist_zoomer )
   // {
   //    plot_dist_zoomer->zoom ( 0 );
   //    delete plot_dist_zoomer;
   //    plot_dist_zoomer = (ScrollZoomer *) 0;
   // }

   switch( current_mode )
   {
   case MODE_PM :
      {
         plot_files();
         gauss_delete_markers();
         gauss_add_marker( le_pm_q_start  ->text().toDouble(), Qt::red, us_tr( "Start" ) );
         gauss_add_marker( le_pm_q_end    ->text().toDouble(), Qt::red, us_tr( "End"   ), Qt::AlignLeft | Qt::AlignTop );
         plot_dist->replot();
         return;
      }
      break;

   case MODE_SCALE :
      {
         gauss_delete_markers();
         gauss_add_marker( le_scale_q_start  ->text().toDouble(), Qt::red, us_tr( "Start") );
         gauss_add_marker( le_scale_q_end    ->text().toDouble(), Qt::red, us_tr( "End"  ), Qt::AlignLeft | Qt::AlignTop );
         scale_replot();
         rescale();
         plot_dist->replot();
         return;
      }
      break;

   case MODE_TESTIQ :
      {
         plot_files();
         gauss_delete_markers();
         gauss_add_marker( le_testiq_q_start  ->text().toDouble(), Qt::red, us_tr( "Start") );
         gauss_add_marker( le_testiq_q_end    ->text().toDouble(), Qt::red, us_tr( "End"  ), Qt::AlignLeft | Qt::AlignTop  );
         
         if ( cb_testiq_from_gaussian->isVisible() )
         {
            for ( int i = 0; i < (int) rb_testiq_gaussians.size(); i++ )
            {
               gauss_add_marker( unified_ggaussian_params[ (vector<double>::size_type) common_size * i ], Qt::blue, QString( "%1" ).arg( i + 1 ) );
            }
            testiq_gauss_line();
         }

         plot_dist->replot();
         return;
      }
      break;
   default : break;
   }

   plot_files();

   if ( !suppress_replot && !no_replot )
   {
      plot_dist->replot();
   }
}

void US_Hydrodyn_Dad::axis_x( bool nochange, bool no_replot )
{
   if ( !nochange ) {
      axis_x_log = !axis_x_log;
   }

   pb_axis_x->setText( axis_x_log ? "Lin X" : "Log X" );

   QString title = plot_dist->axisTitle( QwtPlot::xBottom ).text();
   title = title.replace( " (log scale)", "" );

   if ( axis_x_log )
   {
      plot_dist  ->setAxisTitle(QwtPlot::xBottom,  title + us_tr(" (log scale)") );
      plot_dist  ->setAxisScaleEngine(QwtPlot::xBottom, new QwtLogScaleEngine(10));
      plot_errors->setAxisTitle(QwtPlot::xBottom,  title + us_tr(" (log scale)") );
      plot_errors->setAxisScaleEngine(QwtPlot::xBottom, new QwtLogScaleEngine(10));
   } else {
      plot_dist  ->setAxisTitle(QwtPlot::xBottom,  title );
      plot_dist  ->setAxisScaleEngine(QwtPlot::xBottom, new QwtLinearScaleEngine );
      plot_errors->setAxisTitle(QwtPlot::xBottom,  title );
      plot_errors->setAxisScaleEngine(QwtPlot::xBottom, new QwtLinearScaleEngine );
   }
   if ( !suppress_replot && !no_replot )
   {
      plot_dist->replot();
      plot_errors->replot();
   }
}

void US_Hydrodyn_Dad::options()
{
   map < QString, QString > parameters;

   if ( U_EXPT )
   {
      parameters[ "expert_mode" ] = "true";
   }
   
   parameters[ "dad_bl_linear"             ] = ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_bl_linear"                ];
   parameters[ "dad_bl_integral"           ] = ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_bl_integral"              ];
   parameters[ "dad_bl_save"               ] = ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_bl_save"                  ];
   parameters[ "dad_bl_smooth"             ] = ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_bl_smooth"                ];
   parameters[ "dad_bl_start_region"       ] = ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_bl_start_region"          ];
   parameters[ "dad_bl_i_power"            ] = ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_bl_i_power"               ];
   parameters[ "dad_bl_reps"               ] = ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_bl_reps"                  ];
   parameters[ "dad_bl_epsilon"            ] = ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_bl_epsilon"               ];
   parameters[ "dad_cormap_maxq"           ] = ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_cormap_maxq"              ];
   parameters[ "dad_cormap_alpha"          ] = ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_cormap_alpha"             ];
   parameters[ "dad_zi_window"             ] = ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_zi_window"                ];
   parameters[ "dad_discard_it_sd_mult"    ] = ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_discard_it_sd_mult"       ];
   parameters[ "dad_cb_discard_it_sd_mult" ] = ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_cb_discard_it_sd_mult"    ];
   parameters[ "dad_guinier_qrgmax"        ] = ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_guinier_qrgmax"           ];
   parameters[ "dad_cb_guinier_qrgmax"     ] = ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_cb_guinier_qrgmax"        ];
   parameters[ "dad_gg_smooth"             ] = ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_gg_smooth"                ];
   parameters[ "dad_cb_gg_smooth"          ] = ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_cb_gg_smooth"             ];
   parameters[ "dad_cb_gg_cyclic"          ] = ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_cb_gg_cyclic"             ];
   parameters[ "dad_cb_gg_oldstyle"        ] = ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_cb_gg_oldstyle"           ];
   parameters[ "dad_dist_max"              ] = ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_dist_max"                 ];
   parameters[ "guinier_mwt_k"              ] = ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "guinier_mwt_k"                 ];
   parameters[ "guinier_mwt_c"              ] = ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "guinier_mwt_c"                 ];
   parameters[ "guinier_mwt_qmax"           ] = ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "guinier_mwt_qmax"              ];

   parameters[ "dad_csv_transposed"        ] = ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_csv_transposed" ];

   parameters[ "dad_ampl_width_min"        ] = ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_ampl_width_min"           ];
   parameters[ "dad_lock_min_retry"        ] = ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_lock_min_retry"           ];
   parameters[ "dad_lock_min_retry_mult"   ] = ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_lock_min_retry_mult"      ];
   parameters[ "dad_maxfpk_restart"        ] = ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_maxfpk_restart"           ];
   parameters[ "dad_maxfpk_restart_tries"  ] = ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_maxfpk_restart_tries"     ];
   parameters[ "dad_maxfpk_restart_pct"    ] = ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_maxfpk_restart_pct"       ];

   parameters[ "dad_makeiq_cutmax_pct"     ] = ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_makeiq_cutmax_pct"        ];
   parameters[ "dad_cb_makeiq_cutmax_pct"  ] = ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_cb_makeiq_cutmax_pct"     ];
   parameters[ "dad_cb_makeiq_avg_peaks"   ] = ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_cb_makeiq_avg_peaks"      ];
   parameters[ "dad_makeiq_avg_peaks"      ] = ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_makeiq_avg_peaks"         ];

   
   parameters[ "dad_csv_transposed" ] = 
      (( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "dad_csv_transposed" ) ?
      (( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_csv_transposed" ] : "false";
   
   parameters[ "gaussian_type" ] = QString( "%1" ).arg( gaussian_type );
   dad_options_widget = 
      new US_Hydrodyn_Dad_Options( & parameters, (US_Hydrodyn *) us_hydrodyn, this );
   US_Hydrodyn::fixWinButtons( (US_Hydrodyn_Dad_Options*) dad_options_widget );
   ((US_Hydrodyn_Dad_Options *)dad_options_widget)->exec();
   delete (US_Hydrodyn_Dad_Options *)dad_options_widget;
   dad_options_widget = 0;

   if ( !parameters.count( "ok" ) )
   {
      return;
   }

   if (
       ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_bl_linear"    ] != parameters[ "dad_bl_linear"    ] ||
       ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_bl_integral"  ] != parameters[ "dad_bl_integral"  ] ||
       ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_cormap_maxq"  ] != parameters[ "dad_cormap_maxq"  ] ||
       ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_cormap_alpha" ] != parameters[ "dad_cormap_alpha" ] 
       ) {
      baseline_ready_to_apply = false;
   }

   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_bl_linear"             ] = parameters[ "dad_bl_linear"                ];
   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_bl_integral"           ] = parameters[ "dad_bl_integral"              ];
   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_bl_save"               ] = parameters[ "dad_bl_save"                  ];
   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_bl_smooth"             ] = parameters[ "dad_bl_smooth"                ];
   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_bl_start_region"       ] = parameters[ "dad_bl_start_region"          ];
   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_bl_i_power"            ] = parameters[ "dad_bl_i_power"               ];
   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_bl_reps"               ] = parameters[ "dad_bl_reps"                  ];
   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_bl_epsilon"            ] = parameters[ "dad_bl_epsilon"               ];
   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_cormap_maxq"           ] = parameters[ "dad_cormap_maxq"              ];
   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_cormap_alpha"          ] = parameters[ "dad_cormap_alpha"             ];
   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_zi_window"             ] = parameters[ "dad_zi_window"                ];
   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_discard_it_sd_mult"    ] = parameters[ "dad_discard_it_sd_mult"       ];
   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_cb_discard_it_sd_mult" ] = parameters[ "dad_cb_discard_it_sd_mult"    ];
   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_guinier_qrgmax"        ] = parameters[ "dad_guinier_qrgmax"           ];
   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_cb_guinier_qrgmax"     ] = parameters[ "dad_cb_guinier_qrgmax"        ];
   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_gg_smooth"             ] = parameters[ "dad_gg_smooth"                ];
   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_cb_gg_smooth"          ] = parameters[ "dad_cb_gg_smooth"             ];
   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_cb_gg_cyclic"          ] = parameters[ "dad_cb_gg_cyclic"             ];
   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_cb_gg_oldstyle"        ] = parameters[ "dad_cb_gg_oldstyle"           ];
   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_dist_max"              ] = parameters[ "dad_dist_max"                 ];
   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "guinier_mwt_k"              ] = parameters[ "guinier_mwt_k"                 ];
   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "guinier_mwt_c"              ] = parameters[ "guinier_mwt_c"                 ];
   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "guinier_mwt_qmax"           ] = parameters[ "guinier_mwt_qmax"              ];
   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_csv_transposed"        ] = parameters[ "dad_csv_transposed"           ];

   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_ampl_width_min"        ] = parameters[ "dad_ampl_width_min"           ];
   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_lock_min_retry"        ] = parameters[ "dad_lock_min_retry"           ];
   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_lock_min_retry_mult"   ] = parameters[ "dad_lock_min_retry_mult"      ];
   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_maxfpk_restart"        ] = parameters[ "dad_maxfpk_restart"           ];
   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_maxfpk_restart_tries"  ] = parameters[ "dad_maxfpk_restart_tries"     ];
   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_maxfpk_restart_pct"    ] = parameters[ "dad_maxfpk_restart_pct"       ];

   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_makeiq_cutmax_pct"     ] = parameters[ "dad_makeiq_cutmax_pct"        ];
   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_cb_makeiq_cutmax_pct"  ] = parameters[ "dad_cb_makeiq_cutmax_pct"     ];
   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_cb_makeiq_avg_peaks"   ] = parameters[ "dad_cb_makeiq_avg_peaks"      ];
   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_makeiq_avg_peaks"      ] = parameters[ "dad_makeiq_avg_peaks"         ];

   // maybe ask (warn) here if gaussian data structures have data

   if ( gaussian_type != (gaussian_types)( parameters[ "gaussian_type" ].toInt() ) )
   {
      gaussian_type = (gaussian_types)( parameters[ "gaussian_type" ].toInt() );
      unified_ggaussian_ok = false;
      f_gaussians.clear( );
      gaussians.clear( );
      org_gaussians.clear( );
      org_f_gaussians.clear( );
      update_gauss_mode();
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_gaussian_type" ] = QString( "%1" ).arg( gaussian_type );
   }

   pb_save_created_csv->setText( (( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_csv_transposed" ] == "true" ?
                                 us_tr( "Save CSV Tr" ) : us_tr( " Save CSV " ) );

   cb_guinier_qrgmax->setChecked( parameters[ "dad_cb_guinier_qrgmax" ] == "true" );
   le_guinier_qrgmax->setText( parameters[ "dad_guinier_qrgmax" ] );
   
   (( US_Hydrodyn * ) us_hydrodyn )->display_default_differences();

   check_mwt_constants();

   lbl_dad_lambdas_data->setText( dad_lambdas.summary_rich() + DAD_LAMBDA_EXTC_MSG );

   update_enables();
}

void US_Hydrodyn_Dad::clear_gaussians()
{
   unified_ggaussian_ok = false;
   f_gaussians.clear( );
   gaussians.clear( );
   org_gaussians.clear( );
   org_f_gaussians.clear( );
}

bool US_Hydrodyn_Dad::any_gaussians()
{
   return
      f_gaussians.size()
      || gaussians.size()
      || org_gaussians.size()
      || org_f_gaussians.size()
      ;
}

void US_Hydrodyn_Dad::hide_files()
{
   if ( U_EXPT )
   {
      hide_widgets( files_expert_widgets, files_widgets[ 0 ]->isVisible() );
   }
   hide_widgets( files_widgets, files_widgets[ 0 ]->isVisible() );
      
   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_files_widgets" ]         = files_widgets        [ 0 ]->isVisible() ? "visible" : "hidden";
}

void US_Hydrodyn_Dad::hide_created_files()
{
   if ( U_EXPT )
   {
      hide_widgets( created_files_expert_widgets, created_files_widgets[ 0 ]->isVisible() );
   }
   hide_widgets( created_files_widgets, created_files_widgets[ 0 ]->isVisible() );

   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_created_files_widgets" ] = created_files_widgets[ 0 ]->isVisible() ? "visible" : "hidden";
}

void US_Hydrodyn_Dad::hide_editor()
{
   hide_widgets( editor_widgets, editor_widgets[ 0 ]->isVisible() );

   if ( editor_widgets[ 0 ]->isVisible() )
   {
      lbl_editor->setPalette( PALET_LABEL );
      AUTFBACK( lbl_editor );
   }

   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_editor_widgets" ] = editor_widgets[ 0 ]->isVisible() ? "visible" : "hidden";
}

void US_Hydrodyn_Dad::svd()
{
   disable_all();

   vector < QString > selected_files;

   // int m = 0; // rows
   int n = 0; // cols

   {
      vector < vector < double > > grids;

      for ( int i = 0; i < lb_files->count(); i++ )
      {
         if ( lb_files->item( i )->isSelected() )
         {
            QString this_file = lb_files->item( i )->text();
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
         editor_msg( "red", us_tr( "SVD: curves must be on the same grid, try 'Crop Common' first." ) );
         update_enables();
         return;
      }
   }

   update_enables();

   new US_Hydrodyn_Dad_Svd(
                                 this,
                                 selected_files
                                 );
}

void US_Hydrodyn_Dad::line_width()
{
   US_Plot_Zoom upz ( plot_dist, plot_dist_zoomer );
   use_line_width++;
   if ( use_line_width > 5 )
   {
      use_line_width = 1;
   }
   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_line_width" ] = QString( "%1" ).arg( use_line_width );
   if ( current_mode == MODE_GGAUSSIAN ) {
      if ( cb_ggauss_scroll->isChecked() && unified_ggaussian_ok ) {
         ggaussian_rmsd();
         ggauss_scroll_highlight( qwtw_wheel->value() );
      }
   } else {
      plot_files();
      if ( current_mode == MODE_BASELINE && !baseline_test_mode ) {
         baseline_init_markers();
         replot_baseline( "color rotate" );
      }
   }
   upz.restore( !suppress_replot );
}

void US_Hydrodyn_Dad::color_rotate()
{
   US_Plot_Zoom upz ( plot_dist, plot_dist_zoomer );
   vector < QColor >  new_plot_colors;

   for ( unsigned int i = 1; i < ( unsigned int )plot_colors.size(); i++ )
   {
      new_plot_colors.push_back( plot_colors[ i ] );
   }
   new_plot_colors.push_back( plot_colors[ 0 ] );
   plot_colors = new_plot_colors;
   if ( current_mode == MODE_GGAUSSIAN ) {
      if ( cb_ggauss_scroll->isChecked() && unified_ggaussian_ok ) {
         ggaussian_rmsd();
         ggauss_scroll_highlight( qwtw_wheel->value() );
      }
   } else {
      plot_files();
      if ( current_mode == MODE_BASELINE && !baseline_test_mode ) {
         baseline_init_markers();
         replot_baseline( "color rotate" );
      }
   }
   upz.restore( !suppress_replot );
}

void US_Hydrodyn_Dad::movie()
{
   disable_all();

   US_Hydrodyn_Dad_Movie *shm = 
      new US_Hydrodyn_Dad_Movie( this );
   US_Hydrodyn::fixWinButtons( shm );
   shm->exec();
   delete shm;

   update_enables();
}

void US_Hydrodyn_Dad::ref()
{
   if ( plot_ref->isVisible() || lbl_conc_file->text().isEmpty() )
   {
      plot_ref->hide();
   } else {
      disable_all();
      if ( !f_qs.count( lbl_conc_file->text() ) )
      {
         plot_ref->hide();
         editor_msg( "dark red", us_tr( "Warning: Reference plot selected but no concentration file found" ) );
         update_enables();
         return;
      }
      QStringList files = all_selected_files();
      if ( !f_time.count( last_selected_file ) )
      {
         plot_ref->hide();
         editor_msg( "dark red", us_tr( "Warning: no time known for this curve" ) );
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

void US_Hydrodyn_Dad::update_ref()
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

#if QT_VERSION < 0x040000
   plot_ref->removeMarkers();
#else
   plot_ref->detachItems( QwtPlotItem::Rtti_PlotMarker );
#endif
   
   QColor color( "red" );

#if QT_VERSION < 0x040000
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

double US_Hydrodyn_Dad::tot_intensity( QString &file, double q_min, double q_max )
{
   if ( !f_qs_string .count( file ) ||
        !f_qs        .count( file ) ||
        !f_Is        .count( file ) ||
        !f_pos       .count( file ) )
   {
      editor_msg( "red", QString( us_tr( "Total intensity compute internal error: no curve named %1" ) ).arg( file ) );
      return -9e99;
   }

   double tot_i = 0e0;
   for ( int i = 0; i < (int) f_qs[ file ].size(); ++i )
   {
      if ( f_qs[ file ][ i ] >= q_min &&
           f_qs[ file ][ i ] <= q_max )
      {
         tot_i += f_Is[ file ][ i ];
      }
   }
   return tot_i;
}

void US_Hydrodyn_Dad::set_selected( set < QString > & to_select, bool do_replot, bool scroll_to_first )
{
   disable_updates = true;
   lb_files->clearSelection();
   QListWidgetItem * first_item = (QListWidgetItem *) 0;
   
   for ( int i = 0; i < (int)lb_files->count(); i++ )
   {
      if ( to_select.count( lb_files->item( i )->text() ) )
      {
         lb_files->item( i )->setSelected( true );
         if ( !first_item ) {
            first_item = lb_files->item( i );
         }
      }
   }

   if ( scroll_to_first ) {
      lb_files->scrollToItem( first_item );
   }

   disable_updates = false;
   if ( do_replot )
   {
      plot_files();
   }
}

void US_Hydrodyn_Dad::set_created_selected( set < QString > & to_select, bool do_replot )
{
   disable_updates = true;
   lb_created_files->clearSelection();
   for ( int i = 0; i < (int)lb_created_files->count(); i++ )
   {
      if ( to_select.count( lb_created_files->item( i )->text() ) )
      {
         lb_created_files->item( i )->setSelected( true );
      }
   }
   disable_updates = false;
   if ( do_replot )
   {
      plot_files();
   }
}

void US_Hydrodyn_Dad::artificial_gaussians()
{
   // based upon gaussian mode, generate a bunch of I(t) with an arbitrary center, width and distortions

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
      return;
   }

   vector < double > t( 200 );
   for ( int i = 0; i < (int) t.size(); ++i )
   {
      t[ i ] = (double) i + 1;
   }

   vector < double > g( gaussian_type_size );

   bool ipe = 
      ( QMessageBox::Yes == QMessageBox::question(this, 
                                                  windowTitle() + us_tr( ": Artificial Gaussians" ),
                                                  us_tr( "Add intensity proportional errors?" ),
                                                  QMessageBox::Yes,
                                                  QMessageBox::No ) );

   g[ 1 ] = ( (double) t.size() ) / 2e0;
   g[ 2 ] = 5e0;

   if ( dist1_active )
   {
      g[ 3 ] = 25e0;
   }
   if ( dist2_active )
   {
      g[ 4 ] = -10e0;
   }

   vector < double > gs;

   for ( int i = 0; i < (int) f_Is[ wheel_file ].size(); ++i )
   {
      g[ 0 ] = f_Is[ wheel_file ][ i ];

      gs = compute_gaussian( t, g );

      vector < double > I = compute_gaussian( t, g );

      vector < double > e( I.size() );
      if ( ipe )
      {
         for ( int j = 0; j < (int) I.size(); ++j )
         {
            e[ j ] = I[ j ] * .01;
         }
      } else {
         for ( int j = 0; j < (int) I.size(); ++j )
         {
            e[ j ] = .01 * f_Is[ wheel_file ][ 0 ];
         }
      }

      add_plot( 
               QString( "%1_ag_%2_q%3" )
               .arg( wheel_file )
               .arg( QString( "%1" ).arg( gaussian_type_tag ).toLower().replace( "+", "" ) )
               .arg( f_qs[ wheel_file ][ i ] ).replace( ".", "_" ),
               t,
               I,
               e,
               true,
               false );
   }
   update_enables();
}


bool US_Hydrodyn_Dad::constant_e( QString file, double e )
{
   if ( !f_qs.count( file ) )
   {
      return false;
   }
   QString sde_name = file + "_sde";
   QString use_sde_name = sde_name;

   unsigned int ext = 0;
   
   while ( f_qs.count( use_sde_name ) )
   {
      use_sde_name = QString( "%1-%2" ).arg( use_sde_name ).arg( ++ext );
   }

   lb_created_files->addItem( use_sde_name );
   lb_created_files->scrollToItem( lb_created_files->item( lb_created_files->count() - 1 ) );
   lb_files->addItem( use_sde_name );
   lb_files->scrollToItem( lb_files->item( lb_files->count() - 1 ) );

   created_files_not_saved[ use_sde_name ] = true;
   
   f_pos       [ use_sde_name ] = f_qs.size();
   f_qs_string [ use_sde_name ] = f_qs_string[ file ];
   f_qs        [ use_sde_name ] = f_qs       [ file ];
   f_Is        [ use_sde_name ] = f_Is       [ file ];
   f_errors    [ use_sde_name ] = vector < double > ( f_qs[ file ].size(), e );

   f_is_time   [ use_sde_name ] = f_is_time  [ file ];

   f_gaussians [ use_sde_name ] = f_gaussians.count( file ) ? f_gaussians[ file ] : vector < double > (0);

   f_conc      [ use_sde_name ] = f_conc.count( file ) ? f_conc[ file ] : 0e0;
   f_psv       [ use_sde_name ] = f_psv .count( file ) ? f_psv [ file ] : 0e0;
   f_I0se      [ use_sde_name ] = f_I0se.count( file ) ? f_I0se[ file ] : 0e0;
   f_extc      [ use_sde_name ] = f_extc.count( file ) ? f_extc[ file ] : 0e0;

   editor_msg( "gray", QString( "Created %1\n" ).arg( use_sde_name ) );

   return true;
}

int US_Hydrodyn_Dad::slice( const vector < double > & q,
                                  const vector < double > & I,
                                  double start_q,
                                  double end_q, 
                                  vector < double > & new_q,
                                  vector < double > & new_I )
{
   new_q.clear( );
   new_I.clear( );
   if ( q.size() != I.size() )
   {
      return 0;
   }

   for ( int i = 0; i < (int) q.size(); ++i )
   {
      if ( q[ i ] >= start_q && q[ i ] <= end_q )
      {
         new_q.push_back( q[ i ] );
         new_I.push_back( I[ i ] );
      }
   }
   return (int) new_q.size();
}

double US_Hydrodyn_Dad::wyatt_errors( QString file, 
                                            double start_q, 
                                            double end_q )
{
   if ( !f_qs.count( file ) )
   {
      return -1e0;
   }
   return wyatt_errors( f_qs[ file ], f_Is[ file ], start_q, end_q );
}

double US_Hydrodyn_Dad::wyatt_errors( QString file, 
                                            double start_q, 
                                            double end_q,
                                            vector < double > & new_q,
                                            vector < double > & new_I,
                                            vector < double > & new_y
                                            )
{
   if ( !f_qs.count( file ) )
   {
      return -1e0;
   }
   return wyatt_errors( f_qs[ file ], f_Is[ file ], start_q, end_q, new_q, new_I, new_y );
}


double US_Hydrodyn_Dad::wyatt_errors( const vector < double > & q, 
                                            const vector < double > & I, 
                                            double start_q, 
                                            double end_q )
{
   vector < double > new_q;
   vector < double > new_I;
   vector < double > new_y;
   return wyatt_errors( q, I, start_q, end_q, new_q, new_I, new_y );
}

double US_Hydrodyn_Dad::wyatt_errors( const vector < double > & q, 
                                            const vector < double > & I, 
                                            double start_q, 
                                            double end_q,
                                            vector < double > & new_q,
                                            vector < double > & new_I,
                                            vector < double > & new_y
                                            )
{
   if ( !slice( q, I, start_q, end_q, new_q, new_I ) )
   {
      return -1e0;
   }
   // us_qdebug( US_Vector::qs_vector2( "wyatt new_q, new_I", new_q, new_I ) );

   // compute 3rd degree polynomial fit of new_q, new_I

   // solve via least squares using svd

   // for matrix M of 1, q, q^2, q^3

   int m = (int) new_q.size(); // rows
   int n = 4; // columns

   vector < double > x( 4, 1e0 );

   vector < vector < double > > M;

   for ( int i = 0; i < m; ++i )
   {
      for ( int j = 1; j < 4; ++j )
      {
         x[ j ] = x[ j - 1 ] * new_q[ i ];
      }
      M.push_back( x );
   }

   // us_qdebug( US_Vector::qs_vector_vector( "wyatt M", M ) );

   // compute SVD of M (see: us_hydrodyn_dad_svd.cpp and us_svd.cpp for usage)

   vector < double * > a( m );

   for ( int i = 0; i < m; ++i )
   {
      a[ i ] = &(M[ i ][ 0 ]);
   }

   vector < double > W( n );
   double *w = &(W[ 0 ]);
   vector < double * > v( n );

   vector < vector < double > > V( n );
   for ( int j = 0; j < n; ++j )
   {
      V[ j ].resize( n );
      v[ j ] = &(V[ j ][ 0 ]);
   }
      
   // editor_msg( "blue", us_tr( "Wyatt errors: SVD: matrix M created, computing SVD" ) );
   if ( !SVD::dsvd( &(a[ 0 ]), m, n, &(w[ 0 ]), &(v[ 0 ]) ) )
   {
      editor_msg( "red", us_tr( SVD::errormsg ) );
      update_enables();
      return -1e0;
   }

   // us_qdebug( US_Vector::qs_vector_vector( "wyatt U", M ) );
   // us_qdebug( US_Vector::qs_vector       ( "wyatt W", W ) );
   // us_qdebug( US_Vector::qs_vector_vector( "wyatt V", V ) );

   // multiply pseudoinverse of M by new_I to get 4 polynomial coefficients

   vector < double > winv( n, 0e0 );
   for ( int i = 0; i < n; ++i )
   {
      if ( w[ i ] )
      {
         winv[ i ] = 1e0 / w[ i ];
      }
   }

   // us_qdebug( US_Vector::qs_vector       ( "wyatt Winv", winv ) );

   vector < double > c0( n, 0e0 );

   // http://en.wikipedia.org/wiki/Singular_value_decomposition#Applications_of_the_SVD
   // c = v*winvT*aT*new_I


   // 1st multiply UT by new_I, and also multiply by winv

   for ( int i = 0; i < n; ++i )
   {
      for ( int j = 0; j < m; ++j )
      {
         c0[ i ] += M[ j ][ i ] * new_I[ j ];
      }
      c0[ i ] *= winv[ i ];
   }
   
   // left multiply by V transpose
   vector < double > c( n, 0e0 );

   for ( int i = 0; i < n; ++i )
   {
      for ( int j = 0; j < n; ++j )
      {
         c[ i ] += V[ i ][ j ] * c0[ j ];
      }
   }

   // add plot of cubic over new_q

   new_y.resize( m );

   for ( int i = 0; i < m; ++i )
   {
      double x = new_q[ i ];
      new_y[ i ] = c[ 0 ] + x * (c[ 1 ] + x * ( c[ 2 ] + c[ 3 ] * x ) );
   }

   // us_qdebug( US_Vector::qs_vector2       ( "c0 c", c0, c ) );
   // us_qdebug( US_Vector::qs_vector2       ( "wyatt cubic", new_q, new_y ) );
   // add_plot( "least_squares_cubic_fit", new_q, wyatt_y, true, false );

   // compute chi^2 of fit and that is the variance
   double sum2 = 0e0;
   for ( int i = 0; i < m; ++i )
   {
      if ( new_y[ i ] != 0e0 )
      {
         sum2 += ( new_y[ i ] - new_I[ i ] ) * ( new_y[ i ] - new_I[ i ] );
      }
   }
   
   return sqrt( sum2 / (double) (m-1) );
}

#include <qwt_plot_layout.h>
#include <qwt_scale_widget.h>
#include <qwt_scale_draw.h>

void US_Hydrodyn_Dad::plot_debug() {

   QTextStream tso( stdout );
   for ( map < QString, QwtPlot * >::iterator it = plot_info.begin();
         it != plot_info.end();
         ++it )
   {
      int curves   = 0;

      QwtPlotItemList ilist = it->second->itemList();
      for ( int ii = 0; ii < ilist.size(); ii++ )
      {
         QwtPlotItem* plitem = ilist[ ii ];
         if ( plitem->rtti() != QwtPlotItem::Rtti_PlotCurve ) {
            continue;
         }

         QwtPlotCurve* curve = (QwtPlotCurve*) plitem;

         // qDebug() << curve->title().text();
         
         if ( curve->dataSize() )
         {
            curves++;
            break;
         }
      }

      if ( curves ) {
         int yLeftStartDist = 0;
         int yLeftEndDist   = 0;
         int yRightStartDist = 0;
         int yRightEndDist   = 0;

         it->second->axisWidget( QwtPlot::yLeft )->getBorderDistHint( yLeftStartDist, yLeftEndDist );
         it->second->axisWidget( QwtPlot::yRight )->getBorderDistHint( yRightStartDist, yRightEndDist );

         tso << QString( "plot for %1\n" ).arg( it->first )
             << QString().sprintf(
                                  "plotLayout()->alignCanvasToScale( yLeft )          %s\n"
                                  "plotLayout()->alignCanvasToScale( yRight )         %s\n"
                                  
                                  "plotLayout()->spacing()                            %d\n"

                                  "plotLayout()->canvasRect()                         %g, %g\n"
                                  "plotLayout()->titleRect()                          %g, %g\n"
                                  "plotLayout()->footerRect()                         %g, %g\n"
                                  "plotLayout()->legendRect()                         %g, %g\n"

                                  "plotLayout()->scaleRect( yLeft )                   %g, %g\n"
                                  "plotLayout()->scaleRect( yRight )                  %g, %g\n"

                                  "plotLayout()->canvasMargin( yLeft )                %d\n"
                                  "plotLayout()->canvasMargin( yRight )               %d\n"

                                  "axisWidget( yLeft )->getBorderDistHint( yLeft )    %d, %d\n"
                                  "axisWidget( yRight )->getBorderDistHint( yRight )  %d, %d\n"

                                  "axisWidget( yLeft )->scaleDraw()->extent()         %g\n"
                                  "axisWidget( yRight )->scaleDraw()->extent()        %g\n"

                                  "width guess                                        %g\n"

                                  ,( it->second->plotLayout()->alignCanvasToScale( QwtPlot::yLeft ) ? "True" : "False" )
                                  ,( it->second->plotLayout()->alignCanvasToScale( QwtPlot::yRight ) ? "True" : "False" )

                                  ,it->second->plotLayout()->spacing()

                                  ,it->second->plotLayout()->canvasRect().x()
                                  ,it->second->plotLayout()->canvasRect().y()

                                  ,it->second->plotLayout()->titleRect().x()
                                  ,it->second->plotLayout()->titleRect().y()

                                  ,it->second->plotLayout()->footerRect().x()
                                  ,it->second->plotLayout()->footerRect().y()

                                  ,it->second->plotLayout()->legendRect().x()
                                  ,it->second->plotLayout()->legendRect().y()

                                  ,it->second->plotLayout()->scaleRect( QwtPlot::yLeft ).x()
                                  ,it->second->plotLayout()->scaleRect( QwtPlot::yLeft ).y()

                                  ,it->second->plotLayout()->scaleRect( QwtPlot::yRight ).x()
                                  ,it->second->plotLayout()->scaleRect( QwtPlot::yRight ).y()

                                  ,it->second->plotLayout()->canvasMargin( QwtPlot::yLeft )
                                  ,it->second->plotLayout()->canvasMargin( QwtPlot::yRight )

                                  ,yLeftStartDist
                                  ,yLeftEndDist

                                  ,yRightStartDist
                                  ,yRightEndDist

                                  ,it->second->axisWidget( QwtPlot::yLeft )->scaleDraw()->extent( it->second->axisWidget( QwtPlot::yLeft )->font() )
                                  ,it->second->axisWidget( QwtPlot::yRight )->scaleDraw()->extent( it->second->axisWidget( QwtPlot::yRight )->font() )
                                  , (
                                     it->second->plotLayout()->canvasMargin( QwtPlot::yLeft ) +
                                     it->second->plotLayout()->canvasMargin( QwtPlot::yRight ) +
                                     it->second->plotLayout()->scaleRect( QwtPlot::yLeft ).x() +
                                     it->second->plotLayout()->scaleRect( QwtPlot::yRight ).x() +
                                     it->second->plotLayout()->canvasRect().x()
                                     )
                                  );
      }
   }
}
   
void US_Hydrodyn_Dad::pp() 
{
      
   //   return plot_debug();

   QString use_dir = QDir::currentPath();
   ((US_Hydrodyn  *)us_hydrodyn)->select_from_directory_history( use_dir, this );
   QString fn = 
      QFileDialog::getSaveFileName( this , us_tr( "Select a prefix name to save the plot data" ) , use_dir , "*.csv" );

   if ( fn.isEmpty() )
   {
      return;
   }

   fn = QFileInfo( fn ).path() + QDir::separator() + QFileInfo( fn ).completeBaseName();

   QString errors;
   QString messages;

   if ( !US_Plot_Util::printtofile( fn, plot_info, errors, messages ) )
   {
      editor_msg( "red", errors );
   } else {
      editor_msg( "blue", messages );
      ((US_Hydrodyn *)us_hydrodyn)->add_to_directory_history( fn + ".csv" );
   }
}

bool US_Hydrodyn_Dad::ask_to_decimate( map < QString, QString > & parameters ) 
{
   switch ( QMessageBox::warning(this, 
                                 windowTitle() + us_tr( " : Optional PVP Analysis Correlation Correction" )
                                 ,us_tr( 
                                    "Warning: datasets having finely spaced q-values might exhibit cross-correlation issues.\n"
                                    "Sampling one every few q points will alleviate this problem.\n"
                                    "Would you like to?"
                                     )
                                 ,us_tr( "&Sample alternate q points" )
                                 ,us_tr( "&Specify a larger gap in q points" )
                                 ,us_tr( "&Continue" )
                                 ,2 // Default continue
                                 ,-1 // 
                                 ) )
   {
   case -1 : // escape
      return false;
      break;
   case 0 : // select alternates
      parameters[ "decimate" ] = "2";
      return ask_cormap_minq( parameters );
      break;
   case 1 : // specify alternates
      break;
   case 2 : // continue
      return ask_cormap_minq( parameters );
      break;
   }

   // arrival here here must be to specify alternates

   bool ok;
   int result = US_Static::getInteger(
                                         windowTitle() + us_tr( " : Optional PVP Analysis Correlation Correction" )
                                         ,us_tr( "Sample one q point out of every:" )
                                         ,3
                                         ,3
                                         ,10
                                         ,1 
                                         ,&ok
                                         ,this 
                                         );
   if ( !ok ) {
      return false;
   }
   parameters[ "decimate" ] = QString( "%1" ).arg( result );
   return ask_cormap_minq( parameters );
}

bool US_Hydrodyn_Dad::ask_cormap_minq( map < QString, QString > & parameters ) 
{
   bool ok;
   double result = US_Static::getDouble(
                                           windowTitle() + us_tr( " : Optional PVP Analysis q minimum cutoff" )
                                           ,us_tr( "If you have noisy low q data, you may wish to eliminate these points\n"
                                                "from PVP analysis by setting a non-zero minimum q value here:" )
                                           ,0
                                           ,0
                                           ,parameters.count( "cormap_maxq" ) ? parameters[ "cormap_maxq" ].toDouble() - 0.01 : 0.04
                                           ,4
                                           ,&ok
                                           ,this 
                                           );
   if ( !ok ) {
      return false;
   }
   parameters[ "cormap_minq" ] = QString( "%1" ).arg( result );
   return true;
}

void US_Hydrodyn_Dad::cormap() 
{
   map < QString, QString > parameters;
   cormap( parameters );
}

void US_Hydrodyn_Dad::cormap( map < QString, QString > & parameters )
{
   disable_all();

   // build up all pairs

   parameters[ "msg" ] = "";
   parameters[ "linewidth" ] = QString( "%1" ).arg( use_line_width );
   parameters[ "hide_adjpvalues" ] = "true";
   if ( !parameters.count( "alpha" ) ) {
      parameters[ "alpha" ] = 
         QString( "%1" )
         .arg( ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "dad_cormap_alpha" ) ?
               ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_cormap_alpha" ].toDouble() * 5 : 0.05 );
   }
   
   double cormap_maxq = ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "dad_cormap_maxq" ) ?
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_cormap_maxq" ].toDouble() : 0.05;

   if ( parameters.count( "cormap_maxq" ) ) {
      cormap_maxq = parameters[ "cormap_maxq" ].toDouble();
   }

   // us_qdebug( QString( "cormap maxq %1" ).arg( cormap_maxq ) );

   switch( current_mode ) {
   case MODE_SCALE :
      {
         if ( !scale_selected.size() ||
              !scale_selected_names.size() ) {
            editor_msg( "red", us_tr( "PVP in scale mode has no files" ) );
            scale_enables();
            return;
         }

         if ( !parameters.count( "decimate" ) ) {
            if ( !ask_to_decimate( parameters ) ) {
               scale_enables();
               return;
            }
         }

         int decimate = parameters.count( "decimate" ) ? parameters[ "decimate" ].toInt() : 0;
         double cormap_minq = parameters.count( "cormap_minq" ) ? parameters[ "cormap_minq" ].toDouble() : 0e0;

         // if files are time, then grab alternates, otherwise grab alternate q points
         if ( !f_is_time.count( scale_selected_names[ 0 ] ) ) {
            editor_msg( "red", us_tr( "Internal error: PVP in scale mode files not in global files" ) );
            scale_enables();
            return;
         }

         bool files_are_time = f_is_time[ scale_selected_names[ 0 ] ];

         vector < QString > use_preq_scale_selected_names;

         if ( files_are_time && decimate ) {
            for ( int i = 0; i < (int) scale_selected_names.size(); i += decimate ) {
               use_preq_scale_selected_names.push_back( scale_selected_names[ i ] );
            }
         } else {
            use_preq_scale_selected_names = scale_selected_names;
         }

         vector < QString > use_scale_selected_names;

         if ( files_are_time ) {
            QRegExp rx_cap( "_It_q(\\d+_\\d+)" );
            for ( int i = 0; i < (int) use_preq_scale_selected_names.size(); ++i ) {
               if ( rx_cap.indexIn( use_preq_scale_selected_names[ i ] ) == -1 ) {
                  // QMessageBox::warning( this
                  //                       , windowTitle() + us_tr( " : PVP Analysis" )
                  //                       , QString( us_tr( "Could not extract q value from file name %1" ) )
                  //                       .arg( use_preq_scale_selected_names[ i ] )
                  //                       ,QMessageBox::Ok | QMessageBox::Default
                  //                       ,QMessageBox::NoButton
                  //                       );
                  editor_msg( "red", 
                              QString( us_tr( "PVP Analysis: Could not extract q value from file name %1" ) )
                              .arg( use_preq_scale_selected_names[ i ] ) );
                  scale_enables();
                  return;
               }                  
               double qv = rx_cap.cap( 1 ).replace( "_", "." ).toDouble();
               // us_qdebug( QString( "baseline cormap captured %1 from %2" ).arg( qv ).arg( use_preq_scale_selected_names[ i ] ) );
               if ( qv <= cormap_maxq && qv >= cormap_minq ) {
                  use_scale_selected_names.push_back( use_preq_scale_selected_names[ i ] );
               }
            }
         } else {
            use_scale_selected_names = use_preq_scale_selected_names;
         }
            
         if ( use_scale_selected_names.size() < 2 ) {
            editor_msg( "red", us_tr( "Insufficient curves remaining for PVP Analysis" ) );
            scale_enables();
            return;
         }            

         // and once for the full range
         if ( !files_are_time ) {
            vector < double >            q = scale_q[ use_scale_selected_names[ 0 ] ];
            vector < vector < double > > I( 2 );
            vector < vector < double > > rkl;

            // for ( set < QString >::iterator it = scale_selected.begin();
            //       it != scale_selected.end();
            //       ++it )
            // {
            //    q = scale_q[ *it ];
            //    break;
            // }

            int    N;
            int    S;
            int    C;
            double P;

            int    fcount = (int) use_scale_selected_names.size();
            int    m      = 0;

            for ( int i = 0; i < fcount - 1; ++i ) {
               for ( int j = i + 1; j < fcount; ++j ) {
                  ++m;
               }
            }

            vector < vector < double > > pvaluepairs( fcount );
            vector < vector < double > > adjpvaluepairs( fcount );

            int use_names_max_len = 10;

            for ( int i = 0; i < fcount; ++i ) {
               pvaluepairs   [ i ].resize( fcount );
               pvaluepairs   [ i ][ i ] = 1;
               adjpvaluepairs[ i ].resize( fcount );
               adjpvaluepairs[ i ][ i ] = 1;
               if ( use_names_max_len < (int) use_scale_selected_names[ i ].length() ) {
                  use_names_max_len = (int) use_scale_selected_names[ i ].length();
               }
            }

            parameters[ "msg" ] = QString( "%1\t%2\t    N  Start point  C   P-value\n" )
               .arg( "File", -use_names_max_len )
               .arg( "File", -use_names_max_len )
               // + "\tAdj P-Value"            
               ;

            progress->reset();
            int pp = 0;

            vector < double > undecimated_q = q;
            int q_points = (int) q.size();
            bool do_q_decimate = decimate;
            if ( do_q_decimate ) {
               vector < double > new_q;
               for ( int k = 0; k < q_points; k += decimate ) {
                  if ( q[ k ] >= cormap_minq ) {
                     new_q.push_back( q[ k ] );
                  }
               }
               q = new_q;
            }

            for ( int i = 0; i < fcount - 1; ++i ) {
               I[ 0 ] = scale_I[ use_scale_selected_names[ i ] ];

               if ( do_q_decimate ) {
                  vector < double > new_I;
                  for ( int k = 0; k < q_points; k += decimate ) {
                     if ( undecimated_q[ k ] >= cormap_minq ) {
                        new_I.push_back( I[ 0 ][ k ] );
                     }
                  }
                  I[ 0 ] = new_I;
               }

               for ( int j = i + 1; j < fcount; ++j ) {
                  progress->setValue( pp++ ); progress->setMaximum( m );
                  qApp->processEvents();

                  I[ 1 ] = scale_I[ use_scale_selected_names[ j ] ];

                  if ( do_q_decimate ) {
                     vector < double > new_I;
                     for ( int k = 0; k < q_points; k += decimate ) {
                        if ( undecimated_q[ k ] >= cormap_minq ) {
                           new_I.push_back( I[ 1 ][ k ] );
                        }
                     }
                     I[ 1 ] = new_I;
                  }

                  if ( !usu->cormap( q, I, rkl, N, S, C, P ) ) {
                     editor_msg( "red", usu->errormsg );
                  }
                  double adjP = (double) m * P;
                  if ( adjP > 1e0 ) {
                     adjP = 1e0;
                  }
                  pvaluepairs[ i ][ j ] = P;
                  pvaluepairs[ j ][ i ] = P;
                  adjpvaluepairs[ i ][ j ] = adjP;
                  adjpvaluepairs[ j ][ i ] = adjP;

                  parameters[ "msg" ] += 
                     QString( "%1\t%2\t%3\t%4\t%5\t%6"
                              // "\t%7"
                              "\n" )
                     .arg( use_scale_selected_names[ i ], -use_names_max_len )
                     .arg( use_scale_selected_names[ j ], -use_names_max_len )
                     .arg( N, 6 )
                     .arg( S, 6 )
                     .arg( C, 6 )
                     .arg( QString( "" ).sprintf( "%.4g", P ).leftJustified( 12 ) )
                     // .arg( adjP ) 
                     ;
               }
            }

            progress->reset();

            {
               parameters[ "title" ] = 
                  QString(
                          us_tr( "Scale mode %1: Full q range used for analysis." )
                          )
                  .arg( scale_applied ?
                        QString( "scaled on %1 range %2 to %3" )
                        .arg( files_are_time ? "frame" : "q" ) 
                        .arg( scale_applied_q_min )
                        .arg( scale_applied_q_max ) 
                        :
                        QString( "(no scaling applied)" ) )
                  ;

               parameters[ "ppvm_title"     ] = us_tr( "Pairwise P value map" );
               parameters[ "ppvm_title_adj" ] = us_tr( "Pairwise adjusted P value map" );

               parameters[ "title_adj" ] = 
                  QString(
                          us_tr( "Scale mode %1: Full q range used for analysis." )
                          )
                  .arg( scale_applied ?
                        QString( "scaled on %1 range %2 to %3" )
                        .arg( files_are_time ? "frame" : "q" ) 
                        .arg( scale_applied_q_min )
                        .arg( scale_applied_q_max ) 
                        :
                        QString( "(no scaling applied)" ) )
                  ;

               if ( cormap_minq > 0e0 ) {
                  parameters[ "title"     ] += QString( " Minimum q limit is %1 [A^-1]." ).arg( cormap_minq );
                  parameters[ "title_adj" ] += QString( " Minimum q limit is %1 [A^-1]." ).arg( cormap_minq );
               }

               if ( parameters.count( "decimate" ) ) {
                  int decimate = parameters[ "decimate" ].toInt();
                  if ( decimate > 1 ) {
                     QString nth_tag;
                     switch ( decimate ) {
                     case 2 : nth_tag = "nd"; break;
                     case 3 : nth_tag = "rd"; break;
                     default : nth_tag = "th"; break;
                     }
               
                     parameters[ "title" ] += QString( us_tr( " Only every %1%2 q value sampled." ) )
                        .arg( decimate ).arg( nth_tag );
                     parameters[ "title_adj" ] += QString( us_tr( " Only every %1%2 q value sampled." ) )
                        .arg( decimate ).arg( nth_tag );
                  }
               }

               // parameters[ "adjusted" ] = "true";

               parameters[ "linewisesummary" ] = "true";
               parameters[ "clusteranalysis"   ] = "true";

               parameters[ "global_width" ] = "1000";
               parameters[ "global_height" ] = "700";
               parameters[ "image_height" ] = "500";

               // parameters[ "name"              ] = use_scale_selected_names.front();
               {
                  QStringList qsl = vector_qstring_to_qstringlist( use_scale_selected_names );
                  QString head = qstring_common_head( qsl, true );
                  QString tail = qstring_common_tail( qsl, true );
                  parameters[ "name" ] = QString( "%1_%2%3_%4%5_cqmn%6_mx%7" )
                     .arg( head )
                     .arg( tail )
                     .arg( parameters.count( "decimate" ) ? QString( "_s%1" ).arg( parameters[ "decimate" ] ) : QString( "" ) )
                     .arg( files_are_time ? "q" : "f" )
                     .arg( pvaluepairs.size() )
                     .arg( cormap_minq )
                     .arg( cormap_maxq )
                     .replace( ".", "_" )
                     ;
               }

               US_Hydrodyn_Saxs_Cormap * uhcm = new US_Hydrodyn_Saxs_Cormap( us_hydrodyn,
                                                                             parameters,
                                                                             pvaluepairs,
                                                                             adjpvaluepairs,
                                                                             use_scale_selected_names );
               if ( parameters.count( "close" ) ) {
                  delete uhcm;
               } else {
                  uhcm->show();
               }
            }
         }

         // once for maxq cutoff
         {
            vector < double >            q = scale_q[ use_scale_selected_names[ 0 ] ];
            vector < vector < double > > I( 2 );
            vector < vector < double > > rkl;

            // for ( set < QString >::iterator it = scale_selected.begin();
            //       it != scale_selected.end();
            //       ++it )
            // {
            //    q = scale_q[ *it ];
            //    break;
            // }

            {
               vector < vector < double > > grids;

               for ( int i = 0; i < (int) use_scale_selected_names.size(); ++i )
               {
                  QString this_file = use_scale_selected_names[ i ];
                  if ( scale_q.count( this_file ) &&
                       scale_I.count( this_file ) &&
                       scale_q[ this_file ].size() &&
                       scale_I[ this_file ].size() )
                  {
                     grids.push_back( scale_q[ this_file ] );
                  }
               }

               vector < double > v_union = US_Vector::vunion( grids );
               vector < double > v_int   = US_Vector::intersection( grids );

               bool any_differences = v_union != v_int;

               if ( any_differences )
               {
                  editor_msg( "red", us_tr( "PVP: curves must be on the same grid, try 'Crop Common' first." ) );
                  scale_enables();
                  return;
               }
            }

            int    N;
            int    S;
            int    C;
            double P;

            int    fcount = (int) use_scale_selected_names.size();
            int    m      = 0;

            for ( int i = 0; i < fcount - 1; ++i ) {
               for ( int j = i + 1; j < fcount; ++j ) {
                  ++m;
               }
            }

            vector < vector < double > > pvaluepairs( fcount );
            vector < vector < double > > adjpvaluepairs( fcount );

            int use_names_max_len = 10;

            for ( int i = 0; i < fcount; ++i ) {
               pvaluepairs   [ i ].resize( fcount );
               pvaluepairs   [ i ][ i ] = 1;
               adjpvaluepairs[ i ].resize( fcount );
               adjpvaluepairs[ i ][ i ] = 1;
               if ( use_names_max_len < (int) use_scale_selected_names[ i ].length() ) {
                  use_names_max_len = (int) use_scale_selected_names[ i ].length();
               }
            }

            parameters[ "msg" ] = QString( "%1\t%2\t    N  Start point  C   P-value\n" )
               .arg( "File", -use_names_max_len )
               .arg( "File", -use_names_max_len )
               // + "\tAdj P-Value"            
               ;

            progress->reset();
            int pp = 0;

            vector < double > undecimated_q = q;
            int q_points = (int) q.size();
            bool do_q_decimate = !files_are_time;
            int use_decimate = decimate ? decimate : 1;
            if ( do_q_decimate ) {
               vector < double > new_q;
               for ( int k = 0; k < q_points; k += use_decimate ) {
                  if ( q[ k ] <= cormap_maxq &&
                       q[ k ] >= cormap_minq ) {
                     new_q.push_back( q[ k ] );
                  }
               }
               q = new_q;
            }

            for ( int i = 0; i < fcount - 1; ++i ) {
               I[ 0 ] = scale_I[ use_scale_selected_names[ i ] ];

               if ( do_q_decimate ) {
                  vector < double > new_I;
                  for ( int k = 0; k < q_points; k += use_decimate ) {
                     if ( undecimated_q[ k ] <= cormap_maxq &&
                          undecimated_q[ k ] >= cormap_minq ) {
                        new_I.push_back( I[ 0 ][ k ] );
                     }
                  }
                  I[ 0 ] = new_I;
               }

               for ( int j = i + 1; j < fcount; ++j ) {
                  progress->setValue( pp++ ); progress->setMaximum( m );
                  qApp->processEvents();

                  I[ 1 ] = scale_I[ use_scale_selected_names[ j ] ];

                  if ( do_q_decimate ) {
                     vector < double > new_I;
                     for ( int k = 0; k < q_points; k += use_decimate ) {
                        if ( undecimated_q[ k ] <= cormap_maxq &&
                             undecimated_q[ k ] >= cormap_minq ) {
                           new_I.push_back( I[ 1 ][ k ] );
                        }
                     }
                     I[ 1 ] = new_I;
                  }

                  if ( !usu->cormap( q, I, rkl, N, S, C, P ) ) {
                     editor_msg( "red", usu->errormsg );
                  }
                  double adjP = (double) m * P;
                  if ( adjP > 1e0 ) {
                     adjP = 1e0;
                  }
                  pvaluepairs[ i ][ j ] = P;
                  pvaluepairs[ j ][ i ] = P;
                  adjpvaluepairs[ i ][ j ] = adjP;
                  adjpvaluepairs[ j ][ i ] = adjP;

                  parameters[ "msg" ] += 
                     QString( "%1\t%2\t%3\t%4\t%5\t%6"
                              // "\t%7"
                              "\n" )
                     .arg( use_scale_selected_names[ i ], -use_names_max_len )
                     .arg( use_scale_selected_names[ j ], -use_names_max_len )
                     .arg( N, 6 )
                     .arg( S, 6 )
                     .arg( C, 6 )
                     .arg( QString( "" ).sprintf( "%.4g", P ).leftJustified( 12 ) )
                     // .arg( adjP ) 
                     ;
               }
            }

            progress->reset();

            {
               parameters[ "title" ] = 
                  QString(
                          us_tr( "Scale mode %1: Maximum q limit is %2 [A^-1]." )
                          )
                  .arg( scale_applied ?
                        QString( "scaled on %1 range %2 to %3" )
                        .arg( files_are_time ? "frame" : "q" ) 
                        .arg( scale_applied_q_min )
                        .arg( scale_applied_q_max ) 
                        :
                        QString( "(no scaling applied)" ) )
                  .arg( cormap_maxq )
                  ;
               parameters[ "ppvm_title"     ] = us_tr( "Pairwise P value map" );
               parameters[ "ppvm_title_adj" ] = us_tr( "Pairwise adjusted P value map" );

               parameters[ "title_adj" ] = 
                  QString(
                          us_tr( "Scale mode %1: Maximum q limit is %2 [A^-1]." )
                          )
                  .arg( scale_applied ?
                        QString( "scaled on %1 range %2 to %3" )
                        .arg( files_are_time ? "frame" : "q" ) 
                        .arg( scale_applied_q_min )
                        .arg( scale_applied_q_max ) 
                        :
                        QString( "(no scaling applied)" ) )
                  .arg( cormap_maxq )
                  ;

               if ( cormap_minq > 0e0 ) {
                  parameters[ "title"     ] += QString( " Minimum q limit is %1 [A^-1]." ).arg( cormap_minq );
                  parameters[ "title_adj" ] += QString( " Minimum q limit is %1 [A^-1]." ).arg( cormap_minq );
               }

               if ( parameters.count( "decimate" ) ) {
                  int decimate = parameters[ "decimate" ].toInt();
                  if ( decimate > 1 ) {
                     QString nth_tag;
                     switch ( decimate ) {
                     case 2 : nth_tag = "nd"; break;
                     case 3 : nth_tag = "rd"; break;
                     default : nth_tag = "th"; break;
                     }
               
                     parameters[ "title" ] += QString( us_tr( " Only every %1%2 q value sampled." ) )
                        .arg( decimate ).arg( nth_tag );
                     parameters[ "title_adj" ] += QString( us_tr( " Only every %1%2 q value sampled." ) )
                        .arg( decimate ).arg( nth_tag );
                  }
               }

               parameters[ "adjusted" ] = "true";

               parameters[ "linewisesummary" ] = "true";
               parameters[ "clusteranalysis"   ] = "true";

               parameters[ "global_width" ] = "1000";
               parameters[ "global_height" ] = "700";
               parameters[ "image_height" ] = "300";

               US_Hydrodyn_Saxs_Cormap * uhcm = new US_Hydrodyn_Saxs_Cormap( us_hydrodyn,
                                                                             parameters,
                                                                             pvaluepairs,
                                                                             adjpvaluepairs,
                                                                             use_scale_selected_names );
               if ( parameters.count( "close" ) ) {
                  delete uhcm;
               } else {
                  uhcm->show();
               }
            }
         }

         scale_enables();
         if ( testiq_active )
         {
            pb_testiq->setEnabled( true );
         }
      }
      break;

   case MODE_BLANKS :
      {
         blanks_last_cormap_parameters.clear( );
         blanks_last_cormap_pvaluepairs.clear( );
         blanks_last_brookesmap_sliding_results.clear( );

         {
            vector < vector < double > > grids;

            for ( int i = 0; i < (int) blanks_created.size(); ++i )
            {
               QString this_file = blanks_created[ i ];
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
               editor_msg( "red", us_tr( "PVP: curves must be on the same grid, try 'Crop Common' first." ) );
               blanks_enables();
               return;
            }
         }

         if ( !parameters.count( "decimate" ) ) {
            if ( !ask_to_decimate( parameters ) ) {
               blanks_enables();
               return;
            }
         }

         double cormap_minq = parameters.count( "cormap_minq" ) ? parameters[ "cormap_minq" ].toDouble() : 0e0;

         QStringList use_preq_blanks_created;

         if ( parameters.count( "decimate" ) ) {
            int decimate = parameters[ "decimate" ].toInt();
            for ( int i = 0; i < (int) blanks_created.size(); i += decimate ) {
               use_preq_blanks_created << blanks_created[ i ];
            }
         } else {
            use_preq_blanks_created = blanks_created;
         }

         QStringList use_blanks_created;
         // US_Vector::printvector( "blanks_created_q", blanks_created_q );
         {
            // QRegExp rx_cap( "_It_q(\\d+_\\d+)" );
            int use_decimate = parameters.count( "decimate" ) ? parameters[ "decimate" ].toInt() : 1;
            for ( int i = 0; i < (int) use_preq_blanks_created.size(); ++i ) {
               // if ( rx_cap.indexIn( use_preq_blanks_created[ i ] ) == -1 ) {
               //    editor_msg( "red", 
               //                QString( us_tr( "PVP Analysis: Could not extract q value from file name %1" ) )
               //                .arg( use_preq_blanks_created[ i ] ) );
               //    baseline_enables();
               //    return;
               // }                  
               // double qv = rx_cap.cap( 1 ).replace( "_", "." ).toDouble();
               // us_qdebug( QString( "qv %1 blanks_created_q[ i ] %2" ).arg( qv ).arg( blanks_created_q[ i * use_decimate ] ) );
               
               // if ( qv <= cormap_maxq ) {
               if ( blanks_created_q[ (vector<double>::size_type) i * use_decimate ] <= cormap_maxq &&
                    blanks_created_q[ (vector<double>::size_type) i * use_decimate ] >= cormap_minq ) {
                  use_blanks_created << use_preq_blanks_created[ i ];
               }
            }
         }

         if ( use_blanks_created.size() < 2 ) {
            editor_msg( "red", us_tr( "Insufficient curves remaining for PVP Analysis" ) );
            blanks_enables();
            return;
         }            

         // build up q vectors from le_baseline_end_s to le_baseline_end_e

         double q_start     = le_baseline_end_s->text().toDouble();
         double q_end       = le_baseline_end_e->text().toDouble();
         vector < double > t;
         vector < double > q;

         vector < int > indicies;
         vector < QString > use_names;
         int use_names_max_len = 10;

         for ( int i = 0; i < (int) f_qs[ wheel_file ].size(); ++i ) {
            if ( f_qs[ wheel_file ][ i ] >= q_start &&
                 f_qs[ wheel_file ][ i ] <= q_end ) {
               indicies.push_back( i );
               t.push_back( f_qs[ wheel_file ][ i ] );
               use_names.push_back( QString( "%1" ).arg( t.back() ) );
               if ( use_names_max_len < (int) use_names.back().length() ) {
                  use_names_max_len = use_names.back().length();
               }
            }
         }
         
         if ( !t.size() ) {
            editor_msg( "red", us_tr( "PVP in blanks mode has empty start / end range" ) );
            blanks_enables();
            return;
         }

         vector < vector < double > > I( 2 );
         vector < vector < double > > rkl;

         int blanks_size = (int) use_blanks_created.size();
         // us_qdebug( QString( "cormap blanks blanks_size %1" ).arg( blanks_size ) );

         q.resize( blanks_size );
         I[ 0 ].resize( blanks_size );
         I[ 1 ].resize( blanks_size );

         int    N;
         int    S;
         int    C;
         double P;

         int    fcount = (int) indicies.size();
         int    m      = 0;

         for ( int i = 0; i < fcount - 1; ++i ) {
            for ( int j = i + 1; j < fcount; ++j ) {
               ++m;
            }
         }

         vector < vector < double > > pvaluepairs   ( fcount );
         vector < vector < double > > adjpvaluepairs( fcount );
         for ( int i = 0; i < fcount; ++i ) {
            pvaluepairs   [ i ].resize( fcount );
            pvaluepairs   [ i ][ i ] = 1;
            adjpvaluepairs[ i ].resize( fcount );
            adjpvaluepairs[ i ][ i ] = 1;
         }

         // US_Vector::printvector( "q", q );

         // us_qdebug( 
         //        QString( "mb5 fcount %1 indicies.size %2 blanks.size %3 m %4 " )
         //        .arg( fcount )
         //        .arg( indicies.size() ) 
         //        .arg( blanks_size ) 
         //        .arg( m )
         //         );
         
         parameters[ "msg" ] = QString( "%1\t%2\t    N  Start point  C   P-value\n" )
            .arg( "Time/Frame", -use_names_max_len )
            .arg( "Time/Frame", -use_names_max_len )
            // + "\tAdj P-Value"            
            ;

         progress->reset();
         int pp = 0;

         // compute average sd's

         {
            set < int > skip_k;
            for ( int k = 0; k < blanks_size; ++k ) {
               if ( !f_errors.count( use_blanks_created[ k ] ) ||
                    f_errors[ use_blanks_created[ k ] ].size() != f_Is[ use_blanks_created[ k ] ].size() ||
                    !is_nonzero_vector( f_errors[ use_blanks_created[ k ] ] ) ) {
                  skip_k.insert( k );
               }
            }

            double gsum_sd2 = 0e0;

            for ( int i = 0; i < fcount; ++i ) {
               double sum_sd2 = 0e0;
               for ( int k = 0; k < blanks_size; ++k ) {
                  if ( !skip_k.count( k ) ) {
                     sum_sd2 += 
                        f_errors[ use_blanks_created[ k ] ][ indicies[ i ] ] 
                        * f_errors[ use_blanks_created[ k ] ][ indicies[ i ] ];
                     // us_qdebug( QString( "errors for %1 %2 %3\n" )
                     //         .arg( use_blanks_created[ k ] )
                     //         .arg( f_qs[ use_blanks_created[ k ] ][ indicies[ i ] ] )
                     //         .arg( f_errors[ use_blanks_created[ k ] ][ indicies[ i ] ]  ) );
                  }
               }
               gsum_sd2 += sqrt( sum_sd2 );
            }

            // double avg_sd = sqrt( sum_sd2 ) / (double) fcount;
            double avg_sd = gsum_sd2 / (double) fcount;
            parameters[ "blanks_avg_maxq_sd" ] = QString( "%1" ).arg( avg_sd );

            // us_qdebug( QString( "avg blanks sd %1 fcount %2 skip_k.size() %3" )
            //         .arg( avg_sd )
            //         .arg( fcount )
            //         .arg( skip_k.size() ) )
            //    ;
         }

         for ( int i = 0; i < fcount - 1; ++i ) {
            for ( int k = 0; k < blanks_size; ++k ) {
               I[ 0 ][ k ] = f_Is[ use_blanks_created[ k ] ][ indicies[ i ] ];
            }
            for ( int j = i + 1; j < fcount; ++j ) {
               progress->setValue( pp++ ); progress->setMaximum( m );
               qApp->processEvents();

               for ( int k = 0; k < blanks_size; ++k ) {
                  I[ 1 ][ k ] = f_Is[ use_blanks_created[ k ] ][ indicies[ j ] ];
               }
               // US_Vector::printvector2( QString( "blanks for cormap i %1 j %2 I" ).arg( i ).arg( j ), I[ 0 ], I[ 1 ] );
                                        
               if ( !usu->cormap( q, I, rkl, N, S, C, P ) ) {
                  editor_msg( "red", usu->errormsg );
               }

               double adjP = (double) m * P;
               if ( adjP > 1e0 ) {
                  adjP = 1e0;
               }
               pvaluepairs[ i ][ j ] = P;
               pvaluepairs[ j ][ i ] = P;
               adjpvaluepairs[ i ][ j ] = adjP;
               adjpvaluepairs[ j ][ i ] = adjP;

               parameters[ "msg" ] += 
                  QString( "%1\t%2\t%3\t%4\t%5\t%6"
                           // "\t%7"
                           "\n" )
                  .arg( use_names[ i ], -use_names_max_len )
                  .arg( use_names[ j ], -use_names_max_len )
                  .arg( N, 6 )
                  .arg( S, 6 )
                  .arg( C, 6 )
                  .arg( QString( "" ).sprintf( "%.4g", P ).leftJustified( 12 ) )
                  // .arg( adjP ) 
                  ;
            }
         }

         progress->reset();

         {
            parameters[ "title" ] = 
               QString(
                       us_tr( "Blanks mode t %1 to %2. Maximum q limit is %3 [A^-1]." )
                       )
               .arg( q_start )
               .arg( q_end )
               .arg( cormap_maxq )
               ;

            parameters[ "ppvm_title"     ] = us_tr( "Pairwise P value map" );
            parameters[ "ppvm_title_adj" ] = us_tr( "Pairwise adjusted P value map" );

            parameters[ "csv_id_header" ] = "t start\",\"t end\",\"frames";
            parameters[ "csv_id_data" ]   = QString( "%1,%2,%3" ).arg( q_start ).arg( q_end ).arg( fcount );
            parameters[ "global_width" ] = "1000";
            parameters[ "global_height" ] = "700";
            parameters[ "image_height" ] = "300";

            parameters[ "title_adj" ] = 
               QString(
                       us_tr( "Blanks mode t %1 to %2." )
                       )
               .arg( q_start )
               .arg( q_end )
               ;

            if ( cormap_minq > 0e0 ) {
               parameters[ "title"     ] += QString( " Minimum q limit is %1 [A^-1]." ).arg( cormap_minq );
               parameters[ "title_adj" ] += QString( " Minimum q limit is %1 [A^-1]." ).arg( cormap_minq );
            }

            if ( parameters.count( "decimate" ) ) {
               int decimate = parameters[ "decimate" ].toInt();
               if ( decimate > 1 ) {
                  QString nth_tag;
                  switch ( decimate ) {
                  case 2 : nth_tag = "nd"; break;
                  case 3 : nth_tag = "rd"; break;
                  default : nth_tag = "th"; break;
                  }
               
                  parameters[ "title" ] += QString( us_tr( " Only every %1%2 q value sampled." ) )
                     .arg( decimate ).arg( nth_tag );
                  parameters[ "title_adj" ] += QString( us_tr( " Only every %1%2 q value sampled." ) )
                     .arg( decimate ).arg( nth_tag );
               }
            }

            parameters[ "linewisesummary"   ] = "true";
            parameters[ "fileheader"        ] = "Time/Frame";
            parameters[ "cormap_of_brookes" ] = "true";
            parameters[ "clusteranalysis"   ] = "true";
            // parameters[ "name"              ] = blanks_created.front();
            {
               QString head = qstring_common_head( blanks_selected, true );
               QString tail = qstring_common_tail( blanks_selected, true );
               parameters[ "name" ] = QString( "%1_%2%3_f%4_cqmx%5" )
                  .arg( head )
                  .arg( tail )
                  .arg( parameters.count( "decimate" ) ? QString( "_s%1" ).arg( parameters[ "decimate" ] ) : QString( "" ) )
                  .arg( pvaluepairs.size() )
                  .arg( cormap_maxq )
                  .replace( ".", "_" )
                  ;
            }
            parameters[ "frames" ] = QString( "%1" ).arg( blanks_selected.size() );
            parameters[ "isblanks" ] = "true";

            US_Hydrodyn_Saxs_Cormap * uhcm = new US_Hydrodyn_Saxs_Cormap( us_hydrodyn,
                                                                          parameters,
                                                                          pvaluepairs,
                                                                          adjpvaluepairs,
                                                                          use_names );
            blanks_last_cormap_parameters  = parameters;
            blanks_last_cormap_pvaluepairs = pvaluepairs;
            blanks_last_cormap_run_end_s   = le_baseline_end_s->text().toDouble();
            blanks_last_cormap_run_end_e   = le_baseline_end_e->text().toDouble();

            if ( parameters.count( "close" ) ) {
               delete uhcm;
            } else {
               uhcm->show();
            }
         }
         blanks_enables();
      }

      break;

   case MODE_BASELINE :
      {
         baseline_last_cormap_parameters.clear( );
         baseline_last_cormap_pvaluepairs.clear( );
         // baseline_last_brookesmap_sliding_results.clear( );

         if ( !baseline_multi ) {
            editor_msg( "red", us_tr( "PVP in baseline mode has no files" ) );
            baseline_enables();
            return;
         }

         {
            vector < vector < double > > grids;

            for ( int i = 0; i < (int) baseline_selected.size(); ++i )
            {
               QString this_file = baseline_selected[ i ];
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
               editor_msg( "red", us_tr( "PVP: curves must be on the same grid, try 'Crop Common' first." ) );
               baseline_enables();
               return;
            }
         }

         if ( !parameters.count( "decimate" ) ) {
            if ( !ask_to_decimate( parameters ) ) {
               baseline_enables();
               return;
            }
         }

         double cormap_minq = parameters.count( "cormap_minq" ) ? parameters[ "cormap_minq" ].toDouble() : 0e0;

         QStringList use_preq_baseline_selected;

         if ( parameters.count( "decimate" ) ) {
            int decimate = parameters[ "decimate" ].toInt();
            for ( int i = 0; i < (int) baseline_selected.size(); i += decimate ) {
               use_preq_baseline_selected << baseline_selected[ i ];
            }
         } else {
            use_preq_baseline_selected = baseline_selected;
         }

         QStringList use_baseline_selected;
         {
            QRegExp rx_cap( "_It_q(\\d+_\\d+)" );
            for ( int i = 0; i < (int) use_preq_baseline_selected.size(); ++i ) {
               if ( rx_cap.indexIn( use_preq_baseline_selected[ i ] ) == -1 ) {
                  // QMessageBox::warning( this
                  //                       , windowTitle() + us_tr( " : Baseline PVP Analysis" )
                  //                       , QString( us_tr( "Could not extract q value from file name %1" ) )
                  //                       .arg( use_preq_baseline_selected[ i ] )
                  //                       ,QMessageBox::Ok | QMessageBox::Default
                  //                       ,QMessageBox::NoButton
                  //                       );
                  editor_msg( "red", 
                              QString( us_tr( "Baseline PVP Analysis: Could not extract q value from file name %1" ) )
                              .arg( use_preq_baseline_selected[ i ] ) );
                  baseline_enables();
                  return;
               }                  
               double qv = rx_cap.cap( 1 ).replace( "_", "." ).toDouble();
               // us_qdebug( QString( "baseline cormap captured %1 from %2" ).arg( qv ).arg( use_preq_baseline_selected[ i ] ) );
               if ( qv <= cormap_maxq &&
                    qv >= cormap_minq ) {
                  use_baseline_selected << use_preq_baseline_selected[ i ];
               }
            }
         }
         if ( use_baseline_selected.size() < 2 ) {
            editor_msg( "red", us_tr( "Insufficient curves remaining for PVP Analysis" ) );
            baseline_enables();
            return;
         }            

         // build up q vectors from le_baseline_end_s to le_baseline_end_e

         double q_start     = le_baseline_end_s->text().toDouble();
         double q_end       = parameters.count( "baseline_use_end" ) ? le_baseline_end->text().toDouble() : le_baseline_end_e->text().toDouble();

         vector < double > t;
         vector < double > q;

         vector < int > indicies;
         vector < QString > use_names;
         int use_names_max_len = 10;

         for ( int i = 0; i < (int) f_qs[ wheel_file ].size(); ++i ) {
            if ( f_qs[ wheel_file ][ i ] >= q_start &&
                 f_qs[ wheel_file ][ i ] <= q_end ) {
               indicies.push_back( i );
               t.push_back( f_qs[ wheel_file ][ i ] );
               use_names.push_back( QString( "%1" ).arg( t.back() ) );
               if ( use_names_max_len < (int) use_names.back().length() ) {
                  use_names_max_len = use_names.back().length();
               }
            }
         }
         
         if ( !t.size() ) {
            editor_msg( "red", us_tr( "PVP in baseline mode has empty start / end range" ) );
            baseline_enables();
            return;
         }

         vector < vector < double > > I( 2 );
         vector < vector < double > > rkl;

         int baseline_size = (int) use_baseline_selected.size();

         q.resize( baseline_size );
         I[ 0 ].resize( baseline_size );
         I[ 1 ].resize( baseline_size );

         int    N;
         int    S;
         int    C;
         double P;

         int    fcount = (int) indicies.size();
         int    m      = 0;

         for ( int i = 0; i < fcount - 1; ++i ) {
            for ( int j = i + 1; j < fcount; ++j ) {
               ++m;
            }
         }

         vector < vector < double > > pvaluepairs   ( fcount );
         vector < vector < double > > adjpvaluepairs( fcount );
         for ( int i = 0; i < fcount; ++i ) {
            pvaluepairs   [ i ].resize( fcount );
            pvaluepairs   [ i ][ i ] = 1;
            adjpvaluepairs[ i ].resize( fcount );
            adjpvaluepairs[ i ][ i ] = 1;
         }

         // US_Vector::printvector( "q", q );

         // us_qdebug( 
         //        QString( "mb5 fcount %1 indicies.size %2 baseline.size %3 m %4 " )
         //        .arg( fcount )
         //        .arg( indicies.size() ) 
         //        .arg( baseline_size ) 
         //        .arg( m )
         //         );
         
         parameters[ "msg" ] = QString( "%1\t%2\t    N  Start point  C   P-value\n" )
            .arg( "Time/Frame", -use_names_max_len )
            .arg( "Time/Frame", -use_names_max_len )
            // + "\tAdj P-Value"            
            ;

         progress->reset();
         int pp = 0;

         for ( int i = 0; i < fcount - 1; ++i ) {
            for ( int k = 0; k < baseline_size; ++k ) {
               I[ 0 ][ k ] = f_Is[ use_baseline_selected[ k ] ][ indicies[ i ] ];
            }
            for ( int j = i + 1; j < fcount; ++j ) {
               progress->setValue( pp++ ); progress->setMaximum( m );
               qApp->processEvents();

               for ( int k = 0; k < baseline_size; ++k ) {
                  I[ 1 ][ k ] = f_Is[ use_baseline_selected[ k ] ][ indicies[ j ] ];
               }
               // US_Vector::printvector2( QString( "for cormap i %1 j %2 I" ).arg( i ).arg( j ), I[ 0 ], I[ 1 ] );
                                        
               if ( !usu->cormap( q, I, rkl, N, S, C, P ) ) {
                  editor_msg( "red", usu->errormsg );
               }

               double adjP = (double) m * P;
               if ( adjP > 1e0 ) {
                  adjP = 1e0;
               }
               pvaluepairs[ i ][ j ] = P;
               pvaluepairs[ j ][ i ] = P;
               adjpvaluepairs[ i ][ j ] = adjP;
               adjpvaluepairs[ j ][ i ] = adjP;

               parameters[ "msg" ] += 
                  QString( "%1\t%2\t%3\t%4\t%5\t%6"
                           // "\t%7"
                           "\n" )
                  .arg( use_names[ i ], -use_names_max_len )
                  .arg( use_names[ j ], -use_names_max_len )
                  .arg( N, 6 )
                  .arg( S, 6 )
                  .arg( C, 6 )
                  .arg( QString( "" ).sprintf( "%.4g", P ).leftJustified( 12 ) )
                  // .arg( adjP ) 
                  ;
            }
         }

         progress->reset();

         {
            parameters[ "title" ] = 
               QString(
                       us_tr( "Baseline mode t %1 to %2. Maximum q limit is %3 [A^-1]." )
                       )
               .arg( q_start )
               .arg( q_end )
               .arg( cormap_maxq )
               ;

            parameters[ "ppvm_title"     ] = us_tr( "Pairwise P value map" );
            parameters[ "ppvm_title_adj" ] = us_tr( "Pairwise adjusted P value map" );

            parameters[ "csv_id_header" ] = "t start\",\"t end\",\"frames";
            parameters[ "csv_id_data" ]   = QString( "%1,%2,%3" ).arg( q_start ).arg( q_end ).arg( fcount );
            parameters[ "global_width" ] = "1000";
            parameters[ "global_height" ] = "700";
            parameters[ "image_height" ] = "300";

            parameters[ "title_adj" ] = 
               QString(
                       us_tr( "Baseline mode t %1 to %2." )
                       )
               .arg( q_start )
               .arg( q_end )
               ;

            if ( cormap_minq > 0e0 ) {
               parameters[ "title"     ] += QString( " Minimum q limit is %1 [A^-1]." ).arg( cormap_minq );
               parameters[ "title_adj" ] += QString( " Minimum q limit is %1 [A^-1]." ).arg( cormap_minq );
            }

            if ( parameters.count( "decimate" ) ) {
               int decimate = parameters[ "decimate" ].toInt();
               if ( decimate > 1 ) {
                  QString nth_tag;
                  switch ( decimate ) {
                  case 2 : nth_tag = "nd"; break;
                  case 3 : nth_tag = "rd"; break;
                  default : nth_tag = "th"; break;
                  }
               
                  parameters[ "title" ] += QString( us_tr( " Only every %1%2 q value sampled." ) )
                     .arg( decimate ).arg( nth_tag );
                  parameters[ "title_adj" ] += QString( us_tr( " Only every %1%2 q value sampled." ) )
                     .arg( decimate ).arg( nth_tag );
               }
            }

            parameters[ "linewisesummary"   ] = "true";
            parameters[ "fileheader"        ] = "Time/Frame";
            parameters[ "cormap_of_brookes" ] = "true";
            parameters[ "clusteranalysis"   ] = "true";

            parameters[ "name" ]             = use_names.front();
            {
               QStringList qsl = vector_qstring_to_qstringlist( use_names );
               QString head = qstring_common_head( qsl, true );
               QString tail = qstring_common_tail( qsl, true );
               parameters[ "name" ] = QString( "%1_%2%3_f%4_cqmx%5" )
                  .arg( head )
                  .arg( tail )
                  .arg( parameters.count( "decimate" ) ? QString( "_s%1" ).arg( parameters[ "decimate" ] ) : QString( "" ) )
                  .arg( pvaluepairs.size() )
                  .arg( cormap_maxq )
                  .replace( ".", "_" )
                  ;
            }

            US_Hydrodyn_Saxs_Cormap * uhcm = new US_Hydrodyn_Saxs_Cormap( us_hydrodyn,
                                                                          parameters,
                                                                          pvaluepairs,
                                                                          adjpvaluepairs,
                                                                          use_names );
            baseline_last_cormap_parameters  = parameters;
            baseline_last_cormap_pvaluepairs = pvaluepairs;
            baseline_last_cormap_run_end_s   = q_start;
            baseline_last_cormap_run_end_e   = q_end;

            if ( parameters.count( "close" ) ) {
               delete uhcm;
            } else {
               uhcm->show();
            }
         }
         baseline_enables();
      }
      break;

   case MODE_GGAUSSIAN :
      {
         if ( !unified_ggaussian_ok ) {
            editor_msg( "red", us_tr( "Internal error (PVP): Global Gaussian mode, but unified Global Gaussians are not ok." ) );
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
               editor_msg( "red", us_tr( "Internal error (PVP) building global Gaussians" ) );
               ggaussian_enables();
               return;
            }
         }

         if ( (int) unified_ggaussian_files.size() != (int) ggaussian_last_pfit_P.size() ||
              (int) unified_ggaussian_files.size() != (int) ggaussian_last_pfit_N.size() ||
              (int) unified_ggaussian_files.size() != (int) ggaussian_last_pfit_C.size() ||
              (int) unified_ggaussian_files.size() != (int) ggaussian_last_pfit_S.size() ||
              (int) unified_ggaussian_files.size() != (int) ggaussian_last_chi2.size() ) {
            editor_msg( "red", 
                        QString( us_tr( "Internal error (PVP): Global Gaussian mode, last_pfit_* size (%1 %2 %3 %4 %5) does not match number number of Gaussians (%6)" ) )
                        .arg( ggaussian_last_pfit_P.size() )
                        .arg( ggaussian_last_pfit_N.size() )
                        .arg( ggaussian_last_pfit_C.size() )
                        .arg( ggaussian_last_pfit_S.size() )
                        .arg( ggaussian_last_chi2.size() )
                        .arg( unified_ggaussian_files.size() ) 
                        );
            ggaussian_enables();
            return;
         }            
         
         int    fcount = (int) unified_ggaussian_files.size();
         vector < QString > selected_files;

         vector < vector < double > > pvaluepairs( 1 );

         pvaluepairs[ 0 ] = ggaussian_last_pfit_P;
         int use_names_max_len = 10;


         for ( int i = 0; i < fcount ; ++i ) {
            if ( use_names_max_len < (int) unified_ggaussian_files[ i ].length() ) {
               use_names_max_len = (int) unified_ggaussian_files[ i ].length();
            }
         }

         parameters[ "msg" ] = QString( " Ref. : %1\t   N  Start point  C   P-value\n" )
            .arg( "File", -use_names_max_len )
            ;

         for ( int i = 0; i < fcount ; ++i ) {
            selected_files.push_back( unified_ggaussian_files[ i ] );
            // parameters[ "msg" ] += 
            //    QString( "%1\t%2\t%3\t%4\t%5\n" )
            //    .arg( unified_ggaussian_files[ i ] )
            //    .arg( ggaussian_last_pfit_N[ i ] )
            //    .arg( ggaussian_last_pfit_S[ i ] )
            //    .arg( ggaussian_last_pfit_C[ i ] )
            //    .arg( ggaussian_last_pfit_P[ i ] )
            //    ;
               // us_qdebug( QString( "%1 %2 %3 %4 %5 %6 %7" )
            parameters[ "msg" ] += 
               QString( "%1 : %2\t%3\t%4\t%5\t%6"
                        // "\t%7"
                        "\n" )
               .arg( i + 1, 5 )
               .arg( unified_ggaussian_files[ i ], -use_names_max_len )
               .arg( ggaussian_last_pfit_N[ i ], 6 )
               .arg( ggaussian_last_pfit_S[ i ], 6 )
               .arg( ggaussian_last_pfit_C[ i ], 6 )
               .arg( QString( "" ).sprintf( "%.4g", ggaussian_last_pfit_P[ i ] ).leftJustified( 12 ) )
               // .arg( adjP ) 
               ;

         }            

         {
            parameters[ "as_pairs"  ] = "true";
            parameters[ "title"     ] = QString( us_tr( "Global Gaussian mode t range %1 to %2." ) )
               .arg( le_gauss_fit_start->text() )
               .arg( le_gauss_fit_end->text() )
               ;

            parameters[ "ppvm_title"     ] = us_tr( "q value comaprison" );
            parameters[ "ppvm_title_adj" ] = us_tr( "q value comaprison" );

            parameters[ "title_adj" ] = parameters[ "title" ];
            
            parameters[ "global_width" ] = "1000";
            parameters[ "global_height" ] = "500";
            // parameters[ "image_height" ] = "300";

            parameters[ "name" ]         = selected_files.front();
            {
               QStringList qsl = vector_qstring_to_qstringlist( selected_files );
               QString head = qstring_common_head( qsl, true );
               QString tail = qstring_common_tail( qsl, true );
               parameters[ "name" ] = QString( "%1_%2%3_qcnt%4_cqmx%5" )
                  .arg( head )
                  .arg( tail )
                  .arg( parameters.count( "decimate" ) ? QString( "_s%1" ).arg( parameters[ "decimate" ] ) : QString( "" ) )
                  .arg( pvaluepairs.size() )
                  .arg( cormap_maxq )
                  .replace( ".", "_" )
                  ;
            }

            US_Hydrodyn_Saxs_Cormap * uhcm = new US_Hydrodyn_Saxs_Cormap( us_hydrodyn,
                                                                          parameters,
                                                                          pvaluepairs,
                                                                          pvaluepairs,
                                                                          selected_files );
            if ( parameters.count( "close" ) ) {
               delete uhcm;
            } else {
               uhcm->show();
            }
         }
         ggaussian_enables();
      }
      break;

   default :
      {
         vector < QString > selected_files;

         {
            vector < vector < double > > grids;

            for ( int i = 0; i < lb_files->count(); ++i )
            {
               if ( lb_files->item( i )->isSelected() )
               {
                  QString this_file = lb_files->item( i )->text();
                  if ( f_qs.count( this_file ) &&
                       f_Is.count( this_file ) &&
                       f_qs[ this_file ].size() &&
                       f_Is[ this_file ].size() )
                  {
                     selected_files    .push_back( this_file );
                     grids.push_back( f_qs[ this_file ] );
                  }
               }
            }

            vector < double > v_union = US_Vector::vunion( grids );
            vector < double > v_int   = US_Vector::intersection( grids );

            bool any_differences = v_union != v_int;

            if ( any_differences )
            {
               editor_msg( "red", us_tr( "PVP: curves must be on the same grid, try 'Crop Common' first." ) );
               update_enables();
               return;
            }
         }

         if ( !selected_files.size() ) {
            editor_msg( "red", us_tr( "PVP has no files" ) );
            update_enables();
            return;
         }

         if ( !parameters.count( "decimate" ) ) {
            if ( !ask_to_decimate( parameters ) ) {
               update_enables();
               return;
            }
         }

         double cormap_minq = parameters.count( "cormap_minq" ) ? parameters[ "cormap_minq" ].toDouble() : 0e0;

         int decimate = parameters.count( "decimate" ) ? parameters[ "decimate" ].toInt() : 0;

         // if files are time, then grab alternates, otherwise grab alternate q points
         if ( !f_is_time.count( selected_files[ 0 ] ) ) {
            editor_msg( "red", us_tr( "Internal error: PVP files not in global files" ) );
            update_enables();
            return;
         }

         bool files_are_time = f_is_time[ selected_files[ 0 ] ];

         vector < QString > use_preq_selected_files;

         if ( files_are_time && decimate ) {
            for ( int i = 0; i < (int) selected_files.size(); i += decimate ) {
               use_preq_selected_files.push_back( selected_files[ i ] );
            }
         } else {
            use_preq_selected_files = selected_files;
         }

         vector < QString > use_selected_files;

         if ( files_are_time ) {
            QRegExp rx_cap( "_It_q(\\d+_\\d+)" );
            for ( int i = 0; i < (int) use_preq_selected_files.size(); ++i ) {
               if ( rx_cap.indexIn( use_preq_selected_files[ i ] ) == -1 ) {
                  // QMessageBox::warning( this
                  //                       , windowTitle() + us_tr( " : PVP Analysis" )
                  //                       , QString( us_tr( "Could not extract q value from file name %1" ) )
                  //                       .arg( use_preq_selected_files[ i ] )
                  //                       ,QMessageBox::Ok | QMessageBox::Default
                  //                       ,QMessageBox::NoButton
                  //                       );
                  editor_msg( "red", 
                              QString( us_tr( "PVP Analysis: Could not extract q value from file name %1" ) )
                              .arg( use_preq_selected_files[ i ] ) );
                  update_enables();
                  return;
               }                  
               double qv = rx_cap.cap( 1 ).replace( "_", "." ).toDouble();
               // us_qdebug( QString( "baseline cormap captured %1 from %2" ).arg( qv ).arg( use_preq_selected_files[ i ] ) );
               if ( qv <= cormap_maxq && qv >= cormap_minq ) {
                  use_selected_files.push_back( use_preq_selected_files[ i ] );
               }
            }
         } else {
            use_selected_files = use_preq_selected_files;
         }
            
         if ( use_selected_files.size() < 2 ) {
            editor_msg( "red", us_tr( "Insufficient curves remaining for PVP Analysis" ) );
            update_enables();
            return;
         }            

         vector < double >            q = f_qs[ use_selected_files[ 0 ] ];
         vector < vector < double > > I( 2 );
         vector < vector < double > > rkl;

         int    N;
         int    S;
         int    C;
         double P;

         int    fcount = (int) use_selected_files.size();
         int    m      = 0;

         for ( int i = 0; i < fcount - 1; ++i ) {
            for ( int j = i + 1; j < fcount; ++j ) {
               ++m;
            }
         }

         vector < vector < double > > pvaluepairs( fcount );
         vector < vector < double > > adjpvaluepairs( fcount );

         int use_names_max_len = 10;

         for ( int i = 0; i < fcount; ++i ) {
            pvaluepairs   [ i ].resize( fcount );
            pvaluepairs   [ i ][ i ] = 1;
            adjpvaluepairs[ i ].resize( fcount );
            adjpvaluepairs[ i ][ i ] = 1;
            if ( use_names_max_len < (int) use_selected_files[ i ].length() ) {
               use_names_max_len = (int) use_selected_files[ i ].length();
            }
         }

         parameters[ "msg" ] = QString( "%1\t%2\t    N  Start point  C   P-value\n" )
            .arg( "File", -use_names_max_len )
            .arg( "File", -use_names_max_len )
            // + "\tAdj P-Value"            
            ;

         progress->reset();
         int pp = 0;

         vector < double > undecimated_q = q;
         int q_points = (int) q.size();
         bool do_q_decimate = !files_are_time;
         int use_decimate = decimate ? decimate : 1;
         if ( do_q_decimate ) {
            // us_qdebug( QString( "do_q_decimate use_decimate %1 q_points %2" ).arg( use_decimate ).arg( q_points ) );
            vector < double > new_q;
            for ( int k = 0; k < q_points; k += use_decimate ) {
               if ( q[ k ] <= cormap_maxq &&
                    q[ k ] >= cormap_minq ) {
                  new_q.push_back( q[ k ] );
               }
            }
            q = new_q;
         }

         // US_Vector::printvector2( QString( "cormap minq %1 maxq %2 q undecimated_q" ).arg( cormap_minq ).arg( cormap_maxq ), q, undecimated_q );

         for ( int i = 0; i < fcount - 1; ++i ) {
            I[ 0 ] = f_Is[ use_selected_files[ i ] ];

            if ( do_q_decimate ) {
               vector < double > new_I;
               for ( int k = 0; k < q_points; k += use_decimate ) {
                  if ( undecimated_q[ k ] <= cormap_maxq &&
                       undecimated_q[ k ] >= cormap_minq ) {
                     new_I.push_back( I[ 0 ][ k ] );
                  }
               }
               I[ 0 ] = new_I;
            }

            // US_Vector::printvector2( QString( "i %1" ).arg( i ), q, I[0] );

            for ( int j = i + 1; j < fcount; ++j ) {
               progress->setValue( pp++ ); progress->setMaximum( m );
               qApp->processEvents();

               I[ 1 ] = f_Is[ use_selected_files[ j ] ];
               if ( do_q_decimate ) {
                  vector < double > new_I;
                  for ( int k = 0; k < q_points; k += use_decimate ) {
                     if ( undecimated_q[ k ] <= cormap_maxq &&
                          undecimated_q[ k ] >= cormap_minq ) {
                        new_I.push_back( I[ 1 ][ k ] );
                     }
                  }
                  I[ 1 ] = new_I;
               }

               // US_Vector::printvector2( QString( "default for cormap i %1 j %2 I" ).arg( i ).arg( j ), I[ 0 ], I[ 1 ] );

               if ( !usu->cormap( q, I, rkl, N, S, C, P ) ) {
                  editor_msg( "red", usu->errormsg );
               }
               double adjP = (double) m * P;
               if ( adjP > 1e0 ) {
                  adjP = 1e0;
               }
               pvaluepairs[ i ][ j ] = P;
               pvaluepairs[ j ][ i ] = P;
               adjpvaluepairs[ i ][ j ] = adjP;
               adjpvaluepairs[ j ][ i ] = adjP;

               parameters[ "msg" ] += 
                  QString( "%1\t%2\t%3\t%4\t%5\t%6"
                           // "\t%7"
                           "\n" )
                  .arg( use_selected_files[ i ], -use_names_max_len )
                  .arg( use_selected_files[ j ], -use_names_max_len )
                  .arg( N, 6 )
                  .arg( S, 6 )
                  .arg( C, 6 )
                  .arg( QString( "" ).sprintf( "%.4g", P ).leftJustified( 12 ) )
                  // .arg( adjP ) 
                  ;
            }
         }

         progress->reset();

         {
            parameters[ "title" ]     = 
               QString( us_tr( "Maximum q limit is %1 [A^-1]." ) )
               .arg( cormap_maxq );

            parameters[ "ppvm_title"     ] = us_tr( "Pairwise P value map" );
            parameters[ "ppvm_title_adj" ] = us_tr( "Pairwise adjusted P value map" );

            parameters[ "title_adj" ] = parameters[ "title" ];

            if ( cormap_minq > 0e0 ) {
               parameters[ "title"     ] += QString( " Minimum q limit is %1 [A^-1]." ).arg( cormap_minq );
               parameters[ "title_adj" ] += QString( " Minimum q limit is %1 [A^-1]." ).arg( cormap_minq );
            }

            if ( parameters.count( "decimate" ) ) {
               int decimate = parameters[ "decimate" ].toInt();
               if ( decimate > 1 ) {
                  QString nth_tag;
                  switch ( decimate ) {
                  case 2 : nth_tag = "nd"; break;
                  case 3 : nth_tag = "rd"; break;
                  default : nth_tag = "th"; break;
                  }
               
                  parameters[ "title" ] += QString( us_tr( " Only every %1%2 q value sampled." ) )
                     .arg( decimate ).arg( nth_tag );
                  parameters[ "title_adj" ] += QString( us_tr( " Only every %1%2 q value sampled." ) )
                     .arg( decimate ).arg( nth_tag );
               }
            }

            parameters[ "adjusted" ] = "true";
            parameters[ "linewisesummary" ] = "true";
            parameters[ "clusteranalysis"   ] = "true";

            parameters[ "global_width" ] = "1000";
            parameters[ "global_height" ] = "700";
            parameters[ "image_height" ] = "300";
            // parameters[ "name" ]         = use_selected_files.front();
            {
               QStringList qsl = vector_qstring_to_qstringlist( use_selected_files );
               QString head = qstring_common_head( qsl, true );
               QString tail = qstring_common_tail( qsl, true );
               parameters[ "name" ] = QString( "%1_%2%3_%4%5_cqmx%6" )
                  .arg( head )
                  .arg( tail )
                  .arg( parameters.count( "decimate" ) ? QString( "_s%1" ).arg( parameters[ "decimate" ] ) : QString( "" ) )
                  .arg( files_are_time ? "q" : "f" )
                  .arg( pvaluepairs.size() )
                  .arg( cormap_maxq )
                  .replace( ".", "_" )
                  ;
            }

            US_Hydrodyn_Saxs_Cormap * uhcm = new US_Hydrodyn_Saxs_Cormap( us_hydrodyn,
                                                                          parameters,
                                                                          pvaluepairs,
                                                                          adjpvaluepairs,
                                                                          use_selected_files );
            if ( parameters.count( "close" ) ) {
               delete uhcm;
            } else {
               uhcm->show();
            }
         }
         update_enables();
      }
      break;
   }
}

void US_Hydrodyn_Dad::bin()
{
   QStringList files = all_selected_files();
   bin( files );
}

void US_Hydrodyn_Dad::bin( QStringList files )
{
   bool ok;
   int binning = US_Static::getInteger(
                                            us_tr( "SOMO: HPLC enter binning" ),
                                            us_tr( "Enter the number of points of binning:" ),
                                            2, 
                                            2,
                                            10,
                                            1, 
                                            &ok, 
                                            this 
                                            );
   if ( !ok ) {
      return;
   }

   map < QString, bool > current_files;
   for ( int i = 0; i < (int)lb_files->count(); i++ )
   {
      current_files[ lb_files->item( i )->text() ] = true;
   }

   map < QString, bool > select_files;

   US_Saxs_Util usu;
   vector < double > binned_q;
   vector < double > binned_I;
   vector < double > binned_e;

   for ( unsigned int i = 0; i < ( unsigned int ) files.size(); i++ )
   {
      int ext = 0;
      QString binned_name = files[ i ] + QString( "-bin%1" ).arg( binning );
      while ( current_files.count( binned_name ) )
      {
         binned_name = files[ i ] + QString( "-bin%1-%2" ).arg( binning ).arg( ++ext );
      }

      if ( usu.bin( f_qs[ files[ i ] ], 
                    f_Is[ files[ i ] ], 
                    f_errors[ files[ i ] ], 
                    binned_q, 
                    binned_I, 
                    binned_e, 
                    binning ) )
      {
         select_files[ binned_name ] = true;

         lb_created_files->addItem( binned_name );
         lb_created_files->scrollToItem( lb_created_files->item( lb_created_files->count() - 1 ) );
         lb_files->addItem( binned_name );
         lb_files->scrollToItem( lb_files->item( lb_files->count() - 1 ) );
         created_files_not_saved[ binned_name ] = true;
   
         f_pos       [ binned_name ] = f_qs.size();

         {
            vector < QString > qs_string;
            for ( int j = 0; j < (int) binned_q.size(); ++j ) {
               qs_string.push_back( QString( "%1" ).arg( binned_q[ j ] ) );
            }
            f_qs_string [ binned_name ] = qs_string;
         }

         f_qs        [ binned_name ] = binned_q;
         f_Is        [ binned_name ] = binned_I;
         f_errors    [ binned_name ] = binned_e;
         f_is_time   [ binned_name ] = f_is_time  [ files[ i ] ];
         f_conc      [ binned_name ] = f_conc.count( files[ i ] ) ? f_conc[ files[ i ] ] : 0e0;
         f_psv       [ binned_name ] = f_psv .count( files[ i ] ) ? f_psv [ files[ i ] ] : 0e0;
         f_I0se      [ binned_name ] = f_I0se .count( files[ i ] ) ? f_I0se [ files[ i ] ] : 0e0;
         {
            vector < double > tmp;
            f_gaussians  [ binned_name ] = tmp;
         }
         editor_msg( "gray", QString( "Created %1\n" ).arg( binned_name ) );
      } else {
         editor_msg( "red", QString( "Error: binning error trying to create %1\n" ).arg( binned_name ) );
      }
   }

   disable_updates = true;

   lb_files->clearSelection();
   for ( int i = 0; i < (int)lb_files->count(); i++ )
   {
      if ( select_files.count( lb_files->item( i )->text() ) )
      {
         lb_files->item( i )->setSelected( true );
      }
   }

   disable_updates = false;
   plot_files();

   update_enables();
}

void US_Hydrodyn_Dad::bb_cm_inc()
{
   // us_qdebug( "bb_cm_inc" );

   map < QString, QString > save_parameters;
   map < QString, QString > parameters;

   bool slide = false;
   bool increment = false;

   switch ( QMessageBox::question(this, 
                                 windowTitle() + us_tr( " : PVP analysis auto increment" )
                                 ,us_tr( 
                                     "Choose your options for automatic running"
                                     )
                                 ,us_tr( "&Slide current window forward" )
                                 ,us_tr( "&Increment window size" )
                                 ,us_tr( "&Cancel" )
                                 ,0 // 
                                 ,-1 // 
                                 ) )
   {
   case -1 : // escape
      return;
      break;
   case 0 : // 
      slide = true;
      break;
   case 1 : // 
      increment = true;
      break;
   case 2 : // cancel
      return;
      break;
   }

   save_parameters[ "close" ] = "true";

   QString fn = QFileDialog::getSaveFileName( this , us_tr( "Choose a file name for the images" ) , QString() , "png (*.png *.PNG)" );

   if ( !fn.isEmpty() ) {
      fn = fn.replace( QRegExp( ".png$", Qt::CaseInsensitive ), "" );
   }
   
   save_parameters[ "decimate" ] = "0";
   ask_to_decimate( save_parameters );

   double start = le_baseline_end_s      ->text().toDouble();
   double end   = le_baseline_end_e      ->text().toDouble();
   double o_start = start;
   double o_end   = end;

   // double min   = f_qs[ wheel_file ].front();
   double max   = f_qs[ wheel_file ].back();

   // us_qdebug( QString( "start %1 end %2 min %3 max %4" ).arg( start ).arg( end ).arg( min ).arg( max ) );

   while ( end <= max ) {
      le_baseline_end_s->setText( QString( "%1" ).arg( start ) );
      le_baseline_end_e->setText( QString( "%1" ).arg( end ) );
      // us_qdebug( QString( "run test for start %1 end %2" ).arg( start ).arg( end ) );
      parameters = save_parameters;
      if ( !fn.isEmpty() ) {
         parameters[ "save_png" ] = fn + QString( "_fr%1_%2" ).arg( start ).arg( end ) + ".png";
         parameters[ "save_csv" ] = fn + ".csv";
      }
      cormap( parameters );
      save_parameters[ "csv_skip_report_header" ] = "true";
      if ( slide ) {
         start++;
         end++;
      }
      if ( increment ) {
         end++;
      }
   }      

   le_baseline_end_s->setText( QString( "%1" ).arg( o_start ) );
   le_baseline_end_e->setText( QString( "%1" ).arg( o_end ) );
}

bool US_Hydrodyn_Dad::blanks_params()
{
   us_qdebug( "blanks_params" );
   map < QString, QString > save_parameters;
   map < QString, QString > parameters;

   save_parameters[ "close"    ] = "true";

   save_parameters[ "decimate" ] = "0";
   ask_to_decimate( save_parameters );

   double o_start = le_baseline_end_s      ->text().toDouble();
   double o_end   = le_baseline_end_e      ->text().toDouble();

   int points = (int) f_qs[ wheel_file ].size();

   us_qdebug( QString( "blanks_params points %1" ).arg( points ) );

   // int points_start = 10; 
   // int points_end   = 50;
   // int points_inc   = 1;

   int points_start = 10; 
   int points_end   = 50;
   int points_inc   = 5;

   if ( points < 10 ) {
      QMessageBox::warning( this, 
                            windowTitle() + us_tr( " : Analyze blanks" ),
                            QString( us_tr( "To properly analyze blanks requires a minimum of 10 blank frames.\n"
                                         "Optimally, 80 blanks should be available for this analysis\n" ) ),
                            QMessageBox::Ok,
                            QMessageBox::NoButton );
      return false;
   }

   for ( int pts = points_start; pts <= points && pts <= points_end; pts += points_inc ) {
      double start = f_qs[ wheel_file ].front();
      double end   = f_qs[ wheel_file ][ pts - 1 ];
      int pos = 0;
      us_qdebug( QString( "blanks_params pts %1 start %2 end %3" ).arg( pts ).arg( start ).arg( end ) );
      
      while ( pts + pos - 1 < points ) {
         start = f_qs[ wheel_file ][ pos ];
         end   = f_qs[ wheel_file ][ pos + pts - 1 ];
         le_baseline_end_s->setText( QString( "%1" ).arg( start ) );
         le_baseline_end_e->setText( QString( "%1" ).arg( end ) );
         // us_qdebug( QString( "run test for start %1 end %2" ).arg( start ).arg( end ) );
         parameters = save_parameters;
         parameters[ "data_csv" ] = QString( "%1" ).arg( pts );
         // if ( !fn.isEmpty() ) {
         //    parameters[ "save_png" ] = fn + QString( "_fr%1_%2" ).arg( start ).arg( end ) + ".png";
         //    parameters[ "save_csv" ] = fn + ".csv";
         // }
         cormap( parameters );
         save_parameters[ "csv_skip_report_header" ] = "true";
         pos += 10;
      }      
   }

   le_baseline_end_s->setText( QString( "%1" ).arg( o_start ) );
   le_baseline_end_e->setText( QString( "%1" ).arg( o_end ) );

   map < QString, map < QString, vector < QString > > > data_csv         = ((US_Hydrodyn *)us_hydrodyn)->data_csv;
   QStringList                                          data_csv_headers = ((US_Hydrodyn *)us_hydrodyn)->data_csv_headers;

   cout << "data summary----------\n";

   for ( int i = 0; i < (int) data_csv_headers.size(); ++i ) {
      cout << data_csv_headers[ i ] << endl;
      if ( data_csv.count( data_csv_headers[ i ] ) ) {
         for ( map < QString, vector < QString > >::iterator it = data_csv[ data_csv_headers[ i ] ].begin();
               it != data_csv[ data_csv_headers[ i ] ].end();
               it++ ) {
            cout << QString( "   %1:\n" ).arg( it->first );
            for ( int j = 0; j < (int) it->second.size(); ++j ) {
               cout << QString( "    %1\n" ).arg( it->second[ j ] );
            }
         }
      }
   }

   return true;
}

QStringList US_Hydrodyn_Dad::vector_qstring_to_qstringlist( vector < QString > vqs ) {
   QStringList qsl;
   for ( int i = 0; i < (int) vqs.size(); ++i ) {
      qsl << vqs[ i ];
   }
   return qsl;
}

void US_Hydrodyn_Dad::simulate() {
   us_qdebug( "simulate" );

   QStringList selected = all_selected_files();

   map < QString, vector < double > > I;
   map < QString, vector < double > > q;
   map < QString, vector < double > > e;

   {
      vector < vector < double > > grids;

      for ( int i = 0; i < (int) selected.size(); ++i ) {
         QString this_file = selected[ i ];
         if ( f_qs.count( this_file ) &&
              f_Is.count( this_file ) &&
              f_qs[ this_file ].size() &&
              f_Is[ this_file ].size() ) {
            grids.push_back( f_qs[ this_file ] );
         }
      }

      vector < double > v_union = US_Vector::vunion( grids );
      vector < double > v_int   = US_Vector::intersection( grids );

      bool any_differences = v_union != v_int;

      if ( any_differences )
      {
         editor_msg( "red", us_tr( "Simulate: curves must be on the same grid, try 'Crop Common' first." ) );
         update_enables();
         return;
      }
   }

   for ( int i = 0; i < (int) selected.size(); ++i ) {
      QString this_file = selected[ i ];
      I[ this_file ] = f_Is[ this_file ];
      q[ this_file ] = f_qs[ this_file ];
      e[ this_file ] = f_errors[ this_file ];
   }

   US_Hydrodyn_Dad_Simulate * uhshs = new US_Hydrodyn_Dad_Simulate( 
                                                                               us_hydrodyn,
                                                                               this,
                                                                               selected,
                                                                               q,
                                                                               I,
                                                                               e
                                                                                );
   uhshs->show();
}

void US_Hydrodyn_Dad::gauss_mode()
{
   map < QString, QString > parameters;

   if ( U_EXPT )
   {
      parameters[ "expert_mode" ] = "true";
   }
   
   
   parameters[ "gaussian_type" ] = QString( "%1" ).arg( gaussian_type );
   US_Hydrodyn_Dad_Gauss_Mode *sho = 
      new US_Hydrodyn_Dad_Gauss_Mode( & parameters, (US_Hydrodyn *) us_hydrodyn, this );
   US_Hydrodyn::fixWinButtons( sho );
   sho->exec();
   delete sho;

   if ( !parameters.count( "ok" ) )
   {
      return;
   }

   if ( gaussian_type != (gaussian_types)( parameters[ "gaussian_type" ].toInt() ) )
   {
      gaussian_type = (gaussian_types)( parameters[ "gaussian_type" ].toInt() );
      unified_ggaussian_ok = false;
      f_gaussians.clear( );
      gaussians.clear( );
      org_gaussians.clear( );
      org_f_gaussians.clear( );
      update_gauss_mode();
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "dad_gaussian_type" ] = QString( "%1" ).arg( gaussian_type );
   }

   update_enables();
}

#define TSO QTextStream(stdout)

bool US_Hydrodyn_Dad::dad_load( const QString & filename, const QStringList & qsl, QString & errormsg ) {
   errormsg = "";
   
   if ( !qsl.size() ) {
      errormsg = "not a valid dad file";
      return false;
   }

   // check if qsl is a dad data file, if not return false

   vector < vector < double > > absorption_data;

   {
      set < size_t > row_sizes;

      for ( auto const & line : qsl ) {
         vector < double > absorption_data_row;
         QStringList lineqsl = line.split( QRegularExpression( "[\t ,]" ) );
         for ( auto const & element : lineqsl ) {
            bool isNumeric = false;
            double absorption = element.toDouble( &isNumeric );
            if ( !isNumeric ) {
               errormsg = "not a valid dad file";
               return false;
            }
            absorption_data_row.push_back( absorption );
         }
         absorption_data.push_back( absorption_data_row );
         row_sizes.insert( absorption_data_row.size() );
      }

      if ( row_sizes.size() != 1 ) {
         errormsg = "not a valid dad file";
         return false;
      }
   }   

   size_t absorption_row_size = absorption_data.front().size();
      
   // apparently a valid dad file
   if ( !dad_lambdas.lambdas.size() ) {
      errormsg = QString( UNICODE_LAMBDA_QS + "s must be loaded prior to loading of UV-Vis absorption data" );
      return false;
   }

   if ( dad_lambdas.lambdas.size() != absorption_row_size ) {
      errormsg = QString( "UV-Vis absorption column count %1 does not match number of " + UNICODE_LAMBDA_QS + "s %2" )
         .arg( dad_lambdas.lambdas.size() )
         .arg( absorption_row_size )
         ;
      return false;
   }

   switch ( QMessageBox::question(this, 
                                  windowTitle() + us_tr( " : Load UV-Vis Data" )
                                  ,dad_lambdas.summary_rich()
                                  + QString(
                                            "<hr>"
                                            "Proceed with these UV_Vis " + UNICODE_LAMBDA_QS + "s?"
                                            )
                                  ) )
   {
   case QMessageBox::Yes : 
      break;
   default:
      errormsg = "UV-Vis " + UNICODE_LAMBDA_QS + "s need to be loaded";
      return false;
      break;
   }


   // enter starting / ending times

   double start_time_seconds          = 0;
   double collection_interval_seconds = 1;
   
   double lambda_start = 250; // dad_lambdas.lambdas.front();
   double lambda_end   = 600; // dad_lambdas.lambdas.back();

   // get lambda range
   
   bool lambda_crop;

   {
      bool try_again = false;

      do {
         lambda_crop = false;
         QDialog dialog(this);
         dialog.setWindowTitle( windowTitle() + us_tr( ": UV-Vis load : subselect " + UNICODE_LAMBDA_QS + " range" ) );
         dialog.setMinimumWidth( 200 );

         QFormLayout form(&dialog);

         // Add some text above the fields
         form.addRow( new QLabel(
                                 QString( us_tr( 
                                                "Fill out the values below and click OK to subselect\n"
                                                "or CANCEL to use the full spectrum\n"
                                                "Full %1 range is [%2:%3] [nm]\n"
                                                 ) )
                                 .arg( UNICODE_LAMBDA )
                                 .arg( dad_lambdas.lambdas.front() )
                                 .arg( dad_lambdas.lambdas.back() )
                                 ) );

         // Add the lineEdits with their respective labels
         QList<QWidget *> fields;
   
         vector < QString > labels =
            {
               us_tr( "Start " + UNICODE_LAMBDA_QS + " [nm]:" )
               ,us_tr( "End " + UNICODE_LAMBDA_QS + " [nm]:" )
            };
      
         vector < QWidget * > widgets =
            {
               new QLineEdit( &dialog )
               ,new QLineEdit( &dialog )
            };

         vector < double >  defaults =
            {
               lambda_start
               ,lambda_end
            };

         for( int i = 0; i < (int) widgets.size(); ++i ) {
            form.addRow( labels[i], widgets[i] );
            if ( widgets[i] ) {
               // could switch based on widgets[i]->className()
               // assuming all input fields are doubles for now
               ((QLineEdit *)widgets[i])->setValidator( new QDoubleValidator(this) );
               ((QLineEdit *)widgets[i])->setText( QString( "%1" ).arg( defaults[i] ) );
               fields << widgets[i];
            }
         }

         // Add some standard buttons (Cancel/Ok) at the bottom of the dialog
         QDialogButtonBox buttonBox(
                                    QDialogButtonBox::Ok | QDialogButtonBox::Cancel
                                    ,Qt::Horizontal
                                    ,&dialog
                                    );
         form.addRow(&buttonBox);
         QObject::connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
         QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

         // Show the dialog as modal
         if (dialog.exec() == QDialog::Accepted) {
            if ( ((QLineEdit *)fields[0])->text().toDouble() > ((QLineEdit *)fields[1])->text().toDouble() 
                 ) {
               try_again = true;
            } else {
               // If the user didn't dismiss the dialog, do something with the fields
               lambda_start  = ((QLineEdit *)fields[0])->text().toDouble();
               lambda_end    = ((QLineEdit *)fields[1])->text().toDouble();
               TSO <<
                  QString(
                          "lambda_start %1\n"
                          "lambda_end   %2\n"
                          )
                  .arg( lambda_start )
                  .arg( lambda_end )
                  ;
               lambda_crop = true;
            }
         } else {
            lambda_start = dad_lambdas.lambdas.front();
            lambda_end   = dad_lambdas.lambdas.back();
         }
      } while ( try_again );
   }

   // get times

   {
      bool try_again = false;

      do {
         QDialog dialog(this);
         dialog.setWindowTitle( windowTitle() + us_tr( ": UV-Vis load : Set times" ) );
         dialog.setMinimumWidth( 200 );

         QFormLayout form(&dialog);

         // Add some text above the fields
         form.addRow( new QLabel(
                                 us_tr(
                                       "Fill out the values below and click OK\n"
                                       )
                                 ) );

         // Add the lineEdits with their respective labels
         QList<QWidget *> fields;
   
         vector < QString > labels =
            {
               us_tr( "Start time [s]:" )
               ,us_tr( "Collection interval [s]:" )
            };
      
         vector < QWidget * > widgets =
            {
               new QLineEdit( &dialog )
               ,new QLineEdit( &dialog )
            };

         vector < double >  defaults =
            {
               start_time_seconds
               ,collection_interval_seconds
            };

         for( int i = 0; i < (int) widgets.size(); ++i ) {
            form.addRow( labels[i], widgets[i] );
            if ( widgets[i] ) {
               // could switch based on widgets[i]->className()
               // assuming all input fields are doubles for now
               ((QLineEdit *)widgets[i])->setValidator( new QDoubleValidator(this) );
               ((QLineEdit *)widgets[i])->setText( QString( "%1" ).arg( defaults[i] ) );
               fields << widgets[i];
            }
         }

         // Add some standard buttons (Cancel/Ok) at the bottom of the dialog
         QDialogButtonBox buttonBox(
                                    QDialogButtonBox::Ok | QDialogButtonBox::Cancel
                                    ,Qt::Horizontal
                                    ,&dialog
                                    );
         form.addRow(&buttonBox);
         QObject::connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
         QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

         // Show the dialog as modal
         if (dialog.exec() == QDialog::Accepted) {
            if ( ((QLineEdit *)fields[1])->text().toDouble() == 0 ) {
               try_again = true;
            } else {
               // If the user didn't dismiss the dialog, do something with the fields
               start_time_seconds             = ((QLineEdit *)fields[0])->text().toDouble();
               collection_interval_seconds    = ((QLineEdit *)fields[1])->text().toDouble();
               TSO <<
                  QString(
                          "start time seconds          %1\n"
                          "collection_interval_seconds %2\n"
                          )
                  .arg( start_time_seconds )
                  .arg( collection_interval_seconds )
                  ;
            }
         } else {
            errormsg = "UV-Vis load cancelled";
            return false;
         }
      } while ( try_again );
   }

   // create files

#if 0 // create A(Lambda)
   {
      QString basename = QFileInfo( filename ).baseName();
      size_t absorption_data_size = absorption_data.size();
      
      for ( size_t i = 0; i < absorption_data_size; ++i ) {
         if ( !(i % 20) ) {
            progress->setValue( i ); progress->setMaximum( absorption_data_size );
            qApp->processEvents();
         }
         double time = start_time_seconds + i * collection_interval_seconds;
         QString name = QString( "%1_UV-Vis_AL_t%2" ).arg( basename ).arg( time ).replace( QRegularExpression( "[. &]" ), "_" );
         if ( lambda_crop ) {
            vector < double > x_cropped;
            vector < double > y_cropped;
            QString errormsg;
            if ( US_Saxs_Util::crop(
                                    dad_lambdas.lambdas
                                    ,absorption_data[ i ]
                                    ,lambda_start
                                    ,lambda_end
                                    ,x_cropped
                                    ,y_cropped
                                    ,errormsg
                                    ) ) {
               add_plot( name, x_cropped, y_cropped, false, false );
            } else {
               QMessageBox::critical( this,
                                      windowTitle() + us_tr( ": " ),
                                      QString( 
                                              us_tr(
                                                    "cropping error " + UNICODE_LAMBDA_QS + " for %1 to [%2,%3]\n%4"
                                                    )
                                               )
                                      .arg( name )
                                      .arg( lambda_start )
                                      .arg( lambda_end )
                                      .arg( errormsg )
                                      );
            }               
         } else {
            add_plot( name, dad_lambdas.lambdas, absorption_data[ i ], false, false );
         }
      }
   }
#endif

   // create A(t)
   {
      QString basename = QFileInfo( filename ).baseName();
      size_t absorption_data_size = absorption_data.size();
      
      // build cropped data
      
      vector < double > lambdas = dad_lambdas.lambdas;
      vector < double > times;

      if ( lambda_crop ) {
         for ( size_t i = 0; i < absorption_data_size; ++i ) {
            vector < double > x_cropped;
            vector < double > y_cropped;

            if ( !US_Saxs_Util::crop(
                                    dad_lambdas.lambdas
                                    ,absorption_data[ i ]
                                    ,lambda_start
                                    ,lambda_end
                                    ,x_cropped
                                    ,y_cropped
                                    ,errormsg
                                    ) ) {
      
               QMessageBox::critical( this,
                                      windowTitle() + us_tr( ": " ),
                                      QString( 
                                              us_tr(
                                                    "cropping error " + UNICODE_LAMBDA_QS + " for %1 to [%2,%3]\n%4"
                                                    )
                                               )
                                      .arg( basename )
                                      .arg( lambda_start )
                                      .arg( lambda_end )
                                      .arg( errormsg )
                                      );
               progress->reset();
               plot_files();
               update_enables();

               return false;
            }
            if ( !i ) {
               lambdas = x_cropped;
            }
            absorption_data[ i ] = y_cropped;
            times.push_back( start_time_seconds + i * collection_interval_seconds );
         }
      } else {
         for ( size_t i = 0; i < absorption_data_size; ++i ) {
            times.push_back( start_time_seconds + i * collection_interval_seconds );
         }
      }
      
      vector < vector < double > > absorption_data_by_lambda;
      {
         QString errormsg;
         if ( !US_Saxs_Util::transpose_vvd( absorption_data, absorption_data_by_lambda, errormsg ) ) {
            QMessageBox::critical( this,
                                   windowTitle() + us_tr( ": " ),
                                   QString( 
                                           us_tr(
                                                 "transpose error %1"
                                                 )
                                            )
                                   .arg( errormsg )
                                   );
            progress->reset();

            plot_files();
            update_enables();

            return false;
         }
      }
      
      size_t lambda_size = lambdas.size();

      for ( size_t i = 0; i < lambda_size; ++i ) {
         if ( !(i % 20) ) {
            progress->setValue( i ); progress->setMaximum( lambda_size );
            qApp->processEvents();
         }
         QString name = QString( "%1_UV-Vis_At_L%2" ).arg( basename ).arg( lambdas[ i ] ).replace( QRegularExpression( "[. &]" ), "_" );
         add_plot( name, times, absorption_data_by_lambda[ i ], true, false );
      }
   }

   progress->reset();

   plot_files();
   update_enables();

   return true;
}

// bool US_Hydrodyn_Dad::dad_angles_save() {
//    QString errormsg = "";
//    if ( dad_angles.dad_angle.size() ) {
//       QString use_dir = QDir::current().canonicalPath();
//       QString use_filename = QFileDialog::getSaveFileName( this , us_tr( "Select a file name for saving the UV-Vis Angles" ),  use_dir , "*.csv" );

//       if ( use_filename.isEmpty() ) {
//          return false;
//       }

//       use_filename.replace( QRegularExpression( "\\.csv$", QRegularExpression::CaseInsensitiveOption ), "" );
//       use_filename += ".csv";
   
//       if ( QFile::exists( use_filename ) ){
//          use_filename = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck( use_filename, 0, this );
//          raise();
//       }
      
//       if ( !dad_angles.save( use_filename, errormsg ) ) {
//          QMessageBox::warning( this, 
//                                windowTitle(),
//                                QString(
//                                        us_tr(
//                                              "Error saving UV-Vis Angles file %1\n"
//                                              "Error : %2\n"
//                                              )
//                                        )
//                                .arg( use_filename )
//                                .arg( errormsg )
//                                );
//          return false;
//       }
//       editor_msg( "black", QString( us_tr( "Saved: %1\n" ) ).arg( use_filename ) );
//       return true;
//    }
//    errormsg = us_tr( "No Angles loaded" );
//    return false;
// }

void US_Hydrodyn_Dad::conc_info( const QString & msg ) {
   TSO
      << "============================================================\n"
      << msg << "\n"
      << "------------------------------------------------------------\n"
      ;
   
   TSO << US_Vector::qs_mapqsdouble( "f_conc",       f_conc );
   TSO << "------------------------------------------------------------\n";
   TSO << US_Vector::qs_mapqsqs    ( "f_conc_units", f_conc_units );
   {
      map < QString, double > tmpcc = current_concs( true );
      TSO << US_Vector::qs_mapqsdouble( "current_concs",       tmpcc );
   }
   TSO << "============================================================\n";
}

void US_Hydrodyn_Dad::dndc_info( const QString & msg ) {
   TSO
      << "============================================================\n"
      << msg << "\n"
      << "------------------------------------------------------------\n"
      ;
   
   TSO << US_Vector::qs_mapqsdouble( "f_dndc",       f_dndc );
   TSO << "-----------------------------------------------------------\n";
   TSO << US_Vector::qs_mapqsdouble( "f_g_dndc",     f_g_dndc );

   TSO << "============================================================\n";
}

void US_Hydrodyn_Dad::extc_info( const QString & msg ) {
   TSO
      << "============================================================\n"
      << msg << "\n"
      << "------------------------------------------------------------\n"
      ;
   
   TSO << US_Vector::qs_mapqsdouble( "f_extc",       f_extc );

   TSO << "============================================================\n";
}
