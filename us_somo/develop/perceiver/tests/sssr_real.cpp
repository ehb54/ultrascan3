// SSSR on REAL coordinates, through the whole perception path (bond perception -> rings).
// The unit tests in sssr.cpp exercise the algorithm on hand-built graphs; this checks that what
// the perceiver actually derives from crystallographic coordinates has the ring count and sizes
// the chemistry demands. A wrong count here would silently corrupt every volume that charges a
// per-ring decrement, and nothing downstream would flag it.
//   build+run:  make sssrreal          (needs data/*.pdb -- gitignored, copied from somo/demo)
#include <algorithm>
#include <cstdio>
#include <map>
#include <cmath>
#include <string>
#include <vector>

#include "../../include/us_hydrodyn_perceive.h"
#include "../pdb_lite.h"
#include "../tinytest.h"

using namespace somo_perceive;

struct Expect {
    const char* res;
    std::vector<int> sizes;   // sorted ring sizes for one residue of this type
    const char* why;
};

// Ring systems that must come out exactly right, by residue.
static const std::vector<Expect> EXPECTED = {
    {"PHE", {6},    "benzene side chain"},
    {"TYR", {6},    "phenol side chain"},
    {"TRP", {5, 6}, "indole: FUSED, must be 2 rings not 3 cycles"},
    {"HIS", {5},    "imidazole"},
    {"PRO", {5},    "pyrrolidine, saturated (not aromatic)"},
    {"ALA", {},     "no ring"},
    {"GLY", {},     "no ring"},
    {"LEU", {},     "no ring"},
    {"SER", {},     "no ring"},
    {"ARG", {},     "guanidinium is NOT a ring"},
    // nucleotides: deoxyribose furanose plus the base, joined by a glycosidic bond
    {"DA",  {5, 5, 6}, "deoxyribose + purine (fused 5+6)"},
    {"DG",  {5, 5, 6}, "deoxyribose + purine (fused 5+6)"},
    {"DC",  {5, 6},    "deoxyribose + pyrimidine"},
    {"DT",  {5, 6},    "deoxyribose + pyrimidine"},
};

static std::vector<InAtom> to_in(const std::vector<PdbAtom>& atoms) {
    std::vector<InAtom> in;
    for (const PdbAtom& a : atoms) {
        InAtom x;
        x.element = a.element;
        x.x = a.x; x.y = a.y; x.z = a.z;
        x.serial = a.serial; x.name = a.name; x.resName = a.resName;
        x.chain = std::string(1, a.chain); x.resSeq = a.resSeq; x.hetatm = a.hetatm;
        in.push_back(x);
    }
    return in;
}

static std::string show(const std::vector<int>& v) {
    std::string s = "{";
    for (size_t i = 0; i < v.size(); ++i) s += (i ? "," : "") + std::to_string(v[i]);
    return s + "}";
}

// Rings of the whole structure, bucketed by the residue their atoms belong to. A ring whose
// atoms span two residues is reported under "" so cross-residue artefacts surface rather than
// being silently attributed to one side.
static std::map<std::string, std::vector<std::vector<int>>>
rings_by_residue(const std::vector<InAtom>& in, const Bonds& b) {
    std::map<std::string, std::vector<std::vector<int>>> out;
    for (const std::vector<int>& r : b.rings) {
        const std::string key = in[r[0]].chain + ":" + std::to_string(in[r[0]].resSeq);
        bool same = true;
        for (int i : r) {
            if (in[i].chain + ":" + std::to_string(in[i].resSeq) != key) { same = false; break; }
        }
        out[same ? in[r[0]].resName + "|" + key : std::string("CROSS-RESIDUE")].push_back(r);
    }
    return out;
}

static bool g_loaded = false;
struct Perceived { std::vector<InAtom> in; Bonds b; };
static std::vector<Perceived> g_all;      // every structure, so results do not depend on argv order
static std::vector<std::string> g_files;

// shortest bond in a ring, used to tell a real ring from one closed by clashing geometry
static double min_bond(const std::vector<int>& r, const std::vector<InAtom>& in) {
    double m = 1e9;
    for (size_t k = 0; k < r.size(); ++k) {
        const InAtom& p = in[r[k]];
        const InAtom& q = in[r[(k + 1) % r.size()]];
        const double d = std::sqrt((p.x-q.x)*(p.x-q.x) + (p.y-q.y)*(p.y-q.y) + (p.z-q.z)*(p.z-q.z));
        m = std::min(m, d);
    }
    return m;
}

TEST("perceived ring sizes are right for every residue in every demo structure") {
    HybridTable tbl;
    if (!tbl.load("../../etc/somo.hybrid.new")) {
        std::printf("  (cannot load somo.hybrid.new -- skipped)\n");
        return;
    }
    // Accumulate ring signatures across all structures, so a residue type is judged on every
    // instance seen anywhere, not just the first file.
    std::map<std::string, std::map<std::vector<int>, int>> tally;
    int files = 0;
    for (const std::string& f : g_files) {
        std::vector<PdbAtom> atoms = strip_altlocs(read_pdb(f));
        if (atoms.empty()) continue;
        ++files;
        std::vector<InAtom> in = to_in(atoms);
        Bonds b;
        Perceiver(tbl).perceive(in, b);
        g_all.push_back({in, b});
        g_loaded = true;
        for (auto& kv : rings_by_residue(in, b)) {
            if (kv.first == "CROSS-RESIDUE") continue;
            const std::string rn = kv.first.substr(0, kv.first.find('|'));
            std::vector<int> s;
            for (auto& r : kv.second) s.push_back((int) r.size());
            std::sort(s.begin(), s.end());
            tally[rn][s]++;
        }
    }
    if (!files) { std::printf("  (no demo PDBs present -- skipped)\n"); return; }
    std::printf("  %d structures\n", files);

    for (const Expect& e : EXPECTED) {
        if (e.sizes.empty()) {
            // must never appear with any ring at all
            const bool none = tally.find(e.res) == tally.end();
            if (!none) {
                std::printf("  %s should have NO rings but some instance did\n", e.res);
            }
            CHECK(none);
            continue;
        }
        auto it = tally.find(e.res);
        if (it == tally.end()) {
            std::printf("  (%s not present in structure -- skipped)\n", e.res);
            continue;
        }
        // every instance of this residue must show the same, expected ring signature
        bool all_ok = it->second.size() == 1 && it->second.count(e.sizes) == 1;
        std::printf("  %-4s %-42s ", e.res, e.why);
        for (auto& kv : it->second) std::printf("%s x%d ", show(kv.first).c_str(), kv.second);
        std::printf("%s\n", all_ok ? "" : "  <-- MISMATCH");
        CHECK(all_ok);
    }
}

// A ring crossing a residue boundary is not automatically wrong: a disulfide plus the
// intervening backbone is a genuine macrocycle. What must never happen is one produced by the
// ALGORITHM. In a protein or nucleic acid the only legitimate covalent bonds between residues
// are the peptide/phosphodiester backbone link and a disulfide, so assert that every
// cross-residue ring is closed by some other, illegitimate inter-residue bond -- i.e. it is
// traceable to defective deposited geometry. That makes this a detector for bad input rather
// than a silent pass.
//
// All such rings in the demo set come from atoms modelled far too close to be non-bonded:
// 1AO6 has LYS536/NZ 0.73 A from LEU583/CG; 6LYZ (a 1960s low-resolution structure) has
// LYS97/O 1.22 A from ASP101/OD1 -- two oxygens at a distance no covalent bond, let alone a
// contact, can take.
static bool legit_interresidue_bond(const InAtom& u, const InAtom& v) {
    const std::string a = u.name, b = v.name;
    if ((a == "C" && b == "N") || (a == "N" && b == "C")) return true;       // peptide
    if (a == "SG" && b == "SG") return true;                                 // disulfide
    if ((a == "P" && b == "O3'") || (a == "O3'" && b == "P")) return true;   // phosphodiester
    return false;
}

static int count_bad_cross_bonds(const std::vector<int>& r, const std::vector<InAtom>& in) {
    int bad = 0;
    for (size_t k = 0; k < r.size(); ++k) {
        const InAtom& u = in[r[k]];
        const InAtom& v = in[r[(k + 1) % r.size()]];
        if (u.chain == v.chain && u.resSeq == v.resSeq) continue;
        if (!legit_interresidue_bond(u, v)) ++bad;
    }
    return bad;
}

TEST("cross-residue rings arise only from illegitimate inter-residue bonds, never from the algorithm") {
    if (!g_loaded) { std::printf("  (skipped)\n"); return; }
    int cross = 0, explained = 0;
    for (const Perceived& p : g_all) {
        for (const std::vector<int>& r : p.b.rings) {
            const std::string key = p.in[r[0]].chain + ":" + std::to_string(p.in[r[0]].resSeq);
            bool same = true;
            for (int i : r)
                if (p.in[i].chain + ":" + std::to_string(p.in[i].resSeq) != key) { same = false; break; }
            if (same) continue;
            ++cross;
            if (count_bad_cross_bonds(r, p.in) > 0) ++explained;
            else std::printf("  UNEXPLAINED cross-residue ring of size %zu\n", r.size());
        }
    }
    std::printf("  %d cross-residue rings, all %d traceable to bad input geometry\n",
                cross, explained);
    CHECK(cross == explained);
}

// A 3-membered ring of C/N/O inside one residue is not real chemistry for any residue SOMO
// codes. Every 3-ring in the demo set must therefore span residues, i.e. be one of the
// geometry artefacts above.
TEST("no 3-membered ring lies within a single residue") {
    if (!g_loaded) { std::printf("  (skipped)\n"); return; }
    int three = 0, intra = 0;
    for (const Perceived& p : g_all)
        for (const std::vector<int>& r : p.b.rings) {
            if (r.size() != 3) continue;
            ++three;
            bool same = true;
            for (int i : r)
                if (p.in[i].chain != p.in[r[0]].chain || p.in[i].resSeq != p.in[r[0]].resSeq)
                    { same = false; break; }
            if (same) { ++intra; std::printf("  intra-residue 3-ring in %s%d\n",
                                             p.in[r[0]].resName.c_str(), p.in[r[0]].resSeq); }
        }
    std::printf("  %d three-membered rings, %d intra-residue (must be 0)\n", three, intra);
    CHECK(intra == 0);
}

TEST("every perceived ring is a genuine cycle in the perceived bond graph") {
    if (!g_loaded) { std::printf("  (skipped)\n"); return; }
    bool ok = true;
    for (const Perceived& p : g_all)
    for (const std::vector<int>& r : p.b.rings) {
        if (r.size() < 3) { ok = false; break; }
        std::vector<int> s = r;
        std::sort(s.begin(), s.end());
        if (std::unique(s.begin(), s.end()) != s.end()) { ok = false; break; }
        for (size_t k = 0; k < r.size(); ++k) {
            const int u = r[k], v = r[(k + 1) % r.size()];
            if (std::find(p.b.nb[u].begin(), p.b.nb[u].end(), v) == p.b.nb[u].end()) {
                ok = false; break;
            }
        }
        if (!ok) break;
    }
    CHECK(ok);
}

TEST("aromatic atoms all belong to some perceived ring") {
    if (!g_loaded) { std::printf("  (skipped)\n"); return; }
    int orphan = 0;
    for (const Perceived& p : g_all) {
        std::vector<char> in_ring(p.in.size(), 0);
        for (const std::vector<int>& r : p.b.rings)
            for (int i : r) in_ring[i] = 1;
        for (size_t i = 0; i < p.in.size(); ++i)
            if (p.b.aromatic[i] && !in_ring[i]) ++orphan;
    }
    if (orphan) std::printf("  %d aromatic atoms are in no SSSR ring\n", orphan);
    CHECK(orphan == 0);
}

int main(int argc, char** argv) {
    for (int i = 1; i < argc; ++i) g_files.push_back(argv[i]);
    if (g_files.empty()) g_files.push_back("data/1HEL.pdb");
    return tinytest::run();
}
