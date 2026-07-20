// coverage.cpp -- audit the "no hybridization and/or no SAXS coefficient" bottleneck.
//
// (A) Cross-validate somo.hybrid.new against somo.saxs_atoms: every hybrid type's saxs_name must
//     resolve to a SAXS entry, else SAXS breaks for any atom of that type.
// (B) For given structures, perceive every atom and report which would hit the bottleneck.
// (C) Report which commonly-encountered PDB elements are supported at all.
//
// Usage: coverage [pdb ...]
#include <cstdio>
#include <string>
#include <vector>
#include <map>
#include <set>
#include "../pdb_lite.h"
#include "../../include/us_hydrodyn_perceive_hybrid.h"
#include "../../include/us_hydrodyn_perceive_saxs.h"
#include "../../include/us_hydrodyn_perceive.h"
using namespace somo_perceive;

int main(int argc,char**argv){
    HybridTable tbl; SaxsTable saxs;
    if(!tbl.load("../../etc/somo.hybrid.new")){ std::fprintf(stderr,"no somo.hybrid.new\n"); return 2; }
    if(!saxs.load("../../etc/somo.saxs_atoms.new") && !saxs.load("data/somo.saxs_atoms.new")){
        std::fprintf(stderr,"no somo.saxs_atoms.new\n"); return 2; }

    std::printf("==== (A) somo.hybrid.new -> somo.saxs_atoms resolution ====\n");
    int bad=0, fixed_by_alt=0;
    for(auto&kv : tbl.all()){
        const std::string& sn = kv.second.saxs_name;
        std::string r = saxs.resolve(sn);
        if(r.empty()){
            std::printf("  BROKEN  hybrid %-8s saxs_name %-6s -> NO SAXS ENTRY\n",
                        kv.first.c_str(), sn.c_str());
            ++bad;
        } else if(r!=sn){
            std::printf("  spelling hybrid %-8s saxs_name %-6s -> resolved only as '%s'\n",
                        kv.first.c_str(), sn.c_str(), r.c_str());
            ++fixed_by_alt;
        }
    }
    std::printf("  => %d hybrid types with NO SAXS entry, %d resolved only via alternate spelling\n",
                bad, fixed_by_alt);

    std::printf("\n==== (C) element support across the tables ====\n");
    const char* common[] = {"H","C","N","O","F","NA","MG","AL","SI","P","S","CL","K","CA","MN",
        "FE","CO","NI","CU","ZN","SE","BR","MO","PD","AG","CD","I","W","PT","AU","HG","AS","LI",
        "B","BE","SR","BA","CS","RB","CR","V","TI","SN","PB",0};
    // element part of a table key ("FE+2" -> "FE", "O1-" -> "O")
    auto elem_of=[](const std::string& s){ std::string b;
        for(char c:s){ if(isalpha((unsigned char)c)) b+=(char)toupper(c); else break; } return b; };
    std::set<std::string> saxs_elems, hyb_elems;
    for(auto&k : saxs.keys())      saxs_elems.insert(elem_of(k));
    for(auto&kv: tbl.all())        hyb_elems.insert(elem_of(kv.second.saxs_name));

    std::printf("  %-4s %-9s %-9s\n","el","hybrid?","saxs?");
    std::vector<std::string> unsupported;
    for(int i=0;common[i];++i){
        std::string e=common[i];
        bool h = hyb_elems.count(e)!=0;
        bool s = saxs_elems.count(e)!=0;
        if(!h||!s){
            std::printf("  %-4s %-9s %-9s%s\n",e.c_str(),h?"yes":"NO",s?"yes":"NO",
                        (!h&&!s)?"   <- fully unsupported":"");
            if(!h&&!s) unsupported.push_back(e);
        }
    }
    std::printf("  => fully unsupported (would need BOTH a hybrid type and SAXS coefficients): ");
    for(auto&u:unsupported) std::printf("%s ",u.c_str());
    std::printf("\n");

    if(argc<2){ std::printf("\n(pass PDB files to also scan structures)\n"); return 0; }

    std::printf("\n==== (B) per-structure bottleneck scan ====\n");
    Perceiver perc(tbl);
    std::map<std::string,long> bottleneck;   // "element/hybrid reason" -> count
    for(int ai=1;ai<argc;++ai){
        auto atoms=strip_altlocs(read_pdb(argv[ai]));
        std::vector<InAtom> in;
        for(auto&a:atoms){ InAtom x; x.element=a.element;x.x=a.x;x.y=a.y;x.z=a.z;
            x.serial=a.serial;x.name=a.name;x.resName=a.resName;x.resSeq=a.resSeq;x.hetatm=a.hetatm; in.push_back(x);}
        // Use authoritative CONECT connectivity when the file provides it (essential for
    // metal-oxo clusters, where metal-ligand bonds are the covalent skeleton).
    std::map<int,int> ser2idx;
    for(size_t i=0;i<in.size();++i) ser2idx[in[i].serial]=(int)i;
    std::vector<std::pair<int,int>> ebonds;
    for(auto& c : read_conect(argv[ai])){
        auto ia=ser2idx.find(c.first), ib=ser2idx.find(c.second);
        if(ia!=ser2idx.end() && ib!=ser2idx.end()) ebonds.push_back({ia->second, ib->second});
    }
    auto out=perc.perceive(in, ebonds);
        long nb=0;
        for(size_t i=0;i<out.size();++i){
            bool no_hyb = !out[i].in_table;
            std::string sn = out[i].in_table ? tbl.get(out[i].hybrid)->saxs_name : out[i].element;
            bool no_saxs = saxs.resolve(sn).empty();
            if(no_hyb||no_saxs){
                ++nb;
                std::string why = std::string(no_hyb?"no-hybrid":"") + (no_hyb&&no_saxs?"+":"") + (no_saxs?"no-saxs":"");
                bottleneck[in[i].resName+"/"+in[i].name+" ["+out[i].element+" -> "+out[i].hybrid+"] "+why]++;
            }
        }
        std::printf("  %-28s %6zu atoms, %ld bottleneck\n", argv[ai], in.size(), nb);
    }
    if(!bottleneck.empty()){
        std::printf("\n  atoms that would fail (hybrid and/or SAXS):\n");
        for(auto&kv:bottleneck) std::printf("   %6ld  %s\n",kv.second,kv.first.c_str());
    } else std::printf("\n  no bottleneck atoms in the scanned structures.\n");
    return 0;
}
