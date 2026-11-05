// us_zeno_cxx.cpp -- runs NIST ZENO 5.3.1 (vendored in zeno/) for US-SOMO
//
// Adapted from NIST ZENO's command-line program, zeno/zeno-cli/src/Main.cc
// (public domain; NIST's notice is in zeno/README.US-SOMO).  The library
// itself is compiled by us_zeno_lib.cpp.  Changes from Main.cc:
//
//  - The entry point is zeno_cxx_main(), called with a command line by
//    US_Hydrodyn_Zeno::run().  Output goes to the file it names, not stdout.
//  - Errors return a nonzero status instead of calling exit().
//  - .bod files are read by Somo_Zeno_Bod_Parser below instead of NIST's Boost
//    Spirit parser.  It takes the 5.3.1 keywords plus TRIANGLE, and, as NIST
//    ZENO 5.0.1 did, S for SPHERE, HETATM for ATOM and // and /* */ comments
//    (and # comments).  VOXELS and TRAJECTORY are rejected.
//  - While walks run, progress goes to the SOMO GUI (or UDP progress messages
//    for command-line runs) and the GUI's stop flag is honored.
//  - Results without units are printed without "(1)", as 5.0.1 did, so the
//    .zno lines SOMO reads keep their form.
//  - The default seed comes from std::random_device.
//  - Times are printed with "%F %T", as NIST does, except on Windows, where the C
//    runtime may not support it and "%Y-%m-%d %H:%M:%S" is used instead.
//  - No trajectory mode and no MPI.

#include "../include/us_hydrodyn_zeno.h"
#undef R

#include "../zeno/us_somo_zeno_config.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <list>
#include <random>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "cmdline.h"
#include "ParametersLocal.h"

#include "Zeno.h"
#include "Results.h"

extern bool        * zeno_stop_flag;
extern US_Hydrodyn * zeno_us_hydrodyn;

namespace {

   // [0] header, [1] type, [2] data, as in Main.cc
   typedef std::array < std::vector < std::string >, 3 > Csv_Items;

   // ---- progress and stop -------------------------------------------------

   struct Somo_Zeno_Poll {
      bool          is_cmdline;
      US_Udp_Msg *  udp_msg;
   };

   // Runs on the thread that called zeno_cxx_main() while ZENO's threads work.
   // Returns true to stop the computation.
   bool somo_zeno_poll( void * data, long long num_done, long long num_requested ) {
      Somo_Zeno_Poll * poll = ( Somo_Zeno_Poll * ) data;

      double frac = num_requested > 0 ? ( double ) num_done / ( double ) num_requested : 0e0;

      if ( !poll->is_cmdline ) {
#if !defined( CMDLINE )
         if ( zeno_us_hydrodyn ) {
            zeno_us_hydrodyn->do_update_progress( ( int ) ( 1000e0 * frac ), 1000 );
         }
#endif
         qApp->processEvents();
         return zeno_stop_flag && *zeno_stop_flag;
      }

      if ( poll->udp_msg ) {
         map < QString, QString > msging;
         msging[ "progress_output" ] = QString( "Hydro (Zeno) calculation: %1%" ).arg( ( int ) ( 100e0 * frac ) );
         msging[ "progress1" ]       = QString::number( frac );
         poll->udp_msg->send_json( msging );
      }
      return false;
   }

   // ---- .bod file parser --------------------------------------------------

   class Somo_Zeno_Bod_Parser {
    public:
      Somo_Zeno_Bod_Parser( std::ostream                     & out,
                            zeno::ParametersWalkOnSpheres    * parameters_walk_on_spheres,
                            zeno::ParametersInteriorSampling * parameters_interior_sampling,
                            zeno::ParametersResults          * parameters_results,
                            zeno::MixedModel < double >      * model ) :
         out                         ( out ),
         parameters_walk_on_spheres  ( parameters_walk_on_spheres ),
         parameters_interior_sampling( parameters_interior_sampling ),
         parameters_results          ( parameters_results ),
         model                       ( model ),
         pos                         ( 0 ) {
      }

      // Returns 0 on success.  Errors are written to out.
      int parse( std::istream & in, std::string const & file_name );

    private:
      struct Token {
         std::string text;
         int         line;
      };

      std::ostream                     & out;
      zeno::ParametersWalkOnSpheres    * parameters_walk_on_spheres;
      zeno::ParametersInteriorSampling * parameters_interior_sampling;
      zeno::ParametersResults          * parameters_results;
      zeno::MixedModel < double >      * model;

      std::string                        file_name;
      std::vector < Token >              tokens;
      size_t                             pos;

      void tokenize( std::string const & text );
      int  error( int line, std::string const & msg );
      bool get_word( std::string & word, int keyword_line );
      bool get_number( double & value, int keyword_line );
      bool get_vector( zeno::Vector3 < double > & value, int keyword_line );
      int  statement();
      int  set_length_units( double number, std::string const & unit, int line );
   };

   // Splits the text into whitespace-separated tokens, dropping comments
   // ( // and # to the end of the line, /* ... */ ).
   void Somo_Zeno_Bod_Parser::tokenize( std::string const & text ) {
      tokens.clear();
      pos = 0;

      int         line = 1;
      std::string current;
      int         current_line = 1;

      size_t n = text.size();
      for ( size_t i = 0; i < n; ++i ) {
         char c = text[ i ];
         bool comment_line  = c == '#' || ( c == '/' && i + 1 < n && text[ i + 1 ] == '/' );
         bool comment_block = c == '/' && i + 1 < n && text[ i + 1 ] == '*';
         if ( comment_line || comment_block || isspace( ( unsigned char ) c ) ) {
            if ( !current.empty() ) {
               tokens.push_back( { current, current_line } );
               current.clear();
            }
            if ( comment_line ) {
               while ( i < n && text[ i ] != '\n' ) {
                  ++i;
               }
            } else if ( comment_block ) {
               i += 2;
               while ( i < n && !( text[ i ] == '*' && i + 1 < n && text[ i + 1 ] == '/' ) ) {
                  if ( text[ i ] == '\n' ) {
                     ++line;
                  }
                  ++i;
               }
               ++i;
               continue;
            }
            if ( i < n && text[ i ] == '\n' ) {
               ++line;
            }
            continue;
         }
         if ( current.empty() ) {
            current_line = line;
         }
         current += c;
      }
      if ( !current.empty() ) {
         tokens.push_back( { current, current_line } );
      }
   }

   int Somo_Zeno_Bod_Parser::error( int line, std::string const & msg ) {
      out << "Error: " << file_name << " line " << line << ": " << msg << std::endl;
      return 1;
   }

   bool Somo_Zeno_Bod_Parser::get_word( std::string & word, int keyword_line ) {
      if ( pos >= tokens.size() ) {
         error( keyword_line, "unexpected end of file" );
         return false;
      }
      word = tokens[ pos++ ].text;
      return true;
   }

   bool Somo_Zeno_Bod_Parser::get_number( double & value, int keyword_line ) {
      if ( pos >= tokens.size() ) {
         error( keyword_line, "unexpected end of file" );
         return false;
      }
      Token const & t = tokens[ pos++ ];
      char * end = nullptr;
      value = strtod( t.text.c_str(), &end );
      if ( end == t.text.c_str() || *end != '\0' ) {
         error( t.line, "expected a number, found '" + t.text + "'" );
         return false;
      }
      return true;
   }

   bool Somo_Zeno_Bod_Parser::get_vector( zeno::Vector3 < double > & value, int keyword_line ) {
      double x;
      double y;
      double z;
      if ( !get_number( x, keyword_line ) ||
           !get_number( y, keyword_line ) ||
           !get_number( z, keyword_line ) ) {
         return false;
      }
      value.setXYZ( x, y, z );
      return true;
   }

   int Somo_Zeno_Bod_Parser::set_length_units( double number, std::string const & unit, int line ) {
      zeno::Units::Length units;
      if ( unit == "m" ) {
         units = zeno::Units::Length::m;
      } else if ( unit == "cm" ) {
         units = zeno::Units::Length::cm;
      } else if ( unit == "nm" ) {
         units = zeno::Units::Length::nm;
      } else if ( unit == "A" ) {
         units = zeno::Units::Length::A;
      } else if ( unit == "L" ) {
         units = zeno::Units::Length::L;
      } else {
         return error( line, "invalid length unit " + unit );
      }
      parameters_results->setLengthScale( number, units );
      return 0;
   }

   // Reads one statement starting at tokens[ pos ].  Returns 0 on success.
   int Somo_Zeno_Bod_Parser::statement() {
      Token const & key_token = tokens[ pos++ ];
      int           line      = key_token.line;
      std::string   key       = key_token.text;
      std::transform( key.begin(), key.end(), key.begin(),
                      []( unsigned char c ) { return ( char ) toupper( c ); } );

      if ( key == "SPHERE" || key == "S" ) {
         zeno::Vector3 < double > center;
         double                   r;
         if ( !get_vector( center, line ) || !get_number( r, line ) ) {
            return 1;
         }
         if ( r < 0 ) {
            return error( line, "sphere with negative radius" );
         }
         model->addSphere( zeno::Sphere < double >( center, r ) );
         return 0;
      }

      if ( key == "CUBE" ) {
         zeno::Vector3 < double > min_coords;
         double                   s;
         if ( !get_vector( min_coords, line ) || !get_number( s, line ) ) {
            return 1;
         }
         if ( s < 0 ) {
            return error( line, "cube with negative side" );
         }
         zeno::Vector3 < double > max_coords( min_coords + s );
         model->addCuboid( zeno::Cuboid < double >( min_coords, max_coords ) );
         return 0;
      }

      if ( key == "CUBOID" ) {
         zeno::Vector3 < double > corner1;
         zeno::Vector3 < double > corner2;
         if ( !get_vector( corner1, line ) || !get_vector( corner2, line ) ) {
            return 1;
         }
         double x1;
         double y1;
         double z1;
         double x2;
         double y2;
         double z2;
         corner1.getXYZ( x1, y1, z1 );
         corner2.getXYZ( x2, y2, z2 );
         if ( x2 < x1 ) {
            std::swap( x1, x2 );
         }
         if ( y2 < y1 ) {
            std::swap( y1, y2 );
         }
         if ( z2 < z1 ) {
            std::swap( z1, z2 );
         }
         model->addCuboid( zeno::Cuboid < double >( zeno::Vector3 < double >( x1, y1, z1 ),
                                                    zeno::Vector3 < double >( x2, y2, z2 ) ) );
         return 0;
      }

      if ( key == "TRIANGLE" ) {
         zeno::Vector3 < double > v1;
         zeno::Vector3 < double > v2;
         zeno::Vector3 < double > v3;
         if ( !get_vector( v1, line ) || !get_vector( v2, line ) || !get_vector( v3, line ) ) {
            return 1;
         }
         model->addTriangle( zeno::Triangle < double >( v1, v2, v3 ) );
         return 0;
      }

      if ( key == "ATOM" || key == "HETATM" ) {
         // ATOM serial name resname [chain] resseq x y z occupancy radius, on one line
         std::vector < std::string > fields;
         while ( pos < tokens.size() && tokens[ pos ].line == line ) {
            fields.push_back( tokens[ pos++ ].text );
         }
         if ( fields.size() != 9 && fields.size() != 10 ) {
            return error( line, key + " needs 9 or 10 fields, found " + std::to_string( fields.size() ) );
         }
         double xyzr[ 4 ];
         size_t first = fields.size() - 5;
         size_t which[ 4 ] = { first, first + 1, first + 2, first + 4 };
         for ( int k = 0; k < 4; ++k ) {
            std::string const & f = fields[ which[ k ] ];
            char * end = nullptr;
            xyzr[ k ] = strtod( f.c_str(), &end );
            if ( end == f.c_str() || *end != '\0' ) {
               return error( line, "expected a number, found '" + f + "'" );
            }
         }
         if ( xyzr[ 3 ] < 0 ) {
            return error( line, "atom with negative radius" );
         }
         model->addSphere( zeno::Sphere < double >( zeno::Vector3 < double >( xyzr[ 0 ], xyzr[ 1 ], xyzr[ 2 ] ),
                                                    xyzr[ 3 ] ) );
         return 0;
      }

      if ( key == "ST" ) {
         double v;
         if ( !get_number( v, line ) ) {
            return 1;
         }
         parameters_walk_on_spheres->setSkinThickness( v );
         return 0;
      }

      if ( key == "RLAUNCH" ) {
         double v;
         if ( !get_number( v, line ) ) {
            return 1;
         }
         parameters_walk_on_spheres->setLaunchRadius( v );
         parameters_interior_sampling->setLaunchRadius( v );
         return 0;
      }

      if ( key == "HUNITS" ) {
         double      v;
         std::string unit;
         if ( !get_number( v, line ) || !get_word( unit, line ) ) {
            return 1;
         }
         return set_length_units( v, unit, line );
      }

      if ( key == "UNITS" ) {
         std::string unit;
         if ( !get_word( unit, line ) ) {
            return 1;
         }
         return set_length_units( 1, unit, line );
      }

      if ( key == "TEMP" ) {
         double      v;
         std::string unit;
         if ( !get_number( v, line ) || !get_word( unit, line ) ) {
            return 1;
         }
         zeno::Units::Temperature units;
         if ( unit == "C" ) {
            units = zeno::Units::Temperature::C;
         } else if ( unit == "K" ) {
            units = zeno::Units::Temperature::K;
         } else {
            return error( line, "invalid temperature unit " + unit );
         }
         parameters_results->setTemperature( v, units );
         return 0;
      }

      if ( key == "MASS" ) {
         double      v;
         std::string unit;
         if ( !get_number( v, line ) || !get_word( unit, line ) ) {
            return 1;
         }
         zeno::Units::Mass units;
         if ( unit == "Da" ) {
            units = zeno::Units::Mass::Da;
         } else if ( unit == "kDa" ) {
            units = zeno::Units::Mass::kDa;
         } else if ( unit == "g" ) {
            units = zeno::Units::Mass::g;
         } else if ( unit == "kg" ) {
            units = zeno::Units::Mass::kg;
         } else {
            return error( line, "invalid mass unit " + unit );
         }
         parameters_results->setMass( v, units );
         return 0;
      }

      if ( key == "VISCOSITY" ) {
         double      v;
         std::string unit;
         if ( !get_number( v, line ) || !get_word( unit, line ) ) {
            return 1;
         }
         zeno::Units::Viscosity units;
         if ( unit == "p" ) {
            units = zeno::Units::Viscosity::p;
         } else if ( unit == "cp" ) {
            units = zeno::Units::Viscosity::cp;
         } else {
            return error( line, "invalid viscosity unit " + unit );
         }
         parameters_results->setSolventViscosity( v, units );
         return 0;
      }

      if ( key == "BF" ) {
         double v;
         if ( !get_number( v, line ) ) {
            return 1;
         }
         parameters_results->setBuoyancyFactor( v );
         return 0;
      }

      if ( key == "VOXELS" || key == "TRAJECTORY" ) {
         return error( line, key + " is not supported by US-SOMO's ZENO" );
      }

      return error( line, "unknown keyword '" + key_token.text + "'" );
   }

   int Somo_Zeno_Bod_Parser::parse( std::istream & in, std::string const & name ) {
      file_name = name;
      std::stringstream buffer;
      buffer << in.rdbuf();
      tokenize( buffer.str() );
      while ( pos < tokens.size() ) {
         if ( statement() ) {
            return 1;
         }
      }
      return 0;
   }

   // ---- command line --------------------------------------------------------

   // As parseCommandLine() in Main.cc.  Returns 0 on success.
   int parse_command_line( int                                argc,
                           char                            ** argv,
                           std::ostream                     & out,
                           zeno::ParametersWalkOnSpheres    * parameters_walk_on_spheres,
                           zeno::ParametersInteriorSampling * parameters_interior_sampling,
                           zeno::ParametersResults          * parameters_results,
                           ParametersLocal                  * parameters_local ) {
      ( void ) parameters_results;

      gengetopt_args_info     args_info;
      cmdline_parser_params * parser_params = cmdline_parser_params_create();

      parser_params->initialize      = 1;
      parser_params->override        = 0;
      parser_params->check_required  = 0;
      parser_params->check_ambiguity = 0;

      if ( cmdline_parser_ext( argc, argv, &args_info, parser_params ) != 0 ) {
         out << "Error: could not parse the ZENO command line" << std::endl;
         free( parser_params );
         return 1;
      }

      if ( args_info.config_file_given ) {
         parser_params->initialize      = 0;
         parser_params->override        = 0;
         parser_params->check_required  = 0;
         parser_params->check_ambiguity = 0;

         if ( cmdline_parser_config_file( args_info.config_file_arg, &args_info, parser_params ) != 0 ) {
            out << "Error: could not parse the ZENO config file " << args_info.config_file_arg << std::endl;
            cmdline_parser_free( &args_info );
            free( parser_params );
            return 1;
         }
      }

      if ( cmdline_parser_required( &args_info, argv[ 0 ] ) != 0 ) {
         out << "Error: the ZENO command line has no input file" << std::endl;
         cmdline_parser_free( &args_info );
         free( parser_params );
         return 1;
      }

      parameters_local->setInputFileName( args_info.input_file_arg );

      if ( args_info.csv_output_file_given ) {
         parameters_local->setCsvOutputFileName( args_info.csv_output_file_arg );
      }

      parameters_walk_on_spheres->setMinTotalNumWalks( args_info.min_num_walks_arg );

      parameters_interior_sampling->setMinTotalNumSamples( args_info.min_num_interior_samples_arg );

      if ( args_info.num_walks_given ) {
         parameters_walk_on_spheres->setTotalNumWalks( args_info.num_walks_arg );
      }

      if ( args_info.num_interior_samples_given ) {
         parameters_interior_sampling->setTotalNumSamples( args_info.num_interior_samples_arg );
      }

      if ( args_info.max_rsd_capacitance_given ) {
         parameters_walk_on_spheres->setMaxErrorCapacitance( args_info.max_rsd_capacitance_arg );
      }

      if ( args_info.max_rsd_polarizability_given ) {
         parameters_walk_on_spheres->setMaxErrorPolarizability( args_info.max_rsd_polarizability_arg );
      }

      if ( args_info.max_rsd_volume_given ) {
         parameters_interior_sampling->setMaxErrorVolume( args_info.max_rsd_volume_arg );
      }

      if ( args_info.max_run_time_given ) {
         parameters_walk_on_spheres->setMaxRunTime( args_info.max_run_time_arg );
         parameters_interior_sampling->setMaxRunTime( args_info.max_run_time_arg );
      }

      int num_threads = 0;
      if ( args_info.num_threads_given ) {
         num_threads = args_info.num_threads_arg;
      } else {
         num_threads = std::thread::hardware_concurrency();
      }
      if ( num_threads < 1 ) {
         num_threads = 1;
      }
      parameters_walk_on_spheres->setNumThreads( num_threads );
      parameters_interior_sampling->setNumThreads( num_threads );

      int seed = 0;
      if ( args_info.seed_given ) {
         seed = args_info.seed_arg;
      } else {
         std::random_device rd;
         seed = ( int ) ( rd() & 0x7fffffff );
      }
      parameters_walk_on_spheres->setSeed( seed );
      parameters_interior_sampling->setSeed( seed );

      if ( args_info.surface_points_file_given ) {
         parameters_local->setSurfacePointsFileName( args_info.surface_points_file_arg );
         parameters_walk_on_spheres->setSaveSurfacePoints( true );
      }

      if ( args_info.interior_points_file_given ) {
         parameters_local->setInteriorPointsFileName( args_info.interior_points_file_arg );
         parameters_interior_sampling->setSaveInteriorPoints( true );
      }

      parameters_local->setPrintCounts( args_info.print_counts_given );
      parameters_local->setPrintBenchmarks( args_info.print_benchmarks_given );

      cmdline_parser_free( &args_info );
      free( parser_params );
      return 0;
   }

   // ---- output, as in Main.cc -----------------------------------------------

   template < typename T >
   std::string to_string_scientific( T const & value ) {
      std::ostringstream o;
      o << std::scientific << value;
      return o.str();
   }

   // 5.0.1 printed no units for dimensionless results; keep that
   std::string units_label( std::string const & units ) {
      if ( units.empty() || units == "1" ) {
         return "";
      }
      return " (" + units + ")";
   }

   template < typename T >
   void print_exact_scalar( std::ostream      & out,
                            std::string const & pretty_name,
                            std::string const & csv_name,
                            std::string const & units,
                            T                   property,
                            Csv_Items         * csv_items ) {
      out << pretty_name << units_label( units ) << std::fixed << ": " << property << std::endl;

      if ( !units.empty() ) {
         csv_items->at( 0 ).push_back( csv_name );
         csv_items->at( 1 ).push_back( "units" );
         csv_items->at( 2 ).push_back( units );
      }

      csv_items->at( 0 ).push_back( csv_name );
      csv_items->at( 1 ).push_back( "value" );
      csv_items->at( 2 ).push_back( to_string_scientific( property ) );
   }

   template < typename T >
   void print_exact_vector3( std::ostream                 & out,
                             std::string const            & pretty_name,
                             std::string const            & csv_name,
                             std::string const            & units,
                             zeno::Vector3 < T > const    & property,
                             Csv_Items                    * csv_items ) {
      out << pretty_name << units_label( units ) << std::fixed << ": " << property << std::endl;

      for ( int i = 0; i < 3; ++i ) {
         std::string name = csv_name + "[" + std::to_string( i ) + "]";
         if ( !units.empty() ) {
            csv_items->at( 0 ).push_back( name );
            csv_items->at( 1 ).push_back( "units" );
            csv_items->at( 2 ).push_back( units );
         }
         csv_items->at( 0 ).push_back( name );
         csv_items->at( 1 ).push_back( "value" );
         csv_items->at( 2 ).push_back( to_string_scientific( property.get( i ) ) );
      }
   }

   void print_scalar( std::ostream                                     & out,
                      Result < zeno::Uncertain < double > > const      & result,
                      Csv_Items                                        * csv_items ) {
      out << std::scientific
          << result.prettyName << units_label( result.unit ) << ": "
          << result.value << std::endl
          << std::endl;

      csv_items->at( 0 ).push_back( result.csvName );
      csv_items->at( 1 ).push_back( "units" );
      csv_items->at( 2 ).push_back( result.unit );

      csv_items->at( 0 ).push_back( result.csvName );
      csv_items->at( 1 ).push_back( "value" );
      csv_items->at( 2 ).push_back( to_string_scientific( result.value.getMean() ) );

      csv_items->at( 0 ).push_back( result.csvName );
      csv_items->at( 1 ).push_back( "std_dev" );
      csv_items->at( 2 ).push_back( to_string_scientific( result.value.getStdDev() ) );
   }

   void print_vector3( std::ostream                                                    & out,
                       Result < zeno::Vector3 < zeno::Uncertain < double > > > const   & result,
                       Csv_Items                                                       * csv_items ) {
      out << std::scientific
          << result.prettyName << units_label( result.unit ) << ": " << std::endl
          << result.value << std::endl
          << std::endl;

      for ( int i = 0; i < 3; ++i ) {
         std::string name = result.csvName + "[" + std::to_string( i ) + "]";

         csv_items->at( 0 ).push_back( name );
         csv_items->at( 1 ).push_back( "units" );
         csv_items->at( 2 ).push_back( result.unit );

         csv_items->at( 0 ).push_back( name );
         csv_items->at( 1 ).push_back( "value" );
         csv_items->at( 2 ).push_back( to_string_scientific( result.value.get( i ).getMean() ) );

         csv_items->at( 0 ).push_back( name );
         csv_items->at( 1 ).push_back( "std_dev" );
         csv_items->at( 2 ).push_back( to_string_scientific( result.value.get( i ).getStdDev() ) );
      }
   }

   void print_matrix3x3( std::ostream                                                      & out,
                         Result < zeno::Matrix3x3 < zeno::Uncertain < double > > > const   & result,
                         Csv_Items                                                         * csv_items ) {
      out << std::scientific
          << result.prettyName << units_label( result.unit ) << ": " << std::endl
          << result.value << std::endl
          << std::endl;

      for ( int row = 0; row < 3; ++row ) {
         for ( int col = 0; col < 3; ++col ) {
            std::string name = result.csvName + "[" + std::to_string( row ) + "][" + std::to_string( col ) + "]";

            csv_items->at( 0 ).push_back( name );
            csv_items->at( 1 ).push_back( "units" );
            csv_items->at( 2 ).push_back( result.unit );

            csv_items->at( 0 ).push_back( name );
            csv_items->at( 1 ).push_back( "value" );
            csv_items->at( 2 ).push_back( to_string_scientific( result.value.get( row, col ).getMean() ) );

            csv_items->at( 0 ).push_back( name );
            csv_items->at( 1 ).push_back( "std_dev" );
            csv_items->at( 2 ).push_back( to_string_scientific( result.value.get( row, col ).getStdDev() ) );
         }
      }
   }

   void print_parameters( std::ostream                           & out,
                          zeno::ParametersWalkOnSpheres const    & parameters_walk_on_spheres,
                          zeno::ParametersInteriorSampling const & parameters_interior_sampling,
                          zeno::ParametersResults const          & parameters_results,
                          ParametersLocal const                  & parameters_local,
                          Csv_Items                              * csv_items ) {
      print_exact_scalar( out, "Input file", "input_file", "",
                          parameters_local.getInputFileName(), csv_items );

      print_exact_scalar( out, "Number of nodes", "num_nodes", "",
                          parameters_local.getMpiSize(), csv_items );

      print_exact_scalar( out, "Number of threads", "num_threads", "",
                          parameters_walk_on_spheres.getNumThreads(), csv_items );

      print_exact_scalar( out, "Random number seed", "random_number_seed", "",
                          parameters_walk_on_spheres.getSeed(), csv_items );

      if ( parameters_walk_on_spheres.getTotalNumWalksWasSet() ) {
         print_exact_scalar( out, "Number of walks requested", "num_walks_requested", "",
                             parameters_walk_on_spheres.getTotalNumWalks(), csv_items );
      }

      if ( parameters_interior_sampling.getTotalNumSamplesWasSet() ) {
         print_exact_scalar( out, "Number of interior samples requested", "num_interior_samples_requested", "",
                             parameters_interior_sampling.getTotalNumSamples(), csv_items );
      }

      if ( parameters_walk_on_spheres.getMaxErrorCapacitanceWasSet() ) {
         print_exact_scalar( out, "Max error in capacitance requested", "max_error_capacitance_requested", "%",
                             parameters_walk_on_spheres.getMaxErrorCapacitance(), csv_items );
      }

      if ( parameters_walk_on_spheres.getMaxErrorPolarizabilityWasSet() ) {
         print_exact_scalar( out, "Max error in mean polarizability requested",
                             "max_error_mean_polarizability_requested", "%",
                             parameters_walk_on_spheres.getMaxErrorPolarizability(), csv_items );
      }

      if ( parameters_interior_sampling.getMaxErrorVolumeWasSet() ) {
         print_exact_scalar( out, "Max error in volume requested", "max_error_volume_requested", "%",
                             parameters_interior_sampling.getMaxErrorVolume(), csv_items );
      }

      if ( parameters_walk_on_spheres.getMaxRunTimeWasSet() ) {
         print_exact_scalar( out, "Max run time", "max_run_time", "s",
                             parameters_walk_on_spheres.getMaxRunTime(), csv_items );
      }

      if ( parameters_walk_on_spheres.getSkinThicknessWasSet() ) {
         print_exact_scalar( out, "Skin thickness", "skin_thickness", "",
                             parameters_walk_on_spheres.getSkinThickness(), csv_items );
      }

      if ( parameters_walk_on_spheres.getLaunchCenterWasSet() ) {
         print_exact_vector3( out, "Launch center", "launch_center", "",
                              parameters_walk_on_spheres.getLaunchCenter(), csv_items );
      }

      if ( parameters_walk_on_spheres.getLaunchRadiusWasSet() ) {
         print_exact_scalar( out, "Launch radius", "launch_radius", "",
                             parameters_walk_on_spheres.getLaunchRadius(), csv_items );
      }

      if ( parameters_results.getLengthScaleWasSet() ) {
         print_exact_scalar( out, "Length scale", "length_scale",
                             zeno::Units::getName( parameters_results.getLengthScaleUnit() ),
                             parameters_results.getLengthScaleNumber(), csv_items );
      }

      if ( parameters_results.getTemperatureWasSet() ) {
         print_exact_scalar( out, "Temperature", "temperature",
                             zeno::Units::getName( parameters_results.getTemperatureUnit() ),
                             parameters_results.getTemperatureNumber(), csv_items );
      }

      if ( parameters_results.getMassWasSet() ) {
         print_exact_scalar( out, "Mass", "mass",
                             zeno::Units::getName( parameters_results.getMassUnit() ),
                             parameters_results.getMassNumber(), csv_items );
      }

      if ( parameters_results.getSolventViscosityWasSet() ) {
         print_exact_scalar( out, "Solvent viscosity", "solvent_viscosity",
                             zeno::Units::getName( parameters_results.getSolventViscosityUnit() ),
                             parameters_results.getSolventViscosityNumber(), csv_items );
      }

      if ( parameters_results.getBuoyancyFactorWasSet() ) {
         print_exact_scalar( out, "Buoyancy factor", "buoyancy_factor", "",
                             parameters_results.getBuoyancyFactor(), csv_items );
      }
   }

   void print_results( std::ostream          & out,
                       zeno::Results const   & results,
                       Csv_Items             * csv_items ) {
      if ( results.resultsZenoCompiled ) {
         print_exact_scalar( out, results.numWalks.prettyName, results.numWalks.csvName,
                             results.numWalks.unit, results.numWalks.value, csv_items );
         out << std::endl;
      }

      if ( results.resultsInteriorCompiled ) {
         print_exact_scalar( out, results.numInteriorSamples.prettyName, results.numInteriorSamples.csvName,
                             results.numInteriorSamples.unit, results.numInteriorSamples.value, csv_items );
         out << std::endl;
      }

      if ( results.resultsZenoCompiled ) {
         print_scalar   ( out, results.capacitance,               csv_items );
         print_matrix3x3( out, results.polarizabilityTensor,      csv_items );
         print_vector3  ( out, results.polarizabilityEigenvalues, csv_items );
         print_scalar   ( out, results.meanPolarizability,        csv_items );
         print_scalar   ( out, results.hydrodynamicRadius,        csv_items );
         print_scalar   ( out, results.q_eta,                     csv_items );
         print_scalar   ( out, results.viscometricRadius,         csv_items );

         if ( results.intrinsicViscosityConventionalComputed ) {
            print_scalar( out, results.intrinsicViscosityConventional, csv_items );
         }

         if ( results.frictionCoefficientComputed ) {
            print_scalar( out, results.frictionCoefficient, csv_items );
         }

         if ( results.diffusionCoefficientComputed ) {
            print_scalar( out, results.diffusionCoefficient, csv_items );
         }

         if ( results.sedimentationCoefficientComputed ) {
            print_scalar( out, results.sedimentationCoefficient, csv_items );
         }
      }

      if ( results.resultsInteriorCompiled ) {
         print_scalar   ( out, results.volume,               csv_items );
         print_scalar   ( out, results.capacitanceOfASphere, csv_items );
         print_matrix3x3( out, results.gyrationTensor,       csv_items );
         print_vector3  ( out, results.gyrationEigenvalues,  csv_items );
      }

      if ( results.resultsZenoCompiled && results.resultsInteriorCompiled ) {
         print_scalar( out, results.intrinsicConductivity, csv_items );
         print_scalar( out, results.intrinsicViscosity,    csv_items );
      }
   }

   void print_time( std::ostream & out, std::string const & label ) {
      char   buffer[ 256 ];
      time_t rawtime;
      time( &rawtime );
      struct tm * timeinfo = localtime( &rawtime );
#if defined( Q_OS_WIN )
      // the Windows C runtime may not support %F and %T, so spell them out there
      size_t len = timeinfo ? strftime( buffer, sizeof( buffer ), "%Y-%m-%d %H:%M:%S", timeinfo ) : 0;
#else
      size_t len = timeinfo ? strftime( buffer, sizeof( buffer ), "%F %T", timeinfo ) : 0;
#endif
      if ( len ) {
         out << label << buffer << std::endl;
      }
   }

   // RAM in use, from /proc (Linux only; "unknown" elsewhere)
   void print_ram( std::ostream & out, std::string const & label, Csv_Items * csv_items ) {
      std::ifstream status_file( "/proc/self/status" );
      std::string   line;
      size_t        found = std::string::npos;

      while ( status_file.good() && found == std::string::npos ) {
         std::getline( status_file, line );
         found = line.find( "VmRSS:" );
      }

      std::string ram   = "unknown";
      std::string units = "";

      if ( found != std::string::npos ) {
         size_t units_pos = line.find_last_of( " " ) + 1;
         size_t ram_pos   = line.find_last_of( "\t :", units_pos - 2 ) + 1;
         ram   = line.substr( ram_pos, units_pos - ram_pos );
         units = line.substr( units_pos );
      }

      std::string csv_name = label + "_ram";
      std::replace( csv_name.begin(), csv_name.end(), ' ', '_' );

      print_exact_scalar( out, "RAM after " + label, csv_name, units, ram, csv_items );
   }

   void print_output( std::ostream                           & out,
                      zeno::Results const                    & results,
                      zeno::ParametersWalkOnSpheres const    & parameters_walk_on_spheres,
                      zeno::ParametersInteriorSampling const & parameters_interior_sampling,
                      zeno::ParametersResults const          & parameters_results,
                      ParametersLocal const                  & parameters_local,
                      zeno::Zeno const                       & zeno_calc,
                      double                                   read_time,
                      Csv_Items                              * csv_items ) {
      out << std::endl
          << "Parameters" << std::endl
          << "----------" << std::endl
          << std::endl;

      print_parameters( out, parameters_walk_on_spheres, parameters_interior_sampling,
                        parameters_results, parameters_local, csv_items );

      out << std::endl
          << "Results" << std::endl
          << "-------" << std::endl
          << std::endl;

      print_results( out, results, csv_items );

      if ( parameters_local.getPrintCounts() ) {
         out << "Counts:" << std::endl
             << std::endl;

         if ( results.resultsZenoCompiled ) {
            print_scalar   ( out, results.t, csv_items );
            print_vector3  ( out, results.u, csv_items );
            print_matrix3x3( out, results.v, csv_items );
            print_matrix3x3( out, results.w, csv_items );
         }

         if ( results.resultsInteriorCompiled ) {
            print_scalar( out, results.numInteriorHits, csv_items );
         }
      }

      if ( parameters_local.getPrintBenchmarks() ) {
         print_exact_scalar( out, "Initialize     ", "initialize_time", "s",
                             zeno_calc.getInitializeTime(), csv_items );
         print_exact_scalar( out, "Read           ", "read_time", "s",
                             read_time, csv_items );
         print_exact_scalar( out, "Broadcast      ", "broadcast_time", "s",
                             0e0, csv_items );
         print_exact_scalar( out, "Preprocess     ", "preprocess_time", "s",
                             zeno_calc.getPreprocessTime(), csv_items );
         print_exact_scalar( out, "Exterior Walk  ", "exterior_walk_time", "s",
                             zeno_calc.getWalkOnSpheresTime(), csv_items );
         print_exact_scalar( out, "Exterior Reduce", "exterior_reduce_time", "s",
                             zeno_calc.getWalkOnSpheresReductionTime(), csv_items );
         print_exact_scalar( out, "Volume Sample  ", "volume_sample_time", "s",
                             zeno_calc.getInteriorSamplingTime(), csv_items );
         print_exact_scalar( out, "Volume Reduce  ", "volume_reduce_time", "s",
                             zeno_calc.getInteriorSamplingReductionTime(), csv_items );
         print_exact_scalar( out, "Total Time     ", "total_time", "s",
                             zeno_calc.getTotalTime(), csv_items );
         out << std::endl;
      }
   }

   // Returns 0 on success.
   int write_points( std::ostream                                    & out,
                     std::string const                               & file_name,
                     std::vector < zeno::Vector3 < double > > const  * points,
                     std::vector < zeno::Vector3 < char > > const    * charges ) {
      std::ofstream output_file( file_name, std::ofstream::out );

      if ( !output_file.is_open() ) {
         out << "Error opening output file " << file_name << std::endl;
         return 1;
      }

      for ( size_t i = 0; i < points->size(); i++ ) {
         if ( charges != nullptr ) {
            output_file << charges->at( i ).get( 0 )
                        << charges->at( i ).get( 1 )
                        << charges->at( i ).get( 2 );
         }
         output_file << std::setw( 16 ) << points->at( i ).get( 0 )
                     << std::setw( 16 ) << points->at( i ).get( 1 )
                     << std::setw( 16 ) << points->at( i ).get( 2 ) << std::endl;
      }
      return 0;
   }

   void save_point_files( std::ostream & out, zeno::Zeno const & zeno_calc, ParametersLocal const & parameters_local ) {
      if ( !parameters_local.getSurfacePointsFileName().empty() ) {
         std::vector < zeno::Vector3 < double > > const * points  = nullptr;
         std::vector < zeno::Vector3 < char > > const   * charges = nullptr;

         zeno_calc.getWalkOnSpheresHitPoints( &points, &charges );

         if ( points == nullptr || charges == nullptr ) {
            out << "*** Warning ***" << std::endl
                << "A surface points file was requested but walks were not "
                << "performed.  Surface points file will not be written."
                << std::endl
                << std::endl;
         } else {
            write_points( out, parameters_local.getSurfacePointsFileName(), points, charges );
         }
      }

      if ( !parameters_local.getInteriorPointsFileName().empty() ) {
         std::vector < zeno::Vector3 < double > > const * points = nullptr;

         zeno_calc.getInteriorSamplingHitPoints( &points );

         if ( points == nullptr ) {
            out << "*** Warning ***" << std::endl
                << "An interior points file was requested but interior "
                << "samples were not performed.  Interior points file will "
                << "not be written."
                << std::endl
                << std::endl;
         } else {
            write_points( out, parameters_local.getInteriorPointsFileName(), points, nullptr );
         }
      }
   }

   // Writes the CSV items as rows down a column, as writeCsvOutputCols() in Main.cc.
   void write_csv_output_cols( Csv_Items const                & global_csv_items,
                               std::list < Csv_Items > const  & per_run_csv_items_list,
                               std::ofstream                  * csv_output_file ) {
      for ( size_t row = 0; row < global_csv_items.at( 0 ).size(); ++row ) {
         *csv_output_file << global_csv_items.at( 0 ).at( row ) << ","
                          << global_csv_items.at( 1 ).at( row );
         for ( size_t i = 0; i < per_run_csv_items_list.size(); ++i ) {
            *csv_output_file << "," << global_csv_items.at( 2 ).at( row );
         }
         *csv_output_file << std::endl;
      }

      for ( size_t row = 0; row < per_run_csv_items_list.back().at( 0 ).size(); ++row ) {
         *csv_output_file << per_run_csv_items_list.back().at( 0 ).at( row ) << ","
                          << per_run_csv_items_list.back().at( 1 ).at( row );
         for ( Csv_Items const & per_run_csv_items : per_run_csv_items_list ) {
            *csv_output_file << "," << per_run_csv_items.at( 2 ).at( row );
         }
         *csv_output_file << std::endl;
      }
   }

   // ---- one ZENO run, as runZeno() in Main.cc -------------------------------

   int run_zeno( std::ostream                     & out,
                 ParametersLocal const            & parameters_local,
                 zeno::ParametersWalkOnSpheres    * parameters_walk_on_spheres,
                 zeno::ParametersInteriorSampling * parameters_interior_sampling,
                 zeno::ParametersResults          * parameters_results,
                 double                             read_time,
                 zeno::MixedModel < double >      * model,
                 Somo_Zeno_Poll                   * poll,
                 Csv_Items                        * csv_items ) {
      zeno::Zeno zeno_calc( model );

      zeno_calc.setPollCallback( somo_zeno_poll, poll, 200 );

      if ( parameters_local.getPrintBenchmarks() ) {
         print_ram( out, "building spatial data structure", csv_items );
      }

      zeno::Zeno::Status status = zeno_calc.doWalkOnSpheres( parameters_walk_on_spheres, parameters_results );

      if ( status != zeno::Zeno::Status::Success ) {
         if ( status == zeno::Zeno::Status::EmptyModel ) {
            out << "Error: no geometry loaded" << std::endl;
         }
         return 1;
      }

      if ( parameters_local.getPrintBenchmarks() ) {
         print_ram( out, "walk on spheres", csv_items );
      }

      if ( !zeno_calc.getStopRequested() ) {
         status = zeno_calc.doInteriorSampling( parameters_interior_sampling, parameters_results );

         if ( status != zeno::Zeno::Status::Success ) {
            if ( status == zeno::Zeno::Status::EmptyModel ) {
               out << "Error: no geometry loaded" << std::endl;
            }
            return 1;
         }
      }

      if ( zeno_calc.getStopRequested() ) {
         out << "Stopped before the computation finished; no results" << std::endl;
         return 1;
      }

      if ( parameters_local.getPrintBenchmarks() ) {
         print_ram( out, "interior samples", csv_items );
      }

      if ( parameters_walk_on_spheres->getMaxRunTimeWasSet() &&
           zeno_calc.getTotalTime() > parameters_walk_on_spheres->getMaxRunTime() ) {
         out << std::endl
             << "*** Warning ***" << std::endl
             << "Max run time was reached.  Not all requested computations "
             << "may have been performed." << std::endl;
      }

      zeno::Results results;

      zeno_calc.getResults( parameters_results, &results );

      print_output( out, results, *parameters_walk_on_spheres, *parameters_interior_sampling,
                    *parameters_results, parameters_local, zeno_calc, read_time, csv_items );

      save_point_files( out, zeno_calc, parameters_local );

      return 0;
   }
}

// Runs ZENO with the given command line (zeno-cli options, e.g. -i file.bod
// --num-walks=N --num-threads=T --seed=S), writing the output to fname.
// is_cmdline selects progress by UDP message (zeno_us_udp_msg, may be null)
// instead of the SOMO GUI.  Returns 0 on success.
int zeno_cxx_main( int argc, char ** argv, const char * fname, bool is_cmdline, US_Udp_Msg * zeno_us_udp_msg ) {
   std::ofstream out( fname, std::ios::out | std::ios::trunc );

   if ( !out.is_open() ) {
      std::cerr << "zeno_cxx_main: can not open output file " << fname << std::endl;
      return 1;
   }

   zeno::ParametersWalkOnSpheres    parameters_walk_on_spheres;
   zeno::ParametersInteriorSampling parameters_interior_sampling;
   zeno::ParametersResults          parameters_results;
   ParametersLocal                  parameters_local;

   parameters_local.setMpiSize( 1 );
   parameters_local.setMpiRank( 0 );

   if ( parse_command_line( argc, argv, out,
                            &parameters_walk_on_spheres,
                            &parameters_interior_sampling,
                            &parameters_results,
                            &parameters_local ) ) {
      return 1;
   }

   print_time( out, "Start time: " );

   std::ofstream csv_output_file;

   if ( parameters_local.getCsvOutputFileNameWasSet() ) {
      csv_output_file.open( parameters_local.getCsvOutputFileName(), std::ios::out | std::ios::trunc );

      if ( !csv_output_file.is_open() ) {
         out << "*** Warning ***" << std::endl
             << "Could not open CSV output file "
             << parameters_local.getCsvOutputFileName() << std::endl
             << std::endl;
      }
   }

   Csv_Items                global_csv_items;
   std::list < Csv_Items >  per_run_csv_items_list;

   print_exact_scalar( out, "Version", "version", "", std::string( CMDLINE_PARSER_VERSION ), &global_csv_items );

   if ( parameters_local.getPrintBenchmarks() ) {
      print_ram( out, "initialization", &global_csv_items );
   }

   zeno::MixedModel < double > model;

   zeno::Timer read_timer;
   read_timer.start();

   {
      std::string   file_name = parameters_local.getInputFileName();
      std::ifstream input_file( file_name );

      if ( !input_file.is_open() ) {
         out << "Error opening bod input file " << file_name << std::endl;
         return 1;
      }

      Somo_Zeno_Bod_Parser parser( out,
                                   &parameters_walk_on_spheres,
                                   &parameters_interior_sampling,
                                   &parameters_results,
                                   &model );

      if ( parser.parse( input_file, file_name ) ) {
         out << "Error parsing bod input file " << file_name << std::endl;
         return 1;
      }
   }

   read_timer.stop();

   if ( parameters_local.getPrintBenchmarks() ) {
      print_ram( out, "loading input data", &global_csv_items );
   }

   Somo_Zeno_Poll poll = { is_cmdline, zeno_us_udp_msg };

   per_run_csv_items_list.emplace_back();

   int status = run_zeno( out,
                          parameters_local,
                          &parameters_walk_on_spheres,
                          &parameters_interior_sampling,
                          &parameters_results,
                          read_timer.getTime(),
                          &model,
                          &poll,
                          &per_run_csv_items_list.back() );

   if ( status ) {
      return status;
   }

   if ( csv_output_file.is_open() ) {
      write_csv_output_cols( global_csv_items, per_run_csv_items_list, &csv_output_file );
   }

   print_time( out, "End time: " );

   return 0;
}
