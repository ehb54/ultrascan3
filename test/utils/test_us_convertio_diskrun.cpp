//! \file test_us_convertio_diskrun.cpp
// US_ConvertIO::readDiskRun() and the diagnostic US_Experiment::readFromDisk()
// overload behind it.
//
// The rule these enforce is that readDiskRun() reports and does not reject: it
// returns the most specific status it can, fills in the detail, and leaves the
// decision to the caller.  So most cases here assert on a status *and* on what
// still made it into the DiskRun.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "support/disk_run_fixture.h"
#include "us3_file_test_base.h"

#include "us_convert.h"
#include "us_convertio.h"
#include "us_dataIO.h"
#include "us_experiment.h"
#include "us_util.h"

#include <QDir>
#include <QFile>

using ::testing::HasSubstr;
using us3test::DatasetSpec;
using us3test::DiskRun;

class ReadDiskRunTest : public Us3FileTestBase
{
   protected:
      QString parentDir() const
      {
         return caseWorkRoot();
      }

      int read( const DiskRun& fixture )
      {
         return US_ConvertIO::readDiskRun( fixture.dir, run, error );
      }

      //! The experiment fields US_Experiment::clear() sets, which a record
      //! that is missing or cut short never reaches.
      void expectClearedExperiment() const
      {
         US_Experiment cleared;
         cleared.clear();

         EXPECT_EQ( run.experiment.expID, cleared.expID );
         EXPECT_EQ( run.experiment.invID, cleared.invID );
         EXPECT_EQ( run.experiment.labID, cleared.labID );
         EXPECT_EQ( run.experiment.instrumentID, cleared.instrumentID );
         EXPECT_EQ( run.experiment.operatorID, cleared.operatorID );
         EXPECT_EQ( run.experiment.rotorID, cleared.rotorID );
         EXPECT_EQ( run.experiment.calibrationID, cleared.calibrationID );
         EXPECT_EQ( run.experiment.rotorCoeff1, cleared.rotorCoeff1 );
         EXPECT_EQ( run.experiment.rotorCoeff2, cleared.rotorCoeff2 );
         EXPECT_EQ( run.experiment.syncOK, cleared.syncOK );
      }

      US_ConvertIO::DiskRun run;
      QString               error;
};

TEST_F( ReadDiskRunTest, ReadsAWholeRunInOneCall )
{
   const DiskRun fixture =
      us3test::writeDiskRun( parentDir(), "whole-run", "RA", { DatasetSpec{ 1, 'A', "280" }, DatasetSpec{ 1, 'A', "410" } } );
   ASSERT_FALSE( fixture.dir.isEmpty() );

   EXPECT_EQ( read( fixture ), US_Convert::OK );
   EXPECT_TRUE( error.isEmpty() );

   EXPECT_EQ( run.runID.toStdString(), "whole-run" );
   EXPECT_EQ( run.runType.toStdString(), "RA" );
   EXPECT_EQ( run.rawData.size(), 2 );
   EXPECT_EQ( run.triples.size(), 2 );
   EXPECT_EQ( run.experiment.runID.toStdString(), "whole-run" );
   EXPECT_EQ( run.experiment.project.projectGUID.toStdString(), fixture.projectGUID.toStdString() );
}

TEST_F( ReadDiskRunTest, AcceptsADirectoryWithOrWithoutATrailingSeparator )
{
   // The underlying reader concatenates rather than joins, so this operation
   // owns the separator.  Both forms have to work, and both have to end up
   // with the same normalized directory.
   const DiskRun fixture = us3test::writeDiskRun( parentDir(), "separator-run", "RA", { DatasetSpec{ 1, 'A', "280" } } );
   ASSERT_FALSE( fixture.dir.isEmpty() );

   QString noSlash = fixture.dir;
   noSlash.chop( 1 );

   EXPECT_EQ( US_ConvertIO::readDiskRun( noSlash, run, error ), US_Convert::OK );
   EXPECT_EQ( run.directory.toStdString(), fixture.dir.toStdString() );
   EXPECT_EQ( run.runID.toStdString(), "separator-run" );

   US_ConvertIO::DiskRun withSlash;
   EXPECT_EQ( US_ConvertIO::readDiskRun( fixture.dir, withSlash, error ), US_Convert::OK );
   EXPECT_EQ( withSlash.directory.toStdString(), fixture.dir.toStdString() );
}

TEST_F( ReadDiskRunTest, ReturnsTheSerializedSpeedStepsUnchanged )
{
   QVector<SP_SPEEDPROFILE> written( 2 );
   written[0].rotorspeed   = 40000;
   written[0].scans        = 5;
   written[0].acceleration = 400;
   written[0].set_speed    = 40000;
   written[1].rotorspeed   = 50000;
   written[1].scans        = 7;
   written[1].acceleration = 300;
   written[1].set_speed    = 50000;

   const DiskRun fixture = us3test::writeDiskRun( parentDir(), "speedsteps-run", "RA", { DatasetSpec{ 1, 'A', "280" } }, written );
   ASSERT_FALSE( fixture.dir.isEmpty() );

   ASSERT_EQ( read( fixture ), US_Convert::OK );
   ASSERT_EQ( run.speedSteps.size(), 2 );

   EXPECT_EQ( run.speedSteps[0].rotorspeed, 40000 );
   EXPECT_EQ( run.speedSteps[0].scans, 5 );
   EXPECT_EQ( run.speedSteps[1].rotorspeed, 50000 );
   EXPECT_EQ( run.speedSteps[1].scans, 7 );

   // Acceleration cannot be reconstructed from saved scans, so the serialized
   // value is authoritative and has to survive the round trip per step.
   EXPECT_EQ( run.speedSteps[0].acceleration, 400 );
   EXPECT_EQ( run.speedSteps[1].acceleration, 300 );
}

TEST_F( ReadDiskRunTest, ReturnsAnEmptyProfileWhenTheRunHasNoSpeedSteps )
{
   // No profile is ever synthesized from the scans.  What the loader does with
   // an empty vector is its business, not this operation's.
   const DiskRun fixture = us3test::writeDiskRun( parentDir(), "no-speedsteps-run", "RA", { DatasetSpec{ 1, 'A', "280" } } );
   ASSERT_FALSE( fixture.dir.isEmpty() );

   ASSERT_EQ( read( fixture ), US_Convert::OK );
   EXPECT_TRUE( run.speedSteps.isEmpty() );
}

TEST_F( ReadDiskRunTest, DoesNotLetOneStepInheritThePrevioussValues )
{
   // speedstepFromXml() assigns only the attributes it finds, so a profile
   // reused across elements lets a step silently inherit its predecessor's
   // values.  A step written with a distinct acceleration must come back with
   // its own, not the one before it.
   QVector<SP_SPEEDPROFILE> written( 3 );

   for ( int ii = 0; ii < written.size(); ii++ )
   {
      written[ii].rotorspeed   = 30000 + ii * 10000;
      written[ii].scans        = 3 + ii;
      written[ii].acceleration = 100 + ii * 50;
      written[ii].set_speed    = written[ii].rotorspeed;
   }

   const DiskRun fixture = us3test::writeDiskRun( parentDir(), "inherit-run", "RA", { DatasetSpec{ 1, 'A', "280" } }, written );
   ASSERT_FALSE( fixture.dir.isEmpty() );

   ASSERT_EQ( read( fixture ), US_Convert::OK );
   ASSERT_EQ( run.speedSteps.size(), 3 );

   for ( int ii = 0; ii < written.size(); ii++ )
   {
      EXPECT_EQ( run.speedSteps[ii].rotorspeed, written[ii].rotorspeed );
      EXPECT_EQ( run.speedSteps[ii].acceleration, written[ii].acceleration );
      EXPECT_EQ( run.speedSteps[ii].scans, written[ii].scans );
   }
}

TEST_F( ReadDiskRunTest, RefusesADirectoryNameThatCannotBeARunId )
{
   const QString bad = parentDir() + "/not a run id/";
   ASSERT_TRUE( QDir().mkpath( bad ) );

   EXPECT_EQ( US_ConvertIO::readDiskRun( bad, run, error ), US_Convert::BAD_RUNID );
   EXPECT_EQ( error, QString( US_ConvertIO::invalidRunIdMessage ).arg( "not a run id" ) );
}

TEST_F( ReadDiskRunTest, ReportsADirectoryWithNoData )
{
   const QString empty = parentDir() + "/empty-run/";
   ASSERT_TRUE( QDir().mkpath( empty ) );

   EXPECT_EQ( US_ConvertIO::readDiskRun( empty, run, error ), US_Convert::NODATA );
   EXPECT_EQ( error, QString( US_ConvertIO::noAucFilesMessage ).arg( run.directory ) );
   EXPECT_THAT( run.directory.toStdString(), HasSubstr( "/empty-run/" ) );
}

TEST_F( ReadDiskRunTest, AScanlessDataFileIsStoppedByTheAucReaderFirst )
{
   // A scanless dataset would crash computeSpeedSteps(), which dereferences
   // (*scans)[0] with no size guard, so readDiskRun() guards against one.  It
   // turns out that guard cannot be reached through a file: writeRawData()
   // will write a zero-scan AUC, and readRawData() then refuses to read it,
   // so the run stops at NOAUC one step earlier.  The guard stays as defense
   // in depth for a raw vector that did not come from this reader; what is
   // pinned here is that a scanless file never reaches the speed-step work by
   // either route.
   const DiskRun fixture =
      us3test::writeDiskRun( parentDir(), "scanless-run", "RA", { DatasetSpec{ 1, 'A', "280", /*scans=*/0, /*points=*/8 } } );
   ASSERT_FALSE( fixture.dir.isEmpty() );

   EXPECT_EQ( read( fixture ), US_Convert::NOAUC );
   EXPECT_EQ( error, QString( US_ConvertIO::unreadableAucMessage ).arg( fixture.dir ) );
   EXPECT_TRUE( run.speedSteps.isEmpty() );
}

TEST_F( ReadDiskRunTest, ReportsMixedRunTypesInOneDirectory )
{
   const DiskRun fixture = us3test::writeDiskRun( parentDir(), "mixed-type-run", "RA", { DatasetSpec{ 1, 'A', "280" } } );
   const DiskRun other   = us3test::writeDiskRun( parentDir(), "mixed-type-run-ip", "IP", { DatasetSpec{ 2, 'B', "280" } } );
   ASSERT_FALSE( fixture.dir.isEmpty() );
   ASSERT_FALSE( other.dir.isEmpty() );

   ASSERT_TRUE( QFile::copy( other.dir + other.aucFiles[0], fixture.dir + other.aucFiles[0] ) );

   // The run type is whichever filename sorts first, which is the intruder
   // here -- "-ip" sorts ahead of ".RA" -- so the experiment record looked for
   // is "mixed-type-run.IP.xml", which does not exist.  CANTOPEN is the more
   // fundamental fact and is what comes back; the mixing is named alongside it
   // because it is why the wrong file was looked for.
   EXPECT_EQ( read( fixture ), US_Convert::CANTOPEN );
   const QString mixed = QString( US_ConvertIO::mixedRunTypesMessage ).arg( "IP, RA", "IP" );
   EXPECT_EQ( error,
              QString( US_ConvertIO::experimentUnopenableMessage ).arg( fixture.dir + "mixed-type-run.IP.xml" ) +
                 QString( US_ConvertIO::alsoMixedRunMessage ).arg( mixed ) );

   // Reported, not rejected: the caller still gets what was read.
   EXPECT_EQ( run.rawData.size(), 2 );
   EXPECT_EQ( run.runType.toStdString(), "IP" );
}

TEST_F( ReadDiskRunTest, StillReadsTheExperimentWhenADirectoryHoldsTwoRuns )
{
   // The mixed-run report is held back rather than returned where it is
   // found, so US_ConvertGui still gets the experiment metadata of such a run.
   const DiskRun fixture = us3test::writeDiskRun( parentDir(), "two-runs", "RA", { DatasetSpec{ 1, 'A', "280" } } );
   const DiskRun guest   = us3test::writeDiskRun( parentDir(), "two-runs-guest", "RA", { DatasetSpec{ 3, 'A', "280" } } );
   ASSERT_FALSE( fixture.dir.isEmpty() );
   ASSERT_FALSE( guest.dir.isEmpty() );

   ASSERT_TRUE( QFile::copy( guest.dir + guest.aucFiles[0], fixture.dir + guest.aucFiles[0] ) );

   EXPECT_EQ( read( fixture ), US_Convert::INVALID_RUN );
   EXPECT_EQ( error,
              QString( US_ConvertIO::notOneRunCausedMessage )
                 .arg( fixture.dir,
                       QString( US_ConvertIO::mixedRunIdsMessage ).arg( "two-runs-guest, two-runs" ),
                       QString( US_Experiment::dataWithoutDatasetMessage ).arg( "3 / A / 280" ) ) );

   // Everything the experiment record had is still there.
   EXPECT_EQ( run.experiment.runID.toStdString(), "two-runs" );
   EXPECT_EQ( run.experiment.project.projectGUID.toStdString(), fixture.projectGUID.toStdString() );
   EXPECT_EQ( run.rawData.size(), 2 );
}

TEST_F( ReadDiskRunTest, ReportsMixedRunIdsInOneDirectory )
{
   const DiskRun fixture = us3test::writeDiskRun( parentDir(), "host-run", "RA", { DatasetSpec{ 1, 'A', "280" } } );
   const DiskRun guest   = us3test::writeDiskRun( parentDir(), "guest-run", "RA", { DatasetSpec{ 3, 'A', "280" } } );
   ASSERT_FALSE( fixture.dir.isEmpty() );
   ASSERT_FALSE( guest.dir.isEmpty() );

   ASSERT_TRUE( QFile::copy( guest.dir + guest.aucFiles[0], fixture.dir + guest.aucFiles[0] ) );

   EXPECT_EQ( read( fixture ), US_Convert::INVALID_RUN );
   EXPECT_EQ( error,
              QString( US_ConvertIO::notOneRunCausedMessage )
                 .arg( fixture.dir,
                       QString( US_ConvertIO::mixedRunIdsMessage ).arg( "guest-run, host-run" ),
                       QString( US_Experiment::dataWithoutDatasetMessage ).arg( "3 / A / 280" ) ) );
   EXPECT_EQ( run.rawData.size(), 2 );
}

TEST_F( ReadDiskRunTest, ReportsAMissingExperimentRecordWithItsPath )
{
   const DiskRun fixture = us3test::writeDiskRun( parentDir(), "no-xml-run", "RA", { DatasetSpec{ 1, 'A', "280" } } );
   ASSERT_FALSE( fixture.dir.isEmpty() );
   ASSERT_TRUE( QFile::remove( fixture.dir + "no-xml-run.RA.xml" ) );

   EXPECT_EQ( read( fixture ), US_Convert::CANTOPEN );
   EXPECT_EQ( error, QString( US_ConvertIO::experimentUnopenableMessage ).arg( fixture.dir + "no-xml-run.RA.xml" ) );

   // The raw data was already read and is still there for the caller.
   EXPECT_EQ( run.rawData.size(), 1 );
   expectClearedExperiment();
}

TEST_F( ReadDiskRunTest, ReportsAMalformedExperimentRecord )
{
   const DiskRun fixture = us3test::writeDiskRun( parentDir(), "bad-xml-run", "RA", { DatasetSpec{ 1, 'A', "280" } } );
   ASSERT_FALSE( fixture.dir.isEmpty() );

   // Cut the record just after the <experiment> start tag, so none of the
   // fields inside it are read.
   QFile xml( fixture.dir + "bad-xml-run.RA.xml" );
   ASSERT_TRUE( xml.open( QIODevice::ReadWrite ) );
   const QByteArray content = xml.readAll();
   const int        tagEnd  = content.indexOf( '>', content.indexOf( "<experiment " ) );
   ASSERT_GT( tagEnd, 0 );
   ASSERT_TRUE( xml.resize( tagEnd + 1 ) );
   xml.close();

   EXPECT_EQ( read( fixture ), US_Convert::BADXML );
   EXPECT_EQ( error, QString( US_ConvertIO::experimentMalformedMessage ).arg( fixture.dir + "bad-xml-run.RA.xml" ) );
   expectClearedExperiment();
}

TEST_F( ReadDiskRunTest, NamesTheDatasetTheExperimentRecordDoesNotDescribe )
{
   // An .auc with no <dataset> would otherwise keep a zero triple GUID that a
   // later database write would store.
   const DiskRun fixture = us3test::writeDiskRun( parentDir(), "extra-auc-run", "RA", { DatasetSpec{ 1, 'A', "280" } } );
   const DiskRun donor   = us3test::writeDiskRun( parentDir(), "extra-auc-donor", "RA", { DatasetSpec{ 2, 'B', "280" } } );
   ASSERT_FALSE( fixture.dir.isEmpty() );
   ASSERT_FALSE( donor.dir.isEmpty() );

   ASSERT_TRUE( QFile::copy( donor.dir + donor.aucFiles[0], fixture.dir + "extra-auc-run.RA.2.B.280.auc" ) );

   EXPECT_EQ( read( fixture ), US_Convert::PARTIAL_XML );
   EXPECT_EQ(
      error,
      QString( US_ConvertIO::experimentDisagreesMessage )
         .arg( "extra-auc-run.RA.xml", fixture.dir, QString( US_Experiment::dataWithoutDatasetMessage ).arg( "2 / B / 280" ) ) );
}

TEST_F( ReadDiskRunTest, NamesTheDatasetWithNoDataFile )
{
   // The mirror case: a <dataset> with no .auc.
   const DiskRun fixture = us3test::writeDiskRun(
      parentDir(), "extra-dataset-run", "RA", { DatasetSpec{ 1, 'A', "280" }, DatasetSpec{ 1, 'A', "410" } } );
   ASSERT_FALSE( fixture.dir.isEmpty() );
   ASSERT_TRUE( QFile::remove( fixture.dir + fixture.aucFiles[1] ) );

   EXPECT_EQ( read( fixture ), US_Convert::PARTIAL_XML );
   EXPECT_EQ( error,
              QString( US_ConvertIO::experimentDisagreesMessage )
                 .arg( "extra-dataset-run.RA.xml",
                       fixture.dir,
                       QString( US_Experiment::datasetWithoutDataMessage ).arg( "1 / A / 410" ) ) );
}

TEST_F( ReadDiskRunTest, ReportsADatasetIdentityThatDisagreesWithItsDataFile )
{
   // Same triple description, different raw GUID in the AUC header.
   const DiskRun fixture = us3test::writeDiskRun( parentDir(), "guid-run", "RA", { DatasetSpec{ 1, 'A', "280" } } );
   const DiskRun donor   = us3test::writeDiskRun( parentDir(), "guid-donor", "RA", { DatasetSpec{ 1, 'A', "280" } } );
   ASSERT_FALSE( fixture.dir.isEmpty() );
   ASSERT_FALSE( donor.dir.isEmpty() );

   const QString target = fixture.dir + fixture.aucFiles[0];
   ASSERT_TRUE( QFile::remove( target ) );
   ASSERT_TRUE( QFile::copy( donor.dir + donor.aucFiles[0], target ) );

   EXPECT_EQ( read( fixture ), US_Convert::BADGUID );
   EXPECT_EQ( error,
              QString( US_ConvertIO::identityMismatchMessage ).arg( "1 / A / 280", fixture.rawGUIDs[0], donor.rawGUIDs[0] ) );
}

TEST_F( ReadDiskRunTest, DoesNotRequireEditsOrChemistryRecords )
{
   // A run with no edit XML and no solution on disk still reads.  Requiring
   // either is the caller's policy, not the reader's.
   const DiskRun fixture = us3test::writeDiskRun( parentDir(), "bare-run", "RA", { DatasetSpec{ 1, 'A', "280" } } );
   ASSERT_FALSE( fixture.dir.isEmpty() );

   // Edit XML is named runID.editLabel.runType.cell.channel.wavelength.xml,
   // and an edit label starts with a ten-digit yyMMddhhmm timestamp.
   const QRegularExpression editFile( R"(\.\d{10}[^.]*\.)" );
   ASSERT_TRUE( editFile.match( "bare-run.25103012340.RA.1.A.280.xml" ).hasMatch() );
   ASSERT_FALSE( editFile.match( "bare-run.RA.xml" ).hasMatch() );

   EXPECT_TRUE( QDir( fixture.dir ).entryList( { "*.xml" }, QDir::Files ).filter( editFile ).isEmpty() );

   EXPECT_EQ( read( fixture ), US_Convert::OK );
}

TEST_F( ReadDiskRunTest, StartsFromAnEmptyRunEachTime )
{
   const DiskRun first =
      us3test::writeDiskRun( parentDir(), "first-run", "RA", { DatasetSpec{ 1, 'A', "280" }, DatasetSpec{ 1, 'A', "410" } } );
   const DiskRun second = us3test::writeDiskRun( parentDir(), "second-run", "RA", { DatasetSpec{ 2, 'B', "280" } } );
   ASSERT_FALSE( first.dir.isEmpty() );
   ASSERT_FALSE( second.dir.isEmpty() );

   ASSERT_EQ( read( first ), US_Convert::OK );
   ASSERT_EQ( run.rawData.size(), 2 );

   ASSERT_EQ( read( second ), US_Convert::OK );
   EXPECT_EQ( run.rawData.size(), 1 );
   EXPECT_EQ( run.triples.size(), 1 );
   EXPECT_EQ( run.runID.toStdString(), "second-run" );
   EXPECT_TRUE( run.speedSteps.isEmpty() );
}
