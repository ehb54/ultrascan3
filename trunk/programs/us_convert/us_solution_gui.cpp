//! \file us_solutioninfo.cpp

#include <QtGui>

#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_passwd.h"
#include "us_db2.h"
#include "us_investigator.h"
#include "us_expinfo.h"
#include "us_solution_gui.h"
#include "us_convertio.h"

US_SolutionGui::US_SolutionGui( TripleInfo& dataIn ) :
   US_WidgetsDialog( 0, 0 ), tInfo( dataIn )
{
   reset();
}

void US_SolutionGui::reset( void )
{
}

// function to load what we can initially
// returns true if successful
bool US_SolutionGui::load( void )
{
   return( true );
}

/*
void US_SolutionGui::selectInvestigator( void )
{
   US_Investigator* inv_dialog = new US_Investigator( true );
   connect( inv_dialog, 
      SIGNAL( investigator_accepted( int, const QString&, const QString& ) ),
      SLOT  ( assignInvestigator   ( int, const QString&, const QString& ) ) );
   inv_dialog->exec();
}

void US_SolutionGui::assignInvestigator( int invID,
      const QString& , const QString& )
{
   getInvestigatorInfo( invID );

   if ( US_ConvertIO::checkRunID( expInfo.runID ) > 0 )
   {
      QMessageBox::information( this,
                tr( "Error" ),
                tr( "This run ID is already in the database" ) );
   }

   reset();
}

void US_SolutionGui::getInvestigatorInfo( int invID )
{
   US_Passwd pw;
   QString masterPW = pw.getPasswd();
   US_DB2 db( masterPW );

   if ( db.lastErrno() != US_DB2::OK )
   {
      connect_error( db.lastError() );
      return;
   }

   expInfo.invID = invID;                 // just to be sure
   QStringList q( "get_person_info" );
   q << QString::number( expInfo.invID );
   db.query( q );

   if ( db.next() )
   {
      expInfo.firstName = db.value( 0 ).toString();
      expInfo.lastName  = db.value( 1 ).toString();
      expInfo.invGUID   = db.value( 9 ).toString();
   }
   
}

*/
void US_SolutionGui::accept( void )
{
   US_Passwd pw;
   QString masterPW = pw.getPasswd();
   US_DB2 db( masterPW );

   if ( db.lastErrno() != US_DB2::OK )
   {
      connect_error( db.lastError() );
      return;
   }

   // Overwrite data directly from the form

   // First get the invID
/*
   QString invInfo = le_investigator->text();
   if ( invInfo.isEmpty() )
   {
      QMessageBox::information( this,
                tr( "Error" ),
                tr( "You must choose an investigator before accepting" ) );
      return;
   }

   QStringList components = invInfo.split( ")", QString::SkipEmptyParts );
   components = components[0].split( "(", QString::SkipEmptyParts );
   tInfo.invID = components.last().toInt();
*/

//   emit updateSolutionGuiSelection( expInfo );
   close();
}

void US_SolutionGui::cancel( void )
{
   tInfo.clear();

//   emit cancelSolutionGuiSelection();
   close();
}

void US_SolutionGui::connect_error( const QString& error )
{
   QMessageBox::warning( this, tr( "Connection Problem" ),
         tr( "Could not connect to databasee \n" ) + error );
}

// Initializations
US_SolutionGui::TripleInfo::TripleInfo()
{
   clear();
}

void US_SolutionGui::TripleInfo::clear( void )
{
   tripleID     = 0;
   tripleDesc   = QString( "" );
   excluded     = false;
   centerpiece  = 0;
   bufferID     = 0;
   bufferGUID   = "";
   bufferDesc   = "";
   analyteID    = 0;
   analyteGUID  = "";
   analyteDesc  = "";
   memset( tripleGUID, 0, 16 );
   tripleFilename = "";
   memset( solutionGUID, 0, 16 );
   description  = "";
   storageTemp  = 0;
   notes        = "";

   // Temporary, until we have a dialog
   description  = "Solution description";
   notes        = "Solution notes";
}

void US_SolutionGui::TripleInfo::show( void )
{
   qDebug() << "tripleID     = " << tripleID     << '\n'
            << "tripleDesc   = " << tripleDesc   << '\n'
            << "centerpiece  = " << centerpiece  << '\n'
            << "bufferID     = " << bufferID     << '\n'
            << "bufferGUID   = " << bufferGUID   << '\n'
            << "bufferDesc   = " << bufferDesc   << '\n'
            << "analyteID    = " << analyteID    << '\n'
            << "analyteGUID  = " << analyteGUID  << '\n'
            << "analyteDesc  = " << analyteDesc  << '\n'
            << "tripleGUID   = " << tripleGUID   << '\n'
            << "tripleFilename = " << tripleFilename  << '\n'
            << "solutionGUID = " << solutionGUID << '\n'
            << "description  = " << description  << '\n'
            << "storageTemp  = " << storageTemp  << '\n'
            << "notes        = " << notes        << '\n';
   if ( excluded ) qDebug() << "excluded";
}
