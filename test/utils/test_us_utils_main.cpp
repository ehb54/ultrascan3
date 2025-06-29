#include <QCoreApplication>
#include <QtTest>
#include <gtest/gtest.h>
#include "test_us_util.h"
#include "test_us_ana_profile.h"
#include "test_us_analyte.h"
#include "test_us_astfem_math.h"
#include "test_us_datafiles.h"
#include "test_us_crc.h"
#include "test_us_crypto.h"
#include "test_us_dataIO.h"
#include "test_us_db2.h"
#include "test_us_math2.h"
#include "test_us_matrix.h"
#include "test_us_memory.h"
#include "test_us_project.h"
#include "test_us_simparms.h"
#include "test_us_settings.h"
#include "test_us_time_state.h"

// Function to dynamically load and run all test classes
void runTest(QObject *test, int &status, int argc, char *argv[])
{
    status |= QTest::qExec(test, argc, argv);
    delete test;
}

void runGTest(int &status, int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    status |= RUN_ALL_TESTS();
}

// Main function: entry point for the test application
int main(int argc, char *argv[])
{
    // Create a QCoreApplication object to manage application-wide resources
    QCoreApplication app(argc, argv);

    // Variable to keep track of the overall status of all test executions
    int status = 0;

    // Run tests in the correct dependency order
    runTest(new TestUSUtil, status, argc, argv);
    runTest(new TestUSAnaProfile, status, argc, argv);
    runTest(new TestUSAnalyte, status, argc, argv);
    runTest(new TestUSAstfemMath, status, argc, argv);
    runTest(new TestUSDataFiles, status, argc, argv);
    runTest(new TestUSCrc, status, argc, argv);
    runTest(new TestUSCrypto, status, argc, argv);
    runTest(new TestUSDataFiles, status, argc, argv);
    runTest(new TestUSDataIO, status, argc, argv);
    runTest( new TestUSDB2, status, argc, argv);
    runTest(new TestUSMath2, status, argc, argv);
    runTest(new TestUSMatrix, status, argc, argv);
    runTest(new TestUSMemory, status, argc, argv);
    runTest(new TestUSProject, status, argc, argv);
    runTest(new TestUSSettings, status, argc, argv);
    runTest(new TestUSTimeState, status, argc, argv);
    runTest(new TestUSSimparms, status, argc, argv);

    // Run GoogleTest-based tests
    runGTest(status, argc, argv);

    // Run the tests for the TestUSDataFiles class
    {
        // Create an instance of the TestUSDataFiles class
        TestUSDataFiles testUSDatafiles;

        // Execute the tests in the TestUSDataFiles class
        // qExec returns the status of the test execution
        // Combine the status with the overall status using the bitwise OR operator
        status |= QTest::qExec(&testUSDatafiles, argc, argv);
    }

    // Run the tests for the TestUS_SimulationParameters class
    {
        // Execute the tests in the TestUSDataFiles class
        // qExec returns the status of the test execution
        // Combine the status with the overall status using the bitwise OR operator
        status |= QTest::qExec(&testUSDatafiles, argc, argv);
    }

    // Run the tests for the TestUS_SimulationParameters class
    {
        // Create an instance of the TestUS_SimulationParameters class
        TestUS_SimulationParameters test_SimulationParameters;

        // Execute the tests in the TestUSDataFiles class
        // qExec returns the status of the test execution
        // Combine the status with the overall status using the bitwise OR operator
        status |= QTest::qExec(&test_SimulationParameters, argc, argv);
    }

    // Return the overall status of all test executions
    // A non-zero value indicates that one or more tests failed
    return status;
}
