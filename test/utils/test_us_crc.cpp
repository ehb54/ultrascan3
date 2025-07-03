// test_us_crc.cpp - Converted to Google Test
#include "test_us_crc.h"
#include "us_crc.h"  // Include us_crc.h only once, in the .cpp file
#include <QByteArray>

// Use your custom Qt matchers
using namespace qt_matchers;

// TestUSCrc method implementations
void TestUSCrc::SetUp() {
    QtTestBase::SetUp();
    // Per-test setup for CRC tests
}

void TestUSCrc::TearDown() {
    // Per-test cleanup for CRC tests
    QtTestBase::TearDown();
}

// Suite-level setup for CRC tests
void TestUSCrc::SetUpTestSuite() {
    QtTestBase::SetUpTestSuite();
    // One-time setup for all TestUSCrc tests
}

// Suite-level cleanup for CRC tests
void TestUSCrc::TearDownTestSuite() {
    // One-time cleanup for all TestUSCrc tests
}

// Parameterized test structure for CRC32 tests
struct Crc32TestData {
    quint32 initialCrc;
    QByteArray data;
    quint32 expectedCrc;
    const char* description;
};

class TestUSCrcParameterized : public TestUSCrc,
                               public ::testing::WithParamInterface<Crc32TestData> {
};

// Test data for parameterized tests
INSTANTIATE_TEST_SUITE_P(
        Crc32Tests,
        TestUSCrcParameterized,
        ::testing::Values(
                Crc32TestData{0, QByteArray(""), 0, "empty"},
                Crc32TestData{0, QByteArray("123456789"), 0xcbf43926, "example1"},
                Crc32TestData{0, QByteArray("a"), 0xe8b7be43, "single_char"},
                Crc32TestData{0, QByteArray("abc"), 0x352441c2, "abc_string"},
                Crc32TestData{0, QByteArray("test"), 0xd87f7e0c, "simple_test"}  // Better test case
        ),
        [](const ::testing::TestParamInfo<Crc32TestData>& info) {
            return std::string(info.param.description);
        }
);

// Parameterized test for CRC32
TEST_P(TestUSCrcParameterized, Crc32Calculation) {
    // Arrange
    const Crc32TestData& testData = GetParam();

    // Act
    quint32 result = US_Crc::crc32(
            testData.initialCrc,
            reinterpret_cast<const unsigned char*>(testData.data.constData()),
            testData.data.length()
    );

    // Assert
    EXPECT_EQ(result, testData.expectedCrc)
                        << "CRC32 calculation failed for test case: " << testData.description
                        << " with input data: '" << testData.data.toStdString() << "'";
}

// Individual tests for specific scenarios
TEST_F(TestUSCrc, Crc32EmptyData) {
    // Test empty data
    quint32 result = US_Crc::crc32(0, nullptr, 0);
    EXPECT_EQ(result, 0u) << "CRC32 of empty data should be 0";
}

TEST_F(TestUSCrc, Crc32NullPointer) {
    // Test null pointer with zero length (should be safe)
    quint32 result = US_Crc::crc32(0, nullptr, 0);
    EXPECT_EQ(result, 0u) << "CRC32 with null pointer and zero length should be 0";
}

TEST_F(TestUSCrc, Crc32KnownValue) {
    // Test with a well-known CRC32 value
    QByteArray testData("123456789");
    quint32 result = US_Crc::crc32(
            0,
            reinterpret_cast<const unsigned char*>(testData.constData()),
            testData.length()
    );

    EXPECT_EQ(result, 0xcbf43926u)
                        << "CRC32 of '123456789' should be 0xcbf43926";
}

TEST_F(TestUSCrc, Crc32Consistency) {
    // Test that multiple calls with same data produce same result
    QByteArray testData("consistency_test");

    quint32 result1 = US_Crc::crc32(
            0,
            reinterpret_cast<const unsigned char*>(testData.constData()),
            testData.length()
    );

    quint32 result2 = US_Crc::crc32(
            0,
            reinterpret_cast<const unsigned char*>(testData.constData()),
            testData.length()
    );

    EXPECT_EQ(result1, result2)
                        << "Multiple CRC32 calculations of same data should be consistent";
}

TEST_F(TestUSCrc, Crc32DifferentData) {
    // Test that different data produces different CRC values
    QByteArray data1("test1");
    QByteArray data2("test2");

    quint32 crc1 = US_Crc::crc32(
            0,
            reinterpret_cast<const unsigned char*>(data1.constData()),
            data1.length()
    );

    quint32 crc2 = US_Crc::crc32(
            0,
            reinterpret_cast<const unsigned char*>(data2.constData()),
            data2.length()
    );

    EXPECT_NE(crc1, crc2)
                        << "Different input data should produce different CRC32 values";
}

TEST_F(TestUSCrc, Crc32WithInitialValue) {
    // Test CRC32 with different initial values
    QByteArray testData("test");

    quint32 result_zero = US_Crc::crc32(
            0,
            reinterpret_cast<const unsigned char*>(testData.constData()),
            testData.length()
    );

    quint32 result_ffff = US_Crc::crc32(
            0xFFFFFFFF,
            reinterpret_cast<const unsigned char*>(testData.constData()),
            testData.length()
    );

    // The results should be different when using different initial values
    EXPECT_NE(result_zero, result_ffff)
                        << "CRC32 with different initial values should produce different results";

    // Test that the actual result with 0xFFFFFFFF initial value is what we expect
    EXPECT_EQ(result_ffff, 113532655u)
                        << "CRC32 of 'test' with initial value 0xFFFFFFFF should be 113532655";
}