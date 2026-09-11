// Characterization of US_Convert::readUS3Disk(), the first step of the disk
// prefix inside US_ConvertGui::loadUS3Disk().
//
// These tests describe what the shipping reader does today, including where it
// is permissive.  Phase 3 moves this code behind a shared operation; the
// contract is that every archive that loads now still loads afterwards, so
// these must keep passing unchanged against US_ConvertGui after the migration.

#include <gtest/gtest.h>

#include "support/disk_run_fixture.h"
#include "us3_file_test_base.h"

#include "us_convert.h"
#include "us_dataIO.h"
#include "us_util.h"

#include <QDir>
#include <QFile>

using us3test::DatasetSpec;
using us3test::DiskRun;

namespace
{
QString guidOf( const US_DataIO::RawData& data )
{
   return US_Util::uuid_unparse( (unsigned char*)data.rawGUID );
}
}

class ReadUS3DiskTest : public Us3FileTestBase
{
protected:
   QString parentDir() const { return caseWorkRoot(); }

   // The reader concatenates its directory argument with a bare filename, so
   // every caller has to hand it a trailing separator.  See
   // TakesTheDirectoryArgumentLiterally for what happens when one does not.
   int read( const DiskRun& run )
   {
      return US_Convert::readUS3Disk( run.dir, allData, triples, runType );
   }

   QVector< US_DataIO::RawData > allData;
   QList< US_Convert::TripleInfo > triples;
   QString runType;
};

// ---------------------------------------------------------------- happy path

TEST_F( ReadUS3DiskTest, ReadsASingleVelocityRun )
{
   const DiskRun run = us3test::writeDiskRun(
      parentDir(), "velocity-run", "RA", { DatasetSpec{ 1, 'A', "280" } } );
   ASSERT_FALSE( run.dir.isEmpty() );

   EXPECT_EQ( read( run ), US_Convert::OK );
   ASSERT_EQ( allData.size(), 1 );
   ASSERT_EQ( triples.size(), 1 );
   EXPECT_EQ( runType.toStdString(), "RA" );
   EXPECT_EQ( triples[ 0 ].tripleDesc.toStdString(), "1 / A / 280" );
}

TEST_F( ReadUS3DiskTest, DerivesTheRunTypeFromTheFirstFilename )
{
   for ( const char* type : { "RA", "RI", "IP", "FI", "WI" } )
   {
      const DiskRun run = us3test::writeDiskRun(
         parentDir(), QString( "run-" ) + type, type,
         { DatasetSpec{ 1, 'A', "280" } } );
      ASSERT_FALSE( run.dir.isEmpty() ) << type;

      ASSERT_EQ( read( run ), US_Convert::OK ) << type;
      EXPECT_EQ( runType.toStdString(), std::string( type ) );
   }
}

TEST_F( ReadUS3DiskTest, OrdersTriplesAndDataByFilename )
{
   // QDir::Name sorting, not the order the datasets were written.
   const DiskRun run = us3test::writeDiskRun(
      parentDir(), "ordered-run", "RA",
      { DatasetSpec{ 2, 'B', "410" },
        DatasetSpec{ 1, 'A', "280" },
        DatasetSpec{ 1, 'A', "410" } } );
   ASSERT_FALSE( run.dir.isEmpty() );

   ASSERT_EQ( read( run ), US_Convert::OK );
   ASSERT_EQ( triples.size(), 3 );

   EXPECT_EQ( triples[ 0 ].tripleDesc.toStdString(), "1 / A / 280" );
   EXPECT_EQ( triples[ 1 ].tripleDesc.toStdString(), "1 / A / 410" );
   EXPECT_EQ( triples[ 2 ].tripleDesc.toStdString(), "2 / B / 410" );

   ASSERT_EQ( allData.size(), 3 );
   EXPECT_EQ( allData[ 0 ].cell, 1 );
   EXPECT_EQ( allData[ 0 ].channel, 'A' );
   EXPECT_EQ( allData[ 2 ].cell, 2 );
   EXPECT_EQ( allData[ 2 ].channel, 'B' );
}

TEST_F( ReadUS3DiskTest, PreservesEachDatasetsRawGuid )
{
   const DiskRun run = us3test::writeDiskRun(
      parentDir(), "mwl-run", "RA",
      { DatasetSpec{ 1, 'A', "280" }, DatasetSpec{ 1, 'A', "410" } } );
   ASSERT_FALSE( run.dir.isEmpty() );

   ASSERT_EQ( read( run ), US_Convert::OK );
   ASSERT_EQ( allData.size(), 2 );

   EXPECT_EQ( guidOf( allData[ 0 ] ).toStdString(),
              run.rawGUIDs[ 0 ].toStdString() );
   EXPECT_EQ( guidOf( allData[ 1 ] ).toStdString(),
              run.rawGUIDs[ 1 ].toStdString() );
   EXPECT_NE( guidOf( allData[ 0 ] ), guidOf( allData[ 1 ] ) );
}

TEST_F( ReadUS3DiskTest, ScalesTheWavelengthFieldForWavelengthScans )
{
   // A "WA" run's fifth filename field is a radius in microns, and the reader
   // reports it in centimetres.
   const DiskRun run = us3test::writeDiskRun(
      parentDir(), "wa-run", "WA", { DatasetSpec{ 1, 'A', "6200" } } );
   ASSERT_FALSE( run.dir.isEmpty() );

   ASSERT_EQ( read( run ), US_Convert::OK );
   ASSERT_EQ( triples.size(), 1 );
   EXPECT_EQ( triples[ 0 ].tripleDesc.toStdString(), "1 / A / 6.2" );
}

TEST_F( ReadUS3DiskTest, ClearsPriorResultsBeforeReading )
{
   const DiskRun first = us3test::writeDiskRun(
      parentDir(), "first-run", "RA",
      { DatasetSpec{ 1, 'A', "280" }, DatasetSpec{ 1, 'A', "410" } } );
   const DiskRun second = us3test::writeDiskRun(
      parentDir(), "second-run", "RA", { DatasetSpec{ 2, 'B', "280" } } );
   ASSERT_FALSE( first.dir.isEmpty() );
   ASSERT_FALSE( second.dir.isEmpty() );

   ASSERT_EQ( read( first ), US_Convert::OK );
   ASSERT_EQ( allData.size(), 2 );

   ASSERT_EQ( read( second ), US_Convert::OK );
   EXPECT_EQ( allData.size(), 1 );
   EXPECT_EQ( triples.size(), 1 );
}

// ------------------------------------------------------------------ failures

TEST_F( ReadUS3DiskTest, ReportsNoDataForADirectoryWithoutAucFiles )
{
   const QString empty = parentDir() + "/empty-run/";
   ASSERT_TRUE( QDir().mkpath( empty ) );

   EXPECT_EQ( US_Convert::readUS3Disk( empty, allData, triples, runType ),
              US_Convert::NODATA );
   EXPECT_TRUE( allData.isEmpty() );
}

TEST_F( ReadUS3DiskTest, ReportsNoDataForADirectoryThatIsNotThere )
{
   EXPECT_EQ( US_Convert::readUS3Disk( parentDir() + "/absent-run/",
                                       allData, triples, runType ),
              US_Convert::NODATA );
}

TEST_F( ReadUS3DiskTest, TakesTheDirectoryArgumentLiterally )
{
   // QDir lists the directory either way, so a caller that omits the trailing
   // separator gets past the empty-directory check and then fails opening
   // "<dir><filename>" -- a NOAUC, which reads as a corrupt data file rather
   // than as the caller's path mistake that it is.
   // US_ConvertGui::loadUS3Disk() appends the separator before calling;
   // anything replacing this code has to keep doing so or take a real path.
   const DiskRun run = us3test::writeDiskRun(
      parentDir(), "slashless-run", "RA", { DatasetSpec{ 1, 'A', "280" } } );
   ASSERT_FALSE( run.dir.isEmpty() );

   QString noSlash = run.dir;
   noSlash.chop( 1 );

   EXPECT_EQ( US_Convert::readUS3Disk( noSlash, allData, triples, runType ),
              US_Convert::NOAUC );
}

TEST_F( ReadUS3DiskTest, ReportsNoAucWhenADataFileCannotBeRead )
{
   const DiskRun run = us3test::writeDiskRun(
      parentDir(), "corrupt-run", "RA", { DatasetSpec{ 1, 'A', "280" } } );
   ASSERT_FALSE( run.dir.isEmpty() );

   // One explicit corruption of this test's own copy: truncate the payload.
   QFile auc( run.dir + run.aucFiles[ 0 ] );
   ASSERT_TRUE( auc.open( QIODevice::ReadWrite ) );
   ASSERT_TRUE( auc.resize( 12 ) );
   auc.close();

   EXPECT_EQ( read( run ), US_Convert::NOAUC );
}

// --------------------------------------------------- permissive, and staying
// so: Phase 3 reports at the coordinator boundary instead of rejecting here.

TEST_F( ReadUS3DiskTest, AcceptsMixedRunTypesAndNamesTheRunAfterTheFirst )
{
   const DiskRun run = us3test::writeDiskRun(
      parentDir(), "mixed-type-run", "RA", { DatasetSpec{ 1, 'A', "280" } } );
   ASSERT_FALSE( run.dir.isEmpty() );

   const DiskRun other = us3test::writeDiskRun(
      parentDir(), "other-type-run", "IP", { DatasetSpec{ 2, 'B', "280" } } );
   ASSERT_FALSE( other.dir.isEmpty() );

   ASSERT_TRUE( QFile::copy( other.dir + other.aucFiles[ 0 ],
                             run.dir + other.aucFiles[ 0 ] ) );

   ASSERT_EQ( read( run ), US_Convert::OK );
   EXPECT_EQ( allData.size(), 2 );

   // "mixed-type-run.RA..." sorts before "other-type-run.IP...".
   EXPECT_EQ( runType.toStdString(), "RA" );
}

TEST_F( ReadUS3DiskTest, AcceptsFilesWhoseRunIdDoesNotMatchTheDirectory )
{
   const DiskRun run = us3test::writeDiskRun(
      parentDir(), "host-run", "RA", { DatasetSpec{ 1, 'A', "280" } } );
   const DiskRun guest = us3test::writeDiskRun(
      parentDir(), "guest-run", "RA", { DatasetSpec{ 3, 'A', "280" } } );
   ASSERT_FALSE( run.dir.isEmpty() );
   ASSERT_FALSE( guest.dir.isEmpty() );

   ASSERT_TRUE( QFile::copy( guest.dir + guest.aucFiles[ 0 ],
                             run.dir + guest.aucFiles[ 0 ] ) );

   ASSERT_EQ( read( run ), US_Convert::OK );
   EXPECT_EQ( allData.size(), 2 );
   EXPECT_EQ( triples.size(), 2 );
}

TEST_F( ReadUS3DiskTest, LeavesTripleGuidAndFilenameForTheExperimentToSupply )
{
   // readUS3Disk() names triples from filenames only.  Their GUIDs and
   // tripleFilename come from the experiment XML in the next step, so nothing
   // downstream may assume this call filled them in.
   const DiskRun run = us3test::writeDiskRun(
      parentDir(), "unfilled-run", "RA", { DatasetSpec{ 1, 'A', "280" } } );
   ASSERT_FALSE( run.dir.isEmpty() );

   ASSERT_EQ( read( run ), US_Convert::OK );
   ASSERT_EQ( triples.size(), 1 );

   EXPECT_TRUE( triples[ 0 ].tripleFilename.isEmpty() );
   EXPECT_EQ( triples[ 0 ].tripleID, 0 );
   EXPECT_FALSE( triples[ 0 ].excluded );
}
