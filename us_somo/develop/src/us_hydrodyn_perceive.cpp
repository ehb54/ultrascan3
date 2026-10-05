// somo_perceiver.cpp -- see header. v1: geometry-based hybridization + valence H-count.
#include "us_hydrodyn_perceive.h"
#include "us_hydrodyn_perceive_elements.h"
#include <cmath>
#include <array>
#include <unordered_map>
#include <map>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <functional>
#include <set>
#include <cstdio>

namespace somo_perceive {

// ---------- geometry helpers ----------
static inline double d2(const InAtom& a, const InAtom& b) {
    double dx=a.x-b.x, dy=a.y-b.y, dz=a.z-b.z; return dx*dx+dy*dy+dz*dz;
}
static inline double dist(const InAtom& a, const InAtom& b) { return std::sqrt(d2(a,b)); }

static double angle_deg(const InAtom& c, const InAtom& p, const InAtom& q) {
    double ax=p.x-c.x, ay=p.y-c.y, az=p.z-c.z;
    double bx=q.x-c.x, by=q.y-c.y, bz=q.z-c.z;
    double na=std::sqrt(ax*ax+ay*ay+az*az), nb=std::sqrt(bx*bx+by*by+bz*bz);
    if (na<1e-9||nb<1e-9) return 0;
    double cosv=(ax*bx+ay*by+az*bz)/(na*nb);
    cosv=std::max(-1.0,std::min(1.0,cosv));
    return std::acos(cosv)*180.0/M_PI;
}
// distance of center from the plane through its 3 neighbors
static double out_of_plane(const InAtom& c, const InAtom& n1, const InAtom& n2, const InAtom& n3) {
    double ux=n2.x-n1.x, uy=n2.y-n1.y, uz=n2.z-n1.z;
    double vx=n3.x-n1.x, vy=n3.y-n1.y, vz=n3.z-n1.z;
    double nx=uy*vz-uz*vy, ny=uz*vx-ux*vz, nz=ux*vy-uy*vx;
    double nn=std::sqrt(nx*nx+ny*ny+nz*nz);
    if (nn<1e-9) return 0;
    double wx=c.x-n1.x, wy=c.y-n1.y, wz=c.z-n1.z;
    return std::fabs((wx*nx+wy*ny+wz*nz)/nn);
}

// ---------- bond perception (uniform spatial grid) ----------
Bonds Perceiver::perceive_bonds(const std::vector<InAtom>& a,
                                const std::vector<std::pair<int,int>>& explicit_bonds) const {
    Bonds b; b.nb.assign(a.size(), {});
    // Authoritative connectivity first (PDB CONECT). Atoms named here are exempt from the
    // metal-isolation rule: for a metal-oxo cluster the metal-ligand bonds are the real skeleton.
    std::vector<char> has_explicit(a.size(), 0);
    for (auto& e : explicit_bonds) {
        int i=e.first, j=e.second;
        if (i<0||j<0||i>=(int)a.size()||j>=(int)a.size()||i==j) continue;
        b.nb[i].push_back(j); b.nb[j].push_back(i);
        has_explicit[i]=has_explicit[j]=1;
    }
    const double cell = 2.6; // > max plausible covalent bond
    std::unordered_map<long long, std::vector<int>> grid;
    auto key=[&](int cx,int cy,int cz)->long long{
        return ( (long long)(cx+1024) ) | ((long long)(cy+1024)<<21) | ((long long)(cz+1024)<<42);
    };
    auto cellof=[&](double v){ return (int)std::floor(v/cell); };
    std::vector<bool> skip(a.size(), false);
    for (size_t i=0;i<a.size();++i) {
        // Exclude H from bond perception so heavy-neighbor counts (and thus inferred implicit-H)
        // are identical whether or not the structure carries explicit hydrogens. Also exclude
        // monatomic ions so metal-ligand contacts don't inflate coordinating-atom neighbor counts.
        if (a[i].element=="H" || a[i].element=="D") { skip[i]=true; continue; }
        if (p_.metals_isolated && !has_explicit[i] &&
            monatomic_ion_hybrid().count(a[i].element)) { skip[i]=true; continue; }
        grid[key(cellof(a[i].x),cellof(a[i].y),cellof(a[i].z))].push_back((int)i);
    }
    for (size_t i=0;i<a.size();++i) {
        if (skip[i]) continue;
        int cx=cellof(a[i].x), cy=cellof(a[i].y), cz=cellof(a[i].z);
        double ri=covalent_radius(a[i].element);
        for (int dx=-1;dx<=1;++dx) for (int dy=-1;dy<=1;++dy) for (int dz=-1;dz<=1;++dz) {
            auto it=grid.find(key(cx+dx,cy+dy,cz+dz));
            if (it==grid.end()) continue;
            for (int j : it->second) {
                if (j<=(int)i) continue;
                double rj=covalent_radius(a[j].element);
                double dd=dist(a[i],a[j]);
                double cut=ri+rj+p_.bond_tol;
                if (dd>p_.bond_min && dd<cut) { b.nb[i].push_back(j); b.nb[j].push_back((int)i); }
            }
        }
    }
    // de-duplicate (an explicit bond may also be found by distance)
    for (auto& nb : b.nb) {
        std::sort(nb.begin(), nb.end());
        nb.erase(std::unique(nb.begin(), nb.end()), nb.end());
    }
    return b;
}

// ---------- ring perception + aromaticity (planarity) ----------
// Find simple rings of size 5 and 6 via bounded DFS anchored at each ring's minimum-index atom.
static void find_rings(const Bonds& b, std::vector<std::vector<int>>& rings) {
    const int MAXR = 6;
    int n = (int)b.nb.size();
    std::vector<int> path;
    std::vector<char> onpath(n, 0);
    std::function<void(int,int)> dfs = [&](int start, int cur) {
        for (int nx : b.nb[cur]) {
            if (nx == start && (int)path.size() >= 5) {           // closed 5- or 6-ring
                if (path[1] < path.back()) rings.push_back(path);  // canonical direction (dedup)
                continue;
            }
            if (nx <= start) continue;                             // anchor at min index
            if (onpath[nx]) continue;
            if ((int)path.size() >= MAXR) continue;
            path.push_back(nx); onpath[nx] = 1;
            dfs(start, nx);
            onpath[nx] = 0; path.pop_back();
        }
    };
    for (int s = 0; s < n; ++s) {
        if (b.nb[s].size() < 2) continue;
        path.clear(); path.push_back(s); onpath[s] = 1;
        dfs(s, s);
        onpath[s] = 0;
    }
}

// ---------- SSSR ----------
// Smallest set of smallest rings, from adjacency only. Distinct from find_rings() above, which
// enumerates every simple 5- or 6-cycle and feeds aromaticity: that is the right input for "is
// this atom in a flat conjugated ring", but the wrong one for anything charged once per ring,
// because a fused bicyclic contributes 3 simple cycles while its circuit rank is 2.
//
// Method: for each ring bond take the smallest cycle through it (BFS for the shortest path
// between its ends with that bond removed), then greedily accept candidates smallest-first,
// keeping one only if it covers a bond no accepted ring covers yet, until the circuit rank
// |E| - |V| + |components| is reached. Standard for molecule-sized graphs.
void find_sssr(Bonds& b, int max_ring) {
    const int n = (int) b.nb.size();
    b.rings.clear();
    if (n == 0) return;

    // edges, and circuit rank = |E| - |V| + |components|
    std::vector<std::pair<int,int>> edges;
    for (int u = 0; u < n; ++u)
        for (int v : b.nb[u])
            if (u < v) edges.push_back({u, v});

    std::vector<char> seen(n, 0);
    int comps = 0;
    for (int s = 0; s < n; ++s) {
        if (seen[s]) continue;
        ++comps;
        std::vector<int> st{s};
        seen[s] = 1;
        while (!st.empty()) {
            int c = st.back(); st.pop_back();
            for (int x : b.nb[c]) if (!seen[x]) { seen[x] = 1; st.push_back(x); }
        }
    }
    const int rank = (int) edges.size() - n + comps;
    if (rank <= 0) return;

    // smallest cycle through each edge
    struct Cand { std::vector<int> ring; std::vector<std::pair<int,int>> bonds; };
    std::vector<Cand> cands;
    for (auto& e : edges) {
        const int u = e.first, v = e.second;
        // BFS v -> u without using edge (u,v)
        std::vector<int> prev(n, -1), depth(n, -1);
        std::vector<int> q{v};
        depth[v] = 0;
        for (size_t qi = 0; qi < q.size(); ++qi) {
            const int c = q[qi];
            if (depth[c] >= max_ring - 1) continue;
            for (int x : b.nb[c]) {
                if ((c == u && x == v) || (c == v && x == u)) continue;   // the removed bond
                if (depth[x] != -1) continue;
                depth[x] = depth[c] + 1;
                prev[x] = c;
                q.push_back(x);
            }
        }
        if (depth[u] < 0) continue;                       // bond is not in any ring <= max_ring
        Cand cd;
        for (int c = u; c != -1; c = prev[c]) cd.ring.push_back(c);
        if ((int) cd.ring.size() > max_ring) continue;
        for (size_t k = 0; k < cd.ring.size(); ++k) {
            const int p = cd.ring[k], q2 = cd.ring[(k + 1) % cd.ring.size()];
            cd.bonds.push_back({std::min(p, q2), std::max(p, q2)});
        }
        cands.push_back(std::move(cd));
    }

    std::stable_sort(cands.begin(), cands.end(),
                     [](const Cand& a, const Cand& c) { return a.ring.size() < c.ring.size(); });

    std::set<std::vector<int>> taken;                     // dedupe by sorted atom set
    std::set<std::pair<int,int>> covered;
    for (const Cand& cd : cands) {
        if ((int) b.rings.size() >= rank) break;
        std::vector<int> key = cd.ring;
        std::sort(key.begin(), key.end());
        if (taken.count(key)) continue;
        bool adds = false;
        for (auto& bd : cd.bonds) if (!covered.count(bd)) { adds = true; break; }
        if (!adds) continue;
        taken.insert(key);
        for (auto& bd : cd.bonds) covered.insert(bd);
        b.rings.push_back(cd.ring);
    }
}

// Max out-of-plane deviation of an ordered ring from its mean plane (Newell normal).
static double ring_planarity(const std::vector<int>& ring, const std::vector<InAtom>& a) {
    double cx=0,cy=0,cz=0; int m=(int)ring.size();
    for (int i : ring){ cx+=a[i].x; cy+=a[i].y; cz+=a[i].z; }
    cx/=m; cy/=m; cz/=m;
    double nx=0,ny=0,nz=0;
    for (int k=0;k<m;++k){ const InAtom&p=a[ring[k]]; const InAtom&q=a[ring[(k+1)%m]];
        nx+=(p.y-q.y)*(p.z+q.z); ny+=(p.z-q.z)*(p.x+q.x); nz+=(p.x-q.x)*(p.y+q.y); }
    double nn=std::sqrt(nx*nx+ny*ny+nz*nz); if (nn<1e-9) return 1e9;
    nx/=nn; ny/=nn; nz/=nn;
    double maxd=0;
    for (int i : ring){ double dv=std::fabs((a[i].x-cx)*nx+(a[i].y-cy)*ny+(a[i].z-cz)*nz);
        maxd=std::max(maxd,dv); }
    return maxd;
}

static double ring_avg_bond(const std::vector<int>& ring, const std::vector<InAtom>& a) {
    int m=(int)ring.size(); double s=0;
    for (int k=0;k<m;++k) s+=dist(a[ring[k]],a[ring[(k+1)%m]]);
    return s/m;
}

static void mark_aromatic(const std::vector<InAtom>& a, Bonds& b) {
    b.aromatic.assign(a.size(), 0);
    std::vector<std::vector<int>> rings;
    find_rings(b, rings);
    for (auto& r : rings) {
        if (r.size()!=5 && r.size()!=6) continue;
        // Aromatic: flat AND conjugated. Planarity (<=0.01 A) separates aromatic from puckered
        // furanose (~0.24-0.29 A); the mean-ring-bond guard (<1.43 A: aromatic ~1.34-1.40 vs
        // saturated proline ~1.50) rejects the occasional near-planar all-sp3 proline ring.
        if (ring_planarity(r, a) < 0.10 && ring_avg_bond(r, a) < 1.43)
            for (int i : r) b.aromatic[i] = 1;
    }
}

// ---------- Kekule perception ----------
// Assign ring double bonds by backtracking so every aromatic carbon (that isn't already using its
// pi in an exocyclic C=O) gets exactly one ring double bond. A 2-coordinate aromatic N that ends
// up with a ring double bond is pyridine-type (no H); one left single-bonded is pyrrole-type (H).
static void kekulize(const std::vector<InAtom>& a, Bonds& b) {
    int n=(int)a.size();
    b.ring_double.assign(n,0);
    const auto& arom=b.aromatic;
    std::vector<char> blocked(n,0);   // cannot take a ring double bond (exocyclic C=O, or subst. N)
    for (int i=0;i<n;++i) if (arom[i]) {
        if (a[i].element=="C") {
            for (int j : b.nb[i])   // exocyclic carbonyl/thiocarbonyl => pi used outside the ring
                if ((a[j].element=="O"||a[j].element=="S") && (int)b.nb[j].size()==1 && dist(a[i],a[j])<1.32)
                    blocked[i]=1;
        } else if (a[i].element=="N" && (int)b.nb[i].size()>=3) {
            blocked[i]=1;           // 3-connected aromatic N is pyrrole-type (lone pair in ring)
        }
    }
    std::vector<char> vis(n,0);
    for (int s=0;s<n;++s) {
        if (!arom[s]||vis[s]) continue;
        std::vector<int> comp, stack{s}; vis[s]=1;
        while(!stack.empty()){ int u=stack.back(); stack.pop_back(); comp.push_back(u);
            for (int v : b.nb[u]) if (arom[v]&&!vis[v]){ vis[v]=1; stack.push_back(v); } }
        std::vector<int> must;                 // aromatic C needing exactly one ring double bond
        for (int u : comp) if (a[u].element=="C" && !blocked[u]) must.push_back(u);
        std::vector<char> matched(n,0);
        for (int u : comp) if (blocked[u]) matched[u]=1;
        std::function<bool(size_t)> solve=[&](size_t idx)->bool{
            if (idx==must.size()) return true;
            int u=must[idx];
            if (matched[u]) return solve(idx+1);
            for (int v : b.nb[u]) {
                if (!arom[v]||matched[v]) continue;
                matched[u]=matched[v]=1; b.ring_double[u]=b.ring_double[v]=1;
                if (solve(idx+1)) return true;
                matched[u]=matched[v]=0; b.ring_double[u]=b.ring_double[v]=0;
            }
            return false;
        };
        if (!solve(0)) for (int u : comp) b.ring_double[u]=0;  // no valid Kekule => fall back
    }
}

// A clear (non-delocalized) double bond by length -- tight cutoffs so that merely-short single
// bonds in low-resolution structures do NOT trigger. Used only for non-ring, non-planar carbons.
static bool c_bond_is_double(const std::string& other, double L) {
    if (other=="C") return L < 1.38;   // C=C ~1.33; distorted sp3 single ~1.44 stays single
    if (other=="N") return L < 1.34;   // C=N ~1.30
    if (other=="O") return L < 1.28;   // C=O ~1.23
    if (other=="S") return L < 1.62;   // C=S ~1.60
    return false;
}

// hybridization of a CARBON: returns 4 (sp3),3 (sp2),2 (sp).
// Primary sp2 signal is aromatic-ring membership (planar-ring test, robust to bad bond lengths)
// and, for non-ring carbons, planarity of a 3-neighbor set or a clear localized double bond.
static int geom_hyb(int i, const std::vector<InAtom>& a, const Bonds& b, const Params& p) {
    const auto& nb=b.nb[i];
    int h=(int)nb.size();
    if (h>=4) return 4;
    if (!b.aromatic.empty() && b.aromatic[i]) return 3;   // flat 5/6-ring carbon => sp2
    double minlen=1e9; bool dbl=false;
    for (int j : nb) { double L=dist(a[i],a[j]); minlen=std::min(minlen,L);
        if (c_bond_is_double(a[j].element,L)) dbl=true; }
    if (h==3) {
        double oop=out_of_plane(a[i],a[nb[0]],a[nb[1]],a[nb[2]]);
        return oop < p.planar_thresh ? 3 : 4;   // planar (carbonyl/carboxyl/guanidinium) => sp2
    }
    if (h==2) {
        double ang=angle_deg(a[i],a[nb[0]],a[nb[1]]);
        if (ang>p.sp_angle || minlen<1.25) return 2;  // linear / very short => sp
        return dbl ? 3 : 4;
    }
    if (h==1) {
        // sp only for a genuine triple/allene bond (C#C ~1.20). Anything shorter than 1.15 A is not
        // a real bond -- corrupt coordinates -- so fall back to sp3 rather than invent an sp type.
        if (minlen>=1.15 && minlen<1.22) return 2;
        return dbl ? 3 : 4;                       // =CH2 vinyl vs methyl
    }
    return 4;
}

// is carbon 'ci' a carbonyl/thiocarbonyl-type C (has an O/S neighbor at ~double-bond distance)?
static bool has_double_O(int ci, const std::vector<InAtom>& a, const Bonds& b) {
    for (int j : b.nb[ci]) {
        if (a[j].element=="O" && (int)b.nb[j].size()==1 && dist(a[ci],a[j])<1.30) return true;
    }
    return false;
}

// ---------- per-element classifiers ----------
void Perceiver::classify_C(int i, const std::vector<InAtom>& a, const Bonds& b, OutAtom& o) const {
    int h=(int)b.nb[i].size();
    int d=geom_hyb(i,a,b,p_);
    int nH=std::max(0, d-h);
    o.n_h=nH;
    o.hybrid = "C" + std::to_string(d) + "H" + std::to_string(nH);
    if (h==0) { o.hybrid="C4H3"; o.n_h=3; o.ambiguity=1; o.note="isolated C"; }
}

void Perceiver::classify_N(int i, const std::vector<InAtom>& a, const Bonds& b, OutAtom& o) const {
    int h=(int)b.nb[i].size();
    if (h>=4) { o.hybrid="N4H0"; o.n_h=0; return; }
    if (h==3) { o.hybrid="N3H0"; o.n_h=0; return; }
    if (h==2) {
        // 2-coordinate N: pyridine-type (no H) vs amine/amide/pyrrole (1 H).
        if (!b.aromatic.empty() && b.aromatic[i]) {
            // aromatic ring N: Kekule decides. Ring double bond => pyridine-type (N2H0, no H);
            // single-bonded (lone pair in ring) => pyrrole-type (N3H1, has H).
            if (!b.ring_double.empty() && b.ring_double[i]) { o.hybrid="N2H0"; o.n_h=0; }
            else                                            { o.hybrid="N3H1"; o.n_h=1; }
            return;
        }
        // non-aromatic 2-coordinate N: backbone/other amide (has H) unless a clear localized imine.
        bool amide=false, imine=false, guanidino=false;
        for (int j : b.nb[i]) if (a[j].element=="C") {
            if (has_double_O(j,a,b)) amide=true;
            if (dist(a[i],a[j])<1.27) imine=true;   // clear localized N=C
            int nN=0; for (int k : b.nb[j]) if (a[k].element=="N") ++nN;
            if (nN>=2) guanidino=true;               // guanidinium/amidine NE keeps its H (N3H1)
        }
        if (imine && !amide && !guanidino) { o.hybrid="N2H0"; o.n_h=0; o.note="imine"; return; }
        o.hybrid="N3H1"; o.n_h=1;
        return;
    }
    if (h==1) {
        // terminal N: aliphatic amine (Lys) -> N4H3+ ; amide/guanidinium/aromatic-exo -> N3H2
        int j=b.nb[i][0];
        int chyb = (a[j].element=="C") ? geom_hyb(j,a,b,p_) : 3;
        if (chyb==4) { o.hybrid="N4H3+"; o.n_h=3; o.formal_charge=1; }
        else         { o.hybrid="N3H2";  o.n_h=2; }
        return;
    }
    o.hybrid="N3H3"; o.n_h=3; o.ambiguity=1; o.note="isolated N";
}

void Perceiver::classify_O(int i, const std::vector<InAtom>& a, const Bonds& b, OutAtom& o) const {
    int h=(int)b.nb[i].size();
    if (h>=2) {
        // bridging O (ester/ether/phosphodiester). O2H0B if a neighbor is P.
        bool has_P=false; for (int j : b.nb[i]) if (a[j].element=="P") has_P=true;
        o.hybrid = has_P ? "O2H0B" : "O2H0"; o.n_h=0; return;
    }
    if (h==1) {
        int j=b.nb[i][0];
        double bl=dist(a[i],a[j]);
        // phosphate/sulfate terminal O -> O1H0
        if (a[j].element=="P" || a[j].element=="S") { o.hybrid="O1H0"; o.n_h=0; return; }
        // Terminal O on a METAL (polyoxometalates, metal-oxo clusters, metalloprotein sites).
        // C-O length cutoffs are meaningless here; metal-oxygen bonds are much longer and split
        // cleanly bimodally: a short M=O is a terminal oxo (no H), a long dative M<-OH2 is an aqua
        // ligand. Typical for early-transition-metal oxo clusters: oxo ~1.65-1.90 A, aqua ~2.0-2.4 A.
        if (monatomic_ion_hybrid().count(a[j].element)) {
            if (bl < 1.95) { o.hybrid="O1H0"; o.n_h=0; o.note="terminal oxo (M=O)"; }
            else { o.hybrid="O2H2"; o.n_h=2; o.ambiguity=1;
                   o.note="aqua ligand on metal (assumed H2O; could be hydroxo O2H1)"; }
            return;
        }
        // carbonyl vs hydroxyl -- keyed on the neighbor carbon's hybridization (robust to structures
        // that draw C=O long), with carboxylate handled by symmetry below.
        if (a[j].element=="C") {
            // count terminal O neighbors of this carbon (carboxylate/carbonyl)
            std::vector<int> termO;
            for (int k : b.nb[j]) if (a[k].element=="O" && (int)b.nb[k].size()==1) termO.push_back(k);
            if (termO.size()<2) {
                if (!b.aromatic.empty() && b.aromatic[j]) {
                    // aromatic C: ring carbonyl (C=O ~1.23) vs phenol/enol -OH (C-O ~1.36)
                    o.hybrid = bl<1.31 ? "O1H0" : "O2H1"; o.n_h = bl<1.31?0:1; return;
                }
                if (geom_hyb(j,a,b,p_)==3) { o.hybrid="O1H0"; o.n_h=0; return; }  // sp2 carbonyl/amide
                o.hybrid="O2H1"; o.n_h=1; return;                                  // sp3 hydroxyl
            }
            if (termO.size()>=2) {
                // carboxyl/carboxylate. If the two C-O lengths are clearly unequal the molecule is
                // genuinely protonated -> the short one is the real C=O (O1H0), the long is C-OH.
                // If near-equal (deprotonated/resonant, geometrically symmetric) the =O choice is a
                // convention: assign the lowest input-index terminal O (PDB lists OD1 before OD2,
                // matching somo.residue.new's OD1=O1H0). Physically negligible either way.
                int shortest=termO[0], lowidx=termO[0]; double mn=1e9,mx=-1e9;
                for (int k : termO){ double dv=dist(a[j],a[k]);
                    if (dv<mn){mn=dv;shortest=k;} if (dv>mx)mx=dv; if (k<lowidx)lowidx=k; }
                bool asym = (mx-mn) > 0.08;
                int carbonylO = asym ? shortest : lowidx;
                if (i==carbonylO) { o.hybrid="O1H0"; o.n_h=0; }
                else { o.hybrid="O2H1"; o.n_h=1; o.ambiguity=1;
                       o.note = asym ? "carboxyl -OH" : "carboxylate O (symmetric; protonation convention)"; }
                return;
            }
        }
        o.hybrid = bl<1.30 ? "O1H0" : "O2H1"; o.n_h = bl<1.30?0:1; return;
    }
    o.hybrid="O2H2"; o.n_h=2; // isolated O = water
}

void Perceiver::classify_S(int i, const std::vector<InAtom>& a, const Bonds& b, OutAtom& o) const {
    int h=(int)b.nb[i].size();
    int nO=0; for (int j : b.nb[i]) if (a[j].element=="O") ++nO;
    if (nO>=3) { o.hybrid="S"; o.n_h=0; return; }        // sulfate/sulfonate S (buried, r 0.43)
    if (h>=2) { o.hybrid="S2H0"; o.n_h=0; return; }      // thioether / disulfide
    if (h==1) { o.hybrid="S2H1"; o.n_h=1; return; }      // free thiol
    o.hybrid="S2H0"; o.n_h=0;                            // isolated -> treat as sulfide-ish
}

void Perceiver::classify_P(int, const std::vector<InAtom>&, const Bonds&, OutAtom& o) const {
    o.hybrid="P4H0"; o.n_h=0;
}

OutAtom Perceiver::classify(int i, const std::vector<InAtom>& a, const Bonds& b) const {
    OutAtom o; o.element=a[i].element;
    o.heavy_neighbors=(int)b.nb[i].size();
    const std::string& e=a[i].element;
    auto ion=monatomic_ion_hybrid().find(e);
    if (ion!=monatomic_ion_hybrid().end()) { o.hybrid=ion->second; o.n_h=0;
        auto* hi=tbl_.get(o.hybrid); if(hi)o.formal_charge=hi->charge;
        // Oxidation state is NOT perceivable from coordinates. If the table offers several states
        // for this element (e.g. MO+3/MO+5/MO+6, FE+2/FE+3) the default is a guess that changes
        // radius and electron count -- flag it so the user is asked to confirm.
        std::vector<std::string> alts;
        for (auto& kv : tbl_.all()) {
            const std::string& sn=kv.second.saxs_name; std::string bare;
            for (char c : sn) { if (std::isalpha((unsigned char)c)) bare+=(char)std::toupper(c); else break; }
            if (bare==e && kv.first!=o.hybrid) alts.push_back(kv.first);
        }
        if (!alts.empty()) {
            o.ambiguity=1;
            o.note="oxidation state assumed "+o.hybrid+"; not perceivable from geometry (also in table:";
            for (auto& s : alts) o.note += " "+s;
            o.note += ")";
        }
        finalize_physics(o); return o; }
    if      (e=="C") classify_C(i,a,b,o);
    else if (e=="N") classify_N(i,a,b,o);
    else if (e=="O") classify_O(i,a,b,o);
    else if (e=="S") classify_S(i,a,b,o);
    else if (e=="P") classify_P(i,a,b,o);
    else if (e=="SE"){ o.hybrid="SE2H0"; o.n_h=0; }
    else if (e=="B") { o.hybrid="B4H0"; o.n_h=0; }
    else if (e=="H") { o.hybrid="XH"; o.n_h=0; }
    else { o.hybrid=e; o.ambiguity=1; o.note="unhandled element"; }

    // Vocabulary guard: for the well-covered organic elements the table is authoritative, so never
    // propose a type that does not exist in it (distorted/corrupt geometry can otherwise yield e.g.
    // "C2H1"). Fall back to the saturated form for the observed heavy-atom count and flag it, so a
    // user-facing "tentative entry" always contains real types. Genuinely novel ELEMENTS are still
    // allowed through to physics synthesis below.
    if (!tbl_.has(o.hybrid) && (e=="C"||e=="N"||e=="O"||e=="S"||e=="P")) {
        int h=o.heavy_neighbors, base = (e=="C")?4 : (e=="N")?3 : (e=="P")?4 : 2;
        int nH = std::max(0, base-h);
        std::string fb = e + std::to_string(base) + "H" + std::to_string(nH);
        if (tbl_.has(fb)) {
            o.note = "implausible geometry; '"+o.hybrid+"' not in somo.hybrid.new, fell back to "+fb;
            o.hybrid = fb; o.n_h = nH; o.ambiguity = 1;
        }
    }
    finalize_physics(o);
    return o;
}

void Perceiver::finalize_physics(OutAtom& o) const {
    if (auto* hi=tbl_.get(o.hybrid)) {
        o.in_table=true; o.mw=hi->mw; o.vdw_radius=hi->radius; o.num_elect=hi->num_elect;
        if (o.formal_charge==0) o.formal_charge=hi->charge;
        return;
    }
    // synthesize a novel type: physics from element + nH + charge
    o.in_table=false;
    int Z=atomic_number(o.element);
    o.mw = atomic_mass(o.element) + o.n_h*1.008;
    o.num_elect = Z + o.n_h - o.formal_charge;
    o.vdw_radius = vdw_radius(o.element);
    if (o.note.empty()) o.note="synthesized (not in somo.hybrid.new)";
}

Perceiver::Emitted Perceiver::emit_residue(const std::string& resname,
        const std::vector<InAtom>& atoms, const std::vector<OutAtom>& perceived,
        const std::string& chemical_name, const Properties* props) const {
    Emitted em;
    Properties defaults;
    const Properties& pr = props ? *props : defaults;
    double total_mw=0;
    // somo.residue can express at most TWO ionizable atoms per residue: the header is a vbar
    // followed by one (vbar, pKa) pair per ionization, the loader rejects a third outright
    // ("currently only 2 are supported"), and each atom's ionization index must be unique AND
    // match a declared pKa, which it then consumes (us_hydrodyn_load.cpp).
    //
    // A species with more ionizable groups than that -- citrate has three carboxyls -- follows
    // SOMO's existing PHOSPHATE CONVENTION (ehb54/Mattia, 2026-08-12: "our workaround for
    // phosphate, which is present, was to assume the first oxygen to be always deprotonated").
    // The lowest-pKa groups are taken as PERMANENTLY deprotonated and written straight into the
    // primary fields; the two HIGHEST pKas keep the real, pH-switchable slots.
    //
    // Lowest-first is what makes this defensible rather than arbitrary: the lowest pKa is the one
    // that deprotonates first and is therefore already fully ionized everywhere the entry is
    // usable. Phosphate (2.15 / 7.20 / 12.35) reproduces the hand-written convention exactly --
    // the first oxygen fixed, 7.20 and 12.35 switchable. Citrate (3.13 / 4.76 / 6.40) keeps its
    // 2- / 3- transition instead of being frozen at pH 7, which is what collapsing used to do.
    static const size_t MAX_IONIZATIONS = 2;    // hard limit of the file format, not a policy
    size_t n_ionizable = 0;
    for (const Properties::Alt& a : pr.alternate) if (!a.hybrid.empty()) ++n_ionizable;
    const size_t n_fixed = n_ionizable > MAX_IONIZATIONS ? n_ionizable - MAX_IONIZATIONS : 0;

    // Per atom, not per residue: an entry may now hold both fixed and switchable groups at once.
    std::vector<char> fixed_deprot(atoms.size(), 0);
    if (n_fixed) {
        std::vector<size_t> ion_idx;
        for (size_t k = 0; k < atoms.size() && k < pr.alternate.size(); ++k)
            if (!pr.alternate[k].hybrid.empty()) ion_idx.push_back(k);
        // stable, so chemically equivalent groups (identical pKa) are fixed in atom order and the
        // same input always yields the same entry.
        std::stable_sort(ion_idx.begin(), ion_idx.end(),
                         [&](size_t x, size_t y){ return pr.alternate[x].pKa < pr.alternate[y].pKa; });
        for (size_t j = 0; j < n_fixed; ++j) fixed_deprot[ion_idx[j]] = 1;
    }

    // A switchable slot needs BOTH the atom's 16-field form and a matching header pKa -- the
    // loader builds {1..n_pairs} and rejects the record if an atom's index has no pair to consume.
    // One predicate drives both emissions so they cannot drift apart, which is also why a pKa of
    // zero disqualifies: an alternate with no pKa could otherwise take an index nothing declares.
    auto switchable = [&](size_t k) {
        return k < pr.alternate.size() && !pr.alternate[k].hybrid.empty()
               && !fixed_deprot[k] && pr.alternate[k].pKa > 0;
    };

    std::vector<std::string> pr_review_extra;
    // A fixed group still has to reach the review dialog -- its columns must not go blank just
    // because the file cannot hold it as an alternate (Mattia, 2026-08-10: "now the ionizable
    // fields are empty ... I would left this in at this stage"). A machine-readable comment
    // carries it: the loader ignores "#" lines and the dialog keeps the whole comment block, so
    // this survives without new plumbing. The trailing FIXED distinguishes it from a switchable
    // group, which the dialog needs in order to show it read-only.
    std::ostringstream ion_note;
    if (n_fixed)
        for (size_t k = 0; k < atoms.size(); ++k) {
            if (!fixed_deprot[k]) continue;
            const Properties::Alt& a = pr.alternate[k];
            ion_note << "# ION\t" << atoms[k].name << '\t' << a.hybrid << '\t'
                     << a.waters << '\t' << a.pKa << "\tFIXED\n";
        }
    if (n_fixed) {
        char buf[256];
        std::snprintf(buf, sizeof(buf),
                      "%zu ionizable atoms, but somo.residue supports at most %zu per residue. "
                      "The %zu lowest-pKa group(s) are written permanently deprotonated "
                      "(phosphate convention); the %zu highest pKas stay pH-switchable",
                      n_ionizable, MAX_IONIZATIONS, n_fixed, MAX_IONIZATIONS);
        pr_review_extra.push_back(buf);
    }

    std::ostringstream body;
    std::map<std::string,int> seen_new;
    double bead_hydration = 0;   // accumulated per atom below, so it cannot drift from the atom lines
    int ion_index = 0;   // 1-based, one per ionizable atom, must match a header pKa
    for (size_t k=0;k<atoms.size();++k) {
        const OutAtom& o=perceived[k];
        total_mw += fixed_deprot[k] ? pr.alternate[k].mw : o.mw;
        // atom line: name hybrid mw radius bead 0 index hydration
        const bool has_alt = k < pr.alternate.size() && !pr.alternate[k].hybrid.empty();
        const Properties::Alt* a = has_alt ? &pr.alternate[k] : nullptr;
        double hyd = k < pr.hydration.size() ? pr.hydration[k] : 0.0;
        // For a group held fixed the deprotonated species IS the atom: write its type, mass,
        // radius and waters in the primary fields rather than the protonated ones.
        const std::string pr_hyb = fixed_deprot[k] ? a->hybrid : o.hybrid;
        const double pr_mw  = fixed_deprot[k] ? a->mw     : o.mw;
        const double pr_rad = fixed_deprot[k] ? a->radius : o.vdw_radius;
        if (fixed_deprot[k]) hyd = a->waters;
        // The bead's hydration counts an ionizable atom in its IONIZED state, which is what the
        // coded residues do: ASP's OD2 carries 0 waters protonated and 5 ionized, and ASP's second
        // bead is written 5, not 0. Summing the protonated column instead made 2CMD's citrate bead
        // read 1 water against the 16 the review dialog computed for the same entry (Mattia,
        // 2026-08-10) -- the headless and GUI paths disagreed on the same residue.
        bead_hydration += a ? a->waters : hyd;
        body << atoms[k].name << '\t' << pr_hyb << '\t' << pr_mw << '\t' << pr_rad
             << "\t0\t0\t" << k << '\t' << hyd;
        // 16-field form for a switchable atom: the protonated species above, the deprotonated one
        // here, exactly as somo.residue writes ASP's OD2 / LYS's NZ / ARG's NH2. The loader accepts
        // 8 or 16 fields and nothing between, so this is all or nothing. The ionization index must
        // be unique within the residue and match a declared pKa, so it counts up from 1.
        if (switchable(k)) {
            body << '\t' << ++ion_index << '\t' << a->hybrid << '\t' << a->mw << '\t' << a->radius
                 << "\t0\t0\t" << k << '\t' << a->waters;
        }
        body << '\n';
        if (!o.in_table && !seen_new.count(o.hybrid)) {
            seen_new[o.hybrid]=1;
            std::ostringstream hl;
            std::string saxs=o.element; if(o.formal_charge) hl.str(""),
                saxs=o.element+(o.formal_charge>0?"+":"")+std::to_string(o.formal_charge);
            hl << saxs << '\t' << o.hybrid << '\t' << o.mw << '\t' << o.vdw_radius
               << "\t0\t" << o.n_h << '\t' << o.num_elect << "\t# synthesized by perceiver";
            em.new_hybrids.push_back(hl.str());
        }
    }
    // Flag genuinely uncertain atoms so a user confirming this entry knows what to check.
    // Only ambiguous or novel types qualify: a confident classification that merely carries an
    // informational note (e.g. "terminal oxo") must not be listed as uncertain, and this must
    // agree with the flagged count reported by callers.
    // Two independent streams land in this list -- perception ambiguity notes here, and the
    // psv/volume/hydration reviews collected by the caller -- and both are keyed "NAME (HYB)".
    // Emitted raw, an atom carrying one of each appears twice, which reads as more flagged atoms
    // than there are (Mattia, 2026-08-10, on citrate: the review list "incorrectly tentatively
    // assigns far more waters"). Group by atom instead, first-appearance order, notes joined.
    std::vector<std::pair<std::string,std::string>> items;   // (key, text); key empty = not per-atom
    for (const std::string& r : pr_review_extra) items.push_back({ std::string(), r });
    for (size_t k=0;k<atoms.size();++k) {
        const OutAtom& o=perceived[k];
        if (!o.ambiguity && o.in_table) continue;
        items.push_back({ atoms[k].name + " (" + o.hybrid + ")",
                          o.note.empty() ? (o.in_table ? std::string("uncertain")
                                                       : std::string("NEW type - synthesized coefficients"))
                                         : o.note });
    }
    for (const std::string& r : pr.review) {
        const size_t sep = r.find("): ");
        if (sep == std::string::npos || r.find('(') == std::string::npos)
            items.push_back({ std::string(), r });
        else
            items.push_back({ r.substr(0, sep + 1), r.substr(sep + 3) });
    }
    std::ostringstream review;
    std::vector<std::string> order;                      // keys, first-appearance order
    std::map<std::string,std::string> merged;
    for (auto& it : items) {
        if (it.first.empty()) { review << "#   " << it.second << '\n'; continue; }
        auto f = merged.find(it.first);
        if (f == merged.end()) { order.push_back(it.first); merged[it.first] = it.second; }
        else if (f->second.find(it.second) == std::string::npos) f->second += "; " + it.second;
    }
    for (const std::string& k : order) review << "#   " << k << ": " << merged[k] << '\n';
    std::ostringstream hdr;
    // Comment line = the residue's real chemical name when the PDB header supplied one (HETNAM).
    hdr << "# " << (chemical_name.empty() ? resname : chemical_name)
        << " [" << resname << "]  (generated on-the-fly by perceiver)\n";
    {
        // Say plainly which fields are computed and which are still unset. An entry that is only
        // partly filled must look partly filled.
        std::ostringstream prov;
        prov << "# Perceived from geometry: atom names, hybrid types, masses, vdW radii.\n";
        prov.setf(std::ios::fixed);
        if (pr.have_vbar)
            prov << "# psv (vbar)  : " << std::setprecision(3) << pr.vbar << " cm^3/g, computed from Durchschlag & Zipper\n"
                    "#   atomic volume increments. Typical accuracy +-2-3% for ordinary neutral\n"
                    "#   organic chemistry; poorer for lipids, ionic detergents and charged\n"
                    "#   groups, and not applicable to metal or Fe-S clusters.\n";
        else
            prov << "# psv (vbar)  : NOT SET (0). Must be supplied before this entry is used --\n"
                    "#   vbar enters hydrodynamics through the buoyancy term (1 - vbar*rho).\n";
        if (pr.have_molvol)
            prov << "# molvol      : " << std::setprecision(2) << pr.molvol << " A^3, solvent-excluded volume at a 1.4 A\n"
                    "#   probe, scaled onto the convention the coded residues use.\n";
        else
            prov << "# molvol      : NOT SET (0). This is also the bead volume.\n";
        if (pr.hydration.empty())
            prov << "# hydration   : NOT SET (0) for every atom.\n";
        else
            prov << "# hydration   : proposed from the hybrid types of the coded residues; it is a\n"
                    "#   STARTING POINT to be edited, not a measurement. The residue total is the\n"
                    "#   quantity with literature backing (Kuntz), the per-atom split is convention.\n";
        prov << "# bead assignment and ASA : stubbed (single bead, all atoms).\n";
        hdr << prov.str();
        hdr << ion_note.str();
    }
    hdr << "# Residue mass from perceived atoms: " << total_mw << " Da (its mass fraction of the\n"
           "#   model tells you how much an imprecise psv here actually matters).\n";
    const double asa_out = pr.asa > 0 ? pr.asa : DEFAULT_ASA_PER_ATOM * atoms.size();
    if (pr.asa <= 0)
        review << "#   ASA: placeholder (" << DEFAULT_ASA_PER_ATOM
               << " A^2 per atom, the coded-residue mean). Nothing computes with the residue's\n"
                  "#     tabulated ASA, but it may not be zero -- the loaders drop such a record.\n";
    if (!review.str().empty())
        hdr << "# REVIEW - check these before accepting the entry:\n" << review.str();
    // Header fields, in the order US_Hydrodyn::read_residue_file reads them
    // (us_hydrodyn_load.cpp:151): name, type, molvol, ASA, natoms, nbeads, vbar.
    // ASA must be > 0 or every loader drops the record without a word -- see the policy note at
    // DEFAULT_ASA_PER_ATOM. It is a placeholder, so it is called one in the REVIEW block.
    {
        std::ostringstream h;
        h.setf(std::ios::fixed);
        h << resname << "\t0\t" << std::setprecision(2) << (pr.have_molvol ? pr.molvol : 0.0)
          << '\t' << std::setprecision(2) << asa_out
          << '\t' << atoms.size() << "\t1\t"
          << std::setprecision(3) << (pr.have_vbar ? pr.vbar : 0.0);
        // An entry with an ionizable atom needs the two extra header fields the coded ionizable
        // residues carry: vbar_ionized then pKa (ASP "0.603 0.603 3.67"). The table gives both
        // states the SAME vbar for every one of them -- Asp 0.603/0.603, Glu 0.663/0.663, Arg
        // 0.698/0.698 -- so the psv stays pH-independent here too, deliberately.
        // One (vbar, pKa) pair per SWITCHABLE atom, walked in atom order -- the same order the atom
        // lines number their ionization indices, so index n always lands on pair n. Groups held
        // fixed by the phosphate convention contribute no pair: they are already deprotonated in
        // the primary fields and have no state to switch to.
        for (size_t k = 0; k < atoms.size(); ++k)
            if (switchable(k))
                h << '\t' << std::setprecision(3) << (pr.have_vbar ? pr.vbar : 0.0)
                  << '\t' << std::setprecision(2) << pr.alternate[k].pKa;
        h << '\n';
        hdr << h.str();
    }
    // One default bead, all atoms assigned to it (Mattia: single bead for now).
    // Bead line fields are, in order (see US_Hydrodyn::read_residue_file,
    // us_hydrodyn_load.cpp:509): hydration, colour, placing_method, chain, volume.
    // NB this previously emitted atoms.size() in the COLOUR slot, which silently produced a
    // reserved colour for any residue with 6/7/8 atoms -- 0 and 6 mean "exclude this bead from
    // the hydrodynamic computation" -- and an out-of-range value above 15 atoms.
    std::ostringstream bead;
    bead.setf(std::ios::fixed);
    bead << std::setprecision(2) << bead_hydration << '\t'  // bead hydration = sum over its atoms
         << pr.bead_color << '\t'       // colour: default is DEFAULT_BEAD_COLOR in the header
         << "0\t"                       // placing_method 0 = centre of gravity (the only one used)
         << "0\t"                       // chain 0 = main chain
         << (pr.have_molvol ? pr.molvol : 0.0) << '\n';   // single bead: volume == molvol
    em.residue_block = hdr.str() + body.str() + bead.str();
    return em;
}

std::vector<OutAtom> Perceiver::perceive(const std::vector<InAtom>& atoms,
        Bonds& bonds_out,
        const std::vector<std::pair<int,int>>& explicit_bonds) const {
    // Normalize element symbols once (uppercase, alpha-only) so the classifier is robust to callers
    // that pass "Br"/"Fe"/" c" etc. All downstream element comparisons assume uppercase symbols.
    std::vector<InAtom> a=atoms;
    for (auto& x : a) x.element = norm_element(x.element);
    bonds_out = perceive_bonds(a, explicit_bonds);
    mark_aromatic(a, bonds_out);
    kekulize(a, bonds_out);
    find_sssr(bonds_out);
    std::vector<OutAtom> out; out.reserve(a.size());
    for (size_t i=0;i<a.size();++i) out.push_back(classify((int)i,a,bonds_out));
    return out;
}

std::vector<OutAtom> Perceiver::perceive(const std::vector<InAtom>& atoms,
        const std::vector<std::pair<int,int>>& explicit_bonds) const {
    Bonds discard;
    return perceive(atoms, discard, explicit_bonds);
}

} // namespace somo_perceive
