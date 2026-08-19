// residue_oracle.h -- parse somo.residue.new into (resName, atomName) -> hybrid type.
// Used only for regression scoring (the "ground truth" the perceiver must reproduce).
#pragma once
#include <string>
#include <map>
#include <vector>
#include <fstream>
#include <sstream>

namespace somo_perceive {

struct ResidueOracle {
    // key "RESNAME|ATOMNAME" -> hybrid type
    std::map<std::string, std::string> type_of;
    std::map<std::string, std::vector<std::pair<std::string,std::string>>> residue_atoms;

    static std::vector<std::string> split_ws(const std::string& s){
        std::istringstream ss(s); std::vector<std::string> v; std::string t;
        while (ss>>t) v.push_back(t); return v;
    }
    bool load(const std::string& path){
        std::ifstream f(path); if(!f) return false;
        std::vector<std::string> lines; std::string ln;
        while (std::getline(f,ln)) lines.push_back(ln);
        size_t i=0;
        while (i<lines.size()){
            /*comment*/ ++i; if(i>=lines.size()) break;
            auto hdr=split_ws(lines[i]);
            if (hdr.size()<7){ continue; }   // resync on malformed
            ++i;
            std::string name=hdr[0];
            int numatoms=std::atoi(hdr[4].c_str());
            int numbeads=std::atoi(hdr[5].c_str());
            for (int k=0;k<numatoms && i<lines.size();++k,++i){
                auto p=split_ws(lines[i]);
                if (p.size()<2) continue;
                type_of[name+"|"+p[0]]=p[1];
                residue_atoms[name].push_back({p[0],p[1]});
            }
            i+=numbeads;
        }
        return !type_of.empty();
    }
    const std::string* lookup(const std::string& res, const std::string& atom) const {
        auto it=type_of.find(res+"|"+atom);
        return it==type_of.end()? nullptr : &it->second;
    }
};

} // namespace somo_perceive
