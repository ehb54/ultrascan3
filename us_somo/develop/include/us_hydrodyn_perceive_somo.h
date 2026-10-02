// us_hydrodyn_perceive_somo.h -- SOMO/Qt adapter for the chemical perceiver.
//
// The perception core (us_hydrodyn_perceive.h) is deliberately Qt-free so it can be unit tested
// standalone. This layer is the only Qt-aware part: it converts SOMO's parsed PDB structures into
// the core's input and turns the result into tentative somo.residue entries.
//
// See ehb54/ultrascan-tickets#978.
#ifndef US_HYDRODYN_PERCEIVE_SOMO_H
#define US_HYDRODYN_PERCEIVE_SOMO_H

#include "us_hydrodyn_hydration.h"
#include "us_hydrodyn_residue_builder.h"

#include <QString>
#include <QStringList>
#include <QList>
#include <vector>
#include <set>
#include <map>

#include "us_hydrodyn_pdbdefs.h"
#include "us_hydrodyn_perceive.h"

namespace somo_perceive {

// One proposed somo.residue entry for a residue SOMO does not already code.
struct Tentative {
    QString     resName;        // base residue code (SOMO's "_NC<n>" instance suffix removed)
    int         instances = 0;  // how many instances of this residue the model contains
    QString     chemical_name;  // from the PDB HETNAM header record, when present
    QString     block;          // the somo.residue entry, ready for review
    QStringList new_hybrids;    // somo.hybrid rows for any type not already in the table
    double      vbar = 0;       // computed psv, 0 = not computed
    double      molvol = 0;     // computed anhydrous volume in A^3, 0 = not computed
    double      hydration = 0;  // proposed residue total, waters
    int         atoms   = 0;
    int         flagged = 0;    // atoms whose perception was uncertain (needs user confirmation)
};

// Convert a SOMO-parsed model into perceiver input. The WHOLE model is converted, not one residue:
// bond perception needs the surrounding context (peptide/phosphodiester links, disulfides, metal
// coordination) to get coordination numbers -- and hence implicit-H counts -- right.
std::vector<InAtom> from_pdb_model( const PDB_model & model );

// Explicit connectivity from a PDB file's CONECT records, as index pairs into from_pdb_model()'s
// output. Authoritative where present; essential for metal-oxo/coordination compounds, where
// metal-ligand bonds are the covalent skeleton and distance heuristics alone are not enough.
std::vector< std::pair< int, int > > conect_bonds( const QString & pdb_filename,
                                                   const std::vector<InAtom> & atoms );

// Read HETNAM records -> hetID : chemical name. PDB files name their non-standard residues in the
// header, which gives a far better entry name than the bare 3-letter code.
std::map<QString,QString> hetnam_names( const QString & pdb_filename );

// Hydration lookup built from the residue table SOMO has actually loaded, so the proposal for an
// unknown residue is always consistent with the user's own somo.residue rather than a snapshot
// compiled into the binary.
somo_hydration::Table hydration_from_residue_list( const std::vector<struct residue> & coded );

// Perceive the residues named in 'to_perceive' (SOMO's own unknown_residues set: the residues it
// could not code and for which it would otherwise synthesise a generic ABB average bead).
// SOMO gives every instance a unique "<RESNAME>_NC<n>" name; entries are de-duplicated back to the
// base residue name, since one confirmation should cover all instances of the same chemistry.
// 'pdb_filename' is optional and only used to pick up CONECT/HETNAM records.
// `hyd` may be null, in which case hydration is left unset rather than guessed. `opt` carries
// the psv/volume/hydration switches and the bead colour, so a headless run can pin them.
QList<Tentative> perceive_unknown( const PDB_model         & model,
                                   const HybridTable       & tbl,
                                   const std::set<QString> & to_perceive,
                                   const QString           & pdb_filename = QString(),
                                   const somo_hydration::Table * hyd = nullptr,
                                   const somo_residue_builder::Options & opt =
                                       somo_residue_builder::Options() );

// ---------------------------------------------------------------------------------------------
// Hand-testing aid: run perception over residues somo.residue DOES code and diff the result
// against the curated types. This is the same check the standalone regression harness performs,
// exposed inside SOMO so a structure can be spot-checked with the user's own installed tables.

struct CompareRow {
    QString res, atom, expected, got;
};

struct CompareResult {
    long scored = 0;                       // atoms compared
    long exact  = 0;                       // perceived label identical to the curated one
    long phys   = 0;                       // mw / radius / electrons identical (label may differ)
    long residues = 0;                     // coded residue instances seen
    QList<CompareRow>      mismatches;     // physics mismatches only
    std::map<QString,long> by_pair;        // "expected -> got" counts
};

// 'curated' is resName -> (atomName -> hybrid type), built by the caller from SOMO's residue_list.
CompareResult compare_against_table(
    const PDB_model & model,
    const HybridTable & tbl,
    const std::map< QString, std::map< QString, QString > > & curated,
    const QString & pdb_filename = QString() );

// ---- validation: predict the CODED residues as if they were unknown ------------------------
// The same idea as compare_against_table, but for the computed properties rather than the atom
// typing: rebuild each coded residue from geometry alone and check vbar, molvol and hydration
// against what somo.residue actually stores. This is the only end-to-end measure of the whole
// pipeline -- perception included -- so it is what any accuracy claim must rest on.
struct ValidateRow {
    QString resName;
    int     instances = 0;
    double  vbar_computed = 0,      vbar_stored = 0;
    double  molvol_computed = 0,    molvol_stored = 0;
    double  hydration_computed = 0, hydration_stored = 0;
};

struct ValidateResult {
    QList<ValidateRow> rows;              // one per residue TYPE, averaged over instances
    long skipped_incomplete = 0;          // atom count differs from the stored entry
    long skipped_terminal = 0;            // chain terminus: an extra OXT makes it another molecule
    long instances = 0;
};

ValidateResult validate_against_table(
    const PDB_model & model,
    const HybridTable & tbl,
    const std::vector<struct residue> & coded,
    const somo_hydration::Table * hyd = nullptr,
    const somo_residue_builder::Options & opt = somo_residue_builder::Options(),
    const QString & pdb_filename = QString() );

} // namespace somo_perceive

#endif
