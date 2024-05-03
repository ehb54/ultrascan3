#include "../include/us_fractal_dimension.h"

#define USFD_MIN_POINTS 10

US_Fractal_Dimension::US_Fractal_Dimension() {
   qDebug() << "US_Fractal_Dimension() constructor";

}

US_Fractal_Dimension::~US_Fractal_Dimension() {
   qDebug() << "US_Fractal_Dimension() destructor";
}

bool US_Fractal_Dimension::compute_box_counting(
                                                const vector < point > & points
                                                ,double                box_angstrom_start
                                                ,double                box_angstrom_end
                                                ,double                box_angstrom_steps
                                                ,double                & fd
                                                ,QString               & errormsg
                                                ,vector < double >     & x
                                                ,vector < double >     & y
                                                ) {

   qDebug() << "US_Fractal_Dimension::compute_box_counting()";

   if ( points.size() < USFD_MIN_POINTS ) {
      errormsg =
         QString( "Invalid number of points for box counting %1 (minimum %2)" )
         .arg( points.size() )
         .arg( USFD_MIN_POINTS )
         ;
      return false;
   }

   if ( box_angstrom_steps <= 0 ) {
      errormsg = QString( "Invalid number of steps (%1)" ).arg( box_angstrom_steps );
      return false;
   }

   double stepsize = ( box_angstrom_end - box_angstrom_start ) / box_angstrom_steps;
   
   if ( stepsize <= 0 ) {
      errormsg = QString( "Invalid computed stepsize  (%1)" ).arg( stepsize );
      return false;
   }

   qDebug() << QString( "stepsize %1" ).arg( stepsize );

   // global limits
                                                

   for ( double box_edge_size = box_angstrom_start; box_edge_size <= box_angstrom_end; box_edge_size += stepsize ) {
      // qDebug() << QString( "US_Fractal_Dimension::compute_box_counting() : processing box_edge_size %1" ).arg( box_edge_size );
      set < vector < int > > occupied;

      double box_edge_inverse = 1e0 / box_edge_size;
      
      for ( auto const & p : points ) {
         occupied.insert( {
               (int) ( p.axis[ 0 ] * box_edge_inverse )
                  ,(int) ( p.axis[ 1 ] * box_edge_inverse )
                  ,(int) ( p.axis[ 2 ] * box_edge_inverse )
                  }
            );
      }

      x.push_back( log( pow( box_edge_size, 3 ) ) );
      y.push_back( log( occupied.size() ) );
                   
      QTextStream( stdout ) << QString( "box edge size %1 [%2], number of occupied boxes %3, volume of one box %4 [%5^3] ln(vol) %6 ln(number of occupied boxes) %7\n" )
         .arg( box_edge_size )
         .arg( UNICODE_ANGSTROM )
         .arg( occupied.size() )
         .arg( pow( box_edge_size, 3 ) )
         .arg( UNICODE_ANGSTROM )
         .arg( x.back() )
         .arg( y.back() )
         ;
   }

   errormsg = "US_Fractal_Dimension::compute_box_counting() - not yet implemented";

   return true;
}

