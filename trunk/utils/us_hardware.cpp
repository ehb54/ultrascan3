//! \file us_hardware.cpp
#include "us_hardware.h"

bool US_Hardware::readCenterpieceInfo( QVector< CenterpieceInfo >& cp_list )
{
   QString home = qApp->applicationDirPath().remove( QRegExp( "/bin$" ) );

   QFile cp_file( home + "/etc/centerpiece.dat");

   if ( cp_file.open( QIODevice::ReadOnly ) )
   {
      QTextStream ts( &cp_file );
      
      while ( ! ts.atEnd() )
      {
         QString s = ts.readLine();          // Process line by line

         bool            ok;
         CenterpieceInfo cp;
         
         cp.serial_number = US_Util::getToken( s, " " ).toInt( &ok );
         
         // If the first token is not an integer, 
         // we got a comment and we'll skip this line
         if ( ok )  
         {
            cp.material = US_Util::getToken( s, " " );
            cp.channels = US_Util::getToken( s, " ").toInt();;

            for ( int i = 0; i < cp.channels; i++ )
               cp.bottom_position[ i ] = US_Util::getToken( s, " " ).toFloat();

            cp.sector     = US_Util::getToken( s, " " ).toInt();
            cp.pathlength = US_Util::getToken( s, " " ).toFloat();
            cp.angle      = US_Util::getToken( s, " " ).toFloat();
            cp.width      = US_Util::getToken( s, " " ).toFloat();
            
            cp_list .push_back( cp );
         }
      }

      cp_file.close();
      return true;
   }

   return false;
}

bool US_Hardware::readRotorInfo( QVector< RotorInfo >& rotor_list )
{
   QString home = qApp->applicationDirPath().remove( QRegExp( "/bin$" ) );
   
   QFile rotor_file( home + "/etc/rotor.dat" );
   
   if ( rotor_file.open( QIODevice::ReadOnly ) )
   {
      QTextStream ts( &rotor_file );
      
      while ( ! ts.atEnd() )
      {
         QString s = ts.readLine();   // process line by line

         bool      ok;
         RotorInfo rotor;
         
         rotor.serial_number = US_Util::getToken( s, " " ).toInt( &ok);

         // If the first token is not an integer, 
         // we got a comment and we'll skip this line
         if ( ok )  
         {
            rotor.type = US_Util::getToken( s, " " );
            
            for ( int i = 0; i < 5; i++ )
               rotor.coefficient[ i ] =  US_Util::getToken( s, " " ).toFloat();

            rotor_list .push_back( rotor );
         }
      }

      rotor_file.close();
      return true;
   }

   return false;
}

bool US_Hardware::readRotorMap( QMap< QString, QString >& rotor_map )
{
   bool    ok    = false;
   QString fname = qApp->applicationDirPath().remove( QRegExp( "/bin$" ) )
                   + "/etc/rotor.xml";

   QFile filei( fname );

   if ( filei.open( QIODevice::ReadOnly | QIODevice::Text ) )
   {
      QXmlStreamReader xml( &filei );
      rotor_map.clear();

      while ( ! xml.atEnd() )
      {
         xml.readNext();

         if ( xml.isStartElement()  &&  xml.name() == "rotor" )
         {
            QXmlStreamAttributes a = xml.attributes();

            QString serial      = a.value( "serial"  ).toString();
            QString type        = a.value( "type"    ).toString();
            QString stretch     = a.value( "stretch" ).toString();

            rotor_map[ serial ] = type + ":" + stretch;
         }
      }

      ok        = true;
   }

   return ok;
}

bool US_Hardware::readRotorMap( US_DB2* db,
                                QMap< QString, QString >& rotor_map )
{
   bool    ok    = false;

   if ( db == 0  ||  ! db->isConnected() )
      return readRotorMap( rotor_map );

   QStringList query;
   QStringList rotorIDs;
   QStringList locNames;

   query << "get_experiment_desc" << "0";
   db->query( query );
   db->next();

   QString labID   = db->value( 3 ).toString();

   query.clear();
   query << "get_rotor_names" << labID;
   db->query( query );

   while ( db->next() )
   {
      rotorIDs << db->value( 0 ).toString();
      locNames << db->value( 1 ).toString();
   }

   for ( int ii = 0; ii < rotorIDs.size(); ii++ )
   {
      QString rotorID     = rotorIDs.at( ii );
      QString locName     = locNames.at( ii );

      query.clear();
      query << "get_rotor_info" << rotorID;
      db->query( query );
      db->next();

      QString rotorName   = db->value( 1 ).toString();
      QString serial      = db->value( 2 ).toString();
      QString stretch     = db->value( 3 ).toString();
      QString type        = !locName.isEmpty() ? locName : rotorName;

      rotor_map[ serial ] = type + ":" + stretch;
      ok                  = !serial.isEmpty() ? true : ok;
   }

   return ok;
}

bool US_Hardware::rotorValues( QString serial,
      QMap< QString, QString >rotor_map, QString& type, double* rotcoeffs )
{
   bool ok = rotor_map.contains( serial );

   if ( ok )
   {
      QString rotval = rotor_map[ serial ];
      type           = rotval.section( ":", 0, 0 ).simplified();
      QString coeffs = rotval.section( ":", 1, 1 ).simplified();

      for ( int ii = 0; ii < 5; ii++ )
         rotcoeffs[ ii ] = coeffs.section( " ", ii, ii ).toDouble();

   }

   return ok;
}


