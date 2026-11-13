// sesmesh: mesh output; see io.h.
#include "io.h"

#include "parallel.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <cstdint>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace sesmesh {

namespace {

// MSMS face types: 1 contact, 2 reentrant, 3 toroidal.
int msms_type( int feature ) { return feature == F_SPHERE ? 1 : ( feature == F_TRIPLE ? 2 : 3 ); }

FILE* open_out( const std::string& path ) {
   FILE* f = std::fopen( path.c_str(), "w" );
   if ( !f ) {
      throw std::runtime_error( "cannot write " + path );
   }
   return f;
}

void close_out( FILE* f, const std::string& path ) {
   bool err = std::ferror( f ) != 0;
   if ( std::fclose( f ) != 0 || err ) {
      throw std::runtime_error( "error writing " + path );
   }
}

// Writes n lines: line(i, buf) formats line i into buf (LINE bytes) and returns its length.  Chunks
// of lines are formatted in parallel and written in order.
constexpr int LINE = 320;
template <class F>
void write_lines( FILE* f, long n, F&& line ) {
   const long chunk = 8192, batch = 128;   // at most about a million lines in memory
   const long nchunks = ( n + chunk - 1 ) / chunk;
   for ( long c0 = 0; c0 < nchunks; c0 += batch ) {
      const long c1 = std::min( nchunks, c0 + batch );
      std::vector<std::string> out( c1 - c0 );
      parallel_for( c1 - c0, [&]( long kc, int ) {
         const long c = c0 + kc;
         std::string& s = out[kc];
         char buf[LINE];
         const long i1 = std::min( n, ( c + 1 ) * chunk );
         for ( long i = c * chunk; i < i1; ++i ) {
            int k = line( i, buf );
            s.append( buf, size_t( std::max( 0, std::min( k, LINE - 1 ) ) ) );
         }
      } );
      for ( const std::string& s : out ) {
         std::fwrite( s.data(), 1, s.size(), f );
      }
   }
}

// Edge numbers (from 1) for each triangle's edges k = (t[k], t[k+1]), and the edges themselves.  In a
// closed, consistently oriented mesh each edge is the half-edge a -> b with a < b of exactly one
// triangle; the reverse half-edge finds it through the triangles at b.  False for any other mesh.
bool number_edges_closed( const Mesh& m, std::vector<long>& eid, std::vector<std::array<int, 2>>& edges ) {
   const long nt = long( m.tri.size() ), nv = long( m.x.size() );
   std::vector<long> start( nv + 1, 0 );
   for ( const auto& t : m.tri ) {
      for ( int k = 0; k < 3; ++k ) {
         ++start[t[k] + 1];
      }
   }
   for ( long v = 0; v < nv; ++v ) {
      start[v + 1] += start[v];
   }
   std::vector<int> vt( 3 * nt );
   {
      std::vector<long> fill( start.begin(), start.end() - 1 );
      for ( long t = 0; t < nt; ++t ) {
         for ( int k = 0; k < 3; ++k ) {
            vt[fill[m.tri[t][k]]++] = int( t );
         }
      }
   }
   std::vector<long> base( nt + 1, 0 );
   for ( long t = 0; t < nt; ++t ) {
      int own = 0;
      for ( int k = 0; k < 3; ++k ) {
         own += m.tri[t][k] < m.tri[t][( k + 1 ) % 3];
      }
      base[t + 1] = base[t] + own;
   }
   if ( 2 * base[nt] != 3 * nt ) {
      return false;
   }
   eid.assign( 3 * nt, 0 );
   edges.resize( base[nt] );
   parallel_for( nt, [&]( long t, int ) {
      long id = base[t];
      for ( int k = 0; k < 3; ++k ) {
         int a = m.tri[t][k], b = m.tri[t][( k + 1 ) % 3];
         if ( a < b ) {
            edges[id] = { a, b };
            eid[3 * t + k] = ++id;
         }
      }
   }, 4096 );
   std::vector<std::atomic<char>> matched( base[nt] );
   std::vector<long> bad_t( threads(), 0 );
   parallel_for( nt, [&]( long t, int thread ) {
      for ( int k = 0; k < 3; ++k ) {
         int a = m.tri[t][k], b = m.tri[t][( k + 1 ) % 3];
         if ( a < b ) {
            continue;
         }
         int found = 0;
         for ( long w = start[b]; w < start[b + 1]; ++w ) {
            const auto& u = m.tri[vt[w]];
            for ( int kk = 0; kk < 3; ++kk ) {
               if ( u[kk] == b && u[( kk + 1 ) % 3] == a ) {
                  ++found;
                  eid[3 * t + k] = eid[3 * long( vt[w] ) + kk];
               }
            }
         }
         if ( found != 1 || a == b ) {
            ++bad_t[thread];
         } else {
            matched[eid[3 * t + k] - 1].store( 1, std::memory_order_relaxed );   // one half-edge per edge
         }
      }
   }, 4096 );
   for ( long b : bad_t ) {
      if ( b ) {
         return false;
      }
   }
   for ( const std::atomic<char>& c : matched ) {
      if ( !c.load( std::memory_order_relaxed ) ) {
         return false;
      }
   }
   return true;
}

}  // namespace

void write_msms( const std::string& base, const std::string& name, const Mesh& m, int natoms, double probe, double h ) {
   std::string pv = base + ".vert", pf = base + ".face";
   FILE* f = open_out( pv );
   std::fprintf( f, "# MSMS solvent excluded surface vertices for %s (sesmesh)\n", name.c_str() );
   std::fprintf( f, "#vertex #sphere density probe_r\n" );
   std::fprintf( f, "%7ld %7d %6.2f %5.2f\n", long( m.x.size() ), natoms, 1 / ( h * h ), probe );
   write_lines( f, long( m.x.size() ), [&]( long v, char* buf ) {
      return std::snprintf( buf, LINE, "%14.8f %14.8f %14.8f %10.6f %10.6f %10.6f %7d %7d %2d\n", m.x[v].x, m.x[v].y,
                            m.x[v].z, m.n[v].x, m.n[v].y, m.n[v].z, 0, m.atom[v] + 1, msms_type( m.feature[v] ) );
   } );
   close_out( f, pv );
   f = open_out( pf );
   std::fprintf( f, "# MSMS solvent excluded surface faces for %s (sesmesh)\n", name.c_str() );
   std::fprintf( f, "#faces  #sphere density probe_r\n" );
   std::fprintf( f, "%7ld %7d %6.2f %5.2f\n", long( m.tri.size() ), natoms, 1 / ( h * h ), probe );
   write_lines( f, long( m.tri.size() ), [&]( long i, char* buf ) {
      const auto& t = m.tri[i];
      return std::snprintf( buf, LINE, "%7d %7d %7d %2d %6d\n", t[0] + 1, t[1] + 1, t[2] + 1,
                            msms_type( m.feature[t[0]] ), 0 );
   } );
   close_out( f, pf );
}

// msroll's .c3p (vertices with normals, edges, triangles with their edges) and .c3v (areas and
// volume; rcoal reads the area and volume of component 1 from its line 13).
void write_c3p( const std::string& base, const std::string& name, const Mesh& m, int natoms, double probe,
                double area, double volume ) {
   const long nt = long( m.tri.size() );
   std::vector<long> eid;
   std::vector<std::array<int, 2>> edges;
   if ( !number_edges_closed( m, eid, edges ) ) {
      // Not a closed oriented manifold: number the edges by sorting.
      std::vector<std::pair<uint64_t, long>> key( 3 * nt );
      for ( long t = 0; t < nt; ++t ) {
         for ( int k = 0; k < 3; ++k ) {
            uint64_t a = uint64_t( m.tri[t][k] ), b = uint64_t( m.tri[t][( k + 1 ) % 3] );
            key[3 * t + k] = { ( std::min( a, b ) << 32 ) | std::max( a, b ), 3 * t + k };
         }
      }
      std::sort( key.begin(), key.end() );
      eid.assign( 3 * nt, 0 );
      edges.clear();
      for ( size_t i = 0; i < key.size(); ++i ) {
         if ( i == 0 || key[i].first != key[i - 1].first ) {
            edges.push_back( { int( key[i].first >> 32 ), int( key[i].first & 0xffffffffu ) } );
         }
         eid[key[i].second] = long( edges.size() );
      }
   }

   std::string pc = base + ".c3p", pv = base + ".c3v";
   FILE* f = open_out( pc );
   std::fprintf( f, "%ld %ld %ld\n", long( m.x.size() ), long( edges.size() ), nt );
   write_lines( f, long( m.x.size() ), [&]( long v, char* buf ) {
      return std::snprintf( buf, LINE,
                            "%15.8f %15.8f %15.8f %9.6f %9.6f %9.6f   0.000000   0.000000   0.000000   1 %6d %2d\n",
                            m.x[v].x, m.x[v].y, m.x[v].z, m.n[v].x, m.n[v].y, m.n[v].z, m.atom[v] + 1,
                            msms_type( m.feature[v] ) );
   } );
   write_lines( f, long( edges.size() ), [&]( long e, char* buf ) {
      return std::snprintf( buf, LINE, "%7d %7d   1     0   0\n", edges[e][0] + 1, edges[e][1] + 1 );
   } );
   write_lines( f, nt, [&]( long t, char* buf ) {
      return std::snprintf( buf, LINE, "%8ld %8ld %8ld %7d %7d %7d   1     1   0\n", eid[3 * t], eid[3 * t + 1],
                            eid[3 * t + 2], m.tri[t][0] + 1, m.tri[t][1] + 1, m.tri[t][2] + 1 );
   } );
   close_out( f, pc );

   Vec3 c;
   for ( const Vec3& x : m.x ) {
      c += x;
   }
   if ( !m.x.empty() ) {
      c = c / double( m.x.size() );
   }
   f = open_out( pv );
   std::fprintf( f, "molecule name           = %s\n", name.c_str() );
   std::fprintf( f, "number of atoms         = %10d\n", natoms );
   std::fprintf( f, "probe radius            = %14.3f\n", probe );
   std::fprintf( f, "contact area            = %14.3f\n", 0.0 );
   std::fprintf( f, "reentrant area          = %14.3f\n", 0.0 );
   std::fprintf( f, "molecular area          = %14.3f\n", area );
   std::fprintf( f, "accessible area         = %14.3f\n", 0.0 );
   std::fprintf( f, "solvent-excluded volume = %14.3f\n", volume );
   std::fprintf( f, "\n" );
   std::fprintf( f, "component           center              volume    molecular  accessible\n" );
   std::fprintf( f, " number      x        y        z                    area       area\n" );
   std::fprintf( f, "\n" );
   std::fprintf( f, "    1  %10.3f %8.3f %8.3f %11.3f %11.3f %11.3f\n", c.x, c.y, c.z, volume, area, 0.0 );
   close_out( f, pv );
}

// BEST's triangle format: N, area and volume, then the nine coordinates of each triangle.
void write_best( const std::string& base, const Mesh& m, double area, double volume ) {
   std::string path = base + ".best";
   FILE* f = open_out( path );
   std::fprintf( f, "%ld %.6f %.6f\n", long( m.tri.size() ), area, volume );
   write_lines( f, long( m.tri.size() ), [&]( long i, char* buf ) {
      const auto& t = m.tri[i];
      const Vec3 &a = m.x[t[0]], &b = m.x[t[1]], &c = m.x[t[2]];
      return std::snprintf( buf, LINE, "%.8f %.8f %.8f %.8f %.8f %.8f %.8f %.8f %.8f\n", a.x, a.y, a.z, b.x, b.y, b.z,
                            c.x, c.y, c.z );
   } );
   close_out( f, path );
}

// ZENO's .bod format (ZENO 5.3.1 accepts no comment lines).
void write_bod( const std::string& base, const Mesh& m, double mass, double temp, double visc ) {
   std::string path = base + ".bod";
   FILE* f = open_out( path );
   std::fprintf( f, "UNITS A\nTEMP %g C\nVISCOSITY %g cp\n", temp, visc );
   if ( mass > 0 ) {
      std::fprintf( f, "MASS %g Da\n", mass );
   }
   write_lines( f, long( m.tri.size() ), [&]( long i, char* buf ) {
      const auto& t = m.tri[i];
      const Vec3 &a = m.x[t[0]], &b = m.x[t[1]], &c = m.x[t[2]];
      return std::snprintf( buf, LINE, "TRIANGLE %.8f %.8f %.8f %.8f %.8f %.8f %.8f %.8f %.8f\n", a.x, a.y, a.z, b.x,
                            b.y, b.z, c.x, c.y, c.z );
   } );
   close_out( f, path );
}

}  // namespace sesmesh
