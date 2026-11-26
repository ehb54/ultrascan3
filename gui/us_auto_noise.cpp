//! \file us_auto_noise.cpp
#include "us_auto_noise.h"
#include "us_db2.h"
#include "us_passwd.h"
#include "us_settings.h"

#include <QApplication>
#include <QDateTime>
#include <QMessageBox>
#include <memory>

#ifndef DbgLv
#define DbgLv( a )                                                                                                                 \
   if ( dbg_level >= a )                                                                                                           \
   qDebug()
#endif

bool US_AutoNoise::load( QWidget*                parent,
                         int                     invID,
                         QMap<QString, QString>& triple_information,
                         US_DataIO::EditedData*  edata,
                         US_Model&               model,
                         US_Noise&               ti_noise,
                         US_Noise&               ri_noise )
{
   US_AutoNoise noise( parent, invID, edata, model, ti_noise, ri_noise );
   return noise.loadNoises( triple_information );
}

US_AutoNoise::US_AutoNoise( QWidget*               parent,
                            int                    invID,
                            US_DataIO::EditedData* edata,
                            US_Model&              model,
                            US_Noise&              ti_noise,
                            US_Noise&              ri_noise ) :
    parent( parent ),
    invID( invID ),
    dbg_level( US_Settings::us_debug() ),
    edata( edata ),
    model( model ),
    ti_noise( ti_noise ),
    ri_noise( ri_noise )
{
}

bool US_AutoNoise::loadNoises( QMap<QString, QString>& triple_information )
{
   US_Passwd               pw;
   std::unique_ptr<US_DB2> db( new US_DB2( pw.getPasswd() ) );

   if ( db->lastErrno() != US_DB2::OK )
   {
      QApplication::restoreOverrideCursor();
      QMessageBox::information( parent,
                                QObject::tr( "DB Connection Problem" ),
                                QObject::tr( "There was an error connecting to the database:\n" ) + db->lastError() );

      return false;
   }

   // get noiseIDs, types & lastUpd by modelID
   QStringList query;
   query << "get_noiseTypesIDs" << triple_information["mID"];
   db->query( query );

   qDebug() << "In loadNoises() Query: " << query;

   US_NewestRecord newest_ti;
   US_NewestRecord newest_ri;

   while ( db->next() )
   {
      int       noiseID   = db->value( 0 ).toInt();
      QString   noiseType = db->value( 1 ).toString();
      QDateTime date      = db->value( 2 ).toDateTime();

      QDateTime now = QDateTime::currentDateTime();

      if ( noiseType.contains( "ti_" ) )
      {
         newest_ti.offer( noiseID, date.secsTo( now ) );
      }
      if ( noiseType.contains( "ri_" ) )
      {
         newest_ri.offer( noiseID, date.secsTo( now ) );
      }
   }

   int nID_ti = newest_ti.id();
   int nID_ri = newest_ri.id();

   //ALEXEY: treat the case when model (like -MC does not possess its own noises -- use latest available noises for prior model like -IT  )
   //int US_LoadableNoise::count_noise() in ../../gui/us_loadable_noise.cpp
   //void US_FeMatch::load_noise( ) in us_fematch.cpp
   if ( !nID_ti && !nID_ri )
   {
      loadNoises_whenAbsent();
   }

   //creare US_noise objects
   if ( nID_ti )
   {
      ti_noise.load( QString::number( nID_ti ), db.get() );
      qDebug() << "loadNoises() NORMAL: ti_noise created: ID -- " << nID_ti;
   }
   if ( nID_ri )
   {
      ri_noise.load( QString::number( nID_ri ), db.get() );
      qDebug() << "loadNoises() NORMAL: ri_noise created: ID -- " << nID_ri;
   }

   // noise loaded:  insure that counts jive with data
   int ntinois = ti_noise.values.size();
   int nrinois = ri_noise.values.size();
   int nscans  = edata->scanCount();
   int npoints = edata->pointCount();
   int npadded = 0;


   qDebug() << "ti_noise.count, ri_noise.count: " << ti_noise.count << ri_noise.count;
   qDebug() << "ti_noise.values.size(), ri_noise.values.size(): " << ti_noise.values.size() << ri_noise.values.size();

   if ( ntinois > 0 && ntinois < npoints )
   { // pad out ti noise values to radius count
      int jj = ntinois;
      while ( jj++ < npoints )
      {
         ti_noise.values << 0.0;
      }
      ti_noise.count = ti_noise.values.size();
      npadded++;
   }

   if ( nrinois > 0 && nrinois < nscans )
   { // pad out ri noise values to scan count
      int jj = nrinois;
      while ( jj++ < nscans )
      {
         ri_noise.values << 0.0;
      }
      ri_noise.count = ri_noise.values.size();
      npadded++;
   }

   if ( npadded > 0 )
   { // let user know that padding occurred
      QString pmsg;

      if ( npadded == 1 )
      {
         pmsg = QObject::tr( "The noise file was padded out with zeroes\n"
                             "in order to match the data range." );
      }
      else
      {
         pmsg = QObject::tr( "The noise files were padded out with zeroes\n"
                             "in order to match the data ranges." );
      }

      QMessageBox::information( parent, QObject::tr( "Noise Padded Out" ), pmsg );
   }

   //test:
   db.reset();
   return true;
}

void US_AutoNoise::loadNoises_whenAbsent()
{
   QStringList mieGUIDs;                    // list of GUIDs of models-in-edit
   QStringList nieGUIDs;                    // list of GUIDS:type:index of noises-in-edit
   QString     editGUID  = edata->editGUID; // loaded edit GUID
   QString     modelGUID = model.modelGUID; // loaded model GUID

   int         noisdf = US_Settings::noise_dialog();
   int         nenois = count_noise_auto( edata, &model, mieGUIDs, nieGUIDs );

   qDebug() << "load_noise_whenAbsent(): mieGUIDs, nieGUIDs, editGUID, modelGUID -- " << mieGUIDs << nieGUIDs << editGUID
            << modelGUID;
   qDebug() << "load_noise_whenAbsent(): noisdf, nenois -- " << noisdf << nenois;

   if ( nenois > 0 )
   { // There is/are noise(s):  ask user if she wants to load
      US_Passwd               pw;
      std::unique_ptr<US_DB2> dbP( new US_DB2( pw.getPasswd() ) );

      if ( nenois > 1 && noisdf == 0 )
      { // Noise exists and noise-dialog flag set to "Auto-load"
         QString descn = nieGUIDs.at( 0 );
         QString noiID = descn.section( ":", 0, 0 );
         QString typen = descn.section( ":", 1, 1 );
         QString mdlx1 = descn.section( ":", 2, 2 );

         if ( typen == "ti" )
         {
            ti_noise.load( true, noiID, dbP.get() );
            qDebug() << "load_noise_whenAbsent(): ti_noise created: ID -- " << noiID;
            qDebug() << "load_noise_whenAbsent(): ti_noise.count: -- " << ti_noise.count;
         }
         else
         {
            ri_noise.load( true, noiID, dbP.get() );
            qDebug() << "load_noise_whenAbsent(): ri_noise created: ID -- " << noiID;
            qDebug() << "load_noise_whenAbsent(): ri_noise.count: -- " << ri_noise.count;
         }

         descn          = nieGUIDs.at( 1 );
         QString mdlx2  = descn.section( ":", 2, 2 );
         int     kenois = ( mdlx1 == mdlx2 ) ? 2 : 1;
         if ( kenois == 2 )
         { // Second noise from same model:  g/et it, too
            noiID = descn.section( ":", 0, 0 );
            typen = descn.section( ":", 1, 1 );
            if ( typen == "ti" )
            {
               ti_noise.load( true, noiID, dbP.get() );
               qDebug() << "load_noise_whenAbsent(): Noise 2: noiID, loaded ti--" << noiID;
               qDebug() << "load_noise_whenAbsent(): Noise 2: noiID, loaded ti_noise.count --" << ti_noise.count;
            }
            else
            {
               ri_noise.load( true, noiID, dbP.get() );
               qDebug() << "load_noise_whenAbsent(): Noise 2: noiID, loaded ri--" << noiID;
               qDebug() << "load_noise_whenAbsent(): Noise 2: noiID, loaded ri_noise.count --" << ri_noise.count;
            }
         }
      }
      else
      { // only 1:  just load it
         QString noiID = nieGUIDs.at( 0 );
         QString typen = noiID.section( ":", 1, 1 );
         noiID         = noiID.section( ":", 0, 0 );

         if ( typen == "ti" )
         {
            ti_noise.load( true, noiID, dbP.get() );
         }

         else
         {
            ri_noise.load( true, noiID, dbP.get() );
         }
      }
   }
}

int US_AutoNoise::count_noise_auto( US_DataIO::EditedData* edata, US_Model* model, QStringList& mieGUIDs, QStringList& nieGUIDs )
{
   int noidiag = US_Settings::noise_dialog();

   int nenois = 0; // number of edited-data-related noises

   if ( edata == NULL )
   {
      return nenois;
   }

   QStringList nimGUIDs;                     // list of GUIDs:type:index of noises-in-models
   QStringList tmpGUIDs;                     // temporary noises-in-model list
   QString     daEditGUID = edata->editGUID; // loaded edit GUID
   QString     modelGUID  = ( model == 0 ) ? // loaded model GUID
                          ""
                                           : model->modelGUID;
   QString     lmodlGUID; // list model GUID
   QString     lnoisGUID; // list noise GUID
   QString     modelIndx; // "0001" style model index

   id_list_db_auto( daEditGUID );

   // Get a list of models-with-noise tied to the loaded edit
   int nemods = models_in_edit_auto( daEditGUID, mieGUIDs );

   if ( nemods == 0 )
   {
      //QApplication::restoreOverrideCursor();
      return nemods; // Go no further if no models-with-noise for edit
   }

   int latemx = 0; // Index to latest model-in-edit

   // If no model is loaded, pick the model GUID of the latest noise
   if ( model == 0 )
   {
      modelGUID = mieGUIDs[latemx];
   }

   // Get a list of noises tied to the loaded model
   int nmnois = noises_in_model_auto( modelGUID, nimGUIDs );

   // If the loaded model has no noise, try the latest model
   if ( nmnois == 0 )
   {
      modelGUID = mieGUIDs[latemx];
      nmnois    = noises_in_model_auto( modelGUID, nimGUIDs );
   }

   // Insure that the loaded/latest model heads the model-in-edit list
   if ( modelGUID != mieGUIDs[0] )
   {
      if ( !mieGUIDs.removeOne( modelGUID ) )
      {
         qDebug( "*ERROR* Loaded/Latest model not in model-in-edit list!" );
         QApplication::restoreOverrideCursor();
         return 0;
      }

      mieGUIDs.insert( 0, modelGUID );
   }


   int kk = 0; // running output models index

   if ( nmnois > 0 )
   {                        // If loaded model has noise, put noise in list
      nieGUIDs << nimGUIDs; // initialize noise-in-edit list
      kk++;
   }

   nenois = nmnois; // initial noise-in-edit count is noises in model

   for ( int ii = 1; ii < nemods; ii++ )
   {                                                // Search through models in edit
      lmodlGUID = mieGUIDs[ii];                     // this model's GUID
      modelIndx = QString::asprintf( "%4.4d", kk ); // models-in-edit index

      // Find the noises tied to this model
      int kenois = noises_in_model_auto( lmodlGUID, tmpGUIDs );

      if ( kenois > 0 )
      { // if we have 1 or 2 noises, add to noise-in-edit list
         nenois += qMin( 2, kenois );
         // adjust entry to have the right model-in-edit index
         lnoisGUID = tmpGUIDs.at( 0 ).section( ":", 0, 1 ) + ":" + modelIndx;
         nieGUIDs << lnoisGUID;
         if ( kenois > 1 )
         { // add a second noise to the list
            lnoisGUID = tmpGUIDs.at( 1 ).section( ":", 0, 1 ) + ":" + modelIndx;
            nieGUIDs << lnoisGUID;
         }

         kk++;
      }
   }


   if ( nenois > 0 )
   { // There is/are noise(s):  ask user if she wants to load
      QMessageBox msgBox;
      QString     amsg;
      QString     msg;

      if ( model == 0 )
      {
         amsg = QObject::tr( ", associated with the loaded edit.\n" );
      }

      else
      {
         amsg = QObject::tr( ", associated with the loaded edit/model.\n" );
      }

      if ( nenois > 1 )
      {
         msg = QObject::tr( "There are noise files" ) + amsg + QObject::tr( "Do you want to load some of them?" );
      }

      else
      { // Single noise file: check its value range versus experiment
         QString   noiID = nieGUIDs.at( 0 ).section( ":", 0, 0 );
         US_Noise  i_noise;

         US_Passwd pw;
         US_DB2    db( pw.getPasswd() );
         i_noise.load( true, noiID, &db );

         double datmin = edata->value( 0, 0 );
         double datmax = datmin;
         double noimin = 1.0e10;
         double noimax = -noimin;
         int    npoint = edata->pointCount();

         for ( int ii = 0; ii < edata->scanData.size(); ii++ )
         {
            for ( int jj = 0; jj < npoint; jj++ )
            {
               double datval = edata->value( ii, jj );
               datmin        = qMin( datmin, datval );
               datmax        = qMax( datmax, datval );
            }
         }

         for ( int ii = 0; ii < i_noise.values.size(); ii++ )
         {
            double noival = i_noise.values[ii];
            noimin        = qMin( noimin, noival );
            noimax        = qMax( noimax, noival );
         }

         if ( ( noimax - noimin ) > ( datmax - datmin ) )
         { // Insert a warning if noise appears corrupt or unusual
            amsg = amsg + QObject::tr( "\nBUT THE NOISE HAS AN UNUSUALLY LARGE DATA RANGE.\n\n" );
         }

         msg = QObject::tr( "There is a noise file" ) + amsg + QObject::tr( "Do you want to load it?" );
      }

      DbgLv( 2 ) << "LaNoi:noidiag  " << noidiag;
      if ( noidiag > 0 )
      {
         msgBox.setWindowTitle( QObject::tr( "Edit/Model Associated Noise" ) );
         msgBox.setText( msg );
         msgBox.setStandardButtons( QMessageBox::No | QMessageBox::Yes );
         msgBox.setDefaultButton( QMessageBox::Yes );

         if ( msgBox.exec() != QMessageBox::Yes )
         {              // user did not say "yes":  return zero count
            nenois = 0; // number of edited-data-related noises
         }
      }

      if ( kk < nemods )
      { // Models with noise were found, so truncate models list
         for ( int ii = 0; ii < ( nemods - kk ); ii++ )
         {
            mieGUIDs.removeLast();
         }
      }
   }

   return nenois;
}

int US_AutoNoise::noises_in_model_auto( QString mGUID, QStringList& nGUIDs )
{
   QString xnGUID;
   QString xmGUID;
   QString xntype;

   nGUIDs.clear();

   for ( int ii = 0; ii < noiIDs.size(); ii++ )
   { // Examine noises list; Save to this list if model GUID matches
      xnGUID = noiIDs.at( ii );
      xmGUID = noiMoIDs.at( ii );
      xntype = noiTypes.at( ii );

      xntype = xntype.contains( "ri_nois", Qt::CaseInsensitive ) ? "ri" : "ti";

      if ( mGUID == xmGUID )
      {
         nGUIDs << xnGUID + ":" + xntype + ":0000";
      }
   }

   return nGUIDs.size();
}

int US_AutoNoise::models_in_edit_auto( QString eGUID, QStringList& mGUIDs )
{
   QString     xmGUID;
   QString     xeGUID;
   QString     xrGUID;
   QStringList reGUIDs;

   mGUIDs.clear();

   for ( int ii = 0; ii < modIDs.size(); ii++ )
   { // Examine models list; Save to this list if edit GUID matches
      xmGUID = modIDs.at( ii );
      xeGUID = modEdIDs.at( ii );

      if ( eGUID == xeGUID )
      {
         mGUIDs << xmGUID;
      }
   }

   return mGUIDs.size();
}

int US_AutoNoise::id_list_db_auto( QString daEditGUID )
{
   QStringList query;

   US_Passwd   pw;
   US_DB2      db( pw.getPasswd() );

   if ( db.lastErrno() != US_DB2::OK )
   {
      return 0;
   }

   query.clear();
   query << "get_editID" << daEditGUID;
   db.query( query );
   db.next();
   QString daEditID = db.value( 0 ).toString();
   DbgLv( 1 ) << "LaNoi:idlDB:  daEdit ID GUID" << daEditID << daEditGUID;

   noiIDs.clear();
   noiEdIDs.clear();
   noiMoIDs.clear();
   noiTypes.clear();
   modIDs.clear();
   modEdIDs.clear();
   modDescs.clear();

   QStringList reqIDs;
   QString     noiEdID;

   // Build noise, edit, model ID lists for all noises
   query.clear();
   query << "get_noise_desc_by_editID" << QString::number( invID ) << daEditID;
   db.query( query );

   while ( db.next() )
   { // Accumulate lists from noise records
      noiEdID = db.value( 2 ).toString();

      noiIDs << db.value( 1 ).toString();
      noiTypes << db.value( 4 ).toString();
      noiMoIDs << db.value( 5 ).toString();
   }

   DbgLv( 1 ) << "LaNoi:idlDB: noiTypes size" << noiTypes.size();
   // Build model, edit ID lists for all models
   query.clear();
   query << "get_model_desc_by_editID" << QString::number( invID ) << daEditID;
   db.query( query );

   while ( db.next() )
   { // Accumulate from db desc entries matching noise model IDs
      QString modGUID = db.value( 1 ).toString();
      QString modEdID = db.value( 6 ).toString();

      if ( noiMoIDs.contains( modGUID ) && modEdID == daEditID )
      { // Only list models that have associated noise and match edit
         modIDs << modGUID;
         modDescs << db.value( 2 ).toString();
         modEdIDs << db.value( 5 ).toString();
      }
   }
   DbgLv( 1 ) << "LaNoi:idlDB: modDescs size" << modDescs.size();

   // Loop through models to edit out any extra monteCarlo models
   for ( int ii = modIDs.size() - 1; ii >= 0; ii-- )
   { // Work from the back so any removed records do not affect indexes
      QString mdesc  = modDescs.at( ii );
      QString asysID = mdesc.section( ".", -2, -2 );
      bool    mCarlo = ( asysID.contains( "-MC" ) && asysID.contains( "_mc" ) );
      QString reqID  = asysID.section( "_", 0, -2 );

      if ( mCarlo )
      { // Treat monte carlo in a special way (as single composite model)
         if ( reqIDs.contains( reqID ) )
         { // already have this request GUID, so remove this model
            modIDs.removeAt( ii );
            modDescs.removeAt( ii );
            modEdIDs.removeAt( ii );
         }

         else
         { // This is the first time for this request, so save it in a list
            reqIDs << reqID;
         }
      }
   }

   // Create list of edit GUIDs for noises
   for ( int ii = 0; ii < noiTypes.size(); ii++ )
   {
      QString moGUID = noiMoIDs.at( ii );
      int     jj     = modIDs.indexOf( moGUID );
      DbgLv( 2 ) << "LaNoi:idlDB: ii jj moGUID" << ii << jj << moGUID;

      QString edGUID = ( jj < 0 ) ? "" : modEdIDs.at( jj );

      noiEdIDs << edGUID;
   }

   return noiIDs.size();
}
