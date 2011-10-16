#include "../include/us_saxs_util.h"

#if defined(WIN32)
#   include <dos.h>
#   include <stdlib.h>
#   include <float.h>
#   define isnan _isnan
#   undef SHOW_TIMING
#endif

bool US_Saxs_Util::read_control( QString controlfile )
{
   QFile f( controlfile );
   errormsg = "";
   if ( !f.exists() )
   {
      errormsg = QString( "Error: %1 file does not exist" ).arg( controlfile );
      return false;
   }

   if ( !f.open( IO_ReadOnly ) )
   {
      errormsg = QString( "Error: %1 can not be opened.  Check permissions" ).arg( controlfile );
      return false;
   }

   if ( controlfile.contains( QRegExp( "\\.(tar|TAR)$" ) ) )
   {
      f.close();
      cout << "extracting tar archive\n";
      US_Tar ust;
      QStringList qslt;
      int result = ust.list( controlfile, qslt, true );
      if ( TAR_OK != result )
      {
         errormsg = QString("Error: %1 problem extracting tar archive (%2)").arg( controlfile ).arg( result );
         return false;
      }
      result = ust.extract( controlfile );
      if ( TAR_OK != result )
      {
         errormsg = QString("Error: %1 problem extracting tar archive (%2)").arg( controlfile ).arg( result );
         return false;
      }
      if ( !qslt.size() )
      {
         errormsg = QString("Error: tar archive %1 appears empty").arg( controlfile );
         return false;
      }
         
      cout << QString( "from tar file %1, new control file %2\n" ).arg( controlfile ).arg( qslt[ 0 ] );
      controlfile = qslt[ 0 ];

      f.setName( controlfile );
      if ( !f.open( IO_ReadOnly ) )
      {
         errormsg = QString( "Error: %1 can not be opened.  Check permissions" ).arg( controlfile );
         return false;
      }
   }

   // read and setup control

   QTextStream ts( &f );
   QRegExp rx_blank  ( "^\\s*$" );
   QRegExp rx_comment( "#.*$" );
   QRegExp rx_valid  ( 
                       "^("
                       "atomfile|"
                       "hybridfile|"
                       "saxsfile|"
                       "saxs|"
                       "iqmethod|"
                       "fdbinsize|"
                       "fdmodulatin|"
                       "hypoints|"
                       "crysolharm|"
                       "crysolgrid|"
                       "crysolcsh|"
                       "wateredensity|"
                       "startq|"
                       "endq|"
                       "deltaq|"
                       "pdballmodels|"
                       "experimentgrid|"
                       "inputfile|"
                       "output|"
                       "outputfile|"
                       "process|"
                       "remark)$"
                       );

   QRegExp rx_file  ( 
                     "^("
                     "atomfile|"
                     "hybridfile|"
                     "saxsfile|"
                     "experimentgrid|"
                     "inputfile)$"
                     );

   QRegExp rx_arg_1  ( 
                      "^("
                      "atomfile|"
                      "hybridfile|"
                      "saxsfile|"
                      "iqmethod|"
                      "fdbinsize|"
                      "fdmodulatin|"
                      "hypoints|"
                      "crysolharm|"
                      "crysolgrid|"
                      "crysolcsh|"
                      "wateredensity|"
                      "startq|"
                      "endq|"
                      "deltaq|"
                      "experimentgrid|"
                      "inputfile|"
                      "output|"
                      "outputfile)$"
                      );

   QRegExp rx_valid_saxs_iqmethod (
                                   "^("
                                   "db|"
                                   "hy|"
                                   "hya|"
                                   "h2|"
                                   "h2a|"
                                   "h3|"
                                   "h3a|"
                                   "fd|"
                                   "foxs|"
                                   "crysol)$"
                                   );

   unsigned int line = 0;

   while ( !ts.atEnd() )
   {
      QString qs = ts.readLine().replace( rx_comment, "" ).replace( "^\\s+", "" ).replace( "\\s+$", "" );

      line++;

      if ( qs.contains( rx_blank ) )
      {
         continue;
      }

      QStringList qsl = QStringList::split( QRegExp("\\s+"), qs );

      if ( !qsl.size() )
      {
         continue;
      }

      if ( rx_valid.search( qsl[ 0 ].lower() ) == -1 )
      {
         errormsg = QString( "Error reading %1 line %2 : Unrecognized token %3" )
            .arg( controlfile )
            .arg( line )
            .arg( qsl[ 0 ] );
         return false;
      }

      QString option = qsl[ 0 ].lower();
      qsl.pop_front();
      control_parameters[ option ] = qsl;

      if ( rx_arg_1.search( option ) != -1 && 
           qsl.size() < 1 )
      {
         errormsg = QString( "Error reading %1 line %2 : Missing argument " )
            .arg( controlfile )
            .arg( line );
         return false;
      }

      if ( rx_file.search( option ) != -1 )
      {
         QFile qfc( qsl[ 0 ] );
         if ( !qfc.exists() )
         {
            errormsg = QString( "Error reading %1 line %2 : File %3 does not exist " )
               .arg( controlfile )
               .arg( line )
               .arg( qsl[ 0 ] );
            return false;
         }
      }         

      if ( option == "atomfile" )
      {
         cout << QString("read atom %1\n").arg( qsl[ 0 ] );
         if ( !select_atom_file( qsl[ 0 ] ) )
         {
            errormsg = QString( "Error %1 line %2 : %3" )
               .arg( controlfile )
               .arg( line )
               .arg( errormsg );
            return false;
         }
      }         

      if ( option == "hybridfile" )
      {
         cout << QString("read hybrid %1\n").arg( qsl[ 0 ] );
         if ( !select_hybrid_file( qsl[ 0 ] ) )
         {
            errormsg = QString( "Error %1 line %2 : %3" )
               .arg( controlfile )
               .arg( line )
               .arg( errormsg );
            return false;
         }
      }         

      if ( option == "saxsfile" )
      {
         cout << QString("read saxs atom %1\n").arg( qsl[ 0 ] );
         if ( !select_saxs_file( qsl[ 0 ] ) )
         {
            errormsg = QString( "Error %1 line %2 : %3" )
               .arg( controlfile )
               .arg( line )
               .arg( errormsg );
            return false;
         }
      }

      if ( option == "iqmethod" )
      {
         if ( rx_valid_saxs_iqmethod.search( qsl[ 0 ] ) == -1 )
         {
            errormsg = QString( "Error %1 line %2 : invalid %2 %3" )
               .arg( controlfile )
               .arg( line )
               .arg( option )
               .arg( qsl[ 0 ] );
            return false;
         }
      }            

      if ( option == "process" )
      {
         QStringList qsl_required;
         qsl_required << "startq";
         qsl_required << "endq";
         qsl_required << "deltaq";
         qsl_required << "iqmethod";
         qsl_required << "inputfile";
         qsl_required << "output";
         qsl_required << "outputfile";

         QString missing_required;

         for ( unsigned int i = 0; i < qsl_required.size(); i++ )
         {
            if ( !control_parameters.count( qsl_required[ i ] ) )
            {
               missing_required += " " + qsl_required[ i ];
            }
         }

         if ( !missing_required.isEmpty() )
         {
            errormsg = QString( "Error %1 line %2 : Process requires prior definition of:%3" )
               .arg( controlfile )
               .arg( line )
               .arg( missing_required );
            return false;
         }
      }
   }      
      
   f.close();
   return true;
}

