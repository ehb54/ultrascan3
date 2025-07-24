// test_us_memory_unit.cpp - Unit tests for US_Memory class
#include "qt_test_base.h"
#include "us_memory.h"
#include <QString>
#include <limits>

using namespace qt_matchers;

class TestUSMemoryUnit : public QtTestBase {
protected:
    void SetUp() override {
        QtTestBase::SetUp();
    }

    void TearDown() override {
        QtTestBase::TearDown();
    }
};

// ============================================================================
// RSS_NOW METHOD TESTS
// ============================================================================

TEST_F(TestUSMemoryUnit, RssNowReturnsPositiveValue) {
// Test that rss_now returns a positive value
long int rss = US_Memory::rss_now();

EXPECT_GT(rss, 0L) << "RSS should be positive (process using some memory)";
}

TEST_F(TestUSMemoryUnit, RssNowReturnsReasonableValue) {
// Test that rss_now returns a reasonable value (not extremely large)
long int rss = US_Memory::rss_now();

// RSS should be less than 100GB (reasonable upper bound for a test process)
EXPECT_LT(rss, 100L * 1024L * 1024L) << "RSS should be less than 100GB";

// RSS should be at least 1KB (minimal for any running process)
EXPECT_GE(rss, 1L) << "RSS should be at least 1KB";
}

TEST_F(TestUSMemoryUnit, RssNowConsistentCalls) {
// Test that consecutive calls return similar values
long int rss1 = US_Memory::rss_now();
long int rss2 = US_Memory::rss_now();

// Values should be close (within 10MB) for consecutive calls
long int diff = std::abs(rss2 - rss1);
EXPECT_LT(diff, 10L * 1024L) << "Consecutive RSS calls should be similar";
}

TEST_F(TestUSMemoryUnit, RssNowMultipleCalls) {
// Test multiple calls to ensure no resource leaks in the function
std::vector<long int> rss_values;

for (int i = 0; i < 5; i++) {
rss_values.push_back(US_Memory::rss_now());
}

// All values should be positive
for (size_t i = 0; i < rss_values.size(); i++) {
EXPECT_GT(rss_values[i], 0L) << "RSS call " << i << " should be positive";
}

// Check that values are in a reasonable range relative to each other
long int min_rss = *std::min_element(rss_values.begin(), rss_values.end());
long int max_rss = *std::max_element(rss_values.begin(), rss_values.end());

// Variation should be reasonable (less than 50MB for test process)
EXPECT_LT(max_rss - min_rss, 50L * 1024L)
<< "RSS variation should be reasonable across multiple calls";
}

TEST_F(TestUSMemoryUnit, RssNowAfterMemoryAllocation) {
// Test that RSS increases after allocating memory
long int rss_before = US_Memory::rss_now();

// Allocate a significant amount of memory (10MB)
const size_t allocation_size = 10 * 1024 * 1024;
char* memory_block = new char[allocation_size];

// Touch the memory to ensure it's actually allocated
for (size_t i = 0; i < allocation_size; i += 4096) {
memory_block[i] = static_cast<char>(i % 256);
}

long int rss_after = US_Memory::rss_now();

delete[] memory_block;

// RSS should have increased (allowing for some measurement variance)
EXPECT_GE(rss_after, rss_before)
<< "RSS should increase after memory allocation";

// The increase should be reasonable (at least 1MB, but may be more due to heap overhead)
long int increase = rss_after - rss_before;
EXPECT_GE(increase, 1024L) << "RSS increase should be at least 1MB";
}

// ============================================================================
// RSS_MAX METHOD TESTS
// ============================================================================

TEST_F(TestUSMemoryUnit, RssMaxInitialValue) {
// Test rss_max with initial value
long int initial_max = 1000L; // 1MB initial value
long int result = US_Memory::rss_max(initial_max);

EXPECT_GE(result, initial_max) << "rss_max should return at least the initial value";
EXPECT_EQ(result, initial_max) << "rss_max should update the input parameter";
}

TEST_F(TestUSMemoryUnit, RssMaxWithZeroInitial) {
// Test rss_max with zero initial value
long int max_rss = 0L;
long int result = US_Memory::rss_max(max_rss);

EXPECT_GT(result, 0L) << "rss_max should return positive value";
EXPECT_EQ(result, max_rss) << "rss_max should update the input parameter";
EXPECT_GT(max_rss, 0L) << "Input parameter should be updated to positive value";
}

TEST_F(TestUSMemoryUnit, RssMaxIncreasingValues) {
// Test rss_max with increasing values
long int max_rss = 500L;

long int result1 = US_Memory::rss_max(max_rss);
EXPECT_GE(result1, 500L) << "First call should return at least 500KB";

// Set to a higher value
max_rss = result1 + 1000L;
long int result2 = US_Memory::rss_max(max_rss);
EXPECT_EQ(result2, max_rss) << "Second call should return the higher value";
}

TEST_F(TestUSMemoryUnit, RssMaxCurrentHigherThanInput) {
// Test rss_max when current RSS is higher than input
long int current_rss = US_Memory::rss_now();
long int small_max = 100L; // Intentionally small value

long int result = US_Memory::rss_max(small_max);

EXPECT_GE(result, current_rss) << "Result should be at least current RSS";
EXPECT_EQ(result, small_max) << "Input parameter should be updated";
EXPECT_GE(small_max, current_rss) << "Input should be updated to current RSS";
}

TEST_F(TestUSMemoryUnit, RssMaxMultipleCalls) {
// Test multiple calls to rss_max maintain maximum
long int max_rss = 0L;
std::vector<long int> results;

for (int i = 0; i < 5; i++) {
results.push_back(US_Memory::rss_max(max_rss));
}

// Each result should be >= previous result (non-decreasing)
for (size_t i = 1; i < results.size(); i++) {
EXPECT_GE(results[i], results[i-1])
<< "RSS max should be non-decreasing across calls";
}

// Final max_rss should equal the last result
EXPECT_EQ(max_rss, results.back())
<< "Input parameter should equal final result";
}

TEST_F(TestUSMemoryUnit, RssMaxWithNegativeInput) {
// Test rss_max with negative initial value
long int negative_max = -1000L;
long int result = US_Memory::rss_max(negative_max);

EXPECT_GT(result, 0L) << "Result should be positive despite negative input";
EXPECT_GT(negative_max, 0L) << "Input should be updated to positive value";
}

TEST_F(TestUSMemoryUnit, RssMaxWithLargeInput) {
// Test rss_max with very large initial value
long int large_max = 1000000L * 1024L; // 1TB
long int result = US_Memory::rss_max(large_max);

EXPECT_EQ(result, large_max) << "Should preserve large input value if larger than current";
EXPECT_EQ(result, large_max) << "Input parameter should remain unchanged";
}

// ============================================================================
// MEMORY_PROFILE METHOD TESTS
// ============================================================================

TEST_F(TestUSMemoryUnit, MemoryProfileBasicCall) {
// Test basic memory_profile call without parameters
int available_percent = US_Memory::memory_profile();

EXPECT_GE(available_percent, 0) << "Available percentage should be non-negative";
EXPECT_LE(available_percent, 100) << "Available percentage should not exceed 100%";
}

TEST_F(TestUSMemoryUnit, MemoryProfileWithAllParameters) {
// Test memory_profile with all output parameters
int mem_available = -1;
int mem_total = -1;
int mem_used = -1;

int available_percent = US_Memory::memory_profile(&mem_available, &mem_total, &mem_used);

// Check return value
EXPECT_GE(available_percent, 0) << "Available percentage should be non-negative";
EXPECT_LE(available_percent, 100) << "Available percentage should not exceed 100%";

// Check output parameters
EXPECT_GT(mem_total, 0) << "Total memory should be positive";
EXPECT_GE(mem_available, 0) << "Available memory should be non-negative";
EXPECT_GE(mem_used, 0) << "Used memory should be non-negative";

// Check relationships
EXPECT_EQ(mem_total, mem_available + mem_used)
<< "Total should equal available plus used";

// Check percentage calculation
int expected_percent = static_cast<int>(std::round(mem_available * 100.0 / mem_total));
EXPECT_EQ(available_percent, expected_percent)
<< "Percentage should match calculation";
}

TEST_F(TestUSMemoryUnit, MemoryProfileWithSomeParameters) {
// Test memory_profile with only some parameters
int mem_available = -1;
int mem_total = -1;

int available_percent = US_Memory::memory_profile(&mem_available, &mem_total, nullptr);

EXPECT_GE(available_percent, 0) << "Available percentage should be valid";
EXPECT_GT(mem_total, 0) << "Total memory should be positive";
EXPECT_GE(mem_available, 0) << "Available memory should be non-negative";
EXPECT_LE(mem_available, mem_total) << "Available should not exceed total";
}

TEST_F(TestUSMemoryUnit, MemoryProfileNullParameters) {
// Test memory_profile with null parameters (should not crash)
EXPECT_NO_THROW({
int result = US_Memory::memory_profile(nullptr, nullptr, nullptr);
EXPECT_GE(result, 0) << "Should return valid percentage with null parameters";
EXPECT_LE(result, 100) << "Should return valid percentage with null parameters";
}) << "memory_profile should handle null parameters gracefully";
}

TEST_F(TestUSMemoryUnit, MemoryProfileConsistentCalls) {
// Test that consecutive calls return similar values
int mem_avail1, mem_total1, mem_used1;
int mem_avail2, mem_total2, mem_used2;

int percent1 = US_Memory::memory_profile(&mem_avail1, &mem_total1, &mem_used1);
int percent2 = US_Memory::memory_profile(&mem_avail2, &mem_total2, &mem_used2);

// Total memory should be identical or very close
EXPECT_EQ(mem_total1, mem_total2) << "Total memory should be consistent";

// Available memory should be close (within 100MB variance)
int avail_diff = std::abs(mem_avail2 - mem_avail1);
EXPECT_LT(avail_diff, 100) << "Available memory should be similar in consecutive calls";

// Percentages should be close
int percent_diff = std::abs(percent2 - percent1);
EXPECT_LE(percent_diff, 5) << "Available percentage should be similar";
}

TEST_F(TestUSMemoryUnit, MemoryProfileReasonableValues) {
// Test that memory_profile returns reasonable values
int mem_available, mem_total, mem_used;

int available_percent = US_Memory::memory_profile(&mem_available, &mem_total, &mem_used);

// Total memory should be at least 512MB (reasonable minimum for modern systems)
EXPECT_GE(mem_total, 512) << "Total memory should be at least 512MB";

// Total memory should be less than 1TB (reasonable maximum for most systems)
EXPECT_LT(mem_total, 1024 * 1024) << "Total memory should be less than 1TB";

// Used memory should be reasonable (at least 100MB, less than 90% of total)
EXPECT_GE(mem_used, 100) << "Used memory should be at least 100MB";
EXPECT_LT(mem_used, mem_total * 0.95) << "Used memory should be less than 95% of total";

// Available memory should be reasonable
EXPECT_GT(mem_available, 0) << "Available memory should be positive";
EXPECT_LT(mem_available, mem_total) << "Available memory should be less than total";
}

// ============================================================================
// EDGE CASES AND BOUNDARY CONDITIONS
// ============================================================================

TEST_F(TestUSMemoryUnit, MemoryProfileMultipleCallsStability) {
// Test stability across multiple calls
std::vector<int> totals, availables, percentages;

for (int i = 0; i < 3; i++) {
int avail, total, used;
int percent = US_Memory::memory_profile(&avail, &total, &used);

totals.push_back(total);
availables.push_back(avail);
percentages.push_back(percent);
}

// Total memory should be consistent
for (size_t i = 1; i < totals.size(); i++) {
EXPECT_EQ(totals[i], totals[0]) << "Total memory should be consistent";
}

// Available memory should not vary wildly
int min_avail = *std::min_element(availables.begin(), availables.end());
int max_avail = *std::max_element(availables.begin(), availables.end());
EXPECT_LT(max_avail - min_avail, totals[0] / 10)
<< "Available memory variation should be reasonable";
}

TEST_F(TestUSMemoryUnit, RssMaxBoundaryValues) {
// Test rss_max with boundary values
long int max_long = std::numeric_limits<long int>::max();
long int result = US_Memory::rss_max(max_long);

EXPECT_EQ(result, max_long) << "Should handle maximum long int value";

long int min_long = std::numeric_limits<long int>::min();
long int result_min = US_Memory::rss_max(min_long);

EXPECT_GT(result_min, 0L) << "Should return positive value even with minimum input";
EXPECT_GT(min_long, 0L) << "Should update minimum value to positive";
}

TEST_F(TestUSMemoryUnit, MemoryMethodsIndependence) {
// Test that different memory methods don't interfere with each other
long int rss1 = US_Memory::rss_now();

int mem_avail, mem_total, mem_used;
int percent = US_Memory::memory_profile(&mem_avail, &mem_total, &mem_used);

long int rss2 = US_Memory::rss_now();

// RSS calls should be consistent regardless of memory_profile call
long int rss_diff = std::abs(rss2 - rss1);
EXPECT_LT(rss_diff, 1024L) << "RSS should be stable across other method calls";

// Memory profile should return valid values
EXPECT_GE(percent, 0) << "Memory profile should be valid";
EXPECT_GT(mem_total, 0) << "Total memory should be positive";
}

TEST_F(TestUSMemoryUnit, RssNowAfterLargeDeallocation) {
// Test RSS behavior after deallocating large memory block
long int rss_initial = US_Memory::rss_now();

// Allocate and immediately deallocate large block
const size_t large_size = 50 * 1024 * 1024; // 50MB
char* large_block = new char[large_size];

// Touch memory to ensure allocation
for (size_t i = 0; i < large_size; i += 4096) {
large_block[i] = 1;
}

long int rss_after_alloc = US_Memory::rss_now();
delete[] large_block;

long int rss_after_dealloc = US_Memory::rss_now();

// RSS should have increased during allocation
EXPECT_GE(rss_after_alloc, rss_initial)
<< "RSS should increase during large allocation";

// RSS after deallocation may or may not decrease (depends on OS behavior)
// But it should be reasonable (not negative or extremely large)
EXPECT_GT(rss_after_dealloc, 0L) << "RSS should remain positive after deallocation";
EXPECT_LT(rss_after_dealloc, rss_after_alloc + 10240L)
<< "RSS should not increase significantly after deallocation";
}