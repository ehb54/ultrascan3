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
#include <functional>
#include <map>
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
   bool   stopped         = false;  // the progress function asked to stop
};

// The grid spacings of the "zeno_mesh_h" setting: one value, or several separated by commas,
// for a sequence of meshes extrapolated to an infinitely fine mesh.  Empty if any entry is not
// a positive number.
std::vector < double > us_hydrodyn_zeno_mesh_spacings( const QString & list );

// The options of the zeno_mesh_probe, zeno_mesh_clean and zeno_mesh_fill_cavities settings, with
// grid spacing h.  The probe radius and h are in Angstrom and are multiplied by to_units (0.1 for
// a model in nm).
US_Hydrodyn_Zeno_Mesh_Options us_hydrodyn_zeno_mesh_options(
                                                            const std::map < QString, QString > & gparams,
                                                            double                                h,
                                                            double                                to_units,
                                                            int                                   threads
                                                            );

// progress( fraction, stage ): the fraction of the meshing done (0 to 1, an estimate) and the
// name of its stage, about every 0.1 s while it runs, on the calling thread.  Returning false
// stops the meshing.  The function may process events, but must not start another mesh: a call
// made while one runs fails.
typedef std::function < bool ( double fraction, const QString & stage ) > US_Hydrodyn_Zeno_Mesh_Progress;

// spheres holds x, y, z, r for each sphere.  Writes one line
// "TRIANGLE x1 y1 z1 x2 y2 z2 x3 y3 z3" per triangle to ts, counter-clockwise seen from outside.
// Returns false, with error set, if the spheres cannot be meshed.
// With a progress function that asks to stop, returns false with info.stopped set.
bool us_hydrodyn_zeno_mesh(
                           const std::vector < double >           & spheres,
                           const US_Hydrodyn_Zeno_Mesh_Options    & options,
                           QTextStream                            & ts,
                           US_Hydrodyn_Zeno_Mesh_Info             & info,
                           QString                                & error,
                           const US_Hydrodyn_Zeno_Mesh_Progress   & progress = US_Hydrodyn_Zeno_Mesh_Progress()
                           );

// One mesh of a series of meshes of one model (several grid spacings), for
// us_hydrodyn_zeno_mesh_series_check().
struct US_Hydrodyn_Zeno_Mesh_Series_Mesh {
   double h;           // grid spacing
   long   triangles;
   double area;
   double volume;
};

// How well a quantity (area or volume) of a series follows a straight line in 1/N (N, the number
// of triangles): the largest distance of a mesh's value from the least-squares line, relative to
// the value.
struct US_Hydrodyn_Zeno_Mesh_Series_Trend {
   bool   checked   = false;   // three or more spacings
   double deviation = 0;       // largest relative distance from the line (0.002: 0.2%)
};

// The mesh area and volume should change smoothly with the mesh, about linearly in 1/N, which is
// what the extrapolation assumes.  If between two spacings a narrow channel or a cavity opens or
// closes, the body itself changes and the series leaves the line.  Returns an empty string if the
// area and the volume stay within 0.2% of their lines, or with fewer than three spacings; else a
// warning.  unit: the model's length unit, for the warning.
QString us_hydrodyn_zeno_mesh_series_check(
                                           std::vector < US_Hydrodyn_Zeno_Mesh_Series_Mesh > meshes,
                                           const QString                                    & unit,
                                           US_Hydrodyn_Zeno_Mesh_Series_Trend               * area_trend   = nullptr,
                                           US_Hydrodyn_Zeno_Mesh_Series_Trend               * volume_trend = nullptr
                                           );

#endif
