#define ALT_SH
#define ALT_SPHBES

#include "../include/us_pm.h"

// this will have fitness routines
// sh etc

// #define USE_TIMERS

bool US_PM::compute_I( set < pm_point > & model, vector < double > & I_result )
{
   if ( !model.size() )
   {
      error_msg = "empty model";
      return false;
   }
   if ( use_CYJ )
   {
      return compute_CYJ_I( model, I_result );
   } else {
      // really need to check in comparison with previously cached
      if ( model.size() > max_beads_CA )
      {
         if ( us_log )
         {
            us_log->log( QString( "switching to CYJ mode\n" ).toLatin1().data() );
         }
         pcdata.clear( );
         use_CYJ = true;
         return compute_CYJ_I( model, I_result );
      } else {
         return compute_CA_I( model, I_result );
      }
   }
}

double US_PM::model_fit( vector < double > & params, set < pm_point > & model, vector < double > & I_result )
{
   create_model( params, model );
   compute_I( model, I_result );
   return fitness2( I_result );
}

bool US_PM::compute_CYJ_I( set < pm_point > & model, vector < double > &I_result )
{

   // model should already be centered

#if defined( USE_TIMERS )
   us_timers.clear_timers();
   us_timers.init_timer( "rtp" );
   us_timers.start_timer( "rtp" );
#endif

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
            if ( us_log )
{
us_log->log( QString( "xyz %1 %2 %3 rtp %4 %5 %6\n" )
.arg( it->x[ 0 ] )
.arg( it->x[ 1 ] )
.arg( it->x[ 2 ] )
.arg( tmp_pm_data.rtp[ 0 ] )
.arg( tmp_pm_data.rtp[ 1 ] )
.arg( tmp_pm_data.rtp[ 2 ] )
);
}
         */
      } 
      v_pdata.push_back( &pdata[ *it ] );
   }

#if defined( USE_TIMERS )
   us_timers.end_timer( "rtp" );

   us_timers.init_timer( "legendre" );
   us_timers.init_timer( "combined" );
   us_timers.init_timer( "sphbes" );
   us_timers.init_timer( "sumA" );

   us_timers.start_timer( "combined" );
#endif

   I_result = I0;

   vector < complex < float > >  A = A0;
   // cache legendre

#if defined( USE_TIMERS )
   us_timers.start_timer( "legendre" );
#endif
   // setup associated legendre functions

   for ( unsigned int i = 0; i < v_pdata.size(); ++i )
   {
      pm_data *tmp_pm_data = v_pdata[ i ];
      if ( tmp_pm_data->no_Y )
      {
         /*
           if ( us_log )
{
us_log->log( QString( "point %1 r %2 t %3 p %4\n" )
.arg( i )
.arg( tmp_pm_data->rtp[ 0 ] )
.arg( tmp_pm_data->rtp[ 1 ] )
.arg( tmp_pm_data->rtp[ 2 ] )
.toLatin1().data() );
}
         */
         complex < float > *Yp = &( tmp_pm_data->Y[ 0 ] );
#if defined( ALT_SH )
         if ( !sh::alt_conj_sh( max_harmonics, 
                                tmp_pm_data->rtp[ 1 ],
                                tmp_pm_data->rtp[ 2 ],
                                Yp ) )
         {
            error_msg = "sh::spherical_harmonic failed";
            return false;
         }
#else
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
#endif
         tmp_pm_data->no_Y = false;
      }
   }

#if defined( USE_TIMERS )
   us_timers.end_timer( "legendre" );
   us_timers.start_timer( "sphbes" );
#endif
   // cache spherical bessels
   {
      double  qp_t_rtp0;
      // it's odd that the * version is a bit slower
      // double  *Jp;
#if !defined( ALT_SPHBES )
      unsigned int J_ofs;
#endif
      for ( unsigned int i = 0; i < (unsigned int) v_pdata.size(); ++i )
      {
         if ( v_pdata[ i ]->no_J )
         {
            v_pdata[ i ]->no_J = false;
            // Jp = &( v_pdata[ i ]->J[ 0 ] );
#if !defined( ALT_SPHBES )
            J_ofs = 0;
#endif
            for ( unsigned int j = 0; j < q_points; ++j )
            {
               // J_ofs = j * ( 1 + max_harmonics );
               qp_t_rtp0 = q[ j ] * v_pdata[ i ]->rtp[ 0 ];
#if defined( ALT_SPHBES )
               if ( !shs->shs_compute_sphbes( qp_t_rtp0, &( v_pdata[ i ]->J[ 0 ] ) ) )
               {
                  error_msg = "nr::alt_shbes failed";
                  return false;
               }
#else
               for ( unsigned int l = 0; l <= max_harmonics; ++l )
               {
                  if ( !nr::sphbes( l, qp_t_rtp0, v_pdata[ i ]->J[ J_ofs ] ) )
                  {
                     error_msg = "nr::shbes failed";
                     return false;
                  }
                  // ++Jp;
                  ++J_ofs;
               }
#endif
            }
         }
      }
   }
#if defined( USE_TIMERS )
   us_timers.end_timer( "sphbes" );
   us_timers.start_timer( "sumA" );
#endif

   double *Jp;

   complex < float > *i_lp;
   complex < float > *Ap = &( A[0] );
   
   unsigned int J_ofs;

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
#if defined( USE_TIMERS )
   us_timers.end_timer( "sumA" );
   us_timers.end_timer( "combined" );
   if ( us_log )
   {
      us_log->log( "list times:\n" + us_timers.list_times() );
   }
   msg_log += us_timers.list_times( QString( "CI %1 beads : " ).arg( model.size() ) );
#endif   
   return true;
}

bool US_PM::compute_delta_I( 
                            set < pm_point > &                        model, 
                            set < pm_point > &                        prev_model, 
                            vector < vector < complex < float > > > & Av,
                            vector < double > &                       I_result
                            )
{
#if defined( USE_TIMERS )
   us_timers.clear_timers();
#endif
   if ( !Av.size() || model.size() < 100 )
   {
      Av = Av0;
      prev_model.clear( );
   }

   // find elements that are in model not in new model and vice versa
   set < pm_point > model_adds;
   set < pm_point > model_subtracts;

   // find what we need to add
   set_difference( model.begin(), 
                   model.end(),
                   prev_model.begin(), 
                   prev_model.end(),
                   inserter( model_adds, model_adds.end() ) );

   // and what we need to subtract
   set_difference( prev_model.begin(), 
                   prev_model.end(),
                   model.begin(), 
                   model.end(),
                   inserter( model_subtracts, model_subtracts.end() ) );

   // model should already be centered

#if defined( USE_TIMERS )
   us_timers.init_timer( "dI:rtp" );
   us_timers.start_timer( "dI:rtp" );
#endif

   pm_data tmp_pm_data;
   tmp_pm_data.no_J = true;
   tmp_pm_data.J.resize( J_points );
   tmp_pm_data.no_Y = true;
   tmp_pm_data.Y.resize( Y_points );
   // this may be temporary:
   vector < pm_data * > v_pdata;

   // only adds are not already cached
   for ( set < pm_point >::iterator it = model_adds.begin();
         it != model_adds.end();
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
            if ( us_log )
{
us_log->log( QString( "xyz %1 %2 %3 rtp %4 %5 %6\n" )
.arg( it->x[ 0 ] )
.arg( it->x[ 1 ] )
.arg( it->x[ 2 ] )
.arg( tmp_pm_data.rtp[ 0 ] )
.arg( tmp_pm_data.rtp[ 1 ] )
.arg( tmp_pm_data.rtp[ 2 ] )
);
}
         */
      } 
      v_pdata.push_back( &pdata[ *it ] );
   }

   vector < pm_data * > v_pdata_subtracts;
   for ( set < pm_point >::iterator it = model_subtracts.begin();
         it != model_subtracts.end();
         it++ )
   {
      if ( !pdata.count( *it ) )
      {
         // if ( us_log )
         // {
         //    us_log->log( QString( "pdata does not contain %1 %2 %3\n" )
         //                 .arg( it->x[0] )
         //                 .arg( it->x[1] )
         //                 .arg( it->x[2] ) 
         //                 );
         // }
         exit(-1);
      }
      v_pdata_subtracts.push_back( &pdata[ *it ] );
   }
      
#if defined( USE_TIMERS )
   us_timers.end_timer( "dI:rtp" );

   us_timers.init_timer( "dI:legendre" );
   us_timers.init_timer( "dI:combined" );
   us_timers.init_timer( "dI:sphbes" );
   us_timers.init_timer( "dI:sumA" );

   us_timers.start_timer( "dI:combined" );
#endif
   I_result = I0;

   // cache legendre

#if defined( USE_TIMERS )
   us_timers.start_timer( "dI:legendre" );
#endif
   // setup associated legendre functions

   for ( unsigned int i = 0; i < v_pdata.size(); ++i )
   {
      pm_data *tmp_pm_data = v_pdata[ i ];
      if ( tmp_pm_data->no_Y )
      {
         // if ( us_log )
         // {
         //    us_log->log( QString( "point %1 r %2 t %3 p %4\n" )
         //                 .arg( i )
         //                 .arg( tmp_pm_data->rtp[ 0 ] )
         //                 .arg( tmp_pm_data->rtp[ 1 ] )
         //                 .arg( tmp_pm_data->rtp[ 2 ] )
         //                 .toLatin1().data() );
         // }

         complex < float > *Yp = &( tmp_pm_data->Y[ 0 ] );

#if defined( ALT_SH )
         if ( !sh::alt_conj_sh( max_harmonics, 
                                tmp_pm_data->rtp[ 1 ],
                                tmp_pm_data->rtp[ 2 ],
                                Yp ) )
         {
            error_msg = "sh::spherical_harmonic failed";
            return false;
         }
#else
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
#endif
         tmp_pm_data->no_Y = false;
      }
   }

#if defined( USE_TIMERS )
   us_timers.end_timer( "dI:legendre" );
   us_timers.start_timer( "dI:sphbes" );
#endif

   // cache spherical bessels

   unsigned int J_ofs;
   for ( unsigned int i = 0; i < (unsigned int) v_pdata.size(); ++i )
   {
      if ( v_pdata[ i ]->no_J )
      {
         for ( unsigned int j = 0; j < q_points; ++j )
         {
            J_ofs = j * ( 1 + max_harmonics );
            
#if defined( ALT_SPHBES )
            if ( !shs->shs_compute_sphbes( q[ j ] * v_pdata[ i ]->rtp[ 0 ], &(v_pdata[ i ]->J[ J_ofs ] ) ) )
            {
               error_msg = "nr::alt_shbes failed";
               return false;
            }
#else
            for ( unsigned int l = 0; l <= max_harmonics; ++l )
            {
               if ( !nr::sphbes( l, q[ j ] * v_pdata[ i ]->rtp[ 0 ], v_pdata[ i ]->J[ J_ofs + l ] ) )
               {
                  error_msg = "nr::shbes failed";
                  return false;
               }
            }
#endif
         }
         v_pdata[ i ]->no_J = false;
      }
   }
#if defined( USE_TIMERS )
   us_timers.end_timer( "dI:sphbes" );
   us_timers.start_timer( "dI:sumA" );
#endif
   double *Jp;

   complex < float > *i_lp;
   complex < float > *Ap;
   
   for ( unsigned int j = 0; j < q_points; ++j )
   {
      J_ofs = j * ( 1 + max_harmonics );

      for ( unsigned int i = 0; i < (unsigned int) v_pdata.size(); ++i )
      {
         pm_data *tmp_pm_data = v_pdata[ i ];
         complex < float > *Yp = &( tmp_pm_data->Y[ 0 ] );
         Jp = &( tmp_pm_data->J[ J_ofs ] );
         Ap = &( Av[ j ][ 0 ] );
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

      J_ofs = j * ( 1 + max_harmonics );

      for ( unsigned int i = 0; i < (unsigned int) v_pdata_subtracts.size(); ++i )
      {
         pm_data *tmp_pm_data = v_pdata_subtracts[ i ];
         complex < float > *Yp = &( tmp_pm_data->Y[ 0 ] );
         Jp = &( tmp_pm_data->J[ J_ofs ] );
         Ap = &( Av[ j ][ 0 ] );
         i_lp = &( i_l[ 0 ] );
         for ( unsigned int l = 0; l <= max_harmonics; ++l )
         {
            // these could possible be cached:
            complex < float > tmp_c = (float) *Jp * (float) F[ j ] * (*i_lp);
            ++Jp;
            ++i_lp;
            for ( int m = - (int) l ; m <= (int) l; ++m )
            {
               (*Ap) -=  (*Yp) * tmp_c;
               ++Yp;
               ++Ap;
            }
         }
      }
      Ap = &( Av[ j ][ 0 ] );
      for ( unsigned int k = 0; k < Y_points; ++k )
      {
         I_result[ j ] += norm( (*Ap) );
         ++Ap;
      }
      I_result[ j ] *= M_4PI;
   }

#if defined( USE_TIMERS )
   us_timers.end_timer( "dI:sumA" );
   us_timers.end_timer( "dI:combined" );
   if ( us_log )
   {
      us_log->log( "list times:\n" + us_timers.list_times() );
   }
   msg_log += us_timers.list_times( QString( "DI %1 beads : " ).arg( model.size() ) );
#endif
   return true;
}

bool US_PM::compute_CA_I( set < pm_point > & model, vector < double > &I_result )
{
#if defined( USE_TIMERS )
   us_timers.clear_timers();
#endif
   // model should already be centered

   pmc_data tmp_pm_data;
   pmc_data *pm_datap;
   tmp_pm_data.A1v = A1v0;

   complex < float > *Yp;

   double            *Jp;
   double            *qp;
   double            *Fp;
   double            qp_t_rtp0;

   complex < double > tmp_cd;

#if defined( USE_TIMERS )
   us_timers.init_timer( "CA:combined" );
   us_timers.init_timer( "CA:rtp/legendre/shbes" );
   us_timers.init_timer( "CA:sumA" );
   us_timers.start_timer( "CA:combined" );
   us_timers.start_timer( "CA:rtp/legendre/shbes" );
#endif
   I_result = I0;

   complex < float > *i_lp;
   complex < float > *Ap;
   complex < float > *A1vp;
   complex < float > tmp_cf;

   ccA1v = A1v0;

   for ( set < pm_point >::iterator it = model.begin();
         it != model.end();
         it++ )
   {
      if ( !pcdata.count( *it ) )
      {
         pcdata[ *it ] = tmp_pm_data;
         pm_datap = &(pcdata[ *it ]);

         pm_datap->x[ 0 ] = grid_conversion_factor * (double)it->x[ 0 ];
         pm_datap->x[ 1 ] = grid_conversion_factor * (double)it->x[ 1 ];
         pm_datap->x[ 2 ] = grid_conversion_factor * (double)it->x[ 2 ];

         pm_datap->rtp[ 0 ] = sqrt ( (double) ( pm_datap->x[ 0 ] * pm_datap->x[ 0 ] +
                                                pm_datap->x[ 1 ] * pm_datap->x[ 1 ] +
                                                pm_datap->x[ 2 ] * pm_datap->x[ 2 ] ) );
         if ( pm_datap->rtp[ 0 ] == 0e0 )
         {
            pm_datap->rtp[ 1 ] = 0e0;
            pm_datap->rtp[ 2 ] = 0e0;
         } else {
            pm_datap->rtp[ 1 ] = acos ( pm_datap->x[ 2 ] / pm_datap->rtp[ 0 ] );

            if ( it->x[ 0 ] == 0 &&
                 it->x[ 1 ] == 0 )
            {
               pm_datap->rtp[ 2 ] = 0e0;
            } else {               
               if ( it->x[ 0 ] < 0 )
               {
                  pm_datap->rtp[ 2 ] = M_PI - asin( pm_datap->x[ 1 ] / sqrt( (double) ( pm_datap->x[ 0 ] * 
                                                                                        pm_datap->x[ 0 ] +
                                                                                        pm_datap->x[ 1 ] * 
                                                                                        pm_datap->x[ 1 ] ) ) );
               } else {
                  if ( it->x[ 1 ] < 0 )
                  {
                     pm_datap->rtp[ 2 ] = M_2PI + asin( pm_datap->x[ 1 ] / sqrt( pm_datap->x[ 0 ] * 
                                                                                 pm_datap->x[ 0 ] +
                                                                                 pm_datap->x[ 1 ] * 
                                                                                 pm_datap->x[ 1 ] ) );
                  } else {
                     pm_datap->rtp[ 2 ] = asin( pm_datap->x[ 1 ] / sqrt( pm_datap->x[ 0 ] * 
                                                                         pm_datap->x[ 0 ] +
                                                                         pm_datap->x[ 1 ] * 
                                                                         pm_datap->x[ 1 ] ) );
                  }               
               }
            }
         }
         
         Yp = &( ccY[ 0 ] );

#if defined( ALT_SH )
         if ( !sh::alt_conj_sh( max_harmonics, 
                                pm_datap->rtp[ 1 ],
                                pm_datap->rtp[ 2 ],
                                Yp ) )
         {
            error_msg = "sh::spherical_harmonic failed";
            return false;
         }
#else
         for ( unsigned int l = 0; l <= max_harmonics; ++l )
         {
            for ( int m = - (int) l ; m <= (int) l; ++m )
            {
               if ( !sh::conj_spherical_harmonic( l, 
                                                  m, 
                                                  pm_datap->rtp[ 1 ],
                                                  pm_datap->rtp[ 2 ],
                                                  tmp_cd) )
               {
                  error_msg = "sh::spherical_harmonic failed";
                  return false;
               }
               (*Yp) = tmp_cd;
               ++Yp;
            }
         }
#endif

         Jp  = &( ccJ[ 0 ] );
         qp  = &( q[ 0 ] );
         Fp  = &( F[ 0 ] );
         A1vp = &( ccA1v[ 0 ] );
         Ap   = &( pm_datap->A1v[ 0 ] );

         for ( unsigned int j = 0; j < q_points; ++j )
         {
            qp_t_rtp0 = (*qp) * pm_datap->rtp[ 0 ];
            ++qp;

            i_lp = &( i_l[ 0 ] );
            Yp   = &( ccY[ 0 ] );
#if defined( ALT_SPHBES )
            if ( !shs->shs_compute_sphbes( qp_t_rtp0, Jp ) )
            {
               error_msg = "nr::alt_shbes failed";
               return false;
            }
            for ( unsigned int l = 0; l <= max_harmonics; ++l )
            {
               tmp_cf = (float) *Jp * (float)(*Fp) * (*i_lp);
               for ( int m = - (int) l ; m <= (int) l; ++m )
               {
                  (*Ap)  +=  (*Yp) * tmp_cf;
                  (*A1vp) +=  (*Ap);
                  ++Yp;
                  ++Ap;
                  ++A1vp;
               }
               ++Jp;
               ++i_lp;
            }
#else
            for ( unsigned int l = 0; l <= max_harmonics; ++l )
            {
               if ( !nr::sphbes( l, qp_t_rtp0, *Jp ) )
               {
                  error_msg = "nr::shbes failed";
                  return false;
               }

               tmp_cf = (float) *Jp * (float)(*Fp) * (*i_lp);
               for ( int m = - (int) l ; m <= (int) l; ++m )
               {
                  (*Ap)  +=  (*Yp) * tmp_cf;
                  (*A1vp) +=  (*Ap);
                  ++Yp;
                  ++Ap;
                  ++A1vp;
               }
               ++Jp;
               ++i_lp;
            }
#endif
            ++Fp;
         }
      } else {
         pm_datap = &pcdata[ *it ];
         A1vp = &( ccA1v[ 0 ] );
         Ap   = &( pm_datap->A1v[ 0 ] );
         for ( unsigned int j = 0; j < q_points; ++j )
         {
            for ( unsigned int k = 0; k < Y_points; ++k )
            {
               (*A1vp) +=  (*Ap);
               ++Ap;
               ++A1vp;
            }
         }
      }
   }

#if defined( USE_TIMERS )
   us_timers.end_timer( "CA:rtp/legendre/shbes" );
   us_timers.start_timer( "CA:sumA" );
#endif
   A1vp = &( ccA1v[ 0 ] );
   for ( unsigned int j = 0; j < q_points; ++j )
   {
      for ( unsigned int k = 0; k < Y_points; ++k )
      {
         I_result[ j ] += norm( (*A1vp) );
         ++A1vp;
      }
      I_result[ j ] *= M_4PI;
   }

#if defined( USE_TIMERS )
   us_timers.end_timer( "CA:sumA" );
   us_timers.end_timer( "CA:combined" );

   if ( us_log )
   {
      us_log->log( "list times:\n" + us_timers.list_times() );
   }
   msg_log += us_timers.list_times( QString( "FCI %1 beads : " ).arg( model.size() ) );
#endif
   return true;
}

double US_PM::fitness2( vector < double > & I_result )
{
   double chi2 = 0e0;
   double tmp;

   /* non-scaling fit obsolete
   if ( use_errors )
   {
      for ( unsigned int i = 0; i < q_points; i++ )
      {
         tmp = ( I_result[ i ] - I[ i ] ) / e[ i ];
         chi2 += tmp * tmp;
      }
   } else {
      for ( unsigned int i = 0; i < q_points; i++ )
      {
         tmp = ( I_result[ i ] - I[ i ] );
         chi2 += tmp * tmp;
      }
   }
   */

   double k = 0e0;
   double Sxx = 0e0;
   double Sxy = 0e0;

   if ( use_errors )
   {
      for ( unsigned int i = 0; i < q_points; ++i )
      {
         Sxx += I_result[i] * I_result[i] * oneoversd2[ i ];
         Sxy += I_result[i] * I[i] * oneoversd2[ i ];
      }
      if ( Sxx != 0e0 )
      {
         k = Sxy / Sxx;
      } else {
         k = 1e0;
      }
      for ( unsigned int i = 0; i < q_points; ++i )
      {
         tmp =  k * I_result[i] - I[i];
         chi2 += tmp * tmp * oneoversd2[ i ];
      }
   } else {
      for ( unsigned int i = 0; i < q_points; ++i )
      {
         Sxx += I_result[i] * I_result[i];
         Sxy += I_result[i] * I[i];
      }
      if ( Sxx != 0e0 )
      {
         k = Sxy / Sxx;
      } else {
         k = 1e0;
      }
      for ( unsigned int i = 0; i < q_points; ++i )
      {
         tmp =  k * I_result[i] - I[i];
         chi2 += tmp * tmp;
      }
   }

   return chi2;
}

