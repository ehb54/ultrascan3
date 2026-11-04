//! \file us_mpi_jobcheck.cpp
//! \brief Checks hpcrequest job files as us_mpi_analysis does, without MPI
//!        and without data.
//!
//! Usage: us_mpi_jobcheck [-q] <hpcrequest.xml>...
//!
//! Prints the errors and warnings of each file ("-q": only files with
//! errors) and a summary.  The exit status is 1 if any file has errors.

#include "us_mpi_jobconfig.h"

#include <QtCore>
#include <cstdio>

int main( int argc, char* argv[] )
{
   QCoreApplication app( argc, argv );
   QStringList      args  = app.arguments().mid( 1 );
   bool             quiet = args.removeAll( "-q" ) > 0;

   if ( args.isEmpty() )
   {
      fprintf( stderr, "usage: %s [-q] <hpcrequest.xml>...\n", argv[0] );
      return 2;
   }

   int nerror = 0;
   int nwarn  = 0;
   for ( const QString& path : args )
   {
      US_MPI_JobConfig config = US_MPI_JobConfig::fromFile( path );
      if ( config.ok() )
      {
         config.validate();
      }

      nerror += config.errors.isEmpty() ? 0 : 1;
      nwarn += config.warnings.isEmpty() ? 0 : 1;
      if ( quiet && config.errors.isEmpty() )
      {
         continue;
      }

      for ( const QString& error : config.errors )
      {
         printf( "%s: error: %s\n", qPrintable( path ), qPrintable( error ) );
      }
      for ( const QString& warning : config.warnings )
      {
         printf( "%s: warning: %s\n", qPrintable( path ), qPrintable( warning ) );
      }
   }

   printf( "%d files, %d with errors, %d with warnings\n", (int) args.size(), nerror, nwarn );
   return nerror > 0 ? 1 : 0;
}
