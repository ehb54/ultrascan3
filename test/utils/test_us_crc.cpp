// test_us_crc_unit.cpp - Unit tests for US_Crc class
#include "qt_test_base.h"
#include "us_crc.h"
#include <QByteArray>
#include <limits>

using namespace qt_matchers;

class TestUSCrcUnit : public QtTestBase {
protected:
    void SetUp() override {
        QtTestBase::SetUp();
    }

    void TearDown() override {
        QtTestBase::TearDown();
    }
};

// Test the crc32 method with null buffer
TEST_F(TestUSCrcUnit, Crc32NullBuffer) {
// Test null buffer should return 0
quint32 result = US_Crc::crc32(0, nullptr, 0);
EXPECT_EQ(result, 0u) << "CRC32 with null buffer should return 0";
}

// Test the crc32 method with null buffer and non-zero initial CRC
TEST_F(TestUSCrcUnit, Crc32NullBufferNonZeroInitial) {
// Test null buffer with non-zero initial CRC should still return 0
quint32 result = US_Crc::crc32(0xFFFFFFFF, nullptr, 0);
EXPECT_EQ(result, 0u) << "CRC32 with null buffer should return 0 regardless of initial CRC";
}

// Test the crc32 method with null buffer and non-zero length (edge case)
TEST_F(TestUSCrcUnit, Crc32NullBufferNonZeroLength) {
// This tests the guard condition - null buffer should return 0 even with non-zero length
quint32 result = US_Crc::crc32(0, nullptr, 100);
EXPECT_EQ(result, 0u) << "CRC32 with null buffer should return 0 even with non-zero length";
}

// Test the crc32 method with empty data (zero length)
TEST_F(TestUSCrcUnit, Crc32EmptyData) {
unsigned char dummy = 0x00;
quint32 result = US_Crc::crc32(0, &dummy, 0);
EXPECT_EQ(result, 0u) << "CRC32 with zero length should return initial CRC XOR 0xFFFFFFFF twice (which is initial CRC)";
}

// Test the crc32 method with single byte
TEST_F(TestUSCrcUnit, Crc32SingleByte) {
unsigned char data = 0x61; // 'a'
quint32 result = US_Crc::crc32(0, &data, 1);
EXPECT_EQ(result, 0xe8b7be43u) << "CRC32 of single byte 'a' should be 0xe8b7be43";
}

// Test the crc32 method with different single bytes
TEST_F(TestUSCrcUnit, Crc32DifferentSingleBytes) {
unsigned char data1 = 0x00;
unsigned char data2 = 0xFF;

quint32 result1 = US_Crc::crc32(0, &data1, 1);
quint32 result2 = US_Crc::crc32(0, &data2, 1);

EXPECT_NE(result1, result2) << "Different bytes should produce different CRC values";
EXPECT_EQ(result1, 0xd202ef8du) << "CRC32 of 0x00 should be 0xd202ef8d";
EXPECT_EQ(result2, 0xff000000u) << "CRC32 of 0xFF should be 0xff000000";
}

// Test the crc32 method with known test vector
TEST_F(TestUSCrcUnit, Crc32KnownVector) {
QByteArray testData("123456789");
quint32 result = US_Crc::crc32(
        0,
        reinterpret_cast<const unsigned char*>(testData.constData()),
        testData.length()
);
EXPECT_EQ(result, 0xcbf43926u) << "CRC32 of '123456789' should be 0xcbf43926";
}

// Test the crc32 method with different initial CRC values
TEST_F(TestUSCrcUnit, Crc32DifferentInitialValues) {
unsigned char data = 0x61; // 'a'

quint32 result_zero = US_Crc::crc32(0, &data, 1);
quint32 result_ffff = US_Crc::crc32(0xFFFFFFFF, &data, 1);
quint32 result_custom = US_Crc::crc32(0x12345678, &data, 1);

EXPECT_NE(result_zero, result_ffff) << "Different initial CRC should produce different results";
EXPECT_NE(result_zero, result_custom) << "Different initial CRC should produce different results";
EXPECT_NE(result_ffff, result_custom) << "Different initial CRC should produce different results";
}

// Test the crc32 method with boundary values for initial CRC
TEST_F(TestUSCrcUnit, Crc32BoundaryInitialValues) {
unsigned char data = 0x42;

quint32 result_min = US_Crc::crc32(0, &data, 1);
quint32 result_max = US_Crc::crc32(0xFFFFFFFF, &data, 1);

EXPECT_NE(result_min, result_max) << "Min and max initial CRC values should produce different results";
}

// Test the crc32 method with all byte values (0-255)
TEST_F(TestUSCrcUnit, Crc32AllByteValues) {
std::set<quint32> crcValues;

for (int i = 0; i <= 255; i++) {
unsigned char data = static_cast<unsigned char>(i);
quint32 result = US_Crc::crc32(0, &data, 1);
crcValues.insert(result);
}

// All byte values should produce unique CRC values (property of good CRC)
EXPECT_EQ(crcValues.size(), 256u) << "All 256 byte values should produce unique CRC results";
}

// Test the crc32 method with maximum unsigned int length
TEST_F(TestUSCrcUnit, Crc32MaxLength) {
// Test with a smaller "large" length to avoid memory issues
const unsigned int large_length = 1000;
std::vector<unsigned char> data(large_length, 0xAA);

quint32 result = US_Crc::crc32(0, data.data(), large_length);
EXPECT_NE(result, 0u) << "CRC32 of large data should not be zero";

// Test consistency - same data should produce same result
quint32 result2 = US_Crc::crc32(0, data.data(), large_length);
EXPECT_EQ(result, result2) << "CRC32 should be consistent for same data";
}

// Test the crc32 method with incremental calculation
TEST_F(TestUSCrcUnit, Crc32IncrementalCalculation) {
QByteArray fullData("abcdefgh");
QByteArray part1("abcd");
QByteArray part2("efgh");

// Calculate CRC for full data
quint32 fullCrc = US_Crc::crc32(
        0,
        reinterpret_cast<const unsigned char*>(fullData.constData()),
        fullData.length()
);

// Calculate CRC incrementally
quint32 partialCrc = US_Crc::crc32(
        0,
        reinterpret_cast<const unsigned char*>(part1.constData()),
        part1.length()
);

quint32 incrementalCrc = US_Crc::crc32(
        partialCrc,
        reinterpret_cast<const unsigned char*>(part2.constData()),
        part2.length()
);

EXPECT_EQ(fullCrc, incrementalCrc) << "Incremental CRC calculation should match full calculation";
}

// Test the crc32 method with specific patterns
TEST_F(TestUSCrcUnit, Crc32SpecificPatterns) {
// Test with all zeros
std::vector<unsigned char> zeros(10, 0x00);
quint32 result_zeros = US_Crc::crc32(0, zeros.data(), zeros.size());

// Test with all ones
std::vector<unsigned char> ones(10, 0xFF);
quint32 result_ones = US_Crc::crc32(0, ones.data(), ones.size());

// Test with alternating pattern
std::vector<unsigned char> alternating = {0xAA, 0x55, 0xAA, 0x55, 0xAA};
quint32 result_alt = US_Crc::crc32(0, alternating.data(), alternating.size());

EXPECT_NE(result_zeros, result_ones) << "Different patterns should produce different CRC values";
EXPECT_NE(result_zeros, result_alt) << "Different patterns should produce different CRC values";
EXPECT_NE(result_ones, result_alt) << "Different patterns should produce different CRC values";
}

// Test the crc32 method order independence (CRC should be order dependent)
TEST_F(TestUSCrcUnit, Crc32OrderDependence) {
std::vector<unsigned char> data1 = {0x01, 0x02, 0x03};
std::vector<unsigned char> data2 = {0x03, 0x02, 0x01};

quint32 result1 = US_Crc::crc32(0, data1.data(), data1.size());
quint32 result2 = US_Crc::crc32(0, data2.data(), data2.size());

EXPECT_NE(result1, result2) << "CRC should be order dependent - different order should give different results";
}

// Test the crc32 method with edge case lengths
TEST_F(TestUSCrcUnit, Crc32EdgeCaseLengths) {
unsigned char data[3] = {0x42, 0x43, 0x44};

// Test length 1
quint32 result1 = US_Crc::crc32(0, data, 1);

// Test length 2
quint32 result2 = US_Crc::crc32(0, data, 2);

// Test length 3
quint32 result3 = US_Crc::crc32(0, data, 3);

EXPECT_NE(result1, result2) << "Different lengths should produce different CRC values";
EXPECT_NE(result2, result3) << "Different lengths should produce different CRC values";
EXPECT_NE(result1, result3) << "Different lengths should produce different CRC values";
}

// Test the crc32 method XOR property verification
TEST_F(TestUSCrcUnit, Crc32XORPropertyVerification) {
unsigned char data = 0x42;

// The implementation does: crc = crc ^ 0xffffffffUL at start and end
// With initial CRC = 0, this should be: 0 ^ 0xFFFFFFFF = 0xFFFFFFFF at start
// Then table lookup and final XOR with 0xFFFFFFFF

quint32 result = US_Crc::crc32(0, &data, 1);

// Verify the result is not the input values
EXPECT_NE(result, 0u) << "CRC should not be zero for non-zero data";
EXPECT_NE(result, 0x42u) << "CRC should not equal input data";
EXPECT_NE(result, 0xFFFFFFFFu) << "CRC should not be all 1s for single byte";
}

// Test the crc32 method consistency across multiple calls
TEST_F(TestUSCrcUnit, Crc32ConsistencyMultipleCalls) {
QByteArray testData("consistency test data");
const unsigned char* data_ptr = reinterpret_cast<const unsigned char*>(testData.constData());
unsigned int length = testData.length();

// Calculate CRC multiple times
quint32 result1 = US_Crc::crc32(0, data_ptr, length);
quint32 result2 = US_Crc::crc32(0, data_ptr, length);
quint32 result3 = US_Crc::crc32(0, data_ptr, length);

EXPECT_EQ(result1, result2) << "Multiple calls should produce consistent results";
EXPECT_EQ(result2, result3) << "Multiple calls should produce consistent results";
EXPECT_EQ(result1, result3) << "Multiple calls should produce consistent results";
}