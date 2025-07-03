// test_us_astfem_math.cpp
#include "test_us_astfem_math.h"
#include "us_astfem_math.h"
#include "us_settings.h"
#include "us_dataIO.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>

// Use your custom Qt matchers
using namespace qt_matchers;

// TestUSAstfemMath method implementations
void TestUSAstfemMath::SetUp() {
    QtTestBase::SetUp();
    // Per-test setup - initialize test file path for each test
    tmst_fpath = "test_astfem.tmst";  // Use unique filename
}

void TestUSAstfemMath::TearDown() {
    // Per-test cleanup - remove test file after each test
    QFile::remove(tmst_fpath);
    QtTestBase::TearDown();
}

// Suite-level setup for AstfemMath tests
void TestUSAstfemMath::SetUpTestSuite() {
    QtTestBase::SetUpTestSuite();
    // One-time setup for all TestUSAstfemMath tests
}

// Suite-level cleanup for AstfemMath tests
void TestUSAstfemMath::TearDownTestSuite() {
    // One-time cleanup for all TestUSAstfemMath tests
}

TEST_F(TestUSAstfemMath, Writetimestate) {
    // Create a valid file with expected content for testing
    QFile file(tmst_fpath);
    ASSERT_TRUE(file.open(QIODevice::WriteOnly | QIODevice::Text))
                                << "Failed to create the test file";

    QTextStream out(&file);
    out << "Sample content for timestate file\n";
    file.close();

    US_SimulationParameters simparams;
    US_DataIO::RawData sim_data;

    // Set up simulation parameters
    simparams.mesh_radius.clear();
    simparams.mesh_radius << 5.8 << 6.0 << 6.2;
    simparams.simpoints = 200;
    simparams.meshType = US_SimulationParameters::MeshType::ASTFEM;
    simparams.gridType = US_SimulationParameters::FIXED;
    simparams.radial_resolution = 0.001;
    simparams.meniscus = 5.8;
    simparams.bottom = 7.2;
    simparams.temperature = 20.0;
    simparams.band_forming = false;
    simparams.band_volume = 0.015;
    simparams.rotorCalID = "0";

    US_SimulationParameters::SpeedProfile speedProfile;
    speedProfile.duration_hours = 1.0;
    speedProfile.duration_minutes = 0;
    speedProfile.delay_hours = 0;
    speedProfile.delay_minutes = 0;
    speedProfile.scans = 100;
    speedProfile.acceleration = 400;

    simparams.speed_step << speedProfile; // Add the speed profile

    // Set up raw data
    sim_data.type[0] = 'R';  // Example type "RI"
    sim_data.type[1] = 'I';
    sim_data.xvalues << 280.0 << 290.0;  // Use xvalues instead of wavelengths

    US_DataIO::Scan scan1;
    scan1.seconds = 0.0;  // Set the time elapsed since the start of the run
    scan1.rpm = 50000;
    scan1.temperature = 20.0;  // Example temperature
    scan1.rvalues.clear();
    scan1.rvalues << 1.0 << 2.0 << 3.0;

    US_DataIO::Scan scan2;
    scan2.seconds = 1.0;  // Set the time elapsed since the start of the run
    scan2.rpm = 50000;
    scan2.temperature = 20.0;  // Example temperature
    scan2.rvalues.clear();
    scan2.rvalues << 1.5 << 2.5 << 3.5;

    sim_data.scanData << scan1 << scan2;
    sim_data.scanData << scan1 << scan2;

    // Act
    int result = US_AstfemMath::writetimestate(tmst_fpath, simparams, sim_data);

    // Debugging information
    qDebug() << "writetimestate() result:" << result;

    // Assert
    EXPECT_GT(result, 0) << "writetimestate should return a positive value";
    EXPECT_TRUE(QFile::exists(tmst_fpath)) << "Output file should exist";

    // Additional content verification
    QFile resultFile(tmst_fpath);
    ASSERT_TRUE(resultFile.open(QIODevice::ReadOnly | QIODevice::Text))
                                << "Should be able to read the output file";

    QTextStream in(&resultFile);
    QString content = in.readAll();
    resultFile.close();

    // Verify content is not empty
    EXPECT_FALSE(content.isEmpty()) << "Output file should not be empty";
}

TEST_F(TestUSAstfemMath, LowAcceleration) {
    QVector<US_SimulationParameters::SpeedProfile> speedsteps;

    // Setup speedsteps with valid data
    US_SimulationParameters::SpeedProfile speedProfile;
    speedProfile.duration_minutes = 30;
    speedProfile.delay_minutes = 15;
    speedProfile.w2t_first = 0.0;
    speedProfile.w2t_last = 1.0;
    speedProfile.avg_speed = 50000.0;
    speedProfile.speed_stddev = 0.0;
    speedProfile.duration_hours = 1;
    speedProfile.delay_hours = 0;
    speedProfile.time_first = 0;
    speedProfile.time_last = 5400;  // 1.5 hours in seconds
    speedProfile.scans = 100;
    speedProfile.rotorspeed = 50000;
    speedProfile.acceleration = 400;  // RPM/second
    speedProfile.set_speed = 50000;
    speedProfile.acceleration_flag = true;

    speedsteps << speedProfile;  // Add the speed profile to the vector

    double min_accel = 0.1;  // Minimum acceleration threshold
    double rate;

    // Defensive check before the actual test
    ASSERT_FALSE(speedsteps.isEmpty()) << "Speedsteps should not be empty";

    // Act
    bool result = US_AstfemMath::low_acceleration(speedsteps, min_accel, rate);

    // Assert
    EXPECT_TRUE(result) << "low_acceleration should return true for this test data";
}

TEST_F(TestUSAstfemMath, InterpolateC0) {
    // Arrange
    US_AstfemMath astfemMath;
    US_AstfemMath::MfemInitial C0, C1;

    // Set up C0 with valid test data - need to initialize all required fields
    C0.radius.clear();
    C0.radius << 5.8 << 6.0 << 6.2;

    // Initialize concentration data (assuming this is what the method needs)
    C0.concentration.clear();
    C0.concentration.resize(C0.radius.size());
    for (int i = 0; i < C0.radius.size(); ++i) {
        C0.concentration[i] = 1.0 + i * 0.1;  // Example concentration values
    }

    // Initialize C1 with different radius points (interpolation target)
    C1.radius.clear();
    C1.radius << 5.9 << 6.1 << 6.3;

    // Initialize C1 concentration vector to the correct size
    C1.concentration.clear();
    C1.concentration.resize(C1.radius.size());
    // Values will be filled by interpolation

    // Defensive checks before calling the method
    ASSERT_FALSE(C0.radius.isEmpty()) << "C0 radius should not be empty";
    ASSERT_FALSE(C1.radius.isEmpty()) << "C1 radius should not be empty";
    ASSERT_EQ(C0.radius.size(), C0.concentration.size())
                                << "C0 radius and concentration should have same size";

    // Act - call the interpolation method
    try {
        astfemMath.interpolate_C0(C0, C1);
    } catch (const std::exception& e) {
        FAIL() << "interpolate_C0 threw an exception: " << e.what();
    } catch (...) {
        FAIL() << "interpolate_C0 threw an unknown exception";
    }

    // Assert - validate the interpolation results
    EXPECT_EQ(C1.radius.size(), C1.concentration.size())
                        << "C1 radius and concentration should have same size after interpolation";

    // Check that concentration values were actually interpolated
    for (int i = 0; i < C1.concentration.size(); ++i) {
        EXPECT_GT(C1.concentration[i], 0.0)
                            << "Interpolated concentration should be positive at index " << i;
    }
}