// sesmesh rcoal: the files rcoal reads and writes; see rcoal_io.h.
//
// Copyright (c) 1998-2025 Sergio R. Aragon (Coalesce and rcoal, in ehb54/best-hydro)
// Copyright (c) 2026 Emre H. Brookes (this port, and the fixes of best-hydro PRs #1 and #2)
// MIT licence: see LICENSE.
#include "rcoal_io.h"

#include "fortio.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <stdexcept>

namespace sesmesh::rcoal {

// The output base name: the input name up to its extension (.c3p, or .fac of .face).
std::string base_name( const std::string& in, bool sanner ) {
   std::string ext = sanner ? ".fac" : ".c3p";
   size_t p = in.find( ext );
   if ( p == std::string::npos ) {
      throw std::runtime_error( "input file name must end in " + std::string( sanner ? ".face" : ".c3p" ) + ": " + in );
   }
   return in.substr( 0, p );
}

// msroll's .c3p and, from its .c3v, the volume and molecular area of component 1.
Surface read_c3p( const std::string& name ) {
   Surface s;
   s.input_name = name;
   ListReader r( name );
   std::vector<double> v;
   r.read( v, 3 );
   int nvert = int( v[0] ), nedge = int( v[1] ), ntriang = int( v[2] );
   s.x.resize( nvert + 1 );
   for ( int n = 1; n <= nvert; ++n ) {
      r.read( v, 12 );
      s.x[n] = { v[0], v[1], v[2] };
   }
   s.edges.resize( nedge );
   for ( int n = 0; n < nedge; ++n ) {
      r.read( v, 5 );
      s.edges[n] = { int( v[0] ), int( v[1] ) };
      for ( int k = 0; k < 2; ++k ) {
         if ( s.edges[n][k] < 1 || s.edges[n][k] > nvert ) {
            throw std::runtime_error( "edge with a vertex number out of range in " + name );
         }
      }
   }
   s.tri.resize( ntriang );
   s.comp.resize( ntriang );
   double totarea = 0.0;
   for ( int n = 0; n < ntriang; ++n ) {
      r.read( v, 9 );
      s.tri[n] = { int( v[3] ), int( v[4] ), int( v[5] ) };
      s.comp[n] = int( v[6] );
      for ( int k = 0; k < 3; ++k ) {
         if ( s.tri[n][k] < 1 || s.tri[n][k] > nvert ) {
            throw std::runtime_error( "triangle with a vertex number out of range in " + name );
         }
      }
      totarea = totarea + tri_area( s.x[s.tri[n][0]], s.x[s.tri[n][1]], s.x[s.tri[n][2]] );
   }
   s.begin_area = totarea;
   // .c3v, line 13: component, centre (3), volume, molecular area, accessible area
   std::string c3v = name.substr( 0, name.size() - 4 ) + ".c3v";
   ListReader rv( c3v );
   for ( int n = 0; n < 12; ++n ) {
      rv.skip_line();
   }
   rv.read( v, 7 );
   s.true_vol = v[4];
   s.true_area = v[5];
   return s;
}

// MSMS's .vert and .face, and from its .msms.log the analytical area and numerical volume.
Surface read_msms( const std::string& face ) {
   Surface s;
   s.input_name = face;
   s.sanner = true;
   std::string base = base_name( face, true );
   std::vector<double> v;
   ListReader rv( base + ".vert" );
   rv.skip_line();
   rv.skip_line();
   rv.read( v, 4 );
   int nvert = int( v[0] );
   s.x.resize( nvert + 1 );
   for ( int n = 1; n <= nvert; ++n ) {
      rv.read( v, 8 );
      s.x[n] = { v[0], v[1], v[2] };
   }
   ListReader rf( face );
   rf.skip_line();
   rf.skip_line();
   rf.read( v, 4 );
   int ntriang = int( v[0] );
   s.tri.resize( ntriang );
   s.comp.assign( ntriang, 1 );
   double totarea = 0.0;
   for ( int n = 0; n < ntriang; ++n ) {
      rf.read( v, 5 );
      s.tri[n] = { int( v[0] ), int( v[1] ), int( v[2] ) };
      for ( int k = 0; k < 3; ++k ) {
         if ( s.tri[n][k] < 1 || s.tri[n][k] > nvert ) {
            throw std::runtime_error( "face with a vertex number out of range in " + face );
         }
      }
      totarea = totarea + tri_area( s.x[s.tri[n][0]], s.x[s.tri[n][1]], s.x[s.tri[n][2]] );
   }
   s.begin_area = totarea;
   // The unique edges, in order of first appearance, as the Fortran builds them.
   std::vector<std::vector<int>> adj( nvert + 1 );
   for ( const T3& t : s.tri ) {
      for ( int m = 0; m < 3; ++m ) {
         int a = t[m], b = t[( m + 1 ) % 3];
         int lo = std::min( a, b ), hi = std::max( a, b );
         bool dup = false;
         for ( int w : adj[lo] ) {
            if ( w == hi ) {
               dup = true;
               break;
            }
         }
         if ( !dup ) {
            adj[lo].push_back( hi );
            s.edges.push_back( { a, b } );
         }
      }
   }
   // name.msms.log: the analytical SES area, and the numerical volume and area
   std::ifstream lg( base + ".msms.log" );
   std::string line;
   auto toks = []( const std::string& l ) {
      std::vector<std::string> t;
      size_t i = 0;
      while ( i < l.size() ) {
         while ( i < l.size() && ( l[i] == ' ' || l[i] == '\t' || l[i] == ',' ) ) {
            ++i;
         }
         size_t j = i;
         while ( j < l.size() && l[j] != ' ' && l[j] != '\t' && l[j] != ',' ) {
            ++j;
         }
         if ( j > i ) {
            t.push_back( l.substr( i, j - i ) );
         }
         i = j;
      }
      return t;
   };
   while ( std::getline( lg, line ) ) {
      auto t = toks( line );
      if ( t.size() >= 2 && t[0].compare( 0, 10, "ANALYTICAL" ) == 0 && t[1].compare( 0, 7, "SURFACE" ) == 0 ) {
         std::getline( lg, line );
         std::getline( lg, line );
         auto u = toks( line );
         if ( u.size() >= 6 ) {
            s.true_area = std::strtod( u[5].c_str(), nullptr );
         }
      } else if ( !t.empty() && t[0].compare( 0, 9, "NUMERICAL" ) == 0 ) {
         std::getline( lg, line );
         std::getline( lg, line );
         auto u = toks( line );
         if ( u.size() >= 4 ) {
            s.poly_vol = std::strtod( u[2].c_str(), nullptr );
            s.poly_area = std::strtod( u[3].c_str(), nullptr );
         }
      }
   }
   s.true_vol = s.poly_vol;
   return s;
}

namespace {

// The area histogram: subroutines bin and dstrb, 1-based as in the Fortran.
void bin( double e, double s, std::vector<double>& bins, int numbin ) {
   bins.assign( numbin + 1, 0.0 );
   for ( int j = 1; j <= ( numbin - 1 ) / 2; ++j ) {
      bins[j] = e - double( ( numbin + 1 ) / 2 - j ) * s * e / 2.5;
      bins[j + ( numbin + 1 ) / 2] = e + double( j ) * s * e / 2.5;
   }
   bins[( numbin + 1 ) / 2] = e;
}

void dstrb( double z, std::vector<int>& nd, const std::vector<double>& bins, int numbin ) {
   for ( int j = 1; j <= ( numbin - 2 ) / 2; ++j ) {
      if ( z <= bins[-j + 1 + ( numbin / 2 )] && z > bins[( numbin / 2 ) - j] ) {
         nd[( numbin / 2 ) + 1 - j]++;
         return;
      }
      if ( z > bins[( numbin / 2 ) - 1 + j] && z <= bins[( numbin / 2 ) + j] ) {
         nd[( numbin / 2 ) - 1 + j]++;
         return;
      }
   }
   if ( z <= bins[1] ) {
      nd[1]++;
   } else if ( z > bins[numbin] ) {
      nd[numbin]++;
   } else if ( z <= bins[numbin] && z > bins[numbin - 1] ) {
      nd[numbin - 1]++;
   }
}

void say( std::ostream& os, const ListRecord& r ) { os << r.str() << '\n'; }

}  // namespace

void write_summary( std::ostream& os, const Surface& s, const Stats& st, bool his, bool v3 ) {
   const double ver2 = 3.1, ver3 = 3.5;
   std::vector<ListRecord> L;
   auto add = [&]( const ListRecord& r ) { L.push_back( r ); };
   add( ListRecord().c( "COALESCE PROCESS RESULTS -Version= " ).r( v3 ? ver3 : ver2 ) );
   add( ListRecord().c( " " ) );
   add( ListRecord().c( s.sanner ? "Input File (Sanner): " : "Input File (Connolly): " ).c80( s.input_name ) );
   add( ListRecord().c( " Edge fraction = " ).r( st.edgfrac ) );
   add( ListRecord().c( " " ) );
   add( ListRecord().c( " Beginning polygon Area =  " ).r( s.begin_area ) );
   add( ListRecord().c( his ? "Initial number of triangle records= " : "Number of triangle records, pass 1= " ).i( st.ntriang ) );
   add( ListRecord().c( "Number of degenerate triangles, pass 1= " ).i( st.idegen ) );
   add( ListRecord().c( "Number of small triangles deleted, pass 1= " ).i( st.idelete1 ) );
   add( ListRecord().c( "Number of polygons generated, pass 1= " ).i( st.npolygon1 ) );
   add( ListRecord().c( his ? "total area, pass 1 = " : "Total area, pass 1 = " ).r( st.totarea1 ) );
   add( ListRecord().c( " " ) );
   add( ListRecord().c( "Number of small triangles deleted, pass 2= " ).i( st.idelete ) );
   add( ListRecord().c( "Number of polygons generated, pass 2= " ).i( st.npoly ) );
   add( ListRecord().c( his ? "total area, pass 2 = " : "Total area, pass 2 = " ).r( st.totarea ) );
   add( ListRecord().c( "Average Triangle Area (A^2)= " ).r( st.avarea ) );
   add( ListRecord().c( "Average edgelength (A)= " ).r( st.avelength / st.nedge ) );
   add( ListRecord().c( "Cut-off length (A) = " ).r( st.cutofflength ) );
   add( ListRecord().c( "Found" ).i( st.nobtuse ).c( " slender obtuse triangles, processed" ).i( st.icon ) );
   if ( !his ) {
      add( ListRecord().c( "Collapses skipped by the topology check: " ).i( st.nlinkskip ) );
      add( ListRecord().c( "Flips skipped by the topology check: " ).i( st.nflipskip ) );
   } else {
      add( ListRecord().c( " " ) );
   }
   for ( auto& r : L ) {
      os << r.str() << '\n';
   }
}

int write_result( const std::string& base, const Surface& s, const Result& R, double edgfrac, bool v3, std::ostream& msg,
                  std::vector<std::string>& written, int* nsmall ) {
   const Stats& st = R.st;
   // the coordinates as written
   std::vector<std::string> cv( 3 * st.npoly );
   int nzero = 0;
   if ( nsmall ) {
      *nsmall = 0;
   }
   for ( int m = 0; m < st.npoly; ++m ) {
      for ( int k = 0; k < 3; ++k ) {
         const V3& p = s.x[R.tri[m][k]];
         cv[3 * m + k] = fort_e( p[0], 17, 8 ) + fort_e( p[1], 17, 8 ) + fort_e( p[2], 17, 8 );
      }
      if ( cv[3 * m] == cv[3 * m + 1] || cv[3 * m + 1] == cv[3 * m + 2] || cv[3 * m] == cv[3 * m + 2] ) {
         ++nzero;
      }
   }
   if ( nzero > 0 ) {
      say( msg, ListRecord().c( "Triangles with two vertices at the same position: " ).i( nzero ).c( "; not written" ) );
      return nzero;
   }
   char num[16];
   std::snprintf( num, sizeof num, "%05d", st.npoly % 100000 );
   std::string output = base + "_" + num;
   std::string outputhis = output + ".his";

   {
      std::ofstream h( outputhis );
      write_summary( h, s, st, true, v3 );
      h << ListRecord().c( "Histogram bins and count follows:" ).str() << '\n';
      std::vector<double> bins, bins2;
      std::vector<int> nd1( 51, 0 ), nd2( 51, 0 );
      int numbin = 12;
      bin( 0.1 * st.avarea, 0.5, bins, numbin + 1 );
      for ( int m = 0; m < st.npoly; ++m ) {
         if ( R.area[m] <= 0.2 * st.avarea ) {
            dstrb( R.area[m], nd1, bins, numbin );
         }
      }
      h << ListRecord().c( "Area distribution around" ).r( .1 * st.avarea ).c( " A^2" ).str() << '\n';
      int nsmallarea = 0;
      for ( int n = 2; n <= numbin; ++n ) {
         h << ListRecord().r( bins[n] ).i( nd1[n] ).str() << '\n';
         nsmallarea += nd1[n];
      }
      if ( nsmallarea > 0 ) {
         say( msg, ListRecord().c( " " ) );
         say( msg, ListRecord().c( "************************" ) );
         say( msg, ListRecord().c( "N =  " ).i( st.npoly ) );
         say( msg, ListRecord().c( "ALERT! " ).i( nsmallarea ).c( " Triangles with Area< 20% average area Remain!" ) );
         say( msg, ListRecord().c( "Review histogram." ) );
         say( msg, ListRecord().c( "Edgefrac = " ).r( edgfrac ) );
         say( msg, ListRecord().c( "************************" ) );
      }
      if ( nsmall ) {
         *nsmall = nsmallarea;
      }
      numbin = 22;
      bin( st.avarea, 0.2, bins2, numbin + 1 );
      for ( int m = 0; m < st.npoly; ++m ) {
         if ( R.area[m] > 0.2 * st.avarea ) {
            dstrb( R.area[m], nd2, bins2, numbin );
         }
      }
      h << ListRecord().c( "Area distribution around" ).r( st.avarea ).c( "A^2" ).str() << '\n';
      for ( int n = 2; n <= numbin; ++n ) {
         h << ListRecord().r( bins2[n] ).i( nd2[n] ).str() << '\n';
      }
      if ( !h ) {
         throw std::runtime_error( "cannot write " + outputhis );
      }
   }
   {
      std::ofstream o( output );
      o << ListRecord().i( st.npoly ).r( s.true_area ).r( s.true_vol ).str() << '\n';
      for ( int m = 0; m < st.npoly; ++m ) {
         o << cv[3 * m] << cv[3 * m + 1] << cv[3 * m + 2] << '\n';
      }
      if ( !o ) {
         throw std::runtime_error( "cannot write " + output );
      }
   }
   written.push_back( output );
   written.push_back( outputhis );
   return 0;
}

}  // namespace sesmesh::rcoal
