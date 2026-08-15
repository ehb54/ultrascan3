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
   // Coefficients start empty; vbar20 always has a typical-protein default.
   vbar20 = US_Model::SimulationComponent().vbar20;
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
   if ( c.vbar20 <= 0.0 )
      return QString( "vbar20 must be greater than zero (got %1)" ).arg( c.vbar20 );

   // Only s may legitimately be negative.
   if ( c.mw.supplied  &&  c.mw.value < 0.0 )
      return QString( "mw must be greater than zero (got %1)" ).arg( c.mw.value );
   if ( c.D.supplied   &&  c.D.value < 0.0 )
      return QString( "D must be greater than zero (got %1)" ).arg( c.D.value );
   if ( c.f.supplied   &&  c.f.value < 0.0 )
      return QString( "f must be greater than zero (got %1)" ).arg( c.f.value );
   if ( c.f_f0.supplied  &&  c.f_f0.value < 1.0 )
      return QString( "f-f0 must be at least 1.0, since a particle cannot be "
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

US_Model US_SimSpecies::model( const Component& c )
{
   US_Model model_out;
   model_out.description  = "us3-sim-inputs generated protein model v1";
   model_out.modelGUID    = US_Util::new_guid();
   model_out.optics       = US_Model::ABSORBANCE;
   model_out.analysis     = US_Model::MANUAL;

   US_Model::SimulationComponent sc;
   // Zero every unsupplied field so defaults cannot select the wrong pair.
   sc.vbar20 = c.vbar20;
   sc.s      = c.s.value;
   sc.D      = c.D.value;
   sc.mw     = c.mw.value;
   sc.f      = c.f.value;
   sc.f_f0   = c.f_f0.value;

   model_out.components << sc;
   model_out.update_coefficients();

   return model_out;
}

US_Model US_SimSpecies::model()
{
   return model( defaultComponent() );
}
