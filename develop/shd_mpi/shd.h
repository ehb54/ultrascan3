#ifndef SHD_H
#define SHD_H

#include <math.h>
#include <iostream>
#include <algorithm>
#include <vector>
#include <complex>
#include <string>

using namespace std;

#include "shd_sh.h"

#ifndef M_PI
#  define M_PI       3.14159265358979323846
#endif

#ifndef M_4PI
#  define M_4PI       (4e0 * 3.14159265358979323846)
#endif

class shd_point
{
 public:
   float x[ 3 ];
   int   ff_type;
   bool operator < ( const shd_point & objIn ) const
   {
      return
         x[ 0 ] < objIn.x[ 0 ] ||
         ( x[ 0 ] == objIn.x[ 0 ] &&
           x[ 1 ] <  objIn.x[ 1 ] ) ||
         ( x[ 0 ] == objIn.x[ 0 ] &&
           x[ 1 ] == objIn.x[ 1 ] &&
           x[ 2 ] <  objIn.x[ 2 ] );
   }

   bool operator == ( const shd_point & objIn ) const
   {
      return
         x[ 0 ] == objIn.x[ 0 ] &&
         x[ 1 ] == objIn.x[ 1 ] &&
         x[ 2 ] == objIn.x[ 2 ];
   }
};

struct shd_data
{
   float rtp[ 3 ];
   vector < complex < float > > A1v;
};


/*!
 * \class SHD
 * \brief SHD computes the Debye function using spherical harmonics
 * \author Emre Brookes
 * \version 0.1
 * \date April 2013
 * \copyright BSD
 */

class SHD
{
 private:

   vector < shd_point >                    model;
   vector < vector < double > >            F;
   vector < double >                       q;
   vector < double >                       I;

   unsigned int                            q_points;

   unsigned int                            max_harmonics;
   unsigned int                            no_harmonics;

   vector < complex < float > >            ccY;
   vector < complex < float > >            ccA1v;
   vector < double >                       ccJ;

   complex < float >                       i_; // ( 0e0, 1e0 );
   vector < complex < float > >            i_l;
   vector < complex < float > >            i_k;

   unsigned int                            J_points;
   unsigned int                            Y_points;
   unsigned int                            q_Y_points;

   vector <  complex < float > >           A1v0;

   string              error_msg;

   int                 debug_level;

   // void                debug             ( int level, string str );

 public:
   // 

   /*!
    * \brief SHD constructor
    * \param max_harmonics is the maximum number of harmonics used
    * \param model is vector of shd_points which contain the coordinates and the precomputed structure factors
    * they must be precomputed for each type in the model
    * they also must already have any excluded volume subtraction computed
    * \param q     is a vector of points determining the resulting grid for I 
    * \param I              is a vector of intensities returned
    * \param debug_level    zero provides no debugging
    */

   SHD                 ( 
                        unsigned int max_harmonics,
                        vector < shd_point > & model, 
                        vector < vector < double > > & F,
                        vector < double > & q,
                        vector < double > & I, 
                        int debug_level = 0
                        );

   /*!
    * \brief SHD Destructor
    */
   ~SHD                ();

   /*!
    * \brief compute_amplitudes
    * \param model is the vector of model points
    */

   bool                compute_amplitudes( vector < shd_point > & model, 
                                           vector < complex < float > > & Av );

};

#endif // SHD
