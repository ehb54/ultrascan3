// Failure injection at every write stage of US_ConvertIO::writeRawDataToDB().
//
// The writer takes an IUS_DB2* since the Phase 5 sweep, so a scripted stand-in
// can fail one stored procedure at a time and the caller's reporting and
// cleanup can be checked without a server.  The live rollback contract is a
// separate thing, established against a real database; what these pin is that
// the writer notices each failure, says which one it was, and still runs the
// cleanup that nontransactional GUI callers depend on.

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "mock_us_db2.h"

#include "ius_db2.h"
#include "us_convert.h"
#include "us_convertio.h"
#include "us_experiment.h"
#include "us_util.h"

#include <QMap>
#include <QQueue>
#include <QStringList>

using ::testing::_;
using ::testing::Invoke;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::An;
using ::testing::HasSubstr;
using ::testing::Not;

namespace
{
// A connection whose every stored procedure can be told what to return.
//
// Rows are scripted per procedure so the writer's two listing loops and its
// solution lookup behave; anything not scripted returns OK with no rows.
class ScriptedDb : public NiceMock< US_DB2_Mock >
{
public:
   ScriptedDb()
   {
      rowsFor[ "get_solutionID_from_GUID" ] << ( QStringList() << "7" );

      ON_CALL( *this, query( An< const QStringList& >() ) )
         .WillByDefault( Invoke( this, &ScriptedDb::onQuery ) );
      ON_CALL( *this, statusQuery( An< const QStringList& >() ) )
         .WillByDefault( Invoke( this, &ScriptedDb::onStatusQuery ) );
      ON_CALL( *this, next() )
         .WillByDefault( Invoke( this, &ScriptedDb::onNext ) );
      ON_CALL( *this, value( _ ) )
         .WillByDefault( Invoke( this, &ScriptedDb::onValue ) );
      ON_CALL( *this, lastErrno() )
         .WillByDefault( Invoke( [ this ]{ return errno_; } ) );
      ON_CALL( *this, lastError() )
         .WillByDefault( Return( QString( "scripted failure" ) ) );
      ON_CALL( *this, lastInsertID() )
         .WillByDefault( Invoke( [ this ]{ return insertID; } ) );
      ON_CALL( *this, writeBlobToDB( _, _, _ ) )
         .WillByDefault( Invoke( this, &ScriptedDb::onBlob ) );
   }

   //! Make one procedure fail; everything else keeps succeeding.
   void fail( const QString& procedure, int status = IUS_DB2::DBERROR )
   {
      statusFor[ procedure ] = status;
   }

   QStringList             called;      //!< procedure names, in call order
   QStringList             blobs;       //!< blob procedures, in call order
   QMap< QString, int >    statusFor;
   int                     blobStatus = IUS_DB2::OK;
   int                     insertID   = 55;

private:
   int scripted( const QString& procedure )
   {
      return statusFor.value( procedure, IUS_DB2::OK );
   }

   void onQuery( const QStringList& args )
   {
      const QString procedure = args.value( 0 );
      called << procedure;
      errno_ = scripted( procedure );
      pending.clear();

      if ( errno_ == IUS_DB2::OK )
         for ( const QStringList& row : rowsFor.value( procedure ) )
            pending.enqueue( row );
   }

   int onStatusQuery( const QStringList& args )
   {
      const QString procedure = args.value( 0 );
      called << procedure;
      errno_ = scripted( procedure );
      return errno_;
   }

   bool onNext()
   {
      if ( pending.isEmpty() )
         return false;

      current = pending.dequeue();
      return true;
   }

   QVariant onValue( unsigned index )
   {
      return ( (int)index < current.size() ) ? QVariant( current[ index ] )
                                             : QVariant();
   }

   int onBlob( const QString&, const QString& procedure, const int )
   {
      blobs << procedure;
      return blobStatus;
   }

   QMap< QString, QList< QStringList > > rowsFor;
   QQueue< QStringList >                 pending;
   QStringList                           current;
   int                                   errno_ = IUS_DB2::OK;
};

US_Experiment sampleExperiment()
{
   US_Experiment experiment;
   experiment.clear();
   experiment.expID    = 11;
   experiment.invID    = 1;
   experiment.runID    = "inject-run";
   experiment.label    = "inject-run";
   experiment.comments = "";

   return experiment;
}

QList< US_Convert::TripleInfo > sampleTriples()
{
   US_Convert::TripleInfo triple;
   triple.clear();
   triple.tripleID              = 1;
   triple.tripleDesc            = "1 / A / 280";
   triple.tripleFilename        = "inject-run.RA.1.A.280.auc";
   triple.excluded              = false;
   triple.centerpiece           = 1;
   triple.solution.solutionID   = 7;
   triple.solution.solutionGUID = "11111111-2222-3333-4444-555555555555";
   US_Util::uuid_parse( "66666666-7777-8888-9999-aaaaaaaaaaaa",
                        (unsigned char*)triple.tripleGUID );

   return QList< US_Convert::TripleInfo >() << triple;
}
}

class WriteRawDataInjectionTest : public ::testing::Test
{
protected:
   QString write()
   {
      US_Experiment experiment          = sampleExperiment();
      QList< US_Convert::TripleInfo > t = sampleTriples();

      return US_ConvertIO::writeRawDataToDB( experiment, t, "/runs/inject/",
                                             &db );
   }

   ScriptedDb db;
};

TEST_F( WriteRawDataInjectionTest, WritesARunAndDoesNotCleanUpAfterItself )
{
   const QString error = write();

   EXPECT_TRUE( error.isEmpty() ) << error.toStdString();
   EXPECT_TRUE( db.called.contains( "new_rawData" ) );
   EXPECT_EQ( db.blobs, QStringList{ "upload_aucData" } );

   // The cleanup only runs on failure; a clean write must not delete the
   // experiment it just populated.
   EXPECT_FALSE( db.called.contains( "delete_experiment" ) );
}

TEST_F( WriteRawDataInjectionTest, StopsBeforeTheTriplesWhenTheRawDeleteFails )
{
   db.fail( "delete_rawData" );

   const QString error = write();

   EXPECT_THAT( error.toStdString(), HasSubstr( "deleting rawData" ) );

   // Nothing was written, so there is nothing to clean up and no experiment
   // to delete.
   EXPECT_FALSE( db.called.contains( "new_rawData" ) );
   EXPECT_FALSE( db.called.contains( "delete_experiment" ) );
   EXPECT_TRUE( db.blobs.isEmpty() );
}

TEST_F( WriteRawDataInjectionTest, ReportsAFailedExperimentSolutionDelete )
{
   db.fail( "delete_experiment_solutions" );

   EXPECT_THAT( write().toStdString(),
                HasSubstr( "deleting experiment solutions" ) );
   EXPECT_FALSE( db.called.contains( "new_rawData" ) );
}

TEST_F( WriteRawDataInjectionTest, ReportsAFailedCellExperimentDelete )
{
   db.fail( "delete_cell_experiments" );

   EXPECT_THAT( write().toStdString(),
                HasSubstr( "deleting cell experiments" ) );
   EXPECT_FALSE( db.called.contains( "new_rawData" ) );
}

TEST_F( WriteRawDataInjectionTest, NamesTheFileWhenTheRawRowFails )
{
   db.fail( "new_rawData" );

   const QString error = write();

   EXPECT_THAT( error.toStdString(),
                HasSubstr( "inject-run.RA.1.A.280.auc" ) );

   // No row, so no blob was attempted.
   EXPECT_TRUE( db.blobs.isEmpty() );

   // And the experiment is deleted so the run ID is not left tainted.
   EXPECT_TRUE( db.called.contains( "delete_experiment" ) );
}

TEST_F( WriteRawDataInjectionTest, NamesTheFileWhenTheRawBlobFails )
{
   db.blobStatus = IUS_DB2::DBERROR;

   const QString error = write();

   EXPECT_THAT( error.toStdString(),
                HasSubstr( "inject-run.RA.1.A.280.auc" ) );
   EXPECT_EQ( db.blobs, QStringList{ "upload_aucData" } );
   EXPECT_TRUE( db.called.contains( "delete_experiment" ) );
}

TEST_F( WriteRawDataInjectionTest, ReportsAFailedCellRecord )
{
   db.fail( "new_cell_experiment" );

   const QString error = write();

   EXPECT_THAT( error.toStdString(), HasSubstr( "writing cell record" ) );
   EXPECT_TRUE( db.called.contains( "delete_experiment" ) );
}

TEST_F( WriteRawDataInjectionTest, ReportsAFailedExperimentSolutionLink )
{
   db.fail( "new_experiment_solution" );

   const QString error = write();

   EXPECT_THAT( error.toStdString(),
                HasSubstr( "associating experiment" ) );
   EXPECT_TRUE( db.called.contains( "delete_experiment" ) );
}

TEST_F( WriteRawDataInjectionTest, AddsTheCleanupsOwnFailureToTheReport )
{
   // The cleanup is a last resort and can itself fail.  When it does, the
   // caller has to hear about both, because the run ID really is tainted then.
   db.fail( "new_rawData" );
   db.fail( "delete_experiment" );

   const QString error = write();

   EXPECT_THAT( error.toStdString(),
                HasSubstr( "inject-run.RA.1.A.280.auc" ) );
   EXPECT_THAT( error.toStdString(),
                HasSubstr( "error deleting experiment 11" ) );
}

TEST_F( WriteRawDataInjectionTest, KeepsTheCleanupForNontransactionalCallers )
{
   // US_ConvertGui writes without a transaction, so this delete is the only
   // thing that undoes a half-written run for it.  A later change that drops
   // it because "rollback handles that" would silently break the desktop.
   db.fail( "new_rawData" );

   write();

   const int rawIndex     = db.called.indexOf( "new_rawData" );
   const int deleteIndex  = db.called.indexOf( "delete_experiment" );

   ASSERT_GE( rawIndex, 0 );
   ASSERT_GE( deleteIndex, 0 );
   EXPECT_GT( deleteIndex, rawIndex );
}
