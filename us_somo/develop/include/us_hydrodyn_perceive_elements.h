// element_data.h -- atomic reference data for the SOMO perceiver.
// Self-contained, header-only, no external deps. Values are public-domain constants.
#pragma once
#include <string>
#include <map>
#include <set>
#include <cctype>
#include <algorithm>

namespace somo_perceive {

// Normalize an element token (PDB col 77-78 or an atom-name guess) to uppercase symbol.
inline std::string norm_element(std::string s) {
    // strip spaces/digits/+/-
    std::string out;
    for (char c : s) if (std::isalpha((unsigned char)c)) out += (char)std::toupper((unsigned char)c);
    return out;
}

struct ElementInfo {
    int    Z;           // atomic number
    double mass;        // standard atomic weight
    double r_cov;       // Cordero 2008 single-bond covalent radius (Angstrom)
    double r_vdw;       // Bondi (or estimated) vdW radius, fallback only
};

// Curated subset covering biomolecules + common ions/metals in somo.hybrid.new.
inline const std::map<std::string, ElementInfo>& element_table() {
    static const std::map<std::string, ElementInfo> t = {
        {"H",  {1,   1.008, 0.31, 1.20}},
        {"B",  {5,  10.811, 0.84, 1.92}},
        {"BE", {4,   9.012, 0.96, 1.53}},
        {"C",  {6,  12.011, 0.76, 1.70}},
        {"N",  {7,  14.007, 0.71, 1.55}},
        {"O",  {8,  15.999, 0.66, 1.52}},
        {"F",  {9,  18.998, 0.57, 1.47}},
        {"NA", {11, 22.990, 1.66, 2.27}},
        {"MG", {12, 24.305, 1.41, 1.73}},
        {"AL", {13, 26.982, 1.21, 1.84}},
        {"SI", {14, 28.085, 1.11, 2.10}},
        {"P",  {15, 30.974, 1.07, 1.80}},
        {"S",  {16, 32.06,  1.05, 1.80}},
        {"CL", {17, 35.45,  1.02, 1.75}},
        {"K",  {19, 39.098, 2.03, 2.75}},
        {"CA", {20, 40.078, 1.76, 2.31}},
        {"MN", {25, 54.938, 1.39, 2.05}},
        {"FE", {26, 55.845, 1.32, 2.04}},
        {"CO", {27, 58.933, 1.26, 2.00}},
        {"CU", {29, 63.546, 1.32, 1.96}},
        {"ZN", {30, 65.38,  1.22, 2.01}},
        {"SE", {34, 78.971, 1.20, 1.90}},
        {"BR", {35, 79.904, 1.20, 1.85}},
        {"MO", {42, 95.95,  1.54, 2.10}},
        {"PD", {46, 106.42, 1.39, 2.05}},
        {"CD", {48, 112.414,1.44, 2.18}},
        {"I",  {53, 126.904,1.39, 1.98}},
    };
    return t;
}

// Residue names whose atom naming follows the protein/nucleic convention, where a two-letter
// atom name is NEVER a two-letter element: CA is the alpha carbon, NE2 a nitrogen, SD a sulfur.
inline const std::set<std::string>& standard_residue_names() {
    static const std::set<std::string> t = {
        "ALA","ARG","ASN","ASP","CYS","GLN","GLU","GLY","HIS","ILE","LEU","LYS","MET","PHE",
        "PRO","SER","THR","TRP","TYR","VAL","SEC","PYL","ASX","GLX","UNK",
        "HSD","HSE","HSP","HID","HIE","HIP","CYX","CYM","LYN","ASH","GLH",   // force-field variants
        "A","C","G","T","U","DA","DC","DG","DT","DU","RA","RC","RG","RU",    // nucleotides
        "HOH","WAT","DOD","TIP","SOL"
    };
    return t;
}

// Element from a PDB ATOM NAME, for files whose element column (77-78) is empty.
//
// norm_element() alone is WRONG for this: it only strips non-alphabetics, so the alpha carbon
// "CA" becomes calcium, "NE2" becomes neon, and "CB", "CG", "OD1" become nothing at all. Every
// atomic volume increment then silently evaluates to zero and the psv collapses to roughly the
// electrostriction terms -- a meaningless number that still looks like one.
// 1AO6-compl_monA.pdb in the demo set has no element column, which is how this was found.
//
// The discriminator is the RESIDUE, not the atom name. In a standard residue the convention is
// fixed and the element is always the leading letter. In a ligand it is not: HEM's iron is "FE",
// a chloride is "CL", a selenomethionine sulfur analogue is "SE" -- and reading those by leading
// letter yields fluorine, carbon and sulfur. Taking the leading letter everywhere got 6 of 15
// test cases wrong, all of them ligand atoms, which is the population this code exists for.
//
// Residual ambiguity: a ligand carbon named bare "CA" reads as calcium. Ligand carbons are
// normally C1/CAA/CB1 rather than a bare "CA", so this is rare -- and the psv now refuses to
// compute rather than guessing when an element cannot be resolved at all.
// Atom names used by the protein/nucleic naming convention, where the element is ALWAYS the
// leading letter. This is the set that collides with real two-letter elements: CA (calcium),
// CD (cadmium), CE (cerium), NE (neon), NA (sodium), SE (selenium), CU, CL, CS, CO, NI...
// A modified residue -- MSE, PTR, CGU, or any renamed one -- carries protein atom names under a
// NON-standard residue name, so keying only on the residue name is not enough: it read every
// backbone CA as calcium, which changed the hybrid, the radius, the grid volume and the
// hydration as well as the psv.
inline const std::set<std::string>& protein_atom_names() {
    static const std::set<std::string> t = {
        "CA","CB","CG","CD","CE","CZ","CH","OG","OH","OD","OE","OX","SG","SD",
        "NE","ND","NZ","NH","OP","C","N","O","P","S"
    };
    return t;
}

inline std::string element_from_atom_name(const std::string& atom_name,
                                          const std::string& res_name) {
    std::string n, r;
    for (char c : atom_name) if (std::isalpha((unsigned char)c)) n += (char)std::toupper((unsigned char)c);
    for (char c : res_name)  if (std::isalpha((unsigned char)c)) r += (char)std::toupper((unsigned char)c);
    if (n.empty()) return n;

    const std::string one(1, n[0]);
    const bool one_is_element = element_table().count(one) > 0;

    // 1. a lone ion: the atom name IS the residue name (HETATM "CA" in residue "CA")
    if (n.size() >= 2 && n == r && element_table().count(n)) return n;
    // 2. a standard residue always follows the protein/nucleic convention
    if (standard_residue_names().count(r)) return one_is_element ? one : n;
    // 3. a protein-style atom name follows it too, whatever the residue is called --
    //    this is what makes modified and renamed residues work
    if (protein_atom_names().count(n)) return one_is_element ? one : n;
    // 4. otherwise a two-letter element is what it looks like: FE, CL, SE, BR, CU, ZN, MG
    if (n.size() == 2 && element_table().count(n)) return n;
    return one_is_element ? one : n;
}

// Monatomic ions we treat as non-bonding during perception; value = default hybrid name
// (matches the charge convention used in somo.hybrid.new).
inline const std::map<std::string, std::string>& monatomic_ion_hybrid() {
    static const std::map<std::string, std::string> m = {
        {"NA", "NA+1"}, {"K",  "K+1"},  {"CA", "CA+2"}, {"MG", "MG+2"},
        {"MN", "MN+2"}, {"FE", "FE+2"}, {"CO", "CO+3"}, {"CU", "CU+2"},
        {"ZN", "ZN+2"}, {"CD", "CD+2"}, {"MO", "MO+3"}, {"PD", "PD+2"},
        {"CL", "CL-1"}, {"F",  "F-1"},  {"BE", "BE+2"},
    };
    return m;
}

inline bool has_element(const std::string& e) { return element_table().count(e) != 0; }
inline double covalent_radius(const std::string& e) {
    auto it = element_table().find(e);
    return it == element_table().end() ? 0.77 : it->second.r_cov;
}
inline double vdw_radius(const std::string& e) {
    auto it = element_table().find(e);
    return it == element_table().end() ? 1.70 : it->second.r_vdw;
}
inline int atomic_number(const std::string& e) {
    auto it = element_table().find(e);
    return it == element_table().end() ? 0 : it->second.Z;
}
inline double atomic_mass(const std::string& e) {
    auto it = element_table().find(e);
    return it == element_table().end() ? 0.0 : it->second.mass;
}

} // namespace somo_perceive
