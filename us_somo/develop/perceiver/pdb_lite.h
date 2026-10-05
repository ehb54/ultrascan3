// pdb_lite.h -- minimal PDB ATOM/HETATM reader for testing the perceiver.
// Not the production parser (SOMO has its own); just enough to feed coordinates.
#pragma once
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <cctype>
#include "us_hydrodyn_perceive_elements.h"

namespace somo_perceive {

struct PdbAtom {
    int         serial = 0;
    std::string name;       // atom name, e.g. "CA", "N", "OD1"
    char        altLoc = ' ';
    std::string resName;    // "ALA"
    char        chain = ' ';
    int         resSeq = 0;
    char        iCode = ' ';
    double      x = 0, y = 0, z = 0;
    double      occ = 1.0;
    std::string element;    // normalized symbol from col 77-78 (or guessed)
    bool        hetatm = false;
};

// Guess element from a 4-char PDB atom name when the element column is absent.
inline std::string element_from_name(const std::string& rawname, bool hetatm) {
    // PDB atom name field is columns 13-16. For single-char elements the symbol is usually in
    // col 14 (name right-shifted). Two-letter elements (FE, ZN) start in col 13.
    std::string n;
    for (char c : rawname) if (!std::isspace((unsigned char)c)) n += c;
    if (n.empty()) return "";
    // strip leading digits/altloc markers
    size_t i = 0; while (i < n.size() && std::isdigit((unsigned char)n[i])) ++i;
    std::string s = n.substr(i);
    if (s.empty()) return "";
    // two-letter element candidates (mostly HETATM metals/ions)
    if (s.size() >= 2) {
        std::string two = norm_element(s.substr(0,2));
        if (hetatm && has_element(two) && atomic_number(two) > 0 &&
            two != "CA" ) // CA in protein is C-alpha; only trust 2-letter for hetatm & real 2-letter symbols
            if (two=="FE"||two=="ZN"||two=="MG"||two=="MN"||two=="NA"||two=="CL"||
                two=="CU"||two=="CO"||two=="CD"||two=="MO"||two=="PD"||two=="SE"||
                two=="BR"||two=="BE"||two=="AL"||two=="SI")
                return two;
    }
    std::string one = norm_element(s.substr(0,1));
    return one;
}

inline std::string trim(const std::string& s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == std::string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

// Read HETNAM records -> hetID -> full chemical name. Real PDB files name every non-standard
// residue in the header, which gives a far better label for a generated somo.residue entry than
// the bare 3-letter code. Continuation records (cols 9-10) are concatenated.
//   COLUMNS  9-10 continuation | 12-14 hetID | 16-70 chemical name
inline std::map<std::string,std::string> read_hetnam(const std::string& path){
    std::map<std::string,std::string> out;
    std::ifstream f(path); if(!f) return out;
    std::string line;
    while(std::getline(f,line)){
        if(line.rfind("HETNAM",0)!=0) continue;
        if(line.size()<16) continue;
        std::string id  = line.substr(11, std::min<size_t>(3, line.size()-11));
        std::string txt = line.size()>15 ? line.substr(15) : "";
        // trim
        auto trim_s=[](std::string s){
            size_t a=s.find_first_not_of(" \t\r\n"); if(a==std::string::npos) return std::string();
            size_t b=s.find_last_not_of(" \t\r\n"); return s.substr(a,b-a+1); };
        id=trim_s(id); txt=trim_s(txt);
        if(id.empty()||txt.empty()) continue;
        if(out.count(id)) out[id] += " " + txt;   // continuation line
        else              out[id]  = txt;
    }
    return out;
}

// Read CONECT records -> explicit bonds as (serial, serial) pairs. PDB gives these for HETATM /
// non-standard groups, and they are authoritative connectivity: for things like metal-oxo clusters
// the metal-ligand bonds ARE the covalent skeleton and distance heuristics alone get it wrong.
// An atom may span several CONECT lines; each line lists the origin then up to 4 partners.
inline std::vector<std::pair<int,int>> read_conect(const std::string& path){
    std::vector<std::pair<int,int>> out;
    std::ifstream f(path); if(!f) return out;
    std::string line;
    while(std::getline(f,line)){
        if(line.rfind("CONECT",0)!=0) continue;
        auto fld=[&](int start)->int{
            if((int)line.size()<start+1) return 0;
            std::string s=line.substr(start, std::min<size_t>(5, line.size()-start));
            size_t a=s.find_first_not_of(" \t"); if(a==std::string::npos) return 0;
            return std::atoi(s.c_str());
        };
        int origin=fld(6);
        if(origin<=0) continue;
        for(int k=0;k<4;++k){
            int p=fld(11+5*k);
            if(p>0) out.push_back({origin,p});
        }
    }
    return out;
}

// Read a PDB file. keep_first_model=true stops at ENDMDL of the first MODEL.
inline std::vector<PdbAtom> read_pdb(const std::string& path, bool keep_first_model = true) {
    std::vector<PdbAtom> atoms;
    std::ifstream f(path);
    if (!f) return atoms;
    std::string line;
    bool seen_model = false;
    while (std::getline(f, line)) {
        if (line.rfind("MODEL", 0) == 0) { if (seen_model && keep_first_model) break; seen_model = true; continue; }
        if (line.rfind("ENDMDL", 0) == 0) { if (keep_first_model) break; continue; }
        bool is_atom = line.rfind("ATOM", 0) == 0;
        bool is_het  = line.rfind("HETATM", 0) == 0;
        if (!is_atom && !is_het) continue;
        if (line.size() < 54) continue;
        PdbAtom a;
        a.hetatm = is_het;
        auto col = [&](int start, int len) -> std::string {
            if ((int)line.size() < start) return "";
            return line.substr(start, std::min((size_t)len, line.size() - start));
        };
        a.serial  = std::atoi(trim(col(6,5)).c_str());
        a.name    = trim(col(12,4));
        std::string alt = col(16,1); a.altLoc = alt.empty()? ' ' : alt[0];
        a.resName = trim(col(17,3));
        std::string ch = col(21,1); a.chain = ch.empty()? ' ' : ch[0];
        a.resSeq  = std::atoi(trim(col(22,4)).c_str());
        std::string ic = col(26,1); a.iCode = ic.empty()? ' ' : ic[0];
        a.x = std::atof(trim(col(30,8)).c_str());
        a.y = std::atof(trim(col(38,8)).c_str());
        a.z = std::atof(trim(col(46,8)).c_str());
        std::string oc = trim(col(54,6)); a.occ = oc.empty()? 1.0 : std::atof(oc.c_str());
        std::string elem = line.size() >= 78 ? trim(col(76,2)) : "";
        a.element = elem.empty() ? element_from_name(a.name, is_het) : norm_element(elem);
        atoms.push_back(a);
    }
    return atoms;
}

// Drop alternate locations: keep altLoc ' ' or 'A' (highest occupancy would be better; 'A' is a
// safe default for these demo structures).
inline std::vector<PdbAtom> strip_altlocs(const std::vector<PdbAtom>& in) {
    std::vector<PdbAtom> out;
    for (auto& a : in) if (a.altLoc == ' ' || a.altLoc == 'A') out.push_back(a);
    return out;
}

} // namespace somo_perceive
