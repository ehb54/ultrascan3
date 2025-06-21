//! \file us_load_run_noise.cpp

#include <QtGui>

#include "us_load_run_noise.h"
#include "us_license_t.h"
#include "us_license.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_passwd.h"
#include "us_util.h"
#include "us_investigator.h"


// Constructor
US_LoadRunNoise::US_LoadRunNoise( QObject* parent ) : QObject( parent )
{
   dbg_level   = US_Settings::us_debug();
   dbP         = NULL;
   us_automode = false;
}

// Alt. Constructor
US_LoadRunNoise::US_LoadRunNoise( QMap<QString, QString>& protocol_details, QObject* parent ) : QObject( parent )
{
   dbg_level   = US_Settings::us_debug();
   dbP         = NULL;
   this->protocol_details = protocol_details;
   us_automode = true;
}

// Determine if edit/model related noise available and build lists
int US_LoadRunNoise::count_noise( const bool dbload, const QString runID,
				  QStringList* edIDsP, QStringList* noiInfP, int* neditsP )
{
   int nrnois  = 0;       // Number of runID-data-related noises
   edGUIDs .clear();
   noiInfos.clear();
DbgLv(1) << "LRNoi:cn: dbload" << dbload << "runID" << runID;

   if ( dbload )
   {
      nrnois      = scan_db  ( runID );
   }

   else
   {
      nrnois      = scan_disk( runID );
   }
DbgLv(1) << "LRNoi:cn:  nrnois" << nrnois << "nedits" << edGUIDs.count();

   if ( nrnois < 1 )
      return nrnois;

   if ( edIDsP != NULL )
      *edIDsP     = edGUIDs;

   if ( noiInfP != NULL )
      *noiInfP    = noiInfos;

   if ( neditsP != NULL )
      *neditsP    = edGUIDs.count();

   return nrnois;
}

// Return noises for a given edit GUID
int US_LoadRunNoise::get_noises( const QString edGUID,
      US_Noise& ti_noise, US_Noise& ri_noise )
{
   int noisflag       = 0;
   int edx            = edGUIDs.indexOf( edGUID );

   if ( edx < 0 )
   {  // Edit not found, so no noise
      ti_noise           = US_Noise();
      ri_noise           = US_Noise();
   }

   else
   {  // Get noise(s), based on noise information for given edit
      QString ninfo      = noiInfos[ edx ];
      QString dlm        = ninfo.left( 1 );
      noisflag           = ninfo.section( dlm, 3, 3 ).toInt();
      QString noiIti     = ninfo.section( dlm, 4, 4 );
      QString noiIri     = ninfo.section( dlm, 5, 5 );
DbgLv(1) << "LRNoi:gn: nf" << noisflag << "noiIti,Iri" << noiIti << noiIri
 << "ninfo" << ninfo;

      if ( ( noisflag & 1 ) != 0 )
      {  // Has TI noise, so get it
         if ( dbP != NULL )
         {  // Load from DB
            ti_noise.load( noiIti, dbP );
DbgLv(1) << "LRNoi:gn:   noiIti" << noiIti << "count" << ti_noise.count;
         }
         else
         {  // Load from local file
            ti_noise.load( noiIti );
         }
      }
      else
      {  // No TI noise, so empty it
         ti_noise           = US_Noise();
      }

      if ( ( noisflag & 2 ) != 0 )
      {  // Has RI noise, so get it
         if ( dbP != NULL )
         {  // Load from DB
            ri_noise.load( noiIri, dbP );
DbgLv(1) << "LRNoi:gn:   noiIri" << noiIri << "count" << ri_noise.count;
         }
         else
         {  // Load from local file
            ri_noise.load( noiIri );
         }
      }
      else
      {  // No RI noise, so empty it
         ri_noise           = US_Noise();
      }
   }

   return noisflag;
}

// Build a list of latest edit GUIDs and related noises from database
int US_LoadRunNoise::scan_db( const QString runID )
{
   int krnois     = 0;
   QStringList query;

   //QString invID  = QString::number( US_Settings::us_inv_ID() );
   QString invID  = ( us_automode ) ? protocol_details["invID_passed"] : QString::number( US_Settings::us_inv_ID() );

   QVector< QString >    vEaucIDs;    // Edit info vectors
   QVector< QString >    vEedtIDs;
   QVector< QString >    vEedtGIs;
   QVector< QDateTime >  vEetimes;

   QVector< QString   > vNedtIDs;     // Noise info vectors
   QVector< QString   > vNedtGIs;
   QVector< QString   > vNnoiIDs;
   QVector< QString   > vNnoiGIs;
   QVector< QString   > vNmodIDs;
   QVector< QString   > vNnoTyps;
   QVector< QDateTime > vNntimes;
   QVector< QString   > vNdescrs;

   US_Passwd pw;
   dbP            = new US_DB2( pw.getPasswd() );
DbgLv(1) << "LRNoi:scdb:  dbP" << dbP << "invID" << invID << "runID" << runID;

   if ( dbP->lastErrno() != US_DB2::OK )
      return 0;

   query.clear();
   query << "get_edit_desc_by_runID" << invID << runID;
   dbP->query( query );

   while ( dbP->next() )
   {  // Accumulate latest edits from raw records of the run
      QString   edtID  = dbP->value( 0 ).toString();
      QString   aucID  = dbP->value( 3 ).toString();
      QDateTime etime  = dbP->value( 5 ).toDateTime();
      QString   edtGI  = dbP->value( 9 ).toString();
      int aucx         = vEaucIDs.indexOf( aucID );
DbgLv(1) << "LRNoi:scdb:     aucID" << aucID << "aucx" << aucx
 << "edtGI" << edtGI;

      if ( aucx < 0 )
      {  // First encountered raw for an edit:  append information
         vEaucIDs << aucID;
         vEedtIDs << edtID;
         vEedtGIs << edtGI;
         vEetimes << etime;
      }

      else if ( etime > vEetimes[ aucx ] )
      {  // Duplicate edit in a raw:  save information from the latest one
         vEaucIDs[ aucx ] = aucID;
         vEedtIDs[ aucx ] = edtID;
         vEedtGIs[ aucx ] = edtGI;
         vEetimes[ aucx ] = etime;
      }
   }

   edGUIDs.clear();

   for ( int ii = 0; ii < vEedtGIs.count(); ii++ )
   {  // Build the list of latest-edit GUIDs for the run
      edGUIDs << vEedtGIs[ ii ];
   }
int eeknt=edGUIDs.count();
QString eg0 = (eeknt>0) ? edGUIDs[0]       : "";
QString egn = (eeknt>0) ? edGUIDs[eeknt-1] : "";
DbgLv(1) << "LRNoi:scdb:   edGUIDS count" << eeknt
 << "eG0 eGn" << eg0 << egn;

   query.clear();
   query << "get_noise_desc" << invID;
   dbP->query( query );

   while ( dbP->next() )
   {  // Save noise information where edit is in list
      QString noiID    = dbP->value( 0 ).toString();
      QString noiGI    = dbP->value( 1 ).toString();
      QString edtID    = dbP->value( 2 ).toString();
      int edtx         = vEedtIDs.indexOf( edtID );

      if ( edtID.toInt() < 1  ||  edtx < 0 )   continue;

      QString edtGI    = vEedtGIs[ edtx ];
      QString modID    = dbP->value( 3 ).toString();
      QString noTyp    = dbP->value( 4 ).toString();
      QDateTime ntime  = dbP->value( 6 ).toDateTime();
      QString descr    = dbP->value( 9 ).toString();

      vNedtIDs << edtID;
//DbgLv(1) << "LRNoi:scdb:   edtx" << edtx << "edtID edtGI" << edtID << edtGI;
      vNedtGIs << edtGI;
      vNnoiIDs << noiID;
      vNnoiGIs << noiGI;
      vNmodIDs << modID;
      vNnoTyps << noTyp;
      vNntimes << ntime;
      vNdescrs << descr;
   }

   int nnoie        = vNedtGIs.count();
QString ng0=(nnoie>0)?vNedtGIs[0]:"";
QString ngn=(nnoie>0)?vNedtGIs[nnoie-1]:"";
 DbgLv(1) << "LRNoi:scdb:   vNedtGIs count" << nnoie
  << "nG0 nGn" << ng0 << ngn  << "edGUIDS k" << edGUIDs.count();
   const QString dlm( "^" );
   QDateTime ntime  = ( nnoie > 0 ) ? vNntimes[ 0 ]
                                    : QDateTime::currentDateTime();

   // Build noise information list corresponding to each edit
   for ( int ii = 0; ii < edGUIDs.count(); ii++ )
   {
      QString edtGI    = edGUIDs[ ii ];
      int fendx        = ( nnoie > 0 ) ? vNedtGIs.indexOf( edtGI ) : -1;
      int lendx        = ( nnoie > 0 ) ? vNedtGIs.lastIndexOf( edtGI ) : -1;
      int noiflag      = 0;
      QString noiIt( "" );
      QString noiIr( "" );
      QString noiID( "" );
      QString noiGI( "" );
      QString modID( "" );
      QString noTyp( "" );
      QString descr( "" );
      QString ninfo( "" );
      QString nflag( "" );
DbgLv(1) << "LRNoi:scdb: ii" << ii << "fx lx" << fendx << lendx
 << "edtGI" << edtGI;

      if ( fendx < 0 )
      {  // Create a single entry for edit with no-noise type
         nflag            = QString( "0" );
         noiIt            = QString( "-1" );
         noiIr            = QString( "-1" );
      }

      else if ( fendx < lendx )
      {  // Pare down to single entry with possibly multiple noises
         noiflag          = 0;
         ntime            = vNntimes[ fendx ];
         int jlate        = fendx;
         int jlate2       = -1;
         modID            = vNmodIDs[ fendx ];

         // Find the latest noise and its model ID
         for ( int jj = fendx; jj <= lendx; jj++ )
         {
            if ( vNedtGIs[ jj ] == edtGI  &&  vNntimes[ jj ] >= ntime )
            {
               ntime            = vNntimes[ jj ];
               modID            = vNmodIDs[ jj ];
               noTyp            = vNnoTyps[ jj ];
               jlate            = jj;
               noiflag          = ( noTyp == "ti_noise" ) ? 1 : 2;
            }
         }
//DbgLv(1) << "LRNoi:scdb:   jlate" << jlate << "noif" << noiflag
// << "modID" << modID << vNdescrs[jlate];

         // Look for another noise with the same model ID
         for ( int jj = fendx; jj <= lendx; jj++ )
         {
//DbgLv(1) << "LRNoi:scdb:     jj" << jj << "jmodID" << vNmodIDs[jj];
            if ( jj != jlate  &&  vNmodIDs[ jj ] == modID )
            {
               noTyp            = vNnoTyps[ jj ];
               noiflag          = 3;
               jlate2           = jj;
//DbgLv(1) << "LRNoi:scdb:    jlate2" << jlate2 << "noif" << noiflag
// << "type descr" << noTyp << vNdescrs[jlate2];
               break;
            }
         }

         if ( noiflag == 3 )
         {
            if ( vNnoTyps[ jlate ] == "ti_noise" )
            {
               noiIt            = vNnoiIDs[ jlate ];
               noiIr            = vNnoiIDs[ jlate2 ];
            }

            else
            {
               noiIr            = vNnoiIDs[ jlate ];
               noiIt            = vNnoiIDs[ jlate2 ];
            }
//DbgLv(1) << "LRNoi:scdb:     noiIt noiIr" << noiIt << noiIr;
         }

         else if ( noiflag == 1 )
         {
            noiIt            = noiID;
            noiIr            = QString( "-1" );
         }

         else
         {
            noiIt            = QString( "-1" );
            noiIr            = noiID;
         }
      }

      else if ( fendx == lendx )
      {  // Create a single entry with a single noise
         noiID            = vNnoiIDs[ fendx ];
         noiGI            = vNnoiGIs[ fendx ];
         modID            = vNmodIDs[ fendx ];
         noTyp            = vNnoTyps[ fendx ];
         ntime            = vNntimes[ fendx ];
         descr            = vNdescrs[ fendx ];

         if ( noTyp == "ti_noise" )
         {
            noiIt            = noiID;
            noiIr            = QString( "-1" );
            noiflag          = 1;
         }

         else
         {
            noiIr            = noiID;
            noiIt            = QString( "-1" );
            noiflag          = 2;
         }
      }

      // Save a noise information string for the current edit
      nflag            = QString::number( noiflag );
      ninfo            = dlm + edtGI +
                         dlm + modID +
                         dlm + nflag +
                         dlm + noiIt +
                         dlm + noiIr;
//DbgLv(1) << "LRNoi:scdb:      ninfo" << ninfo;
      noiInfos << ninfo;
      krnois          += noiflag < 1  ? 0 : ( noiflag < 3 ? 1 : 2 );
   }

DbgLv(1) << "LRNoi:scdb: krnois" << krnois;
   return krnois;
}

// Build a list of latest edit GUIDs and related noises from local files
int US_LoadRunNoise::scan_disk( const QString runID )
{
   QVector< QString   > vNedtIDs;     // Noise info vectors
   QVector< QString   > vNedtGIs;
   QVector< QString   > vNnoiIDs;
   QVector< QString   > vNnoiGIs;
   QVector< QString   > vNmodIDs;
   QVector< QString   > vNnoTyps;
   QVector< QDateTime > vNntimes;
   QVector< QString   > vNdescrs;
   QMap< QString, QString > meGImap;  // Map model model-GUIDs to edit-GUIDs

   QXmlStreamAttributes attr;
   int krnois         = 0;
   QString rpath      = US_Settings::resultDir() + "/" + runID;
   QDir dir           = QDir( rpath );
DbgLv(1) << "LRNoi:scdk:  rpath" << rpath;

   if ( ! dir.exists() )
      return krnois;

   // Build the list of edit GUIDs for the run
   QStringList filter( runID + ".*.xml" );
   QStringList f_names = dir.entryList( filter, QDir::Files, QDir::Name );
   edGUIDs .clear();
   noiInfos.clear();
DbgLv(1) << "LRNoi:scdk:  fn count" << f_names.count();

   for ( int ii = 0; ii < f_names.count(); ii++ )
   {
      QString fname( rpath + "/" + f_names[ ii ] );
      QFile   e_file( fname );
DbgLv(1) << "LRNoi:scdk:    ii" << ii << "fname" << fname;

      if ( !e_file.open( QIODevice::ReadOnly | QIODevice::Text ) )
         continue;

      QXmlStreamReader xml( &e_file );

      while ( ! xml.atEnd() )
      {  // Search XML elements until we find "editGUID"
         xml.readNext();

         if ( xml.isStartElement()  &&  xml.name() == "editGUID" )
         {  // test for desired editGUID
            attr    = xml.attributes();
            QString edtGI   = attr.value( "value" ).toString();
//DbgLv(1) << "LRNoi:scdk:      edtGI" << edtGI;
            edGUIDs << edtGI;
            break;
         }
      }

      e_file.close();
   }
DbgLv(1) << "LRNoi:scdk:  eGI count" << edGUIDs.count();

   // Build a mapping of model GUIDs to edit GUIDs
   QString mpath      = US_Settings::dataDir() + "/models";
   dir                = QDir( mpath );

   if ( ! dir.exists() )
      return krnois;

   filter .clear();
   f_names.clear();
   filter << QString( "M*.xml" );
   f_names = dir.entryList( filter, QDir::Files, QDir::Name );
DbgLv(1) << "LRNoi:scdk:  mpath" << mpath << "fn count" << f_names.count();

   for ( int ii = 0; ii < f_names.count(); ii++ )
   {
      QString fname( mpath + "/" + f_names[ ii ] );
      QFile   m_file( fname );

      if ( !m_file.open( QIODevice::ReadOnly | QIODevice::Text ) )
         continue;
//DbgLv(1) << "LRNoi:scdk:    ii" << ii << "fname" << fname;

      QXmlStreamReader xml( &m_file );

      while ( ! xml.atEnd() )
      {  // Search XML elements for model GUID values
         xml.readNext();

         if ( xml.isStartElement()  &&  xml.name() == "model" )
         {  // test for desired editGUID
            attr            = xml.attributes();
            QString edtGI   = attr.value( "editGUID"  ).toString();
            QString modGI   = attr.value( "modelGUID" ).toString();

            if ( ! edGUIDs.contains( edtGI ) )
               break;
//DbgLv(1) << "LRNoi:scdk:      modGI" << modGI << "edtGI" << edtGI;

            meGImap[ modGI ] = edtGI;   // Save model-to-edit GUID mapping
         }
      }

      m_file.close();
   }

   QStringList mmGIs  = meGImap.keys();
int mmk=mmGIs.count();
DbgLv(1) << "LRNoi:scdk:  mmGI count" << mmk;
if(mmk>0)
DbgLv(1) << "LRNoi:scdk:  mmGI 0 n" << mmGIs[0] << mmGIs[mmk-1];
   // Now build the list of noise information strings for each edit
   QString npath      = US_Settings::dataDir() + "/noises";
   dir                = QDir( npath );

   if ( ! dir.exists() )
      return krnois;

   filter .clear();
   f_names.clear();
   filter << QString( "N*.xml" );
   f_names            = dir.entryList( filter, QDir::Files, QDir::Name );
DbgLv(1) << "LRNoi:scdk:  npath" << npath << "fn count" << f_names.count();

   for ( int ii = 0; ii < f_names.count(); ii++ )
   {
      QString fname( npath + "/" + f_names[ ii ] );
      QFile   m_file( fname );

      if ( !m_file.open( QIODevice::ReadOnly | QIODevice::Text ) )
         continue;

      QXmlStreamReader xml( &m_file );

      while ( ! xml.atEnd() )
      {  // Search XML elements until we find "noise"
         xml.readNext();

         if ( xml.isStartElement()  &&  xml.name() == "noise" )
         {  // test for desired editGUID
            attr    = xml.attributes();
            QString modGI   = attr.value( "modelGUID" ).toString();

            if ( ! mmGIs.contains( modGI ) )
               break;

            QString edtGI   = meGImap[ modGI ];

            if ( ! edGUIDs.contains( edtGI ) )
               break;

            vNedtIDs << "-1";
            vNedtGIs << edtGI;
            vNnoiIDs << fname;
            vNnoiGIs << attr.value( "noiseGUID" ).toString();
            vNmodIDs << modGI;
            vNnoTyps << attr.value( "type" ).toString() + "_noise";
            vNntimes << QFileInfo( fname ).lastModified();
            vNdescrs << attr.value( "description" ).toString();
         }
      }
   }

   const QString dlm( "^" );

   // Build noise information list corresponding to each edit
   for ( int ii = 0; ii < edGUIDs.count(); ii++ )
   {
      QString edtGI    = edGUIDs[ ii ];
      int fendx        = vNedtGIs.indexOf( edtGI );
      int lendx        = vNedtGIs.lastIndexOf( edtGI );
      int noiflag      = 0;
      QString noiIt( "" );
      QString noiIr( "" );
      QString noiID( "" );
      QString noiGI( "" );
      QString modID( "" );
      QString noTyp( "" );
      QDateTime ntime  = vNntimes[ 0 ];
      QString descr( "" );
      QString ninfo( "" );
      QString nflag( "" );
//DbgLv(1) << "LRNoi:scdk: ii" << ii << "fx lx" << fendx << lendx
// << "edtGI" << edtGI;

      if ( fendx < 0 )
      {  // Create a single entry for edit with no-noise type
         nflag            = QString( "0" );
         noiIt            = QString( "-1" );
         noiIr            = QString( "-1" );
      }

      else if ( fendx < lendx )
      {  // Pare down to single entry with possibly multiple noises
         noiflag          = 0;
         ntime            = vNntimes[ fendx ];
         int jlate        = fendx;
         int jlate2       = -1;
         modID            = vNmodIDs[ fendx ];

         // Find the latest noise and its model ID
         for ( int jj = fendx; jj <= lendx; jj++ )
         {
            if ( vNedtGIs[ jj ] == edtGI  &&  vNntimes[ jj ] >= ntime )
            {
               ntime            = vNntimes[ jj ];
               modID            = vNmodIDs[ jj ];
               noTyp            = vNnoTyps[ jj ];
               jlate            = jj;
               noiflag          = ( noTyp == "ti_noise" ) ? 1 : 2;
            }
         }
//DbgLv(1) << "LRNoi:scdk:   jlate" << jlate << "noif" << noiflag
// << "modID" << modID << vNdescrs[jlate];

         // Look for another noise with the same model ID
         for ( int jj = fendx; jj <= lendx; jj++ )
         {
//DbgLv(1) << "LRNoi:scdk:     jj" << jj << "jmodID" << vNmodIDs[jj];
            if ( jj != jlate  &&  vNmodIDs[ jj ] == modID )
            {
               noTyp            = vNnoTyps[ jj ];
               noiflag          = 3;
               jlate2           = jj;
//DbgLv(1) << "LRNoi:scdk:    jlate2" << jlate2 << "noif" << noiflag
// << "type descr" << noTyp << vNdescrs[jlate2];
               break;
            }
         }

         if ( noiflag == 3 )
         {
            if ( vNnoTyps[ jlate ] == "ti_noise" )
            {
               noiIt            = vNnoiIDs[ jlate ];
               noiIr            = vNnoiIDs[ jlate2 ];
            }

            else
            {
               noiIr            = vNnoiIDs[ jlate ];
               noiIt            = vNnoiIDs[ jlate2 ];
            }
//DbgLv(1) << "LRNoi:scdk:     noiIt noiIr" << noiIt << noiIr;
         }

         else if ( noiflag == 1 )
         {
            noiIt            = noiID;
            noiIr            = QString( "-1" );
         }

         else
         {
            noiIt            = QString( "-1" );
            noiIr            = noiID;
         }
      }

      else if ( fendx == lendx )
      {  // Create a single entry with a single noise
         noiID            = vNnoiIDs[ fendx ];
         noiGI            = vNnoiGIs[ fendx ];
         modID            = vNmodIDs[ fendx ];
         noTyp            = vNnoTyps[ fendx ];
         ntime            = vNntimes[ fendx ];
         descr            = vNdescrs[ fendx ];

         if ( noTyp == "ti_noise" )
         {
            noiIt            = noiID;
            noiIr            = QString( "-1" );
            noiflag          = 1;
         }

         else
         {
            noiIr            = noiID;
            noiIt            = QString( "-1" );
            noiflag          = 2;
         }
      }

      // Save a noise information string for the current edit
      nflag            = QString::number( noiflag );
      ninfo            = dlm + edtGI +
                         dlm + modID +
                         dlm + nflag +
                         dlm + noiIt +
                         dlm + noiIr;
//DbgLv(1) << "LRNoi:scdk:      ninfo" << ninfo;
      noiInfos << ninfo;
      krnois          += noiflag < 1  ? 0 : ( noiflag < 3 ? 1 : 2 );
   }

   return krnois;
}

