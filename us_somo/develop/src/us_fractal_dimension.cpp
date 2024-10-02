#include "../include/us_fractal_dimension.h"
#include "../include/us_vector.h"
#include "../include/us_unicode.h"

#include <qtextstream.h>

#define USFD_MIN_POINTS       10


US_Fractal_Dimension::US_Fractal_Dimension() {
   qDebug() << "US_Fractal_Dimension() constructor";
   mass_atoms = { "CA", "O5", "O5*", "O5'" };
}

US_Fractal_Dimension::~US_Fractal_Dimension() {
   qDebug() << "US_Fractal_Dimension() destructor";
}

bool US_Fractal_Dimension::compute(
                                   enum US_Fractal_Dimension::methods         method
                                   ,const vector < pointmass >              & points
                                   ,double                                    angstrom_start
                                   ,double                                    angstrom_end
                                   ,double                                    angstrom_steps
                                   ,double                                    enright_ca_pct_start
                                   ,double                                    enright_ca_pct_end
                                   ,double                                  & fd                    // computed fractal dimension
                                   ,vector < vector < vector < double > > > & x                     // x coordinate of plots
                                   ,vector < vector < vector < double > > > & y                     // y coordinate of plots
                                   ,QString                                 & x_title               // title of x axis
                                   ,QString                                 & y_title               // title of y axis
                                   ,QString                                 & type                  // string type
                                   ,QString                                 & errormsg              // errormsg is set if false returned
                                   ) {
   type = method_name( method );
   switch ( method ) {
   case USFD_BOX_MODEL :
      return compute_box_counting(
                                  points
                                  ,angstrom_start
                                  ,angstrom_end
                                  ,angstrom_steps
                                  ,fd
                                  ,x
                                  ,y
                                  ,x_title
                                  ,y_title
                                  ,errormsg
                                  );
      break;

   case USFD_BOX_ALT :
      return compute_box_alternate(
                                  points
                                  ,angstrom_start
                                  ,angstrom_end
                                  ,angstrom_steps
                                  ,fd
                                  ,x
                                  ,y
                                  ,x_title
                                  ,y_title
                                  ,errormsg
                                  );
      break;

   case USFD_BOX_MASS :
      return compute_box_mass(
                              points
                              ,angstrom_start
                              ,angstrom_end
                              ,angstrom_steps
                              ,fd
                              ,x
                              ,y
                              ,x_title
                              ,y_title
                              ,errormsg
                              );
      break;

   case USFD_ENRIGHT :
      return compute_enright(
                             points
                             ,angstrom_start
                             ,angstrom_end
                             ,angstrom_steps
                             ,enright_ca_pct_start
                             ,enright_ca_pct_end
                             ,fd
                             ,x
                             ,y
                             ,x_title
                             ,y_title
                             ,errormsg
                             );
      break;

   case USFD_ENRIGHT_FULL :
      {
         vector < vector < vector < double > > > ret_x;
         vector < vector < vector < double > > > ret_y;

         for ( double enright_ca_pct = 0; enright_ca_pct <= 90; enright_ca_pct += 10 ) {
            x.clear();
            y.clear();
            
            if ( !compute_enright(
                                  points
                                  ,angstrom_start
                                  ,angstrom_end
                                  ,angstrom_steps
                                  ,enright_ca_pct
                                  ,enright_ca_pct + 10
                                  ,fd
                                  ,x
                                  ,y
                                  ,x_title
                                  ,y_title
                                  ,errormsg
                                  ) ) {
               return false;
            }

            if ( !x.size() ) {
               errormsg =
                  QString( "Enright failed to provide a result for slice %1 - %2 %" )
                  .arg( enright_ca_pct )
                  .arg( enright_ca_pct + 10 )
                  ;
            }               

            ret_x.push_back( x[ 0 ] );
            ret_y.push_back( y[ 0 ] );
         }

         x = ret_x;
         y = ret_y;

         return true;
      }
      break;
      
   default:
      errormsg = "US_Fractal_Dimension::compute() invalid or unsupported method";
      return false;
   }
   errormsg = "US_Fractal_Dimension::compute() should never get here";
   return false;
}
                                
bool US_Fractal_Dimension::compute_box_counting(
                                                const vector < pointmass >               & points
                                                ,double                                    angstrom_start
                                                ,double                                    angstrom_end
                                                ,double                                    angstrom_steps
                                                ,double                                  & fd                 // computed fractal dimension
                                                ,vector < vector < vector < double > > > & x                  // x coordinate of plots
                                                ,vector < vector < vector < double > > > & y                  // y coordinate of plots
                                                ,QString                                 & x_title            // title of x axis
                                                ,QString                                 & y_title            // title of y axis
                                                ,QString                                 & errormsg           // errormsg is set if false returned
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

   if ( angstrom_steps <= 0 ) {
      errormsg = QString( "Invalid number of steps (%1)" ).arg( angstrom_steps );
      return false;
   }

   double stepsize = ( angstrom_end - angstrom_start ) / angstrom_steps;
   
   if ( stepsize <= 0 ) {
      errormsg = QString( "Invalid computed stepsize  (%1)" ).arg( stepsize );
      return false;
   }

   qDebug() << QString( "stepsize %1" ).arg( stepsize );

   x.clear();
   y.clear();
   x.resize( 1 );
   y.resize( 1 );
   x[ 0 ].resize( 1 );
   y[ 0 ].resize( 1 );

   for ( double edge_size = angstrom_start; edge_size <= angstrom_end; edge_size += stepsize ) {
      // qDebug() << QString( "US_Fractal_Dimension::compute_box_counting() : processing edge_size %1" ).arg( edge_size );
      set < vector < int > > occupied;

      double edge_inverse = 1e0 / edge_size;
      
      for ( auto const & p : points ) {
         occupied.insert( {
               (int) ( p.axis[ 0 ] * edge_inverse )
                  ,(int) ( p.axis[ 1 ] * edge_inverse )
                  ,(int) ( p.axis[ 2 ] * edge_inverse )
                  }
            );
      }

      x[ 0 ][ 0 ].push_back( log( pow( edge_size, 3 ) ) );
      y[ 0 ][ 0 ].push_back( log( occupied.size() ) );
                   
      QTextStream( stdout ) << QString( "box edge size %1 [%2], number of occupied boxes %3, volume of one box %4 [%5^3] ln(vol) %6 ln(number of occupied boxes) %7\n" )
         .arg( edge_size )
         .arg( UNICODE_ANGSTROM )
         .arg( occupied.size() )
         .arg( pow( edge_size, 3 ) )
         .arg( UNICODE_ANGSTROM )
         .arg( x[ 0 ][ 0 ].back() )
         .arg( y[ 0 ][ 0 ].back() )
         ;
   }

   errormsg = "US_Fractal_Dimension::compute_box_counting() - not fully implemented";

   x_title = "log(volume of each box [" + UNICODE_ANGSTROM_QS + "^3])";
   y_title = "log(number of boxes)";

   return true;
}

bool US_Fractal_Dimension::compute_box_alternate(
                                                 const vector < pointmass >               & points
                                                 ,double                                    angstrom_start
                                                 ,double                                    angstrom_end
                                                 ,double                                    angstrom_steps
                                                 ,double                                  & fd                 // computed fractal dimension
                                                 ,vector < vector < vector < double > > > & x                  // x coordinate of plots
                                                 ,vector < vector < vector < double > > > & y                  // y coordinate of plots
                                                 ,QString                                 & x_title            // title of x axis
                                                 ,QString                                 & y_title            // title of y axis
                                                 ,QString                                 & errormsg           // errormsg is set if false returned
                                                 ) {

   qDebug() << "US_Fractal_Dimension::compute_box_alternate()";

   if ( points.size() < USFD_MIN_POINTS ) {
      errormsg =
         QString( "Invalid number of points for box alternate %1 (minimum %2)" )
         .arg( points.size() )
         .arg( USFD_MIN_POINTS )
         ;
      return false;
   }

   if ( angstrom_steps <= 0 ) {
      errormsg = QString( "Invalid number of steps (%1)" ).arg( angstrom_steps );
      return false;
   }

   double stepsize = ( angstrom_end - angstrom_start ) / angstrom_steps;
   
   if ( stepsize <= 0 ) {
      errormsg = QString( "Invalid computed stepsize  (%1)" ).arg( stepsize );
      return false;
   }

   qDebug() << QString( "stepsize %1" ).arg( stepsize );

   x.clear();
   y.clear();
   x.resize( 1 );
   y.resize( 1 );
   x[ 0 ].resize( 1 );
   y[ 0 ].resize( 1 );

   for ( double edge_size = angstrom_start; edge_size <= angstrom_end; edge_size += stepsize ) {
      // qDebug() << QString( "US_Fractal_Dimension::compute_box_alternate() : processing edge_size %1" ).arg( edge_size );
      set < vector < int > > occupied;

      double edge_inverse = 1e0 / edge_size;
      
      for ( auto const & p : points ) {
         occupied.insert( {
               (int) ( p.axis[ 0 ] * edge_inverse )
                  ,(int) ( p.axis[ 1 ] * edge_inverse )
                  ,(int) ( p.axis[ 2 ] * edge_inverse )
                  }
            );
      }

      x[ 0 ][ 0 ].push_back( log( edge_size ) );
      y[ 0 ][ 0 ].push_back( log( occupied.size() ) );
                   
      QTextStream( stdout ) << QString( "box edge size %1 [%2], number of occupied boxes %3, volume of one box %4 [%5^3] ln(vol) %6 ln(number of occupied boxes) %7\n" )
         .arg( edge_size )
         .arg( UNICODE_ANGSTROM )
         .arg( occupied.size() )
         .arg( pow( edge_size, 3 ) )
         .arg( UNICODE_ANGSTROM )
         .arg( x[ 0 ][ 0 ].back() )
         .arg( y[ 0 ][ 0 ].back() )
         ;
   }

   errormsg = "US_Fractal_Dimension::compute_box_alternate() - not fully implemented";

   x_title = "log(size of each box [" + UNICODE_ANGSTROM_QS + "])";
   y_title = "log(number of boxes)";

   return true;
}

QStringList US_Fractal_Dimension::list_points( const vector < pointmass > & points ) {
   QStringList result;
   result.push_back( "x y z m name" );
   for ( auto const & p : points ) {
      result.push_back(
                       QString( "%1 %2 %3 %4 %5" )
                       .arg( p.axis[ 0 ] )
                       .arg( p.axis[ 0 ] )
                       .arg( p.axis[ 0 ] )
                       .arg( p.mass )
                       .arg( p.name )
                       );
   }
   return result;
}

bool US_Fractal_Dimension::compute_enright(
                                           const vector < pointmass >               & points
                                           ,double                                    angstrom_start
                                           ,double                                    angstrom_end
                                           ,double                                    angstrom_steps
                                           ,double                                    enright_ca_pct_start
                                           ,double                                    enright_ca_pct_end
                                           ,double                                  & fd                 // computed fractal dimension
                                           ,vector < vector < vector < double > > > & x                  // x coordinate of plots
                                           ,vector < vector < vector < double > > > & y                  // y coordinate of plots
                                           ,QString                                 & x_title            // title of x axis
                                           ,QString                                 & y_title            // title of y axis
                                           ,QString                                 & errormsg           // errormsg is set if false returned
                                           ) {

   qDebug() << "US_Fractal_Dimension::compute_enright()";

   if ( points.size() < USFD_MIN_POINTS ) {
      errormsg =
         QString( "Invalid number of points for Enright %1 (minimum %2)" )
         .arg( points.size() )
         .arg( USFD_MIN_POINTS )
         ;
      return false;
   }

   if ( angstrom_steps <= 0 ) {
      errormsg = QString( "Invalid number of steps (%1)" ).arg( angstrom_steps );
      return false;
   }

   double stepsize = ( angstrom_end - angstrom_start ) / angstrom_steps;
   
   if ( stepsize <= 0 ) {
      errormsg = QString( "Invalid computed stepsize  (%1)" ).arg( stepsize );
      return false;
   }

   qDebug() << QString( "stepsize %1" ).arg( stepsize );

   // compute COM

   vector < double > com  = { 0, 0, 0 };
   double            mass = 0;

   for ( auto const & p : points ) {
      mass     += p.mass;
      for ( size_t i = 0; i < 3; ++i ) {
         com[ i ] += p.mass * p.axis[ i ];
      }
   }
   for ( size_t i = 0; i < 3; ++i ) {
      com[ i ] /= mass;
   }
   
   // QTextStream( stdout ) << list_points( points ).join( "\n" ) << "\n";
   
   qDebug() << QString( "com [%1 %2 %3]" ).arg( com[ 0 ] ).arg( com[1] ).arg( com[2] );

   // find distances^2 of each CA to center

   vector < size_t > ca_ordered_by_dist2;

   {
      map < double, size_t > ca_dist2_to_center;
      for ( size_t i = 0; i < points.size(); ++i ) {
         if ( mass_atoms.count( points[ i ].name ) ) {
            ca_dist2_to_center[
                               pow( points[ i ].axis[ 0 ] - com[ 0 ], 2 )
                               + pow( points[ i ].axis[ 1 ] - com[ 1 ], 2 )
                               + pow( points[ i ].axis[ 2 ] - com[ 2 ], 2 )
                               ] = i;
         }
      }


      for ( auto const & ca : ca_dist2_to_center ) {
         ca_ordered_by_dist2.push_back( ca.second );
      }

      // debug CA distances to center in order
      // {
      //    qDebug() << "dist^2 x y z I name";
      
      //    for ( auto const & ca : ca_dist2_to_center ) {
      //       qDebug() << QString( "%1 %2 %3 %4 %5" )
      //          .arg( ca.first )
      //          .arg( points[ ca.second ].axis[ 0 ] )
      //          .arg( points[ ca.second ].axis[ 1 ] )
      //          .arg( points[ ca.second ].axis[ 2 ] )
      //          .arg( points[ ca.second ].name )
      //          ;
      //    }
      // }
   }

   if ( !ca_ordered_by_dist2.size() ) {
      errormsg = "No C%1 atoms found";
      return false;
   }

   // global limits
   size_t ca_index_start = (size_t) ( (double) ca_ordered_by_dist2.size() * enright_ca_pct_start / 100.0 );
   size_t ca_index_end   = (size_t) ( (double) ca_ordered_by_dist2.size() * enright_ca_pct_end / 100.0 );
   
   x.clear();
   y.clear();
   x.resize( 1 );
   y.resize( 1 );

   for ( size_t i = ca_index_start; i < ca_index_end; ++i ) {

      vector < double > ca_com  = {
         points[ ca_ordered_by_dist2[ i ] ].axis[ 0 ]
         ,points[ ca_ordered_by_dist2[ i ] ].axis[ 1 ]
         ,points[ ca_ordered_by_dist2[ i ] ].axis[ 2 ]
      };

      // qDebug() << QString( "ca_com %1 [%2 %3 %4]" ).arg( i ).arg( ca_com[ 0 ] ).arg( ca_com[1] ).arg( ca_com[2] );

      vector < double > tmp_x;
      vector < double > tmp_y;

      for ( double radius = angstrom_start; radius <= angstrom_end; radius += stepsize ) {
         // qDebug() << QString( "US_Fractal_Dimension::compute_enright() : processing size %1" ).arg( radius );
         double radius2 = radius * radius;
         double mass    = 0;
         
         for ( auto const & p : points ) {
            if (
                pow( p.axis[ 0 ] - ca_com[ 0 ], 2 )
                + pow( p.axis[ 1 ] - ca_com[ 1 ], 2 )
                + pow( p.axis[ 2 ] - ca_com[ 2 ], 2 )
                <= radius2
                ) {
               mass += p.mass;
            }
         }
         
         tmp_x.push_back( log10( radius ) );
         tmp_y.push_back( log10( mass ) );
      }
      x[ 0 ].push_back( tmp_x );
      y[ 0 ].push_back( tmp_y );

      // US_Vector::printvector2( QString( "enright %1, x,y" ).arg( i ), x[ i ], y[ i ] );
   }

   if ( !x.size() ) {
      errormsg = "No C%1 atoms found in percent slice";
      return false;
   }
   errormsg = "US_Fractal_Dimension::compute_enright() - not fully implemented";

   x_title = "log10( R [" + UNICODE_ANGSTROM_QS + "] )";
   y_title = "log10( M [Da] )";

   return true;
}


bool US_Fractal_Dimension::compute_box_mass(
                                            const vector < pointmass >               & points
                                            ,double                                    angstrom_start
                                            ,double                                    angstrom_end
                                            ,double                                    angstrom_steps
                                            ,double                                  & fd                 // computed fractal dimension
                                            ,vector < vector < vector < double > > > & x                  // x coordinate of plots
                                            ,vector < vector < vector < double > > > & y                  // y coordinate of plots
                                            ,QString                                 & x_title            // title of x axis
                                            ,QString                                 & y_title            // title of y axis
                                            ,QString                                 & errormsg           // errormsg is set if false returned
                                            ) {

   qDebug() << "US_Fractal_Dimension::compute_box_mass()";

   if ( points.size() < USFD_MIN_POINTS ) {
      errormsg =
         QString( "Invalid number of points for box mass %1 (minimum %2)" )
         .arg( points.size() )
         .arg( USFD_MIN_POINTS )
         ;
      return false;
   }

   if ( angstrom_steps <= 0 ) {
      errormsg = QString( "Invalid number of steps (%1)" ).arg( angstrom_steps );
      return false;
   }

   double stepsize = ( angstrom_end - angstrom_start ) / angstrom_steps;
   
   if ( stepsize <= 0 ) {
      errormsg = QString( "Invalid computed stepsize  (%1)" ).arg( stepsize );
      return false;
   }

   qDebug() << QString( "stepsize %1" ).arg( stepsize );

   x.clear();
   y.clear();
   x.resize( 1 );
   y.resize( 1 );
   x[ 0 ].resize( 1 );
   y[ 0 ].resize( 1 );

   double mass = 0;
   for ( auto const & p : points ) {
      mass += p.mass;
   }

   for ( double edge_size = angstrom_start; edge_size <= angstrom_end; edge_size += stepsize ) {
      // qDebug() << QString( "US_Fractal_Dimension::compute_box_mass() : processing edge_size %1" ).arg( edge_size );
      set < vector < int > > occupied;

      double edge_inverse = 1e0 / edge_size;
      
      for ( auto const & p : points ) {
         occupied.insert( {
               (int) ( p.axis[ 0 ] * edge_inverse )
                  ,(int) ( p.axis[ 1 ] * edge_inverse )
                  ,(int) ( p.axis[ 2 ] * edge_inverse )
                  } );
      }

      x[ 0 ][ 0 ].push_back( log( edge_size ) );
      y[ 0 ][ 0 ].push_back( log( mass / (double)occupied.size() ) );
                   
      QTextStream( stdout ) << QString( "box edge size %1 [%2], number of occupied boxes %3, volume of one box %4 [%5^3] ln(vol) %6 ln(number of occupied boxes) %7\n" )
         .arg( edge_size )
         .arg( UNICODE_ANGSTROM )
         .arg( occupied.size() )
         .arg( pow( edge_size, 3 ) )
         .arg( UNICODE_ANGSTROM )
         .arg( x[ 0 ][ 0 ].back() )
         .arg( y[ 0 ][ 0 ].back() )
         ;
   }

   errormsg = "US_Fractal_Dimension::compute_box_mass() - not fully implemented";

   x_title = "log(size of each box [" + UNICODE_ANGSTROM_QS + "])";
   y_title = "log(average mass per box)";

   return true;
}


set < QString > US_Fractal_Dimension::mass_atoms_set() {
   return mass_atoms;
}

QStringList US_Fractal_Dimension::mass_atoms_qstringlist() {
   QStringList result;
   for ( auto const & e : mass_atoms ) {
      result << e;
   }
   return result;
}

   
