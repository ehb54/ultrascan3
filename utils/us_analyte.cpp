//! \file us_analyte.cpp
#include "us_analyte.h"
#include "us_constants.h"
#include "us_settings.h"
#include "us_math2.h"


//#include <uuid/uuid.h>

US_Analyte::US_Analyte()
{
   invID = -1; 
   vbar  = TYPICAL_VBAR;
   mw    = 50000.0;
   type  = PROTEIN;

   extinction  .clear();
   refraction  .clear();
   fluorescence.clear();
   description .clear();
   guid        .clear();
   sequence    .clear();
}

US_Analyte US_Analyte::load( 
      bool           db_access, 
      const QString& guid,
      int&           error,
      US_DB2*        db )
{
   if ( db_access ) return load_db  ( guid, error, db );
   else             return load_disk( guid, error );
}

US_Analyte US_Analyte::load_db( const QString& guid, int& error, US_DB2* db )
{
   US_Analyte analyte;

   error = US_DB2::OK;

   // Get analyteID
   QStringList q( "get_analyteID" );
   q << guid;

   db->query( q );
   error = db->lastErrno();
   if ( error != US_DB2::OK ) return analyte;

   db->next();
   QString analyteID = db->value( 0 ).toString();

   // Get analyte info;
   q.clear();
   q << "get_analyte_info" << analyteID;

   db->query( q );
   error = db->lastErrno();
   if ( error != US_DB2::OK ) return analyte;

   db->next();

   analyte.guid     = guid;;
  
   int type         = db->value( 1 ).toString().toInt();
   
   switch ( type )
   {
      case PROTEIN     : analyte.type = PROTEIN;
      case DNA         : analyte.type = DNA;
      case RNA         : analyte.type = RNA;
      case CARBOHYDRATE: analyte.type = CARBOHYDRATE;
   }
   
   analyte.sequence    = db->value( 2 ).toString();
   analyte.vbar        = db->value( 3 ).toString().toDouble();
   analyte.description = db->value( 4 ).toString();
   // We don't need spectrum  -- db->value( 5 ).toString();
   analyte.mw          = db->value( 6 ).toString().toDouble();
   analyte.invID       = db->value( 7 ).toString().toInt();

   q.clear();
   q << "get_spectrum" << analyteID << "Analyte" << "'Extinction";

   db->query( q );

   while ( db->next() )
   {
      double lambda = db->value( 1 ).toDouble();
      double coeff  = db->value( 2 ).toDouble();
      analyte.extinction[ lambda ] = coeff;
   }

   q[ 3 ] = "Refraction";
   db->query( q );

   while ( db->next() )
   {
      double lambda = db->value( 1 ).toDouble();
      double coeff  = db->value( 2 ).toDouble();
      analyte.refraction[ lambda ] = coeff;
   }

   q[ 3 ] = "Fluorescence";
   db->query( q );

   while ( db->next() )
   {
      double lambda = db->value( 1 ).toDouble();
      double coeff  = db->value( 2 ).toDouble();
      analyte.fluorescence[ lambda ] = coeff;
   }

   return analyte;
}

US_Analyte US_Analyte::load_disk( const QString& guid, int& error )
{
   error = US_DB2::NO_COMPONENT;  // Error by default
   
   US_Analyte analyte;
   QString    path;

   if ( ! analyte_path( path ) ) return analyte;  

   QDir        f( path );
   QStringList filter( "A*.xml" );
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
            if ( xml.name() == "analyte" )
            {
               QXmlStreamAttributes a = xml.attributes();

               if ( a.value( "guid" ).toString() == guid ) found = true; 
               break;
            }
         }
      }

      file.close();

      if ( found ) 
      {
         error = US_DB2::OK;
         return read_analyte( filename, error );
      };
   }

   return analyte;
}

US_Analyte US_Analyte::read_analyte( const QString& filename, int& error )
{
   US_Analyte analyte;

   QFile file( filename );

   // Read in the filename and populate class
   if ( ! file.open( QIODevice::ReadOnly | QIODevice::Text) )
   {
      // Fail quietly
      qDebug() << "Cannot open file " << filename;
      error = US_DB2::NO_COMPONENT;
      return analyte;
   }

   double               freq;
   double               value;
   QString              type;
   QXmlStreamReader     xml( &file );
   QXmlStreamAttributes a;

   while ( ! xml.atEnd() )
   {
      xml.readNext();

      if ( xml.isStartElement() )
      {
         if ( xml.name() == "analyte" )
         {
            a = xml.attributes();

            type = a.value( "type" ).toString();

            // Set description and guid
            analyte.description = a.value( "description" ).toString();
            analyte.guid        = a.value( "guid"        ).toString();

            // Set type
            if ( type == "PROTEIN" )
            {
               analyte.vbar = a.value( "vbar"        ).toString().toDouble();
               analyte.type = PROTEIN;
            }

            else if ( type == "DNA"  ||  type == "RNA" )
            {
              analyte.type = ( type == "DNA" ) ? DNA : RNA;
              analyte.vbar = a.value( "vbar20" ).toString().toDouble();
            }

            else
               analyte.type = CARBOHYDRATE;
         }

         else if ( xml.name() == "sequence" )
         {
            analyte.sequence = xml.readElementText();

            // Set mw
            if ( type == "PROTEIN" )
            {
               US_Math2::Peptide p;
               US_Math2::calc_vbar( p, analyte.sequence, NORMAL_TEMP );
               analyte.mw = p.mw;

               // The sequence tag comes before the extinction extinction tag
               // so a value set there will override this setting, if it
               // exists.  It's not the way xml is really supposed work, but it
               // will be ok in this case.

               analyte.extinction[ 280.0 ] = p.e280; 
            }
            else if ( type == "DNA"  ||  type == "RNA" )
               analyte.mw = nucleotide( a, analyte.sequence );

            else // CARBOHYDRATE
               analyte.mw = 0.0; 
         }

         else if ( xml.name() == "extinction" )
         {
            QXmlStreamAttributes a = xml.attributes();
            freq  = a.value( "frequency" ).toString().toDouble();
            value = a.value( "value"     ).toString().toDouble();
            analyte.extinction[ freq ] = value;
         }

         else if ( xml.name() == "refraction" )
         {
            QXmlStreamAttributes a = xml.attributes();
            freq  = a.value( "frequency" ).toString().toDouble();
            value = a.value( "value"     ).toString().toDouble();
            analyte.refraction[ freq ] = value;
         }

         else if ( xml.name() == "fluorescence" )
         {
            QXmlStreamAttributes a = xml.attributes();
            freq  = a.value( "frequency" ).toString().toDouble();
            value = a.value( "value"     ).toString().toDouble();
            analyte.fluorescence[ freq ] = value;
         }
      }
   }

   return analyte;
}

double US_Analyte::nucleotide( const QXmlStreamAttributes& a, 
                               const QString&              seq )
{
   QString sequence = seq;
   sequence.toLower();
   sequence.remove( QRegExp( "[^acgtu]" ) );

   uint A = sequence.count( "a" );
   uint C = sequence.count( "c" );
   uint G = sequence.count( "g" );
   uint T = sequence.count( "t" );
   uint U = sequence.count( "u" );

   bool isDNA          = ( a.value( "type"               ).toString() == "DNA" );
   bool doubleStranded = ( a.value( "stranded"           ).toString() == "T" );
   bool complement     = ( a.value( "complement_only"    ).toString() == "T" );
   bool _3prime        = ( a.value( "ThreePrimeHydroxyl" ).toString() == "T" );
   bool _5prime        = ( a.value( "FivePrimeHydroxyl"  ).toString() == "T" );

   const double mw_A = 313.209;
   const double mw_C = 289.184;
   const double mw_G = 329.208;
   const double mw_T = 304.196;
   const double mw_U = 274.170;

   double MW    = 0.0;
   uint   total = A + G + C + T + U;

   if ( isDNA )
   {
      if ( doubleStranded )
      {
         MW += A * mw_A;
         MW += G * mw_G;
         MW += C * mw_C;
         MW += T * mw_T;
         MW += A * mw_T;
         MW += G * mw_C;
         MW += C * mw_G;
         MW += T * mw_A;
      }

      if ( complement )
      {
         MW += A * mw_T;
         MW += G * mw_C;
         MW += C * mw_G;
         MW += T * mw_A;
      }

      if ( ! complement && ! doubleStranded )
      {
         MW += A * mw_A;
         MW += G * mw_G;
         MW += C * mw_C;
         MW += T * mw_T;
      }
   }

   else /* RNA */
   {
      if ( doubleStranded )
      {
         MW += A * ( mw_A + 15.999 );
         MW += G * ( mw_G + 15.999 );
         MW += C * ( mw_C + 15.999 );
         MW += U * ( mw_U + 15.999 );
         MW += A * ( mw_U + 15.999 );
         MW += G * ( mw_C + 15.999 );
         MW += C * ( mw_G + 15.999 );
         MW += U * ( mw_A + 15.999 );
      }

      if ( complement )
      {
         MW += A * ( mw_U + 15.999 );
         MW += G * ( mw_C + 15.999 );
         MW += C * ( mw_G + 15.999 );
         MW += U * ( mw_A + 15.999 );
      }

      if ( ! complement && ! doubleStranded )
      {
         MW += A * ( mw_A + 15.999 );
         MW += G * ( mw_G + 15.999 );
         MW += C * ( mw_C + 15.999 );
         MW += U * ( mw_U + 15.999 );
      }
   }
   
   double sodium    = a.value( "sodium"    ).toString().toDouble();
   double potassium = a.value( "potassium" ).toString().toDouble();
   double lithium   = a.value( "lithium"   ).toString().toDouble();
   double magnesium = a.value( "magnesium" ).toString().toDouble();
   double calcium   = a.value( "calcium"   ).toString().toDouble();

   MW += sodium    * total * 22.99;
   MW += potassium * total * 39.1;
   MW += lithium   * total * 6.94;
   MW += magnesium * total * 24.305;
   MW += calcium   * total * 40.08;

   if ( _3prime )
   {
      MW += 17.01;
      if ( doubleStranded )  MW += 17.01;
   }
   else // we have phosphate
   {
      MW += 94.87;
      if ( doubleStranded ) MW += 94.87;
   }

   if ( _5prime )
   {
      MW -=  77.96;
      if ( doubleStranded )  MW -= 77.96;
   }

   return MW;
}

bool US_Analyte::analyte_path( QString& path )
{
   QDir dir;
   path = US_Settings::dataDir() + "/analytes";

   if ( ! dir.exists( path ) )
   {
      if ( ! dir.mkpath( path ) ) return false;
   }

   return true;
}
