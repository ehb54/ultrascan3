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

using namespace std;

class US_Vector
{
 public:

#ifdef WIN32
# if !defined( QT4 )
  #pragma warning ( disable: 4251 )
# endif
#endif
   static void printvector( QString qs, vector < unsigned int > x );
   static void printvector( QString qs, vector < int > x );
   static void printvector( QString qs, vector < double > x, int digits = 8 );
   static void printvector2( QString qs, vector < double > x, vector < double > y, int digits = 8 );
   static void printvector3( QString qs, vector < double > x, vector < double > y, vector < double > z, int digits = 8 );
   static void printvector2( QString qs, vector < int > x, vector < int > y );
   static void printvector( QString qs, vector < bool > x );

   static QString qs_vector( QString qs, vector < unsigned int > x );
   static QString qs_vector( QString qs, vector < int > x );
   static QString qs_vector( QString qs, vector < double > x, int digits = 8 );
   static QString qs_vector2( QString qs, vector < double > x, vector < double > y, int digits = 8 );
   static QString qs_vector2( QString qs, vector < int > x, vector < int > y );
   static QString qs_vector( QString qs, vector < bool > x );

   static vector < double > intersection( vector < double > &x, vector < double > &y );
   static vector < double > intersection( vector < vector < double > > &x );
   static vector < double > vunion( vector < double > &x, vector < double > &y );
   static vector < double > vunion( vector < vector < double > > &x );
#ifdef WIN32
# if !defined( QT4 )
  #pragma warning ( default: 4251 )
# endif
#endif

};

#endif
