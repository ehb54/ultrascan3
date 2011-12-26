#include "us_mpi_analysis.h"
#include "us_math2.h"
#include "us_tar.h"
#include "us_memory.h"
#include "us_sleep.h"

#include <mpi.h>
#include <sys/user.h>

int main( int argc, char* argv[] )
{
   MPI_Init( &argc, &argv );
   QCoreApplication application( argc, argv );

   if ( argc == 1 )
      new US_MPI_Analysis( argv[ 1 ] );

   else if ( argc == 2 )
      new US_MPI_Analysis( argv[ 1 ], argv[ 2 ] );
}

// Constructor
US_MPI_Analysis::US_MPI_Analysis( const QString& tarfile,
      const QString& jxmlfili ) : QObject()
{
   MPI_Comm_size( MPI_COMM_WORLD, &proc_count );
   MPI_Comm_rank( MPI_COMM_WORLD, &my_rank );

   dbg_level    = 0;
   dbg_timing   = FALSE;

   if ( my_rank == 0 ) 
      socket = new QUdpSocket( this );

   QString output_dir = "output";
   QDir    d( "." );

   if ( my_rank == 0 )
   {
      // Unpack the input tarfile
      US_Tar tar;

      int result = tar.extract( tarfile );

      if ( result != TAR_OK ) abort( "Could not unpack " + tarfile );

      // Create a dedicated output directory and make sure it's empty
      // During testing, it may not always be empty
      QDir output( "." );
      output.mkdir  ( output_dir );
      output.setPath( output_dir );

      QStringList files = output.entryList( QStringList( "*" ), QDir::Files );
      QString     file;

      foreach( file, files ) output.remove( file );
      DbgLv(0) << "Start:  processor_count" << proc_count;
   }
 
   MPI_Barrier( MPI_COMM_WORLD ); // Sync everybody up

   QStringList files = d.entryList( QStringList( "hpc*.xml" ) );
   if ( files.size() != 1 ) abort( "Could not find unique hpc input file." );

   QString xmlfile   = files[ 0 ];
   QString jxmlfile  = jxmlfili;

   // Parse job xml file  (input argument or detected file)
   if ( jxmlfile.isEmpty() )
   {
      QStringList jfilt;
      jfilt << "input/*jobxmlfile.xml";
      jfilt << "us3.pbs";
      QStringList jfiles = d.entryList( jfilt, QDir::Files );
      jxmlfile           = jfiles.size() > 0 ? jfiles[ 0 ]
                                             : "input/jobxmlfile.xml";
DbgLv(1) << "  jfiles size" << jfiles.size() << "jxmlfile" << jxmlfile;
   }

   job_parse( jxmlfile );

   if ( my_rank == 0 )
   {  // Save submit time
      submitTime      = QFileInfo( tarfile ).lastModified();
mgroup_count=job_params["mgroupscount"].toInt();
DbgLv(0) << "submitTime " << submitTime << " parallel-masters count"
 << mgroup_count;
   }

   startTime      = QDateTime::currentDateTime();
   analysisDate   = startTime.toString( "yyMMddhhmm" );
   maxrss         = 0;
   set_count      = 0;
   iterations     = 1;

   previous_values.variance = 1.0e99;  // A large number

   data_sets .clear();
   parameters.clear();
   buckets   .clear();

   parse( xmlfile );

   uint seed = 0;
   
   if ( parameters.keys().contains( "seed" ) ) 
   {
      seed = parameters[ "seed" ].toUInt();
      qsrand( seed + my_rank );   // Set system random sequence
   }
   else
      US_Math2::randomize();

   group_rank = my_rank;    // Temporary setting for send_udp

   send_udp( "Starting" );  // Can't send udp message until xmlfile is parsed

   // Read data 
   for ( int i = 0; i < data_sets.size(); i++ )
   {
      US_SolveSim::DataSet* d = data_sets[ i ];

      try
      {
         int result = US_DataIO2::loadData( ".", d->edit_file, d->run_data );

         if ( result != US_DataIO2::OK ) throw result;
      }
      catch ( int error )
      {
         QString msg = "Abort.  Bad data file " + d->auc_file + " " + d->edit_file;
DbgLv(0) << "BAD DATA. error" << error << "rank" << my_rank;
         abort( msg, error );
      }
      catch ( US_DataIO2::ioError error )
      {
         QString msg = "Abort.  Bad data file " + d->auc_file + " " + d->edit_file;
DbgLv(0) << "BAD DATA. ioError" << error << "rank" << my_rank << proc_count;
//if(proc_count!=16)
         abort( msg, error );
      }
//DbgLv(0) << "Good DATA. rank" << my_rank;

      for ( int j = 0; j < d->noise_files.size(); j++ )
      {
          US_Noise noise;

          int err = noise.load( d->noise_files[ j ] );

          if ( err != 0 )
          {
             QString msg = "Abort.  Bad noise file " + d->noise_files[ j ];
             abort( msg );
          }

          if ( noise.apply_to_data( d->run_data  ) != 0 )
          {
             QString msg = "Abort.  Bad noise file " + d->noise_files[ j ];
             abort( msg );
          }
      }

      d->temperature = d->run_data.average_temperature();
      d->vbartb = US_Math2::calcCommonVbar( d->solution_rec, d->temperature );
   }

   // After reading all input, set the working directory for file output.
   QDir::setCurrent( output_dir );

   // Set some minimums
   max_iterations  = parameters[ "max_iterations" ].toInt();
   max_iterations  = max( max_iterations, 1 );

   mc_iterations   = parameters[ "mc_iterations" ].toInt();
   mc_iterations   = max( mc_iterations, 1 );

   meniscus_range  = parameters[ "meniscus_range"  ].toDouble();
   meniscus_points = parameters[ "meniscus_points" ].toInt();
   meniscus_points = max( meniscus_points, 1 );
   meniscus_range  = ( meniscus_points > 1 ) ? meniscus_range : 0.0;

   // Do some parameter checking
   bool global_fit = data_sets.size() > 1;

   if ( global_fit  &&  meniscus_points > 1 )
   {
      abort( "Meniscus fit is not compatible with multiple data sets" );
   }

   if ( meniscus_points > 1  &&  mc_iterations > 1 )
   {
      abort( "Meniscus fit is not compatible with Monte Carlo analysis" );
   }

   bool noise = parameters[ "rinoise_option" ].toInt() > 0  ||
                parameters[ "rinoise_option" ].toInt() > 0;

   if ( mc_iterations > 1  &&  noise )
   {
      abort( "Monte Carlo iteration is not compatible with noise computation" );
   }

   if ( global_fit && noise )
   {
      abort( "Global fit is not compatible with noise computation" );
   }

   // Calculate meniscus values
   meniscus_values.resize( meniscus_points );

   double meniscus_start = data_sets[ 0 ]->run_data.meniscus 
                         - meniscus_range / 2.0;
   
   double dm = ( meniscus_points > 1 ) ? meniscus_range / ( meniscus_points - 1 ): 0.0;

   for ( int i = 0; i < meniscus_points; i++ )
   {
      meniscus_values[ i ] = meniscus_start + dm * i;
   }

   // Get lower limit of data and last (largest) meniscus value
   double start_range   = data_sets[ 0 ]->run_data.radius( 0 );
   double last_meniscus = meniscus_values[ meniscus_points - 1 ];

   if ( last_meniscus >= start_range )
   {
      abort( "Meniscus value extends into data" );
   }

   population              = parameters[ "population"     ].toInt();
   generations             = parameters[ "generations"    ].toInt();
   crossover               = parameters[ "crossover"      ].toInt();
   mutation                = parameters[ "mutation"       ].toInt();
   plague                  = parameters[ "plague"         ].toInt();
   migrate_count           = parameters[ "migration"      ].toInt();
   elitism                 = parameters[ "elitism"        ].toInt();
   mutate_sigma            = parameters[ "mutate_sigma"   ].toDouble();
   p_mutate_s              = parameters[ "p_mutate_s"     ].toDouble();
   p_mutate_k              = parameters[ "p_mutate_k"     ].toDouble();
   p_mutate_sk             = parameters[ "p_mutate_sk"    ].toDouble();
   regularization          = parameters[ "regularization" ].toDouble();
   concentration_threshold = parameters[ "conc_threshold" ].toDouble();
   beta                    = (double)population / 8.0;

   // Calculate s, D corrections for calc_residuals; simulation parameters
   for ( int i = 0; i < data_sets.size(); i++ )
   {
      US_SolveSim::DataSet* ds = data_sets[ i ];

      // Convert to a different structure and calulate the s and D corrections
      US_Math2::SolutionData sd;
      sd.density   = ds->density;
      sd.viscosity = ds->viscosity;
      sd.vbar20    = ds->vbar20;
      sd.vbar      = ds->vbartb;

if ( my_rank == 0 )
   DbgLv(0) << "density/viscosity/comm vbar20/commvbar" << sd.density
                                                        << sd.viscosity 
                                                        << sd.vbar20 
                                                        << sd.vbar;

      US_Math2::data_correction( ds->temperature, sd );

      ds->s20w_correction = sd.s20w_correction;
      ds->D20w_correction = sd.D20w_correction;

if ( my_rank == 0 )
   DbgLv(0) << "s20w_correction/D20w_correction" << sd.s20w_correction 
    << sd.D20w_correction;

      // Set up simulation parameters for the data set
 
      ds->simparams.initFromData( NULL, ds->run_data );
 
      ds->simparams.rotorcoeffs[ 0 ]  = ds->rotor_stretch[ 0 ];
      ds->simparams.rotorcoeffs[ 1 ]  = ds->rotor_stretch[ 1 ];
      ds->simparams.bottom_position   = ds->centerpiece_bottom;
      ds->simparams.bottom            = ds->centerpiece_bottom;
      ds->simparams.band_forming      = ds->simparams.band_volume != 0.0;
   }

   // Check GA buckets
   if ( analysis_type == "GA" )
   {
      if ( buckets.size() < 2 )
         abort( "Insufficient buckets defined" );

      QList< QRectF > bucket_rects;

      // Put into Qt rectangles (upper left, lower right points)
      for ( int i = 0; i < buckets.size(); i++ )
      {
         limitBucket( buckets[ i ] );

         bucket_rects << QRectF( 
               QPointF( buckets[ i ].s_min, buckets[ i ].ff0_max ),
               QPointF( buckets[ i ].s_max, buckets[ i ].ff0_min ) );
      }

      for ( int i = 0; i < bucket_rects.size() - 1; i++ )
      {
         for ( int j = i + 1; j < bucket_rects.size(); j++ )
         {
            if ( bucket_rects[ i ].intersects( bucket_rects[ j ] ) )
               abort( "Buckets overlap" );
         }
      }
   }

   // Set some defaults
   if ( ! parameters.contains( "mutate_sigma" ) ) 
      parameters[ "mutate_sigma" ] = "2.0";

   if ( ! parameters.contains( "p_mutate_s"   ) ) 
      parameters[ "p_mutate_s"   ] = "20";

   if ( ! parameters.contains( "p_mutate_k"   ) ) 
      parameters[ "p_mutate_k"   ] = "20";

   if ( ! parameters.contains( "p_mutate_sk"  ) ) 
      parameters[ "p_mutate_sk"  ] = "20";

   count_calc_residuals = 0;   // Internal instrumentation
   meniscus_run         = 0;
   mc_iteration         = 0;

   // Determine masters-group count and related controls
   mgroup_count = job_params[ "mgroupscount" ].toInt();
   max_walltime = job_params[ "walltime"     ].toInt();
   if ( mc_iterations < 2  ||  mgroup_count > ( mc_iterations + 2 ) )
      mgroup_count = 1;

   gcores_count = proc_count / mgroup_count;

   if ( mgroup_count < 2 )
      start();                  // Start standard job
   
   else
      pmasters_start();         // Start parallel-masters job
}

// Alternate Constructor (empty jobxmlfile name)
US_MPI_Analysis::US_MPI_Analysis( const QString& tarfile ) : QObject()
{
   US_MPI_Analysis( tarfile, QString( "" ) );
}

// Main function  (single master group)
void US_MPI_Analysis::start( void )
{
   my_communicator         = MPI_COMM_WORLD;
   my_workers              = proc_count - 1;
   gcores_count            = proc_count;
   group_rank              = my_rank;

   // Real processing goes here
   if ( analysis_type == "2DSA" )
   {
      iterations = parameters[ "montecarlo_value" ].toInt();

      if ( iterations < 1 ) iterations = 1;

      if ( my_rank == 0 ) 
          _2dsa_master();
      else
          _2dsa_worker();
   }

   else if ( analysis_type == "GA" )
   {
      if ( my_rank == 0 ) 
          ga_master();
      else
          ga_worker();
   }

   int exit_status = 0;

   // Pack results
   if ( my_rank == 0 )
   {
      // Get job end time (after waiting so it has greatest time stamp)
      US_Sleep::msleep( 900 );
      QDateTime endTime = QDateTime::currentDateTime();
      bool reduced_iter = false;

      if ( mc_iterations > 0 )
      {
         int kc_iterations = parameters[ "mc_iterations" ].toInt();

         if ( mc_iterations < kc_iterations )
         {
            reduced_iter = true;
            exit_status  = 99;
         }
      }

      // Send message and build file with run-time statistics
      int  walltime = qRound(
         submitTime.msecsTo( endTime ) / 1000.0 );
      int  cputime  = qRound(
         startTime .msecsTo( endTime ) / 1000.0 );
      int  maxrssmb = qRound( (double)maxrss / 1024.0 );

      if ( reduced_iter )
      {
         send_udp( "Finished:  maxrss " + QString::number( maxrssmb )
               + " MB,  total run seconds " + QString::number( cputime )
               + "  (Reduced MC Iterations)" );
         DbgLv(0) << "Finished:  maxrss " << maxrssmb
                  << "MB,  total run seconds " << cputime
                  << "  (Reduced MC Iterations)";
      }

      else
      {
         send_udp( "Finished:  maxrss " + QString::number( maxrssmb )
               + " MB,  total run seconds " + QString::number( cputime ) );
         DbgLv(0) << "Finished:  maxrss " << maxrssmb
                  << "MB,  total run seconds " << cputime;
      }

      stats_output( walltime, cputime, maxrssmb,
            submitTime, startTime, endTime );

      // Build list and archive of output files
      QDir        d( "." );
      QStringList files = d.entryList( QStringList( "*" ), QDir::Files );

      US_Tar tar;
      tar.create( "analysis-results.tar", files );

      // Remove the files we just put into the tar archive
      QString file;
      foreach( file, files ) d.remove( file );
   }

   MPI_Finalize();
   exit( exit_status );
}

// Send udp
void US_MPI_Analysis::send_udp( const QString& message )
{
   QByteArray msg;

   if ( my_rank == 0 )
   {  // Send UDP message from supervisor (or single-group master)
///////////////////////////////*DEBUG*
//if(mgroup_count>1) return;   //*DEBUG*
///////////////////////////////*DEBUG*
      QString jobid = db_name + "-" + requestID + ": ";
      msg           = QString( jobid + message ).toAscii();
      socket->writeDatagram( msg.data(), msg.size(), server, port );
   }

   else if ( group_rank == 0 )
   {  // For pm group master, forward message to supervisor
      int     super = 0;
      QString gpfix = QString( "(pmg %1) " ).arg( my_group );
      msg           = QString( gpfix + message ).toAscii();
      int     size  = msg.size();

      MPI_Send( &size,
                sizeof( int ),
                MPI_BYTE,
                super,
                UDPSIZE,
                MPI_COMM_WORLD );

      MPI_Send( msg.data(),
                size,
                MPI_BYTE,
                super,
                UDPMSG,
                MPI_COMM_WORLD );
   }
}

long int US_MPI_Analysis::max_rss( void )
{
   return US_Memory::rss_max( maxrss );
}

void US_MPI_Analysis::abort( const QString& message, int error )
{
    send_udp( message );
    DbgLv(0) << message;
    MPI_Abort( MPI_COMM_WORLD, error );
    exit( error );
}

// Create output statistics file
void US_MPI_Analysis::stats_output( int walltime, int cputime, int maxrssmb,
      QDateTime submitTime, QDateTime startTime, QDateTime endTime )
{
   QString fname = "job_statistics.xml";
   QFile   file( fname );

   if ( ! file.open( QIODevice::WriteOnly | QIODevice::Text ) )
      return;

   QString sSubmitTime = submitTime.toString( Qt::ISODate ).replace( "T", " " );
   QString sStartTime  = startTime .toString( Qt::ISODate ).replace( "T", " " );
   QString sEndTime    = endTime   .toString( Qt::ISODate ).replace( "T", " " );
      
   QXmlStreamWriter xml( &file );

   xml.setAutoFormatting( true );
   xml.writeStartDocument();
   xml.writeDTD          ( "<!DOCTYPE US_Statistics>" );
   xml.writeStartElement ( "US_JobStatistics" );
   xml.writeAttribute    ( "version", "1.0" );
   xml.writeStartElement ( "statistics" );
   xml.writeAttribute    ( "walltime",     QString::number( walltime   ) );
   xml.writeAttribute    ( "cputime",      QString::number( cputime    ) );
   xml.writeAttribute    ( "cpucount",     QString::number( proc_count ) );
   xml.writeAttribute    ( "maxmemory",    QString::number( maxrssmb   ) );
   xml.writeAttribute    ( "cluster",      cluster                     );
   xml.writeAttribute    ( "analysistype", analysis_type               );
   xml.writeEndElement   ();  // statistics

   xml.writeStartElement ( "id" );
   xml.writeAttribute    ( "requestguid",  requestGUID  );
   xml.writeAttribute    ( "dbname",       db_name      );
   xml.writeAttribute    ( "requestid",    requestID    );
   xml.writeAttribute    ( "submittime",   sSubmitTime  );
   xml.writeAttribute    ( "starttime",    sStartTime   );
   xml.writeAttribute    ( "endtime",      sEndTime     );
   xml.writeAttribute    ( "maxwalltime",  QString::number( max_walltime ) );
   xml.writeAttribute    ( "groupcount",   QString::number( mgroup_count ) );
   xml.writeEndElement   ();  // id
   xml.writeEndElement   ();  // US_JobStatistics
   xml.writeEndDocument  ();

   file.close();

   // Add the file name of the statistics file to the output list
   QFile f( "analysis_files.txt" );
   if ( ! f.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append ) )
   {
      abort( "Could not open 'analysis_files.txt' for writing", -1 );
      return;
   }

   QTextStream out( &f );
   out << fname << "\n";
   f.close();
   return;
}

// Insure vertexes of a bucket do not exceed physically possible limits
void US_MPI_Analysis::limitBucket( Bucket& buk )
{
   if ( buk.s_min > 0.0 )
   {  // All-positive s's start at 0.1 at least
      buk.s_min   = qMax( 0.1, buk.s_min );
      buk.s_max   = qMax( ( buk.s_min + 0.0001 ), buk.s_max );
   }

   else if ( buk.s_max <= 0.0 )
   {  // All-negative s's end at -0.1 at most
      buk.s_max   = qMin( -0.1, buk.s_max );
      buk.s_min   = qMin( ( buk.s_max - 0.0001 ), buk.s_min );
   }

   else if ( ( buk.s_min + buk.s_max ) >= 0.0 )
   {  // Mostly positive clipped to all positive starting at 0.1
      buk.s_min   = 0.1;
      buk.s_max   = qMax( 0.2, buk.s_max );
   }

   else
   {  // Mostly negative clipped to all negative ending at -0.1
      buk.s_min   = qMin( -0.2, buk.s_min );
      buk.s_max   = -0.1;
   }

   buk.ff0_min = qMax(  1.0, buk.ff0_min );
   buk.ff0_max = qMax( ( buk.ff0_min + 0.0001 ), buk.ff0_max );
}

