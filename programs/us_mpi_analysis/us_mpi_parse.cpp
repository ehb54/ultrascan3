#include "us_mpi_analysis.h"
#include "us_mpi_jobconfig.h"
#include "us_settings.h"

// Read a file on rank 0 and send its content to all ranks, so that the
//  file system is read once and all ranks get the same text.  Returns
//  false on all ranks if rank 0 cannot read it.  Collective over
//  MPI_COMM_WORLD.
bool US_MPI_Analysis::read_shared_file( const QString& path, QByteArray& data )
{
   int readable = 1;
   data.clear();

   if ( my_rank == 0 )
   {
      QFile file( path );
      readable = file.open( QIODevice::ReadOnly ) ? 1 : 0;
      if ( readable )
      {
         data = file.readAll();
      }
   }

   MPI_Bcast( &readable, 1, MPI_INT, 0, MPI_COMM_WORLD );
   if ( !readable )
   {
      return false;
   }

   int nsize = data.size();
   MPI_Bcast( &nsize, 1, MPI_INT, 0, MPI_COMM_WORLD );
   data.resize( nsize );
   MPI_Bcast( data.data(), nsize, MPI_BYTE, 0, MPI_COMM_WORLD );
   return true;
}

// Read the job description (hpcrequest XML) into job_config, set up the
//  job data, datasets and debug settings from it, and validate it: errors
//  abort, warnings are printed.
void US_MPI_Analysis::read_job_config( const QString& xmlfile )
{
   QByteArray text; // Read by rank 0, sent to all ranks
   if ( !read_shared_file( xmlfile, text ) )
   {
      abort_all( "Cannot open file " + xmlfile );
   }

   QBuffer buffer( &text );
   buffer.open( QIODevice::ReadOnly | QIODevice::Text );
   job_config = US_MPI_JobConfig::fromXml( buffer, xmlfile );

   if ( !job_config.errors.isEmpty() )
   { // Cannot open, malformed XML, invalid values
      abort_all( job_config.errors.join( "; " ) );
   }

   analysis_type = job_config.method;
   cluster       = job_config.cluster;
   db_name       = job_config.db_name;
   server        = QHostAddress( job_config.udp_server );
   port          = job_config.udp_port;
   requestID     = job_config.request_id;
   requestGUID   = job_config.request_guid;

   // Debug level for the first ranks only (and rank 15)
   dbg_level = job_config.debug_level();
   US_Settings::set_us_debug( ( my_rank < 2 || my_rank == 15 ) ? dbg_level : 0 );
   dbg_timing = job_config.debug_timings();

   if ( job_config.parameters.contains( "debug_text" ) )
   {
      US_Settings::set_debug_text( job_config.debug_text() );
   }

   for ( const US_MPI_JobConfig::Bucket& jb : job_config.buckets )
   {
      Bucket bucket;
      bucket.x_min = jb.x_min;
      bucket.x_max = jb.x_max;
      bucket.y_min = jb.y_min;
      bucket.y_max = jb.y_max;
      bucket.ds    = 0.0;
      bucket.dk    = 0.0;
      buckets << bucket;
   }

   data_sets = job_config.make_datasets();
   for ( const US_MPI_JobConfig::DatasetSpec& spec : job_config.datasets )
   {
      concentrations += spec.total_concentrations;
   }
   set_solute_types();

   job_config.validate();

   if ( my_rank == 0 )
   {
      for ( const QString& warning : job_config.warnings )
      {
         printf( "  Warning: %s\n", qPrintable( warning ) );
      }
   }

   if ( !job_config.errors.isEmpty() )
   {
      abort_all( "Invalid job " + xmlfile + ": " + job_config.errors.join( "; " ) );
   }
}

// Set the model file and solute type of each dataset and the solute
//  attributes (attr_x, attr_y, attr_z) from the job parameters
void US_MPI_Analysis::set_solute_types( void )
{
   const US_MPI_JobConfig::Parameters& params = job_config.params;

   for ( US_SolveSim::DataSet* d : data_sets )
   {
      if ( params.cg_model )
      { // Custom Grid model input
         d->model_file  = *params.cg_model;
         d->solute_type = 2;
         attr_x         = ATTR_S;
         attr_y         = ATTR_V;
         attr_z         = ATTR_K;
      }

      else if ( params.dc_model )
      { // DMGA_Constraints model input
         d->model_file = *params.dc_model;
      }

      else if ( !params.ga.ztype.isEmpty() )
      { // Generic x,y model with fixed "z"
         const QStringList s_attrs = { "s", "ff0", "MW", "vbar", "D", "f" };
         attr_x                    = s_attrs.indexOf( params.ga.xtype );
         attr_y                    = s_attrs.indexOf( params.ga.ytype );
         attr_z                    = s_attrs.indexOf( params.ga.ztype );
         attr_x                    = ( attr_x < 0 ) ? ATTR_S : attr_x;
         attr_y                    = ( attr_y < 0 ) ? ATTR_K : attr_y;
         attr_z                    = ( attr_z < 0 ) ? ATTR_V : attr_z;
         // The solute type flag is a number which when viewed as
         //  octal is in the form "01xyz".
         d->solute_type            = ( attr_x << 6 ) + ( attr_y << 3 ) + attr_z;
      }

      else if ( params.ga.ytype == "vbar" )
      { // s,vbar model with fixed f_f0
         d->solute_type = 1;
         attr_x         = ATTR_S;
         attr_y         = ATTR_V;
         attr_z         = ATTR_K;
      }

      else if ( !params.solute_type.isEmpty() )
      { // Explicit solute type
         d->solute_type = US_ModelRecord::stype_flag( params.solute_type );
         attr_x         = ( d->solute_type >> 6 ) & 7;
         attr_y         = ( d->solute_type >> 3 ) & 7;
         attr_z         = d->solute_type & 7;
      }

      else
      { // Standard s,k model with fixed vbar
         d->solute_type = 0;
         attr_x         = ATTR_S;
         attr_y         = ATTR_K;
         attr_z         = ATTR_V;
      }

      if ( my_rank == 0 )
      {
         DbgLv( 0 ) << "PF: solute_type" << d->solute_type << "attr_x _y _z" << attr_x << attr_y << attr_z;
      }
   }
}
