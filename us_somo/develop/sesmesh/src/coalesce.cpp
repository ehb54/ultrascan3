// sesmesh rcoal: Coalesce; see coalesce.h.  Comments name the Fortran they follow
// (scoalesce.f).
//
// Copyright (c) 1998-2025 Sergio R. Aragon (Coalesce and rcoal, in ehb54/best-hydro)
// Copyright (c) 2026 Emre H. Brookes (this port, and the fixes of best-hydro PRs #1 and #2)
// MIT licence: see LICENSE.
#include "coalesce.h"

#include "fortio.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <unordered_set>

namespace sesmesh::rcoal {

double tri_area( const V3& v1, const V3& v2, const V3& v3 ) {
   // subroutine areas
   double v21x = v2[0] - v1[0];
   double v21y = v2[1] - v1[1];
   double v21z = v2[2] - v1[2];
   double v31x = v3[0] - v1[0];
   double v31y = v3[1] - v1[1];
   double v31z = v3[2] - v1[2];
   double a = v31y * v21z - v31z * v21y;
   double b = v31z * v21x - v31x * v21z;
   double c = v31x * v21y - v31y * v21x;
   return std::sqrt( a * a + b * b + c * c ) / 2.;
}

namespace {

// subroutine slength(v1, v2, slen)
double slength( const V3& v1, const V3& v2 ) {
   double x = v2[0] - v1[0];
   double y = v2[1] - v1[1];
   double z = v2[2] - v1[2];
   return std::sqrt( x * x + y * y + z * z );
}

inline bool has( const T3& t, int v ) { return t[0] == v || t[1] == v || t[2] == v; }

void sorted_insert( std::vector<int>& l, int k ) {
   l.insert( std::lower_bound( l.begin(), l.end(), k ), k );
}
void sorted_erase( std::vector<int>& l, int k ) {
   auto it = std::lower_bound( l.begin(), l.end(), k );
   if ( it != l.end() && *it == k ) {
      l.erase( it );
   }
}

struct State {
   const std::vector<V3>& x;
   std::vector<T3> vn;                 // -1 in all three: deleted
   std::vector<std::vector<int>> inc;  // live triangles on each vertex, ascending
   int idelete = 0;
   int nlinkskip = 0;
   // scratch
   std::vector<int> t1, t2, nb1, nb2;

   explicit State( const std::vector<V3>& xx ) : x( xx ) {}

   bool alive( int k ) const { return vn[k][0] != -1; }

   void kill( int k ) {
      for ( int m = 0; m < 3; ++m ) {
         sorted_erase( inc[vn[k][m]], k );
      }
      vn[k] = { -1, -1, -1 };
   }
   // Replaces vertex position m of triangle k by v.
   void set_vertex( int k, int m, int v ) {
      int old = vn[k][m];
      vn[k][m] = v;
      if ( !has( vn[k], old ) ) {
         sorted_erase( inc[old], k );
      }
      // v cannot already be in a live, non-degenerate triangle other than at m
      sorted_insert( inc[v], k );
   }

   // subroutine slender_del: collapses edge v1-v2.
   void slender_del( int v1, int v2 ) {
      t1 = inc[v1];
      t2 = inc[v2];
      int first = int( t1.size() ), second = int( t2.size() );
      if ( first == 0 || second == 0 ) {
         return;
      }
      nb1.clear();
      nb2.clear();
      int nboth = 0;
      int iapex[2] = { 0, 0 };
      for ( int k : t1 ) {
         for ( int m = 0; m < 3; ++m ) {
            if ( vn[k][m] != v1 && vn[k][m] != v2 ) {
               nb1.push_back( vn[k][m] );
            }
         }
         if ( has( vn[k], v2 ) ) {
            ++nboth;
            if ( nboth <= 2 ) {
               for ( int m = 0; m < 3; ++m ) {
                  if ( vn[k][m] != v1 && vn[k][m] != v2 ) {
                     iapex[nboth - 1] = vn[k][m];
                  }
               }
            }
         }
      }
      for ( int k : t2 ) {
         for ( int m = 0; m < 3; ++m ) {
            if ( vn[k][m] != v1 && vn[k][m] != v2 ) {
               nb2.push_back( vn[k][m] );
            }
         }
      }
      const V3 &a = x[v1], &b = x[v2];
      double dx = a[0] - b[0], dy = a[1] - b[1], dz = a[2] - b[2];
      double d2 = dx * dx + dy * dy + dz * dz;
      if ( d2 > 1.0e-10 ) {
         if ( nboth != 2 || iapex[0] == iapex[1] ) {
            ++nlinkskip;
            return;
         }
         // distinct entries of nb1 that are in nb2
         std::sort( nb1.begin(), nb1.end() );
         nb1.erase( std::unique( nb1.begin(), nb1.end() ), nb1.end() );
         std::sort( nb2.begin(), nb2.end() );
         int ncommon = 0;
         for ( int v : nb1 ) {
            if ( std::binary_search( nb2.begin(), nb2.end(), v ) ) {
               ++ncommon;
            }
         }
         if ( ncommon != 2 ) {
            ++nlinkskip;
            return;
         }
      }
      // Delete the triangles on the edge.
      for ( int k : t1 ) {
         if ( has( vn[k], v2 ) ) {
            ++idelete;
            kill( k );
         }
      }
      // Coalesce in the direction of the vertex shared by more triangles.
      if ( second >= first ) {
         for ( int k : t1 ) {
            for ( int m = 0; m < 3; ++m ) {
               if ( vn[k][m] == v1 ) {
                  set_vertex( k, m, v2 );
                  break;
               }
            }
         }
      } else {
         for ( int k : t2 ) {
            for ( int m = 0; m < 3; ++m ) {
               if ( vn[k][m] == v2 ) {
                  set_vertex( k, m, v1 );
                  break;
               }
            }
         }
      }
   }
};

}  // namespace

void heap_index( const std::vector<double>& array, std::vector<int>& indx ) {
   // index.f, with 1-based ix[] internally
   int n = int( array.size() );
   indx.resize( n );
   if ( n < 2 ) {
      for ( int j = 0; j < n; ++j ) {
         indx[j] = j;
      }
      return;
   }
   std::vector<int> ix( n + 1 );
   for ( int j = 1; j <= n; ++j ) {
      ix[j] = j;
   }
   auto a = [&]( int i ) { return array[i - 1]; };
   int k = n / 2 + 1, ir = n;
   for ( ;; ) {
      int indxt;
      double q;
      if ( k > 1 ) {
         --k;
         indxt = ix[k];
         q = a( indxt );
      } else {
         indxt = ix[ir];
         q = a( indxt );
         ix[ir] = ix[1];
         --ir;
         if ( ir == 1 ) {
            ix[1] = indxt;
            break;
         }
      }
      int i = k, j = k + k;
      while ( j <= ir ) {
         if ( j < ir && a( ix[j] ) < a( ix[j + 1] ) ) {
            ++j;
         }
         if ( q < a( ix[j] ) ) {
            ix[i] = ix[j];
            i = j;
            j += j;
         } else {
            j = ir + 1;
         }
      }
      ix[i] = indxt;
   }
   for ( int j = 0; j < n; ++j ) {
      indx[j] = ix[j + 1] - 1;
   }
}

Result coalesce( const Surface& s, double edgfrac, std::vector<std::string>* log ) {
   Result res;
   Stats& st = res.st;
   st.edgfrac = edgfrac;
   const int ncom = 1;
   const int ntriang = int( s.tri.size() );
   const int nvert = int( s.x.size() ) - 1;
   st.ntriang = ntriang;
   auto say = [&]( const ListRecord& r ) {
      if ( log ) {
         log->push_back( r.str() );
      }
   };

   State S( s.x );
   S.vn = s.tri;
   S.inc.assign( nvert + 1, {} );

   // DEGENERATE triangle elimination
   for ( int m = 0; m < ntriang; ++m ) {
      T3& t = S.vn[m];
      if ( t[0] == t[1] || t[0] == t[2] || t[1] == t[2] || s.comp[m] != ncom ) {
         ++st.idegen;
         t = { -1, -1, -1 };
      }
   }
   for ( int m = 0; m < ntriang; ++m ) {
      if ( S.alive( m ) ) {
         for ( int k = 0; k < 3; ++k ) {
            S.inc[S.vn[m][k]].push_back( m );
         }
      }
   }

   std::vector<std::array<int, 2>> edgn = s.edges;
   std::vector<double> edglen;
   std::vector<int> indx;
   int npolygon = 0;
   int ipass = 1;
   double avelength = 0, cutofflength = 0;
   const double slender = 0.97;
   const double slenderfrac = slender / ( 1 + slender );

   for ( ;; ) {
      // SMALL EDGE detection
      int nedge = int( edgn.size() );
      edglen.resize( nedge );
      avelength = 0.0;
      for ( int j = 0; j < nedge; ++j ) {
         const V3 &a = s.x[edgn[j][0]], &b = s.x[edgn[j][1]];
         double a1 = a[0] - b[0], a2 = a[1] - b[1], a3 = a[2] - b[2];
         edglen[j] = std::sqrt( a1 * a1 + a2 * a2 + a3 * a3 );
         avelength = avelength + edglen[j];
      }
      cutofflength = edgfrac * avelength / nedge;
      heap_index( edglen, indx );
      S.idelete = 0;
      // SLENDER BASE TRIANGLE ELIMINATION
      for ( int j = 0; j < nedge; ++j ) {
         if ( edglen[indx[j]] >= cutofflength ) {
            break;
         }
         S.slender_del( edgn[indx[j]][0], edgn[indx[j]][1] );
      }
      say( ListRecord().c( "Finished deleting small triangles: Pass= " ).i( ipass ) );
      say( ListRecord().c( " " ) );
      st.nedge = nedge;

      int idang = 0;   // dangle_del is never enabled
      if ( ipass == 1 ) {
         npolygon = ntriang - st.idegen - S.idelete;
      } else {
         // RESHAPING SLENDER OBTUSE TRIANGLES
         std::vector<int> obt, iobv;
         for ( int j = 0; j < ntriang; ++j ) {
            if ( !S.alive( j ) ) {
               continue;
            }
            const V3 &vx = s.x[S.vn[j][0]], &vy = s.x[S.vn[j][1]], &vz = s.x[S.vn[j][2]];
            double slen21 = slength( vy, vx );
            double slen31 = slength( vz, vx );
            double slen32 = slength( vz, vy );
            double slenmax = std::max( std::max( slen21, slen31 ), slen32 );
            double perim = slen21 + slen31 + slen32;
            if ( slenmax >= slenderfrac * perim ) {
               double slenmin = std::min( std::min( slen21, slen31 ), slen32 );
               if ( slenmin / slenmax >= 0.33 ) {
                  obt.push_back( j );
                  if ( slen32 >= slen21 && slen32 >= slen31 ) {
                     iobv.push_back( 0 );
                  } else if ( slen31 >= slen21 && slen31 >= slen32 ) {
                     iobv.push_back( 1 );
                  } else {
                     iobv.push_back( 2 );
                  }
               }
            }
         }
         st.nobtuse = int( obt.size() );
         say( ListRecord().i( st.nobtuse ).c( " Slender obtuse triangles found!" ) );

         std::vector<int> cand;
         for ( size_t jj = 0; jj < obt.size(); ++jj ) {
            const int J = obt[jj];
            const int ob = iobv[jj];
            const int L1 = ( ob + 2 ) % 3, L2 = ( ob + 1 ) % 3;
            ++st.icon;
            int u1 = S.vn[J][L1], u2 = S.vn[J][L2];
            if ( u1 == -1 ) {
               continue;
            }
            // the first live triangle k != J that has both acute vertices
            const std::vector<int>& la = S.inc[u1];
            const std::vector<int>& lb = S.inc[u2];
            int k = -1;
            {
               size_t p = 0, q = 0;
               while ( p < la.size() && q < lb.size() ) {
                  if ( la[p] < lb[q] ) {
                     ++p;
                  } else if ( lb[q] < la[p] ) {
                     ++q;
                  } else {
                     if ( la[p] != J ) {
                        k = la[p];
                        break;
                     }
                     ++p;
                     ++q;
                  }
               }
            }
            if ( k < 0 ) {
               continue;
            }
            // Skip the flip if its new edge already exists.
            int ia = S.vn[J][ob];
            int id = 0;
            for ( int m = 0; m < 3; ++m ) {
               if ( S.vn[k][m] != u1 && S.vn[k][m] != u2 ) {
                  id = S.vn[k][m];
               }
            }
            bool lflip = ia != id;
            if ( lflip ) {
               const std::vector<int>& lc = S.inc[ia];
               const std::vector<int>& ld = S.inc[id];
               size_t p = 0, q = 0;
               while ( p < lc.size() && q < ld.size() ) {
                  if ( lc[p] < ld[q] ) {
                     ++p;
                  } else if ( ld[q] < lc[p] ) {
                     ++q;
                  } else {
                     if ( lc[p] != k && lc[p] != J ) {
                        lflip = false;
                        break;
                     }
                     ++p;
                     ++q;
                  }
               }
            }
            if ( !lflip ) {
               ++st.nflipskip;
               continue;
            }
            // The flip: J's L1 corner goes to k's third vertex, k's u2 corner to the obtuse vertex.
            {
               const T3 tk = S.vn[k];
               int mL2 = -1, mD = -1;
               for ( int m = 0; m < 3; ++m ) {
                  if ( tk[m] == u2 ) {
                     mL2 = m;
                  } else if ( tk[m] != u1 ) {
                     mD = m;
                  }
               }
               S.set_vertex( J, L1, tk[mD] );
               S.set_vertex( k, mL2, ia );
            }
            // Test that the new areas are ok.
            double areanew = tri_area( s.x[S.vn[k][0]], s.x[S.vn[k][1]], s.x[S.vn[k][2]] );
            if ( areanew < 0.06 ) {
               int iv1 = S.vn[k][0], iv2 = S.vn[k][1];
               S.slender_del( iv1, iv2 );
            }
            if ( S.alive( J ) ) {
               areanew = tri_area( s.x[S.vn[J][0]], s.x[S.vn[J][1]], s.x[S.vn[J][2]] );
               if ( areanew < 0.06 ) {
                  int iv1 = S.vn[J][L1], iv2 = S.vn[J][L2];
                  S.slender_del( iv1, iv2 );
               }
            }
         }
         npolygon = npolygon - idang - S.idelete;
         say( ListRecord().c( "Processed " ).i( st.icon ).c( " obtuse slender triangles." ) );
      }

      // BUILD NEW TRIANGLES
      res.tri.clear();
      res.area.clear();
      double totarea = 0.0;
      for ( int k = 0; k < ntriang; ++k ) {
         if ( S.alive( k ) ) {
            res.tri.push_back( S.vn[k] );
            double a = tri_area( s.x[S.vn[k][0]], s.x[S.vn[k][1]], s.x[S.vn[k][2]] );
            res.area.push_back( a );
            totarea = totarea + a;
         }
      }
      int npoly = int( res.tri.size() );
      if ( npoly != npolygon ) {
         say( ListRecord()
                 .c( "Triangles counted in pass" )
                 .i( ipass )
                 .c( ":" )
                 .i( npoly )
                 .c( " computed npolygon= " )
                 .i( npolygon ) );
      }
      if ( ipass == 1 ) {
         st.idelete1 = S.idelete;
         st.npolygon1 = npolygon;
         st.totarea1 = totarea;
         st.nedge1 = nedge;
         ipass = 2;
         say( ListRecord().c( " " ) );
         say( ListRecord().c( "******Now checking new edge list*******" ) );
         // the unique edges of the live triangles, in order of first appearance
         std::unordered_set<long long> seen;
         seen.reserve( size_t( 3 * npoly * 2 ) );
         edgn.clear();
         for ( int k = 0; k < ntriang; ++k ) {
            if ( !S.alive( k ) ) {
               continue;
            }
            const T3& t = S.vn[k];
            for ( int m = 0; m < 3; ++m ) {
               int a = t[m], b = t[( m + 1 ) % 3];
               long long key = ( long long )std::min( a, b ) * ( nvert + 1LL ) + std::max( a, b );
               if ( seen.insert( key ).second ) {
                  edgn.push_back( { a, b } );
               }
            }
         }
         say( ListRecord().i( long( edgn.size() ) ).c( " New unique edges found" ) );
         say( ListRecord().c( " " ) );
         continue;
      }
      st.idelete = S.idelete;
      st.npoly = npoly;
      st.npolygon = npolygon;
      st.totarea = totarea;
      st.avarea = totarea / npoly;
      st.avelength = avelength;
      st.nedge2 = nedge;
      st.cutofflength = cutofflength;
      st.nlinkskip = S.nlinkskip;
      break;
   }
   return res;
}

double find_edgfrac( const Surface& s, int target, int* n ) {
   auto count = [&]( double e ) { return coalesce( s, e ).st.npoly; };
   double lo = 0.05, hi = 0.7;
   int nlo = count( lo ), nhi = count( hi );
   while ( nhi > target && hi < 20 ) {
      lo = hi;
      nlo = nhi;
      hi *= 1.5;
      nhi = count( hi );
   }
   for ( int it = 0; it < 40 && nlo != target && nhi != target; ++it ) {
      double mid = 0.5 * ( lo + hi );
      int nm = count( mid );
      if ( nm > target ) {
         lo = mid;
         nlo = nm;
      } else {
         hi = mid;
         nhi = nm;
      }
   }
   bool low = std::abs( nlo - target ) <= std::abs( nhi - target );
   if ( n ) {
      *n = low ? nlo : nhi;
   }
   return low ? lo : hi;
}

}  // namespace sesmesh::rcoal
