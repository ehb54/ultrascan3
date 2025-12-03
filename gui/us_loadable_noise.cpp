//! \file us_loadable_noise.cpp

#include "us_loadable_noise.h"
#include "us_license_t.h"
#include "us_license.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_passwd.h"
#include "us_db2.h"
#include "us_noise.h"
#include "us_util.h"
#include "us_investigator.h"


// Null  constructor
US_LoadableNoise::US_LoadableNoise( QObject* parent ) : QObject( parent ) { }


// Determine if edit/model related noise available and build lists
int US_LoadableNoise::count_noise( bool ondisk, US_DataIO::EditedData* edata,
   US_Model* model, QStringList& mieGUIDs, QStringList& nieGUIDs )
{
   dbg_level   = US_Settings::us_debug();
   int noidiag = US_Settings::noise_dialog();

   int nenois  = 0;       // number of edited-data-related noises

   if ( edata == NULL )
      return nenois;


   QStringList nimGUIDs;  // list of GUIDs:type:index of noises-in-models
   QStringList tmpGUIDs;  // temporary noises-in-model list
   QString     daEditGUID = edata->editGUID;        // loaded edit GUID
   QString     modelGUID  = ( model == 0 ) ?        // loaded model GUID
                           "" : model->modelGUID;
   QString     lmodlGUID;                           // list model GUID
   QString     lnoisGUID;                           // list noise GUID
   QString     modelIndx;                           // "0001" style model index
DbgLv(2) << "LaNoi:editGUID  " << daEditGUID;
DbgLv(2) << "LaNoi:modelGUID " << modelGUID;
   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );

   // Build lists of IDs for noises and models
   if ( ondisk )
      id_list_disk( daEditGUID );

   else
      id_list_db  ( daEditGUID );
for ( int ii = 0; ii < noiIDs.size(); ii++ ) {
DbgLv(2) << "LaNoi:allNoi nID eID mID type" << noiIDs.at(ii)
 << noiEdIDs.at(ii) << noiMoIDs.at(ii) << noiTypes.at(ii); }
for ( int ii = 0; ii < modIDs.size(); ii++ ) {
DbgLv(2) << "LaNoi:allMod mID eID" << modIDs.at(ii) << modEdIDs.at(ii); }

   // Get a list of models-with-noise tied to the loaded edit
   int nemods  = models_in_edit(  ondisk, daEditGUID, mieGUIDs );

   if ( nemods == 0 )
   {
      QApplication::restoreOverrideCursor();
      return nemods;          // Go no further if no models-with-noise for edit
   }

   int latemx  = ondisk ? nemods - 1 : 0;   // Index to latest model-in-edit

   // If no model is loaded, pick the model GUID of the latest noise
   if ( model == 0 )
      modelGUID   = mieGUIDs[ latemx ];

   // Get a list of noises tied to the loaded model
   int nmnois  = noises_in_model( ondisk, modelGUID, nimGUIDs );

   // If the loaded model has no noise, try the latest model
   if ( nmnois == 0 )
   {
      modelGUID   = mieGUIDs[ latemx ];
      nmnois      = noises_in_model( ondisk, modelGUID, nimGUIDs );
   }

   // Insure that the loaded/latest model heads the model-in-edit list
   if ( modelGUID != mieGUIDs[ 0 ] )
   {
      if ( ! mieGUIDs.removeOne( modelGUID ) )
      {
         qDebug( "*ERROR* Loaded/Latest model not in model-in-edit list!" );
         QApplication::restoreOverrideCursor();
         return 0;
      }

      mieGUIDs.insert( 0, modelGUID );
   }

   int kk = 0;                // running output models index

   if ( nmnois > 0 )
   {  // If loaded model has noise, put noise in list
      nieGUIDs << nimGUIDs;   // initialize noise-in-edit list
      kk++;
   }

   nenois      = nmnois;      // initial noise-in-edit count is noises in model

   for ( int ii = 1; ii < nemods; ii++ )
   {  // Search through models in edit
      lmodlGUID  = mieGUIDs[ ii ];                    // this model's GUID
      modelIndx  = QString::asprintf( "%4.4d", kk );  // models-in-edit index

      // Find the noises tied to this model
      int kenois = noises_in_model( ondisk, lmodlGUID, tmpGUIDs );

      if ( kenois > 0 )
      {  // if we have 1 or 2 noises, add to noise-in-edit list
         nenois    += qMin( 2, kenois );
         // adjust entry to have the right model-in-edit index
         lnoisGUID  = tmpGUIDs.at( 0 ).section( ":", 0, 1 )
            + ":" + modelIndx;
         nieGUIDs << lnoisGUID;
         if ( kenois > 1 )
         {  // add a second noise to the list
            lnoisGUID  = tmpGUIDs.at( 1 ).section( ":", 0, 1 )
               + ":" + modelIndx;
            nieGUIDs << lnoisGUID;
         }
DbgLv(2) << "LaNoi: kenois nenois niesz" << kenois << nenois << nieGUIDs.size();

         kk++;
      }
   }
DbgLv(2) << "LaNoi:nemods nmnois nenois" << nemods << nmnois << nenois;
for (int jj=0;jj<nenois;jj++)
 DbgLv(2) << "LaNoi: jj nieG" << jj << nieGUIDs.at(jj);
   QApplication::restoreOverrideCursor();

   if ( nenois > 0 )
   {  // There is/are noise(s):  ask user if she wants to load
      QMessageBox msgBox;
      QString     amsg;
      QString     msg;

      if ( model == 0 )
         amsg = tr( ", associated with the loaded edit.\n" );

      else
         amsg = tr( ", associated with the loaded edit/model.\n" );

      if ( nenois > 1 )
      {
         msg  = tr( "There are noise files" ) + amsg
              + tr( "Do you want to load some of them?" );
      }

      else
      {  // Single noise file: check its value range versus experiment
         QString noiID  = nieGUIDs.at( 0 ).section( ":", 0, 0 );
         US_Noise i_noise;

         if ( ondisk )
            i_noise.load( false, noiID, NULL );
         else
         {
            US_Passwd pw;
            US_DB2 db( pw.getPasswd() );
            i_noise.load( true, noiID, &db );
         }

         double datmin  = edata->value( 0, 0 );
         double datmax  = datmin;
         double noimin  = 1.0e10;
         double noimax  = -noimin;
         int    npoint  = edata->pointCount();

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
            double noival = i_noise.values[ ii ];
            noimin        = qMin( noimin, noival );
            noimax        = qMax( noimax, noival );
         }

         if ( ( noimax - noimin ) > ( datmax - datmin ) )
         {  // Insert a warning if noise appears corrupt or unusual
            amsg = amsg
               + tr( "\nBUT THE NOISE HAS AN UNUSUALLY LARGE DATA RANGE.\n\n" );
         }

         msg  = tr( "There is a noise file" ) + amsg
              + tr( "Do you want to load it?" );
      }

DbgLv(2) << "LaNoi:noidiag  " << noidiag;
      if ( noidiag > 0 )
      {
         msgBox.setWindowTitle( tr( "Edit/Model Associated Noise" ) );
         msgBox.setText( msg );
         msgBox.setStandardButtons( QMessageBox::No | QMessageBox::Yes );
         msgBox.setDefaultButton( QMessageBox::Yes );

         if ( msgBox.exec() != QMessageBox::Yes )
         {  // user did not say "yes":  return zero count
            nenois  = 0;       // number of edited-data-related noises
         }
      }

      if ( kk < nemods )
      {  // Models with noise were found, so truncate models list
         for ( int ii = 0; ii < ( nemods - kk ); ii++ )
            mieGUIDs.removeLast();
      }
   }

   return nenois;
}

// Build a list of models(GUIDs) for a given edit(GUID)
int US_LoadableNoise::models_in_edit( bool ondisk, QString eGUID,
   QStringList& mGUIDs )
{
   QString xmGUID;
   QString xeGUID;
   QString xrGUID;
   QStringList reGUIDs;

   mGUIDs.clear();
DbgLv(2) << "LaNoi:MIE: ondisk" << ondisk;

   for ( int ii = 0; ii < modIDs.size(); ii++ )
   {  // Examine models list; Save to this list if edit GUID matches
      xmGUID = modIDs.at( ii );
      xeGUID = modEdIDs.at( ii );
     
      if ( eGUID == xeGUID )
      {
         mGUIDs << xmGUID;
      }
   }

   return mGUIDs.size();
}

// build a list of noise(GUIDs) for a given model(GUID)
int US_LoadableNoise::noises_in_model( bool ondisk, QString mGUID,
      QStringList& nGUIDs )
{
   QString xnGUID;
   QString xmGUID;
   QString xntype;

   nGUIDs.clear();

   for ( int ii = 0; ii < noiIDs.size(); ii++ )
   {  // Examine noises list; Save to this list if model GUID matches
      xnGUID = noiIDs  .at( ii );
      xmGUID = noiMoIDs.at( ii );
      xntype = noiTypes.at( ii );

      if ( !ondisk )
         xntype = xntype.contains( "ri_nois", Qt::CaseInsensitive ) ?
                   "ri" : "ti";
     
      if ( mGUID == xmGUID )
         nGUIDs << xnGUID + ":" + xntype + ":0000";
   }

   return nGUIDs.size();
}

// Build lists of noise and model IDs for database
int US_LoadableNoise::id_list_db( QString daEditGUID )
{
   QStringList query;
   QString     invID  = QString::number( US_Settings::us_inv_ID() );

   US_Passwd pw;
   US_DB2    db( pw.getPasswd() );

   if ( db.lastErrno() != IUS_DB2::OK )
      return 0;

   query.clear();
   query << "get_editID" << daEditGUID;
   db.query( query );
   db.next();
   QString daEditID = db.value( 0 ).toString();
DbgLv(2) << "LaNoi:idlDB:  daEdit ID GUID" << daEditID << daEditGUID;

   noiIDs  .clear();
   noiEdIDs.clear();
   noiMoIDs.clear();
   noiTypes.clear();
   modIDs  .clear();
   modEdIDs.clear();
   modDescs.clear();

   QStringList reqIDs;
   QString     noiEdID;

   // Build noise, edit, model ID lists for all noises
   query.clear();
   query << "get_noise_desc_by_editID" << invID << daEditID;
   db.query( query );

   while ( db.next() )
   {  // Accumulate lists from noise records
      noiEdID   = db.value( 2 ).toString();

      noiIDs   << db.value( 1 ).toString();
      noiTypes << db.value( 4 ).toString();
      noiMoIDs << db.value( 5 ).toString();
   }

DbgLv(2) << "LaNoi:idlDB: noiTypes size" << noiTypes.size();
   // Build model, edit ID lists for all models
   query.clear();
   query << "get_model_desc_by_editID" << invID << daEditID;
   db.query( query );

   while ( db.next() )
   {  // Accumulate from db desc entries matching noise model IDs
      QString modGUID = db.value( 1 ).toString();
      QString modEdID = db.value( 6 ).toString();

      if ( noiMoIDs.contains( modGUID )  &&   modEdID == daEditID )
      {  // Only list models that have associated noise and match edit
         modIDs   << modGUID;
         modDescs << db.value( 2 ).toString();
         modEdIDs << db.value( 5 ).toString();
      }
   }
DbgLv(2) << "LaNoi:idlDB: modDescs size" << modDescs.size();

   // Loop through models to edit out any extra monteCarlo models
   for ( int ii = modIDs.size() - 1; ii >=0; ii-- )
   {  // Work from the back so any removed records do not affect indexes
      QString mdesc  = modDescs.at( ii );
      QString asysID = mdesc.section( ".", -2, -2 );
      bool    mCarlo = ( asysID.contains( "-MC" )  &&
                         asysID.contains( "_mc" ) );
      QString reqID  = asysID.section( "_", 0, -2 );

      if ( mCarlo )
      {  // Treat monte carlo in a special way (as single composite model)
         if ( reqIDs.contains( reqID ) )
         {  // already have this request GUID, so remove this model
            modIDs  .removeAt( ii );
            modDescs.removeAt( ii );
            modEdIDs.removeAt( ii );
         }

         else
         {  // This is the first time for this request, so save it in a list
            reqIDs << reqID;
         }
      }
   }

   // Create list of edit GUIDs for noises
   for ( int ii = 0; ii < noiTypes.size(); ii++ )
   {
      QString moGUID  = noiMoIDs.at( ii );
      int     jj      = modIDs.indexOf( moGUID );
DbgLv(2) << "LaNoi:idlDB: ii jj moGUID" << ii << jj << moGUID;

      QString edGUID  = ( jj < 0 ) ? "" : modEdIDs.at( jj );

      noiEdIDs << edGUID;
   }

   return noiIDs.size();
}

// Build lists of noise and model IDs for local disk
int US_LoadableNoise::id_list_disk( QString daEditGUID )
{
   noiIDs  .clear();
   noiEdIDs.clear();
   noiMoIDs.clear();
   noiTypes.clear();
   modIDs  .clear();
   modEdIDs.clear();

   // First build noise lists

   QDir    dir;
   QString path = US_Settings::dataDir() + "/noises";
   dir.mkpath( path );
   dir          = QDir( path );

   QStringList filter( "N*.xml" );
   QStringList f_names = dir.entryList( filter, QDir::Files, QDir::Name );

   QXmlStreamAttributes attr;

   for ( int ii = 0; ii < f_names.size(); ii++ )
   {
      QString fname( path + "/" + f_names[ ii ] );
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
            noiIDs   << attr.value( "noiseGUID"   ).toString();
            noiMoIDs << attr.value( "modelGUID"   ).toString();
            noiTypes << attr.value( "type"        ).toString();
         }
      }

      m_file.close();
   }

   // Then build lists for models that have associated noise

   QStringList reqIDs;
   path         = US_Settings::dataDir() + "/models";
   dir.mkpath( path );
   dir          = QDir( path );

   filter.clear();
   filter << "M*.xml";
   f_names      = dir.entryList( filter, QDir::Files, QDir::Name );

   for ( int ii = 0; ii < f_names.size(); ii++ )
   {
      QString fname( path + "/" + f_names[ ii ] );
      QFile   m_file( fname );

      if ( !m_file.open( QIODevice::ReadOnly | QIODevice::Text ) )
         continue;

      QXmlStreamReader xml( &m_file );

      while ( ! xml.atEnd() )
      {  // Search XML elements until we find "model"
         xml.readNext();

         if ( xml.isStartElement()  &&  xml.name() == "model" )
         {  // test for desired editGUID
            attr    = xml.attributes();
            QString modelID = attr.value( "modelGUID"   ).toString();
            QString editID  = attr.value( "editGUID"    ).toString();
            QString reqGUID = attr.value( "requestGUID" ).toString();
            QString mcst    = attr.value( "monteCarlo"  ).toString();
            bool    mCarlo  = ( ! mcst.isEmpty()  &&  mcst != "0" );

            if ( noiMoIDs.contains( modelID )  &&  editID == daEditGUID )
            {  // Only list models that have associated noise that match edit
               if ( mCarlo )
               {  // Treat monte carlo in a special way (1 composite model)
                  if ( reqIDs.contains( reqGUID ) )
                     continue;           // Skip 2nd and beyond of same request

                  else
                     reqIDs << reqGUID;  // Record that this request ID is used
               }

               // Model that has some noise associations
               modIDs   << modelID;
               modEdIDs << editID;
            }
         }
      }

      m_file.close();
   }
   QString     dmyGUID  = "00000000-0000-0000-0000-000000000000";

   // Fill in edit GUIDs for noises by cross referencing model
   for ( int ii = 0; ii < noiMoIDs.size(); ii++ )
   {
      QString modelID = noiMoIDs.at( ii );
      int     jj      = modIDs.indexOf( modelID );
      QString editID  = jj >=0 ? modEdIDs.at( jj ) : dmyGUID;

      noiEdIDs << editID;
   }


   return noiIDs.size();
}

