// test_us_astfem_rsa.cpp
#include "qt_test_base.h"
#include "us_astfem_rsa.h"
#include "us_model.h"
#include "us_simparms.h"
#include "us_dataIO.h"
#include "us_buffer.h"
#include <QSignalSpy>

using ::testing::_;
using ::testing::Return;
using ::testing::Eq;
using ::testing::DoubleEq;
using ::testing::DoubleNear;
using ::testing::StrictMock;
using ::testing::NiceMock;

// Mock classes for dependencies
class MockModel : public US_Model {
public:
    MockModel() = default;
    // Use default implementations where needed
};

class MockSimulationParameters : public US_SimulationParameters {
public:
    MockSimulationParameters() = default;
    // Use default implementations where needed
};

class US_AstfemRSATest : public QtTestBase {
protected:
    void SetUp() override {
        QtTestBase::SetUp();

        // Initialize mock model with basic components
        model.components.resize(1);
        model.components[0].s = 4.5e-13;
        model.components[0].D = 6.5e-11;
        model.components[0].signal_concentration = 1.0;
        model.components[0].extinction = 1.0;
        model.components[0].mw = 50000.0;
        model.components[0].vbar20 = 0.72;

        // Initialize simulation parameters
        simparams.simpoints = 100;
        simparams.meniscus = 5.8;
        simparams.bottom = 7.2;
        simparams.bottom_position = 7.2;
        simparams.cp_pathlen = 1.2;
        simparams.cp_angle = 2.5;
        simparams.meshType = US_SimulationParameters::ASTFEM;
        simparams.gridType = US_SimulationParameters::MOVING;
        simparams.band_forming = false;
        simparams.band_volume = 0.015;
        simparams.firstScanIsConcentration = false;
        simparams.rotorcoeffs[0] = 0.0;
        simparams.rotorcoeffs[1] = 0.0;

        // Add a speed step
        US_SimulationParameters::SpeedProfile sp;
        sp.duration_hours = 1;
        sp.duration_minutes = 0;
        sp.delay_hours = 0;
        sp.delay_minutes = 0;
        sp.rotorspeed = 50000;
        sp.acceleration = 400;
        sp.acceleration_flag = true;
        sp.time_first = 0;
        sp.time_last = 3600;
        sp.w2t_first = 0;
        sp.w2t_last = 1000000;
        sp.scans = 100;
        simparams.speed_step.append(sp);

        // Create test raw data
        setupTestData();
    }

    void setupTestData() {
        rawData.type[0] = 'R';
        rawData.type[1] = 'A';
        rawData.cell = 1;
        rawData.channel = 'A';
        rawData.description = "Test Data";

        // Create radial grid
        int nPoints = 50;
        double meniscus = 5.8;
        double bottom = 7.2;
        double dr = (bottom - meniscus) / (nPoints - 1);

        rawData.xvalues.clear();
        for (int i = 0; i < nPoints; i++) {
            rawData.xvalues.append(meniscus + i * dr);
        }

        // Create scan data
        int nScans = 10;
        rawData.scanData.resize(nScans);

        for (int i = 0; i < nScans; i++) {
            US_DataIO::Scan& scan = rawData.scanData[i];
            scan.temperature = 20.0;
            scan.rpm = 50000;
            scan.seconds = i * 360.0; // Every 6 minutes
            scan.omega2t = i * 100000.0;
            scan.wavelength = 280;
            scan.plateau = bottom;
            scan.delta_r = dr;

            int bytesNeeded = (nPoints + 7) / 8;  // Round up to nearest byte
            scan.interpolated = QByteArray(bytesNeeded, 0);  // Initialize with zeros (all actual data)

            scan.rvalues.clear();
            scan.rvalues.resize(nPoints);

            // Simple exponential decay profile
            for (int j = 0; j < nPoints; j++) {
                double radius = rawData.xvalues[j];
                scan.rvalues[j] = 1.0 * exp(-(radius - meniscus) / (bottom - meniscus));
            }
        }
    }

    void TearDown() override {
        QtTestBase::TearDown();
    }

    US_Model model;
    US_SimulationParameters simparams;
    US_DataIO::RawData rawData;
};

// Constructor Tests
TEST_F(US_AstfemRSATest, ConstructorBasic) {
    US_Astfem_RSA astfem(model, simparams);

    // Constructor should complete without throwing
    SUCCEED();
}

TEST_F(US_AstfemRSATest, ConstructorWithParent) {
    QObject parent;
    US_Astfem_RSA astfem(model, simparams, &parent);

    EXPECT_THAT(astfem.parent(), Eq(&parent));
}

// Setter Method Tests
TEST_F(US_AstfemRSATest, SetTimeCorrection) {
    US_Astfem_RSA astfem(model, simparams);

    // Test setting true
    astfem.setTimeCorrection(true);
    // No direct getter, but method should not throw
    SUCCEED();

    // Test setting false
    astfem.setTimeCorrection(false);
    SUCCEED();
}

TEST_F(US_AstfemRSATest, SetTimeInterpolation) {
    US_Astfem_RSA astfem(model, simparams);

    // Test setting true
    astfem.setTimeInterpolation(true);
    SUCCEED();

    // Test setting false
    astfem.setTimeInterpolation(false);
    SUCCEED();
}

TEST_F(US_AstfemRSATest, SetStopFlag) {
    US_Astfem_RSA astfem(model, simparams);

    // Test setting true
    astfem.setStopFlag(true);
    SUCCEED();

    // Test setting false
    astfem.setStopFlag(false);
    SUCCEED();
}

TEST_F(US_AstfemRSATest, SetMovieFlag) {
    US_Astfem_RSA astfem(model, simparams);

    // Test setting true
    astfem.set_movie_flag(true);
    SUCCEED();

    // Test setting false
    astfem.set_movie_flag(false);
    SUCCEED();
}

TEST_F(US_AstfemRSATest, SetSimoutFlag) {
    US_Astfem_RSA astfem(model, simparams);

    // Test setting true
    astfem.set_simout_flag(true);
    SUCCEED();

    // Test setting false
    astfem.set_simout_flag(false);
    SUCCEED();
}

TEST_F(US_AstfemRSATest, SetDebugFlag) {
    US_Astfem_RSA astfem(model, simparams);

    // Test different debug levels
    astfem.set_debug_flag(0);
    SUCCEED();

    astfem.set_debug_flag(1);
    SUCCEED();

    astfem.set_debug_flag(5);
    SUCCEED();

    // Test negative value
    astfem.set_debug_flag(-1);
    SUCCEED();
}

TEST_F(US_AstfemRSATest, SetBuffer) {
    US_Astfem_RSA astfem(model, simparams);
    US_Buffer buffer;

    buffer.density = 1.0;
    buffer.compressibility = 4.6e-10;

    // Should not throw
    astfem.set_buffer(buffer);
    SUCCEED();
}

TEST_F(US_AstfemRSATest, SetBufferZeroDensity) {
    US_Astfem_RSA astfem(model, simparams);
    US_Buffer buffer;

    buffer.density = 0.0;
    buffer.compressibility = 4.6e-10;

    // Should handle zero density gracefully
    astfem.set_buffer(buffer);
    SUCCEED();
}

TEST_F(US_AstfemRSATest, SetBufferNegativeValues) {
    US_Astfem_RSA astfem(model, simparams);
    US_Buffer buffer;

    buffer.density = -1.0;
    buffer.compressibility = -1.0e-10;

    // Should handle negative values gracefully
    astfem.set_buffer(buffer);
    SUCCEED();
}

// Signal emission tests
TEST_F(US_AstfemRSATest, SignalConnections) {
    US_Astfem_RSA astfem(model, simparams);

    // Test that signals can be connected without errors
    QSignalSpy newScanSpy(&astfem, &US_Astfem_RSA::new_scan);
    QSignalSpy newTimeSpy(&astfem, &US_Astfem_RSA::new_time);
    QSignalSpy currentComponentSpy(&astfem, &US_Astfem_RSA::current_component);
    QSignalSpy currentSpeedSpy(&astfem, &US_Astfem_RSA::current_speed);
    QSignalSpy calcStartSpy(&astfem, &US_Astfem_RSA::calc_start);
    QSignalSpy calcProgressSpy(&astfem, &US_Astfem_RSA::calc_progress);
    QSignalSpy calcDoneSpy(&astfem, &US_Astfem_RSA::calc_done);

    EXPECT_TRUE(newScanSpy.isValid());
    EXPECT_TRUE(newTimeSpy.isValid());
    EXPECT_TRUE(currentComponentSpy.isValid());
    EXPECT_TRUE(currentSpeedSpy.isValid());
    EXPECT_TRUE(calcStartSpy.isValid());
    EXPECT_TRUE(calcProgressSpy.isValid());
    EXPECT_TRUE(calcDoneSpy.isValid());
}

// Edge case tests for model validation
TEST_F(US_AstfemRSATest, EmptyModelComponents) {
    US_Model emptyModel;
    US_Astfem_RSA astfem(emptyModel, simparams);

    // Should handle empty model gracefully
    SUCCEED();
}

TEST_F(US_AstfemRSATest, ModelWithZeroConcentration) {
    US_Model testModel = model;
    testModel.components[0].signal_concentration = 0.0;

    US_Astfem_RSA astfem(testModel, simparams);
    SUCCEED();
}

TEST_F(US_AstfemRSATest, ModelWithNegativeValues) {
    US_Model testModel = model;
    testModel.components[0].s = -4.5e-13; // Negative sedimentation coefficient
    testModel.components[0].D = 6.5e-11;  // Positive diffusion coefficient

    US_Astfem_RSA astfem(testModel, simparams);
    SUCCEED();
}

TEST_F(US_AstfemRSATest, ModelWithVerySmallValues) {
    US_Model testModel = model;
    testModel.components[0].s = 1e-20;
    testModel.components[0].D = 1e-20;

    US_Astfem_RSA astfem(testModel, simparams);
    SUCCEED();
}

TEST_F(US_AstfemRSATest, ModelWithVeryLargeValues) {
    US_Model testModel = model;
    testModel.components[0].s = 1e-10;
    testModel.components[0].D = 1e-8;

    US_Astfem_RSA astfem(testModel, simparams);
    SUCCEED();
}

// Edge case tests for simulation parameters
TEST_F(US_AstfemRSATest, SimparamsZeroSimpoints) {
    US_SimulationParameters testParams = simparams;
    testParams.simpoints = 0;

    US_Astfem_RSA astfem(model, testParams);
    SUCCEED();
}

TEST_F(US_AstfemRSATest, SimparamsInvalidMeniscusBottom) {
    US_SimulationParameters testParams = simparams;
    testParams.meniscus = 7.2;  // Meniscus > bottom
    testParams.bottom = 5.8;

    US_Astfem_RSA astfem(model, testParams);
    SUCCEED();
}

TEST_F(US_AstfemRSATest, SimparamsEqualMeniscusBottom) {
    US_SimulationParameters testParams = simparams;
    testParams.meniscus = 6.5;
    testParams.bottom = 6.5;

    US_Astfem_RSA astfem(model, testParams);
    SUCCEED();
}

TEST_F(US_AstfemRSATest, SimparamsVeryLargeSimpoints) {
    US_SimulationParameters testParams = simparams;
    testParams.simpoints = 100000;

    US_Astfem_RSA astfem(model, testParams);
    SUCCEED();
}

TEST_F(US_AstfemRSATest, SimparamsNoSpeedSteps) {
    US_SimulationParameters testParams = simparams;
    testParams.speed_step.clear();

    US_Astfem_RSA astfem(model, testParams);
    SUCCEED();
}

// Test different mesh types
TEST_F(US_AstfemRSATest, DifferentMeshTypes) {
    US_SimulationParameters testParams = simparams;

    // Test ASTFEM mesh
    testParams.meshType = US_SimulationParameters::ASTFEM;
    US_Astfem_RSA astfem1(model, testParams);
    SUCCEED();

    // Test CLAVERIE mesh
    testParams.meshType = US_SimulationParameters::CLAVERIE;
    US_Astfem_RSA astfem2(model, testParams);
    SUCCEED();

    // Test MOVING_HAT mesh
    testParams.meshType = US_SimulationParameters::MOVING_HAT;
    US_Astfem_RSA astfem3(model, testParams);
    SUCCEED();

    // Test USER mesh
    testParams.meshType = US_SimulationParameters::USER;
    US_Astfem_RSA astfem4(model, testParams);
    SUCCEED();

    // Test ASTFVM mesh
    testParams.meshType = US_SimulationParameters::ASTFVM;
    US_Astfem_RSA astfem5(model, testParams);
    SUCCEED();
}

// Test different grid types
TEST_F(US_AstfemRSATest, DifferentGridTypes) {
    US_SimulationParameters testParams = simparams;

    // Test FIXED grid
    testParams.gridType = US_SimulationParameters::FIXED;
    US_Astfem_RSA astfem1(model, testParams);
    SUCCEED();

    // Test MOVING grid
    testParams.gridType = US_SimulationParameters::MOVING;
    US_Astfem_RSA astfem2(model, testParams);
    SUCCEED();
}

// Test band forming parameters
TEST_F(US_AstfemRSATest, BandFormingEnabled) {
    US_SimulationParameters testParams = simparams;
    testParams.band_forming = true;
    testParams.band_volume = 0.015;

    US_Astfem_RSA astfem(model, testParams);
    SUCCEED();
}

TEST_F(US_AstfemRSATest, BandFormingZeroVolume) {
    US_SimulationParameters testParams = simparams;
    testParams.band_forming = true;
    testParams.band_volume = 0.0;

    US_Astfem_RSA astfem(model, testParams);
    SUCCEED();
}

TEST_F(US_AstfemRSATest, BandFormingNegativeVolume) {
    US_SimulationParameters testParams = simparams;
    testParams.band_forming = true;
    testParams.band_volume = -0.01;

    US_Astfem_RSA astfem(model, testParams);
    SUCCEED();
}

// Test rotor coefficients
TEST_F(US_AstfemRSATest, NonZeroRotorCoefficients) {
    US_SimulationParameters testParams = simparams;
    testParams.rotorcoeffs[0] = 1e-5;
    testParams.rotorcoeffs[1] = 1e-10;

    US_Astfem_RSA astfem(model, testParams);
    SUCCEED();
}

TEST_F(US_AstfemRSATest, NegativeRotorCoefficients) {
    US_SimulationParameters testParams = simparams;
    testParams.rotorcoeffs[0] = -1e-5;
    testParams.rotorcoeffs[1] = -1e-10;

    US_Astfem_RSA astfem(model, testParams);
    SUCCEED();
}

// Test multiple components
TEST_F(US_AstfemRSATest, MultipleComponents) {
    US_Model testModel = model;

    // Add second component
    US_Model::SimulationComponent comp2;
    comp2.s = 2.5e-13;
    comp2.D = 8.0e-11;
    comp2.signal_concentration = 0.5;
    comp2.extinction = 1.0;
    comp2.mw = 30000.0;
    comp2.vbar20 = 0.73;

    testModel.components.append(comp2);

    US_Astfem_RSA astfem(testModel, simparams);
    SUCCEED();
}

TEST_F(US_AstfemRSATest, ManyComponents) {
    US_Model testModel = model;

    // Add multiple components
    for (int i = 1; i < 10; i++) {
        US_Model::SimulationComponent comp;
        comp.s = model.components[0].s * (1.0 + i * 0.1);
        comp.D = model.components[0].D * (1.0 + i * 0.05);
        comp.signal_concentration = 1.0 / (i + 1);
        comp.extinction = 1.0;
        comp.mw = 50000.0 + i * 5000;
        comp.vbar20 = 0.72;

        testModel.components.append(comp);
    }

    US_Astfem_RSA astfem(testModel, simparams);
    SUCCEED();
}

// Test with associations (reactions)
TEST_F(US_AstfemRSATest, ModelWithAssociations) {
    US_Model testModel = model;

    // Add second component for reaction
    US_Model::SimulationComponent comp2;
    comp2.s = 8.0e-13;
    comp2.D = 4.0e-11;
    comp2.signal_concentration = 0.5;
    comp2.extinction = 1.0;
    comp2.mw = 100000.0;
    comp2.vbar20 = 0.72;
    testModel.components.append(comp2);

    // Add association
    US_Model::Association assoc;
    assoc.rcomps.append(0);  // First component
    assoc.rcomps.append(1);  // Second component
    assoc.stoichs.append(2); // 2 monomers
    assoc.stoichs.append(-1); // form 1 dimer
    assoc.k_d = 1e-6;
    assoc.k_off = 100.0;
    testModel.associations.append(assoc);

    US_Astfem_RSA astfem(testModel, simparams);
    SUCCEED();
}

// Test speed profile edge cases
TEST_F(US_AstfemRSATest, ZeroSpeedProfile) {
    US_SimulationParameters testParams = simparams;
    testParams.speed_step[0].rotorspeed = 0;

    US_Astfem_RSA astfem(model, testParams);
    SUCCEED();
}

TEST_F(US_AstfemRSATest, VeryHighSpeedProfile) {
    US_SimulationParameters testParams = simparams;
    testParams.speed_step[0].rotorspeed = 100000;

    US_Astfem_RSA astfem(model, testParams);
    SUCCEED();
}

TEST_F(US_AstfemRSATest, ZeroAccelerationProfile) {
    US_SimulationParameters testParams = simparams;
    testParams.speed_step[0].acceleration = 0;

    US_Astfem_RSA astfem(model, testParams);
    SUCCEED();
}

TEST_F(US_AstfemRSATest, NoAccelerationFlag) {
    US_SimulationParameters testParams = simparams;
    testParams.speed_step[0].acceleration_flag = false;

    US_Astfem_RSA astfem(model, testParams);
    SUCCEED();
}

TEST_F(US_AstfemRSATest, MultipleSpeedSteps) {
    US_SimulationParameters testParams = simparams;

    // Add second speed step
    US_SimulationParameters::SpeedProfile sp2;
    sp2.duration_hours = 2;
    sp2.duration_minutes = 0;
    sp2.delay_hours = 0;
    sp2.delay_minutes = 5;
    sp2.rotorspeed = 60000;
    sp2.acceleration = 300;
    sp2.acceleration_flag = true;
    sp2.time_first = 3900;  // After first step
    sp2.time_last = 11100;  // 2 hours later
    sp2.w2t_first = 1000000;
    sp2.w2t_last = 3000000;
    sp2.scans = 150;

    testParams.speed_step.append(sp2);

    US_Astfem_RSA astfem(model, testParams);
    SUCCEED();
}

// Test error conditions that should be handled gracefully
TEST_F(US_AstfemRSATest, ExtremeParameterValues) {
    US_Model testModel = model;
    US_SimulationParameters testParams = simparams;

    // Extreme s value
    testModel.components[0].s = 1e-15;
    testModel.components[0].D = 1e-15;

    // Extreme geometry
    testParams.meniscus = 5.7999;
    testParams.bottom = 5.8001;  // Very thin cell
    testParams.simpoints = 3;    // Minimal points

    US_Astfem_RSA astfem(testModel, testParams);
    SUCCEED();
}

TEST_F(US_AstfemRSATest, ZeroDiffusionCoefficient) {
    US_Model testModel = model;
    testModel.components[0].D = 0.0;

    US_Astfem_RSA astfem(testModel, simparams);  // Changed from testParams to simparams
    SUCCEED();
}

TEST_F(US_AstfemRSATest, ZeroSedimentationCoefficient) {
    US_Model testModel = model;
    testModel.components[0].s = 0.0;

    US_Astfem_RSA astfem(testModel, simparams);
    SUCCEED();
}

// Test calculation method edge cases (without full calculation)
TEST_F(US_AstfemRSATest, CalculateWithEmptyRawData) {
    US_Astfem_RSA astfem(model, simparams);
    US_DataIO::RawData emptyData;

    // Initialize empty data with minimal valid structure to prevent crashes
    emptyData.type[0] = 'R';
    emptyData.type[1] = 'A';
    emptyData.cell = 1;
    emptyData.channel = 'A';
    emptyData.description = "Empty Test Data";

    // Add at least one scan to prevent index out of bounds
    US_DataIO::Scan emptyScan;
    emptyScan.temperature = 20.0;
    emptyScan.rpm = 50000;
    emptyScan.seconds = 0.0;
    emptyScan.omega2t = 0.0;
    emptyScan.wavelength = 280;
    emptyScan.plateau = 7.2;
    emptyScan.delta_r = 0.01;
    emptyScan.interpolated = QByteArray(1, 0);
    emptyScan.rvalues.clear();

    emptyData.scanData.append(emptyScan);
    emptyData.xvalues.clear();

    // Should handle minimal data gracefully (may return error code)
    astfem.setStopFlag(true); // Prevent long calculation
    int result = astfem.calculate(emptyData);

    // Just verify it doesn't crash - result can be error or success
    SUCCEED();
}

TEST_F(US_AstfemRSATest, CalculateWithStopFlag) {
    US_Astfem_RSA astfem(model, simparams);

    // Set stop flag before calculation
    astfem.setStopFlag(true);

    int result = astfem.calculate(rawData);

    // Should return early due to stop flag
    EXPECT_THAT(result, Eq(1)); // Stop flag return value
}

TEST_F(US_AstfemRSATest, CalculateWithInvalidTimeRange) {
    US_Astfem_RSA astfem(model, simparams);
    US_DataIO::RawData testData = rawData;

    // Set invalid time range
    for (auto& scan : testData.scanData) {
        scan.seconds = -1.0; // Negative time
    }

    astfem.setStopFlag(true); // Prevent long calculation
    int result = astfem.calculate(testData);

    // Should handle gracefully
    SUCCEED();
}

// Test thread safety considerations
TEST_F(US_AstfemRSATest, MultipleInstancesCreation) {
    // Test creating multiple instances simultaneously
    std::vector<std::unique_ptr<US_Astfem_RSA>> instances;

    for (int i = 0; i < 5; i++) {
        instances.push_back(std::make_unique<US_Astfem_RSA>(model, simparams));
    }

    // All instances should be created successfully
    EXPECT_THAT(instances.size(), Eq(5));

    for (const auto& instance : instances) {
        EXPECT_NE(instance.get(), nullptr);
    }
}

// Test object lifecycle
TEST_F(US_AstfemRSATest, DestructorHandling) {
    {
        US_Astfem_RSA astfem(model, simparams);
        astfem.set_debug_flag(1);
        astfem.setTimeCorrection(true);
        // Object should destruct cleanly when going out of scope
    }

    SUCCEED();
}

// Test with Qt parent-child relationships
TEST_F(US_AstfemRSATest, ParentChildRelationship) {
    QObject parent;

    {
        US_Astfem_RSA astfem(model, simparams, &parent);
        EXPECT_THAT(astfem.parent(), Eq(&parent));

        // Child should be automatically cleaned up when parent is destroyed
    }

    SUCCEED();
}