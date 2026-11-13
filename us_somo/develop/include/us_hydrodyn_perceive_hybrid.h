// hybrid_table.h -- loads somo.hybrid.new and exposes type -> {mw, radius, electrons, ...}
// plus physics-equivalence classes (types with identical mw/radius/electrons).
#pragma once
#include <string>
#include <map>
#include <vector>
#include <fstream>
#include <sstream>
#include <cmath>

namespace somo_perceive {

struct HybridInfo {
    std::string saxs_name;   // col 1: element+charge, e.g. "C", "N+1", "O-1"
    std::string name;        // col 2: hybrid type, e.g. "C4H3"
    double mw = 0;           // col 3
    double radius = 0;       // col 4  (vdW radius)
    double scat_len = 0;     // col 5
    int    exch_prot = 0;    // col 6
    int    num_elect = 0;    // col 7
    int    n_h = 0;          // parsed from name "...H<d>..."
    int    charge = 0;       // parsed from name/saxs suffix (+/-N)
};

class HybridTable {
public:
    bool load(const std::string& path) {
        std::ifstream f(path);
        if (!f) return false;
        std::string line;
        while (std::getline(f, line)) {
            std::istringstream ss(line);
            HybridInfo h;
            if (!(ss >> h.saxs_name >> h.name >> h.mw >> h.radius >> h.scat_len
                     >> h.exch_prot >> h.num_elect)) continue;
            h.n_h    = parse_h(h.name);
            h.charge = parse_charge(h.name);
            by_name_[h.name] = h;
        }
        build_equiv();
        return !by_name_.empty();
    }

    bool has(const std::string& name) const { return by_name_.count(name) != 0; }
    const HybridInfo* get(const std::string& name) const {
        auto it = by_name_.find(name);
        return it == by_name_.end() ? nullptr : &it->second;
    }
    const std::map<std::string, HybridInfo>& all() const { return by_name_; }

    // Physics-equivalence key: rounded (mw, radius, num_elect). Two names sharing this key are
    // interchangeable for hydrodynamics (e.g. N2H0 <-> N3H0).
    std::string phys_key_of(const std::string& name) const {
        auto it = by_name_.find(name);
        if (it == by_name_.end()) return "?" + name;
        return phys_key(it->second);
    }
    bool phys_equal(const std::string& a, const std::string& b) const {
        if (a == b) return true;
        if (!has(a) || !has(b)) return false;
        return phys_key_of(a) == phys_key_of(b);
    }

    static std::string phys_key(const HybridInfo& h) {
        std::ostringstream o;
        o << std::llround(h.mw * 100) << "|" << std::llround(h.radius * 100)
          << "|" << h.num_elect;
        return o.str();
    }

private:
    std::map<std::string, HybridInfo> by_name_;
    std::map<std::string, std::vector<std::string>> equiv_; // phys_key -> names

    void build_equiv() {
        equiv_.clear();
        for (auto& kv : by_name_) equiv_[phys_key(kv.second)].push_back(kv.first);
    }
    static int parse_h(const std::string& name) {
        // find 'H' followed by a digit
        for (size_t i = 0; i + 1 < name.size(); ++i)
            if (name[i] == 'H' && std::isdigit((unsigned char)name[i+1]))
                return name[i+1] - '0';
        return 0;
    }
    static int parse_charge(const std::string& name) {
        // trailing +N / -N (N optional, default 1)
        for (size_t i = 0; i < name.size(); ++i) {
            if (name[i] == '+' || name[i] == '-') {
                int sign = name[i] == '+' ? 1 : -1;
                if (i + 1 < name.size() && std::isdigit((unsigned char)name[i+1]))
                    return sign * (name[i+1] - '0');
                return sign * 1;
            }
        }
        return 0;
    }
};

} // namespace somo_perceive
