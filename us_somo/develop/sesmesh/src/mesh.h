// sesmesh: triangulation of the zero set of the solvent-excluded surface function.
//
// The function f of ses.h is sampled on a grid of spacing h, but only in a narrow band
// around the surface: the grid is cut into bricks of 8x8x8 nodes, and since f is
// 1-Lipschitz, one value at a brick's centre tells whether the whole brick is clearly
// outside, clearly inside or near the surface.  Only the band bricks are sampled node by
// node.
//
// A flood fill from the edge of the grid through the outside nodes then marks what solvent
// can reach.  Outside nodes it does not reach lie in internal cavities, and are counted as
// inside, so cavities are filled and only the outer surface is kept.
//
// Each grid cube is split into six tetrahedra that all share the cube's main diagonal
// (the Freudenthal split, consistent between neighbouring cubes), and each tetrahedron cut
// by the surface contributes one triangle or a quadrilateral split into two (marching
// tetrahedra).  A vertex is placed on each cut edge at a root of the exact f, found by a
// bracketing iteration, so every vertex lies on the true surface.  The triangles of a
// tetrahedron stay inside it, so the mesh has no self-intersections, and the cut of a
// sign field on a consistent tetrahedral grid is a closed, oriented 2-manifold.
#pragma once

#include <array>
#include <vector>

#include "ses.h"

namespace sesmesh {

struct Mesh {
   std::vector<Vec3> x;                  // vertices, on the surface
   std::vector<Vec3> n;                  // unit outward normals (the gradient of f)
   std::vector<int> atom;                // input index of the nearest atom touched by the probe
   std::vector<int> feature;             // Feature of that contact (see ses.h)
   std::vector<std::array<int, 3>> tri;  // counter-clockwise seen from outside
};

struct MeshOptions {
   double h = 0.2;              // grid spacing (A)
   bool fill_cavities = true;
};

struct BuildInfo {
   int nodes_per_axis[3] = { 0, 0, 0 };
   long bricks = 0, band_bricks = 0, out_bricks = 0, in_bricks = 0;
   long node_evals = 0;      // nodes where f was evaluated
   long root_evals = 0;      // evaluations of f while finding vertices
   long cavity_nodes = 0;    // outside nodes in cavities, counted as inside
   double seconds_classify = 0, seconds_nodes = 0, seconds_fill = 0, seconds_vertices = 0, seconds_triangles = 0;
};

// Throws std::runtime_error on failure.
void build_mesh( const SesModel& model, const MeshOptions& opt, Mesh& mesh, BuildInfo& info );

struct ComponentInfo {
   long triangles;
   double area, volume;   // volume is signed: negative for a cavity's surface
};

struct MeshCheck {
   long vertices = 0, unused_vertices = 0, triangles = 0, edges = 0;
   long open_edges = 0;          // edges of one triangle
   long nonmanifold_edges = 0;   // edges of more than two triangles
   long misoriented_edges = 0;   // edges of two triangles that run the same way
   long nonmanifold_vertices = 0;
   long degenerate = 0;          // triangles of zero area
   long euler = 0;
   double area = 0, volume = 0;
   double min_angle = 0, median_min_angle = 0, below_10 = 0, below_20 = 0;   // degrees; fractions
   std::vector<ComponentInfo> components;   // by number of triangles, largest first

   bool closed_manifold() const {
      return open_edges == 0 && nonmanifold_edges == 0 && misoriented_edges == 0 && nonmanifold_vertices == 0;
   }
};

MeshCheck check_mesh( const Mesh& mesh );

// Area and volume only (the polyhedron's), as check_mesh computes them.
void mesh_area_volume( const Mesh& mesh, double& area, double& volume );

}  // namespace sesmesh
