#include <gtest/gtest.h>
#include "us_model.h"

// Test the US_Model constructor
TEST(US_ModelTest, ConstructorInitializesCorrectly) {
    US_Model model;

    EXPECT_EQ(model.monteCarlo, false);
    EXPECT_DOUBLE_EQ(model.wavelength, 0.0);
    EXPECT_DOUBLE_EQ(model.variance, 0.0);
    EXPECT_DOUBLE_EQ(model.meniscus, 0.0);
    EXPECT_DOUBLE_EQ(model.bottom, 0.0);
    EXPECT_DOUBLE_EQ(model.alphaRP, 0.0);
    EXPECT_EQ(model.description, "New Model");
    EXPECT_TRUE(model.modelGUID.isEmpty());
    EXPECT_TRUE(model.editGUID.isEmpty());
    EXPECT_TRUE(model.requestGUID.isEmpty());
    EXPECT_TRUE(model.dataDescrip.isEmpty());
    EXPECT_EQ(model.optics, US_Model::ABSORBANCE);
}

// Test the equality operator for SimulationComponent
TEST(SimulationComponentTest, EqualityOperatorWorksCorrectly) {
    US_Model::SimulationComponent component1;
    US_Model::SimulationComponent component2;

    // Default components should be equal
    EXPECT_TRUE(component1 == component2);

    // Modify one component and test inequality
    component2.molar_concentration = 10.0;
    EXPECT_FALSE(component1 == component2);
}

