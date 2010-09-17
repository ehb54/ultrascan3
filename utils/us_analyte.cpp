//! \file us_analyte.cpp
#include "us_analyte.h"
#include "us_constants.h"
#include "us_settings.h"
#include "us_math2.h"

US_Analyte::US_Analyte()
{
   invID          = -1; 
   vbar20         = TYPICAL_VBAR;
   mw             = 50000.0;
   description    = "New Analyte";
   analyteGUID    .clear();
   sequence       .clear();
   type           = PROTEIN;

   // Placeholders for DNA/RNA
   doubleStranded = true;
   complement     = false;
   _3prime        = false;
   _5prime        = false;

   sodium         = 0.0;
   potassium      = 0.0;
   lithium        = 0.0;
   magnesium      = 0.0;
   calcium        = 0.0;

   extinction     .clear();
   refraction     .clear();
   fluorescence   .clear();
}

bool US_Analyte::operator== ( const US_Analyte& a ) const
{
   /*
   if ( invID        != a.invID        ) qDebug() << invID        << a.invID;
   if ( vbar20       != a.vbar20       ) qDebug() << vbar20       << a.vbar20;
   if ( mw           != a.mw           ) qDebug() << mw           << a.mw ;
   if ( description  != a.description  ) qDebug() << description  << a.description ;
   if ( guid         != a.guid         ) qDebug() << guid         << a.guid ;
   if ( sequence     != a.sequence     ) qDebug() << sequence     << a.sequence ;
   if ( type         != a.type         ) qDebug() << type         << a.type  ;
   if ( extinction   != a.extinction   ) qDebug() << extinction   << a.extinction ;
   if ( refraction   != a.refraction   ) qDebug() << refraction   << a.refraction;
   if ( fluorescence != a.fluorescence ) qDebug() << fluorescence << a.fluorescence;

   if ( type == DNA  || type == RNA )
   {
      if ( doubleStranded != a.doubleStranded ) qDebug() << doubleStranded << a.doubleStranded ;
      if ( complement     != a.complement     ) qDebug() << complement     << a.complement ;
      if ( _3prime        != a._3prime        ) qDebug() << _3prime        << a._3prime ;
      if ( _5prime        != a._5prime        ) qDebug() << _5prime        << a._5prime ;
      if ( sodium         != a.sodium         ) qDebug() << sodium         << a.sodium ;
      if ( potassium      != a.potassium      ) qDebug() << potassium      << a.potassium ;
      if ( lithium        != a.lithium        ) qDebug() << lithium        << a.lithium ;
      if ( magnesium      != a.magnesium      ) qDebug() << magnesium      << a.magnesium ;
      if ( calcium        != a.calcium        ) qDebug() << calcium        << a.calcium ;
   }
   */

   if ( invID        != a.invID        ) return false;
   if ( vbar20       != a.vbar20       ) return false;
   if ( mw           != a.mw           ) return false;
   if ( description  != a.description  ) return false;
   if ( analyteGUID  != a.analyteGUID  ) return false;
   if ( sequence     != a.sequence     ) return false;
   if ( type         != a.type         ) return false;
   if ( extinction   != extinction     ) return false;
   if ( refraction   != refraction     ) return false;
   if ( fluorescence != a.fluorescence ) return false;

   if ( type == DNA  || type == RNA )
   {
      if ( doubleStranded != a.doubleStranded ) return false;
      if ( complement     != a.complement     ) return false;
      if ( _3prime        != a._3prime        ) return false;
      if ( _5prime        != a._5prime        ) return false;
      if ( sodium         != a.sodium         ) return false;
      if ( potassium      != a.potassium      ) return false;
      if ( lithium        != a.lithium        ) return false;
      if ( magnesium      != a.magnesium      ) return false;
      if ( calcium        != a.calcium        ) return false;
   }

   // Not comparing message and analyteID

   return true;
}

int US_Analyte::load( 
      bool           db_access, 
      const QString& guid,
      US_DB2*        db )
{
   if ( db_access ) return load_db  ( guid, db );
   else             return load_disk( guid );
}

int US_Analyte::load_db( const QString& load_guid, US_DB2* db )
{
   int error = US_DB2::OK;

   // Get analyteID
   QStringList q( "get_analyteID" );
   q << load_guid;

   db->query( q );
   error = db->lastErrno();
  
   if ( error != US_DB2::OK ) 
   {
      message = QObject::tr( "Could not get analyteID" );    
      return error;
   }

   db->next();
   analyteID = db->value( 0 ).toString();

   // Get analyte info;
   q.clear();
   q << "get_analyte_info" << analyteID;

   db->query( q );
   error = db->lastErrno();

   if ( error != US_DB2::OK )
   {
      message = QObject::tr( "Could not get analyte info" );
      return error;
   }

   db->next();

   analyteGUID = load_guid;;
   type = (US_Analyte::analyte_t) db->value( 1 ).toString().toInt();
   
   sequence    = db->value( 2 ).toString();
   vbar20      = db->value( 3 ).toString().toDouble();
   description = db->value( 4 ).toString();
   // We don't need spectrum  -- db->value( 5 ).toString();
   mw          = db->value( 6 ).toString().toDouble();
   invID       = db->value( 7 ).toString().toInt();

   q.clear();
   q << "get_nucleotide" << analyteID;
   db->query( q );
   db->next();
   
   doubleStranded = db->value( 0 ).toString().toInt();
   complement     = db->value( 1 ).toString().toInt();
   _3prime        = db->value( 2 ).toString().toInt();
   _5prime        = db->value( 3 ).toString().toInt();
   sodium         = db->value( 4 ).toString().toDouble();
   potassium      = db->value( 5 ).toString().toDouble();
   lithium        = db->value( 6 ).toString().toDouble();
   magnesium      = db->value( 7 ).toString().toDouble();
   calcium        = db->value( 8 ).toString().toDouble();

   q.clear();
   q << "get_spectrum" << analyteID << "Analyte" << "'Extinction";

   db->query( q );

   while ( db->next() )
   {
      double lambda = db->value( 1 ).toDouble();
      double coeff  = db->value( 2 ).toDouble();
      extinction[ lambda ] = coeff;
   }

   q[ 3 ] = "Refraction";
   db->query( q );

   while ( db->next() )
   {
      double lambda = db->value( 1 ).toDouble();
      double coeff  = db->value( 2 ).toDouble();
      refraction[ lambda ] = coeff;
   }

   q[ 3 ] = "Fluorescence";
   db->query( q );

   while ( db->next() )
   {
      double lambda = db->value( 1 ).toDouble();
      double coeff  = db->value( 2 ).toDouble();
      fluorescence[ lambda ] = coeff;
   }

   return US_DB2::OK;
}

int US_Analyte::load_disk( const QString& guid )
{
   int error = US_DB2::NO_ANALYTE;  // Error by default
   
   QString path;

   if ( ! analyte_path( path ) )
   {
      message = QObject::tr ( "Could not create analyte directory" );
      return error;  
   }

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

               if ( a.value( "analyteGUID" ).toString() == guid ) found = true; 
               break;
            }
         }
      }

      file.close();

      if ( found ) return read_analyte( filename );
   }

   message =  QObject::tr ( "Could not find analyte guid" );
   return error;
}

int US_Analyte::read_analyte( const QString& filename )
{
   QFile file( filename );

   // Read in the filename and populate class
   if ( ! file.open( QIODevice::ReadOnly | QIODevice::Text) )
   {
      qDebug() << "Cannot open file " << filename;
      message = QObject::tr( "Could not open analyte file for reading" );
      return US_DB2::ERROR;
   }

   double               freq;
   double               value;
   QString              type_string;
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

            type_string = a.value( "type" ).toString();

            // Set description and guid
            description = a.value( "description" ).toString();
            analyteGUID = a.value( "analyteGUID" ).toString();

            // Set type
            if ( type_string == "PROTEIN" )
            {
              vbar20 = a.value( "vbar20" ).toString().toDouble();
              type = PROTEIN;
            }

            else if (  type_string == "DNA"  ||  type_string == "RNA" )
            {
              type = ( type_string == "DNA" ) ? DNA : RNA;
              vbar20    = a.value( "vbar20"    ).toString().toDouble();
            }

            else
              type = CARBOHYDRATE;
         }

         else if ( xml.name() == "sequence" )
         {
            sequence = xml.readElementText();

            // Set mw
            if ( type == PROTEIN )
            {
               US_Math2::Peptide p;
               US_Math2::calc_vbar( p, sequence, NORMAL_TEMP );
               mw = p.mw;

               // The sequence tag comes before the extinction extinction tag
               // so a value set there will override this setting, if it
               // exists.  It's not the way xml is really supposed work, but it
               // will be ok in this case.

               extinction[ 280.0 ] = p.e280; 
            }
            else if ( type == DNA  ||  type == RNA )
               mw = nucleotide( a, sequence );

            else // CARBOHYDRATE
               mw = 0.0;
         }
         else if ( xml.name() == "extinction" )
         {
            QXmlStreamAttributes a = xml.attributes();
            freq  = a.value( "frequency" ).toString().toDouble();
            value = a.value( "value"     ).toString().toDouble();
            extinction[ freq ] = value;
         }

         else if ( xml.name() == "refraction" )
         {
            QXmlStreamAttributes a = xml.attributes();
            freq  = a.value( "frequency" ).toString().toDouble();
            value = a.value( "value"     ).toString().toDouble();
            refraction[ freq ] = value;
         }

         else if ( xml.name() == "fluorescence" )
         {
            QXmlStreamAttributes a = xml.attributes();
            freq  = a.value( "frequency" ).toString().toDouble();
            value = a.value( "value"     ).toString().toDouble();
            fluorescence[ freq ] = value;
         }
      }
   }

   return US_DB2::OK;
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

   bool isDNA     = ( a.value( "type"               ).toString() == "DNA");
   doubleStranded = ( a.value( "stranded"           ).toString() == "T" );
   complement     = ( a.value( "complement_only"    ).toString() == "T" );
   _3prime        = ( a.value( "ThreePrimeHydroxyl" ).toString() == "T" );
   _5prime        = ( a.value( "FivePrimeHydroxyl"  ).toString() == "T" );

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
   
   sodium    = a.value( "sodium"    ).toString().toDouble();
   potassium = a.value( "potassium" ).toString().toDouble();
   lithium   = a.value( "lithium"   ).toString().toDouble();
   magnesium = a.value( "magnesium" ).toString().toDouble();
   calcium   = a.value( "calcium"   ).toString().toDouble();

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
      if ( ! dir.mkpath( path ) ) 
      {
         return false;
      }
   }

   return true;
}

int US_Analyte::write( 
      bool           db_access, 
      const QString& filename,
      US_DB2*        db )
{
   if ( db_access ) return write_db  ( db );
   else             return write_disk( filename );
}

int US_Analyte::write_disk( const QString& filename )
{
   QFile file( filename );

   if ( ! file.open( QIODevice::WriteOnly | QIODevice::Text) )
   {
      qDebug() << "Cannot open file for writing: " << filename;
      message = QObject::tr( "Cannot open file for writing" );
      return US_DB2::ERROR;
   }

   QXmlStreamWriter xml( &file );
   xml.setAutoFormatting( true );

   xml.writeStartDocument();
   xml.writeDTD         ( "<!DOCTYPE US_Analyte>" );
   xml.writeStartElement( "AnalyteData" );
   xml.writeAttribute   ( "version", "1.0" );

   xml.writeStartElement( "analyte" );

   // Set attributes depending on type
   QString b; // bool

   switch ( type )
   {
      case US_Analyte::PROTEIN:
      {
         US_Math2::Peptide p;
         US_Math2::calc_vbar( p, sequence, NORMAL_TEMP );

         xml.writeAttribute( "type",  "PROTEIN" );
         xml.writeAttribute( "vbar20", QString::number( p.vbar20 ) );
      }
         break;

      case US_Analyte::DNA:
      case US_Analyte::RNA:
         if ( type == US_Analyte::DNA )
            xml.writeAttribute( "type", "DNA" );
         else
            xml.writeAttribute( "type", "RNA" );

         b = ( doubleStranded ) ? "T" : "F";
         xml.writeAttribute( "stranded", b );

         b = ( complement ) ? "T" : "F";
         xml.writeAttribute( "complement_only", b );

         b = ( _3prime ) ? "T" : "F";
         xml.writeAttribute( "ThreePrimeHydroxyl", b );

         b = ( _5prime ) ? "T" : "F";
         xml.writeAttribute( "FivePrimeHydroxyl", b );

         xml.writeAttribute( "sodium",    QString::number( sodium    ) );
         xml.writeAttribute( "potassium", QString::number( potassium ) );
         xml.writeAttribute( "lithium",   QString::number( lithium   ) );
         xml.writeAttribute( "magnesium", QString::number( magnesium ) );
         xml.writeAttribute( "calcium",   QString::number( calcium   ) );
         xml.writeAttribute( "vbar20",    QString::number( vbar20    ) );
         break;

      case US_Analyte::CARBOHYDRATE:
         xml.writeAttribute( "type", "CARB" );
         break;
   }

   xml.writeAttribute( "description", description );
   xml.writeAttribute( "analyteGUID", analyteGUID );

   xml.writeStartElement( "sequence" );
   xml.writeCharacters( "\n" );

   for ( int i = 0; i < sequence.length() / 80; i++ )
      xml.writeCharacters( sequence.mid( i * 80, 80 ) + "\n" );

   if ( sequence.length() % 80 > 0 )
       xml.writeCharacters( sequence.mid( ( sequence.length() / 80 ) * 80 ) );

   xml.writeCharacters( "\n" );
   xml.writeEndElement(); // sequence

   // Add extinction values
   QList< double > keys = extinction.keys();
   double          freq;
   double          value;

   for ( int i = 0; i < keys.size(); i++ )
   {
      freq  =             keys[ i ];
      value = extinction[ keys[ i ] ];

      xml.writeStartElement( "extinction" );
      xml.writeAttribute( "frequency",  QString::number( freq , 'f', 1 ) );
      xml.writeAttribute( "value",      QString::number( value ) );
      xml.writeEndElement(); // extinction
   }

   // Add refraction values
   keys = refraction.keys();

   for ( int i = 0; i < keys.size(); i++ )
   {
      freq  =             keys[ i ];
      value = refraction[ keys[ i ] ];

      xml.writeStartElement( "refraction" );
      xml.writeAttribute( "frequency",  QString::number( freq , 'f', 1 ) );
      xml.writeAttribute( "value",      QString::number( value ) );
      xml.writeEndElement(); // refraction
   }

   // Add fluorescence values
   keys = fluorescence.keys();

   for ( int i = 0; i < keys.size(); i++ )
   {
      freq  =             keys[ i ];
      value = fluorescence[ keys[ i ] ];

      xml.writeStartElement( "fluorescence" );
      xml.writeAttribute( "frequency",  QString::number( freq , 'f', 1 ) );
      xml.writeAttribute( "value",      QString::number( value ) );
      xml.writeEndElement(); // fluorescence
   }

   xml.writeEndElement(); // analyte
   xml.writeEndDocument();
   file.close();

   file.close();

   return US_DB2::OK;
}

void US_Analyte::set_spectrum( US_DB2* db )
{
   QStringList q;

   q << "delete_spectrum" << analyteID << "Analyte" << "Extinction";
   db->statusQuery( q );
   q[ 3 ] = "Refraction";
   db->statusQuery( q );
   q[ 3 ] = "Fluorescence";
   db->statusQuery( q );

   QList< double > keys = extinction.keys();

   q.clear();
   q << "new_spectrum" << analyteID << "Analyte" << "Extinction" << "" << "";

   for ( int i = 0; i < keys.size(); i++ )
   {
      double key =  keys[ i ];
      QString lambda = QString::number( key, 'f', 1 );
      q[ 4 ] = lambda;

      QString coeff = QString::number( extinction[ key ] );
      q[ 5 ] = coeff;

      db->statusQuery( q );
   }

   keys = refraction.keys();

   q[ 3 ] = "Refraction";

   for ( int i = 0; i < keys.size(); i++ )
   {
      double key =  keys[ i ];
      QString lambda = QString::number( key, 'f', 1 );
      q[ 4 ] = lambda;

      QString coeff = QString::number( refraction[ key ] );
      q[ 5 ] = coeff;

      db->statusQuery( q );
   }

   keys = fluorescence.keys();

   q[ 3 ] = "Fluorescence";

   for ( int i = 0; i < keys.size(); i++ )
   {
      double key =  keys[ i ];
      QString lambda = QString::number( key, 'f', 1 );
      q[ 4 ] = lambda;

      QString coeff = QString::number( fluorescence[ key ] );
      q[ 5 ] = coeff;

      db->statusQuery( q );
   }
}

int US_Analyte::write_db( US_DB2* db )
{
   QStringList q;
   bool        insert = true;

   message = QObject::tr( "inserted into" );

   if ( analyteGUID.size() != 36 ) 
   {
      message = QObject::tr ( "The analyte GUID is invalid" );  
      return US_DB2::BADGUID;
   }
   
   q << "new_analyte" << analyteGUID;

   // Check that the guid exists in the db
   QStringList q2;

   q2 << "get_analyteID" << analyteGUID;

   db->query( q2 );

   if ( db->lastErrno() == US_DB2::OK )
   {
      db->next();
      analyteID = db->value( 0 ).toString();
      q[ 0 ] = "update_analyte";
      q[ 1 ] = analyteID;
      message = QObject::tr( "updated in" );
      insert = false;
   }
      
   // Finish populating the query
   if      ( type == US_Analyte::PROTEIN ) q << "Protein";
   else if ( type == US_Analyte::DNA     ) q << "DNA";
   else if ( type == US_Analyte::RNA     ) q << "RNA";
   else                                    q << "Other";

   q << sequence;
   q << QString::number( vbar20 );
   q << description;

   QString spectrum = "";  // Unused element
   q << spectrum;
   q << QString::number( mw );

   db->statusQuery( q );

   int error = db->lastErrno();
   if ( error != US_DB2::OK ) 
   {
      message = QObject::tr ( "Could not update the DB" );
      return error;
   }

   if ( insert ) analyteID = QString::number( db->lastInsertID() );
   
   if ( type == US_Analyte::DNA  || type == US_Analyte::RNA )
      write_nucleotide( db );

   set_spectrum( db );
   
   return US_DB2::OK;
}

void US_Analyte::write_nucleotide( US_DB2* db )
{
   QStringList q;
   q << "set_nucleotide" << analyteID;
   q << QString::number( doubleStranded );
   q << QString::number( complement );
   q << QString::number( _3prime );
   q << QString::number( _5prime );
   q << QString::number( sodium );
   q << QString::number( potassium );
   q << QString::number( lithium );
   q << QString::number( magnesium );
   q << QString::number( calcium );

   db->statusQuery( q );
}
