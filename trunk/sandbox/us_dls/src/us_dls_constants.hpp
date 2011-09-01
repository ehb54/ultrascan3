 /*****************************************************
  *                                                   *
  * US Dynamic Light Scattering                       *
  *                                                   *
  * This file contains constants in use with the      *
  * US_DLS module.                                    *
  *                                                   *
  ****************************************************/

/*! Tolerance value for correlation baseline:
    it should be ~1 for mathmatical reasons.
    Yet, some (older) softwares do report,
    0 -- which makes physically more sense.

    If the baseline deviates more than this
    constant from 1, it should be corrected.
*/
const double CORR_TOLERANCE = 0.15;
