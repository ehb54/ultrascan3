// Tests for the proposed-hydration lookup.
//
// The table is derived at run time from somo.residue rather than embedded, so these tests both
// exercise the derivation and record what the source data actually supports -- which is less
// than it first appears, because the per-atom values there are a hand distribution of a
// per-residue Kuntz total rather than a per-atom rule.
//   build+run:  make hydration
#include <cmath>
#include <cstdio>
#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "../../include/us_hydrodyn_hydration.h"
#include "../tinytest.h"

using namespace somo_perceive;
using namespace somo_hydration;

// (hybrid, waters) for every atom of every coded residue, plus per-residue totals.
struct Parsed {
    std::vector<std::pair<std::string, double>> obs;
    std::map<std::string, double> residue_total;
    std::map<std::string, int> residue_atoms;
};

static Parsed parse(const std::string& path) {
    Parsed p;
    std::ifstream f(path);
    std::vector<std::string> L;
    std::string ln;
    while (std::getline(f, ln)) L.push_back(ln);

    std::set<std::string> seen;
    size_t i = 0;
    while (i < L.size()) {
        ++i;                                        // descriptive comment line
        if (i >= L.size()) break;
        std::istringstream hs(L[i]);
        std::string name;
        int type = 0, natoms = 0, nbeads = 0;
        double molvol = 0, mw = 0, vbar = 0;
        if (!(hs >> name >> type >> molvol >> mw >> natoms >> nbeads >> vbar)) { continue; }
        ++i;
        const bool first = seen.insert(name).second;
        double tot = 0;
        for (int a = 0; a < natoms && i + a < L.size(); ++a) {
            // tab-separated: name, hybrid, mw, radius, ?, ?, index, hydration, ...
            std::vector<std::string> f2;
            std::string cur;
            std::istringstream as(L[i + a]);
            while (std::getline(as, cur, '\t')) f2.push_back(cur);
            if (f2.size() < 8) continue;
            double w = 0;
            try { w = std::stod(f2[7]); } catch (...) { continue; }
            if (first) p.obs.push_back({f2[1], w});
            tot += w;
        }
        if (first) { p.residue_total[name] = tot; p.residue_atoms[name] = natoms; }
        i += natoms + (nbeads > 0 ? nbeads : 0);
    }
    return p;
}

static Parsed g_p;
static Table g_t;
static bool g_ok = false;

TEST("derive the table from somo.residue") {
    g_p = parse("../../etc/somo.residue.new");
    if (g_p.obs.empty()) { std::printf("  (somo.residue.new not readable -- skipped)\n"); return; }
    g_t = Table::from_observations(g_p.obs);
    g_ok = true;
    int conf = 0;
    for (const auto& kv : g_t.all()) if (kv.second.confident) ++conf;
    std::printf("  %zu observations -> %zu hybrid types, %d confident, %zu flagged\n",
                g_p.obs.size(), g_t.size(), conf, g_t.size() - conf);
    CHECK(g_t.size() > 30);
    CHECK(conf > 0);
}

TEST("every carbon type is unanimously anhydrous") {
    if (!g_ok) return;
    for (const auto& kv : g_t.all()) {
        if (kv.first.substr(0, 1) != "C" || kv.first.size() < 2 || !isdigit(kv.first[1])) continue;
        if (kv.second.waters != 0.0 || !kv.second.confident)
            std::printf("  %s: waters %.1f agreement %.0f%%\n",
                        kv.first.c_str(), kv.second.waters, 100 * kv.second.agreement);
        CHECK(kv.second.waters == 0.0);
        CHECK(kv.second.confident);
    }
}

// These are the types the source data does NOT pin down. They must come back not-confident so
// the caller flags them rather than defaulting silently.
TEST("ambiguous types are marked not-confident") {
    if (!g_ok) return;
    const char* weak[] = {"O2H1", "O2H0", "N3H2", "N3H0"};
    for (const char* h : weak) {
        const Entry* e = g_t.find(h);
        if (!e) { std::printf("  (%s absent)\n", h); continue; }
        std::printf("  %-6s proposed %.0f, agreement %.0f%% of %d\n",
                    h, e->waters, 100 * e->agreement, e->n_obs);
        CHECK(!e->confident);
    }
}

TEST("metal ions keep their distinctive hydration") {
    if (!g_ok) return;
    struct M { const char* h; double w; };
    const M m[] = {{"MG+2", 6}, {"CU+2", 6}, {"CL-1", 6}, {"MN+2", 4}, {"ZN+2", 2}, {"NA+1", 1}};
    for (const M& x : m) {
        const Entry* e = g_t.find(x.h);
        if (!e) { std::printf("  (%s absent)\n", x.h); continue; }
        CHECK(e->waters == x.w);
    }
}

// The residue TOTAL is the quantity with literature backing, so check the source against Kuntz
// (Durchschlag 1986 Table 1, pH 6-8) -- and record that Asp and Glu deliberately differ because
// SOMO stores them protonated.
TEST("somo.residue totals reproduce the Kuntz per-residue numbers") {
    if (!g_ok) return;
    struct K { const char* res; double kuntz; };
    const K k[] = {{"ALA",1.5},{"ARG",3},{"ASN",2},{"CYS",1},{"GLN",2},{"GLY",1},{"HIS",4},
                   {"ILE",1},{"LEU",1},{"LYS",4.5},{"SER",2},{"THR",2},{"TRP",2},{"VAL",1}};
    int agree = 0, n = 0;
    for (const K& x : k) {
        auto it = g_p.residue_total.find(x.res);
        if (it == g_p.residue_total.end()) continue;
        ++n;
        if (std::fabs(it->second - x.kuntz) < 0.51) ++agree;
    }
    std::printf("  %d of %d within 0.5 water of Kuntz\n", agree, n);
    CHECK(n > 0 && agree >= n - 2);

    // The protonation cases: stored well below the charged-carboxylate Kuntz value.
    for (const char* r : {"ASP", "GLU"}) {
        auto it = g_p.residue_total.find(r);
        if (it == g_p.residue_total.end()) continue;
        std::printf("  %s stored %.1f (Kuntz charged: %s) -- SOMO stores it protonated\n",
                    r, it->second, std::string(r) == "ASP" ? "6" : "7.5");
        CHECK(it->second < 3.0);
    }
}

// Mattia, 2026-08-08: "we could then have atomic hydration at neutral pH for all the other
// entries in somo.residue... I mean, nucleotides". This records the gap he suspected.
TEST("GAP: every nucleotide entry carries zero hydration") {
    if (!g_ok) return;
    const char* nuc[] = {"A", "C", "G", "U", "DA", "DC", "DG", "DT"};
    int zero = 0, found = 0;
    for (const char* r : nuc) {
        auto it = g_p.residue_total.find(r);
        if (it == g_p.residue_total.end()) continue;
        ++found;
        if (it->second == 0.0) ++zero;
    }
    std::printf("  %d of %d nucleotide entries have total hydration 0\n", zero, found);
    // Asserted as the CURRENT state so this test turns red the day someone fills them in,
    // which is the moment to revisit any model built on the old values.
    CHECK(found > 0 && zero == found);
}

TEST("proposal flags unknown types instead of inventing a number") {
    if (!g_ok) return;
    std::vector<InAtom> atoms(1);
    atoms[0].name = "X1";
    std::vector<OutAtom> perc(1);
    perc[0].hybrid = "ZZ9H9";                      // certainly not in any table
    Proposal p = propose(g_t, atoms, perc, {0});
    CHECK(p.ok);
    CHECK(p.per_atom.size() == 1 && p.per_atom[0] == 0.0);
    CHECK(p.review.size() == 1);
}

TEST("proposal reproduces a coded residue's own total") {
    if (!g_ok) return;
    // Alanine: N N3H1, CA C4H1, C C3H0, O O1H0, CB C4H3 -- only the backbone N is hydrated.
    const char* hyb[] = {"N3H1", "C4H1", "C3H0", "O1H0", "C4H3"};
    std::vector<InAtom> atoms(5);
    std::vector<OutAtom> perc(5);
    std::vector<int> idx;
    for (int i = 0; i < 5; ++i) {
        atoms[i].name = "A" + std::to_string(i);
        perc[i].hybrid = hyb[i];
        idx.push_back(i);
    }
    Proposal p = propose(g_t, atoms, perc, idx);
    std::printf("  proposed total %.1f, somo.residue ALA total %.1f\n",
                p.total, g_p.residue_total.count("ALA") ? g_p.residue_total["ALA"] : -1.0);
    CHECK(p.ok);
    CHECK(std::fabs(p.total - 1.0) < 0.01);
}

int main() { return tinytest::run(); }
