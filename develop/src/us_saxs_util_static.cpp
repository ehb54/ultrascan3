#include "../include/us_saxs_util.h"

bool US_Saxs_Util::read_sas_data( 
                                 QString             filename,
                                 vector < double > & q,
                                 vector < double > & I,
                                 vector < double > & e,
                                 QString           & error_msg 
                                 )
{
   error_msg = "";
   QFile f( filename );
   if ( !f.exists() )
   {
      error_msg = QString("Error: %1 does not exist").arg( filename );
      return false;
   }
   QString ext = QFileInfo( filename ).extension( false ).lower();

   QRegExp rx_valid_ext (
                         "^("
                         "dat|"
                         "int|"
                         "txt|"
                         // "out|"
                         "ssaxs)$" );

   if ( rx_valid_ext.search( ext ) == -1 )
   {
      error_msg = QString("Error: %1 unsupported file extension %2").arg( filename ).arg( ext );
      return false;
   }
      
   if ( !f.open( IO_ReadOnly ) )
   {
      error_msg = QString("Error: can not open %1, check permissions ").arg( filename );
      return false;
   }

   QTextStream ts(&f);
   vector < QString > qv;
   QStringList qsl;

   while ( !ts.atEnd() )
   {
      QString qs = ts.readLine();
      qv.push_back( qs );
      qsl << qs;
   }
   f.close();

   if ( !qv.size() )
   {
      error_msg = QString("Error: the file %1 is empty ").arg( filename );
      return false;
   }

   // we should make some configuration for matches & offsets or column mapping
   // just an ad-hoc fix for APS 5IDD
   int offset = 0;
   if ( ext == "txt" && qv[ 0 ].contains( "# File Encoding (File origin in Excel)" ) )
   {
      offset = 1;
   }      

   QRegExp rx_ok_line("^(\\s+|\\d+|\\.|\\d(E|e)(\\+|-|\\d))+$");
   rx_ok_line.setMinimal( true );
   for ( int i = 1; i < (int) qv.size(); i++ )
   {
      if ( qv[i].contains(QRegExp("^#")) ||
           rx_ok_line.search( qv[i] ) == -1 )
      {
         continue;
      }
      
      QStringList tokens = QStringList::split(QRegExp("\\s+"), qv[i].replace(QRegExp("^\\s+"),""));

      if ( (int)tokens.size() > 1 + offset )
      {
         double this_q         = tokens[ 0 + offset ].toDouble();
         double this_I         = tokens[ 1 + offset ].toDouble();
         double this_e = 0e0;
         if ( (int)tokens.size() > 2 + offset)
         {
            this_e = tokens[ 2 + offset ].toDouble();
            if ( this_e < 0e0 )
            {
               this_e = 0e0;
            }
         }
         if ( q.size() && this_q <= q[ q.size() - 1 ] )
         {
            cout << QString(" breaking %1 %2\n").arg( this_q ).arg( q[ q.size() - 1 ] );
            break;
         }
         if ( this_I != 0e0 )
         {
            q       .push_back( this_q );
            I       .push_back( this_I );
            if ( (int)tokens.size() > 2 + offset && this_e )
            {
               e.push_back( this_e );
            }
         }
      }
   }

   if ( !q.size() )
   {
      error_msg = QString( "Error: File %1 has no data" ).arg( filename );
      return false;
   }

   if ( e.size() != q.size() )
   {
      e.clear();
   };

   return true;
}

void US_Saxs_Util::clip_data( 
                             double              minq,
                             double              maxq,
                             vector < double > & q,
                             vector < double > & I,
                             vector < double > & e
                             )
{
   vector < double >  q_new;
   vector < double >  I_new;
   vector < double >  e_new;
   for ( int i = 0; i < (int) q.size(); ++i )
   {
      if ( q[ i ] >= minq && q[ i ] <= maxq )
      {
         q_new.push_back( q[ i ] );
         I_new.push_back( I[ i ] );
         if ( e.size() )
         {
            e_new.push_back( e[ i ] );
         }
      }
   }
   
   q = q_new;
   I = I_new;
   e = e_new;
}

void US_Saxs_Util::bin_data( 
                            int                 /* bins */,
                            bool                /* log_bin */,
                            vector < double > & q,
                            vector < double > & I,
                            vector < double > & e
                            )
{
   vector < double >  q_new;
   vector < double >  I_new;
   vector < double >  e_new;

   // divide into bins and compute weighted average value or take sample point ?

   q = q_new;
   I = I_new;
   e = e_new;
}
