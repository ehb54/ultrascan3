// End-to-end: a complete somo.residue entry for a residue treated as non-coded.
// The individual stages are validated in psv.cpp, grid_volume.cpp and hydration.cpp; this checks
// that the orchestration produces a populated entry, that every unset field is visibly unset,
// and that the emitted numbers are the ones the stages computed.
//   build+run:  make builder
#include <cmath>
#include <cstdio>
#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "../../include/us_hydrodyn_residue_builder.h"
#include "../pdb_lite.h"
#include "../tinytest.h"

using namespace somo_perceive;
using namespace somo_residue_builder;

static std::vector<std::pair<std::string, double>> hydration_obs(const std::string& path) {
    std::vector<std::pair<std::string, double>> obs;
    std::ifstream f(path);
    std::vector<std::string> L;
    std::string ln;
    while (std::getline(f, ln)) L.push_back(ln);
    std::set<std::string> seen;
    size_t i = 0;
    while (i < L.size()) {
        ++i;
        if (i >= L.size()) break;
        std::istringstream hs(L[i]);
        std::string name;
        int type = 0, natoms = 0, nbeads = 0;
        double molvol = 0, mw = 0, vbar = 0;
        if (!(hs >> name >> type >> molvol >> mw >> natoms >> nbeads >> vbar)) continue;
        ++i;
        const bool first = seen.insert(name).second;
        for (int a = 0; a < natoms && i + a < L.size(); ++a) {
            std::vector<std::string> f2;
            std::string cur;
            std::istringstream as(L[i + a]);
            while (std::getline(as, cur, '\t')) f2.push_back(cur);
            if (f2.size() < 8) continue;
            try { if (first) obs.push_back({f2[1], std::stod(f2[7])}); } catch (...) {}
        }
        i += natoms + (nbeads > 0 ? nbeads : 0);
    }
    return obs;
}

struct Ctx {
    HybridTable tbl;
    std::vector<InAtom> in;
    std::vector<OutAtom> perc;
    Bonds bonds;
    somo_hydration::Table hyd;
    std::map<std::string, std::vector<int>> res;
    bool ok = false;
};
static Ctx g;

static bool load() {
    if (g.ok) return true;
    if (!g.tbl.load("../../etc/somo.hybrid.new")) return false;
    std::vector<PdbAtom> atoms = strip_altlocs(read_pdb("data/1HEL.pdb"));
    if (atoms.empty()) return false;
    for (const PdbAtom& a : atoms) {
        if (a.resName == "HOH") continue;
        InAtom x;
        x.element = a.element; x.x = a.x; x.y = a.y; x.z = a.z;
        x.serial = a.serial; x.name = a.name; x.resName = a.resName;
        x.chain = std::string(1, a.chain); x.resSeq = a.resSeq; x.hetatm = a.hetatm;
        g.in.push_back(x);
    }
    g.perc = Perceiver(g.tbl).perceive(g.in, g.bonds);
    g.hyd = somo_hydration::Table::from_observations(hydration_obs("../../etc/somo.residue.new"));
    for (size_t i = 0; i < g.in.size(); ++i)
        g.res[g.in[i].resName + "|" + std::to_string(g.in[i].resSeq)].push_back((int) i);
    g.ok = true;
    return true;
}

// Pick one complete INTERNAL instance of a residue type. Both chain ends must be excluded:
// a C-terminus carries an extra OXT, and an N-terminus has a free ammonium backbone nitrogen
// rather than an amide, which is different chemistry from the tabulated entry (an N-terminal
// lysine genuinely has two ammonium groups and so twice the hydration).
static std::vector<int> pick(const std::string& rn, size_t natoms) {
    for (const auto& kv : g.res) {
        if (kv.first.substr(0, kv.first.find('|')) != rn) continue;
        if (kv.second.size() != natoms) continue;
        bool terminal = false;
        for (int i : kv.second) {
            if (g.in[i].name == "OXT") terminal = true;
            if (g.in[i].name == "N" && g.perc[i].formal_charge > 0) terminal = true;
        }
        if (!terminal) return kv.second;
    }
    return {};
}

TEST("a complete entry is produced, with every field populated") {
    if (!load()) { std::printf("  (data or tables absent -- skipped)\n"); return; }
    std::vector<int> idx = pick("TRP", 14);
    if (idx.empty()) { std::printf("  (no complete TRP -- skipped)\n"); return; }

    Built b = build("TRP", g.in, g.perc, g.bonds, idx, Perceiver(g.tbl), &g.hyd, "L-tryptophan");
    CHECK(b.ok);
    std::printf("  vbar %.3f (stored 0.738)   raw volume %.1f -> molvol %.1f A^3   hydration %.1f\n",
                b.psv.vbar, b.raw_volume, b.molvol, b.hydration.total);
    std::printf("  psv decomposition: atoms %.1f  rings -%.1f (%d)  electrostriction -%.1f\n",
                b.psv.atomic_sum, b.psv.ring_decrement, b.psv.n_rings, b.psv.electrostriction);

    CHECK(b.psv.ok && b.psv.vbar > 0.5 && b.psv.vbar < 1.0);
    CHECK(b.psv.n_rings == 2);                    // indole is fused: 5 + 6
    CHECK(b.molvol > 0 && b.raw_volume > 0);
    CHECK(b.molvol > b.raw_volume);               // the convention factor is applied
    CHECK(b.hydration.ok);

    // the header must state provenance, not the old "deliberately not estimated" text
    CHECK(b.residue_block.find("Durchschlag") != std::string::npos);
    CHECK(b.residue_block.find("NOT SET") == std::string::npos);
}

TEST("emitted numbers are the ones the stages computed") {
    if (!load()) return;
    std::vector<int> idx = pick("PHE", 11);
    if (idx.empty()) { std::printf("  (no complete PHE -- skipped)\n"); return; }
    Built b = build("PHE", g.in, g.perc, g.bonds, idx, Perceiver(g.tbl), &g.hyd);

    // header line: name, type, molvol, ASA, natoms, nbeads, vbar
    std::istringstream ls(b.residue_block);
    std::string ln, header;
    while (std::getline(ls, ln)) if (!ln.empty() && ln[0] != '#') { header = ln; break; }
    std::istringstream hs(header);
    std::string nm;
    int type = 0, natoms = 0, nbeads = 0;
    double molvol = 0, asa = 0, vbar = 0;
    hs >> nm >> type >> molvol >> asa >> natoms >> nbeads >> vbar;
    std::printf("  header: %s molvol %.1f asa %.1f natoms %d nbeads %d vbar %.3f\n",
                nm.c_str(), molvol, asa, natoms, nbeads, vbar);
    CHECK(nm == "PHE");
    CHECK(natoms == 11);
    CHECK(nbeads == 1);
    CHECK(std::fabs(molvol - b.molvol) < 0.5);
    CHECK(std::fabs(vbar - b.psv.vbar) < 0.001);
    // ASA MUST be positive. Every SOMO residue loader skips a record whose ASA is zero, and skips
    // it silently -- the entry parses, never enters residue_list or multi_residue_map, and the
    // residue goes on being reported as non-coded no matter what else is right. This emitted 0
    // for a long time and nothing noticed, because nothing had ever loaded a generated entry.
    CHECK(asa > 0);
    CHECK(molvol > 0);                            // the same guard covers molvol
}

TEST("the single bead carries the molvol, a selectable colour, and the summed hydration") {
    if (!load()) return;
    std::vector<int> idx = pick("SER", 6);
    if (idx.empty()) { std::printf("  (no complete SER -- skipped)\n"); return; }
    Built b = build("SER", g.in, g.perc, g.bonds, idx, Perceiver(g.tbl), &g.hyd);

    std::istringstream ls(b.residue_block);
    std::string ln, last;
    while (std::getline(ls, ln)) if (!ln.empty() && ln[0] != '#') last = ln;
    std::istringstream bs(last);
    double hydr = 0, vol = 0;
    int colour = 0, placing = 0, chain = 0;
    bs >> hydr >> colour >> placing >> chain >> vol;
    std::printf("  bead: hydration %.1f colour %d placing %d chain %d volume %.1f\n",
                hydr, colour, placing, chain, vol);
    CHECK(colour == DEFAULT_BEAD_COLOR);
    CHECK(bead_color_is_selectable(colour));      // never a reserved colour
    CHECK(std::fabs(vol - b.molvol) < 0.5);
    CHECK(std::fabs(hydr - b.hydration.total) < 0.01);
}

TEST("with the stages disabled, unset fields are visibly unset") {
    if (!load()) return;
    std::vector<int> idx = pick("ALA", 5);
    if (idx.empty()) return;
    Options o;
    o.compute_psv = false;
    o.compute_volume = false;
    o.compute_hydration = false;
    Built b = build("ALA", g.in, g.perc, g.bonds, idx, Perceiver(g.tbl), nullptr, "", o);
    CHECK(b.ok);
    CHECK(b.residue_block.find("NOT SET") != std::string::npos);
    CHECK(b.molvol == 0);
}

TEST("uncertain hydration types reach the REVIEW block") {
    if (!load()) return;
    // Ser carries OG (O2H1), the weakest type in the table: 55% agreement over 121 observations.
    std::vector<int> idx = pick("SER", 6);
    if (idx.empty()) return;
    Built b = build("SER", g.in, g.perc, g.bonds, idx, Perceiver(g.tbl), &g.hyd);
    const bool flagged = b.residue_block.find("REVIEW") != std::string::npos;
    std::printf("  SER entry has a REVIEW block: %s (%zu hydration flags)\n",
                flagged ? "yes" : "no", b.hydration.review.size());
    CHECK(!b.hydration.review.empty());
    CHECK(flagged);
}

// The convention factor exists to put a computed volume on the same footing as the stored ones,
// so check it does: treat coded residues as unknown, build them, and compare against
// somo.residue's own molvol. This is the whole molvol claim in one test.
TEST("computed molvol reproduces the stored volumes") {
    if (!load()) return;
    struct S { const char* res; size_t natoms; double stored; };
    const S want[] = {{"GLY",4,64.9},{"ALA",5,90.0},{"SER",6,95.4},{"THR",7,121.5},
                      {"PRO",7,122.9},{"VAL",7,139.0},{"LEU",8,164.0},{"ILE",8,163.9},
                      {"MET",8,167.0},{"PHE",11,191.9},{"TYR",12,197.0},{"TRP",14,228.2}};
    double sum = 0; int n = 0;
    std::printf("  %-5s %9s %9s %7s\n", "res", "computed", "stored", "diff%");
    for (const S& w : want) {
        std::vector<int> idx = pick(w.res, w.natoms);
        if (idx.empty()) continue;
        Built b = build(w.res, g.in, g.perc, g.bonds, idx, Perceiver(g.tbl), &g.hyd);
        if (!b.ok || b.molvol <= 0) continue;
        const double d = 100.0 * (b.molvol - w.stored) / w.stored;
        std::printf("  %-5s %9.1f %9.1f %+7.1f\n", w.res, b.molvol, w.stored, d);
        sum += std::fabs(d); ++n;
        CHECK(std::fabs(d) < 6.0);
    }
    if (n) {
        std::printf("  mean |error| over %d residues: %.2f%%\n", n, sum / n);
        CHECK(sum / n < 4.0);
    }
}

// The pH 7 hydration rules (Mattia, 2026-08-08) must reproduce what somo.residue stores at
// pH 7 for the coded residues -- which is the only ground truth available. The values below are
// the runtime, pH-adjusted totals: aspartate's carboxylate carries 5 waters and glutamate's 6,
// the difference being the extra methylene, and Lys's ammonium 3.
TEST("pH 7 hydration rules reproduce the coded residues") {
    if (!load()) return;
    struct H { const char* res; size_t natoms; double total; const char* why; };
    const H want[] = {
        {"ALA", 5,  1.0, "backbone amide N only"},
        {"GLY", 4,  1.0, "backbone amide N only"},
        {"SER", 6,  2.0, "backbone + aliphatic hydroxyl"},
        {"THR", 7,  2.0, "backbone + aliphatic hydroxyl"},
        {"TYR",12,  1.0, "phenolic hydroxyl is NOT hydrated"},
        {"CYS", 6,  1.0, "thiol/disulfide S is not hydrated"},
        {"MET", 8,  2.0, "thioether S is"},
        {"PRO", 7,  1.0, "tertiary ring amide N, no H, still hydrated"},
        {"ASN", 8,  2.0, "backbone + amide NH2"},
        {"GLN", 9,  2.0, "backbone + amide NH2"},
        {"TRP",14,  2.0, "backbone + aromatic ring NH"},
        {"ASP", 8,  6.0, "carboxylate 5 (one methylene) + backbone"},
        {"GLU", 9,  7.0, "carboxylate 6 (two methylenes) + backbone"},
        {"ARG",11,  3.0, "backbone + two terminal guanidinium N, internal one not hydrated"},
        {"LYS", 9,  4.0, "backbone + ammonium 3"},
    };
    int ok = 0, n = 0;
    std::printf("  %-5s %9s %9s   %s\n", "res", "computed", "stored", "rule");
    for (const H& w : want) {
        std::vector<int> idx = pick(w.res, w.natoms);
        if (idx.empty()) continue;
        Built b = build(w.res, g.in, g.perc, g.bonds, idx, Perceiver(g.tbl), &g.hyd);
        if (!b.hydration.ok) continue;
        ++n;
        const double got = b.hydration.total;
        const bool hit = std::fabs(got - w.total) < 0.51;
        if (hit) ++ok;
        std::printf("  %-5s %9.1f %9.1f   %s%s\n", w.res, got, w.total, w.why,
                    hit ? "" : "   <-- MISMATCH");
        CHECK(hit);
    }
    std::printf("  %d/%d residues reproduced\n", ok, n);
    CHECK(n >= 12);
}

// Asp and Glu differ only by a methylene, and that is exactly what the chain-length term is for.
TEST("carboxylate hydration tracks the chain length") {
    if (!load()) return;
    std::vector<int> asp = pick("ASP", 8), glu = pick("GLU", 9);
    if (asp.empty() || glu.empty()) return;
    Built a = build("ASP", g.in, g.perc, g.bonds, asp, Perceiver(g.tbl), &g.hyd);
    Built e = build("GLU", g.in, g.perc, g.bonds, glu, Perceiver(g.tbl), &g.hyd);
    std::printf("  Asp %.1f, Glu %.1f -- the extra methylene is worth %.1f water\n",
                a.hydration.total, e.hydration.total,
                e.hydration.total - a.hydration.total);
    CHECK(e.hydration.total > a.hydration.total);
    CHECK(std::fabs((e.hydration.total - a.hydration.total) - 1.0) < 0.01);
}

int main() { return tinytest::run(); }

TEST("psv is emitted at the table temperature, not the increments' temperature") {
    if (!load()) return;
    std::vector<int> idx = pick("ALA", 5);
    if (idx.empty()) { std::printf("  (no complete ALA -- skipped)\n"); return; }
    somo_residue_builder::Options o;                       // defaults: 25 C -> 20 C
    Built b20 = build("ALA", g.in, g.perc, g.bonds, idx, Perceiver(g.tbl), &g.hyd, "", o);
    somo_residue_builder::Options raw = o;
    raw.psv_table_temperature = raw.psv_increment_temperature;   // no conversion
    Built b25 = build("ALA", g.in, g.perc, g.bonds, idx, Perceiver(g.tbl), &g.hyd, "", raw);
    const double expect = raw.psv_dvdt * (o.psv_table_temperature - o.psv_increment_temperature);
    std::printf("  psv 25 C %.4f -> 20 C %.4f (delta %.4f, expected %.4f)\n",
                b25.psv.vbar, b20.psv.vbar, b20.psv.vbar - b25.psv.vbar, expect);
    // somo.residue is a 20 C table; emitting the raw 25 C value put every entry ~0.3% high
    CHECK(std::fabs((b20.psv.vbar - b25.psv.vbar) - expect) < 1e-9);
    CHECK(b20.psv.vbar < b25.psv.vbar);
}
