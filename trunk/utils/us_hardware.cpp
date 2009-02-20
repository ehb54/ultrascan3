//! \file us_hardware.cpp
#include "us_hardware.h"

bool US_Hardware::readCenterpieceInfo( QList< struct centerpieceInfo >& cp_list )
{
   QString home = qApp->applicationDirPath().remove( QRegExp( "/bin$" ) );

   QFile cp_file( home + "/etc/centerpiece.dat");

   if ( cp_file.open( QIODevice::ReadOnly ) )
   {
      QTextStream ts( &cp_file );
      
      while ( ! ts.atEnd() )
      {
         QString s = ts.readLine();          // Process line by line

         bool                   ok;
         struct centerpieceInfo cp;
         
         cp.serial_number = US_Util::getToken( s, " " ).toInt( &ok );
         
         // If the first token is not an integer, 
         // we got a comment and we'll skip this line
         if ( ok )  
         {
            cp.material = US_Util::getToken( s, " " );
            cp.channels = US_Util::getToken( s, " ").toInt();;

            for ( uint i = 0; i < cp.channels; i++ )
               cp.bottom_position[ i ] = US_Util::getToken( s, " " ).toFloat();

            cp.sector     = US_Util::getToken( s, " " ).toInt();
            cp.pathlength = US_Util::getToken( s, " " ).toFloat();
            cp.angle      = US_Util::getToken( s, " " ).toFloat();
            cp.width      = US_Util::getToken( s, " " ).toFloat();
            
            cp_list << cp;
         }
      }

      cp_file.close();
      return true;
   }

   return false;
}

bool US_Hardware::readRotorInfo( QList< struct rotorInfo >& rotor_list )
{
   QString home = qApp->applicationDirPath().remove( QRegExp( "/bin$" ) );
   
   QFile rotor_file( home + "/etc/rotor.dat" );
   
   if ( rotor_file.open( QIODevice::ReadOnly ) )
   {
      QTextStream ts( &rotor_file );
      
      while ( ! ts.atEnd() )
      {
         QString s = ts.readLine();   // process line by line

         bool             ok;
         struct rotorInfo rotor;
         
         rotor.serial_number = US_Util::getToken( s, " " ).toInt( &ok);

         // If the first token is not an integer, 
         // we got a comment and we'll skip this line
         if ( ok )  
         {
            rotor.type = US_Util::getToken( s, " " );
            
            for ( int i = 0; i < 5; i++ )
               rotor.coefficient[ i ] =  US_Util::getToken( s, " " ).toFloat();

            rotor_list << rotor;
         }
      }

      rotor_file.close();
      return true;
   }

   return false;
}
