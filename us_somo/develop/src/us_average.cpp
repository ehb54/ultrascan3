#include "../include/us_average.h"
#include "../include/us_vector.h"

#include <QTextStream>
#include <qdebug.h>

US_Average::US_Average() {
   qDebug() << "US_Average() constructor";

}

US_Average::~US_Average() {
   qDebug() << "US_Average() destructor";
}

bool US_Average::compute(
                         enum weight_methods method
                         ,const vector < double > & xv
                         ,const vector < double > & sdv
                         ,double & avg
                         ,double & sd
                         ,QString & errormsg
                         ) {
   if ( !xv.size() ) {
      errormsg = "US_Average::weighted() the input x-vector is empty";
      return false;
   }

   if ( xv.size() != sdv.size() ) {
      errormsg = QString( "US_Average::compute() the input x-vector size (%1) is not equal to the input sd-vector size (%2)" )
         .arg( xv.size() )
         .arg( sdv.size() )
         ;
      return false;
   }

   avg = 0;
   sd  = 0;

   switch ( method ) {

   case NO_WEIGHTS :
      
      for ( size_t i = 0; i < xv.size(); ++i ) {
         avg += xv[ i ];
         sd  += sdv[ i ] * sdv[ i ];
      }

      avg /= (double) xv.size();
      sd  /= (double) xv.size();
      sd  = sqrt( sd );

      return true;
      break;

   case ONE_OVER_SD :
      
      {
         double wsum = 0;
         for ( size_t i = 0; i < xv.size(); ++i ) {
            if ( sdv[ i ] <= 0 ) {
               errormsg = QString( "US_Average::compute() sd-vector[ %1 ] is not a positive value (%2)" )
                  .arg( i )
                  .arg( sdv[ i ] )
                  ;
               return false;
            }
            
            double weight = 1 / sdv[ i ];
            wsum += weight;
            avg += xv[ i ] * weight;
            sd  += ( sdv[ i ] * sdv[ i ] ) * weight * weight;
         }

         avg /= wsum;
         sd  /= wsum;
         sd  = sqrt( sd );
      }

      return true;
      break;

   case ONE_OVER_SD_SQ :
      
      {
         double wsum = 0;
         for ( size_t i = 0; i < xv.size(); ++i ) {
            if ( sdv[ i ] <= 0 ) {
               errormsg = QString( "US_Average::compute() sd-vector[ %1 ] is not a positive value (%2)" )
                  .arg( i )
                  .arg( sdv[ i ] )
                  ;
               return false;
            }
            
            double weight = 1 / ( sdv[ i ] * sdv[ i ] );
            wsum += weight;
            avg += xv[ i ] * weight;
            // sd  += ( sdv[ i ] * sdv[ i ] ) * weight * weight;
         }

         avg /= wsum;
         sd  = 1.0 / wsum;
         sd  = sqrt( sd );
      }

      return true;
      break;


   default :
      errormsg = QString( "US_Average::compute() - internal error - unsupported method %1" ).arg( (int) method );
      return false;
      break;
   }

   QString( "US_Average::compute() - internal error - should not get here" );
   return false;
}

bool US_Average::test() {

   double avg;
   double sd;
   QString errormsg;


   vector < vector < double > > xs =
      {
         { 1, 2, 3, 4, 5 }
         ,{ 7, 8, 9, 10, 11 }
         ,{ 927, 1234, 1032, 876, 865, 750, 780, 690, 730, 821, 803, 850 }
         ,{ 11, 10 }
         ,{
            2.502
            ,2.346
            ,2.339
            ,2.293
            ,2.192
            ,2.246
            ,2.258
            ,2.242
            ,2.123
            ,2.144
         }
         ,{
            2.490
            ,2.341
            ,2.337
            ,2.284
            ,2.247
            ,2.260
            ,2.283
            ,2.224
            ,2.139
            ,2.083
         }
      }
   ;
   
   vector < vector < double > > sds =
      {
         { 1, 1, 1, 1, 1 }
         ,{ .1, .2, .3, .4, .5 }
         ,{ 333, 250, 301, 204, 165, 263, 280, 98, 76, 240, 178, 250 }
         ,{ 1, 2 }
         ,{
            0.058
            ,0.041
            ,0.040
            ,0.034
            ,0.038
            ,0.030
            ,0.032
            ,0.034
            ,0.042
            ,0.043
         }

         ,{
            0.014
            ,0.009
            ,0.008
            ,0.007
            ,0.007
            ,0.006
            ,0.006
            ,0.008
            ,0.009
            ,0.007
         }
      }
   ;      

   for ( size_t i = 0; i < xs.size(); ++ i ) {
      QTextStream( stdout ) << "========================================\n";
      
      US_Vector::printvector2( QString( "test %1 x,sd" ).arg( i ), xs[ i ], sds[ i ] );

      for ( auto const method : {
            NO_WEIGHTS
               // ,ONE_OVER_SD
           ,ONE_OVER_SD_SQ
               } ) {
         QTextStream( stdout )
            << "----------------------------------------\n"
            << QString( "method : %1\n" ).arg( method )
            << "----------------------------------------\n"
            ;
         
         if ( !compute( method
                        ,xs[ i ]
                        ,sds[ i ]
                        ,avg
                        ,sd
                        ,errormsg ) ) {
            QTextStream( stdout ) << errormsg;
         } else {
            QTextStream( stdout ) << QString( "average %1 +/- %2\n" ).arg( avg ).arg( sd );
         }
      }
   }

   return true;
}

   
