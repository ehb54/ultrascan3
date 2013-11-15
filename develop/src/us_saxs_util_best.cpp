#include "../include/us_saxs_util.h"

// note: this program uses cout and/or cerr and this should be replaced

bool US_Saxs_Util::run_best()
{
   errormsg = "";
   QStringList required;
   required 
      << "inputfilenoread"
      << "bestrcoalnmin"
      << "bestrcoalnmax"
      << "bestrcoaln"
      << "bestmsrradiifile"
      << "bestmsrpatternfile"
      ;

   for ( int i = 0; i < (int) required.size(); ++i )
   {
      if ( !control_parameters.count( required[ i ] ) )
      {
         errormsg += QString( "BEST: %1 parameter must be defined\n" ).arg( required[ i ] );
      }
   }      

   QString prog_base =
      env_ultrascan + QDir::separator() +
#if defined(BIN64)
      "bin64"
#else
      "bin"
#endif
      + QDir::separator()
      ;

   vector < QString > progs;
   progs.push_back( prog_base + "msroll" );
   progs.push_back( prog_base + "rcoal" );
   progs.push_back( prog_base + "best" );

   for ( int i = 0; i < (int)progs.size(); ++i )
   {
      QFileInfo qfi( progs[ i ] );
      if ( !qfi.exists() )
      {
         errormsg += QString( "BEST: program '%1' does not exist\n" ).arg( progs[ i ] );
      }
   }

   if ( !errormsg.isEmpty() )
   {
      return false;
   }

   QString inputbase = QFileInfo( control_parameters[ "inputfilenoread" ] ).baseName();

   // run msroll

   {
      QString cmd = 
         QString( "%1 -m %2 -r %3 -y %4 -t %5.c3p -v %6.c3v" )
         .arg( progs[ 0 ] )
         .arg( control_parameters[ "inputfilenoread" ] )
         .arg( control_parameters[ "bestmsrradiifile" ] )
         .arg( control_parameters[ "bestmsrpatternfile" ] )
         .arg( inputbase )
         .arg( inputbase )
         ;

      if ( control_parameters.count( "bestmsrprober" ) )
      {
         cmd += QString( " -p %1" ).arg( control_parameters[ "bestmsrprober" ] );
      }

      if ( control_parameters.count( "bestmsrfinenessangle" ) )
      {
         cmd += QString( " -f %1" ).arg( control_parameters[ "bestmsrfinenessangle" ] );
      }

      if ( control_parameters.count( "bestmsrcoalescer" ) )
      {
         cmd += QString( " -l %1" ).arg( control_parameters[ "bestmsrcoalescer" ] );
      }

      qDebug( QString( "best cmd = %1\n" ).arg( cmd ) );

      // run & check output
   }

   // run rcoal
   {
      QString cmd = 
         QString( "%1 -f %2.c3p -nmax %3 -nmin %4 -n %5" )
         .arg( progs[ 1 ] )
         .arg( inputbase )
         .arg( control_parameters[ "bestrcoalnmax" ] )
         .arg( control_parameters[ "bestrcoalnmin" ] )
         .arg( control_parameters[ "bestrcoalnm" ] )
         ;

      qDebug( QString( "best cmd = %1\n" ).arg( cmd ) );

      // run & check output
   }

   // run best
   {
      QString cmd = 
         QString( "%1 -f %2.c3p -nmax %3 -nmin %4 -n %5" )
         .arg( progs[ 1 ] )
         .arg( inputbase )
         .arg( control_parameters[ "bestrcoalnmax" ] )
         .arg( control_parameters[ "bestrcoalnmin" ] )
         .arg( control_parameters[ "bestrcoalnm" ] )
         ;

      qDebug( QString( "best cmd = %1\n" ).arg( cmd ) );

      // run & check output
      // get list of files for next step
   }

   errormsg = "best: not yet\n"; 
   return false;
}
