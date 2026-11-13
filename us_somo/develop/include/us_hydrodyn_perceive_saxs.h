// saxs_table.h -- minimal reader for somo.saxs_atoms (Cromer-Mann / Waasmaier-Kirfel keys).
// We only need the KEY SET to answer "does this atom have SAXS coefficients?", which is the
// bottleneck Mattia flagged: an atom with no hybridization AND/OR no SAXS coefficient.
#pragma once
#include <string>
#include <set>
#include <fstream>
#include <sstream>
#include <cctype>

namespace somo_perceive {

class SaxsTable {
public:
    bool load(const std::string& path){
        std::ifstream f(path); if(!f) return false;
        std::string line;
        while(std::getline(f,line)){
            std::istringstream ss(line); std::string k;
            if(ss>>k) keys_.insert(k);
        }
        return !keys_.empty();
    }
    bool has(const std::string& key) const { return keys_.count(key)!=0; }
    const std::set<std::string>& keys() const { return keys_; }

    // somo.saxs_atoms is inconsistent about charge spelling (it carries BOTH "O-1" and "O1-",
    // "O-2" and "O2-", but only "CO3+" for cobalt(III) while somo.hybrid.new says "CO+3").
    // Try the literal key first, then the transposed charge spelling.
    std::string resolve(const std::string& saxs_name) const {
        if(has(saxs_name)) return saxs_name;
        // "XX+n" <-> "XXn+"
        size_t p = saxs_name.find_first_of("+-");
        if(p!=std::string::npos && p+1<saxs_name.size() && std::isdigit((unsigned char)saxs_name[p+1])){
            std::string alt = saxs_name.substr(0,p) + saxs_name[p+1] + saxs_name[p];
            if(has(alt)) return alt;
        }
        // "XXn+" -> "XX+n"
        if(saxs_name.size()>=2){
            char last=saxs_name.back();
            if((last=='+'||last=='-') && std::isdigit((unsigned char)saxs_name[saxs_name.size()-2])){
                std::string alt = saxs_name.substr(0,saxs_name.size()-2) + last + saxs_name[saxs_name.size()-2];
                if(has(alt)) return alt;
            }
        }
        // bare element fallback (neutral form)
        std::string bare; for(char c:saxs_name){ if(std::isalpha((unsigned char)c)) bare+=c; else break; }
        if(has(bare)) return bare;
        return "";   // unresolvable
    }
private:
    std::set<std::string> keys_;
};

} // namespace somo_perceive
