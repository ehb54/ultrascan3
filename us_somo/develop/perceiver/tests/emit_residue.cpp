// emit_residue.cpp -- perceive a structure and emit a somo.residue entry (+ any new somo.hybrid
// lines) for a chosen residue, demonstrating "on-the-fly" generation for an unknown residue.
// Usage: emit_residue <pdb> <RESNAME> [resSeq]
#include <cstdio>
#include <string>
#include <vector>
#include "../pdb_lite.h"
#include "../../include/us_hydrodyn_perceive_hybrid.h"
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
    auto out=perc.perceive(in, ebonds);

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
    auto em=perc.emit_residue(want,res_atoms,res_out,chem);
    std::printf("===== somo.residue entry (resSeq %d, %zu atoms) =====\n%s",
                found_seq,res_atoms.size(),em.residue_block.c_str());
    if(!em.new_hybrids.empty()){
        std::printf("\n===== new somo.hybrid lines (types not already in table) =====\n");
        for(auto&h:em.new_hybrids) std::printf("%s\n",h.c_str());
    } else {
        std::printf("\n(all perceived hybrid types already present in somo.hybrid.new)\n");
    }
    return 0;
}
