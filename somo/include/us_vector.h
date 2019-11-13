#ifndef US_VECTOR_H
#define US_VECTOR_H

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <qstring.h>
#include <qregexp.h>
#include <map>
#include <vector>
#include <list>
#include <math.h>

using namespace std;

class US_Vector
{
 public:

#ifdef WIN32
# if QT_VERSION < 0x040000
  #pragma warning ( disable: 4251 )
# endif
#endif
   static void printvector( QString qs, vector < unsigned int > x );
   static void printvector( QString qs, vector < int > x );
   static void printvector( QString qs, vector < QString > & x );
   static void printvector( QString qs, vector < double > x, int digits = 8 );
   static void printvector2( QString qs, vector < double > x, vector < double > y, int digits = 8 );
   static void printvector3( QString qs, vector < double > x, vector < double > y, vector < double > z, int digits = 8 );
   static void printvector2( QString qs, vector < int > x, vector < int > y );
   static void printvector( QString qs, vector < bool > x );

   static QString qs_vector( QString qs, vector < unsigned int > x );
   static QString qs_vector( QString qs, vector < int > x );
   static QString qs_vector( QString qs, vector < double > x, int digits = 8, int newline = 0 );
   static QString qs_vector2( QString qs, vector < double > x, vector < double > y, int digits = 8 );
   static QString qs_vector2( QString qs, vector < int > x, vector < double > y, int digits = 8 );
   static QString qs_vector2( QString qs, vector < int > x, vector < int > y );
   static QString qs_vector2( QString qs, vector < QString > x, vector < QString > y );
   static QString qs_vector( QString qs, vector < bool > x );
   static QString qs_vector3( QString qs, vector < double > x, vector < double > y, vector < double > z, int digits = 8 );
   static QString qs_vector4( QString qs, vector < double > x, vector < double > y, vector < double > z, vector < double > s, int digits = 8 );
   static QString qs_vector_vector( QString qs, vector < vector < double > > m, int digits = 8 );

   static vector < double > intersection( vector < double > &x, vector < double > &y );
   static vector < double > intersection( vector < vector < double > > &x );
   static vector < double > vunion( vector < double > &x, vector < double > &y );
   static vector < double > vunion( vector < vector < double > > &x );
#ifdef WIN32
# if QT_VERSION < 0x040000
  #pragma warning ( default: 4251 )
# endif
#endif

};

#define US_STAT_ERROR -1e99

class US_Stat
{
 public:
   US_Stat();
   void              test();
   
   void              clear();

   void              add_point( double x );
   void              add_points( vector < double > & x );
   void              add_accum( double x );
   void              push_accum();
   bool              has_accum();
   
   int               count();
   double            min();
   double            max();
   double            avg();
   double            sd( bool pop = false );
   double            skew( bool adjusted = true );

   QString           summary();

 private:
   void              init_accum();
   vector < double > pts;
   double            pmin;
   double            pmax;
   double            sum;
   double            sum2;
   double            accum;
   bool              accum_ok;
};


#endif
