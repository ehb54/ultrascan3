#include "../include/us_pm.h"

// this will have fitness routines
// sh etc

bool US_PM::compute_I( set < pm_point > & model, vector < double > &I_result )
{
   I_result.resize( q_points ); // this should already be true

   // model should already be centered

   us_timers.init_timer( "rtp" );
   us_timers.start_timer( "rtp" );

   pm_data tmp_pm_data;
   tmp_pm_data.no_J = true;
   tmp_pm_data.J.resize( J_points );
   tmp_pm_data.no_Y = true;
   tmp_pm_data.Y.resize( Y_points );
   // this may be temporary:
   vector < pm_data * > v_pdata;

   for ( set < pm_point >::iterator it = model.begin();
         it != model.end();
         it++ )
   {
      if ( !pdata.count( *it ) )
      {
         tmp_pm_data.x[ 0 ] = grid_conversion_factor * (double)it->x[ 0 ];
         tmp_pm_data.x[ 1 ] = grid_conversion_factor * (double)it->x[ 1 ];
         tmp_pm_data.x[ 2 ] = grid_conversion_factor * (double)it->x[ 2 ];

         tmp_pm_data.rtp[ 0 ] = sqrt ( (double) ( tmp_pm_data.x[ 0 ] * tmp_pm_data.x[ 0 ] +
                                                  tmp_pm_data.x[ 1 ] * tmp_pm_data.x[ 1 ] +
                                                  tmp_pm_data.x[ 2 ] * tmp_pm_data.x[ 2 ] ) );
         if ( tmp_pm_data.rtp[ 0 ] == 0e0 )
         {
            tmp_pm_data.rtp[ 1 ] = 0e0;
            tmp_pm_data.rtp[ 2 ] = 0e0;
         } else {
            tmp_pm_data.rtp[ 1 ] = acos ( tmp_pm_data.x[ 2 ] / tmp_pm_data.rtp[ 0 ] );

            if ( it->x[ 0 ] == 0 &&
                 it->x[ 1 ] == 0 )
            {
               tmp_pm_data.rtp[ 2 ] = 0e0;
            } else {               
               if ( it->x[ 0 ] < 0 )
               {
                  tmp_pm_data.rtp[ 2 ] = M_PI - asin( tmp_pm_data.x[ 1 ] / sqrt( (double) ( tmp_pm_data.x[ 0 ] * 
                                                                                            tmp_pm_data.x[ 0 ] +
                                                                                            tmp_pm_data.x[ 1 ] * 
                                                                                            tmp_pm_data.x[ 1 ] ) ) );
               } else {
                  if ( it->x[ 1 ] < 0 )
                  {
                     tmp_pm_data.rtp[ 2 ] = M_2PI + asin( tmp_pm_data.x[ 1 ] / sqrt( tmp_pm_data.x[ 0 ] * 
                                                                                     tmp_pm_data.x[ 0 ] +
                                                                                     tmp_pm_data.x[ 1 ] * 
                                                                                     tmp_pm_data.x[ 1 ] ) );
                  } else {
                     tmp_pm_data.rtp[ 2 ] = asin( tmp_pm_data.x[ 1 ] / sqrt( tmp_pm_data.x[ 0 ] * 
                                                                             tmp_pm_data.x[ 0 ] +
                                                                             tmp_pm_data.x[ 1 ] * 
                                                                             tmp_pm_data.x[ 1 ] ) );
                  }               
               }
            }
         }
         pdata[ *it ] = tmp_pm_data;
         /* 
         cout << QString( "xyz %1 %2 %3 rtp %4 %5 %6\n" )
            .arg( it->x[ 0 ] )
            .arg( it->x[ 1 ] )
            .arg( it->x[ 2 ] )
            .arg( tmp_pm_data.rtp[ 0 ] )
            .arg( tmp_pm_data.rtp[ 1 ] )
            .arg( tmp_pm_data.rtp[ 2 ] )
            ;
         */
      } 
      v_pdata.push_back( &pdata[ *it ] );
   }

   us_timers.end_timer( "rtp" );

   us_timers.init_timer( "legendre" );
   us_timers.init_timer( "combined" );
   us_timers.init_timer( "sphbes" );
   us_timers.init_timer( "sumA" );

   us_timers.start_timer( "combined" );
   for ( unsigned int j = 0; j < q_points; j++ )
   {
      I_result[ j ] = 0e0;
   }

   vector < complex < float > >  A = A0;
   // cache legendre

   us_timers.start_timer( "legendre" );
   // setup associated legendre functions

   for ( unsigned int i = 0; i < v_pdata.size(); ++i )
   {
      pm_data *tmp_pm_data = v_pdata[ i ];
      if ( tmp_pm_data->no_Y )
      {
         /*
           cout << QString( "point %1 r %2 t %3 p %4\n" )
           .arg( i )
           .arg( tmp_pm_data->rtp[ 0 ] )
           .arg( tmp_pm_data->rtp[ 1 ] )
           .arg( tmp_pm_data->rtp[ 2 ] )
           .ascii();
         */
         complex < float > *Yp = &( tmp_pm_data->Y[ 0 ] );

         for ( unsigned int l = 0; l <= max_harmonics; ++l )
         {
            for ( int m = - (int) l ; m <= (int) l; ++m )
            {
               complex < double > result;
               if ( !sh::conj_spherical_harmonic( l, 
                                                  m, 
                                                  tmp_pm_data->rtp[ 1 ],
                                                  tmp_pm_data->rtp[ 2 ],
                                                  result) )
               {
                  error_msg = "sh::spherical_harmonic failed";
                  return false;
               }
               (*Yp) = result;
               ++Yp;
            }
         }
         tmp_pm_data->no_Y = false;
      }
   }

   us_timers.end_timer( "legendre" );

   // cache spherical bessels

   us_timers.start_timer( "sphbes" );
   unsigned int J_ofs;
   for ( unsigned int i = 0; i < (unsigned int) v_pdata.size(); ++i )
   {
      if ( v_pdata[ i ]->no_J )
      {
         for ( unsigned int j = 0; j < q_points; ++j )
         {
            J_ofs = j * ( 1 + max_harmonics );
            for ( unsigned int l = 0; l <= max_harmonics; ++l )
            {
               complex < float > i_pow_l = pow( i_, l );

               if ( !nr::sphbes( l, q[ j ] * v_pdata[ i ]->rtp[ 0 ], v_pdata[ i ]->J[ J_ofs + l ] ) )
               {
                  error_msg = "nr::shbes failed";
                  return false;
               }
               v_pdata[ i ]->no_J = false;
            }
         }
      }
   }
   us_timers.end_timer( "sphbes" );

   us_timers.start_timer( "sumA" );
   double *Jp;

   complex < float > *i_lp;
   complex < float > *Ap = &( A[0] );
   
   for ( unsigned int j = 0; j < q_points; ++j )
   {
      J_ofs = j * ( 1 + max_harmonics );
      A = A0;

      for ( unsigned int i = 0; i < (unsigned int) v_pdata.size(); ++i )
      {
         pm_data *tmp_pm_data = v_pdata[ i ];
         complex < float > *Yp = &( tmp_pm_data->Y[ 0 ] );
         Jp = &( tmp_pm_data->J[ J_ofs ] );
         Ap = &( A[0] );
         i_lp = &( i_l[ 0 ] );
         for ( unsigned int l = 0; l <= max_harmonics; ++l )
         {
            // these could possible be cached:
            complex < float > tmp_c = (float) *Jp * (float) F[ j ] * (*i_lp);
            ++Jp;
            ++i_lp;
            for ( int m = - (int) l ; m <= (int) l; ++m )
            {
               (*Ap) +=  (*Yp) * tmp_c;
               ++Yp;
               ++Ap;
            }
         }
      }
      Ap = &( A[0] );
      for ( unsigned int k = 0; k < Y_points; ++k )
      {
         I_result[ j ] += norm( (*Ap) );
         ++Ap;
      }
      I_result[ j ] *= M_4PI;
   }
   us_timers.end_timer( "sumA" );
   us_timers.end_timer( "combined" );
   cout << "list times:\n" << us_timers.list_times().ascii() << endl << flush;
   
   return true;
}
