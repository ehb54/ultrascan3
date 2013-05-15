#include "../include/us_saxs_util.h"

bool US_Saxs_Util::read_sas_data( 
                                 QString             filename,
                                 vector < double > & q,
                                 vector < double > & I,
                                 vector < double > & e,
                                 QString           & error_msg 
                                 )
{
   error_msg = "";
   QFile f( filename );
   if ( !f.exists() )
   {
      error_msg = QString("Error: %1 does not exist").arg( filename );
      return false;
   }
   QString ext = QFileInfo( filename ).extension( false ).lower();

   QRegExp rx_valid_ext (
                         "^("
                         "dat|"
                         "int|"
                         "txt|"
                         // "out|"
                         "ssaxs)$" );

   if ( rx_valid_ext.search( ext ) == -1 )
   {
      error_msg = QString("Error: %1 unsupported file extension %2").arg( filename ).arg( ext );
      return false;
   }
      
   if ( !f.open( IO_ReadOnly ) )
   {
      error_msg = QString("Error: can not open %1, check permissions ").arg( filename );
      return false;
   }

   QTextStream ts(&f);
   vector < QString > qv;
   QStringList qsl;

   while ( !ts.atEnd() )
   {
      QString qs = ts.readLine();
      qv.push_back( qs );
      qsl << qs;
   }
   f.close();

   if ( !qv.size() )
   {
      error_msg = QString("Error: the file %1 is empty ").arg( filename );
      return false;
   }

   // we should make some configuration for matches & offsets or column mapping
   // just an ad-hoc fix for APS 5IDD
   int offset = 0;
   if ( ext == "txt" && qv[ 0 ].contains( "# File Encoding (File origin in Excel)" ) )
   {
      offset = 1;
   }      

   QRegExp rx_ok_line("^(\\s+|\\d+|\\.|\\d(E|e)(\\+|-|\\d))+$");
   rx_ok_line.setMinimal( true );
   for ( int i = 1; i < (int) qv.size(); i++ )
   {
      if ( qv[i].contains(QRegExp("^#")) ||
           rx_ok_line.search( qv[i] ) == -1 )
      {
         continue;
      }
      
      QStringList tokens = QStringList::split(QRegExp("\\s+"), qv[i].replace(QRegExp("^\\s+"),""));

      if ( (int)tokens.size() > 1 + offset )
      {
         double this_q         = tokens[ 0 + offset ].toDouble();
         double this_I         = tokens[ 1 + offset ].toDouble();
         double this_e = 0e0;
         if ( (int)tokens.size() > 2 + offset)
         {
            this_e = tokens[ 2 + offset ].toDouble();
            if ( this_e < 0e0 )
            {
               this_e = 0e0;
            }
         }
         if ( q.size() && this_q <= q[ q.size() - 1 ] )
         {
            cout << QString(" breaking %1 %2\n").arg( this_q ).arg( q[ q.size() - 1 ] );
            break;
         }
         if ( this_I != 0e0 )
         {
            q       .push_back( this_q );
            I       .push_back( this_I );
            if ( (int)tokens.size() > 2 + offset && this_e )
            {
               e.push_back( this_e );
            }
         }
      }
   }

   if ( !q.size() )
   {
      error_msg = QString( "Error: File %1 has no data" ).arg( filename );
      return false;
   }

   if ( e.size() != q.size() )
   {
      e.clear();
   };

   return true;
}

void US_Saxs_Util::clip_data( 
                             double              minq,
                             double              maxq,
                             vector < double > & q,
                             vector < double > & I,
                             vector < double > & e
                             )
{
   vector < double >  q_new;
   vector < double >  I_new;
   vector < double >  e_new;
   for ( int i = 0; i < (int) q.size(); ++i )
   {
      if ( q[ i ] >= minq && q[ i ] <= maxq )
      {
         q_new.push_back( q[ i ] );
         I_new.push_back( I[ i ] );
         if ( e.size() )
         {
            e_new.push_back( e[ i ] );
         }
      }
   }
   
   q = q_new;
   I = I_new;
   e = e_new;
}

bool US_Saxs_Util::bin_data( 
                            int                 bins,
                            bool                log_bin,
                            vector < double > & q,
                            vector < double > & I,
                            vector < double > & e,
                            QString           & error_msg,
                            QString           & notice_msg
                            )
{
   error_msg = "";
   notice_msg = "";

   vector < double >  q_new = q;
   vector < double >  I_new = I;
   vector < double >  e_new = e;

   if ( (int) q.size() <= bins )
   {
      notice_msg = "Notice: requested bins greater or equal to available q points, returning full set";
      return true;
   }

   if ( !log_bin )
   {
      double step = (double) q.size() / (double) bins;
      for ( double i = 0; (int)( .5 + i ) < (int)q.size(); i += step )
      {
         int idx = (int)( .5 + i );
         q_new.push_back( q[ idx ] );
         I_new.push_back( I[ idx ] );
         if ( e.size() )
         {
            e_new.push_back( e[ idx ] );
         }
      }
   } else {
      map < int, bool > used;
            
      double lp = log( (double) q.size() );
      double step = lp / (double) bins;
      for ( double i = 0; i <= lp; i += step )
      {
         int idx = int( exp( i ) - .5 );
         if ( !used.count( idx ) )
         {
            q_new.push_back( q[ idx ] );
            I_new.push_back( I[ idx ] );
            if ( e.size() )
            {
               e_new.push_back( e[ idx ] );
            }
            used[ idx ] = true;
         }
      }
      if ( (int)q_new.size() < bins )
      {
         notice_msg = QString( "Notice: log bins produced fewer points (%1) than requested (%2)" ).arg( q_new.size() ).arg( bins );
      }
   }
   q = q_new;
   I = I_new;
   e = e_new;
   return true;
}

double US_Saxs_Util::alt_erf( double x )
{
   /* erf(z) = 2/sqrt(pi) * Integral(0..x) exp( -t^2) dt
      erf(0.01) = 0.0112834772 erf(3.7) = 0.9999998325
      Abramowitz/Stegun: p299, |erf(z)-erf| <= 1.5*10^(-7)
   */

   double y = 1.0e0 / ( 1.0e0 + 0.3275911e0 * x);
   return 1 - (((((
                   + 1.061405429e0 * y
                   - 1.453152027e0) * y
                  + 1.421413741e0) * y
                 - 0.284496736e0) * y
                + 0.254829592e0) * y)
      * exp (-x * x);
}


/*
Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
*
* Developed at SunPro, a Sun Microsystems, Inc. business.
* Permission to use, copy, modify, and distribute this
* software is freely granted, provided that this notice
* is preserved.
*/

static const double tiny = 1e-300,
   half= 5.00000000000000000000e-01, /* 0x3FE00000, 0x00000000 */
   one = 1.00000000000000000000e+00, /* 0x3FF00000, 0x00000000 */
   two = 2.00000000000000000000e+00, /* 0x40000000, 0x00000000 */
   /* c = (float)0.84506291151 */
   erx = 8.45062911510467529297e-01, /* 0x3FEB0AC1, 0x60000000 */
   /*
    * Coefficients for approximation to erf on [0,0.84375]
    */
   efx = 1.28379167095512586316e-01, /* 0x3FC06EBA, 0x8214DB69 */
   efx8= 1.02703333676410069053e+00, /* 0x3FF06EBA, 0x8214DB69 */
   pp0 = 1.28379167095512558561e-01, /* 0x3FC06EBA, 0x8214DB68 */
   pp1 = -3.25042107247001499370e-01, /* 0xBFD4CD7D, 0x691CB913 */
   pp2 = -2.84817495755985104766e-02, /* 0xBF9D2A51, 0xDBD7194F */
   pp3 = -5.77027029648944159157e-03, /* 0xBF77A291, 0x236668E4 */
   pp4 = -2.37630166566501626084e-05, /* 0xBEF8EAD6, 0x120016AC */
   qq1 = 3.97917223959155352819e-01, /* 0x3FD97779, 0xCDDADC09 */
   qq2 = 6.50222499887672944485e-02, /* 0x3FB0A54C, 0x5536CEBA */
   qq3 = 5.08130628187576562776e-03, /* 0x3F74D022, 0xC4D36B0F */
   qq4 = 1.32494738004321644526e-04, /* 0x3F215DC9, 0x221C1A10 */
   qq5 = -3.96022827877536812320e-06, /* 0xBED09C43, 0x42A26120 */
   /*
    * Coefficients for approximation to erf in [0.84375,1.25]
    */
   pa0 = -2.36211856075265944077e-03, /* 0xBF6359B8, 0xBEF77538 */
   pa1 = 4.14856118683748331666e-01, /* 0x3FDA8D00, 0xAD92B34D */
   pa2 = -3.72207876035701323847e-01, /* 0xBFD7D240, 0xFBB8C3F1 */
   pa3 = 3.18346619901161753674e-01, /* 0x3FD45FCA, 0x805120E4 */
   pa4 = -1.10894694282396677476e-01, /* 0xBFBC6398, 0x3D3E28EC */
   pa5 = 3.54783043256182359371e-02, /* 0x3FA22A36, 0x599795EB */
   pa6 = -2.16637559486879084300e-03, /* 0xBF61BF38, 0x0A96073F */
   qa1 = 1.06420880400844228286e-01, /* 0x3FBB3E66, 0x18EEE323 */
   qa2 = 5.40397917702171048937e-01, /* 0x3FE14AF0, 0x92EB6F33 */
   qa3 = 7.18286544141962662868e-02, /* 0x3FB2635C, 0xD99FE9A7 */
   qa4 = 1.26171219808761642112e-01, /* 0x3FC02660, 0xE763351F */
   qa5 = 1.36370839120290507362e-02, /* 0x3F8BEDC2, 0x6B51DD1C */
   qa6 = 1.19844998467991074170e-02, /* 0x3F888B54, 0x5735151D */
   /*
    * Coefficients for approximation to erfc in [1.25,1/0.35]
    */
   ra0 = -9.86494403484714822705e-03, /* 0xBF843412, 0x600D6435 */
   ra1 = -6.93858572707181764372e-01, /* 0xBFE63416, 0xE4BA7360 */
   ra2 = -1.05586262253232909814e+01, /* 0xC0251E04, 0x41B0E726 */
   ra3 = -6.23753324503260060396e+01, /* 0xC04F300A, 0xE4CBA38D */
   ra4 = -1.62396669462573470355e+02, /* 0xC0644CB1, 0x84282266 */
   ra5 = -1.84605092906711035994e+02, /* 0xC067135C, 0xEBCCABB2 */
   ra6 = -8.12874355063065934246e+01, /* 0xC0545265, 0x57E4D2F2 */
   ra7 = -9.81432934416914548592e+00, /* 0xC023A0EF, 0xC69AC25C */
   sa1 = 1.96512716674392571292e+01, /* 0x4033A6B9, 0xBD707687 */
   sa2 = 1.37657754143519042600e+02, /* 0x4061350C, 0x526AE721 */
   sa3 = 4.34565877475229228821e+02, /* 0x407B290D, 0xD58A1A71 */
   sa4 = 6.45387271733267880336e+02, /* 0x40842B19, 0x21EC2868 */
   sa5 = 4.29008140027567833386e+02, /* 0x407AD021, 0x57700314 */
   sa6 = 1.08635005541779435134e+02, /* 0x405B28A3, 0xEE48AE2C */
   sa7 = 6.57024977031928170135e+00, /* 0x401A47EF, 0x8E484A93 */
   sa8 = -6.04244152148580987438e-02, /* 0xBFAEEFF2, 0xEE749A62 */
   /*
    * Coefficients for approximation to erfc in [1/.35,28]
    */
   rb0 = -9.86494292470009928597e-03, /* 0xBF843412, 0x39E86F4A */
   rb1 = -7.99283237680523006574e-01, /* 0xBFE993BA, 0x70C285DE */
   rb2 = -1.77579549177547519889e+01, /* 0xC031C209, 0x555F995A */
   rb3 = -1.60636384855821916062e+02, /* 0xC064145D, 0x43C5ED98 */
   rb4 = -6.37566443368389627722e+02, /* 0xC083EC88, 0x1375F228 */
   rb5 = -1.02509513161107724954e+03, /* 0xC0900461, 0x6A2E5992 */
   rb6 = -4.83519191608651397019e+02, /* 0xC07E384E, 0x9BDC383F */
   sb1 = 3.03380607434824582924e+01, /* 0x403E568B, 0x261D5190 */
   sb2 = 3.25792512996573918826e+02, /* 0x40745CAE, 0x221B9F0A */
   sb3 = 1.53672958608443695994e+03, /* 0x409802EB, 0x189D5118 */
   sb4 = 3.19985821950859553908e+03, /* 0x40A8FFB7, 0x688C246A */
   sb5 = 2.55305040643316442583e+03, /* 0x40A3F219, 0xCEDF3BE6 */
   sb6 = 4.74528541206955367215e+02, /* 0x407DA874, 0xE79FE763 */
   sb7 = -2.24409524465858183362e+01; /* 0xC03670E2, 0x42712D62 */

double US_Saxs_Util::sun_erf( double x )
{
   int n0;
   int hx;
   int ix;
   int i;
   double R_;
   double S;
   double P;
   double Q;
   double s;
   double y;
   double z;
   double r;
   n0 = ((*(int*)&one)>>29)^1;
   hx = *(n0+(int*)&x);
   ix = hx&0x7fffffff;
   if(ix>=0x7ff00000) { /* erf(nan)=nan */
      i = ((unsigned)hx>>31)<<1;
      return (double)(1-i)+one/x; /* erf(+-inf)=+-1 */
   }

   if(ix < 0x3feb0000) { /* |x|<0.84375 */
      if(ix < 0x3e300000) { /* |x|<2**-28 */
         if (ix < 0x00800000)
            return 0.125*(8.0*x+efx8*x); /*avoid underflow */
         return x + efx*x;
      }
      z = x*x;
      r = pp0+z*(pp1+z*(pp2+z*(pp3+z*pp4)));
      s = one+z*(qq1+z*(qq2+z*(qq3+z*(qq4+z*qq5))));
      y = r/s;
      return x + x*y;
   }
   if(ix < 0x3ff40000) { /* 0.84375 <= |x| < 1.25 */
      s = fabs(x)-one;
      P = pa0+s*(pa1+s*(pa2+s*(pa3+s*(pa4+s*(pa5+s*pa6)))));
      Q = one+s*(qa1+s*(qa2+s*(qa3+s*(qa4+s*(qa5+s*qa6)))));
      if(hx>=0) return erx + P/Q; else return -erx - P/Q;
   }
   if (ix >= 0x40180000) { /* inf>|x|>=6 */
      if(hx>=0) return one-tiny; else return tiny-one;
   }
   x = fabs(x);
   s = one/(x*x);
   if(ix< 0x4006DB6E) { /* |x| < 1/0.35 */
      R_=ra0+s*(ra1+s*(ra2+s*(ra3+s*(ra4+s*(
                                           ra5+s*(ra6+s*ra7))))));
      S=one+s*(sa1+s*(sa2+s*(sa3+s*(sa4+s*(
                                           sa5+s*(sa6+s*(sa7+s*sa8)))))));
   } else { /* |x| >= 1/0.35 */
      R_=rb0+s*(rb1+s*(rb2+s*(rb3+s*(rb4+s*(
                                           rb5+s*rb6)))));
      S=one+s*(sb1+s*(sb2+s*(sb3+s*(sb4+s*(
                                           sb5+s*(sb6+s*sb7))))));
   }
   z = x;
   *(1-n0+(int*)&z) = 0;
   r = exp(-z*z-0.5625)*exp((z-x)*(z+x)+R/S);
   if(hx>=0) return one-r/x; else return r/x-one;
}

double US_Saxs_Util::sun_erfc(double x)
{
   int n0,hx,ix;
   double R_,S,P,Q,s,y,z,r;
   n0 = ((*(int*)&one)>>29)^1;
   hx = *(n0+(int*)&x);
   ix = hx&0x7fffffff;
   if(ix>=0x7ff00000) { /* erfc(nan)=nan */
      /* erfc(+-inf)=0,2 */
      return (double)(((unsigned)hx>>31)<<1)+one/x;
   }

   if(ix < 0x3feb0000) { /* |x|<0.84375 */
      if(ix < 0x3c700000) /* |x|<2**-56 */
         return one-x;
      z = x*x;
      r = pp0+z*(pp1+z*(pp2+z*(pp3+z*pp4)));
      s = one+z*(qq1+z*(qq2+z*(qq3+z*(qq4+z*qq5))));
      y = r/s;
      if(hx < 0x3fd00000) { /* x<1/4 */
         return one-(x+x*y);
      } else {
         r = x*y;
         r += (x-half);
         return half - r ;
      }
   }
   if(ix < 0x3ff40000) { /* 0.84375 <= |x| < 1.25 */
      s = fabs(x)-one;
      P = pa0+s*(pa1+s*(pa2+s*(pa3+s*(pa4+s*(pa5+s*pa6)))));
      Q = one+s*(qa1+s*(qa2+s*(qa3+s*(qa4+s*(qa5+s*qa6)))));
      if(hx>=0) {
         z = one-erx; return z - P/Q;
      } else {
         z = erx+P/Q; return one+z;
      }
   }
   if (ix < 0x403c0000) { /* |x|<28 */
      x = fabs(x);
      s = one/(x*x);
      if(ix< 0x4006DB6D) { /* |x| < 1/.35 ~ 2.857143*/
         R_=ra0+s*(ra1+s*(ra2+s*(ra3+s*(ra4+s*(
                                              ra5+s*(ra6+s*ra7))))));
         S=one+s*(sa1+s*(sa2+s*(sa3+s*(sa4+s*(
                                              sa5+s*(sa6+s*(sa7+s*sa8)))))));
      } else { /* |x| >= 1/.35 ~ 2.857143 */
         if(hx<0&&ix>=0x40180000) return two-tiny;/* x < -6 */
         R_=rb0+s*(rb1+s*(rb2+s*(rb3+s*(rb4+s*(
                                              rb5+s*rb6)))));
         S=one+s*(sb1+s*(sb2+s*(sb3+s*(sb4+s*(
                                              sb5+s*(sb6+s*sb7))))));
      }
      z = x;
      *(1-n0+(int*)&z) = 0;
      r = exp(-z*z-0.5625)*
         exp((z-x)*(z+x)+R/S);
      if(hx>0) return r/x; else return two-r/x;
   } else {
      if(hx>0) return tiny*tiny; else return two-tiny;
   }
}
