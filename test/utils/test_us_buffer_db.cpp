//! \file test_us_buffer_db.cpp
#include "mock_us_db2.h"
#include "qt_test_base.h"
#include "us_buffer.h"
#include "us_eprofile.h"
#include "us_settings.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>

using ::testing::_;
using ::testing::AtLeast;
using ::testing::Eq;
using ::testing::InSequence;
using ::testing::Invoke;
using ::testing::Matcher;
using ::testing::Return;
using ::testing::StrictMock;

namespace
{
const QString               kGuid = "0b0f0e0d-1234-4321-a1b2-c3d4e5f60718";

Matcher<const QStringList&> args( const QStringList& expected )
{
   return Matcher<const QStringList&>( Eq( expected ) );
}

QMap<double, double> profileValues( const QString& xml )
{
   QString              copy = xml;
   QMap<double, double> values;
   EXPECT_TRUE( US_ExtProfile::parse_eprofile( copy, nullptr, nullptr, nullptr, values ) );
   return values;
}

QString profileXml( const QMap<double, double>& values )
{
   QString              compType = "Buffer";
   QString              valType  = "molarExtinction";
   QMap<double, double> copy     = values;
   QString              xml;
   US_ExtProfile::xml_eprofile( 41, compType, valType, copy, xml );
   return xml;
}

class BufferDb : public QtTestBase
{
   protected:
      StrictMock<US_DB2_Mock> db;
      US_Buffer               buffer;

      void                    SetUp() override
      {
         ASSERT_NO_FATAL_FAILURE( QtTestBase::SetUp() );
         US_Settings::set_us_inv_ID( 23 );
         removeBlocker();
         ASSERT_TRUE( QDir().mkpath( bufferPath() ) );
         removeDiskCopy();
      }

      void TearDown() override
      {
         removeBlocker();
         removeDiskCopy();
         QtTestBase::TearDown();
      }

      // SaveReportsAFailedDiskCopy puts a file where the buffers directory goes;
      // remove it even when that test stops early so later tests can set up.
      static void removeBlocker()
      {
         if ( QFileInfo( bufferPath() ).isFile() )
         {
            QFile::remove( bufferPath() );
         }
      }

      static QString bufferPath()
      {
         return US_Settings::dataDir() + "/buffers";
      }

      // Name of the file holding kGuid, or an empty string when there is none.
      static QString diskCopy()
      {
         bool          newFile = true;
         const QString name    = US_Buffer::get_filename( bufferPath(), kGuid, newFile );
         return newFile ? QString() : name;
      }

      static void removeDiskCopy()
      {
         const QString name = diskCopy();
         if ( !name.isEmpty() )
         {
            QFile::remove( name );
         }
      }

      void prepareBuffer()
      {
         buffer.GUID            = kGuid;
         buffer.description     = "Phosphate & salt";
         buffer.compressibility = 4.6e-10;
         buffer.pH              = 7.4;
         buffer.density         = 1.00512;
         buffer.viscosity       = 1.0213;
         buffer.manual          = false;
         for ( const auto& entry : { qMakePair( QString( "5" ), 0.15 ), qMakePair( QString( "6" ), 0.025 ) } )
         {
            US_BufferComponent component;
            component.componentID = entry.first;
            buffer.component << component;
            buffer.componentIDs << entry.first;
            buffer.concentration << entry.second;
         }
      }

      // A loaded buffer that a failed readFromDB must leave as it was.
      void prepareLoadedBuffer()
      {
         prepareBuffer();
         buffer.bufferID   = "7";
         buffer.person     = "Previous, Owner";
         buffer.extinction = { { 300.0, 9.0 } };
      }

      void expectLoadedBufferUnchanged()
      {
         EXPECT_EQ( QString( "7" ), buffer.bufferID );
         EXPECT_EQ( kGuid, buffer.GUID );
         EXPECT_EQ( QString( "Phosphate & salt" ), buffer.description );
         EXPECT_DOUBLE_EQ( 7.4, buffer.pH );
         EXPECT_EQ( QString( "Previous, Owner" ), buffer.person );
         EXPECT_EQ( ( QStringList{ "5", "6" } ), buffer.componentIDs );
         EXPECT_EQ( ( QList<double>{ 0.15, 0.025 } ), buffer.concentration );
         EXPECT_EQ( 2, buffer.component.size() );
         EXPECT_EQ( ( QMap<double, double>{ { 300.0, 9.0 } } ), buffer.extinction );
      }

      void expectInfoRow( const QString& description )
      {
         EXPECT_CALL( db, query( args( { "get_buffer_info", "41" } ) ) );
         EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::OK ) );
         EXPECT_CALL( db, next() ).WillOnce( Return( true ) );
         EXPECT_CALL( db, value( 0 ) ).WillOnce( Return( QVariant( kGuid ) ) );
         EXPECT_CALL( db, value( 1 ) ).WillOnce( Return( QVariant( description ) ) );
         EXPECT_CALL( db, value( 2 ) ).WillOnce( Return( QVariant( "4.6e-10" ) ) );
         EXPECT_CALL( db, value( 3 ) ).WillOnce( Return( QVariant( "7.25" ) ) );
         EXPECT_CALL( db, value( 4 ) ).WillOnce( Return( QVariant( "1.0213" ) ) );
         EXPECT_CALL( db, value( 5 ) ).WillOnce( Return( QVariant( "1.00512" ) ) );
         EXPECT_CALL( db, value( 7 ) ).WillOnce( Return( QVariant( "Investigator, Test" ) ) );
      }

      void expectComponentRow( const QString& id, const QString& name )
      {
         EXPECT_CALL( db, query( args( { "get_buffer_component_info", id } ) ) );
         EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::OK ) );
         EXPECT_CALL( db, next() ).WillOnce( Return( true ) );
         EXPECT_CALL( db, value( 0 ) ).WillOnce( Return( QVariant( "mM" ) ) );
         EXPECT_CALL( db, value( 1 ) ).WillOnce( Return( QVariant( name ) ) );
         EXPECT_CALL( db, value( 2 ) ).WillOnce( Return( QVariant( "1 2 3 4 5 6" ) ) );
         EXPECT_CALL( db, value( 3 ) ).WillOnce( Return( QVariant( "6 5 4 3 2 1" ) ) );
         EXPECT_CALL( db, value( 4 ) ).WillOnce( Return( QVariant( "0 to 1" ) ) );
         EXPECT_CALL( db, value( 5 ) ).WillOnce( Return( QVariant( "0" ) ) );
      }

      void expectProfileRead( const QString& id, const QString& xml )
      {
         EXPECT_CALL( db, query( args( { "get_eprofile", id, "Buffer" } ) ) );
         EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::OK ) );
         EXPECT_CALL( db, next() ).WillOnce( Return( true ) );
         EXPECT_CALL( db, value( 0 ) ).WillOnce( Return( QVariant( "9" ) ) );
         EXPECT_CALL( db, value( 1 ) ).WillOnce( Return( QVariant( "molarExtinction" ) ) );
         EXPECT_CALL( db, value( 2 ) ).WillOnce( Return( QVariant( xml ) ) );
      }

      void expectLookup( bool found )
      {
         EXPECT_CALL( db, query( args( { "get_bufferID", kGuid } ) ) );
         EXPECT_CALL( db, lastErrno() ).WillOnce( Return( found ? IUS_DB2::OK : IUS_DB2::NOROWS ) );
      }

      void expectOwnedBegin()
      {
         EXPECT_CALL( db, transactionActive() ).WillOnce( Return( false ) );
         EXPECT_CALL( db, beginTransaction( _ ) ).WillOnce( Return( true ) );
      }

      void expectStatus( const QStringList& query, int status = IUS_DB2::OK )
      {
         EXPECT_CALL( db, statusQuery( args( query ) ) ).WillOnce( Return( status ) );
         if ( status == IUS_DB2::OK )
         {
            EXPECT_CALL( db, lastErrno() ).WillOnce( Return( status ) );
         }
         else
         {
            // Failure branches read the status again for their log line.
            EXPECT_CALL( db, lastErrno() ).Times( AtLeast( 1 ) ).WillRepeatedly( Return( status ) );
         }
      }

      static QStringList newBufferArgs()
      {
         return { "new_buffer", kGuid, "Phosphate & salt", "4.6000e-10", "7.4000", "1.005120", "1.02130", "0", "1", "23" };
      }

      static QStringList updateArgs()
      {
         return { "update_buffer", "41", "Phosphate & salt", "4.6000e-10", "7.4000", "1.005120", "1.02130", "0", "1" };
      }
};

// Load

TEST_F( BufferDb, LoadMapsInfoColumnsComponentsAndProfile )
{
   const QMap<double, double> stored{ { 260.0, 0.5 }, { 280.0, 1.25 } };
   {
      InSequence sequence;
      expectInfoRow( "Phosphate & salt" );
      EXPECT_CALL( db, value( 6 ) ).WillOnce( Return( QVariant( "1" ) ) );
      EXPECT_CALL( db, query( args( { "get_buffer_components", "41" } ) ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::OK ) );
      EXPECT_CALL( db, next() ).WillOnce( Return( true ) );
      EXPECT_CALL( db, value( 0 ) ).WillOnce( Return( QVariant( "5" ) ) );
      EXPECT_CALL( db, value( 4 ) ).WillOnce( Return( QVariant( "0.15" ) ) );
      EXPECT_CALL( db, next() ).WillOnce( Return( true ) );
      EXPECT_CALL( db, value( 0 ) ).WillOnce( Return( QVariant( "6" ) ) );
      EXPECT_CALL( db, value( 4 ) ).WillOnce( Return( QVariant( "0.025" ) ) );
      EXPECT_CALL( db, next() ).WillOnce( Return( false ) );
      expectComponentRow( "5", "Sodium chloride" );
      expectComponentRow( "6", "Glycerol" );
      expectProfileRead( "41", profileXml( stored ) );
   }
   buffer.componentIDs << "stale";
   buffer.concentration << 9.0;

   ASSERT_TRUE( buffer.readFromDB( &db, "41" ) );

   EXPECT_EQ( QString( "41" ), buffer.bufferID );
   EXPECT_EQ( kGuid, buffer.GUID );
   EXPECT_EQ( QString( "Phosphate & salt" ), buffer.description );
   EXPECT_DOUBLE_EQ( 4.6e-10, buffer.compressibility );
   EXPECT_DOUBLE_EQ( 7.25, buffer.pH );
   EXPECT_DOUBLE_EQ( 1.0213, buffer.viscosity );
   EXPECT_DOUBLE_EQ( 1.00512, buffer.density );
   EXPECT_EQ( QString( "Investigator, Test" ), buffer.person );
   EXPECT_TRUE( buffer.manual );
   EXPECT_EQ( ( QStringList{ "5", "6" } ), buffer.componentIDs );
   EXPECT_EQ( ( QList<double>{ 0.15, 0.025 } ), buffer.concentration );
   ASSERT_EQ( 2, buffer.component.size() );
   EXPECT_EQ( QString( "5" ), buffer.component[0].componentID );
   EXPECT_EQ( QString( "Sodium chloride" ), buffer.component[0].name );
   EXPECT_EQ( QString( "6" ), buffer.component[1].componentID );
   EXPECT_EQ( QString( "Glycerol" ), buffer.component[1].name );
   EXPECT_DOUBLE_EQ( 6.0, buffer.component[1].visc_coeff[5] );
   EXPECT_EQ( stored, buffer.extinction );
}

TEST_F( BufferDb, LoadTakesTheManualFlagFromTheDescriptionMarker )
{
   {
      InSequence sequence;
      expectInfoRow( "Phosphate  [M]" );
      // The marker decides; the stored flag column is not read.
      EXPECT_CALL( db, query( args( { "get_buffer_components", "41" } ) ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::NOROWS ) );
      EXPECT_CALL( db, next() ).WillOnce( Return( false ) );
      expectProfileRead( "41", profileXml( { { 280.0, 1.0 } } ) );
   }
   EXPECT_CALL( db, value( 6 ) ).Times( 0 );

   ASSERT_TRUE( buffer.readFromDB( &db, "41" ) );

   EXPECT_TRUE( buffer.manual );
   EXPECT_EQ( QString( "Phosphate" ), buffer.description );
   EXPECT_TRUE( buffer.componentIDs.isEmpty() );
   EXPECT_TRUE( buffer.component.isEmpty() );
}

TEST_F( BufferDb, LoadStopsAtAFailedInfoQueryAndKeepsTheBuffer )
{
   prepareBuffer();
   buffer.bufferID = "7";
   {
      InSequence sequence;
      EXPECT_CALL( db, query( args( { "get_buffer_info", "41" } ) ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::DBERROR ) );
   }

   EXPECT_FALSE( buffer.readFromDB( &db, "41" ) );

   EXPECT_EQ( QString( "7" ), buffer.bufferID );
   EXPECT_EQ( QString( "Phosphate & salt" ), buffer.description );
   EXPECT_EQ( ( QStringList{ "5", "6" } ), buffer.componentIDs );
}

TEST_F( BufferDb, LoadWithoutABufferRowFailsAndKeepsTheBuffer )
{
   prepareLoadedBuffer();
   {
      InSequence sequence;
      EXPECT_CALL( db, query( args( { "get_buffer_info", "41" } ) ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::OK ) );
      EXPECT_CALL( db, next() ).WillOnce( Return( false ) );
   }

   EXPECT_FALSE( buffer.readFromDB( &db, "41" ) );

   expectLoadedBufferUnchanged();
}

TEST_F( BufferDb, LoadWithoutAStoredProfileHasNoExtinction )
{
   prepareLoadedBuffer();
   {
      InSequence sequence;
      expectInfoRow( "Phosphate & salt" );
      EXPECT_CALL( db, value( 6 ) ).WillOnce( Return( QVariant( "0" ) ) );
      EXPECT_CALL( db, query( args( { "get_buffer_components", "41" } ) ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::NOROWS ) );
      EXPECT_CALL( db, next() ).WillOnce( Return( false ) );
      EXPECT_CALL( db, query( args( { "get_eprofile", "41", "Buffer" } ) ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::NOROWS ) );
   }

   ASSERT_TRUE( buffer.readFromDB( &db, "41" ) );

   EXPECT_EQ( QString( "41" ), buffer.bufferID );
   EXPECT_TRUE( buffer.componentIDs.isEmpty() );
   EXPECT_TRUE( buffer.extinction.isEmpty() );
}

TEST_F( BufferDb, LoadStopsAtAFailedComponentQuery )
{
   prepareLoadedBuffer();
   {
      InSequence sequence;
      expectInfoRow( "Phosphate & salt" );
      EXPECT_CALL( db, value( 6 ) ).WillOnce( Return( QVariant( "0" ) ) );
      EXPECT_CALL( db, query( args( { "get_buffer_components", "41" } ) ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::DBERROR ) );
   }

   EXPECT_FALSE( buffer.readFromDB( &db, "41" ) );

   expectLoadedBufferUnchanged();
}

TEST_F( BufferDb, LoadFailsWhenAComponentCannotBeRead )
{
   prepareLoadedBuffer();
   {
      InSequence sequence;
      expectInfoRow( "Phosphate & salt" );
      EXPECT_CALL( db, value( 6 ) ).WillOnce( Return( QVariant( "0" ) ) );
      EXPECT_CALL( db, query( args( { "get_buffer_components", "41" } ) ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::OK ) );
      EXPECT_CALL( db, next() ).WillOnce( Return( true ) );
      EXPECT_CALL( db, value( 0 ) ).WillOnce( Return( QVariant( "5" ) ) );
      EXPECT_CALL( db, value( 4 ) ).WillOnce( Return( QVariant( "0.15" ) ) );
      EXPECT_CALL( db, next() ).WillOnce( Return( true ) );
      EXPECT_CALL( db, value( 0 ) ).WillOnce( Return( QVariant( "6" ) ) );
      EXPECT_CALL( db, value( 4 ) ).WillOnce( Return( QVariant( "0.025" ) ) );
      EXPECT_CALL( db, next() ).WillOnce( Return( false ) );
      EXPECT_CALL( db, query( args( { "get_buffer_component_info", "5" } ) ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::NOROWS ) );
   }
   // Neither the second component nor the profile is read.

   EXPECT_FALSE( buffer.readFromDB( &db, "41" ) );

   expectLoadedBufferUnchanged();
}

// Save

TEST_F( BufferDb, SaveCreatesANewBufferWithComponentsProfileAndDiskCopy )
{
   prepareBuffer();
   buffer.extinction              = { { 260.0, 0.5 }, { 280.0, 1.25 } };
   buffer.new_or_changed_spectrum = true;
   QString sentProfile;
   {
      InSequence sequence;
      expectLookup( false );
      expectOwnedBegin();
      expectStatus( newBufferArgs() );
      EXPECT_CALL( db, lastInsertID() ).WillOnce( Return( 88 ) );
      expectStatus( { "add_buffer_component", "88", "5", "0.15000" } );
      expectStatus( { "add_buffer_component", "88", "6", "0.02500" } );
      EXPECT_CALL( db, query( args( { "get_eprofile", "88", "Buffer" } ) ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::NOROWS ) );
      EXPECT_CALL( db, statusQuery( Matcher<const QStringList&>( _ ) ) )
         .WillOnce( Invoke(
            [&sentProfile]( const QStringList& query )
            {
               EXPECT_EQ( ( QStringList{ "new_eprofile", "88", "Buffer", "molarExtinction" } ), query.mid( 0, 4 ) );
               sentProfile = query.value( 4 );
               return IUS_DB2::OK;
            } ) );
      EXPECT_CALL( db, lastInsertID() ).WillOnce( Return( 9 ) );
      EXPECT_CALL( db, commitTransaction( _ ) )
         .WillOnce( Invoke(
            [&]()
            {
               EXPECT_TRUE( diskCopy().isEmpty() );
               return true;
            } ) );
   }

   EXPECT_EQ( 88, buffer.saveToDB( &db, "1" ) );

   EXPECT_EQ( QString( "88" ), buffer.bufferID );
   EXPECT_FALSE( buffer.new_or_changed_spectrum );
   EXPECT_EQ( buffer.extinction, profileValues( sentProfile ) );

   const QString file = diskCopy();
   ASSERT_FALSE( file.isEmpty() );
   US_Buffer disk;
   ASSERT_TRUE( disk.readFromDisk( file ) );
   EXPECT_EQ( kGuid, disk.GUID );
   EXPECT_EQ( QString( "Phosphate & salt" ), disk.description );
   EXPECT_DOUBLE_EQ( 7.4, disk.pH );
   EXPECT_EQ( ( QStringList{ "5", "6" } ), disk.componentIDs );
   EXPECT_EQ( ( QList<double>{ 0.15, 0.025 } ), disk.concentration );
   EXPECT_EQ( buffer.extinction, disk.extinction );
}

TEST_F( BufferDb, SaveReportsAFailedProfileWriteAndKeepsTheSpectrumPending )
{
   prepareBuffer();
   buffer.component.clear();
   buffer.componentIDs.clear();
   buffer.concentration.clear();
   buffer.extinction              = { { 280.0, 1.25 } };
   buffer.new_or_changed_spectrum = true;
   {
      InSequence sequence;
      expectLookup( false );
      expectOwnedBegin();
      expectStatus( newBufferArgs() );
      EXPECT_CALL( db, lastInsertID() ).WillOnce( Return( 88 ) );
      EXPECT_CALL( db, query( args( { "get_eprofile", "88", "Buffer" } ) ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::NOROWS ) );
      EXPECT_CALL( db, statusQuery( Matcher<const QStringList&>( _ ) ) ).WillOnce( Return( IUS_DB2::DBERROR ) );
      EXPECT_CALL( db, rollbackTransaction( _ ) ).WillOnce( Return( true ) );
   }

   EXPECT_EQ( -5, buffer.saveToDB( &db, "1" ) );
   EXPECT_TRUE( buffer.new_or_changed_spectrum );
   EXPECT_TRUE( diskCopy().isEmpty() );
}

TEST_F( BufferDb, SaveCreatesAMissingBuffersDirectory )
{
   prepareBuffer();
   buffer.component.clear();
   buffer.componentIDs.clear();
   buffer.concentration.clear();
   ASSERT_TRUE( QDir( bufferPath() ).removeRecursively() );
   {
      InSequence sequence;
      expectLookup( false );
      expectOwnedBegin();
      expectStatus( newBufferArgs() );
      EXPECT_CALL( db, lastInsertID() ).WillOnce( Return( 88 ) );
      EXPECT_CALL( db, commitTransaction( _ ) )
         .WillOnce( Invoke(
            [&]()
            {
               EXPECT_TRUE( diskCopy().isEmpty() );
               return true;
            } ) );
   }

   QString error = "stale error";
   EXPECT_EQ( 88, buffer.saveToDB( &db, "1", &error ) );
   EXPECT_TRUE( error.isEmpty() );
   EXPECT_FALSE( diskCopy().isEmpty() );
}

TEST_F( BufferDb, SaveReportsAFailedDiskCopy )
{
   prepareBuffer();
   buffer.component.clear();
   buffer.componentIDs.clear();
   buffer.concentration.clear();
   // A file where the buffers directory should be makes the disk copy impossible.
   ASSERT_TRUE( QDir( bufferPath() ).removeRecursively() );
   {
      QFile blocker( bufferPath() );
      ASSERT_TRUE( blocker.open( QIODevice::WriteOnly ) );
   }
   {
      InSequence sequence;
      expectLookup( false );
      expectOwnedBegin();
      expectStatus( newBufferArgs() );
      EXPECT_CALL( db, lastInsertID() ).WillOnce( Return( 88 ) );
      EXPECT_CALL( db, commitTransaction( _ ) )
         .WillOnce( Invoke(
            [&]()
            {
               EXPECT_TRUE( diskCopy().isEmpty() );
               return true;
            } ) );
   }

   QString error;
   EXPECT_EQ( -6, buffer.saveToDB( &db, "1", &error ) );
   EXPECT_TRUE( error.contains( bufferPath() ) );
   EXPECT_EQ( QString( "88" ), buffer.bufferID );
}

TEST_F( BufferDb, DiskCopyKeepsEachSpectrumAtItsOwnWavelengths )
{
   prepareBuffer();
   buffer.extinction   = { { 250.0, 0.75 }, { 280.0, 1.25 } };
   buffer.refraction   = { { 600.0, 1.333 } };
   buffer.fluorescence = { { 350.0, 0.5 }, { 450.0, 0.25 }, { 550.0, 0.125 } };
   const QString file  = bufferPath() + "/spectra.xml";
   ASSERT_TRUE( buffer.writeToDisk( file ) );

   US_Buffer disk;
   ASSERT_TRUE( disk.readFromDisk( file ) );
   EXPECT_EQ( buffer.extinction, disk.extinction );
   EXPECT_EQ( buffer.refraction, disk.refraction );
   EXPECT_EQ( buffer.fluorescence, disk.fluorescence );
   ASSERT_TRUE( QFile::remove( file ) );
}

TEST_F( BufferDb, SaveUpdatesAnExistingBufferAndReplacesItsComponents )
{
   prepareBuffer();
   buffer.description             = "Phosphate & salt  [M]";
   buffer.extinction              = { { 280.0, 1.25 } };
   buffer.new_or_changed_spectrum = false;
   {
      InSequence sequence;
      expectLookup( true );
      EXPECT_CALL( db, next() ).WillOnce( Return( true ) );
      EXPECT_CALL( db, value( 0 ) ).WillOnce( Return( QVariant( "41" ) ) );
      // The manual marker is stripped from the stored description.
      expectOwnedBegin();
      expectStatus( updateArgs() );
      expectStatus( { "delete_buffer_components", "41" }, IUS_DB2::OK );
      expectStatus( { "add_buffer_component", "41", "5", "0.15000" } );
      expectStatus( { "add_buffer_component", "41", "6", "0.02500" } );
      EXPECT_CALL( db, commitTransaction( _ ) )
         .WillOnce( Invoke(
            [&]()
            {
               EXPECT_TRUE( diskCopy().isEmpty() );
               return true;
            } ) );
   }
   // An unchanged spectrum is not rewritten.
   EXPECT_CALL( db, lastInsertID() ).Times( 0 );

   EXPECT_EQ( 41, buffer.saveToDB( &db, "1" ) );

   EXPECT_EQ( QString( "41" ), buffer.bufferID );
   EXPECT_FALSE( diskCopy().isEmpty() );
}

TEST_F( BufferDb, SaveTreatsNoComponentsToDeleteAsSuccess )
{
   prepareBuffer();
   buffer.component.clear();
   buffer.componentIDs.clear();
   buffer.concentration.clear();
   {
      InSequence sequence;
      expectLookup( true );
      EXPECT_CALL( db, next() ).WillOnce( Return( true ) );
      EXPECT_CALL( db, value( 0 ) ).WillOnce( Return( QVariant( "41" ) ) );
      expectOwnedBegin();
      expectStatus( updateArgs() );
      EXPECT_CALL( db, statusQuery( args( { "delete_buffer_components", "41" } ) ) ).WillOnce( Return( IUS_DB2::NOROWS ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::NOROWS ) );
      EXPECT_CALL( db, commitTransaction( _ ) )
         .WillOnce( Invoke(
            [&]()
            {
               EXPECT_TRUE( diskCopy().isEmpty() );
               return true;
            } ) );
   }

   EXPECT_EQ( 41, buffer.saveToDB( &db, "1" ) );
}

TEST_F( BufferDb, SaveReplacesTheStoredProfileWithTheNewValues )
{
   prepareBuffer();
   buffer.component.clear();
   buffer.componentIDs.clear();
   buffer.concentration.clear();
   const QMap<double, double> replacement{ { 250.0, 0.75 }, { 280.0, 2.5 } };
   buffer.extinction              = replacement;
   buffer.new_or_changed_spectrum = true;
   buffer.replace_spectrum        = true;
   QString sentProfile;
   {
      InSequence sequence;
      expectLookup( true );
      EXPECT_CALL( db, next() ).WillOnce( Return( true ) );
      EXPECT_CALL( db, value( 0 ) ).WillOnce( Return( QVariant( "41" ) ) );
      expectOwnedBegin();
      expectStatus( updateArgs() );
      expectStatus( { "delete_buffer_components", "41" } );
      expectProfileRead( "41", profileXml( { { 300.0, 9.0 } } ) );
      EXPECT_CALL( db, statusQuery( Matcher<const QStringList&>( _ ) ) )
         .WillOnce( Invoke(
            [&sentProfile]( const QStringList& query )
            {
               EXPECT_EQ( ( QStringList{ "update_eprofile", "9", "41", "Buffer", "molarExtinction" } ), query.mid( 0, 5 ) );
               sentProfile = query.value( 5 );
               return IUS_DB2::OK;
            } ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::OK ) );
      EXPECT_CALL( db, commitTransaction( _ ) )
         .WillOnce( Invoke(
            [&]()
            {
               EXPECT_TRUE( diskCopy().isEmpty() );
               return true;
            } ) );
   }

   EXPECT_EQ( 41, buffer.saveToDB( &db, "1" ) );

   EXPECT_EQ( replacement, profileValues( sentProfile ) );
   EXPECT_EQ( replacement, buffer.extinction );
   EXPECT_FALSE( buffer.replace_spectrum );
   EXPECT_FALSE( buffer.new_or_changed_spectrum );
}

TEST_F( BufferDb, SaveInsertsMissingProfilesButUpdatesMalformedStoredProfiles )
{
   const QMap<double, double> replacement{ { 250.0, 0.75 }, { 280.0, 2.5 } };
   // No stored row, or a row whose XML holds no profile: either way the profile is created.
   for ( const QString malformed : { QString(), QString( "corrupt" ), QString( "<extinctionProfile/><!--trailing--><extra/>" ) } )
   {
      removeDiskCopy();
      const bool unreadable = !malformed.isEmpty();
      SCOPED_TRACE( malformed.toStdString() );
      prepareBuffer();
      buffer.component.clear();
      buffer.componentIDs.clear();
      buffer.concentration.clear();
      buffer.extinction              = replacement;
      buffer.new_or_changed_spectrum = true;
      buffer.replace_spectrum        = true;
      QString sentProfile;
      {
         InSequence sequence;
         expectLookup( true );
         EXPECT_CALL( db, next() ).WillOnce( Return( true ) );
         EXPECT_CALL( db, value( 0 ) ).WillOnce( Return( QVariant( "41" ) ) );
         expectOwnedBegin();
         expectStatus( updateArgs() );
         expectStatus( { "delete_buffer_components", "41" } );
         EXPECT_CALL( db, query( args( { "get_eprofile", "41", "Buffer" } ) ) );
         if ( unreadable )
         {
            EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::OK ) );
            EXPECT_CALL( db, next() ).WillOnce( Return( true ) );
            EXPECT_CALL( db, value( 0 ) ).WillOnce( Return( QVariant( "9" ) ) );
            EXPECT_CALL( db, value( 1 ) ).WillOnce( Return( QVariant( "molarExtinction" ) ) );
            EXPECT_CALL( db, value( 2 ) ).WillOnce( Return( QVariant( malformed ) ) );
            // The create path finds the stored row and updates it in place.
            EXPECT_CALL( db, query( args( { "get_eprofile", "41", "Buffer" } ) ) );
            EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::OK ) );
            EXPECT_CALL( db, next() ).WillOnce( Return( true ) );
            EXPECT_CALL( db, value( 0 ) ).WillOnce( Return( QVariant( "9" ) ) );
         }
         else
         {
            EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::NOROWS ) );
            EXPECT_CALL( db, query( args( { "get_eprofile", "41", "Buffer" } ) ) );
            EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::NOROWS ) );
         }
         EXPECT_CALL( db, statusQuery( Matcher<const QStringList&>( _ ) ) )
            .WillOnce( Invoke(
               [&sentProfile, unreadable]( const QStringList& query )
               {
                  const QStringList head = unreadable ? QStringList{ "update_eprofile", "9", "41", "Buffer", "molarExtinction" }
                                                      : QStringList{ "new_eprofile", "41", "Buffer", "molarExtinction" };
                  EXPECT_EQ( head, query.mid( 0, head.size() ) );
                  sentProfile = query.value( head.size() );
                  return IUS_DB2::OK;
               } ) );
         if ( !unreadable )
         {
            EXPECT_CALL( db, lastInsertID() ).WillOnce( Return( 9 ) );
         }
         EXPECT_CALL( db, commitTransaction( _ ) )
            .WillOnce( Invoke(
               [&]()
               {
                  EXPECT_TRUE( diskCopy().isEmpty() );
                  return true;
               } ) );
      }

      EXPECT_EQ( 41, buffer.saveToDB( &db, "1" ) );

      EXPECT_EQ( replacement, profileValues( sentProfile ) );
      EXPECT_EQ( replacement, buffer.extinction );
      EXPECT_FALSE( buffer.replace_spectrum );
      ::testing::Mock::VerifyAndClearExpectations( &db );
   }
}

TEST_F( BufferDb, FailedLookupStopsBeforeAnyWrite )
{
   prepareBuffer();
   buffer.bufferID = "7";
   {
      InSequence sequence;
      EXPECT_CALL( db, query( args( { "get_bufferID", kGuid } ) ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::DBERROR ) );
   }

   EXPECT_EQ( -9, buffer.saveToDB( &db, "1" ) );

   EXPECT_EQ( QString( "7" ), buffer.bufferID );
   EXPECT_TRUE( diskCopy().isEmpty() );
}

TEST_F( BufferDb, FailedCreateKeepsTheIdAndWritesNothingElse )
{
   prepareBuffer();
   buffer.bufferID                = "7";
   buffer.extinction              = { { 280.0, 1.25 } };
   buffer.new_or_changed_spectrum = true;
   {
      InSequence sequence;
      expectLookup( false );
      expectOwnedBegin();
      expectStatus( newBufferArgs(), IUS_DB2::DBERROR );
      EXPECT_CALL( db, lastError() ).WillOnce( Return( QString( "insert permission denied" ) ) );
      EXPECT_CALL( db, rollbackTransaction( _ ) )
         .WillOnce( Invoke(
            []( QString& e )
            {
               e.clear();
               return true;
            } ) );
   }
   EXPECT_CALL( db, lastInsertID() ).Times( 0 );

   QString error;
   EXPECT_EQ( -1, buffer.saveToDB( &db, "1", &error ) );
   EXPECT_EQ( QString( "insert permission denied" ), error );

   EXPECT_EQ( QString( "7" ), buffer.bufferID );
   EXPECT_TRUE( buffer.new_or_changed_spectrum );
   EXPECT_TRUE( diskCopy().isEmpty() );
}

TEST_F( BufferDb, FailedUpdateStopsBeforeTheComponents )
{
   prepareBuffer();
   {
      InSequence sequence;
      expectLookup( true );
      EXPECT_CALL( db, next() ).WillOnce( Return( true ) );
      EXPECT_CALL( db, value( 0 ) ).WillOnce( Return( QVariant( "41" ) ) );
      expectOwnedBegin();
      expectStatus( updateArgs(), IUS_DB2::DBERROR );
      EXPECT_CALL( db, rollbackTransaction( _ ) ).WillOnce( Return( true ) );
   }

   EXPECT_EQ( -2, buffer.saveToDB( &db, "1" ) );
   EXPECT_TRUE( diskCopy().isEmpty() );
}

TEST_F( BufferDb, FailedComponentDeleteStopsBeforeTheNewComponents )
{
   prepareBuffer();
   {
      InSequence sequence;
      expectLookup( true );
      EXPECT_CALL( db, next() ).WillOnce( Return( true ) );
      EXPECT_CALL( db, value( 0 ) ).WillOnce( Return( QVariant( "41" ) ) );
      expectOwnedBegin();
      expectStatus( updateArgs() );
      expectStatus( { "delete_buffer_components", "41" }, IUS_DB2::DBERROR );
      EXPECT_CALL( db, rollbackTransaction( _ ) ).WillOnce( Return( true ) );
   }

   EXPECT_EQ( -3, buffer.saveToDB( &db, "1" ) );
   EXPECT_TRUE( diskCopy().isEmpty() );
}

TEST_F( BufferDb, FailedComponentStopsBeforeTheNextComponentAndTheProfile )
{
   prepareBuffer();
   buffer.extinction              = { { 280.0, 1.25 } };
   buffer.new_or_changed_spectrum = true;
   {
      InSequence sequence;
      expectLookup( false );
      expectOwnedBegin();
      expectStatus( newBufferArgs() );
      EXPECT_CALL( db, lastInsertID() ).WillOnce( Return( 88 ) );
      expectStatus( { "add_buffer_component", "88", "5", "0.15000" }, IUS_DB2::DBERROR );
      EXPECT_CALL( db, rollbackTransaction( _ ) ).WillOnce( Return( true ) );
   }

   EXPECT_EQ( -4, buffer.saveToDB( &db, "1" ) );

   EXPECT_TRUE( buffer.new_or_changed_spectrum );
   EXPECT_TRUE( diskCopy().isEmpty() );
}

TEST_F( BufferDb, FailedBeginDoesNotWriteOrRollback )
{
   prepareBuffer();
   InSequence sequence;
   EXPECT_CALL( db, query( args( { "get_bufferID", kGuid } ) ) );
   EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::NOROWS ) );
   EXPECT_CALL( db, transactionActive() ).WillOnce( Return( false ) );
   EXPECT_CALL( db, beginTransaction( _ ) )
      .WillOnce( Invoke(
         []( QString& e )
         {
            e = "cannot begin on disconnected database";
            return false;
         } ) );
   QString error = "stale error";
   EXPECT_EQ( -7, buffer.saveToDB( &db, "1", &error ) );
   EXPECT_EQ( QString( "cannot begin on disconnected database" ), error );
   EXPECT_TRUE( diskCopy().isEmpty() );
}

TEST_F( BufferDb, FailedSecondComponentRollsBackInsertAndUpdate )
{
   for ( bool existing : { false, true } )
   {
      SCOPED_TRACE( existing );
      buffer = US_Buffer();
      prepareBuffer();
      buffer.bufferID                = "7";
      buffer.new_or_changed_spectrum = true;
      buffer.replace_spectrum        = true;
      {
         InSequence sequence;
         expectLookup( existing );
         if ( existing )
         {
            EXPECT_CALL( db, next() ).WillOnce( Return( true ) );
            EXPECT_CALL( db, value( 0 ) ).WillOnce( Return( QVariant( "41" ) ) );
            expectOwnedBegin();
            expectStatus( updateArgs() );
            expectStatus( { "delete_buffer_components", "41" } );
         }
         else
         {
            expectOwnedBegin();
            expectStatus( newBufferArgs() );
            EXPECT_CALL( db, lastInsertID() ).WillOnce( Return( 41 ) );
         }
         expectStatus( { "add_buffer_component", "41", "5", "0.15000" } );
         expectStatus( { "add_buffer_component", "41", "6", "0.02500" }, IUS_DB2::DBERROR );
         EXPECT_CALL( db, rollbackTransaction( _ ) ).WillOnce( Return( true ) );
      }
      EXPECT_EQ( -4, buffer.saveToDB( &db, "1" ) );
      EXPECT_EQ( QString( "7" ), buffer.bufferID );
      EXPECT_TRUE( buffer.new_or_changed_spectrum );
      EXPECT_TRUE( buffer.replace_spectrum );
      EXPECT_TRUE( diskCopy().isEmpty() );
      ASSERT_TRUE( ::testing::Mock::VerifyAndClearExpectations( &db ) );
   }
}

TEST_F( BufferDb, FailedCommitOrRollbackDoesNotPublishIdOrDiskCopy )
{
   for ( bool rollbackOK : { true, false } )
   {
      buffer = US_Buffer();
      prepareBuffer();
      buffer.bufferID = "7";
      buffer.component.clear();
      {
         InSequence sequence;
         expectLookup( false );
         expectOwnedBegin();
         expectStatus( newBufferArgs() );
         EXPECT_CALL( db, lastInsertID() ).WillOnce( Return( 88 ) );
         EXPECT_CALL( db, commitTransaction( _ ) )
            .WillOnce( Invoke(
               []( QString& e )
               {
                  e = "connection lost during commit";
                  return false;
               } ) );
         EXPECT_CALL( db, rollbackTransaction( _ ) )
            .WillOnce( Invoke(
               [&]( QString& e )
               {
                  e = rollbackOK ? QString() : QString( "rollback connection lost" );
                  return rollbackOK;
               } ) );
      }
      QString error;
      EXPECT_EQ( rollbackOK ? -8 : -10, buffer.saveToDB( &db, "1", &error ) );
      EXPECT_TRUE( error.contains( "connection lost during commit" ) );
      EXPECT_EQ( !rollbackOK, error.contains( "rollback connection lost" ) );
      EXPECT_EQ( QString( "7" ), buffer.bufferID );
      EXPECT_TRUE( diskCopy().isEmpty() );
      ASSERT_TRUE( ::testing::Mock::VerifyAndClearExpectations( &db ) );
   }
}

TEST_F( BufferDb, FailedCommitKeepsWrittenSpectrumPendingAndPreservesDiskCopy )
{
   prepareBuffer();
   buffer.bufferID        = "7";
   const QString filename = bufferPath() + "/B0000001.xml";
   ASSERT_TRUE( buffer.writeToDisk( filename ) );
   QFile before( filename );
   ASSERT_TRUE( before.open( QIODevice::ReadOnly ) );
   const QByteArray previous = before.readAll();
   before.close();

   buffer.component.clear();
   buffer.extinction              = { { 280.0, 1.25 } };
   buffer.new_or_changed_spectrum = true;
   buffer.replace_spectrum        = false;
   {
      InSequence sequence;
      expectLookup( false );
      expectOwnedBegin();
      expectStatus( newBufferArgs() );
      EXPECT_CALL( db, lastInsertID() ).WillOnce( Return( 88 ) );
      EXPECT_CALL( db, query( args( { "get_eprofile", "88", "Buffer" } ) ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::NOROWS ) );
      EXPECT_CALL( db, statusQuery( Matcher<const QStringList&>( _ ) ) )
         .WillOnce( Invoke(
            []( const QStringList& query )
            {
               EXPECT_EQ( ( QStringList{ "new_eprofile", "88", "Buffer", "molarExtinction" } ), query.mid( 0, 4 ) );
               EXPECT_EQ( ( QMap<double, double>{ { 280.0, 1.25 } } ), profileValues( query.value( 4 ) ) );
               return IUS_DB2::OK;
            } ) );
      EXPECT_CALL( db, lastInsertID() ).WillOnce( Return( 9 ) );
      EXPECT_CALL( db, commitTransaction( _ ) ).WillOnce( Return( false ) );
      EXPECT_CALL( db, rollbackTransaction( _ ) ).WillOnce( Return( true ) );
   }
   EXPECT_EQ( -8, buffer.saveToDB( &db, "1" ) );
   EXPECT_EQ( QString( "7" ), buffer.bufferID );
   EXPECT_TRUE( buffer.new_or_changed_spectrum );
   EXPECT_FALSE( buffer.replace_spectrum );
   QFile after( filename );
   ASSERT_TRUE( after.open( QIODevice::ReadOnly ) );
   EXPECT_EQ( previous, after.readAll() );
}

TEST_F( BufferDb, ParticipatesInCallerTransactionWithoutCommittingOrWritingDisk )
{
   prepareBuffer();
   buffer.component.clear();
   buffer.extinction              = { { 280.0, 1.25 } };
   buffer.new_or_changed_spectrum = true;
   buffer.replace_spectrum        = false;
   {
      InSequence sequence;
      expectLookup( false );
      EXPECT_CALL( db, transactionActive() ).WillOnce( Return( true ) );
      EXPECT_CALL( db, statusQuery( args( newBufferArgs() ) ) ).WillOnce( Return( IUS_DB2::OK ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::OK ) );
      EXPECT_CALL( db, lastInsertID() ).WillOnce( Return( 88 ) );
      EXPECT_CALL( db, query( args( { "get_eprofile", "88", "Buffer" } ) ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::NOROWS ) );
      EXPECT_CALL( db, statusQuery( Matcher<const QStringList&>( _ ) ) )
         .WillOnce( Invoke(
            []( const QStringList& q )
            {
               EXPECT_EQ( ( QStringList{ "new_eprofile", "88", "Buffer", "molarExtinction" } ), q.mid( 0, 4 ) );
               return IUS_DB2::OK;
            } ) );
      EXPECT_CALL( db, lastInsertID() ).WillOnce( Return( 9 ) );
   }
   EXPECT_EQ( 88, buffer.saveToDB( &db, "1" ) );
   EXPECT_EQ( QString( "88" ), buffer.bufferID );
   EXPECT_TRUE( buffer.new_or_changed_spectrum );
   EXPECT_TRUE( diskCopy().isEmpty() );
   // StrictMock rejects any begin, commit, or rollback by this participant.
}

TEST_F( BufferDb, ParticipatingFailureLeavesRollbackToCaller )
{
   prepareBuffer();
   buffer.bufferID = "7";
   {
      InSequence sequence;
      expectLookup( false );
      EXPECT_CALL( db, transactionActive() ).WillOnce( Return( true ) );
      EXPECT_CALL( db, statusQuery( args( newBufferArgs() ) ) ).WillOnce( Return( IUS_DB2::OK ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::OK ) );
      EXPECT_CALL( db, lastInsertID() ).WillOnce( Return( 88 ) );
      expectStatus( { "add_buffer_component", "88", "5", "0.15000" } );
      expectStatus( { "add_buffer_component", "88", "6", "0.02500" }, IUS_DB2::DBERROR );
   }
   EXPECT_EQ( -4, buffer.saveToDB( &db, "1" ) );
   EXPECT_EQ( QString( "7" ), buffer.bufferID );
   EXPECT_TRUE( diskCopy().isEmpty() );
}
}
