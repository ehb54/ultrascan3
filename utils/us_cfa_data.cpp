//! \file us_cfa_data.cpp

#include "us_cfa_data.h"
#include "us_util.h"
#include "us_settings.h"
#include "us_math2.h"
#include "us_memory.h"
#include "us_time_state.h"

// Hold data read in and selected from a raw CFA data directory
US_CfaData::US_CfaData( )
{
   clear();                // Clear internal vectors

   dbg_level    = US_Settings::us_debug();
DbgLv(0) << "CfDa: dbg_level" << dbg_level;
}

// Import CFA data from a selected local directory
bool US_CfaData::import_data( QString& cfadir, const int sctypf )
{
   bool status  = true;
   sctype       = sctypf;
   cur_dir      = cfadir;
   dbfile       = cur_dir;
   is_absorb    = false;
   is_raw       = true;

DbgLv(1) << "CfDa: cur_dir" << cur_dir;

   if ( sctype > 0 )
   {  // Scan type given:  data from .sqlite file
      if ( QFileInfo( cur_dir ).isFile() )
      {  // Referenced "directory" is a file, so use that;
         cur_dir      = cur_dir.section( "/",  0, -2 ) + "/";
      }

      else
      {  // Use referenced directory; find (largest) .sqlite file there
         if ( cur_dir.right( 1 ) != "/" )
            cur_dir     += "/";

         QDir ddir( cur_dir, "*", QDir::Name,
                    QDir::Files | QDir::Readable );
         QStringList dbfs = ddir.entryList( QStringList( "*.sqlite" ),
            QDir::Files, QDir::Name );
         int ndfile   = dbfs.count();

         if ( ndfile == 1 )
         {  // Directory has single .sqlite file
            dbfile       = cur_dir + dbfs[ 0 ];
         }

         else if ( ndfile > 1 )
         {  // Directory has multiple .sqlfile files; use largest
            int szfile   = 0;
            for ( int jj = 0; jj < ndfile; jj++ )
            {
               QString fpa  = cur_dir + dbfs[ jj ];
               int sztest   = QFileInfo( fpa ).size();

               if ( sztest > szfile )
               {
                  szfile       = sztest;
                  dbfile       = fpa;
               }
            }
         }

         else
         {  // Directory has no .sqlite file, ERROR!
            dbfile       = QString( "" );
         }
      }  // END: directory (not file path) was specified

      scan_cfafile( dbfile, sctype );
DbgLv(1) << "CfDa: sctype" << sctype << "is_absorb" << is_absorb;

      int kcelchn  = cellchans.count();

      if ( kcelchn == 0 )
      {
         QString stat_text = tr( "No proper CFA .sqlite file was found"
                                 " in directory \"%1\"." ).arg( cur_dir );
         emit status_text( stat_text );
         return false;
      }
   }  // END: input from .sqlite file

   runID        = cur_dir.section( "/", -2, -2 );
   QString old_runID  = runID;

   runID.replace( QRegularExpression( "[^A-Za-z0-9_-]" ), "_" );

   if ( runID != old_runID )
   {
      QString stat_text = tr( "The runID name has been changed. It may"
                              " consist only of alphanumeric characters,"
                              " underscore, or hyphen.  New runID: " )
                          + runID;
      emit status_text( stat_text );
   }

   return status;
}

// Load internal values from a vector of loaded rawDatas
void US_CfaData::load_auc( QVector< US_DataIO::RawData >& allData,
                           const QStringList ifpaths )
{
   QStringList chans;
   sctype       = -1;
   is_absorb    = false;
   is_raw       = false;
   fpaths       = ifpaths;
   nfile        = allData.size();
   ntriple      = nfile;
   nscan        = allData[ 0 ].scanCount();
   npoint       = allData[ 0 ].pointCount();
   npointt      = npoint * nscan;
   int mnscan   = 99999;
   int mxscan   = 0;
   cur_dir      = QString( fpaths[ 0 ] ).section( "/",  0, -2 ) + "/";
   runID        = QString( fpaths[ 0 ] ).section( "/", -2, -2 ).simplified();
DbgLv(1) << "CfDa:LdMw: nfile" << nfile << "nscan npoint" << nscan << npoint;
DbgLv(1) << "CfDa:LdMw: fpaths0" << fpaths[0];
DbgLv(1) << "CfDa:LdMw: runID" << runID;
DbgLv(1) << "CfDa:LdMw: cur_dir" << cur_dir;

   wavelns    .clear();
   scanids    .clear();
   cells      .clear();
   cellchans  .clear();
   ccdescs    .clear();
   triples    .clear();
   fnames     .clear();

   for ( int trx = 0; trx < nfile; trx++ )
   {
      US_DataIO::RawData* rdata = &allData[ trx ];

      nscan            = rdata->scanCount();
      mnscan           = qMin( mnscan, nscan );
      mxscan           = qMax( mxscan, nscan );
      int     icell    = rdata->cell;
      QString cell     = QString::number( icell );
      QString chan     = QString( rdata->channel );
      QString celchn   = cell + " / " + chan;
      int     iwvl     = qRound( rdata->scanData[ 0 ].wavelength );
      QString wavl     = QString::number( iwvl );
      QString triple   = celchn + " / " + wavl;
DbgLv(1) << "CfDa:LdMw:   trx" << trx << "triple" << triple;
      QString fpath    = fpaths[ trx ];
DbgLv(1) << "CfDa:LdMw:    fpath" << fpath;
      fnames << fpath.section( "/", -1, -1 );

      if ( ! triples.contains( triple ) )
         triples << triple;              // Build unique triple list

      else
      {  // Handle duplicate triple
DbgLv(1) << "CfDa:LdMw:    DUPLICATE triple" << triple << "trx" << trx;
         QString trnode   = fpath.section( ".", -4, -2 );
         QString ccnode   = fpath.section( ".", -4, -3 );
         int     iwnode   = fpath.section( ".", -2, -2 ).toInt();
         QString wvnode   = QString::number( iwnode );
         QString triplf   = ccnode.replace( ".", " / " ) + " / " + wvnode;

         if ( triplf != triple )
         {
            triple           = triplf;

            if ( ! triples.contains( triple ) )
            {
               cell             = trnode.section( ".", 0, 0 );
               chan             = trnode.section( ".", 1, 1 );
               iwvl             = trnode.section( ".", 2, 2 ).toInt();
               wavl             = QString::number( iwvl );
               celchn           = triple.section( " / ", 0, 1 );
DbgLv(1) << "CfDa:LdMw:      NEW triple" << triple << trnode;
               triples << triple;
            }
         }
      }

      if ( ! cells.contains( cell ) )
         cells << cell;                  // Build unique cell list

      if ( ! chans.contains( chan ) )
         chans << chan;                  // Build unique channel list

      if ( ! cellchans.contains( celchn ) )
      {
         cellchans << celchn;            // Build unique cell/channel list
         ccdescs   << rdata->description;
      }

      if ( ! wavelns.contains( iwvl ) )
      {
         wavelns << iwvl;                // Build unique wavelength list
      }
   }

   nscan    = mxscan;
   ncell    = cells    .size();
   nchan    = chans    .size();
   ncelchn  = cellchans.size();
   nlambda  = wavelns  .size();;
   slambda  = wavelns[ 0 ];
   elambda  = wavelns[ nlambda - 1 ];
DbgLv(1) << "CfDa:LdMw: ncell nchan nlambda" << ncell << nchan << nlambda;
DbgLv(1) << "CfDa:LdMw:   ncelchn ntriple" << ncelchn << ntriple;
DbgLv(1) << "CfDa:LdMw:   wlns size" << wavelns.size();

   QString stat_text = tr( "%1 triples loaded from %2 ..." )
                      .arg( ntriple ).arg( runID );
   emit status_text( stat_text );
}

// Return the input raw lambdas vector for the data
int US_CfaData::lambdas_raw( QVector< int >& wls )
{
   wls.clear();
   wls.reserve( nlambda );

   for ( int ii = 0; ii < nlambda; ii++ )
   {
      wls << wavelns[ ii ];
   }

   return nlambda;
}

// Private slot to clear arrays
void US_CfaData::clear()
{
   wavelns    .clear();     // Raw input wavelengths
   scanids    .clear();     // Scan IDs, selected type

   fpaths     .clear();
   fnames     .clear();
   cells      .clear();
   cellchans  .clear();
   ccdescs    .clear();
   triples    .clear();
   trnodes    .clear();

   nfile      = 0;
   nscan      = 0;
   ncell      = 0;
   nchan      = 0;
   ncelchn    = 0;
   nlambda    = 0;
   ntriple    = 0;
   npoint     = 0;
   npointt    = 0;
   slambda    = 0;
   elambda    = 0;

   mapCounts();
}

// Find the index of a lambda value in the input raw list of lambdas
int US_CfaData::indexOfLambda( int lambda )
{
   int    wvx     = wavelns.indexOf( lambda );   // Try exact match

   if ( wvx < 0 )
   {  // If lambda is not in the list, find the nearest to a match
      int    diflow  = 9999;

      for ( int ii = 0; ii < nlambda; ii++ )
      {
         int    difval  = qAbs( lambda - wavelns[ ii ] );

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
int US_CfaData::cellchannels( QStringList& celchns )
{
   celchns = cellchans;
   return ncelchn;
}

// Populate the list of RawData objects from raw input CFA data
int US_CfaData::build_rawData( QVector< US_DataIO::RawData >& allData )
{
DbgLv(1) << "BldRawD IN";
   const double signif_dif=100.0;
   const int    low_memApc=20;

   allData.clear();

   // Build the radius vector
   a_radii         = r_radii;
   double rad_inc  = ( a_radii[ npoint - 1 ] - a_radii[ 0 ] ) 
                     / (double)( npoint - 1 );
DbgLv(1) << "BldRawD rad_inc" << rad_inc << "npoint" << npoint;

   r_rpms.clear();
   s_rpms.clear();
   a_rpms.clear();

   // Set up the interpolated byte array (all zeroes)
   int    nbytei   = ( npoint + 7 ) / 8;
   QByteArray interpo( nbytei, '\0' );

   // Build a raw data set for each triple
   char   dtype0   = 'R';
   char   dtype1   = is_absorb ? 'A' : 'I';
   int    ccx      = 0;
   int    wvx      = 0;
   int    scnbr    = 0;
   int    scnid    = 0;
   int    snx      = 0;
   int    six      = 0;
   nscnn           = scnnbrs.count();
   int    mnscan   = nscnn;
   int    mxscan   = 0;
DbgLv(1) << "BldRawD szs: ccd" << ccdescs.size()
 << "wvs" << wavelns.size() << "nlambda" << nlambda;

   for ( int trx = 0; trx < ntriple; trx++ )
   {
DbgLv(1) << "BldRawD     trx" << trx << " building scans... ccx" << ccx;
      US_DataIO::RawData rdata;
      QString uuid_str  = US_Util::new_guid();
      US_Util::uuid_parse( uuid_str, (unsigned char*)rdata.rawGUID );
      // Set triple values
      rdata.type[ 0 ]   = dtype0;
      rdata.type[ 1 ]   = dtype1;
      QString celchn    = cellchans[ ccx ];
      rdata.cell        = celchn.section( "/", 0, 0 ).toInt();
      rdata.channel     = celchn.section( "/", 1, 1 ).simplified()
                          .toLatin1().data()[ 0 ];
QString str_ch=celchn.section("/",1,1).simplified();
QByteArray bar_ch=str_ch.toLatin1();
char* chp_ch=bar_ch.data();
char  chr_ch=chp_ch[0];
DbgLv(1) << "BldRawD        channel" << rdata.channel
 << "str" << str_ch << "bar" << bar_ch << "chp" << chp_ch << "chr" << chr_ch;
      rdata.xvalues     = a_radii;
      npoint            = a_radii.count();
      int ndscan        = 0;
      int rdx           = 0;
      int kspstep       = 1;
      int kscx          = 0;
      int nspstep       = 0;
      int kscan         = 0;
      double rpm_min    = 1e+9;
      double rpm_max    = 1e-9;
      double rpm_sum    = 0.0;
      double rpm_set    = tcmetas[ 0 ].drpm;
      double rpm_setp   = rpm_set;
      double rpm_avg    = 0.0;
      rdata.description = ccdescs.at( ccx );
      QString triple    = triples[ trx ].replace( " / ", "/" );

QDateTime time10=QDateTime::currentDateTime();
      for ( int scx = 0; scx < nscnn; scx++ )
      {  // Set scan values
         int frec, lrec;
         US_DataIO::Scan scan;
         scnbr             = scnnbrs[ scx ];
         snx               = scan_nbr_index( triple, scnbr );

         if ( snx >= 0 )
         {
            scnid             = tsmetas[ snx ].scanId;
            frec              = tsmetas[ snx ].firstRec;
            lrec              = tsmetas[ snx ].lastRec;
         }
         else
         {
            scnid             = 0;
            frec              = -1;
            lrec              = -1;
            continue;
         }
if(scx<3 || (scx+4)>nscnn)
DbgLv(1) << "BldRawD       trx wvx scx snx" << trx << wvx << scx << snx
 << "scnid" << scnid << "frec lrec" << frec << lrec;

         six               = scan_id_index( scnid );
         rpm_set           = tcmetas[ six ].drpm;
         rpm_setp          = rpm_set;
         scan.rpm          = rpm_set;
         scan.seconds      = tcmetas[ six ].time;
         scan.omega2t      = tcmetas[ six ].w2t;
         scan.temperature  = tcmetas[ six ].tempera;
         scan.wavelength   = wavelns[ wvx ];
         scan.delta_r      = rad_inc;
         scan.rvalues.reserve( npoint );
         scan.interpolated = interpo;

         if ( scx > 0  &&  scan.rpm != r_rpms[ scx - 1 ] )
            kspstep++;

if(scx<3 || (scx+4)>nscnn)
DbgLv(1) << "BldRawD        rpm_set rpm_setp" << rpm_set << rpm_setp;
         if ( rpm_set != rpm_setp )
         {  // For a new speed step, re-do speeds in the previous one
            nspstep++;
            rpm_avg           = rpm_sum / (double)kscan;
            double rpmavr     = (double)qRound( rpm_avg );

            for ( int jsx = kscx; jsx < scx; jsx++ )
            {
               // Store the average over the speed step as the scan speed
               rdata.scanData[ jsx ].rpm    = rpmavr;
            }

            if ( trx == 0 )
            {  // Save speeds at first triple
               a_rpms << rpm_avg;
               s_rpms << rpm_setp;
            }

            // Reinitialize for next speed step
            kscx              = scx;
            kscan             = 1;
            rpm_sum           = 0.0;
         }
         else
         {  // Bump scan count in a speed step
            kscan++;
         }

         // Accumulate rpm min,max,sum
         rpm_min           = qMin( rpm_min, scan.rpm );
         rpm_max           = qMax( rpm_max, scan.rpm );
         rpm_sum          += scan.rpm;
//*DEBUG*
if(trx==0) {
DbgLv(1) << "BldRawD      scx" << scx << "trx" << trx
 << "seconds" << scan.seconds << "rpm" << scan.rpm << "ss" << rpm_set
 << "speed step" << kspstep << nspstep+1;
}
//*DEBUG*
         if ( trx == 0 )
            r_rpms << scan.rpm;

         get_readings( scan.rvalues, frec, lrec );
if(scx<3 || (scx+4)>nscnn)
DbgLv(1) << "BldRawD        scx" << scx << "rvalues size" << scan.rvalues.size()
 << "rvalues[mid]" << scan.rvalues[scan.rvalues.size()/2];

         rdata.scanData << scan;      // Append a scan to a triple
         ndscan++;
      } // END: scan loop
DbgLv(1) << "BldRawD         EoSl: trx rdx" << trx << rdx;
QDateTime time20=QDateTime::currentDateTime();
DbgLv(1) << "BldRawD trx" << trx << "TIMEMS:scan loop time"
 << time10.msecsTo(time20);

      // Set the average speed for the final/only speed step
      mnscan            = qMin( mnscan, ndscan );
      mxscan            = qMax( mxscan, ndscan );
      nspstep++;
      rpm_avg           = rpm_sum / (double)kscan;
      double rpmavr     = (double)qRound( rpm_avg );
DbgLv(1) << "BldRawD        kscx" << kscx;

      for ( int scx = kscx; scx < ndscan; scx++ )
      {
         // Store the average over the speed step as the scan speed
         rdata.scanData[ scx ].rpm  = rpmavr;
      }

      if ( trx == 0 )
      {  // Save speeds at first triple
         a_rpms << rpm_avg;
         s_rpms << rpm_set;

         // Test if a set_speed differs significantly from the average
         if ( qAbs( rpm_avg - rpm_set ) > signif_dif )
         {
            QString stat_text = tr( "The stored set_speed (%1) and"
                                    " the average of recorded rotor"
                                    "_speeds (%2) differ significantly!" )
                                .arg( rpm_set ).arg( rpm_avg );
            emit status_text( stat_text );
         }
      }
//*DEBUG*
if(trx==0) {
 DbgLv(1) << "BldRawD trx=" << trx << "rpm_min rpm_max rpm_avg rpm_set"
    << rpm_min << rpm_max << rpm_avg << rpm_set << "speed steps" << nspstep;
}
//*DEBUG*

DbgLv(1) << "BldRawD     trx" << trx << " saving allData... ndscan" << ndscan;
      allData << rdata;               // Append triple data to the array

      QString stat_text = tr( "Of %1 raw AUCs, built %2" )
                          .arg( ntriple ).arg( trx + 1 );
      emit status_text( stat_text );

      wvx++;

      if ( wvx >= nlambda )
      {  // After final wavelength, reset at next cell/channel
         ccx++;
         wvx  = 0;

         // Free up some memory if it is getting tight
         int memAv = US_Memory::memory_profile();

         if ( memAv < low_memApc )
         {
            tscans.clear();
int memAv2 = US_Memory::memory_profile();
DbgLv(1) << "BldRawD  memfree %: 1memAV" << memAv << "2memAV" << memAv2;
         }
      }
   } // END: triple loop

   QString stat_text = tr( "All %1 raw AUCs have been build." ).arg( ntriple );
   emit status_text( stat_text );
  
   nscan             = mxscan;

DbgLv(1) << "BldRawD  DONE ntriple" << ntriple << "mxscan" << mxscan
 << "mnscan" << mnscan;
   return ntriple;
}

// Export RawData to openAUC (.auc) and TMST (.tmst) files
int US_CfaData::export_auc( QVector< US_DataIO::RawData >& allData )
{
   int nfiles        = 0;
   if ( ! is_raw )
      return nfiles;

   int ntrips        = allData.count();
   QString ftype     = QString( allData[ 0 ].type[ 0 ] ) +
                       QString( allData[ 0 ].type[ 1 ] );
   QString fbase     = runID + "." + ftype + ".";

   // Output AUC files

   for ( int ii = 0; ii < ntrips; ii++ )
   {  // Create a file for each triple
      US_DataIO::RawData* rdata = &allData[ ii ];
      QString trnode    = QString::number( rdata->cell ) + "." +
                          QString( rdata->channel ) + "." +
                          QString( "%1" ).arg( qRound( rdata->scanData[0].wavelength ), 3, 10, QChar( '0' ) );
      QString fname     = fbase + trnode + ".auc";
      QString fpath     = cur_dir + fname;

      US_DataIO::writeRawData( fpath, *rdata );

      nfiles++;
DbgLv(1) << "expA: nf" << nfiles << "fname" << fname;
   }

   // Output time state files

   QStringList fkeys;
   QStringList ffmts;
   fkeys << "Scan"    << "Time"        << "RawSpeed" << "SetSpeed"
         << "Omega2T" << "Temperature" << "Pressure" << "Period"
         << "Jitter";
   ffmts << "I2"      << "F4"          << "I4"       <<  "I4"
         << "F4"      << "I4"          << "F4"       <<  "F4"
         << "F4";
   US_TimeState tsobj;

   QString tspath    = cur_dir + runID + ".time_state.tmst";
   int ntimes        = scnnbrs.count();

   tsobj.open_write_data( tspath );      // Initialize TMST creation

   tsobj.set_keys( fkeys, ffmts );       // Define keys and formats

   // Output time state files

   for ( int ii = 0;  ii < ntimes; ii++ )
   {  // Create a record for each scan
      int scannbr    = scnnbrs[ ii ];    // Scan number
      int scanid     = scanids[ ii ];    // Scan ID for matching
      int jjc        = 0;                // Default match index in CFAMeta
      int jjd        = 0;                // Default match index in DIOMeta

      for ( int jj = 0; jj < tcmetas.count(); jj++ )
      {  // Match scan ID in CFAMeta
         if ( tcmetas[ jj ].scanId == scanid )
         {
            jjc            = jj;         // Index to ID in CFAMeta values
            break;
         }
      }

      for ( int jj = 0; jj < tdmetas.count(); jj++ )
      {  // Match scan ID in DIOMeta
         if ( tdmetas[ jj ].scanId == scanid )
         {
            jjd            = jj;         // Index to ID in DIOMeta values
            break;
         }
      }
DbgLv(1) << "expA:  ii" << ii << "scannbr scanid" << scannbr << scanid
 << "jjc" << jjc << "jjd" << jjd << "time" << tcmetas[jjc].time;

      // Set values for this scan
      tsobj.set_value( fkeys[ 0 ], scannbr );                  // Scan
      tsobj.set_value( fkeys[ 1 ], tcmetas[ jjc ].time     );  // Time
      tsobj.set_value( fkeys[ 2 ], tcmetas[ jjc ].rpm      );  // RawSpeed
      tsobj.set_value( fkeys[ 3 ], tcmetas[ jjc ].drpm     );  // SetSpeed
      tsobj.set_value( fkeys[ 4 ], tcmetas[ jjc ].w2t      );  // Omega2T
      tsobj.set_value( fkeys[ 5 ], tcmetas[ jjc ].tempera  );  // Temperature
      tsobj.set_value( fkeys[ 6 ], tcmetas[ jjc ].pressure );  // Pressure
      tsobj.set_value( fkeys[ 7 ], tdmetas[ jjd ].period   );  // Period
      tsobj.set_value( fkeys[ 8 ], tdmetas[ jjd ].jitter   );  // Jitter

      // Write the scan record
      tsobj.flush_record();
   }

   // Complete write of TMST file and defining XML
   if ( tsobj.close_write_data() == 0 )
   {
      tsobj.write_defs();
      nfiles        += 2;
   }

   return nfiles;
}

// Return a count of a specified type
int US_CfaData::countOf( QString key )
{
   mapCounts();

   return counts[ key ];
}

// Return the channel description string for a given cell/channel
QString US_CfaData::cc_description( QString celchn )
{
   int ccx = cellchans.indexOf( celchn );
   return ( ccx < 0 ? "" : ccdescs.at( ccx ) );
}

// Return the runID and runType strings for the data
void US_CfaData::run_values( QString& arunid, QString& aruntype )
{
   arunid   = runID;
   aruntype = is_absorb ? "RA" : "RI";
}

// Private slot to map counts and sizes
void US_CfaData::mapCounts( void )
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
   counts[ "slambda"   ]  = slambda;
   counts[ "elambda"   ]  = elambda;
}

// Scan the CFA .sqlite file and return its values
void US_CfaData::scan_cfafile( const QString dbpath, const int sctp )
{
   tametas.clear();     // Clear table value vectors
   tsmetas.clear();
   twvlens.clear();
   tangles.clear();
   tcmetas.clear();
   tinstrs.clear();
   tscans .clear();
   tdmetas.clear();
   tradius.clear();
   tsampls.clear();
   twvlnss.clear();
QDateTime time00=QDateTime::currentDateTime();

   dbcfa           = QSqlDatabase::addDatabase( "QSQLITE" );
   dbcfa.setDatabaseName( dbpath );

   if ( dbcfa.open() )
   {
      qDebug() << "Open Success for db" << dbpath;
   }
   else
   {
      qDebug() << "Open Failure for db" << dbpath;
      return;
   }

   QStringList sqtabs = dbcfa.tables( QSql::Tables );
   sqtabs << "Scans";
   int ntabs        = sqtabs.count();
DbgLv(1) << "CfDa:scn: ntabs" << ntabs << "tab0 tabn"
 << sqtabs[0] << sqtabs[ntabs-1];
QDateTime time01=QDateTime::currentDateTime();
QVector< QDateTime > stimes;
QVector< QDateTime > etimes;
stimes.fill( time01, ntabs );
etimes.fill( time01, ntabs );

   // Accumulate statistics and some values from the tables
   QSqlQuery  sqry;
   QSqlRecord qrec;

   for ( int ii = 0; ii < ntabs; ii++ )
   {  // Parse each table; save counts and many of the values
QDateTime time10=QDateTime::currentDateTime();
stimes[ii]=time10;
etimes[ii]=time10;
      QString dbtab = sqtabs[ ii ];
      qrec          = dbcfa.record( dbtab );
      int cols      = qrec.count();
      QStringList cnames;

      for ( int col = 0; col < cols; col++ )
         cnames << qrec.fieldName( col );     // Get column names

DbgLv(1) << "CfDa:scn: ii" << ii << "cnames" << cnames;
      int jisc      = cnames.indexOf( "ScanID"       );
      int jtsc      = cnames.indexOf( "ScanType"     );
      int jibl      = cnames.indexOf( "BlankID"      );
      int jida      = cnames.indexOf( "DataID"       );
      int jino      = cnames.indexOf( "NoLightID"    );
      int jwvl      = cnames.indexOf( "Wavelength"   );
      int jabx      = cnames.indexOf( "AbsIndex"     );
      int jstm      = cnames.indexOf( "StartTime"    );
      int jetm      = cnames.indexOf( "EndTime"      );
      int jcel      = cnames.indexOf( "Cell"         );
      int jchn      = cnames.indexOf( "Channel"      );
      int javg      = cnames.indexOf( "Averaging"    );
      int jfrc      = cnames.indexOf( "FirstRecord"  );
      int jlrc      = cnames.indexOf( "LastRecord"   );
      int jscn      = cnames.indexOf( "ScanNumber"   );
      int jgai      = cnames.indexOf( "Gain"         );
      int joff      = cnames.indexOf( "Offset"       );
      int jdly      = cnames.indexOf( "Delay"        );
      int jdur      = cnames.indexOf( "Duration"     );
      int jrpm      = cnames.indexOf( "Rpm"          );
      int jdrp      = cnames.indexOf( "DesiredRpm"   );
      int jtem      = cnames.indexOf( "ActualTemperature"  );
      int jdte      = cnames.indexOf( "DesiredTemperature" );
      int jpre      = cnames.indexOf( "Pressure"     );
      int jtim      = cnames.indexOf( "Time"         );
      int jw2t      = cnames.indexOf( "OmegaSqT"     );
      int jacc      = cnames.indexOf( "Acceleration" );
      int jdec      = cnames.indexOf( "Deceleration" );
      int jtyp      = cnames.indexOf( "Type"         );
      int jitm      = cnames.indexOf( "Installed"    );
      int jotm      = cnames.indexOf( "OperatingTime"   );
      int jlst      = cnames.indexOf( "LastService"     );
      int jttm      = cnames.indexOf( "TotalTime"       );
      int jmag      = cnames.indexOf( "Magnification"   );
      int jnpx      = cnames.indexOf( "NumberOfPixels"  );
      int jtpx      = cnames.indexOf( "TopPixel"        );
      int jbpx      = cnames.indexOf( "BottomPixel"     );
      int jcpx      = cnames.indexOf( "CenterPixel"     );
      int jsrv      = cnames.indexOf( "ServiceInterval" );
      int jsrc      = cnames.indexOf( "SourceID"      );
      int jdet      = cnames.indexOf( "DetectorID"    );
      int jmac      = cnames.indexOf( "MachineID"     );
      int jdsc      = cnames.indexOf( "Description"   );
      int jval      = cnames.indexOf( "Intensity"     );
      int jstd      = cnames.indexOf( "Std"           );
      int jcnt      = cnames.indexOf( "Count"         );
      int jhic      = cnames.indexOf( "HiClock"       );
      int jper      = cnames.indexOf( "Period"        );
      int jjit      = cnames.indexOf( "Jitter"        );
      int jjst      = cnames.indexOf( "JitterStd"     );
      int jco1      = cnames.indexOf( "Coef1"         );
      int jco2      = cnames.indexOf( "Coef2"         );
      int jco3      = cnames.indexOf( "Coef3"         );
      int jco4      = cnames.indexOf( "Coef4"         );
      int jico      = cnames.indexOf( "ComponentID"   );
      int jiso      = cnames.indexOf( "SolventID"     );
      int jiun      = cnames.indexOf( "UnitsID"       );
      int jcon      = cnames.indexOf( "Concentration" );
      int juni      = cnames.indexOf( "UnitsString"   );

      if ( dbtab.startsWith( "sqlite_stat" ) )
         continue;

      sqry          = dbcfa.exec( "SELECT count(*) from " + dbtab );
DbgLv(1) << "CfDa:scn: ii" << ii << "  sqry exec for" << dbtab;
      sqry.next();
      int rows      = sqry.value( 0 ).toInt();
DbgLv(1) << "CfDa:scn: ii" << ii << "  rows" << rows;
      int strows    = 0;
      totals[ dbtab ] = rows;
DbgLv(1) << "CfDa:scn: ii" << ii << "  totals" << totals[dbtab];

      if ( rows < 1 )
      {
DbgLv(1) << "CfDa:scn: ii" << ii << "dbtab" << dbtab << "rows" << rows
 << "cols" << cols;
         rows            = 0;
         totals[ dbtab ] = rows;
         counts[ dbtab ] = strows;
         continue;
      }

      QString stat_text = tr( "Scanning %1 rows from %2 table..." )
                          .arg( rows ).arg( dbtab );
      emit status_text( stat_text );

      QString qtxt  = ( dbtab != "ScanMeta" ) ?
                      QString( "SELECT * from " ) + dbtab :
                      QString( "SELECT * from ScanMeta where"
                               " ScanType=%1" ).arg( sctp );

      if ( dbtab == "Scans" )
      {
         QString tripl = tsmetas[ 0 ].triple;
DbgLv(1) << "CfDa:scn: ii" << ii << "  triple" << tripl;
         int lsc       = 0;

         for ( int jj = 0; jj < tsmetas.count(); jj++ )
            lsc           = qMax( lsc, tsmetas[ jj ].scanNbr );

         int snx       = tscans.count() == 0 ?
                         scan_nbr_index( tripl,   1 ) :
                         scan_nbr_index( tripl, lsc );
DbgLv(1) << "CfDa:scn: ii" << ii << "  snx lsc" << snx << lsc;
         int frec      = tsmetas[ snx ].firstRec;
         int lrec      = tsmetas[ snx ].lastRec;

         qtxt          = QString( "SELECT * from Scans where"
                                  " ID >= %1 AND ID <= %2" )
                         .arg( frec ).arg( lrec );
DbgLv(1) << "CfDa:scn: ii" << ii << "  qtxt" << qtxt;
      }


      sqry          = dbcfa.exec( qtxt );
      int row       = 0;
DbgLv(1) << "CfDa:scn: ii" << ii << "dbtab" << dbtab << "rows" << rows
 << "cols" << cols << "qtxt" << qtxt;

      while( sqry.next() )
      {  // Loop to pick up values from rows of the table
         row++;
         strows++;
         int dbid      = sqry.value( 0 ).toInt();

         if      ( dbtab == "AbsMeta" )
         {
            tAbsMeta datrow;
            datrow.id        = dbid;
            datrow.scanId    = sqry.value( jisc ).toInt();
            datrow.blankId   = sqry.value( jibl ).toInt();
            datrow.dataId    = sqry.value( jida ).toInt();
            datrow.nolightId = sqry.value( jino ).toInt();

            tametas << datrow;
         }

         else if ( dbtab == "ScanMeta" )
         {
            tScanMeta datrow;
            datrow.id        = dbid;
            datrow.scanId    = sqry.value( jisc ).toInt();
            datrow.scanType  = sqry.value( jtsc ).toInt();
            datrow.startTime = sqry.value( jstm ).toDouble();
            datrow.endTime   = sqry.value( jetm ).toDouble();
            datrow.cell      = sqry.value( jcel ).toInt();
            datrow.channel   = sqry.value( jchn ).toInt();
            datrow.avging    = sqry.value( javg ).toInt();
            datrow.firstRec  = sqry.value( jfrc ).toInt();
            datrow.lastRec   = sqry.value( jlrc ).toInt();
            datrow.wavelen   = sqry.value( jwvl ).toInt();
            datrow.scanNbr   = sqry.value( jscn ).toInt();
            datrow.triple    = QString::number( datrow.cell ) + "/"
                             + QString( QChar( 'A' + datrow.channel ) ) + "/"
                             + QString::number( datrow.wavelen );
//if(row<15)
DbgLv(1) << "CfDa:scn: scanID" << datrow.scanId << "scNbr" << datrow.scanNbr
 << "cell chan wvln" << datrow.cell << datrow.channel << datrow.wavelen
 << "fRec lRec" << datrow.firstRec << datrow.lastRec
 << "triple" << datrow.triple;
            tsmetas << datrow;
         }

         else if ( dbtab == "Wavelengths" )
         {
            tWvLens   datrow;
            datrow.id        = dbid;
            datrow.wavelen   = sqry.value( jwvl ).toInt();
            datrow.gain      = sqry.value( jgai ).toDouble();
            datrow.avging    = sqry.value( javg ).toInt();
            twvlens << datrow;
         }

         else if ( dbtab == "Angles" )
         {
            tAngles   datrow;
            datrow.id        = dbid;
            datrow.cell      = sqry.value( jcel ).toInt();
            datrow.channel   = sqry.value( jchn ).toInt();
            datrow.offset    = sqry.value( joff ).toDouble();
            datrow.delay     = sqry.value( jdly ).toDouble();
            datrow.duration  = sqry.value( jdur ).toDouble();
            tangles << datrow;
         }

         else if ( dbtab == "CFAMeta" )
         {
            tCFAMeta  datrow;
            datrow.id        = dbid;
            datrow.scanId    = sqry.value( jisc ).toInt();
            datrow.rpm       = sqry.value( jrpm ).toInt();
            datrow.drpm      = sqry.value( jdrp ).toInt();
            datrow.tempera   = sqry.value( jtem ).toInt();
            datrow.dtempera  = sqry.value( jdte ).toInt();
            datrow.pressure  = sqry.value( jpre ).toDouble();
            datrow.time      = sqry.value( jtim ).toDouble();
            datrow.w2t       = sqry.value( jw2t ).toDouble();
            datrow.accel     = sqry.value( jacc ).toInt();
            datrow.decel     = sqry.value( jdec ).toInt();
            tcmetas << datrow;
         }

         else if ( dbtab == "Instrument" )
         {
            tInstru   datrow;
//DbgLv(1) << "CfDa:scn:     js" << jtyp << jitm << jotm << jstm << jttm
// << jmag << jnpx << jtpx << jbpx << jcpx << jsrv << jsrc << jdet
// << jmac << jdsc;
            datrow.id        = dbid;
            datrow.type      = sqry.value( jtyp ).toInt();
            datrow.insttime  = sqry.value( jitm ).toDouble();
            datrow.opertime  = sqry.value( jotm ).toDouble();
            datrow.servtime  = sqry.value( jlst ).toDouble();
            datrow.totaltime = sqry.value( jttm ).toDouble();
            datrow.magnif    = sqry.value( jmag ).toDouble();
            datrow.npixel    = sqry.value( jnpx ).toInt();
            datrow.toppixel  = sqry.value( jtpx ).toInt();
            datrow.botpixel  = sqry.value( jbpx ).toInt();
            datrow.cenpixel  = sqry.value( jcpx ).toInt();
            datrow.servinter = sqry.value( jsrv ).toInt();
            datrow.srcId     = sqry.value( jsrc ).toInt();
            datrow.detId     = sqry.value( jdet ).toInt();
            datrow.macId     = sqry.value( jmac ).toInt();
            datrow.descript  = sqry.value( jdsc ).toString();
            tinstrs << datrow;
         }

         else if ( dbtab.startsWith( "Scans" ) )
         {
            tScans    datrow;
            datrow.id        = dbid;
            datrow.scanId    = sqry.value( jisc ).toInt();
            datrow.intensity = sqry.value( jval ).toDouble();
            datrow.stdd      = sqry.value( jstd ).toDouble();
            tscans  << datrow;
         }

         else if ( dbtab == "DIOMeta" )
         {
            tDioMeta  datrow;
            datrow.id        = dbid;
            datrow.scanId    = sqry.value( jisc ).toInt();
            datrow.rpm       = sqry.value( jrpm ).toInt();
            datrow.count     = sqry.value( jcnt ).toInt();
            datrow.hiclock   = sqry.value( jhic ).toInt();
            datrow.period    = sqry.value( jper ).toDouble();
            datrow.jitter    = sqry.value( jjit ).toDouble();
            datrow.jitstd    = sqry.value( jjst ).toDouble();
            tdmetas << datrow;
         }

         else if ( dbtab == "Radius" )
         {
            tRadius   datrow;
            datrow.id        = dbid;
            datrow.coef1     = sqry.value( jco1 ).toDouble();
            datrow.coef2     = sqry.value( jco2 ).toDouble();
            datrow.coef3     = sqry.value( jco3 ).toDouble();
            datrow.coef4     = sqry.value( jco4 ).toDouble();
            tradius << datrow;
         }

         else if ( dbtab == "Samples" )
         {
            tSamples  datrow;
            datrow.id        = dbid;
            datrow.cell      = sqry.value( jcel ).toInt();
            datrow.channel   = sqry.value( jchn ).toInt();
            datrow.compId    = sqry.value( jico ).toInt();
            datrow.solvId    = sqry.value( jiso ).toInt();
            datrow.unitsId   = sqry.value( jiun ).toInt();
            datrow.concen    = sqry.value( jcon ).toDouble();
            datrow.descript  = sqry.value( jdsc ).toString();
            datrow.units     = sqry.value( juni ).toString();
            tsampls  << datrow;
if(row<15)
DbgLv(1) << "CfDa:scn: cell chann" << datrow.cell << datrow.channel
 << "concen" << datrow.concen << "units" << datrow.units
 << "descript" << datrow.descript;
         }

         else if ( dbtab == "WavelengthScan" )
         {
            tWvLnScan datrow;
            datrow.id        = dbid;
            datrow.wavelen   = sqry.value( jwvl ).toInt();
            datrow.absndx    = sqry.value( jabx ).toInt();
            twvlnss << datrow;
         }
      }  // END: row loop

      strows          = row;
      counts[ dbtab ] = strows;
DbgLv(1) << "CfDa:scn:   dbtab" << dbtab << "strows" << strows;
QDateTime time19=QDateTime::currentDateTime();
etimes[ii]=time19;
   }  // END: tables loop
QDateTime time20=QDateTime::currentDateTime();
//*DEBUG TIMINGS*
DbgLv(1) << "CfDa:scn: TIMEMS open DB    " << time00.msecsTo(time01);
for(int jj=0; jj<ntabs; jj++)
{
 DbgLv(1) << "CfDa:scn: TIMEMS table scan "
  << stimes[jj].msecsTo(etimes[jj]) << "table" << sqtabs[jj];
}
DbgLv(1) << "CfDa:scn: TIMEMS total scan " << time00.msecsTo(time20);
//*DEBUG TIMINGS*

   // Now build arrays that summarize the contents of the DB

   cellchans .clear();
   ccdescs   .clear();
   triples   .clear();
   trnodes   .clear();
   wavelns   .clear();
   scanids   .clear();
   scnnbrs   .clear();
   int ndups     = 0;

   for ( int ii = 0; ii < tsmetas.count(); ii++ )
   {  // Use ScanMeta data to get cell/channel, triple, lambda lists
      int icell     = tsmetas[ ii ].cell;
      int ichan     = tsmetas[ ii ].channel;
      int iwavl     = tsmetas[ ii ].wavelen;
      int scnid     = tsmetas[ ii ].scanId;
      int scnnbr    = tsmetas[ ii ].scanNbr;
      QString scell = QString::number( icell );
      QString schan = QString( QChar( 'A' + ichan ) );
      QString swavl = QString::number( iwavl );
      QString cechn = scell + " / " + schan;
      QString tripl = cechn + " / " + swavl;
      QString tnode = scell + "." + schan + "." + swavl;
DbgLv(1) << "CfDa:scn: ii" << ii << "icell ichan schan cechn"
 << icell << ichan << schan << cechn;

      if ( !triples.contains( tripl ) )
      {
         triples << tripl;
         trnodes << tnode;
         ntriple++;
      }

      // Build list of used scan Ids and Nbrs
      int scnnx     = scnnbrs.indexOf( scnnbr );
      if ( scnnx < 0 )
      {
         scnnbrs << scnnbr;              // Save unique ScanNumber
#if 1
         scanids << scnid;               // Save corresponding ScanID
      }
#endif
#if 0
      }

      int scinx     = scanids.indexOf( scnid );
      if ( scinx < 0 )
      {
         scanids << scnid;
      }
      else
      {
DbgLv(1) << "CfDa:scn:   *DUP* scinx" << scinx << "scnid" << scnid
 << "scnnbr" << scnnbr << "tripl" << tripl;
         ndups++;
      }
#endif
   }  // END: ScanMeta records loop

   // Sort triples, then build cell/channel and wavelength lists

   triples  .sort();
   trnodes  .sort();

   for ( int ii = 0; ii < triples.count(); ii++ )
   {
      QString tripl = triples[ ii ];
      QString scell = tripl.section( "/", 0, 0 ).simplified();
      QString schan = tripl.section( "/", 1, 1 ).simplified();
      QString swavl = tripl.section( "/", 2, 2 ).simplified();
      int icell     = scell.toInt();
      int ichan     = QString( "ABCDEFGH" ).indexOf( schan );
      int iwavl     = swavl.toInt();
      QString cechn = scell + " / " + schan;
DbgLv(1) << "CfDa:scn: ii" << ii << "icell ichan schan cechn"
 << icell << ichan << schan << cechn;

      if ( !cellchans.contains( cechn ) )
      {
         cellchans << cechn;
         ncelchn++;
         bool no_desc  = true;

         for ( int jj = 0; jj < tsampls.count(); jj++ )
         {  // Use Samples data to get cell/channel description
            int jcell     = tsampls[ jj ].cell;
            int jchan     = tsampls[ jj ].channel;
DbgLv(1) << "CfDa:scn:   jcell jchan" << jcell << jchan;

            if ( jcell == icell  &&  jchan == ichan )
            {
               ccdescs << tsampls[ jj ].descript;
               no_desc       = false;
DbgLv(1) << "CfDa:scn:     ccdesc" << tsampls[jj].descript;
               break;
            }
         }

         if ( no_desc )
            ccdescs << "";
      }

      if ( !wavelns.contains( iwavl ) )
      {
         nlambda++;
         wavelns << iwavl;
      }
   }

   // Compute radius values

   r_radii.clear();
   a_radii.clear();
   npoint        = tinstrs[ 0 ].npixel;
   int cpixel    = tinstrs[ 0 ].cenpixel;
   double magni  = tinstrs[ 0 ].magnif;
   double coef1  = tradius[ 0 ].coef1;
   double coef2  = tradius[ 0 ].coef2;
   double coef3  = tradius[ 0 ].coef3;
   double coef4  = tradius[ 0 ].coef4;
   double pix1   = (double)cpixel;
   double pix2   = pix1 * pix1;
   double pix3   = pix2 * pix1;
   double ccpix  = coef1 + coef2 * pix1 + coef3 * pix2
                         + coef4 * pix3;
   double rbase  = 6.5;

   for ( int ipix = 1; ipix <= npoint; ipix++ )
   {
      pix1          = (double)ipix;
      pix2          = pix1 * pix1;
      pix3          = pix2 * pix1;
      double cppix  = coef1 + coef2 * pix1 + coef3 * pix2
                            + coef4 * pix3;
      double radius = rbase + ( cppix - ccpix ) * magni;

      r_radii << radius;
DbgLv(1) << "CfDa:scn: ipix" << ipix << "radius" << radius;
   }

//*DEBUG*
DbgLv(1) << "CfDa:scn: ncelchn" << ncelchn << "ntriple" << ntriple
 << "nlambda" << nlambda << "nscanids" << scanids.count()
 << "nscnnbrs" << scnnbrs.count() << "ndups" << ndups
 << "nccdescs" << ccdescs.count();
for(int jj=0;jj<ncelchn;jj++)
 DbgLv(1) << "CfDA:scn:  celchn" << jj+1 << ":" << cellchans[jj]
  << "ccdesc" << ccdescs[jj];
for(int jj=0;jj<nlambda;jj++)
 DbgLv(1) << "CfDA:scn:   lambda" << jj+1 << ":" << wavelns[jj];
for(int jj=0;jj<ntriple;jj++)
 DbgLv(1) << "CfDA:scn:  triple" << jj+1 << ":" << triples[jj]
  << "trnode" << trnodes[jj];
for(int jj=0;jj<scanids.count();jj++)
 DbgLv(1) << "CfDA:scn:  scanid" << jj+1 << ":" << scanids[jj];
for(int jj=0;jj<scnnbrs.count();jj++)
 DbgLv(1) << "CfDA:scn:  scannbr" << jj+1 << ":" << scnnbrs[jj];
int lpnt=npoint-1;
double rrng=(double)lpnt;
double ravg=(r_radii[lpnt]-r_radii[0])/rrng;
DbgLv(1) << "CfDa:scn: npoint" << npoint << "rad1,radn"
 << r_radii[0] << r_radii[lpnt] << "radinc avg" << ravg;
ravg=0.0;
double rilo=9999.0;
double rihi=0.0;
double prvl=r_radii[0];
for(int jj=1;jj<npoint;jj++)
{
 double rval = r_radii[jj];
 double rinc = rval-prvl;
 prvl  = rval;
 ravg += rinc;
 rilo  = qMin(rilo,rinc);
 rihi  = qMax(rihi,rinc);
}
ravg /= rrng;
double r00=r_radii[0];
double r01=r_radii[1];
double rmm=r_radii[npoint/2];
double rmn=r_radii[npoint/2+1];
double rnp=r_radii[lpnt-1];
double rnn=r_radii[lpnt];
DbgLv(1) << "CfDa:scn: r-inc: avg,min,max" << ravg << rilo << rihi;
DbgLv(1) << "CfDa:scn:  0,1:   r,rn,ri" << r00 << r01 << (r01-r00);
DbgLv(1) << "CfDa:scn:  m,+1:  r,rn,ri" << rmm << rmn << (rmn-rmm);
DbgLv(1) << "CfDa:scn:  -1,n:  r,rn,ri" << rnp << rnn << (rnn-rnp);
//*DEBUG*
}

// Return a scan record index within the tScanMeta vector
int US_CfaData::scan_nbr_index( const QString tripl, const int snbr )
{
   int srx      = -1;          // Default:  scan record not found

   for ( int jj = 0; jj < tsmetas.count(); jj++ )
   {  // Match triple, scan type, scan number
      if ( tsmetas[ jj ].triple   == tripl   &&
           tsmetas[ jj ].scanType == sctype  &&
           tsmetas[ jj ].scanNbr  == snbr )
      {  // Return with index to matched triple,scan
         srx          = jj;
         break;
      }
   }
DbgLv(1) << "CfDa:srxS: tripl" << tripl << "snbr" << snbr << "sctp" << sctype
 << "tsm.count" << tsmetas.count() << "srx" << srx;

   return srx;
}

// Return a scan ID index within the tCFAMeta vector
int US_CfaData::scan_id_index( const int scid )
{
   int srx      = 0;           // Default:  scan record is the first one

   for ( int jj = 0; jj < tcmetas.count(); jj++ )
   {  // Match scan ID
      if ( tcmetas[ jj ].scanId == scid )
      {  // Return with index to matched scan ID
         srx          = jj;
         break;
      }
   }
DbgLv(1) << "CfDa:srxC: scid" << scid << "tcm.count" << tcmetas.count()
 << "srx" << srx;

   return srx;
}

// Get data readings from a range of Scans table records
int US_CfaData::get_readings( QVector< double >& readings,
                              const int frec, const int lrec )
{
DbgLv(1) << "CfDa:grd: frec lrec" << frec << lrec;
   if ( frec < 0 )
   {  // Return zeroed readings if scan data not present in DB
      readings.fill( 0.0, npoint );
      return npoint;
   }

   if ( !dbcfa.isOpen() )
      dbcfa.open();

   QSqlQuery  sqry;
   QSqlRecord qrec;
   QString qtxt  = QString( "SELECT * from Scans where"
                            " ID >= %1 and ID <= %2" )
                           .arg( frec ).arg( lrec );

   sqry          = dbcfa.exec( qtxt );
   qrec          = dbcfa.record( "Scans" );
   int cols      = qrec.count();
   QStringList cnames;

   for ( int col = 0; col < cols; col++ )
      cnames << qrec.fieldName( col );     // Get column names

//DbgLv(1) << "CfDa:grd: cnames" << cnames;
   int jidb      = cnames.indexOf( "ID"        );
   int jisc      = cnames.indexOf( "ScanID"    );
   int jval      = cnames.indexOf( "Intensity" );
   int row       = 0;
   readings.clear();

   while( sqry.next() )
   {  // Loop to pick up values from rows of the table
      int dbid      = sqry.value( jidb ).toInt();
      int scnId     = sqry.value( jisc ).toInt();
      double rval   = sqry.value( jval ).toDouble();
      row++;
if (row<3 || row>1022)
DbgLv(1) << "CfDa:grd:   row" << row << "dbid scnId rval"
 << dbid << scnId << rval;

      readings << rval;
   }

   return readings.count();
}

// Compose text with detailed information about the data
QString US_CfaData::runDetails( void )
{
   // Create report string
   QString dbfn   = dbfile.section( "/", -1, -1 );
   int kameta  = tametas.count();
   int ksmeta  = tsmetas.count();
   int kcmeta  = tcmetas.count();
   int kdmeta  = tdmetas.count();
   int kangle  = tangles.count();
   int kscans  = tscans .count();
   int ksampl  = tsampls.count();
   int kwvlss  = twvlnss.count();
   int kwvlns  = twvlens.count();
   int lam     = kameta - 1;
   int lsm     = ksmeta - 1;
   int lcm     = kcmeta - 1;
   int ldm     = kdmeta - 1;
   int lan     = kangle - 1;
   int lsc     = kscans - 1;
   int lsa     = ksampl - 1;
   int lws     = kwvlss - 1;
   int lwv     = kwvlns - 1;

   // Create report string

   QString msg = tr( "CFA Data Statistics for RunID \"%1\",\n" )
                                           .arg( runID );
   msg += tr( " from Directory \"%1\",\n" ).arg( cur_dir );

   if ( is_raw )
   {
      msg += tr( " using file \"%1\" .\n" ).arg( dbfn );
      msg += tr( "\nData Summary.\n" );
   }

   else
   {
      msg += tr( " using files from \"%1\"\n" ) .arg( fnames[ 0 ] );
      msg += tr( "               to \"%1\".\n" ).arg( fnames[ nfile - 1 ] );
      msg += tr( "\nData Summary.\n" );
      msg += tr( "    AUC Files Count:          %1\n" ).arg( nfile );
   }

   msg += tr( "    Triples Count:            %1\n" ).arg( ntriple );
   msg += tr( "    Cell/Channel Count:       %1\n" ).arg( ncelchn );
   msg += tr( "    Radius Points:            %1\n" ).arg( npoint );
   msg += tr( "    Scan Count (max.):        %1\n" ).arg( nscan );
   msg += tr( "    Wavelengths Count:        %1\n" ).arg( nlambda );

   if ( is_raw )
   {  // Input is raw CFA DB
      msg += tr( "\nDatabase Tables.\n" );
      msg += tr( "  Table Name:  'AbsMeta'\n" );
      msg += tr( "    Count:                    %1\n" ).arg( kameta );
      msg += tr( "    First ScanID:             %1\n" )
                .arg( tametas[   0 ].scanId );
      msg += tr( "    Last ScanID:              %1\n" )
                .arg( tametas[ lam ].scanId );
      msg += tr( "    First DataID:             %1\n" )
                .arg( tametas[   0 ].dataId );
      msg += tr( "    Last DataID:              %1\n" )
                .arg( tametas[ lam ].dataId );
      msg += tr( "    First BlankID:            %1\n" )
                .arg( tametas[   0 ].blankId );
      msg += tr( "    Last BlankID:             %1\n" )
                .arg( tametas[ lam ].blankId );
      msg += tr( "    First NoLightID:          %1\n" )
                .arg( tametas[   0 ].nolightId );
      msg += tr( "    Last NoLightID:           %1\n" )
                .arg( tametas[ lam ].nolightId );
      msg += tr( "  Table Name:  'ScanMeta'\n" );
      msg += tr( "    Count:                    %1\n" ).arg( ksmeta );
      msg += tr( "    First ScanID:             %1\n" )
                .arg( tsmetas[   0 ].scanId );
      msg += tr( "    Last ScanID:              %1\n" )
                .arg( tsmetas[ lsm ].scanId );
      msg += tr( "    First ScanType:           %1\n" )
                .arg( tsmetas[   0 ].scanType );
      msg += tr( "    Last ScanType:            %1\n" )
                .arg( tsmetas[ lsm ].scanType );
      msg += tr( "    First Cell:               %1\n" )
                .arg( tsmetas[   0 ].cell );
      msg += tr( "    Last Cell:                %1\n" )
                .arg( tsmetas[ lsm ].cell );
      msg += tr( "    First Channel:            %1\n" )
                .arg( tsmetas[   0 ].channel );
      msg += tr( "    Last Channel:             %1\n" )
                .arg( tsmetas[ lsm ].channel );
      msg += tr( "    First ScanNumber:         %1\n" )
                .arg( tsmetas[   0 ].scanNbr );
      msg += tr( "    Last ScanNumber:          %1\n" )
                .arg( tsmetas[ lsm ].scanNbr );
      msg += tr( "    First Wavelength:         %1\n" )
                .arg( tsmetas[   0 ].wavelen );
      msg += tr( "    Last Wavelength:          %1\n" )
                .arg( tsmetas[ lsm ].wavelen );
      msg += tr( "    First StartTime:          %1\n" )
                .arg( tsmetas[   0 ].startTime );
      msg += tr( "    First EndTime:            %1\n" )
                .arg( tsmetas[   0 ].endTime );
      msg += tr( "    Last StartTime:           %1\n" )
                .arg( tsmetas[ lsm ].startTime );
      msg += tr( "    Last EndTime:             %1\n" )
                .arg( tsmetas[ lsm ].endTime );
      msg += tr( "    First FirstRecord:        %1\n" )
                .arg( tsmetas[   0 ].firstRec );
      msg += tr( "    First LastRecord:         %1\n" )
                .arg( tsmetas[   0 ].lastRec );
      msg += tr( "    Last FirstRecord:         %1\n" )
                .arg( tsmetas[ lsm ].firstRec );
      msg += tr( "    Last LastRecord:          %1\n" )
                .arg( tsmetas[ lsm ].lastRec );
      msg += tr( "  Table Name:  'CFAMeta'\n" );
      msg += tr( "    Count:                    %1\n" ).arg( kcmeta );
      msg += tr( "    First ScanID:             %1\n" )
                .arg( tcmetas[   0 ].scanId );
      msg += tr( "    Last ScanID:              %1\n" )
                .arg( tcmetas[ lcm ].scanId );
      msg += tr( "    First RPM:                %1\n" )
                .arg( tcmetas[   0 ].rpm );
      msg += tr( "    Last RPM:                 %1\n" )
                .arg( tcmetas[ lcm ].rpm );
      msg += tr( "    First DesiredRPM:         %1\n" )
                .arg( tcmetas[   0 ].drpm );
      msg += tr( "    Last DesiredRPM:          %1\n" )
                .arg( tcmetas[ lcm ].drpm );
      msg += tr( "    First Temperature:        %1\n" )
                .arg( tcmetas[   0 ].tempera );
      msg += tr( "    Last Temperature:         %1\n" )
                .arg( tcmetas[ lcm ].tempera );
      msg += tr( "    First Pressure:           %1\n" )
                .arg( tcmetas[   0 ].pressure );
      msg += tr( "    Last Pressure:            %1\n" )
                .arg( tcmetas[ lcm ].pressure );
      msg += tr( "    First Time:               %1\n" )
                .arg( tcmetas[   0 ].time );
      msg += tr( "    Last Time:                %1\n" )
                .arg( tcmetas[ lcm ].time );
      msg += tr( "    First Omega^2T:           %1\n" )
                .arg( tcmetas[   0 ].w2t );
      msg += tr( "    Last Omega^2T:            %1\n" )
                .arg( tcmetas[ lcm ].w2t );
      msg += tr( "  Table Name:  'DIOMeta'\n" );
      msg += tr( "    Count:                    %1\n" ).arg( kdmeta );
      msg += tr( "    First ScanID:             %1\n" )
                .arg( tdmetas[   0 ].scanId );
      msg += tr( "    Last ScanID:              %1\n" )
                .arg( tdmetas[ ldm ].scanId );
      msg += tr( "    First RPM:                %1\n" )
                .arg( tdmetas[   0 ].rpm );
      msg += tr( "    Last RPM:                 %1\n" )
                .arg( tdmetas[ ldm ].rpm );
      msg += tr( "    First Count:              %1\n" )
                .arg( tdmetas[   0 ].count );
      msg += tr( "    Last Count:               %1\n" )
                .arg( tdmetas[ ldm ].count );
      msg += tr( "    First Period:             %1\n" )
                .arg( tdmetas[   0 ].period );
      msg += tr( "    Last Period:              %1\n" )
                .arg( tdmetas[ ldm ].period );
      msg += tr( "    First Jitter:             %1\n" )
                .arg( tdmetas[   0 ].jitter );
      msg += tr( "    Last Jitter:              %1\n" )
                .arg( tdmetas[ ldm ].jitter );
      msg += tr( "  Table Name:  'Angles'\n" );
      msg += tr( "    Count:                    %1\n" ).arg( kangle );
      msg += tr( "    First Cell:               %1\n" )
                .arg( tangles[   0 ].cell );
      msg += tr( "    Last Cell:                %1\n" )
                .arg( tangles[ lan ].cell );
      msg += tr( "    First Channel:            %1\n" )
                .arg( tangles[   0 ].channel );
      msg += tr( "    Last Channel:             %1\n" )
                .arg( tangles[ lan ].channel );
      msg += tr( "    First Offset:             %1\n" )
                .arg( tangles[   0 ].offset );
      msg += tr( "    Last Offset:              %1\n" )
                .arg( tangles[ lan ].offset );
      msg += tr( "    First Delay:              %1\n" )
                .arg( tangles[   0 ].delay );
      msg += tr( "    Last Delay:               %1\n" )
                .arg( tangles[ lan ].delay );
      msg += tr( "    First Duration:           %1\n" )
                .arg( tangles[   0 ].duration );
      msg += tr( "    Last Duration:            %1\n" )
                .arg( tangles[ lan ].duration );
      msg += tr( "  Table Name:  'Instrument'\n" );
      msg += tr( "    Instrument Type:          %1\n" )
                .arg( tinstrs[   0 ].type );
      msg += tr( "    Magnification:            %1\n" )
                .arg( tinstrs[   0 ].magnif );
      msg += tr( "    Number of Pixels:         %1\n" )
                .arg( tinstrs[   0 ].npixel );
      msg += tr( "    Top Pixel:                %1\n" )
                .arg( tinstrs[   0 ].toppixel );
      msg += tr( "    Bottom Pixel:             %1\n" )
                .arg( tinstrs[   0 ].botpixel );
      msg += tr( "    Center Pixel:             %1\n" )
                .arg( tinstrs[   0 ].cenpixel );
      msg += tr( "    Machine ID:               %1\n" )
                .arg( tinstrs[   0 ].macId );
      msg += tr( "    Description:              %1\n" )
                .arg( tinstrs[   0 ].descript );
      msg += tr( "  Table Name:  'Radius'\n" );
      msg += tr( "    Coef1:                    %1\n" )
                .arg( tradius[   0 ].coef1 );
      msg += tr( "    Coef2:                    %1\n" )
                .arg( tradius[   0 ].coef2 );
      msg += tr( "    Coef3:                    %1\n" )
                .arg( tradius[   0 ].coef3 );
      msg += tr( "    Coef4:                    %1\n" )
                .arg( tradius[   0 ].coef4 );
      msg += tr( "  Table Name:  'Samples'\n" );
      msg += tr( "    Count:                    %1\n" ).arg( ksampl );
      msg += tr( "    First Cell:               %1\n" )
                .arg( tsampls[   0 ].cell );
      msg += tr( "    Last Cell:                %1\n" )
                .arg( tsampls[ lsa ].cell );
      msg += tr( "    First Channel:            %1\n" )
                .arg( tsampls[   0 ].channel );
      msg += tr( "    Last Channel:             %1\n" )
                .arg( tsampls[ lsa ].channel );
      msg += tr( "    First Concentration:      %1\n" )
                .arg( tsampls[   0 ].concen );
      msg += tr( "    Last Concentration:       %1\n" )
                .arg( tsampls[ lsa ].concen );
      msg += tr( "    First Description:        %1\n" )
                .arg( tsampls[   0 ].descript );
      msg += tr( "    Last Description:         %1\n" )
                .arg( tsampls[ lsa ].descript );
      msg += tr( "    First Units:              %1\n" )
                .arg( tsampls[   0 ].units );
      msg += tr( "    Last Units:               %1\n" )
                .arg( tsampls[ lsa ].units );
      msg += tr( "  Table Name:  'Scans'\n" );
      msg += tr( "    First+Last Count:         %1"   ).arg( kscans )
               + "   ( " + tsmetas[ 0 ].triple + " )\n";
      msg += tr( "    Total Table Rows:         %1\n" )
                .arg( totals[ "Scans" ] );
      msg += tr( "    First ScanID:             %1\n" )
                .arg( tscans [   0 ].scanId );
      msg += tr( "    Last ScanID:              %1\n" )
                .arg( tscans [ lsc ].scanId );
      msg += tr( "    First Intensity:          %1\n" )
                .arg( tscans [   0 ].intensity );
      msg += tr( "    Last Intensity:           %1\n" )
                .arg( tscans [ lsc ].intensity );
      msg += tr( "    First Int.Std.Dev.:       %1\n" )
                .arg( tscans [   0 ].stdd );
      msg += tr( "    Last Int.Std.Dev.:        %1\n" )
                .arg( tscans [ lsc ].stdd );
      msg += tr( "  Table Name:  'WavelengthScan'\n" );
      msg += tr( "    Count:                    %1\n" ).arg( kwvlss );

      if ( kwvlss > 0 )
      {
         msg += tr( "    First Wavelength:         %1\n" )
                   .arg( twvlnss[   0 ].wavelen );
         msg += tr( "    Last Wavelength:          %1\n" )
                   .arg( twvlnss[ lws ].wavelen );
         msg += tr( "    First AbsIndex:           %1\n" )
                   .arg( twvlnss[   0 ].absndx );
         msg += tr( "    Last AbsIndex:            %1\n" )
                   .arg( twvlnss[ lws ].absndx );
      }

      msg += tr( "  Table Name:  'Wavelengths'\n" );
      msg += tr( "    Count:                    %1\n" ).arg( kwvlns);

      if ( kwvlns > 0 )
      {
         msg += tr( "    First Wavelength:         %1\n" )
                   .arg( twvlens[   0 ].wavelen );
         msg += tr( "    Last Wavelength:          %1\n" )
                   .arg( twvlens[ lwv ].wavelen );
         msg += tr( "    First Gain:               %1\n" )
                   .arg( twvlens[   0 ].gain );
         msg += tr( "    Last Gain:                %1\n" )
                   .arg( twvlens[ lwv ].gain );
         msg += tr( "    First Averaging:          %1\n" )
                   .arg( twvlens[   0 ].avging );
         msg += tr( "    Last Averaging:           %1\n" )
                   .arg( twvlens[ lwv ].avging );
      }
   }  // END: input is raw CFA DB

   else
   {  // Input is AUC files
      QDir tdir( cur_dir, "*", QDir::Name,
                 QDir::Files | QDir::Readable );
      QStringList tmfs = tdir.entryList( QStringList( "*.tmst" ),
         QDir::Files, QDir::Name );
      int ntfile     = tmfs.count();
      int ltr        = ntriple - 1;
      int lwv        = nlambda - 1;
DbgLv(1) << "CfDa:Det: ltr" << ltr << triples.count();
DbgLv(1) << "CfDa:Det: lwv" << lwv << wavelns.count();
      msg += tr( "    First Triple:             %1\n" ).arg( triples[   0 ] );
      msg += tr( "    Last  Triple:             %1\n" ).arg( triples[ ltr ] );
      msg += tr( "    First Wavelength:         %1\n" ).arg( wavelns[   0 ] );
      msg += tr( "    Last  Wavelength:         %1\n" ).arg( wavelns[ lwv ] );
      msg += tr( "\nTime State Information.\n" );
      msg += tr( "  Time State Files\n" );
      msg += tr( "    TMST File Count:          %1\n" ).arg( ntfile );

      if ( ntfile > 0 )
      {  // TMST file exists, so report on it
         QString tmfn   = tmfs[ 0 ];
         QString xmfn   = QString( tmfn ).replace( ".tmst", ".xml" );
         QString tpath  = cur_dir + tmfn;
         QString xpath  = cur_dir + xmfn;
DbgLv(1) << "CfDa:Det: tmfn" << tmfn;
DbgLv(1) << "CfDa:Det: xmfn" << xmfn;
         int nxfile     = QFile( xpath ).exists() ? 1 : 0;
         msg += tr( "    TM XML File Count:        %1\n" ).arg( nxfile );
         msg += tr( "    TMST File Name:           %1\n" ).arg( tmfn );
         msg += tr( "    Definition File Name:     %1\n" ).arg( xmfn );
      // msg += tr( "012345678901234567890123458789%1\n" )
         const int flen = 30;
         const QString spad( "                           " );
         US_TimeState* tsobj = new US_TimeState();
DbgLv(1) << "CfDa:Det: tpath" << tpath;
         tsobj->open_read_data( tpath );
DbgLv(1) << "CfDa:Det:  open:lemsg" << tsobj->last_error_message();
         QStringList fkeys;
         QStringList ffmts;
         int nrec       = tsobj->time_count();
         tsobj->field_keys( &fkeys, &ffmts );
DbgLv(1) << "CfDa:Det:  fkey:lemsg" << tsobj->last_error_message();
         int nkey       = fkeys.count();
DbgLv(1) << "CfDa:Det: nrec nkey" << nrec << nkey;
         msg += tr( "  Time State Values\n" );
         msg += tr( "    Total Time Records:       %1\n" ).arg( nrec );

         for ( int jj = 0; jj < 2; jj++ )
         {
            QString ixst( ( jj == 0 ) ? "First" : "Last" );
            int rcx        = ( jj == 0 ) ? 0 : nrec - 1;
            tsobj->read_record( rcx );

            for ( int kk = 0; kk < nkey; kk++ )
            {
               QString ckey   = fkeys[ kk ];
               QString cfmt   = ffmts[ kk ].left( 1 );
               QString ctitl  = QString( "    " ) + ixst + " " + ckey + ":";
               QString cval( "???" );

               if ( cfmt == "I" )
                  cval        = QString::number( tsobj->time_ivalue( ckey ) );
               if ( cfmt == "F" )
                  cval        = QString::number( tsobj->time_dvalue( ckey ) );
               if ( cfmt == "C" )
                  cval        = tsobj->time_svalue( ckey );

               QString cmsg   = QString( ctitl + spad ).left( flen )
                                + cval + "\n";
               msg += cmsg;
            }
         }
      }
   }

   return msg;
}

