// test_us_sim_inputs.cpp
#include "qt_test_base.h"
#include "us_sim_inputs.h"
#include "us_astfem_math.h"
#include "us_constants.h"
#include <QTemporaryDir>

class US_SimInputsTest : public QtTestBase {
protected:
    void SetUp() override {
        QtTestBase::SetUp();
    }

    US_SimulationParameters build(const US_SimInputs::Params& params) {
        US_SimulationParameters result;
        QString error;
        EXPECT_TRUE(US_SimInputs::simParams(params, result, error))
            << error.toStdString();
        return result;
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

    US_SimulationParameters sp = build(p);

    // 45000 rpm / 400 rpm/s = 112.5 s = 1.875 min.
    ASSERT_EQ(sp.speed_step.count(), 1);
    EXPECT_EQ(sp.speed_step[0].delay_hours, 0);
    EXPECT_DOUBLE_EQ(sp.speed_step[0].delay_minutes, 1.875);
}

TEST_F(US_SimInputsTest, DerivedDelaySplitsIntoHoursAndMinutes) {
    US_SimInputs::Params p;
    p.rpm          = 60000.0;
    p.acceleration = 10.0;

    US_SimulationParameters sp = build(p);

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

    US_SimulationParameters sp = build(p);

    ASSERT_EQ(sp.speed_step.count(), 1);
    EXPECT_EQ(sp.speed_step[0].delay_hours, 1);
    EXPECT_DOUBLE_EQ(sp.speed_step[0].delay_minutes, 15.0);
}

TEST_F(US_SimInputsTest, ZeroAccelerationIsRejectedAtTheBuilderBoundary) {
    US_SimInputs::Params p;
    p.acceleration = 0.0;
    US_SimulationParameters sp;
    QString error;

    EXPECT_FALSE(US_SimInputs::simParams(p, sp, error));
    EXPECT_TRUE(error.contains("acceleration"));
}

TEST_F(US_SimInputsTest, AccelerationRoundingToZeroIsRejected) {
    // The speed profile stores whole rpm/s, so a value under half an rpm/s
    // would be stored as zero and stretch the ramp out forever.
    US_SimInputs::Params p;
    p.acceleration = 0.4;
    US_SimulationParameters sp;
    QString error;

    EXPECT_FALSE(US_SimInputs::simParams(p, sp, error));
    EXPECT_TRUE(error.contains("acceleration"));
}

TEST_F(US_SimInputsTest, ExplicitDelayReachingTheEndOfTheRunIsRejected) {
    // Default duration is 2 h 30 min; a 3 h delay leaves no time to scan in.
    US_SimInputs::Params p;
    p.delay_hours   = 3;
    p.delay_minutes = 0.0;
    US_SimulationParameters sp;
    QString error;

    EXPECT_FALSE(US_SimInputs::simParams(p, sp, error));
    EXPECT_TRUE(error.contains("delay"));
}

TEST_F(US_SimInputsTest, DerivedDelayReachingTheEndOfTheRunIsRejected) {
    // 60000 rpm at 5 rpm/s takes 200 min to reach speed, past the 150 min run.
    US_SimInputs::Params p;
    p.rpm          = 60000.0;
    p.acceleration = 5.0;
    US_SimulationParameters sp;
    QString error;

    EXPECT_FALSE(US_SimInputs::simParams(p, sp, error));
    EXPECT_TRUE(error.contains("delay"));
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

TEST_F(US_SimInputsTest, BuilderRejectsInvalidParamsWithoutPartialOutput) {
    US_SimInputs::Params invalid;
    invalid.centerpiece = 9999;
    US_SimulationParameters output;
    output.simpoints = 12345;
    QString error;

    EXPECT_FALSE(US_SimInputs::simParams(invalid, output, error));
    EXPECT_FALSE(error.isEmpty());
    EXPECT_EQ(output.simpoints, 12345);

    invalid = US_SimInputs::Params();
    invalid.scans = 0;
    EXPECT_FALSE(US_SimInputs::simParams(invalid, output, error));
    EXPECT_TRUE(error.contains("at least 1"));
}

TEST_F(US_SimInputsTest, BuilderAcceptsAOneScanRun) {
    US_SimInputs::Params params;
    params.scans = 1;

    US_SimulationParameters result = build(params);
    ASSERT_EQ(result.speed_step.size(), 1);
    EXPECT_EQ(result.speed_step[0].scans, 1);
}

TEST_F(US_SimInputsTest, NonzeroCalibrationIsAppliedExactlyOnceAfterRoundTrip) {
    US_SimInputs::Params params;
    params.rotor_calibr = "1";

    US_SimulationParameters generated = build(params);
    ASSERT_NE(generated.rotorcoeffs[0], 0.0);
    EXPECT_DOUBLE_EQ(generated.meniscus, 5.8);
    EXPECT_DOUBLE_EQ(generated.bottom, generated.bottom_position);

    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    QString file = dir.filePath("sp_nonzero.xml");
    ASSERT_EQ(generated.save_simparms(file), 0);

    US_SimulationParameters loaded;
    ASSERT_EQ(loaded.load_simparms(file), 0);
    // This mirrors both headless simulation consumers: the format does not
    // serialize bottom_position, so they recover it from the at-rest bottom.
    loaded.bottom_position = loaded.bottom;

    double speed = loaded.speed_step[0].rotorspeed;
    double stretch = loaded.rotorcoeffs[0] * speed
                   + loaded.rotorcoeffs[1] * speed * speed;
    double current_bottom = loaded.bottom_position + stretch;
    double current_meniscus = loaded.meniscus + stretch;

    EXPECT_NEAR(current_bottom,
        US_AstfemMath::calc_bottom(speed, loaded.bottom_position,
                                  loaded.rotorcoeffs), 1.0e-12);
    EXPECT_NEAR(current_meniscus, 5.8 + stretch, 1.0e-12);
    // Calibration 1 stretches by about 0.017 cm at the default speed. A
    // double application would put both radii another stretch farther out.
    EXPECT_GT(stretch, 0.01);
    EXPECT_LT(stretch, 0.03);
}
