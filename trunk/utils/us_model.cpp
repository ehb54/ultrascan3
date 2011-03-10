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
   k_eq  = 0.0;
   k_off = 0.0;
   rcomps .clear();
   stoichs.clear();
}

bool US_Model::Association::operator== ( const Association& a ) const
{
   if ( k_eq    != a.k_eq    ) return false;
   if ( k_off   != a.k_off   ) return false;
   if ( rcomps  != a.rcomps  ) return false;
   if ( stoichs != a.stoichs ) return false;

   return true;
}

US_Model::US_Model()
{
   monteCarlo      = false;
   wavelength      = 0.0;
   description     = "New Model";
   optics          = ABSORBANCE;
   analysis        = MANUAL;
   global          = NONE;

   coSedSolute     = -1;
   modelGUID   .clear();
   editGUID    .clear();
   requestGUID .clear();
   components  .clear();
   associations.clear();
}

bool US_Model::operator== ( const US_Model& m ) const
{
   if ( monteCarlo      != m.monteCarlo      ) return false;
   if ( wavelength      != m.wavelength      ) return false;
   if ( modelGUID       != m.modelGUID       ) return false;
   if ( editGUID        != m.editGUID        ) return false;
   if ( requestGUID     != m.requestGUID     ) return false;
   if ( description     != m.description     ) return false;
   if ( optics          != m.optics          ) return false;
   if ( analysis        != m.analysis        ) return false;
   if ( global          != m.global          ) return false;
   if ( coSedSolute     != m.coSedSolute     ) return false;
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
   bool   ok = true;
   double vbar;             // component vbar
   double volume;           // e.g., vbar * mw / AVOGADRO
   double vol_fac;          // volume factor, e.g., 0.75 / M_PI
   double radius_sphere;    // e.g., pow( volume * vol_fac, 1/3 );
   double rsph_fac;         // radius_sphere factor; e.g., 0.06 * PI * VISC
   double onethird;         // one third ( 1.0 / 3.0 )
   double c;                // concentration
   double t;                // temperature in kelvin
   double s;                // sedimentation coefficient
   double D;                // diffusion coefficient
   double mw;               // molecular weight
   double f;                // frictional coefficient
   double fv;               // frictional coefficient (working value)
   double f_f0;             // frictional ratio
   double f0;               // f-zero
   double s20w;
   double buoyancyb;
   US_Math2::SolutionData d; // data correction object

   // Ensure that we have a vbar we can use
   vbar           = component.vbar20;

   if ( vbar <= 0.0 )
   {
      vbar           = TYPICAL_VBAR;
      component.vbar20 = vbar;
   }

   t              = K20;                // temperature kelvin of 20 degr. C
   vol_fac        = 0.75 / M_PI;        // various factors used in calcs below
   onethird       = 1.0 / 3.0;
   rsph_fac       = 0.06 * M_PI * VISC_20W;
   buoyancyb      = 1.0 - vbar * DENS_20W;

   s              = component.s;        // component coefficient values
   D              = component.D;
   mw             = component.mw;
   f              = component.f;
   f_f0           = component.f_f0;
   c              = component.signal_concentration;
   fv             = f;

   d.vbar         = TYPICAL_VBAR;       // data correction for buoyancy
   d.vbar20       = vbar;
   d.density      = DENS_20W;
   d.viscosity    = VISC_20W;

   US_Math2::data_correction( NORMAL_TEMP, d );

   // Start with already calculated s if possible
   if ( s != 0.0 )
   {
      s20w           = qAbs( s );

      // First check s and D
                                                 ///////////////
      if ( D != 0.0 )                            // s and D
      {                                          ///////////////
         mw             = s * R * t / ( D * buoyancyb );
         volume         = vbar * mw / AVOGADRO;
         radius_sphere  = pow( volume * vol_fac, onethird );
         f0             = radius_sphere * rsph_fac;
         fv             = mw * buoyancyb / ( s20w * AVOGADRO );
         double ff0sv   = f_f0;
         f_f0           = fv / f0;
         double ffdif   = qAbs( ff0sv - f_f0 );
         f_f0           = ( ffdif < 1.e-5 ) ? ff0sv : f_f0;
      }

      // Next check s and k (f_f0)
                                                 ///////////////
      else if ( f_f0 != 0.0 )                    // s and f_f0
      {                                          ///////////////
         double numer   = 0.02 * s * f_f0 * vbar * VISC_20W;
         f0             = 0.09 * VISC_20W * M_PI * sqrt( numer / buoyancyb );
         fv             = f_f0 * f0;
         D              = R * t / ( AVOGADRO * fv );
         mw             = s * R * t / ( D * buoyancyb );
      }

      // Then check any other s + combinations
                                                 ///////////////
      else if ( mw != 0.0 )                      // s and mw
      {                                          ///////////////
         D              = s * R * t / ( d.buoyancyb * mw );
         fv             = mw * d.buoyancyb / ( s20w * AVOGADRO );
         volume         = vbar * mw / AVOGADRO;
         radius_sphere  = pow( volume * vol_fac, onethird );
         f0             = radius_sphere * rsph_fac;
         f_f0           = fv / f0;
      }
                                                 ///////////////
      else if ( f != 0.0 )                       // s and f
      {                                          ///////////////
         D              = R * t / ( AVOGADRO * fv );
         mw             = s * R * t / ( D * buoyancyb );
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
         s              = D * buoyancyb * mw / ( R * t );
         fv             = mw * buoyancyb / ( s * AVOGADRO );
         f_f0           = fv / f0;
      }
                                                 ///////////////
      else if ( f_f0 != 0.0 )                    // mw and f_f0
      {                                          ///////////////
         fv             = f_f0 * f0;
         s              = mw * buoyancyb / ( AVOGADRO * fv );
         D              = s * R * t / ( buoyancyb * mw );
      }
                                                 ///////////////
      else if ( f != 0.0 )                       // mw and f
      {                                          ///////////////
         f_f0           = fv / f0;
         s              = mw * buoyancyb / ( AVOGADRO * fv );
         D              = s * R * t / ( buoyancyb * mw );
      }
                                                 //****************************
      else                                       // do not have 2 valid coeffs
         ok             = false;                 //****************************
   }

   else if ( component.D    != 0.0 )
   {                                             ///////////////
      if ( f_f0 >= 1.0 )                         // D and f_f0
      {                                          ///////////////
         fv             = R * t / ( AVOGADRO * D );
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
      D              = R * t / ( AVOGADRO * fv );
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

   return ok;
}

// Test the existence of the models directory path and create it if need be
bool US_Model::model_path( QString& path )
{
   QDir dir;
   path = US_Settings::dataDir() + "/models";

   if ( ! dir.exists( path ) )
   {
      if ( ! dir.mkpath( path ) )
      {
         return false;
      }
   }

   return true;
}

// Short text string describing the model type
QString US_Model::typeText( void )
{
   struct typemap
   {
      AnalysisType typeval;
      QString      typedesc;
   };

   const typemap tmap[] =
   {
      { MANUAL,    QObject::tr( "Manual"  ) },
      { TWODSA,    QObject::tr( "2DSA"    ) },
      { TWODSA_MW, QObject::tr( "2DSA-MW" ) },
      { GA,        QObject::tr( "GA"      ) },
      { GA_MW,     QObject::tr( "GA-MW"   ) },
      { COFS,      QObject::tr( "COFS"    ) },
      { FE,        QObject::tr( "FE"      ) },
      { ONEDSA,    QObject::tr( "1DSA"    ) }
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

         if ( global == MENISCUS )          // Fit Meniscus subtype
            tdesc    = tdesc + "-FM";

         else if ( global == GLOBAL )       // Global subtype
            tdesc    = ( jj > 0 ) ? tdesc + "-GL" : "Global";

         else if ( global == SUPERGLOBAL )  // SuperGlobal subtype
            tdesc    = ( jj > 0 ) ? tdesc + "-SG" : "SuperGlobal";

         if ( monteCarlo )                  // Monte Carlo subtype
            tdesc    = tdesc + "-MC";

         break;
      }
   }

   return tdesc;                            // return type description text
}

int US_Model::load( bool db_access, const QString& guid, US_DB2* db )
{
   if ( db_access ) return load_db  ( guid, db );
   else             return load_disk( guid );
}

// Load model from local disk
int US_Model::load_disk( const QString& guid )
{
   int error = US_DB2::ERROR;  // Error by default

   QString path;

   if ( ! model_path( path ) )
   {
      message = QObject::tr ( "Could not create analyte directory" );
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

int US_Model::load( const QString& filename )
{
   QFile file( filename );

   if ( ! file.open( QIODevice::ReadOnly | QIODevice::Text) )
      return US_DB2::ERROR;

   QXmlStreamReader xml( &file );
   
   int result = load_stream( xml );
   file.close();
   return result;
}

int US_Model::load_stream( QXmlStreamReader& xml )
{
   QString coSedStr;
   QString comprStr;

   components  .clear();
   associations.clear();

   QXmlStreamAttributes a;
   bool                 read_next = true;

   while ( ! xml.atEnd() )
   {
      if ( read_next ) xml.readNext();
      read_next = true;

      if ( xml.isStartElement() )
      {
         if ( xml.name() == "model" )
         {
            a = xml.attributes();

            QString mcst    = a.value( "monteCarlo"     ).toString();
            monteCarlo      = ( ! mcst.isEmpty()  &&  mcst != "0" );
            QString iter    = a.value( "iterations"     ).toString();
            monteCarlo      = monteCarlo ? monteCarlo :
                            ( iter.isEmpty() ? false : ( iter.toInt() > 1 ) );
            wavelength      = a.value( "wavelength"     ).toString().toDouble();
            description     = a.value( "description"    ).toString();
            modelGUID       = a.value( "modelGUID"      ).toString();
            editGUID        = a.value( "editGUID"       ).toString();
            requestGUID     = a.value( "requestGUID"    ).toString();
            coSedStr        = a.value( "coSedSolute"    ).toString();
            coSedSolute     = ( coSedStr.isEmpty() ) ? -1 : coSedStr.toInt();
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
            sc.vbar20       = a.value( "vbar20"     ).toString().toDouble();
            sc.mw           = a.value( "mw"         ).toString().toDouble();
            sc.s            = a.value( "s"          ).toString().toDouble();
            sc.D            = a.value( "D"          ).toString().toDouble();
            sc.f            = a.value( "f"          ).toString().toDouble();
            sc.f_f0         = a.value( "f_f0"       ).toString().toDouble();
            sc.extinction   = a.value( "extinction" ).toString().toDouble();
            sc.axial_ratio  = a.value( "axial"      ).toString().toDouble();
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

void US_Model::get_associations( QXmlStreamReader& xml, Association& as )
{
   QXmlStreamAttributes a = xml.attributes();
   as.k_eq  = a.value( "k_eq"  ).toString().toDouble();
   as.k_off = a.value( "k_off" ).toString().toDouble();

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

int US_Model::load( const QString& id, US_DB2* db )
{
   QStringList q;

   q << "get_model_info" << id;
   db->query( q );
   
   if ( db->lastErrno() != US_DB2::OK ) return db->lastErrno();

   db->next();
   QByteArray contents = db->value( 2 ).toString().toAscii();

   // Write the model file to an array in memory
   QXmlStreamReader xml( contents );
   return load_stream( xml );
}

int US_Model::write( bool db_access, const QString& filename, US_DB2* db )
{
   if ( db_access ) return write( db );
   else             return write( filename );
}

int US_Model::write( US_DB2* db )
{
      // Create the model xml file in a string
      QByteArray temporary;
      QByteArray contents;
      
      QXmlStreamWriter xml( &temporary );
      write_stream( xml );
      db->mysqlEscapeString( contents, temporary, temporary.size() );

      QStringList q;

      // Generate a guid if necessary
      // The guid may be valid from a disk read, but is not in the DB
      if ( modelGUID.size() != 36 ) modelGUID = US_Util::new_guid();

      q << "get_modelID" << modelGUID;
      
      db->query( q );
      
      q.clear();
     
      if ( db->lastErrno() != US_DB2::OK )
      {
         q << "new_model" << modelGUID << description << contents << editGUID
           << QString::number( US_Settings::us_inv_ID() );
         message = QObject::tr( "created" );
      }
      else
      {
         db->next();
         QString id = db->value( 0 ).toString();
         q << "update_model" << id << description << contents << editGUID;
         message = QObject::tr( "updated" );
      }

      return db->statusQuery( q );
}

int US_Model::write( const QString& filename )
{
   QFile file( filename );

   if ( ! file.open( QIODevice::WriteOnly | QIODevice::Text) )
      return US_DB2::ERROR;

   QXmlStreamWriter xml( &file );
   write_stream( xml );
   file.close();

   return US_DB2::OK;
}

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
   xml.writeAttribute   ( "coSedSolute", QString::number( coSedSolute  ) );
   xml.writeAttribute   ( "opticsType",  QString::number( optics       ) );
   xml.writeAttribute   ( "analysisType",QString::number( analysis     ) );
   xml.writeAttribute   ( "globalType",  QString::number( global       ) );

   if ( requestGUID.length() > 0 )
      xml.writeAttribute   ( "requestGUID", requestGUID                     );

   if ( monteCarlo )
      xml.writeAttribute   ( "monteCarlo",  "1"                             );

   // Write components
   for ( int i = 0; i < components.size(); i++ )
   {
      SimulationComponent* sc = &components[ i ];
      xml.writeStartElement( "analyte" );

      xml.writeAttribute( "analyteGUID", ""                                  );
      xml.writeAttribute( "name",       sc->name                             );
      xml.writeAttribute( "vbar20",     QString::number( sc->vbar20        ) );
      xml.writeAttribute( "mw",         QString::number( sc->mw            ) );
      xml.writeAttribute( "s",          QString::number( sc->s             ) );
      xml.writeAttribute( "D",          QString::number( sc->D             ) );
      xml.writeAttribute( "f",          QString::number( sc->f             ) );
      xml.writeAttribute( "f_f0",       QString::number( sc->f_f0          ) );
      xml.writeAttribute( "extinction", QString::number( sc->extinction    ) );
      xml.writeAttribute( "axial",      QString::number( sc->axial_ratio   ) );
      xml.writeAttribute( "sigma",      QString::number( sc->sigma         ) );
      xml.writeAttribute( "delta",      QString::number( sc->delta         ) );
      xml.writeAttribute( "oligomer",   QString::number( sc->oligomer      ) );
      xml.writeAttribute( "shape",      QString::number( sc->shape         ) );
      xml.writeAttribute( "type",       QString::number( sc->analyte_type  ) );

      xml.writeAttribute( "molar",  QString::number( sc->molar_concentration ));
      xml.writeAttribute( "signal", QString::number( sc->signal_concentration));

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
      xml.writeAttribute( "k_eq",  QString::number( as->k_eq  ) );
      xml.writeAttribute( "k_off", QString::number( as->k_off ) );

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

void US_Model::debug( void )
{
   qDebug() << "model dump";
   qDebug() << "desc" << description;
   qDebug() << "model guid" << modelGUID;
   qDebug() << "edit guid" << editGUID;
   qDebug() << "request guid" << requestGUID;
   qDebug() << "waveln" << wavelength;
   qDebug() << "monte carlo" << monteCarlo;
   qDebug() << "coSed" << coSedSolute;
   qDebug() << "AnalysisType" << (int)analysis;
   qDebug() << "GlobalType" << (int)global;
   qDebug() << "OpticsType" << (int)optics;

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

