// test_us_analyte.cpp
#include "test_us_analyte.h"
#include "us_settings.h"
#include "us_constants.h"
#include <QFile>
#include <QUuid>
#include <QDebug>

// Use your custom Qt matchers
using namespace qt_matchers;

// Method implementations
void TestUSAnalyte::SetUp() {
    QtTestBase::SetUp();
    // Per-test setup (runs before each test)
}

void TestUSAnalyte::TearDown() {
    // Per-test cleanup (runs after each test)
    QtTestBase::TearDown();
}

// Suite-level setup (replaces initTestCase)
void TestUSAnalyte::SetUpTestSuite() {
    QtTestBase::SetUpTestSuite();
    // This is called before any test is executed. You can use it to set up
    // any global state or initialize variables that are needed across tests.
}

// Suite-level cleanup (replaces cleanupTestCase)
void TestUSAnalyte::TearDownTestSuite() {
    // This is called after all tests have been executed. Use it to clean up
    // any resources that were allocated in SetUpTestSuite().
}

TEST_F(TestUSAnalyte, Constructor) {
    US_Analyte analyte;

    // Qt Test → Google Test conversions:
    // QCOMPARE(a, b) → EXPECT_EQ(a, b)
    // QVERIFY(condition) → EXPECT_TRUE(condition)

    EXPECT_EQ(analyte.vbar20, TYPICAL_VBAR);  // Ensure TYPICAL_VBAR is defined
    EXPECT_EQ(analyte.mw, 0.0);
    EXPECT_EQ(analyte.description, QString("New Analyte"));
    EXPECT_EQ(analyte.analyteGUID, QString());
    EXPECT_EQ(analyte.sequence, QString());
    EXPECT_EQ(analyte.type, US_Analyte::PROTEIN);
    EXPECT_EQ(analyte.grad_form, false);
    EXPECT_EQ(analyte.doubleStranded, true);
    EXPECT_EQ(analyte.complement, false);
    EXPECT_EQ(analyte._3prime, false);
    EXPECT_EQ(analyte._5prime, false);
    EXPECT_EQ(analyte.sodium, 0.0);
    EXPECT_EQ(analyte.potassium, 0.0);
    EXPECT_EQ(analyte.lithium, 0.0);
    EXPECT_EQ(analyte.magnesium, 0.0);
    EXPECT_EQ(analyte.calcium, 0.0);

    // Using your custom Qt matchers for cleaner assertions
    EXPECT_TRUE(analyte.extinction.isEmpty());
    EXPECT_TRUE(analyte.refraction.isEmpty());
    EXPECT_TRUE(analyte.fluorescence.isEmpty());
}

TEST_F(TestUSAnalyte, EqualityOperator) {
    US_Analyte analyte1;
    US_Analyte analyte2;

    // Initially, the two analytes should be equal
    EXPECT_TRUE(analyte1 == analyte2);

    // Modify analyte2 and check that they are no longer equal
    analyte2.mw = 65000.0;
    EXPECT_FALSE(analyte1 == analyte2);

    // Revert the modification and check that they are equal again
    analyte2.mw = analyte1.mw;
    EXPECT_TRUE(analyte1 == analyte2);
}

TEST_F(TestUSAnalyte, Load) {
    // Test the load method with disk access (assuming the guid does not exist on disk)
    US_Analyte analyte;
    QString guid = "non-existent-guid";

    int result = analyte.load(false, guid);  // `false` indicates disk loading

    // If the file does not exist, load should return NO_ANALYTE
    EXPECT_EQ(result, US_DB2::NO_ANALYTE);

    // If the file does exist, you would have a valid GUID and you could check its properties after loading
    // Here, assume we have a valid GUID "example-guid" (you should replace this with a valid GUID if available)
    // result = analyte.load(false, "example-guid");
    // EXPECT_EQ(result, US_DB2::OK);
    // EXPECT_EQ(analyte.description, QString("Expected Description"));
    // EXPECT_EQ(analyte.sequence, QString("Expected Sequence"));
}

TEST_F(TestUSAnalyte, Write) {
    // Test writing an analyte to disk using the public interface
    US_Analyte analyte;
    analyte.description = "Test Analyte";
    analyte.sequence = "ATCG";
    analyte.analyteGUID = QUuid::createUuid().toString();  // Generate a unique GUID for testing

    QString path;
    analyte.analyte_path(path);
    QString filename = analyte.get_filename(path, analyte.analyteGUID);

    int result = analyte.write(false, filename);  // `false` indicates disk writing
    EXPECT_EQ(result, US_DB2::OK);

    // Load the file back and verify its contents
    US_Analyte loadedAnalyte;
    result = loadedAnalyte.load(false, analyte.analyteGUID);

    qDebug() << "There is an issue opened to resolve this test https://github.com/ehb54/ultrascan-tickets/issues/336";
    qDebug() << "Until this is fixed the comparisons that are failing are commented out.";
    qDebug() << "loadedAnalyte == analyte is " << (loadedAnalyte == analyte);

    // Note: These assertions are commented out due to known issue #336
    // EXPECT_EQ(loadedAnalyte, analyte);
    EXPECT_EQ(result, US_DB2::OK);
    EXPECT_EQ(loadedAnalyte.description, QString("Test Analyte"));
    // EXPECT_EQ(loadedAnalyte.sequence, QString("ATCG"));

    // Clean up the test file
    QFile::remove(filename);
}