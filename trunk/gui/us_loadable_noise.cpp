//! \file us_loadable_noise.cpp

#include <QtGui>

#include "us_loadable_noise.h"
#include "us_license_t.h"
#include "us_license.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_passwd.h"
#include "us_db2.h"
#include "us_util.h"
#include "us_investigator.h"


// Null  constructor
US_LoadableNoise::US_LoadableNoise( QObject* parent ) : QObject( parent ) { }


// Determine if edit/model related noise available and build lists
int US_LoadableNoise::count_noise( bool ondisk, US_DataIO2::EditedData* edata,
   US_Model* model, QStringList& mieGUIDs, QStringList& nieGUIDs )
{
   dbg_level   = US_Settings::us_debug();

   int nenois  = 0;       // number of edited-data-related noises

   if ( edata == NULL )
      return nenois;

   QStringList nimGUIDs;  // list of GUIDs:type:index of noises-in-models
   QStringList tmpGUIDs;  // temporary noises-in-model list
   QString     editGUID  = edata->editGUID;         // loaded edit GUID
   QString     modelGUID = ( model == 0 ) ?         // loaded model GUID
                           "" : model->modelGUID;
   QString     lmodlGUID;                           // list model GUID
   QString     lnoisGUID;                           // list noise GUID
   QString     modelIndx;                           // "0001" style model index
DbgLv(1) << "LaNoi:editGUID  " << editGUID;
DbgLv(1) << "LaNoi:modelGUID " << modelGUID;

   // get a list of models tied to the loaded edit
   int nemods  = models_in_edit(  ondisk, editGUID, mieGUIDs );

   if ( nemods == 0 )
      return nemods;          // go no further if no models in edit

   // if no model is loaded, pick the model GUID of the latest noise
   if ( model == 0 )
      modelGUID   = mieGUIDs[ nemods - 1 ];

   // get a list of noises tied to the loaded model
   int nmnois  = noises_in_model( ondisk, modelGUID, nimGUIDs );

   // insure that the loaded model is at the head of the model-in-edit list
   if ( modelGUID != mieGUIDs[ 0 ] )
   {
      if ( ! mieGUIDs.removeOne( modelGUID ) )
      {
         qDebug( "*ERROR* Loaded model not in model-in-edit list!" );
         return 0;
      }

      mieGUIDs.insert( 0, modelGUID );
   }

   if ( nmnois > 0 )
   {  // if loaded model has noise, put noise in list
      nieGUIDs << nimGUIDs;   // initialize noise-in-edit list
   }

   nenois      = nmnois;      // initial noise-in-edit count is noises in model

   for ( int ii = 1; ii < nemods; ii++ )
   {  // search through models in edit
      lmodlGUID  = mieGUIDs[ ii ];                    // this model's GUID
      modelIndx  = QString().sprintf( "%4.4d", ii );  // models-in-edit index

      // find the noises tied to this model
      int kenois = noises_in_model( ondisk, lmodlGUID, tmpGUIDs );

      if ( kenois > 0 )
      {  // if we have 1 or 2 noises, add to noise-in-edit list
         nenois    += kenois;
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
      }
   }
DbgLv(1) << "LaNoi:nemods nmnois nenois" << nemods << nmnois << nenois;
for (int jj=0;jj<nenois;jj++)
 DbgLv(1) << "LaNoi: jj nieG" << jj << nieGUIDs.at(jj);

   if ( nenois > 0 )
   {  // There is/are noise(s):  ask user if she wants to load
      QMessageBox msgBox;
      QString     amsg;
      QString     msg;

      if ( model == 0 )
         amsg = tr( ", associated with the loaded edit\n" );

      else
         amsg = tr( ", associated with the loaded edit/model\n" );

      if ( nenois > 1 )
         msg  = tr( "There are noise files" ) + amsg
              + tr( "Do you want to load some of them?" );

      else
         msg  = tr( "There is a noise file" ) + amsg
              + tr( "Do you want to load it?" );

      msgBox.setWindowTitle( tr( "Edit/Model Associated Noise" ) );
      msgBox.setText( msg );
      msgBox.setStandardButtons( QMessageBox::No | QMessageBox::Yes );
      msgBox.setDefaultButton( QMessageBox::Yes );

      if ( msgBox.exec() != QMessageBox::Yes )
      {  // user did not say "yes":  return zero count
         nenois  = 0;       // number of edited-data-related noises
      }
   }

   return nenois;
}

// build a list of models(GUIDs) for a given edit(GUID)
int US_LoadableNoise::models_in_edit( bool ondisk, QString eGUID, QStringList& mGUIDs )
{
   QString xmGUID;
   QString xeGUID;
   QString xrGUID;
   QStringList reGUIDs;

   mGUIDs.clear();
DbgLv(1) << "LaNoi:MIE: ondisk" << ondisk;

   if ( ondisk )
   {  // Models from local disk files
      QDir    dir;
      QString path = US_Settings::dataDir() + "/models";

      if ( !dir.exists( path ) )
         dir.mkpath( path );

      dir          = QDir( path );

      QStringList filter( "M*.xml" );
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
         {  // Search XML elements until we find "model"
            xml.readNext();

            if ( xml.isStartElement()  &&  xml.name() == "model" )
            {  // test for desired editGUID
               attr    = xml.attributes();
               xeGUID  = attr.value( "editGUID"    ).toString();
               xmGUID  = attr.value( "modelGUID"   ).toString();
               xrGUID  = attr.value( "requestGUID" ).toString();
               int kmc = attr.value( "monteCarlo"  ).toString().toInt();

               if ( xeGUID != eGUID )
                  continue;

               if ( kmc == 1  &&  xrGUID.length() == 36 )
               {  // treat monte carlo specially
                  if ( reGUIDs.contains( xrGUID ) )
                  {  // already have this request GUID:  skip
                     continue;
                  }
                  reGUIDs << xrGUID;  // this is 1st:  save it for compare
               }

               // save the GUID of each model with a matching edit GUID
               mGUIDs << xmGUID;
            }
         }

         m_file.close();
      }
   }

   else
   {
      US_Passwd pw;
      US_DB2    db( pw.getPasswd() );

      if ( db.lastErrno() != US_DB2::OK )
      {
         qDebug() << "*** DB ERROR: " << db.lastErrno();
         return 0;
      }

      QList< int > mDbIDs;
      QStringList  query;
      QString      invID  = QString::number( US_Settings::us_inv_ID() );
DbgLv(1) << "LaNoi:MIE(db): invID" << invID;

      query.clear();

      query << "get_model_desc" << invID;
      db.query( query );

      while ( db.next() )
      {  // accumulate from db desc entries matching editGUID;
         xmGUID  = db.value( 1 ).toString();
         xeGUID  = db.value( 3 ).toString();
DbgLv(2) << "LaNoi:MIE(db): xm/xe/e GUID" << xmGUID << xeGUID << eGUID;

         if ( xeGUID == eGUID )
         {
            mGUIDs << xmGUID;
            mDbIDs << db.value( 0 ).toString().toInt();
         }
      }
DbgLv(1) << "LaNoi:MIE(db): pass 1 mGUIDs size" << mGUIDs.size() << mDbIDs.size();

      qSort( mDbIDs );            // sort model db IDs into ascending order

      // Make a pass thru models to exclude MC's beyond first

      for ( int ii = 0; ii < mDbIDs.size(); ii++ )
      {
         query.clear();
         query << "get_model_info" << QString::number( mDbIDs.at( ii ) );
         db.query( query );
         db.next();
         QString mxml = db.value( 2 ).toString();
         int     jj   = mxml.indexOf( "requestGUID="  );
         int     kk   = mxml.indexOf( "monteCarlo=" );
         xrGUID       = ( jj < 0 ) ? "" : mxml.mid( jj ).section( '"', 1, 1 );
         int     kmc  = ( kk < 0 ) ? 0 :
                        mxml.mid( kk ).section( '"', 1, 1 ).toInt();
DbgLv(2) << "LaNoi:MIE(db):  ii kmc rGlen" << ii << kmc << xrGUID.length()
 << " DbID" << mDbIDs.at( ii );

         if ( kmc == 1  &&  xrGUID.length() == 36 )
         {  // treat monte carlo specially

            if ( reGUIDs.contains( xrGUID ) )
            {  // already have this request GUID:  remove this model
               mGUIDs.removeOne( db.value( 0 ).toString() );
DbgLv(2) << "LaNoi:MIE(db):    mGI rmvd" << db.value( 0 ).toString();
            }

            else
            {  // this is 1st:  save it for compare
               reGUIDs << xrGUID;
DbgLv(2) << "LaNoi:MIE(db):    dsc savd" << db.value( 1 ).toString();
            }
         }
      }
DbgLv(1) << "LaNoi:MIE(db): pass 2 mGUIDs size" << mGUIDs.size() << mDbIDs.size();
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
   if ( ondisk )
   {  // Noises from local disk files
      QDir    dir;
      QString path = US_Settings::dataDir() + "/noises";

      if ( !dir.exists( path ) )
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
               xmGUID  = attr.value( "modelGUID"   ).toString();
               xnGUID  = attr.value( "noiseGUID"   ).toString();
               xntype  = attr.value( "type"        ).toString();

               if ( xmGUID == mGUID )
                  nGUIDs << xnGUID + ":" + xntype + ":0000";
            }
         }

         m_file.close();
      }
   }

   else
   {
      US_Passwd pw;
      US_DB2    db( pw.getPasswd() );

      if ( db.lastErrno() != US_DB2::OK )
         return 0;

      QStringList query;
      QString     invID  = QString::number( US_Settings::us_inv_ID() );
      QString     xnoiID;
      QString     xmodID;
      QString     modlID;

      query.clear();
      query << "get_modelID" << mGUID;
      db.query( query );
      db.next();
      modlID  = db.value( 0 ).toString();

      query.clear();
      query << "get_noise_desc" << invID;
      db.query( query );

      while ( db.next() )
      {  // accumulate from db desc entries matching editGUID;
         xnoiID  = db.value( 0 ).toString();
         xnGUID  = db.value( 1 ).toString();
         xmodID  = db.value( 3 ).toString();
         xntype  = db.value( 4 ).toString();
         xntype  = xntype.contains( "ri_nois", Qt::CaseInsensitive ) ?
                   "ri" : "ti";

//DbgLv(2) << "LaNoi:NIM(db): xm/xe/e ID" << xnoiID << xmodID << modlID;
         if ( xmodID == modlID )
            nGUIDs << xnGUID + ":" + xntype + ":0000";
      }
   }

   return nGUIDs.size();
}

