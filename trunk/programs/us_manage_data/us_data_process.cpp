//! \file us_data_process.cpp

#include "us_data_process.h"
#include "us_data_model.h"
#include "us_sync_exper.h"
#include "us_util.h"

// class to process operatations on data:  upload/download/remove
US_DataProcess::US_DataProcess( US_DataModel* dmodel, QWidget* parent /*=0*/ )
{
   parentw          = parent;
   da_model         = dmodel;
   QString investig = da_model->invtext();
   QString invID    = investig.section( ":", 0, 0 );
   db               = da_model->dbase();

qDebug() << "DP:  investig invID" << investig << invID;
   syncExper        = new US_SyncExperiment( db, invID, parent );
qDebug() << "DP:  syncExper created";
}

// perform a record upload to the database from local disk
int US_DataProcess::record_upload( int irow )
{
qDebug() << "REC_ULD:EXP: row" << irow+1;
   int stat = 0;
stat = irow & 3;
stat = 888;

   cdesc            = da_model->row_datadesc( irow );

   QStringList query;
   QString filepath = cdesc.filename;
   QString pathdir  = filepath.section( "/",  0, -2 );
   QString filename = filepath.section( "/", -1, -1 );
   int     idData   = cdesc.recordID;

   if      ( cdesc.recType == 1 )
   {  // upload a Raw record
      US_DataIO2::RawData        rdata;

      QString runID    = filename.section( ".",  0,  0 );
      QString tripl    = filename.section( ".", -4, -2 )
                         .replace( ".", " / " );
      QString fileexp  = filename.section( ".",  0,  1 ) + ".xml";
      QString pathexp  = pathdir + "/" + fileexp;
      QString expID;
      QString expGUID  = "";
qDebug() << "REC_ULD:EXP: runID" << runID << "  tripl" << tripl;

      US_DataIO2::readRawData( filepath, rdata );

      stat = syncExper->synchronize( cdesc );

      if ( stat == 0 )
      {
         stat = db->writeBlobToDB( filepath,
                                   QString( "upload_aucData" ),
                                   idData );

      }
      //      stat = new_experiment_local( db, pathexp, tripl );
      //   stat = new_experiment_db( db, pathexp, expID, expGUID );
   }

   else if ( cdesc.recType == 2 )
   {  // upload an EditedData record
      stat = db->writeBlobToDB( filepath,
                                QString( "upload_editData" ),
                                idData );
   }

   else if ( cdesc.recType == 3 )
   {  // upload a Model record
      US_Model model;

      model.load( filepath );
      model.write( db );
   }

   else if ( cdesc.recType == 4 )
   {  // upload a Noise record
      US_Noise noise;

      noise.load( filepath );
      noise.write( db );
   }

   else
   {  // *ERROR*:  invalid type
      stat        = 3045;
   }

   return stat;
}

// perform a record download from the database to local disk
int US_DataProcess::record_download( int irow )
{
   int stat = 0;
   QStringList query;
   QString     filepath = "";
   QString     dataID;
   int         idData;
stat = irow & 3;
stat = 888;
   cdesc            = da_model->row_datadesc( irow );
   idData           = cdesc.recordID;
   dataID           = QString::number( idData );
   filepath         = cdesc.filename;

   if      ( cdesc.recType == 1 )
   {  // download a Raw record
      stat = db->readBlobFromDB( filepath, "download_aucData", idData );
   }

   else if ( cdesc.recType == 2 )
   {  // download an EditedData record
      stat = db->readBlobFromDB( filepath, "download_editData", idData );
   }

   else if ( cdesc.recType == 3 )
   {  // download a Model record
      query.clear();
      query << "get_model_info" << dataID;
      db->query( query );

      if ( db->lastErrno() != US_DB2::OK )
      {
         stat = 3023;
      }

      else
      {
         db->next();
         QByteArray contents = db->value( 2 ).toString().toAscii();

         QFile file( filepath );
         
         if ( !file.open( QIODevice::WriteOnly | QIODevice::Text ) )
         {
            stat = 3033;
         }

         else
         {
            file.write( contents );
            file.close();
         }
      }
   }

   else if ( cdesc.recType == 4 )
   {  // download a Noise record
      query.clear();
      query << "get_noise_info" << dataID;
      db->query( query );

      if ( db->lastErrno() != US_DB2::OK )
      {
         stat = 3024;
      }

      else
      {
         db->next();
         QByteArray contents = db->value( 2 ).toString().toAscii();

         QFile file( filepath );
         
         if ( !file.open( QIODevice::WriteOnly | QIODevice::Text ) )
         {
            stat = 3034;
         }

         else
         {
            file.write( contents );
            file.close();
         }
      }
   }

   else
   {  // *ERROR*:  invalid type
      stat        = 3015;
   }

   return stat;
}
// perform a record remove from the database
int US_DataProcess::record_remove_db( int irow )
{
   int stat = 0;
   QStringList query;
stat = irow & 3;
stat = 888;
   cdesc            = da_model->row_datadesc( irow );
   QString dataID   = QString::number( cdesc.recordID );

   if      ( cdesc.recType == 1 )
   {  // remove a Raw record
      query.clear();
      query << "delete_rawData" << dataID;

      if ( ! db->statusQuery( query ) )
      {
         stat = 2011;
      }
   }

   else if ( cdesc.recType == 2 )
   {  // remove an EditedData record
      query.clear();
      query << "delete_editedData" << dataID;

      if ( ! db->statusQuery( query ) )
      {
         stat = 2012;
      }
   }

   else if ( cdesc.recType == 3 )
   {  // remove a Model record
      query.clear();
      query << "delete_model" << dataID;

      if ( ! db->statusQuery( query ) )
      {
         stat = 2013;
      }
   }

   else if ( cdesc.recType == 4 )
   {  // remove a Noise record
      query.clear();
      query << "delete_noise" << dataID;

      if ( ! db->statusQuery( query ) )
      {
         stat = 2014;
      }
   }

   else
   {  // *ERROR*:  invalid type
      stat        = 2015;
   }
   return stat;
}

// perform a record remove from local disk
int US_DataProcess::record_remove_local( int irow )
{
   int stat = 0;
   cdesc            = da_model->row_datadesc( irow );

   QString filepath = cdesc.filename;
   QString filename = filepath.section( "/", -1, -1 );
   QFile   file( filepath );

   if ( file.exists() )
   {  // the file to remove does exist

      if ( file.remove() )
      {  // the remove was successful

         if ( ( cdesc.recState & US_DataModel::REC_DB ) == 0 )
         {  // it was local-only, so now it's a dummy
            cdesc.recState  = US_DataModel::NOSTAT;
         }

         else
         {  // it was on both, so now it's db-only
            cdesc.recState &= ~US_DataModel::REC_LO;
         }
      }

      else
      {  // an error occurred in removing
         stat     = 1000;
         qDebug() << "*ERROR* removing row file " << filename
            << "  (row" << irow << ")";
      }
   }

   else
   {  // file did not exist
      stat     = 2000;
      qDebug() << "*ERROR* attempt to remove non-existent file " << filename
         << "  (row" << irow << ")";
   }

   return stat;
}

