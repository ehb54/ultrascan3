// test_us_datafiles.cpp
#include "test_us_datafiles.h"
#include "us_datafiles.h"
#include <QFile>
#include <QXmlStreamWriter>
#include <QDir>

// Use your custom Qt matchers
using namespace qt_matchers;

// TestUSDataFiles method implementations
void TestUSDataFiles::SetUp() {
    QtTestBase::SetUp();
    // Per-test setup - create test directory for each test
    path = QString("tmp").append(QDir::separator()).append("test_data");
    QDir().mkpath(path); // Create test directory
}

void TestUSDataFiles::TearDown() {
    // Per-test cleanup - remove test directory after each test
    QDir(path).removeRecursively(); // Cleanup after each test
    QtTestBase::TearDown();
}

// Suite-level setup for DataFiles tests
void TestUSDataFiles::SetUpTestSuite() {
    QtTestBase::SetUpTestSuite();
    // One-time setup for all TestUSDataFiles tests
}

// Suite-level cleanup for DataFiles tests
void TestUSDataFiles::TearDownTestSuite() {
    // One-time cleanup for all TestUSDataFiles tests
}

// Helper function to create an XML file with the specified GUID
void TestUSDataFiles::createXmlFile(const QString& filename, const QString& tag,
                                    const QString& att, const QString& guid) {
    QFile file(filename);
    ASSERT_TRUE(file.open(QIODevice::WriteOnly))
                                << "Should be able to create XML file: " << filename.toStdString();

    QXmlStreamWriter xmlWriter(&file);
    xmlWriter.setAutoFormatting(true);
    xmlWriter.writeStartDocument();
    xmlWriter.writeStartElement(tag);
    xmlWriter.writeAttribute(att, guid);
    xmlWriter.writeEndElement();
    xmlWriter.writeEndDocument();
    file.close();

    // Verify file was created successfully
    EXPECT_TRUE(QFile::exists(filename))
                        << "XML file should exist after creation: " << filename.toStdString();
}

// Test case for no existing files
TEST_F(TestUSDataFiles, NoExistingFiles) {
    // Arrange
    bool newFile;
    QString guid = "12345";
    QString lfchar = "M";
    QString lkupTag = "model";
    QString lkupAtt = "guid";
    QString expectedFilename = path + QDir::separator() + "M0000001.xml";

    // Act - Test case 1: No existing files, should create the first file
    QString filename = US_DataFiles::get_filename(path, guid, lfchar, lkupTag, lkupAtt, newFile);

    // Assert
    EXPECT_THAT(filename, QStringEq(expectedFilename))
            << "Should create the first file with correct naming pattern";
    EXPECT_TRUE(newFile)
                        << "newFile flag should be true when creating a new file";

    // Create the file to simulate it being written (for subsequent tests)
    createXmlFile(filename, lkupTag, lkupAtt, guid);
}

// Test case for existing file with matching GUID
TEST_F(TestUSDataFiles, ExistingFileWithMatchingGuid) {
    // Arrange - First create an existing file
    bool initialNewFile;
    QString guid = "12345";
    QString lfchar = "M";
    QString lkupTag = "model";
    QString lkupAtt = "guid";

    // Create the initial file
    QString initialFilename = US_DataFiles::get_filename(path, guid, lfchar, lkupTag, lkupAtt, initialNewFile);
    createXmlFile(initialFilename, lkupTag, lkupAtt, guid);

    // Act - Test case 2: Existing file with a matching GUID, should return the existing file
    bool newFile;
    QString filename = US_DataFiles::get_filename(path, guid, lfchar, lkupTag, lkupAtt, newFile);
    QString expectedFilename = path + QDir::separator() + "M0000001.xml";

    // Assert
    EXPECT_THAT(filename, QStringEq(expectedFilename))
            << "Should return the existing file with matching GUID";
    EXPECT_FALSE(newFile)
                        << "newFile flag should be false when returning existing file";
}

// Test case for existing files without matching GUID
TEST_F(TestUSDataFiles, ExistingFilesWithoutMatchingGuid) {
    // Arrange - Create an existing file with different GUID
    bool initialNewFile;
    QString existingGuid = "12345";
    QString newGuid = "67890";
    QString lfchar = "M";
    QString lkupTag = "model";
    QString lkupAtt = "guid";

    // Create the first file with different GUID
    QString initialFilename = US_DataFiles::get_filename(path, existingGuid, lfchar, lkupTag, lkupAtt, initialNewFile);
    createXmlFile(initialFilename, lkupTag, lkupAtt, existingGuid);

    // Act - Test case 3: Existing files without a matching GUID, should create the next file
    bool newFile;
    QString filename = US_DataFiles::get_filename(path, newGuid, lfchar, lkupTag, lkupAtt, newFile);
    QString expectedFilename = path + QDir::separator() + "M0000002.xml";

    // Assert
    EXPECT_THAT(filename, QStringEq(expectedFilename))
            << "Should create the next sequential file when GUID doesn't match";
    EXPECT_TRUE(newFile)
                        << "newFile flag should be true when creating new file for different GUID";

    // Create the second file to simulate it being written
    createXmlFile(filename, lkupTag, lkupAtt, newGuid);
}

// Test case for gap in numbering
TEST_F(TestUSDataFiles, GapInNumbering) {
    // Arrange - Create files with a gap
    bool newFile1, newFile2;
    QString guid1 = "11111";
    QString guid2 = "22222";
    QString guid3 = "33333";
    QString lfchar = "M";
    QString lkupTag = "model";
    QString lkupAtt = "guid";

    // Create first file
    QString filename1 = US_DataFiles::get_filename(path, guid1, lfchar, lkupTag, lkupAtt, newFile1);
    createXmlFile(filename1, lkupTag, lkupAtt, guid1);

    // Create second file
    QString filename2 = US_DataFiles::get_filename(path, guid2, lfchar, lkupTag, lkupAtt, newFile2);
    createXmlFile(filename2, lkupTag, lkupAtt, guid2);

    // Remove the first file to create a gap
    QString fileToRemove = path + QDir::separator() + "M0000001.xml";
    ASSERT_TRUE(QFile::remove(fileToRemove))
                                << "Should be able to remove first file to create gap";

    // Verify the gap exists
    EXPECT_FALSE(QFile::exists(fileToRemove))
                        << "First file should be removed";
    EXPECT_TRUE(QFile::exists(path + QDir::separator() + "M0000002.xml"))
                        << "Second file should still exist";

    // Act - Test case 4: Should fill the gap in numbering
    bool newFile;
    QString filename = US_DataFiles::get_filename(path, guid3, lfchar, lkupTag, lkupAtt, newFile);
    QString expectedFilename = path + QDir::separator() + "M0000001.xml";

    // Assert
    EXPECT_THAT(filename, QStringEq(expectedFilename))
            << "Should fill the gap in numbering";
    EXPECT_TRUE(newFile)
                        << "newFile flag should be true when filling gap";
}

// Additional comprehensive tests
TEST_F(TestUSDataFiles, DifferentFileCharacters) {
    // Test with different leading file characters
    QStringList testChars = {"A", "B", "C", "X", "Z"};
    QString guid = "test123";
    QString lkupTag = "model";
    QString lkupAtt = "guid";

    for (const QString& lfchar : testChars) {
        bool newFile;
        QString filename = US_DataFiles::get_filename(path, guid + lfchar, lfchar, lkupTag, lkupAtt, newFile);
        QString expectedPattern = path + QDir::separator() + lfchar + "0000001.xml";

        EXPECT_THAT(filename, QStringEq(expectedPattern))
                << "Should create correct filename pattern for character: " << lfchar.toStdString();
        EXPECT_TRUE(newFile)
                            << "Should indicate new file for character: " << lfchar.toStdString();

        // Create the file for next iteration
        createXmlFile(filename, lkupTag, lkupAtt, guid + lfchar);
    }
}

TEST_F(TestUSDataFiles, EmptyInputHandling) {
    // Test behavior with empty inputs
    bool newFile;
    QString result;

    // Test with empty GUID
    result = US_DataFiles::get_filename(path, "", "M", "model", "guid", newFile);
    // Should handle empty GUID gracefully (exact behavior depends on implementation)
    EXPECT_FALSE(result.isEmpty()) << "Should handle empty GUID";

    // Test with empty path
    result = US_DataFiles::get_filename("", "testguid", "M", "model", "guid", newFile);
    // Should handle empty path gracefully
    EXPECT_FALSE(result.isEmpty()) << "Should handle empty path";
}

TEST_F(TestUSDataFiles, LargeNumberSequence) {
    // Test that the function can handle larger file numbers correctly
    QString guid = "sequence_test";
    QString lfchar = "S";
    QString lkupTag = "model";
    QString lkupAtt = "guid";

    // Create several files to test sequence numbering
    QStringList createdFiles;
    for (int i = 1; i <= 5; i++) {
        bool newFile;
        QString filename = US_DataFiles::get_filename(path, guid + QString::number(i),
                                                      lfchar, lkupTag, lkupAtt, newFile);

        QString expectedPattern = path + QDir::separator() +
                                  QString("S%1.xml").arg(i, 7, 10, QChar('0'));

        EXPECT_THAT(filename, QStringEq(expectedPattern))
                << "Should create correct sequential filename for iteration: " << i;
        EXPECT_TRUE(newFile)
                            << "Should indicate new file for iteration: " << i;

        createXmlFile(filename, lkupTag, lkupAtt, guid + QString::number(i));
        createdFiles << filename;
    }

    // Verify all files exist
    for (const QString& file : createdFiles) {
        EXPECT_TRUE(QFile::exists(file))
                            << "Created file should exist: " << file.toStdString();
    }
}