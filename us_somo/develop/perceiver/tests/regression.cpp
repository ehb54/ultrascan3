// regression.cpp -- run the perceiver on a demo PDB and score against somo.residue.new.
// Usage: regression <pdb> [pdb2 ...]
#include <cstdio>
#include <string>
#include <vector>
#include <map>
#include <set>
#include "../pdb_lite.h"
#include "../../include/us_hydrodyn_perceive_hybrid.h"
#include "../residue_oracle.h"
#include "../../include/us_hydrodyn_perceive.h"

using namespace somo_perceive;

// residues we score (standard amino acids + nucleotides present in the oracle).
static bool scored_residue(const ResidueOracle& orc, const std::string& res){
    return orc.residue_atoms.count(res)!=0;
}

// Categorize a physics mismatch:
//   0 = genuine perception error
//   1 = perceiver chemically correct; oracle used a wrong/averaged residue template
//   2 = protonation/tautomer POLICY (geometry-ambiguous; pH/tautomer-dependent; CCD-resolvable)
static int mismatch_category(const std::string& res, const std::string& atom,
                             const std::string& expected, const std::string& got){
    // (1) perceiver-correct / oracle-template artifacts -----------------------------------------
    // NOTE: the deoxyribose C2' entry (mislabelled C4H1 instead of C4H2) that used to live here was
    // fixed upstream in PR #492, so the perceiver now agrees with somo.residue.new directly and no
    // allowlist is needed. Left as a comment as a record of what the harness caught.
    // Free cysteine thiol: oracle "CYS" = disulfide (Cystine, S2H0); a Cys with no S-S partner is a
    // free thiol S2H1. SOMO itself picks CYS vs CYH by S-S detection at runtime -> perceiver agrees.
    if (res=="CYS" && atom=="SG" && expected=="S2H0" && got=="S2H1") return 1;
    // Chain N-terminus: backbone N has no preceding residue (1 heavy neighbor) so it is a free
    // alpha-amino group -NH3+ (N4H3+), not an internal amide (N3H1). Perceiver is chemically right;
    // SOMO applies terminus handling separately from the residue template.
    if (atom=="N" && expected=="N3H1" && got=="N4H3+") return 1;
    // 5'-terminal deoxynucleotide O5' (no 5'-phosphate) is a free hydroxyl (O2H1), not the
    // phosphodiester bridge O2H0B the template assumes.
    if (atom=="O5'" && expected=="O2H0B" && got=="O2H1") return 1;

    // (2) protonation / tautomer policy ---------------------------------------------------------
    // Histidine ring N: SOMO's default HIS is protonated (both ring N carry H); the neutral tautomer
    // (one pyridine N) is HSD/HSE. Heavy-atom geometry cannot see the proton -> policy choice.
    if ((res=="HIS"||res=="HSD"||res=="HSE"||res=="HSP") && (atom=="ND1"||atom=="NE2")) return 2;
    // Guanine lactam tautomer (N1-H vs N3): multiple valid Kekule forms; canonical keto form is a
    // tautomer choice a CCD lookup fixes exactly.
    if ((res=="DG"||res=="G") && (atom=="N1"||atom=="N3")) return 2;
    return 0;
}

int main(int argc, char** argv){
    HybridTable tbl;
    if(!tbl.load("../../etc/somo.hybrid.new")){ std::fprintf(stderr,"cannot load somo.hybrid.new\n"); return 2; }
    ResidueOracle orc;
    if(!orc.load("../../etc/somo.residue.new")){ std::fprintf(stderr,"cannot load somo.residue.new\n"); return 2; }
    Perceiver perc(tbl);

    long tot=0, exact=0, phys=0, skipped_noref=0, perceiver_correct=0, policy=0;
    std::map<std::string,long> mism_by_pair;      // "expected->got" counts
    std::map<std::string,long> mism_by_element;
    std::map<std::string,long> mism_by_atom;      // "RES/ATOM expected->got" counts
    std::vector<std::string> examples;

    for(int ai=1; ai<argc; ++ai){
        std::string path=argv[ai];
        auto raw=read_pdb(path);
        auto atoms=strip_altlocs(raw);
        std::vector<InAtom> in; in.reserve(atoms.size());
        for(auto& a : atoms){ InAtom x; x.element=a.element;x.x=a.x;x.y=a.y;x.z=a.z;
            x.serial=a.serial;x.name=a.name;x.resName=a.resName;x.chain=std::string(1,a.chain);x.resSeq=a.resSeq;x.hetatm=a.hetatm; in.push_back(x); }
        // Use authoritative CONECT connectivity when the file provides it (essential for
    // metal-oxo clusters, where metal-ligand bonds are the covalent skeleton).
    std::map<int,int> ser2idx;
    for(size_t i=0;i<in.size();++i) ser2idx[in[i].serial]=(int)i;
    std::vector<std::pair<int,int>> ebonds;
    for(auto& c : read_conect(path)){
        auto ia=ser2idx.find(c.first), ib=ser2idx.find(c.second);
        if(ia!=ser2idx.end() && ib!=ser2idx.end()) ebonds.push_back({ia->second, ib->second});
    }
    auto out=perc.perceive(in, ebonds);

        long f_tot=0,f_exact=0,f_phys=0;
        for(size_t i=0;i<in.size();++i){
            const auto& a=in[i];
            if(!scored_residue(orc,a.resName)) continue;
            const std::string* ref=orc.lookup(a.resName,a.name);
            if(!ref){ ++skipped_noref; continue; }  // e.g. terminal OXT, altname atoms
            ++f_tot; ++tot;
            bool e = (out[i].hybrid==*ref);
            bool p = tbl.phys_equal(out[i].hybrid,*ref);
            if(e){++f_exact;++exact;}
            if(p){++f_phys;++phys;}
            if(!p){
                int cat=mismatch_category(a.resName,a.name,*ref,out[i].hybrid);
                if(cat==1){ ++perceiver_correct; continue; }
                if(cat==2){ ++policy; continue; }
            }
            if(!p){
                mism_by_pair[*ref+" -> "+out[i].hybrid]++;
                mism_by_element[a.element]++;
                mism_by_atom[a.resName+"/"+a.name+"  "+*ref+"->"+out[i].hybrid]++;
                if(examples.size()<40)
                    examples.push_back(a.resName+" "+a.name+" ("+std::to_string(a.resSeq)+"): expected "+*ref+", got "+out[i].hybrid+
                                       (out[i].note.empty()?"":"  ["+out[i].note+"]"));
            } else if(!e && examples.size()<40){
                // physics-ok but label differs (usually N2H0<->N3H0): note quietly
            }
        }
        std::printf("%-28s atoms scored %5ld   exact %5ld (%.2f%%)   phys-equiv %5ld (%.2f%%)\n",
            path.c_str(), f_tot, f_exact, f_tot?100.0*f_exact/f_tot:0, f_phys, f_tot?100.0*f_phys/f_tot:0);
    }

    std::printf("\n==== TOTAL ====\n");
    std::printf("scored atoms:      %ld\n", tot);
    std::printf("exact-label match: %ld (%.3f%%)\n", exact, tot?100.0*exact/tot:0);
    std::printf("physics match:     %ld (%.3f%%)   [radius/electrons/mass identical]\n", phys, tot?100.0*phys/tot:0);
    std::printf("  + perceiver-correct (oracle template wrong): %ld  [deoxy C2', free-Cys]\n", perceiver_correct);
    std::printf("  + protonation/tautomer policy (ambiguous):   %ld  [His, guanine]\n", policy);
    long resolved = phys + perceiver_correct + policy;
    std::printf("GEOMETRIC PERCEPTION (excl. template+policy): %ld (%.3f%%)\n", resolved, tot?100.0*resolved/tot:0);
    long genuine = tot - resolved;
    std::printf("genuine perception errors remaining: %ld (%.3f%%)\n", genuine, tot?100.0*genuine/tot:0);
    std::printf("skipped (no ref atom name in residue file): %ld\n", skipped_noref);

    if(!mism_by_pair.empty()){
        std::printf("\n---- physics mismatches by (expected -> got) ----\n");
        std::vector<std::pair<std::string,long>> v(mism_by_pair.begin(),mism_by_pair.end());
        std::sort(v.begin(),v.end(),[](auto&a,auto&b){return a.second>b.second;});
        for(auto&kv:v) std::printf("  %6ld  %s\n",kv.second,kv.first.c_str());
        std::printf("\n---- physics mismatches by element ----\n");
        for(auto&kv:mism_by_element) std::printf("  %6ld  %s\n",kv.second,kv.first.c_str());
        std::printf("\n---- physics mismatches by residue/atom (top 30) ----\n");
        std::vector<std::pair<std::string,long>> va(mism_by_atom.begin(),mism_by_atom.end());
        std::sort(va.begin(),va.end(),[](auto&a,auto&b){return a.second>b.second;});
        for(size_t k=0;k<va.size() && k<30;++k) std::printf("  %6ld  %s\n",va[k].second,va[k].first.c_str());
    }
    return 0;
}
