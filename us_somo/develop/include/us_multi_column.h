#ifndef US_MULTI_COLUMN_H
#define US_MULTI_COLUMN_H

#include "us_saxs_util.h"
#if QT_VERSION >= 0x040000
# include "qdebug.h"
#endif
#include "us_vector.h"
//Added by qt3to4:
#include <QTextStream>

#ifdef WIN32
# if QT_VERSION < 0x040000
  #pragma warning ( disable: 4251 )
# endif
#endif

class umc_sortable_double {
   // sorts the vector in column order
public:
   double        d;
   unsigned int  index;
   bool operator < (const umc_sortable_double& objIn) const
   {
      //      unsigned int i;
      return d < objIn.d;
   }
};

class US_Multi_Column
{
 public:

   US_Multi_Column() 
      {
         clear( );
      }

   US_Multi_Column( QString filename ) 
      {
         clear( );
         this->filename = filename;
         is_csv = filename.right( 4 ).contains( ".csv", Qt::CaseInsensitive );
      }

   QString filename;
   vector < QString >           header;
   vector < vector < double > > data;
   bool                         is_csv;

   QString info() 
      {
         return QString( "US_Multi_Column filename %1: header %2 rows %3 columns %4\n" )
            .arg( filename )
            .arg( header.size() )
            .arg( data.size() )
            .arg( data.size() > 0 ? data[ 0 ].size() : 0 );
      }

   QString cinfo() 
      {
         QString qs;
         for ( int i = 0; i < (int) header.size(); ++i )
         {
            double sum = 0e0;
            double min = 0e0;
            double max = 0e0;
            double avg = 0e0;
            if ( data.size() )
            {
               for ( int j = 0; j < (int) data.size(); ++j )
               {
                  double tmp = data[ j ][ i ];
                  sum += tmp;
                  if ( !j )
                  {
                     min = tmp;
                     max = tmp;
                  } else {
                     if ( min > tmp )
                     {
                        min = tmp;
                     }
                     if ( max < tmp )
                     {
                        max = tmp;
                     }
                  }
               }
               avg = sum / (double) data.size();
               qs += QString( "column %1: '%2' min %3 max %4 avg %5 sum %6\n" )
                  .arg( i + 1 )
                  .arg( header[ i ] )
                  .arg( min )
                  .arg( max )
                  .arg( avg )
                  .arg( sum )
                  ;                  
            } else {
               qs += QString( "column %1: '%2' (data empty)\n" )
                  .arg( i + 1 )
                  .arg( header[ i ] )
                  ;
            }
         }
         return qs;
      }

   void    clear( ) 
      {
         filename = "";
         is_csv   = false;
         header   .clear( );
         data     .clear( );
      }

   QString errormsg;

   bool    read()
      {
         errormsg  = "";
         if ( filename.isEmpty() )
         {
            errormsg = QString( "Error: filename is empty!" );
            return false;
         }
            
         QFile f( filename );
         if ( !f.exists() )
         {
            errormsg = QString( "Error: file %1 does not exist" ).arg( filename );
            return false;
         }
         if ( !f.open( QIODevice::ReadOnly | QIODevice::Text ) )
         {
            errormsg = QString( "Error: file %1 can not be opened" ).arg( filename );
            return false;
         }
         QTextStream ts( &f );
         unsigned int pos = 0;
         while ( !ts.atEnd() )
         {
            QString     qs  = ts.readLine();
            QStringList qsl = qs.split( QRegularExpression( is_csv ? "," : "\\s+" ), Qt::SkipEmptyParts );
            if ( !pos &&
                 qsl.size() &&
                 qsl[ 0 ].contains( QRegularExpression( QStringLiteral( "^\\D" ) ) ) )
            {
               // take as header;
               for ( unsigned int i = 0; i < (unsigned int) qsl.size(); i++ )
               {
                  header.push_back( qsl[ i ] );
               }
               continue;
            }
            vector < double > tmp_data;

            for ( unsigned int i = 0; i < (unsigned int) qsl.size(); i++ )
            {
               tmp_data.push_back( qsl[ i ].trimmed().toDouble() );
            }
            data.push_back( tmp_data );
         }
         f.close();
         return true;
      }      

   bool    split_on_peak( US_Multi_Column &asc, 
                          US_Multi_Column &des,
                          unsigned int    indep,
                          unsigned int    dep )
      {
         errormsg = "";
         if ( !data.size() )
         {
            errormsg = QString( "Error: %1 has no data" ).arg( filename );
            return false;
         }

         if ( data.size() < 3 )
         {
            errormsg = QString( "Error: %1 has only %2 rows and it needs at least 3" )
               .arg( filename )
               .arg( data.size() )
               ;
            return false;
         }

         if ( data[ 0 ].size() < indep || data[ 0 ].size() < dep )
         {
            errormsg = QString( "Error: %1 has %2 columns and columns %3 %4 requested  " )
               .arg( filename )
               .arg( data[ 0 ].size() )
               .arg( indep )
               .arg( dep )
               ;
            return false;
         }

         // offset to zero base
         dep--;
         indep--;

         double       peak     = data[ 0 ][ dep ];
         unsigned int peak_pos = 0;
         
         for ( unsigned int i = 1; i < data.size(); i++ )
         {
            if ( peak < data[ i ][ dep ] )
            {
               peak     = data[ i ][ dep ];
               peak_pos = i;
            }
         }

         if ( peak_pos == 0 ||
              peak_pos == data.size() - 1 )
         {
            errormsg = QString( "Error: %1 peak position (%2, value %3) occured at an end" )
               .arg( filename )
               .arg( peak_pos )
               .arg( peak )
               ;
            return false;
         }
            
         asc.clear( );
         des.clear( );
         asc.filename = filename + "-asc";
         asc.header   = header;
         des.filename = filename + "-des";
         des.header   = header;

         for ( unsigned int i = 0; i < data.size(); i++ )
         {
            if ( i <= peak_pos )
            {
               asc.data.push_back( data[ i ] );
            }
            if ( i >= peak_pos )
            {
               des.data.push_back( data[ i ] );
            }
         }
         return true;
      }

   bool monotonize( US_Multi_Column &mono, unsigned int dep )
      {
         errormsg = "";
         if ( !data.size() )
         {
            errormsg = QString( "Error: %1 has no data" ).arg( filename );
            return false;
         }

         if ( data[ 0 ].size() < dep )
         {
            errormsg = QString( "Error: %1 has %2 columns and column %3 requested  " )
               .arg( filename )
               .arg( data[ 0 ].size() )
               .arg( dep )
               ;
            return false;
         }

         // offset to zero base
         dep--;

         bool ascending = ( data[ 0 ][ dep ] < data.back()[ dep ] );
         // cout << QString( "curve %1 is %2\n" ).arg( filename ).arg( ascending ? "ascending" : "descending" );

         mono.clear( );
         mono.filename = filename + "-mono";
         mono.header   = header;
         mono.data.push_back( data[ 0 ] );

         if ( ascending )
         {
            for ( unsigned int i = 1; i < data.size(); i++ )
            {
               if ( mono.data.back()[ dep ] < data[ i ][ dep ] )
               {
                  mono.data.push_back( data[ i ] );
               }
            }
         } else {
            for ( unsigned int i = 1; i < data.size(); i++ )
            {
               if ( mono.data.back()[ dep ] > data[ i ][ dep ] )
               {
                  mono.data.push_back( data[ i ] );
               }
            }
         }
         return true;
      }

   bool write( QString use_filename = "", bool overwrite = false )
      {
         errormsg  = "";
         if ( use_filename.isEmpty() )
         {
            use_filename = filename;
         }
         if ( use_filename.isEmpty() )
         {
            errormsg = QString( "Error: filename is empty!" );
            return false;
         }

         bool is_write_csv = use_filename.right( 4 ).contains( ".csv", Qt::CaseInsensitive );

         QFile f( use_filename );
         if ( !overwrite && f.exists() )
         {
            errormsg = QString( "Error: file %1 exists" ).arg( use_filename );
            return false;
         }
         if ( !f.open( QIODevice::WriteOnly | QIODevice::Text ) )
         {
            errormsg = QString( "Error: file %1 can not be opened for writing" ).arg( use_filename );
            return false;
         }

         QTextStream ts( &f );
         if ( header.size() )
         {
            for ( unsigned int i = 0; i < header.size(); i++ )
            {
               if ( i )
               {
                  ts << ( is_write_csv ? "," : "\t" );
               }
               ts << header[ i ];
            }
            ts << Qt::endl;
         }

         for ( unsigned int i = 0; i < data.size(); i++ )
         {
            for ( unsigned int j = 0; j < data[ i ].size(); j++ )
            {
               if ( j )
               {
                  ts << ( is_write_csv ? "," : "\t" );
               }
               ts << data[ i ][ j ];
            }
            ts << Qt::endl;
         }
         f.close();
         return true;
      }         

   bool avg( vector < double > &result,
             vector < double > &a, 
             vector < double > &b )
      {
         errormsg = "";
         if ( a.size() != b.size() )
         {
            errormsg = QString( "Error: avg size incompatibility %1 vs %2" )
               .arg( a.size() )
               .arg( b.size() );
            return false;
         }
         result.resize( a.size() );
         for ( unsigned int i = 0; i < a.size(); i++ )
         {
            result[ i ] = 0.5 * ( a[ i ] + b[ i ] );
         }
         return true;
      }

   bool join( US_Multi_Column &part1, 
              US_Multi_Column &part2,  
              QString use_filename = "", 
              unsigned int overlap = 1 )
      {
         errormsg  = "";
         if ( overlap > part1.data.size() ||
              overlap > part2.data.size() )
         {
            errormsg = QString( "Error: overlap %1 is greater than row sizes ( %2 and %3 )" )
               .arg( overlap )
               .arg( part1.data.size() )
               .arg( part2.data.size() )
               ;
            return false;
         }
            
         clear( );

         if ( use_filename.isEmpty() )
         {
            filename = part1.filename + "-" + part2.filename + "-join";
         } else {
            filename = use_filename;
         }
         header = part1.header;
         for ( unsigned int i = 0; i < part1.data.size(); i++ )
         {
            data.push_back( part1.data[ i ] );
         }

         unsigned int offset = data.size() - overlap;
         for ( unsigned int i = 0; i < part2.data.size() && i < overlap; i++ )
         {
            vector < double > result;
            if ( !avg( result, part1.data[ offset + i ], part2.data[ i ] ) )
            {
               return false;
            }
            data[ offset + i ] = result;
         }

         for ( unsigned int i = overlap; i < part2.data.size(); i++ )
         {
            data.push_back( part2.data[ i ] );
         }
         return true;
      }

   // part2, dep2 is monotonic
   // make spline dep2->indep2
   // compute values for indep2( dep1 ) and append with header to part1
   bool spline( US_Multi_Column &part1, 
                US_Multi_Column &part2,
                unsigned int    dep1,
                unsigned int    indep2,
                unsigned int    dep2,
                QString         use_filename )
      {
         errormsg = "";
         if ( !part1.data.size() )
         {
            errormsg = QString( "Error: %1 has no data" ).arg( part1.filename );
            return false;
         }
         if ( !part2.data.size() )
         {
            errormsg = QString( "Error: %1 has no data" ).arg( part2.filename );
            return false;
         }

         if ( part1.data[ 0 ].size() < dep1 )
         {
            errormsg = QString( "Error: %1 has %2 columns and column %3 requested" )
               .arg( part1.filename )
               .arg( part1.data[ 0 ].size() )
               .arg( dep1 )
               ;
            return false;
         }
         
         if ( part2.data[ 0 ].size() < dep2 || part2.data[ 0 ].size() < indep2 )
         {
            errormsg = QString( "Error: %1 has %2 columns and columns %3 %4 requested" )
               .arg( part2.filename )
               .arg( part2.data[ 0 ].size() )
               .arg( dep2 )
               .arg( indep2 )
               ;
            return false;
         }

         // base is 0
         dep1--;
         indep2--;
         dep2--;

         // setup this
         if ( use_filename.isEmpty() )
         {
            filename = part1.filename + "-" + part2.filename + "-splined";
         } else {
            filename = use_filename;
         }
         header = part1.header;
         data   = part1.data;
         if ( header.size() && part2.header.size() > indep2 )
         {
            header.push_back( part2.header[ indep2 ] );
         }

                                 double part1_min_dep = part1.data[ 0 ]  [ dep1 ];
                                 double part1_max_dep = part1.data.back()[ dep1 ];
                                 double part2_min_dep = part2.data[ 0 ]  [ dep2 ];
                                 double part2_max_dep = part2.data.back()[ dep2 ];

                                 bool ascending = ( part1_min_dep < part1_max_dep );
                                 if ( !ascending &&
                                      part2_min_dep < part2_max_dep )
         {
            errormsg = QString( "Error: range reversal over dependent range: %1 [%2:%3] vs %4 [%5:%6]" )
               .arg( part1.filename            )
               .arg( part1.data[ 0 ]  [ dep1 ] )
               .arg( part1.data.back()[ dep1 ] )
               .arg( part2.filename            )
               .arg( part2.data[ 0 ]  [ dep2 ] )
               .arg( part2.data.back()[ dep2 ] )
               ;
            return false;
         }

         if ( !ascending )
         {
            double tmp    = part1_min_dep;
            part1_min_dep = part1_max_dep;
            part1_max_dep = tmp;
            
            tmp           = part2_min_dep;
            part2_min_dep = part2_max_dep;
            part2_max_dep = tmp;
         }

         // setup natural spline
         US_Saxs_Util usu;

         vector < double > x;
         vector < double > y;
         vector < double > y2;
         if ( ascending )
         {
            for ( unsigned int i = 0; i < part2.data.size(); i++ )
            {
               x.push_back( part2.data[ i ][ dep2   ] );
               y.push_back( part2.data[ i ][ indep2 ] );
            }

         } else {
            for ( int i = part2.data.size() - 1; i >= 0;  i-- )
            {
               x.push_back( part2.data[ i ][ dep2   ] );
               y.push_back( part2.data[ i ][ indep2 ] );
            }
         }

         // unsigned int n = x.size();
         // attempt at setting slopes failed
         // double slope1 = ( y[ 1 ] - y[ 0 ] ) / ( x[ 1 ] - x[ 0 ] );
         // double slope2 = ( y[ n - 1 ] - y[ n - 2 ] ) / ( x[ n - 1 ] - x[ n - 2 ] );
         // cout << QString( "slopes for spline %1 %2\n" ).arg( slope1 ).arg( slope2 );

         usu.natural_spline( x, y, y2 );
         // ( y[ 1 ] - y[ 0 ] ) / ( x[ 1 ] - x[ 0 ] ),  
         // ( y[ n - 1 ] - y[ n - 2 ] ) / ( x[ n - 1 ] - x[ n - 2 ] )
         // );

         // debug stuff -----------
         US_Multi_Column mcx;
         mcx.filename = filename + "-xy";
         for ( unsigned int i = 0; i < x.size(); i++ )
         {
            vector < double > tmp_data;
            tmp_data.push_back( x[ i ] );
            tmp_data.push_back( y[ i ] );
            mcx.data.push_back( tmp_data );
         }
         mcx.write( "", true );
         
         mcx.clear( );
         mcx.filename = filename + "-xyspline";
         for ( double pos = part2_min_dep; pos <= part2_max_dep; pos += ( part2_max_dep - part2_min_dep ) / 256 )
         {
            vector < double > tmp_data;
            double new_val;
            tmp_data.push_back( pos );
            if ( !usu.apply_natural_spline( x, y, y2, pos, new_val ) )
            {
               us_qdebug( usu.errormsg );
            }
            tmp_data.push_back( new_val );
            mcx.data.push_back( tmp_data );
         }
         mcx.write( "", true );

         // end debug stuff -----------

         for ( unsigned int i = 0; i < (unsigned int) data.size(); i++ )
         {
            double new_val = 0e0;
            if ( data[ i ][ dep1 ] >= part2_min_dep &&
                 data[ i ][ dep1 ] <= part2_max_dep )
            {
               if ( !usu.apply_natural_spline( x, y, y2, data[ i ][ dep1 ], new_val ) )
               {
                  errormsg = usu.errormsg;
                  return false;
               }
               // cout << QString( "spline: compute point dest value %1, new value %2\n" )
               // .arg( data[ i ][ dep1 ] ).arg( new_val );
            }
            data[ i ].push_back( new_val );
         }

         return true;
      }

   // part2, dep2 is monotonic
   // make spline dep2->indep2
   // compute values for indep2( dep1 ) and append with header to part1
   bool quadratic( US_Multi_Column &part1, 
                US_Multi_Column &part2,
                unsigned int    dep1,
                unsigned int    indep2,
                unsigned int    dep2,
                QString         use_filename )
      {
         errormsg = "";
         if ( !part1.data.size() )
         {
            errormsg = QString( "Error: %1 has no data" ).arg( part1.filename );
            return false;
         }
         if ( !part2.data.size() )
         {
            errormsg = QString( "Error: %1 has no data" ).arg( part2.filename );
            return false;
         }

         if ( part1.data[ 0 ].size() < dep1 )
         {
            errormsg = QString( "Error: %1 has %2 columns and column %3 requested" )
               .arg( part1.filename )
               .arg( part1.data[ 0 ].size() )
               .arg( dep1 )
               ;
            return false;
         }
         
         if ( part2.data[ 0 ].size() < dep2 || part2.data[ 0 ].size() < indep2 )
         {
            errormsg = QString( "Error: %1 has %2 columns and columns %3 %4 requested" )
               .arg( part2.filename )
               .arg( part2.data[ 0 ].size() )
               .arg( dep2 )
               .arg( indep2 )
               ;
            return false;
         }

         // base is 0
         dep1--;
         indep2--;
         dep2--;

         // setup this
         if ( use_filename.isEmpty() )
         {
            filename = part1.filename + "-" + part2.filename + "-quadraticd";
         } else {
            filename = use_filename;
         }
         header = part1.header;
         data   = part1.data;
         if ( header.size() && part2.header.size() > indep2 )
         {
            header.push_back( part2.header[ indep2 ] );
         }

                                 double part1_min_dep = part1.data[ 0 ]  [ dep1 ];
                                 double part1_max_dep = part1.data.back()[ dep1 ];
                                 double part2_min_dep = part2.data[ 0 ]  [ dep2 ];
                                 double part2_max_dep = part2.data.back()[ dep2 ];

                                 bool ascending = ( part1_min_dep < part1_max_dep );
                                 if ( !ascending &&
                                      part2_min_dep < part2_max_dep )
         {
            errormsg = QString( "Error: range reversal over dependent range: %1 [%2:%3] vs %4 [%5:%6]" )
               .arg( part1.filename            )
               .arg( part1.data[ 0 ]  [ dep1 ] )
               .arg( part1.data.back()[ dep1 ] )
               .arg( part2.filename            )
               .arg( part2.data[ 0 ]  [ dep2 ] )
               .arg( part2.data.back()[ dep2 ] )
               ;
            return false;
         }

         if ( !ascending )
         {
            double tmp    = part1_min_dep;
            part1_min_dep = part1_max_dep;
            part1_max_dep = tmp;
            
            tmp           = part2_min_dep;
            part2_min_dep = part2_max_dep;
            part2_max_dep = tmp;
         }

         // setup natural quadratic
         US_Saxs_Util usu;

         vector < double > x1;
         vector < double > y1;
         vector < double > x2;
         vector < double > y2;
         if ( ascending )
         {
            for ( unsigned int i = 0; i < part2.data.size(); i++ )
            {
               x1.push_back( part2.data[ i ][ dep2   ] );
               y1.push_back( part2.data[ i ][ indep2 ] );
            }

         } else {
            for ( int i = part2.data.size() - 1; i >= 0;  i-- )
            {
               x1.push_back( part2.data[ i ][ dep2   ] );
               y1.push_back( part2.data[ i ][ indep2 ] );
            }
         }

         // debug stuff -----------
         US_Multi_Column mcx;
         mcx.filename = filename + "-qxy";
         for ( unsigned int i = 0; i < x1.size(); i++ )
         {
            vector < double > tmp_data;
            tmp_data.push_back( x1[ i ] );
            tmp_data.push_back( y1[ i ] );
            mcx.data.push_back( tmp_data );
         }
         mcx.write( "", true );
         
         for ( double pos = part2_min_dep; pos <= part2_max_dep; pos += ( part2_max_dep - part2_min_dep ) / 256 )
         {
            x2.push_back( pos );
         }

         if ( !usu.quadratic_interpolate( x1, y1, x2, y2 ) )
         {
            errormsg = usu.errormsg;
            return false;
         }

         mcx.clear( );
         mcx.filename = filename + "-qxyquadratic";
         for ( unsigned int i = 0; i < x2.size(); i++ )
         {
            vector < double > tmp_data;
            tmp_data.push_back( x2[ i ] );
            tmp_data.push_back( y2[ i ] );
            mcx.data.push_back( tmp_data );
         }
         mcx.write( "", true );

         x2.clear( );

         // end debug stuff -----------
              
         for ( unsigned int i = 0; i < data.size(); i++ )
         {
            if ( data[ i ][ dep1 ] >= part2_min_dep &&
                 data[ i ][ dep1 ] <= part2_max_dep )
            {
               x2.push_back( data[ i ][ dep1 ] );
            }
         }

         if ( !usu.quadratic_interpolate( x1, y1, x2, y2 ) )
         {
            errormsg = usu.errormsg;
            return false;
         }

         unsigned int pos = 0;
         for ( unsigned int i = 0; i < data.size(); i++ )
         {
            double new_val = 0e0;
            if ( data[ i ][ dep1 ] >= part2_min_dep &&
                 data[ i ][ dep1 ] <= part2_max_dep )
            {
               new_val = y2[ pos++ ];
            }
            data[ i ].push_back( new_val );
         }

         return true;
      }

   // part2, dep2 is monotonic
   // make spline dep2->indep2
   // compute values for indep2( dep1 ) and append with header to part1
   bool linear( US_Multi_Column &part1, 
                US_Multi_Column &part2,
                unsigned int    dep1,
                unsigned int    indep2,
                unsigned int    dep2,
                QString         use_filename )
      {
         errormsg = "";
         if ( !part1.data.size() )
         {
            errormsg = QString( "Error: %1 has no data" ).arg( part1.filename );
            return false;
         }
         if ( !part2.data.size() )
         {
            errormsg = QString( "Error: %1 has no data" ).arg( part2.filename );
            return false;
         }

         if ( part1.data[ 0 ].size() < dep1 )
         {
            errormsg = QString( "Error: %1 has %2 columns and column %3 requested" )
               .arg( part1.filename )
               .arg( part1.data[ 0 ].size() )
               .arg( dep1 )
               ;
            return false;
         }
         
         if ( part2.data[ 0 ].size() < dep2 || part2.data[ 0 ].size() < indep2 )
         {
            errormsg = QString( "Error: %1 has %2 columns and columns %3 %4 requested" )
               .arg( part2.filename )
               .arg( part2.data[ 0 ].size() )
               .arg( dep2 )
               .arg( indep2 )
               ;
            return false;
         }

         // base is 0
         dep1--;
         indep2--;
         dep2--;

         // setup this
         if ( use_filename.isEmpty() )
         {
            filename = part1.filename + "-" + part2.filename + "-linear";
         } else {
            filename = use_filename;
         }
         header = part1.header;
         data   = part1.data;
         if ( header.size() && part2.header.size() > indep2 )
         {
            header.push_back( part2.header[ indep2 ] );
         }

                                 double part1_min_dep = part1.data[ 0 ]  [ dep1 ];
                                 double part1_max_dep = part1.data.back()[ dep1 ];
                                 double part2_min_dep = part2.data[ 0 ]  [ dep2 ];
                                 double part2_max_dep = part2.data.back()[ dep2 ];

                                 bool ascending = ( part1_min_dep < part1_max_dep );
                                 if ( !ascending &&
                                      part2_min_dep < part2_max_dep )
         {
            errormsg = QString( "Error: range reversal over dependent range: %1 [%2:%3] vs %4 [%5:%6]" )
               .arg( part1.filename            )
               .arg( part1.data[ 0 ]  [ dep1 ] )
               .arg( part1.data.back()[ dep1 ] )
               .arg( part2.filename            )
               .arg( part2.data[ 0 ]  [ dep2 ] )
               .arg( part2.data.back()[ dep2 ] )
               ;
            return false;
         }

         if ( !ascending )
         {
            double tmp    = part1_min_dep;
            part1_min_dep = part1_max_dep;
            part1_max_dep = tmp;
            
            tmp           = part2_min_dep;
            part2_min_dep = part2_max_dep;
            part2_max_dep = tmp;
         }

         // setup natural linear
         US_Saxs_Util usu;

         vector < double > x1;
         vector < double > y1;
         vector < double > x2;
         vector < double > y2;
         if ( ascending )
         {
            for ( unsigned int i = 0; i < part2.data.size(); i++ )
            {
               x1.push_back( part2.data[ i ][ dep2   ] );
               y1.push_back( part2.data[ i ][ indep2 ] );
            }

         } else {
            for ( int i = part2.data.size() - 1; i >= 0;  i-- )
            {
               x1.push_back( part2.data[ i ][ dep2   ] );
               y1.push_back( part2.data[ i ][ indep2 ] );
            }
         }

         // debug stuff -----------
         US_Multi_Column mcx;
         mcx.filename = filename + "-lxy";
         for ( unsigned int i = 0; i < x1.size(); i++ )
         {
            vector < double > tmp_data;
            tmp_data.push_back( x1[ i ] );
            tmp_data.push_back( y1[ i ] );
            mcx.data.push_back( tmp_data );
         }
         mcx.write( "", true );
         
         for ( double pos = part2_min_dep; pos <= part2_max_dep; pos += ( part2_max_dep - part2_min_dep ) / 256 )
         {
            x2.push_back( pos );
         }

         if ( !usu.linear_interpolate( x1, y1, x2, y2 ) )
         {
            errormsg = usu.errormsg;
            return false;
         }

         mcx.clear( );
         mcx.filename = filename + "-lxylinear";
         for ( unsigned int i = 0; i < x2.size(); i++ )
         {
            vector < double > tmp_data;
            tmp_data.push_back( x2[ i ] );
            tmp_data.push_back( y2[ i ] );
            mcx.data.push_back( tmp_data );
         }
         mcx.write( "", true );

         x2.clear( );

         // end debug stuff -----------
              
         for ( unsigned int i = 0; i < data.size(); i++ )
         {
            if ( data[ i ][ dep1 ] >= part2_min_dep &&
                 data[ i ][ dep1 ] <= part2_max_dep )
            {
               x2.push_back( data[ i ][ dep1 ] );
            }
         }

         if ( !x2.size() )
         {
            errormsg =
               QString( "In linear: part1 %1 part2 %2 dep1 %3 indep2 %4 dep2 %5 range min %6 max %7:  data in range" )
               .arg( part1.filename )
               .arg( part2.filename )
               .arg( dep1 )
               .arg( indep2 )
               .arg( dep2 )
               .arg( part2_min_dep )
               .arg( part2_max_dep )
               ;
            return false;
         }

         if ( !usu.linear_interpolate( x1, y1, x2, y2 ) )
         {
            errormsg = usu.errormsg;
            return false;
         }

         unsigned int pos = 0;
         for ( unsigned int i = 0; i < data.size(); i++ )
         {
            double new_val = 0e0;
            if ( data[ i ][ dep1 ] >= part2_min_dep &&
                 data[ i ][ dep1 ] <= part2_max_dep )
            {
               new_val = y2[ pos++ ];
            }
            data[ i ].push_back( new_val );
         }

         return true;
      }

   bool smooth( unsigned int col,
                unsigned int points,
                bool         smooth_endpoints = false )
      {
         errormsg = "";
         if ( !points )
         {
            errormsg = "Error: smooth: number of points must be non zero";
            return false;
         }

         if ( !col )
         {
            errormsg = QString( "Error: smooth: %1, col must be nonzero" ).arg( filename );
            return false;
         }

         if ( !data.size() )
         {
            errormsg = QString( "Error: smooth: %1 has no data" ).arg( filename );
            return false;
         }

         if ( data[ 0 ].size() < col )
         {
            errormsg = QString( "Error: smooth: %1 has %2 columns and column %3 requested" )
               .arg( filename )
               .arg( data[ 0 ].size() )
               .arg( col )
               ;
            return false;
         }

         col--;

         vector < double > y ( data.size() );
         vector < double > yn( data.size() );
         vector < double > dist = getnormal( points );
         
         for( unsigned int i = 0; i < data.size(); i++ )
         {
            y[ i ] = data[ i ][ col ];
         }
            
         for( unsigned int i = 0; i < data.size(); i++ )
         {
            yn[ i ]      = 0e0;
            double tdist = 0e0;
            for( int j = -( int ) points; j <= ( int ) points; j++ ) 
            {
               int jp = ( int ) i + j;
               if ( jp >= 0 && jp < (int) y.size() ) 
               {
                  int uj = j;
                  if ( j < 0 ) 
                  {
                     uj = -j;
                  }
                  yn[ i ] +=  y[ jp ] * dist[ uj ];
                  tdist += dist[ uj ];
               }
            }
            yn[ i ] /= tdist;
         }
         {
            unsigned int low  = 0;
            unsigned int high = (unsigned int) data.size();
            if ( !smooth_endpoints )
            {
               low++;
               high--;
            }
            for( unsigned int i = low; i < high; i++ )
            {
               data[ i ][ col ] = yn[ i ];
            }
         }
         return true;
      }      

   bool repeak( unsigned int col )
      {
         errormsg = "";
         if ( !col )
         {
            errormsg = QString( "Error: repeak: %1, col must be nonzero" ).arg( filename );
            return false;
         }

         if ( !data.size() )
         {
            errormsg = QString( "Error: repeak: %1 has no data" ).arg( filename );
            return false;
         }

         if ( (unsigned int) data[ 0 ].size() < col )
         {
            errormsg = QString( "Error: repeak: %1 has %2 columns and column %3 requested" )
               .arg( filename )
               .arg( data[ 0 ].size() )
               .arg( col )
               ;
            return false;
         }

         col--;

         double max = data[ 0 ][ col ];
         
         for( unsigned int i = 1; i < (unsigned int) data.size(); i++ )
         {
            if ( max < data[ i ][ col ] )
            {
               max = data[ i ][ col ];
            }
         }
            
         if ( max <= 0e0 )
         {
            errormsg = QString( "Error: repeak: %1 column %2 max is %3, but must be greater than zero" )
               .arg( filename )
               .arg( col + 1 )
               .arg( max )
               ;
            return false;
         }

         for( unsigned int i = 0; i < (unsigned int) data.size(); i++ )
         {
            data[ i ][ col ] /= max;
         }
         return true;
      }      

   bool range( unsigned int col, double min, double max )
      {
         errormsg = "";
         if ( !col )
         {
            errormsg = QString( "Error: range: %1, col must be nonzero" ).arg( filename );
            return false;
         }

         if ( !data.size() )
         {
            errormsg = QString( "Error: range: %1 has no data" ).arg( filename );
            return false;
         }

         if ( (unsigned int) data[ 0 ].size() < col )
         {
            errormsg = QString( "Error: range: %1 has %2 columns and column %3 requested" )
               .arg( filename )
               .arg( data[ 0 ].size() )
               .arg( col )
               ;
            return false;
         }

         col--;

         vector < vector < double > > new_data;

         for( unsigned int i = 0; i < (unsigned int) data.size(); i++ )
         {
            if ( data[ i ][ col ] >= min && 
                 data[ i ][ col ] <= max )
            {
               new_data.push_back( data[ i ] );
            }
         }
            
         data = new_data;
         return true;
      }      

   bool scale( unsigned int col, double mult )
      {
         errormsg = "";
         if ( !col )
         {
            errormsg = QString( "Error: scale: %1, col must be nonzero" ).arg( filename );
            return false;
         }

         if ( !data.size() )
         {
            errormsg = QString( "Error: scale: %1 has no data" ).arg( filename );
            return false;
         }

         if ( (unsigned int) data[ 0 ].size() < col )
         {
            errormsg = QString( "Error: scale: %1 has %2 columns and column %3 requested" )
               .arg( filename )
               .arg( data[ 0 ].size() )
               .arg( col )
               ;
            return false;
         }

         col--;

         for( unsigned int i = 0; i < (unsigned int) data.size(); i++ )
         {
            data[ i ][ col ] *= mult;
         }

         us_qdebug( QString( "scale %1" ).arg( mult ) );
            
         return true;
      }      


   bool clip( unsigned int col, double min, double max )
      {
         errormsg = "";
         if ( !col )
         {
            errormsg = QString( "Error: clip: %1, col must be nonzero" ).arg( filename );
            return false;
         }

         if ( !data.size() )
         {
            errormsg = QString( "Error: clip: %1 has no data" ).arg( filename );
            return false;
         }

         if ( (unsigned int) data[ 0 ].size() < col )
         {
            errormsg = QString( "Error: clip: %1 has %2 columns and column %3 requested" )
               .arg( filename )
               .arg( data[ 0 ].size() )
               .arg( col )
               ;
            return false;
         }

         col--;

         vector < vector < double > > new_data;

         for( unsigned int i = 0; i < (unsigned int) data.size(); i++ )
         {
            if ( data[ i ][ col ] > min && 
                 data[ i ][ col ] < max )
            {
               new_data.push_back( data[ i ] );
            }
         }
            
         data = new_data;
         return true;
      }      

   bool ijoin( US_Multi_Column & mc1,
               US_Multi_Column & mc2,
               unsigned int      col1,
               unsigned int      col2,
               unsigned int      col2add )
      {
         errormsg = "";
         if ( !col1 || !col2 || !col2add )
         {
            errormsg = QString( "Error: ijoin: %1, cols must be nonzero" ).arg( filename );
            return false;
         }

         if ( !mc1.data.size() )
         {
            errormsg = QString( "Error: ijoin: %1 has no data" ).arg( mc1.filename );
            return false;
         }

         if ( !mc2.data.size() )
         {
            errormsg = QString( "Error: ijoin: %1 has no data" ).arg( mc2.filename );
            return false;
         }

         if ( (unsigned int) mc1.data[ 0 ].size() < col1 )
         {
            errormsg = QString( "Error: ijoin: %1 has %2 columns and column %3 requested" )
               .arg( mc1.filename )
               .arg( mc1.data[ 0 ].size() )
               .arg( col1 )
               ;
            return false;
         }
         if ( (unsigned int) mc2.data[ 0 ].size() < col2 )
         {
            errormsg = QString( "Error: ijoin: %1 has %2 columns and column %3 requested" )
               .arg( mc2.filename )
               .arg( mc2.data[ 0 ].size() )
               .arg( col2 )
               ;
            return false;
         }
         if ( (unsigned int) mc2.data[ 0 ].size() < col2add )
         {
            errormsg = QString( "Error: ijoin: %1 has %2 columns and column %3 requested" )
               .arg( mc2.filename )
               .arg( mc2.data[ 0 ].size() )
               .arg( col2add )
               ;
            return false;
         }
         if ( col2 == col2add )
         {
            errormsg = QString( "Error: ijoin: %1 col2 %2 and col2add %3 can not be identical" )
               .arg( mc2.filename )
               .arg( col2 )
               .arg( col2add )
               ;
            return false;
         }

         col1--;
         col2--;
         col2add--;

         // interpolate col2 w/col2add to col1 (crop zeros?)
         US_Saxs_Util usu;

         vector < double > new_data;
         vector < double > to_grid;
         vector < double > from_grid;
         vector < double > from_data;

         for( unsigned int i = 0; i < (unsigned int) mc1.data.size(); i++ )
         {
            to_grid.push_back( mc1.data[ i ][ col1 ] );
         }

         for( unsigned int i = 0; i < (unsigned int) mc2.data.size(); i++ )
         {
            from_grid.push_back( mc2.data[ i ][ col2    ] );
            from_data.push_back( mc2.data[ i ][ col2add ] );
         }

         // US_Vector::printvector3( "to grid, from grid, from data", to_grid, from_grid, from_data );

         if ( !usu.interpolate( new_data, to_grid, from_grid, from_data ) )
         {
            errormsg = usu.errormsg;
            return false;
         }

         // US_Vector::printvector2( "to grid, new_data", to_grid, new_data );

         header = mc1.header;
         header .push_back( mc2.header[ col2add ] );
         data   = mc1.data;
         for( unsigned int i = 0; i < (unsigned int) data.size(); i++ )
         {
            data[ i ].push_back( new_data[ i ] );
         }

         return true;
      }      


   bool sort( unsigned int col )
      {
         errormsg = "";
         if ( !col )
         {
            errormsg = QString( "Error: sort: %1, col must be nonzero" ).arg( filename );
            return false;
         }

         if ( !data.size() )
         {
            errormsg = QString( "Error: sort: %1 has no data" ).arg( filename );
            return false;
         }

         if ( (unsigned int) data[ 0 ].size() < col )
         {
            errormsg = QString( "Error: sort: %1 has %2 columns and column %3 requested" )
               .arg( filename )
               .arg( data[ 0 ].size() )
               .arg( col )
               ;
            return false;
         }

         col--;

         list < umc_sortable_double > lumcsd;

         for ( unsigned int i = 0; i < (unsigned int) data.size(); i++ )
         {
            umc_sortable_double umcsd;
            umcsd.index = i;
            umcsd.d     = data[ i ][ col ];
            lumcsd.push_back( umcsd );
         }
         lumcsd.sort();

         vector < vector < double > > new_data;

         for ( list < umc_sortable_double >::iterator it = lumcsd.begin();
               it != lumcsd.end();
               it++ )
         {
            new_data.push_back( data[ it->index ] );
         }
         data = new_data;
         return true;
      }

   bool reverse()
      {
         errormsg = "";

         if ( !data.size() )
         {
            errormsg = QString( "Error: reverse: %1 has no data" ).arg( filename );
            return false;
         }

         vector < vector < double > > new_data;
         
         for( int i = (int) data.size() - 1; i >= 0; i-- )
         {
            new_data.push_back( data[ i ] );
         }
         data = new_data;
         return true;
      }      

   bool uniquify( unsigned int col,
                  unsigned int sumcol )
      {
         errormsg = "";
         if ( !col )
         {
            errormsg = QString( "Error: uniquify: %1, col must be nonzero" ).arg( filename );
            return false;
         }

         if ( !data.size() )
         {
            errormsg = QString( "Error: uniquify: %1 has no data" ).arg( filename );
            return false;
         }

         if ( (unsigned int) data[ 0 ].size() < col ||
              (unsigned int) data[ 0 ].size() < sumcol )
         {
            errormsg = QString( "Error: uniquify: %1 has %2 columns and columns %3 and %4 requested" )
               .arg( filename )
               .arg( data[ 0 ].size() )
               .arg( col )
               .arg( sumcol )
               ;
            return false;
         }

         col--;
         sumcol--;

         vector < vector < double > > new_data;

         map < double, unsigned int > ref_map;
         
         unsigned int tot = 0;
         unsigned int rows = 0;

         for( unsigned int i = 0; i < (unsigned int) data.size(); i++ )
         {
            double ref = data[ i ][ col ];
            tot++;
            if ( ref_map.count( ref ) )
            {
               new_data[ ref_map[ ref ] ][ sumcol ] += data[ i ][ sumcol ];
            } else {
               rows++;
               ref_map[ ref ] = new_data.size();
               new_data.push_back( data[ i ] );
            }
         }                 
         data = new_data;
         us_qdebug( QString( "rows %1 from %2" ).arg( rows ).arg( tot ) );
         return true;
      }      

 private:
   double normal( double x )
      {
         return exp( -( x * x ) ) / sqrt( 2e0 * 3.14159265358978323e0 );
      }

   vector < double > getnormal( unsigned int points )
      {
         vector < double > result;
         if ( !points )
         {
            us_qdebug( "Internal error: getnormal called with zero points" );
            return result;
         }
         points++;
         for ( unsigned int i = 0; i < points; i++ ) 
         {
            result.push_back( normal( ( 2e0 / points ) * ( double )i ) );
         }
         double tot = result[ 0 ];
         for ( unsigned int j = 1; j < (unsigned int) result.size(); j++ ) 
         {
            tot += 2 * result[ j ];
         }
         for ( unsigned int j = 1; j < (unsigned int) result.size(); j++ ) 
         {
            result[ j ] /= tot;
         }
         return result;
      }

};

#ifdef WIN32
# if QT_VERSION < 0x040000
  #pragma warning ( default: 4251 )
# endif
#endif

#endif
