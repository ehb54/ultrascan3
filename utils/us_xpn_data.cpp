//! \file us_xpn_data.cpp

#include "us_xpn_data.h"
#include "us_util.h"
#include "us_settings.h"
#include "us_math2.h"
#include "us_memory.h"
#include "us_time_state.h"
#include "us_simparms.h"

// Hold data read in and selected from a raw XPN data directory
US_XpnData::US_XpnData( ) {
   clear();                // Clear internal vectors

   dbg_level    = US_Settings::us_debug();
   dbhost       = QString( "bcf.uthscsa.edu" );
   dbport       = 5432;
   dbname       = QString( "AUC_DATA_DB" );
   dbuser       = QString( "aucuser" );
   dbpasw       = QString( "badpasswd" );
   fnzstx       = 0;
   etimoff      = 0;
   sctype       = 1;
   ntscan       = 0;
   runType      = "RI";
DbgLv(0) << "XpDa: dbg_level" << dbg_level;
}

// Connect to a selected database server with XPN data
bool US_XpnData::connect_data( const QString adbname,
                               const QString xpnhost, const int xpnport )
{
   return connect_data( xpnhost, xpnport, adbname );
}

// Connect to a selected database server with XPN data
bool US_XpnData::connect_data( const QString xpnhost, const int xpnport,
   const QString adbname, const QString adbuser, const QString adbpasw )
{
   bool status   = true;
   dbhost        = ( xpnhost.isEmpty() ) ? dbhost : xpnhost;
   dbport        = ( xpnport <= 0 )      ? 5432   : xpnport;
   dbname        = ( adbname.isEmpty() ) ? dbname : adbname;
   dbuser        = ( adbuser.isEmpty() ) ? dbuser : adbuser;
   dbpasw        = ( adbpasw.isEmpty() ) ? dbpasw : adbpasw;
   is_absorb     = false;
   is_raw        = true;

   dbxpn           = QSqlDatabase::addDatabase( "QPSQL", "XpnData" );
DbgLv(1) << "XpDa:cnc: drivers" << dbxpn.drivers();
   dbxpn.setDatabaseName( "XpnData" );
   dbxpn.setHostName    ( dbhost );
   dbxpn.setPort        ( dbport );
   dbxpn.setDatabaseName( dbname  );
   dbxpn.setUserName    ( dbuser  );
   dbxpn.setPassword    ( dbpasw );

   dbxpn.setConnectOptions("connect_timeout=5");
     
   if ( dbxpn.open() )
   {
      qDebug() << "Open Success for db" << dbname << dbhost << dbport;
   }
   else
   {
      qDebug() << "Open Failure for db" << dbname << dbhost << dbport
               << dbxpn.lastError().text();
      return false;
   }

   QStringList qdrvrs = dbxpn.drivers();
DbgLv(1) << "XpDa:cnc: drivers" << qdrvrs;
   QStringList sqtabs = dbxpn.tables( QSql::Tables );
DbgLv(1) << "XpDa:cnc: sqtabs" << sqtabs;

   if ( qdrvrs.contains( "QPSQL" )  &&
        ( sqtabs.contains( "\"AUC_schema\".\"ExperimentRun\"" )  ||
          sqtabs.contains( "AUC_schema.ExperimentRun" ) ) )
   {
      status        = true;
DbgLv(1) << "XpDa:cnc: QPSQL-in-qdrvrs && ExpRun-in-sqtabs";
   }
   else
   {
DbgLv(1) << "XpDa:cnc: qdrvrs|sqtab content error:"
 << (qdrvrs.contains("QPSQL"))
 << (sqtabs.contains( "\"AUC_schema\".\"ExperimentRun\"" ));
      status        = false;
   }
   return status;
}

// Check Experiment status
int US_XpnData::checkExpStatus( QString runid )
{
  if ( ! dbxpn.open() )
    {
      qDebug() << "In XPN:checkExpStatus:: ! dbxpn.open() !!! ";
    }
  
   QString tabname( "ExperimentRun" );
   QSqlQuery  sqry;
   QString schname( "AUC_schema" );
   QString sqtab   = schname + "." + tabname;
   QString qrytab  = "\"" + schname + "\".\"" + tabname + "\"";
  
   QString qrytext = "SELECT \"RunStatus\" from " + qrytab
                        + " WHERE \"RunId\"=" + runid + ";";
   sqry            = dbxpn.exec( qrytext );
   sqry.next();

   qDebug() << "INSIDE CheckExpSTATUS: status: " <<  sqry.value( 0 ).toInt();
   return sqry.value( 0 ).toInt();
}

// Check Experiment status [for autoflow]
int US_XpnData::checkExpStatus_auto( QString runid, bool& o_conn )
{

  qDebug() << "in [checkExpStatus_auto]: Init o_conn status: " << o_conn;
   
  if ( ! dbxpn.open() )
    {
      o_conn = false;
      qDebug() << "XPN: checkExpStatus_auto: !dbxpn.open() !!! runid, o_conn -- "
	       << runid << o_conn;
     
      return false;
    }
    
   QString tabname( "ExperimentRun" );
   QSqlQuery  sqry;
   QString schname( "AUC_schema" );
   QString sqtab   = schname + "." + tabname;
   QString qrytab  = "\"" + schname + "\".\"" + tabname + "\"";
  
   QString qrytext = "SELECT \"RunStatus\" from " + qrytab
                        + " WHERE \"RunId\"=" + runid + ";";
   sqry            = dbxpn.exec( qrytext );
   sqry.next();

   qDebug() << "INSIDE CheckExpSTATUS: status: " <<  sqry.value( 0 ).toInt();
   return sqry.value( 0 ).toInt();
}


void US_XpnData::setEtimOffZero( void )
{
   //etimoff  = 0;
   //fnzstx   = 0;
   DbgLv(1) << "XpDa:sEOZ: EtimOff set 0, 1st-non-zero-speed-time-index to 1";
}

// Query and update data for the latest SystemStatusData table entry
int US_XpnData::update_isysrec( const int runId )
{
   int lastrpm     = -1;
   static QStringList  cnames;
   static QList< int > cxs;
   static int cols = -1;

   QSqlQuery    sqry;
   QSqlRecord   qrec;
   QString sRunId  = QString::number( runId );
   QString sExpTm;
   QString schname( "AUC_schema" );
   QString tabname( "SystemStatusData" );
   QString sqtab, qrytab, qrytext;
   int nnrows      = 0;

   if ( cols < 1 )
   {
      cols            = column_indexes( tabname, cnames, cxs );
   }

   // Count the number of rows now in the data table
   
   sqtab           = schname + "." + tabname;
   qrytab          = "\"" + schname + "\".\"" + tabname + "\"";
   qrytext         = "SELECT count(*) from " + qrytab
                   + " WHERE \"RunId\"=" + sRunId + ";";
   sqry            = dbxpn.exec( qrytext );
   sqry.next();
   nnrows          = sqry.value( 0 ).toInt();

   qDebug() << "Inside xpn_data->update_isysrec: nnrows: " << nnrows;

   if ( nnrows < 1 )
   {  // If no data yet, return with negative speed
      isyrec.dataId   = -1;
      isyrec.runId    = runId;
      isyrec.exptime  = -1;
      isyrec.stageNum = 0;
      isyrec.tempera  = 0.0;
      isyrec.speed    = 0.0;
      isyrec.omgSqT   = 0.0;
      return -1;
   }

   // Otherwise, get the latest SystemStatusData row
   qrytext         = "SELECT * from " + qrytab
                   + " WHERE \"RunId\"=" + sRunId
                   + " ORDER BY \"DataId\" DESC LIMIT 1;";
/*
      flds << "DataId" << "RunId" << "ExperimentStart" << "ExperimentTime"
           << "Temperature" << "RPM" << "OmegaSquaredT" << "StageNum";
      class tbSyData
      {
         public:
            int       dataId;        //!< Data table entry ID
            int       runId;         //!< Run ID
            int       exptime;       //!< Time in seconds from exp. start
            int       stageNum;      //!< Stage number
            double    tempera;       //!< Temperature
            double    speed;         //!< Speed in revs per minute
            double    omgSqT;        //!< OmegaSquaredT
            QDateTime expstart;      //!< Experiment start
      };
*/
   sqry            = dbxpn.exec( qrytext );
   sqry.next();

   isyrec.dataId   = sqry.value( cxs[  0 ] ).toInt();
   isyrec.runId    = sqry.value( cxs[  1 ] ).toInt();
   isyrec.expstart = sqry.value( cxs[  2 ] ).toDateTime();
   isyrec.exptime  = sqry.value( cxs[  3 ] ).toInt();
   isyrec.tempera  = sqry.value( cxs[  4 ] ).toDouble();
   isyrec.speed    = sqry.value( cxs[  5 ] ).toDouble();
   isyrec.omgSqT   = sqry.value( cxs[  6 ] ).toDouble();
   isyrec.stageNum = sqry.value( cxs[  7 ] ).toInt();

   lastrpm         = qRound( isyrec.speed );
DbgLv(1) << "XpDa:ussx: update_isysrec: lastrpm etimoff " << lastrpm << etimoff;
   
//    if ( lastrpm > 0  &&  fnzstx == 0 )
//    {  // When speed is non-zero and no offset set, get offset
//       scan_xpndata( runId, QChar( 'S' ) );
// DbgLv(1) << "XpDa:ussx: after scan_xpndata(S):  etimoff " << etimoff
//  << "fnzstx" << fnzstx;
//    }

DbgLv(1) << "XpDa:ussx:  lastrpm" << lastrpm << "time temp rpm"
 << isyrec.exptime << isyrec.tempera << isyrec.speed;

 qDebug() << "XpDa:ussx:  lastrpm" << lastrpm << "time temp rpm"
 << isyrec.exptime << isyrec.tempera << isyrec.speed;
   return lastrpm;
}

// Get RunID from ExpID
int US_XpnData::get_runid( QString expid)
{
  //if ( !dbxpn.isOpen() )
  //   return 0;

   QVector< int > RunIDs;
   
   QString tabname( "ExperimentRun" );
   QSqlQuery  sqry;
   QString schname( "AUC_schema" );
   QString sqtab   = schname + "." + tabname;
   QString qrytab  = "\"" + schname + "\".\"" + tabname + "\"";
  
   QString qrytext = "SELECT * from " + qrytab
                        + " WHERE \"ExperimentId\"=" + expid + ";";
   sqry            = dbxpn.exec( qrytext );

   while( sqry.next() )       //ALEXEY: does NOT seem to see all records with the same ExpId ???
     {
       qDebug() << "ARRAY of RunIDs: " << sqry.value( 0 ).toInt() ;
       RunIDs.push_back( sqry.value( 0 ).toInt() );
     }

   int runid = 0;
   for (int i=0; i<RunIDs.size(); ++i)
     {
       if (RunIDs[i] > runid)
	 runid = RunIDs[i];
     }
   qDebug() << "RunID: " << runid ;
   return runid;
}

// Scan an XPN database for run information
int US_XpnData::scan_runs( QStringList& runInfo )
{
   int nruns     = 0;

   if ( !dbxpn.isOpen() )
      return nruns;

   QString tabname( "ExperimentRun" );
   QSqlQuery  sqry;
   QString schname( "AUC_schema" );
   QString sqtab   = schname + "." + tabname;
   QString qrytab  = "\"" + schname + "\".\"" + tabname + "\"";
   QStringList cnames;
   QList< int > cxs;

   int cols        = column_indexes( tabname, cnames, cxs );

   sqry            = dbxpn.exec( "SELECT count(*) from " + qrytab + ";" );
   sqry.next();
   int rows        = sqry.value( 0 ).toInt();
DbgLv(1) << "XpDa:scn: ExpRun rows" << rows;

   if ( rows < 1 )
   {
      return rows;
   }

   QString stat_text = tr( "Scanning %1 rows from %2 table..." )
                       .arg( rows ).arg( tabname );
   emit status_text( stat_text );

   QString qtxt  = QString( "SELECT * from " ) + qrytab;        
   const QString delim( "^" );
   runInfo.clear();
   runInfo.reserve( rows );

   sqry          = dbxpn.exec( qtxt );
   int row       = 0;
DbgLv(1) << "XpDa:scn: tabname" << tabname << "rows" << rows
 << "cols" << cols << "qtxt" << qtxt;
DbgLv(1) << "XpDa:inforow: run exp 0 estart ename rname A F I W stat";

   while( sqry.next() )
   {  // Loop to pick up values from rows of the table
      row++;

      tbExpRun exprow;
      exprow.runId     = sqry.value( cxs[ 0] ).toInt();
      exprow.expId     = sqry.value( cxs[ 1] ).toInt();
      exprow.rotorSN   = sqry.value( cxs[ 2] ).toInt();
      exprow.datapath  = sqry.value( cxs[ 3] ).toString();
      exprow.expstart  = sqry.value( cxs[ 4] ).toDateTime();
      exprow.instrSN   = sqry.value( cxs[ 5] ).toString();
      exprow.scimo1sn  = sqry.value( cxs[ 6] ).toString();
      exprow.scimo2sn  = sqry.value( cxs[ 7] ).toString();
      exprow.scimo3sn  = sqry.value( cxs[ 8] ).toString();
      exprow.runstat   = sqry.value( cxs[ 9] ).toInt();
      exprow.expdef    = sqry.value( cxs[10] ).toString();
      exprow.expname   = sqry.value( cxs[11] ).toString();
      exprow.resname   = sqry.value( cxs[12] ).toString();
      exprow.abscnf    = sqry.value( cxs[13] ).toBool();
      exprow.flscnf    = sqry.value( cxs[14] ).toBool();
      exprow.inscnf    = sqry.value( cxs[15] ).toBool();
      exprow.wlscnf    = sqry.value( cxs[16] ).toBool();

      // Skip any runs with no associated scan table entries
      if ( ! ( exprow.abscnf || exprow.flscnf ||
               exprow.inscnf || exprow.wlscnf ) )
      {
DbgLv(1) << "XpDa: NO AFIW:  run exp" << exprow.runId << exprow.expId
 << "expname resname" << exprow.expname << exprow.resname;
         continue;
      }

      // Save experiment row
      tExprun << exprow;

      QString inforow = delim + QString::number( exprow.runId )
                      + delim + QString::number( exprow.expId )
                      + delim + "0"
                      + delim + exprow.expstart.toString( Qt::ISODate )
                                               .replace( "T", " " )
                      + delim + exprow.expname
                      + delim + exprow.resname
                      + delim + QString( exprow.abscnf ? "1" : "0" )
                      + delim + QString( exprow.flscnf ? "1" : "0" )
                      + delim + QString( exprow.inscnf ? "1" : "0" )
                      + delim + QString( exprow.wlscnf ? "1" : "0" )
                      + delim + QString::number( exprow.runstat );
      runInfo << inforow;
DbgLv(1) << "XpDa: inforow" << inforow;
   }
DbgLv(1) << "XpDa:inforow: run exp 0 estart ename rname A F I W stat";

   nruns         = runInfo.size();
   return nruns;
}


// Scan an XPN database for run information
int US_XpnData::scan_runs_auto( QStringList& runInfo, QString & RunID_to_retrieve )
{
   int nruns     = 0;
   QString RunID = RunID_to_retrieve;

   if ( !dbxpn.isOpen() )
      return nruns;

   QString tabname( "ExperimentRun" );
   QSqlQuery  sqry;
   QString schname( "AUC_schema" );
   QString sqtab   = schname + "." + tabname;
   QString qrytab  = "\"" + schname + "\".\"" + tabname + "\"";
   QStringList cnames;
   QList< int > cxs;

   int cols        = column_indexes( tabname, cnames, cxs );

   sqry            = dbxpn.exec( "SELECT count(*) from " + qrytab + ";" );
   sqry.next();
   int rows        = sqry.value( 0 ).toInt();
DbgLv(1) << "XpDa:scn: ExpRun rows" << rows;

   if ( rows < 1 )
   {
      return rows;
   }

   QString stat_text = tr( "Scanning %1 rows from %2 table..." )
                       .arg( rows ).arg( tabname );
   emit status_text( stat_text );

   QString qtxt  = QString( "SELECT * from " ) + qrytab + QString( " WHERE \"RunId\" = %1" ).arg(RunID);        //ALEXEY add WHERE ExperimentID=xxx for us_comproject
   const QString delim( "^" );
   runInfo.clear();
   runInfo.reserve( rows );

   sqry          = dbxpn.exec( qtxt );
   int row       = 0;
DbgLv(1) << "XpDa:scn: tabname" << tabname << "rows" << rows
 << "cols" << cols << "qtxt" << qtxt;
DbgLv(1) << "XpDa:inforow: run exp 0 estart ename rname A F I W stat";

   while( sqry.next() )
   {  // Loop to pick up values from rows of the table
      row++;

      tbExpRun exprow;
      exprow.runId     = sqry.value( cxs[ 0] ).toInt();
      exprow.expId     = sqry.value( cxs[ 1] ).toInt();
      exprow.rotorSN   = sqry.value( cxs[ 2] ).toInt();
      exprow.datapath  = sqry.value( cxs[ 3] ).toString();
      exprow.expstart  = sqry.value( cxs[ 4] ).toDateTime();
      exprow.instrSN   = sqry.value( cxs[ 5] ).toString();
      exprow.scimo1sn  = sqry.value( cxs[ 6] ).toString();
      exprow.scimo2sn  = sqry.value( cxs[ 7] ).toString();
      exprow.scimo3sn  = sqry.value( cxs[ 8] ).toString();
      exprow.runstat   = sqry.value( cxs[ 9] ).toInt();
      exprow.expdef    = sqry.value( cxs[10] ).toString();
      exprow.expname   = sqry.value( cxs[11] ).toString();
      exprow.resname   = sqry.value( cxs[12] ).toString();
      exprow.abscnf    = sqry.value( cxs[13] ).toBool();
      exprow.flscnf    = sqry.value( cxs[14] ).toBool();
      exprow.inscnf    = sqry.value( cxs[15] ).toBool();
      exprow.wlscnf    = sqry.value( cxs[16] ).toBool();

      // Skip any runs with no associated scan table entries
      if ( ! ( exprow.abscnf || exprow.flscnf ||
               exprow.inscnf || exprow.wlscnf ) )
         continue;

      // Save experiment row
      tExprun << exprow;

      QString inforow = delim + QString::number( exprow.runId )
                      + delim + QString::number( exprow.expId )
                      + delim + "0"
                      + delim + exprow.expstart.toString( Qt::ISODate )
                                               .replace( "T", " " )
                      + delim + exprow.expname
                      + delim + exprow.resname
                      + delim + QString( exprow.abscnf ? "1" : "0" )
                      + delim + QString( exprow.flscnf ? "1" : "0" )
                      + delim + QString( exprow.inscnf ? "1" : "0" )
                      + delim + QString( exprow.wlscnf ? "1" : "0" )
                      + delim + QString::number( exprow.runstat );
      runInfo << inforow;
DbgLv(1) << "XpDa: inforow" << inforow;
   }
DbgLv(1) << "XpDa:inforow: run exp 0 estart ename rname A F I W stat";

   nruns         = runInfo.size();
   return nruns;
}


// Filter runs list to eliminate zero-data entries
int US_XpnData::filter_runs( QStringList& runInfo )
{
   int nruns     = 0;
   int kruns     = runInfo.size();
   const QString delim( "^" );
   QSqlQuery  sqry;
   QStringList runIsav = runInfo;
   QVector< tbExpRun > tExprsv = tExprun;
   QString schname( "AUC_schema" );

   runInfo.clear();
   tExprun.clear();

   emit status_text( tr( "Counting ScanData table rows..." ) );

   for ( int ii = 0; ii < kruns; ii++ )
   {
      tbExpRun exprow = tExprsv[ ii ];
      QString sRunId  = QString::number( exprow.runId );
      int kdat        = 0;

      if ( exprow.abscnf )
      {  // Count AbsorbanceScanData
         QString tabname( "AbsorbanceScanData" );
         QString qrytab  = "\"" + schname + "\".\"" + tabname + "\"";
         QString qrytext = "SELECT count(*) from " + qrytab
                         + " WHERE \"RunId\"=" + sRunId + ";";
         sqry            = dbxpn.exec( qrytext );
         sqry.next();
         kdat           += sqry.value( 0 ).toInt();
      }

      if ( exprow.flscnf )
      {  // Count FluorescenceScanData
         QString tabname( "FluorescenceScanData" );
         QString qrytab  = "\"" + schname + "\".\"" + tabname + "\"";
         QString qrytext = "SELECT count(*) from " + qrytab
                         + " WHERE \"RunId\"=" + sRunId + ";";
         sqry            = dbxpn.exec( qrytext );
         sqry.next();
         kdat           += sqry.value( 0 ).toInt();
      }

      if ( exprow.inscnf )
      {  // Count InterferenceScanData
         QString tabname( "InterferenceScanData" );
         QString qrytab  = "\"" + schname + "\".\"" + tabname + "\"";
         QString qrytext = "SELECT count(*) from " + qrytab
                         + " WHERE \"RunId\"=" + sRunId + ";";
         sqry            = dbxpn.exec( qrytext );
         sqry.next();
         kdat           += sqry.value( 0 ).toInt();
      }

      if ( exprow.wlscnf )
      {  // Count WavelengthScanData
         QString tabname( "WavelengthScanData" );
         QString qrytab  = "\"" + schname + "\".\"" + tabname + "\"";
         QString qrytext = "SELECT count(*) from " + qrytab
                         + " WHERE \"RunId\"=" + sRunId + ";";
         sqry            = dbxpn.exec( qrytext );
         sqry.next();
         kdat           += sqry.value( 0 ).toInt();
      }

      if ( kdat > 0 )
      {  // Recreate entry where data is non-zero
         QString inforow = delim + QString::number( exprow.runId )
                         + delim + QString::number( exprow.expId )
                         + delim + QString::number( kdat )
                         + delim + exprow.expstart.toString( Qt::ISODate )
                                                  .replace( "T", " " )
                         + delim + exprow.expname
                         + delim + exprow.resname
                         + delim + QString( exprow.abscnf ? "1" : "0" )
                         + delim + QString( exprow.flscnf ? "1" : "0" )
                         + delim + QString( exprow.inscnf ? "1" : "0" )
                         + delim + QString( exprow.wlscnf ? "1" : "0" );
         runInfo << inforow;
         tExprun << exprow;
         nruns++;
DbgLv(1) << "XpDa:f_r:  ii nruns" << ii << nruns << "INCLUDED:" << inforow;
      }
DbgLv(1) << "XpDa:f_r: ii" << ii << "EXCLUDED:" << runIsav[ii];
   }

   emit status_text( tr( "Run table reduced to %1 runs..." ).arg( nruns ) );

   return nruns;
}

// Import XPN data from a selected database server
bool US_XpnData::import_data( const int runId, const int scanMask )
{
   bool status   = true;
   ntscan        = 0;
   ntsrow        = 0;

   if ( ! dbxpn.open() )
   {
      return false;
   }

   tAsdata.clear();     // Clear table value vectors
   tFsdata.clear();
   tIsdata.clear();
   tWsdata.clear();
   tSydata.clear();
   tCrprof.clear();
   bool ascnf    = scanMask & 1;
   bool fscnf    = scanMask & 2;
   bool iscnf    = scanMask & 4;
   bool wscnf    = scanMask & 8;

   arows         = 0;
   frows         = 0;
   irows         = 0;
   wrows         = 0;

   // Scan and build data for System Status Data
   int srows     = scan_xpndata( runId, 'S' );

   if ( ascnf )
   {  // Scan and build data for Absorbance Scan Data
      arows      = scan_xpndata( runId, 'A' );
   }

   if ( fscnf )
   {  // Scan and build data for Fluorescence Scan Data
      frows      = scan_xpndata( runId, 'F' );
   }

   if ( iscnf )
   {  // Scan and build data for Interference Scan Data
      irows      = scan_xpndata( runId, 'I' );
   }

   if ( wscnf )
   {  // Scan and build data for Wavelength Scan Data
      wrows      = scan_xpndata( runId, 'W' );
   }

   // Scan and build data for Centrifuge Run Profile
   int crows     = scan_xpndata( runId, 'C' );
DbgLv(1) << "XpDa:i_d: arows frows irows wrows srows crows"
   << arows << frows << irows << wrows << srows << crows;

   ntsrow        = arows + frows + irows + wrows;

   return status;
}

// Import XPN data from a selected database server [for autoflow]
bool US_XpnData::import_data_auto( const int runId, const int scanMask, bool& o_conn )
{
   bool status   = true;
   ntscan        = 0;
   ntsrow        = 0;

   qDebug() << "in [import_data_auto]: Init o_Conn status: " << o_conn;
   
   if ( ! dbxpn.open() )
   {
     o_conn = false;
     qDebug() << "XPN: import_data_auto: !dbxpn.open() !!! runId, scanMask, o_conn -- "
	      << runId << scanMask << o_conn;
     return false;
   }

   tAsdata.clear();     // Clear table value vectors
   tFsdata.clear();
   tIsdata.clear();
   tWsdata.clear();
   tSydata.clear();
   tCrprof.clear();
   bool ascnf    = scanMask & 1;
   bool fscnf    = scanMask & 2;
   bool iscnf    = scanMask & 4;
   bool wscnf    = scanMask & 8;

   arows         = 0;
   frows         = 0;
   irows         = 0;
   wrows         = 0;

   //booleans for diff. types
   bool srows_bool = true;
   bool arows_bool = true;
   bool frows_bool = true;
   bool irows_bool = true;
   bool wrows_bool = true;
   bool crows_bool = true;
   
   // Scan and build data for System Status Data
   QElapsedTimer timer_srows;
   timer_srows.start();
   int srows     = scan_xpndata_auto( runId, 'S', srows_bool, timer_srows );
   qDebug() << "[TIME] of scan_xpndata_auto( runId, 'S', srows_bool ) " << int( timer_srows.elapsed() / 1000 ) << " sec"; 
   
   if ( ascnf )
   {  // Scan and build data for Absorbance Scan Data
     QElapsedTimer timer_arows;
     timer_arows.start();
     arows      = scan_xpndata_auto( runId, 'A', arows_bool, timer_arows );
     qDebug() << "[TIME] of scan_xpndata_auto( runId, 'A', arows_bool ) " << int( timer_arows.elapsed() / 1000 ) << " sec"; 
   }

   if ( fscnf )
   {  // Scan and build data for Fluorescence Scan Data
     QElapsedTimer timer_frows;
     timer_frows.start();
     frows      = scan_xpndata_auto( runId, 'F', frows_bool, timer_frows );
     qDebug() << "[TIME] of scan_xpndata_auto( runId, 'F', frows_bool ) " << int( timer_frows.elapsed() / 1000 ) << " sec"; 
   }

   if ( iscnf )
   {  // Scan and build data for Interference Scan Data
     QElapsedTimer timer_irows;
     timer_irows.start();
     irows      = scan_xpndata_auto( runId, 'I', irows_bool, timer_irows );
     qDebug() << "[TIME] of scan_xpndata_auto( runId, 'I', irows_bool ) " << int( timer_irows.elapsed() / 1000 ) << " sec"; 
   }

   if ( wscnf )
   {  // Scan and build data for Wavelength Scan Data
     QElapsedTimer timer_wrows;
     timer_wrows.start();
     wrows      = scan_xpndata_auto( runId, 'W', wrows_bool, timer_wrows );
     qDebug() << "[TIME] of scan_xpndata_auto( runId, 'W', wrows_bool ) " << int( timer_wrows.elapsed() / 1000 ) << " sec"; 
   }

   // Scan and build data for Centrifuge Run Profile
   QElapsedTimer timer_crows;
   timer_crows.start();
   int crows     = scan_xpndata_auto( runId, 'C', crows_bool, timer_crows );
   qDebug() << "[TIME] of scan_xpndata_auto( runId, 'C', crows_bool ) " << int( timer_crows.elapsed() / 1000 ) << " sec"; 
   
   
   qDebug() << "XpDa:i_d: arows frows irows wrows srows crows"
	    << arows << frows << irows << wrows << srows << crows;

   qDebug() << "XpDa:i_d: [BOOLS] arows_bool frows_bool irows_bool wrows_bool srows_bool crows_bool"
	    << arows_bool << frows_bool << irows_bool << wrows_bool << srows_bool << crows_bool;

   ntsrow        = arows + frows + irows + wrows;
   qDebug() << "in [import_data_auto]:ntsrow = " << ntsrow;

   if ( !arows_bool || !frows_bool || !irows_bool || !wrows_bool || !srows_bool || !crows_bool)
     {
       o_conn = false;
       qDebug() << "No connection in one of scan_xpndata_auto()...";
       return false;
     }

   return status;
}


// Re-import XPN data from a selected database server
bool US_XpnData::reimport_data( const int runId, const int scanMask )
{
   bool status   = false;

   if ( ! dbxpn.open() )
   {
     qDebug() << "XPN: reimport_data: !dbxpn.open() !!! runId, scanMask -- "
	      << runId << scanMask;
     return false;
   }

   int oarows    = tAsdata.count();     // Get old row counts
   int ofrows    = tFsdata.count();
   int oirows    = tIsdata.count();
   int owrows    = tWsdata.count();
   if ( oarows > 1  &&  tAsdata[ 0 ].radPath != tAsdata[ 1 ].radPath )
      oarows       /= 2;
   if ( ofrows > 0  &&  tFsdata[ 0 ].radPath != tFsdata[ 1 ].radPath )
      ofrows       /= 2;
   if ( owrows > 0  &&  tWsdata[ 0 ].radPath != tWsdata[ 1 ].radPath )
      owrows       /= 2;
   bool ascnf    = scanMask & 1;
   bool fscnf    = scanMask & 2;
   bool iscnf    = scanMask & 4;
   bool wscnf    = scanMask & 8;

   int arows     = 0;
   int frows     = 0;
   int irows     = 0;
   int wrows     = 0;

   if ( ascnf )
   {  // Scan and update data for Absorbance Scan Data
      arows      = update_xpndata( runId, 'A' );
      status     = ( status  ||  arows > oarows );
DbgLv(1) << "XpDa: rei_dat: arows oarows status" << arows << oarows << status;
 qDebug() << "XpDa: rei_dat: arows oarows status" << arows << oarows << status; 
   }

   if ( fscnf )
   {  // Scan and update data for Fluorescence Scan Data
      frows      = update_xpndata( runId, 'F' );
      status     = ( status  ||  frows > ofrows );
   }

   if ( iscnf )
   {  // Scan and update data for Interference Scan Data
      irows      = update_xpndata( runId, 'I' );
      status     = ( status  ||  irows > oirows );
   }

   if ( wscnf )
   {  // Scan and update data for Wavelength Scan Data
      wrows      = update_xpndata( runId, 'W' );
      status     = ( status  ||  wrows > owrows );
   }

DbgLv(1) << "XpDa: rei_dat: arows frows irows wrows"
   << arows << frows << irows << wrows << "status" << status;
 qDebug() << "XpDa: rei_dat: arows frows irows wrows"
   << arows << frows << irows << wrows << "status" << status;
//*DEBUG*
//status=true;
//*DEBUG*

   return status;
}


// Re-import XPN data from a selected database server [for autoflow]
bool US_XpnData::reimport_data_auto( const int runId, const int scanMask, bool& o_conn )
{
   bool status   = false;

   qDebug() << "in [reimport_data_auto]: Init o_Conn status: " << o_conn;
   
   if ( ! dbxpn.open() )
   {
     o_conn = false;
     qDebug() << "XPN: reimport_data_auto: !dbxpn.open() !!! runId, scanMask, o_conn -- "
	      << runId << scanMask << o_conn;
     
     return false;
   }

   int oarows    = tAsdata.count();     // Get old row counts
   int ofrows    = tFsdata.count();
   int oirows    = tIsdata.count();
   int owrows    = tWsdata.count();
   if ( oarows > 1  &&  tAsdata[ 0 ].radPath != tAsdata[ 1 ].radPath )
      oarows       /= 2;
   if ( ofrows > 0  &&  tFsdata[ 0 ].radPath != tFsdata[ 1 ].radPath )
      ofrows       /= 2;
   if ( owrows > 0  &&  tWsdata[ 0 ].radPath != tWsdata[ 1 ].radPath )
      owrows       /= 2;
   bool ascnf    = scanMask & 1;
   bool fscnf    = scanMask & 2;
   bool iscnf    = scanMask & 4;
   bool wscnf    = scanMask & 8;

   int arows     = 0;
   int frows     = 0;
   int irows     = 0;
   int wrows     = 0;

   if ( ascnf )
   {  // Scan and update data for Absorbance Scan Data
      arows      = update_xpndata( runId, 'A' );
      status     = ( status  ||  arows > oarows );
DbgLv(1) << "XpDa: rei_dat: arows oarows status" << arows << oarows << status;
 qDebug() << "XpDa: rei_dat: arows oarows status" << arows << oarows << status; 
   }

   if ( fscnf )
   {  // Scan and update data for Fluorescence Scan Data
      frows      = update_xpndata( runId, 'F' );
      status     = ( status  ||  frows > ofrows );
   }

   if ( iscnf )
   {  // Scan and update data for Interference Scan Data
      irows      = update_xpndata( runId, 'I' );
      status     = ( status  ||  irows > oirows );
   }

   if ( wscnf )
   {  // Scan and update data for Wavelength Scan Data
      wrows      = update_xpndata( runId, 'W' );
      status     = ( status  ||  wrows > owrows );
   }

DbgLv(1) << "XpDa: rei_dat: arows frows irows wrows"
   << arows << frows << irows << wrows << "status" << status;
 qDebug() << "XpDa: rei_dat: arows frows irows wrows"
   << arows << frows << irows << wrows << "status" << status;
//*DEBUG*
//status=true;
//*DEBUG*

   return status;
}


// Query and save data for a [AIFW]ScanData or [SC] table
int US_XpnData::scan_xpndata( const int runId, const QChar scantype )
{
   QSqlQuery  sqry;
   QSqlRecord qrec;
   QString schname( "AUC_schema" );
   QString tabname( "AbsorbanceScanData" );
   tabname         = ( scantype == 'F' ) ? "FluorescenceScanData" : tabname;
   tabname         = ( scantype == 'I' ) ? "InterferenceScanData" : tabname;
   tabname         = ( scantype == 'W' ) ? "WavelengthScanData"   : tabname;
   tabname         = ( scantype == 'S' ) ? "SystemStatusData"     : tabname;
   tabname         = ( scantype == 'C' ) ? "CentrifugeRunProfile" : tabname;
   QString sqtab   = schname + "." + tabname;
   QString qrytab  = "\"" + schname + "\".\"" + tabname + "\"";
   QString sRunId  = QString::number( runId );
   QStringList cnames;
   QList< int > cxs;

   int count       = 0;
   int rows        = 0;

   // Get count of rows matching runId
   QString qrytext = "SELECT count(*) from " + qrytab
                   + " WHERE \"RunId\"=" + sRunId + ";";
   if ( scantype == 'C' )
      qrytext         = "SELECT count(*) from " + qrytab + ";";

   sqry            = dbxpn.exec( qrytext );
   sqry.next();
   count           = sqry.value( 0 ).toInt();
DbgLv(1) << "XpDa:s_x: sRunId" << sRunId << "count" << count;
 
   if ( count < 1 )
   {
     return count;
   }

   emit status_text( tr( " Scanning %1 rows of %2 table..." )
                     .arg( count ).arg( tabname ) );

   // Get columns and determine indecies of fields
   qrytext         = "SELECT * from " + qrytab
                     + " WHERE \"RunId\"=" + sRunId
                     + " ORDER BY \"DataId\" ;";
   if ( scantype == 'C' )
      qrytext         = "SELECT * from " + qrytab + ";";

   int cols        = column_indexes( tabname, cnames, cxs );
DbgLv(1) << "XpDa:s_x:  cols" << cols << "cnames" << cnames[0] << "..."
 << cnames[cols-1] << "tabname" << tabname;

   sqry            = dbxpn.exec( qrytext );

   int isctyp    = ( scantype == 'A' ) ? 1 : 0;
   isctyp        = ( scantype == 'F' ) ? 2 : isctyp;
   isctyp        = ( scantype == 'I' ) ? 3 : isctyp;
   isctyp        = ( scantype == 'W' ) ? 4 : isctyp;
   isctyp        = ( scantype == 'S' ) ? 5 : isctyp;
   isctyp        = ( scantype == 'C' ) ? 6 : isctyp;
DbgLv(1) << "XpDa:s_x:  isctyp scantype" << isctyp << scantype
 << "qrytext" << qrytext;

   // Loop to read data and store in internal array

   while ( sqry.next() )
   {
      rows++;
      emit status_text( tr( "Of %1 ScanData(%2) rows, queried row %3" )
                        .arg( count ).arg( scantype ).arg( rows ) );

      switch ( isctyp )
      {
         case 1:
         {
            update_ATable( sqry, cxs );
//*DEBUG*
if(rows<21 || (rows+21)>count) {
int ldx=tAsdata.count()-1;
tbAsData asdrow = tAsdata[ldx];
DbgLv(1) << "XpDa:scn:    row" << rows << "run" << asdrow.runId
 << "dat" << asdrow.dataId << "count" << asdrow.count;
DbgLv(1) << "XpDa:scn:     rads0 rads1 vals0 vals1"
 << asdrow.rads[0] << asdrow.rads[1] << asdrow.vals[0] << asdrow.vals[1];
DbgLv(1) << "XpDa:scn:      etim scn temp speed omg estr"
 << asdrow.exptime << asdrow.scanSeqN << asdrow.tempera << asdrow.speed
 << asdrow.omgSqT << asdrow.expstart.toString();
}
//*DEBUG*

            break;
         }
         case 2:
         {
            update_FTable( sqry, cxs );
//*DEBUG*
if(rows<9 || (rows+9)>count) {
int ldx=tFsdata.count()-1;
tbFsData fsdrow = tFsdata[ldx];
DbgLv(1) << "XpDa:scn:    row" << rows << "run" << fsdrow.runId
 << "dat" << fsdrow.dataId;
DbgLv(1) << "XpDa:scn:       cnames" << cnames;
}
//*DEBUG*
            break;
         }
         case 3:
         {
            update_ITable( sqry, cxs );
//*DEBUG*
if(rows<9 || (rows+9)>count) {
int ldx=tIsdata.count()-1;
tbIsData isdrow = tIsdata[ldx];
DbgLv(1) << "XpDa:scn:    row" << rows << "run" << isdrow.runId
 << "dat" << isdrow.dataId;
DbgLv(1) << "XpDa:scn:     rads0 rads1 vals0 vals1"
 << isdrow.rads[0] << isdrow.rads[1] << isdrow.vals[0] << isdrow.vals[1];
}
//*DEBUG*
            break;
         }
         case 4:
         {
            update_WTable( sqry, cxs );
//*DEBUG*
if(rows<9 || (rows+9)>count) {
int ldx=tWsdata.count()-1;
tbWsData wsdrow = tWsdata[ldx];
DbgLv(1) << "XpDa:scn:    row" << rows << "run dat count" << wsdrow.runId
 << wsdrow.dataId << wsdrow.count;
}
//*DEBUG*
            break;
         }
         case 5:
         {
            tbSyData sydrow;

            sydrow.dataId    = sqry.value( cxs[  0 ] ).toInt();
            sydrow.runId     = sqry.value( cxs[  1 ] ).toInt();
            sydrow.expstart  = sqry.value( cxs[  2 ] ).toDateTime();
            sydrow.exptime   = sqry.value( cxs[  3 ] ).toInt();
            sydrow.tempera   = sqry.value( cxs[  4 ] ).toDouble();
            sydrow.speed     = sqry.value( cxs[  5 ] ).toDouble();
            sydrow.omgSqT    = sqry.value( cxs[  6 ] ).toDouble();
            sydrow.stageNum  = sqry.value( cxs[  7 ] ).toInt();

            tSydata << sydrow;
//*DEBUG*
if(rows<21 || (rows+21)>count) {
DbgLv(1) << "XpDa:scn:    row" << rows << "run dat" << sydrow.runId << sydrow.dataId
 << "time" << sydrow.exptime << "temp" << sydrow.tempera << "speed" << sydrow.speed
 << "omg2t" << sydrow.omgSqT;
}
//*DEBUG*
            break;
         }
         case 6:
         {
            tbCrProf crprow;

            crprow.frunId    = sqry.value( cxs[  0 ] ).toInt();
            crprow.tempera   = sqry.value( cxs[  1 ] ).toDouble();
            crprow.sbrake    = sqry.value( cxs[  2 ] ).toInt();
            crprow.brrate    = sqry.value( cxs[  3 ] ).toDouble();
            crprow.holdte    = sqry.value( cxs[  4 ] ).toInt();
            crprow.holdsp    = sqry.value( cxs[  5 ] ).toInt();
            crprow.sette     = sqry.value( cxs[  6 ] ).toInt();
            crprow.sstintv   = sqry.value( cxs[  7 ] ).toDouble();
            crprow.stages    = sqry.value( cxs[  8 ] ).toString();
            crprow.stgrpm    = sqry.value( cxs[  9 ] ).toDouble();
            crprow.stgstart  = sqry.value( cxs[ 10 ] ).toDouble();
            crprow.stgdurat  = sqry.value( cxs[ 11 ] ).toDouble();
            crprow.stgaccel  = sqry.value( cxs[ 12 ] ).toDouble();
            crprow.paramids  = sqry.value( cxs[ 13 ] ).toString();

            tCrprof << crprow;
//*DEBUG*
if(rows<21 || (rows+21)>count) {
DbgLv(1) << "XpDa:scn:    row" << rows << "fugeRunId" << crprow.frunId
 << "ssInterval" << crprow.sstintv;
}
//*DEBUG*
            break;
         }
      }
   }

   if ( scantype == 'C' )
   {  // Show mapping of RunId to ExperimentId to FugeRunProfileId
      QList< int >  rExpIds;  // ExperimentRun experiment Ids
      QList< int >  rRunIds;  // ExperimentRun run Ids
      QList< int >  dExpIds;  // ExperimentDefinition experiment Ids
      QList< int >  dFugIds;  // ExperimentDefinition fugeProfile Ids
DbgLv(1) << "XpDa:scn:    tCrprof count" << tCrprof.count();

      int iRunId      = sRunId.toInt();
      tabname         = "ExperimentRun";
      sqtab           = schname + "." + tabname;
      qrytab          = "\"" + schname + "\".\"" + tabname + "\"";
      qrytext         = "SELECT * from " + qrytab + ";"; 
      sqry            = dbxpn.exec( qrytext );
      qrec            = dbxpn.record( qrytab );
      // Get indexes to ExperimentId and RunId
      cols            = column_indexes( tabname, cnames, cxs );
      int jexpid      = cnames.indexOf( "ExperimentId" );
      int jrunid      = cnames.indexOf( "RunId" );
      while ( sqry.next() )
      {  // Build a list of ExperimentRun expIds and runIds
         int rExpId      = sqry.value( jexpid ).toInt();
         int rRunId      = sqry.value( jrunid ).toInt();
         rExpIds << rExpId;
         rRunIds << rRunId;
      }

      tabname         = "ExperimentDefinition";
      sqtab           = schname + "." + tabname;
      qrytab          = "\"" + schname + "\".\"" + tabname + "\"";
      qrytext         = "SELECT * from " + qrytab + ";"; 
      sqry            = dbxpn.exec( qrytext );
      qrec            = dbxpn.record( qrytab );
      cols            = column_indexes( tabname, cnames, cxs );
      // Get indexes to ExperimentId and FugeRunProfileId
      jexpid          = cnames.indexOf( "ExperimentId" );
      int jfruid      = cnames.indexOf( "FugeRunProfileId" );
      int jexpnm      = cnames.indexOf( "Name" );
//DbgLv(1) << "XpDa:scn:    jexpid jfruid" << jexpid << jfruid;
      while ( sqry.next() )
      {  // Build a list of ExperimentDefinition expIds and fugeIds
         int dExpId      = sqry.value( jexpid ).toInt();
         int dFugId      = sqry.value( jfruid ).toInt();
         dExpIds << dExpId;
         dFugIds << dFugId;
DbgLv(1) << "XpDa:scn:  dExpId" << dExpId << "dFugId" << dFugId
 << "Name" << sqry.value(jexpnm).toString();
      }
//DbgLv(1) << "XpDa:scn: dExpIds" << dExpIds;
//DbgLv(1) << "XpDa:scn: dFugIds" << dFugIds;
      // Map experimentId to runId, then fugeId to experimentId
      int rrndx       = rRunIds.indexOf( iRunId );
DbgLv(1) << "XpDa:scn:  iRunId" << iRunId << "rrndx" << rrndx;
      int iExpId      = rExpIds[ rrndx ];
      int dendx       = dExpIds.indexOf( iExpId );
DbgLv(1) << "XpDa:scn:  iExpId" << iExpId << "dendx" << dendx;
      int iFugId      = ( dendx >= 0 ) ? dFugIds[ dendx ] : iExpId;
      sstintv         = 0.0;
      double sstintl  = 1000.0;
DbgLv(1) << "XpDa:scn:    RunId" << iRunId << "ExperimentId" << iExpId
 << "FugeRunProfileId" << iFugId; 

      for ( int ii = 0; ii < tCrprof.count(); ii++ )
      {  // Look for matching fuge Id in CentrifugeRunProfile values
         sstintl          = tCrprof[ ii ].sstintv;

         if ( tCrprof[ ii ].frunId == iFugId )
         {  // Match found:  save corresponding SystemStatusInterval
            sstintv          = sstintl;
            break;
         }
      }

      sstintv         = ( sstintv > 0.0 ) ? sstintv : sstintl;
DbgLv(1) << "XpDa:scn:    FugId" << iFugId << "sstInterval" << sstintv;
   }

   if ( scantype == 'S' )
   {  // Determine experiment time offset (time at last rms=0 point)
      int ntssda        = tSydata.count();
      fnzstx            = 0;

      for ( int ii = 0; ii < ntssda; ii++ )
      {  // Find the index to the first non-zero speed
         int irSpeed       = (int)qRound( tSydata[ ii ].speed );
         if ( irSpeed > 0 )
         {
            fnzstx            = ii;
            break;
         }
      }

      // First non-zero-speed time index
      fnzstx            = ( fnzstx > 0 ) ? fnzstx : 1;                        //ALEXEY: if tSydata.count() == 1 (so only tSydata[0] exists), AND fnzstx was 0, THEN
                                                                              //        fnzstx = ( fnzstx > 0 ) ? fnzstx : 1; will lead to fnzstx = 1 !!!
                                                                              //        THEN   int etime1        = tSydata[ 1 ].exptime; WILL CRASH  
      int t2tx          = fnzstx + 1;
      // Experiment time offset (== negative of first time + interval)
      //int etime1        = tSydata[ fnzstx ].exptime;

      //ALEXEY: check additionally if tSydata[fnzstx] exists!!! 
      int etime1 = 0;
      if ( fnzstx < tSydata.count() )
      {
	etime1          = tSydata[ fnzstx ].exptime;
      }
      ////////////////
      
      int timeintv      = sstintv;
      if ( t2tx < tSydata.count() )
      {
         timeintv          = tSydata[ t2tx ].exptime - etime1;
      }
      timeintv          = qMax( timeintv, 1 );
      etimoff           = timeintv - etime1;
//*DEBUG*
DbgLv(1) << "XpDa:scn: fnzstx" << fnzstx << "etimoff" << etimoff
 << "  etime1 timeintv" << etime1 << timeintv << sstintv;
int nn=qMin(tSydata.count(),fnzstx+3);
DbgLv(1) << "XpDa:scn:  nn=" << nn << " Vals nn-5 to nn-1 --";
//if(nn>4) {
// DbgLv(1) << "XpDa:scn: times:"
//  << tSydata[nn-5].exptime
//  << tSydata[nn-4].exptime
//  << tSydata[nn-3].exptime
//  << tSydata[nn-2].exptime
//  << tSydata[nn-1].exptime;
// DbgLv(1) << "XpDa:scn: speeds:"
//  << tSydata[nn-5].speed
//  << tSydata[nn-4].speed
//  << tSydata[nn-3].speed
//  << tSydata[nn-2].speed
//  << tSydata[nn-1].speed; }
//*DEBUG*
   }

   return rows;
}


// Query and save data for a [AIFW]ScanData or [SC] table: [for autoflow framework]
int US_XpnData::scan_xpndata_auto( const int runId, const QChar scantype, bool& o_conn, QElapsedTimer elapse_t )
{
   QSqlQuery  sqry;
   QSqlRecord qrec;
   QString schname( "AUC_schema" );
   QString tabname( "AbsorbanceScanData" );
   tabname         = ( scantype == 'F' ) ? "FluorescenceScanData" : tabname;
   tabname         = ( scantype == 'I' ) ? "InterferenceScanData" : tabname;
   tabname         = ( scantype == 'W' ) ? "WavelengthScanData"   : tabname;
   tabname         = ( scantype == 'S' ) ? "SystemStatusData"     : tabname;
   tabname         = ( scantype == 'C' ) ? "CentrifugeRunProfile" : tabname;
   QString sqtab   = schname + "." + tabname;
   QString qrytab  = "\"" + schname + "\".\"" + tabname + "\"";
   QString sRunId  = QString::number( runId );
   QStringList cnames;
   QList< int > cxs;

   int count       = 0;
   int rows        = 0;

   // Get count of rows matching runId
   QString qrytext = "SELECT count(*) from " + qrytab
                   + " WHERE \"RunId\"=" + sRunId + ";";
   if ( scantype == 'C' )
      qrytext         = "SELECT count(*) from " + qrytab + ";";

   qDebug() << "in [scan_xpndata_auto()]: scantype, qrytext -- "
	    << scantype << qrytext;
   
   qDebug() << "[TIME 1] of scan_xpndata_auto(): " << int( elapse_t.elapsed() / 1000 ) << " sec";
   
   sqry            = dbxpn.exec( qrytext );
   qDebug() << "sqry.isActive() ? " << sqry.isActive();

   qDebug() << "[TIME 2] of scan_xpndata_auto(): " << int( elapse_t.elapsed() / 1000 ) << " sec";
   
   bool scan_xpndata_query_success = sqry.next();

   qDebug() << "[TIME 3] of scan_xpndata_auto(): " << int( elapse_t.elapsed() / 1000 ) << " sec";
   
   count           = sqry.value( 0 ).toInt();
DbgLv(1) << "XpDa:s_x: sRunId" << sRunId << "count" << count;
   qDebug() << "XpDa:s_x: sRunId" << sRunId << "scan_xpndata_query_success, count: " << scan_xpndata_query_success << count;
   
   qDebug() << "[TIME 4] of scan_xpndata_auto(): " << int( elapse_t.elapsed() / 1000 ) << " sec";
   
   if( !scan_xpndata_query_success || !sqry.isActive() )
     {
       qDebug() << "in [scan_xpndata()], scan_xpndata_query_success, NO CONNECTION? "
		<< scan_xpndata_query_success << !scan_xpndata_query_success;
       qDebug() << "count = 0 ? " << count;
       o_conn = false;

       return count;
     }
   
   if ( count < 1 )
   {
     qDebug() << "in [scan_xpndata()], count < 1...";
     return count;
   }

   emit status_text( tr( " Scanning %1 rows of %2 table..." )
                     .arg( count ).arg( tabname ) );

   qDebug() << "[TIME 5] of scan_xpndata_auto(): " << int( elapse_t.elapsed() / 1000 ) << " sec";

   // Get columns and determine indecies of fields
   qrytext         = "SELECT * from " + qrytab
                     + " WHERE \"RunId\"=" + sRunId
                     + " ORDER BY \"DataId\" ;";
   if ( scantype == 'C' )
      qrytext         = "SELECT * from " + qrytab + ";";

   int cols        = column_indexes( tabname, cnames, cxs );
DbgLv(1) << "XpDa:s_x:  cols" << cols << "cnames" << cnames[0] << "..."
 << cnames[cols-1] << "tabname" << tabname;

 qDebug() << "XpDa:s_x:  cols" << cols << "cnames" << cnames[0] << "..."
 << cnames[cols-1] << "tabname" << tabname;

   sqry            = dbxpn.exec( qrytext );

   qDebug() << "[TIME 6] of scan_xpndata_auto(): " << int( elapse_t.elapsed() / 1000 ) << " sec";

   int isctyp    = ( scantype == 'A' ) ? 1 : 0;
   isctyp        = ( scantype == 'F' ) ? 2 : isctyp;
   isctyp        = ( scantype == 'I' ) ? 3 : isctyp;
   isctyp        = ( scantype == 'W' ) ? 4 : isctyp;
   isctyp        = ( scantype == 'S' ) ? 5 : isctyp;
   isctyp        = ( scantype == 'C' ) ? 6 : isctyp;
DbgLv(1) << "XpDa:s_x:  isctyp scantype" << isctyp << scantype
 << "qrytext" << qrytext;

 qDebug() << "XpDa:s_x:  isctyp scantype" << isctyp << scantype
 << "qrytext" << qrytext;
 
   // Loop to read data and store in internal array

   while ( sqry.next() )
   {
      rows++;
      emit status_text( tr( "Of %1 ScanData(%2) rows, queried row %3" )
                        .arg( count ).arg( scantype ).arg( rows ) );

      switch ( isctyp )
      {
         case 1:
         {
            update_ATable( sqry, cxs );
//*DEBUG*
if(rows<21 || (rows+21)>count) {
int ldx=tAsdata.count()-1;
tbAsData asdrow = tAsdata[ldx];
DbgLv(1) << "XpDa:scn:    row" << rows << "run" << asdrow.runId
 << "dat" << asdrow.dataId << "count" << asdrow.count;
DbgLv(1) << "XpDa:scn:     rads0 rads1 vals0 vals1"
 << asdrow.rads[0] << asdrow.rads[1] << asdrow.vals[0] << asdrow.vals[1];
DbgLv(1) << "XpDa:scn:      etim scn temp speed omg estr"
 << asdrow.exptime << asdrow.scanSeqN << asdrow.tempera << asdrow.speed
 << asdrow.omgSqT << asdrow.expstart.toString();
}
//*DEBUG*

            break;
         }
         case 2:
         {
            update_FTable( sqry, cxs );
//*DEBUG*
if(rows<9 || (rows+9)>count) {
int ldx=tFsdata.count()-1;
tbFsData fsdrow = tFsdata[ldx];
DbgLv(1) << "XpDa:scn:    row" << rows << "run" << fsdrow.runId
 << "dat" << fsdrow.dataId;
DbgLv(1) << "XpDa:scn:       cnames" << cnames;
}
//*DEBUG*
            break;
         }
         case 3:
         {
            update_ITable( sqry, cxs );
//*DEBUG*
if(rows<9 || (rows+9)>count) {
int ldx=tIsdata.count()-1;
tbIsData isdrow = tIsdata[ldx];
DbgLv(1) << "XpDa:scn:    row" << rows << "run" << isdrow.runId
 << "dat" << isdrow.dataId;
DbgLv(1) << "XpDa:scn:     rads0 rads1 vals0 vals1"
 << isdrow.rads[0] << isdrow.rads[1] << isdrow.vals[0] << isdrow.vals[1];
}
//*DEBUG*
            break;
         }
         case 4:
         {
            update_WTable( sqry, cxs );
//*DEBUG*
if(rows<9 || (rows+9)>count) {
int ldx=tWsdata.count()-1;
tbWsData wsdrow = tWsdata[ldx];
DbgLv(1) << "XpDa:scn:    row" << rows << "run dat count" << wsdrow.runId
 << wsdrow.dataId << wsdrow.count;
}
//*DEBUG*
            break;
         }
         case 5:
         {
            tbSyData sydrow;

            sydrow.dataId    = sqry.value( cxs[  0 ] ).toInt();
            sydrow.runId     = sqry.value( cxs[  1 ] ).toInt();
            sydrow.expstart  = sqry.value( cxs[  2 ] ).toDateTime();
            sydrow.exptime   = sqry.value( cxs[  3 ] ).toInt();
            sydrow.tempera   = sqry.value( cxs[  4 ] ).toDouble();
            sydrow.speed     = sqry.value( cxs[  5 ] ).toDouble();
            sydrow.omgSqT    = sqry.value( cxs[  6 ] ).toDouble();
            sydrow.stageNum  = sqry.value( cxs[  7 ] ).toInt();

            tSydata << sydrow;

	    qDebug() << "[TIME 7 'S'] of scan_xpndata_auto(): " << int( elapse_t.elapsed() / 1000 ) << " sec";
//*DEBUG*
if(rows<21 || (rows+21)>count) {
DbgLv(1) << "XpDa:scn:    row" << rows << "run dat" << sydrow.runId << sydrow.dataId
 << "time" << sydrow.exptime << "temp" << sydrow.tempera << "speed" << sydrow.speed
 << "omg2t" << sydrow.omgSqT;

 qDebug() << "XpDa:scn:    row" << rows << "run dat" << sydrow.runId << sydrow.dataId
 << "time" << sydrow.exptime << "temp" << sydrow.tempera << "speed" << sydrow.speed
 << "omg2t" << sydrow.omgSqT;
 
 }
//*DEBUG*
            break;
         }
         case 6:
         {
            tbCrProf crprow;

            crprow.frunId    = sqry.value( cxs[  0 ] ).toInt();
            crprow.tempera   = sqry.value( cxs[  1 ] ).toDouble();
            crprow.sbrake    = sqry.value( cxs[  2 ] ).toInt();
            crprow.brrate    = sqry.value( cxs[  3 ] ).toDouble();
            crprow.holdte    = sqry.value( cxs[  4 ] ).toInt();
            crprow.holdsp    = sqry.value( cxs[  5 ] ).toInt();
            crprow.sette     = sqry.value( cxs[  6 ] ).toInt();
            crprow.sstintv   = sqry.value( cxs[  7 ] ).toDouble();
            crprow.stages    = sqry.value( cxs[  8 ] ).toString();
            crprow.stgrpm    = sqry.value( cxs[  9 ] ).toDouble();
            crprow.stgstart  = sqry.value( cxs[ 10 ] ).toDouble();
            crprow.stgdurat  = sqry.value( cxs[ 11 ] ).toDouble();
            crprow.stgaccel  = sqry.value( cxs[ 12 ] ).toDouble();
            crprow.paramids  = sqry.value( cxs[ 13 ] ).toString();

            tCrprof << crprow;

	    qDebug() << "[TIME 7 'C'] of scan_xpndata_auto(): " << int( elapse_t.elapsed() / 1000 ) << " sec";
//*DEBUG*
if(rows<21 || (rows+21)>count) {
DbgLv(1) << "XpDa:scn:    row" << rows << "fugeRunId" << crprow.frunId
 << "ssInterval" << crprow.sstintv;
 qDebug() << "XpDa:scn:    row" << rows << "fugeRunId" << crprow.frunId
 << "ssInterval" << crprow.sstintv;
 
}
//*DEBUG*
            break;
         }
      }
   }

   if ( scantype == 'C' )
   {  // Show mapping of RunId to ExperimentId to FugeRunProfileId
      QList< int >  rExpIds;  // ExperimentRun experiment Ids
      QList< int >  rRunIds;  // ExperimentRun run Ids
      QList< int >  dExpIds;  // ExperimentDefinition experiment Ids
      QList< int >  dFugIds;  // ExperimentDefinition fugeProfile Ids
DbgLv(1) << "XpDa:scn:    tCrprof count" << tCrprof.count();

      int iRunId      = sRunId.toInt();
      tabname         = "ExperimentRun";
      sqtab           = schname + "." + tabname;
      qrytab          = "\"" + schname + "\".\"" + tabname + "\"";
      qrytext         = "SELECT * from " + qrytab + ";";

      qDebug() << "[TIME 8 'C'] of scan_xpndata_auto(): " << int( elapse_t.elapsed() / 1000 ) << " sec";
      
      sqry            = dbxpn.exec( qrytext );

      qDebug() << "[TIME 9 'C'] of scan_xpndata_auto(): " << int( elapse_t.elapsed() / 1000 ) << " sec";
      
      qrec            = dbxpn.record( qrytab );

      qDebug() << "[TIME 10 'C'] of scan_xpndata_auto(): " << int( elapse_t.elapsed() / 1000 ) << " sec";
      
      // Get indexes to ExperimentId and RunId
      cols            = column_indexes( tabname, cnames, cxs );
      int jexpid      = cnames.indexOf( "ExperimentId" );
      int jrunid      = cnames.indexOf( "RunId" );
      while ( sqry.next() )
      {  // Build a list of ExperimentRun expIds and runIds
         int rExpId      = sqry.value( jexpid ).toInt();
         int rRunId      = sqry.value( jrunid ).toInt();
         rExpIds << rExpId;
         rRunIds << rRunId;
      }

      qDebug() << "[TIME 11 'C'] of scan_xpndata_auto(): " << int( elapse_t.elapsed() / 1000 ) << " sec";
     
      tabname         = "ExperimentDefinition";
      sqtab           = schname + "." + tabname;
      qrytab          = "\"" + schname + "\".\"" + tabname + "\"";
      qrytext         = "SELECT * from " + qrytab + ";";

      qDebug() << "[TIME 12 'C'] of scan_xpndata_auto(): " << int( elapse_t.elapsed() / 1000 ) << " sec";
      
      sqry            = dbxpn.exec( qrytext );

      qDebug() << "[TIME 13 'C'] of scan_xpndata_auto(): " << int( elapse_t.elapsed() / 1000 ) << " sec";
      
      qrec            = dbxpn.record( qrytab );

      qDebug() << "[TIME 14 'C'] of scan_xpndata_auto(): " << int( elapse_t.elapsed() / 1000 ) << " sec";
      
      cols            = column_indexes( tabname, cnames, cxs );
      // Get indexes to ExperimentId and FugeRunProfileId
      jexpid          = cnames.indexOf( "ExperimentId" );
      int jfruid      = cnames.indexOf( "FugeRunProfileId" );
      int jexpnm      = cnames.indexOf( "Name" );
//DbgLv(1) << "XpDa:scn:    jexpid jfruid" << jexpid << jfruid;
      while ( sqry.next() )
      {  // Build a list of ExperimentDefinition expIds and fugeIds
         int dExpId      = sqry.value( jexpid ).toInt();
         int dFugId      = sqry.value( jfruid ).toInt();
         dExpIds << dExpId;
         dFugIds << dFugId;
DbgLv(1) << "XpDa:scn:  dExpId" << dExpId << "dFugId" << dFugId
 << "Name" << sqry.value(jexpnm).toString();
      }

      qDebug() << "[TIME 15 'C'] of scan_xpndata_auto(): " << int( elapse_t.elapsed() / 1000 ) << " sec";
      
//DbgLv(1) << "XpDa:scn: dExpIds" << dExpIds;
//DbgLv(1) << "XpDa:scn: dFugIds" << dFugIds;
      // Map experimentId to runId, then fugeId to experimentId
      int rrndx       = rRunIds.indexOf( iRunId );
DbgLv(1) << "XpDa:scn:  iRunId" << iRunId << "rrndx" << rrndx;
      int iExpId      = rExpIds[ rrndx ];
      int dendx       = dExpIds.indexOf( iExpId );
DbgLv(1) << "XpDa:scn:  iExpId" << iExpId << "dendx" << dendx;
      int iFugId      = ( dendx >= 0 ) ? dFugIds[ dendx ] : iExpId;
      sstintv         = 0.0;
      double sstintl  = 1000.0;
DbgLv(1) << "XpDa:scn:    RunId" << iRunId << "ExperimentId" << iExpId
 << "FugeRunProfileId" << iFugId; 

      for ( int ii = 0; ii < tCrprof.count(); ii++ )
      {  // Look for matching fuge Id in CentrifugeRunProfile values
         sstintl          = tCrprof[ ii ].sstintv;

         if ( tCrprof[ ii ].frunId == iFugId )
         {  // Match found:  save corresponding SystemStatusInterval
            sstintv          = sstintl;
            break;
         }
      }

      sstintv         = ( sstintv > 0.0 ) ? sstintv : sstintl;
DbgLv(1) << "XpDa:scn:    FugId" << iFugId << "sstInterval" << sstintv;
   }

   if ( scantype == 'S' )
   {  // Determine experiment time offset (time at last rms=0 point)
      int ntssda        = tSydata.count();
      fnzstx            = 0;

      qDebug() << "[TIME 8 'S'] of scan_xpndata_auto(): " << int( elapse_t.elapsed() / 1000 ) << " sec";
      
      for ( int ii = 0; ii < ntssda; ii++ )
      {  // Find the index to the first non-zero speed
         int irSpeed       = (int)qRound( tSydata[ ii ].speed );
         if ( irSpeed > 0 )
         {
            fnzstx            = ii;
            break;
         }
      }

      // First non-zero-speed time index
      fnzstx            = ( fnzstx > 0 ) ? fnzstx : 1;                        //ALEXEY: if tSydata.count() == 1 (so only tSydata[0] exists), AND fnzstx was 0, THEN
                                                                              //        fnzstx = ( fnzstx > 0 ) ? fnzstx : 1; will lead to fnzstx = 1 !!!
                                                                              //        THEN   int etime1        = tSydata[ 1 ].exptime; WILL CRASH  
      int t2tx          = fnzstx + 1;
      // Experiment time offset (== negative of first time + interval)
      //int etime1        = tSydata[ fnzstx ].exptime;

      //ALEXEY: check additionally if tSydata[fnzstx] exists!!! 
      int etime1 = 0;
      if ( fnzstx < tSydata.count() )
      {
	etime1          = tSydata[ fnzstx ].exptime;
      }
      ////////////////
      
      int timeintv      = sstintv;
      if ( t2tx < tSydata.count() )
      {
         timeintv          = tSydata[ t2tx ].exptime - etime1;
      }
      timeintv          = qMax( timeintv, 1 );
      etimoff           = timeintv - etime1;
//*DEBUG*
DbgLv(1) << "XpDa:scn: fnzstx" << fnzstx << "etimoff" << etimoff
 << "  etime1 timeintv" << etime1 << timeintv << sstintv;
int nn=qMin(tSydata.count(),fnzstx+3);
DbgLv(1) << "XpDa:scn:  nn=" << nn << " Vals nn-5 to nn-1 --";
//if(nn>4) {
// DbgLv(1) << "XpDa:scn: times:"
//  << tSydata[nn-5].exptime
//  << tSydata[nn-4].exptime
//  << tSydata[nn-3].exptime
//  << tSydata[nn-2].exptime
//  << tSydata[nn-1].exptime;
// DbgLv(1) << "XpDa:scn: speeds:"
//  << tSydata[nn-5].speed
//  << tSydata[nn-4].speed
//  << tSydata[nn-3].speed
//  << tSydata[nn-2].speed
//  << tSydata[nn-1].speed; }
//*DEBUG*

 qDebug() << "[TIME 9 'S'] of scan_xpndata_auto(): " << int( elapse_t.elapsed() / 1000 ) << " sec";
   }

   return rows;
}



// Query and update data for a [AIFW]ScanData table
int US_XpnData::update_xpndata( const int runId, const QChar scantype )
{
   QStringList  cnames;
   QList< int > cxs;
   QSqlQuery    sqry;
   QSqlRecord   qrec;
   QString sRunId  = QString::number( runId );
   QString sExpTm;
   QString schname( "AUC_schema" );
   QString tabname( "AbsorbanceScanData" );
   QString sqtab, qrytab, qrytext;
   int norows      = 0;
   int nnrows      = 0;
   int nradps      = 2;

   if ( scantype == 'A' )
   {  // Get Absorbance old and new counts
      int tknt        = tAsdata.count();
      nradps          = ( tAsdata[ 0 ].radPath == tAsdata[ 1 ].radPath )
                        ? 1 : 2;
      for ( int ii = 0; ii < tknt; ii++ )
      {
         if ( tAsdata[ ii ].runId == runId )
            norows++;
      }
      norows         /= nradps;
      sExpTm          = QString::number( tAsdata[ tknt - 1 ].exptime );
//sExpTm = QString::number( tAsdata[ tknt - 8 ].exptime );
DbgLv(1) << "XpDa:updx:  sdat count" << tknt << "last-exptime" << sExpTm;;
   }
   else if ( scantype == 'F' )
   {  // Get Fluorescence old and new counts
      int tknt        = tFsdata.count();
      for ( int ii = 0; ii < tFsdata.count(); ii++ )
      {
         if ( tFsdata[ ii ].runId == runId )
            norows++;
      }
      tabname         = "FluorescenceScanData";
      sExpTm          = QString::number( tFsdata[ tknt - 1 ].exptime );
   }
   else if ( scantype == 'I' )
   {  // Get Interference old and new counts
      int tknt        = tIsdata.count();
      for ( int ii = 0; ii < tIsdata.count(); ii++ )
      {
         if ( tIsdata[ ii ].runId == runId )
            norows++;
      }
      tabname         = "InterferenceScanData";
      sExpTm          = QString::number( tIsdata[ tknt - 1 ].exptime );
   }
   else if ( scantype == 'W' )
   {  // Get Wavelength old and new counts
      int tknt        = tWsdata.count();
      for ( int ii = 0; ii < tWsdata.count(); ii++ )
      {
         if ( tWsdata[ ii ].runId == runId )
            norows++;
      }
      tabname         = "WavelengthScanData";
      sExpTm          = QString::number( tWsdata[ tknt - 1 ].exptime );
   }

   // Count the number of rows now in the data table
   sqtab           = schname + "." + tabname;
   qrytab          = "\"" + schname + "\".\"" + tabname + "\"";
   qrytext         = "SELECT count(*) from " + qrytab
                   + " WHERE \"RunId\"=" + sRunId + ";";
   sqry            = dbxpn.exec( qrytext );
   sqry.next();
   nnrows          = sqry.value( 0 ).toInt();

   qrytext         = "SELECT count(*) from " + qrytab
                   + " WHERE \"RunId\"=" + sRunId
                   + " AND \"ExperimentTime\">=" + sExpTm + ";";
   sqry            = dbxpn.exec( qrytext );
   sqry.next();
   int narows      = sqry.value( 0 ).toInt();
DbgLv(1) << "XpDa:updx:  qrytext" << qrytext;
DbgLv(1) << "XpDa:updx:  norows" << norows << "nnrows" << nnrows
 << "narows" << narows;

   // Return now if the new count is zero or the same as the old
   if ( nnrows == 0 )
      return nnrows;
   else if ( nnrows == norows )
      return (-nnrows);

   // Set new rows count or flag that it is the same as old count
   nnrows          = ( nnrows > norows ) ? nnrows : ( -nnrows );

   if ( nnrows > 0 )
   {  // There are new rows, so update data tables
      qrytext         = "SELECT * from " + qrytab
                      + " WHERE \"RunId\"=" + sRunId
                      + " AND \"ExperimentTime\">=" + sExpTm + ";";
      sqry            = dbxpn.exec( qrytext );
      int cols        = column_indexes( tabname, cnames, cxs );
DbgLv(1) << "XpDa:updx:   cols" << cols << "flds" << cxs.count();
      int rows        = 0;

      if ( scantype == 'A' )
      {
         while ( sqry.next() )
         {
            rows++;

            // Update new table entries
            update_ATable( sqry, cxs );
         }
      }

      else if ( scantype == 'F' )
      {
         while ( sqry.next() )
         {
            rows++;

            // Update new table entries
            update_FTable( sqry, cxs );
         }
      }

      else if ( scantype == 'I' )
      {
         while ( sqry.next() )
         {
            rows++;

            // Update new table entries
            update_ITable( sqry, cxs );
         }
      }

      else if ( scantype == 'W' )
      {
         while ( sqry.next() )
         {
            rows++;

            // Update new table entries
            update_WTable( sqry, cxs );
         }
      }
DbgLv(1) << "XpDa:updx:   narows" << narows << "updd rows" << rows;
   }

   return nnrows;
}

// Parse a string into a vector of doubles
int US_XpnData::parse_doubles( const QString svals, QVector< double >& dvals )
{
   QString svwk       = QString( svals ).replace( "{", "" ).replace( "}", "" );
   QStringList slvals = svwk.split( "," );
   int count          = slvals.count();
//DbgLv(1) << "XpDa:p_d: svals svwk" << QString(svals).left(20)
// << QString(svwk).left(20) << "count slv0 slv1" << count
// << slvals[0] << slvals[1];
   dvals.clear();
   dvals.reserve( count );

   for ( int ii = 0; ii < count; ii++ )
   {
      dvals << slvals[ ii ].toDouble();
   }
//DbgLv(1) << "XpDa:p_d:   dvals0 dvals1" << dvals[0] << dvals[1];

   return count;
}

// Load internal values from a vector of loaded rawDatas
void US_XpnData::load_auc( QVector< US_DataIO::RawData >& allData,
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
DbgLv(1) << "XpDa:LdMw: nfile" << nfile << "nscan npoint" << nscan << npoint;
DbgLv(1) << "XpDa:LdMw: fpaths0" << fpaths[0];
DbgLv(1) << "XpDa:LdMw: runID" << runID;
DbgLv(1) << "XpDa:LdMw: cur_dir" << cur_dir;

   wavelns    .clear();
   stgnbrs    .clear();
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
DbgLv(1) << "XpDa:LdMw:   trx" << trx << "triple" << triple;
      QString fpath    = fpaths[ trx ];
DbgLv(1) << "XpDa:LdMw:    fpath" << fpath;
      fnames << fpath.section( "/", -1, -1 );

      if ( ! triples.contains( triple ) )
         triples << triple;              // Build unique triple list

      else
      {  // Handle duplicate triple
DbgLv(1) << "XpDa:LdMw:    DUPLICATE triple" << triple << "trx" << trx;
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
               celchn           = triple.section( " / ", 0, 1 ).simplified();
DbgLv(1) << "XpDa:LdMw:      NEW triple" << triple << trnode;
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
DbgLv(1) << "XpDa:LdMw: ncell nchan nlambda" << ncell << nchan << nlambda;
DbgLv(1) << "XpDa:LdMw:   ncelchn ntriple" << ncelchn << ntriple;
DbgLv(1) << "XpDa:LdMw:   wlns size" << wavelns.size();

   QString stat_text = tr( "%1 triples loaded from %2 ..." )
                      .arg( ntriple ).arg( runID );
   emit status_text( stat_text );
}

// Return the input raw lambdas vector for the data
int US_XpnData::lambdas_raw( QVector< int >& wls )
{
   wls.clear();
   wls.reserve( nlambda );

   for ( int ii = 0; ii < nlambda; ii++ )
   {
      wls << wavelns[ ii ];
   }

   return nlambda;
}

// Clear all data arrays
void US_XpnData::clear()
{
   wavelns    .clear();     // Raw input wavelengths
   stgnbrs    .clear();     // Stage Numbers, selected type
   scnnbrs    .clear();     // Scan Numbers, selected type
   a_radii    .clear();
   datrecs    .clear();
   tExprun    .clear();
   tAsdata    .clear();
   tFsdata    .clear();
   tIsdata    .clear();
   tWsdata    .clear();

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

// Close the database connection
void US_XpnData::close()
{
   if ( dbxpn.open() )
      dbxpn.close();
}

// Find the index of a lambda value in the input raw list of lambdas
int US_XpnData::indexOfLambda( int lambda )
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
int US_XpnData::cellchannels( QStringList& celchns )
{
   celchns = cellchans;
   return ncelchn;
}

// Return the list of triple strings
int US_XpnData::data_triples( QStringList& trips )
{
   trips = triples;
   return triples.count();
}

// Populate the list of RawData objects from raw input XPN data
int US_XpnData::build_rawData( QVector< US_DataIO::RawData >& allData )
{
DbgLv(1) << "BldRawD IN";

   // First build the internal arrays and variables
   build_internals();

   const int low_memApc = 20;
   npoint          = a_radii.count();

   allData.clear();

   // Set up the interpolated byte array (all one bits)
   int    nbytei   = ( npoint + 7 ) / 8;
   QByteArray interpo( nbytei, '\255' );

   // Build a raw data set for each triple
   char   dtype0   = QString( runType ).toLatin1().constData()[ 0 ];
   char   dtype1   = QString( runType ).toLatin1().constData()[ 1 ];
   int    ccx      = 0;
   int    wvx      = 0;
   int    scnnbr   = 0;
   nscnn           = scnnbrs.count();                       // ALEXEY: is nscnn, the #scans, is the same for all stages ? NO!
   int    stgnbr   = 0;
   nstgn           = stgnbrs.count();
   ntscan          = 0;
DbgLv(1) << "BldRawD szs: ccd" << ccdescs.size()
 << "wvs" << wavelns.size() << "nlambda" << nlambda << "nscnn" << nscnn;

   for ( int trx = 0; trx < ntriple; trx++ )
   {
DbgLv(1) << "BldRawD     trx" << trx << " building scans... ccx" << ccx;
      US_DataIO::RawData rdata;
      QString uuid_str  = US_Util::new_guid();
      US_Util::uuid_parse( uuid_str, (unsigned char*)rdata.rawGUID );
      // Set triple values
      rdata.type[ 0 ]   = dtype0;
      rdata.type[ 1 ]   = dtype1;
      QString dtrip     = triples[ trx ];
      //QString celchn    = cellchans[ ccx ];
      QString celchn    = dtrip .section( "/", 0, 1 ).simplified();
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
      int ndscan        = 0;
      int rdx           = 0;
      //rdata.description = ccdescs.at( ccx );
      rdata.description = cc_description( celchn );
      qDebug() << "::build_rawData();  rdata.description -- " << rdata.description;
      
      QString triple    = triples[ trx ].replace( " / ", "/" );
      QString trnode    = trnodes[ trx ];
DbgLv(1) << "BldRawD         trip" << trnode << "descr" << rdata.description
 << "celchn" << celchn;

 qDebug() << "BUILD_RAW_DATA trip: descr:  " <<  trnode << "descr" << rdata.description
 << "celchn" << celchn;

QDateTime time10=QDateTime::currentDateTime();
      for ( int sgx = 0; sgx < nstgn; sgx++ )
      {  // Set scan values
         stgnbr            = stgnbrs[ sgx ];
         for ( int scx = 0; scx < nscnn; scx++ )
         {  // Set scan values
            scnnbr            = scnnbrs[ scx ];

            int datx          = scan_data_index( trnode, stgnbr, scnnbr );

            if ( datx < 0 )  continue;

            set_scan_data( datx );

            US_DataIO::Scan scan;
            scan.temperature  = csdrec.tempera;
            scan.rpm          = csdrec.speed;
            scan.seconds      = (double)csdrec.exptime;
            scan.omega2t      = csdrec.omgSqT;
            scan.wavelength   = csdrec.wavelen;
            scan.nz_stddev    = false;
            int npoint        = csdrec.rads->count();
            int nbytei        = ( npoint + 7 ) / 8;
            QByteArray interpo( nbytei, '\255' );
            scan.interpolated = interpo;

if(scx<3 || (scx+4)>nscnn)
DbgLv(1) << "BldRawD       trx wvx scx scnnbr" << trx << wvx << scx << scnnbr
 << "wavl" << scan.wavelength;

//*DEBUG*
if(trx==0) {
DbgLv(1) << "BldRawD      scx" << scx << "trx" << trx
 << "seconds" << scan.seconds << "rpm" << scan.rpm;
}
//*DEBUG*
            int kpoint        = get_readings( scan.rvalues, trx, sgx, scx );

            if ( kpoint < 0 )
               continue;                 // Skip output if no stage,scan match

if(scx<3 || (scx+4)>nscnn)
DbgLv(1) << "BldRawD        scx" << scx << "rvalues size" << scan.rvalues.size()
 << "rvalues[mid]" << scan.rvalues[scan.rvalues.size()/2];

            rdata.scanData << scan;      // Append a scan to a triple
            ndscan++;
         } // END: scan loop
      } // END: stage loop
DbgLv(1) << "BldRawD         EoSl: trx rdx" << trx << rdx;
QDateTime time20=QDateTime::currentDateTime();
DbgLv(1) << "BldRawD trx" << trx << "TIMEMS:scan loop time"
 << time10.msecsTo(time20);

      // Set the average speed for the final/only speed step
      mnscnn            = qMin( mnscnn, ndscan );
      mxscnn            = qMax( mxscnn, ndscan );
      ntscan           += ndscan;

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
//            tscans.clear();
int memAv2 = US_Memory::memory_profile();
DbgLv(1) << "BldRawD  memfree %: 1memAV" << memAv << "2memAV" << memAv2;
         }
      }
   } // END: triple loop
//*DEBUG*
for ( int trx = 0; trx < ntriple; trx++ )
{
 US_DataIO::RawData* rdata = &allData[trx];
 for (int scx=0; scx<rdata->scanCount(); scx++ )
 {
  double speed=rdata->scanData[scx].rpm;
DbgLv(1) << "BldRawD trx" << trx << "scx" << scx << "speed" << speed;
 }
}
//*DEBUG*

   QString stat_text = tr( "All %1 raw AUCs have been built." ).arg( ntriple );
   emit status_text( stat_text );
  
   nscan             = mxscnn;

DbgLv(1) << "BldRawD  DONE ntriple" << ntriple << "mxscnn" << mxscnn
 << "mnscnn" << mnscnn;
   return ntriple;
}

// Update the list of RawData objects from raw input XPN data
int US_XpnData::rebuild_rawData( QVector< US_DataIO::RawData >& allData )
{
DbgLv(1) << "rBldRawD IN";
QDateTime time00=QDateTime::currentDateTime();
   int mxscno      = 0;
   int mxscnn      = 0;
//   int mxrown      = 0;
int timi1=0;
int timi2=0;
int timi3=0;
int timi4=0;
int timi5=0;
int timi6=0;
int timi9=0;
QDateTime time10;
QDateTime time20;

   // Count the maximum scans for current AUC data
   for ( int ii = 0; ii < allData.count(); ii++ )
   {
      mxscno          = qMax( mxscno, allData[ ii ].scanCount() );
   }
DbgLv(1) << "rBldRawD mxscno" << mxscno;

   // First rebuild the internal arrays and variables
time10=QDateTime::currentDateTime();
   rebuild_internals();

   const int low_memApc = 20;
   npoint          = a_radii.count();

   // Set up the interpolated byte array (all one bits)
   int    nbytei   = ( npoint + 7 ) / 8;
   QByteArray interpo( nbytei, '\255' );

   // Rebuild the raw data set for each triple
   int    ccx      = 0;
   int    wvx      = 0;
   int    scnnbr   = 0;
   nscnn           = scnnbrs.count();                  //ALEXEY: is nscnn (# scan) is the same for all stages ? NO!
   int    stgnbr   = 0;
   nstgn           = stgnbrs.count();
DbgLv(1) << "rBldRawD ntriple nstgn nscnn" << ntriple << nstgn << nscnn
 << "st0 stn" << stgnbrs[0] << stgnbrs[nstgn-1]
 << "sc0 scn" << scnnbrs[0] << scnnbrs[nscnn-1];
time20=QDateTime::currentDateTime();
timi1+=time10.msecsTo(time20);
QDateTime time30=time20;
int timi7=0;

   for ( int trx = 0; trx < ntriple; trx++ )
   {  // Update scans for each triple
DbgLv(1) << "rBldRawD     trx" << trx << " rebuilding scans... ccx" << ccx;
time10=QDateTime::currentDateTime();
      US_DataIO::RawData* rdata = &allData[ trx ];
      QString triple    = triples[ trx ].replace( " / ", "/" );
      QString trnode    = trnodes[ trx ];
      int oscknt        = rdata->scanCount();   // Old scan count, this triple
      int ndscan        = 0;                    // New scan count
time20=QDateTime::currentDateTime();
timi2+=time10.msecsTo(time20);
QDateTime time07a=QDateTime::currentDateTime();

      for ( int sgx = 0; sgx < nstgn; sgx++ )
      {  // Set stage values
         stgnbr            = stgnbrs[ sgx ];
         for ( int scx = 0; scx < nscnn; scx++ )        //ALEXEY: is nscnn (# scan) is the same for all stages ? NO! 
         {  // Set scan values
time10=QDateTime::currentDateTime();
            scnnbr            = scnnbrs[ scx ];

            int datx          = scan_data_index( trnode, stgnbr, scnnbr );

if(datx<0 ) {
time20=QDateTime::currentDateTime();
timi3+=time10.msecsTo(time20); }
            if ( datx < 0 )  continue;

            ndscan++;
DbgLv(1) << "rBldRawD      sqx" << sgx << "scx" << scx
 << "ndscan oscknt" << ndscan << oscknt;
if(ndscan<oscknt) {
time20=QDateTime::currentDateTime();
timi3+=time10.msecsTo(time20); }
            if ( ndscan < oscknt )  continue;    // Skip old AUC scan

            set_scan_data( datx );               // Get scan table data

            US_DataIO::Scan scan;
            scan.temperature  = csdrec.tempera;
            scan.rpm          = csdrec.speed;
            scan.seconds      = (double)csdrec.exptime;
            scan.omega2t      = csdrec.omgSqT;
            scan.wavelength   = csdrec.wavelen;
            scan.nz_stddev    = false;
//            int npoint        = csdrec.rads->count();
            scan.interpolated = interpo;

if(scx<3 || (scx+4)>nscnn)
DbgLv(1) << "rBldRawD       trx wvx scx scnnbr" << trx << wvx << scx << scnnbr
 << "wavl" << scan.wavelength;

//*DEBUG*
if(trx==0) {
DbgLv(1) << "rBldRawD      scx" << scx << "trx" << trx
 << "seconds" << scan.seconds << "rpm" << scan.rpm;
}
//*DEBUG*
time20=QDateTime::currentDateTime();
timi3+=time10.msecsTo(time20);
time10=QDateTime::currentDateTime();
            int kpoint        = get_readings( scan.rvalues, trx, sgx, scx );

	    qDebug() << "INSIDE xpn_data: AFTER get_readings() 0";
	    
            if ( kpoint < 0 )
               continue;                 // Skip output if no stage,scan match

	    
	    qDebug() << "INSIDE xpn_data: AFTER get_readings() 1";
	     
if(scx<3 || (scx+4)>nscnn)
DbgLv(1) << "rBldRawD        scx" << scx << "rvalues size" << scan.rvalues.size()
 << "rvalues[mid]" << scan.rvalues[scan.rvalues.size()/2];

            //if ( ( scx + 1 ) == oscknt )
            if ( ndscan == oscknt )
            {  // If last of old scans, update in case readings were added


	       //int ksc           = sgx * nscnn + scx;                          //ALEXEY: crashes after this line: is this correct for ksc at multi-speed ?
	       //ALEXEY: is nscnn (# scan) is the same for all stages ? NO!

	       int ksc = oscknt - 1;

	       qDebug() << "INSIDE xpn_data: AFTER get_readings() 2a: ksc = sgx * nscnn + scx: " <<  sgx << " * " <<  nscnn << " + " << scx; 
	       qDebug() << "INSIDE xpn_data: AFTER get_readings() 2: ksc: " << ksc << ", rdata->scanData.size(): " << rdata->scanData.size();
		 
               rdata->scanData[ ksc ] = scan;
DbgLv(1) << "rBldRawD        ksc" << ksc << "++Scan Replace";
            }
            else
            {  // If new scan, append the scan to the triple
               rdata->scanData << scan;
int nscknt=rdata->scanCount();
DbgLv(1) << "rBldRawD        ndscan oscknt" << ndscan << oscknt << "++Scan Add  nscknt" << nscknt;
            }
time20=QDateTime::currentDateTime();

 qDebug() << "INSIDE xpn_Data 1";
   
timi4+=time10.msecsTo(time20);

  qDebug() << "INSIDE xpn_Data 2";
         } // END: scan loop
      } // END: stage loop
QDateTime time07b=QDateTime::currentDateTime();
timi7+=time07a.msecsTo(time07b);
DbgLv(1) << "rBldRawD         EoSl: trx" << trx << "ndscan" << ndscan;
//QDateTime time20=QDateTime::currentDateTime();
//DbgLv(1) << "rBldRawD trx" << trx << "TIMEMS:scan loop time"
// << time10.msecsTo(time20);

time10=QDateTime::currentDateTime();
      // Set the average speed for the final/only speed step
      mnscnn            = qMin( mnscnn, ndscan );
      mxscnn            = qMax( mxscnn, ndscan );
      ntscan           += ndscan;

DbgLv(1) << "rBldRawD    trx" << trx << " saving allData... ndscan" << ndscan;

      QString stat_text = tr( "Of %1 raw AUCs, updated %2" )
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
//            tscans.clear();
int memAv2 = US_Memory::memory_profile();
DbgLv(1) << "rBldRawD  memfree %: 1memAV" << memAv << "2memAV" << memAv2;
         }
      }
time20=QDateTime::currentDateTime();
timi5+=time10.msecsTo(time20);
   } // END: triple loop
time10=QDateTime::currentDateTime();
QDateTime time40=time10;
int timi8=time30.msecsTo(time40);
//*DEBUG*
for ( int trx = 0; trx < ntriple; trx++ )
{
 US_DataIO::RawData* rdata = &allData[trx];
 for (int scx=0; scx<rdata->scanCount(); scx++ )
 {
  double speed=rdata->scanData[scx].rpm;
DbgLv(1) << "rBldRawD trx" << trx << "scx" << scx << "speed" << speed;
 }
}
//*DEBUG*

   QString stat_text = tr( "All %1 raw AUCs have been rebuilt." ).arg( ntriple );
   emit status_text( stat_text );
  
   nscan             = mxscnn;

time20=QDateTime::currentDateTime();
timi6+=time10.msecsTo(time20);
DbgLv(1) << "rBldRawD  DONE ntriple" << ntriple << "mxscnn" << mxscnn
 << "mnscnn" << mnscnn;
timi9+=time00.msecsTo(time20);
DbgLv(1) << "rBldRawD   timi1" << timi1 << "timi2" << timi2
 << "timi3" << timi3 << "timi4" << timi4 << "timi5" << timi5 << "timi6" << timi6
 << "   timi9" << timi9 << "  timi8 timi7" << timi8 << timi7;
   return ntriple;
}

// Export RawData to openAUC (.auc) and TMST (.tmst) files
int US_XpnData::export_auc( QVector< US_DataIO::RawData >& allData )
{
   int nfiles        = 0;
#if 0
   if ( ! is_raw )
      return nfiles;
#endif

   int ss_reso        = 100;
   // If debug_text so directs, change set_speed_resolution
   QStringList dbgtxt = US_Settings::debug_text();
   for ( int ii = 0; ii < dbgtxt.count(); ii++ )
   {  // If debug text modifies ss_reso, apply it
      if ( dbgtxt[ ii ].startsWith( "SetSpeedReso" ) )
         ss_reso       = QString( dbgtxt[ ii ] ).section( "=", 1, 1 ).toInt();
   }
   int ntrips        = allData.count();
   QString ftype     = QString( allData[ 0 ].type[ 0 ] ) +
                       QString( allData[ 0 ].type[ 1 ] );
   QString fbase     = runID + "." + ftype + ".";
   QString cur_dir   = US_Settings::importDir() + "/" + runID + "/";
DbgLv(1) << "expA: ntrips ftype fbase" << ntrips << ftype << fbase
 << "cur_dir" << cur_dir;
   QDir dir;
   if ( ! dir.exists( cur_dir ) )
   {
      if ( dir.mkpath( cur_dir ) )
      {
         qDebug() << "Create directory " << cur_dir;
      }
      else
      {
         qDebug() << "*ERROR* Unable to create directory " << cur_dir;
      }
   }

   // Output AUC files and find first dataset with maximum scans
   int mxscans       = 0;
   int iiuse         = 0;
DbgLv(1) << "expA: ktrips" << triples.count() << "ktnodes" << trnodes.count();
   if ( trnodes.count() == 0  &&  ntrips > 0 )
   {
      for ( int ii = 0; ii < ntrips; ii++ )
      {
         QString tripl = triples[ ii ];
         QString tnode = tripl.replace( " / ", "." );
         trnodes << tnode;
      }
DbgLv(1) << "expA: ntrips" << ntrips << "ktnodes" << trnodes.count();
   }

   for ( int ii = 0; ii < ntrips; ii++ )
   {  // Create a file for each triple
      US_DataIO::RawData* rdata = &allData[ ii ];
#if 0
      QString trnode    = QString::number( rdata->cell ) + "." +
                          QString( rdata->channel ) + "." +
                          QString().sprintf( "%03d",
                                qRound( rdata->scanData[ 0 ].wavelength ) );
#endif
      QString trnode    = trnodes[ ii ];
//DbgLv(1) << "expA: ii" << ii << "triples[ii]" << triples[ii]
// << "trnodes[ii]" << trnodes[ii] << "trnode" << trnode;
DbgLv(1) << "expA: ii" << ii << "trnodes[ii]" << trnodes[ii] << "trnode" << trnode;
      QString fname     = fbase + trnode + ".auc";
      QString fpath     = cur_dir + fname;

      US_DataIO::writeRawData( fpath, *rdata );

      nfiles++;

      // Find index to first dataset with maximum scans
      int kscans        = rdata->scanCount();

      if ( kscans > mxscans )
      {
         mxscans           = kscans;
         iiuse             = ii;
      }
DbgLv(1) << "expA:   nf" << nfiles << "fname" << fname
 << "kscans" << kscans << "mxscans" << mxscans << "iiuse" << iiuse;
   }

   // Create a speed step vector
   QVector< US_SimulationParameters::SpeedProfile > speedsteps;
   US_DataIO::RawData* udata = &allData[ iiuse ];

   US_SimulationParameters::computeSpeedSteps( &udata->scanData, speedsteps );

   // Output time state files

   QStringList fkeys;
   QStringList ffmts;
   fkeys << "Time"        << "RawSpeed" << "SetSpeed" << "Omega2T"
         << "Temperature" << "Step"     << "Scan";
   ffmts << "I4"          << "F4"       << "I4"       << "F4"
         << "F4"          << "I2"       << "I2";

   US_TimeState tsobj;

   QString tspath    = cur_dir + runID + ".time_state.tmst";
   int ntimes        = udata->scanCount();
   int ntssda        = tSydata.count();
   double e_utime    = udata->scanData[ ntimes - 1 ].seconds;
   double e_stime    = ntssda > 0 ? tSydata[ ntssda - 1 ].exptime : 0.0;
DbgLv(1) << "expA:   ntimes ntssda" << ntimes << ntssda
 << "e_utime e_stime" << e_utime << e_stime;

   // If last scan time beyond last sysstat time, reload System Status
   if ( e_utime > e_stime )
   {
      int idRun         = tSydata[ 0 ].runId;
      tSydata.clear();
      scan_xpndata( idRun, 'S' );
      ntssda            = tSydata.count();
      e_stime           = ntssda > 0 ? tSydata[ ntssda - 1 ].exptime : 0.0;
DbgLv(1) << "expA:    UPDATED ntssda" << ntssda << "e_stime" << e_stime << "idRun" << idRun;
   }
//*DEBUG*
QVector< double > xv_scn;
QVector< double > yv_scn;
QVector< double > xv_tms;
QVector< double > yv_tms;
double utime1=udata->scanData[0].seconds;
double uomgt1=udata->scanData[0].omega2t;
double utime2=udata->scanData[ntimes-1].seconds;
double uomgt2=udata->scanData[ntimes-1].omega2t;
double avgrpm=0.0;
for (int jj=0; jj<ntimes; jj++)
{
 avgrpm += udata->scanData[jj].rpm;
 xv_scn << udata->scanData[jj].seconds;
 yv_scn << udata->scanData[jj].omega2t;
}
avgrpm /= (double)ntimes;
double delta_t = utime2 - utime1;
double delta_o = uomgt2 - uomgt1;
double cdelt_o = sq( avgrpm * M_PI / 30. ) * delta_t;
DbgLv(1) << "expA:DLT:SCN: time1 time2" << utime1 << utime2;
DbgLv(1) << "expA:DLT:SCN: omgt1 omgt2" << uomgt1 << uomgt2;
DbgLv(1) << "expA:DLT:SCN: delta-t delta-o" << delta_t << delta_o;
DbgLv(1) << "expA:DLT:SCN: avgrpm cdelt-o" << avgrpm << cdelt_o;
int jutime1=(int)utime1;
int jutime2=(int)utime2;
int jt1=0;
int jt2=0;
for ( int jj = 0; jj<ntssda; jj++ )
{
 int jtime=tSydata[jj].exptime+etimoff;
 if ( jtime < jutime1 ) continue;
 if ( jtime >= jutime2 )
 {
  jt2 = jj;
  break;
 }
 else if ( jt1 == 0 )
 {
  jt1 = jj;
 }
}
double stime1=tSydata[jt1].exptime+etimoff;
double stime2=tSydata[jt2].exptime+etimoff;
double somgt1=tSydata[jt1].omgSqT;
double somgt2=tSydata[jt2].omgSqT;
avgrpm=0.0;
for (int jj=jt1; jj<=jt2; jj++)
{
 avgrpm += tSydata[jj].speed;
 xv_tms << tSydata[jj].exptime+etimoff;
 yv_tms << tSydata[jj].omgSqT;
}
avgrpm /= (double)(jt2-jt1+1);
delta_t = stime2 - stime1;
delta_o = somgt2 - somgt1;
cdelt_o = sq( avgrpm * M_PI / 30. ) * delta_t;
DbgLv(1) << "expA:DLT:TMS: time1 time2" << stime1 << stime2;
DbgLv(1) << "expA:DLT:TMS: omgt1 omgt2" << somgt1 << somgt2;
DbgLv(1) << "expA:DLT:TMS: delta-t delta-o" << delta_t << delta_o;
DbgLv(1) << "expA:DLT:TMS: avgrpm cdelt-o" << avgrpm << cdelt_o;
double* xa_scn = xv_scn.data();
double* ya_scn = yv_scn.data();
double* xa_tms = xv_tms.data();
double* ya_tms = yv_tms.data();
double slope_s, slope_t, icept_s, icept_t;
double sigma_s, sigma_t, corre_s, corre_t;
int nscnp = xv_scn.count();
int ntmsp = xv_tms.count();
US_Math2::linefit( &xa_scn, &ya_scn, &slope_s, &icept_s, &sigma_s, &corre_s, nscnp );
US_Math2::linefit( &xa_tms, &ya_tms, &slope_t, &icept_t, &sigma_t, &corre_t, ntmsp );
DbgLv(1) << "expA:DLT:SCN: npoint" << nscnp << "x1,y1,x2,y2" << xa_scn[0] << ya_scn[0] << xa_scn[nscnp-1] << ya_scn[nscnp-1];
DbgLv(1) << "expA:DLT:TMS: npoint" << ntmsp << "x1,y1,x2,y2" << xa_tms[0] << ya_tms[0] << xa_tms[ntmsp-1] << ya_tms[ntmsp-1];
DbgLv(1) << "expA:DLT:SCN:  slope" << slope_s << "intercept" << icept_s;
DbgLv(1) << "expA:DLT:TMS:  slope" << slope_t << "intercept" << icept_t;
DbgLv(1) << "expA:DLT:SCN:   stddev" << sigma_s << "correlation" << corre_s;
DbgLv(1) << "expA:DLT:TMS:   stddev" << sigma_t << "correlation" << corre_t;
US_Math2::linefit( &ya_scn, &xa_scn, &slope_s, &icept_s, &sigma_s, &corre_s, nscnp );
US_Math2::linefit( &ya_tms, &xa_tms, &slope_t, &icept_t, &sigma_t, &corre_t, ntmsp );
DbgLv(1) << "expA:DLT:SCN: npoint" << nscnp << "x1,y1,x2,y2" << ya_scn[0] << xa_scn[0] << ya_scn[nscnp-1] << xa_scn[nscnp-1];
DbgLv(1) << "expA:DLT:TMS: npoint" << ntmsp << "x1,y1,x2,y2" << ya_tms[0] << xa_tms[0] << ya_tms[ntmsp-1] << xa_tms[ntmsp-1];
DbgLv(1) << "expA:DLT:SCN:  slope" << slope_s << "intercept" << icept_s;
DbgLv(1) << "expA:DLT:TMS:  slope" << slope_t << "intercept" << icept_t;
DbgLv(1) << "expA:DLT:SCN:   stddev" << sigma_s << "correlation" << corre_s;
DbgLv(1) << "expA:DLT:TMS:   stddev" << sigma_t << "correlation" << corre_t;
//*DEBUG*

   tsobj.open_write_data( tspath );      // Initialize TMST creation

   tsobj.set_keys( fkeys, ffmts );       // Define keys and formats

   if ( ntssda < ntimes )
   {
DbgLv(1) << "expA: ntimes" << ntimes << "tspath" << tspath;
      for ( int ii = 0;  ii < ntimes; ii++ )
      {  // Create a record for each scan
         US_DataIO::Scan* uscan = &udata->scanData[ ii ];
         int scannbr       = ii + 1;           // Scan number
         double rawSpeed   = uscan->rpm;
         double tempera    = uscan->temperature;
         double omega2t    = uscan->omega2t;
         double time       = uscan->seconds;
         double setSpeed   = qRound( rawSpeed / (double)ss_reso ) * (double)ss_reso;

         // Find the speed step (stage) to which this scan belongs
         int jstage        = 0;
         double ssDiff     = 1e+99;

         for( int jj = 0; jj < speedsteps.count(); jj++ )
         {
            double ssSpeed    = speedsteps[ jj ].set_speed;
            double jjDiff     = qAbs( ssSpeed - setSpeed );

            if ( jjDiff < ssDiff )
            {
               ssDiff            = jjDiff;
               jstage            = jj;
            }
         }

         int istagen       = jstage + 1;
         setSpeed          = speedsteps[ jstage ].set_speed;
         int isSpeed       = (int)qRound( setSpeed );
DbgLv(1) << "expA:   ii" << ii << "scan" << scannbr << "stage" << istagen
 << "speed" << rawSpeed << isSpeed << "time" << time;

         // Set values for this scan
         tsobj.set_value( fkeys[ 0 ], time     );      // Time in seconds
         tsobj.set_value( fkeys[ 1 ], rawSpeed );      // Raw speed
         tsobj.set_value( fkeys[ 2 ], isSpeed  );      // Set (stage) speed
         tsobj.set_value( fkeys[ 3 ], omega2t  );      // Omega-Squared-T
         tsobj.set_value( fkeys[ 4 ], tempera  );      // Temperature
         tsobj.set_value( fkeys[ 5 ], istagen  );      // Stage (speed step)
         tsobj.set_value( fkeys[ 6 ], scannbr  );      // Scan

         // Write the scan record
         tsobj.flush_record();
      }
   }
   else
   {  // Build TMST from System Status Data table values
      int ftx           = 1;
      int stgoff        = 0;

      for ( int ii = 0; ii < ntssda; ii++ )
      {  // Find the index to the first non-zero speed
         int irSpeed       = (int)qRound( tSydata[ ii ].speed );
         if ( irSpeed > 0 )
         {
            ftx               = ii;
            stgoff            = 1 - tSydata[ ii ].stageNum;
            int jj            = ntssda - 1;
            while ( tSydata[ jj ].speed < irSpeed  &&  jj > 1 )
            {
               jj--;
            }
            ntssda            = jj + 1;
            break;
         }
      }

      ftx               = ( ftx > 0 ) ? ( ftx - 1 ) : 0;
      etimoff           = -tSydata[ ftx ].exptime;   // Experiment time offset
DbgLv(1) << "expA: ftx" << ftx << "etimoff" << etimoff << "ntssda" << ntssda;
      QList< int > sctimes;

      for ( int ii = 0; ii < ntimes; ii++ )
      {  // Build a list of scan times
         US_DataIO::Scan* uscan = &udata->scanData[ ii ];
         int time          = (int)qRound( uscan->seconds );
         sctimes << time;
DbgLv(1) << "expA:  ii" << ii << "sctime" << time;
      }

      ntimes            = ntssda;
      int ietime        = (int)qRound( e_utime ) + 60;
      int time_n        = -1;                       // Initial values
      double speed_n    = 0.0;
      double omg2t_n    = 0.0;
      double tempe_n    = tSydata[ ftx ].tempera;
      double omg2t_sm   = 0.0;
  
      for ( int ii = ftx; ii < ntimes; ii++ )
      {  // Create a record for each system status row
         int time_p        = time_n;                // Previous values
         double speed_p    = speed_n;
         double omg2t_p    = omg2t_n;
         double tempe_p    = tempe_n;
         int time_e        = tSydata[ ii ].exptime;
         time_n            = time_e + etimoff;
         speed_n           = tSydata[ ii ].speed;
         omg2t_n           = tSydata[ ii ].omgSqT;
         tempe_n           = tSydata[ ii ].tempera;
         int stage         = tSydata[ ii ].stageNum + stgoff;
         if ( time_e > ietime  ||
              ( ( speed_n < 100 ) && ( ii > ( ftx + 10 ) ) ) )
         {  // Time well beyond last scan or speed dropping back down to zero
DbgLv(1) << "expA:   ii" << ii << "ftx" << ftx << "stage" << stage << "time_n" << time_n
 << "speed_n" << speed_n << "omg2t_n" << omg2t_n << "*BREAK*";
            break;
         }
         int timeinc       = time_n - time_p;       // Increments
         double trange     = timeinc > 0 ? ( 1.0 / (double)timeinc ) : 1.0;
         double speed_i    = ( speed_n - speed_p ) * trange;
         double omg2t_i    = ( omg2t_n - omg2t_p ) * trange;
         double tempe_i    = ( tempe_n - tempe_p ) * trange;
         int time_c        = time_p;                // Initial 1-sec values
         double rawSpeed   = speed_p;
         double omega2t    = omg2t_p;
         double tempera    = tempe_p;
DbgLv(1) << "expA:   ii" << ii << "stage" << stage << "time_n" << time_n
 << "speed_n" << speed_n << "omg2t_n" << omg2t_n;

         for ( int jj = 0; jj < timeinc; jj++ )
         {  // Expand values for each second in present range
            time_c++;                                // Bump to next second
            rawSpeed         += speed_i;
            omega2t          += omg2t_i;
            tempera          += tempe_i;

            omg2t_sm         += sq( rawSpeed * M_PI / 30.0 );
            int isSpeed       = (int)qRound( rawSpeed / (double)ss_reso ) * ss_reso;

            // Set scan number to matching-time scan or 0
            int scannbr       = sctimes.indexOf( time_c );
//DbgLv(1) << "expA:                 scan" << scannbr;
            scannbr           = ( scannbr >= 0 ) ? ( scannbr + 1 ) : 0;
 
DbgLv(1) << "expA:      jj" << jj << "scan" << scannbr
 << "time_c" << time_c << "speed" << rawSpeed << isSpeed
 << "omg2t(rec)" << omega2t << "omg2t(sum)" << omg2t_sm;
            omega2t           = omg2t_sm;

            // Set values for this time
            tsobj.set_value( fkeys[ 0 ], time_c   );  // Time in seconds
            tsobj.set_value( fkeys[ 1 ], rawSpeed );  // Raw speed
            tsobj.set_value( fkeys[ 2 ], isSpeed  );  // Set (stage) speed
            tsobj.set_value( fkeys[ 3 ], omega2t  );  // Omega-Squared-T
            tsobj.set_value( fkeys[ 4 ], tempera  );  // Temperature
            tsobj.set_value( fkeys[ 5 ], stage    );  // Stage (speed step)
            tsobj.set_value( fkeys[ 6 ], scannbr  );  // Scan

            // Write the scan record
            tsobj.flush_record();
         }  // END: loop thru seconds in status data interval
      }  // END: table data loop
   }  // END: have System Status Data

   // Complete write of TMST file and defining XML
   if ( tsobj.close_write_data() == 0 )
   {
      tsobj.write_defs( 0.0, "Optima" );
      nfiles        += 2;
DbgLv(1) << "expA: TMST files written.";
   }

   return nfiles;
}

// [AUTO, GMP] Export RawData to openAUC (.auc) and TMST (.tmst) files
int US_XpnData::export_auc_auto( QVector< US_DataIO::RawData >& allData, bool& tmstampOK )
{
   int nfiles        = 0;
#if 0
   if ( ! is_raw )
      return nfiles;
#endif

   int ss_reso        = 100;
   // If debug_text so directs, change set_speed_resolution
   QStringList dbgtxt = US_Settings::debug_text();
   for ( int ii = 0; ii < dbgtxt.count(); ii++ )
   {  // If debug text modifies ss_reso, apply it
      if ( dbgtxt[ ii ].startsWith( "SetSpeedReso" ) )
         ss_reso       = QString( dbgtxt[ ii ] ).section( "=", 1, 1 ).toInt();
   }
   int ntrips        = allData.count();
   QString ftype     = QString( allData[ 0 ].type[ 0 ] ) +
                       QString( allData[ 0 ].type[ 1 ] );
   QString fbase     = runID + "." + ftype + ".";
   QString cur_dir   = US_Settings::importDir() + "/" + runID + "/";
DbgLv(1) << "expA: ntrips ftype fbase" << ntrips << ftype << fbase
 << "cur_dir" << cur_dir;
   QDir dir;
   if ( ! dir.exists( cur_dir ) )
   {
      if ( dir.mkpath( cur_dir ) )
      {
         qDebug() << "Create directory " << cur_dir;
      }
      else
      {
         qDebug() << "*ERROR* Unable to create directory " << cur_dir;
      }
   }

   // Output AUC files and find first dataset with maximum scans
   int mxscans       = 0;
   int iiuse         = 0;
DbgLv(1) << "expA: ktrips" << triples.count() << "ktnodes" << trnodes.count();
   if ( trnodes.count() == 0  &&  ntrips > 0 )
   {
      for ( int ii = 0; ii < ntrips; ii++ )
      {
         QString tripl = triples[ ii ];
         QString tnode = tripl.replace( " / ", "." );
         trnodes << tnode;
      }
DbgLv(1) << "expA: ntrips" << ntrips << "ktnodes" << trnodes.count();
   }

   for ( int ii = 0; ii < ntrips; ii++ )
   {  // Create a file for each triple
      US_DataIO::RawData* rdata = &allData[ ii ];
#if 0
      QString trnode    = QString::number( rdata->cell ) + "." +
                          QString( rdata->channel ) + "." +
                          QString().sprintf( "%03d",
                                qRound( rdata->scanData[ 0 ].wavelength ) );
#endif
      QString trnode    = trnodes[ ii ];
//DbgLv(1) << "expA: ii" << ii << "triples[ii]" << triples[ii]
// << "trnodes[ii]" << trnodes[ii] << "trnode" << trnode;
DbgLv(1) << "expA: ii" << ii << "trnodes[ii]" << trnodes[ii] << "trnode" << trnode;
      QString fname     = fbase + trnode + ".auc";
      QString fpath     = cur_dir + fname;

      US_DataIO::writeRawData( fpath, *rdata );

      nfiles++;

      // Find index to first dataset with maximum scans
      int kscans        = rdata->scanCount();

      if ( kscans > mxscans )
      {
         mxscans           = kscans;
         iiuse             = ii;
      }
DbgLv(1) << "expA:   nf" << nfiles << "fname" << fname
 << "kscans" << kscans << "mxscans" << mxscans << "iiuse" << iiuse;
   }

   // Create a speed step vector
   QVector< US_SimulationParameters::SpeedProfile > speedsteps;
   US_DataIO::RawData* udata = &allData[ iiuse ];

   US_SimulationParameters::computeSpeedSteps( &udata->scanData, speedsteps );

   // Output time state files

   QStringList fkeys;
   QStringList ffmts;
   fkeys << "Time"        << "RawSpeed" << "SetSpeed" << "Omega2T"
         << "Temperature" << "Step"     << "Scan";
   ffmts << "I4"          << "F4"       << "I4"       << "F4"
         << "F4"          << "I2"       << "I2";

   US_TimeState tsobj;

   QString tspath    = cur_dir + runID + ".time_state.tmst";
   int ntimes        = udata->scanCount();
   int ntssda        = tSydata.count();
   double e_utime    = udata->scanData[ ntimes - 1 ].seconds;
   double e_stime    = ntssda > 0 ? tSydata[ ntssda - 1 ].exptime : 0.0;
DbgLv(1) << "expA:   ntimes ntssda" << ntimes << ntssda
 << "e_utime e_stime" << e_utime << e_stime;

   // If last scan time beyond last sysstat time, reload System Status
   if ( e_utime > e_stime )
   {
      int idRun         = tSydata[ 0 ].runId;
      tSydata.clear();
      scan_xpndata( idRun, 'S' );
      ntssda            = tSydata.count();
      e_stime           = ntssda > 0 ? tSydata[ ntssda - 1 ].exptime : 0.0;
DbgLv(1) << "expA:    UPDATED ntssda" << ntssda << "e_stime" << e_stime << "idRun" << idRun;
   }
//*DEBUG*
QVector< double > xv_scn;
QVector< double > yv_scn;
QVector< double > xv_tms;
QVector< double > yv_tms;
double utime1=udata->scanData[0].seconds;
double uomgt1=udata->scanData[0].omega2t;
double utime2=udata->scanData[ntimes-1].seconds;
double uomgt2=udata->scanData[ntimes-1].omega2t;
double avgrpm=0.0;
for (int jj=0; jj<ntimes; jj++)
{
 avgrpm += udata->scanData[jj].rpm;
 xv_scn << udata->scanData[jj].seconds;
 yv_scn << udata->scanData[jj].omega2t;
}
avgrpm /= (double)ntimes;
double delta_t = utime2 - utime1;
double delta_o = uomgt2 - uomgt1;
double cdelt_o = sq( avgrpm * M_PI / 30. ) * delta_t;
DbgLv(1) << "expA:DLT:SCN: time1 time2" << utime1 << utime2;
DbgLv(1) << "expA:DLT:SCN: omgt1 omgt2" << uomgt1 << uomgt2;
DbgLv(1) << "expA:DLT:SCN: delta-t delta-o" << delta_t << delta_o;
DbgLv(1) << "expA:DLT:SCN: avgrpm cdelt-o" << avgrpm << cdelt_o;
int jutime1=(int)utime1;
int jutime2=(int)utime2;
int jt1=0;
int jt2=0;
for ( int jj = 0; jj<ntssda; jj++ )
{
 int jtime=tSydata[jj].exptime+etimoff;
 if ( jtime < jutime1 ) continue;
 if ( jtime >= jutime2 )
 {
  jt2 = jj;
  break;
 }
 else if ( jt1 == 0 )
 {
  jt1 = jj;
 }
}
double stime1=tSydata[jt1].exptime+etimoff;
double stime2=tSydata[jt2].exptime+etimoff;
double somgt1=tSydata[jt1].omgSqT;
double somgt2=tSydata[jt2].omgSqT;
avgrpm=0.0;
for (int jj=jt1; jj<=jt2; jj++)
{
 avgrpm += tSydata[jj].speed;
 xv_tms << tSydata[jj].exptime+etimoff;
 yv_tms << tSydata[jj].omgSqT;
}
avgrpm /= (double)(jt2-jt1+1);
delta_t = stime2 - stime1;
delta_o = somgt2 - somgt1;
cdelt_o = sq( avgrpm * M_PI / 30. ) * delta_t;
DbgLv(1) << "expA:DLT:TMS: time1 time2" << stime1 << stime2;
DbgLv(1) << "expA:DLT:TMS: omgt1 omgt2" << somgt1 << somgt2;
DbgLv(1) << "expA:DLT:TMS: delta-t delta-o" << delta_t << delta_o;
DbgLv(1) << "expA:DLT:TMS: avgrpm cdelt-o" << avgrpm << cdelt_o;
double* xa_scn = xv_scn.data();
double* ya_scn = yv_scn.data();
double* xa_tms = xv_tms.data();
double* ya_tms = yv_tms.data();
double slope_s, slope_t, icept_s, icept_t;
double sigma_s, sigma_t, corre_s, corre_t;
int nscnp = xv_scn.count();
int ntmsp = xv_tms.count();
US_Math2::linefit( &xa_scn, &ya_scn, &slope_s, &icept_s, &sigma_s, &corre_s, nscnp );
US_Math2::linefit( &xa_tms, &ya_tms, &slope_t, &icept_t, &sigma_t, &corre_t, ntmsp );
DbgLv(1) << "expA:DLT:SCN: npoint" << nscnp << "x1,y1,x2,y2" << xa_scn[0] << ya_scn[0] << xa_scn[nscnp-1] << ya_scn[nscnp-1];
DbgLv(1) << "expA:DLT:TMS: npoint" << ntmsp << "x1,y1,x2,y2" << xa_tms[0] << ya_tms[0] << xa_tms[ntmsp-1] << ya_tms[ntmsp-1];
DbgLv(1) << "expA:DLT:SCN:  slope" << slope_s << "intercept" << icept_s;
DbgLv(1) << "expA:DLT:TMS:  slope" << slope_t << "intercept" << icept_t;
DbgLv(1) << "expA:DLT:SCN:   stddev" << sigma_s << "correlation" << corre_s;
DbgLv(1) << "expA:DLT:TMS:   stddev" << sigma_t << "correlation" << corre_t;
US_Math2::linefit( &ya_scn, &xa_scn, &slope_s, &icept_s, &sigma_s, &corre_s, nscnp );
US_Math2::linefit( &ya_tms, &xa_tms, &slope_t, &icept_t, &sigma_t, &corre_t, ntmsp );
DbgLv(1) << "expA:DLT:SCN: npoint" << nscnp << "x1,y1,x2,y2" << ya_scn[0] << xa_scn[0] << ya_scn[nscnp-1] << xa_scn[nscnp-1];
DbgLv(1) << "expA:DLT:TMS: npoint" << ntmsp << "x1,y1,x2,y2" << ya_tms[0] << xa_tms[0] << ya_tms[ntmsp-1] << xa_tms[ntmsp-1];
DbgLv(1) << "expA:DLT:SCN:  slope" << slope_s << "intercept" << icept_s;
DbgLv(1) << "expA:DLT:TMS:  slope" << slope_t << "intercept" << icept_t;
DbgLv(1) << "expA:DLT:SCN:   stddev" << sigma_s << "correlation" << corre_s;
DbgLv(1) << "expA:DLT:TMS:   stddev" << sigma_t << "correlation" << corre_t;
//*DEBUG*

   tsobj.open_write_data( tspath );      // Initialize TMST creation

   tsobj.set_keys( fkeys, ffmts );       // Define keys and formats

   if ( ntssda < ntimes )
   {
DbgLv(1) << "expA: ntimes" << ntimes << "tspath" << tspath;
      for ( int ii = 0;  ii < ntimes; ii++ )
      {  // Create a record for each scan
         US_DataIO::Scan* uscan = &udata->scanData[ ii ];
         int scannbr       = ii + 1;           // Scan number
         double rawSpeed   = uscan->rpm;
         double tempera    = uscan->temperature;
         double omega2t    = uscan->omega2t;
         double time       = uscan->seconds;
         double setSpeed   = qRound( rawSpeed / (double)ss_reso ) * (double)ss_reso;

         // Find the speed step (stage) to which this scan belongs
         int jstage        = 0;
         double ssDiff     = 1e+99;

         for( int jj = 0; jj < speedsteps.count(); jj++ )
         {
            double ssSpeed    = speedsteps[ jj ].set_speed;
            double jjDiff     = qAbs( ssSpeed - setSpeed );

            if ( jjDiff < ssDiff )
            {
               ssDiff            = jjDiff;
               jstage            = jj;
            }
         }

         int istagen       = jstage + 1;
         setSpeed          = speedsteps[ jstage ].set_speed;
         int isSpeed       = (int)qRound( setSpeed );
DbgLv(1) << "expA:   ii" << ii << "scan" << scannbr << "stage" << istagen
 << "speed" << rawSpeed << isSpeed << "time" << time;

         // Set values for this scan
         tsobj.set_value( fkeys[ 0 ], time     );      // Time in seconds
         tsobj.set_value( fkeys[ 1 ], rawSpeed );      // Raw speed
         tsobj.set_value( fkeys[ 2 ], isSpeed  );      // Set (stage) speed
         tsobj.set_value( fkeys[ 3 ], omega2t  );      // Omega-Squared-T
         tsobj.set_value( fkeys[ 4 ], tempera  );      // Temperature
         tsobj.set_value( fkeys[ 5 ], istagen  );      // Stage (speed step)
         tsobj.set_value( fkeys[ 6 ], scannbr  );      // Scan

         // Write the scan record
         tsobj.flush_record();
      }
   }
   else
   {  // Build TMST from System Status Data table values
      int ftx           = 1;
      int stgoff        = 0;

      for ( int ii = 0; ii < ntssda; ii++ )
      {  // Find the index to the first non-zero speed
         int irSpeed       = (int)qRound( tSydata[ ii ].speed );
         if ( irSpeed > 0 )
         {
            ftx               = ii;
            stgoff            = 1 - tSydata[ ii ].stageNum;
            int jj            = ntssda - 1;
            while ( tSydata[ jj ].speed < irSpeed  &&  jj > 1 )
            {
               jj--;
            }
            ntssda            = jj + 1;
            break;
         }
      }

      ftx               = ( ftx > 0 ) ? ( ftx - 1 ) : 0;
      etimoff           = -tSydata[ ftx ].exptime;   // Experiment time offset
DbgLv(1) << "expA: ftx" << ftx << "etimoff" << etimoff << "ntssda" << ntssda;
      QList< int > sctimes;

      for ( int ii = 0; ii < ntimes; ii++ )
      {  // Build a list of scan times
         US_DataIO::Scan* uscan = &udata->scanData[ ii ];
         int time          = (int)qRound( uscan->seconds );
         sctimes << time;
DbgLv(1) << "expA:  ii" << ii << "sctime" << time;
      }

      ntimes            = ntssda;
      int ietime        = (int)qRound( e_utime ) + 60;
      int time_n        = -1;                       // Initial values
      double speed_n    = 0.0;
      double omg2t_n    = 0.0;
      double tempe_n    = tSydata[ ftx ].tempera;
      double omg2t_sm   = 0.0;
  
      for ( int ii = ftx; ii < ntimes; ii++ )
      {  // Create a record for each system status row
         int time_p        = time_n;                // Previous values
         double speed_p    = speed_n;
         double omg2t_p    = omg2t_n;
         double tempe_p    = tempe_n;
         int time_e        = tSydata[ ii ].exptime;
         time_n            = time_e + etimoff;
         speed_n           = tSydata[ ii ].speed;
         omg2t_n           = tSydata[ ii ].omgSqT;
         tempe_n           = tSydata[ ii ].tempera;
         int stage         = tSydata[ ii ].stageNum + stgoff;
         if ( time_e > ietime  ||
              ( ( speed_n < 100 ) && ( ii > ( ftx + 10 ) ) ) )
         {  // Time well beyond last scan or speed dropping back down to zero
DbgLv(1) << "expA:   ii" << ii << "ftx" << ftx << "stage" << stage << "time_n" << time_n
 << "speed_n" << speed_n << "omg2t_n" << omg2t_n << "*BREAK*";

            if ( ii == ftx )
	      {
		tmstampOK = false;
		qDebug() << "tmstateOK ? " << tmstampOK;
	      }
            break;
         }
         int timeinc       = time_n - time_p;       // Increments
         double trange     = timeinc > 0 ? ( 1.0 / (double)timeinc ) : 1.0;
         double speed_i    = ( speed_n - speed_p ) * trange;
         double omg2t_i    = ( omg2t_n - omg2t_p ) * trange;
         double tempe_i    = ( tempe_n - tempe_p ) * trange;
         int time_c        = time_p;                // Initial 1-sec values
         double rawSpeed   = speed_p;
         double omega2t    = omg2t_p;
         double tempera    = tempe_p;
DbgLv(1) << "expA:   ii" << ii << "stage" << stage << "time_n" << time_n
 << "speed_n" << speed_n << "omg2t_n" << omg2t_n;

         for ( int jj = 0; jj < timeinc; jj++ )
         {  // Expand values for each second in present range
            time_c++;                                // Bump to next second
            rawSpeed         += speed_i;
            omega2t          += omg2t_i;
            tempera          += tempe_i;

            omg2t_sm         += sq( rawSpeed * M_PI / 30.0 );
            int isSpeed       = (int)qRound( rawSpeed / (double)ss_reso ) * ss_reso;

            // Set scan number to matching-time scan or 0
            int scannbr       = sctimes.indexOf( time_c );
//DbgLv(1) << "expA:                 scan" << scannbr;
            scannbr           = ( scannbr >= 0 ) ? ( scannbr + 1 ) : 0;
 
DbgLv(1) << "expA:      jj" << jj << "scan" << scannbr
 << "time_c" << time_c << "speed" << rawSpeed << isSpeed
 << "omg2t(rec)" << omega2t << "omg2t(sum)" << omg2t_sm;
            omega2t           = omg2t_sm;

            // Set values for this time
            tsobj.set_value( fkeys[ 0 ], time_c   );  // Time in seconds
            tsobj.set_value( fkeys[ 1 ], rawSpeed );  // Raw speed
            tsobj.set_value( fkeys[ 2 ], isSpeed  );  // Set (stage) speed
            tsobj.set_value( fkeys[ 3 ], omega2t  );  // Omega-Squared-T
            tsobj.set_value( fkeys[ 4 ], tempera  );  // Temperature
            tsobj.set_value( fkeys[ 5 ], stage    );  // Stage (speed step)
            tsobj.set_value( fkeys[ 6 ], scannbr  );  // Scan

            // Write the scan record
            tsobj.flush_record();
         }  // END: loop thru seconds in status data interval
      }  // END: table data loop
   }  // END: have System Status Data

   // Complete write of TMST file and defining XML
   if ( tsobj.close_write_data() == 0 )
   {
      tsobj.write_defs( 0.0, "Optima" );
      nfiles        += 2;
DbgLv(1) << "expA: TMST files written.";
   }

   return nfiles;
}




// Return a count of a specified type
int US_XpnData::countOf( QString key )
{
   mapCounts();

   return counts[ key ];
}

// Return a counts of sysData
QString US_XpnData::countOf_sysdata( QString key )
{
   mapCounts_sysdata();

   return counts_sysdata[ key ];
}

// Private slot to map counts and sizes
void US_XpnData::mapCounts_sysdata( void )
{
  counts_sysdata[ "exp_time"  ]  = QString::number(isyrec.exptime);
  counts_sysdata[ "stage_number" ]  = QString::number(isyrec.stageNum);
  counts_sysdata[ "tempera" ]    = QString::number(isyrec.tempera);
  counts_sysdata[ "last_rpm"  ]  =  QString::number(isyrec.speed);
  counts_sysdata[ "etim_off"  ]  = QString::number(etimoff);
}

// Return the channel description string for a given cell/channel
QString US_XpnData::cc_description( QString celchn )
{
   int ccx = cellchans.indexOf( celchn );
   return ( ccx < 0 ? "" : ccdescs.at( ccx ) );
}

// Set the run Id and run type ("RI"|"FI"|"IP"|"WI") for fetch/store
void US_XpnData::set_run_values( const QString arunid, const QString aruntype )
{
   runID    = ( arunid  .isEmpty() ) ? runID   : arunid;
   runType  = ( aruntype.isEmpty() ) ? runType : aruntype;
}

// Return the runID and runType strings for the data
void US_XpnData::run_values( QString& arunid, QString& aruntype )
{
   arunid   = runID;
   aruntype = runType;
}

// Set a common Scan Data table record
void US_XpnData::set_scan_data( const int datx, const QString arType )
{
   QString rType    = arType.isEmpty() ? runType : arType;

   if ( rType == "RI" )
   {
      tbAsData* asdrow = &tAsdata[ datx ];
      csdrec.dataId    = asdrow->dataId;
      csdrec.runId     = asdrow->runId;
      csdrec.samplName = asdrow->samplName;
      csdrec.exptime   = asdrow->exptime;
      csdrec.stageNum  = asdrow->stageNum;
      csdrec.scanSeqN  = asdrow->scanSeqN;
      csdrec.modPos    = asdrow->modPos;
      csdrec.cellPos   = asdrow->cellPos;
      csdrec.replic    = asdrow->replic;
      csdrec.wavelen   = asdrow->wavelen;
      csdrec.tempera   = asdrow->tempera;
      csdrec.speed     = asdrow->speed;
      csdrec.omgSqT    = asdrow->omgSqT;
      csdrec.count     = asdrow->count;
      csdrec.expstart  = asdrow->expstart;
      csdrec.scanTypeF = asdrow->scanTypeF;
      csdrec.radPath   = asdrow->radPath;
      csdrec.rads      = &asdrow->rads;
      csdrec.vals      = &asdrow->vals;
   }

   else if ( rType == "FI" )
   {
      tbFsData* fsdrow = &tFsdata[ datx ];
      csdrec.dataId    = fsdrow->dataId;
      csdrec.runId     = fsdrow->runId;
      csdrec.samplName = fsdrow->samplName;
      csdrec.exptime   = fsdrow->exptime;
      csdrec.stageNum  = fsdrow->stageNum;
      csdrec.scanSeqN  = fsdrow->scanSeqN;
      csdrec.modPos    = fsdrow->modPos;
      csdrec.cellPos   = fsdrow->cellPos;
      csdrec.replic    = fsdrow->replic;
      csdrec.wavelen   = fsdrow->wavelen;
      csdrec.tempera   = fsdrow->tempera;
      csdrec.speed     = fsdrow->speed;
      csdrec.omgSqT    = fsdrow->omgSqT;
      csdrec.count     = fsdrow->count;
      csdrec.expstart  = fsdrow->expstart;
      csdrec.scanTypeF = fsdrow->scanTypeF;
      csdrec.radPath   = fsdrow->radPath;
      csdrec.rads      = &fsdrow->rads;
      csdrec.vals      = &fsdrow->vals;
   }

   else if ( rType == "IP" )
   {
      tbIsData* isdrow = &tIsdata[ datx ];
      csdrec.dataId    = isdrow->dataId;
      csdrec.runId     = isdrow->runId;
      csdrec.samplName = isdrow->samplName;
      csdrec.exptime   = isdrow->exptime;
      csdrec.stageNum  = isdrow->stageNum;
      csdrec.scanSeqN  = isdrow->scanSeqN;
      csdrec.modPos    = isdrow->modPos;
      csdrec.cellPos   = isdrow->cellPos;
      csdrec.replic    = isdrow->replic;
      csdrec.wavelen   = isdrow->wavelen;
      csdrec.tempera   = isdrow->tempera;
      csdrec.speed     = isdrow->speed;
      csdrec.omgSqT    = isdrow->omgSqT;
      csdrec.count     = isdrow->count;
      csdrec.expstart  = isdrow->expstart;
      csdrec.scanTypeF = isdrow->scanTypeF;
      csdrec.radPath   = "A";
      csdrec.rads      = &isdrow->rads;
      csdrec.vals      = &isdrow->vals;
   }

   else if ( rType == "WI" )
   {
      tbWsData* wsdrow = &tWsdata[ datx ];
      csdrec.dataId    = wsdrow->dataId;
      csdrec.runId     = wsdrow->runId;
      csdrec.samplName = wsdrow->samplName;
      csdrec.exptime   = wsdrow->exptime;
      csdrec.stageNum  = wsdrow->stageNum;
      csdrec.scanSeqN  = wsdrow->scanSeqN;
      csdrec.modPos    = wsdrow->modPos;
      csdrec.cellPos   = wsdrow->cellPos;
      csdrec.replic    = wsdrow->replic;
      csdrec.wavelen   = wsdrow->scanPos;
      csdrec.tempera   = wsdrow->tempera;
      csdrec.speed     = wsdrow->speed;
      csdrec.omgSqT    = wsdrow->omgSqT;
      csdrec.count     = wsdrow->count;
      csdrec.expstart  = wsdrow->expstart;
      csdrec.scanTypeF = wsdrow->scanTypeF;
      csdrec.radPath   = wsdrow->radPath;
      csdrec.rads      = &wsdrow->wvls;
      csdrec.vals      = &wsdrow->vals;
   }
if (datx<5)
DbgLv(1) << "XpDa:ssd: datx" << datx << "csd: dId rId scan"
 << csdrec.dataId << csdrec.runId << csdrec.scanSeqN
 << "rType" << rType;
}

// Private slot to map counts and sizes
void US_XpnData::mapCounts( void )
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
   counts[ "scan_all"  ]  = ntscan;
   counts[ "scan_rows" ]  = ntsrow;
   counts[ "ascn_rows" ]  = arows;
   counts[ "fscn_rows" ]  = frows;
   counts[ "iscn_rows" ]  = irows;
   counts[ "wscn_rows" ]  = wrows;

   counts[ "last_rpm"  ]  = qRound( isyrec.speed );
   counts[ "etim_off"  ]  = etimoff;
}

#if 0
// Return a scan data index within the ScanData vector
int US_XpnData::scan_data_index( const QString trnode,
                                 const int mstage, const int mscann )
{
QDateTime time10;
QDateTime time20;
QDateTime time30;
QDateTime time40;
QDateTime time00;
time00=QDateTime::currentDateTime();
time10=QDateTime::currentDateTime();
int timi1=0;
int timi2=0;
int timi3=0;
int timi4=0;
int timi5=0;
int timi6=0;
int timi7=0;
int timi8=0;
int timi9=0;
   int datx       = -1;          // Default:  scan record not found
   int sdknt      = 0;
   sdknt          = ( runType == "RI" ) ? tAsdata.count() : sdknt;
   sdknt          = ( runType == "FI" ) ? tFsdata.count() : sdknt;
   sdknt          = ( runType == "IP" ) ? tIsdata.count() : sdknt;
   sdknt          = ( runType == "WI" ) ? tWsdata.count() : sdknt;
   int     mcell  = QString( trnode ).section( ".", 0, 0 ).toInt();
   QString mchan  = QString( trnode ).section( ".", 1, 1 ).simplified();
   int     mwavl  = QString( trnode ).section( ".", 2, 2 ).toInt();
time20=QDateTime::currentDateTime();
timi1+=time10.msecsTo(time20);
time10=QDateTime::currentDateTime();

   for ( int ii = 0; ii < sdknt; ii++ )
   {
time30=QDateTime::currentDateTime();
      set_scan_data( ii );
time40=QDateTime::currentDateTime();
timi2+=time30.msecsTo(time40);

      if ( ( csdrec.cellPos  != mcell  )  ||
           ( csdrec.radPath  != mchan  )  ||
           ( csdrec.wavelen  != mwavl  )  ||
           ( csdrec.stageNum != mstage )  ||
           ( csdrec.scanSeqN != mscann ) )
         continue;
      
      datx           = ii;
      break;
   }
time20=QDateTime::currentDateTime();
timi3+=time10.msecsTo(time20);
timi9+=time00.msecsTo(time20);
DbgLv(1) << "XpDa:scdX: trnode mstage mscann" << trnode << mstage << mscann
 << "datx" << datx << "ti's" << timi1 << timi2 << timi3 << timi9
 << "datrec size" << datrec.count() << sdknt;

   return datx;
}
#endif

#if 1
// Return a scan data index within the ScanData vector
int US_XpnData::scan_data_index( const QString trnode,
                                 const int mstage, const int mscann )
{
   static int pdatx = 0;
QDateTime time10;
QDateTime time20;
QDateTime time30;
QDateTime time40;
QDateTime time00;
time00=QDateTime::currentDateTime();
time10=QDateTime::currentDateTime();
int timi1=0;
int timi2=0;
int timi3=0;
int timi9=0;
   int datx       = -1;          // Default:  scan record not found
   int sdknt      = 0;
   int ntry       = 0;
   int jj         = pdatx;
   int     mcell  = QString( trnode ).section( ".", 0, 0 ).toInt();
   QString mchan  = QString( trnode ).section( ".", 1, 1 ).simplified();
   int     mwavl  = QString( trnode ).section( ".", 2, 2 ).toInt();
time20=QDateTime::currentDateTime();
timi1+=time10.msecsTo(time20);
time10=QDateTime::currentDateTime();

   if ( runType == "RI" )
   {
      tbAsData* asdrec;
      sdknt          = tAsdata.count();
      jj             = ( jj < sdknt ) ? jj : 0;

      for ( int ii = 0; ii < sdknt; ii++ )
      {
//time30=QDateTime::currentDateTime();
         asdrec         = &tAsdata[ jj ];
         ntry++;

         if ( ( asdrec->scanSeqN == mscann )  &&
              ( asdrec->stageNum == mstage )  &&
              ( asdrec->wavelen  == mwavl  )  &&
              ( asdrec->radPath  == mchan  )  &&
              ( asdrec->cellPos  == mcell  ) )
         {
            datx           = jj;
//time40=QDateTime::currentDateTime();
//timi2+=time30.msecsTo(time40);
            break;
         }
//time40=QDateTime::currentDateTime();
//timi2+=time30.msecsTo(time40);
         jj             = ( (++jj) < sdknt ) ? jj : 0;
      }
   }

   else if ( runType == "FI" )
   {
      tbFsData* fsdrec;
      sdknt          = tFsdata.count();
      jj             = ( jj < sdknt ) ? jj : 0;

      for ( int ii = 0; ii < sdknt; ii++ )
      {
//time30=QDateTime::currentDateTime();
         fsdrec         = &tFsdata[ jj ];
         ntry++;

         if ( ( fsdrec->scanSeqN == mscann )  &&
              ( fsdrec->stageNum == mstage )  &&
              ( fsdrec->wavelen  == mwavl  )  &&
              ( fsdrec->radPath  == mchan  )  &&
              ( fsdrec->cellPos  == mcell  ) )
         {
            datx           = jj;
            break;
         }
//time40=QDateTime::currentDateTime();
//timi2+=time30.msecsTo(time40);
         jj             = ( (++jj) < sdknt ) ? jj : 0;
      }
   }

   else if ( runType == "IP" )
   {
      tbIsData* isdrec;
      sdknt          = tIsdata.count();
      jj             = ( jj < sdknt ) ? jj : 0;

      for ( int ii = 0; ii < sdknt; ii++ )
      {
//time30=QDateTime::currentDateTime();
         isdrec         = &tIsdata[ jj ];
         ntry++;

         if ( ( isdrec->scanSeqN == mscann )  &&
              ( isdrec->stageNum == mstage )  &&
              ( isdrec->wavelen  == mwavl  )  &&
              ( mchan == "A"               )  &&
              ( isdrec->cellPos  == mcell  ) )
         {
            datx           = jj;
            break;
         }
//time40=QDateTime::currentDateTime();
//timi2+=time30.msecsTo(time40);
         jj             = ( (++jj) < sdknt ) ? jj : 0;
      }
   }

   else if ( runType == "WI" )
   {
      tbWsData* wsdrec;
      sdknt          = tWsdata.count();
      jj             = ( jj < sdknt ) ? jj : 0;

      for ( int ii = 0; ii < sdknt; ii++ )
      {
//time30=QDateTime::currentDateTime();
         wsdrec         = &tWsdata[ jj ];
         ntry++;

         if ( ( wsdrec->scanSeqN == mscann )  &&
              ( wsdrec->stageNum == mstage )  &&
              ( wsdrec->scanPos  == mwavl  )  &&
              ( wsdrec->radPath  == mchan  )  &&
              ( wsdrec->cellPos  == mcell  ) )
         {
            datx           = jj;
            break;
         }
//time40=QDateTime::currentDateTime();
//timi2+=time30.msecsTo(time40);
         jj             = ( (++jj) < sdknt ) ? jj : 0;
      }
   }

time20=QDateTime::currentDateTime();
timi3+=time10.msecsTo(time20);
timi9+=time00.msecsTo(time20);
DbgLv(1) << "XpDa:scdX: trnode mstage mscann" << trnode << mstage << mscann
 << "datx" << datx << "ti's" << timi1 << timi2 << timi3 << timi9
 << "sdknt" << sdknt << "ntry" << ntry;
   pdatx          = ( datx >= 0 ) ? datx : pdatx;

   return datx;
}
#endif

// Get data readings from a range of Scans table records
int US_XpnData::get_readings( QVector< double >& rvalues,
                              const int trx, const int sgx, const int scx )
{
   QString trnode = trnodes[trx];
   int stgnbr     = stgnbrs[sgx];
   int scnnbr     = scnnbrs[scx];
DbgLv(1) << "XpDa:grd: trx sgx scx" << trx << sgx << scx
 << "trnode stgnbr scannbr" << trnode << stgnbr << scnnbr;

   int datx       = scan_data_index( trnode, stgnbr, scnnbr );

   if ( datx < 0 )
   {
DbgLv(0) << "XpDa:grd: *** Unable to find data for" << trnode << stgnbr << scnnbr;
      return datx;
   }

   set_scan_data( datx );

   interp_rvalues( *csdrec.rads, *csdrec.vals, a_radii, rvalues );

   //return npoint;
   return datx;
}

// Interpolate readings values for a scan for radii at a regular interval
void US_XpnData::interp_rvalues( QVector< double >& rads,
                                 QVector< double >& vals,
                                 QVector< double >& rrads,
                                 QVector< double >& rvals )
{
   int jjs        = 1;
   int radknt     = rads.count();
   int lstrx      = radknt - 1;
   rvals.clear();

   for ( int ii = 0; ii < npoint; ii++ )
   {
      double radv    = rrads[ ii ];
      double valout  = vals[ jjs ];

      for ( int jj = jjs; jj < radknt; jj++ )
      {
         double radv1   = rads[ jj - 1 ];
         double radv2   = rads[ jj ];
         double drad    = radv2 - radv1;
         if ( drad == 0.0 )
            continue;
         valout         = vals[ jj ];
//if(valout<(-1e+66))
//DbgLv(1) << "XpDa:irv: ii jj" << ii << jj << "radv2" << radv2 << "valout" << valout;

         if ( radv < radv2  ||  jj == lstrx )
         {
            double val1    = vals[ jj - 1 ];
            double dval    = valout - val1;
            valout         = val1 + ( radv - radv1 ) * dval / drad;
            jjs            = jj;
//if(valout<(-1e+66))
//DbgLv(1) << "XpDa:irv: ii jj" << ii << jj << "valout" << valout
// << "dr r1 r2 rv" << drad << radv1 << radv2 << radv
// << "v1 v2" << val1 << vals[jj];
            break;
         }

         else if ( radv == radv2 )
         {
            jjs            = jj;
            break;
         }
      }

      rvals << valout;
   }
int mi=rads.count()/2;
int mo=rrads.count()/2;
int ni=rads.count()-1;
int no=rads.count()-1;
//DbgLv(0) << "XpDa:i_r:  rai0 rao0 raim raom rain raon"
// << rads[0] << rrads[0] << rads[mi] << rrads[mo] << rads[ni] << rrads[no];
//DbgLv(0) << "XpDa:i_r:   vli0 vlo0 vlim vlom vlin vlon"
// << vals[0] << rvals[0] << vals[mi] << rvals[mo] << vals[ni] << rvals[no];
}

// Compose text with detailed information about the data
QString US_XpnData::runDetails( void )
{
   // Create report string
   QString dbfn   = dbfile.section( "/", -1, -1 );

   // Create report string

   QString msg = tr( "Optima Data Statistics for RunID \"%1\",\n" )
                                           .arg( runID );
   if ( is_raw )
   {
      msg += tr( " from Database at Host:Port %1:%2 ." )
               .arg( dbhost ).arg( dbport ) ;
      msg += tr( "\nData Summary.\n" );
   }

   else
   {
      msg += tr( " from Directory \"%1\",\n" ).arg( cur_dir );
      msg += tr( " using files from \"%1\"\n" ) .arg( fnames[ 0 ] );
      msg += tr( "               to \"%1\".\n" ).arg( fnames[ nfile - 1 ] );
      msg += tr( "\nData Summary.\n" );
      msg += tr( "    AUC Files Count:          %1\n" ).arg( nfile );
   }

   msg += tr( "    Triples Count:            %1\n" ).arg( ntriple );
   msg += tr( "    Cell/Channel Count:       %1\n" ).arg( ncelchn );
   msg += tr( "    Radius Points:            %1\n" ).arg( npoint );
   msg += tr( "    Max. Scan/Triple Count:   %1\n" ).arg( nscan );
   msg += tr( "    Total Scans Count:        %1\n" ).arg( ntscan );
   msg += tr( "    Wavelengths Count:        %1\n" ).arg( nlambda );
   msg += tr( "    System Status Interval:   %1\n" ).arg( sstintv );

   // Report on *ScanData table statistics
   QStringList rtyps;
   rtyps << "RI" << "FI" << "IP" << "WI";
   msg += tr( "\nScanData Tables.\n" );

   for ( int ii = 0; ii < rtyps.count(); ii++ )
   {
      QString rtype = rtyps[ ii ];
      QString tabname( "(unknown)" );
      int sdknt     = 0;

      if (      rtype == "RI" )
      {
         tabname  = "AbsorbanceScanData";
         sdknt    = tAsdata.count();
      }
      else if ( rtype == "FI" )
      {
         tabname  = "FluorescenceScanData";
         sdknt    = tFsdata.count();
      }
      else if ( rtype == "IP" )
      {
         tabname  = "InterferenceScanData";
         sdknt    = tIsdata.count();
      }
      else if ( rtype == "WI" )
      {
         tabname  = "WavelengthScanData";
         sdknt    = tWsdata.count();
      }

      msg += tr( "\n  Table Name:  '" ) + tabname + "'  (" + rtype + ")\n";
      msg += tr( "    Count of Data Records:    %1\n" ).arg( sdknt );

      if ( sdknt < 1 )  continue;

      int exptm1, exptm2, stage1, stage2, scann1, scann2;
      int wavel1, wavel2, vlknt1, vlknt2;
      double tempe1, tempe2, speed1, speed2, omgsq1, omgsq2;
      double radvs1, radvs2, radve1, radve2;
      double valus1, valus2, value1, value2;
      QString sname1, sname2;
      exptm1 = stage1 = scann1 = wavel1 = vlknt1 = 0;
      tempe1 = speed1 = omgsq1 = radvs1 = radve1 = valus1 = value1 = 0.0;

      for ( int jj = 0; jj < sdknt; jj++ )
      {
         set_scan_data( jj, rtype );

         exptm2        = csdrec.exptime;
         stage2        = csdrec.stageNum;
         scann2        = csdrec.scanSeqN;
         wavel2        = csdrec.wavelen;
         tempe2        = csdrec.tempera;
         speed2        = csdrec.speed;
         omgsq2        = csdrec.omgSqT;
         vlknt2        = csdrec.count;
         radvs2        = csdrec.rads->at( 0 );
         radve2        = csdrec.rads->at( vlknt2 - 1 );
         valus2        = csdrec.vals->at( 0 );
         value2        = csdrec.vals->at( vlknt2 - 1 );
         sname2        = csdrec.samplName;

         if ( jj == 0 )
         {
            exptm1        = exptm2;
            stage1        = stage2;
            scann1        = scann2;
            wavel1        = wavel2;
            tempe1        = tempe2;
            speed1        = speed2;
            omgsq1        = omgsq2;
            vlknt1        = vlknt2;
            radvs1        = radvs2;
            radve1        = radve2;
            valus1        = valus2;
            value1        = value2;
            sname1        = sname2;
         }
      }  // END: table instance loop

      msg += tr( "    First Seconds:            %1\n" ).arg( exptm1 );
      msg += tr( "    Last  Seconds:            %1\n" ).arg( exptm2 );
      msg += tr( "    First Stage:              %1\n" ).arg( stage1 );
      msg += tr( "    Last  Stage:              %1\n" ).arg( stage2 );
      msg += tr( "    First Scan Number:        %1\n" ).arg( scann1 );
      msg += tr( "    Last  Scan Number:        %1\n" ).arg( scann2 );
      msg += tr( "    First Wavelength:         %1\n" ).arg( wavel1 );
      msg += tr( "    Last  Wavelength:         %1\n" ).arg( wavel2 );
      msg += tr( "    First Temperature:        %1\n" ).arg( tempe1 );
      msg += tr( "    Last  Temperature:        %1\n" ).arg( tempe2 );
      msg += tr( "    First Speed (RPM):        %1\n" ).arg( speed1 );
      msg += tr( "    Last  Speed (RPM):        %1\n" ).arg( speed2 );
      msg += tr( "    First OmegaSquaredT:      %1\n" ).arg( omgsq1 );
      msg += tr( "    Last  OmegaSquaredT:      %1\n" ).arg( omgsq2 );
      msg += tr( "    First Value Count:        %1\n" ).arg( vlknt1 );
      msg += tr( "    Last  Value Count:        %1\n" ).arg( vlknt2 );
      msg += tr( "    First Start Radius:       %1\n" ).arg( radvs1 );
      msg += tr( "    Last  Start Radius:       %1\n" ).arg( radvs2 );
      msg += tr( "    First End Radius:         %1\n" ).arg( radve1 );
      msg += tr( "    Last  End Radius:         %1\n" ).arg( radve2 );
      msg += tr( "    First Start Reading:      %1\n" ).arg( valus1 );
      msg += tr( "    Last  Start Reading:      %1\n" ).arg( valus2 );
      msg += tr( "    First End Reading:        %1\n" ).arg( value1 );
      msg += tr( "    Last  End Reading:        %1\n" ).arg( value2 );
      msg += tr( "    First Sample Name:        %1\n" ).arg( sname1 );
      msg += tr( "    Last  Sample Name:        %1\n" ).arg( sname2 );

   }  // END: table type loop


   msg += tr( "    System Status Interval:   %1\n" ).arg( sstintv );
   msg += tr( "    Instant SysStat Interval: %1\n" ).arg( issintv );
   
   // Compose message for values in System Status Data table
   int ssknt      = tSydata.count();
   if ( ssknt < 2 )
      return msg;

   int sdknt       = 0;
   double e_dtime  = 0.0;
   double e_stime  = 0.0;
   if      ( runType == "RI" )
   {
      sdknt          = tAsdata.count();
      e_dtime        = tAsdata[ sdknt - 1 ].exptime;
   }
   else if ( runType == "FI" )
   {
      sdknt          = tFsdata.count();
      e_dtime        = tFsdata[ sdknt - 1 ].exptime;
   }
   else if ( runType == "IP" )
   {
      sdknt          = tIsdata.count();
      e_dtime        = tIsdata[ sdknt - 1 ].exptime;
   }
   else if ( runType == "WI" )
   {
      sdknt          = tWsdata.count();
      e_dtime        = tWsdata[ sdknt - 1 ].exptime;
   }

   int runID      = tSydata[ 0 ].runId;
   e_stime        = ssknt > 0 ? tSydata[ ssknt - 1 ].exptime : 0.0;
DbgLv(1) << "XpDa:Det: sdknt ssknt" << sdknt << ssknt
 << "  e_dtime e_stime" << e_dtime << e_stime;
   if ( e_stime < e_dtime )
   {  // For System Status last time less than last data time, rescan SysStat
      tSydata.clear();
      scan_xpndata( runID, 'S' );
      ssknt          = tSydata.count();
      e_stime        = ssknt > 0 ? tSydata[ ssknt - 1 ].exptime : 0.0;
DbgLv(1) << "XpDa:Det:  UPD ssknt e_stime" << ssknt << e_stime;
   }

   int kk         = ssknt - 1;

   QString tabname( "SystemStatusData" );
   msg += tr( "\n  Table Name:  '" ) + tabname + "'  ( runID ="
          + QString::number( runID ) + " )\n";
   msg += tr( "    Count of Data Records:    %1\n" ).arg( ssknt );
   int stage1     = tSydata[  0 ].stageNum;
   int stage2     = tSydata[ kk ].stageNum;
   int time1      = tSydata[  0 ].exptime + etimoff;
   int time2      = tSydata[ kk ].exptime + etimoff;
   double temp1   = tSydata[  0 ].tempera;
   double temp2   = tSydata[ kk ].tempera;
   double speed1  = tSydata[  0 ].speed;
   double speed2  = tSydata[ kk ].speed;
   double omega1  = tSydata[  0 ].omgSqT;
   double omega2  = tSydata[ kk ].omgSqT;
   double templ   = 10000.0;
   double temph   = 0.0;
   double speedh  = -1.0;
   for ( int jj = 0; jj < ssknt; jj++ )
   {
      templ          = qMin( templ, tSydata[ jj ].tempera );
      temph          = qMax( temph, tSydata[ jj ].tempera );
      speedh         = qMax( speedh, tSydata[ jj ].speed );
   }
   msg += tr( "    First Stage:              %1\n" ).arg( stage1 );
   msg += tr( "    Last  Stage:              %1\n" ).arg( stage2 );
   msg += tr( "    First Time (secs.):       %1\n" ).arg( time1  );
   msg += tr( "    Last  Time (secs.):       %1\n" ).arg( time2  );
   msg += tr( "    First Temperature:        %1\n" ).arg( temp1  );
   msg += tr( "    Last  Temperature:        %1\n" ).arg( temp2  );
   msg += tr( "    First Speed (rpm):        %1\n" ).arg( speed1 );
   msg += tr( "    Last  Speed (rpm):        %1\n" ).arg( speed2 );
   msg += tr( "    First OmegaSquaredT:      %1\n" ).arg( omega1 );
   msg += tr( "    Last  OmegaSquaredT:      %1\n" ).arg( omega2 );
   msg += tr( "    Low   Temperature:        %1\n" ).arg( templ  );
   msg += tr( "    High  Temperature:        %1\n" ).arg( temph  );
   msg += tr( "    High  Speed (rpm):        %1\n" ).arg( speedh );

   return msg;
}

// Return the connection last error string
QString US_XpnData::lastError( void )
{
   return dbxpn.lastError().text();
}

// Build the internal variables and arrays
void US_XpnData::build_internals( )
{
   cellchans .clear();
   ccdescs   .clear();
   triples   .clear();
   trnodes   .clear();
   wavelns   .clear();
   stgnbrs   .clear();
   scnnbrs   .clear();
   datrecs   .clear();
   QString scell = "1";
   QString schan = "A";
   QString swavl = "280";
   mnstgn        = 99999;
   mxstgn        = 0;
   mnscnn        = 99999;
   mxscnn        = 0;
   nlambda       = 0;
   ntriple       = 0;
   
   //ncelchn       = 0;  //ALEXEY do we need to reset it fro automated flow and in general ?
      
   int sdknt     = 0;
   sdknt         = ( runType == "RI" ) ? tAsdata.count() : sdknt;
   sdknt         = ( runType == "FI" ) ? tFsdata.count() : sdknt;
   sdknt         = ( runType == "IP" ) ? tIsdata.count() : sdknt;
   sdknt         = ( runType == "WI" ) ? tWsdata.count() : sdknt;
   QVector< double > radmins;
   QVector< double > radmaxs;
   QVector< double > radincs;
   QVector< int >    radknts;

   // Analyze *ScanData table values
DbgLv(1) << "XpDa:b_i:   csdrec count" << sdknt;
   for ( int ii = 0; ii < sdknt; ii++ )
   {  // Get cell/channel, triple, lambda lists
      set_scan_data( ii );

      int stage     = csdrec.stageNum;
      int scnnbr    = csdrec.scanSeqN;
      int celpos    = csdrec.cellPos;
      int iwavln    = csdrec.wavelen;
      schan         = csdrec.radPath;
      scell         = QString::number( celpos );
      swavl         = QString::number( iwavln );
      QString smpnm = csdrec.samplName;
      int vcount    = csdrec.count;
      int rknt      = csdrec.rads->count();
      int vknt      = csdrec.vals->count();
      mnstgn        = qMin( mnstgn, stage );
      mxstgn        = qMax( mxstgn, stage );
      mnscnn        = qMin( mnscnn, scnnbr );
      mxscnn        = qMax( mxscnn, scnnbr );

      QString cechn = scell + " / " + schan;
      QString tripl = cechn + " / " + swavl;
      QString tnode = scell + "." + schan + "." + swavl;
      QString darec = tnode + "."
                    + QString().sprintf( "%05i.%05i", stage, scnnbr );
DbgLv(1) << "XpDa:b_i: ii" << ii << "schan cechn"
 << schan << cechn << "darec" << darec
 << "rad0 rad1" << csdrec.rads->at(0) << csdrec.rads->at(1);

      if ( !triples.contains( tripl ) )
      {
         triples << tripl;
         trnodes << tnode;
         ntriple++;
      }

      // Build lists of used stages, scans, datarecs (trnode.stage.scan)
      int stgnx     = stgnbrs.indexOf( stage );
      if ( stgnx < 0 )
      {
         stgnbrs << stage;
      }

      int scnnx     = scnnbrs.indexOf( scnnbr );
      if ( scnnx < 0 )
      {
         scnnbrs << scnnbr;              // Save unique ScanNumber
      }

      int rdatx     = datrecs.indexOf( darec );
      if ( rdatx < 0 )
      {
         datrecs << darec;
      }

      if ( vcount != rknt  ||  vcount != vknt )
      {
         qDebug() << "*** ii vcount rknt vknt tnode"
                  << ii << vcount << rknt << vknt << tnode;
      }

      // Accumulate radius statistics
      double rmin   = csdrec.rads->at( 0 );
      double rmax   = csdrec.rads->at( rknt - 1 );
      double rinc   = ( rmax - rmin ) / (double)( rknt -1 );
      radknts << rknt;
      radmins << rmin;
      radmaxs << rmax;
      radincs << rinc; 

   }  // END: *ScanData records loop
DbgLv(1) << "XpDa:b_i:    radknts count" << radknts.count();
DbgLv(1) << "XpDa:b_i:    scnnbrs count" << scnnbrs.count();

   // Determine variations in radius values
   double rminl  = radmins[ 0 ];
   double rminh  = rminl;
   double rmaxl  = radmaxs[ 0 ];
   double rmaxh  = rmaxl;
   double rincl  = radincs[ 0 ];
   double rinch  = rincl;
   int    rkntl  = radknts[ 0 ];
   int    rknth  = rkntl;

   for ( int ii = 0; ii < sdknt; ii++ )
   {
      double rmin   = radmins[ ii ];
      double rmax   = radmaxs[ ii ];
      double rinc   = radincs[ ii ];
      int    rknt   = radknts[ ii ];
      rminl         = qMin( rminl, rmin );
      rminh         = qMax( rminh, rmin );
      rmaxl         = qMin( rmaxl, rmax );
      rmaxh         = qMax( rmaxh, rmax );
      rincl         = qMin( rincl, rinc );
      rinch         = qMax( rinch, rinc );
      rkntl         = qMin( rkntl, rknt );
      rknth         = qMax( rknth, rknt );
   }
DbgLv(1) << "XpDa:b_i:   rminl rminh" << rminl << rminh;
DbgLv(1) << "XpDa:b_i:   rmaxl rmaxh" << rmaxl << rmaxh;
DbgLv(1) << "XpDa:b_i:   rincl rinch" << rincl << rinch;
DbgLv(1) << "XpDa:b_i:   rkntl rknth" << rkntl << rknth;
   // Build AUC radius values to use
   rminl         = ( rminl + rminh ) * 0.5;
   rmaxl         = ( rmaxl + rmaxh ) * 0.5;
   rincl         = ( rincl + rinch ) * 0.5;
   rkntl         = qMax( rkntl, rknth );
   rincl         = (double)qRound( rincl * 1.0e4 ) * 1.0e-4;
   rminl         = (double)qRound( rminl / rincl ) * rincl;
   rmaxl         = (double)qRound( rmaxl / rincl ) * rincl;
   radinc        = rincl;
DbgLv(1) << "XpDa:b_i:     rmin rmax" << rminl << rmaxl;
DbgLv(1) << "XpDa:b_i:     rinc rknt" << rincl << rkntl;
   a_radii.clear();
   for ( int ii = 0; ii < rkntl; ii++ )
   {
      a_radii << rminl;
      rminl        += rincl;
   }
DbgLv(1) << "XpDa:b_i:       rad0 radn" << a_radii[0] << a_radii[rkntl-1];


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
      int ichan     = QString( " AB" ).indexOf( schan );
      int iwavl     = swavl.toInt();
      QString cechn = scell + " / " + schan;
DbgLv(1) << "XpDa:b_i:trip: ii" << ii << "icell ichan schan cechn"
 << icell << ichan << schan << cechn << "tripl" << tripl;

      if ( !cellchans.contains( cechn ) )
      {
         cellchans << cechn;
         ncelchn++;
         bool no_desc  = true;

         for ( int jj = 0; jj < sdknt; jj++ )
         {  // Use Samples data to get cell/channel description
            set_scan_data( jj );

            int jcell     = csdrec.cellPos;
            QString chanj = csdrec.radPath;
            int jchan     = QString( " AB" ).indexOf( chanj );
DbgLv(1) << "XpDa:b_i:   jcell jchan" << jcell << jchan;

            if ( jcell == icell  &&  jchan == ichan )
            {
               no_desc       = false;
               ccdescs << csdrec.samplName;
DbgLv(1) << "XpDa:b_i:     samplName" << csdrec.samplName << "ccdescs sz" << ccdescs.count();
 qDebug() << "::build_internals(); csdrec.samplName -- " << csdrec.samplName;
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
DbgLv(1) << "XpDa:b_i:     nlambda" << nlambda << "iwavl" << iwavl;
      }
   }  // END: triples loop
DbgLv(1) << "XpDa:b_i: END of triples loop";
DbgLv(1) << "XpDa:b_i:       kdarec" << datrecs.count()
 << "da0 dan" << datrecs[0] << datrecs[datrecs.count()-1];
DbgLv(1) << "XpDa:b_i:       kstgn " << stgnbrs.count()
 << "stg0 stgn" << stgnbrs[0] << stgnbrs[stgnbrs.count()-1];
DbgLv(1) << "XpDa:b_i:       kscnn " << scnnbrs.count()
 << "scn0 scnn" << scnnbrs[0] << scnnbrs[scnnbrs.count()-1];
qSort(datrecs);
qSort(stgnbrs);
qSort(scnnbrs);
DbgLv(1) << "XpDa:b_i:       kdarec" << datrecs.count()
 << "da0 dan" << datrecs[0] << datrecs[datrecs.count()-1];
DbgLv(1) << "XpDa:b_i:       kstgn " << stgnbrs.count()
 << "stg0 stgn" << stgnbrs[0] << stgnbrs[stgnbrs.count()-1];
DbgLv(1) << "XpDa:b_i:       kscnn " << scnnbrs.count()
 << "scn0 scnn" << scnnbrs[0] << scnnbrs[scnnbrs.count()-1];
DbgLv(1) << "XpDa:b_i:       rad0 radn" << a_radii[0] << a_radii[rkntl-1]
 << " nrad" << rkntl << a_radii.count();

   // Compute radius values

}

// Rebuild the internal variables and arrays (mainly, just scan information)
void US_XpnData::rebuild_internals( )
{

   int nscno     = scnnbrs.count();
   int nstgo     = stgnbrs.count();
   scnnbrs   .clear();
   stgnbrs   .clear();
   mnscnn        = 99999;
   mxscnn        = 0;
   int sdknt     = 0;
   sdknt         = ( runType == "RI" ) ? tAsdata.count() : sdknt;
   sdknt         = ( runType == "FI" ) ? tFsdata.count() : sdknt;
   sdknt         = ( runType == "IP" ) ? tIsdata.count() : sdknt;
   sdknt         = ( runType == "WI" ) ? tWsdata.count() : sdknt;

   // Analyze *ScanData table values
DbgLv(1) << "XpDa:rb_i:   csdrec count" << sdknt << "nscno" << nscno << "nstgo" << nstgo;
   for ( int ii = 0; ii < sdknt; ii++ )
   {  // Get cell/channel, triple, lambda lists
      set_scan_data( ii );

      int stage     = csdrec.stageNum;
      int scnnbr    = csdrec.scanSeqN;
      int celpos    = csdrec.cellPos;
      int iwavln    = csdrec.wavelen;
      QString schan = csdrec.radPath;
      QString scell = QString::number( celpos );
      QString swavl = QString::number( iwavln );

      QString cechn = scell + " / " + schan;
      QString tripl = cechn + " / " + swavl;
      QString tnode = scell + "." + schan + "." + swavl;

      QString darec = tnode + "."
                    + QString().sprintf( "%05i.%05i", stage, scnnbr );
DbgLv(1) << "XpDa:b_i: ii" << ii << "scnnbr" << scnnbr
 << "darec" << darec;

      // Update lists of used stages, scans, datarecs (trnode.stage.scan)

      int stgnx     = stgnbrs.indexOf( stage  );
      if ( stgnx < 0 )
      {
         stgnbrs << stage;               // Save unique StageNumber
      }

      int scnnx     = scnnbrs.indexOf( scnnbr );
      if ( scnnx < 0 )
      {
         scnnbrs << scnnbr;              // Save unique ScanNumber
      }

      int rdatx     = datrecs.indexOf( darec );
      if ( rdatx < 0 )
      {
         datrecs << darec;
      }

   }  // END: *ScanData records loop
DbgLv(1) << "XpDa:b_i:    scnnbrs count" << scnnbrs.count();

DbgLv(1) << "XpDa:b_i:       kdarec" << datrecs.count()
 << "da0 dan" << datrecs[0] << datrecs[datrecs.count()-1];
qSort(datrecs);
qSort(scnnbrs);
DbgLv(1) << "XpDa:b_i:       kdarec" << datrecs.count()
 << "da0 dan" << datrecs[0] << datrecs[datrecs.count()-1];
DbgLv(1) << "XpDa:b_i:       kstgn " << stgnbrs.count()
 << "scn0 scnn" << scnnbrs[0] << scnnbrs[scnnbrs.count()-1];

}

// Dump (print) information on tables and their fields
void US_XpnData::dump_tables()
{
   if ( !dbxpn.isOpen() )
   {
      qDebug() << "*ERROR* Optima DB not opened.";
      return;
   }

   QStringList sqtabs = dbxpn.tables( QSql::Tables );
   QSqlRecord qrec;
   QString schname( "AUC_schema" );

   for ( int ii = 0; ii < sqtabs.count(); ii++ )
   {
      QString tabname = QString( sqtabs[ ii ] ).section( ".", 1, 1 );
//DbgLv(1) << "XpDa:dmp:  ii tabname" << ii << tabname;
      QString qrytab  = "\"" + schname + "\".\"" + tabname + "\"";

      qrec            = dbxpn.record( qrytab );
      int cols        = qrec.count();
//DbgLv(1) << "XpDa:dmp:  record lsterr" << dbxpn.lastError().text();
      QStringList cnames;
      qDebug() << "Table Name :" << tabname << "  #Columns :" << cols;

      for ( int col = 0; col < cols; col++ )
      {
         QString fldname = qrec.fieldName( col );
         //cnames << fldname;                   // Get column names
         qDebug() << " " << col << ": Field :" << fldname;
      }

      //qDebug() << "  Columns:" << cnames;
   }
}

// Get column indexes and other properties of a db table
int US_XpnData::column_indexes( const QString tabname, QStringList& cnames,
                                QList< int >& cxs )
{
   const QString schname( "AUC_schema" );
   QString sqtab   = schname + "." + tabname;
   QString qrytab  = "\"" + schname + "\".\"" + tabname + "\"";
   QSqlRecord qrec = dbxpn.record( qrytab );
   int ncols       = qrec.count();
   cnames.clear();
   cxs   .clear();
   QStringList flds;

   if ( tabname == "AbsorbanceScanData" )
   {
      flds << "DataId" << "RunId" << "ExperimentStart" << "ExperimentTime"
           << "Temperature" << "RPM" << "OmegaSquaredT" << "StageNum"
           << "ScanSeqNum" << "SampleName" << "ScanTypeFlag" << "ModulePosition"
           << "CellPosition" << "Replicate" << "Wavelength" << "RadialPath"
           << "Count" << "Positions" << "V1_Array" << "Positions2"
           << "V2_Array";
   }

   else if ( tabname == "FluorescenceScanData" )
   {
      flds << "DataId" << "RunId" << "ExperimentStart" << "ExperimentTime"
           << "Temperature" << "RPM" << "OmegaSquaredT" << "StageNum"
           << "ScanSeqNum" << "SampleName" << "ScanTypeFlag" << "ModulePosition"
           << "CellPosition" << "Replicate" << "Wavelength" << "RadialPath"
           << "Count" << "Positions" << "V1_Array" << "Positions2"
           << "V2_Array";
   }

   else if ( tabname == "InterferenceScanData" )
   {
      flds << "DataId" << "RunId" << "ExperimentStart" << "ExperimentTime"
           << "Temperature" << "RPM" << "OmegaSquaredT" << "StageNum"
           << "ScanSeqNum" << "SampleName" << "ScanTypeFlag" << "ModulePosition"
           << "CellPosition" << "Replicate" << "Count" << "StartPosition"
           << "Resolution" << "Wavelength" << "Positions" << "Values";
   }

   else if ( tabname == "WavelengthScanData" )
   {
      flds << "DataId" << "RunId" << "ExperimentStart" << "ExperimentTime"
           << "Temperature" << "RPM" << "OmegaSquaredT" << "StageNum"
           << "ScanSeqNum" << "SampleName" << "ScanTypeFlag" << "ModulePosition"
           << "CellPosition" << "Replicate" << "ScanPosition" << "RadialPath"
           << "Count" << "Wavelengths" << "V1_Array" << "V2_Array";
   }

   else if ( tabname == "ExperimentRun" )
   {
      flds << "RunId" << "ExperimentId" << "RotorSN" << "DataPath"
           << "ExperimentStart" << "InstrumentSN" << "ScienceModule1SN" << "ScienceModule2SN"
           << "ScienceModule3SN" << "RunStatus" << "ExperimentDefinition" << "ExperimentName"
           << "ResearcherName" << "AbsorbanceScan" << "FluorescenceScan" << "InterferenceScan"
           << "WavelengthScan";
   }

   else if ( tabname == "ExperimentDefinition" )
   {
      flds << "ExperimentId" << "FugeRunProfileId" << "CellCount" << "Researcher"
           << "Name" << "Project" << "Comments" << "Used"
           << "UseCount";
   }

   else if ( tabname == "SystemStatusData" )
   {
      flds << "DataId" << "RunId" << "ExperimentStart" << "ExperimentTime"
           << "Temperature" << "RPM" << "OmegaSquaredT" << "StageNum";
   }

   else if ( tabname == "CentrifugeRunProfile" )
   {
      flds << "FugeRunProfileId" << "Temperature" << "StepBraking" << "BrakingRate"
           << "HoldTempAfterFinal" << "HoldSpeedAfterFinal" << "SetTempBeforeStart"
           << "SystemStatusInterval"
           << "Stages" << "StageRPM" << "StageStart" << "StageDuration"
           << "StageAccelRate" << "StageCellParameterIds" << "Used";
   }

   else
   {
      qDebug() << "*WARNING:  flds unspecified for tabname" << tabname;
      return 0;
   }

   int nflds       = flds.count();
   if ( ncols != nflds )
   {
      qDebug() << "*WARNING:  for" << tabname << "field,column counts:"
               << nflds << ncols;
   }

   for ( int col = 0; col < ncols; col++ )
   {
      QString fldname = qrec.fieldName( col );
      cnames << fldname;                   // Get column names
DbgLv(1) << "XpDa:cox:    col" << col << "fldname" << fldname;
   }

   for ( int fld = 0; fld < nflds; fld++ )
   {
      cxs << cnames.indexOf( flds[ fld ] );
   }
DbgLv(1) << "XpDa:cox: ncols" << ncols << "nflds" << nflds
 << "tabname" << tabname;

   return qMin( ncols, nflds );
}

// Update an entry in the Absorbance data table
void US_XpnData::update_ATable( QSqlQuery& sqry, QList< int >& cxs )
{
   // Construct an AData entry
   tbAsData asdrow;
   asdrow.dataId    = sqry.value( cxs[  0 ] ).toInt();
   asdrow.runId     = sqry.value( cxs[  1 ] ).toInt();
   asdrow.expstart  = sqry.value( cxs[  2 ] ).toDateTime();
   asdrow.exptime   = sqry.value( cxs[  3 ] ).toInt() + etimoff;
   asdrow.tempera   = sqry.value( cxs[  4 ] ).toDouble();
   asdrow.speed     = sqry.value( cxs[  5 ] ).toDouble();
   asdrow.omgSqT    = sqry.value( cxs[  6 ] ).toDouble();
   asdrow.stageNum  = sqry.value( cxs[  7 ] ).toInt();
   asdrow.scanSeqN  = sqry.value( cxs[  8 ] ).toInt();
   asdrow.samplName = sqry.value( cxs[  9 ] ).toString();
   asdrow.scanTypeF = sqry.value( cxs[ 10 ] ).toString();
   asdrow.modPos    = sqry.value( cxs[ 11 ] ).toInt();
   asdrow.cellPos   = sqry.value( cxs[ 12 ] ).toInt();
   asdrow.replic    = sqry.value( cxs[ 13 ] ).toInt();
   asdrow.wavelen   = sqry.value( cxs[ 14 ] ).toInt();
   asdrow.radPath   = sqry.value( cxs[ 15 ] ).toString();
   asdrow.count     = sqry.value( cxs[ 16 ] ).toInt();
   QString sPoss    = sqry.value( cxs[ 17 ] ).toString();
   QString sVals    = sqry.value( cxs[ 18 ] ).toString();
   parse_doubles( sPoss, asdrow.rads );
   parse_doubles( sVals, asdrow.vals );
//DbgLv(1) << "XpDa:updA: sPoss" << QString(sPoss).left(20) << "exptime scanSeqN"
// << asdrow.exptime << asdrow.exptime-etimoff << asdrow.scanSeqN;
   int mdx1         = -1;
   int mdx2         = -1;
   int strow        = tAsdata.count() - 1;
   int enrow        = qMax( ( strow - 100 ), -1 );

   for ( int ii = strow; ii > enrow; ii-- )
   {  // Look from end for any dataId match in already captured rows
      if ( tAsdata[ ii ].dataId == asdrow.dataId )
      {  // Match: get index(es)
         if ( mdx1 < 0 )
            mdx1       = ii;
         else
         {
            mdx2       = ii;
            break;
         }
      }
   }

   if ( asdrow.radPath == " "  ||  asdrow.radPath.isEmpty() )
   {  // Store both an 'A' and 'B' channel record
      asdrow.radPath   = "A";
      if ( mdx1 < 0 )
         tAsdata << asdrow;          // Update table with data entry
      else
         tAsdata[ mdx1 ] = asdrow;   // Replace table data entry
      asdrow.radPath   = "B";
      sPoss            = sqry.value( cxs[ 19 ] ).toString();
      sVals            = sqry.value( cxs[ 20 ] ).toString();
      parse_doubles( sPoss, asdrow.rads );
      parse_doubles( sVals, asdrow.vals );
      if ( mdx2 < 0 )
         tAsdata << asdrow;          // Update table with data entry
      else
         tAsdata[ mdx2 ] = asdrow;   // Replace table data entry
   }

   else
   {  // Store either an 'A' or a 'B' channel record
      if ( mdx1 < 0 )
         tAsdata << asdrow;          // Update table with data entry
      else
         tAsdata[ mdx1 ] = asdrow;   // Replace table data entry
   }
}

// Update an entry in the Fluorescence data table
void US_XpnData::update_FTable( QSqlQuery& sqry, QList< int >& cxs )
{
   // Construct an FData entry
   tbFsData fsdrow;
   fsdrow.dataId    = sqry.value( cxs[  0 ] ).toInt();
   fsdrow.runId     = sqry.value( cxs[  1 ] ).toInt();
   fsdrow.expstart  = sqry.value( cxs[  2 ] ).toDateTime();
   fsdrow.exptime   = sqry.value( cxs[  3 ] ).toInt() + etimoff;
   fsdrow.tempera   = sqry.value( cxs[  4 ] ).toDouble();
   fsdrow.speed     = sqry.value( cxs[  5 ] ).toDouble();
   fsdrow.omgSqT    = sqry.value( cxs[  6 ] ).toDouble();
   fsdrow.stageNum  = sqry.value( cxs[  7 ] ).toInt();
   fsdrow.scanSeqN  = sqry.value( cxs[  8 ] ).toInt();
   fsdrow.samplName = sqry.value( cxs[  9 ] ).toString();
   fsdrow.scanTypeF = sqry.value( cxs[ 10 ] ).toString();
   fsdrow.modPos    = sqry.value( cxs[ 11 ] ).toInt();
   fsdrow.cellPos   = sqry.value( cxs[ 12 ] ).toInt();
   fsdrow.replic    = sqry.value( cxs[ 13 ] ).toInt();
   fsdrow.wavelen   = sqry.value( cxs[ 14 ] ).toInt();
   fsdrow.radPath   = sqry.value( cxs[ 15 ] ).toString();
   fsdrow.count     = sqry.value( cxs[ 16 ] ).toInt();
   QString sPoss    = sqry.value( cxs[ 17 ] ).toString();
   QString sVals    = sqry.value( cxs[ 18 ] ).toString();
   parse_doubles( sPoss, fsdrow.rads );
   parse_doubles( sVals, fsdrow.vals );
   int mdx1         = -1;
   int mdx2         = -1;
   int strow        = tFsdata.count() - 1;
   int enrow        = qMax( ( strow - 100 ), -1 );

   for ( int ii = strow; ii > enrow; ii-- )
   {  // Look from end for any dataId match in already captured rows
      if ( tFsdata[ ii ].dataId == fsdrow.dataId )
      {  // Match: get index(es)
         if ( mdx1 < 0 )
            mdx1       = ii;
         else
         {
            mdx2       = ii;
            break;
         }
      }
   }

   if ( fsdrow.radPath == " "  ||  fsdrow.radPath.isEmpty() )
   {  // Store both an 'A' and 'B' channel record
      fsdrow.radPath   = "A";
      if ( mdx1 < 0 )
         tFsdata << fsdrow;          // Update table with data entry
      else
         tFsdata[ mdx1 ] = fsdrow;   // Replace table data entry
      fsdrow.radPath   = "B";
      sPoss            = sqry.value( cxs[ 19 ] ).toString();
      sVals            = sqry.value( cxs[ 20 ] ).toString();
      parse_doubles( sPoss, fsdrow.rads );
      parse_doubles( sVals, fsdrow.vals );
      if ( mdx2 < 0 )
         tFsdata << fsdrow;          // Update table with data entry
      else
         tFsdata[ mdx2 ] = fsdrow;   // Replace table data entry
   }

   else
   {  // Store either an 'A' or a 'B' channel record
      if ( mdx1 < 0 )
         tFsdata << fsdrow;          // Update table with data entry
      else
         tFsdata[ mdx1 ] = fsdrow;   // Replace table data entry
   }
}

// Update an entry in the Interference data table
void US_XpnData::update_ITable( QSqlQuery& sqry, QList< int >& cxs )
{
   // Construct an IData entry
   tbIsData isdrow;
   isdrow.dataId    = sqry.value( cxs[  0 ] ).toInt();
   isdrow.runId     = sqry.value( cxs[  1 ] ).toInt();
   isdrow.expstart  = sqry.value( cxs[  2 ] ).toDateTime();
   isdrow.exptime   = sqry.value( cxs[  3 ] ).toInt() + etimoff;
   isdrow.tempera   = sqry.value( cxs[  4 ] ).toDouble();
   isdrow.speed     = sqry.value( cxs[  5 ] ).toDouble();
   isdrow.omgSqT    = sqry.value( cxs[  6 ] ).toDouble();
   isdrow.stageNum  = sqry.value( cxs[  7 ] ).toInt();
   isdrow.scanSeqN  = sqry.value( cxs[  8 ] ).toInt();
   isdrow.samplName = sqry.value( cxs[  9 ] ).toString();
   isdrow.scanTypeF = sqry.value( cxs[ 10 ] ).toString();
   isdrow.modPos    = sqry.value( cxs[ 11 ] ).toInt();
   isdrow.cellPos   = sqry.value( cxs[ 12 ] ).toInt();
   isdrow.replic    = sqry.value( cxs[ 13 ] ).toInt();
   isdrow.count     = sqry.value( cxs[ 14 ] ).toInt();
   isdrow.startPos  = sqry.value( cxs[ 15 ] ).toDouble();
   isdrow.resolu    = sqry.value( cxs[ 16 ] ).toDouble();
   isdrow.wavelen   = sqry.value( cxs[ 17 ] ).toInt();
   QString sPoss    = sqry.value( cxs[ 18 ] ).toString();
   QString sVals    = sqry.value( cxs[ 19 ] ).toString();
   parse_doubles( sPoss, isdrow.rads );
   parse_doubles( sVals, isdrow.vals );
   int mdx1         = -1;
   int strow        = tIsdata.count() - 1;
   int enrow        = qMax( ( strow - 100 ), -1 );

   for ( int ii = strow; ii > enrow; ii-- )
   {  // Look from end for any dataId match in already captured rows
      if ( tIsdata[ ii ].dataId == isdrow.dataId )
      {  // Match: get index(es)
         if ( mdx1 < 0 )
         {
            mdx1       = ii;
            break;
         }
      }
   }

   if ( mdx1 < 0 )
      tIsdata << isdrow;          // Update table with data entry
   else
      tIsdata[ mdx1 ] = isdrow;   // Replace table data entry
}

// Update an entry in the Wavelength data table
void US_XpnData::update_WTable( QSqlQuery& sqry, QList< int >& cxs )
{
   // Construct a WData entry
   tbWsData wsdrow;
   wsdrow.dataId    = sqry.value( cxs[  0 ] ).toInt();
   wsdrow.runId     = sqry.value( cxs[  1 ] ).toInt();
   wsdrow.expstart  = sqry.value( cxs[  2 ] ).toDateTime();
   wsdrow.exptime   = sqry.value( cxs[  3 ] ).toInt() + etimoff;
   wsdrow.tempera   = sqry.value( cxs[  4 ] ).toDouble();
   wsdrow.speed     = sqry.value( cxs[  5 ] ).toDouble();
   wsdrow.omgSqT    = sqry.value( cxs[  6 ] ).toDouble();
   wsdrow.stageNum  = sqry.value( cxs[  7 ] ).toInt();
   wsdrow.scanSeqN  = sqry.value( cxs[  8 ] ).toInt();
   wsdrow.samplName = sqry.value( cxs[  9 ] ).toString();
   wsdrow.scanTypeF = sqry.value( cxs[ 10 ] ).toString();
   wsdrow.modPos    = sqry.value( cxs[ 11 ] ).toInt();
   wsdrow.cellPos   = sqry.value( cxs[ 12 ] ).toInt();
   wsdrow.replic    = sqry.value( cxs[ 13 ] ).toInt();
   wsdrow.scanPos   = sqry.value( cxs[ 14 ] ).toInt();
   wsdrow.radPath   = sqry.value( cxs[ 15 ] ).toString();
   wsdrow.count     = sqry.value( cxs[ 16 ] ).toInt();
   QString sPoss    = sqry.value( cxs[ 17 ] ).toString();
   QString sVals    = sqry.value( cxs[ 18 ] ).toString();
   parse_doubles( sPoss, wsdrow.wvls );
   parse_doubles( sVals, wsdrow.vals );
   int mdx1         = -1;
   int mdx2         = -1;
   int strow        = tWsdata.count() - 1;
   int enrow        = qMax( ( strow - 100 ), -1 );

   for ( int ii = strow; ii > enrow; ii-- )
   {  // Look from end for any dataId match in already captured rows
      if ( tWsdata[ ii ].dataId == wsdrow.dataId )
      {  // Match: get index(es)
         if ( mdx1 < 0 )
            mdx1       = ii;
         else
         {
            mdx2       = ii;
            break;
         }
      }
   }
   if ( wsdrow.radPath == " "  ||  wsdrow.radPath.isEmpty() )
   {  // Store both an 'A' and 'B' channel record
      wsdrow.radPath   = "A";
      if ( mdx1 < 0 )
         tWsdata << wsdrow;          // Update table with data entry
      else
         tWsdata[ mdx1 ] = wsdrow;   // Replace table data entry
      wsdrow.radPath   = "B";
      sVals            = sqry.value( cxs[ 19 ] ).toString();
      parse_doubles( sVals, wsdrow.vals );
      if ( mdx2 < 0 )
         tWsdata << wsdrow;          // Update table with data entry
      else
         tWsdata[ mdx2 ] = wsdrow;   // Replace table data entry
   }

   else
   {  // Store either an 'A' or a 'B' channel record
      if ( mdx1 < 0 )
         tWsdata << wsdrow;          // Update table with data entry
      else
         tWsdata[ mdx1 ] = wsdrow;   // Replace table data entry
   }
}

