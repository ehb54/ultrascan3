//! \file us_model.cpp

#include "us_model.h"
#include "us_constants.h"
#include "us_settings.h"
#include "us_util.h"
#include "us_math2.h"

US_Model::SimulationComponent::SimulationComponent()
{
   name                 = "New Component";
   analyteGUID.clear();
   molar_concentration  = 0.0;
   signal_concentration = 1.0;
   vbar20               = TYPICAL_VBAR;
   mw                   = 50000.0;
   s                    = 0.0;
   D                    = 0.0;
   f                    = 1.0;
   f_f0                 = 1.25;
   extinction           = 0.0;
   sigma                = 0.0;
   delta                = 0.0;
   oligomer             = 1;
   shape                = SPHERE;
   axial_ratio          = 10.0;
   analyte_type         = 0;  // Protein
   c0.radius       .clear();
   c0.concentration.clear();
}

bool US_Model::SimulationComponent::operator== 
                  ( const SimulationComponent& sc ) const
{
   if ( analyteGUID          != sc.analyteGUID           ) return false;
   if ( name                 != sc.name                  ) return false;
   if ( molar_concentration  != sc.molar_concentration   ) return false;
   if ( signal_concentration != sc.signal_concentration  ) return false;
   if ( vbar20               != sc.vbar20                ) return false;
   if ( mw                   != sc.mw                    ) return false;
   if ( s                    != sc.s                     ) return false;
   if ( D                    != sc.D                     ) return false;
   if ( f                    != sc.f                     ) return false;
   if ( f_f0                 != sc.f_f0                  ) return false;
   if ( extinction           != sc.extinction            ) return false;
   if ( sigma                != sc.sigma                 ) return false;
   if ( delta                != sc.delta                 ) return false;
   if ( oligomer             != sc.oligomer              ) return false;
   if ( shape                != sc.shape                 ) return false;
   if ( axial_ratio          != sc.axial_ratio           ) return false;
   if ( analyte_type         != sc.analyte_type          ) return false;

   if ( c0.radius            != sc.c0.radius             ) return false;
   if ( c0.concentration     != sc.c0.concentration      ) return false;

   return true;
}

US_Model::Association::Association()
{
   k_d       = 0.0;
   k_off     = 0.0;
   rcomps .clear();
   stoichs.clear();
}

bool US_Model::Association::operator== ( const Association& a ) const
{
   if ( k_d     != a.k_d     ) return false;
   if ( k_off   != a.k_off   ) return false;
   if ( rcomps  != a.rcomps  ) return false;
   if ( stoichs != a.stoichs ) return false;

   return true;
}

// Model constructor
US_Model::US_Model()
{
   monteCarlo      = false;
   wavelength      = 0.0;
   variance        = 0.0;
   meniscus        = 0.0;
   bottom          = 0.0;
   alphaRP         = 0.0;
   subGrids        = 0;
   description     = "New Model";
   optics          = ABSORBANCE;
   analysis        = MANUAL;
   global          = NONE;
   nmcixs          = 0;
   timeCreated     = "";
   editDataUpdated = "";
   
   coSedSolute     = -1;
   modelGUID   .clear();
   editGUID    .clear();
   requestGUID .clear();
   components  .clear();
   associations.clear();
   dataDescrip .clear();
   mcixmls     .clear();
}

// Equality operator
bool US_Model::operator== ( const US_Model& m ) const
{
   if ( monteCarlo      != m.monteCarlo      ) return false;
   if ( wavelength      != m.wavelength      ) return false;
   if ( variance        != m.variance        ) return false;
   if ( meniscus        != m.meniscus        ) return false;
   if ( bottom          != m.bottom          ) return false;
   if ( alphaRP         != m.alphaRP         ) return false;
   if ( modelGUID       != m.modelGUID       ) return false;
   if ( editGUID        != m.editGUID        ) return false;
   if ( requestGUID     != m.requestGUID     ) return false;
   if ( description     != m.description     ) return false;
   if ( optics          != m.optics          ) return false;
   if ( analysis        != m.analysis        ) return false;
   if ( global          != m.global          ) return false;
   if ( coSedSolute     != m.coSedSolute     ) return false;
   if ( subGrids        != m.subGrids        ) return false;
   if ( dataDescrip     != m.dataDescrip     ) return false;
   if ( associations.size() != m.associations.size() ) return false;

   for ( int i = 0; i < associations.size(); i++ )
      if ( associations[ i ] != m.associations[ i ] ) return false;

   if ( components.size() != m.components.size() ) return false;

   for ( int i = 0; i < components.size(); i++ )
      if ( components[ i ] != m.components[ i ] ) return false;

   return true;
}

// Update any missing coefficient values in the components of the model
bool US_Model::update_coefficients()
{
   bool ok = true;
   
   // Calculate missing coefficients for each component; note overall success
   for ( int ii = 0; ii < components.size(); ii++ )
      ok = ok && calc_coefficients( components[ ii ] );

   return ok;
}

// Calculate any missing coefficient values in a model component
bool US_Model::calc_coefficients( SimulationComponent& component )
{
   // For documentation for the derivation of the coefficient calculations 
   // please see: calc_coefficient_documentation.pdf in this directory

   bool   ok = true;
   double vbar;             // component vbar
   double volume;           // e.g., vbar * mw / AVOGADRO
   double vol_fac;          // volume factor, e.g., 0.75 / M_PI
   double radius_sphere;    // e.g., pow( volume * vol_fac, 1/3 );
   double rsph_fac;         // radius_sphere factor; e.g., 0.06 * PI * VISC
   double onethird;         // one third ( 1.0 / 3.0 )
   double c;                // concentration
   double s;                // sedimentation coefficient
   double D;                // diffusion coefficient
   double mw;               // molecular weight
   double f;                // frictional coefficient
   double fv;               // frictional coefficient (working value)
   double f_f0;             // frictional ratio
   double f0;               // f-zero
   double s20w;
   double buoyancyb;
// Ideal gas constant times 20 degrees C in Kelvin
#define RT (R_GC*K20)

   // Ensure that we have a vbar we can use
   vbar           = component.vbar20;

   if ( vbar <= 0.0 )
   {
      vbar           = TYPICAL_VBAR;
      component.vbar20 = vbar;
   }

   vol_fac        = 0.75 / M_PI;        // various factors used in calcs below
   onethird       = 1.0 / 3.0;
   rsph_fac       = 0.06 * M_PI * VISC_20W;

   s              = component.s;        // component coefficient values
   D              = component.D;
   mw             = component.mw;
   f              = component.f;
   f_f0           = component.f_f0;
   c              = component.signal_concentration;
   fv             = f;

   // Adjust buoyancy to be consistent with s-value. Negative s-values require negative buoyancy.
   // Unless custom grid is used, we set the vbar to the reciprocal of the positive s-value vbar.

   buoyancyb      = 1.0 - ( vbar * DENS_20W );

   if ( ( s < 0.0  &&  buoyancyb > 0.0 )  ||
       ( s > 0.0  &&  buoyancyb < 0.0 ) )
   {
      return false;
   }

   // Start with already calculated s if possible
   if ( s != 0.0 )
   {
      s20w           = s;

      // First check s and k (f_f0)
                                                 ///////////////
      if ( f_f0 != 0.0 )                         // s and f_f0
      {                                          ///////////////
         // Please see calc_coefficient_documentation.pdf in this directory,
         // equation 14, with adjustments for units in poise.
         double numer   = 0.02 * vbar * s * VISC_20W * f_f0;
         f0             = 0.09 * VISC_20W * M_PI * sqrt( numer / buoyancyb );
         fv             = f_f0 * f0;
         D              = RT / ( AVOGADRO * fv );
         mw             = s * RT / ( D * buoyancyb );
      }

      // Next check s and D
                                                 ///////////////
      else if ( D != 0.0 )                       // s and D
      {                                          ///////////////
         mw             = s * RT / ( D * buoyancyb );
         volume         = vbar * mw / AVOGADRO;
         radius_sphere  = pow( volume * vol_fac, onethird );
         f0             = radius_sphere * rsph_fac;
         fv             = mw * buoyancyb / ( s20w * AVOGADRO );
         double ff0sv   = f_f0;
         f_f0           = fv / f0;
         double ffdif   = qAbs( ff0sv - f_f0 );
         f_f0           = ( ffdif < 1.e-5 ) ? ff0sv : f_f0;
      }

      // Then check any other s + combinations
                                                 ///////////////
      else if ( mw != 0.0 )                      // s and mw
      {                                          ///////////////
         D              = s * RT / ( buoyancyb * mw );
         fv             = mw * buoyancyb / ( s20w * AVOGADRO );
         volume         = vbar * mw / AVOGADRO;
         radius_sphere  = pow( volume * vol_fac, onethird );
         f0             = radius_sphere * rsph_fac;
         f_f0           = fv / f0;
      }
                                                 ///////////////
      else if ( f != 0.0 )                       // s and f
      {                                          ///////////////
         D              = RT / ( AVOGADRO * fv );
         mw             = s * RT / ( D * buoyancyb );
         volume         = vbar * mw / AVOGADRO;
         radius_sphere  = pow( volume * vol_fac, onethird );
         f0             = radius_sphere * rsph_fac;
         f_f0           = fv / f0;
      }
                                                 //****************************
      else                                       // do not have 2 valid coeffs
         ok             = false;                 //****************************
   }

   else if ( component.mw   != 0.0 )
   {
      volume         = vbar * mw / AVOGADRO;
      radius_sphere  = pow( volume * vol_fac, onethird );
      f0             = radius_sphere * rsph_fac;
                                                 ///////////////
      if ( D != 0.0 )                            // mw and D
      {                                          ///////////////
         s              = D * buoyancyb * mw / RT;
         fv             = mw * buoyancyb / ( s * AVOGADRO );
         f_f0           = fv / f0;
      }
                                                 ///////////////
      else if ( f_f0 != 0.0 )                    // mw and f_f0
      {                                          ///////////////
         fv             = f_f0 * f0;
         s              = mw * buoyancyb / ( AVOGADRO * fv );
         D              = s * RT / ( buoyancyb * mw );
      }
                                                 ///////////////
      else if ( f != 0.0 )                       // mw and f
      {                                          ///////////////
         f_f0           = fv / f0;
         s              = mw * buoyancyb / ( AVOGADRO * fv );
         D              = s * RT / ( buoyancyb * mw );
      }
                                                 //****************************
      else                                       // do not have 2 valid coeffs
         ok             = false;                 //****************************
   }

   else if ( component.D    != 0.0 )
   {                                             ///////////////
      if ( f_f0 >= 1.0 )                         // D and f_f0
      {                                          ///////////////
         fv             = RT / ( AVOGADRO * D );
         f0             = fv / f_f0;
         radius_sphere  = f0 / ( 0.06 * M_PI * VISC_20W );
         double volume  = ( 4.0 / 3.0 ) * M_PI * pow( radius_sphere, 3.0 );
         mw             = volume * AVOGADRO / vbar;
         s              = mw * buoyancyb / ( AVOGADRO * fv );
      }
                                                 //****************************
                                                 // D and f - not a valid combo
                                                 //****************************
      else                                       // do not have 2 valid coeffs
         ok             = false;                 //****************************
   }
                                                 ///////////////
   else if ( fv > 0.0  &&  f_f0 >= 1.0 )         // f and f_f0
   {                                             ///////////////
      f0             = fv / f_f0;
      D              = RT / ( AVOGADRO * fv );
      radius_sphere  = f0 / ( 0.06 * M_PI * VISC_20W );
      double volume  = ( 4.0 / 3.0 ) * M_PI * pow( radius_sphere, 3.0 );
      mw             = volume * AVOGADRO / vbar;
      s              = mw * buoyancyb / ( AVOGADRO * fv );
   }
                                                 //****************************
   else                                          // do not have 2 valid coeffs
      ok             = false;                    //****************************

   double df      = qAbs( f - fv ) / fv;

   if ( df > 0.1 )
   {  // Significant change in f:  replace and use old as concentration

      if ( c == 0.0 )
         component.signal_concentration = f;

      f              = fv;
   }

   component.s    = s;     // set component properties (some re-calculated)
   component.D    = D;
   component.mw   = mw;
   component.f_f0 = f_f0;
   component.f    = f;
   component.vbar20 = vbar;

   return ok;
}

// Test the existence of the models directory path and create it if need be
bool US_Model::model_path( QString& path, bool is_perm )
{
#ifdef NO_DB
   path = is_perm ? "" : "";
#else
   QDir dir;
   path = is_perm ? US_Settings::dataDir() + "/models"
                  : US_Settings::tmpDir() + "/temp_models";

   if ( ! dir.exists( path ) )
   {
      if ( ! dir.mkpath( path ) )
      {
         return false;
      }
   }
#endif

   return true;
}

// Short text string describing the model type
QString US_Model::typeText( int subtype )
{
   struct typemap
   {
      AnalysisType typeval;
      QString      typedesc;
   };

   const typemap tmap[] =
   {
      { MANUAL,      QObject::tr( "Manual"      ) },
      { TWODSA,      QObject::tr( "2DSA"        ) },
      { TWODSA_MW,   QObject::tr( "2DSA-MW"     ) },
      { GA,          QObject::tr( "GA"          ) },
      { GA_MW,       QObject::tr( "GA-MW"       ) },
      { COFS,        QObject::tr( "COFS"        ) },
      { FE,          QObject::tr( "FE"          ) },
      { PCSA,        QObject::tr( "PCSA"        ) },
      { CUSTOMGRID,  QObject::tr( "CUSTOMGRID"  ) },
      { DMGA,        QObject::tr( "DMGA"        ) },
      { DMGA_CONSTR, QObject::tr( "DMGA_CONSTR" ) }
   };

   const int ntmap = sizeof( tmap ) / sizeof( tmap[ 0 ] );

   QString tdesc = tmap[ 0 ].typedesc;

   for ( int jj = 0; jj < ntmap; jj++ )
   {  // look for model type match

      if ( analysis == tmap[ jj ].typeval )
      {  // we have a match:  build type description
         tdesc    = tmap[ jj ].typedesc;    // set basic model analyis type

         if ( associations.size() > 0 )     // Reversible Associations subtype
            tdesc    = tdesc + "-RA";

         //if ( alphaRP != 0.0 )
         //   tdesc    = tdesc + "-TR";       // Tikhonov Regularization subtype

         if ( global == MENISCUS )          // Fit Meniscus subtype
            tdesc    = tdesc + "-FM";

         else if ( global == BOTTOM )       // Fit Bottom subtype
            tdesc    = tdesc + "-FB";

         else if ( global == MENIBOTT )     // Fit Meniscus+Bottom subtype
            tdesc    = tdesc + "-FMB";

         else if ( global == GLOBAL )       // Global subtype
         {
            tdesc    = ( jj > 0 ) ? tdesc + "-GL" : "Global";
            if (      subtype_match( subtype, SUBT_SC ) )
               tdesc    = tdesc + "-SC";
            else if ( subtype_match( subtype, SUBT_VR ) )
               tdesc    = tdesc + "-VR";
         }

         else if ( global == SUPERGLOBAL )  // SuperGlobal subtype
            tdesc    = ( jj > 0 ) ? tdesc + "-GL-SG" : "SuperGlobal";

         if ( analysis == PCSA )            // Add sub-type (SL,IS,DS,HL) to PCSA
         {
            if ( subtype > 0 )
            {  // Append sub-type based on a given flag
               if (      subtype_match( subtype, SUBT_SL ) )
                  tdesc    = tdesc + "-SL";
               else if ( subtype_match( subtype, SUBT_IS ) )
                  tdesc    = tdesc + "-IS";
               else if ( subtype_match( subtype, SUBT_DS ) )
                  tdesc    = tdesc + "-DS";
               else if ( subtype_match( subtype, SUBT_HL ) )
                  tdesc    = tdesc + "-HL";
               else if ( subtype_match( subtype, SUBT_2O ) )
                  tdesc    = tdesc + "-2O";
               else
                  tdesc    = tdesc + "-IS";
            }

            else
            {  // Append sub-type based on already-created description
               int kk   = description.indexOf( "_PCSA" );
               if ( kk > 0 )  // Grab type string from within model description
               {
                  kk++;
                  int mm   = description.indexOf( "_", kk ) - kk;
                  tdesc    = description.mid( kk, mm );

                  // Insure '-MC'/'-TR' won't be repeated
                  if ( monteCarlo )
                     tdesc    = tdesc.replace( "-MC", "" );
                  if ( alphaRP != 0.0 )
                     tdesc    = tdesc.replace( "-TR", "" );
               }
               else           // By default, assume "-IS" gets appended
                  tdesc    = tdesc + "-IS";
            }
         }

         if ( monteCarlo )                  // Monte Carlo subtype
            tdesc    = tdesc + "-MC";

         if ( alphaRP != 0.0 )
            tdesc    = tdesc + "-TR";       // Tikhonov Regularization subtype

         break;
      }
   }

   return tdesc;                            // return type description text
}

// Flag whether model component frictional ratios are constant
bool US_Model::constant_ff0( void )
{
   double valmin = components[ 0 ].f_f0;
   double valmax = components[ 0 ].f_f0;

   for ( int ii = 1; ii < components.size(); ii++ )
   {
      valmin = qMin( valmin, components[ ii ].f_f0 );
      valmax = qMax( valmax, components[ ii ].f_f0 );
   }

   return ( ( valmax - valmin ) < 1.0e-3 );
}

// Flag whether model component vbar values are constant
bool US_Model::constant_vbar( void )
{
   double valmin = components[ 0 ].vbar20;
   double valmax = components[ 0 ].vbar20;

   for ( int ii = 1; ii < components.size(); ii++ )
   {
      valmin = qMin( valmin, components[ ii ].vbar20 );
      valmax = qMax( valmax, components[ ii ].vbar20 );
   }

   return ( ( valmax - valmin ) < 1.0e-4 );
}

// Flag whether a model component is a reactant
bool US_Model::is_reactant( const int compx )
{
   bool is_react  = false;

   for ( int ii = 0; ii < associations.size(); ii++ )
   {
      Association* as   = &associations[ ii ];
      int          rcx  = as->rcomps.indexOf( compx );

      if ( rcx >= 0   &&  as->stoichs[ rcx ] >= 0 )
      {
         is_react       = true;
         break;
      }
   }

   return is_react;
}

// Flag whether a model component is a product of a reaction
bool US_Model::is_product( const int compx )
{
   bool is_prod   = false;

   for ( int ii = 0; ii < associations.size(); ii++ )
   {
      Association* as   = &associations[ ii ];
      int          rcx  = as->rcomps.indexOf( compx );

      if ( rcx >= 0   &&  as->stoichs[ rcx ] < 0 )
      {
         is_prod        = true;
         break;
      }
   }

   return is_prod;
}

// Load a model from DB or local file
int US_Model::load( bool db_access, const QString& guid, US_DB2* db )
{
   if ( db_access ) return load_db  ( guid, db );
   else             return load_disk( guid );
}

// Load model from local disk
int US_Model::load_disk( const QString& guid )
{
   int error = US_DB2::DBERROR;  // Error by default

   QString path;

   if ( ! model_path( path ) )
   {
      message = QObject::tr ( "Could not create model directory" );
      return error;
   }

   QDir        f( path );
   QStringList filter( "M*.xml" );
   QStringList names = f.entryList( filter, QDir::Files, QDir::Name );
   QString     filename;
   bool        found = false;

   for ( int i = 0; i < names.size(); i++ )
   {
      filename = path + "/" + names[ i ];
      QFile file( filename );

      if ( ! file.open( QIODevice::ReadOnly | QIODevice::Text) ) continue;

      QXmlStreamReader xml( &file );

      while ( ! xml.atEnd() )
      {
         xml.readNext();

         if ( xml.isStartElement() )
         {
            if ( xml.name() == "model" )
            {
               QXmlStreamAttributes a = xml.attributes();

               if ( a.value( "modelGUID" ).toString() == guid )
                  found = true;
               break;
            }
         }
      }

      file.close();

      if ( found ) return load( filename );
   }

   message =  QObject::tr ( "Could not find analyte guid" );
   return error;
}

// Load a model from a local file
int US_Model::load( const QString& filename )
{
   QFile file( filename );

   if ( ! file.open( QIODevice::ReadOnly | QIODevice::Text) )
      return US_DB2::DBERROR;

   QXmlStreamReader xml( &file );
   
   int result = load_stream( xml );

   if ( result == US_DB2::NO_MODEL  &&  monteCarlo )
   {  // Handle a multi-model stream
      file.close();
      file.open( QIODevice::ReadOnly | QIODevice::Text );

      QTextStream tsi( &file );

      result     = load_multi_model( tsi );
   }

   file.close();
   return result;
}

// Load a model from an XML string
int US_Model::load_string( const QString& mcont )
{
   QXmlStreamReader xml( mcont );
   int result = load_stream( xml );
   return result;
}

// Load a model from an XML stream
int US_Model::load_stream( QXmlStreamReader& xml )
{
   QString coSedStr;
   QString comprStr;

   components  .clear();
   associations.clear();

   QXmlStreamAttributes a;
   bool read_next = true;
   int  nmtag     = 0;

   while ( ! xml.atEnd() )
   {
      if ( read_next ) xml.readNext();
      read_next = true;

      if ( xml.isStartElement() )
      {
         if ( xml.name() == "model" )
         {
            nmtag++;

            if ( nmtag > 1 )
            {  // A second model tag:  return to handle multi-model stream
               monteCarlo     = true;
               return US_DB2::NO_MODEL;
            }

            a = xml.attributes();

            QString mcst    = a.value( "monteCarlo"     ).toString();
            monteCarlo      = ( ! mcst.isEmpty()  &&  mcst != "0" );
            QString iter    = a.value( "iterations"     ).toString();
            monteCarlo      = monteCarlo ? monteCarlo :
                              ( iter.isEmpty() ? false : ( iter.toInt() > 1 ) );
            wavelength      = a.value( "wavelength"     ).toString().toDouble();
            QString vari    = a.value( "variance"       ).toString();
            variance        = vari.isEmpty() ? 0.0 : vari.toDouble();
            QString meni    = a.value( "meniscus"       ).toString();
            meniscus        = meni.isEmpty() ? 0.0 : meni.toDouble();
            QString bott    = a.value( "bottom"         ).toString();
            bottom          = bott.isEmpty() ? 0.0 : bott.toDouble();
            QString alph    = a.value( "alphaRP"        ).toString();
            alphaRP         = alph.isEmpty() ? 0.0 : alph.toDouble();
            description     = a.value( "description"    ).toString();
            modelGUID       = a.value( "modelGUID"      ).toString();
            editGUID        = a.value( "editGUID"       ).toString();
            requestGUID     = a.value( "requestGUID"    ).toString();
            dataDescrip     = a.value( "dataDescrip"    ).toString();
            coSedStr        = a.value( "coSedSolute"    ).toString();
            coSedSolute     = ( coSedStr.isEmpty() ) ? -1 : coSedStr.toInt();
            QString subgs   = a.value( "subGrids"       ).toString();
            subGrids        = subgs.isEmpty() ? subGrids
                                              : subgs.toInt();
            QString anal1   = a.value( "type"           ).toString();
            QString anal2   = a.value( "analysisType"   ).toString();
            analysis        = anal1.isEmpty() ? analysis
                                              : (AnalysisType)anal1.toInt();
            analysis        = anal2.isEmpty() ? analysis
                                              : (AnalysisType)anal2.toInt();
            global          =
                (GlobalType)  a.value( "globalType"   ).toString().toInt();
            optics          =
                (OpticsType)  a.value( "opticsType"   ).toString().toInt();
         }

         else if ( xml.name() == "analyte" )
         {
            SimulationComponent sc;
            a = xml.attributes();

            sc.analyteGUID  = a.value( "analyteGUID" ).toString();

            sc.name         = a.value( "name"       ).toString();
            QString avbar   = a.value( "vbar20"     ).toString();
            sc.vbar20       = avbar.isEmpty() ? TYPICAL_VBAR : avbar.toDouble();
            sc.mw           = a.value( "mw"         ).toString().toDouble();
            sc.s            = a.value( "s"          ).toString().toDouble();
            sc.D            = a.value( "D"          ).toString().toDouble();
            sc.f            = a.value( "f"          ).toString().toDouble();
            sc.f_f0         = a.value( "f_f0"       ).toString().toDouble();
            sc.extinction   = a.value( "extinction" ).toString().toDouble();
            QString aaxia   = a.value( "axial"      ).toString();
            sc.axial_ratio  = aaxia.isEmpty() ? 10.0 : aaxia.toDouble();
            sc.sigma        = a.value( "sigma"      ).toString().toDouble();
            sc.delta        = a.value( "delta"      ).toString().toDouble();

            sc.molar_concentration  = a.value( "molar"  ).toString().toDouble();
            sc.signal_concentration = a.value( "signal" ).toString().toDouble();
            sc.oligomer     = a.value( "oligomer"   ).toString().toInt();

            if ( sc.oligomer < 1 )
            {
               sc.oligomer     = a.value( "stoich"     ).toString().toInt();
               sc.oligomer     = ( sc.oligomer > 0 ) ? sc.oligomer : 1;
            }

            sc.shape        =
                   (ShapeType)a.value( "shape"      ).toString().toInt();
            sc.analyte_type = a.value( "type"       ).toString().toInt();

            mfem_scans( xml, sc );

            read_next = false; // Skip the next read

            components << sc;
         }

         else if ( xml.name() == "association" )
         {
            Association as;
            get_associations( xml, as );

            associations << as;

            read_next = false; // Skip the next read
         }
      }
   }

   if ( US_Settings::us_debug() > 2 ) debug();

   return US_DB2::OK;
}

// Load from a multiple-model stream and create an MC composite model
int US_Model::load_multi_model( QTextStream& tsi )
{
   int result    = US_DB2::OK;
   QString mline, mdesc, mcont;
   nmcixs        = 0;
   mcixmls.clear();

   // Read and save the first three XML lines
   QString line1 = tsi.readLine() + "\n";
   QString line2 = tsi.readLine() + "\n";
   QString line3 = tsi.readLine() + "\n";

   // Read remaining lines and save description,contents of all models
   while ( ! tsi.atEnd() )
   {
      mline         = tsi.readLine();
      if ( mline.contains( "</ModelData>" ) )
         break;

      if ( mline.contains( "<model " ) )
      {  // At model tag, create initial contents
         mcont         = line1 + line2 + line3 + mline + "\n";
         // Parse description and save it, if first iteration
         if ( nmcixs == 0 )
         {
            int idx       = qMax( mline.indexOf( "description=" ), 0 );
            mdesc         = QString( mline ).mid( idx, 200 ).section( "\"", 1, 1 );
         }
      }

      else if ( mline.contains( "</model>" ) )
      {  // At end of model section, save a model content and bump count
         mcont         = mcont + mline + "\n</ModelData>\n";
         mcixmls << mcont;
         nmcixs++;
      }

      else
      {  // For any other type of line, add it to contents;
         mcont         = mcont + mline + "\n";
      }
   }

   QVector< SimulationComponent >  mmcomps;
   QVector< Association >          mmassos;
   int ncnstv    = 0;
   int ncnstk    = 0;

   // Build composite components and associations
   for ( int ii = 0; ii < nmcixs; ii++ )
   {
      mcont            = mcixmls[ ii ];
      QXmlStreamReader xml( mcont );
      load_stream( xml );

      mmcomps << components;
      mmassos << associations;

      if ( constant_vbar() )  ncnstv++;
      if ( constant_ff0()  )  ncnstk++;
   }

   // Compress and scale components to only unique solute points

   components  .clear();
   associations.clear();
   double sclnrm = 1.0 / (double)nmcixs;    // Scale for concentrations
   bool cnst_vb  = ( ncnstv >= ncnstk );    // Flag for constant vbar
   QStringList sklist;                      // List of all solute points
   QStringList skvals;                      // List of unique solute points

   for ( int ii = 0; ii < mmcomps.size(); ii++ )
   {  // Build list of solute point strings and list of unique ones
      double sval   = mmcomps[ ii ].s * 1.0e+13;
      double kval   = cnst_vb ? mmcomps[ ii ].f_f0 : mmcomps[ ii ].vbar20;
      QString skval = QString().sprintf( "%10.4f %8.5f", sval, kval );
      sklist << skval; 
      if ( ! skvals.contains( skval ) )
         skvals << skval;
   }

   int nskl      = sklist.size();
   int nskv      = skvals.size();
   skvals.sort();                     // Sort solute points
   SimulationComponent scomp;

   for ( int ii = 0; ii < nskv; ii ++ )
   {  // Average concentration at each unique solute point
      QString skval = skvals[ ii ];   // Identifying solute point string
      double conc   = 0.0;

      for ( int jj = 0; jj < nskl; jj++ )
      {  // Search all solute points
         if ( skval == sklist[ jj ] )
         {  // If a match, sum the concentration
            scomp      = mmcomps[ jj ];
            conc      += scomp.signal_concentration;
         }
      }

      scomp.name                 = QString().sprintf( "SC%04d", ii + 1 );
      scomp.signal_concentration = conc * sclnrm;
      components << scomp;
   }

   if ( mmassos.size() > 0 )
      associations << mmassos[ 0 ];

   QString mdsc1 = QString( mdesc ).section( ".",  0, -3 );
   QString mdsc2 = QString( mdesc ).section( ".", -2, -2 )
                                   .section( "_",  0, -2 );
   QString mdsc3 = QString( mdesc ).section( ".", -1, -1 );
   QString miter = QString().sprintf( "_mcN%03i", nmcixs );
   description   = mdsc1 + "." + mdsc2 + miter + "." + mdsc3;
qDebug() << "MDL:LMM: miter" << miter << "desc" << description << mdesc;

   return result;
}

// Write a multiple-model stream
void US_Model::write_mm_stream( QTextStream& tso )
{
   if ( ! monteCarlo  ||  nmcixs < 1 )     // Do nothing if no MC iterations
      return;

   // Build an output stream from MC iteration input streams
   for ( int ii = 0; ii < nmcixs; ii++ )
   {
      QString mlines = mcixmls[ ii ];

      // Limit contents to <model>...</model> except for first and last
      int flx        = ( ii == 0 ) ? 0 : 3;
      int llx        = ( ( ii + 1 ) < nmcixs ) ? -3 : -2;
      QString mcont  = mlines.section( "\n", flx, llx ) + "\n";
      // Concatenate to output stream
      tso << mcont;
   }
}

// Read scan C0 values from an XML stream
void US_Model::mfem_scans( QXmlStreamReader& xml, SimulationComponent& sc )
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

// Get associations from an XML stream
void US_Model::get_associations( QXmlStreamReader& xml, Association& as )
{
   QXmlStreamAttributes a = xml.attributes();
   QString skassoc = a.value( "k_assoc" ).toString();
   QString skeq    = a.value( "k_eq"    ).toString();
   QString skdisso = a.value( "K_d"     ).toString();
   QString skoff   = a.value( "k_off"   ).toString();

   if ( ! skdisso.isEmpty() )
      as.k_d          = skdisso.toDouble();
   else
   {
      double k_assoc  = 0.0;
      if ( ! skassoc.isEmpty() )
         k_assoc         = skassoc.toDouble();
      else if ( ! skeq.isEmpty() )
         k_assoc         = skeq.toDouble();
      as.k_d          = ( k_assoc != 0.0 ) ? ( 1.0 / k_assoc ) : 0.0;
   }

   if ( ! skoff.isEmpty() )
      as.k_off        = skoff.toDouble();

   while ( ! xml.atEnd() )
   {
      xml.readNext();

      if ( xml.isStartElement() )
      {
         if ( xml.name() == "component" )
         {
            a = xml.attributes();

            as.rcomps  << a.value( "index"  ).toString().toInt();
            as.stoichs << a.value( "stoich" ).toString().toInt();
         }
         else 
            break;
      }
   }
}

// Load a model from DB (by GUID)
int US_Model::load_db( const QString& guid, US_DB2* db )
{
   QStringList q;

   q << "get_modelID" << guid;
   db->query( q );

   if ( db->lastErrno() != US_DB2::OK ) return db->lastErrno();
   
   db->next();
   QString id = db->value( 0 ).toString();
   return load( id, db );
}

// Load a model from DB (by DB id)
int US_Model::load( const QString& id, US_DB2* db )
{
   QStringList q;

   q << "get_model_info" << id;
   db->query( q );
   
   if ( db->lastErrno() != US_DB2::OK ) return db->lastErrno();

   db->next();
   QByteArray contents  = db->value( 2 ).toString().toLatin1();
   timeCreated = db->value( 6 ).toString();

   // Read the model file into an array in memory
   QXmlStreamReader xml( contents );
   
   int result = load_stream( xml );

   if ( result == US_DB2::NO_MODEL  &&  monteCarlo )
   {  // Handle a multi-model stream
      QTextStream tsi( contents );

      result     = load_multi_model( tsi );
   }

   return result;
}

// Write a model to DB or local file
int US_Model::write( bool db_access, const QString& filename, US_DB2* db )
{
   if ( db_access ) return write( db );
   else             return write( filename );
}

// Write a model DB record
int US_Model::write( US_DB2* db )
{
   // Create the model xml file in a stream
   QByteArray temporary;
   QByteArray contents;

   if ( ! monteCarlo  ||  nmcixs < 1 )
   {
      QXmlStreamWriter xml( &temporary );
      write_stream( xml );
   }

   else
   {
      QTextStream tso( &temporary );
      write_mm_stream( tso );
   }

   db->mysqlEscapeString( contents, temporary, temporary.size() );
qDebug() << "model writedb contsize tempsize" << contents.size() << temporary.size();

   QStringList q;

   // Generate a guid if necessary
   // The guid may be valid from a disk read, but is not in the DB
   if ( modelGUID.size() != 36 ) modelGUID = US_Util::new_guid();

   q << "get_modelID" << modelGUID;
      
   db->query( q );
    
   QString meni = QString::number( meniscus );
   QString vari = QString::number( variance );
     
   if ( db->lastErrno() != US_DB2::OK )
   {
      q.clear();
      q << "new_model" << modelGUID << description << contents
        << vari << meni << editGUID
        << QString::number( US_Settings::us_inv_ID() );
      message = QObject::tr( "created" );
   }
   else
   {
      db->next();
      QString id = db->value( 0 ).toString();
      q.clear();
      q << "update_model" << id << description << contents
        << vari << meni << editGUID;
      message = QObject::tr( "updated" );
   }

   int wstat = db->statusQuery( q );
qDebug() << "model writedb message" << message << "wstat" << wstat;
   QString path;
   model_path( path );
   bool newFile;
   QString filename = get_filename( path, modelGUID, newFile );
   write( filename );

   return wstat;
}

// Write a model file
int US_Model::write( const QString& filename )
{
   QFile file( filename );

   if ( ! file.open( QIODevice::WriteOnly | QIODevice::Text) )
      return US_DB2::DBERROR;

   if ( ! monteCarlo  ||  nmcixs < 1 )
   {
      QXmlStreamWriter xml( &file );
      write_stream( xml );
   }

   else
   {
      QTextStream tso( &file );
      write_mm_stream( tso );
   }

   file.close();

   return US_DB2::OK;
}

// Write to an XML stream
void US_Model::write_stream( QXmlStreamWriter& xml )
{
   if ( modelGUID.size() != 36 )
      modelGUID = US_Util::new_guid();

   xml.setAutoFormatting( true );

   xml.writeStartDocument();
   xml.writeDTD         ( "<!DOCTYPE US_Model>" );
   xml.writeStartElement( "ModelData" );
   xml.writeAttribute   ( "version", "1.0" );

   xml.writeStartElement( "model" );
   xml.writeAttribute   ( "description", description );
   xml.writeAttribute   ( "modelGUID",   modelGUID   );
   xml.writeAttribute   ( "editGUID",    editGUID    );
   xml.writeAttribute   ( "wavelength",  QString::number( wavelength   ) );
   if ( variance != 0.0 )
      xml.writeAttribute( "variance",    QString::number( variance     ) );
   if ( meniscus != 0.0 )
      xml.writeAttribute( "meniscus",    QString::number( meniscus     ) );
   if ( bottom   != 0.0 )
      xml.writeAttribute( "bottom",      QString::number( bottom       ) );
   if ( alphaRP  != 0.0 )
      xml.writeAttribute( "alphaRP",     QString::number( alphaRP      ) );
   xml.writeAttribute   ( "coSedSolute", QString::number( coSedSolute  ) );
   if ( subGrids != 0.0 )
      xml.writeAttribute( "subGrids",    QString::number( subGrids     ) );
   xml.writeAttribute   ( "opticsType",  QString::number( optics       ) );
   xml.writeAttribute   ( "analysisType",QString::number( analysis     ) );
   xml.writeAttribute   ( "globalType",  QString::number( global       ) );

   if ( requestGUID.length() > 0 )
      xml.writeAttribute   ( "requestGUID", requestGUID                     );

   if ( monteCarlo )
      xml.writeAttribute   ( "monteCarlo",  "1"                             );

   if ( ! dataDescrip.isEmpty() )
   {
      dataDescrip.replace( '"', "&quot;" ); // Replace '"' character
      dataDescrip.replace( '<', "&lt;" );   // Replace '<' character
      dataDescrip.replace( '>', "&gt;" );   // Replace '>' character
      xml.writeAttribute   ( "dataDescrip", dataDescrip                     );
   }

   // Write components
   int  ncomps  = components.size();
   bool notmany = ( ncomps < 400 );

   for ( int i = 0; i < ncomps; i++ )
   {
      SimulationComponent* sc = &components[ i ];
      xml.writeStartElement( "analyte" );

      if ( !sc->analyteGUID.isEmpty() )
         xml.writeAttribute( "analyteGUID", sc->analyteGUID            );
      xml.writeAttribute( "name",       sc->name                    );
      xml.writeAttribute( "mw",         QString::number( sc->mw   ) );
      xml.writeAttribute( "s",          QString::number( sc->s    ) );
      xml.writeAttribute( "D",          QString::number( sc->D    ) );
      xml.writeAttribute( "f",          QString::number( sc->f    ) );
      xml.writeAttribute( "f_f0",       QString::number( sc->f_f0 ) );
      QString strVbar   = QString::number( sc->vbar20       );
      QString strExtinc = QString::number( sc->extinction   );
      QString strAxial  = QString::number( sc->axial_ratio  );
      QString strSigma  = QString::number( sc->sigma        );
      QString strDelta  = QString::number( sc->delta        );
      QString strOligo  = QString::number( sc->oligomer     );
      QString strShape  = QString::number( sc->shape        );
      QString strType   = QString::number( sc->analyte_type );
      QString strMolar  = QString::number( sc->molar_concentration  );
      QString strSignal = QString::number( sc->signal_concentration );
      if ( notmany )
      {  // In most cases, don't test values to write
         xml.writeAttribute( "vbar20",     strVbar   );
         xml.writeAttribute( "extinction", strExtinc );
         xml.writeAttribute( "axial",      strAxial  );
         xml.writeAttribute( "sigma",      strSigma  );
         xml.writeAttribute( "delta",      strDelta  );
         xml.writeAttribute( "oligomer",   strOligo  );
         xml.writeAttribute( "shape",      strShape  );
         xml.writeAttribute( "type",       strType   );
         xml.writeAttribute( "molar",      strMolar  );
      }
      else
      {  // If many components, only write non-default values
         if ( sc->vbar20       != TYPICAL_VBAR )
            xml.writeAttribute( "vbar20",     strVbar   );
         if ( sc->extinction   != 0.0 )
            xml.writeAttribute( "extinction", strExtinc );
         if ( sc->axial_ratio  != 10.0 )
            xml.writeAttribute( "axial",      strAxial  );
         if ( sc->sigma        != 0.0 )
            xml.writeAttribute( "sigma",      strSigma  );
         if ( sc->delta        != 0.0 )
            xml.writeAttribute( "delta",      strDelta  );
         if ( sc->oligomer     != 1 )
            xml.writeAttribute( "oligomer",   strOligo  );
         if ( sc->shape        != SPHERE )
            xml.writeAttribute( "shape",      strShape  );
         if ( sc->analyte_type != 0 )
            xml.writeAttribute( "type",       strType   );
         if ( sc->molar_concentration != 0.0 )
            xml.writeAttribute( "molar",      strMolar  );
      }
      xml.writeAttribute( "signal",     strSignal );

      for ( int j = 0; j < sc->c0.radius.size(); j++ )
      {
         xml.writeStartElement( "mfem_scan" );

         MfemInitial* scan = &sc->c0;
         xml.writeAttribute( "radius",
            QString::number( scan->radius       [ j ] ) );
         xml.writeAttribute( "conc",
            QString::number( scan->concentration[ j ] ) );
         xml.writeEndElement();  // mfem_scan
      }

      xml.writeEndElement(); // analyte (SimulationComponent)
   }

   // Write associations
   for ( int i = 0; i < associations.size(); i++ )
   {
      Association* as = &associations[ i ];
      xml.writeStartElement( "association" );
      xml.writeAttribute( "K_d",     QString::number( as->k_d   ) );
      xml.writeAttribute( "k_off",   QString::number( as->k_off ) );

      for ( int j = 0; j < as->rcomps.size(); j++ )
      {
         xml.writeStartElement( "component" );

         QString index  = QString::number( as->rcomps [ j ] );
         QString stoich = QString::number( as->stoichs[ j ] );

         xml.writeAttribute( "index",  index  );
         xml.writeAttribute( "stoich", stoich );
         xml.writeEndElement(); // component
      }

      xml.writeEndElement(); // association
   }

   xml.writeEndElement(); // model
   xml.writeEndElement(); // ModelData
   xml.writeEndDocument();
}

// Get the name of a model file with matching GUID (if any)
QString US_Model::get_filename( const QString& path, const QString& guid,
                                bool& newFile )
{
   QDir f( path );
   QStringList filter( "M???????.xml" );
   QStringList f_names = f.entryList( filter, QDir::Files, QDir::Name );
   QString fnamo;
   newFile   = true;

   for ( int i = 0; i < f_names.size(); i++ )
   {
      QFile m_file( path + "/" + f_names[ i ] );

      if ( ! m_file.open( QIODevice::ReadOnly | QIODevice::Text) ) continue;

      QXmlStreamReader xml( &m_file );

      while ( ! xml.atEnd() )
      {
         xml.readNext();

         if ( xml.isStartElement() )
         {
            if ( xml.name() == "model" )
            {
               QXmlStreamAttributes a = xml.attributes();

               if ( a.value( "modelGUID" ).toString() == guid )
               {  // Break when we have found a file with a matching GUID
                  fnamo   = path + "/" + f_names[ i ];
                  newFile = false;
                  break;
               }
            }
         }
      }

      m_file.close();
   }

   if ( newFile )
   {  // No matching-GUID file, so look for a break in the number sequence
      int number = ( f_names.size() > 0 ) ?       // Last used file number
         f_names.last().mid( 1, 7 ).toInt() : 0;  // and default last sequence

      for ( int ii = 0; ii < number; ii++ )
      {
         QString fnamck = "M" + QString().sprintf( "%07i", ii + 1 ) + ".xml";

         if ( ! f_names.contains( fnamck ) )
         {  // There is a hole in the sequence, so re-use this number
            number = ii;
            break;
         }
      }

      // File name uses a break in the sequence or one past last used.
      fnamo = path + "/M" + QString().sprintf( "%07i", number + 1 ) + ".xml";
   }

   return fnamo;
}

// Create or append to a composite MC model file for a single triple
QString US_Model::composite_mc_file( QStringList& mcfiles, const bool rmvi )
{
   const QChar dquo( '"' );
   QString empty_str( "" );
   QString cmfname  = empty_str;
   int mc_iters     = mcfiles.size();
   int mc_comps     = 0;

   // Return an empty name if the list is empty
   if ( mc_iters < 1 )                  return cmfname;

   cmfname          = mcfiles[ 0 ];
   bool name_desc   = cmfname.contains( ".mc" ) || cmfname.contains( "_mc" );
//qDebug() << "MDL:CMF: name_desc" << name_desc;

   // Return the name of an existing composite if it is all of the list
   if ( mc_iters == 1 )
   {
      if ( cmfname.contains( ".mcN" )  ||  cmfname.contains( "_mcN" ) )
                                        return cmfname;
      if ( ! name_desc )
      {  // If no ".mc" in file name, must check description in contents
         QFile filei( cmfname );
         if ( ! filei.open( QIODevice::ReadOnly | QIODevice::Text) )
         {
            qDebug() << "**MC iteration file open error**";
                                        return empty_str;
         }

         QTextStream tsi( &filei );
         QString mcont   = tsi.readAll();
         filei.close();
         int jj          = qMax( 0, mcont.indexOf( "description=" ) );
         QString mdesc   = QString( mcont ).mid( jj, 99 ).section( dquo, 1, 1 );
         if ( mdesc.contains( "_mcN" ) )
                                        return cmfname;
      }
   }

   // Otherwise, scan the MC iteration file names
   for ( int ii = 0; ii < mcfiles.size(); ii++ )
   {
      if ( name_desc )
      {  // Handle a file whose name tells that it is a composite
         if ( mcfiles[ ii ].contains( ".mcN" )  ||
              mcfiles[ ii ].contains( "_mcN" ) )
         {  // For composite, bump composite count, decrement iters, save name
            mc_comps++;
            mc_iters--;
            cmfname          = mcfiles[ ii ];
         }
      }
      else
      {  // Handle a file whose description tells that it is a composite
         QFile filei( mcfiles[ ii ] );
         if ( ! filei.open( QIODevice::ReadOnly | QIODevice::Text) )
            continue;
         QTextStream tsi( &filei );
         QString mcont  = tsi.readAll();
         filei.close();
         int jj         = qMax( 0, mcont.indexOf( "description=" ) );
         QString mdesc  = QString( mcont ).mid( jj, 99 ).section( dquo, 1, 1 );
         if ( mdesc.contains( "_mcN" ) )
         {  // For composite, bump composite count, decrement iters, save name
            mc_comps++;
            mc_iters--;
            cmfname          = mcfiles[ ii ];
//qDebug() << "MDL:CMF: (A)cmfname" << cmfname;
         }
      }
   }

   if ( mc_comps > 1  ||  mc_iters < 1 )
   {  // Return now if we don't have 0 or 1 composite, plus some iter models
      qDebug() << "**" << mc_comps << "MC composites, and" << mc_iters
               << "MC iterations **";
                                        return empty_str;
   }

   QString mditer1  = QString().sprintf( ".mcN%03i", mc_iters );
   QString mditer2  = QString( mditer1 ).replace( ".mcN", "_mcN" );
   QString mditer   = mditer1;
//qDebug() << "MDL:CMF: mditer1 mditer2" << mditer1 << mditer2;

   if ( mc_comps == 0 )
   {  // No composite exists, so create one from the iteration models
      cmfname          = mcfiles[ 0 ];
      QFile filei( mcfiles[ 0 ] );
      if ( ! filei.open( QIODevice::ReadOnly | QIODevice::Text) )
      {
         qDebug() << "**MC iteration file open error**";
                                        return empty_str;
      }
      QTextStream tsi( &filei );
      QString mcont    = tsi.readAll();
      filei.close();
      int jj           = qMax( 0, mcont.indexOf( "modelGUID=" ) );
      QString mcguid   = QString( mcont ).mid( jj, 99 ).section( dquo, 1, 1 );

      if ( ! name_desc )
      {  // No description in name ("M00...xml"), so determine composite name
         if ( rmvi )
         {  // Removing iteration files, so reuse first name for composite
            filei.remove();
         }

         else
         {  // If not removing iteration files, get a new name for composite
            bool newFile     = true;
            QString path     = QString( cmfname ).section( "/", 0, -2 );
            mcguid           = US_Util::new_guid();
            cmfname          = get_filename( path, mcguid, newFile );
//qDebug() << "MDL:CMF: (B)cmfname" << cmfname;
         }
      }

      else
      {  // Description in name, so create a new name with iters count in it
         int moix1        = cmfname.indexOf( ".mc" );
         int moix2        = cmfname.indexOf( "_mc" );
         int moix         = ( moix1 > 0 ) ? moix1 : qMax( 0, moix2 );
         QString moiter   = QString( cmfname ).mid( moix, 7 );
         mditer           = ( moix1 > 0 ) ? mditer1 : mditer2; 
         cmfname          = cmfname.replace( moiter, mditer )
                            .replace( ".mdl.tmp", ".model.xml" );
//qDebug() << "MDL:CMF: moix1 moix2 moiter mditer" << moix1 << moix2 << moiter << mditer;
//qDebug() << "MDL:CMF: (C)cmfname" << cmfname;
      }

      QFile fileo( cmfname );
      if ( ! fileo.open( QIODevice::WriteOnly | QIODevice::Text ) )
      {
         qDebug() << "**MC composite file open error**";
                                        return empty_str;
      }

      // Output contents of first iteration model except last line
      QTextStream tso( &fileo );
      jj               = qMax( 0, mcont.indexOf( "modelGUID=" ) );
      QString miguid   = QString( mcont ).mid( jj, 99 ).section( dquo, 1, 1 );
      if ( miguid != mcguid )
         mcont.replace( miguid, mcguid );
      int flx        = 0;
      int llx        = ( mc_iters > 1 ) ? -3 : -2;
      tso << mcont.section( "\n", flx, llx ) << "\n";
      flx            = 3;

      // Output <model>...</model> for 2nd thru last iteration model
      for ( int ii = 1; ii < mc_iters; ii++ )
      {
         QFile filei( mcfiles[ ii ] );
         if ( ! filei.open( QIODevice::ReadOnly | QIODevice::Text) )
            continue;
         QTextStream tsi( &filei );
         mcont          = tsi.readAll();
         filei.close();
         jj             = qMax( 0, mcont.indexOf( "modelGUID=" ) );
         QString miguid = QString( mcont ).mid( jj, 99 ).section( dquo, 1, 1 );
         if ( miguid != mcguid )
            mcont.replace( miguid, mcguid );
         llx            = ( ( ii + 1 ) < mc_iters ) ? -3 : -2;
         tso << mcont.section( "\n", flx, llx ) << "\n";
      }

      fileo.close();
   }

   else
   {  // A composite exists, so append to it from new iteration models
      QFile filei( cmfname );
      if ( ! filei.open( QIODevice::ReadOnly | QIODevice::Text) )
      {
         qDebug() << "**MC composite file open error**";
                                        return empty_str;
      }
      // Read in contents of existing composite model; skip last line
      QTextStream tsi( &filei );
      QString mcont    = tsi.readAll();
      filei.close();
      mcont            = mcont.section( "\n", 0, -3 ) + "\n";
      int jj           = qMax( 0, mcont.indexOf( "modelGUID=" ) );
      QString mcguid   = QString( mcont ).mid( jj, 99 ).section( dquo, 1, 1 );

      if ( name_desc )
      {  // Name has description, so see if it needs to be renamed
         int moix1        = cmfname.indexOf( ".mc" );
         int moix2        = cmfname.indexOf( "_mc" );
         int moix         = ( moix1 > 0 ) ? moix1 : qMax( 0, moix2 );
         QString moiter   = QString( cmfname ).mid( moix, 7 );
         int mc_ittot     = QString( moiter ).mid( 4 ).toInt() + mc_iters;
         mditer           = ( ( moix1 > 0 ) ? "." : "_" )
                            + QString().sprintf( "mcN%03i", mc_ittot );
//qDebug() << "MDL:CMF: (B)moix1 moix2 moiter mditer" << moix1 << moix2 << moiter << mditer;

         if ( moiter != mditer )
         {  // Iterations changes (almost always), so rename is necessary
            cmfname          = cmfname.replace( moiter, mditer )
                               .replace( ".mdl.tmp", ".model.xml" );
            filei.rename( cmfname );
//qDebug() << "MDL:CMF: (D)cmfname" << cmfname;
         }
         else
         {  // No name change (unlikely), so must delete and re-create
            filei.remove();
         }
      }

      else
      {  // Name has no description, so no renaming necessary and just recreate
         filei.remove();
      }

      QFile fileo( cmfname );
      if ( ! fileo.open( QIODevice::WriteOnly | QIODevice::Text ) )
      {
         qDebug() << "**MC composite file open error**";
                                        return empty_str;
      }

      // Output contents of previous composite except for last line
      QTextStream tso( &fileo );
      int flx        = 0;
      int llx        = -3;
      tso << mcont.section( "\n", flx, llx ) << "\n";
      flx            = 3;

      // Append <model>...</model> of all new iteration models
      for ( int ii = 0; ii < mc_iters; ii++ )
      {
         QFile filei( mcfiles[ ii ] );
         if ( ! filei.open( QIODevice::ReadOnly | QIODevice::Text) )
            continue;
         QTextStream tsi( &filei );
         mcont          = tsi.readAll();
         filei.close();
         jj             = qMax( 0, mcont.indexOf( "modelGUID=" ) );
         QString miguid = QString( mcont ).mid( jj, 99 ).section( dquo, 1, 1 );
         if ( miguid != mcguid )
            mcont.replace( miguid, mcguid );
         llx            = ( ( ii + 1 ) < mc_iters ) ? -3 : -2;
         tso << mcont.section( "\n", flx, llx ) << "\n";
      }

      fileo.close();
   }

   if ( rmvi )
   {  // If so requested, remove the MC iteration files
      if ( mc_comps == 0 )
      {  // If list is all iteration files, remove them all
         for ( int ii = 0; ii < mcfiles.size(); ii++ )
         {  // Delete unless file name was reused for composite
            if ( mcfiles[ 0 ] != cmfname )
               QFile( mcfiles[ ii ] ).remove();
         }
      }

      else if ( name_desc )
      {  // If list has composite, name has description;  selectively delete
         for ( int ii = 0; ii < mcfiles.size(); ii++ )
         {  // Delete iteration files (not composite)
            if ( ! mcfiles[ ii ].contains( "mcN" ) )
               QFile( mcfiles[ ii ] ).remove();
         }
      }

      else
      {  // If no description in file names, delete based on descr. content
         for ( int ii = 0; ii < mcfiles.size(); ii++ )
         {  // Delete iteration files (not composite)
            if ( mcfiles[ ii ] == cmfname )
               continue;
            QFile filei( mcfiles[ ii ] );
            if ( ! filei.open( QIODevice::ReadOnly | QIODevice::Text) )
            {
               qDebug() << "**MC iteration file open error**";
                                        continue;
            }

            QTextStream tsi( &filei );
            QString mcont  = tsi.readAll();
            filei.close();
            int jj         = qMax( 0, mcont.indexOf( "description=" ) );
            QString mdesc  = QString(mcont).mid( jj, 99 ).section( dquo, 1, 1 );
            if ( ! mdesc.contains( "_mcN" ) )
               QFile( mcfiles[ ii ] ).remove();
         }
      }
   }

   return cmfname;
}

// Get MC iteration xml content strings
int US_Model::mc_iter_xmls( QStringList& mcixs )
{
   int kixmls   = mcixmls.size();
   mcixs        = mcixmls;
   return kixmls;
}

// Flag if subtype values matches a given flag
bool US_Model::subtype_match( const int subtype, const int stflag )
{
   return ( ( subtype & stflag ) != 0 );
}

// Output debug print model details
void US_Model::debug( void )
{
   qDebug() << "model dump";
   qDebug() << "desc" << description;
   qDebug() << "model guid" << modelGUID;
   qDebug() << "variance" << variance;
   qDebug() << "meniscus" << meniscus;
   qDebug() << "bottom" << bottom;
   qDebug() << "alphaRP" << alphaRP;
   qDebug() << "edit guid" << editGUID;
   qDebug() << "request guid" << requestGUID;
   qDebug() << "waveln" << wavelength;
   qDebug() << "monte carlo" << monteCarlo;
   qDebug() << "coSed" << coSedSolute;
   qDebug() << "subGrids" << subGrids;
   qDebug() << "AnalysisType" << (int)analysis;
   qDebug() << "GlobalType" << (int)global;
   qDebug() << "OpticsType" << (int)optics;
   qDebug() << "data description" << dataDescrip;

   for ( int i = 0; i < components.size(); i++ )
   {
      SimulationComponent* sc = &components[ i ];
      qDebug() << " component" << ( i + 1 );
      qDebug() << "  name" << sc->name;
      qDebug() << "  vbar20" << sc->vbar20;
      qDebug() << "  mw" << sc->mw;
      qDebug() << "  s" << sc->s;
      qDebug() << "  D" << sc->D;
      qDebug() << "  f" << sc->f;
      qDebug() << "  f/f0" << sc->f_f0;
      qDebug() << "  extinction" << sc->extinction;
      qDebug() << "  axial" << sc->axial_ratio;
      qDebug() << "  sigma" << sc->sigma;
      qDebug() << "  delta" << sc->delta;
      qDebug() << "  molar C" << sc->molar_concentration;
      qDebug() << "  signal C" << sc->signal_concentration;
      qDebug() << "  oligomer" << sc->oligomer;
      qDebug() << "  shape" << (int)sc->shape;
      qDebug() << "  type" << (int)sc->analyte_type;

      for ( int j = 0; j < sc->c0.radius.size(); j++ )
      {
         qDebug() << "   c0 r c"
            << sc->c0.radius[ j ] << sc->c0.concentration[ j ];
      }
   }
}

