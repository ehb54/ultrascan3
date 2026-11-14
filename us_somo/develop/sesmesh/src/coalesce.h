// sesmesh rcoal: Coalesce, the mesh coarsener of BEST's rcoal, ported to C++.
//
// Copyright (c) 1998-2025 Sergio R. Aragon (Coalesce and rcoal, in ehb54/best-hydro)
// Copyright (c) 2026 Emre H. Brookes (this port, and the fixes of best-hydro PRs #1 and #2)
// MIT licence: see LICENSE.
//
// Coalesce coarsens a triangulated surface in two passes.  Each pass collapses the edges
// shorter than e times the mean edge length, shortest first, onto the end that has more
// triangles.  Between the passes the edge list is rebuilt, and the second pass then flips the
// long edge of each slender obtuse triangle.  No vertex moves: every output vertex is an input
// vertex.  e is the edge fraction (edgfrac).
//
// This is a port of scoalesce.f and scoalesce3.f as fixed in best-hydro PRs #1 and #2:
//  - a collapse is made only if the link condition holds;
//  - a flip is made only if its new edge does not exist yet;
//  - edges of zero length are always collapsed.
// It gives the same triangles as the Fortran, in the same order, because it keeps the
// Fortran's arithmetic and its order of operations; build it with -ffp-contract=off so the
// compiler does not fuse multiplies and adds.  It replaces the Fortran's searches through all
// triangles with a list of triangles for each vertex, kept in ascending order, so that every
// loop still visits triangles in the Fortran's order.  It runs in about T log T time, where the
// Fortran takes T^2.
#pragma once

#include <array>
#include <string>
#include <vector>

namespace sesmesh::rcoal {

using V3 = std::array<double, 3>;
using T3 = std::array<int, 3>;

// A surface as rcoal reads it.  Vertex numbers are 1-based, as in the input files; x[0] is
// unused.
struct Surface {
   std::vector<V3> x;                     // x[1..nvert]
   std::vector<std::array<int, 2>> edges; // the edge list for the first pass
   std::vector<T3> tri;                   // vertex numbers of each triangle
   std::vector<int> comp;                 // component of each triangle; only 1 is kept
   double begin_area = 0;                 // total area of the input triangles
   double true_area = 0, true_vol = 0;    // written on the first line of each output
   bool sanner = false;                   // MSMS input
   double poly_area = 0, poly_vol = 0;    // MSMS: the .msms.log's numerical area and volume
   std::string input_name;                // as given on the command line
};

struct Stats {
   double edgfrac = 0;
   int ntriang = 0, idegen = 0;
   int idelete1 = 0, npolygon1 = 0;
   double totarea1 = 0;
   int idelete = 0, npoly = 0, npolygon = 0;
   double totarea = 0, avarea = 0;
   double avelength = 0;           // sum of the edge lengths of the second pass
   int nedge = 0;                  // edges in the second pass
   double cutofflength = 0;
   int nedge1 = 0, nedge2 = 0;     // edges of pass 1 (the input's) and pass 2
   int nobtuse = 0, icon = 0;
   int nlinkskip = 0, nflipskip = 0;
};

struct Result {
   std::vector<T3> tri;       // the output triangles, in output order
   std::vector<double> area;  // their areas
   Stats st;
};

// Runs Coalesce with edge fraction edgfrac.  If log is not null, it receives the progress lines
// the Fortran prints as it works (list-directed records, without newlines).
Result coalesce( const Surface& s, double edgfrac, std::vector<std::string>* log = nullptr );

// The edge fraction whose output is closest to target triangles, found by bisection: from
// [0.05, 0.7], widened by factors of 1.5 while the upper end still leaves too many triangles,
// then up to 40 halvings, stopping early on an exact hit.  This replaces rcoal's two-run fit of
// N(e) = N0 / (1 + a e^p), which can miss the target by 20% on marching-cubes and
// marching-tetrahedra input.  If n is not null it receives the number of triangles reached.
double find_edgfrac( const Surface& s, int target, int* n = nullptr );

// The Fortran's area of a triangle (subroutine areas).
double tri_area( const V3& a, const V3& b, const V3& c );

// Numerical Recipes' heapsort index (index.f): array[indx[j]] ascending, 0-based.  It is not
// stable; the order it gives equal lengths decides which edge is collapsed first.
void heap_index( const std::vector<double>& array, std::vector<int>& indx );

}  // namespace sesmesh::rcoal
