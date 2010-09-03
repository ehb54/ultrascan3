//! \file us_sync_exper.cpp

#include "us_sync_exper.h"
#include "us_util.h"

// Check synchronizing of experiment on database and local with raw data
US_SyncExperiment::US_SyncExperiment( US_DB2* a_db, QString a_invID,
      QWidget* parent /*= 0*/ )
{
   db       = a_db;
   invID    = a_invID;
   parentw  = parent;

   // get lists of experimental IDs and run IDs
   QStringList query;
   query << "get_experiment_desc" << invID;
   db->query( query );

   while ( db->next() )
   {
      expIDs << db->value( 0 ).toString();
      runIDs << db->value( 1 ).toString();
   }
}

// flag state of synchronization of raw data with experiment
int US_SyncExperiment::synchronize( US_DataModel::DataDesc& cdesc )
{
   int status = 0;
   QString runID;
   QString oper     = "upload/download";
   QString lexpGUID = ""; 
   QString dexpGUID = ""; 
   QString rexpGUID = cdesc.parentGUID; 
   QStringList query;
   bool    havelraw = false;
   bool    havelexp = false;
   bool    havedraw = false;
   bool    havedexp = false;
   bool    haverexp = !rexpGUID.isEmpty();
   bool    no_go    = true;

   if ( cdesc.recType != 1 )   // error return if this is not a raw record
      return 3001;

   // get the associated experiment for this raw data record

   if ( ( cdesc.recState & US_DataModel::REC_LO ) != 0 )
   {  // there is a local raw record:  check its parent
      havelraw   = true;
      lexpGUID   = expGUIDauc( cdesc.filename );
      havelexp   = !lexpGUID.startsWith( "0000" );
      runID      = cdesc.filename.section( "/", -1, -1 ).section( ".", 0, 0 );

      if ( ( cdesc.recState & US_DataModel::REC_DB ) == 0 )
      {  // local only, so this must be an upload operation
         oper       = "upload";

         if ( havelexp  &&  havelraw )
            no_go      = false;  // we can proceed if we have both exp and raw
      }
   }

   if ( ( cdesc.recState & US_DataModel::REC_DB ) != 0 )
   {  // there is a database raw record:  check its parent
      query.clear();
      query << "get_rawDataID_from_GUID" << cdesc.dataGUID;

      db->query( query );

      if ( db->lastErrno() == US_DB2::OK )
      {  // get experiment ID and run ID
         db->next();
         expID      = db->value( 1 ).toString();
         int jj     = expIDs.indexOf( expID );
         havedexp   = ( jj >= 0 );
         havedraw   = true;
         runID      = ( havedexp ) ? runIDs.at( jj ) : runID;

         if ( havedexp )
         {  // pick up GUID for possible compare to local
            query.clear();
            query << "get_experiment_info" << expID;
            db->query( query );
            db->next();
            dexpGUID   = db->value( 0 ).toString();
qDebug() << "db get_exp_inf: expGUID" << dexpGUID;
qDebug() << "db get_exp_inf: projID " << db->value(1).toString();
qDebug() << "db get_exp_inf: runID  " << db->value(2).toString();
qDebug() << "db get_exp_inf: rotorID" << db->value(6).toString();
qDebug() << "db get_exp_inf: type   " << db->value(7).toString();
qDebug() << "db get_exp_inf: comment" << db->value(10).toString();
qDebug() << "db get_exp_inf: dateUpd" << db->value(12).toString();
         }
      }

      if ( ( cdesc.recState & US_DataModel::REC_LO ) == 0 )
      {  // database only, so this must be an download operation
         oper       = "download";

         if ( havedexp  &&  havedraw )
            no_go      = false;
      }
   }

   if ( havedexp  &&  havelexp )
   {  // if we have experiment on db and local, check for consistency
      if ( dexpGUID == lexpGUID )
         no_go      = false;      // can proceed if GUIDs match

      else
         no_go      = true;       // otherwise, we cannot
   }

   if ( no_go )
   {  // appears we can not do the upload/download:  inform user
      QString msg =
         tr( "A parent Experiment record in the database\n" 
             "or on local disk is missing or inconsistent, so\n"
             "you are unable to %1 this Raw record.\n\n"
             "You may separately run us_convert and create\n"
             "or repair the experiment record; then return here\n"
             "later to attempt the operation on this record.\n\n"
             "NOTES:\n" )
             .arg( oper );

      msg += havedexp ? tr( "    Experiment is in database;\n" )
                      : tr( "    Experiment missing from database;\n" );
      msg += havelexp ? tr( "    Experiment file exists locally;\n" )
                      : tr( "    Experiment missing from local disk;\n" );
      msg += havedraw ? tr( "    Raw data is in database;\n" )
                      : tr( "    Raw data missing from database;\n" );
      msg += havelraw ? tr( "    Raw data file exists locally;\n" )
                      : tr( "    Raw data missing from local disk;\n" );
      msg += tr( "    Database experiment GUID:" );
      msg += ( havedexp ? "\n      \"" + dexpGUID + "\"\n"
                        : tr( "  (missing)\n" ) );
      msg += tr( "    Local experiment GUID:" );
      msg += ( havelexp ? "\n      \"" + lexpGUID + "\"\n"
                        : tr( "  (missing)\n" ) );
      msg += tr( "    Raw record parent GUID:" );
      msg += ( haverexp ? "\n      \"" + rexpGUID + "\"\n"
                        : tr( "  (missing)\n" ) );

      QMessageBox::information( parentw,
            tr( "Experiment Record Problem" ),
            msg );
      status = 3003;
   }

   else
   {  // if we can to do operation, insure record has correct parent GUID
      cdesc.parentGUID = dexpGUID.isEmpty() ? lexpGUID : dexpGUID;
   }

   return status;
}

// get the experiment GUID from the local XML file associated with an AUC
QString US_SyncExperiment::expGUIDauc( QString aucfile )
{
   // default dummy GUID
   QString expGUID = "00000000-0000-0000-0000-000000000000";
   // compose experiment file name from that of AUC
   QString expfnam = aucfile.section( "/", -1, -1 )
                            .section( ".", 0, 1 ) + ".xml";
   QString expfile = aucfile.section( "/", 0, -2 ) + "/" + expfnam;

   QFile file( expfile );

   if ( file.open( QIODevice::ReadOnly | QIODevice::Text ) )
   {  // navigate to experiment guid attribute
      QXmlStreamReader xml( &file );

      while( ! xml.atEnd() )
      {
         xml.readNext();

         if ( xml.isStartElement()  &&  xml.name() == "experiment" )
         {  // found it!  return its value
            QXmlStreamAttributes a = xml.attributes();
            expGUID  = a.value( "guid" ).toString();
            break;
         }
      }

      file.close();
   }

   return expGUID;
}

