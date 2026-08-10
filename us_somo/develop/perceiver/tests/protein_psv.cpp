// protein_psv.cpp -- whole-protein psv three ways, against measured values.
//
// Answers the question Mattia put: trust the tabulated amino-acid psv set, or the one derived
// from the atomic volume increments? The residue-level four-way table localises the differences
// but cannot say which is RIGHT; only a protein-level comparison against measurement can.
//
//   [table]   somo.residue vbar per residue -- the incumbent (Cohn & Edsall based)
//   [D&Z]     perceived from coordinates, assume_ph7_ionization = false
//   [D&Z+pH]  perceived from coordinates, assume_ph7_ionization = true
//
// All three use SOMO's own aggregation (US_Hydrodyn::calc_vbar_updated):
//     vbar = ( SUM vbar_i * mw_i + covolume ) / SUM mw_i,  covolume = 12.4 once per structure.
// Terminal N/OXT corrections are omitted -- they are a <0.1% effect on a 100+ residue chain and
// would apply equally to all three columns, so they cannot shift the comparison.
//
// Usage: protein_psv <measured.txt> <pdb> [<pdb> ...]
//   measured.txt lines: <pdb-basename> <measured-psv> <n> <label>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>
#include "../pdb_lite.h"
#include "../../include/us_hydrodyn_perceive.h"
#include "../../include/us_hydrodyn_perceive_hybrid.h"
#include "../../include/us_hydrodyn_psv.h"
using namespace somo_perceive;

struct TabEntry { double vbar = 0; double mw = 0; };

// somo.residue positional format: comment line, header, natoms atom lines, nbeads bead lines.
// The header's 4th field is ASA, not mw -- the residue mass is the sum of the (united-atom)
// per-atom masses in field 2, which is how 71.09 for ALA comes out.
static std::map<std::string, TabEntry> load_residue_table(const std::string& path, bool& ok) {
    std::map<std::string, TabEntry> t;
    std::ifstream f(path);
    ok = f.good();
    if (!ok) return t;
    std::vector<std::string> L; std::string ln;
    while (std::getline(f, ln)) L.push_back(ln);
    size_t i = 0;
    while (i < L.size()) {
        ++i;                                  // skip the free-text comment line
        if (i >= L.size()) break;
        std::istringstream hs(L[i]);
        std::string nm; int type = 0, na = 0, nb = 0; double molvol = 0, asa = 0, vbar = 0;
        if (!(hs >> nm >> type >> molvol >> asa >> na >> nb >> vbar)) { continue; }
        ++i;
        double mw = 0;
        for (int a = 0; a < na && i + (size_t)a < L.size(); ++a) {
            std::istringstream as(L[i + a]);
            std::string an, hyb; double amw = 0;
            if (as >> an >> hyb >> amw) mw += amw;
        }
        i += (size_t)na + (size_t)(nb > 0 ? nb : 0);
        if (!t.count(nm)) t[nm] = TabEntry{vbar, mw};   // first entry wins (neutral state)
    }
    return t;
}

static bool is_water(const std::string& r) {
    return r == "HOH" || r == "WAT" || r == "DOD" || r == "TIP" || r == "SOL";
}

struct Acc { double mv = 0, mw = 0; int n = 0; };

int main(int argc, char** argv) {
    if (argc < 3) {
        std::fprintf(stderr, "usage: protein_psv <measured.txt> <pdb> [<pdb> ...]\n");
        return 2;
    }
    const double COVOLUME = 12.4;

    HybridTable tbl;
    if (!tbl.load("../../etc/somo.hybrid.new")) { std::fprintf(stderr, "no somo.hybrid.new\n"); return 2; }
    bool rok = false;
    auto restab = load_residue_table("../../etc/somo.residue.new", rok);
    if (!rok || restab.empty()) { std::fprintf(stderr, "no somo.residue.new\n"); return 2; }
    Perceiver perc(tbl);

    // measured values
    struct Meas { double psv; int n; std::string label; };
    std::map<std::string, Meas> meas;
    { std::ifstream mf(argv[1]); std::string k, lbl; double v; int n;
      while (mf >> k >> v >> n) { std::getline(mf, lbl);
          size_t p = lbl.find_first_not_of(" \t"); lbl = p == std::string::npos ? "" : lbl.substr(p);
          meas[k] = Meas{v, n, lbl}; } }

    std::printf("%-22s %8s %8s %8s %8s   %7s %7s %7s\n",
                "protein", "measured", "table", "D&Z", "D&Z+pH", "dTab%", "dD&Z%", "dpH%");
    std::printf("%s\n", std::string(88, '-').c_str());

    std::vector<double> et, ed, ep;             // signed % errors
    struct RD { double dsum = 0; double mw = 0; int n = 0; };
    std::map<std::string, RD> per_res_delta;   // resname -> mean dvbar, total mass, count

    for (int ai = 2; ai < argc; ++ai) {
        std::string path = argv[ai];
        std::string base = path.substr(path.find_last_of('/') + 1);
        std::string key  = base.substr(0, base.find_last_of('.'));

        auto atoms = strip_altlocs(read_pdb(path));
        if (atoms.empty()) { std::fprintf(stderr, "%s: no atoms\n", path.c_str()); continue; }
        // The D&Z increments are a UNITED-ATOM scheme: a heavy atom's increment already contains
        // its hydrogens (C 9.9, amide N 2.0 ...), and somo.residue is united-atom for the same
        // reason -- its ALA masses are N 15.02, CA 13.02, CB 15.04. Feeding explicit H atoms in
        // would double count, and the psv guard correctly refuses them, so strip H/D here.
        // Deposited NMR ensembles repeat every atom once per MODEL; first occurrence wins, which
        // takes model 1 and leaves single-model files untouched.
        std::vector<InAtom> in;
        std::set<std::string> seen_atom;
        size_t n_h = 0, n_dup = 0;
        for (auto& a : atoms) {
            std::string el = a.element;
            for (auto& c : el) c = (char)std::toupper((unsigned char)c);
            while (!el.empty() && el.front() == ' ') el.erase(el.begin());
            while (!el.empty() && el.back()  == ' ') el.pop_back();
            if (el == "H" || el == "D") { ++n_h; continue; }
            std::string k = std::string(1, a.chain) + "|" + std::to_string(a.resSeq) + "|"
                          + a.resName + "|" + a.name;
            if (!seen_atom.insert(k).second) { ++n_dup; continue; }
            InAtom x; x.element = a.element; x.x = a.x; x.y = a.y; x.z = a.z;
            x.serial = a.serial; x.name = a.name; x.resName = a.resName;
            x.chain = std::string(1, a.chain); x.resSeq = a.resSeq; x.hetatm = a.hetatm;
            in.push_back(x);
        }
        if (n_h || n_dup)
            std::printf("    (%s: dropped %zu explicit H/D, %zu duplicate-model atoms)\n",
                        key.c_str(), n_h, n_dup);
        std::map<int,int> ser2idx;
        for (size_t i = 0; i < in.size(); ++i) ser2idx[in[i].serial] = (int)i;
        std::vector<std::pair<int,int>> ebonds;
        for (auto& c : read_conect(path)) {
            auto ia = ser2idx.find(c.first), ib = ser2idx.find(c.second);
            if (ia != ser2idx.end() && ib != ser2idx.end()) ebonds.push_back({ia->second, ib->second});
        }
        Bonds bonds;
        auto out = perc.perceive(in, bonds, ebonds);

        // group by (chain, resSeq, resName), preserving order
        std::vector<std::pair<std::string, std::vector<int>>> groups;
        std::map<std::string, size_t> gidx;
        for (size_t i = 0; i < in.size(); ++i) {
            if (is_water(in[i].resName)) continue;
            std::string k = in[i].chain + "|" + std::to_string(in[i].resSeq) + "|" + in[i].resName;
            auto it = gidx.find(k);
            if (it == gidx.end()) { gidx[k] = groups.size(); groups.push_back({k, {(int)i}}); }
            else groups[it->second].second.push_back((int)i);
        }

        Acc a_tab, a_off, a_on;
        std::set<std::string> missing_tab, failed_psv;
        size_t n_missing = 0, n_failed = 0;
        for (auto& g : groups) {
            const std::vector<int>& idx = g.second;
            std::string rn = in[idx[0]].resName;

            // All three columns must cover the SAME residues or the comparison is rigged: a
            // ligand the table cannot represent (1BEB's SO4) would otherwise count towards D&Z
            // only. Table membership gates the residue for every column.
            auto te = restab.find(rn);
            if (te == restab.end() || te->second.mw <= 0) { missing_tab.insert(rn); ++n_missing; continue; }

            somo_psv::Options o_off; o_off.free_molecule = false; o_off.assume_ph7_ionization = false;
            somo_psv::Options o_on;  o_on.free_molecule  = false; o_on.assume_ph7_ionization  = true;
            auto r_off = somo_psv::compute(in, out, bonds, idx, o_off);
            auto r_on  = somo_psv::compute(in, out, bonds, idx, o_on);
            if (!r_off.ok || !r_on.ok) { failed_psv.insert(rn); ++n_failed; continue; }

            a_off.mv += r_off.molar_volume; a_off.mw += r_off.mw; ++a_off.n;
            a_on .mv += r_on .molar_volume; a_on .mw += r_on .mw; ++a_on .n;
            a_tab.mv += te->second.vbar * te->second.mw; a_tab.mw += te->second.mw; ++a_tab.n;
            auto& acc = per_res_delta[rn];
            acc.dsum += r_off.vbar - te->second.vbar; acc.mw += te->second.mw; ++acc.n;
        }

        auto vb = [&](const Acc& a) { return a.mw > 0 ? (a.mv + COVOLUME) / a.mw : 0.0; };
        double v_tab = vb(a_tab), v_off = vb(a_off), v_on = vb(a_on);

        // A psv averaged over a surviving handful of residues is not a protein psv. Refuse it
        // rather than print a plausible-looking number: 2AAS did exactly that at +13.7%.
        const size_t n_total = (size_t)a_tab.n + n_missing + n_failed;
        const double cover = n_total ? (double)a_tab.n / (double)n_total : 0.0;
        if (cover < 0.95) {
            std::printf("%-22s  EXCLUDED: only %zu of %zu residues usable (%.0f%%)"
                        " -- missing from table %zu, psv refused %zu\n",
                        key.c_str(), (size_t)a_tab.n, n_total, cover * 100, n_missing, n_failed);
            if (!failed_psv.empty()) {
                std::printf("    (psv refused for:");
                for (auto& s2 : failed_psv) std::printf(" %s", s2.c_str());
                std::printf(")\n");
            }
            continue;
        }

        auto m = meas.find(key);
        if (m == meas.end()) {
            std::printf("%-22s %8s %8.4f %8.4f %8.4f   %7s %7s %7s\n",
                        key.c_str(), "-", v_tab, v_off, v_on, "-", "-", "-");
        } else {
            double mv = m->second.psv;
            double dt = (v_tab - mv) / mv * 100, dd = (v_off - mv) / mv * 100, dp = (v_on - mv) / mv * 100;
            et.push_back(dt); ed.push_back(dd); ep.push_back(dp);
            std::printf("%-22s %8.4f %8.4f %8.4f %8.4f   %+7.2f %+7.2f %+7.2f\n",
                        m->second.label.empty() ? key.c_str() : m->second.label.c_str(),
                        mv, v_tab, v_off, v_on, dt, dd, dp);
        }
        if (!missing_tab.empty()) {
            std::printf("    (not in somo.residue, excluded from [table]:");
            for (auto& s : missing_tab) std::printf(" %s", s.c_str());
            std::printf(")\n");
        }
        if (!failed_psv.empty()) {
            std::printf("    (psv could not be computed, excluded from all:");
            for (auto& s : failed_psv) std::printf(" %s", s.c_str());
            std::printf(")\n");
        }
    }

    auto summ = [](const char* lab, std::vector<double>& e) {
        if (e.empty()) return;
        double s = 0, sa = 0;
        for (double x : e) { s += x; sa += std::fabs(x); }
        double mean = s / e.size();
        double sd = 0; for (double x : e) sd += (x - mean) * (x - mean);
        sd = e.size() > 1 ? std::sqrt(sd / (e.size() - 1)) : 0;
        double rms = 0; for (double x : e) rms += x * x;
        rms = std::sqrt(rms / e.size());
        std::printf("  %-9s bias %+6.2f%%   mean|err| %5.2f%%   RMS %5.2f%%   SD %5.2f%%\n",
                    lab, mean, sa / e.size(), rms, sd);
    };
    std::printf("%s\n", std::string(88, '-').c_str());
    std::printf("n = %zu proteins\n", et.size());
    summ("[table]",  et);
    summ("[D&Z]",    ed);
    summ("[D&Z+pH]", ep);

    std::printf("\nper-residue (D&Z - table) vbar, and each type's share of the protein-level gap:\n");
    double tot_mw = 0;
    for (auto& kv : per_res_delta) tot_mw += kv.second.mw;
    std::vector<std::pair<double,std::string>> pr;
    for (auto& kv : per_res_delta)
        if (kv.second.n > 0) pr.push_back({kv.second.dsum / kv.second.n, kv.first});
    std::sort(pr.begin(), pr.end());
    double contrib_sum = 0;
    for (auto& p : pr) {
        auto& rd = per_res_delta[p.second];
        auto te = restab.find(p.second);
        double base = te != restab.end() ? te->second.vbar : 0;
        double contrib = tot_mw > 0 ? rd.mw * p.first / tot_mw : 0;   // cm^3/g on the protein
        contrib_sum += contrib;
        std::printf("  %-5s %+7.4f (%.3f -> %.3f, %+6.2f%%)  n=%-4d  contributes %+7.4f cm3/g\n",
                    p.second.c_str(), p.first, base, base + p.first,
                    base > 0 ? p.first / base * 100 : 0, rd.n, contrib);
    }
    std::printf("  %-5s %38s %20s %+7.4f cm3/g  (= %+.2f%% on a 0.73 protein)\n",
                "TOTAL", "", "", contrib_sum, contrib_sum / 0.73 * 100);
    return 0;
}
