//! \file us_dmga_constr.cpp

#include "us_dmga_constr.h"
#include "us_settings.h"
#include "us_util.h"
#include "us_model.h"

// The constructor initializes the base or constraints model
US_dmGA_Constraints::US_dmGA_Constraints( US_Model* imodel )
{
   x_attr         = ATYPE_S;
   y_attr         = ATYPE_FF0;
   z_attr         = ATYPE_VBAR;

   if ( imodel != NULL )
   {
      if ( imodel->analysis == US_Model::DMGA_CONSTR )
      {  // The input is a constraints model
         cmodel         = *imodel;

         constraints_from_model();
      }

      else
      {  // The input is a base model
         bmodel         = *imodel;
      }
   }
}

// A function to load a base model
void US_dmGA_Constraints::load_base_model( US_Model* bmodelP )
{
   bmodel         = *bmodelP;
}

// A function to load a constraints model
void US_dmGA_Constraints::load_constraints( US_Model* cmodelP )
{
   cmodel         = *cmodelP;
}

// A function to return the current base model
bool US_dmGA_Constraints::get_base_model( US_Model* bmodelP )
{
   bool is_ok     = false;

   if ( bmodel.components.size() > 0  &&  bmodelP != NULL )
   {
      *bmodelP       = bmodel;
      is_ok          = true;
   }

   return is_ok;
}

// A function to return the current constraints model
bool US_dmGA_Constraints::get_constraints( US_Model* cmodelP )
{
   bool is_ok     = false;

   if ( cmodel.components.size() > 0  &&  cmodelP != NULL )
   {
      *cmodelP       = cmodel;
      is_ok          = true;
   }

   return is_ok;
}

// Internal utility to complete constraints definitions from constraints model
void US_dmGA_Constraints::constraints_from_model()
{
#if 0
   Constraint attr;
   attribs.clear();

   QString cname  = "";
   QString pcname = "";
   int     lcx    = cmodel.components.size() - 1;
   int     mcompx = 0;
   int     ccompx = 0;

   for ( int ii = 0; ii < cmodel.components.size(); ii++ )
   {
      US_Model::SimulationComponent* sc  = &cmodel.components[ ii ];
      cname          = sc->name;
      pcname         = cname;
      double sval    = sc->s;
      double kval    = sc->f_f0;
      double wval    = sc->mw;
      double dval    = sc->D;
      double fval    = sc->f;
      double vval    = sc->vbar20;

      if ( sval != 0.0 )
         x_attr         = ATYPE_S;
      else if ( kval != 0.0 )
         x_attr         = ATYPE_FF0;
      else if ( wval != 0.0 )
         x_attr         = ATYPE_MW;
      else if ( dval != 0.0 )
         x_attr         = ATYPE_D;
      else if ( fval != 0.0 )
         x_attr         = ATYPE_F;
      else if ( vval != 0.0 )
         x_attr         = ATYPE_VBAR;

      if (      x_attr != ATYPE_FF0   &&  wval != 0.0 )
         y_attr         = ATYPE_FF0;
      else if ( x_attr != ATYPE_D     &&  dval != 0.0 )
         y_attr         = ATYPE_D;
      else if ( x_attr != ATYPE_F     &&  fval != 0.0 )
         y_attr         = ATYPE_F;
      else if ( x_attr != ATYPE_VBAR  &&  vval != 0.0 )
         y_attr         = ATYPE_VBAR;
   }

#endif
}

// Internal utility to build an initial constraints from a base model
void US_dmGA_Constraints::constraints_from_base()
{
   if ( bmodel.components.size() < 1 )
   {
      qDebug() << "*ERROR* Unable to build constraints from empty base";
      return;
   }

}

