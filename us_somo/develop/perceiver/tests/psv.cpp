// Tests for the Durchschlag & Zipper psv engine.
//
// Three layers:
//   1. arithmetic -- the increment constants must reproduce the published GROUP increments
//      (DZ94 Table 4), which is an independent check that the atomic table was transcribed right
//   2. synthetic molecules with real coordinates -- exercises the whole perceive -> psv path on
//      chemistry whose published calculated volume is known exactly, especially the topological
//      hydroxyl rule that a running count would get wrong
//   3. real residues from the demo structures against somo.residue's own vbar
//   build+run:  make psv
#include <cmath>
#include <cstdio>
#include <map>
#include <string>
#include <vector>

#include "../../include/us_hydrodyn_psv.h"
#include "../pdb_lite.h"
#include "../tinytest.h"

using namespace somo_perceive;
using namespace somo_psv;

// ---------------------------------------------------------------- layer 1: arithmetic
TEST("increments reproduce the published GROUP increments (DZ94 Table 4)") {
    using namespace somo_psv::inc;
    struct G { const char* name; double got, want; };
    const G g[] = {
        // the peptide group is the one that pins the amide N at 2.0 rather than Table 1's 4.0
        {"peptide -N(H)-C(H)-C(=O)-", N_AMIDE + 2 * C + 2 * H + O_CARBONYL, 33.5},
        {"guanidinium -C(=NH2)NH2",   C + 2 * N_GUANIDINIUM + 4 * H,        38.3},
        {"-SH",                       S + H,                                18.6},
        {"-C(=O)OH",                  C + O_CARBONYL + O_ACID + H,          18.9},
        {"-CH2-",                     C + 2 * H,                            16.1},
        {"-CH3",                      C + 3 * H,                            19.2},
        {"-OH (1st)",                 O_OH_FIRST + H,                        5.4},
    };
    for (const G& x : g) {
        if (std::fabs(x.got - x.want) > 0.05)
            std::printf("  %-26s got %.2f want %.2f\n", x.name, x.got, x.want);
        CHECK(std::fabs(x.got - x.want) < 0.05);
    }
}

TEST("ring decrements match Table 1, with >=9 sharing the 9-membered value") {
    CHECK(std::fabs(inc::ring_decrement(3) - 2.1) < 1e-9);
    CHECK(std::fabs(inc::ring_decrement(4) - 4.1) < 1e-9);
    CHECK(std::fabs(inc::ring_decrement(5) - 6.1) < 1e-9);
    CHECK(std::fabs(inc::ring_decrement(6) - 8.1) < 1e-9);
    CHECK(std::fabs(inc::ring_decrement(7) - 10.1) < 1e-9);
    CHECK(std::fabs(inc::ring_decrement(8) - 12.1) < 1e-9);
    CHECK(std::fabs(inc::ring_decrement(9) - 14.1) < 1e-9);
    CHECK(std::fabs(inc::ring_decrement(15) - 14.1) < 1e-9);
    CHECK(inc::ring_decrement(2) == 0.0);
}

// ---------------------------------------------------------------- layer 2: synthetic molecules
static InAtom at(const char* el, const char* nm, double x, double y, double z) {
    InAtom a;
    a.element = el; a.name = nm; a.resName = "LIG"; a.chain = "A"; a.resSeq = 1;
    a.x = x; a.y = y; a.z = z;
    return a;
}

static Result run(const std::vector<InAtom>& mol, bool free_molecule = true) {
    static HybridTable tbl;
    static bool loaded = tbl.load("../../etc/somo.hybrid.new");
    if (!loaded) return Result{};
    Bonds b;
    std::vector<OutAtom> p = Perceiver(tbl).perceive(mol, b);
    std::vector<int> idx;
    for (size_t i = 0; i < mol.size(); ++i) idx.push_back((int) i);
    Options o;
    o.free_molecule = free_molecule;
    return compute(mol, p, b, idx, o);
}

// Zigzag carbon chain (C-C 1.53 A) with hydroxyl oxygens 1.43 A off alternating faces.
// The alternation matters: putting them all on one side leaves adjacent oxygens 1.5 A apart,
// which the bond perceiver quite correctly reads as an O-O bond, turning the molecule into a
// spurious ring. Real polyols keep neighbouring hydroxyl oxygens near 2.9 A.
static std::vector<InAtom> diol(int n_carbon, std::vector<int> oh_on) {
    std::vector<InAtom> m;
    auto cy = [](int i) { return (i % 2) ? 0.44 : -0.44; };
    for (int i = 0; i < n_carbon; ++i)
        m.push_back(at("C", ("C" + std::to_string(i)).c_str(), 1.25 * i, cy(i), 0));
    for (int c : oh_on)
        m.push_back(at("O", ("O" + std::to_string(c)).c_str(),
                       1.25 * c, cy(c), (c % 2) ? -1.43 : 1.43));
    return m;
}

// The rule is "2nd or further NEIGHBOURING OH = 0.4"; an isolated hydroxyl is a 1st OH at 2.3.
// Both published values below come from DZ94 Table 5 and a running count reproduces neither.
TEST("hydroxyl rule is topological: adjacent OH share a cluster, isolated ones do not") {
    const Result eth = run(diol(2, {0, 1}));          // 1,2-ethanediol, adjacent
    std::printf("  1,2-ethanediol  V %.1f (published calc 53.5)  vbar %.3f\n",
                eth.molar_volume, eth.vbar);
    CHECK(eth.ok);
    CHECK(std::fabs(eth.molar_volume - 53.5) < 0.15);

    const Result oct = run(diol(8, {0, 7}));          // 1,8-octanediol, isolated
    std::printf("  1,8-octanediol  V %.1f (published calc 152.0) vbar %.3f\n",
                oct.molar_volume, oct.vbar);
    CHECK(oct.ok);
    CHECK(std::fabs(oct.molar_volume - 152.0) < 0.15);
}

TEST("glycerol: three OH in one neighbouring cluster") {
    const Result g = run(diol(3, {0, 1, 2}));
    std::printf("  glycerol V %.1f (published calc 70.0)  vbar %.3f (measured 0.767-0.772)\n",
                g.molar_volume, g.vbar);
    CHECK(std::fabs(g.molar_volume - 70.0) < 0.15);
    CHECK(std::fabs(g.vbar - 0.760) < 0.005);
}

TEST("urea: amide nitrogens and a carbonyl") {
    // O=C(N)(N), planar
    std::vector<InAtom> m{at("C", "C", 0, 0, 0), at("O", "O", 0, 1.23, 0),
                          at("N", "N1", -1.16, -0.67, 0), at("N", "N2", 1.16, -0.67, 0)};
    const Result u = run(m);
    std::printf("  urea V %.1f (published calc 44.2)  vbar %.3f (measured 0.735-0.745)\n",
                u.molar_volume, u.vbar);
    CHECK(std::fabs(u.molar_volume - 44.2) < 0.15);
}

TEST("covolume is charged once per molecule and only when asked") {
    const Result freem = run(diol(3, {0, 1, 2}), true);
    const Result resid = run(diol(3, {0, 1, 2}), false);
    std::printf("  free %.1f  as-residue %.1f  difference %.1f (covolume 12.4)\n",
                freem.molar_volume, resid.molar_volume,
                freem.molar_volume - resid.molar_volume);
    CHECK(std::fabs((freem.molar_volume - resid.molar_volume) - inc::COVOLUME) < 1e-6);
    CHECK(resid.covolume == 0.0);
}

TEST("an element with no published increment is reported for review, not guessed") {
    // Tungsten is in neither DZ94 Table 2 nor Table 3.
    std::vector<InAtom> m{at("C", "C", 0, 0, 0), at("W", "W", 2.0, 0, 0)};
    const Result f = run(m);
    CHECK(!f.review.empty());
}

// Metals DO have published volumes -- Table 2 for an atom in a complex, Table 3 for a free
// aqueous ion -- so "no increment for metals" was wrong. An aqueous Fe3+ occupies NEGATIVE
// apparent volume, -55.1 cm^3/mol, because electrostriction pulls water in around it harder
// than the ion itself displaces.
TEST("metal volumes come from the published tables") {
    CHECK(std::fabs(inc::metal_atomic_volume("FE") - 7.1) < 1e-9);
    CHECK(std::fabs(inc::metal_atomic_volume("CO") - 6.6) < 1e-9);
    CHECK(std::fabs(inc::metal_atomic_volume("ZN") - 9.2) < 1e-9);
    CHECK(inc::metal_atomic_volume("W") < 0);              // genuinely absent

    bool found = false;
    CHECK(std::fabs(inc::ion_volume("FE", 3, found) + 55.1) < 1e-9);
    CHECK(found);
    CHECK(std::fabs(inc::ion_volume("FE", 2, found) + 32.3) < 1e-9);
    CHECK(std::fabs(inc::ion_volume("MG", 2, found) + 28.8) < 1e-9);
    CHECK(std::fabs(inc::ion_volume("CL", -1, found) - 21.6) < 1e-9);
    inc::ion_volume("W", 6, found);
    CHECK(!found);
    // Sanity on the magnitude Mattia quoted: -0.78 mL/g for Fe3+ is -43.6 cm^3/mol, the same
    // sign and order as D&Z's -55.1 from a different compilation.
    std::printf("  Fe3+ : D&Z %.1f cm^3/mol = %.2f mL/g   (quoted elsewhere: -0.78 mL/g)\n",
                inc::ion_volume("FE", 3, found), inc::ion_volume("FE", 3, found) / 55.845);
}

// A free ion's Table 3 value already contains its ionisation, so electrostriction must not be
// charged again on top of it.
TEST("a free ion is not electrostricted twice") {
    std::vector<InAtom> m{at("MG", "MG", 0, 0, 0)};
    const Result f = run(m, false);
    std::printf("  isolated Mg: V %.1f  electrostriction %.1f (must be 0)\n",
                f.molar_volume, f.electrostriction);
    CHECK(f.electrostriction == 0.0);
}

// ---------------------------------------------------------------- layer 3: real residues
struct ResRef { const char* res; double vbar; };

// vbar as stored in somo.residue (Cohn & Edsall minus the 0.002 25->20 C shift).
static const std::vector<ResRef> STORED = {
    {"ALA",0.738},{"ARG",0.698},{"ASN",0.618},{"ASP",0.603},{"CYS",0.629},{"GLN",0.668},
    {"GLU",0.663},{"GLY",0.638},{"HIS",0.668},{"ILE",0.898},{"LEU",0.898},{"LYS",0.818},
    {"MET",0.748},{"PHE",0.768},{"PRO",0.758},{"SER",0.628},{"THR",0.698},{"TRP",0.738},
    {"TYR",0.708},{"VAL",0.858},
};
// Residues whose stored charge state is a convention rather than perceived chemistry, so a
// disagreement here is a protonation-policy question and not an increment error.
static bool charge_sensitive(const std::string& r) {
    return r == "ARG" || r == "LYS" || r == "HIS" || r == "ASP" || r == "GLU";
}

TEST("computed vbar reproduces somo.residue for the coded amino acids") {
    HybridTable tbl;
    if (!tbl.load("../../etc/somo.hybrid.new")) { std::printf("  (no hybrid table)\n"); return; }
    std::vector<PdbAtom> atoms = strip_altlocs(read_pdb("data/1HEL.pdb"));
    if (atoms.empty()) { std::printf("  (data/1HEL.pdb absent -- skipped)\n"); return; }

    std::vector<InAtom> in;
    for (const PdbAtom& a : atoms) {
        InAtom x;
        x.element = a.element; x.x = a.x; x.y = a.y; x.z = a.z;
        x.serial = a.serial; x.name = a.name; x.resName = a.resName;
        x.chain = std::string(1, a.chain); x.resSeq = a.resSeq; x.hetatm = a.hetatm;
        in.push_back(x);
    }
    Bonds b;
    std::vector<OutAtom> p = Perceiver(tbl).perceive(in, b);

    std::map<std::string, std::vector<int>> res;      // key -> atom indices
    for (size_t i = 0; i < in.size(); ++i) {
        if (in[i].resName == "HOH") continue;
        res[in[i].resName + "|" + in[i].chain + std::to_string(in[i].resSeq)].push_back((int) i);
    }

    std::map<std::string, std::vector<double>> got;
    for (auto& kv : res) {
        const std::string rn = kv.first.substr(0, kv.first.find('|'));
        // skip chain termini: an extra OXT makes the residue a different molecule
        bool oxt = false;
        for (int i : kv.second) if (in[i].name == "OXT") oxt = true;
        if (oxt) continue;
        Result r = compute(in, p, b, kv.second);       // residue basis: no covolume
        if (r.ok) got[rn].push_back(r.vbar);
    }

    std::printf("  %-5s %8s %8s %7s\n", "res", "computed", "stored", "diff%");
    double sum_clean = 0; int n_clean = 0;
    for (const ResRef& x : STORED) {
        auto it = got.find(x.res);
        if (it == got.end() || it->second.empty()) continue;
        double m = 0;
        for (double v : it->second) m += v;
        m /= it->second.size();
        const double d = 100.0 * (m - x.vbar) / x.vbar;
        std::printf("  %-5s %8.3f %8.3f %+7.1f%s\n", x.res, m, x.vbar, d,
                    charge_sensitive(x.res) ? "   (charge-state convention)" : "");
        if (!charge_sensitive(x.res)) {
            sum_clean += std::fabs(d);
            ++n_clean;
            CHECK(std::fabs(d) < 6.0);
        } else {
            // Wider, because two effects stack. somo.residue stores Arg and His protonated but
            // the perceiver assigns them no formal charge (only Lys NZ gets one), so an
            // electrostriction term is missing; and Arg is the known outlier of the increment
            // scheme -- the six published residue-volume sets in Perkins 1986 Table 1 disagree
            // with each other about it by 17%. Tightening this needs the pH layer, not a
            // different increment.
            CHECK(std::fabs(d) < 20.0);
        }
    }
    if (n_clean) {
        const double mad = sum_clean / n_clean;
        std::printf("  mean |error| over %d charge-neutral residues: %.2f%%\n", n_clean, mad);
        CHECK(mad < 3.0);
    }
}

// Does assuming pH 7 ionisation improve psv? Measure, do not assume. The hydration rules made a
// large difference by charging these same groups, so the symmetry is tempting -- but
// somo.residue lists an identical vbar for the protonated and deprotonated state of every
// ionisable residue, which says its tabulated values already absorb whatever ionisation the
// underlying Cohn-Edsall measurements carried. Adding D&Z electrostriction on top would
// double-count it.
TEST("pH 7 electrostriction: measured, not assumed") {
    HybridTable tbl;
    if (!tbl.load("../../etc/somo.hybrid.new")) return;
    std::vector<PdbAtom> atoms = strip_altlocs(read_pdb("data/1HEL.pdb"));
    if (atoms.empty()) { std::printf("  (data absent -- skipped)\n"); return; }
    std::vector<InAtom> in;
    for (const PdbAtom& a : atoms) {
        if (a.resName == "HOH") continue;
        InAtom x;
        x.element = a.element; x.x = a.x; x.y = a.y; x.z = a.z;
        x.serial = a.serial; x.name = a.name; x.resName = a.resName;
        x.chain = std::string(1, a.chain); x.resSeq = a.resSeq; x.hetatm = a.hetatm;
        in.push_back(x);
    }
    Bonds b;
    std::vector<OutAtom> p = Perceiver(tbl).perceive(in, b);
    std::map<std::string, std::vector<int>> res;
    for (size_t i = 0; i < in.size(); ++i)
        res[in[i].resName + "|" + std::to_string(in[i].resSeq)].push_back((int) i);

    struct T { const char* res; double stored; };
    const T want[] = {{"ASP",0.603},{"GLU",0.663},{"ARG",0.698},{"LYS",0.818}};
    double off_sum = 0, on_sum = 0; int n = 0;
    std::printf("  %-5s %9s %9s %9s\n", "res", "stored", "neutral", "pH7 ions");
    for (const T& w : want) {
        for (const auto& kv : res) {
            if (kv.first.substr(0, kv.first.find('|')) != w.res) continue;
            bool term = false;
            for (int i : kv.second) {
                if (in[i].name == "OXT") term = true;
                if (in[i].name == "N" && p[i].formal_charge > 0) term = true;
            }
            if (term) continue;
            Options o_off, o_on;
            o_on.assume_ph7_ionization = true;
            const double v_off = compute(in, p, b, kv.second, o_off).vbar;
            const double v_on  = compute(in, p, b, kv.second, o_on ).vbar;
            const double d_off = 100.0 * (v_off - w.stored) / w.stored;
            const double d_on  = 100.0 * (v_on  - w.stored) / w.stored;
            std::printf("  %-5s %9.3f %6.3f(%+5.1f%%) %6.3f(%+5.1f%%)\n",
                        w.res, w.stored, v_off, d_off, v_on, d_on);
            off_sum += std::fabs(d_off); on_sum += std::fabs(d_on); ++n;
            break;
        }
    }
    if (!n) return;
    std::printf("  mean |error|: neutral %.2f%%, with pH 7 ionisation %.2f%%\n",
                off_sum / n, on_sum / n);
    // The default must be whichever is actually better. It is the neutral one.
    CHECK(off_sum <= on_sum);
    CHECK(!Options().assume_ph7_ionization);
}

int main() { return tinytest::run(); }
