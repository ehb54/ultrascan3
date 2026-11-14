#include "qt_test_base.h"
#include "us_settings.h"
#include <QDir>
#include <QFileInfo>
#include <QSettings>
#include <QStandardPaths>

// The utility runner restores all persisted and process-only settings per case.
class US_SettingsTest : public QtTestBase
{
};

// Basic Set/Get Tests - Most Important
TEST_F(US_SettingsTest, BrowserSetAndGet) {
QString testBrowser = "/usr/bin/test-browser";
US_Settings::set_browser(testBrowser);
EXPECT_EQ(US_Settings::browser(), testBrowser);
}

TEST_F(US_SettingsTest, WorkBaseDirSetAndGet) {
QString testDir = "/test/work/directory";
US_Settings::set_workBaseDir(testDir);
EXPECT_EQ(US_Settings::workBaseDir(), testDir);
}

TEST_F(US_SettingsTest, TempToleranceSetAndGet) {
double testTolerance = 1.25;
US_Settings::set_tempTolerance(testTolerance);
EXPECT_DOUBLE_EQ(US_Settings::tempTolerance(), testTolerance);
}

TEST_F(US_SettingsTest, ThreadsSetAndGet) {
US_Settings::set_threads(8);
EXPECT_EQ(US_Settings::threads(), 8);
}

TEST_F(US_SettingsTest, BeckmanBugSetAndGet) {
US_Settings::set_beckmanBug(true);
EXPECT_TRUE(US_Settings::beckmanBug());

US_Settings::set_beckmanBug(false);
EXPECT_FALSE(US_Settings::beckmanBug());
}

TEST_F(US_SettingsTest, LicenseSetAndGet) {
QStringList testLicense = {"Line 1", "Line 2", "Line 3"};
US_Settings::set_license(testLicense);
EXPECT_EQ(US_Settings::license(), testLicense);
}

TEST_F(US_SettingsTest, MasterPasswordSetAndGet) {
QByteArray testHash = "test_hash_12345";
US_Settings::set_UltraScanPW(testHash);
EXPECT_EQ(US_Settings::UltraScanPW(), testHash);
}

TEST_F(US_SettingsTest, DebugSetAndGet) {
US_Settings::set_us_debug(3);
EXPECT_EQ(US_Settings::us_debug(), 3);
}

TEST_F(US_SettingsTest, InvestigatorInfoSetAndGet) {
QString testName = "Test Investigator";
int testID = 999;
int testLevel = 5;

US_Settings::set_us_inv_name(testName);
US_Settings::set_us_inv_ID(testID);
US_Settings::set_us_inv_level(testLevel);

EXPECT_EQ(US_Settings::us_inv_name(), testName);
EXPECT_EQ(US_Settings::us_inv_ID(), testID);
EXPECT_EQ(US_Settings::us_inv_level(), testLevel);
}

// Test Directory Path Dependencies
TEST_F( US_SettingsTest, DirectoryPathsFollowWorkBaseChangesWhenOverridesAreAbsent )
{
   const QString    caseRoot = US_Settings::workBaseDir();
   US_SettingsStore settings;
   settings.remove( "importDir" );
   settings.remove( "tmpDir" );
   settings.sync();
   ASSERT_EQ( settings.status(), QSettings::NoError );

   for ( const QString& base : { caseRoot + "/first", caseRoot + "/second" } )
   {
      SCOPED_TRACE( base.toStdString() );
      US_Settings::set_workBaseDir( base );
      EXPECT_EQ( US_Settings::importDir(), base + "/imports" );
      EXPECT_EQ( US_Settings::tmpDir(), base + "/tmp" );
      EXPECT_EQ( US_Settings::dataDir(), base + "/data" );
      EXPECT_EQ( US_Settings::archiveDir(), base + "/archive" );
      EXPECT_EQ( US_Settings::resultDir(), base + "/results" );
      EXPECT_EQ( US_Settings::reportDir(), base + "/reports" );
      EXPECT_EQ( US_Settings::etcDir(), base + "/etc" );
      EXPECT_FALSE( QFileInfo::exists( base ) ) << "Reading a path must not create its directory";
   }
   EXPECT_FALSE( settings.contains( "importDir" ) );
   EXPECT_FALSE( settings.contains( "tmpDir" ) );
}

// Test Database Configuration
TEST_F(US_SettingsTest, DatabaseConfigSetAndGet) {
QList<QStringList> testDatabases;
testDatabases << (QStringList() << "host1" << "db1" << "user1" << "desc1");
testDatabases << (QStringList() << "host2" << "db2" << "user2" << "desc2");

US_Settings::set_databases(testDatabases);

QList<QStringList> retrieved = US_Settings::databases();
EXPECT_EQ(retrieved.size(), 2);
EXPECT_EQ(retrieved[0], testDatabases[0]);
EXPECT_EQ(retrieved[1], testDatabases[1]);
}

TEST_F(US_SettingsTest, DefaultDBSetAndGet) {
QStringList testDB = {"testhost", "testdb", "testuser", "Test DB"};
US_Settings::set_defaultDB(testDB);
EXPECT_EQ(US_Settings::defaultDB(), testDB);
}

// Test DA Status
TEST_F(US_SettingsTest, DAStatusOperations) {
US_Settings::set_DA_status("COM:1");
EXPECT_TRUE(US_Settings::get_DA_status("COM"));

US_Settings::set_DA_status("COM:0");
EXPECT_FALSE(US_Settings::get_DA_status("COM"));
}

// Test Settings Status
TEST_F(US_SettingsTest, SettingsStatusCheck) {
QString status = US_Settings::status();
// Should be empty if no errors
EXPECT_TRUE(status.isEmpty() || !status.contains("error", Qt::CaseInsensitive));
}

// Test set-speed resolution
TEST_F(US_SettingsTest, SpeedResolutionDefault) {
US_Settings::set_debug_text(QStringList());
EXPECT_DOUBLE_EQ(US_Settings::speedResolution(), 100.0);
}

TEST_F(US_SettingsTest, SpeedResolutionFromDebugText) {
US_Settings::set_debug_text(QStringList() << "SetSpeedReso=50");
EXPECT_DOUBLE_EQ(US_Settings::speedResolution(), 50.0);

// The manual spells it SetSpeedResolution; matching is on the prefix
US_Settings::set_debug_text(QStringList() << "SetSpeedResolution=25");
EXPECT_DOUBLE_EQ(US_Settings::speedResolution(), 25.0);
}

TEST_F(US_SettingsTest, SpeedResolutionRejectsBadValues) {
const QStringList unusable = QStringList()
    << "SetSpeedReso="
    << "SetSpeedReso=abc"
    << "SetSpeedReso=0"
    << "SetSpeedReso=-100"
    << "SetSpeedReso=inf"
    << "SetSpeedReso=nan";

for (const QString& setting : unusable) {
    US_Settings::set_debug_text(QStringList() << setting);
    EXPECT_DOUBLE_EQ(US_Settings::speedResolution(), 100.0) << setting.toStdString();
}
}

// These cases seed the sandbox store directly where appropriate, then invoke
// production accessors. They cover missing values and state transitions rather
// than duplicating setter/getter round trips. The runner restores the full store.
TEST_F( US_SettingsTest, MissingScalarKeysUseDefaultsWithoutCreatingEntries )
{
   US_SettingsStore  settings;
   const QStringList keys{ "browser", "tempTolerance", "threads", "beckmanBug", "dataLocation" };
   for ( const QString& key : keys )
   {
      settings.remove( key );
   }
   settings.sync();
   ASSERT_EQ( settings.status(), QSettings::NoError );
   US_Settings::clear_default_data_location_override();

   EXPECT_TRUE( US_Settings::browser().isEmpty() );
   EXPECT_DOUBLE_EQ( US_Settings::tempTolerance(), 0.5 );
   EXPECT_EQ( US_Settings::threads(), 1 );
   EXPECT_FALSE( US_Settings::beckmanBug() );
   EXPECT_EQ( US_Settings::default_data_location(), 2 );
   for ( const QString& key : keys )
   {
      EXPECT_FALSE( settings.contains( key ) ) << key.toStdString();
   }
}

TEST_F( US_SettingsTest, MissingCredentialAndListKeysReturnEmptyValues )
{
   US_SettingsStore settings;
   for ( const QString& key : { QString( "license" ), QString( "UltraScanPW" ), QString( "defaultDB" ) } )
   {
      settings.remove( key );
   }
   settings.sync();
   ASSERT_EQ( settings.status(), QSettings::NoError );

   EXPECT_TRUE( US_Settings::license().isEmpty() );
   EXPECT_TRUE( US_Settings::UltraScanPW().isEmpty() );
   EXPECT_TRUE( US_Settings::defaultDB().isEmpty() );
   EXPECT_FALSE( settings.contains( "license" ) );
   EXPECT_FALSE( settings.contains( "UltraScanPW" ) );
   EXPECT_FALSE( settings.contains( "defaultDB" ) );
}

TEST_F( US_SettingsTest, SettingScalarDefaultsRemovesPreviouslyStoredOverrides )
{
   US_SettingsStore settings;
   settings.setValue( "tempTolerance", 2.25 );
   settings.setValue( "threads", 8 );
   settings.setValue( "beckmanBug", true );
   settings.setValue( "dataLocation", 1 );
   settings.sync();
   ASSERT_EQ( settings.status(), QSettings::NoError );
   US_Settings::clear_default_data_location_override();

   US_Settings::set_tempTolerance( 0.5 );
   US_Settings::set_threads( 1 );
   US_Settings::set_beckmanBug( false );
   US_Settings::set_default_data_location( 2 );

   EXPECT_FALSE( settings.contains( "tempTolerance" ) );
   EXPECT_FALSE( settings.contains( "threads" ) );
   EXPECT_FALSE( settings.contains( "beckmanBug" ) );
   EXPECT_FALSE( settings.contains( "dataLocation" ) );
   EXPECT_DOUBLE_EQ( US_Settings::tempTolerance(), 0.5 );
   EXPECT_EQ( US_Settings::threads(), 1 );
   EXPECT_FALSE( US_Settings::beckmanBug() );
   EXPECT_EQ( US_Settings::default_data_location(), 2 );
}

TEST_F( US_SettingsTest, StoredTextValuesAreDecodedByTypedGetters )
{
   // INI values can arrive as text. No save/reopen durability claim is made here.
   US_SettingsStore settings;
   settings.setValue( "threads", QString( "6" ) );
   settings.setValue( "tempTolerance", QString( "0.75" ) );
   settings.setValue( "beckmanBug", QString( "false" ) );
   settings.sync();
   ASSERT_EQ( settings.status(), QSettings::NoError );

   EXPECT_EQ( US_Settings::threads(), 6 );
   EXPECT_DOUBLE_EQ( US_Settings::tempTolerance(), 0.75 );
   EXPECT_FALSE( US_Settings::beckmanBug() );

   // A stored zero is a value, not an absent-key fallback to 0.5.
   settings.setValue( "tempTolerance", QString( "0" ) );
   EXPECT_DOUBLE_EQ( US_Settings::tempTolerance(), 0.0 );
}

TEST_F( US_SettingsTest, BinaryHashAndLicenseListRetainTheirContents )
{
   const QByteArray  hash = QByteArray::fromHex( "0041ff00fe" );
   const QStringList license{ "synthetic license", QString(), QString::fromUtf8( "synthetic \xC2\xB5" ) };
   US_SettingsStore  settings;
   settings.setValue( "UltraScanPW", hash );
   settings.setValue( "license", license );
   settings.sync();
   ASSERT_EQ( settings.status(), QSettings::NoError );

   EXPECT_EQ( US_Settings::UltraScanPW(), hash );
   EXPECT_EQ( US_Settings::license(), license );
}

TEST_F( US_SettingsTest, ExplicitImportAndTempOverridesStayPinnedUntilReset )
{
   const QString caseRoot  = US_Settings::workBaseDir();
   const QString imports   = caseRoot + "/chosen-imports";
   const QString temporary = caseRoot + "/chosen-tmp";
   US_Settings::set_importDir( imports );
   US_Settings::set_tmpDir( temporary );
   US_Settings::set_workBaseDir( caseRoot + "/moved" );

   EXPECT_EQ( US_Settings::importDir(), imports );
   EXPECT_EQ( US_Settings::tmpDir(), temporary );
   EXPECT_EQ( US_Settings::dataDir(), caseRoot + "/moved/data" );

   // Selecting the current defaults removes the explicit overrides so later
   // work-root changes can move these paths again.
   US_Settings::set_importDir( caseRoot + "/moved/imports" );
   US_Settings::set_tmpDir( caseRoot + "/moved/tmp" );
   US_SettingsStore settings;
   EXPECT_FALSE( settings.contains( "importDir" ) );
   EXPECT_FALSE( settings.contains( "tmpDir" ) );
   US_Settings::set_workBaseDir( caseRoot + "/moved-again" );
   EXPECT_EQ( US_Settings::importDir(), caseRoot + "/moved-again/imports" );
   EXPECT_EQ( US_Settings::tmpDir(), caseRoot + "/moved-again/tmp" );
}

TEST_F( US_SettingsTest, HomeWorkBaseDefaultRemainsImplicit )
{
   US_SettingsStore settings;
   settings.remove( "workBaseDir" );
   settings.sync();
   ASSERT_EQ( settings.status(), QSettings::NoError );
   // Compute the default path only; never read or create home data.
   const QString defaultBase = QDir::homePath() + "/ultrascan";
   EXPECT_EQ( US_Settings::workBaseDir(), defaultBase );
   EXPECT_FALSE( settings.contains( "workBaseDir" ) );

   US_Settings::set_workBaseDir( defaultBase );
   EXPECT_FALSE( settings.contains( "workBaseDir" ) );
   EXPECT_EQ( US_Settings::workBaseDir(), defaultBase );
}

TEST_F( US_SettingsTest, ReplacingDatabaseListRemovesStaleEntries )
{
   const QStringList first{ "first-host", "first-db", "first-user", "first description" };
   const QStringList second{ "second-host", "second-db", "second-user", "second description" };
   US_Settings::set_databases( { first, second } );
   US_Settings::set_databases( { second } );

   EXPECT_EQ( US_Settings::databases(), QList<QStringList>{ second } );
   US_SettingsStore settings;
   EXPECT_EQ( settings.value( "dbCount" ).toInt(), 1 );
   EXPECT_FALSE( settings.contains( "dblist1" ) );

   US_Settings::set_databases( {} );
   EXPECT_TRUE( US_Settings::databases().isEmpty() );
   EXPECT_EQ( settings.value( "dbCount" ).toInt(), 0 );
   EXPECT_FALSE( settings.contains( "dblist0" ) );
   EXPECT_FALSE( settings.contains( "dblist1" ) );
}

TEST_F( US_SettingsTest, InvalidNumericStorageUsesDefaultsWithoutRewritingValues )
{
   US_SettingsStore settings;
   for ( const QString& input :
         { QString(), QString( "garbage" ), QString( "-1" ), QString( "99999999999999999999999999999999999999999999999999" ) } )
   {
      SCOPED_TRACE( input.toStdString() );
      settings.setValue( "threads", input );
      EXPECT_EQ( US_Settings::threads(), 1 );
      EXPECT_EQ( settings.value( "threads" ).toString(), input );
   }
   settings.setValue( "threads", 0 );
   EXPECT_EQ( US_Settings::threads(), 1 );
   for ( const QString& input :
         { QString(), QString( "garbage" ), QString( "-0.1" ), QString( "nan" ), QString( "inf" ), QString( "1e999" ) } )
   {
      SCOPED_TRACE( input.toStdString() );
      settings.setValue( "tempTolerance", input );
      EXPECT_DOUBLE_EQ( US_Settings::tempTolerance(), 0.5 );
      EXPECT_EQ( settings.value( "tempTolerance" ).toString(), input );
   }
   US_Settings::set_threads( 2 );
   US_Settings::set_tempTolerance( 0.0 );
   EXPECT_EQ( US_Settings::threads(), 2 );
   EXPECT_DOUBLE_EQ( US_Settings::tempTolerance(), 0.0 );
}

TEST_F( US_SettingsTest, InvalidDaUpdatesPreserveAbsentAndExistingKeys )
{
   US_SettingsStore  settings;
   const QStringList invalid{ "",        "COM",     "ACAD",     "COM:",    "ACAD:",  ":1",
                              "OTHER:1", "COM:1:0", "ACAD:0:1", "COM:abc", "ACAD:2", "COM:-1" };
   for ( bool present : { false, true } )
   {
      settings.remove( "daComOpened" );
      settings.remove( "daAcadOpened" );
      if ( present )
      {
         US_Settings::set_DA_status( "COM:1" );
         US_Settings::set_DA_status( "ACAD:0" );
      }
      for ( const QString& input : invalid )
      {
         SCOPED_TRACE( input.toStdString() );
         US_Settings::set_DA_status( input );
         EXPECT_EQ( settings.contains( "daComOpened" ), present );
         EXPECT_EQ( settings.contains( "daAcadOpened" ), present );
         EXPECT_EQ( US_Settings::get_DA_status( "COM" ), present );
         EXPECT_FALSE( US_Settings::get_DA_status( "ACAD" ) );
      }
   }
}

TEST_F( US_SettingsTest, DaUpdatesAreIndependentAndReversible )
{
   US_Settings::set_DA_status( "COM:1" );
   US_Settings::set_DA_status( "ACAD:1" );
   US_Settings::set_DA_status( "COM:0" );
   EXPECT_FALSE( US_Settings::get_DA_status( "COM" ) );
   EXPECT_TRUE( US_Settings::get_DA_status( "ACAD" ) );
   US_Settings::set_DA_status( "COM:1" );
   US_Settings::set_DA_status( "ACAD:0" );
   EXPECT_TRUE( US_Settings::get_DA_status( "COM" ) );
   EXPECT_FALSE( US_Settings::get_DA_status( "ACAD" ) );
   EXPECT_FALSE( US_Settings::get_DA_status( "UNKNOWN" ) );
}

TEST_F( US_SettingsTest, EmptyCredentialsAndDefaultDatabaseClearPreviousValues )
{
   US_Settings::set_defaultDB( { "synthetic-host", "synthetic-db" } );
   US_Settings::set_license( { "synthetic-license" } );
   US_Settings::set_UltraScanPW( QByteArray( "synthetic-hash" ) );
   US_Settings::set_defaultDB( {} );
   US_Settings::set_license( {} );
   US_Settings::set_UltraScanPW( {} );
   EXPECT_TRUE( US_Settings::defaultDB().isEmpty() );
   EXPECT_TRUE( US_Settings::license().isEmpty() );
   EXPECT_TRUE( US_Settings::UltraScanPW().isEmpty() );
   US_SettingsStore settings;
   EXPECT_FALSE( settings.contains( "defaultDB" ) );
}
