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

TEST_F(US_SimInputsTest, ModelIsSingleComponentAbsorbanceManual) {
    US_Model model = US_SimInputs::model();

    EXPECT_FALSE(model.description.isEmpty());
    EXPECT_FALSE(model.modelGUID.isEmpty());
    EXPECT_EQ(model.optics, US_Model::ABSORBANCE);
    EXPECT_EQ(model.analysis, US_Model::MANUAL);
    ASSERT_EQ(model.components.count(), 1);
}

TEST_F(US_SimInputsTest, ModelUsesLibraryComponentDefaults) {
    US_Model model = US_SimInputs::model();
    const US_Model::SimulationComponent& sc = model.components[0];

    EXPECT_DOUBLE_EQ(sc.mw, 50000.0);
    EXPECT_DOUBLE_EQ(sc.f_f0, 1.25);
    EXPECT_DOUBLE_EQ(sc.vbar20, TYPICAL_VBAR);
}

TEST_F(US_SimInputsTest, ModelCoefficientsAreComputed) {
    US_Model model = US_SimInputs::model();
    const US_Model::SimulationComponent& sc = model.components[0];

    // update_coefficients() should calculate s and D from mw, vbar20, and f_f0.
    EXPECT_GT(sc.s, 0.0);
    EXPECT_GT(sc.D, 0.0);
}

TEST_F(US_SimInputsTest, TwoCallsToModelProduceDistinctGuids) {
    US_Model model1 = US_SimInputs::model();
    US_Model model2 = US_SimInputs::model();

    EXPECT_NE(model1.modelGUID, model2.modelGUID);
}

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
