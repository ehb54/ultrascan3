#include "../include/us_hydrodyn.h"
#include "../include/us_hydrodyn_saxs_hplc.h"
// #include <assert.h>

bool US_Hydrodyn_Saxs_Hplc::unified_ggaussian_to_f_gaussians()
{
   if ( !unified_ggaussian_ok )
   {
      return false;
   }

   // cb_fix_width
   // cb_fix_dist1
   // cb_fix_dist2

   // layouts:
   // n is number of gaussians
   // layout of regular (file specific gaussians)
   // g=height,center,width,{dist1,{dist2}}
   // g1,g2,...,gn

   // layout of unified global gaussians
   // common portion (shared by all): f=center{,width}{,dist1}{,dist2}
   // f1,f2,...,fn
   // variable portion (per file): v=height{,width}{,dist1}{,dist2}
   // v1,v2,...,vn

   // these are both per gaussian:
   /* now handled in create_unified_ggaussian_target 
   unsigned int common_size   = 0;
   unsigned int per_file_size = 0;

   vector < bool >         is_common; // is common maps the offsets to layout of the regular file specific gaussians
   vector < unsigned int > offset;

   // height:
   is_common.push_back( false           );  // height always variable
   offset   .push_back( per_file_size++ );  // first variable entry

   // center
   is_common.push_back( true            );  // center always common
   offset   .push_back( common_size++   );  // first common entry

   // width
   if ( cb_fix_width->isChecked() )
   {
      is_common.push_back( true );
      offset   .push_back( common_size++   );  // first common entry
   } else {
      is_common.push_back( false );
      offset   .push_back( per_file_size++ );  // first variable entry
   }

   if ( dist1_active )
   {
      if ( cb_fix_dist1->isChecked() )
      {
         is_common.push_back( true );
         offset   .push_back( common_size++   );  // first common entry
      } else {
         is_common.push_back( false );
         offset   .push_back( per_file_size++ );  // first variable entry
      }
      if ( dist2_active )
      {
         if ( cb_fix_dist2->isChecked() )
         {
            is_common.push_back( true );
            offset   .push_back( common_size++   );  // first common entry
         } else {
            is_common.push_back( false );
            offset   .push_back( per_file_size++ );  // first variable entry
         }
      }
   }
   */

   US_Vector::printvector( "unified_ggaussian_to_f_gaussians is_common", is_common );
   US_Vector::printvector( "unified_ggaussian_to_f_gaussians offset"   , offset    );

   if ( common_size + per_file_size != (unsigned int)gaussian_type_size )
   {
      errormsg = QString( tr( "Internal error: unified_ggaussians_to_f_gaussians(): common_size %1 + per_file_size %2 != gaussian_type_size %3\n" ) )
         .arg( common_size )
         .arg( per_file_size )
         .arg( gaussian_type_size )
         ;
      editor_msg( "red", errormsg );
      return false;
   }

   for ( unsigned int i = 0; i < ( unsigned int ) unified_ggaussian_files.size(); i++ )
   {
      vector < double > g;
      unsigned int  index = common_size * unified_ggaussian_gaussians_size + i * per_file_size * unified_ggaussian_gaussians_size;

      for ( unsigned int j = 0; j < unified_ggaussian_gaussians_size; j++ )
      {
         for ( int k = 0; k < gaussian_type_size; ++k )
         {
            if ( is_common[ k ] )
            {
               g.push_back( unified_ggaussian_params[ offset[ k ] + common_size * j           ] );
            } else {
               g.push_back( unified_ggaussian_params[ offset[ k ] + per_file_size * j + index ] );
            }
         }               
      }
      f_gaussians[ unified_ggaussian_files[ i ] ] = g;
   }

   return true;
}

vector < double > US_Hydrodyn_Saxs_Hplc::compute_ggaussian_gaussian_sum()
{
   vector < double > result;
   if ( !unified_ggaussian_ok )
   {
      editor_msg( "red", tr( "Internal error: gaussian rmsd called but unified gaussians not ok" ) );
      return result;
   }

   // assert( unified_ggaussian_curves > 1 && "unified_ggaussian_curves > 1" );

   result.resize( unified_ggaussian_I.size() );

   // cout << QString( "cggs: unified_ggaussian_I.size() %1\n" ).arg( result.size() );

   vector < double > this_t;
   unsigned int      t_size;

   for ( unsigned int i = 0; i < unified_ggaussian_curves; ++i )
   {
      t_size = unified_ggaussian_q_end[ i ] - unified_ggaussian_q_start[ i ];
      this_t.resize( t_size );
      /*
      cout << QString( "cggs: curve %1 q start %2 q end %3 t_size %4\n" )
         .arg( i )
         .arg( unified_ggaussian_q_start[ i ] )
         .arg( unified_ggaussian_q_end[ i ] )
         .arg( t_size )
         ;         
      */
      for ( unsigned int t = 0; t < t_size; ++t )
      {
         this_t[ t ] = unified_ggaussian_q[ unified_ggaussian_q_start[ i ] + t ];
      }

      // build up g

      vector < double > g;
      unsigned int  index = common_size * unified_ggaussian_gaussians_size + i * per_file_size * unified_ggaussian_gaussians_size;

      for ( unsigned int j = 0; j < unified_ggaussian_gaussians_size; ++j )
      {
         for ( int k = 0; k < gaussian_type_size; ++k )
         {
            if ( is_common[ k ] )
            {
               g.push_back( unified_ggaussian_params[ offset[ k ] + common_size * j           ] );
            } else {
               g.push_back( unified_ggaussian_params[ offset[ k ] + per_file_size * j + index ] );
            }
         }               
      }
      
      // US_Vector::printvector( QString( "cggs: file %1, this_t" ).arg( i ), this_t );
      // US_Vector::printvector( QString( "cggs: file %1, g" ).arg( i ), g );
      vector < double > this_result = compute_gaussian_sum( this_t, g );

      for ( unsigned int t = 0; t < t_size; ++t )
      {
         result[ unified_ggaussian_q_start[ i ] + t ] = this_result[ t ];
      }
   }
   return result;
}

double US_Hydrodyn_Saxs_Hplc::ggaussian_rmsd()
{
   if ( !unified_ggaussian_ok )
   {
      editor_msg( "red", tr( "Internal error: gaussian rmsd called but unified gaussians not ok" ) );
      return 1e99;
   }

   vector < double > result = compute_ggaussian_gaussian_sum();

   double rmsd = 0e0;

   if ( unified_ggaussian_use_errors && cb_sd_weight->isChecked() )
   {
      for ( unsigned int i = 0; i < ( unsigned int ) result.size(); i++ )
      {
         double tmp = ( result[ i ] - unified_ggaussian_I[ i ] ) / unified_ggaussian_e[ i ];
         rmsd += tmp * tmp;
      }
   } else {
      for ( unsigned int i = 0; i < ( unsigned int ) result.size(); i++ )
      {
         double tmp = result[ i ] - unified_ggaussian_I[ i ];
         rmsd += tmp * tmp;
      }
   }

   //    printvector( "rmsd, ugq", unified_ggaussian_q );

   update_plot_errors( unified_ggaussian_t, unified_ggaussian_I, result, unified_ggaussian_e );
   plot_errors_jump_markers();

   return sqrt( rmsd );
}
