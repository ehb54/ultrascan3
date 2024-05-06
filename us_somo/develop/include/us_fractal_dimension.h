#ifndef US_FRACTAL_DIMENSION_H
#define US_FRACTAL_DIMENSION_H

#include <set>
#include <qdebug.h>

class pointmass
{
 public:
   double  axis[3];
   double  mass;
   QString name;
   // double radius;
   bool operator < ( const pointmass & objIn ) const
   {
      return 
         axis[ 0 ] < objIn.axis[ 0 ] ||
         ( axis[ 0 ] == objIn.axis[ 0 ] &&
           axis[ 1 ] <  objIn.axis[ 1 ] ) ||
         ( axis[ 0 ] == objIn.axis[ 0 ] &&
           axis[ 1 ] == objIn.axis[ 1 ] &&
           axis[ 2 ] <  objIn.axis[ 2 ] );
   }

   bool operator == ( const pointmass & objIn ) const
   {
      return 
         axis[ 0 ] == objIn.axis[ 0 ] &&
         axis[ 1 ] == objIn.axis[ 1 ] &&
         axis[ 2 ] == objIn.axis[ 2 ]
         ;
   }
};


#define UNICODE_ANGSTROM u8"\u212B"
#define UNICODE_ANGSTROM_QS QString( "%1" ).arg( UNICODE_ANGSTROM )

using namespace std;

class US_Fractal_Dimension {
 public:
   US_Fractal_Dimension();
   ~US_Fractal_Dimension();

   enum methods : int {
      USFD_BOX_MODEL = 0
     ,USFD_ENRIGHT = 1
   };

   bool compute(
                enum US_Fractal_Dimension::methods   method
                ,const vector < pointmass >        & points
                ,double                              angstrom_start
                ,double                              angstrom_end
                ,double                              angstrom_steps
                ,double                            & fd                 // computed fractal dimension
                ,vector < vector < double > >      & x                  // x coordinate of plots
                ,vector < vector < double > >      & y                  // y coordinate of plots
                ,QString                           & x_title            // title of x axis
                ,QString                           & y_title            // title of y axis
                ,QString                           & type               // string type
                ,QString                           & errormsg           // errormsg is set if false returned
                );

   // list points for debugging
   QStringList list_points( const vector < pointmass > & points );

 private:

   bool compute_box_counting(
                             const vector < pointmass >    & points
                             ,double                         angstrom_start
                             ,double                         angstrom_end
                             ,double                         angstrom_steps
                             ,double                       & fd                 // computed fractal dimension
                             ,vector < vector < double > > & x                  // x coordinate of plots
                             ,vector < vector < double > > & y                  // y coordinate of plots
                             ,QString                      & x_title            // title of x axis
                             ,QString                      & y_title            // title of y axis
                             ,QString                      & errormsg           // errormsg is set if false returned
                             );

   // as per Enright & Leitner Phys. Rev. E 71, 011912 (2005)

   bool compute_enright(
                        const vector < pointmass >    & points
                        ,double                         angstrom_start
                        ,double                         angstrom_end
                        ,double                         angstrom_steps
                        ,double                       & fd                 // computed fractal dimension
                        ,vector < vector < double > > & x                  // x coordinate of plots
                        ,vector < vector < double > > & y                  // y coordinate of plots
                        ,QString                      & x_title            // title of x axis
                        ,QString                      & y_title            // title of y axis
                        ,QString                      & errormsg           // errormsg is set if false returned
                        );
};

#endif
