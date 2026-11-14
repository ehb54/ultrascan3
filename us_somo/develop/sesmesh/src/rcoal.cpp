// rcoal: makes a series of BEST input meshes with Coalesce.  A C++ drop-in for rcoal 2.0
// (rcoal.f) and rcoal 3.0 (rcoal3.f) in ehb54/best-hydro, as fixed there in PRs #1 and #2: the
// same command line, the same output files, byte for byte, and the same stdout, including
// the "Output files written:" lines that SOMO reads.
//
// Copyright (c) 1998-2025 Sergio R. Aragon (Coalesce and rcoal, in ehb54/best-hydro)
// Copyright (c) 2026 Emre H. Brookes (this port, and the fixes of best-hydro PRs #1 and #2)
// MIT licence: see LICENSE.
//
//   rcoal -f name.c3p -nmax N -nmin N -n K   K meshes from nmin up to nmax triangles
//   rcoal -f name.c3p -e edgfrac             one mesh
//   -s        MSMS input: give name.face; name.vert and name.msms.log are read too
//   -search   find e for each target by bisection, instead of rcoal's two-run fit
//   -v3       rcoal 3.0's messages (the default when run as rcoal3, and with -s)
//
// Differences from the Fortran:
//  - Options are matched exactly.  The Fortran matches -n, -e and -s as prefixes, so a -n
//    given before -nmax or -nmin was overwritten by their values.
//  - No limit on the length of file names or on the number of meshes.
//  - -search is new.
#include "coalesce.h"
#include "fortio.h"
#include "rcoal_io.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace sesmesh::rcoal;

namespace {

bool v3 = false;                 // rcoal 3.0's messages
std::string prog = "rcoal";      // the name the program was run as, for messages
std::vector<std::string> outf;   // the files written, in order

void out( const ListRecord& r ) { std::cout << r.str() << '\n'; }

// One call of subroutine scoalesce: coalesce with edge fraction edgfrac, print what the
// Fortran prints, and, if write, write the result.  Returns the number of triangles.
int scoalesce( const Surface& s, double edgfrac, bool write, int& nzero ) {
   nzero = 0;
   out( ListRecord().c( " " ) );
   out( ListRecord().c( "Begin Coalesce Processing" ) );
   out( ListRecord().c( " " ) );
   std::string base = base_name( s.input_name, s.sanner );
   if ( v3 ) {
      out( ListRecord().c( "basename = " ).c80( base ) );
   }
   if ( s.sanner ) {
      // scoalesce3 reads the MSMS files again on every call, and says so.
      out( ListRecord().c( "Number triangles = " ).i( long( s.tri.size() ) ) );
      out( ListRecord().i( long( s.edges.size() ) ).c( " unique edges found" ) );
      out( ListRecord().c( "Input file = " ).c80( base + ".msms.log" ) );
      out( ListRecord().c( "Areas: True, Poly = " ).r( s.true_area ).r( s.poly_area ) );
      out( ListRecord().c( "Volumes: True, Poly =" ).r( s.true_vol ).r( s.poly_vol ) );
   }
   std::vector<std::string> log;
   Result R = coalesce( s, edgfrac, &log );
   for ( auto& l : log ) {
      std::cout << l << '\n';
   }
   out( ListRecord().c( "  " ) );
   write_summary( std::cout, s, R.st, false, v3 );
   if ( write ) {
      nzero = write_result( base, s, R, edgfrac, v3, std::cout, outf );
   }
   return R.st.npoly;
}

void list_files() {
   for ( size_t i = 0; i + 1 < outf.size(); i += 2 ) {
      out( ListRecord().c( " " ) );
      out( ListRecord().c( " Output files written:" ) );
      out( ListRecord().c( "                      " ).c80( outf[i] ) );
      out( ListRecord().c( "                      " ).c80( outf[i + 1] ) );
      out( ListRecord().c( " " ) );
   }
}

// The target triangle counts, as rcoal computes and prints them.
std::vector<int> targets( int nmax, int nmin, int& n ) {
   int ndelta = 0;
   if ( nmax == nmin ) {
      n = 1;
   } else if ( n > 1 ) {
      ndelta = ( nmax - nmin ) / ( n - 1 );
   }
   std::vector<int> nt( n );
   out( ListRecord().c( " " ) );
   out( ListRecord().c( "Target values of n" ) );
   for ( int j = 0; j < n; ++j ) {
      nt[j] = nmin + j * ndelta;
      out( ListRecord().c( "             " ).i( nt[j] ) );
   }
   out( ListRecord().c( " " ) );
   return nt;
}

void usage() {
   if ( v3 ) {
      out( ListRecord().c( "Usage: rcoal -f filename -nmax val -nmin val " ) );
      out( ListRecord().c( "-n numb [-s]" ) );
      out( ListRecord().c( "Usage: rcoal -f filename -e val [-s]" ) );
   } else {
      out( ListRecord().c( "Usage: rcoal -f filename -nmax val -nmin val " ) );
      out( ListRecord().c( "-n numb " ) );
      out( ListRecord().c( "Usage: rcoal -f filename -e val " ) );
   }
   out( ListRecord().c( "Also: -search (find e for each target by bisection), -v3 (rcoal 3.0's messages)" ) );
}

}  // namespace

int main( int argc, char** argv ) {
   prog = argv[0];
   size_t slash = prog.find_last_of( "/\\" );
   if ( slash != std::string::npos ) {
      prog = prog.substr( slash + 1 );
   }
   if ( prog.compare( 0, 6, "rcoal3" ) == 0 ) {
      v3 = true;
   }
   if ( argc < 3 ) {
      usage();
      return 0;
   }
   std::string file;
   int nmax = -1, nmin = -1, n = -1;
   double edgefrac = -1.0;
   bool sanner = false, search = false;
   for ( int k = 1; k < argc; ++k ) {
      std::string a = argv[k];
      std::string next = k + 1 < argc ? argv[k + 1] : "";
      if ( a == "-f" ) {
         file = next;
      } else if ( a == "-nmax" ) {
         nmax = std::atoi( next.c_str() );
      } else if ( a == "-nmin" ) {
         nmin = std::atoi( next.c_str() );
      } else if ( a == "-n" ) {
         n = std::atoi( next.c_str() );
      } else if ( a == "-e" ) {
         edgefrac = std::atof( next.c_str() );
      } else if ( a == "-s" ) {
         sanner = true;
      } else if ( a == "-v3" ) {
         v3 = true;
      } else if ( a == "-search" ) {
         search = true;
      }
   }
   if ( sanner ) {
      v3 = true;
   }
   out( ListRecord().c( v3 ? "Rcoal Ver. 3.0" : "Rcoal Ver. 2.0" ) );
   out( ListRecord().c( "SFSU, Aragon Lab." ) );
   int nzero = 0;
   try {
      Surface s = sanner ? read_msms( file ) : read_c3p( file );
      int ntriang = int( s.tri.size() );
      if ( edgefrac > 0.0 ) {
         int n3 = scoalesce( s, edgefrac, true, nzero );
         if ( nzero == 0 ) {
            out( ListRecord().c( "Actual coalesce n" ).i( n3 ) );
         }
      } else if ( nmax == -1 || nmin <= 3 || n == 0 || n == -1 ) {
         out( ListRecord().c( "Check input parameters: n=0?, nmin <3,n,nmax missing?" ) );
      } else if ( nmax < nmin ) {
         out( ListRecord().c( "nmax is less than nmin" ) );
      } else if ( ntriang < nmax ) {
         out( ListRecord().c( "Nmax must be less than" ).i( ntriang + 1 ).c( "in this instance." ) );
      } else {
         std::vector<int> nt = targets( nmax, nmin, n );
         double p = 0, a = 0;
         if ( !search ) {
            // N(e) = ntriang / (1 + a e^p), fitted to runs at e = 0.7 and 1.0
            int n1 = scoalesce( s, 0.7, false, nzero );
            if ( v3 ) {
               out( ListRecord().c( "Just finished first scoalesce call" ) );
            }
            int n2 = scoalesce( s, 1.0, false, nzero );
            out( ListRecord().c( "initial n " ).i( n1 ).i( n2 ) );
            double y2 = std::log( double( ntriang ) / double( n2 ) - 1.0 );
            double y1 = std::log( double( ntriang ) / double( n1 ) - 1.0 );
            double x2 = std::log( 1.0 );
            double x1 = std::log( 0.7 );
            p = ( y2 - y1 ) / ( x2 - x1 );
            a = std::exp( y2 - p * x2 );
            out( ListRecord().c( " p = " ).r( p ) );
            out( ListRecord().c( " a = " ).r( a ) );
         }
         for ( int j = 0; j < n; ++j ) {
            double e = search ? find_edgfrac( s, nt[j] ) : 1.000 * std::pow( ( double( ntriang ) / double( nt[j] ) - 1.0 ) / a, 1 / p );
            out( ListRecord().c( " e = " ).r( e ) );
            int nn = scoalesce( s, e, true, nzero );
            if ( nzero > 0 ) {
               break;
            }
            out( ListRecord().c( "Target n         " ).i( nt[j] ) );
            out( ListRecord().c( "Actual coalesce n" ).i( nn ) );
            if ( !search ) {
               out( ListRecord().c( " " ) );
            }
         }
      }
   } catch ( const std::exception& e ) {
      std::cerr << prog << ": " << e.what() << '\n';
      return 2;
   }
   if ( nzero > 0 ) {
      for ( const std::string& f : outf ) {
         std::remove( f.c_str() );
      }
      std::cerr << " rcoal: a triangulation has triangles of zero area\n"
                << " (two vertices at the same position), which BEST\n"
                << " cannot use.  No triangulation written;\n"
                << " try a coarser input triangulation.\n"
                << "STOP 1\n";
      return 1;
   }
   list_files();
   return 0;
}
