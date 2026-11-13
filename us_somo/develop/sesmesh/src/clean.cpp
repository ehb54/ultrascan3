// sesmesh: mesh clean-up; see clean.h.
#include "clean.h"

#include "parallel.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <utility>
#include <vector>

namespace sesmesh {

namespace {

const double PI = 3.14159265358979323846;

double now() {
   return std::chrono::duration<double>( std::chrono::steady_clock::now().time_since_epoch() ).count();
}

// Smallest angle of a triangle (radians); 0 if degenerate.
double min_angle( const Vec3& A, const Vec3& B, const Vec3& C ) {
   Vec3 ab = B - A, ac = C - A, bc = C - B;
   double cr = norm( cross( ab, ac ) );
   if ( !( cr > 0 ) ) {
      return 0;
   }
   double a1 = std::atan2( cr, dot( ab, ac ) );
   double a2 = std::atan2( cr, -dot( ab, bc ) );
   return std::min( a1, std::min( a2, PI - a1 - a2 ) );
}

// Largest angle of a triangle (radians); pi if degenerate.
double max_angle( const Vec3& A, const Vec3& B, const Vec3& C ) {
   Vec3 ab = B - A, ac = C - A, bc = C - B;
   double cr = norm( cross( ab, ac ) );
   if ( !( cr > 0 ) ) {
      return PI;
   }
   double a1 = std::atan2( cr, dot( ab, ac ) );
   double a2 = std::atan2( cr, -dot( ab, bc ) );
   return std::max( a1, std::max( a2, PI - a1 - a2 ) );
}

// Moves x onto the surface: for x inside U at distance d from its nearest boundary point q,
// x + (q - x)(d - p)/d is at distance p from q, on the surface if q stays the nearest point.
bool project( const SesModel& model, double dmax, Vec3& x, Vec3& n, int& atom, int& feature ) {
   const double p = model.probe();
   if ( !( p > 0 ) ) {
      return false;
   }
   for ( int it = 0; it < 8; ++it ) {
      EvalInfo inf;
      double f = model.eval( x, dmax, &inf );
      if ( !inf.inside || inf.feature == F_NONE ) {
         return false;
      }
      Vec3 dir = inf.q - x;
      double d = norm( dir );
      if ( !( d > 0 ) ) {
         return false;
      }
      if ( std::fabs( f ) < 1e-11 ) {
         n = dir / d;
         atom = inf.atom;
         feature = inf.feature;
         return true;
      }
      x = x + dir * ( ( d - p ) / d );
   }
   return false;
}

struct Dyn {
   Mesh& m;
   std::vector<char> talive, valive;
   std::vector<std::vector<int>> vt;   // alive triangles at each vertex (pruned lazily)

   explicit Dyn( Mesh& mesh ) : m( mesh ) {
      talive.assign( m.tri.size(), 1 );
      valive.assign( m.x.size(), 1 );
      vt.resize( m.x.size() );
      for ( size_t t = 0; t < m.tri.size(); ++t ) {
         for ( int k = 0; k < 3; ++k ) {
            vt[m.tri[t][k]].push_back( int( t ) );
         }
      }
   }
   void prune( int v ) {
      std::vector<int>& l = vt[v];
      l.erase( std::remove_if( l.begin(), l.end(), [&]( int t ) { return !talive[t]; } ), l.end() );
   }
   static bool has( const std::array<int, 3>& t, int v ) { return t[0] == v || t[1] == v || t[2] == v; }
   void neighbours( int v, std::vector<int>& out ) {
      prune( v );
      out.clear();
      for ( int t : vt[v] ) {
         for ( int k = 0; k < 3; ++k ) {
            if ( m.tri[t][k] != v ) {
               out.push_back( m.tri[t][k] );
            }
         }
      }
      std::sort( out.begin(), out.end() );
      out.erase( std::unique( out.begin(), out.end() ), out.end() );
   }
   // The two triangles on edge (a, b); false unless there are exactly two.
   bool edge_tris( int a, int b, int& t1, int& t2 ) {
      prune( a );
      t1 = t2 = -1;
      int n = 0;
      for ( int t : vt[a] ) {
         if ( has( m.tri[t], b ) ) {
            ( n == 0 ? t1 : t2 ) = t;
            ++n;
         }
      }
      return n == 2;
   }
   static int third( const std::array<int, 3>& t, int a, int b ) {
      for ( int k = 0; k < 3; ++k ) {
         if ( t[k] != a && t[k] != b ) {
            return t[k];
         }
      }
      return -1;
   }
};

}  // namespace

void clean_mesh( const SesModel& model, double h, const CleanOptions& opt, Mesh& m, CleanInfo& info ) {
   double t0 = now();
   info = CleanInfo();
   Dyn d( m );
   const double dmax = model.probe() + 2 * h;
   const double lmax = opt.collapse * h;
   const double tiny = 1e-3 * h;
   const double cos_flip = std::cos( opt.flip_dihedral * PI / 180 );
   const double good = opt.good_angle * PI / 180;
   const double target = opt.target_angle * PI / 180;
   const double cap_angle = 150 * PI / 180;
   std::vector<int> na, nb, nc, around;

   // Smallest angle of triangle t with vertex `from` moved to P (from < 0: as it is).
   auto angle_with = [&]( int t, int from, const Vec3& P ) {
      const std::array<int, 3>& tr = m.tri[t];
      Vec3 X[3];
      for ( int k = 0; k < 3; ++k ) {
         X[k] = tr[k] == from ? P : m.x[tr[k]];
      }
      return min_angle( X[0], X[1], X[2] );
   };
   // True if moving `from` to P (with surface normal nP) keeps triangle t facing the same way.
   auto keeps_facing = [&]( int t, int from, const Vec3& P, const Vec3& nP ) {
      const std::array<int, 3>& tr = m.tri[t];
      Vec3 X[3], Y[3];
      for ( int k = 0; k < 3; ++k ) {
         X[k] = m.x[tr[k]];
         Y[k] = tr[k] == from ? P : X[k];
      }
      Vec3 n0 = cross( X[1] - X[0], X[2] - X[0] ), n1 = cross( Y[1] - Y[0], Y[2] - Y[0] );
      double l0 = norm( n0 ), l1 = norm( n1 );
      if ( !( l1 > 0 ) ) {
         return false;
      }
      // A triangle that turns by less than about 25 degrees cannot fold over, whatever the surface
      // normal at P says (at a sharp crease it can be far from both sides' triangles).
      if ( l0 >= 1e-12 * l1 && dot( n0, n1 ) > 0.9 * l0 * l1 ) {
         return true;
      }
      if ( dot( n1, nP ) <= 0 ) {
         return false;
      }
      return l0 < 1e-12 * l1 || dot( n0, n1 ) > 0.5 * l0 * l1;
   };

   for ( int pass = 0; pass < opt.passes; ++pass ) {
      long collapsed = 0, flipped = 0;
      ++info.passes;

      double ts = now();
      // Edge collapses, shortest first.
      // Short edges, and the two edges at the obtuse corner of a degenerate cap (whatever their length).
      std::vector<std::pair<double, std::pair<int, int>>> edges;
      {
         std::vector<std::vector<std::pair<double, std::pair<int, int>>>> mine_t( threads() );
         parallel_for( long( m.tri.size() ), [&]( long t, int thread ) {
            std::vector<std::pair<double, std::pair<int, int>>>& mine = mine_t[thread];
            if ( !d.talive[t] ) {
               return;
            }
            int obtuse = -1;
            {
               const Vec3 &A = m.x[m.tri[t][0]], &B = m.x[m.tri[t][1]], &C = m.x[m.tri[t][2]];
               if ( max_angle( A, B, C ) > cap_angle ) {
                  double la = norm( C - B ), lb = norm( A - C ), lc = norm( B - A );   // opposite each corner
                  obtuse = la >= lb && la >= lc ? 0 : ( lb >= lc ? 1 : 2 );
               }
            }
            for ( int k = 0; k < 3; ++k ) {
               int a = m.tri[t][k], b = m.tri[t][( k + 1 ) % 3];
               double l = norm( m.x[a] - m.x[b] );
               bool at_cap = obtuse == k || obtuse == ( k + 1 ) % 3;
               if ( l < lmax || at_cap ) {
                  mine.push_back( { l, { std::min( a, b ), std::max( a, b ) } } );
               }
            }
         }, 1024 );
         for ( const auto& mine : mine_t ) {
            edges.insert( edges.end(), mine.begin(), mine.end() );
         }
      }
      std::sort( edges.begin(), edges.end() );
      edges.erase( std::unique( edges.begin(), edges.end() ), edges.end() );
      // The projected midpoints, in parallel; used below while neither end has moved.
      struct Mid {
         Vec3 xa, xb, x, n;
         int atom, feature;
         bool ok;
      };
      std::vector<Mid> mids( edges.size() );
      parallel_for( long( edges.size() ), [&]( long k, int ) {
         Mid& md = mids[k];
         md.xa = m.x[edges[k].second.first];
         md.xb = m.x[edges[k].second.second];
         md.x = ( md.xa + md.xb ) * 0.5;
         md.atom = -1;
         md.feature = F_NONE;
         md.ok = project( model, dmax, md.x, md.n, md.atom, md.feature );
      }, 256 );
      // Collapses run in parallel over spatial cells coloured like a 3-D checkerboard: within one colour
      // the cells are apart, and an edge is collapsed in its cell only if every vertex the collapse
      // reads or writes belongs to that cell.  A triangle that another thread changes has all its
      // vertices in that thread's cell, so reading next to a cell border is safe.  Edges whose
      // neighbourhood crosses a border are deferred to a serial pass.
      struct Buf {
         std::vector<int> na, nb, nc, around, common;
         long collapsed = 0, flipped = 0, rejected_link = 0, rejected_shape = 0;
         void add( const Buf& o ) {
            collapsed += o.collapsed;
            flipped += o.flipped;
            rejected_link += o.rejected_link;
            rejected_shape += o.rejected_shape;
         }
      };
      std::vector<int> vcell;
      // 0: nothing done, 1: collapsed, 2: deferred (cell >= 0 only).
      auto collapse_edge = [&]( size_t ei, int cell, Buf& B ) -> int {
         const auto& e = edges[ei];
         int a = e.second.first, b = e.second.second;
         auto inside = [&]( int v ) { return cell < 0 || vcell[v] == cell; };
         if ( !inside( a ) || !inside( b ) ) {
            return 2;
         }
         if ( !d.valive[a] || !d.valive[b] ) {
            return 0;
         }
         double len = norm( m.x[a] - m.x[b] );
         int t1, t2;
         if ( !d.edge_tris( a, b, t1, t2 ) ) {
            return 0;
         }
         int c = Dyn::third( m.tri[t1], a, b ), dd = Dyn::third( m.tri[t2], a, b );
         d.neighbours( a, B.na );
         d.neighbours( b, B.nb );
         for ( const std::vector<int>* l : { &B.na, &B.nb } ) {
            for ( int v : *l ) {
               if ( !inside( v ) ) {
                  return 2;
               }
            }
         }
         if ( len >= lmax && max_angle( m.x[m.tri[t1][0]], m.x[m.tri[t1][1]], m.x[m.tri[t1][2]] ) <= cap_angle &&
              max_angle( m.x[m.tri[t2][0]], m.x[m.tri[t2][1]], m.x[m.tri[t2][2]] ) <= cap_angle ) {
            return 0;
         }
         B.common.clear();
         std::set_intersection( B.na.begin(), B.na.end(), B.nb.begin(), B.nb.end(), std::back_inserter( B.common ) );
         d.neighbours( c, B.nc );
         size_t vc = B.nc.size();
         for ( int v : B.nc ) {
            if ( !inside( v ) ) {
               return 2;
            }
         }
         d.neighbours( dd, B.nc );
         size_t vd = B.nc.size();
         for ( int v : B.nc ) {
            if ( !inside( v ) ) {
               return 2;
            }
         }
         if ( B.common.size() != 2 || c == dd || vc <= 3 || vd <= 3 || B.na.size() + B.nb.size() > 24 ) {
            ++B.rejected_link;
            return 0;
         }
         // Triangles that survive, and the smallest angle around the edge now.
         B.around.clear();
         double worst_old = PI;
         for ( int v : { a, b } ) {
            for ( int t : d.vt[v] ) {
               worst_old = std::min( worst_old, angle_with( t, -1, Vec3() ) );
               if ( t != t1 && t != t2 ) {
                  B.around.push_back( t );
               }
            }
         }
         // Candidate positions: either end, or the projected midpoint.
         struct Cand {
            Vec3 x, n;
            int atom, feature;
         };
         Cand cand[3] = { { m.x[a], m.n[a], m.atom[a], m.feature[a] }, { m.x[b], m.n[b], m.atom[b], m.feature[b] },
                          { ( m.x[a] + m.x[b] ) * 0.5, Vec3(), -1, F_NONE } };
         int ncand = 2;
         const Mid& md = mids[ei];
         bool ok;
         if ( md.xa.x == m.x[a].x && md.xa.y == m.x[a].y && md.xa.z == m.x[a].z && md.xb.x == m.x[b].x &&
              md.xb.y == m.x[b].y && md.xb.z == m.x[b].z ) {
            cand[2] = { md.x, md.n, md.atom, md.feature };
            ok = md.ok;
         } else {
            ok = project( model, dmax, cand[2].x, cand[2].n, cand[2].atom, cand[2].feature );
         }
         if ( ok && norm( cand[2].x - ( m.x[a] + m.x[b] ) * 0.5 ) < 0.25 * h ) {
            ncand = 3;
         }
         int best = -1;
         double best_worst = -1;
         for ( int ci = 0; ci < ncand; ++ci ) {
            bool fine = true;
            double worst = PI;
            for ( int t : B.around ) {
               int from = Dyn::has( m.tri[t], a ) ? a : b;
               if ( !keeps_facing( t, from, cand[ci].x, cand[ci].n ) ) {
                  fine = false;
                  break;
               }
               worst = std::min( worst, angle_with( t, from, cand[ci].x ) );
            }
            if ( fine && worst > best_worst ) {
               best_worst = worst;
               best = ci;
            }
         }
         // A collapse that removes a poor triangle may leave the worst angle around it unchanged: that
         // worst triangle is often one the collapse does not touch.
         const bool removes_poor = std::min( angle_with( t1, -1, Vec3() ), angle_with( t2, -1, Vec3() ) ) < target;
         if ( best < 0 ||
              !( best_worst > worst_old || best_worst > good || len < tiny || ( removes_poor && best_worst >= worst_old ) ) ) {
            ++B.rejected_shape;
            return 0;
         }
         // Collapse b into a.
         for ( int t : d.vt[b] ) {
            if ( !d.talive[t] ) {
               continue;
            }
            if ( Dyn::has( m.tri[t], a ) ) {
               d.talive[t] = 0;
            } else {
               for ( int k = 0; k < 3; ++k ) {
                  if ( m.tri[t][k] == b ) {
                     m.tri[t][k] = a;
                  }
               }
               d.vt[a].push_back( t );
            }
         }
         d.vt[b].clear();
         d.valive[b] = 0;
         m.x[a] = cand[best].x;
         m.n[a] = cand[best].n;
         m.atom[a] = cand[best].atom;
         m.feature[a] = cand[best].feature;
         d.prune( a );
         d.prune( c );
         d.prune( dd );
         ++B.collapsed;
         return 1;
      };

      // Cells: at least 16 h across, and no more than about 4096 over the mesh's box.  in_cells runs
      // fn( item, cell, buf ) for each item in two rounds over the cells, the second on a grid shifted by
      // half a cell for the items the first deferred (fn returns 2), then serially (cell -1).
      Vec3 lo = m.x[0], hi = m.x[0];
      for ( size_t v = 0; v < m.x.size(); ++v ) {
         if ( d.valive[v] ) {
            for ( int k = 0; k < 3; ++k ) {
               lo[k] = std::min( lo[k], m.x[v][k] );
               hi[k] = std::max( hi[k], m.x[v][k] );
            }
         }
      }
      const double cs = std::max( 16 * h, std::cbrt( ( hi.x - lo.x ) * ( hi.y - lo.y ) * ( hi.z - lo.z ) / 4096 ) );
      int nc3[3];
      for ( int k = 0; k < 3; ++k ) {
         nc3[k] = int( ( hi[k] - lo[k] ) / cs ) + 2;
      }
      const int ncells = nc3[0] * nc3[1] * nc3[2];
      long deferred_total = 0;
      auto in_cells = [&]( std::vector<size_t> todo, auto&& first_vertex, auto&& fn ) {
         Buf total;
         std::vector<size_t> deferred;
         for ( int round = 0; round < 2 && !todo.empty(); ++round ) {
            const double shift = round * 0.5 * cs;
            vcell.assign( m.x.size(), -1 );
            for ( size_t v = 0; v < m.x.size(); ++v ) {
               int ix = int( ( m.x[v].x - lo.x + shift ) / cs ), iy = int( ( m.x[v].y - lo.y + shift ) / cs );
               int iz = int( ( m.x[v].z - lo.z + shift ) / cs );
               ix = std::min( std::max( ix, 0 ), nc3[0] - 1 );
               iy = std::min( std::max( iy, 0 ), nc3[1] - 1 );
               iz = std::min( std::max( iz, 0 ), nc3[2] - 1 );
               vcell[v] = ( iz * nc3[1] + iy ) * nc3[0] + ix;
            }
            std::vector<std::vector<size_t>> cell_items( ncells );
            for ( size_t it : todo ) {
               cell_items[vcell[first_vertex( it )]].push_back( it );
            }
            deferred.clear();
            for ( int colour = 0; colour < 8; ++colour ) {
               std::vector<int> cells;
               for ( int c = 0; c < ncells; ++c ) {
                  int ix = c % nc3[0], iy = ( c / nc3[0] ) % nc3[1], iz = c / ( nc3[0] * nc3[1] );
                  if ( ( ix & 1 ) + 2 * ( iy & 1 ) + 4 * ( iz & 1 ) == colour && !cell_items[c].empty() ) {
                     cells.push_back( c );
                  }
               }
               std::vector<Buf> bufs( threads() );
               std::vector<std::vector<size_t>> defs( threads() );
               parallel_for( long( cells.size() ), [&]( long k, int thread ) {
                  for ( size_t it : cell_items[cells[k]] ) {
                     if ( fn( it, cells[k], bufs[thread] ) == 2 ) {
                        defs[thread].push_back( it );
                     }
                  }
               } );
               for ( size_t th = 0; th < bufs.size(); ++th ) {
                  total.add( bufs[th] );
                  deferred.insert( deferred.end(), defs[th].begin(), defs[th].end() );
               }
            }
            std::sort( deferred.begin(), deferred.end() );
            todo.swap( deferred );
         }
         deferred_total += long( todo.size() );
         for ( size_t it : todo ) {
            fn( it, -1, total );
         }
         return total;
      };

      {
         std::vector<size_t> todo( edges.size() );
         for ( size_t ei = 0; ei < edges.size(); ++ei ) {
            todo[ei] = ei;
         }
         Buf r = in_cells( todo, [&]( size_t ei ) { return edges[ei].second.first; }, collapse_edge );
         collapsed += r.collapsed;
         info.rejected_link += r.rejected_link;
         info.rejected_shape += r.rejected_shape;
      }

      info.seconds_collapse += now() - ts;
      ts = now();
      // Edge flips where they raise the smallest angle of two nearly coplanar triangles; only around
      // triangles with an angle under the target (a flip between two good triangles gains little).
      // The poor triangles are found in parallel, and flipped over the same cells: a flip changes two
      // triangles whose four vertices must all be in the cell.
      auto flip_tri = [&]( size_t t, int cell, Buf& B ) -> int {
         auto inside = [&]( int v ) { return cell < 0 || vcell[v] == cell; };
         for ( int k = 0; k < 3; ++k ) {
            if ( !inside( m.tri[t][k] ) ) {
               return 2;
            }
         }
         if ( !d.talive[t] ) {
            return 0;
         }
         for ( int k = 0; k < 3; ++k ) {
            int a = m.tri[t][k], b = m.tri[t][( k + 1 ) % 3];
            int t1, t2;
            if ( !d.edge_tris( a, b, t1, t2 ) ) {
               continue;
            }
            if ( t2 == int( t ) ) {
               std::swap( t1, t2 );
            }
            // t1 = (a, b, c) and t2 = (b, a, dd), counter-clockwise; the quadrilateral is b, c, a, dd.
            int c = Dyn::third( m.tri[t1], a, b ), dd = Dyn::third( m.tri[t2], a, b );
            if ( !inside( dd ) ) {
               return 2;
            }
            const Vec3 &A = m.x[a], &Bv = m.x[b], &C = m.x[c], &D = m.x[dd];
            Vec3 n1 = cross( Bv - A, C - A ), n2 = cross( A - Bv, D - Bv );
            double l1 = norm( n1 ), l2 = norm( n2 );
            double old_min = std::min( min_angle( A, Bv, C ), min_angle( Bv, A, D ) );
            double new_min = std::min( min_angle( C, A, D ), min_angle( D, Bv, C ) );
            if ( !( new_min > old_min + 1e-3 ) ) {
               continue;
            }
            Vec3 m1 = cross( A - C, D - C ), m2 = cross( Bv - D, C - D );
            // Two nearly coplanar triangles; or a degenerate cap (an angle near 180 degrees, so its normal
            // means little), where the new triangles must face the way the exact surface normals do.
            const bool cap = max_angle( A, Bv, C ) > cap_angle || max_angle( Bv, A, D ) > cap_angle;
            if ( l1 > 0 && l2 > 0 && dot( n1, n2 ) >= cos_flip * l1 * l2 ) {
               Vec3 avg = n1 / l1 + n2 / l2;
               if ( dot( m1, avg ) <= 0 || dot( m2, avg ) <= 0 ) {
                  continue;
               }
            } else if ( cap ) {
               Vec3 s1 = m.n[c] + m.n[a] + m.n[dd], s2 = m.n[dd] + m.n[b] + m.n[c];
               double k1 = norm( m1 ) * norm( s1 ), k2 = norm( m2 ) * norm( s2 );
               if ( !( k1 > 0 && k2 > 0 ) || dot( m1, s1 ) < 0.5 * k1 || dot( m2, s2 ) < 0.5 * k2 ) {
                  continue;
               }
            } else {
               continue;
            }
            d.neighbours( a, B.na );
            d.neighbours( b, B.nb );
            d.neighbours( c, B.nc );
            if ( B.na.size() <= 3 || B.nb.size() <= 3 || std::binary_search( B.nc.begin(), B.nc.end(), dd ) ) {
               continue;
            }
            m.tri[t1] = { c, a, dd };
            m.tri[t2] = { dd, b, c };
            auto drop = [&]( int v, int tt ) {
               std::vector<int>& l = d.vt[v];
               l.erase( std::remove( l.begin(), l.end(), tt ), l.end() );
            };
            drop( a, t2 );
            drop( b, t1 );
            d.vt[c].push_back( t2 );
            d.vt[dd].push_back( t1 );
            ++B.flipped;
            return 1;   // this triangle has changed
         }
         return 0;
      };
      {
         std::vector<char> poor_t( m.tri.size(), 0 );
         parallel_for( long( m.tri.size() ), [&]( long t, int ) {
            poor_t[t] = d.talive[t] && angle_with( int( t ), -1, Vec3() ) < target;
         }, 1024 );
         std::vector<size_t> todo;
         for ( size_t t = 0; t < m.tri.size(); ++t ) {
            if ( poor_t[t] ) {
               todo.push_back( t );
            }
         }
         Buf r = in_cells( todo, [&]( size_t t ) { return m.tri[t][0]; }, flip_tri );
         flipped += r.flipped;
      }
      info.seconds_flip += now() - ts;
      ts = now();
      // Tangential smoothing: each vertex moves halfway to its neighbours' centroid in the
      // tangent plane, then back onto the surface, if that raises the smallest angle around it.
      long moved = 0;
      if ( pass < opt.smooth ) {
         // The vertices next to a poor triangle, coloured so that no two of one colour share a
         // triangle: the vertices of a colour then move independently, in parallel.
         std::vector<int> colour( m.x.size(), -1 );
         std::vector<std::vector<int>> classes;
         std::vector<char> used, poor_v( m.x.size(), 0 );
         parallel_for( long( m.x.size() ), [&]( long v, int ) {
            if ( !d.valive[v] ) {
               return;
            }
            d.prune( int( v ) );
            for ( int t : d.vt[v] ) {
               if ( angle_with( t, -1, Vec3() ) < target ) {
                  poor_v[v] = 1;
                  break;
               }
            }
         }, 1024 );
         for ( size_t v = 0; v < m.x.size(); ++v ) {
            if ( !poor_v[v] ) {
               continue;
            }
            d.neighbours( int( v ), na );
            used.assign( classes.size() + 1, 0 );
            for ( int u : na ) {
               if ( colour[u] >= 0 ) {
                  used[colour[u]] = 1;
               }
            }
            int c = 0;
            while ( used[c] ) {
               ++c;
            }
            if ( c == int( classes.size() ) ) {
               classes.emplace_back();
            }
            colour[v] = c;
            classes[c].push_back( int( v ) );
         }
         std::vector<std::vector<int>> nl_t( threads() );
         std::vector<long> moved_t( threads(), 0 );
         for ( const std::vector<int>& cls : classes ) {
            {
               parallel_for( long( cls.size() ), [&]( long i, int thread ) {
                  std::vector<int>& nl = nl_t[thread];
                  int v = cls[i];
                  d.neighbours( v, nl );
                  if ( nl.size() < 3 ) {
                     return;
                  }
                  Vec3 c;
                  for ( int u : nl ) {
                     c += m.x[u];
                  }
                  c = c / double( nl.size() );
                  Vec3 del = c - m.x[v];
                  del = del - m.n[v] * dot( del, m.n[v] );
                  Vec3 P = m.x[v] + del * 0.5, nP;
                  int at = -1, fe = F_NONE;
                  if ( !project( model, dmax, P, nP, at, fe ) || norm( P - m.x[v] ) > 0.5 * h ) {
                     return;
                  }
                  double worst_old = PI, worst_new = PI;
                  bool ok = true;
                  for ( int t : d.vt[v] ) {
                     worst_old = std::min( worst_old, angle_with( t, -1, Vec3() ) );
                     if ( !keeps_facing( t, v, P, nP ) ) {
                        ok = false;
                        break;
                     }
                     worst_new = std::min( worst_new, angle_with( t, v, P ) );
                  }
                  if ( !ok || !( worst_new > worst_old ) ) {
                     return;
                  }
                  m.x[v] = P;
                  m.n[v] = nP;
                  m.atom[v] = at;
                  m.feature[v] = fe;
                  ++moved_t[thread];
               }, 64 );
            }
         }
         for ( long k : moved_t ) {
            moved += k;
         }
      }
      info.deferred += deferred_total;
      info.seconds_smooth += now() - ts;
      info.collapses += collapsed;
      info.flips += flipped;
      info.smoothed += moved;
      if ( double( collapsed + flipped + moved ) < 1e-3 * double( m.x.size() ) ) {
         break;
      }
   }

   // Compact.
   std::vector<int> newid( m.x.size(), -1 );
   Mesh out;
   for ( size_t v = 0; v < m.x.size(); ++v ) {
      if ( d.valive[v] ) {
         newid[v] = int( out.x.size() );
         out.x.push_back( m.x[v] );
         out.n.push_back( m.n[v] );
         out.atom.push_back( m.atom[v] );
         out.feature.push_back( m.feature[v] );
      }
   }
   for ( size_t t = 0; t < m.tri.size(); ++t ) {
      if ( d.talive[t] ) {
         out.tri.push_back( { newid[m.tri[t][0]], newid[m.tri[t][1]], newid[m.tri[t][2]] } );
      }
   }
   m = std::move( out );
   info.seconds = now() - t0;
}

}  // namespace sesmesh
