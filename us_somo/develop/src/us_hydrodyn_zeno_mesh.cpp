// us_hydrodyn_zeno_mesh.cpp -- the solvent-excluded surface of a bead model for ZENO,
// meshed with sesmesh (sesmesh/, see sesmesh/README.US-SOMO).

#include "../include/us_hydrodyn_zeno_mesh.h"

#include "../sesmesh/src/clean.h"
#include "../sesmesh/src/mesh.h"
#include "../sesmesh/src/parallel.h"
#include "../sesmesh/src/ses.h"

#include <QStringList>

#include <chrono>
#include <cstdio>
#include <exception>

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

bool us_hydrodyn_zeno_mesh(
                           const std::vector < double >         & spheres,
                           const US_Hydrodyn_Zeno_Mesh_Options  & options,
                           QTextStream                          & ts,
                           US_Hydrodyn_Zeno_Mesh_Info           & info,
                           QString                              & error
                           )
{
   info  = US_Hydrodyn_Zeno_Mesh_Info();
   error = "";

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

   try {
      auto start = std::chrono::steady_clock::now();

      sesmesh::set_threads( options.threads < 1 ? 1 : options.threads );

      sesmesh::SesModel model( balls, options.probe );

      sesmesh::MeshOptions mesh_options;
      mesh_options.h             = options.h;
      mesh_options.fill_cavities = options.fill_cavities;

      sesmesh::Mesh      mesh;
      sesmesh::BuildInfo build_info;
      sesmesh::build_mesh( model, mesh_options, mesh, build_info );

      if ( options.clean ) {
         sesmesh::CleanOptions clean_options;
         sesmesh::CleanInfo    clean_info;
         sesmesh::clean_mesh( model, options.h, clean_options, mesh, clean_info );
      }

      sesmesh::MeshCheck check = sesmesh::check_mesh( mesh );

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

      char buf[ 320 ];
      for ( const auto & t : mesh.tri ) {
         const sesmesh::Vec3 & a = mesh.x[ t[ 0 ] ];
         const sesmesh::Vec3 & b = mesh.x[ t[ 1 ] ];
         const sesmesh::Vec3 & c = mesh.x[ t[ 2 ] ];
         std::snprintf( buf, sizeof( buf ), "TRIANGLE %.8f %.8f %.8f %.8f %.8f %.8f %.8f %.8f %.8f\n",
                        a.x, a.y, a.z, b.x, b.y, b.z, c.x, c.y, c.z );
         ts << buf;
      }

      info.seconds = std::chrono::duration < double >( std::chrono::steady_clock::now() - start ).count();
   } catch ( const std::exception & e ) {
      error = QString( "sesmesh: %1" ).arg( e.what() );
      return false;
   }

   return true;
}
