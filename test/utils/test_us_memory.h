#ifndef TEST_US_MEMORY_H
#define TEST_US_MEMORY_H

#include <gtest/gtest.h>
#include "qt_test_base.h"
// Forward declaration to avoid multiple inclusion issues
class US_Memory;

// Test fixture class for US_Memory - inherits from your QtTestBase
class TestUSMemory : public QtTestBase {
protected:
    void SetUp() override;
    void TearDown() override;

    // Suite-level setup and cleanup declarations
    static void SetUpTestSuite();
    static void TearDownTestSuite();

protected:
    // Helper methods for memory testing
    void allocateMemory(size_t bytes);
    void validateMemoryMetrics(int memA, int memT, int memU, int memAvPc);
};

#endif // TEST_US_MEMORY_H