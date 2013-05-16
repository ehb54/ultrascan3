#include "../include/us_hydrodyn.h"
#include "../include/us_hydrodyn_saxs_hplc.h"

// note: this program uses cout and/or cerr and this should be replaced

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

   /* old way
      if ( cb_fix_width->isChecked() )
      {
      for ( unsigned int i = 0; i < ( unsigned int ) unified_ggaussian_files.size(); i++ )
      {
      vector < double > g;
      unsigned int  index = 2 * unified_ggaussian_gaussians_size + i * unified_ggaussian_gaussians_size;

      for ( unsigned int j = 0; j < unified_ggaussian_gaussians_size; j++ )
      {
      g.push_back( unified_ggaussian_params[ index + j + 0 ] );
      g.push_back( unified_ggaussian_params[ 2 * j + 0 ] );
      g.push_back( unified_ggaussian_params[ 2 * j + 1 ] );
      }
      f_gaussians[ unified_ggaussian_files[ i ] ] = g;
      }
      } else {
      for ( unsigned int i = 0; i < ( unsigned int ) unified_ggaussian_files.size(); i++ )
      {
      vector < double > g;
      unsigned int  index = unified_ggaussian_gaussians_size + i * 2 * unified_ggaussian_gaussians_size;

      for ( unsigned int j = 0; j < unified_ggaussian_gaussians_size; j++ )
      {
      g.push_back( unified_ggaussian_params[ index + 2 * j + 0 ] );
      g.push_back( unified_ggaussian_params[ j ] );
      g.push_back( unified_ggaussian_params[ index + 2 * j + 1 ] );
      }
      f_gaussians[ unified_ggaussian_files[ i ] ] = g;
      }
      }            
   */
   return true;
}
