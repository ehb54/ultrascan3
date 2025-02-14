#include "us_mpi_analysis.h"
#include "us_math2.h"
#include "us_astfem_math.h"
#include "us_archive.h"
#include "us_memory.h"
#include "us_sleep.h"
#include "us_util.h"
#include "us_settings.h"
#include "us_revision.h"

#include <mpi.h>
#include <sys/user.h>
#include <cstdio>

#define ELAPSED_SECS (startTime.msecsTo(QDateTime::currentDateTime())/1000.0)

// Message handler possibly needed for qDebug()
void myMessageOutput( QtMsgType type, const QMessageLogContext& /*context*/, const QString &msg )
{
   QByteArray localMsg = msg.toLocal8Bit();
   switch ( type ) {
   case QtDebugMsg:
      fprintf( stderr, "%s\n", localMsg.constData() );
      break;
   case QtInfoMsg:
      fprintf( stderr, "Info: %s\n", localMsg.constData() );
      break;
   case QtWarningMsg:
      fprintf( stderr, "Warning: %s\n", localMsg.constData() );
      break;
   case QtCriticalMsg:
      fprintf( stderr, "Critical: %s\n", localMsg.constData() );
      break;
   case QtFatalMsg:
      fprintf( stderr, "Fatal: %s\n", localMsg.constData() );
      abort();
   }
}


int main( int argc, char* argv[] )
{
#ifdef NEED_MSG_HANDLER
   // On some systems we need to install a message handler for qDebug()
   qInstallMessageHandler( myMessageOutput );
#endif

   MPI_Init( &argc, &argv );
   QCoreApplication application( argc, argv );

   QStringList cmdargs;

   for ( int jj = 0; jj < argc; jj++ )
      cmdargs << argv[ jj ];

   new US_MPI_Analysis( argc, cmdargs );
}

// Constructor
US_MPI_Analysis::US_MPI_Analysis( int nargs, QStringList& cmdargs ) : QObject()
{
//printf( "QT_LOGGING_RULES=\"%s\"\n", (char*)getenv("QT_LOGGING_RULES") );
   // Command line special parameter keys
   const QString wallkey ( "-walltime" );
   const QString pmgckey ( "-mgroupcount" );
   const QString sgszkey ( "-sgsize" );
   // Alternate versions of those keys
   const QString wallkey2( "-WallTimeLimit" );
   const QString pmgckey2( "-MGroupCount" );
   const QString sgszkey2( "-SubGridSize" );
   const QString wallkey3( "-wallTimeLimit" );
   const QString pmgckey3( "-pmgc" );
   const QString sgszkey3( "-subgridsize" );

   MPI_Comm_size( MPI_COMM_WORLD, &proc_count );
   MPI_Comm_rank( MPI_COMM_WORLD, &my_rank );

   dbg_level    = 0;
   dbg_timing   = false;
   maxrss       = 0L;
   minimize_opt = 2;
   in_gsm       = false;
   min_experiment_size = def_experiment_size;
   QString tarfile;
   QString jxmlfili;
   task_params[ "walltime"    ] = "1440";
   task_params[ "mgroupcount" ] = "1";
   task_params[ "subgridsize" ] = "100";

   // Get some task parameters from the command line
   for ( int jj = 1; jj < nargs; jj++ )
   {
      QString cmdarg = cmdargs[ jj ];
if(my_rank==0)
DbgLv(0) << "CmdArg: jj" << jj << "cmdarg" << cmdarg;

      if ( cmdarg.startsWith( "-" ) )
      {  // Argument pair is keyed set (.e.g., "-maxwall <n>")
         QString cmdval = "";
         int valx       = cmdarg.indexOf( "=" ); 

         if ( valx > 0 )
         {  // Syntax is "-key=value"
            cmdval         = cmdarg.mid( valx + 1 ).simplified();
         }

         else if ( ( jj + 1 ) < nargs )
         {  // Syntax is "-key value"
            cmdval         = cmdargs[ ++jj ];
         }

         if ( cmdarg.contains( wallkey )   ||
              cmdarg.contains( wallkey2 )  ||
              cmdarg.contains( wallkey3 ) )
         {  // Get maximum wall time in minutes
            task_params[ "walltime"    ] = cmdval;
         }

         else if ( cmdarg.contains( pmgckey )   ||
                   cmdarg.contains( pmgckey2 )  ||
                   cmdarg.contains( pmgckey3 ) )
         {  // Get number of parallel masters groups
            task_params[ "mgroupcount" ] = cmdval;
         }
         else if ( cmdarg.contains( sgszkey )   ||
                   cmdarg.contains( sgszkey2 )  ||
                   cmdarg.contains( sgszkey3 ) )
         {  // Get minimum subgrid size in solute points
            task_params[ "subgridsize" ] = cmdval;
         }
else
DbgLv(0) << "CmdArg -- unknown cmdarg " << cmdarg << "cmdval" << cmdval;
if(my_rank==0)
DbgLv(0) << "CmdArg:   valx" << valx << "cmdval" << cmdval;
      }

      else if ( tarfile.isEmpty() )
      {  // First non-keyed argument is tar file path
         tarfile      = cmdarg;
      }

      else
      {  // Second non-keyed argument is jobxmlfile
         jxmlfili     = cmdarg;
      }
   }
if(my_rank==0) {
DbgLv(0) << "CmdArg: walltime" << task_params["walltime"];
DbgLv(0) << "CmdArg: mgroupcount" << task_params["mgroupcount"];
DbgLv(0) << "CmdArg: minexpsize" << task_params["subgridsize"];
DbgLv(0) << "CmdArg: tarfile" << tarfile;
DbgLv(0) << "CmdArg: jxmlfili" << jxmlfili;
}

   attr_x       = ATTR_S;
   attr_y       = ATTR_K;
   attr_z       = ATTR_V;

   if ( my_rank == 0 ) 
      socket = new QUdpSocket( this );

   QStringList ffilt;
   QDir    wkdir      = QDir::current();
   QString work_dir   = QDir::currentPath();
   QString output_dir = work_dir + "/output";
   QString input_dir  = work_dir + "/input";

   if ( my_rank == 0 )
   {
DbgLv(0) << "wkdir=" << wkdir;
DbgLv(0) << "work_dir=" << work_dir;
      DbgLv(0) << "Us_Mpi_Analysis  " << REVISION;

      // Unpack the input tarfile
      US_Archive archive;
      bool ok = archive.extract( tarfile );
      if ( !ok ) {
         QString error = archive.getError();
         abort( "Could not unpack\n " + error + "\n" + tarfile );
      }

      // Create a dedicated output directory and make sure it's empty
      // During testing, it may not always be empty
      wkdir.mkdir  ( output_dir );

      QDir odir( output_dir );
      ffilt.clear();
      ffilt << "*";
      QStringList files = odir.entryList( ffilt, QDir::Files );
      QString     file;

      foreach( file, files ) odir.remove( file );
      DbgLv(0) << "Start:  processor_count" << proc_count;
   }
 
   MPI_Barrier( MPI_COMM_WORLD ); // Sync everybody up


   startTime      = QDateTime::currentDateTime();
   analysisDate   = startTime.toUTC().toString( "yyMMddhhmm" );
   set_count      = 0;
   iterations     = 1;

   previous_values.variance = 1.0e39;  // A large number

   concentrations.clear();

   data_sets .clear();
   parameters.clear();
   buckets   .clear();
   maxods    .clear();

   QString xmlfile;
   ffilt.clear();
   ffilt << "hpc*.xml";
   QStringList files = wkdir.entryList( ffilt );
   if ( files.size() == 0 )
   {
      files = QDir( input_dir ).entryList( ffilt );
      if ( files.size() == 1 )
         xmlfile = input_dir + "/" + files[ 0 ];
   }
   else
      xmlfile = files[ 0 ];

   if ( files.size() != 1 ) abort( "Could not find unique hpc input file." );

   // Parse analysis parameters
   parse( xmlfile );

   uint seed = 0;
   
   if ( parameters.keys().contains( "seed" ) ) 
   {
      seed = parameters[ "seed" ].toUInt();
      qsrand( seed + my_rank );   // Set system random sequence
   }
   else
      US_Math2::randomize();

   QString jxmlfile  = jxmlfili;

   // Parse task xml file if present or needed (input argument or detected file)
   if ( jxmlfile.isEmpty() )
   {  // Not on command line: just look for its presence in the work directory
      ffilt.clear();
      ffilt << "*jobxmlfile.xml";
      ffilt << "jobxmlfile.xml";
      QStringList jfiles = QDir( input_dir ).entryList( ffilt, QDir::Files );
if(my_rank==0) DbgLv(0) << "  jfiles size" << jfiles.size();

      if ( jfiles.size() > 0 )
      {  // Files found in ./input
         jxmlfile           = input_dir + "/" + jfiles[ 0 ];
      }

      else
      {  // Files not found in ./input, so try base work directory
         ffilt << "us3.pbs";
         jfiles          = wkdir.entryList( ffilt, QDir::Files );
         jxmlfile           = jfiles.size() > 0 ? jfiles[ 0 ] : "";
      }
if(my_rank==0)
DbgLv(0) << "  jfiles size" << jfiles.size() << "jxmlfile" << jxmlfile;
   }

   task_parse( jxmlfile );

   if ( my_rank == 0 )
   {  // Save submit time
      submitTime      = QFileInfo( tarfile ).lastModified();
DbgLv(0) << "submitTime " << submitTime
 << " mgroupcount" << task_params["mgroupcount"].toInt()
 << " walltime" << task_params["walltime"].toInt();

      printf( "Us_Mpi_Analysis %s has started.\n", REVISION );
   }

   group_rank = my_rank;    // Temporary setting for send_udp

   QString msg_start = QString( "Starting --  " ) + QString( REVISION );
   send_udp( msg_start );   // Can't send udp message until xmlfile is parsed

   // Read data 
   for ( int ii = 0; ii < data_sets.size(); ii++ )
   {
      US_SolveSim::DataSet* dset = data_sets[ ii ];

      try
      {
         int result = US_DataIO::loadData( ".", dset->edit_file,
                                                dset->run_data );

         if ( result != US_DataIO::OK ) throw result;
      }
      catch ( int error )
      {
         QString msg = "Bad data file " + dset->auc_file + " "
                                        + dset->edit_file;
DbgLv(0) << "BAD DATA. error" << error << "rank" << my_rank;
         abort( msg, error );
      }
      catch ( US_DataIO::ioError error )
      {
         QString msg = "Bad data file " + dset->auc_file + " "
                                        + dset->edit_file;
DbgLv(0) << "BAD DATA. ioError" << error << "rank" << my_rank << proc_count;
//if(proc_count!=16)
         abort( msg, error );
      }
//DbgLv(0) << "Good DATA. rank" << my_rank;

      for ( int jj = 0; jj < dset->noise_files.size(); jj++ )
      {
          US_Noise noise;

          int err = noise.load( dset->noise_files[ jj ] );

          if ( err != 0 )
          {
             QString msg = "Bad noise file " + dset->noise_files[ jj ];
             abort( msg );
          }

          if ( noise.apply_to_data( dset->run_data  ) != 0 )
          {
             QString msg = "Bad noise file " + dset->noise_files[ jj ];
             abort( msg );
          }
      }

      dset->temperature = dset->run_data.average_temperature();
      dset->vbartb = US_Math2::calcCommonVbar( dset->solution_rec,
                                               dset->temperature );

      if ( dset->centerpiece_bottom == 7.3 )
         abort( "The bottom is set to the invalid default value of 7.3" );
   }

   // After reading all input, set the working directory for file output.
   QDir::setCurrent( output_dir );

   // Set some minimums
   max_iterations  = parameters[ "max_iterations" ].toInt();
   max_iterations  = qMax( max_iterations, 1 );

   mc_iterations   = parameters[ "mc_iterations" ].toInt();
   mc_iterations   = qMax( mc_iterations, 1 );

   // Set Fit-Meniscus/Bottom parameters
   fit_mb_select   = parameters[ "fit_mb_select"   ].toInt();
   meniscus_range  = parameters[ "meniscus_range"  ].toDouble();
   meniscus_points = parameters[ "meniscus_points" ].toInt();
   fit_mb_select   = ( meniscus_points > 1 ) ?  qMax( 1, fit_mb_select ) : 0;
   fit_meni        = ( ( fit_mb_select & 1 ) != 0 );
   fit_bott        = ( ( fit_mb_select & 2 ) != 0 );
   fit_menbot      = ( fit_meni  &&  fit_bott );
if (my_rank==0) {
DbgLv(0) << "FMB: fit_mb_select" << fit_mb_select
 << "    fit_meni _bott _menbot" << fit_meni << fit_bott << fit_menbot; }

   if ( fit_menbot )
   {  // Meniscus-and-bottom fit iterations will be run
      bottom_range    = meniscus_range;
      bottom_points   = meniscus_points;
      menibott_count  = meniscus_points * bottom_points;
   }

   else if ( fit_meni )
   {  // Meniscus-only fit iterations will be run
      bottom_range    = 0.0;
      bottom_points   = 1;
      menibott_count  = meniscus_points;
   }

   else if ( fit_bott )
   {  // Bottom-only fit iterations will be run
      bottom_range    = meniscus_range;
      bottom_points   = meniscus_points;
      meniscus_range  = 0.0;
      meniscus_points = 1;
      menibott_count  = bottom_points;
   }

   else
   {  // No meniscus/bottom fit
      meniscus_range  = 0.0;
      meniscus_points = 1;
      bottom_range    = 0.0;
      bottom_points   = 1;
      menibott_count  = 1;
   }
if (my_rank==0) {
DbgLv(0) << "FMB: meni range points" << meniscus_range << meniscus_points
 << "  bott range points" << bottom_range << bottom_points; }

   // Do some parameter checking
   count_datasets     = data_sets.size();
   is_global_fit      = US_Util::bool_flag( parameters[ "global_fit" ] );
   is_composite_job   = ( count_datasets > 1  &&  ! is_global_fit );
   if ( my_rank == 0 )
   {
      DbgLv(0) << " count_datasets   " << count_datasets;
      DbgLv(0) << " is_global_fit    " << is_global_fit;
      DbgLv(0) << " is_composite_job " << is_composite_job;
   }

   if ( is_global_fit )
   {
      if ( data_sets[ 0 ]->simparams.speed_step.count() > 1 )
      {  // Change concentrations back to 1.0 if multi-speed/global-fit
         concentrations.resize( count_datasets );
         for ( int ee = 0; ee < count_datasets; ee++ )
         {
            concentrations[ ee ] = 1.0;
         }
      }

      if ( meniscus_points > 1 )
      {
         abort( "Meniscus fit is not compatible with global fit" );
      }

      if ( concentrations.count() < count_datasets )
      {
         abort( "Unable to obtain all total_concentrations with global fit" );
      }

      else
      {
         double concen_min  = 99999.9;
         for ( int ee = 0; ee < count_datasets; ee++ )
         {
            concen_min         = qMin( concen_min, concentrations[ ee ] );
         }
         if ( concen_min <= 0.0 )
         {
            abort( "Unable to obtain all total_concentrations with global fit" );
         }
      }

   }

   if ( meniscus_points > 1  &&  mc_iterations > 1 )
   {
      abort( "Meniscus fit is not compatible with Monte Carlo analysis" );
   }

   bool noise = parameters[ "tinoise_option" ].toInt() > 0  ||
                parameters[ "rinoise_option" ].toInt() > 0;

   if ( ! analysis_type.startsWith( "PCSA" )  &&
        mc_iterations > 1  &&  noise )
   {
      abort( "Monte Carlo iteration is not compatible with noise computation" );
   }

   if ( is_global_fit  &&  noise )
   {
      abort( "Global fit is not compatible with noise computation" );
   }

   // Calculate meniscus values
   meniscus_values.resize( meniscus_points );

   double meniscus_start = data_sets[ 0 ]->run_data.meniscus 
                         - meniscus_range / 2.0;
   
   double dmen           = fit_meni ?
                           ( meniscus_range / ( meniscus_points - 1 ) ) :
                           0.0;

   for ( int ii = 0; ii < meniscus_points; ii++ )
   {
      meniscus_values[ ii ]  = meniscus_start + dmen * ii;
   }

   // Get lower limit of data and last (largest) meniscus value
   double start_range    = data_sets[ 0 ]->run_data.radius( 0 );
   double last_meniscus  = meniscus_values[ meniscus_points - 1 ];
if (my_rank==0) {
 DbgLv(0) << "FMB: meniscus:  start delta" << meniscus_start << dmen; }

   if ( last_meniscus >= start_range )
   {
      if ( my_rank == 0 )
      {
         qDebug() << "*ERROR* Meniscus value extends into data";
         qDebug() << " data meniscus" << data_sets[0]->run_data.meniscus;
         qDebug() << " meniscus_start" << meniscus_start;
         qDebug() << " meniscus_range" << meniscus_range;
         qDebug() << " meniscus_points" << meniscus_points;
         qDebug() << " meniscus delta" << dmen;
         qDebug() << " last_meniscus" << last_meniscus;
         qDebug() << " left_data" << start_range;
      }
      abort( "Meniscus value extends into data" );
   }

   // Calculate bottom values
   bottom_values.resize( bottom_points );

   // Use bottom from edited data if it is given
   US_SolveSim::DataSet*  ds    = data_sets[ 0 ];
   double bottom_ds    = ds->run_data.bottom;
   if ( bottom_ds == 0.0 )
   {
      double rpm          = ds->run_data.scanData[ 0 ].rpm;
      bottom_ds           = US_AstfemMath::calc_bottom( rpm,
                                                        ds->centerpiece_bottom,
                                                        ds->rotor_stretch );
   }
   double bottom_start = bottom_ds - bottom_range / 2.0;
   
   double dbot         = fit_bott ?
                         ( bottom_range / ( bottom_points - 1 ) ) :
                         0.0;

   for ( int ii = 0; ii < bottom_points; ii++ )
   {
      bottom_values[ ii ]  = bottom_start + dbot * ii;
   }
if (my_rank==0) {
 DbgLv(0) << "FMB: bottom:  start delta" << bottom_start << dbot; }

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
   minimize_opt            = parameters[ "minimize_opt"   ].toInt();
minimize_opt=(minimize_opt==0?2:minimize_opt);
   total_points            = 0;
   bool redo_ss            = false;     // By default, accept speed step as is
   bool redo_tstate        = false;     // By default, accept timestate as is
   double ds_concen        = 1.0;
   int ntmsf_ne            = 0;         // Number timestate files non-empty
   int ntmsf_ex            = 0;         // Number timestate files that exist
   int ntmsf_1s            = 0;         // Nbr tstate files at 1-sec interval

   // Calculate s, D corrections for calc_residuals; simulation parameters
   for ( int ee = 0; ee < data_sets.size(); ee++ )
   {
      US_SolveSim::DataSet*  ds    = data_sets[ ee ];
      US_DataIO::EditedData* edata = &ds->run_data;

      // Convert to a different structure and calculate the s and D corrections
      US_Math2::SolutionData sd;
      sd.density   = ds->density;
      sd.viscosity = ds->viscosity;
      sd.manual    = ds->manual;
      sd.vbar20    = ds->vbar20;
      sd.vbar      = ds->vbartb;
      ds_concen    = is_global_fit ? concentrations[ ee ] : 1.0;
      ds_concen    = ( ds_concen > 0.0 ) ? ( 1.0 / ds_concen ) : 1.0;
if ( my_rank == 0 )
 DbgLv(0) << "density/viscosity/comm vbar20/commvbar/compress"
  << sd.density << sd.viscosity << sd.vbar20 << sd.vbar << ds->compress;

      US_Math2::data_correction( ds->temperature, sd );

      ds->s20w_correction = sd.s20w_correction;
      ds->D20w_correction = sd.D20w_correction;
if ( my_rank == 0 )
 DbgLv(0) << "s20w_correction/D20w_correction"
  << sd.s20w_correction << sd.D20w_correction;

      // Set up simulation parameters for the data set
 
      int nssp            = ds->simparams.speed_step.count();
      bool incl_speed     = ( nssp < 1 );
      ds->simparams.initFromData( NULL, *edata, incl_speed );
      // Use bottom from edited data if it is given
      double bottom_ds    = ( edata->bottom > 0.0 ) ? edata->bottom
                            : ds->centerpiece_bottom;
if ( my_rank == 0 )
 DbgLv(0) << "incl_speed" << incl_speed << "nssp" << nssp
  << "bottom_ds bottom_cp" << bottom_ds << ds->centerpiece_bottom;
 
      if ( ! incl_speed &&  ! redo_ss )
      {  // If experiment speed step used, test against data set times
         int stm1            = ds->simparams.speed_step[        0 ].time_first;
         int stm2            = ds->simparams.speed_step[ nssp - 1 ].time_last;
         int nesc            = edata->scanData.size();
         int etm1            = edata->scanData[        0 ].seconds;
         int etm2            = edata->scanData[ nesc - 1 ].seconds;
if ( my_rank == 0 )
 DbgLv(0) << "etm1 stm1" << etm1 << stm1 << "stm2 etm2" << stm2 << etm2;

         if ( etm1 < stm1  ||  etm2 > stm2 )
         {  // If data time range outside that of speed step, flag it
            redo_ss             = true;
         }
      }

      ds->simparams.rotorcoeffs[ 0 ]  = ds->rotor_stretch[ 0 ];
      ds->simparams.rotorcoeffs[ 1 ]  = ds->rotor_stretch[ 1 ];
      ds->simparams.bottom_position   = ds->centerpiece_bottom;
      ds->simparams.bottom            = bottom_ds;
      ds->simparams.band_forming      = ds->simparams.band_volume != 0.0;
      US_SolveSim::DataSet*  ds0      = data_sets[ 0 ];

      // If we have a timestate file set, add to simparams
      if ( ! ds->tmst_file.isEmpty() )
      {  // Dataset has non-empty timestate file string
         ntmsf_ne++;
         QString tmst_fpath = "../" + ds->tmst_file;
         QFileInfo check_file( tmst_fpath );

         if ( ( check_file.exists() )  &&  ( check_file.isFile() ) )
         {  // Dataset timestate file exists
            ntmsf_ex++;
            US_DataIO::RawData simdat;
            US_AstfemMath::initSimData( simdat, *edata, 0.0 );
            if ( US_AstfemMath::timestate_onesec( tmst_fpath, simdat ) )
            {  // Timestate file is at 1-second-interval
               ntmsf_1s++;
            }
            else
            {  // Existing timestate file is not at 1-second-interval
            }
         }
      }
      else
      {  // Dataset's timestate file string is empty
         if ( ee > 0  &&  ! ds0->tmst_file.isEmpty() )
         {  // Dataset 0's timestate file is not empty:  copy
            ds->tmst_file      = ds0->tmst_file;
         }
      }

      // Accumulate total points and set dataset index,points
      int npoints         = edata->scanCount() * edata->pointCount();
      ds_startx << total_points;
      ds_points << npoints;
      total_points       += npoints;
      
//      // Initialize concentrations vector in case of global fit
//      concentrations << 1.0;

      // Accumulate maximum OD for each dataset
      //   and, if global-fit, normalize-scale the data
      double odlim      = edata->ODlimit;
      double odmax      = 0.0;

      for ( int ss = 0; ss < edata->scanCount(); ss++ )
      {
         for ( int rr = 0; rr < edata->pointCount(); rr++ )
         {
            double dval       = edata->value( ss, rr ) * ds_concen;
            odmax             = qMax( odmax, dval );
            edata->setValue( ss, rr, dval );
         }
      }

      odmax             = qMin( odmax, odlim );
DbgLv(2) << "ee" << ee << "odlim odmax" << odlim << odmax;

      maxods << odmax;
   }

   // Determine whether we need to re-do timestate files
   if ( ( ntmsf_ne == 0 )  ||          // No non-empty tmst_files
        ( ntmsf_ex < ntmsf_ne )  ||    //  or not all given exist
        ( ntmsf_1s < ntmsf_ex ) )      //  or not all at 1-second,
      redo_tstate             = true;  // Then need to re-do

   // Sync up after all processes have evaluated timestates
//DbgLv(0) << "rank" << my_rank << ": pre-barrier2 :TM:" << ELAPSED_SECS;
   MPI_Barrier( MPI_COMM_WORLD );
DbgLv(0) << "rank" << my_rank << ": redo_tstate" << redo_tstate
 << "ntmsf_ne ntmsf_ex ntmsf_1s" << ntmsf_ne << ntmsf_ex << ntmsf_1s;
//DbgLv(0) << "rank" << my_rank << ": barrier2 :TM:" << ELAPSED_SECS;

   if ( redo_tstate )
   {  // We need to re-do the timestate files

      if ( my_rank == 0 )
      { // Do any writing or re-writing of timestates in master process
         QStringList tmsf_fs;

         bool is_sim             = ( data_sets[ 0 ]->run_data.channel == "S" );
DbgLv(0) << "rank" << my_rank << ": is_sim" << is_sim << "chan"
 << data_sets[0]->run_data.channel;

         for ( int ee = 0; ee < count_datasets; ee++ )
         {  // Examine each data set for associated timestate
            US_SolveSim::DataSet*  ds    = data_sets[ ee ];
            ds->simparams.sim  = is_sim;
            QString tmst_fpath = "../" + ds->tmst_file;
            QString tmst_fdefs = QString( tmst_fpath ).replace( ".tmst", ".xml" );
            QString tmst_fpsav = tmst_fpath + "-orig";
            QString tmst_fdsav = tmst_fdefs + "-orig";

            if ( ntmsf_ne == 0 )
            {  // No tmst_file given:  create one
               tmst_fpath         = "../" + ds->run_data.runID + ".time_state.tmst";
               tmst_fdefs         = QString( tmst_fpath ).replace( ".tmst", ".xml" );

               if ( ee == 0 )
               {  // First (only) dataset:  create a tmst file set
                  US_DataIO::RawData simdat;
                  US_AstfemMath::initSimData( simdat, ds->run_data, 0.0 );
                  US_AstfemMath::writetimestate( tmst_fpath, ds->simparams, simdat );
                  tmsf_fs << tmst_fpath;
               }

               else if ( ! tmsf_fs.contains( tmst_fpath ) )
               {  // Not-first dataset, but unique file:  create another file set
                  US_DataIO::RawData simdat;
                  US_AstfemMath::initSimData( simdat, ds->run_data, 0.0 );
                  US_AstfemMath::writetimestate( tmst_fpath, ds->simparams, simdat );
                  tmsf_fs << tmst_fpath;
               }

               ds->tmst_file      = tmst_fpath;   // Save the used timestate file set
DbgLv(0) << "rank" << my_rank << ": ee" << ee << "(NE)tmst_file" << ds->tmst_file;
            }

            else if ( ntmsf_ex == 1  &&  ntmsf_1s == 0 )
            {  // Existing tmst_file given, but not at 1-second interval
               tmst_fpsav         = tmst_fpath + "-orig";
               tmst_fdsav         = tmst_fdefs + "-orig";
               QString tmst_fpold = tmst_fpath;
               QString tmst_fdold = tmst_fdefs;
               tmst_fpath         = "../" + ds->run_data.runID + ".time_state.tmst";
               tmst_fdefs         = QString( tmst_fpath ).replace( ".tmst", ".xml" );

               if ( ee == 0 )
               {
                  // Rename existing (non-1sec-intv) files
                  QFile::rename( tmst_fpold, tmst_fpsav );
                  QFile::rename( tmst_fdold, tmst_fdsav );
                  // Create a new 1-second-interval file set
                  US_DataIO::RawData simdat;
                  US_AstfemMath::initSimData( simdat, ds->run_data, 0.0 );
                  US_AstfemMath::writetimestate( tmst_fpath, ds->simparams, simdat );
                  tmsf_fs << tmst_fpath;
               }

               ds->tmst_file      = tmst_fpath;   // Save the used timestate file set
DbgLv(0) << "rank" << my_rank << ": ee" << ee << "(E1)tmst_file" << ds->tmst_file;
            }

            else if ( ntmsf_ex > 1  &&  ntmsf_1s < ntmsf_ex )
            {  // Existing tmst_files given, but not all at 1-second interval
               tmst_fpsav         = tmst_fpath + "-orig";
               tmst_fdsav         = tmst_fdefs + "-orig";
               QString tmst_fpold = tmst_fpath;
               QString tmst_fdold = tmst_fdefs;
               tmst_fpath         = "../" + ds->run_data.runID + ".time_state.tmst";
               tmst_fdefs         = QString( tmst_fpath ).replace( ".tmst", ".xml" );

               if ( ! tmsf_fs.contains( tmst_fpath )  &&
                    ! QFile( tmst_fpath ).exists() )
               {  // Unique data set timestate file that does not yet exist
                  QFile::rename( tmst_fpold, tmst_fpsav );
                  QFile::rename( tmst_fdold, tmst_fdsav );
                  // Create a new 1-second-interval file set
                  US_DataIO::RawData simdat;
                  US_AstfemMath::initSimData( simdat, ds->run_data, 0.0 );
                  US_AstfemMath::writetimestate( tmst_fpath, ds->simparams, simdat );
                  tmsf_fs << tmst_fpath;
               }

               ds->tmst_file      = tmst_fpath;   // Save the used timestate file set
DbgLv(0) << "rank" << my_rank << ": ee" << ee << "(EX)tmst_file" << ds->tmst_file;
            }

            ds->simparams.simSpeedsFromTimeState( tmst_fpath );
         }
      }
//DbgLv(0) << "rank" << my_rank << ": pre-barrier3 :TM:" << ELAPSED_SECS;

//      // Get all processes synched up, then fix non-master processes
      // Get all processes synched up, then build timestate objects
      MPI_Barrier( MPI_COMM_WORLD );
//DbgLv(0) << "rank" << my_rank << ": barrier3 :TM:" << ELAPSED_SECS;
   }  // END:  Re-do timestate files

   // Load dataset timestate(s)  [ and maybe adjust bottom ]

   for ( int ee = 0; ee < count_datasets; ee++ )
   {
      US_SolveSim::DataSet*  ds    = data_sets[ ee ];
      ds->tmst_file      = ds->tmst_file.isEmpty() ?
                           "../" + ds->run_data.runID + ".time_state.tmst" :
                           ( ds->tmst_file.startsWith( "../" ) ?
                             ds->tmst_file : "../" + ds->tmst_file );

      if ( QFile( ds->tmst_file ).exists() )
      {
DbgLv(0) << "rank" << my_rank << ": ee" << ee << "   tmst UPLOADED";
         // Build simulation speed profile from time state
         ds->simparams.simSpeedsFromTimeState( ds->tmst_file );
         // Build speed steps from simulation speed profile
         ds->simparams.speedstepsFromSSprof();
      }
DbgLv(0) << "rank" << my_rank << ": ee" << ee << "tmst_file" << ds->tmst_file
 << "ssp count" << ds->simparams.sim_speed_prof.count();

#if 0
      // If EditedData has a bottom, use that to adjust bottom_position
      US_DataIO::EditedData* edata = &ds->run_data;

      if ( edata->bottom > 0.0 )
      {
         // Compute speed-adjusted bottom and compare that to edit value
         double bottom_pos  = ds->simparams.bottom_position;
         double speed       = edata->scanData[ 0 ].rpm;
         double bottom_clc  = US_AstfemMath::calc_bottom( speed, bottom_pos,
                                  ds->simparams.rotorcoeffs );
         if ( bottom_clc != edata->bottom )
         {  // Calculated and dataset bottoms differ:
            // We change the centerpiece unstretched bottom position so
            // that Astfem_Rsa's adjust_limits() function arrives at a
            // post-stretch bottom value equal to what is in the EditedData
            ds->simparams.bottom_position += ( edata->bottom - bottom_clc );
if ( my_rank == 0 )
 DbgLv(0) << "rank" << my_rank << ": ee" << ee << "bottom_pos" << bottom_pos
  << "speed" << speed << "bottom_clc bottom_ds" << bottom_clc << edata->bottom
  << "adj.bottom_pos" << ds->simparams.bottom_position;
         }
      }
#endif
   }  // END: datasets loop to upload timestate
//DbgLv(0) << "rank" << my_rank << ": simSpeed :TM:" << ELAPSED_SECS;
//   }
if ( my_rank == 0 )
 DbgLv(0) << "rank" << my_rank << ": redo_ss" << redo_ss;

   if ( redo_ss )
   {  // If speed step re-do flagged, set all speed steps from data
      for ( int ee = 0; ee < data_sets.size(); ee++ )
      {
         US_SolveSim::DataSet*  ds    = data_sets[ ee ];

         ds->simparams.computeSpeedSteps( &ds->run_data.scanData,
                                          ds->simparams.speed_step );
if ( my_rank == 0 )
 DbgLv(0) << "rank" << my_rank << ": ee" << ee << "speed_step RE-DONE from data";
int nssp= ds->simparams.speed_step.count();
int stm1= ds->simparams.speed_step[     0].time_first;
int stm2= ds->simparams.speed_step[nssp-1].time_last;
if ( my_rank == 0 ) {
 DbgLv(0) << my_rank << ": stm1" << stm1 << "stm2" << stm2;
 ds->simparams.debug(); }
      }
   }

   // If debug text modifies SetSpeedLowA value, apply it
   int lo_ss_acc = 250;
   QStringList dbgtxt = US_Settings::debug_text();
   for ( int ii = 0; ii < dbgtxt.count(); ii++ )
   {
      if ( dbgtxt[ ii ].startsWith( "SetSpeedLowA=" ) )
      {
         lo_ss_acc        = QString( dbgtxt[ ii ] ).section( "=", 1, 1 ).toInt();
         if ( my_rank == 0 ){
            DbgLv(1) << "DSM: SetSpeedLowA:    SetSpeedLowA" << lo_ss_acc;
         }
      }
   }

   // Check for low acceleration in every dataset
   for ( int ee = 0; ee < data_sets.size(); ee++ )
   {
      US_SolveSim::DataSet*  dset    = data_sets[ ee ];
      // Do a quick test of the speed step implied by TimeState
      int tf_scan   = dset->simparams.speed_step[ 0 ].time_first;
      int accel1    = dset->simparams.speed_step[ 0 ].acceleration;
      int rspeed    = dset->simparams.speed_step[ 0 ].rotorspeed;
      int tf_aend   = ( rspeed + accel1 - 1 ) / ( accel1 == 0 ? 1 : accel1 );
      int accel2    = dset->simparams.sim_speed_prof[ 0 ].acceleration;
      if ( my_rank == 0 ){
         DbgLv(1) << "DSM: ssck: rspeed accel1 tf_aend tf_scan"
                 << rspeed << accel1 << tf_aend << tf_scan
                 << "accel2" << accel2 << "lo_ss_acc" << lo_ss_acc;
      }
      if ( accel1 < lo_ss_acc  ||  tf_aend > ( tf_scan - 3 ) )
      {
         DbgLv(0) << "rank: " << my_rank << "  Dataset " << ee << " Name: " << dset->run_data.runID <<
                  " likely bad Timestate. Implied acceleration: " << accel1 <<
                  " accel end: " << tf_aend << "s. First scan: " << tf_scan << "s.";
      }
   }

   double  s_max = parameters[ "s_max" ].toDouble() * 1.0e-13;
   double  x_max = parameters[ "x_max" ].toDouble() * 1.0e-13;
   s_max         = ( s_max == 0.0 ) ? x_max : s_max;
   double  y_max = 1e-39;

   // Check GA buckets
   if ( analysis_type == "GA" )
   {
      if ( buckets.size() < 1 )
         abort( "No buckets defined" );

      QList< QRectF > bucket_rects;
      x_max             = buckets[ 0 ].x_max;

      // Put into Qt rectangles (upper left, lower right points)
      for ( int i = 0; i < buckets.size(); i++ )
      {
         limitBucket( buckets[ i ] );

         bucket_rects << QRectF( 
               QPointF( buckets[ i ].x_min, buckets[ i ].y_max ),
               QPointF( buckets[ i ].x_max, buckets[ i ].y_min ) );

         x_max             = qMax( x_max, buckets[ i ].x_max );
         y_max             = qMax( y_max, buckets[ i ].y_max );
      }

      x_max            *= 1.0e-13;

      for ( int i = 0; i < bucket_rects.size() - 1; i++ )
      {
         for ( int j = i + 1; j < bucket_rects.size(); j++ )
         {
            if ( bucket_rects[ i ].intersects( bucket_rects[ j ] ) )
            {
               QRectF bukov = bucket_rects[i].intersected( bucket_rects[j] );
               double sdif  = bukov.width();
               double fdif  = bukov.height();

               if ( my_rank == 0 )
               {
                  DbgLv(0) << "Bucket" << i << "overlaps bucket" << j;
                  DbgLv(0) << " Overlap: st w h" << bukov.topLeft()
                    << sdif << fdif;
                  DbgLv(0) << "  Bucket i" << bucket_rects[i].topLeft()
                    << bucket_rects[i].bottomRight();
                  DbgLv(0) << "  Bucket j" << bucket_rects[j].topLeft()
                    << bucket_rects[j].bottomRight();
                  DbgLv(0) << "  Bucket i right "
                    << QString().sprintf( "%22.18f", bucket_rects[i].right() );
                  DbgLv(0) << "  Bucket j left  "
                    << QString().sprintf( "%22.18f", bucket_rects[j].left() );
                  DbgLv(0) << "  Bucket i bottom"
                    << QString().sprintf( "%22.18f", bucket_rects[i].bottom() );
                  DbgLv(0) << "  Bucket j top   "
                    << QString().sprintf( "%22.18f", bucket_rects[j].top() );
               }

               if ( qMin( sdif, fdif ) < 1.e-6 )
               { // Ignore the overlap if it is trivial
                  if ( my_rank == 0 )
                     DbgLv(0) << "Trivial overlap is ignored.";
                  continue;
               }

               abort( "Buckets overlap" );
            }
         }
      }

      s_max         = x_max;            // By default s_max is x_max for GA
   }

   // Do a check of implied grid size
   QString smsg;
   double  zval     = parameters[ "bucket_fixed" ].toDouble();
   if ( analysis_type.startsWith( "PCSA" ) )
      zval             = parameters[ "z_value" ].toDouble();

   if ( attr_x != ATTR_S )
   {  // We will need to determine the maximum s value
      if ( attr_y == ATTR_S )           // s_max may be y_max
         s_max         = y_max * 1.0e-13;
      else if ( attr_z == ATTR_S )      // s_max may be fixed value
         s_max         = zval  * 1.0e-13;
      else if ( analysis_type == "GA" )
      {                                 // s_max may need computing
         s_max         = 1e-39;
         zval          = ( attr_z == ATTR_V ) ? data_sets[ 0 ]->vbar20 : zval;
         zval          = ( zval == 0.0      ) ? data_sets[ 0 ]->vbar20 : zval;
         US_Model::SimulationComponent zcomp;
         zcomp.s       = 0.0;
         zcomp.f_f0    = 0.0;
         zcomp.mw      = 0.0;
         zcomp.vbar20  = 0.0;
         zcomp.D       = 0.0;
         zcomp.f       = 0.0;

         for ( int jj = 0; jj < buckets.size(); jj++ )
         {  // Compute s using given x,y,fixed and find the max
            double xval   = buckets[ jj ].x_max;
            double yval   = buckets[ jj ].y_max;
            US_Model::SimulationComponent mcomp = zcomp;
            set_comp_attrib( mcomp, xval, attr_x );
            set_comp_attrib( mcomp, yval, attr_y );
            set_comp_attrib( mcomp, zval, attr_z );
            US_Model::calc_coefficients( mcomp );
if (my_rank==0) DbgLv(0) << "ckGrSz:  buck" << jj << "xv yv zv"
 << xval << yval << zval << "mc s k w v d f"
 << mcomp.s << mcomp.f_f0 << mcomp.mw << mcomp.vbar20 << mcomp.D << mcomp.f;
            s_max         = qMax( s_max, mcomp.s );
         }
      }

   }

   s_max         = ( s_max == 0.0 ) ? 10e-13 : s_max;
if (my_rank==0) DbgLv(0) << "ckGrSz: s_max" << s_max << "attr_x,y,z"
 << attr_x << attr_y << attr_z;

   bool g_too_big  = false;
   if ( data_sets[ 0 ]->simparams.sim_speed_prof.count() < 2 )
        g_too_big  = US_SolveSim::checkGridSize( data_sets, s_max, smsg );
if (my_rank==0) DbgLv(0) << "ckGrSz: ssp count"
 << data_sets[0]->simparams.sim_speed_prof.count() << "g_too_big" << g_too_big;
   if ( g_too_big )
   {
      if ( my_rank == 0 )
      {
         qDebug() << smsg;
         printf( smsg.toLatin1().data() );
      }
      abort( "Implied Grid Size is Too Large!" );
   }
//else
// qDebug() << "check_grid_size FALSE  x_max" << x_max
//    << "rpm" << data_sets[0]->simparams.speed_step[0].rotorspeed;

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
   menibott_ndx         = 0;
   meniscus_run         = 0;
   bottom_run           = 0;
   mc_iteration         = 0;

   // Determine masters-group count and related controls
   mgroup_count = task_params[ "mgroupcount" ].toInt();
   max_walltime = task_params[ "walltime"    ].toInt();
   min_experiment_size = task_params[ "subgridsize" ].toInt();

   if ( is_composite_job )
   {
      if ( count_datasets < 2  ||  mgroup_count > ( count_datasets + 2 ) )
         mgroup_count = 1;
   }
   else
   {
      if ( mc_iterations < 2  ||  mgroup_count > ( mc_iterations + 2 ) )
         mgroup_count = 1;
   }

   mgroup_count = qMax( 1, mgroup_count );
   gcores_count = proc_count / mgroup_count;

   if ( mgroup_count < 2 )
      start();                  // Start standard job
   
   else if ( is_composite_job )
      pm_cjobs_start();         // Start parallel-masters job (composite)

   else
      pmasters_start();         // Start parallel-masters job (mc_iters)
}

// Main function  (single master group)
void US_MPI_Analysis::start( void )
{
   my_communicator         = MPI_COMM_WORLD;
   my_workers              = proc_count - 1;
   gcores_count            = proc_count;
   group_rank              = my_rank;

   // Real processing goes here
   if ( analysis_type.startsWith( "2DSA" ) )
   {
      iterations = parameters[ "montecarlo_value" ].toInt();

      if ( iterations < 1 ) iterations = 1;

      if ( my_rank == 0 ) 
          _2dsa_master();
      else
          _2dsa_worker();
   }

   else if ( analysis_type.startsWith( "GA" ) )
   {
      if ( my_rank == 0 ) 
          ga_master();
      else
          ga_worker();
   }

   else if ( analysis_type.startsWith( "DMGA" ) )
   {
      if ( my_rank == 0 ) 
          dmga_master();
      else
          dmga_worker();
   }

   else if ( analysis_type.startsWith( "PCSA" ) )
   {
      if ( my_rank == 0 ) 
          pcsa_master();
      else
          pcsa_worker();
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

      // Build file with run-time statistics
      int  walltime = qRound(
         submitTime.msecsTo( endTime ) / 1000.0 );
      int  cputime  = qRound(
         startTime .msecsTo( endTime ) / 1000.0 );
      int  maxrssmb = qRound( (double)maxrss / 1024.0 );
      int kc_iters  = data_sets.size();

      stats_output( walltime, cputime, maxrssmb,
            submitTime, startTime, endTime );

      // Create archive file of outputs and remove other output files
      update_outputs( true );

      // Send "Finished" message.
      int wt_hr      = walltime / 3600;
      int wt_min     = ( walltime - wt_hr * 3600 ) / 60;
      int wt_sec     = walltime - wt_hr * 3600 - wt_min * 60;
      int ct_hr      = cputime / 3600;
      int ct_min     = ( cputime - ct_hr * 3600 ) / 60;
      int ct_sec     = cputime - ct_hr * 3600 - ct_min * 60;
      printf( "Us_Mpi_Analysis has finished successfully"
              " (Wall=%d:%02d:%02d Cpu=%d:%02d:%02d).\n"
              , wt_hr, wt_min, wt_sec, ct_hr, ct_min, ct_sec );
      fflush( stdout );

      if ( count_datasets < kc_iters )
      {
         send_udp( "Finished:  maxrss " + QString::number( maxrssmb )
               + " MB,  total run seconds " + QString::number( cputime )
               + "  (Reduced Datasets Count)" );
         DbgLv(0) << "Finished:  maxrss " << maxrssmb
                  << "MB,  total run seconds " << cputime
                  << "  (Reduced Datasets Count)";
      }

      else if ( reduced_iter )
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
      msg           = QString( jobid + message ).toLatin1();
      socket->writeDatagram( msg.data(), msg.size(), server, port );
   }

   else if ( group_rank == 0 )
   {  // For pm group master, forward message to supervisor
      int     super = 0;
      QString gpfix = QString( "(pmg %1) " ).arg( my_group );
      msg           = QString( gpfix + message ).toLatin1();
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

// Update and return the maximum memory used
long int US_MPI_Analysis::max_rss( void )
{
   return (long int)US_Memory::rss_max( maxrss );
}

// Send an abort message and do orderly MPI abort
void US_MPI_Analysis::abort( const QString& message, int error )
{
   if ( my_rank == 0 )
   { // Send abort message to both stdout and udp
      US_Sleep::msleep( 1100 );       // Delay a bit so rank 0 completes first
      printf( "\n  ***ABORTED***:  %s\n\n", message.toLatin1().data() );
      fflush( stdout );
      send_udp( "Abort.  " + message );
   }

   MPI_Barrier( MPI_COMM_WORLD );     // Sync everybody up so stdout msg first

   if ( my_rank == 0 )
   {  // Create archive file of outputs and remove other output files
      update_outputs( true );
   }

   DbgLv(0) << my_rank << ": " << message;
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
   {
      DbgLv(0) << "*WARNING* Unable to open job_statistics.xml file for write.";
      return;
   }

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
   if ( buk.x_min > 0.0 )
   {  // All-positive s's start at 0.1 at least (other attribs are different)
      double xmin = 0.1;
      xmin        = ( attr_x == ATTR_K ) ? 1.0   : xmin;
      xmin        = ( attr_x == ATTR_V ) ? 0.01  : xmin;
      xmin        = ( attr_x == ATTR_D ) ? 1.e-9 : xmin;
      xmin        = ( attr_x == ATTR_F ) ? 1.e-9 : xmin;
      double xinc = xmin / 1000.0;
      buk.x_min   = qMax( xmin, buk.x_min );
      buk.x_max   = qMax( ( buk.x_min + xinc ), buk.x_max );
   }

   else if ( buk.x_max <= 0.0 )
   {  // All-negative s's end at -0.1 at most
      buk.x_max   = qMin( -0.1, buk.x_max );
      buk.x_min   = qMin( ( buk.x_max - 0.0001 ), buk.x_min );
   }

   else if ( ( buk.x_min + buk.x_max ) >= 0.0 )
   {  // Mostly positive clipped to all positive starting at 0.1
      buk.x_min   = 0.1;
      buk.x_max   = qMax( 0.2, buk.x_max );
   }

   else
   {  // Mostly negative clipped to all negative ending at -0.1
      buk.x_min   = qMin( -0.2, buk.x_min );
      buk.x_max   = -0.1;
   }

   if ( attr_y == ATTR_K )
   {  // If y-type is "ff0", insure minimum is at least 1.0
      buk.y_min   = qMax(  1.0, buk.y_min );
      buk.y_max   = qMax( ( buk.y_min + 0.0001 ), buk.y_max );
   }
}

// Get the A,b matrices for a data set
void US_MPI_Analysis::dset_matrices( int dsx, int* nsolutesP,
      QVector< double >& nnls_a, QVector< double >& nnls_b,
      QVector< int >& c_used )
{
   double norm_cut = 1.0;

   // If debug text modifies norm_cut value, apply it
   QStringList dbgtxt = US_Settings::debug_text();
   for ( int ii = 0; ii < dbgtxt.count(); ii++ )
   {
      if ( dbgtxt[ ii ].startsWith( "normCutoff=" ) )
      {
         norm_cut        = QString( dbgtxt[ ii ] ).section( "=", 1, 1 ).toDouble();
DbgLv(0) << "DSM: NORM_CUT:    norm_cut" << norm_cut;
      }
   }

   int nsolutes    = *nsolutesP;            // Solutes (columns)
   int ksolutes    = nsolutes;              // Output solutes count
   int rowx        = ds_startx[ dsx ];      // Initial dataset row index
   int ndspts      = ds_points[ dsx ];      // Number dataset points (rows)
   double concen   = concentrations[ dsx ]; // Dataset concentration
   int ka          = 0;                     // Output A index
   int kc          = 0;                     // Column-used index
   int jj          = rowx;                  // Global matrix index
   int inccx       = total_points - ndspts; // Next-column increment
DbgLv(0) << "DSM: dsx" << dsx << "rowx ndspts" << rowx << ndspts
 << "nsolutes" << nsolutes << "concen" << concen << "inccx" << inccx;

   // Copy the data set portion of the global A matrix
   for ( int cc = 0; cc < nsolutes; cc++ )
   {
      int kas         = ka;      // Save column start index
      double norm_a   = 0.0;

      for ( int pp = 0; pp < ndspts; pp++, ka++, jj++ )
      {  // Copy A column and accumulate column norm
         nnls_a[ ka ]    = gl_nnls_a[ jj ];
         norm_a         += sq( nnls_a[ ka ] );
      }

      norm_a          = sqrt( norm_a );  // Complete norm calculation

      if ( norm_a < norm_cut )           // Test norm tolerance
      {  // This column should be excluded
         ksolutes--;             // Decrement column count
         ka              = kas;  // Set to overwrite this column
DbgLv(0) << "DSM: NORM_CUT:  norm_a" << norm_a << "ksol" << ksolutes;
      }

      else
      {  // This column is used
         c_used[ kc++ ]  = cc;   // Save original column index
      }

      jj             += inccx;   // Bump to start of next column`
   }

   if ( ksolutes < nsolutes )
   {  // Norm tolerance has excluded some columns
      nnls_a.resize( ksolutes * ndspts ); // Resize A matrix
      *nsolutesP      = ksolutes;         // Save new solutes count
DbgLv(0) << "DSM:   ASIZE" << nnls_a.size() << "c_used[n]" << c_used[kc-1];
   }

   // Copy and restore scaling for data set portion of the global b matrix
   jj              = rowx;

   for ( int kk = 0; kk < ndspts; kk++, jj++ )
   {
      nnls_b[ kk ]    = gl_nnls_b[ jj ] * concen;
   }
}


// Calculate residuals (FE Modeling and NNLS)
void US_MPI_Analysis::calc_residuals( int         offset,
                                      int         dataset_count,
                                      SIMULATION& simu_values )
{
   count_calc_residuals++;
bool do_dbg=( dbg_level > 0 && ( group_rank < 2 || group_rank == (proc_count/2) ) );
if ( do_dbg )
 DbgLv(1) << "w:" << my_rank << ": CALC_RES : count" << count_calc_residuals
  << "offs dsknt" << offset << dataset_count;

   US_SolveSim solvesim( data_sets, my_rank, false );

//*DEBUG*
int dbglvsv=simu_values.dbg_level;
simu_values.dbg_level=(dbglvsv>1||my_rank<0)?dbglvsv:0;
//simu_values.dbg_level=(dbglvsv>0)?dbglvsv:0;
int nsoli=simu_values.solutes.size();
int nsolz=simu_values.zsolutes.size();
if ( do_dbg ) DbgLv(1) << "w:" << my_rank << ":nsoli" << nsoli << "nsolz" << nsolz;
QVector< US_Solute > isols;
if (nsolz==0)
 { isols = simu_values.solutes; }
else
 { nsoli=nsolz; }
if ( do_dbg ) DbgLv(1) << "w:" << my_rank << ":nsoli" << nsoli << "nsolz" << nsolz;
if ( do_dbg ) simu_values.dbg_level = qMax( simu_values.dbg_level, 1 );
//*DEBUG*

   solvesim.calc_residuals( offset, dataset_count, simu_values );

//*DEBUG*
simu_values.dbg_level=dbglvsv;
bool hicee = false;
for (int jj=0;jj<simu_values.solutes.size();jj++ )
 if ( simu_values.solutes[jj].c > 1.0 ) hicee = true;

//if ( do_dbg )
if ( do_dbg || hicee )
{
 DbgLv(1) << "w:" << my_rank << ": ss.ca_re completed";
 int nsolo=simu_values.solutes.size();
 nsolo = (nsolz>0) ? simu_values.zsolutes.size() : nsolo;
 US_DataIO::EditedData* edat = &data_sets[offset]->run_data;
 US_SolveSim::DataSet*  dset = data_sets[offset];
 US_DataIO::RawData*    sdat = &simu_values.sim_data;
 int nsc = edat->scanCount();
 int nrp = edat->pointCount();
 double d0 = edat->scanData[0].rvalues[0];
 double d1 = edat->scanData[0].rvalues[1];
 double dh = edat->scanData[nsc/2].rvalues[nrp/2];
 double dm = edat->scanData[nsc-1].rvalues[nrp-2];
 double dn = edat->scanData[nsc-1].rvalues[nrp-1];
 DbgLv(1) << "w:" << my_rank << ":d(01hmn)" << d0 << d1 << dh << dm << dn;
 double dt = 0.0;
 for ( int ss=0;ss<nsc;ss++ )
  for ( int rr=0;rr<nrp;rr++ ) dt += edat->scanData[ss].rvalues[rr];
 DbgLv(1) << "w:" << my_rank << ":dtot" << dt;
 double s0 = sdat->value(0,0);
 double s1 = sdat->value(0,1);
 double sh = sdat->value(nsc/2,nrp/2);
 double sm = sdat->value(nsc-1,nrp-2);
 double sn = sdat->value(nsc-1,nrp-1);
 DbgLv(1) << "w:" << my_rank << ": s(01hmn)" << s0 << s1 << sh << sm << sn;
 if ( dataset_count > 1  &&  (offset+1) < data_sets.size() ) {
  edat = &data_sets[offset+1]->run_data;
  int nxx = nsc;
  nsc = edat->scanCount();
  nrp = edat->pointCount();
  d0 = edat->scanData[0].rvalues[0];
  d1 = edat->scanData[0].rvalues[1];
  dh = edat->scanData[nsc/2].rvalues[nrp/2];
  dm = edat->scanData[nsc-1].rvalues[nrp-2];
  dn = edat->scanData[nsc-1].rvalues[nrp-1];
  DbgLv(1) << "w:" << my_rank << ":d2(01hmn)" << d0 << d1 << dh << dm << dn;
  dt = 0.0;
  for ( int ss=0;ss<nsc;ss++ )
   for ( int rr=0;rr<nrp;rr++ ) dt += edat->scanData[ss].rvalues[rr];
  DbgLv(1) << "w:" << my_rank << ":dtot" << dt;
  s0 = sdat->value(nxx+0,0);
  s1 = sdat->value(nxx+0,1);
  sh = sdat->value(nxx+nsc/2,nrp/2);
  sm = sdat->value(nxx+nsc-1,nrp-2);
  sn = sdat->value(nxx+nsc-1,nrp-1);
  DbgLv(1) << "w:" << my_rank << ": s2(01hmn)" << s0 << s1 << sh << sm << sn;
 }
 DbgLv(1) << "w:" << my_rank << ":  nsoli nsolo" << nsoli << nsolo;
 DbgLv(1) << "w:" << my_rank << ":  simpt men bott temp coef1"
  << dset->simparams.simpoints
  << dset->simparams.meniscus
  << dset->simparams.bottom
  << dset->simparams.temperature
  << dset->simparams.rotorcoeffs[ 0 ];
 DbgLv(1) << "w:" << my_rank << ":  vbar soltype manual visc dens"
  << dset->vbar20 << dset->solute_type << dset->manual
  << dset->viscosity << dset->density;
 DbgLv(1) << "w:" << my_rank << ":  noisf alpha" << simu_values.noisflag
  << simu_values.alpha << "s20w_c D20w_c vbar" << dset->s20w_correction
  << dset->D20w_correction << dset->vbar20;
 if ( dataset_count > 1  &&  (offset+1) < data_sets.size() ) {
  dset = data_sets[offset+1];
  DbgLv(1) << "w:" << my_rank << ":  2)simpt men bott temp coef1"
   << dset->simparams.simpoints
   << dset->simparams.meniscus
   << dset->simparams.bottom
   << dset->simparams.temperature
   << dset->simparams.rotorcoeffs[ 0 ];
  DbgLv(1) << "w:" << my_rank << ":  2)vbar soltype manual visc dens"
   << dset->vbar20 << dset->solute_type << dset->manual
   << dset->viscosity << dset->density;
  DbgLv(1) << "w:" << my_rank << ":  2)noisf alpha" << simu_values.noisflag
   << simu_values.alpha << "s20w_c D20w_c vbar" << dset->s20w_correction
   << dset->D20w_correction << dset->vbar20;
 }
 int nn = isols.size() - 1;
 int mm = nn/2;
 if ( nn > mm )
 DbgLv(1) << "w:" << my_rank << ": sol0 solm soln"
  << isols[0].s << isols[0].k << isols[0].c
  << isols[mm].s << isols[mm].k << isols[mm].c
  << isols[nn].s << isols[nn].k << isols[nn].c;
}
//*DEBUG*
 
}

// Write model (and maybe noise) output at the end of an iteration
void US_MPI_Analysis::write_output( void )
{
   US_SolveSim::Simulation sim = simulation_values;

   double save_meniscus = meniscus_value;
   double save_bottom   = bottom_value;
   US_Model::AnalysisType mdl_type = model_type( analysis_type );
   int mxdssz   = -1;

   if ( mdl_type == US_Model::TWODSA  ||  mdl_type == US_Model::GA )
   {
      sim.solutes  = calculated_solutes[ max_depth ]; 
      mxdssz       = sim.solutes.size();
   }

   else if ( mdl_type == US_Model::DMGA )
   {  // Handle DMGA need for dummy solutes
      QVector< US_Solute > solvec;
      max_depth    = 0;
      calculated_solutes.clear();
      calculated_solutes << solvec;
      sim.solutes  = solvec;
DbgLv(1) << "MAST: wrout: mdl_type DMGA";
   }

   else if ( mdl_type == US_Model::PCSA )
   {  // PCSA: Order model records and pick best model
      max_depth    = 0;
      qSort( mrecs );
//*DEBUG*
DbgLv(1) << "MAST: wrout: mdl_type PCSA  mrecs size" << mrecs.size();
if(dbg_level>0)
{
 for(int jj=0;jj<mrecs.size();jj++)
 {
  DbgLv(1) << "M:wo: jj" << jj << "typ tx" << mrecs[jj].ctype << mrecs[jj].taskx << "isz csz"
   << mrecs[jj].isolutes.size() << mrecs[jj].csolutes.size() << "rmsd" << mrecs[jj].rmsd
   << "sy ey" << mrecs[jj].str_y << mrecs[jj].end_y
   << "p1 p2" << mrecs[jj].par1 << mrecs[jj].par2;
 }
}
//*DEBUG*
      sim.zsolutes = mrecs[ 0 ].csolutes;
      mxdssz       = sim.zsolutes.size();
      sim.ti_noise = mrecs[ 0 ].ti_noise;
      sim.ri_noise = mrecs[ 0 ].ri_noise;
   }
DbgLv(1) << "WrO: mxdssz" << mxdssz;

   if ( mxdssz == 0 )
   { // Handle the case of a zero-solute final model
      int simssz   = simulation_values.zsolutes.size();
      int dm1ssz   = max_depth > 0 ?
                     calculated_solutes[ max_depth - 1 ].size() : 0;
      DbgLv( 0 ) << "*WARNING* Final solutes size" << mxdssz
         << "max_depth" << max_depth << "Sim and Depth-1 solutes size"
         << simssz << dm1ssz;
      if ( simssz > 0 )
      { // Use the last result solutes if there are some
         sim.zsolutes = simulation_values.zsolutes;
         DbgLv( 0 ) << "   SimValues solutes used";
      }
      else if ( dm1ssz > 0 )
      { // Else use the max-depth-minus-1 solutes if there are some
         //sim.zsolutes = calculated_solutes[ max_depth - 1 ];
         DbgLv( 0 ) << "   CalcValue[mxdepth-1] solutes used";
      }
      else
      { // Else report the bad situation of no solutes available
         DbgLv( 0 ) << "   *ERROR* No solutes will be used";
      }
   }

DbgLv(1) << "WrO: meniscus_run" << meniscus_run << "mvsz" << meniscus_values.size();
   meniscus_value  = meniscus_run < meniscus_values.size() 
                     ? meniscus_values[ meniscus_run ] : save_meniscus;
   bottom_value    = bottom_run < bottom_values.size() 
                     ? bottom_values[ bottom_run ] : save_bottom;

   if ( mdl_type == US_Model::PCSA )
   {
DbgLv(1) << "WrO: qSort solutes  sssz" << sim.zsolutes.size();
      qSort( sim.zsolutes );
   }

   else if ( mdl_type != US_Model::DMGA )
   {
DbgLv(1) << "WrO: qSort solutes  sssz" << sim.solutes.size();
      qSort( sim.solutes );
   }

DbgLv(1) << "WrO: wr_model  mdl_type" << mdl_type;
   write_model( sim, mdl_type );
   meniscus_value  = save_meniscus;
   bottom_value    = save_bottom;

DbgLv(1) << "WrO: wr_noise";
   if (  parameters[ "tinoise_option" ].toInt() > 0 )
      write_noise( US_Noise::TI, sim.ti_noise );

   if (  parameters[ "rinoise_option" ].toInt() > 0 )
      write_noise( US_Noise::RI, sim.ri_noise );

DbgLv(1) << "WrO: wr_mrecs";
   if ( mdl_type == US_Model::PCSA )
   {  // Output mrecs for PCSA, if we have a final mrecs vector
      int tikreg   = parameters[ "tikreg_option" ].toInt();
      int mc_iters = parameters[ "mc_iterations" ].toInt();

      if ( tikreg == 0  &&  mc_iters < 2 )
         write_mrecs();
   }
}

// Write global model outputs at the end of an iteration
void US_MPI_Analysis::write_global( void )
{
   US_SolveSim::Simulation  sim      = simulation_values;
                wksim_vals           = simulation_values;
   US_SolveSim::Simulation* gsim     = &simulation_values;
   //US_SolveSim::Simulation* gsim     = &wksim_vals;
   US_Model::AnalysisType   mdl_type = model_type( analysis_type );

   int nsolutes = ( mdl_type != US_Model::DMGA ) ? sim.solutes.size() : -1;
   nsolutes     = ( mdl_type != US_Model::PCSA ) ? nsolutes : sim.zsolutes.size();

DbgLv(0) << "WrGlob: mciter mxdepth" << mc_iteration+1 << max_depth
 << "simvsols size" << nsolutes;
   if ( nsolutes == 0 )
   { // Handle the case of a zero-solute final model
      DbgLv( 0 ) << "   *ERROR* No solutes available for global model";
      return;
   }

   if ( mdl_type == US_Model::TWODSA )
   {
      // 2DSA: Recompute the global fit and save A and b matrices for later use
      US_SolveSim solvesim( data_sets, my_rank, false );
      solvesim.calc_residuals( 0, data_sets.size(), wksim_vals, false,
                               &gl_nnls_a, &gl_nnls_b );
DbgLv(0) << "WrGlob:  glob recompute nsols" << wksim_vals.solutes.size()
 << "globrec A,b sizes" << gl_nnls_a.size() << gl_nnls_b.size();
      nsolutes             = gsim->solutes.size();
      sim.variance         = wksim_vals.variance;

      // Compute the average concentration and output superglobal model
      double avg_conc      = 0.0;
      for ( int ee = 0; ee < data_sets.size(); ee++ )
      {
         avg_conc            += concentrations[ ee ];
         sim.variances[ ee ]  = wksim_vals.variances[ ee ];
      }
      avg_conc            /= (double)( data_sets.size() );

      for ( int cc = 0; cc < nsolutes; cc++ )
      {
         sim.solutes[ cc ].c   = gsim->solutes[ cc ].c * avg_conc;
DbgLv(0) << "WrGlob:     cc" << cc << "conc-in" << gsim->solutes[cc].c
 << "SGconc-out" << sim.solutes[cc].c;
      }

      write_superg( sim, mdl_type );

DbgLv(0) << "WrGlob: dssize" << data_sets.size() << "concsize" << concentrations.size()
 << "simsolssz" << sim.solutes.size() << "gsimsolssz" << gsim->solutes.size() << nsolutes;
      // Build and write scaled global models
      for ( int ee = 0; ee < data_sets.size(); ee++ )
      {
         US_DataIO::EditedData* edata = &data_sets[ ee ]->run_data;
         current_dataset      = ee;
         meniscus_value       = edata->meniscus;
         sim.variance         = sim.variances[ ee ];
         double concentration = concentrations[ ee ];
         nsolutes             = gsim->solutes.size();
DbgLv(0) << "WrGlob:   currds" << ee << "concen" << concentration;

         for ( int cc = 0; cc < nsolutes; cc++ )
         {
            sim.solutes[ cc ].c   = gsim->solutes[ cc ].c * concentration;
DbgLv(0) << "WrGlob:     cc" << cc << "conc-in" << gsim->solutes[cc].c
 << "conc-out" << sim.solutes[cc].c;
         }


DbgLv(0) << "WrGlob:    call write_model(1)";
         // Output the model from global solute points
         write_model( sim, mdl_type, true );

         // Grab dataset portion of A and b, then re-fit
         wksim_vals           = sim;
         wksim_vals.solutes.clear();
         int kscans           = edata->scanCount();
         int kpoints          = edata->pointCount();
         int narows           = kscans * kpoints;
         int navals           = narows * nsolutes;
         int ksolutes         = 0;
         QVector< double > nnls_a( navals,   0.0 );
         QVector< double > nnls_b( narows,   0.0 );
         QVector< double > nnls_x( nsolutes, 0.0 );
         QVector< int >    c_used( nsolutes, 0   );
DbgLv(0) << "WrGlob:    ks kp nar nav" << kscans << kpoints << narows << navals;

         dset_matrices( ee, &nsolutes, nnls_a, nnls_b, c_used );

DbgLv(0) << "WrGlob:    mats built; calling NNLS";
         US_Math2::nnls( nnls_a.data(), narows, narows, nsolutes,
                         nnls_b.data(), nnls_x.data() );

DbgLv(0) << "WrGlob:     building solutes from nnls_x";
         for ( int cc = 0; cc < nsolutes; cc++ )
         {
            double soluval       = nnls_x[ cc ];
            if ( soluval > 0.0 )
            {
               int jcu              = c_used[ cc ];
               US_Solute solu       = sim.solutes[ jcu ];
               solu.c               = soluval;
               wksim_vals.solutes << solu;
               ksolutes++;
if(soluval>1.0) {
 DbgLv(0) << "WrGlob:     SOLUVAL" << soluval << "ee cc" << ee << cc << "ksol" << ksolutes;
 double avlo=1e+100;
 double avhi=0.0;
 double avav=0.0;
 double bvlo=1e+100;
 double bvhi=0.0;
 double bvav=0.0;
 int ja=cc*narows;
 for (int jb=0; jb<narows;jb++ ) {
  double aval=nnls_a[ja++];
  avlo=qMin(avlo,aval);
  avhi=qMax(avhi,aval);
  avav+=aval;
  double bval=nnls_b[jb];
  bvlo=qMin(bvlo,bval);
  bvhi=qMax(bvhi,bval);
  bvav+=bval;
 }
 DbgLv(0) << "WrGlob:      A lo,hi,av" << avlo << avhi << avav
  << "B lo,hi,av" << bvlo << bvhi << bvav;
}
            }
         }
DbgLv(0) << "WrGlob:    currds" << ee << "nsol ksol" << nsolutes << ksolutes;

         // Output the model refitted to individual dataset
         write_model( wksim_vals, mdl_type, false );
      }
   }

   else if ( mdl_type == US_Model::GA )
   {  // GA:  Compute and output each dataset model
//      wksim_vals           = simulation_values;
      US_SolveSim solvesim( data_sets, my_rank, false );
      solvesim.calc_residuals( 0, data_sets.size(), wksim_vals, false,
                               &gl_nnls_a, &gl_nnls_b );
DbgLv(1) << "WrGlob:  glob recompute nsols" << wksim_vals.solutes.size()
 << "globrec A,b sizes" << gl_nnls_a.size() << gl_nnls_b.size();

      // Compute the average concentration and output superglobal model
      nsolutes             = gsim->solutes.size();
//      nsolutes             = wksim_vals.solutes.size();
//      gsim->solutes        = wksim_vals.solutes;
      double avg_conc      = 0.0;
      for ( int ee = 0; ee < data_sets.size(); ee++ )
      {
         avg_conc            += concentrations[ ee ];
      }
      avg_conc            /= (double)( data_sets.size() );
DbgLv(1) << "WrGlob:   avg_conc" << avg_conc;

      for ( int cc = 0; cc < nsolutes; cc++ )
      {
         sim.solutes[ cc ].c   = gsim->solutes[ cc ].c * avg_conc;
DbgLv(1) << "WrGlob:     cc" << cc << "conc-in" << gsim->solutes[cc].c
 << "SGconc-out" << sim.solutes[cc].c;
      }

      write_superg( sim, mdl_type );
DbgLv(1) << "WrGlob: dssize" << data_sets.size() << "concsize" << concentrations.size()
 << "simsolssz" << sim.solutes.size() << "gsimsolssz" << gsim->solutes.size() << nsolutes;

      // Build and write scaled global models
      for ( int ee = 0; ee < data_sets.size(); ee++ )
      {
         US_DataIO::EditedData* edata = &data_sets[ ee ]->run_data;
         current_dataset      = ee;
         meniscus_value       = edata->meniscus;
         sim.variance         = sim.variances[ ee ];
         double concentration = concentrations[ ee ];
DbgLv(1) << "WrGlob:   currds" << ee << "concen" << concentration;

         for ( int cc = 0; cc < nsolutes; cc++ )
         {
            sim.solutes[ cc ].c   = gsim->solutes[ cc ].c * concentration;
DbgLv(1) << "WrGlob:     cc" << cc << "conc-in" << gsim->solutes[cc].c
 << "conc-out" << sim.solutes[cc].c;
         }

         // Output the model from global solute points
         write_model( sim, mdl_type, true );

         // Grab dataset portion of A and b, then re-fit
         wksim_vals           = sim;
         wksim_vals.solutes.clear();
         int kscans           = edata->scanCount();
         int kpoints          = edata->pointCount();
         int narows           = kscans * kpoints;
         int navals           = narows * nsolutes;
         int ksolutes         = 0;
         QVector< double > nnls_a( navals,   0.0 );
         QVector< double > nnls_b( narows,   0.0 );
         QVector< double > nnls_x( nsolutes, 0.0 );
         QVector< int >    c_used( nsolutes, 0   );

         dset_matrices( ee, &nsolutes, nnls_a, nnls_b, c_used );

         US_Math2::nnls( nnls_a.data(), narows, narows, nsolutes,
                         nnls_b.data(), nnls_x.data() );

         for ( int cc = 0; cc < nsolutes; cc++ )
         {
            double soluval       = nnls_x[ cc ];
            if ( soluval > 0.0 )
            {
               int jcu              = c_used[ cc ];
               US_Solute solu       = sim.solutes[ jcu ];
               solu.c               = soluval;
               wksim_vals.solutes << solu;
               ksolutes++;
            }
         }

         // Output the model refitted to the individual dataset
         write_model( wksim_vals, mdl_type, false );
DbgLv(1) << "WrGlob:    currds" << ee << "nsol ksol" << nsolutes << ksolutes;
      }
   }

   else if ( mdl_type == US_Model::PCSA )
   {  // PCSA: Recompute the global fit and save A and b matrices for later use
      US_SolveSim solvesim( data_sets, my_rank, false );
      solvesim.calc_residuals( 0, data_sets.size(), wksim_vals, false,
                               &gl_nnls_a, &gl_nnls_b );
      nsolutes             = gsim->zsolutes.size();
DbgLv(1) << "WrGlob:  glob recompute nzsols" << wksim_vals.zsolutes.size() << nsolutes
 << "globrec A,b sizes" << gl_nnls_a.size() << gl_nnls_b.size();
double dsum_b=0.0;
for (int ii=0;ii<gl_nnls_b.size();ii++) dsum_b += gl_nnls_b[ii];
DbgLv(1) << "WrGlob:   ALL dsum_b" << dsum_b;

      // Compute the average concentration and output superglobal model
      double avg_conc      = 0.0;
      for ( int ee = 0; ee < data_sets.size(); ee++ )
      {
//concentrations[ee] *= 0.5;
         avg_conc            += concentrations[ ee ];
      }
      avg_conc            /= (double)( data_sets.size() );

      for ( int cc = 0; cc < nsolutes; cc++ )
      {
         sim.zsolutes[ cc ].c  = gsim->zsolutes[ cc ].c * avg_conc;
DbgLv(1) << "WrGlob:     cc" << cc << "conc-in" << gsim->zsolutes[cc].c
 << "SGconc-out" << sim.zsolutes[cc].c;
      }

      write_superg( sim, mdl_type );

DbgLv(1) << "WrGlob: dssize" << data_sets.size() << "concsize" << concentrations.size()
 << "simsolssz" << sim.zsolutes.size() << "gsimsolssz" << gsim->zsolutes.size() << nsolutes;
double dsum_b_all=0.0;
      // Build and write scaled global models
      for ( int ee = 0; ee < data_sets.size(); ee++ )
      {
         US_DataIO::EditedData* edata = &data_sets[ ee ]->run_data;
int ks=edata->scanCount();
int kp=edata->pointCount();
DbgLv(1) << "WrGlob:  ee" << ee << "AA-dset(m)" << edata->value(ks/2,kp/2);
         current_dataset      = ee;
         meniscus_value       = edata->meniscus;
         sim.variance         = sim.variances[ ee ];
         double concentration = concentrations[ ee ];
DbgLv(1) << "WrGlob:   currds" << ee << "concen" << concentration;

         for ( int cc = 0; cc < nsolutes; cc++ )
         {
            sim.zsolutes[ cc ].c  = gsim->zsolutes[ cc ].c * concentration;
DbgLv(1) << "WrGlob:     cc" << cc << "conc-in" << gsim->zsolutes[cc].c
 << "conc-out" << sim.zsolutes[cc].c;
         }

DbgLv(1) << "WrGlob:    call write_model(1)";
         // Output the model from global solute points
         write_model( sim, mdl_type, true );

         // Grab dataset portion of A and b, then re-fit
         wksim_vals           = sim;
         wksim_vals.zsolutes.clear();
         int kscans           = edata->scanCount();
         int kpoints          = edata->pointCount();
         int narows           = kscans * kpoints;
         int navals           = narows * nsolutes;
         int ksolutes         = 0;
         QVector< double > nnls_a( navals,   0.0 );
         QVector< double > nnls_b( narows,   0.0 );
         QVector< double > nnls_x( nsolutes, 0.0 );
         QVector< int >    c_used( nsolutes, 0   );
DbgLv(1) << "WrGlob:    ks kp nar nav" << kscans << kpoints << narows << navals;

         dset_matrices( ee, &nsolutes, nnls_a, nnls_b, c_used );
double dsum_b=0.0;
for (int ii=0;ii<narows;ii++) dsum_b += nnls_b[ii];
DbgLv(1) << "WrGlob:      ee" << ee << "dsum_b" << dsum_b
 << "stx pts" << ds_startx[ee] << ds_points[ee] << narows << "conc" << concentrations[ee];
dsum_b_all += dsum_b;

DbgLv(1) << "WrGlob:    mats built; calling NNLS";
         US_Math2::nnls( nnls_a.data(), narows, narows, nsolutes,
                         nnls_b.data(), nnls_x.data() );

DbgLv(1) << "WrGlob:     building solutes from nnls_x";
double sum_sol=0.0;
         for ( int cc = 0; cc < nsolutes; cc++ )
         {
            double soluval       = nnls_x[ cc ];
            if ( soluval > 0.0 )
            {
               int jcu              = c_used[ cc ];
               US_ZSolute solu      = sim.zsolutes[ jcu ];
               solu.c               = soluval;
               wksim_vals.zsolutes << solu;
               ksolutes++;
sum_sol+=soluval;
            }
         }
DbgLv(1) << "WrGlob:    currds" << ee << "nsol ksol" << nsolutes << ksolutes
 << sim.zsolutes.size() << "sum_sol" << sum_sol;

         // Output the model refitted to individual dataset
         write_model( wksim_vals, mdl_type, false );
      }
DbgLv(1) << "WrGlob:     dsum_b_all" << dsum_b_all;
   }

   else if ( mdl_type == US_Model::DMGA )
   {  // DMGA:  Compute and output each dataset model
   }

   current_dataset      = 0;
}

// Shutdown the workers.   Actually this doesn't necessarily shut them down,
// it breaks out of the processing loop.
void US_MPI_Analysis::shutdown_all( void )
{
   MPI_Job job;
   job.command = MPI_Job::SHUTDOWN;
DbgLv(1) << "2dsa master shutdown : master maxrss" << maxrss;
 
   for ( int i = 1; i <= my_workers; i++ )
   {
      MPI_Send( &job, 
         sizeof( job ), 
         MPI_BYTE,
         i,               // Send to each worker
         MPI_Job::MASTER,
         my_communicator );

      maxrss += work_rss[ i ];
DbgLv(1) << "2dsa master shutdown : worker" << i << " upd. maxrss" << maxrss
 << "  wkrss" << work_rss[ i ];
   }
}

// Write model output at the end of an iteration
void US_MPI_Analysis::write_model( const SIMULATION&      sim, 
                                   US_Model::AnalysisType type,
                                   bool                   glob_sols )
{
   US_DataIO::EditedData* edata = &data_sets[ current_dataset ]->run_data;

   // Fill in and write out the model file
   US_Model model;
   int subtype       = ( type == US_Model::PCSA ) ? mrecs[ 0 ].ctype : 0;

DbgLv(1) << "wrMo: type" << type << "(DMGA=" << US_Model::DMGA << ") (PCSA="
 << US_Model::PCSA << ") subtype=" << subtype;
   if ( type == US_Model::DMGA )
   {  // For discrete GA, get the already constructed model
      model             = data_sets[ 0 ]->model;
DbgLv(1) << "wrMo:  model comps" << model.components.size();
   }

   int mc_iter       = ( mgroup_count < 2  ||  is_composite_job ) 
                       ? ( mc_iteration + 1 ) : mc_iteration;
   model.monteCarlo  = mc_iterations > 1;
   model.wavelength  = edata->wavelength.toDouble();
   model.modelGUID   = ( ! model.monteCarlo  ||  mc_iter == 1 )
                       ? US_Util::new_guid() : modelGUID;
DbgLv(1) << "wrMo:  mc mciter mGUID" << model.monteCarlo << mc_iter
 << model.modelGUID;
   model.editGUID    = edata->editGUID;
   model.requestGUID = requestGUID;
   model.dataDescrip = edata->description;
   //model.optics      = ???  How to get this?  Is is needed?
   model.analysis    = type;
   QString runID     = edata->runID;

   if ( fit_menbot )
      model.global      = US_Model::MENIBOTT;

   else if ( fit_meni )
      model.global      = US_Model::MENISCUS;

   else if ( fit_bott )
      model.global      = US_Model::BOTTOM;

   else if ( is_global_fit )
   {
      model.global      = US_Model::GLOBAL;
      subtype          += ( glob_sols ? SUBT_SC : SUBT_VR );
      model.modelGUID   = US_Util::new_guid();
      modelGUID         = model.modelGUID;
   }

   else
      model.global      = US_Model::NONE; 
DbgLv(0) << "wrMo:  is_glob glob_sols" << is_global_fit << glob_sols
 << "f_men f_bot f_mbo" << fit_meni << fit_bott << fit_menbot << "m.glob" << model.global;

   model.meniscus    = meniscus_value;
   model.bottom      = fit_bott ? bottom_value : 0.0;
   model.variance    = sim.variance;

   // demo1_veloc. 1A999. e201101171200_a201101171400_2DSA us3-0000003           .model
   // demo1_veloc. 1A999. e201101171200_a201101171400_2DSA us3-0000003           .ri_noise
   // demo1.veloc. 1A999. e201101171200_a201101171400_2DSA_us3-0000003_i01-m62345.ri_noise
   // demo1_veloc. 1A999. e201101171200_a201101171400_2DSA_us3-0000003_mc001     .model
   // runID.tripleID.analysisID.recordType
   //    analysisID = editID_analysisDate_analysisType_requestID_iterID (underscores)
   //       editID:     
   //       requestID: from lims or 'local' 
   //       analysisType : 2DSA GA others
   //       iterID:       'i01-m62345' for meniscus, mc001 for monte carlo, i01 default 
   //      
   //       recordType: ri_noise, ti_noise, model

   QString tripleID  = edata->cell + edata->channel + edata->wavelength;
   QString dates     = "e" + edata->editID + "_a" + analysisDate;
DbgLv(1) << "wrMo: tripleID" << tripleID << "dates" << dates;
   QString iterID;

   if ( mc_iterations > 1 )
      iterID.sprintf( "mc%04d", mc_iter );
   else if ( fit_menbot )
      iterID.sprintf( "i%02d-m%05db%05d", 
              menibott_ndx + 1,
              (int)( meniscus_value * 10000 ),
              (int)( bottom_value * 10000 ) );
   else if (  fit_meni )
      iterID.sprintf( "i%02d-m%05d", 
              meniscus_run + 1,
              (int)( meniscus_value * 10000 ) );
   else if (  fit_bott )
      iterID.sprintf( "i%02d-b%05d", 
              bottom_run + 1,
              (int)( bottom_value * 10000 ) );
   else
      iterID = "i01";

   QString mdlid     = tripleID + "." + iterID;
   QString id        = model.typeText( subtype );
   if ( analysis_type.contains( "CG" ) )
      id                = id.replace( "2DSA", "2DSA-CG" );
   if ( max_iterations > 1  &&  mc_iterations == 1 )
      id               += "-IT";
   QString analyID   = dates + "_" + id + "_" + requestID + "_" + iterID;
   int     stype     = data_sets[ current_dataset ]->solute_type;
   double  vbar20    = data_sets[ current_dataset ]->vbar20;

   model.description = runID + "." + tripleID + "." + analyID + ".model";
DbgLv(0) << "wrMo: model descr" << model.description;

   // Save as class variable for later reference
   modelGUID         = model.modelGUID;

   if ( type == US_Model::PCSA )
   {  // For PCSA, construct the model from zsolutes
      for ( int ii = 0; ii < sim.zsolutes.size(); ii++ )
      {
         US_ZSolute zsolute = sim.zsolutes[ ii ];

         US_Model::SimulationComponent component;
         US_ZSolute::set_mcomp_values( component, zsolute, stype, true );
         component.name     = QString().sprintf( "SC%04d", ii + 1 );

         US_Model::calc_coefficients( component );
         model.components << component;
      }
   }

   else if ( type != US_Model::DMGA )
   {  // For other non-DMGA, construct the model from solutes
      for ( int ii = 0; ii < sim.solutes.size(); ii++ )
      {
         const US_Solute* solute = &sim.solutes[ ii ];

         US_Model::SimulationComponent component;
         component.s       = solute->s;
         component.f_f0    = solute->k;
         component.name    = QString().sprintf( "SC%04d", ii + 1 );
         // component.vbar20  = (attr_z == ATTR_V) ? vbar20 : solute->v;
         if ( stype == 0) {
            component.vbar20  = vbar20;
         } else {
            component.vbar20  = solute->v;
         }
         component.signal_concentration = solute->c;

         US_Model::calc_coefficients( component );
         model.components << component;
      }
   }
DbgLv(1) << "wrMo: stype" << stype << QString().sprintf("0%o",stype)
 << "attr_z vbar20 mco0.v" << attr_z << vbar20 << model.components[0].vbar20;

   QString fext      = model.monteCarlo ? ".mdl.tmp" : ".model.xml";
   QString fileid    = "." + id + "." + mdlid + fext;
   QString fn        = runID + fileid;
   int lenfn         = fn.length();

   if ( lenfn > 96 )
   { // Insure a model file name less than 100 characters in length (tar limit)
      int lenri         = runID.length() + 96 - lenfn;
      fn                = runID.left( lenri ) + fileid;
   }

   // Output the model to a file
   model.write( fn );

   // Save the model in case needed for noise
   data_sets[ current_dataset ]->model = model;

   // Add the file name of the model file to the output list
   QFile fileo( "analysis_files.txt" );

   if ( ! fileo.open( QIODevice::WriteOnly | QIODevice::Text
                                           | QIODevice::Append ) )
   {
      abort( "Could not open 'analysis_files.txt' for writing" );
      return;
   }

   QTextStream tsout( &fileo );

   QString meniscus = QString::number( meniscus_value, 'e', 4 );
   QString variance = QString::number( sim.variance,   'e', 4 );

   int run     = 1;

   if ( menibott_ndx > 0 ) 
       run        = menibott_ndx + 1;
   else if ( mc_iterations > 0 )
       run        = mc_iter;

   QString runstring = "Run: " + QString::number( run ) + " " + tripleID;

   tsout << fn << ";meniscus_value=" << meniscus_value
               << ";MC_iteration="   << mc_iter
               << ";variance="       << sim.variance
               << ";run="            << runstring
               << ";bottom_value="   << bottom_value
               << "\n";
   fileo.close();
}

// Write noise output at the end of an iteration
void US_MPI_Analysis::write_noise( US_Noise::NoiseType      type, 
                                   const QVector< double >& noise_data )
{
   US_DataIO::EditedData* data = &data_sets[ current_dataset ]->run_data;

   QString  type_name;
   US_Noise noise;

   if ( type == US_Noise::TI ) 
   {
      type_name         = "ti";
      int radii         = data->pointCount();
      noise.minradius   = data->radius( 0 );
      noise.maxradius   = data->radius( radii - 1 );
   }
   else
   {
      type_name = "ri";
   }

   // demo1_veloc.1A999.e201101171200_a201101171400_2DSA us3-0000003
   //    _i01
   //    _mc001
   //    _i01-m62345.ri_noise
   //                        .model
   //                        .ri_noise
   // runID.tripleID.analysID.recordType
   //    analysID = editID_analysisDate_analysisType_requestID_iterID
   //       editID:     
   //       requestID: from lims or 'local' 
   //       analysisType : 2DSA GA others
   //       iterID:       'i01-m62345' for meniscus,
   //                     'mc001' for monte carlo, 'i01' default 
   //      
   //       recordType: ri_noise, ti_noise, model

   QString tripleID   = data->cell + data->channel + data->wavelength;
   QString dates      = "e" + data->editID + "_a" + analysisDate;
   QString anType     = "_" + data_sets[ current_dataset ]->model.typeText()
                        + "_";
   if ( analysis_type.contains( "CG" ) )
      anType             = anType.replace( "2DSA", "2DSA-CG" );

   QString iterID;

   if ( mc_iterations > 1 )           // MonteCarlo iteration
      iterID.sprintf( "mc%04d", mc_iteration + 1 );

   else if ( fit_menbot )             // Meniscus+Bottom fit
      iterID.sprintf( "i%02d-m%05db%05d", 
              menibott_ndx + 1,
              (int)( meniscus_value * 10000 ),
              (int)( bottom_value * 10000 ) );
   else if (  fit_meni )              // Meniscus fit
      iterID.sprintf( "i%02d-m%05d", 
              meniscus_run + 1,
              (int)( meniscus_value * 10000 ) );
   else if (  fit_bott )              // Bottom fit
      iterID.sprintf( "i%02d-b%05d", 
              bottom_run + 1,
              (int)( bottom_value * 10000 ) );

   else                               // Non-iterative single
      iterID = "i01";

   QString analysID  = dates + anType + requestID + "_" + iterID;

   noise.description = data->runID + "." + tripleID + "." + analysID
      + "." + type_name + "_noise";

   noise.type        = type;
   noise.noiseGUID   = US_Util::new_guid();
   noise.modelGUID   = modelGUID;
   noise.values      = noise_data;
   noise.count       = noise_data.size();

   // Add in input noise for associated noise type
   // We are not checking for errors here, because that was checked when
   // the input noise was applied.

   US_Noise         input_noise;
   QList< QString > noise_filenames = data_sets[ current_dataset ]->noise_files;

   for ( int j = 0; j < noise_filenames.size(); j++ )
   {
      QString fn = "../" + noise_filenames[ j ];
      input_noise.load( fn );
      if ( input_noise.type == type ) noise.sum_noise( input_noise );
   }

   QString fn = type_name + ".noise." + noise.noiseGUID + ".xml";
   noise.write( fn );

   // Add the file name of the noise file to the output list
   QFile f( "analysis_files.txt" );
   if ( ! f.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append ) )
   {
      abort( "Could not open 'analysis_files.txt' for writing", -1 );
      return;
   }

   QTextStream out( &f );
   out << fn << "\n";
   f.close();
}

// Find the next available worker that is ready for processing
int US_MPI_Analysis::ready_worker( )
{
   // Find next ready worker by searching status array
   int worker = worker_status.indexOf( READY, worknext );
int w1=worker;
int w1n=worknext;
   worker     = ( worker > 0 ) ? worker :
                worker_status.indexOf( READY, 1 );

   // Set index to start with on next search
   worknext   = ( worker > 0 ) ? ( worker + 1 ) : 1;
   worknext   = ( worknext > my_workers ) ? 1 : worknext;
DbgLv(1) << "ready_worker  w1 w1n worker worknext" << w1 << w1n << worker << worknext;
if(w1<0)
DbgLv(1) << "ready_worker  w1234...wmn"
 << worker_status[1] << worker_status[2] << worker_status[3] << worker_status[4]
 << worker_status[my_workers-1] << worker_status[my_workers];

   return worker;
}

// Update the output TAR file after composite job output has been produced
void US_MPI_Analysis::update_outputs( bool is_final )
{
   // Get a list of output files, minus any TAR file
   QDir odir( "." );
   QStringList files = odir.entryList( QStringList( "*" ), QDir::Files );
DbgLv(0) << my_rank << ": UpdOut : final" << is_final
   << "files size" << files.size();

   if ( files.size() == 1  &&  files[ 0 ] == "analysis-results.tar" )
   {  // If the tar file exists alone, do nothing here
DbgLv(0) << my_rank << ": A single output file, the archive, already exists!!!";
      return;
   }

   // Otherwise, remove the tar file from the list of output files
   files.removeOne( "analysis-results.tar" );

   // Sort file list
   files.sort();

   // For Monte Carlo, replace component temporary model files with
   //  concatenated model files
   if ( mc_iterations > 1 )
   {
      // Get a list of model files currently present
      QStringList mfilt;
      mfilt << "*.mdl.tmp" << "*.model.xml";
      QStringList mfiles = odir.entryList( mfilt, QDir::Files );
      QStringList mtrips;
      mfiles.sort();
DbgLv(0) << my_rank << ": MC : mfiles count" << mfiles.count();

      // Scan for unique triples
      for ( int ii = 0; ii < mfiles.size(); ii++ )
      {
         QString mftrip     = QString( mfiles[ ii ] ).section( ".", 0, -4 );
         if ( ! mtrips.contains( mftrip ) )
            mtrips << mftrip;
      }

int kmt=mtrips.count();
DbgLv(0) << my_rank << ": MC : mtrips count" << kmt;
DbgLv(0) << my_rank << ": MC : mtrips[0]" << mtrips[0];
if(kmt>1)
 DbgLv(0) << my_rank << ": MC : mtrips[n]" << mtrips[kmt-1];
      // Get a list of files in the text file
      QStringList tfiles;
      QFile filet( "analysis_files.txt" );
      if ( filet.open( QIODevice::ReadOnly | QIODevice::Text ) )
      {
         QTextStream tstxt( &filet );
         while ( ! tstxt.atEnd() )
         {
            QString line       = tstxt.readLine();
            QString fname      = line.section( ";", 0, 0 );
            tfiles << fname;
         }
         filet.close();
      }
DbgLv(0) << my_rank << ": MC : tfiles count" << tfiles.count();

      // Open text file for appending composite file names
      QFile fileo( "analysis_files.txt" );
      if ( ! fileo.open( QIODevice::WriteOnly | QIODevice::Text
                                              | QIODevice::Append ) )
      {
         abort( "Could not open 'analysis_files.txt' for writing" );
         return;
      }
      QTextStream tsout( &fileo );

      // For each triple, build or update a composite model file
      for ( int ii = 0; ii < mtrips.size(); ii++ )
      {
         // Build a list of model files relating to this triple
         QString mftrip   = mtrips[ ii ];
         QString tripleID = QString( mftrip ).section( ".", -1, -1 );
         QStringList mfilt;
         mfilt << mftrip + ".mc0*";
         QStringList mtfiles = odir.entryList( mfilt, QDir::Files );
DbgLv(0) << my_rank << ": MC : ii" << ii << "mftrip" << mftrip << "mtf count" << mtfiles.count();

         // Skip composite build if not yet enough triple mc iteration models
         if ( mtfiles.count() < mc_iterations )  continue;

         // Build a composite model file and get its name
         QString cmfname  = US_Model::composite_mc_file( mtfiles, true );
DbgLv(0) << my_rank << ":  ii" << ii << "cmfname" << cmfname;

         if ( cmfname.isEmpty() )  continue;

         // Remove iteration (non-composite) files from the list
         for ( int jj = 0; jj < mtfiles.size(); jj++ )
         {
            if ( mtfiles[ jj ] != cmfname )
            {
               files.removeOne( mtfiles[ jj ] );
DbgLv(0) << my_rank << ": ii,jj" << ii << jj << "REMOVED from list:" << mtfiles[jj];
            }
         }

         // Add the composite file name to the list if need be
         if ( ! files.contains( cmfname ) )
         {
            files << cmfname;
DbgLv(0) << my_rank << ":     files.size" << files.size() << "after cmfname add";
         }

         // Add composite name to text file if need be
         int f_iters      = QString( cmfname ).section( ".", -3, -3 )
                            .mid( 3 ).toInt();
         if ( ! tfiles.contains( cmfname )  &&
              ( is_final  ||  f_iters == mc_iterations ) )
         {
            US_Model model2;
DbgLv(0) << my_rank << ":      model2.load(" << cmfname << ")";
            model2.load( cmfname );
DbgLv(0) << my_rank << ":       model2.description" << model2.description;
            QString runstring = "Run: " + QString::number( ii + 1 )
                                + " " + tripleID;
            tsout << cmfname 
                  << ";meniscus_value=" << model2.meniscus
                  << ";MC_iteration="   << mc_iterations
                  << ";variance="       << model2.variance
                  << ";run="            << runstring << "\n";

            tfiles << cmfname;

            if ( analysis_type.contains( "PCSA" ) )
            {
               int mrx          = mrecs[ 2 ].taskx == mrecs[ 0 ].taskx ? 2 : 1;
               mrecs[ mrx ].modelGUID = model2.modelGUID;
            }
         }
      }

      fileo.close();
   }

   // Create the archive file containing all outputs
   US_Archive archive;
   QString filename = "analysis-results.tar";
   bool ok = archive.compress( files, filename );
   if ( !ok ) {
      QString error = archive.getError();
      abort( "Could not compress files\n " + error + "\n" + filename );
   }

for(int jf=0;jf<files.size();jf++)
 DbgLv(0) << my_rank << "   tar file" << jf << ":" << files[jf];

   // If this is the final call, remove all but the archive file
   if ( is_final )
   {  // Remove the files we just put into the tar archive
DbgLv(0) << my_rank << ": All output files except the archive are now removed.";
      QString file;
      foreach( file, files ) odir.remove( file );
   }
}

// Return the model type flag for a given analysis type string
US_Model::AnalysisType US_MPI_Analysis::model_type( const QString a_type )
{
   US_Model::AnalysisType
      m_type      = US_Model::TWODSA;
   if (      a_type.startsWith( "GA" ) )
      m_type      = US_Model::GA;
   else if ( a_type.startsWith( "DMGA" ) )
      m_type      = US_Model::DMGA;
   else if ( a_type.startsWith( "PCSA" ) )
      m_type      = US_Model::PCSA;

   return m_type;
}

// Write superglobal model output at the end of an iteration
void US_MPI_Analysis::write_superg( const SIMULATION&      sim, 
                                    US_Model::AnalysisType type )
{
   const QString uaGUID( "00000000-0000-0000-0000-000000000000" );
   US_DataIO::EditedData* edata = &data_sets[ 0 ]->run_data;

   // Fill in and write out the model file
   US_Model model;
   int subtype       = ( type == US_Model::PCSA ) ? mrecs[ 0 ].ctype : 0;

DbgLv(1) << "wrMo: type" << type << "(DMGA=" << US_Model::DMGA << ") (PCSA="
 << US_Model::PCSA << ") subtype=" << subtype;
   if ( type == US_Model::DMGA )
   {  // For discrete GA, get the already constructed model
      model             = data_sets[ 0 ]->model;
DbgLv(1) << "wrMo:  model comps" << model.components.size();
   }

   int mc_iter       = ( mgroup_count < 2  ||  is_composite_job ) 
                       ? ( mc_iteration + 1 ) : mc_iteration;
   model.monteCarlo  = mc_iterations > 1;
   model.wavelength  = edata->wavelength.toDouble();
   model.modelGUID   = ( ! model.monteCarlo  ||  mc_iter == 1 )
                       ? US_Util::new_guid() : modelGUID;
DbgLv(1) << "wrMo:  mc mciter mGUID" << model.monteCarlo << mc_iter
 << model.modelGUID;
   model.editGUID    = uaGUID;
   model.requestGUID = requestGUID;
   model.dataDescrip = edata->description;
   model.analysis    = type;
   model.global      = US_Model::SUPERGLOBAL;
   QString runID     = edata->runID;

   model.meniscus    = meniscus_value;
   model.variance    = sim.variance;

   QString tripleID  = edata->cell + edata->channel + edata->wavelength;
   QString dates     = "e" + edata->editID + "_a" + analysisDate;
DbgLv(1) << "wrMo: tripleID" << tripleID << "dates" << dates;
   QString iterID;

   if ( mc_iterations > 1 )
      iterID.sprintf( "mc%04d", mc_iter );
   else if ( fit_menbot )
      iterID.sprintf( "i%02d-m%05db%05d", 
              menibott_ndx + 1,
              (int)( meniscus_value * 10000 ),
              (int)( bottom_value * 10000 ) );
   else if (  fit_meni )
      iterID.sprintf( "i%02d-m%05d", 
              meniscus_run + 1,
              (int)( meniscus_value * 10000 ) );
   else if (  fit_bott )
      iterID.sprintf( "i%02d-b%05d", 
              bottom_run + 1,
              (int)( bottom_value * 10000 ) );
   else
      iterID = "i01";

   QString mdlid     = tripleID + "." + iterID;
   QString id        = model.typeText( subtype );
   if ( analysis_type.contains( "CG" ) )
      id                = id.replace( "2DSA", "2DSA-CG" );
   if ( max_iterations > 1  &&  mc_iterations == 1 )
      id               += "-IT";
   QString analyID   = dates + "_" + id + "_" + requestID + "_" + iterID;
   int     stype     = data_sets[ current_dataset ]->solute_type;
   double  vbar20    = data_sets[ current_dataset ]->vbar20;

   model.description = runID + "." + tripleID + "." + analyID + ".model";
DbgLv(1) << "wrMo: model descr" << model.description;

   // Save as class variable for later reference
   modelGUID         = model.modelGUID;

   if ( type == US_Model::PCSA )
   {  // For PCSA, construct the model from zsolutes
      for ( int ii = 0; ii < sim.zsolutes.size(); ii++ )
      {
         US_ZSolute zsolute = sim.zsolutes[ ii ];

         US_Model::SimulationComponent component;
         US_ZSolute::set_mcomp_values( component, zsolute, stype, true );
         component.name     = QString().sprintf( "SC%04d", ii + 1 );

         US_Model::calc_coefficients( component );
         model.components << component;
      }
   }

   else if ( type != US_Model::DMGA )
   {  // For other non-DMGA, construct the model from solutes
      for ( int ii = 0; ii < sim.solutes.size(); ii++ )
      {
         const US_Solute* solute = &sim.solutes[ ii ];

         US_Model::SimulationComponent component;
         component.s       = solute->s;
         component.f_f0    = solute->k;
         component.name    = QString().sprintf( "SC%04d", ii + 1 );
         component.vbar20  = (attr_z == ATTR_V) ? vbar20 : solute->v;
         component.signal_concentration = solute->c;

         US_Model::calc_coefficients( component );
         model.components << component;
      }
   }
DbgLv(1) << "wrMo: stype" << stype << QString().sprintf("0%o",stype)
 << "attr_z vbar20 mco0.v" << attr_z << vbar20 << model.components[0].vbar20;

   QString fext      = model.monteCarlo ? ".mdl.tmp" : ".model.xml";
   QString fileid    = "." + id + "." + mdlid + fext;
   QString fn        = runID + fileid;
   int lenfn         = fn.length();

   if ( lenfn > 96 )
   { // Insure a model file name less than 100 characters in length (tar limit)
      int lenri         = runID.length() + 96 - lenfn;
      fn                = runID.left( lenri ) + fileid;
   }

   // Output the model to a file
   model.write( fn );

   // Add the file name of the model file to the output list
   QFile fileo( "analysis_files.txt" );

   if ( ! fileo.open( QIODevice::WriteOnly | QIODevice::Text
                                           | QIODevice::Append ) )
   {
      abort( "Could not open 'analysis_files.txt' for writing" );
      return;
   }

   QTextStream tsout( &fileo );

   QString meniscus = QString::number( meniscus_value, 'e', 4 );
   QString variance = QString::number( sim.variance,   'e', 4 );

   int run     = 1;

   if ( menibott_ndx > 0 ) 
       run        = menibott_ndx + 1;
   else if ( mc_iterations > 0 )
       run        = mc_iter;

   QString runstring = "Run: " + QString::number( run ) + " " + tripleID;

   tsout << fn << ";meniscus_value=" << meniscus_value
               << ";MC_iteration="   << mc_iter
               << ";variance="       << sim.variance
               << ";run="            << runstring
               << ";bottom_value="   << bottom_value
               << "\n";
   fileo.close();
}

