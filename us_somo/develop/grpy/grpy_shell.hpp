// Self-validating shell reduction over an injected solver.
//
// The solver is reached through grpy::SolveFn rather than named directly: the GRPY-derived
// code is GPLv3 and lives in its own program (ehb54/grpy-cpp), while this file and
// grpy_exposure.hpp are original work and stay with UltraScan. SOMO supplies a SolveFn that
// runs that program once per rung. See ehb54/ultrascan-tickets#1012.
//
// Shell reduction is only defensible if it reports its own error. A fixed "keep X% of
// beads" rule is just another tuned constant, and the user has no way to know what it cost
// them on their structure. So instead of one reduced solve we run a LADDER of increasing
// bead counts and stop when two consecutive rungs agree: the converged value is the
// answer, and the gap between rungs -- Richardson-extrapolated -- is the reported bar.
//
// COST. The ladder is geometric (each rung ~2x the previous) against O(N^3), so the whole
// ladder costs ~1.14x its final rung: the convergence check is nearly free relative to
// simply solving at the final size.
//
// MEASURED, against unreduced exact GRPY as ground truth. The full grid is 23 models
// (N=204-4068) x 5 observables x 4 tolerances; every reduced evaluation met its requested
// tolerance and the reported bar bounded the true error, worst case spending 55.3% of the
// allowed budget. The observed convergence order (median 1.83) matches the value predicted
// by a separate raw reduction sweep -- the error model is derived, not fitted.
//
// An earlier version of this note claimed 92/92 and 252/252. Both were wrong. The second
// counted 7 observables, three of which are research-only with no shipped equivalent; and
// the multi-observable grid covered 12 of the 23 models, so 11 models never had intrinsic
// viscosity or rotational diffusion checked at all. Scored honestly on the value this code
// actually returns -- the finest rung, not the extrapolation -- the same evidence was
// 87/92 and 177/180, and widening the grid to all 23 models then exposed two outright
// tolerance misses. That is what raising floor_frac to 0.75 fixes; see its comment below.
//
// SCOPE OF THE SPEEDUP. Relative to an unreduced model the gain is large (up to ~120x),
// but SOMO defaults to ASA buried-bead exclusion, which already removes most of the dead
// beads. On top of that production baseline the incremental gain is ~2-3x, rising with
// model size. The durable contribution is therefore the error bar, not the speed: the
// existing exclusion is a binary heuristic that reports no uncertainty at all.
//
// DO NOT use a finer ladder. A x1.5 ladder was tested and rejected: it bought no median
// speedup and broke the guarantee (honesty 100% -> 84%). Closely-spaced rungs make the gap
// ratio a noisy estimator of the convergence order, and bead selection is not smooth mesh
// refinement -- a finer rung swaps in a different subset rather than refining the previous
// one. Well-separated rungs are what make the bar trustworthy.
#pragma once
#include <algorithm>
#include <cmath>
#include <cstdio>      // std::snprintf, used in annotation()
#include <functional>
#include <string>
#include <vector>
#include "grpy_types.hpp"
#include "grpy_exposure.hpp"

namespace grpy {


struct ShellOptions {
   bool   enabled = false;          // off by default: results must not move silently
   double tol     = 5e-3;           // relative tolerance on every required observable
   double probe   = 1.4;            // Shrake-Rupley probe radius (model length units)
   // Exposure sample points per bead. Raised from 64 to 512 to make the selection far less
   // sensitive to the ORIENTATION of the input coordinates.
   //
   // The point pattern is generated once in the coordinate frame and applied to every bead by
   // translation and scaling, so it does not rotate with the structure: a rigid rotation changes
   // 34-77% of bead exposures by a few of the K sample points and therefore changes the ranking.
   // That reaches the answer almost entirely through the stopping decision -- where the ladder
   // stops at the same rung in every frame the reported value moves by at most 0.075%, and where
   // the orientation flips the rung by one it moves by up to 1.043%.
   //
   // Measured over twelve rigid motions of six models, matched but for K: at 64 one case of
   // eighteen changed its stopping rung and the frame-to-frame spread reached 0.23%; at 512 no
   // case changes rung and the spread falls to 0.016%. Retained beads are unchanged (mean 31.5%
   // vs 31.3%) and compliance and coverage are perfect either way, so the finer quadrature costs
   // nothing but exposure time -- about 3% of the ladder, since exposure is milliseconds against
   // a solve of seconds. It does not make selection frame-independent, only ~14x less sensitive.
   int    K       = 512;
   // Doubling ladder. The final 1.0 rung is the unreduced model, so an unreducible
   // structure degrades to exactly today's behaviour rather than to a wrong answer.
   std::vector<double> ladder = {0.0625, 0.125, 0.25, 0.5, 1.0};
   std::vector<Obs> require = {Obs::Dt, Obs::Dr, Obs::Sedimentation};
   double k_min = 1.0, k_max = 3.0, safety = 1.5;

   // Floor on the error estimate, as a fraction of the raw inter-rung gap: the estimate never
   // claims a tightening better than 1/floor_frac. Raised from 0.5 to 0.75 after validation on
   // the full 23-model grid with all five observables -- the earlier grid ran five observables
   // over only 12 models, and widening it exposed two evaluations that met the estimator's stop
   // test yet missed the requested tolerance (2GD1 intrinsic viscosity at 1%: estimate 0.859%,
   // true error 1.066%), plus eleven more that were compliant but undercovered.
   //
   // Raising THIS rather than `safety` is deliberate. The estimate is the max of the two terms,
   // so while the floor binds the safety factor does nothing: with a doubling ladder and the
   // order at k_max = 3 the Richardson remainder is gap/7, and safety * gap/7 stays under
   // 0.5 * gap for any safety <= 3.5. Measured on the same grid, safety = 2.0 fixed neither
   // failure and still drove the median case to retain every bead, because it inflates the
   // estimate for the well-behaved majority where the floor is slack. The floor acts only on
   // the high-observed-order rows that actually fail.
   //
   // At 0.75 the full grid is clean: 200/200 reduced evaluations inside their tolerance and
   // all 200 covered, worst case using 55.3% of the allowed error budget (was 106.6%), for
   // 4.6 percentage points more beads retained on average. Exposed rather than hardcoded so it
   // can be validated without patching a copy of this header -- which is how it had to be
   // measured, and a copy of an estimator is exactly what makes results unattributable later.
   double floor_frac = 0.75;

   // Largest rung the ladder may attempt, in beads (0 = unlimited). Set by the caller from
   // the available memory: the solver holds the tiled upper triangle of an 11N x 11N
   // matrix, so a rung that exceeds RAM would thrash the machine. Capping the ladder turns
   // the caller's "model too large, refused" into "answer from the largest rung that fits,
   // with its error bar" -- which is the whole point of having a bar. A capped ladder that
   // has not converged is reported as not converged; it is never silently passed off as if
   // it had.
   int max_beads = 0;

   // Optional per-rung notification, called once after each rung completes with
   // (rung index 0-based, rungs planned, beads used, worst error estimate so far).
   // The estimate is 1.0 until a second rung exists to difference against. Lets the
   // caller log the ladder converging instead of leaving the user watching a bar.
   std::function<void( int, int, int, double )> on_rung;

   // Record which beads each rung kept, in ShellReport::kept. Off by default: it is only
   // wanted when the reduced models are to be written out or inspected.
   bool record_subsets = false;

   // Optional cancellation, consulted before each rung. Returning true stops the ladder
   // where it stands: whatever has been computed keeps its error bar and is reported as
   // NOT converged, exactly like any other early stop.
   //
   // Checked between rungs, which is where stopping is worth most: each rung costs roughly
   // eight times the one before it, so cancelling before the next one begins saves almost
   // everything that remained. A rung already running is not immune either -- the solve is
   // a separate program and ProcessSolver kills it -- and run() treats a stop DURING a rung
   // exactly like one between rungs (see the Stopped handler below).
   std::function<bool()> should_stop;
};

struct ShellReport {
   bool   attempted = false;
   bool   converged = false;
   bool   unreduced = false;   // ladder ran out onto the full model: result is exact
   // Ladder was stopped short by ShellOptions::max_beads rather than by convergence. The
   // result still stands on its reported bar; it simply could not be refined further on
   // this machine. When levels == 0 not even the smallest rung fit, and there is no result.
   bool   mem_capped = false;
   // Ladder was cancelled between rungs via ShellOptions::should_stop. As with mem_capped,
   // the result stands on its bar and is never marked converged; levels == 0 means it was
   // stopped before anything ran, so there is no result at all.
   bool   stopped = false;
   int    n_full = 0, n_used = 0, levels = 0;
   double err_max = 1.0;                 // max bar over the required observables
   Obs    worst = Obs::Dt;
   std::vector<Obs>    require;          // echo of what was asked for
   std::vector<double> err_est;          // parallel to `require`
   std::vector<double> extrapolated;     // parallel to `require`; Richardson value
   // Parallel to `require`; the finest rung's own value -- i.e. what Results carries and
   // what every reported scalar in this run is. Recorded so that the report can state the
   // computed and the extrapolated value side by side: the two are different answers, the
   // error estimate is the distance between them, and a reader who is shown only one of
   // them cannot tell which the estimate is attached to.
   std::vector<double> reported;
   std::vector<double> k_obs;            // parallel to `require`; 0 => not extrapolated
   std::vector<int>    ns;               // bead count per rung
   // Every rung's value for every requested observable: values[rung][observable], with
   // rung indexed as `ns`. Kept so that a validation run can recompute any variant of the
   // estimator from the stored sequence instead of re-solving the whole ladder, which
   // costs hours per corpus. Small: rungs x observables doubles.
   std::vector<std::vector<double>> values;

   // Which mechanism produced each observable's estimate, parallel to `require`. Without
   // this an audit cannot tell whether the extrapolation or one of the safeguards is
   // responsible for a reported margin.
   struct Provenance {
      bool extrapolated = false;      // Richardson succeeded (else the raw gap was used)
      bool clamped_low = false;       // observed order pinned at k_min
      bool clamped_high = false;      // observed order pinned at k_max
      bool floored = false;           // the 2x-tightening floor set the estimate
      const char* declined = nullptr; // why extrapolation was refused, if it was
   };
   std::vector<Provenance> prov;
   // Indices, into the bead list passed to run(), of the beads kept at each rung -- only
   // when ShellOptions::record_subsets is set, since it is wanted for writing the reduced
   // models out and is dead weight otherwise. Parallel to `ns`. The full rung (if reached)
   // records every index, so a consumer never has to special-case it.
   std::vector<std::vector<int>> kept;

   // True when intrinsic viscosity was NOT among the required observables, or was but did
   // not converge. Callers MUST NOT propagate viscosity (or any viscosity-derived
   // quantity, e.g. the Einstein radius) when this is set -- the value is present in the
   // report for the record, but is not trustworthy.
   bool viscosity_unreliable = true;
};

// Richardson over a geometric ladder.
//
//   mu(f) = mu_inf + C f^-k        (error shrinks as the kept fraction grows)
//   gap1/gap2 = (1 - r1^-k) / ( r1^-k (1 - r2^-k) )        -> solve for k
//   remaining error at the finest rung = gap2 / (r2^k - 1)
//
// For a pure doubling ladder this reduces to k = log2(gap1/gap2), err = gap2/(2^k - 1).
// Ratios come from ACTUAL bead counts, not the nominal fractions, because reduce_top_frac
// quantizes (ceil, tie-breaking, small-N floor).
//
// SAFETY. k is clamped and the bar carries a margin. Clamping k LOW is the conservative
// direction: k = k_min = 1 with a doubling ladder returns exactly the raw gap, i.e. the
// un-extrapolated estimate. Non-monotone or sign-flipped gaps mean we are outside the
// asymptotic regime, so we decline to extrapolate and fall back to the raw gap.
struct Richardson {
   double mu = 0, err = 0, k = 0;
   bool ok = false;
   // Which mechanism produced this estimate. Recorded rather than inferred because the
   // safeguards, not the extrapolation, turn out to carry much of the reported margin,
   // and a validation run that cannot say which one fired cannot apportion the credit.
   bool clamped_low = false;    // observed order pinned at k_min (returns ~the raw gap)
   bool clamped_high = false;   // observed order pinned at k_max
   bool floored = false;        // the 2x-tightening floor set the estimate, not Richardson
   const char* declined = nullptr;   // why extrapolation was refused (nullptr = it wasn't)
};

inline double richardson_order( double r1, double r2, double ratio_obs,
                               double k_min, double k_max ) {
   auto G = [ & ]( double k ) {
      double a = std::pow( r1, -k ), b = std::pow( r2, -k );
      return ( 1.0 - a ) / ( a * ( 1.0 - b ) ) - ratio_obs;
   };
   if ( G( k_min ) >= 0.0 ) {
      return k_min;
   }
   if ( G( k_max ) <= 0.0 ) {
      return k_max;
   }
   double lo = k_min, hi = k_max;
   for ( int it = 0; it < 60; ++it ) {
      double mid = 0.5 * ( lo + hi );
      ( G( mid ) < 0.0 ? lo : hi ) = mid;
   }
   return 0.5 * ( lo + hi );
}

inline Richardson richardson( const std::vector<double>& v, const std::vector<int>& ns,
                             double k_min, double k_max, double safety,
                             double floor_frac ) {
   Richardson r;
   const size_t n = v.size();
   if ( n < 3 || ns.size() != n )   { r.declined = "fewer than three rungs"; return r; }
   double gap1 = v[ n - 3 ] - v[ n - 2 ], gap2 = v[ n - 2 ] - v[ n - 1 ];
   if ( gap1 == 0.0 || gap2 == 0.0 ) { r.declined = "a gap was exactly zero"; return r; }
   if ( ( gap1 > 0 ) != ( gap2 > 0 ) )  { r.declined = "gaps not monotone"; return r; }
   double ratio = std::fabs( gap1 ) / std::fabs( gap2 );
   if ( !( ratio > 1.0 ) )            { r.declined = "gaps not shrinking"; return r; }
   double r1 = (double) ns[ n - 2 ] / ns[ n - 3 ], r2 = (double) ns[ n - 1 ] / ns[ n - 2 ];
   if ( !( r1 > 1.0 ) || !( r2 > 1.0 ) ) { r.declined = "rung sizes not increasing"; return r; }
   double k = richardson_order( r1, r2, ratio, k_min, k_max );
   // richardson_order returns the bound verbatim when it clamps, so equality identifies it.
   r.clamped_low  = ( k == k_min );
   r.clamped_high = ( k == k_max );
   double rem = gap2 / ( std::pow( r2, k ) - 1.0 );
   r.k = k;
   r.mu = v[ n - 1 ] - rem;
   if ( r.mu == 0.0 ) { r.declined = "extrapolated value was zero"; return r; }
   double err = safety * std::fabs( rem ) / std::fabs( r.mu );

   // FLOOR: never claim better than half the raw inter-rung gap.
   //
   // The power-law model assumes the error falls smoothly as the kept fraction grows.
   // That fails when the exposure distribution is highly degenerate -- a perfect cubic
   // lattice realizes only ~9 distinct exposure values over 216 beads, so successive
   // rungs swallow whole symmetry shells at once instead of refining. The estimated
   // order then comes out spuriously high (measured k=2.73 against a true convergence
   // far slower), the extrapolation tightens too aggressively, and the bar understates.
   // Breaking the symmetry with even a 0.05 A jitter restores honesty.
   //
   // This floor caps the tightening at 2x rather than the (r^k_max - 1) = 7x the
   // extrapolation could otherwise claim. It binds only when k is high -- exactly the
   // aggressive regime that carries the risk -- and is slack at the median observed
   // k~1.83, so the extrapolation keeps its benefit on well-behaved models.
   double raw_gap = std::fabs( gap2 ) / std::fabs( r.mu );
   r.floored = ( floor_frac * raw_gap > err );
   r.err = std::max( err, floor_frac * raw_gap );
   r.ok = true;
   return r;
}

// Wraps Solver. Returns the FINAL RUNG's Results verbatim, so the structured scalars and
// the embedded report text always agree with each other; the Richardson-extrapolated
// values and the per-observable bars are delivered separately in ShellReport. (The
// alternative -- overwriting the scalars with extrapolated values -- would produce a
// Results whose fields contradict its own on-disk report.)
class ShellSolver {
public:
   // `solve` runs ONE bead list to completion -- in SOMO, one invocation of the external
   // GRPY program. The ladder calls it once per rung, at most five times per model, which
   // is negligible against O((11N)^3): the last rung alone is ~7/8 of the total work.
   ShellSolver( SolveFn solve, ShellOptions sopt = {} )
      : solve_( std::move( solve ) ), sopt_( sopt ) {}

   Results run( const std::vector<Bead>& beads, const PhysParams& p,
               ShellReport& rep, const ProgressFn& progress = {} ) const {
      rep = ShellReport{};
      rep.n_full = (int) beads.size();
      rep.require = sopt_.require;

      if ( !sopt_.enabled || beads.size() < 32 ) {          // too small to be worth reducing
         Results r = solve_( beads, p, progress );
         rep.n_used = (int) beads.size();
         rep.converged = true;
         rep.viscosity_unreliable = false;              // unreduced: everything stands
         return r;
      }
      rep.attempted = true;

      // MW and Rg are properties of the MOLECULE, not of the hydrodynamic subset. If we
      // let Solver re-derive them from a reduced bead list, MW would fall with the
      // dropped beads (silently corrupting sedimentation and both intrinsic viscosities,
      // which are mass-normalized) and Rg would rise (a hollow shell has a larger radius
      // of gyration than the solid body). Both are therefore pinned to full-model values
      // and carried across every rung.
      PhysParams pin = p;
      if ( pin.mw <= 0 ) { double s = 0; for ( const auto& b : beads ) s += b.mw; pin.mw = s; }
      const double rg2_full = full_rg2( beads );

      std::vector<double> ex = shell::exposure( beads, (int) beads.size(),
                                               sopt_.K, sopt_.probe );

      // PROGRESS. Every rung is a separate solve sweeping 0..100%, so reporting each one
      // raw makes the bar restart several times per model and the stage text never says
      // which rung is running. Instead map each rung onto its share of the WHOLE ladder,
      // weighted by predicted cost (~N^3), and prefix the stage with the rung. The bar
      // then advances monotonically across the ladder.
      //
      // The denominator assumes the worst case -- the ladder runs to its last planned
      // rung -- so converging early makes the bar jump to done, which is correct: it
      // finished. Sizes here are the nominal ceil(f*N); reduce_top_frac quantizes
      // slightly differently, but this only weights a progress bar.
      std::vector<int> planned;
      for ( double f : sopt_.ladder ) {
         int n = ( f >= 1.0 ) ? (int) beads.size()
                       : (int) std::ceil( f * (double) beads.size() );
         if ( n > ( int )beads.size() ) {
            n = (int) beads.size();
         }
         if ( !planned.empty() && n <= planned.back() ) {
            continue;
         }
         if ( sopt_.max_beads > 0 && n > sopt_.max_beads ) {
            break;
         }
         planned.push_back( n );
      }
      if ( planned.empty() ) {
         planned.push_back( (int) beads.size() );
      }
      std::vector<double> cum( planned.size() + 1, 0.0 );
      for ( size_t i = 0; i < planned.size(); ++i ) {
         double n = (double) planned[ i ];
         cum[ i + 1 ] = cum[ i ] + n * n * n;
      }
      const double work_total = cum.back() > 0.0 ? cum.back() : 1.0;
      std::string stage_buf;   // outlives each synchronous progress() call
      std::vector<std::vector<double>> hist;             // hist[rung][observable]
      Results last;
      for ( double f : sopt_.ladder ) {
         if ( sopt_.should_stop && sopt_.should_stop() ) { rep.stopped = true; break; }
         std::vector<int>  rung_idx;
         std::vector<Bead> rb;
         if ( f >= 1.0 ) {
            rb = beads;
            if ( sopt_.record_subsets ) {
               rung_idx.reserve( beads.size() );
               for ( int i = 0; i < (int) beads.size(); ++i ) {
                  rung_idx.push_back( i );
               }
            }
         } else {
            rb = subset( beads, ex, f, sopt_.record_subsets ? &rung_idx : nullptr );
         }
         if ( !rep.ns.empty() && (int) rb.size() <= rep.ns.back() ) {
            continue;
         }
         // Memory cap. Checked after building the subset (cheap, O(N) selection) but
         // before the solve (the expensive part), so the rung costs nothing to reject.
         // The ladder ascends, so once one rung is too large every later one is too:
         // stop rather than continue. Whatever rungs already ran keep their bars.
         if ( sopt_.max_beads > 0 && (int) rb.size() > sopt_.max_beads ) {
            rep.mem_capped = true;
            break;
         }
         const size_t ri = rep.ns.size();               // 0-based index of this rung
         const double f0 = cum[ std::min( ri, planned.size() ) ] / work_total;
         const double f1 = cum[ std::min( ri + 1, planned.size() ) ] / work_total;
         ProgressFn wrapped;
         if ( progress ) {
            const int nb = (int) rb.size();
            wrapped = [ &, ri, nb, f0, f1 ]( int pct, const char* stage ) {
               stage_buf = "rung " + std::to_string( ri + 1 ) + "/"
                       + std::to_string( planned.size() ) + ", " + std::to_string( nb )
                       + " beads: " + ( stage ? stage : "" );
               progress( (int) std::lround( 100.0 * ( f0 + ( f1 - f0 ) * ( pct / 100.0 ) ) ),
                        stage_buf.c_str() );
            };
         }
         // A solver that runs out of process can be killed the instant Stop is
         // pressed, rather than having to finish the rung it is on -- but the rungs
         // already completed are still perfectly good, and the caller was promised
         // "the result and error estimate obtained so far". So a stop DURING a rung is
         // treated exactly like a stop between rungs: the ladder ends where it stands
         // and reports itself as not converged. Only a stop before the first rung has
         // finished leaves nothing to report, and that one propagates.
         try {
            last = solve_( rb, pin, wrapped );
         } catch ( const Stopped& ) {
            rep.stopped = true;
            if ( rep.levels == 0 ) {
               throw;
            }
            break;
         }
         last.rg2 = rg2_full;                           // see note above
         rep.ns.push_back( (int) rb.size() );
         if ( sopt_.record_subsets ) {
            rep.kept.push_back( std::move( rung_idx ) );
         }
         rep.n_used = (int) rb.size();
         ++rep.levels;

         std::vector<double> cur;
         for ( Obs o : sopt_.require ) {
            cur.push_back( obs_value( last, o ) );
         }
         hist.push_back( cur );

         const size_t M = sopt_.require.size();
         rep.reported = cur;                            // the finest rung solved so far
         rep.err_est.assign( M, 1.0 ); rep.extrapolated.assign( M, 0.0 ); rep.k_obs.assign( M, 0.0 );
         rep.prov.assign( M, ShellReport::Provenance{} );
         rep.values = hist;                             // every rung, every observable
         rep.err_max = 0.0;
         for ( size_t m = 0; m < M; ++m ) {
            // Per-observable Richardson: each quantity converges at its own order, so
            // a single shared k would be wrong for all but one of them.
            std::vector<double> series;
            for ( auto& h : hist ) {
               series.push_back( h[ m ] );
            }
            Richardson ri = richardson( series, rep.ns, sopt_.k_min, sopt_.k_max, sopt_.safety,
                                       sopt_.floor_frac );
            // Assigned field by field, not with a braced list. Provenance carries default
            // member initializers, which makes it a non-aggregate before C++14, so
            // `= { ... }` does not compile under the C++11 the Qt5 builds still use. The
            // initializers are kept: prov is filled by assign() above and they are what make
            // an untouched entry read false/nullptr rather than indeterminate.
            ShellReport::Provenance& pv = rep.prov[ m ];
            pv.extrapolated = ri.ok;
            pv.clamped_low  = ri.clamped_low;
            pv.clamped_high = ri.clamped_high;
            pv.floored      = ri.floored;
            pv.declined     = ri.declined;
            if ( ri.ok ) { rep.extrapolated[ m ] = ri.mu; rep.err_est[ m ] = ri.err; rep.k_obs[ m ] = ri.k; }
            else {
               rep.extrapolated[ m ] = cur[ m ];
               rep.err_est[ m ] = ( series.size() > 1 && cur[ m ] != 0.0 )
                  ? std::fabs( series.back() - series[ series.size() - 2 ] ) / std::fabs( cur[ m ] )
                  : 1.0;
            }
            if ( rep.err_est[ m ] > rep.err_max ) { rep.err_max = rep.err_est[ m ]; rep.worst = sopt_.require[ m ]; }
         }
         if ( sopt_.on_rung ) {
            sopt_.on_rung( (int) ri, (int) planned.size(), (int) rb.size(), rep.err_max );
         }

         // The full-model test comes FIRST, before the tolerance test. If the rung just
         // solved is the whole model the answer is exact, and that is true whether or not
         // the tolerance happened to be met at the same rung. Testing convergence first
         // let an exact result exit as merely "converged", keeping the inter-rung gap as
         // its estimate: a 216-bead model at tol=0.5% reported 0.159% on an answer whose
         // true error was zero, and did not set `unreduced`. The earlier regression test
         // missed it because it used an unsatisfiable tolerance, so the tolerance branch
         // could never win the race.
         if ( f >= 1.0 || rep.n_used >= rep.n_full ) {
            // Ladder exhausted onto the UNREDUCED model. The result is then exact, so
            // every bar is zero -- the inter-rung gap that produced them describes the
            // coarser rung we just discarded, not this answer. Reporting that stale gap
            // would be honest (it bounds a true error of zero) but plainly misleading,
            // and it would wrongly mark viscosity unreliable on an exact result.
            rep.converged = true;
            rep.unreduced = true;
            for ( auto& e : rep.err_est ) {
               e = 0.0;
            }
            for ( size_t m = 0; m < rep.extrapolated.size() && m < cur.size(); ++m ) {
               rep.extrapolated[ m ] = cur[ m ];
            }
            rep.err_max = 0.0;
            break;
         }
         if ( hist.size() > 1 && rep.err_max < sopt_.tol ) { rep.converged = true; break; }
      }

      rep.viscosity_unreliable = !viscosity_ok( rep );
      last.report += annotation( rep );
      return last;
   }

private:
   SolveFn       solve_;
   ShellOptions  sopt_;

   // Volume-weighted mean square radius of gyration of a union of uniform spheres:
   // Rg^2 = SUM[ (3/5)a_i^2 + |r_i - R|^2 ] V_i / SUM V_i, about the volume centroid R,
   // where the (3/5)a^2 term is a solid sphere's own second moment. Standard result,
   // written here from the definition so that this file needs nothing from the solver.
   static double full_rg2( const std::vector<Bead>& b ) {
     double cx = 0, cy = 0, cz = 0, vol = 0;
     for ( const Bead& q : b ) {
        const double v = q.radius * q.radius * q.radius;   // 4/3 pi cancels in the ratio
        cx  += q.x * v;
        cy  += q.y * v;
        cz  += q.z * v;
        vol += v;
     }
     if ( vol <= 0 ) {
        return 0.0;
     }
     cx /= vol;
     cy /= vol;
     cz /= vol;
     double rg2 = 0;
     for ( const Bead& q : b ) {
        const double v  = q.radius * q.radius * q.radius;
        const double dx = q.x - cx, dy = q.y - cy, dz = q.z - cz;
        rg2 += ( 3.0 / 5.0 * q.radius * q.radius + dx * dx + dy * dy + dz * dz ) * v / vol;
     }
     return rg2;
   }
   // Selection by INDEX. The earlier version rebuilt the API beads by searching the full
   // list for matching coordinates -- O(keep*N), 32M comparisons on an 11328-bead model --
   // and could not tell the caller which beads it had chosen. Indices are what the ranking
   // produces anyway, so taking them directly is both cheaper and more informative.
   static std::vector<Bead> subset( const std::vector<Bead>& b,
                                   const std::vector<double>& ex, double f,
                                   std::vector<int>* kept_idx ) {
      std::vector<size_t> idx = shell::reduce_top_frac_idx( b, ex, f );
      std::vector<Bead> out; out.reserve( idx.size() );
      for ( size_t k : idx ) {
         out.push_back( b[ k ] );
      }
      if ( kept_idx ) {
         kept_idx->clear(); kept_idx->reserve( idx.size() );
         for ( size_t k : idx ) {
            kept_idx->push_back( (int) k );
         }
      }
      return out;
   }
   // Viscosity is trustworthy if the solve was unreduced (exact, so nothing to converge),
   // or if it was actually required to converge and did.
   bool viscosity_ok( const ShellReport& rep ) const {
      if ( rep.unreduced ) {
         return true;
      }
      if ( !rep.converged ) {
         return false;
      }
      bool required = false;
      for ( Obs o : sopt_.require ) {
         if ( o == Obs::EtaInf || o == Obs::EtaZero ) {
            required = true;
         }
      }
      return required;
   }
   std::string annotation( const ShellReport& rep ) const {
      std::string s = "\n";
      s += "-------------------------------------------------------------------------------\n";
      s += " SHELL REDUCTION was applied to this calculation.\n";
      if ( rep.levels == 0 ) {
         // No rung completed, so there is no result to describe. The caller is responsible
         // for refusing; say plainly WHICH reason applies rather than print empty stats.
         // levels == 0 is reached two ways -- the memory cap rejected even the smallest
         // rung, or should_stop fired before the first rung finished -- and reporting a
         // user's own Stop as a machine limitation sends them looking for the wrong thing.
         s += rep.stopped
           ? "   NO RESULT: the calculation was stopped before the first ladder rung\n"
             "   finished, so no result was produced.\n"
           : rep.mem_capped
           ? "   NO RESULT: even the smallest ladder rung exceeded the available\n"
             "   memory, so no calculation was performed.\n"
           : "   NO RESULT: no ladder rung completed, so no calculation was performed.\n";
         s += "-------------------------------------------------------------------------------\n";
         return s;
      }
      s += "   beads used: " + std::to_string( rep.n_used ) + " of " + std::to_string( rep.n_full )
        + "   ladder rungs: " + std::to_string( rep.levels ) + "\n";
      s += rep.unreduced
        ? "   converged: yes -- ladder reached the FULL model, so this result is\n"
          "              exact and no reduction error was introduced.\n"
        : rep.converged
        ? "   converged: yes\n"
        : rep.stopped
        ? "   converged: NO -- the calculation was stopped before the ladder had\n"
          "              converged. The estimated errors below are the reliable\n"
          "              statement about this result.\n"
        : rep.mem_capped
        ? "   converged: NO -- the ladder was stopped by the available memory before\n"
          "              the requested tolerance was reached. The estimated errors\n"
          "              below are the reliable statement about this result; a machine\n"
          "              with more memory would refine it further.\n"
        : "   converged: NO -- the requested tolerance was not reached; the estimated\n"
          "              errors below are the reliable statement about this result.\n";
      // Both values, side by side. The scalars reported everywhere else in this file are
      // the COMPUTED ones -- the finest rung actually solved -- because a reported value
      // must be one the calculation produced, not one inferred from a trend. The
      // extrapolated column is what the ladder projects the full model would give, and
      // the estimated error is the distance between the two, safety factor included.
      // Showing only one of them would leave the reader unable to tell which value the
      // error belongs to, which is the whole content of the estimate.
      s += "\n";
      s += "   quantity                                       computed    extrapolated   est. error\n";
      for ( size_t m = 0; m < rep.require.size() && m < rep.err_est.size(); ++m ) {
         char buf[ 200 ];
         const bool have = m < rep.reported.size();
         std::snprintf( buf, sizeof( buf ), "   %-42s %12.6g %15.6g %9.4f %%\n",
                       obs_name( rep.require[ m ] ),
                       have ? rep.reported[ m ] : 0.0,
                       rep.extrapolated[ m ], 100.0 * rep.err_est[ m ] );
         s += buf;
      }
      s += "\n";
      s += "   The results reported elsewhere in this file are the COMPUTED column. The\n";
      s += "   estimated error is an estimate of how far that value stands from the full\n";
      s += "   model, obtained as its distance from the EXTRAPOLATED column; the two\n";
      s += "   columns are different answers and neither is a correction to the other.\n";
      if ( rep.viscosity_unreliable ) {
         s += "\n";
         s += " *** WARNING: intrinsic viscosity (and the viscosity-derived Einstein\n";
         s += " *** radius) were NOT converged and are UNRELIABLE. They are retained\n";
         s += " *** here for the record only and are withheld from the reported\n";
         s += " *** results. Re-run requiring intrinsic viscosity to obtain usable\n";
         s += " *** values (this keeps more beads and is correspondingly slower).\n";
      }
      s += "-------------------------------------------------------------------------------\n";
      return s;
   }
};

}  // namespace grpy
