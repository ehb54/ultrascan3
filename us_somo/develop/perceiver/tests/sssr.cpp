// Unit tests for find_sssr(). Ring COUNT and SIZES drive the Durchschlag & Zipper ring-formation
// volume decrement, which is charged once per ring, so both must be exact -- a fused bicyclic
// must yield 2 rings, not the 3 simple cycles a full cycle enumeration finds.
//
// These run on hand-built adjacency, with no geometry, so pathological topologies (spiro,
// bridged, macrocyclic, disconnected) can be covered without needing a structure that contains
// them.  build+run:  make sssr
#include <algorithm>
#include <cstdio>
#include <map>
#include <string>
#include <vector>

#include "../../include/us_hydrodyn_perceive.h"
#include "../tinytest.h"

using namespace somo_perceive;

// Build adjacency from an edge list.
static Bonds graph(int n, std::vector<std::pair<int,int>> e) {
    Bonds b;
    b.nb.assign(n, {});
    for (auto& x : e) {
        b.nb[x.first].push_back(x.second);
        b.nb[x.second].push_back(x.first);
    }
    return b;
}

static Bonds cycle(int n, int base = 0) {
    std::vector<std::pair<int,int>> e;
    for (int i = 0; i < n; ++i) e.push_back({base + i, base + (i + 1) % n});
    Bonds b;
    b.nb.assign(base + n, {});
    for (auto& x : e) {
        b.nb[x.first].push_back(x.second);
        b.nb[x.second].push_back(x.first);
    }
    return b;
}

// multiset of ring sizes, sorted
static std::vector<int> sizes(const Bonds& b) {
    std::vector<int> s;
    for (auto& r : b.rings) s.push_back((int) r.size());
    std::sort(s.begin(), s.end());
    return s;
}

static std::string show(const std::vector<int>& v) {
    std::string s = "{";
    for (size_t i = 0; i < v.size(); ++i) s += (i ? "," : "") + std::to_string(v[i]);
    return s + "}";
}

static void expect(Bonds b, std::vector<int> want, const char* what) {
    find_sssr(b);
    const std::vector<int> got = sizes(b);
    if (got != want) {
        std::printf("  %s: got %s want %s\n", what, show(got).c_str(), show(want).c_str());
    }
    CHECK(got == want);
}

// Every ring must be a genuine closed walk in the graph: consecutive members bonded, no repeats.
static bool rings_are_valid_cycles(const Bonds& b) {
    for (auto& r : b.rings) {
        if (r.size() < 3) return false;
        std::vector<int> sorted = r;
        std::sort(sorted.begin(), sorted.end());
        if (std::unique(sorted.begin(), sorted.end()) != sorted.end()) return false;  // repeat
        for (size_t k = 0; k < r.size(); ++k) {
            const int u = r[k], v = r[(k + 1) % r.size()];
            if (std::find(b.nb[u].begin(), b.nb[u].end(), v) == b.nb[u].end()) return false;
        }
    }
    return true;
}

TEST("acyclic graphs have no rings") {
    expect(graph(1, {}), {}, "single atom");
    expect(graph(2, {{0,1}}), {}, "single bond");
    expect(graph(4, {{0,1},{1,2},{2,3}}), {}, "chain");
    expect(graph(4, {{0,1},{0,2},{0,3}}), {}, "star");
    expect(graph(0, {}), {}, "empty graph");
}

TEST("simple rings, every size the increments define (3..9)") {
    for (int n = 3; n <= 9; ++n) {
        Bonds b = cycle(n);
        find_sssr(b);
        const std::vector<int> got = sizes(b);
        if (got != std::vector<int>{n}) std::printf("  cycle %d gave %s\n", n, show(got).c_str());
        CHECK(got == std::vector<int>{n});
    }
}

TEST("ring with a substituent chain is still one ring") {
    // cyclohexane + ethyl tail
    expect(graph(8, {{0,1},{1,2},{2,3},{3,4},{4,5},{5,0},{0,6},{6,7}}), {6}, "ring + tail");
}

TEST("FUSED bicyclics give 2 rings, not the 3 simple cycles") {
    // naphthalene: two 6-rings sharing an edge (0-1). Simple-cycle enumeration finds 6,6,10.
    Bonds naph = graph(10, {{0,1},{1,2},{2,3},{3,4},{4,5},{5,0},
                            {0,9},{9,8},{8,7},{7,6},{6,1}});
    expect(naph, {6,6}, "naphthalene");

    // indole: 5-ring fused to 6-ring on a shared edge -- the tryptophan case.
    Bonds ind = graph(9, {{0,1},{1,2},{2,3},{3,4},{4,5},{5,0},
                          {0,6},{6,7},{7,8},{8,1}});
    expect(ind, {5,6}, "indole (Trp)");

    // purine: 5 fused to 6 -- adenine/guanine.
    Bonds pur = graph(9, {{0,1},{1,2},{2,3},{3,4},{4,5},{5,0},
                          {4,6},{6,7},{7,8},{8,5}});
    expect(pur, {5,6}, "purine (adenine/guanine)");
}

TEST("spiro: two rings sharing one atom") {
    // two cyclopentanes sharing atom 0
    expect(graph(9, {{0,1},{1,2},{2,3},{3,4},{4,0},
                     {0,5},{5,6},{6,7},{7,8},{8,0}}), {5,5}, "spiro[4.4]");
}

TEST("bridged bicyclic: circuit rank 2 (norbornane-like)") {
    // bicyclo[2.2.1]: 6-ring 0..5 plus a one-atom bridge 6 between 0 and 3.
    // Circuit rank = 8 bonds - 7 atoms + 1 = 2. Smallest two cycles are both 5-membered.
    Bonds b = graph(7, {{0,1},{1,2},{2,3},{3,4},{4,5},{5,0},{0,6},{6,3}});
    find_sssr(b);
    std::printf("  norbornane rings: %s (circuit rank 2)\n", show(sizes(b)).c_str());
    CHECK(b.rings.size() == 2);
    CHECK(rings_are_valid_cycles(b));
}

TEST("three fused rings: anthracene-like gives exactly 3") {
    Bonds b = graph(14, {{0,1},{1,2},{2,3},{3,4},{4,5},{5,0},
                         {1,6},{6,7},{7,8},{8,2},
                         {7,9},{9,10},{10,11},{11,8}});
    find_sssr(b);
    std::printf("  three-fused: %s\n", show(sizes(b)).c_str());
    CHECK(b.rings.size() == 3);
    CHECK(rings_are_valid_cycles(b));
}

TEST("disconnected molecules: rings counted per component") {
    // benzene (0..5) + cyclopentane (6..10), no bond between them
    Bonds b = graph(11, {{0,1},{1,2},{2,3},{3,4},{4,5},{5,0},
                         {6,7},{7,8},{8,9},{9,10},{10,6}});
    expect(b, {5,6}, "two separate rings");

    // a ring plus a lone atom -- component count must still be right
    Bonds c = graph(7, {{0,1},{1,2},{2,3},{3,4},{4,5},{5,0}});
    expect(c, {6}, "ring + isolated atom");
}

TEST("sugar rings: pyranose (6) and furanose (5)") {
    expect(cycle(6), {6}, "pyranose");
    expect(cycle(5), {5}, "furanose");
    // Nucleotide-like: furanose joined by a glycosidic bond to a purine (6-ring fused to a
    // 5-ring across a shared edge). Circuit rank = 16 bonds - 14 atoms + 1 = 3.
    Bonds b = graph(14, {{0,1},{1,2},{2,3},{3,4},{4,0},                    // furanose  0-4
                         {0,5},                                            // glycosidic
                         {5,6},{6,7},{7,8},{8,9},{9,10},{10,5},            // 6-ring    5-10
                         {10,11},{11,12},{12,13},{13,9}});                 // fused 5-ring, shares 9-10
    find_sssr(b);
    std::printf("  nucleotide-like: %s\n", show(sizes(b)).c_str());
    CHECK(sizes(b) == (std::vector<int>{5,5,6}));
}

TEST("macrocycle beyond max_ring is not reported") {
    Bonds b = cycle(20);
    find_sssr(b, 12);
    std::printf("  20-ring with max_ring=12: %zu rings\n", b.rings.size());
    CHECK(b.rings.empty());
    Bonds c = cycle(20);
    find_sssr(c, 25);
    CHECK(sizes(c) == (std::vector<int>{20}));
}

TEST("reported rings are always genuine cycles") {
    std::vector<Bonds> cases = {
        cycle(6),
        graph(10, {{0,1},{1,2},{2,3},{3,4},{4,5},{5,0},{0,9},{9,8},{8,7},{7,6},{6,1}}),
        graph(9, {{0,1},{1,2},{2,3},{3,4},{4,0},{0,5},{5,6},{6,7},{7,8},{8,0}}),
        graph(7, {{0,1},{1,2},{2,3},{3,4},{4,5},{5,0},{0,6},{6,3}}),
    };
    for (Bonds b : cases) {
        find_sssr(b);
        CHECK(rings_are_valid_cycles(b));
    }
}

TEST("idempotent: running twice does not accumulate rings") {
    Bonds b = cycle(6);
    find_sssr(b);
    const size_t first = b.rings.size();
    find_sssr(b);
    CHECK(b.rings.size() == first);
}

// The ring counts above feed a per-ring volume decrement, so verify the D-Z arithmetic they
// drive: Table 1 gives 5-ring 6.1, 6-ring 8.1, and the published calculated volumes for
// phenylalanine (one 6-ring) and proline (one 5-ring) are reproduced only with exactly one
// decrement each. Two decrements for Phe would give 113.8, not the published 121.9.
TEST("ring count drives the D-Z decrement correctly") {
    const double C = 9.9, H = 3.1, O_ACID = 0.4, O_C = 5.5, N_AMIDE = 2.0, N_AMINE = 4.0;
    const double CV = 12.4, RF5 = 6.1, RF6 = 8.1, ESP = 6.8, ESN = 6.7;
    const double phe = 9 * C + 11 * H + N_AMIDE + O_ACID + O_C + CV - RF6 - ESP - ESN;
    const double pro = 5 * C + 9 * H + N_AMINE + O_ACID + O_C + CV - RF5 - ESP - ESN;
    std::printf("  Phe %.1f (published 121.9)   Pro %.1f (published 80.1)\n", phe, pro);
    CHECK(std::abs(phe - 121.9) < 0.05);
    CHECK(std::abs(pro - 80.1) < 0.05);
}

int main() { return tinytest::run(); }
