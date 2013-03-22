#include "../include/us_vector.h"

void US_Vector::printvector( QString qs, vector < unsigned int > x )
{
   cout << QString( "%1: size %2:" ).arg( qs ).arg( x.size() );
   for ( unsigned int i = 0; i < x.size(); i++ )
   {
      cout << QString( " %1" ).arg( x[ i ] );
   }
   cout << endl;
}

void US_Vector::printvector( QString qs, vector < double > x, int digits )
{
   cout << QString( "%1: size %2:" ).arg( qs ).arg( x.size() );
   for ( unsigned int i = 0; i < x.size(); i++ )
   {
      cout << QString( " %1" ).arg( x[ i ], 0, 'g', digits );
   }
   cout << endl;
}

void US_Vector::printvector( QString qs, vector < bool > x )
{
   cout << QString( "%1: size %2:" ).arg( qs ).arg( x.size() );
   for ( unsigned int i = 0; i < x.size(); i++ )
   {
      cout << QString( " %1" ).arg( x[ i ] ? "true" : "false" );
   }
   cout << endl;
}

vector < double > US_Vector::intersection( vector < double > &x,  vector < double > &y )
{
   map < double, bool > map_x;
   for ( unsigned int i = 0; i < ( unsigned int ) x.size(); i++ )
   {
      map_x[ x[ i ] ] = true;
   }
   vector < double > result;
   for ( unsigned int i = 0; i < ( unsigned int ) y.size(); i++ )
   {
      if ( map_x.count( y[ i ] ) )
      {
         result.push_back( y[ i ] );
      }
   }
   return result;
}

vector < double > US_Vector::intersection( vector < vector < double > > &x )
{
   // make a common grid
   
   vector < double > result;

   if ( !x.size() )
   {
      return result;
   }

   if ( x.size() == 1 )
   {
      return x[ 0 ];
   }

   result = x[ 0 ];
   for ( unsigned int i = 1; i < ( unsigned int ) x.size(); i++ )
   {
      result = intersection( result, x[ i ] );
   }
   return result;
}

vector < double > US_Vector::vunion( vector < double > &x,  vector < double > &y )
{
   list < double > lx;
   list < double > ly;
   
   for ( unsigned int i = 0; i < ( unsigned int ) x.size(); i++ )
   {
      lx.push_back( x[ i ] );
   }
   for ( unsigned int i = 0; i < ( unsigned int ) y.size(); i++ )
   {
      ly.push_back( y[ i ] );
   }

   lx.merge( ly );
   lx.unique();

   vector < double > result;
   for ( list < double >::iterator it = lx.begin();
         it != lx.end();
         it++ )
   {
      result.push_back( *it );
   }
   return result;
}

vector < double > US_Vector::vunion( vector < vector < double > > &x )
{
   // make a common grid
   
   vector < double > result;

   if ( !x.size() )
   {
      return result;
   }

   if ( x.size() == 1 )
   {
      return x[ 0 ];
   }

   result = x[ 0 ];
   for ( unsigned int i = 1; i < ( unsigned int ) x.size(); i++ )
   {
      result = vunion( result, x[ i ] );
   }
   return result;
}
