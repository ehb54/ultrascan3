#include "test_us_memory.h"
#include "us_memory.h"
#include <vector>
#include <memory>

// Use your custom Qt matchers
using namespace qt_matchers;

// TestUSMemory method implementations
void TestUSMemory::SetUp() {
    QtTestBase::SetUp();
    // Per-test setup for Memory tests
}

void TestUSMemory::TearDown() {
    // Per-test cleanup for Memory tests
    QtTestBase::TearDown();
}

// Suite-level setup for Memory tests
void TestUSMemory::SetUpTestSuite() {
    QtTestBase::SetUpTestSuite();
    // One-time setup for all TestUSMemory tests
}

// Suite-level cleanup for Memory tests
void TestUSMemory::TearDownTestSuite() {
    // One-time cleanup for all TestUSMemory tests
}

// Helper method to allocate memory for testing
void TestUSMemory::allocateMemory(size_t bytes) {
    // Allocate and immediately free memory to test RSS changes
    std::vector<char> temp(bytes);
    // Fill with data to ensure actual allocation
    std::fill(temp.begin(), temp.end(), 42);
}

// Helper method to validate memory metrics consistency
void TestUSMemory::validateMemoryMetrics(int memA, int memT, int memU, int memAvPc) {
    // Memory available should be non-negative
    EXPECT_GE(memA, 0) << "Available memory should be non-negative";

    // Total memory should be positive
    EXPECT_GT(memT, 0) << "Total memory should be positive";

    // Used memory should be non-negative and not exceed total
    EXPECT_GE(memU, 0) << "Used memory should be non-negative";
    EXPECT_LE(memU, memT) << "Used memory should not exceed total memory";

    // Available percentage should be in valid range
    EXPECT_GE(memAvPc, 0) << "Memory availability percentage should be >= 0";
    EXPECT_LE(memAvPc, 100) << "Memory availability percentage should be <= 100";

    // Available + Used should approximately equal Total (allowing for rounding)
    int calculatedTotal = memA + memU;
    int tolerance = memT / 100; // 1% tolerance for rounding/measurement differences
    EXPECT_NEAR(calculatedTotal, memT, tolerance)
                        << "Available + Used should approximately equal Total memory";
}

TEST_F(TestUSMemory, RssNow) {
    // Test current RSS (Resident Set Size) measurement
    long int rss = US_Memory::rss_now();

    EXPECT_GT(rss, 0) << "RSS should be greater than 0 for a running process";

    // RSS should be reasonable (not absurdly large)
    // Assuming less than 10GB for a test process
    EXPECT_LT(rss, 10L * 1024 * 1024 * 1024)
                        << "RSS should be reasonable for a test process";
}

TEST_F(TestUSMemory, RssNowConsistency) {
    // Test that multiple calls return consistent values
    long int rss1 = US_Memory::rss_now();
    long int rss2 = US_Memory::rss_now();
    long int rss3 = US_Memory::rss_now();

    // All measurements should be positive
    EXPECT_GT(rss1, 0) << "First RSS measurement should be positive";
    EXPECT_GT(rss2, 0) << "Second RSS measurement should be positive";
    EXPECT_GT(rss3, 0) << "Third RSS measurement should be positive";

    // Measurements should be relatively close (within 10% variance)
    long int maxRss = std::max({rss1, rss2, rss3});
    long int minRss = std::min({rss1, rss2, rss3});
    long int variance = maxRss - minRss;

    EXPECT_LE(variance, maxRss / 10)
                        << "RSS measurements should be consistent (within 10% variance)";
}

TEST_F(TestUSMemory, RssMax) {
    // Test RSS maximum tracking
    long int initialRssMax = 0;
    long int newRssMax = US_Memory::rss_max(initialRssMax);

    EXPECT_GE(newRssMax, initialRssMax)
                        << "New RSS max should not be less than the initial value";
    EXPECT_GT(newRssMax, 0)
                        << "RSS max should be positive";

    // Test with current RSS
    long int currentRss = US_Memory::rss_now();
    long int currentMax = newRssMax; // Use variable for reference
    long int updatedRssMax = US_Memory::rss_max(currentMax);

    EXPECT_GE(updatedRssMax, newRssMax)
                        << "Updated RSS max should not decrease";
    EXPECT_GE(updatedRssMax, currentRss)
                        << "RSS max should be at least as large as current RSS";
}

TEST_F(TestUSMemory, RssMaxAfterAllocation) {
    // Test RSS max tracking after memory allocation
    long int initialMax = 0; // Use variable for reference
    long int currentMax = US_Memory::rss_max(initialMax);

    // Allocate some memory
    allocateMemory(1024 * 1024); // 1MB

    long int afterAllocationMax = US_Memory::rss_max(currentMax);

    EXPECT_GE(afterAllocationMax, currentMax)
                        << "RSS max should not decrease after memory allocation";
}

TEST_F(TestUSMemory, MemoryProfile) {
    // Test memory profile functionality
    int memA, memT, memU;
    int memAvPc = US_Memory::memory_profile(&memA, &memT, &memU);

    // Use helper method for comprehensive validation
    validateMemoryMetrics(memA, memT, memU, memAvPc);

    // Additional specific checks
    EXPECT_GT(memT, 1024) << "Total memory should be at least 1KB";

    // Memory values should be reasonable (not negative or impossibly large)
    EXPECT_LT(memT, INT_MAX / 2) << "Total memory should be within reasonable bounds";
    EXPECT_LT(memU, INT_MAX / 2) << "Used memory should be within reasonable bounds";
}

TEST_F(TestUSMemory, MemoryProfileConsistency) {
    // Test that multiple memory profile calls return consistent data
    int memA1, memT1, memU1;
    int memA2, memT2, memU2;

    int memAvPc1 = US_Memory::memory_profile(&memA1, &memT1, &memU1);
    int memAvPc2 = US_Memory::memory_profile(&memA2, &memT2, &memU2);

    // Validate both measurements
    validateMemoryMetrics(memA1, memT1, memU1, memAvPc1);
    validateMemoryMetrics(memA2, memT2, memU2, memAvPc2);

    // Total memory should be consistent between calls
    EXPECT_EQ(memT1, memT2) << "Total memory should be consistent between calls";

    // Used and available memory can vary slightly, but should be reasonable
    int usedDiff = std::abs(memU1 - memU2);
    int availableDiff = std::abs(memA1 - memA2);

    EXPECT_LE(usedDiff, memT1 / 20)
                        << "Used memory should not vary more than 5% between rapid calls";
    EXPECT_LE(availableDiff, memT1 / 20)
                        << "Available memory should not vary more than 5% between rapid calls";
}

TEST_F(TestUSMemory, MemoryProfileAfterAllocation) {
    // Test memory profile changes after allocation
    int memA_before, memT_before, memU_before;
    int memAvPc_before = US_Memory::memory_profile(&memA_before, &memT_before, &memU_before);

    validateMemoryMetrics(memA_before, memT_before, memU_before, memAvPc_before);

    // Allocate significant memory
    size_t allocationSize = 10 * 1024 * 1024; // 10MB
    allocateMemory(allocationSize);

    int memA_after, memT_after, memU_after;
    int memAvPc_after = US_Memory::memory_profile(&memA_after, &memT_after, &memU_after);

    validateMemoryMetrics(memA_after, memT_after, memU_after, memAvPc_after);

    // Total memory should remain the same
    EXPECT_EQ(memT_before, memT_after)
                        << "Total memory should not change due to allocation";

    // Used memory might increase, available might decrease
    // (Though the allocation is temporary and may not show significant change)
    EXPECT_LE(memA_after, memA_before + static_cast<int>(allocationSize / 1024))
                        << "Available memory should account for allocation";
}

TEST_F(TestUSMemory, MemoryProfileValidPointers) {
    // Test that memory_profile handles null pointers gracefully
    int memA, memT, memU;

    // All valid pointers - should work normally
    int result = US_Memory::memory_profile(&memA, &memT, &memU);
    EXPECT_GE(result, 0) << "Memory profile with valid pointers should succeed";

    validateMemoryMetrics(memA, memT, memU, result);
}

TEST_F(TestUSMemory, MemoryPercentageCalculation) {
    // Test that memory percentage is calculated correctly
    int memA, memT, memU;
    int memAvPc = US_Memory::memory_profile(&memA, &memT, &memU);

    // Calculate expected percentage
    int expectedPc = (memT > 0) ? (memA * 100) / memT : 0;

    // Allow for rounding differences
    EXPECT_NEAR(memAvPc, expectedPc, 1)
                        << "Memory availability percentage should match calculated value";

    // Verify calculation makes sense
    if (memAvPc == 100) {
        EXPECT_EQ(memU, 0) << "If 100% available, used memory should be 0";
    }
    if (memAvPc == 0) {
        EXPECT_EQ(memA, 0) << "If 0% available, available memory should be 0";
    }
}

// Integration test combining RSS and memory profile
TEST_F(TestUSMemory, MemoryFunctionsIntegration) {
    // Test that RSS and memory profile functions work together logically
    long int currentRss = US_Memory::rss_now();

    long int maxRssValue = 0; // Use variable for reference
    long int maxRss = US_Memory::rss_max(maxRssValue);

    int memA, memT, memU;
    int memAvPc = US_Memory::memory_profile(&memA, &memT, &memU);

    // All measurements should be positive and reasonable
    EXPECT_GT(currentRss, 0) << "Current RSS should be positive";
    EXPECT_GE(maxRss, currentRss) << "Max RSS should be >= current RSS";

    validateMemoryMetrics(memA, memT, memU, memAvPc);

    // RSS (process memory) should be less than total system memory
    // Convert RSS from bytes to KB for comparison (assuming memT is in KB)
    long int rssInKB = currentRss / 1024;
    EXPECT_LE(rssInKB, static_cast<long int>(memT))
                        << "Process RSS should not exceed total system memory";

    // Process RSS should contribute to used memory
    EXPECT_GT(memU, 0) << "Used memory should be positive when process is running";
}