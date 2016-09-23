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
   dbname       = QString( "AUC_DATA_DB" );
   dbhost       = QString( "bcf.uthscsa.edu" );
   dbport       = 5432;
   sctype       = 1;
   runType      = "RI";
DbgLv(0) << "XpDa: dbg_level" << dbg_level;
}

// Connect to a selected database server with XPN data
bool US_XpnData::connect_data( QString& adbname,
                               QString& xpnhost, const int xpnport )
{
   bool status   = true;
   dbname        = ( adbname.isEmpty() ) ? dbname : adbname;
   dbhost        = ( xpnhost.isEmpty() ) ? dbhost : xpnhost;
   dbport        = ( xpnport <= 0 )      ? 5432   : xpnport;
   is_absorb     = false;
   is_raw        = true;
   QString dbuser( "auc_admin" );
   QString dbpasw( "auc_admin" );

   dbxpn           = QSqlDatabase::addDatabase( "QPSQL", "XpnData" );
   dbxpn.setDatabaseName( "XpnData" );
   dbxpn.setHostName    ( dbhost );
   dbxpn.setPort        ( dbport );
   dbxpn.setDatabaseName( dbname  );
   dbxpn.setUserName    ( dbuser  );
   dbxpn.setPassword    ( dbpasw );

   if ( dbxpn.open() )
   {
      qDebug() << "Open Success for db" << dbname << dbhost << dbport;
   }
   else
   {
      qDebug() << "Open Failure for db" << dbname << dbhost << dbport;
      return false;
   }

   QStringList qdrvrs = dbxpn.drivers();
DbgLv(1) << "XpDa:scn: drivers" << qdrvrs;
   QStringList sqtabs = dbxpn.tables( QSql::Tables );
DbgLv(1) << "XpDa:scn: sqtabs" << sqtabs;

   if ( qdrvrs.contains( "QPSQL" )  &&
        sqtabs.contains( "\"AUC_schema\".\"ExperimentRun\"" ) )
   {
      status        = true;
   }
   else
   {
      status        = false;
   }
   return status;
}

// Scan an XPN database for run information
int US_XpnData::scan_runs( QStringList& runInfo )
{
   int nruns     = 0;

   if ( !dbxpn.isOpen() )
      return nruns;

   QSqlQuery  sqry;
   QSqlRecord qrec;
   QString schname( "AUC_schema" );
   QString tabname( "ExperimentRun" );
   QString sqtab   = schname + "." + tabname;
   QString qrytab  = "\"" + schname + "\".\"" + tabname + "\"";

   qrec            = dbxpn.record( qrytab );
   int cols        = qrec.count();
DbgLv(1) << "XpDa:scn:  record lsterr" << dbxpn.lastError().text();
   QStringList cnames;

   for ( int col = 0; col < cols; col++ )
   {
      QString fldname = qrec.fieldName( col );
      cnames << fldname;                   // Get column names
DbgLv(1) << "XpDa:scn:    col" << col << "fldname" << fldname;
   }

   int jrunid    = cnames.indexOf( "RunId"                );
   int jexpid    = cnames.indexOf( "ExperimentId"         );
   int jrotsn    = cnames.indexOf( "RotorSN"              );
   int jdatpa    = cnames.indexOf( "DataPath"             );
   int jexpst    = cnames.indexOf( "ExperimentStart"      );
   int jinssn    = cnames.indexOf( "InstrumentSN"         );
   int jsmo1s    = cnames.indexOf( "ScienceModule1SN"     );
   int jsmo2s    = cnames.indexOf( "ScienceModule2SN"     );
   int jsmo3s    = cnames.indexOf( "ScienceModule3SN"     );
   int jrunst    = cnames.indexOf( "RunStatus"            );
   int jexpde    = cnames.indexOf( "ExperimentDefinition" );
   int jexpnm    = cnames.indexOf( "ExperimentName"       );
   int jresnm    = cnames.indexOf( "ResearcherName"       );
   int jascnf    = cnames.indexOf( "AbsorbanceScan"       );
   int jfscnf    = cnames.indexOf( "FluorescenceScan"     );
   int jiscnf    = cnames.indexOf( "InterferenceScan"     );
   int jwscnf    = cnames.indexOf( "WavelengthScan"       );

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

   while( sqry.next() )
   {  // Loop to pick up values from rows of the table
      row++;

      tbExpRun exprow;
      exprow.runId     = sqry.value( jrunid ).toInt();
//DbgLv(1) << "XpDa:    jrunid       " << jrunid;
      exprow.expId     = sqry.value( jexpid ).toInt();
      exprow.rotorSN   = sqry.value( jrotsn ).toInt();
      exprow.datapath  = sqry.value( jdatpa ).toString();
      exprow.expstart  = sqry.value( jexpst ).toDateTime();
//DbgLv(1) << "XpDa:    jexpid jrotsn" << jexpid << jrotsn;
//DbgLv(1) << "XpDa:    jdatpa jexpst" << jdatpa << jexpst;
      exprow.instrSN   = sqry.value( jinssn ).toString();
      exprow.scimo1sn  = sqry.value( jsmo1s ).toString();
      exprow.scimo2sn  = sqry.value( jsmo2s ).toString();
      exprow.scimo3sn  = sqry.value( jsmo3s ).toString();
//DbgLv(1) << "XpDa:    jinssn jsmo1s" << jinssn << jsmo1s;
//DbgLv(1) << "XpDa:    jsmo2s jsmo3s" << jsmo2s << jsmo3s;
      exprow.runstat   = sqry.value( jrunst ).toInt();
      exprow.expdef    = sqry.value( jexpde ).toString();
      exprow.expname   = sqry.value( jexpnm ).toString();
      exprow.resname   = sqry.value( jresnm ).toString();
//DbgLv(1) << "XpDa:    jrunst jexpde" << jrunst << jexpde;
//DbgLv(1) << "XpDa:    jexpnm jresnm" << jexpnm << jresnm;
      exprow.abscnf    = sqry.value( jascnf ).toBool();
      exprow.flscnf    = sqry.value( jfscnf ).toBool();
      exprow.inscnf    = sqry.value( jiscnf ).toBool();
      exprow.wlscnf    = sqry.value( jwscnf ).toBool();
//DbgLv(1) << "XpDa:    jascnf jfscnf" << jascnf << jfscnf;
//DbgLv(1) << "XpDa:    jiscnf jwscnf" << jiscnf << jwscnf;

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
                      + delim + QString( exprow.wlscnf ? "1" : "0" );
      runInfo << inforow;
DbgLv(1) << "XpDa: inforow" << inforow;
   }

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

   if ( ! dbxpn.open() )
   {
      return false;
   }

   tAsdata.clear();     // Clear table value vectors
   tFsdata.clear();
   tIsdata.clear();
   tWsdata.clear();
   bool ascnf    = scanMask & 1;
   bool fscnf    = scanMask & 2;
   bool iscnf    = scanMask & 4;
   bool wscnf    = scanMask & 8;

   int arows     = 0;
   int frows     = 0;
   int irows     = 0;
   int wrows     = 0;

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
DbgLv(1) << "XpDa:i_d: arows frows irows wrows"
   << arows << frows << irows << wrows;

   return status;
}

// Query and save data for a [AIFW]ScanData table
int US_XpnData::scan_xpndata( const int runId, const QChar scantype )
{
   QSqlQuery  sqry;
   QSqlRecord qrec;
   QString schname( "AUC_schema" );
   QString tabname( "AbsorbanceScanData" );
   tabname         = ( scantype == 'F' ) ? "FluorescenceScanData" : tabname;
   tabname         = ( scantype == 'I' ) ? "InterferenceScanData" : tabname;
   tabname         = ( scantype == 'W' ) ? "WavelengthScanData"   : tabname;
   QString sqtab   = schname + "." + tabname;
   QString qrytab  = "\"" + schname + "\".\"" + tabname + "\"";
   QString sRunId  = QString::number( runId );

   int count       = 0;
   int rows        = 0;

   // Get count of rows matching runId
   sqtab           = schname + "." + tabname;
   qrytab          = "\"" + schname + "\".\"" + tabname + "\"";
   QString qrytext = "SELECT count(*) from " + qrytab
                   + " WHERE \"RunId\"=" + sRunId + ";";
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
                     + " WHERE \"RunId\"=" + sRunId + ";";
   sqry            = dbxpn.exec( qrytext );
   qrec            = dbxpn.record( qrytab );
   int cols        = qrec.count();
   QStringList cnames;

   for ( int col = 0; col < cols; col++ )
   {
      QString fldname = qrec.fieldName( col );
      cnames << qrec.fieldName( col );     // Get column names
   }
DbgLv(1) << "XpDa:s_x:  cols" << cols << "cnames" << cnames[0] << "..."
 << cnames[cols-1];

   int jdatid    = cnames.indexOf( "DataId"               );
   int jrunid    = cnames.indexOf( "RunId"                );
   int jexpst    = cnames.indexOf( "ExperimentStart"      );
   int jexptm    = cnames.indexOf( "ExperimentTime"       );
   int jtempe    = cnames.indexOf( "Temperature"          );
   int jspeed    = cnames.indexOf( "RPM"                  );
   int jomgsq    = cnames.indexOf( "OmegaSquaredT"        );
   int jstage    = cnames.indexOf( "StageNum"             );
   int jscanx    = cnames.indexOf( "ScanSeqNum"           );
   int jsname    = cnames.indexOf( "SampleName"           );
   int jsctyp    = cnames.indexOf( "ScanTypeFlag"         );
   int jmodpo    = cnames.indexOf( "ModulePosition"       );
   int jcelpo    = cnames.indexOf( "CellPosition"         );
   int jrepli    = cnames.indexOf( "Replicate"            );
   int jwavel    = cnames.indexOf( "Wavelength"           );
   int jradpa    = cnames.indexOf( "RadialPath"           );
   int jcount    = cnames.indexOf( "Count"                );
   int jposis    = cnames.indexOf( "Positions"            );
   int jv1ary    = cnames.indexOf( "V1_Array"             );
   int jpos2s    = cnames.indexOf( "Positions2"           );
   int jv2ary    = cnames.indexOf( "V2_Array"             );
   int jstrpo    = cnames.indexOf( "StartPosition"        );
   int jresol    = cnames.indexOf( "Resolution"           );
   int jvalus    = cnames.indexOf( "Values"               );
   int jscnpo    = cnames.indexOf( "ScanPosition"         );
   int jwavls    = cnames.indexOf( "Wavelengths"          );
   int isctyp    = ( scantype == 'A' ) ? 1 : 0;
   isctyp        = ( scantype == 'F' ) ? 2 : isctyp;
   isctyp        = ( scantype == 'I' ) ? 3 : isctyp;
   isctyp        = ( scantype == 'W' ) ? 4 : isctyp;
DbgLv(1) << "XpDa:s_x:  isctyp scantype" << isctyp << scantype;

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
            tbAsData asdrow;
            asdrow.runId     = sqry.value( jrunid ).toInt();
            asdrow.dataId    = sqry.value( jdatid ).toInt();
            asdrow.exptime   = sqry.value( jexptm ).toInt();
            asdrow.stageNum  = sqry.value( jstage ).toInt();
            asdrow.scanSeqN  = sqry.value( jscanx ).toInt();
            asdrow.modPos    = sqry.value( jmodpo ).toInt();
            asdrow.cellPos   = sqry.value( jcelpo ).toInt();
            asdrow.replic    = sqry.value( jrepli ).toInt();
            asdrow.wavelen   = sqry.value( jwavel ).toInt();
            asdrow.tempera   = sqry.value( jtempe ).toDouble();
            asdrow.speed     = sqry.value( jspeed ).toDouble();
            asdrow.omgSqT    = sqry.value( jomgsq ).toDouble();
            asdrow.count     = sqry.value( jcount ).toInt();
            asdrow.expstart  = sqry.value( jexpst ).toDateTime();
            asdrow.samplName = sqry.value( jsname ).toString();
            asdrow.scanTypeF = sqry.value( jsctyp ).toString();
            asdrow.radPath   = sqry.value( jradpa ).toString();
            QString sPoss    = sqry.value( jposis ).toString();
            QString sVals    = sqry.value( jv1ary ).toString();
            int pcount       = parse_doubles( sPoss, asdrow.rads );
            int vcount       = parse_doubles( sVals, asdrow.vals );
            int pcount2      = 0;
            int vcount2      = 0;
QString rpsave = asdrow.radPath;
            if ( asdrow.radPath == " "  ||  asdrow.radPath.isEmpty() )
            {  // Store both an 'A' and 'B' channel record
               asdrow.radPath   = "A";
               tAsdata << asdrow;
               asdrow.radPath   = "B";
               sPoss            = sqry.value( jpos2s ).toString();
               sVals            = sqry.value( jv2ary ).toString();
               pcount2          = parse_doubles( sPoss, asdrow.rads );
               vcount2          = parse_doubles( sVals, asdrow.vals );
               tAsdata << asdrow;
            }
            else
            {  // Store either an 'A' or a 'B' channel record
               tAsdata << asdrow;
            }
//*DEBUG*
if(rows<9 || (rows+9)>count) {
DbgLv(1) << "XpDa:scn:    row" << rows << "run" << asdrow.runId
 << "dat" << asdrow.dataId << "pc vc c pc2 vc2"
 << pcount << vcount << asdrow.count << pcount2 << vcount2 << "rp" << rpsave;
DbgLv(1) << "XpDa:scn:     rads0 rads1 vals0 vals1"
 << asdrow.rads[0] << asdrow.rads[1] << asdrow.vals[0] << asdrow.vals[1]
 << QString(sPoss).left(20) << QString(sVals).left(20);
}
//*DEBUG*

            break;
         }
         case 2:
         {
            tbFsData fsdrow;
               tFsdata << fsdrow;
            break;
         }
         case 3:
         {
            tbIsData isdrow;
//*DEBUG*
if(rows<9 || (rows+9)>count) {
DbgLv(1) << "XpDa:scn:      jstrpo jresol jposis jvalus"
 << jstrpo << jresol << jposis << jvalus;
DbgLv(1) << "XpDa:scn:       cnames" << cnames;
}
//*DEBUG*
            isdrow.runId     = sqry.value( jrunid ).toInt();
            isdrow.dataId    = sqry.value( jdatid ).toInt();
            isdrow.exptime   = sqry.value( jexptm ).toInt();
            isdrow.stageNum  = sqry.value( jstage ).toInt();
            isdrow.scanSeqN  = sqry.value( jscanx ).toInt();
            isdrow.modPos    = sqry.value( jmodpo ).toInt();
            isdrow.cellPos   = sqry.value( jcelpo ).toInt();
            isdrow.replic    = sqry.value( jrepli ).toInt();
            isdrow.count     = sqry.value( jcount ).toInt();
            isdrow.startPos  = sqry.value( jstrpo ).toDouble();
            isdrow.resolu    = sqry.value( jresol ).toDouble();
            QString sPoss    = sqry.value( jposis ).toString();
            QString sVals    = sqry.value( jvalus ).toString();
            int pcount       = parse_doubles( sPoss, isdrow.rads );
            int vcount       = parse_doubles( sVals, isdrow.vals );
            int pcount2      = 0;
            int vcount2      = 0;
               tIsdata << isdrow;
//*DEBUG*
if(rows<9 || (rows+9)>count) {
DbgLv(1) << "XpDa:scn:      jstrpo jresol jposis jvalus"
 << jstrpo << jresol << jposis << jvalus;
DbgLv(1) << "XpDa:scn:    row" << rows << "run" << isdrow.runId
 << "dat" << isdrow.dataId << "pc vc c pc2 vc2"
 << pcount << vcount << isdrow.count << pcount2 << vcount2
 << "modpos celpos" << isdrow.modPos << isdrow.cellPos;
DbgLv(1) << "XpDa:scn:     rads0 rads1 vals0 vals1"
 << isdrow.rads[0] << isdrow.rads[1] << isdrow.vals[0] << isdrow.vals[1];
}
//*DEBUG*
            break;
         }
         case 4:
         {
            tbWsData wsdrow;
            wsdrow.scanPos   = sqry.value( jscnpo ).toDouble();
            QString sPoss    = sqry.value( jwavls ).toString();
            QString sVals    = sqry.value( jv1ary ).toString();
            int pcount       = parse_doubles( sPoss, wsdrow.wvls );
            int vcount       = parse_doubles( sVals, wsdrow.vals );
               tWsdata << wsdrow;
//*DEBUG*
if(rows<9 || (rows+9)>count) {
DbgLv(1) << "XpDa:scn:    row" << rows << "run dat pc vc c"
 << wsdrow.runId << wsdrow.dataId << pcount << vcount << wsdrow.count;
}
//*DEBUG*
            break;
         }
      }
   }

DbgLv(1) << "XpDa:scn: rows count" << rows << count << "out recs"
 << tAsdata.count();
   return rows;
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
                           QStringList ifpaths )
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
               celchn           = triple.section( " / ", 0, 1 );
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

// Private slot to clear arrays
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

// Populate the list of RawData objects from raw input XPN data
int US_XpnData::build_rawData( QVector< US_DataIO::RawData >& allData )
{
DbgLv(1) << "BldRawD IN";

   // First build the internal arrays and variables
   build_internals();

   const int    low_memApc=20;
   npoint          = a_radii.count();

   allData.clear();

   // Set up the interpolated byte array (all one bits)
   int    nbytei   = ( npoint + 7 ) / 8;
   QByteArray interpo( nbytei, '\255' );

   // Build a raw data set for each triple
   char   dtype0   = 'R';
   char   dtype1   = 'I';
   int    ccx      = 0;
   int    wvx      = 0;
   int    scnnbr   = 0;
   nscnn           = scnnbrs.count();
   int    stgnbr   = 0;
   nstgn           = stgnbrs.count();
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
      int ndscan        = 0;
      int rdx           = 0;
      rdata.description = ccdescs.at( ccx );
      QString triple    = triples[ trx ].replace( " / ", "/" );
      QString trnode    = trnodes[ trx ];

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

   QString stat_text = tr( "All %1 raw AUCs have been build." ).arg( ntriple );
   emit status_text( stat_text );
  
   nscan             = mxscnn;

DbgLv(1) << "BldRawD  DONE ntriple" << ntriple << "mxscnn" << mxscnn
 << "mnscnn" << mnscnn;
   return ntriple;
}

// Export RawData to openAUC (.auc) and TMST (.tmst) files
int US_XpnData::export_auc( QVector< US_DataIO::RawData >& allData )
{
   int nfiles        = 0;
   if ( ! is_raw )
      return nfiles;

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

   for ( int ii = 0; ii < ntrips; ii++ )
   {  // Create a file for each triple
      US_DataIO::RawData* rdata = &allData[ ii ];
      QString trnode    = QString::number( rdata->cell ) + "." +
                          QString( rdata->channel ) + "." +
                          QString().sprintf( "%03d",
                                qRound( rdata->scanData[ 0 ].wavelength ) );
      QString fname     = fbase + trnode + ".auc";
      QString fpath     = cur_dir + fname;

      US_DataIO::writeRawData( fpath, *rdata );

      nfiles++;
DbgLv(1) << "expA:  nf" << nfiles << "fname" << fname;

      // Find index to first dataset with maximum scans
      int kscans        = rdata->scanCount();

      if ( kscans > mxscans )
      {
         mxscans           = kscans;
         iiuse             = ii;
      }
   }

   // Create a speed step vector
   QVector< US_SimulationParameters::SpeedProfile > speedsteps;
   US_DataIO::RawData* udata = &allData[ iiuse ];

   US_SimulationParameters::computeSpeedSteps( &udata->scanData, speedsteps );

   // Output time state files

   QStringList fkeys;
   QStringList ffmts;
   fkeys << "Scan"     << "Stage"   << "Time"        << "RawSpeed"
         << "SetSpeed" << "Omega2T" << "Temperature";
   ffmts << "I2"       << "I2"      << "F4"          << "I4"
         << "I4"       << "F4"      << "F4";
   US_TimeState tsobj;

   QString tspath    = cur_dir + runID + ".time_state.tmst";
   int ntimes        = udata->scanCount();
DbgLv(1) << "expA: ntimes" << ntimes << "tspath" << tspath;

   tsobj.open_write_data( tspath );      // Initialize TMST creation

   tsobj.set_keys( fkeys, ffmts );       // Define keys and formats

   for ( int ii = 0;  ii < ntimes; ii++ )
   {  // Create a record for each scan
      US_DataIO::Scan* uscan = &udata->scanData[ ii ];
      int scannbr       = ii + 1;           // Scan number
      double rawSpeed   = uscan->rpm;
      double tempera    = uscan->temperature;
      double omega2t    = uscan->omega2t;
      double time       = uscan->seconds;
      double setSpeed   = qRound( rawSpeed * 0.01 ) * 100.0;

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
      int irSpeed       = (int)qRound( rawSpeed );
      int isSpeed       = (int)qRound( setSpeed );
DbgLv(1) << "expA:   ii" << ii << "scan" << scannbr << "stage" << istagen
 << "speed" << irSpeed << isSpeed << "time" << time;

      // Set values for this scan
      tsobj.set_value( fkeys[ 0 ], scannbr );       // Scan
      tsobj.set_value( fkeys[ 1 ], istagen );       // Stage (speed step)
      tsobj.set_value( fkeys[ 2 ], time    );       // Time in seconds
      tsobj.set_value( fkeys[ 3 ], irSpeed );       // Raw speed
      tsobj.set_value( fkeys[ 4 ], isSpeed );       // Set (stage) speed
      tsobj.set_value( fkeys[ 5 ], omega2t );       // Omega-Squared-T
      tsobj.set_value( fkeys[ 6 ], tempera );       // Temperature

      // Write the scan record
      tsobj.flush_record();
   }

   // Complete write of TMST file and defining XML
   if ( tsobj.close_write_data() == 0 )
   {
      tsobj.write_defs( 0.0, "XPN" );
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
}

// Return a scan data index within the ScanData vector
int US_XpnData::scan_data_index( const QString trnode,
                                 const int mstage, const int mscann )
{
   int datx       = -1;          // Default:  scan record not found
   int sdknt      = 0;
   sdknt          = ( runType == "RI" ) ? tAsdata.count() : sdknt;
   sdknt          = ( runType == "FI" ) ? tFsdata.count() : sdknt;
   sdknt          = ( runType == "IP" ) ? tIsdata.count() : sdknt;
   sdknt          = ( runType == "WI" ) ? tWsdata.count() : sdknt;
   int     mcell  = QString( trnode ).section( ".", 0, 0 ).toInt();
   QString mchan  = QString( trnode ).section( ".", 1, 1 ).simplified();
   int     mwavl  = QString( trnode ).section( ".", 2, 2 ).toInt();

   for ( int ii = 0; ii < sdknt; ii++ )
   {
      set_scan_data( ii );

      if ( ( csdrec.cellPos  != mcell  )  ||
           ( csdrec.radPath  != mchan  )  ||
           ( csdrec.wavelen  != mwavl  )  ||
           ( csdrec.stageNum != mstage )  ||
           ( csdrec.scanSeqN != mscann ) )
         continue;
      
      datx           = ii;
      break;
   }
DbgLv(1) << "XpDa:scdX: trnode mstage mscann" << trnode << mstage << mscann
 << "datx" << datx;

   return datx;
}

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
      return 0;
   }

   set_scan_data( datx );

   interp_rvalues( *csdrec.rads, *csdrec.vals, a_radii, rvalues );

   return npoint;
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
         double rad1    = rads[ jj - 1];
         double rad2    = rads[ jj ];
         valout         = vals[ jj ];

         if ( radv < rad2  ||  jj == lstrx )
         {
            double drad    = rad2 - rad1;
            double val1    = vals[ jj - 1 ];
            double dval    = valout - val1;
            valout         = val1 + ( radv - rad1 ) * dval / drad;
            jjs            = jj;
            break;
         }

         else if ( radv == rad2 )
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
DbgLv(0) << "XpDa:i_r:  rai0 rao0 raim raom rain raon"
 << rads[0] << rrads[0] << rads[mi] << rrads[mo] << rads[ni] << rrads[no];
DbgLv(0) << "XpDa:i_r:   vli0 vlo0 vlim vlom vlin vlon"
 << vals[0] << rvals[0] << vals[mi] << rvals[mo] << vals[ni] << rvals[no];
}

// Compose text with detailed information about the data
QString US_XpnData::runDetails( void )
{
   // Create report string
   QString dbfn   = dbfile.section( "/", -1, -1 );

   // Create report string

   QString msg = tr( "XPN Data Statistics for RunID \"%1\",\n" )
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
   msg += tr( "    Scan Count (max.):        %1\n" ).arg( nscan );
   msg += tr( "    Wavelengths Count:        %1\n" ).arg( nlambda );

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

   return msg;
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
 << schan << cechn << "darec" << darec;

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
DbgLv(1) << "XpDa:b_i:       kstgn " << stgnbrs.count()
 << "scn0 scnn" << scnnbrs[0] << scnnbrs[scnnbrs.count()-1];
qSort(datrecs);
qSort(stgnbrs);
qSort(scnnbrs);
DbgLv(1) << "XpDa:b_i:       kdarec" << datrecs.count()
 << "da0 dan" << datrecs[0] << datrecs[datrecs.count()-1];
DbgLv(1) << "XpDa:b_i:       kstgn " << stgnbrs.count()
 << "stg0 stgn" << stgnbrs[0] << stgnbrs[stgnbrs.count()-1];
DbgLv(1) << "XpDa:b_i:       kstgn " << stgnbrs.count()
 << "scn0 scnn" << scnnbrs[0] << scnnbrs[scnnbrs.count()-1];
DbgLv(1) << "XpDa:b_i:       rad0 radn" << a_radii[0] << a_radii[rkntl-1]
 << " nrad" << rkntl << a_radii.count();

   // Compute radius values

}

