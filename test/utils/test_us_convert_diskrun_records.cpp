// Characterization of the record-reading steps of the disk prefix inside
// US_ConvertGui::loadUS3Disk(): the experiment XML, the project it names, the
// per-triple solutions, and an RI run's intensity profile.
//
// As with the raw-data step, these describe today's behavior -- permissive
// parts included -- so the Phase 3 extraction can be checked against it.

#include <gtest/gtest.h>

#include "support/disk_run_fixture.h"
#include "us3_file_test_base.h"

#include "ius_db2.h"
#include "us_convert.h"
#include "us_experiment.h"
#include "us_project.h"
#include "us_simparms.h"
#include "us_solution.h"
#include "us_util.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>

using us3test::DatasetSpec;
using us3test::DiskRun;

namespace
{
// Two steps, as US_SimulationParameters serializes them.
QVector< SP_SPEEDPROFILE > twoSpeedSteps()
{
   QVector< SP_SPEEDPROFILE > steps( 2 );
   steps[ 0 ].rotorspeed = 40000;
   steps[ 0 ].scans      = 5;
   steps[ 1 ].rotorspeed = 50000;
   steps[ 1 ].scans      = 5;

   return steps;
}

QString guidOf( const US_Convert::TripleInfo& triple )
{
   return US_Util::uuid_unparse(
             (unsigned char*)const_cast< char* >( triple.tripleGUID ) );
}
}

class DiskRunRecordsTest : public Us3FileTestBase
{
protected:
   QString parentDir() const { return caseWorkRoot(); }

   // The raw-data step, which every record step builds on.
   int readRaw( const DiskRun& run )
   {
      return US_Convert::readUS3Disk( run.dir, allData, triples, runType );
   }

   QVector< US_DataIO::RawData >   allData;
   QList< US_Convert::TripleInfo > triples;
   QString                         runType;
   US_Experiment                   experiment;
};

// --------------------------------------------------------- experiment XML

TEST_F( DiskRunRecordsTest, ReadsTheExperimentAndMatchesItToTheTriples )
{
   const DiskRun run = us3test::writeDiskRun(
      parentDir(), "records-run", "RA",
      { DatasetSpec{ 1, 'A', "280" }, DatasetSpec{ 1, 'A', "410" } } );
   ASSERT_FALSE( run.dir.isEmpty() );
   ASSERT_EQ( readRaw( run ), US_Convert::OK );

   EXPECT_EQ( experiment.readFromDisk( triples, runType, run.runID, run.dir ),
              US_Convert::OK );

   EXPECT_EQ( experiment.runID.toStdString(), run.runID.toStdString() );
   EXPECT_EQ( experiment.expType.toStdString(), "velocity" );

   ASSERT_EQ( triples.size(), 2 );

   for ( int ii = 0; ii < triples.size(); ii++ )
   {
      // The experiment supplies what the filename scan could not.
      EXPECT_EQ( guidOf( triples[ ii ] ).toStdString(),
                 run.rawGUIDs[ ii ].toStdString() );
      EXPECT_EQ( triples[ ii ].tripleFilename.toStdString(),
                 run.aucFiles[ ii ].toStdString() );
      EXPECT_EQ( triples[ ii ].tripleID, ii + 1 );
      EXPECT_FALSE( triples[ ii ].excluded );
      EXPECT_EQ( triples[ ii ].solution.solutionGUID.toStdString(),
                 run.solutionGUID.toStdString() );
   }
}

TEST_F( DiskRunRecordsTest, NamesTheProjectButDoesNotLoadIt )
{
   // readFromDisk() copies the project id, GUID and description straight out
   // of the experiment XML.  Everything else about the project comes from the
   // separate P record, which the caller reads next.
   const DiskRun run = us3test::writeDiskRun(
      parentDir(), "project-named-run", "RA",
      { DatasetSpec{ 1, 'A', "280" } } );
   ASSERT_FALSE( run.dir.isEmpty() );
   ASSERT_EQ( readRaw( run ), US_Convert::OK );

   ASSERT_EQ( experiment.readFromDisk( triples, runType, run.runID, run.dir ),
              US_Convert::OK );

   EXPECT_EQ( experiment.project.projectGUID.toStdString(),
              run.projectGUID.toStdString() );
   EXPECT_EQ( experiment.project.projectDesc.toStdString(),
              "disk-run fixture project" );
}

TEST_F( DiskRunRecordsTest, ReportsCantOpenWhenTheExperimentXmlIsMissing )
{
   const DiskRun run = us3test::writeDiskRun(
      parentDir(), "no-xml-run", "RA", { DatasetSpec{ 1, 'A', "280" } } );
   ASSERT_FALSE( run.dir.isEmpty() );
   ASSERT_EQ( readRaw( run ), US_Convert::OK );

   ASSERT_TRUE( QFile::remove( run.dir + run.runID + ".RA.xml" ) );

   EXPECT_EQ( experiment.readFromDisk( triples, runType, run.runID, run.dir ),
              US_Convert::CANTOPEN );
}

TEST_F( DiskRunRecordsTest, ReportsBadXmlForAMalformedExperiment )
{
   const DiskRun run = us3test::writeDiskRun(
      parentDir(), "bad-xml-run", "RA", { DatasetSpec{ 1, 'A', "280" } } );
   ASSERT_FALSE( run.dir.isEmpty() );
   ASSERT_EQ( readRaw( run ), US_Convert::OK );

   // One explicit corruption of this test's own copy: truncate mid-document.
   QFile xml( run.dir + run.runID + ".RA.xml" );
   ASSERT_TRUE( xml.open( QIODevice::ReadWrite ) );
   ASSERT_TRUE( xml.resize( xml.size() / 2 ) );
   xml.close();

   EXPECT_EQ( experiment.readFromDisk( triples, runType, run.runID, run.dir ),
              US_Convert::BADXML );
}

TEST_F( DiskRunRecordsTest, IgnoresAnExperimentDatasetWithNoMatchingTriple )
{
   // The experiment describes two datasets but only one .auc is present.  The
   // reader reports OK and leaves the unmatched dataset out.  Phase 3 reports
   // this at the coordinator boundary; it must not start failing here.
   const DiskRun run = us3test::writeDiskRun(
      parentDir(), "extra-dataset-run", "RA",
      { DatasetSpec{ 1, 'A', "280" }, DatasetSpec{ 1, 'A', "410" } } );
   ASSERT_FALSE( run.dir.isEmpty() );

   ASSERT_TRUE( QFile::remove( run.dir + run.aucFiles[ 1 ] ) );
   ASSERT_EQ( readRaw( run ), US_Convert::OK );
   ASSERT_EQ( triples.size(), 1 );

   EXPECT_EQ( experiment.readFromDisk( triples, runType, run.runID, run.dir ),
              US_Convert::OK );

   ASSERT_EQ( triples.size(), 1 );
   EXPECT_EQ( guidOf( triples[ 0 ] ).toStdString(),
              run.rawGUIDs[ 0 ].toStdString() );
}

TEST_F( DiskRunRecordsTest, LeavesATripleWithNoExperimentDatasetUnfilled )
{
   // The mirror case: an .auc the experiment says nothing about.  Its triple
   // survives with no GUID and no filename rather than being rejected.
   const DiskRun run = us3test::writeDiskRun(
      parentDir(), "extra-auc-run", "RA", { DatasetSpec{ 1, 'A', "280" } } );
   const DiskRun donor = us3test::writeDiskRun(
      parentDir(), "donor-run", "RA", { DatasetSpec{ 2, 'B', "280" } } );
   ASSERT_FALSE( run.dir.isEmpty() );
   ASSERT_FALSE( donor.dir.isEmpty() );

   ASSERT_TRUE( QFile::copy( donor.dir + donor.aucFiles[ 0 ],
                             run.dir + run.runID + ".RA.2.B.280.auc" ) );

   ASSERT_EQ( readRaw( run ), US_Convert::OK );
   ASSERT_EQ( triples.size(), 2 );

   EXPECT_EQ( experiment.readFromDisk( triples, runType, run.runID, run.dir ),
              US_Convert::OK );

   ASSERT_EQ( triples.size(), 2 );
   EXPECT_EQ( triples[ 1 ].tripleDesc.toStdString(), "2 / B / 280" );
   EXPECT_TRUE( triples[ 1 ].tripleFilename.isEmpty() );
   EXPECT_EQ( triples[ 1 ].tripleID, 0 );
}

TEST_F( DiskRunRecordsTest, DoesNotReturnTheSerializedSpeedSteps )
{
   // saveToDisk() writes <speedstep> elements and readFromDisk() walks past
   // them: there is no out parameter for them and no member they land in.  A
   // caller that needs the profile has to parse the same file itself, which is
   // why the seed path has been writing zero speedstep rows.
   const DiskRun run = us3test::writeDiskRun(
      parentDir(), "speedstep-run", "RA", { DatasetSpec{ 1, 'A', "280" } },
      twoSpeedSteps() );
   ASSERT_FALSE( run.dir.isEmpty() );
   ASSERT_EQ( readRaw( run ), US_Convert::OK );

   ASSERT_EQ( experiment.readFromDisk( triples, runType, run.runID, run.dir ),
              US_Convert::OK );

   QFile xml( run.dir + run.runID + ".RA.xml" );
   ASSERT_TRUE( xml.open( QIODevice::ReadOnly ) );
   const QString text = QString::fromUtf8( xml.readAll() );

   EXPECT_EQ( text.count( "<speedstep" ), 2 );
   EXPECT_TRUE( text.contains( "rotorspeed=\"40000\"" ) );
   EXPECT_TRUE( text.contains( "rotorspeed=\"50000\"" ) );
}

// ------------------------------------------------------------- P record

TEST_F( DiskRunRecordsTest, LoadsTheProjectRecordTheExperimentNames )
{
   const DiskRun run = us3test::writeDiskRun(
      parentDir(), "project-run", "RA", { DatasetSpec{ 1, 'A', "280" } } );
   ASSERT_FALSE( run.dir.isEmpty() );
   ASSERT_EQ( readRaw( run ), US_Convert::OK );
   ASSERT_EQ( experiment.readFromDisk( triples, runType, run.runID, run.dir ),
              US_Convert::OK );

   // saveToDisk() archived the P record alongside the experiment XML.
   US_Project project;
   QString guid = experiment.project.projectGUID;

   EXPECT_EQ( project.readFromDisk( guid ), (int)IUS_DB2::OK );
   EXPECT_EQ( project.projectGUID.toStdString(), guid.toStdString() );
   EXPECT_EQ( project.projectDesc.toStdString(), "disk-run fixture project" );
}

TEST_F( DiskRunRecordsTest, ReportsNoProjectForAGuidWithNoRecord )
{
   US_Project project;
   QString absent = "11111111-2222-3333-4444-555555555555";

   EXPECT_EQ( project.readFromDisk( absent ), (int)IUS_DB2::NO_PROJECT );
}

TEST_F( DiskRunRecordsTest, ReportsNoProjectForAnEmptyGuid )
{
   // What an experiment written before the simulator set a project GUID gives
   // the caller.  The GUI reports it and clears the project rather than
   // refusing the run.
   US_Project project;
   QString empty;

   EXPECT_EQ( project.readFromDisk( empty ), (int)IUS_DB2::NO_PROJECT );
}

// ------------------------------------------------------------- solutions

TEST_F( DiskRunRecordsTest, ReportsNoSolutionForAGuidWithNoRecord )
{
   US_Solution solution;
   QString absent = "66666666-7777-8888-9999-aaaaaaaaaaaa";

   EXPECT_EQ( solution.readFromDisk( absent ), (int)IUS_DB2::NO_SOLUTION );
}

TEST_F( DiskRunRecordsTest, TheGuiCarriesThePrecedingSolutionForwardOverAGap )
{
   // US_ConvertGui::loadUS3Disk() walks the triples in order and, for a triple
   // whose solution GUID is empty, substitutes the previous triple's -- but
   // only when there is a previous one, so an empty GUID on the first triple
   // stays empty.  The rule is reproduced here because it lives in the GUI
   // loop rather than in a reader; the extraction has to preserve it.
   const DiskRun run = us3test::writeDiskRun(
      parentDir(), "solution-gap-run", "RA",
      { DatasetSpec{ 1, 'A', "280" },
        DatasetSpec{ 1, 'A', "410" },
        DatasetSpec{ 2, 'B', "280" } } );
   ASSERT_FALSE( run.dir.isEmpty() );
   ASSERT_EQ( readRaw( run ), US_Convert::OK );
   ASSERT_EQ( experiment.readFromDisk( triples, runType, run.runID, run.dir ),
              US_Convert::OK );
   ASSERT_EQ( triples.size(), 3 );

   triples[ 0 ].solution.solutionGUID.clear();
   triples[ 1 ].solution.solutionGUID.clear();

   QString previous;

   for ( int ii = 0; ii < triples.size(); ii++ )
   {
      QString current = triples[ ii ].solution.solutionGUID;

      if ( current == previous )
         continue;

      if ( current.isEmpty()  &&  ii > 0 )
      {
         current = previous;
         triples[ ii ].solution.solutionGUID = current;
      }

      previous = current;
   }

   EXPECT_TRUE( triples[ 0 ].solution.solutionGUID.isEmpty() );

   // Second triple: empty, and the first is empty too, so it stays empty.
   EXPECT_TRUE( triples[ 1 ].solution.solutionGUID.isEmpty() );

   // Third triple names its own solution and is left alone.
   EXPECT_EQ( triples[ 2 ].solution.solutionGUID.toStdString(),
              run.solutionGUID.toStdString() );
}

// ---------------------------------------------------------- RI profile

TEST_F( DiskRunRecordsTest, RoundTripsAnRiIntensityProfile )
{
   const DiskRun run = us3test::writeDiskRun(
      parentDir(), "ri-run", "RI", { DatasetSpec{ 1, 'A', "280" } } );
   ASSERT_FALSE( run.dir.isEmpty() );
   ASSERT_EQ( readRaw( run ), US_Convert::OK );
   ASSERT_EQ( runType.toStdString(), "RI" );

   US_Experiment written;
   written.clear();
   written.opticalSystem = "RI";
   written.RIProfile << 1.5 << 2.5 << 3.5;

   ASSERT_EQ( written.saveRIDisk( run.runID, run.dir ), US_Convert::OK );

   EXPECT_EQ( experiment.readRIDisk( run.runID, run.dir ), US_Convert::OK );
   ASSERT_EQ( experiment.RIProfile.size(), 3 );
   EXPECT_DOUBLE_EQ( experiment.RIProfile[ 0 ], 1.5 );
   EXPECT_DOUBLE_EQ( experiment.RIProfile[ 2 ], 3.5 );
}

TEST_F( DiskRunRecordsTest, WritesAnEmptyProfileWhenTheOpticalSystemIsNotRi )
{
   // createRIXml() produces nothing unless opticalSystem is "RI", and
   // saveRIDisk() reports OK after writing that nothing.  The empty file then
   // reads back as BADXML, so the failure surfaces one step away from its
   // cause.  Pinned as current behavior, not endorsed.
   const DiskRun run = us3test::writeDiskRun(
      parentDir(), "ri-mislabelled-run", "RI",
      { DatasetSpec{ 1, 'A', "280" } } );
   ASSERT_FALSE( run.dir.isEmpty() );

   US_Experiment written;
   written.clear();
   written.opticalSystem = "RA";
   written.RIProfile << 1.5 << 2.5;

   EXPECT_EQ( written.saveRIDisk( run.runID, run.dir ), US_Convert::OK );

   const QFileInfo profile( run.dir + run.runID + ".RIProfile.xml" );
   EXPECT_TRUE( profile.exists() );
   EXPECT_EQ( profile.size(), 0 );

   EXPECT_EQ( experiment.readRIDisk( run.runID, run.dir ),
              US_Convert::BADXML );
}

TEST_F( DiskRunRecordsTest, BlanksTheSolutionGuidOfADatasetWhoseSolutionIdIsUnset )
{
   // saveToDisk() run-length encodes repeated solutions across datasets, and
   // starts from id -1 with an empty GUID.  A triple whose solutionID is still
   // -1 therefore matches that seed and is written with no solution GUID at
   // all -- which is how a run ends up with the empty GUIDs the GUI then
   // patches over with the preceding triple's.
   const DiskRun run = us3test::writeDiskRun(
      parentDir(), "unset-solution-run", "RA",
      { DatasetSpec{ 1, 'A', "280" }, DatasetSpec{ 1, 'A', "410" } } );
   ASSERT_FALSE( run.dir.isEmpty() );
   ASSERT_EQ( readRaw( run ), US_Convert::OK );
   ASSERT_EQ( experiment.readFromDisk( triples, runType, run.runID, run.dir ),
              US_Convert::OK );
   ASSERT_EQ( triples.size(), 2 );

   // Rewrite the same run with the sentinel id the fixture avoids.
   for ( int ii = 0; ii < triples.size(); ii++ )
      triples[ ii ].solution.solutionID = -1;

   QVector< SP_SPEEDPROFILE > none;
   ASSERT_EQ( experiment.saveToDisk( triples, runType, run.runID, run.dir,
                                     none ),
              US_Convert::OK );

   QList< US_Convert::TripleInfo > reread;
   QVector< US_DataIO::RawData >   rawAgain;
   QString                         typeAgain;
   ASSERT_EQ( US_Convert::readUS3Disk( run.dir, rawAgain, reread, typeAgain ),
              US_Convert::OK );

   US_Experiment again;
   ASSERT_EQ( again.readFromDisk( reread, typeAgain, run.runID, run.dir ),
              US_Convert::OK );
   ASSERT_EQ( reread.size(), 2 );

   EXPECT_TRUE( reread[ 0 ].solution.solutionGUID.isEmpty() );
   EXPECT_TRUE( reread[ 1 ].solution.solutionGUID.isEmpty() );
}

TEST_F( DiskRunRecordsTest, ReportsCantOpenWhenAnRiRunHasNoProfile )
{
   // An RI run without a profile is a case the GUI reports and carries on
   // from; the run still loads.
   const DiskRun run = us3test::writeDiskRun(
      parentDir(), "ri-no-profile-run", "RI",
      { DatasetSpec{ 1, 'A', "280" } } );
   ASSERT_FALSE( run.dir.isEmpty() );

   EXPECT_EQ( experiment.readRIDisk( run.runID, run.dir ),
              US_Convert::CANTOPEN );
   EXPECT_TRUE( experiment.RIProfile.isEmpty() );
}
