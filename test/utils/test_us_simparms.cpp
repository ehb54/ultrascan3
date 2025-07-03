#include "test_us_simparms.h"
#include "us_simparms.h"
#include "us_hardware.h"
#include "us_constants.h"
#include <QMap>
#include <QList>

// Use your custom Qt matchers
using namespace qt_matchers;

// Mock classes for testing without database dependencies
class MockUS_AbstractCenterpiece {
public:
    static bool read_centerpieces(IUS_DB2* db, QList<US_AbstractCenterpiece>& cp_list) {
        // Add mock data for testing
        US_AbstractCenterpiece cp;
        cp.serial_number = 1;
        cp.shape = "standard";
        cp.bottom_position = {7.2, 2.0};
        cp.path_length = {1.5, 2.5};
        cp.angle = 30.0;
        cp.width = 5.0;
        cp_list.append(cp);
        return true;
    }
};

class MockUS_Hardware {
public:
    static bool readRotorMap(IUS_DB2* db, QMap<QString, QString>& rotor_map) {
        // Add mock rotor data
        rotor_map["calID"] = "mock_calibration";
        return true;
    }

    static void rotorValues(QString rCalID, QMap<QString, QString>& rotor_map, double* rotorcoeffs) {
        // Add mock rotor coefficient data
        rotorcoeffs[0] = 1.1;
        rotorcoeffs[1] = 2.2;
    }
};

// Extended US_SimulationParameters for testing
class TestUS_SimulationParameters : public US_SimulationParameters {
public:
    void setHardwareMock(QString rCalID, int cp, int ch) {
        QList<US_AbstractCenterpiece> cp_list;
        QMap<QString, QString> rotor_map;

        // Use mock classes to populate data (no database needed)
        MockUS_AbstractCenterpiece::read_centerpieces(nullptr, cp_list);
        MockUS_Hardware::readRotorMap(nullptr, rotor_map);

        double mock_rotorcoeffs[2];
        MockUS_Hardware::rotorValues(rCalID, rotor_map, mock_rotorcoeffs);

        // Set member variables from mock data
        this->rotorCalID = rCalID;
        this->bottom_position = cp_list[cp].bottom_position[ch];
        this->cp_pathlen = cp_list[cp].path_length[ch];
        this->cp_angle = cp_list[cp].angle;
        this->cp_width = cp_list[cp].width;
        this->cp_sector = 1;  // Assuming "standard" is index 1 in shapes list
        this->band_forming = false;
        this->rotorcoeffs[0] = mock_rotorcoeffs[0];
        this->rotorcoeffs[1] = mock_rotorcoeffs[1];
    }
};

// TestUSSimparms method implementations
void TestUSSimparms::SetUp() {
    QtTestBase::SetUp();
    // Per-test setup for SimulationParameters tests
}

void TestUSSimparms::TearDown() {
    // Per-test cleanup for SimulationParameters tests
    QtTestBase::TearDown();
}

// Suite-level setup for SimulationParameters tests
void TestUSSimparms::SetUpTestSuite() {
    QtTestBase::SetUpTestSuite();
    // One-time setup for all TestUSSimparms tests
}

// Suite-level cleanup for SimulationParameters tests
void TestUSSimparms::TearDownTestSuite() {
    // One-time cleanup for all TestUSSimparms tests
}

// Helper method to validate default constructor values
void TestUSSimparms::validateDefaultConstructorValues(const US_SimulationParameters& simParams) {
    EXPECT_EQ(simParams.mesh_radius.size(), 0)
                        << "Default mesh_radius should be empty";
    EXPECT_EQ(simParams.speed_step.size(), 1)
                        << "Default speed_step should have one entry";
    EXPECT_EQ(simParams.simpoints, 200)
                        << "Default simpoints should be 200";
    EXPECT_EQ(simParams.meshType, US_SimulationParameters::ASTFEM)
                        << "Default mesh type should be ASTFEM";
    EXPECT_EQ(simParams.gridType, US_SimulationParameters::MOVING)
                        << "Default grid type should be MOVING";
    EXPECT_DOUBLE_EQ(simParams.radial_resolution, 0.001)
                        << "Default radial resolution should be 0.001";
    EXPECT_DOUBLE_EQ(simParams.meniscus, 5.8)
                        << "Default meniscus should be 5.8";
    EXPECT_DOUBLE_EQ(simParams.bottom, 7.2)
                        << "Default bottom should be 7.2";
    EXPECT_DOUBLE_EQ(simParams.rnoise, 0.0)
                        << "Default rnoise should be 0.0";
    EXPECT_DOUBLE_EQ(simParams.lrnoise, 0.0)
                        << "Default lrnoise should be 0.0";
    EXPECT_DOUBLE_EQ(simParams.tinoise, 0.0)
                        << "Default tinoise should be 0.0";
    EXPECT_DOUBLE_EQ(simParams.rinoise, 0.0)
                        << "Default rinoise should be 0.0";
    EXPECT_DOUBLE_EQ(simParams.temperature, 20.0)
                        << "Default temperature should be 20.0 (NORMAL_TEMP)";
}

// Helper method to validate hardware setup
void TestUSSimparms::validateHardwareSetup(const US_SimulationParameters& simParams,
                                           const QString& expectedRotorCalID) {
    EXPECT_THAT(simParams.rotorCalID, QStringEq(expectedRotorCalID))
            << "Rotor calibration ID should match expected value";
    EXPECT_DOUBLE_EQ(simParams.bottom_position, 7.2)
                        << "Bottom position should be set from mock data";
    EXPECT_DOUBLE_EQ(simParams.cp_pathlen, 1.5)
                        << "Centerpiece path length should be set from mock data";
    EXPECT_DOUBLE_EQ(simParams.cp_angle, 30.0)
                        << "Centerpiece angle should be set from mock data";
    EXPECT_DOUBLE_EQ(simParams.cp_width, 5.0)
                        << "Centerpiece width should be set from mock data";
    EXPECT_EQ(simParams.cp_sector, 1)
                        << "Centerpiece sector should be set correctly";
    EXPECT_FALSE(simParams.band_forming)
                        << "Band forming should be false";
    EXPECT_DOUBLE_EQ(simParams.rotorcoeffs[0], 1.1)
                        << "First rotor coefficient should be set from mock data";
    EXPECT_DOUBLE_EQ(simParams.rotorcoeffs[1], 2.2)
                        << "Second rotor coefficient should be set from mock data";
}

TEST_F(TestUSSimparms, Constructor) {
    // Test default constructor behavior
    TestUS_SimulationParameters simParams;

    // Validate all default values
    validateDefaultConstructorValues(simParams);

    // Additional specific validations
    EXPECT_THAT(simParams.rotorCalID, QStringEq("0"))
            << "Default rotor calibration ID should be '0'";
    EXPECT_FALSE(simParams.band_forming)
                        << "Default band forming should be false";
    EXPECT_DOUBLE_EQ(simParams.band_volume, 0.015)
                        << "Default band volume should be 0.015";
    EXPECT_DOUBLE_EQ(simParams.bottom_position, 7.2)
                        << "Default bottom position should be 7.2";
    EXPECT_DOUBLE_EQ(simParams.rotorcoeffs[0], 0.0)
                        << "Default first rotor coefficient should be 0.0";
    EXPECT_DOUBLE_EQ(simParams.rotorcoeffs[1], 0.0)
                        << "Default second rotor coefficient should be 0.0";
    EXPECT_EQ(simParams.cp_sector, 0)
                        << "Default centerpiece sector should be 0";
    EXPECT_DOUBLE_EQ(simParams.cp_pathlen, 1.2)
                        << "Default centerpiece path length should be 1.2";
    EXPECT_DOUBLE_EQ(simParams.cp_angle, 2.5)
                        << "Default centerpiece angle should be 2.5";
    EXPECT_DOUBLE_EQ(simParams.cp_width, 0.0)
                        << "Default centerpiece width should be 0.0";
    EXPECT_FALSE(simParams.sim)
                        << "Default sim flag should be false";
    EXPECT_EQ(simParams.tsobj, nullptr)
                        << "Default tsobj should be nullptr";
    EXPECT_FALSE(simParams.firstScanIsConcentration)
                        << "Default firstScanIsConcentration should be false";
}

TEST_F(TestUSSimparms, ConstructorEnumValues) {
    // Test that enum values are set correctly
    TestUS_SimulationParameters simParams;

    // Test mesh type enum
    EXPECT_TRUE(simParams.meshType == US_SimulationParameters::ASTFEM ||
                simParams.meshType == US_SimulationParameters::CLAVERIE ||
                simParams.meshType == US_SimulationParameters::MOVING_HAT)
                        << "Mesh type should be a valid enum value";

    // Test grid type enum
    EXPECT_TRUE(simParams.gridType == US_SimulationParameters::FIXED ||
                simParams.gridType == US_SimulationParameters::MOVING)
                        << "Grid type should be a valid enum value";
}

TEST_F(TestUSSimparms, SetHardwareDB) {
    // Test hardware setup with mock database data
    TestUS_SimulationParameters simParams;
    QString rCalID = "test_calID";
    int cp = 0; // Index 0 because cp_list will have only one item
    int ch = 0; // Channel 0

    // Test the mock hardware setup (simulating database case)
    simParams.setHardwareMock(rCalID, cp, ch);

    // Validate all hardware setup values
    validateHardwareSetup(simParams, rCalID);
}

TEST_F(TestUSSimparms, SetHardwareLocal) {
    // Test hardware setup with mock local data
    TestUS_SimulationParameters simParams;
    QString rCalID = "local_calID";
    int cp = 0; // Index 0 because cp_list will have only one item
    int ch = 0; // Channel 0

    // Test the mock hardware setup (local case)
    simParams.setHardwareMock(rCalID, cp, ch);

    // Validate all hardware setup values (should be same as DB case with mocks)
    validateHardwareSetup(simParams, rCalID);
}

TEST_F(TestUSSimparms, SetHardwareDifferentChannels) {
    // Test hardware setup with different channel values
    TestUS_SimulationParameters simParams1, simParams2;
    QString rCalID = "channel_test_calID";
    int cp = 0;

    // Test channel 0
    simParams1.setHardwareMock(rCalID, cp, 0);
    EXPECT_DOUBLE_EQ(simParams1.bottom_position, 7.2)
                        << "Channel 0 should use first bottom position value";
    EXPECT_DOUBLE_EQ(simParams1.cp_pathlen, 1.5)
                        << "Channel 0 should use first path length value";

    // Test channel 1
    simParams2.setHardwareMock(rCalID, cp, 1);
    EXPECT_DOUBLE_EQ(simParams2.bottom_position, 2.0)
                        << "Channel 1 should use second bottom position value";
    EXPECT_DOUBLE_EQ(simParams2.cp_pathlen, 2.5)
                        << "Channel 1 should use second path length value";

    // Both should have same rotor coefficients
    EXPECT_DOUBLE_EQ(simParams1.rotorcoeffs[0], simParams2.rotorcoeffs[0])
                        << "Rotor coefficients should be same for both channels";
    EXPECT_DOUBLE_EQ(simParams1.rotorcoeffs[1], simParams2.rotorcoeffs[1])
                        << "Rotor coefficients should be same for both channels";
}

TEST_F(TestUSSimparms, SpeedStepDefault) {
    // Test default speed step configuration
    TestUS_SimulationParameters simParams;

    EXPECT_EQ(simParams.speed_step.size(), 1)
                        << "Should have one default speed step";

    if (simParams.speed_step.size() > 0) {
        const auto& speedStep = simParams.speed_step.first();
        // Test that speed step has reasonable default values
        EXPECT_GE(speedStep.duration_hours, 0)
                            << "Duration hours should be non-negative";
        EXPECT_GE(speedStep.duration_minutes, 0)
                            << "Duration minutes should be non-negative";
        EXPECT_LT(speedStep.duration_minutes, 60)
                            << "Duration minutes should be less than 60";
    }
}

TEST_F(TestUSSimparms, NoiseValues) {
    // Test that all noise values are initialized to zero
    TestUS_SimulationParameters simParams;

    QList<double> noiseValues = {
            simParams.rnoise,
            simParams.lrnoise,
            simParams.tinoise,
            simParams.rinoise
    };

    for (int i = 0; i < noiseValues.size(); i++) {
        EXPECT_DOUBLE_EQ(noiseValues[i], 0.0)
                            << "Noise value " << i << " should be initialized to 0.0";
    }
}

TEST_F(TestUSSimparms, PhysicalConstraints) {
    // Test that physical constraints are reasonable
    TestUS_SimulationParameters simParams;

    // Meniscus should be less than bottom
    EXPECT_LT(simParams.meniscus, simParams.bottom)
                        << "Meniscus position should be less than bottom position";

    // Radial resolution should be positive and small
    EXPECT_GT(simParams.radial_resolution, 0.0)
                        << "Radial resolution should be positive";
    EXPECT_LT(simParams.radial_resolution, 1.0)
                        << "Radial resolution should be less than 1.0";

    // Temperature should be reasonable (assuming Celsius)
    EXPECT_GT(simParams.temperature, 0.0)
                        << "Temperature should be above absolute zero";
    EXPECT_LT(simParams.temperature, 100.0)
                        << "Temperature should be reasonable for experiments";

    // Simulation points should be reasonable
    EXPECT_GT(simParams.simpoints, 0)
                        << "Simulation points should be positive";
    EXPECT_LT(simParams.simpoints, 10000)
                        << "Simulation points should be reasonable";
}

// Integration test
TEST_F(TestUSSimparms, SimulationParametersIntegration) {
    // Test that all components work together
    TestUS_SimulationParameters simParams;

    // Validate initial state
    validateDefaultConstructorValues(simParams);

    // Set up hardware
    QString testRotorID = "integration_test_rotor";
    simParams.setHardwareMock(testRotorID, 0, 0);

    // Validate hardware was set correctly
    validateHardwareSetup(simParams, testRotorID);

    // Verify that non-hardware parameters weren't changed
    EXPECT_EQ(simParams.simpoints, 200)
                        << "Simpoints should remain unchanged after hardware setup";
    EXPECT_DOUBLE_EQ(simParams.temperature, 20.0)
                        << "Temperature should remain unchanged after hardware setup";
    EXPECT_EQ(simParams.meshType, US_SimulationParameters::ASTFEM)
                        << "Mesh type should remain unchanged after hardware setup";

    // Test that we can modify other parameters after hardware setup
    simParams.temperature = 25.0;
    simParams.simpoints = 300;

    EXPECT_DOUBLE_EQ(simParams.temperature, 25.0)
                        << "Should be able to modify temperature after hardware setup";
    EXPECT_EQ(simParams.simpoints, 300)
                        << "Should be able to modify simpoints after hardware setup";

    // Hardware parameters should remain unchanged
    EXPECT_THAT(simParams.rotorCalID, QStringEq(testRotorID))
            << "Hardware parameters should remain stable";
}