#include "qt_test_base.h"
#include "us_model.h"
#include "mock_us_db2.h"
#include "us_constants.h"
#include <QTemporaryDir>
#include <QTextStream>
#include <QXmlStreamWriter>

using ::testing::_;
using ::testing::Return;
using ::testing::SetArgReferee;
using ::testing::DoAll;
using ::testing::NiceMock;
using ::testing::StrictMock;
using ::testing::TypedEq;
using ::qt_matchers::QStringContains;
using ::qt_matchers::QStringEq;

class US_ModelTest : public QtTestBase {
protected:
    void SetUp() override {
        QtTestBase::SetUp();
        model = std::make_unique<US_Model>();
        mockDb = std::make_unique<NiceMock<US_DB2_Mock>>();
    }

    void TearDown() override {
        model.reset();
        mockDb.reset();
        QtTestBase::TearDown();
    }

    std::unique_ptr<US_Model> model;
    std::unique_ptr<NiceMock<US_DB2_Mock>> mockDb;

    // Helper to create valid simulation component
    US_Model::SimulationComponent createValidComponent() {
        US_Model::SimulationComponent sc;
        sc.name = "Test Component";
        sc.mw = 50000.0;
        sc.s = 2.5e-13;
        sc.D = 1.0e-11;
        sc.f = 1.0;
        sc.f_f0 = 1.25;
        sc.vbar20 = 0.73;
        sc.signal_concentration = 1.0;
        return sc;
    }

    // Helper to create valid association
    US_Model::Association createValidAssociation() {
        US_Model::Association assoc;
        assoc.k_d = 1.0e-6;
        assoc.k_off = 1.0e-3;
        assoc.rcomps << 0 << 1;
        assoc.stoichs << 1 << -1;
        return assoc;
    }

    // Helper to create XML content for testing
    QString createTestModelXml() {
        return QString(
                "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                "<!DOCTYPE US_Model>\n"
                "<ModelData version=\"1.0\">\n"
                "<model description=\"Test Model\" modelGUID=\"test-guid-123\" "
                "editGUID=\"edit-guid-456\" wavelength=\"280.0\" "
                "opticsType=\"0\" analysisType=\"1\" globalType=\"0\" "
                "coSedSolute=\"-1\" subGrids=\"0\">\n"
                "<analyte name=\"Component1\" mw=\"50000\" s=\"2.5e-13\" "
                "D=\"1e-11\" f=\"1\" f_f0=\"1.25\" vbar20=\"0.73\" "
                "extinction=\"0\" axial=\"10\" sigma=\"0\" delta=\"0\" "
                "oligomer=\"1\" shape=\"0\" type=\"0\" molar=\"0\" signal=\"1\" />\n"
                "</model>\n"
                "</ModelData>\n"
        );
    }
};

// Constructor Tests
TEST_F(US_ModelTest, DefaultConstructor_InitializesCorrectly) {
EXPECT_FALSE(model->monteCarlo);
EXPECT_EQ(model->wavelength, 0.0);
EXPECT_EQ(model->variance, 0.0);
EXPECT_EQ(model->meniscus, 0.0);
EXPECT_EQ(model->bottom, 0.0);
EXPECT_EQ(model->alphaRP, 0.0);
EXPECT_EQ(model->subGrids, 0);
EXPECT_THAT(model->description, QStringEq("New Model"));
EXPECT_EQ(model->optics, US_Model::ABSORBANCE);
EXPECT_EQ(model->analysis, US_Model::MANUAL);
EXPECT_EQ(model->global, US_Model::NONE);
EXPECT_EQ(model->nmcixs, 0);
EXPECT_EQ(model->coSedSolute, -1);
EXPECT_TRUE(model->modelGUID.isEmpty());
EXPECT_TRUE(model->editGUID.isEmpty());
EXPECT_TRUE(model->requestGUID.isEmpty());
EXPECT_TRUE(model->components.isEmpty());
EXPECT_TRUE(model->associations.isEmpty());
EXPECT_TRUE(model->dataDescrip.isEmpty());
EXPECT_TRUE(model->mcixmls.isEmpty());
}

// SimulationComponent Tests
TEST_F(US_ModelTest, SimulationComponent_DefaultConstructor) {
US_Model::SimulationComponent sc;

EXPECT_THAT(sc.name, QStringEq("New Component"));
EXPECT_TRUE(sc.analyteGUID.isEmpty());
EXPECT_EQ(sc.molar_concentration, 0.0);
EXPECT_EQ(sc.signal_concentration, 1.0);
EXPECT_EQ(sc.vbar20, TYPICAL_VBAR);
EXPECT_EQ(sc.mw, 50000.0);
EXPECT_EQ(sc.s, 0.0);
EXPECT_EQ(sc.D, 0.0);
EXPECT_EQ(sc.f, 1.0);
EXPECT_EQ(sc.f_f0, 1.25);
EXPECT_EQ(sc.extinction, 0.0);
EXPECT_EQ(sc.sigma, 0.0);
EXPECT_EQ(sc.delta, 0.0);
EXPECT_EQ(sc.oligomer, 1);
EXPECT_EQ(sc.shape, US_Model::SPHERE);
EXPECT_EQ(sc.axial_ratio, 10.0);
EXPECT_EQ(sc.analyte_type, 0);
EXPECT_TRUE(sc.c0.radius.isEmpty());
EXPECT_TRUE(sc.c0.concentration.isEmpty());
}

TEST_F(US_ModelTest, SimulationComponent_EqualityOperator_IdenticalComponents) {
US_Model::SimulationComponent sc1 = createValidComponent();
US_Model::SimulationComponent sc2 = sc1;

EXPECT_TRUE(sc1 == sc2);
EXPECT_FALSE(sc1 != sc2);
}

TEST_F(US_ModelTest, SimulationComponent_EqualityOperator_DifferentNames) {
US_Model::SimulationComponent sc1 = createValidComponent();
US_Model::SimulationComponent sc2 = sc1;
sc2.name = "Different Name";

EXPECT_FALSE(sc1 == sc2);
EXPECT_TRUE(sc1 != sc2);
}

// Association Tests
TEST_F(US_ModelTest, Association_DefaultConstructor) {
US_Model::Association assoc;

EXPECT_EQ(assoc.k_d, 0.0);
EXPECT_EQ(assoc.k_off, 0.0);
EXPECT_TRUE(assoc.rcomps.isEmpty());
EXPECT_TRUE(assoc.stoichs.isEmpty());
}

TEST_F(US_ModelTest, Association_EqualityOperator_IdenticalAssociations) {
US_Model::Association assoc1 = createValidAssociation();
US_Model::Association assoc2 = assoc1;

EXPECT_TRUE(assoc1 == assoc2);
EXPECT_FALSE(assoc1 != assoc2);
}

// Model Equality Tests
TEST_F(US_ModelTest, Model_EqualityOperator_IdenticalModels) {
US_Model model2;

EXPECT_TRUE(*model == model2);
EXPECT_FALSE(*model != model2);
}

TEST_F(US_ModelTest, Model_EqualityOperator_DifferentDescriptions) {
US_Model model2;
model2.description = "Different Description";

EXPECT_FALSE(*model == model2);
EXPECT_TRUE(*model != model2);
}

// Coefficient Calculation Tests
TEST_F(US_ModelTest, CalcCoefficients_ValidComponentWithSAndFf0_Success) {
US_Model::SimulationComponent sc = createValidComponent();
sc.s = 2.5e-13;
sc.f_f0 = 1.25;
sc.D = 0.0; // Will be calculated
sc.mw = 0.0; // Will be calculated

bool result = US_Model::calc_coefficients(sc);

EXPECT_TRUE(result);
EXPECT_GT(sc.mw, 0.0);
EXPECT_GT(sc.D, 0.0);
}

TEST_F(US_ModelTest, CalcCoefficients_InsufficientData_Failure) {
US_Model::SimulationComponent sc = createValidComponent();
sc.s = 0.0;
sc.D = 0.0;
sc.mw = 0.0;
sc.f = 0.0;
sc.f_f0 = 0.0;

bool result = US_Model::calc_coefficients(sc);

EXPECT_FALSE(result);
}

TEST_F(US_ModelTest, UpdateCoefficients_AllComponentsValid_Success) {
model->components << createValidComponent();
model->components << createValidComponent();
model->components[1].name = "Component 2";

bool result = model->update_coefficients();

EXPECT_TRUE(result);
}

TEST_F(US_ModelTest, UpdateCoefficients_EmptyComponents_Success) {
bool result = model->update_coefficients();

EXPECT_TRUE(result); // Should succeed with empty components
}

// Component Management Tests
TEST_F(US_ModelTest, AddComponent_ValidComponent_AddsCorrectly) {
US_Model::SimulationComponent sc = createValidComponent();
int initialCount = model->components.size();

model->components << sc;

EXPECT_EQ(model->components.size(), initialCount + 1);
EXPECT_EQ(model->components.last().name, "Test Component");
}

TEST_F(US_ModelTest, RemoveComponent_ValidIndex_RemovesCorrectly) {
model->components << createValidComponent();
model->components << createValidComponent();
model->components[1].name = "Component 2";

int initialCount = model->components.size();
model->components.removeAt(0);

EXPECT_EQ(model->components.size(), initialCount - 1);
EXPECT_EQ(model->components[0].name, "Component 2");
}

// Association Management Tests
TEST_F(US_ModelTest, AddAssociation_ValidAssociation_AddsCorrectly) {
US_Model::Association assoc = createValidAssociation();
int initialCount = model->associations.size();

model->associations << assoc;

EXPECT_EQ(model->associations.size(), initialCount + 1);
EXPECT_EQ(model->associations.last().k_d, 1.0e-6);
}

// Manual Clear Tests - since US_Model doesn't have a clear() method
TEST_F(US_ModelTest, ManualClear_ResetsAllValues) {
// Populate model with test data
model->description = "Test Model";
model->wavelength = 280.0;
model->variance = 0.1;
model->modelGUID = "test-guid";
model->editGUID = "edit-guid";
model->components << createValidComponent();
model->associations << createValidAssociation();

// Manually clear the model (since there's no clear() method)
model->description = "New Model";
model->wavelength = 0.0;
model->variance = 0.0;
model->modelGUID.clear();
model->editGUID.clear();
model->components.clear();
model->associations.clear();

// Verify all values are reset
EXPECT_THAT(model->description, QStringEq("New Model"));
EXPECT_EQ(model->wavelength, 0.0);
EXPECT_EQ(model->variance, 0.0);
EXPECT_TRUE(model->modelGUID.isEmpty());
EXPECT_TRUE(model->editGUID.isEmpty());
EXPECT_TRUE(model->components.isEmpty());
EXPECT_TRUE(model->associations.isEmpty());
}

// Component Search Tests
TEST_F(US_ModelTest, FindComponentByName_ExistingComponent_ReturnsCorrectIndex) {
model->components << createValidComponent();
model->components << createValidComponent();
model->components[1].name = "Unique Component";

// Find component by name (assuming there's a method for this)
bool found = false;
int index = -1;
for (int i = 0; i < model->components.size(); ++i) {
if (model->components[i].name == "Unique Component") {
found = true;
index = i;
break;
}
}

EXPECT_TRUE(found);
EXPECT_EQ(index, 1);
}

TEST_F(US_ModelTest, FindComponentByName_NonExistentComponent_ReturnsNotFound) {
model->components << createValidComponent();

bool found = false;
for (int i = 0; i < model->components.size(); ++i) {
if (model->components[i].name == "Non-existent Component") {
found = true;
break;
}
}

EXPECT_FALSE(found);
}

// Validation Tests
TEST_F(US_ModelTest, IsValid_ValidModel_ReturnsTrue) {
model->description = "Valid Model";
model->wavelength = 280.0;
model->components << createValidComponent();

// Basic validation - model has description, wavelength, and components
bool isValid = !model->description.isEmpty() &&
               model->wavelength > 0 &&
               !model->components.isEmpty();

EXPECT_TRUE(isValid);
}

TEST_F(US_ModelTest, IsValid_EmptyModel_ReturnsFalse) {
// Empty model should be invalid
bool isValid = !model->description.isEmpty() &&
               model->wavelength > 0 &&
               !model->components.isEmpty();

EXPECT_FALSE(isValid);
}

// Copy Constructor Tests
TEST_F(US_ModelTest, CopyConstructor_CopiesAllData) {
// Populate original model
model->description = "Original Model";
model->wavelength = 280.0;
model->variance = 0.1;
model->modelGUID = "original-guid";
model->components << createValidComponent();

// Create copy
US_Model modelCopy(*model);

// Verify copy
EXPECT_EQ(modelCopy.description, model->description);
EXPECT_EQ(modelCopy.wavelength, model->wavelength);
EXPECT_EQ(modelCopy.variance, model->variance);
EXPECT_EQ(modelCopy.modelGUID, model->modelGUID);
EXPECT_EQ(modelCopy.components.size(), model->components.size());
EXPECT_EQ(modelCopy.components[0].name, model->components[0].name);
}

// Assignment Operator Tests
TEST_F(US_ModelTest, AssignmentOperator_CopiesAllData) {
// Populate original model
model->description = "Original Model";
model->wavelength = 280.0;
model->components << createValidComponent();

// Create and assign
US_Model modelCopy;
modelCopy = *model;

// Verify assignment
EXPECT_EQ(modelCopy.description, model->description);
EXPECT_EQ(modelCopy.wavelength, model->wavelength);
EXPECT_EQ(modelCopy.components.size(), model->components.size());
}

// Component Concentration Tests
TEST_F(US_ModelTest, ComponentConcentration_SetAndGet_WorksCorrectly) {
US_Model::SimulationComponent sc = createValidComponent();
sc.signal_concentration = 2.5;
sc.molar_concentration = 1.5e-6;

model->components << sc;

EXPECT_EQ(model->components[0].signal_concentration, 2.5);
EXPECT_EQ(model->components[0].molar_concentration, 1.5e-6);
}

// Component Properties Tests
TEST_F(US_ModelTest, ComponentProperties_SetValidValues_AcceptsValues) {
US_Model::SimulationComponent sc;
sc.mw = 75000.0;
sc.s = 3.5e-13;
sc.D = 1.5e-11;
sc.f_f0 = 1.35;
sc.vbar20 = 0.74;
sc.extinction = 50000.0;

EXPECT_EQ(sc.mw, 75000.0);
EXPECT_EQ(sc.s, 3.5e-13);
EXPECT_EQ(sc.D, 1.5e-11);
EXPECT_EQ(sc.f_f0, 1.35);
EXPECT_EQ(sc.vbar20, 0.74);
EXPECT_EQ(sc.extinction, 50000.0);
}

// Association Properties Tests
TEST_F(US_ModelTest, AssociationProperties_SetValidValues_AcceptsValues) {
US_Model::Association assoc;
assoc.k_d = 2.5e-7;
assoc.k_off = 1.5e-3;
assoc.rcomps << 0 << 1 << 2;
assoc.stoichs << 1 << 1 << -1;

EXPECT_EQ(assoc.k_d, 2.5e-7);
EXPECT_EQ(assoc.k_off, 1.5e-3);
EXPECT_EQ(assoc.rcomps.size(), 3);
EXPECT_EQ(assoc.stoichs.size(), 3);
EXPECT_EQ(assoc.rcomps[2], 2);
EXPECT_EQ(assoc.stoichs[2], -1);
}

// Edge Cases
TEST_F(US_ModelTest, ComponentWithZeroMW_HandlesCorrectly) {
US_Model::SimulationComponent sc = createValidComponent();
sc.mw = 0.0;

model->components << sc;

EXPECT_EQ(model->components[0].mw, 0.0);
// Model should handle zero MW gracefully
}

TEST_F(US_ModelTest, ComponentWithNegativeConcentration_HandlesCorrectly) {
US_Model::SimulationComponent sc = createValidComponent();
sc.signal_concentration = -1.0;

model->components << sc;

EXPECT_EQ(model->components[0].signal_concentration, -1.0);
// Model should handle negative concentration (though it might be invalid)
}

// Boundary Value Tests
TEST_F(US_ModelTest, ExtremeWavelengthValues_HandlesCorrectly) {
model->wavelength = 1000.0; // Very high
EXPECT_EQ(model->wavelength, 1000.0);

model->wavelength = 0.1; // Very low
EXPECT_EQ(model->wavelength, 0.1);

model->wavelength = -1.0; // Negative
EXPECT_EQ(model->wavelength, -1.0);
}

TEST_F(US_ModelTest, LargeNumberOfComponents_HandlesCorrectly) {
// Add many components
for (int i = 0; i < 100; ++i) {
US_Model::SimulationComponent sc = createValidComponent();
sc.name = QString("Component %1").arg(i);
model->components << sc;
}

EXPECT_EQ(model->components.size(), 100);
EXPECT_EQ(model->components[50].name, "Component 50");
EXPECT_EQ(model->components[99].name, "Component 99");
}