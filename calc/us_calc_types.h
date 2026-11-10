//! \file us_calc_types.h
//! \brief Value types of the UltraScan calculation library.
//!
//! The library holds the scientific calculations that the desktop programs
//! share.  It uses QtCore only:  no widgets, database access, file reading
//! or report formatting.
#ifndef US_CALC_TYPES_H
#define US_CALC_TYPES_H

#include <QString>
#include <QVector>

#include <optional>

namespace US_Calc
{
//! \brief A physical quantity of a model component.
enum class Quantity
{
   Sedimentation,        //!< s, sedimentation coefficient
   Diffusion,            //!< D, diffusion coefficient
   MolecularWeight,      //!< MW, molar mass
   FrictionalRatio,      //!< f/f0
   PartialSpecificVolume //!< vbar at 20 C
};

//! \brief One solute of a model, in the units UltraScan models store.
//!
//! s in seconds, D in cm^2/s, mw in g/mol (Da), f in g/s, f_f0 and the
//! signal concentration without units, vbar20 in mL/g.
struct Component
{
      double s                    = 0.0;
      double D                    = 0.0;
      double mw                   = 0.0;
      double f                    = 0.0;
      double f_f0                 = 0.0;
      double vbar20               = 0.0;
      double signal_concentration = 0.0;
};

//! \brief A model:  its components in their stored order.
//!
//! The order matters.  Sums run in component order so that results match
//! the legacy calculations exactly, also at tolerance edges.
struct Model
{
      QVector<Component> components;
      QString            description; //!< UltraScan model description
      bool               monte_carlo = false;
};

//! \brief A closed range of one quantity, in report units.
//!
//! Report units are those of the reporter's range items:  s in S
//! (1e-13 s), D in 1e-7 cm^2/s, MW in kDa, f/f0 and vbar without scaling.
//! Both bounds are included.
struct Range
{
      Quantity quantity = Quantity::Sedimentation;
      double   low      = 0.0;
      double   high     = 0.0;
};

//! \brief How serious a diagnostic is.
enum class Severity
{
   Warning, //!< The result is defined but should be checked
   Error    //!< The result, or part of it, is undefined
};

//! \brief What a diagnostic reports.
enum class Issue
{
   EmptyModel,            //!< The model has no components
   ZeroTotal,             //!< The total concentration is zero
   EmptyRange,            //!< No concentration lies within the range
   NegativeConcentration, //!< A component has a negative concentration
   NonFiniteValue,        //!< An input value is NaN or infinite
   InvalidRange,          //!< A range has low > high or a non-finite bound
   UnsupportedQuantity,   //!< The calculation does not support the quantity
   NoEligibleMembers,     //!< No member of a group can be used
   SeriesTooShort         //!< A series is too short for the requested operation
};

//! \brief A condition found while calculating.
//!
//! Results never contain NaN or infinite values.  A value that cannot be
//! defined is left empty and explained by a diagnostic.
struct Diagnostic
{
      Issue    issue    = Issue::EmptyModel;
      Severity severity = Severity::Error;
      QString  detail; //!< Human-readable explanation
};

//! \brief The name and version of the algorithm that produced a result.
//!
//! A new version is assigned whenever a result for the same input can
//! change, so stored results can be traced to the rule that made them.
struct AlgorithmVersion
{
      QString name;
      int     version = 0;
      QString description;
};

//! \brief True when any diagnostic is an error.
bool has_error( const QVector<Diagnostic>& diagnostics );
} // namespace US_Calc

#endif
