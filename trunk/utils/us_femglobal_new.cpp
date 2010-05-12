//! \file us_femglobal_new.cpp

#include "us_femglobal_new.h"
#include "us_constants.h"

US_FemGlobal_New::SimulationComponent::SimulationComponent()
{
   name                 = "New Component";
   bzero( analyteGUID, 16 );
   molar_concentration  = 0.0;
   signal_concentration = 1.0;
   vbar20               = TYPICAL_VBAR;
   mw                   = 50000.0;
   s                    = 0.0;
   D                    = 0.0;
   f                    = 1.0;
   f_f0                 = 1.25;
   wavelength           = 0.0;
   extinction           = 0.0;
   sigma                = 0.0;
   delta                = 0.0;
   stoichiometry        = 1;
   shape                = SPHERE;
   axial_ratio          = 10.0;
   analyte_type         = 0;  // Protein
   c0.radius       .clear();
   c0.concentration.clear();
}


US_FemGlobal_New::Association::Association()
{
   k_eq  = 0.0;
   k_off = 0.0;
   reaction_components.clear();
   stoichiometry.clear();
}

US_FemGlobal_New::ModelSystem::ModelSystem()
{
   viscosity       = VISC_20W;
   compressibility = COMP_25W;
   temperature     = NORMAL_TEMP;
   optics          = ABSORBANCE;
   description     = "New Model";
   coSedSolute     = -1;
   components  .clear();
   associations.clear();
}
