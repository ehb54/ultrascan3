//! \file test_us_analyte.cpp
#include <QBuffer>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QLoggingCategory>
#include <QMap>
#include <QStringList>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include "mock_us_db2.h"
#include "qt_test_base.h"
#include "us3_test_io.h"
#include "us_analyte.h"
#include "us_constants.h"
#include "us_settings.h"

using ::qt_matchers::QStringContains;
using ::qt_matchers::QStringEq;
using ::testing::_;
using ::testing::Contains;
using ::testing::DoubleEq;
using ::testing::DoubleNear;
using ::testing::HasSubstr;
using ::testing::IsEmpty;
using ::testing::Not;
using ::testing::Return;
using ::testing::StrictMock;

namespace {

// Test fixture for US_Analyte functionality
class US_AnalyteTest : public QtTestBase
{
   protected:
      void SetUp() override
      {
         ASSERT_NO_FATAL_FAILURE( QtTestBase::SetUp() );
         analyte = std::make_unique<US_Analyte>();
      }

      void TearDown() override
      {
         analyte.reset();
         QtTestBase::TearDown();
      }

      std::unique_ptr<US_Analyte> analyte;
};

// Test fixture for constructor testing
    class US_AnalyteConstructorTest : public US_AnalyteTest {
    };

    // Test fixture for equality operator testing
    class US_AnalyteEqualityTest : public US_AnalyteTest
    {
       protected:
          void SetUp() override
          {
             ASSERT_NO_FATAL_FAILURE( US_AnalyteTest::SetUp() );
             analyte1 = std::make_unique<US_Analyte>();
             analyte2 = std::make_unique<US_Analyte>();
          }

          std::unique_ptr<US_Analyte> analyte1, analyte2;
    };

    // Test fixture for load method testing
    class US_AnalyteLoadTest : public US_AnalyteTest
    {
       protected:
          void SetUp() override
          {
             ASSERT_NO_FATAL_FAILURE( US_AnalyteTest::SetUp() );
             mockDb = std::make_unique<StrictMock<US_DB2_Mock>>();
          }

          std::unique_ptr<StrictMock<US_DB2_Mock>> mockDb;
    };

    // Test fixture for write method testing
    class US_AnalyteWriteTest : public US_AnalyteTest
    {
       protected:
          void SetUp() override
          {
             ASSERT_NO_FATAL_FAILURE( US_AnalyteTest::SetUp() );
             mockDb  = std::make_unique<StrictMock<US_DB2_Mock>>();
             tempDir = std::make_unique<QTemporaryDir>();
             ASSERT_TRUE( tempDir->isValid() );
          }

          std::unique_ptr<StrictMock<US_DB2_Mock>> mockDb;
          std::unique_ptr<QTemporaryDir>           tempDir;
    };

// Test fixture for static methods testing
    class US_AnalyteStaticTest : public US_AnalyteTest {
    };

// Test fixture for dump method testing
    class US_AnalyteDumpTest : public US_AnalyteTest {
    };

// =============================================================================
// Constructor Tests
// =============================================================================

    TEST_F(US_AnalyteConstructorTest, DefaultConstructorInitializesBasicFields) {
    EXPECT_THAT(analyte->vbar20, DoubleEq(TYPICAL_VBAR));
    EXPECT_THAT(analyte->mw, DoubleEq(0.0));
    EXPECT_THAT(analyte->description, QStringEq("New Analyte"));
    EXPECT_TRUE(analyte->analyteGUID.isEmpty());
    EXPECT_TRUE(analyte->sequence.isEmpty());
    EXPECT_EQ(analyte->type, US_Analyte::PROTEIN);
    EXPECT_FALSE(analyte->grad_form);
}

TEST_F(US_AnalyteConstructorTest, DefaultConstructorInitializesSpectrumFlags) {
EXPECT_FALSE(analyte->replace_spectrum);
EXPECT_FALSE(analyte->new_or_changed_spectrum);
}

TEST_F(US_AnalyteConstructorTest, DefaultConstructorInitializesNucleotideProperties) {
EXPECT_TRUE(analyte->doubleStranded);
EXPECT_FALSE(analyte->complement);
EXPECT_FALSE(analyte->_3prime);
EXPECT_FALSE(analyte->_5prime);
EXPECT_THAT(analyte->sodium, DoubleEq(0.0));
EXPECT_THAT(analyte->potassium, DoubleEq(0.0));
EXPECT_THAT(analyte->lithium, DoubleEq(0.0));
EXPECT_THAT(analyte->magnesium, DoubleEq(0.0));
EXPECT_THAT(analyte->calcium, DoubleEq(0.0));
}

TEST_F(US_AnalyteConstructorTest, DefaultConstructorInitializesSpectrumMaps) {
EXPECT_TRUE(analyte->extinction.isEmpty());
EXPECT_TRUE(analyte->refraction.isEmpty());
EXPECT_TRUE(analyte->fluorescence.isEmpty());
}

TEST_F(US_AnalyteConstructorTest, DefaultConstructorInitializesInternalFields) {
EXPECT_TRUE(analyte->message.isEmpty());
EXPECT_TRUE(analyte->analyteID.isEmpty());
}

// =============================================================================
// Equality Operator Tests
// =============================================================================

TEST_F(US_AnalyteEqualityTest, DefaultAnalytesAreEqual) {
EXPECT_TRUE(*analyte1 == *analyte2);
}

TEST_F(US_AnalyteEqualityTest, ModifiedVbar20MakesAnalytesUnequal) {
analyte1->vbar20 = 0.75;
EXPECT_FALSE(*analyte1 == *analyte2);
}

TEST_F(US_AnalyteEqualityTest, ModifiedMolecularWeightMakesAnalytesUnequal) {
analyte1->mw = 50000.0;
EXPECT_FALSE(*analyte1 == *analyte2);
}

TEST_F(US_AnalyteEqualityTest, ModifiedDescriptionMakesAnalytesUnequal) {
analyte1->description = "Modified Analyte";
EXPECT_FALSE(*analyte1 == *analyte2);
}

TEST_F(US_AnalyteEqualityTest, ModifiedGUIDMakesAnalytesUnequal) {
analyte1->analyteGUID = "12345678-1234-1234-1234-123456789012";
EXPECT_FALSE(*analyte1 == *analyte2);
}

TEST_F(US_AnalyteEqualityTest, ModifiedSequenceMakesAnalytesUnequal) {
analyte1->sequence = "ATGCATGC";
EXPECT_FALSE(*analyte1 == *analyte2);
}

TEST_F(US_AnalyteEqualityTest, ModifiedTypeMakesAnalytesUnequal) {
analyte1->type = US_Analyte::DNA;
EXPECT_FALSE(*analyte1 == *analyte2);
}

TEST_F(US_AnalyteEqualityTest, ModifiedGradientFormMakesAnalytesUnequal) {
analyte1->grad_form = true;
EXPECT_FALSE(*analyte1 == *analyte2);
}

TEST_F(US_AnalyteEqualityTest, ModifiedExtinctionMakesAnalytesUnequal) {
analyte1->extinction[280.0] = 43824.0;
EXPECT_FALSE(*analyte1 == *analyte2);
}

TEST_F(US_AnalyteEqualityTest, ModifiedRefractionMakesAnalytesUnequal) {
analyte1->refraction[280.0] = 1.5;
EXPECT_FALSE(*analyte1 == *analyte2);
}

TEST_F(US_AnalyteEqualityTest, ModifiedFluorescenceMakesAnalytesUnequal) {
analyte1->fluorescence[350.0] = 1000.0;
EXPECT_FALSE(*analyte1 == *analyte2);
}

TEST_F(US_AnalyteEqualityTest, DNATypeComparesNucleotideProperties) {
analyte1->type = US_Analyte::DNA;
analyte2->type = US_Analyte::DNA;

EXPECT_TRUE(*analyte1 == *analyte2);

analyte1->doubleStranded = false;
EXPECT_FALSE(*analyte1 == *analyte2);
}

TEST_F(US_AnalyteEqualityTest, RNATypeComparesNucleotideProperties) {
analyte1->type = US_Analyte::RNA;
analyte2->type = US_Analyte::RNA;

EXPECT_TRUE(*analyte1 == *analyte2);

analyte1->complement = true;
EXPECT_FALSE(*analyte1 == *analyte2);
}

TEST_F(US_AnalyteEqualityTest, ProteinTypeIgnoresNucleotideProperties) {
analyte1->type = US_Analyte::PROTEIN;
analyte2->type = US_Analyte::PROTEIN;

// Modify nucleotide properties - should still be equal for proteins
analyte1->doubleStranded = false;
analyte1->sodium = 0.1;
analyte1->_3prime = true;

EXPECT_TRUE(*analyte1 == *analyte2);
}

TEST_F(US_AnalyteEqualityTest, DoesNotCompareMessageAndAnalyteID) {
// These fields are not compared in equality operator
analyte1->message = "Test message";
analyte1->analyteID = "12345";

EXPECT_TRUE(*analyte1 == *analyte2);
}

// =============================================================================
// Load Method Tests
// =============================================================================

TEST_F( US_AnalyteLoadTest, LoadDbHandlesGetAnalyteInfoError )
{
   const QString guid = "synthetic-guid";
   EXPECT_CALL( *mockDb, lastError() ).Times( ::testing::AnyNumber() ).WillRepeatedly( Return( QString() ) );
   ::testing::InSequence sequence;
   EXPECT_CALL( *mockDb, query( ::testing::Matcher<const QStringList&>( ::testing::Eq( QStringList{ "get_analyteID", guid } ) ) ) );
   EXPECT_CALL( *mockDb, lastErrno() ).Times( ::testing::AtLeast( 1 ) ).WillRepeatedly( Return( IUS_DB2::OK ) );
   EXPECT_CALL( *mockDb, next() ).WillOnce( Return( true ) );
   EXPECT_CALL( *mockDb, value( 0 ) ).WillOnce( Return( QVariant( "17" ) ) );
   EXPECT_CALL( *mockDb,
                query( ::testing::Matcher<const QStringList&>( ::testing::Eq( QStringList{ "get_analyte_info", "17" } ) ) ) );
   EXPECT_CALL( *mockDb, lastErrno() ).Times( ::testing::AtLeast( 1 ) ).WillRepeatedly( Return( IUS_DB2::DBERROR ) );
   EXPECT_EQ( analyte->load( true, guid, mockDb.get() ), IUS_DB2::DBERROR );
   EXPECT_FALSE( analyte->message.isEmpty() );
}

// =============================================================================
// Write Method Tests
// =============================================================================

TEST_F(US_AnalyteWriteTest, WriteWithDiskAccessCallsWriteDisk) {
QString filename = tempDir->path() + "/test_analyte.xml";
analyte->analyteGUID = "12345678-1234-1234-1234-123456789012";
analyte->description = "Test Analyte";

int result = analyte->write(false, filename, nullptr);
EXPECT_EQ(result, IUS_DB2::OK);

// Verify file was created
QFile file(filename);
EXPECT_TRUE(file.exists());
}

TEST_F(US_AnalyteWriteTest, WriteDiskHandlesInvalidFilename) {
QString invalidFilename = "/invalid/path/test.xml";

int result = analyte->write(false, invalidFilename, nullptr);
EXPECT_EQ(result, IUS_DB2::DBERROR);
EXPECT_THAT(analyte->message, QStringContains("Cannot open file for writing"));
}

// =============================================================================
// Static Method Tests
// =============================================================================

TEST_F(US_AnalyteStaticTest, AnalytePathReturnsValidPath) {
QString path;
bool result = US_Analyte::analyte_path(path);

EXPECT_TRUE(result);
EXPECT_FALSE(path.isEmpty());
EXPECT_THAT(path, QStringContains("analytes"));
}

TEST_F(US_AnalyteStaticTest, GetFilenameReturnsValidFilename) {
   QString path = US_Settings::workBaseDir() + "/analytes";
   ASSERT_TRUE( QDir().mkpath( path ) );
   QString guid = "12345678-1234-1234-1234-123456789012";

   QString filename = US_Analyte::get_filename( path, guid );

   EXPECT_FALSE( filename.isEmpty() );
   EXPECT_THAT( filename, QStringContains( path ) );
}

TEST_F(US_AnalyteStaticTest, GetFilenameHandlesEmptyPath) {
QString emptyPath;
QString guid = "12345678-1234-1234-1234-123456789012";

QString filename = US_Analyte::get_filename(emptyPath, guid);

// Should still return a filename even with empty path
EXPECT_FALSE(filename.isEmpty());
}

TEST_F(US_AnalyteStaticTest, GetFilenameHandlesEmptyGuid) {
   QString path = US_Settings::workBaseDir() + "/analytes";
   ASSERT_TRUE( QDir().mkpath( path ) );
   QString emptyGuid;

   QString filename = US_Analyte::get_filename( path, emptyGuid );

   // Should still return a filename structure
   EXPECT_FALSE( filename.isEmpty() );
}

// =============================================================================
// Dump Method Tests
// =============================================================================

// Only capture messages emitted on the calling test thread.
thread_local QStringList* capturedLines = nullptr;

void                      captureLine( QtMsgType, const QMessageLogContext&, const QString& line )
{
   if ( capturedLines != nullptr )
   {
      capturedLines->append( line );
   }
}

class ScopedDumpCapture
{
   public:
      explicit ScopedDumpCapture( QStringList& lines ) :
          category( QLoggingCategory::defaultCategory() ),
          debugEnabled( category->isDebugEnabled() ),
          previousLines( capturedLines )
      {
         capturedLines   = &lines;
         previousHandler = qInstallMessageHandler( captureLine );
         // Changing this category directly avoids installFilter's synchronous callbacks.
         category->setEnabled( QtDebugMsg, true );
      }
      ~ScopedDumpCapture()
      {
         category->setEnabled( QtDebugMsg, debugEnabled );
         qInstallMessageHandler( previousHandler );
         capturedLines = previousLines;
      }
      ScopedDumpCapture( const ScopedDumpCapture& )            = delete;
      ScopedDumpCapture& operator=( const ScopedDumpCapture& ) = delete;

   private:
      QLoggingCategory* category;
      bool              debugEnabled;
      QStringList*      previousLines;
      QtMessageHandler  previousHandler;
};

// The lines dump() writes through qDebug(). No process-wide category filter is replaced.
QStringList dumpLines( US_Analyte& analyte )
{
   QStringList lines;
   {
      ScopedDumpCapture capture( lines );
      analyte.dump();
   }
   return lines;
}

TEST_F( US_AnalyteDumpTest, ExistingLoggingCategoryAndRepeatedCaptureRestoreLoggingState )
{
   // Capturing must work when the default category already exists and must restore its state.
   QLoggingCategory*      category       = QLoggingCategory::defaultCategory();
   const bool             initialDebug   = category->isDebugEnabled();
   const QtMessageHandler initialHandler = qInstallMessageHandler( nullptr );
   qInstallMessageHandler( initialHandler );
   for ( bool enabled : { false, true } )
   {
      category->setEnabled( QtDebugMsg, enabled );
      for ( int repetition = 0; repetition < 2; ++repetition )
      {
         EXPECT_FALSE( dumpLines( *analyte ).isEmpty() );
         EXPECT_EQ( enabled, category->isDebugEnabled() );
         const QtMessageHandler restored = qInstallMessageHandler( initialHandler );
         EXPECT_EQ( initialHandler, restored );
         EXPECT_EQ( nullptr, capturedLines );
      }
   }
   category->setEnabled( QtDebugMsg, initialDebug );
}

TEST_F( US_AnalyteDumpTest, DumpNamesEachAnalyteType )
{
   const QList<QPair<US_Analyte::analyte_t, QString>> types{ { US_Analyte::PROTEIN, "Type: PROTEIN" },
                                                             { US_Analyte::DNA, "Type: DNA" },
                                                             { US_Analyte::RNA, "Type: RNA" },
                                                             { US_Analyte::CARBOHYDRATE, "Type: OTHER" } };
   for ( const auto& type : types )
   {
      analyte->type           = type.first;
      const QStringList lines = dumpLines( *analyte );
      ASSERT_FALSE( lines.isEmpty() );
      EXPECT_EQ( type.second, lines.first() );
   }
}

TEST_F( US_AnalyteDumpTest, DumpListsScalarFields )
{
   analyte->description    = "Test for dump";
   analyte->analyteGUID    = "12345678-1234-1234-1234-123456789012";
   analyte->vbar20         = 0.73;
   analyte->mw             = 45000.0;
   const QStringList lines = dumpLines( *analyte );
   EXPECT_TRUE( lines.contains( "vbar20: 0.73" ) ) << lines.join( "\n" ).toStdString();
   EXPECT_TRUE( lines.contains( "mw    : 45000" ) ) << lines.join( "\n" ).toStdString();
   EXPECT_TRUE( lines.contains( "descr : \"Test for dump\"" ) ) << lines.join( "\n" ).toStdString();
   EXPECT_TRUE( lines.contains( "guid  : \"12345678-1234-1234-1234-123456789012\"" ) ) << lines.join( "\n" ).toStdString();
}

TEST_F( US_AnalyteDumpTest, DumpListsEachSpectrumInWavelengthOrder )
{
   analyte->extinction[280.0]   = 43824.0;
   analyte->extinction[260.0]   = 35000.0;
   analyte->refraction[280.0]   = 1.5;
   analyte->fluorescence[350.0] = 1000.0;
   const QStringList lines      = dumpLines( *analyte );
   const int         start      = lines.indexOf( "extinction" );
   ASSERT_GE( start, 0 ) << lines.join( "\n" ).toStdString();
   EXPECT_EQ( ( QStringList{ "extinction", "260 35000", "280 43824", "refraction", "280 1.5", "fluorescence", "350 1000" } ),
              lines.mid( start ) );
}

} // anonymous namespace

TEST_F( US_AnalyteWriteTest, OutputDirectoryAndFileParentFailuresPreserveExistingData )
{
   const QString blocker = tempDir->path() + "/blocker";
   ASSERT_TRUE( us3WriteTestFile( blocker, "sentinel" ) );
   analyte->description = "synthetic analyte";
   for ( const QString& destination : { tempDir->path(), blocker + "/child.xml" } )
   {
      SCOPED_TRACE( destination.toStdString() );
      EXPECT_EQ( analyte->write( false, destination, nullptr ), IUS_DB2::DBERROR );
      EXPECT_FALSE( analyte->message.isEmpty() );
      EXPECT_EQ( analyte->description, QString( "synthetic analyte" ) );
   }
   QFile file( blocker );
   ASSERT_TRUE( file.open( QIODevice::ReadOnly ) );
   EXPECT_EQ( file.readAll(), QByteArray( "sentinel" ) );
   EXPECT_EQ( QDir( tempDir->path() ).entryList( QDir::Files ), QStringList{ "blocker" } );
}

TEST_F( US_AnalyteLoadTest, MissingAndUnrecognizableFilesDoNotReplaceLoadedAnalyte )
{
   QString path;
   ASSERT_TRUE( US_Analyte::analyte_path( path ) );
   analyte->analyteGUID = "previous";
   analyte->description = "previous description";
   EXPECT_EQ( analyte->load( false, "missing" ), IUS_DB2::NO_ANALYTE );
   for ( const QByteArray& bytes : { QByteArray(), QByteArray( "not XML" ), QByteArray( "<AnalyteData><analyte" ) } )
   {
      SCOPED_TRACE( bytes.constData() );
      ASSERT_TRUE( us3WriteTestFile( path + "/A0000001.xml", bytes ) );
      EXPECT_EQ( analyte->load( false, "missing" ), IUS_DB2::NO_ANALYTE );
      EXPECT_EQ( analyte->analyteGUID, QString( "previous" ) );
      EXPECT_EQ( analyte->description, QString( "previous description" ) );
      EXPECT_FALSE( analyte->message.isEmpty() );
   }
}
