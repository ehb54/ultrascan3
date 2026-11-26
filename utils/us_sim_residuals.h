//! \file us_sim_residuals.h
#ifndef US_SIM_RESIDUALS_H
#define US_SIM_RESIDUALS_H

#include "us_dataIO.h"
#include "us_extern.h"
#include "us_noise.h"

//! \brief Residuals of edited data against a simulation, as FEMatch, autoflow
//! analysis and the GMP reporter calculate them.
class US_UTIL_EXTERN US_SimResiduals
{
   public:
      //! \brief Fill \a resids with data minus simulation minus RI and TI
      //! noise, one row per scan.
      //!
      //! Where the simulation's radii differ from the data's, the simulation is
      //! interpolated linearly at each data radius.
      //! \param edited    Experimental data
      //! \param simulated Simulated data
      //! \param ti_noise  Time-invariant noise, used if its count is above 0
      //! \param ri_noise  Radially-invariant noise, used if its count is above 0
      //! \param excluded  Scan indexes left out of the variance
      //! \param scanCount Number of scans to calculate
      //! \param resids    Residuals, replaced
      //! \return The mean square residual of the scans not excluded; NaN if
      //! every scan is excluded.
      static double calculate( US_DataIO::EditedData&    edited,
                               US_DataIO::RawData&       simulated,
                               const US_Noise&           ti_noise,
                               const US_Noise&           ri_noise,
                               const QList<int>&         excluded,
                               int                       scanCount,
                               QVector<QVector<double>>& resids );

      //! \brief The value at \a xv, interpolated linearly in \a sx and \a sy;
      //! beyond the last point, the last two points are extended.
      static double interpolate( double xv, const double* sx, const double* sy, int ssize );
};

#endif
