#include "us3_file_test_base.h"
#include "us_project.h"
#include "mock_us_db2.h"
#include <QTemporaryDir>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QXmlStreamWriter>
#include <QDateTime>
#include <QRegularExpression>

using ::testing::_;
using ::testing::Return;
using ::testing::SetArgReferee;
using ::testing::DoAll;
using ::testing::NiceMock;
using ::testing::StrictMock;
using ::testing::InSequence;

class US_ProjectTest : public Us3FileTestBase {
protected:
    void SetUp() override {
        Us3FileTestBase::SetUp();
        project = std::make_unique<US_Project>();
        mockDb = std::make_unique<NiceMock<US_DB2_Mock>>();

        // Create temporary directory for testing
        tempDir = std::make_unique<QTemporaryDir>();
        ASSERT_TRUE(tempDir->isValid());
    }

    void TearDown() override {
        project.reset();
        mockDb.reset();
        tempDir.reset();
        Us3FileTestBase::TearDown();
    }

    std::unique_ptr<US_Project> project;
    std::unique_ptr<NiceMock<US_DB2_Mock>> mockDb;
    std::unique_ptr<QTemporaryDir> tempDir;

    // Helper to create a valid project XML file
    QString createTestProjectXml(const QString& guid = "test-guid-123", int id = 42) {
        return QString(
                "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                "<!DOCTYPE US_Project>\n"
                "<ProjectData version=\"1.0\">\n"
                "<project id=\"%1\" guid=\"%2\">\n"
                "<goals>Test research goals</goals>\n"
                "<molecules>Test proteins and DNA</molecules>\n"
                "<purity>95%</purity>\n"
                "<expense>Moderate expense</expense>\n"
                "<bufferComponents>Tris, NaCl, EDTA</bufferComponents>\n"
                "<saltInformation>150mM NaCl acceptable</saltInformation>\n"
                "<AUC_questions>What is the binding affinity?</AUC_questions>\n"
                "<expDesign>Sedimentation velocity experiments</expDesign>\n"
                "<notes>Handle with care</notes>\n"
                "<description>Test project description</description>\n"
                "</project>\n"
                "</ProjectData>\n"
        ).arg(id).arg(guid);
    }

    // Helper to write XML to temporary file
    QString writeTestFile(const QString& content, const QString& filename = "P0000001.xml") {
        QString fullPath = tempDir->path() + "/projects/" + filename;
        QDir().mkpath(tempDir->path() + "/projects");

        QFile file(fullPath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            file.write(content.toUtf8());
            file.close();
        }
        return fullPath;
    }

    QString projectsDir() {
        QString path;
        EXPECT_TRUE(US_Project().diskPath(path));
        return path;
    }

    // Writes content where readFromDisk() will find it.
    QString writeIntoProjectsDir(const QString& content,
                                 const QString& filename = "P0000001.xml") {
        const QString fullPath = projectsDir() + "/" + filename;
        QFile file(fullPath);
        EXPECT_TRUE(file.open(QIODevice::WriteOnly | QIODevice::Text));
        file.write(content.toUtf8());
        file.close();
        return fullPath;
    }

    // Helper to populate project with test data
    void populateTestProject() {
        project->projectID = 42;
        project->projectGUID = "test-guid-123";
        project->goals = "Test research goals";
        project->molecules = "Test proteins";
        project->purity = "95%";
        project->expense = "Moderate";
        project->bufferComponents = "Tris, NaCl";
        project->saltInformation = "150mM NaCl";
        project->AUC_questions = "Binding affinity?";
        project->expDesign = "SV experiments";
        project->notes = "Test notes";
        project->projectDesc = "Test description";
        project->status = "submitted";
    }
};

// Constructor Tests
TEST_F(US_ProjectTest, Constructor_InitializesCorrectly) {
EXPECT_EQ(project->projectID, 0);
EXPECT_TRUE(project->projectGUID.isEmpty());
EXPECT_TRUE(project->goals.isEmpty());
EXPECT_TRUE(project->molecules.isEmpty());
EXPECT_TRUE(project->purity.isEmpty());
EXPECT_TRUE(project->expense.isEmpty());
EXPECT_TRUE(project->bufferComponents.isEmpty());
EXPECT_TRUE(project->saltInformation.isEmpty());
EXPECT_TRUE(project->AUC_questions.isEmpty());
EXPECT_TRUE(project->expDesign.isEmpty());
EXPECT_TRUE(project->notes.isEmpty());
EXPECT_TRUE(project->projectDesc.isEmpty());
EXPECT_EQ(project->status, "submitted");
EXPECT_EQ(project->saveStatus, US_Project::NOT_SAVED);
}

// Clear Method Tests
TEST_F(US_ProjectTest, Clear_ResetsAllValues) {
populateTestProject();
project->saveStatus = US_Project::BOTH;

project->clear();

EXPECT_EQ(project->projectID, 0);
EXPECT_TRUE(project->projectGUID.isEmpty());
EXPECT_TRUE(project->goals.isEmpty());
EXPECT_TRUE(project->molecules.isEmpty());
EXPECT_TRUE(project->purity.isEmpty());
EXPECT_TRUE(project->expense.isEmpty());
EXPECT_TRUE(project->bufferComponents.isEmpty());
EXPECT_TRUE(project->saltInformation.isEmpty());
EXPECT_TRUE(project->AUC_questions.isEmpty());
EXPECT_TRUE(project->expDesign.isEmpty());
EXPECT_TRUE(project->notes.isEmpty());
EXPECT_TRUE(project->projectDesc.isEmpty());
EXPECT_EQ(project->status, "submitted");
EXPECT_EQ(project->saveStatus, US_Project::NOT_SAVED);
}

// DiskPath Tests
TEST_F(US_ProjectTest, DiskPath_ValidPath_ReturnsTrue) {
QString path;
bool result = project->diskPath(path);

EXPECT_TRUE(result);
EXPECT_FALSE(path.isEmpty());
EXPECT_TRUE(path.contains("projects"));
}

TEST_F(US_ProjectTest, DiskPath_CreatesDirectoryIfNeeded) {
QString path;
bool result = project->diskPath(path);

EXPECT_TRUE(result);
QDir dir(path);
EXPECT_TRUE(dir.exists());
}

// DiskFilename Tests
TEST_F(US_ProjectTest, DiskFilename_ExistingFile_FindsCorrectly) {
QString testGuid = "test-guid-456";
const QString written = writeIntoProjectsDir(createTestProjectXml(testGuid),
                                             "P0000042.xml");

QString filename;
EXPECT_TRUE(project->diskFilename(testGuid, filename));
EXPECT_EQ(filename, written);
}

TEST_F(US_ProjectTest, DiskFilename_NonExistentFile_ReturnsFalse) {
QString filename;
QString nonExistentGuid = "non-existent-guid";

bool found = project->diskFilename(nonExistentGuid, filename);

EXPECT_FALSE(found);
EXPECT_TRUE(filename.isEmpty());
}

// ReadFromDisk Tests
TEST_F(US_ProjectTest, ReadFromDisk_ValidFile_LoadsCorrectly) {
QString testGuid = "test-guid-789";
writeIntoProjectsDir(createTestProjectXml(testGuid, 99));

ASSERT_EQ(project->readFromDisk(testGuid), IUS_DB2::OK);

EXPECT_EQ(project->projectID, 99);
EXPECT_EQ(project->projectGUID, testGuid);
EXPECT_EQ(project->goals, "Test research goals");
EXPECT_EQ(project->molecules, "Test proteins and DNA");
EXPECT_EQ(project->purity, "95%");
EXPECT_EQ(project->expense, "Moderate expense");
EXPECT_EQ(project->bufferComponents, "Tris, NaCl, EDTA");
EXPECT_EQ(project->saltInformation, "150mM NaCl acceptable");
EXPECT_EQ(project->AUC_questions, "What is the binding affinity?");
EXPECT_EQ(project->expDesign, "Sedimentation velocity experiments");
EXPECT_EQ(project->notes, "Handle with care");
EXPECT_EQ(project->projectDesc, "Test project description");
}

TEST_F(US_ProjectTest, ReadFromDisk_NonExistentFile_ReturnsError) {
QString nonExistentGuid = "non-existent-guid";

int result = project->readFromDisk(nonExistentGuid);

EXPECT_EQ(result, IUS_DB2::NO_PROJECT);
}

TEST_F(US_ProjectTest, ReadFromDisk_InvalidXml_ReturnsError) {
writeIntoProjectsDir("This is not valid XML content", "P0000900.xml");

QString guid = "guid-only-in-the-unparseable-file";

// A file that is not XML exposes no guid attribute, so the guid search that
// precedes any parsing never matches it and the read reports NO_PROJECT.
EXPECT_EQ(project->readFromDisk(guid), IUS_DB2::NO_PROJECT);
EXPECT_EQ(project->projectID, 0);
}

// XML Parsing Edge Cases - Test via public methods
TEST_F(US_ProjectTest, ReadFromDisk_ReplacesPreviouslyLoadedState) {
populateTestProject();
project->notes = "stale notes that must not survive";

QString testGuid = "test-guid-xml-parse";
QString xml = createTestProjectXml(testGuid, 99);
xml.remove("<notes>Handle with care</notes>\n");
writeIntoProjectsDir(xml);

ASSERT_EQ(project->readFromDisk(testGuid), IUS_DB2::OK);

EXPECT_TRUE(project->notes.isEmpty());
EXPECT_EQ(project->projectID, 99);
}

// GUID Generation Tests
TEST_F(US_ProjectTest, SaveToDisk_NoGUID_GeneratesNewGUID) {
populateTestProject();
project->projectGUID = ""; // No GUID

project->saveToDisk();

// Should have generated a valid GUID
EXPECT_FALSE(project->projectGUID.isEmpty());
EXPECT_EQ(project->projectGUID.length(), 36); // Standard GUID length
EXPECT_TRUE(project->projectGUID.contains("-"));
}

TEST_F(US_ProjectTest, SaveToDisk_InvalidGUID_GeneratesNewGUID) {
populateTestProject();
project->projectGUID = "invalid-guid-format";

project->saveToDisk();

// Should have generated a new valid GUID
EXPECT_EQ(project->projectGUID.length(), 36);
EXPECT_NE(project->projectGUID, "invalid-guid-format");
}

TEST_F(US_ProjectTest, SaveToDisk_ValidGUID_KeepsGUID) {
populateTestProject();
QString validGuid = "12345678-1234-1234-1234-123456789012";
project->projectGUID = validGuid;

project->saveToDisk();

EXPECT_EQ(project->projectGUID, validGuid);
}

// SaveStatus Tests
TEST_F(US_ProjectTest, SaveToDisk_UpdatesSaveStatus_HDOnly) {
populateTestProject();
project->saveStatus = US_Project::NOT_SAVED;

project->saveToDisk();

EXPECT_EQ(project->saveStatus, US_Project::HD_ONLY);
}

TEST_F(US_ProjectTest, SaveToDisk_FromDBOnly_UpdatesToBoth) {
populateTestProject();
project->saveStatus = US_Project::DB_ONLY;

project->saveToDisk();

EXPECT_EQ(project->saveStatus, US_Project::BOTH);
}

// Edge Cases and Error Conditions
TEST_F(US_ProjectTest, ReadFromDisk_CorruptedXML_HandlesGracefully) {
QString guid = "test-guid";
writeIntoProjectsDir(
        "<?xml version=\"1.0\"?>\n"
        "<ProjectData>\n"
        "<project id=\"1\" guid=\"test-guid\">\n"
        "<goals>Test goals\n"   // never closed
        "</project>\n"
        "</ProjectData>\n");

project->projectID   = 77;
project->projectGUID = "the guid held before the failed read";
project->goals       = "the goals held before the failed read";

EXPECT_EQ(project->readFromDisk(guid), IUS_DB2::DBERROR);

EXPECT_EQ(project->projectID, 77);
EXPECT_EQ(project->projectGUID, "the guid held before the failed read");
EXPECT_EQ(project->goals, "the goals held before the failed read");
}

TEST_F(US_ProjectTest, SaveToDisk_NewProjectGetsASequentialGeneratedFilename) {
const QString dir = projectsDir();
ASSERT_TRUE(QDir(dir).entryList(QStringList("P???????.xml"), QDir::Files).isEmpty());

populateTestProject();
project->projectGUID = "12345678-1234-1234-1234-123456789012";
project->saveToDisk();

EXPECT_EQ(QDir(dir).entryList(QStringList("P???????.xml"), QDir::Files, QDir::Name),
          QStringList("P0000001.xml"));
}

TEST_F(US_ProjectTest, SaveToDisk_ExistingProjectReusesItsFileRatherThanAddingOne) {
const QString dir = projectsDir();
populateTestProject();
project->projectGUID = "12345678-1234-1234-1234-123456789012";

project->saveToDisk();
project->goals = "revised goals";
project->saveToDisk();

// The second save reuses the same file rather than allocating P0000002.xml.
EXPECT_EQ(QDir(dir).entryList(QStringList("P???????.xml"), QDir::Files, QDir::Name),
          QStringList("P0000001.xml"));

US_Project reloaded;
QString    guid = project->projectGUID;
ASSERT_EQ(reloaded.readFromDisk(guid), IUS_DB2::OK);
EXPECT_EQ(reloaded.goals, "revised goals");
}