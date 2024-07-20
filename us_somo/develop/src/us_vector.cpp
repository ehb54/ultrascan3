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

void US_Vector::printvector( QString qs, vector < QString > & x )
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

void US_Vector::printvector2( QString qs, vector < double > x, vector < double > y, int digits, unsigned int limit )
{
   cout << QString( "%1: size %2 %3" ).arg( qs ).arg( x.size() ).arg( y.size() );
   unsigned int max_size = ( unsigned int )( x.size() > y.size() ? x.size() : y.size() );

   if ( limit && max_size > limit ) {
      max_size = limit;
      cout << QString( "; output limited to first %1 elements" ).arg( limit );
   }
   cout << ":\n";
   
   for ( unsigned int i = 0; i < max_size; i++ )
   {
      cout << QString( "\t%1 %2\n" )
         .arg( ( x.size() > i ) ? QString( "%1" ).arg( x[ i ], 0, 'g', digits ) : QString( "n/a" ) )
         .arg( ( y.size() > i ) ? QString( "%1" ).arg( y[ i ], 0, 'g', digits ) : QString( "n/a" ) )
         ;
   }
}

void US_Vector::printvector3( QString qs, vector < double > x, vector < double > y, vector < double > z, int digits, unsigned int limit )
{
   cout << QString( "%1: size %2 %3 %4" ).arg( qs ).arg( x.size() ).arg( y.size() ).arg( z.size() );
   unsigned int max_size = ( unsigned int )( x.size() > y.size() ? x.size() : y.size() );
   max_size = max_size > z.size() ? max_size : z.size();

   if ( limit && max_size > limit ) {
      max_size = limit;
      cout << QString( "; output limited to first %1 elements" ).arg( limit );
   }
   cout << ":\n";
   
   for ( unsigned int i = 0; i < max_size; i++ )
   {
      cout << QString( "\t%1 %2 %3\n" )
         .arg( ( x.size() > i ) ? QString( "%1" ).arg( x[ i ], 0, 'g', digits ) : QString( "n/a" ) )
         .arg( ( y.size() > i ) ? QString( "%1" ).arg( y[ i ], 0, 'g', digits ) : QString( "n/a" ) )
         .arg( ( z.size() > i ) ? QString( "%1" ).arg( z[ i ], 0, 'g', digits ) : QString( "n/a" ) )
         ;
   }
}

void US_Vector::printvector4( QString qs, vector < double > x, vector < double > y, vector < double > z, vector < double > s, int digits, unsigned int limit )
{
   cout << QString( "%1: size %2 %3 %4 %5" ).arg( qs ).arg( x.size() ).arg( y.size() ).arg( z.size() ).arg( s.size() );
   unsigned int max_size = ( unsigned int )( x.size() > y.size() ? x.size() : y.size() );
   max_size = max_size > z.size() ? max_size : z.size();
   max_size = max_size > s.size() ? max_size : s.size();

   if ( limit && max_size > limit ) {
      max_size = limit;
      cout << QString( "; output limited to first %1 elements" ).arg( limit );
   }
   cout << ":\n";
   
   for ( unsigned int i = 0; i < max_size; i++ )
   {
      cout << QString( "\t%1 %2 %3 %4\n" )
         .arg( ( x.size() > i ) ? QString( "%1" ).arg( x[ i ], 0, 'g', digits ) : QString( "n/a" ) )
         .arg( ( y.size() > i ) ? QString( "%1" ).arg( y[ i ], 0, 'g', digits ) : QString( "n/a" ) )
         .arg( ( z.size() > i ) ? QString( "%1" ).arg( z[ i ], 0, 'g', digits ) : QString( "n/a" ) )
         .arg( ( s.size() > i ) ? QString( "%1" ).arg( s[ i ], 0, 'g', digits ) : QString( "n/a" ) )
         ;
   }
}

void US_Vector::printvector5( QString qs, vector < double > x, vector < double > y, vector < double > z, vector < double > s, vector < double > t, int digits, unsigned int limit )
{
   cout << QString( "%1: size %2 %3 %4 %5 %6" ).arg( qs ).arg( x.size() ).arg( y.size() ).arg( z.size() ).arg( s.size() ).arg( t.size() );
   unsigned int max_size = ( unsigned int )( x.size() > y.size() ? x.size() : y.size() );
   max_size = max_size > z.size() ? max_size : z.size();
   max_size = max_size > s.size() ? max_size : s.size();
   max_size = max_size > t.size() ? max_size : t.size();

   if ( limit && max_size > limit ) {
      max_size = limit;
      cout << QString( "; output limited to first %1 elements" ).arg( limit );
   }
   cout << ":\n";
   
   for ( unsigned int i = 0; i < max_size; i++ )
   {
      cout << QString( "\t%1 %2 %3 %4 %5\n" )
         .arg( ( x.size() > i ) ? QString( "%1" ).arg( x[ i ], 0, 'g', digits ) : QString( "n/a" ) )
         .arg( ( y.size() > i ) ? QString( "%1" ).arg( y[ i ], 0, 'g', digits ) : QString( "n/a" ) )
         .arg( ( z.size() > i ) ? QString( "%1" ).arg( z[ i ], 0, 'g', digits ) : QString( "n/a" ) )
         .arg( ( s.size() > i ) ? QString( "%1" ).arg( s[ i ], 0, 'g', digits ) : QString( "n/a" ) )
         .arg( ( t.size() > i ) ? QString( "%1" ).arg( t[ i ], 0, 'g', digits ) : QString( "n/a" ) )
         ;
   }
}

void US_Vector::printvector7( QString qs, vector < double > x, vector < double > y, vector < double > z, vector < double > s, vector < double > t, vector < double > u, vector < double > v, int digits, unsigned int limit )
{
   cout << QString( "%1: size %2 %3 %4 %5 %6" ).arg( qs ).arg( x.size() ).arg( y.size() ).arg( z.size() ).arg( s.size() ).arg( t.size() );
   unsigned int max_size = ( unsigned int )( x.size() > y.size() ? x.size() : y.size() );
   max_size = max_size > z.size() ? max_size : z.size();
   max_size = max_size > s.size() ? max_size : s.size();
   max_size = max_size > t.size() ? max_size : t.size();
   max_size = max_size > u.size() ? max_size : u.size();
   max_size = max_size > v.size() ? max_size : v.size();

   if ( limit && max_size > limit ) {
      max_size = limit;
      cout << QString( "; output limited to first %1 elements" ).arg( limit );
   }
   cout << ":\n";
   
   for ( unsigned int i = 0; i < max_size; i++ )
   {
      cout << QString( "\t%1 %2 %3 %4 %5 %6 %7\n" )
         .arg( ( x.size() > i ) ? QString( "%1" ).arg( x[ i ], 0, 'g', digits ) : QString( "n/a" ) )
         .arg( ( y.size() > i ) ? QString( "%1" ).arg( y[ i ], 0, 'g', digits ) : QString( "n/a" ) )
         .arg( ( z.size() > i ) ? QString( "%1" ).arg( z[ i ], 0, 'g', digits ) : QString( "n/a" ) )
         .arg( ( s.size() > i ) ? QString( "%1" ).arg( s[ i ], 0, 'g', digits ) : QString( "n/a" ) )
         .arg( ( t.size() > i ) ? QString( "%1" ).arg( t[ i ], 0, 'g', digits ) : QString( "n/a" ) )
         .arg( ( u.size() > i ) ? QString( "%1" ).arg( u[ i ], 0, 'g', digits ) : QString( "n/a" ) )
         .arg( ( v.size() > i ) ? QString( "%1" ).arg( v[ i ], 0, 'g', digits ) : QString( "n/a" ) )
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

QString US_Vector::qs_mapqsqs( QString qs, map < QString, QString > & x ) 
{
   QString result;
   result += QString( "%1: size %2:" ).arg( qs ).arg( x.size() );
   for ( auto it = x.begin();
         it != x.end();
         ++it ) {
      result += QString( "\t%1 %2\n" ).arg( it->first ).arg( it->second );
   }
   return result;
}

QString US_Vector::qs_mapqsfloat( QString qs, map < QString, float > & x ) 
{
   QString result;
   result += QString( "%1: size %2:" ).arg( qs ).arg( x.size() );
   for ( auto it = x.begin();
         it != x.end();
         ++it ) {
      result += QString( "\t%1 %2\n" ).arg( it->first ).arg( it->second );
   }
   return result;
}

QString US_Vector::qs_mapqsdouble( QString qs, map < QString, double > & x ) 
{
   QString result;
   result += QString( "%1: size %2:" ).arg( qs ).arg( x.size() );
   for ( auto it = x.begin();
         it != x.end();
         ++it ) {
      result += QString( "\t%1 %2\n" ).arg( it->first ).arg( it->second );
   }
   return result;
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

QString US_Vector::qs_vector( vector < double > x, int digits )
{
   QString result;
   result += QString( "size %1:" ).arg( x.size() );
   for ( unsigned int i = 0; i < x.size(); i++ ) {
      result += QString( " %1" ).arg( x[ i ], 0, 'g', digits );
   }
   return result;
}

QString US_Vector::qs_vector( vector < float > x, int digits )
{
   QString result;
   result += QString( "size %1:" ).arg( x.size() );
   for ( unsigned int i = 0; i < x.size(); i++ ) {
      result += QString( " %1" ).arg( x[ i ], 0, 'g', digits );
   }
   return result;
}

QString US_Vector::qs_vector( QString qs, vector < double > x, int digits, int newline )
{
   QString result;
   result += QString( "%1: size %2:" ).arg( qs ).arg( x.size() );
   for ( unsigned int i = 0; i < x.size(); i++ )
   {
      result += QString( " %1" ).arg( x[ i ], 0, 'g', digits );
      if ( newline && !( i % newline ) ) {
         result += "\n";
      }
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

QString US_Vector::qs_vector2( QString qs, vector < int > x, vector < double > y, int digits )
{
   QString result;
   result += QString( "%1: size %2 %3:\n" ).arg( qs ).arg( x.size() ).arg( y.size() );
   unsigned int max_size = ( unsigned int )( x.size() > y.size() ? x.size() : y.size() );
   for ( unsigned int i = 0; i < max_size; i++ )
   {
      result += QString( "\t%1 %2\n" )
         .arg( ( x.size() > i ) ? QString( "%1" ).arg( x[ i ] ) : QString( "n/a" ) )
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

QString US_Vector::qs_vector2( QString qs, vector < QString > x, vector < QString > y )
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

QString US_Vector::qs_vector3( QString qs, vector < double > x, vector < double > y, vector < double > z, int digits )
{
   QString result;
   unsigned int max_size = ( unsigned int )( x.size() > y.size() ? x.size() : y.size() );
   max_size = max_size > z.size() ? max_size : z.size();
   result += QString( "%1: size %2 %3 %4 max size %5:\n" ).arg( qs ).arg( x.size() ).arg( y.size() ).arg( z.size() ).arg( max_size );
   for ( unsigned int i = 0; i < max_size; i++ )
   {
      result += QString( "\t%1 %2 %3\n" )
         .arg( ( x.size() > i ) ? QString( "%1" ).arg( x[ i ], 0, 'g', digits ) : QString( "n/a" ) )
         .arg( ( y.size() > i ) ? QString( "%1" ).arg( y[ i ], 0, 'g', digits ) : QString( "n/a" ) )
         .arg( ( z.size() > i ) ? QString( "%1" ).arg( z[ i ], 0, 'g', digits ) : QString( "n/a" ) )
         ;
   }
   result += "\n";
   return result;
}

QString US_Vector::qs_vector4( QString qs,
                               vector < double > x,
                               vector < double > y,
                               vector < double > z,
                               vector < double > s,
                               int digits )
{
   QString result;
   unsigned int max_size = ( unsigned int )( x.size() > y.size() ? x.size() : y.size() );
   max_size = max_size > z.size() ? max_size : z.size();
   max_size = max_size > s.size() ? max_size : s.size();
   result += QString( "%1: size %2 %3 %4 %5:\n" ).arg( qs ).arg( x.size() ).arg( y.size() ).arg( z.size() ).arg( s.size() );
   for ( unsigned int i = 0; i < max_size; i++ )
   {
      result += QString( "\t%1 %2 %3 %4\n" )
         .arg( ( x.size() > i ) ? QString( "%1" ).arg( x[ i ], 0, 'g', digits ) : QString( "n/a" ) )
         .arg( ( y.size() > i ) ? QString( "%1" ).arg( y[ i ], 0, 'g', digits ) : QString( "n/a" ) )
         .arg( ( z.size() > i ) ? QString( "%1" ).arg( z[ i ], 0, 'g', digits ) : QString( "n/a" ) )
         .arg( ( s.size() > i ) ? QString( "%1" ).arg( s[ i ], 0, 'g', digits ) : QString( "n/a" ) )
         ;
   }
   result += "\n";
   return result;
}

QString US_Vector::qs_vector_vector( QString qs, vector < vector < double > > m, int digits )
{
   QString result;
   result += QString( "%1: size %1" ).arg( qs ).arg( m.size() );
   if ( !m.size() )
   {
      return result;
   }

   result += QString( " %1:\n" ).arg( m[ 0 ].size() );
   for ( unsigned int i = 0; i < m.size(); ++i )
   {
      result += "\t";
      for ( unsigned int j = 0; j < m[ i ].size(); ++j )
      {
         result += QString( "%1 " ).arg( m[ i ][ j ], 0, 'g', digits );
      }
      result += "\n"; 
   }
   result += "\n";
   return result;
}

US_Stat::US_Stat() {
   clear();
}

void US_Stat::clear() {
   pts.clear();
   sum = 0e0;
   sum2 = 0e0;
   accum_ok = false;
}

void US_Stat::add_point( double x ) {
   if ( pts.size() ) {
      if ( pmin > x ) {
         pmin = x;
      }
      if ( pmax < x ) {
        pmax = x;
      }
   } else {
      pmin = x;
      pmax = x;
   }
   pts.push_back( x );
   sum += x;
   sum2 += x * x;
}

void US_Stat::add_accum( double x ) {
   if ( !accum_ok ) {
      init_accum();
   }
   accum += x;
}

void US_Stat::init_accum() {
   accum = 0;
   accum_ok = true;
}

void US_Stat::push_accum() {
   if ( accum_ok ) {
      add_point( accum );
   }
   init_accum();
}

bool US_Stat::has_accum() {
   return accum_ok;
}

void US_Stat::add_points( vector < double > & x ) {
   int n = (int) x.size();
   for ( int i = 0; i < n; ++i ) {
      add_point( x[ i ] );
   }
}

int US_Stat::count() {
   return (int) pts.size();
}

double US_Stat::min() {
   return pmin;
}

double US_Stat::max() {
   return pmax;
}

double US_Stat::avg() {
   double n = (double) count();
   if ( n ) {
      double countinv = 1e0 / n;
      return sum * countinv;
   }
   return US_STAT_ERROR;
}

double US_Stat::sd( bool pop ) {
   double n = (double) count();
   if ( n > 1 ) {
      double countinv = 1e0 / n;
      double countm1inv;
      if ( pop ) {
         countm1inv = 1e0 / ( n );
      } else {
         countm1inv = 1e0 / ( n - 1e0 );
      }
      return sqrt( countm1inv * ( sum2 - countinv * sum * sum ) );
   }
   return 0;
}

#include <QTextStream>


double US_Stat::skew( bool adjusted ) {
   // adjusted fisher-pearson coefficient of skewness
   int n = count();
   if ( n < 3 ) {
      return 0;
   }

   double m = avg();
   double s = sd( true );
   if ( s == 0 ) {
      return 0;
   }
   double recips3 = 1e0 / ( s * s * s );

   double symybar3 = 0e0;
   for ( int i = 0; i < n; ++i ) {
      double ymybar = pts[ i ] - m;
      symybar3 += ymybar * ymybar * ymybar;
   }
   double symybar3_n = symybar3 / (double) n;
   double retval = symybar3_n * recips3;
   /*
   QTextStream( stdout ) << QString(
                       "symybar3   = %1\n"
                        "symybar3_n = %2\n"
                        "recips3    = %3\n"
                        "retval     = %4\n"
                       )
      .arg( symybar3 )
      .arg( symybar3_n )
      .arg( recips3 )
      .arg( retval )
      ;
   */

   if ( adjusted ) {
      retval *= sqrt( n * ( n - 1e0 ) ) / ( n - 2 );
   }
   return retval;
}

QString US_Stat::summary() {
   return QString(
                  "points     %1\n"
                  "min        %2\n"
                  "max        %3\n"
                  "avg        %4\n"
                  "sd sample  %5\n"
                  "sd pop     %6\n"
                  "skew       %7\n"
                  "skew adj   %8\n"
                  )
      .arg( count() )
      .arg( min() )
      .arg( max() )
      .arg( avg() )
      .arg( sd() )
      .arg( sd( true ) )
      .arg( skew( false ) )
      .arg( skew() )
      ;
}
                  
void US_Stat::test() {
   QTextStream tso( stdout );

   {
      vector < double > x{ 1e0, 2e0, 3e0, 4e0 };
      clear();
      add_points( x );
      tso << US_Vector::qs_vector( "data", pts );
      tso << summary();
   }
   {
      vector < double > x{ 2, 4, 5, 7, 8, 10, 11, 25, 26, 27, 36 };
      clear();
      add_points( x );
      tso << US_Vector::qs_vector( "data", pts );
      tso << summary();
   }
   {
      vector < double > x{ 1, 1, 4 };
      clear();
      add_points( x );
      tso << US_Vector::qs_vector( "data", pts );
      tso << summary();
   }
}

