// Unit tests for shell reduction (grpy_exposure.hpp + grpy_shell.hpp).
//
// The central property under test is HONESTY: the reported error bar must bound the true
// error, measured against an independently computed unreduced solve. A bar that
// understates is worse than no bar at all, so that check is the one that matters.
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>
#include "grpy_shell.hpp"

using namespace grpy;

// A compact, densely packed blob: enough interior beads to have something to screen.
static std::vector<Bead> blob( int per_side, double spacing, double radius ) {
   std::vector<Bead> b;
   for ( int i = 0; i < per_side; ++i ) {
      for ( int j = 0; j < per_side; ++j ) {
         for ( int k = 0; k < per_side; ++k ) {
            b.push_back( {i * spacing, j * spacing, k * spacing, radius, 1000.0} );
         }
      }
   }
   return b;
}

static int chk( const char* what, bool ok ) {
   std::printf( "  %-64s %s\n", what, ok ? "OK" : "FAIL" );
   return ok ? 0 : 1;
}

// The ladder reaches its solver through grpy::SolveFn (issue 1012), so these tests drive it
// with an ANALYTIC model instead of the real solver -- which now lives in its own program
// (ehb54/grpy-cpp) and is not linked here.
//
// This is a stronger test than driving GRPY would be, not a weaker one. Each observable
// approaches the full-model value as the kept fraction grows,
//
//     mu(n) = mu_full * ( 1 + c * ( (N/n)^k - 1 ) ),
//
// so the TRUE error of any rung is known exactly and the reported bar can be checked for
// actually bounding it, rather than being compared against another estimate. The rates
// differ per observable, with intrinsic viscosity slowest, which is the ordering measured
// against real GRPY and is what drives the viscosity-reliability logic.
//
// The stage names are the ones the solver emits, since the ladder wraps them.
struct Analytic {
   int n_full;

   static double converge( double full, double c, double k, double frac ) {
      return full * ( 1.0 + c * ( std::pow( 1.0 / frac, k ) - 1.0 ) );
   }

   Results operator()( const std::vector<Bead>& b, const PhysParams& p,
                      const ProgressFn& prog ) const {
      const double frac = n_full > 0 ? (double) b.size() / (double) n_full : 1.0;
      if ( prog ) {
         prog( 20, "BUILDING MOBILITY MATRIX" );
         prog( 60, "INVERTING MATRICES" );
         prog( 100, "SOLVING" );
      }
      // MW and Rg are derived the way the real solver derives them -- summed over the
      // beads it was given -- so that the ladder's PINNING of both to full-model values
      // is what the corresponding tests actually exercise. A model that simply echoed
      // them back could not tell a pinned value from an unpinned one.
      double summed_mw = 0, cx = 0, cy = 0, cz = 0, vol = 0;
      for ( const Bead& q : b ) {
         const double v = q.radius * q.radius * q.radius;
         summed_mw += q.mw;
         cx += q.x * v; cy += q.y * v; cz += q.z * v; vol += v;
      }
      Results r;
      if ( vol > 0 ) {
         cx /= vol; cy /= vol; cz /= vol;
         for ( const Bead& q : b ) {
            const double v = q.radius * q.radius * q.radius;
            const double dx = q.x - cx, dy = q.y - cy, dz = q.z - cz;
            r.rg2 += ( 3.0 / 5.0 * q.radius * q.radius + dx*dx + dy*dy + dz*dz ) * v / vol;
         }
      }
      r.translational_diffusion_centre = converge( 6.0e-7, 0.02, 1.8, frac );
      r.translational_diffusion        = r.translational_diffusion_centre;
      r.rotational_diffusion           = converge( 3.5e+6, 0.02, 1.6, frac );
      r.sedimentation                  = converge( 2.5e-2, 0.02, 1.8, frac );
      r.intrinsic_viscosity_high       = converge( 2.6e+2, 0.02, 1.1, frac );
      r.intrinsic_viscosity_zero       = converge( 2.8e+2, 0.02, 1.1, frac );
      r.mass                           = p.mw > 0 ? p.mw : summed_mw;
      char buf[ 256 ];
      std::snprintf( buf, sizeof buf,
                    " GRPY program\n\n Translational diffusion coefficient: %11.3E [cm^2/s]\n",
                    r.translational_diffusion_centre );
      r.report = buf;
      return r;
   }
};

// A solver for a model of `n_full` beads.
static SolveFn analytic( int n_full ) { return Analytic{n_full}; }

int main() {
   int fails = 0;
   PhysParams phys;                      // defaults = -u mode (20 C, eta 0.01, rho 1)

   // ---- exposure ------------------------------------------------------------
   {
      auto b = blob( 5, 3.0, 2.0 );       // 125 beads, 27 of them strictly interior
      const std::vector<Bead>& c = b;                    // exposure takes grpy::Bead now
      auto ex = shell::exposure( c, (int) c.size(), 64, 1.4 );
      int buried = 0, exposed = 0;
      for ( double e : ex ) { if ( e <= 0.0 ) ++buried; else ++exposed; }
      fails += chk( "some beads are fully buried", buried > 0 );
      fails += chk( "most beads remain exposed", exposed > buried );
      // A corner bead must be more exposed than the centre bead.
      fails += chk( "corner more exposed than centre", ex[ 0 ] > ex[ c.size() / 2 ] );
   }

   // ---- reduce_top_frac -----------------------------------------------------
   {
      auto b = blob( 4, 3.0, 2.0 );
      const std::vector<Bead>& c = b;                    // exposure takes grpy::Bead now
      auto ex = shell::exposure( c, (int) c.size(), 64, 1.4 );
      auto half = shell::reduce_top_frac( c, ex, 0.5 );
      fails += chk( "keeps ceil(frac*N)", (int) half.size() == (int) std::ceil( 0.5 * c.size() ) );
      fails += chk( "frac>=1 returns everything", shell::reduce_top_frac( c, ex, 1.0 ).size() == c.size() );
      // Deterministic: same inputs, same subset.
      auto again = shell::reduce_top_frac( c, ex, 0.5 );
      bool same = true;
      for ( size_t i = 0; i < half.size(); ++i ) {
         if ( half[ i ].x != again[ i ].x || half[ i ].y != again[ i ].y || half[ i ].z != again[ i ].z ) {
            same = false;
         }
      }
      fails += chk( "selection is deterministic", same );
   }

   // ---- disabled by default reproduces the plain Solver exactly ---------------
   {
      auto b = blob( 4, 3.0, 2.0 );
      Results a = analytic( (int) b.size() )( b, phys, {} );
      ShellSolver sh( analytic( (int) b.size() ), {} );                       // ShellOptions.enabled = false
      ShellReport rep;
      Results c = sh.run( b, phys, rep );
      fails += chk( "disabled => identical Dt", a.translational_diffusion_centre == c.translational_diffusion_centre );
      fails += chk( "disabled => identical eta", a.intrinsic_viscosity_high == c.intrinsic_viscosity_high );
      fails += chk( "disabled => report byte-identical", a.report == c.report );
      fails += chk( "disabled => viscosity is usable", !rep.viscosity_unreliable );
      fails += chk( "disabled => not attempted", !rep.attempted );
   }

   // ---- MW and Rg are pinned to the full model --------------------------------
   // If Solver re-derived them from a reduced bead list, MW would fall with the dropped
   // beads (corrupting sedimentation and both viscosities) and Rg would rise.
   {
      auto b = blob( 5, 3.0, 2.0 );
      Results full = analytic( (int) b.size() )( b, phys, {} );
      ShellOptions so; so.enabled = true; so.tol = 1e-9;   // force deep reduction attempts
      so.ladder = {0.25, 0.5};
      ShellSolver sh( analytic( (int) b.size() ), so );
      ShellReport rep;
      Results red = sh.run( b, phys, rep );
      fails += chk( "reduction actually happened", rep.n_used < rep.n_full );
      fails += chk( "MW pinned to full model", std::fabs( red.mass - full.mass ) < 1e-9 * full.mass );
      fails += chk( "Rg2 pinned to full model", std::fabs( red.rg2 - full.rg2 ) < 1e-9 * full.rg2 );
   }

   // ---- HONESTY: the reported bar bounds the true error ------------------------
   {
      auto b = blob( 6, 3.0, 2.0 );                        // 216 beads
      Results full = analytic( (int) b.size() )( b, phys, {} );

      for ( double tol : {5e-2, 2e-2, 1e-2} ) {
         ShellOptions so;
         so.enabled = true; so.tol = tol;
         so.require = {Obs::Dt, Obs::Dr, Obs::Sedimentation, Obs::EtaInf, Obs::EtaZero};
         so.ladder = {0.125, 0.25, 0.5};                // never the full model
         ShellSolver sh( analytic( (int) b.size() ), so );
         ShellReport rep;
         Results r = sh.run( b, phys, rep );
         for ( size_t m = 0; m < rep.require.size(); ++m ) {
            double ref = obs_value( full, rep.require[ m ] );
            double got = obs_value( r, rep.require[ m ] );
            double true_err = std::fabs( got - ref ) / std::fabs( ref );
            char msg[ 200 ];
            std::snprintf( msg, sizeof( msg ), "tol=%.3g %s: bar %.4g%% >= true %.4g%%",
                          tol, obs_name( rep.require[ m ] ), 100 * rep.err_est[ m ], 100 * true_err );
            fails += chk( msg, rep.err_est[ m ] >= true_err );
         }
      }
   }

   // ---- the report gives BOTH values, and says which one the scalars are -------
   //
   // The estimate is the distance between the computed value and the extrapolated one, so
   // a report showing only one of them cannot tell the reader what the estimate applies
   // to. It applies to the computed value -- the finest rung -- which is also what
   // Results carries; rep.reported must therefore agree with Results exactly, or the
   // report and the returned scalars would describe different answers.
   {
      auto b = blob( 6, 3.0, 2.0 );
      ShellOptions so;
      so.enabled = true; so.tol = 1e-2;
      so.require = {Obs::Dt, Obs::Dr, Obs::EtaInf};
      so.ladder = {0.125, 0.25, 0.5};                    // never the full model
      ShellSolver sh( analytic( (int) b.size() ), so );
      ShellReport rep;
      Results r = sh.run( b, phys, rep );

      fails += chk( "reported[] is parallel to require[]", rep.reported.size() == rep.require.size() );
      for ( size_t m = 0; m < rep.require.size() && m < rep.reported.size(); ++m ) {
         char msg[ 200 ];
         std::snprintf( msg, sizeof( msg ), "%s: reported[] == the returned scalar",
                       obs_name( rep.require[ m ] ) );
         fails += chk( msg, rep.reported[ m ] == obs_value( r, rep.require[ m ] ) );
      }
      fails += chk( "report carries both columns",
                   r.report.find( "computed" ) != std::string::npos &&
                   r.report.find( "extrapolated" ) != std::string::npos );
      fails += chk( "report says which column the results are",
                   r.report.find( "are the COMPUTED column" ) != std::string::npos );
      // The two columns must actually differ somewhere, or the test would pass on a
      // report that silently printed the same number twice.
      bool differ = false;
      for ( size_t m = 0; m < rep.reported.size() && m < rep.extrapolated.size(); ++m ) {
         if ( rep.reported[ m ] != rep.extrapolated[ m ] ) {
            differ = true;
         }
      }
      fails += chk( "computed and extrapolated are distinct answers", differ );
   }

   // ---- the report carries the per-rung sequence and the estimator's provenance -
   //
   // Both exist so that a validation run can be audited without re-solving: `values`
   // lets any variant of the estimator be recomputed from the stored ladder, and `prov`
   // says which mechanism produced each estimate. Reconstructing either from a separate
   // harness is how an earlier round of benchmarks ended up unattributable.
   {
      auto b = blob( 6, 3.0, 2.0 );
      ShellOptions so;
      so.enabled = true; so.tol = 1e-2;
      so.require = {Obs::Dt, Obs::Dr, Obs::EtaInf};
      so.ladder = {0.125, 0.25, 0.5};
      ShellSolver sh( analytic( (int) b.size() ), so );
      ShellReport rep;
      Results r = sh.run( b, phys, rep );

      fails += chk( "values has one row per rung", rep.values.size() == rep.ns.size() );
      bool widths_ok = !rep.values.empty();
      for ( auto& row : rep.values ) {
         if ( row.size() != rep.require.size() ) {
            widths_ok = false;
         }
      }
      fails += chk( "each row has one value per observable", widths_ok );
      fails += chk( "the last row IS the reported value",
                   !rep.values.empty() && rep.values.back() == rep.reported );
      // The sequence must be the ladder actually solved, not a copy of one rung.
      bool moved = rep.values.size() > 1 && rep.values.front()[ 0 ] != rep.values.back()[ 0 ];
      fails += chk( "values differ between rungs", moved );

      fails += chk( "prov is parallel to require", rep.prov.size() == rep.require.size() );
      bool consistent = true;
      for ( size_t m = 0; m < rep.prov.size(); ++m ) {
         const auto& pv = rep.prov[ m ];
         // extrapolated and declined are mutually exclusive, and k_obs is set iff
         // extrapolation succeeded -- otherwise the report contradicts itself.
         if ( pv.extrapolated == ( pv.declined != nullptr ) ) {
            consistent = false;
         }
         if ( pv.extrapolated != ( rep.k_obs[ m ] != 0.0 ) ) {
            consistent = false;
         }
         if ( !pv.extrapolated && ( pv.clamped_low || pv.clamped_high || pv.floored ) ) {
            consistent = false;
         }
         if ( pv.clamped_low && pv.clamped_high ) {
            consistent = false;
         }
      }
      fails += chk( "provenance is self-consistent", consistent );
   }

   // ---- a two-rung ladder declines to extrapolate, and says so -----------------
   {
      auto b = blob( 5, 3.0, 2.0 );
      ShellOptions so;
      so.enabled = true; so.tol = 1e-9;                  // unsatisfiable: use both rungs
      so.require = {Obs::Dt};
      so.ladder = {0.25, 0.5};                           // only two rungs: Richardson needs 3
      ShellSolver sh( analytic( (int) b.size() ), so );
      ShellReport rep;
      Results r = sh.run( b, phys, rep );
      fails += chk( "two rungs => not extrapolated", !rep.prov.empty() && !rep.prov[ 0 ].extrapolated );
      fails += chk( "two rungs => a reason is given",
                   !rep.prov.empty() && rep.prov[ 0 ].declined != nullptr );
      fails += chk( "two rungs => k_obs stays zero", rep.k_obs.size() == 1 && rep.k_obs[ 0 ] == 0.0 );
   }

   // ---- full model reached AND tolerance met => still exact ---------------------
   //
   // Regression: the tolerance test used to precede the full-model test, so when the last
   // rung was the whole model and also satisfied the tolerance, the run exited as merely
   // "converged" and kept the inter-rung gap as its estimate -- 0.159% on an answer whose
   // true error was zero, with `unreduced` left false. A satisfiable tolerance is required
   // to catch it: with an unsatisfiable one the tolerance branch can never win the race,
   // which is why the older exhaustion test above passed throughout.
   {
      auto b = blob( 6, 3.0, 2.0 );                        // 216 beads
      Results full = analytic( (int) b.size() )( b, phys, {} );
      ShellOptions so;
      so.enabled = true; so.tol = 5e-3;                  // loose enough to be met AT the full rung
      so.require = {Obs::Dt, Obs::Dr, Obs::Sedimentation};
      so.ladder = {0.0625, 0.125, 0.25, 0.5, 1.0};       // ends at the full model
      ShellSolver sh( analytic( (int) b.size() ), so );
      ShellReport rep;
      Results r = sh.run( b, phys, rep );

      if ( rep.n_used == rep.n_full ) {                    // only meaningful if it got there
         fails += chk( "full rung => flagged unreduced", rep.unreduced );
         fails += chk( "full rung => zero error estimate", rep.err_max == 0.0 );
         bool all_zero = true;
         for ( double e : rep.err_est ) {
            if ( e != 0.0 ) {
               all_zero = false;
            }
         }
         fails += chk( "full rung => every per-observable estimate is zero", all_zero );
         fails += chk( "full rung => result is the exact one",
                      r.translational_diffusion_centre == full.translational_diffusion_centre );
         fails += chk( "full rung => viscosity not flagged unreliable", !rep.viscosity_unreliable );
      } else {
         fails += chk( "full rung => reached the full model (test precondition)", false );
      }
   }

   // ---- selection is invariant to input bead ORDER ----------------------------
   //
   // Exposure ties are large (quantised to K sample points) and the rung boundary usually
   // falls inside one, so whatever breaks ties decides much of the selection. Breaking
   // them by bead index made the retained subset depend on the order the file was written
   // in: measured at 20 distinct subsets over 20 permutations of one real model. Ties now
   // break on geometry, which is a property of the model, so permuting the input must not
   // change which beads are kept.
   {
      auto b = blob( 5, 3.0, 2.0 );
      const std::vector<Bead>& c = b;                    // exposure takes grpy::Bead now
      auto ex = shell::exposure( c, (int) c.size(), 64, 1.4 );
      auto ref = shell::reduce_top_frac_idx( c, ex, 0.25 );
      std::vector<std::array<double,3>> ref_xyz;
      for ( size_t i : ref ) {
         ref_xyz.push_back( {c[ i ].x, c[ i ].y, c[ i ].z} );
      }
      std::sort( ref_xyz.begin(), ref_xyz.end() );

      bool invariant = true;
      unsigned st = 12345;
      for ( int trial = 0; trial < 8 && invariant; ++trial ) {
         std::vector<size_t> perm( c.size() );
         for ( size_t i = 0; i < perm.size(); ++i ) {
            perm[ i ] = i;
         }
         for ( size_t i = perm.size(); i > 1; --i ) {       // deterministic shuffle
            st = st * 1103515245u + 12345u;
            std::swap( perm[ i - 1 ], perm[ st % i ] );
         }
         std::vector<Bead> pc( c.size() );
         for ( size_t i = 0; i < c.size(); ++i ) {
            pc[ i ] = c[ perm[ i ] ];
         }
         auto pex = shell::exposure( pc, (int) pc.size(), 64, 1.4 );
         auto got = shell::reduce_top_frac_idx( pc, pex, 0.25 );
         std::vector<std::array<double,3>> got_xyz;
         for ( size_t i : got ) {
            got_xyz.push_back( {pc[ i ].x, pc[ i ].y, pc[ i ].z} );
         }
         std::sort( got_xyz.begin(), got_xyz.end() );
         if ( got_xyz != ref_xyz ) {
            invariant = false;
         }
      }
      fails += chk( "selection is invariant to input bead order", invariant );
   }

   // ---- viscosity is flagged unreliable when not required ----------------------
   {
      auto b = blob( 5, 3.0, 2.0 );
      ShellOptions so;
      so.enabled = true; so.tol = 5e-2;
      so.require = {Obs::Dt, Obs::Dr};                   // viscosity deliberately absent
      so.ladder = {0.125, 0.25, 0.5};
      ShellSolver sh( analytic( (int) b.size() ), so );
      ShellReport rep;
      Results r = sh.run( b, phys, rep );
      fails += chk( "viscosity flagged unreliable when not required", rep.viscosity_unreliable );
      fails += chk( "report warns about viscosity", r.report.find( "UNRELIABLE" ) != std::string::npos );
      fails += chk( "report records the reduction", r.report.find( "SHELL REDUCTION" ) != std::string::npos );
      fails += chk( "viscosity value is still present in the report for the record", r.intrinsic_viscosity_high != 0.0 );
   }

   // ---- ladder exhausting onto the full model reports an EXACT result ----------
   // Caught end-to-end: a 246-bead model at tol=2% could not converge, ran out onto the
   // unreduced rung, and then reported the stale inter-rung gap (1.68%) as the error of
   // an answer that was in fact exact -- and wrongly flagged viscosity unreliable.
   {
      auto b = blob( 4, 3.0, 2.0 );
      Results full = analytic( (int) b.size() )( b, phys, {} );
      ShellOptions so;
      so.enabled = true; so.tol = 1e-12;              // unsatisfiable: force exhaustion
      so.require = {Obs::Dt, Obs::Dr};                // viscosity deliberately absent
      so.ladder = {0.25, 0.5, 1.0};                   // last rung IS the full model
      ShellSolver sh( analytic( (int) b.size() ), so );
      ShellReport rep;
      Results r = sh.run( b, phys, rep );
      fails += chk( "exhausted ladder flags unreduced", rep.unreduced );
      fails += chk( "unreduced uses every bead", rep.n_used == rep.n_full );
      fails += chk( "unreduced reports zero error", rep.err_max == 0.0 );
      fails += chk( "unreduced matches the plain Solver",
                   r.translational_diffusion_centre == full.translational_diffusion_centre );
      fails += chk( "unreduced => viscosity usable even when not required",
                   !rep.viscosity_unreliable );
      fails += chk( "unreduced report says the result is exact",
                   r.report.find( "exact" ) != std::string::npos );
   }

   // ---- memory cap (issue 987 interaction) ------------------------------------
   // The caller's pre-flight guard sizes the mobility matrix from the FULL bead count and
   // refuses when it exceeds RAM. That would turn away exactly the large models shell
   // reduction exists to make feasible, since the ladder usually stops well short of the
   // full model. ShellOptions::max_beads lets the caller cap the ladder instead, so an
   // oversized model yields the largest rung that fits WITH its bar rather than nothing.
   {
      auto b = blob( 6, 3.0, 2.0 );                     // 216 beads

      // A cap above the full model must change nothing at all.
      {
         ShellOptions so;
         so.enabled = true; so.tol = 1e-12;          // unsatisfiable: force exhaustion
         so.require = {Obs::Dt, Obs::Dr};
         so.ladder = {0.25, 0.5, 1.0};
         so.max_beads = 100000;
         ShellSolver sh( analytic( (int) b.size() ), so );
         ShellReport rep;
         Results r = sh.run( b, phys, rep );
         fails += chk( "slack cap does not bind", !rep.mem_capped );
         fails += chk( "slack cap still reaches the full model", rep.unreduced );
         fails += chk( "slack cap still reports an exact result", rep.err_max == 0.0 );
         (void) r;
      }

      // A cap that binds must stop the ladder, say so, and NOT claim convergence.
      {
         const int cap = 60;                         // admits 0.125 (27) and 0.25 (54)
         ShellOptions so;
         so.enabled = true; so.tol = 1e-12;          // unsatisfiable: only the cap can stop it
         so.require = {Obs::Dt, Obs::Dr};
         so.ladder = {0.125, 0.25, 0.5, 1.0};
         so.max_beads = cap;
         ShellSolver sh( analytic( (int) b.size() ), so );
         ShellReport rep;
         Results r = sh.run( b, phys, rep );
         fails += chk( "binding cap is reported", rep.mem_capped );
         fails += chk( "binding cap never exceeds the budget", rep.n_used <= cap );
         fails += chk( "binding cap did not reach the full model", !rep.unreduced );
         fails += chk( "binding cap does not claim convergence", !rep.converged );
         fails += chk( "binding cap still produced a result", rep.levels >= 1 && rep.n_used > 0 );
         fails += chk( "binding cap reports a finite error bar",
                      rep.err_max > 0.0 && std::isfinite( rep.err_max ) );
         fails += chk( "binding cap explains itself in the report",
                      r.report.find( "stopped by the available memory" ) != std::string::npos );
      }

      // A cap below even the smallest rung yields no result at all -- the caller must
      // detect this (levels == 0) and fall back to refusing, rather than report zeros.
      {
         ShellOptions so;
         so.enabled = true; so.tol = 1e-12;
         so.require = {Obs::Dt, Obs::Dr};
         so.ladder = {0.125, 0.25, 0.5, 1.0};
         so.max_beads = 4;                           // smaller than 0.125 * 216
         ShellSolver sh( analytic( (int) b.size() ), so );
         ShellReport rep;
         sh.run( b, phys, rep );
         fails += chk( "impossible cap runs no rung", rep.levels == 0 && rep.mem_capped );
      }
   }

   // ---- progress is ladder-wide, not per-rung ---------------------------------
   // Each rung is its own solve sweeping 0..100%, so forwarding progress raw made the
   // bar restart once per rung and the stage text never said which rung was running.
   {
      auto b = blob( 5, 3.0, 2.0 );
      ShellOptions so;
      so.enabled = true; so.tol = 1e-12;              // unsatisfiable: run the whole ladder
      so.require = {Obs::Dt, Obs::Dr};
      so.ladder = {0.125, 0.25, 0.5, 1.0};
      ShellSolver sh( analytic( (int) b.size() ), so );
      ShellReport rep;

      std::vector<int> pcts;
      std::vector<std::string> stages;
      sh.run( b, phys, rep, [ & ]( int pct, const char* stage ) {
          pcts.push_back( pct );
          stages.push_back( stage ? stage : "" );
      } );

      bool monotone = true, in_range = true, labelled = !stages.empty();
      for ( size_t i = 0; i < pcts.size(); ++i ) {
         if ( i && pcts[ i ] < pcts[ i - 1 ] ) {
            monotone = false;
         }
         if ( pcts[ i ] < 0 || pcts[ i ] > 100 ) {
            in_range = false;
         }
         if ( stages[ i ].find( "rung " ) == std::string::npos ) {
            labelled = false;
         }
      }
      fails += chk( "progress was reported at all", !pcts.empty() );
      fails += chk( "progress never goes backwards across rungs", monotone );
      fails += chk( "progress stays within 0..100", in_range );
      fails += chk( "every stage names its rung", labelled );
      fails += chk( "stage names the bead count",
                   !stages.empty() && stages.back().find( " beads: " ) != std::string::npos );
      // Several rungs ran, but the bar restarted zero times: proof of the remap.
      int restarts = 0;
      for ( size_t i = 1; i < pcts.size(); ++i ) {
         if ( pcts[ i ] < pcts[ i - 1 ] ) {
            ++restarts;
         }
      }
      fails += chk( "multiple rungs ran", rep.levels > 1 );
      fails += chk( "bar restarted zero times despite multiple rungs", restarts == 0 );
   }

   // ---- disabled shell reduction leaves progress untouched ---------------------
   {
      auto b = blob( 4, 3.0, 2.0 );
      ShellSolver sh( analytic( (int) b.size() ), {} );                    // enabled = false
      ShellReport rep;
      bool any_rung_text = false, saw = false;
      sh.run( b, phys, rep, [ & ]( int, const char* stage ) {
          saw = true;
          if ( stage && std::string( stage ).find( "rung " ) != std::string::npos ) {
             any_rung_text = true;
          }
      } );
      fails += chk( "disabled => progress still reported", saw );
      fails += chk( "disabled => stage text is unmodified", !any_rung_text );
   }

   // ---- chunked progress is numerically inert, and covers all three phases ------
   // assemble/factor/solve are chunked so the callback can fire inside them (the calling
   // thread is a compute worker, so nothing else can keep a GUI alive). Chunking changes
   // parallel granularity, so the first thing to prove is that it changes no result.
   {
      auto b = blob( 5, 3.0, 2.0 );
      Results without = analytic( (int) b.size() )( b, phys, {} );                  // no callback -> single chunk

      std::vector<int> pcts;
      std::vector<std::string> stages;
      Results with = analytic( (int) b.size() )( b, phys, [ & ]( int pct, const char* stage ) {
          pcts.push_back( pct );
          stages.push_back( stage ? stage : "" );
      } );

      fails += chk( "chunking does not change Dt",
                   with.translational_diffusion_centre == without.translational_diffusion_centre );
      fails += chk( "chunking does not change eta",
                   with.intrinsic_viscosity_high == without.intrinsic_viscosity_high );
      fails += chk( "chunking does not change the report", with.report == without.report );

      bool monotone = true, in_range = true;
      for ( size_t i = 0; i < pcts.size(); ++i ) {
         if ( i && pcts[ i ] < pcts[ i - 1 ] ) {
            monotone = false;
         }
         if ( pcts[ i ] < 0 || pcts[ i ] > 100 ) {
            in_range = false;
         }
      }
      auto saw = [ & ]( const char* s ) {
         for ( auto& t : stages ) {
            if ( t.find( s ) != std::string::npos ) {
               return true;
            }
         }
         return false;
      };
      fails += chk( "progress is monotone", monotone );
      fails += chk( "progress within 0..100", in_range );
      fails += chk( "assembly phase reports", saw( "BUILDING MOBILITY MATRIX" ) );
      fails += chk( "factor phase reports", saw( "INVERTING MATRICES" ) );
      fails += chk( "solve phase reports", saw( "SOLVING" ) );
      // Tick COUNT is deliberately not asserted tightly. The chunker targets a wall-clock
      // slice, so on a small model that finishes in milliseconds it correctly uses few
      // large chunks; fine granularity only appears where it is needed. What must hold is
      // that no phase is silent -- assembly and solve previously reported nothing at all.
      fails += chk( "every phase produced at least one tick", pcts.size() >= 3 );
   }

   // ---- recorded subsets identify the beads actually solved on ------------------
   // The caller writes these out as bead models, so an index that does not name the bead
   // the solver used would produce a picture of the wrong thing -- silently.
   {
      auto b = blob( 5, 3.0, 2.0 );
      ShellOptions so;
      so.enabled = true; so.tol = 1e-12;             // unsatisfiable: run the whole ladder
      so.require = {Obs::Dt, Obs::Dr};
      so.ladder = {0.125, 0.25, 0.5, 1.0};
      so.record_subsets = true;
      ShellSolver sh( analytic( (int) b.size() ), so );
      ShellReport rep;
      sh.run( b, phys, rep );

      fails += chk( "one recorded subset per rung", rep.kept.size() == rep.ns.size() );
      bool sized = true, in_range = true, unique = true, exposed_first = true;
      for ( size_t r = 0; r < rep.kept.size(); ++r ) {
         if ( (int) rep.kept[ r ].size() != rep.ns[ r ] ) {
            sized = false;
         }
         std::vector<bool> seen( b.size(), false );
         for ( int k : rep.kept[ r ] ) {
            if ( k < 0 || k >= (int) b.size() ) { in_range = false; continue; }
            if ( seen[ k ] ) {
               unique = false;
            }
            seen[ k ] = true;
         }
         // Each rung is a superset of the one before: the ladder grows the shell.
         if ( r ) {
            for ( int k : rep.kept[ r - 1 ] ) {
               if ( std::find( rep.kept[ r ].begin(), rep.kept[ r ].end(), k ) == rep.kept[ r ].end() ) {
                  exposed_first = false;
               }
            }
         }
      }
      fails += chk( "recorded size matches the rung bead count", sized );
      fails += chk( "indices are in range", in_range );
      fails += chk( "indices are unique within a rung", unique );
      fails += chk( "each rung contains the previous rung's beads", exposed_first );
      // The final rung is the full model, so it must name every bead.
      fails += chk( "full rung records every index",
                   rep.unreduced && rep.kept.back().size() == b.size() );

      // And the selection must be the same beads the solver actually used.
      std::vector<double> ex = shell::exposure( b, (int) b.size(), so.K, so.probe );
      bool matches = true;
      for ( size_t r = 0; r + 1 < rep.kept.size(); ++r ) {
         auto want = shell::reduce_top_frac_idx( b, ex, so.ladder[ r ] );
         if ( want.size() != rep.kept[ r ].size() ) { matches = false; continue; }
         for ( size_t i = 0; i < want.size(); ++i ) {
            if ( (int) want[ i ] != rep.kept[ r ][ i ] ) {
               matches = false;
            }
         }
      }
      fails += chk( "recorded indices are the ranking's own selection", matches );

      // Off by default: nothing recorded, so nothing paid for.
      ShellOptions off = so; off.record_subsets = false;
      ShellSolver sh2( analytic( (int) b.size() ), off );
      ShellReport rep2;
      sh2.run( b, phys, rep2 );
      fails += chk( "not recorded unless asked", rep2.kept.empty() && rep2.ns.size() > 1 );
   }

   // ---- cancellation between rungs ---------------------------------------------
   // A user watching the shells appear must be able to stop on seeing something wrong.
   // Stopping keeps whatever was computed, on its bar, and never claims convergence.
   {
      auto b = blob( 5, 3.0, 2.0 );
      ShellOptions so;
      so.enabled = true; so.tol = 1e-12;            // unsatisfiable: only a stop can end it
      so.require = {Obs::Dt, Obs::Dr};
      so.ladder = {0.125, 0.25, 0.5, 1.0};
      int seen = 0;
      so.should_stop = [ & ]{ return seen >= 2; };    // let two rungs run, then stop
      so.on_rung = [ & ]( int, int, int, double ){ ++seen; };
      ShellSolver sh( analytic( (int) b.size() ), so );
      ShellReport rep;
      Results r = sh.run( b, phys, rep );

      fails += chk( "stop is reported", rep.stopped );
      fails += chk( "stop does not claim convergence", !rep.converged );
      fails += chk( "stop keeps the rungs already run", rep.levels == 2 );
      fails += chk( "stop did not reach the full model", !rep.unreduced );
      fails += chk( "stopped result still carries a bar",
                   rep.err_max > 0.0 && std::isfinite( rep.err_max ) );
      fails += chk( "stopped result is usable", r.translational_diffusion_centre > 0.0 );
      fails += chk( "report explains the stop",
                   r.report.find( "stopped before the ladder" ) != std::string::npos );

      // Stopping before anything runs leaves no result, like an impossible memory cap.
      ShellOptions so2 = so;
      so2.should_stop = []{ return true; };
      so2.on_rung = nullptr;
      ShellSolver sh2( analytic( (int) b.size() ), so2 );
      ShellReport rep2;
      sh2.run( b, phys, rep2 );
      fails += chk( "stop before any rung leaves no result",
                   rep2.stopped && rep2.levels == 0 );

      // And with no hook the ladder is unaffected.
      ShellOptions so3 = so; so3.should_stop = nullptr; so3.on_rung = nullptr;
      ShellSolver sh3( analytic( (int) b.size() ), so3 );
      ShellReport rep3;
      sh3.run( b, phys, rep3 );
      fails += chk( "no hook => ladder runs to the full model",
                   !rep3.stopped && rep3.unreduced );
   }

   // ---- a stop DURING a rung keeps the rungs already finished --------------------
   // The in-process solver could only be stopped between rungs, because its
   // factorization had no interior abort. A solver running as a separate program can be
   // killed mid-rung, and when that happens the ladder must still report what it had --
   // which is what the manual promises the Stop button does.
   {
      auto b = blob( 6, 3.0, 2.0 );
      ShellOptions so;
      so.enabled = true; so.tol = 1e-12;                 // unsatisfiable: run every rung
      int calls = 0;
      SolveFn stops_on_third = [ & ]( const std::vector<Bead>& rb, const PhysParams& p,
                                   const ProgressFn& pr ) {
         if ( ++calls == 3 ) {
            throw Stopped();
         }
         return Analytic{(int) b.size()}( rb, p, pr );
      };
      ShellSolver sh( stops_on_third, so );
      ShellReport rep;
      Results r = sh.run( b, phys, rep );
      fails += chk( "mid-rung stop is reported as a stop", rep.stopped );
      fails += chk( "mid-rung stop keeps the finished rungs", rep.levels == 2 );
      fails += chk( "mid-rung stop keeps a usable result", r.translational_diffusion_centre > 0 );
      fails += chk( "mid-rung stop carries an error bar", rep.err_max > 0.0 );
      fails += chk( "mid-rung stop does not claim convergence", !rep.converged );
   }

   // ---- a stop before ANY rung finishes has nothing to report --------------------
   {
      auto b = blob( 6, 3.0, 2.0 );
      ShellOptions so;
      so.enabled = true;
      SolveFn always_stops = []( const std::vector<Bead>&, const PhysParams&,
                                const ProgressFn& ) -> Results { throw Stopped(); };
      ShellSolver sh( always_stops, so );
      ShellReport rep;
      bool propagated = false;
      try {
         sh.run( b, phys, rep );
      } catch ( const Stopped& ) {
         propagated = true;
      }
      fails += chk( "a stop with no completed rung propagates", propagated );
   }

   std::printf( "%s (%d failures)\n", fails ? "FAILURES" : "ALL PASS", fails );
   return fails ? 1 : 0;
}
