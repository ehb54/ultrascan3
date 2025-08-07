//! \file us_constraints.h
#ifndef US_CONSTRAINTS_H
#define US_CONSTRAINTS_H

#include <QtCore>

//! A class to describe constraints on a model system.
//! Note:  This class is not currently used and may go away.

class US_UTIL_EXTERN US_Constraints {
   public:
      //! \brief A class to give details to an individual constraint
      class Constraint {
         public:
            bool fit; //!< A flag to indicate if this parameter shoul dbe it or not
            double low; //!< Low value of the constraint
            double high; //!< High value of the constraint
      };

      //! A class to specify constraints of a component analyte
      class SimulationComponentConstraints {
         public:
            Constraint vbar20; //!<  Specific volume
            Constraint mw; //!<  Molecualr weight
            Constraint s; //!<  Sedimentation coefficient
            Constraint D; //!<  Diffusion corefficient
            Constraint sigma; //!<  Concentration dependency of s
            Constraint delta; //!<  Concentration dependency of D
            Constraint concentration; //!< Concentration of this analyte
            Constraint f_f0; //!<  Frictional ratio
      };

      //! A class to specify constraints of a component analyte
      class AssociationConstraints {
         public:
            Constraint keq; //!< Equiibrium Constant
            Constraint koff; //!< Dissociation Constant
      };

      //! A class that describes all constraints to a model when searching
      //! for best fit values.
      class ModelSystemConstraints {
         public:
            //! Constraints for each analyte
            vector<SimulationComponentConstraints> component_constraints;

            //! Constraints for each reaction
            vector<AssociationConstraints> association_constraints;

            /* The values below are not constraints!  They are a part of 
       * simulation parameters.
      //! Number of radial grid points used in simulation
      uint   simpoints;

      //! 
      US_FemGlobal::MeshType  mesh;

      //! Use moving or fixed time grid   (bool?)
      int    moving_grid;

      //! Loading volume (of lamella) in a band-forming centerpiece
      double band_volume;
      */
      };

      /*
   static int read_constraints(    ModelSystem&,
                                   ModelSystemConstraints&,
                                   const QString& );

   static int read_constraints(    ModelSystem&,
                                   ModelSystemConstraints&,
                                   const QStringList& );

   static int write_constraints(   ModelSystem&,
                                   ModelSystemConstraints&,
                                   const QString& );
 
   */
};
#endif
