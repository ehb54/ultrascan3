// tests_unit.cpp -- synthetic-geometry unit tests for the perceiver.
#include <sstream>
#include <string>
#include <vector>
#include "../tinytest.h"
#include "../../include/us_hydrodyn_perceive.h"
#include "../../include/us_hydrodyn_perceive_hybrid.h"

using namespace somo_perceive;

static HybridTable& table(){
    static HybridTable t; static bool ok = t.load("../../etc/somo.hybrid.new");
    (void)ok; return t;
}
static InAtom A(const std::string& e,double x,double y,double z){ InAtom a; a.element=e;a.x=x;a.y=y;a.z=z; return a; }
static std::vector<OutAtom> run(const std::vector<InAtom>& v){ return Perceiver(table()).perceive(v); }

TEST("hybrid table loads"){ REQUIRE(!table().all().empty()); CHECK(table().has("C4H3")); }

TEST("ethane methyls -> C4H3"){
    auto o=run({A("C",0,0,0),A("C",1.52,0,0)});
    CHECK_EQ(o[0].hybrid,std::string("C4H3"));
    CHECK_EQ(o[1].hybrid,std::string("C4H3"));
}

TEST("benzene ring carbons -> C3H1"){
    std::vector<InAtom> v; double R=1.40;
    for(int k=0;k<6;++k){ double a=k*M_PI/3.0; v.push_back(A("C",R*std::cos(a),R*std::sin(a),0)); }
    auto o=run(v);
    for(int k=0;k<6;++k) CHECK_EQ(o[k].hybrid,std::string("C3H1"));
}

TEST("acetate: carboxyl C sp2, O split O1H0/O2H1"){
    // C_me, C_cx, O1(short), O2(long)
    auto o=run({A("C",0,0,0), A("C",1.52,0,0), A("O",2.15,1.03,0), A("O",2.17,-1.06,0)});
    CHECK_EQ(o[1].hybrid,std::string("C3H0"));   // carboxyl carbon
    // shorter C-O is the =O
    CHECK_EQ(o[2].hybrid,std::string("O1H0"));
    CHECK_EQ(o[3].hybrid,std::string("O2H1"));
    CHECK_EQ(o[0].hybrid,std::string("C4H3"));   // methyl
}

TEST("planar 3-coordinate C -> C3H0; tetrahedral 3-coordinate C -> C4H1"){
    // planar (all z=0)
    auto p=run({A("C",0,0,0),A("C",1.5,0,0),A("C",-0.75,1.3,0),A("C",-0.75,-1.3,0)});
    CHECK_EQ(p[0].hybrid,std::string("C3H0"));
    // tetrahedral basal 3 neighbors, center out of plane
    double s=0.866;
    auto t=run({A("C",0,0,0),A("C",s,s,s),A("C",s,-s,-s),A("C",-s,s,-s)});
    CHECK_EQ(t[0].hybrid,std::string("C4H1"));
}

TEST("sulfur: thioether S2H0, free thiol S2H1, disulfide both S2H0"){
    auto met=run({A("C",0,0,0),A("S",1.8,0,0),A("C",2.5,1.6,0)});          // C-S-C
    CHECK_EQ(met[1].hybrid,std::string("S2H0"));
    auto thi=run({A("C",0,0,0),A("S",1.8,0,0)});                            // C-S(H)
    CHECK_EQ(thi[1].hybrid,std::string("S2H1"));
    auto ss=run({A("C",0,0,0),A("S",1.8,0,0),A("S",3.85,0,0),A("C",5.65,0,0)}); // C-S-S-C
    CHECK_EQ(ss[1].hybrid,std::string("S2H0"));
    CHECK_EQ(ss[2].hybrid,std::string("S2H0"));
}

TEST("oxygen: hydroxyl O2H1, ether O2H0, water O2H2"){
    auto oh=run({A("C",0,0,0),A("C",1.52,0,0),A("O",2.2,1.25,0)});   // C-C-OH, C-O 1.43
    CHECK_EQ(oh[2].hybrid,std::string("O2H1"));
    auto et=run({A("C",0,0,0),A("O",1.43,0,0),A("C",2.86,0,0)});     // C-O-C
    CHECK_EQ(et[1].hybrid,std::string("O2H0"));
    auto w=run({A("O",0,0,0)});
    CHECK_EQ(w[0].hybrid,std::string("O2H2"));
}

TEST("nitrogen: Lys-like amine N4H3+, amide NH2 N3H2"){
    // sp3 carbon neighbor -> aliphatic amine.  NZ-CE-CD with tetrahedral CE-angle (~110deg).
    auto lys=run({A("N",0,0,0),A("C",1.48,0,0),A("C",2.0,1.4,0)}); // NZ, CE(sp3, 2 heavy nb), CD
    CHECK_EQ(lys[0].hybrid,std::string("N4H3+"));
    // sp2 (carbonyl) carbon neighbor -> amide NH2
    auto amide=run({A("N",0,0,0),A("C",1.33,0,0),A("O",1.9,1.05,0),A("C",1.9,-1.05,0)});
    CHECK_EQ(amide[0].hybrid,std::string("N3H2"));
}

TEST("backbone amide N -> N3H1 (2 heavy nb incl carbonyl C)"){
    // Cprev(=Oprev) - N - CA
    auto o=run({
        A("N",0,0,0), A("C",-1.33,0,0), A("O",-1.9,1.05,0),   // Cprev carbonyl
        A("C",1.46,0.2,0)                                     // CA
    });
    CHECK_EQ(o[0].hybrid,std::string("N3H1"));
}

TEST("metal ion Zn -> ZN+2, isolated (no bonds inflate neighbors)"){
    auto o=run({A("ZN",0,0,0),A("O",2.1,0,0),A("O",-2.1,0,0)}); // Zn coordinated by 2 O at 2.1
    CHECK_EQ(o[0].hybrid,std::string("ZN+2"));
    // the O's should NOT count Zn as a neighbor
    CHECK_EQ(o[1].heavy_neighbors,0);
}

TEST("phosphate P4H0 + terminal O1H0 + bridging O2H0B"){
    // P with 3 terminal O + 1 bridging O(-C)
    auto o=run({
        A("P",0,0,0), A("O",1.5,0,0), A("O",-0.75,1.3,0), A("O",-0.75,-1.3,0), // terminal
        A("O",0,0,1.6), A("C",0,0,3.0)                                          // bridging O-C
    });
    CHECK_EQ(o[0].hybrid,std::string("P4H0"));
    CHECK_EQ(o[1].hybrid,std::string("O1H0"));
    CHECK_EQ(o[4].hybrid,std::string("O2H0B")); // bridges to P
}

static std::vector<InAtom> ngon(int n,double R,const std::vector<std::string>& elems){
    std::vector<InAtom> v;
    for(int k=0;k<n;++k){ double a=2*M_PI*k/n; v.push_back(A(elems[k],R*std::cos(a),R*std::sin(a),0)); }
    return v;
}

TEST("Kekule: pyridine N -> N2H0 (no H)"){
    // planar 6-ring, one N; all 5 carbons need a ring double bond => N must be matched => pyridine
    auto o=run(ngon(6,1.39,{"N","C","C","C","C","C"}));
    CHECK_EQ(o[0].hybrid,std::string("N2H0"));
}
TEST("Kekule: pyrrole N -> N3H1 (has H)"){
    // planar 5-ring, one N; 4 carbons pair up, N left single-bonded => pyrrole (H)
    auto o=run(ngon(5,1.17,{"N","C","C","C","C"}));
    CHECK_EQ(o[0].hybrid,std::string("N3H1"));
}
TEST("Kekule: imidazole -> one pyridine N2H0 + one pyrrole N3H1"){
    auto o=run(ngon(5,1.17,{"N","C","N","C","C"})); // N at 0 and 2
    std::string t0=o[0].hybrid, t2=o[2].hybrid;
    CHECK( (t0=="N2H0"&&t2=="N3H1") || (t0=="N3H1"&&t2=="N2H0") );
}
TEST("carbonyl O stays O1H0 even when C=O drawn long (hybridization-keyed)"){
    // sp2 carbon (3 planar heavy nb) with a single terminal O at a long 1.33 A distance
    auto o=run({A("C",0,0,0),A("C",1.5,0,0),A("C",-0.75,1.3,0),A("O",-0.75,-1.33,0)});
    CHECK_EQ(o[3].hybrid,std::string("O1H0"));   // amide/backbone carbonyl, not hydroxyl
}
TEST("phenol OH on aromatic C -> O2H1"){
    auto v=ngon(6,1.39,{"C","C","C","C","C","C"});
    v.push_back(A("O",2.75,0,0));  // O on ring C0 at ~1.36
    auto o=run(v);
    CHECK_EQ(o[6].hybrid,std::string("O2H1"));
}

TEST("metal stays isolated without explicit bonds (protein Zn site behaviour)"){
    auto o=run({A("ZN",0,0,0),A("O",2.1,0,0),A("O",-2.1,0,0)});
    CHECK_EQ(o[0].hybrid,std::string("ZN+2"));
    CHECK_EQ(o[1].heavy_neighbors,0);      // Zn must not inflate the O's neighbour count
}

TEST("explicit CONECT bonds make a metal-oxo cluster perceive correctly"){
    // Mo with a short terminal oxo (M=O) and a long aqua ligand (M<-OH2).
    std::vector<InAtom> v={A("MO",0,0,0), A("O",1.70,0,0), A("O",0,2.25,0)};
    std::vector<std::pair<int,int>> bonds={{0,1},{0,2}};
    auto o=Perceiver(table()).perceive(v,bonds);
    CHECK_EQ(o[0].hybrid,std::string("MO+3"));
    CHECK_EQ(o[1].hybrid,std::string("O1H0"));   // short M=O => terminal oxo, no H
    CHECK_EQ(o[2].hybrid,std::string("O2H2"));   // long dative bond => aqua ligand
    // without the explicit bonds the same oxygens would be seen as isolated water
    auto iso=Perceiver(table()).perceive(v);
    CHECK_EQ(iso[1].hybrid,std::string("O2H2"));
}

TEST("bridging oxo between two metals -> O2H0 (no H)"){
    std::vector<InAtom> v={A("MO",0,0,0), A("MO",3.6,0,0), A("O",1.8,0.6,0)};
    std::vector<std::pair<int,int>> bonds={{0,2},{1,2}};
    auto o=Perceiver(table()).perceive(v,bonds);
    CHECK_EQ(o[2].hybrid,std::string("O2H0"));
}

TEST("physics equivalence: N2H0 == N3H0, and both differ from N3H1"){
    CHECK(table().phys_equal("N2H0","N3H0"));
    CHECK(!table().phys_equal("N2H0","N3H1"));
}

TEST("novel element (Br) synthesizes physics not in somo.hybrid.new"){
    // C-Br: bromine has no entry in the 50-type vocab -> perceiver must synthesize its physics.
    auto o=run({A("C",0,0,0),A("Br",1.94,0,0)});
    const OutAtom& br=o[1];
    CHECK(!br.in_table);                    // synthesized, not a table lookup
    CHECK_EQ(br.num_elect,35);              // Z(Br)=35, neutral
    CHECK(br.mw>79.0 && br.mw<80.5);        // ~79.9
    CHECK(br.vdw_radius>1.8 && br.vdw_radius<1.9); // Bondi fallback ~1.85
}
TEST("emit surfaces a new somo.hybrid line for a novel type"){
    std::vector<InAtom> v={A("C",0,0,0),A("Br",1.94,0,0)};
    auto out=run(v);
    auto em=Perceiver(table()).emit_residue("LIG",v,out);
    bool has_br=false; for(auto&h:em.new_hybrids) if(h.find("Br")!=std::string::npos||h.find("BR")!=std::string::npos) has_br=true;
    CHECK(has_br);                          // a synthesized somo.hybrid line is emitted
    CHECK(!em.residue_block.empty());
}

TEST("finalize physics pulls radius/electrons from table"){
    auto o=run({A("C",0,0,0),A("C",1.52,0,0)});
    CHECK(o[0].in_table);
    CHECK(o[0].vdw_radius>1.8 && o[0].vdw_radius<1.95); // C4H3 radius 1.88
    CHECK_EQ(o[0].num_elect,9);                          // C(6)+3H
}

int main(){ return tinytest::run(); }
