// us_hydrodyn_perceive_somo.h -- SOMO/Qt adapter for the chemical perceiver.
//
// The perception core (us_hydrodyn_perceive.h) is deliberately Qt-free so it can be unit tested
// standalone. This layer is the only Qt-aware part: it converts SOMO's parsed PDB structures into
// the core's input and turns the result into tentative somo.residue entries.
//
// See ehb54/ultrascan-tickets#978.
#ifndef US_HYDRODYN_PERCEIVE_SOMO_H
#define US_HYDRODYN_PERCEIVE_SOMO_H

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

// Perceive the residues named in 'to_perceive' (SOMO's own unknown_residues set: the residues it
// could not code and for which it would otherwise synthesise a generic ABB average bead).
// SOMO gives every instance a unique "<RESNAME>_NC<n>" name; entries are de-duplicated back to the
// base residue name, since one confirmation should cover all instances of the same chemistry.
// 'pdb_filename' is optional and only used to pick up CONECT/HETNAM records.
QList<Tentative> perceive_unknown( const PDB_model         & model,
                                   const HybridTable       & tbl,
                                   const std::set<QString> & to_perceive,
                                   const QString           & pdb_filename = QString() );

} // namespace somo_perceive

#endif
