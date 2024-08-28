#include "test_us_datafiles.h"
#include <QFile>
#include <QXmlStreamWriter>
#include <QDir>

// Helper function to create an XML file with the specified GUID
void createXmlFile(const QString& filename, const QString& tag, const QString& att, const QString& guid)
{
    QFile file(filename);
    if (file.open(QIODevice::WriteOnly))
    {
        QXmlStreamWriter xmlWriter(&file);
        xmlWriter.setAutoFormatting(true);
        xmlWriter.writeStartDocument();
        xmlWriter.writeStartElement(tag);
        xmlWriter.writeAttribute(att, guid);
        xmlWriter.writeEndElement();
        xmlWriter.writeEndDocument();
        file.close();
    }
}

void TestUSDataFiles::initTestCase()
{
    path = QString("tmp").append(QDir::separator()).append("test_data");
    QDir().mkpath(path); // Create test directory
}

void TestUSDataFiles::cleanupTestCase()
{
    QDir(path).removeRecursively(); // Cleanup after all tests
}

// Test case for no existing files
void TestUSDataFiles::test_no_existing_files()
{
    bool newFile;
    QString guid = "12345";
    QString lfchar = "M";
    QString lkupTag = "model";
    QString lkupAtt = "guid";

    // Test case 1: No existing files, should create the first file
    QString filename = US_DataFiles::get_filename(path, guid, lfchar, lkupTag, lkupAtt, newFile);
    QCOMPARE(filename, path + QDir::separator() + "M0000001.xml");
    QVERIFY(newFile);

    // Create the file to simulate it being written
    createXmlFile(filename, lkupTag, lkupAtt, guid);
}

// Test case for existing file with matching GUID
void TestUSDataFiles::test_existing_file_with_matching_guid()
{
    bool newFile;
    QString guid = "12345";
    QString lfchar = "M";
    QString lkupTag = "model";
    QString lkupAtt = "guid";

    // Test case 2: Existing file with a matching GUID, should return the existing file
    QString filename = US_DataFiles::get_filename(path, guid, lfchar, lkupTag, lkupAtt, newFile);
    QCOMPARE(filename, path + QDir::separator() + "M0000001.xml");
    QVERIFY(!newFile);
}

// Test case for existing files without matching GUID
void TestUSDataFiles::test_existing_files_without_matching_guid()
{
    bool newFile;
    QString guid = "67890";
    QString lfchar = "M";
    QString lkupTag = "model";
    QString lkupAtt = "guid";

    // Test case 3: Existing files without a matching GUID, should create the next file
    QString filename = US_DataFiles::get_filename(path, guid, lfchar, lkupTag, lkupAtt, newFile);
    QCOMPARE(filename, path + QDir::separator() + "M0000002.xml");
    QVERIFY(newFile);

    // Create the second file to simulate it being written
    createXmlFile(filename, lkupTag, lkupAtt, guid);
}

// Test case for gap in numbering
void TestUSDataFiles::test_gap_in_numbering()
{
    bool newFile;
    QString guid = "67891";
    QString lfchar = "M";
    QString lkupTag = "model";
    QString lkupAtt = "guid";

    // Remove the first file to create a gap
    QFile::remove(path + QDir::separator() + "M0000001.xml");

    // Test case 4: Should fill the gap in numbering
    QString filename = US_DataFiles::get_filename(path, guid, lfchar, lkupTag, lkupAtt, newFile);
    QCOMPARE(filename, path + QDir::separator() + "M0000001.xml"); // Should fill the gap
    QVERIFY(newFile);
}
