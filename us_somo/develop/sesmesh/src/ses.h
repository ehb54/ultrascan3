// sesmesh: the solvent-excluded surface of a set of spheres as an exact signed function.
//
// Atoms have centres c_i and radii r_i; the probe radius is p.  Let U be the union of the
// balls of radius R_i = r_i + p, the solvent-accessible solid.  Every probe centre that does
// not overlap an atom lies outside U, so a point x belongs to the solvent-excluded solid when
// its distance to the complement of U is at least p.  The function
//
//     f(x) = p - dist(x, boundary of U)    for x inside U,
//     f(x) = p + dist(x, U)                for x outside U,
//
// is p minus the signed distance to the boundary of U.  It is continuous and 1-Lipschitz,
// negative inside the solvent-excluded solid and positive outside it, and its zero set is
// exactly the solvent-excluded surface of Connolly (1983), cusps included.
//
// The point of the boundary of U nearest to an interior point x lies on
//   - an exposed sphere patch: it is then the radial projection of x onto that sphere;
//   - an exposed circle where two spheres meet: it is then the point of that circle nearest x;
//   - an exposed point where three spheres meet.
// Each such point is a point of the boundary, so dist(x, boundary) is the smallest distance
// from x to the candidates of these three kinds that lie inside no other ball.  The exposed
// arcs of every circle (and with them the exposed triple points, which are the arc ends) are
// computed once; a spatial hash of the spheres finds the candidates near a point.
#pragma once

#include <vector>

#include "vec3.h"

namespace sesmesh {

struct Ball {
   Vec3 c;
   double r;
};

// The kind of feature of the boundary of U that is nearest to a point: where the probe that
// defines the surface there touches one atom (contact), two (toroidal) or three (reentrant).
enum Feature : int { F_NONE = 0, F_SPHERE = 1, F_CIRCLE = 2, F_TRIPLE = 3 };

struct EvalInfo {
   Vec3 q;            // nearest point of the boundary of U (for x outside U: nearest point of U)
   int feature = F_NONE;
   int atom = -1;     // input index of the atom of that sphere (circle, triple: one of them)
   bool inside = false;   // x inside U
};

// Uniform-grid hash of points.
class PointHash {
 public:
   void build( const std::vector<Vec3>& pts, double cell );
   // Calls fn(index) for every point in the cells that a ball of radius reach around x touches.
   template <class Fn>
   void visit( const Vec3& x, double reach, Fn&& fn ) const;

 private:
   Vec3 lo_;
   double cell_ = 1;
   int dim_[3] = { 0, 0, 0 };
   std::vector<long> start_;
   std::vector<int> idx_;
};

template <class Fn>
void PointHash::visit( const Vec3& x, double reach, Fn&& fn ) const {
   if ( idx_.empty() ) {
      return;
   }
   int lo[3], hi[3];
   for ( int a = 0; a < 3; ++a ) {
      double l = ( x[a] - reach - lo_[a] ) / cell_;
      double h = ( x[a] + reach - lo_[a] ) / cell_;
      if ( h < 0 || l >= dim_[a] ) {
         return;
      }
      lo[a] = l < 0 ? 0 : int( l );
      hi[a] = h >= dim_[a] ? dim_[a] - 1 : int( h );
   }
   for ( int k = lo[2]; k <= hi[2]; ++k ) {
      for ( int j = lo[1]; j <= hi[1]; ++j ) {
         long row = ( long( k ) * dim_[1] + j ) * dim_[0];
         for ( long c = row + lo[0]; c <= row + hi[0]; ++c ) {
            for ( long n = start_[c]; n < start_[c + 1]; ++n ) {
               fn( idx_[n] );
            }
         }
      }
   }
}

class SesModel {
 public:
   SesModel( const std::vector<Ball>& atoms, double probe );

   double probe() const { return p_; }
   int atoms() const { return natoms_; }
   // Bounding box of U.
   Vec3 lo() const { return lo_; }
   Vec3 hi() const { return hi_; }

   // Spheres that a query with clamp dmax at points within `radius` of x may need:
   // all spheres with |x - c| < R + radius.  Pass them to eval().
   void gather( const Vec3& x, double radius, std::vector<int>& out ) const;

   // f(x) as defined above, with the distance clamped at dmax: the result is
   // p - min(dist(x, boundary of U), dmax) inside U and p + min(dist(x, U), dmax) outside.
   // cand must hold every sphere with |x - c| < R + dmax (gather() with radius >= dmax).
   double eval( const Vec3& x, const std::vector<int>& cand, double dmax, EvalInfo* info = nullptr ) const;
   // The same, gathering the candidates itself.
   double eval( const Vec3& x, double dmax, EvalInfo* info = nullptr ) const;

   // Counts, for reporting.
   int spheres_kept() const { return int( s_.size() ); }
   int spheres_buried() const;
   int circles_exposed() const { return int( circ_.size() ); }
   int arcs_exposed() const { return int( arcs_.size() ); }

 private:
   struct Sph {
      Vec3 c;
      double R, R2;
      int atom;
      bool buried;            // no exposed area
      int nb_begin, nb_end;   // neighbours (overlapping spheres) in nbr_
      int cir_begin, cir_end; // exposed circles with this sphere as the first (lower) index
   };
   struct Arc {
      double lo, hi;          // 0 <= lo < 2 pi, lo < hi <= lo + 2 pi
   };
   struct Circle {
      Vec3 m, n, u, v;        // centre, axis (from sphere i to j), in-plane basis
      double rho;
      int i, j;
      bool full;              // the whole circle is exposed
      int arc_begin, arc_end; // exposed arcs in arcs_ (none when full)
      int tp_begin, tp_end;   // exposed triple points (the arc ends) in tp_
   };

   bool exposed_on_sphere( int i, const Vec3& q ) const;
   bool in_arcs( const Circle& c, double th ) const;

   double p_;
   int natoms_;
   double rmax_ = 0;
   Vec3 lo_, hi_;
   std::vector<Sph> s_;
   std::vector<int> nbr_;
   std::vector<Circle> circ_;
   std::vector<Arc> arcs_;
   std::vector<Vec3> tp_;
   PointHash hash_;
};

}  // namespace sesmesh
