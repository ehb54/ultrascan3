#ifndef US_HYDRODYN_PSV_H
#define US_HYDRODYN_PSV_H

// Partial specific volume (psv / vbar) from published atomic volume increments, Qt-free.
//
// Method: Durchschlag & Zipper's universal approach, built on Traube's additivity principle.
//   H. Durchschlag & P. Zipper, Prog. Colloid Polym. Sci. 94 (1994) 20-39   [tables 1-5]
//   H. Durchschlag & P. Zipper, J. Appl. Cryst. 30 (1997) 803-807 + SUP84592
//
//   V = SUM(V_i) + V_CV - SUM(V_RF) - SUM(V_ES)
//
// V_i        per-atom increment, depending on the atom's chemical environment
// V_CV       Traube covolume, 12.4 cm^3/mol, once per MOLECULE
// V_RF       ring-formation decrement, once per RING, by ring size
// V_ES       electrostriction, per formal charge
//
// vbar = V / M.  Increments are for aqueous solution at 25 C.
//
// COVOLUME POLICY. For a monomeric unit inside a polymer the covolume belongs to the assembled
// chain, not to each unit (DZ94 sec 2.3), so it is omitted by default -- which is also what
// SOMO needs, since somo.residue entries are summed and US_Hydrodyn::calc_vbar_updated adds a
// single structure-level covolume from gparams["covolume"] (12.4). Set Options::free_molecule
// when computing a standalone small molecule, e.g. to compare against a measured value.
//
// RINGS. Only rings lying entirely within the residue are charged. That is correct chemistry --
// the decrement is a property of the molecule -- and it also makes the result immune to spurious
// cross-residue rings arising from clashing coordinates in deposited structures.
//
// ACCURACY. Reproduces the papers' own worked values exactly (see tests/psv.cpp). Against
// measured values for non-amino-acid chemistry: mean |error| 2.5%, 10 of 11 within 3%,
// consistent with D&Z's own validation over ~500 compounds (75% within 2%, 90% within 3%).
// Poorer for lipids and ionic detergents, which the authors flag; metal and Fe-S clusters are
// outside any additive scheme and need measured values.

#include <string>
#include <vector>

#include "us_hydrodyn_perceive.h"

namespace somo_psv {

struct Options {
    // Add the Traube covolume. False = a residue/monomeric unit (the SOMO case); true = a
    // standalone molecule.
    bool free_molecule = false;

    // Charge the ionisable groups as they stand at pH 7 -- carboxyl as carboxylate, guanidinium
    // protonated -- and apply an electrostriction term for each, in the same spirit as the pH 7
    // hydration rules.
    //
    // OFF by default, and the measurement is why. somo.residue lists the SAME vbar for the
    // protonated and deprotonated states of every ionisable residue (Asp 0.603/0.603,
    // Glu 0.663/0.663, Arg 0.698/0.698 ...), i.e. SOMO's tabulated values already absorb
    // whatever ionisation the underlying Cohn-Edsall measurements had. Adding an explicit D&Z
    // electrostriction on top double-counts it: see tests/psv.cpp, where switching this on moves
    // aspartate and glutamate several percent further from the table than leaving it off.
    bool assume_ph7_ionization = false;
};

struct Result {
    double molar_volume = 0;    // cm^3/mol
    double vbar = 0;            // cm^3/g
    double mw = 0;              // g/mol, from the perceived atoms

    // Decomposition, so a value can be audited rather than taken on trust.
    double atomic_sum = 0;
    double covolume = 0;
    double ring_decrement = 0;
    double electrostriction = 0;
    int    n_rings = 0;
    int    n_pos = 0, n_neg = 0;

    // Atoms whose environment could not be classified confidently. Non-empty means the caller
    // should surface the value for review rather than use it silently.
    std::vector<std::string> review;
    bool ok = false;
};

// atoms/perceived/bonds describe the whole structure; idx lists the atom indices forming the
// residue of interest. Passing every index treats the input as a single molecule.
Result compute(const std::vector<somo_perceive::InAtom>& atoms,
               const std::vector<somo_perceive::OutAtom>& perceived,
               const somo_perceive::Bonds& bonds,
               const std::vector<int>& idx,
               const Options& opt = {});

// ---- increments, exposed so tests can assert against the published tables ------------------
namespace inc {
constexpr double C = 9.9, H = 3.1, S = 15.5, F = 5.0, CL = 15.0, BR = 19.7, I = 32.1;
constexpr double P3 = 17.0, P5 = 28.5;

constexpr double O_CARBONYL = 5.5;   // =O, ether, ester, anhydride, ring O, amine oxide, nitro
constexpr double O_OH_FIRST = 2.3;   // 1st OH; O- in a phenolate
constexpr double O_OH_NEXT  = 0.4;   // 2nd or further NEIGHBOURING OH
constexpr double O_ACID     = 0.4;   // OH or O- in carboxyl, phosphate, sulfate, sulfonate
constexpr double O_RING_2DO = 0.9;   // 2nd =O at a ring where tautomerism is possible

constexpr double N_AMIDE       = 2.0;   // amide N, incl. the peptide backbone (see note below)
constexpr double N_AMINE       = 4.0;   // primary/secondary/tertiary monoamines, 2nd ammonium
constexpr double N_AROMATIC    = 7.0;   // N or NH in aromatic rings; nitro, nitrile
constexpr double N_GUANIDINIUM = 8.0;   // per N of a guanidinium group
constexpr double N_AMMONIUM_1  = 12.2;  // 1st ammonium N; N in betaines

constexpr double COVOLUME = 12.4;
constexpr double ES_POS = 6.8, ES_NEG = 6.7;

// Ring-formation decrement by ring size; sizes >= 9 all take the 9-membered value.
double ring_decrement(int ring_size);

// Metal ATOMIC volume, DZ94 Table 2 -- for a metal bonded into a complex, e.g. the iron of a
// haem or the cobalt of B12. Returns <0 if the element is not tabulated.
double metal_atomic_volume(const std::string& element);

// Monatomic ION volume, DZ94 Table 3, keyed "FE+2", "MG+2", "CL-1" ... These ALREADY contain
// the ionisation contribution, so no electrostriction term is added on top of them.
// Returns a sentinel via `found` because legitimate values are negative -- an aqueous Fe3+ is
// -55.1 cm^3/mol, water pulled in so tightly that the ion occupies negative apparent volume.
double ion_volume(const std::string& element, int charge, bool& found);
} // namespace inc

} // namespace somo_psv

#endif
