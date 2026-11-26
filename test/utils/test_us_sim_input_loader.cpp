//! \file test_us_sim_input_loader.cpp
// US_SimInputLoader: the simulators' command-line model, buffer and rotor
// lookups, from the local data folders (in the test sandbox) and, for the
// refusals, a mock database.
#include "mock_us_db2.h"
#include "qt_test_base.h"
#include "us_settings.h"
#include "us_sim_input_loader.h"

#include <QDir>

using ::testing::_;
using ::testing::Matcher;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::StrictMock;

namespace
{
const QString kModelGuid  = "5f9c1a3e-1111-4c4e-9a1b-000000000001";
const QString kBufferGuid = "5f9c1a3e-2222-4c4e-9a1b-000000000002";

US_Model      syntheticModel( const QString& description )
{
   US_Model model;
   model.modelGUID   = kModelGuid;
   model.description = description;
   US_Model::SimulationComponent component;
   component.s                    = 4.0e-13;
   component.D                    = 6.0e-7;
   component.mw                   = 50000.0;
   component.vbar20               = 0.72;
   component.signal_concentration = 1.0;
   model.components << component;
   return model;
}
} // namespace

class US_SimInputLoaderTest : public QtTestBase
{
};

TEST_F( US_SimInputLoaderTest, RotorCoefficientsAreTwoNumbers )
{
   double c1 = 0.0;
   double c2 = 0.0;
   EXPECT_TRUE( US_SimInputLoader::rotorCoefficients( "1.2e-07:5.7e-12", c1, c2 ) );
   EXPECT_DOUBLE_EQ( c1, 1.2e-07 );
   EXPECT_DOUBLE_EQ( c2, 5.7e-12 );
   EXPECT_FALSE( US_SimInputLoader::rotorCoefficients( "3", c1, c2 ) );
   EXPECT_FALSE( US_SimInputLoader::rotorCoefficients( "1:2:3", c1, c2 ) );
}

TEST_F( US_SimInputLoaderTest, RotorEntriesMatchAnyWholePart )
{
   // As the rotor dialog matches its "ID:name:labID" entries: the ID, the
   // whole name, or the lab ID, never part of a name.
   const QString entry = "3:Default 1003 (Simulation):1";
   EXPECT_TRUE( US_SimInputLoader::rotorEntryMatches( entry, "3" ) );
   EXPECT_TRUE( US_SimInputLoader::rotorEntryMatches( entry, "Default 1003 (Simulation)" ) );
   EXPECT_TRUE( US_SimInputLoader::rotorEntryMatches( entry, "1" ) );
   EXPECT_FALSE( US_SimInputLoader::rotorEntryMatches( entry, "Default" ) );
   EXPECT_FALSE( US_SimInputLoader::rotorEntryMatches( entry, "13" ) );
}

TEST_F( US_SimInputLoaderTest, CoefficientsNeedNoRotorOrDatabase )
{
   US_SimInputLoader::RotorChoice choice;
   QString                        error;
   ASSERT_TRUE( US_SimInputLoader::rotor( "1.0e-07:2.0e-12", nullptr, choice, error ) ) << error.toStdString();
   EXPECT_TRUE( choice.coefficientsOnly );
   EXPECT_DOUBLE_EQ( choice.coeff1, 1.0e-07 );
   EXPECT_DOUBLE_EQ( choice.coeff2, 2.0e-12 );
}

TEST_F( US_SimInputLoaderTest, ALocalModelIsFoundByGuid )
{
   QString path;
   ASSERT_TRUE( US_Model::model_path( path ) );
   US_Model model = syntheticModel( "loader test model" );
   ASSERT_EQ( model.write( path + "/M0000001.xml" ), IUS_DB2::OK );

   US_Model loaded;
   QString  error;
   ASSERT_TRUE( US_SimInputLoader::model( kModelGuid, nullptr, -1, loaded, error ) ) << error.toStdString();
   EXPECT_EQ( loaded.description, QString( "loader test model" ) );
   EXPECT_EQ( loaded.components.size(), 1 );

   EXPECT_FALSE( US_SimInputLoader::model( "no-such-guid", nullptr, -1, loaded, error ) );
   EXPECT_TRUE( error.contains( "no-such-guid" ) ) << error.toStdString();
}

TEST_F( US_SimInputLoaderTest, AModelFilePathIsLoadedDirectly )
{
   const QString file  = US_Settings::tmpDir() + "/loader-model.xml";
   US_Model      model = syntheticModel( "loader file model" );
   ASSERT_EQ( model.write( file ), IUS_DB2::OK );

   StrictMock<US_DB2_Mock> db; // a file path never reaches the database
   US_Model                loaded;
   QString                 error;
   ASSERT_TRUE( US_SimInputLoader::model( file, &db, 7, loaded, error ) ) << error.toStdString();
   EXPECT_EQ( loaded.description, QString( "loader file model" ) );
}

TEST_F( US_SimInputLoaderTest, ALocalBufferIsFoundByGuid )
{
   const QString path = US_Settings::dataDir() + "/buffers";
   ASSERT_TRUE( QDir().mkpath( path ) );
   US_Buffer buffer;
   buffer.GUID        = kBufferGuid;
   buffer.description = "loader test buffer";
   buffer.pH          = 7.0;
   buffer.density     = 0.998234;
   buffer.viscosity   = 1.00194;
   ASSERT_TRUE( buffer.writeToDisk( path + "/B0000001.xml" ) );

   US_Buffer loaded;
   QString   error;
   ASSERT_TRUE( US_SimInputLoader::buffer( kBufferGuid, nullptr, -1, loaded, error ) ) << error.toStdString();
   EXPECT_EQ( loaded.description, QString( "loader test buffer" ) );
   EXPECT_DOUBLE_EQ( loaded.density, 0.998234 );
}

TEST_F( US_SimInputLoaderTest, ADatabaseModelNeedsAnInvestigator )
{
   StrictMock<US_DB2_Mock> db;
   EXPECT_CALL( db, isConnected() ).WillOnce( Return( true ) );
   US_Model loaded;
   QString  error;
   EXPECT_FALSE( US_SimInputLoader::model( kModelGuid, &db, -1, loaded, error ) );
   EXPECT_TRUE( error.contains( "investigator" ) ) << error.toStdString();
}

TEST_F( US_SimInputLoaderTest, AnUnconnectedDatabaseIsReportedAsSuch )
{
   NiceMock<US_DB2_Mock> db;
   ON_CALL( db, isConnected() ).WillByDefault( Return( false ) );
   ON_CALL( db, lastError() ).WillByDefault( Return( QString( "the master password was not entered" ) ) );
   EXPECT_CALL( db, query( Matcher<const QStringList&>( _ ) ) ).Times( 0 );

   US_Model                       model;
   US_Buffer                      buffer;
   US_SimInputLoader::RotorChoice rotor;
   QString                        error;
   EXPECT_FALSE( US_SimInputLoader::model( kModelGuid, &db, 7, model, error ) );
   EXPECT_TRUE( error.contains( "not connected: the master password was not entered" ) ) << error.toStdString();
   EXPECT_FALSE( US_SimInputLoader::buffer( kBufferGuid, &db, 7, buffer, error ) );
   EXPECT_TRUE( error.contains( "not connected" ) ) << error.toStdString();
   EXPECT_FALSE( US_SimInputLoader::rotor( "3", &db, rotor, error ) );
   EXPECT_TRUE( error.contains( "not connected" ) ) << error.toStdString();
}

TEST_F( US_SimInputLoaderTest, AnUnknownDatabaseBufferIsReported )
{
   NiceMock<US_DB2_Mock> db;
   ON_CALL( db, isConnected() ).WillByDefault( Return( true ) );
   EXPECT_CALL( db, query( Matcher<const QStringList&>( QStringList{ "get_buffer_desc", "7" } ) ) );
   EXPECT_CALL( db, query( Matcher<const QStringList&>( QStringList{ "get_bufferID", kBufferGuid } ) ) );
   ON_CALL( db, next() ).WillByDefault( Return( false ) );
   ON_CALL( db, lastErrno() ).WillByDefault( Return( IUS_DB2::NOROWS ) );

   US_Buffer loaded;
   QString   error;
   EXPECT_FALSE( US_SimInputLoader::buffer( kBufferGuid, &db, 7, loaded, error ) );
   EXPECT_TRUE( error.contains( "no buffer" ) ) << error.toStdString();
}
