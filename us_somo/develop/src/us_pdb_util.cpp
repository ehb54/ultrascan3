#include "../include/us_pdb_util.h"

class pdb_sortable_qstring {
public:
   double       x;
   QString      name;
   bool operator < (const pdb_sortable_qstring& objIn) const
   {
      return x < objIn.x;
   }
};

QStringList US_Pdb_Util::sort_pdbs( const QStringList & filenames )
{
   if ( filenames.size() <= 1 )
   {
      return filenames;
   }

   bool reorder = true;

   list < pdb_sortable_qstring > svals;
   
   QString head = qstring_common_head( filenames, true );
   QString tail = qstring_common_tail( filenames, true );

   set < QString > used;

   QRegExp rx_cap( "(\\d+)_(\\d+)" );
   rx_cap.setMinimal( true );

   for ( int i = 0; i < (int) filenames.size(); ++i )
   {
      QString tmp = filenames[ i ].mid( head.length() );
      tmp = tmp.mid( 0, tmp.length() - tail.length() );
      if ( rx_cap.indexIn( tmp ) != -1 )
      {
         tmp = rx_cap.cap( 2 );
      }
      // cout << QString( "sort tmp <%1>\n" ).arg( tmp );

      if ( used.count( tmp ) )
      {
         reorder = false;
         break;
      }
      used.insert( tmp );

      pdb_sortable_qstring sval;
      sval.x     = tmp.toDouble();
      sval.name  = filenames[ i ];
      svals      .push_back( sval );
   }
   if ( !reorder )
   {
      return filenames;
   }

   svals.sort();
   
   QStringList sorted_filenames;
   for ( list < pdb_sortable_qstring >::iterator it = svals.begin();
         it != svals.end();
         ++it )
   {
      sorted_filenames << it->name;
   }
   return sorted_filenames;
}
      
QString US_Pdb_Util::qstring_common_head( const QStringList & qsl, bool strip_digits )
{
   if ( !qsl.size() )
   {
      return "";
   }
   if ( qsl.size() == 1 )
   {
      return qsl[ 0 ];
   }
   QString s = qsl[ 0 ];
   for ( int i = 1; i < (int)qsl.size(); i++ )
   {
      s = qstring_common_head( s, qsl[ i ] );
   }

   if ( strip_digits )
   {
      s.replace( QRegExp( "\\d+$" ), "" );
   }
   return s;
}

QString US_Pdb_Util::qstring_common_tail( const QStringList & qsl, bool strip_digits )
{
   if ( !qsl.size() )
   {
      return "";
   }
   if ( qsl.size() == 1 )
   {
      return qsl[ 0 ];
   }
   QString s = qsl[ 0 ];
   for ( int i = 1; i < (int)qsl.size(); i++ )
   {
      s = qstring_common_tail( s, qsl[ i ] );
   }
   if ( strip_digits )
   {
      s.replace( QRegExp( "^\\d+" ), "" );
   }
   return s;
}
      
QString US_Pdb_Util::qstring_common_head( const QString & s1, const QString & s2 )
{
   int min_len = (int)s1.length();
   if ( min_len > (int)s2.length() )
   {
      min_len = (int)s2.length();
   }

   // could do this more efficiently via "divide & conquer"
   // i.e. split the distance in halfs and compare 
   
   int match_max = 0;
   for ( int i = 1; i <= min_len; i++ )
   {
      match_max = i;
      if ( s1.left( i ) != s2.left( i ) )
      {
         match_max = i - 1;
         break;
      }
   }
   return s1.left( match_max );
}

QString US_Pdb_Util::qstring_common_tail( const QString & s1, const QString & s2 )
{
   int min_len = (int)s1.length();
   if ( min_len > (int)s2.length() )
   {
      min_len = (int)s2.length();
   }

   // could do this more efficiently via "divide & conquer"
   // i.e. split the distance in halfs and compare 
   
   int match_max = 0;
   for ( int i = 1; i <= min_len; i++ )
   {
      match_max = i;
      if ( s1.right( i ) != s2.right( i ) )
      {
         match_max = i - 1;
         break;
      }
   }
   return s1.right( match_max );
}

bool US_Pdb_Util::range_to_set( set < QString > & result, const QString & s )
{
   // us_qdebug( QString( "range_to_set '%1'" ).arg( s ) );

   QStringList qsl;
   {
      QRegExp rx = QRegExp( "\\s*(\\s|,|;)\\s*" );
      qsl = (s ).split( rx , Qt::SkipEmptyParts );
   }

   // us_qdebug( qsl.join("\n") + QString( "\n" ) );

   QRegExp rx_1( "(.?):(\\d+)" );
   QRegExp rx_2( "(.?):(\\d+)-(\\d+)" );

   result.clear( );

   for ( int i = 0; i < (int) qsl.size(); i++ )
   {
      if ( rx_2.indexIn( qsl[ i ] ) != -1 )
      {
         QString chain  = rx_2.cap( 1 );
         int     startr = rx_2.cap( 2 ).toInt();
         int     endr   = rx_2.cap( 3 ).toInt();
         if ( endr < startr )
         {
            return false;
         }
         for ( int j = startr; j <= endr; ++j )
         {
            result.insert( QString( "%1~%2" ).arg( chain ).arg( j ) );
         }
      } else {
         if ( rx_1.indexIn( qsl[ i ] ) != -1 )
         {
            QString chain  = rx_1.cap( 1 );
            int     r      = rx_1.cap( 2 ).toInt();
            result.insert( QString( "%1~%2" ).arg( chain ).arg( r ) );
         } else {
            return false;
         }
      }
   }
   return true;
}
