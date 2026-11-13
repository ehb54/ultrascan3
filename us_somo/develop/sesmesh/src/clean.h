// sesmesh: mesh clean-up.
//
// Marching tetrahedra leaves many slender triangles: needles, with one very short edge where
// the surface passes close to a grid node, and caps, with one angle near 180 degrees.  The
// clean-up removes them without moving any vertex off the exact surface.
//
//  - Edge collapse: an edge shorter than a fraction of h is merged to one vertex, placed at
//    either end or at the edge's midpoint projected onto the surface (a Newton step on f,
//    exact while the nearest feature stays the same), whichever leaves the best smallest
//    angle.  The link condition (the two ends share exactly the two opposite vertices) keeps
//    the mesh a manifold of the same topology.  A collapse is made only if it improves the
//    smallest angle around it, and no triangle turns over.
//  - Edge flip: the diagonal of two triangles is swapped when that raises their smallest
//    angle and the two are nearly coplanar, so the surface hardly moves.
//  - Tangential smoothing: a vertex moves towards its neighbours' centroid within the tangent
//    plane and is projected back onto the surface, when that raises the smallest angle around it.
#pragma once

#include "mesh.h"
#include "ses.h"

namespace sesmesh {

struct CleanOptions {
   double collapse = 0.3;      // collapse edges shorter than this times h
   double good_angle = 20;     // a collapse may lower the smallest angle around it if it stays above this (degrees)
   double flip_dihedral = 20;  // flip only between triangles whose normals differ by less (degrees)
   double target_angle = 30;   // flips and smoothing work only around triangles with an angle under this (degrees)
   int smooth = 4;             // passes with tangential smoothing
   int passes = 8;
};

struct CleanInfo {
   long collapses = 0, flips = 0, smoothed = 0;
   long rejected_link = 0, rejected_shape = 0;
   long deferred = 0;
   int passes = 0;   // collapses left to the serial pass: their neighbourhood crosses a cell border
   double seconds = 0, seconds_collapse = 0, seconds_flip = 0, seconds_smooth = 0;
};

void clean_mesh( const SesModel& model, double h, const CleanOptions& opt, Mesh& mesh, CleanInfo& info );

}  // namespace sesmesh
