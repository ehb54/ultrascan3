#ifndef TEST_US_SIMPARMS_H
#define TEST_US_SIMPARMS_H

#include <gtest/gtest.h>
#include "qt_test_base.h"
// Forward declarations to avoid multiple inclusion issues
class US_SimulationParameters;
class US_AbstractCenterpiece;
class US_Hardware;

// Test fixture class for US_SimulationParameters - inherits from your QtTestBase
class TestUSSimparms : public QtTestBase {
protected:
    void SetUp() override;
    void TearDown() override;

    // Suite-level setup and cleanup declarations
    static void SetUpTestSuite();
    static void TearDownTestSuite();

protected:
    // Helper methods and mock classes
    void validateDefaultConstructorValues(const US_SimulationParameters& simParams);
    void validateHardwareSetup(const US_SimulationParameters& simParams,
                               const QString& expectedRotorCalID);
};

#endif // TEST_US_SIMPARMS_H