//! \file us_mwl_data.cpp

#include "us_mwl_data.h"
#include "us_util.h"
#include "us_settings.h"
#include "us_math2.h"
#include "us_memory.h"

// Hold data read in and selected from a raw MWL data directory
US_MwlData::US_MwlData( )
{
   clear();                // Clear internal vectors
   curccx      = 0;        // Current cell/channel index
   dbg_level  = US_Settings::us_debug();
DbgLv(0) << "MwDa: dbg_level" << dbg_level;
}

// Import MWL data from a selected local directory
bool US_MwlData::import_data( QString& mwldir, QLineEdit* lestat )
{
   bool status  = true;
   int nlmb_dup = 0;
   cur_dir      = mwldir;
   le_status    = lestat;
   evers        = 0.0;
   is_absorb    = false;
DbgLv(1) << "MwDa: cur_dir" << cur_dir;

   QDir ddir( cur_dir, "*", QDir::Name, QDir::Files | QDir::Readable );
   ddir.makeAbsolute();
   if ( cur_dir.right( 1 ) != "/" )  cur_dir += "/";

   read_runxml( ddir, cur_dir );
DbgLv(1) << "MwDa: evers" << evers << "is_absorb" << is_absorb;

   int kcelchn  = cellchans.count();

   if ( kcelchn == 0 )
   {
      QMessageBox::critical( 0,
            tr( "No mwrs XML File Found" ),
            tr( "No proper mwrs XML file was found in directory \"%1\".\n"
                "No import of MWRS data is possible without"
                " such a file." ).arg( cur_dir ) );
      return false;
   }

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
   QVector< int > ccscans;
   cells    .clear();
   cellchans.clear();
   nscan         = -1;
   int scnmin    = 99999;
   mwrfs.sort();
   nfile         = mwrfs.size();
   ccscans.fill( 0, nfile );
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
      nscan           = qMax( nscan,  scann );
      scnmin          = qMin( scnmin, scann );

      fnames << fname;
      fpaths << fpath;

      QString celchn  = acell + "." + chann;

      int      ccx    = cellchans.indexOf( celchn );
      if ( ccx < 0 )
      {
         ccx             = cellchans.size();
         cellchans << celchn;
      }

      ccscans[ ccx ]  = qMax( ccscans[ ccx ], scann );
   }

   nscan       = ( nscan - scnmin ) + 1;
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

   if ( ( ncelchn * nscan ) != nfile )
   {
      qDebug() << "*** nfile" << nfile << "ncelchn*nscn" << ncelchn*nscan;
      int scn_lo  = ccscans[ 0 ];
      int scn_hi  = scn_lo;

      for ( int ii = 1; ii < cellchans.size(); ii++ )
      {
         scn_lo      = qMin( scn_lo, ccscans[ ii ] );
         scn_hi      = qMax( scn_hi, ccscans[ ii ] );
      }

      scn_lo      = ( scn_lo - scnmin ) + 1;
      scn_hi      = ( scn_hi - scnmin ) + 1;

      if ( scn_lo != scn_hi )
      {
         QApplication::restoreOverrideCursor();
         QApplication::restoreOverrideCursor();

         int response = QMessageBox::warning( 0,
               tr( "Varying Number of Scans" ),
               tr( "The number of scans in each cell varies"
                   " from %1 to %2.<br/><br/>Select <b>[ Cancel ]</b>"
                   " to abort the import<br/>or <b>[ OK ]</b> to"
                   " process only the first %3 scans of each cell." )
               .arg( scn_lo ).arg( scn_hi ).arg( scn_lo ),
               QMessageBox::Cancel|QMessageBox::Ok, QMessageBox::Ok );

         if ( response == QMessageBox::Cancel )
            return false;

         QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
      }
      nscan       = scn_lo;
   }

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
      QString acell   = fname.section( ".", -5, -5 );
      QString chann   = fname.section( ".", -4, -4 );
      QString ascan   = fname.section( ".", -2, -2 );
      int scann       = ascan.toInt() - scnmin + 1;
      QString celchn  = acell + " / " + chann;

      if ( scann > nscan )  continue;

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
         nlamb_i     = hd.nlambda;
         ntrip_i     = nlamb_i * ncelchn;
         nlambda     = nlamb_i;
         ntriple     = ntrip_i;
         npoint      = hd.npoint;
         npointt     = npoint  * nscan;
         slambda     = 0;
DbgLv(1) << "MwDa: npoint nlambda" << npoint << nlambda;

         read_lambdas( ds, ri_wavelns, nlamb_i );

         // Test for duplicate wavelengths
         for ( int jj = 1; jj < nlamb_i; jj++ )
         {
            if ( ri_wavelns[ jj ] == ri_wavelns[ jj - 1 ] )
            {  // Mark dup with zero value for now; and bump dupl. count
               ri_wavelns[ jj - 1 ] = 0;
               nlmb_dup++;
DbgLv(1) << "MwDa:   jj" << jj << "ndup lmb" << nlmb_dup << ri_wavelns[jj]; 
            }
            else if ( slambda == 0 )
            {  // Save start non-duplicate lambda
               slambda              = ri_wavelns[ jj - 1 ];
            }
         }
DbgLv(1) << "MwDa:   read_lambdas COMPLETE";
         slambda     = ( slambda == 0 ) ? ri_wavelns[ 0 ] : slambda;
         elambda     = ri_wavelns[ nlamb_i - 1 ];
         nlambda     = nlamb_i - nlmb_dup;
         ntriple     = nlambda * ncelchn;
int ww=nlamb_i-1;
DbgLv(1) << "MwDa:    w0 w1 w3 wi wj wk" << ri_wavelns[0] << ri_wavelns[1]
 << ri_wavelns[2] << ri_wavelns[ww-2] << ri_wavelns[ww-1] << ri_wavelns[ww];
         if ( nlmb_dup == 0 )
         {
            le_status->setText( tr( "%1 wavelengths ..." )
                                .arg( nlamb_i ) );
         }
         else
         {
            le_status->setText( tr( "%1 wavelengths ..."
                                    " (%2 duplicate(s) removed)" )
                                .arg( nlambda ).arg( nlmb_dup ) );
         }
         qApp->processEvents();

         // And initialize the data vector
         QVector< double > wave_reads( npointt, 0.0 );
         ri_readings.clear  ();
         ri_readings.reserve( ntriple );

         for ( int tx = 0; tx < ntriple; tx++ )
         {
            ri_readings << wave_reads;
         }
DbgLv(1) << "MwDa:   ri_readings CREATED size" << ri_readings.size();
      }
      else
      {  // Otherwise, skip past wavelengths
         ds.skipRawData( nlamb_i * 2 );
      }

      int ccx    = cellchans.indexOf( celchn );
DbgLv(1) << "MwDa:  cell chann celchn ccx" << acell << chann << celchn << ccx;
DbgLv(1) << "MwDa:   celchn size cc0 ccn" << cellchans.size()
 << cellchans[0] << cellchans[cellchans.size()-1];
      ccscans[ ccx ] = ccscans[ ccx ] + 1;
      int tripx  = ccx * nlambda;
      int scnx   = ( fname.section( ".", -2, -2 ).toInt() - scnmin ) * npoint;
DbgLv(1) << "MwDa:  PREPARE rdata ccx tripx scnx" << ccx << tripx << scnx;
DbgLv(1) << "MwDa:  PREPARE   icell ichan nchan" << hd.icell << hd.ichan
 << nchan << "channel" << hd.channel;

      // Read in the radius point data
      for ( int wavx = 0; wavx < nlamb_i; wavx++ )
      {
         if ( ri_wavelns[ wavx ] > 0 )
            read_rdata( ds, ri_readings[ tripx++ ], scnx, npoint );
         else
            ds.skipRawData( npoint * 4 );
      }

      le_status->setText( QString( "Data in for triple %1, scan %2 ..." )
          .arg( tripx + 1 ).arg( scnx + 1 ) );
      qApp->processEvents();
//DbgLv(1) << "MwDa:    read_data COMPLETE";
   } // END: header read loop

DbgLv(1) << "MwDa: wv0 wvm wvn" << ri_wavelns[0]
 << ri_wavelns[nlamb_i/2] << ri_wavelns[nlamb_i-1];
#if 0
DbgLv(1) << "MwDa: da20,40" << ri_readings[20][40] << "m+40 n-40"
 << ri_readings[20][npointt/2+40] << ri_readings[20][npointt-41];
#endif
   if ( nlmb_dup == 0 )
   {
      le_status->setText( tr( "Initial MWL import from %1 files is complete." )
                          .arg( nfile ) );
   }
   else
   {
      le_status->setText( tr( "Initial MWL import from %1 files is complete."
                              " (%2 duplicate(s) encountered!)" )
                          .arg( nfile ).arg( nlmb_dup ) );
      int kk     = 0;

      for ( int ii = 0; ii < nlamb_i; ii++ )
      {
         if ( ri_wavelns[ ii ] > 0 )
            ri_wavelns[ kk++ ] = ri_wavelns[ ii ];
      }

      ri_wavelns.resize( nlambda );
      nlamb_i    = nlambda;
      ntrip_i    = ntriple;
      status     = false;
   }
   qApp->processEvents();

   // Initialize the wavelengths lists for all channels
   ex_wavelns.clear();

   for ( int cc = 0; cc < ncelchn; cc++ )
   {
      ex_wavelns << ri_wavelns;
   }

DbgLv(1) << "MwDa: status" << status << " stat_text" << le_status->text();
   return status;
}

// Load internal values from a vector of loaded rawDatas
void US_MwlData::load_mwl( QVector< US_DataIO::RawData >& allData )
{
   QStringList chans;
   nfile    = allData.size();
   ntriple  = nfile;
   ntrip_i  = nfile;
   ntriple  = ntrip_i;
   nscan    = allData[ 0 ].scanCount();
   npoint   = allData[ 0 ].pointCount();
   npointt  = npoint * nscan;
DbgLv(1) << "MwDa:LdMw: nfile" << nfile << "nscan npoint" << nscan << npoint;

   ri_readings.clear();
   ri_wavelns .clear();
   ex_wavelns .clear();
   cells      .clear();
   cellchans  .clear();
   ccdescs    .clear();
   triples    .clear();
#if 0
   QString cech_p      = "";
   int iwvl_p          = 0;
#endif

   for ( int trx = 0; trx < nfile; trx++ )
   {
      US_DataIO::RawData* edata = &allData[ trx ];

      nscan               = edata->scanCount();
      npoint              = edata->pointCount();
      npointt             = npoint * nscan;
      int     icell       = edata->cell;
      QString cell        = QString::number( icell );
      QString chan        = QString( edata->channel );
      QString celchn      = cell + " / " + chan;
      int     iwvl        = qRound( edata->scanData[ 0 ].wavelength );
#if 0
      int     kwvl        = iwvl;

      if ( iwvl < 300  &&  iwvl_p > 655  &&  celchn == cech_p )
      {  // Handle wavelength wrap-around for values > 835 
         iwvl               += 655;
DbgLv(1) << "MwDa:L  trx" << trx << "kwvl iwvl_p iwvl cech cech_p"
 << kwvl << iwvl_p << iwvl << celchn << cech_p;
         double wvlen        = (double)iwvl;

         for ( int jj = 0; jj < edata->scanCount(); jj++ )
         {
            edata->scanData[ jj ].wavelength = wvlen;
         }
      }

      cech_p              = celchn;
      iwvl_p              = iwvl;
#endif
      QString wavl        = QString::number( iwvl );
      QString triple      = celchn + " / " + wavl;
DbgLv(1) << "MwDa:LdMw:  trx" << trx << "triple" << triple
 << "nscan npoint npointt" << nscan << npoint << npointt;

      if ( ! cells.contains( cell ) )
         cells << cell;

      if ( ! chans.contains( chan ) )
         chans << chan;

      if ( ! cellchans.contains( celchn ) )
      {
         cellchans << celchn;
         ccdescs   << edata->description;
      }

      if ( ! triples.contains( triple ) )
         triples << triple;

      if ( ! ri_wavelns.contains( iwvl ) )
      {
         ri_wavelns << iwvl;
      }
else if ( chans.size() < 2 )
{
int wx=ri_wavelns.indexOf(iwvl);
QString tripd=triples[wx];
DbgLv(1) << "MwDa:L  trx" << trx << "duplicate iwvl" << iwvl << triple
 << "dup wx" << wx << "trip[wx]" << tripd;
}

      for ( int ss = 0; ss < nscan; ss++ )
      {
         ri_readings << edata->scanData[ ss ].rvalues;
      }
   }

   ncell    = cells.size();
   nchan    = chans.size();
   ncelchn  = cellchans.size();
   nlamb_i  = ri_wavelns.size();;
   nlambda  = nlamb_i;
   slambda  = ri_wavelns[ 0 ];
DbgLv(1) << "MwDa:LdMw: ncell nchan nlambda" << ncell << nchan << nlambda;
DbgLv(1) << "MwDa:LdMw:   ncelchn ntriple" << ncelchn << ntriple;
int klam=ntriple/ncelchn;
if(nlambda>(klam+2))
 DbgLv(1) << "MwDa:LdMw:   wl k.."
  << ri_wavelns[klam-2]
  << ri_wavelns[klam-1]
  << ri_wavelns[klam  ]
  << ri_wavelns[klam+1]
  << ri_wavelns[klam+2];

   if ( ( nlambda * ncelchn ) == ntriple )
   {  // If all cells have same wavelengths, just duplicate list
DbgLv(1) << "MwDa:LdMw:   All Cells Same #Wavelengths";
DbgLv(1) << "MwDa:LdMw:    ri_wavelns size" << ri_wavelns.size();
      for ( int ccx = 0; ccx < ncelchn; ccx++ )
      {
         ex_wavelns << ri_wavelns;
      }
   }

   else
   {  // If wavelength lists vary, we must build them carefully
      QVector< int > wvs;
DbgLv(1) << "MwDa:LdMw:   All Cells NOT Same #Wavelengths";

      for ( int ccx = 0; ccx < ncelchn; ccx++ )
      {  // First fill with empty vectors
         ex_wavelns << wvs;
      }

      for ( int trx = 0; trx < ntriple; trx++ )
      {  // Fill each cells list with only the wavelengths it has
         QString triple  = triples[ trx ];
         QString celchn  = triple.section( " / ", 0, 1 );
         int     iwvl    = triple.section( " / ", 2, 2 ).toInt();
         int     ccx     = cellchans.indexOf( celchn );
//DbgLv(1) << "MwDa:LdMw:    trx" << trx << "ccx" << ccx << "celchn" << celchn;

         if ( ccx < 0 )
         {
            qDebug() << "load_mwl:*ERROR* unexpected missing cell" << celchn;
            continue;
         }

         ex_wavelns[ ccx ] << iwvl;
      }
   }
DbgLv(1) << "MwDa:LdMw:   wlns size" << ex_wavelns.size();
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
int US_MwlData::lambdas( QVector< int >& wls, int ccx )
{
   set_celchnx( ccx );
   nlambda    = ex_wavelns[ curccx ].count();

   wls.clear();
   wls.reserve( nlambda );

   for ( int ii = 0; ii < nlambda; ii++ )
   {
      wls << ex_wavelns[ curccx ][ ii ];
   }

   return nlambda;
}

// Return the input raw lambdas vector for the data
int US_MwlData::lambdas_raw( QVector< int >& wls )
{
   wls.clear();
   wls.reserve( nlamb_i );

   for ( int ii = 0; ii < nlamb_i; ii++ )
   {
      wls << ri_wavelns[ ii ];
   }

   return nlamb_i;
}

// Private slot to clear arrays
void US_MwlData::clear()
{
   ri_readings.clear();     // Raw input readings
   ri_wavelns .clear();     // Raw input wavelengths
   ex_wavelns .clear();     // Export wavelengths
   headers    .clear();     // MWL file headers

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
   nlamb_i    = 0;
   ntrip_i    = 0;
   nlambda    = 0;
   ntriple    = 0;
   npoint     = 0;
   npointt    = 0;
   slambda    = 0;
   elambda    = 0;
   curccx     = 0;
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
#ifdef Q_OS_LINUX
// The following code is necessary to counter a compiler bug on 64-bit Linux
if(sizeof(long)==8)
{
static int kk=0;
if(((++kk)%1000)==1)
qDebug() << "fval" << *fptr << "   kk" << kk;
}
#endif
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
   char  chary[ 28 ];
   char* cbuf       = (char*)chary;

   int nhbyte       = ( evers == 1.0 ) ? 24 : 26;
   ds.readRawData( cbuf, nhbyte );

   hd.cell          = QChar( '0' | cbuf[ 0 ] );
   hd.channel       = QChar( cbuf[ 1 ] );
   hd.icell         = cells.indexOf( QString( hd.cell ) );
   hd.ichan         = QString( "ABCDEFGH" ).indexOf( hd.channel );
   hd.iscan         = hword( cbuf + 2 );
   hd.set_speed     = hword( cbuf + 4 );

   if ( evers > 1.0 )
   {  // Current version that includes both set_speed and rotor_speed
      hd.rotor_speed   = hword( cbuf + 6 );
      hd.temperature   = (double)( hword( cbuf + 8 ) ) / 10.0;
      hd.omega2t       = dword( cbuf + 10 );
      hd.elaps_time    = iword( cbuf + 14 );
      hd.npoint        = hword( cbuf + 18 );
      hd.radius_start  = (double)( hword( cbuf + 20 ) ) / 1000.0;
      hd.radius_step   = (double)( hword( cbuf + 22 ) ) / 10000.0;
      hd.nlambda       = hword( cbuf + 24 );
   }

   else
   {  // Older version with only rotor_speed (use that as set_speed)
      hd.rotor_speed   = hd.set_speed;
      hd.set_speed     = qRound( hd.rotor_speed * 0.01 ) * 100.0;
//DbgLv(1) << "EVER=1.0:  rspeed sspeed" << hd.rotor_speed << hd.set_speed;
      hd.temperature   = (double)( hword( cbuf + 6 ) ) / 10.0;
      hd.omega2t       = dword( cbuf + 8 );
      hd.elaps_time    = iword( cbuf + 12 );
      hd.npoint        = hword( cbuf + 16 );
      hd.radius_start  = (double)( hword( cbuf + 18 ) ) / 1000.0;
      hd.radius_step   = (double)( hword( cbuf + 20 ) ) / 10000.0;
      hd.nlambda       = hword( cbuf + 22 );
   }
}

// Utility to read the lambdas from a data stream
void US_MwlData::read_lambdas( QDataStream& ds, QVector< int >& wvs,
      int& klambda )
{
   char cbuf[ 4 ];

   wvs.reserve( klambda );

   for ( int ii = 0; ii < klambda; ii++ )
   { // Pick up each 2-byte value and store as integer
      ds.readRawData( cbuf, 2 );
      int    wvv   = hword( cbuf );
      wvs << wvv;
   }
}

// Utility to read the radius point data from a data stream
void US_MwlData::read_rdata( QDataStream& ds, QVector< double >& rvs,
      int& scnx, int& npoint )
{
   char cbuf[ 4 ];
   int  kk      = scnx;
   // Scale by 1.0 or 1/1000 depending on version
   double rscl  = ( evers > 1.2 ) ? 1.0 : 0.001;
   // In latest versions, scale by 1/10000 if Absorbance
   rscl         = ( ( evers > 1.3 ) && is_absorb ) ? 0.0001 : rscl;

   for ( int ii = 0; ii < npoint; ii++ )
   { // Pick up each 4-byte value, convert to double and possibly scale
      ds.readRawData( cbuf, 4 );
      int    ival  = iword( cbuf );
      double rvv   = (double)ival * rscl;
      rvs[ kk++ ]  = rvv;
   }
//if(scnx<1)
//DbgLv(1) << "MwDa:ReadRdata evers absorb" << evers << is_absorb
// << "rscl ival rvv" << rscl << iword(cbuf) << rvs[kk-1];
}

// Set Lambda ranges for export
int US_MwlData::set_lambdas( int start, int end, int ccx )
{
   set_celchnx( ccx );
DbgLv(1) << "MwDa:SetLamb  s/e" << start << end;
   if ( ex_wavelns[ curccx ].count() == 0 )
   {  // If out list is empty, build from input
      slambda       = ( start > 0 ) ? start : ri_wavelns[ 0 ];
      elambda       = ( end   > 0 ) ? end   : ri_wavelns[ nlamb_i - 1 ];
      ex_wavelns[ curccx ].clear();

      // Set up export lambdas
      int    wvxs   = indexOfLambda( slambda );
      int    wvxe   = indexOfLambda( elambda );
      nlambda       = wvxe - wvxs + 1;
      int    wvx    = wvxs;

      while ( wvx <= wvxe )
      {  // Duplicate lambdas from the range of raw lambdas
         ex_wavelns[ curccx ] << ri_wavelns[ wvx++ ];
      }
DbgLv(1) << "SetLamb  (2)n" << nlambda << wvxs << wvxe << wvx;
   }

   else
   {  // If out list exists, pair it down to the new range
      QVector< int > wkwaves = ex_wavelns[ curccx ];
      nlambda       = wkwaves.count();
      int old_start = wkwaves[ 0 ];
      int old_end   = wkwaves[ nlambda - 1 ];
      slambda       = ( start > 0 ) ? start : old_start;
      elambda       = ( end   > 0 ) ? end   : old_end;
      ex_wavelns[ curccx ].clear();
DbgLv(1) << "SetLamb  (3)n" << nlambda << slambda << elambda << "ccx" << curccx;

      // Set up export lambdas
      int    wvxs   = wkwaves.indexOf( slambda );
      int    wvxe   = wkwaves.indexOf( elambda );
             wvxe   = ( wvxe < 0 ) ? ( nlambda - 1 ) : wvxe;
      int    wvx    = wvxs;
DbgLv(1) << "SetLamb   wvxs wvxe" << wvxs << wvxe;

      if ( slambda < old_start )
      {  // If start is before old list, grab some from the original input
         wvxs       = indexOfLambda( slambda );
         slambda    = ri_wavelns[ wvxs++ ];

         while ( slambda < old_start  &&  wvxs < nlamb_i )
         {
            ex_wavelns[ curccx ] << slambda;
            slambda    = ri_wavelns[ wvxs++ ];
         }
         wvx        = 0;
      }

      while ( wvx <= wvxe )
      {  // Duplicate lambdas from the range of previous lambdas
         ex_wavelns[ curccx ] << wkwaves[ wvx++ ];
      }

DbgLv(1) << "SetLamb   elambda old_end" << elambda << old_end;
      if ( elambda > old_end )
      {  // If end is after old list, append some from the original list
         wvxs       = indexOfLambda( old_end ) + 1;
         wvxe       = indexOfLambda( elambda );
         old_end    = ri_wavelns[ wvxs++ ];
DbgLv(1) << "SetLamb     wvx s,e" << wvxs << wvxe << "old_end" << old_end;

         while ( old_end <= elambda )
         {
            ex_wavelns[ curccx ] << old_end;
            if ( wvxs >= nlamb_i )  break;
            old_end    = ri_wavelns[ wvxs++ ];
         }
      }

DbgLv(1) << "SetLamb  (4)n" << nlambda << wvxs << wvxe << wvx;
   }

   nlambda       = ex_wavelns[ curccx ].count();
   slambda       = ex_wavelns[ curccx ][ 0 ];
   elambda       = ex_wavelns[ curccx ][ nlambda - 1 ];
   ntriple       = nlambda * ncelchn;
DbgLv(1) << "SetLamb    s/e/n" << slambda << elambda << nlambda;
   return nlambda;
}

// Set a new lambdas vector for a channel
int US_MwlData::set_lambdas( QVector< int >& wls, int ccx )
{
   set_celchnx( ccx );
   ex_wavelns[ curccx ] = wls;
   nlambda       = wls.count();
   slambda       = wls[ 0 ];
   elambda       = wls[ nlambda - 1 ];

   return nlambda;
}

// Find the index of a lambda value in the input raw list of lambdas
int US_MwlData::indexOfLambda( int lambda )
{
   int    wvx     = ri_wavelns.indexOf( lambda );   // Try exact match

   if ( wvx < 0 )
   {  // If lambda is not in the list, find the nearest to a match
      int    diflow  = 9999;

      for ( int ii = 0; ii < nlamb_i; ii++ )
      {
         int    difval  = qAbs( lambda - ri_wavelns[ ii ] );

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

// Populate the list of RawData objects from raw input MWL data
int US_MwlData::build_rawData( QVector< US_DataIO::RawData >& allData )
{
   const double signif_dif=100.0;
   const int    low_memApc=20;

   allData.clear();

   // Build the radius vector that is constant
   QVector< double > xout;
   double rad_val  = headers[ 0 ].radius_start;
   double rad_inc  = headers[ 0 ].radius_step;
   r_rpms.clear();
   s_rpms.clear();
   a_rpms.clear();
   d_rpms.clear();
DbgLv(1) << "BldRawD radv radi" << rad_val << rad_inc << "npoint" << npoint
 << "  evers" << evers;

   for ( int ii = 0; ii < npoint; ii++ )
   {
      xout << rad_val;
      rad_val  += rad_inc;
   }
DbgLv(1) << "BldRawD   xout size ntrip" << xout.size() << npoint << ntrip_i;

   // Set up the interpolated byte array (all zeroes)
   int    nbytei   = ( npoint + 7 ) / 8;
   QByteArray interpo( nbytei, '\0' );

   // Build a raw data set for each triple
   char   dtype0   = 'R';
   char   dtype1   = is_absorb ? 'A' : 'I';
   int    ccx      = 0;
   int    wvx      = 0;
   int    hdx      = 0;
DbgLv(1) << "BldRawD szs: ccd" << ccdescs.size() << "hdrs" << headers.size()
 << "rds" << ri_readings.size() << ri_readings[0].size()
 << "wvs" << ri_wavelns.size() << "nli nlo" << nlamb_i << nlambda;

   for ( int trx = 0; trx < ntrip_i; trx++ )
   {
DbgLv(1) << "BldRawD     trx" << trx << " building scans... ccx" << ccx;
      US_DataIO::RawData rdata;
      QString uuid_str  = US_Util::new_guid();
      US_Util::uuid_parse( uuid_str, (unsigned char*)rdata.rawGUID );
      // Set triple values
      rdata.type[ 0 ]   = dtype0;
      rdata.type[ 1 ]   = dtype1;
      rdata.cell        = QString( headers[ hdx ].cell ).toInt();
      rdata.channel     = headers[ hdx ].channel.toLatin1();
      rdata.xvalues     = xout;
      int jhx           = hdx; 
      int rdx           = 0;
      int kspstep       = 1;
      int kscx          = 0;
      int nspstep       = 0;
      int kscan         = 0;
      double rpm_min    = 1e+9;
      double rpm_max    = 1e-9;
      double rpm_sum    = 0.0;
      double rpm_setp   = 0.0;
      double rpm_set    = headers[ jhx ].set_speed;
      double rpm_avg    = 0.0;
      double rpm_stdd   = 0.0;
      rdata.description = ccdescs.at( ccx );
DbgLv(1) << "BldRawD      hdx" << hdx << "hdsize" << headers.size();

      for ( int scx = 0; scx < nscan; scx++ )
      {  // Set scan values
         US_DataIO::Scan scan;
if(scx<3 || (scx+4)>nscan)
DbgLv(1) << "BldRawD       scx" << scx << "jhx" << jhx << "wvx" << wvx;
         scan.temperature  = headers[ jhx ].temperature;
         rpm_setp          = rpm_set;
         rpm_set           = headers[ jhx ].set_speed;
         scan.rpm          = headers[ jhx ].rotor_speed;
         scan.seconds      = headers[ jhx ].elaps_time;
         scan.omega2t      = headers[ jhx ].omega2t;
         scan.wavelength   = ri_wavelns[ wvx ];
         scan.delta_r      = rad_inc;
         scan.rvalues.reserve( npoint );
         scan.interpolated = interpo;

         if ( scx > 0  &&  scan.rpm != r_rpms[ scx - 1 ] )
            kspstep++;

if(scx<3 || (scx+4)>nscan)
DbgLv(1) << "BldRawD        rpm_set rpm_setp" << rpm_set << rpm_setp;
         if ( rpm_set != rpm_setp )
         {  // A new speed step is reached, re-do speeds in the previous one
            nspstep++;
            rpm_avg           = rpm_sum / (double)kscan;
            rpm_stdd          = 0.0;

            for ( int jsx = kscx; jsx < scx; jsx++ )
            {
               // Store the average over the speed step as the scan speed
               rdata.scanData[ jsx ].rpm          = (double)qRound( rpm_avg );

               // Compute chi-squared sum
               rpm_stdd         += sq( r_rpms[ jsx ] - rpm_avg );
            }

            // Compute standard deviation of speeds and save step values
            rpm_stdd          = sqrt( rpm_stdd  ) / (double)kscan;
            if ( trx == 0 )
            {
               a_rpms << rpm_avg;
               s_rpms << rpm_setp;
               d_rpms << rpm_stdd;
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
DbgLv(1) << "BldRawD      scx" << scx << "jhx" << jhx
 << "seconds" << scan.seconds << "rpm" << scan.rpm << "ss" << rpm_set
 << "speed step" << kspstep << nspstep+1;
}
//*DEBUG*
         if ( trx == 0 )
            r_rpms << scan.rpm;
         jhx++;
if(scx<3 || (scx+4)>nscan)
DbgLv(1) << "BldRawD        trx jhx rdx" << trx << jhx << rdx;
if(scx<3 || (scx+4)>nscan)
DbgLv(1) << "BldRawD        riread[trx] size" << ri_readings[trx].size();

         for ( int kk = 0; kk < npoint; kk++ )
         {  // Set readings values
            scan.rvalues << ri_readings[ trx ][ rdx++ ];
         } // END: radius points loop

         rdata.scanData << scan;      // Append a scan to a triple
      } // END: scan loop
DbgLv(1) << "BldRawD         EoSl: trx rdx" << trx << rdx;

      if ( evers < 1.2 )
      {  // For an old data version, get the average of all scans
         rpm_min           = 1.e+9;
         rpm_max           = 1.e-9;
         rpm_sum           = 0.0;

         for ( int scx = 0; scx < nscan; scx++ )
         {
            double srpm       = rdata.scanData[ scx ].rpm;
            rpm_min           = qMin( rpm_min, srpm );
            rpm_max           = qMax( rpm_max, srpm );
            rpm_sum          += srpm;
         }

         kscan             = nscan;
         nspstep           = 0;
         kscx              = 0;
         rpm_set           = (double)qRound( ( rpm_min + rpm_max ) * 0.5 );
      }

      // Set the average speed for the final/only speed step
      nspstep++;
      rpm_stdd          = 0.0;
      rpm_avg           = rpm_sum / (double)kscan;
DbgLv(1) << "BldRawD        kscx" << kscx;

      for ( int scx = kscx; scx < nscan; scx++ )
      {
         // Store the average over the speed step as the scan speed
         rdata.scanData[ scx ].rpm          = (double)qRound( rpm_avg );

         // Compute the chi-squared sum
         rpm_stdd         += sq( r_rpms[ scx ] - rpm_avg );
      }

      // Compute standard deviation of speeds and update step values
      rpm_stdd          = sqrt( rpm_stdd  ) / (double)kscan;
DbgLv(1) << "BldRawD        rpm_stdd" << rpm_stdd;

      if ( trx == 0 )
      {
         a_rpms << rpm_avg;
         s_rpms << rpm_set;
         d_rpms << rpm_stdd;

         if ( evers > 1.0 )
         {  // In newer data, a set_speed may differ from the average
            if ( qAbs( rpm_avg - rpm_set ) > signif_dif )
            {
               QMessageBox::warning( 0,
                     tr( "Set/Average Speed Difference" ),
                     tr( "The stored set_speed (%1) and the average of"
                         " recorded rotor_speeds (%2) differ"
                         " significantly!" ).arg( rpm_set ).arg( rpm_avg ) );
            }
         }
      }
//*DEBUG*
if(trx==0) {
 DbgLv(1) << "BldRawD trx=" << trx << "rpm_min rpm_max rpm_avg rpm_set"
    << rpm_min << rpm_max << rpm_avg << rpm_set << "speed steps" << nspstep;
}
//*DEBUG*

DbgLv(1) << "BldRawD     trx" << trx << " saving allData...";
      allData << rdata;               // Append triple data to the array
      le_status->setText( tr( "Of %1 raw AUCs, built %2" )
          .arg( ntriple ).arg( trx + 1 ) );
      qApp->processEvents();
      wvx++;

      if ( wvx >= nlambda )
      {  // After final wavelength, reset at next cell/channel
         ccx++;
         wvx  = 0;
         hdx  = ccx * nscan;

         // Free up some memory if it is getting tight
         int memAv = US_Memory::memory_profile();

         if ( memAv < low_memApc )
         {
            for ( int rr = ( trx - nlambda + 1 ); rr < ( trx + 1 ); rr++ )
               ri_readings[ rr ].clear();
int memAv2 = US_Memory::memory_profile();
DbgLv(1) << "BldRawD  memfree %: 1memAV" << memAv << "2memAV" << memAv2;
         }
      }
DbgLv(1) << "BldRawD   ccx wvx hdx" << ccx << wvx << hdx << headers.size();
   } // END: triple loop

   le_status->setText( tr( "All %1 raw AUCs have been build." )
                       .arg( ntriple ) );
   qApp->processEvents();

DbgLv(1) << "BldRawD  DONE ntriple" << ntriple << ntrip_i;
   return ntriple;
}

// Return a count of a specified type
int US_MwlData::countOf( QString key )
{
   mapCounts();

   return counts[ key ];
}

// Return the channel description string for a given cell/channel
QString US_MwlData::cc_description( QString celchn )
{
   int ccx = cellchans.indexOf( celchn );
   return ( ccx < 0 ? "" : ccdescs.at( ccx ) );
}

// Return the runID and runType strings for the data
void US_MwlData::run_values( QString& arunid, QString& aruntype )
{
   arunid   = runID;
   aruntype = is_absorb ? "RA" : "RI";
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
   counts[ "lamb_i"    ]  = nlamb_i;
   counts[ "trip_i"    ]  = ntrip_i;
   counts[ "point"     ]  = npoint;
   counts[ "point_all" ]  = npointt;
   counts[ "slambda"   ]  = slambda;
   counts[ "elambda"   ]  = elambda;
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

   else if ( nxfile == 0 )
   {
      qDebug() << "*ERROR* '*.mwrs.xml' does not exist in " << curdir;
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
            runID           = att.value( "name"    ).toString();
            is_absorb       = att.value( "take_intensity" ).toString() == "N";
DbgLv(1) << "MwDa:RdXML take_i" << att.value( "take_intensity" ).toString()
 << "evers absorb" << evers << is_absorb;
         }
         else if ( xml.name() == "cell" )
         {
            celi            = att.value( "id"      ).toString();
            celi            = QString::number( celi.toInt() );
         }
         else if ( xml.name() == "channel" )
         {
            chni            = att.value( "id"      ).toString();
            cech            = celi + " / " + chni;
            QString desc    = att.value( "sample"  ).toString();

            if ( ! cellchans.contains( cech ) )
            {
               cellchans << cech;
               ccdescs << desc;
            }
         }
         else if ( xml.name() == "settings_mwrs_experiment" )
         {
            evers           = att.value( "version" ).toString().toDouble();
         }
      }
   }

   is_absorb       = ( evers > 1.3 ) ? is_absorb : false;
   xfi.close();
}

// Set the current cell/channel index
int US_MwlData::set_celchnx( int ccx )
{
//DbgLv(1) << "SetCCX" << ccx;
   if (ccx != -1)
   curccx    = qMax( 0, qMin( ccx, ( ncelchn - 1 ) ) );

   return curccx;
}

// Return the output data index of a wavelength in a channel
int US_MwlData::data_index( int waveln, int ccx )
{
   // Set the current cell/channel index
   set_celchnx( ccx );

   // Initially, data index is wavelength index in current cell/channel
   int datax = qMax( 0, ex_wavelns[ curccx ].indexOf( waveln ) );

   // Bump the data index by the sum of wavelengths preceding the channel
   for ( int ii = 0; ii < curccx; ii++ )
      datax    += ex_wavelns[ ii ].count();

   return datax;
}

// Return the output data index of a wavelength in a channel
int US_MwlData::data_index( QString clambda, int ccx )
{
   return data_index( clambda.toInt(), ccx );
}

// Return the output data index of a wavelength in a channel
int US_MwlData::data_index( QString clambda, QString celchn )
{
   int ccx   = cellchans.indexOf( celchn );
   return data_index( clambda.toInt(), ccx );
}

// Update the speed profile for the current MWL data
int US_MwlData::update_speedsteps( QVector< SP_SPEEDPROFILE >& speedsteps )
{
//   int nsteps  = qMin( speedsteps.size(), s_rpms.size() );
   int nsteps  = s_rpms.size();
DbgLv(1) << "MwDa:uSS: nsteps" << nsteps << speedsteps.size() << s_rpms.size();
   if ( nsteps > 0 )
   {
      int kdecr   = 0;
      const int mxstep = 10;
      speedsteps.resize( nsteps );

      for ( int jj = 0; jj < nsteps; jj++ )
      {
         speedsteps[ jj ].set_speed     = (int)s_rpms[ jj ];
         speedsteps[ jj ].avg_speed     = a_rpms[ jj ];
         speedsteps[ jj ].speed_stddev  = d_rpms[ jj ];
         speedsteps[ jj ].rotorspeed    = qRound( a_rpms[ jj ] );
DbgLv(1) << "MwDa:uSS:   jj" << jj << "set_speed avg_speed speed_stddev"
 << s_rpms[ jj ] << a_rpms[ jj ] << d_rpms[ jj ];
         if ( jj > 0  && 
             speedsteps[ jj ].set_speed < speedsteps[ jj - 1 ].set_speed )
            kdecr++;
      }

      if ( kdecr > 0 )
         nsteps      = -1;
      if ( nsteps > mxstep )
         nsteps      = -2;
   }

   return nsteps;
}

// Return a vector of raw speeds for all MWL data scans
int US_MwlData::raw_speeds( QVector< double >& rrpms )
{
   int nscans  = r_rpms.size();
   rrpms       = r_rpms;
   return nscans;
}

