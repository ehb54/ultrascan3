//! \file us_2dsa_process.cpp

#include "us_2dsa_process.h"
#include "us_util.h"
#include "us_settings.h"

// class to process operatations on data:  upload/download/remove
US_2dsaProcess::US_2dsaProcess( US_DataIO2::EditedData* da_exper,
      QObject* parent /*=0*/ ) : QObject( parent )
{
   parentw          = parent;
   edata            = da_exper;

   dbg_level        = US_Settings::us_debug();
}

// find model filename matching a given GUID
QString US_2dsaProcess::get_model_filename( QString guid )
{
   QString fname = "";
   QString path;

   if ( ! US_Model::model_path( path ) )
      return fname;

   QDir f( path );
   QStringList filter( "M???????.xml" );
   QStringList f_names = f.entryList( filter, QDir::Files, QDir::Name );
   f_names.sort();

   int         nnames  = f_names.size();
   int         newnum  = nnames + 1;
   bool        found   = false;

   for ( int ii = 0; ii < nnames; ii++ )
   {
      QString fn = f_names[ ii ];
      int     kf = fn.mid( 1, 7 ).toInt() - 1;  // expected index in file name
      fn         = path + "/" + fn;             // full path file name

      if ( kf != ii  &&  newnum > nnames )
         newnum     = kf;                       // 1st opened number slot

      QFile m_file( fn );

      if ( ! m_file.open( QIODevice::ReadOnly | QIODevice::Text ) )
            continue;

      QXmlStreamReader xml( &m_file );

      while ( !xml.atEnd() )
      {
         xml.readNext();

         if ( xml.isStartElement()  &&  xml.name() == "model" )
         {
            QXmlStreamAttributes a = xml.attributes();

            if ( a.value( "modelGUID" ).toString() == guid )
            {
               fname    = fn;                   // name of file with match
               found    = true;                 // match to guid found
               break;
            }
         }

      }

      m_file.close();

      if ( found )
         break;
   }

 
   // if no guid match found, create new file name with a numeric part from
   //   the first gap in the file list sequence or from count plus one
   if ( ! found )
      fname     = path + "/M" + QString().sprintf( "%07i", newnum ) + ".xml";

   return fname;
}

// find noise file name matching a given GUID
QString US_2dsaProcess::get_noise_filename( QString guid )
{
   QString fname = "";
   QString path  = US_Settings::dataDir() + "/noises";
   QDir    dir;

   if ( ! dir.exists( path ) )
   {
      if ( ! dir.mkpath( path ) )
         return fname;
   }

   QDir f( path );
   QStringList filter( "N???????.xml" );
   QStringList f_names = f.entryList( filter, QDir::Files, QDir::Name );
   f_names.sort();

   int         nnames  = f_names.size();
   int         newnum  = nnames + 1;
   bool        found   = false;

   for ( int ii = 0; ii < nnames; ii++ )
   {
      QString fn = f_names[ ii ];
      int     kf = fn.mid( 1, 7 ).toInt() - 1;  // expected index in file name
      fn         = path + "/" + fn;             // full path file name

      if ( kf != ii  &&  newnum > nnames )
         newnum     = kf;                       // 1st opened number slot

      QFile m_file( fn );

      if ( ! m_file.open( QIODevice::ReadOnly | QIODevice::Text ) )
            continue;

      QXmlStreamReader xml( &m_file );

      while ( !xml.atEnd() )
      {
         xml.readNext();

         if ( xml.isStartElement()  &&  xml.name() == "noise" )
         {
            QXmlStreamAttributes a = xml.attributes();

            if ( a.value( "noiseGUID" ).toString() == guid )
            {
               fname    = fn;                   // name of file with match
               found    = true;                 // match to guid found
               break;
            }
         }

      }

      m_file.close();

      if ( found )
         break;
   }

 
   // if no guid match found, create new file name with a numeric part from
   //   the first gap in the file list sequence or from count plus one
   if ( ! found )
      fname     = path + "/N" + QString().sprintf( "%07i", newnum ) + ".xml";

   return fname;
}

