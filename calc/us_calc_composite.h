//! \file us_calc_composite.h
//! \brief Monte Carlo composite model (algorithm "mc_composite").
//!
//! The model US_Model::load makes of a Monte Carlo file, which holds one
//! model per iteration.  US_Model::load_multi_model calls this.
#ifndef US_CALC_COMPOSITE_H
#define US_CALC_COMPOSITE_H

#include "us_calc_types.h"

#include <cstddef>
#include <string>
#include <vector>

namespace US_Calc
{
//! \brief The composite model and where each of its components came from.
struct CompositeResult
{
      //! Components in key order, each a copy of its last matching source
      //! component with the concentration averaged over the iterations.
      Model                    model;
      //! For each component:  the index of its last matching source
      //! component, counting the iterations' components in order.  Callers
      //! with richer component records copy the record at this index.
      std::vector<std::size_t> source_index;
      std::vector<Diagnostic>  diagnostics;
      AlgorithmVersion         algorithm;
};

//! \brief The text that identifies a solute point:  "%10.4f %8.5f" of s in
//! S (s * 1e13) and of the second value (f/f0 or vbar20).
std::string     mc_solute_key( double s, double second );

//! \brief Build the composite model of Monte Carlo iterations.
//!
//! - The second key value is f/f0 when at least as many iterations have a
//!   constant vbar20 (range < 1e-4) as a constant f/f0 (range < 1e-3), and
//!   vbar20 otherwise.
//! - Components with the same key are merged:  the concentration is their
//!   sum, in order, times 1/iterations; every other value comes from the
//!   last match.  Components are ordered by their sorted keys.
//! - The description is the first iteration's with its label's last '_'
//!   field replaced by _mcN<iterations>:
//!   a.b.c_d_mc0001.model gives a.b.c_d_mcN003.model for 3 iterations.
//! - No iterations, or an iteration without components, is an error.
CompositeResult mc_composite( const std::vector<Model>& iterations );
} // namespace US_Calc

#endif
