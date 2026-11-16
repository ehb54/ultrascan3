//! \file test_us_analyte_db.cpp
#include "mock_us_db2.h"
#include "qt_test_base.h"
#include "us_analyte.h"
#include "us_eprofile.h"
#include "us_settings.h"
#include <QDir>
#include <QFile>

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::AtLeast;
using ::testing::Eq;
using ::testing::InSequence;
using ::testing::Invoke;
using ::testing::Matcher;
using ::testing::Return;
using ::testing::StrictMock;

// Load lookups and their failures are covered in test_us_experiment_db_failures.cpp; this suite
// covers the loaded column mapping and the write path.

namespace
{
const QString               kGuid = "12345678-1234-1234-1234-123456789012";

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

class AnalyteDb : public QtTestBase
{
   protected:
      StrictMock<US_DB2_Mock> db;
      US_Analyte              analyte;

      void                    SetUp() override
      {
         ASSERT_NO_FATAL_FAILURE( QtTestBase::SetUp() );
         US_Settings::set_us_inv_ID( 23 );
         // DEBUG_QUERY reads the error text after every query.
         EXPECT_CALL( db, lastError() ).Times( AnyNumber() ).WillRepeatedly( Return( QString() ) );
      }

      // Expectations from the analyte-ID lookup through the analyte-info row (columns 1-6).
      void expectInfoRow( const QString& type, const QString& sequence )
      {
         EXPECT_CALL( db, query( args( { "get_analyteID", kGuid } ) ) );
         EXPECT_CALL( db, lastErrno() ).Times( AtLeast( 1 ) ).WillRepeatedly( Return( IUS_DB2::OK ) );
         EXPECT_CALL( db, next() ).WillOnce( Return( true ) );
         EXPECT_CALL( db, value( 0 ) ).WillOnce( Return( QVariant( "17" ) ) );
         EXPECT_CALL( db, query( args( { "get_analyte_info", "17" } ) ) );
         EXPECT_CALL( db, lastErrno() ).Times( AtLeast( 1 ) ).WillRepeatedly( Return( IUS_DB2::OK ) );
         EXPECT_CALL( db, next() ).WillOnce( Return( true ) );
         EXPECT_CALL( db, value( 1 ) ).WillOnce( Return( QVariant( type ) ) );
         EXPECT_CALL( db, value( 2 ) ).WillOnce( Return( QVariant( sequence ) ) );
         EXPECT_CALL( db, value( 3 ) ).WillOnce( Return( QVariant( "0.7125" ) ) );
         EXPECT_CALL( db, value( 4 ) ).WillOnce( Return( QVariant( "Salt & <buffer>" ) ) );
         EXPECT_CALL( db, value( 6 ) ).WillOnce( Return( QVariant( "45000.5" ) ) );
      }

      void expectNoProfile()
      {
         EXPECT_CALL( db, query( args( { "get_eprofile", "17", "Analyte" } ) ) );
         EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::NOROWS ) );
      }

      void expectLookup( bool found )
      {
         EXPECT_CALL( db, query( args( { "get_analyteID", kGuid } ) ) );
         EXPECT_CALL( db, lastErrno() ).Times( AtLeast( 1 ) ).WillRepeatedly( Return( found ? IUS_DB2::OK : IUS_DB2::NOROWS ) );
         if ( found )
         {
            EXPECT_CALL( db, next() ).WillOnce( Return( true ) );
            EXPECT_CALL( db, value( 0 ) ).WillOnce( Return( QVariant( "17" ) ) );
         }
      }

      void prepareProtein()
      {
         analyte.analyteGUID = kGuid;
         analyte.type        = US_Analyte::PROTEIN;
         analyte.sequence    = "MKV";
         analyte.vbar20      = 0.7125;
         analyte.description = "Salt & <buffer>";
         analyte.mw          = 45000.5;
         analyte.grad_form   = false;
      }

      // File name -> contents for every file in the analytes directory.
      QMap<QString, QByteArray> analyteFiles()
      {
         QString path;
         EXPECT_TRUE( US_Analyte::analyte_path( path ) );
         QMap<QString, QByteArray> files;
         for ( const QString& name : QDir( path ).entryList( QDir::Files ) )
         {
            QFile file( path + "/" + name );
            EXPECT_TRUE( file.open( QIODevice::ReadOnly ) ) << name.toStdString();
            files[name] = file.readAll();
         }
         return files;
      }

      QString diskDescription()
      {
         US_Analyte disk;
         if ( disk.load( false, kGuid ) != IUS_DB2::OK )
         {
            return QString();
         }
         return disk.description;
      }
};

// Load mapping

TEST_F( AnalyteDb, LoadMapsInfoColumnsAndProfile )
{
   InSequence sequence;
   expectInfoRow( "Other", "Glc-Gal" );
   EXPECT_CALL( db, value( 7 ) ).WillOnce( Return( QVariant( "1" ) ) );
   EXPECT_CALL( db, query( args( { "get_eprofile", "17", "Analyte" } ) ) );
   EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::OK ) );
   EXPECT_CALL( db, next() ).WillOnce( Return( true ) );
   EXPECT_CALL( db, value( 0 ) ).WillOnce( Return( QVariant( "5" ) ) );
   EXPECT_CALL( db, value( 1 ) ).WillOnce( Return( QVariant( "molarExtinction" ) ) );
   EXPECT_CALL( db, value( 2 ) )
      .WillOnce( Return( QVariant( "<EProfileData><extinctionProfile componentID='17' componentType='Analyte' "
                                   "valueType='molarExtinction'><extinction wavelength='260' value='1.5'/>"
                                   "<extinction wavelength='280' value='2.25'/></extinctionProfile></EProfileData>" ) ) );

   ASSERT_EQ( IUS_DB2::OK, analyte.load( true, kGuid, &db ) );
   EXPECT_EQ( QString( "17" ), analyte.analyteID );
   EXPECT_EQ( kGuid, analyte.analyteGUID );
   EXPECT_EQ( US_Analyte::CARBOHYDRATE, analyte.type );
   EXPECT_EQ( QString( "Glc-Gal" ), analyte.sequence );
   EXPECT_DOUBLE_EQ( 0.7125, analyte.vbar20 );
   EXPECT_EQ( QString( "Salt & <buffer>" ), analyte.description );
   EXPECT_DOUBLE_EQ( 45000.5, analyte.mw );
   EXPECT_TRUE( analyte.grad_form );
   EXPECT_EQ( ( QMap<double, double>{ { 260, 1.5 }, { 280, 2.25 } } ), analyte.extinction );
}

TEST_F( AnalyteDb, LoadMapsTypeNamesAndIgnoresGradientFormOutsideCarbohydrates )
{
   const QList<QPair<QString, US_Analyte::analyte_t>> types{ { "Protein", US_Analyte::PROTEIN },
                                                             { "Lipid", US_Analyte::PROTEIN },
                                                             { "", US_Analyte::PROTEIN } };
   for ( const auto& type : types )
   {
      SCOPED_TRACE( type.first.toStdString() );
      US_Analyte loaded;
      loaded.type      = US_Analyte::RNA;
      loaded.grad_form = true;
      {
         InSequence sequence;
         expectInfoRow( type.first, "MKV" );
         expectNoProfile();
      }
      ASSERT_EQ( IUS_DB2::OK, loaded.load( true, kGuid, &db ) );
      EXPECT_EQ( type.second, loaded.type );
      EXPECT_FALSE( loaded.grad_form );
      ASSERT_TRUE( ::testing::Mock::VerifyAndClearExpectations( &db ) );
      EXPECT_CALL( db, lastError() ).Times( AnyNumber() ).WillRepeatedly( Return( QString() ) );
   }
}

TEST_F( AnalyteDb, LoadMapsNucleotideFlagsAndCounterions )
{
   InSequence sequence;
   expectInfoRow( "DNA", "ACGT" );
   EXPECT_CALL( db, query( args( { "get_nucleotide_info", "17" } ) ) );
   EXPECT_CALL( db, lastErrno() ).Times( AtLeast( 1 ) ).WillRepeatedly( Return( IUS_DB2::OK ) );
   EXPECT_CALL( db, next() ).WillOnce( Return( true ) );
   const QStringList columns{ "0", "1", "T", "0", "0.1", "0.2", "0.3", "0.4", "0.5" };
   for ( unsigned col = 0; col < 9; ++col )
   {
      EXPECT_CALL( db, value( col ) ).WillOnce( Return( QVariant( columns[col] ) ) );
   }
   expectNoProfile();

   ASSERT_EQ( IUS_DB2::OK, analyte.load( true, kGuid, &db ) );
   EXPECT_EQ( US_Analyte::DNA, analyte.type );
   EXPECT_FALSE( analyte.doubleStranded );
   EXPECT_TRUE( analyte.complement );
   EXPECT_TRUE( analyte._3prime );
   EXPECT_FALSE( analyte._5prime );
   EXPECT_DOUBLE_EQ( 0.1, analyte.sodium );
   EXPECT_DOUBLE_EQ( 0.2, analyte.potassium );
   EXPECT_DOUBLE_EQ( 0.3, analyte.lithium );
   EXPECT_DOUBLE_EQ( 0.4, analyte.magnesium );
   EXPECT_DOUBLE_EQ( 0.5, analyte.calcium );
   EXPECT_EQ( QString( "acgt" ), analyte.sequence );
}

TEST_F( AnalyteDb, LoadWithoutAStoredProfileHasNoExtinction )
{
   analyte.extinction = { { 300.0, 9.0 } }; // left over from another analyte
   {
      InSequence sequence;
      expectInfoRow( "Protein", "MKV" );
      expectNoProfile();
   }
   ASSERT_EQ( IUS_DB2::OK, analyte.load( true, kGuid, &db ) );
   EXPECT_TRUE( analyte.extinction.isEmpty() );
}

// Write

TEST_F( AnalyteDb, WriteInsertsWhenTheLookupFindsNoRow )
{
   prepareProtein();
   {
      InSequence sequence;
      EXPECT_CALL( db, query( args( { "get_analyteID", kGuid } ) ) );
      // DEBUG_QUERY also reads the status.
      EXPECT_CALL( db, lastErrno() ).Times( AtLeast( 1 ) ).WillRepeatedly( Return( IUS_DB2::OK ) );
      EXPECT_CALL( db, next() ).WillOnce( Return( false ) );
      EXPECT_CALL(
         db,
         statusQuery( args( { "new_analyte", kGuid, "Protein", "MKV", "0.7125", "Salt & <buffer>", "", "45000.5", "0", "23" } ) ) )
         .WillOnce( Return( IUS_DB2::OK ) );
      EXPECT_CALL( db, lastErrno() ).Times( AtLeast( 1 ) ).WillRepeatedly( Return( IUS_DB2::OK ) );
      EXPECT_CALL( db, lastInsertID() ).WillOnce( Return( 456 ) );
   }
   EXPECT_CALL( db, value( _ ) ).Times( 0 );

   EXPECT_EQ( IUS_DB2::OK, analyte.write( true, QString(), &db ) );
   EXPECT_EQ( QString( "456" ), analyte.analyteID );
}

TEST_F( AnalyteDb, FailedLookupWritesNothing )
{
   prepareProtein();
   for ( int status : { IUS_DB2::DBERROR, IUS_DB2::NOT_CONNECTED } )
   {
      SCOPED_TRACE( status );
      {
         InSequence sequence;
         EXPECT_CALL( db, query( args( { "get_analyteID", kGuid } ) ) );
         EXPECT_CALL( db, lastErrno() ).Times( AtLeast( 1 ) ).WillRepeatedly( Return( status ) );
      }
      EXPECT_EQ( status, analyte.write( true, QString(), &db ) );
      EXPECT_EQ( QString( "Could not look up the analyte in the DB" ), analyte.message );
      EXPECT_TRUE( diskDescription().isEmpty() );
      ASSERT_TRUE( ::testing::Mock::VerifyAndClearExpectations( &db ) );
      EXPECT_CALL( db, lastError() ).Times( AnyNumber() ).WillRepeatedly( Return( QString() ) );
   }
}

TEST_F( AnalyteDb, FailedNucleotideWriteIsReportedBeforeTheProfileAndDisk )
{
   prepareProtein();
   analyte.type                    = US_Analyte::DNA;
   analyte.extinction[280.0]       = 43824.0;
   analyte.new_or_changed_spectrum = true;
   {
      InSequence sequence;
      expectLookup( true );
      EXPECT_CALL( db, statusQuery( Matcher<const QStringList&>( _ ) ) ).WillOnce( Return( IUS_DB2::OK ) );
      EXPECT_CALL( db, lastErrno() ).Times( AtLeast( 1 ) ).WillRepeatedly( Return( IUS_DB2::OK ) );
      EXPECT_CALL( db, statusQuery( Matcher<const QStringList&>( _ ) ) ).WillOnce( Return( IUS_DB2::DBERROR ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::DBERROR ) );
   }

   EXPECT_EQ( IUS_DB2::DBERROR, analyte.write( true, QString(), &db ) );
   EXPECT_EQ( QString( "Could not update nucleotide info" ), analyte.message );
   EXPECT_TRUE( analyte.new_or_changed_spectrum );
   EXPECT_TRUE( diskDescription().isEmpty() );
}

TEST_F( AnalyteDb, FailedDiskCopyIsReported )
{
   prepareProtein();
   // A file where the analytes directory should be makes the disk copy impossible.
   QString path;
   ASSERT_TRUE( US_Analyte::analyte_path( path ) );
   ASSERT_TRUE( QDir( path ).removeRecursively() );
   {
      QFile blocker( path );
      ASSERT_TRUE( blocker.open( QIODevice::WriteOnly ) );
   }
   {
      InSequence sequence;
      expectLookup( true );
      EXPECT_CALL( db, statusQuery( Matcher<const QStringList&>( _ ) ) ).WillOnce( Return( IUS_DB2::OK ) );
      EXPECT_CALL( db, lastErrno() ).Times( AtLeast( 1 ) ).WillRepeatedly( Return( IUS_DB2::OK ) );
   }

   EXPECT_NE( IUS_DB2::OK, analyte.write( true, QString(), &db ) );
   EXPECT_FALSE( analyte.message.isEmpty() );
   EXPECT_NE( QString( "updated in" ), analyte.message );
   ASSERT_TRUE( QFile::remove( path ) );
}

TEST_F( AnalyteDb, WriteInsertsNewAnalyteWithInvestigatorAndSavesToDisk )
{
   prepareProtein();
   analyte.extinction[280.0] = 43824.0;
   analyte.extinction[260.0] = 35000.0;
   {
      InSequence sequence;
      expectLookup( false );
      EXPECT_CALL(
         db,
         statusQuery(
            args( { "new_analyte", kGuid, "Protein", "MKV", "0.7125", "Salt & <buffer>", "260/35000", "45000.5", "0", "23" } ) ) )
         .WillOnce( Return( IUS_DB2::OK ) );
      EXPECT_CALL( db, lastErrno() ).Times( AtLeast( 1 ) ).WillRepeatedly( Return( IUS_DB2::OK ) );
      EXPECT_CALL( db, lastInsertID() ).WillOnce( Return( 456 ) );
   }

   EXPECT_EQ( IUS_DB2::OK, analyte.write( true, QString(), &db ) );
   EXPECT_EQ( QString( "456" ), analyte.analyteID );
   EXPECT_EQ( QString( "Salt & <buffer>" ), diskDescription() );
}

TEST_F( AnalyteDb, WriteUpdatesExistingAnalyteWithoutInvestigatorOrInsertId )
{
   prepareProtein();
   analyte.type      = US_Analyte::CARBOHYDRATE;
   analyte.grad_form = true;
   EXPECT_CALL( db, lastInsertID() ).Times( 0 );
   {
      InSequence sequence;
      expectLookup( true );
      EXPECT_CALL(
         db, statusQuery( args( { "update_analyte", "17", "Other", "MKV", "0.7125", "Salt & <buffer>", "", "45000.5", "1" } ) ) )
         .WillOnce( Return( IUS_DB2::OK ) );
      EXPECT_CALL( db, lastErrno() ).Times( AtLeast( 1 ) ).WillRepeatedly( Return( IUS_DB2::OK ) );
   }

   EXPECT_EQ( IUS_DB2::OK, analyte.write( true, QString(), &db ) );
   EXPECT_EQ( QString( "17" ), analyte.analyteID );
   EXPECT_EQ( QString( "Salt & <buffer>" ), diskDescription() );
}

TEST_F( AnalyteDb, WriteSpectrumFieldUsesTheFirstNonEmptyMapAndOmitsAZeroValue )
{
   struct Case
   {
         const char*          name;
         QMap<double, double> extinction, refraction, fluorescence;
         QString              spectrum;
   };
   const QList<Case> cases{ { "refraction", {}, { { 589.5, 1.333 } }, { { 350, 9 } }, "589.5/1.333" },
                            { "fluorescence", {}, {}, { { 350, 9 } }, "350/9" },
                            { "zero", { { 280, 0 } }, { { 589.5, 1.333 } }, {}, "" } };
   for ( const Case& c : cases )
   {
      SCOPED_TRACE( c.name );
      prepareProtein();
      analyte.extinction   = c.extinction;
      analyte.refraction   = c.refraction;
      analyte.fluorescence = c.fluorescence;
      {
         InSequence sequence;
         expectLookup( true );
         EXPECT_CALL( db,
                      statusQuery( args(
                         { "update_analyte", "17", "Protein", "MKV", "0.7125", "Salt & <buffer>", c.spectrum, "45000.5", "0" } ) ) )
            .WillOnce( Return( IUS_DB2::OK ) );
         EXPECT_CALL( db, lastErrno() ).Times( AtLeast( 1 ) ).WillRepeatedly( Return( IUS_DB2::OK ) );
      }
      EXPECT_EQ( IUS_DB2::OK, analyte.write( true, QString(), &db ) );
      ASSERT_TRUE( ::testing::Mock::VerifyAndClearExpectations( &db ) );
      EXPECT_CALL( db, lastError() ).Times( AnyNumber() ).WillRepeatedly( Return( QString() ) );
   }
}

TEST_F( AnalyteDb, WriteSendsNucleotideInfoForNucleicAcids )
{
   const QList<QPair<US_Analyte::analyte_t, QString>> types{ { US_Analyte::DNA, "DNA" }, { US_Analyte::RNA, "RNA" } };
   for ( const auto& type : types )
   {
      SCOPED_TRACE( type.second.toStdString() );
      prepareProtein();
      analyte.type           = type.first;
      analyte.doubleStranded = false;
      analyte.complement     = true;
      analyte._3prime        = true;
      analyte._5prime        = false;
      analyte.sodium         = 0.1;
      analyte.potassium      = 0.2;
      analyte.lithium        = 0.3;
      analyte.magnesium      = 0.4;
      analyte.calcium        = 0.5;
      {
         InSequence sequence;
         expectLookup( true );
         EXPECT_CALL(
            db,
            statusQuery( args( { "update_analyte", "17", type.second, "MKV", "0.7125", "Salt & <buffer>", "", "45000.5", "0" } ) ) )
            .WillOnce( Return( IUS_DB2::OK ) );
         EXPECT_CALL( db, lastErrno() ).Times( AtLeast( 1 ) ).WillRepeatedly( Return( IUS_DB2::OK ) );
         EXPECT_CALL(
            db, statusQuery( args( { "set_nucleotide_info", "17", "0", "1", "1", "0", "0.1", "0.2", "0.3", "0.4", "0.5" } ) ) )
            .WillOnce( Return( IUS_DB2::OK ) );
         EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::OK ) );
      }
      EXPECT_EQ( IUS_DB2::OK, analyte.write( true, QString(), &db ) );
      ASSERT_TRUE( ::testing::Mock::VerifyAndClearExpectations( &db ) );
      EXPECT_CALL( db, lastError() ).Times( AnyNumber() ).WillRepeatedly( Return( QString() ) );
   }
}

TEST_F( AnalyteDb, WriteCreatesExtinctionProfileForANewSpectrum )
{
   prepareProtein();
   analyte.extinction[280.0]       = 43824.0;
   analyte.new_or_changed_spectrum = true;
   QString profileXml;
   {
      InSequence sequence;
      expectLookup( true );
      EXPECT_CALL( db, statusQuery( Matcher<const QStringList&>( _ ) ) ).WillOnce( Return( IUS_DB2::OK ) );
      EXPECT_CALL( db, lastErrno() ).Times( AtLeast( 1 ) ).WillRepeatedly( Return( IUS_DB2::OK ) );
      EXPECT_CALL( db, query( args( { "get_eprofile", "17", "Analyte" } ) ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::NOROWS ) );
      EXPECT_CALL( db, statusQuery( Matcher<const QStringList&>( _ ) ) )
         .WillOnce( Invoke(
            [&profileXml]( const QStringList& q )
            {
               EXPECT_EQ( ( QStringList{ "new_eprofile", "17", "Analyte", "molarExtinction" } ), q.mid( 0, 4 ) );
               profileXml = q.value( 4 );
               return IUS_DB2::OK;
            } ) );
      EXPECT_CALL( db, lastInsertID() ).WillOnce( Return( 9 ) );
   }

   EXPECT_EQ( IUS_DB2::OK, analyte.write( true, QString(), &db ) );
   EXPECT_EQ( ( QMap<double, double>{ { 280, 43824 } } ), profileValues( profileXml ) );
   EXPECT_FALSE( analyte.new_or_changed_spectrum );
}

TEST_F( AnalyteDb, FailedProfileWriteIsReportedAndStaysPendingWithTheDiskCopy )
{
   prepareProtein();
   analyte.extinction[280.0]       = 43824.0;
   analyte.new_or_changed_spectrum = true;
   {
      InSequence sequence;
      expectLookup( true );
      EXPECT_CALL( db, statusQuery( Matcher<const QStringList&>( _ ) ) ).WillOnce( Return( IUS_DB2::OK ) );
      EXPECT_CALL( db, lastErrno() ).Times( AtLeast( 1 ) ).WillRepeatedly( Return( IUS_DB2::OK ) );
      EXPECT_CALL( db, query( args( { "get_eprofile", "17", "Analyte" } ) ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::NOROWS ) );
      EXPECT_CALL( db, statusQuery( Matcher<const QStringList&>( _ ) ) ).WillOnce( Return( IUS_DB2::DBERROR ) );
   }

   EXPECT_EQ( IUS_DB2::DBERROR, analyte.write( true, QString(), &db ) );
   EXPECT_EQ( QString( "The extinction profile could not be saved to the DB" ), analyte.message );
   // The spectrum is retried by the next save; the disk copy is written regardless.
   EXPECT_TRUE( analyte.new_or_changed_spectrum );
   EXPECT_FALSE( diskDescription().isEmpty() );
}

TEST_F( AnalyteDb, WriteReplacesTheStoredProfileAndKeepsTheNewValues )
{
   prepareProtein();
   const QMap<double, double> replacement{ { 260.0, 30000.0 }, { 280.0, 43824.0 } };
   analyte.extinction              = replacement;
   analyte.new_or_changed_spectrum = true;
   analyte.replace_spectrum        = true;
   QString profileXml;
   {
      InSequence sequence;
      expectLookup( true );
      EXPECT_CALL( db, statusQuery( Matcher<const QStringList&>( _ ) ) ).WillOnce( Return( IUS_DB2::OK ) );
      EXPECT_CALL( db, lastErrno() ).Times( AtLeast( 1 ) ).WillRepeatedly( Return( IUS_DB2::OK ) );
      EXPECT_CALL( db, query( args( { "get_eprofile", "17", "Analyte" } ) ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::OK ) );
      EXPECT_CALL( db, next() ).WillOnce( Return( true ) );
      EXPECT_CALL( db, value( 0 ) ).WillOnce( Return( QVariant( "5" ) ) );
      EXPECT_CALL( db, value( 1 ) ).WillOnce( Return( QVariant( "molarExtinction" ) ) );
      EXPECT_CALL( db, value( 2 ) )
         .WillOnce( Return( QVariant( "<EProfileData><extinctionProfile componentID='17' componentType='Analyte' "
                                      "valueType='molarExtinction'><extinction wavelength='300' value='1'/>"
                                      "</extinctionProfile></EProfileData>" ) ) );
      EXPECT_CALL( db, statusQuery( Matcher<const QStringList&>( _ ) ) )
         .WillOnce( Invoke(
            [&profileXml]( const QStringList& q )
            {
               EXPECT_EQ( ( QStringList{ "update_eprofile", "5", "17", "Analyte", "molarExtinction" } ), q.mid( 0, 5 ) );
               profileXml = q.value( 5 );
               return IUS_DB2::OK;
            } ) );
      EXPECT_CALL( db, lastErrno() ).WillOnce( Return( IUS_DB2::OK ) );
   }

   EXPECT_EQ( IUS_DB2::OK, analyte.write( true, QString(), &db ) );
   EXPECT_EQ( replacement, profileValues( profileXml ) );
   EXPECT_EQ( replacement, analyte.extinction );
   EXPECT_FALSE( analyte.replace_spectrum );

   US_Analyte disk;
   ASSERT_EQ( IUS_DB2::OK, disk.load( false, kGuid ) );
   EXPECT_EQ( replacement, disk.extinction );
}

TEST_F( AnalyteDb, WriteSkipsProfileWhenTheSpectrumIsUnchanged )
{
   prepareProtein();
   analyte.extinction[280.0] = 43824.0;
   {
      InSequence sequence;
      expectLookup( true );
      EXPECT_CALL( db, statusQuery( Matcher<const QStringList&>( _ ) ) ).WillOnce( Return( IUS_DB2::OK ) );
      EXPECT_CALL( db, lastErrno() ).Times( AtLeast( 1 ) ).WillRepeatedly( Return( IUS_DB2::OK ) );
   }
   EXPECT_EQ( IUS_DB2::OK, analyte.write( true, QString(), &db ) );
}

TEST_F( AnalyteDb, FailedWriteStopsBeforeInsertIdNucleotidesProfileAndDisk )
{
   prepareProtein();
   analyte.type                    = US_Analyte::DNA;
   analyte.analyteID               = "previous";
   analyte.extinction[280.0]       = 43824.0;
   analyte.new_or_changed_spectrum = true;

   // A successful write would overwrite this file, which has the same GUID.
   QString path;
   ASSERT_TRUE( US_Analyte::analyte_path( path ) );
   US_Analyte stored;
   stored.analyteGUID = kGuid;
   stored.description = "stored on disk";
   ASSERT_EQ( IUS_DB2::OK, stored.write( false, path + "/A0000001.xml" ) );
   const QMap<QString, QByteArray> before = analyteFiles();
   ASSERT_EQ( QStringList{ "A0000001.xml" }, before.keys() );

   EXPECT_CALL( db, lastInsertID() ).Times( 0 );
   {
      InSequence sequence;
      expectLookup( false );
      EXPECT_CALL( db, statusQuery( Matcher<const QStringList&>( _ ) ) ).WillOnce( Return( IUS_DB2::DBERROR ) );
      EXPECT_CALL( db, lastErrno() ).Times( AtLeast( 1 ) ).WillRepeatedly( Return( IUS_DB2::DBERROR ) );
   }

   EXPECT_EQ( IUS_DB2::DBERROR, analyte.write( true, QString(), &db ) );
   EXPECT_EQ( QString( "Could not update the DB" ), analyte.message );
   EXPECT_EQ( QString( "previous" ), analyte.analyteID );
   EXPECT_TRUE( analyte.new_or_changed_spectrum );
   EXPECT_EQ( before, analyteFiles() );
}

TEST_F( AnalyteDb, InvalidGuidIsRejectedWithoutDatabaseAccess )
{
   for ( const QString& guid : { QString(), QString( "short" ), kGuid + "0" } )
   {
      SCOPED_TRACE( guid.toStdString() );
      analyte.analyteGUID = guid;
      EXPECT_EQ( IUS_DB2::BADGUID, analyte.write( true, QString(), &db ) );
      EXPECT_EQ( QString( "The analyte GUID is invalid" ), analyte.message );
   }
}
} // namespace
