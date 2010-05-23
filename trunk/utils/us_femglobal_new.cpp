//! \file us_femglobal_new.cpp

#include "us_femglobal_new.h"
#include "us_constants.h"

#include <uuid/uuid.h>

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
   bufferGUID  .clear();
   guid        .clear();
   components  .clear();
   associations.clear();
}

bool US_FemGlobal_New::ModelSystem::read_from_disk( const QString& filename )
{
   QFile file( filename );

   if ( ! file.open( QIODevice::ReadOnly | QIODevice::Text) )
   {
      qDebug() << "Cannot open file for reading: " << filename;
      return false;
   }

   QXmlStreamReader     xml( &file );
   QXmlStreamAttributes a;
   bool                 read_next = true;

   while ( ! xml.atEnd() )
   {
      if ( read_next) xml.readNext();
      read_next = true;

      if ( xml.isStartElement() )
      {
         if ( xml.name() == "model" )
         {
            a = xml.attributes();

            description     = a.value( "description"     ).toString();
            bufferGUID      = a.value( "bufferGuid"      ).toString();
            bufferDesc      = a.value( "bufferDesc"      ).toString();
            guid            = a.value( "guid"            ).toString();
            density         = a.value( "density"         ).toString().toDouble();
            viscosity       = a.value( "viscosity"       ).toString().toDouble();
            compressibility = a.value( "compressibility" ).toString().toDouble();
            temperature     = a.value( "temperature"     ).toString().toDouble();
            coSedSolute     = a.value( "coSedSolute"     ).toString().toInt();
         }

         else if ( xml.name() == "analyte" )
         {
            US_FemGlobal_New::SimulationComponent sc;
            a = xml.attributes();

            QString guid = a.value( "guid" ).toString();
            uuid_parse( guid.toAscii().data(), sc.analyteGUID );
            
            sc.name        = a.value( "name"       ).toString();
            sc.vbar20      = a.value( "vbar20"     ).toString().toDouble();
            sc.mw          = a.value( "mw"         ).toString().toDouble();
            sc.s           = a.value( "s"          ).toString().toDouble();
            sc.D           = a.value( "D"          ).toString().toDouble();
            sc.f           = a.value( "f"          ).toString().toDouble();
            sc.f_f0        = a.value( "f_f0"       ).toString().toDouble();
            sc.wavelength  = a.value( "wavelength" ).toString().toDouble();
            sc.extinction  = a.value( "extinction" ).toString().toDouble();
            sc.axial_ratio = a.value( "axial"      ).toString().toDouble();
            sc.sigma       = a.value( "sigma"      ).toString().toDouble();
            sc.delta       = a.value( "delta"      ).toString().toDouble();

            sc.molar_concentration  = a.value( "molar"  ).toString().toDouble();
            sc.signal_concentration = a.value( "signal" ).toString().toDouble();
            sc.stoichiometry        = a.value( "stoich" ).toString().toInt();
            sc.shape                = 
               (US_FemGlobal_New::ShapeType)a.value( "shape"  ).toString().toInt();
            sc.analyte_type         = a.value( "type"   ).toString().toInt();

            mfem_scans( xml, sc );
            read_next = false; // Skip the next read

            components << sc;
         }

         else if ( xml.name() == "association" )
         {
            US_FemGlobal_New::Association as;
            a = xml.attributes();

            as.k_eq  = a.value( "k_eq"  ).toString().toDouble();
            as.k_off = a.value( "k_off" ).toString().toDouble();

            get_associations( xml, as );
            read_next = false; // Skip the next read

            associations << as;
         }
      }
   }

   file.close();
   return true;
}

void US_FemGlobal_New::ModelSystem::mfem_scans( 
      QXmlStreamReader& xml, 
      US_FemGlobal_New::SimulationComponent& sc )
{
   while ( ! xml.atEnd() )
   {
      xml.readNext();

      if ( xml.isStartElement() )
      {
         if ( xml.name() == "mfem_scan" )
         {
            QXmlStreamAttributes a = xml.attributes();

            sc.c0.radius        << a.value( "radius" ).toString().toDouble();
            sc.c0.concentration << a.value( "conc"   ).toString().toDouble();
         }
         else 
            break;
      }
   }
}

void US_FemGlobal_New::ModelSystem::get_associations( 
      QXmlStreamReader& xml, 
      US_FemGlobal_New::Association& as )
{
   while ( ! xml.atEnd() )
   {
      xml.readNext();

      if ( xml.isStartElement() )
      {
         if ( xml.name() == "component" )
         {
            QXmlStreamAttributes a = xml.attributes();

            as.reaction_components << a.value( "index"  ).toString().toInt();
            as.stoichiometry       << a.value( "stoich" ).toString().toInt();
         }
         else 
            break;
      }
   }

}


