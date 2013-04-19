#include "shd.h"

int main( int argc, char **argv )
{
   unsigned int max_harmonics = 20;

   vector < shd_point >         model;
   vector < vector < double > > F;
   vector < double >            q( 20 );
   vector < double >            I;

   SHD tSHD( max_harmonics, model, F, q, I, 0 );

   printf( "done\n" );
   return 0;
}
             

