//! \file us_experiment.cpp

#include <QtCore>

#include "us_settings.h"
#include "us_db2.h"
#include "us_passwd.h"
#include "us_investigator.h"
#include "us_experiment.h"

US_Experiment::US_Experiment( void )
{
   // Experiment types
   experimentTypes.clear();
   experimentTypes << "Velocity"
                   << "Equilibrium"
                   << "Diffusion"
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

int US_Experiment::saveToDB( bool update, US_DB2* db )
{
   // Let's see if the project is in the db already
   int status = project.saveToDB( db );
   if ( status == US_DB2::NO_PROJECT )
      return status;

   else if ( status != US_DB2::OK )
      return status;

   // Check for experiment runID in database
   QStringList q;
   status = checkRunID( db );
   if ( status == US_DB2::OK && update )
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
         << runTemp
         << label
         << comments
         << centrifugeProtocol;

      db->statusQuery( q );
   }

   else if ( status == US_DB2::OK && ! update )
   {
      // Then the runID exists, but we have to leave it alone
      return US_DB2::DUPFIELD;
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
         << runTemp
         << label
         << comments
         << centrifugeProtocol
         << QString::number( US_Settings::us_inv_ID() );

      db->statusQuery( q );
      expID = db->lastInsertID();
   }

   else     // unspecified error
   {
      qDebug() << "MySQL error: " << db->lastError();
      return status;
   }

   if ( expID == 0 )      // double check
      return US_DB2::NO_EXPERIMENT;

   // Let's get some info after db update
   q.clear();
   q << "get_experiment_info"
     << QString::number( expID );
   db->query( q );
   db->next();

   date = db->value( 12 ).toString();

   return US_DB2::OK;
}

// Function to read the ExperimentInfo structure from DB
int US_Experiment::readSecondaryInfoDB( US_DB2* db )
{
   // Investigator info
   invID = US_Settings::us_inv_ID();
   name  = US_Settings::us_inv_name();
   QStringList q( "get_person_info" );
   q << QString::number( invID );
   db->query( q );
   if ( db->next() )
   {
      invGUID   = db->value( 9 ).toString();
   }

   // Project info
   project.projectGUID = QString( "" );
   project.projectDesc = QString( "" );
   q.clear();
   q << "get_project_info" 
     << QString::number( project.projectID );
   db->query( q );
   if ( db->next() )
   {
      project.projectGUID   = db->value( 1  ).toString();
      project.projectDesc   = db->value( 10 ).toString();
   }

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

   return US_DB2::OK;
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
   centrifugeProtocol = QString( "" );
   date               = QString( "" );
   syncOK             = false;

   rpms.clear();
}

US_Experiment& US_Experiment::operator=( const US_Experiment& rhs )
{

   if ( this != &rhs )            // Guard against self assignment
   {
      clear();

      invID         = rhs.invID;
      invGUID       = rhs.invGUID;
      name          = rhs.name;
      expID         = rhs.expID;
      expGUID       = rhs.expGUID;
      project.clear();
      runID         = rhs.runID;
      labID         = rhs.labID;
      instrumentID  = rhs.instrumentID;
      instrumentSerial  = rhs.instrumentSerial;
      operatorID    = rhs.operatorID;
      rotorID       = rhs.rotorID;
      calibrationID = rhs.calibrationID;
      rotorCoeff1   = rhs.rotorCoeff1;
      rotorCoeff2   = rhs.rotorCoeff2;
      rotorGUID     = rhs.rotorGUID;
      rotorSerial   = rhs.rotorSerial;
      rotorName     = rhs.rotorName;
      rotorUpdated  = rhs.rotorUpdated;
      expType       = rhs.expType;
      opticalSystem = rhs.opticalSystem;
      runTemp       = rhs.runTemp;
      label         = rhs.label;
      comments      = rhs.comments;
      centrifugeProtocol = rhs.centrifugeProtocol;
      date          = rhs.date;
      syncOK        = rhs.syncOK;

      rpms.clear();
      for ( int i = 0; i < rhs.rpms.size(); i++ )
         rpms << rhs.rpms[ i ];

   }

   return *this;
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
            << "centrifugeProtocol = " << centrifugeProtocol << '\n'
            << "date         = " << date << '\n'
            << "syncOK       = " << syncOK_text << '\n';

   for ( int i = 0; i < rpms.size(); i++ )
   {
      qDebug() << "i = " << i ;
      qDebug() << "rpm = " << rpms[ i ];
   }

}
