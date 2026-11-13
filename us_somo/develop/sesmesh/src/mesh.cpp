// sesmesh: narrow-band grid, cavity filling and marching tetrahedra; see mesh.h.
#include "mesh.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <string>

#include "parallel.h"

namespace sesmesh {

namespace {

constexpr int BS = 8;               // nodes per brick edge
constexpr int BSH = 3;              // log2(BS)
constexpr int BS3 = BS * BS * BS;
constexpr int32_t OUT = -1, IN = -2, BAND = -3;   // brick classes; band bricks hold a slot >= 0
constexpr double TMIN = 1e-4;       // closest a vertex may be to a grid node, as a fraction of its edge

double now() {
   return std::chrono::duration<double>( std::chrono::steady_clock::now().time_since_epoch() ).count();
}

// Edge directions from a node, as bits: 1 = +x, 2 = +y, 4 = +z.  The seven nonzero ones are the
// edges of the Freudenthal tetrahedra that start at a node; direction index = bits - 1.
// Node neighbours along these edges, both ways: the 14 grid neighbours a vertex can connect to.
const int NB14[14][3] = { { 1, 0, 0 },  { 0, 1, 0 },  { 1, 1, 0 },  { 0, 0, 1 },  { 1, 0, 1 },
                          { 0, 1, 1 },  { 1, 1, 1 },  { -1, 0, 0 }, { 0, -1, 0 }, { -1, -1, 0 },
                          { 0, 0, -1 }, { -1, 0, -1 }, { 0, -1, -1 }, { -1, -1, -1 } };

// Edges of a tetrahedron (v0, v1, v2, v3).
const int TE[6][2] = { { 0, 1 }, { 0, 2 }, { 0, 3 }, { 1, 2 }, { 1, 3 }, { 2, 3 } };

struct TetCase {
   int ntri = 0;          // 1: one triangle; 2: a quadrilateral, two triangles
   int8_t tri[3];         // the triangle, as tetrahedron edges
   int8_t quad[2][2][3];  // the quadrilateral split along either diagonal
   int8_t diag[2][2];     // that diagonal's end edges
};

// The six Freudenthal tetrahedra of a cube, v0 = corner 0 to v3 = corner 7 (corner bits as for
// directions), and for each pattern of outside corners the oriented triangles that separate
// them.  Orientation is fixed with the edge midpoints, where the cut is the zero set of the
// linear interpolant of -1 (inside) and +1 (outside): a triangle's normal must point the way
// that interpolant grows.  Moving the vertices along their edges changes no triangle's
// orientation, since none can become degenerate.
struct TetTable {
   int cv[6][4];
   TetCase c[6][16];

   TetTable() {
      const int perm[6][3] = { { 0, 1, 2 }, { 0, 2, 1 }, { 1, 0, 2 }, { 1, 2, 0 }, { 2, 0, 1 }, { 2, 1, 0 } };
      for ( int t = 0; t < 6; ++t ) {
         int a = 1 << perm[t][0], b = 1 << perm[t][1];
         cv[t][0] = 0;
         cv[t][1] = a;
         cv[t][2] = a | b;
         cv[t][3] = 7;
         auto P = [&]( int q ) {
            int bits = cv[t][q];
            return Vec3( bits & 1, ( bits >> 1 ) & 1, ( bits >> 2 ) & 1 );
         };
         auto edge = [&]( int q1, int q2 ) -> int8_t {
            int lo = std::min( q1, q2 ), hi = std::max( q1, q2 );
            for ( int e = 0; e < 6; ++e ) {
               if ( TE[e][0] == lo && TE[e][1] == hi ) {
                  return int8_t( e );
               }
            }
            throw std::logic_error( "tetrahedron edge" );
         };
         auto mid = [&]( int e ) { return ( P( TE[e][0] ) + P( TE[e][1] ) ) * 0.5; };
         for ( int cfg = 1; cfg < 15; ++cfg ) {
            TetCase& tc = c[t][cfg];
            int nout = 0;
            Vec3 cin, cout;
            for ( int q = 0; q < 4; ++q ) {
               if ( ( cfg >> q ) & 1 ) {
                  ++nout;
                  cout += P( q );
               } else {
                  cin += P( q );
               }
            }
            Vec3 ref = cout / nout - cin / ( 4 - nout );
            auto orient = [&]( int8_t* tr ) {
               Vec3 A = mid( tr[0] );
               double s = dot( cross( mid( tr[1] ) - A, mid( tr[2] ) - A ), ref );
               if ( s == 0 ) {
                  throw std::logic_error( "degenerate tetrahedron case" );
               }
               if ( s < 0 ) {
                  std::swap( tr[1], tr[2] );
               }
            };
            if ( nout == 1 || nout == 3 ) {
               int odd = 0;
               for ( int q = 0; q < 4; ++q ) {
                  if ( ( ( cfg >> q ) & 1 ) == ( nout == 1 ? 1 : 0 ) ) {
                     odd = q;
                  }
               }
               int k = 0;
               for ( int q = 0; q < 4; ++q ) {
                  if ( q != odd ) {
                     tc.tri[k++] = edge( odd, q );
                  }
               }
               orient( tc.tri );
               tc.ntri = 1;
            } else {
               int in[2], out[2], ni = 0, no = 0;
               for ( int q = 0; q < 4; ++q ) {
                  if ( ( cfg >> q ) & 1 ) {
                     out[no++] = q;
                  } else {
                     in[ni++] = q;
                  }
               }
               int8_t ac = edge( in[0], out[0] ), ad = edge( in[0], out[1] );
               int8_t bd = edge( in[1], out[1] ), bc = edge( in[1], out[0] );
               // the quadrilateral ac, ad, bd, bc, split along ac-bd or along ad-bc
               int8_t q0[2][3] = { { ac, ad, bd }, { ac, bd, bc } };
               int8_t q1[2][3] = { { ac, ad, bc }, { ad, bd, bc } };
               for ( int k = 0; k < 2; ++k ) {
                  for ( int e = 0; e < 3; ++e ) {
                     tc.quad[0][k][e] = q0[k][e];
                     tc.quad[1][k][e] = q1[k][e];
                  }
                  orient( tc.quad[0][k] );
                  orient( tc.quad[1][k] );
               }
               tc.diag[0][0] = ac;
               tc.diag[0][1] = bd;
               tc.diag[1][0] = ad;
               tc.diag[1][1] = bc;
               tc.ntri = 2;
            }
         }
      }
   }
};

const TetTable& tet_table() {
   static const TetTable table;
   return table;
}

struct Grid {
   double h = 0;
   Vec3 o;
   int nb[3] = { 0, 0, 0 };   // bricks per axis
   int nn[3] = { 0, 0, 0 };   // nodes per axis
   std::vector<int32_t> slot; // per brick: OUT, IN or a band slot
   std::vector<uint8_t> reached;   // per brick: an OUT brick reached from outside
   std::vector<long> band;    // slot -> brick
   std::vector<float> f;      // per band node (slot * BS3 + local)
   std::vector<uint8_t> out;  // per band node: outside, reached from outside

   long brick( int bi, int bj, int bk ) const { return ( long( bk ) * nb[1] + bj ) * nb[0] + bi; }
   long brick_of( int i, int j, int k ) const { return brick( i >> BSH, j >> BSH, k >> BSH ); }
   static int local( int i, int j, int k ) {
      return ( ( k & ( BS - 1 ) ) * BS + ( j & ( BS - 1 ) ) ) * BS + ( i & ( BS - 1 ) );
   }
   Vec3 pos( int i, int j, int k ) const { return o + Vec3( i, j, k ) * h; }
   bool in_grid( int i, int j, int k ) const {
      return i >= 0 && j >= 0 && k >= 0 && i < nn[0] && j < nn[1] && k < nn[2];
   }
   long band_index( int i, int j, int k ) const {
      int32_t s = slot[brick_of( i, j, k )];
      return s >= 0 ? long( s ) * BS3 + local( i, j, k ) : -1;
   }
   bool is_out( int i, int j, int k ) const {
      long b = brick_of( i, j, k );
      int32_t s = slot[b];
      if ( s >= 0 ) {
         return out[long( s ) * BS3 + local( i, j, k )] != 0;
      }
      return s == OUT && reached[b];
   }
   void brick_coords( long b, int& bi, int& bj, int& bk ) const {
      bi = int( b % nb[0] );
      bj = int( ( b / nb[0] ) % nb[1] );
      bk = int( b / ( long( nb[0] ) * nb[1] ) );
   }
   Vec3 brick_centre( int bi, int bj, int bk ) const {
      const double c = 0.5 * ( BS - 1 );
      return o + Vec3( bi * BS + c, bj * BS + c, bk * BS + c ) * h;
   }
};

// A root of g on [0, 1], given g0 = g(0) <= 0 < g1 = g(1): false position with the Illinois
// modification, and a bisection step whenever four steps fail to halve the bracket.
template <class G>
double solve_root( G&& g, double g0, double g1, long& evals ) {
   double a = 0, b = 1, fa = g0, fb = g1;
   if ( fa >= 0 ) {
      return 0;
   }
   int side = 0;
   double wref = 1;
   for ( int it = 1; it <= 200; ++it ) {
      double t = ( a * fb - b * fa ) / ( fb - fa );
      if ( ( it % 4 == 0 && b - a > 0.5 * wref ) || !( t > a && t < b ) ) {
         t = 0.5 * ( a + b );
      }
      double ft = g( t );
      ++evals;
      if ( ft > 0 ) {
         b = t;
         fb = ft;
         if ( side == 1 ) {
            fa *= 0.5;
         }
         side = 1;
      } else {
         a = t;
         fa = ft;
         if ( side == -1 ) {
            fb *= 0.5;
         }
         side = -1;
      }
      if ( it % 4 == 0 ) {
         wref = b - a;
      }
      if ( std::fabs( ft ) < 1e-13 || b - a < 1e-13 ) {
         return t;
      }
   }
   return 0.5 * ( a + b );
}

// Sorts v in parallel: sorted chunks, then rounds of pairwise merges.
void parallel_sort( std::vector<uint64_t>& v ) {
   const int nthreads = threads();
   const size_t n = v.size();
   if ( nthreads < 2 || n < 100000 ) {
      std::sort( v.begin(), v.end() );
      return;
   }
   int chunks = 1;
   while ( chunks < nthreads ) {
      chunks *= 2;
   }
   std::vector<size_t> b( chunks + 1 );
   for ( int i = 0; i <= chunks; ++i ) {
      b[i] = n * size_t( i ) / size_t( chunks );
   }
   parallel_for( chunks, [&]( long i, int ) { std::sort( v.begin() + b[i], v.begin() + b[i + 1] ); } );
   std::vector<uint64_t> tmp( n );
   for ( int width = 1; width < chunks; width *= 2 ) {
      const int pairs = ( chunks + 2 * width - 1 ) / ( 2 * width );
      parallel_for( pairs, [&]( long k, int ) {
         const int i = int( k ) * 2 * width;
         const size_t lo = b[i], mid = b[std::min( i + width, chunks )], hi = b[std::min( i + 2 * width, chunks )];
         std::merge( v.begin() + lo, v.begin() + mid, v.begin() + mid, v.begin() + hi, tmp.begin() + lo );
      } );
      v.swap( tmp );
   }
}

}  // namespace

void mesh_area_volume( const Mesh& m, double& area, double& volume ) {
   area = volume = 0;
   if ( m.tri.empty() ) {
      return;
   }
   const Vec3 o = m.x[m.tri[0][0]];
   const long nt = long( m.tri.size() ), block = 4096, nb = blocks( nt, block );
   std::vector<double> pa( nb, 0 ), pv( nb, 0 );
   parallel_for( nb, [&]( long k, int ) {
      double a = 0, v = 0;
      for ( long t = k * block; t < std::min( nt, ( k + 1 ) * block ); ++t ) {
         const Vec3 &A = m.x[m.tri[t][0]], &B = m.x[m.tri[t][1]], &C = m.x[m.tri[t][2]];
         a += 0.5 * norm( cross( B - A, C - A ) );
         v += dot( A - o, cross( B - o, C - o ) ) / 6;
      }
      pa[k] = a;
      pv[k] = v;
   } );
   for ( long k = 0; k < nb; ++k ) {
      area += pa[k];
      volume += pv[k];
   }
}

void build_mesh( const SesModel& model, const MeshOptions& opt, Mesh& mesh, BuildInfo& info ) {
   const double h = opt.h;
   if ( !( h > 0 ) ) {
      throw std::invalid_argument( "the grid spacing must be positive" );
   }
   info = BuildInfo();
   mesh = Mesh();
   const double p = model.probe();
   const double s3 = std::sqrt( 3.0 );
   const double rb = h * ( BS - 1 ) * s3 / 2;   // brick centre to its corner nodes
   const double cut = h * s3;                   // a cube's diagonal
   // A cube cut by the surface has all its corners within `cut` of it, where |f| <= cut.  A brick
   // whose centre has |f| > rb + cut therefore holds no corner of such a cube.
   const double tau = rb + cut;
   const double dmax_brick = p + 1.01 * tau + 1e-9;
   const double dmax_node = p + 1.01 * cut + 1e-9;
   // Points a band brick evaluates (its nodes and its edges' roots) lie within rb + cut of its centre.
   const double reach = rb + cut + dmax_node;

   Grid g;
   g.h = h;
   const Vec3 lo = model.lo(), hi = model.hi();
   const double margin = ( 2 * BS + 1 ) * h;
   // Shift the grid by irrational fractions of h, so that its nodes do not line up with the
   // centres, as they would with an origin at a multiple of h from the box of U.
   const double shift[3] = { 0.3819660113, 0.2360679775, 0.1458980338 };
   long nbricks = 1;
   for ( int a = 0; a < 3; ++a ) {
      g.o[a] = lo[a] - margin - shift[a] * h;
      double nodes = std::ceil( ( hi[a] - lo[a] + 2 * margin ) / h ) + 1;
      if ( nodes > 1 << 28 ) {
         throw std::runtime_error( "grid too large; use a larger spacing" );
      }
      g.nb[a] = ( int( nodes ) + BS - 1 ) / BS;
      g.nn[a] = g.nb[a] * BS;
      info.nodes_per_axis[a] = g.nn[a];
      nbricks *= g.nb[a];
   }
   if ( nbricks > ( 1L << 31 ) ) {
      throw std::runtime_error( "grid too large; use a larger spacing" );
   }
   info.bricks = nbricks;
   g.slot.assign( nbricks, OUT );
   g.reached.assign( nbricks, 0 );

   // 1. Classify bricks by f at their centres.
   double t0 = now();
   std::vector<std::vector<int>> cands( threads() );
   parallel_for( nbricks, [&]( long b, int thread ) {
      std::vector<int>& cand = cands[thread];
      int bi, bj, bk;
      g.brick_coords( b, bi, bj, bk );
      Vec3 xc = g.brick_centre( bi, bj, bk );
      model.gather( xc, dmax_brick, cand );
      if ( cand.empty() ) {
         return;   // farther than dmax from U: outside
      }
      double fc = model.eval( xc, cand, dmax_brick );
      g.slot[b] = fc > tau ? OUT : ( fc < -tau ? IN : BAND );
   }, 256 );
   for ( long b = 0; b < nbricks; ++b ) {
      if ( g.slot[b] == BAND ) {
         g.slot[b] = int32_t( g.band.size() );
         g.band.push_back( b );
      } else if ( g.slot[b] == OUT ) {
         ++info.out_bricks;
      } else {
         ++info.in_bricks;
      }
   }
   const long nslots = long( g.band.size() );
   info.band_bricks = nslots;
   g.f.resize( nslots * BS3 );
   g.out.assign( nslots * BS3, 0 );
   double t1 = now();
   info.seconds_classify = t1 - t0;

   // 2. f at every node of the band bricks.
   parallel_for( nslots, [&]( long s, int thread ) {
      std::vector<int>& cand = cands[thread];
      int bi, bj, bk;
      g.brick_coords( g.band[s], bi, bj, bk );
      model.gather( g.brick_centre( bi, bj, bk ), reach, cand );
      float* fs = &g.f[s * BS3];
      for ( int l = 0; l < BS3; ++l ) {
         Vec3 x = g.pos( bi * BS + ( l & 7 ), bj * BS + ( ( l >> 3 ) & 7 ), bk * BS + ( l >> 6 ) );
         fs[l] = float( model.eval( x, cand, dmax_node ) );
      }
   }, 16 );
   info.node_evals = nslots * BS3;
   double t2 = now();
   info.seconds_nodes = t2 - t1;

   // 3. Flood fill from the edge of the grid through outside nodes, along the 14 directions
   //    that tetrahedron edges take, a whole brick at a time for OUT bricks.
   {
      std::vector<long> bstack;
      std::vector<std::array<int, 3>> nstack;
      for ( int bk = 0; bk < g.nb[2]; ++bk ) {
         for ( int bj = 0; bj < g.nb[1]; ++bj ) {
            for ( int bi = 0; bi < g.nb[0]; ++bi ) {
               if ( bi > 0 && bj > 0 && bk > 0 && bi < g.nb[0] - 1 && bj < g.nb[1] - 1 && bk < g.nb[2] - 1 ) {
                  continue;
               }
               long b = g.brick( bi, bj, bk );
               if ( g.slot[b] != OUT ) {
                  throw std::logic_error( "grid margin: a brick on the grid's edge is not outside" );
               }
               if ( !g.reached[b] ) {
                  g.reached[b] = 1;
                  bstack.push_back( b );
               }
            }
         }
      }
      auto visit_node = [&]( int i, int j, int k ) {
         long b = g.brick_of( i, j, k );
         int32_t s = g.slot[b];
         if ( s == OUT ) {
            if ( !g.reached[b] ) {
               g.reached[b] = 1;
               bstack.push_back( b );
            }
         } else if ( s >= 0 ) {
            long idx = long( s ) * BS3 + Grid::local( i, j, k );
            if ( !g.out[idx] && g.f[idx] > 0 ) {
               g.out[idx] = 1;
               nstack.push_back( { i, j, k } );
            }
         }
      };
      while ( !bstack.empty() || !nstack.empty() ) {
         if ( !nstack.empty() ) {
            std::array<int, 3> n = nstack.back();
            nstack.pop_back();
            for ( const auto& d : NB14 ) {
               int i = n[0] + d[0], j = n[1] + d[1], k = n[2] + d[2];
               if ( g.in_grid( i, j, k ) ) {
                  visit_node( i, j, k );
               }
            }
            continue;
         }
         long b = bstack.back();
         bstack.pop_back();
         int bi, bj, bk;
         g.brick_coords( b, bi, bj, bk );
         for ( int dz = -1; dz <= 1; ++dz ) {
            for ( int dy = -1; dy <= 1; ++dy ) {
               for ( int dx = -1; dx <= 1; ++dx ) {
                  int ci = bi + dx, cj = bj + dy, ck = bk + dz;
                  if ( ( !dx && !dy && !dz ) || ci < 0 || cj < 0 || ck < 0 || ci >= g.nb[0] || cj >= g.nb[1] ||
                       ck >= g.nb[2] ) {
                     continue;
                  }
                  long b2 = g.brick( ci, cj, ck );
                  int32_t s2 = g.slot[b2];
                  if ( s2 == OUT ) {
                     if ( !g.reached[b2] ) {
                        g.reached[b2] = 1;
                        bstack.push_back( b2 );
                     }
                     continue;
                  }
                  if ( s2 < 0 ) {
                     continue;
                  }
                  // The nodes of band brick b2 that are next to brick b: every node of b is outside.
                  int lo3[3], hi3[3];
                  const int dd[3] = { dx, dy, dz };
                  for ( int a = 0; a < 3; ++a ) {
                     lo3[a] = dd[a] == 1 ? 0 : ( dd[a] == -1 ? BS - 1 : 0 );
                     hi3[a] = dd[a] == 1 ? 0 : ( dd[a] == -1 ? BS - 1 : BS - 1 );
                  }
                  for ( int lz = lo3[2]; lz <= hi3[2]; ++lz ) {
                     for ( int ly = lo3[1]; ly <= hi3[1]; ++ly ) {
                        for ( int lx = lo3[0]; lx <= hi3[0]; ++lx ) {
                           int i = ci * BS + lx, j = cj * BS + ly, k = ck * BS + lz;
                           long idx = long( s2 ) * BS3 + Grid::local( i, j, k );
                           if ( g.out[idx] || !( g.f[idx] > 0 ) ) {
                              continue;
                           }
                           for ( const auto& d : NB14 ) {
                              int i2 = i + d[0], j2 = j + d[1], k2 = k + d[2];
                              if ( g.in_grid( i2, j2, k2 ) && g.brick_of( i2, j2, k2 ) == b ) {
                                 g.out[idx] = 1;
                                 nstack.push_back( { i, j, k } );
                                 break;
                              }
                           }
                        }
                     }
                  }
               }
            }
         }
      }
      for ( long b = 0; b < nbricks; ++b ) {
         if ( g.slot[b] == OUT && !g.reached[b] ) {
            info.cavity_nodes += BS3;
         }
      }
      for ( long n = 0; n < nslots * BS3; ++n ) {
         if ( g.f[n] > 0 && !g.out[n] ) {
            ++info.cavity_nodes;
         }
      }
      if ( !opt.fill_cavities ) {
         for ( long b = 0; b < nbricks; ++b ) {
            if ( g.slot[b] == OUT ) {
               g.reached[b] = 1;
            }
         }
         for ( long n = 0; n < nslots * BS3; ++n ) {
            g.out[n] = g.f[n] > 0;
         }
      }
   }
   double t3 = now();
   info.seconds_fill = t3 - t2;

   // 4. A vertex on every edge between an inside and an outside node, at a root of f.  The edge
   //    belongs to the brick of its lower node; key = local node * 8 + direction index.
   struct VRec {
      Vec3 x, n;
      int atom, feature;
   };
   std::vector<std::vector<uint16_t>> keys( nslots );
   std::vector<std::vector<VRec>> vrec( nslots );
   std::atomic<bool> failed( false );
   std::vector<long> evals_t( threads(), 0 );
   {
      parallel_for( nslots, [&]( long s, int thread ) {
         std::vector<int>& cand = cands[thread];
         long& root_evals = evals_t[thread];
         int bi, bj, bk;
         g.brick_coords( g.band[s], bi, bj, bk );
         bool gathered = false;
         for ( int l = 0; l < BS3; ++l ) {
            int i = bi * BS + ( l & 7 ), j = bj * BS + ( ( l >> 3 ) & 7 ), k = bk * BS + ( l >> 6 );
            bool o1 = g.out[s * BS3 + l] != 0;
            for ( int bits = 1; bits < 8; ++bits ) {
               int i2 = i + ( bits & 1 ), j2 = j + ( ( bits >> 1 ) & 1 ), k2 = k + ( ( bits >> 2 ) & 1 );
               if ( !g.in_grid( i2, j2, k2 ) || g.is_out( i2, j2, k2 ) == o1 ) {
                  continue;
               }
               long idx2 = g.band_index( i2, j2, k2 );
               if ( idx2 < 0 ) {
                  failed = true;
                  continue;
               }
               if ( !gathered ) {
                  model.gather( g.brick_centre( bi, bj, bk ), reach, cand );
                  gathered = true;
               }
               Vec3 xin = g.pos( i, j, k ), xout = g.pos( i2, j2, k2 );
               double fin = g.f[s * BS3 + l], fout = g.f[idx2];
               if ( o1 ) {
                  std::swap( xin, xout );
                  std::swap( fin, fout );
               }
               Vec3 e = xout - xin;
               long ev = 0;
               double t = solve_root( [&]( double u ) { return model.eval( xin + e * u, cand, dmax_node ); }, fin, fout,
                                      ev );
               root_evals += ev;
               // A root at a node would give several edges the same vertex, and triangles of zero
               // area; keep each vertex at least TMIN of its edge from either end.
               t = std::min( std::max( t, TMIN ), 1 - TMIN );
               Vec3 x = xin + e * t;
               EvalInfo inf;
               model.eval( x, cand, dmax_node, &inf );
               Vec3 nrm = inf.q - x;
               double ln = norm( nrm );
               nrm = ln > 1e-9 ? nrm / ln : unit( e );
               keys[s].push_back( uint16_t( l * 8 + bits - 1 ) );
               vrec[s].push_back( { x, nrm, inf.atom, inf.feature } );
            }
         }
      }, 16 );
   }
   if ( failed ) {
      throw std::logic_error( "a cut edge leaves the band" );
   }
   for ( long e : evals_t ) {
      info.root_evals += e;
   }
   std::vector<long> base( nslots + 1, 0 );
   for ( long s = 0; s < nslots; ++s ) {
      base[s + 1] = base[s] + long( vrec[s].size() );
   }
   const long nv = base[nslots];
   if ( nv >= ( 1L << 31 ) ) {
      throw std::runtime_error( "too many vertices; use a larger spacing" );
   }
   mesh.x.resize( nv );
   mesh.n.resize( nv );
   mesh.atom.resize( nv );
   mesh.feature.resize( nv );
   parallel_for( nslots, [&]( long s, int ) {
      for ( size_t k = 0; k < vrec[s].size(); ++k ) {
         const VRec& r = vrec[s][k];
         long v = base[s] + long( k );
         mesh.x[v] = r.x;
         mesh.n[v] = r.n;
         mesh.atom[v] = r.atom;
         mesh.feature[v] = r.feature;
      }
      std::vector<VRec>().swap( vrec[s] );
   }, 64 );
   double t4 = now();
   info.seconds_vertices = t4 - t3;

   // 5. Triangles: each cube with corners on both sides, a cube for every node of a band brick.
   const TetTable& tt = tet_table();
   auto vid = [&]( int i, int j, int k, int dir ) -> long {
      int32_t s = g.slot[g.brick_of( i, j, k )];
      if ( s < 0 ) {
         return -1;
      }
      uint16_t key = uint16_t( Grid::local( i, j, k ) * 8 + dir );
      const std::vector<uint16_t>& ks = keys[s];
      auto it = std::lower_bound( ks.begin(), ks.end(), key );
      if ( it == ks.end() || *it != key ) {
         return -1;
      }
      return base[s] + long( it - ks.begin() );
   };
   std::vector<std::vector<std::array<int, 3>>> tris( nslots );
   parallel_for( nslots, [&]( long s, int ) {
      int bi, bj, bk;
      g.brick_coords( g.band[s], bi, bj, bk );
      for ( int l = 0; l < BS3; ++l ) {
         int i = bi * BS + ( l & 7 ), j = bj * BS + ( ( l >> 3 ) & 7 ), k = bk * BS + ( l >> 6 );
         if ( i + 1 >= g.nn[0] || j + 1 >= g.nn[1] || k + 1 >= g.nn[2] ) {
            continue;
         }
         bool oc[8];
         int mask = 0;
         for ( int c = 0; c < 8; ++c ) {
            oc[c] = g.is_out( i + ( c & 1 ), j + ( ( c >> 1 ) & 1 ), k + ( ( c >> 2 ) & 1 ) );
            mask |= int( oc[c] ) << c;
         }
         if ( mask == 0 || mask == 255 ) {
            continue;
         }
         for ( int t = 0; t < 6; ++t ) {
            int cfg = 0;
            for ( int q = 0; q < 4; ++q ) {
               cfg |= int( oc[tt.cv[t][q]] ) << q;
            }
            if ( cfg == 0 || cfg == 15 ) {
               continue;
            }
            const TetCase& tc = tt.c[t][cfg];
            auto ev = [&]( int e ) -> long {
               int c0 = tt.cv[t][TE[e][0]], c1 = tt.cv[t][TE[e][1]];
               long v = vid( i + ( c0 & 1 ), j + ( ( c0 >> 1 ) & 1 ), k + ( ( c0 >> 2 ) & 1 ), ( c0 ^ c1 ) - 1 );
               if ( v < 0 ) {
                  failed = true;
                  v = 0;
               }
               return v;
            };
            if ( tc.ntri == 1 ) {
               tris[s].push_back( { int( ev( tc.tri[0] ) ), int( ev( tc.tri[1] ) ), int( ev( tc.tri[2] ) ) } );
            } else {
               double l0 = norm2( mesh.x[ev( tc.diag[0][0] )] - mesh.x[ev( tc.diag[0][1] )] );
               double l1 = norm2( mesh.x[ev( tc.diag[1][0] )] - mesh.x[ev( tc.diag[1][1] )] );
               int opt2 = l0 <= l1 ? 0 : 1;
               for ( int q = 0; q < 2; ++q ) {
                  const int8_t* tr = tc.quad[opt2][q];
                  tris[s].push_back( { int( ev( tr[0] ) ), int( ev( tr[1] ) ), int( ev( tr[2] ) ) } );
               }
            }
         }
      }
   }, 16 );
   if ( failed ) {
      throw std::logic_error( "a triangle has a vertex on an uncut edge" );
   }
   std::vector<long> tbase( nslots + 1, 0 );
   for ( long s = 0; s < nslots; ++s ) {
      tbase[s + 1] = tbase[s] + long( tris[s].size() );
   }
   mesh.tri.resize( tbase[nslots] );
   parallel_for( nslots, [&]( long s, int ) {
      std::copy( tris[s].begin(), tris[s].end(), mesh.tri.begin() + tbase[s] );
      std::vector<std::array<int, 3>>().swap( tris[s] );
   }, 64 );
   info.seconds_triangles = now() - t4;
}

MeshCheck check_mesh( const Mesh& m ) {
   MeshCheck c;
   const long nv = long( m.x.size() ), nt = long( m.tri.size() );
   c.vertices = nv;
   c.triangles = nt;
   if ( nt == 0 ) {
      return c;
   }
   const double deg = 180 / 3.14159265358979323846;

   // Area, volume and angles.
   // Volumes are taken about a point near the mesh, to keep the sums well conditioned.
   const Vec3 o = m.x[m.tri[0][0]];
   std::vector<double> minang( nt );
   double area = 0, vol = 0;
   long degenerate = 0;
   {
      // Sums per fixed block, added in order: the same whatever the number of threads.
      const long block = 4096, nb = blocks( nt, block );
      std::vector<double> pa( nb, 0 ), pv( nb, 0 );
      std::vector<long> pd( nb, 0 );
      parallel_for( nb, [&]( long kb, int ) {
         for ( long t = kb * block; t < std::min( nt, ( kb + 1 ) * block ); ++t ) {
            const Vec3 &A = m.x[m.tri[t][0]], &B = m.x[m.tri[t][1]], &C = m.x[m.tri[t][2]];
            Vec3 cr = cross( B - A, C - A );
            double a2 = norm( cr );
            pa[kb] += 0.5 * a2;
            pv[kb] += dot( A - o, cross( B - o, C - o ) ) / 6;
            if ( a2 == 0 ) {
               ++pd[kb];
               minang[t] = 0;
               continue;
            }
            double angA = std::atan2( a2, dot( B - A, C - A ) );
            double angB = std::atan2( a2, dot( A - B, C - B ) );
            double angC = 3.14159265358979323846 - angA - angB;
            minang[t] = deg * std::min( angA, std::min( angB, angC ) );
         }
      } );
      for ( long kb = 0; kb < nb; ++kb ) {
         area += pa[kb];
         vol += pv[kb];
         degenerate += pd[kb];
      }
   }
   c.area = area;
   c.volume = vol;
   c.degenerate = degenerate;
   {
      std::vector<double> s( minang );
      std::nth_element( s.begin(), s.begin() + nt / 2, s.end() );
      c.median_min_angle = s[nt / 2];
      c.min_angle = *std::min_element( minang.begin(), minang.end() );
      long b10 = 0, b20 = 0;
      for ( double a : minang ) {
         b10 += a < 10;
         b20 += a < 20;
      }
      c.below_10 = double( b10 ) / nt;
      c.below_20 = double( b20 ) / nt;
   }

   // Edges: each must be in exactly two triangles, once each way.
   {
      std::vector<uint64_t> e( 3 * nt );
      parallel_for( nt, [&]( long t, int ) {
         for ( int k = 0; k < 3; ++k ) {
            uint64_t a = uint64_t( m.tri[t][k] ), b = uint64_t( m.tri[t][( k + 1 ) % 3] );
            uint64_t lo = std::min( a, b ), hi = std::max( a, b );
            e[3 * t + k] = ( lo << 33 ) | ( hi << 1 ) | ( a > b ? 1 : 0 );
         }
      }, 4096 );
      parallel_sort( e );
      for ( size_t i = 0; i < e.size(); ) {
         size_t j = i;
         int fwd = 0;
         while ( j < e.size() && ( e[j] >> 1 ) == ( e[i] >> 1 ) ) {
            fwd += int( e[j] & 1 );
            ++j;
         }
         size_t n = j - i;
         ++c.edges;
         if ( n == 1 ) {
            ++c.open_edges;
         } else if ( n > 2 ) {
            ++c.nonmanifold_edges;
         } else if ( fwd != 1 ) {
            ++c.misoriented_edges;
         }
         i = j;
      }
   }

   // Vertices: the triangles around each must form one fan that closes.
   std::vector<long> start( nv + 1, 0 );
   for ( const auto& t : m.tri ) {
      for ( int k = 0; k < 3; ++k ) {
         ++start[t[k] + 1];
      }
   }
   for ( long v = 0; v < nv; ++v ) {
      start[v + 1] += start[v];
   }
   std::vector<std::array<int, 2>> wedge( start[nv] );
   {
      std::vector<long> fill( start.begin(), start.end() - 1 );
      for ( const auto& t : m.tri ) {
         for ( int k = 0; k < 3; ++k ) {
            wedge[fill[t[k]]++] = { t[( k + 1 ) % 3], t[( k + 2 ) % 3] };
         }
      }
   }
   std::vector<long> unused_t( threads(), 0 ), nmv_t( threads(), 0 );
   parallel_for( nv, [&]( long v, int thread ) {
      long b = start[v], n = start[v + 1] - start[v];
      if ( n == 0 ) {
         ++unused_t[thread];
         return;
      }
      int first = wedge[b][0], cur = wedge[b][1];
      long steps = 1;
      bool ok = true;
      while ( cur != first && ok ) {
         int found = 0;
         int next = -1;
         for ( long w = b; w < b + n; ++w ) {
            if ( wedge[w][0] == cur ) {
               ++found;
               next = wedge[w][1];
            }
         }
         if ( found != 1 || ++steps > n ) {
            ok = false;
         } else {
            cur = next;
         }
      }
      if ( !ok || steps != n ) {
         ++nmv_t[thread];
      }
   }, 1024 );
   long unused = 0, nmv = 0;
   for ( size_t k = 0; k < unused_t.size(); ++k ) {
      unused += unused_t[k];
      nmv += nmv_t[k];
   }
   c.unused_vertices = unused;
   c.nonmanifold_vertices = nmv;
   c.euler = ( nv - unused ) - c.edges + nt;

   // Connected components.
   std::vector<int> parent( nv );
   for ( long v = 0; v < nv; ++v ) {
      parent[v] = int( v );
   }
   auto find = [&]( int x ) {
      while ( parent[x] != x ) {
         parent[x] = parent[parent[x]];
         x = parent[x];
      }
      return x;
   };
   for ( const auto& t : m.tri ) {
      int a = find( t[0] ), b = find( t[1] ), d = find( t[2] );
      parent[b] = a;
      parent[find( d )] = find( a );
   }
   std::vector<int> comp_of( nv, -1 );
   std::vector<ComponentInfo> comps;
   for ( long t = 0; t < nt; ++t ) {
      int r = find( m.tri[t][0] );
      if ( comp_of[r] < 0 ) {
         comp_of[r] = int( comps.size() );
         comps.push_back( { 0, 0, 0 } );
      }
      ComponentInfo& ci = comps[comp_of[r]];
      const Vec3 &A = m.x[m.tri[t][0]], &B = m.x[m.tri[t][1]], &C = m.x[m.tri[t][2]];
      ++ci.triangles;
      ci.area += 0.5 * norm( cross( B - A, C - A ) );
      ci.volume += dot( A - o, cross( B - o, C - o ) ) / 6;
   }
   std::sort( comps.begin(), comps.end(),
              []( const ComponentInfo& a, const ComponentInfo& b ) { return a.triangles > b.triangles; } );
   c.components = comps;
   return c;
}

}  // namespace sesmesh
