#ifndef US_HYDRODYN_ZENO_MESH_H
#define US_HYDRODYN_ZENO_MESH_H

// Meshes the solvent-excluded surface (SES) of a set of spheres with sesmesh (sesmesh/,
// ehb54/sesmesh, MIT; see sesmesh/README.US-SOMO), so that ZENO can run on the surface of a
// bead model instead of on its spheres.  Used by US_Hydrodyn_Zeno::run.
//
// Kept in its own file so that sesmesh's headers stay away from the Fortran-translation
// macros of us_hydrodyn_zeno.cpp.

#include <QString>
#include <QTextStream>
#include <vector>

struct US_Hydrodyn_Zeno_Mesh_Options {
   double probe         = 1.5;    // probe radius, in the model's length unit
   double h             = 0.5;    // grid spacing, in the model's length unit
   bool   clean         = true;   // remove slender triangles (sesmesh's clean-up)
   bool   fill_cavities = true;   // mesh only the outer surface
   int    threads       = 1;
};

struct US_Hydrodyn_Zeno_Mesh_Info {
   int    spheres         = 0;      // spheres meshed (beads of radius > 0)
   int    skipped         = 0;      // beads of radius 0, left out
   long   vertices        = 0;
   long   triangles       = 0;
   double area            = 0;      // of the mesh
   double volume          = 0;      // enclosed by the mesh (internal cavities, if kept, subtracted)
   int    components      = 0;      // separate closed surfaces
   long   euler           = 0;      // Euler characteristic, 2 per surface without handles
   bool   closed_manifold = false;
   double min_angle       = 0;      // smallest triangle angle, degrees
   double seconds         = 0;
};

// The grid spacings of the "zeno_mesh_h" setting: one value, or several separated by commas,
// for a sequence of meshes extrapolated to an infinitely fine mesh.  Empty if any entry is not
// a positive number.
std::vector < double > us_hydrodyn_zeno_mesh_spacings( const QString & list );

// spheres holds x, y, z, r for each sphere.  Writes one line
// "TRIANGLE x1 y1 z1 x2 y2 z2 x3 y3 z3" per triangle to ts, counter-clockwise seen from outside.
// Returns false, with error set, if the spheres cannot be meshed.
bool us_hydrodyn_zeno_mesh(
                           const std::vector < double >         & spheres,
                           const US_Hydrodyn_Zeno_Mesh_Options  & options,
                           QTextStream                          & ts,
                           US_Hydrodyn_Zeno_Mesh_Info           & info,
                           QString                              & error
                           );

#endif
