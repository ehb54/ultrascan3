// sesmesh: triangulate the solvent-excluded surface of a set of spheres.
//
//   sesmesh [options] input.xyzr
//
// See README.md at the top of the repository.
#include <algorithm>
#include <cerrno>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "clean.h"
#include "io.h"
#include "mesh.h"
#include "parallel.h"
#include "ses.h"

using namespace sesmesh;

namespace {

const char* USAGE =
   "usage: sesmesh [options] input.xyzr\n"
   "\n"
   "Triangulates the solvent-excluded surface (SES) of the spheres in input.xyzr, one\n"
   "sphere per line as x y z r (further columns are ignored; # starts a comment).\n"
   "\n"
   "  -p probe     probe radius in A (default 1.5)\n"
   "  -g h         grid spacing in A (default 0.2); vertices lie on the exact surface\n"
   "  -n N         choose the grid spacing for a mesh of about N triangles (instead of -g)\n"
   "  -x h2,h3,..  also mesh at these spacings, report area and volume for each, and\n"
   "               extrapolate both to h = 0 by a least-squares fit a + b h^2\n"
   "  -r           raw marching-tetrahedra mesh: skip the clean-up of slender triangles\n"
   "  -c frac      clean-up: collapse edges shorter than frac times h (default 0.3)\n"
   "  -o base      output base name (default: the input's name without .xyzr)\n"
   "  -f list      output formats, a comma list of:\n"
   "                 msms  base.vert, base.face\n"
   "                 c3p   base.c3p, base.c3v as msroll writes them, for rcoal\n"
   "                 best  base.best, BEST's triangle format\n"
   "                 bod   base.bod, ZENO's format\n"
   "               or none (default msms,c3p)\n"
   "  -M mass      mass in Da for base.bod (no MASS line without it)\n"
   "  -T temp      temperature in C for base.bod (default 20)\n"
   "  -V visc      solvent viscosity in cP for base.bod (default 1.002)\n"
   "  -k           keep internal cavities (default: fill them, keeping the outer surface)\n"
   "  -t n         threads (default: all)\n"
   "  -v           report the time each stage takes\n"
   "\n"
   "base.c3v and the first line of base.best get the extrapolated area and volume when -x\n"
   "is given (from the meshes before clean-up), else the written mesh's own.\n";

double now() {
   return std::chrono::duration<double>( std::chrono::steady_clock::now().time_since_epoch() ).count();
}

[[noreturn]] void fail( const std::string& msg ) {
   std::fprintf( stderr, "sesmesh: %s\n", msg.c_str() );
   std::exit( 1 );
}

double to_double( const std::string& s, const char* what ) {
   char* end = nullptr;
   errno = 0;
   double v = std::strtod( s.c_str(), &end );
   if ( s.empty() || *end || errno ) {
      fail( std::string( "bad " ) + what + ": " + s );
   }
   return v;
}

std::vector<std::string> split( const std::string& s, char sep ) {
   std::vector<std::string> out;
   std::string cur;
   std::istringstream in( s );
   while ( std::getline( in, cur, sep ) ) {
      out.push_back( cur );
   }
   return out;
}

std::vector<Ball> read_xyzr( const std::string& path ) {
   std::ifstream in( path );
   if ( !in ) {
      fail( "cannot read " + path );
   }
   std::vector<Ball> atoms;
   std::string line;
   long lineno = 0;
   while ( std::getline( in, line ) ) {
      ++lineno;
      size_t hash = line.find( '#' );
      if ( hash != std::string::npos ) {
         line.erase( hash );
      }
      std::istringstream ls( line );
      std::string tok[4];
      int n = 0;
      while ( n < 4 && ls >> tok[n] ) {
         ++n;
      }
      if ( n == 0 ) {
         continue;
      }
      if ( n < 4 ) {
         fail( path + " line " + std::to_string( lineno ) + ": expected x y z r" );
      }
      double v[4];
      for ( int k = 0; k < 4; ++k ) {
         char* end = nullptr;
         v[k] = std::strtod( tok[k].c_str(), &end );
         if ( *end || !std::isfinite( v[k] ) ) {
            fail( path + " line " + std::to_string( lineno ) + ": not a number: " + tok[k] );
         }
      }
      atoms.push_back( { Vec3( v[0], v[1], v[2] ), v[3] } );
   }
   if ( atoms.empty() ) {
      fail( "no spheres in " + path );
   }
   return atoms;
}

void print_check( const char* label, double h, const MeshCheck& c, const BuildInfo* info, bool fill, bool verbose ) {
   std::printf( "%s h %.4f: ", label, h );
   if ( info ) {
      std::printf( "grid %dx%dx%d, %ld band bricks; ", info->nodes_per_axis[0], info->nodes_per_axis[1],
                   info->nodes_per_axis[2], info->band_bricks );
   }
   std::printf( "%ld vertices, %ld triangles, %zu component%s, %s, Euler %ld; area %.4f, volume %.4f; smallest "
                "angle %.2f deg, %ld (%.2f%%) under 10 deg, %ld (%.2f%%) under 20 deg\n",
                c.vertices, c.triangles, c.components.size(), c.components.size() == 1 ? "" : "s",
                c.closed_manifold() ? "closed manifold" : "NOT a closed manifold", c.euler, c.area, c.volume,
                c.min_angle, std::lround( c.below_10 * c.triangles ), 100 * c.below_10,
                std::lround( c.below_20 * c.triangles ), 100 * c.below_20 );
   if ( !c.closed_manifold() || c.degenerate ) {
      std::printf( "  open edges %ld, non-manifold edges %ld, misoriented edges %ld, non-manifold vertices %ld, "
                   "zero-area triangles %ld\n",
                   c.open_edges, c.nonmanifold_edges, c.misoriented_edges, c.nonmanifold_vertices, c.degenerate );
   }
   if ( c.components.size() > 1 ) {
      for ( size_t k = 0; k < c.components.size() && k < 8; ++k ) {
         std::printf( "  component %zu: %ld triangles, area %.4f, volume %.4f\n", k + 1, c.components[k].triangles,
                      c.components[k].area, c.components[k].volume );
      }
   }
   if ( info && info->cavity_nodes ) {
      std::printf( "  %ld grid nodes in internal cavities %s\n", info->cavity_nodes, fill ? "filled" : "kept" );
   }
   if ( info && verbose ) {
      std::printf( "  %ld node and %ld root evaluations; seconds: bricks %.2f, nodes %.2f, fill %.2f, vertices %.2f, "
                   "triangles %.2f\n",
                   info->node_evals, info->root_evals, info->seconds_classify, info->seconds_nodes, info->seconds_fill,
                   info->seconds_vertices, info->seconds_triangles );
   }
}

// Least-squares fit y = a + b x; returns a (and b).
double fit_line( const std::vector<double>& x, const std::vector<double>& y, double& b ) {
   const size_t n = x.size();
   double sx = 0, sy = 0, sxx = 0, sxy = 0;
   for ( size_t i = 0; i < n; ++i ) {
      sx += x[i];
      sy += y[i];
      sxx += x[i] * x[i];
      sxy += x[i] * y[i];
   }
   double d = n * sxx - sx * sx;
   b = d != 0 ? ( n * sxy - sx * sy ) / d : 0;
   return ( sy - b * sx ) / n;
}

}  // namespace

int main( int argc, char** argv ) {
   double probe = 1.5, h = 0.2, mass = 0, temp = 20, visc = 1.002, collapse = -1;
   long target = 0;
   std::vector<double> extra;
   std::string input, base;
   bool want_msms = true, want_c3p = true, want_best = false, want_bod = false;
   bool fill = true, clean = true, verbose = false, h_given = false;
   int threads = 0;

   for ( int a = 1; a < argc; ++a ) {
      std::string o = argv[a];
      auto value = [&]() -> std::string {
         if ( a + 1 >= argc ) {
            fail( "option " + o + " needs a value" );
         }
         return argv[++a];
      };
      if ( o == "-h" || o == "--help" ) {
         std::fputs( USAGE, stdout );
         return 0;
      } else if ( o == "-p" ) {
         probe = to_double( value(), "probe radius" );
      } else if ( o == "-g" ) {
         h = to_double( value(), "grid spacing" );
         h_given = true;
      } else if ( o == "-n" ) {
         target = long( to_double( value(), "triangle count" ) );
         if ( target < 100 ) {
            fail( "the target triangle count must be at least 100" );
         }
      } else if ( o == "-x" ) {
         for ( const std::string& s : split( value(), ',' ) ) {
            extra.push_back( to_double( s, "grid spacing" ) );
         }
      } else if ( o == "-r" ) {
         clean = false;
      } else if ( o == "-c" ) {
         collapse = to_double( value(), "collapse length" );
      } else if ( o == "-o" ) {
         base = value();
      } else if ( o == "-f" ) {
         want_msms = want_c3p = want_best = want_bod = false;
         for ( const std::string& s : split( value(), ',' ) ) {
            if ( s == "msms" ) {
               want_msms = true;
            } else if ( s == "c3p" ) {
               want_c3p = true;
            } else if ( s == "best" ) {
               want_best = true;
            } else if ( s == "bod" ) {
               want_bod = true;
            } else if ( s != "none" ) {
               fail( "unknown format " + s );
            }
         }
      } else if ( o == "-M" ) {
         mass = to_double( value(), "mass" );
      } else if ( o == "-T" ) {
         temp = to_double( value(), "temperature" );
      } else if ( o == "-V" ) {
         visc = to_double( value(), "viscosity" );
      } else if ( o == "-k" ) {
         fill = false;
      } else if ( o == "-t" ) {
         threads = int( to_double( value(), "thread count" ) );
      } else if ( o == "-v" ) {
         verbose = true;
      } else if ( !o.empty() && o[0] == '-' ) {
         fail( "unknown option " + o + "\n" + USAGE );
      } else if ( input.empty() ) {
         input = o;
      } else {
         fail( "more than one input file\n" + std::string( USAGE ) );
      }
   }
   if ( input.empty() ) {
      std::fputs( USAGE, stderr );
      return 1;
   }
   if ( !( probe >= 0 ) ) {
      fail( "the probe radius must be zero or positive" );
   }
   if ( !( h > 0 ) ) {
      fail( "the grid spacing must be positive" );
   }
   for ( double x : extra ) {
      if ( !( x > 0 ) ) {
         fail( "grid spacings must be positive" );
      }
   }
   std::string name = input.substr( input.find_last_of( '/' ) == std::string::npos ? 0 : input.find_last_of( '/' ) + 1 );
   if ( name.size() > 5 && name.compare( name.size() - 5, 5, ".xyzr" ) == 0 ) {
      name.erase( name.size() - 5 );
   }
   if ( base.empty() ) {
      base = name;
   }
   if ( threads > 0 ) {
      set_threads( threads );
   }
   threads = sesmesh::threads();

   const double t_start = now();
   std::vector<Ball> atoms = read_xyzr( input );
   try {
      SesModel model( atoms, probe );
      const double t_model = now();
      std::printf( "sesmesh: %s, %d spheres (%d inside others, %d buried), probe %.3f A; %d exposed circles, "
                   "%d exposed arcs; %d threads\n",
                   input.c_str(), model.atoms(), model.atoms() - model.spheres_kept(), model.spheres_buried(), probe,
                   model.circles_exposed(), model.arcs_exposed(), threads );
      CleanOptions copt;
      if ( collapse >= 0 ) {
         copt.collapse = collapse;
      }
      auto build = [&]( double hh, Mesh& m, BuildInfo& info ) {
         MeshOptions o;
         o.h = hh;
         o.fill_cavities = fill;
         build_mesh( model, o, m, info );
      };

      // The spacing for a target triangle count: the count goes as 1/h^2.
      if ( target > 0 ) {
         double hh = h_given ? h : 1.0, best_h = hh;
         long best_diff = -1;
         for ( int it = 0; it < 8; ++it ) {
            Mesh m;
            BuildInfo bi;
            build( hh, m, bi );
            if ( clean ) {
               CleanInfo ci;
               clean_mesh( model, hh, copt, m, ci );
            }
            long nt = long( m.tri.size() ), diff = std::labs( nt - target );
            std::printf( "target %ld triangles: h %.4f gives %ld\n", target, hh, nt );
            if ( best_diff < 0 || diff < best_diff ) {
               best_diff = diff;
               best_h = hh;
            }
            if ( diff <= 0.02 * target || nt == 0 ) {
               break;
            }
            hh *= std::sqrt( double( nt ) / double( target ) );
         }
         h = best_h;
      }

      std::vector<double> h2, areas, vols;
      for ( double hx : extra ) {
         Mesh m;
         BuildInfo bi;
         build( hx, m, bi );
         MeshCheck c;
         if ( verbose ) {
            c = check_mesh( m );
            print_check( "raw", hx, c, &bi, fill, verbose );
         } else {
            mesh_area_volume( m, c.area, c.volume );
            std::printf( "raw h %.4f: %ld triangles; area %.4f, volume %.4f\n", hx, long( m.tri.size() ), c.area,
                         c.volume );
         }
         h2.push_back( hx * hx );
         areas.push_back( c.area );
         vols.push_back( c.volume );
      }
      Mesh mesh;
      BuildInfo info;
      build( h, mesh, info );
      // The raw mesh is checked in full when it is the one written, or with -v; else only its area and
      // volume are needed (for -x).
      double tc = now();
      MeshCheck chk;
      if ( !clean || verbose ) {
         chk = check_mesh( mesh );
         print_check( "raw", h, chk, &info, fill, verbose );
      } else {
         mesh_area_volume( mesh, chk.area, chk.volume );
         std::printf( "raw h %.4f: grid %dx%dx%d, %ld band bricks; %ld vertices, %ld triangles; area %.4f, volume %.4f\n",
                      h, info.nodes_per_axis[0], info.nodes_per_axis[1], info.nodes_per_axis[2], info.band_bricks,
                      long( mesh.x.size() ), long( mesh.tri.size() ), chk.area, chk.volume );
      }
      double t_check = now() - tc;
      h2.push_back( h * h );
      areas.push_back( chk.area );
      vols.push_back( chk.volume );
      if ( clean ) {
         CleanInfo ci;
         clean_mesh( model, h, copt, mesh, ci );
         tc = now();
         chk = check_mesh( mesh );
         t_check += now() - tc;
         std::printf( "clean-up: %ld edge collapses, %ld flips, %ld vertex moves; %ld collapses refused by the link "
                      "condition, %ld by the shape test (%.2f s)\n",
                      ci.collapses, ci.flips, ci.smoothed, ci.rejected_link, ci.rejected_shape, ci.seconds );
         if ( verbose ) {
            std::printf( "  clean-up seconds: collapses %.2f, flips %.2f, smoothing %.2f; %d passes; %ld collapse or flip "
                         "tries left to the serial pass\n",
                         ci.seconds_collapse, ci.seconds_flip, ci.seconds_smooth, ci.passes, ci.deferred );
         }
         print_check( "cleaned", h, chk, nullptr, fill, verbose );
      }

      // A coarse grid can miss a narrow channel or pinch a thin neck: compare the topology with a
      // finer grid's when the spacing was chosen for a triangle count.
      if ( target > 0 && h > 0.5 ) {
         double href = std::min( 0.5, h / 2 );
         Mesh m;
         BuildInfo bi;
         build( href, m, bi );
         MeshCheck c = check_mesh( m );
         if ( c.euler != chk.euler || c.components.size() != chk.components.size() ) {
            std::printf( "WARNING: at h %.4f the surface has %zu component(s) and Euler characteristic %ld, but at h "
                         "%.4f %zu and %ld: the coarse grid changes the topology; consider a larger -n\n",
                         h, chk.components.size(), chk.euler, href, c.components.size(), c.euler );
         } else {
            std::printf( "topology checked at h %.4f: the same (%zu component(s), Euler %ld)\n", href,
                         c.components.size(), c.euler );
         }
      }

      double area = chk.area, volume = chk.volume;
      if ( h2.size() > 1 ) {
         double ba, bv;
         area = fit_line( h2, areas, ba );
         volume = fit_line( h2, vols, bv );
         std::printf( "extrapolated to h = 0 (a + b h^2, %zu spacings): area %.4f (b %.4f), volume %.4f (b %.4f)\n",
                      h2.size(), area, ba, volume, bv );
      }
      const double t_write0 = now();
      if ( want_msms ) {
         write_msms( base, name, mesh, model.atoms(), probe, h );
         std::printf( "wrote %s.vert, %s.face\n", base.c_str(), base.c_str() );
      }
      if ( want_c3p ) {
         write_c3p( base, name, mesh, model.atoms(), probe, area, volume );
         std::printf( "wrote %s.c3p, %s.c3v\n", base.c_str(), base.c_str() );
      }
      if ( want_best ) {
         write_best( base, mesh, area, volume );
         std::printf( "wrote %s.best\n", base.c_str() );
      }
      if ( want_bod ) {
         write_bod( base, mesh, mass, temp, visc );
         std::printf( "wrote %s.bod\n", base.c_str() );
      }
      if ( verbose ) {
         std::printf( "seconds: reading and setup %.2f, mesh checks %.2f, writing %.2f, total %.2f\n", t_model - t_start,
                      t_check, now() - t_write0, now() - t_start );
      }
      if ( !chk.closed_manifold() ) {
         return 2;
      }
   } catch ( const std::exception& e ) {
      fail( e.what() );
   }
   return 0;
}
