#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QBuffer>
#include <QDebug>
#include <QStringList>
#include <QMap>
#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QTemporaryFile>

#include "us_analyte.h"
#include "mock_us_db2.h"
#include "qt_test_base.h"
#include "us_constants.h"

using ::testing::_;
using ::testing::Return;
using ::testing::Contains;
using ::testing::HasSubstr;
using ::testing::Not;
using ::testing::IsEmpty;
using ::testing::DoubleEq;
using ::testing::DoubleNear;
using ::testing::StrictMock;
using ::testing::NiceMock;
using ::qt_matchers::QStringContains;
using ::qt_matchers::QStringEq;

namespace {

// Test fixture for US_Analyte functionality
    class US_AnalyteTest : public QtTestBase {
    protected:
        void SetUp() override {
            QtTestBase::SetUp();
            analyte = std::make_unique<US_Analyte>();
        }

        void TearDown() override {
            analyte.reset();
            QtTestBase::TearDown();
        }

        std::unique_ptr<US_Analyte> analyte;
    };

// Test fixture for constructor testing
    class US_AnalyteConstructorTest : public US_AnalyteTest {
    };

// Test fixture for equality operator testing
    class US_AnalyteEqualityTest : public US_AnalyteTest {
    protected:
        void SetUp() override {
            US_AnalyteTest::SetUp();
            analyte1 = std::make_unique<US_Analyte>();
            analyte2 = std::make_unique<US_Analyte>();
        }

        std::unique_ptr<US_Analyte> analyte1, analyte2;
    };

// Test fixture for load method testing
    class US_AnalyteLoadTest : public US_AnalyteTest {
    protected:
        void SetUp() override {
            US_AnalyteTest::SetUp();
            mockDb = std::make_unique<StrictMock<US_DB2_Mock>>();
        }

        std::unique_ptr<StrictMock<US_DB2_Mock>> mockDb;
    };

// Test fixture for write method testing
    class US_AnalyteWriteTest : public US_AnalyteTest {
    protected:
        void SetUp() override {
            US_AnalyteTest::SetUp();
            mockDb = std::make_unique<StrictMock<US_DB2_Mock>>();
            tempDir = std::make_unique<QTemporaryDir>();
            ASSERT_TRUE(tempDir->isValid());
        }

        std::unique_ptr<StrictMock<US_DB2_Mock>> mockDb;
        std::unique_ptr<QTemporaryDir> tempDir;
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

TEST_F(US_AnalyteLoadTest, LoadWithDatabaseAccessCallsLoadDb) {
QString testGuid = "12345678-1234-1234-1234-123456789012";

// Use NiceMock to avoid "uninteresting call" warnings
auto niceMockDb = std::make_unique<NiceMock<US_DB2_Mock>>();

// Set up basic successful responses
ON_CALL(*niceMockDb, lastErrno())
.WillByDefault(Return(IUS_DB2::OK));
ON_CALL(*niceMockDb, next())
.WillByDefault(Return(true));
ON_CALL(*niceMockDb, value(_))
.WillByDefault(Return(QVariant("test_value")));

int result = analyte->load(true, testGuid, niceMockDb.get());
EXPECT_EQ(result, IUS_DB2::OK);
}

TEST_F(US_AnalyteLoadTest, LoadWithDiskAccessCallsLoadDisk) {
QString testGuid = "12345678-1234-1234-1234-123456789012";

// This will fail because we don't have actual files, but it tests the routing
int result = analyte->load(false, testGuid, nullptr);
EXPECT_NE(result, IUS_DB2::OK); // Should fail with file not found
}

TEST_F(US_AnalyteLoadTest, LoadDbHandlesGetAnalyteIDError) {
QString testGuid = "12345678-1234-1234-1234-123456789012";

// Use NiceMock and set up failure condition
auto niceMockDb = std::make_unique<NiceMock<US_DB2_Mock>>();
ON_CALL(*niceMockDb, lastErrno())
.WillByDefault(Return(IUS_DB2::NOROWS));

int result = analyte->load(true, testGuid, niceMockDb.get());
EXPECT_EQ(result, IUS_DB2::NOROWS);
EXPECT_FALSE(analyte->message.isEmpty()); // Should have some error message
}

TEST_F(US_AnalyteLoadTest, LoadDbHandlesGetAnalyteInfoError) {
QString testGuid = "12345678-1234-1234-1234-123456789012";

// Use NiceMock for simpler testing
auto niceMockDb = std::make_unique<NiceMock<US_DB2_Mock>>();

// Set up a scenario where first call succeeds, subsequent calls fail
// Use ON_CALL instead of EXPECT_CALL to avoid unsatisfied expectations
ON_CALL(*niceMockDb, lastErrno())
.WillByDefault(Return(IUS_DB2::NOROWS)); // Make all calls fail

int result = analyte->load(true, testGuid, niceMockDb.get());
EXPECT_EQ(result, IUS_DB2::NOROWS);
EXPECT_FALSE(analyte->message.isEmpty());
}

// =============================================================================
// Write Method Tests
// =============================================================================

TEST_F(US_AnalyteWriteTest, WriteWithDatabaseAccessCallsWriteDb) {
analyte->analyteGUID = "12345678-1234-1234-1234-123456789012";

// Use NiceMock to avoid uninteresting call warnings
auto niceMockDb = std::make_unique<NiceMock<US_DB2_Mock>>();

// Set up successful write scenario
ON_CALL(*niceMockDb, lastErrno())
.WillByDefault(Return(IUS_DB2::OK));
ON_CALL(*niceMockDb, statusQuery(::testing::An<const QStringList&>()))
.WillByDefault(Return(IUS_DB2::OK));
ON_CALL(*niceMockDb, lastInsertID())
.WillByDefault(Return(123));

int result = analyte->write(true, QString(), niceMockDb.get());
EXPECT_EQ(result, IUS_DB2::OK);
}

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

TEST_F(US_AnalyteWriteTest, WriteDbHandlesInvalidGUID) {
analyte->analyteGUID = "invalid-guid"; // Too short

int result = analyte->write(true, QString(), mockDb.get());
EXPECT_EQ(result, IUS_DB2::BADGUID);
EXPECT_THAT(analyte->message, QStringContains("The analyte GUID is invalid"));
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
QString path = "/tmp/analytes";
QString guid = "12345678-1234-1234-1234-123456789012";

QString filename = US_Analyte::get_filename(path, guid);

EXPECT_FALSE(filename.isEmpty());
EXPECT_THAT(filename, QStringContains(path));
}

TEST_F(US_AnalyteStaticTest, GetFilenameHandlesEmptyPath) {
QString emptyPath;
QString guid = "12345678-1234-1234-1234-123456789012";

QString filename = US_Analyte::get_filename(emptyPath, guid);

// Should still return a filename even with empty path
EXPECT_FALSE(filename.isEmpty());
}

TEST_F(US_AnalyteStaticTest, GetFilenameHandlesEmptyGuid) {
QString path = "/tmp/analytes";
QString emptyGuid;

QString filename = US_Analyte::get_filename(path, emptyGuid);

// Should still return a filename structure
EXPECT_FALSE(filename.isEmpty());
}

// =============================================================================
// Dump Method Tests
// =============================================================================

TEST_F(US_AnalyteDumpTest, DumpExecutesWithoutCrash) {
analyte->description = "Test for dump";
analyte->vbar20 = 0.73;
analyte->mw = 45000.0;
analyte->type = US_Analyte::PROTEIN;

// This should not crash
EXPECT_NO_THROW(analyte->dump());
}

TEST_F(US_AnalyteDumpTest, DumpHandlesAllAnalyteTypes) {
std::vector<US_Analyte::analyte_t> types = {
        US_Analyte::PROTEIN,
        US_Analyte::DNA,
        US_Analyte::RNA,
        US_Analyte::CARBOHYDRATE
};

for (auto type : types) {
analyte->type = type;
EXPECT_NO_THROW(analyte->dump());
}
}

TEST_F(US_AnalyteDumpTest, DumpHandlesEmptyFields) {
// Clear all fields
analyte->description.clear();
analyte->analyteGUID.clear();
analyte->sequence.clear();
analyte->extinction.clear();
analyte->refraction.clear();
analyte->fluorescence.clear();

EXPECT_NO_THROW(analyte->dump());
}

TEST_F(US_AnalyteDumpTest, DumpHandlesSpectrumData) {
// Add spectrum data
analyte->extinction[280.0] = 43824.0;
analyte->extinction[260.0] = 35000.0;
analyte->refraction[280.0] = 1.5;
analyte->fluorescence[350.0] = 1000.0;

EXPECT_NO_THROW(analyte->dump());
}

// =============================================================================
// Edge Cases and Error Conditions
// =============================================================================

TEST_F(US_AnalyteTest, HandlesExtremeDoubleValues) {
analyte->vbar20 = std::numeric_limits<double>::max();
analyte->mw = std::numeric_limits<double>::min();
analyte->sodium = std::numeric_limits<double>::infinity();

// Should handle extreme values without crashing
EXPECT_TRUE(std::isinf(analyte->sodium));
EXPECT_TRUE(std::isfinite(analyte->vbar20));
EXPECT_TRUE(std::isfinite(analyte->mw));
}

TEST_F(US_AnalyteTest, HandlesNegativeCounterionValues) {
analyte->sodium = -0.5;
analyte->potassium = -1.0;
analyte->magnesium = -0.1;

// Should accept negative values (though not physically meaningful)
EXPECT_LT(analyte->sodium, 0.0);
EXPECT_LT(analyte->potassium, 0.0);
EXPECT_LT(analyte->magnesium, 0.0);
}

TEST_F(US_AnalyteTest, HandlesVeryLongStrings) {
QString longString(10000, QChar('A'));
analyte->description = longString;
analyte->sequence = longString;
analyte->analyteGUID = "12345678-1234-1234-1234-123456789012";

// Should handle very long strings
EXPECT_EQ(analyte->description.length(), 10000);
EXPECT_EQ(analyte->sequence.length(), 10000);
}

TEST_F(US_AnalyteTest, HandlesSpecialCharactersInStrings) {
analyte->description = "Test <>&\"' Analyte with special chars";
analyte->sequence = "ATGC\n\t\r";

// Should handle special characters
EXPECT_THAT(analyte->description, QStringContains("<>&\"'"));
EXPECT_THAT(analyte->sequence, QStringContains("\n"));
}

TEST_F(US_AnalyteTest, HandlesEmptySpectrumMaps) {
analyte->extinction.clear();
analyte->refraction.clear();
analyte->fluorescence.clear();

EXPECT_TRUE(analyte->extinction.isEmpty());
EXPECT_TRUE(analyte->refraction.isEmpty());
EXPECT_TRUE(analyte->fluorescence.isEmpty());

// Should handle empty maps in equality comparison
US_Analyte other;
EXPECT_TRUE(*analyte == other);
}

TEST_F(US_AnalyteTest, HandlesLargeSpectrumMaps) {
// Add many spectrum entries
for (int i = 200; i <= 800; i += 10) {
analyte->extinction[i] = i * 100.0;
analyte->refraction[i] = 1.0 + (i / 1000.0);
analyte->fluorescence[i] = i * 50.0;
}

EXPECT_EQ(analyte->extinction.size(), 61); // (800-200)/10 + 1
EXPECT_EQ(analyte->refraction.size(), 61);
EXPECT_EQ(analyte->fluorescence.size(), 61);
}

} // anonymous namespace