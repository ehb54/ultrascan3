/*****************************************************
 *                                                   *
 * US Dynamic Light Scattering                       *
 *                                                   *
 * Fitting routines for single measurements.         *
 *                                                   *
 ****************************************************/

#include "us_dls_measurement.hpp"

class DLS_fit; // forward declaration

//! implements the cumulant fit after Koppel 1972
DLS_fit koppel_fit(const us::dls::DLS_Measurement &d)

//! implements the cumulant fit after Friske, XXX
DLS_fit friske_fit(const us::dls::DLS_Measurement &d)
