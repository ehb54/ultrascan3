// emit_residue.cpp -- perceive a structure and emit a somo.residue entry (+ any new somo.hybrid
// lines) for a chosen residue, demonstrating "on-the-fly" generation for an unknown residue.
// Usage: emit_residue <pdb> <RESNAME> [resSeq]
#include <cstdio>
#include <fstream>
#include <set>
#include <sstream>
#include <string>
#include <vector>
#include "../pdb_lite.h"
#include "../../include/us_hydrodyn_perceive_hybrid.h"
#include "../../include/us_hydrodyn_residue_builder.h"
#include "../../include/us_hydrodyn_perceive.h"
using namespace somo_perceive;

int main(int argc,char**argv){
    if(argc<3){ std::fprintf(stderr,"usage: emit_residue <pdb> <RESNAME> [resSeq]\n"); return 2; }
    std::string pdb=argv[1], want=argv[2];
    int wantSeq = argc>3 ? std::atoi(argv[3]) : -1;

    HybridTable tbl;
    if(!tbl.load("../../etc/somo.hybrid.new")){ std::fprintf(stderr,"cannot load somo.hybrid.new\n"); return 2; }
    Perceiver perc(tbl);

    auto atoms=strip_altlocs(read_pdb(pdb));
    std::vector<InAtom> in;
    for(auto&a:atoms){ InAtom x; x.element=a.element;x.x=a.x;x.y=a.y;x.z=a.z;
        x.serial=a.serial;x.name=a.name;x.resName=a.resName;x.chain=std::string(1,a.chain);x.resSeq=a.resSeq;x.hetatm=a.hetatm; in.push_back(x);}
    // perceive on the WHOLE structure (context matters for bonds), then slice out the residue.
    // Use authoritative CONECT connectivity when the file provides it (essential for
    // metal-oxo clusters, where metal-ligand bonds are the covalent skeleton).
    std::map<int,int> ser2idx;
    for(size_t i=0;i<in.size();++i) ser2idx[in[i].serial]=(int)i;
    std::vector<std::pair<int,int>> ebonds;
    for(auto& c : read_conect(pdb)){
        auto ia=ser2idx.find(c.first), ib=ser2idx.find(c.second);
        if(ia!=ser2idx.end() && ib!=ser2idx.end()) ebonds.push_back({ia->second, ib->second});
    }
    Bonds bonds;
    auto out=perc.perceive(in, bonds, ebonds);

    // Slice exactly ONE residue instance: the same residue number can repeat across chains
    // (e.g. ACE 0 in chains A-D), so the chain must be part of the key.
    std::vector<InAtom> res_atoms; std::vector<OutAtom> res_out;
    int found_seq=-1; std::string found_chain; bool have=false;
    for(size_t i=0;i<in.size();++i){
        if(in[i].resName!=want) continue;
        if(wantSeq>=0 && in[i].resSeq!=wantSeq) continue;
        if(!have){ found_seq=in[i].resSeq; found_chain=in[i].chain; have=true; }
        if(in[i].resSeq!=found_seq || in[i].chain!=found_chain) continue;
        res_atoms.push_back(in[i]); res_out.push_back(out[i]);
    }
    if(res_atoms.empty()){ std::fprintf(stderr,"residue %s not found in %s\n",want.c_str(),pdb.c_str()); return 1; }

    // PDB headers name non-standard residues (HETNAM) -- use that as the entry's name.
    auto hetnam=read_hetnam(pdb);
    std::string chem = hetnam.count(want) ? hetnam[want] : "";
    if(!chem.empty()) std::printf("(HETNAM header name: \"%s\")\n", chem.c_str());
    // Build the complete entry: perception plus psv, anhydrous volume and proposed hydration.
    // Hydration needs the coded residues as its source, so it is only offered when they load.
    somo_hydration::Table hyd;
    bool have_hyd = false;
    {
        std::vector<std::pair<std::string,double>> obs;
        std::ifstream rf("../../etc/somo.residue.new");
        std::vector<std::string> L; std::string ln;
        while (std::getline(rf, ln)) L.push_back(ln);
        std::set<std::string> seen; size_t li = 0;
        while (li < L.size()) {
            ++li; if (li >= L.size()) break;
            std::istringstream hs(L[li]); std::string nm; int t=0,na=0,nb=0; double mv=0,mw=0,vb=0;
            if(!(hs>>nm>>t>>mv>>mw>>na>>nb>>vb)){continue;} ++li;
            const bool first = seen.insert(nm).second;
            for(int a=0;a<na && li+a<L.size();++a){
                std::vector<std::string> f2; std::string cur; std::istringstream as(L[li+a]);
                while(std::getline(as,cur,'\t')) f2.push_back(cur);
                if(f2.size()<8) continue;
                try { if(first) obs.push_back({f2[1], std::stod(f2[7])}); } catch(...) {}
            }
            li += na + (nb>0?nb:0);
        }
        if(!obs.empty()){ hyd = somo_hydration::Table::from_observations(obs); have_hyd = true; }
    }
    std::vector<int> idx;
    for(size_t i=0;i<in.size();++i)
        if(in[i].resName==want && in[i].resSeq==found_seq && in[i].chain==found_chain)
            idx.push_back((int)i);
    auto built = somo_residue_builder::build(want, in, out, bonds, idx, perc,
                                             have_hyd ? &hyd : nullptr, chem);
    std::printf("===== somo.residue entry (resSeq %d, %zu atoms) =====\n%s",
                found_seq,res_atoms.size(),built.residue_block.c_str());
    if(built.psv.ok)
        std::printf("\n[psv] V %.1f cm^3/mol = atoms %.1f + covolume %.1f - rings %.1f (%d) "
                    "- electrostriction %.1f   -> vbar %.3f\n",
                    built.psv.molar_volume, built.psv.atomic_sum, built.psv.covolume,
                    built.psv.ring_decrement, built.psv.n_rings,
                    built.psv.electrostriction, built.psv.vbar);
    if(built.molvol>0)
        std::printf("[volume] solvent-excluded %.1f A^3 -> %.1f A^3 on the coded-residue basis\n",
                    built.raw_volume, built.molvol);
    auto em=built;
    if(!em.new_hybrids.empty()){
        std::printf("\n===== new somo.hybrid lines (types not already in table) =====\n");
        for(auto&h:em.new_hybrids) std::printf("%s\n",h.c_str());
    } else {
        std::printf("\n(all perceived hybrid types already present in somo.hybrid.new)\n");
    }
    return 0;
}
