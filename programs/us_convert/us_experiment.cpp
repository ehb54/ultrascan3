//! \file us_experiment.cpp

#include <QtCore>

#include "us_settings.h"
#include "us_db2.h"
#include "us_passwd.h"
#include "us_investigator.h"
#include "us_util.h"
#include "us_simparms.h"
#include "us_experiment.h"

US_Experiment::US_Experiment( void )
{
   // Experiment types
   experimentTypes.clear();
   experimentTypes << "Velocity"
                   << "Equilibrium"
                   << "Diffusion"
                   << "Buoyancy"
                   << "Calibration"
                   << "other";
}

// Function to see if the current runID already exists in the database
int US_Experiment::checkRunID( US_DB2* db )
{
   // Let's see if we can find the run ID
   expID = 0;
   QStringList q( "get_experiment_info_by_runID" );
   q << runID
     << QString::number( US_Settings::us_inv_ID() );
   db->query( q );
   if ( db->lastErrno() == US_DB2::NOROWS )
      return US_DB2::NOROWS;
   
   // Ok, let's update the experiment ID
   db->next();
   expID = db->value( 1 ).toString().toInt();
   return US_DB2::OK;
}

// Copy of checkRunID when invID propagated in autoflow
int US_Experiment::checkRunID_auto( int invID_passed, US_DB2* db )
{
   // Let's see if we can find the run ID
   expID = 0;
   QStringList q( "get_experiment_info_by_runID" );
   q << runID
     << QString::number( invID_passed );
   db->query( q );
   if ( db->lastErrno() == US_DB2::NOROWS )
      return US_DB2::NOROWS;
   
   // Ok, let's update the experiment ID
   db->next();
   expID = db->value( 1 ).toString().toInt();
   return US_DB2::OK;
}

int US_Experiment::saveToDB( bool update, US_DB2* db,
                             QVector< SP_SPEEDPROFILE >& speedsteps )
{
qDebug() << "Exp:svToDB: update" << update << "ss-count" << speedsteps.count();
   // Let's see if the project is in the db already
   int status = project.saveToDB( db );
qDebug() << "Exp:svToDB: projsv status(+NO_PR)" << status << US_DB2::NO_PROJECT
 << "expID" << expID;
   if ( status == US_DB2::NO_PROJECT )
      return status;

   else if ( status != US_DB2::OK )
      return status;

   // Get the RI information, if appropriate
   QByteArray RIxml;
   QByteArray RIxmlEscaped;
   createRIXml( RIxml );
   //unsigned long length = db->mysqlEscapeString( RIxmlEscaped, RIxml, RIxml.size() );
   db->mysqlEscapeString( RIxmlEscaped, RIxml, RIxml.size() );

   // Check for experiment runID in database
   int saveStatus = 0;
   QStringList q;
   status = checkRunID( db );
   if ( status == US_DB2::OK && ! update )
   {
      // Then the runID exists already, and we're not updating
      return US_DB2::DUPFIELD;
   }

   if ( status == US_DB2::OK )
   {
      // It's ok to update the existing experiment entry
      q.clear();
      q  << "update_experiment"
         << QString::number( expID )
         << QString( expGUID )
         << QString::number( project.projectID )
         << runID
         << QString::number( labID )
         << QString::number( instrumentID )
         << QString::number( operatorID )
         << QString::number( rotorID )
         << QString::number( calibrationID )
         << expType
         << opticalSystem
         << RIxmlEscaped
         << runTemp
         << label
         << comments
         << protocolGUID;

      saveStatus = db->statusQuery( q );
   }

   else if ( status == US_DB2::NOROWS )
   {
      // Create new experiment entry
      q.clear();
      q  << "new_experiment"
         << expGUID
         << QString::number( project.projectID )
         << runID
         << QString::number( labID )
         << QString::number( instrumentID )
         << QString::number( operatorID )
         << QString::number( rotorID )
         << QString::number( calibrationID )
         << expType
         << opticalSystem
         << RIxml
         << runTemp
         << label
         << comments
         << protocolGUID
         << QString::number( US_Settings::us_inv_ID() );

      saveStatus = db->statusQuery( q );
      expID = db->lastInsertID();
   }

   if ( expID == 0 )      // double check
   {
      qDebug() << "Error saving experiment: " << saveStatus
               << " " << db->lastError();
      return saveStatus;
   }

   // Let's get some info after db update
   QString idExp = QString::number( expID );
   q.clear();
   q << "get_experiment_info" << idExp;
   db->query( q );
   db->next();

   date = db->value( 12 ).toString();

   // If multispeed, add speed steps to the database
   q.clear();
   q << "delete_speedsteps" << idExp;   // Delete any records for experiment
   saveStatus = db->statusQuery( q );
   if ( saveStatus != 0 )
   {
      qDebug() << "Delete_SpeedSteps" << saveStatus << db->lastError();
   }
qDebug() << "Exp:svToDB: delss status" << saveStatus << db->lastError();

qDebug() << "Exp:svToDB:  ss count" << speedsteps.count() << "expID" << expID;
   for ( int jj = 0; jj < speedsteps.count(); jj++ )
   {
qDebug() << "Exp:svToDB:   jj" << jj << "expID" << expID;
int ssstat=
      US_SimulationParameters::speedstepToDB( db, expID, &speedsteps[ jj ] );
qDebug() << "Exp:svToDB:     ssstat=" << ssstat;
   }

   return US_DB2::OK;
}

//Same as above but for AutoFlow with ExpData.invID set as owner
int US_Experiment::saveToDB_auto( bool update, US_DB2* db,
				  QVector< SP_SPEEDPROFILE >& speedsteps, int invID_passed )
{
qDebug() << "Exp:svToDB: update" << update << "ss-count" << speedsteps.count();
   // Let's see if the project is in the db already

 int status = project.saveToDB_auto( invID_passed, db );    // ALEXEY -- invID_passed must be used !!!
 qDebug() << "Exp:svToDB: projsv status(+NO_PR)"
	  << status << US_DB2::NO_PROJECT
	  << "expID" << expID
	  << invID_passed;

   if ( status == US_DB2::NO_PROJECT )
      return status;

   else if ( status != US_DB2::OK )
      return status;

   // Get the RI information, if appropriate
   QByteArray RIxml;
   QByteArray RIxmlEscaped;
   createRIXml( RIxml );
   //unsigned long length = db->mysqlEscapeString( RIxmlEscaped, RIxml, RIxml.size() );
   db->mysqlEscapeString( RIxmlEscaped, RIxml, RIxml.size() );

   // Check for experiment runID in database
   int saveStatus = 0;
   QStringList q;
   status = checkRunID_auto( invID_passed, db );
   if ( status == US_DB2::OK && ! update )
   {
      // Then the runID exists already, and we're not updating
      return US_DB2::DUPFIELD;
   }

   if ( status == US_DB2::OK )
   {
      // It's ok to update the existing experiment entry
      q.clear();
      q  << "update_experiment"
         << QString::number( expID )
         << QString( expGUID )
         << QString::number( project.projectID )
         << runID
         << QString::number( labID )
         << QString::number( instrumentID )
         << QString::number( operatorID )
         << QString::number( rotorID )
         << QString::number( calibrationID )
         << expType
         << opticalSystem
         << RIxmlEscaped
         << runTemp
         << label
         << comments
         << protocolGUID;

      saveStatus = db->statusQuery( q );
   }

   else if ( status == US_DB2::NOROWS )
   {
      // Create new experiment entry
      q.clear();
      q  << "new_experiment"
         << expGUID
         << QString::number( project.projectID )
         << runID
         << QString::number( labID )
         << QString::number( instrumentID )
         << QString::number( operatorID )
         << QString::number( rotorID )
         << QString::number( calibrationID )
         << expType
         << opticalSystem
         << RIxml
         << runTemp
         << label
         << comments
         << protocolGUID
         << QString::number( invID_passed ); //ALEXEY: passed invID, NOT the US_Settings ::us_inv_ID()

      saveStatus = db->statusQuery( q );
      expID = db->lastInsertID();
   }

   if ( expID == 0 )      // double check
   {
      qDebug() << "Error saving experiment: " << saveStatus
               << " " << db->lastError();
      return saveStatus;
   }

   // Let's get some info after db update
   QString idExp = QString::number( expID );
   q.clear();
   q << "get_experiment_info" << idExp;
   db->query( q );
   db->next();

   date = db->value( 12 ).toString();

   // If multispeed, add speed steps to the database
   q.clear();
   q << "delete_speedsteps" << idExp;   // Delete any records for experiment
   saveStatus = db->statusQuery( q );
   if ( saveStatus != 0 )
   {
      qDebug() << "Delete_SpeedSteps" << saveStatus << db->lastError();
   }
qDebug() << "Exp:svToDB: delss status" << saveStatus << db->lastError();

qDebug() << "Exp:svToDB:  ss count" << speedsteps.count() << "expID" << expID;
   for ( int jj = 0; jj < speedsteps.count(); jj++ )
   {
qDebug() << "Exp:svToDB:   jj" << jj << "expID" << expID;
int ssstat=
      US_SimulationParameters::speedstepToDB( db, expID, &speedsteps[ jj ] );
qDebug() << "Exp:svToDB:     ssstat=" << ssstat;
   }

   return US_DB2::OK;
}


// Function to read an experiment from DB
int US_Experiment::readFromDB( QString runID, US_DB2* db,
                               QVector< SP_SPEEDPROFILE >& speedsteps )
{
   QStringList q( "get_experiment_info_by_runID" );
   q << runID
     << QString::number( US_Settings::us_inv_ID() );
   db->query( q );

   QByteArray xmlFile;

   if ( db->next() )
   {
      this->runID        = runID;
      project.projectID  = db->value( 0 ).toInt();
      expID              = db->value( 1 ).toInt();
      expGUID            = db->value( 2 ).toString();
      labID              = db->value( 3 ).toInt();
      instrumentID       = db->value( 4 ).toInt();
      operatorID         = db->value( 5 ).toInt();
      rotorID            = db->value( 6 ).toInt();
      calibrationID      = db->value( 7 ).toInt();
      expType            = db->value( 8 ).toString();
      runTemp            = db->value( 9 ).toString();
      label              = db->value( 10 ).toString();
      comments           = db->value( 11 ).toString();
      protocolGUID       = db->value( 12 ).toString();
      date               = db->value( 13 ).toString();
      invID              = db->value( 14 ).toInt();
      opticalSystem      = db->value( 15 ).toString().toLatin1();
      xmlFile            = db->value( 16 ).toString().toLatin1();
qDebug() << "Exp:rdDB: ExpInfRun: xmlFile size" << xmlFile.size();
   }

   else if ( db->lastErrno() == US_DB2::NOROWS )
      return US_DB2::NO_EXPERIMENT;

   else
      return( db->lastErrno() );

   // Get the rest of the info we need
   q.clear();
   q  << QString( "get_person_info" )
      << QString::number( invID );
   db->query( q );
   if ( db->next() )
      invGUID = db->value( 9 ).toString();

   project.readFromDB( project.projectID, db );

   // Hardware info
   operatorGUID = QString( "" );
   q.clear();
   q  << QString( "get_person_info" )
      << QString::number( operatorID );
   db->query( q );
   if ( db->next() )
      operatorGUID   = db->value( 9 ).toString();

   instrumentSerial = QString( "" );
   q.clear();
   q << QString( "get_instrument_info" )
     << QString::number( instrumentID );
   db->query( q );
   if ( db->next() )
      instrumentSerial = db->value( 1 ).toString();

   rotorGUID = QString( "" );
   q.clear();
   q << QString( "get_rotor_info" )
     << QString::number( rotorID );
   db->query( q );
   if ( db->next() )
   {
      rotorGUID   = db->value( 0 ).toString();
      rotorName   = db->value( 1 ).toString();
      rotorSerial = db->value( 2 ).toString();
   }

   if ( calibrationID == 0 )     // In this case, get the first one
   {
      q.clear();
      q << QString( "get_rotor_calibration_profiles" )
        << QString::number( rotorID );
      db->query( q );
      if ( db->next() )
         calibrationID = db->value( 0 ).toInt();
   }

   // Now get more calibration info
   q.clear();
   q << QString( "get_rotor_calibration_info" )
     << QString::number( calibrationID );
   db->query( q );
   if ( db->next() )
   {
      rotorCoeff1  = db->value( 4 ).toString().toFloat();
      rotorCoeff2  = db->value( 5 ).toString().toFloat();
      QStringList dateParts = db->value( 7 ).toString().split( " " );
      rotorUpdated = QDate::fromString( dateParts[ 0 ], "yyyy-MM-dd" );
   }

   // If this is RI data, get the intensity profile
   RIProfile.clear();
   if ( opticalSystem == "RI" )
   {
      // If we have a profile from DB, make sure there is a local copy
qDebug() << "Exp:rdDB: xmlFile size" << xmlFile.size();
      if ( xmlFile.size() > 0 )
      {
         QDir    readDir( US_Settings::resultDir() );
         QString RIPfname = readDir.absolutePath() + "/" + runID + "/"
                            + runID + ".RIProfile.xml";
qDebug() << "Exp:rdDB: RIPfname" << RIPfname;

         QFile rfo( RIPfname );
qDebug() << "Exp:rdDB: rfo exists" << rfo.exists();
qDebug() << "Exp:rdDB: rfo size" << rfo.size();

         if ( ! rfo.exists()  ||  rfo.size() == 0 )
         {  // Need to make a local copy
            if ( rfo.open( QIODevice::WriteOnly ) )
            {
qDebug() << "Exp:rdDB: rfo OPENED  xmlFile size" << xmlFile.size();
               rfo.write( xmlFile );
               rfo.close();
            }
         }
      }

      int status = importRIxml( xmlFile );
qDebug() << "Exp:rdDB: importRIxml status" << status;

      if ( status != US_Convert::OK )
      {
         // This isn't exactly an error; data is read but we don't have an
         // intensity profile
         RIProfile.clear();
         return US_DB2::DBERROR;
      }

   }

   // If this is multi-speed, get speed steps for the experiment
   US_SimulationParameters::speedstepsFromDB( db, expID, speedsteps );
qDebug() << "Exp:rdDB: speedsteps size expID" << speedsteps.size() << expID;
if(speedsteps.size()>0)
 qDebug() << "Exp:rdDB:  ss0 rspeed avgspeed" << speedsteps[0].rotorspeed
  << speedsteps[0].avg_speed;

   return US_DB2::OK;
}

int US_Experiment::saveToDisk(
    QList< US_Convert::TripleInfo >& triples,
    QString runType,
    QString runID,
    QString dirname,
    QVector< SP_SPEEDPROFILE >& speedsteps )
{ 
   QRegExp rx( "^[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}$" );


   if ( this->expGUID.isEmpty() || ! rx.exactMatch( this->expGUID ) )
      this->expGUID = US_Util::new_guid();

   if ( dirname.right( 1 ) != "/" ) dirname += "/"; // Ensure trailing /
   QString writeFile = runID      + "." 
                     + runType    + ".xml";
   QFile file( dirname + writeFile );
   if ( !file.open( QIODevice::WriteOnly | QIODevice::Text) )
      return( US_Convert::CANTOPEN );


qDebug() << "  EsTD: writeFile" << writeFile;
   QXmlStreamWriter xml;
   xml.setDevice( &file );
   xml.setAutoFormatting( true );

   xml.writeStartDocument();
   xml.writeDTD("<!DOCTYPE US_Scandata>");
   xml.writeStartElement("US_Scandata");
   xml.writeAttribute("version", "1.0");

   // elements
   xml.writeStartElement( "experiment" );
   xml.writeAttribute   ( "id",   QString::number( this->expID ) );
   xml.writeAttribute   ( "guid", this->expGUID );
   xml.writeAttribute   ( "type", this->expType );
   xml.writeAttribute   ( "runID", this->runID );

   xml.writeStartElement( "investigator" );
   xml.writeAttribute   ( "id", QString::number( this->invID ) );
   xml.writeAttribute   ( "guid", this->invGUID );
   xml.writeEndElement  ();
      
   xml.writeStartElement( "name" );
   xml.writeAttribute   ( "value", this->name );
   xml.writeEndElement  ();
      
   xml.writeStartElement( "project" );
   xml.writeAttribute   ( "id", QString::number( this->project.projectID ) );
   xml.writeAttribute   ( "guid", this->project.projectGUID );
   xml.writeAttribute   ( "desc", this->project.projectDesc );
   xml.writeEndElement  ();
      
   xml.writeStartElement( "lab" );
   xml.writeAttribute   ( "id",   QString::number( this->labID   ) );
   xml.writeEndElement  ();
      
   xml.writeStartElement( "instrument" );
   xml.writeAttribute   ( "id",     QString::number( this->instrumentID ) );
   xml.writeAttribute   ( "serial", this->instrumentSerial );
   xml.writeEndElement  ();
      
   xml.writeStartElement( "operator" );
   xml.writeAttribute   ( "id", QString::number( this->operatorID ) );
   xml.writeAttribute   ( "guid", this->operatorGUID );
   xml.writeEndElement  ();

   xml.writeStartElement( "rotor" );
   xml.writeAttribute   ( "id",     QString::number( this->rotorID   ) );
   xml.writeAttribute   ( "guid",   this->rotorGUID );
   xml.writeAttribute   ( "serial", this->rotorSerial );
   xml.writeAttribute   ( "name", this->rotorName );
   xml.writeEndElement  ();

   xml.writeStartElement( "calibration" );
   xml.writeAttribute   ( "id",     QString::number( this->calibrationID ) );
   xml.writeAttribute   ( "coeff1", QString::number( this->rotorCoeff1   ) );
   xml.writeAttribute   ( "coeff2", QString::number( this->rotorCoeff2   ) );
   xml.writeAttribute( "date", this->rotorUpdated.toString( "yyyy-MM-dd" ) );
   xml.writeEndElement  ();
   int     psolID    = -1;
   QString psolGUID  = "";
   QString psolDesc  = "";
qDebug() << "  EsTD: triples loop" << triples.size();

   // loop through the following for c/c/w combinations
   for ( int trx = 0; trx < triples.size(); trx++ )
   {
      US_Convert::TripleInfo* trp = &triples[ trx ];
      if ( trp->excluded ) continue;

      QString triple     = trp->tripleDesc;
      QStringList parts  = triple.split(" / ");

      QString cell       = parts[ 0 ];
      QString channel    = parts[ 1 ];
      QString wl         = parts[ 2 ];

      QString uuidc = US_Util::uuid_unparse( (unsigned char*)trp->tripleGUID );

      xml.writeStartElement( "dataset" );
      xml.writeAttribute   ( "id",      QString::number( trp->tripleID ) );
      xml.writeAttribute   ( "guid",    uuidc );
      xml.writeAttribute   ( "cell",    cell );
      xml.writeAttribute   ( "channel", channel );

      if ( runType == "WA" )
         xml.writeAttribute( "radius", wl );

      else
         xml.writeAttribute( "wavelength", wl );

      xml.writeStartElement( "centerpiece" );
      xml.writeAttribute   ( "id", QString::number( trp->centerpiece ) );
      xml.writeEndElement  ();

      int     csolID   = trp->solution.solutionID;
      QString csolGUID = trp->solution.solutionGUID;
      QString csolDesc = trp->solution.solutionDesc;

      if ( csolID == psolID  ||  ( csolID < 0 && csolGUID == psolGUID ) )
      {
         csolID           = psolID;
         csolGUID         = psolGUID;
         csolDesc         = psolDesc;
      }

      psolID           = csolID;
      psolGUID         = csolGUID;
      psolDesc         = csolDesc;
      xml.writeStartElement( "solution" );
      xml.writeAttribute   ( "id",   QString::number( csolID ) );
      xml.writeAttribute   ( "guid", csolGUID );
      xml.writeAttribute   ( "desc", csolDesc );
      xml.writeEndElement  ();

      xml.writeEndElement  ();
   }

   for ( int jj = 0; jj < speedsteps.count(); jj++ )
      US_SimulationParameters::speedstepToXml( xml, &speedsteps[ jj ] );

   xml.writeStartElement( "opticalSystem" );
   xml.writeAttribute   ( "value", this->opticalSystem );
   xml.writeEndElement  ();

   xml.writeStartElement( "date" );
   xml.writeAttribute   ( "value", this->date );
   xml.writeEndElement  ();

   xml.writeStartElement( "runTemp" );
   xml.writeAttribute   ( "value", this->runTemp );
   xml.writeEndElement  ();

   xml.writeTextElement ( "label", this->label );
   xml.writeTextElement ( "comments", this->comments );
   xml.writeTextElement ( "protocolGUID", this->protocolGUID );

   xml.writeEndElement(); // US_Scandata
   xml.writeEndDocument();

   // Make sure the project is saved to disk too
qDebug() << "  EsTD: call proj saveToDisk";
   this->project.saveToDisk();

   return( US_Convert::OK );
}

int US_Experiment::readFromDisk( 
    QList< US_Convert::TripleInfo >& triples,
    QString runType,
    QString runID,
    QString dirname )
{
   // First figure out the xml file name, and try to open it
   QString filename = runID      + "." 
                    + runType    + ".xml";

   QFile f( dirname + filename );
   if ( ! f.open( QIODevice::ReadOnly ) ) return US_Convert::CANTOPEN;
   QTextStream ds( &f );

   QXmlStreamReader xml( &f );

   while ( ! xml.atEnd() )
   {
      xml.readNext();

      if ( xml.isStartElement() )
      {
         if ( xml.name() == "experiment" )
         {
            QXmlStreamAttributes a = xml.attributes();
            this->expID          = a.value("id").toString().toInt();
            this->expGUID        = a.value( "guid" ).toString();
            this->expType        = a.value( "type" ).toString();
            this->runID          = a.value( "runID" ).toString();
            readExperiment ( xml, triples, runType, runID );
         }
      }
   }

   bool error = xml.hasError();
qDebug() << "readFromDisk() error" << error;
   f.close();

   if ( error ) return US_Convert::BADXML;

   return US_Convert::OK;
}

void US_Experiment::readExperiment( 
     QXmlStreamReader& xml, 
     QList< US_Convert::TripleInfo >& triples,
     QString runType,
     QString runID )
{
   while ( ! xml.atEnd() )
   {
      xml.readNext();

      if ( xml.isEndElement()  &&  xml.name() == "experiment" ) return;

      if ( xml.isStartElement() )
      {
         if ( xml.name() == "investigator" )
         {
            QXmlStreamAttributes a = xml.attributes();
            this->invID          = a.value( "id" ).toString().toInt();
            this->invGUID        = a.value( "guid" ).toString();
         }
 
         else if ( xml.name() == "name" )
         {
            QXmlStreamAttributes a = xml.attributes();
            this->name           = a.value( "value" ).toString();
         }

         else if ( xml.name() == "project" )
         {
            QXmlStreamAttributes a = xml.attributes();
            this->project.projectID      = a.value( "id"   ).toString().toInt();
            this->project.projectGUID    = a.value( "guid" ).toString();
            this->project.projectDesc    = a.value( "desc" ).toString();
         }
   
         else if ( xml.name() == "lab" )
         {
            QXmlStreamAttributes a = xml.attributes();
            this->labID    = a.value( "id" )  .toString().toInt();
         }
   
         else if ( xml.name() == "instrument" )
         {
            QXmlStreamAttributes a    = xml.attributes();
            this->instrumentID      = a.value( "id" )    .toString().toInt();
            this->instrumentSerial  = a.value( "serial" ).toString();
         }
   
         else if ( xml.name() == "operator" )
         {
            QXmlStreamAttributes a = xml.attributes();
            this->operatorID     = a.value( "id" ).toString().toInt();
            this->operatorGUID   = a.value( "guid" ).toString();
         }
   
         else if ( xml.name() == "rotor" )
         {
            QXmlStreamAttributes a = xml.attributes();
            this->rotorID        = a.value( "id"     ).toString().toInt();
            this->rotorGUID      = a.value( "guid"   ).toString();
            this->rotorSerial    = a.value( "serial" ).toString();
            this->rotorName      = a.value( "name" ).toString();
            this->calibrationID  = a.value( "calibrationID" ).toString().toInt();
         }
   
         else if ( xml.name() == "calibration" )
         {
            QXmlStreamAttributes a = xml.attributes();
            this->calibrationID      = a.value( "id"     ).toString().toInt();
            this->rotorCoeff1        = a.value( "coeff1" ).toString().toFloat();
            this->rotorCoeff2        = a.value( "coeff2" ).toString().toFloat();
            this->rotorUpdated       =
               QDate::fromString( a.value( "date" ).toString(), "yyyy-MM-dd" );
         }

         else if ( xml.name() == "dataset" )
         {
            QXmlStreamAttributes a = xml.attributes();
            QString cell           = a.value( "cell" ).toString();
            QString channel        = a.value( "channel" ).toString();

            QString wl;
            if ( runType == "WA" )
               wl                  = a.value( "radius" ).toString();

            else
               wl                  = a.value( "wavelength" ).toString();
   
            // Find the index of this triple
            QString triple         = cell + " / " + channel + " / " + wl;
            bool found             = false;
            int ndx                = 0;
            for ( int i = 0; i < triples.size(); i++ )
            {
               if ( triples[ i ].excluded ) continue;

               if ( triple == triples[ i ].tripleDesc )
               {
                  found = true;
                  ndx   = i;
                  break;
               }
            }

            if ( found )
            {
               triples[ ndx ].tripleID = a.value( "id" ).toString().toInt();
               QString uuidc = a.value( "guid" ).toString();
               US_Util::uuid_parse( uuidc,
                  (unsigned char*) triples[ ndx ].tripleGUID );

               if ( runType == "WA" )
               {
                   double r       = wl.toDouble() * 1000.0;
                   QString radius = QString::number( qRound( r ) );
                   triples[ ndx ].tripleFilename = runID      + "." 
                                                 + runType    + "." 
                                                 + cell       + "." 
                                                 + channel    + "." 
                                                 + radius     + ".auc";
               }
               
               else
               {
                   triples[ ndx ].tripleFilename = runID      + "." 
                                                 + runType    + "." 
                                                 + cell       + "." 
                                                 + channel    + "." 
                                                 + wl         + ".auc";
               }

               triples[ ndx ].excluded       = false;

               readDataset( xml, triples[ ndx ] );
            }
         }

         else if ( xml.name() == "opticalSystem" )
         {
            QXmlStreamAttributes a = xml.attributes();
            this->opticalSystem  = a.value( "value" ).toString().toLatin1();
         }

         else if ( xml.name() == "date" )
         {
            QXmlStreamAttributes a = xml.attributes();
            this->date           = a.value( "value" ).toString();
         }

         else if ( xml.name() == "runTemp" )
         {
            QXmlStreamAttributes a = xml.attributes();
            this->runTemp        = a.value( "value" ).toString();
         }

         else if ( xml.name() == "label" )
         {
            xml.readNext();
            this->label = xml.text().toString();
         }

         else if ( xml.name() == "comments" )
         {
            xml.readNext();
            this->comments = xml.text().toString();
         }

         else if ( xml.name() == "protocolGUID" )
         {
            xml.readNext();
            this->protocolGUID = xml.text().toString();
         }
      }
   }
}

void US_Experiment::readDataset( QXmlStreamReader& xml,
                                 US_Convert::TripleInfo& triple )
{
   while ( ! xml.atEnd() )
   {
      xml.readNext();

      if ( xml.isEndElement()  &&  xml.name() == "dataset" ) return;

      if ( xml.isStartElement() )
      {
         if ( xml.name() == "centerpiece" )
         {
            QXmlStreamAttributes a = xml.attributes();
            triple.centerpiece     = a.value( "id" ).toString().toInt();
         }
 
         else if ( xml.name() == "solution" )
         {
            QXmlStreamAttributes a = xml.attributes();
            triple.solution.solutionID   = a.value( "id"   ).toString().toInt();
            triple.solution.solutionGUID = a.value( "guid" ).toString();
            triple.solution.solutionDesc = a.value( "desc" ).toString();
         }

      }
   }
}

int US_Experiment::saveRIDisk( 
    QString runID,
    QString dirname )
{ 
   if ( dirname.right( 1 ) != "/" ) dirname += "/"; // Ensure trailing /
   QString writeFile = runID      + "." 
                     + "RIProfile.xml";
   QFile file( dirname + writeFile );
   if ( !file.open( QIODevice::WriteOnly | QIODevice::Text) )
      return( US_Convert::CANTOPEN );

   // Get the RI information
   QByteArray RIxml;
   createRIXml( RIxml );

   QTextStream out( &file );
   out << RIxml;

   return( US_Convert::OK );
}

int US_Experiment::readRIDisk( 
    QString runID,
    QString dirname )
{
   // First figure out the xml file name, and try to open it
   QString filename = runID      + "." 
                    + "RIProfile.xml";

   this->RIProfile.clear();
   this->RIwvlns  .clear();
   this->RI_nscans  = 0;
   this->RI_nwvlns  = 0;

   QFile f( dirname + filename );
   if ( ! f.open( QIODevice::ReadOnly ) ) return US_Convert::CANTOPEN;

   QTextStream ds( &f );
   QXmlStreamReader xml( &f );

   while ( ! xml.atEnd() )
   {
      xml.readNext();

      if ( xml.isStartElement() )
      {
         QXmlStreamAttributes a = xml.attributes();
         QStringRef xname       = xml.name();

         if ( xname == "RI" )
         {
            double value     = a.value( "value"       ).toString().toDouble();
            this->RIProfile << value;
         }

         else if ( xname == "WVL" )
         {
            int    iwavl     = a.value( "value"       ).toString().toInt();
            this->RIwvlns   << iwavl;
         }

         else if ( xname == "WVPro" )
         {
            this->RI_nscans  = a.value( "scans"       ).toString().toInt();
            this->RI_nwvlns  = a.value( "wavelengths" ).toString().toInt();
         }
      }
   }

   bool error = xml.hasError();
   f.close();
qDebug() << "readRIDisk() error" << error;

   if ( error ) return US_Convert::BADXML;

   return US_Convert::OK;
}

void US_Experiment::createRIXml( QByteArray& str )
{
   if ( this->opticalSystem != "RI" )
   {
      str = QByteArray( "" );
      return ;
   }

   if ( this->RIProfile.size() == 0 )
   {
      str = QByteArray( "" );
      return ;
   }

   // Create the RI xml directly, and write it to a buffer
   // QByteArray xmlFile;
   // QBuffer buffer( &xmlFile );
   QBuffer buffer( &str );
   buffer.open( QIODevice::WriteOnly );

   QXmlStreamWriter xml;
   xml.setDevice( &buffer );
   xml.setAutoFormatting( true );

   xml.writeStartDocument();
   xml.writeDTD("<!DOCTYPE US_RIProfile>");
   xml.writeStartElement("US_RIProfile");
   xml.writeAttribute("version", "1.0");

   // Loop through the intensity profile values
   foreach ( double value, this->RIProfile )
   {
      xml.writeStartElement( "RI" );
      xml.writeAttribute   ( "value", QString::number( value ) );
      xml.writeEndElement  ();
   }

   if ( this->RIwvlns.size() > 1 )
   {
      int nscans = this->RI_nscans;
      int nwvlns = this->RI_nwvlns;

      for ( int jj = 0; jj < nwvlns; jj++ )
      {
         int iwavl  = this->RIwvlns[ jj ];
         xml.writeStartElement( "WVL" );
         xml.writeAttribute   ( "value",    QString::number( iwavl  ) );
         xml.writeEndElement  ();
      }

      xml.writeStartElement( "WVPro" );
      xml.writeAttribute   ( "scans",       QString::number( nscans ) );
      xml.writeAttribute   ( "wavelengths", QString::number( nwvlns ) );
      xml.writeEndElement  ();
   }

   xml.writeEndElement(); // US_RIProfile
   xml.writeEndDocument();

   buffer.close();
}

int US_Experiment::importRIxml( QByteArray& str )
{
   QBuffer buffer( &str );
   buffer.open( QIODevice::ReadOnly );

   QXmlStreamReader xml( &buffer );

   this->RIProfile.clear();
   this->RIwvlns  .clear();
   this->RI_nscans   = 0;
   this->RI_nwvlns   = 0;

   while ( ! xml.atEnd() )
   {
      xml.readNext();

      if ( xml.isStartElement() )
      {
         QXmlStreamAttributes a = xml.attributes();
         QStringRef xname       = xml.name();

         if ( xname == "RI" )
         {
            double value     = a.value( "value"       ).toString().toDouble();
            this->RIProfile << value;
         }

         else if ( xname == "WVL" )
         {
            int    iwavl     = a.value( "value"       ).toString().toInt();
            this->RIwvlns << iwavl;
         }

         else if ( xname == "WVPro" )
         {
            this->RI_nscans  = a.value( "scans"       ).toString().toInt();
            this->RI_nwvlns  = a.value( "wavelengths" ).toString().toInt();
         }
      }
   }

   bool error = xml.hasError();
   buffer.close();
qDebug() << "importRIxml() error" << error;

   if ( error ) return US_Convert::BADXML;

   return US_Convert::OK ;
}

// Delete pcsa_modelrecs for the current experiment run
bool US_Experiment::deleteRunPcsaMrecs( US_DB2* db, const QString invID,
                                                    const QString runID )
{
   bool is_ok         = true;

   if ( db != NULL )
   {  // Delete modelrecs from the database
      QStringList mrcIDs;
      QStringList edtIDs;
      QStringList qry;

      // Build a list of edit IDs for the run
      qry.clear();
      qry << "get_edit_desc_by_runID" << invID << runID;
      db->query( qry );
      int status          = db->lastErrno();
qDebug() << "Exp:delMR: runID" << runID << "invID" << invID << "qry:" << qry << status << db->lastError();
      if ( status == US_DB2::NOROWS )
      {  // If no edits for run, no modelrecs, so return
         return is_ok;
      }
      else if ( status != US_DB2::OK )
      {  // Return if query had an error
qDebug() << qry << status << db->lastError();
         return false;
      }
      while( db->next() )
      {
         edtIDs << db->value( 0 ).toString();
      }
qDebug() << "Exp:delMR:  n(edtIDs)" << edtIDs.count();

      // Build a list of pcsa_modelrec IDs for run's edits
      qry.clear();
      qry << "get_mrecs_desc" << invID;
      db->query( qry );
      status              = db->lastErrno();
qDebug() << "Exp:delMR: qry:" << qry << status << db->lastError();
      if ( status != US_DB2::OK )
      {
qDebug() << qry << status << db->lastError();
         return false;
      }
      while( db->next() )
      {
         QString mrecID      = db->value( 0 ).toString();
         QString editID      = db->value( 2 ).toString();
         if ( edtIDs.contains( editID ) )
         {  // For an edit in the list, add this mrecID to the list
            mrcIDs << mrecID;
         }
      }

      // Delete all pcsa_modelrec entries in our run list
      int nredts          = edtIDs.count();
      int nrmrcs          = mrcIDs.count();
qDebug() << "Exp:delMR:  nredts nrmrcs" << nredts << nrmrcs;
if(nredts>0)
qDebug() << "Exp:delMR:   edtIDs[0]" << edtIDs[0];
if(nrmrcs>0)
qDebug() << "Exp:delMR:   mrcIDs[0]" << mrcIDs[0];
      int kmrdel          = 0;
      for ( int ii = 0; ii < nrmrcs; ii++ )
      {
         QString mrecID      = mrcIDs[ ii ];
         qry.clear();
         qry << "delete_mrecs" << mrecID;
         status              = db->statusQuery( qry );
qDebug() << "Exp:delMR:    ii" << ii << "mrecID" << mrecID << "stat" << status;
         if ( status == US_DB2::OK )
         {
            kmrdel++;
         }
         else
         {
qDebug() << "Exp:delMR:" << qry << status << db->lastError();
         }
      }
qDebug() << "Exp:delMR: nredts nrmrcs kmrdel" << nredts << nrmrcs << kmrdel;
      is_ok               = ( kmrdel == nrmrcs );
   }

   else
   {  // Delete modelrecs from local disk
      QStringList mrPaths;
      QString rpath       = US_Settings::resultDir() + "/" + runID + "/";
      QDir rdir( rpath );
      QStringList filter( "pcsa-mrs*.xml" );
      QStringList mrNames = rdir.entryList( filter, QDir::Files, QDir::Name );
      for ( int ii = 0; ii < mrNames.count(); ii++ )
      {
         QString mrPath      = rpath + mrNames[ ii ];
         QFile filemr( mrPath );
         if ( filemr.exists() )
         {
            if ( ! filemr.remove() )
            {
qDebug() << "Exp:delMR: remove error: " << mrPath;
               is_ok               = false;
            }
         }
      }
   }

qDebug() << "Exp:delMR: is_ok" << is_ok;
   return is_ok;
}

// Zero out all data structures
void US_Experiment::clear( void )
{
   invID              = US_Settings::us_inv_ID();
   invGUID            = QString( "" );
   name               = US_Settings::us_inv_name();
   expID              = 0;
   expGUID            = QString( "" );
   project.clear();
   runID              = QString( "" );
   labID              = 0;
   instrumentID       = 0;
   instrumentSerial   = QString( "" );
   operatorID         = 0;
   operatorGUID       = QString( "" );
   rotorID            = 0;
   calibrationID      = 0;
   rotorCoeff1        = 0.0;
   rotorCoeff2        = 0.0;
   rotorGUID          = QString( "" );
   rotorSerial        = QString( "" );
   rotorName          = QString( "" );
   expType            = QString( "" );
   opticalSystem      = QByteArray( "  " );
   rpms.clear();
   runTemp            = QString( "" );
   label              = QString( "" );
   comments           = QString( "" );
   protocolGUID       = QString( "" );
   date               = QString( "" );
   syncOK             = false;
   RIProfile.clear();

   rpms.clear();
}

void US_Experiment::show( void )
{
   QString syncOK_text = ( syncOK ) ? "true" : "false";

   qDebug() << "invID        = " << invID << '\n'
            << "invGUID      = " << invGUID << '\n'
            << "name         = " << name << '\n'
            << "expID        = " << expID << '\n'
            << "expGUID      = " << expGUID << '\n'
            << "projectID    = " << project.projectID << '\n'
            << "projectGUID  = " << project.projectGUID << '\n'
            << "projectDesc  = " << project.projectDesc << '\n'
            << "runID        = " << runID << '\n'
            << "labID        = " << labID << '\n'
            << "instrumentID = " << instrumentID << '\n'
            << "instrumentSerial = " << instrumentSerial << '\n'
            << "operatorID   = " << operatorID << '\n'
            << "operatorGUID = " << operatorGUID << '\n'
            << "rotorID      = " << rotorID << '\n'
            << "rotorGUID    = " << rotorGUID << '\n'
            << "rotorSerial  = " << rotorSerial << '\n'
            << "rotorName    = " << rotorName << '\n'
            << "calibrationID = " << calibrationID << '\n'
            << "rotorCoeff1  = " << rotorCoeff1 << '\n'
            << "rotorCoeff2  = " << rotorCoeff2 << '\n'
            << "rotorUpdated = " << rotorUpdated.toString( "yyyy-MM-dd" ) << '\n'
            << "expType      = " << expType << '\n'
            << "opticalSystem = " << opticalSystem << '\n'
            << "runTemp      = " << runTemp << '\n'
            << "label        = " << label << '\n'
            << "comments     = " << comments << '\n'
            << "protocolGUID = " << protocolGUID << '\n'
            << "date         = " << date << '\n'
            << "syncOK       = " << syncOK_text << '\n';

   for ( int i = 0; i < rpms.size(); i++ )
   {
      qDebug() << "i = " << i ;
      qDebug() << "rpm = " << rpms[ i ];
   }

   qDebug() << "RI Profile data";
   for ( int i = 0; i < RIProfile.size(); i++ )
      qDebug() << RIProfile[ i ];
}

