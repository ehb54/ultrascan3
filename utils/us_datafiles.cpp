//! \file us_datafiles.cpp

#include "us_datafiles.h"

// Get a data file name; either by matching the GUID,
//  finding a numeric gap in existing file names,
//  or bumping to the next number past the last used.
//    path     = path to data files directory (e.g., "dataDir()/models")
//    guid     = GUID value to match
//    lfchar   = leading file character ("M", "N", ...)
//    lkupTag  = look-up tag ("model", "noise", ...)
//    lkupAtt  = look-up attribute ("guid", "modelGUID", ...)
//    newFile  = returned new-file flag
//    (return:)  full path name for new or old file to write
QString US_DataFiles::get_filename( const QString& path, const QString& guid,
      const QString& lfchar, const QString& lkupTag, const QString& lkupAtt,
      bool& newFile )
{
   QDir f( path );
   QStringList filter( lfchar + "???????.xml" );
   QStringList f_names = f.entryList( filter, QDir::Files, QDir::Name );
   f_names.sort();
   newFile             = true;   // Assume file will be new
   QString     ofname  = "";     // Start with empty output file name
   int         numFile = 1;      // Default number if no files exist

   for ( int ii = 0; ii < f_names.size(); ii++ )
   {  // Browse all the existing files in the directory
      QString fname  = f_names[ ii ];

      if ( !guid.isEmpty() )
      {  // If guid is not empty, search for a match in existing files
         QFile m_file( path + "/" + fname );

         if ( ! m_file.open( QIODevice::ReadOnly | QIODevice::Text) ) continue;

         QXmlStreamReader xml( &m_file );

         while ( ! xml.atEnd() )
         {  // Search for a matching tag and attribute
            xml.readNext();

            if ( xml.isStartElement() )
            {
               if ( xml.name() == lkupTag )
               {  // Found the look-up tag
                  QXmlStreamAttributes a = xml.attributes();

                  if ( a.value( lkupAtt ).toString() == guid )
                  {  // There is a match of an attribute value to the GUID
                     ofname    = fname;       // File name will be this one
                     newFile   = false;       // Not a new file
                     break;                   // We're done looking
                  }
               }
            }
         }

         m_file.close();
      }

      if ( newFile )
      {  // No match yet found:  look for a gap in numbering
         if ( ofname.isEmpty() )
         {  // No gap was previously found
            int numCurr = fname.mid( 1, 7 ).toInt();  // Current file numeric
            numFile     = ii + 1;                     // Expected file numeric

            if ( numCurr > numFile )  // There is a gap: use missing number name
               ofname  = lfchar + QString().asprintf( "%07i", numFile ) + ".xml";
         }
      }

      else  // A match was found in XML, so break from the file loop
         break;

   }

   if ( ofname.isEmpty() )
   {  // No match and no numeric gap, so bump to number past last existing file
      if ( f_names.size() > 0 )
         numFile     = f_names.last().mid( 1, 7 ).toInt() + 1;
      ofname      = lfchar + QString().asprintf( "%07i", numFile ) + ".xml";
   }

   return path + "/" + ofname;
}

// Get a data file name. This version has no newFile return argument.
//    path     = path to data files directory (e.g., "dataDir()/models")
//    guid     = GUID value to match
//    lfchar   = leading file character ("M", "N", ...)
//    lkupTag  = look-up tag ("model", "noise", ...)
//    lkupAtt  = look-up attribute ("guid", "modelGUID", ...)
//    (return:)  full path name for new or old file to write
QString US_DataFiles::get_filename( const QString& path, const QString& guid,
      const QString& lfchar, const QString& lkupTag, const QString& lkupAtt )
{
   bool newf = false;
   return get_filename( path, guid, lfchar, lkupTag, lkupAtt, newf );
}

