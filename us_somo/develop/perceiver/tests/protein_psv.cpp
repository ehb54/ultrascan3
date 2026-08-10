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

        // Hydrogens are stripped and MODELs are indexed by read_pdb itself, matching SOMO -- see
        // pdb_lite.h. Read EVERY model: SOMO keeps them all in model_vector and treats "which
        // model" as a later choice, so a psv that silently used model 1 would be answering a
        // question nobody asked. An NMR ensemble also gives a free precision check, since psv is
        // a composition property and should barely move across models.
        auto all_atoms = strip_altlocs(read_pdb(path, /*keep_first_model=*/false));
        if (all_atoms.empty()) { std::fprintf(stderr, "%s: no atoms\n", path.c_str()); continue; }
        const int n_models = model_count(all_atoms);

        std::vector<double> mv_tab, mv_off, mv_on;   // one entry per model
        for (int mdl = 1; mdl <= n_models; ++mdl) {
        auto atoms = n_models > 1 ? model_of(all_atoms, mdl) : all_atoms;
        std::vector<InAtom> in;
        for (auto& a : atoms) {
            InAtom x; x.element = a.element; x.x = a.x; x.y = a.y; x.z = a.z;
            x.serial = a.serial; x.name = a.name; x.resName = a.resName;
            x.chain = std::string(1, a.chain); x.resSeq = a.resSeq; x.hetatm = a.hetatm;
            in.push_back(x);
        }
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
            if (mdl == 1) {   // once per structure, not once per NMR model
                auto& acc = per_res_delta[rn];
                acc.dsum += r_off.vbar - te->second.vbar; acc.mw += te->second.mw; ++acc.n;
            }
        }

        auto vb = [&](const Acc& a) { return a.mw > 0 ? (a.mv + COVOLUME) / a.mw : 0.0; };
        double v_tab = vb(a_tab), v_off = vb(a_off), v_on = vb(a_on);

        // A psv averaged over a surviving handful of residues is not a protein psv. Refuse it
        // rather than print a plausible-looking number: 2AAS did exactly that at +13.7% before
        // hydrogens were stripped.
        const size_t n_total = (size_t)a_tab.n + n_missing + n_failed;
        const double cover = n_total ? (double)a_tab.n / (double)n_total : 0.0;
        if (cover < 0.95) {
            if (mdl == 1)
                std::printf("%-22s  EXCLUDED: only %zu of %zu residues usable (%.0f%%)"
                            " -- missing from table %zu, psv refused %zu\n",
                            key.c_str(), (size_t)a_tab.n, n_total, cover * 100, n_missing, n_failed);
            continue;
        }
        mv_tab.push_back(v_tab); mv_off.push_back(v_off); mv_on.push_back(v_on);

        if (mdl == 1 && !missing_tab.empty()) {
            std::printf("    (not in somo.residue, excluded from all three:");
            for (auto& s2 : missing_tab) std::printf(" %s", s2.c_str());
            std::printf(")\n");
        }
        if (mdl == 1 && !failed_psv.empty()) {
            std::printf("    (psv refused for:");
            for (auto& s2 : failed_psv) std::printf(" %s", s2.c_str());
            std::printf(")\n");
        }
        }   // ---- end model loop ----

        if (mv_tab.empty()) continue;
        auto mean = [](const std::vector<double>& v) {
            double s2 = 0; for (double x : v) s2 += x; return s2 / v.size(); };
        auto spread = [](const std::vector<double>& v) {
            if (v.size() < 2) return 0.0;
            double lo = v[0], hi = v[0];
            for (double x : v) { lo = std::min(lo, x); hi = std::max(hi, x); }
            return hi - lo; };
        double v_tab = mean(mv_tab), v_off = mean(mv_off), v_on = mean(mv_on);

        auto m = meas.find(key);
        if (m == meas.end()) {
            std::printf("%-22s %8s %8.4f %8.4f %8.4f   %7s %7s %7s\n",
                        key.c_str(), "-", v_tab, v_off, v_on, "-", "-", "-");
        } else {
            double mvv = m->second.psv;
            double dt = (v_tab - mvv) / mvv * 100, dd = (v_off - mvv) / mvv * 100,
                   dp = (v_on - mvv) / mvv * 100;
            et.push_back(dt); ed.push_back(dd); ep.push_back(dp);
            std::printf("%-22s %8.4f %8.4f %8.4f %8.4f   %+7.2f %+7.2f %+7.2f\n",
                        m->second.label.empty() ? key.c_str() : m->second.label.c_str(),
                        mvv, v_tab, v_off, v_on, dt, dd, dp);
        }
        if (mv_tab.size() > 1)
            std::printf("    (%zu MODELs averaged; psv spread across the ensemble:"
                        " table %.4f, D&Z %.4f, D&Z+pH %.4f)\n",
                        mv_tab.size(), spread(mv_tab), spread(mv_off), spread(mv_on));
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
