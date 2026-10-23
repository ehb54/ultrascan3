//! \file us_mpi_jobconfig.cpp
#include "us_mpi_jobconfig.h"
#include "us_math2.h"

#include <cmath>
#include <limits>

namespace
{
double value_attr( const QXmlStreamAttributes& attr, const char* name )
{
   return attr.value( name ).toString().toDouble();
}

// Round to 7 digits of the larger magnitude, to forestall round-off
//  problems.  Limits may be zero or negative (flotation s).
double rounded( double value, double vmin, double vmax )
{
   double vmag = qMax( qAbs( vmin ), qAbs( vmax ) );
   double vinc = ( vmag > 0.0 ) ? pow( 10.0, qFloor( log10( vmag ) ) - 6 ) : 1.0;
   return qRound64( value / vinc ) * vinc;
}
} // namespace

US_MPI_JobConfig US_MPI_JobConfig::fromFile( const QString& path )
{
   QFile file( path );

   if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) )
   {
      US_MPI_JobConfig config;
      config.errors << "Cannot open file " + path;
      return config;
   }

   return fromXml( file, path );
}

US_MPI_JobConfig US_MPI_JobConfig::fromXml( QIODevice& device, const QString& source )
{
   US_MPI_JobConfig config;
   QXmlStreamReader xml( &device );

   while ( !xml.atEnd() )
   {
      xml.readNext();

      if ( !xml.isStartElement() )
      {
         continue;
      }

      if ( xml.name() == QLatin1String( "US_JobSubmit" ) )
      {
         config.method = xml.attributes().value( "method" ).toString();
      }

      else if ( xml.name() == QLatin1String( "job" ) )
      {
         config.read_job( xml );
      }

      else if ( xml.name() == QLatin1String( "dataset" ) )
      {
         DatasetSpec spec;
         config.read_dataset( xml, spec );
         config.datasets << spec;
      }
   }

   if ( xml.hasError() )
   {
      config.errors << QString( "XML error in %1, line %2 column %3: %4" )
                          .arg( source )
                          .arg( xml.lineNumber() )
                          .arg( xml.columnNumber() )
                          .arg( xml.errorString() );
   }

   config.read_parameters();
   return config;
}

void US_MPI_JobConfig::read_parameters( void )
{
   const QMap<QString, QString>& p       = parameters;
   auto                          integer = [&p]( const char* key ) { return p.value( key ).toInt(); };
   auto                          number  = [&p]( const char* key ) { return p.value( key ).toDouble(); };
   auto                          given   = [&p]( const char* key )
   {
      Optional<double> value;
      if ( p.contains( key ) )
      {
         value = p.value( key ).toDouble();
      }
      return value;
   };
   // A default applies only if the parameter is not given at all
   auto        integer_or = [&p]( const char* key, int other ) { return p.contains( key ) ? p.value( key ).toInt() : other; };

   Parameters& pp      = params;
   pp.mc_iterations    = integer( "mc_iterations" );
   pp.max_iterations   = integer( "max_iterations" );
   pp.gfit_iterations  = integer( "gfit_iterations" );
   pp.montecarlo_value = integer( "montecarlo_value" );
   pp.fit_mb_select    = integer( "fit_mb_select" );
   pp.meniscus_range   = number( "meniscus_range" );
   pp.meniscus_points  = integer( "meniscus_points" );
   pp.tinoise_option   = integer( "tinoise_option" );
   pp.rinoise_option   = integer( "rinoise_option" );
   pp.global_fit       = p.value( "global_fit" ) == "1" || p.value( "global_fit" ) == "T";
   pp.seed             = p.value( "seed" ).toUInt();
   pp.req_mgroupcount  = p.value( "req_mgroupcount" );
   pp.z_value          = number( "z_value" );
   pp.solute_type      = p.value( "solute_type" );
   if ( p.contains( "CG_model" ) )
   {
      pp.cg_model = p.value( "CG_model" );
   }
   if ( p.contains( "DC_model" ) )
   {
      pp.dc_model = p.value( "DC_model" );
   }

   Parameters::Grid& grid = pp.grid;
   grid.s_min             = number( "s_min" );
   grid.s_max             = number( "s_max" );
   grid.ff0_min           = number( "ff0_min" );
   grid.ff0_max           = number( "ff0_max" );
   grid.x_min             = number( "x_min" );
   grid.x_max             = number( "x_max" );
   grid.y_min             = number( "y_min" );
   grid.y_max             = number( "y_max" );
   grid.uniform_grid      = integer( "uniform_grid" );
   grid.s_grid_points     = given( "s_grid_points" );
   grid.s_resolution      = given( "s_resolution" );
   grid.ff0_grid_points   = given( "ff0_grid_points" );
   grid.ff0_resolution    = given( "ff0_resolution" );

   Parameters::Ga& ga = pp.ga;
   ga.population      = integer( "population" );
   ga.generations     = integer( "generations" );
   ga.crossover       = integer( "crossover" );
   ga.mutation        = integer( "mutation" );
   ga.plague          = integer( "plague" );
   ga.migration       = integer( "migration" );
   ga.elitism         = integer( "elitism" );
   ga.mutate_sigma    = p.contains( "mutate_sigma" ) ? number( "mutate_sigma" ) : 2.0;
   ga.p_mutate_s      = integer_or( "p_mutate_s", 20 );
   ga.p_mutate_k      = integer_or( "p_mutate_k", 20 );
   ga.p_mutate_sk     = integer_or( "p_mutate_sk", 20 );
   ga.regularization  = number( "regularization" );
   ga.conc_threshold  = number( "conc_threshold" );
   ga.minimize_opt    = integer( "minimize_opt" );
   ga.s_grid          = integer_or( "s_grid", 100 );
   ga.k_grid          = integer_or( "k_grid", 100 );
   ga.p_grid          = integer_or( "p_grid", 100 );
   ga.bucket_fixed    = number( "bucket_fixed" );
   ga.xtype           = p.value( "xtype" );
   ga.ytype           = p.value( "ytype" );
   ga.ztype           = p.value( "ztype" );

   Parameters::Pcsa& pcsa = pp.pcsa;
   pcsa.curve_type        = p.value( "curve_type" );
   pcsa.vars_count        = integer( "vars_count" );
   pcsa.curves_points     = integer( "curves_points" );
   pcsa.thr_deltr_ratio   = number( "thr_deltr_ratio" );
   pcsa.tikreg_option     = integer( "tikreg_option" );
   pcsa.tikreg_alpha      = number( "tikreg_alpha" );
}

void US_MPI_JobConfig::read_job( QXmlStreamReader& xml )
{
   bool read_any = false;

   while ( !xml.atEnd() )
   {
      xml.readNext();

      if ( xml.isEndElement() && xml.name() == QLatin1String( "job" ) )
      {
         break;
      }

      read_any = true;

      if ( !xml.isStartElement() )
      {
         continue;
      }

      QXmlStreamAttributes attr  = xml.attributes();
      QString              xname = xml.name().toString();

      if ( xname == "cluster" )
      {
         cluster = attr.value( "shortname" ).toString();
      }

      else if ( xname == "name" )
      {
         db_name = attr.value( "value" ).toString();
      }

      else if ( xname == "udp" )
      {
         udp_server = attr.value( "server" ).toString();
         udp_port   = (quint16) attr.value( "port" ).toString().toInt();
      }

      else if ( xname == "global_fit" )
      {
         parameters[xname] = attr.value( "value" ).toString();
      }

      else if ( xname == "datasetCount" )
      {
         bool ok;
         int  count = attr.value( "value" ).toString().toInt( &ok );
         if ( ok )
         {
            dataset_count = count;
         }
         else
         {
            errors << "Invalid datasetCount \"" + attr.value( "value" ).toString() + "\"";
         }
      }

      else if ( xname == "request" )
      {
         request_id   = QString::asprintf( "%06d", attr.value( "id" ).toString().toInt() );
         request_guid = attr.value( "guid" ).toString();
      }

      else if ( xname == "jobParameters" )
      {
         read_job_parameters( xml );
      }
   }

   // An empty job element leaves z_value unset, as before
   if ( read_any && !parameters.contains( "z_value" ) )
   {
      parameters["z_value"] = "0.0";
   }
}

void US_MPI_JobConfig::read_job_parameters( QXmlStreamReader& xml )
{
   QString ytype; // Set by bucket_fixed or by the first bucket

   while ( !xml.atEnd() )
   {
      xml.readNext();
      QString name = xml.name().toString();

      if ( xml.isEndElement() && name == "jobParameters" )
      {
         break;
      }

      if ( !xml.isStartElement() )
      {
         continue;
      }

      QXmlStreamAttributes attr = xml.attributes();

      if ( name == "bucket" )
      {
         read_bucket( attr, ytype );
      }

      else if ( name == "CG_model" || name == "DC_model" )
      {
         parameters[name] = attr.value( "filename" ).toString();
      }

      else if ( name == "bucket_fixed" )
      {
         parameters[name]    = attr.value( "value" ).toString();
         parameters["xtype"] = attr.value( "xtype" ).toString();
         parameters["ytype"] = attr.value( "ytype" ).toString();
         parameters["ztype"] = attr.value( "fixedtype" ).toString();
         ytype               = parameters.value( "ytype" );
      }

      else if ( name.startsWith( "s_m" ) || name.startsWith( "ff0_m" ) )
      { // Also stored as the generic x/y limit
         QString svalu    = attr.value( "value" ).toString();
         parameters[name] = svalu;
         name.replace( "s_m", "x_m" );
         name.replace( "ff0_m", "y_m" );
         parameters[name] = svalu;
      }

      else
      {
         parameters[name] = attr.value( "value" ).toString();
      }
   }
}

void US_MPI_JobConfig::read_bucket( const QXmlStreamAttributes& attr, QString& ytype )
{
   // x/y limits; without x_max, s and f/f0 (or vbar) limits
   QString xmin_name = "x_min";
   QString xmax_name = "x_max";
   QString ymin_name = "y_min";
   QString ymax_name = "y_max";
   QString ytyp0     = "ff0";

   if ( attr.value( "x_max" ).isEmpty() )
   {
      xmin_name = "s_min";
      xmax_name = "s_max";

      if ( !attr.value( "ff0_max" ).isEmpty() )
      {
         ymin_name = "ff0_min";
         ymax_name = "ff0_max";
      }

      else if ( !attr.value( "vbar_max" ).isEmpty() )
      {
         ymin_name = "vbar_min";
         ymax_name = "vbar_max";
         ytyp0     = "vbar";
      }
   }

   Bucket bucket;
   auto   limit = [&attr, &bucket]( const QString& name ) { // A missing or invalid limit reads as 0, as before, and is recorded
      bool   ok;
      double value = attr.value( name ).toString().toDouble( &ok );
      if ( !ok )
      {
         bucket.unreadable << ( name.startsWith( "s_" ) ? name + " (no x_max)" : name );
      }
      return value;
   };
   double xmin = limit( xmin_name );
   double xmax = limit( xmax_name );
   double ymin = limit( ymin_name );
   double ymax = limit( ymax_name );

   if ( ytype.isEmpty() )
   {
      ytype               = ytyp0;
      parameters["ytype"] = ytype;
   }

   bucket.x_min = rounded( xmin, xmin, xmax );
   bucket.x_max = rounded( xmax, xmin, xmax );
   bucket.y_min = rounded( ymin, ymin, ymax );
   bucket.y_max = rounded( ymax, ymin, ymax );
   buckets << bucket;
}

void US_MPI_JobConfig::read_dataset( QXmlStreamReader& xml, DatasetSpec& spec )
{
   const QStringList shapes = { "sector", "standard", "rectangular", "band-forming", "meniscus-matching", "circular", "synthetic" };

   while ( !xml.atEnd() )
   {
      xml.readNext();

      if ( xml.isEndElement() && xml.name() == QLatin1String( "dataset" ) )
      {
         return;
      }

      if ( !xml.isStartElement() )
      {
         continue;
      }

      QString              name = xml.name().toString();
      QXmlStreamAttributes attr = xml.attributes();
      QString              sval = attr.value( "value" ).toString();

      if ( name == "files" )
      {
         read_files( xml, spec );
      }
      else if ( name == "solution" )
      {
         read_solution( xml, spec );
      }
      else if ( name == "simpoints" )
      {
         spec.simpoints = sval.toInt();
      }
      else if ( name == "band_volume" )
      {
         spec.band_volume = sval.toDouble();
      }
      else if ( name == "radial_grid" )
      {
         spec.mesh_type = sval.toInt();
      }
      else if ( name == "time_grid" )
      {
         spec.grid_type = sval.toInt();
      }
      else if ( name == "density" )
      {
         spec.density = sval.toDouble();
      }
      else if ( name == "viscosity" )
      {
         spec.viscosity = sval.toDouble();
      }
      else if ( name == "compress" )
      {
         spec.compress = sval.toDouble();
      }
      else if ( name == "rotor_stretch" )
      {
         QStringList stretch = sval.split( " ", Qt::SkipEmptyParts );

         if ( stretch.size() < 2 )
         {
            errors << "Invalid rotor_stretch value \"" + sval + "\": two coefficients expected";
         }
         else
         {
            spec.rotor_stretch[0] = stretch[0].toDouble();
            spec.rotor_stretch[1] = stretch[1].toDouble();
         }
      }
      else if ( name == "centerpiece_bottom" )
      {
         spec.centerpiece_bottom = sval.toDouble();
      }
      else if ( name == "centerpiece_shape" )
      {
         spec.cp_sector = qMax( 0, shapes.indexOf( sval ) );
      }
      else if ( name == "centerpiece_angle" )
      {
         spec.cp_angle = sval.toDouble();
      }
      else if ( name == "centerpiece_pathlength" )
      {
         spec.cp_pathlen = sval.toDouble();
      }
      else if ( name == "centerpiece_width" )
      {
         spec.cp_width = sval.toDouble();
      }
      else if ( name == "speedstep" )
      {
         US_SimulationParameters::SpeedProfile sp;
         US_SimulationParameters::speedstepFromXml( xml, sp );
         spec.speed_steps << sp;
      }
      else if ( name == "total_concentration" )
      {
         spec.total_concentrations << sval.toDouble();
      }
   }
}

void US_MPI_JobConfig::read_files( QXmlStreamReader& xml, DatasetSpec& spec )
{
   while ( !xml.atEnd() )
   {
      xml.readNext();

      if ( xml.isEndElement() && xml.name() == QLatin1String( "files" ) )
      {
         break;
      }

      if ( !xml.isStartElement() )
      {
         continue;
      }

      QString type     = xml.name().toString();
      QString filename = xml.attributes().value( "filename" ).toString();

      if ( type == "auc" )
      {
         spec.auc_file = filename;
      }
      else if ( type == "edit" )
      {
         spec.edit_file = filename;
      }
      else if ( type == "noise" )
      {
         spec.noise_files << filename;
      }
      else if ( type == "timestate" )
      {
         spec.tmst_file = filename;
      }
   }
}

void US_MPI_JobConfig::read_solution( QXmlStreamReader& xml, DatasetSpec& spec )
{
   spec.has_solution = true;

   while ( !xml.atEnd() )
   {
      xml.readNext();

      if ( xml.isEndElement() && xml.name() == QLatin1String( "solution" ) )
      {
         break;
      }

      if ( !xml.isStartElement() )
      {
         continue;
      }

      QXmlStreamAttributes attr = xml.attributes();

      if ( xml.name() == QLatin1String( "buffer" ) )
      {
         spec.density   = value_attr( attr, "density" );
         spec.viscosity = value_attr( attr, "viscosity" );
         spec.compress  = value_attr( attr, "compress" );
         spec.manual    = attr.value( "manual" ).toString().toInt() != 0;
      }

      else if ( xml.name() == QLatin1String( "analyte" ) )
      {
         US_Solution::AnalyteInfo aninfo;
         aninfo.analyte.mw     = value_attr( attr, "mw" );
         aninfo.analyte.vbar20 = value_attr( attr, "vbar20" );
         aninfo.amount         = value_attr( attr, "amount" );
         QString atype         = attr.value( "type" ).toString();
         aninfo.analyte.type   = US_Analyte::PROTEIN;

         if ( atype == "DNA" )
         {
            aninfo.analyte.type = US_Analyte::DNA;
         }
         else if ( atype == "RNA" )
         {
            aninfo.analyte.type = US_Analyte::RNA;
         }
         else if ( atype == "Other" )
         {
            aninfo.analyte.type = US_Analyte::CARBOHYDRATE;
         }

         spec.analytes << aninfo;
      }
   }
}

QString US_MPI_JobConfig::DatasetSpec::edit_file_name() const
{
   QString clambda = edit_file.section( ".", -2, -2 );

   if ( !clambda.contains( "-" ) )
   {
      return edit_file;
   }

   // MWL: "<run>.<edit>.<triple>.<lambda1-lambda2>.xml" becomes
   //  "<run>.<edit>.<triple>.<lambda1-lambda2>@<lambda>.xml"
   clambda        = clambda + "@" + auc_file.section( ".", -2, -2 );
   QString fpart1 = edit_file.section( ".", 0, -3 );
   QString fpart2 = edit_file.section( ".", -1, -1 );
   return fpart1 + "." + clambda + "." + fpart2;
}

int US_MPI_JobConfig::debug_level() const
{
   return parameters.value( "debug_level", "0" ).toInt();
}

bool US_MPI_JobConfig::debug_timings() const
{
   return parameters.value( "debug_timings", "0" ).toInt() != 0;
}

QStringList US_MPI_JobConfig::debug_text() const
{
   QStringList entries;

   if ( parameters.contains( "debug_text" ) )
   {
      for ( const QString& entry : parameters.value( "debug_text" ).split( ',' ) )
      {
         entries << entry.simplified();
      }
   }

   return entries;
}

QList<US_SolveSim::DataSet*> US_MPI_JobConfig::make_datasets() const
{
   QList<US_SolveSim::DataSet*> dsets;
   QString                      tmst_prev; // Timestate of the previous dataset

   for ( const DatasetSpec& spec : datasets )
   {
      US_SolveSim::DataSet*    ds = new US_SolveSim::DataSet();
      US_SimulationParameters& sp = ds->simparams;

      ds->auc_file    = spec.auc_file;
      ds->edit_file   = spec.edit_file_name();
      ds->tmst_file   = spec.tmst_file.value_or( tmst_prev );
      tmst_prev       = ds->tmst_file;
      ds->noise_files = spec.noise_files;
      sp.speed_step   = spec.speed_steps;

      // Values the file does not give keep their defaults
      auto set = []( auto& target, const auto& value )
      {
         if ( value )
         {
            target = *value;
         }
      };
      set( ds->density, spec.density );
      set( ds->viscosity, spec.viscosity );
      set( ds->compress, spec.compress );
      set( ds->manual, spec.manual );
      set( ds->rotor_stretch[0], spec.rotor_stretch[0] );
      set( ds->rotor_stretch[1], spec.rotor_stretch[1] );
      set( ds->centerpiece_bottom, spec.centerpiece_bottom );
      set( sp.simpoints, spec.simpoints );
      set( sp.band_volume, spec.band_volume );
      set( sp.cp_sector, spec.cp_sector );
      set( sp.cp_angle, spec.cp_angle );
      set( sp.cp_pathlen, spec.cp_pathlen );
      set( sp.cp_width, spec.cp_width );

      if ( spec.mesh_type )
      {
         sp.meshType = (US_SimulationParameters::MeshType) *spec.mesh_type;
      }
      if ( spec.grid_type )
      {
         sp.gridType = (US_SimulationParameters::GridType) *spec.grid_type;
      }

      ds->solution_rec.analyteInfo = spec.analytes;
      if ( spec.has_solution )
      {
         ds->vbar20 = US_Math2::calcCommonVbar( ds->solution_rec, 20.0 );
      }

      dsets << ds;
   }

   return dsets;
}

// --- Validation ---------------------------------------------------------------

namespace
{
// Method families as bits
enum
{
   M_2DSA = 1,
   M_GA   = 2,
   M_DMGA = 4,
   M_PCSA = 8,
   M_ANY  = 15
};

enum class Kind
{
   Int,    // Integer in [lo, hi]
   Double, // Finite number in [lo, hi]
   Flag,   // 0/1 (also T/F)
   Choice, // One of the words in choices
   Text,   // Any text
   File    // A file name, not empty
};

const double NOLIM = std::numeric_limits<double>::infinity();

// The job parameters us_mpi_analysis reads.  Where a parameter is used
//  by a method, lo..hi is its valid range.  An empty value counts as not
//  given; "required" parameters must be given.
struct ParamSpec
{
      const char* key;
      Kind        kind;
      int         methods;  // Methods that use it (M_ANY for all)
      int         required; // Methods that require it
      double      lo;
      double      hi;
      const char* choices; // Kind::Choice: words separated by blanks
};

const ParamSpec PARAMS[] = {
   // All methods
   { "mc_iterations", Kind::Int, M_ANY, 0, 0, NOLIM, nullptr },
   { "max_iterations", Kind::Int, M_ANY, 0, 0, NOLIM, nullptr },
   { "req_mgroupcount", Kind::Int, M_ANY, 0, 0, NOLIM, nullptr },
   { "tinoise_option", Kind::Int, M_ANY, 0, 0, NOLIM, nullptr },
   { "rinoise_option", Kind::Int, M_ANY, 0, 0, NOLIM, nullptr },
   { "fit_mb_select", Kind::Int, M_ANY, 0, 0, 3, nullptr },
   { "meniscus_range", Kind::Double, M_ANY, 0, 0, NOLIM, nullptr },
   { "meniscus_points", Kind::Int, M_ANY, 0, 0, NOLIM, nullptr },
   { "global_fit", Kind::Flag, M_ANY, 0, 0, 0, nullptr },
   { "seed", Kind::Int, M_ANY, 0, 0, 4294967295.0, nullptr },
   { "debug_level", Kind::Int, M_ANY, 0, 0, NOLIM, nullptr },
   { "debug_timings", Kind::Int, M_ANY, 0, 0, NOLIM, nullptr },
   { "debug_text", Kind::Text, M_ANY, 0, 0, 0, nullptr },
   { "z_value", Kind::Double, M_ANY, 0, -NOLIM, NOLIM, nullptr },
   { "bucket_fixed", Kind::Double, M_ANY, 0, -NOLIM, NOLIM, nullptr },
   { "xtype", Kind::Choice, M_ANY, 0, 0, 0, "s ff0 MW vbar D f" },
   { "ytype", Kind::Choice, M_ANY, 0, 0, 0, "s ff0 MW vbar D f" },
   { "ztype", Kind::Choice, M_ANY, 0, 0, 0, "s ff0 MW vbar D f" },
   { "solute_type", Kind::Text, M_ANY, 0, 0, 0, nullptr },
   // Grid limits; x_* and y_* are also set from s_* and ff0_*
   { "s_min", Kind::Double, M_ANY, 0, -NOLIM, NOLIM, nullptr },
   { "s_max", Kind::Double, M_ANY, 0, -NOLIM, NOLIM, nullptr },
   { "ff0_min", Kind::Double, M_ANY, 0, -NOLIM, NOLIM, nullptr },
   { "ff0_max", Kind::Double, M_ANY, 0, -NOLIM, NOLIM, nullptr },
   { "x_min", Kind::Double, M_ANY, M_PCSA, -NOLIM, NOLIM, nullptr },
   { "x_max", Kind::Double, M_ANY, M_PCSA, -NOLIM, NOLIM, nullptr },
   { "y_min", Kind::Double, M_ANY, M_PCSA, -NOLIM, NOLIM, nullptr },
   { "y_max", Kind::Double, M_ANY, M_PCSA, -NOLIM, NOLIM, nullptr },
   // 2DSA
   { "s_resolution", Kind::Double, M_2DSA | M_PCSA, 0, 0, NOLIM, nullptr },
   { "ff0_resolution", Kind::Double, M_2DSA | M_PCSA, 0, 0, NOLIM, nullptr },
   { "s_grid_points", Kind::Int, M_2DSA | M_PCSA, 0, 1, NOLIM, nullptr },
   { "ff0_grid_points", Kind::Int, M_2DSA | M_PCSA, 0, 1, NOLIM, nullptr },
   { "uniform_grid", Kind::Int, M_2DSA | M_PCSA, 0, 0, NOLIM, nullptr },
   { "montecarlo_value", Kind::Int, M_2DSA, 0, 0, NOLIM, nullptr },
   { "gfit_iterations", Kind::Int, M_2DSA | M_PCSA, 0, 0, NOLIM, nullptr },
   { "CG_model", Kind::File, M_2DSA, 0, 0, 0, nullptr },
   // GA and DMGA
   { "population", Kind::Int, M_GA | M_DMGA, M_GA | M_DMGA, 1, NOLIM, nullptr },
   { "generations", Kind::Int, M_GA | M_DMGA, M_GA | M_DMGA, 1, NOLIM, nullptr },
   { "crossover", Kind::Int, M_GA | M_DMGA, 0, 0, 100, nullptr },
   { "mutation", Kind::Int, M_GA | M_DMGA, 0, 0, 100, nullptr },
   { "plague", Kind::Int, M_GA | M_DMGA, 0, 0, 100, nullptr },
   { "migration", Kind::Int, M_GA | M_DMGA, 0, 0, 100, nullptr },
   { "elitism", Kind::Int, M_GA | M_DMGA, 0, 0, NOLIM, nullptr },
   { "mutate_sigma", Kind::Double, M_GA | M_DMGA, 0, 0, NOLIM, nullptr },
   { "p_mutate_s", Kind::Int, M_GA | M_DMGA, 0, 0, 100, nullptr },
   { "p_mutate_k", Kind::Int, M_GA | M_DMGA, 0, 0, 100, nullptr },
   { "p_mutate_sk", Kind::Int, M_GA | M_DMGA, 0, 0, 100, nullptr },
   { "regularization", Kind::Double, M_GA | M_DMGA, 0, 0, NOLIM, nullptr },
   { "conc_threshold", Kind::Double, M_GA | M_DMGA, 0, 0, NOLIM, nullptr },
   { "minimize_opt", Kind::Int, M_GA | M_DMGA, 0, 0, NOLIM, nullptr },
   { "s_grid", Kind::Int, M_GA, 0, 1, NOLIM, nullptr },
   { "k_grid", Kind::Int, M_GA, 0, 1, NOLIM, nullptr },
   { "p_grid", Kind::Int, M_DMGA, 0, 1, NOLIM, nullptr },
   { "demes", Kind::Int, M_GA | M_DMGA, 0, 0, NOLIM, nullptr }, // Sent by the LIMS, not used
   { "DC_model", Kind::File, M_DMGA, M_DMGA, 0, 0, nullptr },
   // PCSA
   { "curve_type", Kind::Choice, M_PCSA, M_PCSA, 0, 0, "SL IS DS HL 2O All" },
   { "vars_count", Kind::Int, M_PCSA, M_PCSA, 1, NOLIM, nullptr },
   { "curves_points", Kind::Int, M_PCSA, M_PCSA, 1, NOLIM, nullptr },
   { "thr_deltr_ratio", Kind::Double, M_PCSA, 0, 0, NOLIM, nullptr },
   { "tikreg_option", Kind::Int, M_PCSA, 0, 0, 2, nullptr },
   { "tikreg_alpha", Kind::Double, M_PCSA, 0, 0, NOLIM, nullptr },
};

int method_bit( const QString& method )
{
   const QStringList families = US_MPI_JobConfig::method_families();
   const int         bits[]   = { M_2DSA, M_DMGA, M_GA, M_PCSA }; // Order of families

   for ( int ii = 0; ii < families.size(); ii++ )
   {
      if ( method.startsWith( families[ii] ) )
      {
         return bits[ii];
      }
   }
   return 0;
}

// Check one value against its spec; returns an error text or empty
QString check_value( const ParamSpec& spec, const QString& value )
{
   QString name = QString( spec.key ) + " \"" + value + "\"";
   bool    ok   = true;
   double  dval = 0.0;

   switch ( spec.kind )
   {
      case Kind::Int:
         dval = (double) value.toLongLong( &ok );
         if ( !ok )
         {
            return name + ": not an integer";
         }
         break;
      case Kind::Double:
         dval = value.toDouble( &ok );
         if ( !ok || !std::isfinite( dval ) )
         {
            return name + ": not a number";
         }
         break;
      case Kind::Flag:
         if ( !QStringList( { "0", "1", "T", "F" } ).contains( value ) )
         {
            return name + ": not 0 or 1";
         }
         return QString();
      case Kind::Choice:
         if ( !QString( spec.choices ).split( ' ' ).contains( value ) )
         {
            return name + ": not one of " + QString( spec.choices ).replace( " ", ", " );
         }
         return QString();
      case Kind::Text:
      case Kind::File:
         return QString();
   }

   if ( dval < spec.lo || dval > spec.hi )
   {
      QString range =
         std::isinf( spec.hi ) ? QString( "at least %1" ).arg( spec.lo ) : QString( "from %1 to %2" ).arg( spec.lo ).arg( spec.hi );
      return name + ": must be " + range;
   }
   return QString();
}
} // namespace

QStringList US_MPI_JobConfig::method_families()
{
   // DMGA before GA: "DMGA" does not start with "GA", but keep the
   //  longer name first for clarity
   return { "2DSA", "DMGA", "GA", "PCSA" };
}

void US_MPI_JobConfig::validate()
{
   int  mbit = method_bit( method );

   // s_m* and ff0_m* parameters are also stored as x_m* and y_m*: report
   //  a problem only under the name given in the file
   auto mirrored = [this]( const QString& key )
   {
      QString source = key.startsWith( "x_m" )   ? "s_m" + key.mid( 3 )
                       : key.startsWith( "y_m" ) ? "ff0_m" + key.mid( 3 )
                                                 : QString();
      return !source.isEmpty() && parameters.contains( source ) && parameters.value( source ) == parameters.value( key );
   };

   if ( mbit == 0 )
   {
      errors << "Unknown analysis method \"" + method + "\" (expected " + method_families().join( ", " ) + ")";
   }

   // Parameter values
   QSet<QString> known;
   for ( const ParamSpec& spec : PARAMS )
   {
      known << spec.key;
      QString value = parameters.value( spec.key ).trimmed();

      if ( value.isEmpty() )
      {
         if ( ( spec.required & mbit ) != 0 )
         {
            errors << QString( "%1: required for %2" ).arg( spec.key, method );
         }
         continue;
      }

      QString error = check_value( spec, value );
      if ( !error.isEmpty() && ( spec.methods & mbit ) != 0 && !mirrored( spec.key ) )
      {
         errors << error;
      }
   }

   for ( auto it = parameters.constBegin(); it != parameters.constEnd(); ++it )
   {
      if ( !known.contains( it.key() ) && !mirrored( it.key() ) )
      {
         warnings << "Unknown parameter " + it.key() + " \"" + it.value() + "\" is ignored";
      }
   }

   // Ranges given by a pair of parameters
   auto check_range = [this]( const char* kmin, const char* kmax )
   {
      bool   okn, okx;
      double vmin = parameters.value( kmin ).toDouble( &okn );
      double vmax = parameters.value( kmax ).toDouble( &okx );
      if ( okn && okx && vmin >= vmax )
      {
         errors << QString( "%1 (%2) must be less than %3 (%4)" ).arg( kmin ).arg( vmin ).arg( kmax ).arg( vmax );
      }
   };

   if ( mbit == M_2DSA && !parameters.contains( "CG_model" ) )
   { // A 2DSA grid: s and f/f0 limits
      for ( const char* key : { "s_min", "s_max", "ff0_min", "ff0_max" } )
      {
         if ( parameters.value( key ).trimmed().isEmpty() )
         {
            errors << QString( "%1: required for %2" ).arg( key, method );
         }
      }
      check_range( "s_min", "s_max" );
      check_range( "ff0_min", "ff0_max" );
   }

   if ( mbit == M_PCSA )
   {
      check_range( "x_min", "x_max" );
      check_range( "y_min", "y_max" );
   }

   // GA buckets
   for ( int ii = 0; ii < buckets.size(); ii++ )
   {
      const Bucket& b = buckets[ii];
      if ( !b.unreadable.isEmpty() )
      {
         errors << QString( "Bucket %1: %2 missing or not a number" ).arg( ii + 1 ).arg( b.unreadable.join( ", " ) );
      }
      // Equal limits are valid: limitBucket() widens them
      else if ( !std::isfinite( b.x_min ) || !std::isfinite( b.x_max ) || !std::isfinite( b.y_min ) || !std::isfinite( b.y_max ) ||
                b.x_min > b.x_max || b.y_min > b.y_max )
      {
         errors << QString( "Bucket %1: limits x %2 to %3, y %4 to %5 are inverted" )
                      .arg( ii + 1 )
                      .arg( b.x_min )
                      .arg( b.x_max )
                      .arg( b.y_min )
                      .arg( b.y_max );
      }
   }

   // Datasets
   if ( datasets.isEmpty() )
   {
      errors << "No dataset";
   }
   if ( dataset_count && *dataset_count != datasets.size() )
   {
      errors << QString( "datasetCount is %1, but %2 datasets were read" ).arg( *dataset_count ).arg( datasets.size() );
   }

   for ( int ii = 0; ii < datasets.size(); ii++ )
   {
      const DatasetSpec& ds = datasets[ii];
      QString            dx = QString( "Dataset %1: " ).arg( ii + 1 );

      if ( ds.auc_file.isEmpty() )
      {
         errors << dx + "no auc file";
      }
      if ( ds.edit_file.isEmpty() )
      {
         errors << dx + "no edit file";
      }
      if ( !ds.rotor_stretch[0] )
      {
         errors << dx + "no rotor_stretch";
      }
      if ( !ds.centerpiece_bottom )
      {
         errors << dx + "no centerpiece_bottom";
      }
      else if ( *ds.centerpiece_bottom == 7.3 )
      { // The value of the LIMS for a bottom not set
         errors << dx + "the bottom is set to the invalid default value of 7.3";
      }
      if ( !ds.density || !ds.viscosity )
      {
         errors << dx + "no buffer density and viscosity";
      }
      if ( ds.total_concentrations.size() > 1 )
      {
         errors << dx + "more than one total_concentration";
      }
   }
}

QString US_MPI_JobConfig::debug_value( const QString& key, bool* found ) const
{
   for ( const QString& entry : debug_text() )
   { // "key=value", "key value" or "key"
      int     sep   = entry.indexOf( QRegularExpression( "[= ]" ) );
      QString ekey  = ( sep < 0 ) ? entry : entry.left( sep );
      QString value = ( sep < 0 ) ? QString() : entry.mid( sep + 1 ).trimmed();

      if ( ekey == key )
      {
         if ( found )
         {
            *found = true;
         }
         return value;
      }
   }

   if ( found )
   {
      *found = false;
   }
   return QString();
}
