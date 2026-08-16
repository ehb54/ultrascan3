//! \file us_sim_species.h
#ifndef US_SIM_SPECIES_H
#define US_SIM_SPECIES_H

#include "us_extern.h"
#include "us_model.h"

//! \brief Describe one or more solute species and turn them into a model.
//! Accepts any coefficient pair US_Model::calc_coefficients() can solve.
class US_UTIL_EXTERN US_SimSpecies
{
   public:
      //! \brief One coefficient of a species, and whether it was supplied.
      //! Assignment marks it supplied, keeping zero distinguishable from an
      //! omitted value so validation can report it accurately.
      class US_UTIL_EXTERN Coeff
      {
         public:
            Coeff();

            //! Construct a supplied coefficient; implicit for direct assignment.
            Coeff( double value );

            double value;     //!< The coefficient, or zero when not supplied
            bool   supplied;  //!< Whether a caller stated this coefficient
      };

      //! \brief The hydrodynamic description of one solute species.
      //! Supply exactly two supported coefficients. A default-constructed
      //! instance supplies none; use defaultComponent() for a complete species.
      class US_UTIL_EXTERN Component
      {
         public:
            Component();

            //! Partial specific volume at 20 C, defaulting to a typical protein.
            double vbar20;

            Coeff s;       //!< Sedimentation coefficient (seconds)
            Coeff D;       //!< Diffusion coefficient (cm^2/s)
            Coeff mw;      //!< Molecular weight (Daltons)
            Coeff f;       //!< Frictional coefficient (g/s)
            Coeff f_f0;    //!< Frictional ratio (1.0 = perfect sphere)

            //! Descriptive name. Empty keeps the US_Model default.
            QString name;

            //! Absolute loading signal concentration for this species.
            //! A model's total loading concentration is the sum across all
            //! components; values are not normalized as fractions.
            double signal_concentration;
      };

      //! \brief One coefficient a caller may supply on a Component.
      struct US_UTIL_EXTERN Coefficient
      {
         const char* name;           //!< "s", "D", "mw", "f", or "f-f0"
         const char* description;    //!< What the value means, and its units
         Coeff Component::* field;   //!< The field the value is read into
      };

      //! \brief The coefficients validateComponent() accepts.
      //! Shared by validation and command-line option generation.
      static const QVector< Coefficient >& coefficients();

      //! \brief The species used when the caller specifies none.
      //! Uses the US_Model::SimulationComponent defaults.
      static Component defaultComponent();

      //! \brief Validate a species before US_Model solves for its coefficients.
      //! Checks the pair, value ranges, and sedimentation/buoyancy signs.
      //! \return An empty string if valid; otherwise, an error message.
      static QString validateComponent( const Component& c );

      //! \brief Validate every species of a mixture.
      //! Each must satisfy validateComponent(); the list must not be empty.
      //! \return An empty string if valid; otherwise, an error message
      //!         identifying which component failed.
      static QString validateComponents( const QVector< Component >& components );

      //! \brief Create a single-component absorbance model from a species.
      //! Validation is enforced here rather than left to the caller.
      //! \param c     Species to build.
      //! \param model Model populated only on success.
      //! \param error Error message on failure; empty on success.
      static bool model( const Component& c, US_Model& model, QString& error );

      //! \brief Create a multi-component absorbance model from a mixture.
      //! Components keep the given order and each carries its own
      //! concentration, so a mixture such as a monomer/dimer pair is one
      //! model rather than several.
      //! Validation and coefficient calculation are enforced here rather than
      //! left to the caller.
      //! \param components Species to build.
      //! \param model      Model populated only on success.
      //! \param error      Error message on failure; empty on success.
      static bool model( const QVector< Component >& components,
                         US_Model& model, QString& error );

      //! \brief Create a default single-component absorbance model.
      //! Built through the same checked path as defaultComponent().
      static US_Model model();
};

#endif // US_SIM_SPECIES_H
