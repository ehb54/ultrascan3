#include "test_us_project.h"
#include "us_project.h"
#include "us_db2.h"

void TestUSProject::initTestCase()
{
    // Setup code that needs to run before any tests are executed
}

void TestUSProject::cleanupTestCase()
{
    // Cleanup code that needs to run after all tests are done
}

void TestUSProject::init()
{
    // Code that runs before each test function
}

void TestUSProject::cleanup()
{
    // Clean up any XML files created during the test
    QString path;
    US_Project project;
    project.diskPath(path);

    QDir dir(path);
    QStringList filter("P*.xml");
    QStringList files = dir.entryList(filter, QDir::Files);

    for (const QString &file : files) {
        dir.remove(file);
    }
}


void TestUSProject::testClear()
{
    // Arrange
    US_Project project;
    project.projectID = 123;
    project.goals = "Test Goal";

    // Act
    project.clear();

    // Assert
    QCOMPARE(project.projectID, 0);
    QVERIFY(project.goals.isEmpty());
    QCOMPARE(project.saveStatus, US_Project::NOT_SAVED);
}

void TestUSProject::testSaveToDisk()
{
    // Arrange
    US_Project project;
    project.projectID = 123;
    project.projectGUID = "123e4567-e89b-12d3-a456-426614174000";  // Use a valid GUID
    project.goals = "Test Goal";
    project.molecules = "Test Molecule";
    project.purity = "99%";
    project.expense = "High";
    project.bufferComponents = "Buffer A";
    project.saltInformation = "NaCl";
    project.AUC_questions = "None";
    project.expDesign = "Test Design";
    project.notes = "Test Notes";
    project.projectDesc = "Test Description";

    // Act
    project.saveToDisk();

    // Assert
    QString path;
    project.diskPath(path);
    QString expectedFile = path + "/P0000001.xml";  // Adjust as necessary
    QFile file(expectedFile);
    QVERIFY(file.exists());

    // Additional validation: check file content
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    QString fileContent = file.readAll();
    file.close();

    QVERIFY(fileContent.contains("123e4567-e89b-12d3-a456-426614174000"));  // Ensure correct GUID is saved
}



void TestUSProject::testReadFromDisk()
{
    // Arrange
    testSaveToDisk();  // Ensure the file is created

    US_Project project;
    QString guid = "123e4567-e89b-12d3-a456-426614174000";  // Use the same valid GUID
    QString path;
    project.diskPath(path);
    QString expectedFile = path + "/P0000001.xml";  // Adjust as needed

    // Ensure the file exists before attempting to read
    QFile file(expectedFile);
    QVERIFY(file.exists());

    // Act
    int result = project.readFromDisk(guid);

    // Assert
    QCOMPARE(result, US_DB2::OK);
    QCOMPARE(project.projectGUID, guid);
    QCOMPARE(project.projectID, 123);
    QCOMPARE(project.goals, QString("Test Goal"));
    QCOMPARE(project.molecules, QString("Test Molecule"));
    QCOMPARE(project.purity, QString("99%"));
    QCOMPARE(project.expense, QString("High"));
    QCOMPARE(project.bufferComponents, QString("Buffer A"));
    QCOMPARE(project.saltInformation, QString("NaCl"));
    QCOMPARE(project.AUC_questions, QString("None"));
    QCOMPARE(project.expDesign, QString("Test Design"));
    QCOMPARE(project.notes, QString("Test Notes"));
    QCOMPARE(project.projectDesc, QString("Test Description"));

    // Assert on the default or initial values
    QCOMPARE(project.status, QString("submitted"));  // Assuming this is the default status
    QCOMPARE(project.saveStatus, US_Project::HD_ONLY);  // Assuming the status after saving to disk
}

void TestUSProject::testDeleteFromDisk()
{
    // Arrange
    US_Project project;
    project.projectGUID = "test-guid";

    // Act
    project.deleteFromDisk();

    // Assert
    QString path = US_Settings::dataDir() + "/projects";
    QString expectedFile = path + "/P0000001.xml";
    QFile file(expectedFile);
    QVERIFY(!file.exists());
}
