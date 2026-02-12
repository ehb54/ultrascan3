#include "qt_test_base.h"
#include <gmock/gmock-matchers.h>
#include "us_simparms.h"
#include "mock_us_db2.h"
#include <QTemporaryDir>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QFile>
#include <QDir>

using ::testing::_;
using ::testing::An;
using ::testing::Return;
using ::testing::SetArgReferee;
using ::testing::DoAll;
using ::testing::NiceMock;
using ::testing::StrictMock;

class US_SimulationParametersTest : public QtTestBase {
protected:
    void SetUp() override {
        QtTestBase::SetUp();
        simparms = std::make_unique<US_SimulationParameters>();
        mockDb = std::make_unique<NiceMock<US_DB2_Mock>>();
        tempDir = std::make_unique<QTemporaryDir>();
        ASSERT_TRUE(tempDir->isValid());
    }

    void TearDown() override {
        simparms.reset();
        mockDb.reset();
        tempDir.reset();
        QtTestBase::TearDown();
    }

    std::unique_ptr<US_SimulationParameters> simparms;
    std::unique_ptr<NiceMock<US_DB2_Mock>> mockDb;
    std::unique_ptr<QTemporaryDir> tempDir;

    // Helper to create test RawData
    US_DataIO::RawData createTestRawData() {
        US_DataIO::RawData rawData;
        rawData.type[0] = '\0';  // Clear the type array by setting first char to null terminator
        rawData.channel = 'A';
        rawData.cell = 1;
        rawData.description = "Test data";

        // Add some test scans
        US_DataIO::Scan scan;
        scan.seconds = 0.0;
        scan.omega2t = 0.0;
        scan.rpm = 3000.0;
        scan.temperature = 20.0;
        rawData.scanData << scan;

        scan.seconds = 3600.0;
        scan.omega2t = 3600.0 * 3000.0 * 3000.0 * 4.0 * M_PI * M_PI / (60.0 * 60.0);
        rawData.scanData << scan;

        return rawData;
    }

    // Helper to create test EditedData
    US_DataIO::EditedData createTestEditedData() {
        US_DataIO::EditedData editData;
        editData.runID = "test_run";
        editData.dataType = "RA";
        editData.channel = "A";
        editData.cell = "1";
        editData.meniscus = 5.8;
        editData.bottom = 7.2;

        // Add test scans
        US_DataIO::Scan scan;
        scan.seconds = 0.0;
        scan.omega2t = 0.0;
        scan.rpm = 3000.0;
        editData.scanData << scan;

        scan.seconds = 3600.0;
        scan.omega2t = 3600.0 * 3000.0 * 3000.0 * 4.0 * M_PI * M_PI / (60.0 * 60.0);
        editData.scanData << scan;

        return editData;
    }

    // Helper to create test XML content
    QString createTestSimParmsXml() {
        return QString(
                "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                "<!DOCTYPE US_SimParams>\n"
                "<SimParams version=\"1.0\">\n"
                "<params meshType=\"ASTFEM\" gridType=\"Moving\" simpoints=\"200\" "
                "radialres=\"0.001\" meniscus=\"5.8\" bottom=\"7.2\" "
                "rnoise=\"0.0\" lrnoise=\"0.0\" tinoise=\"0.0\" rinoise=\"0.0\" "
                "temperature=\"20.0\" bandform=\"0\" "
                "sector=\"0\" pathlength=\"1.2\" angle=\"2.5\" width=\"0.0\">\n"
                "<speedstep rotorspeed=\"3000\" scans=\"50\" timefirst=\"0\" timelast=\"3600\" "
                "w2tfirst=\"0.0\" w2tlast=\"3600000.0\" duration_hrs=\"1\" duration_mins=\"0\" "
                "delay_hrs=\"0\" delay_mins=\"0\" acceleration=\"400\" accelerflag=\"1\" />\n"
                "</params>\n"
                "</SimParams>\n"
        );
    }

    // Helper to write test file
    QString writeTestFile(const QString& content, const QString& filename = "test_simparms.xml") {
        QString fullPath = tempDir->path() + "/" + filename;
        QFile file(fullPath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            file.write(content.toUtf8());
            file.close();
        }
        return fullPath;
    }
};

// Constructor Tests
TEST_F(US_SimulationParametersTest, Constructor_InitializesCorrectly) {
    EXPECT_TRUE(simparms->mesh_radius.isEmpty());
    EXPECT_EQ(simparms->speed_step.size(), 1);
    EXPECT_EQ(simparms->sim_speed_prof.size(), 1);
    EXPECT_EQ(simparms->simpoints, 200);
    EXPECT_EQ(simparms->meshType, US_SimulationParameters::ASTFEM);
    EXPECT_EQ(simparms->gridType, US_SimulationParameters::MOVING);
    EXPECT_EQ(simparms->radial_resolution, 0.001);
    EXPECT_EQ(simparms->meniscus, 5.8);
    EXPECT_EQ(simparms->bottom, 7.2);
    EXPECT_EQ(simparms->rnoise, 0.0);
    EXPECT_EQ(simparms->lrnoise, 0.0);
    EXPECT_EQ(simparms->tinoise, 0.0);
    EXPECT_EQ(simparms->rinoise, 0.0);
    EXPECT_EQ(simparms->temperature, 20.0);
    EXPECT_EQ(simparms->rotorCalID, "0");
    EXPECT_FALSE(simparms->band_forming);
    EXPECT_EQ(simparms->band_volume, 0.015);
    EXPECT_EQ(simparms->bottom_position, 7.2);
    EXPECT_EQ(simparms->rotorcoeffs[0], 0.0);
    EXPECT_EQ(simparms->rotorcoeffs[1], 0.0);
    EXPECT_EQ(simparms->cp_sector, 0);
    EXPECT_EQ(simparms->cp_pathlen, 1.2);
    EXPECT_EQ(simparms->cp_angle, 2.5);
    EXPECT_EQ(simparms->cp_width, 0.0);
    EXPECT_FALSE(simparms->sim);
    EXPECT_EQ(simparms->tsobj, nullptr);
    EXPECT_FALSE(simparms->firstScanIsConcentration);
}

// SpeedProfile Constructor Tests
TEST_F(US_SimulationParametersTest, SpeedProfile_Constructor_InitializesCorrectly) {
    US_SimulationParameters::SpeedProfile sp;

    EXPECT_EQ(sp.duration_hours, 0);
    EXPECT_EQ(sp.duration_minutes, 0.0);
    EXPECT_EQ(sp.delay_hours, 0);
    EXPECT_EQ(sp.delay_minutes, 0.0);
    EXPECT_EQ(sp.scans, 0);
    EXPECT_EQ(sp.rotorspeed, 0);
    EXPECT_EQ(sp.acceleration, 400);
    EXPECT_TRUE(sp.acceleration_flag);
    EXPECT_EQ(sp.w2t_first, 0.0);
    EXPECT_EQ(sp.w2t_last, 0.0);
    EXPECT_EQ(sp.time_first, 0);
    EXPECT_EQ(sp.time_last, 0);
    EXPECT_EQ(sp.avg_speed, 0.0);
    EXPECT_EQ(sp.speed_stddev, 0.0);
    EXPECT_EQ(sp.set_speed, 0);
}

// SimSpeedProf Constructor Tests
TEST_F(US_SimulationParametersTest, SimSpeedProf_Constructor_InitializesCorrectly) {
    US_SimulationParameters::SimSpeedProf ssp;

    EXPECT_EQ(ssp.acceleration, 400.0);
    EXPECT_TRUE(ssp.rpm_timestate.isEmpty());
    EXPECT_TRUE(ssp.w2t_timestate.isEmpty());
    EXPECT_EQ(ssp.w2t_b_accel, 0.0);
    EXPECT_EQ(ssp.w2t_e_accel, 0.0);
    EXPECT_EQ(ssp.w2t_e_step, 0.0);
    EXPECT_EQ(ssp.avg_speed, 0.0);
    EXPECT_EQ(ssp.rotorspeed, 0);
    EXPECT_EQ(ssp.duration, 0);
    EXPECT_EQ(ssp.time_b_accel, 0);
    EXPECT_EQ(ssp.time_e_accel, 0);
    EXPECT_EQ(ssp.time_f_scan, 0);
    EXPECT_EQ(ssp.time_l_scan, 0);
}

// Debug Method Tests
TEST_F(US_SimulationParametersTest, Debug_DoesNotCrash) {
    // Add some test data
    simparms->simpoints = 100;
    simparms->temperature = 25.0;

    EXPECT_NO_THROW(simparms->debug());
}

// SetHardware Tests
TEST_F(US_SimulationParametersTest, SetHardware_WithoutDatabase_SetsDefaults) {
    QString testCalID = "test_cal_123";
    int testCp = 1;
    int testCh = 0;

    simparms->setHardware(testCalID, testCp, testCh);

    EXPECT_EQ(simparms->rotorCalID, testCalID);
}

TEST_F(US_SimulationParametersTest, SetHardware_WithDatabase_CallsCorrectly) {
    QString testCalID = "test_cal_456";
    int testCp = 2;

    EXPECT_NO_THROW(simparms->setHardware(mockDb.get(), testCalID, testCp));
    EXPECT_EQ(simparms->rotorCalID, testCalID);
}

TEST_F(US_SimulationParametersTest, SetHardware_NegativeCp_HandlesSerialNumber) {
    QString testCalID = "test_cal_789";
    int testSerialNumber = -12345;  // Negative indicates serial number
    int testCh = 0;

    EXPECT_NO_THROW(simparms->setHardware(testCalID, testSerialNumber, testCh));
}

// Load SimParms Tests
TEST_F(US_SimulationParametersTest, LoadSimparms_ValidFile_LoadsCorrectly) {
    QString xmlContent = createTestSimParmsXml();
    QString filename = writeTestFile(xmlContent);

    int result = simparms->load_simparms(filename);

    EXPECT_EQ(result, 0);
    EXPECT_EQ(simparms->meshType, US_SimulationParameters::ASTFEM);
    EXPECT_EQ(simparms->gridType, US_SimulationParameters::MOVING);
    EXPECT_EQ(simparms->simpoints, 200);
    EXPECT_EQ(simparms->radial_resolution, 0.001);
    EXPECT_EQ(simparms->meniscus, 5.8);
    EXPECT_EQ(simparms->bottom, 7.2);
    EXPECT_EQ(simparms->temperature, 20.0);
    EXPECT_FALSE(simparms->band_forming);
    EXPECT_EQ(simparms->speed_step.size(), 1);
    EXPECT_EQ(simparms->speed_step[0].rotorspeed, 3000);
}

TEST_F(US_SimulationParametersTest, LoadSimparms_NonExistentFile_ReturnsError) {
    QString nonExistentFile = tempDir->path() + "/non_existent.xml";

    int result = simparms->load_simparms(nonExistentFile);

    EXPECT_EQ(result, -1);
}

// Save SimParms Tests
TEST_F(US_SimulationParametersTest, SaveSimparms_ValidParameters_SavesCorrectly) {
    simparms->meshType = US_SimulationParameters::CLAVERIE;
    simparms->gridType = US_SimulationParameters::FIXED;
    simparms->simpoints = 150;
    simparms->temperature = 25.0;
    simparms->band_forming = true;
    simparms->band_volume = 0.02;

    QString filename = tempDir->path() + "/test_save.xml";

    int result = simparms->save_simparms(filename);

    EXPECT_EQ(result, 0);
    EXPECT_TRUE(QFile::exists(filename));

    // Verify content by loading it back
    US_SimulationParameters loadedParams;
    int loadResult = loadedParams.load_simparms(filename);
    EXPECT_EQ(loadResult, 0);
    EXPECT_EQ(loadedParams.meshType, US_SimulationParameters::CLAVERIE);
    EXPECT_EQ(loadedParams.gridType, US_SimulationParameters::FIXED);
    EXPECT_EQ(loadedParams.simpoints, 150);
    EXPECT_EQ(loadedParams.temperature, 25.0);
    EXPECT_TRUE(loadedParams.band_forming);
    EXPECT_EQ(loadedParams.band_volume, 0.02);
}

// Compute Speed Steps Tests
TEST_F(US_SimulationParametersTest, ComputeSpeedSteps_SingleSpeedScans_CreatesOneStep) {
    QVector<US_DataIO::Scan> scans;

    // Create scans with constant speed
    for (int i = 0; i < 10; ++i) {
        US_DataIO::Scan scan;
        scan.seconds = i * 360.0; // 6-minute intervals
        scan.rpm = 3000.0;
        scan.omega2t = scan.seconds * scan.rpm * scan.rpm * 4.0 * M_PI * M_PI / (60.0 * 60.0);
        scans << scan;
    }

    QVector<US_SimulationParameters::SpeedProfile> speedSteps;
    US_SimulationParameters::computeSpeedSteps(&scans, speedSteps);

    EXPECT_EQ(speedSteps.size(), 1);
    EXPECT_EQ(speedSteps[0].rotorspeed, 3000);
    EXPECT_EQ(speedSteps[0].scans, 10);
    EXPECT_GT(speedSteps[0].avg_speed, 2999.0);
    EXPECT_LT(speedSteps[0].avg_speed, 3001.0);
}

// InitFromData Tests
TEST_F(US_SimulationParametersTest, InitFromData_RawData_SetsBasicParameters) {
    US_DataIO::RawData rawData = createTestRawData();
    QString runID = "test_run";
    QString dataType = "RA";

    EXPECT_NO_THROW(simparms->initFromData(nullptr, rawData, true, runID, dataType));

    // Basic parameters should be set
    EXPECT_EQ(simparms->rotorCalID, "0");
    EXPECT_EQ(simparms->bottom_position, 7.2);
}

TEST_F(US_SimulationParametersTest, InitFromData_EditedData_SetsBasicParameters) {
    US_DataIO::EditedData editData = createTestEditedData();

    EXPECT_NO_THROW(simparms->initFromData(nullptr, editData, true));

    EXPECT_EQ(simparms->meniscus, 5.8);
    EXPECT_EQ(simparms->bottom_position, 7.2);
}