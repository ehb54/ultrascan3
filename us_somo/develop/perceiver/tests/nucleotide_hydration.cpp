// The nucleotide hydration rules on real geometry: somo.residue's nucleotide entries (2026-10,
// phosphate oxygens from MD) as the reference, flavin by analogy with the bases (Mattia,
// 2026-10-26). The cofactors are the CCD's model coordinates (data/non-coded/ccd/); the
// nucleic-acid backbone is the DNA of 3CRO, where every internal phosphate is a diester.
//
// The expected values are written out here rather than read from somo.residue.new, so these
// tests say what the rules must do whichever table version is checked out.
//   build+run:  make nucleotide
#include <cmath>
#include <cstdio>
#include <map>
#include <string>
#include <vector>

#include "../../include/us_hydrodyn_hydration.h"
#include "../pdb_lite.h"
#include "../tinytest.h"

using namespace somo_perceive;
using namespace somo_hydration;

struct Structure {
    std::vector<InAtom> in;
    std::vector<OutAtom> perc;
    Bonds bonds;
    std::map<std::string, std::vector<int>> residues;   // "resName|chain|resSeq" -> atoms
    std::vector<std::string> order;                      // residue keys in file order
    bool ok = false;
};

static HybridTable g_tbl;

static Structure load(const std::string& path) {
    Structure s;
    static bool tbl_ok = g_tbl.load("../../etc/somo.hybrid.new");
    if (!tbl_ok) return s;
    for (const PdbAtom& a : strip_altlocs(read_pdb(path))) {
        if (a.resName == "HOH") continue;
        InAtom x;
        x.element = a.element; x.x = a.x; x.y = a.y; x.z = a.z;
        x.serial = a.serial; x.name = a.name; x.resName = a.resName;
        x.chain = std::string(1, a.chain); x.resSeq = a.resSeq; x.hetatm = a.hetatm;
        s.in.push_back(x);
    }
    if (s.in.empty()) return s;
    s.perc = Perceiver(g_tbl).perceive(s.in, s.bonds);
    for (size_t i = 0; i < s.in.size(); ++i) {
        const std::string k = s.in[i].resName + "|" + s.in[i].chain + "|" + std::to_string(s.in[i].resSeq);
        if (!s.residues.count(k)) s.order.push_back(k);
        s.residues[k].push_back((int) i);
    }
    s.ok = true;
    return s;
}

struct Want { const char* atom; double waters; };

// Propose for one residue and compare atom by atom; returns the proposal's total.
static double check(const Structure& s, const std::vector<int>& idx, const std::vector<Want>& want,
                    const char* label, Proposal* out = nullptr) {
    Proposal p = propose_by_rules(s.in, s.perc, s.bonds, idx);
    CHECK(p.ok);
    std::map<std::string, double> got;
    for (size_t k = 0; k < idx.size() && k < p.per_atom.size(); ++k) got[s.in[idx[k]].name] = p.per_atom[k];
    for (const Want& w : want) {
        auto it = got.find(w.atom);
        const bool hit = it != got.end() && std::fabs(it->second - w.waters) < 1e-9;
        if (!hit)
            std::printf("  %s %s: proposed %s, want %g\n", label, w.atom,
                        it == got.end() ? "nothing (atom absent)" : std::to_string(it->second).c_str(),
                        w.waters);
        CHECK(hit);
    }
    if (out) *out = p;
    return p.total;
}

static std::vector<int> only_residue(const Structure& s) {
    return s.order.size() == 1 ? s.residues.at(s.order[0]) : std::vector<int>();
}

TEST("FAD: phosphates, ribose, ribityl, adenine and flavin") {
    Structure s = load("data/non-coded/ccd/FAD.pdb");
    std::vector<int> idx = only_residue(s);
    if (idx.empty()) { std::printf("  (fixture or tables absent -- skipped)\n"); return; }
    Proposal p;
    const double total = check(s, idx, {
        // the FMN half's phosphate and the AMP half's, joined by the O3P bridge
        {"P", 0}, {"O1P", 1}, {"O2P", 5}, {"O3P", 2}, {"O5'", 0},
        {"PA", 0}, {"O1A", 1}, {"O2A", 5}, {"O5B", 0},
        // ribose: ring oxygen and hydroxyls; ribityl hydroxyls
        {"O4B", 1}, {"O3B", 1}, {"O2B", 1}, {"O2'", 1}, {"O3'", 1}, {"O4'", 1},
        // adenine: N1 and the glycosidic N9 dry, N3, N7 and the amino N6 hydrated
        {"N1A", 0}, {"N3A", 1}, {"N7A", 1}, {"N9A", 0}, {"N6A", 1},
        // isoalloxazine: N10 on the ribityl and N1/N3 beside the carbonyls dry, N5 and the
        // carbonyls hydrated
        {"N10", 0}, {"N1", 0}, {"N3", 0}, {"N5", 1}, {"O2", 1}, {"O4", 1},
    }, "FAD", &p);
    std::printf("  FAD total %.0f waters (phosphates 14, sugars 6, adenine 3, flavin 3)\n", total);
    CHECK(std::fabs(total - 26) < 1e-9);
    int fallbacks = 0;
    for (const std::string& r : p.review)
        if (r.find("no pH 7 rule") != std::string::npos) { ++fallbacks; std::printf("  %s\n", r.c_str()); }
    CHECK(fallbacks == 0);
}

TEST("AMP, ADP and NAD phosphates reproduce the table") {
    struct Case { const char* res; std::vector<Want> want; };
    const Case cases[] = {
        {"AMP", {{"O1P", 1}, {"O2P", 5}, {"O3P", 5}, {"O5'", 0}}},
        {"ADP", {{"O1A", 1}, {"O2A", 5}, {"O3A", 2}, {"O5'", 0}, {"O1B", 1}, {"O2B", 4}, {"O3B", 5}}},
        {"NAD", {{"O1A", 1}, {"O2A", 5}, {"O5B", 0}, {"O3", 2}, {"O1N", 1}, {"O2N", 5}, {"O5D", 0}}},
    };
    for (const Case& c : cases) {
        Structure s = load(std::string("data/non-coded/ccd/") + c.res + ".pdb");
        std::vector<int> idx = only_residue(s);
        if (idx.empty()) { std::printf("  (%s fixture absent -- skipped)\n", c.res); continue; }
        check(s, idx, c.want, c.res);
    }
}

// ATP's own alpha (1, 4) and gamma (1, 3, 3) are lower than ADP's for the same topologies, and
// the rules use ADP's. Pinned here so a change to either is deliberate.
TEST("ATP takes ADP's values for its alpha and gamma phosphates") {
    Structure s = load("data/non-coded/ccd/ATP.pdb");
    std::vector<int> idx = only_residue(s);
    if (idx.empty()) { std::printf("  (fixture absent -- skipped)\n"); return; }
    check(s, idx, {
        {"O1A", 1}, {"O2A", 5}, {"O3A", 2}, {"O5'", 0},     // table: O2A 4
        {"O1B", 1}, {"O2B", 4}, {"O3B", 2},                 // the table's own beta
        {"O1G", 1}, {"O2G", 4}, {"O3G", 5},                 // table: O2G 3, O3G 3
    }, "ATP");
}

TEST("DNA backbone and bases reproduce the nucleotide entries") {
    Structure s = load("data/3CRO.pdb");
    if (!s.ok) { std::printf("  (3CRO absent -- skipped)\n"); return; }
    const std::vector<Want> backbone = {{"P", 0}, {"OP1", 1}, {"OP2", 4}, {"O5'", 1}, {"O4'", 1}, {"O3'", 1}};
    struct Base { const char* res; std::vector<Want> want; double total; };
    const Base bases[] = {
        {"DA", {{"N9", 0}, {"N7", 1}, {"N6", 1}, {"N1", 0}, {"N3", 1}}, 11},
        {"DC", {{"N1", 0}, {"O2", 1}, {"N3", 0}, {"N4", 1}}, 10},
        {"DG", {{"N9", 0}, {"N7", 1}, {"O6", 1}, {"N1", 0}, {"N2", 1}, {"N3", 1}}, 12},
        {"DT", {{"N1", 0}, {"O2", 1}, {"N3", 0}, {"O4", 1}}, 10},
    };
    for (const Base& b : bases) {
        // an internal residue: it has its own phosphate and its O3' is the next one's ester
        std::vector<int> idx;
        for (const std::string& k : s.order) {
            if (k.substr(0, k.find('|')) != b.res) continue;
            const std::vector<int>& r = s.residues.at(k);
            bool has_p = false, o3_ester = false;
            for (int i : r) {
                if (s.in[i].name == "P") has_p = true;
                if (s.in[i].name == "O3'")
                    for (int n : s.bonds.nb[i]) if (s.perc[n].element == "P") o3_ester = true;
            }
            if (has_p && o3_ester) { idx = r; break; }
        }
        if (idx.empty()) { std::printf("  (no internal %s in 3CRO -- skipped)\n", b.res); continue; }
        std::vector<Want> want = backbone;
        want.insert(want.end(), b.want.begin(), b.want.end());
        const double total = check(s, idx, want, b.res);
        std::printf("  %s %s: total %.0f (somo.residue %.0f)\n", b.res,
                    s.in[idx[0]].chain.c_str(), total, b.total);
        CHECK(std::fabs(total - b.total) < 1e-9);
    }
}

// The furanose ring oxygen is hydrated; a pyranose's stays dry, as every coded sugar stores it.
TEST("a pyranose ring oxygen stays dry") {
    Structure s = load("data/non-coded/ccd/GLC.pdb");
    std::vector<int> idx = only_residue(s);
    if (idx.empty()) { std::printf("  (fixture absent -- skipped)\n"); return; }
    check(s, idx, {{"O5", 0}, {"O1", 1}, {"O2", 1}, {"O3", 1}, {"O6", 1}}, "GLC");
}

int main() { return tinytest::run(); }
