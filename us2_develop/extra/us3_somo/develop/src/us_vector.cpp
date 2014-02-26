#include "../include/us_vector.h"

// note: this program uses cout and/or cerr and this should be replaced

static std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const QString& str) { 
   return os << qPrintable(str);
}

void US_Vector::printvector( QString qs, vector < unsigned int > x )
{
   cout << QString( "%1: size %2:" ).arg( qs ).arg( x.size() );
   for ( unsigned int i = 0; i < x.size(); i++ )
   {
      cout << QString( " %1" ).arg( x[ i ] );
   }
   cout << endl;
}

void US_Vector::printvector( QString qs, vector < int > x )
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

void US_Vector::printvector2( QString qs, vector < double > x, vector < double > y, int digits )
{
   cout << QString( "%1: size %2 %3:\n" ).arg( qs ).arg( x.size() ).arg( y.size() );
   unsigned int max_size = ( unsigned int )( x.size() > y.size() ? x.size() : y.size() );
   for ( unsigned int i = 0; i < max_size; i++ )
   {
      cout << QString( "\t%1 %2\n" )
         .arg( ( x.size() > i ) ? QString( "%1" ).arg( x[ i ], 0, 'g', digits ) : QString( "n/a" ) )
         .arg( ( y.size() > i ) ? QString( "%1" ).arg( y[ i ], 0, 'g', digits ) : QString( "n/a" ) )
         ;
   }
}

void US_Vector::printvector3( QString qs, vector < double > x, vector < double > y, vector < double > z, int digits )
{
   cout << QString( "%1: size %2 %3:\n" ).arg( qs ).arg( x.size() ).arg( y.size() );
   unsigned int max_size = ( unsigned int )( x.size() > y.size() ? x.size() : y.size() );
   max_size = max_size > z.size() ? max_size : z.size();
   for ( unsigned int i = 0; i < max_size; i++ )
   {
      cout << QString( "\t%1 %2 %3\n" )
         .arg( ( x.size() > i ) ? QString( "%1" ).arg( x[ i ], 0, 'g', digits ) : QString( "n/a" ) )
         .arg( ( y.size() > i ) ? QString( "%1" ).arg( y[ i ], 0, 'g', digits ) : QString( "n/a" ) )
         .arg( ( z.size() > i ) ? QString( "%1" ).arg( z[ i ], 0, 'g', digits ) : QString( "n/a" ) )
         ;
   }
}

void US_Vector::printvector2( QString qs, vector < int > x, vector < int > y )
{
   cout << QString( "%1: size %2 %3:\n" ).arg( qs ).arg( x.size() ).arg( y.size() );
   unsigned int max_size = ( unsigned int )( x.size() > y.size() ? x.size() : y.size() );
   for ( unsigned int i = 0; i < max_size; i++ )
   {
      cout << QString( "\t%1 %2\n" )
         .arg( ( x.size() > i ) ? QString( "%1" ).arg( x[ i ] ) : QString( "n/a" ) )
         .arg( ( y.size() > i ) ? QString( "%1" ).arg( y[ i ] ) : QString( "n/a" ) )
         ;
   }
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

QString US_Vector::qs_vector( QString qs, vector < unsigned int > x )
{
   QString result;
   result += QString( "%1: size %2:" ).arg( qs ).arg( x.size() );
   for ( unsigned int i = 0; i < x.size(); i++ )
   {
      result += QString( " %1" ).arg( x[ i ] );
   }
   result += "\n";
   return result;
}

QString US_Vector::qs_vector( QString qs, vector < int > x )
{
   QString result;
   result += QString( "%1: size %2:" ).arg( qs ).arg( x.size() );
   for ( unsigned int i = 0; i < x.size(); i++ )
   {
      result += QString( " %1" ).arg( x[ i ] );
   }
   result += "\n";
   return result;
}

QString US_Vector::qs_vector( QString qs, vector < double > x, int digits )
{
   QString result;
   result += QString( "%1: size %2:" ).arg( qs ).arg( x.size() );
   for ( unsigned int i = 0; i < x.size(); i++ )
   {
      result += QString( " %1" ).arg( x[ i ], 0, 'g', digits );
   }
   result += "\n";
   return result;
}

QString US_Vector::qs_vector2( QString qs, vector < double > x, vector < double > y, int digits )
{
   QString result;
   result += QString( "%1: size %2 %3:\n" ).arg( qs ).arg( x.size() ).arg( y.size() );
   unsigned int max_size = ( unsigned int )( x.size() > y.size() ? x.size() : y.size() );
   for ( unsigned int i = 0; i < max_size; i++ )
   {
      result += QString( "\t%1 %2\n" )
         .arg( ( x.size() > i ) ? QString( "%1" ).arg( x[ i ], 0, 'g', digits ) : QString( "n/a" ) )
         .arg( ( y.size() > i ) ? QString( "%1" ).arg( y[ i ], 0, 'g', digits ) : QString( "n/a" ) )
         ;
   }
   return result;
}

QString US_Vector::qs_vector2( QString qs, vector < int > x, vector < int > y )
{
   QString result;
   result += QString( "%1: size %2 %3:\n" ).arg( qs ).arg( x.size() ).arg( y.size() );
   unsigned int max_size = ( unsigned int )( x.size() > y.size() ? x.size() : y.size() );
   for ( unsigned int i = 0; i < max_size; i++ )
   {
      result += QString( "\t%1 %2\n" )
         .arg( ( x.size() > i ) ? QString( "%1" ).arg( x[ i ] ) : QString( "n/a" ) )
         .arg( ( y.size() > i ) ? QString( "%1" ).arg( y[ i ] ) : QString( "n/a" ) )
         ;
   }
   return result;
}

QString US_Vector::qs_vector( QString qs, vector < bool > x )
{
   QString result;
   result += QString( "%1: size %2:" ).arg( qs ).arg( x.size() );
   for ( unsigned int i = 0; i < x.size(); i++ )
   {
      result += QString( " %1" ).arg( x[ i ] ? "true" : "false" );
   }
   result += "\n";
   return result;
}
