#include "us_hydrodyn_residue_builder.h"

#include <cstdio>

namespace somo_residue_builder {

using somo_perceive::InAtom;
using somo_perceive::OutAtom;
using somo_perceive::Perceiver;

Built build(const std::string& resname,
            const std::vector<InAtom>& atoms,
            const std::vector<OutAtom>& perceived,
            const somo_perceive::Bonds& bonds,
            const std::vector<int>& idx,
            const Perceiver& perc,
            const somo_hydration::Table* hyd,
            const std::string& chemical_name,
            const Options& opt) {
    Built out;
    if (idx.empty() || atoms.size() != perceived.size()) return out;

    Perceiver::Properties props;
    props.bead_color = opt.bead_color;

    // ---- psv -------------------------------------------------------------------------------
    if (opt.compute_psv) {
        // free_molecule stays false: the entry is a monomeric unit, and SOMO adds one
        // structure-level covolume of its own in calc_vbar_updated.
        out.psv = somo_psv::compute(atoms, perceived, bonds, idx);
        if (out.psv.ok && out.psv.vbar > 0) {
            // The increments describe 25 C; somo.residue stores 20 C. Convert, or every entry
            // lands ~0.3% high -- see Options::psv_table_temperature.
            const double dT = opt.psv_table_temperature - opt.psv_increment_temperature;
            out.psv.vbar += opt.psv_dvdt * dT;
            props.have_vbar = true;
            props.vbar = out.psv.vbar;
        }
        for (const std::string& r : out.psv.review) props.review.push_back(r);
    }

    // ---- anhydrous volume ------------------------------------------------------------------
    if (opt.compute_volume) {
        std::vector<somo_volume::Sphere> sph;
        sph.reserve(idx.size());
        for (int i : idx)
            sph.push_back({atoms[i].x, atoms[i].y, atoms[i].z, perceived[i].vdw_radius});
        somo_volume::Options vo;
        vo.probe = opt.volume_probe;
        vo.grid = opt.volume_grid;
        const somo_volume::Result v = somo_volume::grid_volume(sph, vo);
        if (v.ok) {
            out.raw_volume = v.volume;
            const double f = v.volume >= opt.convention_split ? opt.convention_factor_large
                                                             : opt.convention_factor_small;
            out.molvol = f * v.volume;
            props.have_molvol = true;
            props.molvol = out.molvol;
        } else {
            props.review.push_back("anhydrous volume could not be computed on this grid");
        }
    }

    // ---- hydration --------------------------------------------------------------------------
    if (opt.compute_hydration) {
        // Chemistry rules first (pH 7), with the observed-majority table only as a fallback for
        // groups the rules do not recognise. The rules encode what the coded residues actually
        // do; the table is a statistical shadow of the same thing and is much weaker.
        out.hydration = somo_hydration::propose_by_rules(atoms, perceived, bonds, idx, hyd);
        if (out.hydration.ok) {
            props.hydration = out.hydration.per_atom;
            // Carry the deprotonated alternates through, resolving each one's mass and radius from
            // somo.hybrid so the emitted second half of the line is the table's own numbers rather
            // than anything invented here.
            props.alternate.assign(out.hydration.alternate.size(), {});
            for (size_t k = 0; k < out.hydration.alternate.size(); ++k) {
                const somo_hydration::Alternate& a = out.hydration.alternate[k];
                if (a.hybrid.empty()) continue;
                const somo_perceive::HybridInfo* hi = perc.table().get(a.hybrid);
                if (!hi) {   // the alternate type is not in somo.hybrid: say so, do not invent one
                    props.review.push_back(a.hybrid + ": ionized alternate type is not in "
                                           "somo.hybrid, the alternate state was not written");
                    continue;
                }
                props.alternate[k].mw     = hi->mw;
                props.alternate[k].radius = hi->radius;
                props.alternate[k].hybrid = a.hybrid;
                props.alternate[k].waters = a.waters;
                props.alternate[k].pKa    = a.pKa;
            }
            for (const std::string& r : out.hydration.review) props.review.push_back(r);
        }
    }

    // ---- emit -------------------------------------------------------------------------------
    std::vector<InAtom> res_atoms;
    std::vector<OutAtom> res_perc;
    res_atoms.reserve(idx.size());
    res_perc.reserve(idx.size());
    for (int i : idx) {
        res_atoms.push_back(atoms[i]);
        res_perc.push_back(perceived[i]);
    }
    const Perceiver::Emitted em =
        perc.emit_residue(resname, res_atoms, res_perc, chemical_name, &props);
    out.residue_block = em.residue_block;
    out.new_hybrids = em.new_hybrids;
    out.ok = true;
    return out;
}

} // namespace somo_residue_builder
