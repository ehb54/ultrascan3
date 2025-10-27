//! \file us_hydrosim.h
#ifndef US_HYDROSIM_H
#define US_HYDROSIM_H

#include <QtCore>

#include "us_extern.h"
//#include "us_math.h"
//#include "us_buffer.h"
//#include "us_analyte.h"


//! A class describing the characteristics of a molecule
class US_UTIL_EXTERN US_Hydrosim
{
   public:
      US_Hydrosim();

      //!  A class to describe the characteristics of a molecule
      //!  assuming a particular shape
      class US_Hydrocomp
      {
         public:
            US_Hydrocomp();
            double s;       //!< Sedimentation coefficient
            double D;       //!< Diffusion coefficient
            double f;       //!< Frictional coefficient
            double f_f0;    //!< Frictional ratio f/f0
            double a;       //!< Major axis of the molecule
            double b;       //!< Minor axis of the molecule
            double volume;  //!< Volume of the molecule
      };

      double    mw;          //!< Molecular weight
      double    density;     //!< Density
      double    viscosity;   //!< Viscosity
      double    vbar;        //!< vbar at temperature
      double    temperature; //!< Temperature for current characteristics
      double    axial_ratio; //!< Axial ratio of major and minor radii
      QString   analyteGUID; //!< Analyte GUID related to this structure 

      US_Hydrocomp sphere;   //!< Spherical characteristics
      US_Hydrocomp prolate;  //!< Prolate Ellipsoid
      US_Hydrocomp oblate;   //!< Oblate Ellipsoid
      US_Hydrocomp rod;      //!< Rod characteristics

      //! Calculate characteristics of the molecule
      //! \param temperature - Degress C for the calulation
      void calculate( double );
};
#endif
