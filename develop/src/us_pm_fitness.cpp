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

   // this may be temporary:
   vector < pm_data * > v_pdata;

   for ( set < pm_point >::iterator it = model.begin();
         it != model.end();
         it++ )
   {
      if ( !pdata.count( *it ) )
      {
         tmp_pm_data.rtp[ 0 ] = sqrt ( (double) ( (double)it->x[ 0 ] * (double)it->x[ 0 ] +
                                                  (double)it->x[ 1 ] * (double)it->x[ 1 ] +
                                                  (double)it->x[ 2 ] * (double)it->x[ 2 ] ) );
         if ( tmp_pm_data.rtp[ 0 ] == 0e0 )
         {
            tmp_pm_data.rtp[ 1 ] = 0e0;
            tmp_pm_data.rtp[ 2 ] = 0e0;
         } else {
            tmp_pm_data.rtp[ 1 ] = acos ( (double) it->x[ 2 ] / tmp_pm_data.rtp[ 0 ] );

            if ( it->x[ 0 ] == 0 &&
                 it->x[ 1 ] == 0 )
            {
               tmp_pm_data.rtp[ 2 ] = 0e0;
            } else {               
               if ( it->x[ 0 ] < 0 )
               {
                  tmp_pm_data.rtp[ 2 ] = M_PI - asin( (double)it->x[ 1 ] / sqrt( (double) ( (double)it->x[ 0 ] * 
                                                                                            (double)it->x[ 0 ] +
                                                                                            (double)it->x[ 1 ] * 
                                                                                            (double)it->x[ 1 ] ) ) );
               } else {
                  if ( it->x[ 1 ] < 0 )
                  {
                     tmp_pm_data.rtp[ 2 ] = M_2PI + asin( (double)it->x[ 1 ] / sqrt( (double)it->x[ 0 ] * 
                                                                                     (double)it->x[ 0 ] +
                                                                                     (double)it->x[ 1 ] * 
                                                                                     (double)it->x[ 1 ] ) );
                  } else {
                     tmp_pm_data.rtp[ 2 ] = asin( (double) it->x[ 1 ] / sqrt( (double) it->x[ 0 ] * 
                                                                              (double) it->x[ 0 ] +
                                                                              (double) it->x[ 1 ] * 
                                                                              (double) it->x[ 1 ] ) );
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

   us_timers.start_timer( "combined" );
   for ( unsigned int j = 0; j < q_points; j++ )
   {
      I_result[ j ] = 0e0;
   }

   double  J;
   vector < vector < complex < float > > >  A( max_harmonics + 1 ); // A[ l ][ m ]
         
   us_timers.start_timer( "legendre" );
   // setup associated legendre functions

   vector < vector < vector < complex < float > > > > Y; // Y[atom][l][m]
   Y.resize( v_pdata.size() );

   for ( unsigned int i = 0; i < v_pdata.size(); i++ )
   {
      pm_data *tmp_pm_data = v_pdata[ i ];
      /*
      cout << QString( "point %1 r %2 t %3 p %4\n" )
         .arg( i )
         .arg( tmp_pm_data->rtp[ 0 ] )
         .arg( tmp_pm_data->rtp[ 1 ] )
         .arg( tmp_pm_data->rtp[ 2 ] )
         .ascii();
      */

      Y[ i ].resize( max_harmonics + 1 );

      for ( unsigned int l = 0; l <= max_harmonics; l++ )
      {
         unsigned int m_size = 1 + l * 2;
         Y[ i ][ l ].resize( m_size );

         for ( int m = - (int) l ; m <= (int) l; m++ )
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
            Y[ i ][ l ][ m + l ] = result;
         }
      }            
   }

   us_timers.end_timer( "legendre" );

   for ( unsigned int j = 0; j < q_points; j++ )
   {
      for ( unsigned int l = 0; l <= max_harmonics; l++ )
      {
         unsigned int m_size = 1 + l * 2;
         A[ l ].resize( m_size );
         for ( unsigned int m = 0; m < m_size; m++ )
         {
            A[ l ][ m ] = Z0;
         }
      }

      for ( unsigned int i = 0; i < (unsigned int) v_pdata.size(); i++ )
      {
         for ( unsigned int l = 0; l <= max_harmonics; l++ )
         {
            complex < float > i_pow_l = pow( i_, l );
            if ( !nr::sphbes( l, q[ j ] * v_pdata[ i ]->rtp[ 0 ], J ) )
            {
               error_msg = "nr::shbes failed";
               return false;
            }
            for ( int m = - (int) l ; m <= (int) l; m++ )
            {
               A[ l ][ m + l ] += (float) J * Y[ i ][ l ][ m + l ] * (float) F[ j ] * i_pow_l;
            }
         }
      }
      for ( unsigned int l = 0; l <= max_harmonics; l++ )
      {
         unsigned int m_size = 1 + l * 2;
         for ( unsigned int m = 0 ; m <= m_size; m++ )
         {
            I_result[ j ] += norm( A[ l ][ m ] );
         }
      }
      I_result[ j ] *= M_4PI;
   }
   us_timers.end_timer( "combined" );
   cout << "list times:\n" << us_timers.list_times().ascii() << endl << flush;
   
   return true;
}
