//! \file us_sim_species.cpp
#include "us_sim_species.h"
#include "us_util.h"
#include "us_constants.h"
#include <QStringList>

US_SimSpecies::Coeff::Coeff()
{
   value    = 0.0;
   supplied = false;
}

US_SimSpecies::Coeff::Coeff( double coeff_value )
{
   value    = coeff_value;
   supplied = true;
}

US_SimSpecies::Component::Component()
{
   // Coefficients start empty; vbar20 and the loading concentration always
   // have the library defaults.
   US_Model::SimulationComponent sc_defaults;

   vbar20        = sc_defaults.vbar20;
   signal_concentration = sc_defaults.signal_concentration;
}

const QVector< US_SimSpecies::Coefficient >& US_SimSpecies::coefficients()
{
   static const QVector< Coefficient > table = {
      { "s",    "Sedimentation coefficient in seconds", &Component::s    },
      { "D",    "Diffusion coefficient (cm^2/s)",       &Component::D    },
      { "mw",   "Molecular weight (Da)",                &Component::mw   },
      { "f",    "Frictional coefficient (g/s)",         &Component::f    },
      { "f-f0", "Frictional ratio (1.0 = perfect sphere)",
                                                        &Component::f_f0 },
   };

   return table;
}

US_SimSpecies::Component US_SimSpecies::defaultComponent()
{
   // Do not copy the default f; it would become a third supplied coefficient.
   US_Model::SimulationComponent sc_defaults;
   Component c;

   c.vbar20 = sc_defaults.vbar20;
   c.mw     = sc_defaults.mw;
   c.f_f0   = sc_defaults.f_f0;

   return c;
}

QString US_SimSpecies::validateComponent( const Component& c )
{
   if ( ! qIsFinite( c.vbar20 ) || c.vbar20 <= 0.0 )
      return QString( "vbar20 must be finite and greater than zero (got %1)" )
         .arg( c.vbar20 );

   if ( ! qIsFinite( c.signal_concentration ) || c.signal_concentration <= 0.0 )
      return QString( "signal concentration must be finite and greater than "
                      "zero (got %1)" ).arg( c.signal_concentration );

   // Only s may legitimately be negative. Zero is rejected along with the
   // negatives: a massless, non-diffusing, frictionless species is not one
   // calc_coefficients() can solve for.
   if ( c.s.supplied && ( ! qIsFinite( c.s.value ) || c.s.value == 0.0 ) )
      return QString( "s must be finite and nonzero (got %1)" ).arg( c.s.value );
   if ( c.mw.supplied && ( ! qIsFinite( c.mw.value ) || c.mw.value <= 0.0 ) )
      return QString( "mw must be finite and greater than zero (got %1)" )
         .arg( c.mw.value );
   if ( c.D.supplied && ( ! qIsFinite( c.D.value ) || c.D.value <= 0.0 ) )
      return QString( "D must be finite and greater than zero (got %1)" )
         .arg( c.D.value );
   if ( c.f.supplied && ( ! qIsFinite( c.f.value ) || c.f.value <= 0.0 ) )
      return QString( "f must be finite and greater than zero (got %1)" )
         .arg( c.f.value );
   if ( c.f_f0.supplied &&
        ( ! qIsFinite( c.f_f0.value ) || c.f_f0.value < 1.0 ) )
      return QString( "f-f0 must be finite and at least 1.0, since a particle cannot be "
                      "more compact than the equivalent sphere (got %1)" )
         .arg( c.f_f0.value );

   // calc_coefficients() requires exactly two inputs.
   QStringList supplied;
   QStringList names;

   for ( const Coefficient& coeff : coefficients() )
   {
      names << coeff.name;

      if ( ( c.*( coeff.field ) ).supplied )
         supplied << coeff.name;
   }

   if ( supplied.size() != 2 )
      return QString( "exactly two of %1 must be given (got %2: %3)" )
         .arg( names.join( ", " ) )
         .arg( supplied.size() )
         .arg( supplied.isEmpty() ? QString( "none" ) : supplied.join( ", " ) );

   // D and f are the one pair calc_coefficients() has no branch for: both
   // describe transport without fixing the mass, so the system is undetermined.
   if ( supplied.contains( "D" )  &&  supplied.contains( "f" ) )
      return "D and f cannot be given as the only pair; add or substitute one "
             "of s, mw, or f-f0";

   // Explain sedimentation/buoyancy sign errors before coefficient calculation.
   if ( c.s.supplied )
   {
      double buoyancy = 1.0 - ( c.vbar20 * DENS_20W );

      if ( ( c.s.value < 0.0  &&  buoyancy > 0.0 )  ||
           ( c.s.value > 0.0  &&  buoyancy < 0.0 ) )
         return QString( "the sign of s (%1) disagrees with the buoyancy "
                         "implied by vbar20 (%2); a species floats only when "
                         "vbar20 exceeds %3 mL/g" )
            .arg( c.s.value ).arg( c.vbar20 ).arg( 1.0 / DENS_20W );
   }

   return QString();
}

QString US_SimSpecies::validateComponents( const QVector< Component >& components )
{
   if ( components.isEmpty() )
      return "at least one component must be given";

   for ( int ii = 0; ii < components.count(); ii++ )
   {
      QString error = validateComponent( components[ ii ] );

      if ( ! error.isEmpty() )
         return QString( "component %1: %2" ).arg( ii + 1 ).arg( error );
   }

   return QString();
}

bool US_SimSpecies::model( const Component& c, US_Model& model_out,
                           QString& error )
{
   return model( QVector< Component >() << c, model_out, error );
}

bool US_SimSpecies::model( const QVector< Component >& components,
                           US_Model& model_out, QString& error )
{
   error = validateComponents( components );
   if ( ! error.isEmpty() )
      return false;

   US_Model candidate;
   candidate.description  = "us3-sim-inputs generated protein model v1";
   candidate.modelGUID    = US_Util::new_guid();
   candidate.optics       = US_Model::ABSORBANCE;
   candidate.analysis     = US_Model::MANUAL;

   for ( const Component& c : components )
   {
      US_Model::SimulationComponent sc;
      // Zero every unsupplied field so defaults cannot select the wrong pair.
      sc.vbar20 = c.vbar20;
      sc.s      = c.s.value;
      sc.D      = c.D.value;
      sc.mw     = c.mw.value;
      sc.f      = c.f.value;
      sc.f_f0   = c.f_f0.value;

      sc.signal_concentration = c.signal_concentration;

      if ( ! c.name.isEmpty() )
         sc.name = c.name;

      candidate.components << sc;
   }

   // Solves each component independently; components do not interact.
   if ( ! candidate.update_coefficients() )
   {
      error = "US_Model could not calculate a complete coefficient set";
      return false;
   }

   model_out = candidate;
   error.clear();
   return true;
}

US_Model US_SimSpecies::model()
{
   US_Model model_out;
   QString  error;
   bool     ok = model( defaultComponent(), model_out, error );
   Q_ASSERT_X( ok, "US_SimSpecies::model", qPrintable( error ) );
   return model_out;
}
