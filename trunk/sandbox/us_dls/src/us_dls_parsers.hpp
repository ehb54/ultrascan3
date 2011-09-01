/*****************************************************
 *                                                   *
 * US Dynamic Light Scattering                       *
 *                                                   *
 * This file contains parser declarations, currently *
 * for:                                              *
 * - DLS-raw measurement (Dynamics, 7 from Wyatt)    *
 *                                                   *
 ****************************************************/

// $Rev$
// $LastChangeDate$
// $Author$

#include<iostreams>
#include<vector>
#include<string> // TODO: probably to be replaced by Qt-string

#infdef US_DLS_PARSERS
#define US_DLS_PARSERS

namespace us::dls {
  class DLS_Measurement; // forward class declaration

  //! reading a single measurement recorded by Dynamics 7
  DLS_Measurement read_measurement(const std::string &fname);
} // namespace us::dls

#endif // ~US_DLS_PARSERS
