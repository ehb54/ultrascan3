#include "../include/us_pdb_util.h"
#include <QRegularExpression>

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

   QRegularExpression rx_cap( "(\\d+)_(\\d+)" );
   rx_cap.setPatternOptions(QRegularExpression::InvertedGreedinessOption);

   for ( int i = 0; i < (int) filenames.size(); ++i )
   {
      QString tmp = filenames[ i ].mid( head.length() );
      tmp = tmp.mid( 0, tmp.length() - tail.length() );
      QRegularExpressionMatch rx_cap_m = rx_cap.match( tmp );
      if ( rx_cap_m.hasMatch() )
      {
         tmp = rx_cap_m.captured(2);
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
      s.replace( QRegularExpression( QStringLiteral( "\\d+$" ) ), "" );
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
      s.replace( QRegularExpression( QStringLiteral( "^\\d+" ) ), "" );
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
      QRegularExpression rx = QRegularExpression( QStringLiteral( "\\s*(\\s|,|;)\\s*" ) );
      qsl = (s ).split( rx , Qt::SkipEmptyParts );
   }

   // us_qdebug( qsl.join("\n") + QString( "\n" ) );

   QRegularExpression rx_1( "(.?):(\\d+)" );
   QRegularExpression rx_2( "(.?):(\\d+)-(\\d+)" );

   result.clear( );

   for ( int i = 0; i < (int) qsl.size(); i++ )
   {
      QRegularExpressionMatch rx_2_m = rx_2.match( qsl[ i ] );
      if ( rx_2_m.hasMatch() )
      {
         QString chain  = rx_2_m.captured(1);
         int     startr = rx_2_m.captured(2).toInt();
         int     endr   = rx_2_m.captured(3).toInt();
         if ( endr < startr )
         {
            return false;
         }
         for ( int j = startr; j <= endr; ++j )
         {
            result.insert( QString( "%1~%2" ).arg( chain ).arg( j ) );
         }
      } else {
         QRegularExpressionMatch rx_1_m = rx_1.match( qsl[ i ] );
         if ( rx_1_m.hasMatch() )
         {
            QString chain  = rx_1_m.captured(1);
            int     r      = rx_1_m.captured(2).toInt();
            result.insert( QString( "%1~%2" ).arg( chain ).arg( r ) );
         } else {
            return false;
         }
      }
   }
   return true;
}
