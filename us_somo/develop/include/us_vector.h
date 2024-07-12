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

   static void printvector( QString qs, vector < unsigned int > x );
   static void printvector( QString qs, vector < int > x );
   static void printvector( QString qs, vector < QString > & x );
   static void printvector( QString qs, vector < double > x, int digits = 8 );
   static void printvector2( QString qs, vector < double > x, vector < double > y, int digits = 8, unsigned int limit = 0 );
   static void printvector3( QString qs, vector < double > x, vector < double > y, vector < double > z, int digits = 8, unsigned int limit = 0 );
   static void printvector4( QString qs, vector < double > x, vector < double > y, vector < double > z, vector < double > s, int digits = 8, unsigned int limit = 0 );
   static void printvector5( QString qs, vector < double > x, vector < double > y, vector < double > z, vector < double > s, vector < double > t, int digits = 8, unsigned int limit = 0 );
   static void printvector7( QString qs, vector < double > x, vector < double > y, vector < double > z, vector < double > s, vector < double > t, vector < double > u, vector < double > v, int digits = 8, unsigned int limit = 0 );
   static void printvector( QString qs, vector < bool > x );
   static void printvector2( QString qs, vector < int > x, vector < int > y );
   static QString qs_vector( QString qs, vector < unsigned int > x );
   static QString qs_vector( QString qs, vector < int > x );
   static QString qs_vector( QString qs, vector < double > x, int digits = 8, int newline = 0 );
   static QString qs_vector( vector < double > x, int digits = 8 );
   static QString qs_vector( vector < float > x, int digits = 8 );
   static QString qs_vector2( QString qs, vector < double > x, vector < double > y, int digits = 8 );
   static QString qs_vector2( QString qs, vector < int > x, vector < double > y, int digits = 8 );
   static QString qs_vector2( QString qs, vector < int > x, vector < int > y );
   static QString qs_vector2( QString qs, vector < QString > x, vector < QString > y );
   static QString qs_vector( QString qs, vector < bool > x );
   static QString qs_vector3( QString qs, vector < double > x, vector < double > y, vector < double > z, int digits = 8 );
   static QString qs_vector4( QString qs, vector < double > x, vector < double > y, vector < double > z, vector < double > s, int digits = 8 );
   static QString qs_vector_vector( QString qs, vector < vector < double > > m, int digits = 8 );

   static QString qs_mapqsqs( QString qs, map < QString, QString > & x );
   static QString qs_mapqsfloat( QString qs, map < QString, float > & x );
   static QString qs_mapqsdouble( QString qs, map < QString, double > & x );

   static vector < double > intersection( vector < double > &x, vector < double > &y );
   static vector < double > intersection( vector < vector < double > > &x );
   static vector < double > vunion( vector < double > &x, vector < double > &y );
   static vector < double > vunion( vector < vector < double > > &x );

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
