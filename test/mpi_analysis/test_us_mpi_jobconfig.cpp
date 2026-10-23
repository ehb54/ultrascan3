//! \file test_us_mpi_jobconfig.cpp
//! \brief Unit tests of US_MPI_JobConfig, the hpcrequest XML reader of
//!        us_mpi_analysis.
//!
//! The files in jobs/ are anonymized hpcrequest files written by the LIMS
//! (2DSA with noise, GA with buckets, 2DSA custom grid, PCSA).  The inline cases
//! cover the rules of the reader that those files do not exercise.

#include <gtest/gtest.h>

#include "us_math2.h"
#include "us_mpi_jobconfig.h"

#include <memory>

namespace
{
typedef US_MPI_JobConfig Config;

Config                   parse( const QString& text )
{
   QByteArray data = text.toUtf8();
   QBuffer    buffer( &data );
   buffer.open( QIODevice::ReadOnly );
   return Config::fromXml( buffer, "test.xml" );
}

// A job document with the given job parameters and datasets
QString job( const QString& params, const QString& datasets = QString(), const QString& method = "2DSA" )
{
   return QString( "<?xml version=\"1.0\"?>\n<US_JobSubmit method=\"%1\" version=\"1.0\">"
                   "<job><jobParameters>%2</jobParameters></job>%3</US_JobSubmit>" )
      .arg( method, params, datasets );
}

QString job_file( const QString& name )
{
   return QString( US_MPI_TEST_JOBS_DIR ) + "/" + name;
}

// Owns the datasets of make_datasets()
std::vector<std::unique_ptr<US_SolveSim::DataSet>> datasets_of( const Config& config )
{
   std::vector<std::unique_ptr<US_SolveSim::DataSet>> owned;
   for ( US_SolveSim::DataSet* ds : config.make_datasets() )
   {
      owned.emplace_back( ds );
   }
   return owned;
}
} // namespace

// --- LIMS files -------------------------------------------------------------

TEST( US_MPI_JobConfig, Lims2dsaJob )
{
   Config config = Config::fromFile( job_file( "hpcrequest-2dsa.xml" ) );
   ASSERT_TRUE( config.ok() ) << config.errors.join( "\n" ).toStdString();

   EXPECT_EQ( config.method, "2DSA" );
   EXPECT_EQ( config.cluster, "node0" );
   EXPECT_EQ( config.db_name, "uslims3_TEST" ); // <database><name>
   EXPECT_EQ( config.udp_server, "127.0.0.1" );
   EXPECT_EQ( config.udp_port, 12233 );
   EXPECT_EQ( config.request_id, "004711" );
   EXPECT_EQ( config.request_guid, "00000000-0000-4000-8000-000000000001" );

   const QMap<QString, QString>& p = config.parameters;
   EXPECT_EQ( p.value( "s_max" ), "20" );
   EXPECT_EQ( p.value( "x_max" ), "20" ); // s_* also as x_*
   EXPECT_EQ( p.value( "x_min" ), "1" );
   EXPECT_EQ( p.value( "y_min" ), "1" ); // ff0_* also as y_*
   EXPECT_EQ( p.value( "y_max" ), "4" );
   EXPECT_EQ( p.value( "max_iterations" ), "10" );
   EXPECT_EQ( p.value( "global_fit" ), "0" );
   EXPECT_EQ( p.value( "z_value" ), "0.0" ); // Default
   EXPECT_FALSE( p.contains( "ytype" ) );    // No buckets
   EXPECT_EQ( config.debug_level(), 0 );
   EXPECT_FALSE( config.debug_timings() );
   EXPECT_EQ( config.debug_text(), QStringList( "" ) );
   EXPECT_TRUE( config.buckets.isEmpty() );

   ASSERT_EQ( config.datasets.size(), 1 );
   const Config::DatasetSpec& spec = config.datasets[0];
   EXPECT_EQ( spec.auc_file, "demo-2dsa.RA.1.A.590.auc" );
   EXPECT_EQ( spec.edit_file, "demo-2dsa.2610061116.RA.1.A.590.xml" );
   EXPECT_EQ( spec.noise_files, QStringList( { "clEgOh.ti_noise", "zuJEFc.ri_noise" } ) );
   EXPECT_EQ( spec.tmst_file.value_or( "" ), "demo-2dsa.time_state.tmst" );
   EXPECT_EQ( *spec.simpoints, 200 );
   EXPECT_EQ( *spec.band_volume, 0.0 );
   EXPECT_EQ( *spec.mesh_type, 0 );
   EXPECT_EQ( *spec.grid_type, 1 );
   EXPECT_DOUBLE_EQ( *spec.rotor_stretch[0], 2.01959e-7 );
   EXPECT_DOUBLE_EQ( *spec.rotor_stretch[1], 5.56967e-12 );
   EXPECT_EQ( *spec.cp_sector, 1 ); // "standard"
   EXPECT_EQ( *spec.centerpiece_bottom, 7.2 );
   EXPECT_EQ( spec.total_concentrations, QVector<double>( { -1.0 } ) );
   ASSERT_EQ( spec.speed_steps.size(), 1 );
   EXPECT_EQ( spec.speed_steps[0].rotorspeed, 42000 );
   EXPECT_EQ( spec.speed_steps[0].scans, 1200 );
   EXPECT_EQ( *spec.density, 1.0042 );
   EXPECT_EQ( *spec.viscosity, 1.03809 );
   EXPECT_FALSE( *spec.manual );
   ASSERT_EQ( spec.analytes.size(), 2 );
   EXPECT_EQ( spec.analytes[0].analyte.type, US_Analyte::PROTEIN );
   EXPECT_EQ( spec.analytes[1].analyte.type, US_Analyte::RNA );
   EXPECT_EQ( spec.analytes[1].analyte.mw, 15613.5 );

   auto dsets = datasets_of( config );
   ASSERT_EQ( dsets.size(), 1u );
   EXPECT_EQ( dsets[0]->edit_file, spec.edit_file );
   EXPECT_EQ( dsets[0]->rotor_stretch[1], *spec.rotor_stretch[1] );
   EXPECT_EQ( dsets[0]->simparams.cp_sector, 1 );
   EXPECT_DOUBLE_EQ( dsets[0]->vbar20, US_Math2::calcCommonVbar( dsets[0]->solution_rec, 20.0 ) );
}

TEST( US_MPI_JobConfig, LimsGaJob )
{
   Config config = Config::fromFile( job_file( "hpcrequest-ga.xml" ) );
   ASSERT_TRUE( config.ok() ) << config.errors.join( "\n" ).toStdString();

   EXPECT_EQ( config.method, "GA" );
   const QMap<QString, QString>& p = config.parameters;
   EXPECT_EQ( p.value( "bucket_fixed" ), "0" );
   EXPECT_EQ( p.value( "xtype" ), "s" );
   EXPECT_EQ( p.value( "ytype" ), "ff0" );
   EXPECT_EQ( p.value( "ztype" ), "vbar" );
   EXPECT_EQ( p.value( "mutate_sigma" ), "0" );
   EXPECT_EQ( p.value( "demes" ), "1" ); // Not used by us_mpi_analysis

   ASSERT_EQ( config.buckets.size(), 15 );
   EXPECT_DOUBLE_EQ( config.buckets[0].x_min, 0.5956 );
   EXPECT_DOUBLE_EQ( config.buckets[0].x_max, 1.404 );
   EXPECT_DOUBLE_EQ( config.buckets[0].y_min, 1.0 );
   EXPECT_DOUBLE_EQ( config.buckets[0].y_max, 1.24 );
   ASSERT_EQ( config.datasets.size(), 1 );
   EXPECT_EQ( config.datasets[0].total_concentrations, QVector<double>( { 0.219131808 } ) );
}

TEST( US_MPI_JobConfig, LimsCustomGridJob )
{
   Config config = Config::fromFile( job_file( "hpcrequest-2dsa-cg.xml" ) );
   ASSERT_TRUE( config.ok() ) << config.errors.join( "\n" ).toStdString();

   EXPECT_EQ( config.method, "2DSA_CG" );
   EXPECT_EQ( config.parameters.value( "CG_model" ), "demo-cg.model" );
   ASSERT_EQ( config.datasets.size(), 1 );
   EXPECT_TRUE( config.datasets[0].noise_files.isEmpty() );
}

TEST( US_MPI_JobConfig, LimsPcsaJob )
{
   Config config = Config::fromFile( job_file( "hpcrequest-pcsa.xml" ) );
   ASSERT_TRUE( config.ok() ) << config.errors.join( "\n" ).toStdString();

   EXPECT_EQ( config.method, "PCSA" );
   const QMap<QString, QString>& p = config.parameters;
   EXPECT_EQ( p.value( "curve_type" ), "IS" );
   EXPECT_EQ( p.value( "solute_type" ), "013.skv" );
   EXPECT_EQ( p.value( "x_max" ), "4" ); // Given as x_*, no s_* copies
   EXPECT_FALSE( p.contains( "s_max" ) );
   EXPECT_EQ( p.value( "z_value" ), "0" ); // Given, not the default
   EXPECT_EQ( p.value( "tikreg_option" ), "2" );
   EXPECT_EQ( p.value( "tikreg_alpha" ), "0.275" );
   ASSERT_EQ( config.datasets.size(), 1 );
   EXPECT_EQ( config.datasets[0].noise_files.size(), 2 );
}

// --- Job parameters ---------------------------------------------------------

TEST( US_MPI_JobConfig, ZValueDefaultOnlyWithJobElement )
{
   EXPECT_EQ( parse( job( "" ) ).parameters.value( "z_value" ), "0.0" );
   EXPECT_EQ( parse( job( "<z_value value=\"1.5\"/>" ) ).parameters.value( "z_value" ), "1.5" );

   Config empty = parse( "<US_JobSubmit method=\"2DSA\"><job/></US_JobSubmit>" );
   EXPECT_TRUE( empty.ok() );
   EXPECT_FALSE( empty.parameters.contains( "z_value" ) );
}

TEST( US_MPI_JobConfig, ModelFileParameters )
{
   Config config = parse( job( "<DC_model id=\"7\" filename=\"c.model.xml\"/>" ) );
   EXPECT_EQ( config.parameters.value( "DC_model" ), "c.model.xml" );
}

TEST( US_MPI_JobConfig, DebugParameters )
{
   Config config = parse( job( "<debug_level value=\"2\"/><debug_timings value=\"1\"/>"
                               "<debug_text value=\" SetSpeedLowA=100 , base_sig 0.3\"/>" ) );
   EXPECT_EQ( config.debug_level(), 2 );
   EXPECT_TRUE( config.debug_timings() );
   EXPECT_EQ( config.debug_text(), QStringList( { "SetSpeedLowA=100", "base_sig 0.3" } ) );

   Config none = parse( job( "" ) );
   EXPECT_EQ( none.debug_level(), 0 );
   EXPECT_FALSE( none.debug_timings() );
   EXPECT_TRUE( none.debug_text().isEmpty() );
}

// --- Buckets ----------------------------------------------------------------

TEST( US_MPI_JobConfig, BucketFixedSetsTypes )
{
   // bucket_fixed after a bucket overrides the ytype the bucket set
   Config config = parse( job( "<bucket s_min=\"1\" s_max=\"2\" vbar_min=\"0.7\" vbar_max=\"0.8\"/>"
                               "<bucket_fixed value=\"2.5\" fixedtype=\"ff0\" xtype=\"s\" ytype=\"mw\"/>",
                               QString(),
                               "GA" ) );
   EXPECT_EQ( config.parameters.value( "ytype" ), "mw" );
   EXPECT_EQ( config.parameters.value( "ztype" ), "ff0" );
}

TEST( US_MPI_JobConfig, FirstBucketSetsYType )
{
   Config ff0  = parse( job( "<bucket s_min=\"1\" s_max=\"2\" ff0_min=\"1\" ff0_max=\"2\"/>" ) );
   Config vbar = parse( job( "<bucket s_min=\"1\" s_max=\"2\" vbar_min=\"0.7\" vbar_max=\"0.8\"/>"
                             "<bucket s_min=\"2\" s_max=\"3\" ff0_min=\"1\" ff0_max=\"2\"/>" ) );
   Config xy   = parse( job( "<bucket x_min=\"1\" x_max=\"2\" y_min=\"0.7\" y_max=\"0.8\"/>" ) );
   EXPECT_EQ( ff0.parameters.value( "ytype" ), "ff0" );
   EXPECT_EQ( vbar.parameters.value( "ytype" ), "vbar" ); // The first bucket decides
   EXPECT_EQ( xy.parameters.value( "ytype" ), "ff0" );
   ASSERT_EQ( vbar.buckets.size(), 2 );
   EXPECT_DOUBLE_EQ( vbar.buckets[0].y_min, 0.7 );
   EXPECT_DOUBLE_EQ( vbar.buckets[1].y_max, 2.0 );
}

TEST( US_MPI_JobConfig, BucketRoundingKeepsSevenDigits )
{
   Config config = parse( job( "<bucket x_min=\"1.23456789\" x_max=\"9.87654321\" "
                               "y_min=\"1.000000049\" y_max=\"2.5\"/>" ) );
   ASSERT_EQ( config.buckets.size(), 1 );
   EXPECT_DOUBLE_EQ( config.buckets[0].x_min, 1.234568 );
   EXPECT_DOUBLE_EQ( config.buckets[0].x_max, 9.876543 );
   EXPECT_DOUBLE_EQ( config.buckets[0].y_min, 1.0 );
}

TEST( US_MPI_JobConfig, NegativeAndZeroBuckets )
{
   Config config = parse( job( "<bucket x_min=\"-3.0\" x_max=\"-1.0\" y_min=\"1\" y_max=\"2\"/>"
                               "<bucket x_min=\"0\" x_max=\"0\" y_min=\"1\" y_max=\"2\"/>" ) );
   ASSERT_EQ( config.buckets.size(), 2 );
   EXPECT_DOUBLE_EQ( config.buckets[0].x_min, -3.0 );
   EXPECT_DOUBLE_EQ( config.buckets[0].x_max, -1.0 );
   EXPECT_EQ( config.buckets[1].x_min, 0.0 );
   EXPECT_EQ( config.buckets[1].x_max, 0.0 );
}

// --- Datasets ---------------------------------------------------------------

TEST( US_MPI_JobConfig, TimestateTakenFromPreviousDataset )
{
   Config config = parse( job( "",
                               "<dataset><files><timestate filename=\"a.tmst\"/></files></dataset>"
                               "<dataset><files/></dataset>"
                               "<dataset><files><timestate filename=\"\"/></files></dataset>"
                               "<dataset/>" ) );
   auto   dsets  = datasets_of( config );
   ASSERT_EQ( dsets.size(), 4u );
   EXPECT_EQ( dsets[0]->tmst_file, "a.tmst" );
   EXPECT_EQ( dsets[1]->tmst_file, "a.tmst" ); // Not given: previous
   EXPECT_EQ( dsets[2]->tmst_file, "" );       // Given empty
   EXPECT_EQ( dsets[3]->tmst_file, "" );       // Previous (empty)
}

TEST( US_MPI_JobConfig, MwlEditFileName )
{
   Config::DatasetSpec spec;
   spec.auc_file  = "run.RI.2.A.280.auc";
   spec.edit_file = "run.e1.RI.2.A.260-290.xml";
   EXPECT_EQ( spec.edit_file_name(), "run.e1.RI.2.A.260-290@280.xml" );

   spec.edit_file = "run.e1.RI.2.A.260.xml";
   EXPECT_EQ( spec.edit_file_name(), "run.e1.RI.2.A.260.xml" );
}

TEST( US_MPI_JobConfig, LastDensityWins )
{
   // A density element after the buffer overrides the buffer density
   Config config = parse( job( "",
                               "<dataset><solution><buffer density=\"1.01\" viscosity=\"1.02\" "
                               "compress=\"0\" manual=\"1\"/></solution>"
                               "<density value=\"0.999\"/></dataset>" ) );
   ASSERT_EQ( config.datasets.size(), 1 );
   EXPECT_EQ( *config.datasets[0].density, 0.999 );
   EXPECT_EQ( *config.datasets[0].viscosity, 1.02 );
   EXPECT_TRUE( *config.datasets[0].manual );
}

TEST( US_MPI_JobConfig, UnsetValuesStayUnset )
{
   Config config = parse( job( "", "<dataset/>" ) );
   ASSERT_EQ( config.datasets.size(), 1 );
   const Config::DatasetSpec& spec = config.datasets[0];
   EXPECT_FALSE( spec.density );
   EXPECT_FALSE( spec.rotor_stretch[0] );
   EXPECT_FALSE( spec.centerpiece_bottom );
   EXPECT_FALSE( spec.tmst_file );
   EXPECT_FALSE( spec.has_solution );
   EXPECT_TRUE( spec.total_concentrations.isEmpty() );
}

TEST( US_MPI_JobConfig, CenterpieceShapes )
{
   auto shape = []( const QString& name )
   {
      Config config = parse( job( "", "<dataset><centerpiece_shape value=\"" + name + "\"/></dataset>" ) );
      return *config.datasets[0].cp_sector;
   };
   EXPECT_EQ( shape( "sector" ), 0 );
   EXPECT_EQ( shape( "band-forming" ), 3 );
   EXPECT_EQ( shape( "synthetic" ), 6 );
   EXPECT_EQ( shape( "unknown" ), 0 );
}

TEST( US_MPI_JobConfig, AnalyteTypes )
{
   Config      config   = parse( job( "",
                               "<dataset><solution>"
                                      "<analyte mw=\"1\" vbar20=\"0.5\" amount=\"1\" type=\"DNA\"/>"
                                      "<analyte mw=\"1\" vbar20=\"0.5\" amount=\"1\" type=\"Other\"/>"
                                      "<analyte mw=\"1\" vbar20=\"0.5\" amount=\"1\" type=\"Protein\"/>"
                                      "</solution></dataset>" ) );
   const auto& analytes = config.datasets[0].analytes;
   ASSERT_EQ( analytes.size(), 3 );
   EXPECT_EQ( analytes[0].analyte.type, US_Analyte::DNA );
   EXPECT_EQ( analytes[1].analyte.type, US_Analyte::CARBOHYDRATE );
   EXPECT_EQ( analytes[2].analyte.type, US_Analyte::PROTEIN );
}

// --- Errors -----------------------------------------------------------------

TEST( US_MPI_JobConfig, RotorStretchNeedsTwoCoefficients )
{
   Config config = parse( job( "", "<dataset><rotor_stretch value=\"0\"/></dataset>" ) );
   ASSERT_EQ( config.errors.size(), 1 );
   EXPECT_EQ( config.errors[0], "Invalid rotor_stretch value \"0\": two coefficients expected" );
   EXPECT_FALSE( config.datasets[0].rotor_stretch[0] );
}

TEST( US_MPI_JobConfig, TruncatedXml )
{
   QString text   = job( "<s_max value=\"10\"/>", "<dataset><files>" );
   Config  config = parse( text.left( text.indexOf( "</US_JobSubmit>" ) ) );
   ASSERT_FALSE( config.ok() );
   EXPECT_TRUE( config.errors.last().startsWith( "XML error in test.xml, line " ) ) << config.errors.last().toStdString();
}

TEST( US_MPI_JobConfig, MissingFile )
{
   Config config = Config::fromFile( job_file( "no-such-file.xml" ) );
   ASSERT_EQ( config.errors.size(), 1 );
   EXPECT_TRUE( config.errors[0].startsWith( "Cannot open file " ) );
}

// --- Validation -------------------------------------------------------------

namespace
{
// A dataset with everything validate() requires
const QString DATASET = "<dataset><files><auc filename=\"r.RI.1.A.260.auc\"/>"
                        "<edit filename=\"r.e1.RI.1.A.260.xml\"/></files>"
                        "<rotor_stretch value=\"0 1e-12\"/><centerpiece_bottom value=\"7.2\"/>"
                        "<solution><buffer density=\"0.998\" viscosity=\"1.002\" manual=\"0\"/></solution>"
                        "</dataset>";

const QString GRID_2DSA = "<s_min value=\"1\"/><s_max value=\"10\"/>"
                          "<ff0_min value=\"1\"/><ff0_max value=\"4\"/>";

Config        validated( const QString& params, const QString& method = "2DSA", const QString& datasets = DATASET )
{
   Config config = parse( job( params, datasets, method ) );
   EXPECT_TRUE( config.ok() ) << config.errors.join( "\n" ).toStdString();
   config.validate();
   return config;
}

std::string all_of( const Config& config )
{
   return ( config.errors + config.warnings ).join( "\n" ).toStdString();
}
} // namespace

TEST( US_MPI_JobConfig, LimsJobsAreValid )
{
   for ( const char* name : { "hpcrequest-2dsa.xml", "hpcrequest-ga.xml", "hpcrequest-2dsa-cg.xml", "hpcrequest-pcsa.xml" } )
   {
      Config config = Config::fromFile( job_file( name ) );
      config.validate();
      EXPECT_TRUE( config.errors.isEmpty() ) << name << ": " << all_of( config );
      EXPECT_TRUE( config.warnings.isEmpty() ) << name << ": " << all_of( config );
   }
}

TEST( US_MPI_JobConfig, ValidMinimalJobs )
{
   EXPECT_TRUE( validated( GRID_2DSA ).errors.isEmpty() );
   EXPECT_TRUE( validated( "<CG_model filename=\"g.model\"/>", "2DSA_CG" ).errors.isEmpty() ); // No grid needed
   EXPECT_TRUE( validated( "<population value=\"30\"/><generations value=\"15\"/>"
                           "<bucket x_min=\"1\" x_max=\"2\" y_min=\"1\" y_max=\"2\"/>",
                           "GA" )
                   .errors.isEmpty() );
   EXPECT_TRUE( validated( "<population value=\"30\"/><generations value=\"15\"/>"
                           "<DC_model filename=\"c.model.xml\"/>",
                           "DMGA" )
                   .errors.isEmpty() );
   EXPECT_TRUE( validated( "<curve_type value=\"All\"/><x_min value=\"1\"/><x_max value=\"10\"/>"
                           "<y_min value=\"1\"/><y_max value=\"4\"/><vars_count value=\"10\"/>"
                           "<curves_points value=\"200\"/>",
                           "PCSA" )
                   .errors.isEmpty() );
}

TEST( US_MPI_JobConfig, UnknownMethod )
{
   Config config = validated( GRID_2DSA, "2DXA" );
   ASSERT_EQ( config.errors.size(), 1 ) << all_of( config );
   EXPECT_TRUE( config.errors[0].contains( "\"2DXA\"" ) );
}

TEST( US_MPI_JobConfig, InvalidNumberReportedOnce )
{
   // s_max is also stored as x_max: one error, under the given name
   Config config = validated( "<s_min value=\"1\"/><s_max value=\"ten\"/><ff0_min value=\"1\"/><ff0_max value=\"4\"/>" );
   ASSERT_EQ( config.errors.size(), 1 ) << all_of( config );
   EXPECT_EQ( config.errors[0], "s_max \"ten\": not a number" );
}

TEST( US_MPI_JobConfig, IntegerParameters )
{
   EXPECT_EQ( validated( GRID_2DSA + "<mc_iterations value=\"10.5\"/>" ).errors,
              QStringList( "mc_iterations \"10.5\": not an integer" ) );
   EXPECT_TRUE( validated( GRID_2DSA + "<mc_iterations value=\" 100 \"/>" ).errors.isEmpty() );
}

TEST( US_MPI_JobConfig, RequiredParameters )
{
   EXPECT_EQ( validated( "<s_min value=\"1\"/><ff0_min value=\"1\"/><ff0_max value=\"4\"/>" ).errors,
              QStringList( "s_max: required for 2DSA" ) );
   EXPECT_EQ( validated( "<generations value=\"15\"/>", "GA" ).errors, QStringList( "population: required for GA" ) );
   EXPECT_EQ( validated( "<population value=\"30\"/><generations value=\"15\"/>", "DMGA" ).errors,
              QStringList( "DC_model: required for DMGA" ) );
   EXPECT_TRUE( validated( "", "PCSA" ).errors.contains( "curve_type: required for PCSA" ) );
}

TEST( US_MPI_JobConfig, RangesAndChoices )
{
   auto error_of = []( const QString& params, const QString& method = "2DSA" )
   {
      Config config = validated( method == "2DSA" ? GRID_2DSA + params : params, method );
      return config.errors.isEmpty() ? QString() : config.errors.last();
   };
   EXPECT_EQ( error_of( "<meniscus_points value=\"-3\"/>" ), "meniscus_points \"-3\": must be at least 0" );
   EXPECT_EQ( error_of( "<fit_mb_select value=\"4\"/>" ), "fit_mb_select \"4\": must be from 0 to 3" );
   EXPECT_EQ( error_of( "<global_fit value=\"2\"/>" ), "global_fit \"2\": not 0 or 1" );
   EXPECT_EQ( error_of( "<s_grid_points value=\"0\"/>" ), "s_grid_points \"0\": must be at least 1" );
   EXPECT_EQ( error_of( "<population value=\"30\"/><generations value=\"15\"/><crossover value=\"101\"/>", "GA" ),
              "crossover \"101\": must be from 0 to 100" );
   EXPECT_EQ( error_of( "<curve_type value=\"XY\"/><x_min value=\"1\"/><x_max value=\"10\"/><y_min value=\"1\"/>"
                        "<y_max value=\"4\"/><vars_count value=\"10\"/><curves_points value=\"200\"/>",
                        "PCSA" ),
              "curve_type \"XY\": not one of SL, IS, DS, HL, 2O, All" );
   EXPECT_EQ( error_of( "<s_min value=\"10\"/><s_max value=\"1\"/><ff0_min value=\"1\"/><ff0_max value=\"4\"/>" ),
              "s_min (10) must be less than s_max (1)" );
}

TEST( US_MPI_JobConfig, EmptyValueCountsAsNotGiven )
{
   // As in LIMS GA jobs: bucket_fixed with empty value and types
   Config config = validated( "<population value=\"30\"/><generations value=\"15\"/>"
                              "<bucket_fixed value=\"\" fixedtype=\"\" xtype=\"\" ytype=\"\"/>"
                              "<debug_text value=\"\"/><mutate_sigma value=\"\"/>",
                              "GA" );
   EXPECT_TRUE( config.errors.isEmpty() ) << all_of( config );
}

TEST( US_MPI_JobConfig, OtherMethodsParametersNotChecked )
{
   // A 2DSA grid value in a GA job is not used, so not checked
   Config config = validated( "<population value=\"30\"/><generations value=\"15\"/><s_grid_points value=\"x\"/>", "GA" );
   EXPECT_TRUE( config.errors.isEmpty() ) << all_of( config );
}

TEST( US_MPI_JobConfig, UnknownParameterWarnedOnce )
{
   Config config = validated( GRID_2DSA + "<s_mx value=\"10\"/><demes value=\"1\"/>" );
   EXPECT_TRUE( config.errors.isEmpty() ) << all_of( config );
   ASSERT_EQ( config.warnings.size(), 1 ) << all_of( config ); // Not for x_mx, not for demes
   EXPECT_EQ( config.warnings[0], "Unknown parameter s_mx \"10\" is ignored" );
}

TEST( US_MPI_JobConfig, BucketValidation )
{
   const QString ga = "<population value=\"30\"/><generations value=\"15\"/>";
   // Equal limits are valid (a fixed f/f0 per bucket, as in LIMS jobs)
   EXPECT_TRUE(
      validated( ga + "<bucket x_min=\"1.905\" x_max=\"1.915\" y_min=\"1.169\" y_max=\"1.169\"/>", "GA" ).errors.isEmpty() );
   EXPECT_EQ( validated( ga + "<bucket x_min=\"9.5\" x_max=\"8.6\" y_min=\"1\" y_max=\"2\"/>", "GA" ).errors,
              QStringList( "Bucket 1: limits x 9.5 to 8.6, y 1 to 2 are inverted" ) );
   // As in a LIMS job: empty x limits would become a flotation bucket
   EXPECT_EQ( validated( ga + "<bucket x_min=\"\" x_max=\"\" y_min=\"1\" y_max=\"4\"/>", "GA" ).errors,
              QStringList( "Bucket 1: s_min (no x_max), s_max (no x_max) missing or not a number" ) );
   EXPECT_EQ( validated( ga + "<bucket s_min=\"1\" s_max=\"2\" ff0_min=\"1\" ff0_max=\"x\"/>", "GA" ).errors,
              QStringList( "Bucket 1: ff0_max missing or not a number" ) );
}

TEST( US_MPI_JobConfig, DatasetValidation )
{
   EXPECT_EQ( validated( GRID_2DSA, "2DSA", "" ).errors, QStringList( "No dataset" ) );
   EXPECT_EQ( validated( GRID_2DSA, "2DSA", "<dataset/>" ).errors,
              QStringList( { "Dataset 1: no auc file",
                             "Dataset 1: no edit file",
                             "Dataset 1: no rotor_stretch",
                             "Dataset 1: no centerpiece_bottom",
                             "Dataset 1: no buffer density and viscosity" } ) );

   QString bottom73 = DATASET;
   bottom73.replace( "value=\"7.2\"", "value=\"7.3\"" );
   EXPECT_EQ( validated( GRID_2DSA, "2DSA", bottom73 ).errors,
              QStringList( "Dataset 1: the bottom is set to the invalid default value of 7.3" ) );

   QString two_concs = DATASET;
   two_concs.replace( "</dataset>", "<total_concentration value=\"1\"/><total_concentration value=\"2\"/></dataset>" );
   EXPECT_EQ( validated( GRID_2DSA, "2DSA", two_concs ).errors, QStringList( "Dataset 1: more than one total_concentration" ) );
}

TEST( US_MPI_JobConfig, DatasetCount )
{
   // datasetCount (LIMS) must match the datasets read: catches a file cut
   //  off between datasets
   QString text = job( GRID_2DSA, DATASET + DATASET );
   text.replace( "<job>", "<job><datasetCount value=\"3\"/>" );
   Config config = parse( text );
   config.validate();
   EXPECT_EQ( config.errors, QStringList( "datasetCount is 3, but 2 datasets were read" ) );

   text.replace( "value=\"3\"", "value=\"2\"" );
   Config good = parse( text );
   good.validate();
   EXPECT_TRUE( good.errors.isEmpty() ) << all_of( good );
}

TEST( US_MPI_JobConfig, DebugValues )
{
   Config config = parse( job( "<debug_text value=\"SetSpeedLowA=100, base_sig 0.3,flag, base_sig2 5\"/>" ) );
   bool   found  = false;
   EXPECT_EQ( config.debug_value( "SetSpeedLowA", &found ), "100" );
   EXPECT_TRUE( found );
   EXPECT_EQ( config.debug_value( "base_sig" ), "0.3" ); // Not base_sig2
   EXPECT_EQ( config.debug_value( "flag", &found ), "" );
   EXPECT_TRUE( found );
   EXPECT_EQ( config.debug_value( "g_redo_inc", &found ), "" );
   EXPECT_FALSE( found );
   EXPECT_EQ( config.debug_value( "base" ), "" ); // Exact keys only
}
