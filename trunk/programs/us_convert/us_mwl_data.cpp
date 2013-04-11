//! \file us_mwl_data.cpp

#include <QtGui>
#include "us_mwl_data.h"
#include "us_util.h"
#include "us_settings.h"
#include "us_editor.h"

// Hold data read in and averaged from a raw MWL data directory
US_MwlData::US_MwlData( )
{
   clear();                // Clear internal vectors

   dbg_level  = US_Settings::us_debug();
DbgLv(0) << "MwDa: dbg_level" << dbg_level;
}

// Import MWL data from a selected local directory
bool US_MwlData::import_data( QString& mwldir, QLineEdit* lestat )
{
   bool status  = true;
   cur_dir      = mwldir;
   le_status    = lestat;
DbgLv(1) << "MwDa: cur_dir" << cur_dir;

   QDir ddir( cur_dir, "*", QDir::Name, QDir::Files | QDir::Readable );
   ddir.makeAbsolute();
   if ( cur_dir.right( 1 ) != "/" )  cur_dir += "/";

   read_runxml( ddir, cur_dir );

   int kcelchn  = cellchans.count();
   cellchans.clear();
   QString old_runID  = runID;

   runID.replace( QRegExp( "![A-Za-z0-9_-]" ), "_" );

   if ( runID != old_runID )
   {
      QMessageBox::warning( 0,
            tr( "RunID Name Changed" ),
            tr( "The runID name has been changed. It may consist only\n"
                "of alphanumeric characters, underscore, or hyphen.\n"
                "  New runID: " ) + runID );
   }

   // Read in the data
   QStringList mwrfs = ddir.entryList( QStringList( "*.mwrs" ),
         QDir::Files, QDir::Name );
   QStringList chans;
   cells.clear();
   nscan         = -1;
   mwrfs.sort();
   nfile         = mwrfs.size();
   le_status->setText( QString( "%1 files from %2 ..." )
       .arg( nfile ).arg( runID ) );
   qApp->processEvents();
DbgLv(1) << "MwDa: nfile" << nfile;

   for ( int ii = 0; ii < nfile; ii++ )
   {
      QString fname   = mwrfs.at( ii );
      QString fpath   = cur_dir + fname;
      QString acell   = fname.section( ".", -5, -5 );
      QString chann   = fname.section( ".", -4, -4 );
      QString adesc   = fname.section( ".", -3, -3 );
      QString ascan   = fname.section( ".", -2, -2 );

      if ( !cells.contains( acell ) )  cells << acell;
      if ( !chans.contains( chann ) )  chans << chann;

      int     scann   = ascan.toInt();
      nscan           = qMax( nscan, scann );

      fnames << fname;
      fpaths << fpath;
   }

   nscan++;
   ncell       = cells.size();
   nchan       = chans.size();
DbgLv(1) << "MwDa: nscan ncell nchan" << nscan << ncell << nchan;
   ncelchn     = ncell * nchan;
   if ( ncelchn != kcelchn )
   {
      qDebug() << "kcelchn ncelchn" << kcelchn << ncelchn;
   }
   cells    .sort();
   chans    .sort();
   cellchans.clear();
   le_status->setText( QString( "%1 cells and %2 channels ..." )
       .arg( ncell ).arg( nchan ) );
   qApp->processEvents();

   for ( int ii = 0; ii < ncell; ii++ )
   {
      for ( int jj = 0; jj < nchan; jj++ )
      {
         QString celchn = cells[ ii ] + " / " + chans[ jj ];
         cellchans << celchn;
      }
   }

   // Read in all the headers
   for ( int ii = 0; ii < nfile; ii++ )
   {
      QString fname   = fnames[ ii ];
      QString fpath   = fpaths[ ii ];

      QFile fi( fpath );
      if ( ! fi.open( QIODevice::ReadOnly ) )
      {
         qDebug() << "*ERROR* Unable to open" << fname;
         qDebug() << fpath;
         status = false;
         break;
      }

      QDataStream ds( &fi );
      DataHdr     hd;

      read_header( ds, hd );

      headers << hd;

      if ( ii == 0 )
      {  // At the first file, read in the wavelengths
         nlambda     = hd.nlambda;
         ntriple     = nlambda * ncelchn;
         npoint      = hd.npoint;
         npointt     = npoint  * nscan;
DbgLv(1) << "MwDa: npoint nlambda" << npoint << nlambda;

         read_lambdas( ds, ri_wavelns, nlambda );
DbgLv(1) << "MwDa:   read_lambdas COMPLETE";
int ww=nlambda-1;
DbgLv(1) << "MwDa:    w0 w1 w3 wi wj wk" << ri_wavelns[0] << ri_wavelns[1]
 << ri_wavelns[2] << ri_wavelns[ww-2] << ri_wavelns[ww-1] << ri_wavelns[ww];
         le_status->setText( QString( "%1 wavelengths ..." )
             .arg( nlambda ) );
         qApp->processEvents();

         // And initialize the data vector
         QVector< double > wave_reads( npointt, 0.0 );
         ri_readings.clear  ();
         ri_readings.reserve( ntriple );

         for ( int tx = 0; tx < ntriple; tx++ )
         {
            ri_readings << wave_reads;
         }
//DbgLv(1) << "MwDa:   ri_readings CREATED size" << ri_readings.size();
      }
      else
      {  // Otherwise, skip past wavelengths
         ds.skipRawData( nlambda * 2 );
      }

      int ccx    = hd.icell * nchan + hd.ichan;
      int tripx  = ccx * nlambda;
      int scnx   = fname.section( ".", -2, -2 ).toInt() * npoint;
//DbgLv(1) << "MwDa:  PREPARE rdata ccx tripx scnx" << ccx << tripx << scnx;
//DbgLv(1) << "MwDa:  PREPARE   icell ichan nchan" << hd.icell << hd.ichan
// << nchan << "channel" << hd.channel;

      // Read in the radius point data
      for ( int wavx = 0; wavx < nlambda; wavx++ )
      {
         read_rdata( ds, ri_readings[ tripx++ ], scnx, npoint );
      }

      le_status->setText( QString( "Data in for triple %1, scan %2 ..." )
          .arg( tripx + 1 ).arg( scnx + 1 ) );
      qApp->processEvents();
//DbgLv(1) << "MwDa:    read_data COMPLETE";
   } // END: header read loop

DbgLv(1) << "MwDa: wv0 wvm wvn" << ri_wavelns[0]
 << ri_wavelns[nlambda/2] << ri_wavelns[nlambda-1];
DbgLv(1) << "MwDa: da20 40 m+40 n-40" << ri_readings[20][40]
 << ri_readings[20][npointt/2+40] << ri_readings[20][npointt-41];
   le_status->setText(
      QString( "Initial MWL import from %1 files is complete." )
      .arg( nfile ) );
   qApp->processEvents();

   return status;
}

// Return a readings values vector for a given triple, scan
int US_MwlData::rvalues( int& tripx, int& scanx, QVector< double >& rvs )
{
   int jj = scanx * npoint;
   rvs.clear();
   rvs.reserve( npoint );

   for ( int ii = 0; ii < npoint; ii++ )
   {
      rvs << ri_readings[ tripx ][ jj++ ];
   }

   return jj;
}

// Return the lambdas vector for the data
int US_MwlData::lambdas( QVector< double >& wls )
{
   wls.clear();
   wls.reserve( nlambda );

   for ( int ii = 0; ii < nlambda; ii++ )
   {
      wls << ri_wavelns[ ii ];
   }

   return nlambda;
}

// Private slot to clear arrays
void US_MwlData::clear()
{
   ri_readings.clear();     // Input raw readings
   ri_wavelns .clear();     // Input raw wavelengths
   headers   .clear();     // MWL file headers

   fpaths    .clear();
   fnames    .clear();
   cells     .clear();
   cellchans .clear();
   ccdescs   .clear();
   triples   .clear();
   trnodes   .clear();

   nfile      = 0;
   nscan      = 0;
   ncell      = 0;
   nchan      = 0;
   ncelchn    = 0;
   nlambda    = 0;
   ntriple    = 0;
   npoint     = 0;
   npointt    = 0;

   mapCounts();
}

// Utility to extract a half-word (2-byte) from a character buffer
int US_MwlData::hword( char* cc )
{
   unsigned char* cbuf = (unsigned char*)cc;
   int j0 = (int)cbuf[ 0 ] & 255;
   int j1 = (int)cbuf[ 1 ] & 255;
   return ( ( j0 << 8 ) | j1 );
}

// Utility to extract a full-word (4-byte) from a character buffer
int US_MwlData::iword( char* cbuf )
{
   int j0 = hword( cbuf );
   int j1 = hword( cbuf + 2 );
   return ( ( j0 << 16 ) | j1 );
}

// Utility to extract a float from a character buffer (4-byte)
float US_MwlData::fword( char* cbuf )
{
   int    ival  = iword( cbuf );
   int*   iptr  = &ival;
   float* fptr  = (float*)iptr;
   return *fptr;
}

// Utility to extract a double from a character buffer (4-byte)
double US_MwlData::dword( char* cbuf )
{
   float fval   = fword( cbuf );
   return (double)fval;
}

// Utility to read an MWL header from a data stream
void US_MwlData::read_header( QDataStream& ds, DataHdr& hd )
{
   char cbuf[ 28 ];

   ds.readRawData( cbuf, 24 );

   hd.cell          = QChar( '0' | cbuf[ 0 ] );
   hd.channel       = QChar( cbuf[ 1 ] );
   hd.icell         = cells.indexOf( QString( hd.cell ) );
   hd.ichan         = QString( "ABCDEFGH" ).indexOf( hd.channel );
   hd.iscan         = hword( cbuf + 2 );
   hd.rotor_speed   = hword( cbuf + 4 );
   hd.temperature   = (double)( hword( cbuf + 6 ) ) / 10.0;
   hd.omega2t       = dword( cbuf + 8 );
   hd.elaps_time    = iword( cbuf + 12 );
   hd.npoint        = hword( cbuf + 16 );
   hd.radius_start  = (double)( hword( cbuf + 18 ) ) / 1000.0;
   hd.radius_step   = (double)( hword( cbuf + 20 ) ) / 10000.0;
   hd.nlambda       = hword( cbuf + 22 );
}

// Utility to read the lambdas from a data stream
void US_MwlData::read_lambdas( QDataStream& ds, QVector< double >& wvs,
      int& nlambda )
{
   char cbuf[ 4 ];

   wvs.reserve( nlambda );

   for ( int ii = 0; ii < nlambda; ii++ )
   { // Pick up each 2-byte value and convert to double
      ds.readRawData( cbuf, 2 );
      double wvv   = (double)hword( cbuf );
      wvs << wvv;
   }
}

// Utility to read the radius point data from a data stream
void US_MwlData::read_rdata( QDataStream& ds, QVector< double >& rvs,
      int& scnx, int& npoint )
{
   char cbuf[ 4 ];
   int  kk      = scnx;

   for ( int ii = 0; ii < npoint; ii++ )
   { // Pick up each 4-byte value, convert to double and divide by 1000
      ds.readRawData( cbuf, 4 );
      int    ival  = iword( cbuf );
      double rvv   = (double)ival / 1000.0;
      rvs[ kk++ ]  = rvv;
   }
}

// Find the index of a lambda value in the input raw list of lambdas
int US_MwlData::indexOfLambda( double lambda )
{
   int    wvx     = ri_wavelns.indexOf( lambda );   // Try exact match

   if ( wvx < 0 )
   {  // If lambda in not in the list, find the nearest to a match
      double diflow  = 1e+99;

      for ( int ii = 0; ii < nlambda; ii++ )
      {
         double difval  = qAbs( lambda - ri_wavelns[ ii ] );

         if ( difval < diflow )
         {  // New low difference, so save index and new low
            wvx            = ii;
            diflow         = difval;
         }
      }
   }

   return wvx;
}

// Return the list of cell/channel strings
int US_MwlData::cellchannels( QStringList& celchns )
{
   celchns = cellchans;
   return ncelchn;
}

// Populate the list of RawData objects from averaged MWL data
int US_MwlData::build_rawData( QVector< US_DataIO2::RawData >& allData )
{
   allData.clear();

   // Build the radius vector that is constant
   QVector< US_DataIO2::XValue > xout;
   double rad_val  = headers[ 0 ].radius_start;
   double rad_inc  = headers[ 0 ].radius_step;
qDebug() << "BldRawD radv radi" << rad_val << rad_inc << "npoint" << npoint;

   for ( int ii = 0; ii < npoint; ii++ )
   {
      xout << US_DataIO2::XValue( rad_val );
      rad_val  += rad_inc;
   }
qDebug() << "BldRawD   xout size" << xout.size() << npoint;

   // Set up the interpolated byte array (all zeroes)
   int    nbytei   = ( npoint + 7 ) / 8;
   QByteArray interpo( nbytei, '\0' );

   // Build a raw data set for each triple
   char   dtype0   = 'R';
   char   dtype1   = 'I';
   int    ccx      = 0;
   int    wvx      = 0;
   int    hdx      = 0;

   for ( int trx = 0; trx < ntriple; trx++ )
   {
      US_DataIO2::RawData rdata;
      QString uuid_str  = US_Util::new_guid();
      US_Util::uuid_parse( uuid_str, (unsigned char*)rdata.rawGUID );
      // Set triple values
      rdata.type[ 0 ]   = dtype0;
      rdata.type[ 1 ]   = dtype1;
      rdata.cell        = QString( headers[ hdx ].cell ).toInt();
      rdata.channel     = headers[ hdx ].channel.toAscii();
      rdata.x           = xout;
      int jhx           = hdx; 
      int rdx           = 0;
      rdata.description = ccdescs.at( ccx );
//qDebug() << "BldRawD     trx" << trx << " building scans...";

      for ( int scx = 0; scx < nscan; scx++ )
      {  // Set scan values
         US_DataIO2::Scan scan;
         scan.temperature  = headers[ jhx ].temperature;
         scan.rpm          = headers[ jhx ].rotor_speed;
         scan.seconds      = headers[ jhx ].elaps_time;
         scan.omega2t      = headers[ jhx ].omega2t;
         scan.wavelength   = ri_wavelns[ wvx ];
         scan.delta_r      = rad_inc;
         scan.readings.reserve( npoint );
         scan.interpolated = interpo;
//qDebug() << "BldRawD      scx" << scx << "jhx" << jhx
// << "seconds" << scan.seconds;
         jhx++;

         for ( int kk = 0; kk < npoint; kk++ )
         {  // Set readings values
            double dvalue     = ri_readings[ trx ][ rdx++ ];
            scan.readings << US_DataIO2::Reading( dvalue );
         } // END: radius points loop

         rdata.scanData << scan;      // Append a scan to a triple
      } // END: scan loop

//qDebug() << "BldRawD     trx" << trx << " saving allData...";
      allData << rdata;               // Append triple data to the array
      le_status->setText( QString( "Of %1 raw AUCs, built %2" )
          .arg( ntriple ).arg( trx + 1 ) );
      qApp->processEvents();

      wvx++;
      if ( wvx >= nlambda )
      {  // After final wavelength, reset at next cell/channel
         ccx++;
         wvx  = 0;
         hdx  = ccx * nscan;
      }
//qDebug() << "BldRawD   ccx wvx hdx" << ccx << wvx << hdx;
   } // END: triple loop

   le_status->setText( QString( "All %1 raw AUCs have been build." )
       .arg( ntriple ) );
   qApp->processEvents();

   return ntriple;
}

// Return a count of a specified type
int US_MwlData::countOf( QString key )
{
   mapCounts();

   return counts[ key ];
}

// Return a count of a specified type
QString US_MwlData::cc_description( QString celchn )
{
   int ccx = cellchans.indexOf( celchn );
   return ( ccx < 0 ? "" : ccdescs.at( ccx ) );
}

void US_MwlData::run_values( QString& arunid, QString& aruntype )
{
   arunid   = runID;
   aruntype = intensity ? "RI" : "RA";
}

// Private slot to map counts and sizes
void US_MwlData::mapCounts( void )
{
   counts[ "file"      ]  = nfile;
   counts[ "scan"      ]  = nscan;
   counts[ "cell"      ]  = ncell;
   counts[ "channel"   ]  = nchan;
   counts[ "cellchann" ]  = ncelchn;
   counts[ "lambda"    ]  = nlambda;
   counts[ "triple"    ]  = ntriple;
   counts[ "point"     ]  = npoint;
   counts[ "point_all" ]  = npointt;
}

// Read the run XML file and return its values
void US_MwlData::read_runxml( QDir ddir, QString curdir )
{
   QStringList mwrfs = ddir.entryList( QStringList( "*.mwrs.xml" ),
         QDir::Files, QDir::Name );
   int nxfile    = mwrfs.count();

   if ( nxfile > 1 )
   {
      qDebug() << "*ERROR* '*.mwrs.xml' count > 1" << nxfile << curdir;
      return;
   }

   QString fname = mwrfs.at( 0 );
   QString fpath = cur_dir + fname;
   QFile xfi( fpath );

   if ( ! xfi.open( QIODevice::ReadOnly ) )
   {
      qDebug() << "*ERROR* Unable to open" << fname;
      qDebug() << fpath;
      return;
   }

   QXmlStreamReader xml( &xfi );
   QString celi;
   QString chni;
   QString cech;
   cellchans.clear();

   while( ! xml.atEnd() )
   {
      xml.readNext();
      if ( xml.isStartElement() )
      {
         QXmlStreamAttributes att = xml.attributes();
         if ( xml.name() == "runID" )
         {
            runID           = att.value( "name"           ).toString();
            QString speed   = att.value( "speed_mode"     ).toString();
            QString intens  = att.value( "take_intensity" ).toString();
            speed_mode      = ( speed  == "Y" );
            intensity       = ( intens == "Y" );
         }
         else if ( xml.name() == "cell" )
         {
            celi            = att.value( "id"             ).toString();
            celi            = QString::number( celi.toInt() );
         }
         else if ( xml.name() == "channel" )
         {
            chni            = att.value( "id"             ).toString();
            cech            = celi + " / " + chni;
            QString desc    = att.value( "sample"         ).toString();

            if ( ! cellchans.contains( cech ) )
            {
               cellchans << cech;
               ccdescs << desc;
            }
         }
      }
   }

   xfi.close();
}

