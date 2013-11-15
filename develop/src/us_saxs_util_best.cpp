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

   int p = 0;
   {
      QString cmd = 
         QString( "%1 -m %2 -r %3 -y %4 -t %5.c3p -v %6.c3v" )
         .arg( progs[ p ] )
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

      cmd += QString( " 2> msr_%1.stderr > msr_%2.stdout" ).arg( inputbase ).arg( inputbase );
      qDebug( QString( "best cmd = %1" ).arg( cmd ) );

      cout << "Starting " + progs[ p ] + "\n";
      cout << cmd << endl;
      system( cmd.ascii() );
      cout << "Finished " + progs[ p ] + "\n";

      QStringList expected;
      expected 
         << inputbase + ".c3p"
         << inputbase + ".c3v"
         << "msr_" + inputbase + ".stdout"
         << "msr_" + inputbase + ".stderr"
         ;

      for ( int i = 0; i < (int) expected.size(); ++i )
      {
         if ( !QFile( expected[ i ] ).exists() )
         {
            errormsg += QString( "BEST: %1 did not produce expected output file %2\n" )
               .arg( progs[ p ] )
               .arg( expected[ i ] )
               ;
         }
      }
   }

   if ( !errormsg.isEmpty() )
   {
      return false;
   }

   // run rcoal
   p++;
   {
      QString cmd = 
         QString( "%1 -f %2.c3p -nmax %3 -nmin %4 -n %5" )
         .arg( progs[ p ] )
         .arg( inputbase )
         .arg( control_parameters[ "bestrcoalnmax" ] )
         .arg( control_parameters[ "bestrcoalnmin" ] )
         .arg( control_parameters[ "bestrcoaln" ] )
         ;

      cmd += QString( " 2> rcoal_%1.stderr > rcoal_%2.stdout" ).arg( inputbase ).arg( inputbase );

      qDebug( QString( "best cmd = %1" ).arg( cmd ) );
      cout << "Starting " + progs[ p ] + "\n";
      cout << cmd << endl;
      system( cmd.ascii() );
      cout << "Finished " + progs[ p ] + "\n";

      // run & check output
      QStringList expected;
      expected 
         << "rcoal_" + inputbase + ".stdout"
         << "rcoal_" + inputbase + ".stderr"
         ;

      for ( int i = 0; i < (int) expected.size(); ++i )
      {
         if ( !QFile( expected[ i ] ).exists() )
         {
            errormsg += QString( "BEST: %1 did not produce expected output file %2\n" )
               .arg( progs[ p ] )
               .arg( expected[ i ] )
               ;
         }
      }
   }

   if ( !errormsg.isEmpty() )
   {
      return false;
   }

   QStringList outfiles;
   {
      QFile f( "rcoal_" + inputbase + ".stdout" );
      f.open( IO_ReadOnly );
      QTextStream ts( &f );
      while ( !ts.atEnd() )
      {
         QString qs = ts.readLine();
         if ( qs.contains( "Output files written:" ) )
         {
            outfiles << QString( ts.readLine() ).stripWhiteSpace();
         }
      }
      f.close();
   }

   // run best
   p++;
   for ( int i = 0; i < (int) outfiles.size(); ++i )
   {
      qDebug( QString( "processing outfile %1" ).arg( outfiles[ i ] ) );
      QString cmd = 
         QString( "%1 -f %2 -mw %3" )
         .arg( progs[ p ] )
         .arg( outfiles[ i ] )
         .arg( control_parameters[ "bestbestmw" ] )
         ;

      if ( control_parameters.count( "bestbestna" ) )
      {
         cmd += " -na";
      }
      if ( control_parameters.count( "bestbestp" ) )
      {
         cmd += " -p";
      }
      if ( control_parameters.count( "bestbestv" ) )
      {
         cmd += " -v";
      }
      if ( control_parameters.count( "bestbestvc" ) )
      {
         cmd += " -vc";
      }

      cmd += QString( " 2> best_%1.stderr > best_%2.stdout" ).arg( outfiles[ i ] ).arg( outfiles[ i ] );

      qDebug( QString( "best cmd = %1" ).arg( cmd ) );
      cout << "Starting " + progs[ p ] + "\n";
      cout << cmd << endl;
      system( cmd.ascii() );
      cout << "Finished " + progs[ p ] + "\n";

      QStringList expected;
      expected 
         << "best_" + outfiles[ i ] + ".stdout"
         << "best_" + outfiles[ i ] + ".stderr"
         ;

      for ( int i = 0; i < (int) expected.size(); ++i )
      {
         if ( !QFile( expected[ i ] ).exists() )
         {
            errormsg += QString( "BEST: %1 did not produce expected output file %2\n" )
               .arg( progs[ p ] )
               .arg( expected[ i ] )
               ;
         }
      }
   }

   errormsg = "best: not yet\n"; 
   return false;
}
