// test_us_sim_inputs.cpp
#include "qt_test_base.h"
#include "us_sim_inputs.h"
#include "us_constants.h"
#include <QTemporaryDir>

class US_SimInputsTest : public QtTestBase {
protected:
    void SetUp() override {
        QtTestBase::SetUp();
    }
};

TEST_F(US_SimInputsTest, BufferMatchesWaterAt20C) {
    US_Buffer buffer = US_SimInputs::buffer();

    EXPECT_FALSE(buffer.description.isEmpty());
    EXPECT_FALSE(buffer.GUID.isEmpty());
    EXPECT_DOUBLE_EQ(buffer.density, DENS_20W);
    EXPECT_DOUBLE_EQ(buffer.viscosity, VISC_20W);
}

TEST_F(US_SimInputsTest, SimParamsMatchesDocumentedDefaults) {
    US_SimulationParameters sp = US_SimInputs::simParams();

    ASSERT_EQ(sp.speed_step.count(), 1);
    EXPECT_EQ(sp.speed_step[0].rotorspeed, 45000);
    EXPECT_EQ(sp.speed_step[0].scans, 30);
    EXPECT_EQ(sp.speed_step[0].duration_hours, 2);
    EXPECT_DOUBLE_EQ(sp.speed_step[0].duration_minutes, 30.0);
    EXPECT_EQ(sp.meshType, US_SimulationParameters::ASTFEM);
    EXPECT_EQ(sp.gridType, US_SimulationParameters::MOVING);
    EXPECT_FALSE(sp.band_forming);
}

TEST_F(US_SimInputsTest, ParamsDefaultsTrackSimulationParameters) {
    // Shared defaults must track their source types.
    US_SimInputs::Params p;
    US_SimulationParameters sp_defaults;
    US_SimulationParameters::SpeedProfile step_defaults;

    EXPECT_EQ(p.simpoints, sp_defaults.simpoints);
    EXPECT_DOUBLE_EQ(p.radial_resolution, sp_defaults.radial_resolution);
    EXPECT_EQ(p.meshType, sp_defaults.meshType);
    EXPECT_EQ(p.gridType, sp_defaults.gridType);
    EXPECT_DOUBLE_EQ(p.band_volume, sp_defaults.band_volume);
    EXPECT_EQ(p.band_forming, sp_defaults.band_forming);
    EXPECT_EQ(p.rotor_calibr, sp_defaults.rotorCalID);
    EXPECT_DOUBLE_EQ(p.acceleration, step_defaults.acceleration);
}

TEST_F(US_SimInputsTest, DelayDefaultsToTimeToReachSpeed) {
    US_SimInputs::Params p;
    p.rpm          = 45000.0;
    p.acceleration = 400.0;

    US_SimulationParameters sp = US_SimInputs::simParams(p);

    // 45000 rpm / 400 rpm/s = 112.5 s = 1.875 min.
    ASSERT_EQ(sp.speed_step.count(), 1);
    EXPECT_EQ(sp.speed_step[0].delay_hours, 0);
    EXPECT_DOUBLE_EQ(sp.speed_step[0].delay_minutes, 1.875);
}

TEST_F(US_SimInputsTest, DerivedDelaySplitsIntoHoursAndMinutes) {
    US_SimInputs::Params p;
    p.rpm          = 60000.0;
    p.acceleration = 10.0;

    US_SimulationParameters sp = US_SimInputs::simParams(p);

    // 60000 / 10 = 100 minutes, stored as 1 hour 40 minutes.
    ASSERT_EQ(sp.speed_step.count(), 1);
    EXPECT_EQ(sp.speed_step[0].delay_hours, 1);
    EXPECT_DOUBLE_EQ(sp.speed_step[0].delay_minutes, 40.0);
    EXPECT_LT(sp.speed_step[0].delay_minutes, 60.0);
}

TEST_F(US_SimInputsTest, ExplicitDelayOverridesDerivedValue) {
    US_SimInputs::Params p;
    p.delay_hours   = 1;
    p.delay_minutes = 15.0;

    US_SimulationParameters sp = US_SimInputs::simParams(p);

    ASSERT_EQ(sp.speed_step.count(), 1);
    EXPECT_EQ(sp.speed_step[0].delay_hours, 1);
    EXPECT_DOUBLE_EQ(sp.speed_step[0].delay_minutes, 15.0);
}

TEST_F(US_SimInputsTest, ZeroAccelerationDoesNotDivideByZero) {
    US_SimInputs::Params p;
    p.acceleration = 0.0;

    US_SimulationParameters sp = US_SimInputs::simParams(p);

    ASSERT_EQ(sp.speed_step.count(), 1);
    EXPECT_EQ(sp.speed_step[0].delay_hours, 0);
    EXPECT_DOUBLE_EQ(sp.speed_step[0].delay_minutes, 0.0);
}

TEST_F(US_SimInputsTest, WriteAllProducesReadableTriple) {
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());

    EXPECT_TRUE(US_SimInputs::writeAll(dir.path()));

    QFileInfo sp_file(dir.filePath("sp_default.xml"));
    QFileInfo model_file(dir.filePath("model_default.xml"));
    QFileInfo buffer_file(dir.filePath("buffer_default.xml"));

    EXPECT_TRUE(sp_file.exists());
    EXPECT_GT(sp_file.size(), 0);
    EXPECT_TRUE(model_file.exists());
    EXPECT_GT(model_file.size(), 0);
    EXPECT_TRUE(buffer_file.exists());
    EXPECT_GT(buffer_file.size(), 0);

    // The generated files should load into new objects.
    US_SimulationParameters sp_loaded;
    EXPECT_EQ(sp_loaded.load_simparms(sp_file.filePath()), 0);
    EXPECT_EQ(sp_loaded.speed_step.count(), 1);

    US_Model model_loaded;
    EXPECT_EQ(model_loaded.load(model_file.filePath()), IUS_DB2::OK);
    EXPECT_EQ(model_loaded.components.count(), 1);
}

TEST_F(US_SimInputsTest, WriteAllFailsForNonexistentDirectory) {
    EXPECT_FALSE(US_SimInputs::writeAll("/nonexistent/path/that/should/not/exist"));
}
