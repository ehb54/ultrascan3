//! \file us_convert.cpp

#include "us_gui_settings.h"
#include "us_math2.h"
#include "us_util.h"
#include "us_convert.h"
#include "us_convertio.h"

#ifdef WIN32
  #define round(x) floor( (x) + 0.5 )
#endif

// Generic constructor
US_Convert::US_Convert( void )
{
}

void US_Convert::readLegacyData(
     QString                              dir,
     QList< US_DataIO::BeckmanRawScan >&  rawLegacyData,
     QString&                             runType )
{
   if ( dir.isEmpty() ) return;

   // Get legacy file names and set channels
   QDir d( dir, "*", QDir::Name, QDir::Files | QDir::Readable );
   d.makeAbsolute();
   if ( dir.right( 1 ) != "/" ) dir += "/"; // Ensure trailing /

   QStringList files = d.entryList( QDir::Files );
qDebug() << "CVT:rdLegD: dir" << dir << "f sz" << files.size();

   // Maybe dir had only directories ( i.e., not empty )
   if ( files.size() < 1 ) return;

   runType = files[ 0 ].right( 3 ).left( 2 ).toUpper(); // 1st 2 chars of extention
qDebug() << "CVT:rdLegD: runType" << runType;
   QStringList fileList;
   QStringList channels;
   QString f;
   QString arunType = runType;
   bool mixed_type = false;

   foreach ( f, files )
   {
      // Look for a proper filename match:
      // Optional channel + 4 to 6 digits + dot + file type + cell number

      QRegExp rx( "^[A-J]?\\d{4,6}\\.(?:RA|RI|IP|FI|WA|WI)\\d$" );

      if ( rx.indexIn( f.toUpper() ) >= 0 )
      {
         fileList << f;

         // Parse the filtered file list to determine cells and channels
         QChar c = f.at( 0 ).toUpper();
         if ( c.isLetter() && ! channels.contains( c ) )
            channels << c;

         // Test to see if the directory holds mixed types
         QString frunType = f.right( 3 ).left( 2 ).toUpper();
qDebug() << "CVT:rdLegD:    f" << f << "frunType" << frunType << "c" << c;
         if ( frunType != runType )
         {
            mixed_type     = true;
            arunType       = frunType;
//qDebug() << "CVT: MIXED : runType frunType" << runType << frunType;
         }
      }
   }
qDebug() << "CVT:rdLegD: mixed" << mixed_type;

   if ( mixed_type )
   {  // If mixed type, have user select one type and rebuild appropriate lists
      QString prType = QObject::tr( "Absorbance" );
      QString arType = QObject::tr( "Absorbance" );

      if ( runType == "RI"  ||  runType == "WI" )
         prType      = QObject::tr( "Intensity" );
      else if ( runType == "IP" )
         prType      = QObject::tr( "Interference" );
      else if ( runType == "FI"  )
         prType      = QObject::tr( "Fluorensce" );

      if ( arunType == "RI"  ||  arunType == "WI" )
         arType      = QObject::tr( "Intensity" );
      else if ( arunType == "IP" )
         arType      = QObject::tr( "Interference" );
      else if ( arunType == "FI"  )
         arType      = QObject::tr( "Fluorensce" );

      int status = QMessageBox::information( 0,
            QObject::tr( "Mixed Import Data Types" ),
            QObject::tr( "The Import directory holds multiple data types.\n"
                         "Choose the type to import in this session." ),
            prType, arType, 0, 0, 1 );

      if ( status != 0 )
         runType        = arunType;
qDebug() << "CVT:rdLegD:  runTypes chosen" << prType << arType << runType << "status" << status;

      fileList.clear();
      channels.clear();

      foreach ( f, files )
      {
         // Look for a proper filename match:
         // Optional channel + 4 to 6 digits + dot + file type + cell number

         QRegExp rx( "^[A-J]?\\d{4,6}\\.(?:RA|RI|IP|FI|WA|WI)\\d$" );

         if ( rx.indexIn( f.toUpper() ) >= 0 )
         {
            QString frunType = f.right( 3 ).left( 2 ).toUpper();
            if ( frunType != runType )
               continue;

            fileList << f;

            // Parse the filtered file list to determine cells and channels
            QChar c = f.at( 0 ).toUpper();
            if ( c.isLetter() && ! channels.contains( c ) )
               channels << c;
         }
      }
   }

   if ( runType == "FI" )
   {  // For flourescence data, resort the file list by cell, then channel
      QStringList oldfList = fileList;
      fileList.clear();
      QStringList fexts;   // List of possible flourescence extensions
      fexts << "FI1" << "FI2" << "FI3" << "FI4"
            << "FI5" << "FI6" << "FI7" << "FI8"
            << "fi1" << "fi2" << "fi3" << "fi4"
            << "fi5" << "fi6" << "fi7" << "fi8";
      int         ntotf    = oldfList.size();
      int         ktotf    = 0;
      QString     fext;
      QString     fname;

      foreach ( fext, fexts )
      {  // Sort by file extension so cell is the first order
         foreach( fname, oldfList )
         {
            if ( fname.endsWith( fext ) )
            {
               fileList << fname;
               ktotf++;
            }
         }

         if ( ktotf >= ntotf )  break;
      }
   }

   if ( channels.isEmpty() ) channels << "A";

   // Now read the data.
   for ( int i = 0; i < fileList.size(); i++ )
   {
      US_DataIO::BeckmanRawScan data;
      US_DataIO::readLegacyFile( dir + fileList[ i ], data );

      // Add channel
      QChar c = fileList[ i ].at( 0 );  // Get 1st character

      data.channel = ( c.isDigit() ) ? 'A' : (uchar)c.unicode();

      if ( runType == "RI" )          // Split out the two readings in RI data
      {
         US_DataIO::BeckmanRawScan data2 = data;  // Alter to store 2nd dataset
         for ( int j = 0; j < data.rvalues.size(); j++ )
         {
            data2.rvalues[ j ] = data2.stddevs[ j ]; // Reading 2 here for RI
            data .stddevs[ j ] = 0.0;
            data2.stddevs[ j ] = 0.0;
         }

         data2.channel = 'B';

         rawLegacyData << data;
         rawLegacyData << data2;
      }

      else
      {
         rawLegacyData << data;
      }
   }
}

void US_Convert::convertLegacyData(
     QList< US_DataIO::BeckmanRawScan >& rawLegacyData,
     QVector< US_DataIO::RawData  >&     rawConvertedData,
     QList< TripleInfo >&     triples,
     QString                              runType,
     double                               tolerance
     )
{
   setTriples( rawLegacyData, triples, runType, tolerance );

   US_DataIO::RawData newRawData;     // filtered legacy data in new raw format

   rawConvertedData.clear();

   if ( triples.size() == 1 )
   {
      convert( rawLegacyData, newRawData, triples[ 0 ].tripleDesc,
               runType, tolerance );

      rawConvertedData << newRawData;
   }

   else
   {
      // Now convert the data.
      for ( int i = 0; i < triples.size(); i++ )
      {
         // Convert data for this cell / channel / wavelength
         convert( rawLegacyData, newRawData, triples[ i ].tripleDesc,
                  runType, tolerance );
qDebug() << "Cvt:cvLD: i, trip" << i << triples[i].tripleDesc;

         // and save it
         rawConvertedData << newRawData;
      }
   }
}

int US_Convert::saveToDisk(
    QVector< US_DataIO::RawData* >& rawConvertedData,
    QList< TripleInfo >& triples,
    QVector< Excludes >& allExcludes,
    QString runType,
    QString runID,
    QString dirname,
    bool saveGUIDs
    )
{
   if ( rawConvertedData[ 0 ]->scanData.empty() )
      return NODATA;

   // Write the data
   int status;

   // Make sure directory is empty
   QDir d( dirname );
   QStringList rmvfilt( "*.auc" );
   QStringList rmvfiles = d.entryList( rmvfilt, QDir::Files, QDir::Name );
   for ( int ii = 0; ii < rmvfiles.size(); ii++ )
      if ( ! d.remove( rmvfiles[ ii ] ) )
         qDebug() << "Unable to remove file" << rmvfiles[ ii ];

   QString wavelnp = "";

   for ( int i = 0; i < triples.size(); i++ )
   {
      if ( triples[ i ].excluded ) continue;

      QString     triple     = triples[ i ].tripleDesc;
      QStringList parts      = triple.split(" / ");

      QString     cell       = parts[ 0 ];
      QString     channel    = parts[ 1 ];
      QString     filename;

      if ( runType == "WA" )
      {
          double r       = parts[ 2 ].toDouble() * 1000.0;
          QString radius = QString::number( (int) round( r ) );
          filename       = runID      + "."
                         + runType    + "."
                         + cell       + "."
                         + channel    + "."
                         + radius     + ".auc";
      }

      else
      {
          QString wavelength = parts[ 2 ];
          if ( wavelength.contains( "." ) )
          {
             double dwlen    = wavelength.toDouble();
             wavelength      = QString::number( qRound( dwlen ) );
             if ( wavelength == wavelnp )
                wavelength      = wavelength + "b";
          }
          wavelnp            = wavelength;
          filename           = runID      + "."
                             + runType    + "."
                             + cell       + "."
                             + channel    + "."
                             + wavelength + ".auc";
      }

      // Let's see if there is a triple guid already (from a previous save)
      // Otherwise the rawGUID characters should already be initialized to 0
      QString uuidc = US_Util::uuid_unparse(
            (unsigned char*) rawConvertedData[ i ]->rawGUID );

      if ( saveGUIDs && uuidc != "00000000-0000-0000-0000-000000000000" )
      {
         // Make sure xml file matches
         memcpy( triples [ i ].tripleGUID,
                 (char*) rawConvertedData[ i ]->rawGUID, 16 );
      }

      else
      {
         // Calculate and save the guid for this triple
         uchar uuid[ 16 ];
         QString uuid_string = US_Util::new_guid();
         US_Util::uuid_parse( uuid_string, uuid );
         memcpy( rawConvertedData[ i ]->rawGUID,   (char*) uuid, 16 );
         memcpy( triples         [ i ].tripleGUID, (char*) uuid, 16 );
      }

      // Same with solutionGUID
      QRegExp rx( "^[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}$" );
      if ( ( triples[ i ].solution.saveStatus == US_Solution::NOT_SAVED  ) ||
           ( ! rx.exactMatch( triples[ i ].solution.solutionGUID )       ) )
      {
         qDebug() << "It is not saving";
         triples[ i ].solution.solutionGUID = "";
         triples[ i ].solution.solutionDesc = "";
      }

      // Make sure solution is saved to disk
      if ( ! triples[ i ].solution.solutionGUID.isEmpty() )
         triples[ i ].solution.saveToDisk();

      // Save the filename of this triple
      triples[ i ].tripleFilename = filename;

      // Create a copy of the current dataset so we can alter it
      US_DataIO::RawData  currentData     = *rawConvertedData[ i ];
      Excludes currentExcludes = allExcludes     [ i ];

      // Now recopy scans, except for excluded ones
      currentData.scanData.clear();
      QVector< US_DataIO::Scan > sourceScans = rawConvertedData[ i ]->scanData;
      for ( int j = 0; j < sourceScans.size(); j++ )
      {
         if ( ! currentExcludes.contains( j ) )
            currentData.scanData << sourceScans[ j ];  // copy this scan
      }

      // Now write altered dataset
      status = US_DataIO::writeRawData( d.absoluteFilePath(filename) , currentData );

      if ( status !=  US_DataIO::OK ) break;
   }

   if ( status != US_DataIO::OK )
   {
      qDebug() << "Status is not ok";
      // Try to delete the files and tell the user
      return NOT_WRITTEN;
   }


   return OK;
}

int US_Convert::readUS3Disk( QString dir,
      QVector< US_DataIO::RawData >& rawConvertedData,
      QList< TripleInfo >& triples, QString& runType )
{
   rawConvertedData.clear();

   QStringList nameFilters = QStringList( "*.auc" );

   QDir d( dir );

   QStringList files =  d.entryList( nameFilters,
         QDir::Files | QDir::Readable, QDir::Name );

   if ( files.size() == 0 )
      return NODATA;

   // Get runType
   QStringList part = files[ 0 ].split( "." );
   runType = part[ 1 ];

   // Set up cell / channel / wavelength combinations
   triples.clear();
   for ( int i = 0; i < files.size(); i++ )
   {
      part.clear();
      part = files[ i ].split( "." );
      QString wl;
      if ( runType == "WA" )
         wl = QString::number( part[ 4 ].toDouble() / 1000.0 );
      else
         wl = part[ 4 ];

      TripleInfo t;

      t.tripleDesc = part[ 2 ] + " / " + part[ 3 ] + " / " + wl;
      t.excluded   = false;
      triples << t;
   }

   // Read all data
   QString file;
   foreach ( file, files )
   {
      QString filename = dir + file;
      US_DataIO::RawData data;

      int result = US_DataIO::readRawData( filename, data );
      if ( result != US_DataIO::OK )
         return NOAUC;

      rawConvertedData << data;
      data.scanData.clear();
   }

   return OK;
}

void US_Convert::convert(
     QList< US_DataIO::BeckmanRawScan >&  rawLegacyData,
     US_DataIO::RawData&                  newRawData,
     QString                              triple,
     QString                              runType,
     double                               tolerance )
{
   // Convert the data into the UltraScan3 data structure
qDebug() << "Cvt:cnvt: IN";
   QStringList parts      = triple.split(" / ");

   int         cell       = parts[ 0 ].toInt();
   char        channel    = parts[ 1 ].toLatin1()[ 0 ];
   double      wavelength = parts[ 2 ].toDouble();

   QList< US_DataIO::BeckmanRawScan > ccwLegacyData;

   // Get a list of the data that matches the cell / channel / wl
   ccwLegacyData.clear();
   newRawData.scanData.clear();

   for ( int i = 0; i < rawLegacyData.size(); i++ )
   {
      US_DataIO::BeckmanRawScan data = rawLegacyData[ i ];

      if ( data.cell == cell       &&
           data.channel == channel &&
           fabs ( data.rpoint - wavelength ) < tolerance )
         ccwLegacyData << data;
   }

   // Sort the list according to time.  Use a simple bubble sort
   for ( int i = 0; i < ccwLegacyData.size(); i++ )
   {
      for ( int j = i + 1; j < ccwLegacyData.size(); j++ )
      {
         if ( ccwLegacyData[ j ].seconds < ccwLegacyData[ i ].seconds )
            ccwLegacyData.swap( i, j );
      }
   }

   if ( ccwLegacyData.isEmpty() ) return;

   strncpy( newRawData.type, runType.toLatin1().constData(), 2 );
   memset( newRawData.rawGUID, 0, 16 );           // Initialize to 0's
   newRawData.cell        = cell;
   newRawData.channel     = channel;
   newRawData.description = ccwLegacyData[ 0 ].description;

   // Get the min and max radius
   double min_radius = 1.0e99;
   double max_radius = 0.0;
   int    max_size   = 0.0;

   // Calculate mins and maxes for proper scaling
   for ( int i = 0; i < rawLegacyData.size(); i++ )
   {
      double first = rawLegacyData[ i ].xvalues[ 0 ];
      int    size  = rawLegacyData[ i ].xvalues.size();
      double last  = rawLegacyData[ i ].xvalues[ size - 1 ];

      min_radius = qMin( min_radius, first );
      max_radius = qMax( max_radius, last  );
      max_size   = qMax( max_size,   size  );
   }

   // Set the distance between readings
   double delta_r;
qDebug() << "Cvt:  runType" << runType;
   if ( runType == "IP" )
   {
      // Get the actual delta out of the header lines
      QStringList descriptionParts = rawLegacyData[ 0 ]
            .description.split( " ", QString::SkipEmptyParts );
      QString proto = descriptionParts[ 1 ].toLatin1();
      proto.remove( "," );

      // Some IP data doesn't have this
      if ( proto.toDouble() == 0.0 )
         delta_r = ( max_radius - min_radius ) / ( max_size - 1 );

      else
         delta_r = proto.toDouble();
qDebug() << "Cvt:  proto" << proto << "delta_r" << delta_r;

   }

   else
      delta_r = 0.001;

   // Calculate the radius vector
   newRawData.xvalues .clear();
   newRawData.scanData.clear();
   int radius_count = (int) round( ( max_radius - min_radius ) / delta_r ) + 1;
qDebug() << "Cvt:  rad_cnt delta_r" << radius_count << delta_r;
   double radius = min_radius;
   for ( int j = 0; j < radius_count; j++ )
   {
      newRawData.xvalues << radius;
      radius += delta_r;
   }

   // Convert the scans
   for ( int i = 0; i < ccwLegacyData.size(); i++ )
   {
      // Start loading the data
      US_DataIO::Scan s;
      s.temperature = ccwLegacyData[ i ].temperature;
      s.rpm         = ccwLegacyData[ i ].rpm;
      s.seconds     = ccwLegacyData[ i ].seconds;
      s.omega2t     = ccwLegacyData[ i ].omega2t;
      s.wavelength  = ccwLegacyData[ i ].rpoint;
      s.delta_r     = delta_r;

      // Readings here and interpolated array
      int bitmap_size = ( radius_count + 7 ) / 8;
      uchar* interpolated = new uchar[ bitmap_size ];
      //bzero( interpolated, bitmap_size );
      memset( interpolated, 0, bitmap_size );
qDebug() << "Cvt:   sc i" << i << "interp set";

      /*
      There are two indexes needed here.  The new radius as iterated
      from min_radius to max_radius and the pointer to the current
      scan readings is j.

      The old scan reading is ccwLegacyData[ i ]->readings[ j ]

      If the current new radius is within 0.0003 of the i
      ccwLegacyData[ i ]->readings[ j ].d.radius
         copy ccwLegacyData[ i ]->readings[ j ].value into the new reading
         copy ccwLegacyData[ i ]->readings[ j ].stdDev into the new reading
         increment j

      If the current new radius is less than i
      ccwLegacyData[ i ]->readings[ 0 ].d.radius,
      then
         copy ccwLegacyData[ i ]->readings[ 0 ].value into the new reading
         set the std dev to 0.0.
         set the interpolated flag

      If the current new radius is greater than
      ccwLegacyData[ i ]->readings[ last() ].d.radius
         copy ccwLegacyData[ i ]->readings[ last ].value into the new reading
         set the std dev to 0.0.
         set the interpolated flag

      else
         interplate between ccwLegacyData[ i ]->readings[ j ] and
                            ccwLegacyData[ i ]->readings[ j -1 ]
         set the std dev to 0.0.
         set the interpolated flag

      Append the new reading and continue.
      */

      radius        = min_radius;
      int    rCount = ccwLegacyData[ i ].xvalues.size();
      double r0     = ccwLegacyData[ i ].xvalues[ 0 ];
      double rLast  = ccwLegacyData[ i ].xvalues[ rCount - 1 ];

      int    k      = 0;
      int    nnz    = 0;
qDebug() << "Cvt:  rad" << radius << "rCount" << rCount << radius_count;

      for ( int j = 0; j < radius_count; j++ )
      {
         double  rvalue;
         double  rstdev;
         double  dr    = 0.0;

         if ( k < rCount )
            dr      = radius - ccwLegacyData[ i ].xvalues[ k ];

         if ( runType == "IP" )
         {
            if ( dr > -3.0e-4 && k < rCount ) // No interpolation here
            {
               rvalue  = ccwLegacyData[ i ].rvalues[ k ];
               k++;
            }

            else if ( radius < r0 ) // Before the first
            {
               rvalue = ccwLegacyData[ i ].rvalues[ 0 ];
               setInterpolated( interpolated, j );
            }

            else if ( radius > rLast || k >= rCount ) // After the last
            {
               rvalue = ccwLegacyData[ i ].rvalues[ rCount - 1 ];
               setInterpolated( interpolated, j );
            }

            rstdev = 0.0;
         }

         else if ( dr > -3.0e-4   &&  k < rCount ) // A value
         {
            rvalue = ccwLegacyData[ i ].rvalues[ k ];
            rstdev = ccwLegacyData[ i ].nz_stddev ?
                     ccwLegacyData[ i ].stddevs[ k ] : 0.0;
//double xvk = ccwLegacyData[i].xvalues[k];
//if (xvk>=6.07 && xvk<=6.08)
// qDebug() << "Cvt:   j k" << j << k << "rvalue" << rvalue << "xvk" << xvk;
            k++;
            if ( rstdev != 0.0 )
               nnz++;
         }
         else if ( radius < r0 ) // Before the first
         {
            rvalue = ccwLegacyData[ i ].rvalues[ 0 ];
            rstdev = 0.0;
            setInterpolated( interpolated, j );
         }
         else if ( radius > rLast  ||  k >= rCount ) // After the last
         {
            rvalue = ccwLegacyData[ i ].rvalues[ rCount - 1 ];
            rstdev = 0.0;
            setInterpolated( interpolated, j );
         }
         else  // Interpolate the value
         {
            double dv = ccwLegacyData[ i ].rvalues[ k     ] -
                        ccwLegacyData[ i ].rvalues[ k - 1 ];

            double dR = ccwLegacyData[ i ].xvalues[ k     ] -
                        ccwLegacyData[ i ].xvalues[ k - 1 ];

            dr        = radius - ccwLegacyData[ i ].xvalues[ k - 1 ];

            rvalue    = ccwLegacyData[ i ].rvalues[ k - 1 ] + dr * dv / dR;
            rstdev    =  0.0;
//double xvk = ccwLegacyData[i].xvalues[k];
//if (xvk>=6.07 && xvk<=6.08)
// qDebug() << "Cvt:   j k" << j << k << "dv dR dr rvalue" << dv << dR << dr
//  << rvalue << "xvk rvk" << xvk << radius;

            setInterpolated( interpolated, j );
         }

         s.rvalues << rvalue;
         s.stddevs << rstdev;
         radius   += delta_r;
      }
qDebug() << "Cvt:   rCount loop complete";
      s.interpolated = QByteArray( (char*)interpolated, bitmap_size );
      delete [] interpolated;

      s.nz_stddev    = ( nnz > 0 );
      if ( nnz == 0 )
         s.stddevs.clear();

      newRawData.scanData <<  s ;
   }
qDebug() << "Cvt:cnvt: RTN";
}

// Subdivides existing RA triple into subsets
void US_Convert::splitRAData(
     QVector< US_DataIO::RawData >&  rawConvertedData,
     QList< TripleInfo >& triples,
     int                              currentTriple,
     QList< double >&                 subsets )
{
   // Create a place to store the old data temporarily
   QVector< US_DataIO::RawData >* oldData = new QVector< US_DataIO::RawData >;
   *oldData = rawConvertedData;
   rawConvertedData.clear();

   // A pointer to the individual old RawData records
   US_DataIO::RawData* oldRawData = oldData->data();

   QList< TripleInfo > oldTriples = triples;
   triples.clear();

   for ( int i = 0; i < oldData->size(); i++ )
   {
qDebug() << "Cvt:splRA: i cTrip" << i << currentTriple;
      if ( i != currentTriple )
      {
         // Copy this triple over as is
         rawConvertedData << oldRawData[ i ];
         triples << oldTriples[ i ];
      }

      else
      {
         // 4 limits define 3 regions
         for ( int j = 1; j < subsets.size(); j++ )
         {
            US_DataIO::RawData newRawData;

            // Modify the raw data information
            strncpy( newRawData.type, oldRawData[ i ].type, 2 );
            memset( newRawData.rawGUID, 0, 16 );
            newRawData.cell        = oldRawData[ i ].cell;
            newRawData.channel     = oldRawData[ i ].channel + (j-1) * 2;
            newRawData.description = oldRawData[ i ].description;

            // Copy the radius subset just once
            for ( int k = 0; k < oldRawData[ i ].xvalues.size(); k++ )
               if ( ( oldRawData[ i ].radius( k ) >= subsets[ j - 1 ] ) &&
                    ( oldRawData[ i ].radius( k ) <= subsets[ j ]     ) )
                  newRawData.xvalues << oldRawData[ i ].xvalues[ k ];

            // Now copy the parent scan information
            newRawData.scanData.clear();
            for ( int k = 0; k < oldRawData[ i ].scanData.size(); k++ )
               newRawData.scanData << newScanSubset( oldRawData[ i ].scanData[ k ],
                                                     oldRawData[ i ].xvalues,
                                                     subsets[ j - 1 ],
                                                     subsets[ j ] );

            // Add to the new data set
            rawConvertedData << newRawData;

            // Modify the triple information
            TripleInfo t = oldTriples[ i ];
            QStringList parts        = t.tripleDesc.split(" / ");
            QString     wavelength   = parts[ 2 ];

            t.tripleDesc = QString::number( newRawData.cell    ) + " / " +
                           QString        ( newRawData.channel ) + " / " +
                           wavelength;
qDebug() << "Cvt:splRA:  tDesc" << t.tripleDesc;

            // Avoid duplication of GUID's
            QString uuidc = US_Util::uuid_unparse( (unsigned char*)t.tripleGUID );
            if ( uuidc != "00000000-0000-0000-0000-000000000000" )
            {
               uchar uuid[ 16 ];
               QString uuid_string = US_Util::new_guid();
               US_Util::uuid_parse( uuid_string, uuid );
               memcpy( t.tripleGUID, (char*) uuid, 16 );
            }

            t.tripleFilename = "";

            triples << t;
         }

      }
   }

   // Finished with the old data now
   oldData->clear();
   delete oldData;

   // Renumber the triple ID's
   for ( int i = 0; i < triples.size(); i++ )
      triples[ i ].tripleID = i;
}

// Returns a new scan, but only with readings in a specified range
US_DataIO::Scan US_Convert::newScanSubset(
     US_DataIO::Scan& oldScan,
     QVector< double >& xvalues,
     double r_start,
     double r_end )
{
   US_DataIO::Scan s;

   s.temperature = oldScan.temperature;
   s.rpm         = oldScan.rpm;
   s.seconds     = oldScan.seconds;
   s.omega2t     = oldScan.omega2t;
   s.wavelength  = oldScan.wavelength;
   s.plateau     = oldScan.plateau;
   s.delta_r     = oldScan.delta_r;

   // Now copy the readings that are in this subset
   s.rvalues.clear();
   int first_reading = 0;
   for ( int i = 0; i < oldScan.rvalues.size(); i++ )
   {
      if ( ( xvalues[ i ] >= r_start ) &&
           ( xvalues[ i ] <= r_end   ) )
      {
         s.rvalues << oldScan.rvalues[ i ];  // copy this dataset point
         if ( first_reading == 0 ) first_reading = i;
      }
   }

   // Now copy the interpolation bitflags for this subset
   // They might not be on the same byte boundary as the originals
   int bitmap_size = ( s.rvalues.size() + 7 ) / 8;
   uchar* interpolated = new uchar[ bitmap_size ];
   memset( interpolated, 0, bitmap_size );

   for ( int i = first_reading; i < first_reading + s.rvalues.size(); i++ )
   {
      int byte = i / 8;
      int bit  = i % 8;
      int mask = 1 << ( 7 - bit );

      if ( ( oldScan.interpolated[ byte ] & mask ) != 0x00 )
         setInterpolated( interpolated, i - first_reading );
   }

   s.interpolated = QByteArray( (char*)interpolated, bitmap_size );
   delete [] interpolated;

   return s;
}

void US_Convert::setTriples(
     QList< US_DataIO::BeckmanRawScan >& rawLegacyData,
     QList< TripleInfo >&                triples,
     QString                             runType,
     double                              tolerance )
{
   // Wavelength data is handled differently here
   if ( runType == "WA" )
      setCcrTriples( rawLegacyData, triples, tolerance );
   else
      setCcwTriples( rawLegacyData, triples, tolerance );

}

void US_Convert::setCcwTriples(
     QList< US_DataIO::BeckmanRawScan >& rawLegacyData,
     QList< TripleInfo >&     triples,
     double                               tolerance )
{
   // Most triples are ccw
   triples.clear();

   // Get wavelengths
   QStringList wavelengths;

   for ( int i = 0; i < rawLegacyData.size(); i++ )
   {
      QString wl = QString::number( rawLegacyData[ i ].rpoint, 'f', 1 );
      wavelengths << wl;
   }

   // Merge wavelengths
   wavelengths.sort();

   QList< QList< double > > modes;
   QList< double >          mode;

   for ( int i = 0; i < wavelengths.size(); i++ )
   {
      double wl = wavelengths[ i ].toDouble();

      if ( ! mode.empty()  &&  fabs( mode.last() - wl ) > tolerance )
      {
         modes << mode;
         mode.clear();
      }

      mode << wl;
   }

   if ( mode.size() > 0 ) modes << mode;

   // Now we have a list of modes.
   // Average each list and round to the closest integer.
   QList< double > wl_average;

   for ( int i = 0; i < modes.size(); i++ )
   {
      double sum = 0.0;

      for ( int j = 0; j < modes[ i ].size(); j++ ) sum += modes[ i ][ j ];

      wl_average << (double) round( 10.0 * sum / modes[ i ].size() ) / 10.0;
   }

   // Now that we have a more reliable list of wavelengths, let's
   // find out the possible cell, channel, and wavelength combinations
   for ( int i = 0; i < rawLegacyData.size(); i++ )
   {
      QString cell       = QString::number( rawLegacyData[ i ].cell );
      QString channel    = QString( rawLegacyData[ i ].channel );
      double wl          = rawLegacyData[ i ].rpoint;
      QString wavelength = "0";

      // find the average wavelength
      for ( int j = 0; j < wl_average.size(); j++ )
      {
         if ( fabs( wl_average[ j ] - wl ) < tolerance )
         {
            wavelength = QString::number( (int) round( wl_average[ j ] ) );
            break;
         }
      }

      QString t = cell + " / " + channel + " / " + wavelength;
      bool found = false;
      for ( int j = 0; j < triples.size(); j++ )
      {
         if ( triples[ j ].tripleDesc == t )
            found = true;
      }
      if ( ! found )
      {
         TripleInfo triple;
         triple.tripleDesc = t;
         triple.tripleID   = triples.size();    // The next number
         triples << triple;
      }
   }
}

void US_Convert::setCcrTriples(
     QList< US_DataIO::BeckmanRawScan >& rawLegacyData,
     QList< TripleInfo >&     triples,
     double                               tolerance )
{
   // First of all, wavelength triples are ccr.
   triples.clear();

   // Now get the radius values
   QStringList radii;

   for ( int i = 0; i < rawLegacyData.size(); i++ )
   {
      QString r = QString::number( rawLegacyData[ i ].rpoint, 'f', 1 );
      radii << r;
qDebug() << "CCR:   i, r" << i << r;
   }

   // Merge radii

   radii.sort();

   QList< QList< double > > modes;
   QList< double >          mode;

   for ( int i = 0; i < radii.size(); i++ )
   {
      double r = radii[ i ].toDouble();

      if ( ! mode.empty()  &&  fabs( mode.last() - r ) > tolerance )
      {
         modes << mode;
         mode.clear();
      }

      mode << r;
   }

   if ( mode.size() > 0 ) modes << mode;
qDebug() << "CCR: mode" << mode;

   // Now we have a list of modes.
   // Average each list and round to the closest integer.
   QList< double > r_average;

   for ( int i = 0; i < modes.size(); i++ )
   {
      double sum = 0.0;

      for ( int j = 0; j < modes[ i ].size(); j++ ) sum += modes[ i ][ j ];

      r_average << (double) round( 10.0 * sum / modes[ i ].size() ) / 10.0;
   }
qDebug() << "CCR: r_average" << r_average;

   // Now that we have a more reliable list of radii, let's
   // find out the possible cell, channel, and radius combinations
   for ( int i = 0; i < rawLegacyData.size(); i++ )
   {
      QString cell       = QString::number( rawLegacyData[ i ].cell );
      QString channel    = QString( rawLegacyData[ i ].channel );
      double r           = rawLegacyData[ i ].rpoint;
      QString radius     = "0";

      // find the average radius
      for ( int j = 0; j < r_average.size(); j++ )
      {
         if ( fabs( r_average[ j ] - r ) < tolerance )
         {
            radius = QString::number( r_average[ j ] );
            break;
         }
      }
qDebug() << "CCR: radius" << radius;

      QString t = cell + " / " + channel + " / " + radius;
      bool found = false;

      for ( int j = 0; j < triples.size(); j++ )
      {
         if ( triples[ j ].tripleDesc == t )
            found = true;
      }

qDebug() << "CCR:  t" << t << "found" << found;
      if ( ! found )
      {
         TripleInfo triple;
         triple.tripleID   = triples.size();    // The next number
         triple.tripleDesc = t;
         triples << triple;
      }
   }
}

void US_Convert::setInterpolated ( unsigned char* bitmap, int location )
{
   int byte = location / 8;
   int bit  = location % 8;

   bitmap[ byte ] |= 1 << ( 7 - bit );
}

// Initializations
US_Convert::TripleInfo::TripleInfo()
{
   clear();
}

void US_Convert::TripleInfo::clear( void )
{
   tripleID     = 0;
   tripleDesc   = QString( "" );
   description  = QString( "" );
   excluded     = false;
   centerpiece  = 0;
   memset( tripleGUID, 0, 16 );
   tripleFilename = QString( "" );
   channelID    = 1;                // One that is implemented currently
   solution.clear();
}

void US_Convert::TripleInfo::show( void )
{
   QString uuidc = US_Util::uuid_unparse( (unsigned char*)tripleGUID );

   qDebug() << "tripleID     = " << tripleID     << '\n'
            << "tripleDesc   = " << tripleDesc   << '\n'
            << "description  = " << description  << '\n'
            << "centerpiece  = " << centerpiece  << '\n'
            << "tripleGUID   = " << QString( uuidc )  << '\n'
            << "tripleFilename = " << tripleFilename << '\n'
            << "channelID    = " << QString::number( channelID ) << '\n'
            << "solutionID   = " << QString::number( solution.solutionID ) << '\n'
            << "solutionGUID = " << solution.solutionGUID << '\n'
            << "solutionDesc = " << solution.solutionDesc;

   if ( excluded ) qDebug() << "excluded";
}

// Adjusts times and omegas in AUCs and speedsteps so that
//  the first speed step acceleration is 400 rpm/second.
int US_Convert::adjustSpeedstep( QVector< US_DataIO::RawData >& allData,
     QVector< US_SimulationParameters::SpeedProfile >& speedsteps )
{
   int stat          = 0;

   // Re-check that there is a problem with low acceleration.
   //  [ See comments for US_AstfemMath::low_acceleration() ]
   const double tfac = ( 4.0 / 3.0 );
   double t2         = speedsteps[ 0 ].time_first;
   double w2t        = speedsteps[ 0 ].w2t_first;
   double om1t       = speedsteps[ 0 ].rotorspeed * M_PI / 30.0;
   double w2         = sq( om1t );
   double t1w        = w2t / w2;
   double t1         = tfac * ( t2 - t1w );

   if ( t1 >= t2 )
   {  // Something wrong!  set t1 to a few seconds before t2
      t1             = t2 - 5.0;
   }

   int t_acc      = (int)qRound( t1 );
   t1             = (double)t_acc;
   double rate    = (double)( speedsteps[ 0 ].rotorspeed ) / t1;
qDebug() << "CVT:Adj: t_acc rate" << t_acc << rate;

   if ( rate >= 250.0 )
   {  // No adjustment will be done, since acceleration is large enough.
      return 1;
   }

   // Determine time and omega offsets. A "t0" value is initially time-zero,
   //  the beginning of the first acceleration zone. Using an acceleration
   //  of 400.0 rpm/second, a new zone start is determined and that becomes
   //  the time offset to subtract from all AUC and speedstep times. The
   //  omega^2t offset is based on the value at the new zone start.
   double azdur      = (double)qRound( speedsteps[ 0 ].rotorspeed / 400.0 );
   double t_offs     = t1 - azdur;
   double azwrate    = sq( speedsteps[ 0 ].rotorspeed * 0.5 * M_PI / 30.0 );
   double w_offs     = w2t - ( w2 * ( t2 - t1 ) ) - ( azwrate * azdur );
qDebug() << "CVT:Adj: azdur azwrate" << azdur << azwrate << "t_offs w_offs"
 << t_offs << w_offs;

   for ( int ii = 0; ii < allData.count(); ii++ )
   {  // Adjust scan times,omegas so t0 is zero
      US_DataIO::RawData* rdata = &allData[ ii ];
      for ( int jj = 0; jj < rdata->scanCount(); jj++ )
      {
qDebug() << "CVT:Adj:  t w" << rdata->scanData[ jj ].seconds
 << rdata->scanData[ jj ].omega2t << "ii jj" << ii << jj;
         rdata->scanData[ jj ].seconds  -= t_offs;
         rdata->scanData[ jj ].omega2t  -= w_offs;
qDebug() << "CVT:Adj:    new t w" << rdata->scanData[ jj ].seconds
 << rdata->scanData[ jj ].omega2t;
      }
   }

   for ( int ii = 0; ii < speedsteps.count(); ii++ )
   {  // Adjust speed step times,omegas
qDebug() << "CVT:Adj:  tf tl" << speedsteps[ii].time_first
 << speedsteps[ii].time_last << "wf wl" << speedsteps[ii].w2t_first
 << speedsteps[ii].w2t_last;
      speedsteps[ ii ].time_first  -= t_offs;
      speedsteps[ ii ].w2t_first   -= w_offs;
      speedsteps[ ii ].time_last   -= t_offs;
      speedsteps[ ii ].w2t_last    -= w_offs;
      speedsteps[ ii ].acceleration = 400;
qDebug() << "CVT:Adj:   new tf tl" << speedsteps[ii].time_first
 << speedsteps[ii].time_last << "wf wl" << speedsteps[ii].w2t_first
 << speedsteps[ii].w2t_last;

      if ( ii == 0 )
      {  // For 1st step, adjust duration and delay-to-1st-scan
         double dur_sec    =  speedsteps[ ii ].duration_hours * 3600
                            + speedsteps[ ii ].duration_minutes * 60.0
                            - t_offs;
         double dur_min    = dur_sec / 60.0;
         int    dur_hrs    = (int)( dur_min / 60.0 );
         dur_min           = dur_min - ( dur_hrs * 60 );
         speedsteps[ ii ].duration_hours   = dur_hrs;
         speedsteps[ ii ].duration_minutes = dur_min;
         double dly_sec    =  speedsteps[ ii ].delay_hours * 3600
                            + speedsteps[ ii ].delay_minutes * 60.0
                            - t_offs;
         double dly_min    = dly_sec / 60.0;
         int    dly_hrs    = (int)( dly_min / 60.0 );
         dly_min           = dly_min - ( dly_hrs * 60 );
         speedsteps[ ii ].delay_hours   = dly_hrs;
         speedsteps[ ii ].delay_minutes = dly_min;
qDebug() << "CVT:Adj:   new dur" << speedsteps[ii].duration_hours
 << speedsteps[ii].duration_minutes << "new dly" << speedsteps[ii].delay_hours
 << speedsteps[ii].delay_minutes;
      }
   }

   return stat;
}

