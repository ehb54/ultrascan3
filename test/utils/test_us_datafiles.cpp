// test_us_datafiles_unit.cpp - Unit tests for US_DataFiles class
#include "qt_test_base.h"
#include "us_datafiles.h"
#include <QString>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QTemporaryDir>
#include <QXmlStreamWriter>

using namespace qt_matchers;

class TestUSDataFilesUnit : public QtTestBase {
protected:
    void SetUp() override {
        QtTestBase::SetUp();

        // Create temporary directory for each test
        tempDir = std::make_unique<QTemporaryDir>();
        ASSERT_TRUE(tempDir->isValid()) << "Failed to create temporary directory";
        testPath = tempDir->path();
    }

    void TearDown() override {
        // Cleanup happens automatically with QTemporaryDir destructor
        tempDir.reset();
        QtTestBase::TearDown();
    }

    // Helper method to create a test XML file with specific content
    void createXmlFile(const QString& filename, const QString& tag,
                       const QString& attribute, const QString& guid) {
        QFile file(testPath + "/" + filename);
        ASSERT_TRUE(file.open(QIODevice::WriteOnly | QIODevice::Text));

        QXmlStreamWriter xml(&file);
        xml.setAutoFormatting(true);
        xml.writeStartDocument();
        xml.writeStartElement("root");
        xml.writeStartElement(tag);
        xml.writeAttribute(attribute, guid);
        xml.writeEndElement(); // tag
        xml.writeEndElement(); // root
        xml.writeEndDocument();

        file.close();
    }

    // Helper method to create a simple XML file without specific content
    void createEmptyXmlFile(const QString& filename) {
        QFile file(testPath + "/" + filename);
        ASSERT_TRUE(file.open(QIODevice::WriteOnly | QIODevice::Text));

        QXmlStreamWriter xml(&file);
        xml.writeStartDocument();
        xml.writeStartElement("root");
        xml.writeEndElement();
        xml.writeEndDocument();

        file.close();
    }

    // Helper method to verify file name format
    bool isValidFileName(const QString& fileName, const QString& lfchar) {
        QString baseName = QFileInfo(fileName).baseName();
        if (baseName.length() != 8) return false;
        if (!baseName.startsWith(lfchar)) return false;

        QString numPart = baseName.mid(1);
        bool ok;
        numPart.toInt(&ok);
        return ok;
    }

    std::unique_ptr<QTemporaryDir> tempDir;
    QString testPath;
};

// ============================================================================
// MAIN GET_FILENAME METHOD TESTS (with newFile reference parameter)
// ============================================================================

TEST_F(TestUSDataFilesUnit, GetFilenameEmptyDirectory) {
// Test with empty directory - should create first file
bool newFile = false;
QString result = US_DataFiles::get_filename(testPath, "test-guid", "M",
                                            "model", "guid", newFile);

EXPECT_TRUE(newFile) << "Should indicate new file creation";
EXPECT_TRUE(result.endsWith("/M0000001.xml")) << "Should create first numbered file";
EXPECT_TRUE(result.startsWith(testPath)) << "Should use correct path";
}

TEST_F(TestUSDataFilesUnit, GetFilenameEmptyGuid) {
// Test with empty GUID - should skip XML searching and create new file
createEmptyXmlFile("M0000001.xml");

bool newFile = false;
QString result = US_DataFiles::get_filename(testPath, "", "M",
                                            "model", "guid", newFile);

EXPECT_TRUE(newFile) << "Should indicate new file creation with empty GUID";
EXPECT_TRUE(result.endsWith("/M0000002.xml")) << "Should create next numbered file";
}

TEST_F(TestUSDataFilesUnit, GetFilenameMatchingGuidFound) {
// Test finding existing file with matching GUID
QString testGuid = "12345678-1234-1234-1234-123456789abc";
createXmlFile("M0000001.xml", "model", "guid", testGuid);

bool newFile = true; // Start as true to verify it gets set to false
QString result = US_DataFiles::get_filename(testPath, testGuid, "M",
                                            "model", "guid", newFile);

EXPECT_FALSE(newFile) << "Should indicate existing file found";
EXPECT_TRUE(result.endsWith("/M0000001.xml")) << "Should return existing file";
EXPECT_TRUE(result.startsWith(testPath)) << "Should use correct path";
}

TEST_F(TestUSDataFilesUnit, GetFilenameNoMatchingGuid) {
// Test with GUID that doesn't match any existing file
QString existingGuid = "existing-guid";
QString searchGuid = "different-guid";
createXmlFile("M0000001.xml", "model", "guid", existingGuid);

bool newFile = false;
QString result = US_DataFiles::get_filename(testPath, searchGuid, "M",
                                            "model", "guid", newFile);

EXPECT_TRUE(newFile) << "Should indicate new file creation";
EXPECT_TRUE(result.endsWith("/M0000002.xml")) << "Should create next numbered file";
}

TEST_F(TestUSDataFilesUnit, GetFilenameGapInNumbering) {
// Test finding gap in file numbering
createEmptyXmlFile("M0000001.xml");
createEmptyXmlFile("M0000003.xml"); // Gap at 0000002

bool newFile = false;
QString result = US_DataFiles::get_filename(testPath, "non-existing-guid", "M",
                                            "model", "guid", newFile);

EXPECT_TRUE(newFile) << "Should indicate new file creation";
EXPECT_TRUE(result.endsWith("/M0000002.xml")) << "Should fill the gap";
}

TEST_F(TestUSDataFilesUnit, GetFilenameMultipleGaps) {
// Test with multiple gaps - should use first gap
createEmptyXmlFile("M0000001.xml");
createEmptyXmlFile("M0000004.xml"); // Gaps at 0000002 and 0000003
createEmptyXmlFile("M0000006.xml");

bool newFile = false;
QString result = US_DataFiles::get_filename(testPath, "non-existing-guid", "M",
                                            "model", "guid", newFile);

EXPECT_TRUE(newFile) << "Should indicate new file creation";
EXPECT_TRUE(result.endsWith("/M0000002.xml")) << "Should use first gap";
}

TEST_F(TestUSDataFilesUnit, GetFilenameIncrementPastLast) {
// Test incrementing past last existing file when no gaps
createEmptyXmlFile("M0000001.xml");
createEmptyXmlFile("M0000002.xml");
createEmptyXmlFile("M0000003.xml");

bool newFile = false;
QString result = US_DataFiles::get_filename(testPath, "non-existing-guid", "M",
                                            "model", "guid", newFile);

EXPECT_TRUE(newFile) << "Should indicate new file creation";
EXPECT_TRUE(result.endsWith("/M0000004.xml")) << "Should increment past last";
}

TEST_F(TestUSDataFilesUnit, GetFilenameDifferentLeadingChar) {
// Test with different leading character
createEmptyXmlFile("N0000001.xml");

bool newFile = false;
QString result = US_DataFiles::get_filename(testPath, "", "N",
                                            "noise", "guid", newFile);

EXPECT_TRUE(newFile) << "Should indicate new file creation";
EXPECT_TRUE(result.endsWith("/N0000002.xml")) << "Should create next N file";
EXPECT_TRUE(isValidFileName(result, "N")) << "Should have valid N file format";
}

TEST_F(TestUSDataFilesUnit, GetFilenameDifferentTagAndAttribute) {
// Test with different XML tag and attribute names
QString testGuid = "special-guid";
createXmlFile("S0000001.xml", "analyte", "analyteGUID", testGuid);

bool newFile = true;
QString result = US_DataFiles::get_filename(testPath, testGuid, "S",
                                            "analyte", "analyteGUID", newFile);

EXPECT_FALSE(newFile) << "Should find existing file with different tag/attribute";
EXPECT_TRUE(result.endsWith("/S0000001.xml")) << "Should return existing file";
}

TEST_F(TestUSDataFilesUnit, GetFilenameWrongTagName) {
// Test searching for wrong tag name - should not find match
QString testGuid = "test-guid";
createXmlFile("M0000001.xml", "model", "guid", testGuid);

bool newFile = false;
QString result = US_DataFiles::get_filename(testPath, testGuid, "M",
                                            "wrongtag", "guid", newFile);

EXPECT_TRUE(newFile) << "Should not find match with wrong tag name";
EXPECT_TRUE(result.endsWith("/M0000002.xml")) << "Should create new file";
}

TEST_F(TestUSDataFilesUnit, GetFilenameWrongAttributeName) {
// Test searching for wrong attribute name - should not find match
QString testGuid = "test-guid";
createXmlFile("M0000001.xml", "model", "guid", testGuid);

bool newFile = false;
QString result = US_DataFiles::get_filename(testPath, testGuid, "M",
                                            "model", "wrongattr", newFile);

EXPECT_TRUE(newFile) << "Should not find match with wrong attribute name";
EXPECT_TRUE(result.endsWith("/M0000002.xml")) << "Should create new file";
}

// ============================================================================
// EDGE CASES AND ERROR CONDITIONS
// ============================================================================

TEST_F(TestUSDataFilesUnit, GetFilenameNonExistentPath) {
// Test with non-existent directory path
QString fakePath = testPath + "/nonexistent";

bool newFile = false;
QString result = US_DataFiles::get_filename(fakePath, "guid", "M",
                                            "model", "guid", newFile);

EXPECT_TRUE(newFile) << "Should indicate new file for non-existent path";
EXPECT_TRUE(result.endsWith("/M0000001.xml")) << "Should create first file";
EXPECT_TRUE(result.startsWith(fakePath)) << "Should use provided path";
}

TEST_F(TestUSDataFilesUnit, GetFilenameEmptyParameters) {
// Test with empty string parameters
bool newFile = false;
QString result = US_DataFiles::get_filename("", "", "", "", "", newFile);

EXPECT_TRUE(newFile) << "Should indicate new file with empty parameters";
EXPECT_TRUE(result.endsWith("/0000001.xml")) << "Should handle empty leading char";
}

TEST_F(TestUSDataFilesUnit, GetFilenameUnreadableFile) {
// Create a file but make it unreadable (this test may be platform dependent)
QFile file(testPath + "/M0000001.xml");
ASSERT_TRUE(file.open(QIODevice::WriteOnly));
file.write("invalid xml content");
file.close();

// Note: Making file unreadable may not work on all platforms in test environment
// but we test the behavior when file can't be opened
bool newFile = false;
QString result = US_DataFiles::get_filename(testPath, "any-guid", "M",
                                            "model", "guid", newFile);

// Should either find the file or create a new one
EXPECT_TRUE(result.contains(".xml")) << "Should return valid filename";
}

TEST_F(TestUSDataFilesUnit, GetFilenameLargeNumbering) {
// Test with large file numbers
createEmptyXmlFile("M0000999.xml");
createEmptyXmlFile("M0001000.xml");

bool newFile = false;
QString result = US_DataFiles::get_filename(testPath, "non-existing", "M",
                                            "model", "guid", newFile);

EXPECT_TRUE(newFile) << "Should indicate new file creation";
EXPECT_TRUE(result.endsWith("/M0001001.xml")) << "Should handle large numbers";
}

TEST_F(TestUSDataFilesUnit, GetFilenameMaxNumberFormat) {
// Test with maximum 7-digit number
createEmptyXmlFile("M9999999.xml");

bool newFile = false;
QString result = US_DataFiles::get_filename(testPath, "non-existing", "M",
                                            "model", "guid", newFile);

EXPECT_TRUE(newFile) << "Should indicate new file creation";
// Note: This will overflow the 7-digit format, behavior depends on implementation
EXPECT_TRUE(result.contains(".xml")) << "Should still return XML filename";
}

// ============================================================================
// OVERLOADED METHOD TESTS (without newFile parameter)
// ============================================================================

TEST_F(TestUSDataFilesUnit, GetFilenameOverloadEmptyDirectory) {
// Test overloaded method with empty directory
QString result = US_DataFiles::get_filename(testPath, "test-guid", "M",
                                            "model", "guid");

EXPECT_TRUE(result.endsWith("/M0000001.xml")) << "Should create first numbered file";
EXPECT_TRUE(result.startsWith(testPath)) << "Should use correct path";
}

TEST_F(TestUSDataFilesUnit, GetFilenameOverloadMatchingGuid) {
// Test overloaded method finding existing file
QString testGuid = "existing-guid";
createXmlFile("M0000001.xml", "model", "guid", testGuid);

QString result = US_DataFiles::get_filename(testPath, testGuid, "M",
                                            "model", "guid");

EXPECT_TRUE(result.endsWith("/M0000001.xml")) << "Should return existing file";
}

TEST_F(TestUSDataFilesUnit, GetFilenameOverloadGapFilling) {
// Test overloaded method gap filling
createEmptyXmlFile("M0000001.xml");
createEmptyXmlFile("M0000003.xml");

QString result = US_DataFiles::get_filename(testPath, "non-existing", "M",
                                            "model", "guid");

EXPECT_TRUE(result.endsWith("/M0000002.xml")) << "Should fill the gap";
}

TEST_F(TestUSDataFilesUnit, GetFilenameOverloadConsistency) {
// Test that overloaded method produces same result as main method
QString testGuid = "consistency-test";

bool newFile = false;
QString result1 = US_DataFiles::get_filename(testPath, testGuid, "M",
                                             "model", "guid", newFile);
QString result2 = US_DataFiles::get_filename(testPath, testGuid, "M",
                                             "model", "guid");

EXPECT_EQ(result1, result2) << "Both methods should return same result";
EXPECT_TRUE(newFile) << "Main method should indicate new file";
}

// ============================================================================
// FILENAME FORMAT VALIDATION TESTS
// ============================================================================

TEST_F(TestUSDataFilesUnit, GetFilenameFormatValidation) {
// Test that generated filenames follow correct format
bool newFile = false;
QString result = US_DataFiles::get_filename(testPath, "", "X",
                                            "test", "attr", newFile);

QString fileName = QFileInfo(result).fileName();

EXPECT_EQ(fileName.length(), 12) << "Filename should be 12 characters (X0000001.xml)";
EXPECT_TRUE(fileName.startsWith("X")) << "Should start with leading character";
EXPECT_TRUE(fileName.endsWith(".xml")) << "Should end with .xml";

QString numPart = fileName.mid(1, 7);
bool ok;
int num = numPart.toInt(&ok);
EXPECT_TRUE(ok) << "Middle part should be valid number";
EXPECT_GE(num, 1) << "Number should be at least 1";
}

TEST_F(TestUSDataFilesUnit, GetFilenamePathHandling) {
// Test proper path concatenation
QString customPath = testPath + "/subdir";
QDir().mkpath(customPath);

bool newFile = false;
QString result = US_DataFiles::get_filename(customPath, "", "P",
                                            "test", "attr", newFile);

EXPECT_TRUE(result.startsWith(customPath)) << "Should use provided path";
EXPECT_TRUE(result.contains("/P0000001.xml")) << "Should append correct filename";
EXPECT_EQ(result.count("/"), testPath.count("/") + 2) << "Should have correct path depth";
}