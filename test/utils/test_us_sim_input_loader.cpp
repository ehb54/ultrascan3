//! \file test_us_sim_input_loader.cpp
// US_SimInputLoader: the simulators' command-line model, buffer and rotor
// lookups, from the local data folders (in the test sandbox) and from a
// scripted database.
#include "mock_us_db2.h"
#include "qt_test_base.h"
#include "us_rotor.h"
#include "us_settings.h"
#include "us_sim_input_loader.h"

#include <QDir>
#include <QFile>
#include <QMap>
#include <QVector>

using ::testing::_;
using ::testing::Invoke;
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
// Answers each procedure call with scripted rows, keyed by the whole call
// ("get_rotor_info 3"); anything not scripted returns no rows. A status other
// than OK makes the call fail.
class ScriptedDb : public NiceMock<US_DB2_Mock>
{
   public:
      ScriptedDb()
      {
         ON_CALL( *this, isConnected() ).WillByDefault( Return( true ) );
         ON_CALL( *this, query( Matcher<const QStringList&>( _ ) ) )
            .WillByDefault( Invoke(
               [this]( const QStringList& q )
               {
                  const QString key = q.join( " " );
                  calls << key;
                  current = rows.value( key );
                  status  = statuses.value( key, current.isEmpty() ? IUS_DB2::NOROWS : IUS_DB2::OK );
                  row     = -1;
               } ) );
         ON_CALL( *this, next() )
            .WillByDefault( Invoke(
               [this]
               {
                  if ( status != IUS_DB2::OK || row + 1 >= current.size() )
                  {
                     return false;
                  }
                  row++;
                  return true;
               } ) );
         ON_CALL( *this, value( _ ) )
            .WillByDefault( Invoke(
               [this]( unsigned index )
               { return row >= 0 && row < current.size() ? current[row].value( static_cast<int>( index ) ) : QVariant(); } ) );
         ON_CALL( *this, lastErrno() ).WillByDefault( Invoke( [this] { return status; } ) );
         ON_CALL( *this, lastError() )
            .WillByDefault( Invoke(
               [this]
               { return status == IUS_DB2::OK || status == IUS_DB2::NOROWS ? QString() : QString( "scripted failure" ); } ) );
      }

      void answer( const QString& call, const QVector<QVariantList>& result )
      {
         rows[call] = result;
      }
      void fail( const QString& call )
      {
         statuses[call] = IUS_DB2::DBERROR;
      }

      QStringList calls;

   private:
      QMap<QString, QVector<QVariantList>> rows;
      QMap<QString, int>                   statuses;
      QVector<QVariantList>                current;
      int                                  status = IUS_DB2::NOROWS;
      int                                  row    = -1;
};

const QString kRotorGuid       = "5f9c1a3e-3333-4c4e-9a1b-000000000003";
const QString kCalibrationGuid = "5f9c1a3e-4444-4c4e-9a1b-000000000004";

// A lab with one rotor (ID 3) and its calibration profile (ID 5).
void          scriptRotor( ScriptedDb& db )
{
   db.answer( "get_lab_names", { { "1", "Lab one" } } );
   db.answer( "get_lab_info 1", { { "5f9c1a3e-5555-4c4e-9a1b-000000000005", "Lab one", "B", "R" } } );
   db.answer( "get_rotor_names 1", { { "3", "Rotor three" } } );
   db.answer( "get_rotor_info 3", { { kRotorGuid, "Rotor three", "SN3", "", "1", "", "1" } } );
   db.answer( "get_rotor_calibration_profiles 3", { { "5" } } );
   db.answer( "get_rotor_calibration_info 5",
              { { kCalibrationGuid, "3", kRotorGuid, "report", "1.5e-07", "2.5e-12", "0", "2024-01-02 03:04:05", "0", "cal" } } );
}
} // namespace

class US_SimInputLoaderTest : public QtTestBase
{
};

TEST_F( US_SimInputLoaderTest, ADatabaseModelIsFoundByIdOrGuid )
{
   const QString file = US_Settings::tmpDir() + "/loader-db-model.xml";
   ASSERT_EQ( syntheticModel( "database model" ).write( file ), IUS_DB2::OK );
   QFile xml( file );
   ASSERT_TRUE( xml.open( QIODevice::ReadOnly ) );
   const QString contents = QString::fromUtf8( xml.readAll() );

   ScriptedDb    db;
   db.answer( "get_model_desc 7", { { "11", "5f9c1a3e-9999-4c4e-9a1b-000000000009" }, { "12", kModelGuid } } );
   db.answer( "get_model_info 12", { { kModelGuid, "database model", contents, "", "", "", "2024-01-02 03:04:05" } } );

   for ( const QString& spec : { QString( "12" ), kModelGuid } )
   {
      US_Model loaded;
      QString  error;
      ASSERT_TRUE( US_SimInputLoader::model( spec, &db, 7, loaded, error ) ) << spec.toStdString() << ": " << error.toStdString();
      EXPECT_EQ( loaded.description, QString( "database model" ) );
   }
}

TEST_F( US_SimInputLoaderTest, ADatabaseBufferIsFoundByIdOrGuid )
{
   ScriptedDb db;
   db.answer( "get_buffer_desc 7", { { "21", "database buffer" } } );
   db.answer( "get_bufferID " + kBufferGuid, { { "21" } } );
   db.answer( "get_buffer_info 21", { { kBufferGuid, "database buffer", "0", "7.0", "1.002", "0.998", "0", "7" } } );

   for ( const QString& spec : { QString( "21" ), kBufferGuid } )
   {
      US_Buffer loaded;
      QString   error;
      ASSERT_TRUE( US_SimInputLoader::buffer( spec, &db, 7, loaded, error ) ) << spec.toStdString() << ": " << error.toStdString();
      EXPECT_EQ( loaded.description, QString( "database buffer" ) );
      EXPECT_DOUBLE_EQ( loaded.density, 0.998 );
   }
}

TEST_F( US_SimInputLoaderTest, ADatabaseBufferThatReadsBackAsAnotherIsRefused )
{
   ScriptedDb db;
   db.answer( "get_buffer_desc 7", { { "21", "listed buffer" } } );
   db.answer( "get_buffer_info 21", { { kBufferGuid, "some other buffer", "0", "7.0", "1.002", "0.998", "0", "7" } } );

   US_Buffer loaded;
   QString   error;
   EXPECT_FALSE( US_SimInputLoader::buffer( "21", &db, 7, loaded, error ) );
   EXPECT_TRUE( error.contains( "listed buffer" ) ) << error.toStdString();
}

TEST_F( US_SimInputLoaderTest, ADatabaseBufferNeedsAnInvestigator )
{
   ScriptedDb db;
   US_Buffer  loaded;
   QString    error;
   EXPECT_FALSE( US_SimInputLoader::buffer( "21", &db, -1, loaded, error ) );
   EXPECT_TRUE( error.contains( "investigator" ) ) << error.toStdString();
   EXPECT_TRUE( db.calls.isEmpty() ) << db.calls.join( ", " ).toStdString();
}

TEST_F( US_SimInputLoaderTest, ADatabaseRotorComesWithItsFirstCalibration )
{
   ScriptedDb db;
   scriptRotor( db );

   for ( const QString& spec : { QString( "3" ), QString( "Rotor three" ) } )
   {
      US_SimInputLoader::RotorChoice choice;
      QString                        error;
      ASSERT_TRUE( US_SimInputLoader::rotor( spec, &db, choice, error ) ) << spec.toStdString() << ": " << error.toStdString();
      EXPECT_EQ( choice.rotor.ID, 3 );
      EXPECT_EQ( choice.calibration.ID, 5 );
      EXPECT_FLOAT_EQ( choice.coeff1, 1.5e-07 ); // stored as float
      EXPECT_FLOAT_EQ( choice.coeff2, 2.5e-12 );
   }
}

TEST_F( US_SimInputLoaderTest, ARotorThatCannotBeReadIsReported )
{
   ScriptedDb db;
   scriptRotor( db );
   db.fail( "get_rotor_info 3" );

   US_SimInputLoader::RotorChoice choice;
   QString                        error;
   EXPECT_FALSE( US_SimInputLoader::rotor( "3", &db, choice, error ) );
   EXPECT_TRUE( error.contains( "rotor 3" ) ) << error.toStdString();
}

TEST_F( US_SimInputLoaderTest, ACalibrationThatCannotBeReadIsReportedNotSimulatedWithZeroStretch )
{
   ScriptedDb db;
   scriptRotor( db );
   db.fail( "get_rotor_calibration_info 5" );

   US_SimInputLoader::RotorChoice choice;
   QString                        error;
   EXPECT_FALSE( US_SimInputLoader::rotor( "3", &db, choice, error ) );
   EXPECT_TRUE( error.contains( "calibration 5" ) ) << error.toStdString();

   ScriptedDb listing;
   scriptRotor( listing );
   listing.fail( "get_rotor_calibration_profiles 3" );
   EXPECT_FALSE( US_SimInputLoader::rotor( "3", &listing, choice, error ) );
   EXPECT_TRUE( error.contains( "calibration profiles" ) ) << error.toStdString();
}

TEST_F( US_SimInputLoaderTest, LocalRotorsAreFoundInEveryLabAndLabsWithoutRotorsAreSkipped )
{
   // Lab 1 has no rotor files; lab 2 has rotor 3. The dialog listed the
   // current lab's rotors under every lab, so it could not find rotor 3 from
   // lab 1.
   const QString rotors = US_Settings::dataDir() + "/rotors";
   ASSERT_TRUE( QDir().mkpath( rotors ) );
   QFile labs( rotors + "/labs.xml" );
   ASSERT_TRUE( labs.open( QIODevice::WriteOnly | QIODevice::Text ) );
   labs.write( "<?xml version=\"1.0\"?>\n<!DOCTYPE US_Lab>\n<LabData version=\"1.0\">\n"
               "<lab id=\"1\" guid=\"5f9c1a3e-6666-4c4e-9a1b-000000000006\" name=\"Lab one\" building=\"\" room=\"\"/>\n"
               "<lab id=\"2\" guid=\"5f9c1a3e-7777-4c4e-9a1b-000000000007\" name=\"Lab two\" building=\"\" room=\"\"/>\n"
               "</LabData>\n" );
   labs.close();

   US_Rotor::Rotor rotor;
   rotor.ID              = 3;
   rotor.GUID            = kRotorGuid;
   rotor.labID           = 2;
   rotor.name            = "Rotor three";
   rotor.abstractRotorID = 1;
   rotor.saveDisk();

   US_Rotor::RotorCalibration calibration;
   calibration.ID        = 5;
   calibration.GUID      = kCalibrationGuid;
   calibration.rotorID   = 3;
   calibration.rotorGUID = kRotorGuid;
   calibration.coeff1    = 1.5e-07;
   calibration.coeff2    = 2.5e-12;
   calibration.saveDisk();

   US_SimInputLoader::RotorChoice choice;
   QString                        error;
   ASSERT_TRUE( US_SimInputLoader::rotor( "Rotor three", nullptr, choice, error ) ) << error.toStdString();
   EXPECT_EQ( choice.rotor.ID, 3 );
   EXPECT_EQ( choice.rotor.labID, 2 );
   EXPECT_FLOAT_EQ( choice.coeff1, 1.5e-07 ); // stored as float
   EXPECT_FLOAT_EQ( choice.coeff2, 2.5e-12 );
}

TEST_F( US_SimInputLoaderTest, ADirectoryIsReportedAsNotAReadableFile )
{
   const QString dir = US_Settings::tmpDir() + "/loader-not-a-file";
   ASSERT_TRUE( QDir().mkpath( dir ) );
   StrictMock<US_DB2_Mock> db; // never looked up as an ID or GUID

   US_Model                model;
   US_Buffer               buffer;
   QString                 error;
   EXPECT_FALSE( US_SimInputLoader::model( dir, &db, 7, model, error ) );
   EXPECT_TRUE( error.contains( "not a readable file" ) ) << error.toStdString();
   EXPECT_FALSE( US_SimInputLoader::buffer( dir, &db, 7, buffer, error ) );
   EXPECT_TRUE( error.contains( "not a readable file" ) ) << error.toStdString();
}

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
