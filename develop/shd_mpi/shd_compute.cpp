#include "shd.h"

bool SHD::compute_amplitudes( vector < shd_point > & model, vector < complex < float > > & Av )
{
   // model should already be centered

   shd_data tmp_data;
   shd_data *datap = &tmp_data;
   tmp_data.A1v    = A1v0;

   complex < float > *Yp;

   double            *Jp;
   double            *qp;
   double            *Fp;
   double            qp_t_rtp0;

   complex < double > tmp_cd;

   complex < float > *i_lp;
   complex < float > *Ap;
   complex < float > *A1vp;
   complex < float > tmp_cf;

   Av = A1v0;

   shd_point * modelp = (shd_point *)(& model[ 0 ]);
   int model_size = (int)model.size();

   for ( int m = 0; m < model_size; ++m, ++modelp )
   {
      datap->rtp[ 0 ] = sqrt ( (double) ( modelp->x[ 0 ] * modelp->x[ 0 ] +
                                             modelp->x[ 1 ] * modelp->x[ 1 ] +
                                             modelp->x[ 2 ] * modelp->x[ 2 ] ) );
      if ( datap->rtp[ 0 ] == 0e0 )
      {
         datap->rtp[ 1 ] = 0e0;
         datap->rtp[ 2 ] = 0e0;
      } else {
         datap->rtp[ 1 ] = acos ( modelp->x[ 2 ] / datap->rtp[ 0 ] );

         if ( modelp->x[ 0 ] == 0 &&
              modelp->x[ 1 ] == 0 )
         {
            datap->rtp[ 2 ] = 0e0;
         } else {               
            if ( modelp->x[ 0 ] < 0 )
            {
               datap->rtp[ 2 ] = M_PI - asin( modelp->x[ 1 ] / sqrt( (double) ( modelp->x[ 0 ] * 
                                                                                   modelp->x[ 0 ] +
                                                                                   modelp->x[ 1 ] * 
                                                                                   modelp->x[ 1 ] ) ) );
            } else {
               if ( modelp->x[ 1 ] < 0 )
               {
                  datap->rtp[ 2 ] = M_2PI + asin( modelp->x[ 1 ] / sqrt( modelp->x[ 0 ] * 
                                                                            modelp->x[ 0 ] +
                                                                            modelp->x[ 1 ] * 
                                                                            modelp->x[ 1 ] ) );
               } else {
                  datap->rtp[ 2 ] = asin( modelp->x[ 1 ] / sqrt( modelp->x[ 0 ] * 
                                                                    modelp->x[ 0 ] +
                                                                    modelp->x[ 1 ] * 
                                                                    modelp->x[ 1 ] ) );
               }               
            }
         }
      }
         
      Yp = &( ccY[ 0 ] );

      for ( unsigned int l = 0; l <= max_harmonics; ++l )
      {
         for ( int m = - (int) l ; m <= (int) l; ++m )
         {
            if ( !sh::conj_spherical_harmonic( l, 
                                               m, 
                                               datap->rtp[ 1 ],
                                               datap->rtp[ 2 ],
                                               tmp_cd) )
            {
               error_msg = "sh::spherical_harmonic failed";
               return false;
            }
            (*Yp) = tmp_cd;
            ++Yp;
         }
      }

      Jp  = &( ccJ[ 0 ] );
      qp  = &( q[ 0 ] );
      Fp  = &( F[ modelp->ff_type ][ 0 ] );
      A1vp = &( Av[ 0 ] );
      Ap   = &( datap->A1v[ 0 ] );

      for ( unsigned int j = 0; j < q_points; ++j )
      {
         qp_t_rtp0 = (*qp) * datap->rtp[ 0 ];
         ++qp;

         i_lp = &( i_l[ 0 ] );
         Yp   = &( ccY[ 0 ] );

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
         ++Fp;
      }
   } 

   return true;
}

