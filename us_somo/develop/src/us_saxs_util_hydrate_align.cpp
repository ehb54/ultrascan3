#include "../include/us_saxs_util.h"
#include "../include/us_tnt_jama.h"

// note: this program uses cout and/or cerr and this should be replaced

static std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const QString& str) { 
   return os << qPrintable(str);
}

// #define DEBUG_FUNKY_BUG
// #define DEBUG_ALIGN
// #define DEBUG_ALIGN2

#define KILL_INTERNAL_REPRESENTATION

bool US_Saxs_Util::atom_align( vector < point > transform_from, 
                               vector < point > transform_to, 
                               vector < point > apply_from, 
                               vector < point > &result )
{
   errormsg = "";
#if defined( DEBUG_ALIGN )
   // for testing:
   vector < point > save_from = transform_from; 
#endif

   if ( transform_from.size() < 3 )
   {
      errormsg = "At least 3 points are needed to align atoms";
      return false;
   }
   if ( transform_from.size() != transform_to.size() )
   {
      errormsg = QString( "Size of transform from (%1) does not equal size of transform to (%2)" )
         .arg( transform_from.size() )
         .arg( transform_to.size() );
      return false;
   }
   
   // point center_from = transform_from[ 0 ];
   // point center_to = transform_to[ 0 ];
   point center_from;
   point center_to;

   for ( unsigned int j = 0; j < 3; j++ )
   {
      center_from.axis[ j ] = transform_from[ 0 ].axis[ j ];
      center_to  .axis[ j ] = transform_to  [ 0 ].axis[ j ];
   }

   for ( unsigned int i = 1; i < transform_from.size(); i++ )
   {
      for ( unsigned int j = 0; j < 3; j++ )
      {
#if defined( KILL_INTERNAL_REPRESENTATION )
         transform_from[ i ].axis[ j ] = QString::asprintf( "%.8f", transform_from[ i ].axis[ j ] ).toFloat();
         transform_to  [ i ].axis[ j ] = QString::asprintf( "%.8f", transform_to  [ i ].axis[ j ] ).toFloat();
#endif
         center_from.axis[ j ] += transform_from[ i ].axis[ j ];
         center_to.axis  [ j ] += transform_to  [ i ].axis[ j ];
      }
   }

#if defined( KILL_INTERNAL_REPRESENTATION )
   for ( unsigned int j = 0; j < 3; j++ )
   {
      center_from.axis[ j ] = QString::asprintf( "%.8f", center_from.axis[ j ] ).toFloat();
      center_to  .axis[ j ] = QString::asprintf( "%.8f", center_to  .axis[ j ] ).toFloat();
   }
#endif

   for ( unsigned int j = 0; j < 3; j++ )
   {
      center_from.axis[ j ] /= (float) transform_from.size();
      center_to.axis  [ j ] /= (float) transform_to.size();
   }

#if defined( KILL_INTERNAL_REPRESENTATION )
   for ( unsigned int j = 0; j < 3; j++ )
   {
      center_from.axis[ j ] = QString::asprintf( "%.8f", center_from.axis[ j ] ).toFloat();
      center_to  .axis[ j ] = QString::asprintf( "%.8f", center_to  .axis[ j ] ).toFloat();
   }
#endif

   for ( unsigned int i = 0; i < transform_from.size(); i++ )
   {
      for ( unsigned int j = 0; j < 3; j++ )
      {
         transform_from[ i ].axis[ j ] -= center_from.axis[ j ];
         transform_to  [ i ].axis[ j ] -= center_to  .axis[ j ];
      }
   }

#if defined( DEBUG_ALIGN ) || defined( DEBUG_FUNKY_BUG )
   cout << "center point from " << center_from << endl;
   cout << "center point to   " << center_to << endl;
#endif

   TNT::Array2D < float > H( 3, 3 );

   for ( int i = 0; i < 3; i++ )
   {
      for ( int j = 0; j < 3; j++ ) 
      {
         H[ i ][ j ] = 0.0f;
      }
   }

   for ( unsigned int i = 0; i < transform_from.size(); i++ ) 
   {
      for ( int j = 0; j < 3; j++ ) 
      {
         for ( int k = 0; k < 3; k++ ) 
         {
            H[ j ][ k ] +=  transform_from[ i ].axis[ j ] * transform_to[ i ].axis[ k ];
         }
      }
   }

#if defined( DEBUG_ALIGN )
   puts( "svd\n" );
   printf( "det H %g\n", det( H ) );
#endif
   JAMA::SVD < float >    svd( H );
   TNT::Array2D < float > U(3, 3);
   TNT::Array2D < float > V(3, 3);
#if defined( DEBUG_ALIGN )
   printf( "iter for svd %s\n", svd.over_iter_limit ? "true" : "no, it's ok" );
   puts( "getU\n" );
#endif
   svd.getU( U );
#if defined( DEBUG_ALIGN )
   puts( "getV\n" );
#endif
   svd.getV( V );

#if defined( DEBUG_ALIGN )
   puts( "transpose\n" );
#endif
   // the rotation matrix is R = VU^T
   TNT::Array2D < float > UT = transpose( U );
   TNT::Array2D < float > rot(3, 3);
#if defined( DEBUG_ALIGN )
   puts( "mat mult\n" );
#endif
   rot = matmult( V, UT );

#if defined( DEBUG_ALIGN )
   puts( "check for reflection\n" );
#endif
   // check for reflection
   if ( det( rot ) < 0) {
      TNT::Array2D < float > VT = transpose( V );
      TNT::Array2D < float > UVT = matmult( U, VT );
      TNT::Array2D < float > S( 3, 3 );
      S[ 0 ][ 0 ] = S[ 1 ][ 1 ] = 1.0;
      S[ 2 ][ 2 ] = det( UVT );
      S[ 0 ][ 1 ] = 
         S[ 0 ][ 2 ] = 
         S[ 1 ][ 0 ] = 
         S[ 1 ][ 2 ] = 
         S[ 2 ][ 0 ] = 
         S[ 2 ][ 1 ] = 0.0;
      rot = matmult( matmult( U, S ), VT );
   }

#if defined( DEBUG_ALIGN )
   cout << "rotation matrix: ";
   for ( int i = 0; i < 3; i++ )
   {
      for ( int j = 0; j < 3; j++ )
      {
         cout << QString(" %1").arg( rot[ i ][ j ] );
      }
      cout << " : ";
   }
   cout << "\n";
#endif

   // compute translation:

   point trans = center_to;

   for ( int i = 0; i < 3; i++ )
   {
      for ( int j = 0; j < 3; j++ )
      {
         trans.axis[ i ] -= rot[ i ][ j ] * center_from.axis[ j ];
      }
   }

#if defined( DEBUG_ALIGN )
   cout << "trans:\n" << trans << endl;

   // test transformation on transform_from:

   cout << "test on transform_from:\n";
   vector < point > test_to( save_from.size() );
   for ( int i = 0; i < save_from.size(); i++ )
   {
      for ( int j = 0; j < 3; j++ )
      {
         test_to[ i ].axis[ j ] = trans.axis[ j ];
         for ( int k = 0; k < 3; k++ )
         {
            test_to[ i ].axis[ j ] += rot[ j ][ k ] * save_from[ i ].axis[ k ];
         }
      }
      cout << test_to[ i ] << endl;
   }
#endif

   // apply translations to results

   result.resize( apply_from.size() );

   for ( unsigned int i = 0; i < apply_from.size(); i++ )
   {
      for ( int j = 0; j < 3; j++ )
      {
         result[ i ].axis[ j ] = trans.axis[ j ];
         for ( int k = 0; k < 3; k++ )
         {
            result[ i ].axis[ j ] += rot[ j ][ k ] * apply_from[ i ].axis[ k ];
         }
      }
   }      

#if defined( DEBUG_ALIGN2 )
   {
      QString out;
      for ( unsigned int i = 0; i < transform_from.size(); i++ ) 
      {
         for ( int j = 0; j < 3; j++ ) 
         {
            out += QString::asprintf( "z2 pos i %u j %u xfrom %.8f xto %.8f\n",
                                      i, j,
                                      transform_from[ i ].axis[ j ],
                                      transform_to  [ i ].axis[ j ] )
               ;
         }
      }
      cout << out << flush;
   }
#endif

#if defined( DEBUG_FUNKY_BUG )
   cout << center_from << endl;
#endif

   return true;
}

bool US_Saxs_Util::align_test()
{
#if defined( USUHA_ACTIVATE_ALIGN )
   cout << "align_test()\n";
   unsigned int waterpos = 0;
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = 1.3079999685f;
      p2[ 0 ].axis[ 1 ] = 27.8400001526f;
      p2[ 0 ].axis[ 2 ] = 25.6250000000f;
      p2[ 1 ].axis[ 0 ] = 2.1129999161f;
      p2[ 1 ].axis[ 1 ] = 27.4500007629f;
      p2[ 1 ].axis[ 2 ] = 24.3950004578f;
      p2[ 2 ].axis[ 0 ] = 0.3600000143f;
      p2[ 2 ].axis[ 1 ] = 28.8700008392f;
      p2[ 2 ].axis[ 2 ] = 25.2500000000f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = 4.6911883354f;
      new_waters[ 0 ].axis[ 1 ] = 27.8160820007f;
      new_waters[ 0 ].axis[ 2 ] = 24.6586856842f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = -11.2620000839f;
      p2[ 0 ].axis[ 1 ] = 17.1289997101f;
      p2[ 0 ].axis[ 2 ] = 9.5950002670f;
      p2[ 1 ].axis[ 0 ] = -11.0340003967f;
      p2[ 1 ].axis[ 1 ] = 16.2059993744f;
      p2[ 1 ].axis[ 2 ] = 10.7799997330f;
      p2[ 2 ].axis[ 0 ] = -10.1239995956f;
      p2[ 2 ].axis[ 1 ] = 18.0489997864f;
      p2[ 2 ].axis[ 2 ] = 9.4250001907f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = -7.3927063942f;
      new_waters[ 0 ].axis[ 1 ] = 17.4092578888f;
      new_waters[ 0 ].axis[ 2 ] = 10.4614000320f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = -9.5439996719f;
      p2[ 0 ].axis[ 1 ] = 14.9079999924f;
      p2[ 0 ].axis[ 2 ] = 12.1359996796f;
      p2[ 1 ].axis[ 0 ] = -9.6510000229f;
      p2[ 1 ].axis[ 1 ] = 15.6409997940f;
      p2[ 1 ].axis[ 2 ] = 13.4940004349f;
      p2[ 2 ].axis[ 0 ] = -9.8149995804f;
      p2[ 2 ].axis[ 1 ] = 15.7709999084f;
      p2[ 2 ].axis[ 2 ] = 10.9879999161f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = -9.4814014435f;
      new_waters[ 0 ].axis[ 1 ] = 18.7487220764f;
      new_waters[ 0 ].axis[ 2 ] = 11.1911716461f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = -7.5879998207f;
      p2[ 0 ].axis[ 1 ] = 30.7770004272f;
      p2[ 0 ].axis[ 2 ] = 5.9640002251f;
      p2[ 1 ].axis[ 0 ] = -7.1519999504f;
      p2[ 1 ].axis[ 1 ] = 29.3120002747f;
      p2[ 1 ].axis[ 2 ] = 5.9549999237f;
      p2[ 2 ].axis[ 0 ] = -8.2220001221f;
      p2[ 2 ].axis[ 1 ] = 31.1609992981f;
      p2[ 2 ].axis[ 2 ] = 7.2290000916f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = -7.8539829254f;
      new_waters[ 0 ].axis[ 1 ] = 29.4537105560f;
      new_waters[ 0 ].axis[ 2 ] = 9.7061910629f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = 8.3760004044f;
      p2[ 0 ].axis[ 1 ] = 15.7480001450f;
      p2[ 0 ].axis[ 2 ] = 14.6719999313f;
      p2[ 1 ].axis[ 0 ] = 8.8240003586f;
      p2[ 1 ].axis[ 1 ] = 15.7100000381f;
      p2[ 1 ].axis[ 2 ] = 16.2369995117f;
      p2[ 2 ].axis[ 0 ] = 7.4600000381f;
      p2[ 2 ].axis[ 1 ] = 14.6569995880f;
      p2[ 2 ].axis[ 2 ] = 14.3050003052f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = 5.5444841385f;
      new_waters[ 0 ].axis[ 1 ] = 17.3358917236f;
      new_waters[ 0 ].axis[ 2 ] = 16.0695915222f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = 2.4519999027f;
      p2[ 0 ].axis[ 1 ] = 8.0329999924f;
      p2[ 0 ].axis[ 2 ] = 22.1849994659f;
      p2[ 1 ].axis[ 0 ] = 2.0000000000f;
      p2[ 1 ].axis[ 1 ] = 9.2159996033f;
      p2[ 1 ].axis[ 2 ] = 21.3250007629f;
      p2[ 2 ].axis[ 0 ] = 3.8570001125f;
      p2[ 2 ].axis[ 1 ] = 8.1689996719f;
      p2[ 2 ].axis[ 2 ] = 22.5979995728f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = 4.5526113510f;
      new_waters[ 0 ].axis[ 1 ] = 11.4180402756f;
      new_waters[ 0 ].axis[ 2 ] = 22.4347076416f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = -3.4830000401f;
      p2[ 0 ].axis[ 1 ] = 11.2500000000f;
      p2[ 0 ].axis[ 2 ] = 22.4440002441f;
      p2[ 1 ].axis[ 0 ] = -2.9709999561f;
      p2[ 1 ].axis[ 1 ] = 12.6850004196f;
      p2[ 1 ].axis[ 2 ] = 22.4120006561f;
      p2[ 2 ].axis[ 0 ] = -4.1859998703f;
      p2[ 2 ].axis[ 1 ] = 10.8999996185f;
      p2[ 2 ].axis[ 2 ] = 21.2159996033f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = -3.6776349545f;
      new_waters[ 0 ].axis[ 1 ] = 12.3434371948f;
      new_waters[ 0 ].axis[ 2 ] = 18.6386737823f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = 1.5269999504f;
      p2[ 0 ].axis[ 1 ] = 33.1069984436f;
      p2[ 0 ].axis[ 2 ] = 19.6590003967f;
      p2[ 1 ].axis[ 0 ] = 2.2209999561f;
      p2[ 1 ].axis[ 1 ] = 34.0130004883f;
      p2[ 1 ].axis[ 2 ] = 18.6299991608f;
      p2[ 2 ].axis[ 0 ] = 1.4969999790f;
      p2[ 2 ].axis[ 1 ] = 31.7010002136f;
      p2[ 2 ].axis[ 2 ] = 19.2180004120f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = 4.8101091385f;
      new_waters[ 0 ].axis[ 1 ] = 33.4083671570f;
      new_waters[ 0 ].axis[ 2 ] = 17.6135063171f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = 2.6040000916f;
      p2[ 0 ].axis[ 1 ] = 33.1290016174f;
      p2[ 0 ].axis[ 2 ] = 13.8730001450f;
      p2[ 1 ].axis[ 0 ] = 1.1710000038f;
      p2[ 1 ].axis[ 1 ] = 33.0019989014f;
      p2[ 1 ].axis[ 2 ] = 13.5520000458f;
      p2[ 2 ].axis[ 0 ] = 2.9639999866f;
      p2[ 2 ].axis[ 1 ] = 33.2729988098f;
      p2[ 2 ].axis[ 2 ] = 15.2869997025f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = 1.0160481930f;
      new_waters[ 0 ].axis[ 1 ] = 32.2880516052f;
      new_waters[ 0 ].axis[ 2 ] = 17.3650684357f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = -10.5979995728f;
      p2[ 0 ].axis[ 1 ] = 27.3659992218f;
      p2[ 0 ].axis[ 2 ] = 3.5859999657f;
      p2[ 1 ].axis[ 0 ] = -10.4239997864f;
      p2[ 1 ].axis[ 1 ] = 26.2590007782f;
      p2[ 1 ].axis[ 2 ] = 2.5690000057f;
      p2[ 2 ].axis[ 0 ] = -9.9770002365f;
      p2[ 2 ].axis[ 1 ] = 27.0030002594f;
      p2[ 2 ].axis[ 2 ] = 4.8480000496f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = -8.4790124893f;
      new_waters[ 0 ].axis[ 1 ] = 24.1738357544f;
      new_waters[ 0 ].axis[ 2 ] = 4.7964415550f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = -14.9289999008f;
      p2[ 0 ].axis[ 1 ] = 19.7569999695f;
      p2[ 0 ].axis[ 2 ] = 3.2000000477f;
      p2[ 1 ].axis[ 0 ] = -15.1160001755f;
      p2[ 1 ].axis[ 1 ] = 19.3869991302f;
      p2[ 1 ].axis[ 2 ] = 4.6449999809f;
      p2[ 2 ].axis[ 0 ] = -13.6090002060f;
      p2[ 2 ].axis[ 1 ] = 20.2989997864f;
      p2[ 2 ].axis[ 2 ] = 3.0230000019f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = -17.7253093719f;
      new_waters[ 0 ].axis[ 1 ] = 19.3274841309f;
      new_waters[ 0 ].axis[ 2 ] = 2.3180801868f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = -13.8780002594f;
      p2[ 0 ].axis[ 1 ] = 15.0209999084f;
      p2[ 0 ].axis[ 2 ] = 14.6669998169f;
      p2[ 1 ].axis[ 0 ] = -13.4799995422f;
      p2[ 1 ].axis[ 1 ] = 14.7460002899f;
      p2[ 1 ].axis[ 2 ] = 16.0990009308f;
      p2[ 2 ].axis[ 0 ] = -13.2729997635f;
      p2[ 2 ].axis[ 1 ] = 16.2399997711f;
      p2[ 2 ].axis[ 2 ] = 14.2200002670f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = -16.2918701172f;
      new_waters[ 0 ].axis[ 1 ] = 13.9123144150f;
      new_waters[ 0 ].axis[ 2 ] = 13.4350347519f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = -12.2500000000f;
      p2[ 0 ].axis[ 1 ] = 26.7150001526f;
      p2[ 0 ].axis[ 2 ] = 24.7910003662f;
      p2[ 1 ].axis[ 0 ] = -12.2639999390f;
      p2[ 1 ].axis[ 1 ] = 27.9869995117f;
      p2[ 1 ].axis[ 2 ] = 24.0170001984f;
      p2[ 2 ].axis[ 0 ] = -12.7209997177f;
      p2[ 2 ].axis[ 1 ] = 25.5930004120f;
      p2[ 2 ].axis[ 2 ] = 23.9769992828f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = -10.7371664047f;
      new_waters[ 0 ].axis[ 1 ] = 25.0588798523f;
      new_waters[ 0 ].axis[ 2 ] = 21.7520236969f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = 15.0609998703f;
      p2[ 0 ].axis[ 1 ] = 19.1119995117f;
      p2[ 0 ].axis[ 2 ] = 21.8269996643f;
      p2[ 1 ].axis[ 0 ] = 15.2500000000f;
      p2[ 1 ].axis[ 1 ] = 20.5550003052f;
      p2[ 1 ].axis[ 2 ] = 22.2520008087f;
      p2[ 2 ].axis[ 0 ] = 14.5450000763f;
      p2[ 2 ].axis[ 1 ] = 19.0349998474f;
      p2[ 2 ].axis[ 2 ] = 20.4790000916f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = 14.9138984680f;
      new_waters[ 0 ].axis[ 1 ] = 21.2407608032f;
      new_waters[ 0 ].axis[ 2 ] = 18.4843254089f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = -4.1059999466f;
      p2[ 0 ].axis[ 1 ] = 20.5599994659f;
      p2[ 0 ].axis[ 2 ] = 5.6739997864f;
      p2[ 1 ].axis[ 0 ] = -5.5399999619f;
      p2[ 1 ].axis[ 1 ] = 20.2259998322f;
      p2[ 1 ].axis[ 2 ] = 5.9920001030f;
      p2[ 2 ].axis[ 0 ] = -3.4140000343f;
      p2[ 2 ].axis[ 1 ] = 19.5179996490f;
      p2[ 2 ].axis[ 2 ] = 5.0089998245f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = -6.0277762413f;
      new_waters[ 0 ].axis[ 1 ] = 18.6073341370f;
      new_waters[ 0 ].axis[ 2 ] = 2.9168753624f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = 8.9809999466f;
      p2[ 0 ].axis[ 1 ] = 18.9330005646f;
      p2[ 0 ].axis[ 2 ] = 31.1250000000f;
      p2[ 1 ].axis[ 0 ] = 7.7010002136f;
      p2[ 1 ].axis[ 1 ] = 18.8059997559f;
      p2[ 1 ].axis[ 2 ] = 31.9349994659f;
      p2[ 2 ].axis[ 0 ] = 8.8059997559f;
      p2[ 2 ].axis[ 1 ] = 18.3470001221f;
      p2[ 2 ].axis[ 2 ] = 29.7870006561f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = 6.1389660835f;
      new_waters[ 0 ].axis[ 1 ] = 19.1865119934f;
      new_waters[ 0 ].axis[ 2 ] = 28.4557647705f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = 15.0889997482f;
      p2[ 0 ].axis[ 1 ] = 11.5120000839f;
      p2[ 0 ].axis[ 2 ] = 30.2369995117f;
      p2[ 1 ].axis[ 0 ] = 15.0459995270f;
      p2[ 1 ].axis[ 1 ] = 12.9490003586f;
      p2[ 1 ].axis[ 2 ] = 30.5599994659f;
      p2[ 2 ].axis[ 0 ] = 13.7869997025f;
      p2[ 2 ].axis[ 1 ] = 10.8909997940f;
      p2[ 2 ].axis[ 2 ] = 30.1940002441f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = 11.3153629303f;
      new_waters[ 0 ].axis[ 1 ] = 12.5254526138f;
      new_waters[ 0 ].axis[ 2 ] = 29.6933917999f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = 5.5500001907f;
      p2[ 0 ].axis[ 1 ] = 16.0550003052f;
      p2[ 0 ].axis[ 2 ] = 35.8190002441f;
      p2[ 1 ].axis[ 0 ] = 4.5640001297f;
      p2[ 1 ].axis[ 1 ] = 15.0810003281f;
      p2[ 1 ].axis[ 2 ] = 35.1739997864f;
      p2[ 2 ].axis[ 0 ] = 6.8010001183f;
      p2[ 2 ].axis[ 1 ] = 16.3110008240f;
      p2[ 2 ].axis[ 2 ] = 35.1129989624f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = 7.0183625221f;
      new_waters[ 0 ].axis[ 1 ] = 15.7497072220f;
      new_waters[ 0 ].axis[ 2 ] = 32.1899414062f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = -2.5729999542f;
      p2[ 0 ].axis[ 1 ] = 30.8640003204f;
      p2[ 0 ].axis[ 2 ] = 29.2460002899f;
      p2[ 1 ].axis[ 0 ] = -3.1760001183f;
      p2[ 1 ].axis[ 1 ] = 30.4969997406f;
      p2[ 1 ].axis[ 2 ] = 27.8759994507f;
      p2[ 2 ].axis[ 0 ] = -2.9790000916f;
      p2[ 2 ].axis[ 1 ] = 29.9150009155f;
      p2[ 2 ].axis[ 2 ] = 30.2840003967f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = -1.1496769190f;
      new_waters[ 0 ].axis[ 1 ] = 27.8497467041f;
      new_waters[ 0 ].axis[ 2 ] = 30.6463165283f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = -0.2700000107f;
      p2[ 0 ].axis[ 1 ] = 31.0370006561f;
      p2[ 0 ].axis[ 2 ] = 24.2689990997f;
      p2[ 1 ].axis[ 0 ] = 0.8090000153f;
      p2[ 1 ].axis[ 1 ] = 30.0459995270f;
      p2[ 1 ].axis[ 2 ] = 24.7649993896f;
      p2[ 2 ].axis[ 0 ] = -1.5089999437f;
      p2[ 2 ].axis[ 1 ] = 30.3729991913f;
      p2[ 2 ].axis[ 2 ] = 24.1049995422f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = -2.2434899807f;
      new_waters[ 0 ].axis[ 1 ] = 27.8526515961f;
      new_waters[ 0 ].axis[ 2 ] = 25.5608005524f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = 4.1319999695f;
      p2[ 0 ].axis[ 1 ] = 34.2830009460f;
      p2[ 0 ].axis[ 2 ] = 17.1520004272f;
      p2[ 1 ].axis[ 0 ] = 3.6570000648f;
      p2[ 1 ].axis[ 1 ] = 34.3030014038f;
      p2[ 1 ].axis[ 2 ] = 15.6949996948f;
      p2[ 2 ].axis[ 0 ] = 3.3310000896f;
      p2[ 2 ].axis[ 1 ] = 33.5009994507f;
      p2[ 2 ].axis[ 2 ] = 18.0779991150f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = 1.5931818485f;
      new_waters[ 0 ].axis[ 1 ] = 31.2821464539f;
      new_waters[ 0 ].axis[ 2 ] = 17.0077056885f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = -14.0459995270f;
      p2[ 0 ].axis[ 1 ] = 26.0100002289f;
      p2[ 0 ].axis[ 2 ] = 19.2530002594f;
      p2[ 1 ].axis[ 0 ] = -13.8509998322f;
      p2[ 1 ].axis[ 1 ] = 25.5069999695f;
      p2[ 1 ].axis[ 2 ] = 20.6709995270f;
      p2[ 2 ].axis[ 0 ] = -13.5419998169f;
      p2[ 2 ].axis[ 1 ] = 25.1299991608f;
      p2[ 2 ].axis[ 2 ] = 18.2290000916f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = -15.9568710327f;
      new_waters[ 0 ].axis[ 1 ] = 27.4189968109f;
      new_waters[ 0 ].axis[ 2 ] = 17.4044303894f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = -6.1319999695f;
      p2[ 0 ].axis[ 1 ] = 28.1590003967f;
      p2[ 0 ].axis[ 2 ] = 15.2550001144f;
      p2[ 1 ].axis[ 0 ] = -5.3169999123f;
      p2[ 1 ].axis[ 1 ] = 26.8889999390f;
      p2[ 1 ].axis[ 2 ] = 15.4639997482f;
      p2[ 2 ].axis[ 0 ] = -7.2319998741f;
      p2[ 2 ].axis[ 1 ] = 27.9279994965f;
      p2[ 2 ].axis[ 2 ] = 14.2799997330f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = -8.3366374969f;
      new_waters[ 0 ].axis[ 1 ] = 25.1984291077f;
      new_waters[ 0 ].axis[ 2 ] = 13.7709474564f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = 4.7519998550f;
      p2[ 0 ].axis[ 1 ] = 21.1030006409f;
      p2[ 0 ].axis[ 2 ] = 9.6049995422f;
      p2[ 1 ].axis[ 0 ] = 3.8250000477f;
      p2[ 1 ].axis[ 1 ] = 19.9179992676f;
      p2[ 1 ].axis[ 2 ] = 9.3210000992f;
      p2[ 2 ].axis[ 0 ] = 4.9749999046f;
      p2[ 2 ].axis[ 1 ] = 21.2229995728f;
      p2[ 2 ].axis[ 2 ] = 11.0500001907f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = 4.6779584885f;
      new_waters[ 0 ].axis[ 1 ] = 18.9209842682f;
      new_waters[ 0 ].axis[ 2 ] = 12.9329690933f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = 4.2709999084f;
      p2[ 0 ].axis[ 1 ] = 15.7370004654f;
      p2[ 0 ].axis[ 2 ] = 10.6990003586f;
      p2[ 1 ].axis[ 0 ] = 4.1009998322f;
      p2[ 1 ].axis[ 1 ] = 15.2110004425f;
      p2[ 1 ].axis[ 2 ] = 12.1579999924f;
      p2[ 2 ].axis[ 0 ] = 3.6670000553f;
      p2[ 2 ].axis[ 1 ] = 17.0729999542f;
      p2[ 2 ].axis[ 2 ] = 10.6000003815f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = 0.6747938991f;
      new_waters[ 0 ].axis[ 1 ] = 15.8586683273f;
      new_waters[ 0 ].axis[ 2 ] = 12.0729131699f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = 12.8280000687f;
      p2[ 0 ].axis[ 1 ] = 18.9090003967f;
      p2[ 0 ].axis[ 2 ] = 18.8630008698f;
      p2[ 1 ].axis[ 0 ] = 13.2580003738f;
      p2[ 1 ].axis[ 1 ] = 19.0359992981f;
      p2[ 1 ].axis[ 2 ] = 20.2810001373f;
      p2[ 2 ].axis[ 0 ] = 11.8940000534f;
      p2[ 2 ].axis[ 1 ] = 17.8010005951f;
      p2[ 2 ].axis[ 2 ] = 18.8080005646f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = 9.9470090866f;
      new_waters[ 0 ].axis[ 1 ] = 20.1497268677f;
      new_waters[ 0 ].axis[ 2 ] = 20.7988910675f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = 15.2469997406f;
      p2[ 0 ].axis[ 1 ] = 22.1350002289f;
      p2[ 0 ].axis[ 2 ] = 24.0620002747f;
      p2[ 1 ].axis[ 0 ] = 16.5079994202f;
      p2[ 1 ].axis[ 1 ] = 22.2450008392f;
      p2[ 1 ].axis[ 2 ] = 24.8999996185f;
      p2[ 2 ].axis[ 0 ] = 15.0559997559f;
      p2[ 2 ].axis[ 1 ] = 20.7959995270f;
      p2[ 2 ].axis[ 2 ] = 23.5429992676f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = 15.4628448486f;
      new_waters[ 0 ].axis[ 1 ] = 25.1884593964f;
      new_waters[ 0 ].axis[ 2 ] = 24.1627712250f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = 6.7969999313f;
      p2[ 0 ].axis[ 1 ] = 19.3050003052f;
      p2[ 0 ].axis[ 2 ] = 26.0340003967f;
      p2[ 1 ].axis[ 0 ] = 6.8930001259f;
      p2[ 1 ].axis[ 1 ] = 18.2390003204f;
      p2[ 1 ].axis[ 2 ] = 27.0990009308f;
      p2[ 2 ].axis[ 0 ] = 5.5560002327f;
      p2[ 2 ].axis[ 1 ] = 19.2970008850f;
      p2[ 2 ].axis[ 2 ] = 25.2819995880f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = 4.6615753174f;
      new_waters[ 0 ].axis[ 1 ] = 21.8430118561f;
      new_waters[ 0 ].axis[ 2 ] = 26.8803157806f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = 7.1269998550f;
      p2[ 0 ].axis[ 1 ] = 11.1140003204f;
      p2[ 0 ].axis[ 2 ] = 30.2000007629f;
      p2[ 1 ].axis[ 0 ] = 7.9990000725f;
      p2[ 1 ].axis[ 1 ] = 10.5469999313f;
      p2[ 1 ].axis[ 2 ] = 29.0729999542f;
      p2[ 2 ].axis[ 0 ] = 5.9130001068f;
      p2[ 2 ].axis[ 1 ] = 11.6510000229f;
      p2[ 2 ].axis[ 2 ] = 29.7199993134f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = 10.4262981415f;
      new_waters[ 0 ].axis[ 1 ] = 11.6045084000f;
      new_waters[ 0 ].axis[ 2 ] = 29.3296985626f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = 3.7149999142f;
      p2[ 0 ].axis[ 1 ] = 13.8330001831f;
      p2[ 0 ].axis[ 2 ] = 33.3129997253f;
      p2[ 1 ].axis[ 0 ] = 2.1940000057f;
      p2[ 1 ].axis[ 1 ] = 14.1470003128f;
      p2[ 1 ].axis[ 2 ] = 33.5009994507f;
      p2[ 2 ].axis[ 0 ] = 4.6680002213f;
      p2[ 2 ].axis[ 1 ] = 14.7810001373f;
      p2[ 2 ].axis[ 2 ] = 33.8959999084f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = 3.7433173656f;
      new_waters[ 0 ].axis[ 1 ] = 16.5416069031f;
      new_waters[ 0 ].axis[ 2 ] = 36.1644210815f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = -0.1169999987f;
      p2[ 0 ].axis[ 1 ] = 10.6070003510f;
      p2[ 0 ].axis[ 2 ] = 31.9260005951f;
      p2[ 1 ].axis[ 0 ] = 0.0989999995f;
      p2[ 1 ].axis[ 1 ] = 9.6969995499f;
      p2[ 1 ].axis[ 2 ] = 30.7469997406f;
      p2[ 2 ].axis[ 0 ] = -0.1940000057f;
      p2[ 2 ].axis[ 1 ] = 12.0380001068f;
      p2[ 2 ].axis[ 2 ] = 31.5860004425f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = 3.1686227322f;
      new_waters[ 0 ].axis[ 1 ] = 11.6081609726f;
      new_waters[ 0 ].axis[ 2 ] = 30.5056934357f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = -6.5110001564f;
      p2[ 0 ].axis[ 1 ] = 14.8620004654f;
      p2[ 0 ].axis[ 2 ] = 24.5359992981f;
      p2[ 1 ].axis[ 0 ] = -5.6020002365f;
      p2[ 1 ].axis[ 1 ] = 15.6470003128f;
      p2[ 1 ].axis[ 2 ] = 25.4720001221f;
      p2[ 2 ].axis[ 0 ] = -6.0469999313f;
      p2[ 2 ].axis[ 1 ] = 14.9729995728f;
      p2[ 2 ].axis[ 2 ] = 23.1399993896f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = -6.7334380150f;
      new_waters[ 0 ].axis[ 1 ] = 15.8349266052f;
      new_waters[ 0 ].axis[ 2 ] = 21.0516109467f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = -4.4850001335f;
      p2[ 0 ].axis[ 1 ] = 25.4139995575f;
      p2[ 0 ].axis[ 2 ] = 31.9549999237f;
      p2[ 1 ].axis[ 0 ] = -4.2389998436f;
      p2[ 1 ].axis[ 1 ] = 26.8789997101f;
      p2[ 1 ].axis[ 2 ] = 32.2649993896f;
      p2[ 2 ].axis[ 0 ] = -5.4949998856f;
      p2[ 2 ].axis[ 1 ] = 25.0610008240f;
      p2[ 2 ].axis[ 2 ] = 30.9810009003f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = -5.3745450974f;
      new_waters[ 0 ].axis[ 1 ] = 23.1893024445f;
      new_waters[ 0 ].axis[ 2 ] = 29.5259304047f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = -6.8480000496f;
      p2[ 0 ].axis[ 1 ] = 35.4080009460f;
      p2[ 0 ].axis[ 2 ] = 9.6199998856f;
      p2[ 1 ].axis[ 0 ] = -6.6929998398f;
      p2[ 1 ].axis[ 1 ] = 33.8919982910f;
      p2[ 1 ].axis[ 2 ] = 9.5670003891f;
      p2[ 2 ].axis[ 0 ] = -5.5729999542f;
      p2[ 2 ].axis[ 1 ] = 35.9729995728f;
      p2[ 2 ].axis[ 2 ] = 10.1020002365f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = -2.9410345554f;
      new_waters[ 0 ].axis[ 1 ] = 34.6006965637f;
      new_waters[ 0 ].axis[ 2 ] = 9.8387384415f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = -13.2609996796f;
      p2[ 0 ].axis[ 1 ] = 23.1779994965f;
      p2[ 0 ].axis[ 2 ] = 16.8840007782f;
      p2[ 1 ].axis[ 0 ] = -13.9860000610f;
      p2[ 1 ].axis[ 1 ] = 23.9120006561f;
      p2[ 1 ].axis[ 2 ] = 17.9790000916f;
      p2[ 2 ].axis[ 0 ] = -12.7040004730f;
      p2[ 2 ].axis[ 1 ] = 21.9300003052f;
      p2[ 2 ].axis[ 2 ] = 17.4050006866f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = -16.5785522461f;
      new_waters[ 0 ].axis[ 1 ] = 23.2525806427f;
      new_waters[ 0 ].axis[ 2 ] = 18.5693531036f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = 16.6259994507f;
      p2[ 0 ].axis[ 1 ] = 22.8320007324f;
      p2[ 0 ].axis[ 2 ] = 29.2609996796f;
      p2[ 1 ].axis[ 0 ] = 16.7000007629f;
      p2[ 1 ].axis[ 1 ] = 21.3059997559f;
      p2[ 1 ].axis[ 2 ] = 29.3059997559f;
      p2[ 2 ].axis[ 0 ] = 16.7970008850f;
      p2[ 2 ].axis[ 1 ] = 23.3390007019f;
      p2[ 2 ].axis[ 2 ] = 27.9349994659f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = 17.6787776947f;
      new_waters[ 0 ].axis[ 1 ] = 24.1398200989f;
      new_waters[ 0 ].axis[ 2 ] = 25.9433555603f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = 8.7670001984f;
      p2[ 0 ].axis[ 1 ] = 18.6079998016f;
      p2[ 0 ].axis[ 2 ] = 22.1809997559f;
      p2[ 1 ].axis[ 0 ] = 8.4510002136f;
      p2[ 1 ].axis[ 1 ] = 17.4629993439f;
      p2[ 1 ].axis[ 2 ] = 21.1979999542f;
      p2[ 2 ].axis[ 0 ] = 10.1579999924f;
      p2[ 2 ].axis[ 1 ] = 18.3540000916f;
      p2[ 2 ].axis[ 2 ] = 22.7010002136f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = 12.2161245346f;
      new_waters[ 0 ].axis[ 1 ] = 16.7776546478f;
      new_waters[ 0 ].axis[ 2 ] = 21.2523517609f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = 10.1529998779f;
      p2[ 0 ].axis[ 1 ] = 10.4340000153f;
      p2[ 0 ].axis[ 2 ] = 27.9950008392f;
      p2[ 1 ].axis[ 0 ] = 11.3540000916f;
      p2[ 1 ].axis[ 1 ] = 9.7419996262f;
      p2[ 1 ].axis[ 2 ] = 28.6009998322f;
      p2[ 2 ].axis[ 0 ] = 9.2539997101f;
      p2[ 2 ].axis[ 1 ] = 10.9930000305f;
      p2[ 2 ].axis[ 2 ] = 28.9820003510f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = 10.2220964432f;
      new_waters[ 0 ].axis[ 1 ] = 11.6820335388f;
      new_waters[ 0 ].axis[ 2 ] = 31.7418785095f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = -3.6110000610f;
      p2[ 0 ].axis[ 1 ] = 8.4829998016f;
      p2[ 0 ].axis[ 2 ] = 15.1949996948f;
      p2[ 1 ].axis[ 0 ] = -3.4260001183f;
      p2[ 1 ].axis[ 1 ] = 9.6730003357f;
      p2[ 1 ].axis[ 2 ] = 16.1620006561f;
      p2[ 2 ].axis[ 0 ] = -2.2880001068f;
      p2[ 2 ].axis[ 1 ] = 8.2270002365f;
      p2[ 2 ].axis[ 2 ] = 14.5749998093f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = -4.4950575829f;
      new_waters[ 0 ].axis[ 1 ] = 11.5652656555f;
      new_waters[ 0 ].axis[ 2 ] = 13.7575988770f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = -12.0089998245f;
      p2[ 0 ].axis[ 1 ] = 22.0590000153f;
      p2[ 0 ].axis[ 2 ] = 3.1640000343f;
      p2[ 1 ].axis[ 0 ] = -13.4420003891f;
      p2[ 1 ].axis[ 1 ] = 21.5779991150f;
      p2[ 1 ].axis[ 2 ] = 3.2909998894f;
      p2[ 2 ].axis[ 0 ] = -11.9119997025f;
      p2[ 2 ].axis[ 1 ] = 23.4090003967f;
      p2[ 2 ].axis[ 2 ] = 2.6849999428f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = -14.1830692291f;
      new_waters[ 0 ].axis[ 1 ] = 25.3524169922f;
      new_waters[ 0 ].axis[ 2 ] = 3.0011897087f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = -7.7670001984f;
      p2[ 0 ].axis[ 1 ] = 19.5720005035f;
      p2[ 0 ].axis[ 2 ] = 5.1890001297f;
      p2[ 1 ].axis[ 0 ] = -7.9970002174f;
      p2[ 1 ].axis[ 1 ] = 18.2689990997f;
      p2[ 1 ].axis[ 2 ] = 5.9159998894f;
      p2[ 2 ].axis[ 0 ] = -6.3270001411f;
      p2[ 2 ].axis[ 1 ] = 19.8659992218f;
      p2[ 2 ].axis[ 2 ] = 4.9670000076f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = -8.9755496979f;
      new_waters[ 0 ].axis[ 1 ] = 17.1326103210f;
      new_waters[ 0 ].axis[ 2 ] = 3.1230370998f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = -1.1529999971f;
      p2[ 0 ].axis[ 1 ] = 13.9700002670f;
      p2[ 0 ].axis[ 2 ] = 30.5130004883f;
      p2[ 1 ].axis[ 0 ] = -1.1369999647f;
      p2[ 1 ].axis[ 1 ] = 12.4630002975f;
      p2[ 1 ].axis[ 2 ] = 30.7380008698f;
      p2[ 2 ].axis[ 0 ] = -0.1459999979f;
      p2[ 2 ].axis[ 1 ] = 14.6630001068f;
      p2[ 2 ].axis[ 2 ] = 31.3589992523f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = 2.2573664188f;
      new_waters[ 0 ].axis[ 1 ] = 13.2232627869f;
      new_waters[ 0 ].axis[ 2 ] = 32.4082031250f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = -9.8079996109f;
      p2[ 0 ].axis[ 1 ] = 30.9810009003f;
      p2[ 0 ].axis[ 2 ] = 9.0389995575f;
      p2[ 1 ].axis[ 0 ] = -9.2849998474f;
      p2[ 1 ].axis[ 1 ] = 30.4990005493f;
      p2[ 1 ].axis[ 2 ] = 7.6939997673f;
      p2[ 2 ].axis[ 0 ] = -8.7609996796f;
      p2[ 2 ].axis[ 1 ] = 31.6790008545f;
      p2[ 2 ].axis[ 2 ] = 9.7760000229f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = -6.0493431091f;
      new_waters[ 0 ].axis[ 1 ] = 29.8135013580f;
      new_waters[ 0 ].axis[ 2 ] = 8.7392845154f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = 6.8600001335f;
      p2[ 0 ].axis[ 1 ] = 12.3079996109f;
      p2[ 0 ].axis[ 2 ] = 14.0190000534f;
      p2[ 1 ].axis[ 0 ] = 7.8610000610f;
      p2[ 1 ].axis[ 1 ] = 13.3719997406f;
      p2[ 1 ].axis[ 2 ] = 14.4329996109f;
      p2[ 2 ].axis[ 0 ] = 5.5939998627f;
      p2[ 2 ].axis[ 1 ] = 12.8190002441f;
      p2[ 2 ].axis[ 2 ] = 13.5590000153f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = 5.3864479065f;
      new_waters[ 0 ].axis[ 1 ] = 15.6256914139f;
      new_waters[ 0 ].axis[ 2 ] = 12.4886960983f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = 4.8889999390f;
      p2[ 0 ].axis[ 1 ] = 23.5429992676f;
      p2[ 0 ].axis[ 2 ] = 15.2629995346f;
      p2[ 1 ].axis[ 0 ] = 5.6409997940f;
      p2[ 1 ].axis[ 1 ] = 22.3519992828f;
      p2[ 1 ].axis[ 2 ] = 14.7060003281f;
      p2[ 2 ].axis[ 0 ] = 4.2010002136f;
      p2[ 2 ].axis[ 1 ] = 24.3239994049f;
      p2[ 2 ].axis[ 2 ] = 14.2250003815f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = 3.4586744308f;
      new_waters[ 0 ].axis[ 1 ] = 23.0745162964f;
      new_waters[ 0 ].axis[ 2 ] = 11.6015291214f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = -7.3090000153f;
      p2[ 0 ].axis[ 1 ] = 15.9809999466f;
      p2[ 0 ].axis[ 2 ] = 6.3229999542f;
      p2[ 1 ].axis[ 0 ] = -7.1290001869f;
      p2[ 1 ].axis[ 1 ] = 16.1809997559f;
      p2[ 1 ].axis[ 2 ] = 7.8480000496f;
      p2[ 2 ].axis[ 0 ] = -7.1420001984f;
      p2[ 2 ].axis[ 1 ] = 17.2740001678f;
      p2[ 2 ].axis[ 2 ] = 5.6529998779f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = -10.2422647476f;
      new_waters[ 0 ].axis[ 1 ] = 15.6339702606f;
      new_waters[ 0 ].axis[ 2 ] = 5.1925196648f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = -4.5250000954f;
      p2[ 0 ].axis[ 1 ] = 29.1700000763f;
      p2[ 0 ].axis[ 2 ] = 32.0929985046f;
      p2[ 1 ].axis[ 0 ] = -4.0819997787f;
      p2[ 1 ].axis[ 1 ] = 30.1919994354f;
      p2[ 1 ].axis[ 2 ] = 31.0489997864f;
      p2[ 2 ].axis[ 0 ] = -4.8759999275f;
      p2[ 2 ].axis[ 1 ] = 27.8199996948f;
      p2[ 2 ].axis[ 2 ] = 31.6170005798f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = -1.6474876404f;
      new_waters[ 0 ].axis[ 1 ] = 28.4059467316f;
      new_waters[ 0 ].axis[ 2 ] = 29.7208862305f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = -4.2129998207f;
      p2[ 0 ].axis[ 1 ] = 37.7369995117f;
      p2[ 0 ].axis[ 2 ] = 15.4930000305f;
      p2[ 1 ].axis[ 0 ] = -4.0910000801f;
      p2[ 1 ].axis[ 1 ] = 37.7589988708f;
      p2[ 1 ].axis[ 2 ] = 13.9720001221f;
      p2[ 2 ].axis[ 0 ] = -3.8619999886f;
      p2[ 2 ].axis[ 1 ] = 36.4620018005f;
      p2[ 2 ].axis[ 2 ] = 16.1310005188f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = -2.0560891628f;
      new_waters[ 0 ].axis[ 1 ] = 34.5336036682f;
      new_waters[ 0 ].axis[ 2 ] = 14.6928853989f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = -11.7410001755f;
      p2[ 0 ].axis[ 1 ] = 24.9589996338f;
      p2[ 0 ].axis[ 2 ] = 0.8579999804f;
      p2[ 1 ].axis[ 0 ] = -11.9029998779f;
      p2[ 1 ].axis[ 1 ] = 23.5699996948f;
      p2[ 1 ].axis[ 2 ] = 1.3559999466f;
      p2[ 2 ].axis[ 0 ] = -11.5810003281f;
      p2[ 2 ].axis[ 1 ] = 25.9570007324f;
      p2[ 2 ].axis[ 2 ] = 1.9170000553f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = -10.5756359100f;
      new_waters[ 0 ].axis[ 1 ] = 25.1394596100f;
      new_waters[ 0 ].axis[ 2 ] = 4.6261324883f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = -13.6870002747f;
      p2[ 0 ].axis[ 1 ] = 17.9559993744f;
      p2[ 0 ].axis[ 2 ] = 19.4009990692f;
      p2[ 1 ].axis[ 0 ] = -12.8710002899f;
      p2[ 1 ].axis[ 1 ] = 19.2269992828f;
      p2[ 1 ].axis[ 2 ] = 19.5540008545f;
      p2[ 2 ].axis[ 0 ] = -13.1459999084f;
      p2[ 2 ].axis[ 1 ] = 16.9519996643f;
      p2[ 2 ].axis[ 2 ] = 18.5510005951f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = -12.0736856461f;
      new_waters[ 0 ].axis[ 1 ] = 20.5339050293f;
      new_waters[ 0 ].axis[ 2 ] = 17.0542984009f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = -13.2899999619f;
      p2[ 0 ].axis[ 1 ] = 29.3120002747f;
      p2[ 0 ].axis[ 2 ] = 22.2530002594f;
      p2[ 1 ].axis[ 0 ] = -12.2760000229f;
      p2[ 1 ].axis[ 1 ] = 29.4990005493f;
      p2[ 1 ].axis[ 2 ] = 21.1250000000f;
      p2[ 2 ].axis[ 0 ] = -13.1730003357f;
      p2[ 2 ].axis[ 1 ] = 28.0760002136f;
      p2[ 2 ].axis[ 2 ] = 23.0450000763f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = -10.5220785141f;
      new_waters[ 0 ].axis[ 1 ] = 26.6472034454f;
      new_waters[ 0 ].axis[ 2 ] = 23.1088619232f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = 16.9500007629f;
      p2[ 0 ].axis[ 1 ] = 19.2049999237f;
      p2[ 0 ].axis[ 2 ] = 28.1819992065f;
      p2[ 1 ].axis[ 0 ] = 15.6079998016f;
      p2[ 1 ].axis[ 1 ] = 18.5340003967f;
      p2[ 1 ].axis[ 2 ] = 27.9769992828f;
      p2[ 2 ].axis[ 0 ] = 16.8829994202f;
      p2[ 2 ].axis[ 1 ] = 20.6369991302f;
      p2[ 2 ].axis[ 2 ] = 28.1660003662f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = 14.3194484711f;
      new_waters[ 0 ].axis[ 1 ] = 22.0805587769f;
      new_waters[ 0 ].axis[ 2 ] = 28.7934226990f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = -2.6449999809f;
      p2[ 0 ].axis[ 1 ] = 17.2730007172f;
      p2[ 0 ].axis[ 2 ] = 4.5110001564f;
      p2[ 1 ].axis[ 0 ] = -3.4560000896f;
      p2[ 1 ].axis[ 1 ] = 18.2609996796f;
      p2[ 1 ].axis[ 2 ] = 5.3499999046f;
      p2[ 2 ].axis[ 0 ] = -2.2290000916f;
      p2[ 2 ].axis[ 1 ] = 16.2280006409f;
      p2[ 2 ].axis[ 2 ] = 5.3930001259f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = -3.6071760654f;
      new_waters[ 0 ].axis[ 1 ] = 15.4518222809f;
      new_waters[ 0 ].axis[ 2 ] = 7.9264087677f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = 12.4490003586f;
      p2[ 0 ].axis[ 1 ] = 8.9409999847f;
      p2[ 0 ].axis[ 2 ] = 30.6650009155f;
      p2[ 1 ].axis[ 0 ] = 13.7379999161f;
      p2[ 1 ].axis[ 1 ] = 9.6770000458f;
      p2[ 1 ].axis[ 2 ] = 30.6970005035f;
      p2[ 2 ].axis[ 0 ] = 11.3950004578f;
      p2[ 2 ].axis[ 1 ] = 9.6440000534f;
      p2[ 2 ].axis[ 2 ] = 29.9200000763f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = 13.3347482681f;
      new_waters[ 0 ].axis[ 1 ] = 6.5906925201f;
      new_waters[ 0 ].axis[ 2 ] = 28.5963726044f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = 11.7849998474f;
      p2[ 0 ].axis[ 1 ] = 16.6089992523f;
      p2[ 0 ].axis[ 2 ] = 37.1300010681f;
      p2[ 1 ].axis[ 0 ] = 10.5469999313f;
      p2[ 1 ].axis[ 1 ] = 16.7280006409f;
      p2[ 1 ].axis[ 2 ] = 36.2200012207f;
      p2[ 2 ].axis[ 0 ] = 12.6979999542f;
      p2[ 2 ].axis[ 1 ] = 15.5729999542f;
      p2[ 2 ].axis[ 2 ] = 36.6720008850f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = 12.7865209579f;
      new_waters[ 0 ].axis[ 1 ] = 14.6558380127f;
      new_waters[ 0 ].axis[ 2 ] = 33.8337402344f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = -11.7910003662f;
      p2[ 0 ].axis[ 1 ] = 15.0159997940f;
      p2[ 0 ].axis[ 2 ] = 17.8460006714f;
      p2[ 1 ].axis[ 0 ] = -12.2209997177f;
      p2[ 1 ].axis[ 1 ] = 16.0739994049f;
      p2[ 1 ].axis[ 2 ] = 18.8880004883f;
      p2[ 2 ].axis[ 0 ] = -12.3000001907f;
      p2[ 2 ].axis[ 1 ] = 15.2189998627f;
      p2[ 2 ].axis[ 2 ] = 16.4979991913f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = -14.4575881958f;
      new_waters[ 0 ].axis[ 1 ] = 14.7764644623f;
      new_waters[ 0 ].axis[ 2 ] = 15.2867078781f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = -9.5729999542f;
      p2[ 0 ].axis[ 1 ] = 25.6590003967f;
      p2[ 0 ].axis[ 2 ] = 6.8309998512f;
      p2[ 1 ].axis[ 0 ] = -10.3529996872f;
      p2[ 1 ].axis[ 1 ] = 25.9270000458f;
      p2[ 1 ].axis[ 2 ] = 5.5300002098f;
      p2[ 2 ].axis[ 0 ] = -8.5900001526f;
      p2[ 2 ].axis[ 1 ] = 26.6959991455f;
      p2[ 2 ].axis[ 2 ] = 7.1259999275f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = -7.0183134079f;
      new_waters[ 0 ].axis[ 1 ] = 28.4897575378f;
      new_waters[ 0 ].axis[ 2 ] = 6.7925977707f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = 1.3730000257f;
      p2[ 0 ].axis[ 1 ] = 9.1909999847f;
      p2[ 0 ].axis[ 2 ] = 28.8379993439f;
      p2[ 1 ].axis[ 0 ] = 2.8729999065f;
      p2[ 1 ].axis[ 1 ] = 9.2580003738f;
      p2[ 1 ].axis[ 2 ] = 28.4990005493f;
      p2[ 2 ].axis[ 0 ] = 1.1139999628f;
      p2[ 2 ].axis[ 1 ] = 10.0059995651f;
      p2[ 2 ].axis[ 2 ] = 29.9790000916f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = 2.8308827877f;
      new_waters[ 0 ].axis[ 1 ] = 12.3148326874f;
      new_waters[ 0 ].axis[ 2 ] = 30.8125953674f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = -15.9759998322f;
      p2[ 0 ].axis[ 1 ] = 19.8689994812f;
      p2[ 0 ].axis[ 2 ] = 6.8109998703f;
      p2[ 1 ].axis[ 0 ] = -17.4489994049f;
      p2[ 1 ].axis[ 1 ] = 19.9060001373f;
      p2[ 1 ].axis[ 2 ] = 7.1409997940f;
      p2[ 2 ].axis[ 0 ] = -15.7749996185f;
      p2[ 2 ].axis[ 1 ] = 20.2259998322f;
      p2[ 2 ].axis[ 2 ] = 5.4039998055f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = -17.7358684540f;
      new_waters[ 0 ].axis[ 1 ] = 21.8613491058f;
      new_waters[ 0 ].axis[ 2 ] = 3.8510077000f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = 0.4709999859f;
      p2[ 0 ].axis[ 1 ] = 15.7740001678f;
      p2[ 0 ].axis[ 2 ] = 33.4580001831f;
      p2[ 1 ].axis[ 0 ] = -0.5049999952f;
      p2[ 1 ].axis[ 1 ] = 15.0889997482f;
      p2[ 1 ].axis[ 2 ] = 32.5649986267f;
      p2[ 2 ].axis[ 0 ] = 1.8509999514f;
      p2[ 2 ].axis[ 1 ] = 15.4049997330f;
      p2[ 2 ].axis[ 2 ] = 33.3339996338f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = 2.0920007229f;
      new_waters[ 0 ].axis[ 1 ] = 13.7176361084f;
      new_waters[ 0 ].axis[ 2 ] = 30.4928264618f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = 4.1310000420f;
      p2[ 0 ].axis[ 1 ] = 10.5120000839f;
      p2[ 0 ].axis[ 2 ] = 17.6180000305f;
      p2[ 1 ].axis[ 0 ] = 3.3610000610f;
      p2[ 1 ].axis[ 1 ] = 9.6569995880f;
      p2[ 1 ].axis[ 2 ] = 16.6299991608f;
      p2[ 2 ].axis[ 0 ] = 3.5420000553f;
      p2[ 2 ].axis[ 1 ] = 10.5559997559f;
      p2[ 2 ].axis[ 2 ] = 18.9400005341f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = 4.4609069824f;
      new_waters[ 0 ].axis[ 1 ] = 9.1394433975f;
      new_waters[ 0 ].axis[ 2 ] = 20.9207687378f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = -2.3719999790f;
      p2[ 0 ].axis[ 1 ] = 34.7229995728f;
      p2[ 0 ].axis[ 2 ] = 16.9759998322f;
      p2[ 1 ].axis[ 0 ] = -2.5620000362f;
      p2[ 1 ].axis[ 1 ] = 36.0320014954f;
      p2[ 1 ].axis[ 2 ] = 16.2280006409f;
      p2[ 2 ].axis[ 0 ] = -1.6299999952f;
      p2[ 2 ].axis[ 1 ] = 33.7960014343f;
      p2[ 2 ].axis[ 2 ] = 16.1959991455f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = 0.2400287539f;
      new_waters[ 0 ].axis[ 1 ] = 34.6350250244f;
      new_waters[ 0 ].axis[ 2 ] = 14.0026445389f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = -12.8439998627f;
      p2[ 0 ].axis[ 1 ] = 18.5540008545f;
      p2[ 0 ].axis[ 2 ] = 14.1459999084f;
      p2[ 1 ].axis[ 0 ] = -13.5520000458f;
      p2[ 1 ].axis[ 1 ] = 17.4020004272f;
      p2[ 1 ].axis[ 2 ] = 14.7620000839f;
      p2[ 2 ].axis[ 0 ] = -11.4209995270f;
      p2[ 2 ].axis[ 1 ] = 18.2390003204f;
      p2[ 2 ].axis[ 2 ] = 13.9849996567f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = -10.4019260406f;
      new_waters[ 0 ].axis[ 1 ] = 15.4353189468f;
      new_waters[ 0 ].axis[ 2 ] = 13.8063583374f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = 2.3880000114f;
      p2[ 0 ].axis[ 1 ] = 10.5329999924f;
      p2[ 0 ].axis[ 2 ] = 9.1680002213f;
      p2[ 1 ].axis[ 0 ] = 2.4379999638f;
      p2[ 1 ].axis[ 1 ] = 12.0489997864f;
      p2[ 1 ].axis[ 2 ] = 8.8889999390f;
      p2[ 2 ].axis[ 0 ] = 3.2939999104f;
      p2[ 2 ].axis[ 1 ] = 10.1639995575f;
      p2[ 2 ].axis[ 2 ] = 10.2659997940f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = 3.1712613106f;
      new_waters[ 0 ].axis[ 1 ] = 13.3271999359f;
      new_waters[ 0 ].axis[ 2 ] = 11.8864316940f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = 1.5420000553f;
      p2[ 0 ].axis[ 1 ] = 23.7950000763f;
      p2[ 0 ].axis[ 2 ] = 10.6350002289f;
      p2[ 1 ].axis[ 0 ] = 2.8670001030f;
      p2[ 1 ].axis[ 1 ] = 24.3330001831f;
      p2[ 1 ].axis[ 2 ] = 11.0970001221f;
      p2[ 2 ].axis[ 0 ] = 0.6529999971f;
      p2[ 2 ].axis[ 1 ] = 23.4629993439f;
      p2[ 2 ].axis[ 2 ] = 11.7860002518f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = 1.0229842663f;
      new_waters[ 0 ].axis[ 1 ] = 24.7594432831f;
      new_waters[ 0 ].axis[ 2 ] = 14.4543294907f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = -7.1079998016f;
      p2[ 0 ].axis[ 1 ] = 19.9270000458f;
      p2[ 0 ].axis[ 2 ] = 25.5960006714f;
      p2[ 1 ].axis[ 0 ] = -6.8039999008f;
      p2[ 1 ].axis[ 1 ] = 20.2290000916f;
      p2[ 1 ].axis[ 2 ] = 27.0909996033f;
      p2[ 2 ].axis[ 0 ] = -5.9320001602f;
      p2[ 2 ].axis[ 1 ] = 19.4050006866f;
      p2[ 2 ].axis[ 2 ] = 24.9659996033f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = -3.7216355801f;
      new_waters[ 0 ].axis[ 1 ] = 18.0300827026f;
      new_waters[ 0 ].axis[ 2 ] = 26.4482479095f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = -5.5380001068f;
      p2[ 0 ].axis[ 1 ] = 19.7059993744f;
      p2[ 0 ].axis[ 2 ] = 29.1580009460f;
      p2[ 1 ].axis[ 0 ] = -4.6719999313f;
      p2[ 1 ].axis[ 1 ] = 20.9810009003f;
      p2[ 1 ].axis[ 2 ] = 29.2089996338f;
      p2[ 2 ].axis[ 0 ] = -5.9439997673f;
      p2[ 2 ].axis[ 1 ] = 19.4470005035f;
      p2[ 2 ].axis[ 2 ] = 27.7500000000f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = -4.5291733742f;
      new_waters[ 0 ].axis[ 1 ] = 19.3430080414f;
      new_waters[ 0 ].axis[ 2 ] = 25.4210224152f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = 4.0289998055f;
      p2[ 0 ].axis[ 1 ] = 25.9580001831f;
      p2[ 0 ].axis[ 2 ] = 12.4359998703f;
      p2[ 1 ].axis[ 0 ] = 4.8460001945f;
      p2[ 1 ].axis[ 1 ] = 25.1919994354f;
      p2[ 1 ].axis[ 2 ] = 13.4549999237f;
      p2[ 2 ].axis[ 0 ] = 2.8069999218f;
      p2[ 2 ].axis[ 1 ] = 25.3449993134f;
      p2[ 2 ].axis[ 2 ] = 11.9610004425f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = 2.7380404472f;
      new_waters[ 0 ].axis[ 1 ] = 28.5092658997f;
      new_waters[ 0 ].axis[ 2 ] = 13.8866415024f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = 1.8619999886f;
      p2[ 0 ].axis[ 1 ] = 18.6250000000f;
      p2[ 0 ].axis[ 2 ] = 10.0749998093f;
      p2[ 1 ].axis[ 0 ] = 2.4110000134f;
      p2[ 1 ].axis[ 1 ] = 17.2140007019f;
      p2[ 1 ].axis[ 2 ] = 10.1680002213f;
      p2[ 2 ].axis[ 0 ] = 2.8640000820f;
      p2[ 2 ].axis[ 1 ] = 19.6959991455f;
      p2[ 2 ].axis[ 2 ] = 10.2200002670f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = 5.7532615662f;
      new_waters[ 0 ].axis[ 1 ] = 19.1086616516f;
      new_waters[ 0 ].axis[ 2 ] = 9.6337137222f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = 14.5629997253f;
      p2[ 0 ].axis[ 1 ] = 14.9639997482f;
      p2[ 0 ].axis[ 2 ] = 35.3149986267f;
      p2[ 1 ].axis[ 0 ] = 13.6540002823f;
      p2[ 1 ].axis[ 1 ] = 16.0030002594f;
      p2[ 1 ].axis[ 2 ] = 35.9039993286f;
      p2[ 2 ].axis[ 0 ] = 14.4309997559f;
      p2[ 2 ].axis[ 1 ] = 14.9600000381f;
      p2[ 2 ].axis[ 2 ] = 33.8740005493f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = 12.4903669357f;
      new_waters[ 0 ].axis[ 1 ] = 15.4032363892f;
      new_waters[ 0 ].axis[ 2 ] = 32.0265998840f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = 4.7150001526f;
      p2[ 0 ].axis[ 1 ] = 8.2290000916f;
      p2[ 0 ].axis[ 2 ] = 27.3619995117f;
      p2[ 1 ].axis[ 0 ] = 4.7379999161f;
      p2[ 1 ].axis[ 1 ] = 9.2340002060f;
      p2[ 1 ].axis[ 2 ] = 26.1870002747f;
      p2[ 2 ].axis[ 0 ] = 3.3429999352f;
      p2[ 2 ].axis[ 1 ] = 8.2100000381f;
      p2[ 2 ].axis[ 2 ] = 27.8430004120f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = 0.9360138774f;
      new_waters[ 0 ].axis[ 1 ] = 8.9132175446f;
      new_waters[ 0 ].axis[ 2 ] = 26.2349510193f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = -7.4689998627f;
      p2[ 0 ].axis[ 1 ] = 24.5590000153f;
      p2[ 0 ].axis[ 2 ] = 29.7280006409f;
      p2[ 1 ].axis[ 0 ] = -6.8099999428f;
      p2[ 1 ].axis[ 1 ] = 25.2329998016f;
      p2[ 1 ].axis[ 2 ] = 30.9519996643f;
      p2[ 2 ].axis[ 0 ] = -6.5289998055f;
      p2[ 2 ].axis[ 1 ] = 24.2740001678f;
      p2[ 2 ].axis[ 2 ] = 28.6229991913f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = -8.2448272705f;
      new_waters[ 0 ].axis[ 1 ] = 26.6020450592f;
      new_waters[ 0 ].axis[ 2 ] = 27.1910343170f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = -11.7299995422f;
      p2[ 0 ].axis[ 1 ] = 28.0319995880f;
      p2[ 0 ].axis[ 2 ] = 16.2530002594f;
      p2[ 1 ].axis[ 0 ] = -10.7259998322f;
      p2[ 1 ].axis[ 1 ] = 27.0750007629f;
      p2[ 1 ].axis[ 2 ] = 15.6160001755f;
      p2[ 2 ].axis[ 0 ] = -11.2189998627f;
      p2[ 2 ].axis[ 1 ] = 28.5900001526f;
      p2[ 2 ].axis[ 2 ] = 17.5170001984f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = -8.2855644226f;
      new_waters[ 0 ].axis[ 1 ] = 28.7349681854f;
      new_waters[ 0 ].axis[ 2 ] = 18.0972290039f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = 9.1569995880f;
      p2[ 0 ].axis[ 1 ] = 15.8599996567f;
      p2[ 0 ].axis[ 2 ] = 34.5019989014f;
      p2[ 1 ].axis[ 0 ] = 7.9060001373f;
      p2[ 1 ].axis[ 1 ] = 15.6149997711f;
      p2[ 1 ].axis[ 2 ] = 35.3740005493f;
      p2[ 2 ].axis[ 0 ] = 10.3389997482f;
      p2[ 2 ].axis[ 1 ] = 15.7969999313f;
      p2[ 2 ].axis[ 2 ] = 35.3240013123f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = 5.9269938469f;
      new_waters[ 0 ].axis[ 1 ] = 14.9283819199f;
      new_waters[ 0 ].axis[ 2 ] = 36.0869789124f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = 6.2420001030f;
      p2[ 0 ].axis[ 1 ] = 8.4320001602f;
      p2[ 0 ].axis[ 2 ] = 22.1539993286f;
      p2[ 1 ].axis[ 0 ] = 4.8150000572f;
      p2[ 1 ].axis[ 1 ] = 8.2229995728f;
      p2[ 1 ].axis[ 2 ] = 21.6870002747f;
      p2[ 2 ].axis[ 0 ] = 6.2589998245f;
      p2[ 2 ].axis[ 1 ] = 9.1149997711f;
      p2[ 2 ].axis[ 2 ] = 23.4419994354f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = 3.9484860897f;
      new_waters[ 0 ].axis[ 1 ] = 9.0334959030f;
      new_waters[ 0 ].axis[ 2 ] = 25.3416309357f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = 1.4850000143f;
      p2[ 0 ].axis[ 1 ] = 8.1479997635f;
      p2[ 0 ].axis[ 2 ] = 16.3330001831f;
      p2[ 1 ].axis[ 0 ] = 0.7919999957f;
      p2[ 1 ].axis[ 1 ] = 8.8269996643f;
      p2[ 1 ].axis[ 2 ] = 15.1940002441f;
      p2[ 2 ].axis[ 0 ] = 2.3320000172f;
      p2[ 2 ].axis[ 1 ] = 9.0229997635f;
      p2[ 2 ].axis[ 2 ] = 17.0960006714f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = 3.5013523102f;
      new_waters[ 0 ].axis[ 1 ] = 11.0317707062f;
      new_waters[ 0 ].axis[ 2 ] = 17.8060798645f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = -0.4300000072f;
      p2[ 0 ].axis[ 1 ] = 8.5050001144f;
      p2[ 0 ].axis[ 2 ] = 13.0249996185f;
      p2[ 1 ].axis[ 0 ] = -1.8270000219f;
      p2[ 1 ].axis[ 1 ] = 8.8839998245f;
      p2[ 1 ].axis[ 2 ] = 13.4870004654f;
      p2[ 2 ].axis[ 0 ] = 0.3709999919f;
      p2[ 2 ].axis[ 1 ] = 8.0389995575f;
      p2[ 2 ].axis[ 2 ] = 14.1859998703f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = -3.3137164116f;
      new_waters[ 0 ].axis[ 1 ] = 7.7971782684f;
      new_waters[ 0 ].axis[ 2 ] = 15.6499385834f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = -3.9400000572f;
      p2[ 0 ].axis[ 1 ] = 36.5369987488f;
      p2[ 0 ].axis[ 2 ] = 11.8850002289f;
      p2[ 1 ].axis[ 0 ] = -5.2849998474f;
      p2[ 1 ].axis[ 1 ] = 35.9770011902f;
      p2[ 1 ].axis[ 2 ] = 11.4069995880f;
      p2[ 2 ].axis[ 0 ] = -4.0190000534f;
      p2[ 2 ].axis[ 1 ] = 36.6119995117f;
      p2[ 2 ].axis[ 2 ] = 13.3400001526f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = -5.0966267586f;
      new_waters[ 0 ].axis[ 1 ] = 35.0073432922f;
      new_waters[ 0 ].axis[ 2 ] = 15.3213930130f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = 10.7049999237f;
      p2[ 0 ].axis[ 1 ] = 15.9919996262f;
      p2[ 0 ].axis[ 2 ] = 17.7730007172f;
      p2[ 1 ].axis[ 0 ] = 11.6940002441f;
      p2[ 1 ].axis[ 1 ] = 17.1119995117f;
      p2[ 1 ].axis[ 2 ] = 17.6919994354f;
      p2[ 2 ].axis[ 0 ] = 10.1319999695f;
      p2[ 2 ].axis[ 1 ] = 15.8649997711f;
      p2[ 2 ].axis[ 2 ] = 16.4449996948f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = 11.4515657425f;
      new_waters[ 0 ].axis[ 1 ] = 16.7167568207f;
      new_waters[ 0 ].axis[ 2 ] = 13.9138641357f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = 18.1079998016f;
      p2[ 0 ].axis[ 1 ] = 23.7679996490f;
      p2[ 0 ].axis[ 2 ] = 25.9309997559f;
      p2[ 1 ].axis[ 0 ] = 17.9960002899f;
      p2[ 1 ].axis[ 1 ] = 23.2689990997f;
      p2[ 1 ].axis[ 2 ] = 27.3579998016f;
      p2[ 2 ].axis[ 0 ] = 16.9060001373f;
      p2[ 2 ].axis[ 1 ] = 23.4890003204f;
      p2[ 2 ].axis[ 2 ] = 25.1459999084f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = 14.2477331161f;
      new_waters[ 0 ].axis[ 1 ] = 23.0643959045f;
      new_waters[ 0 ].axis[ 2 ] = 26.4671802521f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = -6.0580000877f;
      p2[ 0 ].axis[ 1 ] = 11.1029996872f;
      p2[ 0 ].axis[ 2 ] = 19.5839996338f;
      p2[ 1 ].axis[ 0 ] = -5.3260002136f;
      p2[ 1 ].axis[ 1 ] = 11.5200004578f;
      p2[ 1 ].axis[ 2 ] = 20.9069995880f;
      p2[ 2 ].axis[ 0 ] = -5.2569999695f;
      p2[ 2 ].axis[ 1 ] = 11.0690002441f;
      p2[ 2 ].axis[ 2 ] = 18.3810005188f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = -2.2636845112f;
      new_waters[ 0 ].axis[ 1 ] = 10.7555465698f;
      new_waters[ 0 ].axis[ 2 ] = 18.6204128265f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = 0.8209999800f;
      p2[ 0 ].axis[ 1 ] = 29.9009990692f;
      p2[ 0 ].axis[ 2 ] = 17.7889995575f;
      p2[ 1 ].axis[ 0 ] = 0.8149999976f;
      p2[ 1 ].axis[ 1 ] = 31.3990001678f;
      p2[ 1 ].axis[ 2 ] = 18.1000003815f;
      p2[ 2 ].axis[ 0 ] = 2.1900000572f;
      p2[ 2 ].axis[ 1 ] = 29.3980007172f;
      p2[ 2 ].axis[ 2 ] = 17.8920001984f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = 0.9836748242f;
      new_waters[ 0 ].axis[ 1 ] = 33.6837463379f;
      new_waters[ 0 ].axis[ 2 ] = 18.0614986420f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = -6.3039999008f;
      p2[ 0 ].axis[ 1 ] = 27.4599990845f;
      p2[ 0 ].axis[ 2 ] = 7.3049998283f;
      p2[ 1 ].axis[ 0 ] = -7.3260002136f;
      p2[ 1 ].axis[ 1 ] = 26.3689994812f;
      p2[ 1 ].axis[ 2 ] = 7.0310001373f;
      p2[ 2 ].axis[ 0 ] = -6.7919998169f;
      p2[ 2 ].axis[ 1 ] = 28.8290004730f;
      p2[ 2 ].axis[ 2 ] = 7.1380000114f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = -6.6391248703f;
      new_waters[ 0 ].axis[ 1 ] = 30.7492179871f;
      new_waters[ 0 ].axis[ 2 ] = 8.4872236252f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = 5.2789998055f;
      p2[ 0 ].axis[ 1 ] = 18.9549999237f;
      p2[ 0 ].axis[ 2 ] = 32.0260009766f;
      p2[ 1 ].axis[ 0 ] = 4.2810001373f;
      p2[ 1 ].axis[ 1 ] = 17.9160003662f;
      p2[ 1 ].axis[ 2 ] = 31.6819992065f;
      p2[ 2 ].axis[ 0 ] = 6.5419998169f;
      p2[ 2 ].axis[ 1 ] = 19.0709991455f;
      p2[ 2 ].axis[ 2 ] = 31.3290004730f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = 6.6367459297f;
      new_waters[ 0 ].axis[ 1 ] = 17.8349895477f;
      new_waters[ 0 ].axis[ 2 ] = 28.5578193665f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = 3.0360000134f;
      p2[ 0 ].axis[ 1 ] = 16.8579998016f;
      p2[ 0 ].axis[ 2 ] = 29.8479995728f;
      p2[ 1 ].axis[ 0 ] = 3.1549999714f;
      p2[ 1 ].axis[ 1 ] = 15.3959999084f;
      p2[ 1 ].axis[ 2 ] = 29.5919990540f;
      p2[ 2 ].axis[ 0 ] = 4.1519999504f;
      p2[ 2 ].axis[ 1 ] = 17.6000003815f;
      p2[ 2 ].axis[ 2 ] = 30.3850002289f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = 3.8976631165f;
      new_waters[ 0 ].axis[ 1 ] = 13.3194742203f;
      new_waters[ 0 ].axis[ 2 ] = 28.9771289825f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = -12.6520004272f;
      p2[ 0 ].axis[ 1 ] = 24.0270004272f;
      p2[ 0 ].axis[ 2 ] = 22.1060009003f;
      p2[ 1 ].axis[ 0 ] = -12.0369997025f;
      p2[ 1 ].axis[ 1 ] = 25.1590003967f;
      p2[ 1 ].axis[ 2 ] = 22.9290008545f;
      p2[ 2 ].axis[ 0 ] = -12.9560003281f;
      p2[ 2 ].axis[ 1 ] = 24.5520000458f;
      p2[ 2 ].axis[ 2 ] = 20.8269996643f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = -11.6141738892f;
      new_waters[ 0 ].axis[ 1 ] = 26.9079780579f;
      new_waters[ 0 ].axis[ 2 ] = 19.5627994537f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = -10.4189996719f;
      p2[ 0 ].axis[ 1 ] = 28.5839996338f;
      p2[ 0 ].axis[ 2 ] = 19.7870006561f;
      p2[ 1 ].axis[ 0 ] = -10.9639997482f;
      p2[ 1 ].axis[ 1 ] = 27.8320007324f;
      p2[ 1 ].axis[ 2 ] = 18.5639991760f;
      p2[ 2 ].axis[ 0 ] = -11.4139995575f;
      p2[ 2 ].axis[ 1 ] = 28.5109996796f;
      p2[ 2 ].axis[ 2 ] = 20.8630008698f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = -13.4726715088f;
      new_waters[ 0 ].axis[ 1 ] = 26.3589687347f;
      new_waters[ 0 ].axis[ 2 ] = 21.0635738373f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = 6.8990001678f;
      p2[ 0 ].axis[ 1 ] = 15.7449998856f;
      p2[ 0 ].axis[ 2 ] = 20.5480003357f;
      p2[ 1 ].axis[ 0 ] = 5.5799999237f;
      p2[ 1 ].axis[ 1 ] = 15.9219999313f;
      p2[ 1 ].axis[ 2 ] = 19.7900009155f;
      p2[ 2 ].axis[ 0 ] = 7.2789998055f;
      p2[ 2 ].axis[ 1 ] = 16.9080009460f;
      p2[ 2 ].axis[ 2 ] = 21.2800006866f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = 5.2570157051f;
      new_waters[ 0 ].axis[ 1 ] = 18.9585876465f;
      new_waters[ 0 ].axis[ 2 ] = 22.1490058899f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = 5.0460000038f;
      p2[ 0 ].axis[ 1 ] = 29.6720008850f;
      p2[ 0 ].axis[ 2 ] = 21.0440006256f;
      p2[ 1 ].axis[ 0 ] = 4.8000001907f;
      p2[ 1 ].axis[ 1 ] = 30.0249996185f;
      p2[ 1 ].axis[ 2 ] = 19.5450000763f;
      p2[ 2 ].axis[ 0 ] = 4.3200001717f;
      p2[ 2 ].axis[ 1 ] = 28.5230007172f;
      p2[ 2 ].axis[ 2 ] = 21.5079994202f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = 3.3635520935f;
      new_waters[ 0 ].axis[ 1 ] = 26.4241294861f;
      new_waters[ 0 ].axis[ 2 ] = 19.5710945129f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = -6.6370000839f;
      p2[ 0 ].axis[ 1 ] = 31.8349990845f;
      p2[ 0 ].axis[ 2 ] = 10.8950004578f;
      p2[ 1 ].axis[ 0 ] = -7.6640000343f;
      p2[ 1 ].axis[ 1 ] = 31.0109996796f;
      p2[ 1 ].axis[ 2 ] = 10.1490001678f;
      p2[ 2 ].axis[ 0 ] = -6.8359999657f;
      p2[ 2 ].axis[ 1 ] = 33.2910003662f;
      p2[ 2 ].axis[ 2 ] = 10.7500000000f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = -9.5351915359f;
      new_waters[ 0 ].axis[ 1 ] = 34.3932189941f;
      new_waters[ 0 ].axis[ 2 ] = 10.0463275909f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << QString( "testing water %1\n" ).arg( ++waterpos );
   {
      vector < point > p1( 3 );
      vector < point > p2( 3 );
      vector < point > rotamer_waters( 1 );
      vector < point > new_waters( 1 );
      vector < point > cmp_waters( 1 );
      p1[ 0 ].axis[ 0 ] = 15.6979999542f;
      p1[ 0 ].axis[ 1 ] = 15.6829996109f;
      p1[ 0 ].axis[ 2 ] = 16.2970008850f;
      p1[ 1 ].axis[ 0 ] = 17.1660003662f;
      p1[ 1 ].axis[ 1 ] = 15.4280004501f;
      p1[ 1 ].axis[ 2 ] = 16.5429992676f;
      p1[ 2 ].axis[ 0 ] = 15.4700002670f;
      p1[ 2 ].axis[ 1 ] = 16.1550006866f;
      p1[ 2 ].axis[ 2 ] = 14.9350004196f;
      p2[ 0 ].axis[ 0 ] = 2.5239999294f;
      p2[ 0 ].axis[ 1 ] = 13.8400001526f;
      p2[ 0 ].axis[ 2 ] = 7.2820000648f;
      p2[ 1 ].axis[ 0 ] = 1.1200000048f;
      p2[ 1 ].axis[ 1 ] = 14.1800003052f;
      p2[ 1 ].axis[ 2 ] = 6.7699999809f;
      p2[ 2 ].axis[ 0 ] = 2.5520000458f;
      p2[ 2 ].axis[ 1 ] = 12.4280004501f;
      p2[ 2 ].axis[ 2 ] = 7.6259999275f;
      rotamer_waters[ 0 ].axis[ 0 ] = 17.3840007782f;
      rotamer_waters[ 0 ].axis[ 1 ] = 15.5780000687f;
      rotamer_waters[ 0 ].axis[ 2 ] = 12.7060003281f;
      new_waters[ 0 ].axis[ 0 ] = 1.4371086359f;
      new_waters[ 0 ].axis[ 1 ] = 10.1066093445f;
      new_waters[ 0 ].axis[ 2 ] = 6.7562508583f;
      if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )
      {
         return false;
      }
      for ( unsigned int i = 0; i < cmp_waters.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )
            {
               errormsg = QString( "mismatch water %1 axis %2\n" ).arg( i ).arg( j );
               return false;
            }
         }
      }
   }
   cout << "align_test() results ok!\n";
   return true;
#else 
   errormsg = "align_test() not active";
   cout << errormsg << endl << flush;
   return false;
#endif
}
