// us_hydrodyn_zeno_mesh.cpp -- the solvent-excluded surface of a bead model for ZENO,
// meshed with sesmesh (sesmesh/, see sesmesh/README.US-SOMO).

#include "../include/us_hydrodyn_zeno_mesh.h"

#include "../sesmesh/src/clean.h"
#include "../sesmesh/src/mesh.h"
#include "../sesmesh/src/parallel.h"
#include "../sesmesh/src/ses.h"

#include <QIODevice>
#include <QStringList>

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <exception>
#include <stdexcept>
#include <string>

namespace {

// One mesh at a time: a progress function that processes events could otherwise start another,
// which would wait forever for sesmesh's thread pool.
bool us_hydrodyn_zeno_mesh_running = false;

}

std::vector < double > us_hydrodyn_zeno_mesh_spacings( const QString & list )
{
   std::vector < double > spacings;
   const QStringList parts = list.split( ",", Qt::SkipEmptyParts );
   for ( const QString & part : parts ) {
      bool ok = false;
      double h = part.trimmed().toDouble( &ok );
      if ( !ok || !( h > 0 ) ) {
         return std::vector < double >();
      }
      spacings.push_back( h );
   }
   return spacings;
}

US_Hydrodyn_Zeno_Mesh_Options us_hydrodyn_zeno_mesh_options(
                                                            const std::map < QString, QString > & gparams,
                                                            double                                h,
                                                            double                                to_units,
                                                            int                                   threads
                                                            )
{
   auto setting = [ & ]( const char * key ) {
      auto it = gparams.find( key );
      return it == gparams.end() ? QString() : it->second;
   };

   US_Hydrodyn_Zeno_Mesh_Options options;
   options.probe         = setting( "zeno_mesh_probe" ).toDouble() * to_units;
   options.h             = h * to_units;
   options.clean         = setting( "zeno_mesh_clean" ) != "false";
   options.fill_cavities = setting( "zeno_mesh_fill_cavities" ) != "false";
   options.threads       = threads;
   return options;
}

bool us_hydrodyn_zeno_mesh(
                           const std::vector < double >           & spheres,
                           const US_Hydrodyn_Zeno_Mesh_Options    & options,
                           QTextStream                            & ts,
                           US_Hydrodyn_Zeno_Mesh_Info             & info,
                           QString                                & error,
                           const US_Hydrodyn_Zeno_Mesh_Progress   & progress
                           )
{
   info  = US_Hydrodyn_Zeno_Mesh_Info();
   error = "";

   if ( us_hydrodyn_zeno_mesh_running ) {
      error = "another mesh is being made; wait for it or stop it";
      return false;
   }

   if ( !( options.probe >= 0 ) ) {
      error = QString( "the probe radius must be zero or positive (it is %1)" ).arg( options.probe );
      return false;
   }
   if ( !( options.h > 0 ) ) {
      error = QString( "the grid spacing must be positive (it is %1)" ).arg( options.h );
      return false;
   }

   std::vector < sesmesh::Ball > balls;
   balls.reserve( spheres.size() / 4 );
   for ( size_t i = 0; i + 3 < spheres.size(); i += 4 ) {
      if ( spheres[ i + 3 ] > 0 ) {
         balls.push_back( { sesmesh::Vec3( spheres[ i ], spheres[ i + 1 ], spheres[ i + 2 ] ), spheres[ i + 3 ] } );
      } else {
         ++info.skipped;
      }
   }
   info.spheres = (int) balls.size();
   if ( balls.empty() ) {
      error = "no beads with a radius above zero";
      return false;
   }

   struct Running {
      Running()  { us_hydrodyn_zeno_mesh_running = true; }
      ~Running() { us_hydrodyn_zeno_mesh_running = false; }
   } running;

   // The progress of each sesmesh call, as a part [from, to] of the whole: on 1AO6 (16 threads) the
   // mesh takes about 40% of the time, the clean-up 48%, the check 5% and writing the lines the
   // rest (without the clean-up: the mesh 70%, the check 8%, writing the rest).
   auto part = [ & ]( double from, double to, const QString & what ) -> sesmesh::ProgressFn {
      if ( !progress ) {
         return sesmesh::ProgressFn();
      }
      return [ &progress, from, to, what ]( double fraction, const char * stage ) {
         return progress( from + ( to - from ) * fraction, what.isEmpty() ? QString( stage ) : what + ", " + stage );
      };
   };
   const double mesh_to  = options.clean ? 0.40 : 0.70;
   const double clean_to = options.clean ? 0.88 : 0.70;
   const double check_to = options.clean ? 0.93 : 0.78;

   try {
      auto start = std::chrono::steady_clock::now();

      if ( progress && !progress( 0, "setup" ) ) {
         throw sesmesh::Cancelled();
      }

      sesmesh::set_threads( options.threads < 1 ? 1 : options.threads );

      sesmesh::SesModel model( balls, options.probe );

      sesmesh::MeshOptions mesh_options;
      mesh_options.h             = options.h;
      mesh_options.fill_cavities = options.fill_cavities;
      mesh_options.progress      = part( 0, mesh_to, "mesh" );

      sesmesh::Mesh      mesh;
      sesmesh::BuildInfo build_info;
      sesmesh::build_mesh( model, mesh_options, mesh, build_info );

      if ( options.clean ) {
         sesmesh::CleanOptions clean_options;
         sesmesh::CleanInfo    clean_info;
         clean_options.progress = part( mesh_to, clean_to, "clean-up" );
         sesmesh::clean_mesh( model, options.h, clean_options, mesh, clean_info );
      }

      sesmesh::MeshCheck check = sesmesh::check_mesh( mesh, part( clean_to, check_to, "" ) );

      info.vertices        = check.vertices - check.unused_vertices;
      info.triangles       = check.triangles;
      info.area            = check.area;
      info.volume          = check.volume;
      info.components      = (int) check.components.size();
      info.euler           = check.euler;
      info.closed_manifold = check.closed_manifold();
      info.min_angle       = check.min_angle;

      if ( mesh.tri.empty() ) {
         error = "the mesh has no triangles";
         return false;
      }

      // The TRIANGLE lines: formatted on sesmesh's threads, a few blocks per thread at a time, and
      // written in order straight to the stream's device (formatting them on one thread took a
      // quarter of the time).
      sesmesh::Progress write_progress( part( check_to, 1, "" ) );
      write_progress.stage( "writing", 0, 1 );
      const long triangles = (long) mesh.tri.size();
      const long block     = 2048;
      const long blocks    = ( triangles + block - 1 ) / block;
      const long batch     = 2 * std::max( 1, sesmesh::threads() );
      std::vector < std::string > text( batch );
      QIODevice * device = ts.device();
      ts.flush();
      for ( long first = 0; first < blocks; first += batch ) {
         const long count = std::min( batch, blocks - first );
         sesmesh::parallel_for( count, [ & ]( long i, int ) {
               std::string & s = text[ i ];
               s.clear();
               char buf[ 320 ];
               const long end = std::min( triangles, ( first + i + 1 ) * block );
               for ( long k = ( first + i ) * block; k < end; ++k ) {
                  const auto & t = mesh.tri[ k ];
                  const sesmesh::Vec3 & a = mesh.x[ t[ 0 ] ];
                  const sesmesh::Vec3 & b = mesh.x[ t[ 1 ] ];
                  const sesmesh::Vec3 & c = mesh.x[ t[ 2 ] ];
                  int n = std::snprintf( buf, sizeof( buf ), "TRIANGLE %.8f %.8f %.8f %.8f %.8f %.8f %.8f %.8f %.8f\n",
                                         a.x, a.y, a.z, b.x, b.y, b.z, c.x, c.y, c.z );
                  s.append( buf, (size_t) std::max( 0, std::min( n, (int) sizeof( buf ) - 1 ) ) );
               }
            } );
         for ( long i = 0; i < count; ++i ) {
            if ( device ) {
               if ( device->write( text[ i ].data(), (qint64) text[ i ].size() ) != (qint64) text[ i ].size() ) {
                  throw std::runtime_error( "could not write the mesh: " + device->errorString().toStdString() );
               }
            } else {
               ts << QLatin1String( text[ i ].data(), (int) text[ i ].size() );
            }
         }
         write_progress.at( double( std::min( triangles, ( first + count ) * block ) ) / double( triangles ) );
      }

      info.seconds = std::chrono::duration < double >( std::chrono::steady_clock::now() - start ).count();
   } catch ( const sesmesh::Cancelled & ) {
      info.stopped = true;
      error = "stopped";
      return false;
   } catch ( const std::exception & e ) {
      error = QString( "sesmesh: %1" ).arg( e.what() );
      return false;
   }

   return true;
}
