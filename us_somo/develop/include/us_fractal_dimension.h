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

using namespace std;

class US_Fractal_Dimension {
 public:
   US_Fractal_Dimension();
   ~US_Fractal_Dimension();

   enum methods : int {
      USFD_BOX_MODEL    = 0
      ,USFD_BOX_ALT      = 1
      ,USFD_BOX_MASS     = 2
      ,USFD_ENRIGHT      = 3
      ,USFD_ENRIGHT_FULL = 4
      ,USFD_ROLL_SPHERE  = 5
   };

   bool compute(
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
                );

   static QString method_name( enum US_Fractal_Dimension::methods method ) {
      switch ( method ) {
      case USFD_BOX_MODEL    : return "Box"; break;
      case USFD_BOX_ALT      : return "Box Alternate"; break;
      case USFD_BOX_MASS     : return "Box Mass"; break;
      case USFD_ENRIGHT      : return "Mass fractal D (slice)"; break;
      case USFD_ENRIGHT_FULL : return "Mass fractal D (full)"; break;
      case USFD_ROLL_SPHERE  : return "Surface fractal D"; break;
      default                : return "Error - unknown method"; break;
      }
   };

   // list points for debugging
   QStringList list_points( const vector < pointmass > & points );

   set < QString > mass_atoms_set();
   QStringList mass_atoms_qstringlist();

 private:
   set < QString > mass_atoms;

   bool compute_box_counting(
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
                             );

   // as compute_box_counting() using size vs volume
   bool compute_box_alternate(
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
                              );


   // as compute_box_counting() adding mass effects
   bool compute_box_mass(
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
                         );

   // as per Enright & Leitner Phys. Rev. E 71, 011912 (2005)

   bool compute_enright(
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
                        );
};

#endif
