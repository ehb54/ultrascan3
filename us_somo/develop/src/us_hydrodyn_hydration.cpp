#include "us_hydrodyn_hydration.h"

#include <algorithm>
#include <cmath>

namespace somo_hydration {

Table Table::from_observations(const std::vector<std::pair<std::string, double>>& obs,
                               double confident_at) {
    Table t;
    std::map<std::string, std::map<double, int>> counts;   // hybrid -> waters -> times seen
    for (const auto& o : obs) counts[o.first][o.second]++;

    for (const auto& kv : counts) {
        int total = 0, best_n = 0;
        double best_v = 0;
        for (const auto& c : kv.second) {
            total += c.second;
            // Ties break toward the smaller water count: under-hydrating a proposal the user
            // will edit is safer than over-hydrating one they may accept unchanged.
            if (c.second > best_n || (c.second == best_n && c.first < best_v)) {
                best_n = c.second;
                best_v = c.first;
            }
        }
        Entry e;
        e.waters = best_v;
        e.n_obs = total;
        e.agreement = total ? double(best_n) / total : 0.0;
        e.confident = e.agreement >= confident_at;
        t.by_hybrid_[kv.first] = e;
    }
    return t;
}

const Entry* Table::find(const std::string& hybrid) const {
    auto it = by_hybrid_.find(hybrid);
    return it == by_hybrid_.end() ? nullptr : &it->second;
}

Proposal propose(const Table& t,
                 const std::vector<somo_perceive::InAtom>& atoms,
                 const std::vector<somo_perceive::OutAtom>& perceived,
                 const std::vector<int>& idx) {
    Proposal p;
    if (atoms.size() != perceived.size() || idx.empty()) return p;

    for (int i : idx) {
        const std::string& hyb = perceived[i].hybrid;
        const Entry* e = t.find(hyb);
        if (!e) {
            p.per_atom.push_back(0.0);
            p.review.push_back(atoms[i].name + " (" + hyb +
                               "): type never seen in somo.residue, hydration left at 0");
            continue;
        }
        p.per_atom.push_back(e->waters);
        p.total += e->waters;
        if (!e->confident) {
            char buf[64];
            std::snprintf(buf, sizeof(buf), "%.0f%% of %d", 100 * e->agreement, e->n_obs);
            p.review.push_back(atoms[i].name + " (" + hyb + "): proposed " +
                               std::to_string((long) std::lround(e->waters)) +
                               " but the coded residues agree only " + buf + " -- please check");
        }
    }
    p.ok = true;
    return p;
}

} // namespace somo_hydration
