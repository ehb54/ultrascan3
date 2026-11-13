// residue_variants.cpp -- per-residue psv under D&Z, with COMPOSITION and ELECTROSTRICTION
// separated. Answers Mattia's question (2026-08-13): what do the ionizable residues look like if
// they are computed by the D&Z additivity scheme in a FIXED protonation state, with the ionization
// (electrostriction) contribution left out?
//
//   [neutral ]  side chain fully protonated and uncharged  (COOH, NH2, SH, phenol OH, guanidine)
//   [ionized ]  side chain in its charged composition      (COO-, NH3+, S-, phenolate, guanidinium)
//   both WITHOUT the -6.8/-6.7 electrostriction decrement.
//
// The two are NOT the same calculation minus a constant: the increment table itself distinguishes
// the states (a 1st ammonium N is 12.2 against an amine's 4.0), so composition moves the atomic sum
// as well as the mass. That is exactly why this is worth computing rather than deriving by hand.
//
// Electrostriction is removed by reconstructing from the exposed components rather than by a flag,
// so the shipped psv module is untouched: molar_volume = atomic_sum + covolume - ring_decrement
// - electrostriction, and here covolume is 0 (residue, not free molecule).
//
// Usage: residue_variants <pdb> [<pdb> ...]
#include <cstdio>
#include <cmath>
#include <map>
#include <set>
#include <string>
#include <vector>
#include "../pdb_lite.h"
#include "../../include/us_hydrodyn_perceive.h"
#include "../../include/us_hydrodyn_perceive_hybrid.h"
#include "../../include/us_hydrodyn_psv.h"
using namespace somo_perceive;

static const double H_MASS = 1.008;

// The ionizable side-chain atoms of the coded residues, by name. Keyed on residue+atom rather than
// perceived type because the point here is to SET the state, not to read whichever one the
// structure happened to be deposited in.
static bool ionizable_side_chain(const std::string& res, const std::string& at) {
    if (res == "ASP") return at == "OD1" || at == "OD2";
    if (res == "GLU") return at == "OE1" || at == "OE2";
    if (res == "LYS") return at == "NZ";
    if (res == "ARG") return at == "NH1" || at == "NH2" || at == "NE";
    if (res == "CYS") return at == "SG";
    if (res == "TYR") return at == "OH";
    return false;                       // HIS handled separately: neutral at pH 7 either way
}

// Force one side-chain group into a state. Mass follows n_h so the psv denominator stays honest.
// Carboxyls carry their proton on exactly ONE of the two oxygens, so only that one is touched.
static void set_state(const std::string& res, const std::vector<InAtom>& in,
                      std::vector<OutAtom>& out, const std::vector<int>& idx, bool ionized) {
    // carboxyl: find the oxygen currently bearing the H (or the one already deprotonated)
    if (res == "ASP" || res == "GLU" || res == "CYS" || res == "TYR") {
        int target = -1;
        for (int i : idx)
            if (ionizable_side_chain(res, in[i].name) &&
                (out[i].n_h >= 1 || out[i].formal_charge < 0)) { target = i; break; }
        if (target < 0)                                  // fully deprotonated in the file
            for (int i : idx) if (ionizable_side_chain(res, in[i].name)) { target = i; break; }
        if (target < 0) return;
        OutAtom& o = out[target];
        if (ionized) { if (o.n_h > 0) { o.n_h -= 1; o.mw -= H_MASS; } o.formal_charge = -1; }
        else         { if (o.n_h == 0) { o.n_h = 1; o.mw += H_MASS; } o.formal_charge = 0; }
        return;
    }
    if (res == "LYS") {
        for (int i : idx) {
            if (in[i].name != "NZ") continue;
            OutAtom& o = out[i];
            if (ionized) { if (o.n_h < 3) { o.mw += H_MASS * (3 - o.n_h); o.n_h = 3; }
                           o.formal_charge = 1; }
            else         { if (o.n_h > 2) { o.mw -= H_MASS * (o.n_h - 2); o.n_h = 2; }
                           o.formal_charge = 0; }
        }
        return;
    }
    if (res == "ARG") {
        // One charge per GROUP, carried on NH2 by convention. Guanidinium C(NH2)2=NH2+ has five
        // N-H; neutral guanidine has four, so the neutral form must LOSE a proton and not merely
        // the charge -- the N_GUANIDINIUM increment (8.0/N) is keyed off the CN3 pattern and is
        // blind to charge, so without the proton the two states would come out identical.
        for (int i : idx) {
            if (in[i].name != "NH2") continue;
            OutAtom& o = out[i];
            if (ionized) { o.formal_charge = 1; if (o.n_h < 2) { o.mw += H_MASS * (2 - o.n_h);
                                                                 o.n_h = 2; } }
            else         { o.formal_charge = 0; if (o.n_h > 1) { o.mw -= H_MASS * (o.n_h - 1);
                                                                 o.n_h = 1; } }
        }
        return;
    }
}

struct Acc { double sum = 0; int n = 0; void add(double v){ sum += v; ++n; }
             double mean() const { return n ? sum / n : 0.0; } };

int main(int argc, char** argv) {
    if (argc < 2) { std::fprintf(stderr, "usage: residue_variants <pdb> [<pdb> ...]\n"); return 2; }
    HybridTable tbl;
    if (!tbl.load("../../etc/somo.hybrid.new")) {
        std::fprintf(stderr, "cannot load somo.hybrid.new\n"); return 2; }
    Perceiver perc(tbl);

    std::map<std::string,Acc> asis, neutral, ionized;
    for (int ai = 1; ai < argc; ++ai) {
        auto atoms = strip_altlocs(read_pdb(argv[ai]));
        if (atoms.empty()) { std::fprintf(stderr, "no atoms in %s\n", argv[ai]); continue; }
        std::vector<InAtom> in;
        for (auto& a : atoms) { InAtom x; x.element=a.element; x.x=a.x; x.y=a.y; x.z=a.z;
            x.serial=a.serial; x.name=a.name; x.resName=a.resName;
            x.chain=std::string(1,a.chain); x.resSeq=a.resSeq; x.hetatm=a.hetatm; in.push_back(x); }
        Bonds bonds;
        auto out = perc.perceive(in, bonds);

        // group atoms by residue instance
        std::map<std::string,std::vector<int>> res;
        for (size_t i = 0; i < in.size(); ++i)
            res[in[i].chain + "/" + std::to_string(in[i].resSeq) + "/" + in[i].resName]
                .push_back((int)i);

        somo_psv::Options opt; opt.free_molecule = false; opt.assume_ph7_ionization = false;
        for (auto& kv : res) {
            const std::string rn = kv.first.substr(kv.first.rfind('/') + 1);
            const auto& idx = kv.second;
            auto r0 = somo_psv::compute(in, out, bonds, idx, opt);
            if (!r0.ok || r0.mw <= 0) continue;
            asis[rn].add(r0.vbar);
            for (int pass = 0; pass < 2; ++pass) {
                std::vector<OutAtom> mod = out;                 // per-residue edit, whole-structure
                set_state(rn, in, mod, idx, pass == 1);         // bonds/geometry unchanged
                auto r = somo_psv::compute(in, mod, bonds, idx, opt);
                if (!r.ok || r.mw <= 0) continue;
                // strip the ionization contribution: covolume is already 0 for a residue
                const double v_noes = (r.atomic_sum - r.ring_decrement) / r.mw;
                (pass ? ionized : neutral)[rn].add(v_noes);
            }
        }
    }

    // PSV_DETAIL=GLU dumps one instance term by term, so a disagreement with an independent
    // implementation can be localised to an increment rather than argued about as a total.
    if (const char* want = std::getenv("PSV_DETAIL")) {
        auto atoms = strip_altlocs(read_pdb(argv[1]));
        std::vector<InAtom> in;
        for (auto& a : atoms) { InAtom x; x.element=a.element; x.x=a.x; x.y=a.y; x.z=a.z;
            x.serial=a.serial; x.name=a.name; x.resName=a.resName;
            x.chain=std::string(1,a.chain); x.resSeq=a.resSeq; x.hetatm=a.hetatm; in.push_back(x); }
        Bonds bonds; auto out = perc.perceive(in, bonds);
        std::map<std::string,std::vector<int>> res;
        for (size_t i = 0; i < in.size(); ++i)
            res[in[i].chain + "/" + std::to_string(in[i].resSeq) + "/" + in[i].resName]
                .push_back((int)i);
        for (auto& kv : res) {
            if (kv.first.substr(kv.first.rfind('/') + 1) != want) continue;
            somo_psv::Options opt; opt.free_molecule = false;
            const char* names[3] = {"as-is (as perceived)", "NEUTRAL side chain", "IONIZED side chain"};
            for (int pass = 0; pass < 3; ++pass) {
                std::vector<OutAtom> mod = out;
                if (pass) set_state(want, in, mod, kv.second, pass == 2);
                auto r = somo_psv::compute(in, mod, bonds, kv.second, opt);
                std::printf("\n=== %s  %s ===\n", kv.first.c_str(), names[pass]);
                std::printf("%-6s %-8s %3s %5s   %8s\n","atom","hybrid","nH","chg","incr+H");
                double chk = 0;
                for (int i : kv.second) {
                    const OutAtom& o = mod[i];
                    // recompute this atom's contribution the same way compute() does, one atom at
                    // the running total -- printed per atom so each term can be checked by hand
                    std::vector<int> one{ i };
                    auto r1 = somo_psv::compute(in, mod, bonds, one, opt);
                    chk += r1.atomic_sum;
                    std::printf("%-6s %-8s %3d %5d   %8.2f\n", in[i].name.c_str(),
                                o.hybrid.c_str(), o.n_h, o.formal_charge, r1.atomic_sum);
                }
                std::printf("  atomic sum %8.2f   rings -%.2f   electrostriction -%.2f "
                            "(n+ %d, n- %d)\n", r.atomic_sum, r.ring_decrement,
                            r.electrostriction, r.n_pos, r.n_neg);
                std::printf("  V = %.2f cm3/mol   MW = %.3f   vbar = %.4f  (@20C %.4f)\n",
                            r.molar_volume, r.mw, r.vbar, r.vbar - 0.002);
                std::printf("  without electrostriction: V = %.2f  vbar = %.4f  (@20C %.4f)\n",
                            r.atomic_sum - r.ring_decrement,
                            (r.atomic_sum - r.ring_decrement) / r.mw,
                            (r.atomic_sum - r.ring_decrement) / r.mw - 0.002);
                (void)chk;
            }
            break;
        }
        return 0;
    }

    static const char* ORDER[] = {"GLY","ALA","VAL","LEU","ILE","SER","THR","CYS","MET","PRO",
                                  "PHE","TYR","TRP","ASP","ASN","GLU","GLN","LYS","ARG","HIS"};
    std::printf("%-5s %7s  %9s %9s %9s   %s\n",
                "res", "n", "as-is", "neutral", "ionized", "(no electrostriction in the last two)");
    for (const char* rn : ORDER) {
        auto a = asis.find(rn); if (a == asis.end()) continue;
        std::printf("%-5s %7d  %9.4f %9.4f %9.4f\n", rn, a->second.n, a->second.mean(),
                    neutral.count(rn) ? neutral[rn].mean() : a->second.mean(),
                    ionized.count(rn) ? ionized[rn].mean() : a->second.mean());
    }
    // override tokens, ready to paste into protein_psv
    for (int pass = 0; pass < 2; ++pass) {
        std::printf("\n%s override tokens:\n", pass ? "IONIZED (no ES)" : "NEUTRAL (no ES)");
        auto& m = pass ? ionized : neutral;
        for (const char* rn : ORDER) {
            auto a = asis.find(rn); if (a == asis.end()) continue;
            std::printf("%s=%.4f ", rn, m.count(rn) ? m[rn].mean() : a->second.mean());
        }
        std::printf("\n");
    }
    return 0;
}
