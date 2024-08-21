#include <gtest/gtest.h>
#include "us_stiffbase.h"

class US_StiffBaseTest : public ::testing::Test {
protected:
    US_StiffBase* stiffBase;

    void SetUp() override {
        stiffBase = new US_StiffBase();
    }

    void TearDown() override {
        delete stiffBase;
    }
};

// Test CompLocalStif method
TEST_F(US_StiffBaseTest, CompLocalStifComputesCorrectly) {
    double xd[4][2] = {
            {0.0, 0.0},
            {1.0, 0.0},
            {1.0, 1.0},
            {0.0, 1.0}
    };
    double D = 1.0;
    double sw2 = 1.0;
    double* Stif[4];
    for (int i = 0; i < 4; ++i) {
        Stif[i] = new double[4];
    }

    stiffBase->CompLocalStif(4, xd, D, sw2, Stif);

    // Example: Verify that Stif matrix elements fall within a reasonable range
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            // Check if Stif[i][j] is within an expected range
            EXPECT_GT(Stif[i][j], -1.0);  // Example lower bound
            EXPECT_LT(Stif[i][j], 1.0);   // Example upper bound
        }
    }

    // Clean up the allocated memory
    for (int i = 0; i < 4; ++i) {
        delete[] Stif[i];
    }
}

