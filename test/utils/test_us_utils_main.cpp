#include <QCoreApplication>
#include <QtTest>
#include "test_us_util.h"
#include "test_us_ana_profile.h"
#include "test_us_datafiles.h"
#include "test_us_crc.h"
#include "test_us_crypto.h"
#include "test_us_dataIO.h"
#include "test_us_math2.h"
#include "test_us_matrix.h"
#include "test_us_memory.h"
#include "test_us_simparms.h"
#include "test_us_settings.h"
#include "test_us_time_state.h"

// Function to dynamically load and run all test classes
void runTest(QObject *test, int &status, int argc, char *argv[])
{
    status |= QTest::qExec(test, argc, argv);
    delete test;
}

// Main function: entry point for the test application
int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    int status = 0;

    // Run tests in the correct dependency order
    runTest(new TestUSUtil, status, argc, argv);
    runTest(new TestUSAnaProfile, status, argc, argv);
    runTest(new TestUSDataFiles, status, argc, argv);
    runTest(new TestUSCrc, status, argc, argv);
    runTest(new TestUSCrypto, status, argc, argv);
    runTest(new TestUSDataFiles, status, argc, argv);
    runTest(new TestUSDataIO, status, argc, argv);
    runTest(new TestUSMath2, status, argc, argv);
    runTest(new TestUSMatrix, status, argc, argv);
    runTest(new TestUSMemory, status, argc, argv);
    runTest(new TestUSSettings, status, argc, argv);
    runTest(new TestUSTimeState, status, argc, argv);
    runTest(new TestUSSimparms, status, argc, argv);

    return status;
}
