//! \file us_calc_units.cpp
#include "us_calc_units.h"

namespace US_Calc
{
double report_scale( Quantity quantity )
{
   switch ( quantity )
   {
      case Quantity::Sedimentation:
         return 1.0e-13;
      case Quantity::Diffusion:
         return 1.0e-7;
      case Quantity::MolecularWeight:
         return 1.0e3;
      case Quantity::FrictionalRatio:
      case Quantity::PartialSpecificVolume:
         return 1.0;
   }
   return 1.0;
}

double to_model_units( Quantity quantity, double report_value )
{
   if ( quantity == Quantity::FrictionalRatio || quantity == Quantity::PartialSpecificVolume )
   {
      return report_value;
   }
   return report_value * report_scale( quantity );
}

double component_value( const Component& component, Quantity quantity )
{
   switch ( quantity )
   {
      case Quantity::Sedimentation:
         return component.s;
      case Quantity::Diffusion:
         return component.D;
      case Quantity::MolecularWeight:
         return component.mw;
      case Quantity::FrictionalRatio:
         return component.f_f0;
      case Quantity::PartialSpecificVolume:
         return component.vbar20;
   }
   return 0.0;
}

std::optional<Quantity> quantity_from_report_type( const std::string& type )
{
   if ( type == "s" )
   {
      return Quantity::Sedimentation;
   }
   if ( type == "D" )
   {
      return Quantity::Diffusion;
   }
   if ( type == "MW" )
   {
      return Quantity::MolecularWeight;
   }
   if ( type == "f/f0" )
   {
      return Quantity::FrictionalRatio;
   }
   return std::nullopt;
}

std::string quantity_name( Quantity quantity )
{
   switch ( quantity )
   {
      case Quantity::Sedimentation:
         return "s";
      case Quantity::Diffusion:
         return "D";
      case Quantity::MolecularWeight:
         return "MW";
      case Quantity::FrictionalRatio:
         return "f/f0";
      case Quantity::PartialSpecificVolume:
         return "vbar";
   }
   return "";
}
} // namespace US_Calc
