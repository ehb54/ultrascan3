// Value types exchanged with GRPY, and the observables the shell reduction converges.
//
// These are plain data structures and an injection point -- original work, with nothing
// translated from GRPY.f. They are what lets the shell reduction (grpy_shell.hpp) and the
// exposure ranking (grpy_exposure.hpp), which are also original work, stay in UltraScan
// under its own licence while the GRPY-derived solver lives in its own GPLv3 program.
// See ehb54/ultrascan-tickets#1012.
//
// The solver is reached through SolveFn, so this side never names it: SOMO supplies a
// function that runs the external GRPY program on a bead list and returns its results.
#pragma once
#include <functional>
#include <stdexcept>
#include <string>
#include <vector>

namespace grpy {

// Thrown by a SolveFn when the caller's stop predicate fires. Distinct from a failure, so
// that "the user pressed Stop" and "the calculation broke" can be told apart -- they read
// identically when both arrive as a plain runtime_error. Declared here rather than beside
// the process solver so the shell reduction can catch it without depending on Qt.
class Stopped : public std::runtime_error {
public:
   Stopped() : std::runtime_error( "GRPY: stopped at the user's request" ) {}
};

struct Bead {
   double x;
   double y;
   double z;
   double radius;
   double mw;
};

// Physical / solvent parameters. Defaults are GRPY's -u (us-somo) mode: the hydrodynamic
// quantities come out at 20 C, eta = 0.01 P, rho = 1.
struct PhysParams {
   double      temperature_C = 20.0;
   double      eta           = 0.01;      // solvent viscosity [P]
   double      rho           = 1.0;       // solution density
   double      vbar          = 0.0;       // partial specific volume (from the model)
   double      units         = 1e-8;      // model length scale [cm]
   double      mw            = 0.0;       // total mass; if <= 0, summed from bead mw
   std::string input_label   = "us-somo"; // "from the: <label> input file" in the report
};

// Settings passed through to the solver. The external program takes these as environment
// variables, so that its command line stays exactly the one the Fortran GRPY published.
struct Options {
   bool        single = false;            // single-precision storage/factor: half the memory
   int         tile   = 256;
   std::string ooc_dir;                   // "" = in core; else spill the tiled matrix here
};

// What a solve returns. `report` is the GRPY report text verbatim, which is what SOMO
// parses and writes to disk; the scalars are the same quantities in structured form.
struct Results {
   double rotational_diffusion         = 0;   // Dr        [s^-1]
   double sedimentation                = 0;   // s         [Svedberg]
   double translational_diffusion      = 0;   // Dt origin [cm^2/s]
   double translational_diffusion_centre = 0; // Dt at the mobility centre [cm^2/s]
   double intrinsic_viscosity_high     = 0;   // eta_oo    [cm^3/g]
   double intrinsic_viscosity_zero     = 0;   // eta_0     [cm^3/g]
   double tau_vector[ 3 ]              = { 0, 0, 0 };
   double tau_tensor[ 5 ]              = { 0, 0, 0, 0, 0 };
   double tau_harmonic                 = 0;
   double stokes_radius_Dt             = 0;
   double stokes_radius_Dr             = 0;
   double rg2                          = 0;
   double mass                         = 0;
   double diffusion_origin[ 6 ][ 6 ]   = {};
   double diffusion_centre[ 6 ][ 6 ]   = {};
   std::string report;
};

// Progress, as percent complete and a stage name.
using ProgressFn = std::function<void( int, const char* )>;

// One solve of one bead list. SOMO's implementation writes the beads to a .grpy file, runs
// the GRPY program on it and parses what comes back; a test can substitute anything.
using SolveFn = std::function<Results( const std::vector<Bead>&, const PhysParams&,
                                       const ProgressFn& )>;

// Observables the ladder can be asked to converge. They do NOT share a reduction frontier:
// measured median error ratio relative to D_t at equal reduction is 1.77x for D_r but
// 3.34x for intrinsic viscosity, which drove the stopping decision in 36/36 test cases.
// Requiring viscosity therefore costs a large part of the speedup, so the caller chooses.
enum class Obs { Dt, Dr, Sedimentation, EtaInf, EtaZero };

inline const char* obs_name( Obs o ) {
   switch ( o ) {
      case Obs::Dt:            return "translational diffusion";
      case Obs::Dr:            return "rotational diffusion";
      case Obs::Sedimentation: return "sedimentation coefficient";
      case Obs::EtaInf:        return "intrinsic viscosity (infinite frequency)";
      case Obs::EtaZero:       return "intrinsic viscosity (zero frequency)";
   }
   return "?";
}

inline double obs_value( const Results& r, Obs o ) {
   switch ( o ) {
      case Obs::Dt:            return r.translational_diffusion_centre;
      case Obs::Dr:            return r.rotational_diffusion;
      case Obs::Sedimentation: return r.sedimentation;
      case Obs::EtaInf:        return r.intrinsic_viscosity_high;
      case Obs::EtaZero:       return r.intrinsic_viscosity_zero;
   }
   return 0.0;
}

}  // namespace grpy
